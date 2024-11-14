// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_CPU_M68000_M68008_H
#define MAME_CPU_M68000_M68008_H

#pragma once

#include "m68000.h"

class m68008_device : public m68000_device
{
public:
	struct mmu8 {
		virtual u8 read_program(offs_t addr) = 0;
		virtual void write_program(offs_t addr, u8 data) = 0;
		virtual u8 read_data(offs_t addr) = 0;
		virtual void write_data(offs_t addr, u8 data) = 0;
		virtual u8 read_cpu(offs_t addr) = 0;
		virtual void set_super(bool super) = 0;
	};

	// construction/destruction
	m68008_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void set_current_mmu(mmu8 *m);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

protected:
	// Typed constructor
	m68008_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	using handler8 = void (m68008_device::*)();

#include "m68008-head.h"

	static const handler8 s_handlers_df8[];
	static const handler8 s_handlers_if8[];
	static const handler8 s_handlers_dp8[];
	static const handler8 s_handlers_ip8[];

	// Fixed specifics
	memory_access<24, 0, 0, ENDIANNESS_BIG>::specific m_r_program8, m_r_opcodes8, m_r_uprogram8, m_r_uopcodes8, m_cpu_space8;

	// Dynamic specifics, depending on supervisor state
	memory_access<24, 0, 0, ENDIANNESS_BIG>::specific m_program8, m_opcodes8;

	// MMU, if one present
	mmu8 *m_mmu8;

	virtual void update_user_super() override;

};

class m68008fn_device : public m68008_device
{
public:
	// construction/destruction
	m68008fn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	virtual u32 execute_min_cycles() const noexcept override { return 4; }
	virtual u32 execute_max_cycles() const noexcept override { return 158; }

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(M68008, m68008_device)
DECLARE_DEVICE_TYPE(M68008FN, m68008fn_device)

#endif
