/*********************************************************************

    formats/d81_dsk.c

    Floppy format code for Commodore 1581 disk images

*********************************************************************/

#include "basicdsk.h"
#include "d81_dsk.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define D81_SIZE	819200

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    d81_translate_offset - translates the
    physical offset to a logical offset within
    the disk image
-------------------------------------------------*/

/*

     file offset       | CBM logical  |  drive physical   |   specials
     decimal sedecimal | track/sector | cyl head sec offs |
     ------------------+--------------+-------------------+--------------
           0 0x000000  |    01;00     |   00;01;01        | first block
         256 0x000100  |    01;01     |   00;01;01   +256 |
           .     .     |      .       |     .  .          |
        4864 0x001300  |    01;19     |   00;01;10   +256 |
        5120 0x001400  |    01;20     |   00;00;01        |
           .     .     |      .       |     .  .          |
        9984 0x002700  |    01;39     |   00;00;10   +256 |
       10240 0x002800  |    02;00     |   01;01;01        |
           .     .     |      .       |     .  .          |
       15360 0x003C00  |    02;20     |   01;00;01        |
           .     .     |      .       |     .  .          |
       20480 0x005000  |    03;00     |   02;01;01        |
           .     .     |      .       |     .  .          |
           .     .     |      .       |     .  .          |
       30729 0x007800  |    04;00     |   03;01;01        |
           .     .     |      .       |     .  .          |
           .     .     |      .       |     .  .          |
           .     .     |      .       |     .  .          |
      399360 0x061800  |    40;00     |   39;01;01        | disk header
      399616 0x061900  |    40;01     |   39;01;01   +256 | 1st BAM block
      399872 0x061A00  |    40;02     |   39;01;02        | 2nd BAM block
      400128 0x061B00  |    40;03     |   39;01;02   +256 | 1st dir block
           .     .     |      .       |     .  .          |
      409600 0x064000  |    41;00     |   40;01;01        |
           .     .     |      .       |     .  .          |
           .     .     |      .       |     .  .          |
           .     .     |      .       |     .  .          |
      808960 0x0C5800  |    80;00     |   79;01;01        |
           .     .     |      .       |     .  .          |
      813824 0x0C6B00  |    80;19     |   79;01;10   +256 |
      814080 0x0C6C00  |    80;20     |   79;00;01        |
           .     .     |      .       |     .  .          |
      818688 0x0C7E00  |    80;38     |   79;00;10        |
      818944 0x0C7F00  |    80;39     |   79;00;10   +256 | last block

*/

static UINT64 d81_translate_offset(floppy_image_legacy *floppy, const struct basicdsk_geometry *geom, int track, int head, int sector)
{
	UINT64 offset = (track * 20) + (!head * 10) + sector;

	return offset;
}

/*-------------------------------------------------
    FLOPPY_IDENTIFY( d81_dsk_identify )
-------------------------------------------------*/

FLOPPY_IDENTIFY( d81_dsk_identify )
{
	*vote = (floppy_image_size(floppy) == D81_SIZE) ? 100 : 0;

	return FLOPPY_ERROR_SUCCESS;
}

/*-------------------------------------------------
    FLOPPY_CONSTRUCT( d81_dsk_construct )
-------------------------------------------------*/

/*
PER TRACK ORGANIZATION:

      Hex 4E written as a gap, with 10 sectors of data, with full gaps written for motor speed variation.

PER SECTOR ORGANIZATION:

      MFM Encoding
      12 Bytes of Hex 00
      3 Bytes of Hex A1 (Data Hex A1, Clock Hex 0A)
      1 Byte of Hex FE (ID Address Mark)
      1 Byte (Track Number)
      1 Byte (Side Number)
      1 Byte (Sector Number)
      1 Byte (Sector Length, 02 for 512 Byte Sectors)
      2 Bytes CRC (Cyclic Redundancy Check)
      22 Bytes of Hex 22
      12 Bytes of Hex 00
      3 Bytes of Hex A1 (Data Hex A1, Clock Hex 0A)
      1 Byte of Hex FB (Data Address Mark)
      512 Bytes of Data
      2 Bytes of CRC (Cyclic Redundancy Check)
      38 Bytes of Hex 4E
*/

FLOPPY_CONSTRUCT( d81_dsk_construct )
{
	struct basicdsk_geometry geometry;

	memset(&geometry, 0, sizeof(geometry));

	geometry.heads = 2;
	geometry.first_sector_id = 1;
	geometry.sector_length = 512;
	geometry.tracks = 80;
	geometry.sectors = 10;
	geometry.translate_offset = d81_translate_offset;

	return basicdsk_construct(floppy, &geometry);
}
