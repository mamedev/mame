// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/ami_dsk.h

    Amiga disk images

*********************************************************************/
#ifndef MAME_FORMATS_AMI_DSK_H
#define MAME_FORMATS_AMI_DSK_H

#pragma once

#include "flopimg.h"

class adf_format : public floppy_image_format_t
{
public:
	adf_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	static uint32_t g32(const uint8_t *trackbuf, int track_size, int pos);
	static uint32_t checksum(const uint8_t *trackbuf, int track_size, int pos, int long_count);
};

extern const floppy_format_type FLOPPY_ADF_FORMAT;

#endif // MAME_FORMATS_AMI_DSK_H
