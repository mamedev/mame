// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Mathis Rosenhauer
/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates all the functionality of 6522
    versatile interface adapters.

    This is based on the pre-existing 6821 emulation.

    Written by Mathis Rosenhauer

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  CA1
                   PA0   2 |             | 39  CA2
                   PA1   3 |             | 38  RS0
                   PA2   4 |             | 37  RS1
                   PA3   5 |             | 36  RS2
                   PA4   6 |             | 35  RS3
                   PA5   7 |             | 34  _RES
                   PA6   8 |             | 33  D0
                   PA7   9 |             | 32  D1
                   PB0  10 |             | 31  D2
                   PB1  11 |   MCS6522   | 30  D3
                   PB2  12 |             | 29  D4
                   PB3  13 |             | 28  D5
                   PB4  14 |             | 27  D6
                   PB5  15 |             | 26  D7
                   PB6  16 |             | 25  ϕ2
                   PB7  17 |             | 24  CS1
                   CB1  18 |             | 23  _CS2
                   CB2  19 |             | 22  R/_W
                   Vcc  20 |_____________| 21  _IRQ

**********************************************************************/

#ifndef MAME_MACHINE_6522VIA_H
#define MAME_MACHINE_6522VIA_H

#pragma once

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> via6522_device

class via6522_device : public device_t
{
public:
	enum
	{
		VIA_PB = 0,
		VIA_PA = 1,
		VIA_DDRB = 2,
		VIA_DDRA = 3,
		VIA_T1CL = 4,
		VIA_T1CH = 5,
		VIA_T1LL = 6,
		VIA_T1LH = 7,
		VIA_T2CL = 8,
		VIA_T2CH = 9,
		VIA_SR = 10,
		VIA_ACR = 11,
		VIA_PCR = 12,
		VIA_IFR = 13,
		VIA_IER = 14,
		VIA_PANH = 15
	};

	// TODO: REMOVE THESE
	auto readpa_handler() { return m_in_a_handler.bind(); }
	auto readpb_handler() { return m_in_b_handler.bind(); }

	// TODO: CONVERT THESE TO WRITE LINE
	auto writepa_handler() { return m_out_a_handler.bind(); }
	auto writepb_handler() { return m_out_b_handler.bind(); }

	auto ca2_handler() { return m_ca2_handler.bind(); }
	auto cb1_handler() { return m_cb1_handler.bind(); }
	auto cb2_handler() { return m_cb2_handler.bind(); }
	auto irq_handler() { return m_irq_handler.bind(); }

	void map(address_map &map) ATTR_COLD;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	void write_pa0(int state) { set_pa_line(0, state); }
	void write_pa1(int state) { set_pa_line(1, state); }
	void write_pa2(int state) { set_pa_line(2, state); }
	void write_pa3(int state) { set_pa_line(3, state); }
	void write_pa4(int state) { set_pa_line(4, state); }
	void write_pa5(int state) { set_pa_line(5, state); }
	void write_pa6(int state) { set_pa_line(6, state); }
	void write_pa7(int state) { set_pa_line(7, state); }
	void write_pa(u8 data);
	void write_ca1(int state);
	void write_ca2(int state);

	void write_pb0(int state) { set_pb_line(0, state); }
	void write_pb1(int state) { set_pb_line(1, state); }
	void write_pb2(int state) { set_pb_line(2, state); }
	void write_pb3(int state) { set_pb_line(3, state); }
	void write_pb4(int state) { set_pb_line(4, state); }
	void write_pb5(int state) { set_pb_line(5, state); }
	void write_pb6(int state) { set_pb_line(6, state); }
	void write_pb7(int state) { set_pb_line(7, state); }
	void write_pb(u8 data);
	void write_cb1(int state);
	void write_cb2(int state);

	uint8_t read_pa() const;
	uint8_t read_pb() const;

protected:
	// construction/destruction
	via6522_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(shift_irq_tick);
	TIMER_CALLBACK_MEMBER(shift_tick);
	TIMER_CALLBACK_MEMBER(t1_tick);
	TIMER_CALLBACK_MEMBER(t2_tick);
	TIMER_CALLBACK_MEMBER(ca2_tick);

private:
	uint16_t get_counter1_value();
	void counter2_decrement();

	void set_int(int data);
	void clear_int(int data);
	void shift_out();
	void shift_in();
	void set_pa_line(int line, int state);
	void set_pb_line(int line, int state);

	uint8_t input_pa();
	void output_pa();
	uint8_t input_pb();
	void output_pb();
	void output_irq();

	// TODO: REMOVE THESE
	devcb_read8 m_in_a_handler;
	devcb_read8 m_in_b_handler;

	// TODO: CONVERT THESE TO WRITE LINE
	devcb_write8 m_out_a_handler;
	devcb_write8 m_out_b_handler;

	devcb_write_line m_ca2_handler;
	devcb_write_line m_cb1_handler;
	devcb_write_line m_cb2_handler;
	devcb_write_line m_irq_handler;

	uint8_t m_in_a;
	int m_in_ca1;
	int m_in_ca2;
	uint8_t m_out_a;
	int m_out_ca2;
	uint8_t m_ddr_a;
	uint8_t m_latch_a;

	uint8_t m_in_b;
	int m_in_cb1;
	int m_in_cb2;
	uint8_t m_out_b;
	int m_out_cb1;
	int m_out_cb2;
	uint8_t m_ddr_b;
	uint8_t m_latch_b;

	uint8_t m_t1cl;
	uint8_t m_t1ch;
	uint8_t m_t1ll;
	uint8_t m_t1lh;
	uint8_t m_t2cl;
	uint8_t m_t2ch;
	uint8_t m_t2ll;
	uint8_t m_t2lh;

	uint8_t m_sr;
	uint8_t m_pcr;
	uint8_t m_acr;
	uint8_t m_ier;
	uint8_t m_ifr;

	emu_timer *m_t1;
	attotime m_time1;
	uint8_t m_t1_active;
	int m_t1_pb7;
	emu_timer *m_t2;
	attotime m_time2;
	uint8_t m_t2_active;
	emu_timer *m_ca2_timer;
	emu_timer *m_cb2_timer;

	emu_timer *m_shift_timer;
	emu_timer *m_shift_irq_timer;
	uint8_t m_shift_counter;
};

// ======================> mos6522_device

class mos6522_device : public via6522_device
{
public:
	// construction/destruction
	mos6522_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> r65c22_device

class r65c22_device : public via6522_device
{
public:
	// construction/destruction
	r65c22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> r65nc22_device

class r65nc22_device : public via6522_device
{
public:
	// construction/destruction
	r65nc22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

// ======================> w65c22s_device

class w65c22s_device : public via6522_device
{
public:
	// construction/destruction
	w65c22s_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type declarations
DECLARE_DEVICE_TYPE(MOS6522, mos6522_device)
DECLARE_DEVICE_TYPE(R65C22, r65c22_device)
DECLARE_DEVICE_TYPE(R65NC22, r65nc22_device)
DECLARE_DEVICE_TYPE(W65C22S, w65c22s_device)

#endif // MAME_MACHINE_6522VIA_H
