// license:BSD-3-Clause
// copyright-holders:Sven Schnelle
#ifndef MAME_BUS_ISA_HPBLP_H
#define MAME_BUS_ISA_HPBLP_H

#pragma once

#include "isa.h"

#include "bus/hp_dio/hp_dio.h"
#include "bus/ieee488/ieee488.h"
#include "cpu/m68000/m68000.h"
#include "machine/tms9914.h"

class isa8_hpblp_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	isa8_hpblp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	// ISA part
	void isamap(address_map &map) ATTR_COLD;

	uint8_t datal_r(offs_t offset);
	uint8_t datah_r(offs_t offset);
	uint8_t status_r(offs_t offset);
	uint8_t addrh_r(offs_t offset);
	uint8_t addrm_r(offs_t offset);
	uint8_t addrl_r(offs_t offset);
	uint8_t reg5_r(offs_t offset);
	uint8_t reg6_r(offs_t offset);

	void datal_w(offs_t offset, uint8_t data);
	void datah_w(offs_t offset, uint8_t data);
	void irq_w(offs_t offset, uint8_t data);
	void reg2_w(offs_t offset, uint8_t data);
	void reg3_w(offs_t offset, uint8_t data);
	void reg4_w(offs_t offset, uint8_t data);
	void reg5_w(offs_t offset, uint8_t data);
	void reg6_w(offs_t offset, uint8_t data);
	void reg7_w(address_space &space, offs_t offset, uint8_t data);

	static uint8_t status_val(offs_t offset);

	void hpblp_interrupt(int state);
	TIMER_CALLBACK_MEMBER(timer10ms);

	// M68000 part
	void gpib_irq(int state);
	void m68map(address_map &map) ATTR_COLD;

	uint16_t bus_r(offs_t offset, uint16_t mem_mask);
	void bus_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t gpib_r(offs_t offset);
	void gpib_w(offs_t offset, uint16_t data);

	static offs_t get_bus_address(offs_t offset, uint16_t mem_mask);
	uint8_t get_bus_address_partial(int shift);

	static bool forward_to_host(offs_t offset);
	void update_gpib_irq();

	required_device<m68000_device> m_maincpu;
	required_device<tms9914_device> m_tms9914;
	required_device<ieee488_device> m_ieee488;
	emu_timer *m_timer_10ms;

	uint32_t m_bus_address;
	uint16_t m_bus_mem_mask;
	uint16_t m_bus_data;
	uint8_t m_gpib_reg1;
	uint8_t m_reg5;
	uint8_t m_reg6;
	uint8_t m_irq;

	bool m_installed;
	bool m_ack_buscycle;
	bool m_bus_read;
	bool m_bus_write;
	bool m_irq_state;
	bool m_reset;
};

// device type definition
DECLARE_DEVICE_TYPE(HPBLP, isa8_hpblp_device)

#endif // MAME_BUS_ISA_HPBLP_H
