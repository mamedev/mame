// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/***************************************************************************

    NABU PC Keyboard Interface

***************************************************************************/


#ifndef MAME_NABUPC_KEYBOARD_DEVICE_H
#define MAME_NABUPC_KEYBOARD_DEVICE_H

#pragma once

#include "cpu/m6800/m6801.h"
#include "machine/adc0808.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class nabupc_keyboard_device: public device_t
{
public:
	// construction/destruction
	nabupc_keyboard_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	auto rxd_cb() { return m_rxd_cb.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void nabu_kb_mem(address_map &map) ATTR_COLD;

	uint8_t port1_r();
	void port1_w(uint8_t data);

	void irq_w(uint8_t data);

	uint8_t gameport_r(offs_t offset);
	void adc_latch_w(offs_t offset, uint8_t data);
	void adc_start_w(offs_t offset, uint8_t data);
	void cpu_ack_irq_w(offs_t offset, uint8_t data);
	void ser_tx_w(uint8_t data);

	required_device<m6803_cpu_device> m_mcu;
	required_device<adc0809_device> m_adc;
	devcb_write_line m_rxd_cb;

	required_ioport m_modifiers;
	required_ioport_array<8> m_keyboard;
	required_ioport_array<4> m_gameport;

	uint8_t m_port1;
	uint8_t m_eoc;
};

DECLARE_DEVICE_TYPE(NABUPC_KEYBOARD, nabupc_keyboard_device)

#endif // MAME_NABUPC_KEYBOARD_DEVICE_H
