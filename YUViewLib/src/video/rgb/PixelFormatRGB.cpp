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
                               Endianness        endianness)
    : channelOrder(channelOrder), alphaMode(alphaMode), endianness(endianness),
      dataLayout(DataLayout::Interleaved), bytePacking(true)
{
  setDiffCompType(diffType);
}

PixelFormatRGB::PixelFormatRGB(const std::string &name)
{
  if (name.empty() || name == "Unknown Pixel Format")
    return;

  std::string parseName = name;
  std::transform(parseName.begin(), parseName.end(), parseName.begin(), ::tolower);

  if (parseName == "rgb332")
  {
    this->diffCompType = DiffCompDepthType::BPP8_RGB332;
    this->bitsPerPixel = 8;
    this->bitsPerSample = 3;
    this->alphaMode = AlphaMode::None;
    this->paddingInfo = PaddingInfo::NoPadding;
    this->bytePacking = true;
    this->dataLayout = DataLayout::Interleaved;
    return;
  }

  if (parseName == "rgb565")
  {
    this->diffCompType = DiffCompDepthType::BPP16_RGB565;
    this->bitsPerPixel = 16;
    this->bitsPerSample = 6;
    this->alphaMode = AlphaMode::None;
    this->paddingInfo = PaddingInfo::NoPadding;
    this->bytePacking = true;
    this->dataLayout = DataLayout::Interleaved;
    return;
  }

  if (parseName == "rgba5551" || parseName == "argb1555")
  {
    this->diffCompType = DiffCompDepthType::BPP16_RGBA5551;
    this->bitsPerPixel = 16;
    this->bitsPerSample = 5;
    this->alphaMode = (parseName == "rgba5551") ? AlphaMode::InLsb : AlphaMode::InMsb;
    this->paddingInfo = PaddingInfo::NoPadding;
    this->bytePacking = true;
    this->dataLayout = DataLayout::Interleaved;
    return;
  }

  if (parseName == "rgbx5551" || parseName == "xrgb1555")
  {
    this->diffCompType = DiffCompDepthType::BPP16_RGBA5551;
    this->bitsPerPixel = 16;
    this->bitsPerSample = 5;
    this->alphaMode = AlphaMode::None;
    this->paddingInfo = (parseName == "rgbx5551") ? PaddingInfo::PaddingInLSB : PaddingInfo::PaddingInMSB;
    this->bytePacking = true;
    this->dataLayout = DataLayout::Interleaved;
    return;
  }

  if (parseName == "rgba1010102" || parseName == "argb2101010")
  {
    this->diffCompType = DiffCompDepthType::BPP32_RGBA1010102;
    this->bitsPerPixel = 32;
    this->bitsPerSample = 10;
    this->alphaMode = (parseName == "rgba1010102") ? AlphaMode::InLsb : AlphaMode::InMsb;
    this->paddingInfo = PaddingInfo::NoPadding;
    this->bytePacking = true;
    this->dataLayout = DataLayout::Interleaved;
    return;
  }

  if (parseName == "rgbx1010102" || parseName == "xrgb2101010")
  {
    this->diffCompType = DiffCompDepthType::BPP32_RGBA1010102;
    this->bitsPerPixel = 32;
    this->bitsPerSample = 10;
    this->alphaMode = AlphaMode::None;
    this->paddingInfo = (parseName == "rgbx1010102") ? PaddingInfo::PaddingInLSB : PaddingInfo::PaddingInMSB;
    this->bytePacking = true;
    this->dataLayout = DataLayout::Interleaved;
    return;
  }

  std::string lowerName = name;
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
  this->bytePacking = (lowerName.find("bytepacking") != std::string::npos);

  if (lowerName.find("planar") != std::string::npos)
    this->dataLayout = DataLayout::Planar;

  if (lowerName.find("be") != std::string::npos)
    this->endianness = Endianness::Big;

  char firstChar = std::tolower(parseName[0]);
  char lastChar = std::tolower(parseName.back());

  if (this->bytePacking)
  {
    if (firstChar == 'a')
    {
      this->alphaMode = AlphaMode::InMsb;
      parseName = parseName.substr(1);
    }
    else if (firstChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInMSB;
      parseName = parseName.substr(1);
    }

    lastChar = std::tolower(parseName.back());
    if (parseName.find('a') != std::string::npos && lastChar == 'a')
    {
      this->alphaMode = AlphaMode::InLsb;
      parseName.pop_back();
    }
    else if (parseName.find('x') != std::string::npos && lastChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInLSB;
      parseName.pop_back();
    }
  }
  else
  {
    if (firstChar == 'a')
    {
      this->alphaMode = AlphaMode::First;
      parseName = parseName.substr(1);
    }
    else if (firstChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInLSB;
      parseName = parseName.substr(1);
    }

    lastChar = std::tolower(parseName.back());
    if (lastChar == 'a')
    {
      this->alphaMode = AlphaMode::Last;
      parseName.pop_back();
    }
    else if (lastChar == 'x')
    {
      this->paddingInfo = PaddingInfo::PaddingInMSB;
      parseName.pop_back();
    }
  }

  std::string channelOrderStr = parseName.substr(0, 3);

  for (char &c : parseName)
    c = std::tolower(c);

  for (char c : parseName)
  {
    if (c == 'r' || c == 'g' || c == 'b')
      continue;
    if (c == 'b' || c == 'i' || c == 't' || c == ' ')
      continue;
    break;
  }

  auto order = ChannelOrderMapper.getValue(channelOrderStr);
  if (order)
    this->channelOrder = *order;

  auto bitIdx = lowerName.find("bit");
  if (bitIdx != std::string::npos)
  {
    std::string bitStr;
    for (int i = bitIdx - 2; i >= 0 && std::isdigit(lowerName[i]); i--)
      bitStr = lowerName[i] + bitStr;
    if (!bitStr.empty())
    {
      this->bitsPerSample = std::stoi(bitStr);
      if (!this->bytePacking)
        this->bitsPerPixel = this->bitsPerSample;
    }
  }
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

  // if (!this->name.empty())
  //   return this->name;

  /* diff component depth case, fix component order */
  if (this->diffCompType != DiffCompDepthType::None)
  {
    switch (this->diffCompType)
    {
    case DiffCompDepthType::BPP8_RGB332:
      return "RGB332";
    case DiffCompDepthType::BPP16_RGB565:
      return "RGB565";
    case DiffCompDepthType::BPP16_RGBA5551:
    {
      if (this->alphaMode == AlphaMode::InLsb)
        return "RGBA5551";
      if (this->alphaMode == AlphaMode::InMsb)
        return "ARGB1555";
      if (this->paddingInfo == PaddingInfo::PaddingInLSB)
        return "RGBX5551";
      if (this->paddingInfo == PaddingInfo::PaddingInMSB)
        return "XRGB1555";
    }
    case DiffCompDepthType::BPP32_RGBA1010102:
    {
      if (this->alphaMode == AlphaMode::InLsb)
        return "RGBA55511010102";
      if (this->alphaMode == AlphaMode::InMsb)
        return "ARGB2101010";
      if (this->paddingInfo == PaddingInfo::PaddingInLSB)
        return "RGBX1010102";
      if (this->paddingInfo == PaddingInfo::PaddingInMSB)
        return "XRGB2101010";
    }
    default:
      break;
    }
    return "UnknownPixelFormat4CurrentDiffCompType";
  }

  std::string name = std::string(ChannelOrderMapper.getName(this->channelOrder));

  /* bytepacking case, MSB order */
  if (this->bytePacking)
  {
    if (this->alphaMode == AlphaMode::InLsb)
      name = name + "A";
    else if (this->paddingInfo == PaddingInfo::PaddingInLSB)
      name = name + "X";
    else if (this->alphaMode == AlphaMode::InMsb)
      name = "A" + name;
    else if (this->paddingInfo == PaddingInfo::PaddingInMSB)
      name = "X" + name;

    name += " " + std::to_string(this->bitsPerSample) + "bit";
    name += " bytepacking";
  }
  /* normal case, LSB order */
  else {
    if (this->alphaMode == AlphaMode::First)
      name = "A"  + name;
    else if (this->paddingInfo == PaddingInfo::PaddingInLSB)
      name = "X" + name;
    else if (this->alphaMode == AlphaMode::Last)
      name = name + "A";
    else if (this->paddingInfo == PaddingInfo::PaddingInMSB)
      name = name + "X";

    name += " " + std::to_string(this->bitsPerSample) + "bit";
  }

  if (this->dataLayout == DataLayout::Planar)
    name += " planar";
  if (this->bitsPerPixel > 8 && this->endianness == Endianness::Big)
    name += " BE";

  // this->name = name;
  return name;
}

void PixelFormatRGB::setDiffCompType(DiffCompDepthType diffCompType)
{
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
        this->alphaMode = AlphaMode::InLsb;
    } break;
    case DiffCompDepthType::BPP32_RGBA1010102 : {
      this->bitsPerPixel = 32;
      this->bitsPerSample = 10;
      if (this->alphaMode == AlphaMode::None && this->paddingInfo == PaddingInfo::NoPadding)
        this->alphaMode = AlphaMode::First;
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
  if (!isValid() || !frameSize.isValid())
    return 0;

  const size_t numSamples = std::size_t(frameSize.height) * std::size_t(frameSize.width);
  size_t nrBytes = 0;

  if (this->diffCompType == DiffCompDepthType::None)
  {
    if (this->bytePacking)
    {
      size_t pitch = (frameSize.width * this->bitsPerSample + 7) / 8;
      nrBytes      = pitch * frameSize.height;
    }
    else
    {
      size_t Bpc = (this->bitsPerSample + 7) / 8;
      nrBytes = numSamples * Bpc * (this->hasAlpha() ? 4 : 3);
    }
  }
  else
    nrBytes = numSamples * this->bitsPerPixel / 8;

  LOGD("PixelFormatRGB::bytesPerFrame {}, size {}x{}, bytes {}",
       this->getName(), frameSize.width, frameSize.height, nrBytes);
  return nrBytes;
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