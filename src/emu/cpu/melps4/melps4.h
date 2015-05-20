// license:BSD-3-Clause
// copyright-holders:hap
/*

  Mitsubishi MELPS 4 MCU family cores

*/

#ifndef _MELPS4_H_
#define _MELPS4_H_

#include "emu.h"


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

	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space *m_program;
	address_space *m_data;

	int m_prgwidth;
	int m_datawidth;
	int m_prgmask;
	int m_datamask;
	
	int m_icount;
	
	UINT16 m_pc;            // program counter (11 or 10-bit)
	UINT16 m_prev_pc;
	UINT16 m_stack[3];      // callstack, 3 levels
	UINT16 m_op;
	
	UINT8 m_cps;            // DP,CY or DP',CY' selected
	bool m_skip;            // skip next opcode

	// registers (unless specified, each is 4-bit)
	UINT8 m_a;              // accumulator
	UINT8 m_b;              // generic
	UINT8 m_y, m_y2;        // RAM index Y, Y' (Z.XX.YYYY is DP aka Data Pointer)
	UINT8 m_x, m_x2;        // RAM index X, X', 2-bit
	UINT8 m_z, m_z2;        // RAM index Z, Z', 1-bit, optional
	UINT8 m_cy, m_cy2;      // carry flag(s)
	UINT8 m_e;              // 8-bit register, hold data for S output
	
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
