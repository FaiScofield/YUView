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

  this->ui.rgbOrderComboBox->addItems(functions::toQStringList(ChannelOrderMapper.getNames()));
  if (auto index = ChannelOrderMapper.indexOf(rgbFormat.getChannelOrder()))
    this->ui.rgbOrderComboBox->setCurrentIndex(int(index));

  this->ui.bitDepthSpinBox->setValue(rgbFormat.getBitsPerSample());
  this->ui.comboBoxEndianness->setCurrentIndex(rgbFormat.getEndianess() == Endianness::Big ? 0 : 1);

  this->ui.planarCheckBox->setChecked(false);
  this->ui.checkBoxBytePacking->setChecked(false);

  this->updateAlphaPosComboBox();
  this->updatePaddingPosComboBox();

  this->ui.comboBoxAlphaPos->setCurrentIndex(int(rgbFormat.getAlphaMode()));
  this->ui.comboBoxPaddingPos->setCurrentIndex(int(rgbFormat.getPaddingInfo()));

  this->ui.groupBoxDiffCompDepth->setChecked(false);
  this->ui.comboBoxDiffType->addItems(functions::toQStringList(DiffCompDepthTypeMapper.getNames()));
  if (auto index = DiffCompDepthTypeMapper.indexOf(rgbFormat.getDiffCompType()))
    this->ui.comboBoxDiffType->setCurrentIndex(int(index));

  this->updateControlsEnabledState();

  auto updateUiControls = [this]()
  {
    this->updateControlsEnabledState();
    emit formatChanged();
  };

  connect(this->ui.rgbOrderComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.bitDepthSpinBox,
          QOverload<int>::of(&QSpinBox::valueChanged),
          this,
          updateUiControls);
  connect(this->ui.comboBoxEndianness,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.planarCheckBox,
          QOverload<int>::of(&QCheckBox::stateChanged),
          this,
          updateUiControls);
  connect(this->ui.checkBoxBytePacking,
          QOverload<int>::of(&QCheckBox::stateChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxAlphaPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_comboBoxAlphaPos_currentIndexChanged);
  connect(this->ui.comboBoxPaddingPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_comboBoxPaddingPos_currentIndexChanged);
  connect(this->ui.groupBoxDiffCompDepth,
          &QGroupBox::toggled,
          this,
          &videoHandlerRGBCustomFormatDialog::on_groupBoxDiffCompDepth_toggled);
  connect(this->ui.comboBoxDiffType,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          updateUiControls);
}

PixelFormatRGB videoHandlerRGBCustomFormatDialog::getSelectedRGBFormat() const
{
  // get order
  ChannelOrder channelOrder = ChannelOrder::RGB;
  if (this->ui.rgbOrderComboBox->currentIndex() >= 0)
  {
    auto currentText = this->ui.rgbOrderComboBox->currentText();
    // remove comment: (MSB to LSB) / (LSB to MSB)
    // currentText = currentText.split(" (").first();
    if (auto order = ChannelOrderMapper.getValue(currentText.toStdString()))
      channelOrder = *order;
  }

  // get alpha mode
  AlphaMode alphaMode = AlphaMode::None;
  auto alphaPosIndex = this->ui.comboBoxAlphaPos->currentIndex();
  if (alphaPosIndex == 1)
    alphaMode = AlphaMode::First;
  else if (alphaPosIndex == 2)
    alphaMode = AlphaMode::Last;

  // get padding info
  PaddingInfo paddingInfo = PaddingInfo::NoPadding;
  auto paddingPosIndex = this->ui.comboBoxPaddingPos->currentIndex();
  if (paddingPosIndex == 1)
    paddingInfo = PaddingInfo::PaddingInMSB;
  else if (paddingPosIndex == 2)
    paddingInfo = PaddingInfo::PaddingInLSB;

  // get other info
  auto endianness =
    this->ui.comboBoxEndianness->currentIndex() == 0 ? Endianness::Big : Endianness::Little;
  auto bitsPerSample = unsigned(this->ui.bitDepthSpinBox->value());
  auto dataLayout = this->ui.planarCheckBox->isChecked() ? DataLayout::Planar : DataLayout::Interleaved;
  auto bytePacking = this->ui.checkBoxBytePacking->isChecked();

  /* diff comp depth case */
  if (this->ui.groupBoxDiffCompDepth->isChecked())
  {
    auto diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
    auto diffType = DiffCompDepthTypeMapper.getValueAt(static_cast<std::size_t>(diffTypeIndex));
    if (diffType && *diffType != DiffCompDepthType::None)
      return PixelFormatRGB(*diffType, channelOrder, alphaMode, paddingInfo, endianness);
  }

  /* none diff comp depth case */
  return PixelFormatRGB(
      bitsPerSample, dataLayout, channelOrder, alphaMode, endianness, paddingInfo, bytePacking, DiffCompDepthType::None);
}

void videoHandlerRGBCustomFormatDialog::updateControlsEnabledState()
{
  bool isDiffCompDepth = this->ui.groupBoxDiffCompDepth->isChecked() &&
                         this->ui.comboBoxDiffType->currentIndex() > 0;

  if (isDiffCompDepth)
  {
    const int  diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
    const auto diffType =
      DiffCompDepthTypeMapper.getValueAt(static_cast<std::size_t>(diffTypeIndex));
    const bool hasAlphaAndPadding = (*diffType == DiffCompDepthType::BPP16_RGBA5551 ||
                                     *diffType == DiffCompDepthType::BPP32_RGBA1010102);
    const int  bps4DiffTypeMap[]  = {0, 3, 6, 5, 10};
    const int  bitsPerSample      = bps4DiffTypeMap[diffTypeIndex];

    {
      QSignalBlocker blockerAlpha(this->ui.rgbOrderComboBox);
      this->ui.rgbOrderComboBox->setCurrentIndex(0); // RGB order
      this->ui.rgbOrderComboBox->setEnabled(false);
    }

    {
      QSignalBlocker blockerAlpha(this->ui.bitDepthSpinBox);
      this->ui.bitDepthSpinBox->setValue(bitsPerSample);
      this->ui.bitDepthSpinBox->setEnabled(false);
    }

    this->ui.comboBoxAlphaPos->setEnabled(hasAlphaAndPadding);
    this->ui.comboBoxPaddingPos->setEnabled(hasAlphaAndPadding);
    if (!hasAlphaAndPadding)
    {
      QSignalBlocker blockerPad(this->ui.comboBoxPaddingPos);
      QSignalBlocker blockerAlpha(this->ui.comboBoxAlphaPos);
      this->ui.comboBoxPaddingPos->setCurrentIndex(0);
      this->ui.comboBoxAlphaPos->setCurrentIndex(0);
    }

    {
      QSignalBlocker blockerPad(this->ui.checkBoxBytePacking);
      this->ui.checkBoxBytePacking->setChecked(true);
      this->ui.checkBoxBytePacking->setEnabled(false);
    }

    {
      QSignalBlocker blockerPad(this->ui.planarCheckBox);
      this->ui.planarCheckBox->setChecked(false);
      this->ui.planarCheckBox->setEnabled(false);
    }
  }
  else
  {
    const int  bitsPerSample     = this->ui.bitDepthSpinBox->value();
    const bool isNonByteMultiple = (bitsPerSample % 8) != 0;
    const bool isPlanarChecked   = this->ui.planarCheckBox->isChecked();

    this->ui.comboBoxEndianness->setEnabled(bitsPerSample > 8);

    this->ui.rgbOrderComboBox->setEnabled(true);
    this->ui.bitDepthSpinBox->setEnabled(true);
    this->ui.comboBoxAlphaPos->setEnabled(true);
    this->ui.comboBoxPaddingPos->setEnabled(isNonByteMultiple);
    this->ui.planarCheckBox->setEnabled(true);
    this->ui.checkBoxBytePacking->setEnabled(isNonByteMultiple);
    this->ui.checkBoxBytePacking->setChecked(false);

    this->ui.groupBoxDiffCompDepth->setEnabled(!isPlanarChecked);
    if (isPlanarChecked)
    {
      QSignalBlocker blocker(this->ui.groupBoxDiffCompDepth);
      this->ui.groupBoxDiffCompDepth->setChecked(false);
    }
  }
}

void videoHandlerRGBCustomFormatDialog::updateAlphaPosComboBox()
{
  QSignalBlocker blocker(this->ui.comboBoxAlphaPos);
  this->ui.comboBoxAlphaPos->clear();
  this->ui.comboBoxAlphaPos->addItem("NoAlpha");
  this->ui.comboBoxAlphaPos->addItem("First (InLsb)");
  this->ui.comboBoxAlphaPos->addItem("Last (InMsb)");
  this->ui.comboBoxAlphaPos->setCurrentIndex(0);
}

void videoHandlerRGBCustomFormatDialog::updatePaddingPosComboBox()
{
  QSignalBlocker blocker(this->ui.comboBoxPaddingPos);
  this->ui.comboBoxPaddingPos->clear();
  this->ui.comboBoxPaddingPos->addItem("NoPadding");
  this->ui.comboBoxPaddingPos->addItem("PaddingOnMsb");
  this->ui.comboBoxPaddingPos->addItem("PaddingOnLsb");
  this->ui.comboBoxPaddingPos->setCurrentIndex(0);
}

void videoHandlerRGBCustomFormatDialog::updateDiffTypeComboBox()
{
  QSignalBlocker blocker(this->ui.comboBoxDiffType);

  this->ui.comboBoxDiffType->clear();
  for (auto diffType : DiffCompDepthTypeMapper.getValues())
  {
    if (diffType != DiffCompDepthType::None)
      this->ui.comboBoxDiffType->addItem(
        QString::fromStdString(std::string(DiffCompDepthTypeMapper.getName(diffType))));
  }
}

void videoHandlerRGBCustomFormatDialog::on_groupBoxDiffCompDepth_toggled(bool checked)
{
  if (!checked)
  {
    QSignalBlocker blocker(this->ui.comboBoxDiffType);
    this->ui.comboBoxDiffType->setCurrentIndex(0);
  }

  if (checked && this->ui.planarCheckBox->isChecked())
  {
    QSignalBlocker blocker(this->ui.planarCheckBox);
    this->ui.planarCheckBox->setChecked(false);
  }

  this->updateControlsEnabledState();
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::on_comboBoxAlphaPos_currentIndexChanged(int index)
{
  bool isDiffCompDepth = this->ui.groupBoxDiffCompDepth->isChecked() &&
                        this->ui.comboBoxDiffType->currentIndex() > 0;
  if (!isDiffCompDepth)
    return;

  int diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
  bool hasAlphaAndPadding = (diffTypeIndex >= 3 && diffTypeIndex <= 4);
  if (!hasAlphaAndPadding)
    return;

  if (index != 0)
  {
    QSignalBlocker blockerPad(this->ui.comboBoxPaddingPos);
    this->ui.comboBoxPaddingPos->setCurrentIndex(0);
  }
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::on_comboBoxPaddingPos_currentIndexChanged(int index)
{
  bool isDiffCompDepth = this->ui.groupBoxDiffCompDepth->isChecked() &&
                        this->ui.comboBoxDiffType->currentIndex() > 0;
  if (!isDiffCompDepth)
    return;

  int diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
  bool hasAlphaAndPadding = (diffTypeIndex >= 3 && diffTypeIndex <= 4);
  if (!hasAlphaAndPadding)
    return;

  if (index != 0)
  {
    QSignalBlocker blockerAlpha(this->ui.comboBoxAlphaPos);
    this->ui.comboBoxAlphaPos->setCurrentIndex(0);
  }
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::updateFormatNameLabel()
{
  auto rgbFormat = this->getSelectedRGBFormat();
  std::string formatName = rgbFormat.getName();
  this->ui.labelRgbFmtName->setText(QString::fromStdString(formatName));
}

void videoHandlerRGBCustomFormatDialog::slotUpdateFormatAndUi(const PixelFormatRGB &newFormat)
{
  // Block formatChanged signal during update to avoid unnecessary emissions
  QSignalBlocker blocker(this);

  // 1. BytePacking + DiffCompDepth 类格式
  if (newFormat.getDiffCompType() != DiffCompDepthType::None)
  {
    // Update groupBoxDiffCompDepth
    this->ui.groupBoxDiffCompDepth->setChecked(true);

    // Update diff type combo box
    size_t diffTypeIndex = DiffCompDepthTypeMapper.indexOf(newFormat.getDiffCompType());
    if (diffTypeIndex)
      this->ui.comboBoxDiffType->setCurrentIndex(int(diffTypeIndex) - 1);

    // Update bitDepthSpinBox - disabled, set according to DiffCompType
    this->ui.bitDepthSpinBox->setEnabled(false);
    int bitsPerSample = 0;
    switch (newFormat.getDiffCompType())
    {
    case DiffCompDepthType::BPP8_RGB332:
      bitsPerSample = 3;
      break;
    case DiffCompDepthType::BPP16_RGB565:
      bitsPerSample = 6;
      break;
    case DiffCompDepthType::BPP16_RGBA5551:
      bitsPerSample = 5;
      break;
    case DiffCompDepthType::BPP32_RGBA1010102:
      bitsPerSample = 10;
      break;
    default:
      break;
    }
    this->ui.bitDepthSpinBox->setValue(bitsPerSample);

    // Update rgbOrderComboBox - enabled
    this->ui.rgbOrderComboBox->setEnabled(true);
    if (auto index = ChannelOrderMapper.indexOf(newFormat.getChannelOrder()))
      this->ui.rgbOrderComboBox->setCurrentIndex(int(index));

    // Update comboBoxEndianness
    bool isRGB332 = (newFormat.getDiffCompType() == DiffCompDepthType::BPP8_RGB332);
    this->ui.comboBoxEndianness->setEnabled(!isRGB332);
    this->ui.comboBoxEndianness->setCurrentIndex(newFormat.getEndianess() == Endianness::Big ? 0 : 1);

    // Update planarCheckBox - disabled, set to false
    this->ui.planarCheckBox->setEnabled(false);
    this->ui.planarCheckBox->setChecked(false);

    // Update checkBoxBytePacking - disabled, set to true
    this->ui.checkBoxBytePacking->setEnabled(false);
    this->ui.checkBoxBytePacking->setChecked(true);

    // Update comboBoxAlphaPos and comboBoxPaddingPos
    bool hasAlphaAndPadding = (newFormat.getDiffCompType() == DiffCompDepthType::BPP16_RGBA5551 ||
                              newFormat.getDiffCompType() == DiffCompDepthType::BPP32_RGBA1010102);
    if (hasAlphaAndPadding)
    {
      // Update comboBoxAlphaPos - enabled
      this->ui.comboBoxAlphaPos->setEnabled(true);
      this->ui.comboBoxAlphaPos->setCurrentIndex(int(newFormat.getAlphaMode()));

      // Update comboBoxPaddingPos - enabled if alpha is None
      if (newFormat.getAlphaMode() == AlphaMode::None)
        this->ui.comboBoxPaddingPos->setEnabled(true);
      else
      {
        this->ui.comboBoxPaddingPos->setEnabled(false);
        this->ui.comboBoxPaddingPos->setCurrentIndex(0); // NoPadding
      }
      this->ui.comboBoxPaddingPos->setCurrentIndex(int(newFormat.getPaddingInfo()));
    }
    else
    {
      // Update comboBoxAlphaPos - disabled, set to NoAlpha
      this->ui.comboBoxAlphaPos->setEnabled(false);
      this->ui.comboBoxAlphaPos->setCurrentIndex(0); // NoAlpha

      // Update comboBoxPaddingPos - disabled, set to NoPadding
      this->ui.comboBoxPaddingPos->setEnabled(false);
      this->ui.comboBoxPaddingPos->setCurrentIndex(0); // NoPadding
    }
  }
  // 2. 其他 BytePacking 格式
  else if (newFormat.isBytePacking())
  {
    // Update groupBoxDiffCompDepth - not checked
    this->ui.groupBoxDiffCompDepth->setChecked(false);

    // Update checkBoxBytePacking - checked
    this->ui.checkBoxBytePacking->setChecked(true);

    // Update bitDepthSpinBox - enabled
    this->ui.bitDepthSpinBox->setEnabled(true);
    this->ui.bitDepthSpinBox->setValue(newFormat.getBitsPerSample());

    // Update rgbOrderComboBox - enabled
    this->ui.rgbOrderComboBox->setEnabled(true);
    if (auto index = ChannelOrderMapper.indexOf(newFormat.getChannelOrder()))
      this->ui.rgbOrderComboBox->setCurrentIndex(int(index));

    // Update comboBoxEndianness - enabled
    this->ui.comboBoxEndianness->setEnabled(true);
    this->ui.comboBoxEndianness->setCurrentIndex(newFormat.getEndianess() == Endianness::Big ? 0 : 1);

    // Update planarCheckBox - enabled
    this->ui.planarCheckBox->setEnabled(true);
    this->ui.planarCheckBox->setChecked(newFormat.getDataLayout() == DataLayout::Planar);

    // Update comboBoxAlphaPos - enabled
    this->ui.comboBoxAlphaPos->setEnabled(true);
    this->ui.comboBoxAlphaPos->setCurrentIndex(int(newFormat.getAlphaMode()));

    // Update comboBoxPaddingPos - disabled, set to NoPadding
    this->ui.comboBoxPaddingPos->setEnabled(false);
    this->ui.comboBoxPaddingPos->setCurrentIndex(0); // NoPadding
  }
  // 3. 普通格式（非 BytePacking）
  else
  {
    // Update groupBoxDiffCompDepth - not checked
    this->ui.groupBoxDiffCompDepth->setChecked(false);

    // Update checkBoxBytePacking - not checked
    this->ui.checkBoxBytePacking->setChecked(false);

    // Update bitDepthSpinBox - enabled
    this->ui.bitDepthSpinBox->setEnabled(true);
    this->ui.bitDepthSpinBox->setValue(newFormat.getBitsPerSample());

    // Update rgbOrderComboBox - enabled
    this->ui.rgbOrderComboBox->setEnabled(true);
    if (auto index = ChannelOrderMapper.indexOf(newFormat.getChannelOrder()))
      this->ui.rgbOrderComboBox->setCurrentIndex(int(index));

    // Update comboBoxEndianness
    this->ui.comboBoxEndianness->setEnabled(newFormat.getBitsPerSample() > 8);
    this->ui.comboBoxEndianness->setCurrentIndex(newFormat.getEndianess() == Endianness::Big ? 0 : 1);

    // Update planarCheckBox - enabled
    this->ui.planarCheckBox->setEnabled(true);
    this->ui.planarCheckBox->setChecked(newFormat.getDataLayout() == DataLayout::Planar);

    // Update comboBoxAlphaPos - enabled
    this->ui.comboBoxAlphaPos->setEnabled(true);
    this->ui.comboBoxAlphaPos->setCurrentIndex(int(newFormat.getAlphaMode()));

    // Update comboBoxPaddingPos
    this->ui.comboBoxPaddingPos->setEnabled((newFormat.getBitsPerSample() % 8) != 0);
    this->ui.comboBoxPaddingPos->setCurrentIndex(int(newFormat.getPaddingInfo()));
  }

  // Update controls state and format name
  // this->updateControlsState();
  this->updateFormatNameLabel();

  // Update RGB order label note
  bool isBytePacking = (newFormat.getDiffCompType() != DiffCompDepthType::None) || newFormat.isBytePacking();
  QString note = isBytePacking ? "(M2L)" : "(L2M)";
  this->ui.labelRGBOrder->setText("RGB Order " + note);
}

} // namespace video::rgb