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

#include "PixelFormatYUV.h"
#include "Logger.h"

#include <regex>
#include <map>

namespace video::yuv
{

std::map<std::string, PixelFormatYUV> knownYuvFormatMap = {
  {"NV12", PixelFormatYUV(Subsampling::YUV_420, 8, DataLayout::SemiPlanar, ComponentOrder::YUV, false, {}, false, PaddingInfo::NoPadding)},
  {"NV16", PixelFormatYUV(Subsampling::YUV_422, 8, DataLayout::SemiPlanar, ComponentOrder::YUV, false, {}, false, PaddingInfo::NoPadding)},
  {"NV24", PixelFormatYUV(Subsampling::YUV_444, 8, DataLayout::SemiPlanar, ComponentOrder::YUV, false, {}, false, PaddingInfo::NoPadding)},
  {"NV21", PixelFormatYUV(Subsampling::YUV_420, 8, DataLayout::SemiPlanar, ComponentOrder::YVU, false, {}, false, PaddingInfo::NoPadding)},
  {"NV61", PixelFormatYUV(Subsampling::YUV_422, 8, DataLayout::SemiPlanar, ComponentOrder::YVU, false, {}, false, PaddingInfo::NoPadding)},
  {"NV42", PixelFormatYUV(Subsampling::YUV_444, 8, DataLayout::SemiPlanar, ComponentOrder::YVU, false, {}, false, PaddingInfo::NoPadding)},
  {"NV15", PixelFormatYUV(Subsampling::YUV_420, 10, DataLayout::SemiPlanar, ComponentOrder::YUV, false, {}, true, PaddingInfo::NoPadding)},
  {"NV20", PixelFormatYUV(Subsampling::YUV_422, 10, DataLayout::SemiPlanar, ComponentOrder::YUV, false, {}, true, PaddingInfo::NoPadding)},
  {"NV30", PixelFormatYUV(Subsampling::YUV_444, 10, DataLayout::SemiPlanar, ComponentOrder::YUV, false, {}, true, PaddingInfo::NoPadding)},
};

void getColorConversionCoefficients(ColorConversion colorConversion, int RGBConv[5])
{
  // The conversion parameters for the components of the different supported YUV->RGB conversions
  // The first index is the index of the ColorConversion enum. The second index is [Y, cRV, cGU,
  // cGV, cBU].
  const int yuvRgbConvCoeffs[6][5] = {
      {76309, 117489, -13975, -34925, 138438}, // BT709_LimitedRange
      {65536, 103206, -12276, -30679, 121608}, // BT709_FullRange
      {76309, 104597, -25675, -53279, 132201}, // BT601_LimitedRange
      {65536, 91881, -22553, -46802, 116129},  // BT601_FullRange
      {76309, 110013, -12276, -42626, 140363}, // BT2020_LimitedRange
      {65536, 96638, -10783, -37444, 123299}   // BT2020_FullRange
  };
  const auto index = ColorConversionMapper.indexOf(colorConversion);
  for (unsigned i = 0; i < 5; i++)
    RGBConv[i] = yuvRgbConvCoeffs[index][i];
}

// All values between 0 and this value are possible for the subsampling.
int getMaxPossibleChromaOffsetValues(bool horizontal, Subsampling subsampling)
{
  if (subsampling == Subsampling::YUV_444)
    return 1;
  else if (subsampling == Subsampling::YUV_422)
    return (horizontal) ? 3 : 1;
  else if (subsampling == Subsampling::YUV_420)
    return 3;
  else if (subsampling == Subsampling::YUV_440)
    return (horizontal) ? 1 : 3;
  else if (subsampling == Subsampling::YUV_410)
    return 7;
  else if (subsampling == Subsampling::YUV_411)
    return (horizontal) ? 7 : 1;
  return 0;
}

// Return a list with all the packing formats that are supported with this subsampling
std::vector<ComponentOrder> getSupportedComponentOrders(Subsampling subsampling, DataLayout layout)
{
  if (layout == DataLayout::Interleaved &&
      (subsampling == Subsampling::YUV_422 || subsampling == Subsampling::YUV_420))
    return std::vector<ComponentOrder>(
      {ComponentOrder::UYVY, ComponentOrder::VYUY, ComponentOrder::YUYV, ComponentOrder::YVYU});

  return std::vector<ComponentOrder>({ComponentOrder::YUV,
                                      ComponentOrder::YVU,
                                      ComponentOrder::AYUV,
                                      ComponentOrder::VUYA,
                                      ComponentOrder::YUVA,
                                      ComponentOrder::YVUA});

  return {};
}

// Is this the default chroma offset for the subsampling type?
bool isDefaultChromaFormat(int chromaOffset, bool offsetX, Subsampling subsampling)
{
  if (subsampling == Subsampling::YUV_420 && !offsetX)
    // The default subsampling for YUV 420 has a Y offset of 1/2
    return chromaOffset == 1;
  else if (subsampling == Subsampling::YUV_400)
    return true;
  return chromaOffset == 0;
}

static std::optional<Subsampling> parseSubsamplingText(const std::string_view text)
{
  if (text.size() == 5) // 4:4:4
  {
    if (text.at(1) != ':' || text.at(3) != ':')
      return {};

    const std::string subsamplingName = {text.at(0), text.at(2), text.at(4)};
    return SubsamplingMapper.getValue(subsamplingName);
  }

  if (text.size() == 3) // 444
  {
    return SubsamplingMapper.getValue(text);
  }
  return {};
}

std::string formatSubsamplingWithColons(const Subsampling &subsampling)
{
  const auto name = SubsamplingMapper.getName(subsampling);

  std::stringstream s;
  s << name.at(0) << ":" << name.at(1) << ":" << name.at(2);
  return s.str();
}

PixelFormatYUV::PixelFormatYUV(const std::string &name)
{
  if (auto predefinedFormat = PredefinedPixelFormatMapper.getValue(name))
  {
    if (*predefinedFormat == PredefinedPixelFormat::V210)
      this->predefinedPixelFormat = predefinedFormat;
  }

  if (knownYuvFormatMap.find(name) != knownYuvFormatMap.end())
  {
    *this = knownYuvFormatMap.at(name);
    this->name = name;
    return;
  }

  std::regex strExpr(
      "^([YUVA]{3,6}|UYVY|VYUY|YUYV|YVYU)(4:\\d{1}:\\d{1})(I|SP|P)? (\\d{1,2})-bit( BE| LE)?( "
      "BytePacking| UnPacking)?( PaddingInLSB| PaddingInMSB)?( Cx\\d+)?( Cy\\d+)?$");

  std::smatch sm;
  if (!std::regex_match(name, sm, strExpr))
    return;

  try
  {
    PixelFormatYUV newFormat;

    // Parse the component order
    auto orderName = sm.str(1);
    if (auto co = ComponentOrderMapper.getValue(orderName))
      newFormat.componentOrder = *co;

    // Parse subsampling (e.g., 4:4:4 -> 444)
    if (auto subsampling = parseSubsamplingText(sm.str(2)))
      newFormat.subsampling = *subsampling;

    // Parse component layout
    auto layoutStr = sm.str(3);
    if (layoutStr == "I")
      newFormat.dataLayout = DataLayout::Interleaved;
    else if (layoutStr == "SP")
      newFormat.dataLayout = DataLayout::SemiPlanar;
    else
      newFormat.dataLayout = DataLayout::Planar;

    // Get the bit depth
    {
      auto   bitdepthStr = sm.str(4);
      size_t sz          = 0;
      int    bitDepth    = std::stoi(bitdepthStr, &sz);
      if (sz > 0 && bitDepth >= 8 && bitDepth <= 32)
        newFormat.bitsPerSample = bitDepth;
    }

    // Get the endianness. If not in the name, assume LE
    newFormat.bigEndian = (sm.str(5) == " BE");

    // Parse byte packing
    auto packingStr = sm.str(6);
    newFormat.bytePacking = (packingStr == " BytePacking");

    // Get the padding info
    auto paddingStr = sm.str(7);
    if (!paddingStr.empty())
    {
      auto paddingName = paddingStr.substr(1);
      if (auto pi = PaddingInfoMapper.getValue(paddingName))
        newFormat.paddingInfo = *pi;
    }

    // Get the chroma offsets
    newFormat.setDefaultChromaOffset();
    auto chromaOffsetXStr = sm.str(8);
    if (!chromaOffsetXStr.empty() && chromaOffsetXStr.substr(1, 2) == "Cx")
    {
      size_t sz;
      auto   offsetX = std::stoi(chromaOffsetXStr.substr(3), &sz);
      if (sz > 0 && offsetX >= 0)
        newFormat.chromaOffset.x = offsetX;
    }

    auto chromaOffsetYStr = sm.str(9);
    if (!chromaOffsetYStr.empty() && chromaOffsetYStr.substr(1, 2) == "Cy")
    {
      size_t sz;
      auto   offsetY = std::stoi(chromaOffsetYStr.substr(3), &sz);
      if (sz > 0 && offsetY >= 0)
        newFormat.chromaOffset.y = offsetY;
    }

    // Check if the format is valid.
    if (newFormat.isValid())
    {
      // Set all the values from the new format
      this->subsampling    = newFormat.subsampling;
      this->bitsPerSample  = newFormat.bitsPerSample;
      this->bigEndian      = newFormat.bigEndian;
      this->chromaOffset   = newFormat.chromaOffset;
      this->dataLayout     = newFormat.dataLayout;
      this->componentOrder = newFormat.componentOrder;
      this->paddingInfo    = newFormat.paddingInfo;
      this->bytePacking    = newFormat.bytePacking;
    }
  }
  catch (const std::exception &e)
  {
    LOGE("generate PixelFormatYUV by name failed: %s", e.what());
  }
}

PixelFormatYUV::PixelFormatYUV(Subsampling    subsampling,
                               unsigned       bitsPerSample,
                               DataLayout     dataLayout,
                               ComponentOrder componentOrder,
                               bool           bigEndian,
                               Offset         chromaOffset,
                               bool           bytePacking,
                               PaddingInfo    paddingInfo)
    : subsampling(subsampling), bitsPerSample(bitsPerSample), bigEndian(bigEndian),
      dataLayout(dataLayout), chromaOffset(chromaOffset), componentOrder(componentOrder)
{
  if (bitsPerSample % 8 > 0)
  {
    this->bytePacking = bytePacking;
    this->paddingInfo = paddingInfo;
  }
  else
  {
    this->bytePacking = false;
    this->paddingInfo = PaddingInfo::NoPadding;
  }

  this->setDefaultChromaOffset();
  this->name = getName();
}


PixelFormatYUV::PixelFormatYUV(PredefinedPixelFormat predefinedPixelFormat)
    : predefinedPixelFormat(predefinedPixelFormat)
{
}

std::optional<PredefinedPixelFormat> PixelFormatYUV::getPredefinedFormat() const
{
  return this->predefinedPixelFormat;
}

bool PixelFormatYUV::isValid() const
{
  if (this->predefinedPixelFormat.has_value())
    return true;

  if (this->componentOrder == ComponentOrder::UNKNOWN) {
    LOGW("PixelFormatYUV::isValid: componentOrder is UNKNOWN");
    return false;
  }
  // if ((subsampling == Subsampling::YUV_422) && (this->componentOrder < ComponentOrder::UYVY))
  //   return false;
  // if (this->componentOrder >= ComponentOrder::UYVY)
  //   return false;
  if (this->dataLayout == DataLayout::Interleaved)
  {
    if (this->subsampling > Subsampling::YUV_422) {
      LOGW("PixelFormatYUV::isValid: No support for interleaved formats with this subsampling {} (yet)", SubsamplingMapper.getName(this->subsampling));
      return false;
    }
  }
  if (this->subsampling != Subsampling::YUV_400)
  {
    // There are chroma components. Check the chroma offsets.
    if (this->chromaOffset.x < 0 ||
        this->chromaOffset.x > getMaxPossibleChromaOffsetValues(true, this->subsampling))
    {
      LOGW("PixelFormatYUV::isValid: chromaOffset.x {} is out of range [0, {}]",
           this->chromaOffset.x,
           getMaxPossibleChromaOffsetValues(true, this->subsampling));
      return false;
    }
    if (this->chromaOffset.y < 0 ||
        this->chromaOffset.y > getMaxPossibleChromaOffsetValues(false, this->subsampling))
    {
      LOGW("PixelFormatYUV::isValid: chromaOffset.y {} is out of range [0, {}]",
           this->chromaOffset.y,
           getMaxPossibleChromaOffsetValues(false, this->subsampling));
      return false;
    }
  }
  // Check the bit depth
  if (this->bitsPerSample < 7) {
    LOGW("PixelFormatYUV::isValid: bitsPerSample {} is out of range [7, 16]", this->bitsPerSample);
    return false;
  }
  return true;
}

bool PixelFormatYUV::canConvertToRGB(Size imageSize, std::string *whyNot) const
{
  if (this->predefinedPixelFormat.has_value())
    return true;
  if (!this->isValid())
  {
    if (whyNot)
      whyNot->append("Invalid format");
    return false;
  }

  // Check the bit depth
  const int  bps         = this->bitsPerSample;
  const bool bytePacking = this->bytePacking;
  bool       canConvert  = true;
  if (bps < 8 || bps > 16)
  {
    if (whyNot)
    {
      std::stringstream ss;
      ss << "The currently set bit depth " << bps << " is not supported. Only [8, 16] supported.\n";
      whyNot->append(ss.str());
    }
    canConvert = false;
  }
  if (imageSize.width % this->getSubsamplingHor() != 0)
  {
    if (whyNot)
    {
      std::stringstream ss;
      ss << "The item width " << imageSize.width
         << " must be divisible by the horizontal subsampling factor " << this->getSubsamplingHor()
         << ".\n";
      whyNot->append(ss.str());
    }
    canConvert = false;
  }
  if (imageSize.height % this->getSubsamplingVer() != 0)
  {
    if (whyNot)
    {
      std::stringstream ss;
      ss << "The item height " << imageSize.height
         << " must be divisible by the vertical subsampling factor " << this->getSubsamplingVer()
         << ".\n";
      whyNot->append(ss.str());
    }
    canConvert = false;
  }
  if (this->subsampling == Subsampling::UNKNOWN)
  {
    if (whyNot)
      whyNot->append("The current yuv subsampling is unknown.\n");
    canConvert = false;
  }
  if (this->isInterleaved() && this->subsampling != Subsampling::YUV_422 &&
      this->subsampling != Subsampling::YUV_444)
  {
    if (whyNot)
      whyNot->append("Interleaved YUV formats are onyl supported for 4:2:2 and 4:4:4 subsampling.\n");
    canConvert = false;
  }
  return canConvert;
}

int64_t PixelFormatYUV::bytesPerFrame(const Size &frameSize) const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
    {
      // 422 10 bit with 6 Y values per 16 bytes. Width is rounded up to a multiple of 48.
      // Although there is a weird expception to this in the standard.
      auto roundedUpWidth = (((frameSize.width + 48 - 1) / 48) * 48);
      return frameSize.height * roundedUpWidth * 16 / 6;
    }
    return -1;
  }

  const unsigned rowPitch = getMinRowPitch(frameSize.width, this->bitsPerSample, this->bytePacking);
  const unsigned planeHeights[4] = {0}; // TODO
  int64_t        bytes    = 0;

  if (this->dataLayout == DataLayout::Planar)
  {
      bytes += rowPitch * frameSize.height; // Luma plane
      if (this->subsampling == Subsampling::YUV_444)
        bytes += rowPitch * frameSize.height * 2; // U/V planes
      else if (this->subsampling == Subsampling::YUV_422 || this->subsampling == Subsampling::YUV_440)
        bytes += (rowPitch / 2) * frameSize.height * 2; // U/V planes, half the width
      else if (this->subsampling == Subsampling::YUV_420)
        bytes += (rowPitch / 2) * (frameSize.height / 2) * 2; // U/V planes, half the width and height
      else if (this->subsampling == Subsampling::YUV_410)
        bytes += (rowPitch / 4) * (frameSize.height / 4) * 2; // U/V planes, half the width and height
      else if (this->subsampling == Subsampling::YUV_411)
        bytes += (rowPitch / 4) * frameSize.height * 2; // U/V planes, quarter the width
      else if (this->subsampling == Subsampling::YUV_400)
        bytes += 0; // No chroma components
      else
        return -1; // Unknown subsampling

      // There is an additional alpha plane. The alpha plane is not subsampled
      if (this->hasAlpha())
        bytes += rowPitch * frameSize.height; // Alpha plane
  }
  else if (this->dataLayout == DataLayout::SemiPlanar)
  {
    bytes += rowPitch * frameSize.height; // Luma plane
    if (this->subsampling == Subsampling::YUV_444)
      bytes += (rowPitch * 2) * frameSize.height; // No subsampling
    else if (this->subsampling == Subsampling::YUV_422)
      bytes += rowPitch * frameSize.height; // Chroma: half horizontal resolution
    else if (this->subsampling == Subsampling::YUV_440)
      bytes += (rowPitch * 2) * (frameSize.height / 2); // Chroma: half vertical resolution
    else if (this->subsampling == Subsampling::YUV_420)
      bytes += rowPitch * (frameSize.height / 2); // Chroma: half vertical and horizontal resolution
    else if (this->subsampling == Subsampling::YUV_410)
      bytes += (rowPitch / 2) * (frameSize.height / 4); // Chroma: quarter vertical, quarter horizontal resolution
    else if (this->subsampling == Subsampling::YUV_411)
      bytes += (rowPitch / 2) * frameSize.height; // Chroma: quarter horizontal resolution
    else if (this->subsampling == Subsampling::YUV_400)
      bytes += 0; // No chroma components
    else
      return -1; // Unknown subsampling

    if (this->hasAlpha())
      return -1; // invalid format
  }
  else if (this->dataLayout == DataLayout::Interleaved)
  {
    // This is an interleaved format with byte packing
    unsigned rowPitchInterleaved = rowPitch * (hasAlpha() ? 4 : 3);

    if (this->subsampling == Subsampling::YUV_422 || subsampling == Subsampling::YUV_440)
      // All packing orders have 4 values per interleaved value (which has 2 Y samples)
      bytes = (rowPitch * 2) * frameSize.height;
    else if (this->subsampling == Subsampling::YUV_444)
      bytes = (rowPitch * 4) * frameSize.height;
    else
      return -1;  // Unknown subsampling
  }
  else
    return -1; // Unknown component layout
  return bytes;
}

// Generate a unique name for the YUV format
std::string PixelFormatYUV::getName() const
{
  if (!this->isValid())
    return "Invalid";
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return "V210";
    return "Invalid";
  }

  if (!this->name.empty())
    return this->name;

  /* format a name with attributes, e.g. "YUV4:2:2P 10-bit LE" */
  std::stringstream ss;

  ss << ComponentOrderMapper.getName(this->componentOrder); // YUV,YVU,YUYV...

  ss << formatSubsamplingWithColons(this->subsampling); // 4:2:0

  // Add component layout suffix: 'I'/'SP'/'P'
  if (this->subsampling != Subsampling::YUV_400)
  {
    if (this->dataLayout == DataLayout::Interleaved)
      ss << "I";
    else if (this->dataLayout == DataLayout::SemiPlanar)
      ss << "SP";
    else
      ss << "P";
  }

  ss << " " << this->bitsPerSample << "-bit";

  // Add the endianness (if the bit depth is greater 8)
  if (this->bitsPerSample > 8)
    ss << ((this->bigEndian) ? " BE" : " LE");

  if (this->bytePacking)
    ss << " BytePacking";
  else
    ss << " UnPacking";

  // Add the padding info (if not NoPadding and bit depth is not 8/16/24/32)
  if (this->paddingInfo != PaddingInfo::NoPadding && this->bitsPerSample % 8 > 0)
    ss << " " << PaddingInfoMapper.getName(this->paddingInfo);

  // Add the Chroma offsets (if it is not the default offset)
  if (!isDefaultChromaFormat(this->chromaOffset.x, true, this->subsampling))
    ss << " Cx" << this->chromaOffset.x;
  if (!isDefaultChromaFormat(this->chromaOffset.y, false, this->subsampling))
    ss << " Cy" << this->chromaOffset.y;

  return ss.str();
}

unsigned PixelFormatYUV::getNrPlanes() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return 3;
    return 0;
  }

  if (this->subsampling == Subsampling::YUV_400)
    return 1;
  if (this->dataLayout == DataLayout::Interleaved)
    return 1;
  if (this->dataLayout == DataLayout::SemiPlanar)
    return 2;
  return hasAlpha() ? 4 : 3;;
}

Subsampling PixelFormatYUV::getSubsampling() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return Subsampling::YUV_422;
    return Subsampling::UNKNOWN;
  }

  return this->subsampling;
}

int PixelFormatYUV::getSubsamplingHor(Component component) const
{
  auto sub = this->getSubsampling();

  if (component == Component::Luma)
    return 1;
  if (sub == Subsampling::YUV_410 || sub == Subsampling::YUV_411)
    return 4;
  if (sub == Subsampling::YUV_422 || sub == Subsampling::YUV_420)
    return 2;
  return 1;
}

int PixelFormatYUV::getSubsamplingVer(Component component) const
{
  auto sub = this->getSubsampling();

  if (component == Component::Luma)
    return 1;
  if (sub == Subsampling::YUV_410)
    return 4;
  if (sub == Subsampling::YUV_420 || sub == Subsampling::YUV_440)
    return 2;
  return 1;
}

void PixelFormatYUV::setDefaultChromaOffset()
{
  this->chromaOffset = Offset({0, 0});
  if (this->getSubsampling() == Subsampling::YUV_420)
    this->chromaOffset.y = 1;
}

bool PixelFormatYUV::isChromaSubsampled() const
{
  auto sub = this->getSubsampling();
  return sub != Subsampling::YUV_444;
}

unsigned PixelFormatYUV::getBitsPerSample() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return 10;
    return 0;
  }

  return this->bitsPerSample;
}

bool PixelFormatYUV::isBigEndian() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return false;
    return false;
  }

  return this->bigEndian;
}

bool PixelFormatYUV::isPlanar() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return false;
    return false;
  }

  return this->dataLayout == DataLayout::Interleaved;
}

bool PixelFormatYUV::hasAlpha() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return false;
    return false;
  }

  return this->componentOrder == ComponentOrder::AYUV || this->componentOrder == ComponentOrder::YUVA ||
         this->componentOrder == ComponentOrder::VUYA || this->componentOrder == ComponentOrder::YVUA;
}

/**
 * chroma pixel offset to luma pxiel
 * 0 for 0, 1 for 0.5, 2 for 1
 */
Offset PixelFormatYUV::getChromaOffset() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return Offset({0, 0});
    return Offset({0, 0});
  }

  return this->chromaOffset;
}

bool PixelFormatYUV::isBytePacking() const
{
  if (this->predefinedPixelFormat)
  {
    if (*this->predefinedPixelFormat == PredefinedPixelFormat::V210)
      return true;
    return false;
  }

  return this->bytePacking;
}


unsigned getMinRowPitch(unsigned width, unsigned bitsPerSample, bool bytePacking)
{
  unsigned minRowPitch = 0;

  // BitDepthList = {8, 9, 10, 12, 14, 16, 24, 32}
  switch (bitsPerSample)
  {
  case 9: /* 9bytes for 8 components */
    minRowPitch = bytePacking ? (width * 9 + 7) / 8 : width * 2;
    break;
  case 10: /* 5bytes for 4 components */
    minRowPitch = bytePacking ? (width * 5 + 3) / 4 : width * 2;
    break;
  case 12: /* 3bytes for 2 components */
    minRowPitch = bytePacking ? (width * 3 + 1) / 2 : width * 2;
    break;
  case 14: /* 7bytes for 4 components */
    minRowPitch = bytePacking ? (width * 7 + 3) / 4 : width * 2;
    break;
  case 8: /* 1byte for 1 component */
    minRowPitch = width * 1;
    break;
  case 16: /* 2bytes for 1 component */
    minRowPitch = width * 2;
    break;
  case 24: /* 3bytes for 1 component */
    minRowPitch = width * 3;
    break;
  case 32: /* 4bytes for 1 component */
    minRowPitch = width * 4;
    break;
  default:
    return width; // Unknown bitsPerSample
  }
  return minRowPitch;
}

} // namespace video::yuv
