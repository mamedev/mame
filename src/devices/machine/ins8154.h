// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    National Semiconductor INS8154

    N-Channel 128-by-8 Bit RAM Input/Output (RAM I/O)

                            _____   _____
                   PB6   1 |*    \_/     | 40  VCC
                   PB5   2 |             | 39  PB7
                   PB4   3 |             | 38  NWDS
                   PB3   4 |             | 37  NRDS
                   PB2   5 |             | 36  NRST
                   PB1   6 |             | 35  _CS0
                   PB0   7 |             | 34  CS1
                   DB7   8 |             | 33  M/_IO
                   DB6   9 |             | 32  AD6
                   DB5  10 |   INS8154   | 31  AD5
                   DB4  11 |             | 30  AD4
                   DB3  12 |             | 29  AD3
                   DB2  13 |             | 28  AD2
                   DB1  14 |             | 27  AD1
                   DB0  15 |             | 26  AD0
                   PA7  16 |             | 25  INTR
                   PA6  17 |             | 24  PA0
                   PA5  18 |             | 23  PA1
                   PA4  19 |             | 22  PA2
                   GND  20 |_____________| 21  PA3

***************************************************************************/

#ifndef MAME_MACHINE_INS8154_H
#define MAME_MACHINE_INS8154_H

#pragma once

class ins8154_device :  public device_t
{
public:
	// construction/destruction
	ins8154_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto in_a() { return m_in_a_cb.bind(); }
	auto out_a() { return m_out_a_cb.bind(); }
	auto in_b() { return m_in_b_cb.bind(); }
	auto out_b() { return m_out_b_cb.bind(); }
	auto out_irq() { return m_out_irq_cb.bind(); }

	uint8_t read_io(offs_t offset);
	void write_io(offs_t offset, uint8_t data);
	uint8_t read_ram(offs_t offset);
	void write_ram(offs_t offset, uint8_t data);

	void porta_w(uint8_t data);
	void portb_w(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

private:

	/* i/o lines */
	devcb_read8         m_in_a_cb;
	devcb_write8        m_out_a_cb;
	devcb_read8         m_in_b_cb;
	devcb_write8        m_out_b_cb;
	devcb_write_line    m_out_irq_cb;

	/* registers */
	uint8_t m_in_a;  /* Input Latch Port A */
	uint8_t m_in_b;  /* Input Latch Port B */
	uint8_t m_out_a; /* Output Latch Port A */
	uint8_t m_out_b; /* Output Latch Port B */
	uint8_t m_mdr;   /* Mode Definition Register */
	uint8_t m_odra;  /* Output Definition Register Port A */
	uint8_t m_odrb;  /* Output Definition Register Port B */

	/* on-board RAM */
	uint8_t m_ram[0x80];
};


// device type definition
DECLARE_DEVICE_TYPE(INS8154, ins8154_device)

#endif // MAME_MACHINE_INS8154_H
