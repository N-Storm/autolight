# Auto light experimental attiny13 branch (untested)
Attiny13 &amp; APDS-9930 based automatic light/switch

https://github.com/N-Storm/autolight

This project utilizes small ATtiny10 mcu to work with APDS-9930 ambient light (AL) and proximity I2C sensor.
This is used for automatic "smart" light in a drawer. Once the drawer are open the APDS-9930 will notice proximity change 
and trigger a preset interrupt and MCU will come from sleep to check if AL level are below set (i.e. it's dark around).
If the set condition for AL level will be met, it will enable HIGH level on PB4 pin. Which is connected to a MOSFET or any other switch to enable LED light.

The PB1 port are used as INT0 for sensor interrupts to wake MCU from sleep on proximity change event.

## Building

Fetch the latest trunk:
`git clone https://github.com/N-Storm/autolight && cd autolight`
Then tweak some settings in autolight.h if required. They are commented. After that do a:
`make && make flash && make fuse`. 

## Connecting

PB0 ↔ SCL<br />
PB1 ↔ INT<br />
PB2 ↔ SDA<br />

PB4 are the module digital output, with logic LOW when light should be turned off and HIGH when it should be on. Suited for connecting to an N-channel FET driving the LED light.

Please be aware that the APDS-9930 module requires 2.5 – 3.6 V for it’s operation on Vdd+Vbatt (IR LED power). I recommend to use a 3.3V LDO for powering both MCU & APDS-9930.

This project utilizes software I2C library by eXtreme Electronics, India.
www.eXtremeElectronics.co.in
