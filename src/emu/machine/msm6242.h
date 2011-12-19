/***************************************************************************

    MSM6242 Real Time Clock

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MSM6242DEV_H__
#define __MSM6242DEV_H__


#define MCFG_MSM6242_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, msm6242, XTAL_32_768kHz)

typedef struct
{
	UINT8 sec, min, hour, day, wday, month, year;
} rtc_regs_t;


// ======================> msm6242_device

class msm6242_device :	public device_t
{
public:
	// construction/destruction
	msm6242_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	void timer_callback();

protected:
	// device-level overrides
	virtual bool device_validity_check(emu_options &options, const game_driver &driver) const;
	virtual void device_start();
	virtual void device_reset();

	static TIMER_CALLBACK( rtc_inc_callback );

	UINT8 m_reg[3];

	rtc_regs_t m_rtc;
	rtc_regs_t m_hold;
};


// device type definition
extern const device_type msm6242;


#endif /* __MSM6242DEV_H__ */
