// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    dspp.h

    Core implementation for the portable DSPP emulator.

***************************************************************************/

#ifndef MAME_CPU_DSPP_DSPP_H
#define MAME_CPU_DSPP_DSPP_H

#pragma once

#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dspp_frontend;

// ======================> dspp_device

class dspp_device : public cpu_device
{
	friend class dspp_frontend;
public:
	// Construction/destruction
	dspp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor code_map_ctor,
		address_map_constructor data_map_ctor);
	dspp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Static configuration helpers
	auto int_handler() { return m_int_handler.bind(); }
	auto dma_read_handler() { return m_dma_read_handler.bind(); }
	auto dma_write_handler() { return m_dma_write_handler.bind(); }

	// Public interfaces
	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data);

	uint16_t read_output_fifo();

	void dump_state(); // TODO: DEBUG REMOVE ME

	// Internal registers
	uint16_t input_r();
	void output_w(offs_t offset, uint16_t data);
	uint16_t fifo_osc_r(offs_t offset);
	void fifo_osc_w(offs_t offset, uint16_t data);
	void input_control_w(uint16_t data);
	void output_control_w(uint16_t data);
	uint16_t input_status_r();
	uint16_t output_status_r();
	void cpu_int_w(uint16_t data);
	uint16_t pc_r();
	void pc_w(uint16_t data);
	uint16_t audlock_r();
	void audlock_w(uint16_t data);
	uint16_t clock_r();
	void clock_w(uint16_t data);
	uint16_t noise_r();

	void update_fifo_dma();
	void print_sums() { printf("%04x: %04x\n", (uint16_t)m_core->m_arg0, (uint16_t)m_core->m_arg1); }
	void print_branches() { printf("Branch: %d %d %d %d %d\n", m_core->m_arg0 ? 1 : 0, m_core->m_arg1 ? 1 : 0, m_core->m_arg2 ? 1 : 0, m_core->m_arg3 ? 1 : 0, m_core->m_arg4 ? 1 : 0); }
	void print_value() { printf("Value is %08x\n", m_core->m_arg0); }
	void print_addr() { printf("New value is %08x from %08x\n", m_core->m_arg0, m_core->m_arg1); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override;
	virtual uint32_t execute_max_cycles() const noexcept override;
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void code_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	enum
	{
		DSPX_CONTROL_GWILLING             = 0x0001,
		DSPX_CONTROL_STEP_CYCLE           = 0x0002,
		DSPX_CONTROL_STEP_PC              = 0x0004,
		DSPX_CONTROL_SNOOP                = 0x0008,

		DSPX_RESET_DSPP                   = 0x0001,
		DSPX_RESET_INPUT                  = 0x0002,
		DSPX_RESET_OUTPUT                 = 0x0004,

		DSPX_F_DMA_NEXTVALID              = 0x0001,
		DSPX_F_DMA_GO_FOREVER             = 0x0002,
		DSPX_F_INT_DMANEXT_EN             = 0x0004,
		DSPX_F_SHADOW_SET_DMANEXT         = 0x00040000,
		DSPX_F_SHADOW_SET_FOREVER         = 0x00020000,
		DSPX_F_SHADOW_SET_NEXTVALID       = 0x00010000,
		DSPX_F_SHADOW_SET_ADDRESS_COUNT   = 0x80000000,

		DSPX_F_INT_TIMER                  = 0x00000100,
		DSPX_F_INT_INPUT_UNDER            = 0x00000080,
		DSPX_F_INT_INPUT_OVER             = 0x00000040,
		DSPX_F_INT_OUTPUT_UNDER           = 0x00000020,
		DSPX_F_INT_OUTPUT_OVER            = 0x00000010,
		DSPX_F_INT_UNDEROVER              = 0x00000008,
		DSPX_F_INT_CONSUMED               = 0x00000002,
		DSPX_F_INT_DMANEXT                = 0x00000001,

		DSPX_F_INT_ALL_DMA                = (DSPX_F_INT_DMANEXT | DSPX_F_INT_CONSUMED | DSPX_F_INT_UNDEROVER),

		DSPX_FLD_INT_SOFT_WIDTH           = 16,         /* width of the field and the number of interrupts */
		DSPX_FLD_INT_SOFT_SHIFT           = 16,
		DSPX_FLD_INT_SOFT_MASK            = 0xffff0000
	};

private:
	// Constants
	static const uint32_t PC_STACK_DEPTH = 4;
	static const uint32_t MAX_OPERANDS = 8;

	static const uint32_t NUM_DMA_CHANNELS = 32;
	static const uint32_t DMA_FIFO_DEPTH = 8;
	static const uint32_t DMA_FIFO_MASK = DMA_FIFO_DEPTH - 1;

	static const uint32_t NUM_INPUTS = 2;
	static const uint32_t NUM_OUTPUTS = 8;
	static const uint32_t OUTPUT_FIFO_DEPTH = 8;
	static const uint32_t OUTPUT_FIFO_MASK = OUTPUT_FIFO_DEPTH - 1;

	// Handlers
	devcb_write_line    m_int_handler;
	devcb_read8         m_dma_read_handler;
	devcb_write8        m_dma_write_handler;

	// Internal functions
	uint16_t read_op(offs_t pc);
	inline uint16_t read_data(offs_t addr);
	inline void write_data(offs_t addr, uint16_t data);

	inline void update_pc();
	inline void update_ticks();
	inline void exec_control();
	inline void exec_super_special();
	inline void exec_special();
	inline void exec_branch();
	inline void exec_complex_branch();
	inline void exec_arithmetic();
	void parse_operands(uint32_t numops);
	uint16_t read_next_operand();
	void write_next_operand(uint16_t value);
	inline void push_pc();
	inline uint16_t pop_pc();
	inline void set_rbase(uint32_t base, uint32_t addr);
	inline uint16_t translate_reg(uint16_t reg);

	void process_next_dma(int32_t channel);
	void service_input_dma(int32_t channel);
	void service_output_dma(int32_t channel);
	int16_t read_fifo_to_dspp(int32_t channel);
	int16_t read_fifo_to_dma(int32_t channel);
	void write_dma_to_fifo(int32_t channel, int16_t value);
	void write_dspp_to_fifo(int32_t channel, int16_t value);

	void run_oscillator(int32_t channel);
	void reset_channel(int32_t channel);
	void advance_audio_timer();
	void advance_audio_frame();
	int16_t decode_sqxd(int8_t val, int16_t prev);

	uint32_t get_interrupt_state();
	void update_host_interrupt();

	uint32_t read_dma_stack(offs_t offset);
	void write_dma_stack(offs_t offset, uint32_t data);

	uint32_t read_ext_control(offs_t offset);
	void write_ext_control(offs_t offset, uint32_t data);

	bool m_isdrc;

	// Address spaces
	const address_space_config  m_code_config;
	const address_space_config  m_data_config;
	memory_access<10, 1, -1, ENDIANNESS_BIG>::cache m_code_cache;
	memory_access<10, 1, -1, ENDIANNESS_BIG>::specific m_code;
	memory_access<10, 1, -1, ENDIANNESS_BIG>::specific m_data;

	struct dspp_internal_state
	{
		// Internal state
		int         m_icount;
		uint16_t    m_pc;
		uint16_t    m_stack[PC_STACK_DEPTH];
		uint32_t    m_stack_ptr;
		uint16_t    m_rbase[4];
		uint32_t    m_acc;
		uint32_t    m_tclock;

		uint32_t    m_flag_carry;
		uint32_t    m_flag_zero;
		uint32_t    m_flag_neg;
		uint32_t    m_flag_over;
		uint32_t    m_flag_exact;
		uint32_t    m_flag_audlock;
		uint32_t    m_flag_sleep;

		uint32_t    m_partial_int;
		uint16_t    m_op;
		uint32_t    m_opidx;
		int32_t     m_writeback;
		uint32_t    m_jmpdest;

		const char *m_format;
		uint32_t    m_arg0;
		uint32_t    m_arg1;
		uint32_t    m_arg2;
		uint32_t    m_arg3;
		uint32_t    m_arg4;

		struct
		{
			uint32_t value;
			uint32_t addr;
		} m_operands[MAX_OPERANDS];

		// External control registers
		uint32_t    m_dspx_control;
	} * m_core;

	// DMA
	struct fifo_dma
	{
		uint32_t    m_current_addr;
		int32_t     m_current_count;
		uint32_t    m_next_addr;
		uint32_t    m_next_count;
		uint32_t    m_prev_value;
		uint32_t    m_prev_current;
		uint8_t     m_go_forever;
		uint8_t     m_next_valid;
		uint8_t     m_reserved;
		uint16_t    m_fifo[DMA_FIFO_DEPTH];
		uint32_t    m_dma_ptr;
		uint32_t    m_dspi_ptr;
		uint32_t    m_depth;
	} m_fifo_dma[NUM_DMA_CHANNELS];

	// Oscillator
	uint32_t    m_last_frame_clock;
	uint32_t    m_last_osc_count;
	uint32_t    m_osc_phase;
	uint32_t    m_osc_freq;

	// Output FIFO
	uint16_t    m_outputs[NUM_OUTPUTS];
	uint16_t    m_output_fifo[OUTPUT_FIFO_DEPTH];
	uint32_t    m_output_fifo_start;
	uint32_t    m_output_fifo_count;

	// External control registers
	uint32_t    m_dspx_reset;
	uint32_t    m_dspx_int_enable;
	uint32_t    m_dspx_channel_enable;
	uint32_t    m_dspx_channel_complete;
	uint32_t    m_dspx_channel_direction;
	uint32_t    m_dspx_channel_8bit;
	uint32_t    m_dspx_channel_sqxd;
	uint32_t    m_dspx_shadow_current_addr;
	uint32_t    m_dspx_shadow_current_count;
	uint32_t    m_dspx_shadow_next_addr;
	uint32_t    m_dspx_shadow_next_count;
	uint32_t    m_dspx_dmanext_int;
	uint32_t    m_dspx_dmanext_enable;
	uint32_t    m_dspx_consumed_int;
	uint32_t    m_dspx_consumed_enable;
	uint32_t    m_dspx_underover_int;
	uint32_t    m_dspx_underover_enable;
	uint32_t    m_dspx_audio_time;
	uint16_t    m_dspx_audio_duration;

	//
	// DRC
	//

	// Core state
	/* internal stuff */
	bool        m_cache_dirty;
	drc_cache   m_cache;
	std::unique_ptr<drcuml_state>   m_drcuml;
	std::unique_ptr<dspp_frontend>  m_drcfe;
	uint32_t    m_drcoptions;

	/* internal compiler state */
	struct compiler_state
	{
		uint32_t         cycles;                     /* accumulated cycles */
		uint8_t          checkints;                  /* need to check interrupts before next instruction */
		uint8_t          checksoftints;              /* need to check software interrupts before next instruction */
		uml::code_label  abortlabel;                 /* label to abort execution of this block */
		uml::code_label  labelnum;                   /* index for local labels */
	};

public: // TODO
	void alloc_handle(drcuml_state *drcuml, uml::code_handle **handleptr, const char *name);
	void load_fast_iregs(drcuml_block &block);
	void save_fast_iregs(drcuml_block &block);
//  void arm7_drc_init();
//  void arm7_drc_exit();
	void execute_run_drc();
//  void arm7drc_set_options(uint32_t options);
//  void arm7drc_add_fastram(offs_t start, offs_t end, uint8_t readonly, void *base);
//  void arm7drc_add_hotspot(offs_t pc, uint32_t opcode, uint32_t cycles);
	void flush_cache();
	void compile_block(offs_t pc);
	void cfunc_get_cycles();
	void cfunc_unimplemented();
	void static_generate_entry_point();
	void static_generate_nocode_handler();
	void static_generate_out_of_cycles();
	void static_generate_memory_accessor(bool iswrite, const char *name, uml::code_handle *&handleptr);
	void generate_update_cycles(drcuml_block &block, compiler_state *compiler, uml::parameter param);
	void generate_checksum_block(drcuml_block &block, compiler_state *compiler, const opcode_desc *seqhead, const opcode_desc *seqlast);
	void generate_sequence_instruction(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);

	void generate_push_pc(drcuml_block &block);
	void generate_read_next_operand(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_write_next_operand(drcuml_block &block, compiler_state *compiler);
	void generate_update_pc(drcuml_block &block);
	void generate_update_ticks(drcuml_block &block);
	void generate_translate_reg(drcuml_block &block, uint16_t reg);
	void generate_set_rbase(drcuml_block &block, compiler_state *compiler, uint32_t base, uint32_t addr);
	void generate_branch(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_branch_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_complex_branch_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_super_special(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_special_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_arithmetic_opcode(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc);
	void generate_parse_operands(drcuml_block &block, compiler_state *compiler, const opcode_desc *desc, uint32_t numops);

	/* subroutines */
	uml::code_handle *m_entry;                      /* entry point */
	uml::code_handle *m_nocode;                     /* nocode exception handler */
	uml::code_handle *m_out_of_cycles;              /* out of cycles exception handler */

	enum
	{
		MEM_ACCESSOR_PM_READ48,
		MEM_ACCESSOR_PM_WRITE48,
		MEM_ACCESSOR_PM_READ32,
		MEM_ACCESSOR_PM_WRITE32,
		MEM_ACCESSOR_DM_READ32,
		MEM_ACCESSOR_DM_WRITE32
	};

	uml::code_handle *m_dm_read16;
	uml::code_handle *m_dm_write16;
};


/***************************************************************************
 COMPILER-SPECIFIC OPTIONS
 ***************************************************************************/

#define DSPPDRC_STRICT_VERIFY      0x0001          /* verify all instructions */
#define DSPPDRC_FLUSH_PC           0x0002          /* flush the PC value before each memory access */

#define DSPPDRC_COMPATIBLE_OPTIONS (DSPPDRC_STRICT_VERIFY | DSPPDRC_FLUSH_PC)
#define DSPPDRC_FASTEST_OPTIONS    (0)

// device type definition
DECLARE_DEVICE_TYPE(DSPP, dspp_device);


#endif // MAME_CPU_DSPP_DSPP_H
