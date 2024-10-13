// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS2100, TMS2170, TMS2300, TMS2370

*/

#ifndef MAME_CPU_TMS1000_TMS2100_H
#define MAME_CPU_TMS1000_TMS2100_H

#pragma once

#include "tms1100.h"

enum
{
	TMS2100_INPUT_LINE_INT = 0,
	TMS2100_INPUT_LINE_EC1
};


// pinout reference

/*
            ____   ____
   AVss  1 |*   \_/    | 28 INT
   AVdd  2 |           | 27 OSC OUT
     R0  3 |           | 26 OSC IN
     R1  4 |           | 25 Vdd
     R2  5 |           | 24 A1
     R3  6 |           | 23 K1
     R4  7 |  TMS2100  | 22 K2
     R5  8 |           | 21 K4
     R6  9 |           | 20 K8
    Vss 10 |           | 19 INIT
     O7 11 |           | 18 O0
     O6 12 |           | 17 O1
     O5 13 |           | 16 O2
     O4 14 |___________| 15 O3

            ____   ____
   AVss  1 |*   \_/    | 40 EC1
   AVdd  2 |           | 39 INT
     R0  3 |           | 38 OSC OUT
     R1  4 |           | 37 OSC IN
     R2  5 |           | 36 Vdd
     R3  6 |           | 35 J1
     R4  7 |           | 34 J2
     R5  8 |           | 33 J4/A1
     R6  9 |           | 32 J8/A2
     R7 10 |  TMS2300  | 31 K1
     R8 11 |           | 30 K2
     R9 12 |           | 29 K4
    R10 13 |           | 28 K8
    R11 14 |           | 27 INIT
    R12 15 |           | 26 O0
    R13 16 |           | 25 O1
    R14 17 |           | 24 O2
    Vss 18 |           | 23 O3
     O7 19 |           | 22 O4
     O6 20 |___________| 21 O5

  note: TMS2170/TMS2370 chips change the highest R pin to Vpp, other than that they are the same

*/


class tms2100_cpu_device : public tms1100_cpu_device
{
public:
	tms2100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms2100_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	// overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_clock_changed() override { reset_prescaler(); }

	virtual void execute_set_input(int line, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == TMS2100_INPUT_LINE_INT || inputnum == TMS2100_INPUT_LINE_EC1; }

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void clock_decrementer();
	TIMER_CALLBACK_MEMBER(prescaler_timeout);
	void reset_prescaler();
	virtual void read_opcode() override;
	void interrupt();

	virtual u8 read_k_input() override;
	virtual void write_r_output(u32 data) override;

	virtual void op_call() override;
	virtual void op_retn() override;
	virtual void op_tax() override;
	virtual void op_tra() override;
	virtual void op_tac() override;
	virtual void op_tca() override;
	virtual void op_tadm() override;
	virtual void op_tma() override;

	emu_timer *m_prescaler;

	// internal state
	u8 m_ac2;           // 4-bit storage register, or R0-R3 output latch
	u8 m_ivr;           // 8-bit initial value register
	u8 m_dec;           // 8-bit decrementer counter
	u8 m_il;            // interrupt latch(es)
	bool m_int_pending;
	u32 m_r_prev;
	int m_int_pin;      // INT pin state
	int m_ec1_pin;      // EC1 pin state

	// interrupt stack
	u8 m_pb_stack;
	u8 m_cb_stack;
	u8 m_a_stack;
	u8 m_ac2_stack;
	u8 m_x_stack;
	u8 m_y_stack;
	u8 m_s_stack;
	u8 m_sl_stack;
	u8 m_o_stack;
};

class tms2170_cpu_device : public tms2100_cpu_device
{
public:
	tms2170_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class tms2300_cpu_device : public tms2100_cpu_device
{
public:
	tms2300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	tms2300_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);
};

class tms2370_cpu_device : public tms2300_cpu_device
{
public:
	tms2370_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(TMS2100, tms2100_cpu_device)
DECLARE_DEVICE_TYPE(TMS2170, tms2170_cpu_device)
DECLARE_DEVICE_TYPE(TMS2300, tms2300_cpu_device)
DECLARE_DEVICE_TYPE(TMS2370, tms2370_cpu_device)

#endif // MAME_CPU_TMS1000_TMS2100_H
