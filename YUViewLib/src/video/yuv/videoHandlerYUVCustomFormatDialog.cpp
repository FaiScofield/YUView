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
#include "common/Functions.h"

#include <QSignalBlocker>
#include <QTimer>

namespace video::yuv
{

videoHandlerYUVCustomFormatDialog::videoHandlerYUVCustomFormatDialog(
    const PixelFormatYUV &yuvFormat, QWidget *parent)
    : QWidget(parent)
{
  this->ui.setupUi(this);

  // Fill the comboBoxes with available options

  // Chroma subsampling
  this->ui.comboBoxChromaSubsampling->addItems(
    functions::toQStringList(SubsamplingMapper.getNames()));

  // Bit depth
  for (auto bitDepth : BitDepthList)
    this->ui.comboBoxBitDepth->addItem(QString("%1").arg(bitDepth));

  // Padding info is already populated in the UI file, do not add items here

  // Initialize UI from the given format
  this->updateUiFromFormat(yuvFormat);

  // Setup unified signal handling
  auto onUiControlChanged = [this]()
  {
    if (this->updatingUiFromFormat || this->ignoreUiChanges)
      return;

    // Use QTimer::singleShot to ensure all UI controls have finished their updates
    QTimer::singleShot(0, this, &videoHandlerYUVCustomFormatDialog::onUiControlsChanged);
  };

  // Connect all controls to the unified handler
  connect(this->ui.comboBoxChromaSubsampling,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxBitDepth,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxEndianness,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxChromaOffsetX,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxChromaOffsetY,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxElemOrder,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.comboBoxPaddingInfo,
          QOverload<int>::of(&QComboBox::currentIndexChanged),
          this,
          onUiControlChanged);
  connect(this->ui.checkBoxBytePacking,
          &QCheckBox::stateChanged,
          this,
          onUiControlChanged);
  connect(this->ui.radioButtonInterleaved,
          &QRadioButton::toggled,
          this,
          onUiControlChanged);
  connect(this->ui.radioButtonSemiPlanar,
          &QRadioButton::toggled,
          this,
          onUiControlChanged);
  connect(this->ui.radioButtonPlanar,
          &QRadioButton::toggled,
          this,
          onUiControlChanged);
}

void videoHandlerYUVCustomFormatDialog::updateComponentOrderComboBox()
{
  // Update element order combo box based on current layout selection
  DataLayout layout = DataLayout::Planar;
  if (this->ui.radioButtonInterleaved->isChecked())
    layout = DataLayout::Interleaved;
  else if (this->ui.radioButtonSemiPlanar->isChecked())
    layout = DataLayout::SemiPlanar;

  Subsampling subsampling =
    static_cast<Subsampling>(this->ui.comboBoxChromaSubsampling->currentIndex());

  // Check if YUV_400 subsampling
  bool isYUV400 = (subsampling == Subsampling::YUV_400);

  auto supportedOrders = getSupportedComponentOrders(subsampling, layout);

  // Block signals to prevent multiple formatChanged emissions
  QSignalBlocker blocker(this->ui.comboBoxElemOrder);

  // Check if we need to update the combo box items
  QStringList currentItems;
  for (int i = 0; i < this->ui.comboBoxElemOrder->count(); i++)
    currentItems.append(this->ui.comboBoxElemOrder->itemText(i));

  QStringList newItems;
  for (auto order : supportedOrders)
  {
    const auto name = ComponentOrderMapper.getName(order);
    newItems.append(QString::fromStdString(std::string(name)));
  }

  if (currentItems != newItems)
  {
    this->ui.comboBoxElemOrder->clear();
    for (auto order : supportedOrders)
    {
      const auto name = ComponentOrderMapper.getName(order);
      this->ui.comboBoxElemOrder->addItem(QString::fromStdString(std::string(name)));
    }
  }

  // Handle YUV_400 special case
  if (isYUV400)
  {
    this->ui.comboBoxElemOrder->setCurrentIndex(0);
    this->ui.comboBoxElemOrder->setEnabled(false);
  }
  else
  {
    this->ui.comboBoxElemOrder->setEnabled(true);
  }
}

void videoHandlerYUVCustomFormatDialog::updateControlsState()
{
  QSignalBlocker blocker(this);  // Block all signals during this function

  const auto subsamplingIndex = this->ui.comboBoxChromaSubsampling->currentIndex();
  const auto subsampling = static_cast<Subsampling>(subsamplingIndex);
  const auto bitDepthIndex = this->ui.comboBoxBitDepth->currentIndex();
  const auto bitsPerSample = BitDepthList.at(unsigned(bitDepthIndex));

  // Update chroma offset combo boxes based on subsampling
  auto maxValsX = getMaxPossibleChromaOffsetValues(true, subsampling);
  auto maxValsY = getMaxPossibleChromaOffsetValues(false, subsampling);

  // Rebuild chroma offset X combo box if needed
  QStringList offsetXItems;
  if (maxValsX >= 1)
    offsetXItems << "0" << "1/2";
  if (maxValsX >= 3)
    offsetXItems << "1" << "3/2";
  if (maxValsX >= 7)
    offsetXItems << "2" << "5/2" << "3" << "7/2";

  QStringList currentOffsetXItems;
  for (int i = 0; i < this->ui.comboBoxChromaOffsetX->count(); i++)
    currentOffsetXItems.append(this->ui.comboBoxChromaOffsetX->itemText(i));

  if (currentOffsetXItems != offsetXItems)
  {
    this->ui.comboBoxChromaOffsetX->clear();
    this->ui.comboBoxChromaOffsetX->addItems(offsetXItems);
  }

  // Rebuild chroma offset Y combo box if needed
  QStringList offsetYItems;
  if (maxValsY >= 1)
    offsetYItems << "0" << "1/2";
  if (maxValsY >= 3)
    offsetYItems << "1" << "3/2";
  if (maxValsY >= 7)
    offsetYItems << "2" << "5/2" << "3" << "7/2";

  QStringList currentOffsetYItems;
  for (int i = 0; i < this->ui.comboBoxChromaOffsetY->count(); i++)
    currentOffsetYItems.append(this->ui.comboBoxChromaOffsetY->itemText(i));

  if (currentOffsetYItems != offsetYItems)
  {
    this->ui.comboBoxChromaOffsetY->clear();
    this->ui.comboBoxChromaOffsetY->addItems(offsetYItems);
  }

  // Disable the combo boxes if there are no chroma components
  bool chromaPresent = (subsampling != Subsampling::YUV_400);
  this->ui.comboBoxChromaOffsetX->setEnabled(chromaPresent);
  this->ui.comboBoxChromaOffsetY->setEnabled(chromaPresent);

  // Disable interleaved if subsampling is 440/411/410/400
  if (subsampling == Subsampling::YUV_400 || subsampling == Subsampling::YUV_410 ||
      subsampling == Subsampling::YUV_411 || subsampling == Subsampling::YUV_440)
    this->ui.radioButtonInterleaved->setEnabled(false);
  else
    this->ui.radioButtonInterleaved->setEnabled(true);

  // Handle YUV_400 special case for layout buttons
  if (subsampling == Subsampling::YUV_400)
  {
    this->ui.radioButtonSemiPlanar->setEnabled(false);
    this->ui.radioButtonPlanar->setChecked(true);
  }
  else
  {
    this->ui.radioButtonSemiPlanar->setEnabled(true);
  }

  // Endianness only makes sense when the bit depth is > 8bit.
  const bool bitDepth8 = (bitsPerSample == 8);
  this->ui.comboBoxEndianness->setEnabled(!bitDepth8);

  // Byte packing only valid for bit depths that are not divisible by 8.
  const bool bytePackingEnabled = bitsPerSample % 8 > 0;
  this->ui.checkBoxBytePacking->setEnabled(bytePackingEnabled);

  // Padding info is relevant for bit depths that are not divisible by 8.
  const bool paddingInfoEnabled = bytePackingEnabled;
  this->ui.comboBoxPaddingInfo->setEnabled(paddingInfoEnabled);
  if (!paddingInfoEnabled)
    this->ui.comboBoxPaddingInfo->setCurrentIndex(0); // NoPadding

  // Update component order combo box after layout & subsampling are set
  this->updateComponentOrderComboBox();

  // Update format name label
  this->updateFormatNameLabel();
}

void videoHandlerYUVCustomFormatDialog::updateFormatNameLabel()
{
  std::string formatName = this->currentFormat.getName();
  this->ui.labelYuvFmtName->setText(QString::fromStdString(formatName));
}

void videoHandlerYUVCustomFormatDialog::onUiControlsChanged()
{
  if (this->updatingUiFromFormat || this->ignoreUiChanges)
    return;

  this->ignoreUiChanges = true;

  // Update currentFormat from UI
  this->currentFormat = this->getSelectedYUVFormat();

  // Update controls state based on current selection
  this->updateControlsState();

  // Update format name label
  this->updateFormatNameLabel();

  // Emit formatChanged signal
  emit formatChanged();

  // Reset ignoreUiChanges after all pending deferred UI change events have been processed
  QTimer::singleShot(0, this, [this]() { this->ignoreUiChanges = false; });
}

void videoHandlerYUVCustomFormatDialog::updateUiFromFormat(const PixelFormatYUV &newFormat)
{
  this->currentFormat = newFormat;
  this->updateUiFromFormat();
}

void videoHandlerYUVCustomFormatDialog::updateUiFromFormat()
{
  QSignalBlocker blocker(this);  // Block all signals from this widget and its children

  this->updatingUiFromFormat = true;

  const PixelFormatYUV &format = this->currentFormat;

  // Set subsampling
  if (format.getSubsampling() != Subsampling::UNKNOWN)
  {
    if (auto index = SubsamplingMapper.indexOf(format.getSubsampling()))
      this->ui.comboBoxChromaSubsampling->setCurrentIndex(int(index));
  }

  // Set bit depth
  {
    const auto idx = vectorIndexOf(BitDepthList, format.getBitsPerSample());
    this->ui.comboBoxBitDepth->setCurrentIndex(idx ? static_cast<int>(*idx) : 0);
  }

  // Set endianness
  this->ui.comboBoxEndianness->setCurrentIndex(format.isBigEndian() ? 0 : 1);

  // Set chroma offsets
  this->ui.comboBoxChromaOffsetX->setCurrentIndex(format.getChromaOffset().x);
  this->ui.comboBoxChromaOffsetY->setCurrentIndex(format.getChromaOffset().y);

  // Set component layout
  DataLayout layout = format.getDataLayout();
  if (layout == DataLayout::Interleaved)
    this->ui.radioButtonInterleaved->setChecked(true);
  else if (layout == DataLayout::SemiPlanar)
    this->ui.radioButtonSemiPlanar->setChecked(true);
  else
    this->ui.radioButtonPlanar->setChecked(true);

  // Update component order combo box
  this->updateComponentOrderComboBox();

  // Set component order
  const auto orderName = ComponentOrderMapper.getName(format.getComponentOrder());
  this->ui.comboBoxElemOrder->setCurrentText(QString::fromStdString(std::string(orderName)));

  // Set padding info
  const auto paddingName = PaddingInfoMapper.getName(format.getPaddingInfo());
  this->ui.comboBoxPaddingInfo->setCurrentText(QString::fromStdString(std::string(paddingName)));

  // Set byte packing
  this->ui.checkBoxBytePacking->setChecked(format.isBytePacking());

  // Update controls state
  this->updateControlsState();

  this->updatingUiFromFormat = false;
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
  DataLayout dataLayout = DataLayout::Planar;
  if (this->ui.radioButtonInterleaved->isChecked())
    dataLayout = DataLayout::Interleaved;
  else if (this->ui.radioButtonSemiPlanar->isChecked())
    dataLayout = DataLayout::SemiPlanar;

  // Get component order
  const std::string orderName = this->ui.comboBoxElemOrder->currentText().toStdString();
  const auto componentOrder = ComponentOrderMapper.getValueFromNameOrIndex(orderName);
  if (!componentOrder)
    return {};

  // Get padding info
  const std::string paddingName = this->ui.comboBoxPaddingInfo->currentText().toStdString();
  const auto paddingInfo = PaddingInfoMapper.getValueFromNameOrIndex(paddingName);
  if (!paddingInfo)
    return {};

  const auto bytePacking = this->ui.checkBoxBytePacking->isChecked();

  return PixelFormatYUV(
    *subsampling, bitsPerSample, dataLayout, *componentOrder, bigEndian,
    chromaOffset, bytePacking, *paddingInfo);
}

} // namespace video::yuv
