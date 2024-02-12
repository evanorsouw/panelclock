
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







Add to FPGA code:

- setcolor(r,g,b)
0x03,<R>,<G>,<B>

- setpixel(x,y,alpha) - use current color and blend pixel at x,y with existing pixel using <alpha>
0b10aaaaaa,<x>,<y>

- setnextpixel(alpha) - use current color and last position (x+1,y) and blend existing pixel using alpha then update current position to (x+1,y)
0b11aaaaaa 



the FPGA must get ability to read an RGB pixel value back from memory.
- writing a pixel takes 8 writes (each RGB bit need 1 read)
- reading a pixel also takes 8 writes

the alpha pixel blend must:
- read 24 pixel from memory 
- merge color using 6 bit alpha value:
  - calculate the color value for each RGB channel in the pixel:
    - clip(old * (63-alpha) + new * alpha, 255)
- write 24 pixel back to memory

performance:
- each read/write of a set of RGB pixels takes 2 clock cycles
- given 8 reads and 8 write leaves requires 16*x = 32 clock cycles.
- at 60MHz this leaves an ideal bandwith of ~1.877 million pixels, or 228 full 128x64 screens

using above techniques we can render a screen that has say 30% filling:
- worst case: 128 * 64 * 30% * 3 (setpixel) = 7372 bytes (@115200 baud = 0.64 sec)
- with 90% scanlines: (128 * 64 * 0.3) * 10% * 3 + (128 * 64 * 0.3) * 90% * 1 = 2949 (@115200 baud = 0.256 sec)

