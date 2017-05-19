// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_SNES_ROM21_H
#define MAME_BUS_SNES_ROM21_H

#pragma once

#include "snes_slot.h"


// ======================> sns_rom21_device

class sns_rom21_device : public device_t,
						public device_sns_cart_interface
{
public:
	// construction/destruction
	sns_rom21_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_l) override;
	virtual DECLARE_READ8_MEMBER(read_h) override;

protected:
	sns_rom21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
};

// ======================> sns_rom21_srtc_device

class sns_rom21_srtc_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom21_srtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(chip_read) override;
	virtual DECLARE_WRITE8_MEMBER(chip_write) override;

protected:
	// S-RTC specific variables
	enum
	{
		RTCM_Ready = 0,
		RTCM_Command,
		RTCM_Read,
		RTCM_Write
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	void update_time();
	uint8_t srtc_weekday(uint32_t year, uint32_t month, uint32_t day);

	//this is now allocated in the main snes cart class, to allow saving to nvram
	//uint8_t  m_rtc_ram[13];
	int32_t  m_mode;
	int8_t   m_index;
};


// device type definition
DECLARE_DEVICE_TYPE(SNS_HIROM,      sns_rom21_device)
DECLARE_DEVICE_TYPE(SNS_HIROM_SRTC, sns_rom21_srtc_device)

#endif // MAME_BUS_SNES_ROM21_H
