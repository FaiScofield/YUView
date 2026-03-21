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

PixelFormatRGB::PixelFormatRGB(unsigned      bitsPerPixel,
                               DataLayout    dataLayout,
                               ChannelOrder  channelOrder,
                               AlphaMode     alphaMode,
                               Endianness    endianness,
                               BitPackedType bitPackedType,
                               PaddingInfo   paddingInfo)
    : bitsPerPixel(bitsPerPixel), dataLayout(dataLayout), channelOrder(channelOrder),
      alphaMode(alphaMode), endianness(endianness), bitPackedType(bitPackedType),
      paddingInfo(paddingInfo)
{
}

PixelFormatRGB::PixelFormatRGB(const std::string &name)
{
  if (name != "Unknown Pixel Format")
  {
    auto channelOrderString = name.substr(0, 3);
    if (name[0] == 'a' || name[0] == 'A')
    {
      this->alphaMode    = AlphaMode::First;
      channelOrderString = name.substr(1, 3);
    }
    else if (name[3] == 'a' || name[3] == 'A')
    {
      this->alphaMode    = AlphaMode::Last;
      channelOrderString = name.substr(0, 3);
    }
    auto order = ChannelOrderMapper.getValue(channelOrderString);
    if (order)
      this->channelOrder = *order;

    auto bitIdx = name.find("bit");
    if (bitIdx != std::string::npos)
      this->bitsPerPixel = std::stoi(name.substr(bitIdx - 2, 2), nullptr);
    if (name.find("planar") != std::string::npos)
      this->dataLayout = DataLayout::Planar;
    if (this->bitsPerPixel > 8 && name.find("BE") != std::string::npos)
      this->endianness = Endianness::Big;
  }
}

bool PixelFormatRGB::isValid() const
{
  bool depthValid = this->bitsPerPixel >= 8 && this->bitsPerPixel <= 32 && this->bitsPerPixel % 8 == 0;
  bool alphaValid = true;
  bool paddingValid = true;
  switch (this->bitPackedType)
  {
  case BitPackedType::BPP8_RGB332:
    depthValid &= this->bitsPerPixel == 8;
    alphaValid &= this->alphaMode == AlphaMode::None;
    paddingValid &= this->paddingInfo == PaddingInfo::NoPadding;
    break;
  case BitPackedType::BPP16_RGB565:
    depthValid &= this->bitsPerPixel == 16;
    alphaValid &= this->alphaMode == AlphaMode::None;
    paddingValid &= this->paddingInfo == PaddingInfo::NoPadding;
    break;
  case BitPackedType::BPP16_RGBX5551:
  case BitPackedType::BPP16_RGBX4444:
    depthValid &= this->bitsPerPixel == 16;
    alphaValid &= !(this->alphaMode == AlphaMode::None && this->paddingInfo == PaddingInfo::NoPadding);
    break;
  case BitPackedType::BPP32_RGBX8888:
  case BitPackedType::BPP32_RGBX1010102:
    depthValid &= this->bitsPerPixel == 32;
    alphaValid &= !(this->alphaMode == AlphaMode::None && this->paddingInfo == PaddingInfo::NoPadding);
    break;
  case BitPackedType::Unpacked:
  default:
    break;
  }

  return depthValid && alphaValid && paddingValid;
}

unsigned PixelFormatRGB::nrChannels() const
{
  return this->alphaMode != AlphaMode::None ? 4 : 3;
}

bool PixelFormatRGB::hasAlpha() const
{
  return this->alphaMode != AlphaMode::None;
}

std::string PixelFormatRGB::getName() const
{
  if (!this->isValid())
    return "Unknown Pixel Format";

  std::string name;
  if (this->alphaMode == AlphaMode::First)
    name += "A";
  name += ChannelOrderMapper.getName(this->channelOrder);
  if (this->alphaMode == AlphaMode::Last)
    name += "A";

  name += " " + std::to_string(this->bitsPerPixel) + "bit";
  if (this->dataLayout == DataLayout::Planar)
    name += " planar";
  if (this->bitsPerPixel > 8 && this->endianness == Endianness::Big)
    name += " BE";

  return name;
}

/* Get the number of bytes for a frame with this RGB format and the given size
 */
std::size_t PixelFormatRGB::bytesPerFrame(Size frameSize) const
{
  if (!isValid() || !frameSize.isValid())
    return 0;

  auto numSamples = std::size_t(frameSize.height) * std::size_t(frameSize.width);
  auto nrBytes    = numSamples * this->bitsPerPixel;
  LOGD("PixelFormatRGB::bytesPerFrame samples {} channels {} bytes {}",
                   int(numSamples),
                   this->nrChannels(),
                   nrBytes);
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