/**********************************************************************

    Luxor ABC-55/77 keyboard emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __ABC77__
#define __ABC77__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/discrete.h"
#include "sound/speaker.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define ABC77_TAG	"abc77"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC55_ADD(_config) \
    MCFG_DEVICE_ADD(ABC77_TAG, ABC55, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define MCFG_ABC77_ADD(_config) \
    MCFG_DEVICE_ADD(ABC77_TAG, ABC77, 0) \
	MCFG_DEVICE_CONFIG(_config)


#define ABC77_INTERFACE(_name) \
	const abc77_interface (_name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc77_interface

struct abc77_interface
{
	devcb_write_line	m_out_clock_cb;
	devcb_write_line	m_out_keydown_cb;
};


// ======================> abc77_device

class abc77_device :  public device_t,
                      public abc77_interface
{
public:
    // construction/destruction
    abc77_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
    abc77_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_INPUT_CHANGED_MEMBER( keyboard_reset );

	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ8_MEMBER( t1_r );
	DECLARE_WRITE8_MEMBER( prog_w );
	DECLARE_WRITE8_MEMBER( j3_w );

	DECLARE_WRITE_LINE_MEMBER( rxd_w );
	DECLARE_READ_LINE_MEMBER( txd_r );
	DECLARE_WRITE_LINE_MEMBER( reset_w );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

	static const device_timer_id TIMER_SERIAL = 0;
	static const device_timer_id TIMER_RESET = 1;

	inline void serial_output(int state);
	inline void serial_clock();
	inline void key_down(int state);

	devcb_resolved_write_line	m_out_clock_func;
	devcb_resolved_write_line	m_out_keydown_func;

	required_device<cpu_device> m_maincpu;
	required_device<discrete_sound_device> m_discrete;

	int m_txd;						// transmit data
	int m_keylatch;					// keyboard row latch
	int m_keydown;					// key down
	int m_clock;					// transmit clock
	int m_hys;						// hysteresis
	int m_reset;					// reset
	int m_stb;						// strobe
	UINT8 m_j3;

	// timers
	emu_timer *m_serial_timer;
	emu_timer *m_reset_timer;
};


class abc55_device :  public abc77_device
{
public:
    // construction/destruction
    abc55_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const;
};


// device type definition
extern const device_type ABC77;
extern const device_type ABC55;



#endif
