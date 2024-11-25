// license:BSD-3-Clause
// copyright-holders:ElSemi
#ifndef MAME_CPU_SE3208_SE3208_H
#define MAME_CPU_SE3208_SE3208_H

#pragma once

enum
{
	SE3208_PC=1, SE3208_SR, SE3208_ER, SE3208_SP,SE3208_PPC,
	SE3208_R0, SE3208_R1, SE3208_R2, SE3208_R3, SE3208_R4, SE3208_R5, SE3208_R6, SE3208_R7
};

#define SE3208_INT  0


class se3208_device :  public cpu_device
{
public:
	// construction/destruction
	se3208_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callback configuration
	auto machinex_cb() { return m_machinex_cb.bind(); }
	auto iackx_cb() { return m_iackx_cb.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 1; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface implementation
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface implementation
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface implementation
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

private:
	address_space_config m_program_config;

	devcb_write8 m_machinex_cb;
	devcb_read8 m_iackx_cb;

	//GPR
	uint32_t m_R[8];
	//SPR
	uint32_t m_PC;
	uint32_t m_SR;
	uint32_t m_SP;
	uint32_t m_ER;
	uint32_t m_PPC;

	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;
	uint8_t m_IRQ;
	uint8_t m_NMI;

	int m_icount;

	inline void CLRFLAG(uint32_t f) { m_SR&=~f; }
	inline void SETFLAG(uint32_t f) { m_SR|=f; }
	inline bool TESTFLAG(uint32_t f) const { return m_SR&f; }

	inline uint8_t SE3208_Read8(uint32_t addr);
	inline uint16_t SE3208_Read16(uint32_t addr);
	inline uint32_t SE3208_Read32(uint32_t addr);
	inline void SE3208_Write8(uint32_t addr,uint8_t val);
	inline void SE3208_Write16(uint32_t addr,uint16_t val);
	inline void SE3208_Write32(uint32_t addr,uint32_t val);
	inline uint32_t AddWithFlags(uint32_t a,uint32_t b);
	inline uint32_t SubWithFlags(uint32_t a,uint32_t b);
	inline uint32_t AdcWithFlags(uint32_t a,uint32_t b);
	inline uint32_t SbcWithFlags(uint32_t a,uint32_t b);
	inline uint32_t MulWithFlags(uint32_t a,uint32_t b);
	inline uint32_t NegWithFlags(uint32_t a);
	inline uint32_t AsrWithFlags(uint32_t Val, uint8_t By);
	inline uint32_t LsrWithFlags(uint32_t Val, uint8_t By);
	inline uint32_t AslWithFlags(uint32_t Val, uint8_t By);
	inline void PushVal(uint32_t Val);
	inline uint32_t PopVal();
	inline void TakeExceptionVector(uint8_t vector);

	typedef void (se3208_device::*OP)(uint16_t Opcode);
	OP OpTable[0x10000];

	void INVALIDOP(uint16_t Opcode);
	void LDB(uint16_t Opcode);
	void STB(uint16_t Opcode);
	void LDS(uint16_t Opcode);
	void STS(uint16_t Opcode);
	void LD(uint16_t Opcode);
	void ST(uint16_t Opcode);
	void LDBU(uint16_t Opcode);
	void LDSU(uint16_t Opcode);
	void LERI(uint16_t Opcode);
	void LDSP(uint16_t Opcode);
	void STSP(uint16_t Opcode);
	void PUSH(uint16_t Opcode);
	void POP(uint16_t Opcode);
	void LEATOSP(uint16_t Opcode);
	void LEAFROMSP(uint16_t Opcode);
	void LEASPTOSP(uint16_t Opcode);
	void MOV(uint16_t Opcode);
	void LDI(uint16_t Opcode);
	void LDBSP(uint16_t Opcode);
	void STBSP(uint16_t Opcode);
	void LDSSP(uint16_t Opcode);
	void STSSP(uint16_t Opcode);
	void LDBUSP(uint16_t Opcode);
	void LDSUSP(uint16_t Opcode);
	void ADDI(uint16_t Opcode);
	void SUBI(uint16_t Opcode);
	void ADCI(uint16_t Opcode);
	void SBCI(uint16_t Opcode);
	void ANDI(uint16_t Opcode);
	void ORI(uint16_t Opcode);
	void XORI(uint16_t Opcode);
	void CMPI(uint16_t Opcode);
	void TSTI(uint16_t Opcode);
	void ADD(uint16_t Opcode);
	void SUB(uint16_t Opcode);
	void ADC(uint16_t Opcode);
	void SBC(uint16_t Opcode);
	void AND(uint16_t Opcode);
	void OR(uint16_t Opcode);
	void XOR(uint16_t Opcode);
	void CMP(uint16_t Opcode);
	void TST(uint16_t Opcode);
	void MULS(uint16_t Opcode);
	void NEG(uint16_t Opcode);
	void CALL(uint16_t Opcode);
	void JV(uint16_t Opcode);
	void JNV(uint16_t Opcode);
	void JC(uint16_t Opcode);
	void JNC(uint16_t Opcode);
	void JP(uint16_t Opcode);
	void JM(uint16_t Opcode);
	void JNZ(uint16_t Opcode);
	void JZ(uint16_t Opcode);
	void JGE(uint16_t Opcode);
	void JLE(uint16_t Opcode);
	void JHI(uint16_t Opcode);
	void JLS(uint16_t Opcode);
	void JGT(uint16_t Opcode);
	void JLT(uint16_t Opcode);
	void JMP(uint16_t Opcode);
	void JR(uint16_t Opcode);
	void CALLR(uint16_t Opcode);
	void ASR(uint16_t Opcode);
	void LSR(uint16_t Opcode);
	void ASL(uint16_t Opcode);
	void EXTB(uint16_t Opcode);
	void EXTS(uint16_t Opcode);
	void SET(uint16_t Opcode);
	void CLR(uint16_t Opcode);
	void SWI(uint16_t Opcode);
	void HALT(uint16_t Opcode);
	void MVTC(uint16_t Opcode);
	void MVFC(uint16_t Opcode);

	void BuildTable(void);
	OP DecodeOp(uint16_t Opcode);
	void SE3208_NMI();
	void SE3208_Interrupt();

};


DECLARE_DEVICE_TYPE(SE3208, se3208_device)

#endif // MAME_CPU_SE3208_SE3208_H
