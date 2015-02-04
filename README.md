# fuse-libretro

**fuse-libretro** is an *work in progress* port of the [Fuse Unix Spectrum Emulator](http://fuse-emulator.sourceforge.net/) to [libretro](http://www.libretro.com/). It's being developed on Windows with MinGW (32 bits) and tested on RetroArch 1.0.0.3-beta frontend, but it's known to compile and run at least on Linux 64-bits.

The only emulated machine for now is the ZX Spectrum 48K. The port correctly loads and runs some games I have around.

## Core Options

The core options available on the frontend are:

* Tape Fast Load (enabled|disabled): Instantly loads tape files if enabled, or disabled it to see the moving horizontal lines in the video border while the game loads
* Tape Load Sound (enabled|disabled): Outputs the tape sound if fast load is disabled
* Speaker Type (tv speaker|beeper|unfiltered): Applies an audio filter (libretro should allow for audio filters on the frontend)
* AY Stereo Separation (none|acb|abc): The AY sound chip stereo separation (whatever it is)
* Transparent Keyboard Overlay (enabled|disabled): If the keyboard overlay is transparent or opaque
* Time to Release Key in ms (100|300|500|1000): How much time to keep a key pressed before releasing it (used when a key is pressed using the keyboard overlay)

## Controllers

There are seven types of joysticks emulated:

1. Cursor
2. Kempston
3. Sinclair 1
4. Sinclair 2
5. Timex 1
6. Timex 2
7. Fuller Joystick

Fuse allows for two joysticks at maximum so only users one and two can choose their joystick types via the input configuration on the frontend. Actually all users can choose their joystick types, but only users one and two can use theirs in the emulation.

## Keyboard

Keyboard is **not** being emulated right now, but it's on my TODO list. If you need to press keys in a game (i.e. to choose the joystick type) use the keyboard overlay, which is displayed by pressing *select* on the controller. Don't play games where you have to write a lot yet, nor use this core to write a program for the ZX Spectrum.

## Supported Formats

Fuse can load a number of different file formats. For now, **fuse-libretro** only loads `tzx`, `tap`, and `z80` files. This decision is arbitrary, so feel free to bug me to add other extensions. Please do so via issues here on GitHub.

## Save States

Supported.

## Setup

1. Compile **fuse-libretro** with `make -f Makefile.libretro`
1. Copy the resulting `fuse_libretro.dll` or `fuse_libretro.so` into the `cores` folder of your libretro frontend
1. Profit!

> It's *not* necessary to copy files to the `system` folder of your libretro frontend anymore!

## Versions

Versions that are being used to build and test **fuse-libretro**:

* Fuse 1.1.1
* libspectrum 1.1.1
* zlib 1.2.8
* bzip2 1.0.6

## TODO

See the open issues.

## Bugs

Many. Open issues or send pull requests.

## License

GNU GPLv3.0
