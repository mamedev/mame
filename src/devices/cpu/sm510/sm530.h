// license:BSD-3-Clause
// copyright-holders:hap
/*

  Sharp SM530 MCU family cores

*/

#ifndef MAME_CPU_SM510_SM530_H
#define MAME_CPU_SM510_SM530_H

#pragma once

#include "sm511.h"

// pinout reference

/*
                                 OSC OSC
         O21 O31 O41 O11 H1  K3  out in  ACL BA  GND Vm  Vdd DDC Vcc K4  O10 O20 O30 O40 O1B
         60  59  58  57  56  55  54  53  52  51  50  49  48  47  46  45  44  43  42  41  40
        ____________________________________________________________________________________
       |                                                                                    |
O12 61 |                                                                                    | 39 O2B
       |                                                                                    |
O42 62 |                                                                                    | 38 O3B
       |                                                                                    |
O32 63 |                                                                                    | 37 O4B
       |                                                                                    |
O22 64 |                                                                                    | 36 O1A
       |                                                                                    |
O13 65 |                                                                                    | 35 O2A
       |                                                                                    |
O43 66 |                                                                                    | 34 O3A
       |                                                                                    |
O33 67 |                                                                                    | 33 O4A
       |                                                                                    |
O23 68 |                                                                                    | 32 O19
       |                                                                                    |
O14 69 |                                                                                    | 31 O29
       |                                                                                    |
O44 70 |                                                                                    | 30 O39
       |                                      SM530                                         |
O34 71 |                                                                                    | 29 O49
       |                                                                                    |
O24 72 |                                                                                    | 28 O18
       |                                                                                    |
O15 73 |                                                                                    | 27 O28
       |                                                                                    |
O45 74 |                                                                                    | 26 O38
       |                                                                                    |
O35 75 |                                                                                    | 25 O48
       |                                                                                    |
O25 76 |                                                                                    | 24 O17
       |                                                                                    |
O16 77 |                                                                                    | 23 O27
       |                                                                                    |
O46 78 |                                                                                    | 22 O37
       |                                                                                    |
O36 79 |                                                                                    | 21 O47
       |                                                                                    /
O26 80 |  *                                                                                /
       |__________________________________________________________________________________/

          1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20
         H2  K2  KE1 KE2 KE3 KE3 S1  S2  S3  S4  GND SO  NC  NC  F1  F2  F3  F4 TEST K1
*/

class sm530_device : public sm511_device
{
public:
	sm530_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 32768);

	// 4-bit F output port
	auto write_f() { return m_write_f.bind(); }

protected:
	sm530_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_2k(address_map &map) ATTR_COLD;
	void data_64_24x4(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 3 - 1) / 3; } // 3 cycles per machine cycle
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 3); } // "
	virtual void execute_one() override;
	virtual bool op_argument() override;

	virtual void reset_vector() override { do_branch(0xf, 0); }
	virtual void wakeup_vector() override { do_branch(0, 0); }

	devcb_write8 m_write_f;

	// lcd driver
	virtual void lcd_update() override;
	bool m_ds;

	// divider
	virtual TIMER_CALLBACK_MEMBER(div_timer_cb) override;
	u16 m_subdiv;
	u8 m_count_1s;
	u8 m_count_10ms;

	virtual u16 melody_step_mask() override { return 0xff; }

	// opcode handlers
	using sm510_base_device::do_branch;
	virtual void do_branch(u8 pu, u8 pl); // does not have Pm

	virtual void op_incb() override;
	virtual void op_lb() override;
	virtual void op_sabl();

	virtual void op_tl() override;
	virtual void op_trs();

	virtual void op_dta() override;

	virtual void op_adx() override;

	virtual void op_tg();

	virtual void op_keta();
	virtual void op_ats();
	virtual void op_atf();
	virtual void op_sds();
	virtual void op_rds();

	virtual void op_idiv() override;
	virtual void op_inis();
};


DECLARE_DEVICE_TYPE(SM530, sm530_device)

#endif // MAME_CPU_SM510_SM530_H
