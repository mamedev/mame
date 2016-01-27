// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __AICARTCDEV_H__
#define __AICARTCDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AICARTC_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, AICARTC, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aicartc_device

class aicartc_device : public device_t,
						public device_rtc_interface
{
public:
	// construction/destruction
	aicartc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_READ16_MEMBER( read );

	UINT16 m_rtc_reg_lo,m_rtc_reg_hi;
	UINT16 m_rtc_tick;
	UINT8 m_we;

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	emu_timer *m_clock_timer;
};


// device type definition
extern const device_type AICARTC;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
