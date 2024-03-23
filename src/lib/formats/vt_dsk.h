// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    formats/vt_dsk.h

    VTech disk image formats

*********************************************************************/
#ifndef MAME_FORMATS_VT_DSK_H
#define MAME_FORMATS_VT_DSK_H

#pragma once

#include "flopimg.h"

class vtech_common_format : public floppy_image_format_t {
public:
	virtual bool supports_save() const noexcept override { return true; }

protected:
	static void image_to_flux(const uint8_t *bdata, size_t size, floppy_image &image);
	static std::vector<uint8_t> flux_to_image(const floppy_image &image);

	static void wbit(std::vector<uint32_t> &buffer, uint32_t &pos, bool bit);
	static void wbyte(std::vector<uint32_t> &buffer, uint32_t &pos, uint8_t byte);
};

class vtech_bin_format : public vtech_common_format {
public:
	vtech_bin_format() = default;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;
};

class vtech_dsk_format : public vtech_common_format {
public:
	vtech_dsk_format() = default;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;
	virtual bool save(util::random_read_write &io, const std::vector<uint32_t> &variants, const floppy_image &image) const override;
};

extern const vtech_bin_format FLOPPY_VTECH_BIN_FORMAT;
extern const vtech_dsk_format FLOPPY_VTECH_DSK_FORMAT;

#endif // MAME_FORMATS_VT_DSK_H
