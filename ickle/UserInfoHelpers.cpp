/*
 * User Info Helpers
 *
 * Copyright (C) 2001-2003 Barnaby Gray <barnaby@beedesign.co.uk>.
 * Copyright (C) 2003 Nils Nordman <nino@nforced.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "UserInfoHelpers.h"

#include "ickle.h"
#include "sstream_fix.h"

#include <iostream>

using std::vector;
using std::string;
using std::map;
using std::ostringstream;
using std::istringstream;

map<ICQ2000::Status, const char *> UserInfoHelpers::status_map;
map<ICQ2000::Language, const char *> UserInfoHelpers::language_map;
map<ICQ2000::Country, const char *> UserInfoHelpers::country_map;
map<ICQ2000::Interest, const char *> UserInfoHelpers::interest_map;
map<ICQ2000::Background, const char *> UserInfoHelpers::background_map;
map<ICQ2000::AgeRange, const char *> UserInfoHelpers::age_map;
map<ICQ2000::Sex, const char *> UserInfoHelpers::sex_map;

void UserInfoHelpers::initialize() {

  static bool done = false;

  if (done)
    return;

  done = true;

  // statuses
  status_map[ICQ2000::STATUS_ONLINE]      = _("Online");
  status_map[ICQ2000::STATUS_AWAY]        = _("Away");
  status_map[ICQ2000::STATUS_NA]          = _("N/A");
  status_map[ICQ2000::STATUS_OCCUPIED]    = _("Occupied");
  status_map[ICQ2000::STATUS_DND]         = _("DND");
  status_map[ICQ2000::STATUS_FREEFORCHAT] = _("Free for chat");
  status_map[ICQ2000::STATUS_OFFLINE]     = _("Offline");

  // languages
  language_map[ICQ2000::LANGUAGE_UNKNOWN] = _("Unspecified");
  language_map[ICQ2000::LANGUAGE_ARABIC] = _("Arabic");
  language_map[ICQ2000::LANGUAGE_BHOJPURI] = _("Bhojpuri");
  language_map[ICQ2000::LANGUAGE_BULGARIAN] = _("Bulgarian");
  language_map[ICQ2000::LANGUAGE_BURMESE] = _("Burmese");
  language_map[ICQ2000::LANGUAGE_CANTONESE] = _("Cantonese");
  language_map[ICQ2000::LANGUAGE_CATALAN] = _("Catalan");
  language_map[ICQ2000::LANGUAGE_CHINESE] = _("Chinese");
  language_map[ICQ2000::LANGUAGE_CROATIAN] = _("Croatian");
  language_map[ICQ2000::LANGUAGE_CZECH] = _("Czech");
  language_map[ICQ2000::LANGUAGE_DANISH] = _("Danish");
  language_map[ICQ2000::LANGUAGE_DUTCH] = _("Dutch");
  language_map[ICQ2000::LANGUAGE_ENGLISH] = _("English");
  language_map[ICQ2000::LANGUAGE_ESPERANTO] = _("Esperanto");
  language_map[ICQ2000::LANGUAGE_ESTONIAN] = _("Estonian");
  language_map[ICQ2000::LANGUAGE_FARSI] = _("Farsi");
  language_map[ICQ2000::LANGUAGE_FINNISH] = _("Finnish");
  language_map[ICQ2000::LANGUAGE_FRENCH] = _("French");
  language_map[ICQ2000::LANGUAGE_GAELIC] = _("Gaelic");
  language_map[ICQ2000::LANGUAGE_GERMAN] = _("German");
  language_map[ICQ2000::LANGUAGE_GREEK] = _("Greek");
  language_map[ICQ2000::LANGUAGE_HEBREW] = _("Hebrew");
  language_map[ICQ2000::LANGUAGE_HINDI] = _("Hindi");
  language_map[ICQ2000::LANGUAGE_HUNGARIAN] = _("Hungarian");
  language_map[ICQ2000::LANGUAGE_ICELANDIC] = _("Icelandic");
  language_map[ICQ2000::LANGUAGE_INDONESIAN] = _("Indonesian");
  language_map[ICQ2000::LANGUAGE_ITALIAN] = _("Italian");
  language_map[ICQ2000::LANGUAGE_JAPANESE] = _("Japanese");
  language_map[ICQ2000::LANGUAGE_KHMER] = _("Khmer");
  language_map[ICQ2000::LANGUAGE_KOREAN] = _("Korean");
  language_map[ICQ2000::LANGUAGE_LAO] = _("Lao");
  language_map[ICQ2000::LANGUAGE_LATVIAN] = _("Latvian");
  language_map[ICQ2000::LANGUAGE_LITHUANIAN] = _("Lithuanian");
  language_map[ICQ2000::LANGUAGE_MALAY] = _("Malay");
  language_map[ICQ2000::LANGUAGE_NORWEGIAN] = _("Norwegian");
  language_map[ICQ2000::LANGUAGE_POLISH] = _("Polish");
  language_map[ICQ2000::LANGUAGE_PORTUGUESE] = _("Portuguese");
  language_map[ICQ2000::LANGUAGE_ROMANIAN] = _("Romanian");
  language_map[ICQ2000::LANGUAGE_RUSSIAN] = _("Russian");
  language_map[ICQ2000::LANGUAGE_SERBIAN] = _("Serbian");
  language_map[ICQ2000::LANGUAGE_SLOVAK] = _("Slovak");
  language_map[ICQ2000::LANGUAGE_SLOVENIAN] = _("Slovenian");
  language_map[ICQ2000::LANGUAGE_SOMALI] = _("Somali");
  language_map[ICQ2000::LANGUAGE_SPANISH] = _("Spanish");
  language_map[ICQ2000::LANGUAGE_SWAHILI] = _("Swahili");
  language_map[ICQ2000::LANGUAGE_SWEDISH] = _("Swedish");
  language_map[ICQ2000::LANGUAGE_TAGALOG] = _("Tagalog");
  language_map[ICQ2000::LANGUAGE_TATAR] = _("Tatar");
  language_map[ICQ2000::LANGUAGE_THAI] = _("Thai");
  language_map[ICQ2000::LANGUAGE_TURKISH] = _("Turkish");
  language_map[ICQ2000::LANGUAGE_UKRAINIAN] = _("Ukrainian");
  language_map[ICQ2000::LANGUAGE_URDU] = _("Urdu");
  language_map[ICQ2000::LANGUAGE_VIETNAMESE] = _("Vietnamese");
  language_map[ICQ2000::LANGUAGE_YIDDISH] = _("Yiddish");
  language_map[ICQ2000::LANGUAGE_YORUBA] = _("Yoruba");
  language_map[ICQ2000::LANGUAGE_TAIWANESE] = _("Taiwanese");
  language_map[ICQ2000::LANGUAGE_AFRIKAANS] = _("Afrikaans");
  language_map[ICQ2000::LANGUAGE_PERSIAN] = _("Persian");
  language_map[ICQ2000::LANGUAGE_ALBANIAN] = _("Albanian");
  language_map[ICQ2000::LANGUAGE_ARMENIAN] = _("Armenian");  

  // countries
  country_map[ICQ2000::COUNTRY_UNKNOWN] = _("Unspecified");
  country_map[ICQ2000::COUNTRY_AFGHANISTAN] = _("Afghanistan");
  country_map[ICQ2000::COUNTRY_ALBANIA] = _("Albania");
  country_map[ICQ2000::COUNTRY_ALGERIA] = _("Algeria");
  country_map[ICQ2000::COUNTRY_AMERICAN_SAMOA] = _("American Samoa");
  country_map[ICQ2000::COUNTRY_ANDORRA] = _("Andorra");
  country_map[ICQ2000::COUNTRY_ANGOLA] = _("Angola");
  country_map[ICQ2000::COUNTRY_ANGUILLA] = _("Anguilla");
  country_map[ICQ2000::COUNTRY_ANTIGUA] = _("Antigua");
  country_map[ICQ2000::COUNTRY_ARGENTINA] = _("Argentina");
  country_map[ICQ2000::COUNTRY_ARMENIA] = _("Armenia");
  country_map[ICQ2000::COUNTRY_ARUBA] = _("Aruba");
  country_map[ICQ2000::COUNTRY_ASCENSION_ISLAND] = _("Ascension Island");
  country_map[ICQ2000::COUNTRY_AUSTRALIA] = _("Australia");
  country_map[ICQ2000::COUNTRY_AUSTRALIAN_ANTARCTIC_TERRITORY] = _("Australian Antarctic Territory");
  country_map[ICQ2000::COUNTRY_AUSTRIA] = _("Austria");
  country_map[ICQ2000::COUNTRY_AZERBAIJAN] = _("Azerbaijan");
  country_map[ICQ2000::COUNTRY_BAHAMAS] = _("Bahamas");
  country_map[ICQ2000::COUNTRY_BAHRAIN] = _("Bahrain");
  country_map[ICQ2000::COUNTRY_BANGLADESH] = _("Bangladesh");
  country_map[ICQ2000::COUNTRY_BARBADOS] = _("Barbados");
  country_map[ICQ2000::COUNTRY_BARBUDA] = _("Barbuda");
  country_map[ICQ2000::COUNTRY_BELARUS] = _("Belarus");
  country_map[ICQ2000::COUNTRY_BELGIUM] = _("Belgium");
  country_map[ICQ2000::COUNTRY_BELIZE] = _("Belize");
  country_map[ICQ2000::COUNTRY_BENIN] = _("Benin");
  country_map[ICQ2000::COUNTRY_BERMUDA] = _("Bermuda");
  country_map[ICQ2000::COUNTRY_BHUTAN] = _("Bhutan");
  country_map[ICQ2000::COUNTRY_BOLIVIA] = _("Bolivia");
  country_map[ICQ2000::COUNTRY_BOSNIA_AND_HERZEGOVINA] = _("Bosnia and Herzegovina");
  country_map[ICQ2000::COUNTRY_BOTSWANA] = _("Botswana");
  country_map[ICQ2000::COUNTRY_BRAZIL] = _("Brazil");
  country_map[ICQ2000::COUNTRY_BRITISH_VIRGIN_ISLANDS] = _("British Virgin Islands");
  country_map[ICQ2000::COUNTRY_BRUNEI] = _("Brunei");
  country_map[ICQ2000::COUNTRY_BULGARIA] = _("Bulgaria");
  country_map[ICQ2000::COUNTRY_BURKINA_FASO] = _("Burkina Faso");
  country_map[ICQ2000::COUNTRY_BURUNDI] = _("Burundi");
  country_map[ICQ2000::COUNTRY_CAMBODIA] = _("Cambodia");
  country_map[ICQ2000::COUNTRY_CAMEROON] = _("Cameroon");
  country_map[ICQ2000::COUNTRY_CANADA] = _("Canada");
  country_map[ICQ2000::COUNTRY_CAPE_VERDE_ISLANDS] = _("Cape Verde Islands");
  country_map[ICQ2000::COUNTRY_CAYMAN_ISLANDS] = _("Cayman Islands");
  country_map[ICQ2000::COUNTRY_CENTRAL_AFRICAN_REPUBLIC] = _("Central African Republic");
  country_map[ICQ2000::COUNTRY_CHAD] = _("Chad");
  country_map[ICQ2000::COUNTRY_CHILE] = _("Chile");
  country_map[ICQ2000::COUNTRY_CHINA] = _("China");
  country_map[ICQ2000::COUNTRY_CHRISTMAS_ISLAND] = _("Christmas Island");
  country_map[ICQ2000::COUNTRY_COCOS_KEELING_ISLANDS] = _("Cocos-Keeling Islands");
  country_map[ICQ2000::COUNTRY_COLOMBIA] = _("Colombia");
  country_map[ICQ2000::COUNTRY_COMOROS] = _("Comoros");
  country_map[ICQ2000::COUNTRY_CONGO] = _("Congo");
  country_map[ICQ2000::COUNTRY_COOK_ISLANDS] = _("Cook Islands");
  country_map[ICQ2000::COUNTRY_COSTA_RICA] = _("Costa Rica");
  country_map[ICQ2000::COUNTRY_CROATIA] = _("Croatia");
  country_map[ICQ2000::COUNTRY_CUBA] = _("Cuba");
  country_map[ICQ2000::COUNTRY_CYPRUS] = _("Cyprus");
  country_map[ICQ2000::COUNTRY_CZECH_REPUBLIC] = _("Czech Republic");
  country_map[ICQ2000::COUNTRY_DENMARK] = _("Denmark");
  country_map[ICQ2000::COUNTRY_DIEGO_GARCIA] = _("Diego Garcia");
  country_map[ICQ2000::COUNTRY_DJIBOUTI] = _("Djibouti");
  country_map[ICQ2000::COUNTRY_DOMINICA] = _("Dominica");
  country_map[ICQ2000::COUNTRY_DOMINICAN_REPUBLIC] = _("Dominican Republic");
  country_map[ICQ2000::COUNTRY_ECUADOR] = _("Ecuador");
  country_map[ICQ2000::COUNTRY_EGYPT] = _("Egypt");
  country_map[ICQ2000::COUNTRY_EL_SALVADOR] = _("El Salvador");
  country_map[ICQ2000::COUNTRY_EQUATORIAL_GUINEA] = _("Equatorial Guinea");
  country_map[ICQ2000::COUNTRY_ERITREA] = _("Eritrea");
  country_map[ICQ2000::COUNTRY_ESTONIA] = _("Estonia");
  country_map[ICQ2000::COUNTRY_ETHIOPIA] = _("Ethiopia");
  country_map[ICQ2000::COUNTRY_FAEROE_ISLANDS] = _("Faeroe Islands");
  country_map[ICQ2000::COUNTRY_FALKLAND_ISLANDS] = _("Falkland Islands");
  country_map[ICQ2000::COUNTRY_FIJI_ISLANDS] = _("Fiji Islands");
  country_map[ICQ2000::COUNTRY_FINLAND] = _("Finland");
  country_map[ICQ2000::COUNTRY_FRANCE] = _("France");
  country_map[ICQ2000::COUNTRY_FRENCH_ANTILLES] = _("French Antilles");
  country_map[ICQ2000::COUNTRY_FRENCH_GUIANA] = _("French Guiana");
  country_map[ICQ2000::COUNTRY_FRENCH_POLYNESIA] = _("French Polynesia");
  country_map[ICQ2000::COUNTRY_GABON] = _("Gabon");
  country_map[ICQ2000::COUNTRY_GAMBIA] = _("Gambia");
  country_map[ICQ2000::COUNTRY_GEORGIA] = _("Georgia");
  country_map[ICQ2000::COUNTRY_GERMANY] = _("Germany");
  country_map[ICQ2000::COUNTRY_GHANA] = _("Ghana");
  country_map[ICQ2000::COUNTRY_GIBRALTAR] = _("Gibraltar");
  country_map[ICQ2000::COUNTRY_GREECE] = _("Greece");
  country_map[ICQ2000::COUNTRY_GREENLAND] = _("Greenland");
  country_map[ICQ2000::COUNTRY_GRENADA] = _("Grenada");
  country_map[ICQ2000::COUNTRY_GUADELOUPE] = _("Guadeloupe");
  country_map[ICQ2000::COUNTRY_GUAM] = _("Guam");
  country_map[ICQ2000::COUNTRY_GUANTANAMO_BAY] = _("Guantanamo Bay");
  country_map[ICQ2000::COUNTRY_GUATEMALA] = _("Guatemala");
  country_map[ICQ2000::COUNTRY_GUINEA] = _("Guinea");
  country_map[ICQ2000::COUNTRY_GUINEA_BISSAU] = _("Guinea-Bissau");
  country_map[ICQ2000::COUNTRY_GUYANA] = _("Guyana");
  country_map[ICQ2000::COUNTRY_HAITI] = _("Haiti");
  country_map[ICQ2000::COUNTRY_HONDURAS] = _("Honduras");
  country_map[ICQ2000::COUNTRY_HONG_KONG] = _("Hong Kong");
  country_map[ICQ2000::COUNTRY_HUNGARY] = _("Hungary");
  country_map[ICQ2000::COUNTRY_INMARSAT_ATLANTIC_EAST] = _("INMARSAT (Atlantic-East)");
  country_map[ICQ2000::COUNTRY_INMARSAT_ATLANTIC_WEST] = _("INMARSAT (Atlantic-West)");
  country_map[ICQ2000::COUNTRY_INMARSAT_INDIAN] = _("INMARSAT (Indian)");
  country_map[ICQ2000::COUNTRY_INMARSAT_PACIFIC] = _("INMARSAT (Pacific)");
  country_map[ICQ2000::COUNTRY_INMARSAT] = _("INMARSAT");
  country_map[ICQ2000::COUNTRY_ICELAND] = _("Iceland");
  country_map[ICQ2000::COUNTRY_INDIA] = _("India");
  country_map[ICQ2000::COUNTRY_INDONESIA] = _("Indonesia");
  country_map[ICQ2000::COUNTRY_INTERNATIONAL_FREEPHONE_SERVICE] = _("International Freephone Service");
  country_map[ICQ2000::COUNTRY_IRAN] = _("Iran");
  country_map[ICQ2000::COUNTRY_IRAQ] = _("Iraq");
  country_map[ICQ2000::COUNTRY_IRELAND] = _("Ireland");
  country_map[ICQ2000::COUNTRY_ISRAEL] = _("Israel");
  country_map[ICQ2000::COUNTRY_ITALY] = _("Italy");
  country_map[ICQ2000::COUNTRY_IVORY_COAST] = _("Ivory Coast");
  country_map[ICQ2000::COUNTRY_JAMAICA] = _("Jamaica");
  country_map[ICQ2000::COUNTRY_JAPAN] = _("Japan");
  country_map[ICQ2000::COUNTRY_JORDAN] = _("Jordan");
  country_map[ICQ2000::COUNTRY_KAZAKHSTAN] = _("Kazakhstan");
  country_map[ICQ2000::COUNTRY_KENYA] = _("Kenya");
  country_map[ICQ2000::COUNTRY_KIRIBATI_REPUBLIC] = _("Kiribati Republic");
  country_map[ICQ2000::COUNTRY_KOREA_NORTH] = _("Korea (North)");
  country_map[ICQ2000::COUNTRY_KOREA_REPUBLIC_OF] = _("Korea (Republic of)");
  country_map[ICQ2000::COUNTRY_KUWAIT] = _("Kuwait");
  country_map[ICQ2000::COUNTRY_KYRGYZ_REPUBLIC] = _("Kyrgyz Republic");
  country_map[ICQ2000::COUNTRY_LAOS] = _("Laos");
  country_map[ICQ2000::COUNTRY_LATVIA] = _("Latvia");
  country_map[ICQ2000::COUNTRY_LEBANON] = _("Lebanon");
  country_map[ICQ2000::COUNTRY_LESOTHO] = _("Lesotho");
  country_map[ICQ2000::COUNTRY_LIBERIA] = _("Liberia");
  country_map[ICQ2000::COUNTRY_LIBYA] = _("Libya");
  country_map[ICQ2000::COUNTRY_LIECHTENSTEIN] = _("Liechtenstein");
  country_map[ICQ2000::COUNTRY_LITHUANIA] = _("Lithuania");
  country_map[ICQ2000::COUNTRY_LUXEMBOURG] = _("Luxembourg");
  country_map[ICQ2000::COUNTRY_MACAU] = _("Macau");
  country_map[ICQ2000::COUNTRY_MADAGASCAR] = _("Madagascar");
  country_map[ICQ2000::COUNTRY_MALAWI] = _("Malawi");
  country_map[ICQ2000::COUNTRY_MALAYSIA] = _("Malaysia");
  country_map[ICQ2000::COUNTRY_MALDIVES] = _("Maldives");
  country_map[ICQ2000::COUNTRY_MALI] = _("Mali");
  country_map[ICQ2000::COUNTRY_MALTA] = _("Malta");
  country_map[ICQ2000::COUNTRY_MARSHALL_ISLANDS] = _("Marshall Islands");
  country_map[ICQ2000::COUNTRY_MARTINIQUE] = _("Martinique");
  country_map[ICQ2000::COUNTRY_MAURITANIA] = _("Mauritania");
  country_map[ICQ2000::COUNTRY_MAURITIUS] = _("Mauritius");
  country_map[ICQ2000::COUNTRY_MAYOTTE_ISLAND] = _("Mayotte Island");
  country_map[ICQ2000::COUNTRY_MEXICO] = _("Mexico");
  country_map[ICQ2000::COUNTRY_MICRONESIA_FEDERATED_STATES_OF] = _("Micronesia");
  country_map[ICQ2000::COUNTRY_MOLDOVA] = _("Moldova");
  country_map[ICQ2000::COUNTRY_MONACO] = _("Monaco");
  country_map[ICQ2000::COUNTRY_MONGOLIA] = _("Mongolia");
  country_map[ICQ2000::COUNTRY_MONTSERRAT] = _("Montserrat");
  country_map[ICQ2000::COUNTRY_MOROCCO] = _("Morocco");
  country_map[ICQ2000::COUNTRY_MOZAMBIQUE] = _("Mozambique");
  country_map[ICQ2000::COUNTRY_MYANMAR] = _("Myanmar");
  country_map[ICQ2000::COUNTRY_NAMIBIA] = _("Namibia");
  country_map[ICQ2000::COUNTRY_NAURU] = _("Nauru");
  country_map[ICQ2000::COUNTRY_NEPAL] = _("Nepal");
  country_map[ICQ2000::COUNTRY_NETHERLANDS_ANTILLES] = _("Netherlands Antilles");
  country_map[ICQ2000::COUNTRY_NETHERLANDS] = _("Netherlands");
  country_map[ICQ2000::COUNTRY_NEVIS] = _("Nevis");
  country_map[ICQ2000::COUNTRY_NEW_CALEDONIA] = _("New Caledonia");
  country_map[ICQ2000::COUNTRY_NEW_ZEALAND] = _("New Zealand");
  country_map[ICQ2000::COUNTRY_NICARAGUA] = _("Nicaragua");
  country_map[ICQ2000::COUNTRY_NIGER] = _("Niger");
  country_map[ICQ2000::COUNTRY_NIGERIA] = _("Nigeria");
  country_map[ICQ2000::COUNTRY_NIUE] = _("Niue");
  country_map[ICQ2000::COUNTRY_NORFOLK_ISLAND] = _("Norfolk Island");
  country_map[ICQ2000::COUNTRY_NORWAY] = _("Norway");
  country_map[ICQ2000::COUNTRY_OMAN] = _("Oman");
  country_map[ICQ2000::COUNTRY_PAKISTAN] = _("Pakistan");
  country_map[ICQ2000::COUNTRY_PALAU] = _("Palau");
  country_map[ICQ2000::COUNTRY_PANAMA] = _("Panama");
  country_map[ICQ2000::COUNTRY_PAPUA_NEW_GUINEA] = _("Papua New Guinea");
  country_map[ICQ2000::COUNTRY_PARAGUAY] = _("Paraguay");
  country_map[ICQ2000::COUNTRY_PERU] = _("Peru");
  country_map[ICQ2000::COUNTRY_PHILIPPINES] = _("Philippines");
  country_map[ICQ2000::COUNTRY_POLAND] = _("Poland");
  country_map[ICQ2000::COUNTRY_PORTUGAL] = _("Portugal");
  country_map[ICQ2000::COUNTRY_PUERTO_RICO] = _("Puerto Rico");
  country_map[ICQ2000::COUNTRY_QATAR] = _("Qatar");
  country_map[ICQ2000::COUNTRY_REPUBLIC_OF_MACEDONIA] = _("Republic of Macedonia");
  country_map[ICQ2000::COUNTRY_REUNION_ISLAND] = _("Reunion Island");
  country_map[ICQ2000::COUNTRY_ROMANIA] = _("Romania");
  country_map[ICQ2000::COUNTRY_ROTA_ISLAND] = _("Rota Island");
  country_map[ICQ2000::COUNTRY_RUSSIA] = _("Russia");
  country_map[ICQ2000::COUNTRY_RWANDA] = _("Rwanda");
  country_map[ICQ2000::COUNTRY_SAINT_LUCIA] = _("Saint Lucia");
  country_map[ICQ2000::COUNTRY_SAIPAN_ISLAND] = _("Saipan Island");
  country_map[ICQ2000::COUNTRY_SAN_MARINO] = _("San Marino");
  country_map[ICQ2000::COUNTRY_SAO_TOME_AND_PRINCIPE] = _("Sao Tome and Principe");
  country_map[ICQ2000::COUNTRY_SAUDI_ARABIA] = _("Saudi Arabia");
  country_map[ICQ2000::COUNTRY_SENEGAL_REPUBLIC] = _("Senegal Republic");
  country_map[ICQ2000::COUNTRY_SEYCHELLE_ISLANDS] = _("Seychelle Islands");
  country_map[ICQ2000::COUNTRY_SIERRA_LEONE] = _("Sierra Leone");
  country_map[ICQ2000::COUNTRY_SINGAPORE] = _("Singapore");
  country_map[ICQ2000::COUNTRY_SLOVAK_REPUBLIC] = _("Slovak Republic");
  country_map[ICQ2000::COUNTRY_SLOVENIA] = _("Slovenia");
  country_map[ICQ2000::COUNTRY_SOLOMON_ISLANDS] = _("Solomon Islands");
  country_map[ICQ2000::COUNTRY_SOMALIA] = _("Somalia");
  country_map[ICQ2000::COUNTRY_SOUTH_AFRICA] = _("South Africa");
  country_map[ICQ2000::COUNTRY_SPAIN] = _("Spain");
  country_map[ICQ2000::COUNTRY_SRI_LANKA] = _("Sri Lanka");
  country_map[ICQ2000::COUNTRY_ST_HELENA] = _("St. Helena");
  country_map[ICQ2000::COUNTRY_ST_KITTS] = _("St. Kitts");
  country_map[ICQ2000::COUNTRY_ST_PIERRE_AND_MIQUELON] = _("St. Pierre and Miquelon");
  country_map[ICQ2000::COUNTRY_ST_VINCENT_AND_THE_GRENADINES] = _("St. Vincent and the Grenadines");
  country_map[ICQ2000::COUNTRY_SUDAN] = _("Sudan");
  country_map[ICQ2000::COUNTRY_SURINAME] = _("Suriname");
  country_map[ICQ2000::COUNTRY_SWAZILAND] = _("Swaziland");
  country_map[ICQ2000::COUNTRY_SWEDEN] = _("Sweden");
  country_map[ICQ2000::COUNTRY_SWITZERLAND] = _("Switzerland");
  country_map[ICQ2000::COUNTRY_SYRIA] = _("Syria");
  country_map[ICQ2000::COUNTRY_TAIWAN_REPUBLIC_OF_CHINA] = _("Taiwan");
  country_map[ICQ2000::COUNTRY_TAJIKISTAN] = _("Tajikistan");
  country_map[ICQ2000::COUNTRY_TANZANIA] = _("Tanzania");
  country_map[ICQ2000::COUNTRY_THAILAND] = _("Thailand");
  country_map[ICQ2000::COUNTRY_TINIAN_ISLAND] = _("Tinian Island");
  country_map[ICQ2000::COUNTRY_TOGO] = _("Togo");
  country_map[ICQ2000::COUNTRY_TOKELAU] = _("Tokelau");
  country_map[ICQ2000::COUNTRY_TONGA] = _("Tonga");
  country_map[ICQ2000::COUNTRY_TRINIDAD_AND_TOBAGO] = _("Trinidad and Tobago");
  country_map[ICQ2000::COUNTRY_TUNISIA] = _("Tunisia");
  country_map[ICQ2000::COUNTRY_TURKEY] = _("Turkey");
  country_map[ICQ2000::COUNTRY_TURKMENISTAN] = _("Turkmenistan");
  country_map[ICQ2000::COUNTRY_TURKS_AND_CAICOS_ISLANDS] = _("Turks and Caicos Islands");
  country_map[ICQ2000::COUNTRY_TUVALU] = _("Tuvalu");
  country_map[ICQ2000::COUNTRY_USA] = _("USA");
  country_map[ICQ2000::COUNTRY_UGANDA] = _("Uganda");
  country_map[ICQ2000::COUNTRY_UKRAINE] = _("Ukraine");
  country_map[ICQ2000::COUNTRY_UNITED_ARAB_EMIRATES] = _("United Arab Emirates");
  country_map[ICQ2000::COUNTRY_UNITED_KINGDOM] = _("United Kingdom");
  country_map[ICQ2000::COUNTRY_UNITED_STATES_VIRGIN_ISLANDS] = _("United States Virgin Islands");
  country_map[ICQ2000::COUNTRY_URUGUAY] = _("Uruguay");
  country_map[ICQ2000::COUNTRY_UZBEKISTAN] = _("Uzbekistan");
  country_map[ICQ2000::COUNTRY_VANUATU] = _("Vanuatu");
  country_map[ICQ2000::COUNTRY_VATICAN_CITY] = _("Vatican City");
  country_map[ICQ2000::COUNTRY_VENEZUELA] = _("Venezuela");
  country_map[ICQ2000::COUNTRY_VIETNAM] = _("Vietnam");
  country_map[ICQ2000::COUNTRY_WALLIS_AND_FUTUNA_ISLANDS] = _("Wallis and Futuna Islands");
  country_map[ICQ2000::COUNTRY_WESTERN_SAMOA] = _("Western Samoa");
  country_map[ICQ2000::COUNTRY_YEMEN] = _("Yemen");
  country_map[ICQ2000::COUNTRY_YUGOSLAVIA] = _("Yugoslavia");
  country_map[ICQ2000::COUNTRY_ZAIRE] = _("Zaire");
  country_map[ICQ2000::COUNTRY_ZAMBIA] = _("Zambia");
  country_map[ICQ2000::COUNTRY_ZIMBABWE] = _("Zimbabwe");

  // interests
  interest_map[ICQ2000::INTEREST_ART] = _("Art");
  interest_map[ICQ2000::INTEREST_CARS] = _("Cars");
  interest_map[ICQ2000::INTEREST_CELEBRITY_FANS] = _("Celebrity Fans");
  interest_map[ICQ2000::INTEREST_COLLECTIONS] = _("Collections");
  interest_map[ICQ2000::INTEREST_COMPUTERS] = _("Computers");
  interest_map[ICQ2000::INTEREST_CULTURE] = _("Culture");
  interest_map[ICQ2000::INTEREST_FITNESS] = _("Fitness");
  interest_map[ICQ2000::INTEREST_GAMES] = _("Games");
  interest_map[ICQ2000::INTEREST_HOBBIES] = _("Hobbies");
  interest_map[ICQ2000::INTEREST_ICQ___HELP] = _("ICQ - Help");
  interest_map[ICQ2000::INTEREST_INTERNET] = _("Internet");
  interest_map[ICQ2000::INTEREST_LIFESTYLE] = _("Lifestyle");
  interest_map[ICQ2000::INTEREST_MOVIES_AND_TV] = _("Movies and TV");
  interest_map[ICQ2000::INTEREST_MUSIC] = _("Music");
  interest_map[ICQ2000::INTEREST_OUTDOORS] = _("Outdoors");
  interest_map[ICQ2000::INTEREST_PARENTING] = _("Parenting");
  interest_map[ICQ2000::INTEREST_PETS_AND_ANIMALS] = _("Pets and Animals");
  interest_map[ICQ2000::INTEREST_RELIGION] = _("Religion");
  interest_map[ICQ2000::INTEREST_SCIENCE] = _("Science");
  interest_map[ICQ2000::INTEREST_SKILLS] = _("Skills");
  interest_map[ICQ2000::INTEREST_SPORTS] = _("Sports");
  interest_map[ICQ2000::INTEREST_WEB_DESIGN] = _("Web Design");
  interest_map[ICQ2000::INTEREST_ECOLOGY] = _("Ecology");
  interest_map[ICQ2000::INTEREST_NEWS_AND_MEDIA] = _("News and Media");
  interest_map[ICQ2000::INTEREST_GOVERNMENT] = _("Government");
  interest_map[ICQ2000::INTEREST_BUSINESS] = _("Business");
  interest_map[ICQ2000::INTEREST_MYSTICS] = _("Mystics");
  interest_map[ICQ2000::INTEREST_TRAVEL] = _("Travel");
  interest_map[ICQ2000::INTEREST_ASTRONOMY] = _("Astronomy");
  interest_map[ICQ2000::INTEREST_SPACE] = _("Space");
  interest_map[ICQ2000::INTEREST_CLOTHING] = _("Clothing");
  interest_map[ICQ2000::INTEREST_PARTIES] = _("Parties");
  interest_map[ICQ2000::INTEREST_WOMEN] = _("Women");
  interest_map[ICQ2000::INTEREST_SOCIAL_SCIENCE] = _("Social science");
  interest_map[ICQ2000::INTEREST_60S] = _("60's");
  interest_map[ICQ2000::INTEREST_70S] = _("70's");
  interest_map[ICQ2000::INTEREST_80S] = _("80's");
  interest_map[ICQ2000::INTEREST_50S] = _("50's");
  interest_map[ICQ2000::INTEREST_FINANCE_AND_CORPORATE] = _("Finance and Corporate");
  interest_map[ICQ2000::INTEREST_ENTERTAINMENT] = _("Entertainment");
  interest_map[ICQ2000::INTEREST_CONSUMER_ELECTRONICS] = _("Consumer Electronics");
  interest_map[ICQ2000::INTEREST_RETAIL_STORES] = _("Retail Stores");
  interest_map[ICQ2000::INTEREST_HEALTH_AND_BEAUTY] = _("Health and Beauty");
  interest_map[ICQ2000::INTEREST_MEDIA] = _("Media");
  interest_map[ICQ2000::INTEREST_HOUSEHOLD_PRODUCTS] = _("Household Products");
  interest_map[ICQ2000::INTEREST_MAIL_ORDER_CATALOG] = _("Mail Order Catalog");
  interest_map[ICQ2000::INTEREST_BUSINESS_SERVICES] = _("Business Services");
  interest_map[ICQ2000::INTEREST_AUDIO_AND_VISUAL] = _("Audio and Visual");
  interest_map[ICQ2000::INTEREST_SPORTING_AND_ATHLETICS] = _("Sporting and Athletics");
  interest_map[ICQ2000::INTEREST_PUBLISHING] = _("Publishing");
  interest_map[ICQ2000::INTEREST_HOME_AUTOMATION] = _("Home Automation");

  // backgrounds
  background_map[ICQ2000::BACKGROUND_UNIVERSITY] = _("University");
  background_map[ICQ2000::BACKGROUND_HIGH_SCHOOL] = _("High school");
  background_map[ICQ2000::BACKGROUND_COLLEGE] = _("College");
  background_map[ICQ2000::BACKGROUND_ELEMENTARY_SCHOOL] = _("Elementary School");
  background_map[ICQ2000::BACKGROUND_MILITARY] = _("Military");
  background_map[ICQ2000::BACKGROUND_OTHER] = _("Other");
  background_map[ICQ2000::BACKGROUND_PAST_ORGANIZATION] = _("Past Organization");
  background_map[ICQ2000::BACKGROUND_PAST_WORK_PLACE] = _("Past Work Place");

  // ages
  age_map[ICQ2000::RANGE_NORANGE] = _("none");
  age_map[ICQ2000::RANGE_18_22] = _("18-22");
  age_map[ICQ2000::RANGE_23_29] = _("23-29");
  age_map[ICQ2000::RANGE_30_39] = _("30-39");
  age_map[ICQ2000::RANGE_40_49] = _("40-49");
  age_map[ICQ2000::RANGE_50_59] = _("50-59");
  age_map[ICQ2000::RANGE_60_ABOVE] = _("60-above");

  // sex
  sex_map[ICQ2000::SEX_MALE] = _("Male");
  sex_map[ICQ2000::SEX_FEMALE] = _("Female");
  sex_map[ICQ2000::SEX_UNKNOWN] = _("Unspecified");
}

template <typename T>
const char * valueFromKey(map<T, const char*> &m, T key, T def) {
  typename map<T, const char*>::const_iterator itr = m.find(key);
  if (itr != m.end())
    return itr->second;
  else
    return m[def];
}


template <typename T>
T keyFromValue(map<T, const char*> &m, const string &s, T def) {
  for (typename map<T, const char*>::iterator itr = m.begin(); itr != m.end(); ++itr) {
    if (s == itr->second) {
      return itr->first;
    }
  }
  return def;
}

template <typename T>
vector<string> vectorOfValues(map<T, const char*> &m) {
  vector<string> vec;
  for (typename map<T, const char*>::iterator itr = m.begin(); itr != m.end(); ++itr) {
    vec.push_back(itr->second);
  }
  //The first choice is mostly something special..
  if ( (vec.begin() != vec.end() ) && (++(vec.begin()) !=vec.end() ) )
    std::sort(++(vec.begin()),vec.end() );
  return vec;
}

string UserInfoHelpers::getStringFromStatus(ICQ2000::Status st)
{
  return valueFromKey(status_map, st, ICQ2000::STATUS_OFFLINE);
}

string UserInfoHelpers::getStringFromSex(ICQ2000::Sex sex)
{
  return valueFromKey(sex_map, sex, ICQ2000::SEX_UNKNOWN);
}
      
ICQ2000::Sex UserInfoHelpers::getSexFromString(const string& s)
{
  return keyFromValue(sex_map, s, ICQ2000::SEX_UNKNOWN);
}

vector<string> UserInfoHelpers::getSexAllStrings()
{
  return vectorOfValues(sex_map);
}
    
/*
  TODO: Redo the timezone stuff as well to be backed up by maps.
  As it currently stands the conversions back and forth from enums
  to ints is an atrocity.
*/

string UserInfoHelpers::getStringFromTimezone(ICQ2000::Timezone tz)
{
  int id = (int)tz;

  if (id > 24 || id < -24) {
    return "Unspecified";
  } else {
    ostringstream ostr;
    ostr << "GMT " << (id > 0 ? "-" : "+")
	 << abs(id/2)
	 << ":"
	 << (id % 2 == 0 ? "00" : "30");
    return ostr.str();
  }
}
      
ICQ2000::Timezone UserInfoHelpers::getTimezoneFromString(const string& s)
{
  string t;
  char c1, c2;
  int h, m;
      
  istringstream istr(s);
  if ((istr >> t >> c1 >> h >> c2 >> m)
      && t == "GMT" && (c1 == '+' || c1 == '-')
      && h <= 24 && (m == 30 || m == 0)) {
    if (c1 == '+') {
      return ICQ2000::Timezone(-(h * 2 + (m == 30 ? 1 : 0)));
    } else {
      return ICQ2000::Timezone(h * 2 + (m == 30 ? 1 : 0));
    }
  } else {
    return ICQ2000::TIMEZONE_UNKNOWN;
  }
}
    
vector<string> UserInfoHelpers::getTimezoneAllStrings()
{
  vector<string> ret;

  ret.push_back( getStringFromTimezone( ICQ2000::TIMEZONE_UNKNOWN ) );
  for (signed char n =  -24; n <= 24; ++n) {
    ret.push_back( getStringFromTimezone( ICQ2000::Timezone(n) ) );
  }
  return ret;
}
    
string UserInfoHelpers::getTimezoneToLocaltime(ICQ2000::Timezone tz)
{
  string r;
  int id = (int)tz;

  if(id <= 24 && id >= -24) {
    time_t rt = time(0) + getSystemTimezone()*1800;
    rt -= id*1800;
    r = ctime(&rt);
  }

  return r;
}

ICQ2000::Timezone UserInfoHelpers::getSystemTimezone()
{
  time_t t = time(NULL);
  struct tm *tzone = localtime(&t);
  int nTimezone = 0;

#ifdef HAVE_TM_ZONE
  nTimezone = -(tzone->tm_gmtoff);
#else
  nTimezone = timezone;
#endif

  nTimezone += (tzone->tm_isdst == 1 ? 3600 : 0);
  nTimezone /= 1800;

  if(nTimezone > 23) return ICQ2000::Timezone(23-nTimezone);

  return ICQ2000::Timezone(nTimezone);
}
    
string UserInfoHelpers::getStringFromLanguage(ICQ2000::Language lang)
{
  return valueFromKey(language_map, lang, ICQ2000::LANGUAGE_UNKNOWN);
}
    
ICQ2000::Language UserInfoHelpers::getLanguageFromString(const string& s)
{
  return keyFromValue(language_map, s, ICQ2000::LANGUAGE_UNKNOWN);
}

vector<string> UserInfoHelpers::getLanguageAllStrings()
{
  return vectorOfValues(language_map);
}
    
string UserInfoHelpers::getStringFromCountry(ICQ2000::Country country)
{
  return valueFromKey(country_map, country, ICQ2000::COUNTRY_UNKNOWN);
}
    
ICQ2000::Country UserInfoHelpers::getCountryFromString(const string& s)
{
  return keyFromValue(country_map, s, ICQ2000::COUNTRY_UNKNOWN);
}

vector<string> UserInfoHelpers::getCountryAllStrings()
{
  return vectorOfValues(country_map);
}
    
string UserInfoHelpers::getStringFromInterest(ICQ2000::Interest interest)
{
  return valueFromKey(interest_map, interest, ICQ2000::INTEREST_INTERNET); // everbody loves the net
}
    
ICQ2000::Interest UserInfoHelpers::getInterestFromString(const string& s)
{
  return keyFromValue(interest_map, s, ICQ2000::INTEREST_INTERNET); // oh yes they do
}

vector<string> UserInfoHelpers::getInterestsAllStrings()
{
  return vectorOfValues(interest_map);
}
    
string UserInfoHelpers::getStringFromBackground(ICQ2000::Background background)
{
  return valueFromKey(background_map, background, ICQ2000::BACKGROUND_OTHER);
}
    
ICQ2000::Background UserInfoHelpers::getBackgroundFromString(const string& s)
{
  return keyFromValue(background_map, s, ICQ2000::BACKGROUND_OTHER);
}

vector<string> UserInfoHelpers::getBackgroundAllStrings()
{
  return vectorOfValues(background_map);
}
