// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_BUS_ISA_STEREO_FX_H
#define MAME_BUS_ISA_STEREO_FX_H

#pragma once

#include "isa.h"
#include "bus/pc_joy/pc_joy.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/3812intf.h"

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
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

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

	// mcu ports
	DECLARE_READ8_MEMBER( dev_dsp_data_r );
	DECLARE_WRITE8_MEMBER( dev_dsp_data_w );
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_READ8_MEMBER( p3_r );
	DECLARE_WRITE8_MEMBER( p3_w );
	DECLARE_WRITE8_MEMBER( dev_host_irq_w );
	DECLARE_WRITE8_MEMBER( raise_drq_w );
	DECLARE_WRITE8_MEMBER( port20_w );
	DECLARE_WRITE8_MEMBER( port00_w );

	// host ports
	DECLARE_READ8_MEMBER( dsp_data_r );
	DECLARE_WRITE8_MEMBER( dsp_cmd_w );
	DECLARE_WRITE8_MEMBER( dsp_reset_w );
	DECLARE_READ8_MEMBER( dsp_wbuf_status_r );
	DECLARE_READ8_MEMBER( dsp_rbuf_status_r );
	DECLARE_READ8_MEMBER( invalid_r );
	DECLARE_WRITE8_MEMBER( invalid_w );

	void stereo_fx_io(address_map &map);
	void stereo_fx_rom(address_map &map);
};

// device type definition

DECLARE_DEVICE_TYPE(ISA8_STEREO_FX, stereo_fx_device)

#endif // MAME_BUS_ISA_STEREO_FX_H
