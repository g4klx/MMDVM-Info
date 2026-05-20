/*
 *   Copyright (C) 2026 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "ReadConfig.h"

#include "Log.h"

#include <algorithm>

const int BUFFER_SIZE = 500;

CReadConfig::CReadConfig()
{
}

CReadConfig::~CReadConfig()
{
}

bool CReadConfig::read(const std::string& name, const std::string& path, const std::vector<std::string>& exclusions) const
{
	nlohmann::json json;
	nlohmann::json sectionJSON;

	std::string section;

	FILE* fp = ::fopen(path.c_str(), "rt");
	if (fp != nullptr) {
		char buffer[BUFFER_SIZE];
		while (::fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
			if (buffer[0U] == '#')
				continue;

			if (buffer[0U] == '[') {
				if (!sectionJSON.empty()) {
					json[section] = sectionJSON;
					sectionJSON.clear();
				}

				section.clear();

				size_t len = ::strlen(buffer);
				if (len > 1U && buffer[len - 2U] == ']') {
					buffer[len - 2U] = '\0';
					section = std::string(buffer + 1U);
				}

				continue;
			}

			char* key = ::strtok(buffer, " \t=\r\n");
			if (key == nullptr)
				continue;

			char* value = ::strtok(nullptr, "\r\n");
			if (value == nullptr)
				continue;

			// Remove quotes from the value
			size_t len = ::strlen(value);
			if (len > 1U && *value == '"' && value[len - 1U] == '"') {
				value[len - 1U] = '\0';
				value++;
			}

			if (!section.empty()) {
				// Filter out excluded keys
				if (std::find(exclusions.cbegin(), exclusions.cend(), key) == exclusions.cend())
					sectionJSON[key] = value;
			}
		}

		if (!sectionJSON.empty())
			json[section] = sectionJSON;

		::fclose(fp);
	}

	if (!json.empty()) {
		WriteJSON(name, json);

		return true;
	} else {
		writeJSONMessage("Invalid config name");

		return false;
	}
}
