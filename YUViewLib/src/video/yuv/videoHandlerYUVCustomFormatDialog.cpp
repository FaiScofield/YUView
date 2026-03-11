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

#include "videoHandlerYUVCustomFormatDialog.h"

#include <common/Functions.h>

namespace video::yuv
{

videoHandlerYUVCustomFormatDialog::videoHandlerYUVCustomFormatDialog(
    const PixelFormatYUV &yuvFormat, QWidget *parent)
    : QWidget(parent)
{
  this->ui.setupUi(this);

  // Fill the comboBoxes and set all values correctly from the given yuvFormat

  // Chroma subsampling
  this->ui.comboBoxChromaSubsampling->addItems(
    functions::toQStringList(SubsamplingMapper.getNames()));
  if (yuvFormat.getSubsampling() != Subsampling::UNKNOWN)
  {
    if (auto index = SubsamplingMapper.indexOf(yuvFormat.getSubsampling()))
    {
      this->ui.comboBoxChromaSubsampling->setCurrentIndex(int(index));
      // The Q_Object auto connection is performed later so call the slot manually.
      // This will fill comboBoxPackingOrder
      this->on_comboBoxChromaSubsampling_currentIndexChanged(
        this->ui.comboBoxChromaSubsampling->currentIndex());
    }
  }

  // Bit depth
  for (auto bitDepth : BitDepthList)
    this->ui.comboBoxBitDepth->addItem(QString("%1").arg(bitDepth));
  {
    const auto idx = vectorIndexOf(BitDepthList, yuvFormat.getBitsPerSample());
    this->ui.comboBoxBitDepth->setCurrentIndex(idx ? static_cast<int>(*idx) : 0);
    this->ui.comboBoxEndianness->setEnabled(idx.has_value());
  }

  // Endianness
  this->ui.comboBoxEndianness->setCurrentIndex(yuvFormat.isBigEndian() ? 0 : 1);

  // Chroma offsets
  this->ui.comboBoxChromaOffsetX->setCurrentIndex(yuvFormat.getChromaOffset().x);
  this->ui.comboBoxChromaOffsetY->setCurrentIndex(yuvFormat.getChromaOffset().y);

  // Component layout
  ComponentLayout layout = yuvFormat.getComponentLayout();
  if (layout == ComponentLayout::Interleaved)
    this->ui.radioButtonInterleaved->setChecked(true);
  else if (layout == ComponentLayout::SemiPlanar)
    this->ui.radioButtonSemiPlanar->setChecked(true);
  else
    this->ui.radioButtonPlanar->setChecked(true);

  // Component order
  updateComponentOrderComboBox();
  if (auto idx = ComponentOrderMapper.indexOf(yuvFormat.getComponentOrder()))
    this->ui.comboBoxElemOrder->setCurrentIndex(static_cast<int>(idx));

  // Padding info
  if (auto idx = PaddingInfoMapper.indexOf(yuvFormat.getPaddingInfo()))
    this->ui.comboBoxPaddingInfo->setCurrentIndex(static_cast<int>(idx));

  // Byte packing
  this->ui.checkBoxBytePacking->setChecked(yuvFormat.isBytePacking());

  // Connect all other controls to emit formatChanged signal
  connect(this->ui.comboBoxEndianness,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerYUVCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxChromaOffsetX,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerYUVCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxChromaOffsetY,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerYUVCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxElemOrder,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerYUVCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxPaddingInfo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerYUVCustomFormatDialog::formatChanged);
  connect(this->ui.checkBoxBytePacking,
          &QCheckBox::stateChanged,
          this,
          &videoHandlerYUVCustomFormatDialog::formatChanged);
  connect(this->ui.radioButtonInterleaved,
          &QRadioButton::toggled,
          this,
          &videoHandlerYUVCustomFormatDialog::updateComponentOrderComboBox);
  connect(this->ui.radioButtonSemiPlanar,
          &QRadioButton::toggled,
          this,
          &videoHandlerYUVCustomFormatDialog::updateComponentOrderComboBox);
  connect(this->ui.radioButtonPlanar,
          &QRadioButton::toggled,
          this,
          &videoHandlerYUVCustomFormatDialog::updateComponentOrderComboBox);

  // Update UI state based on initial bit depth
  this->on_comboBoxBitDepth_currentIndexChanged(this->ui.comboBoxBitDepth->currentIndex());
}

void videoHandlerYUVCustomFormatDialog::updateComponentOrderComboBox()
{
  // Update element order combo box based on current layout selection
  ComponentLayout layout = ComponentLayout::Planar;
  if (this->ui.radioButtonInterleaved->isChecked())
    layout = ComponentLayout::Interleaved;
  else if (this->ui.radioButtonSemiPlanar->isChecked())
    layout = ComponentLayout::SemiPlanar;

  Subsampling subsampling =
    static_cast<Subsampling>(this->ui.comboBoxChromaSubsampling->currentIndex());

  this->ui.comboBoxElemOrder->clear();
  auto supportedOrders = getSupportedComponentOrders(subsampling, layout);
  for (auto order : supportedOrders)
  {
    const auto name = ComponentOrderMapper.getName(order);
    this->ui.comboBoxElemOrder->addItem(QString::fromStdString(std::string(name)));
  }

  // if (layout == ComponentLayout::Interleaved && subsampling == Subsampling::YUV_422)
  // {
  //   this->ui.comboBoxElemOrder->addItem("UYVY");
  //   this->ui.comboBoxElemOrder->addItem("VYUY");
  //   this->ui.comboBoxElemOrder->addItem("YUYV");
  //   this->ui.comboBoxElemOrder->addItem("YVYU");
  // }
  // else
  // {
  //   this->ui.comboBoxElemOrder->addItem("YUV");
  //   this->ui.comboBoxElemOrder->addItem("YVU");
  //   this->ui.comboBoxElemOrder->addItem("AYUV");
  //   this->ui.comboBoxElemOrder->addItem("VUYA");
  //   this->ui.comboBoxElemOrder->addItem("YUVA");
  //   this->ui.comboBoxElemOrder->addItem("YVUA");
  // }
  emit formatChanged();
}

void videoHandlerYUVCustomFormatDialog::on_comboBoxChromaSubsampling_currentIndexChanged(int idx)
{
  auto subsampling = static_cast<Subsampling>(idx);

  // What chroma offsets are possible?
  this->ui.comboBoxChromaOffsetX->clear();
  auto maxValsX = getMaxPossibleChromaOffsetValues(true, subsampling);
  if (maxValsX >= 1)
    this->ui.comboBoxChromaOffsetX->addItems(QStringList() << "0" << "1/2");
  if (maxValsX >= 3)
    this->ui.comboBoxChromaOffsetX->addItems(QStringList() << "1" << "3/2");
  if (maxValsX >= 7)
    this->ui.comboBoxChromaOffsetX->addItems(QStringList() << "2" << "5/2" << "3" << "7/2");

  this->ui.comboBoxChromaOffsetY->clear();
  int maxValsY = getMaxPossibleChromaOffsetValues(false, subsampling);
  if (maxValsY >= 1)
    this->ui.comboBoxChromaOffsetY->addItems(QStringList() << "0" << "1/2");
  if (maxValsY >= 3)
    this->ui.comboBoxChromaOffsetY->addItems(QStringList() << "1" << "3/2");
  if (maxValsY >= 7)
    this->ui.comboBoxChromaOffsetY->addItems(QStringList() << "2" << "5/2" << "3" << "7/2");

  // Disable the combo boxes if there are no chroma components
  bool chromaPresent = (subsampling != Subsampling::YUV_400);
  this->ui.comboBoxChromaOffsetX->setEnabled(chromaPresent);
  this->ui.comboBoxChromaOffsetY->setEnabled(chromaPresent);

  // disable interleaved if subsampling is 420/400
  if (subsampling == Subsampling::YUV_420 || subsampling == Subsampling::YUV_400 ||
      subsampling == Subsampling::YUV_410 || subsampling == Subsampling::YUV_411)
    this->ui.radioButtonInterleaved->setEnabled(false);
  else
    this->ui.radioButtonInterleaved->setEnabled(true);

  updateComponentOrderComboBox();
}

PixelFormatYUV videoHandlerYUVCustomFormatDialog::getSelectedYUVFormat() const
{
  const auto subsamplingIndex = this->ui.comboBoxChromaSubsampling->currentIndex();
  if (subsamplingIndex < 0)
    return {};
  const auto subsampling = SubsamplingMapper.getValueAt(static_cast<std::size_t>(subsamplingIndex));
  if (!subsampling)
    return {};

  const auto bitDepthIndex = this->ui.comboBoxBitDepth->currentIndex();
  if (bitDepthIndex < 0 || bitDepthIndex >= int(BitDepthList.size()))
    return {};
  const auto bitsPerSample = BitDepthList.at(unsigned(bitDepthIndex));

  const auto bigEndian    = (this->ui.comboBoxEndianness->currentIndex() == 0);
  const auto chromaOffset = Offset({this->ui.comboBoxChromaOffsetX->currentIndex(),
                                    this->ui.comboBoxChromaOffsetY->currentIndex()});

  // Get component layout
  ComponentLayout componentLayout;
  if (this->ui.radioButtonInterleaved->isChecked())
    componentLayout = ComponentLayout::Interleaved;
  else if (this->ui.radioButtonSemiPlanar->isChecked())
    componentLayout = ComponentLayout::SemiPlanar;
  else
    componentLayout = ComponentLayout::Planar;

  // Get component order
  const std::string orderName = this->ui.comboBoxElemOrder->currentText().toStdString();
  const auto componentOrder = ComponentOrderMapper.getValueFromNameOrIndex(orderName);
  if (!componentOrder)
    return {};

  // Get padding info
  const auto paddingInfoIndex = this->ui.comboBoxPaddingInfo->currentIndex();
  if (paddingInfoIndex < 0)
    return {};
  const auto paddingInfo = PaddingInfoMapper.getValueAt(static_cast<std::size_t>(paddingInfoIndex));
  if (!paddingInfo)
    return {};

  const auto bytePacking = this->ui.checkBoxBytePacking->isChecked();

  return PixelFormatYUV(
    *subsampling, bitsPerSample, componentLayout, *componentOrder, bigEndian,
    chromaOffset, bytePacking, *paddingInfo);
}

void videoHandlerYUVCustomFormatDialog::on_comboBoxBitDepth_currentIndexChanged(int idx)
{
  // Endianness only makes sense when the bit depth is > 8bit.
  const bool bitDepth8 = (idx == 0);
  this->ui.comboBoxEndianness->setEnabled(!bitDepth8);

  // Byte packing only valid for bit depths that are not divisible by 8.
  const auto bitsPerSample = BitDepthList.at(unsigned(idx));
  const bool bytePackingEnabled = bitsPerSample % 8 > 0;
  this->ui.checkBoxBytePacking->setEnabled(bytePackingEnabled);

  // Padding info is relevant for bit depths that are not divisible by 8.
  const bool paddingInfoEnabled = bytePackingEnabled;
  this->ui.comboBoxPaddingInfo->setEnabled(paddingInfoEnabled);
  if (!paddingInfoEnabled)
    this->ui.comboBoxPaddingInfo->setCurrentIndex(0); // NoPadding

  emit formatChanged();
}

} // namespace video::yuv
