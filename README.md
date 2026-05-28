The purpose of MMDVM-Info is to allow limited access to environmental information via MQTT, particularly from a remote location. This can be used by dashboards to access (for example) the ini files used by the various programs, and have the information returned in JSON format.

It provides four pieces of information, firstly the contents of one of the named ini files in JSON format. Comments are filtered out, as well as empty lines, and the program has the option to exclude sensitive configuration options, for example, passwords. Keys to be excluded are listed in the ini file for MMDVM-Info.

The second sort of data is about network interfaces that face the outside world, this means that information about loopback interfaces is ignored. Both the IPv4 and IPv6 address are listed for each interface if available.

Thirdly, a list of MMDVM programs that are running on the system is returned. This list of programs is to be found in the MMDVM-Info ini file which restricts the information available. Any program can be added, and if it is running, its name appears along with with its PID. Unlike the other information, this is supplied periodically based on a configurable timer, by default every minute.

Finally it provides periodic updates of the status of the CPU, the frequency of these updates is settable within the configuration file. The default isonce per minute.

It builds on 32-bit and 64-bit Linux as well as on Windows using Visual Studio 2022 on x86 and x64.

This software is licenced under the GPL v2 and is primarily intended for amateur and educational use.
