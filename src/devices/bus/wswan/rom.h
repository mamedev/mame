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
	ws_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	ws_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// reading and writing
	virtual uint8_t read_rom20(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_rom30(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_rom40(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual uint8_t read_io(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_io(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

protected:
	uint8_t m_io_regs[0x10];
	uint32_t m_base20, m_base30, m_base40;

	// RTC
	uint8_t   m_rtc_setting;    /* Timer setting byte */
	uint8_t   m_rtc_year;       /* Year */
	uint8_t   m_rtc_month;      /* Month */
	uint8_t   m_rtc_day;        /* Day */
	uint8_t   m_rtc_day_of_week;    /* Day of the week */
	uint8_t   m_rtc_hour;       /* Hour, high bit = 0 => AM, high bit = 1 => PM */
	uint8_t   m_rtc_minute;     /* Minute */
	uint8_t   m_rtc_second;     /* Second */
	uint8_t   m_rtc_index;      /* index for reading/writing of current of alarm time */

	static const device_timer_id TIMER_RTC = 0;
	emu_timer *rtc_timer;
};


// ======================> ws_rom_sram_device

class ws_rom_sram_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;
	virtual void write_io(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint32_t m_nvram_base;
};


// ======================> ws_rom_eeprom_device

class ws_rom_eeprom_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual uint8_t read_io(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void write_io(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint8_t   m_eeprom_mode;       /* eeprom mode */
	uint16_t  m_eeprom_address;    /* Read/write address */
	uint8_t   m_eeprom_command;    /* Commands: 00, 01, 02, 03, 04, 08, 0C */
	uint8_t   m_eeprom_start;      /* start bit */
	uint8_t   m_eeprom_write_enabled;  /* write enabled yes/no */
};



// device type definition
extern const device_type WS_ROM_STD;
extern const device_type WS_ROM_SRAM;
extern const device_type WS_ROM_EEPROM;


#endif
