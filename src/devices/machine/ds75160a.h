// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    National Semiconductor DS75160A IEEE-488 GPIB Transceiver emulation

**********************************************************************
                            _____   _____
                    TE   1 |*    \_/     | 20  Vcc
                    D1   2 |             | 19  D1
                    D2   3 |             | 18  D2
                    D3   4 |             | 17  D3
                    D4   5 |   DS75160A  | 16  D4
                    D5   6 |             | 15  D5
                    D6   7 |             | 14  D6
                    D7   8 |             | 13  D7
                    D8   8 |             | 12  D8
                   GND  10 |_____________| 11  PE

**********************************************************************/

#ifndef MAME_MACHINE_DS75160A_H
#define MAME_MACHINE_DS75160A_H

#pragma once


///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> ds75160a_device

class ds75160a_device : public device_t
{
public:
	// construction/destruction
	ds75160a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto read_callback() { return m_read.bind(); }
	auto write_callback() { return m_write.bind(); }

	uint8_t read();
	void write(uint8_t data);

	void te_w(int state);
	void pe_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_read8  m_read;
	devcb_write8 m_write;

	uint8_t m_data;

	int m_te;
	int m_pe;
};


// device type definition
DECLARE_DEVICE_TYPE(DS75160A, ds75160a_device)

#endif // MAME_MACHINE_DS75160A_H
