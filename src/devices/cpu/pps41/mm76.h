// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM76 MCU

*/

#ifndef MAME_CPU_PPS41_MM76_H
#define MAME_CPU_PPS41_MM76_H

#pragma once

#include "pps41base.h"

// pinout reference

/*
            ____   ____                         ____   ____
      A  1 |*   \_/    | 42 BP           BP  1 |*   \_/    | 40 A
  EXCLK  2 |           | 41 DIO9         VC  2 |           | 39 DIO9
  CLKIN  3 |           | 40 DIO8      XTLIN  3 |           | 38 DIO8
     VC  4 |           | 39 N/C      XTLOUT  4 |           | 37 DIO7
    Vdd  5 |           | 38 DIO7        Vdd  5 |           | 36 DIO6
    Vss  6 |           | 37 DIO6        PI2  6 |           | 35 DIO5
   TEST  7 |           | 36 DIO5       TEST  7 |           | 34 DIO4
    PI2  8 |           | 35 DIO4        PI6  8 |           | 33 DIO3
    PI6  9 |           | 34 DIO3        PI1  9 |           | 32 DIO2
    PI1 10 |   MM76    | 33 DIO2        PI5 10 |   MM76L   | 31 DIO1
    PI5 11 |   MM76E   | 32 DIO1        PI7 11 |   MM76EL  | 30 DIO0
    PI7 12 |           | 31 DIO0        PI3 12 |           | 29 CLOCK
    PI3 13 |           | 30 CLOCK       PI8 13 |           | 28 DATAO
    PI8 14 |           | 29 DATAO       PI4 14 |           | 27 DATAI
    PI4 15 |           | 28 DATAI        PO 15 |           | 26 RIO4
    Vdd 16 |           | 27 RIO4       INT0 16 |           | 25 RIO3
     PO 17 |           | 26 RIO3       INT1 17 |           | 24 RIO2
   INT0 18 |           | 25 RIO2       RIO5 18 |           | 23 RIO1
   INT1 19 |           | 24 RIO1       RIO6 19 |           | 22 RIO8
   RIO5 20 |           | 23 RIO8        Vss 20 |___________| 21 RIO7
   RIO6 21 |___________| 22 RIO7

*/

class mm76_device : public pps41_base_device
{
public:
	mm76_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mm76_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	void data_48x4(address_map &map);
	void program_0_6k(address_map &map);

	// opcode helpers
	u8 ram_r();
	void ram_w(u8 data);
	void pop_pc();
	void push_pc();
	void op_illegal();
	void op_todo();

	virtual bool op_is_tr(u8 op) override { return (op & 0xf0) == 0x30; }
	virtual bool op_is_eob(u8 op) { return (op & 0xfc) == 0x1c; }
	virtual bool op_is_lb(u8 op) { return (op & 0xf0) == 0x20; }
	virtual bool op_is_lai(u8 op) { return (op & 0xf0) == 0x70; }

	// opcode handlers
	virtual void op_xab();
	virtual void op_lba();
	virtual void op_lb();
	virtual void op_eob();

	virtual void op_sb();
	virtual void op_rb();
	virtual void op_skbf();

	virtual void op_xas();
	virtual void op_lsa();

	virtual void op_l();
	virtual void op_x();
	virtual void op_xdsk();
	virtual void op_xnsk();

	virtual void op_a();
	virtual void op_ac();
	virtual void op_acsk();
	virtual void op_ask();
	virtual void op_com();
	virtual void op_rc();
	virtual void op_sc();
	virtual void op_sknc();
	virtual void op_lai();
	virtual void op_aisk();

	virtual void op_rt();
	virtual void op_rtsk();
	virtual void op_t();
	virtual void op_tl();
	virtual void op_tm();
	virtual void op_tml();
	virtual void op_tr();
	virtual void op_nop();

	virtual void op_skmea();
	virtual void op_skbei();
	virtual void op_skaei();

	virtual void op_ibm();
	virtual void op_ob();
	virtual void op_iam();
	virtual void op_oa();
	virtual void op_ios();
	virtual void op_i1();
	virtual void op_i2c();
	virtual void op_int1h();
	virtual void op_din1();
	virtual void op_int0l();
	virtual void op_din0();
	virtual void op_seg1();
	virtual void op_seg2();
};

class mm76l_device : public mm76_device
{
public:
	mm76l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mm76e_device : public mm76_device
{
public:
	mm76e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mm76e_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	void program_1k(address_map &map);
};

class mm76el_device : public mm76e_device
{
public:
	mm76el_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(MM76, mm76_device)
DECLARE_DEVICE_TYPE(MM76L, mm76l_device)
DECLARE_DEVICE_TYPE(MM76E, mm76e_device)
DECLARE_DEVICE_TYPE(MM76EL, mm76el_device)

#endif // MAME_CPU_PPS41_MM76_H
