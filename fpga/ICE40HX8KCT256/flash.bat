SET BITMAP=ICE40HX8KCT256_Implmnt\sbt\outputs\bitmap\toplevel_bitmap.bin
SET COMPORT=com9

mode %COMPORT% baud=115200 data=8 dtr=off stop=1 parity=n rts=off to=on
copy /b %BITMAP% \\.\%COMPORT%
