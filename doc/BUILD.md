# How to build the UFO firmware yourself. 
Please be warned that we cannot provide any support or warranty as soon as you upload non-officially released firmware.

## get the build environment
* [https://esp-idf.readthedocs.io](https://esp-idf.readthedocs.io)

* Windows, Mac, Linux [https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html)

* Windows Subsystem for Linux (WSL)
on Windows 10 1803 and newer you can use the WSL with e.g. Ubuntu. This allows you to still code in Windows (e.g. Visual Code) yet compile and flash in Linux on WSL.
    * [https://docs.espressif.com/projects/esp-idf/en/latest/get-started/linux-setup.html](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/linux-setup.html)
    * to make e.g. COM5 accessible in WSL you need to run ```sudo chmod 666 /dev/ttyS5```
    * ```make monitor``` launches miniterm to see the UFO output via USB-to-serial interface. to stop it you need to invoke the Ctrl-] key. If you have a non-english keyboard, you still need to use same key location as on english keyboard (e.g. Ctrl-+ on german keyboard)

## getting the latest version
Tested with ESP-IDF version 3.1.1:
```
git clone --recursive --branch release/v3.1.1 https://github.com/espressif/esp-idf.git
git submodule update 
```

## build settings
### run make menuconfig
```
make menuconfig
``` 

* Serial flasher config --> (COMx) Default serial port e.g. `COM4` on Windows (you find on Windows COM port in device manager), `/dev/ttyS4` on WSL or `/dev/ttyUSB0` on Linux (look at /dev/tty*); you can use speeds up to 921600bps
* Serial flasher config --> Flash size --> 4 MB
* Component config --> ESP32-specific --> (10) Task watchdog timeout (seconds)
* Component config --> ESP32-specific --> Core dump destination --> Flash
* Component config --> Amazon Web Services IoT Platform --> turn ON [*]
* Partition table --> Custom parition table --> partitions.csv


## useful build commands
executed in the ufo-esp32 folder
* ``make menuconfig`` to create buildenvironment
* ``make erase_flash`` to erase all partitions of the flash
* ``make clean`` to force a fresh build from scratch
* ``make -j all`` just build all with max parallel threads (omit -j in case of compile errors caused by race conditions)
* ``make -j all flash monitor`` build all, flash to UFO, and start monitoring over serial interface
* ``make monitor`` reboots the UFO and monitors output over serial interface

