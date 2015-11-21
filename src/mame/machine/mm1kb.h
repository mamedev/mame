// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Nokia MikroMikko 1 keyboard emulation

*********************************************************************/

#pragma once

#ifndef __MM1_KEYBOARD__
#define __MM1_KEYBOARD__

#include "emu.h"
#include "sound/samples.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MM1_KEYBOARD_KBST_CALLBACK(_write) \
	devcb = &mm1_keyboard_t::set_kbst_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mm1_keyboard_t

class mm1_keyboard_t :  public device_t
{
public:
	// construction/destruction
	mm1_keyboard_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_kbst_wr_callback(device_t &device, _Object object) { return downcast<mm1_keyboard_t &>(device).m_write_kbst.set_callback(object); }

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER( read ) { return m_data; }

	DECLARE_WRITE_LINE_MEMBER( bell_w )
	{
		if (state == 1)
		{
			if (first_time)
			{
				m_samples->start(1, 1); // power switch
				first_time = false;
			}
			if (!m_samples->playing(0)) m_samples->start(0, 0); // beep; during boot, the second beep is in real HW very short (just before floppy seeks) but that's NYI
		}
		else if (m_samples->playing(0)) m_samples->stop(0); // happens only once during boot, no effect on output
	}
	void shut_down_mm1();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	devcb_write_line m_write_kbst;

	required_device<samples_device> m_samples;
	required_memory_region m_rom;
	required_ioport m_y0;
	required_ioport m_y1;
	required_ioport m_y2;
	required_ioport m_y3;
	required_ioport m_y4;
	required_ioport m_y5;
	required_ioport m_y6;
	required_ioport m_y7;
	required_ioport m_y8;
	required_ioport m_y9;
	required_ioport m_special;

	int m_sense;
	int m_drive;
	UINT8 m_data;

	static bool first_time;                 // for power switch sound
	emu_timer *m_scan_timer;                // scan timer
};


// device type definition
extern const device_type MM1_KEYBOARD;



#endif
