// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/***************************************************************************

    rtc9701.h

    Serial rtc9701s.

***************************************************************************/

#pragma once

#ifndef __rtc9701DEV_H__
#define __rtc9701DEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RTC9701_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, rtc9701, XTAL_32_768kHz)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


enum rtc9701_state_t
{
	RTC9701_CMD_WAIT = 0,
	RTC9701_RTC_READ,
	RTC9701_RTC_WRITE,
	RTC9701_EEPROM_READ,
	RTC9701_EEPROM_WRITE,
	RTC9701_AFTER_WRITE_ENABLE

};

struct rtc_regs_t
{
	uint8_t sec, min, hour, day, wday, month, year;
};


// ======================> rtc9701_device

class rtc9701_device :  public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	rtc9701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);


	// I/O operations
	void write_bit(int state);
	int read_bit();
	void set_cs_line(int state);
	void set_clock_line(int state);
	void timer_callback(void *ptr, int32_t param);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;
	inline uint8_t rtc_read(uint8_t offset);
	inline void rtc_write(uint8_t offset,uint8_t data);

	int                     m_latch;
	int                     m_reset_line;
	int                     m_clock_line;


	rtc9701_state_t rtc_state;
	int cmd_stream_pos;
	int current_cmd;

	int rtc9701_address_pos;
	int rtc9701_current_address;

	uint16_t rtc9701_current_data;
	int rtc9701_data_pos;

	uint16_t rtc9701_data[0x100];

	rtc_regs_t m_rtc;
};


// device type definition
extern const device_type rtc9701;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
