// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    er2055.h

    GI 512 bit electrically alterable read-only memory.

****************************************************************************
                            _____   _____
                    D3   1 |*    \_/     | 22  D2
                    D7   2 |             | 21  D1
                    D6   3 |             | 20  D0
                    D5   4 |             | 19  Vgi (GND)
                    D4   5 |             | 18  CS2
              Vss (+5V)  6 |   ER-2055   | 17  CS1
                    A5   7 |             | 16  C2
                    A4   8 |             | 15  C1
                    A3   9 |             | 14  Vgg (-28V)
                    A2  10 |             | 13  CLK
                    A1  11 |_____________| 12  A0

***************************************************************************/

#ifndef MAME_MACHINE_ER2055_H
#define MAME_MACHINE_ER2055_H

#pragma once



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> er2055_device

class er2055_device :   public device_t,
						public device_nvram_interface
{
public:
	// construction/destruction
	er2055_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// I/O operations
	uint8_t data() const { return m_data; }
	void set_address(uint8_t address) { m_address = address & 0x3f; }
	void set_data(uint8_t data) { m_data = data; }

	// control lines -- all lines are specified as active-high (even CS2)
	void set_control(uint8_t cs1, uint8_t cs2, uint8_t c1, uint8_t c2);
	void set_clk(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	void update_state();

	static constexpr int SIZE_DATA = 0x40;

	static constexpr uint8_t CK  = 0x01;
	static constexpr uint8_t C1  = 0x02;
	static constexpr uint8_t C2  = 0x04;
	static constexpr uint8_t CS1 = 0x08;
	static constexpr uint8_t CS2 = 0x10;

	optional_region_ptr<uint8_t> m_default_data;

	// internal state
	uint8_t       m_control_state;
	uint8_t       m_address;
	uint8_t       m_data;
	std::unique_ptr<uint8_t[]> m_rom_data;
};


// device type definition
DECLARE_DEVICE_TYPE(ER2055, er2055_device)

#endif // MAME_MACHINE_ER2055_H
