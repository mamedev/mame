// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/dim_dsk.h

    DIM disk images

*********************************************************************/

#ifndef DIM_DSK_H
#define DIM_DSK_H

#include "flopimg.h"

/**************************************************************************/

FLOPPY_IDENTIFY(dim_dsk_identify);
FLOPPY_CONSTRUCT(dim_dsk_construct);


class dim_format : public floppy_image_format_t
{
public:
	dim_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_DIM_FORMAT;

#endif /* DIM_DSK_H */
