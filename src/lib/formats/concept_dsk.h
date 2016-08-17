// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*********************************************************************

    formats/concept_dsk.h

    Formats for Corvus Concept

*********************************************************************/

#ifndef CONCEPT_DSK_H_
#define CONCEPT_DSK_H_

#include "flopimg.h"

class cc525dsdd_format : public floppy_image_format_t
{
public:
	cc525dsdd_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e cc_9_desc[];

private:
	void find_size(io_generic *io, UINT8 &track_count, UINT8 &head_count, UINT8 &sector_count);
};

extern const floppy_format_type FLOPPY_CONCEPT_525DSDD_FORMAT;

#endif /* CONCEPT_DSK_H_ */
