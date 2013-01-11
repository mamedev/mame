/**********************************************************************

    Luxor ABC-99 keyboard and mouse emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __ABC99__
#define __ABC99__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC99_TAG   "abc99"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC99_ADD(_config) \
	MCFG_DEVICE_ADD(ABC99_TAG, ABC99, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define ABC99_INTERFACE(_name) \
	const abc99_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc99_interface

struct abc99_interface
{
	devcb_write_line    m_out_clock_cb;
	devcb_write_line    m_out_keydown_cb;
};

// ======================> abc99_device

class abc99_device :  public device_t,
						public abc99_interface
{
public:
	// construction/destruction
	abc99_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER( keyboard_reset );

	DECLARE_WRITE8_MEMBER( z2_led_w );
	DECLARE_WRITE8_MEMBER( z2_p1_w );
	DECLARE_READ8_MEMBER( z2_p2_r );
	DECLARE_READ8_MEMBER( z2_t0_r );
	DECLARE_READ8_MEMBER( z2_t1_r );
	DECLARE_READ8_MEMBER( z5_p1_r );
	DECLARE_WRITE8_MEMBER( z5_p2_w );
	DECLARE_WRITE8_MEMBER( z5_t0_w );
	DECLARE_READ8_MEMBER( z5_t1_r );

	DECLARE_WRITE_LINE_MEMBER( rxd_w );
	DECLARE_READ_LINE_MEMBER( txd_r );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

private:
	static const device_timer_id TIMER_SERIAL = 0;
	static const device_timer_id TIMER_MOUSE = 1;

	inline void serial_input();
	inline void serial_output();
	inline void serial_clock();
	inline void key_down(int state);
	inline void scan_mouse();

	devcb_resolved_write_line   m_out_clock_func;
	devcb_resolved_write_line   m_out_keydown_func;

	emu_timer *m_serial_timer;
	emu_timer *m_mouse_timer;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mousecpu;
	required_device<speaker_sound_device> m_speaker;

	int m_si;
	int m_si_en;
	int m_so;
	int m_so_z2;
	int m_so_z5;
	int m_keydown;
	int m_t1_z2;
	int m_t1_z5;
	int m_led_en;
	int m_reset;
};


// device type definition
extern const device_type ABC99;



#endif
