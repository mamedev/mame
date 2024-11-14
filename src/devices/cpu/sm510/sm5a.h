// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM5A MCU family cores

*/

#ifndef MAME_CPU_SM510_SM5A_H
#define MAME_CPU_SM510_SM5A_H

#pragma once

#include "sm500.h"


// pinout reference

/*

         OS2 OS3 OS4 K4  K3  K2  K1  GND α   β   ACL _R1 _Tp NC  NC   note: on SM5L, pin 31=V1, 32=V2, 33=NC
         45  44  43  42  41  40  39  38  37  36  35  34  33  32  31
        ____________________________________________________________
       |                                                            |
OS1 46 |                                                            | 30 H1
       |                                                            |
O41 47 |                                                            | 29 H2
       |                                                            |
O31 48 |                                                            | 28 Vm
       |                                                            |
O21 49 |                                                            | 27 Vdd
       |                                                            |
O11 50 |                                                            | 26 _R2
       |                                                            |
O42 51 |                                                            | 25 _R3
       |                                                            |
O32 52 |                                                            | 24 _R4
       |                            SM5A                            |
O22 53 |                            SM5L                            | 23 OSCin
       |                                                            |
O12 54 |                                                            | 22 OSCout
       |                                                            |
O43 55 |                                                            | 21 _T2
       |                                                            |
O33 56 |                                                            | 20 _T1
       |                                                            |
O23 57 |                                                            | 19 O18
       |                                                            |
O13 58 |                                                            | 18 O28
       |                                                            |
O44 59 |                                                            | 17 O38
       |                                                            |
O34 60 |  *                                                         | 16 O48
       |____________________________________________________________/

          1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
         O24 O14 O45 O35 O25 O15 O46 GND O36 O26 O16 O47 O37 O27 O17
*/

class sm5a_device : public sm500_device
{
public:
	sm5a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	sm5a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int o_pins, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_1_8k(address_map &map) ATTR_COLD;
	void data_5x13x4(address_map &map) ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void execute_one() override;

	u8 mirror_r(offs_t offset) { return m_data->read_byte(offset & 0x7c); }
	void mirror_w(offs_t offset, u8 data) { m_data->write_byte(offset & 0x7c, data); }

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


DECLARE_DEVICE_TYPE(SM5A, sm5a_device)
DECLARE_DEVICE_TYPE(SM5L, sm5l_device)
DECLARE_DEVICE_TYPE(KB1013VK12, kb1013vk12_device)

#endif // MAME_CPU_SM510_SM5A_H
