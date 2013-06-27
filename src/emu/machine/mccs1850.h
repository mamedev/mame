/**********************************************************************

    Motorola MCCS1850 Serial Real-Time Clock emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                  Vbat   1 |*    \_/     | 16  Vdd
                  _POR   2 |             | 15  TEST
                  _INT   3 |             | 14  XTAL1
                   SCK   4 |   MCCS1850  | 13  XTAL2
                   SDI   5 |             | 12  _PWRSW
                   SDO   6 |             | 11  NUC
                    CE   7 |             | 10  _PSE
                   Vss   8 |_____________| 9   PSE

**********************************************************************/

#pragma once

#ifndef __MCCS1850__
#define __MCCS1850__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MCCS1850_ADD(_tag, _clock, _int_cb, _pse_cb, _nuc_cb)  \
	MCFG_DEVICE_ADD(_tag, MCCS1850, _clock) \
	downcast<mccs1850_device *>(device)->set_cb(_int_cb, _pse_cb, _nuc_cb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mccs1850_device

class mccs1850_device : public device_t,
						public device_rtc_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	mccs1850_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_cb(line_cb_t int_cb, line_cb_t pse_cb, line_cb_t nuc_cb);

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( sck_w );
	DECLARE_READ_LINE_MEMBER( sdo_r );
	DECLARE_WRITE_LINE_MEMBER( sdi_w );
	DECLARE_WRITE_LINE_MEMBER( pwrsw_w );
	DECLARE_WRITE_LINE_MEMBER( por_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_nvram_interface overrides
	virtual void nvram_default();
	virtual void nvram_read(emu_file &file);
	virtual void nvram_write(emu_file &file);

private:
	inline void check_interrupt();
	inline void set_pse_line(bool state);
	inline UINT8 read_register(offs_t offset);
	inline void write_register(offs_t offset, UINT8 data);
	inline void advance_seconds();

	static const device_timer_id TIMER_CLOCK = 0;

	line_cb_t int_cb, pse_cb, nuc_cb;

	UINT8 m_ram[0x80];          // RAM

	// power supply
	int m_pse;                  // power supply enable

	// counter
	UINT32 m_counter;           // seconds counter

	// serial interface
	int m_ce;                   // chip enable
	int m_sck;                  // serial clock
	int m_sdo;                  // serial data out
	int m_sdi;                  // serial data in
	int m_state;                // serial interface state
	UINT8 m_address;            // address counter
	int m_bits;                 // bit counter
	UINT8 m_shift;              // shift register

	// timers
	emu_timer *m_clock_timer;
};


// device type definition
extern const device_type MCCS1850;



#endif
