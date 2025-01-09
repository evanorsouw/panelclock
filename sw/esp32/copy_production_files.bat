
set ENV=panel16
set TARGET=D:\OneDrive\whitemagic\promotie\website\whitemagic.it\panelclock

copy data\*.* %TARGET%
copy .pio\build\%ENV%\firmware.bin %TARGET%
