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
	arm_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	arm_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source, endianness_t endianness);

	static void set_copro_type(device_t &device, int type) { downcast<arm_cpu_device &>(device).m_copro_type = type; }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const override { return 3; }
	virtual uint32_t execute_max_cycles() const override { return 4; }
	virtual uint32_t execute_input_lines() const override { return 2; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : nullptr; }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual uint32_t disasm_min_opcode_bytes() const override { return 4; }
	virtual uint32_t disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;

	address_space_config m_program_config;

	int m_icount;
	uint32_t m_sArmRegister[27];
	uint32_t m_coproRegister[16];
	uint8_t m_pendingIrq;
	uint8_t m_pendingFiq;
	address_space *m_program;
	direct_read_data *m_direct;
	endianness_t m_endian;
	uint8_t m_copro_type;

	void cpu_write32( int addr, uint32_t data );
	void cpu_write8( int addr, uint8_t data );
	uint32_t cpu_read32( int addr );
	uint8_t cpu_read8( int addr );
	uint32_t GetRegister( int rIndex );
	void SetRegister( int rIndex, uint32_t value );
	uint32_t GetModeRegister( int mode, int rIndex );
	void SetModeRegister( int mode, int rIndex, uint32_t value );
	void HandleALU(uint32_t insn);
	void HandleMul(uint32_t insn);
	void HandleBranch(uint32_t insn);
	void HandleMemSingle(uint32_t insn);
	void HandleMemBlock(uint32_t insn);
	void HandleCoPro(uint32_t insn);
	void HandleCoProVL86C020(uint32_t insn);
	uint32_t decodeShift(uint32_t insn, uint32_t *pCarry);
	void arm_check_irq_state();
	int loadInc(uint32_t pat, uint32_t rbv, uint32_t s);
	int loadDec(uint32_t pat, uint32_t rbv, uint32_t s, uint32_t* deferredR15, int* defer);
	int storeInc(uint32_t pat, uint32_t rbv);
	int storeDec(uint32_t pat, uint32_t rbv);
	static uint32_t BCDToDecimal(uint32_t value);
	static uint32_t DecimalToBCD(uint32_t value);
};


class arm_be_cpu_device : public arm_cpu_device
{
public:
	// construction/destruction
	arm_be_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual offs_t disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options) override;
};


extern const device_type ARM;
extern const device_type ARM_BE;


#endif /* __ARM_H__ */
