/*************************************************************************

    formats/coupedsk.h

    SAM Coupe disk image formats

**************************************************************************/

#ifndef __COUPEDSK_H__
#define __COUPEDSK_H__

#include "flopimg.h"

class mgt_format : public floppy_image_format_t
{
public:
	mgt_format();

	virtual int identify(io_generic *io);
	virtual bool load(io_generic *io, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;

	static const floppy_image_format_t::desc_e desc_10[];
};

extern const floppy_format_type FLOPPY_MGT_FORMAT;

#endif /* __COUPEDSK_H__ */
