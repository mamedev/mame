// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    ATMEL AT28C16

    16K ( 2K x 8 ) Parallel EEPROM

***************************************************************************/

#pragma once

#ifndef __AT28C16_H__
#define __AT28C16_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_AT28C16_ADD( _tag, _interface ) \
	MCFG_DEVICE_ADD( _tag, AT28C16, 0 )


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
	at28c16_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock );

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void set_a9_12v(int state);
	void set_oe_12v(int state);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config( address_spacenum spacenum = AS_0 ) const override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read( emu_file &file ) override;
	virtual void nvram_write( emu_file &file ) override;

	// internal state
	address_space_config m_space_config;
	emu_timer *m_write_timer;
	int m_a9_12v;
	int m_oe_12v;
	int m_last_write;
	optional_region_ptr<uint8_t> m_default_data;
};


// device type definition
extern const device_type AT28C16;

#endif
