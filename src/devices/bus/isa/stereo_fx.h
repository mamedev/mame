// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_STEREO_FX_H
#define MAME_BUS_ISA_STEREO_FX_H

#pragma once

#include "isa.h"
#include "bus/pc_joy/pc_joy.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ymopl.h"

//*********************************************************************
//   TYPE DEFINITIONS
//*********************************************************************

// ====================> stereo_fx_device

class stereo_fx_device : public device_t,
						public device_isa8_card_interface
{
public:
	// construction/destruction
	stereo_fx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	uint8_t dack_r(int line) override;
	void dack_w(int line, uint8_t data) override;

	required_device<pc_joy_device> m_joy;
	required_device<i80c31_device> m_cpu;

private:
	// internal state
	bool m_data_in;
	uint8_t m_in_byte;
	bool m_data_out;
	uint8_t m_out_byte;

	uint8_t m_port20;
	uint8_t m_port00;
	emu_timer *m_timer;
	uint8_t m_t0;
	uint8_t m_t1;

	TIMER_CALLBACK_MEMBER(clock_tick);

	// mcu ports
	uint8_t dev_dsp_data_r();
	void dev_dsp_data_w(uint8_t data);
	uint8_t p1_r();
	uint8_t p3_r();
	void p3_w(uint8_t data);
	void dev_host_irq_w(uint8_t data);
	void raise_drq_w(uint8_t data);
	void port20_w(uint8_t data);
	void port00_w(uint8_t data);

	// host ports
	uint8_t dsp_data_r();
	void dsp_cmd_w(uint8_t data);
	void dsp_reset_w(uint8_t data);
	uint8_t dsp_wbuf_status_r();
	uint8_t dsp_rbuf_status_r();
	uint8_t invalid_r();
	void invalid_w(uint8_t data);

	void stereo_fx_io(address_map &map) ATTR_COLD;
	void stereo_fx_rom(address_map &map) ATTR_COLD;
};

// device type definition

DECLARE_DEVICE_TYPE(ISA8_STEREO_FX, stereo_fx_device)

#endif // MAME_BUS_ISA_STEREO_FX_H
