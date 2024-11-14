// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    5/7404 Hex Inverters

***********************************************************************

    Connection Diagram:

            SN5404 J Package
            SN54LS04, SN54S04 J or W Package
            SN7404 D, DB, N or NS Package
            SN74S04 D or N Package
              ___ ___
       1A  1 |*  u   | 14  Vcc
       1Y  2 |       | 13  6A
       2A  3 |       | 12  6Y
       2Y  4 |       | 11  5A
       3A  5 |       | 10  5Y
       3Y  6 |       | 9   4A
      GND  7 |_______| 8   4Y


            SN5404 W Package
              ___ ___
       1A  1 |*  u   | 14  1Y
       2Y  2 |       | 13  6A
       2A  3 |       | 12  6Y
      Vcc  4 |       | 11  GND
       3A  5 |       | 10  5Y
       3Y  6 |       | 9   5A
       4A  7 |_______| 8   4Y


        SN54LS04, SN54S04 FK Package

          1Y  1A  NC  Vcc 6A
         _______________________
        /  |_| |_| |_| |_| |_|  |
        |_  3   2   1  20  19  _|
    2A  |_|                 18|_|  6Y
        |_                     _|
    NC  |_|                 17|_|  NC
        |_                     _|
    2Y  |_|                 16|_|  5A
        |_                     _|
    NC  |_|                 15|_|  NC
        |_                     _|
    3A  |_|                 14|_|  5Y
        |   9  10  11  12  13   |
        |__|=|_|=|_|=|_|=|_|=|__|

            3Y  GND NC  4Y  4A

**********************************************************************/

#ifndef MAME_MACHINE_7404_H
#define MAME_MACHINE_7404_H

#pragma once

class ttl7404_device : public device_t
{
public:
	// construction/destruction
	ttl7404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <std::size_t Bit> auto y_cb() { return m_y_func[Bit].bind(); }

	// public interfaces
	void a1_w(int state);
	void a2_w(int state);
	void a3_w(int state);
	void a4_w(int state);
	void a5_w(int state);
	void a6_w(int state);

	int y1_r();
	int y2_r();
	int y3_r();
	int y4_r();
	int y5_r();
	int y6_r();

protected:
	void a_w(uint8_t line, uint8_t state);
	uint8_t y_r(uint8_t line);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update();

	// callbacks
	devcb_write_line::array<6> m_y_func;

	// inputs
	uint8_t m_a;        // pins 1,3,5,9,11,13

	// outputs
	uint8_t m_y;        // pins 2,4,6,8,10,12
};

// device type definition
DECLARE_DEVICE_TYPE(TTL7404, ttl7404_device)

#endif // MAME_MACHINE_7404_H
