// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    er2055.h

    GI 512 bit electrically alterable read-only memory.

***************************************************************************/

#ifndef MAME_MACHINE_ER2055_H
#define MAME_MACHINE_ER2055_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> er2055_device

class er2055_device :   public device_t,
						public device_memory_interface,
						public device_nvram_interface
{
public:
	// construction/destruction
	er2055_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	uint8_t data() const { return m_data; }
	void set_address(uint8_t address) { m_address = address & 0x3f; }
	void set_data(uint8_t data) { m_data = data; }

	// control lines -- all lines are specified as active-high (even CS2)
	void set_control(uint8_t cs1, uint8_t cs2, uint8_t c1, uint8_t c2);
	DECLARE_WRITE_LINE_MEMBER(set_clk);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual void nvram_read(emu_file &file) override;
	virtual void nvram_write(emu_file &file) override;

private:
	void update_state();

	static const int SIZE_DATA = 0x40;

	static const uint8_t CK  = 0x01;
	static const uint8_t C1  = 0x02;
	static const uint8_t C2  = 0x04;
	static const uint8_t CS1 = 0x08;
	static const uint8_t CS2 = 0x10;

	optional_memory_region      m_region;

	// configuration state
	address_space_config        m_space_config;

	// internal state
	uint8_t       m_control_state;
	uint8_t       m_address;
	uint8_t       m_data;

	void er2055_map(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ER2055, er2055_device)

#endif // MAME_MACHINE_ER2055_H
