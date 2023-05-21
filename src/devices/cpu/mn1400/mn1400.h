// license:BSD-3-Clause
// copyright-holders:hap
/*

  Matsushita MN1400, MN1405

*/

#ifndef MAME_CPU_MN1400_MN1400_H
#define MAME_CPU_MN1400_MN1400_H

#pragma once

#include "mn1400base.h"

// pinout reference

/*
            ____   ____                      ____   ____
    Vss  1 |    \_/    | 40 OSC      Vss  1 |    \_/    | 28 OSC
   CO11  2 |           | 39 Vdd      CO9  2 |           | 27 Vdd
   CO10  3 |           | 38 DO7      CO8  3 |           | 26 SNS1
    CO9  4 |           | 37 DO6      CO7  4 |           | 25 SNS0
    CO8  5 |           | 36 DO5      CO6  5 |           | 24 DO0
    CO7  6 |           | 35 DO4      CO5  6 |  MN1400   | 23 DO1
    CO6  7 |           | 34 DO3      AI3  7 |  MN1402   | 22 DO2
    CO5  8 |  MN1400   | 33 DO2      AI2  8 |  MN1432   | 21 DO3
    CO4  9 |  MN1405   | 32 DO1      AI1  9 |           | 20 TST
    CO3 10 |  MN1430   | 31 DO0      AI0 10 |           | 19 RST
    CO2 11 |  MN1435   | 30 SNS1     BI3 11 |           | 18 EO3
    CO1 12 |  MN1450   | 29 SNS0     BI2 12 |           | 17 EO2
    CO0 13 |  MN1455   | 28 CSLCT    BI1 13 |           | 16 EO1
    AI3 14 |           | 27 RST      BI0 14 |___________| 15 EO0
    AI2 15 |           | 26 TST
    AI1 16 |           | 25 EO3
    AI0 17 |           | 24 EO2
    BI3 18 |           | 23 EO1
    BI2 19 |           | 22 EO0
    BI1 20 |___________| 21 BI0

*/

class mn1400_cpu_device : public mn1400_base_device
{
public:
	mn1400_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mn1400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int stack_levels, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;
	virtual bool op_has_param(u8 op) override;

	// opcode helpers
	u8 ram_r();
	void ram_w(u8 data);
	void set_z(u8 data);
	void set_cz(u8 data);
	void op_illegal();

	// opcode handlers
	void op_l();
	void op_ld();
	void op_li();
	void op_lic();
	void op_ldc();
	void op_st();
	void op_std();
	void op_stic();
	void op_stdc();
	void op_lx();
	void op_ly();
	void op_tax();
	void op_tay();
	void op_tya();
	void op_tacu();
	void op_tacl();
	void op_tcau();
	void op_tcal();

	void op_nop();
	void op_and();
	void op_andi();
	void op_or();
	void op_xor();
	void op_a();
	void op_ai();
	void op_cpl();
	void op_c();
	void op_ci();
	void op_cy();
	void op_sl();
	void op_icy();
	void op_dcy();
	void op_icm();
	void op_dcm();
	void op_sm();
	void op_rm();
	void op_tb();

	void op_ina();
	void op_inb();
	void op_otd();
	void op_otmd();
	void op_ote();
	void op_otie();
	void op_rco();
	void op_sco();
	void op_cco();

	void op_rc();
	void op_rp();
	void op_sc();
	void op_sp();
	void op_bs01();
	void op_bpcz();
	void op_jmp();
	void op_cal();
	void op_ret();
	void op_ec();
	void op_dc();
};


class mn1400_reduced_cpu_device : public mn1400_cpu_device
{
public:
	mn1400_reduced_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


class mn1405_cpu_device : public mn1400_cpu_device
{
public:
	mn1405_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(MN1400_40PINS, mn1400_cpu_device)
DECLARE_DEVICE_TYPE(MN1400_28PINS, mn1400_reduced_cpu_device)
DECLARE_DEVICE_TYPE(MN1405, mn1405_cpu_device)

#endif // MAME_CPU_MN1400_MN1400_H
