// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family cores

*/

#ifndef _MELPS4_H_
#define _MELPS4_H_

#include "emu.h"


// pinout reference

/*
            ______   ______
    D9   1 |*     \_/      | 42 D8
    D10  2 |               | 41 D7
    D11  3 |               | 40 D6
  RESET  4 |               | 39 D5
      T  5 |               | 38 D4
     K0  6 |               | 37 D3
     K1  7 |               | 36 D2
     K2  8 |               | 35 D1
     K3  9 |               | 34 D0
     G0 10 |               | 33 Xin
     G1 11 |     M58846    | 32 Xout
     G2 12 |               | 31 S7
     G3 13 |               | 30 S6
      U 14 |               | 29 S5
     F0 15 |               | 28 S4
     F1 16 |               | 27 S3
     F2 17 |               | 26 S2
     F3 18 |               | 25 S1
    INT 19 |               | 24 S0
  CNVss 20 |               | 23 Vp
    Vss 21 |_______________| 22 Vdd

*/


class melps4_cpu_device : public cpu_device
{
public:
	// construction/destruction
	melps4_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_LITTLE, 16, prgwidth, -1, program)
		, m_data_config("data", ENDIANNESS_LITTLE, 8, datawidth, 0, data)
		, m_prgwidth(prgwidth)
		, m_datawidth(datawidth)
		, m_stack_levels(3)
		, m_bm_page(14)
		, m_int_page(12)
		, m_xami_mask(0xf)
	{ }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return (clocks + 6 - 1) / 6; } // 6 t-states per machine cycle
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return (cycles * 6); } // "
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 1; }
	virtual UINT32 execute_input_lines() const { return 3; } // up to 3 (some internal)
	virtual void execute_run();
	virtual void execute_one();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual void state_string_export(const device_state_entry &entry, std::string &str);

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_icount;

	// fixed settings or mask options
	int m_prgwidth;         // number of bits and bitmask for ROM/RAM size
	int m_datawidth;        // "
	int m_prgmask;          // "
	int m_datamask;         // "

	UINT8 m_stack_levels;   // 3 levels on MELPS 4, 12 levels on MELPS 41/42
	UINT8 m_bm_page;        // short BM default page: 14 on '40 to '44, 2 on '45,'46, 0 on '47
	UINT8 m_int_page;       // interrupt routine page: 12 on '40 to '44, 1 on '45,'46, 2 on '47
	UINT8 m_xami_mask;      // mask option for XAMI opcode on '40,'41,'45 (0xf for others)

	// internal state, misc regs
	UINT16 m_pc;            // program counter (11 or 10-bit)
	UINT16 m_prev_pc;
	UINT16 m_stack[12];     // callstack
	UINT16 m_op;
	UINT16 m_prev_op;
	UINT8 m_bitmask;        // opcode bit argument

	UINT8 m_cps;            // DP,CY or DP',CY' selected
	bool m_skip;            // skip next opcode
	UINT8 m_inte;           // interrupt enable flag
	UINT8 m_intp;           // external interrupt polarity ('40 to '44)

	// work registers (unless specified, each is 4-bit)
	UINT8 m_a;              // accumulator
	UINT8 m_b;              // generic
	UINT8 m_e;              // 8-bit register, hold data for S output
	UINT8 m_y, m_y2;        // RAM index Y, Y' (Z.XX.YYYY is DP aka Data Pointer)
	UINT8 m_x, m_x2;        // RAM index X, X', 2-bit
	UINT8 m_z, m_z2;        // RAM index Z, Z', 1-bit, optional
	UINT8 m_cy, m_cy2;      // carry flag(s)

	UINT8 m_h;              // A/D converter H or generic
	UINT8 m_l;              // A/D converter L or generic
	UINT8 m_c;              // A/D converter counter
	UINT8 m_v;              // timer control V
	UINT8 m_w;              // timer control W

	// misc internal helpers
	UINT8 ram_r();
	void ram_w(UINT8 data);
	void pop_pc();
	void push_pc();

	// opcode handlers
	void op_tab();
	void op_tba();
	void op_tay();
	void op_tya();
	void op_teab();
	void op_tepa();
	void op_txa();
	void op_tax();

	void op_lxy();
	void op_lz();
	void op_iny();
	void op_dey();
	void op_lcps();
	void op_sadr();

	void op_tam();
	void op_xam();
	void op_xamd();
	void op_xami();

	void op_la();
	void op_am();
	void op_amc();
	void op_amcs();
	void op_a();
	void op_sc();
	void op_rc();
	void op_szc();
	void op_cma();

	void op_sb();
	void op_rb();
	void op_szb();

	void op_seam();
	void op_sey();

	void op_tla();
	void op_tha();
	void op_taj();
	void op_xal();
	void op_xah();
	void op_lc7();
	void op_dec();
	void op_shl();
	void op_rhl();
	void op_cpa();
	void op_cpas();
	void op_cpae();
	void op_szj();

	void op_t1ab();
	void op_trab();
	void op_t2ab();
	void op_tab1();
	void op_tabr();
	void op_tab2();
	void op_tva();
	void op_twa();
	void op_snz1();
	void op_snz2();

	void op_ba();
	void op_sp();
	void op_b();
	void op_bm();

	void op_rt();
	void op_rts();
	void op_rti();

	void op_cld();
	void op_cls();
	void op_clds();
	void op_sd();
	void op_rd();
	void op_szd();
	void op_osab();
	void op_ospa();
	void op_ose();
	void op_ias();
	void op_ofa();
	void op_iaf();
	void op_oga();
	void op_iak();
	void op_szk();
	void op_su();

	void op_ei();
	void op_di();
	void op_inth();
	void op_intl();

	void op_nop();
	void op_illegal();
};



#endif /* _MELPS4_H_ */
