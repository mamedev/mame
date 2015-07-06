// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/g64_dsk.h

    Commodore 1541/1571 GCR disk image format

*********************************************************************/

#ifndef G64_DSK_H_
#define G64_DSK_H_

#include "flopimg.h"
#include "imageutl.h"

class g64_format : public floppy_image_format_t {
public:
	g64_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const { return true; }

protected:
	enum
	{
		POS_SIGNATURE = 0x0,
		POS_VERSION = 0x8,
		POS_TRACK_COUNT = 0x9,
		POS_MAX_TRACK_SIZE = 0xa,
		POS_TRACK_OFFSET = 0xc
	};

	enum
	{
		TRACK_LENGTH = 0x1ef8,
		TRACK_COUNT = 84
	};

	static const UINT32 c1541_cell_size[];

	int generate_bitstream(int track, int head, int speed_zone, UINT8 *trackbuf, int &track_size, floppy_image *image);
};

extern const floppy_format_type FLOPPY_G64_FORMAT;

#endif
