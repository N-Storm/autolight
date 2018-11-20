# Auto light
Attiny10 &amp; APDS-9930 based automatic light/switch

This project utilizes small attiny10 mcu to work with APDS-9930 ambient light (AL) and proximity I2C sensor.
This is used for automatic light in a drawer. Once the drawer are open the APDS-9930 will notice proximity change 
and trigger a pre-set interrupt and MCU will come from sleep to check if AL level are below set (i.e. it's dark around).
If the set condition for AL level will be met, it will enable HIGH level on PB2 pin. Which is connected to MOSFET enabling 
LED light.

This project utilizes software I2C library by eXtreme Electronics, India.
