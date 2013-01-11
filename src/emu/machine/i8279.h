/***************************************************************************

    Intel 8279 Programmable Keyboard/Display Interface emulation

    Copyright the MESS Team 2012.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************
                            _____   _____
                   RL2   1 |*    \_/     | 40  Vcc
                   RL3   2 |             | 39  RL1
                   CLK   3 |             | 38  RL0
                   IRQ   4 |             | 37  CNTL/STB
                   RL4   5 |             | 36  SHIFT
                   RL5   6 |             | 35  SL3
                   RL6   7 |             | 34  SL2
                   RL7   8 |             | 33  SL1
                 RESET   9 |             | 32  SL0
                   /RD  10 |     8279    | 31  B0
                   /WR  11 |             | 30  B1
                   DB0  12 |             | 29  B2
                   DB1  13 |             | 28  B3
                   DB2  14 |             | 27  A0
                   DB3  15 |             | 26  A1
                   DB4  16 |             | 25  A2
                   DB5  17 |             | 24  A3
                   DB6  18 |             | 23  /BD
                   DB7  19 |             | 22  /CS
                   Vss  20 |_____________| 21  CTRL/DATA


***************************************************************************/

#pragma once

#ifndef __I8279__
#define __I8279__

#include "emu.h"



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8279_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, I8279, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define I8279_INTERFACE(_name) \
	const i8279_interface (_name) =


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> i8279_interface

struct i8279_interface
{
	devcb_write_line    m_out_irq_cb;       // IRQ
	devcb_write8        m_out_sl_cb;        // Scanlines SL0-3
	devcb_write8        m_out_disp_cb;      // B0-3,A0-3
	devcb_write_line    m_out_bd_cb;        // BD
	devcb_read8     m_in_rl_cb;     // kbd readlines RL0-7
	devcb_read_line     m_in_shift_cb;      // Shift key
	devcb_read_line     m_in_ctrl_cb;       // Ctrl-Strobe line
};



// ======================> i8279_device

class i8279_device :  public device_t, public i8279_interface
{
public:
	// construction/destruction
	i8279_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// read & write handlers
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(data_r);
	DECLARE_WRITE8_MEMBER(cmd_w);
	DECLARE_WRITE8_MEMBER(data_w);
	void timer_mainloop();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_post_load() { }
	virtual void device_clock_changed() { }

	static TIMER_CALLBACK( timerproc_callback );

private:

	void timer_adjust();
	void clear_display();
	void new_key(UINT8 data, bool skey, bool ckey);
	void new_fifo(UINT8 data);
	UINT8 get_segments();
	void set_irq(bool state);
	void set_display_mode(UINT8 data);

	devcb_resolved_write_line   m_out_irq_func;
	devcb_resolved_write8       m_out_sl_func;
	devcb_resolved_write8       m_out_disp_func;
	devcb_resolved_write_line   m_out_bd_func;
	devcb_resolved_read8        m_in_rl_func;
	devcb_resolved_read_line    m_in_shift_func;
	devcb_resolved_read_line    m_in_ctrl_func;

	emu_timer *m_timer;

	UINT8 m_d_ram[16];      // display ram
	UINT8 m_d_ram_ptr;
	UINT8 m_s_ram[8]; // might be same as fifo ram
	UINT8 m_s_ram_ptr;
	UINT8 m_fifo[8];    // queued keystrokes
	UINT8 m_cmd[8];   // Device settings
	UINT8 m_status;     // Returned via status_r
	UINT32 m_clock;     // Internal scan clock
	UINT8 m_scanner;    // next output on SL lines

	bool m_autoinc;     // auto-increment flag
	bool m_read_flag;   // read from where
	bool m_ctrl_key;    // previous state of strobe input
	UINT16 m_key_down;
};


// device type definition
extern const device_type I8279;



#endif
