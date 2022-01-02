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
		2000, 18, 35, 1, 256, {}, -1, { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18 }, 32, 22, 24
	},
	{}
};


const floppy_format_type FLOPPY_COCO_RAWDSK_FORMAT = &floppy_image_format_creator<coco_rawdsk_format>;
