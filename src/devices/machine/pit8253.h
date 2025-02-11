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

enum class pit_type
{
	I8254,
	I8253,
	FE2010
};

class pit_counter_device : public device_t
{
	friend class pit8253_device;
	friend class pit8254_device;

public:
	// construction/destruction
	pit_counter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	inline uint32_t adjusted_count() const;
	inline void adjust_timer(attotime target);
	void decrease_counter_value(int64_t cycles);
	void load_counter_value();
	void set_output(int output);
	void simulate(int64_t elapsed_cycles);
	TIMER_CALLBACK_MEMBER(update_tick);
	void update();
	uint16_t masked_value() const;
	uint8_t read();
	void load_count(uint16_t newcount);
	void readback(int command);
	void control_w(uint8_t data) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(pit_counter_device::control_w_deferred), this), data); }
	TIMER_CALLBACK_MEMBER(control_w_deferred);
	void count_w(uint8_t data) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(pit_counter_device::count_w_deferred), this), data); }
	TIMER_CALLBACK_MEMBER(count_w_deferred);
	void gate_w(int state) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(pit_counter_device::gate_w_deferred), this), state); }
	TIMER_CALLBACK_MEMBER(gate_w_deferred);
	void set_clock_signal(int state);
	void set_clockin(double new_clockin);

	// internal state
	int m_index;                // index number of the timer
	double m_clockin;           // input clock frequency in Hz
	attotime m_clock_period;    // precomputed input clock period
	int m_clock_signal;         // clock signal when clockin is 0

	attotime m_last_updated;    // time when last updated
	attotime m_next_update;     // time of next update

	emu_timer *m_update_timer;  // MAME timer to process updates

	uint16_t m_value;           // current counter value ("CE" in Intel docs)
	uint16_t m_latch;           // latched counter value ("OL" in Intel docs)
	uint16_t m_count;           // new counter value ("CR" in Intel docs)
	uint8_t m_control;          // 6-bit control byte
	uint8_t m_status;           // status byte - 8254 only
	uint8_t m_lowcount;         // LSB of new counter value for 16-bit writes
	bool m_rmsb;                // true = Next read is MSB of 16-bit value
	bool m_wmsb;                // true = Next write is MSB of 16-bit value
	int m_output;               // 0 = low, 1 = high

	int m_gate;                 // gate input (0 = low, 1 = high)
	int m_latched_count;        // number of bytes of count latched
	int m_latched_status;       // 1 = status latched (8254 only)
	int m_null_count;           // 1 = mode control or count written, 0 = count loaded
	int m_phase;                // see phase definition tables in simulate2(), below
};

DECLARE_DEVICE_TYPE(PIT_COUNTER, pit_counter_device)

class pit8253_device : public device_t
{
	friend class pit_counter_device;

public:
	// construction/destruction
	pit8253_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configuration helpers
	template <unsigned N> void set_clk(double clk) { m_clk[N] = clk; }
	template <unsigned N> void set_clk(const XTAL &xtal) { set_clk<N>(xtal.dvalue()); }
	template <unsigned N> auto out_handler() { return m_out_handler[N].bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void write_gate0(int state) { m_counter[0]->gate_w(state); }
	void write_gate1(int state) { m_counter[1]->gate_w(state); }
	void write_gate2(int state) { m_counter[2]->gate_w(state); }

	/* In the 8253/8254 the CLKx input lines can be attached to a regular clock
	 signal. Another option is to use the output from one timer as the input
	 clock to another timer.

	 The functions below should supply both functionalities. If the signal is
	 a regular clock signal, use the pit8253_set_clockin function. If the
	 CLKx input signal is the output of the different source, set the new_clockin
	 to 0 with pit8253_set_clockin and call pit8253_clkX_w to change
	 the state of the input CLKx signal.
	 */
	void write_clk0(int state) { m_counter[0]->set_clock_signal(state); }
	void write_clk1(int state) { m_counter[1]->set_clock_signal(state); }
	void write_clk2(int state) { m_counter[2]->set_clock_signal(state); }

	void set_clockin(int timer, double new_clockin) { m_counter[timer]->set_clockin(new_clockin); }

protected:
	pit8253_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, pit_type chip_type);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	virtual void readback_command(uint8_t data);

	double m_clk[3];
	devcb_write_line::array<3> m_out_handler;

	required_device_array<pit_counter_device, 3> m_counter;

	pit_type m_type;
};

DECLARE_DEVICE_TYPE(PIT8253, pit8253_device)

class pit8254_device : public pit8253_device
{
public:
	pit8254_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void readback_command(uint8_t data) override;
};

DECLARE_DEVICE_TYPE(PIT8254, pit8254_device)

class fe2010_pit_device : public pit8253_device
{
public:
	fe2010_pit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(FE2010_PIT, fe2010_pit_device)
#endif // MAME_MACHINE_PIT8253_H
