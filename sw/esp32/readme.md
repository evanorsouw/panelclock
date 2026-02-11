# Board config

Because I used a ESP32-WROVER-E 16Mb (and with PSRAM) module on a custom PCB, I created a custom board configuration.
I copied an existing board file using a wrover module and modified it.

This board configuration, named `standalone-panel.json` is stored in the `boards`  directory of the project.

From the `platform.ini` you can refer to it by using the filename without json extension.

There were the links I used:
- <https://docs.platformio.org/en/stable/platforms/creating_board.html>
- <https://docs.platformio.org/en/latest/platforms/espressif32.html>

# Storing FPGA bitstream on ESP32 filesystem

SPIFFS is the concept used to stored data file in ESP32 flash memory. 
The ESP32-WROVER-E has 16MB of flash. you have to create a custom partition table to set space aside for the filesystem.

The gist is as follows:
- create custom partition table, e.g. `partitions.csv` and store it in the root of the project.
- copy the contents of an existing partition file and add the following line:
  - `storage,  data, spiffs,  ,        2M` to reserve 2Mb for the data partition.
- in `platform.ini` add an entry `board_build.partitions = partitions.csv`
- Add the following line to the file `CMakeFile.txt`:
  - `spiffs_create_partition_image(spiffs data)` 
- create the directory `data` in the root of yout project
- place the file you want to have available on the ESP32 in the `data` directory.
- run `menuconfig` and make the following change
  - in the partitions section select `Custom partition table CSV` and name it `partitions.csv`
- Now from platformio, select `Build Filesystem Image` and `Upload Filesystem Image`
  or  or even simpler, just use `Upload Filesystem Image`.


There were the links I used:
- https://community.platformio.org/t/unable-to-build-and-upload-spiffs-filesystem-image-with-framework-esp-idf/17820


# 2 Updating software

1. Build the software  
When all is fine, a firmware package `firmware.bin` is copied to the `sw\esp32\data` directory, along with a new manifest that contains a newer version number. 
2. Upload the files `firmware.bin` and `manifest.bin` to the web location that is monitored by the panelclock (default configuration: `whitemagic.it/panelclock`)
3. The panelclock will autmomatically pick it up next time it checks. By default the interval setting `Update` setting  is set to `weekly`. During development this can be changed to `minutely`.
