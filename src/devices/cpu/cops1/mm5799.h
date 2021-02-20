// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor MM5799 MCU

*/

#ifndef MAME_CPU_COPS1_MM5799_H
#define MAME_CPU_COPS1_MM5799_H

#pragma once

#include "cops1base.h"

// pinout reference

/*
            ____   ____
     K1  1 |*   \_/    | 28 DO1
     K2  2 |           | 27 DO2
     K3  3 |           | 26 DO3
     K4  4 |           | 25 DO4
    INB  5 |           | 24 SI
   SYNC  6 |           | 23 SO
    OSC  7 |  MM5799N  | 22 BLK
     F3  8 |           | 21 Vdd
     F2  9 |           | 20 Sa
     F1 10 |           | 19 Sb
 PWR ON 11 |           | 18 Sc
     Sp 12 |           | 17 Sd
     Sg 13 |           | 16 Se
     Sf 14 |___________| 15 Vss

*/

class mm5799_device : public cops1_base_device
{
public:
	mm5799_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;
	virtual bool op_argument() override;

	void data_map(address_map &map);
	void program_map(address_map &map);
};


DECLARE_DEVICE_TYPE(MM5799, mm5799_device)

#endif // MAME_CPU_COPS1_MM5799_H
