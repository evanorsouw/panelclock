
programming the ICE40.

Using my own Flash Programmer
--------------------------------
mode com12 baud=115200 data=8 dtr=off stop=1 parity=n rts=off
copy /b ice40_Implmnt\sbt\outputs\bitmap\toplevel_bitmap.bin \\.\com12




using Olimes 32u4 board;
-----------------------

1) have a Olimex 32U4 board programmed and connected via comport.
   determine comport (must be <10). if higher, then use device manager to change the comport
2) have the iCE40HX8K0-EVB board connected via 16pin flatcable.
   and powered via external jack to 5V.
4) from cmd prompt:
   winiceprog.exe -ICOMX -t
   => Manufacturer ID: 0x1C / Device ID: 0x7015
   this is the indication that the ICE40 board is found.
5) winiceprog -ICOM2 -v <binfile>
e.g.: WinICEProg.exe -ICOM2 -v ice40\ice40_Implmnt\sbt\outputs\bitmap\toplevel_bitmap.bin



