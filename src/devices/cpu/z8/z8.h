// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

**********************************************************************/

#ifndef MAME_CPU_Z8_Z8_H
#define MAME_CPU_Z8_Z8_H

#pragma once


enum
{
	Z8_PC, Z8_SP, Z8_RP, Z8_T0, Z8_T1,

	Z8_R0, Z8_R1, Z8_R2, Z8_R3, Z8_R4, Z8_R5, Z8_R6, Z8_R7, Z8_R8, Z8_R9, Z8_R10, Z8_R11, Z8_R12, Z8_R13, Z8_R14, Z8_R15
};


class z8_device : public cpu_device
{
protected:
	// construction/destruction
	z8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int size);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 6; }
	virtual uint32_t execute_max_cycles() const override { return 20; }
	virtual uint32_t execute_input_lines() const override { return 4; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const override { return (cycles * 2); }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override
	{
		switch ( spacenum )
		{
			case AS_PROGRAM:   return &m_program_config;
			case AS_DATA:      return &m_data_config;
			case AS_IO:        return &m_io_config;
			default:           return nullptr;
		}
		return nullptr;
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 1; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 3; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;


private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	/* registers */
	uint16_t m_pc;              /* program counter */
	uint8_t m_r[256];           /* register file */
	uint8_t m_input[4];         /* port input latches */
	uint8_t m_output[4];        /* port output latches */
	uint8_t m_t0;               /* timer 0 current count */
	uint8_t m_t1;               /* timer 1 current count */

	/* fake registers */
	uint16_t m_fake_sp;         /* fake stack pointer */
	uint8_t m_fake_r[16];       /* fake working registers */

	/* interrupts */
	int m_irq[6];             /* interrupts */

	/* execution logic */
	int m_icount;             /* instruction counter */

	/* timers */
	emu_timer *m_t0_timer;
	emu_timer *m_t1_timer;

	TIMER_CALLBACK_MEMBER( t0_tick );
	TIMER_CALLBACK_MEMBER( t1_tick );

	inline uint8_t fetch();
	inline uint8_t register_read(uint8_t offset);
	inline uint16_t register_pair_read(uint8_t offset);
	inline void register_write(uint8_t offset, uint8_t data);
	inline void register_pair_write(uint8_t offset, uint16_t data);
	inline uint8_t get_working_register(int offset);
	inline uint8_t get_register(uint8_t offset);
	inline uint8_t get_intermediate_register(int offset);
	inline void stack_push_byte(uint8_t src);
	inline void stack_push_word(uint16_t src);
	inline uint8_t stack_pop_byte();
	inline uint16_t stack_pop_word();
	inline void set_flag(uint8_t flag, int state);
	inline void clear(uint8_t dst);
	inline void load(uint8_t dst, uint8_t src);
	inline void load_from_memory(address_space *space);
	inline void load_to_memory(address_space *space);
	inline void load_from_memory_autoinc(address_space *space);
	inline void load_to_memory_autoinc(address_space *space);
	inline void pop(uint8_t dst);
	inline void push(uint8_t src);
	inline void add_carry(uint8_t dst, int8_t src);
	inline void add(uint8_t dst, int8_t src);
	inline void compare(uint8_t dst, uint8_t src);
	inline void decimal_adjust(uint8_t dst);
	inline void decrement(uint8_t dst);
	inline void decrement_word(uint8_t dst);
	inline void increment(uint8_t dst);
	inline void increment_word(uint8_t dst);
	inline void subtract_carry(uint8_t dst, uint8_t src);
	inline void subtract(uint8_t dst, uint8_t src);
	inline void _and(uint8_t dst, uint8_t src);
	inline void complement(uint8_t dst);
	inline void _or(uint8_t dst, uint8_t src);
	inline void _xor(uint8_t dst, uint8_t src);
	inline void call(uint16_t dst);
	inline void jump(uint16_t dst);
	inline int check_condition_code(int cc);
	inline void test_complement_under_mask(uint8_t dst, uint8_t src);
	inline void test_under_mask(uint8_t dst, uint8_t src);
	inline void rotate_left(uint8_t dst);
	inline void rotate_left_carry(uint8_t dst);
	inline void rotate_right(uint8_t dst);
	inline void rotate_right_carry(uint8_t dst);
	inline void shift_right_arithmetic(uint8_t dst);
	inline void swap(uint8_t dst);

	#define INSTRUCTION(inst) void inst(uint8_t opcode, int *cycles);
	INSTRUCTION( illegal )
	INSTRUCTION( clr_R1 )
	INSTRUCTION( clr_IR1 )
	INSTRUCTION( ld_r1_IM )
	INSTRUCTION( ld_r1_R2 )
	INSTRUCTION( ld_r2_R1 )
	INSTRUCTION( ld_Ir1_r2 )
	INSTRUCTION( ld_R2_IR1 )
	INSTRUCTION( ld_r1_x_R2 )
	INSTRUCTION( ld_r2_x_R1 )
	INSTRUCTION( ld_r1_r2 )
	INSTRUCTION( ld_r1_Ir2 )
	INSTRUCTION( ld_R2_R1 )
	INSTRUCTION( ld_IR2_R1 )
	INSTRUCTION( ld_R1_IM )
	INSTRUCTION( ld_IR1_IM )
	INSTRUCTION( ldc_r1_Irr2 )
	INSTRUCTION( ldc_r2_Irr1 )
	INSTRUCTION( ldci_Ir1_Irr2 )
	INSTRUCTION( ldci_Ir2_Irr1 )
	INSTRUCTION( lde_r1_Irr2 )
	INSTRUCTION( lde_r2_Irr1 )
	INSTRUCTION( ldei_Ir1_Irr2 )
	INSTRUCTION( ldei_Ir2_Irr1 )
	INSTRUCTION( pop_R1 )
	INSTRUCTION( pop_IR1 )
	INSTRUCTION( push_R2 )
	INSTRUCTION( push_IR2 )
	INSTRUCTION( adc_r1_r2 )
	INSTRUCTION( adc_r1_Ir2 )
	INSTRUCTION( adc_R2_R1 )
	INSTRUCTION( adc_IR2_R1 )
	INSTRUCTION( adc_R1_IM )
	INSTRUCTION( adc_IR1_IM )
	INSTRUCTION( add_r1_r2 )
	INSTRUCTION( add_r1_Ir2 )
	INSTRUCTION( add_R2_R1 )
	INSTRUCTION( add_IR2_R1 )
	INSTRUCTION( add_R1_IM )
	INSTRUCTION( add_IR1_IM )
	INSTRUCTION( cp_r1_r2 )
	INSTRUCTION( cp_r1_Ir2 )
	INSTRUCTION( cp_R2_R1 )
	INSTRUCTION( cp_IR2_R1 )
	INSTRUCTION( cp_R1_IM )
	INSTRUCTION( cp_IR1_IM )
	INSTRUCTION( da_R1 )
	INSTRUCTION( da_IR1 )
	INSTRUCTION( dec_R1 )
	INSTRUCTION( dec_IR1 )
	INSTRUCTION( decw_RR1 )
	INSTRUCTION( decw_IR1 )
	INSTRUCTION( inc_r1 )
	INSTRUCTION( inc_R1 )
	INSTRUCTION( inc_IR1 )
	INSTRUCTION( incw_RR1 )
	INSTRUCTION( incw_IR1 )
	INSTRUCTION( sbc_r1_r2 )
	INSTRUCTION( sbc_r1_Ir2 )
	INSTRUCTION( sbc_R2_R1 )
	INSTRUCTION( sbc_IR2_R1 )
	INSTRUCTION( sbc_R1_IM )
	INSTRUCTION( sbc_IR1_IM )
	INSTRUCTION( sub_r1_r2 )
	INSTRUCTION( sub_r1_Ir2 )
	INSTRUCTION( sub_R2_R1 )
	INSTRUCTION( sub_IR2_R1 )
	INSTRUCTION( sub_R1_IM )
	INSTRUCTION( sub_IR1_IM )
	INSTRUCTION( and_r1_r2 )
	INSTRUCTION( and_r1_Ir2 )
	INSTRUCTION( and_R2_R1 )
	INSTRUCTION( and_IR2_R1 )
	INSTRUCTION( and_R1_IM )
	INSTRUCTION( and_IR1_IM )
	INSTRUCTION( com_R1 )
	INSTRUCTION( com_IR1 )
	INSTRUCTION( or_r1_r2 )
	INSTRUCTION( or_r1_Ir2 )
	INSTRUCTION( or_R2_R1 )
	INSTRUCTION( or_IR2_R1 )
	INSTRUCTION( or_R1_IM )
	INSTRUCTION( or_IR1_IM )
	INSTRUCTION( xor_r1_r2 )
	INSTRUCTION( xor_r1_Ir2 )
	INSTRUCTION( xor_R2_R1 )
	INSTRUCTION( xor_IR2_R1 )
	INSTRUCTION( xor_R1_IM )
	INSTRUCTION( xor_IR1_IM )
	INSTRUCTION( call_IRR1 )
	INSTRUCTION( call_DA )
	INSTRUCTION( djnz_r1_RA )
	INSTRUCTION( iret )
	INSTRUCTION( ret )
	INSTRUCTION( jp_IRR1 )
	INSTRUCTION( jp_cc_DA )
	INSTRUCTION( jr_cc_RA )
	INSTRUCTION( tcm_r1_r2 )
	INSTRUCTION( tcm_r1_Ir2 )
	INSTRUCTION( tcm_R2_R1 )
	INSTRUCTION( tcm_IR2_R1 )
	INSTRUCTION( tcm_R1_IM )
	INSTRUCTION( tcm_IR1_IM )
	INSTRUCTION( tm_r1_r2 )
	INSTRUCTION( tm_r1_Ir2 )
	INSTRUCTION( tm_R2_R1 )
	INSTRUCTION( tm_IR2_R1 )
	INSTRUCTION( tm_R1_IM )
	INSTRUCTION( tm_IR1_IM )
	INSTRUCTION( rl_R1 )
	INSTRUCTION( rl_IR1 )
	INSTRUCTION( rlc_R1 )
	INSTRUCTION( rlc_IR1 )
	INSTRUCTION( rr_R1 )
	INSTRUCTION( rr_IR1 )
	INSTRUCTION( rrc_R1 )
	INSTRUCTION( rrc_IR1 )
	INSTRUCTION( sra_R1 )
	INSTRUCTION( sra_IR1 )
	INSTRUCTION( swap_R1 )
	INSTRUCTION( swap_IR1 )
	INSTRUCTION( ccf )
	INSTRUCTION( di )
	INSTRUCTION( ei )
	INSTRUCTION( nop )
	INSTRUCTION( rcf )
	INSTRUCTION( scf )
	INSTRUCTION( srp_IM )
	#undef INSTRUCTION

	typedef void (z8_device::*z8_opcode_func) (uint8_t opcode, int *cycles);
	struct z8_opcode_map
	{
		z8_opcode_func  function;
		int             execution_cycles;
		int             pipeline_cycles;
	};
	static const z8_opcode_map Z8601_OPCODE_MAP[256];

};


class z8601_device : public z8_device
{
public:
	z8601_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class ub8830d_device : public z8_device
{
public:
	ub8830d_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class z8611_device : public z8_device
{
public:
	z8611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// Zilog Z8601
DECLARE_DEVICE_TYPE(Z8601, z8601_device)

// VEB Mikroelektronik Erfurt UB8830D MME
DECLARE_DEVICE_TYPE(UB8830D, ub8830d_device)

// Zilog Z8611
DECLARE_DEVICE_TYPE(Z8611, z8611_device)

#endif // MAME_CPU_Z8_Z8_H
