README_LINUX.txt for PortMidi
Roger Dannenberg
6 Dec 2012, revised May 2022

Contents:
        To make PortMidi
        The pmdefaults program
        Setting LD_LIBRARY_PATH
        A note about amd64
        Using autoconf
        Using configure
        Changelog


See ../README.md for general instructions.

THE pmdefaults PROGRAM

(This may be obsolete. It is older than `../README.md` which
also discusses pmdefaults, and Java support may be removed
unless someone claims they use it... -RBD)

You should install pmdefaults. It provides a graphical interface
for selecting default MIDI IN and OUT devices so that you don't
have to build device selection interfaces into all your programs
and so users have a single place to set a preference.

Follow the instructions above to run ccmake, making sure that
CMAKE_BUILD_TYPE is Release. Run make as described above. Then:

sudo make install

This will install PortMidi libraries and the pmdefault program.
You must alos have the environment variable LD_LIBRARY_PATH set
to include /usr/local/lib (where libpmjni.so is installed).

Now, you can run pmdefault.


SETTING LD_LIBRARY_PATH

pmdefaults will not work unless LD_LIBRARY_PATH includes a 
directory (normally /usr/local/lib) containing libpmjni.so,
installed as described above.

To set LD_LIBRARY_PATH, you might want to add this to your
~/.profile (if you use the bash shell):

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
export LD_LIBRARY_PATH


A NOTE ABOUT AMD64:

When compiling portmidi under linux on an AMD64, I had to add the -fPIC 
flag to the gcc flags.

Reason: when trying to build John Harrison's pyPortMidi gcc bailed out
with this error:

./linux/libportmidi.a(pmlinux.o): relocation R_X86_64_32 against `a local symbol' can not be used when making a shared object; recompile with -fPIC
./linux/libportmidi.a: could not read symbols: Bad value
collect2: ld returned 1 exit status
error: command 'gcc' failed with exit status 1

What they said:
http://www.gentoo.org/proj/en/base/amd64/howtos/index.xml?part=1&chap=3
On certain architectures (AMD64 amongst them), shared libraries *must* 
be "PIC-enabled".

CHANGELOG

27-may-2022 Roger B. Dannenberg
   Some updates to this file.

6-dec-2012 Roger B. Dannenberg
   Copied notes on Autoconf from Audacity sources

22-jan-2010 Roger B. Dannenberg
   Updated instructions about Java paths

14-oct-2009 Roger B. Dannenberg
   Using CMake now for building and configuration

29-aug-2006 Roger B. Dannenberg
   Fixed PortTime to join with time thread for clean exit.    

28-aug-2006 Roger B. Dannenberg
    Updated this documentation.
 
08-Jun-2004 Roger B. Dannenberg
      Updated code to use new system abstraction.

12-Apr-2003 Roger B. Dannenberg
      Fixed pm_test/test.c to filter clocks and active messages.
      Integrated changes from Clemens Ladisch:
          cleaned up pmlinuxalsa.c
          record timestamp on sysex input
          deallocate some resources previously left open
