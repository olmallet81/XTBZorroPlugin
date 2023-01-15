# XTBZorroPlugin

## Introduction
XTB is a low-commission broker offering a wide range of tradables instruments. For opening a demo account go to: https://www.xtb.com/en/demo-account.

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

#### <ins>Broker API functions</ins>
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

BrokerHistory2 has not been implemented as XTB does not provide any historical data download service.
  
#### <ins>Broker commands</ins>
The following commands have been implemented:
- GET_TIME
- GET_DIGITS
- GET_MINLOT
- GET_LOTSTEP
- GET_MAXLOT
- GET_SERVERSTATE
- GET_BROKERZONE
- GET_DELAY
- GET_NTRADES
- GET_POSITION
- GET_AVGENTRY
- GET_TRADES
- GET_TRADEPOSITION (new command added for getting the current position on a trade by its id)
- GET_WAIT
- SET_DELAY
- SET_PATCH (returns 16 as rollover and commission computation are not implemented yet)
- SET_WAIT
- SET_LASTCONNECTION (new command added for setting the last connection date for getting all closed trades since the last connection)

#### <ins>Streaming</ins>
All streaming services (tick prices, balance and trades) run on another thread. The plugin will start this thread will start when the user will click on the Zorro Trade button and the plugin will wait for this thread to stop when clicking on Zorro Stop button.  

#### <ins>Log</ins>
A log file is automatically generated at each connection to XTB and dumped into the Zorro\Log\XTB folder.

## Known Issues
Limit/Stop orders to reduce or close a trade cannot be deleted if their size is greater than 50% of the trade size.
Hence, this plugin has been implemented to cap limit/stop orders to 50% of the opened trade current position otherwise the plugin would not be able to close a trade with such an order attached.

## MIT License
This project is MIT-licensed. See the LICENSE.md file for more details.
