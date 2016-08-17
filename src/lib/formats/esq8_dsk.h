// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*********************************************************************

    formats/esq8_dsk.h

    Formats for 8-bit Ensoniq synthesizers and samplers

    Disk is PC MFM, 40 tracks, single (Mirage) or double (SQ-80) sided,
    with 6 sectors per track.
    Sectors 0-4 are 1024 bytes, sector 5 is 512 bytes

*********************************************************************/

#ifndef ESQ8_DSK_H_
#define ESQ8_DSK_H_

#include "flopimg.h"

class esq8img_format : public floppy_image_format_t
{
public:
	esq8img_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e esq_6_desc[];

private:
	void find_size(io_generic *io, int &track_count, int &head_count, int &sector_count);
};

extern const floppy_format_type FLOPPY_ESQ8IMG_FORMAT;

#endif /* ESQ8_DSK_H_ */
