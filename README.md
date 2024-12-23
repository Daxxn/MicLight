# Mic Light

A light on a stick! This is a simple work light that can be mounted on a mic stand.

## Thermals

This thing is very bright but cannot handle the amount of heat generated at full brightness. There is a dimming system that with start to dim the lights when the temperature starts to rise. Future versions will have space for heatsinks, but for now, this works. I've been using this light for months with no issues so hopefully it's not a huge problem.

## Power Supply

Its a power hog and needs a **good** USB Type-C PD 3.0 power source to function. It can run from a 40W supply but a 65W or better is recommended. I've limited it to Type-C power supplies only. I don't have the time to get this working with normal Type-A connectors and the [PD IC](./Docs/Datasheets/AP33772.pdf) is designed to use the CC lines only. Besides the LEDs need a 6V forward voltage anyway. It's just not designed for 5V power supplies.

## USB Communication

The [MCU](./Docs/Datasheets/STM32F072.pdf) has a full USB stack and is able to communicate over a virtual COM port. It's not complete at the moment though. I don't have a Type-C hub that can supply the needed power so I can't test the complete system.

### PCB Design File:

This board was designed when I was still using Altium Designer. I don't have the project file anymore. The [Schematic](./Docs/MicLight_Schematic_REV5.pdf) and gerber files are the best I got. The next revision will have a full KiCAD project file.