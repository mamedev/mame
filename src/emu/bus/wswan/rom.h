// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef __WS_ROM_H
#define __WS_ROM_H

#include "slot.h"


// ======================> ws_rom_device

class ws_rom_device : public device_t,
						public device_ws_cart_interface
{
public:
	// construction/destruction
	ws_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	ws_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom20);
	virtual DECLARE_READ8_MEMBER(read_rom30);
	virtual DECLARE_READ8_MEMBER(read_rom40);
	virtual DECLARE_READ8_MEMBER(read_io);
	virtual DECLARE_WRITE8_MEMBER(write_io);

protected:
	UINT8 m_io_regs[0x10];
	UINT32 m_base20, m_base30, m_base40;

	// RTC
	UINT8   m_rtc_setting;    /* Timer setting byte */
	UINT8   m_rtc_year;       /* Year */
	UINT8   m_rtc_month;      /* Month */
	UINT8   m_rtc_day;        /* Day */
	UINT8   m_rtc_day_of_week;    /* Day of the week */
	UINT8   m_rtc_hour;       /* Hour, high bit = 0 => AM, high bit = 1 => PM */
	UINT8   m_rtc_minute;     /* Minute */
	UINT8   m_rtc_second;     /* Second */
	UINT8   m_rtc_index;      /* index for reading/writing of current of alarm time */

	static const device_timer_id TIMER_RTC = 0;
	emu_timer *rtc_timer;
};


// ======================> ws_rom_sram_device

class ws_rom_sram_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_ram);
	virtual DECLARE_WRITE8_MEMBER(write_ram);
	virtual DECLARE_WRITE8_MEMBER(write_io);

private:
	UINT32 m_nvram_base;
};


// ======================> ws_rom_eeprom_device

class ws_rom_eeprom_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_io);
	virtual DECLARE_WRITE8_MEMBER(write_io);

private:
	UINT8   m_eeprom_mode;       /* eeprom mode */
	UINT16  m_eeprom_address;    /* Read/write address */
	UINT8   m_eeprom_command;    /* Commands: 00, 01, 02, 03, 04, 08, 0C */
	UINT8   m_eeprom_start;      /* start bit */
	UINT8   m_eeprom_write_enabled;  /* write enabled yes/no */
};



// device type definition
extern const device_type WS_ROM_STD;
extern const device_type WS_ROM_SRAM;
extern const device_type WS_ROM_EEPROM;


#endif
