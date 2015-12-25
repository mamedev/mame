// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/hxcmfm_dsk.h

    HxC Floppy Emulator disk images

*********************************************************************/

#ifndef HXCMFM_DSK_H
#define HXCMFM_DSK_H

#include "flopimg.h"

class mfm_format : public floppy_image_format_t
{
public:
	mfm_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_MFM_FORMAT;

#endif /* HXCMFM_DSK_H */
