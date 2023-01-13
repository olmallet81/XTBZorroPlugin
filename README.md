# XTBZorroPlugin
XTB is a broker offering a large range of tradables instruments.
For opening a demo account go to: https://www.xtb.com/en/demo-account.

XTB offers free access to their API. The connection is made through sockets.
Link to API documentation: http://developers.xstore.pro/documentation/#introduction.

This plugin allows to connect to their API from Zorro. It was written in Win32 C++ under Visual Studio 2017.

# Build Instructions
All dependencies have been fully integrated into the folder. They are:

RapidJson (for JSON parsing and writing)
OpenSSL (for secure socket connection)

An old version of this library had been used to match the existing binaries inside Zorro and required by this library: libeay32.dll and ssleay32.dll.

This project includes a Visual Studio 2017 solution. You should be able to simply download the entire folder, open the solution, and build it, using the Release x86 build configuration.


