// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC-55/77 keyboard emulation

*********************************************************************/

#pragma once

#ifndef __ABC77__
#define __ABC77__

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "abckb.h"
#include "sound/discrete.h"
#include "sound/speaker.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc77_device

class abc77_device :  public device_t,
						public abc_keyboard_interface
{
public:
	// construction/destruction
	abc77_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	abc77_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// abc_keyboard_interface overrides
	virtual void txd_w(int state) override;

	DECLARE_INPUT_CHANGED_MEMBER( keyboard_reset );

	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( t1_r );
	DECLARE_WRITE8_MEMBER( prog_w );
	DECLARE_WRITE8_MEMBER( j3_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	enum
	{
		TIMER_SERIAL,
		TIMER_RESET
	};

	inline void serial_output(int state);
	inline void serial_clock();
	inline void key_down(int state);

	required_device<cpu_device> m_maincpu;
	required_device<discrete_sound_device> m_discrete;
	required_ioport m_x0;
	required_ioport m_x1;
	required_ioport m_x2;
	required_ioport m_x3;
	required_ioport m_x4;
	required_ioport m_x5;
	required_ioport m_x6;
	required_ioport m_x7;
	required_ioport m_x8;
	required_ioport m_x9;
	required_ioport m_x10;
	required_ioport m_x11;
	required_ioport m_dsw;

	int m_txd;                      // transmit data
	int m_keylatch;                 // keyboard row latch
	int m_keydown;                  // key down
	int m_clock;                    // transmit clock
	int m_hys;                      // hysteresis
	int m_reset;                    // reset
	int m_stb;                      // strobe
	UINT8 m_j3;

	// timers
	emu_timer *m_serial_timer;
	emu_timer *m_reset_timer;
};


class abc55_device :  public abc77_device
{
public:
	// construction/destruction
	abc55_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;
};


// device type definition
extern const device_type ABC77;
extern const device_type ABC55;



#endif
