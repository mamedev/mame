/**********************************************************************

    VideoBrain UV201/UV202 video chip emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 40
                     G   2 |             | 39
                     I   3 |             | 38  RESET
                     B   4 |             | 37  BRC
                     R   5 |             | 36
                   BA0   6 |             | 35  B1 STROBE
                   BA1   7 |             | 34  COLOR XTAL (3.636 MHz)
                   BA2   8 |             | 33
                   BA3   9 |             | 32
                   BA4  10 |    UV201    | 31
                   BA5  11 |             | 30  EXTINT
                   BA6  12 |             | 29  O/E
                   BA7  13 |             | 28  BO7
                   BA8  14 |             | 27  BO6
                   BA9  15 |             | 26  BO5
                  BA10  16 |             | 25  BO4
                  BA11  17 |             | 24  BO3
                  BA12  18 |             | 23  BO2
                 HSYNC  19 |             | 22  BO1
                 HSYNC  20 |_____________| 21  BO0

                            _____   _____
                         1 |*    \_/     | 40  B1 STROBE
                         2 |             | 39  CPU RQ1
               CPU RQ0   3 |             | 38  GND
               XTAL IN   4 |             | 37
               XTAL IN   5 |             | 36  RESET
                         6 |             | 35
               CPU phi   7 |             | 34  COLOR XTAL
                         8 |             | 33  BRC
                   BD0   9 |             | 32  BD7
                   BO0  10 |    UV202    | 31  BO7
                   BD1  11 |             | 30  BD6
                   BO1  12 |             | 29  BO6
                   BD2  13 |             | 28  BD5
                   BO2  14 |             | 27  BO5
                   BD3  15 |             | 26  BD4
                   BO3  16 |             | 25  BO4
                HBLANK  17 |             | 24
                        18 |             | 23
                        19 |             | 22
                        20 |_____________| 21

**********************************************************************/

#pragma once

#ifndef __UV201__
#define __UV201__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_UV201_ADD(_tag, _screen_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, UV201, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER) \
	MCFG_SCREEN_UPDATE_DEVICE(_tag, uv201_device, screen_update) \
	MCFG_SCREEN_RAW_PARAMS(_clock, 232, 18, 232, 262, 21, 262)


#define UV201_INTERFACE(name) \
	const uv201_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> uv201_interface

struct uv201_interface
{
	const char *m_screen_tag;

	devcb_write_line        m_out_ext_int_cb;
	devcb_write_line        m_out_hblank_cb;
	devcb_read8             m_in_db_cb;
};


// ======================> uv201_device

class uv201_device :    public device_t,
						public uv201_interface
{
public:
	// construction/destruction
	uv201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( ext_int_w );
	DECLARE_READ_LINE_MEMBER( kbd_r );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum
	{
		TIMER_Y_ODD,
		TIMER_Y_EVEN,
		TIMER_HBLANK_ON,
		TIMER_HBLANK_OFF
	};

	void initialize_palette();
	int get_field_vpos();
	int get_field();
	void set_y_interrupt();
	void do_partial_update();

	devcb_resolved_write_line   m_out_ext_int_func;
	devcb_resolved_write_line   m_out_hblank_func;
	devcb_resolved_read8        m_in_db_func;

	screen_device *m_screen;

	rgb_t m_palette[32];
	UINT8 m_ram[0x90];
	UINT8 m_y_int;
	UINT8 m_fmod;
	UINT8 m_bg;
	UINT8 m_cmd;
	UINT8 m_freeze_x;
	UINT16 m_freeze_y;
	int m_field;

	// timers
	emu_timer *m_timer_y_odd;
	emu_timer *m_timer_y_even;
	emu_timer *m_timer_hblank_on;
	emu_timer *m_timer_hblank_off;
};


// device type definition
extern const device_type UV201;



#endif
