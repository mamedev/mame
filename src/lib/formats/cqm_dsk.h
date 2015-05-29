// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/cqm_dsk.h

    CopyQM disk images

*********************************************************************/

#ifndef CQM_DSK_H_
#define CQM_DSK_H_

#include "flopimg.h"

class cqm_format : public floppy_image_format_t
{
public:
	cqm_format();

	virtual int identify(io_generic *io, UINT32 form_factor);
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image);
	virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const;
	virtual const char *description() const;
	virtual const char *extensions() const;
	virtual bool supports_save() const;
};

extern const floppy_format_type FLOPPY_CQM_FORMAT;

#endif /* CQM_DSK_H_ */
