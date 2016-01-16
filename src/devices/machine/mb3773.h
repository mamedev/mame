// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Fujistu MB3773

    Power Supply Monitor with Watch Dog Timer (i.e. Reset IC)

***************************************************************************/

#pragma once

#ifndef __MB3773_H__
#define __MB3773_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MB3773_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MB3773, 0)


// ======================> mb3773_device

class mb3773_device :
	public device_t
{
public:
	// construction/destruction
	mb3773_device( const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock );

	// I/O operations
	WRITE_LINE_MEMBER( write_line_ck );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	void reset_timer();

	// internal state
	emu_timer *m_watchdog_timer;
	int m_ck;
};


// device type definition
extern const device_type MB3773;

#endif
