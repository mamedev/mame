// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef DFI_DSK_H
#define DFI_DSK_H

#include "flopimg.h"

class dfi_format : public floppy_image_format_t
{
public:
	dfi_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	//  virtual bool save(io_generic *io, floppy_image *image);

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;
};

extern const floppy_format_type FLOPPY_DFI_FORMAT;

#endif /* DFI_DSK_H */
