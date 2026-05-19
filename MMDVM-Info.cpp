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

#include "MMDVM-Info.h"

#include "MQTTConnection.h"
#include "StopWatch.h"
#include "Version.h"
#include "Thread.h"
#include "Timer.h"
#include "Utils.h"
#include "Log.h"
#include "GitVersion.h"

#include <cstring>
#include <cassert>

#include <vector>

#if !defined(_WIN32) && !defined(_WIN64)
#include <sys/types.h>
#include <signal.h>
#include <pwd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
const char* DEFAULT_INI_FILE = "MMDVM-Info.ini";
#else
const char* DEFAULT_INI_FILE = "/etc/MMDVM-Info.ini";
#endif

static bool m_killed = false;
static int  m_signal = 0;
static bool m_reload = false;

const int BUFFER_SIZE = 500;

// In Log.cpp
extern CMQTTConnection* m_mqtt;

#if !defined(_WIN32) && !defined(_WIN64)
static void sigHandler1(int signum)
{
	m_killed = true;
	m_signal = signum;
}

static void sigHandler2(int signum)
{
	m_reload = true;
}
#endif

static CMMDVMInfo* info = nullptr;

int main(int argc, char** argv)
{
	std::string iniFile = DEFAULT_INI_FILE;

	if ((argc == 2) && ((::strcmp(argv[1U], "-v") == 0) || (::strcmp(argv[1U], "--version") == 0))) {
		::fprintf(stdout, "MMDVM-Info version %s git #%.7s\n", VERSION, gitversion);
		return 0;
	}

	if (argc == 2) {
		iniFile  = std::string(argv[1U]);
	} else {
		::fprintf(stderr, "Usage: MMDVM-Info [-v|--version] [filename]\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	::signal(SIGINT, sigHandler1);
	::signal(SIGTERM, sigHandler1);
	::signal(SIGHUP, sigHandler1);
	::signal(SIGUSR1, sigHandler2);
#endif

	int ret = 0;

	do {
		m_signal = 0;

		info = new CMMDVMInfo(iniFile);
		ret = info->run();

		delete info;
		info = nullptr;

		switch (m_signal) {
		case 2:
			::LogInfo("MMDVM-Info-%s exited on receipt of SIGINT", VERSION);
			break;
		case 15:
			::LogInfo("MMDVM-Info-%s exited on receipt of SIGTERM", VERSION);
			break;
		case 1:
			::LogInfo("MMDVM-Info-%s exited on receipt of SIGHUP", VERSION);
			break;
		case 10:
			::LogInfo("MMDVM-Info-%s is restarting on receipt of SIGUSR1", VERSION);
			break;
		default:
			::LogInfo("MMDVM-Info-%s exited on receipt of an unknown signal", VERSION);
			break;
		}
	} while (m_signal == 10);

	::LogFinalise();

	return ret;
}

CMMDVMInfo::CMMDVMInfo(const std::string& fileName) :
m_conf(fileName),
m_configs()
{
	assert(!fileName.empty());
}

CMMDVMInfo::~CMMDVMInfo()
{
}

int CMMDVMInfo::run()
{
	bool ret = m_conf.read();
	if (!ret) {
		::fprintf(stderr, "MMDVM-Info: cannot read the .ini file\n");
		return 1;
	}

#if !defined(_WIN32) && !defined(_WIN64)
	bool m_daemon = m_conf.getDaemon();
	if (m_daemon) {
		// Create new process
		pid_t pid = ::fork();
		if (pid == -1) {
			::fprintf(stderr, "Couldn't fork() , exiting\n");
			return -1;
		} else if (pid != 0) {
			exit(EXIT_SUCCESS);
		}

		// Create new session and process group
		if (::setsid() == -1) {
			::fprintf(stderr, "Couldn't setsid(), exiting\n");
			return -1;
		}

		// Set the working directory to the root directory
		if (::chdir("/") == -1) {
			::fprintf(stderr, "Couldn't cd /, exiting\n");
			return -1;
		}

		// If we are currently root...
		if (getuid() == 0) {
			struct passwd* user = ::getpwnam("mmdvm");
			if (user == nullptr) {
				::fprintf(stderr, "Could not get the mmdvm user, exiting\n");
				return -1;
			}

			uid_t mmdvm_uid = user->pw_uid;
			gid_t mmdvm_gid = user->pw_gid;

			// Set user and group ID's to mmdvm:mmdvm
			if (::setgid(mmdvm_gid) != 0) {
				::fprintf(stderr, "Could not set mmdvm GID, exiting\n");
				return -1;
			}

			if (::setuid(mmdvm_uid) != 0) {
				::fprintf(stderr, "Could not set mmdvm UID, exiting\n");
				return -1;
			}

			// Double check it worked (AKA Paranoia)
			if (::setuid(0) != -1) {
				::fprintf(stderr, "It's possible to regain root - something is wrong!, exiting\n");
				return -1;
			}
		}
	}
#endif

	::LogInitialise(m_conf.getLogDisplayLevel(), m_conf.getLogMQTTLevel());

	std::vector<std::pair<std::string, void (*)(const unsigned char*, unsigned int)>> subscriptions;
	subscriptions.push_back(std::make_pair("command", CMMDVMInfo::onCommand));

	m_mqtt = new CMQTTConnection(m_conf.getMQTTAddress(), m_conf.getMQTTPort(), m_conf.getMQTTName(), m_conf.getMQTTAuthEnabled(), m_conf.getMQTTUsername(), m_conf.getMQTTPassword(), subscriptions, m_conf.getMQTTKeepalive());
	ret = m_mqtt->open();
	if (!ret)
		return 1; 

#if !defined(_WIN32) && !defined(_WIN64)
	if (m_daemon) {
		::close(STDIN_FILENO);
		::close(STDOUT_FILENO);
	}
#endif

	m_configs = m_conf.getConfigs();

	LogMessage("MMDVM-Info-%s is starting", VERSION);
	LogMessage("Built %s %s (GitID #%.7s)", __TIME__, __DATE__, gitversion);

	writeJSONMessage("MMDVM-Info is starting");

	while (!m_killed) {
		CThread::sleep(100U);
	}

	LogInfo("MMDVM-Info is stopping");
	writeJSONMessage("MMDVM-Info is stopping");

	return 0;
}

bool CMMDVMInfo::readConfig(const std::string& name, const std::string& path)
{
	nlohmann::json fileJSON;
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
					fileJSON[section] = sectionJSON;
					sectionJSON.clear();
				}

				section.clear();

				size_t len = ::strlen(buffer);
				if (len > 1U && buffer[len - 1U] == ']') {
					buffer[len - 1U] = '\0';
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

			if (!section.empty())
				sectionJSON[key] = value;
		}

		::fclose(fp);
	}

	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();

	if (!fileJSON.empty()) {
		json["status"] = "found";
		json[name]     = fileJSON;

		WriteJSON("response", json);

		return true;
	} else {
		json["status"] = "not found";

		WriteJSON("response", json);

		return false;
	}
}

void CMMDVMInfo::writeJSONMessage(const std::string& message)
{
	nlohmann::json json;

	json["timestamp"] = CUtils::createTimestamp();
	json["message"]   = message;

	WriteJSON("Message", json);
}

void CMMDVMInfo::remoteControl(const std::string& command)
{
	writeJSONMessage("Received command: " + command);

	if (command.substr(0, 6) == "Config") {
		std::string name = command.substr(7);

		for (const auto& it : m_configs) {
			if (it.first == name) {
				readConfig(it.first, it.second);
				return;
			}
		}

		writeJSONMessage("Invalid config name");
	} else {
		writeJSONMessage("Invalid command");
	}
}

void CMMDVMInfo::onCommand(const unsigned char* command, unsigned int length)
{
	assert(info != nullptr);
	assert(command != nullptr);

	info->remoteControl(std::string((char*)command, length));
}
