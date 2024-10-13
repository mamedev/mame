// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#ifndef MAME_BUS_VME_SYS68K_CPU1_H
#define MAME_BUS_VME_SYS68K_CPU1_H

#pragma once

#include "bus/vme/vme.h"

#include "machine/mm58167.h"
#include "machine/68230pit.h"
#include "machine/mc14411.h"
#include "machine/6850acia.h"
#include "bus/centronics/ctronics.h"
#include "bus/generic/carts.h"
#include "bus/generic/slot.h"

class vme_sys68k_cpu1_card_device
	: public device_t
	, public device_vme_card_interface
{
public:
	vme_sys68k_cpu1_card_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void fccpu1_eprom_sockets(machine_config &config);

	uint16_t vme_a24_r();
	void vme_a24_w(uint16_t data);
	uint16_t vme_a16_r();
	void vme_a16_w(uint16_t data);

	// Clocks
	void write_acia_clocks(int id, int state);
	void write_f1_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F1, state); }
	void write_f3_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F3, state); }
	void write_f5_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F5, state); }
	void write_f7_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F7, state); }
	void write_f8_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F8, state); }
	void write_f9_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F9, state); }
	void write_f11_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F11, state); }
	void write_f13_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F13, state); }
	void write_f15_clock(int state) { write_acia_clocks(mc14411_device::TIMER_F15, state); }

	// Centronics printer interface
	void centronics_ack_w(int state);
	void centronics_busy_w(int state);
	void centronics_perror_w(int state);
	void centronics_select_w(int state);

	// User EPROM/SRAM slot(s)
	std::pair<std::error_condition, std::string> force68k_load_cart(device_image_interface &image, generic_slot_device *slot);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(exp1_load) { return force68k_load_cart(image, m_cart); }
	uint16_t read16_rom(offs_t offset);

	void force68k_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<mm58167_device> m_rtc;
	required_device<pit68230_device> m_pit;
	required_device<mc14411_device> m_brg;
	required_device<acia6850_device> m_aciahost;
	required_device<acia6850_device> m_aciaterm;
	required_device<acia6850_device> m_aciaremt;
	optional_device<centronics_device> m_centronics;

	int32_t m_centronics_ack;
	int32_t m_centronics_busy;
	int32_t m_centronics_perror;
	int32_t m_centronics_select;

	// fake inputs for hardware configuration and things that need rewiring
	required_ioport             m_serial_brf;
	required_ioport             m_serial_p3;
	required_ioport             m_serial_p4;
	required_ioport             m_serial_p5;

	uint16_t *m_usrrom;

	required_device<generic_slot_device> m_cart;

	required_memory_region m_eprom;
	required_shared_ptr<uint16_t> m_ram;
	memory_passthrough_handler m_boot_mph;
};

DECLARE_DEVICE_TYPE(VME_SYS68K_CPU1, vme_sys68k_cpu1_card_device)

#endif // MAME_BUS_VME_SYS68K_CPU1_H
