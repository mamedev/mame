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

#ifndef MAME_CPU_SATURN_SATURN_H
#define MAME_CPU_SATURN_SATURN_H

#pragma once

#include "saturnds.h"

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


class saturn_device : public cpu_device, public saturn_disassembler::config
{
public:
	// construction/destruction
	saturn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto out_func() { return m_out_func.bind(); }
	auto in_func() { return m_in_func.bind(); }
	auto reset_func() { return m_reset_func.bind(); }
	auto config_func() { return m_config_func.bind(); }
	auto unconfig_func() { return m_unconfig_func.bind(); }
	auto id_func() { return m_id_func.bind(); }
	auto crc_func() { return m_crc_func.bind(); }
	auto rsi_func() { return m_rsi_func.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 2; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 21; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;
	virtual bool get_nonstandard_mnemonics_mode() const override;

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
	typedef uint8_t Saturn64[16];

	Saturn64 m_reg[9]; //r0,r1,r2,r3,r4,a,b,c,d

	uint32_t m_d[2], m_pc, m_oldpc, m_rstk[8]; // 20 bit, packed addresses

	uint8_t m_p; // 4 bit pointer

	uint16_t m_out; // 12 bit (packed)
	uint8_t  m_carry, m_decimal;
	uint16_t m_st; // status 16 bit

	uint8_t m_hst; // hardware status 4 bit

	uint8_t   m_nmi_state;
	uint8_t   m_irq_state;
	uint8_t   m_irq_enable;     /* INTON / INTOFF */
	uint8_t   m_in_irq;         /* already servicing IRQ */
	uint8_t   m_pending_irq;    /* IRQ is pending */
	uint8_t   m_sleeping;       /* low-consumption state */
	int     m_monitor_id;
	int     m_monitor_in;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::specific m_program;
	int m_icount;
	int64_t m_debugger_temp;

	void saturn_take_irq();
	void IntReg64(Saturn64 r, int64_t d);
	int64_t Reg64Int(Saturn64 r);

	int READ_OP();
	int READ_OP_ARG();
	int READ_OP_ARG8();
	int8_t READ_OP_DIS8();
	int READ_OP_ARG12();
	int READ_OP_DIS12();
	int READ_OP_ARG16();
	int16_t READ_OP_DIS16();
	int READ_OP_ARG20();
	int READ_NIBBLE(uint32_t adr);
	int READ_8(uint32_t adr);
	int READ_12(uint32_t adr);
	int READ_16(uint32_t adr);
	int READ_20(uint32_t adr);
	void WRITE_NIBBLE(uint32_t adr, uint8_t nib);
	int S64_READ_X(int r);
	int S64_READ_WORD(int r);
	int S64_READ_A(int r);
	void S64_WRITE_X(int r, int v);
	void S64_WRITE_WORD(int r, int v);
	void S64_WRITE_A(int r, int v);
	uint32_t saturn_pop();
	void saturn_push(uint32_t adr);
	void saturn_interrupt_on();
	void saturn_interrupt_off();
	void saturn_reset_interrupt();
	void saturn_mem_reset();
	void saturn_mem_config();
	void saturn_mem_unconfig();
	void saturn_mem_id();
	void saturn_shutdown();
	void saturn_bus_command_b();
	void saturn_bus_command_c();
	void saturn_bus_command_d();
	void saturn_serial_request();
	void saturn_out_c();
	void saturn_out_cs();
	void saturn_in(int reg);
	void saturn_sethex() { m_decimal=0; }
	void saturn_setdec() { m_decimal=1; }
	void saturn_clear_st();
	void saturn_st_to_c();
	void saturn_c_to_st();
	void saturn_exchange_c_st();
	void saturn_jump_after_test();
	void saturn_st_clear_bit();
	void saturn_st_set_bit();
	void saturn_st_jump_bit_clear();
	void saturn_st_jump_bit_set();
	void saturn_hst_clear_bits();
	void saturn_hst_bits_cleared();
	void saturn_exchange_p();
	void saturn_p_to_c();
	void saturn_c_to_p();
	void saturn_dec_p();
	void saturn_inc_p();
	void saturn_load_p();
	void saturn_p_equals();
	void saturn_p_not_equals();
	void saturn_ca_p_1();
	void saturn_load_reg(int reg);
	void saturn_jump(int adr, int jump);
	void saturn_call(int adr);
	void saturn_return(int yes);
	void saturn_return_carry_set();
	void saturn_return_carry_clear();
	void saturn_return_interrupt();
	void saturn_return_xm_set();
	void saturn_pop_c();
	void saturn_push_c();
	void saturn_indirect_jump(int reg);
	void saturn_equals_zero(int reg, int begin, int count);
	void saturn_equals(int reg, int begin, int count, int right);
	void saturn_not_equals_zero(int reg, int begin, int count);
	void saturn_not_equals(int reg, int begin, int count, int right);
	void saturn_greater(int reg, int begin, int count, int right);
	void saturn_greater_equals(int reg, int begin, int count, int right);
	void saturn_smaller_equals(int reg, int begin, int count, int right);
	void saturn_smaller(int reg, int begin, int count, int right);
	void saturn_jump_bit_clear(int reg);
	void saturn_jump_bit_set(int reg);
	void saturn_load_pc(int reg);
	void saturn_store_pc(int reg);
	void saturn_exchange_pc(int reg);
	void saturn_load_adr(int reg, int nibbles);
	void saturn_add_adr(int reg);
	void saturn_sub_adr(int reg);
	void saturn_adr_to_reg(int adr, int reg);
	void saturn_reg_to_adr(int reg, int adr);
	void saturn_adr_to_reg_word(int adr, int reg);
	void saturn_reg_to_adr_word(int reg, int adr);
	void saturn_exchange_adr_reg(int adr, int reg);
	void saturn_exchange_adr_reg_word(int adr, int reg);
	void saturn_load_nibbles(int reg, int begin, int count, int adr);
	void saturn_store_nibbles(int reg, int begin, int count, int adr);
	void saturn_clear_bit(int reg);
	void saturn_set_bit(int reg);
	void saturn_clear(int reg, int begin, int count);
	void saturn_exchange(int left, int begin, int count, int right);
	void saturn_copy(int dest, int begin, int count, int src);
	void saturn_add(int reg, int begin, int count, int right);
	void saturn_add_const(int reg, int begin, int count, uint8_t right);
	void saturn_sub(int reg, int begin, int count, int right);
	void saturn_sub_const(int reg, int begin, int count, int right);
	void saturn_sub2(int reg, int begin, int count, int right);
	void saturn_increment(int reg, int begin, int count);
	void saturn_decrement(int reg, int begin, int count);
	void saturn_invert(int reg, int begin, int count);
	void saturn_negate(int reg, int begin, int count);
	void saturn_or(int dest, int begin, int count, int src);
	void saturn_and(int dest, int begin, int count, int src);
	void saturn_shift_nibble_left(int reg, int begin, int count);
	void saturn_shift_nibble_right(int reg, int begin, int count);
	void saturn_rotate_nibble_left_w(int reg);
	void saturn_rotate_nibble_right_w(int reg);
	void saturn_shift_right(int reg, int begin, int count);
	void saturn_invalid3(int op1, int op2, int op3);
	void saturn_invalid4(int op1, int op2, int op3, int op4);
	void saturn_invalid5(int op1, int op2, int op3, int op4, int op5);
	void saturn_invalid6(int op1, int op2, int op3, int op4, int op5, int op6);
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

DECLARE_DEVICE_TYPE(SATURN, saturn_device)

#endif // MAME_CPU_SATURN_SATURN_H
