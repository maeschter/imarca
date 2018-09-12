/*****************************************************************************
 * kbvCountrySelector
 * Select Country and ISO country code
 * Derived from libkexiv2
 * (C): G. Trauth, Erlangen
 * $LastChangedDate: 2017-03-28 21:01:06 +0200 (Di, 28. Mär 2017) $
 * $Rev: 1199 $
 * Created: 2015.10.18
 * This program is free software under the terms of the GNU General Public License,
 * either version 3 of the License, or (at your option) any later version.
 * For details see the GNU General Public License <http://www.gnu.org/licenses/>.
 *****************************************************************************/
#include <QtDebug>
#include "kbvCountrySelector.h"

//std constructor
KbvCountrySelector::KbvCountrySelector(QWidget *parent) : QComboBox(parent)
{
  this->setupMap();
  CountryMap::iterator iter = countryCodeMap.begin();
  while(iter != countryCodeMap.end())
    {
      this->addItem(QString("%1 - %2").arg(iter.key()).arg(iter.value()));
      iter++;
    }

  model()->sort(0);

  //this->insertSeparator(this->count());             //->getCountryFromIndex()
  //this->addItem(tr("Unknown country", "Unknown"));
}

//copy constructor
KbvCountrySelector::KbvCountrySelector(const KbvCountrySelector &d) : QComboBox()
{
  countryCodeMap = d.countryCodeMap;
}

KbvCountrySelector::~KbvCountrySelector()
{

}

KbvCountrySelector& KbvCountrySelector::operator=(const KbvCountrySelector &d)
{
  if(this != &d)
    {
      countryCodeMap = d.countryCodeMap;
    }
  return *this;
}

/*************************************************************************//*!
*/
QString KbvCountrySelector::getCountryFromIndex(int index)
{
  //qDebug() << "KbvCountrySelector::getCountryFromIndex"<<index; //###########
  //don't consider last 2 entrys when the list contains separator and unknown
  if(index < this->count())
    {
      return this->itemText(index);
    }
  return QString();
}
/*************************************************************************//*!
 * ISO 3166-1 alpha-3 country codes
*/
void  KbvCountrySelector::setupMap()
{
    countryCodeMap.insert("AFG", tr("Afghanistan"));
    countryCodeMap.insert("ALB", tr("Albania"));
    countryCodeMap.insert("DZA", tr("Algeria"));
    countryCodeMap.insert("ASM", tr("American Samoa"));
    countryCodeMap.insert("AND", tr("Andorra"));
    countryCodeMap.insert("AGO", tr("Angola"));
    countryCodeMap.insert("AIA", tr("Anguilla"));
    countryCodeMap.insert("AGO", tr("Angola"));
    countryCodeMap.insert("ATA", tr("Antarctica"));
    countryCodeMap.insert("ATG", tr("Antigua and Barbuda"));
    countryCodeMap.insert("ARG", tr("Argentina"));
    countryCodeMap.insert("ARM", tr("Armenia"));
    countryCodeMap.insert("ABW", tr("Aruba"));
    countryCodeMap.insert("AUS", tr("Australia"));
    countryCodeMap.insert("AUT", tr("Austria"));
    countryCodeMap.insert("AZE", tr("Azerbaijan"));
    countryCodeMap.insert("BHS", tr("Bahamas"));
    countryCodeMap.insert("BHR", tr("Bahrain"));
    countryCodeMap.insert("BGD", tr("Bangladesh"));
    countryCodeMap.insert("BRB", tr("Barbados"));
    countryCodeMap.insert("BLR", tr("Belarus"));
    countryCodeMap.insert("BEL", tr("Belgium"));
    countryCodeMap.insert("BLZ", tr("Belize"));
    countryCodeMap.insert("BEN", tr("Benin"));
    countryCodeMap.insert("BMU", tr("Bermuda"));
    countryCodeMap.insert("BTN", tr("Bhutan"));
    countryCodeMap.insert("BOL", tr("Bolivia"));
    countryCodeMap.insert("BIH", tr("Bosnia and Herzegovina"));
    countryCodeMap.insert("BWA", tr("Botswana"));
    countryCodeMap.insert("BVT", tr("Bouvet Island"));
    countryCodeMap.insert("BRA", tr("Brazil"));
    countryCodeMap.insert("IOT", tr("British Indian Ocean Territory"));
    countryCodeMap.insert("VGB", tr("British Virgin Islands"));
    countryCodeMap.insert("BRN", tr("Brunei Darussalam"));
    countryCodeMap.insert("BGR", tr("Bulgaria"));
    countryCodeMap.insert("BFA", tr("Burkina Faso"));
    countryCodeMap.insert("BDI", tr("Burundi"));
    countryCodeMap.insert("KHM", tr("Cambodia"));
    countryCodeMap.insert("CMR", tr("Cameroon"));
    countryCodeMap.insert("CAN", tr("Canada"));
    countryCodeMap.insert("CPV", tr("Cape Verde"));
    countryCodeMap.insert("CYM", tr("Cayman Islands"));
    countryCodeMap.insert("CAF", tr("Central African Republic"));
    countryCodeMap.insert("TCD", tr("Chad"));
    countryCodeMap.insert("CHL", tr("Chile"));
    countryCodeMap.insert("CHN", tr("China"));
    countryCodeMap.insert("CXR", tr("Christmas Island "));
    countryCodeMap.insert("CCK", tr("Cocos Islands"));
    countryCodeMap.insert("COL", tr("Colombia"));
    countryCodeMap.insert("COM", tr("Comoros"));
    countryCodeMap.insert("COD", tr("Zaire"));
    countryCodeMap.insert("COG", tr("Congo"));
    countryCodeMap.insert("COK", tr("Cook Islands"));
    countryCodeMap.insert("CRI", tr("Costa Rica"));
    countryCodeMap.insert("CIV", tr("Ivory Coast"));
    countryCodeMap.insert("CUB", tr("Cuba"));
    countryCodeMap.insert("CYP", tr("Cyprus"));
    countryCodeMap.insert("CZE", tr("Czech Republic"));
    countryCodeMap.insert("DNK", tr("Denmark"));
    countryCodeMap.insert("DJI", tr("Djibouti"));
    countryCodeMap.insert("DMA", tr("Dominica"));
    countryCodeMap.insert("DOM", tr("Dominican Republic"));
    countryCodeMap.insert("ECU", tr("Ecuador"));
    countryCodeMap.insert("EGY", tr("Egypt"));
    countryCodeMap.insert("SLV", tr("El Salvador"));
    countryCodeMap.insert("GNQ", tr("Equatorial Guinea"));
    countryCodeMap.insert("ERI", tr("Eritrea"));
    countryCodeMap.insert("EST", tr("Estonia"));
    countryCodeMap.insert("ETH", tr("Ethiopia"));
    countryCodeMap.insert("FRO", tr("Faeroe Islands"));
    countryCodeMap.insert("FLK", tr("Falkland Islands"));
    countryCodeMap.insert("FJI", tr("Fiji Islands"));
    countryCodeMap.insert("FIN", tr("Finland"));
    countryCodeMap.insert("FRA", tr("France"));
    countryCodeMap.insert("GUF", tr("French Guiana"));
    countryCodeMap.insert("PYF", tr("French Polynesia"));
    countryCodeMap.insert("ATF", tr("French Southern Territories"));
    countryCodeMap.insert("GAB", tr("Gabon"));
    countryCodeMap.insert("GMB", tr("Gambia"));
    countryCodeMap.insert("GEO", tr("Georgia"));
    countryCodeMap.insert("DEU", tr("Germany"));
    countryCodeMap.insert("GHA", tr("Ghana"));
    countryCodeMap.insert("GIB", tr("Gibraltar"));
    countryCodeMap.insert("GRC", tr("Greece"));
    countryCodeMap.insert("GRL", tr("Greenland"));
    countryCodeMap.insert("GRD", tr("Grenada"));
    countryCodeMap.insert("GLP", tr("Guadaloupe"));
    countryCodeMap.insert("GUM", tr("Guam"));
    countryCodeMap.insert("GTM", tr("Guatemala"));
    countryCodeMap.insert("GIN", tr("Guinea"));
    countryCodeMap.insert("GNB", tr("Guinea-Bissau"));
    countryCodeMap.insert("GUY", tr("Guyana"));
    countryCodeMap.insert("HTI", tr("Haiti"));
    countryCodeMap.insert("HMD", tr("Heard and McDonald Islands"));
    countryCodeMap.insert("VAT", tr("Vatican"));
    countryCodeMap.insert("HND", tr("Honduras"));
    countryCodeMap.insert("HKG", tr("Hong Kong"));
    countryCodeMap.insert("HRV", tr("Croatia"));
    countryCodeMap.insert("HUN", tr("Hungary"));
    countryCodeMap.insert("ISL", tr("Iceland"));
    countryCodeMap.insert("IND", tr("India"));
    countryCodeMap.insert("IDN", tr("Indonesia"));
    countryCodeMap.insert("IRN", tr("Iran"));
    countryCodeMap.insert("IRQ", tr("Iraq"));
    countryCodeMap.insert("IRL", tr("Ireland"));
    countryCodeMap.insert("ISR", tr("Israel"));
    countryCodeMap.insert("ITA", tr("Italy"));
    countryCodeMap.insert("JAM", tr("Jamaica"));
    countryCodeMap.insert("JPN", tr("Japan"));
    countryCodeMap.insert("JOR", tr("Jordan"));
    countryCodeMap.insert("KAZ", tr("Kazakhstan"));
    countryCodeMap.insert("KEN", tr("Kenya"));
    countryCodeMap.insert("KIR", tr("Kiribati"));
    countryCodeMap.insert("PRK", tr("North-Korea"));
    countryCodeMap.insert("KOR", tr("South-Korea"));
    countryCodeMap.insert("KWT", tr("Kuwait"));
    countryCodeMap.insert("KGZ", tr("Kyrgyz Republic"));
    countryCodeMap.insert("LAO", tr("Lao"));
    countryCodeMap.insert("LVA", tr("Latvia"));
    countryCodeMap.insert("LBN", tr("Lebanon"));
    countryCodeMap.insert("LSO", tr("Lesotho"));
    countryCodeMap.insert("LBR", tr("Liberia"));
    countryCodeMap.insert("LBY", tr("Libyan Arab Jamahiriya"));
    countryCodeMap.insert("LIE", tr("Liechtenstein"));
    countryCodeMap.insert("LTU", tr("Lithuania"));
    countryCodeMap.insert("LUX", tr("Luxembourg"));
    countryCodeMap.insert("MAC", tr("Macao"));
    countryCodeMap.insert("MKD", tr("Macedonia"));
    countryCodeMap.insert("MDG", tr("Madagascar"));
    countryCodeMap.insert("MWI", tr("Malawi"));
    countryCodeMap.insert("MYS", tr("Malaysia"));
    countryCodeMap.insert("MDV", tr("Maldives"));
    countryCodeMap.insert("MLI", tr("Mali"));
    countryCodeMap.insert("MLT", tr("Malta"));
    countryCodeMap.insert("MHL", tr("Marshall Islands"));
    countryCodeMap.insert("MTQ", tr("Martinique"));
    countryCodeMap.insert("MRT", tr("Mauritania"));
    countryCodeMap.insert("MUS", tr("Mauritius"));
    countryCodeMap.insert("MYT", tr("Mayotte"));
    countryCodeMap.insert("MEX", tr("Mexico"));
    countryCodeMap.insert("FSM", tr("Micronesia"));
    countryCodeMap.insert("MDA", tr("Moldova"));
    countryCodeMap.insert("MCO", tr("Monaco"));
    countryCodeMap.insert("MNG", tr("Mongolia"));
    countryCodeMap.insert("MSR", tr("Montserrat"));
    countryCodeMap.insert("MAR", tr("Morocco"));
    countryCodeMap.insert("MOZ", tr("Mozambique"));
    countryCodeMap.insert("MMR", tr("Myanmar"));
    countryCodeMap.insert("NAM", tr("Namibia"));
    countryCodeMap.insert("NRU", tr("Nauru"));
    countryCodeMap.insert("NPL", tr("Nepal"));
    countryCodeMap.insert("ANT", tr("Netherlands Antilles"));
    countryCodeMap.insert("NLD", tr("Netherlands"));
    countryCodeMap.insert("NCL", tr("New Caledonia"));
    countryCodeMap.insert("NZL", tr("New Zealand"));
    countryCodeMap.insert("NIC", tr("Nicaragua"));
    countryCodeMap.insert("NER", tr("Niger"));
    countryCodeMap.insert("NGA", tr("Nigeria"));
    countryCodeMap.insert("NIU", tr("Niue"));
    countryCodeMap.insert("NFK", tr("Norfolk Island"));
    countryCodeMap.insert("MNP", tr("Northern Mariana Islands"));
    countryCodeMap.insert("NOR", tr("Norway"));
    countryCodeMap.insert("OMN", tr("Oman"));
    countryCodeMap.insert("PAK", tr("Pakistan"));
    countryCodeMap.insert("PLW", tr("Palau"));
    countryCodeMap.insert("PSE", tr("Palestinian Territory"));
    countryCodeMap.insert("PAN", tr("Panama"));
    countryCodeMap.insert("PNG", tr("Papua New Guinea"));
    countryCodeMap.insert("PRY", tr("Paraguay"));
    countryCodeMap.insert("PER", tr("Peru"));
    countryCodeMap.insert("PHL", tr("Philippines"));
    countryCodeMap.insert("PCN", tr("Pitcairn Island"));
    countryCodeMap.insert("POL", tr("Poland"));
    countryCodeMap.insert("PRT", tr("Portugal"));
    countryCodeMap.insert("PRI", tr("Puerto Rico"));
    countryCodeMap.insert("QAT", tr("Qatar"));
    countryCodeMap.insert("REU", tr("Reunion"));
    countryCodeMap.insert("ROU", tr("Romania"));
    countryCodeMap.insert("RUS", tr("Russian Federation"));
    countryCodeMap.insert("RWA", tr("Rwanda"));
    countryCodeMap.insert("SHN", tr("St. Helena"));
    countryCodeMap.insert("KNA", tr("St. Kitts and Nevis"));
    countryCodeMap.insert("LCA", tr("St. Lucia"));
    countryCodeMap.insert("SPM", tr("St. Pierre and Miquelon"));
    countryCodeMap.insert("VCT", tr("St. Vincent and the Grenadines"));
    countryCodeMap.insert("WSM", tr("Samoa"));
    countryCodeMap.insert("SMR", tr("San Marino"));
    countryCodeMap.insert("STP", tr("Sao Tome and Principe"));
    countryCodeMap.insert("SAU", tr("Saudi Arabia"));
    countryCodeMap.insert("SEN", tr("Senegal"));
    countryCodeMap.insert("SCG", tr("Serbia and Montenegro"));
    countryCodeMap.insert("SYC", tr("Seychelles"));
    countryCodeMap.insert("SLE", tr("Sierra Leone"));
    countryCodeMap.insert("SGP", tr("Singapore"));
    countryCodeMap.insert("SVK", tr("Slovakia"));
    countryCodeMap.insert("SVN", tr("Slovenia"));
    countryCodeMap.insert("SLB", tr("Solomon Islands"));
    countryCodeMap.insert("SOM", tr("Somalia"));
    countryCodeMap.insert("ZAF", tr("South Africa"));
    countryCodeMap.insert("SGS", tr("South Georgia and the South Sandwich Islands"));
    countryCodeMap.insert("ESP", tr("Spain"));
    countryCodeMap.insert("LKA", tr("Sri Lanka"));
    countryCodeMap.insert("SDN", tr("Sudan"));
    countryCodeMap.insert("SUR", tr("Suriname"));
    countryCodeMap.insert("SJM", tr("Svalbard & Jan Mayen Islands"));
    countryCodeMap.insert("SWZ", tr("Swaziland"));
    countryCodeMap.insert("SWE", tr("Sweden"));
    countryCodeMap.insert("CHE", tr("Switzerland"));
    countryCodeMap.insert("SYR", tr("Syrian Arab Republic"));
    countryCodeMap.insert("TWN", tr("Taiwan"));
    countryCodeMap.insert("TJK", tr("Tajikistan"));
    countryCodeMap.insert("TZA", tr("Tanzania"));
    countryCodeMap.insert("THA", tr("Thailand"));
    countryCodeMap.insert("TLS", tr("Timor-Leste"));
    countryCodeMap.insert("TGO", tr("Togo"));
    countryCodeMap.insert("TKL", tr("Tokelau Islands"));
    countryCodeMap.insert("TON", tr("Tonga"));
    countryCodeMap.insert("TTO", tr("Trinidad and Tobago"));
    countryCodeMap.insert("TUN", tr("Tunisia"));
    countryCodeMap.insert("TUR", tr("Turkey"));
    countryCodeMap.insert("TKM", tr("Turkmenistan"));
    countryCodeMap.insert("TCA", tr("Turks and Caicos Islands"));
    countryCodeMap.insert("TUV", tr("Tuvalu"));
    countryCodeMap.insert("VIR", tr("US Virgin Islands"));
    countryCodeMap.insert("UGA", tr("Uganda"));
    countryCodeMap.insert("UKR", tr("Ukraine"));
    countryCodeMap.insert("ARE", tr("United Arab Emirates"));
    countryCodeMap.insert("GBR", tr("United Kingdom"));
    countryCodeMap.insert("UMI", tr("United States Minor Outlying Islands"));
    countryCodeMap.insert("USA", tr("United States of America"));
    countryCodeMap.insert("URY", tr("Uruguay, Eastern Republic of"));
    countryCodeMap.insert("UZB", tr("Uzbekistan"));
    countryCodeMap.insert("VUT", tr("Vanuatu"));
    countryCodeMap.insert("VEN", tr("Venezuela"));
    countryCodeMap.insert("VNM", tr("Viet Nam"));
    countryCodeMap.insert("WLF", tr("Wallis and Futuna Islands "));
    countryCodeMap.insert("ESH", tr("Western Sahara"));
    countryCodeMap.insert("YEM", tr("Yemen"));
    countryCodeMap.insert("ZMB", tr("Zambia"));
    countryCodeMap.insert("ZWE", tr("Zimbabwe"));
    // Supplemental IPTC/IIM country codes.
    countryCodeMap.insert("XUN", tr("United Nations"));
    countryCodeMap.insert("XEU", tr("European Union"));
    countryCodeMap.insert("XSP", tr("Space"));
    countryCodeMap.insert("XSE", tr("At Sea"));
    countryCodeMap.insert("XIF", tr("In Flight"));
    countryCodeMap.insert("XEN", tr("England"));
    countryCodeMap.insert("XSC", tr("Scotland"));
    countryCodeMap.insert("XNI", tr("Northern Ireland"));
    countryCodeMap.insert("XWA", tr("Wales"));
    countryCodeMap.insert("PSE", tr("Palestine"));
    countryCodeMap.insert("GZA", tr("Gaza"));
    countryCodeMap.insert("JRO", tr("Jericho"));
}
/****************************************************************************/
