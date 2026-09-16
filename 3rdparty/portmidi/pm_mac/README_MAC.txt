README_MAC.txt for PortMidi
Roger Dannenberg
20 nov 2009

revised Mar 2024 to remove pmdefaults references
revised Jan 2022 for the PortMidi/portmidi repo on github.com  
revised 20 Sep 2010 for Xcode 4.3.2 and CMake 2.8.8

This documents how I build PortMidi for macOS. It's not the only way,
and command-line/scripting enthusiasts will say it's not even a good
way. Feel free to contribute your approach if you are willing to
describe it carefully and test it.

Install Xcode and the CMake application, CMake.app. I use the GUI
version of CMake which makes it easy to see/edit variables and
options.

==== USING CMAKE ====

Run CMake.app and select your portmidi repo working directory as the
location for source and build. (Yes, I use so called "in-tree"
builds -- it doesn't hurt, but I don't think it is necessary.)

Default settings should all be fine, but select options under BUILD if
you wish:

BUILD_NATIVE_JAVA_INTERFACE to build a Java interface (JNI) library.

BUILD_PORTMIDI_TESTS to create some test programs. Of particular
interest are test/mm, a handy command-line MIDI Input Monitor, and
test/testio, a simple command-line program to send or receive some
MIDI notes in case you need a quick test: What devices do I have? Does
this input work? Does this output work?

I disable BUILD_SHARED_LIBS and always link statically: Static linking only
adds about 40KB to any application and then you don't have to worry
about versions, instally, copying or finding the dynamic link library,
etc.

To make sure you link statically, I rename the library to
libportmidi_static.a. To do this, set PM_STATIC_LIB_NAME (in CMake,
under the "PM" group) to "portmidi_static", and of course your
application will have to specify portmidi_static as the library to
link to.

If you are building simple command-line applications, you might want
to enable PM_CHECK_ERRORS. If you do, then calls into the PortMidi
library will print error messages and exit in the event of an error
(such as trying to open a device that does not exist). This saves you
from having to check for errors everytime you call a library function
or getting confused when errors are detected but not reported. For
high-quality applications, do NOT enable PM_CHECK_ERRORS -- any
failure could immediately abort your whole application, which is not
very friendly to users.

Click on Configure (maybe a couple of times).

Click on Generate and make an Xcode project.

Open portmidi/portmidi.xcodeproj with Xcode and build what you
need. The simplest thing is to build the ALL_BUILD target. Be careful
to specify a Debug or Release depending on what you want. "ALL_BUILD"
is a misnomer -- it only builds the version you select.


