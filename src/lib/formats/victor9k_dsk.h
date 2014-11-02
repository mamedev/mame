// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*********************************************************************

    formats/victor9k_dsk.h

    Victor 9000 sector disk image format

*********************************************************************/

#ifndef VICTOR9K_DSK_H_
#define VICTOR9K_DSK_H_

#include "flopimg.h"

class victor9k_format : public floppy_image_format_t {
public:
	struct format {
		UINT32 form_factor;      // See floppy_image for possible values
		UINT32 variant;          // See floppy_image for possible values

		UINT8 track_count;
		UINT8 head_count;
		UINT16 sector_base_size;
	};

	victor9k_format();

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool supports_save() const;

protected:
	static const format formats[];

	static const UINT32 cell_size[];
	static const int sectors_per_track[];
	static const int speed_zone[];
};

extern const floppy_format_type FLOPPY_VICTOR_9000_FORMAT;


FLOPPY_IDENTIFY( victor9k_dsk_identify );

FLOPPY_CONSTRUCT( victor9k_dsk_construct );

#endif
