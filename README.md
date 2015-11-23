# fuse-libretro

**fuse-libretro** is an *work in progress* port of the [Fuse Unix Spectrum Emulator](http://fuse-emulator.sourceforge.net/) to [libretro](http://www.libretro.com/). It's being developed on Windows with MinGW (32 bits) and tested on RetroArch 1.0.0.3-beta frontend, but it's known to compile and run on
Linux x86/x64 and ARM. **fuse-libretro** has been tested with some [Lakka](http://www.lakka.tv/) builds.

## Games

There are hundreds of free, legally available ZX Spectrum games at [World of Spectrum](http://www.worldofspectrum.org/). You should start at the [Visitor Voted Top 100 Best Games](http://www.worldofspectrum.org/bestgames.html).

## Emulated Machines

1. Spectrum 48K
1. Spectrum 48K (NTSC)
1. Spectrum 128K
1. Spectrum +2
1. Spectrum +2A
1. Spectrum +3
1. Spectrum +3e
1. Spectrum SE
1. Timex TC2048
1. Timex TC2068
1. Timex TS2068
1. Spectrum 16K
1. Pentagon 128K (needs `128p-0.rom`, `128p-1.rom`, and `trdos.rom`)
1. Pentagon 512K (needs `128p-0.rom`, `128p-1.rom`, `gluck.rom`, and `trdos.rom`)
1. Pentagon 1024 (needs `128p-0.rom`, `128p-1.rom`, `gluck.rom`, and `trdos.rom`)
1. Scorpion 256K (needs `256s-0.rom`, `256s-1.rom`, `256s-2.rom`, and `256s-3.rom`)

The last four machines need additional ROMs to work. Create a `fuse` folder inside the `<system>` folder, download the required ROMs and put them into that folder.

The port correctly loads and runs ~~some~~ many games I have around. Most of them are 48K tapes, but I've successfully loaded one RZX file, one SCL file, and one TRD file.

## Core Options

The core options available on the frontend are:

* Model (Spectrum 16K|Spectrum 48K|Spectrum 48K (NTSC)|Spectrum 128K|Spectrum +2|Spectrum +2A|Spectrum +3|Spectrum +3e|Spectrum SE|Timex TC2048|Timex TC2068|Timex TS2068|Spectrum 16K|Pentagon 128K|Pentagon 512K|Pentagon 1024|Scorpion 256K): Set the machine to emulate. Note that the this setting will have effect only when a new content is loaded
* Hide video border (enabled|disabled): Hides the video border, making the game occupy the entire screen area
* Tape Fast Load (enabled|disabled): Instantly loads tape files if enabled, or disabled it to see the moving horizontal lines in the video border while the game loads
* Tape Load Sound (enabled|disabled): Outputs the tape sound if fast load is disabled
* Speaker Type (tv speaker|beeper|unfiltered): Applies an audio filter (libretro should allow for audio filters on the frontend)
* AY Stereo Separation (none|acb|abc): The AY sound chip stereo separation (whatever it is)
* Transparent Keyboard Overlay (enabled|disabled): If the keyboard overlay is transparent or opaque
* Time to Release Key in ms (100|300|500|1000): How much time to keep a key pressed before releasing it (used when a key is pressed using the keyboard overlay)

## Input Devices

There are seven types of joysticks emulated:

1. Cursor
1. Kempston
1. Sinclair 1
1. Sinclair 2
1. Timex 1
1. Timex 2
1. Fuller Joystick

Users can configure their joystick types in the input configuration on the front end. However, **fuse-libretro** allows for two joysticks at maximum so only users one and two can actually use theirs in the emulation.

Users 1 and 2 can choose any of the joysticks as their device types, user 3 can only choose the Sinclair Keyboard.

Buttons A, X and Y are mapped to the joystick's fire button, and button B is mapped to the UP directional button. Buttons L1 and R1 are mapped to RETURN and SPACE, respectively. The SELECT button brings up the embedded, on-screen keyboard which is useful if you only have controllers attached to your box.

There are some conflicts in the way the input devices interact because of the use of the physical keyboard keys as joystick buttons. For a good gaming experience, set the user device types as follows:

* For joystick games: Set user 1 to a joystick type. Optionally, set user 2 to another joystick type (local cooperative games). Set user 3 to none. This way, you can use L1 as RETURN, R1 as SPACE, and SELECT to bring the embedded keyboard.
* For keyboard games: Set users 1 and 2 to none, and user 3 to Sinclair Keyboard. You won't have any joystick and the embedded keyboard won't work, but the entire physical keyboard will be available for you to type in those text adventure commands.

If you set a joystick along with the keyboard, the joystick will work just fine except for the bindings to RETURN and SPACE, and the keyboard won't register the keys assigned to the Cursor joystick, or to the L1 and R1 buttons for all other joystick types.

## Supported Formats

Fuse can load a number of different file formats. For now, **fuse-libretro** only loads `tzx`, `tap`, `z80`, `rzx`, `scl` and `trd` files. This decision is somewhat arbitrary (it depends if I can find a file that I can be sure is not corrupted), so feel free to bug me to add other extensions. Please do so via issues here on GitHub.

## Save States

Supported.

## Setup

1. Compile **fuse-libretro** with `make -f Makefile.libretro`
1. Copy the resulting `fuse_libretro.dll` or `fuse_libretro.so` into the `cores` folder of your libretro frontend
1. Profit!

> It's *not* necessary to copy files to the `system` folder of your libretro frontend anymore! All supporting files are baked into the core, except ROMs for the more exotic Spectrum clones (see Emulated Machines.)

## Versions

Versions that are being used to build and test **fuse-libretro**:

* Fuse 1.1.1
* libspectrum 1.1.1
* zlib 1.2.8
* bzip2 1.0.6

## TODO

See the open issues.

## Bugs

Many, probably. Open issues or send pull requests.

## License

GNU GPLv3.0
