// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Tomasz Slanina
#pragma once

#ifndef __V810_H__
#define __V810_H__


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
	v810_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 3; }
	virtual UINT32 execute_max_cycles() const override { return 6; }
	virtual UINT32 execute_input_lines() const override { return 16; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ((spacenum == AS_IO) ? &m_io_config : nullptr); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	typedef UINT32 (v810_device::*opcode_func)(UINT32 op);
	static const opcode_func s_OpCodeTable[64];

	address_space_config m_program_config;
	address_space_config m_io_config;

	UINT32 m_reg[65];
	UINT8 m_irq_line;
	UINT8 m_irq_state;
	UINT8 m_nmi_line;
	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;
	UINT32 m_PPC;
	int m_icount;

	inline void SETREG(UINT32 reg,UINT32 val);
	inline UINT32 GETREG(UINT32 reg);
	UINT32 opUNDEF(UINT32 op);
	UINT32 opMOVr(UINT32 op);
	UINT32 opMOVEA(UINT32 op);
	UINT32 opMOVHI(UINT32 op);
	UINT32 opMOVi(UINT32 op);
	UINT32 opADDr(UINT32 op);
	UINT32 opADDi(UINT32 op);
	UINT32 opADDI(UINT32 op);
	UINT32 opSUBr(UINT32 op);
	UINT32 opCMPr(UINT32 op);
	UINT32 opCMPi(UINT32 op);
	UINT32 opSETFi(UINT32 op);
	UINT32 opANDr(UINT32 op);
	UINT32 opANDI(UINT32 op);
	UINT32 opORr(UINT32 op);
	UINT32 opORI(UINT32 op);
	UINT32 opXORr(UINT32 op);
	UINT32 opLDSR(UINT32 op);
	UINT32 opSTSR(UINT32 op);
	UINT32 opXORI(UINT32 op);
	UINT32 opNOTr(UINT32 op);
	UINT32 opSHLr(UINT32 op);
	UINT32 opSHLi(UINT32 op);
	UINT32 opSHRr(UINT32 op);
	UINT32 opSHRi(UINT32 op);
	UINT32 opSARr(UINT32 op);
	UINT32 opSARi(UINT32 op);
	UINT32 opJMPr(UINT32 op);
	UINT32 opJR(UINT32 op);
	UINT32 opJAL(UINT32 op);
	UINT32 opEI(UINT32 op);
	UINT32 opDI(UINT32 op);
	UINT32 opTRAP(UINT32 op);
	UINT32 opRETI(UINT32 op);
	UINT32 opHALT(UINT32 op);
	UINT32 opB(UINT32 op);
	UINT32 opLDB(UINT32 op);
	UINT32 opLDH(UINT32 op);
	UINT32 opLDW(UINT32 op);
	UINT32 opINB(UINT32 op);
	UINT32 opCAXI(UINT32 op);
	UINT32 opINH(UINT32 op);
	UINT32 opINW(UINT32 op);
	UINT32 opSTB(UINT32 op);
	UINT32 opSTH(UINT32 op);
	UINT32 opSTW(UINT32 op);
	UINT32 opOUTB(UINT32 op);
	UINT32 opOUTH(UINT32 op);
	UINT32 opOUTW(UINT32 op);
	UINT32 opMULr(UINT32 op);
	UINT32 opMULUr(UINT32 op);
	UINT32 opDIVr(UINT32 op);
	UINT32 opDIVUr(UINT32 op);
	void opADDF(UINT32 op);
	void opSUBF(UINT32 op);
	void opMULF(UINT32 op);
	void opDIVF(UINT32 op);
	void opTRNC(UINT32 op);
	void opCMPF(UINT32 op);
	void opCVTS(UINT32 op);
	void opCVTW(UINT32 op);
	void opMPYHW(UINT32 op);
	void opXB(UINT32 op);
	void opXH(UINT32 op);
	UINT32 opFpoint(UINT32 op);
	UINT32 opBSU(UINT32 op);
	void take_interrupt();

};


extern const device_type V810;


#endif /* __V810_H__ */
