.. _assetsearch:

How does MAME look for files?
=============================

.. contents:: :local:

Introduction
------------

Unlike typical desktop applications where you browse your disk and select a file
to open or a location to save to, MAME has settings to tell it where to look for
the files it needs.  You can change these settings by starting MAME without
specifying a system, selecting **Configure Options** from the system selection
menu, and then selecting **Configure Directories** (remember to select **Save
Configuration** if you want to keep your changes).  You can also change settings
by editing your mame.ini and ui.ini files directly, or specify settings on the
command line.  For information on available options for controlling where MAME
searches for files, see :ref:`mame-commandline-pathoptions`.

Terminology
~~~~~~~~~~~

It’s necessary to understand some MAME-specific terminology used in the
explanations here:

System
    A system is a complete machine that can be emulated by MAME.  Some systems
    run fixed software, while others can load software from software list items
    and/or media files.
Device
    An emulated component that can be used by multiple systems, or by other
    devices.  Some devices require ROM dumps, and some devices allow software
    from additional software lists to be used with a system.
Parent system
    MAME uses so-called parent/clone relationships to group related systems.
    One system in the group is chosen to be the *parent* and the others are
    called *clones*.  (The choice of the parent system is somewhat arbitrary.
    It is not necessarily the original or definitive variant.)
BIOS system
    A system configured with no software.  This is mostly applicable for arcade
    systems that used interchangeable game cartridges or ROM boards.  Note that
    this is *not* the same as the BIOS selection settings that allow you to
    select system boot ROMs or device firmware.
Software item
    A software package described in a software list.  Software items may consist
    of multiple *parts* that can be mounted independently.  Due to the large
    variety of media supported by MAME, software parts may use different
    *loaders*.  These include the *ROM loader*, typically used for cartridge
    media, and the *image file loader*, used for software parts consisting of a
    single media image (including floppy disk and cassette media).
Parent software item
    Related software items are grouped using parent/clone relationships, in a
    similar way to related systems.  This is usually used to group different
    versions or releases of the same piece of software.  If a software item has
    a parent item, it will always be in the same software list.
Short name
    MAME uses *short names* to uniquely identify systems and devices, to
    uniquely identify software lists, to uniquely identify software items within
    a software list, and to uniquely identify software parts within a software
    item.

    You can see the short name for a system by highlighting it in the system
    selection menu, ensuring the info panel is visible on the right, and
    showing the **General Info** in the **Infos** tab.  For example the short
    name for the Nintendo Virtual Boy is ``vboy``.  System and device short
    names can also be seen in the output of various command line verbs,
    including ``-listxml``, ``-listfull``, ``-listroms`` and ``-listcrc``.

    You can see the short names for a software item and the software list it
    belongs to by highlighting it in the software selection menu, ensuring the
    info panel is visible on the right, and showing the **Software List Info**
    in the **Infos** tab.  For example the short name for Macintosh System
    Software 6.0.3 is ``sys603`` and the short name of the software list it
    belongs to is ``mac_flop``.  Software list short names match their file
    names (for example the Sega Mega Drive/Genesis cartridge software list is
    called **megadriv.xml** and its short name is ``megadriv``).  You can also
    see the short names software lists, software items and parts by finding the
    ``name`` attributes in the XML software list files.


Search path options
-------------------

Most options for specifying locations to search allow multiple directories to be
specified, separated by semicolon (``;``) characters.  Environment variables are
expanded, using CMD shell syntax on Windows, or Bourne shell syntax on UNIX-like
systems.

Relative paths are interpreted relative to the current working directory at the
time of use.  If you start MAME by double-clicking it in Windows Explorer, the
working directory is set to the folder containing the MAME executable.  If you
start MAME by double-clicking it in the macOS Finder or from most Linux desktop
environments, the working directory will be set to your home directory.


Archive files
-------------

MAME can load files from PKZIP and 7-Zip archives (these must have ``.zip`` and
``.7z`` file name extensions, respectively).  A number of extensions to the
PKZIP format are supported, including Zip64 for large archives, NTFS timestamps,
and LZMA compression.  Only ASCII or UTF-8 filenames are supported in PKZIP
archives (7-Zip archives always use UTF-16 filenames).

MAME *does not* load files from nested archives.  MAME will not load files
stored in a PKZIP or 7-Zip archive which is itself contained within a PKZIP or
7-Zip archive.  Multi-segment archives and encrypted archives are not supported.
The legacy “implode” compression method in PKZIP archives is not supported.

MAME may perform poorly with archives containing large numbers of files.  Files
compressed using the LZMA compression algorithm are inherently more
CPU-intensive to decompress than files compressed using simpler algorithms.
MAME does not take the archive layout into consideration when loading files from
archives, so using “solid compression” often results in MAME decompressing the
same data repeatedly when loading media.


How does MAME search for media?
-------------------------------

Use the :ref:`rompath <mame-commandline-rompath>` option sets the folders where
searches for ROM dumps, disk images, and other media.  By default MAME looks for
media in a folder called **roms** in the working directory.  For the purpose of
this discussion, floppy disk, cassette, paper tape and other media images that
are not stored in CHD format are treated as ROM dumps.

When searching for system, device and software ROM dumps, MAME treats folders
and archives inside the folders configured in you ``rompath`` setting as
equivalent, but remember the limitation that MAME cannot load files from an
archive contained within another archive.  MAME looks for a folder first, then a
PKZIP archive, and finally a 7-Zip archive.  When searching for a ROM dump in an
archive, MAME first looks for a file with the expected name and CRC.  If no
matching file is found, MAME looks for a file with the expected CRC ignoring the
name.  If no matching file is found, MAME finally looks for a file with the
expected name, ignoring the CRC.

While MAME can load disk images in CHD format from inside archives, this is not
recommended.  CHD files contain compressed data stored in a format allowing
random access.  If a CHD format disk image is stored in a PKZIP or 7-Zip
archive, MAME needs to load the entire file into memory in order to use it.  For
hard disk or LaserDisc images in particular, this will likely use an excessive
amount of swap file space, hurting performance and possibly reducing the life
expectancy of your disks or SSDs.  It’s best to keep CHD format disk images in
folders.

System ROMs
~~~~~~~~~~~

For each folder configured in your ``rompath`` setting, MAME looks for system
ROMs in the following locations:

* A folder or archive matching the short name of the system itself.
* A folder or archive matching the short name of the system’s parent system, if
  applicable.
* A folder or archive matching the short name of the corresponding BIOS system,
  if applicable.

Using Shiritsu Justice Gakuen as an example, MAME will search for system ROMs as
follows:

* The short name of the system is ``jgakuen``, so MAME will look for a folder
  called **jgakuen**, a PKZIP archive called **jgakuen.zip**, or a 7-Zip archive
  called **jgakuen.7z**.
* The parent system is the European version of Rival Schools, which has the
  short name ``rvschool``, so MAME will look for a folder called **rvschool**, a
  PKZIP archive called **rvschool.zip**, or a 7-Zip archive called
  **rvschool.7z**.
* The corresponding BIOS system is the Capcom ZN2 board, which has the short
  name ``coh3002c``, so MAME will look for a folder called **coh3002c**, a PKZIP
  archive called **coh3002c.zip**, or a 7-Zip archive called **coh3002c.7z**.

Device ROMs
~~~~~~~~~~~

For each folder configured in your ``rompath`` setting, MAME looks for device
ROMs in the following locations:

* A folder or archive matching the short name of the device.
* A folder or archive matching the short name of the device’s parent ROM device,
  if applicable.
* A folder or archive matching the short name of the system.
* A folder or archive matching the short name of the system’s parent system, if
  applicable.
* A folder or archive matching the short name of the corresponding BIOS system,
  if applicable.

Using a Unitron 1024 Macintosh clone with a French Macintosh Plus keyboard with
integrated numeric keypad attached as an example, MAME will look for the
keyboard microcontroller ROM as follows:

* The short name of the French Macintosh Plus keyboard is ``mackbd_m0110a_f``,
  so MAME will look for a folder called **mackbd_m0110a_f**, a PKZIP archive
  called **mackbd_m0110a_f.zip**, or a 7-Zip archive called
  **mackbd_m0110a_f.7z**.
* The parent ROM device is the U.S. Macintosh Plus keyboard with integrated
  numeric keypad, whcih has the short name ``mackbd_m0110a``, so MAME will look
  for a folder called **mackbd_m0110a**, a PKZIP archive called
  **mackbd_m0110a.zip**, or a 7-Zip archive called **mackbd_m0110a.7z**.
* The short name of the Unitron 1024 system is ``utrn1024``, so MAME will look
  for a folder called **utrn1024**, a PKZIP archive called **utrn1024.zip**, or
  a 7-Zip archive called **utrn1024.7z**.
* The parent system of the Unitron 1024 is the Macintosh Plus, which has the
  short name ``macplus``, so MAME will look for a folder called **macplus**, a
  PKZIP archive called **macplus.zip**, or a 7-Zip archive called
  **macplus.7z**.
* There is no corresponding BIOS system, so MAME will not search in any further
  locations.

Software Item ROMs
~~~~~~~~~~~~~~~~~~

For each folder configured in your ``rompath`` setting, MAME looks for software
item ROMs in the following locations:

* A folder or archive matching the short name of the software item inside a
  folder matching the short name of the software list (or a folder matching the
  short name of the software item inside an archive matching the name of the
  software list).
* A folder or archive matching the short name of the parent software item inside
  a folder matching the short name of the software list, if applicable (or a
  folder matching the short name of the parent software item in an archive
  matching the name of the software list).
* A folder or archive matching the short name of the software item.  (This is
  for convenience for software items that also run as stand-alone systems with
  the same short name, such as Neo Geo games.)
* A folder or archive matching the short name of the parent software item, if
  applicable.  (This is for convenience for software items that also run as
  stand-alone systems with the same short name, such as Neo Geo games.)
* Any folders and archives that would be searched for system or device ROMs for
  the system or device that the software list belongs to.  This is for
  historical reasons due to the way software list support was originally added
  to MESS and will be removed in a future version of MAME.

If you load the German version of Dune II from the Mega Drive/Genesis cartridge
software list in the PAL Mega Drive console, MAME will look for the cartridge
ROM as follows:

* The short name of the software item for the German version of Dune II is
  ``dune2g`` and the short name of the Mega Drive/Genesis cartridge software
  list is ``megadriv``, so MAME will look for a folder called **dune2g**, a
  PKZIP archive called **dune2g.zip** or a 7-Zip archive called **dune2g.7z**
  inside a folder called **megadriv** (or a folder called **dune2g** inside a
  PKZIP archive called **megadriv.zip** or a 7-Zip archive called
  **megadriv.7z**).
* The parent software item is the general European PAL version of Dune II in the
  same software list, which has the short name ``dune2``, so MAME will look for
  a folder called **dune2**, a PKZIP archive called **dune2.zip** or a 7-Zip
  archive called **dune2.7z** inside a folder called **megadriv** (or a folder
  called **dune2** inside a PKZIP archive called **megadriv.zip** or a 7-Zip
  archive called **megadriv.7z**).
* Next MAME will ignore the short name of the software list and use the short
  name of the software item only, looking for a folder called **dune2g**, a
  PKZIP archive called **dune2g.zip** or a 7-Zip archive called **dune2g.7z**.
* Still ignoring the short name of the software list, MAME will use the short
  name of the parent software item only, looking for a folder called **dune2**,
  a PKZIP archive called **dune2.zip** or a 7-Zip archive called **dune2.7z**.
* The short name of the PAL Mega Drive system is ``megadriv``, so MAME will look
  for a folder called **megadriv**, a PKZIP archive called **megadriv.zip**, or
  a 7-Zip archive called **megadriv.7z**.
* The parent system of the PAL Mega Drive is the North American Genesis system,
  which has the short name ``genesis``, so MAME will look for a folder called
  **genesis**, a PKZIP archive called **genesis.zip**, or a 7-Zip archive called
  **genesis.7z**.

CHD format disk images
~~~~~~~~~~~~~~~~~~~~~~

MAME searches for system, device and software item CHD format disk images in
almost the same way it searches for ROMs, with just a few differences:

* For systems and software items, MAME will check the parent system or software
  item if applicable for alternate names for a disk image with the same content
  digest.  This allows you to keep a single copy of a CHD format disk image for
  a parent system or software item and any clones that expect a disk image with
  the same content, irrespective of the name the clones expect.
* For software items, MAME will look for CHD format disk images in a folder
  matching the short name of the software list.  This is for convenience when
  all items in a software list only contain a single CHD format disk image each.
* We recommend that you *do not* store CHD format disk images inside PKZIP or
  7-Zip archives.  However, if you do decide to do this, MAME will only find CHD
  format disk images inside archives with an expected name.  This is because
  MAME uses the content digest from the CHD header, not the checksum of the CHD
  file itself.  The checksum of the CHD file itself can vary depending on
  compression options.

Loose software
~~~~~~~~~~~~~~

Many systems support loading media from a file by supplying the path on the
command line for one of the media options.  Relative paths are interpreted
relative to the current working directory.

You can specify a path to a file inside a PKZIP or 7-Zip archive similarly to
specifying a path to a file in a folder (keep in mind that you can have at most
a single archive file in a path, as MAME does not support loading files from
archives contained within other archives).  If you specify a path to a PKZIP or
7-Zip archive, MAME will use the first file found in the archive (this depends
on the order that files are stored in the archive – it’s most useful for
archives containing a single file).

Start the Nintendo Entertainment System/Famicom system with the file
**amazon_diet_EN.nes** mounted in the cartridge slot:

.. code-block:: bash

   mame nes -cart amazon_diet_EN.nes

Start the Osborne-1 system with the first file in the archive **os1xutls.zip**
mounted in the first floppy disk drive:

.. code-block:: bash

   mame osborne1 -flop1 os1xutils.zip

Start the Macintosh Plus system with the file **system tools.img** in the
archive **sys603.zip** mounted in the first floppy disk drive:

.. code-block:: bash

   mame macplus -flop1 "sys603.zip/system tools.img"

Diagnosing missing media
~~~~~~~~~~~~~~~~~~~~~~~~

When starting a system from MAME’s system selection menu or software selection
menu, MAME will list any missing system or device ROM dumps or disk images, as
long as at least one ROM dump or disk image for the system is present.  For
clone systems, at least one ROM dump or disk image *unique to the clone* must be
present for MAME to list missing ROM dumps and disk images.

If all system and device ROM dump and disk images are present and the system is
being started with a software item, MAME will check that ROM dumps and disk
images for the software item are present.  If at least one ROM dump or disk
image for the software item is present, MAME will list any missing ROM dumps or
disk images.

For example if you try to start the Macintosh Plus system and the keyboard
microcontroller ROM dump is missing, MAME displays the following error message:

    Required ROM/disk images for the selected system are missing or incorrect.
    Please acquire the correct files or select a different system.

    341-0332-a.bin (mackbd_m0110a) - not found

    Press any key to continue.

The name of the missing ROM dump is shown (**341-0332-a.bin**), as well as the
short name of the device it belongs to (``mackbd_m0110a``).  When a missing ROM
dump or disk image is not specific to the selected system, the short name of the
system or device it belongs to is shown.

If you start a system in MAME from a command prompt, MAME will show where it
searched for any ROM dumps or disk images that were not found.

Using the example of a Unitron 1024 Macintosh clone with a French keyboard
connected, MAME will show the following error messages if no ROMs are present::

    mame utrn1024 -kbd frp
    342-0341-a.u6d NOT FOUND (tried in utrn1024 macplus)
    342-0342-a.u8d NOT FOUND (tried in utrn1024 macplus)
    341-0332-a.bin NOT FOUND (tried in mackbd_m0110a_f mackbd_m0110a utrn1024 macplus)

MAME used the system short name ``utrn1024`` and the parent system short name
``macplus`` when searching for system ROMs.  When searching for the keyboard
microcontroller ROM, MAME used the device short name ``mackbd_m0110a_f``, the
parent ROM device short name ``mackbd_m0110a``, the system short name
``utrn1024``, and the parent system short name ``macplus``.

Software parts that use the ROM loader (typically cartridge media) show similar
messages when ROM dumps are not found.  Using the example of the German version
of Dune II on a PAL Mega Drive, MAME will show the following error messages if
no ROMs are present::

    mame megadriv dune2g
    mpr-16838-f.u1 NOT FOUND (tried in megadriv\dune2g megadriv\dune2 dune2g dune2 megadriv genesis)
    Fatal error: Required files are missing, the machine cannot be run.

MAME searched for the cartridge ROM using:

* The software list short name ``megadriv`` and the software item short name
  ``dune2g``.
* The software list short name ``megadriv`` and the parent software item short
  name ``dune2``.
* The software item short name ``dune2g`` only.
* The parent software item short name ``dune2`` only.
* The locations that would be searched for the PAL Mega Drive system (the system
  short name ``megadriv`` and the parent system short name ``genesis``).

Software parts that use the image file loader (including floppy disk and
cassette media) only check for media after ROM images are loaded, and missing
media files are shown differently.  Using the example of Macintosh System 6.0.3,
MAME will show these error messages if the software is missing::

    mame macplus -flop1 sys603:flop1
    :fdc:0:35dd: error opening image file system tools.img: No such file or directory (generic:2) (tried in mac_flop\sys603 sys603 macplus)
    Fatal error: Device Apple/Sony 3.5 DD (400/800K GCR) load (-floppydisk1 sys603:flop1) failed: No such file or directory

The error messages show where MAME searched for the image file in the same
format.  In this case, it used the software list short name ``mac_flop`` and the
software short name ``sys603``, the software short name ``sys603`` only, and
the locations that would be searched for system ROMs.
