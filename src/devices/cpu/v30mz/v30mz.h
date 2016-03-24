// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Bryan McPhail
#ifndef __V30MZ_H__
#define __V30MZ_H__


struct nec_config
{
	const UINT8*    v25v35_decryptiontable; // internal decryption table
};

enum
{
	NEC_PC=0,
	NEC_IP, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_ES, NEC_CS, NEC_SS, NEC_DS,
	NEC_VECTOR, NEC_PENDING
};


/////////////////////////////////////////////////////////////////

extern const device_type V30MZ;

class v30mz_cpu_device : public cpu_device
{
public:
	// construction/destruction
	v30mz_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 80; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 7; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

	void interrupt(int int_num);

	inline UINT32 pc();
	// Accessing memory and io
	inline UINT8 read_byte(UINT32 addr);
	inline UINT16 read_word(UINT32 addr);
	inline void write_byte(UINT32 addr, UINT8 data);
	inline void write_word(UINT32 addr, UINT16 data);
	inline UINT8 read_port(UINT16 port);
	inline void write_port(UINT16 port, UINT8 data);

	// Executing instructions
	inline UINT8 fetch_op();
	inline UINT8 fetch();
	inline UINT16 fetch_word();
	inline UINT8 repx_op();

	// Cycles passed while executing instructions
	inline void CLK(UINT32 cycles);
	inline void CLKM(UINT32 cycles_reg, UINT32 cycles_mem);

	// Memory handling while executing instructions
	inline UINT32 default_base(int seg);
	inline UINT32 get_ea();
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
	inline void ADDB();
	inline void ADDW();
	inline void SUBB();
	inline void SUBW();
	inline void ORB();
	inline void ORW();
	inline void ANDB();
	inline void ANDW();
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
	inline void XchgAWReg(UINT8 reg);
	inline void IncWordReg(UINT8 reg);
	inline void DecWordReg(UINT8 reg);
	inline void PUSH(UINT16 data);
	inline UINT16 POP();
	inline void JMP(bool cond);
	inline void ADJ4(INT8 param1, INT8 param2);
	inline void ADJB(INT8 param1, INT8 param2);

protected:
	address_space_config m_program_config;
	address_space_config m_io_config;

	union
	{                   /* eight general registers */
		UINT16 w[8];    /* viewed as 16 bits registers */
		UINT8  b[16];   /* or as 8 bit registers */
	} m_regs;
	UINT16  m_sregs[4];

	UINT16  m_ip;

	INT32   m_SignVal;
	UINT32  m_AuxVal, m_OverVal, m_ZeroVal, m_CarryVal, m_ParityVal; /* 0 or non-0 valued flags */
	UINT8   m_TF, m_IF, m_DF, m_MF;     /* 0 or 1 valued flags */   /* OB[19.07.99] added Mode Flag V30 */
	UINT32  m_int_vector;
	UINT32  m_pending_irq;
	UINT32  m_nmi_state;
	UINT32  m_irq_state;
	UINT8   m_no_interrupt;
	UINT8   m_fire_trap;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_io;
	int m_icount;

	UINT32 m_prefix_base;   /* base address of the latest prefix segment */
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
};


#endif /* __V30MZ_H__ */
