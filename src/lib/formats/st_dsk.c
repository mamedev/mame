/*********************************************************************

    formats/st_dsk.c

    Atari ST generic 9/10/11 sector-per-track formats

*********************************************************************/

#include "formats/st_dsk.h"

st_gen_format::st_gen_format(const char *name,const char *extensions,const char *description,const char *param_guidelines) :
	floppy_image_format_t(name,extensions,description,param_guidelines)
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
