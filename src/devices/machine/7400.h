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


#define MCFG_7400_Y1_CB(_devcb) \
	devcb = &downcast<ttl7400_device &>(*device).set_y1_cb(DEVCB_##_devcb);

#define MCFG_7400_Y2_CB(_devcb) \
	devcb = &downcast<ttl7400_device &>(*device).set_y2_cb(DEVCB_##_devcb);

#define MCFG_7400_Y3_CB(_devcb) \
	devcb = &downcast<ttl7400_device &>(*device).set_y3_cb(DEVCB_##_devcb);

#define MCFG_7400_Y4_CB(_devcb) \
	devcb = &downcast<ttl7400_device &>(*device).set_y4_cb(DEVCB_##_devcb);


class ttl7400_device : public device_t
{
public:
	// construction/destruction
	ttl7400_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// static configuration helpers
	template <class Object> devcb_base &set_y1_cb(Object &&cb) { return m_y1_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y2_cb(Object &&cb) { return m_y2_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y3_cb(Object &&cb) { return m_y3_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_y4_cb(Object &&cb) { return m_y4_func.set_callback(std::forward<Object>(cb)); }

	// public interfaces
	DECLARE_WRITE_LINE_MEMBER( a1_w );
	DECLARE_WRITE_LINE_MEMBER( a2_w );
	DECLARE_WRITE_LINE_MEMBER( a3_w );
	DECLARE_WRITE_LINE_MEMBER( a4_w );
	DECLARE_WRITE_LINE_MEMBER( b1_w );
	DECLARE_WRITE_LINE_MEMBER( b2_w );
	DECLARE_WRITE_LINE_MEMBER( b3_w );
	DECLARE_WRITE_LINE_MEMBER( b4_w );

	DECLARE_READ_LINE_MEMBER( y1_r );
	DECLARE_READ_LINE_MEMBER( y2_r );
	DECLARE_READ_LINE_MEMBER( y3_r );
	DECLARE_READ_LINE_MEMBER( y4_r );

protected:
	void a_w(uint8_t line, uint8_t state);
	void b_w(uint8_t line, uint8_t state);
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

	// inputs
	uint8_t m_a;        // pins 1,4,9,12
	uint8_t m_b;        // pins 2,5,10,13

	// outputs
	uint8_t m_y;        // pins 3,6,8,11
};

// device type definition
DECLARE_DEVICE_TYPE(TTL7400, ttl7400_device)

#endif // MAME_MACHINE_7400_H
