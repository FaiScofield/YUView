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

#include "ConversionRGB.h"

#include "video/LimitedRangeToFullRange.h"

namespace video::rgb
{

/* UintValueType is a pointer of uint8_t/uint16_t/uint32_t depending on bitDepth */
template <int bitDepth>
using UintValueType =
  typename std::conditional_t<bitDepth <= 8,
                              uint8_t *,
                              std::conditional_t<bitDepth <= 16, uint16_t *, uint32_t *>>;

template <int bitDepth, typename T> T swapBytesEndianess(const T &val)
{
  if (bitDepth <= 8)
    return val;
  if (bitDepth <= 16)
    return ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
  if (bitDepth <= 32)
    return ((val & 0xff) << 24) | ((val & 0xff00) << 8) | ((val & 0xff0000) >> 8) |
           ((val & 0xff000000) >> 24);
};

// Helper functions to extract RGB/RGBA values for different formats (raw bit values)
std::tuple<uint8_t, uint8_t, uint8_t> extractRGB332Raw(uint8_t value, ChannelOrder order)
{
  uint8_t r, g, b;
  switch (order)
  {
  default:
  case ChannelOrder::RGB:
    r = (value >> 5) & 0x07;
    g = (value >> 2) & 0x07;
    b = value & 0x03;
    break;
  case ChannelOrder::RBG:
    r = (value >> 5) & 0x07;
    b = (value >> 3) & 0x03;
    g = value & 0x07;
    break;
  case ChannelOrder::GRB:
    g = (value >> 5) & 0x07;
    r = (value >> 2) & 0x07;
    b = value & 0x03;
    break;
  case ChannelOrder::GBR:
    g = (value >> 5) & 0x07;
    b = (value >> 3) & 0x03;
    r = value & 0x07;
    break;
  case ChannelOrder::BRG:
    b = (value >> 6) & 0x03;
    r = (value >> 3) & 0x07;
    g = value & 0x07;
    break;
  case ChannelOrder::BGR:
    b = (value >> 6) & 0x03;
    g = (value >> 3) & 0x07;
    r = value & 0x07;
    break;
  }
  return std::make_tuple(r, g, b);
}

std::tuple<uint16_t, uint16_t, uint16_t> extractRGB565Raw(uint16_t value, ChannelOrder order)
{
  uint16_t r, g, b;
  switch (order)
  {
  default:
  case ChannelOrder::RGB:
    r = (value >> 11) & 0x1F;
    g = (value >> 5) & 0x3F;
    b = value & 0x1F;
    break;
  case ChannelOrder::RBG:
    r = (value >> 11) & 0x1F;
    b = (value >> 6) & 0x1F;
    g = value & 0x3F;
    break;
  case ChannelOrder::GRB:
    g = (value >> 10) & 0x3F;
    r = (value >> 5) & 0x1F;
    b = value & 0x1F;
    break;
  case ChannelOrder::GBR:
    g = (value >> 10) & 0x3F;
    b = (value >> 5) & 0x1F;
    r = value & 0x1F;
    break;
  case ChannelOrder::BRG:
    b = (value >> 11) & 0x1F;
    r = (value >> 6) & 0x1F;
    g = value & 0x3F;
    break;
  case ChannelOrder::BGR:
    b = (value >> 11) & 0x1F;
    g = (value >> 5) & 0x3F;
    r = value & 0x1F;
    break;
  }
  return std::make_tuple(r, g, b);
}

std::tuple<uint16_t, uint16_t, uint16_t, uint16_t> extractRGBA5551Raw(uint16_t value, ChannelOrder order, AlphaMode alphaMode, PaddingInfo paddingInfo)
{
  uint16_t r, g, b, a;
  if (AlphaMode::InMsb == alphaMode || PaddingInfo::PaddingInMSB == paddingInfo)
  {
    a = (value >> 15) & 0x01;
    switch (order)
    {
    default:
    case ChannelOrder::RGB:
      r = (value >> 10) & 0x1F;
      g = (value >> 5) & 0x1F;
      b = value & 0x1F;
      break;
    case ChannelOrder::RBG:
      r = (value >> 10) & 0x1F;
      b = (value >> 5) & 0x1F;
      g = value & 0x1F;
      break;
    case ChannelOrder::GRB:
      g = (value >> 10) & 0x1F;
      r = (value >> 5) & 0x1F;
      b = value & 0x1F;
      break;
    case ChannelOrder::GBR:
      g = (value >> 10) & 0x1F;
      b = (value >> 5) & 0x1F;
      r = value & 0x1F;
      break;
    case ChannelOrder::BRG:
      b = (value >> 10) & 0x1F;
      r = (value >> 5) & 0x1F;
      g = value & 0x1F;
      break;
    case ChannelOrder::BGR:
      b = (value >> 10) & 0x1F;
      g = (value >> 5) & 0x1F;
      r = value & 0x1F;
      break;
    }
  }
  else
  {
    a = value & 0x01;
    switch (order)
    {
    default:
    case ChannelOrder::RGB:
      r = (value >> 11) & 0x1F;
      g = (value >> 6) & 0x1F;
      b = (value >> 1) & 0x1F;
      break;
    case ChannelOrder::RBG:
      r = (value >> 11) & 0x1F;
      b = (value >> 6) & 0x1F;
      g = (value >> 1) & 0x1F;
      break;
    case ChannelOrder::GRB:
      g = (value >> 11) & 0x1F;
      r = (value >> 6) & 0x1F;
      b = (value >> 1) & 0x1F;
      break;
    case ChannelOrder::GBR:
      g = (value >> 11) & 0x1F;
      b = (value >> 6) & 0x1F;
      r = (value >> 1) & 0x1F;
      break;
    case ChannelOrder::BRG:
      b = (value >> 11) & 0x1F;
      r = (value >> 6) & 0x1F;
      g = (value >> 1) & 0x1F;
      break;
    case ChannelOrder::BGR:
      b = (value >> 11) & 0x1F;
      g = (value >> 6) & 0x1F;
      r = (value >> 1) & 0x1F;
      break;
    }
  }
  return std::make_tuple(r, g, b, a);
}

std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> extractRGBA1010102Raw(uint32_t value, ChannelOrder order, AlphaMode alphaMode, PaddingInfo paddingInfo)
{
  uint32_t r, g, b, a;
  if (AlphaMode::InMsb == alphaMode || PaddingInfo::PaddingInMSB == paddingInfo)
  {
    a = (value >> 30) & 0x03;
    switch (order)
    {
    default:
    case ChannelOrder::RGB:
      r = (value >> 20) & 0x3FF;
      g = (value >> 10) & 0x3FF;
      b = value & 0x3FF;
      break;
    case ChannelOrder::RBG:
      r = (value >> 20) & 0x3FF;
      b = (value >> 10) & 0x3FF;
      g = value & 0x3FF;
      break;
    case ChannelOrder::GRB:
      g = (value >> 20) & 0x3FF;
      r = (value >> 10) & 0x3FF;
      b = value & 0x3FF;
      break;
    case ChannelOrder::GBR:
      g = (value >> 20) & 0x3FF;
      b = (value >> 10) & 0x3FF;
      r = value & 0x3FF;
      break;
    case ChannelOrder::BRG:
      b = (value >> 20) & 0x3FF;
      r = (value >> 10) & 0x3FF;
      g = value & 0x3FF;
      break;
    case ChannelOrder::BGR:
      b = (value >> 20) & 0x3FF;
      g = (value >> 10) & 0x3FF;
      r = value & 0x3FF;
      break;
    }
  }
  else
  {
    a = value & 0x03;
    switch (order)
    {
    default:
    case ChannelOrder::RGB:
      r = (value >> 22) & 0x3FF;
      g = (value >> 12) & 0x3FF;
      b = (value >> 2) & 0x3FF;
      break;
    case ChannelOrder::RBG:
      r = (value >> 22) & 0x3FF;
      b = (value >> 12) & 0x3FF;
      g = (value >> 2) & 0x3FF;
      break;
    case ChannelOrder::GRB:
      g = (value >> 22) & 0x3FF;
      r = (value >> 12) & 0x3FF;
      b = (value >> 2) & 0x3FF;
      break;
    case ChannelOrder::GBR:
      g = (value >> 22) & 0x3FF;
      b = (value >> 12) & 0x3FF;
      r = (value >> 2) & 0x3FF;
      break;
    case ChannelOrder::BRG:
      b = (value >> 22) & 0x3FF;
      r = (value >> 12) & 0x3FF;
      g = (value >> 2) & 0x3FF;
      break;
    case ChannelOrder::BGR:
      b = (value >> 22) & 0x3FF;
      g = (value >> 12) & 0x3FF;
      r = (value >> 2) & 0x3FF;
      break;
    }
  }
  return std::make_tuple(r, g, b, a);
}

int getOffsetToFirstByteOfComponent(const Channel         channel,
                                    const PixelFormatRGB &pixelFormat,
                                    const Size            frameSize)
{
  auto offset = pixelFormat.getChannelPosition(channel);
  if (pixelFormat.getDataLayout() == DataLayout::Planar)
    offset *= frameSize.width * frameSize.height;
  return offset;
}

// Convert the input format to the output RGBA format. Apply inversion, scaling,
// limited range conversion and alpha multiplication. The input can be any supported
// format. The output is always 8 bit ARGB little endian.
template <int bitDepth>
void convertRGBToARGB(const QByteArray     &sourceBuffer,
                      const PixelFormatRGB &srcPixelFormat,
                      unsigned char        *targetBuffer,
                      const Size            frameSize,
                      const bool            componentInvert[4],
                      const int             componentScale[4],
                      const bool            limitedRange,
                      const bool            outputHasAlpha,
                      const bool            premultiplyAlpha)
{
  const auto bps = srcPixelFormat.getBitsPerSample();
  const auto Bpc = (bps + 7) / 8;
  const auto Bpp = Bpc * (srcPixelFormat.hasAlpha() ? 4 : 3);
  const auto alphaMode = srcPixelFormat.getAlphaMode();
  const auto channelOrder = srcPixelFormat.getChannelOrder();
  const auto paddingInfo = srcPixelFormat.getPaddingInfo();
  const auto hasAlpha = srcPixelFormat.hasAlpha();
  const auto isBigEndian = bitDepth > 8 && srcPixelFormat.getEndianess() == Endianness::Big;
  const auto maxValue = (1 << bps) - 1;
  assert(srcPixelFormat.getBitsPerPixel() == Bpp * 8);

  /* r = (val & channelMask) >> rightShift */
  int rightShift = 0;
  int channelMask = (1 << bps) - 1; // 0x3FF for 10bit data
  if (bps % 8 > 0) {
    if (paddingInfo == PaddingInfo::PaddingInLSB) {
      rightShift = Bpc * 8 - bps;
      channelMask = channelMask << rightShift;
    }
    else { // PaddingInMSB (see NoPadding as PaddingInMSB)
      rightShift = 0;
    }
  }

  const auto offsetToNextValue =
    srcPixelFormat.getDataLayout() == DataLayout::Planar ? 1 : srcPixelFormat.nrChannels();

  using InValueType   = UintValueType<bitDepth>;
  const auto setAlpha = outputHasAlpha && srcPixelFormat.hasAlpha();

  const auto rawData = (InValueType)sourceBuffer.data();

  auto srcR = rawData + getOffsetToFirstByteOfComponent(Channel::Red, srcPixelFormat, frameSize);
  auto srcG = rawData + getOffsetToFirstByteOfComponent(Channel::Green, srcPixelFormat, frameSize);
  auto srcB = rawData + getOffsetToFirstByteOfComponent(Channel::Blue, srcPixelFormat, frameSize);
  auto srcA = setAlpha ? rawData + getOffsetToFirstByteOfComponent(Channel::Alpha, srcPixelFormat, frameSize) : nullptr;

  auto convertValue =
    [=](const InValueType sourceData, const int scale, const bool invert)
  {
    auto value = static_cast<int64_t>(sourceData[0]);
    if (isBigEndian)
      value = swapBytesEndianess<bitDepth>(value);
    value = ((value & channelMask) * scale) >> rightShift;
    value = functions::clip(value, 0, maxValue);
    if (invert)
      value = maxValue - value;
    return value;
  };

  for (unsigned i = 0; i < frameSize.width * frameSize.height; i++)
  {
    auto valR = convertValue(srcR, componentScale[0], componentInvert[0]);
    auto valG = convertValue(srcG, componentScale[1], componentInvert[1]);
    auto valB = convertValue(srcB, componentScale[2], componentInvert[2]);
    if (bps != 8)
    {
      valR = (valR * 255 + (maxValue >> 1)) / maxValue;
      valG = (valG * 255 + (maxValue >> 1)) / maxValue;
      valB = (valB * 255 + (maxValue >> 1)) / maxValue;
    }

    if (limitedRange)
    {
      valR = LimitedRangeToFullRange.at(valR);
      valG = LimitedRangeToFullRange.at(valG);
      valB = LimitedRangeToFullRange.at(valB);
      // No limited range for alpha
    }

    int valA = maxValue;
    if (setAlpha)
    {
      valA = convertValue(srcA, componentScale[3], componentInvert[3]);
      srcA += offsetToNextValue;

      if (premultiplyAlpha)
      {
        valR = (valR * valA + (maxValue >> 1)) / maxValue;
        valG = (valG * valA + (maxValue >> 1)) / maxValue;
        valB = (valB * valA + (maxValue >> 1)) / maxValue;
      }
    }

    srcR += offsetToNextValue;
    srcG += offsetToNextValue;
    srcB += offsetToNextValue;

    targetBuffer[0] = valB;
    targetBuffer[1] = valG;
    targetBuffer[2] = valR;
    targetBuffer[3] = valA;

    targetBuffer += 4;
  }
}

// Convert one single plane of the input format to RGBA. This is used to visualize the individual
// components.
template <int bitDepth>
void convertRGBPlaneToARGB(const QByteArray     &sourceBuffer,
                           const PixelFormatRGB &srcPixelFormat,
                           unsigned char        *targetBuffer,
                           const Size            frameSize,
                           const Channel         displayChannel,
                           const int             scale,
                           const bool            invert,
                           const bool            limitedRange)
{
  const auto shiftTo8Bit = srcPixelFormat.getBitsPerSample() - 8;
  const auto offsetToNextValue =
    srcPixelFormat.getDataLayout() == DataLayout::Planar ? 1 : srcPixelFormat.nrChannels();

  using InValueType = UintValueType<bitDepth>;

  auto       src                    = (InValueType)sourceBuffer.data();
  const auto displayComponentOffset = srcPixelFormat.getChannelPosition(displayChannel);
  if (srcPixelFormat.getDataLayout() == DataLayout::Planar)
    src += displayComponentOffset * frameSize.width * frameSize.height;
  else
    src += displayComponentOffset;

  for (size_t i = 0; i < frameSize.width * frameSize.height; i++)
  {
    auto val = static_cast<int64_t>(src[0]);
    if (bitDepth > 8 && srcPixelFormat.getEndianess() == Endianness::Big)
      val = swapBytesEndianess<bitDepth>(val);
    val = (val * scale) >> shiftTo8Bit;
    val = functions::clip(val, 0, 255);
    if (invert)
      val = 255 - val;
    if (limitedRange)
      val = LimitedRangeToFullRange.at(val);

    targetBuffer[0] = val;
    targetBuffer[1] = val;
    targetBuffer[2] = val;
    targetBuffer[3] = 255;

    src += offsetToNextValue;
    targetBuffer += 4;
  }
}

template <int bitDepth>
rgba_t getPixelValue(const QByteArray     &sourceBuffer,
                     const PixelFormatRGB &srcPixelFormat,
                     const Size            frameSize,
                     const QPoint         &pixelPos)
{
  const auto offsetToNextValue =
    srcPixelFormat.getDataLayout() == DataLayout::Planar ? 1 : srcPixelFormat.nrChannels();
  const auto offsetPixelPos = frameSize.width * pixelPos.y() + pixelPos.x();
  const auto paddingInfo = srcPixelFormat.getPaddingInfo();
  const int bps = srcPixelFormat.getBitsPerSample();
  const int Bpc = (bps + 7) / 8;

  /* r = (val & channelMask) >> rightShift */
  int rightShift = 0;
  int channelMask = (1 << bps) - 1; // 0x3FF for 10bit data
  if (bps % 8 > 0) {
    if (paddingInfo == PaddingInfo::PaddingInLSB) {
      rightShift = Bpc * 8 - bps;
      channelMask = channelMask << rightShift;
    }
    else { // PaddingInMSB (see NoPadding as PaddingInMSB)
      rightShift = 0;
    }
  }

  using InValueType = UintValueType<bitDepth>;

  const auto rawData  = (InValueType)sourceBuffer.data();
  auto       srcPixel = rawData + offsetPixelPos * offsetToNextValue;

  rgba_t value{};
  for (auto channel : {Channel::Red, Channel::Green, Channel::Blue, Channel::Alpha})
  {
    if (channel == Channel::Alpha && !srcPixelFormat.hasAlpha())
      continue;

    const auto offset = getOffsetToFirstByteOfComponent(channel, srcPixelFormat, frameSize);

    auto src = srcPixel + offset;
    auto val = (unsigned)src[0];
    if (bitDepth > 8 && srcPixelFormat.getEndianess() == Endianness::Big)
      val = swapBytesEndianess<bitDepth>(val);

    val = (val & channelMask) >> rightShift;
    value[channel] = val;
  }

  return value;
}


void convertRGB332ToARGB(const QByteArray     &sourceBuffer,
                         const PixelFormatRGB &srcPixelFormat,
                         unsigned char        *targetBuffer,
                         const Size            frameSize,
                         const bool            componentInvert[4],
                         const int             componentScale[4],
                         const bool            limitedRange,
                         const bool            outputHasAlpha,
                         const bool            premultiplyAlpha)
{
  uint8_t   *rawData   = (uint8_t *)sourceBuffer.data();
  const auto numPixels = frameSize.width * frameSize.height;
  const auto order = srcPixelFormat.getChannelOrder();

  for (unsigned i = 0; i < numPixels; i++)
  {
    uint8_t value = rawData[i];
    auto [r_raw, g_raw, b_raw] = extractRGB332Raw(value, order);

    uint16_t r = (r_raw * 255 + 3) / 7;
    uint16_t g = (g_raw * 255 + 3) / 7;
    uint16_t b = (b_raw * 255 + 1) / 3;

    r = functions::clip(r * componentScale[0], 0, 255);
    g = functions::clip(g * componentScale[1], 0, 255);
    b = functions::clip(b * componentScale[2], 0, 255);

    if (componentInvert[0])
      r = 255 - r;
    if (componentInvert[1])
      g = 255 - g;
    if (componentInvert[2])
      b = 255 - b;

    targetBuffer[0] = b;
    targetBuffer[1] = g;
    targetBuffer[2] = r;
    targetBuffer[3] = 255;
    targetBuffer += 4;
  }
}

void convertRGB565ToARGB(const QByteArray     &sourceBuffer,
                         const PixelFormatRGB &srcPixelFormat,
                         unsigned char        *targetBuffer,
                         const Size            frameSize,
                         const bool            componentInvert[4],
                         const int             componentScale[4],
                         const bool            limitedRange,
                         const bool            outputHasAlpha,
                         const bool            premultiplyAlpha)
{
  uint16_t  *rawData     = (uint16_t *)sourceBuffer.data();
  const auto isBigEndian = srcPixelFormat.getEndianess() == Endianness::Big;
  const auto numPixels   = frameSize.width * frameSize.height;
  const auto order = srcPixelFormat.getChannelOrder();

  for (unsigned i = 0; i < numPixels; i++)
  {
    uint16_t value = rawData[i];
    if (isBigEndian)
        value = swapBytesEndianess<16>(value);

    auto [r_raw, g_raw, b_raw] = extractRGB565Raw(value, order);

    uint16_t r = (r_raw * 255 + 15) / 31;
    uint16_t g = (g_raw * 255 + 31) / 63;
    uint16_t b = (b_raw * 255 + 15) / 31;

    r = functions::clip(r * componentScale[0], 0, 255);
    g = functions::clip(g * componentScale[1], 0, 255);
    b = functions::clip(b * componentScale[2], 0, 255);

    if (componentInvert[0])
      r = 255 - r;
    if (componentInvert[1])
      g = 255 - g;
    if (componentInvert[2])
      b = 255 - b;

    targetBuffer[0] = b;
    targetBuffer[1] = g;
    targetBuffer[2] = r;
    targetBuffer[3] = 255;
    targetBuffer += 4;
  }
}

void convertRGBA5551ToARGB(const QByteArray     &sourceBuffer,
                           const PixelFormatRGB &srcPixelFormat,
                           unsigned char        *targetBuffer,
                           const Size            frameSize,
                           const bool            componentInvert[4],
                           const int             componentScale[4],
                           const bool            limitedRange,
                           const bool            outputHasAlpha,
                           const bool            premultiplyAlpha)
{
  uint16_t  *rawData     = (uint16_t *)sourceBuffer.data();
  const auto setAlpha    = outputHasAlpha && srcPixelFormat.hasAlpha();
  const auto isBigEndian = srcPixelFormat.getEndianess() == Endianness::Big;
  const auto numPixels   = frameSize.width * frameSize.height;
  const auto alphaMode = srcPixelFormat.getAlphaMode();
  const auto paddingInfo = srcPixelFormat.getPaddingInfo();
  const auto order = srcPixelFormat.getChannelOrder();

  for (unsigned i = 0; i < numPixels; i++)
  {
    uint16_t value = rawData[i];
    if (isBigEndian)
        value = swapBytesEndianess<16>(value);

    auto [r_raw, g_raw, b_raw, a_raw] = extractRGBA5551Raw(value, order, alphaMode, paddingInfo);

    uint16_t r = (r_raw * 255 + 15) / 31;
    uint16_t g = (g_raw * 255 + 15) / 31;
    uint16_t b = (b_raw * 255 + 15) / 31;
    uint16_t a = a_raw ? 255 : 0;

    r = functions::clip(r * componentScale[0], 0, 255);
    g = functions::clip(g * componentScale[1], 0, 255);
    b = functions::clip(b * componentScale[2], 0, 255);
    a = functions::clip(a * componentScale[3], 0, 255);

    if (componentInvert[0])
      r = 255 - r;
    if (componentInvert[1])
      g = 255 - g;
    if (componentInvert[2])
      b = 255 - b;
    if (componentInvert[3])
      a = 255 - a;

    if (premultiplyAlpha && a != 255)
    {
      r = (r * a + 127) / 255;
      g = (g * a + 127) / 255;
      b = (b * a + 127) / 255;
    }

    targetBuffer[0] = b;
    targetBuffer[1] = g;
    targetBuffer[2] = r;
    targetBuffer[3] = setAlpha ? a : 255;

    targetBuffer += 4;
  }
}

void convertRGBA1010102ToARGB(const QByteArray     &sourceBuffer,
                              const PixelFormatRGB &srcPixelFormat,
                              unsigned char        *targetBuffer,
                              const Size            frameSize,
                              const bool            componentInvert[4],
                              const int             componentScale[4],
                              const bool            limitedRange,
                              const bool            outputHasAlpha,
                              const bool            premultiplyAlpha)
{
  uint32_t  *rawData     = (uint32_t *)sourceBuffer.data();
  const auto setAlpha    = outputHasAlpha && srcPixelFormat.hasAlpha();
  const auto isBigEndian = srcPixelFormat.getEndianess() == Endianness::Big;
  const auto numPixels   = frameSize.width * frameSize.height;
  const auto alphaMode = srcPixelFormat.getAlphaMode();
  const auto paddingInfo = srcPixelFormat.getPaddingInfo();
  const auto order = srcPixelFormat.getChannelOrder();

  for (unsigned i = 0; i < numPixels; i++)
  {
    uint32_t value = rawData[i];
    if (isBigEndian)
        value = swapBytesEndianess<32>(value);

    auto [r_raw, g_raw, b_raw, a_raw] = extractRGBA1010102Raw(value, order, alphaMode, paddingInfo);

    uint32_t r = (r_raw * 255 + 511) / 1023;
    uint32_t g = (g_raw * 255 + 511) / 1023;
    uint32_t b = (b_raw * 255 + 511) / 1023;
    uint32_t a = (a_raw * 255 + 1) / 3;

    r = functions::clip(r * componentScale[0], 0, 255);
    g = functions::clip(g * componentScale[1], 0, 255);
    b = functions::clip(b * componentScale[2], 0, 255);
    a = functions::clip(a * componentScale[3], 0, 255);

    if (componentInvert[0])
      r = 255 - r;
    if (componentInvert[1])
      g = 255 - g;
    if (componentInvert[2])
      b = 255 - b;
    if (componentInvert[3])
      a = 255 - a;

    if (premultiplyAlpha && a != 255)
    {
      r = (r * a + 127) / 255;
      g = (g * a + 127) / 255;
      b = (b * a + 127) / 255;
    }

    targetBuffer[0] = b;
    targetBuffer[1] = g;
    targetBuffer[2] = r;
    targetBuffer[3] = setAlpha ? a : 255;

    targetBuffer += 4;
  }
}

void convertBitPackedToARGB(const QByteArray     &sourceBuffer,
                            const PixelFormatRGB &srcPixelFormat,
                            unsigned char        *targetBuffer,
                            const Size            frameSize,
                            const bool            componentInvert[4],
                            const int             componentScale[4],
                            const bool            limitedRange,
                            const bool            outputHasAlpha,
                            const bool            premultiplyAlpha)
{
  const uint8_t * rawData = (uint8_t *)sourceBuffer.data();
  const auto bps = srcPixelFormat.getBitsPerSample();
  const auto bpp = bps * (srcPixelFormat.hasAlpha() ? 4 : 3);
  const auto alphaMode = srcPixelFormat.getAlphaMode();
  const auto channelOrder = srcPixelFormat.getChannelOrder();
  const auto isBigEndian = srcPixelFormat.getEndianess() == Endianness::Big;
  const auto numPixels = frameSize.width * frameSize.height;
  const auto hasAlpha = srcPixelFormat.hasAlpha();
  const auto bytesPerPixel = (bpp + 7) / 8;
  const auto maxValue = (1 << bps) - 1;

  const auto isPlanar = srcPixelFormat.getDataLayout() == DataLayout::Planar;

  const uint8_t *planeR = nullptr;
  const uint8_t *planeG = nullptr;
  const uint8_t *planeB = nullptr;
  const uint8_t *planeA = nullptr;

  if (isPlanar)
  {
    int channelOffsetR = getOffsetToFirstByteOfComponent(Channel::Red, srcPixelFormat, frameSize);
    int channelOffsetG = getOffsetToFirstByteOfComponent(Channel::Green, srcPixelFormat, frameSize);
    int channelOffsetB = getOffsetToFirstByteOfComponent(Channel::Blue, srcPixelFormat, frameSize);
    int channelOffsetA = getOffsetToFirstByteOfComponent(Channel::Alpha, srcPixelFormat, frameSize);
    // bytepacking offset
    channelOffsetR = channelOffsetR * bps / 8;
    channelOffsetG = channelOffsetG * bps / 8;
    channelOffsetB = channelOffsetB * bps / 8;
    channelOffsetA = channelOffsetA * bps / 8;

    planeR = rawData + channelOffsetR;
    planeG = rawData + channelOffsetG;
    planeB = rawData + channelOffsetB;
    planeA = hasAlpha ? rawData + channelOffsetA : nullptr;
  }

  auto getChannelBitPos = [&](Channel ch) -> int
  {
    if (ch == Channel::Alpha)
    {
      if (hasAlpha)
      {
        if (alphaMode == AlphaMode::First || alphaMode == AlphaMode::InMsb)
          return bpp - bps;
        else
          return 0;
      }
      return 0;
    }

    int rgbIdx = 0;
    switch (channelOrder)
    {
    case ChannelOrder::RGB: rgbIdx = (ch == Channel::Red) ? 0 : (ch == Channel::Green) ? 1 : 2; break;
    case ChannelOrder::RBG: rgbIdx = (ch == Channel::Red) ? 0 : (ch == Channel::Green) ? 2 : 1; break;
    case ChannelOrder::GRB: rgbIdx = (ch == Channel::Red) ? 1 : (ch == Channel::Green) ? 0 : 2; break;
    case ChannelOrder::GBR: rgbIdx = (ch == Channel::Red) ? 2 : (ch == Channel::Green) ? 0 : 1; break;
    case ChannelOrder::BRG: rgbIdx = (ch == Channel::Red) ? 2 : (ch == Channel::Green) ? 1 : 0; break;
    case ChannelOrder::BGR: rgbIdx = (ch == Channel::Red) ? 2 : (ch == Channel::Green) ? 1 : 0; break;
    }

    if (hasAlpha)
    {
      if (alphaMode == AlphaMode::First || alphaMode == AlphaMode::InMsb)
        return bpp - (rgbIdx + 1) * bps;
      else
        return rgbIdx * bps;
    }

    return (3 - 1 - rgbIdx) * bps;
  };

  const int rBitPos = getChannelBitPos(Channel::Red);
  const int gBitPos = getChannelBitPos(Channel::Green);
  const int bBitPos = getChannelBitPos(Channel::Blue);
  const int aBitPos = hasAlpha ? getChannelBitPos(Channel::Alpha) : 0;

  for (unsigned i = 0; i < numPixels; i++)
  {
    uint64_t valueR = 0, valueG = 0, valueB = 0, valueA = 0;

    if (isPlanar)
    {
      const int bitStart  = i * bps;
      const int byteStart = bitStart / 8;
      const int bitOffset = bitStart % 8;
      if (bps <= 8)
      {
        valueR = planeR[byteStart];
        valueG = planeG[byteStart];
        valueB = planeB[byteStart];
        valueA = planeA ? planeA[byteStart] : maxValue;
        if (bitOffset + bps > 8)
        {
          valueR |= planeR[byteStart + 1] << 8;
          valueG |= planeG[byteStart + 1] << 8;
          valueB |= planeB[byteStart + 1] << 8;
          valueA |= planeA ? planeA[byteStart + 1] << 8 : 0;
        }
      }
      else if (bps <= 16)
      {
        valueR = planeR[byteStart] | (planeR[byteStart + 1] << 8);
        valueG = planeG[byteStart] | (planeG[byteStart + 1] << 8);
        valueB = planeB[byteStart] | (planeB[byteStart + 1] << 8);
        valueA = planeA ? (planeA[byteStart] | (planeA[byteStart + 1] << 8)) : maxValue;
        if (bitOffset + bps > 16)
        {
          valueR |= planeR[byteStart + 2] << 16;
          valueG |= planeG[byteStart + 2] << 16;
          valueB |= planeB[byteStart + 2] << 16;
          valueA |= planeA ? planeA[byteStart + 2] << 16 : 0;
        }
      }
      else if (bps <= 24)
      {
        valueR = planeR[byteStart] | (planeR[byteStart + 1] << 8) | (planeR[byteStart + 2] << 16);
        valueG = planeG[byteStart] | (planeG[byteStart + 1] << 8) | (planeG[byteStart + 2] << 16);
        valueB = planeB[byteStart] | (planeB[byteStart + 1] << 8) | (planeB[byteStart + 2] << 16);
        valueA = planeA
            ? (planeA[byteStart]) | (planeA[byteStart + 1] << 8) | (planeA[byteStart + 2] << 16)
            : maxValue;
        if (bitOffset + bps > 24)
        {
          valueR |= planeR[byteStart + 3] << 24;
          valueG |= planeG[byteStart + 3] << 24;
          valueB |= planeB[byteStart + 3] << 24;
          valueA |= planeA ? planeA[byteStart + 3] << 24 : 0;
        }
      }
      else
      {
        valueR = planeR[byteStart] | (planeR[byteStart + 1] << 8) |
                            (planeR[byteStart + 2] << 16) | (planeR[byteStart + 3] << 24);
        valueG = planeG[byteStart] | (planeG[byteStart + 1] << 8) |
                            (planeG[byteStart + 2] << 16) | (planeG[byteStart + 3] << 24);
        valueB = planeB[byteStart] | (planeB[byteStart + 1] << 8) |
                            (planeB[byteStart + 2] << 16) | (planeB[byteStart + 3] << 24);
        valueA = planeA ? (planeA[byteStart] | (planeA[byteStart + 1] << 8) |
                                     (planeA[byteStart + 2] << 16) | (planeA[byteStart + 3] << 24))
                        : maxValue;
        if (bitOffset + bps > 32)
        {
          valueR |= static_cast<uint64_t>(planeR[byteStart + 4]) << 32;
          valueG |= static_cast<uint64_t>(planeG[byteStart + 4]) << 32;
          valueB |= static_cast<uint64_t>(planeB[byteStart + 4]) << 32;
          valueA |= planeA ? static_cast<uint64_t>(planeA[byteStart + 4]) << 32 : 0;
        }
      }
    }
    else
    {
      const int bitStart  = i * bpp;
      const int byteStart = bitStart / 8;
      const int bitOffset = bitStart % 8;
      uint64_t value = 0;

      if (bpp <= 8)
      {
        value = rawData[byteStart];
        if (bitOffset + bps > 8)
          value |= rawData[byteStart + 1] << 8;
      }
      else if (bpp <= 16)
      {
        value = rawData[byteStart] | (rawData[byteStart + 1] << 8);
        if (bitOffset + bps > 16)
          value |= rawData[byteStart + 2] << 16;
      }
      else if (bpp <= 24)
      {
        value = rawData[byteStart] | (rawData[byteStart + 1] << 8) | (rawData[byteStart + 2] << 16);
        if (bitOffset + bps > 24)
          value |= rawData[byteStart + 3] << 24;
      }
      else
      {
        value = rawData[byteStart] | (rawData[byteStart + 1] << 8) |
                (rawData[byteStart + 2] << 16) | (rawData[byteStart + 3] << 24);
        if (bitOffset + bps > 32)
          value |= static_cast<uint64_t>(rawData[byteStart + 4]) << 32;
      }
      valueR = (value >> rBitPos) & maxValue;
      valueG = (value >> gBitPos) & maxValue;
      valueB = (value >> bBitPos) & maxValue;
      valueA = hasAlpha ? ((value >> aBitPos) & maxValue) : maxValue;
    }

    int r = valueR & maxValue;
    int g = valueG & maxValue;
    int b = valueB & maxValue;
    int a = valueA & maxValue;

    if (bps != 8)
    {
      r = (r * 255 + maxValue / 2) / maxValue;
      g = (g * 255 + maxValue / 2) / maxValue;
      b = (b * 255 + maxValue / 2) / maxValue;
      a = (a * 255 + maxValue / 2) / maxValue;
    }

    r = functions::clip(r * componentScale[0], 0, 255);
    g = functions::clip(g * componentScale[1], 0, 255);
    b = functions::clip(b * componentScale[2], 0, 255);
    a = functions::clip(a * componentScale[3], 0, 255);

    if (componentInvert[0])
      r = 255 - r;
    if (componentInvert[1])
      g = 255 - g;
    if (componentInvert[2])
      b = 255 - b;
    if (componentInvert[3])
      a = 255 - a;

    if (premultiplyAlpha && a != 255)
    {
      r = (r * a + 127) / 255;
      g = (g * a + 127) / 255;
      b = (b * a + 127) / 255;
    }

    targetBuffer[0] = b;
    targetBuffer[1] = g;
    targetBuffer[2] = r;
    targetBuffer[3] = a;

    targetBuffer += 4;
  }
}

void convertInputRGBToARGB(const QByteArray     &sourceBuffer,
                           const PixelFormatRGB &srcPixelFormat,
                           unsigned char        *targetBuffer,
                           const Size            frameSize,
                           const bool            componentInvert[4], /* {f,f,f,f} by default */
                           const int             componentScale[4], /* {1,1,1,1} by default */
                           const bool            limitedRange,
                           const bool            outputHasAlpha,
                           const bool            premultiplyAlpha)
{
  if (srcPixelFormat.isDiffCompDepth())
  {
    switch (srcPixelFormat.getDiffCompType())
    {
    case DiffCompDepthType::BPP8_RGB332:
      convertRGB332ToARGB(sourceBuffer,
                          srcPixelFormat,
                          targetBuffer,
                          frameSize,
                          componentInvert,
                          componentScale,
                          limitedRange,
                          outputHasAlpha,
                          premultiplyAlpha);
      break;
    case DiffCompDepthType::BPP16_RGB565:
      convertRGB565ToARGB(sourceBuffer,
                          srcPixelFormat,
                          targetBuffer,
                          frameSize,
                          componentInvert,
                          componentScale,
                          limitedRange,
                          outputHasAlpha,
                          premultiplyAlpha);
      break;
    case DiffCompDepthType::BPP16_RGBA5551:
      convertRGBA5551ToARGB(sourceBuffer,
                            srcPixelFormat,
                            targetBuffer,
                            frameSize,
                            componentInvert,
                            componentScale,
                            limitedRange,
                            outputHasAlpha,
                            premultiplyAlpha);
      break;
    case DiffCompDepthType::BPP32_RGBA1010102:
      convertRGBA1010102ToARGB(sourceBuffer,
                               srcPixelFormat,
                               targetBuffer,
                               frameSize,
                               componentInvert,
                               componentScale,
                               limitedRange,
                               outputHasAlpha,
                               premultiplyAlpha);
      break;
    default:
      throw std::invalid_argument("Unsupported DiffCompDepthType for conversion");
    }
  }
  else if (srcPixelFormat.isBytePacking())
  {
    convertBitPackedToARGB(sourceBuffer,
                           srcPixelFormat,
                           targetBuffer,
                           frameSize,
                           componentInvert,
                           componentScale,
                           limitedRange,
                           outputHasAlpha,
                           premultiplyAlpha);
  }
  else
  {
    const auto bps = srcPixelFormat.getBitsPerSample();
    if (bps < 1 || bps > 32)
      throw std::invalid_argument("Invalid bit depth in pixel format for conversion");

    if (bps <= 8)
      convertRGBToARGB<8>(sourceBuffer,
                          srcPixelFormat,
                          targetBuffer,
                          frameSize,
                          componentInvert,
                          componentScale,
                          limitedRange,
                          outputHasAlpha,
                          premultiplyAlpha);
    else if (bps <= 16)
      convertRGBToARGB<16>(sourceBuffer,
                           srcPixelFormat,
                           targetBuffer,
                           frameSize,
                           componentInvert,
                           componentScale,
                           limitedRange,
                           outputHasAlpha,
                           premultiplyAlpha);
    else
      convertRGBToARGB<32>(sourceBuffer,
                           srcPixelFormat,
                           targetBuffer,
                           frameSize,
                           componentInvert,
                           componentScale,
                           limitedRange,
                           outputHasAlpha,
                           premultiplyAlpha);
  }
}

void convertSinglePlaneOfRGBToGreyscaleARGB(const QByteArray     &sourceBuffer,
                                            const PixelFormatRGB &srcPixelFormat,
                                            unsigned char        *targetBuffer,
                                            const Size            frameSize,
                                            const Channel         displayChannel,
                                            const int             scale,
                                            const bool            invert,
                                            const bool            limitedRange)
{
  const auto bps = srcPixelFormat.getBitsPerSample();
  if (bps < 1 || bps > 32)
    throw std::invalid_argument("Invalid bit depth in pixel format for conversion");

  if (bps <= 8)
    convertRGBPlaneToARGB<8>(sourceBuffer,
                             srcPixelFormat,
                             targetBuffer,
                             frameSize,
                             displayChannel,
                             scale,
                             invert,
                             limitedRange);
  else if (bps <= 16)
    convertRGBPlaneToARGB<16>(sourceBuffer,
                              srcPixelFormat,
                              targetBuffer,
                              frameSize,
                              displayChannel,
                              scale,
                              invert,
                              limitedRange);
  else
    convertRGBPlaneToARGB<32>(sourceBuffer,
                              srcPixelFormat,
                              targetBuffer,
                              frameSize,
                              displayChannel,
                              scale,
                              invert,
                              limitedRange);
}

rgba_t getPixelValue4DiffType(const QByteArray     &sourceBuffer,
                               const PixelFormatRGB &srcPixelFormat,
                               const QPoint         &pixelPos,
                               const Size            frameSize)
{
  const auto diffCompType = srcPixelFormat.getDiffCompType();
  const auto alphaMode = srcPixelFormat.getAlphaMode();
  const auto paddingMode = srcPixelFormat.getPaddingInfo();
  const auto channelOrder = srcPixelFormat.getChannelOrder();
  const auto offsetPixelPos = frameSize.width * pixelPos.y() + pixelPos.x();

  if (diffCompType == DiffCompDepthType::BPP8_RGB332)
  {
    const uint8_t *rawData = (uint8_t *)sourceBuffer.data();
    uint8_t value = rawData[offsetPixelPos];
    auto [r, g, b] = extractRGB332Raw(value, channelOrder);
    rgba_t result{r, g, b, 0};
    return result;
  }
  else if (diffCompType == DiffCompDepthType::BPP16_RGB565)
  {
    const uint16_t *rawData = (uint16_t *)sourceBuffer.data();
    uint16_t value = rawData[offsetPixelPos];
    auto [r, g, b] = extractRGB565Raw(value, channelOrder);
    rgba_t result{r, g, b, 0};
    return result;
  }
  else if (diffCompType == DiffCompDepthType::BPP16_RGBA5551)
  {
    const uint16_t *rawData = (uint16_t *)sourceBuffer.data();
    uint16_t value = rawData[offsetPixelPos];
    auto [r, g, b, a] = extractRGBA5551Raw(value, channelOrder, alphaMode, paddingMode);
    rgba_t result{r, g, b, a};
    return result;
  }
  else if (diffCompType == DiffCompDepthType::BPP32_RGBA1010102)
  {
    const uint32_t *rawData = (uint32_t *)sourceBuffer.data();
    uint32_t value = rawData[offsetPixelPos];
    auto [r, g, b, a] = extractRGBA1010102Raw(value, channelOrder, alphaMode, paddingMode);
    rgba_t result{r, g, b, a};
    return result;
  }
  throw std::invalid_argument("Unsupported DiffCompDepthType for getPixelValue");
}

rgba_t getPixelValueFromBuffer(const QByteArray     &sourceBuffer,
                               const PixelFormatRGB &srcPixelFormat,
                               const Size            frameSize,
                               const QPoint         &pixelPos)
{
  if (srcPixelFormat.isDiffCompDepth())
  {
    return getPixelValue4DiffType(sourceBuffer, srcPixelFormat, pixelPos, frameSize);
  }
  else
  {
    const auto bpc = srcPixelFormat.getBitsPerSample();
    if (bpc < 1 || bpc > 32)
      throw std::invalid_argument("Invalid bit depth in pixel format for conversion");

    if (bpc <= 8)
      return getPixelValue<8>(sourceBuffer, srcPixelFormat, frameSize, pixelPos);
    else if (bpc <= 16)
      return getPixelValue<16>(sourceBuffer, srcPixelFormat, frameSize, pixelPos);
    else
      return getPixelValue<32>(sourceBuffer, srcPixelFormat, frameSize, pixelPos);
  }
}

} // namespace video::rgb
