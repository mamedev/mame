// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM500 MCU family cores

*/

#ifndef MAME_CPU_SM510_SM500_H
#define MAME_CPU_SM510_SM500_H

#pragma once

#include "sm510.h"


// pinout reference

/*

         O33 O43 O12 O22 O32 GND O42 O11 O21 O31 O41 OS1
         36  35  34  33  32  31  30  29  28  27  26  25
        ________________________________________________
       |                                                |
O23 37 |                                                | 24 OS2
O13 38 |                                                | 23 OS3
O44 39 |                                                | 22 OS4
O34 40 |                                                | 21 H1
O24 41 |                                                | 20 H2
O14 42 |                     SM500                      | 19 Vm
O45 43 |                                                | 18 OSCin
O35 44 |                                                | 17 OSCout
O25 45 |                                                | 16 Vdd
O15 46 |                                                | 15 K4
O46 47 |                                                | 14 K3
O36 48 | *                                              | 13 K2
       |________________________________________________/

         1   2   3   4   5   6   7   8   9   10  11  12
         O26 O16 R4  R3  R2  R1  GND _T  bt  al  ACL K1   note: bt = beta symbol, al = alpha symbol


         OS2 OS3 OS4 K4  K3  K2  K1  GND al  bt  ACL _R1 _Tp NC  NC   note: on SM5L, pin 31=V1, 32=V2, 33=NC
         45  44  43  42  41  40  39  38  37  36  35  34  33  32  31
        ____________________________________________________________
       |                                                            |
OS1 46 |                                                            | 30 H1
O41 47 |                                                            | 29 H2
O31 48 |                                                            | 28 Vm
O21 49 |                                                            | 27 Vdd
O11 50 |                                                            | 26 _R2
O42 51 |                                                            | 25 _R3
O32 52 |                          SM5A                              | 24 _R4
O22 53 |                          SM5L                              | 23 OSCin
O12 54 |                                                            | 22 OSCout
O43 55 |                                                            | 21 _T2
O33 56 |                                                            | 20 _T1
O23 57 |                                                            | 19 O18
O13 58 |                                                            | 18 O28
O44 59 |                                                            | 17 O38
O34 60 | *                                                          | 16 O48
       |____________________________________________________________/

         1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
         O24 O14 O45 O35 O25 O15 O46 GND O36 O26 O16 O47 O37 O27 O17
*/

class sm500_device : public sm510_base_device
{
public:
	sm500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	sm500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int o_pins, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_1_2k(address_map &map);
	void data_4x10x4(address_map &map);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void execute_one() override;
	virtual void get_opcode_param() override;
	virtual void clock_melody() override;

	virtual void reset_vector() override { do_branch(0, 0xf, 0); }
	virtual void wakeup_vector() override { do_branch(0, 0, 0); }

	// lcd driver
	virtual void lcd_update() override;

	int m_o_pins; // number of 4-bit O pins
	u8 m_ox[9];   // W' latch, max 9
	u8 m_o[9];    // W latch
	u8 m_cn;
	u8 m_mx;

	u8 m_cb;
	u8 m_s;
	bool m_rsub;

	void shift_w();
	u8 get_digit();
	void set_su(u8 su) { m_stack[0] = (m_stack[0] & ~0x3c0) | (su << 6 & 0x3c0); }
	u8 get_su() { return m_stack[0] >> 6 & 0xf; }
	virtual int get_trs_field() { return 0; }

	// opcode handlers
	virtual void op_lb() override;
	virtual void op_incb() override;
	virtual void op_sbm() override;
	virtual void op_rbm();

	virtual void op_comcb();
	virtual void op_rtn0() override;
	virtual void op_ssr();
	virtual void op_tr();
	virtual void op_trs();

	virtual void op_atbp() override;
	virtual void op_ptw() override;
	virtual void op_tw();
	virtual void op_pdtw();
	virtual void op_dtw();
	virtual void op_wr() override;
	virtual void op_ws() override;

	virtual void op_ats();
	virtual void op_exksa();
	virtual void op_exkfa();

	virtual void op_idiv() override;

	virtual void op_rmf();
	virtual void op_smf();
	virtual void op_comcn();
};


class sm5a_device : public sm500_device
{
public:
	sm5a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	sm5a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int o_pins, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_1_8k(address_map &map);
	void data_5x13x4(address_map &map);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void execute_one() override;
	virtual int get_trs_field() override { return 1; }
};

class sm5l_device : public sm5a_device
{
public:
	sm5l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);
};

class kb1013vk12_device : public sm5a_device
{
public:
	kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);
};


DECLARE_DEVICE_TYPE(SM500, sm500_device)
DECLARE_DEVICE_TYPE(SM5A, sm5a_device)
DECLARE_DEVICE_TYPE(SM5L, sm5l_device)
DECLARE_DEVICE_TYPE(KB1013VK12, kb1013vk12_device)

#endif // MAME_CPU_SM510_SM500_H
