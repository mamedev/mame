// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_MK1_MK1_H
#define MAME_CPU_MK1_MK1_H

#pragma once

class mk1_cpu_device : public cpu_device
{
public:
	enum {
		MK1_PC, MK1_OP,
		MK1_W, MK1_IP,
		MK1_TOS, MK1_RS,
		MK1_PSP, MK1_RSP,
		MK1_A, MK1_B, MK1_ALU, MK1_F,
		MK1_IE
	};

	static constexpr int AS_STACK = AS_DATA + 1;
	static constexpr int IRQ_LINE = 0;

	// device type constructor
	mk1_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int linenum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	// internal helpers
	void alu_update();
	void set_alu_function(u8 data);
	void set_irq_enable(bool state);
	void execute_one();

	static const u8 s_alu_decode[16];

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_data_config;
	const address_space_config m_stack_config;
	memory_access<12, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;
	memory_access<10, 0, 0, ENDIANNESS_LITTLE>::specific m_stack;

	// internal state
	u16 m_pc;
	u8 m_inst;
	u8 m_op_latch;
	u16 m_index_reg[2];
	u8 m_sp[2];
	u8 m_alu_a;
	u8 m_alu_b;
	u8 m_alu_function;
	u8 m_alu_result;
	u8 m_cond_flags;
	bool m_irq_asserted;
	bool m_irq_enabled;
	s32 m_icount;
};

// device type declaration
DECLARE_DEVICE_TYPE(MK1_CPU, mk1_cpu_device)

#endif // MAME_CPU_MK1_MK1_H
