// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef __STEREO_FX__
#define __STEREO_FX__

#include "emu.h"
#include "isa.h"
#include "sound/dac.h"
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
	stereo_fx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<dac_device> m_dacl;
	required_device<dac_device> m_dacr;
	required_device<pc_joy_device> m_joy;
	required_device<cpu_device> m_cpu;

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

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	UINT8 dack_r(int line);
	void dack_w(int line, UINT8 data);
private:
	// internal state
	bool m_data_in;
	UINT8 m_in_byte;
	bool m_data_out;
	UINT8 m_out_byte;

	UINT8 m_port20;
	UINT8 m_port00;
	emu_timer *m_timer;
	UINT8 m_t0;
	UINT8 m_t1;
};

// device type definition

extern const device_type ISA8_STEREO_FX;

#endif
