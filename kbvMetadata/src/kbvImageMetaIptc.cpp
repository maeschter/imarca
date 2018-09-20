/*****************************************************************************
 * kvb image meta IPTC
 * This file contains functions to handle the IPTC part of meta data gui.
 * (C): G. Trauth, Erlangen
 * LastChangedDate: 2018-09-16
 * Created: 2015.04.27
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *
 * The IPTC group encourages people to move from IPTC-IIM to its newer IPTC Core
 * IPTC Extension standard. Reading and writing IPTC follows the guide
 * (mwg_guidance.pdf).
 * Reading IPTC (consumer):
 *                                      |
 *                    -------n----- IPTC-IIM?
 *                   |                  |
 *          ---n--- XMP?       ---n--- XMP?
 *         |         |        |         |
 *        end      read     read    checksum ---n---
 *                  XMP   IPTC-IIM      |           |
 *                                   matches ---y---
 *                                      |           |
 *                                    read        read
 *                                  IPTC-IIM   XMP & IPTC-IIM
 *                                             prefer XMP
 *
 * Writing IPTC
 * Creator: Write Core & Extension or convert from IPTC-IIM
 *          If IPTC-IIM and XMP are both written, create the checksum
 * Changer: If IPTC-IIM is already in the file, write back XMP and IPTC-IIM
 *          otherwise only XMP SHOULD be written.
 *          Use Coded Character Set (1:90) as UTF-8
 *          If IPTC-IIM and XMP are both present, create or update the checksum
 *          If no metadata are present, act as creator
 * The checksum over the entire IPTC-IIM block must be stored in the Photoshop
 * Image Resource 1061 (photoshop files) as 16 byte MD5 hash.
  *****************************************************************************/
#include <QtDebug>
#include <QFileDialog>
#include <kbvConstants.h>
#include "kbvImageMetaData.h"

/*************************************************************************//*!
 * Set IPTC core field names in table column 0. The order of item entry
 * defines the structure of the table and must be identical to the enum.
 */
void  KbvImageMetaData::iptcPresetTable()
{
  QTableWidgetItem  *tag, *value;
  tag = new QTableWidgetItem(QTableWidgetItem::Type);
  tag->setFlags(Qt::ItemIsEnabled);
  value = new QTableWidgetItem(QTableWidgetItem::Type);
  value->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);

  uiIptcData.iptcTagTable->clearContents();
  while(uiIptcData.iptcTagTable->rowCount()>0)
    {
      uiIptcData.iptcTagTable->removeRow(0);
    }
  tag->setText("Creator");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Byline);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Byline, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Byline, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Byline, KbvMeta::valueCol)->setToolTip(QString("optional, multiple entries, Text\n"
                               "Enter the name of the person that created this image"));
  tag->setText("Creator's Jobtitle");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::BylineTitle);
  uiIptcData.iptcTagTable->setItem(KbvMeta::BylineTitle, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::BylineTitle, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::BylineTitle, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter the Job Title of the person listed in the Creator field"));
  tag->setText("Date / Time created");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::DateTime);
  uiIptcData.iptcTagTable->setItem(KbvMeta::DateTime, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::DateTime, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::DateTime, KbvMeta::valueCol)->setToolTip(QString("ISO8601 Date/Time definition\n"
                                "Enter the Date and the Time the image was taken"));
  tag->setText("Intellectual genre");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Genre);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Genre, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Genre, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Genre, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, IPTC Genre NewsCodes\n"
                               "Enter a term to describe the nature of the image in terms of its intellectual or journalistic\n"
                               "characteristics, such as daybook, or feature\n"
                               "e.g. use code <genre:Response_to_a_Question> for genre <Response to a Question>"));
  tag->setText("Scene Code");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Scene);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Scene, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Scene, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Scene, KbvMeta::valueCol)->setToolTip(QString("optional, multiple entries, IPTC Scene NewsCode\n"
                               "Enter only values from the IPTC Scene NewsCodes Controlled Vocabulary\n"
                               "e.g. use code <iso4217n> for scene <ISO currency codes - ISO 4217 - numeric codes>"));
  tag->setText("Sublocation");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Sublocation);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Sublocation, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Sublocation, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Sublocation, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter the name of the Sublocation pictured in this image"));
  tag->setText("City");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::City);
  uiIptcData.iptcTagTable->setItem(KbvMeta::City, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::City, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::City, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter the name of the city pictured in this image"));
  tag->setText("Province/State");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::State);
  uiIptcData.iptcTagTable->setItem(KbvMeta::State, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::State, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::State, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter the name of the province or state pictured in this image"));
  tag->setText("Country");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Country);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Country, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Country, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Country, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter the name of the country pictured in this image"));
  tag->setText("ISO Country Code");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::CountryCode);
  uiIptcData.iptcTagTable->setItem(KbvMeta::CountryCode, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::CountryCode, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::CountryCode, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter the 2 or 3 letter ISO 3166 Country Code of the Country pictured in this image"));
  tag->setText("Headline");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Headline);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Headline, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Headline, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Headline, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter a brief publishable synopsis or summary of the contents of the image"));
  tag->setText("Keywords");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Keywords);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Keywords, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Keywords, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Keywords, KbvMeta::valueCol)->setToolTip(QString("optional, multiple entries, Text\n"
                               "Enter any number of keywords, terms or phrases used to express the subject matter in the image"));
  tag->setText("Subject Code");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Subject);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Subject, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Subject, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Subject, KbvMeta::valueCol)->setToolTip(QString("optional, multiple entries, IPTC Subject NewsCode\n"
                               "Enter only values from the IPTC Subject NewsCode Controlled Vocabulary\n"
                               "e.g. code <01000000> for subject <arts, culture and entertainment>"));
  tag->setText("Caption/Description Writer");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Writer);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Writer, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Writer, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Writer, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter the name of the person involved in writing, editing or correcting the description of the image"));
  tag->setText("Title");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::ObjectName);
  uiIptcData.iptcTagTable->setItem(KbvMeta::ObjectName, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::ObjectName, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::ObjectName, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter a short verbal and human readable name for the image,\n this may be the file name"));
  tag->setText("Job Identifier");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::JobId);
  uiIptcData.iptcTagTable->setItem(KbvMeta::JobId, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::JobId, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::JobId, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter a number or identifier needed for workflow control or tracking"));
  tag->setText("Credit Line");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Credit);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Credit, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Credit, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Credit, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter who should be credited when this image is published"));
  tag->setText("Source");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Source);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Source, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Source, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Source, KbvMeta::valueCol)->setToolTip(QString("optional, single entry, Text\n"
                               "Enter or edit the name of a person or party who has a role in the content supply chain,\n such as a person or entity from whom you received this image from"));
  tag->setText("Copyright Notice");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Copyright);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Copyright, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Copyright, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Copyright, KbvMeta::valueCol)->setToolTip(QString("optional, Text\n"
                               "Enter a Notice on the current owner of the Copyright for this image, such as (c)2008 Jane Doe"));
  tag->setText("Rights Usage Terms");
  uiIptcData.iptcTagTable->insertRow(KbvMeta::Rights);
  uiIptcData.iptcTagTable->setItem(KbvMeta::Rights, KbvMeta::tagCol, tag->clone());
  uiIptcData.iptcTagTable->setItem(KbvMeta::Rights, KbvMeta::valueCol, value->clone());
  uiIptcData.iptcTagTable->item(KbvMeta::Rights, KbvMeta::valueCol)->setToolTip(QString("optional, Text\n"
                               "Enter instructions on how this image can legally be used"));

  delete tag;
  delete value;
}
/*************************************************************************//*!
 * Open editor for IPTC template.
 * A template is special set of data which can replace or amend existing data.
 * This is a convenient function.
 */
void  KbvImageMetaData::openIptcTemplate()
{
  uiMetaDataFile.fileIcon->clear();
  uiMetaDataFile.fileName->clear();

  //Disable all tabs except IPTC
  for(int i=1; i<metaDataTab->count(); i++)
    {
      this->metaDataTab->setTabEnabled(i, false);
    }
  this->metaDataTab->setTabText(0, QString("IPTC template"));
  uiIptcData.iptcButtonSet->setEnabled(false);
  this->metaDataVLayout->removeWidget(metaDataButt);
  metaDataButt->hide();
  this->metaDataVLayout->addWidget(iptcTemplateButt);
  iptcTemplateButt->show();
  this->show();
}
/*************************************************************************//*!
 * Copy all non-empty GUI fields to IPTC meta data container by overwriting
 * existent or adding new datasets. Text is coded in UTF-8.
 */
void  KbvImageMetaData::iptcGuiToIptcData()
{
  QByteArray    ba;
  QString       text;
  QStringList   newlist;

  //Clear iptc container then add non empty values
  this->metaData.clearIptc();

  //TODO: mapping to XMP
  //Rights Usage Terms, 0..1
  //maps to Xmp.xmpRights.UsageTerms, Value Type: Lang Alt
  text = uiIptcData.iptcTagTable->item(KbvMeta::Rights, KbvMeta::valueCol)->text();
  rightsUsageTerms = text;

  //IPTC scene code, 0..n
  //maps to Xmp.Iptc4xmpCore.Scene, Value Type: bag closed choice text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Scene, KbvMeta::valueCol)->text();
  iptcSceneCode = text;

  //1:90 Coded Character Set
  metaData.setIptcCharacterSet(KbvExiv::IptcUTF8);

  //2:00 Record Version, max. 2 octets
  text = "4";
  metaData.setIptcTagDataString("Iptc.Application2.RecordVersion", text);

  //2:65 Originating Program, max. 32 octets
  text = QString(appName);
  metaData.setIptcTagDataString("Iptc.Application2.Program", text);
  //2:70 Program Version, max. 10 octets
  text = QString(appVersion);
  metaData.setIptcTagDataString("Iptc.Application2.ProgramVersion", text);

  //2:04 Intellectual genre (Object attribute reference, repeatable, max. 68 octets)
  //maps to Xmp.Iptc4xmpCore.IntellectualGenre, Value Type: text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Genre, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.ObjectAttribute", ba);
    }

  //2:05 Title (Object name, max. 64 octets)
  //maps to Xmp.dc.title, Value Type: Lang Alt
  text = uiIptcData.iptcTagTable->item(KbvMeta::ObjectName, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.ObjectName", ba);
    }

  //2:12 Subject code (Subject reference, repeatable, max. 236 octets)
  //maps to Xmp.Iptc4xmpCore.SubjectCode, Value Type: bag closed choice Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Subject, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      newlist = text.split(" ", QString::SkipEmptyParts);
      metaData.setIptcSubjects(newlist);
    }

  //2:25 Keywords (repeatable, max. 64 octets)
  //maps to Xmp.dc.subject, Value Type: bag Text
  //Keywords consist of graphic characters plus spaces.
  //The graphic character subset includes all visible characters.
  //Control codes, space character and DEL are not graphic characters.
  text = uiIptcData.iptcTagTable->item(KbvMeta::Keywords, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      newlist = text.split(" ", QString::SkipEmptyParts);
      metaData.setIptcKeywords(newlist);
    }

  //2:55 Date Created, 8 octets and 2:60 Time Created, 11 octets
  //merged into 2:55, when possible use Exif Date/Time created
  //maps to Xmp.photoshop.DateCreated, Value Type: Date
  text = uiIptcData.iptcTagTable->item(KbvMeta::DateTime, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      text.remove("-");
      text.remove(":");
      newlist = text.split(" ", QString::SkipEmptyParts);
      if(newlist.size() > 0)
        {
          ba = newlist.at(0).toLatin1();
          metaData.setIptcTagData("Iptc.Application2.DateCreated", ba);
          if(newlist.size() > 1)
            {
              ba = newlist.at(1).toLatin1();
              metaData.setIptcTagData("Iptc.Application2.TimeCreated", ba);
            }
        }
    }

  //TODO: repeatable creators
  //2:80 Creator (By-line, repeatable, max. 32 octets)
  //maps to Xmp.dc.creator, Value Type: Seq ProperName
  text = uiIptcData.iptcTagTable->item(KbvMeta::Byline, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.Byline", ba);
    }

  //2:85 Creators Jobtitle (By-line title, max. 32 octets)
  //maps to Xmp.photoshop.AuthorsPosition, Value Type: text
  text = uiIptcData.iptcTagTable->item(KbvMeta::BylineTitle, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.BylineTitle", ba);
    }

  //2:90 City, max. 32 octets
  //maps to Xmp.photoshop.city, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::City, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.City", ba);
    }

  //2:92 Sublocation, max. 32 octets
  //maps to Xmp.Iptc4xmpCore.Location, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Sublocation, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.SubLocation", ba);
    }

  //2:95 Province/State, max. 32 octets
  //maps to Xmp.photoshop.state, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::State, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.ProvinceState", ba);
    }

  //2:100 Country code, 3 octets
  //maps to Xmp.Iptc4xmpCore.CountryCode, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::CountryCode, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagDataString("Iptc.Application2.CountryCode", ba);
    }

  //2:101 Country, max. 64 octets
  //maps to Xmp.photoshop.Country, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Country, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.CountryName", ba);
    }

  //2:103 Job identifier (Orig. transmission reference, max. 32 octets)
  //maps to Xmp.photoshop.TransmissionReference, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::JobId, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.TransmissionReference", ba);
    }

  //2:105 Headline, max. 256 octets
  //maps to Xmp.photoshop.Headline, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Headline, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.Headline", ba);
    }

  //2:110 Credit, max. 32 octets
  //maps to Xmp.photoshop.Credit, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Credit, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.Credit", ba);
    }

  //2:115 Source, max. 32 octets
  //maps to Xmp.photoshop.Source, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Source, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.Source", ba);
    }

  //2:116 Copyright (copyright, max. 128 octets)
  //maps to Xmp.dc.rights, Value Type: Lang Alt
  text = uiIptcData.iptcTagTable->item(KbvMeta::Copyright, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.Copyright", ba);
    }

  //2:120 Caption, max. 2000 octets
  //maps to Xmp.dc.description, Value Type: Lang Alt
  text = uiIptcData.iptcCaption->toPlainText();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.Caption", ba);
    }

  //2:122 Caption writer (Writer/Editor, max. 32 octets)
  //maps to Xmp.photoshop.CaptionWriter, Value Type: Text
  text = uiIptcData.iptcTagTable->item(KbvMeta::Writer, KbvMeta::valueCol)->text();
  if(!text.isEmpty())
    {
      ba = text.toUtf8();
      metaData.setIptcTagData("Iptc.Application2.Writer", ba);
    }
}
/*************************************************************************//*!
 * Read IPTC data set into GUI.\n
 * Due to mwg-guidance v2.0 (p.32) IPTC-IIM metadata MUST be read as UTF-8 if
 * a 1:90 DataSet is present indicating the use of UTF-8. IIM metadata MUST be
 * read using the appropriate character set if a 1:90 DataSet is present,
 * otherwise it MAY be ignored.
 * IIM without a 1:90 DataSet: If all bytes are in the range 0..127 then the
 * character set is ASCII. If the entire sequence is valid UTF-8 then the
 * character set is UTF-8. Otherwise assume some a fallback charSet, or ignore
 * the value. Fallback:
 * 1:90 indicates UTF-8 or is not present: read as UTF-8.
 * 1:90 indicates ISO 8859-1: read as ISO 8859-1
 */
bool  KbvImageMetaData::iptcDataToGui()
{
  QString       str;
  QStringList   strlist;
  int           charSet;

  if(!metaData.hasIptc())
    {
      //qDebug() << "KbvImageMetaData::metaDataFromFile no IPTC"; //###########
      return false;
    }

  charSet = metaData.getIptcCharacterSet();

  //TODO: Iptc deprecated elements:
  //2:10 Urgency, 1 octet, maps to Xmp.photoshop.Urgency
  //2:15 Category, max. 3 octets, maps to Xmp.photoshop.Category
  //2:20 Supplemental Categories, repeatable, max. 32 octets, maps to Xmp.photoshop.SupplementalCategories
  //for further use im XMP


  //2:04 Intellectual genre (Object attribute reference)
  str = metaData.getIptcTagDataString("Iptc.Application2.ObjectAttribute", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Genre, KbvMeta::valueCol)->setText(str);

  //2:05 Title (Object name)
  str = metaData.getIptcTagDataString("Iptc.Application2.ObjectName", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::ObjectName, KbvMeta::valueCol)->setText(str);
  //qDebug() << "KbvImageMetaData Object name"<<str; //###########

  //TODO: IPTC repeatable subjects
  //2:12 Subject code (Subject reference)
  strlist = metaData.getIptcTagStringList("Iptc.Application2.Subject", true, charSet);
  //strlist = metaData.getIptcSubjects();
  str.clear();
  for(int i=0; i<strlist.length(); i++)
    {
      str.append(strlist.at(i) + " ");
    }
  uiIptcData.iptcTagTable->item(KbvMeta::Subject, KbvMeta::valueCol)->setText(str);

  //2:25 Keywords
  strlist = metaData.getIptcKeywords(charSet);
  str.clear();
  for(int i=0; i<strlist.length(); i++)
    {
      str.append(strlist.at(i) + " ");
    }
  uiIptcData.iptcTagTable->item(KbvMeta::Keywords, KbvMeta::valueCol)->setText(str);

  //2:55 Date Created ISO8601
  str = QString::fromLatin1(metaData.getIptcTagData("Iptc.Application2.DateCreated"));
  if(8 == str.length())
    {
      str.insert(4,"-");
      str.insert(7, "-");
    }
  strlist.clear();
  strlist.append(str);

  //2:60 Time Created ISO8601
  str = QString::fromLatin1(metaData.getIptcTagData("Iptc.Application2.TimeCreated"));
  if(6 == str.length())
    {
      str.insert(2,":");
      str.insert(5, ":");
    }
  if(11 == str.length())
    {
      str.insert(2,":");
      str.insert(5, ":");
      str.insert(11, ":");
    }
  str = strlist.at(0) + "  " + str;
  uiIptcData.iptcTagTable->item(KbvMeta::DateTime, KbvMeta::valueCol)->setText(str);

  //2:80 Creator (By-line)
  str = metaData.getIptcTagDataString("Iptc.Application2.Byline", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Byline, KbvMeta::valueCol)->setText(str);

  //2:85 Creators Jobtitle (By-line title)
  str = metaData.getIptcTagDataString("Iptc.Application2.BylineTitle", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::BylineTitle, KbvMeta::valueCol)->setText(str);

  //2:90 City
  str = metaData.getIptcTagDataString("Iptc.Application2.City", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::City, KbvMeta::valueCol)->setText(str);

  //2:92 Sublocation
  str = metaData.getIptcTagDataString("Iptc.Application2.SubLocation", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Sublocation, KbvMeta::valueCol)->setText(str);

  //2:95 Province/State
  str = metaData.getIptcTagDataString("Iptc.Application2.ProvinceState", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::State, KbvMeta::valueCol)->setText(str);

  //2:100 Country code
  str = metaData.getIptcTagDataString("Iptc.Application2.CountryCode", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::CountryCode, KbvMeta::valueCol)->setText(str);

  //2:101 Country
  str = metaData.getIptcTagDataString("Iptc.Application2.CountryName", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Country, KbvMeta::valueCol)->setText(str);

  //2:103 Job identifier (Orig. transmission reference)
  str = metaData.getIptcTagDataString("Iptc.Application2.TransmissionReference", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::JobId, KbvMeta::valueCol)->setText(str);

  //2:105 Headline
  str = metaData.getIptcTagDataString("Iptc.Application2.Headline", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Headline, KbvMeta::valueCol)->setText(str);

  //2:110 Credit
  str = metaData.getIptcTagDataString("Iptc.Application2.Credit", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Credit, KbvMeta::valueCol)->setText(str);

  //2:115 Source
  str = metaData.getIptcTagDataString("Iptc.Application2.Source", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Source, KbvMeta::valueCol)->setText(str);

  //2:116 Copyright
  str = metaData.getIptcTagDataString("Iptc.Application2.Copyright", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Copyright, KbvMeta::valueCol)->setText(str);

  //2:120 Caption
  str = metaData.getIptcTagDataString("Iptc.Application2.Caption", charSet);
  uiIptcData.iptcCaption->setPlainText(str);

  //2:122 Caption writer (Writer/Editor)
  str = metaData.getIptcTagDataString("Iptc.Application2.Writer", charSet);
  uiIptcData.iptcTagTable->item(KbvMeta::Writer, KbvMeta::valueCol)->setText(str);

  return true;
}
/*************************************************************************//*!
 * SLOT: button IPTC date/time: set current date/time as ISO8601 representation.
 */
void  KbvImageMetaData::iptcDate(bool checked)
{
  Q_UNUSED(checked)
  QDateTime     datetime;
  QString       dt;

  datetime = QDateTime::currentDateTime();
  datetime.setTimeSpec(Qt::OffsetFromUTC);

  dt = datetime.toString(Qt::ISODate).replace("T", "   ");
  uiIptcData.iptcTagTable->item(KbvMeta::DateTime, KbvMeta::valueCol)->setText(dt);
}
/*************************************************************************//*!
 * SLOT: button IPTC country: insert country name and code.
 */
void  KbvImageMetaData::iptcCountry(int index)
{
  QString str, str1;

  str = iptcCountryBox->getCountryFromIndex(index);
  if(!str.isEmpty())
    {
      //qDebug() << "KbvImageMetaData::iptcCountry"<<str; //###########
      str1 = str.left(3);
      uiIptcData.iptcTagTable->item(KbvMeta::CountryCode, KbvMeta::valueCol)->setText(str1);
      str1 = str.remove(0, 6);
      uiIptcData.iptcTagTable->item(KbvMeta::Country, KbvMeta::valueCol)->setText(str1);
    }
}
/*************************************************************************//*!
 * SLOT: button IPTC genre: insert genre codes.
 */
void  KbvImageMetaData::iptcGenre(bool checked)
{
  Q_UNUSED(checked)
  QStringList       paramsList;

  paramsList.append("http://cv.iptc.org/newscodes/genre/");
  openApplication("application/xhtml+xml", paramsList);
}
/*************************************************************************//*!
 * SLOT: button IPTC scene code: insert scene codes.
 */
void  KbvImageMetaData::iptcScene(bool checked)
{
  Q_UNUSED(checked)
  QStringList       paramsList;

  paramsList.append("http://cv.iptc.org/newscodes/scene/");
  openApplication("application/xhtml+xml", paramsList);
}
/*************************************************************************//*!
 * SLOT: button copyright sign: append © to text of copyright field.
 */
void  KbvImageMetaData::iptcCopyrrightSign(bool checked)
{
  Q_UNUSED(checked)
  QString   text;

  text = uiIptcData.iptcTagTable->item(KbvMeta::Copyright, KbvMeta::valueCol)->text();
  text.append(QString::fromUtf8("©"));
  uiIptcData.iptcTagTable->item(KbvMeta::Copyright, KbvMeta::valueCol)->setText(text);
}
/*************************************************************************//*!
 * SLOT: button IPTC subject code: insert subject codes.
 */
void  KbvImageMetaData::iptcSubject(bool checked)
{
  Q_UNUSED(checked)
  QStringList       paramsList;

  paramsList.append("http://cv.iptc.org/newscodes/subjectcode/");
  openApplication("application/xhtml+xml", paramsList);
}
/*************************************************************************//*!
 * SLOT: button IPTC data clear. Remove all data set values.
 */
void  KbvImageMetaData::iptcClear(bool checked)
{
  Q_UNUSED(checked)

  this->iptcClearGui();
}
/*************************************************************************//*!
 * Clear all IPTC gui data.
 */
void  KbvImageMetaData::iptcClearGui()
{
  QTableWidgetItem  *item;

  for(int i=0; i<uiIptcData.iptcTagTable->rowCount(); i++)
    {
      item = uiIptcData.iptcTagTable->item(i, KbvMeta::valueCol);
      if(item->type() == QTableWidgetItem::Type)
        {
          item->setText("");
        }
    }
  uiIptcData.iptcCaption->clear();
}
/*************************************************************************//*!
 * SLOT: button set data from template. The active template file is read and
 * non-empty fields are copied to the GUI.
 */
void  KbvImageMetaData::iptcSetFromTemplate(bool checked)
{
  Q_UNUSED(checked)

  if(!templateActive.isEmpty())
    {
      xmlFile->setFileName(templateActive);
      if(xmlFile->open(QIODevice::ReadOnly))
        {
          iptcReadTemplate(xmlFile);
          xmlFile->close();
        }
      else
        {
          //Display warning on file error: caption, info, icon
          QMessageBox::warning(this, errorMsg.arg(templateActive), helpMsg);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Open existing template file.
 */
void  KbvImageMetaData::iptcTemplateOpen(bool checked)
{
  Q_UNUSED(checked)

  if(templateFile.isEmpty())
    {
      templateFile = QDir::homePath();
    }
  templateFile = QFileDialog::getOpenFileName(this, tr("Open template"), templateFile, tr("XML template files (*.xml)"));

  //qDebug() << "KbvImageMetaData::openTemplate"<<templateFile; //###########
  if(!templateFile.isEmpty())
    {
      xmlFile->setFileName(templateFile);
      if(xmlFile->open(QIODevice::ReadOnly))
        {
          iptcReadTemplate(xmlFile);
          xmlFile->close();
        }
      else
        {
          //Display warning on file error: caption, info, icon
          QMessageBox::warning(this, errorMsg.arg(templateFile), helpMsg);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Save GUI to existing template file.
 */
void  KbvImageMetaData::iptcTemplateSave(bool checked)
{
  Q_UNUSED(checked)
  if(!templateFile.isEmpty())
    {
      xmlFile->setFileName(templateFile);
      xmlFile->resize(0);
      if(xmlFile->open(QIODevice::ReadWrite))
        {
          //qDebug() << "KbvImageMetaData::saveTemplate"; //###########
          iptcWriteTemplate(xmlFile);
          xmlFile->close();
        }
      else
        {
          //Display warning on file error: caption, info, icon
          QMessageBox::warning(this, errorMsg.arg(templateFile), helpMsg);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Save GUI as new template file.
 */
void  KbvImageMetaData::iptcTemplateSaveAs(bool checked)
{
  Q_UNUSED(checked)

  if(templateFile.isEmpty())
    {
      templateFile = QDir::homePath();
    }
  templateFile = QFileDialog::getSaveFileName(this, tr("Save template as"), templateFile, tr("XML template files (*.xml)"));
  if(!templateFile.isEmpty())
    {
      xmlFile->setFileName(templateFile);
      xmlFile->resize(0);
      if(xmlFile->open(QIODevice::ReadWrite))
        {
          //qDebug() << "KbvImageMetaData::saveAsTemplate"; //###########
          iptcWriteTemplate(xmlFile);
          xmlFile->close();
        }
      else
        {
          //Display warning on file error: caption, info, icon
          QMessageBox::warning(this, errorMsg.arg(templateFile), helpMsg);
        }
    }
}
/*************************************************************************//*!
 * SLOT: Set current template file for use in IPTC editor.
 */
void  KbvImageMetaData::iptcTemplateActivate(bool checked)
{
  Q_UNUSED(checked)
  QString   file;

  //qDebug() << "KbvImageMetaData::activateTemplate"; //###########
  if(!templateFile.isEmpty())
    {
      templateActive = templateFile;
      file = templateActive.right(templateActive.length() - templateActive.lastIndexOf("/") - 1);
      this->setWindowTitle(this->windowTitle() + " - " + file);
    }
}
/*************************************************************************//*!
 * Read template from XML file and wite all non-empty attributes into GUI.\n
 * The order of the attributes is defined by the write function and must match
 * the corresponding elements of the GUI.
 */
void  KbvImageMetaData::iptcReadTemplate(QIODevice *device)
{
  bool      ok=true;
  QString   attrib;

  xmlreader.setDevice(device);

  if(xmlreader.readNextStartElement())
    {
      if(xmlreader.name() == "ImarcaIptcTemplate" && xmlreader.attributes().value("version") == "1.0")
        {
          if(xmlreader.readNextStartElement())
            {
              if(xmlreader.name() == "IptcCore")
                {
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("object").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::ObjectName, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("date").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::DateTime, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("creator").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Byline, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("city").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::City, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("sublocation").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Sublocation, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("state").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::State, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("country").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Country, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("countrycode").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::CountryCode, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("captionwriter").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Writer, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("credit").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Credit, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("jobtitle").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::BylineTitle, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("source").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Source, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("jobid").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::JobId, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("genre").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Genre, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("scene").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Scene, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("subject").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Subject, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("copyright").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Copyright, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("rights").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Rights, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("caption").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcCaption->setPlainText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("keywords").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Keywords, KbvMeta::valueCol)->setText(attrib);}
                  xmlreader.readNext();
                  xmlreader.readNextStartElement();
                  attrib = xmlreader.attributes().value("headline").toString();
                  if(!attrib.isEmpty()) {uiIptcData.iptcTagTable->item(KbvMeta::Headline, KbvMeta::valueCol)->setText(attrib);}
                }
              else
                {
                  ok = false;
                }
            }
          else
            {
              ok = false;
            }
        }
      else
        {
          ok = false;
        }
    }
  else
    {
      ok = false;
    }
  if(!ok)
    {
      QMessageBox::warning(this, errorMsg.arg(templateFile), helpMsg);
    }
}
/*************************************************************************//*!
 * Write template as XML file. The order of the attributes is defines the
 * read sequence.
 */
void  KbvImageMetaData::iptcWriteTemplate(QIODevice *device)
{
  QString date;

  date = QDate::currentDate().toString("dd.MM.yyyy");

  xmlwriter.setDevice(device);
  xmlwriter.setAutoFormatting(true);
  xmlwriter.writeStartDocument("1.0", true);

  xmlwriter.writeStartElement("ImarcaIptcTemplate");
  xmlwriter.writeAttribute("version", "1.0");
  xmlwriter.writeAttribute("lastchange", date);
  xmlwriter.writeStartElement("IptcCore");
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("object", uiIptcData.iptcTagTable->item(KbvMeta::ObjectName, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("date", uiIptcData.iptcTagTable->item(KbvMeta::DateTime, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("creator", uiIptcData.iptcTagTable->item(KbvMeta::Byline, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("city", uiIptcData.iptcTagTable->item(KbvMeta::City, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("sublocation", uiIptcData.iptcTagTable->item(KbvMeta::Sublocation, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("state", uiIptcData.iptcTagTable->item(KbvMeta::State, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("country", uiIptcData.iptcTagTable->item(KbvMeta::Country, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("countrycode", uiIptcData.iptcTagTable->item(KbvMeta::CountryCode, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("captionwriter", uiIptcData.iptcTagTable->item(KbvMeta::Writer, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("credit", uiIptcData.iptcTagTable->item(KbvMeta::Credit, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("jobtitle", uiIptcData.iptcTagTable->item(KbvMeta::BylineTitle, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("source", uiIptcData.iptcTagTable->item(KbvMeta::Source, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("jobid", uiIptcData.iptcTagTable->item(KbvMeta::JobId, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("genre", uiIptcData.iptcTagTable->item(KbvMeta::Genre, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("scene", uiIptcData.iptcTagTable->item(KbvMeta::Scene, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("subject", uiIptcData.iptcTagTable->item(KbvMeta::Subject, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("copyright", uiIptcData.iptcTagTable->item(KbvMeta::Copyright, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("rights", uiIptcData.iptcTagTable->item(KbvMeta::Rights, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("caption", uiIptcData.iptcCaption->toPlainText());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("keywords", uiIptcData.iptcTagTable->item(KbvMeta::Keywords, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
    xmlwriter.writeStartElement("iptc");
    xmlwriter.writeAttribute("headline", uiIptcData.iptcTagTable->item(KbvMeta::Headline, KbvMeta::valueCol)->text());
    xmlwriter.writeEndElement();
  xmlwriter.writeEndElement();  //IptcCore
  xmlwriter.writeEndElement();  //ImarcaIptcTemplate
  xmlwriter.writeEndDocument(); //Device
}


/****************************************************************************/
