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
	virtual uint8_t read_l(offs_t offset) override;
	virtual uint8_t read_h(offs_t offset) override;

protected:
	sns_rom21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

// ======================> sns_rom21_srtc_device

class sns_rom21_srtc_device : public sns_rom21_device
{
public:
	// construction/destruction
	sns_rom21_srtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t chip_read(offs_t offset) override;
	virtual void chip_write(offs_t offset, uint8_t data) override;

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
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

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
