#Ovde-boot

Bootloader based on the lwIP tcp/udp stack for the stm32F7-discovery development board.

The bootloader can be used to flash every program written for stm32F7 over ethernet.
This can be done with the provided uploading software, written in qt.

It is possible to send a running application a command to reboot in bootloader mode for uploading a update in runtime.

It should be possible to modify this project to work with othere embedded systems.

## Used library's and compiler:

- SW4Stm32 developement environement
- Lwip 1.5.0
- Stm32 HAL library's (created with CubeMx) (V1.5.1)
- Qt5.7 (Upload software)

## Conditions
Every programm can be flashed to the stm32f7 Discovery if it meets these conditions:
- Doesn't use Timer2
- Doesn't use more than 160kb of Ram
- Doesn't use more than 896kb of FLASH

The prepare-script is written to convert Stm32f7 v1.5.1 generated projects.
If you use anothere generator, you should take the prepare-script as a base for a custom script.

## Usage

1. Compile the ovde-bootloader and flash it to stm32f7 discovery via openOcd or St-link
2. Copy prepareOvde-boot.php into the root of the program to flash (ex: /blinky)
3. Cd into the directory and run:
```
php prepareOvde-boot.php
```
4. In eclipse (or othere ide) do 'make clean' to fore a rebuild of your project
5. Open the upload software and flash your compiled bin to the device
6. Restart your device
