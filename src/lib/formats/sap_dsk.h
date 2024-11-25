// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_FORMATS_SAP_DSK_H
#define MAME_FORMATS_SAP_DSK_H

#pragma once

#include "flopimg.h"

class sap_dsk_format : public floppy_image_format_t
{
public:
	sap_dsk_format() = default;

	virtual const char *name() const noexcept override;
	virtual const char *description() const noexcept override;
	virtual const char *extensions() const noexcept override;
	virtual bool supports_save() const noexcept override;

	virtual int identify(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants) const override;
	virtual bool load(util::random_read &io, uint32_t form_factor, const std::vector<uint32_t> &variants, floppy_image &image) const override;

private:
	static inline uint16_t accumulate_crc(uint16_t crc, uint8_t data) noexcept;
};

extern const sap_dsk_format FLOPPY_SAP_FORMAT;

#endif // MAME_FORMATS_SAP_DSK_H
