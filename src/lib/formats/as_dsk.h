// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Applesauce solved output formats

#ifndef MAME_FORMATS_AS_DSK_H
#define MAME_FORMATS_AS_DSK_H

#pragma once

#include "flopimg.h"
#include <utility>

class as_format : public floppy_image_format_t
{
public:
	as_format();

protected:
	struct tdata {
		std::vector<uint8_t> data;
		int track_size = 0;
		bool flux = false;
	};

	static bool load_bitstream_track(const uint8_t *img, floppy_image &image, int head, int track, int subtrack, uint8_t idx, uint32_t off_trks, bool may_be_short, bool set_variant);
	static void load_flux_track(const uint8_t *img, floppy_image &image, int head, int track, int subtrack, uint8_t fidx, uint32_t off_trks);

	static tdata analyze_for_save(const floppy_image &image, int head, int track, int subtrack, int speed_zone);
	static std::pair<int, int> count_blocks(const std::vector<tdata> &tracks);
	static bool test_flux(const std::vector<tdata> &tracks);
	static void save_tracks(std::vector<uint8_t> &data, const std::vector<tdata> &tracks, uint32_t total_blocks, bool has_flux);
};

class woz_format : public as_format
{
public:
	woz_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;
	virtual bool supports_save() const noexcept override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const uint8_t signature[8];
	static const uint8_t signature2[8];
};

extern const woz_format FLOPPY_WOZ_FORMAT;

class moof_format : public as_format
{
public:
	moof_format();

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;
	virtual bool supports_save() const noexcept override;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

private:
	static const uint8_t signature[8];
};

extern const moof_format FLOPPY_MOOF_FORMAT;

#endif
