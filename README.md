# ESP8266 based Arduino wardriving


## What is this?
A simple/cheap hardware + software setup to do wardriving trips and store as much information as we can on a SD card, using a small Arduino box.
Part of the datalogging code stolen from [Sparkfun](https://learn.sparkfun.com/tutorials/gps-logger-shield-hookup-guide/example-sketch-gps-serial-passthrough) guys

## What do we need?
* [$2.98](http://www.ebay.com/itm/222612803340) ESP8266 board (NodeMCU v3 works great)
* [$0.99](http://www.ebay.com/itm/261720518170) Arduino SD Card shield + SD card (a 10 minute trip could generate a 250KB file)
* [$4.21](http://www.ebay.com/itm/142233250679) Blox GY-NEO6MV2 GPS board
* [$3.07](http://www.ebay.com/itm/222311849398) HD44780 i2c LCD screen
* [$1.25](http://www.ebay.com/itm/332023213881) USB powerbank

## What does this do?
* Checks for a working SD Card
* Waits for GPS signal
* Collects all WiFi signals on 2.4Ghz on every (configurable) GPS sample
* Stores this information as a CSV file

## What does this doesn't do?
* This doesn't capture pcap files due power limitations
* This doesn't attack any network

## What does this need?
~~* A better/sorted insertion on CSV files to get rid of duplicate network entries~~ Fixed on d88815b43919bbf54e27a75a84c77be891900372
* Fix some race conditions
* A PCB design that I'm working on
* A better antenna for GPS and WiFi, that will help to speed up satellite lock-up and enlarge WiFi coverage

## What do you need to configure?
Nothing, but you can:
* Setup CS pin for SD card module on *ARDUINO_USD_CS* variable
* Change log file prefix and suffix on *LOG_FILE_PREFIX* and *LOG_FILE_SUFFIX*
* Remove or add columns to CSV datalog on *log_col_names*
* Define GPS log rate time on *LOG_RATE* (milliseconds)
* Define GPS TX/RX pins on *ARDUINO_GPS_RX* and *ARDUINO_GPS_TX*

## Electric setup
Electric setup has nothing special, there is a Fritzing diagram attached as well as a breadboard view of the setup
![](https://i.imgur.com/B4DcCvU.png)

## Working examples
I did a small walking trip around my neighborhood, and put the collected data on a [Google MyMaps](https://www.google.com/maps/d/u/0/viewer?mid=1M18iTRElKQexUaB8HIwEB5qV68c&ll=-34.595372511704866%2C-58.416522499999985&z=18). Note that MyMaps has a limitation of 2000 entries of each CSV (or XML) file. I will try using Open street maps.
![](https://i.imgur.com/PK7mwBZ.jpg)
