// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_BUS_VME_ENP10_H
#define MAME_BUS_VME_ENP10_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/am79c90.h"
#include "machine/timer.h"
#include "bus/vme/vme.h"

class vme_enp10_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_enp10_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void cpu_map(address_map &map) ATTR_COLD;
	void cpu_ack(address_map &map) ATTR_COLD;
	void vme_map(address_map &map) ATTR_COLD;

	// vme bus outgoing interrupt
	u8 vect_r();
	void vect_w(u8 data);
	u8 iack_r();

	// one-bit register handlers
	u8 obr_r(offs_t offset);
	void obr_w(offs_t offset, u8 data);

	void timer(timer_device &timer, s32 param);
	void interrupt();

	// vme bus access handlers
	u16 vme_a16_r(offs_t offset, u16 mem_mask);
	void vme_a16_w(offs_t offset, u16 data, u16 mem_mask);
	u16 vme_a24_r(offs_t offset, u16 mem_mask);
	void vme_a24_w(offs_t offset, u16 data, u16 mem_mask);

	required_device<m68000_device> m_cpu;
	required_device<am7990_device> m_net;

	output_finder<2> m_led;
	required_ioport m_base;

	u8 m_ivr; // interrupt vector register
	u8 m_csr; // control/status register
	u8 m_obr; // one-bit registers
	u8 m_exr; // exception register

	bool m_bint;    // host to card interrupt
	bool m_lint;    // lance interrupt
	u8 m_int_state; // current interrupt state

	memory_view m_boot;
};

DECLARE_DEVICE_TYPE(VME_ENP10, vme_enp10_card_device)

#endif // MAME_BUS_VME_ENP10_H
