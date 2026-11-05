// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
#ifndef MAME_MACHINE_EEPROMSPI_H
#define MAME_MACHINE_EEPROMSPI_H

#pragma once

#include "eeprom.h"

#include <memory>


class eeprom_spi_base_device : public eeprom_base_device
{
public:
	void cs_w(int state);
	void sck_w(int state);
	void si_w(int state) { m_si = bool(state); }
	int so_r() const { return (m_cs || !m_hold) ? 1 : m_so; }
	void wp_w(int state) { m_wp = bool(state); }
	void hold_w(int state);

protected:
	eeprom_spi_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 page_bytes, u8 address_bytes);

	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_nvram_interface implementation
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	enum : u8
	{
		COMMAND_WRSR = 0x01,
		COMMAND_WRITE = 0x02,
		COMMAND_READ = 0x03,
		COMMAND_WRDI = 0x04,
		COMMAND_RDSR = 0x05,
		COMMAND_WREN = 0x06
	};

	enum : u8
	{
		STATE_COMMAND,
		STATE_ADDRESS,
		STATE_READ,
		STATE_WRITE,
		STATE_READ_STATUS,
		STATE_WRITE_STATUS,
		STATE_DONE,
		STATE_IGNORE
	};

	static constexpr u8 STATUS_NONVOLATILE = 0x8c;

	bool valid_geometry() const ATTR_COLD;
	void receive_byte(u8 data);
	void finish_command();
	void start_write(u8 command);
	u8 status_r() const;
	bool write_protected() const;
	TIMER_CALLBACK_MEMBER(write_complete);

	u32 const m_page_bytes;
	u8 const m_address_bytes;
	std::unique_ptr<u8[]> m_page;
	emu_timer *m_write_timer;
	bool m_cs;
	bool m_sck;
	bool m_si;
	bool m_so;
	bool m_wp;
	bool m_hold;
	bool m_held;
	bool m_write_enable;
	u8 m_state;
	u8 m_command;
	u8 m_shift;
	u8 m_bits;
	u8 m_address_count;
	u32 m_address;
	u8 m_status;
	u8 m_pending_status;
	u8 m_pending_command;
	u32 m_page_base;
	u32 m_page_start;
	u32 m_page_count;
};


class eeprom_spi_25320_device : public eeprom_spi_base_device
{
public:
	eeprom_spi_25320_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
};

DECLARE_DEVICE_TYPE(EEPROM_25320, eeprom_spi_25320_device)

#endif // MAME_MACHINE_EEPROMSPI_H
