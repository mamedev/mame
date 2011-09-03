/*********************************************************************

    formats/st_dsk.c

    Atari ST generic 9/10/11 sector-per-track formats

*********************************************************************/

#include "formats/st_dsk.h"

st_gen_format::st_gen_format()
{
}

#define SECTOR_42_HEADER(cid)   \
	{ CRC_CCITT_START, cid },   \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfe, 1 },         \
	{   TRACK_ID },             \
	{   HEAD_ID },              \
	{   MFM, 0x42, 1 },         \
	{   MFM, 0x02, 1 },         \
	{ CRC_END, cid },           \
	{ CRC, cid }

#define NORMAL_SECTOR(cid)      \
	{ CRC_CCITT_START, cid },   \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfe, 1 },         \
	{   TRACK_ID },             \
	{   HEAD_ID },              \
	{   SECTOR_ID },            \
	{   SIZE_ID },              \
	{ CRC_END, cid },           \
	{ CRC, cid },               \
	{ MFM, 0x4e, 22 },          \
	{ MFM, 0x00, 12 },          \
	{ CRC_CCITT_START, cid+1 }, \
	{   RAW, 0x4489, 3 },       \
	{   MFM, 0xfb, 1 },         \
	{   SECTOR_DATA, -1 },      \
	{ CRC_END, cid+1 },         \
	{ CRC, cid+1 }

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_9[] = {
	{ MFM, 0x4e, 501 },
	{ MFM, 0x00, 12 },

	SECTOR_42_HEADER(1),

	{ MFM, 0x4e, 22 },
	{ MFM, 0x00, 12 },

	{ SECTOR_LOOP_START, 1, 9 },
	NORMAL_SECTOR(2),
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	{ SECTOR_LOOP_END },

	SECTOR_42_HEADER(4),

	{ MFM, 0x4e, 157 },

	{ END }
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_0[] = {
	{ MFM, 0x4e, 46 },
	SECTOR_42_HEADER(1),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 10 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END }
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_1[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 10, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 9 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_2[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 9, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 8 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_3[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 8, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 7 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_4[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 7, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 6 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_5[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 6, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 5 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_6[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 5, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 4 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_7[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 4, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 3 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_8[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 3, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 2 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_10_9[] = {
	{ MFM, 0x4e, 20 },
	{ SECTOR_LOOP_START, 2, 10 },
	{   MFM, 0x4e, 40 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(1),
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 26 },
	SECTOR_42_HEADER(3),
	{ MFM, 0x4e, 5 },

	{ SECTOR_LOOP_START, 1, 1 },
	{   MFM, 0x00, 12 },
	NORMAL_SECTOR(4),
	{   MFM, 0x4e, 40 },
	{ SECTOR_LOOP_END },

	{ MFM, 0x4e, 49 },
	{ END },
};

const floppy_image_format_t::desc_e *const st_gen_format::desc_fcp_10[] = {
	desc_fcp_10_0,
	desc_fcp_10_1,
	desc_fcp_10_2,
	desc_fcp_10_3,
	desc_fcp_10_4,
	desc_fcp_10_5,
	desc_fcp_10_6,
	desc_fcp_10_7,
	desc_fcp_10_8,
	desc_fcp_10_9,
};


const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_0[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_1[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_2[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_3[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_4[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_5[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_6[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_7[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_8[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_9[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};

const floppy_image_format_t::desc_e st_gen_format::desc_fcp_11_10[] = {
	{ MFM, 0x4e, 3 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  2,  2 }, NORMAL_SECTOR( 5), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  8,  8 }, NORMAL_SECTOR( 7), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  3,  3 }, NORMAL_SECTOR( 9), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  9,  9 }, NORMAL_SECTOR(11), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  4,  4 }, NORMAL_SECTOR(13), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 10, 10 }, NORMAL_SECTOR(15), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  5,  5 }, NORMAL_SECTOR(17), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START, 11, 11 }, NORMAL_SECTOR(19), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  6,  6 }, NORMAL_SECTOR(21), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  1,  1 }, NORMAL_SECTOR( 1), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 2 }, { MFM, 0x00, 2 },
	{ SECTOR_LOOP_START,  7,  7 }, NORMAL_SECTOR( 3), { SECTOR_LOOP_END },
	{ MFM, 0x4e, 23 },
	{ END },
};


const floppy_image_format_t::desc_e *const st_gen_format::desc_fcp_11[] = {
	desc_fcp_11_0,
	desc_fcp_11_1,
	desc_fcp_11_2,
	desc_fcp_11_3,
	desc_fcp_11_4,
	desc_fcp_11_5,
	desc_fcp_11_6,
	desc_fcp_11_7,
	desc_fcp_11_8,
	desc_fcp_11_9,
	desc_fcp_11_10,
};

void st_gen_format::generate(int track, int head, int track_count, int head_count, UINT8 *buffer, int sector_count, floppy_image *image)
{
	desc_s sectors[11];
	for(int i=0; i<sector_count; i++) {
		sectors[i].data = buffer + 512*i;
		sectors[i].size = 512;
	}

	const desc_e *desc = 0;
	switch(sector_count) {
	case 9:
		desc = desc_fcp_9;
		break;
	case 10:
		desc = desc_fcp_10[(track*head_count + head) % 10];
		break;
	case 11:
		desc = desc_fcp_11[(track*head_count + head) % 11];
		break;
	}

	generate_track(desc, track, head, sectors, sector_count, 100000, image);
}

st_format::st_format()
{
}

const char *st_format::name() const
{
	return "st";
}

const char *st_format::description() const
{
	return "Atari ST floppy disk image";
}

const char *st_format::extensions() const
{
	return "st";
}

bool st_format::supports_save() const
{
	return false;
}

void st_format::find_size(floppy_image *image, int &track_count, int &head_count, int &sector_count)
{
	int size = image->image_size();
	for(track_count=80; track_count <= 82; track_count++)
		for(head_count=1; head_count <= 2; head_count++)
			for(sector_count=9; sector_count <= 11; sector_count++)
				if(size == 512*track_count*head_count*sector_count)
					return;
	track_count = head_count = sector_count = 0;
}

int st_format::identify(floppy_image *image)
{
	int track_count, head_count, sector_count;
	find_size(image, track_count, head_count, sector_count);

	if(track_count)
		return 50;
	return 0;
}

bool st_format::load(floppy_image *image)
{
	UINT8 sectdata[11*512];
	int track_count, head_count, sector_count;
	find_size(image, track_count, head_count, sector_count);

	image->set_meta_data(track_count, head_count);
	int track_size = sector_count*512;
	for(int track=0; track < track_count; track++) {
		for(int side=0; side < head_count; side++) {
			image->image_read(sectdata, (track*head_count + side)*track_size, track_size);
			generate(track, side, track_count, head_count, sectdata, sector_count, image);
		}
	}

	return TRUE;
}

msa_format::msa_format()
{
}

const char *msa_format::name() const
{
	return "msa";
}

const char *msa_format::description() const
{
	return "Atari MSA floppy disk image";
}

const char *msa_format::extensions() const
{
	return "msa";
}

bool msa_format::supports_save() const
{
	return false;
}

void msa_format::read_header(floppy_image *image, UINT16 &sign, UINT16 &sect, UINT16 &head, UINT16 &strack, UINT16 &etrack)
{
	UINT8 h[10];
	image->image_read(h, 0, 10);
	sign = (h[0] << 8) | h[1];
	sect = (h[2] << 8) | h[3];
	head = (h[4] << 8) | h[5];
	strack = (h[6] << 8) | h[7];
	etrack = (h[8] << 8) | h[9];
}

bool msa_format::uncompress(UINT8 *buffer, int csize, int usize)
{
	UINT8 sectdata[11*512];
	int src=0, dst=0;
	while(src<csize && dst<usize) {
		unsigned char c = buffer[src++];
		if(c == 0xe5) {
			if(csize-src < 3)
				return false;
			c = buffer[src++];
			int count = (buffer[src] << 8) | buffer[src+1];
			if(usize-dst < count)
				return false;
			for(int i=0; i<count; i++)
				sectdata[dst++] = c;
		} else
			sectdata[dst++] = c;
	}
	if(src != csize || dst != usize)
		return false;
	memcpy(buffer, sectdata, usize);
	return true;
}

int msa_format::identify(floppy_image *image)
{
	UINT16 sign, sect, head, strack, etrack;
	read_header(image, sign, sect, head, strack, etrack);

	if(sign == 0x0e0f &&
	   (sect >=9 && sect <= 11) &&
	   (head == 0 || head == 1) &&
	   strack <= etrack &&
	   etrack < 82)
		return 100;
	return 0;
}

bool msa_format::load(floppy_image *image)
{
	UINT16 sign, sect, head, strack, etrack;
	read_header(image, sign, sect, head, strack, etrack);
	image->set_meta_data(etrack+1, head+1);

	UINT8 sectdata[11*512];
	UINT8 th[2];

	int pos = 10;
	int track_size = sect*512;

	for(int track=strack; track <= etrack; track++) {
		for(int side=0; side <= head; side++) {
			image->image_read(th, pos, 2);
			pos += 2;
			int tsize = (th[0] << 8) | th[1];
			image->image_read(sectdata, pos, tsize);
			pos += tsize;
			if(tsize < track_size) {
				if(!uncompress(sectdata, tsize, track_size))
					return FALSE;
			}
			generate(track, side, etrack+1, head+1, sectdata, sect, image);
		}
	}

	return TRUE;
}

const floppy_format_type FLOPPY_ST_FORMAT = &floppy_image_format_creator<st_format>;
const floppy_format_type FLOPPY_MSA_FORMAT = &floppy_image_format_creator<msa_format>;
