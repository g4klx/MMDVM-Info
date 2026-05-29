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

#include "ReadCPU.h"

#include "Log.h"

const unsigned int BUFLEN = 50U;

CReadCPU::CReadCPU()
{
}

CReadCPU::~CReadCPU()
{
}

bool CReadCPU::read() const
{
	char buffer[BUFLEN];

	nlohmann::json json;

	FILE* fp = ::fopen("/sys/class/thermal/thermal_zone0/temp", "rt");
	if (fp != nullptr) {
		char* ptr = ::fgets(buffer, BUFLEN, fp);
		if (ptr != nullptr) {
			int temp = ::atoi(buffer);
			if (temp > 0)
				json["temperature"] = float(temp) / 1000.0F;
		}

		::fclose(fp);
	}

	fp = ::fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq", "rt");
	if (fp != nullptr) {
		char* ptr = ::fgets(buffer, BUFLEN, fp);
		if (ptr != nullptr) {
			int freq = ::atoi(buffer);
			if (freq > 0)
				json["frequency"] = float(freq) / 1000.0F;
		}

		::fclose(fp);
	}

	fp = ::fopen("/proc/loadavg", "rt");
	if (fp != nullptr) {
		char* ptr = ::fgets(buffer, BUFLEN, fp);
		if (ptr != nullptr) {
			char* s = ::strtok(buffer, " ");
			if (s != nullptr) {
				float avg = ::atof(s);
				json["load"] = avg;
			}
		}

		::fclose(fp);
	}

	WriteJSON("CPU", json);

	return true;
}
