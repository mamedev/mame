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

constexpr int APPLE2_STD_TRACK_COUNT = 35;
constexpr int APPLE2_TRACK_COUNT = 40;
constexpr int APPLE2_SECTOR_SIZE = 256;
constexpr int APPLE2_MAX_SECTOR_COUNT = 16;

/**************************************************************************/
class a2_sect_format : public floppy_image_format_t
{
public:
	a2_sect_format(int nsect);

	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;

	virtual bool supports_save() const noexcept override;

protected:
	class byte_reader;

private:
	const int m_nsect;

	virtual bool check_dosver(int dosver) const = 0;
	virtual void decode_sector_data(
			byte_reader &br, uint8_t (&decoded_buf)[APPLE2_SECTOR_SIZE],
			uint8_t &dchk_expected, uint8_t &dchk_actual) const = 0;
	virtual int logical_sector_index(int physical) const = 0;
};

class a2_13sect_format : public a2_sect_format
{
public:
	static constexpr int SECTOR_COUNT = 13;

	a2_13sect_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	virtual bool check_dosver(int dosver) const override;
	virtual void decode_sector_data(
			byte_reader &br, uint8_t (&decoded_buf)[APPLE2_SECTOR_SIZE],
			uint8_t &dchk_expected, uint8_t &dchk_actual) const override;
	virtual int logical_sector_index(int physical) const override;
};

extern const a2_13sect_format FLOPPY_A213S_FORMAT;

class a2_16sect_format : public a2_sect_format
{
public:
	static constexpr int SECTOR_COUNT = 16;

	a2_16sect_format(bool prodos_order);

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

private:
	const bool m_prodos_order;

	virtual bool check_dosver(int dosver) const override;
	virtual void decode_sector_data(
			byte_reader &br, uint8_t (&decoded_buf)[APPLE2_SECTOR_SIZE],
			uint8_t &dchk_expected, uint8_t &dchk_actual) const override;
	virtual int logical_sector_index(int physical) const override;
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


class a2_edd_format : public floppy_image_format_t
{
public:
	a2_edd_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

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

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;


private:
	static constexpr size_t nibbles_per_track = 0x1a00;
	static constexpr size_t min_sync_bytes = 4;
	static constexpr auto expected_size_35t = APPLE2_STD_TRACK_COUNT * nibbles_per_track;
	static constexpr auto expected_size_40t = APPLE2_TRACK_COUNT * nibbles_per_track;

	static std::vector<uint32_t> generate_levels_from_nibbles(const std::vector<uint8_t>& nibbles);
};

extern const a2_nib_format FLOPPY_NIB_FORMAT;

#endif // MAME_FORMATS_AP2_DSK_H
