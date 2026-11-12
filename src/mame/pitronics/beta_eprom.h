// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_PITRONICS_BETA_EPROM_H
#define MAME_PITRONICS_BETA_EPROM_H

#pragma once

#include "imagedev/memcard.h"

class beta_eprom_device : public device_t, public device_memcard_image_interface
{
public:
	beta_eprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t read(offs_t offset) const { return m_rom[offset]; }
	void program(offs_t offset, uint8_t data) { m_rom[offset] &= data; }

protected:
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

private:
	std::vector<uint8_t> m_rom;
};

DECLARE_DEVICE_TYPE(BETA_EPROM, beta_eprom_device)

#endif // MAME_PITRONICS_BETA_EPROM_H
