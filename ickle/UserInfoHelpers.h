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

#ifndef USERINFOHELPERS_H
#define USERINFOHELPERS_H

#include <string>
#include <vector>
#include <map>

#include <time.h>

#include <libicq2000/userinfoconstants.h>

class UserInfoHelpers
{
private:

    static std::map<ICQ2000::Language, const char *> language_map;
    static std::map<ICQ2000::Country, const char *> country_map;
    static std::map<ICQ2000::Interest, const char *> interest_map;
    static std::map<ICQ2000::Background, const char *> background_map;
    static std::map<ICQ2000::AgeRange, const char *> age_map;
    static std::map<ICQ2000::Sex, const char *> sex_map;

public:

    static void initialize();

    static std::string getStringFromSex(ICQ2000::Sex id);
    static ICQ2000::Sex getSexFromString(const std::string& s);
    static std::vector<std::string> getSexAllStrings();
    
    static std::string getStringFromTimezone(ICQ2000::Timezone tz);
    static ICQ2000::Timezone getTimezoneFromString(const std::string& s);
    static std::vector<std::string> getTimezoneAllStrings();
    static std::string getTimezoneToLocaltime(ICQ2000::Timezone tz);
    static ICQ2000::Timezone getSystemTimezone();

    static std::string getStringFromLanguage(ICQ2000::Language lang);
    static ICQ2000::Language getLanguageFromString(const std::string& s);
    static std::vector<std::string> getLanguageAllStrings();

    static std::string getStringFromCountry(ICQ2000::Country country);
    static ICQ2000::Country getCountryFromString(const std::string& s);
    static std::vector<std::string> getCountryAllStrings();

    static std::string getStringFromInterest(ICQ2000::Interest interest);
    static ICQ2000::Interest getInterestFromString(const std::string& s);
    static std::vector<std::string> getInterestsAllStrings();
    
    static std::string getStringFromBackground(ICQ2000::Background background);
    static ICQ2000::Background getBackgroundFromString(const std::string& s);
    static std::vector<std::string> getBackgroundAllStrings();
};
  


#endif
