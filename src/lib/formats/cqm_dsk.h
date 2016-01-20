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

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_CQM_FORMAT;

#endif /* CQM_DSK_H_ */
