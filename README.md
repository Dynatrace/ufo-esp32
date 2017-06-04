# ufo-esp32
2nd generation of Dynatrace UFO with ESP32 microcontroller.
If you dont want to create one yourself, you can buy it here: [https://dynatrace.com/ufo](https://dynatrace.com/ufo)


Note: this is still under development and pre-release.

![ufo builds devops culture](ufobuildsdevopsculture.jpg) 
[https://www.youtube.com/watch?v=6z9BTHhvWSU](https://www.youtube.com/watch?v=6z9BTHhvWSU)


# get the build environment
* [http://esp-idf.readthedocs.io](http://esp-idf.readthedocs.io)

* [http://esp-idf.readthedocs.io/en/latest/windows-setup.html](http://esp-idf.readthedocs.io/en/latest/windows-setup.html)

# getting the latest version
Tested with ESP-IDF version 2.1:
```
git clone --recursive --branch release/v2.1 https://github.com/espressif/esp-idf.git
git submodule update 
```

# build settings
## make menuconfig
* Serial flasher config --> (COMx) Default serial port e.g. COM4 (you find on Windows COM port in device manager); you can use speeds up to 921600bps
* Component config --> ESP32-specific --> (10) Task watchdog timeout (seconds)
* Component config --> ESP32-specific --> Core dump destination --> Flash
* Partition table --> Custom parition table --> partitions.csv


# useful build commands
* ``make erase_flash`` to erase all partitions of the flash
* ``make clean`` to force a fresh build from scratch
* ``make -j flash monitor`` build all, flash to UFO, and start monitoring over serial interface

