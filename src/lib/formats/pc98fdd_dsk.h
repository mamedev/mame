/*********************************************************************

    formats/pc98fdd_dsk.h

    PC98FDD disk images

*********************************************************************/

#ifndef PC98FDD_DSK_H
#define PC98FDD_DSK_H

#include "flopimg.h"


class pc98fdd_format : public floppy_image_format_t
{
public:
	pc98fdd_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_PC98FDD_FORMAT;

#endif /* PC98FDD_DSK_H */
