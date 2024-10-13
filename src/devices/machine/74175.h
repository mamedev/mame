// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    5/74174/5 Hex/Quad D Flip-Flops with Clear

***********************************************************************

    Connection Diagram:
              ___ ___                         ___ ___
    CLEAR  1 |*  u   | 16  Vcc      CLEAR  1 |*  u   | 16  Vcc
       Q1  2 |       | 15  Q6          Q1  2 |       | 15  Q4
       D1  3 |       | 14  D6         /Q1  3 |       | 14  /Q4
       D2  4 |       | 13  D5          D1  4 |       | 13  D4
       Q2  5 |       | 12  Q5          D2  5 |       | 12  D3
       D3  6 |       | 11  D4         /Q2  6 |       | 11  /Q3
       Q3  7 |       | 10  Q4          Q2  7 |       | 10  Q3
      GND  8 |_______| 9   CLOCK      GND  8 |_______| 9   CLOCK

              5/74174                         5/74175

***********************************************************************

    Function Table:
     _________________________________
    |       Inputs        |  Outputs* |
    |---------------------|-----------|
    | Clear | Clock |  D  |  Q  | /Q  |
    |-------|-------|-----|-----|-----|
    |   L   |   X   |  X  |  L  |  H  |
    |   H   |   ^   |  H  |  H  |  L  |
    |   H   |   ^   |  L  |  L  |  H  |
    |   H   |   L   |  X  |  Q0 |  Q0 |
    |_______|_______|_____|_____|_____|

    H = High Level (steady state)
    L = Low Level (steady state)
    X = Don't Care
    ^ = Transition from low to high level
    Q0 = The level of Q before the indicated steady-state input conditions were established.
    * = 175 only

**********************************************************************/

#ifndef MAME_MACHINE_74175_H
#define MAME_MACHINE_74175_H

#pragma once


class ttl741745_device : public device_t
{
public:
	auto q1_callback() { return m_q1_func.bind(); }
	auto q2_callback() { return m_q2_func.bind(); }
	auto q3_callback() { return m_q3_func.bind(); }
	auto q4_callback() { return m_q4_func.bind(); }

	void clear_w(int state);
	void d1_w(int state);
	void d2_w(int state);
	void d3_w(int state);
	void d4_w(int state);
	void clock_w(int state);

	uint8_t q_w();

protected:
	ttl741745_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void init();
	virtual void tick();

	devcb_write_line m_q1_func;
	devcb_write_line m_q2_func;
	devcb_write_line m_q3_func;
	devcb_write_line m_q4_func;

	uint8_t m_clock;
	uint8_t m_clear;

	uint8_t m_d1;
	uint8_t m_d2;
	uint8_t m_d3;
	uint8_t m_d4;

	uint8_t m_q1;
	uint8_t m_q2;
	uint8_t m_q3;
	uint8_t m_q4;
};

class ttl74174_device : public ttl741745_device
{
public:
	ttl74174_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto q5_cb() { return m_q5_func.bind(); }
	auto q6_cb() { return m_q6_func.bind(); }

	void d5_w(int state);
	void d6_w(int state);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void init() override;
	virtual void tick() override;

private:
	devcb_write_line m_q5_func;
	devcb_write_line m_q6_func;

	uint8_t m_d5;
	uint8_t m_d6;

	uint8_t m_q5;
	uint8_t m_q6;
};

class ttl74175_device : public ttl741745_device
{
public:
	ttl74175_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto not_q1_cb() { return m_not_q1_func.bind(); }
	auto not_q2_cb() { return m_not_q2_func.bind(); }
	auto not_q3_cb() { return m_not_q3_func.bind(); }
	auto not_q4_cb() { return m_not_q4_func.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual void tick() override;

private:
	devcb_write_line m_not_q1_func;
	devcb_write_line m_not_q2_func;
	devcb_write_line m_not_q3_func;
	devcb_write_line m_not_q4_func;

	uint8_t m_not_q1;
	uint8_t m_not_q2;
	uint8_t m_not_q3;
	uint8_t m_not_q4;
};

// device type definition
DECLARE_DEVICE_TYPE(TTL74174, ttl74174_device)
DECLARE_DEVICE_TYPE(TTL74175, ttl74175_device)

#endif // MAME_MACHINE_74175_H
