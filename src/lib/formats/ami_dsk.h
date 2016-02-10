// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/ami_dsk.h

    Amiga disk images

*********************************************************************/

#ifndef AMI_DSK_H_
#define AMI_DSK_H_

#include "flopimg.h"

class adf_format : public floppy_image_format_t
{
public:
	adf_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	static UINT32 g32(const UINT8 *trackbuf, int track_size, int pos);
	static UINT32 checksum(const UINT8 *trackbuf, int track_size, int pos, int long_count);
};

extern const floppy_format_type FLOPPY_ADF_FORMAT;

#endif /*AMI_DSK_H_*/
