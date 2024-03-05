# Auto light
Attiny10 &amp; APDS-9930 based automatic light/switch

https://github.com/N-Storm/autolight [![Makefile CI](https://github.com/N-Storm/autolight/actions/workflows/makefile.yml/badge.svg?branch=master)](https://github.com/N-Storm/autolight/actions/workflows/makefile.yml)

https://easyeda.com/NStorm/autolight-board

<a href="https://oshpark.com/shared_projects/VqVSVvTY"><img src="https://oshpark.com/packs/media/images/badge-5f4e3bf4bf68f72ff88bd92e0089e9cf.png" alt="Order from OSH Park"></img></a>

This project utilizes small ATtiny10 mcu to work with APDS-9930 ambient light (AL) and proximity I2C sensor.
This is used for automatic "smart" light in a drawer. Once the drawer are open the APDS-9930 will notice proximity change 
and trigger a preset interrupt and MCU will come from sleep to check if AL level are below set (i.e. it's dark around).
If the set condition for AL level will be met, it will enable HIGH level on PB3 pin. Which is connected to a MOSFET or any other switch to enable LED light.

Reset shares same PB3 pin so it must be disabled via fuses. You will have to apply +12V to PB3/RESET pin to reprogram attiny10 after flashing RSTDISBL fuse.

The PB2 port are used as INT0 for sensor interrupts to wake MCU from sleep on proximity change event.

## Building

Fetch the latest trunk:

`git clone https://github.com/N-Storm/autolight && cd autolight`

Then tweak some settings in autolight.h if required. They are commented. After that do a:

`make && make flash && make fuse`. 

### :warning: After you do a `make fuse` step, you won’t be able to program MCU again without applying +12V to RESET pin (make sure it’s not connected to your programmer hardware :warning:

Alternatively you can use pre-built binaries with the default settings from [releases section](https://github.com/N-Storm/autolight/releases).

## Connecting

PB0 ↔ SDA<br />
PB1 ↔ SCL<br />
PB2 ↔ INT<br />

PB3 are the module digital output, with logic LOW when light should be turned off and HIGH when it should be on. Suited for connecting to an N-channel FET driving the LED light.

Please be aware that the APDS-9930 module requires 2.5 – 3.6 V for it’s operation on Vdd+Vbatt (IR LED power). I recommend to use a 3.3V LDO for powering both MCU & APDS-9930.

Schematics & board are available on EasyEDA: https://easyeda.com/NStorm/autolight-board

### :warning: This board has two places for LDOs - U3 & U4. Solder only one of them! :warning:

They replace each other. The board are made so they could replace each other. Alternatively edit board/schematics to replace them with your favorite LDO. 

This project utilizes software I2C library by eXtreme Electronics, India.
www.eXtremeElectronics.co.in
