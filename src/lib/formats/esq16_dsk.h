// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    formats/esq16_dsk.h

    Formats for 16-bit Ensoniq synthesizers and samplers

    Disk is PC MFM, 80 tracks, double-sided, with 10 sectors per track

*********************************************************************/

#ifndef ESQ16_DSK_H_
#define ESQ16_DSK_H_

#include "flopimg.h"

class esqimg_format : public floppy_image_format_t
{
public:
	esqimg_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

	static const desc_e esq_10_desc[];

private:
	void find_size(io_generic *io, UINT8 &track_count, UINT8 &head_count, UINT8 &sector_count);
};

extern const floppy_format_type FLOPPY_ESQIMG_FORMAT;

#endif /* ESQ16_DSK_H_ */
