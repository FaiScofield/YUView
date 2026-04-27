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
#include "common/Functions.h"

#include <QTimer>

namespace video::rgb
{

videoHandlerRGBCustomFormatDialog::videoHandlerRGBCustomFormatDialog(
    const PixelFormatRGB &rgbFormat, QWidget *parent)
    : QWidget(parent)
{
  this->ui.setupUi(this);

  this->ui.rgbOrderComboBox->addItems(functions::toQStringList(ChannelOrderMapper.getNames()));
  this->updateDiffTypeComboBox();
  this->ui.comboBoxDiffType->setCurrentIndex(1); // Default: BPP16_RGB565

  this->updateAlphaPosComboBox();
  this->updatePaddingPosComboBox();

  // 直接调用 updateUiFromFormat，传入新格式，会同时更新 currentFormat 和 UI
  this->updateUiFromFormat(rgbFormat);

  auto onUiControlChanged = [this]()
  {
    if (this->updatingUiFromFormat || this->ignoreUiChanges)
      return;

    // 使用 QTimer::singleShot 确保所有 UI 控件联动变化完成后再处理
    QTimer::singleShot(0, this, &videoHandlerRGBCustomFormatDialog::onUiControlsChanged);
  };


  // 全部使用统一的连接，避免重复触发
  connect(this->ui.rgbOrderComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.bitDepthSpinBox,
          QOverload<int>::of(&QSpinBox::valueChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxEndianness,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.planarCheckBox,
          QOverload<int>::of(&QCheckBox::stateChanged),
          this,
          onUiControlChanged);
  connect(this->ui.checkBoxBytePacking,
          QOverload<int>::of(&QCheckBox::stateChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxAlphaPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxPaddingPos,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.groupBoxDiffCompDepth,
          &QGroupBox::toggled,
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxDiffType,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
}

PixelFormatRGB videoHandlerRGBCustomFormatDialog::getSelectedRGBFormat() const
{
  // get order
  ChannelOrder channelOrder = ChannelOrder::RGB;
  if (this->ui.rgbOrderComboBox->currentIndex() >= 0)
  {
    auto currentText = this->ui.rgbOrderComboBox->currentText();
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
    auto diffType = DiffCompDepthTypeMapper.getValueAt(static_cast<std::size_t>(diffTypeIndex + 1));
    if (diffType && *diffType != DiffCompDepthType::None)
    {
      // For RGBA5551/RGBA1010102, at least one of alpha and padding must be None
      if (*diffType == DiffCompDepthType::BPP16_RGBA5551 || *diffType == DiffCompDepthType::BPP32_RGBA1010102)
      {
        if (alphaMode == AlphaMode::None && paddingInfo == PaddingInfo::NoPadding)
          alphaMode = AlphaMode::InLsb;
        else if (alphaMode != AlphaMode::None && paddingInfo != PaddingInfo::NoPadding)
          paddingInfo = PaddingInfo::NoPadding;
      }
      return PixelFormatRGB(*diffType, channelOrder, alphaMode, paddingInfo, endianness);
    }
  }

  /* none diff comp depth case */
  return PixelFormatRGB(
      bitsPerSample, dataLayout, channelOrder, alphaMode, endianness, paddingInfo, bytePacking, DiffCompDepthType::None);
}

void videoHandlerRGBCustomFormatDialog::updateControlsState()
{
  QSignalBlocker blocker(this);  // Block all signals during this function

  bool isDiffCompDepth = this->ui.groupBoxDiffCompDepth->isChecked();

  if (isDiffCompDepth)
  {
    const int  diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
    const auto diffType =
      DiffCompDepthTypeMapper.getValueAt(static_cast<std::size_t>(diffTypeIndex + 1));
    const bool hasAlphaAndPadding = (*diffType == DiffCompDepthType::BPP16_RGBA5551 ||
                                     *diffType == DiffCompDepthType::BPP32_RGBA1010102);
    const int  bps4DiffTypeMap[]  = {3, 6, 5, 10};
    const int  bitsPerSample      = bps4DiffTypeMap[diffTypeIndex];

    // rgbOrderComboBox 应该是启用的，根据文档要求
    this->ui.rgbOrderComboBox->setEnabled(true);

    this->ui.bitDepthSpinBox->setValue(bitsPerSample);
    this->ui.bitDepthSpinBox->setEnabled(false);

    if (hasAlphaAndPadding)
    {
      // For RGBA5551/RGBA1010102, at least one of alpha and padding must be None
      int  alphaIdx  = this->ui.comboBoxAlphaPos->currentIndex();
      int  paddingIdx = this->ui.comboBoxPaddingPos->currentIndex();
      bool alphaSelected = alphaIdx != 0;
      bool paddingSelected = paddingIdx != 0;

      if (!alphaSelected && !paddingSelected)
      {
        QSignalBlocker blockerAlpha(this->ui.comboBoxAlphaPos);
        this->ui.comboBoxAlphaPos->setCurrentIndex(1); // InLsb
        alphaIdx      = 1;
        alphaSelected = true;
      }
      else if (alphaSelected && paddingSelected)
      {
        QSignalBlocker blockerPadding(this->ui.comboBoxPaddingPos);
        this->ui.comboBoxPaddingPos->setCurrentIndex(0); // NoPadding
        paddingIdx      = 0;
        paddingSelected = false;
      }

      // Mutual exclusivity: each is only enabled when the other is at index 0
      this->ui.comboBoxAlphaPos->setEnabled(paddingIdx == 0);
      this->ui.comboBoxPaddingPos->setEnabled(alphaIdx == 0);
    }
    else
    {
      // For RGB332/RGB565, no alpha and padding are allowed
      this->ui.comboBoxAlphaPos->setEnabled(false);
      this->ui.comboBoxPaddingPos->setEnabled(false);
      this->ui.comboBoxPaddingPos->setCurrentIndex(0);
      this->ui.comboBoxAlphaPos->setCurrentIndex(0);
    }

    this->ui.checkBoxBytePacking->setChecked(true);
    this->ui.checkBoxBytePacking->setEnabled(false);

    this->ui.planarCheckBox->setChecked(false);
    this->ui.planarCheckBox->setEnabled(false);
  }
  else if (this->ui.checkBoxBytePacking->isChecked())
  {
    // 2. Other BytePacking format
    const int  bitsPerSample     = this->ui.bitDepthSpinBox->value();
    const bool isPlanarChecked   = this->ui.planarCheckBox->isChecked();

    this->ui.comboBoxEndianness->setEnabled(bitsPerSample > 8);

    this->ui.rgbOrderComboBox->setEnabled(true);
    this->ui.bitDepthSpinBox->setEnabled(true);
    this->ui.comboBoxAlphaPos->setEnabled(true);
    this->ui.comboBoxPaddingPos->setEnabled(false);
    this->ui.comboBoxPaddingPos->setCurrentIndex(0);
    this->ui.planarCheckBox->setEnabled(true);
    this->ui.checkBoxBytePacking->setEnabled(true);

    this->ui.groupBoxDiffCompDepth->setEnabled(!isPlanarChecked);
    if (isPlanarChecked)
    {
      this->ui.groupBoxDiffCompDepth->setChecked(false);
    }
  }
  else
  {
    // 3. Normal format (non BytePacking)
    const int  bitsPerSample     = this->ui.bitDepthSpinBox->value();
    const bool isNonByteMultiple = (bitsPerSample % 8) != 0;
    const bool isPlanarChecked   = this->ui.planarCheckBox->isChecked();

    this->ui.comboBoxEndianness->setEnabled(bitsPerSample > 8);

    this->ui.rgbOrderComboBox->setEnabled(true);
    this->ui.bitDepthSpinBox->setEnabled(true);
    this->ui.comboBoxAlphaPos->setEnabled(true);
    this->ui.comboBoxPaddingPos->setEnabled(isNonByteMultiple);
    this->ui.planarCheckBox->setEnabled(true);
    this->ui.checkBoxBytePacking->setEnabled(true);

    this->ui.groupBoxDiffCompDepth->setEnabled(!isPlanarChecked);
    if (isPlanarChecked)
    {
      this->ui.groupBoxDiffCompDepth->setChecked(false);
    }
  }
}

void videoHandlerRGBCustomFormatDialog::updateAlphaPosComboBox()
{
  QSignalBlocker blocker(this);
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

void videoHandlerRGBCustomFormatDialog::updateFormatNameLabel()
{
  std::string formatName = this->currentFormat.getName();
  this->ui.labelRgbFmtName->setText(QString::fromStdString(formatName));
}

void videoHandlerRGBCustomFormatDialog::onUiControlsChanged()
{
  if (this->updatingUiFromFormat || this->ignoreUiChanges)
    return;

  this->ignoreUiChanges = true;

  // 处理 groupBoxDiffCompDepth 的联动逻辑
  if (this->ui.groupBoxDiffCompDepth->isChecked() && this->ui.planarCheckBox->isChecked())
  {
    QSignalBlocker blocker(this->ui.planarCheckBox);
    this->ui.planarCheckBox->setChecked(false);
  }

  // Handle mutual exclusivity between alpha and padding for DiffCompDepth formats
  bool isDiffCompDepth = this->ui.groupBoxDiffCompDepth->isChecked();
  if (isDiffCompDepth)
  {
    // At least Alpha or Padding is required for RGBA5551/RGBA1010102
    int diffTypeIndex = this->ui.comboBoxDiffType->currentIndex();
    bool hasAlphaAndPadding = (diffTypeIndex == 2 || diffTypeIndex == 3);
    if (hasAlphaAndPadding)
    {
      int alphaIndex = this->ui.comboBoxAlphaPos->currentIndex();
      int paddingIndex = this->ui.comboBoxPaddingPos->currentIndex();

      // Both zero: set Alpha InLsb
      if (alphaIndex == 0 && paddingIndex == 0)
      {
        QSignalBlocker blockerAlpha(this->ui.comboBoxAlphaPos);
        this->ui.comboBoxAlphaPos->setCurrentIndex(1); // InLsb
        alphaIndex = 1;
      }
      // Both non-zero: keep alpha, clear padding
      else if (alphaIndex != 0 && paddingIndex != 0)
      {
        QSignalBlocker blocker(this->ui.comboBoxPaddingPos);
        this->ui.comboBoxPaddingPos->setCurrentIndex(0);
        paddingIndex = 0;
      }
    }
  }

  this->ignoreUiChanges = false;

  // Update currentFormat and UI
  this->currentFormat = this->getSelectedRGBFormat();
  this->updateControlsState();
  this->updateFormatNameLabel();

  // Update labelRGBOrder based on bytePacking state
  bool isBytePacking = (this->currentFormat.getDiffCompType() != DiffCompDepthType::None) ||
                       this->currentFormat.isBytePacking();
  QString note = isBytePacking ? "(M2L)" : "(L2M)";
  this->ui.labelRGBOrder->setText("RGB Order " + note);

  emit formatChanged();
}

void videoHandlerRGBCustomFormatDialog::updateUiFromFormat(const PixelFormatRGB &newFormat)
{
  this->currentFormat = newFormat;
  this->updateUiFromFormat();
}

void videoHandlerRGBCustomFormatDialog::updateUiFromFormat()
{
  QSignalBlocker blocker(this);  // Block all signals from this widget and its children

  this->updatingUiFromFormat = true;

  const PixelFormatRGB &format = this->currentFormat;

  // 1. BytePacking + DiffCompDepth 类格式
  if (format.getDiffCompType() != DiffCompDepthType::None)
  {
    this->ui.groupBoxDiffCompDepth->setChecked(true);

    size_t diffTypeIndex = DiffCompDepthTypeMapper.indexOf(format.getDiffCompType());
    if (diffTypeIndex)
      this->ui.comboBoxDiffType->setCurrentIndex(int(diffTypeIndex) - 1);

    // Set control state based on the loaded format
    const int diffTypeIndexVal = this->ui.comboBoxDiffType->currentIndex();
    const int bps4DiffTypeMap[]  = {3, 6, 5, 10};
    const int bitsPerSample      = bps4DiffTypeMap[diffTypeIndexVal];

    this->ui.bitDepthSpinBox->setValue(bitsPerSample);
    this->ui.bitDepthSpinBox->setEnabled(false);

    this->ui.rgbOrderComboBox->setEnabled(true);
    if (auto index = ChannelOrderMapper.indexOf(format.getChannelOrder()))
      this->ui.rgbOrderComboBox->setCurrentIndex(int(index));

    bool isRGB332 = (format.getDiffCompType() == DiffCompDepthType::BPP8_RGB332);
    this->ui.comboBoxEndianness->setEnabled(!isRGB332);
    this->ui.comboBoxEndianness->setCurrentIndex(format.getEndianess() == Endianness::Big ? 0 : 1);

    this->ui.planarCheckBox->setEnabled(false);
    this->ui.planarCheckBox->setChecked(false);

    this->ui.checkBoxBytePacking->setEnabled(false);
    this->ui.checkBoxBytePacking->setChecked(true);

    bool hasAlphaAndPadding = (format.getDiffCompType() == DiffCompDepthType::BPP16_RGBA5551 ||
                              format.getDiffCompType() == DiffCompDepthType::BPP32_RGBA1010102);
    if (hasAlphaAndPadding)
    {
      bool alphaSet = format.getAlphaMode() != AlphaMode::None;
      bool paddingSet = format.getPaddingInfo() != PaddingInfo::NoPadding;

      // Mutual exclusivity: each enabled only when the other is NoPadding/NoAlpha
      this->ui.comboBoxAlphaPos->setEnabled(!paddingSet);
      this->ui.comboBoxAlphaPos->setCurrentIndex(alphaSet ? int(format.getAlphaMode()) : 0);

      this->ui.comboBoxPaddingPos->setEnabled(!alphaSet);
      this->ui.comboBoxPaddingPos->setCurrentIndex(paddingSet ? int(format.getPaddingInfo()) : 0);
    }
    else
    {
      this->ui.comboBoxAlphaPos->setEnabled(false);
      this->ui.comboBoxAlphaPos->setCurrentIndex(0);

      this->ui.comboBoxPaddingPos->setEnabled(false);
      this->ui.comboBoxPaddingPos->setCurrentIndex(0);
    }
  }
  // 2. 其他 BytePacking 格式
  else if (format.isBytePacking())
  {
    this->ui.groupBoxDiffCompDepth->setChecked(false);

    this->ui.checkBoxBytePacking->setChecked(true);

    this->ui.bitDepthSpinBox->setEnabled(true);
    this->ui.bitDepthSpinBox->setValue(format.getBitsPerSample());

    this->ui.rgbOrderComboBox->setEnabled(true);
    if (auto index = ChannelOrderMapper.indexOf(format.getChannelOrder()))
      this->ui.rgbOrderComboBox->setCurrentIndex(int(index));

    this->ui.comboBoxEndianness->setEnabled(true);
    this->ui.comboBoxEndianness->setCurrentIndex(format.getEndianess() == Endianness::Big ? 0 : 1);

    this->ui.planarCheckBox->setEnabled(true);
    this->ui.planarCheckBox->setChecked(format.getDataLayout() == DataLayout::Planar);

    this->ui.comboBoxAlphaPos->setEnabled(true);
    this->ui.comboBoxAlphaPos->setCurrentIndex(int(format.getAlphaMode()));

    this->ui.comboBoxPaddingPos->setEnabled(false);
    this->ui.comboBoxPaddingPos->setCurrentIndex(0);
  }
  // 3. 普通格式（非 BytePacking）
  else
  {
    this->ui.groupBoxDiffCompDepth->setChecked(false);

    this->ui.checkBoxBytePacking->setChecked(false);

    this->ui.bitDepthSpinBox->setEnabled(true);
    this->ui.bitDepthSpinBox->setValue(format.getBitsPerSample());

    this->ui.rgbOrderComboBox->setEnabled(true);
    if (auto index = ChannelOrderMapper.indexOf(format.getChannelOrder()))
      this->ui.rgbOrderComboBox->setCurrentIndex(int(index));

    this->ui.comboBoxEndianness->setEnabled(format.getBitsPerSample() > 8);
    this->ui.comboBoxEndianness->setCurrentIndex(format.getEndianess() == Endianness::Big ? 0 : 1);

    this->ui.planarCheckBox->setEnabled(true);
    this->ui.planarCheckBox->setChecked(format.getDataLayout() == DataLayout::Planar);

    this->ui.comboBoxAlphaPos->setEnabled(true);
    this->ui.comboBoxAlphaPos->setCurrentIndex(int(format.getAlphaMode()));

    this->ui.comboBoxPaddingPos->setEnabled((format.getBitsPerSample() % 8) != 0);
    this->ui.comboBoxPaddingPos->setCurrentIndex(int(format.getPaddingInfo()));
  }

  this->updateFormatNameLabel();

  bool isBytePacking = (format.getDiffCompType() != DiffCompDepthType::None) || format.isBytePacking();
  QString note = isBytePacking ? "(M2L)" : "(L2M)";
  this->ui.labelRGBOrder->setText("RGB Order " + note);

  this->updatingUiFromFormat = false;
}

} // namespace video::rgb