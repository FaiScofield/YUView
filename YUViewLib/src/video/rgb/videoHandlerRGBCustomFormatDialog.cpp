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
  this->updateDiffTypeComboBox();
  if (auto index = DiffCompDepthTypeMapper.indexOf(rgbFormat.getDiffCompType()))
    this->ui.comboBoxDiffType->setCurrentIndex(int(index));

  this->updateControlsEnabledState();

  connect(this->ui.rgbOrderComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.bitDepthSpinBox,
          QOverload<int>::of(&QSpinBox::valueChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_bitDepthSpinBox_valueChanged);
  connect(this->ui.comboBoxEndianness,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.planarCheckBox,
          &QCheckBox::stateChanged,
          this,
          &videoHandlerRGBCustomFormatDialog::on_planarCheckBox_stateChanged);
  connect(this->ui.checkBoxBytePacking,
          &QCheckBox::stateChanged,
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxAlphaPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.comboBoxPaddingPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::formatChanged);
  connect(this->ui.groupBoxDiffCompDepth,
          &QGroupBox::toggled,
          this,
          &videoHandlerRGBCustomFormatDialog::on_groupBoxDiffCompDepth_toggled);
  connect(this->ui.comboBoxDiffType,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_comboBoxDiffType_currentIndexChanged);
  connect(this->ui.comboBoxAlphaPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_comboBoxAlphaPos_currentIndexChanged);
  connect(this->ui.comboBoxPaddingPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          &videoHandlerRGBCustomFormatDialog::on_comboBoxPaddingPos_currentIndexChanged);
}

PixelFormatRGB videoHandlerRGBCustomFormatDialog::getSelectedRGBFormat() const
{
  const auto channelOrderIndex = this->ui.rgbOrderComboBox->currentIndex();

  if (this->ui.groupBoxDiffCompDepth->isChecked())
  {
    auto diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
    if (diffTypeIndex >= 0)
    {
      auto diffType = DiffCompDepthTypeMapper.getValueAt(static_cast<std::size_t>(diffTypeIndex));
      if (diffType != DiffCompDepthType::None)
      {
        auto channelOrder = ChannelOrderMapper.getValueAt(static_cast<std::size_t>(channelOrderIndex));
        auto endianness =
            this->ui.comboBoxEndianness->currentIndex() == 0 ? Endianness::Big : Endianness::Little;
        auto alphaModeIndex = this->ui.comboBoxAlphaPos->currentIndex();
        auto alphaMode = AlphaModeMapper.getValueAt(static_cast<std::size_t>(alphaModeIndex));
        return PixelFormatRGB(*diffType, *channelOrder, *alphaMode, endianness);
      }
    } else return {};
  }

  auto channelOrder = ChannelOrderMapper.getValueAt(static_cast<std::size_t>(channelOrderIndex));
  if (!channelOrder)
    return {};

  auto bitsPerSample = unsigned(this->ui.bitDepthSpinBox->value());

  auto endianness =
      this->ui.comboBoxEndianness->currentIndex() == 0 ? Endianness::Big : Endianness::Little;

  auto dataLayout = this->ui.planarCheckBox->isChecked() ? DataLayout::Planar : DataLayout::Interleaved;

  auto bytePacking = this->ui.checkBoxBytePacking->isChecked();

  AlphaMode alphaMode = AlphaMode::None;
  auto alphaPosIndex = this->ui.comboBoxAlphaPos->currentIndex();
  if (alphaPosIndex == 1)
    alphaMode = AlphaMode::First;
  else if (alphaPosIndex == 2)
    alphaMode = AlphaMode::Last;

  PaddingInfo paddingInfo = PaddingInfo::NoPadding;
  auto paddingPosIndex = this->ui.comboBoxPaddingPos->currentIndex();
  if (paddingPosIndex == 1)
    paddingInfo = PaddingInfo::PaddingInMSB;
  else if (paddingPosIndex == 2)
    paddingInfo = PaddingInfo::PaddingInLSB;

  return PixelFormatRGB(
      bitsPerSample, dataLayout, *channelOrder, alphaMode, endianness, paddingInfo, bytePacking, DiffCompDepthType::None);
}

void videoHandlerRGBCustomFormatDialog::updateControlsEnabledState()
{
  bool isDiffCompDepth = this->ui.groupBoxDiffCompDepth->isChecked() &&
                         this->ui.comboBoxDiffType->currentIndex() > 0;

  int bitsPerSample = this->ui.bitDepthSpinBox->value();
  bool isNonByteMultiple = (bitsPerSample % 8) != 0;
  bool isPlanarChecked = this->ui.planarCheckBox->isChecked();

  if (isDiffCompDepth)
  {
    this->ui.rgbOrderComboBox->setEnabled(false);
    this->ui.bitDepthSpinBox->setEnabled(false);
    this->ui.planarCheckBox->setEnabled(false);
    this->ui.planarCheckBox->setChecked(false);
    this->ui.checkBoxBytePacking->setEnabled(false);
    this->ui.checkBoxBytePacking->setChecked(true);

    this->ui.groupBoxDiffCompDepth->setEnabled(true);

    int diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
    bool hasAlphaAndPadding = (diffTypeIndex >= 3 && diffTypeIndex <= 4);

    if (hasAlphaAndPadding)
    {
      this->ui.comboBoxAlphaPos->setEnabled(true);
      this->ui.comboBoxPaddingPos->setEnabled(true);

      int alphaPos = this->ui.comboBoxAlphaPos->currentIndex();
      int paddingPos = this->ui.comboBoxPaddingPos->currentIndex();

      if (alphaPos != 0)
      {
        QSignalBlocker blockerPad(this->ui.comboBoxPaddingPos);
        this->ui.comboBoxPaddingPos->setCurrentIndex(0);
      }
      if (paddingPos != 0)
      {
        QSignalBlocker blockerAlpha(this->ui.comboBoxAlphaPos);
        this->ui.comboBoxAlphaPos->setCurrentIndex(0);
      }
    }
    else
    {
      QSignalBlocker blockerAlpha(this->ui.comboBoxAlphaPos);
      QSignalBlocker blockerPad(this->ui.comboBoxPaddingPos);
      this->ui.comboBoxAlphaPos->setCurrentIndex(0);
      this->ui.comboBoxPaddingPos->setCurrentIndex(0);
      this->ui.comboBoxAlphaPos->setEnabled(false);
      this->ui.comboBoxPaddingPos->setEnabled(false);
    }
  }
  else
  {
    this->ui.rgbOrderComboBox->setEnabled(!isPlanarChecked);
    this->ui.bitDepthSpinBox->setEnabled(!isPlanarChecked);

    this->ui.groupBoxDiffCompDepth->setEnabled(!isPlanarChecked);
    if (isPlanarChecked)
    {
      QSignalBlocker blocker(this->ui.groupBoxDiffCompDepth);
      this->ui.groupBoxDiffCompDepth->setChecked(false);
    }

    this->ui.planarCheckBox->setEnabled(true);

    this->ui.checkBoxBytePacking->setEnabled(!isPlanarChecked);
    if (isPlanarChecked)
    {
      QSignalBlocker blockerBP(this->ui.checkBoxBytePacking);
      this->ui.checkBoxBytePacking->setChecked(false);
    }

    this->ui.comboBoxAlphaPos->setEnabled(!isPlanarChecked);

    bool paddingEnabled = isNonByteMultiple || !isPlanarChecked;
    this->ui.comboBoxPaddingPos->setEnabled(paddingEnabled);
    if (!paddingEnabled)
    {
      QSignalBlocker blocker(this->ui.comboBoxPaddingPos);
      this->ui.comboBoxPaddingPos->setCurrentIndex(0);
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

  this->updateDiffTypeComboBox();
  this->updateControlsEnabledState();
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::on_bitDepthSpinBox_valueChanged(int)
{
  this->updateControlsEnabledState();
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::on_planarCheckBox_stateChanged(int)
{
  this->updateControlsEnabledState();
  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::on_comboBoxDiffType_currentIndexChanged(int)
{
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

} // namespace video::rgb