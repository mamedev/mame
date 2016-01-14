// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef __I8086_H__
#define __I8086_H__

#include "emu.h"

/////////////////////////////////////////////////////////////////

extern const device_type I8086;
extern const device_type I8088;

#define INPUT_LINE_INT0         INPUT_LINE_IRQ0
#define INPUT_LINE_TEST         20


#define MCFG_I8086_LOCK_HANDLER(_write) \
	devcb = &i8086_common_cpu_device::set_lock_handler(*device, DEVCB_##_write);


enum
{
	I8086_PC=0,
	I8086_IP, I8086_AX, I8086_CX, I8086_DX, I8086_BX, I8086_SP, I8086_BP, I8086_SI, I8086_DI,
	I8086_FLAGS, I8086_ES, I8086_CS, I8086_SS, I8086_DS,
	I8086_VECTOR, I8086_PENDING
};


class i8086_common_cpu_device : public cpu_device
{
public:
	// construction/destruction
	i8086_common_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &set_lock_handler(device_t &device, _Object object)
		{ return downcast<i8086_common_cpu_device &>(device).m_lock_handler.set_callback(object); }

protected:
	enum
	{
		EXCEPTION, IRET,                                /* EXCEPTION, iret */
		INT3, INT_IMM, INTO_NT, INTO_T,                 /* intS */
		OVERRIDE,                                       /* SEGMENT OVERRIDES */
		FLAG_OPS, LAHF, SAHF,                           /* FLAG OPERATIONS */
		AAA, AAS, AAM, AAD,                             /* ARITHMETIC ADJUSTS */
		DAA, DAS,                                       /* DECIMAL ADJUSTS */
		CBW, CWD,                                       /* SIGN EXTENSION */
		HLT, LOAD_PTR, LEA, NOP, WAIT, XLAT,            /* MISC */

		JMP_SHORT, JMP_NEAR, JMP_FAR,                   /* DIRECT jmpS */
		JMP_R16, JMP_M16, JMP_M32,                      /* INDIRECT jmpS */
		CALL_NEAR, CALL_FAR,                            /* DIRECT callS */
		CALL_R16, CALL_M16, CALL_M32,                   /* INDIRECT callS */
		RET_NEAR, RET_FAR, RET_NEAR_IMM, RET_FAR_IMM,   /* RETURNS */
		JCC_NT, JCC_T, JCXZ_NT, JCXZ_T,                 /* CONDITIONAL jmpS */
		LOOP_NT, LOOP_T, LOOPE_NT, LOOPE_T,             /* LOOPS */

		IN_IMM8, IN_IMM16, IN_DX8, IN_DX16,             /* PORT READS */
		OUT_IMM8, OUT_IMM16, OUT_DX8, OUT_DX16,         /* PORT WRITES */

		MOV_RR8, MOV_RM8, MOV_MR8,                      /* MOVE, 8-BIT */
		MOV_RI8, MOV_MI8,                               /* MOVE, 8-BIT IMMEDIATE */
		MOV_RR16, MOV_RM16, MOV_MR16,                   /* MOVE, 16-BIT */
		MOV_RI16, MOV_MI16,                             /* MOVE, 16-BIT IMMEDIATE */
		MOV_AM8, MOV_AM16, MOV_MA8, MOV_MA16,           /* MOVE, al/ax MEMORY */
		MOV_SR, MOV_SM, MOV_RS, MOV_MS,                 /* MOVE, SEGMENT REGISTERS */
		XCHG_RR8, XCHG_RM8,                             /* EXCHANGE, 8-BIT */
		XCHG_RR16, XCHG_RM16, XCHG_AR16,                /* EXCHANGE, 16-BIT */

		PUSH_R16, PUSH_M16, PUSH_SEG, PUSHF,            /* PUSHES */
		POP_R16, POP_M16, POP_SEG, POPF,                /* POPS */

		ALU_RR8, ALU_RM8, ALU_MR8,                      /* alu OPS, 8-BIT */
		ALU_RI8, ALU_MI8, ALU_MI8_RO,                   /* alu OPS, 8-BIT IMMEDIATE */
		ALU_RR16, ALU_RM16, ALU_MR16,                   /* alu OPS, 16-BIT */
		ALU_RI16, ALU_MI16, ALU_MI16_RO,                /* alu OPS, 16-BIT IMMEDIATE */
		ALU_R16I8, ALU_M16I8, ALU_M16I8_RO,             /* alu OPS, 16-BIT W/8-BIT IMMEDIATE */
		MUL_R8, MUL_R16, MUL_M8, MUL_M16,               /* mul */
		IMUL_R8, IMUL_R16, IMUL_M8, IMUL_M16,           /* imul */
		DIV_R8, DIV_R16, DIV_M8, DIV_M16,               /* div */
		IDIV_R8, IDIV_R16, IDIV_M8, IDIV_M16,           /* idiv */
		INCDEC_R8, INCDEC_R16, INCDEC_M8, INCDEC_M16,   /* inc/dec */
		NEGNOT_R8, NEGNOT_R16, NEGNOT_M8, NEGNOT_M16,   /* neg/not */

		ROT_REG_1, ROT_REG_BASE, ROT_REG_BIT,           /* REG SHIFT/ROTATE */
		ROT_M8_1, ROT_M8_BASE, ROT_M8_BIT,              /* M8 SHIFT/ROTATE */
		ROT_M16_1, ROT_M16_BASE, ROT_M16_BIT,           /* M16 SHIFT/ROTATE */

		CMPS8, REP_CMPS8_BASE, REP_CMPS8_COUNT,         /* cmps 8-BIT */
		CMPS16, REP_CMPS16_BASE, REP_CMPS16_COUNT,      /* cmps 16-BIT */
		SCAS8, REP_SCAS8_BASE, REP_SCAS8_COUNT,         /* scas 8-BIT */
		SCAS16, REP_SCAS16_BASE, REP_SCAS16_COUNT,      /* scas 16-BIT */
		LODS8, REP_LODS8_BASE, REP_LODS8_COUNT,         /* lods 8-BIT */
		LODS16, REP_LODS16_BASE, REP_LODS16_COUNT,      /* lods 16-BIT */
		STOS8, REP_STOS8_BASE, REP_STOS8_COUNT,         /* stos 8-BIT */
		STOS16, REP_STOS16_BASE, REP_STOS16_COUNT,      /* stos 16-BIT */
		MOVS8, REP_MOVS8_BASE, REP_MOVS8_COUNT,         /* movs 8-BIT */
		MOVS16, REP_MOVS16_BASE, REP_MOVS16_COUNT,      /* movs 16-BIT */

		INS8, REP_INS8_BASE, REP_INS8_COUNT,            /* (80186) ins 8-BIT */
		INS16, REP_INS16_BASE, REP_INS16_COUNT,         /* (80186) ins 16-BIT */
		OUTS8, REP_OUTS8_BASE, REP_OUTS8_COUNT,         /* (80186) outs 8-BIT */
		OUTS16, REP_OUTS16_BASE, REP_OUTS16_COUNT,      /* (80186) outs 16-BIT */
		PUSH_IMM, PUSHA, POPA,                          /* (80186) push IMMEDIATE, pusha/popa */
		IMUL_RRI8, IMUL_RMI8,                           /* (80186) imul IMMEDIATE 8-BIT */
		IMUL_RRI16, IMUL_RMI16,                         /* (80186) imul IMMEDIATE 16-BIT */
		ENTER0, ENTER1, ENTER_BASE, ENTER_COUNT, LEAVE, /* (80186) enter/leave */
		BOUND                                           /* (80186) bound */
	};

	enum SREGS { ES=0, CS, SS, DS };
	enum WREGS { AX=0, CX, DX, BX, SP, BP, SI, DI };

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 50; }
	virtual void execute_set_input(int inputnum, int state) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	virtual void interrupt(int int_num, int trap = 1);
	bool common_op(UINT8 op);

	// Accessing memory and io
	inline UINT8 read_byte(UINT32 addr);
	inline UINT16 read_word(UINT32 addr);
	inline void write_byte(UINT32 addr, UINT8 data);
	inline void write_word(UINT32 addr, UINT16 data);
	virtual UINT8 read_port_byte(UINT16 port);
	virtual UINT16 read_port_word(UINT16 port);
	virtual void write_port_byte(UINT16 port, UINT8 data);
	virtual void write_port_word(UINT16 port, UINT16 data);

	// Executing instructions
	virtual UINT8 fetch_op() = 0;
	virtual UINT8 fetch() = 0;
	inline UINT16 fetch_word();
	inline UINT8 repx_op();

	// Cycles passed while executing instructions
	inline void CLK(UINT8 op);
	inline void CLKM(UINT8 op_reg, UINT8 op_mem);

	// Memory handling while executing instructions
	virtual UINT32 calc_addr(int seg, UINT16 offset, int size, int op, bool override = true);
	inline UINT32 get_ea(int size, int op);
	inline void PutbackRMByte(UINT8 data);
	inline void PutbackRMWord(UINT16 data);
	inline void RegByte(UINT8 data);
	inline void RegWord(UINT16 data);
	inline UINT8 RegByte();
	inline UINT16 RegWord();
	inline UINT16 GetRMWord();
	inline UINT16 GetnextRMWord();
	inline UINT8 GetRMByte();
	inline void PutMemB(int seg, UINT16 offset, UINT8 data);
	inline void PutMemW(int seg, UINT16 offset, UINT16 data);
	inline UINT8 GetMemB(int seg, UINT16 offset);
	inline UINT16 GetMemW(int seg, UINT16 offset);
	inline void PutImmRMWord();
	inline void PutRMWord(UINT16 val);
	inline void PutRMByte(UINT8 val);
	inline void PutImmRMByte();
	inline void DEF_br8();
	inline void DEF_wr16();
	inline void DEF_r8b();
	inline void DEF_r16w();
	inline void DEF_ald8();
	inline void DEF_axd16();

	// Flags
	inline void set_CFB(UINT32 x);
	inline void set_CFW(UINT32 x);
	inline void set_AF(UINT32 x,UINT32 y,UINT32 z);
	inline void set_SF(UINT32 x);
	inline void set_ZF(UINT32 x);
	inline void set_PF(UINT32 x);
	inline void set_SZPF_Byte(UINT32 x);
	inline void set_SZPF_Word(UINT32 x);
	inline void set_OFW_Add(UINT32 x,UINT32 y,UINT32 z);
	inline void set_OFB_Add(UINT32 x,UINT32 y,UINT32 z);
	inline void set_OFW_Sub(UINT32 x,UINT32 y,UINT32 z);
	inline void set_OFB_Sub(UINT32 x,UINT32 y,UINT32 z);
	inline UINT16 CompressFlags() const;
	inline void ExpandFlags(UINT16 f);

	// rep instructions
	inline void i_insb();
	inline void i_insw();
	inline void i_outsb();
	inline void i_outsw();
	inline void i_movsb();
	inline void i_movsw();
	inline void i_cmpsb();
	inline void i_cmpsw();
	inline void i_stosb();
	inline void i_stosw();
	inline void i_lodsb();
	inline void i_lodsw();
	inline void i_scasb();
	inline void i_scasw();
	inline void i_popf();

	// sub implementations
	inline UINT32 ADDB();
	inline UINT32 ADDX();
	inline UINT32 SUBB();
	inline UINT32 SUBX();
	inline void ORB();
	inline void ORW();
	inline void ANDB();
	inline void ANDX();
	inline void XORB();
	inline void XORW();
	inline void ROL_BYTE();
	inline void ROL_WORD();
	inline void ROR_BYTE();
	inline void ROR_WORD();
	inline void ROLC_BYTE();
	inline void ROLC_WORD();
	inline void RORC_BYTE();
	inline void RORC_WORD();
	inline void SHL_BYTE(UINT8 c);
	inline void SHL_WORD(UINT8 c);
	inline void SHR_BYTE(UINT8 c);
	inline void SHR_WORD(UINT8 c);
	inline void SHRA_BYTE(UINT8 c);
	inline void SHRA_WORD(UINT8 c);
	inline void XchgAXReg(UINT8 reg);
	inline void IncWordReg(UINT8 reg);
	inline void DecWordReg(UINT8 reg);
	inline void PUSH(UINT16 data);
	inline UINT16 POP();
	inline void JMP(bool cond);
	inline void ADJ4(INT8 param1, INT8 param2);
	inline void ADJB(INT8 param1, INT8 param2);

protected:

	union
	{                   /* eight general registers */
		UINT16 w[8];    /* viewed as 16 bits registers */
		UINT8  b[16];   /* or as 8 bit registers */
	} m_regs;

	enum BREGS {
		AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
		AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
		CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
		CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
		DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
		DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
		BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
		BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6),
		SPL = NATIVE_ENDIAN_VALUE_LE_BE(0x8, 0x9),
		SPH = NATIVE_ENDIAN_VALUE_LE_BE(0x9, 0x8),
		BPL = NATIVE_ENDIAN_VALUE_LE_BE(0xa, 0xb),
		BPH = NATIVE_ENDIAN_VALUE_LE_BE(0xb, 0xa),
		SIL = NATIVE_ENDIAN_VALUE_LE_BE(0xc, 0xd),
		SIH = NATIVE_ENDIAN_VALUE_LE_BE(0xd, 0xc),
		DIL = NATIVE_ENDIAN_VALUE_LE_BE(0xe, 0xf),
		DIH = NATIVE_ENDIAN_VALUE_LE_BE(0xf, 0xe)
	};

	enum {
		I8086_READ,
		I8086_WRITE,
		I8086_FETCH,
		I8086_NONE
	};

	UINT16  m_sregs[4];

	UINT16  m_ip;
	UINT16  m_prev_ip;

	INT32   m_SignVal;
	UINT32  m_AuxVal, m_OverVal, m_ZeroVal, m_CarryVal, m_ParityVal; /* 0 or non-0 valued flags */
	UINT8   m_TF, m_IF, m_DF;     /* 0 or 1 valued flags */
	UINT8   m_IOPL, m_NT, m_MF;
	UINT32  m_int_vector;
	UINT32  m_pending_irq;
	UINT32  m_nmi_state;
	UINT32  m_irq_state;
	UINT8   m_no_interrupt;
	UINT8   m_fire_trap;
	UINT8   m_test_state;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;
	offs_t m_fetch_xor;
	int m_icount;

	UINT32 m_prefix_seg;   /* the latest prefix segment */
	bool m_seg_prefix;      /* prefix segment indicator */
	bool m_seg_prefix_next; /* prefix segment for next instruction */

	UINT32 m_ea;
	UINT16 m_eo;
	UINT16 m_e16;

	// Used during execution of instructions
	UINT8   m_modrm;
	UINT32  m_dst;
	UINT32  m_src;
	UINT32  m_pc;

	// Lookup tables
	UINT8 m_parity_table[256];
	struct {
		struct {
			int w[256];
			int b[256];
		} reg;
		struct {
			int w[256];
			int b[256];
		} RM;
	} m_Mod_RM;

	UINT8 m_timing[200];
	bool m_halt;

	bool m_lock;
	devcb_write_line m_lock_handler;
};

class i8086_cpu_device : public i8086_common_cpu_device
{
public:
	// construction/destruction
	i8086_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i8086_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int data_bus_size);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

protected:
	virtual void execute_run() override;
	virtual void device_start() override;
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual UINT8 fetch_op() override;
	virtual UINT8 fetch() override;
	UINT32 pc() { return m_pc = (m_sregs[CS] << 4) + m_ip; }

	address_space_config m_program_config;
	address_space_config m_io_config;
	static const UINT8 m_i8086_timing[200];
};

class i8088_cpu_device : public i8086_cpu_device
{
public:
	// construction/destruction
	i8088_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


#endif /* __I8086_H__ */
