// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM510 MCU family cores

*/

#ifndef MAME_CPU_SM510_SM510_H
#define MAME_CPU_SM510_SM510_H

#pragma once

#include "sm510base.h"


// pinout reference

/*

        b2  a3  b3  a4  b4  a5  b5  GND a6  b6  a7  b7  a8  b8  a9
        45  44  43  42  41  40  39  38  37  36  35  34  33  32  31
       ____________________________________________________________
      |                                                            |
a2 46 |                                                            | 30 b9
      |                                                            |
b1 47 |                                                            | 29 a10
      |                                                            |
a1 48 |                                                            | 28 b10
      |                                                            |
H4 49 |                                                            | 27 a11
      |                                                            |
H3 50 |                                                            | 26 b11
      |                                                            |
H2 51 |                                                            | 25 a12
      |                                                            |
H1 52 |                                                            | 24 b12
      |                           SM510                            |
S1 53 |                           SM511                            | 23 a13
      |                                                            |
S2 54 |                                                            | 22 b13
      |                                                            |
S3 55 |                                                            | 21 a14
      |                                                            |
S4 56 |                                                            | 20 b14
      |                                                            |
S5 57 |                                                            | 19 a15
      |                                                            |
S6 58 |                                                            | 18 b15
      |                                                            |
S7 59 |                                                            | 17 a16
      |                                                            |
S8 60 |  *                                                         | 16 b16
      |____________________________________________________________/

         1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
         T  K1  K2  K3  K4  ACL BA  GND OSC OSC Vdd β   R1  R2  bs
                                        out in
*/

class sm510_device : public sm510_base_device
{
public:
	sm510_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

protected:
	void program_2_7k(address_map &map) ATTR_COLD;
	void data_96_32x4(address_map &map) ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual void execute_one() override;
	virtual bool op_argument() override;

	virtual void update_w_latch() override { m_write_s(m_w); } // W is connected directly to S

	virtual void clock_melody() override;
};


DECLARE_DEVICE_TYPE(SM510, sm510_device)

#endif // MAME_CPU_SM510_SM510_H
