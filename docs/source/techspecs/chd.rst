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

Hunk Count
~~~~~~~~~~
The total number of hunks in a CHD file.

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
has a different layout, but the layout of the first 16 bytes are always the same and are
sufficient to determine the CHD version. All numbers are in **big endian** order.

Version 1
~~~~~~~~~
The CHD version 1 header is 76 bytes long. The structure of the version 1 header 
is as follows. CHD version 1 only supports hard disks.

+------------------+----------+
| Magic_Number     | 8 bytes  |
+------------------+----------+
| Header_Size      | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Hunk_Size        | 4 bytes  |
+------------------+----------+
| Hunk_Count       | 4 bytes  |
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

Header_Size
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

Hunk_Count
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
| Header_Size      | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Hunk_Size        | 4 bytes  |
+------------------+----------+
| Hunk_Count       | 4 bytes  |
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
| Sector_Size      | 4 bytes  |
+------------------+----------+

Magic_Number
''''''''''''
'MComprHD', 8 bytes

Header_Size
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
4 byte unsigned integer, big-endian. Number of ``Sector_Size``-length sectors per hunk. 
**Not** the *hunk size* as used conventionally in this document. To calculate
the *hunk size*, multiply ``Hunk_Size`` by ``Sector_Size``.

Hunk_Count
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

Sector_Size
'''''''''''''
4 byte unsigned integer, big-endian. The number of bytes per sector.

Version 3
~~~~~~~~~
The CHD version 3 header is 120 bytes long. The structure of the version 3 header is as follows.

+------------------+----------+
| Magic_Number     | 8 bytes  |
+------------------+----------+
| Header_Size      | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Hunk_Count       | 4 bytes  |
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

Header_Size
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

Hunk_Count
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
16 byte MD5 hash of the decompressed data in the CHD file. 

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
| Header_Size      | 4 bytes  |
+------------------+----------+
| Header_Version   | 4 bytes  |
+------------------+----------+
| Flags            | 4 bytes  |
+------------------+----------+
| Compression_Type | 4 bytes  |
+------------------+----------+
| Hunk_Count       | 4 bytes  |
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

Header_Size
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

Hunk_Count
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
| Header_Size         | 4 bytes  |
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

Header_Size
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
CHD version 1 and 2 share a map format, CHD version 3 and 4 extends the V1-2 map format differently,
and CHD version 5 uses a completely different map format. For CHD version 1-4, the map begins directly after
the header, and in CHD v5, the map occurs at ``Map_Offset``. The map has a total length of the size of
a map entry multiplied by the *hunk count*, and each map entry is laid out sequentially for all versions.

Version 1-2 Map
~~~~~~~~~~~~~~~
Each map entry in the V1-2 map format is stored as an 8-byte big-endian integer. The total size of the map in CHD version 1-2 can be calculated by
multiplying the *hunk count* by 8. The structure of a V1-2 map entry is as follows, assuming little-endian conversion.

+------------+---------+
| Block_Size | 20 bits |
+------------+---------+
| Offset     | 44 bits |
+------------+---------+

Block_Size
''''''''''
High 20 bits, interpreted as a 4 byte, unsigned little-endian integer. The *block size* of the hunk this map entry refers to.

Offset
''''''
Low 44 bits, interpreted as a 8 byte unsigned little-endian integer. The offset in the CHD file to the beginning of the hunk this map entry refers to.

Version 3-4 Map
~~~~~~~~~~~~~~~
Each map entry in the V3-4 map format is 16 bytes long. The total size of the map in CHD version 1-2 can be calculated by
multiplying the *hunk count* by 16. The structure of a V3-4 map entry is as follows.

+------------------+---------+
| Offset           | 8 bytes |
+------------------+---------+
| CRC              | 4 bytes |
+------------------+---------+
| Block_Size       | 3 bytes |
+------------------+---------+
| Compression_Type | 1 byte  |
+------------------+---------+

Offset
''''''
8 byte unsigned integer, big-endian. The offset in the CHD file to the beginning of the hunk this map entry refers to.

CRC
'''
4 byte unsigned integer, big-endian. The `CRC32 ISO/HDLC <https://reveng.sourceforge.io/crc-catalogue/17plus.htm#crc.cat.crc-32-iso-hdlc>`_ checksum for the 
decompressed hunk data.

Block_Size
''''''''''
3 byte unsigned integer, big-endian. The *block size* of the hunk this map entry refers to.

Compression_Type
''''''''''''''''
1 byte unsigned integer, the upper four bits are undefined. The type of compression used in the hunk this map entry refers to.

Possible compression types are

* ``0x0`` Invalid
* ``0x1`` Compressed with the CHD file Codec
* ``0x2`` Uncompressed
* ``0x3`` Mini (Raw Data stored in Offset)
* ``0x4`` Decompress from Self
* ``0x5`` Decompress from Parent
* ``0x6`` Secondary Compression

Version 5 Map 
~~~~~~~~~~~~~
CHD V5 maps can be either compressed or uncompressed. If the first codec in the header `Compression_Type` is `0x0`, then the entire CHD file, including the map, is uncompressed. 
If the first codec in the header `Compression_Type` is anything other than `0x0`, then a form of compression is used, and the map will be compressed.

Uncompressed Map
''''''''''''''''
Since the uncompressed map only occurs when the entire CHD file is uncompressed, each uncompressed map entry contains only the 4-byte offset to the beginning of the hunk data. 
The total size of an uncompressed map in CHD version 5 can be calculated by multiplying the *hunk count* by 4. The structure of a V5 uncompressed map entry is as follows.

+------------------+---------+
| Offset           | 4 bytes |
+------------------+---------+

Offset
>>>>>>
4 byte unsigned integer, big-endian. The offset in the CHD file to the beginning of the hunk this map entry refers to.

Compressed Map
''''''''''''''
The structure of the compressed map header is as follows.

+--------------+---------+
| Map_Size     | 4 bytes |
+--------------+---------+
| First_Offset | 6 bytes |
+--------------+---------+
| Map_CRC      | 2 bytes |
+--------------+---------+
| Size_Bits    | 1 byte  |
+--------------+---------+
| Self_Bits    | 1 byte  |
+--------------+---------+
| Parent_Bits  | 1 byte  |
+--------------+---------+
| Reserved     | 1 byte  |
+--------------+---------+

Map_Size
>>>>>>>>
4 byte unsigned integer, big-endian. The compressed length of the map.

First_Offset
>>>>>>>>>>
6 byte unsigned integer, big-endian. The offset in the CHD file to the beginning of the hunk the first map entry refers to.

Map_CRC
>>>>>>>
2 byte unsigned integer, big-endian. The `CRC16 IBM/3740 <https://reveng.sourceforge.io/crc-catalogue/16.htm#crc.cat.crc-16-ibm-3740>`_ checksum for the 
decompressed map data.

Size_Bits
>>>>>>>>>
1 byte unsigned integer. The number of bits used to store the *block size* of a hunk in a map entry, for hunks compressed with one of four codecs used in the CHD file.

Self_Bits
>>>>>>>>>
1 byte unsigned integer. The number of bits used to store the offset to a hunk that is a reference to another hunk in the same CHD file, for hunks that are decompressed from Self.

Parent_Bits
>>>>>>>>>>>
1 byte unsigned integer. The number of bits used to store the offset to a hunk that is a reference to another hunk in the parent file, for hunks that are decompressed from Parent.

Reserved
>>>>>>>>
Reserved for future use.

The structure of each map entry in the compressed map, once decompressed, is as follows.

+------------------+---------+
| Compression_Type | 1 byte  |
+------------------+---------+
| Block_Size       | 3 bytes |
+------------------+---------+
| Offset           | 6 bytes |
+------------------+---------+
| CRC              | 2 bytes |
+------------------+---------+

Compression_Type
>>>>>>>>>>>>>>>>
1 byte unsigned integer, the upper four bits are undefined. The type of compression used in the hunk this map entry refers to.

Possible compression types are

* ``0x0`` Compression Type 0 (`Compression_Type[0]`)
* ``0x1`` Compression Type 1 (`Compression_Type[1]`)
* ``0x2`` Compression Type 2 (`Compression_Type[2]`)
* ``0x3`` Compression Type 3 (`Compression_Type[3]`)
* ``0x4`` Uncompressed
* ``0x5`` Decompress from Self
* ``0x6`` Decompress from Parent

Block_Size
>>>>>>>>>>
3 byte unsigned integer, big-endian. The *block size* of the hunk this map entry refers to.

Offset
>>>>>>
6 byte unsigned integer, big-endian. The offset in the CHD file to the beginning of the hunk this map entry refers to.

CRC
>>>
2 byte unsigned integer, big-endian. The `CRC16 IBM/3740 <https://reveng.sourceforge.io/crc-catalogue/16.htm#crc.cat.crc-16-ibm-3740>`_ checksum for the 
decompressed hunk data.

Compression Codecs
------------------
CHD hunks can be compressed with a variety of codecs. Some of these codecs are implemented via vendored libraries whereas
some are implemented within MAME. For well-known algorithms, this document only describes necessary compression parameters
needed to decompress a chunk with a well-behaved implementation of the codec. Lesser-known algorithms will have their implementation
details and data layout described in more detail.

CHD compression works at a hunk granularity. A compressed hunk always decompresses to a buffer of **hunk sized** bytes, regardless
of codec used. Hunks may also be "compressed" with ``CHD_CODEC_NONE`` (``0x0``), which indicates uncompressed data, or refer to another
hunk in the same or parent CHD, but this section only describes the codecs and parameters used to compress hunks. See :ref:`Hunk Decoding` for
more details on how a hunk is decompressed.

Raw Deflate/Zlib (``zlib``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~
This codec is used in CHD versions 1-5. 

The ``zlib`` codec compresses hunks using the `Deflate <https://en.wikipedia.org/wiki/Deflate>`_ algorithm. The zlib header
is not used, and each hunk is raw, Deflate compressed bytes.

In CHD versions 1-4, this codec is known as ``CHDCOMPRESSION_ZLIB``. CHD versions 3 and 4 supported ``CHDCOMPRESSION_ZLIB_PLUS``, which is decoded in 
an identical manner as ``CHDCOMPRESSION_ZLIB``. 

Raw LZMA (``lzma``)
~~~~~~~~~~~~~~~~~~~
This codec is only used in CHD version 5.

The ``lzma`` codec compresses hunks with the `LZMA <https://en.wikipedia.org/wiki/Lempel%E2%80%93Ziv%E2%80%93Markov_chain_algorithm>`_ algorithm.
Hunks are compressed with LZMA1 **without any stream headers**, with compression level **9** and the default ``lclppb`` compression parameters for LZMA 19.0. These settings are

* Literal Context Bits (``lc``): 3
* Literal Position Bits (``lp``): 0
* Position Bits (``pb``): 2

While an unlimited dictionary size can be used, calculation of an appropriate dictionary size can be done with the following algorithm, lifted from 
`LzmaEnc::LzmaEncProps_Normallize <https://github.com/mamedev/mame/blob/2e91b7bbd5afbdc48011d1020f78fb675bac2011/3rdparty/lzma/C/LzmaEnc.c#L59>`_, where
``level`` is the compression level (``9``), and ``hunk_size`` is the hunk size of the CHD. If relevant, all integers should be truncated to 32 bits long. 

.. code-block:: python
    def get_lzma_dict_size(level = 9, hunk_size):
        if level <= 5:
            dict_size = 1 << (level * 2 + 14)
        elif level <= 7:
            dict_size = 1 << 25
        else:
            dict_size = 1 << 26
        
        if dict_size > hunk_size:
            for i in range(11, 31): # Inclusive range [11, 30]
                if hunk_size <= (2 << i):
                    dict_size = 2 << i 
                    break
                if hunk_size <= (3 << i):
                    dict_size = 3 << i
                    break
        return dict_size


Raw FLAC (``flac``)
~~~~~~~~~~~~~~~~~~~
This codec is only used in CHD version 5.

The ``flac`` codec compresses hunks with `FLAC <https://en.wikipedia.org/wiki/FLAC>`_ audio compression codec. 

At the start of each compressed hunk, there is a one byte header of either ``L`` (``0x4C``) to indicate little-endian output, or ``B`` (``0x42``) to indicate big-endian output. The FLAC-compressed bytes begin
after this one byte header. The FLAC decompressor implementation must be correctly configured according to the header byte.

FLAC data is compressed as raw FLAC frames, without `metadata blocks or a fLaC header <https://xiph.org/flac/format.html#stream>`_. There are 2 channels per block, 
each channel encoded as 16-bit signed integer PCM. The samples are interleaved with the left channel first, then the right channel. 


Raw Huffman (``huff``)
~~~~~~~~~~~~~~~~~~~~~~
This codec is only used in CHD version 5.

CD-ROM LZMA (``cdlz``)
~~~~~~~~~~~~~~~~~~~~~~

CD-ROM Deflate (``cdzl``)
~~~~~~~~~~~~~~~~~~~~~~~~~

CD-ROM FLAC (``cdfl``)
~~~~~~~~~~~~~~~~~~~~~~
This codec is used in CHD versions 3-5.

A/V Huffman (``avhu``)
~~~~~~~~~~~~~~~~~~~~~~
This codec is used in CHD versions 3-5.

In CHD versions 3 and 4, this codec is known as ``CHDCOMPRESSION_AV``.

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

Hunk Decoding
-------------

Decoding Legacy Hunks
~~~~~~~~~~~~~~~~~~~~~

Decoding V5 Hunks
~~~~~~~~~~~~~~~~~
