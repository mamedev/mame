// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM500 MCU family cores

*/

#ifndef MAME_CPU_SM510_SM500_H
#define MAME_CPU_SM510_SM500_H

#pragma once

#include "sm510.h"


// I/O ports setup

// ..


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
O14 42 |                     SM500                      | 19 VM
O45 43 |                                                | 18 OSCin
O35 44 |                                                | 17 OSCout
O25 45 |                                                | 16 VDD
O15 46 |                                                | 15 K4
O46 47 |                                                | 14 K3
O36 48 | *                                              | 13 K2
       |________________________________________________/

          1   2   3   4   5   6   7   8   9  10  11  12
         O26 O16 R4  R3  R2  R1  GND _T  bt  al  ACL K1   note: bt = beta symbol, al = alpha symbol
*/

class sm500_device : public sm510_base_device
{
public:
	sm500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	sm500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const u8 *oprom, const u8 *opram, u32 options) override;
	virtual void execute_one() override;
	virtual void get_opcode_param() override;

	// opcode handlers
	virtual void op_lb() override;
	virtual void op_incb() override;

	virtual void op_comcb();
	virtual void op_ssr();
	virtual void op_trs();

	virtual void op_pdtw();
	virtual void op_tw();
	virtual void op_dtw();

	virtual void op_ats();
	virtual void op_exksa();
	virtual void op_exkfa();

	virtual void op_rmf();
	virtual void op_smf();
	virtual void op_comcn();
};


DECLARE_DEVICE_TYPE(SM500, sm500_device)

#endif // MAME_CPU_SM510_SM500_H
