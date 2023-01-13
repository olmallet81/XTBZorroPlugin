# XTBZorroPlugin
XTB is a broker offering a large range of tradables instruments.
For opening a demo account go to: https://www.xtb.com/en/demo-account.

XTB offers free access to their well documented API: http://developers.xstore.pro/documentation/#introduction.

This plugin allows to connect to their API from Zorro. It was written in Win32 C/C++ using Visual Studio 2017.

# Build Instructions
All dependencies have been fully integrated into the folder and slightly modified where necessary. They are:

liboauthcpp (for OAuth 1.0a authentication)
pugixml (for XML parsing and writing)
This project includes a Visual Studio 2017 solution. You should be able to simply download the entire folder, open the solution, and build it, using the Release x86 build configuration.
