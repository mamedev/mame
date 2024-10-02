// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef MAME_CPU_I86_I286_H
#define MAME_CPU_I86_I286_H

#pragma once

#include "i86.h"
#define INPUT_LINE_A20      1

DECLARE_DEVICE_TYPE(I80286, i80286_cpu_device)

enum
{   // same order as I8086 registers
	I286_PC = STATE_GENPC,

	I286_IP = 1,
	I286_AX,
	I286_CX,
	I286_DX,
	I286_BX,
	I286_SP,
	I286_BP,
	I286_SI,
	I286_DI,

	I286_AL,
	I286_AH,
	I286_CL,
	I286_CH,
	I286_DL,
	I286_DH,
	I286_BL,
	I286_BH,

	I286_FLAGS,

	I286_ES,
	I286_CS,
	I286_SS,
	I286_DS,

	I286_VECTOR,
	I286_HALT,

	I286_ES_BASE,
	I286_ES_LIMIT,
	I286_ES_FLAGS,
	I286_CS_BASE,
	I286_CS_LIMIT,
	I286_CS_FLAGS,
	I286_SS_BASE,
	I286_SS_LIMIT,
	I286_SS_FLAGS,
	I286_DS_BASE,
	I286_DS_LIMIT,
	I286_DS_FLAGS,

	I286_MSW,

	I286_GDTR_BASE,
	I286_GDTR_LIMIT,
	I286_IDTR_BASE,
	I286_IDTR_LIMIT,
	I286_TR,
	I286_TR_BASE,
	I286_TR_LIMIT,
	I286_TR_FLAGS,
	I286_LDTR,
	I286_LDTR_BASE,
	I286_LDTR_LIMIT,
	I286_LDTR_FLAGS
};

class i80286_cpu_device : public i8086_common_cpu_device
{
public:
	// construction/destruction
	i80286_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	template <typename... T>
	void set_a20_callback(T &&... args) { m_a20_callback.set(std::forward<T>(args)...); }

	auto shutdown_callback() { return m_out_shutdown_func.bind(); }

protected:
	typedef device_delegate<uint32_t (bool)> a20_cb;

	virtual void execute_run() override;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual void execute_set_input(int inputnum, int state) override;
	bool memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space) override;

	virtual void interrupt(int int_num, int trap = 1) override { if(trap) throw TRAP(int_num, (uint16_t)-1); else interrupt_descriptor(int_num, 0, 0); }
	virtual uint8_t read_port_byte(uint16_t port) override;
	virtual uint16_t read_port_word(uint16_t port) override;
	virtual void write_port_byte(uint16_t port, uint8_t data) override;
	virtual void write_port_word(uint16_t port, uint16_t data) override;

	// Executing instructions
	virtual uint8_t fetch() override;
	virtual uint32_t calc_addr(int seg, uint16_t offset, int size, int op, bool override = true) override;

private:
	void check_permission(uint8_t check_seg, uint32_t offset, uint16_t size, int operation);
	void code_descriptor(uint16_t selector, uint16_t offset, int gate);
	void data_descriptor(int reg, uint16_t selector);
	void data_descriptor(int reg, uint16_t selector, int cpl, uint32_t trap, uint16_t offset = 0, int size = 0);
	uint16_t far_return(int iret, int bytes);
	void interrupt_descriptor(int number, int hwint, int error);
	void load_flags(uint16_t flags, int cpl);
	void pop_seg(int reg);
	uint32_t selector_address(uint16_t sel);
	void switch_task(uint16_t ntask, int type);
	void trap(uint32_t error);
	int verify(uint16_t selector, int operation, uint8_t rights, bool valid);
	uint32_t update_pc() { return m_pc = m_base[CS] + m_ip; }

	int m_trap_level;
	uint16_t m_msw;
	uint32_t m_base[4];
	uint16_t m_limit[4];
	uint8_t m_rights[4];
	bool m_valid[4];
	uint32_t m_amask;

	struct {
		uint32_t base;
		uint16_t limit;
	} m_gdtr, m_idtr;
	struct {
		uint16_t sel;
		uint32_t base;
		uint16_t limit;
		uint8_t rights;
	} m_ldtr, m_tr;

	uint32_t TRAP(uint16_t fault, uint16_t code)  { return ((((uint32_t)fault&0xffff)<<16)|(code&0xffff)); }

	address_space_config m_program_config;
	address_space_config m_opcodes_config;
	address_space_config m_io_config;
	static const uint8_t m_i80286_timing[200];

	enum {
		FAULT_DE = 0,
		FAULT_DB,
		NMI,
		FAULT_BP,
		FAULT_OF,
		FAULT_BR,
		FAULT_UD,
		FAULT_NM,
		FAULT_DF,
		FAULT_MP,
		FAULT_TS,
		FAULT_NP,
		FAULT_SS,
		FAULT_GP
	};

	a20_cb m_a20_callback;
	bool m_shutdown;
	devcb_write_line m_out_shutdown_func;
};

#endif // MAME_CPU_I86_I286_H
