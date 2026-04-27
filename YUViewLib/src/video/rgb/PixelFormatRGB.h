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

#pragma once

#include <common/EnumMapper.h>
#include <common/Typedef.h>
#include <video/PixelFormat.h>

#include <string>

namespace video::rgb
{

enum class Channel
{
  Red,
  Green,
  Blue,
  Alpha
};

constexpr EnumMapper<Channel, 4> ChannelMapper = {std::make_pair(Channel::Red, "Red"),
                                                  std::make_pair(Channel::Green, "Green"),
                                                  std::make_pair(Channel::Blue, "Blue"),
                                                  std::make_pair(Channel::Alpha, "Alpha")};

struct rgba_t
{
  unsigned R{0}, G{0}, B{0}, A{0};
  unsigned dr{0}, dg{0}, db{0}, da{0}; // component depth of each channel

  unsigned &operator[](const Channel channel)
  {
    if (channel == Channel::Red)
      return this->R;
    if (channel == Channel::Green)
      return this->G;
    if (channel == Channel::Blue)
      return this->B;
    if (channel == Channel::Alpha)
      return this->A;

    throw std::out_of_range("Unsupported channel for value access");
  }

  unsigned at(const Channel channel) const
  {
    if (channel == Channel::Red)
      return this->R;
    if (channel == Channel::Green)
      return this->G;
    if (channel == Channel::Blue)
      return this->B;
    if (channel == Channel::Alpha)
      return this->A;

    throw std::out_of_range("Unsupported channel for value access");
  }

  bool operator==(const rgba_t &other) const
  {
    return this->R == other.R && this->G == other.G && this->B == other.B && this->A == other.A;
  };

  bool operator!=(const rgba_t &other) const
  {
    return this->R != other.R || this->G != other.G || this->B != other.B || this->A != other.A;
  };
};

template<typename T>
inline T convertBitness(T value, unsigned src_bitness, unsigned dst_bitness) {
  if (src_bitness > dst_bitness)
    return value >> (src_bitness - dst_bitness);
  else
    return value << (dst_bitness - src_bitness);
}

inline rgba_t convertBitness(rgba_t value, unsigned src_bitness, unsigned dst_bitness) {
  return rgba_t({
    convertBitness(value.R, src_bitness, dst_bitness),
    convertBitness(value.G, src_bitness, dst_bitness),
    convertBitness(value.B, src_bitness, dst_bitness),
    convertBitness(value.A, src_bitness, dst_bitness)
  });
}

inline rgba_t convertBitnessTo8Bit(rgba_t value)
{
  const int maxR = (1 << value.dr) - 1;
  const int maxG = (1 << value.dg) - 1;
  const int maxB = (1 << value.db) - 1;
  const int maxA = (1 << value.da) - 1;
  if (maxR > 0) {
    value.R = (value.R * 255 + (maxR >> 1)) / maxR;
    value.dr = 8;
  }
  if (maxG > 0) {
    value.G = (value.G * 255 + (maxG >> 1)) / maxG;
    value.dg = 8;
  }
  if (maxB > 0) {
    value.B = (value.B * 255 + (maxB >> 1)) / maxB;
    value.db = 8;
  }
  if (maxA > 0) {
    value.A = (value.A * 255 + (maxA >> 1)) / maxA;
    value.da = 8;
  }
  return value;
}

/**
 * for common unpacked formats, order starts from LSB to MSB,
 * but from MSB to LSB order for the bitpacked formats
 */
enum class ChannelOrder
{
  RGB,
  RBG,
  GRB,
  GBR,
  BRG,
  BGR
};

constexpr EnumMapper<ChannelOrder, 6> ChannelOrderMapper = {
    std::make_pair(ChannelOrder::RGB, "RGB"),
    std::make_pair(ChannelOrder::RBG, "RBG"),
    std::make_pair(ChannelOrder::GRB, "GRB"),
    std::make_pair(ChannelOrder::GBR, "GBR"),
    std::make_pair(ChannelOrder::BRG, "BRG"),
    std::make_pair(ChannelOrder::BGR, "BGR")};

enum class AlphaMode
{
  None,
  First,
  Last,
  InLsb = First, // lowest bits
  InMsb = Last, // highest bits
};

constexpr EnumMapper<AlphaMode, 3> AlphaModeMapper = {std::make_pair(AlphaMode::None, "None"),
                                                      std::make_pair(AlphaMode::First, "First"),
                                                      std::make_pair(AlphaMode::Last, "Last")};

enum class DiffCompDepthType
{
  None,
  BPP8_RGB332,
  BPP16_RGB565,
  BPP16_RGBA5551,
  BPP32_RGBA1010102,
  // RGBX8888, RGBX4444
};

constexpr EnumMapper<DiffCompDepthType, 5> DiffCompDepthTypeMapper = {
  std::make_pair(DiffCompDepthType::None, "None"),
  std::make_pair(DiffCompDepthType::BPP8_RGB332, "BPP8_RGB332"),
  std::make_pair(DiffCompDepthType::BPP16_RGB565, "BPP16_RGB565"),
  std::make_pair(DiffCompDepthType::BPP16_RGBA5551, "BPP16_RGBA5551"), // include RGBX5551
  std::make_pair(DiffCompDepthType::BPP32_RGBA1010102, "BPP32_RGBA1010102"), // include RGBX1010102
};


// This class defines a specific RGB format with all properties like order of R/G/B, bitsPerValue,
// planarity...
class PixelFormatRGB
{
public:
  // The default constructor (will create an "Unknown Pixel Format")
  PixelFormatRGB() = default;
  PixelFormatRGB(const std::string &name);
  PixelFormatRGB(unsigned          bitsPerSample,
                 DataLayout        dataLayout   = DataLayout::Interleaved,
                 ChannelOrder      channelOrder = ChannelOrder::RGB,
                 AlphaMode         alphaMode    = AlphaMode::None,
                 Endianness        endianness   = Endianness::Little,
                 PaddingInfo       paddingInfo  = PaddingInfo::NoPadding,
                 bool              bytePacking  = false,
                 DiffCompDepthType diffType     = DiffCompDepthType::None);

  /**
  * DataLayout must be Interleaved,
  * bytePacking must be true,
  * AlphaMode/PaddingInfo are dependent on diffType
  */
  PixelFormatRGB(DiffCompDepthType diffType,
                 ChannelOrder      channelOrder = ChannelOrder::RGB,
                 AlphaMode         alphaMode    = AlphaMode::None,
                 PaddingInfo       paddingInfo  = PaddingInfo::NoPadding,
                 Endianness        endianness   = Endianness::Little);

  bool        isValid() const;
  bool        isDiffCompDepth() const { return this->diffCompType != DiffCompDepthType::None; }
  unsigned    nrChannels() const { return this->alphaMode != AlphaMode::None ? 4 : 3; }
  bool        hasAlpha() const { return this->alphaMode != AlphaMode::None; }
  bool        hasPadding() const { return this->paddingInfo != PaddingInfo::NoPadding; }
  std::string getName() const;

  unsigned          getBitsPerSample() const { return bitsPerSample; }
  DataLayout        getDataLayout() const { return this->dataLayout; }
  ChannelOrder      getChannelOrder() const { return this->channelOrder; }
  AlphaMode         getAlphaMode() const { return this->alphaMode; }
  Endianness        getEndianess() const { return this->endianness; }
  PaddingInfo       getPaddingInfo() const { return this->paddingInfo; }
  bool              isBytePacking() const { return bytePacking; }
  DiffCompDepthType getDiffCompType() const { return this->diffCompType; }
  unsigned          getBitsPerPixel() const { return bitsPerPixel; }


  void setBitsPerSample(unsigned bitsPerSample) { this->bitsPerSample = bitsPerSample; this->name.clear(); }
  void setDataLayout(DataLayout dataLayout) { this->dataLayout = dataLayout; this->name.clear(); }
  void setChannelOrder(ChannelOrder channelOrder) { this->channelOrder = channelOrder; this->name.clear(); }
  void setAlphaMode(AlphaMode alphaMode) { this->alphaMode = alphaMode; this->name.clear(); }
  void setEndianess(Endianness endianness) { this->endianness = endianness; this->name.clear(); }
  void setPaddingInfo(PaddingInfo paddingInfo) { this->paddingInfo = paddingInfo; this->name.clear(); }
  void setBytePacking(bool bytePacking) { this->bytePacking = bytePacking; this->name.clear(); }
  void setDiffCompType(DiffCompDepthType diffCompType);

  std::size_t bytesPerFrame(Size frameSize) const;
  int         getChannelPosition(Channel channel) const; // todo: check for DiffCompDepthType
  Channel     getChannelAtPosition(int position) const; // todo: check for DiffCompDepthType

  bool operator==(const PixelFormatRGB &a) const { return getName() == a.getName(); }
  bool operator!=(const PixelFormatRGB &a) const { return getName() != a.getName(); }
  bool operator==(const std::string &a) const { return getName() == a; }
  bool operator!=(const std::string &a) const { return getName() != a; }

private:
  mutable std::string name{};

  unsigned     bitsPerSample{8};
  DataLayout   dataLayout{DataLayout::Interleaved};
  ChannelOrder channelOrder{ChannelOrder::RGB};
  AlphaMode    alphaMode{AlphaMode::None};
  Endianness   endianness{Endianness::Little};
  PaddingInfo  paddingInfo{PaddingInfo::NoPadding};
  bool         bytePacking{false};

  // used when the depth of components are different
  unsigned          bitsPerPixel{24}; // depends on the diffCompType
  DiffCompDepthType diffCompType{DiffCompDepthType::None};
};

} // namespace video::rgb
