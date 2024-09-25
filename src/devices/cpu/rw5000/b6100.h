// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell B6100 MCU

*/

#ifndef MAME_CPU_RW5000_B6100_H
#define MAME_CPU_RW5000_B6100_H

#pragma once

#include "b6000.h"

// pinout reference

/*
            _____   _____
     NC  1 |*    \_/     | 42 NC
     NC  2 |             | 41 NC
    VSS  3 |             | 40 VDD
     NC  4 |             | 39 STR8
   C12P  5 |             | 38 STR7
     NC  6 |             | 37 STR6
     NC  7 |             | 36 STR5
     NC  8 |             | 35 STR4
     VC  9 |             | 34 STR3
   DIN4 10 |    B6100    | 33 STR2
   DIN2 11 |             | 32 STR1
   DIN3 12 |             | 31 STR0
   DIN1 13 |             | 30 SEG9
     PO 14 |             | 29 SEG8
    KB1 15 |             | 28 SEG10
    KB2 16 |             | 27 SEG1
    KB3 17 |             | 26 SEG4
    KB4 18 |             | 25 SEG5
  SPKOP 19 |             | 24 SEG7
   SPKO 20 |      _      | 23 SEG6
   SEG2 21 |_____/ \_____| 22 SEG3

*/

class b6100_cpu_device : public b6000_cpu_device
{
public:
	b6100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	virtual bool op_is_tl(u8 op) override;
	virtual bool op_is_lb(u8 op) override;
	virtual void reset_pc() override { set_pc(0, 0); }
	virtual u8 sr_page() override { return 15; }
	virtual u16 decode_digit(u8 data) override;

	void program_896x8(address_map &map) ATTR_COLD;
	void data_48x4(address_map &map) ATTR_COLD;

	// opcode handlers
	virtual void op_tkbs() override;
	virtual void op_read() override;
};


DECLARE_DEVICE_TYPE(B6100, b6100_cpu_device)

#endif // MAME_CPU_RW5000_B6100_H
