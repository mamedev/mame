// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  Namco System 12 CDXA PCB

***************************************************************************/

#ifndef MAME_NAMCO_NAMCOS12_CDXA_H
#define MAME_NAMCO_NAMCOS12_CDXA_H

#pragma once

#include "bus/ata/ataintf.h"
#include "cpu/sh/sh7014.h"
#include "machine/icd2061a.h"
#include "machine/mb87078.h"
#include "sound/lc78836m.h"


class namcos12_cdxa_device : public device_t
{
public:
	namcos12_cdxa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <typename... T> void add_route(T &&... args) { m_lc78836m.lookup()->add_route(std::forward<T>(args)...); }

	auto psx_int10_callback() { return m_psx_int10_cb.bind(); }

	void psx_map(address_map &map) ATTR_COLD;

	uint32_t sh2_ram_r(offs_t offset);
	void sh2_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void reset_sh2_w(uint16_t data);

	void ide_sh2_enabled_w(uint16_t data);

	void ide_ps1_enabled_w(uint16_t data);

	void sram_enabled_w(uint16_t data);

	void ps1_int10_finished_w(uint16_t data);

	void volume_w(offs_t offset, uint16_t data);

	void clockgen_w(offs_t offset, uint16_t data);

	uint16_t cdrom_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void cdrom_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void sh7014_map(address_map &map) ATTR_COLD;

	void audio_dac_w(int state);

	void portb_w(uint16_t data);

	uint16_t cdrom_status_flag_r(offs_t offset, uint16_t mem_mask = ~0);
	void trigger_psx_int10_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t sh2_cdrom_cs0_r(offs_t offset, uint16_t mem_mask = ~0);
	void sh2_cdrom_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void mb87078_gain_changed(offs_t offset, uint8_t data);

	required_device<sh7014_device> m_maincpu;
	required_shared_ptr<uint32_t> m_cram;
	required_shared_ptr<uint32_t> m_sram;
	required_device<ata_interface_device> m_ata;
	required_device<lc78836m_device> m_lc78836m;
	required_device <mb87078_device> m_mb87078;
	required_device <icd2061a_device> m_icd2061a;
	devcb_write_line m_psx_int10_cb;

	bool m_ide_sh2_enabled, m_ide_ps1_enabled;
	bool m_sram_enabled;
	bool m_psx_int10_busy;

	uint8_t m_audio_cur_bit;
	uint8_t m_audio_lrck;

	uint8_t m_volume_write_counter;
};

DECLARE_DEVICE_TYPE(NAMCOS12_CDXA, namcos12_cdxa_device)

#endif // MAME_NAMCO_NAMCOS12_CDXA_H
