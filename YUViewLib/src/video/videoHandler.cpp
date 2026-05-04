/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut für Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "videoHandler.h"

#include <QPainter>
#include <QMessageBox>

#include "common/FunctionsGui.h"
#include "common/Logger.h"

namespace video
{

videoHandler::videoHandler()
{
}

void videoHandler::slotVideoControlChanged()
{
  // Update the controls and get the new selected size
  auto newSize = getNewSizeFromControls();

  // Set the current frame in the buffer to be invalid
  this->currentImageIndex = -1;
  this->hasError          = false;
  this->errorMessage.clear();

  if (newSize != frameSize && newSize.isValid())
  {
    // Validate that the new resolution does not require more bytes per frame than the file has
    if (fileSize > 0)
    {
      int64_t newBpf = bytesPerFrameForSize(newSize);
      if (newBpf > fileSize)
      {
        QMessageBox::warning(
          nullptr,
          "Resolution Too Large",
          QString("The selected resolution (%1x%2) requires %3 bytes per frame,\n"
                  "but the source file is only %4 bytes.\n\n"
                  "The resolution has been reverted.")
            .arg(newSize.width)
            .arg(newSize.height)
            .arg(newBpf)
            .arg(fileSize));

        // Revert UI controls to the current frame size
        revertSizeControlsTo(frameSize);
        return;
      }
    }

    // Set the new size and update the controls.
    this->advanceRawDataGeneration();
    this->advanceCacheGeneration();
    this->cacheJobToken++;
    this->setFrameSize(newSize);
    // The frame size changed. We need to redraw/re-cache.
    emit signalHandlerChanged(true, RECACHE_CLEAR);
  }
}

void videoHandler::setFrameSize(Size size)
{
  if (size != frameSize)
  {
    this->currentFrameRawData_frameIndex = -1;
    this->currentImageIndex              = -1;
    this->rawData_frameIndex             = -1;
  }

  FrameHandler::setFrameSize(size);
}

ItemLoadingState videoHandler::needsLoading(int frameIdx, bool loadRawValues)
{
  if (loadRawValues)
  {
    // First, let's check the raw values buffer.
    auto state = needsLoadingRawValues(frameIdx);
    if (state != ItemLoadingState::LoadingNotNeeded)
      return state;
  }

  // Lock the mutex for checking the cache
  QMutexLocker lock(&imageCacheAccess);

  // The raw values are not needed.
  if (frameIdx == currentImageIndex)
  {
    if (doubleBufferImageFrameIndex == frameIdx + 1)
    {
      LOGT("videoHandler::needsLoading frameIdx {} is current and {} found in double buffer",
           frameIdx, frameIdx + 1);
      return ItemLoadingState::LoadingNotNeeded;
    }
    else if (isCachedFrameValidLocked(frameIdx + 1))
    {
      LOGT("videoHandler::needsLoading frameIdx {} is current and {} found in cache",
           frameIdx, frameIdx + 1);
      return ItemLoadingState::LoadingNotNeeded;
    }
    else
    {
      // The next frame is not in the double buffer so that needs to be loaded.
      LOGT("videoHandler::needsLoading frameIdx {} is current but {} not found in double buffer",
           frameIdx, frameIdx + 1);
      return ItemLoadingState::LoadingNeededDoubleBuffer;
    }
  }

  // Check the double buffer
  if (doubleBufferImageFrameIndex == frameIdx)
  {
    // The frame in question is in the double buffer...
    if (isCachedFrameValidLocked(frameIdx + 1))
    {
      // ... and the one after that is in the cache.
      LOGT("videoHandler::needsLoading frameIdx {} found in double buffer. Next frame in cache.",
           frameIdx);
      return ItemLoadingState::LoadingNotNeeded;
    }
    else
    {
      // .. and the one after that is not in the cache.
      // Loading of the given frame index is not needed because it is in the double buffer but if
      // you draw it, the double buffer needs an update.
      LOGT("videoHandler::needsLoading frameIdx {} found in double buffer", frameIdx);
      return ItemLoadingState::LoadingNeededDoubleBuffer;
    }
  }

  // Check the cache
  if (isCachedFrameValidLocked(frameIdx))
  {
    // What about the next frame? Is it also in the cache or in the double buffer?
    if (doubleBufferImageFrameIndex == frameIdx + 1)
    {
      LOGT("videoHandler::needsLoading frameIdx {} in cache and {} found in double buffer",
           frameIdx, frameIdx + 1);
      return ItemLoadingState::LoadingNotNeeded;
    }
    else if (isCachedFrameValidLocked(frameIdx + 1))
    {
      LOGT("videoHandler::needsLoading frameIdx {} in cache and {} found in cache",
           frameIdx, frameIdx + 1);
      return ItemLoadingState::LoadingNotNeeded;
    }
    else
    {
      // The next frame is not in the double buffer so that needs to be loaded.
      LOGT("videoHandler::needsLoading frameIdx {} found in cache but {} not found in double buffer",
           frameIdx, frameIdx + 1);
      return ItemLoadingState::LoadingNeededDoubleBuffer;
    }
  }

  // Frame not in buffer. Return false and request the background loading thread to load the frame.
  LOGT("videoHandler::needsLoading frameIdx {} not found in cache - request load", frameIdx);
  return ItemLoadingState::LoadingNeeded;
}

void videoHandler::drawFrame(QPainter *painter, int frameIdx, double zoomFactor, bool drawRawValues)
{
  // Check if the frameIdx changed and if we have to load a new frame
  if (frameIdx != currentImageIndex)
  {
    // The current buffer is out of date. Update it.

    // Check the double buffer
    if (frameIdx == doubleBufferImageFrameIndex)
    {
      currentImage      = doubleBufferImage;
      currentImageIndex = frameIdx;
      LOGT("videoHandler::drawFrame frameIdx {} loaded from double buffer", frameIdx);
    }
    else
    {
      QMutexLocker lock(&imageCacheAccess);
      auto it = imageCache.find(frameIdx);
      if (it != imageCache.end() && it->generation == cacheGeneration)
      {
        currentImage      = it->image;
        currentImageIndex = frameIdx;
        LOGT("videoHandler::drawFrame frameIdx {} loaded from cache", frameIdx);
      }
    }
  }

  LOGT("videoHandler::drawFrame frameIdx {} currentImageIndex {}",
       frameIdx, currentImageIndex);

  // Create the video QRect with the size of the sequence and center it.
  QRect videoRect;
  videoRect.setSize(QSize(frameSize.width * zoomFactor, frameSize.height * zoomFactor));
  videoRect.moveCenter(QPoint(0, 0));

  // Draw the current image (currentImage)
  currentImageSetMutex.lock();
  painter->drawImage(videoRect, currentImage);
  currentImageSetMutex.unlock();

  if (drawRawValues && zoomFactor >= SPLITVIEW_DRAW_VALUES_ZOOMFACTOR)
  {
    // Draw the pixel values onto the pixels
    drawPixelValues(painter, frameIdx, videoRect, zoomFactor);
  }
}

QImage videoHandler::calculateDifference(FrameHandler *   item2,
                                         const int        frameIdxItem0,
                                         const int        frameIdxItem1,
                                         QList<InfoItem> &differenceInfoList,
                                         const int        amplificationFactor,
                                         const bool       markDifference)
{
  // Try to cast item2 to a videoHandler
  videoHandler *videoItem2 = dynamic_cast<videoHandler *>(item2);
  if (videoItem2 == nullptr)
  {
    // The item2 is not a videoItem but this one is.
    if (currentImageIndex != frameIdxItem0)
      loadFrame(frameIdxItem0);
    // Call the FrameHandler implementation to calculate the difference
    return FrameHandler::calculateDifference(item2,
                                             frameIdxItem0,
                                             frameIdxItem1,
                                             differenceInfoList,
                                             amplificationFactor,
                                             markDifference);
  }

  // Load the right images, if not already loaded)
  if (currentImageIndex != frameIdxItem0)
    loadFrame(frameIdxItem0);
  if (videoItem2->currentImageIndex != frameIdxItem1)
    videoItem2->loadFrame(frameIdxItem1);

  return FrameHandler::calculateDifference(
      item2, frameIdxItem0, frameIdxItem1, differenceInfoList, amplificationFactor, markDifference);
}

QRgb videoHandler::getPixelVal(int x, int y)
{
  return currentImage.pixel(x, y);
}

int videoHandler::getNrFramesCached() const
{
  QMutexLocker lock(&imageCacheAccess);
  int count = 0;
  for (auto it = imageCache.begin(); it != imageCache.end(); ++it)
    if (it->generation == cacheGeneration)
      count++;
  return count;
}

// Put the frame into the cache (if it is not already in there)
void videoHandler::cacheFrame(int frameIdx, bool testMode)
{
  LOGT("videoHandler::cacheFrame {} {}", frameIdx, testMode ? "testMode" : "");

  if (isCachedFrameValid(frameIdx) && !testMode)
  {
    // No need to add it again
    LOGT("videoHandler::cacheFrame frame {} already in cache - returning", frameIdx);
    return;
  }

  // Load the frame. While this is happening in the background the frame size must not change.
  QImage cacheImage;
  loadFrameForCaching(frameIdx, cacheImage);

  // Put it into the cache
  if (!cacheImage.isNull())
  {
    LOGT("videoHandler::cacheFrame insert frame {} into cache", frameIdx);
    QMutexLocker imageCacheLock(&imageCacheAccess);
    if (!testMode)
      imageCache.insert(frameIdx, {cacheImage, cacheGeneration});
  }
  else
    LOGT("videoHandler::cacheFrame loading frame {} for caching failed", frameIdx);
}

unsigned videoHandler::getCachingFrameSize() const
{
  const auto hasAlpha = false;
  auto       bytes    = functionsGui::bytesPerPixel(functionsGui::platformImageFormat(hasAlpha));
  return this->frameSize.width * this->frameSize.height * bytes;
}

QList<int> videoHandler::getCachedFrames() const
{
  QMutexLocker lock(&imageCacheAccess);
  QList<int>   keys;
  for (auto it = imageCache.begin(); it != imageCache.end(); ++it)
    if (it->generation == cacheGeneration)
      keys.append(it.key());
  return keys;
}

int videoHandler::getNumberCachedFrames() const
{
  QMutexLocker lock(&imageCacheAccess);
  int count = 0;
  for (auto it = imageCache.begin(); it != imageCache.end(); ++it)
    if (it->generation == cacheGeneration)
      count++;
  return count;
}

bool videoHandler::isInCache(int idx) const
{
  return isCachedFrameValid(idx);
}

void videoHandler::removeFrameFromCache(int frameIdx)
{
  LOGD("removeFrameFromCache {}", frameIdx);
  QMutexLocker lock(&imageCacheAccess);
  imageCache.remove(frameIdx);
  lock.unlock();
}

void videoHandler::removeAllFrameFromCache()
{
  LOGD("removeAllFrameFromCache");
  QMutexLocker lock(&imageCacheAccess);
  imageCache.clear();
}

void videoHandler::loadFrame(int frameIndex, bool loadToDoubleBuffer)
{
  LOGD(
      "videoHandler::loadFrame {} {}", frameIndex, (loadToDoubleBuffer) ? "toDoubleBuffer" : "");

  if (requestedFrame_idx != frameIndex)
  {
    // Lock the mutex for requesting raw data (we share the requestedFrame buffer with the caching
    // function)
    QMutexLocker lock(&requestDataMutex);

    // Request the image to be loaded
    emit signalRequestFrame(frameIndex, false);

    if (requestedFrame_idx != frameIndex)
      // Loading failed
      return;
  }

  if (loadToDoubleBuffer)
  {
    // Save the requested frame in the double buffer
    doubleBufferImage           = requestedFrame;
    doubleBufferImageFrameIndex = frameIndex;
  }
  else
  {
    // Set the requested frame as the current frame
    QMutexLocker imageLock(&currentImageSetMutex);
    currentImage      = requestedFrame;
    currentImageIndex = frameIndex;
  }
}

void videoHandler::loadFrameForCaching(int frameIndex, QImage &frameToCache)
{
  LOGT("videoHandler::loadFrameForCaching {}", frameIndex);

  QMutexLocker lock(&requestDataMutex);

  // Request the image to be loaded
  emit signalRequestFrame(frameIndex, true);

  if (requestedFrame_idx != frameIndex)
    // Loading failed
    return;

  frameToCache = requestedFrame;
}

void videoHandler::invalidateAllBuffers()
{
  currentFrameRawData_frameIndex = -1;
  rawData_frameIndex             = -1;

  // Set the current frame in the buffer to be invalid
  currentImageIndex = -1;
  currentImageSetMutex.lock();
  currentImage = QImage();
  currentImageSetMutex.unlock();
  requestedFrame_idx = -1;

  imageCache.clear();
  cacheGeneration++;
  cacheJobToken++;
  hasError = false;
  errorMessage.clear();
}

void videoHandler::activateDoubleBuffer()
{
  if (doubleBufferImageFrameIndex != -1)
  {
    currentImage      = doubleBufferImage;
    currentImageIndex = doubleBufferImageFrameIndex;
    LOGT("videoHandler::drawFrame {} loaded from double buffer", currentImageIndex);
  }
}

QLayout *videoHandler::createVideoHandlerControls(bool)
{
  return nullptr;
}

ItemLoadingState videoHandler::needsLoadingRawValues(int frameIndex)
{
  return (this->currentFrameRawData_frameIndex == frameIndex) ? ItemLoadingState::LoadingNotNeeded
                                                              : ItemLoadingState::LoadingNeeded;
}

bool videoHandler::isCachedFrameValid(int frameIdx) const
{
  QMutexLocker lock(&imageCacheAccess);
  return isCachedFrameValidLocked(frameIdx);
}

bool videoHandler::isCachedFrameValidLocked(int frameIdx) const
{
  auto it = imageCache.find(frameIdx);
  if (it == imageCache.end())
    return false;
  return it->generation == cacheGeneration;
}

void videoHandler::purgeExpiredCacheEntries()
{
  QMutexLocker lock(&imageCacheAccess);
  for (auto it = imageCache.begin(); it != imageCache.end();)
  {
    if (it->generation != cacheGeneration)
      it = imageCache.erase(it);
    else
      ++it;
  }
}

} // namespace video
