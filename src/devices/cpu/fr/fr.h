// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_FR_FR_H
#define MAME_CPU_FR_FR_H

#pragma once

class fr_cpu_device : public cpu_device
{
public:
	enum {
		FR_PC, FR_PS, FR_CCR, FR_ILM,
		FR_TBR, FR_RP, FR_SSP, FR_USP,
		FR_MD, FR_MDH, FR_MDL,
		FR_R0, FR_R1, FR_R2, FR_R3,
		FR_R4, FR_R5, FR_R6, FR_R7,
		FR_R8, FR_R9, FR_R10, FR_R11,
		FR_R12, FR_R13, FR_R14, FR_R15
	};

protected:
	// construction/destruction
	fr_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor map);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str) const override;

private:
	// address space
	address_space_config m_space_config;
	memory_access<24, 2, 0, ENDIANNESS_BIG>::cache m_cache;
	memory_access<24, 2, 0, ENDIANNESS_BIG>::specific m_space;

	// internal state
	u32 m_regs[17]; // includes both SSP and USP
	u32 m_pc;
	u32 m_ps;
	u32 m_tbr;
	u32 m_rp;
	u64 m_md;
	s32 m_icount;
};

class mb91f155a_device : public fr_cpu_device
{
public:
	// device type constructor
	mb91f155a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

private:
	void internal_map(address_map &map) ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(MB91F155A, mb91f155a_device)

#endif // MAME_CPU_FR_FR_H
