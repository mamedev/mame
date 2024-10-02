// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Psion Organiser II series

****************************************************************************/
#ifndef MAME_PSION_PSION_H
#define MAME_PSION_PSION_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "machine/nvram.h"
#include "psion_pack.h"
#include "machine/timer.h"
#include "video/hd44780.h"
#include "sound/beep.h"
#include "emupal.h"


// ======================> psion_state

class psion_state : public driver_device
{
public:
	psion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd44780")
		, m_beep(*this, "beeper")
		, m_pack1(*this, "pack1")
		, m_pack2(*this, "pack2")
		, m_nvram1(*this, "nvram1")
		, m_nvram2(*this, "nvram2")
		, m_ram(*this, "ram")
		, m_kb_lines(*this, "K%u", 1U)
	{ }

	void psion_2lines(machine_config &config);
	void psion_4lines(machine_config &config);
	void psionlam(machine_config &config);
	void psioncm(machine_config &config);
	void psionlz(machine_config &config);
	void psionla(machine_config &config);
	void psionp350(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(psion_on);

protected:
	required_device<hd6301x_cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<beep_device> m_beep;
	required_device<datapack_device> m_pack1;
	required_device<datapack_device> m_pack2;
	required_device<nvram_device> m_nvram1;
	optional_device<nvram_device> m_nvram2;

	uint16_t m_kb_counter = 0;
	bool m_enable_nmi = false;
	uint8_t m_pulse = 0;

	// RAM/ROM banks
	required_shared_ptr<uint8_t> m_ram;
	std::unique_ptr<uint8_t[]> m_paged_ram;
	uint8_t m_rom_bank = 0;
	uint8_t m_ram_bank = 0;
	uint8_t m_ram_bank_count = 0;
	uint8_t m_rom_bank_count = 0;

	required_ioport_array<7> m_kb_lines;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void nvram_init(nvram_device &nvram, void *data, size_t size);

	uint8_t kb_read();
	void update_banks();
	void port2_w(offs_t offset, uint8_t data, uint8_t ddr);
	uint8_t port2_r();
	uint8_t port5_r();
	void port6_w(uint8_t data);
	uint8_t port6_r();
	void io_rw(uint16_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void psion_palette(palette_device &palette) const;
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_timer);

	HD44780_PIXEL_UPDATE(lz_pixel_update);
	void psion_int_reg(address_map &map) ATTR_COLD;
	void psioncm_mem(address_map &map) ATTR_COLD;
	void psionla_mem(address_map &map) ATTR_COLD;
	void psionlam_mem(address_map &map) ATTR_COLD;
	void psionlz_mem(address_map &map) ATTR_COLD;
	void psionp350_mem(address_map &map) ATTR_COLD;
};


class psion1_state : public psion_state
{
public:
	psion1_state(const machine_config &mconfig, device_type type, const char *tag)
		: psion_state(mconfig, type, tag)
	{ }

	void psion1(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;

	uint8_t reset_kb_counter_r();
	uint8_t inc_kb_counter_r();
	uint8_t switchoff_r();

	HD44780_PIXEL_UPDATE(psion1_pixel_update);
	void psion1_mem(address_map &map) ATTR_COLD;
};

#endif // MAME_PSION_PSION_H
