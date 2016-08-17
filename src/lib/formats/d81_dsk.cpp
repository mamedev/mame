// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/d81_dsk.c

    Commodore 1581 disk image format

*********************************************************************/

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

#include <assert.h>

#include "formats/d81_dsk.h"

d81_format::d81_format() : wd177x_format(formats)
{
}

const char *d81_format::name() const
{
	return "d81";
}

const char *d81_format::description() const
{
	return "Commodore 1581 disk image";
}

const char *d81_format::extensions() const
{
	return "d81";
}

const d81_format::format d81_format::formats[] = {
	// track description
	// 32x4e 12x00 3xf5 fe 2x00 01 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 02 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 03 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 04 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 05 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 06 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 07 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 08 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 09 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 35x4e 12x00 3xf5 fe 2x00 0a 02 f7 22x4e 12x00 3xf5 fb 512x00 f7
	// 164x4e
	{
		floppy_image::FF_35, floppy_image::DSDD, floppy_image::MFM,
		2000, 10, 80, 2, 512, {}, 1, {}, 32, 22, 35
	},

	{}
};

floppy_image_format_t::desc_e* d81_format::get_desc_mfm(const format &f, int &current_size, int &end_gap_index)
{
	static floppy_image_format_t::desc_e desc[25] = {
	/* 00 */ { MFM, 0x4e, f.gap_1 },
	/* 01 */ { SECTOR_LOOP_START, 0, f.sector_count-1 },
	/* 02 */ {   MFM, 0x00, 12 },
	/* 03 */ {   CRC_CCITT_START, 1 },
	/* 04 */ {     RAW, 0x4489, 3 },
	/* 05 */ {     MFM, 0xfe, 1 },
	/* 06 */ {     TRACK_ID },
	/* 07 */ {     HEAD_ID_SWAP },
	/* 08 */ {     SECTOR_ID },
	/* 09 */ {     SIZE_ID },
	/* 10 */ {   CRC_END, 1 },
	/* 11 */ {   CRC, 1 },
	/* 12 */ {   MFM, 0x4e, f.gap_2 },
	/* 13 */ {   MFM, 0x00, 12 },
	/* 14 */ {   CRC_CCITT_START, 2 },
	/* 15 */ {     RAW, 0x4489, 3 },
	/* 16 */ {     MFM, 0xfb, 1 },
	/* 17 */ {     SECTOR_DATA, -1 },
	/* 18 */ {   CRC_END, 2 },
	/* 19 */ {   CRC, 2 },
	/* 20 */ {   MFM, 0x4e, f.gap_3 },
	/* 21 */ { SECTOR_LOOP_END },
	/* 22 */ { MFM, 0x4e, 0 },
	/* 23 */ { RAWBITS, 0x9254, 0 },
	/* 24 */ { END }
	};

	current_size = f.gap_1*16;
	if(f.sector_base_size)
	current_size += f.sector_base_size * f.sector_count * 16;
	else {
	for(int j=0; j != f.sector_count; j++)
		current_size += f.per_sector_size[j] * 16;
	}
	current_size += (12+3+1+4+2+f.gap_2+12+3+1+2+f.gap_3) * f.sector_count * 16;

	end_gap_index = 22;

	return desc;
}

const floppy_format_type FLOPPY_D81_FORMAT = &floppy_image_format_creator<d81_format>;
