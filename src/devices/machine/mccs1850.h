// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Motorola MCCS1850 Serial Real-Time Clock emulation

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

#define MCFG_MCCS1850_INT_CALLBACK(_write) \
	devcb = &mccs1850_device::set_int_wr_callback(*device, DEVCB_##_write);

#define MCFG_MCCS1850_PSE_CALLBACK(_write) \
	devcb = &mccs1850_device::set_pse_wr_callback(*device, DEVCB_##_write);

#define MCFG_MCCS1850_NUC_CALLBACK(_write) \
	devcb = &mccs1850_device::set_nuc_wr_callback(*device, DEVCB_##_write);



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
	mccs1850_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_int_wr_callback(device_t &device, _Object object) { return downcast<mccs1850_device &>(device).int_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_pse_wr_callback(device_t &device, _Object object) { return downcast<mccs1850_device &>(device).pse_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_nuc_wr_callback(device_t &device, _Object object) { return downcast<mccs1850_device &>(device).nuc_cb.set_callback(object); }

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( sck_w );
	DECLARE_READ_LINE_MEMBER( sdo_r );
	DECLARE_WRITE_LINE_MEMBER( sdi_w );
	DECLARE_WRITE_LINE_MEMBER( pwrsw_w );
	DECLARE_WRITE_LINE_MEMBER( por_w );
	DECLARE_WRITE_LINE_MEMBER( test_w );

	// For setting the time at startup
	void set_counter(UINT32 value);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	inline void check_interrupt();
	inline void set_pse_line(bool state);
	inline UINT8 read_register(offs_t offset);
	inline void write_register(offs_t offset, UINT8 data);
	inline void advance_seconds();

	static const device_timer_id TIMER_CLOCK = 0;

	devcb_write_line int_cb, pse_cb, nuc_cb;

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
