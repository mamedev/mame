// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_CPU_VAX_VAX_H
#define MAME_CPU_VAX_VAX_H

#pragma once


class vax_cpu_device : public cpu_device
{
public:
	enum
	{
		VAX_R0, VAX_R1, VAX_R2, VAX_R3,
		VAX_R4, VAX_R5, VAX_R6, VAX_R7,
		VAX_R8, VAX_R9, VAX_R10, VAX_R11,
		VAX_AP, VAX_FP, VAX_SP, VAX_PC,
		VAX_PSL
	};

protected:
	// construction/destruction
	vax_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrwidth);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// device_execute_interface overrides
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// address space
	address_space_config m_program_config;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program_space;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;

	// internal state
	u32 m_gpr[16];
	u32 m_psl;

	// execution sequencing
	s32 m_icount;
};

class kd32a_device : public vax_cpu_device
{
public:
	// construction/destruction
	kd32a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class dc333_device : public vax_cpu_device
{
public:
	// construction/destruction
	dc333_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class dc341_device : public vax_cpu_device
{
public:
	// construction/destruction
	dc341_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_reset() override ATTR_COLD;
};


// device type declarations
DECLARE_DEVICE_TYPE(KD32A, kd32a_device)
DECLARE_DEVICE_TYPE(DC333, dc333_device)
DECLARE_DEVICE_TYPE(DC341, dc341_device)

#endif // MAME_CPU_VAX_VAX_H
