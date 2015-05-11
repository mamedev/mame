// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    iflopimg.h

    Bridge code for Imgtool into the standard floppy code

*********************************************************************/

#ifndef IFLOPIMG_H
#define IFLOPIMG_H

#include "formats/flopimg.h"
#include "library.h"

/***************************************************************************

    Prototypes

***************************************************************************/

enum
{
	IMGTOOLINFO_PTR_FLOPPY_FORMAT = IMGTOOLINFO_PTR_CLASS_SPECIFIC,
	IMGTOOLINFO_PTR_FLOPPY_OPEN,
	IMGTOOLINFO_PTR_FLOPPY_CREATE
};

int imgtool_floppy_make_class(int index, imgtool_class *imgclass);

floppy_image_legacy *imgtool_floppy(imgtool_image *img);
imgtoolerr_t imgtool_floppy_error(floperr_t err);

imgtoolerr_t imgtool_floppy_read_sector_to_stream(imgtool_image *img, int head, int track, int sector, int offset, size_t length, imgtool_stream *f);
imgtoolerr_t imgtool_floppy_write_sector_from_stream(imgtool_image *img, int head, int track, int sector, int offset, size_t length, imgtool_stream *f);


void *imgtool_floppy_extrabytes(imgtool_image *img);

#endif /* IFLOPIMG_H */
