// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

    Saitek OSA Module: Kasparov Maestro/Analyst

***************************************************************************/

#ifndef MAME_BUS_SAITEKOSA_MAESTRO_H
#define MAME_BUS_SAITEKOSA_MAESTRO_H

#pragma once

#include "expansion.h"

#include "bus/generic/slot.h"
#include "video/hd44780.h"

DECLARE_DEVICE_TYPE(OSA_MAESTRO, saitekosa_maestro_device)
DECLARE_DEVICE_TYPE(OSA_ANALYST, saitekosa_analyst_device)


class saitekosa_maestro_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_maestro_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(switch_cpu_freq) { set_cpu_freq(); }

	// from host
	virtual u8 data_r() override;
	virtual void nmi_w(int state) override;
	virtual void ack_w(int state) override;

protected:
	saitekosa_maestro_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<cpu_device> m_maincpu;
	memory_share_creator<u8> m_banked_ram;
	required_memory_bank m_rambank;
	required_memory_bank m_rombank;
	required_device<generic_slot_device> m_extrom;

	virtual void main_map(address_map &map);

	u8 extrom_r(offs_t offset);
	template <int N> void stall_w(u8 data = 0);
	u8 rts_r();
	u8 xdata_r();
	void xdata_w(u8 data);
	u8 ack_r();
	void control_w(u8 data);

	void set_cpu_freq();

	u8 m_latch = 0xff;
	bool m_latch_enable = false;
	u8 m_extrom_bank = 0;
};

class saitekosa_analyst_device : public saitekosa_maestro_device
{
public:
	saitekosa_analyst_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	static auto parent_rom_device_type() { return &OSA_MAESTRO; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_config_complete() override;

private:
	required_device<hd44780_device> m_lcd;

	virtual void main_map(address_map &map) override;
};

#endif // MAME_BUS_SAITEKOSA_MAESTRO_H
