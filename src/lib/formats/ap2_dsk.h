// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/*********************************************************************

    ap2_dsk.h

    Apple II disk images

*********************************************************************/
#ifndef MAME_FORMATS_AP2_DSK_H
#define MAME_FORMATS_AP2_DSK_H

#pragma once

#include "flopimg.h"


/***************************************************************************

    Constants

***************************************************************************/

#define APPLE2_NIBBLE_SIZE              416
#define APPLE2_SMALL_NIBBLE_SIZE        374
//#define APPLE2_TRACK_COUNT              35
#define APPLE2_TRACK_COUNT              40
#define APPLE2_SECTOR_COUNT             16
#define APPLE2_SECTOR_SIZE              256



/**************************************************************************/

LEGACY_FLOPPY_OPTIONS_EXTERN(apple2);

// new style code

class a2_16sect_format : public floppy_image_format_t
{
public:
	a2_16sect_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	static const desc_e mac_gcr[];

	uint8_t gb(const uint8_t *buf, int ts, int &pos, int &wrap);
	void update_chk(const uint8_t *data, int size, uint32_t &chk);

	bool m_prodos_order;

	int m_tracks;
};

extern const floppy_format_type FLOPPY_A216S_FORMAT;

class a2_rwts18_format : public floppy_image_format_t
{
public:
	a2_rwts18_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool save(io_generic *io, floppy_image *image) override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;
	virtual bool supports_save() const override;

private:
	static const desc_e mac_gcr[];

	uint8_t gb(const uint8_t *buf, int ts, int &pos, int &wrap);
	void update_chk(const uint8_t *data, int size, uint32_t &chk);
};

extern const floppy_format_type FLOPPY_RWTS18_FORMAT;


class a2_edd_format : public floppy_image_format_t
{
public:
	a2_edd_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool supports_save() const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static uint8_t pick(const uint8_t *data, int pos);
};

extern const floppy_format_type FLOPPY_EDD_FORMAT;

class a2_woz_format : public floppy_image_format_t
{
public:
	a2_woz_format();

	virtual int identify(io_generic *io, uint32_t form_factor) override;
	virtual bool load(io_generic *io, uint32_t form_factor, floppy_image *image) override;
	virtual bool supports_save() const override;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

private:
	static const uint8_t signature[8];
	static const uint8_t signature2[8];

	static uint32_t r32(const std::vector<uint8_t> &data, uint32_t offset);
	static uint16_t r16(const std::vector<uint8_t> &data, uint32_t offset);
	static uint8_t r8(const std::vector<uint8_t> &data, uint32_t offset);
	static uint32_t crc32r(const uint8_t *data, uint32_t size);
	static uint32_t find_tag(const std::vector<uint8_t> &data, uint32_t tag);
};

extern const floppy_format_type FLOPPY_WOZ_FORMAT;

#endif // MAME_FORMATS_AP2_DSK_H
