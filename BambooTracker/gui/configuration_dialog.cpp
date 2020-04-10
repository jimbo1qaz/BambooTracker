#include "configuration_dialog.hpp"
#include "ui_configuration_dialog.h"
#include <algorithm>
#include <functional>
#include <map>
#include <QPushButton>
#include <QMenu>
#include <QMessageBox>
#include <QAudio>
#include <QAudioDeviceInfo>
#include <QFont>
#include <QColorDialog>
#include <QFileDialog>
#include <QApplication>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QToolButton>
#include <QHBoxLayout>
#include "slider_style.hpp"
#include "fm_envelope_set_edit_dialog.hpp"
#include "midi/midi.hpp"
#include "jam_manager.hpp"
#include "chips/chip_misc.h"
#include "color_palette_handler.hpp"

ConfigurationDialog::ConfigurationDialog(std::weak_ptr<Configuration> config, std::weak_ptr<ColorPalette> palette,
										 std::string curApi, std::vector<std::string> apis, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::ConfigurationDialog),
	  config_(config),
	  palette_(palette)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
	QObject::connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
					 this, [&] {
		on_ConfigurationDialog_accepted();
		emit applyPressed();
	});

	std::shared_ptr<Configuration> configLocked = config.lock();
	// General //
	// General settings
	auto glfunc = [&](int i, bool enabled, QString desc) {
		QListWidgetItem* item = ui->generalSettingsListWidget->item(i);
		item->setCheckState(toCheckState(enabled));
		item->setData(Qt::UserRole, desc);
	};
	glfunc(0, configLocked->getWarpCursor(),
		   tr("Warp the cursor around the edges of the pattern editor."));
	glfunc(1, configLocked->getWarpAcrossOrders(),
		   tr("Move to previous or next order when reaching top or bottom in the pattern editor."));
	glfunc(2, configLocked->getShowRowNumberInHex(),
		   tr("Display row numbers and the playback position on the status bar in hexadecimal."));
	glfunc(3, configLocked->getShowPreviousNextOrders(),
		   tr("Preview previous and next orders in the pattern editor."));
	glfunc(4, configLocked->getBackupModules(),
		   tr("Create a backup copy of the existing file when saving a module."));
	glfunc(5, configLocked->getDontSelectOnDoubleClick(),
		   tr("Don't select the whole track when double-clicking in the pattern editor."));
	glfunc(6, configLocked->getReverseFMVolumeOrder(),
		   tr("Reverse the order of FM volume so that 00 is the quietest in the pattern editor."));
	glfunc(7, configLocked->getMoveCursorToRight(),
		   tr("Move the cursor to right after entering effects in the pattern editor."));
	glfunc(8, configLocked->getRetrieveChannelState(),
		   tr("Reconstruct the current channel's state from previous orders upon playing."));
	glfunc(9, configLocked->getEnableTranslation(),
		   tr("Translate to your language from the next launch. See readme to check supported languages."));
	glfunc(10, configLocked->getShowFMDetuneAsSigned(),
		   tr("Display FM detune values as signed numbers in the FM envelope editor."));
	glfunc(11, configLocked->getShowWaveVisual(),
		   tr("Enable an oscilloscope which displays a waveform of the sound output."));
	glfunc(12, configLocked->getFill00ToEffectValue(),
		   tr("Fill 00 to effect value column upon entering effect id."));
	glfunc(13, configLocked->getMoveCursorByHorizontalScroll(),
		   tr("Move the cursor position by cell with horizontal scroll bar in the order list and the pattern editor."));
	glfunc(14, configLocked->getOverwriteUnusedUneditedPropety(),
		   tr("Overwrite unused and unedited instrument properties on creating new properties. "
			  "When disabled, override unused properties regardless of editing."));
	glfunc(15, configLocked->getWriteOnlyUsedSamples(),
		   tr("Send only ADPCM samples used by instruments to the ADPCM memory. "
			  "Recommend to turn off if you change ADPCM samples frequently due to take the high rewriting cost."));

	// Edit settings
	ui->pageJumpLengthSpinBox->setValue(static_cast<int>(configLocked->getPageJumpLength()));

	// Keys
	ui->shortcutsTreeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	ui->shortcutsTreeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
	std::map<Configuration::ShortcutAction, QString> shortcutsActions = {
		{ Configuration::KeyOff, QT_TR_NOOP("Key off") },
		{ Configuration::OctaveUp, QT_TR_NOOP("Octave up") },
		{ Configuration::OctaveDown, QT_TR_NOOP("Octave down") },
		{ Configuration::EchoBuffer, QT_TR_NOOP("Echo buffer") },
		{ Configuration::PlayAndStop, QT_TR_NOOP("Play and stop") },
		{ Configuration::Play, QT_TR_NOOP("Play") },
		{ Configuration::PlayFromStart, QT_TR_NOOP("Play from start") },
		{ Configuration::PlayPattern, QT_TR_NOOP("Play pattern") },
		{ Configuration::PlayFromCursor, QT_TR_NOOP("Play from cursor") },
		{ Configuration::PlayFromMarker, QT_TR_NOOP("Play from marker") },
		{ Configuration::PlayStep, QT_TR_NOOP("Play step") },
		{ Configuration::Stop, QT_TR_NOOP("Stop") },
		{ Configuration::FocusOnPattern, QT_TR_NOOP("Focus on pattern editor") },
		{ Configuration::FocusOnOrder, QT_TR_NOOP("Focus on order list") },
		{ Configuration::FocusOnInstrument, QT_TR_NOOP("Focus on instrument list") },
		{ Configuration::ToggleEditJam, QT_TR_NOOP("Toggle edit/jam mode") },
		{ Configuration::SetMarker, QT_TR_NOOP("Set marker") },
		{ Configuration::PasteMix, QT_TR_NOOP("Paste and mix") },
		{ Configuration::PasteOverwrite, QT_TR_NOOP("Paste and overwrite") },
		{ Configuration::SelectAll, QT_TR_NOOP("Select all") },
		{ Configuration::Deselect, QT_TR_NOOP("Deselect") },
		{ Configuration::SelectRow, QT_TR_NOOP("Select row") },
		{ Configuration::SelectColumn, QT_TR_NOOP("Select column") },
		{ Configuration::SelectPattern, QT_TR_NOOP("Select pattern") },
		{ Configuration::SelectOrder, QT_TR_NOOP("Select order") },
		{ Configuration::GoToStep, QT_TR_NOOP("Go to step") },
		{ Configuration::ToggleTrack, QT_TR_NOOP("Toggle track") },
		{ Configuration::SoloTrack, QT_TR_NOOP("Solo track") },
		{ Configuration::Interpolate, QT_TR_NOOP("Interpolate") },
		{ Configuration::Reverse, QT_TR_NOOP("Reverse") },
		{ Configuration::GoToPrevOrder, QT_TR_NOOP("Go to previous order") },
		{ Configuration::GoToNextOrder, QT_TR_NOOP("Go to next order") },
		{ Configuration::ToggleBookmark, QT_TR_NOOP("Toggle bookmark") },
		{ Configuration::PrevBookmark, QT_TR_NOOP("Previous bookmark") },
		{ Configuration::NextBookmark, QT_TR_NOOP("Next bookmark") },
		{ Configuration::DecreaseNote, QT_TR_NOOP("Transpose, decrease note") },
		{ Configuration::IncreaseNote, QT_TR_NOOP("Transpose, increase note") },
		{ Configuration::DecreaseOctave, QT_TR_NOOP("Transpose, decrease octave") },
		{ Configuration::IncreaseOctave, QT_TR_NOOP("Transpose, increase octave") },
		{ Configuration::PrevInstrument, QT_TR_NOOP("Previous instrument") },
		{ Configuration::NextInstrument, QT_TR_NOOP("Next instrument") },
		{ Configuration::MaskInstrument, QT_TR_NOOP("Mask instrument") },
		{ Configuration::MaskVolume, QT_TR_NOOP("Mask volume") },
		{ Configuration::EditInstrument, QT_TR_NOOP("Edit instrument") },
		{ Configuration::FollowMode, QT_TR_NOOP("Follow mode") },
		{ Configuration::DuplicateOrder, QT_TR_NOOP("Duplicate order") },
		{ Configuration::ClonePatterns, QT_TR_NOOP("Clone patterns") },
		{ Configuration::CloneOrder, QT_TR_NOOP("Clone order") },
		{ Configuration::ReplaceInstrument, QT_TR_NOOP("Replace instrument") },
		{ Configuration::ExpandPattern, QT_TR_NOOP("Expand pattern") },
		{ Configuration::ShrinkPattern, QT_TR_NOOP("Shrink pattern") },
		{ Configuration::FineDecreaseValues, QT_TR_NOOP("Fine decrease values") },
		{ Configuration::FineIncreaseValues, QT_TR_NOOP("Fine increase values") },
		{ Configuration::CoarseDecreaseValues, QT_TR_NOOP("Coarse decrease values") },
		{ Configuration::CoarseIncreaseValuse, QT_TR_NOOP("Coarse increase valuse") },
		{ Configuration::ExpandEffect, QT_TR_NOOP("Expand effect column") },
		{ Configuration::ShrinkEffect, QT_TR_NOOP("Shrink effect column") },
		{ Configuration::PrevHighlighted, QT_TR_NOOP("Previous highlighted step") },
		{ Configuration::NextHighlighted, QT_TR_NOOP("Next highlighted step") },
		{ Configuration::IncreasePatternSize, QT_TR_NOOP("Increase pattern size") },
		{ Configuration::DecreasePatternSize, QT_TR_NOOP("Decrease pattern size") },
		{ Configuration::IncreaseEditStep, QT_TR_NOOP("Increase edit step") },
		{ Configuration::DecreaseEditStep, QT_TR_NOOP("Decrease edit step") }
	};
	std::unordered_map<Configuration::ShortcutAction, std::string> shortcuts = configLocked->getShortcuts();
	for (const auto& pair : shortcutsActions) {
		int row = ui->shortcutsTreeWidget->topLevelItemCount();
		auto item = new QTreeWidgetItem();
		item->setText(0, pair.second);
		ui->shortcutsTreeWidget->insertTopLevelItem(row, item);
		auto widget = new QWidget();
		widget->setLayout(new QHBoxLayout());
		std::string shortcut = shortcuts.at(pair.first);
		auto seq = new QKeySequenceEdit(QString::fromUtf8(shortcut.c_str(), static_cast<int>(shortcut.length())));
		shortcutsMap_[pair.first] = seq;
		auto button = new QToolButton();
		button->setIcon(QIcon(":/icon/remove_inst"));
		QObject::connect(button, &QToolButton::clicked, seq, &QKeySequenceEdit::clear);
		auto layout = widget->layout();
		layout->setSpacing(0);
		layout->setMargin(0);
		layout->addWidget(seq);
		layout->addWidget(button);
		ui->shortcutsTreeWidget->setItemWidget(item, 1, widget);
	}

	ui->keyboardTypeComboBox->setCurrentIndex(static_cast<int>(configLocked->getNoteEntryLayout()));
	customLayoutKeysMap_ = {
		{ JamKey::LowC,     ui->lowCEdit },
		{ JamKey::LowCS,    ui->lowCSEdit },
		{ JamKey::LowD,     ui->lowDEdit },
		{ JamKey::LowDS,    ui->lowDSEdit },
		{ JamKey::LowE,     ui->lowEEdit },
		{ JamKey::LowF,     ui->lowFEdit },
		{ JamKey::LowFS,    ui->lowFSEdit },
		{ JamKey::LowG,     ui->lowGEdit },
		{ JamKey::LowGS,    ui->lowGSEdit },
		{ JamKey::LowA,     ui->lowAEdit },
		{ JamKey::LowAS,    ui->lowASEdit },
		{ JamKey::LowB,     ui->lowBEdit },
		{ JamKey::LowC2,   ui->lowHighCEdit },
		{ JamKey::LowCS2,  ui->lowHighCSEdit },
		{ JamKey::LowD2,   ui->lowHighDEdit },

		{ JamKey::HighC,    ui->highCEdit },
		{ JamKey::HighCS,   ui->highCSEdit },
		{ JamKey::HighD,    ui->highDEdit },
		{ JamKey::HighDS,   ui->highDSEdit },
		{ JamKey::HighE,    ui->highEEdit },
		{ JamKey::HighF,    ui->highFEdit },
		{ JamKey::HighFS,   ui->highFSEdit },
		{ JamKey::HighG,    ui->highGEdit },
		{ JamKey::HighGS,   ui->highGSEdit },
		{ JamKey::HighA,    ui->highAEdit },
		{ JamKey::HighAS,   ui->highASEdit },
		{ JamKey::HighB,    ui->highBEdit },
		{ JamKey::HighC2,  ui->highHighCEdit },
		{ JamKey::HighCS2, ui->highHighCSEdit },
		{ JamKey::HighD2,  ui->highHighDEdit }
	};
	for (const auto& pair : configLocked->getCustomLayoutKeys()) {
		customLayoutKeysMap_.at(pair.second)->setKeySequence(QKeySequence(QString::fromUtf8(pair.first.c_str(), static_cast<int>(pair.first.length()))));
	}

	// Emulation //
	ui->emulatorComboBox->addItem("MAME YM2608", static_cast<int>(chip::Emu::Mame));
	ui->emulatorComboBox->addItem("Nuked OPN-Mod", static_cast<int>(chip::Emu::Nuked));
	ui->emulatorComboBox->setCurrentIndex(ui->emulatorComboBox->findData(configLocked->getEmulator()));

	// Sound //
	int apiRow = -1;
	int defApiRow = 0;
	for (auto& name : apis) {
		ui->soundAPIComboBox->addItem(QString::fromUtf8(name.c_str(), static_cast<int>(name.length())));
		if (name == configLocked->getSoundAPI()) apiRow = ui->soundAPIComboBox->count() - 1;
		if (name == curApi) defApiRow = apiRow = ui->soundAPIComboBox->count() - 1;
	}
	ui->soundAPIComboBox->setCurrentIndex((apiRow == -1) ? defApiRow : apiRow);

	int devRow = -1;
	int defDevRow = 0;
	for (auto& info : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
		ui->soundDeviceComboBox->addItem(info.deviceName());
		if (info.deviceName() == QString::fromUtf8(configLocked->getSoundDevice().c_str(),
												   static_cast<int>(configLocked->getSoundDevice().length())))
			devRow = ui->soundDeviceComboBox->count() - 1;
		if (info.deviceName() == QAudioDeviceInfo::defaultOutputDevice().deviceName()) {
			defDevRow = ui->soundDeviceComboBox->count() - 1;
		}
	}
	ui->soundDeviceComboBox->setCurrentIndex((devRow == -1) ? defDevRow : devRow);

	ui->realChipComboBox->addItem(tr("None"), static_cast<int>(RealChipInterface::NONE));
	ui->realChipComboBox->addItem("SCCI", static_cast<int>(RealChipInterface::SCCI));
	ui->realChipComboBox->addItem("C86CTL", static_cast<int>(RealChipInterface::C86CTL));
	switch (configLocked->getRealChipInterface()) {
	default:	// Fall through
	case RealChipInterface::NONE:	ui->realChipComboBox->setCurrentIndex(0);	break;
	case RealChipInterface::SCCI:	ui->realChipComboBox->setCurrentIndex(1);	break;
	case RealChipInterface::C86CTL:	ui->realChipComboBox->setCurrentIndex(2);	break;
	}

	MidiInterface &midiIntf = MidiInterface::instance();
	if (midiIntf.supportsVirtualPort())
		ui->midiInputNameLine->setPlaceholderText(tr("Virtual port"));
	ui->midiInputNameLine->setText(QString::fromStdString(configLocked->getMidiInputPort()));

	ui->sampleRateComboBox->addItem("44100Hz", 44100);
	ui->sampleRateComboBox->addItem("48000Hz", 48000);
	ui->sampleRateComboBox->addItem("55466Hz", 55466);
	switch (configLocked->getSampleRate()) {
	default:	// Fall through
	case 44100:	ui->sampleRateComboBox->setCurrentIndex(0);	break;
	case 48000:	ui->sampleRateComboBox->setCurrentIndex(1);	break;
	case 55466:	ui->sampleRateComboBox->setCurrentIndex(2);	break;
	}
	ui->bufferLengthHorizontalSlider->setStyle(new SliderStyle());
	QObject::connect(ui->bufferLengthHorizontalSlider, &QSlider::valueChanged,
					 this, [&](int value) {
		ui->bufferLengthLabel->setText(QString::number(value) + "ms");
	});
	ui->bufferLengthHorizontalSlider->setValue(static_cast<int>(configLocked->getBufferLength()));


	// Mixer //
	ui->masterMixerSlider->setText(tr("Master"));
	ui->masterMixerSlider->setSuffix("%");
	ui->masterMixerSlider->setMaximum(200);
	ui->masterMixerSlider->setMinimum(0);
	ui->masterMixerSlider->setTickPosition(QSlider::TicksBothSides);
	ui->masterMixerSlider->setTickInterval(20);
	ui->masterMixerSlider->setValue(configLocked->getMixerVolumeMaster());

	ui->fmMixerSlider->setText("FM");
	ui->fmMixerSlider->setSuffix("dB");
	ui->fmMixerSlider->setMaximum(120);
	ui->fmMixerSlider->setMinimum(-120);
	ui->fmMixerSlider->setValueRate(0.1);
	ui->fmMixerSlider->setSign(true);
	ui->fmMixerSlider->setTickPosition(QSlider::TicksBothSides);
	ui->fmMixerSlider->setTickInterval(20);
	ui->fmMixerSlider->setValue(static_cast<int>(configLocked->getMixerVolumeFM() * 10));

	ui->ssgMixerSlider->setText("SSG");
	ui->ssgMixerSlider->setSuffix("dB");
	ui->ssgMixerSlider->setMaximum(120);
	ui->ssgMixerSlider->setMinimum(-120);
	ui->ssgMixerSlider->setValueRate(0.1);
	ui->ssgMixerSlider->setSign(true);
	ui->ssgMixerSlider->setTickPosition(QSlider::TicksBothSides);
	ui->ssgMixerSlider->setTickInterval(20);
	ui->ssgMixerSlider->setValue(static_cast<int>(configLocked->getMixerVolumeSSG() * 10));

	// Formats //
	fmEnvelopeTexts_ = configLocked->getFMEnvelopeTexts();
	updateEnvelopeSetUi();

	// Appearance //
	ui->colorsTreeWidget->setColumnWidth(0, 250);
	updateColorTree();

	ui->ptnHdFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getPatternEditorHeaderFont().c_str(),
										static_cast<int>(configLocked->getPatternEditorHeaderFont().size()))));
	ui->ptnHdFontSizeComboBox->setCurrentText(QString::number(configLocked->getPatternEditorHeaderFontSize()));
	ui->ptnRowFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getPatternEditorRowsFont().c_str(),
										static_cast<int>(configLocked->getPatternEditorRowsFont().size()))));
	ui->ptnRowFontSizeComboBox->setCurrentText(QString::number(configLocked->getPatternEditorRowsFontSize()));
	ui->odrHdFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getOrderListHeaderFont().c_str(),
										static_cast<int>(configLocked->getOrderListHeaderFont().size()))));
	ui->odrHdFontSizeComboBox->setCurrentText(QString::number(configLocked->getOrderListHeaderFontSize()));
	ui->odrRowFontComboBox->setCurrentFont(
				QFont(QString::fromUtf8(configLocked->getOrderListRowsFont().c_str(),
										static_cast<int>(configLocked->getOrderListRowsFont().size()))));
	ui->odrRowFontSizeComboBox->setCurrentText(QString::number(configLocked->getOrderListRowsFontSize()));
}

ConfigurationDialog::~ConfigurationDialog()
{
	delete ui;
}

void ConfigurationDialog::on_ConfigurationDialog_accepted()
{
	std::shared_ptr<Configuration> configLocked = config_.lock();
	// General //
	// General settings
	configLocked->setWarpCursor(fromCheckState(ui->generalSettingsListWidget->item(0)->checkState()));
	configLocked->setWarpAcrossOrders(fromCheckState(ui->generalSettingsListWidget->item(1)->checkState()));
	configLocked->setShowRowNumberInHex(fromCheckState(ui->generalSettingsListWidget->item(2)->checkState()));
	configLocked->setShowPreviousNextOrders(fromCheckState(ui->generalSettingsListWidget->item(3)->checkState()));
	configLocked->setBackupModules(fromCheckState(ui->generalSettingsListWidget->item(4)->checkState()));
	configLocked->setDontSelectOnDoubleClick(fromCheckState(ui->generalSettingsListWidget->item(5)->checkState()));
	configLocked->setReverseFMVolumeOrder(fromCheckState(ui->generalSettingsListWidget->item(6)->checkState()));
	configLocked->setMoveCursorToRight(fromCheckState(ui->generalSettingsListWidget->item(7)->checkState()));
	configLocked->setRetrieveChannelState(fromCheckState(ui->generalSettingsListWidget->item(8)->checkState()));
	configLocked->setEnableTranslation(fromCheckState(ui->generalSettingsListWidget->item(9)->checkState()));
	configLocked->setShowFMDetuneAsSigned(fromCheckState(ui->generalSettingsListWidget->item(10)->checkState()));
	configLocked->setShowWaveVisual(fromCheckState(ui->generalSettingsListWidget->item(11)->checkState()));
	configLocked->setFill00ToEffectValue(fromCheckState(ui->generalSettingsListWidget->item(12)->checkState()));
	configLocked->setMoveCursorByHorizontalScroll(fromCheckState(ui->generalSettingsListWidget->item(13)->checkState()));
	configLocked->setOverwriteUnusedUneditedPropety(fromCheckState(ui->generalSettingsListWidget->item(14)->checkState()));
	configLocked->setWriteOnlyUsedSamples(fromCheckState(ui->generalSettingsListWidget->item(15)->checkState()));

	// Edit settings
	configLocked->setPageJumpLength(static_cast<size_t>(ui->pageJumpLengthSpinBox->value()));

	// Keys
	std::unordered_map<Configuration::ShortcutAction, std::string> shortcuts;
	for (const auto& pair: shortcutsMap_) {
		shortcuts[pair.first] = pair.second->keySequence().toString().toStdString();
	}
	configLocked->setShortcuts(shortcuts);

	configLocked->setNoteEntryLayout(static_cast<Configuration::KeyboardLayout>(ui->keyboardTypeComboBox->currentIndex()));
	std::unordered_map<std::string, JamKey> customLayoutNewKeys;
	for (const auto& pair : customLayoutKeysMap_) {
		customLayoutNewKeys[pair.second->keySequence().toString().toStdString()] = pair.first;
	}
	configLocked->setCustomLayoutKeys(customLayoutNewKeys);

	// Emulation //
	int emu = ui->emulatorComboBox->currentData().toInt();
	bool changedEmu = false;
	if (emu != configLocked->getEmulator()) {
		configLocked->setEmulator(emu);
		changedEmu = true;
	}

	// Sound //
	configLocked->setSoundDevice(ui->soundDeviceComboBox->currentText().toUtf8().toStdString());
	configLocked->setSoundAPI(ui->soundAPIComboBox->currentText().toUtf8().toStdString());
	configLocked->setRealChipInterface(static_cast<RealChipInterface>(
										   ui->realChipComboBox->currentData(Qt::UserRole).toInt()));
	configLocked->setMidiInputPort(ui->midiInputNameLine->text().toStdString());
	configLocked->setSampleRate(ui->sampleRateComboBox->currentData(Qt::UserRole).toUInt());
	configLocked->setBufferLength(static_cast<size_t>(ui->bufferLengthHorizontalSlider->value()));

	// Mixer //
	configLocked->setMixerVolumeMaster(ui->masterMixerSlider->value());
	configLocked->setMixerVolumeFM(ui->fmMixerSlider->value() * 0.1);
	configLocked->setMixerVolumeSSG(ui->ssgMixerSlider->value() * 0.1);

	// Formats //
	std::sort(fmEnvelopeTexts_.begin(), fmEnvelopeTexts_.end(),
			  [](const FMEnvelopeText& a, const FMEnvelopeText& b) -> bool { return (a.name < b.name); });
	configLocked->setFMEnvelopeTexts(fmEnvelopeTexts_);

	// Appearance //
	auto* pl = palette_.lock().get();
	QTreeWidgetItem* ptnColors = ui->colorsTreeWidget->topLevelItem(0);
	pl->ptnDefTextColor = ptnColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnDefStepColor = ptnColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl1StepColor = ptnColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl2StepColor = ptnColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurTextColor = ptnColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurStepColor = ptnColors->child(5)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurEditStepColor = ptnColors->child(6)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnCurCellColor = ptnColors->child(7)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnPlayStepColor = ptnColors->child(8)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnSelCellColor = ptnColors->child(9)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHovCellColor = ptnColors->child(10)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnDefStepNumColor = ptnColors->child(11)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl1StepNumColor = ptnColors->child(12)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHl2StepNumColor = ptnColors->child(13)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnNoteColor = ptnColors->child(14)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnInstColor = ptnColors->child(15)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnVolColor = ptnColors->child(16)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnEffColor = ptnColors->child(17)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnErrorColor = ptnColors->child(18)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHeaderTextColor = ptnColors->child(19)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnHeaderRowColor = ptnColors->child(20)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnMaskColor = ptnColors->child(21)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnBorderColor = ptnColors->child(22)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnMuteColor = ptnColors->child(23)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnUnmuteColor = ptnColors->child(24)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ptnBackColor = ptnColors->child(25)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* odrColors = ui->colorsTreeWidget->topLevelItem(1);
	pl->odrDefTextColor = odrColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrDefRowColor = odrColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurTextColor = odrColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurRowColor = odrColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurEditRowColor = odrColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrCurCellColor = odrColors->child(5)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrPlayRowColor = odrColors->child(6)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrSelCellColor = odrColors->child(7)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrHovCellColor = odrColors->child(8)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrRowNumColor = odrColors->child(9)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrHeaderTextColor = odrColors->child(10)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrHeaderRowColor = odrColors->child(11)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrBorderColor = odrColors->child(12)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->odrBackColor = odrColors->child(13)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* ilistColors = ui->colorsTreeWidget->topLevelItem(2);
	pl->ilistTextColor = ilistColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistBackColor = ilistColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistSelBackColor = ilistColors->child(2)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistHovBackColor = ilistColors->child(3)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->ilistHovSelBackColor = ilistColors->child(4)->data(1, Qt::BackgroundRole).value<QColor>();

	QTreeWidgetItem* wavColors = ui->colorsTreeWidget->topLevelItem(3);
	pl->wavBackColor = wavColors->child(0)->data(1, Qt::BackgroundRole).value<QColor>();
	pl->wavDrawColor = wavColors->child(1)->data(1, Qt::BackgroundRole).value<QColor>();

	configLocked->setPatternEditorHeaderFont(ui->ptnHdFontComboBox->currentFont().family().toStdString());
	configLocked->setPatternEditorHeaderFontSize(ui->ptnHdFontSizeComboBox->currentText().toInt());
	configLocked->setPatternEditorRowsFont(ui->ptnRowFontComboBox->currentFont().family().toStdString());
	configLocked->setPatternEditorRowsFontSize(ui->ptnRowFontSizeComboBox->currentText().toInt());
	configLocked->setOrderListHeaderFont(ui->odrHdFontComboBox->currentFont().family().toStdString());
	configLocked->setOrderListHeaderFontSize(ui->odrHdFontSizeComboBox->currentText().toInt());
	configLocked->setOrderListRowsFont(ui->odrRowFontComboBox->currentFont().family().toStdString());
	configLocked->setOrderListRowsFontSize(ui->odrRowFontSizeComboBox->currentText().toInt());

	if (changedEmu) {
		QMessageBox::information(this, tr("Configuration"), tr("The change of emulator will be effective after restarting the program."));
	}
}

/***** General *****/
void ConfigurationDialog::on_generalSettingsListWidget_itemSelectionChanged()
{
	QString text("");
	if (QListWidgetItem* item = ui->generalSettingsListWidget->currentItem()) {
		text = item->data(Qt::UserRole).toString();
	}
	ui->descPlainTextEdit->setPlainText(tr("Description: %1").arg(text));
}

/***** Mixer *****/
void ConfigurationDialog::on_mixerResetPushButton_clicked()
{
	ui->fmMixerSlider->setValue(0);
	ui->ssgMixerSlider->setValue(0);
}

/***** Sound *****/
void ConfigurationDialog::on_midiInputChoiceButton_clicked()
{
	QToolButton *button = ui->midiInputChoiceButton;
	QMenu menu;
	QAction *action;

	MidiInterface &intf = MidiInterface::instance();
	std::vector<std::string> ports = intf.getRealInputPorts();
	bool vport = intf.supportsVirtualPort();

	if (vport) {
		ui->midiInputNameLine->setPlaceholderText(tr("Virtual port"));
		action = menu.addAction(tr("Virtual port"));
		action->setData(QString());
	}

	for (unsigned i = 0, n = ports.size(); i < n; ++i) {
		if (i == 0 && vport)
			menu.addSeparator();
		QString portName = QString::fromStdString(ports[i]);
		action = menu.addAction(portName);
		action->setData(portName);
	}

	QAction *choice = menu.exec(button->mapToGlobal(button->rect().bottomLeft()));
	if (choice) {
		QString portName = choice->data().toString();
		ui->midiInputNameLine->setText(portName);
	}
}

/*****Formats *****/
void ConfigurationDialog::on_addEnvelopeSetPushButton_clicked()
{
	auto name = tr("Set %1").arg(fmEnvelopeTexts_.size() + 1);
	fmEnvelopeTexts_.push_back({ name.toUtf8().toStdString(), std::vector<FMEnvelopeTextType>() });
	updateEnvelopeSetUi();
	for (int i = ui->envelopeTypeListWidget->count() - 1; i >= 0; --i) {
		if (ui->envelopeTypeListWidget->item(i)->text() == name) {
			ui->envelopeTypeListWidget->setCurrentRow(i);
			break;
		}
	}
}

void ConfigurationDialog::on_removeEnvelopeSetpushButton_clicked()
{
	fmEnvelopeTexts_.erase(fmEnvelopeTexts_.begin() + ui->envelopeTypeListWidget->currentRow());
	updateEnvelopeSetUi();
}

void ConfigurationDialog::on_editEnvelopeSetPushButton_clicked()
{
	size_t row = static_cast<size_t>(ui->envelopeTypeListWidget->currentRow());
	FMEnvelopeSetEditDialog diag(fmEnvelopeTexts_.at(row).texts);
	diag.setWindowTitle(diag.windowTitle() + ": " + ui->envelopeSetNameLineEdit->text());
	if (diag.exec() == QDialog::Accepted) {
		fmEnvelopeTexts_.at(row).texts = diag.getSet();
	}
}

void ConfigurationDialog::on_envelopeSetNameLineEdit_textChanged(const QString &arg1)
{
	fmEnvelopeTexts_.at(static_cast<size_t>(ui->envelopeTypeListWidget->currentRow())).name = arg1.toStdString();
	ui->envelopeTypeListWidget->currentItem()->setText(arg1);
}

void ConfigurationDialog::on_envelopeTypeListWidget_currentRowChanged(int currentRow)
{
	if (currentRow == -1) {
		ui->editEnvelopeSetPushButton->setEnabled(false);
		ui->removeEnvelopeSetpushButton->setEnabled(false);
		ui->envelopeSetNameLineEdit->setEnabled(false);
	}
	else {
		ui->editEnvelopeSetPushButton->setEnabled(true);
		ui->removeEnvelopeSetpushButton->setEnabled(true);
		ui->envelopeSetNameLineEdit->setEnabled(true);
		ui->envelopeSetNameLineEdit->setText(ui->envelopeTypeListWidget->item(currentRow)->text());
	}
}

void ConfigurationDialog::updateEnvelopeSetUi()
{
	std::sort(fmEnvelopeTexts_.begin(), fmEnvelopeTexts_.end(),
			  [](const FMEnvelopeText& a, const FMEnvelopeText& b) -> bool { return (a.name < b.name); });

	ui->envelopeTypeListWidget->clear();
	for (auto& texts : fmEnvelopeTexts_)
		ui->envelopeTypeListWidget->addItem(
					QString::fromUtf8(texts.name.c_str(), static_cast<int>(texts.name.length())));
}

/***** Keys *****/
void ConfigurationDialog::on_keyboardTypeComboBox_currentIndexChanged(int index)
{
	Q_UNUSED(index)
	bool enableCustomLayoutInterface = ui->keyboardTypeComboBox->currentIndex() == 0;
	ui->lowHighKeysTabWidget->setEnabled(enableCustomLayoutInterface);
	ui->customLayoutResetButton->setEnabled(enableCustomLayoutInterface);
}

void ConfigurationDialog::on_customLayoutResetButton_clicked()
{
	std::unordered_map<std::string, JamKey> QWERTYLayoutMapping = config_.lock()->mappingLayouts.at (Configuration::KeyboardLayout::QWERTY);
	std::unordered_map<std::string, JamKey>::const_iterator QWERTYLayoutMappingIterator = QWERTYLayoutMapping.begin();
	while (QWERTYLayoutMappingIterator != QWERTYLayoutMapping.end()) {
		customLayoutKeysMap_.at(QWERTYLayoutMappingIterator->second)->setKeySequence(QKeySequence(QString::fromStdString(QWERTYLayoutMappingIterator->first)));
		QWERTYLayoutMappingIterator++;
	}
}

void ConfigurationDialog::addShortcutItem(QString action, std::string shortcut)
{
	int row = ui->shortcutsTreeWidget->topLevelItemCount();
	auto titem = new QTreeWidgetItem();
	titem->setText(0, action);
	ui->shortcutsTreeWidget->insertTopLevelItem(row, titem);
	ui->shortcutsTreeWidget->setItemWidget(titem, 1, new QKeySequenceEdit(QString::fromUtf8(shortcut.c_str(), static_cast<int>(shortcut.length()))));
}

std::string ConfigurationDialog::getShortcutString(int row) const
{
	return qobject_cast<QKeySequenceEdit*>(
				ui->shortcutsTreeWidget->itemWidget(ui->shortcutsTreeWidget->topLevelItem(row), 1)
				)->keySequence().toString().toStdString();
}

/***** Appearance *****/
void ConfigurationDialog::on_colorEditPushButton_clicked()
{
	QTreeWidgetItem* item = ui->colorsTreeWidget->currentItem();
	if (item == nullptr || item->parent() == nullptr) return;
	QColorDialog diag(item->data(1, Qt::BackgroundRole).value<QColor>());
	diag.setOption(QColorDialog::ShowAlphaChannel);
	if (diag.exec() == QDialog::Accepted)
		item->setData(1, Qt::BackgroundRole, diag.currentColor());
}

void ConfigurationDialog::on_colorLoadPushButton_clicked()
{
	QString file = QFileDialog::getOpenFileName(this, tr("Open color scheme"),
												QApplication::applicationDirPath() + "/skins",
												tr("ini file (*.ini)"));
	if (file.isNull()) return;

	if (ColorPaletteHandler::loadPalette(file, palette_)) {
		updateColorTree();
	}
	else {
		QMessageBox::critical(this, tr("Error"), tr("An unknown error occurred while loading the color scheme."));
	}
}

void ConfigurationDialog::on_colorSavePushButton_clicked()
{
	QString file = QFileDialog::getSaveFileName(this, tr("Save color scheme"),
												QApplication::applicationDirPath() + "/skins",
												tr("ini file (*.ini)"));
	if (file.isNull()) return;
	if (!file.endsWith(".ini")) file += ".ini";	// For linux

	if (!ColorPaletteHandler::savePalette(file, palette_))
		QMessageBox::critical(this, tr("Error"), tr("Failed to save the color scheme."));
}

void ConfigurationDialog::updateColorTree()
{
	auto* pl = palette_.lock().get();
	QTreeWidgetItem* ptnColors = ui->colorsTreeWidget->topLevelItem(0);
	ptnColors->child(0)->setData(1, Qt::BackgroundRole, pl->ptnDefTextColor);
	ptnColors->child(1)->setData(1, Qt::BackgroundRole, pl->ptnDefStepColor);
	ptnColors->child(2)->setData(1, Qt::BackgroundRole, pl->ptnHl1StepColor);
	ptnColors->child(3)->setData(1, Qt::BackgroundRole, pl->ptnHl2StepColor);
	ptnColors->child(4)->setData(1, Qt::BackgroundRole, pl->ptnCurTextColor);
	ptnColors->child(5)->setData(1, Qt::BackgroundRole, pl->ptnCurStepColor);
	ptnColors->child(6)->setData(1, Qt::BackgroundRole, pl->ptnCurEditStepColor);
	ptnColors->child(7)->setData(1, Qt::BackgroundRole, pl->ptnCurCellColor);
	ptnColors->child(8)->setData(1, Qt::BackgroundRole, pl->ptnPlayStepColor);
	ptnColors->child(9)->setData(1, Qt::BackgroundRole, pl->ptnSelCellColor);
	ptnColors->child(10)->setData(1, Qt::BackgroundRole, pl->ptnHovCellColor);
	ptnColors->child(11)->setData(1, Qt::BackgroundRole, pl->ptnDefStepNumColor);
	ptnColors->child(12)->setData(1, Qt::BackgroundRole, pl->ptnHl1StepNumColor);
	ptnColors->child(13)->setData(1, Qt::BackgroundRole, pl->ptnHl2StepNumColor);
	ptnColors->child(14)->setData(1, Qt::BackgroundRole, pl->ptnNoteColor);
	ptnColors->child(15)->setData(1, Qt::BackgroundRole, pl->ptnInstColor);
	ptnColors->child(16)->setData(1, Qt::BackgroundRole, pl->ptnVolColor);
	ptnColors->child(17)->setData(1, Qt::BackgroundRole, pl->ptnEffColor);
	ptnColors->child(18)->setData(1, Qt::BackgroundRole, pl->ptnErrorColor);
	ptnColors->child(19)->setData(1, Qt::BackgroundRole, pl->ptnHeaderTextColor);
	ptnColors->child(20)->setData(1, Qt::BackgroundRole, pl->ptnHeaderRowColor);
	ptnColors->child(21)->setData(1, Qt::BackgroundRole, pl->ptnMaskColor);
	ptnColors->child(22)->setData(1, Qt::BackgroundRole, pl->ptnBorderColor);
	ptnColors->child(23)->setData(1, Qt::BackgroundRole, pl->ptnMuteColor);
	ptnColors->child(24)->setData(1, Qt::BackgroundRole, pl->ptnUnmuteColor);
	ptnColors->child(25)->setData(1, Qt::BackgroundRole, pl->ptnBackColor);

	QTreeWidgetItem* odrColors = ui->colorsTreeWidget->topLevelItem(1);
	odrColors->child(0)->setData(1, Qt::BackgroundRole, pl->odrDefTextColor);
	odrColors->child(1)->setData(1, Qt::BackgroundRole, pl->odrDefRowColor);
	odrColors->child(2)->setData(1, Qt::BackgroundRole, pl->odrCurTextColor);
	odrColors->child(3)->setData(1, Qt::BackgroundRole, pl->odrCurRowColor);
	odrColors->child(4)->setData(1, Qt::BackgroundRole, pl->odrCurEditRowColor);
	odrColors->child(5)->setData(1, Qt::BackgroundRole, pl->odrCurCellColor);
	odrColors->child(6)->setData(1, Qt::BackgroundRole, pl->odrPlayRowColor);
	odrColors->child(7)->setData(1, Qt::BackgroundRole, pl->odrSelCellColor);
	odrColors->child(8)->setData(1, Qt::BackgroundRole, pl->odrHovCellColor);
	odrColors->child(9)->setData(1, Qt::BackgroundRole, pl->odrRowNumColor);
	odrColors->child(10)->setData(1, Qt::BackgroundRole, pl->odrHeaderTextColor);
	odrColors->child(11)->setData(1, Qt::BackgroundRole, pl->odrHeaderRowColor);
	odrColors->child(12)->setData(1, Qt::BackgroundRole, pl->odrBorderColor);
	odrColors->child(13)->setData(1, Qt::BackgroundRole, pl->odrBackColor);

	QTreeWidgetItem* ilistColors = ui->colorsTreeWidget->topLevelItem(2);
	ilistColors->child(0)->setData(1, Qt::BackgroundRole, pl->ilistTextColor);
	ilistColors->child(1)->setData(1, Qt::BackgroundRole, pl->ilistBackColor);
	ilistColors->child(2)->setData(1, Qt::BackgroundRole, pl->ilistSelBackColor);
	ilistColors->child(3)->setData(1, Qt::BackgroundRole, pl->ilistHovBackColor);
	ilistColors->child(4)->setData(1, Qt::BackgroundRole, pl->ilistHovSelBackColor);

	QTreeWidgetItem* wavColors = ui->colorsTreeWidget->topLevelItem(3);
	wavColors->child(0)->setData(1, Qt::BackgroundRole, pl->wavBackColor);
	wavColors->child(1)->setData(1, Qt::BackgroundRole, pl->wavDrawColor);
}
