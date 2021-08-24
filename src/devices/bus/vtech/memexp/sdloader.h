// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    BennVenn SD Loader for VZ300

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_SDLOADER_H
#define MAME_BUS_VTECH_MEMEXP_SDLOADER_H

#pragma once

#include "memexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vtech_sdloader_device

class vtech_sdloader_device : public vtech_memexp_device
{
public:
	// construction/destruction
	vtech_sdloader_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void mem_map(address_map &map) override;
	virtual void io_map(address_map &map) override;

private:
	required_memory_bank m_dosbank;
	memory_view m_dosview;
	memory_bank_creator m_expbank;

	void mapper_w(uint8_t data);
	void sdcfg_w(uint8_t data);
	uint8_t sdio_r();
	void sdio_w(uint8_t data);
	void mode_w(uint8_t data);

	uint8_t exp_ram_r(offs_t offset);
	void exp_ram_w(offs_t offset, uint8_t data);

	std::unique_ptr<uint8_t[]> m_ram;
	bool m_vz300_mode;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_SDLOADER, vtech_sdloader_device)

#endif // MAME_BUS_VTECH_MEMEXP_SDLOADER_H
