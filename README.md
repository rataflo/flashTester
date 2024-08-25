# Flash Tester
Arduino device to test studio flash.

Fire flash every xx seconds and record if the flash actually flash or not.

![Video in action](https://github.com/rataflo/flashTester/blob/main/doc/20240824_201901.mp4)

Possible evolution: Calculate the time between sending the order to the flash and when the flash is performed.

## Usage

- Thresold: Choose the min photo resitor value to catch a flash. Max 1024. Default 900. You also can adjust sensitivity directly on the photo resistor with a small potentiometer.
- Cycles: Number of cycles to test flash.
- Recycling : time between flashing to let the flash recycle.
- Good: value of previous test.
- Bad: value of previous test.
- Launch test : start the test. When finished the screen go to the main menu.

Every parameters and test values are kept in the arduino memory.

## BOM
- Arduino nano.
- Photo resistor. https://amzn.eu/d/iNbF2g8
- Oled screen with I2C.https://amzn.eu/d/dOr0XgS
- rotary encoder + switch. https://amzn.eu/d/dy2wDRH
- NPN 2222 transistor.
- 4.7k resistor.
- jack cable. Prefer the one with only 2 pin. If 3 pins, the GND as to be put on the 2 parts next to cable housing.

## Schematic
![Draw of schematic](https://github.com/rataflo/flashTester/blob/main/doc/schematic.png)
