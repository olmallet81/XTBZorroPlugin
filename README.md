# XTBZorroPlugin
XTB is a broker offering a large range of tradables instruments. For opening a demo account go to: https://www.xtb.com/en/demo-account.

XTB offers free access to their API. The connection is made through TCP sockets. Link to API their documentation: http://developers.xstore.pro/documentation/#introduction.

This plugin allows to connect to their API from Zorro. It was written in Win32 C++ under Visual Studio 2017.

# Build Instructions
All dependencies have been fully integrated into the folder:

- RapidJson (for JSON parsing and writing)
- OpenSSL (for secure socket connection)

An old version of OpenSSL has been used to match the existing binaries within Zorro and required by this library: libeay32.dll and ssleay32.dll.

This project includes a Visual Studio 2017 solution. You should be able to simply download the entire folder, open the solution, and build it, using the Release x86 build configuration.

# Installation Instructions
To install the plugin, simply place the AllyInvest.dll file in the Plugin folder where Zorro is installed.


# MIT License
This project is MIT-licensed. See the LICENSE.md file for more details.
