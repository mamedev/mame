// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#ifndef TD0_DSK_H_
#define TD0_DSK_H_

#include "flopimg.h"


class td0_format : public floppy_image_format_t
{
public:
	td0_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_TD0_FORMAT;

#endif /* TD0_DSK_H_ */
