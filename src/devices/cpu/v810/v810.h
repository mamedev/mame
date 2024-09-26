// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Tomasz Slanina
#ifndef MAME_CPU_V810_V810_H
#define MAME_CPU_V810_V810_H

#pragma once


enum
{
	V810_R0=1,
	V810_R1,
	V810_R2,  /* R2 - handler stack pointer */
	V810_SP,  /* R3 - stack pointer */
	V810_R4,  /* R4 - global pointer */
	V810_R5,  /* R5 - text pointer */
	V810_R6,
	V810_R7,
	V810_R8,
	V810_R9,
	V810_R10,
	V810_R11,
	V810_R12,
	V810_R13,
	V810_R14,
	V810_R15,
	V810_R16,
	V810_R17,
	V810_R18,
	V810_R19,
	V810_R20,
	V810_R21,
	V810_R22,
	V810_R23,
	V810_R24,
	V810_R25,
	V810_R26,
	V810_R27,
	V810_R28,
	V810_R29,
	V810_R30,
	V810_R31, /* R31 - link pointer */

	/* System Registers */
	V810_EIPC, /* Exception/interrupt  saving - PC */
	V810_EIPSW,/* Exception/interrupt  saving - PSW */
	V810_FEPC, /* Duplexed exception/NMI  saving - PC */
	V810_FEPSW,/* Duplexed exception/NMI  saving - PSW */
	V810_ECR,  /* Exception cause register */
	V810_PSW,  /* Program status word */
	V810_PIR,  /* Processor ID register */
	V810_TKCW, /* Task control word */
	V810_res08,
	V810_res09,
	V810_res10,
	V810_res11,
	V810_res12,
	V810_res13,
	V810_res14,
	V810_res15,
	V810_res16,
	V810_res17,
	V810_res18,
	V810_res19,
	V810_res20,
	V810_res21,
	V810_res22,
	V810_res23,
	V810_CHCW,  /* Cache control word */
	V810_ADTRE, /* Address trap register */
	V810_res26,
	V810_res27,
	V810_res28,
	V810_res29,
	V810_res30,
	V810_res31,

	V810_PC
};


class v810_device : public cpu_device
{
public:
	// construction/destruction
	v810_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 3; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 6; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	typedef uint32_t (v810_device::*opcode_func)(uint32_t op);
	static const opcode_func s_OpCodeTable[64];

	address_space_config m_program_config;
	address_space_config m_io_config;

	uint32_t m_reg[65]{};
	uint16_t m_irq_state;
	uint8_t m_nmi_line;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_io;
	uint32_t m_PPC;
	int m_icount;

	inline void SETREG(uint32_t reg,uint32_t val);
	inline uint32_t GETREG(uint32_t reg);
	uint32_t opUNDEF(uint32_t op);
	uint32_t opMOVr(uint32_t op);
	uint32_t opMOVEA(uint32_t op);
	uint32_t opMOVHI(uint32_t op);
	uint32_t opMOVi(uint32_t op);
	uint32_t opADDr(uint32_t op);
	uint32_t opADDi(uint32_t op);
	uint32_t opADDI(uint32_t op);
	uint32_t opSUBr(uint32_t op);
	uint32_t opCMPr(uint32_t op);
	uint32_t opCMPi(uint32_t op);
	uint32_t opSETFi(uint32_t op);
	uint32_t opANDr(uint32_t op);
	uint32_t opANDI(uint32_t op);
	uint32_t opORr(uint32_t op);
	uint32_t opORI(uint32_t op);
	uint32_t opXORr(uint32_t op);
	uint32_t opLDSR(uint32_t op);
	uint32_t opSTSR(uint32_t op);
	uint32_t opXORI(uint32_t op);
	uint32_t opNOTr(uint32_t op);
	uint32_t opSHLr(uint32_t op);
	uint32_t opSHLi(uint32_t op);
	uint32_t opSHRr(uint32_t op);
	uint32_t opSHRi(uint32_t op);
	uint32_t opSARr(uint32_t op);
	uint32_t opSARi(uint32_t op);
	uint32_t opJMPr(uint32_t op);
	uint32_t opJR(uint32_t op);
	uint32_t opJAL(uint32_t op);
	uint32_t opEI(uint32_t op);
	uint32_t opDI(uint32_t op);
	uint32_t opTRAP(uint32_t op);
	uint32_t opRETI(uint32_t op);
	uint32_t opHALT(uint32_t op);
	uint32_t opB(uint32_t op);
	uint32_t opLDB(uint32_t op);
	uint32_t opLDH(uint32_t op);
	uint32_t opLDW(uint32_t op);
	uint32_t opINB(uint32_t op);
	uint32_t opCAXI(uint32_t op);
	uint32_t opINH(uint32_t op);
	uint32_t opINW(uint32_t op);
	uint32_t opSTB(uint32_t op);
	uint32_t opSTH(uint32_t op);
	uint32_t opSTW(uint32_t op);
	uint32_t opOUTB(uint32_t op);
	uint32_t opOUTH(uint32_t op);
	uint32_t opOUTW(uint32_t op);
	uint32_t opMULr(uint32_t op);
	uint32_t opMULUr(uint32_t op);
	uint32_t opDIVr(uint32_t op);
	uint32_t opDIVUr(uint32_t op);
	uint32_t opADDF(uint32_t op);
	uint32_t opSUBF(uint32_t op);
	uint32_t opMULF(uint32_t op);
	uint32_t opDIVF(uint32_t op);
	uint32_t opTRNC(uint32_t op);
	uint32_t opCMPF(uint32_t op);
	uint32_t opCVTS(uint32_t op);
	uint32_t opCVTW(uint32_t op);
	uint32_t opMPYHW(uint32_t op);
	uint32_t opXB(uint32_t op);
	uint32_t opXH(uint32_t op);
	uint32_t opFpoint(uint32_t op);
	uint32_t opBSU(uint32_t op);
	void check_interrupts();
};


DECLARE_DEVICE_TYPE(V810, v810_device)

#endif // MAME_CPU_V810_V810_H
