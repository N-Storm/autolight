# Auto light
Attiny10 &amp; APDS-9930 based automatic light/switch

https://github.com/N-Storm/autolight

**Work in progress!**

This project utilizes small attiny10 mcu to work with APDS-9930 ambient light (AL) and proximity I2C sensor.
This is used for automatic light in a drawer. Once the drawer are open the APDS-9930 will notice proximity change 
and trigger a preset interrupt and MCU will come from sleep to check if AL level are below set (i.e. it's dark around).
If the set condition for AL level will be met, it will enable HIGH level on PB3 pin. Which is connected to MOSFET enabling 
LED light. Reset shares same PB3 pin so it must be disabled via fuses. You will have to apply +12V to PB3/RESET pin 
to reprogramm attiny10 after flashing RSTDISBL fuse.
The PB2 port are used as INT0 for sensor interrupts to wake MCU from sleep.

Schematics coming soon.

This project utilizes software I2C library by eXtreme Electronics, India.
www.eXtremeElectronics.co.in