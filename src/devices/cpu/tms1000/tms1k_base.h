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


class tms1k_base_device : public cpu_device
{
public:
	// common handlers
	auto read_k() { return m_read_k.bind(); } // K input pins
	auto write_o() { return m_write_o.bind(); } // O/Segment output pins
	auto write_r() { return m_write_r.bind(); } // R output pins (also called D on some chips)

	// TMS2100 handlers
	auto read_j() { return m_read_j.bind(); } // J input pins
	auto read_r() { return m_read_r.bind(); } // R0-R3 input pins
	auto &set_option_dec_div(u8 div) { m_option_dec_div = div; return *this; }

	// OFF request on TMS0980 and up
	auto power_off() { return m_power_off.bind(); }

	// note: for HALT input pin on CMOS chips, use set_input_line with INPUT_LINE_HALT
	// similarly with the INIT pin, simply use INPUT_LINE_RESET

	// TMS0270 was designed to interface with TMS5100, set it up at driver level
	auto read_ctl() { return m_read_ctl.bind(); }
	auto write_ctl() { return m_write_ctl.bind(); }
	auto write_pdc() { return m_write_pdc.bind(); }

	// Use this if the output PLA is unknown:
	void set_output_pla(const u16 *output_pla) { m_output_pla_table = output_pla; }

	// If the microinstructions PLA is unknown, try using one from another romset.
	// If that's not possible, use this callback:
	auto set_decode_micro() { return m_decode_micro.bind(); }

	u8 debug_peek_o_index() { return m_o_index; } // get output PLA index, for debugging (don't use in emulation)

protected:
	// construction/destruction
	tms1k_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 o_pins, u8 r_pins, u8 pc_bits, u8 byte_bits, u8 x_bits, u8 stack_levels, int rom_width, address_map_constructor rom_map, int ram_width, address_map_constructor ram_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
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

		F_TAX   = (1<<15),
		F_TXA   = (1<<16),
		F_TRA   = (1<<17),
		F_TAC   = (1<<18),
		F_TCA   = (1<<19),
		F_TADM  = (1<<20),
		F_TMA   = (1<<21),

		F_OFF   = (1<<22),
		F_REAC  = (1<<23),
		F_SAL   = (1<<24),
		F_SBL   = (1<<25),
		F_SEAC  = (1<<26),
		F_XDA   = (1<<27)
	};

	void rom_10bit(address_map &map);
	void rom_11bit(address_map &map);
	void rom_12bit(address_map &map);
	void ram_6bit(address_map &map);
	void ram_7bit(address_map &map);
	void ram_8bit(address_map &map);

	void next_pc();

	virtual void write_o_reg(u8 index);
	virtual void write_o_output(u16 data) { m_write_o(data & m_o_mask); }
	virtual void write_r_output(u32 data) { m_write_r(data & m_r_mask); }
	virtual u8 read_k_input() { return m_read_k() & 0xf; }
	virtual void set_cki_bus();
	virtual void dynamic_output() { ; } // not used by default
	virtual void read_opcode();

	virtual void op_br();
	virtual void op_call();
	virtual void op_retn();

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

	virtual void op_tax() { ; }
	virtual void op_txa() { ; }
	virtual void op_tra() { ; }
	virtual void op_tac() { ; }
	virtual void op_tca() { ; }
	virtual void op_tadm() { ; }
	virtual void op_tma() { ; }

	virtual void op_xda() { ; }
	virtual void op_off() { ; }
	virtual void op_seac() { ; }
	virtual void op_reac() { ; }
	virtual void op_sal() { ; }
	virtual void op_sbl() { ; }

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	optional_device<pla_device> m_mpla;
	optional_device<pla_device> m_ipla;
	optional_device<pla_device> m_opla;
	optional_memory_region m_opla_b; // binary dump of output PLA, in place of PLA file
	optional_device<pla_device> m_spla;

	// internal state
	u8 m_pc;            // 6 or 7-bit program counter
	u32 m_sr;           // 6 or 7-bit subroutine return register(s)
	u8 m_pa;            // 4-bit page address register
	u8 m_pb;            // 4-bit page buffer register
	u16 m_ps;           // 4-bit page subroutine register(s)
	u8 m_a;             // 4-bit accumulator
	u8 m_x;             // 2,3,or 4-bit RAM X register
	u8 m_y;             // 4-bit RAM Y register
	u8 m_ca;            // chapter address register
	u8 m_cb;            // chapter buffer register
	u16 m_cs;           // chapter subroutine register(s)
	u32 m_r;
	u16 m_o;
	u8 m_cki_bus;
	u8 m_c4;
	u8 m_p;             // 4-bit adder p(lus)-input
	u8 m_n;             // 4-bit adder n(egative)-input
	u8 m_adder_out;     // adder result
	u8 m_carry_in;      // adder carry-in bit
	u8 m_carry_out;     // adder carry-out bit
	u8 m_status;
	u8 m_status_latch;
	u8 m_eac;           // end around carry bit
	u8 m_clatch;        // call latch bit(s)
	u8 m_add;           // add latch bit
	u8 m_bl;            // branch latch bit

	u8 m_ram_in;
	u8 m_dam_in;
	int m_ram_out;      // signed!
	u8 m_ram_address;
	u16 m_rom_address;
	u16 m_opcode;
	u32 m_fixed;
	u32 m_micro;
	int m_subcycle;
	u8 m_o_index;

	// fixed settings or mask options
	u8 m_o_pins;        // how many O pins
	u8 m_r_pins;        // how many R pins
	u8 m_pc_bits;       // how many program counter bits
	u8 m_byte_bits;     // how many bits per 'byte'
	u8 m_x_bits;        // how many X register bits
	u8 m_stack_levels;  // number of stack levels (max 4)

	u32 m_o_mask;
	u32 m_r_mask;
	u32 m_pc_mask;
	u32 m_x_mask;

	u8 m_option_dec_div;

	// i/o handlers
	devcb_read8 m_read_k;
	devcb_write16 m_write_o;
	devcb_write32 m_write_r;

	devcb_read8 m_read_j;
	devcb_read8 m_read_r;

	devcb_write_line m_power_off;

	devcb_read8 m_read_ctl;
	devcb_write8 m_write_ctl;
	devcb_write_line m_write_pdc;

	const u16 *m_output_pla_table;
	devcb_read32 m_decode_micro;

	int m_icount;
	int m_state_count;

	// lookup tables
	std::vector<u32> m_fixed_decode;
	std::vector<u32> m_micro_decode;
	std::vector<u32> m_micro_direct;
};


#endif // MAME_CPU_TMS1000_TMS1K_BASE_H
