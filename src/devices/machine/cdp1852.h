// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA CDP1852 Byte-Wide Input/Output Port emulation

**********************************************************************
                            _____   _____
              CSI/_CSI   1 |*    \_/     | 24  Vdd
                  MODE   2 |             | 23  _SR/SR
                   DI0   3 |             | 22  DI7
                   DO0   4 |             | 21  DO7
                   DI1   5 |             | 20  DI6
                   DO1   6 |   CDP1852   | 19  DO6
                   DI2   7 |             | 18  DI5
                   DO2   8 |             | 17  DO5
                   DI3   9 |             | 16  DI4
                   DO3  10 |             | 15  DO4
                 CLOCK  11 |             | 14  _CLEAR
                   Vss  12 |_____________| 13  CS2

**********************************************************************/

#pragma once

#ifndef __CDP1852__
#define __CDP1852__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CDP1852_MODE_CALLBACK(_read) \
	devcb = &cdp1852_device::set_mode_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1852_SR_CALLBACK(_write) \
	devcb = &cdp1852_device::set_sr_wr_callback(*device, DEVCB_##_write);

#define MCFG_CDP1852_DI_CALLBACK(_read) \
	devcb = &cdp1852_device::set_data_rd_callback(*device, DEVCB_##_read);

#define MCFG_CDP1852_DO_CALLBACK(_write) \
	devcb = &cdp1852_device::set_data_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cdp1852_device

class cdp1852_device :  public device_t
{
public:
	// construction/destruction
	cdp1852_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_mode_rd_callback(device_t &device, _Object object) { return downcast<cdp1852_device &>(device).m_read_mode.set_callback(object); }
	template<class _Object> static devcb_base &set_sr_wr_callback(device_t &device, _Object object) { return downcast<cdp1852_device &>(device).m_write_sr.set_callback(object); }
	template<class _Object> static devcb_base &set_data_rd_callback(device_t &device, _Object object) { return downcast<cdp1852_device &>(device).m_read_data.set_callback(object); }
	template<class _Object> static devcb_base &set_data_wr_callback(device_t &device, _Object object) { return downcast<cdp1852_device &>(device).m_write_data.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void set_sr_line(int state);

	devcb_read_line    m_read_mode;
	devcb_write_line   m_write_sr;
	devcb_read8        m_read_data;
	devcb_write8       m_write_data;

	int m_new_data;             // new data written
	UINT8 m_data;               // data latch
	UINT8 m_next_data;          // next data

	int m_sr;                   // service request flag
	int m_next_sr;              // next value of service request flag

	// timers
	emu_timer *m_scan_timer;
};


// device type definition
extern const device_type CDP1852;



#endif
