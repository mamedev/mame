// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    v3021.h

    EM Microelectronic-Marin SA Ultra Low Power 32kHz CMOS RTC (DIP8)

    Serial Real Time Clock

***************************************************************************/

#pragma once

#ifndef __v3021DEV_H__
#define __v3021DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_V3021_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, v3021, XTAL_32_768kHz)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct rtc_regs_t
{
	uint8_t sec, min, hour, day, wday, month, year;
};


// ======================> v3021_device

class v3021_device :    public device_t
{
public:
	// construction/destruction
	v3021_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void timer_callback(void *ptr, int32_t param);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	uint8_t m_cal_mask,m_cal_com,m_cal_cnt,m_cal_val;

	rtc_regs_t m_rtc;
};


// device type definition
extern const device_type v3021;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
