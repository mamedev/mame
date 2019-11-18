// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Bryan McPhail
#ifndef MAME_CPU_V32MZ_V30MZ_H
#define MAME_CPU_V32MZ_V30MZ_H


struct nec_config
{
	const uint8_t*    v25v35_decryptiontable; // internal decryption table
};

enum
{
	NEC_PC=0,
	NEC_IP, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_ES, NEC_CS, NEC_SS, NEC_DS,
	NEC_VECTOR, NEC_PENDING
};


/////////////////////////////////////////////////////////////////

DECLARE_DEVICE_TYPE(V30MZ, v30mz_cpu_device)

class v30mz_cpu_device : public cpu_device
{
public:
	// construction/destruction
	v30mz_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual uint32_t execute_min_cycles() const noexcept override { return 1; }
	virtual uint32_t execute_max_cycles() const noexcept override { return 80; }
	virtual uint32_t execute_input_lines() const noexcept override { return 1; }
	virtual bool execute_input_edge_triggered(int inputnum) const noexcept override { return inputnum == INPUT_LINE_NMI; }
	virtual void execute_run() override;
	virtual void execute_set_input(int inputnum, int state) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, std::string &str) const override;

	// device_disasm_interface overrides
	virtual std::unique_ptr<util::disasm_interface> create_disassembler() override;

	void interrupt(int int_num);

	inline uint32_t pc();
	// Accessing memory and io
	inline uint8_t read_byte(uint32_t addr);
	inline uint16_t read_word(uint32_t addr);
	inline void write_byte(uint32_t addr, uint8_t data);
	inline void write_word(uint32_t addr, uint16_t data);
	inline uint8_t read_port(uint16_t port);
	inline void write_port(uint16_t port, uint8_t data);

	// Executing instructions
	inline uint8_t fetch_op();
	inline uint8_t fetch();
	inline uint16_t fetch_word();
	inline uint8_t repx_op();

	// Cycles passed while executing instructions
	inline void CLK(uint32_t cycles);
	inline void CLKM(uint32_t cycles_reg, uint32_t cycles_mem);

	// Memory handling while executing instructions
	inline uint32_t default_base(int seg);
	inline uint32_t get_ea();
	inline void PutbackRMByte(uint8_t data);
	inline void PutbackRMWord(uint16_t data);
	inline void RegByte(uint8_t data);
	inline void RegWord(uint16_t data);
	inline uint8_t RegByte();
	inline uint16_t RegWord();
	inline uint16_t GetRMWord();
	inline uint16_t GetnextRMWord();
	inline uint8_t GetRMByte();
	inline void PutMemB(int seg, uint16_t offset, uint8_t data);
	inline void PutMemW(int seg, uint16_t offset, uint16_t data);
	inline uint8_t GetMemB(int seg, uint16_t offset);
	inline uint16_t GetMemW(int seg, uint16_t offset);
	inline void PutImmRMWord();
	inline void PutRMWord(uint16_t val);
	inline void PutRMByte(uint8_t val);
	inline void PutImmRMByte();
	inline void DEF_br8();
	inline void DEF_wr16();
	inline void DEF_r8b();
	inline void DEF_r16w();
	inline void DEF_ald8();
	inline void DEF_axd16();

	// Flags
	inline void set_CFB(uint32_t x);
	inline void set_CFW(uint32_t x);
	inline void set_AF(uint32_t x,uint32_t y,uint32_t z);
	inline void set_SF(uint32_t x);
	inline void set_ZF(uint32_t x);
	inline void set_PF(uint32_t x);
	inline void set_SZPF_Byte(uint32_t x);
	inline void set_SZPF_Word(uint32_t x);
	inline void set_OFW_Add(uint32_t x,uint32_t y,uint32_t z);
	inline void set_OFB_Add(uint32_t x,uint32_t y,uint32_t z);
	inline void set_OFW_Sub(uint32_t x,uint32_t y,uint32_t z);
	inline void set_OFB_Sub(uint32_t x,uint32_t y,uint32_t z);
	inline uint16_t CompressFlags() const;
	inline void ExpandFlags(uint16_t f);

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
	inline void SHL_BYTE(uint8_t c);
	inline void SHL_WORD(uint8_t c);
	inline void SHR_BYTE(uint8_t c);
	inline void SHR_WORD(uint8_t c);
	inline void SHRA_BYTE(uint8_t c);
	inline void SHRA_WORD(uint8_t c);
	inline void XchgAWReg(uint8_t reg);
	inline void IncWordReg(uint8_t reg);
	inline void DecWordReg(uint8_t reg);
	inline void PUSH(uint16_t data);
	inline uint16_t POP();
	inline void JMP(bool cond);
	inline void ADJ4(int8_t param1, int8_t param2);
	inline void ADJB(int8_t param1, int8_t param2);

	address_space_config m_program_config;
	address_space_config m_io_config;

	union
	{                   /* eight general registers */
		uint16_t w[8];    /* viewed as 16 bits registers */
		uint8_t  b[16];   /* or as 8 bit registers */
	} m_regs;
	uint16_t  m_sregs[4];

	uint16_t  m_ip;

	int32_t   m_SignVal;
	uint32_t  m_AuxVal, m_OverVal, m_ZeroVal, m_CarryVal, m_ParityVal; /* 0 or non-0 valued flags */
	uint8_t   m_TF, m_IF, m_DF, m_MF;     /* 0 or 1 valued flags */   /* OB[19.07.99] added Mode Flag V30 */
	uint32_t  m_int_vector;
	uint32_t  m_pending_irq;
	uint32_t  m_nmi_state;
	uint32_t  m_irq_state;
	uint8_t   m_no_interrupt;
	uint8_t   m_fire_trap;

	address_space *m_program;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_cache;
	address_space *m_io;
	int m_icount;

	uint32_t m_prefix_base;   /* base address of the latest prefix segment */
	bool m_seg_prefix;      /* prefix segment indicator */
	bool m_seg_prefix_next; /* prefix segment for next instruction */

	uint32_t m_ea;
	uint16_t m_eo;
	uint16_t m_e16;

	// Used during execution of instructions
	uint8_t   m_modrm;
	uint32_t  m_dst;
	uint32_t  m_src;
	uint32_t  m_pc;

	// Lookup tables
	uint8_t m_parity_table[256];
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

#endif // MAME_CPU_V32MZ_V30MZ_H
