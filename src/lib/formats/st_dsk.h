// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/st_dsk.h

    Atari ST 9/10/11 sector-per-track formats

*********************************************************************/
#ifndef MAME_FORMATS_ST_DSK_H
#define MAME_FORMATS_ST_DSK_H

#pragma once

#include "flopimg.h"

class st_format : public floppy_image_format_t
{
public:
	st_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	void find_size(io_generic *io, uint8_t &track_count, uint8_t &head_count, uint8_t &sector_count);
};

class msa_format : public floppy_image_format_t
{
public:
	msa_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	bool uncompress(uint8_t *buffer, int csize, int usize);
	bool compress(const uint8_t *src, int usize, uint8_t *dest, int &csize);
	void read_header(io_generic *io, uint16_t &sign, uint16_t &sect, uint16_t &head, uint16_t &strack, uint16_t &etrack);
};

extern const floppy_format_type FLOPPY_ST_FORMAT;
extern const floppy_format_type FLOPPY_MSA_FORMAT;

#endif // MAME_FORMATS_ST_DSK_H
