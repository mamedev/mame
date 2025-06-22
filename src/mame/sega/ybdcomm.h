// license:BSD-3-Clause
// copyright-holders:Ariane Fugmann
#ifndef MAME_SEGA_YBDCOMM_H
#define MAME_SEGA_YBDCOMM_H

#pragma once

#define YBDCOMM_SIMULATION

#include "cpu/z80/z80.h"
#include "machine/mb8421.h"
//#include "machine/mb89372.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_ybdcomm_device : public device_t
{
public:
	sega_ybdcomm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// ex-bus connection to host
	uint8_t ex_r(offs_t offset);
	void ex_w(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<z80_device> m_cpu;
	required_device<mb8421_device> m_dpram;
	//required_device<mb89372_device> m_mpc;
	required_ioport m_dip_sw1;

	uint8_t m_ybd_stat; // not sure about those yet - 7474 for top bit? and 74161 for lower 4 bits
	uint8_t m_z80_stat; // not sure about those yet - 74LS374

	void ybdcomm_mem(address_map &map) ATTR_COLD;
	void ybdcomm_io(address_map &map) ATTR_COLD;

	// MB8421
	void dpram_int5_w(int state);

	// MB89372
	void mpc_hreq_w(int state);
	void mpc_int7_w(int state);
	uint8_t mpc_mem_r(offs_t offset);
	void mpc_mem_w(offs_t offset, uint8_t data);

	uint8_t z80_stat_r();
	void z80_stat_w(uint8_t data);

#ifdef YBDCOMM_SIMULATION
	emu_timer *m_tick_timer;

	class context;
	std::unique_ptr<context> m_context;

	uint8_t m_buffer[0x200];
	uint8_t m_framesync;

	uint8_t m_linkenable;
	uint16_t m_linktimer;
	uint8_t m_linkalive;
	uint8_t m_linkid;
	uint8_t m_linkcount;

	TIMER_CALLBACK_MEMBER(tick_timer_callback);

	unsigned comm_frame_offset(uint8_t cab_index);
	unsigned comm_frame_size(uint8_t cab_index);
	void comm_tick();
	unsigned read_frame(unsigned data_size);
	void send_data(uint8_t frame_type, unsigned frame_offset, unsigned frame_size, unsigned data_size);
	void send_frame(unsigned data_size);
#endif
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_YBOARD_COMM, sega_ybdcomm_device)

#endif // MAME_MACHINE_YBDCOMM_H
