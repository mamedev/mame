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


#define MCFG_7404_Y1_CB(_devcb) \
	devcb = &downcast<ttl7404_device &>(*device).set_y1_cb(DEVCB_##_devcb);

#define MCFG_7404_Y2_CB(_devcb) \
	devcb = &downcast<ttl7404_device &>(*device).set_y2_cb(DEVCB_##_devcb);

#define MCFG_7404_Y3_CB(_devcb) \
	devcb = &downcast<ttl7404_device &>(*device).set_y3_cb(DEVCB_##_devcb);

#define MCFG_7404_Y4_CB(_devcb) \
	devcb = &downcast<ttl7404_device &>(*device).set_y4_cb(DEVCB_##_devcb);

#define MCFG_7404_Y5_CB(_devcb) \
	devcb = &downcast<ttl7404_device &>(*device).set_y5_cb(DEVCB_##_devcb);

#define MCFG_7404_Y6_CB(_devcb) \
	devcb = &downcast<ttl7404_device &>(*device).set_y6_cb(DEVCB_##_devcb);

class ttl7404_device : public device_t
{
public:
	// construction/destruction
	ttl7404_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// static configuration helpers
	template <class Object> devcb_base &set_y1_cb(Object &&cb) { return m_y1_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y2_cb(Object &&cb) { return m_y2_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y3_cb(Object &&cb) { return m_y3_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y4_cb(Object &&cb) { return m_y4_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y5_cb(Object &&cb) { return m_y5_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y6_cb(Object &&cb) { return m_y6_func.set_callback(std::forward<Object>(cb)); }

	// public interfaces
	DECLARE_WRITE_LINE_MEMBER( a1_w );
	DECLARE_WRITE_LINE_MEMBER( a2_w );
	DECLARE_WRITE_LINE_MEMBER( a3_w );
	DECLARE_WRITE_LINE_MEMBER( a4_w );
	DECLARE_WRITE_LINE_MEMBER( a5_w );
	DECLARE_WRITE_LINE_MEMBER( a6_w );

	DECLARE_READ_LINE_MEMBER( y1_r );
	DECLARE_READ_LINE_MEMBER( y2_r );
	DECLARE_READ_LINE_MEMBER( y3_r );
	DECLARE_READ_LINE_MEMBER( y4_r );
	DECLARE_READ_LINE_MEMBER( y5_r );
	DECLARE_READ_LINE_MEMBER( y6_r );

protected:
	void a_w(uint8_t line, uint8_t state);
	uint8_t y_r(uint8_t line);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void init();
	void update();

	// callbacks
	devcb_write_line m_y1_func;
	devcb_write_line m_y2_func;
	devcb_write_line m_y3_func;
	devcb_write_line m_y4_func;
	devcb_write_line m_y5_func;
	devcb_write_line m_y6_func;

	// inputs
	uint8_t m_a;        // pins 1,3,5,9,11,13

	// outputs
	uint8_t m_y;        // pins 2,4,6,8,10,12
};

// device type definition
DECLARE_DEVICE_TYPE(TTL7404, ttl7404_device)

#endif // MAME_MACHINE_7404_H
