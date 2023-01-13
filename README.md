# XTBZorroPlugin

## Introduction
XTB is a broker offering a large range of tradables instruments. For opening a demo account go to: https://www.xtb.com/en/demo-account.

XTB offers free access to their API. The connection is made through TCP sockets. Link to API their documentation: http://developers.xstore.pro/documentation/#introduction.

This plugin allows to connect to their API from Zorro. It was written in Win32 C++ under Visual Studio 2017.

## Build Instructions
All dependencies have been fully integrated into the folder:

- RapidJson (for JSON parsing and writing)
- OpenSSL (for secure socket connection)

An old version of OpenSSL has been used to match the existing binary files within Zorro and required by this library: libeay32.dll and ssleay32.dll.

This project includes a Visual Studio 2017 solution. You should be able to simply download the entire folder, open the solution, and build it, using the Release x86 build configuration.

## Installation Instructions
To install the plugin, simply place the binary file XTB.dll in the Plugin folder where Zorro is installed.

## Plugin Description
The following Broker API Zorro functions are implemented:
- BrokerOpen
- BrokerLogin
- BrokerTime
- BrokerAccount
- BrokerAsset
- BrokerBuy2
- BrokerTrade
- BrokerStop
- BrokerSell2
- BrokerCommand

## Known Issues
- Limit/Stop orders to partially close a trade cannot be deleted if their size if greater than 50% of the trade total size.

## MIT License
This project is MIT-licensed. See the LICENSE.md file for more details.
