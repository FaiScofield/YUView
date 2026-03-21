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

#include "videoHandlerRGBCustomFormatDialog.h"

#include <common/Functions.h>

namespace video::rgb
{

videoHandlerRGBCustomFormatDialog::videoHandlerRGBCustomFormatDialog(
    const PixelFormatRGB &rgbFormat, QWidget *parent)
    : QWidget(parent)
{
  this->ui.setupUi(this);

  // Set the default (RGB no alpha)
  this->ui.rgbOrderComboBox->addItems(functions::toQStringList(ChannelOrderMapper.getNames()));
  this->ui.rgbOrderComboBox->setCurrentIndex(0);
  if (auto index = ChannelOrderMapper.indexOf(rgbFormat.getChannelOrder()))
  {
    this->ui.rgbOrderComboBox->setCurrentIndex(int(index));
  }

  this->ui.hasAlphaCheckBox->setChecked(rgbFormat.hasAlpha());

  auto bpp = rgbFormat.getBitsPerPixel();
  if (bpp <= 32)
    this->ui.comboBoxPixelDepth->setCurrentIndex(bpp / 8);
  else
    this->ui.comboBoxPixelDepth->setCurrentIndex(0);

  this->ui.comboBoxEndianness->setEnabled(bpp > 8);
  this->ui.comboBoxEndianness->setCurrentIndex(rgbFormat.getEndianess() == Endianness::Big ? 0 : 1);

  this->ui.planarCheckBox->setChecked(rgbFormat.getDataLayout() == DataLayout::Planar);

  this->updateBitDepthComboBox();
  this->updateAlphaXComboBox();
  this->updateBitPackedComboBox();

  connect(this->ui.rgbOrderComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxPixelDepth,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_comboBoxPixelDepth_currentIndexChanged);
  connect(this->ui.comboBoxEndianness,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.planarCheckBox,
          &QCheckBox::stateChanged,
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.hasAlphaCheckBox,
          &QCheckBox::stateChanged,
          this,
          &videoHandlerRGBCustomFormatDialog::on_hasAlphaCheckBox_stateChanged);
  connect(this->ui.comboBoxAlphaX,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_comboBoxAlphaX_currentIndexChanged);
  connect(this->ui.comboBoxBitPacked,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
}

PixelFormatRGB videoHandlerRGBCustomFormatDialog::getSelectedRGBFormat() const
{
  const auto channelOrderIndex = this->ui.rgbOrderComboBox->currentIndex();
  if (channelOrderIndex < 0)
    return {};
  auto channelOrder = ChannelOrderMapper.getValueAt(static_cast<std::size_t>(channelOrderIndex));
  if (!channelOrder)
    return {};

  int totalBitsPerPixel = 8;
  switch (this->ui.comboBoxPixelDepth->currentIndex())
  {
    case 0:
      totalBitsPerPixel = 8;
      break;
    case 1:
      totalBitsPerPixel = 16;
      break;
    case 2:
      totalBitsPerPixel = 24;
      break;
    case 3:
      totalBitsPerPixel = 32;
      break;
    default:
      totalBitsPerPixel = 8;
  }

  auto dataLayout = DataLayout::Packed;
  if (this->ui.planarCheckBox->checkState() == Qt::Checked)
    dataLayout = DataLayout::Planar;

  auto alphaMode = AlphaMode::None;
  if (this->ui.hasAlphaCheckBox->isChecked())
  {
    if (this->ui.comboBoxAlphaX->currentIndex() == 0)
      alphaMode = AlphaMode::Last;
    else
      alphaMode = AlphaMode::First;
  }

  auto endianness = Endianness::Little;
  if (this->ui.comboBoxEndianness->currentIndex() == 0)
    endianness = Endianness::Big;

  return PixelFormatRGB(totalBitsPerPixel, dataLayout, *channelOrder, alphaMode, endianness);
}

void videoHandlerRGBCustomFormatDialog::on_comboBoxPixelDepth_currentIndexChanged(int index)
{
  this->ui.comboBoxEndianness->setEnabled(index >= 2);
  this->updateBitPackedComboBox();
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::on_hasAlphaCheckBox_stateChanged(int state)
{
  this->updateAlphaXComboBox();
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::on_comboBoxAlphaX_currentIndexChanged(int index)
{
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::updateBitDepthComboBox()
{
  // block signal while clearing the combo box to prevent signal emission
  QSignalBlocker blocker(this->ui.comboBoxPixelDepth);

  const bool hasAlpha = this->ui.hasAlphaCheckBox->isChecked();
  if (hasAlpha)
  {
  this->ui.comboBoxPixelDepth->clear();
}

void videoHandlerRGBCustomFormatDialog::updateAlphaXComboBox()
{
  // block signal while clearing the combo box to prevent signal emission
  QSignalBlocker blockerAlphaX(this->ui.comboBoxAlphaX);
  this->ui.comboBoxAlphaX->clear();

  const bool hasAlpha = this->ui.hasAlphaCheckBox->isChecked();

  if (hasAlpha)
  {
    this->ui.labelAlphaX->setText("Alpha Pos");
    this->ui.comboBoxAlphaX->addItem("NoAlpha");
    this->ui.comboBoxAlphaX->addItem("AlphaOnMsb");
    this->ui.comboBoxAlphaX->addItem("AlphaOnLsb");
  }
  else
  {
    this->ui.labelAlphaX->setText("Padding Pos");
    this->ui.comboBoxAlphaX->addItem("NoPadding");
    this->ui.comboBoxAlphaX->addItem("PaddingOnMsb");
    this->ui.comboBoxAlphaX->addItem("PaddingOnLsb");
  }

  this->ui.comboBoxAlphaX->setCurrentIndex(0);
}

void videoHandlerRGBCustomFormatDialog::updateBitPackedComboBox()
{
  // block signal while clearing the combo box to prevent signal emission
  QSignalBlocker blocker(this->ui.comboBoxBitPacked);
  this->ui.comboBoxBitPacked->clear();


  const int bpp = this->ui.comboBoxPixelDepth->currentText().toInt();
  const bool hasAlpha = this->ui.hasAlphaCheckBox->isChecked();
  const bool isPlanar = this->ui.planarCheckBox->isChecked();

  if (isPlanar)
  {
    this->ui.comboBoxBitPacked->addItem(BitPackedTypeMapper.getName(BitPackedType::Unpacked));
    this->ui.comboBoxBitPacked->setCurrentIndex(0);
    this->ui.comboBoxBitPacked->setEnabled(false);


    return;
  }

  auto supportedTypes = getSupportedBitPackedTypes(depthIndex * 8, hasAlpha);

  switch (bpp)
  {
    case 0:
      this->ui.comboBoxBitPacked->addItem("RGB332");
      break;
    case 1:
      this->ui.comboBoxBitPacked->addItem(QString("RGB%14444").arg(prefix));
      this->ui.comboBoxBitPacked->addItem(QString("RGB%15551").arg(prefix));
      this->ui.comboBoxBitPacked->addItem("RGB565");
      break;
    case 2:
      this->ui.comboBoxBitPacked->addItem("RGB888");
      break;
    case 3:
      this->ui.comboBoxBitPacked->addItem(QString("RGB%18888").arg(prefix));
      this->ui.comboBoxBitPacked->addItem(QString("RGB%11010102").arg(prefix));
      break;
    default:
      break;
  }

  this->ui.comboBoxBitPacked->setCurrentIndex(0);
}

} // namespace video::rgb