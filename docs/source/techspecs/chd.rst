MAME Compressed Hunks of Data (CHD)
===================================

.. contents:: :local:

Introduction
------------

Compressed Hunks of Data (CHD) is a container format for compressing hard disks, CD-ROMs, 
or LaserDiscs originally written by Aaron Giles. CHD divides an input stream into 'hunks' of 
equal size, each of which can potentially be compressed by a different codec or encoded as a 
duplicate of another hunk in the same, or a 'parent' CHD file. 

This document describes the CHD format. It is explicitly *descriptive*, and does not prescribe
how to encode a stream into a CHD file. It also describes the format parameters for each compression
codec used to compress individual hunks.

Definitions
-----------
Some terms used elsewhere in this document are defined here for clarity.

Hunk
~~~~
A *hunk* is a logical unit of compressed data in a CHD file. Hunks are described by their
*map entry* by their offset in the stream, compressed size (*block size*) and optionally
a checksum depending on the format and version of the CHD file. Each hunk decompresses 
completely into a buffer of consistent length (*hunk size*), which is the same for all
hunks and is global on a CHD file level.

Block Size 
~~~~~~~~~~
The compressed length of a hunk. Not to be confused with the *hunk size*.

Block Offset 
~~~~~~~~~~~~
The offset in the CHD file to the compressed data of the hunk. The compressed hunk data
begins at the block offset for block size number of bytes. 

Hunk Size 
~~~~~~~~~
The length of an uncompressed hunk. This length is the same for all hunks in a CHD file.
Not to be confused with the *block size*. 

Map Entry
~~~~~~~~~
Each hunk is defined by a co-indexed map entry in the map. A valid map entry for a hunk 
contains at least a *block offset* and *block size* for a hunk, and for hunks compressed
with a codec, a checksum value.

Parent
~~~~~~
A separate CHD file that contains hunks referred to in the child CHD file. Successful decoding
of the child CHD file requires the parent CHD.

Codec
~~~~~
A compression algorithm used to compress a hunk.

Header Format
-------------
There have been 5 versions of the CHD file format. All versions but version
5 are considered deprecated and are no longer in common use. Each CHD version
has a different layout, but the first 16 bytes are always the same and are
sufficient to determine the CHD version. All numbers are in **big endian** order.

Version 1
~~~~~~~~~
The CHD version 1 header is 76 bytes long. The structure of the version 1 header 
is as follows. CHD version 1 only supports hard disks.

+------------------+----------+
| Magic_Number     | 8 bytes  |
+------------------+----------+
| Header_Length    | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Hunk_Size        | 4 bytes  |
+------------------+----------+
| Total_Hunks      | 4 bytes  |
+------------------+----------+
| Cylinders        | 4 bytes  |
+------------------+----------+
| Heads            | 4 bytes  |
+------------------+----------+
| Sectors          | 4 bytes  |
+------------------+----------+
| MD5_Hash         | 16 bytes |
+------------------+----------+
| Parent_MD5_Hash  | 16 bytes |
+------------------+----------+

Magic_Number
''''''''''''
'MComprHD', 8 bytes

Header_Length
'''''''''''''
4 byte unsigned integer, big-endian. The length of the header. Value: 76. 

Header_Version
''''''''''''''
4 byte unsigned integer, big-endian. The version of the header. Value: 1. 

Flags
'''''
4 byte unsigned integer, big-endian. 

Possible values:

* ``0x00000001`` CHD requires a parent
* ``0x00000002`` CHD allows writes

Compression_Type
''''''''''''''''
4 byte unsigned integer, big-endian. The type of compression used for all
compressed hunks in the CHD file. 

Possible values:

* ``0x00000000`` No compression (``CHDCOMPRESSION_NONE``)
* ``0x00000001`` Deflate/Zlib (``CHDCOMPRESSION_ZLIB``)

Hunk_Size
'''''''''
4 byte unsigned integer, big-endian. Number of 512-byte sectors per hunk. 
**Not** the *hunk size* as used conventionally in this document. To calculate
the *hunk size*, multiply ``Hunk_Size`` by 512.

Total_Hunks
'''''''''''
4 byte unsigned integer, big-endian. The total number of hunks in the CHD file. 

Cylinders 
'''''''''
4 byte unsigned integer, big-endian. The total number of cylinders in the CHD file. 

Heads
'''''
4 byte unsigned integer, big-endian. The total number of heads in the CHD file. 

Sectors
'''''''
4 byte unsigned integer, big-endian. The total number of sectors in the CHD file. 

MD5_Hash
''''''''
16 byte MD5 hash of the decompressed data in this CHD file. 

Parent_MD5_Hash
'''''''''''''''
16 byte MD5 hash of the compressed parent CHD file.


Version 2
~~~~~~~~~
The CHD version 2 header is 80 bytes long. The structure of the version 2 header 
is as follows. CHD version 2 only supports hard disks.

+------------------+----------+
| Magic_Number     | 8 bytes  |
+------------------+----------+
| Header_Length    | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Hunk_Size        | 4 bytes  |
+------------------+----------+
| Total_Hunks      | 4 bytes  |
+------------------+----------+
| Cylinders        | 4 bytes  |
+------------------+----------+
| Heads            | 4 bytes  |
+------------------+----------+
| Sectors          | 4 bytes  |
+------------------+----------+
| MD5_Hash         | 16 bytes |
+------------------+----------+
| Parent_MD5_Hash  | 16 bytes |
+------------------+----------+
| Sector_Length    | 4 bytes  |
+------------------+----------+

Magic_Number
''''''''''''
'MComprHD', 8 bytes

Header_Length
'''''''''''''
4 byte unsigned integer, big-endian. The length of the header. Value: 76. 

Header_Version
''''''''''''''
4 byte unsigned integer, big-endian. The version of the header. Value: 1. 

Flags
'''''
4 byte unsigned integer, big-endian. 

Possible values:

* ``0x00000001`` CHD requires a parent
* ``0x00000002`` CHD allows writes

Compression_Type
''''''''''''''''
4 byte unsigned integer, big-endian. The type of compression used for all
compressed hunks in the CHD file. 

Possible values:

* ``0x00000000`` No compression (``CHDCOMPRESSION_NONE``)
* ``0x00000001`` Deflate/Zlib (``CHDCOMPRESSION_ZLIB``)

Hunk_Size
'''''''''
4 byte unsigned integer, big-endian. Number of ``Sector_Length``-length sectors per hunk. 
**Not** the *hunk size* as used conventionally in this document. To calculate
the *hunk size*, multiply ``Hunk_Size`` by ``Sector_Length``.

Total_Hunks
'''''''''''
4 byte unsigned integer, big-endian. The total number of hunks in the CHD file. 

Cylinders 
'''''''''
4 byte unsigned integer, big-endian. The total number of cylinders in the CHD file. 

Heads
'''''
4 byte unsigned integer, big-endian. The total number of heads in the CHD file. 

Sectors
'''''''
4 byte unsigned integer, big-endian. The total number of sectors in the CHD file. 

MD5_Hash
''''''''
16 byte MD5 hash of the decompressed data in this CHD file. 

Parent_MD5_Hash
'''''''''''''''
16 byte MD5 hash of the compressed parent CHD file.

Sector_Length
'''''''''''''
4 byte unsigned integer, big-endian. The number of bytes per sector.

Version 3
~~~~~~~~~
The CHD version 3 header is 120 bytes long. The structure of the version 3 header is as follows.

+------------------+----------+
| Magic_Number     | 8 bytes  |
+------------------+----------+
| Header_Length    | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Total_Hunks      | 4 bytes  |
+------------------+----------+
| Logical_Size     | 8 bytes  |
+------------------+----------+
| Metadata_Offset  | 8 bytes  |
+------------------+----------+
| MD5_Hash         | 16 bytes |
+------------------+----------+
| Parent_MD5_Hash  | 16 bytes |
+------------------+----------+
| Hunk_Size        | 4 bytes  |
+------------------+----------+
| SHA1_Hash        | 20 bytes |
+------------------+----------+
| Parent_SHA1_Hash | 20 bytes |
+------------------+----------+

Magic_Number
''''''''''''
'MComprHD', 8 bytes

Header_Length
'''''''''''''
4 byte unsigned integer, big-endian. The length of the header. Value: 76. 

Header_Version
''''''''''''''
4 byte unsigned integer, big-endian. The version of the header. Value: 1. 

Flags
'''''
4 byte unsigned integer, big-endian. 

Possible values:

* ``0x00000001`` CHD requires a parent
* ``0x00000002`` CHD allows writes

Compression_Type
''''''''''''''''
4 byte unsigned integer, big-endian. The type of compression used for all
compressed hunks in the CHD file. 

Possible values:

* ``0x00000000`` No compression (``CHDCOMPRESSION_NONE``)
* ``0x00000001`` Deflate/Zlib (``CHDCOMPRESSION_ZLIB``)
* ``0x00000002`` Deflate/Zlib+ (``CHDCOMPRESSION_ZLIB_PLUS``)

Total_Hunks
'''''''''''
4 byte unsigned integer, big-endian. The total number of hunks in the CHD file. 

Logical_Size
''''''''''''
4 byte unsigned integer, big-endian. The logical length in bytes of the decompressed data. 

Metadata_Offset
'''''''''''''''
8 byte unsigned integer, big-endian. The offset in the CHD file to the first metadata entry.

MD5_Hash
''''''''
16 byte MD5 hash of the decompressed data in this CHD file. 

Parent_MD5_Hash
'''''''''''''''
16 byte MD5 hash of the compressed parent CHD file.

Hunk_Size
'''''''''
4 byte unsigned integer, big-endian. The *hunk size*; the decompressed length of each hunk in the file.

SHA1_Hash
'''''''''
20 byte SHA1 hash of the decompressed data in this CHD file. 

Parent_SHA1_Hash
''''''''''''''''
20 byte SHA1 hash of the compressed parent CHD file.

Version 4
~~~~~~~~~
The CHD version 4 header is 108 bytes long. The structure of the version 4 header is as follows.

+------------------+----------+
| Magic_Number     | 8 bytes  |
+------------------+----------+
| Header_Length    | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Total_Hunks      | 4 bytes  |
+------------------+----------+
| Logical_Size     | 8 bytes  |
+------------------+----------+
| Metadata_Offset  | 8 bytes  |
+------------------+----------+
| Hunk_Size        | 4 bytes  |
+------------------+----------+
| SHA1_Hash        | 20 bytes |
+------------------+----------+
| Parent_SHA1_Hash | 20 bytes |
+------------------+----------+
| Raw_SHA1_Hash    | 20 bytes |
+------------------+----------+

Magic_Number
''''''''''''
'MComprHD', 8 bytes

Header_Length
'''''''''''''
4 byte unsigned integer, big-endian. The length of the header. Value: 76. 

Header_Version
''''''''''''''
4 byte unsigned integer, big-endian. The version of the header. Value: 1. 

Flags
'''''
4 byte unsigned integer, big-endian. 

Possible values:

* ``0x00000001`` CHD requires a parent
* ``0x00000002`` CHD allows writes

Compression_Type
''''''''''''''''
4 byte unsigned integer, big-endian. The type of compression used for all
compressed hunks in the CHD file. 

Possible values:

* ``0x00000000`` No compression (``CHDCOMPRESSION_NONE``)
* ``0x00000001`` Deflate/Zlib (``CHDCOMPRESSION_ZLIB``)
* ``0x00000002`` Deflate/Zlib+ (``CHDCOMPRESSION_ZLIB_PLUS``)
* ``0x00000003`` AV Huffman (``CHDCOMPRESSION_AV``)

Total_Hunks
'''''''''''
4 byte unsigned integer, big-endian. The total number of hunks in the CHD file. 

Logical_Size
''''''''''''
4 byte unsigned integer, big-endian. The logical length in bytes of the decompressed data. 

Metadata_Offset
'''''''''''''''
8 byte unsigned integer, big-endian. The offset in the CHD file to the first metadata entry.

Hunk_Size
'''''''''
4 byte unsigned integer, big-endian. The *hunk size*; the decompressed length of each hunk in the file.

SHA1_Hash
'''''''''
20 byte SHA1 hash of the CHD file including compressed data and metadata.

Parent_SHA1_Hash
''''''''''''''''
20 byte SHA1 hash of the parent CHD file including compressed data and metadata.

Raw_SHA1_Hash
'''''''''''''
20 byte SHA1 hash of the decompressed data in this CHD file. 

Version 5
~~~~~~~~~
The CHD version 5 header is 124 bytes long. The structure of the version 5 header is as follows.

+---------------------+----------+
| Magic_Number        | 8 bytes  |
+---------------------+----------+
| Header_Length       | 4 bytes  |
+---------------------+----------+
| Header_Version      | 4 bytes  |
+---------------------+----------+
| Compression_Type[4] | 16 bytes |
+---------------------+----------+
| Logical_Size        | 8 bytes  |
+---------------------+----------+
| Map_Offset          | 8 bytes  |
+---------------------+----------+
| Metadata_Offset     | 8 bytes  |
+---------------------+----------+
| Hunk_Size           | 4 bytes  |
+---------------------+----------+
| Unit_Size           | 4 bytes  |
+---------------------+----------+
| Raw_SHA1_Hash       | 20 bytes |
+---------------------+----------+
| SHA1_Hash           | 20 bytes |
+---------------------+----------+
| Parent_SHA1_Hash    | 20 bytes |
+---------------------+----------+

Magic_Number
''''''''''''
'MComprHD', 8 bytes

Header_Length
'''''''''''''
4 byte unsigned integer, big-endian. The length of the header. Value: 76. 

Header_Version
''''''''''''''
4 byte unsigned integer, big-endian. The version of the header. Value: 1. 

Compression_Type
''''''''''''''''
Array of 4, 4 byte unsigned integers, big-endian. The types of compression used
when compressing hunks in this CHD file. Each hunk can be compressed with any one 
of the four compression types. Version 5 compression codes are all FourCC codes except
for ``CHD_CODEC_NONE``, which uses the value ``0``.

Possible values:

* ``0x00000000`` No compression (``CHD_CODEC_NONE``)
* ``zlib`` Raw Deflate/zlib (``CHD_CODEC_ZLIB``)
* ``lzma`` Raw LZMA (``CHD_CODEC_LZMA``)
* ``flac`` Raw FLAC (``CHD_CODEC_FLAC``)
* ``huff`` Raw Huffman (``CHD_CODEC_HUFF``)
* ``cdzl`` CD-ROM Deflate/zlib (``CHD_CODEC_CDZL``)
* ``cdlz`` CD-ROM LZMA (``CHD_CODEC_CDLZ``)
* ``cdfl`` CD-ROM FLAC (``CHD_CODEC_CDFL``)
* ``avhu`` A/V Huffman (``CHD_CODEC_AVHUFF``)
  
Logical_Size
''''''''''''
4 byte unsigned integer, big-endian. The logical length in bytes of the decompressed data. 

Map_Offset
''''''''''
8 byte unsigned integer, big-endian. The offset in the CHD file to the beginning of the hunk map.

Metadata_Offset
'''''''''''''''
8 byte unsigned integer, big-endian. The offset in the CHD file to the first metadata entry.

Hunk_Size
'''''''''
4 byte unsigned integer, big-endian. The *hunk size*; the decompressed length of each hunk in the file.

Unit_Size
'''''''''
4 byte unsigned integer, big-endian. The length of each unit within each hunk.

Raw_SHA1_Hash
'''''''''''''
20 byte SHA1 hash of the decompressed data in this CHD file. 

SHA1_Hash
'''''''''
20 byte SHA1 hash of the CHD file including compressed data and metadata.

Parent_SHA1_Hash
''''''''''''''''
20 byte SHA1 hash of the parent CHD file including compressed data and metadata.


Hunk Map Format
---------------

Version 1-2 Map
~~~~~~~~~~~~~~~

Version 3-4 Map
~~~~~~~~~~~~~~~

Version 5 Map 
~~~~~~~~~~~~~

Compression Codecs
------------------

Raw Deflate/Zlib (``zlib``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Raw LZMA (``lzma``)
~~~~~~~~~~~~~~~~~~~

Raw FLAC (``flac``)
~~~~~~~~~~~~~~~~~~~

Raw Huffman (``huff``)
~~~~~~~~~~~~~~~~~~~~~~

CD-ROM LZMA (``cdlz``)
~~~~~~~~~~~~~~~~~~~~~~

CD-ROM Deflate (``cdzl``)
~~~~~~~~~~~~~~~~~~~~~~~~~

CD-ROM FLAC (``cdfl``)
~~~~~~~~~~~~~~~~~~~~~~

A/V Huffman (``avhu``)
~~~~~~~~~~~~~~~~~~~~~~


Metadata
--------


Static Huffman Coding
---------------------

Importing from a RLE-encoded Huffman Tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Importing from a Small Huffman-encoded Huffman Tree
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Delta-RLE Huffman 
~~~~~~~~~~~~~~~~~
