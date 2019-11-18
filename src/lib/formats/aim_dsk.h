// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    formats/aim_dsk.h

    AIM disk images

*********************************************************************/

#ifndef AIM_DSK_H
#define AIM_DSK_H

#include "flopimg.h"

/**************************************************************************/

class aim_format : public floppy_image_format_t
{
public:
	aim_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override { return false; }
};

extern const floppy_format_type FLOPPY_AIM_FORMAT;

#endif /* AIM_DSK_H */
