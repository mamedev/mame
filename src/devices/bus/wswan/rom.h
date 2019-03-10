// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_BUS_WSWAN_ROM_H
#define MAME_BUS_WSWAN_ROM_H

#include "slot.h"


// ======================> ws_rom_device

class ws_rom_device : public device_t,
						public device_ws_cart_interface
{
public:
	// construction/destruction
	ws_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_rom20(offs_t offset) override;
	virtual uint8_t read_rom30(offs_t offset) override;
	virtual uint8_t read_rom40(offs_t offset) override;
	virtual uint8_t read_io(offs_t offset) override;
	virtual void write_io(offs_t offset, uint8_t data) override;

protected:
	static constexpr device_timer_id TIMER_RTC = 0;

	ws_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

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

	emu_timer *rtc_timer;
};


// ======================> ws_rom_sram_device

class ws_rom_sram_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
	virtual void write_io(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint32_t m_nvram_base;
};


// ======================> ws_rom_eeprom_device

class ws_rom_eeprom_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual uint8_t read_io(offs_t offset) override;
	virtual void write_io(offs_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t   m_eeprom_mode;       /* eeprom mode */
	uint16_t  m_eeprom_address;    /* Read/write address */
	uint8_t   m_eeprom_command;    /* Commands: 00, 01, 02, 03, 04, 08, 0C */
	uint8_t   m_eeprom_start;      /* start bit */
	uint8_t   m_eeprom_write_enabled;  /* write enabled yes/no */
};



// device type definition
DECLARE_DEVICE_TYPE(WS_ROM_STD,    ws_rom_device)
DECLARE_DEVICE_TYPE(WS_ROM_SRAM,   ws_rom_sram_device)
DECLARE_DEVICE_TYPE(WS_ROM_EEPROM, ws_rom_eeprom_device)

#endif // MAME_BUS_WSWAN_ROM_H
