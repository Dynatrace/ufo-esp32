# How to build the UFO firmware yourself. 
Please be warned that we cannot provide any support or warranty as soon as you upload non-officially released firmware.

## get the build environment
* [https://esp-idf.readthedocs.io](https://esp-idf.readthedocs.io)

* [https://docs.espressif.com/projects/esp-idf/en/latest/get-started/windows-setup.html](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/windows-setup.html)

## getting the latest version
Tested with ESP-IDF version 3.0:
```
git clone --recursive --branch release/v3.0 https://github.com/espressif/esp-idf.git
git submodule update 
```

## build settings
### run make menuconfig
```
make menuconfig
``` 

* Serial flasher config --> (COMx) Default serial port e.g. `COM4` on Windows (you find on Windows COM port in device manager), or `/dev/ttyUSB0` on Linux (look at /dev/tty*); you can use speeds up to 921600bps
* Serial flasher config --> Flash size --> 4 MB
* Component config --> ESP32-specific --> (10) Task watchdog timeout (seconds)
* Component config --> ESP32-specific --> Core dump destination --> Flash
* Component config --> Amazon Web Services IoT Platform --> turn ON [*]
* Partition table --> Custom parition table --> partitions.csv


## useful build commands
* ``make erase_flash`` to erase all partitions of the flash
* ``make clean`` to force a fresh build from scratch
* ``make -j all`` just build all with max parallel threads (omit -j in case of compile errors caused by race conditions)
* ``make -j flash monitor`` build all, flash to UFO, and start monitoring over serial interface
