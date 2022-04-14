// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    ATMEL AT28C64B

    64K ( 8K x 8 ) Parallel EEPROM

***************************************************************************/

#ifndef MAME_MACHINE_AT28C64B_H
#define MAME_MACHINE_AT28C64B_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> at28c64b_device

class at28c64b_device :
	public device_t,
	public device_memory_interface,
	public device_nvram_interface
{
public:
	// construction/destruction
	at28c64b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, u8 data);
	u8 read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	// internal state
	enum
	{
		STATE_IDLE,
		STATE_ID_1,
		STATE_ID_2,
		STATE_SECTOR_WRITE,
		STATE_WRITE_PROTECT
	};

	address_space_config m_space_config;
	emu_timer *m_write_timer;
	int m_a9_12v;
	int m_oe_12v;
	int m_last_write;
	int m_state;
	int m_bytes_in_sector;

	optional_region_ptr<uint8_t> m_default_data;

	// I/O operations
	DECLARE_WRITE_LINE_MEMBER( set_a9_12v );
	DECLARE_WRITE_LINE_MEMBER( set_oe_12v );

	void at28c64b_map8(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(AT28C64B, at28c64b_device)

#endif // MAME_MACHINE_AT28C64B_H
