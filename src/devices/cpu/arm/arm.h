// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#pragma once

#ifndef __ARM_H__
#define __ARM_H__

/****************************************************************************************************
 *  INTERRUPT CONSTANTS
 ***************************************************************************************************/

#define ARM_IRQ_LINE    0
#define ARM_FIRQ_LINE   1

/****************************************************************************************************
 *  PUBLIC FUNCTIONS
 ***************************************************************************************************/

enum
{
	ARM_COPRO_TYPE_UNKNOWN_CP15 = 0,
	ARM_COPRO_TYPE_VL86C020
};

#define MCFG_ARM_COPRO(_type) \
	arm_cpu_device::set_copro_type(*device, _type);


enum
{
	ARM32_PC=0,
	ARM32_R0, ARM32_R1, ARM32_R2, ARM32_R3, ARM32_R4, ARM32_R5, ARM32_R6, ARM32_R7,
	ARM32_R8, ARM32_R9, ARM32_R10, ARM32_R11, ARM32_R12, ARM32_R13, ARM32_R14, ARM32_R15,
	ARM32_FR8, ARM32_FR9, ARM32_FR10, ARM32_FR11, ARM32_FR12, ARM32_FR13, ARM32_FR14,
	ARM32_IR13, ARM32_IR14, ARM32_SR13, ARM32_SR14
};


class arm_cpu_device : public cpu_device
{
public:
	// construction/destruction
	arm_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	arm_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, endianness_t endianness);

	static void set_copro_type(device_t &device, int type) { downcast<arm_cpu_device &>(device).m_copro_type = type; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 3; }
	virtual UINT32 execute_max_cycles() const override { return 4; }
	virtual UINT32 execute_input_lines() const override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 4; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	address_space_config m_program_config;

	int m_icount;
	UINT32 m_sArmRegister[27];
	UINT32 m_coproRegister[16];
	UINT8 m_pendingIrq;
	UINT8 m_pendingFiq;
	address_space *m_program;
	direct_read_data *m_direct;
	endianness_t m_endian;
	UINT8 m_copro_type;

	void cpu_write32( int addr, UINT32 data );
	void cpu_write8( int addr, UINT8 data );
	UINT32 cpu_read32( int addr );
	UINT8 cpu_read8( int addr );
	UINT32 GetRegister( int rIndex );
	void SetRegister( int rIndex, UINT32 value );
	UINT32 GetModeRegister( int mode, int rIndex );
	void SetModeRegister( int mode, int rIndex, UINT32 value );
	void HandleALU(UINT32 insn);
	void HandleMul(UINT32 insn);
	void HandleBranch(UINT32 insn);
	void HandleMemSingle(UINT32 insn);
	void HandleMemBlock(UINT32 insn);
	void HandleCoPro(UINT32 insn);
	void HandleCoProVL86C020(UINT32 insn);
	UINT32 decodeShift(UINT32 insn, UINT32 *pCarry);
	void arm_check_irq_state();
	int loadInc(UINT32 pat, UINT32 rbv, UINT32 s);
	int loadDec(UINT32 pat, UINT32 rbv, UINT32 s, UINT32* deferredR15, int* defer);
	int storeInc(UINT32 pat, UINT32 rbv);
	int storeDec(UINT32 pat, UINT32 rbv);
	static UINT32 BCDToDecimal(UINT32 value);
	static UINT32 DecimalToBCD(UINT32 value);
};


class arm_be_cpu_device : public arm_cpu_device
{
public:
	// construction/destruction
	arm_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;
};


extern const device_type ARM;
extern const device_type ARM_BE;


#endif /* __ARM_H__ */
