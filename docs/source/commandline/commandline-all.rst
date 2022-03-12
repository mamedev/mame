.. _mame-commandline-universal:

Universal Commandline Options
=============================

This section contains configuration options that are applicable to *all* MAME
sub-builds (both SDL and Windows native).

.. contents:: :local:


Commands and Verbs
------------------

Commands include **mame** itself as well as various tools included with the MAME
distribution such as **romcmp** and **srcclean**.

Verbs are actions to take upon something with the command (e.g.
**mame -validate pacman** has *mame* as a command and *-validate* as a verb)


.. _mame-commandline-patterns:

Patterns
--------

Many verbs support the use of *patterns*, which are either a system or device
short name (e.g. **a2600**, **zorba_kbd**) or a glob pattern that matches either
(e.g. **zorba_\***).

Depending on the command you're using the pattern with, pattern matching may
match systems or systems and devices.  It is advised to put quotes around your
patterns to avoid having your shell try to expand them against filenames (e.g.
**mame -validate "pac\*"**).


.. _mame-commandline-paths:

File Names and Directory Paths
------------------------------

A number of options for specifying directories support multiple paths (for
for example to search for ROMs in multiple locations).  MAME expects multiple
paths to be separated with semicolons ( ``;`` ).

MAME expands environment variable expressions in paths.  The syntax used depends
on your operating system.  On Windows, ``%`` (percent) syntax is used.  For
example ``%APPDATA%\mame\cfg`` will expand the application data path for the
current user's roaming profile.  On UNIX-like system (including macOS and
Linux), Bourne shell syntax is used, and a leading ``~`` expands to the current
user's home directory.  For example, ``~/.mame/${HOSTNAME}/cfg`` expands to
a host-specific path inside the ``.mame`` directory in the current user's home
directory.  Note that only simple variable substitutions are supported; more
complex expressions supported by Bash, ksh or zsh are not recognized by MAME.

Relative paths are resolved relative to the current working directory.  If you
start MAME by double-clicking it in Windows Explorer, the working directory is
set to the folder containing the MAME executable.  If you start MAME by
double-clicking it in the macOS Finder, it will open a Terminal window with the
working directory is set to your home directory (usually ``/Users/<username>`` )
and start MAME.

If you want behaviour similar to what Windows Explorer provides on macOS, create
a script file containing these lines in the directory containing the MAME
executable (for example you could call it ``mame-here`` )::

    #!/bin/sh
    cd "`dirname "$0"`"
    exec ./mame

You should be able to use any text editor.  If you have a choice of file format
or line ending style, choose UNIX. This assumes you're using a 64-bit release
build of MAME, but if you aren't you just need to change ``mame`` to the name
of your MAME executable (e.g. mamed, mamep, mamedp).  Once you've created the
file, you need to mark it as executable.  You can do this by opening a Terminal
window, typing **chmod a+x** followed by a space, dragging the file you created
onto the window (this causes Terminal to insert the full escaped path to the
file), and then ensuring the Terminal window is active and hitting **Return**
(or **Enter**) on your keyboard.  You can close the Terminal window after doing
this.  Now if you double-click the script in the Finder, it will open a
Terminal window, set the working directory to the location of the script
(i.e. the folder containing MAME), and then start MAME.


.. _mame-commandline-coreverbs:

Core Verbs
----------

.. Tip:: Examples that have the output abbreviated for space reasons will show
         "**...**" in the output where needed. For instance:
         .. code-block:: bash

                A
                B
                C
                ...
                Z

.. _mame-commandline-help:

**-help** / **-h** / **-?**

    Displays current MAME version and copyright notice.

    Example:
        .. code-block:: bash

            mame -help

.. _mame-commandline-validate:

**-validate** / **-valid** [*<pattern>*]

    Performs internal validation on one or more drivers and devices in the
    system.  Run this before submitting changes to ensure that you haven't
    violated any of the core system rules.

    If a pattern is specified, it will validate systems matching the pattern,
    otherwise it will validate all systems and devices.  Note that if a pattern
    is specified, it will be matched against systems only (not other devices),
    and no device type validation will be performed.

    Example:
        .. code-block:: bash

            mame -validate
            Driver ace100 (file apple2.cpp): 1 errors, 0 warnings
            Errors:
            Software List device 'flop525_orig': apple2_flop_orig.xml: Errors parsing software list:
            apple2_flop_orig.xml(126.2): Unknown tag: year
            apple2_flop_orig.xml(126.8): Unexpected content
            apple2_flop_orig.xml(127.2): Unknown tag: publisher
            apple2_flop_orig.xml(127.13): Unexpected content
            apple2_flop_orig.xml(128.2): Unknown tag: info
            apple2_flop_orig.xml(129.2): Unknown tag: sharedfeat
            apple2_flop_orig.xml(132.2): Unknown tag: part
            apple2_flop_orig.xml(133.3): Tag dataarea found outside of software context
            apple2_flop_orig.xml(134.4): Tag rom found outside of part context
            apple2_flop_orig.xml(137.3): mismatched tag



.. _mame-commandline-configverbs:

Configuration Verbs
-------------------

.. _mame-commandline-createconfig:

**-createconfig** / **-cc**

    Creates the default ``mame.ini`` file.  All the configuration options (not
    verbs) described below can be permanently changed by editing this
    configuration file.

    Example:
        .. code-block:: bash

            mame -createconfig

.. _mame-commandline-showconfig:

**-showconfig** / **-sc**

    Displays the current configuration settings.  If you route this to a file,
    you can use it as an INI file.

    Example:
        .. code-block:: bash

            mame -showconfig > mame.ini

        This example is equivalent to **-createconfig**.

.. _mame-commandline-showusage:

**-showusage** / **-su**

    Displays a summary of all the command line options.  For options that are
    not mentioned here, the short summary given by "mame -showusage" is usually
    a sufficient description.


.. _mame-commandline-fronendverbs:

Frontend Verbs
--------------

Note: By default, all the '**-list**' verbs below write info to the standard
output (usually the terminal/command window where you typed the command).  If
you wish to write the info to a text file instead, add this to the end of your
command:

    **>** *filename*

where *filename* is the name of the file to save the output in (e.g.
``list.txt`` ).  Note that if this file already exists, it will be completely
overwritten.

    Example:
        .. code-block:: bash

            mame -listcrc puckman > list.txt

    This creates (or overwrites the existing file if already there) ``list.txt``
    and fills the file with the results of **-listcrc puckman**.  In other
    words, the list of each ROM used in Puckman and the CRC for that ROM are
    written into that file.

.. _mame-commandline-listxml:

**-listxml** / **-lx** [*<pattern>*...]

    List comprehensive details for all of the supported systems and devices in
    XML format.  The output is quite long, so it is usually better to redirect
    this into a file.  By default all systems are listed; however, you can limit
    this list by specifying one or more *patterns* after the **-listxml** verb.

    This XML output is typically imported into other tools (like graphical
    front-ends and ROM managers), or processed with scripts query detailed
    information.

    Example:
        .. code-block:: bash

            mame galaxian -listxml
            <?xml version="1.0"?>
            <!DOCTYPE mame [
            <!ELEMENT mame (machine+)>
	            <!ATTLIST mame build CDATA #IMPLIED>
	            <!ATTLIST mame debug (yes|no) "no">
	            <!ATTLIST mame mameconfig CDATA #REQUIRED>
	            <!ELEMENT machine (description, year?, manufacturer?, biosset*, rom*, disk*, device_ref*, sample*, chip*, display*, sound?, input?, dipswitch*, configuration*, port*, adjuster*, driver?, feature*, device*, slot*, softwarelist*, ramoption*)>
		            <!ATTLIST machine name CDATA #REQUIRED>
		            <!ATTLIST machine sourcefile CDATA #IMPLIED>
            ...
            <mame build="0.216 (mame0216-154-gabddfb0404c-dirty)" debug="no" mameconfig="10">
            	<machine name="galaxian" sourcefile="galaxian.cpp">
		            <description>Galaxian (Namco set 1)</description>
		            <year>1979</year>
		            <manufacturer>Namco</manufacturer>
                    ...
            	<machine name="z80" sourcefile="src/devices/cpu/z80/z80.cpp" isdevice="yes" runnable="no">
		            <description>Zilog Z80</description>
	        </machine>
            </mame>

.. Tip:: Output from this command is typically more useful if redirected to
         an output file. For instance, doing
         **mame -listxml galaxian > galax.xml** will make ``galax.xml`` or
         overwrite any existing data in the file with the results of
         **-listxml**; this will allow you to view it in a text editor or parse
         it with external tools.

.. _mame-commandline-listfull:

**-listfull** / **-ll** [*<pattern>*...]

    Example:
        .. code-block:: bash

            mame -listfull galaxian*
            Name:             Description:
            galaxian          "Galaxian (Namco set 1)"
            galaxiana         "Galaxian (Namco set 2)"
            galaxianbl        "Galaxian (bootleg, set 2)"
            galaxianbl2       "Galaxian (bootleg, set 4)"
            galaxiani         "Galaxian (Irem)"
            galaxianm         "Galaxian (Midway set 1)"
            galaxianmo        "Galaxian (Midway set 2)"
            galaxiant         "Galaxian (Taito)"
            galaxian_sound    "Galaxian Custom Sound"

    Displays a list of system driver names and descriptions.  By default all
    systems and devices are listed; however, you can limit this list by
    specifying one or more *patterns* after the **-listfull** verb.

.. _mame-commandline-listsource:

**-listsource** / **-ls** [*<pattern>*...]

    Displays a list of system drivers/devices and the names of the source files
    where they are defined.  Useful for finding which driver a system runs on in
    order to fix bugs.  By default all systems and devices are listed; however,
    you can limit this list by specifying one or more *pattern* after the
    **-listsource** verb.

    Example:
        .. code-block:: bash

            mame galaga -listsource
            galaga           galaga.cpp

.. _mame-commandline-listclones:

**-listclones** / **-lc** [*<pattern>*]

    Displays a list of clones.  By default all clones are listed; however, you
    can limit this list by specifying a *pattern* after the **-listsource**
    verb.  If a pattern is specified, MAME will list clones of systems that
    match the pattern, as well as clones that match the pattern themselves.

    Example 1:
        .. code-block:: bash

            mame pacman -listclones
            Name:            Clone of:
            pacman           puckman

    Example 2:
        .. code-block:: bash

            mame puckman -listclones
            Name:            Clone of:
            abscam           puckman
            bucaner          puckman
            crockman         puckman
            crockmnf         puckman
            ...
            puckmod          puckman
            titanpac         puckman

.. _mame-commandline-listbrothers:

**-listbrothers** / **-lb** [*<pattern>*]

    Displays a list of *brothers*, i.e. other systems that are defined in the
    same source file as a system that matches the specified *pattern*.

    Example:
        .. code-block:: bash

            mame galaxian -listbrothers
            Source file:         Name:            Parent:
            galaxian.cpp         amidar
            galaxian.cpp         amidar1          amidar
            galaxian.cpp         amidarb          amidar
            ...
            galaxian.cpp         zigzagb
            galaxian.cpp         zigzagb2         zigzagb

.. _mame-commandline-listcrc:

**-listcrc** [*<pattern>*...]

    Displays a full list of CRCs and names of all ROM images referenced by
    systems and devices matching the specified pattern(s).  If no patterns are
    specified, ROMs referenced by all supported systems and devices will be
    included.

    Example:
        .. code-block:: bash

            mame playch10 -listcrc
            d52fa07a pch1-c__8t_e-2.8t                      playch10                PlayChoice-10 BIOS
            503ee8b1 pck1-c.8t                              playch10                PlayChoice-10 BIOS
            123ffa37 pch1-c_8te.8t                          playch10                PlayChoice-10 BIOS
            0be8ceb4 pck1-c_fix.8t                          playch10                PlayChoice-10 BIOS
            9acffb30 pch1-c__8k.8k                          playch10                PlayChoice-10 BIOS
            c1232eee pch1-c__8m_e-1.8m                      playch10                PlayChoice-10 BIOS
            30c15e23 pch1-c__8p_e-1.8p                      playch10                PlayChoice-10 BIOS
            9acffb30 pch1-c__8k.8k                          playch10                PlayChoice-10 BIOS
            c1232eee pch1-c__8m_e-1.8m                      playch10                PlayChoice-10 BIOS
            30c15e23 pch1-c__8p_e-1.8p                      playch10                PlayChoice-10 BIOS
            9acffb30 pch1-c__8k.8k                          playch10                PlayChoice-10 BIOS
            83ebc7a3 pch1-c_8m.8m                           playch10                PlayChoice-10 BIOS
            90e1b80c pch1-c_8p-8p                           playch10                PlayChoice-10 BIOS
            9acffb30 pch1-c__8k.8k                          playch10                PlayChoice-10 BIOS
            c1232eee pch1-c__8m_e-1.8m                      playch10                PlayChoice-10 BIOS
            30c15e23 pch1-c__8p_e-1.8p                      playch10                PlayChoice-10 BIOS
            e5414ca3 pch1-c-6f.82s129an.6f                  playch10                PlayChoice-10 BIOS
            a2625c6e pch1-c-6e.82s129an.6e                  playch10                PlayChoice-10 BIOS
            1213ebd4 pch1-c-6d.82s129an.6d                  playch10                PlayChoice-10 BIOS
            48de65dc rp2c0x.pal                             playch10                PlayChoice-10 BIOS

.. _mame-commandline-listroms:

**-listroms** / **-lr** [*<pattern>*...]

    Displays a list of ROM images referenced by supported systems/devices that
    match the specified pattern(s). If no patterns are specified, the results
    will include *all* supported systems and devices.

    Example:
        .. code-block:: bash

            mame neogeo -listroms
            ROMs required for driver "neogeo".
            Name                                   Size Checksum
            sp-s2.sp1                            131072 CRC(9036d879) SHA1(4f5ed7105b7128794654ce82b51723e16e389543)
            sp-s.sp1                             131072 CRC(c7f2fa45) SHA1(09576ff20b4d6b365e78e6a5698ea450262697cd)
            sp-45.sp1                            524288 CRC(03cc9f6a) SHA1(cdf1f49e3ff2bac528c21ed28449cf35b7957dc1)
            ...
            sm1.sm1                              131072 CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf)
            000-lo.lo                            131072 CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a)
            sfix.sfix                            131072 CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3)

.. _mame-commandline-listsamples:

**-listsamples** [<*pattern*>]

    Displays a list of samples referenced by the specified pattern of system or
    device names. If no pattern is specified, the results will be *all* systems
    and devices.

    Example:
        .. code-block:: bash

            mame armorap -listsamples
            Samples required for driver "armorap".
            loexp
            jeepfire
            hiexp
            tankfire
            tankeng
            beep
            chopper

.. _mame-commandline-verifyroms:

**-verifyroms** [<*pattern*>]

    Checks for invalid or missing ROM images. By default all drivers that have
    valid ZIP files or directories in the rompath are verified; however, you can
    limit this list by specifying a *pattern* after the **-verifyroms** command.

    Example:
        .. code-block:: bash

            mame gradius -verifyroms
            romset gradius [nemesis] is good
            1 romsets found, 1 were OK.

.. _mame-commandline-verifysamples:

**-verifysamples** [<*pattern*>]

    Checks for invalid or missing samples. By default all drivers that have
    valid ZIP files or directories in the samplepath are verified; however, you
    can limit this list by specifying a *pattern* after the **-verifyroms**
    command.

    Example:
        .. code-block:: bash

            mame armorap -verifysamples
            sampleset armorap [armora] is good
            1 samplesets found, 1 were OK.

.. _mame-commandline-romident:

**-romident** [*path\\to\\romstocheck.zip*]

    Attempts to identify ROM files, if they are known to MAME, in the specified
    .zip file or directory. This command can be used to try and identify ROM
    sets taken from unknown boards. On exit, the errorlevel is returned as one
    of the following:

		* 0: means all files were identified
		* 7: means all files were identified except for 1 or more "non-ROM" files
		* 8: means some files were identified
		* 9: means no files were identified

    Example:
        .. code-block:: bash

            mame unknown.rom -romident
            Identifying unknown.rom....
            unknown.rom         = 456-a07.17l           gradius    Gradius (Japan, ROM version)

.. _mame-commandline-listdevices:

**-listdevices** / **-ld** [<*pattern*>]

    Displays a list of all devices known to be hooked up to a system. The ":" is
    considered the system itself with the devices list being attached to give
    the user a better understanding of what the emulation is using.

    If slots are populated with devices, any additional slots those devices
    provide will be visible with **-listdevices** as well. For instance,
    installing a floppy controller into a PC will expose the disk drive slots.

    Example:
        .. code-block:: bash

            mame apple2e -listdevices
            Driver apple2e (Apple //e):
               <root>                         Apple //e
                 a2bus                        Apple II Bus
                 a2common                     Apple II Common Components @ 14.31 MHz
                 a2video                      Apple II video @ 14.31 MHz
                 aux                          Apple IIe AUX Slot
                   ext80                      Apple IIe Extended 80-Column Card
                 auxbus                       Apple IIe AUX Bus
                 ay3600                       AY-5-3600 Keyboard Encoder
                 ...
                 speaker                      Filtered 1-bit DAC
                 tape                         Cassette

.. _mame-commandline-listslots:

**-listslots** / **-lslot** [<*pattern*>]

    Show available slots and options for each slot (if available). Primarily
    used for MAME to allow control over internal plug-in cards, much like PCs
    needing video, sound and other expansion cards.

    If slots are populated with devices, any additional slots those devices
    provide will be visible with **-listslots** as well. For instance,
    installing a floppy controller into a PC will expose the disk drive slots.

    The slot name (e.g. **ctrl1**) can be used from the command
    line (**-ctrl1** in this case)

    Example:
        .. code-block:: bash

            mame apple2e -listslots
            SYSTEM           SLOT NAME        SLOT OPTIONS     SLOT DEVICE NAME
            ---------------- ---------------- ---------------- ----------------------------
            apple2e          sl1              4play            4play Joystick Card (rev. B)
                                              ...
                                              aevm80           Applied Engineering Viewmaster 80
                                              alfam2           ALF MC1 / Apple Music II
                                              ...
                                              zipdrive         Zip Technologies ZipDrive

                             ...
                             aux              ext80            Apple IIe Extended 80-Column Card
                                              rw3              Applied Engineering RamWorks III
                                              std80            Apple IIe Standard 80-Column Card

                             gameio           compeyes         Digital Vision ComputerEyes
                                              joy              Apple II analog joysticks
                                              paddles          Apple II paddles

.. _mame-commandline-listmedia:

**-listmedia** / **-lm** [<*pattern*>]

    List available media that the chosen system allows to be used. This
    includes media types (cartridge, cassette, diskette and more) as well as
    common file extensions which are supported.

    Example:
        .. code-block:: bash

            mame coco3 -listmedia
            SYSTEM           MEDIA NAME       (brief)    IMAGE FILE EXTENSIONS SUPPORTED
            ---------------- --------------------------- -------------------------------
            coco3            cassette         (cass)     .wav  .cas
                             printout         (prin)     .prn
                             cartridge        (cart)     .ccc  .rom
                             floppydisk1      (flop1)    .dmk  .jvc  .dsk  .vdk  .sdf  .os9  .d77  .d88  .1dd  .dfi  .hfe  .imd  .ipf  .mfi  .mfm  .td0  .cqm  .cqi
                             floppydisk2      (flop2)    .dmk  .jvc  .dsk  .vdk  .sdf  .os9  .d77  .d88  .1dd  .dfi  .hfe  .imd  .ipf  .mfi  .mfm  .td0  .cqm  .cqi
                             harddisk1        (hard1)    .vhd
                             harddisk2        (hard2)    .vhd

.. _mame-commandline-listsoftware:

**-listsoftware** / **-lsoft** [<*pattern*>]

    Displays the contents of all software lists that can be used by the system
    or systems represented by *pattern*.

    Example:
        .. code-block:: bash

            mame coco3 -listsoftware
            <?xml version="1.0"?>
            <!DOCTYPE softwarelists [
            <!ELEMENT softwarelists (softwarelist*)>
	            <!ELEMENT softwarelist (software+)>
		            <!ATTLIST softwarelist name CDATA #REQUIRED>
		            <!ATTLIST softwarelist description CDATA #IMPLIED>
		            <!ELEMENT software (description, year, publisher, info*, sharedfeat*, part*)>
                    ...
            <softwarelists>
	            <softwarelist name="coco_cart" description="Tandy Radio Shack Color Computer cartridges">
		            <software name="7cardstd">
			            <description>7 Card Stud</description>
			            <year>1983</year>
			            <publisher>Tandy</publisher>
			            <info name="developer" value="Intelligent Software"/>
			            <info name="serial" value="26-3074"/>
			            <part name="cart" interface="coco_cart">
				            <dataarea name="rom" size="8192">
					            <rom name="7 card stud (1983) (26-3074) (intelligent software).rom" size="8192" crc="f38d8c97" sha1="5cfcb699ce09840dbb52714c8d91b3d86d3a86c3"/>
				            </dataarea>
			            </part>
		            </software>
                    ...

.. _mame-commandline-verifysoftware:

**-verifysoftware** / **-vsoft** [<*pattern*>]

    Checks for invalid or missing ROM images in your software lists. By default
    all drivers that have valid ZIP files or directories in the rompath are
    verified; however, you can limit this list by specifying a specific driver
    name or *pattern* after the **-verifysoftware** command.

    Example:
        .. code-block:: bash

            mame coco3 -verifysoftware
            romset coco_cart:7cardstd is good
            coco_cart:amazing: a mazing world of malcom mortar (1987)(26-3160)(zct systems).rom (16384 bytes) - NEEDS REDUMP
            romset coco_cart:amazing is best available
            coco_cart:amazing1: a mazing world of malcom mortar (1987)(26-3160)(zct systems)[a].rom (16384 bytes) - NEEDS REDUMP
            romset coco_cart:amazing1 is best available
            romset coco_cart:androne is good
            ...

.. _mame-commandline-getsoftlist:

**-getsoftlist** / **-glist** [<*pattern*>]

    Displays the contents of a specific softlist with the filename represented
    by *pattern*.

    Example:
        .. code-block:: bash

            mame -getsoftlist apple2_flop_orig
            <?xml version="1.0"?>
            <!DOCTYPE softwarelists [
            <!ELEMENT softwarelists (softwarelist*)>
	            <!ELEMENT softwarelist (software+)>
		            <!ATTLIST softwarelist name CDATA #REQUIRED>
		            <!ATTLIST softwarelist description CDATA #IMPLIED>
		            <!ELEMENT software (description, year, publisher, info*, sharedfeat*, part*)>
			            <!ATTLIST software name CDATA #REQUIRED>
			            <!ATTLIST software cloneof CDATA #IMPLIED>
			            <!ATTLIST software supported (yes|partial|no) "yes">
			            <!ELEMENT description (#PCDATA)>
			            <!ELEMENT year (#PCDATA)>
			            <!ELEMENT publisher (#PCDATA)>
			            <!ELEMENT info EMPTY>
				            <!ATTLIST info name CDATA #REQUIRED>
				            <!ATTLIST info value CDATA #IMPLIED>
			            <!ELEMENT sharedfeat EMPTY>
				            <!ATTLIST sharedfeat name CDATA #REQUIRED>
				            <!ATTLIST sharedfeat value CDATA #IMPLIED>
                        ...

.. _mame-commandline-verifysoftlist:

**-verifysoftlist** / **-vlist** [softwarelistname]

    Checks a specified software list for missing ROM images if files exist for
    issued softwarelistname. By default, all drivers that have valid ZIP files
    or directories in the rompath are verified; however, you can limit this list
    by specifying a specific softwarelistname (without .XML) after the
    -verifysoftlist command.

    Example:
        .. code-block:: bash

            mame -verifysoftlist apple2_flop_orig
            romset apple2_flop_orig:agentusa is good
            romset apple2_flop_orig:airheart is good
            romset apple2_flop_orig:aplpanic is good
            romset apple2_flop_orig:alambush is good
            romset apple2_flop_orig:ankh is good
            romset apple2_flop_orig:aplcdspd is good
            romset apple2_flop_orig:agalxian is good
            romset apple2_flop_orig:aquatron is good
            romset apple2_flop_orig:archon is good
            romset apple2_flop_orig:archon2 is good
            romset apple2_flop_orig:ardyardv is good
            romset apple2_flop_orig:autobahn is good
            ...


.. _mame-commandline-osdoptions:

OSD-related Options
-------------------

.. _mame-commandline-uimodekey:

**-uimodekey** [*keystring*]

    Key used to enable/disable MAME keyboard controls when the emulated system
    has keyboard inputs.  The default setting is **Forward Delete** on macOS or
    **SCRLOCK** on other operating systems (including Windows and Linux).  Use
    **FN-Delete** on Macintosh computers with notebook/compact keyboards.

    Example:
        .. code-block:: bash

            mame ibm5150 -uimodekey DEL

.. _mame-commandline-uifontprovider:

**-uifontprovider** *<module>*

    Chooses provider for UI font rendering. The default setting is ``auto``.

.. list-table:: Supported UI font providers per-platform
    :header-rows: 0
    :stub-columns: 0

    * - **Microsoft Windows**
      - win
      - dwrite
      - none
      - auto
      -
      - sdl [#UIFPSDLWindows]_.
    * - **macOS**
      -
      -
      - none
      - auto
      - osx
      - sdl
    * - **Linux**
      -
      -
      - none
      - auto
      -
      - sdl

..  rubric:: Footnotes

..  [#UIFPSDLWindows] SDL support on Windows requires that you compile MAME with
                      the support in. By default SDL is not included in Windows
                      builds of MAME.

Example:
    .. code-block:: bash

        mame ajax -uifontprovider dwrite

.. _mame-commandline-keyboardprovider:

**-keyboardprovider** *<module>*

    Chooses how MAME will get keyboard input. The default is ``auto``.

.. list-table:: Supported keyboard input providers per-platform
    :header-rows: 0
    :stub-columns: 0

    * - **Microsoft Windows**
      - auto [#KBIPAutoWindows]_.
      - rawinput
      - dinput
      - win32
      - none
      - sdl [#KBIPSDLWindows]_.
    * - **SDL (macOS and Linux)**
      - auto [#KBIPAutoSDL]_.
      -
      -
      -
      - none
      - sdl
    * - **Linux**
      - auto [#KBIPAutoSDL]_.
      -
      -
      -
      - none
      - sdl

..  rubric:: Footnotes

..  [#KBIPAutoWindows] ``auto`` on Windows will try ``rawinput`` with
                       fallback to ``dinput``.

..  [#KBIPSDLWindows] SDL support on Windows requires that you compile MAME with
                      the support in. By default SDL is not included in Windows
                      builds of MAME.

..  [#KBIPAutoSDL] ``auto`` on SDL will default to ``sdl``.

.. Tip:: Note that user-mode keyboard emulation tools such as joy2key will
         almost certainly require the use of **-keyboardprovider win32** on
         Windows machines.

Example:
    .. code-block:: bash

        mame c64 -keyboardprovider win32

.. _mame-commandline-mouseprovider:

**-mouseprovider** *<module>*

    Chooses how MAME will get mouse input. The default is ``auto``.

.. list-table:: Supported mouse input providers per-platform
    :header-rows: 0
    :stub-columns: 0

    * - **Microsoft Windows**
      - auto [#MIPAutoWindows]_.
      - rawinput
      - dinput
      - win32
      - none
      - sdl [#MIPSDLWindows]_.
    * - **SDL (macOS and Linux)**
      - auto [#MIPAutoSDL]_.
      -
      -
      -
      - none
      - sdl
    * - **Linux**
      - auto [#MIPAutoSDL]_.
      -
      -
      -
      - none
      - sdl

..  rubric:: Footnotes

..  [#MIPAutoWindows] On Windows, ``auto`` will try ``rawinput`` with fallback
                      to ``dinput``.

..  [#MIPSDLWindows] SDL support on Windows requires that you compile MAME with
                      the support in. By default SDL is not included in Windows
                      builds of MAME.

..  [#MIPAutoSDL] ``auto`` on SDL will default to ``sdl``.

Example:
    .. code-block:: bash

        mame indy_4610 -mouseprovider win32

.. _mame-commandline-lightgunprovider:

**-lightgunprovider** *<module>*

    Chooses how MAME will get light gun input. The default is ``auto``.

.. list-table:: Supported light gun input providers per-platform
    :header-rows: 0
    :stub-columns: 0

    * - **Microsoft Windows**
      - auto [#LGIPAutoWindows]_.
      - rawinput
      - win32
      - none
      -
      -
    * - **macOS**
      - auto [#LGIPAutoSDL]_.
      -
      -
      - none
      -
      -
    * - **Linux**
      - auto [#LGIPAutoLinux]_.
      -
      -
      - none
      -
      - x11

..  rubric:: Footnotes

..  [#LGIPAutoWindows] On Windows, auto will try ``rawinput`` with fallback to
                       ``win32``, or ``none`` if it doesn't find any.

..  [#LGIPAutoSDL] On non-Linux SDL, ``auto`` will default to ``none``.

..  [#LGIPAutoLinux] On SDL/Linux, ``auto`` will default to ``x11``, or ``none``
                     if it doesn't find any.

Example:
    .. code-block:: bash

        mame lethalen -lightgunprovider x11

.. _mame-commandline-joystickprovider:

**-joystickprovider** *<module>*

    Chooses how MAME will get joystick input. The default is ``auto``.

.. list-table:: Supported joystick input providers per-platform
    :header-rows: 0
    :stub-columns: 0

    * - **Microsoft Windows**
      - auto [#JIPAutoWindows]_.
      - winhybrid
      - dinput
      - xinput
      - none
      - sdl
    * - **SDL**
      - auto [#JIPAutoSDL]_.
      -
      -
      -
      - none
      - sdl

..  rubric:: Footnotes

..  [#JIPAutoWindows] On Windows, auto will default to ``dinput``.

..  [#JIPAutoSDL] On SDL, auto will default to ``sdl``.

.. Tip:: Note that Microsoft XBox 360 and XBox One controllers connected to
         Windows will work best with ``winhybrid`` or ``xinput``. The
         ``winhybrid`` option supports a mix of DirectInput and XInput
         controllers at the same time.

Example:
    .. code-block:: bash

        mame mk2 -joystickprovider winhybrid

.. Tip:: On Windows, winhybrid is likely to give the best experience by
         supporting both XInput and DirectInput controllers.


.. _mame-commandline-cliverbs:

OSD CLI Verbs
-------------

.. _mame-commandline-listmidi:

**-listmidi**

    List available MIDI I/O devices for use with emulation.

    Example:
        .. code-block:: bash

            mame -listmidi
            MIDI input ports:

            MIDI output ports:
            Microsoft MIDI Mapper (default)
            Microsoft GS Wavetable Synth

.. _mame-commandline-listnetwork:

**-listnetwork**

    List available network adapters for use with emulation.

    Example 1:
        .. code-block:: bash

            mame -listnetwork
            No network adapters were found

    Example 2:
        .. code-block:: bash

            mame -listnetwork
            Available network adapters:
                Local Area Connection

.. Tip:: On Windows, you'll need the TAP driver from OpenVPN for MAME to see
         any network adapters.


.. _mame-commandline-osdoutput:

OSD Output Options
------------------

.. _mame-commandline-output:

**\-output**

    Chooses how MAME will handle processing of output notifiers. These are used
    to connect external outputs such as the LED lights for the Player 1/2 start
    buttons on certain arcade machines.

    You can choose from: ``auto``, ``none``, ``console`` or ``network``

    Note that network port is fixed at 8000.

    Example:
        .. code-block:: bash

            mame asteroid -output console
            led0 = 1
            led0 = 0
            ...
            led0 = 1
            led0 = 0


.. _mame-commandline-configoptions:

Configuration Options
---------------------

.. _mame-commandline-noreadconfig:

**-[no]readconfig** / **-[no]rc**

    Enables or disables the reading of the config files. When enabled (which is
    the default), MAME reads the following config files in order:

      - ``mame.ini``
      - ``debug.ini``                       (if the debugger is enabled)
      - ``source/``\ *<driver>*\ ``.ini``   (based on the source filename of the driver)
      - ``vertical.ini``                    (for systems with vertical monitor orientation)
      - ``horizont.ini``                    (for systems with horizontal monitor orientation)
      - ``arcade.ini``                      (for systems in source added with ``GAME()`` macro)
      - ``console.ini``                     (for systems in source added with ``CONS()`` macro)
      - ``computer.ini``                    (for systems in source added with ``COMP()`` macro)
      - ``othersys.ini``                    (for systems in source added with ``SYST()`` macro)
      - ``vector.ini``                      (for vector systems only)
      - *<parent>*\ ``.ini``                (for clones only, may be called recursively)
      - *<systemname>*\ ``.ini``

      (See :ref:`advanced-multi-CFG` for further details)

    The settings in the later INIs override those in the earlier INIs.  So, for
    example, if you wanted to disable overlay effects in the vector systems, you
    can create a ``vector.ini`` with line ``effect none`` in it, and it will
    override whatever ``effect`` value you have in your ``mame.ini``.

    The default is ON (**-readconfig**).

    Example:
        .. code-block:: bash

            mame apple2ee -noreadconfig -sl6 diskii -sl7 cffa2 -hard1 TotalReplay.2mg


.. _mame-commandline-pathoptions:

Core Search Path Options
------------------------

.. _mame-commandline-homepath:

**-homepath** *<path>*

    Specifies a path for Lua plugins to store data.

    The default is ``.`` (that is, in the current working directory).

    Example:
        .. code-block:: bash

            mame -homepath c:\mame\lua

.. _mame-commandline-rompath:

**-rompath** / **-rp** *<path>*

    Specifies one or more paths within which to find ROM or disk images.
    Multiple paths can be specified by separating them with semicolons.

    The default is ``roms`` (that is, a directory ``roms`` in the current
    working directory).

    Example:
        .. code-block:: bash

            mame -rompath c:\mame\roms;c:\roms\another

.. _mame-commandline-hashpath:

**-hashpath** / **-hash_directory** / **-hash** *<path>*

    Specifies one or more paths within which to find software definition files.
    Multiple paths can be specified by separating them with semicolons.

    The default is ``hash`` (that is, a directory ``hash`` in the current
    working directory).

    Example:
        .. code-block:: bash

            mame -hashpath c:\mame\hash;c:\roms\softlists

.. _mame-commandline-samplepath:

**-samplepath** / **-sp** *<path>*

    Specifies one or more paths within which to find audio sample files.
    Multiple paths can be specified by separating them with semicolons.

    The default is ``samples`` (that is, a directory ``samples`` in the current
    working directory).

    Example:
        .. code-block:: bash

            mame -samplepath c:\mame\samples;c:\roms\samples

.. _mame-commandline-artpath:

**-artpath** *<path>*

    Specifies one or more paths within which to find external layout and artwork
    files.  Multiple paths can be specified by separating them with semicolons.

    The default is ``artwork`` (that is, a directory ``artwork`` in the current
    working directory).

    Example:
        .. code-block:: bash

            mame -artpath c:\mame\artwork;c:\emu\shared-artwork

.. _mame-commandline-ctrlrpath:

**-ctrlrpath** *<path>*

    Specifies one or more paths within which to find controller configuration
    files.  Multiple paths can be specified by separating them with semicolons.
    Used in conjunction with the ``-ctrlr`` option.

    The default is ``ctrlr`` (that is, a directory ``ctrlr`` in the current
    working directory).

    Example:
        .. code-block:: bash

            mame -ctrlrpath c:\mame\ctrlr;c:\emu\controllers

.. _mame-commandline-inipath:

**-inipath** *<path>*

    Specifies one or more paths within which to find INI files.  Multiple paths
    can be specified by separating them with semicolons.

    On Windows, the default is ``.;ini;ini/presets`` (that is, search in the
    current directory first, then in the directory ``ini`` in the current
    working directory, and finally the directory ``presets`` inside that
    directory).

    On macOS, the default is
    ``$HOME/Library/Application Support/mame;$HOME/.mame;.;ini`` (that is,
    search the ``mame`` folder inside the current user's Application Support
    folder, followed by the ``.mame`` folder in the current user's home
    directory, then the current working directory, and finally the directory
    ``ini`` in the current working directory).

    On other platforms (including Linux), the default is ``$HOME/.mame;.;ini``
    (that is search the ``.mame`` directory in the current user's home
    directory, followed by the current working directory, and finally the
    directory ``ini`` in the current working directory).

    Example:
        .. code-block:: bash

            mame -inipath c:\users\thisuser\documents\mameini

.. _mame-commandline-fontpath:

**-fontpath** *<path>*

    Specifies one or more paths within which to find BDF (Adobe Glyph Bitmap
    Distribution Format) font files.  Multiple paths can be specified by
    separating them with semicolons.

    The default is ``.`` (that is, search in the current working directory).

    Example:
        .. code-block:: bash

            mame -fontpath c:\mame\;c:\emu\artwork\mamefonts

.. _mame-commandline-cheatpath:

**-cheatpath** *<path>*

    Specifies one or more paths within which to find XML cheat files.  Multiple
    paths can be specified by separating them with semicolons.

    The default is ``cheat`` (that is, a folder called ``cheat`` located in the
    current working directory).

    Example:
        .. code-block:: bash

            mame -cheatpath c:\mame\cheat;c:\emu\cheats

.. _mame-commandline-crosshairpath:

**-crosshairpath** *<path>*

    Specifies one or more paths within which to find crosshair image files.
    Multiple paths can be specified by separating them with semicolons.

    The default is ``crsshair`` (that is, a directory ``crsshair`` in the
    current working directory).

    Example:
        .. code-block:: bash

            mame -crosshairpath c:\mame\crsshair;c:\emu\artwork\crosshairs

.. _mame-commandline-pluginspath:

**-pluginspath** *<path>*

    Specifies one or more paths within which to find Lua plugins for MAME.

    The default is ``plugins`` (that is, a directory ``plugins`` in the current
    working directory).

    Example:
        .. code-block:: bash

            mame -pluginspath c:\mame\plugins;c:\emu\lua

.. _mame-commandline-languagepath:

**-languagepath** *<path>*

    Specifies one or more paths within which to find language files for
    localized UI text.

    The default is ``language`` (that is, a directory ``language`` in the
    current working directory).

    Example:
        .. code-block:: bash

            mame -languagepath c:\mame\language;c:\emu\mame-languages

.. _mame-commandline-swpath:

**-swpath** *<path>*

    Specifies the default path from which to load loose software image files.

    The default is ``sofware`` (that is, a directory ``software`` in the current
    working directory).

    Example:
        .. code-block:: bash

            mame -swpath c:\mame\software;c:\emu\mydisks


.. _mame-commandline-coreoutdir:

Core Output Directory Options
-----------------------------

.. _mame-commandline-cfgdirectory:

**-cfg_directory** *<path>*

    Specifies the directory where configuration files are stored.  Configuration
    files are read when starting MAME or when starting an emulated machine, and
    written on exit.  Configuration files preserve settings including input
    assignment, DIP switch settings, bookkeeping statistics, and debugger window
    arrangement.

    The default is ``cfg`` (that is, a directory ``cfg`` in the current working
    directory). If this directory does not exist, it will be created
    automatically.

    Example:
        .. code-block:: bash

            mame -cfg_directory c:\mame\cfg

.. _mame-commandline-nvramdirectory:

**-nvram_directory** *<path>*

    Specifies the directory where NVRAM files are stored.  NVRAM files store the
    contents of EEPROM, non-volatile RAM (NVRAM), and other programmable devices
    for systems that used this type of hardware.  This data is read when
    starting an emulated machine and written on exit.

    The default is ``nvram`` (that is, a directory ``nvram`` in the current
    working directory)).  If this directory does not exist, it will be created
    automatically.

    Example:
        .. code-block:: bash

            mame -nvram_directory c:\mame\nvram

.. _mame-commandline-inputdirectory:

**-input_directory** *<path>*

    Specifies the directory where input recording files are stored.  Input
    recordings are created using the **-record** option and played back using
    the **-playback** option.

    The default is ``inp`` (that is, a directory ``inp`` in the current working
    directory).  If this directory does not exist, it will be created
    automatically.

    Example:
        .. code-block:: bash

            mame -input_directory c:\mame\inp

.. _mame-commandline-statedirectory:

**-state_directory** *<path>*

    Specifies the directory where save state files are stored.  Save state files
    are read and written either upon user request, or when using the
    **-autosave** option.

    The default is ``sta`` (that is, a directory ``sta`` in the current working
    directory).  If this directory does not exist, it will be created
    automatically.

    Example:
        .. code-block:: bash

            mame -state_directory c:\mame\sta

.. _mame-commandline-snapshotdirectory:

**-snapshot_directory** *<path>*

    Specifies the directory where screen snapshots and video recordings are
    stored when requested by the user.

    The default is ``snap`` (that is, a directory ``snap`` in the current
    working directory). If this directory does not exist, it will be created
    automatically.

    Example:
        .. code-block:: bash

            mame -snapshot_directory c:\mame\snap

.. _mame-commandline-diffdirectory:

**-diff_directory** *<path>*

    Specifies the directory where hard drive difference files are stored.  Hard
    drive difference files store data that is written back to an emulated hard
    disk, in order to preserve the original image file.  The difference files
    are created when starting an emulated system with a compressed hard disk
    image.

    The default is ``diff`` (that is, a directory ``diff`` in the current
    working directory).  If this directory does not exist, it will be created
    automatically.

    Example:
        .. code-block:: bash

            mame -diff_directory c:\mame\diff

.. _mame-commandline-commentdirectory:

**-comment_directory** *<path>*

    Specifies a directory where debugger comment files are stored.  Debugger
    comment files are written by the debugger when comments are added to the
    disassembly for a system.

    The default is ``comments`` (that is, a directory ``comments`` in the
    current working directory).  If this directory does not exist, it will be
    created automatically.

    Example:
        .. code-block:: bash

            mame -comment_directory c:\mame\comments


.. _mame-commandline-savestate:

Core State/Playback Options
---------------------------

.. _mame-commandline-norewind:

**-[no]rewind**

    When enabled and emulation is paused, automatically creates a save state in
    memory every time a frame is advanced.  Rewind save states can then be
    loaded consecutively by pressing the rewind single step shortcut key
    (**Left Shift + Tilde** by default).

    The default rewind value is OFF (**-norewind**).

    If debugger is in a 'break' state, a save state is instead created every
    time step in, step over, or step out occurs.  In that mode, rewind save
    states can be loaded by executing the debugger **rewind** (or **rw**)
    command.

    Example:
        .. code-block:: bash

            mame -norewind

.. _mame-commandline-rewindcapacity:

**-rewind_capacity** *<value>*

    Sets the rewind capacity value, in megabytes.  It is the total amount of
    memory rewind savestates can occupy.  When capacity is hit, old savestates
    get erased as new ones are captured.  Setting capacity lower than the
    current savestate size disables rewind. Values below 0 are automatically
    clamped to 0.

    Example:
        .. code-block:: bash

            mame -rewind_capacity 30

.. _mame-commandline-state:

**-state** *<slot>*

    Immediately after starting the specified system, will cause the save state
    in the specified <slot> to be loaded.

    Example:
        .. code-block:: bash

            mame -state 1

.. _mame-commandline-noautosave:

**-[no]autosave**

    When enabled, automatically creates a save state file when exiting MAME and
    automatically attempts to reload it when later starting MAME with the same
    system.  This only works for systems that have explicitly enabled save state
    support in their driver.

    The default is OFF (**-noautosave**).

    Example:
        .. code-block:: bash

            mame -autosave

.. _mame-commandline-playback:

**-playback** / **-pb** *<filename>*

    Specifies a file from which to play back a series of inputs.  This feature
    does not work reliably for all systems, but can be used to watch a
    previously recorded game session from start to finish.

    The default is ``NULL`` (no playback).

    Example:
        .. code-block:: bash

            mame pacman -playback worldrecord

.. Tip:: You may experience desync in playback if the configuration, NVRAM, and
         memory card files don't match the original; this is why it is suggested
         you should only record and playback with all configuration (.cfg),
         NVRAM (.nv), and memory card files deleted.

.. _mame-commandline-exitafterplayback:

**-[no]exit_after_playback**

    When used in conjunction with the **-playback** option, MAME will exit after
    playing back the input file.  By default, MAME continues to run the emulated
    system after playback completes.

    The default is OFF (**-noexit_after_playback**).

    Example:
        .. code-block:: bash

            mame pacman -playback worldrecord -exit_after_playback

.. _mame-commandline-record:

**-record** / **-rec** *<filename>*

    Specifies a file to record all input from a session.  This can be used to
    record a session for later playback.  This feature does not work reliably
    for all systems, but can be used to record a session from start to finish.

    The default is ``NULL`` (no recording).

    Example:
        .. code-block:: bash

            mame pacman -record worldrecord

.. Tip:: You may experience desync in playback if the configuration, NVRAM, and
         memory card files don't match the original; this is why it is suggested
         you should only record and playback with all configuration (.cfg),
         NVRAM (.nv), and memory card files deleted.

.. _mame-commandline-mngwrite:

**-mngwrite** *<filename>*

    Writes each video frame to the given <filename> in MNG format, producing an
    animation of the session.  Note that **-mngwrite** only writes video frames;
    it does not save any audio data.  Either use **-wavwrite** to record audio
    and combine the audio and video tracks using video editing software, or use
    **-aviwrite** to record audio and video to a single file.

    The default is ``NULL`` (no recording).

    Example:
        .. code-block:: bash

            mame pacman -mngwrite pacman-video

.. _mame-commandline-aviwrite:

**-aviwrite** *<filename>*

    Stream video and sound data to the given <filename> in uncompressed AVI
    format, producing an animation of the session complete with sound.  Note
    that the AVI format does not changes to resolution or frame rate,
    uncompressed video consumes a lot of disk space, and recording uncompressed
    video in realtime requires a fast disk.  It may be more practical to record
    an emulation session using **-record** then make a video of it with
    **-aviwrite** in combination with **-playback** and **-exit_after_playback**
    options.

    The default is ``NULL`` (no recording).

    Example:
        .. code-block:: bash

            mame pacman -playback worldrecord -exit_after_playback -aviwrite worldrecord

.. _mame-commandline-wavwrite:

**-wavwrite** *<filename>*

    Writes the final mixer output to the given <filename> in WAV format,
    producing an audio recording of the session.

    The default is ``NULL`` (no recording).

    Example:
        .. code-block:: bash

            mame pacman -wavewrite pacsounds

.. _mame-commandline-snapname:

**-snapname** *<name>*

    Describes how MAME should name files for snapshots.  <name> is a string that
    provides a template that is used to generate a filename.

    Three simple substitutions are provided: the ``/`` character represents the
    path separator on any target platform (even Windows); the string ``%g``
    represents the driver name of the current system; and the string ``%i``
    represents an incrementing index.  If ``%i`` is omitted, then each snapshot
    taken will overwrite the previous one; otherwise, MAME will find the next
    empty value for ``%i`` and use that for a filename.

    The default is ``%g/%i``, which creates a separate folder for each system,
    and names the snapshots under it starting with 0000 and increasing from
    there.

    In addition to the above, for drivers using different media, like carts or
    floppy disks, you can also use the ``%d_[media]`` indicator. Replace [media]
    with the media switch you want to use.

    Example 1:
        .. code-block:: bash

            mame robby -snapname foo\%g%i

        Snapshots will be saved as ``snaps\foo\robby0000.png``, ``snaps\foo\robby0001.png`` and so on.

    Example 2:
        .. code-block:: bash

            mame nes -cart robby -snapname %g\%d_cart

        Snapshots will be saved as ``snaps\nes\robby.png``.

    Example 3:
        .. code-block:: bash

            mame c64 -flop1 robby -snapname %g\%d_flop1/%i

        Snapshots will be saved as ``snaps\c64\robby\0000.png``.

.. _mame-commandline-snapsize:

**-snapsize** *<width>x<height>*

    Hard-codes the size for snapshots and movie recording.  By default, MAME
    will create snapshots at the system's current resolution in raw pixels, and
    will create movies at the system's starting resolution in raw pixels.  If
    you specify this option, then MAME will create both snapshots and movies at
    the size specified, and will bilinear filter the result.

    The default is ``auto``.

    Example:
        .. code-block:: bash

            mame pacman -snapsize 1920x1080

.. Tip:: -snapsize does not automatically rotate if the system is vertically
         oriented, so for vertical systems you'll want to swap the width and
         height options.

.. _mame-commandline-snapview:

**-snapview** *<viewname>*

    Specifies the view to use when rendering snapshots and videos.  The
    *<viewname>* does not need to be the full name of a view, MAME will choose
    the first view with a name that has the *<viewname>* as a prefix.  For
    example **-snapview "screen 0 pixel"** will match the
    “\ *Screen 0 Pixel Aspect (10:7)*\ ” view.

    If the *<viewname>* is ``auto`` or an empty string, MAME will select a view
    based on the number of emulated screens in the system, and the available
    external and internal artwork.  MAME tries to select a view that shows all
    emulated screens by default.

    If the *<viewname>* is ``native``, MAME uses special internal view to save a
    separate snapshot for each visible emulated screen, or to record a video for
    the first visible screen only.  The snapshot(s) or video will have the same
    resolution as the emulated screen(s) with no artwork elements drawn or
    effects applied.

    The default value is ``auto``.

    Example:
        .. code-block:: bash

            mame wrecking -snapview cocktail


.. _mame-commandline-nosnapbilinear:

**-[no]snapbilinear**

    Specify if the snapshot or movie should have bilinear filtering applied.
    Disabling this off can improve performance while recording video to a file.

    The default is ON (**-snapbilinear**).

    Example:
        .. code-block:: bash

            mame pacman -nosnapbilinear

.. _mame-commandline-statename:

**-statename** *<name>*

    Describes how MAME should store save state files, relative to the
    state_directory path.  <name> is a string that provides a template that is
    used to generate a relative path.

    Two simple substitutions are provided: the ``/`` character represents the
    path separator on any target platform (even Windows); the string ``%g``
    represents the driver name of the current system.

    The default is ``%g``, which creates a separate folder for each system.

    In addition to the above, for drivers using different media, like carts or
    floppy disks, you can also use the ``%d_[media]`` indicator. Replace
    ``[media]`` with the media switch you want to use.

    Example 1:
        .. code-block:: bash

            mame robby -statename foo\%g
            All save states will be stored inside sta\foo\robby\

    Example 2:
        .. code-block:: bash

            mame nes -cart robby -statename %g/%d_cart
            All save states will be stored inside sta\nes\robby\

    Example 3:
        .. code-block:: bash

            mame c64 -flop1 robby -statename %g/%d_flop1
            All save states will be stored inside sta\c64\robby\

.. Tip:: Note that even on Microsoft Windows, you should use ``/`` as your
         path seperator for **-statename**


.. _mame-commandline-noburnin:

**-[no]burnin**

    Tracks brightness of the screen during play and at the end of emulation
    generates a PNG that can be used to simulate burn-in effects on other
    systems.  The resulting PNG is created such that the least used-areas of
    the screen are fully white (since burned-in areas are darker, all other
    areas of the screen must be lightened a touch).

    The intention is that this PNG can be loaded via an artwork file with a low
    alpha (e.g, 0.1-0.2 seems to work well) and blended over the entire screen.

    The PNG files are saved in the snap directory under the
    ``<systemname>/burnin-<screen.name>.png``.

    The default is OFF (**-noburnin**).

    Example:
        .. code-block:: bash

            mame neogeo -burnin


.. _mame-commandline-perf:

Core Performance Options
------------------------

.. _mame-commandline-noautoframeskip:

**-[no]autoframeskip** / **-[no]afs**

    Dynamically adjust the frameskip level while you're running the system to
    maintain full speed.  Turning this on overrides the **-frameskip** setting
    described below.

    This is off by default (**-noautoframeskip**).

    Example:
        .. code-block:: bash

            mame gradius4 -autoframeskip

.. _mame-commandline-frameskip:

**-frameskip** / **-fs** *<level>*

    Specifies the frameskip value.  This is the number of frames out of every 12
    to drop when running.  For example, if you specify **-frameskip 2**, MAME
    will render and display 10 out of every 12 emulated frames.  By skipping
    some frames, you may be able to get full speed emulation for a system that
    would otherwise be too demanding for your computer.

    The default value is **-frameskip 0**, which skips no frames.

    Example:
        .. code-block:: bash

            mame gradius4 -frameskip 2

.. _mame-commandline-secondstorun:

**-seconds_to_run** / **-str** *<seconds>*

    This option tells MAME to automatically stop emulation after a fixed number
    of seconds of emulated time have elapsed.  This may be useful for
    benchmarking and automated testing.  By combining this with a fixed set of
    other command line options, you can set up a consistent environment for
    benchmarking MAME's emulation performance.  In addition, upon exit, the
    **-str** option will write a screenshot to the system's snapshot directory
    with the file name determined by the **-snapname** option.

    Example:
        .. code-block:: bash

            mame pacman -seconds_to_run 60

.. _mame-commandline-nothrottle:

**-[no]throttle**

   Enable or disable thottling emulation speed.  When throttling is enabled,
   MAME limits emulation speed to so the emulated system will not run faster
   than the original hardware.  When throttling is disabled, MAME runs the
   emulation as fast as possible. Depending on your settings and the
   characteristics of the emulated system, performance may be limited by your
   CPU, graphics card, or even memory performance.

   The default is to enable throttling (**-throttle**).

    Example:
        .. code-block:: bash

            mame pacman -nothrottle

.. _mame-commandline-nosleep:

**-[no]sleep**

    When enabled along with **-throttle**, MAME will yield the CPU when
    limiting emulation speed.  This allows other programs to use CPU time,
    assuming the main emulation thread isn't completely utilising a CPU core.
    This option can potentially cause hiccups in performance if other demanding
    programs are running.

    The default is on (**-sleep**).

    Example:
        .. code-block:: bash

            mame gradius 4 -nosleep

.. _mame-commandline-speed:

**-speed** *<factor>*

    Changes the way MAME throttles the emulation so that it runs at some
    multiple of the system's original speed.  A *<factor>* of ``1.0`` means to
    run the system at its normal speed, a *<factor>* of ``0.5`` means run at
    half speed, and a *<factor>* of 2.0 means run at double speed.  Note that
    changing this value affects sound playback as well, which will scale in
    pitch accordingly.  The internal precision of the fraction is two decimal
    places, so a *<factor>* of ``1.002`` is rounded to ``1.00``.

    The default is ``1.0`` (normal speed).

    Example:
        .. code-block:: bash

            mame pacman -speed 1.25

.. _mame-commandline-norefreshspeed:

**-[no]refreshspeed** / **-[no]rs**

    Allows MAME to adjust the emulation speed so that the refresh rate of the
    first emulated screen does not exceed the slowest refresh rate for any
    targeted monitors in your system.  Thus, if you have a 60Hz monitor and run
    a system that is designed to run at 60.6Hz, MAME will reduce the emulation
    speed to 99% in order to prevent sound hiccups or other undesirable side
    effects of running at a slower refresh rate.

    The default is off (**-norefreshspeed**).

    Example:
        .. code-block:: bash

            mame pacman -refreshspeed

.. _mame-commandline-numprocessors:

**-numprocessors** / **-np** **auto**\ \|\ *<value>*

    Specify the number of threads to use for work queues.  Specifying ``auto``
    will use the value reported by the system or environment variable
    ``OSDPROCESSORS``.  This value is internally limited to four times the
    number of processors reported by the system.

    The default is ``auto``.

    Example:
        .. code-block:: bash

            mame gradius4 -numprocessors 2

.. _mame-commandline-bench:

**-bench** *<n>*

    Benchmark for *<n>* emulated seconds.  This is equivalent to the following
    options:

    **-str** *<n>* **-video none -sound none -nothrottle**

    Example:
        .. code-block:: bash

            mame gradius4 -bench 300

.. _mame-commandline-lowlatency:

**-[no]lowlatency**

    This tells MAME to draw a new frame before throttling to reduce input
    latency.  This is particularly effective with VRR (Variable Refresh Rate)
    displays.

    This may cause frame pacing issues in the form of jitter with some
    systems (especially newer 3D-based systems or systems that run software
    akin to an operating system), so the default is off (**-nolowlatency**).

    Example:
        .. code-block:: bash

            mame bgaregga -lowlatency


.. _mame-commandline-rotation:

Core Rotation Options
---------------------

.. _mame-commandline-norotate:

**-[no]rotate**

    Rotate the system to match its normal state (horizontal/vertical).  This
    ensures that both vertically and horizontally oriented systems show up
    correctly without the need to rotate your monitor.  If you want to keep the
    system displaying 'raw' on the screen the way it would have in the arcade,
    turn this option OFF.

    The default is ON (**-rotate**).

    Example:
        .. code-block:: bash

            mame pacman -norotate

.. _mame-commandline-noror:

**-[no]ror**

.. _mame-commandline-norol:

**-[no]rol**

    Rotate the system screen to the right (clockwise) or left
    (counter-clockwise) relative to either its normal state
    (if **-rotate** is specified) or its native state (if **-norotate** is
    specified).

    The default for both of these options is OFF (**-noror -norol**).

    Example 1:
        .. code-block:: bash

            mame pacman -ror

    Example 2:
        .. code-block:: bash

            mame pacman -rol


.. _mame-commandline-noautoror:

**-[no]autoror**

.. _mame-commandline-noautorol:

**-[no]autorol**


    These options are designed for use with pivoting screens that only pivot in
    a single direction.  If your screen only pivots clockwise, use **-autorol**
    to ensure that the system will fill the screen either horizontally or
    vertically in one of the directions you can handle.  If your screen only
    pivots counter-clockwise, use **-autoror**.

    Example 1:
        .. code-block:: bash

            mame pacman -autoror

    Example 2:
        .. code-block:: bash

            mame pacman -autorol

.. Tip:: If you have a display that can be rotated, **-autorol** or
         **-autoror** will allow you to get a larger display for both horizontal
         and vertical systems.

.. _mame-commandline-noflipx:

**-[no]flipx**

.. _mame-commandline-noflipy:

**-[no]flipy**

    Flip (mirror) the system screen either horizontally (**-flipx**) or
    vertically (**-flipy**). The flips are applied after the **-rotate** and
    **-ror**/**-rol** options are applied.

    The default for both of these options is OFF (**-noflipx -noflipy**).

    Example 1:
        .. code-block:: bash

            mame -flipx pacman

    Example 2:
        .. code-block:: bash

            mame -flipy suprmrio


.. _mame-commandline-videooptions:

Core Video Options
------------------

.. _mame-commandline-video:

**-video** *<bgfx|gdi|d3d|opengl|soft|accel|none>*

    Specifies which video subsystem to use for drawing. Options here depend on
    the operating system and whether this is an SDL-compiled version of MAME.

    Generally Available:

      |	Using ``bgfx`` specifies the new hardware accelerated renderer.
      |
      |	Using ``opengl`` tells MAME to render video using OpenGL acceleration.
      |
      |	Using ``none`` displays no windows and does no drawing.  This is primarily present for doing CPU benchmarks without the overhead of the video system.
      |

    On Windows:

      |	Using ``gdi`` tells MAME to render video using older standard Windows graphics drawing calls.  This is the slowest but most compatible option on older versions of Windows.
      |
      |	Using ``d3d`` tells MAME to use Direct3D for rendering.  This produces the better quality output than ``gdi`` and enables additional rendering options.  It is recommended if you have a semi-recent (2002+) video card or onboard Intel video of the HD3000 line or better.
      |

    On other platforms (including SDL on Windows):

      |	Using ``accel`` tells MAME to render video using SDL's 2D acceleration if possible.
      |
      |	Using ``soft`` uses software rendering for video output.  This isn't as fast or as nice as OpenGL but will work on any platform.
      |

    Defaults:

      |	The default on Windows is ``d3d``.
      |
      |	The default for Mac OS X is ``opengl`` because OS X is guaranteed to have a compliant OpenGL stack.
      |
      |	The default on all other systems is ``soft``.
      |

    Example:
        .. code-block:: bash

            mame gradius3 -video bgfx

.. _mame-commandline-numscreens:

**-numscreens** *<count>*

    Tells MAME how many output windows or screens to create.  For most systems,
    a single output window is all you need, but some systems originally used
    multiple screens (*e.g. Darius and PlayChoice-10 arcade machines*).  Some
    systems with front panel controls and/or status lights also may let you put
    these in different windows/screens. Each screen (up to 4) has its own
    independent settings for physical monitor, aspect ratio, resolution, and
    view, which can be set using the options below.

    The default is ``1``.

    Example 1:
        .. code-block:: bash

            mame darius -numscreens 3

    Example 2:
        .. code-block:: bash

            mame pc_cntra -numscreens 2

.. _mame-commandline-window:

**-[no]window** / **-[no]w**

    Run MAME in either a window or full screen.

    The default is OFF (**-nowindow**).

    Example:
        .. code-block:: bash

            mame coco3 -window

.. _mame-commandline-maximize:

**-[no]maximize** / **-[no]max**

    Controls initial window size in windowed mode.  If it is set on, the window
    will initially be set to the maximum supported size when you start MAME.  If
    it is turned off, the window will start out at the closest possible size to
    the original size of the display; it will scale on only one axis where
    non-square pixels are used. This option only has an effect when the
    **-window** option is used.

    The default is ON (**-maximize**).

    Example:
        .. code-block:: bash

            mame apple2e -window -nomaximize

.. _mame-commandline-keepaspect:

**-[no]keepaspect** / **-[no]ka**

    When enabled, MAME preserves the correct aspect ratio for the emulated
    system's screen(s).  This is most often 4:3 or 3:4 for CRT monitors
    (depending on the orientation), though many other aspect ratios have been
    used, such as 3:2 (Nintendo Game Boy), 5:4 (some workstations), and various
    other ratios.  If the emulated screen and/or artwork does not fill MAME's
    screen or Window, the image will be centred and black bars will be added
    as necessary to fill unused space (either above/below or to the left and
    right).

    When this option is disabled, the emulated screen and/or artwork will be
    stretched to fill MAME's screen or window.  The image will be distorted by
    non-proportional scaling if the aspect ratio does not match.  This is very
    pronounced when the emulated system uses a vertically-oriented screen and
    MAME stretches the image to fill a horizontally-oriented screen.

    On Windows, when this option is enabled and MAME is running in a window (not
    full screen), the aspect ratio will be maintained when you resize the window
    unless you hold the **Control** (or **Ctrl**) key on your keyboard.  The
    window size will not be restricted when this option is disabled.

    The default is ON (**-keepaspect**).

    The MAME team strongly recommends leaving this option enabled.  Stretching
    systems beyond their original aspect ratio will mangle the appearance of the
    system in ways that no filtering or shaders can repair.

    Example:
        .. code-block:: bash

            mame sf2ua -nokeepaspect

.. _mame-commandline-waitvsync:

**-[no]waitvsync**

    Waits for the refresh period on your computer's monitor to finish before
    starting to draw video to your screen.  If this option is off, MAME will
    just draw to the screen as a frame is ready, even if in the middle of a
    refresh cycle.  This can cause "tearing" artifacts, where the top portion of
    the screen is out of sync with the bottom portion.

    The effect of turning **-waitvsync** on differs a bit between combinations
    of different operating systems and video drivers.

    On Windows, **-waitvsync** will block until video blanking before allowing
    MAME to draw the next frame, limiting the emulated machine's framerate to
    that of the host display. Note that this option does not work with
    **-video gdi** mode in Windows.

    On macOS, **-waitvsync** does not block; instead the most recent completely
    drawn frame will be displayed at vblank. This means that if an emulated
    system has a higher framerate than your host display, emulated frames will
    be dropped periodically resulting in motion judder.

    On Windows, you should only need to turn this on in windowed mode. In full
    screen mode, it is only needed if **-triplebuffer** does not remove the
    tearing, in which case you should use **-notriplebuffer -waitvsync**.

    Note that SDL-based MAME support for this option depends entirely on your
    operating system and video drivers; in general it will not work in windowed
    mode so **-video opengl** and fullscreen give the greatest chance of
    success with SDL builds of MAME.

    The default is OFF (**-nowaitvsync**).

    Example:
        .. code-block:: bash

            mame gradius2 -waitvsync

.. _mame-commandline-syncrefresh:

**-[no]syncrefresh**

    Enables speed throttling only to the refresh of your monitor.  This means
    that the system's actual refresh rate is ignored; however, the sound code
    still attempts to keep up with the system's original refresh rate, so you
    may encounter sound problems.

    This option is intended mainly for those who have tweaked their video card's
    settings to provide carefully matched refresh rate options.  Note that this
    option does not work with **-video gdi** mode.

    The default is OFF (**-nosyncrefresh**).

.. _mame-commandline-prescale:

    Example:
        .. code-block:: bash

            mame mk -syncrefresh

**-prescale** *<amount>*

    Controls the size of the screen images when they are passed off to the
    graphics system for scaling.  At the minimum setting of 1, the screen is
    rendered at its original resolution before being scaled.  At higher
    settings, the screen is expanded in both axes by a factor of *<amount>*
    using nearest-neighbor sampling before applying filters or shaders.  With
    **-video d3d**, this produces a less blurry image at the expense of speed.

    The default is ``1``.

    This is supported with all video output types ( ``bgfx``, ``d3d``, etc) on
    Windows and is supported with BGFX and OpenGL on other platforms.

    Example:
        .. code-block:: bash

            mame pacman -video d3d -prescale 3

.. _mame-commandline-filter:

**-[no]filter** / **-[no]d3dfilter** / **-[no]flt**

    Enable bilinear filtering on the system screen graphics.  When disabled,
    point filtering is applied, which is crisper but leads to scaling artifacts.
    If you don't like the filtered look, you are probably better off increasing
    the **-prescale** value rather than turning off filtering altogether.

    The default is ON (**-filter**).

    This is supported with OpenGL and D3D video on Windows and is **ONLY**
    supported with OpenGL on other platforms.

    Use ``bgfx_screen_chains`` in your INI file(s) to adjust filtering with the
    BGFX video system.

    Example:
        .. code-block:: bash

            mame pacman -nofilter

.. _mame-commandline-unevenstretch:

**-[no]unevenstretch**

    Allow non-integer scaling factors allowing for great window sizing
    flexability.

    The default is ON. (**-unevenstretch**)

    Example:
        .. code-block:: bash

            mame dkong -nounevenstretch


.. _mame-commandline-fullscreen:

Core Full Screen Options
------------------------

.. _mame-commandline-switchres:

**-[no]switchres**

    Enables resolution switching. This option is required for the
    **-resolution\*** options to switch resolutions in full screen mode.

    On modern video cards, there is little reason to switch resolutions unless
    you are trying to achieve the "exact" pixel resolutions of the original
    systems, which requires significant tweaking.  This is also true
    on LCD displays, since they run with a fixed resolution and switching
    resolutions on them is just silly.  This option does not work with
    **-video gdi** and **-video bgfx**.

    The default is OFF (**-noswitchres**).

    Example:
        .. code-block:: bash

            mame kof97 -video d3d -switchres -resolution 1280x1024


.. _mame-commandline-windowoptions:

Core Per-Window Options
-----------------------

.. _mame-commandline-screen:

**-screen** *<display>*

**-screen0** *<display>*

**-screen1** *<display>*

**-screen2** *<display>*

**-screen3** *<display>*


    Specifies which physical monitor on your system you wish to have each window
    use by default.  In order to use multiple windows, you must have increased
    the value of the **-numscreens** option.  The name of each display in your
    system can be determined by running MAME with the -verbose option.  The
    display names are typically in the format of: ``\\\\.\\DISPLAYn``, where
    'n' is a number from 1 to the number of connected monitors.

    The default value for these options is ``auto``, which means that the first
    window is placed on the first display, the second window on the second
    display, etc.

    The **-screen0**, **-screen1**, **-screen2**, **-screen3** parameters apply
    to the specific window. The **-screen** parameter applies to all windows.
    The window-specific options override values from the all window option.

    Example 1:
        .. code-block:: bash

            mame pc_cntra -numscreens 2 -screen0 \\.\DISPLAY1 -screen1 \\.\DISPLAY2

    Example 2:
        .. code-block:: bash

            mame darius -numscreens 3 -screen0 \\.\DISPLAY1 -screen1 \\.\DISPLAY3 -screen2 \\.\DISPLAY2

.. Tip:: Using **-verbose** will tell you which displays you have on your
         system, where they are connected, and what their current resolutions
         are.

.. Tip:: **Multiple Screens may fail to work correctly on some Mac machines as
         of right now.**

.. _mame-commandline-aspect:

**-aspect** *<width:height>* / **-screen_aspect** *<num:den>*

**-aspect0** *<width:height>*

**-aspect1** *<width:height>*

**-aspect2** *<width:height>*

**-aspect3** *<width:height>*


    Specifies the physical aspect ratio of the physical monitor for each window.
    In order to use multiple windows, you must have increased the value of the
    **-numscreens** option.  The physical aspect ratio can be determined by
    measuring the width and height of the visible screen image and specifying
    them separated by a colon.

    The default value for these options is ``auto``, which means that MAME
    assumes the aspect ratio is proportional to the number of pixels in the
    desktop video mode for each monitor.

    The **-aspect0**, **-aspect1**, **-aspect2**, **-aspect3** parameters apply
    to the specific window.  The **-aspect** parameter applies to all windows.
    The window-specific options override values from the all window option.

    Example 1:
        .. code-block:: bash

            mame contra -aspect 16:9

    Example 2:
        .. code-block:: bash

            mame pc_cntra -numscreens 2 -aspect0 16:9 -aspect1 5:4


.. _mame-commandline-resolution:

**-resolution** *<widthxheight[@refresh]>* / **-r** *<widthxheight[@refresh]>*

**-resolution0** *<widthxheight[@refresh]>* / **-r0** *<widthxheight[@refresh]>*

**-resolution1** *<widthxheight[@refresh]>* / **-r1** *<widthxheight[@refresh]>*

**-resolution2** *<widthxheight[@refresh]>* / **-r2** *<widthxheight[@refresh]>*

**-resolution3** *<widthxheight[@refresh]>* / **-r3** *<widthxheight[@refresh]>*

    Specifies an exact resolution to run in.  In full screen mode, MAME will try
    to use the specific resolution you request.  The width and height are
    required; the refresh rate is optional.  If omitted or set to 0, MAME will
    determine the mode automatically.  For example, **-resolution 640x480** will
    force 640x480 resolution, but MAME is free to choose the refresh rate.
    Similarly, **-resolution 0x0@60** will force a 60Hz refresh rate, but allows
    MAME to choose the resolution.  The string ``auto`` is also supported, and
    is equivalent to ``0x0@0``.

    In window mode, this resolution is used as a maximum size for the window.
    This option requires the **-switchres** option as well in order to actually
    enable resolution switching with **-video d3d**.

    The default value for these options is ``auto``.

    The **-resolution0**, **-resolution1**, **-resolution2**, **-resolution3**
    parameters apply to the specific window. The **-resolution** parameter
    applies to all windows.  The window-specific options override values from
    the all window option.

    Example:
        .. code-block:: bash

            mame pc_cntra -numscreens 2 -resolution0 1920x1080 -resolution1 1280x1024

.. _mame-commandline-view:

**-view** *<viewname>*

**-view0** *<viewname>*

**-view1** *<viewname>*

**-view2** *<viewname>*

**-view3** *<viewname>*

    Specifies the initial view setting for each window/screen.  The *<viewname>*
    does not need to be the full name of a view, MAME will choose the first view
    with a name that has the *<viewname>* as a prefix.  For example
    **-view "screen 0 pixel"** will match the
    “\ *Screen 0 Pixel Aspect (10:7)*\ ” view.

    If the *<viewname>* is ``auto`` or an empty string, MAME will select views
    based on the number of emulated screens in the system, the number of
    windows/screens MAME is using, and the available external and internal
    artwork.  MAME tries to select views so that all emulated screens are
    visible by default.

    The default value for these options is ``auto``.

    The **-view0**, **-view1**, **-view2**, **-view3** parameters apply to the
    specific window.  The **-view** parameter applies to all windows.  The
    window-specific options override values from the all windows option.

    Note that view settings saved in the configuration file for the machine take
    precedence over the initial view settings.  If you change the selected views
    in the Video Options menu, this will be saved in the configuration file for
    the machine and take precedence over any initial views specified in INI
    files or on the command line.

    Example:
        .. code-block:: bash

            mame contra -view native


.. _mame-commandline-artwork:

Core Artwork Options
--------------------

.. _mame-commandline-noartworkcrop:

**-[no]artwork_crop** / **-[no]artcrop**

    Enable cropping of artwork to the system screen area only.  This means that
    vertically oriented systems running full screen can display their artwork to
    the left and right sides of the screen.  This option can also be controlled
    via the Video Options menu in the user interface.

    The default is OFF **-noartwork_crop**.

    Example:
        .. code-block:: bash

            mame pacman -artwork_crop

.. Tip:: **-artwork_crop** is great for widescreen displays. You will get a
         full-sized system display and the artwork will fill the empty space on
         the sides as much as possible.

.. _mame-commandline-fallbackartwork:

**-fallback_artwork**

    Specifies fallback artwork if no external artwork or internal driver layout
    is defined. If external artwork for the system is present or a layout is
    included in the driver for the system, then that will take precedence.

    Example:
        .. code-block:: bash

            mame coco -fallback_artwork suprmrio

.. Tip:: You can use **fallback_artwork <artwork name>** in
         ``horizontal.ini`` and ``vertical.ini`` to specify different
         fallback artwork choices for horizontal and vertical systems.


.. _mame-commandline-overrideartwork:

**-override_artwork**

    Specifies override artwork for external artwork and internal driver layout.

    Example:
        .. code-block:: bash

            mame galaga -override_artwork puckman


.. _mame-commandline-screenoptions:

Core Screen Options
-------------------

.. _mame-commandline-brightness:

**-brightness** *<value>*

    Controls the default brightness, or black level, of the system screens.
    This option does not affect the artwork or other parts of the display.
    Using the MAME UI, you can individually set the brightness for each system
    screen; this option controls the initial value for all visible system
    screens. The standard and default value is ``1.0``.  Selecting lower values
    (down to 0.1) will produce a darkened display, while selecting higher values
    (up to 2.0) will give a brighter display.

    Example:
        .. code-block:: bash

            mame pacman -brightness 0.5

.. _mame-commandline-contrast:

**-contrast** *<value>*

    Controls the contrast, or white level, of the system screens. This option
    does not affect the artwork or other parts of the display.  Using the MAME
    UI, you can individually set the contrast for each system screen; this
    option controls the initial value for all visible system screens.  The
    standard and default value is ``1.0``.  Selecting lower values (down to 0.1)
    will produce a dimmer display, while selecting higher values (up to 2.0)
    will give a more saturated display.

    Example:
        .. code-block:: bash

            mame pacman -contrast 0.5

.. _mame-commandline-gamma:

**-gamma** *<value>*

    Controls the gamma, which produces a potentially nonlinear black to white
    ramp, for the system screens.  This option does not affect the artwork or
    other parts of the display.  Using the MAME UI, you can individually set
    the gamma for each system screen; this option controls the initial value for
    all visible system screens.  The standard and default value is ``1.0``,
    which gives a linear ramp from black to white.  Selecting lower values
    (down to 0.1) will increase the nonlinearity toward black, while
    selecting higher values (up to 3.0) will push the nonlinearity toward white.

    The default is ``1.0``.

    Example:
        .. code-block:: bash

            mame pacman -gamma 0.8

.. _mame-commandline-pausebrightness:

**-pause_brightness** *<value>*

    This controls the brightness level when MAME is paused.

    The default value is ``0.65``.

    Example:
        .. code-block:: bash

            mame pacman -pause_brightness 0.33

.. _mame-commandline-effect:

**-effect** *<filename>*

    Specifies a single PNG file that is used as an overlay over any system
    screens in the video display.  This PNG file is assumed to live in the root
    of one of the artpath directories.  The pattern in the PNG file is repeated
    both horizontally and vertically to cover the entire system screen areas
    (but not any external artwork), and is rendered at the target resolution of
    the system image.

    For **-video gdi** and **-video d3d** modes, this means that one pixel in
    the PNG will map to one pixel on your output display.  The RGB values of
    each pixel in the PNG are multiplied against the RGB values of the target
    screen.

    The default is ``none``, meaning no effect.

    Example:
        .. code-block:: bash

            mame pacman -effect scanlines


.. _mame-commandline-vectoroptions:

Core Vector Options
-------------------

.. _mame-commandline-beamwidthmin:

**-beam_width_min** *<width>*

    Sets the vector beam minimum width. The beam width varies between
    the minimum and maximum beam widths as the intensity of the vector
    drawn changes. To disable vector width changes based on intensity,
    set the maximum equal to the minimum.

    Example:
        .. code-block:: bash

            mame asteroid -beam_width_min 0.1

.. _mame-commandline-beamwidthmax:

**-beam_width_max** *<width>*

    Sets the vector beam maximum width. The beam width varies between
    the minimum and maximum beam widths as the intensity of the vector
    drawn changes. To disable vector width changes based on intensity,
    set the maximum equal to the minimum.

    Example:
        .. code-block:: bash

            mame asteroid -beam_width_max 2

.. _mame-commandline-beamintensityweight:

**-beam_intensity_weight** *<weight>*

    Sets the vector beam intensity weight. This value determines how the
    intensity of the vector drawn affects the width. A value of 0 creates
    a linear mapping from intensity to width. Negative values mean that
    lower intensities will increase the width toward maximum faster,
    while positive values will increase the width toward maximum more
    slowly.

    Example:
        .. code-block:: bash

            mame asteroid -beam_intensity_weight 0.5

.. _mame-commandline-beamdotsize:

**-beam_dot_size** *<scale>*

    Scale factor to apply to the size of single-point dots in vector games.
    Normally these are rendered according to the computed beam width; however,
    it is common for this to produce dots that are difficult to see. The
    beam_dot_size option applies a scale factor on top of the beam width to
    help them show up better.

    The default is ``1``.

    Example:
        .. code-block:: bash

            mame asteroid -beam_dot_size 2

.. _mame-commandline-flicker:

**-flicker** *<value>*

    Simulates a vector "flicker" effect, similar to a vector monitor that needs
    adjustment.  This option requires a float argument in the range of
    0.00 - 100.00 (0=none, 100=maximum).

    The default is ``0``.

    Example:
        .. code-block:: bash

            mame asteroid -flicker 0.15


.. _mame-commandline-opengloptions:

Core Video OpenGL Feature Options
---------------------------------

These options are for compatibility in **-video opengl**.  If you report
rendering artifacts you may be asked to try messing with them by the developers,
but normally they should be left at their defaults which results in the best
possible video performance.

.. Tip:: Examples are not provided for these options as MAMEdev will provide
         suitable test options in the case of needing them for debugging.

.. _mame-commandline-glforcepow2texture:

**-[no]gl_forcepow2texture**

    Always use only power-of-2 sized textures.

    The default is OFF. (**-nogl_forcepow2texture**)

.. _mame-commandline-glnotexturerect:

**-[no]gl_notexturerect**

    Don't use OpenGL GL_ARB_texture_rectangle.

    The default is ON. (**-gl_notexturerect**)

.. _mame-commandline-glvbo:

**-[no]gl_vbo**

    Enable OpenGL VBO (Vertex Buffer Objects), if available.

    The default is ON. (**-gl_vbo**)

.. _mame-commandline-glpbo:

**-[no]gl_pbo**

    Enable OpenGL PBO (Pixel Buffer Objects), if available (default ``on`` )

    The default is ON. (**-gl_pbo**)


.. _mame-commandline-openglglsl:

Core Video OpenGL GLSL Options
------------------------------

.. _mame-commandline-glglsl:

**-[no]gl_glsl**

    Enable OpenGL GLSL, if available.

    The default is OFF (**-nogl_glsl**).

    Example:
        .. code-block:: bash

            mame galaxian -gl_glsl

.. _mame-commandline-glglslfilter:

**-gl_glsl_filter**

    Use OpenGL GLSL shader-based filtering instead of fixed function
    pipeline-based filtering.

    *0-plain, 1-bilinear, 2-bicubic*

    The default is 1. (**-gl_glsl_filter 1**)

    Example:
        .. code-block:: bash

            mame galaxian -gl_glsl -gl_glsl_filter 0

.. _mame-commandline-glslshadermame:

**-glsl_shader_mame0**

**-glsl_shader_mame1**

...

**-glsl_shader_mame9**

    Set a custom OpenGL GLSL shader effect to the internal systcm screen in the
    given slot. MAME does not include a vast selection of shaders by default;
    more can be found online.

    Example:
        .. code-block:: bash

            mame suprmrio -gl_glsl -glsl_shader_mame0 NTSC/NTSC_chain -glsl_shader_mame1 CRT-geom/CRT-geom

.. _mame-commandline-glslshaderscreen:

**-glsl_shader_screen0**

**-glsl_shader_screen1**

...

**-glsl_shader_screen9**


    Set a custom OpenGL GLSL shader effect to the whole scaled-up output screen
    that will be rendered by your graphics card.MAME does not include a vast
    selection of shaders by default; more can be found online.

    Example:
        .. code-block:: bash

            mame suprmrio -gl_glsl -glsl_shader_screen0 gaussx -glsl_shader_screen1 gaussy -glsl_shader_screen2 CRT-geom-halation


.. _mame-commandline-soundoptions:

Core Sound Options
------------------

.. _mame-commandline-samplerate:

**-samplerate** *<value>* / **-sr** *<value>*

    Sets the audio sample rate.  Smaller values (e.g. 11025) cause lower audio
    quality but faster emulation speed.  Higher values (e.g. 48000) cause higher
    audio quality but slower emulation speed.

    The default is ``48000``.

    Example:
        .. code-block:: bash

            mame galaga -samplerate 44100

.. _mame-commandline-nosamples:

**-[no]samples**

    Use samples if available.

    The default is ON (**-samples**).

    Example:
        .. code-block:: bash

            mame qbert -nosamples

.. _mame-commandline-nocompressor:

**-[no]compressor**

    Enable audio compressor. It temporarily reduces the overall volume when
    the audio output is overdriven.

    The default is ON (**-compressor**).

    Example:
        .. code-block:: bash

            mame popeye -nocompressor

.. _mame-commandline-volume:

**-volume** / **-vol** *<value>*

    Sets the startup volume. It can later be changed with the user interface
    (see Keys section).  The volume is an attenuation in dB: e.g.,
    "**-volume -12**" will start with -12dB attenuation.

    The default is ``0``.

    Example:
        .. code-block:: bash

            mame pacman -volume -30

.. _mame-commandline-sound:

**-sound** *<dsound | coreaudio | sdl | xaudio2 | portaudio | none>*

    Specifies which sound subsystem to use. Selecting ``none`` disables sound
    output altogether (sound hardware is still emulated).

    On Windows and Linux, *portaudio* is likely to give the lowest possible
    latency, while Mac users will find *coreaudio* provides the best results.

    When using the ``sdl`` sound subsystem, the audio API to use may be selected
    by setting the *SDL_AUDIODRIVER* environment variable.  Available audio APIs
    depend on the operating system.  On Windows, it may be necessary to set
    ``SDL_AUDIODRIVER=directsound`` if no sound output is produced by default.

    The default is ``dsound`` on Windows. On Mac, ``coreaudio`` is the default.
    On all other platforms, ``sdl`` is the default.

    Example:
        .. code-block:: bash

            mame pacman -sound portaudio

.. list-table:: Supported sound subsystems per-platform
    :header-rows: 0
    :stub-columns: 0

    * - **Microsoft Windows**
      - dsound
      - xaudio2
      - portaudio
      -
      - sdl [#SoundWinSDL]_.
      - none
    * - **macOS**
      -
      -
      - portaudio
      - coreaudio
      - sdl
      - none
    * - **Linux** and others
      -
      -
      - portaudio
      -
      - sdl
      - none


..  rubric:: Footnotes

..  [#SoundWinSDL] While SDL is not a supported option on official builds for Windows, you can compile MAME with SDL support on Windows.

.. _mame-commandline-audiolatency:

**-audio_latency** *<value>*

    The exact behavior depends on the selected audio output module.  Smaller
    values provide less audio delay while requiring better system performance.
    Higher values increase audio delay but may help avoid buffer under-runs and
    audio interruptions.

    The default is ``1``.

    | For PortAudio, see the section on :ref:`-pa_latency <mame-commandline-palatency>`.
    | XAudio2 calculates audio_latency as 10ms steps.
    | DSound calculates audio_latency as 10ms steps.
    | CoreAudio calculates audio_latency as 25ms steps.
    | SDL calculates audio_latency as Xms steps.

    Example:
        .. code-block:: bash

            mame galaga -audio_latency 1


.. _mame-commandline-inputoptions:

Core Input Options
------------------

.. _mame-commandline-nocoinlockout:

**-[no]coin_lockout** / **-[no]coinlock**

    Enables simulation of the "coin lockout" feature that is implemented on a
    number of arcade game PCBs.  It was up to the operator whether or not the
    coin lockout outputs were actually connected to the coin mechanisms.  If
    this feature is enabled, then attempts to enter a coin while the lockout is
    active will fail and will display a popup message in the user interface
    (in debug mode).  If this feature is disabled, the coin lockout signal will
    be ignored.

    The default is ON (**-coin_lockout**).

    Example:
        .. code-block:: bash

            mame suprmrio -coin_lockout

.. _mame-commandline-ctrlr:

**-ctrlr** *<controller>*

    Specifies a controller configuration file, typically used to set more
    suitable default input assignments for special controllers. Directories
    specified using the ``ctrlrpath`` option are searched.  Controller
    configuration files use a similar format to ``.cfg`` used to save system
    settings. See :ref:`ctrlrcfg` for more details.

    The default is ``NULL`` (no controller configuration file).

    Example:
        .. code-block:: bash

            mame dkong -ctrlr xarcade

.. _mame-commandline-nomouse:

**-[no]mouse**

    Controls whether or not MAME makes use of mouse controllers.  When this is
    enabled, you will likely be unable to use your mouse for other purposes
    until you exit or pause the system.

    The default is OFF (**-nomouse**).

    Example:
        .. code-block:: bash

            mame centiped -mouse

.. _mame-commandline-nojoystick:

**-[no]joystick** / **-[no]joy**

    Controls whether or not MAME makes use of joystick/gamepad controllers.

    When this is enabled, MAME will ask the system about which controllers are
    connected.

    The default is OFF (**-nojoystick**).

    Example:
        .. code-block:: bash

            mame mappy -joystick

.. _mame-commandline-nolightgun:

**-[no]lightgun** / **-[no]gun**

    Controls whether or not MAME makes use of lightgun controllers.  Note that
    most lightguns map to the mouse, so using **-lightgun** and **-mouse**
    together may produce strange results.

    The default is OFF (**-nolightgun**).

    Example:
        .. code-block:: bash

            mame lethalen -lightgun

.. _mame-commandline-nomultikeyboard:

**-[no]multikeyboard** / **-[no]multikey**

    Determines whether MAME differentiates between multiple keyboards.  Some
    systems may report more than one keyboard; by default, the data from all of
    these keyboards is combined so that it looks like a single keyboard.

    Turning this option on will enable MAME to report keypresses on different
    keyboards independently.

    The default is OFF (**-nomultikeyboard**).

    Example:
        .. code-block:: bash

            mame sf2ceua -multikey

.. _mame-commandline-nomultimouse:

**-[no]multimouse**

    Determines whether MAME differentiates between multiple mice.  Some systems
    may report more than one mouse device; by default, the data from all of
    these mice is combined so that it looks like a single mouse.  Turning this
    option on will enable MAME to report mouse movement and button presses on
    different mice independently.

    The default is OFF (**-nomultimouse**).

    Example:
        .. code-block:: bash

            mame warlords -multimouse

.. _mame-commandline-nosteadykey:

**-[no]steadykey** / **-[no]steady**

    Some systems require two or more buttons to be pressed at exactly the same
    time to make special moves.  Due to limitations in the keyboard hardware,
    it can be difficult or even impossible to accomplish that using the standard
    keyboard handling.  This option selects a different handling that makes it
    easier to register simultaneous button presses, but has the disadvantage of
    making controls less responsive.

    The default is OFF (**-nosteadykey**)

    Example:
        .. code-block:: bash

            mame sf2ua -steadykey

.. _mame-commandline-uiactive:

**-[no]ui_active**

    Enable user interface on top of emulated keyboard (if present).

    The default is OFF (**-noui_active**)

    Example:
        .. code-block:: bash

            mame apple2e -ui_active

.. _mame-commandline-nooffscreenreload:

**-[no]offscreen_reload** / **-[no]reload**

    Controls whether or not MAME treats a second button input from a lightgun as
    a reload signal.  In this case, MAME will report the gun's position as
    (0,MAX) with the trigger held, which is equivalent to an offscreen reload.

    This is only needed for games that required you to shoot offscreen to
    reload, and then only if your gun does not support off screen reloads.

    The default is OFF (**-nooffscreen_reload**).

    Example:
        .. code-block:: bash

            mame lethalen -offscreen_reload

.. _mame-commandline-joystickmap:

**-joystick_map** *<map>* / **-joymap** *<map>*

    Controls how analog joystick values map to digital joystick controls.

    Systems such as Pac-Man use a 4-way digital joystick and will exhibit
    undesired behavior when a diagonal is triggered; in the case of Pac-Man,
    movement will stop completely at intersections when diagonals are triggered
    and the game will be considerably harder to play correctly.  Many other
    arcade cabinets used 4-way or 8-way joysticks (as opposed to full analog
    joysticks), so for true analog joysticks such as flight sticks and analog
    thumb sticks, this then needs to be mapped down to the expected
    4-way or 8-way digital joystick values.

    To do this, MAME divides the analog range into a 9x9 grid that looks
    like this:

    **insert 9x9 grid picture here**

    MAME then takes the joystick axis position (for X and Y axes only), maps it
    to this grid, and then looks up a translation from a joystick map.  This
    parameter allows you to specify the map.

    For instance, an 8-way joystick map traditionally looks like this:

    **insert 8-way map picture here**

    This mapping gives considerable leeway to the angles accepted for a given
    direction, so that being approximately in the area of the direction you want
    will give you the results you want.  Without that, if you were slightly off
    center while holding the stick left, it would not recognize the action
    correctly.

    The default is ``auto``, which means that a standard 8-way, 4-way, or 4-way
    diagonal map is selected automatically based on the input port configuration
    of the current system.

    Generally you will want to set up the **-joystick_map** setting in the
    per-system ``<system>.ini`` file as opposed to the main ``MAME.INI``
    file so that the mapping only affects the systems you want it to.  See
    :ref:`Multiple Configuration Files <advanced-multi-CFG>` for further
    details on per-system configuration.

    Maps are defined as a string of numbers and characters. Since the grid is
    9x9, there are a total of 81 characters necessary to define a complete
    map.  Below is an example map for an 8-way joystick that matches the
    picture shown above:

		+-------------+--------------------------------------------------------+
		| | 777888999 |                                                        |
		| | 777888999 | | Note that the numeric digits correspond to the keys  |
		| | 777888999 | | on a numeric keypad. So '7' maps to up+left, '4' maps|
		| | 444555666 | | to left, '5' maps to neutral, etc. In addition to the|
		| | 444555666 | | numeric values, you can specify the character 's',   |
		| | 444555666 | | which means "sticky".  Sticky map positions will keep|
		| | 111222333 | | the output the same as the last non-sticky input sent|
		| | 111222333 | | to the system.                                       |
		| | 111222333 |                                                        |
		+-------------+--------------------------------------------------------+

    To specify the map for this parameter, you can specify a string of rows
    separated by a '.' (which indicates the end of a row), like so:

    +-------------------------------------------------------------------------------------------------------+
    | **-joymap 777888999.777888999.777888999.444555666.444555666.444555666.111222333.111222333.111222333** |
    +-------------------------------------------------------------------------------------------------------+

    However, this can be reduced using several shorthands supported by the <map>
    parameter.  If information about a row is missing, then it is assumed that
    any missing data in columns 5-9 are left/right symmetric with data in
    columns 0-4; and any missing data in columns 0-4 is assumed to be copies of
    the previous data.  The same logic applies to missing rows, except that
    up/down symmetry is assumed.

    By using these shorthands, the 81 character map can be simply specified by
    this 11 character string: 7778...4445 (which means we then use
    **-joymap 7778...4445**)

    Looking at the first row, 7778 is only 4 characters long.  The 5th entry
    can't use symmetry, so it is assumed to be equal to the previous character
    '8'.  The 6th character is left/right symmetric with the 4th character,
    giving an '8'.  The 7th character is left/right symmetric with the 3rd
    character, giving a '9' (which is '7' with left/right flipped).
    Eventually this gives the full 777888999 string of the row.

    The second and third rows are missing, so they are assumed to be identical
    to the first row.  The fourth row decodes similarly to the first row,
    producing 444555666.  The fifth row is missing so it is assumed to be the
    same as the fourth.

    The remaining three rows are also missing, so they are assumed to be the
    up/down mirrors of the first three rows, giving three final rows of
    111222333.

    With 4-way games, sticky becomes important to avoid problems with
    diagonals.  Typically you would choose a map that looks something like this:

    **insert 9x9 4-way sticky grid picture here**

    This means that if you press left, then roll the stick towards up (without
    re-centering it) you'll pass through the sticky section in the corner.  As
    you do, MAME will read that sticky corner as **left** as that's the last
    non-sticky input it received.  As the roll gets into the upward space of
    the map, this then switches to an up motion.

    This map would look somewhat like:

		+-------------+---------------------------------------------------------+
		| | s8888888s |                                                         |
		| | 4s88888s6 | | For this mapping, we have a wide range for the        |
		| | 44s888s66 | | cardinal directions on 8, 4, 6, and 2.  We have sticky|
		| | 444555666 | | on the meeting points between those cardinal          |
		| | 444555666 | | directions where the appropriate direction isn't      |
		| | 444555666 | | going to be completely obvious.                       |
		| | 44s222s66 |                                                         |
		| | 4s22222s6 |                                                         |
		| | s2222222s |                                                         |
		+-------------+---------------------------------------------------------+

    To specify the map for this parameter, you can specify a string of rows
    separated by a '.' (which indicates the end of a row), like so:

    +-------------------------------------------------------------------------------------------------------+
    | **-joymap s8888888s.4s88888s6.44s888s66.444555666.444555666.444555666.44s222s66.4s22222s6.s2222222s** |
    +-------------------------------------------------------------------------------------------------------+

    Like before, because of the symmetry between top and bottom and left and
    right, we can shorten this down to:

    +------------------------------+
    | **-joymap s8.4s8.44s8.4445** |
    +------------------------------+



.. _mame-commandline-joystickdeadzone:

**-joystick_deadzone** *<value>* / **-joy_deadzone** *<value>* / **-jdz** *<value>*

  If you play with an analog joystick, the center can drift a little.
  joystick_deadzone tells how far along an axis you must move before the axis
  starts to change. This option expects a float in the range of 0.0 to 1.0.
  Where 0 is the center of the joystick and 1 is the outer limit.

  The default is ``0.3``.

    Example:
        .. code-block:: bash

            mame sinistar -joystick_deadzone 0.45

.. _mame-commandline-joysticksaturation:

**-joystick_saturation** *<value>* / **joy_saturation** *<value>* / **-jsat** *<value>*

    If you play with an analog joystick, the ends can drift a little, and may
    not match in the +/- directions.  joystick_saturation tells how far along
    an axis movement change will be accepted before it reaches the maximum
    range.  This option expects a float in the range of 0.0 to 1.0, where 0 is
    the center of the joystick and 1 is the outer limit.

    The default is ``0.85``.

    Example:
        .. code-block:: bash

            mame sinistar -joystick_saturation 1.0

.. _mame-commandline-natural:

**\-[no]natural**

    Allows user to specify whether or not to use a natural keyboard or not.
    This allows you to start your system in a 'native' mode, depending on your
    region, allowing compatability for non-"QWERTY" style keyboards.

    The default is OFF (**-nonatural**)

    In "emulated keyboard" mode (the default mode), MAME translates
    pressing/releasing host keys/buttons to emulated keystrokes.  When you
    press/release a key/button mapped to an emulated key, MAME
    presses/releases the emulated key.

    In "natural keyboard" mode, MAME attempts to translate characters to
    keystrokes.  The OS translates keystrokes to characters
    (similarly when you type into a text editor), and MAME attempts to translate
    these characters to emulated keystrokes.

    **There are a number of unavoidable limitations in "natural keyboard" mode:**

      * The emulated system driver and/or keyboard device or has to support it.
      * The selected keyboard *must* match the keyboard layout selected in the emulated OS!
      * Keystrokes that don't produce characters can't be translated. (e.g. pressing a modifier on its own such as **shift**, **ctrl**, or **alt**)
      * Holding a key until the character repeats will cause the emulated key to be pressed repeatedly as opposed to being held down.
      * Dead key sequences are cumbersome to use at best.
      * It won't work at all if IME edit is involved. (e.g. for Chinese/Japanese/Korean)

    Example:
        .. code-block:: bash

            mame coco2 -natural

.. _mame-commandline-joystickcontradictory:

**-[no]joystick_contradictory**

    Enable contradictory direction digital joystick input at the same time such
    as **Left and Right** or **Up and Down** at the same time.

    The default is OFF (**-nojoystick_contradictory**)

    Example:
        .. code-block:: bash

            mame pc_smb -joystick_contradictory

.. _mame-commandline-coinimpulse:

**-coin_impulse** *[n]*

    Set coin impulse time based on n (n<0 disable impulse, n==0 obey driver,
    0<n set time n).

    Default is ``0``.

    Example:
        .. code-block:: bash

            mame contra -coin_impulse 1


.. _mame-commandline-inputenable:

Core Input Automatic Enable Options
-----------------------------------

.. _mame-commandline-paddledevice:

**-paddle_device** ( ``none`` | ``keyboard`` | ``mouse`` | ``lightgun`` | ``joystick`` )

.. _mame-commandline-adstickdevice:

**-adstick_device** ( ``none`` | ``keyboard`` | ``mouse`` | ``lightgun`` | ``joystick`` )

.. _mame-commandline-pedaldevice:

**-pedal_device** ( ``none`` | ``keyboard`` | ``mouse`` | ```lightgun`` | ``joystick`` )

.. _mame-commandline-dialdevice:

**-dial_device** ( ``none`` | ``keyboard`` | ``mouse`` | ``lightgun`` | ``joystick`` )

.. _mame-commandline-trackballdevice:

**-trackball_device** ( ``none`` | ``keyboard`` | ``mouse`` | ``lightgun`` | ``joystick`` )

.. _mame-commandline-lightgundevice:

**-lightgun_device** ( ``none`` | ``keyboard`` | ``mouse`` | ``lightgun`` | ``joystick`` )

.. _mame-commandline-positionaldevice:

**-positional_device** ( ``none`` | ``keyboard`` | ``mouse`` | ``lightgun`` | ``joystick`` )

.. _mame-commandline-mousedevice:

**-mouse_device** ( ``none`` | ``keyboard`` | ``mouse`` | ``lightgun`` | ``joystick`` )

    Each of these options controls autoenabling the mouse, joystick, or lightgun
    depending on the presence of a particular class of analog control for a
    particular system.  For example, if you specify the option
    **-paddle mouse**, then any game that has a paddle control will
    automatically enable mouse controls just as if you had explicitly specified
    **-mouse**.

    Example:
        .. code-block:: bash

            mame sbrkout -paddle_device mouse

.. Tip:: Note that these controls override the values of **-[no]mouse**,
         **-[no]joystick**, etc.


.. _mame-commandline-debugging:

Debugging Options
-----------------

.. _mame-commandline-verbose:

**-[no]verbose** / **-[no]v**

    Displays internal diagnostic information. This information is very useful
    for debugging problems with your configuration.

    The default is OFF (**-noverbose**).

    Example:
        .. code-block:: bash

            mame polepos -verbose

.. Tip:: IMPORTANT: When reporting bugs to MAMEdev, please run with **-verbose**
         and include the resulting information.

.. _mame-commandline-oslog:

**-[no]oslog**

    Output ``error.log`` messages to the system diagnostic output, if one is
    present.

    By default messages are sent to the standard error output (this is typically
    displayed in the terminal or command prompt window, or saved to a system log
    file).  On Windows, if a debugger is attached (e.g. the Visual Studio
    debugger or WinDbg), messages will be sent to the debugger instead.

    The default is OFF (**-nooslog**).

    Example:
        .. code-block:: bash

            mame mappy -oslog

.. _mame-commandline-log:

**-[no]log**

    Creates a file called error.log which contains all of the internal log
    messages generated by the MAME core and system drivers.  This can be used at
    the same time as **-oslog** to output the log data to both targets as well.

    The default is OFF (**-nolog**).

    Example 1:
        .. code-block:: bash

            mame qbert -log

    Example 2:
        .. code-block:: bash

            mame qbert -oslog -log

.. _mame-commandline-debug:

**-[no]debug**

    Activates the integrated debugger.  By default, pressing the backtick/tilde
    (**~**) key during emulation breaks into the debugger.  MAME also breaks
    into the debugger after the initial soft reset on startup if the debugger is
    active.  See :ref:`debugger` for information on using the debugger.

    The default is OFF (**-nodebug**).

    Example:
        .. code-block:: bash

            mame indy_4610 -debug

.. _mame-commandline-debugger:

**-debugger** *<module>*

    Chooses the module to use for debugging the target system when the
    :ref:`debug <mame-commandline-debug>` option is on.  Available debugger
    modules depend on the host platform and build options.

    Supported debugger modules:

    windows
        Win32 GUI debugger (default on Windows).  Only supported on Windows.
    qt
        Qt GUI debugger (default on Linux).  Supported on Windows, Linux and
        macOS, but only included on Linux by default.  Set ``USE_QTDEBUG=1``
        when compiling MAME to include the Qt debugger on Windows or macOS.
    osx
        Cocoa GUI debugger (default on macOS).  Only supported on macOS.
    imgui
        ImgUi GUI debugger displayed in first MAME window.  Requires
        :ref:`video <mame-commandline-video>` option to be set to **bgfx**.
        Supported on all platforms with BGFX video output support.
    gdbstub
        Acts as a remote debugging server for the GNU debugger (GDB).  Only a
        small subset of the CPUs emulated by MAME are supported.  Use the
        :ref:`debugger_port <mame-commandline-debuggerport>` option to set the
        listening port on the loopback interface.  Supported on all platforms
        with TCP socket support.

    Example:
        .. code-block:: bash

            mame ambush -debug -debugger qt

.. _mame-commandline-debugscript:

**-debugscript** *<filename>*

    Specifies a file that contains a list of debugger commands to execute
    immediately upon startup.

    The default is ``NULL`` (*no commands*).

    Example:
        .. code-block:: bash

            mame galaga -debug -debugscript testscript.txt

.. _mame-commandline-updateinpause:

**-[no]update_in_pause**

    Enables updating of the main screen bitmap while the system is paused.  This
    means that the video update callback will be called repeatedly while the
    emulation is paused, which can be useful for debugging.

    The default is OFF (**-noupdate_in_pause**).

    Example:
        .. code-block:: bash

            mame indy_4610 -update_in_pause

.. _mame-commandline-watchdog:

**-watchdog** *<duration>* / **-wdog** *<duration>*

    Enables an internal watchdog timer that will automatically kill the MAME
    process if more than *<duration>* seconds passes without a frame update.
    Keep in mind that some systems sit for a while during load time without
    updating the screen, so *<duration>* should be long enough to cover that.

    10-30 seconds on a modern system should be plenty in general.

    By default there is no watchdog.

    Example:
        .. code-block:: bash

            mame ibm_5150 -watchdog 30

.. _mame-commandline-debuggerport:

**-debugger_port** *<port>*

    Set the TCP port number to listen on for GDB connections when using the GDB
    stub debugger module (see the :ref:`debugger <mame-commandline-debugger>`
    option).

    The default is ``23946``.

    Example:
        .. code-block:: bash

            mame rfjet -debug -debugger gdbstub -debugger_port 2159

.. _mame-commandline-debuggerfont:

**-debugger_font** *<fontname>* / **-dfont** *<fontname>*

    Specifies the name of the font to use for debugger windows.

    | The Windows default font is ``Lucida Console``.
    | The Mac (Cocoa) default font is system fixed-pitch font default (typically ``Monaco``).
    | The Qt default font is ``Courier New``.

    Example:
        .. code-block:: bash

            mame marble -debug -debugger_font "Comic Sans MS"

.. _mame-commandline-debuggerfontsize:

**-debugger_font_size** *<points>* / **-dfontsize** *<points>*

    Specifies the size of the font to use for debugger windows, in points.

    | The Windows default size is ``9`` points.
    | The Qt default size is ``11`` points.
    | The Mac (Cocoa) default size is the system default size.

    Example:
        .. code-block:: bash

            mame marble -debug -debugger_font "Comic Sans MS" -debugger_font_size 16


.. _mame-commandline-commoptions:

Core Communication Options
--------------------------

.. _mame-commandline-commlocalhost:

**-comm_localhost** *<string>*

    Local address to bind to. This can be a traditional ``xxx.xxx.xxx.xxx``
    address or a string containing a resolvable hostname.

    The default is value is ``0.0.0.0`` (which binds to all local IPv4
    addresses).

    Example:
        .. code-block:: bash

            mame arescue -comm_localhost 192.168.1.2

.. _mame-commandline-commlocalport:

**-comm_localport** *<string>*

    Local port to bind to. This can be any traditional communications port as
    an unsigned 16-bit integer (0-65535).

    The default value is ``15122``.

    Example:
        .. code-block:: bash

            mame arescue -comm_localhost 192.168.1.2 -comm_localport 30100

.. _mame-commandline-commremotehost:

**-comm_remotehost** *<string>*

    Remote address to connect to. This can be a traditional xxx.xxx.xxx.xxx
    address or a string containing a resolvable hostname.

    The default is value is "``0.0.0.0``" (which binds to all local IPv4
    addresses).

    Example:
        .. code-block:: bash

            mame arescue -comm_remotehost 192.168.1.2

.. _mame-commandline-commremoteport:

**-comm_remoteport** *<string>*

    Remote port to connect to. This can be any traditional communications port
    as an unsigned 16-bit integer (0-65535).

    The default value is "``15122``".

    Example:
        .. code-block:: bash

            mame arescue -comm_remotehost 192.168.1.2 -comm_remoteport 30100

.. _mame-commandline-commframesync:

**-[no]comm_framesync**

    Synchronize frames between the communications network.

    The default is OFF (**-nocomm_framesync**).

    Example:
        .. code-block:: bash

            mame arescue -comm_remotehost 192.168.1.3 -comm_remoteport 30100 -comm_framesync


.. _mame-commandline-miscoptions:

Core Misc Options
-----------------

.. _mame-commandline-drc:

**-[no]drc**

    Enable DRC (dynamic recompiler) CPU core if available for maximum speed.

    The default is ON (**-drc**).

    Example:
        .. code-block:: bash

            mame ironfort -nodrc

.. _mame-commandline-drcusec:

**\-[no]drc_use_c**

    Force DRC to use the C code backend.

    The default is OFF (**-nodrc_use_c**).

    Example:
        .. code-block:: bash

            mame ironfort -drc_use_c

.. _mame-commandline-drcloguml:

**\-[no]drc_log_uml**

    Write DRC UML disassembly log.

    The default is OFF (**-nodrc_log_uml**).

    Example:
        .. code-block:: bash

            mame ironfort -drc_log_uml

.. _mame-commandline-drclognative:

**\-[no]drc_log_native**

    Write DRC native disassembly log.

    The default is OFF (**-nodrc_log_native**).

    Example:
        .. code-block:: bash

            mame ironfort -drc_log_native

.. _mame-commandline-bios:

**-bios** *<biosname>*

    Specifies the specific BIOS to use with the current system, for systems that
    make use of a BIOS. The **-listxml** output will list all of the possible
    BIOS names for a system.

    The default is ``default``.

    Example:
        .. code-block:: bash

            mame mslug -bios unibios33

.. _mame-commandline-cheat:

**-[no]cheat** / **-[no]c**

    Activates the cheat menu with autofire options and other tricks from the
    cheat database, if present. This also activates additional options on the
    slider menu for overclocking/underclocking.

    *Be advised that savestates created with cheats on may not work correctly
    with this turned off and vice-versa.*

    The default is OFF (**-nocheat**).

    Example:
        .. code-block:: bash

            mame dkong -cheat

.. _mame-commandline-skipgameinfo:

**-[no]skip_gameinfo**

    Forces MAME to skip displaying the system info screen.

    The default is OFF (**-noskip_gameinfo**).

    Example:
        .. code-block:: bash

            mame samsho5 -skip_gameinfo

.. _mame-commandline-uifont:

**-uifont** *<fontname>*

    Specifies the name of a font file to use for the UI font. If this font
    cannot be found or cannot be loaded, the system will fall back to its
    built-in UI font. On some platforms *fontname* can be a system font name
    instead of a BDF font file.

    The default is ``default`` (use the OSD-determined default font).

    Example:
        .. code-block:: bash

            mame -uifont "Comic Sans MS"

.. _mame-commandline-ui:

**-ui** *<type>*

    Specifies the type of UI to use, either ``simple`` or ``cabinet``.

    The default is Cabinet (**-ui cabinet**).

    Example:
        .. code-block:: bash

            mame -ui simple

.. _mame-commandline-ramsize:

**-ramsize** *[n]*

    Allows you to change the default RAM size (if supported by driver).

    Example:
        .. code-block:: bash

            mame coco -ramsize 16K

.. _mame-commandline-confirmquit:

**\-[no]confirm_quit**

    Display a Confirm Quit dialog to screen on exit, requiring one extra step
    to exit MAME.

    The default is OFF (**-noconfirm_quit**).

    Example:
        .. code-block:: bash

            mame pacman -confirm_quit

.. _mame-commandline-uimouse:

**\-[no]ui_mouse**

    Displays a mouse cursor when using the built-in UI for MAME.

    The default is (**-noui_mouse**).

.. _mame-commandline-language:

    Example:
        .. code-block:: bash

            mame -ui_mouse

**-language** *<language>*

    Specify a localization language found in the ``languagepath`` tree.

    Example:
        .. code-block:: bash

            mame -language Japanese

.. _mame-commandline-nvramsave:

**-[no]nvram_save**

    Save the NVRAM contents when exiting machine emulation. By turning this off,
    you can retain your previous NVRAM contents as any current changes made will
    not be saved. Turning this option off will also unconditionally suppress the
    saving of .nv files associated with some types of software cartridges.

    The default is ON (**-nvram_save**).

    Example:
        .. code-block:: bash

            mame galaga88 -nonvram_save


.. _mame-commandline-scripting:

Scripting Options
-----------------

.. _mame-commandline-autobootcommand:

**-autoboot_command** *"<command>"*

    Command string to execute after machine boot (in quotes " "). To issue a
    quote to the emulation, use """ in the string. Using **\\n** will issue a
    create a new line, issuing what was typed prior as a command.

    This works only with systems that support natural keyboard mode.

    Example:
        .. code-block:: bash

            mame c64 -autoboot_delay 5 -autoboot_command "load """$""",8,1\n"

.. _mame-commandline-autobootdelay:

**-autoboot_delay** *[n]*

    Timer delay (in seconds) to trigger command execution on autoboot.

    Example:
        .. code-block:: bash

            mame c64 -autoboot_delay 5 -autoboot_command "load """$""",8,1\n"

.. _mame-commandline-autobootscript:

**-autoboot_script** / **-script** *[filename.lua]*

    File containing scripting to execute after machine boot.

    Example:
        .. code-block:: bash

            mame ibm5150 -autoboot_script myscript.lua

.. _mame-commandline-console:

**-[no]console**

    Enables emulator Lua Console window.

    The default of OFF (**-noconsole**).

    Example:
        .. code-block:: bash

            mame ibm5150 -console

.. _mame-commandline-plugins:

**-plugins**

    Enable the use of Lua Plugins.

    The default is ON (**-plugins**).

    Example:
        .. code-block:: bash

            mame apple2e -plugins

.. _mame-commandline-plugin:

**-plugin** *[plugin shortname]*

    A list of Lua Plugins to enable, comma separated.

    Example:
        .. code-block:: bash

            mame alcon -plugin cheat,discord,autofire

.. _mame-commandline-noplugin:

**-noplugin** *[plugin shortname]*

    A list of Lua Plugins to disable, comma separated.

    Example:
        .. code-block:: bash

            mame alcon -noplugin cheat


.. _mame-commandline-webserver:

HTTP Server Options
-------------------

.. _mame-commandline-http:

**-[no]http**

    Enable HTTP server.

    The default is OFF (**-nohttp**).

    Example:
        .. code-block:: bash

            mame -http

.. _mame-commandline-httpport:

**-http_port** *<port>*

    Choose HTTP server port.

    The default is ``8080``.

    Example:
        .. code-block:: bash

            mame apple2 -http -http_port 6502

.. _mame-commandline-httproot:

**-http_root** *<rootfolder>*

    Choose HTTP server document root.

    The default is ``web``.

    Example:
        .. code-block:: bash

            mame apple2 -http -http_port 6502 -http_root c:\users\me\appleweb\root


.. _mame-commandline-portaudio:

PortAudio Options
-----------------

.. _mame-commandline-paapi:

**-pa_api** *API*

    Choose which API that PortAudio should use to talk to your sound hardware. You can use **-verbose** to see which APIs are available.

    The default is ``none``.

    Example 1:
        .. code-block:: bash

            mame -sound portaudio -verbose
            Attempting load of mame.ini
            ...
            PortAudio: API MME has 20 devices
            PortAudio: MME: " - Input"
            PortAudio: MME: "Microphone (3- USB Camera-B4.09"
            PortAudio: MME: "Line (AVerMedia Live Gamer HD 2"
            PortAudio: MME: "Digital Audio Interface (AVerMe"
            PortAudio: MME: "Headset Microphone (Razer Krake"
            ...
            PortAudio: MME: " - Output"
            PortAudio: MME: "Headset Earphone (Razer Kraken "
            PortAudio: MME: "Digital Audio (S/PDIF) (High De"
            PortAudio: MME: "NX-EDG27 (NVIDIA High Definitio"
            ...
            PortAudio: API Windows DirectSound has 20 devices
            PortAudio: Windows DirectSound: "Primary Sound Capture Driver"
            PortAudio: Windows DirectSound: "Headset Microphone (Razer Kraken 7.1 V2)"
            PortAudio: Windows DirectSound: "Primary Sound Driver" (default)
            PortAudio: Windows DirectSound: "Headset Earphone (Razer Kraken 7.1 V2)"
            PortAudio: Windows DirectSound: "Digital Audio (S/PDIF) (High Definition Audio Device)"
            PortAudio: Windows DirectSound: "NX-EDG27 (NVIDIA High Definition Audio)"
            ...
            PortAudio: API Windows WASAPI has 18 devices
            PortAudio: Windows WASAPI: "Headset Earphone (Razer Kraken 7.1 V2)"
            PortAudio: Windows WASAPI: "Digital Audio (S/PDIF) (High Definition Audio Device)"
            PortAudio: Windows WASAPI: "NX-EDG27 (NVIDIA High Definition Audio)"
            PortAudio: Windows WASAPI: "Headset Microphone (Razer Kraken 7.1 V2)"
            ...
            PortAudio: API Windows WDM-KS has 22 devices
            PortAudio: Windows WDM-KS: "Output (NVIDIA High Definition Audio)"
            PortAudio: Windows WDM-KS: "SPDIF Out (HD Audio SPDIF out)"
            PortAudio: Windows WDM-KS: "Headset Microphone (Razer Kraken 7.1 V2)"
            PortAudio: Windows WDM-KS: "Headset Earphone (Razer Kraken 7.1 V2)"
            PortAudio: Windows WDM-KS: "Microphone (VDVAD Wave)"
            PortAudio: Windows WDM-KS: "Speakers (VDVAD Wave)"
            ...
            PortAudio: Sample rate is 48000 Hz, device output latency is 218.67 ms
            PortAudio: Allowed additional buffering latency is 18.00 ms/864 frames

    Example 2:
        .. code-block:: bash

            mame suprmrio -sound portaudio -pa_api "Windows WASAPI"

.. _mame-commandline-padevice:

**-pa_device** *device*

    Choose which sound device to output through. This would typically be one of
    the outputs on your sound card or a USB headset.

    The default is ``none``.

    Example:
        .. code-block:: bash

            mame suprmrio -sound portaudio -pa_api "Windows WASAPI" -pa_device "NX-EDG27 (NVIDIA High Definition Audio)"

.. _mame-commandline-palatency:

**-pa_latency** *latency*

    Choose the buffer size for PortAudio output; this is specified in seconds.
    Lower numbers have less latency but may increase stutter in the sound.
    Decimal places are supported. Try starting from 0.20 and decrease or
    increase until you find the best number your hardware and OS are capable of
    handling.

    The default is ``0``.

    Example:
        .. code-block:: bash

            mame suprmrio -sound portaudio -pa_api "Windows WASAPI" -pa_device "NX-EDG27 (NVIDIA High Definition Audio)" -pa_latency 0.20
