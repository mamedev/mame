// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    BennVenn SD Loader for VZ300

***************************************************************************/

#ifndef MAME_BUS_VTECH_MEMEXP_SDLOADER_H
#define MAME_BUS_VTECH_MEMEXP_SDLOADER_H

#pragma once

#include "machine/spi_sdcard.h"
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
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void mem_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

private:
	required_device<spi_sdcard_device> m_sdcard;
	required_memory_bank m_dosbank;
	memory_view m_dosview;
	memory_bank_creator m_expbank;

	TIMER_CALLBACK_MEMBER(spi_clock);
	void spi_miso_w(int state);

	void mapper_w(uint8_t data);
	void sdcfg_w(uint8_t data);
	uint8_t sdio_r();
	void sdio_w(uint8_t data);
	void mode_w(uint8_t data);

	uint8_t exp_ram_r(offs_t offset);
	void exp_ram_w(offs_t offset, uint8_t data);

	emu_timer *m_spi_clock;
	bool m_spi_clock_state;
	bool m_spi_clock_sysclk;
	int m_spi_clock_cycles;
	int m_in_bit;
	uint8_t m_in_latch;
	uint8_t m_out_latch;

	std::unique_ptr<uint8_t[]> m_ram;
	bool m_vz300_mode;
};

// device type definition
DECLARE_DEVICE_TYPE(VTECH_SDLOADER, vtech_sdloader_device)

#endif // MAME_BUS_VTECH_MEMEXP_SDLOADER_H
