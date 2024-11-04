// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B6000 MCU

*/

#ifndef MAME_CPU_RW5000_B6000_H
#define MAME_CPU_RW5000_B6000_H

#pragma once

#include "b5000.h"

// pinout reference (preliminary)

/*
            _____   _____
   SEG0  1 |*    \_/     | 42 SEG0P
   SEG1  2 |             | 41 NC
    GRD  3 |             | 40 NC
   SEG2  4 |             | 39 KB1
   SEG3  5 |             | 38 KB4
   SEG4  6 |             | 37 KB2
   SEG5  7 |             | 36 DIN1
   SEG6  8 |             | 35 ?
   SEG7  9 |             | 34 KB3
   STR0 10 |    B6000    | 33 NC
   STR1 11 |             | 32 NC
   STR2 12 |             | 31 NC
   STR3 13 |             | 30 NC
   STR4 14 |             | 29 NC
   STR5 15 |             | 28 NC
   STR6 16 |             | 27 NC
   STR7 17 |             | 26 ?
   STR8 18 |             | 25 ?
    VDD 19 |             | 24 VC?
    VSS 20 |      _      | 23 CLK
     NC 21 |_____/ \_____| 22 NC

*/

class b6000_cpu_device : public b5000_cpu_device
{
public:
	b6000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	b6000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device-level overrides
	virtual void device_reset() override ATTR_COLD;

	virtual void execute_one() override;
	virtual u16 decode_digit(u8 data) override;

	void program_512x8(address_map &map) ATTR_COLD;

	// opcode handlers
	virtual void op_tkbs() override;
	virtual void op_atbz();
};


DECLARE_DEVICE_TYPE(B6000, b6000_cpu_device)

#endif // MAME_CPU_RW5000_B6000_H
