// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*****************************************************************************
*
*   ns32000.h
*
*   NS32000 CPU family
*
*****************************************************************************/

#ifndef MAME_CPU_NS32016_NS32016_H
#define MAME_CPU_NS32016_NS32016_H

#pragma once


/***********************************************************************
  CONSTANTS
***********************************************************************/



/***********************************************************************
  TYPE DEFINITIONS
***********************************************************************/

class ns32000_cpu_device : public cpu_device
{
public:
	// construction/destruction
	ns32000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

protected:
	ns32000_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int databits, int addrbits);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 6; }
	virtual u32 execute_input_lines() const noexcept override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }


	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;
	virtual bool memory_translate(int spacenum, int intention, offs_t &address) override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	// addressing modes
	enum
	{
		ADDRESSING_MODE_REGISTER_0 = 0,
		ADDRESSING_MODE_REGISTER_1,
		ADDRESSING_MODE_REGISTER_2,
		ADDRESSING_MODE_REGISTER_3,
		ADDRESSING_MODE_REGISTER_4,
		ADDRESSING_MODE_REGISTER_5,
		ADDRESSING_MODE_REGISTER_6,
		ADDRESSING_MODE_REGISTER_7,
		ADDRESSING_MODE_REGISTER_0_RELATIVE,
		ADDRESSING_MODE_REGISTER_1_RELATIVE,
		ADDRESSING_MODE_REGISTER_2_RELATIVE,
		ADDRESSING_MODE_REGISTER_3_RELATIVE,
		ADDRESSING_MODE_REGISTER_4_RELATIVE,
		ADDRESSING_MODE_REGISTER_5_RELATIVE,
		ADDRESSING_MODE_REGISTER_6_RELATIVE,
		ADDRESSING_MODE_REGISTER_7_RELATIVE,
		ADDRESSING_MODE_FRAME_MEMORY_RELATIVE,
		ADDRESSING_MODE_STACK_MEMORY_RELATIVE,
		ADDRESSING_MODE_STATIC_MEMORY_RELATIVE,
		ADDRESSING_MODE_RESERVED,
		ADDRESSING_MODE_IMMEDIATE,
		ADDRESSING_MODE_ABSOLUTE,
		ADDRESSING_MODE_EXTERNAL,
		ADDRESSING_MODE_TOP_OF_STACK,
		ADDRESSING_MODE_FRAME_MEMORY,
		ADDRESSING_MODE_STACK_MEMORY,
		ADDRESSING_MODE_STATIC_MEMORY,
		ADDRESSING_MODE_PROGRAM_MEMORY,
		ADDRESSING_MODE_INDEX_BYTES,
		ADDRESSING_MODE_INDEX_WORDS,
		ADDRESSING_MODE_INDEX_DOUBLE_WORDS,
		ADDRESSING_MODE_INDEX_QUAD_WORDS,
	};

	// registers
	enum
	{
		NS32000_PC = 1,
		NS32000_SB, NS32000_FP, NS32000_SP1, NS32000_SP0, NS32000_INTBASE, NS32000_PSR, NS32000_MOD,
		NS32000_R0, NS32000_R1, NS32000_R2, NS32000_R3, NS32000_R4, NS32000_R5, NS32000_R6, NS32000_R7,
	};

private:
	// configuration
	address_space_config m_program_config;

	// emulation state
	int m_icount;

	u32 m_pc;
	u32 m_sb;
	u32 m_fp;
	u32 m_sp1;
	u32 m_sp0;
	u32 m_intbase;
	u32 m_psr;
	u32 m_mod;
	u32 m_r[8];
	u32 m_f[8];

	u8 m_irq_line;
	u8 m_nmi_line;
};

class ns32008_cpu_device : public ns32000_cpu_device
{
public:
	ns32008_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class ns32016_cpu_device : public ns32000_cpu_device
{
public:
	ns32016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

class ns32032_cpu_device : public ns32000_cpu_device
{
public:
	ns32032_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

/***********************************************************************
  DEVICE TYPE DECLARATIONS
***********************************************************************/

DECLARE_DEVICE_TYPE(NS32008, ns32008_cpu_device)
DECLARE_DEVICE_TYPE(NS32016, ns32016_cpu_device)
DECLARE_DEVICE_TYPE(NS32032, ns32032_cpu_device)

#endif // MAME_CPU_NS32016_NS32016_H
