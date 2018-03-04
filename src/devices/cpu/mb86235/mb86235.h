// license:BSD-3-Clause
// copyright-holders:Angelo Salese, ElSemi
/*****************************************************************************
 *
 * template for CPU cores
 *
 *****************************************************************************/

#ifndef MAME_CPU_MB86235_MB86235_H
#define MAME_CPU_MB86235_MB86235_H

#pragma once

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"

class mb86235_frontend;


class mb86235_device :  public cpu_device
{
	friend class mb86235_frontend;

public:
	// construction/destruction
	mb86235_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t clock);

	void unimplemented_op();
	void unimplemented_alu();
	void unimplemented_control();
	void unimplemented_double_xfer1();
	void unimplemented_double_xfer2();
	void pcs_overflow();
	void pcs_underflow();

	void fifoin_w(uint64_t data);
	bool is_fifoin_full();
	uint64_t fifoout0_r();
	bool is_fifoout0_empty();

	enum
	{
		MB86235_PC = 1,
		MB86235_AA0, MB86235_AA1, MB86235_AA2, MB86235_AA3, MB86235_AA4, MB86235_AA5, MB86235_AA6, MB86235_AA7,
		MB86235_AB0, MB86235_AB1, MB86235_AB2, MB86235_AB3, MB86235_AB4, MB86235_AB5, MB86235_AB6, MB86235_AB7,
		MB86235_MA0, MB86235_MA1, MB86235_MA2, MB86235_MA3, MB86235_MA4, MB86235_MA5, MB86235_MA6, MB86235_MA7,
		MB86235_MB0, MB86235_MB1, MB86235_MB2, MB86235_MB3, MB86235_MB4, MB86235_MB5, MB86235_MB6, MB86235_MB7,
		MB86235_AR0, MB86235_AR1, MB86235_AR2, MB86235_AR3, MB86235_AR4, MB86235_AR5, MB86235_AR6, MB86235_AR7,
	};

	static constexpr int FIFOIN_SIZE = 16;
	static constexpr int FIFOOUT0_SIZE = 16;
	static constexpr int FIFOOUT1_SIZE = 16;

	void internal_abus(address_map &map);
	void internal_bbus(address_map &map);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 1; }
	virtual uint32_t execute_max_cycles() const override { return 7; }
	virtual uint32_t execute_input_lines() const override { return 0; }
	virtual void execute_run() override;
	//virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual util::disasm_interface *create_disassembler() override;

	direct_read_data<-3> *m_direct;

private:

	struct mb86235_flags
	{
		uint32_t az;
		uint32_t an;
		uint32_t av;
		uint32_t au;
		uint32_t ad;
		uint32_t zc;
		uint32_t il;
		uint32_t nr;
		uint32_t zd;
		uint32_t mn;
		uint32_t mz;
		uint32_t mv;
		uint32_t mu;
		uint32_t md;
	};

	struct fifo
	{
		int rpos;
		int wpos;
		int num;
		uint64_t data[16];
	};

	struct mb86235_internal_state
	{
		uint32_t pc;
		uint32_t aa[8];
		uint32_t ab[8];
		uint32_t ma[8];
		uint32_t mb[8];
		uint32_t ar[8];

		uint32_t sp;
		uint32_t eb;
		uint32_t eo;
		uint32_t rpc;
		uint32_t lpc;

		uint32_t prp;
		uint32_t pwp;
		uint32_t pr[24];

		uint32_t mod;
		mb86235_flags flags;

		int icount;

		uint32_t arg0;
		uint32_t arg1;
		uint32_t arg2;
		uint32_t arg3;
		uint64_t arg64;

		uint32_t pcs[4];
		int pcs_ptr;

		uint32_t jmpdest;
		uint32_t alutemp;
		uint32_t multemp;
		uint32_t condtemp;

		uint32_t pdr;
		uint32_t ddr;

		float fp0;

		fifo fifoin;
		fifo fifoout0;
		fifo fifoout1;
	};

	mb86235_internal_state  *m_core;

	uml::parameter   m_regmap[32];

	uml::code_handle *m_entry;                      /* entry point */
	uml::code_handle *m_nocode;                     /* nocode exception handler */
	uml::code_handle *m_out_of_cycles;              /* out of cycles exception handler */
	uml::code_handle *m_clear_fifo_in;
	uml::code_handle *m_clear_fifo_out0;
	uml::code_handle *m_clear_fifo_out1;
	uml::code_handle *m_read_fifo_in;
	uml::code_handle *m_write_fifo_out0;
	uml::code_handle *m_write_fifo_out1;
	uml::code_handle *m_read_abus;
	uml::code_handle *m_write_abus;

	address_space_config m_program_config;
	address_space_config m_dataa_config;
	address_space_config m_datab_config;
	drc_cache m_cache;
	std::unique_ptr<drcuml_state> m_drcuml;
	std::unique_ptr<mb86235_frontend> m_drcfe;

	address_space *m_program;
	address_space *m_dataa;
	address_space *m_datab;

	/* internal compiler state */
	struct compiler_state
	{
		uint32_t cycles;                             /* accumulated cycles */
		uint8_t  checkints;                          /* need to check interrupts before next instruction */
		uml::code_label  labelnum;                 /* index for local labels */
	};

	void run_drc();
	void flush_cache();
	void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	void compile_block(offs_t pc);
	void load_fast_iregs(drcuml_block *block);
	void save_fast_iregs(drcuml_block *block);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_fifo();
	void static_generate_memory_accessors();
	void generate_sequence_instruction(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_update_cycles(drcuml_block *block, compiler_state *compiler, uml::parameter param, bool allow_exception);
	bool generate_opcode(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_alu(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int aluop, bool alu_temp);
	void generate_mul(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int mulop, bool mul_temp);
	void generate_pre_control(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_control(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_xfer1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_double_xfer1(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_xfer2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_double_xfer2(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_xfer3(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_branch(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc);
	void generate_ea(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int md, int arx, int ary, int disp);
	void generate_reg_read(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int reg, uml::parameter dst);
	void generate_reg_write(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int reg, uml::parameter src);
	void generate_alumul_input(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int reg, uml::parameter dst, bool fp, bool mul);
	uml::parameter get_alu1_input(int reg);
	uml::parameter get_alu_output(int reg);
	uml::parameter get_mul1_input(int reg);
	void generate_condition(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int cc, bool n, uml::code_label skip_label, bool condtemp);
	void generate_branch_target(drcuml_block *block, compiler_state *compiler, const opcode_desc *desc, int type, int ef2);
	bool has_register_clash(const opcode_desc *desc, int outreg);
	bool aluop_has_result(int aluop);
};


DECLARE_DEVICE_TYPE(MB86235, mb86235_device)

#endif // MAME_CPU_MB86235_MB86235_H
