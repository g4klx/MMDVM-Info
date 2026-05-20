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

#include "ReadAddresses.h"

#include "Log.h"

#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>

const socklen_t ADDR_LEN = 30;

CReadAddresses::CReadAddresses()
{
}

CReadAddresses::~CReadAddresses()
{
}

bool CReadAddresses::read() const
{
	nlohmann::json json;
	nlohmann::json deviceJSON;

	struct ifaddrs* ifaddr = nullptr;

	int ret = ::getifaddrs(&ifaddr);
	if (ret != 0) {
		writeJSONMessage("No addresses found");
		return false;
	}

	struct addrData {
		std::string type;
		std::string address;
	};

	std::map<std::string, std::vector<addrData>> ifData;

	struct ifaddrs* ptr = ifaddr;
	while (ptr != nullptr) {
		// Only display active interfaces
		if ((ptr->ifa_flags & IFF_UP) == IFF_UP) {
			// We don't care about loopback
			if ((ptr->ifa_flags & IFF_LOOPBACK) != IFF_LOOPBACK) {
				std::string ifName    = std::string(ptr->ifa_name);
				struct sockaddr* addr = ptr->ifa_addr;

				if (addr != nullptr) {
					char address[ADDR_LEN];

					switch (addr->sa_family) {
					case AF_INET: {
							struct sockaddr_in* in4 = (struct sockaddr_in*)addr;
							const char* ret = ::inet_ntop(AF_INET, &in4->sin_addr, address, ADDR_LEN);
							if (ret != nullptr) {
								addrData ipv4;
								ipv4.type    = "IPv4";
								ipv4.address = address;

								auto it = ifData.find(ifName);
								if (it == ifData.end()) {
									std::vector<addrData> arr;
									arr.push_back(ipv4);
									ifData[ifName] = arr;
								} else {
									it->second.push_back(ipv4);
								}
							}
						}
						break;

					case AF_INET6: {
							struct sockaddr_in6* in6 = (struct sockaddr_in6*)addr;
							const char* ret = ::inet_ntop(AF_INET6, &in6->sin6_addr, address, ADDR_LEN);
							if (ret != nullptr) {
								addrData ipv6;
								ipv6.type    = "IPv6";
								ipv6.address = address;

								auto it = ifData.find(ifName);
								if (it == ifData.end()) {
									std::vector<addrData> arr;
									arr.push_back(ipv6);
									ifData[ifName] = arr;
								} else {
									it->second.push_back(ipv6);
								}
							}
						}
						break;

					default:
						break;
					}
				}
			}
		}

		ptr = ptr->ifa_next;
	}

	::freeifaddrs(ifaddr);

	for (const auto& it1 : ifData) {
		nlohmann::json ipJSON;

		for (const auto& it2 : it1.second)
			ipJSON[it2.type] = it2.address;

		nlohmann::json ifJSON;
		ifJSON[it1.first] = ipJSON;

		json.push_back(ifJSON);
	}

	WriteJSON("Addresses", json);

	return true;
}
