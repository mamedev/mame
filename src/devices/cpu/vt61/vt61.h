// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VT61_VT61_H
#define MAME_CPU_VT61_VT61_H

#pragma once

class vt61_cpu_device : public cpu_device
{
public:
	enum {
		VT61_PC,
		VT61_AC,
		VT61_MAR, VT61_MALO, VT61_MAHI,
		VT61_MD, VT61_MDR,
		VT61_IR,
		VT61_R0, VT61_R1, VT61_R2, VT61_R3,
		VT61_R4, VT61_R5, VT61_R6, VT61_R7,
		VT61_R8, VT61_R9, VT61_R10, VT61_R11,
		VT61_R12, VT61_R13, VT61_R14, VT61_R15,
		VT61_MISC, VT61_MOD, VT61_INTRC
	};

	enum {
		AS_IDR = 2
	};

	// construction/destruction
	vt61_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	enum class state : u8 {
		T1, T2, T3, T4, T5, T6, T7, T8, T9, T10
	};

	enum class pc_mode : u8 {
		CLR, INC, LD
	};

	enum pv_flags : u8 {
		CARRY_OUT = 1 << 0,
		EQUAL = 1 << 1,
		MDR_OF_OUT = 1 << 2
	};

	// instruction helpers
	u8 get_db(u16 i);
	bool branch_mux(u8 cond);
	void operate_t5(u16 i);
	void operate_t8(u16 i);

	// address spaces
	address_space_config m_program_config;
	address_space_config m_memory_config;
	address_space_config m_idr_config;
	memory_access<10, 1, -1, ENDIANNESS_LITTLE>::cache m_program_cache;
	memory_access<16, 0,  0, ENDIANNESS_LITTLE>::cache m_memory_cache;
	memory_access< 6, 0,  0, ENDIANNESS_LITTLE>::cache m_idr_cache;

	// processor state
	state m_time_state;
	u16 m_pc;
	pc_mode m_pc_mode;
	u8 m_pv_flags;
	u8 m_ac;
	u16 m_mar;
	u8 m_md; // memory output
	u8 m_mdr;
	u8 m_ir;
	u8 m_sp[16]; // scratchpad memory
	s32 m_icount;

	// I/O registers
	u8 m_misc_flags;
	u8 m_modem_flags;
	u8 m_intrpt_control;
};

DECLARE_DEVICE_TYPE(VT61_CPU, vt61_cpu_device)

#endif // MAME_CPU_VT61_VT61_H
