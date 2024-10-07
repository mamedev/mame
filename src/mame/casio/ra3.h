// license:BSD-3-Clause
// copyright-holders:Devin Acker
/*********************************************************************
    Casio CZ-series RAM cartridges
*********************************************************************/
#ifndef MAME_CASIO_RA3_H
#define MAME_CASIO_RA3_H

#pragma once

#include "imagedev/memcard.h"
#include "softlist_dev.h"

#include <vector>

class casio_ram_cart_device : public device_t, public device_memcard_image_interface
{
public:
	virtual const char *image_interface() const noexcept override { return "cz_cart"; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }
	virtual const char *image_type_name() const noexcept override { return "cartridge"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart"; }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	casio_ram_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, unsigned max_size);

	virtual void device_start() override ATTR_COLD;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

private:
	std::vector<u8> m_ram;
	const unsigned m_max_size;
	unsigned m_size, m_mask;
};

class casio_ra3_device : public casio_ram_cart_device
{
public:
	casio_ra3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class casio_ra5_device : public casio_ram_cart_device
{
public:
	casio_ra5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

class casio_ra6_device : public casio_ram_cart_device
{
public:
	casio_ra6_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// device type definition
DECLARE_DEVICE_TYPE(CASIO_RA3, casio_ra3_device)
DECLARE_DEVICE_TYPE(CASIO_RA5, casio_ra5_device)
DECLARE_DEVICE_TYPE(CASIO_RA6, casio_ra6_device)

#endif // MAME_CASIO_RA3_H
