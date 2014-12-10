/*********************************************************************

    formats/pc98dip_dsk.h

    PC98DIP disk images

*********************************************************************/

#ifndef PC98DIP_DSK_H
#define PC98DIP_DSK_H

#include "flopimg.h"


class pc98dip_format : public floppy_image_format_t
{
public:
	pc98dip_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_PC98DIP_FORMAT;

#endif /* PC98DIP_DSK_H */
