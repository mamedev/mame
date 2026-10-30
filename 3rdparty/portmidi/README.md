# PortMidi - Cross-Platform MIDI IO

This is the canonical release of PortMidi.

See other repositories within [PortMidi](https://github.com/PortMidi)
for related code and bindings (although currently, not much is here).

## [Full C API documentation is here.](https://portmidi.github.io/portmidi_docs/)

## Compiling and Using PortMidi


Use CMake (or ccmake) to create a Makefile for Linux/BSD or a
project file for Xcode or MS Visual Studio. Use make or an IDE to compile:

On Linux, you need ALSA.

Ubuntu / Debian:
`sudo apt install libasound2-dev`

Fedora / Red Hat / CentOS:
`sudo dnf install alsa-lib-devel`

All:
```
cd portmidi  # start in the top-level portmidi directory
ccmake .     # set any options interactively, type c to configure
             # type g to generate a Makefile or IDE project
             # type q to quit
             # (alternatively, run the CMake GUI and use
             #  Configure and Generate buttons to build IDE project)
make         # compile sources and build PortMidi library
             # (alternatively, open project file with your IDE)
sudo make install  # if you want to install to your system
```

## Installation

My advice is to build PortMidi and statically link it to your
application. This gives you more control over versions. However,
installing PortMidi to your system is preferred by some, and the
following should do it:
```
cmake .
make
sudo make install
```

## Language Bindings

Here is a guide to some other projects using PortMidi. There is not
much coordination, so let us know if there are better or alternative
bindings for these and other languages:

- Python: Various libraries and packages exist; search and ye shall
  find. If you wouldn't like to do research, check out [mido](https://mido.readthedocs.io/en/stable/)
- [SML](https://github.com/jh-midi/portmidi-sml2)
- [OCaml](https://ocaml.org/p/portmidi/0.1)
- [Haskell](https://hackage.haskell.org/package/PortMidi)
- [Erlang](https://hexdocs.pm/portmidi/PortMidi.html)
- [Julia](https://github.com/SteffenPL/PortMidi.jl)
- [C#](https://github.com/net-core-audio/portmidi)
- [Rust](https://musitdev.github.io/portmidi-rs/)
- [Go](https://github.com/rakyll/portmidi)
- [Odin](https://pkg.odin-lang.org/vendor/portmidi/)
- [Serpent](https://sourceforge.net/projects/serpent/) - a real-time
  Python-like language has PortMidi built-in, a MIDI-timestamp-aware
  scheduler, and GUI support for device selection.
- [Pd (Pure Data)](https://puredata.info/) uses PortMidi.


## What's New? 

(Not so new, but significant:) Support for the **PmDefaults** program,
which enabled a graphical interface to select default MIDI devices,
has been removed for lack of interest. This allowed us to also remove
C code to read and parse Java preference files on various systems,
simplifying the library. **PmDefaults** allowed simple command-line
programs to use `Pm_DefaultInputDeviceID()` and
`Pm_DefaultOutputDeviceID()` rather than creating device selection
interfaces. Now, you should either pass devices on the command line or
create your own selection interface when building a GUI
application. (See pm_tests for examples.)  `Pm_DefaultInputDeviceID()`
and `Pm_DefaultOutputDeviceID()` now return a valid device if
possible, but they may not actually reflect any user preference.

Haiku support in a minimal implementation. See TODO's in source.

sndio is also minimally supported, allowing basic PortMidi functions
in OpenBSD, FreeBSD, and NetBSD by setting USE_SNDIO for CMake, but
not delayed/timestamped output and virtual devices.

See CHANGELOG.txt for more details.

# Other Repositories

PortMidi used to be part of the PortMedia suite, but this repo has
been reduced to mostly just C/C++ code for PortMidi. You will find
some other repositories in this PortMidi project set up for language
bindings (volunteers and contributors are invited!). Other code
removed from previous releases of PortMedia include:

## PortSMF

A Standard MIDI File (SMF) (and more) library is in the [portsmf
repository](https://github.com/rbdannenberg/portsmf).

PortSMF is a library for reading/writing/editing Standard MIDI
Files. It is actually much more, with a general representation of
events and updates with properties consisting of attributes and typed
values. Familiar properties of pitch, time, duration, and channel are
built into events and updates to make them faster to access and more
compact.

To my knowledge, PortSMF has the most complete and useful handling of
MIDI tempo tracks. E.g., you can edit notes according to either beat
or time, and you can edit tempo tracks, for example, flattening the
tempo while preserving the beat alignment, preserving the real time
while changing the tempo or stretching the tempo over some interval.

In addition to Standard MIDI Files, PortSMF supports an ASCII
representation called Allegro. PortSMF and Allegro are used for
Audacity Note Tracks.

## scorealign

Scorealign used to be part of the PortMedia suite. It is now at the
[scorealign repository](https://github.com/rbdannenberg/scorealign).

Scorealign aligns audio-to-audio, audio-to-MIDI or MIDI-to-MIDI using
dynamic time warping (DTW) of a computed chromagram
representation. There are some added smoothing tricks to improve
performance. This library is written in C and runs substantially
faster than most other implementations, especially those written in
MATLAB, due to the core DTW algorithm. Users should be warned that
while chromagrams are robust features for alignment, they achieve
robustness by operating at fairly high granularity, e.g., durations of
around 100ms, which limits time precision. Other more recent
algorithms can doubtless do better, but be cautious of claims, since
it all depends on what assumptions you can make about the music.
