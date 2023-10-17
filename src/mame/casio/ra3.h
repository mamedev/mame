// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*********************************************************************
    Casio CZ-series RAM cartridges
*********************************************************************/
#ifndef MAME_CASIO_RA3_H
#define MAME_CASIO_RA3_H

#pragma once

#include "imagedev/memcard.h"

#include <vector>

class casio_ra3_device : public device_t, public device_memcard_image_interface
{
public:
	casio_ra3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }
	virtual const char *image_type_name() const noexcept override { return "cartridge"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	bool present() { return is_loaded(); }

protected:
	virtual void device_start() override;

private:
	std::vector<u8> m_ram;
};

// device type definition
DECLARE_DEVICE_TYPE(CASIO_RA3, casio_ra3_device)

#endif // MAME_CASIO_RA3_H
