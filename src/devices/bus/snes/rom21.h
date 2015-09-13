// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SNS_ROM21_H
#define __SNS_ROM21_H

#include "snes_slot.h"


// ======================> sns_rom21_device

class sns_rom21_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_rom21_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	sns_rom21_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l);
	virtual DECLARE_READ8_MEMBER(read_h);
};

// ======================> sns_rom21_srtc_device

class sns_rom21_srtc_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom21_srtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read);
	virtual DECLARE_WRITE8_MEMBER(chip_write);

	// S-RTC specific variables
	enum
	{
		RTCM_Ready = 0,
		RTCM_Command,
		RTCM_Read,
		RTCM_Write
	};

	void update_time();
	UINT8 srtc_weekday(UINT32 year, UINT32 month, UINT32 day);

	//this is now allocated in the main snes cart class, to allow saving to nvram
	//UINT8  m_rtc_ram[13];
	INT32  m_mode;
	INT8   m_index;
};


// device type definition
extern const device_type SNS_HIROM;
extern const device_type SNS_HIROM_SRTC;

#endif
