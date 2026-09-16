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
	ws_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// reading and writing
	virtual u16 read_rom20(offs_t offset, u16 mem_mask) override;
	virtual u16 read_rom30(offs_t offset, u16 mem_mask) override;
	virtual u16 read_rom40(offs_t offset, u16 mem_mask) override;
	virtual u16 read_io(offs_t offset, u16 mem_mask) override;
	virtual void write_io(offs_t offset, u16 data, u16 mem_mask) override;

protected:
	ws_rom_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(rtc_tick);

	u16 m_io_regs[8];
	u32 m_base20, m_base30, m_base40;
	u32 m_rom_mask;

	// RTC
	u8   m_rtc_setting;    /* Timer setting byte */
	u8   m_rtc_year;       /* Year */
	u8   m_rtc_month;      /* Month */
	u8   m_rtc_day;        /* Day */
	u8   m_rtc_day_of_week;    /* Day of the week */
	u8   m_rtc_hour;       /* Hour, high bit = 0 => AM, high bit = 1 => PM */
	u8   m_rtc_minute;     /* Minute */
	u8   m_rtc_second;     /* Second */
	u8   m_rtc_index;      /* index for reading/writing of current of alarm time */

	emu_timer *rtc_timer;
};


// ======================> ws_rom_sram_device

class ws_rom_sram_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_sram_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// reading and writing
	virtual u16 read_ram(offs_t offset, u16 mem_mask) override;
	virtual void write_ram(offs_t offset, u16 data, u16 mem_mask) override;
	virtual void write_io(offs_t offset, u16 data, u16 mem_mask) override;

protected:
	ws_rom_sram_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u32 m_nvram_base;
};


// ======================> ws_rom_eeprom_device

class ws_rom_eeprom_device : public ws_rom_device
{
public:
	// construction/destruction
	ws_rom_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// reading and writing
	virtual u16 read_io(offs_t offset, u16 mem_mask) override;
	virtual void write_io(offs_t offset, u16 data, u16 mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8   m_eeprom_mode;       /* eeprom mode */
	u16  m_eeprom_address;    /* Read/write address */
	u8   m_eeprom_command;    /* Commands: 00, 01, 02, 03, 04, 08, 0C */
	u8   m_eeprom_start;      /* start bit */
	u8   m_eeprom_write_enabled;  /* write enabled yes/no */
};


class ws_wwitch_device : public ws_rom_sram_device
{
public:
	// construction/destruction
	ws_wwitch_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// reading and writing
	virtual u16 read_ram(offs_t offset, u16 mem_mask) override;
	virtual void write_ram(offs_t offset, u16 data, u16 mem_mask) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum {
		READ_MODE = 0,
		COMMAND_MODE
	};
	u8 m_flash_seq;
	u8 m_flash_command;
	bool m_write_flash;
	bool m_writing_flash;
	bool m_write_resetting;
	u8 m_flash_mode;
	u8 m_flash_status;
	u8 m_flash_count;
};


// device type definition
DECLARE_DEVICE_TYPE(WS_ROM_STD,    ws_rom_device)
DECLARE_DEVICE_TYPE(WS_ROM_SRAM,   ws_rom_sram_device)
DECLARE_DEVICE_TYPE(WS_ROM_EEPROM, ws_rom_eeprom_device)
DECLARE_DEVICE_TYPE(WS_ROM_WWITCH, ws_wwitch_device)

#endif // MAME_BUS_WSWAN_ROM_H
