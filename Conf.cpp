/*
 *	 Copyright (C) 2015,2016,2017,2024,2026 by Jonathan Naylor G4KLX
 *
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the Free Software
 *	 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Conf.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>

#include <utility>

const int BUFFER_SIZE = 500;

enum class SECTION {
	NONE,
	GENERAL,
	LOG,
	MQTT,
	EXCLUDE,
	CONFIGS,
	PROGRAMS
};

CConf::CConf(const std::string& file) :
m_file(file),
m_daemon(false),
m_logDisplayLevel(0U),
m_logMQTTLevel(0U),
m_mqttAddress("127.0.0.1"),
m_mqttPort(1883U),
m_mqttKeepalive(60U),
m_mqttName("info"), 
m_mqttAuthEnabled(false),
m_mqttUsername(),
m_mqttPassword(),
m_exclusions(),
m_configs(),
m_programs()
{
}

CConf::~CConf()
{
}

bool CConf::read()
{
	FILE* fp = ::fopen(m_file.c_str(), "rt");
	if (fp == nullptr) {
		::fprintf(stderr, "Couldn't open the .ini file - %s\n", m_file.c_str());
		return false;
	}

	SECTION section = SECTION::NONE;

	char buffer[BUFFER_SIZE];
	while (::fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
		if (buffer[0U] == '#')
			continue;

		if (buffer[0U] == '[') {
			if (::strncmp(buffer, "[General]", 9U) == 0)
				section = SECTION::GENERAL;
			else if (::strncmp(buffer, "[Log]", 5U) == 0)
				section = SECTION::LOG;
			else if (::strncmp(buffer, "[MQTT]", 6U) == 0)
				section = SECTION::MQTT;
			else if (::strncmp(buffer, "[Exclude]", 9U) == 0)
				section = SECTION::EXCLUDE;
			else if (::strncmp(buffer, "[Configs]", 9U) == 0)
				section = SECTION::CONFIGS;
			else if (::strncmp(buffer, "[Programs]", 10U) == 0)
				section = SECTION::PROGRAMS;
			else
				section = SECTION::NONE;

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

		if (section == SECTION::GENERAL) {
			if (::strcmp(key, "Daemon") == 0)
				m_daemon = ::atoi(value) == 1;
		} else if (section == SECTION::LOG) {
			if (::strcmp(key, "DisplayLevel") == 0)
				m_logDisplayLevel = (unsigned int)::atoi(value);
			else if (::strcmp(key, "MQTTLevel") == 0)
				m_logMQTTLevel = (unsigned int)::atoi(value);
		} else if (section == SECTION::MQTT) {
			if (::strcmp(key, "Address") == 0)
				m_mqttAddress = value;
			else if (::strcmp(key, "Port") == 0)
				m_mqttPort = uint16_t(::atoi(value));
			else if (::strcmp(key, "Keepalive") == 0)
				m_mqttKeepalive = (unsigned int)::atoi(value);
			else if (::strcmp(key, "Name") == 0)
				m_mqttName = value;
			else if (::strcmp(key, "Auth") == 0)
				m_mqttAuthEnabled = ::atoi(value) == 1;
			else if (::strcmp(key, "Username") == 0)
				m_mqttUsername = value;
			else if (::strcmp(key, "Password") == 0)
				m_mqttPassword = value;
		} else if (section == SECTION::EXCLUDE) {
			if (::strcmp(key, "Key") == 0)
				m_exclusions.push_back(value);
		} else if (section == SECTION::CONFIGS) {
			std::pair<std::string, std::string> data = std::make_pair(key, value);
			m_configs.push_back(data);
		} else if (section == SECTION::PROGRAMS) {
			if (::strcmp(key, "Program") == 0)
				m_programs.push_back(value);
		}
	}

	::fclose(fp);

	return true;
}


bool CConf::getDaemon() const
{
	return m_daemon;
}

unsigned int CConf::getLogDisplayLevel() const
{
	return m_logDisplayLevel;
}

unsigned int CConf::getLogMQTTLevel() const
{
	return m_logMQTTLevel;
}

std::string CConf::getMQTTAddress() const
{
	return m_mqttAddress;
}

uint16_t CConf::getMQTTPort() const
{
	return m_mqttPort;
}

unsigned int CConf::getMQTTKeepalive() const
{
	return m_mqttKeepalive;
}

std::string CConf::getMQTTName() const
{
	return m_mqttName;
}

bool CConf::getMQTTAuthEnabled() const
{
	return m_mqttAuthEnabled;
}

std::string CConf::getMQTTUsername() const
{
	return m_mqttUsername;
}

std::string CConf::getMQTTPassword() const
{
	return m_mqttPassword;
}

std::vector<std::string> CConf::getExclusions() const
{
	return m_exclusions;
}

std::vector<std::pair<std::string, std::string>> CConf::getConfigs() const
{
	return m_configs;
}

std::vector<std::string> CConf::getPrograms() const
{
	return m_programs;
}
