// license:BSD-3-Clause
// copyright-holders:Peter Trauner,Antoine Mine
/*****************************************************************************
 *
 *   cpustate->h
 *   portable saturn emulator interface
 *   (hp calculators)
 *
 *
 *****************************************************************************/
/*
Calculator        Release Date          Chip Version     Analog/Digital IC
HP71B (early)     02/01/84              1LF2              -
HP71B (later)     ??/??/??              1LK7              -
HP18C             06/01/86              1LK7              -
HP28C             01/05/87              1LK7              -
HP17B             01/04/88              1LT8             Lewis
HP19B             01/04/88              1LT8             Lewis
HP27S             01/04/88              1LT8             Lewis
HP28S             01/04/88              1LT8             Lewis
HP48SX            03/16/91              1LT8             Clarke
HP48S             04/02/91              1LT8             Clarke
HP48GX            06/01/93              1LT8             Yorke
HP48G             06/01/93              1LT8             Yorke
HP38G             09/??/95              1LT8             Yorke
*/
/* 4 bit processor
   20 address lines */

#pragma once

#ifndef __SATURN_H__
#define _SATURN_H


#define SATURN_INT_NONE 0
#define SATURN_INT_IRQ  1
#define SATURN_INT_NMI  2


enum
{
	SATURN_A=1, SATURN_B, SATURN_C, SATURN_D,
	SATURN_R0, SATURN_R1, SATURN_R2, SATURN_R3, SATURN_R4,
	SATURN_RSTK0, SATURN_RSTK1, SATURN_RSTK2, SATURN_RSTK3,
	SATURN_RSTK4, SATURN_RSTK5, SATURN_RSTK6, SATURN_RSTK7,
	SATURN_PC, SATURN_D0, SATURN_D1,

	SATURN_P,
	SATURN_OUT,
	SATURN_CARRY,
	SATURN_ST,
	SATURN_HST,

	SATURN_IRQ_STATE,
	SATURN_SLEEPING
};

#define SATURN_IRQ_LINE 0
#define SATURN_NMI_LINE 1
#define SATURN_WAKEUP_LINE 2


#define MCFG_SATURN_CONFIG(_out, _in, _reset, _config, _unconfig, _id, _crc, _rsi) \
	saturn_device::set_out_func(*device, DEVCB_##_out); \
	saturn_device::set_in_func(*device, DEVCB_##_in); \
	saturn_device::set_reset_func(*device, DEVCB_##_reset); \
	saturn_device::set_config_func(*device, DEVCB_##_config); \
	saturn_device::set_unconfig_func(*device, DEVCB_##_unconfig); \
	saturn_device::set_id_func(*device, DEVCB_##_id); \
	saturn_device::set_crc_func(*device, DEVCB_##_crc); \
	saturn_device::set_rsi_func(*device, DEVCB_##_rsi);


class saturn_device : public cpu_device
{
public:
	// construction/destruction
	saturn_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_out_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_out_func.set_callback(object); }
	template<class _Object> static devcb_base &set_in_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_in_func.set_callback(object); }
	template<class _Object> static devcb_base &set_reset_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_reset_func.set_callback(object); }
	template<class _Object> static devcb_base &set_config_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_config_func.set_callback(object); }
	template<class _Object> static devcb_base &set_unconfig_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_unconfig_func.set_callback(object); }
	template<class _Object> static devcb_base &set_id_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_id_func.set_callback(object); }
	template<class _Object> static devcb_base &set_crc_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_crc_func.set_callback(object); }
	template<class _Object> static devcb_base &set_rsi_func(device_t &device, _Object object) { return downcast<saturn_device &>(device).m_rsi_func.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 2; }
	virtual UINT32 execute_max_cycles() const override { return 21; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 20; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;

	devcb_write32     m_out_func;
	devcb_read32      m_in_func;
	devcb_write_line  m_reset_func;
	devcb_write32     m_config_func;
	devcb_write32     m_unconfig_func;
	devcb_read32      m_id_func;
	devcb_write32     m_crc_func;
	devcb_write_line  m_rsi_func;

// 64 bit, unpacked (one nibble per byte)
typedef UINT8 Saturn64[16];

	Saturn64 m_reg[9]; //r0,r1,r2,r3,r4,a,b,c,d

	UINT32 m_d[2], m_pc, m_oldpc, m_rstk[8]; // 20 bit, packed addresses

	UINT8 m_p; // 4 bit pointer

		UINT16 m_out; // 12 bit (packed)
	UINT8  m_carry, m_decimal;
	UINT16 m_st; // status 16 bit

	UINT8 m_hst; // hardware status 4 bit

	UINT8   m_nmi_state;
	UINT8   m_irq_state;
	UINT8   m_irq_enable;     /* INTON / INTOFF */
	UINT8   m_in_irq;         /* already servicing IRQ */
	UINT8   m_pending_irq;    /* IRQ is pending */
	UINT8   m_sleeping;       /* low-consumption state */
	int     m_monitor_id;
	int     m_monitor_in;
	address_space *m_program;
	direct_read_data *m_direct;
	int m_icount;
	INT64 m_debugger_temp;

	void saturn_take_irq();
	void IntReg64(Saturn64 r, INT64 d);
	INT64 Reg64Int(Saturn64 r);

	inline int READ_OP();
	inline int READ_OP_ARG();
	inline int READ_OP_ARG8();
	inline INT8 READ_OP_DIS8();
	inline int READ_OP_ARG12();
	inline int READ_OP_DIS12();
	inline int READ_OP_ARG16();
	inline INT16 READ_OP_DIS16();
	inline int READ_OP_ARG20();
	inline int READ_NIBBLE(UINT32 adr);
	inline int READ_8(UINT32 adr);
	inline int READ_12(UINT32 adr);
	inline int READ_16(UINT32 adr);
	inline int READ_20(UINT32 adr);
	inline void WRITE_NIBBLE(UINT32 adr, UINT8 nib);
	inline int S64_READ_X(int r);
	inline int S64_READ_WORD(int r);
	inline int S64_READ_A(int r);
	inline void S64_WRITE_X(int r, int v);
	inline void S64_WRITE_WORD(int r, int v);
	inline void S64_WRITE_A(int r, int v);
	inline UINT32 saturn_pop();
	inline void saturn_push(UINT32 adr);
	inline void saturn_interrupt_on();
	inline void saturn_interrupt_off();
	inline void saturn_reset_interrupt();
	inline void saturn_mem_reset();
	inline void saturn_mem_config();
	inline void saturn_mem_unconfig();
	inline void saturn_mem_id();
	inline void saturn_shutdown();
	inline void saturn_bus_command_b();
	inline void saturn_bus_command_c();
	inline void saturn_bus_command_d();
	inline void saturn_serial_request();
	inline void saturn_out_c();
	inline void saturn_out_cs();
	inline void saturn_in(int reg);
	inline void saturn_sethex() { m_decimal=0; }
	inline void saturn_setdec() { m_decimal=1; }
	inline void saturn_clear_st();
	inline void saturn_st_to_c();
	inline void saturn_c_to_st();
	inline void saturn_exchange_c_st();
	inline void saturn_jump_after_test();
	inline void saturn_st_clear_bit();
	inline void saturn_st_set_bit();
	inline void saturn_st_jump_bit_clear();
	inline void saturn_st_jump_bit_set();
	inline void saturn_hst_clear_bits();
	inline void saturn_hst_bits_cleared();
	inline void saturn_exchange_p();
	inline void saturn_p_to_c();
	inline void saturn_c_to_p();
	inline void saturn_dec_p();
	inline void saturn_inc_p();
	inline void saturn_load_p();
	inline void saturn_p_equals();
	inline void saturn_p_not_equals();
	inline void saturn_ca_p_1();
	inline void saturn_load_reg(int reg);
	inline void saturn_jump(int adr, int jump);
	inline void saturn_call(int adr);
	inline void saturn_return(int yes);
	inline void saturn_return_carry_set();
	inline void saturn_return_carry_clear();
	inline void saturn_return_interrupt();
	inline void saturn_return_xm_set();
	inline void saturn_pop_c();
	inline void saturn_push_c();
	inline void saturn_indirect_jump(int reg);
	inline void saturn_equals_zero(int reg, int begin, int count);
	inline void saturn_equals(int reg, int begin, int count, int right);
	inline void saturn_not_equals_zero(int reg, int begin, int count);
	inline void saturn_not_equals(int reg, int begin, int count, int right);
	inline void saturn_greater(int reg, int begin, int count, int right);
	inline void saturn_greater_equals(int reg, int begin, int count, int right);
	inline void saturn_smaller_equals(int reg, int begin, int count, int right);
	inline void saturn_smaller(int reg, int begin, int count, int right);
	inline void saturn_jump_bit_clear(int reg);
	inline void saturn_jump_bit_set(int reg);
	inline void saturn_load_pc(int reg);
	inline void saturn_store_pc(int reg);
	inline void saturn_exchange_pc(int reg);
	inline void saturn_load_adr(int reg, int nibbles);
	inline void saturn_add_adr(int reg);
	inline void saturn_sub_adr(int reg);
	inline void saturn_adr_to_reg(int adr, int reg);
	inline void saturn_reg_to_adr(int reg, int adr);
	inline void saturn_adr_to_reg_word(int adr, int reg);
	inline void saturn_reg_to_adr_word(int reg, int adr);
	inline void saturn_exchange_adr_reg(int adr, int reg);
	inline void saturn_exchange_adr_reg_word(int adr, int reg);
	inline void saturn_load_nibbles(int reg, int begin, int count, int adr);
	inline void saturn_store_nibbles(int reg, int begin, int count, int adr);
	inline void saturn_clear_bit(int reg);
	inline void saturn_set_bit(int reg);
	inline void saturn_clear(int reg, int begin, int count);
	inline void saturn_exchange(int left, int begin, int count, int right);
	inline void saturn_copy(int dest, int begin, int count, int src);
	inline void saturn_add(int reg, int begin, int count, int right);
	inline void saturn_add_const(int reg, int begin, int count, UINT8 right);
	inline void saturn_sub(int reg, int begin, int count, int right);
	inline void saturn_sub_const(int reg, int begin, int count, int right);
	inline void saturn_sub2(int reg, int begin, int count, int right);
	inline void saturn_increment(int reg, int begin, int count);
	inline void saturn_decrement(int reg, int begin, int count);
	inline void saturn_invert(int reg, int begin, int count);
	inline void saturn_negate(int reg, int begin, int count);
	inline void saturn_or(int dest, int begin, int count, int src);
	inline void saturn_and(int dest, int begin, int count, int src);
	inline void saturn_shift_nibble_left(int reg, int begin, int count);
	inline void saturn_shift_nibble_right(int reg, int begin, int count);
	inline void saturn_rotate_nibble_left_w(int reg);
	inline void saturn_rotate_nibble_right_w(int reg);
	inline void saturn_shift_right(int reg, int begin, int count);
	void saturn_invalid3( int op1, int op2, int op3 );
	void saturn_invalid4( int op1, int op2, int op3, int op4 );
	void saturn_invalid5( int op1, int op2, int op3, int op4, int op5 );
	void saturn_invalid6( int op1, int op2, int op3, int op4, int op5, int op6 );
	void saturn_instruction_0e();
	void saturn_instruction_1();
	void saturn_instruction_80();
	void saturn_instruction_81a();
	void saturn_instruction_81();
	void saturn_instruction_8();
	void saturn_instruction_9();
	void saturn_instruction_a();
	void saturn_instruction_b();
	void saturn_instruction();
};

extern const device_type SATURN;


#endif /* __SATURN_H__ */
