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

#define APPLE2_NIBBLE_SIZE 416
#define APPLE2_SMALL_NIBBLE_SIZE 374
#define APPLE2_STD_TRACK_COUNT 35
#define APPLE2_TRACK_COUNT 40
#define APPLE2_SECTOR_COUNT 16
#define APPLE2_SECTOR_SIZE 256

/**************************************************************************/
class a2_13sect_format : public floppy_image_format_t
{
public:
	a2_13sect_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

private:
	static uint8_t gb(const std::vector<bool> &buf, int &pos, int &wrap);
};

extern const a2_13sect_format FLOPPY_A213S_FORMAT;

class a2_16sect_format : public floppy_image_format_t
{
public:
	a2_16sect_format(bool prodos_order);

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual bool supports_save() const noexcept override;

private:
	const bool m_prodos_order;

	static uint8_t gb(const std::vector<bool> &buf, int &pos, int &wrap);
};

class a2_16sect_dos_format : public a2_16sect_format
{
public:
	a2_16sect_dos_format();
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
};

class a2_16sect_prodos_format : public a2_16sect_format
{
public:
	a2_16sect_prodos_format();
	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
};

extern const a2_16sect_dos_format FLOPPY_A216S_DOS_FORMAT;
extern const a2_16sect_prodos_format FLOPPY_A216S_PRODOS_FORMAT;

class a2_rwts18_format : public floppy_image_format_t
{
public:
	a2_rwts18_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

private:
	static const desc_e mac_gcr[];

	static uint8_t gb(const std::vector<bool> &buf, int &pos, int &wrap);
};

extern const a2_rwts18_format FLOPPY_RWTS18_FORMAT;


class a2_edd_format : public floppy_image_format_t
{
public:
	a2_edd_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool supports_save() const noexcept override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static uint8_t pick(const uint8_t *data, int pos);
};

extern const a2_edd_format FLOPPY_EDD_FORMAT;

class a2_nib_format : public floppy_image_format_t
{
public:
	a2_nib_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool supports_save() const noexcept override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;


private:
	static constexpr size_t nibbles_per_track = 0x1a00;
	static constexpr size_t min_sync_bytes = 4;
	static constexpr auto expected_size_35t = 35 * nibbles_per_track;
	static constexpr auto expected_size_40t = 40 * nibbles_per_track;

	static std::vector<uint32_t> generate_levels_from_nibbles(const std::vector<uint8_t>& nibbles);
};

extern const a2_nib_format FLOPPY_NIB_FORMAT;

#endif // MAME_FORMATS_AP2_DSK_H
