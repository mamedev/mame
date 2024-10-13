// license:BSD-3-Clause
// copyright-holders:Curt Coder, AJR
/**********************************************************************

    Zilog Z8 Single-Chip MCU emulation

**********************************************************************/

#ifndef MAME_CPU_Z8_Z8_H
#define MAME_CPU_Z8_Z8_H

#pragma once


class z8_device : public cpu_device
{
public:
	// configuration
	auto p0_in_cb() { return m_input_cb[0].bind(); }
	auto p1_in_cb() { return m_input_cb[1].bind(); }
	auto p2_in_cb() { return m_input_cb[2].bind(); }
	auto p3_in_cb() { return m_input_cb[3].bind(); }
	auto p0_out_cb() { return m_output_cb[0].bind(); }
	auto p1_out_cb() { return m_output_cb[1].bind(); }
	auto p2_out_cb() { return m_output_cb[2].bind(); }
	auto p3_out_cb() { return m_output_cb[3].bind(); }

protected:
	enum
	{
		Z8_PC, Z8_SP, Z8_RP,
		Z8_IMR, Z8_IRQ, Z8_IPR,
		Z8_P0, Z8_P1, Z8_P2, Z8_P3,
		Z8_P01M, Z8_P3M, Z8_P2M,
		Z8_PRE0, Z8_T0, Z8_PRE1, Z8_T1, Z8_TMR, Z8_TOUT,

		Z8_R0, Z8_R1, Z8_R2, Z8_R3, Z8_R4, Z8_R5, Z8_R6, Z8_R7, Z8_R8, Z8_R9, Z8_R10, Z8_R11, Z8_R12, Z8_R13, Z8_R14, Z8_R15,
		Z8_RR0, Z8_RR2, Z8_RR4, Z8_RR6, Z8_RR8, Z8_RR10, Z8_RR12, Z8_RR14
	};

	// construction/destruction
	z8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t rom_size, bool preprogrammed);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 6; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 27; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return true; }
	virtual uint64_t execute_clocks_to_cycles(uint64_t clocks) const noexcept override { return (clocks + 2 - 1) / 2; }
	virtual uint64_t execute_cycles_to_clocks(uint64_t cycles) const noexcept override { return (cycles * 2); }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void program_map(address_map &map) ATTR_COLD;
	void preprogrammed_map(address_map &map) ATTR_COLD;
	void register_map(address_map &map) ATTR_COLD;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_register_config;

	memory_access<16, 0, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_program;
	memory_access<16, 0, 0, ENDIANNESS_BIG>::specific m_data;
	memory_access< 8, 0, 0, ENDIANNESS_BIG>::specific m_regs;

	// callbacks
	devcb_read8::array<4> m_input_cb;
	devcb_write8::array<4> m_output_cb;

	uint32_t m_rom_size;

	// basic registers
	uint16_t m_pc;              // program counter
	uint16_t m_ppc;             // program counter at last opcode fetch
	PAIR16 m_sp;                // stack pointer (8-bit for internal stack, 16-bit for external stack)
	uint8_t m_rp;               // register pointer
	uint8_t m_flags;            // condition flags
	uint8_t m_imr;              // interrupt mask
	uint8_t m_irq;              // interrupt request
	uint8_t m_ipr;              // interrupt priority

	// port registers
	uint8_t m_input[4];         // port input latches
	uint8_t m_output[4];        // port output latches
	uint8_t m_p01m;             // port 0/1 mode
	uint8_t m_p2m;              // port 2 mode
	uint8_t m_p3m;              // port 3 mode
	uint8_t m_p3_output;        // port 3 output (alternate functions included)

	// timer registers
	uint8_t m_tmr;              // timer mode
	uint8_t m_t[2];             // initial values
	uint8_t m_count[2];         // current counts
	uint8_t m_pre[2];           // prescalers
	uint8_t m_pre_count[2];     // prescaler counts
	bool m_tout;                // toggle output

	// serial transmitter registers
	uint16_t m_transmit_sr;     // transmitter shift register
	uint8_t m_transmit_count;   // counter for transmitter timing
	bool m_transmit_parity;     // transmitter parity calculation

	// serial receiver registers
	uint8_t m_receive_buffer;   // received character
	uint16_t m_receive_sr;      // receiver shift register
	uint8_t m_receive_count;    // counter for receiver timing
	bool m_receive_parity;      // receiver parity calculation
	bool m_receive_started;     // true if receiver has seen start bit

	// interrupts
	int m_irq_line[4];
	bool m_irq_taken;
	bool m_irq_initialized;     // IRQ must be unlocked by EI after reset

	// execution logic
	int32_t m_icount;           // instruction counter

	// timers
	emu_timer *m_internal_timer[2];

	bool get_serial_in();
	void sio_receive();
	void sio_transmit();

	template <int T> void timer_start();
	template <int T> void timer_stop();
	template <int T> void timer_end();
	void t1_trigger();
	void tout_init();
	void tout_toggle();

	template <int T> TIMER_CALLBACK_MEMBER(timeout);

	void request_interrupt(int irq);
	void take_interrupt(int irq);
	void process_interrupts();

	uint8_t p0_read();
	void p0_write(uint8_t data);
	uint8_t p1_read();
	void p1_write(uint8_t data);
	uint8_t p2_read();
	void p2_write(uint8_t data);
	uint8_t p3_read();
	void p3_write(uint8_t data);
	void p3_update_output();
	uint8_t sio_read();
	void sio_write(uint8_t data);
	uint8_t tmr_read();
	void tmr_write(uint8_t data);
	uint8_t t0_read();
	void t0_write(uint8_t data);
	uint8_t t1_read();
	void t1_write(uint8_t data);
	void pre0_write(uint8_t data);
	void pre1_write(uint8_t data);
	void p01m_write(uint8_t data);
	void p2m_write(uint8_t data);
	void p3m_write(uint8_t data);
	void ipr_write(uint8_t data);
	uint8_t irq_read();
	void irq_write(uint8_t data);
	uint8_t imr_read();
	void imr_write(uint8_t data);
	uint8_t flags_read();
	void flags_write(uint8_t data);
	uint8_t rp_read();
	void rp_write(uint8_t data);
	uint8_t sph_read();
	void sph_write(uint8_t data);
	uint8_t spl_read();
	void spl_write(uint8_t data);

	inline uint16_t mask_external_address(uint16_t addr);
	inline uint8_t fetch();
	inline uint8_t fetch_opcode();
	inline uint16_t fetch_word();
	inline uint8_t register_read(uint8_t offset) { return m_regs.read_byte(offset); }
	inline uint16_t register_pair_read(uint8_t offset);
	inline void register_write(uint8_t offset, uint8_t data) { m_regs.write_byte(offset, data); }
	inline void register_pair_write(uint8_t offset, uint16_t data);
	inline uint8_t get_working_register(int offset) const;
	inline uint8_t get_register(uint8_t offset) const;
	inline uint8_t get_intermediate_register(int offset);
	inline void stack_push_byte(uint8_t src);
	inline void stack_push_word(uint16_t src);
	inline uint8_t stack_pop_byte();
	inline uint16_t stack_pop_word();
	inline void set_flag(uint8_t flag, int state);
	inline void clear(uint8_t dst);
	inline void load(uint8_t dst, uint8_t src);
	inline void load_from_memory(memory_access<16, 0, 0, ENDIANNESS_BIG>::specific &space);
	inline void load_to_memory(memory_access<16, 0, 0, ENDIANNESS_BIG>::specific &space);
	inline void load_from_memory_autoinc(memory_access<16, 0, 0, ENDIANNESS_BIG>::specific &space);
	inline void load_to_memory_autoinc(memory_access<16, 0, 0, ENDIANNESS_BIG>::specific &space);
	inline void pop(uint8_t dst);
	inline void push(uint8_t src);
	inline void add_carry(uint8_t dst, uint8_t src);
	inline void add(uint8_t dst, uint8_t src);
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
	inline bool check_condition_code(int cc);
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


class z8671_device : public z8_device
{
public:
	z8671_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	const tiny_rom_entry *device_rom_region() const override;
};


class z8681_device : public z8_device
{
public:
	z8681_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


class z8682_device : public z8_device
{
public:
	z8682_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	const tiny_rom_entry *device_rom_region() const override;
};


class z86e02_device : public z8_device
{
public:
	z86e02_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// Zilog Z8601
DECLARE_DEVICE_TYPE(Z8601, z8601_device)

// VEB Mikroelektronik Erfurt UB8830D MME
DECLARE_DEVICE_TYPE(UB8830D, ub8830d_device)

// Zilog Z8611
DECLARE_DEVICE_TYPE(Z8611, z8611_device)

// Zilog Z8671 BASIC/DEBUG interpreter
DECLARE_DEVICE_TYPE(Z8671, z8671_device)

// Zilog Z8681 ROMless
DECLARE_DEVICE_TYPE(Z8681, z8681_device)

// Zilog Z8682 ROMless (boot to 0812H)
DECLARE_DEVICE_TYPE(Z8682, z8682_device)

// Zilog Z86E02
DECLARE_DEVICE_TYPE(Z86E02, z86e02_device)

#endif // MAME_CPU_Z8_Z8_H
