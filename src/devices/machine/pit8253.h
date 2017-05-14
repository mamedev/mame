// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Nathan Woods
/***************************************************************************

    Intel 8253/8254
    Programmable Interval Timer

    As uPD71054C (8MHz), uPD71054C-10 (10MHz) - it is a clone of Intel 82C54
    also available in 28-pin QFP and 44-pin PLCC (many pins NC)

                            _____   _____
                    D7   1 |*    \_/     | 24  VCC
                    D6   2 |             | 23  _WR
                    D5   3 |             | 22  _RD
                    D4   4 |             | 21  _CS
                    D3   5 |             | 20  A1
                    D2   6 |    8253     | 19  A0
                    D1   7 |    8254     | 18  CLK2
                    D0   8 |             | 17  OUT2
                  CLK0   9 |             | 16  GATE2
                  OUT0  10 |             | 15  CLK1
                 GATE0  11 |             | 14  GATE1
                   GND  12 |_____________| 13  OUT1


***************************************************************************/

#ifndef MAME_MACHINE_PIT8253_H
#define MAME_MACHINE_PIT8253_H

#pragma once

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PIT8253_CLK0(_clk) \
	pit8253_device::set_clk0(*device, _clk);

#define MCFG_PIT8253_CLK1(_clk) \
	pit8253_device::set_clk1(*device, _clk);

#define MCFG_PIT8253_CLK2(_clk) \
	pit8253_device::set_clk2(*device, _clk);

#define MCFG_PIT8253_OUT0_HANDLER(_devcb) \
	devcb = &pit8253_device::set_out0_handler(*device, DEVCB_##_devcb);

#define MCFG_PIT8253_OUT1_HANDLER(_devcb) \
	devcb = &pit8253_device::set_out1_handler(*device, DEVCB_##_devcb);

#define MCFG_PIT8253_OUT2_HANDLER(_devcb) \
	devcb = &pit8253_device::set_out2_handler(*device, DEVCB_##_devcb);


class pit8253_device : public device_t
{
public:
	pit8253_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	static void set_clk0(device_t &device, double clk0) { downcast<pit8253_device &>(device).m_clk0 = clk0; }
	static void set_clk1(device_t &device, double clk1) { downcast<pit8253_device &>(device).m_clk1 = clk1; }
	static void set_clk2(device_t &device, double clk2) { downcast<pit8253_device &>(device).m_clk2 = clk2; }
	template <class Object> static devcb_base &set_out0_handler(device_t &device, Object &&cb) { return downcast<pit8253_device &>(device).m_out0_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out1_handler(device_t &device, Object &&cb) { return downcast<pit8253_device &>(device).m_out1_handler.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out2_handler(device_t &device, Object &&cb) { return downcast<pit8253_device &>(device).m_out2_handler.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	WRITE_LINE_MEMBER(write_gate0);
	WRITE_LINE_MEMBER(write_gate1);
	WRITE_LINE_MEMBER(write_gate2);

	/* In the 8253/8254 the CLKx input lines can be attached to a regular clock
	 signal. Another option is to use the output from one timer as the input
	 clock to another timer.

	 The functions below should supply both functionalities. If the signal is
	 a regular clock signal, use the pit8253_set_clockin function. If the
	 CLKx input signal is the output of the different source, set the new_clockin
	 to 0 with pit8253_set_clockin and call pit8253_clkX_w to change
	 the state of the input CLKx signal.
	 */
	WRITE_LINE_MEMBER(write_clk0);
	WRITE_LINE_MEMBER(write_clk1);
	WRITE_LINE_MEMBER(write_clk2);

	void set_clockin(int timer, double new_clockin);

protected:
	pit8253_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal state
	struct pit8253_timer
	{
		int index;              /* index number of the timer */
		double clockin;         /* input clock frequency in Hz */
		int clock;              /* clock signal when clockin is 0 */

		attotime last_updated;  /* time when last updated */

		emu_timer *updatetimer; /* MAME timer to process updates */

		uint16_t value;           /* current counter value ("CE" in Intel docs) */
		uint16_t latch;           /* latched counter value ("OL" in Intel docs) */
		uint16_t count;           /* new counter value ("CR" in Intel docs) */
		uint8_t control;          /* 6-bit control byte */
		uint8_t status;           /* status byte - 8254 only */
		uint8_t lowcount;         /* LSB of new counter value for 16-bit writes */
		int rmsb;               /* 1 = Next read is MSB of 16-bit value */
		int wmsb;               /* 1 = Next write is MSB of 16-bit value */
		int output;             /* 0 = low, 1 = high */

		int gate;               /* gate input (0 = low, 1 = high) */
		int latched_count;      /* number of bytes of count latched */
		int latched_status;     /* 1 = status latched (8254 only) */
		int null_count;         /* 1 = mode control or count written, 0 = count loaded */
		int phase;              /* see phase definition tables in simulate2(), below */
	};

	void readback(pit8253_timer *timer, int command);
	virtual void readback_command(uint8_t data);
	pit8253_timer *get_timer(int which);

private:
	double m_clk0;
	double m_clk1;
	double m_clk2;
	devcb_write_line m_out0_handler;
	devcb_write_line m_out1_handler;
	devcb_write_line m_out2_handler;

	enum
	{
		PIT8253_MAX_TIMER = 3
	};

	pit8253_timer m_timers[PIT8253_MAX_TIMER];

	inline uint32_t adjusted_count(int bcd, uint16_t val);
	void decrease_counter_value(pit8253_timer *timer, int64_t cycles);
	void load_counter_value(pit8253_timer *timer);
	void set_output(pit8253_timer *timer, int output);
	void simulate2(pit8253_timer *timer, int64_t elapsed_cycles);
	void simulate(pit8253_timer *timer, int64_t elapsed_cycles);
	void update(pit8253_timer *timer);
	uint16_t masked_value(pit8253_timer *timer);
	void load_count(pit8253_timer *timer, uint16_t newcount);
	void gate_w(int gate, int state);
	void set_clock_signal(int timerno, int state);
};

DECLARE_DEVICE_TYPE(PIT8253, pit8253_device)


class pit8254_device : public pit8253_device
{
public:
	pit8254_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void readback_command(uint8_t data) override;
};

DECLARE_DEVICE_TYPE(PIT8254, pit8254_device)

#endif // MAME_MACHINE_PIT8253_H
