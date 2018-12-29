// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS1000 family - base/shared

  Don't include this file, include the specific device header instead,
  for example tms1000.h

*/

#ifndef MAME_CPU_TMS1000_TMS1K_BASE_H
#define MAME_CPU_TMS1000_TMS1K_BASE_H

#pragma once

#include "machine/pla.h"


// HALT input pin on CMOS chips (use set_input_line)
#define TMS1XXX_INPUT_LINE_HALT 0


// pinout reference

/*

            ____   ____                         ____   ____
     R8  1 |*   \_/    | 28 R7           R0  1 |*   \_/    | 28 Vss
     R9  2 |           | 27 R6           R1  2 |           | 27 OSC2
    R10  3 |           | 26 R5           R2  3 |           | 26 OSC1
    Vdd  4 |           | 25 R4           R3  4 |           | 25 O0
     K1  5 |           | 24 R3           R4  5 |           | 24 O1
     K2  6 |  TMS1000  | 23 R2           R5  6 |           | 23 O2
     K4  7 |  TMS1070  | 22 R1           R6  7 |  TMS1400  | 22 O3
     K8  8 |  TMS1100  | 21 R0           R7  8 |           | 21 O4
   INIT  9 |  TMS1170  | 20 Vss          R8  9 |           | 20 O5
     O7 10 |           | 19 OSC2         R9 10 |           | 19 O6
     O6 11 |           | 18 OSC1        R10 11 |           | 18 O7
     O5 12 |           | 17 O0          Vdd 12 |           | 17 K8
     O4 13 |           | 16 O1         INIT 13 |           | 16 K4
     O3 14 |___________| 15 O2           K1 14 |___________| 15 K2


            ____   ____
     R2  1 |*   \_/    | 28 R3
     R1  2 |           | 27 R4
     R0  3 |           | 26 R5
      ?  4 |           | 25 R6
    Vdd  5 |           | 24 R7
     K3  6 |           | 23 R8
     K8  7 |  TMS0980  | 22 ?
     K4  8 |           | 21 ?
     K2  9 |           | 20 Vss
     K1 10 |           | 19 ?
     O7 11 |           | 18 O0
     O6 12 |           | 17 O1
     O5 13 |           | 16 O2
     O4 14 |___________| 15 O3

  note: TMS0980 official pin names for R0-R8 is D9-D1, O0-O7 is S(A-G,DP)

*/


class tms1k_base_device : public cpu_device
{
public:
	// K input pins
	auto k() { return m_read_k.bind(); }

	// O/Segment output pins
	auto o() { return m_write_o.bind(); }

	// R output pins (also called D on some chips)
	auto r() { return m_write_r.bind(); }

	// OFF request on TMS0980 and up
	auto power_off() { return m_power_off.bind(); }

	// Use this if the output PLA is unknown:
	// If the microinstructions (or other) PLA is unknown, try using one from another romset.
	void set_output_pla(const u16 *output_pla) { m_output_pla_table = output_pla; }

	u8 debug_peek_o_index() { return m_o_index; } // get output PLA index, for debugging (don't use in emulation)

protected:
	// construction/destruction
	tms1k_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const override { return 1; }
	virtual u32 execute_max_cycles() const override { return 1; }
	virtual u32 execute_input_lines() const override { return 1; }
	virtual void execute_set_input(int line, int state) override;
	virtual void execute_run() override;
	virtual void execute_one();

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// microinstructions
	enum
	{
		M_15TN  = (1<<0),  /* 15 to -ALU */
		M_ATN   = (1<<1),  /* ACC to -ALU */
		M_AUTA  = (1<<2),  /* ALU to ACC */
		M_AUTY  = (1<<3),  /* ALU to Y */
		M_C8    = (1<<4),  /* CARRY8 to STATUS */
		M_CIN   = (1<<5),  /* Carry In to ALU */
		M_CKM   = (1<<6),  /* CKB to MEM */
		M_CKN   = (1<<7),  /* CKB to -ALU */
		M_CKP   = (1<<8),  /* CKB to +ALU */
		M_MTN   = (1<<9),  /* MEM to -ALU */
		M_MTP   = (1<<10), /* MEM to +ALU */
		M_NATN  = (1<<11), /* ~ACC to -ALU */
		M_NE    = (1<<12), /* COMP to STATUS */
		M_STO   = (1<<13), /* ACC to MEM */
		M_STSL  = (1<<14), /* STATUS to Status Latch */
		M_YTP   = (1<<15), /* Y to +ALU */

		M_CME   = (1<<16), /* Conditional Memory Enable */
		M_DMTP  = (1<<17), /* DAM to +ALU */
		M_NDMTP = (1<<18), /* ~DAM to +ALU */
		M_SSE   = (1<<19), /* Special Status Enable */
		M_SSS   = (1<<20), /* Special Status Sample */

		M_SETR  = (1<<21), /* -> line #0d, F_SETR (TP0320 custom), */
		M_RSTR  = (1<<22), /* -> line #36, F_RSTR (TMS02x0 custom), */
		M_UNK1  = (1<<23)  /* -> line #37, F_???? (TMS0270 custom), */
	};

	// standard/fixed instructions - these are documented more in their specific handlers
	enum
	{
		F_BR    = (1<<0),
		F_CALL  = (1<<1),
		F_CLO   = (1<<2),
		F_COMC  = (1<<3),
		F_COMX  = (1<<4),
		F_COMX8 = (1<<5),
		F_LDP   = (1<<6),
		F_LDX   = (1<<7),
		F_RBIT  = (1<<8),
		F_RETN  = (1<<9),
		F_RSTR  = (1<<10),
		F_SBIT  = (1<<11),
		F_SETR  = (1<<12),
		F_TDO   = (1<<13),
		F_TPC   = (1<<14),

		F_OFF   = (1<<15),
		F_REAC  = (1<<16),
		F_SAL   = (1<<17),
		F_SBL   = (1<<18),
		F_SEAC  = (1<<19),
		F_XDA   = (1<<20)
	};

	void next_pc();

	virtual void write_o_output(u8 index);
	virtual u8 read_k_input();
	virtual void set_cki_bus();
	virtual void dynamic_output() { ; } // not used by default
	virtual void read_opcode();

	virtual void op_br();
	virtual void op_call();
	virtual void op_retn();
	virtual void op_br3();
	virtual void op_call3();
	virtual void op_retn3();

	virtual void op_sbit();
	virtual void op_rbit();
	virtual void op_setr();
	virtual void op_rstr();
	virtual void op_tdo();
	virtual void op_clo();
	virtual void op_ldx();
	virtual void op_comx();
	virtual void op_comx8();
	virtual void op_ldp();

	virtual void op_comc();
	virtual void op_tpc();
	virtual void op_xda();
	virtual void op_off();
	virtual void op_seac();
	virtual void op_reac();
	virtual void op_sal();
	virtual void op_sbl();

	address_space_config m_program_config;
	address_space_config m_data_config;

	optional_device<pla_device> m_mpla;
	optional_device<pla_device> m_ipla;
	optional_device<pla_device> m_opla;
	optional_device<pla_device> m_spla;

	u8 m_pc;        // 6 or 7-bit program counter
	u32 m_sr;       // 6 or 7-bit subroutine return register(s)
	u8 m_pa;        // 4-bit page address register
	u8 m_pb;        // 4-bit page buffer register
	u16 m_ps;       // 4-bit page subroutine register(s)
	u8 m_a;         // 4-bit accumulator
	u8 m_x;         // 2,3,or 4-bit RAM X register
	u8 m_y;         // 4-bit RAM Y register
	u8 m_ca;        // chapter address register
	u8 m_cb;        // chapter buffer register
	u16 m_cs;       // chapter subroutine register(s)
	u16 m_r;
	u16 m_o;
	u8 m_cki_bus;
	u8 m_c4;
	u8 m_p;         // 4-bit adder p(lus)-input
	u8 m_n;         // 4-bit adder n(egative)-input
	u8 m_adder_out; // adder result
	u8 m_carry_in;  // adder carry-in bit
	u8 m_carry_out; // adder carry-out bit
	u8 m_status;
	u8 m_status_latch;
	u8 m_eac;       // end around carry bit
	u8 m_clatch;    // call latch bit(s)
	u8 m_add;       // add latch bit
	u8 m_bl;        // branch latch bit

	u8 m_ram_in;
	u8 m_dam_in;
	int m_ram_out;  // signed!
	u8 m_ram_address;
	u16 m_rom_address;
	u16 m_opcode;
	u32 m_fixed;
	u32 m_micro;
	int m_subcycle;
	int m_icount;
	u8 m_o_index;
	bool m_halt_pin;

	u8 m_o_pins;    // how many O pins
	u8 m_r_pins;    // how many R pins
	u8 m_pc_bits;   // how many program counter bits
	u8 m_byte_bits; // how many bits per 'byte'
	u8 m_x_bits;    // how many X register bits

	address_space *m_program;
	address_space *m_data;

	const u16 *m_output_pla_table;
	devcb_read8 m_read_k;
	devcb_write16 m_write_o;
	devcb_write16 m_write_r;
	devcb_write_line m_power_off;

	u32 m_o_mask;
	u32 m_r_mask;
	u32 m_k_mask;
	u32 m_pc_mask;
	u32 m_x_mask;

	// lookup tables
	std::vector<u32> m_fixed_decode;
	std::vector<u32> m_micro_decode;
	std::vector<u32> m_micro_direct;
};


#endif // MAME_CPU_TMS1000_TMS1K_BASE_H
