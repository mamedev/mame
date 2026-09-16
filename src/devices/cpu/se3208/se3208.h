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



class se3208_device :  public cpu_device
{
public:
	enum
	{
		SE3208_INT = 0
	};
	// construction/destruction
	se3208_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// callback configuration
	auto machinex_cb() { return m_machinex_cb.bind(); }
	auto iackx_cb() { return m_iackx_cb.bind(); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u32 execute_min_cycles() const noexcept override { return 1; }
	virtual u32 execute_max_cycles() const noexcept override { return 1; }
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
	u32 m_R[8]{};
	//SPR
	u32 m_PC;
	u32 m_SR;
	u32 m_SP;
	u32 m_ER;
	u32 m_PPC;

	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::cache m_cache;
	memory_access<32, 2, 0, ENDIANNESS_LITTLE>::specific m_program;
	u8 m_IRQ;
	u8 m_NMI;

	int m_icount;

	typedef void (se3208_device::*OP)(u16 opcode);
	OP m_optable[0x10000];

	inline void CLRFLAG(u32 f) { m_SR &= ~f; }
	inline void SETFLAG(u32 f) { m_SR |= f; }
	inline bool TESTFLAG(u32 f) const { return m_SR & f; }

	inline u8 read8(u32 addr);
	inline u16 read16(u32 addr);
	inline u32 read32(u32 addr);
	inline void write8(u32 addr, u8 val);
	inline void write16(u32 addr, u16 val);
	inline void write32(u32 addr, u32 val);
	inline u32 add_with_lfags(u32 a, u32 b);
	inline u32 sub_with_lfags(u32 a, u32 b);
	inline u32 adc_with_lfags(u32 a, u32 b);
	inline u32 sbc_with_lfags(u32 a, u32 b);
	inline u32 mul_with_lfags(u32 a, u32 b);
	inline u32 neg_with_lfags(u32 a);
	inline u32 asr_with_lfags(u32 Val, u8 By);
	inline u32 lsr_with_lfags(u32 Val, u8 By);
	inline u32 asl_with_lfags(u32 Val, u8 By);
	inline u32 get_index(u32 index);
	inline u32 get_extended_operand(u32 imm, u8 shift);
	inline void push_val(u32 Val);
	inline u32 pop_val();
	inline void take_exception_vector(u8 vector);

	void INVALIDOP(u16 opcode);
	void LDB(u16 opcode);
	void STB(u16 opcode);
	void LDS(u16 opcode);
	void STS(u16 opcode);
	void LD(u16 opcode);
	void ST(u16 opcode);
	void LDBU(u16 opcode);
	void LDSU(u16 opcode);
	void LERI(u16 opcode);
	void LDSP(u16 opcode);
	void STSP(u16 opcode);
	void PUSH(u16 opcode);
	void POP(u16 opcode);
	void LEATOSP(u16 opcode);
	void LEAFROMSP(u16 opcode);
	void LEASPTOSP(u16 opcode);
	void MOV(u16 opcode);
	void LDI(u16 opcode);
	void LDBSP(u16 opcode);
	void STBSP(u16 opcode);
	void LDSSP(u16 opcode);
	void STSSP(u16 opcode);
	void LDBUSP(u16 opcode);
	void LDSUSP(u16 opcode);
	void ADDI(u16 opcode);
	void SUBI(u16 opcode);
	void ADCI(u16 opcode);
	void SBCI(u16 opcode);
	void ANDI(u16 opcode);
	void ORI(u16 opcode);
	void XORI(u16 opcode);
	void CMPI(u16 opcode);
	void TSTI(u16 opcode);
	void ADD(u16 opcode);
	void SUB(u16 opcode);
	void ADC(u16 opcode);
	void SBC(u16 opcode);
	void AND(u16 opcode);
	void OR(u16 opcode);
	void XOR(u16 opcode);
	void CMP(u16 opcode);
	void TST(u16 opcode);
	void MULS(u16 opcode);
	void NEG(u16 opcode);
	void CALL(u16 opcode);
	void JV(u16 opcode);
	void JNV(u16 opcode);
	void JC(u16 opcode);
	void JNC(u16 opcode);
	void JP(u16 opcode);
	void JM(u16 opcode);
	void JNZ(u16 opcode);
	void JZ(u16 opcode);
	void JGE(u16 opcode);
	void JLE(u16 opcode);
	void JHI(u16 opcode);
	void JLS(u16 opcode);
	void JGT(u16 opcode);
	void JLT(u16 opcode);
	void JMP(u16 opcode);
	void JR(u16 opcode);
	void CALLR(u16 opcode);
	void ASR(u16 opcode);
	void LSR(u16 opcode);
	void ASL(u16 opcode);
	void EXTB(u16 opcode);
	void EXTS(u16 opcode);
	void SET(u16 opcode);
	void CLR(u16 opcode);
	void SWI(u16 opcode);
	void HALT(u16 opcode);
	void MVTC(u16 opcode);
	void MVFC(u16 opcode);

	void build_table();
	OP decode_op(u16 opcode);
	void nmi_execute();
	void interrupt_execute();

};


DECLARE_DEVICE_TYPE(SE3208, se3208_device)

#endif // MAME_CPU_SE3208_SE3208_H
