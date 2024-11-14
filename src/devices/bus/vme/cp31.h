// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_BUS_VME_CP31_H
#define MAME_BUS_VME_CP31_H

#pragma once

#include "vme.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68030.h"
#include "machine/68153bim.h"
#include "machine/68230pit.h"
#include "machine/68561mpcc.h"
#include "machine/clock.h"
#include "machine/msm6242.h"

DECLARE_DEVICE_TYPE(VME_CP31, vme_cp31_card_device)

class vme_cp31_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_cp31_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_cp31_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(bus_error_off);

private:
	required_device<m68000_musashi_device> m_maincpu;
	required_device<bim68153_device> m_bim;
	required_device<mpcc68561_device> m_mpcc;
	required_device<rtc62421_device> m_rtc;
	required_device<pit68230_device> m_pit1;
	required_device<pit68230_device> m_pit2;
	required_shared_ptr<uint32_t> m_p_ram;
	required_region_ptr<uint32_t> m_sysrom;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	memory_passthrough_handler m_rom_shadow_tap;
	bool m_bus_error;
	emu_timer *m_bus_error_timer;

	uint8_t bim_irq_state;
	int bim_irq_level;

	void bim_irq_callback(int state);
	void update_irq_to_maincpu();

	void pit1_pb_w(uint8_t data);
	uint8_t pit1_pc_r();
	void pit1_pc_w(uint8_t data);

	uint32_t trap_r(offs_t offset, uint32_t mem_mask);
	void trap_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void set_bus_error(uint32_t address, bool write, uint32_t mem_mask);

	void cp31_mem(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
};

#endif // MAME_BUS_VME_CP31_H
