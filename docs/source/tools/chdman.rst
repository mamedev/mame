.. _chdman:

chdman – CHD (Compressed Hunks of Data) File Manager
====================================================

chdman can be used to create, convert, check the integrity of and extract data
from media images in CHD (Compressed Hunks of Data) format.

The basic usage is ``chdman <command> <option>...``

.. contents:: :local:


.. _chdman-commonopts:

Common options
--------------

The options available depend on the command, but the following options are
used by multiple commands:

--input <file> / -i <file>
   Specify the input file.  This option is required for most commands.  The
   input file formats supported depend on the command
--inputparent <chdfile> / -ip <chdfile>
   Specify the parent CHD file for the input file.  This option is supported for
   most commands that operate on CHD format input files.  This option must be
   used if the input file is a *delta CHD*, storing only the hunks that differ
   from its parent CHD,
--inputstartbyte <offset> / -isb <offset>
    Specify the offset to the data in the input file in bytes.  This is useful
    for creating CHD format files from input files that contain a header before
    the start of the data, or for extracting partial content from a CHD format
    file.  May not be specified in combination with the
    ``--inputstarthunk``/``-ish`` option.
--inputstarthunk <offset> / -ish <offset>
    Specify the offset to the data in the input file in hunks.  May not be
    specified in combination with the ``--inputstartbyte``/``-isb`` option.
--inputbytes <length> / -ib <length>
    Specify the amount of input data to use in bytes, starting from the offset
    to the data in the input file.  This is useful for creating CHD format files
    from input files that contain additional content after the data, or for
    extracting partial content from a CHD format file.  May not be specified in
    combination with the ``--inputhunks``/``-ih`` option.
--inputhunks <length> / -ih <length>
    Specify the amount of input data to use in hunks, starting from the offset
    to the data in the input file.  May not be specified in combination with the
    ``--inputbytes``/``-ib`` option.
--output <file> / -o <file>
   Specify the output file name.  This option is required for commands that
   produce output files.  The output file formats supported depend on the
   command.
--outputparent <chdfile> / -op <chdfile>
   Specify the parent CHD file for the output file.  This option is supported
   for commands that produce CHD format output files.  Using this option
   produces a *delta CHD*, storing only the hunks that differ from its parent
   CHD.  The parent CHD should be the same media type and size, with the same
   hunk size.
--compression none|<type>[,<type>]... / -c none|<type>[,<type>]...
   Specify compression algorithms to use.  This option is supported for commands
   that produce CHD format output files.  Specify ``none`` to disable
   compression, or specify up to four comma-separated compression algorithms.
   See :ref:`compression algorithms <chdman-compression>` for supported
   compression algorithms.  Compression must be disable to create writable media
   image files.
--hunksize <bytes> / -hs <bytes>
   Specifies the hunk size in bytes.  This option is supported for commands that
   produce CHD format output files.  The hunk size must be no smaller than
   16 bytes and no larger than 1048576 bytes (1 MiB).  The hunk size must be a
   multiple of the sector size or unit size of the media.  Larger hunk sizes may
   give better compression ratios, but reduce performance for small random
   reads as an entire hunk needs to be read and decompressed at a time.
--force / -f
   Overwrite output files if they already exist.  This option is supported for
   commands that produce output files.
--verbose / -v
    Enable verbose output.  This prints more detailed information for some
    commands.
--numprocessors <count> / -np <count>
   Limit the maximum number of concurrent threads used for data compression.
   This option is supported for commands that produce CHD format output files.


.. _chdman-commands:

Commands
--------

info
~~~~

Display information about a CHD format file.  Information includes:

* CHD format version and compression algorithms used.
* Compressed and uncompressed sizes and overall compression ratio.
* Hunk size, unit size and number of hunks in the file.
* SHA1 digests of the data and metadata.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--verbose`` / ``-v``

verify
~~~~~~

Verify the integrity of a CHD format file.  The input file must be a read-only
CHD format file (the integrity of writable CHD files cannot be verified).  Note
that this command modifies its input file if the ``--fix``/``-f`` option is
specified.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputparent <chdfile>`` / ``-ip <chdfile>``

Additional options:

* ``--fix`` / ``-f``

createraw
~~~~~~~~~

Create a CHD format file from a raw media image.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputstartbyte <offset>`` / ``-isb <offset>``
* ``--inputstarthunk <offset>`` / ``-ish <offset>``
* ``--inputbytes <length>`` / ``-ib <length>``
* ``--inputhunks <length>`` / ``-ih <length>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--outputparent <chdfile>`` / ``-op <chdfile>``
* ``--compression none|<type>[,<type>]...`` / ``-c none|<type>[,<type>]...``
* ``--hunksize <bytes>`` / ``-hs <bytes>``
* ``--force`` / ``-f``
* ``--numprocessors <count>`` / ``-np <count>``

Additional options:

--unitsize <bytes> / -us <bytes> (required)
    The unit size for the output CHD file in bytes.  This is the smallest unit
    of data that can be addressed within the CHD file.  It should match the
    sector size or page size of the source media.  The hunk size must be a whole
    multiple of the unit size.  The unit size must be specified if no parent CHD
    file for the output is supplied.  If a parent CHD file for the output is
    supplied, the unit size must match the unit size of the parent CHD file.

If the ``--hunksize`` or ``-hs`` option is not supplied, the default will be:

* The hunk size of the parent CHD file if a parent CHD file for the output is
  supplied.
* The smallest whole multiple of the unit size not larger than 4 KiB if the unit
  size is not larger than 4 KiB (4096 bytes).
* The unit size if it is larger than 4 KiB (4096 bytes).

If the ``--compression`` or ``-c`` option is not supplied, it defaults to
``lzma,zlib,huff,flac``.

createhd
~~~~~~~~

Create a CHD format hard disk image file.

Common options supported:

* ``--input <file>`` / ``-i <file>``
* ``--inputstartbyte <offset>`` / ``-isb <offset>``
* ``--inputstarthunk <offset>`` / ``-ish <offset>``
* ``--inputbytes <length>`` / ``-ib <length>``
* ``--inputhunks <length>`` / ``-ih <length>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--outputparent <chdfile>`` / ``-op <chdfile>``
* ``--compression none|<type>[,<type>]...`` / ``-c none|<type>[,<type>]...``
* ``--hunksize <bytes>`` / ``-hs <bytes>`` (required)
* ``--force`` / ``-f``
* ``--numprocessors <count>`` / ``-np <count>``

Additional options:

* ``--sectorsize <bytes>`` / ``-ss <bytes>``
* ``--size <bytes>`` / ``-s <bytes>``
* ``--chs <cylinders>,<heads>,<sectors>`` / ``-chs <cylinders>,<heads>,<sectors>``
* ``--template <template>`` / ``-tp <template>``

Creates a blank (zero-filled) hard disk image if no input file is supplied.  The
input start/length (``--inputstartbyte``/``-isb``,
``--inputstarthunk``/``-ish``, ``--inputbytes``/``-ib`` and
``--inputhunks``/``-ih`` options) cannot be used if no input file is supplied.

If the ``--hunksize`` or ``-hs`` option is not supplied, the default will be:

* The hunk size of the parent CHD file if a parent CHD file for the output is
  supplied.
* The smallest whole multiple of the sector size not larger than 4 KiB if the
  sector size is not larger than 4 KiB (4096 bytes).
* The sector size if it is larger than 4 KiB (4096 bytes).

If the ``--compression`` or ``-c`` option is not supplied, it defaults to
``lzma,zlib,huff,flac`` if an input file is supplied, or ``none`` if no input
file is supplied.

createcd
~~~~~~~~

Create a CHD format CD-ROM image file.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--output <file>`` / ``-o <file>`` (required)
* ``--outputparent <chdfile>`` / ``-op <chdfile>``
* ``--compression none|<type>[,<type>]...`` / ``-c none|<type>[,<type>]...``
* ``--hunksize <bytes>`` / ``-hs <bytes>`` (required)
* ``--force`` / ``-f``
* ``--numprocessors <count>`` / ``-np <count>``

If the ``--hunksize`` or ``-hs`` option is not supplied, the default will be
the hunk size of the parent CHD if a parent CHD file for the output is supplied,
or eight sectors per hunk (18,816 bytes) otherwise.

If the ``--compression`` or ``-c`` option is not supplied, it defaults to
``cdlz,cdzl,cdfl``.

createdvd
~~~~~~~~~

Create a CHD format DVD-ROM image file.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputstartbyte <offset>`` / ``-isb <offset>``
* ``--inputstarthunk <offset>`` / ``-ish <offset>``
* ``--inputbytes <length>`` / ``-ib <length>``
* ``--inputhunks <length>`` / ``-ih <length>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--outputparent <chdfile>`` / ``-op <chdfile>``
* ``--compression none|<type>[,<type>]...`` / ``-c none|<type>[,<type>]...``
* ``--hunksize <bytes>`` / ``-hs <bytes>`` (required)
* ``--force`` / ``-f``
* ``--numprocessors <count>`` / ``-np <count>``

If the ``--hunksize`` or ``-hs`` option is not supplied, the default will be
the hunk size of the parent CHD if a parent CHD file for the output is supplied,
or two sectors per hunk (4096 bytes) otherwise.

If the ``--compression`` or ``-c`` option is not supplied, it defaults to
``lzma,zlib,huff,flac``.

createld
~~~~~~~~

Create a CHD format LaserDisc image file.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--output <file>`` / ``-o <file>`` (required)
* ``--outputparent <chdfile>`` / ``-op <chdfile>``
* ``--compression none|<type>[,<type>]...`` / ``-c none|<type>[,<type>]...``
* ``--hunksize <bytes>`` / ``-hs <bytes>`` (required)
* ``--force`` / ``-f``
* ``--numprocessors <count>`` / ``-np <count>``

Additional options:

* ``--inputstartframe <offset>`` / ``-isf <offset>``
* ``--inputframes <length>`` / ``-if <length>``

If the ``--compression`` or ``-c`` option is not supplied, it defaults to
``avhu``.

extractraw
~~~~~~~~~~

Extract data from a CHD format raw media image.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputparent <chdfile>`` / ``-ip <chdfile>``
* ``--inputstartbyte <offset>`` / ``-isb <offset>``
* ``--inputstarthunk <offset>`` / ``-ish <offset>``
* ``--inputbytes <length>`` / ``-ib <length>``
* ``--inputhunks <length>`` / ``-ih <length>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--force`` / ``-f``

extracthd
~~~~~~~~~

Extract data from a CHD format hard disk image.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputparent <chdfile>`` / ``-ip <chdfile>``
* ``--inputstartbyte <offset>`` / ``-isb <offset>``
* ``--inputstarthunk <offset>`` / ``-ish <offset>``
* ``--inputbytes <length>`` / ``-ib <length>``
* ``--inputhunks <length>`` / ``-ih <length>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--force`` / ``-f``

extractcd
~~~~~~~~~

Extract data from a CHD format CD-ROM image.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputparent <chdfile>`` / ``-ip <chdfile>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--force`` / ``-f``

Additional options:

* ``--outputbin <file>`` / ``-ob <file>``
* ``--splitbin`` / ``-sb``

extractdvd
~~~~~~~~~~

Extract data from a CHD format DVD-ROM image.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputparent <chdfile>`` / ``-ip <chdfile>``
* ``--inputstartbyte <offset>`` / ``-isb <offset>``
* ``--inputstarthunk <offset>`` / ``-ish <offset>``
* ``--inputbytes <length>`` / ``-ib <length>``
* ``--inputhunks <length>`` / ``-ih <length>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--force`` / ``-f``

extractld
~~~~~~~~~

Extract data from a CHD format DVD-ROM image.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--inputparent <chdfile>`` / ``-ip <chdfile>``
* ``--output <file>`` / ``-o <file>`` (required)
* ``--force`` / ``-f``

Additional options:

* ``--inputstartframe <offset>`` / ``-isf <offset>``
* ``--inputframes <length>`` / ``-if <length>``

addmeta
~~~~~~~

Add a metadata item to a CHD format file.  Note that this command modifies its
input file.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)

Additional options:

* ``--tag <tag>`` / ``-t <tag>`` (required)
* ``--index <index>`` / ``-ix <index>``
* ``--valuetext <text>`` / ``-vt <text>``
* ``--valuefile <file>`` / ``-vf <file>``
* ``--nochecksum`` / ``-nocs``

delmeta
~~~~~~~

Delete a metadata item from a CHD format file.  Note that this command modifies
its input file.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)

Additional options:

* ``--tag <tag>`` / ``-t <tag>`` (required)
* ``--index <index>`` / ``-ix <index>``

dumpmeta
~~~~~~~~

Extract metadata items from a CHD format file to the standard output or to a
file.

Common options supported:

* ``--input <file>`` / ``-i <file>`` (required)
* ``--output <file>`` / ``-o <file>``
* ``--force`` / ``-f``

Additional options:

* ``--tag <tag>`` / ``-t <tag>`` (required)
* ``--index <index>`` / ``-ix <index>``

listtemplates
~~~~~~~~~~~~~

List available hard disk templates.  This command does not accept any options.


.. _chdman-compression:

Compression algorithms
----------------------

The following compression algorithms are supported:

zlib – zlib deflate
   Compresses data using the zlib deflate algorithm.
zstd – Zstandard
   Compresses data using the Zstandard algorithm.  This gives very good
   compression and decompression performance with better compression ratios than
   zlib deflate, but older software may not support CHD files that use Zstandard
   compression.
lzma – Lempel-Ziv-Markov chain algorithm
   Compresses data using the Lempel-Ziv-Markov-chain algorithm (LZMA).  This
   gives high compression ratios at the cost of poor compression and
   decompression performance.
huff – Huffman coding
   Compresses data using 8-bit Huffman entropy coding.
flac – Free Lossless Audio Codec
   Compresses data as two-channel (stereo) 16-bit 44.1 kHz PCM audio using the
   Free Lossless Audio Codec (FLAC).  This gives good compression ratios if the
   media contains 16-bit PCM audio data.
cdzl – zlib deflate for CD-ROM data
   Compresses audio data and subchannel data from CD-ROM sectors separately
   using the zlib deflate algorithm.
cdzs – Zstandard for CD-ROM data
   Compresses audio data and subchannel data from CD-ROM sectors separately
   using the Zstandard algorithm.  This gives very good compression and
   decompression performance with better compression ratios than zlib deflate,
   but older software may not support CHD files that use Zstandard compression.
cdlz - Lempel-Ziv-Markov chain algorithm/zlib deflate for CD-ROM data
   Compresses audio data and subchannel data from CD-ROM sectors separately
   using the Lempel-Ziv-Markov chain algorithm (LZMA) for audio data and the
   zlib deflate algorithm for subchannel data.  This gives high compression
   ratios at the cost of poor compression and decompression performance.
cdfl – Free Lossless Audio Codec/zlib deflate for CD-ROM data
   Compresses audio data and subchannel data from CD-ROM sectors separately
   using the Free Lossless Audio Codec (FLAC) for audio data and the zlib
   deflate algorithm for subchannel data.  This gives good compression ratios
   for audio CD tracks.
avhu – Huffman coding for audio-visual data
   This is a specialised compression algorithm for audio-visual (A/V) data.  It
   should only be used for LaserDisc CHD files.
