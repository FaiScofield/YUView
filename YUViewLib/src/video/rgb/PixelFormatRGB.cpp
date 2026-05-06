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

#include "PixelFormatRGB.h"
#include "common/Logger.h"

#include <QStringList>
#include <regex>

namespace video::rgb
{

PixelFormatRGB::PixelFormatRGB(unsigned          bitsPerSample,
                               DataLayout        dataLayout,
                               ChannelOrder      channelOrder,
                               AlphaMode         alphaMode,
                               Endianness        endianness,
                               PaddingInfo       paddingInfo,
                               bool              bytePacking,
                               DiffCompDepthType diffType)
    : bitsPerSample(bitsPerSample), dataLayout(dataLayout), channelOrder(channelOrder),
      alphaMode(alphaMode), endianness(endianness), paddingInfo(paddingInfo),
      bytePacking(bytePacking)
{
  setDiffCompType(diffType);
}

PixelFormatRGB::PixelFormatRGB(DiffCompDepthType diffType,
                               ChannelOrder      channelOrder,
                               AlphaMode         alphaMode,
                               PaddingInfo       paddingInfo,
                               Endianness        endianness)
    : channelOrder(channelOrder), alphaMode(alphaMode), paddingInfo(paddingInfo),
      endianness(endianness), dataLayout(DataLayout::Interleaved), bytePacking(true)
{
  // handle the conflict between AlphaMode & PaddingInfo
  if (alphaMode == AlphaMode::None && paddingInfo == PaddingInfo::NoPadding)
  {
    if (diffType == DiffCompDepthType::BPP16_RGBA5551 ||
        diffType == DiffCompDepthType::BPP32_RGBA1010102)
      this->alphaMode = AlphaMode::InLsb;
  }
  else if (alphaMode != AlphaMode::None && paddingInfo != PaddingInfo::NoPadding)
  {
    this->paddingInfo = PaddingInfo::NoPadding;
  }

  setDiffCompType(diffType);
}

PixelFormatRGB::PixelFormatRGB(const std::string &name)
{
  if (name.empty() || name == "Unknown Pixel Format")
    return;

  std::string lowerName = name;
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

  // Try to parse diff component depth format (e.g., BGR233, BGRA5551, ABGR1555)
  auto parseDiffCompFormat = [this, &lowerName]() -> bool
  {
    // Pattern: [A/X]RGBnnn[n] or RGB[A/X]nnn[n] where n are digits
    std::regex diffCompPattern(R"(([ax])?([rgb]+)([ax])?([0-9]+))", std::regex::icase);
    std::smatch match;

    if (!std::regex_match(lowerName, match, diffCompPattern))
      return false;

    // Extract components
    std::string alphaOrPaddingStart = match[1].str();
    std::string rgbOrderPart = match[2].str();
    std::string alphaOrPaddingEnd = match[3].str();
    std::string bitsPart = match[4].str();

    // Convert rgbOrderPart to uppercase
    AlphaMode   alphaMode   = AlphaMode::None;
    PaddingInfo paddingInfo = PaddingInfo::NoPadding;
    if (!alphaOrPaddingStart.empty())
    {
      if (alphaOrPaddingStart[0] == 'a')
        alphaMode = AlphaMode::InMsb;
      else if (alphaOrPaddingStart[0] == 'x')
        paddingInfo = PaddingInfo::PaddingInMSB;
    }
    else if (!alphaOrPaddingEnd.empty())
    {
      if (alphaOrPaddingEnd[0] == 'a')
        alphaMode = AlphaMode::InLsb;
      else if (alphaOrPaddingEnd[0] == 'x')
        paddingInfo = PaddingInfo::PaddingInLSB;
    }

    // Set channel order
    std::transform(rgbOrderPart.begin(), rgbOrderPart.end(), rgbOrderPart.begin(), ::toupper);
    auto order = ChannelOrderMapper.getValue(rgbOrderPart);
    if (!order)
      return false;

    if (bitsPart == "332" || bitsPart == "323" || bitsPart == "233")
    {
      // BPP8_RGB332
      this->diffCompType  = DiffCompDepthType::BPP8_RGB332;
      this->bitsPerPixel  = 8;
      this->bitsPerSample = 3;
      this->alphaMode     = AlphaMode::None;
      this->paddingInfo   = PaddingInfo::NoPadding;
      this->channelOrder  = *order;
      return true;
    }
    else if (bitsPart == "565" || bitsPart == "655" || bitsPart == "556")
    {
      // RGB565
      this->diffCompType  = DiffCompDepthType::BPP16_RGB565;
      this->bitsPerPixel  = 16;
      this->bitsPerSample = 6;
      this->alphaMode     = AlphaMode::None;
      this->paddingInfo   = PaddingInfo::NoPadding;
      this->channelOrder  = *order;
      return true;
    }
    else if (bitsPart == "5551" || bitsPart == "1555")
    {
      // RGBA5551
      this->diffCompType  = DiffCompDepthType::BPP16_RGBA5551;
      this->bitsPerPixel  = 16;
      this->bitsPerSample = 5;
      this->alphaMode     = alphaMode;
      this->paddingInfo   = paddingInfo;
      this->channelOrder  = *order;

      return true;
    }
    else if (bitsPart == "1010102" || bitsPart == "2101010")
    {
      // BPP32_RGBA1010102
      this->diffCompType  = DiffCompDepthType::BPP32_RGBA1010102;
      this->bitsPerPixel  = 32;
      this->bitsPerSample = 10;
      this->alphaMode     = alphaMode;
      this->paddingInfo   = paddingInfo;
      this->channelOrder  = *order;
      return true;
    }

    return false;
  };

  if (parseDiffCompFormat()) {
    // Set common properties
    this->dataLayout  = DataLayout::Interleaved;
    this->bytePacking = true;
    return;
  }

  /* rgba 10bit [bytepacking] [planar] [be] */
  this->bytePacking = (lowerName.find(" bytepacking") != std::string::npos);

  if (lowerName.find(" planar") != std::string::npos)
    this->dataLayout = DataLayout::Planar;

  if (lowerName.find(" be") != std::string::npos)
    this->endianness = Endianness::Big;

  // alpha & padding
  this->alphaMode   = AlphaMode::None;
  this->paddingInfo = PaddingInfo::NoPadding;

  // paddingInfo for non-bytepacking format
  if (!this->bytePacking)
  {
    if (lowerName.find(" paddinginmsb") != std::string::npos)
      this->paddingInfo = PaddingInfo::PaddingInMSB;
    else if (lowerName.find(" paddinginlsb") != std::string::npos)
      this->paddingInfo = PaddingInfo::PaddingInLSB;
  }

  QStringList splitStr        = QString::fromStdString(lowerName).split(" ");
  std::string channelOrderStr = splitStr[0].toStdString();
  char        firstChar       = lowerName[0];
  char        lastChar        = lowerName.back();
  if (this->bytePacking)
  {
    if (firstChar == 'a')
    {
      this->alphaMode = AlphaMode::InMsb;
      channelOrderStr = channelOrderStr.substr(1);
    }
    else if (firstChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInMSB;
      channelOrderStr   = channelOrderStr.substr(1);
    }
    if (lastChar == 'a')
    {
      this->alphaMode = AlphaMode::InLsb;
      channelOrderStr.pop_back();
    }
    else if (lastChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInLSB;
      channelOrderStr.pop_back();
    }

    // For bytepacking case, channelOrderStr is MSB order, need to reverse to get LSB order
    channelOrderStr = std::string(channelOrderStr.rbegin(), channelOrderStr.rend());
  }
  else
  {
    if (firstChar == 'a')
    {
      this->alphaMode = AlphaMode::First;
      channelOrderStr = channelOrderStr.substr(1);
    }
    else if (firstChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInLSB;
      channelOrderStr   = channelOrderStr.substr(1);
    }
    if (lastChar == 'a')
    {
      this->alphaMode = AlphaMode::Last;
      channelOrderStr.pop_back();
    }
    else if (lastChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInMSB;
      channelOrderStr.pop_back();
    }
  }

  // check order
  std::transform(
    channelOrderStr.begin(), channelOrderStr.end(), channelOrderStr.begin(), ::toupper);
  auto order = ChannelOrderMapper.getValue(channelOrderStr);
  if (order)
    this->channelOrder = *order;

  // depth
  this->bitsPerSample = 8; // default set to 8bit
  if (splitStr.length() > 1)
  {
    std::string depthStr = splitStr[1].toStdString();
    auto        bitIdx   = depthStr.find("bit");
    if (bitIdx != std::string::npos)
    {
      std::string bitStr = depthStr.substr(0, bitIdx);
      if (!bitStr.empty())
        this->bitsPerSample = std::atoi(bitStr.c_str());
    }
    else
      LOGW("no 'xxbit' in depthStr: {}", depthStr);
  }
  else
  {
    LOGW("no 'xxbit' in format name: {}", lowerName);
  }

  const int nbChannels = nrChannels();
  this->bitsPerPixel   = this->bytePacking ? (this->bitsPerSample * nbChannels)
                                           : ((this->bitsPerSample + 7) / 8 * 8 * nbChannels);
}

bool PixelFormatRGB::isValid() const
{
  bool depthValid = this->bitsPerSample >= 1 && this->bitsPerSample <= 32;
  bool alphaValid = true;
  bool paddingValid = true;
  bool bytePackValid = true;

  switch (this->diffCompType)
  {
  case DiffCompDepthType::BPP8_RGB332: {
    depthValid = this->bitsPerPixel == 8;
    alphaValid &= this->alphaMode == AlphaMode::None;
    paddingValid &= this->paddingInfo == PaddingInfo::NoPadding;
    bytePackValid &= this->bytePacking == true;
  } break;
  case DiffCompDepthType::BPP16_RGB565: {
    depthValid = this->bitsPerPixel == 16;
    alphaValid &= this->alphaMode == AlphaMode::None;
    paddingValid &= this->paddingInfo == PaddingInfo::NoPadding;
    bytePackValid &= this->bytePacking == true;
  } break;
  case DiffCompDepthType::BPP16_RGBA5551: {
    depthValid = this->bitsPerPixel == 16;
    alphaValid &=
      (this->alphaMode == AlphaMode::None && this->paddingInfo != PaddingInfo::NoPadding) ||
      (this->alphaMode != AlphaMode::None && this->paddingInfo == PaddingInfo::NoPadding);
    bytePackValid &= this->bytePacking == true;
  } break;
  case DiffCompDepthType::BPP32_RGBA1010102: {
    depthValid = this->bitsPerPixel == 32;
    alphaValid &=
      (this->alphaMode == AlphaMode::None && this->paddingInfo != PaddingInfo::NoPadding) ||
      (this->alphaMode != AlphaMode::None && this->paddingInfo == PaddingInfo::NoPadding);
    bytePackValid &= this->bytePacking == true;
  } break;
  case DiffCompDepthType::None:
  default: {
    if (this->bytePacking) // RGBA[X]4444, RGB101010bp, ...
    {
      depthValid &= this->bitsPerSample % 8 != 0;
      paddingValid &= this->paddingInfo == PaddingInfo::NoPadding;
    }
    else // RGBA[X]8888, RGB161616, RGB101010_bytepacking
    {
      // depthValid &= this->bitsPerSample * ((this->hasAlpha() || this->hasPadding()) ? 4 : 3) % 8 == 0;
      // if (this->bitsPerSample % 8 != 0)
      //   paddingValid &= this->paddingInfo != PaddingInfo::NoPadding;
    }
  }
  break;
  }

  return depthValid && alphaValid && paddingValid && bytePackValid;
}

std::string PixelFormatRGB::getName() const
{
  if (!this->isValid())
    return "Unknown Pixel Format";

  if (!this->name.empty())
    return this->name;

  std::string orderName = std::string(ChannelOrderMapper.getName(this->channelOrder));
  std::string finalName;

  /* diff component depth case, named with MSB channel order */
  if (this->diffCompType != DiffCompDepthType::None)
  {
    int rBits = 0, gBits = 0, bBits = 0, aBits = 0;
    switch (this->diffCompType)
    {
    case DiffCompDepthType::BPP8_RGB332:
      rBits = 3; gBits = 3; bBits = 2; aBits = 0;
      break;
    case DiffCompDepthType::BPP16_RGB565:
      rBits = 5; gBits = 6; bBits = 5; aBits = 0;
      break;
    case DiffCompDepthType::BPP16_RGBA5551:
      rBits = 5; gBits = 5; bBits = 5; aBits = 1;
      break;
    case DiffCompDepthType::BPP32_RGBA1010102:
      rBits = 10; gBits = 10; bBits = 10; aBits = 2;
      break;
    default:
      break;
    }

    // Build the name with channel letters and bit counts (MSB to LSB order)
    finalName = orderName;
    for (char c : orderName)
    {
      if (c == 'R')
        finalName += std::to_string(rBits);
      else if (c == 'G')
        finalName += std::to_string(gBits);
      else if (c == 'B')
        finalName += std::to_string(bBits);
    }

    // Insert alpha/padding position prefix or suffix
    if (this->alphaMode == AlphaMode::InMsb)
    {
      finalName.insert(0, "A");
      finalName.insert(4, std::to_string(aBits));
    }
    else if (this->alphaMode == AlphaMode::InLsb)
    {
      finalName.insert(3, "A");
      finalName += std::to_string(aBits);
    }
    else if (this->paddingInfo == PaddingInfo::PaddingInMSB)
    {
      finalName.insert(0, "X");
      finalName.insert(4, std::to_string(aBits));
    }
    else if (this->paddingInfo == PaddingInfo::PaddingInLSB)
    {
      finalName.insert(3, "X");
      finalName += std::to_string(aBits);
    }
    this->name = finalName;
    return finalName;
  }

  /* bytepacking case, named with MSB channel order */
  if (this->bytePacking)
  {
    if (this->alphaMode == AlphaMode::InLsb)
      finalName = orderName + "A";
    else if (this->paddingInfo == PaddingInfo::PaddingInLSB)
      finalName = orderName + "X";
    else if (this->alphaMode == AlphaMode::InMsb)
      finalName = "A" + orderName;
    else if (this->paddingInfo == PaddingInfo::PaddingInMSB)
      finalName = "X" + orderName;
    else
      finalName = orderName;

    finalName += " " + std::to_string(this->bitsPerSample) + "bit";
    finalName += " bytepacking";
  }
  /* normal case, named with LSB channel order */
  else {
    if (this->alphaMode == AlphaMode::First)
      finalName = "A"  + orderName;
    else if (this->paddingInfo == PaddingInfo::PaddingInLSB)
      finalName = "X" + orderName;
    else if (this->alphaMode == AlphaMode::Last)
      finalName = orderName + "A";
    else if (this->paddingInfo == PaddingInfo::PaddingInMSB)
      finalName = orderName + "X";
    else
      finalName = orderName;

    finalName += " " + std::to_string(this->bitsPerSample) + "bit";

    // 添加 paddingInfo 到普通格式的命名中
    if (this->paddingInfo != PaddingInfo::NoPadding)
    {
      if (this->paddingInfo == PaddingInfo::PaddingInMSB)
        finalName += " paddingInMsb";
      else if (this->paddingInfo == PaddingInfo::PaddingInLSB)
        finalName += " paddingInLsb";
    }
  }

  if (this->dataLayout == DataLayout::Planar)
    finalName += " planar";

  if (this->bitsPerPixel > 8 && this->endianness == Endianness::Big)
    finalName += " BE";

  this->name = finalName;
  return finalName;
}

void PixelFormatRGB::setDiffCompType(DiffCompDepthType diffCompType)
{
  this->name.clear();
  this->diffCompType = diffCompType;
  if (diffCompType == DiffCompDepthType::None)
  {
    const auto Bps = (this->bitsPerSample + 7) / 8;
    this->bitsPerPixel = (this->bytePacking ? this->bitsPerSample : Bps * 8) * nrChannels();
    return;
  }

  this->dataLayout = DataLayout::Interleaved;
  this->bytePacking = true;

  /* name with MSB -> LSB order */
  switch (diffCompType)
  {
    case DiffCompDepthType::BPP8_RGB332: {
      this->bitsPerPixel = 8;
      this->bitsPerSample = 3;
      this->alphaMode = AlphaMode::None;
      this->paddingInfo = PaddingInfo::NoPadding;
    } break;
    case DiffCompDepthType::BPP16_RGB565 : {
      this->bitsPerPixel = 16;
      this->bitsPerSample = 6;
      this->alphaMode = AlphaMode::None;
      this->paddingInfo = PaddingInfo::NoPadding;
    } break;
    case DiffCompDepthType::BPP16_RGBA5551 : {
      this->bitsPerPixel = 16;
      this->bitsPerSample = 5;
      if (this->alphaMode == AlphaMode::None && this->paddingInfo == PaddingInfo::NoPadding)
      {
        this->alphaMode   = AlphaMode::InLsb;
        this->paddingInfo = PaddingInfo::NoPadding;
      }
      else if (this->alphaMode != AlphaMode::None && this->paddingInfo != PaddingInfo::NoPadding)
        this->paddingInfo = PaddingInfo::NoPadding;
    }
    break;
    case DiffCompDepthType::BPP32_RGBA1010102 : {
      this->bitsPerPixel = 32;
      this->bitsPerSample = 10;
      if (this->alphaMode == AlphaMode::None && this->paddingInfo == PaddingInfo::NoPadding)
      {
        this->alphaMode = AlphaMode::First;
        this->paddingInfo = PaddingInfo::NoPadding;
      }
      else if (this->alphaMode != AlphaMode::None && this->paddingInfo != PaddingInfo::NoPadding)
        this->paddingInfo = PaddingInfo::NoPadding;
    } break;
    default:
      break;
  }
}

#if 0
std::vector<DiffCompDepthType> getSupportedDiffCompDepthTypes(unsigned bitsPerPixel)
{
  std::vector<DiffCompDepthType> supportedTypes;

  if (bitsPerPixel == 8)
  {
    supportedTypes.push_back(DiffCompDepthType::BPP8_RGB332);
  }
  else if (bitsPerPixel == 16)
  {
    supportedTypes.push_back(DiffCompDepthType::BPP16_RGB565);
    supportedTypes.push_back(DiffCompDepthType::BPP16_RGBA5551);
  }
  else if (bitsPerPixel == 32)
  {
    supportedTypes.push_back(DiffCompDepthType::BPP32_RGBA1010102);
  }

  return supportedTypes;
}
#endif

/* Get the number of bytes for a frame with this RGB format and the given size
 */
std::size_t PixelFormatRGB::bytesPerFrame(Size frameSize) const
{
  return bytesPerFrameWithVirtualSize(frameSize);
}

std::size_t PixelFormatRGB::bytesPerFrameWithVirtualSize(const Size &frameSize) const
{
  if (!isValid() || !frameSize.isValid())
    return 0;

  // If has valid virtual size, validate and use it for calculation
  if (frameSize.hasValidVirtualSize()) {
    unsigned rowPitch = getRowPitchForPlane(frameSize, true);
    unsigned minPitch = getRowPitchForPlane(frameSize, false);
    unsigned height   = getHeightForPlane(frameSize);

    if (height >= frameSize.height && rowPitch >= minPitch) {
      if (this->dataLayout == DataLayout::Planar)
        return (std::size_t)(rowPitch)*height * nrChannels();
      else
        return (std::size_t)(rowPitch)*height;
    }
  }

  // Use original calculation logic (no virtual size)
  const size_t numSamples = std::size_t(frameSize.height) * std::size_t(frameSize.width);
  size_t nrBytes = 0;

  if (this->diffCompType == DiffCompDepthType::None)
  {
    if (this->bytePacking)
    {
      if (this->dataLayout == DataLayout::Planar)
      {
        size_t pitch = (frameSize.width * this->bitsPerSample + 7) / 8;
        nrBytes      = pitch * frameSize.height * nrChannels();
      }
      else
      {
        size_t pitch = (frameSize.width * this->bitsPerSample * nrChannels() + 7) / 8;
        nrBytes      = pitch * frameSize.height;
      }
    }
    else
    {
      size_t Bps = (this->bitsPerSample + 7) / 8;
      nrBytes = numSamples * Bps * nrChannels();
    }
  }
  else
    nrBytes = numSamples * this->bitsPerPixel / 8;

  LOGD("PixelFormatRGB::bytesPerFrame {}, size {}x{}, bytes {}",
       this->getName(), frameSize.width, frameSize.height, nrBytes);
  return nrBytes;
}

bool PixelFormatRGB::validateAndNormalizeVirtualSize(Size &frameSize) const
{
  if (!isValid())
    return false;

  unsigned bps = this->bitsPerSample;
  unsigned channels = nrChannels();
  bool bytePacking = this->bytePacking;

  // Calculate theoretical minimum pitch
  unsigned minPitch = 0;
  if (bytePacking)
  {
    if (this->dataLayout == DataLayout::Planar)
      minPitch = (frameSize.width * bps + 7) / 8;
    else
      minPitch = (frameSize.width * bps * channels + 7) / 8;
  }
  else
  {
    size_t Bps = (bps + 7) / 8;
    if (this->dataLayout == DataLayout::Planar)
      minPitch = frameSize.width * Bps;
    else
      minPitch = frameSize.width * Bps * channels;
  }

  // Check user-provided values
  if (frameSize.rowPitches[0] >= minPitch && frameSize.virtualHeights[0] >= frameSize.height)
  {
    // User provided valid values
    frameSize.validVirtualPlaneNum = 1;
    return true;
  }
  else if (frameSize.rowPitches[0] > 0 && frameSize.rowPitches[0] < minPitch)
  {
    // User provided value but it's invalid, use theoretical minimum
    frameSize.rowPitches[0]     = minPitch;
    frameSize.virtualHeights[0] = frameSize.height;
    frameSize.validVirtualPlaneNum = 1;
    return true;
  }
  else
  {
    // Not provided or zero, use theoretical values
    frameSize.rowPitches[0]     = minPitch;
    frameSize.virtualHeights[0] = frameSize.height;
    frameSize.validVirtualPlaneNum = 0;
    return false;
  }
}

unsigned PixelFormatRGB::getRowPitchForPlane(const Size &frameSize, bool useVirtualSize) const
{
  // calculate theoretical value first
  unsigned bps         = this->bitsPerSample;
  unsigned channels    = nrChannels();
  bool     bytePacking = this->bytePacking;
  unsigned minPitch    = 0;

  if (bytePacking) {
    if (this->dataLayout == DataLayout::Planar)
      minPitch = (frameSize.width * bps + 7) / 8;
    else
      minPitch = (frameSize.width * bps * channels + 7) / 8;
  } else {
    size_t Bps = (bps + 7) / 8;
    if (this->dataLayout == DataLayout::Planar)
      minPitch = frameSize.width * Bps;
    else
      minPitch = frameSize.width * Bps * channels;
  }

  if (useVirtualSize && frameSize.hasValidVirtualSize() && frameSize.rowPitches[0] >= minPitch)
    return frameSize.rowPitches[0];

  return minPitch;
}

unsigned PixelFormatRGB::getHeightForPlane(const Size &frameSize) const
{
  if (frameSize.hasValidVirtualSize() && frameSize.virtualHeights[0] >= frameSize.height)
    return frameSize.virtualHeights[0];

  return frameSize.height;
}

int PixelFormatRGB::getChannelPosition(Channel channel) const
{
  if (channel == Channel::Alpha)
  {
    switch (this->alphaMode)
    {
    case AlphaMode::First:
      return 0;
    case AlphaMode::Last:
      return 3;
    default:
      return -1;
    }
  }

  auto rgbIdx = 0;
  if (channel == Channel::Red)
  {
    if (this->channelOrder == ChannelOrder::RGB || this->channelOrder == ChannelOrder::RBG)
      rgbIdx = 0;
    if (this->channelOrder == ChannelOrder::GRB || this->channelOrder == ChannelOrder::BRG)
      rgbIdx = 1;
    if (this->channelOrder == ChannelOrder::GBR || this->channelOrder == ChannelOrder::BGR)
      rgbIdx = 2;
  }
  else if (channel == Channel::Green)
  {
    if (this->channelOrder == ChannelOrder::GRB || this->channelOrder == ChannelOrder::GBR)
      rgbIdx = 0;
    if (this->channelOrder == ChannelOrder::RGB || this->channelOrder == ChannelOrder::BGR)
      rgbIdx = 1;
    if (this->channelOrder == ChannelOrder::RBG || this->channelOrder == ChannelOrder::BRG)
      rgbIdx = 2;
  }
  else if (channel == Channel::Blue)
  {
    if (this->channelOrder == ChannelOrder::BGR || this->channelOrder == ChannelOrder::BRG)
      rgbIdx = 0;
    if (this->channelOrder == ChannelOrder::RBG || this->channelOrder == ChannelOrder::GBR)
      rgbIdx = 1;
    if (this->channelOrder == ChannelOrder::RGB || this->channelOrder == ChannelOrder::GRB)
      rgbIdx = 2;
  }

  if (this->alphaMode == AlphaMode::First)
    return rgbIdx + 1;
  return rgbIdx;
}

Channel PixelFormatRGB::getChannelAtPosition(int position) const
{
  if (this->hasAlpha())
  {
    if (position == 0 && this->alphaMode == AlphaMode::First)
      return Channel::Alpha;
    if (position == 3 && this->alphaMode == AlphaMode::Last)
      return Channel::Alpha;

    if (this->alphaMode == AlphaMode::First)
      position--;
  }

  if (position == 0)
  {
    if (this->channelOrder == ChannelOrder::RGB || this->channelOrder == ChannelOrder::RBG)
      return Channel::Red;
    if (this->channelOrder == ChannelOrder::GRB || this->channelOrder == ChannelOrder::GBR)
      return Channel::Green;
    if (this->channelOrder == ChannelOrder::BGR || this->channelOrder == ChannelOrder::BRG)
      return Channel::Blue;
  }
  else if (position == 1)
  {
    if (this->channelOrder == ChannelOrder::GRB || this->channelOrder == ChannelOrder::BRG)
      return Channel::Red;
    if (this->channelOrder == ChannelOrder::RGB || this->channelOrder == ChannelOrder::BGR)
      return Channel::Green;
    if (this->channelOrder == ChannelOrder::RBG || this->channelOrder == ChannelOrder::GBR)
      return Channel::Blue;
  }
  else if (position == 2)
  {
    if (this->channelOrder == ChannelOrder::GBR || this->channelOrder == ChannelOrder::BGR)
      return Channel::Red;
    if (this->channelOrder == ChannelOrder::RBG || this->channelOrder == ChannelOrder::BRG)
      return Channel::Green;
    if (this->channelOrder == ChannelOrder::RGB || this->channelOrder == ChannelOrder::GRB)
      return Channel::Blue;
  }

  throw std::invalid_argument("Invalid argument for channel position");
}

} // namespace video::rgb