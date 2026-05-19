The purpose of MMDVM-Info is to allow limited access to environmental information via MQTT, particularly from a remote location. This can be used by dashboards to access (for example) the ini files used by the various programs, and have the information returned in JSON format.

Initially the program will provide only specified configuation information, but will be expanded later to include other information such as IP addresses and other platform dependent data.

When accessing ini files, the program has the option to excluse sensitive configuration options, for example, passwords. Keys to be excluded are listed in the ini file for MMDVM-Info.

It builds on 32-bit and 64-bit Linux as well as on Windows using Visual Studio 2022 on x86 and x64.

This software is licenced under the GPL v2 and is primarily intended for amateur and educational use.
