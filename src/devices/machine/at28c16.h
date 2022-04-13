// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    ATMEL AT28C16

    16K ( 2K x 8 ) Parallel EEPROM

***************************************************************************/

#ifndef MAME_MACHINE_AT28C16_H
#define MAME_MACHINE_AT28C16_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> at28c16_device

class at28c16_device :
	public device_t,
	public device_memory_interface,
	public device_nvram_interface
{
public:
	// construction/destruction
	at28c16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);
	uint8_t read(offs_t offset);

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
	address_space_config m_space_config;
	emu_timer *m_write_timer;
	int m_a9_12v;
	int m_oe_12v;
	int m_last_write;
	optional_region_ptr<uint8_t> m_default_data;

	// I/O operations
	DECLARE_WRITE_LINE_MEMBER( set_a9_12v );
	DECLARE_WRITE_LINE_MEMBER( set_oe_12v );

	void at28c16_map8(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(AT28C16, at28c16_device)

#endif // MAME_MACHINE_AT28C16_H
