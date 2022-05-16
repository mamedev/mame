// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    CoCo Raw Disk

***************************************************************************/

#include "coco_rawdsk.h"


coco_rawdsk_format::coco_rawdsk_format() : wd177x_format(formats)
{
}

const char *coco_rawdsk_format::name() const
{
	return "coco_rawdsk";
}

const char *coco_rawdsk_format::description() const
{
	return "CoCo Raw Disk";
}

const char *coco_rawdsk_format::extensions() const
{
	return "raw";
}

const coco_rawdsk_format::format coco_rawdsk_format::formats[] =
{
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 35, 1, 256, {}, -1, { 1,12,5,16,9,2,13,6,17,10,3,14,7,18,11,4,15,8 }, 32, 22, 24
	},
	{
		floppy_image::FF_525, floppy_image::SSDD, floppy_image::MFM,
		2000, 18, 40, 1, 256, {}, -1, { 1,12,5,16,9,2,13,6,17,10,3,14,7,18,11,4,15,8 }, 32, 22, 24
	},
	{}
};


const coco_rawdsk_format FLOPPY_COCO_RAWDSK_FORMAT;
