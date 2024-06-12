# SpriteSheetSplitter
Automate the crust away.

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

## Example Use

For command line usage, use --help and go from there.

### Config file use:

Point the program to a config file by supplying -c or --config.
By default, a 'config.json' is searched in the directory the program was called from.

`Splitter.exe --config="/path/to/file.json"`

The config file has the following shape:

```
{
  "jobs": [
    <comma separated list of job objects>
  ]
}
```

A job object has the following shape:

```
{
  "in": "/path/to/file/or/folder/",      <-- required
  "out": "/path/to/output/folder/",      <-- required
  "cap": (number),                       <-- [OPTIONAL] maximum number of files to process in this job, default infinite.
  "recursive": (boolean),                <-- [OPTIONAL] whether to search folders inside the 'in' folder for more spritesheets, default false.
  "singleFolderOutput": (boolean),       <-- [OPTIONAL] whether to place all sprites in a single folder, or to generate folders for each spritesheet in the output directory. Default true.
  "subtractAlphaFromIndex": (boolean),   <-- [OPTIONAL] whether to map sprite sheet position to file name 1:1, or to generate a continuous range of file name numbers by ignoring alpha sprites. Alpha sprites will not be generated as file either way: only the file name is affected. Default false.
  "groundFilePattern": "/JS Regex/",     <-- [OPTIONAL] Any file which matches this regex pattern will be treated as a ground spritesheet instead of object spritesheet. Ground sprites are generated with a ring of alpha pixels as requried by the FrontEnd. The syntax is as seen in JavaScript. Helpful site: regexr.com. Default '/ground/i'; Any file with 'ground' in it will match, case insensitive.
  "groundIndexOffset": (number),         <-- [OPTIONAL] offset to apply to the numerical file name of Ground sprites. When singleFolderOutput is enabled, an offset is recommended, because otherwise an object & ground sheet could overwrite by file name, both being named '0.png' and so on. Default is '1000' or '0', depending on whether 'singleFolderOutput' is enabled.
}
