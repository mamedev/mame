// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_CR16B_CR16B_H
#define MAME_CPU_CR16B_CR16B_H

#pragma once


class cr16b_device : public cpu_device
{
public:
	cr16b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	enum {
		CR16_PC, CR16_ISP, CR16_INTBASE,
		CR16_PSR, CR16_CFG, CR16_DCR, CR16_DSR, CR16_CAR,
		CR16_R0, CR16_R1, CR16_R2, CR16_R3,
		CR16_R4, CR16_R5, CR16_R6, CR16_R7,
		CR16_R8, CR16_R9, CR16_R10, CR16_R11,
		CR16_R12, CR16_R13, CR16_R14, CR16_R15
	};

protected:
	// construction/destruction
	cr16b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map);

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

private:
	// address space
	address_space_config m_space_config;
	memory_access<21, 1, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<21, 1, 0, ENDIANNESS_LITTLE>::specific m_space;

	// internal state
	u16 m_regs[16];
	u32 m_pc;
	u32 m_isp;
	u32 m_intbase;
	u16 m_psr;
	u16 m_cfg;
	u16 m_dcr;
	u16 m_dsr;
	u32 m_car;
	s32 m_icount;
};

DECLARE_DEVICE_TYPE(CR16B, cr16b_device)

#endif // MAME_CPU_CR16B_CR16B_H
