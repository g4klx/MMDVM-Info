/*
 *   Copyright (C) 2024,2026 by Jonathan Naylor G4KLX
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

#if !defined(MMDVM_Info_H)
#define	MMDVM_Info_H

#include "Conf.h"

#include <vector>
#include <string>
#include <map>

class CMMDVMInfo {
public:
	CMMDVMInfo(const std::string& fileName);
	~CMMDVMInfo();

	int run();

private:
	CConf m_conf;

	std::vector<std::string> m_exclusions;
	std::vector<std::pair<std::string, std::string>> m_configs;

	bool readConfig(const std::string& name, const std::string& path);

	void remoteControl(const std::string& commandString);

	void writeJSONMessage(const std::string& message);

	static void onCommand(const unsigned char* command, unsigned int length);
};

#endif
