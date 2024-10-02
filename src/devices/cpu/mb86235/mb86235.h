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
#include "machine/gen_fifo.h"

class mb86235_frontend;

class mb86235_device :  public cpu_device
{
	friend class mb86235_frontend;

public:
	// construction/destruction
	mb86235_device(const machine_config &mconfig, const char *_tag, device_t *_owner, uint32_t clock);
	virtual ~mb86235_device() override;

	template <typename T> void set_fifoin_tag(T &&fifo_tag) { m_fifoin.set_tag(std::forward<T>(fifo_tag)); }
	template <typename T> void set_fifoout0_tag(T &&fifo_tag) { m_fifoout0.set_tag(std::forward<T>(fifo_tag)); }
	template <typename T> void set_fifoout1_tag(T &&fifo_tag) { m_fifoout1.set_tag(std::forward<T>(fifo_tag)); }

	void unimplemented_op();
	void unimplemented_alu();
	void unimplemented_control();
	void unimplemented_double_xfer1();
	void unimplemented_double_xfer2();
	void pcs_overflow();
	void pcs_underflow();

	enum
	{
		MB86235_PC = 1,
		MB86235_AA0, MB86235_AA1, MB86235_AA2, MB86235_AA3, MB86235_AA4, MB86235_AA5, MB86235_AA6, MB86235_AA7,
		MB86235_AB0, MB86235_AB1, MB86235_AB2, MB86235_AB3, MB86235_AB4, MB86235_AB5, MB86235_AB6, MB86235_AB7,
		MB86235_MA0, MB86235_MA1, MB86235_MA2, MB86235_MA3, MB86235_MA4, MB86235_MA5, MB86235_MA6, MB86235_MA7,
		MB86235_MB0, MB86235_MB1, MB86235_MB2, MB86235_MB3, MB86235_MB4, MB86235_MB5, MB86235_MB6, MB86235_MB7,
		MB86235_AR0, MB86235_AR1, MB86235_AR2, MB86235_AR3, MB86235_AR4, MB86235_AR5, MB86235_AR6, MB86235_AR7,
		MB86235_MOD, MB86235_EB,  MB86235_EO,  MB86235_SP,  MB86235_PDR, MB86235_DDR, MB86235_RPC, MB86235_LPC,
		MB86235_PRP, MB86235_PWP,
		MB86235_ST
	};

	void internal_abus(address_map &map) ATTR_COLD;
	void internal_bbus(address_map &map) ATTR_COLD;
protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 7; }
	virtual void execute_run() override;
	//virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

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

	struct fifo_state
	{
		uint32_t pc;
		bool has_stalled;
	};

	struct mb86235_internal_state
	{
		uint32_t pc;
		uint32_t delay_pc;
		uint32_t ppc;
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
		// TODO: remove this, use ST instead
		mb86235_flags flags;
		uint32_t st;

		int icount;

		uint32_t arg0;
		uint32_t arg1;
		uint32_t arg2;
		uint32_t arg3;
		uint64_t arg64;

		uint32_t pcp;       /**< PC stack pointer */
		uint32_t pcs[4];    /**< PC stack contents */

		uint32_t jmpdest;
		uint32_t alutemp;
		uint32_t multemp;
		uint32_t condtemp;

		uint32_t pdr;
		uint32_t ddr;

		float fp0;
		bool delay_slot;

		fifo_state cur_fifo_state;
	};

	mb86235_internal_state  *m_core;

	uml::parameter   m_regmap[32];

	uml::code_handle *m_entry;                      /* entry point */
	uml::code_handle *m_nocode;                     /* nocode exception handler */
	uml::code_handle *m_out_of_cycles;              /* out of cycles exception handler */
	uml::code_handle *m_read_abus;
	uml::code_handle *m_write_abus;

	address_space_config m_program_config;
	address_space_config m_dataa_config;
	address_space_config m_datab_config;
	optional_device<generic_fifo_u32_device> m_fifoin;
	optional_device<generic_fifo_u32_device> m_fifoout0;
	optional_device<generic_fifo_u32_device> m_fifoout1;

	drc_cache m_cache;
	std::unique_ptr<drcuml_state> m_drcuml;
	std::unique_ptr<mb86235_frontend> m_drcfe;

	memory_access<32, 3, -3, ENDIANNESS_LITTLE>::cache m_pcache;
	memory_access<32, 3, -3, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<24, 2, -2, ENDIANNESS_LITTLE>::specific m_dataa;
	memory_access<10, 2, -2, ENDIANNESS_LITTLE>::specific m_datab;

	/* internal compiler state */
	struct compiler_state
	{
		compiler_state &operator=(compiler_state const &) = delete;

		uint32_t cycles;                             /* accumulated cycles */
		uint8_t  checkints;                          /* need to check interrupts before next instruction */
		uml::code_label  labelnum;                 /* index for local labels */
	};

	void run_drc();
	void flush_cache();
	void alloc_handle(uml::code_handle *&handleptr, const char *name);
	void compile_block(offs_t pc);
	void load_fast_iregs(drcuml_block &block);
	void save_fast_iregs(drcuml_block &block);
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessors();
	void generate_sequence_instruction(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_update_cycles(drcuml_block &block, compiler_state &compiler, uml::parameter param, bool allow_exception);
	bool generate_opcode(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_alu(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int aluop, bool alu_temp);
	void generate_mul(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int mulop, bool mul_temp);
	void generate_pre_control(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_control(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_xfer1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_double_xfer1(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_xfer2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_double_xfer2(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_xfer3(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_branch(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc);
	void generate_ea(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int md, int arx, int ary, int disp);
	void generate_reg_read(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int reg, uml::parameter dst);
	void generate_reg_write(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int reg, uml::parameter src);
	void generate_alumul_input(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int reg, uml::parameter dst, bool fp, bool mul);
	uml::parameter get_alu1_input(int reg);
	uml::parameter get_alu_output(int reg);
	uml::parameter get_mul1_input(int reg);
	void generate_condition(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int cc, bool n, uml::code_label skip_label, bool condtemp);
	void generate_branch_target(drcuml_block &block, compiler_state &compiler, const opcode_desc *desc, int type, int ef2);
	bool has_register_clash(const opcode_desc *desc, int outreg);
	bool aluop_has_result(int aluop);
	bool check_previous_op_stall();

	// UML is sad...
	u32 m_cur_value;

	static void clear_fifoin(void *param);
	static void clear_fifoout0(void *param);
	static void clear_fifoout1(void *param);
	static void read_fifoin(void *param);
	static void write_fifoout0(void *param);
	static void write_fifoout1(void *param);
	static void empty_fifoin(void *param);
	static void full_fifoout0(void *param);
	static void full_fifoout1(void *param);

//  interpreter
	void execute_op(uint32_t h, uint32_t l);
	void do_alu1(uint32_t h, uint32_t l);
	void do_alu2(uint32_t h, uint32_t l);
	void do_trans2_1(uint32_t h, uint32_t l);
	void do_trans1_1(uint32_t h, uint32_t l);
	void do_trans2_2(uint32_t h, uint32_t l);
	void do_trans1_2(uint32_t h, uint32_t l);
	void do_trans1_3(uint32_t h, uint32_t l);
	void do_control(uint32_t h, uint32_t l);
	inline uint32_t get_prx(uint8_t which);
	inline uint32_t get_constfloat(uint8_t which);
	inline uint32_t get_constint(uint8_t which);
	inline uint32_t get_alureg(uint8_t which, bool isfloatop);
	inline uint32_t get_mulreg(uint8_t which, bool isfloatop);
	inline void set_alureg(uint8_t which, uint32_t value);
	inline void decode_aluop(uint8_t opcode, uint32_t src1, uint32_t src2, uint8_t imm, uint8_t dst_which);
	inline void decode_mulop(bool isfmul, uint32_t src1, uint32_t src2, uint8_t dst_which);
	inline bool decode_branch_jump(uint8_t which);
	inline uint32_t do_control_dst(uint32_t l);
	inline void push_pc(uint32_t pcval);
	inline uint32_t pop_pc();
	inline void set_mod(uint16_t mod1, uint16_t mod2);
	inline uint32_t get_transfer_reg(uint8_t which);
	inline void set_transfer_reg(uint8_t which, uint32_t value);
	inline uint32_t decode_ea(uint8_t mode, uint8_t rx, uint8_t ry, uint16_t disp, bool isbbus);
	inline uint32_t read_bus(bool isbbus, uint32_t addr);
	inline void write_bus(bool isbbus, uint32_t addr, uint32_t data);
	inline void increment_pwp();
	inline void increment_prp();
	inline void decrement_prp();
	inline void zero_prp();
	inline void set_alu_flagsd(uint32_t val);
	inline void set_alu_flagsf(double val);
	inline void set_alu_flagsi(int val);
	inline bool get_alu_second_src(uint8_t which);
	void handle_single_step_execution();
};


DECLARE_DEVICE_TYPE(MB86235, mb86235_device)

#endif // MAME_CPU_MB86235_MB86235_H
