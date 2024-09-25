// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Xicor X2201 1024 x 1 bit Nonvolatile Static RAM

****************************************************************************
                             ____   ____
                     A0   1 |*   \_/    | 18  Vcc
                     A1   2 |           | 17  A5
                     A2   3 |           | 16  A6
                     A3   4 |           | 15  A7
                     A4   5 |   X2201   | 14  A8
                   DOut   6 |           | 13  A9
                 /STORE   7 |           | 12  DIn
                    /WE   8 |           | 11  /ARRAY RECALL
                    GND   9 |___________| 10  /CS

***************************************************************************/

#ifndef MAME_MACHINE_X2201_H
#define MAME_MACHINE_X2201_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> x2201_device

class x2201_device : public device_t, public device_nvram_interface
{
public:
	// construction/destruction
	x2201_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// control lines
	void cs_w(int state);
	void array_recall_w(int state);
	void store_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	// optional default data
	optional_region_ptr<u8> m_default_data;

	// memory arrays
	std::unique_ptr<u8[]> m_ram;
	std::unique_ptr<u8[]> m_eeprom;

	// line state
	bool m_cs;
	bool m_store;
	bool m_array_recall;
};

// device type definition
DECLARE_DEVICE_TYPE(X2201, x2201_device)

#endif // MAME_MACHINE_X2201_H
