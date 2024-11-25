// license:BSD-3-Clause
// copyright-holders:hap
/*

  Rockwell MM77/MM78 MCU

*/

#ifndef MAME_CPU_PPS41_MM78_H
#define MAME_CPU_PPS41_MM78_H

#pragma once

#include "mm76.h"

// pinout reference

/*
            ____   ____                         ____   ____
     BP  1 |*   \_/    | 42 DIO9         BP  1 |*   \_/    | 40 DIO9
      A  2 |           | 41 DIO8          A  2 |           | 39 DIO8
  CLKIN  3 |           | 40 DIO7     XTLOUT  3 |           | 38 DIO7
  EXCLK  4 |           | 39 DIO6      XTLIN  4 |           | 37 DIO6
     VC  5 |           | 38 DIO5         VC  5 |           | 36 DIO5
    Vdd  6 |           | 37 DIO4        Vdd  6 |           | 35 DIO4
    Vss  7 |           | 36 DIO3        Vss  7 |           | 34 DIO3
    N/C  8 |           | 35 DIO2       TEST  8 |           | 33 DIO2
   TEST  9 |           | 34 DIO1        PI4  9 |           | 32 DIO1
    PI4 10 |   MM77    | 33 DIO0        PI8 10 |   MM77L   | 31 DIO0
    PI8 11 |   MM78    | 32 INT1        PI3 11 |   MM78L   | 30 INT1
    PI3 12 |           | 31 INT0        PI7 12 |           | 29 INT0
    PI7 13 |           | 30 DATAI       PI6 13 |           | 28 DATAI
    PI6 14 |           | 29 DATAO       PI2 14 |           | 27 DATAO
    PI2 15 |           | 28 CLOCK       PI5 15 |           | 26 CLOCK
    PI5 16 |           | 27 RIO4        PI1 16 |           | 25 RIO4
    Vdd 17 |           | 26 RIO3         PO 17 |           | 24 RIO3
    PI1 18 |           | 25 RIO2       RIO5 18 |           | 23 RIO2
     PO 19 |           | 24 RIO1       RIO6 19 |           | 22 RIO1
   RIO5 20 |           | 23 RIO8       RIO7 20 |___________| 21 RIO8
   RIO6 21 |___________| 22 RIO7

    MM78 also exists as 40-pin DIP and can have the same pinout as MM78L
*/

class mm78_device : public mm76_device
{
public:
	mm78_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mm78_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override { ; }

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_one() override;

	void data_96x4(address_map &map) ATTR_COLD;
	void data_128x4(address_map &map) ATTR_COLD;
	void program_1_3k(address_map &map) ATTR_COLD;
	void program_1_5k(address_map &map) ATTR_COLD;
	void program_2k(address_map &map) ATTR_COLD;

	// opcode helpers
	virtual bool op_is_eob(u8 op) override { return (op & 0xf8) == 0x08; }
	virtual bool op_is_lb(u8 op) override { return (op & 0xf0) == 0x10; }
	virtual bool op_is_lai(u8 op) override { return (op & 0xf0) == 0x40; }

	// opcode handlers
	virtual void op_lba() override;
	virtual void op_acsk() override;
	virtual void op_aisk() override;
	virtual void op_sb() override;
	virtual void op_rb() override;
	virtual void op_skbf() override;

	virtual void op_sos();
	virtual void op_ros();
	virtual void op_skisl();

	virtual void op_sag();
	virtual void op_lxa();
	virtual void op_xax();
	virtual void op_tlb();
	virtual void op_tmlb();
	virtual void op_tab();
	virtual void op_ix();
	virtual void op_ox();
	virtual void op_ioa();
	virtual void op_i1sk();
	virtual void op_int0h();
	virtual void op_int1l();
};

class mm78l_device : public mm78_device
{
public:
	mm78l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class mm77_device : public mm78_device
{
public:
	mm77_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	mm77_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);
};

class mm77l_device : public mm77_device
{
public:
	mm77l_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};


DECLARE_DEVICE_TYPE(MM78, mm78_device)
DECLARE_DEVICE_TYPE(MM78L, mm78l_device)
DECLARE_DEVICE_TYPE(MM77, mm77_device)
DECLARE_DEVICE_TYPE(MM77L, mm77l_device)

#endif // MAME_CPU_PPS41_MM78_H
