// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    7400 Quad 2-Input NAND Gate

***********************************************************************

    Connection Diagram:
              ___ ___
       1A  1 |*  u   | 14  Vcc
       1B  2 |       | 13  4B
       1Y  3 |       | 12  4A
       2A  4 |       | 11  4Y
       2B  5 |       | 10  3B
       2Y  6 |       | 9   3A
      GND  7 |_______| 8   3Y

    Truth Table:
        ___________
       | A | B | Y |
       |---|---|---|
       | 0 | 0 | 0 |
       | 0 | 1 | 0 |
       | 1 | 0 | 0 |
       | 1 | 1 | 1 |
       |___|___|___|

**********************************************************************/

#ifndef MAME_MACHINE_7400_H
#define MAME_MACHINE_7400_H

#pragma once

class ttl7400_device : public device_t
{
public:
	// construction/destruction
	ttl7400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <std::size_t Bit> auto y_cb() { return m_y_func[Bit].bind(); }

	// public interfaces
	void a1_w(int state);
	void a2_w(int state);
	void a3_w(int state);
	void a4_w(int state);
	void b1_w(int state);
	void b2_w(int state);
	void b3_w(int state);
	void b4_w(int state);

	int y1_r();
	int y2_r();
	int y3_r();
	int y4_r();

protected:
	void a_w(uint8_t line, uint8_t state);
	void b_w(uint8_t line, uint8_t state);
	uint8_t y_r(uint8_t line);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void update();

	// callbacks
	devcb_write_line::array<4> m_y_func;

	// inputs
	uint8_t m_a;        // pins 1,4,9,12
	uint8_t m_b;        // pins 2,5,10,13

	// outputs
	uint8_t m_y;        // pins 3,6,8,11
};

// device type definition
DECLARE_DEVICE_TYPE(TTL7400, ttl7400_device)

#endif // MAME_MACHINE_7400_H
