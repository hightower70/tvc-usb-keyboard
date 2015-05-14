# TVC USB Keyboard Interface #

Hardware and firmware for connecting modern USB keyboard to the vintage Videoton TV Computer. The interface PCB can be mounted into the place on the original joystick connector, so no irreversible modification need on the computer.

## The hardware ##
The hardware consists only one active element (except the power regulator). This is the PIC32MX250F128D microcontroller. The microcontroller has an USB host port, and it's IO pins directly connected to the keyboard circuit of the TVC. It receives the four bit keyboard row selection signal and issues the eight bit keyboard column signal. The column signal outputs are open-drain outputs, so it can be used work parallel with the original keyboard.

## The firmware ##
The firmware handles the USB host functions and provides an interrupt routine for the keyboard row scan function. Currently only Hungarian keyboard is supported, but the firmware easily can be adapted to other keyboard configuration.

## Further development ##
The near future USB game controller support feature is planned together with english keyboard support.