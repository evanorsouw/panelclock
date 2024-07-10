# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/espressif/esp-idf/components/bootloader/subproject"
  "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader"
  "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader-prefix"
  "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader-prefix/tmp"
  "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader-prefix/src/bootloader-stamp"
  "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader-prefix/src"
  "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/projects/fpgaclock/sw/esp32_clock/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
