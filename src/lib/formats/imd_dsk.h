// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

    formats/imd_dsk.h

    IMD disk images

*********************************************************************/
#ifndef MAME_FORMATS_IMD_DSK_H
#define MAME_FORMATS_IMD_DSK_H

#include "flopimg.h"

class imd_format : public floppy_image_format_t
{
public:
	imd_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic* io, floppy_image* image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	void fixnum(char *start, char *end) const;

	std::vector<uint8_t> m_comment;
	std::vector<std::vector<uint8_t> > m_snum;
	std::vector<std::vector<uint8_t> > m_tnum;
	std::vector<std::vector<uint8_t> > m_hnum;

	std::vector<uint8_t> m_mode;
	std::vector<uint8_t> m_track;
	std::vector<uint8_t> m_head;
	std::vector<uint8_t> m_sector_count;
	std::vector<uint8_t> m_ssize;
};

extern const floppy_format_type FLOPPY_IMD_FORMAT;

#endif // MAME_FORMATS_IMD_DSK_H
