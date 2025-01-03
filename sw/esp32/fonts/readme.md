I use a number of different fonts for both text as wel as graphics;

- `ArialRoundedMTExtraBold.ttf`
  => stripped of excess characters using FontForge and generated as new truetype 
  font into `ArialRoundBold-strip.ttf` to reduce memory. 
  Note that during this process the space was somehow also removed. To restore that
  i copied an arbitrary character, removed all content and reduced its width to 308.
- ArialRoundedMTRegular.ttf
  => also stripped and generated as ArialRoundRegular-strip.ttf
- All .svg files we turned into a panel-icons font.
  I used the same character mechanism to draw icons like circles, clouds etc.
  All because I did not have the means to draw filled arcs and elippses.

These truetype fonts are loaded in memory and used at different sizes.
Rendering is done using the opensource 'script.c' library.

- `fixedfont.txt`
- `fixedfont-condensed.txt`

These are bitmap fonts in their textual form.
I made up the format myself.
The `tools\fontcompiler` directory contains an simple application
to convert it into a binary form, that can readily be loaded into memory
and access as-is without much further ado.
This font is rendered by setting individual pixels on the exact pixel boundary.
It is very fast compared to truetype, looks very crisp but not so appealing.
I use the fixed fonts only for the settings pages, to avoid confusion in some
small truetype letters. The condensed font is used for the 1-panel version.

Use the batchfile `compile-fixed-fonts.bat` to generate the binary versions and 
copy them to the data directory.

All outputfiles are collected in the `sw\esp32\data` directory.



