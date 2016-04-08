// license:BSD-3-Clause
// copyright-holders:hap, Igor
/*

  Sharp SM500 MCU family cores

*/

#ifndef _SM500_H_
#define _SM500_H_

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
	sm500_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	sm500_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
	virtual void execute_one() override;
	virtual void get_opcode_param() override;

	// opcode handlers
	void op_comcb();
	void op_ssr();
	void op_trs();

	void op_pdtw();
	void op_tw();
	void op_dtw();

	void op_ats();
	void op_exksa();
	void op_exkfa();

	void op_rmf();
	void op_smf();
	void op_comcn();

	void op_ta();
};


class kb1013vk12_device : public sm500_device
{
public:
	kb1013vk12_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type SM500;
extern const device_type KB1013VK12;


#endif /* _SM500_H_ */
