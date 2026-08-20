# regnecentralen

**CURRENT STATE: work in progress taking of from where it was left.**

This folder contains source code and documentation related to emulation of some of the computers and hardware produced by Regnecentralen, a Danish computer manufacturer.

The RC702 is a 8-bit Z80 machine with either 8" external floppy drives or built-in 5,25" 360 Kb floppy drives, 64 Kb RAM, an optional 10 Mb Winchester hard disk and an external keyboard connected with the parallel port.  There is a 2 Kb boot prom ("autoload") which is presented as the bios in MAME.  The successor RC703 had high density 5,25" 1.2 Mb diskettes, a larger bios and slightly changed handling of it.  MAME emulates both RC702 and RC703.

The RC750 Partner/RC759 Piccoline was their 16-bit machine based on 80186, with the Partner targetting businesses and the Piccoline targetting the Danish school system.  It ran the technically superior CCP/M-86 system which allowed for 4 virtual consoles, but wasn't 100% compatible with the IBM PC.

## RC702: Keyboard

The RC702 keyboard is connected to Z80 PIO port A.  The driver wires the generic keyboard and (optionally) MAME’s natural keyboard into the PIO so that CP/M sees keypresses.

- **Emulated keyboard**: US-ASCII layout with typematic repeat (configurable delay and rate via MAME's Machine Configuration menu).
- **Natural keyboard**: In the MAME menu, choose **Keyboard Selection** → **Keyboard Mode** → **Natural** to use the host OS layout (e.g. Danish: Shift+2 → `"`, Æ/Ø keys correct).  Characters are sent as Latin-1 (0–255).

## Source Files

- [`rc702.cpp`](rc702.cpp): Implements the driver and emulation logic for the Regnecentralen RC702 Piccolo, including Z80 CPU, memory mapping, PIO keyboard path, and device support.
- [`rc759.cpp`](rc759.cpp): Contains the driver for the Regnecentralen RC759 Piccoline, handling system emulation, peripherals, and video output.
- [`rc759_kbd.h`](rc759_kbd.h):
- [`rc759_kbd.cpp`](rc759_kbd.cpp): Emulation of the keyboard scanning input

## RC702: Getting started

For now, copy '*.rom' from a clone of https://github.com/ringgaard/rc700 to `mame/roms/rc702` and copy `cp roa375.rom roa375.ic66`.

You may also want to just run this script instead on MacOS:

```sh
OUTPUT_DIR=../../../roms/rc702
mkdir -p $OUTPUT_DIR
curl --output-dir $OUTPUT_DIR -L -O https://github.com/ravn/rc700/raw/refs/heads/master/roa296.rom
curl --output-dir $OUTPUT_DIR -L -O https://github.com/ravn/rc700/raw/refs/heads/master/roa327.rom
curl --output-dir $OUTPUT_DIR -L -O https://github.com/ravn/rc700/raw/refs/heads/master/rob357.rom
curl --output-dir $OUTPUT_DIR -L -O https://github.com/ravn/rc700/raw/refs/heads/master/rob358.rom
curl --output-dir $OUTPUT_DIR -L -o roa375.ic66 https://github.com/ravn/rc700/raw/refs/heads/master/roa375.rom
echo "*** All ROMS should be 2048 bytes ***"
ls -l $OUTPUT_DIR
```


Note that the IMD images in this project are not compatible with MAME due to a different sector offset.

Now build MAME using something like (-j10 requires a modern machine):

```sh
make SUBTARGET=regnecentralen DEBUG=1 SOURCES=src/mame/regnecentralen/rc702.cpp,src/mame/regnecentralen/pio_port/pio_port.cpp,src/mame/regnecentralen/pio_port/keyboard.cpp,src/mame/regnecentralen/pio_port/cpnet_bridge.cpp TOOLS=1 SYMLEVEL=3  SYMBOLS=1  OSD=sdl -j 10
```

`SOURCES` is comma-separated (no spaces). Since upstream #15805 the PIO-port
slot lives in the driver folder (`pio_port/`), so its source files must be
listed explicitly alongside `rc702.cpp` — `cpnet_bridge.cpp` is the fork-only
CP/NET host-socket card. `OSD=sdl` builds against SDL2 (the macOS default is
now `sdl3`, which is not installed here).

and run it similar to:

```sh
./regnecentralend rc702mini -bios 0 -window -skip_gameinfo -flop1 ~/Downloads/CPM_med_COMAL80.imd
./regnecentralend rc702 -bios 0 -window -skip_gameinfo -flop1 ~/Downloads/SW1711-I8.imd   # 8" maxi
./regnecentralend rc703 -bios 1 -window -skip_gameinfo -flop1 ~/Downloads/RC703_CPM_v2.2_r1.2.imd
```

You should get a yellow screen saying either "** NO PROGRAM OR LINEPROG" which is the ROM saying it cannot find a boot sector on the floppy, or "** BAD DISKETTE" which mean that the sanity check on the diskette read failed.   This is most likely because the disk drive emulated is not compatible with the image.

## RC759 - Getting started

See https://rc700.dk/emulator.php for details about previous emulator work.

Get the necessary ROMs:

```sh
OUTPUT_DIR=../../../roms/rc759
mkdir -p $OUTPUT_DIR
curl --output-dir $OUTPUT_DIR -L -O http://www.hampa.ch/pce/rom/rc759/rc759-1-2.1.rom
curl --output-dir $OUTPUT_DIR -L -O http://www.hampa.ch/pce/rom/rc759/rc759-1-5.1.rom
curl --output-dir $OUTPUT_DIR -L -O http://www.hampa.ch/pce/rom/rc759/rc759-2-4.0.rom
curl --output-dir $OUTPUT_DIR -L -O http://www.hampa.ch/pce/rom/rc759/rc759-2-5.1.rom
echo "*** All ROMS should be 32768 bytes ***"
ls -l $OUTPUT_DIR
```

Now build MAME using something like (-j10 requires a modern machine):

```sh
make SUBTARGET=regnecentralen DEBUG=1 SOURCES="src/mame/regnecentralen/rc759.cpp,src/mame/regnecentralen/rc750.cpp" TOOLS=1 SYMLEVEL=3  SYMBOLS=1  OSD=sdl -j 10
```

(add `REGENIE=1` the first time after adding/removing a source file in this
folder, so the generated project picks up the new files).

and run it similar to:

```sh
./regnecentralend rc759 -window
```

## RC750 Partner

The RC750 Partner is the sibling of the RC759 Piccoline. Both are driven
from the shared base class `rc75x_state` (`rc75x.h` / `rc75x.cpp`): the
common Intel 80186 + 8259A + 8255 + 82730 + MM58167 + NVM + SN76489A +
keyboard core lives in the base, and each model's `.cpp` adds only its own
floppy/serial/expansion side. `rc750.cpp` is NOT a subclass of `rc759.cpp`
- both derive independently from `rc75x_state`.

Partner-specific hardware (from the PARTNER Programmer's Guide v3, jun 1986,
saved in `rc700-gensmedet/docs/`): WD1797 FDC (modelled as `FD1797`), an
Intel 8274 dual serial controller, a SCSI host adapter and an optional 8087,
instead of the Piccoline's cassette / iSBX slot. It runs Concurrent DOS.

The driver is marked `MACHINE_NOT_WORKING`: no Partner boot ROM dump is
available yet (none on hampa.ch/pce or rc700.dk), and Appendix B of the
guide - the I/O port map - is an OCR-blank scanned figure, so the
Partner-specific WD1797/8274/SCSI port addresses in the driver are
provisional placeholders. The shared core is verified against the working
RC759. Once a ROM dump and the real port map surface, drop `rc750.rom`
into `roms/rc750/` and revisit `rc750_io()`.

## References:

* Variuos materials: https://ddhf.dk/wiki/RC700_Piccolo
* Technical manual, not searchable:  https://ddhf.dk/w/images/5/5b/RC702_Tech_Man.pdf

