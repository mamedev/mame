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
	virtual bool supports_save() const override { return true; }

protected:
	static void image_to_flux(const std::vector<uint8_t> &bdata, floppy_image *image);
	static std::vector<uint8_t> flux_to_image(floppy_image *image);

	static void wbit(std::vector<uint32_t> &buffer, uint32_t &pos, uint32_t &mg, bool bit);
	static void wbyte(std::vector<uint32_t> &buffer, uint32_t &pos, uint32_t &mg, uint8_t byte);
};

class vtech_bin_format : public vtech_common_format {
public:
	vtech_bin_format() = default;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(io_generic *io, const std::vector<uint32_t> &variants, floppy_image *image) override;
};

class vtech_dsk_format : public vtech_common_format {
public:
	vtech_dsk_format() = default;

	virtual const char *name() const override;
	virtual const char *description() const override;
	virtual const char *extensions() const override;

	virtual int identify(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants) override;
	virtual bool load(io_generic *io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image *image) override;
	virtual bool save(io_generic *io, const std::vector<uint32_t> &variants, floppy_image *image) override;
};

extern const floppy_format_type FLOPPY_VTECH_BIN_FORMAT;
extern const floppy_format_type FLOPPY_VTECH_DSK_FORMAT;

#endif // MAME_FORMATS_VT_DSK_H
