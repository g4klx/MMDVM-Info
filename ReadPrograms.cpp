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

#include "ReadPrograms.h"

#include "Log.h"

#include <sys/types.h>
#include <dirent.h>

#include <cstring>


CReadPrograms::CReadPrograms(const std::vector<std::string>& programs) :
m_programs(programs)
{
}

CReadPrograms::~CReadPrograms()
{
}

bool CReadPrograms::read() const
{
	nlohmann::json json;

	DIR* dir = ::opendir("/proc");
	if (dir != nullptr) {
		struct dirent* ent;
		while ((ent = ::readdir(dir)) != nullptr) {
			char* endptr;
			long lpid = ::strtol(ent->d_name, &endptr, 10);
			if (*endptr != '\0')
				continue;

			char buffer[512U];
			::sprintf(buffer, "/proc/%ld/cmdline", lpid);

			FILE* fp = ::fopen(buffer, "rt");
			if (fp != nullptr) {
				if (::fgets(buffer, 512, fp) != nullptr) {
					for (const auto& it : m_programs) {
						if (::strstr(buffer, it.c_str()) != nullptr) {
							nlohmann::json pidJSON;
							pidJSON["pid"] = lpid;

							nlohmann::json procJSON;
							procJSON[it] = pidJSON;

							json.push_back(procJSON);
						}
					}
				}

				::fclose(fp);
			}
		}

		::closedir(dir);
	}
	
	WriteJSON("Programs", json);

	return true;
}
