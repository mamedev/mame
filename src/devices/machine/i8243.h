// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    i8243.h

    Intel 8243 Input/Output Expander

****************************************************************************
                            _____   _____
                   P50   1 |*    \_/     | 24  VCC
                   P40   2 |             | 23  P51
                   P41   3 |             | 22  P52
                   P42   4 |             | 21  P53
                   P43   5 |             | 20  P60
                   _CS   6 |             | 19  P61
                  PROG   7 |    8243     | 18  P62
                   P23   8 |             | 17  P63
                   P22   9 |             | 16  P73
                   P21  10 |             | 15  P72
                   P20  11 |             | 14  P71
                   GND  12 |_____________| 13  P70

***************************************************************************/

#ifndef MAME_MACHINE_I8243_H
#define MAME_MACHINE_I8243_H

#pragma once

class i8243_device :  public device_t
{
public:
	// construction/destruction
	i8243_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	auto p4_in_cb() { return m_readhandler[0].bind(); }
	auto p4_out_cb() { return m_writehandler[0].bind(); }
	auto p5_in_cb() { return m_readhandler[1].bind(); }
	auto p5_out_cb() { return m_writehandler[1].bind(); }
	auto p6_in_cb() { return m_readhandler[2].bind(); }
	auto p6_out_cb() { return m_writehandler[2].bind(); }
	auto p7_in_cb() { return m_readhandler[3].bind(); }
	auto p7_out_cb() { return m_writehandler[3].bind(); }

	uint8_t p2_r();
	void p2_w(uint8_t data);

	void prog_w(int state);
	void cs_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	void output_update(int which);

	uint8_t         m_p[4];         // 4 ports' worth of data
	uint8_t         m_p2out;        // port 2 bits that will be returned
	uint8_t         m_p2;           // most recent port 2 value
	uint8_t         m_opcode;       // latched opcode
	bool            m_prog;         // previous PROG state
	bool            m_cs;           // chip select

	devcb_read8::array<4> m_readhandler;
	devcb_write8::array<4> m_writehandler;
};


// device type definition
DECLARE_DEVICE_TYPE(I8243, i8243_device)

#endif // MAME_MACHINE_I8243_H
