# PortMidi - Cross-Platform MIDI IO

This is the canonical release of PortMidi.

See other repositories within [PortMidi](https://github.com/PortMidi)
for related code and bindings (although currently, not much is here).

## [Full C API documentation is here.](https://portmidi.github.io/portmidi_docs/)

## Compiling and Using PortMidi

Use CMake (or ccmake) to create a Makefile for Linux/BSD or a 
project file for Xcode or MS Visual Studio. Use make or an IDE to compile:
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

**PmDefaults** is a Java-based program for setting default MIDI
devices. It is necessary if you use `Pm_DefaultInputDeviceID()` or
`Pm_DefaultOutputDeviceID()` to avoid implementing your own device
browsing, selection and preferences in your applications. Enable
`BUILD_PMDEFAULTS` and `BUILD_JAVA_NATIVE_INTERFACE` in ccmake, and
see `pm_java/README.txt` for more information.

See also notes in `pm_mac/README_MAC.txt`, `pm_win/README_WIN.txt` and
`pm_linux/README_LINUX.txt`.

## What's New?

PortMidi has some fixes for Apple M1 cpus as of May 23, 2022. This has not yet
been formally released to allow for further testing, but please use the latest
code if you want to run on an M1.

PortMidi has some changes in 2021:

 - added Pm_CreateVirtualInput() and Pm_CreateVirtualOutput() functions that allow
   applications to create named ports analogous to devices.
   
 - improvements for macOS CoreMIDI include higher data rates for devices, better
   handling of Unicode interface names in addition to virtual device creation.
   
 - the notion of default devices, Pm_GetDefaultInputDeviceID(), 
   Pm_GetDefaultOutputDeviceID and the PmDefaults program have fallen into disuse
   and are now deprecated.
   
 - Native Interfaces for Python, Java, Go, Rust, Lua and more seem best left
   to individual repos, so support within this repo has been dropped. A Java
   interface is still here and probably usable -- let me know if you need it
   and/or would like to help bring it up to date. I am happy to help with, 
   link to, or collaborate in supporting PortMidi for other languages. 
   
For up-to-date PortMidi for languages other than C/C++, check with
developers. As of 27 Sep 2021, this (and SourceForge) is the only repo with
the features described above.

# Other Repositories

PortMidi used to be part of the PortMedia suite, but this repo has been reduced to
mostly just C/C++ code for PortMidi. You will find some other repositories in this PortMidi project
set up for language bindings (volunteers and contributors are invited!). Other code removed from
previous releases of PortMedia include:

## PortSMF

A Standard MIDI File (SMF) (and more) library is in the [portsmf repository](https://github.com/PortMidi/portsmf).

PortSMF is a library for reading/writing/editing Standard MIDI Files. It is
actually much more, with a general representation of events and updates with
properties consisting of attributes and typed values. Familiar properties of
pitch, time, duration, and channel are built into events and updates to make
them faster to access and more compact.

To my knowledge, PortSMF has the most complete and useful handling of MIDI
tempo tracks. E.g., you can edit notes according to either beat or time, and
you can edit tempo tracks, for example, flattening the tempo while preserving
the beat alignment, preserving the real time while changing the tempo or 
stretching the tempo over some interval.

In addition to Standard MIDI Files, PortSMF supports an ASCII representation
called Allegro. PortSMF and Allegro are used for Audacity Note Tracks.

## scorealign

Scorealign used to be part of the PortMedia suite. It is now at the [scorealign repository](https://github.com/rbdannenberg/scorealign).

Scorealign aligns
audio-to-audio, audio-to-MIDI or MIDI-to-MIDI using dynamic time warping (DTW)
of a computed chromagram representation. There are some added smoothing tricks
to improve performance. This library is written in C and runs substantially 
faster than most other implementations, especially those written in MATLAB,
due to the core DTW algorithm. Users should be warned that while chromagrams
are robust features for alignment, they achieve robustness by operating at 
fairly high granularity, e.g., durations of around 100ms, which limits 
time precision. Other more recent algorithms can doubtless do better, but
be cautious of claims, since it all depends on what assumptions you can 
make about the music.
