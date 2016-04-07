// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS1000 family - base/shared

  Don't include this file, include the specific device header instead,
  for example tms1000.h

*/

#ifndef _TMS1KBASE_H_
#define _TMS1KBASE_H_

#include "emu.h"
#include "machine/pla.h"


// K input pins
#define MCFG_TMS1XXX_READ_K_CB(_devcb) \
	tms1k_base_device::set_read_k_callback(*device, DEVCB_##_devcb);

// O/Segment output pins
#define MCFG_TMS1XXX_WRITE_O_CB(_devcb) \
	tms1k_base_device::set_write_o_callback(*device, DEVCB_##_devcb);

// Use this if the output PLA is unknown:
// If the microinstructions (or other) PLA is unknown, try using one from another romset.
#define MCFG_TMS1XXX_OUTPUT_PLA(_pla) \
	tms1k_base_device::set_output_pla(*device, _pla);

// R output pins (also called D on some chips)
#define MCFG_TMS1XXX_WRITE_R_CB(_devcb) \
	tms1k_base_device::set_write_r_callback(*device, DEVCB_##_devcb);

// OFF request on TMS0980 and up
#define MCFG_TMS1XXX_POWER_OFF_CB(_devcb) \
	tms1k_base_device::set_power_off_callback(*device, DEVCB_##_devcb);


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
	// construction/destruction
	tms1k_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_BIG, byte_bits > 8 ? 16 : 8, prgwidth, 0, program)
		, m_data_config("data", ENDIANNESS_BIG, 8, datawidth, 0, data)
		, m_mpla(*this, "mpla")
		, m_ipla(*this, "ipla")
		, m_opla(*this, "opla")
		, m_spla(*this, "spla")
		, m_o_pins(o_pins)
		, m_r_pins(r_pins)
		, m_pc_bits(pc_bits)
		, m_byte_bits(byte_bits)
		, m_x_bits(x_bits)
		, m_output_pla_table(nullptr)
		, m_read_k(*this)
		, m_write_o(*this)
		, m_write_r(*this)
		, m_power_off(*this)
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_k_callback(device_t &device, _Object object) { return downcast<tms1k_base_device &>(device).m_read_k.set_callback(object); }
	template<class _Object> static devcb_base &set_write_o_callback(device_t &device, _Object object) { return downcast<tms1k_base_device &>(device).m_write_o.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r_callback(device_t &device, _Object object) { return downcast<tms1k_base_device &>(device).m_write_r.set_callback(object); }
	template<class _Object> static devcb_base &set_power_off_callback(device_t &device, _Object object) { return downcast<tms1k_base_device &>(device).m_power_off.set_callback(object); }
	static void set_output_pla(device_t &device, const UINT16 *output_pla) { downcast<tms1k_base_device &>(device).m_output_pla_table = output_pla; }

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
		F_BR =    (1<<0),
		F_CALL =  (1<<1),
		F_CLO =   (1<<2),
		F_COMC =  (1<<3),
		F_COMX =  (1<<4),
		F_COMX8 = (1<<5),
		F_LDP =   (1<<6),
		F_LDX =   (1<<7),
		F_RBIT =  (1<<8),
		F_RETN =  (1<<9),
		F_RSTR =  (1<<10),
		F_SBIT =  (1<<11),
		F_SETR =  (1<<12),
		F_TDO =   (1<<13),
		F_TPC =   (1<<14),

		F_OFF =   (1<<15),
		F_REAC =  (1<<16),
		F_SAL =   (1<<17),
		F_SBL =   (1<<18),
		F_SEAC =  (1<<19),
		F_XDA =   (1<<20)
	};

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 6; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return(spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_DATA) ? &m_data_config : nullptr); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 1; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	void next_pc();

	virtual void write_o_output(UINT8 index);
	virtual UINT8 read_k_input();
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

	UINT8   m_pc;        // 6 or 7-bit program counter
	UINT32  m_sr;        // 6 or 7-bit subroutine return register(s)
	UINT8   m_pa;        // 4-bit page address register
	UINT8   m_pb;        // 4-bit page buffer register
	UINT16  m_ps;        // 4-bit page subroutine register(s)
	UINT8   m_a;         // 4-bit accumulator
	UINT8   m_x;         // 2,3,or 4-bit RAM X register
	UINT8   m_y;         // 4-bit RAM Y register
	UINT8   m_ca;        // chapter address register
	UINT8   m_cb;        // chapter buffer register
	UINT16  m_cs;        // chapter subroutine register(s)
	UINT16  m_r;
	UINT16  m_o;
	UINT8   m_cki_bus;
	UINT8   m_c4;
	UINT8   m_p;         // 4-bit adder p(lus)-input
	UINT8   m_n;         // 4-bit adder n(egative)-input
	UINT8   m_adder_out; // adder result
	UINT8   m_carry_in;  // adder carry-in bit
	UINT8   m_carry_out; // adder carry-out bit
	UINT8   m_status;
	UINT8   m_status_latch;
	UINT8   m_eac;       // end around carry bit
	UINT8   m_clatch;    // call latch bit(s)
	UINT8   m_add;       // add latch bit
	UINT8   m_bl;        // branch latch bit

	UINT8   m_ram_in;
	UINT8   m_dam_in;
	int     m_ram_out; // signed!
	UINT8   m_ram_address;
	UINT16  m_rom_address;
	UINT16  m_opcode;
	UINT32  m_fixed;
	UINT32  m_micro;
	int     m_subcycle;
	int     m_icount;

	UINT8   m_o_pins;    // how many O pins
	UINT8   m_r_pins;    // how many R pins
	UINT8   m_pc_bits;   // how many program counter bits
	UINT8   m_byte_bits; // how many bits per 'byte'
	UINT8   m_x_bits;    // how many X register bits

	address_space *m_program;
	address_space *m_data;

	const UINT16 *m_output_pla_table;
	devcb_read8 m_read_k;
	devcb_write16 m_write_o;
	devcb_write16 m_write_r;
	devcb_write_line m_power_off;

	UINT32 m_o_mask;
	UINT32 m_r_mask;
	UINT32 m_k_mask;
	UINT32 m_pc_mask;
	UINT32 m_x_mask;

	// lookup tables
	std::vector<UINT32> m_fixed_decode;
	std::vector<UINT32> m_micro_decode;
	std::vector<UINT32> m_micro_direct;
};


#endif /* _TMS1KBASE_H_ */
