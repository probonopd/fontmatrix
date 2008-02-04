//
// C++ Implementation: prefspaneldialog
//
// Description: 
//
//
// Author: Pierre Marchand <pierremarc@oep-h.com>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "prefspaneldialog.h"
#include "typotek.h"
#include <QDebug>
#include <QToolTip>
#include <QSettings>
#include <QFileDialog>

PrefsPanelDialog::PrefsPanelDialog(QWidget *parent)
 : QDialog(parent)
{
	//get this before anything
	double pSize = typotek::getInstance()->getPreviewSize();
	setupUi(this);
	fontEditorPath->setText(typotek::getInstance()->fontEditorPath());
	doConnect();
	systrayFrame->setCheckable(true);
	previewWord->setText(typotek::getInstance()->word());
	previewSizeSpin->setValue(pSize);
	initTagBox->setChecked(typotek::getInstance()->initialTags());
}


PrefsPanelDialog::~PrefsPanelDialog()
{
}

void PrefsPanelDialog::initSystrayPrefs(bool hasSystray, bool isVisible, bool hasActivateAll, bool allConfirmation, bool tagConfirmation)
{
	if (!hasSystray) {
		systrayFrame->setEnabled(false);
		systrayFrame->setToolTip(tr("Looks like your setup does not have a system tray available."));
	}
	systrayFrame->setToolTip("");
	systrayFrame->setChecked(isVisible);
	activateAllFrame->setChecked(hasActivateAll);
	activateAllConfirmation->setChecked(allConfirmation);
	tagsConfirmation->setChecked(tagConfirmation);
	QSettings settings ;
	closeToSystray->setChecked(settings.value("SystrayCloseToTray", true).toBool());
	previewSizeSpin->setValue(typotek::getInstance()->getPreviewSize());
	
}

void PrefsPanelDialog::initSampleTextPrefs()
{
	//At least fill the sampletext list :)
	sampleTextNamesList->addItems(typotek::getInstance()->namedSamplesNames());
	QSettings settings;
	fontSizeSpin->setValue( settings.value("SampleFontSize",12.0).toDouble() );
	interLineSpin->setValue( settings.value("SampleInterline",16.0).toDouble() );
}

void PrefsPanelDialog::initFilesAndFolders()
{
	QSettings settings;
	templatesFolder->setText(typotek::getInstance()->getTemplatesDir());
}


void PrefsPanelDialog::doConnect()
{
	connect(validateNamedSampleTextButton,SIGNAL(released()),this,SLOT(validateSampleName()));
	connect(addSampleTextNameButton,SIGNAL(released()),this,SLOT(addSampleName()));
	connect(newSampleTextNameText,SIGNAL(editingFinished()),this,SLOT(addSampleName()));
	connect(sampleTextNamesList,SIGNAL(currentTextChanged( const QString& )),this,SLOT(displayNamedText()));
	connect(applySampleTextButton,SIGNAL(released()),this,SLOT(applySampleText()));

	connect(systrayFrame, SIGNAL(clicked(bool)), this, SLOT(setSystrayVisible(bool)));
	connect(activateAllFrame, SIGNAL(clicked(bool)), this, SLOT(setSystrayActivateAll(bool)));
	connect(activateAllConfirmation, SIGNAL(clicked(bool)), this, SLOT(setSystrayAllConfirmation(bool)));
	connect(tagsConfirmation, SIGNAL(clicked(bool)), this, SLOT(setSystrayTagsConfirmation(bool)));
	connect(closeToSystray, SIGNAL(clicked(bool)), typotek::getInstance(), SLOT(slotCloseToSystray(bool)));
	connect(previewWord, SIGNAL(textChanged ( const QString  ) ), this, SLOT(updateWord(QString)));
	connect(previewSizeSpin, SIGNAL(valueChanged ( double  ) ), this, SLOT(updateWordSize(double)));
	connect(fontEditorPath, SIGNAL(textChanged ( const QString  ) ), this, SLOT(setupFontEditor(QString)));
	connect(fontEditorBrowse, SIGNAL(clicked()), this, SLOT(slotFontEditorBrowse()));
	connect(initTagBox, SIGNAL(clicked(bool)), typotek::getInstance(), SLOT(slotUseInitialTags(bool)));
	
	connect(templatesDirBrowse,SIGNAL(clicked( )),this, SLOT(slotTemplatesBrowse()));
	connect(templatesFolder,SIGNAL(textChanged( const QString& )),this,SLOT(setupTemplates(const QString&)));

}

void PrefsPanelDialog::applySampleText()
{
	typotek::getInstance()->changeFontSizeSettings( fontSizeSpin->value(), interLineSpin->value() );
	typotek::getInstance()->forwardUpdateView();
}

void PrefsPanelDialog::addSampleName()
{
	QString n = newSampleTextNameText->text();
	if(n.isEmpty())
		return;
	if(typotek::getInstance()->namedSamplesNames().contains(n))
		return;
	
	typotek::getInstance()->addNamedSample(n, tr("A text"));
	sampleTextNamesList->addItem(n);
	newSampleTextNameText->clear();
// 	displayNamedText();
	
}


void PrefsPanelDialog::displayNamedText()
{
	QString name(sampleTextNamesList->currentItem()->text());
	qDebug() << "name is "<< name;
	QString text(typotek::getInstance()->namedSample(name));
	qDebug() << "text is " << text;
	namedSampleTextText->setPlainText(text);
}

void PrefsPanelDialog::validateSampleName()
{
	typotek::getInstance()->changeSample(sampleTextNamesList->currentItem()->text(), namedSampleTextText->toPlainText());
}



void PrefsPanelDialog::setSystrayVisible(bool isVisible)
{
	typotek::getInstance()->setSystrayVisible(isVisible);
}

void PrefsPanelDialog::setSystrayActivateAll(bool isVisible)
{
	typotek::getInstance()->showActivateAllSystray(isVisible);
}

void PrefsPanelDialog::setSystrayAllConfirmation(bool isEnabled)
{
	typotek::getInstance()->systrayAllConfirmation(isEnabled);
}

void PrefsPanelDialog::setSystrayTagsConfirmation(bool isEnabled)
{
	typotek::getInstance()->systrayTagsConfirmation(isEnabled);
}

void PrefsPanelDialog::updateWord(QString s)
{
	typotek::getInstance()->setPreviewSize(previewSizeSpin->value());
	typotek::getInstance()->setWord(s, true);
}

void PrefsPanelDialog::updateWordSize(double d)
{
	
	QSettings settings;
	settings.setValue("PreviewSize", d);
	typotek::getInstance()->setPreviewSize(d);
	typotek::getInstance()->setWord(previewWord->text(), true);
}


void PrefsPanelDialog::setupFontEditor(QString s)
{
	typotek::getInstance()->setFontEditorPath(s);
}

void PrefsPanelDialog::slotFontEditorBrowse()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Select font editor"));
	if (!s.isEmpty()) {
		fontEditorPath->setText(s);
	}
}

void PrefsPanelDialog::showPage(PAGE page)
{
	if(page == PAGE_GENERAL)
		mainTab->setCurrentIndex ( 0 );
	else if(page == PAGE_SAMPLETEXT)
		mainTab->setCurrentIndex ( 1 );
	else if(page == PAGE_FILES)
		mainTab->setCurrentIndex ( 2 );
}

void PrefsPanelDialog::slotTemplatesBrowse()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Select Templates Folder"), QDir::homePath(), QFileDialog::ShowDirsOnly);
	if (!s.isEmpty()) {
		templatesFolder->setText(s);
	}
}

void PrefsPanelDialog::setupTemplates(const QString &tdir)
{
	if(!tdir.isEmpty())
		typotek::getInstance()->setTemplatesDir(tdir);
}




