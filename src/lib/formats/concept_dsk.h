// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*********************************************************************

    formats/concept_dsk.h

    Formats for Corvus Concept

*********************************************************************/
#ifndef MAME_FORMATS_CONCEPT_DSK_H
#define MAME_FORMATS_CONCEPT_DSK_H

#include "flopimg.h"

class cc525dsdd_format : public floppy_image_format_t
{
public:
	cc525dsdd_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, floppy_image *image) const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

	static const desc_e cc_9_desc[];

private:
	static void find_size(util::random_read &io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count);
};

extern const cc525dsdd_format FLOPPY_CONCEPT_525DSDD_FORMAT;

#endif // MAME_FORMATS_CONCEPT_DSK_H
