# SpriteSheetSplitter
Automate the crust away

## What
Splits traditional sprite sheets into singular files as required by RotMG: Exalt.

Supports splitting of ground, object, and character sprites.
Character sprites with more than the default (5) sprites of animation are not yet supported (as of Sept 2023)

Has a number of options to configure the behavior of the program, accessible through the command line or a configuration json file.
The configurable features include, but are not limited to:

* configurable input directory
* configurable output directory
* Work on a single file, or an entire directory of files.
* skip over empty (alpha) sprites in spritesheet numbering. Alpha sprites are never saved, but they may be accounted for in the naming of the output files.
* dedicate an output subfolder named after the sprite sheet that was split.

## How

This program uses C++ with OpenMP's auto-vectorisation and auto-paralellisation to rapidly split massive amounts of sprites.

[lodepng](https://lodev.org/lodepng/) is used for encoding and decoding png files.

[struct_mapping](https://github.com/bk192077/struct_mapping) is used for mapping JSON to C++ structs.
