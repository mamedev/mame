// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/st_dsk.h

    Atari ST 9/10/11 sector-per-track formats

*********************************************************************/

#ifndef ST_DSK_H_
#define ST_DSK_H_

#include "flopimg.h"

class st_format : public floppy_image_format_t
{
public:
	st_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	void find_size(io_generic *io, UINT8 &track_count, UINT8 &head_count, UINT8 &sector_count);
};

class msa_format : public floppy_image_format_t
{
public:
	msa_format();

	virtual int identify(io_generic *io, UINT32 form_factor) override;
	virtual bool load(io_generic *io, UINT32 form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	bool uncompress(UINT8 *buffer, int csize, int usize);
	bool compress(const UINT8 *src, int usize, UINT8 *dest, int &csize);
	void read_header(io_generic *io, UINT16 &sign, UINT16 &sect, UINT16 &head, UINT16 &strack, UINT16 &etrack);
};

extern const floppy_format_type FLOPPY_ST_FORMAT;
extern const floppy_format_type FLOPPY_MSA_FORMAT;

#endif /*ST_DSK_H_*/
