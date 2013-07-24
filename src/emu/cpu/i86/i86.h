#ifndef __I8086_H__
#define __I8086_H__

#include "emu.h"

/////////////////////////////////////////////////////////////////

extern const device_type I8086;
extern const device_type I8088;
extern const device_type I80186;
extern const device_type I80188;

#define INPUT_LINE_INT0         INPUT_LINE_IRQ0
#define INPUT_LINE_INT1         INPUT_LINE_IRQ1
#define INPUT_LINE_INT2         INPUT_LINE_IRQ2
#define INPUT_LINE_INT3         INPUT_LINE_IRQ3
#define INPUT_LINE_TEST         20    /* PJB 03/05 */
#define INPUT_LINE_DRQ0         21
#define INPUT_LINE_DRQ1         22
#define INPUT_LINE_TMRIN0       23
#define INPUT_LINE_TMRIN1       24

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

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 50; }
	virtual UINT32 execute_input_lines() const { return 2; }
	virtual void execute_set_input(int inputnum, int state);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 8; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	void interrupt(int int_num);
	bool common_op(UINT8 op);

	inline UINT32 pc();
	// Accessing memory and io
	inline UINT8 read_byte(UINT32 addr);
	inline UINT16 read_word(UINT32 addr);
	inline void write_byte(UINT32 addr, UINT8 data);
	inline void write_word(UINT32 addr, UINT16 data);
	inline UINT8 read_port_byte(UINT16 port);
	inline UINT16 read_port_word(UINT16 port);
	inline void write_port_byte(UINT16 port, UINT8 data);
	inline void write_port_word(UINT16 port, UINT16 data);

	// Executing instructions
	inline UINT8 fetch_op();
	inline UINT8 fetch();
	inline UINT16 fetch_word();
	inline UINT8 repx_op();

	// Cycles passed while executing instructions
	inline void CLK(UINT8 op);
	inline void CLKM(UINT8 op_reg, UINT8 op_mem);

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
	inline UINT16 CompressFlags();
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
	inline void ADDX();
	inline void SUBB();
	inline void SUBX();
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
	UINT16  m_sregs[4];

	UINT16  m_ip;
	UINT16  m_prev_ip;

	INT32   m_SignVal;
	UINT32  m_AuxVal, m_OverVal, m_ZeroVal, m_CarryVal, m_ParityVal; /* 0 or non-0 valued flags */
	UINT8   m_TF, m_IF, m_DF;     /* 0 or 1 valued flags */
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

	UINT8 m_timing[200];
};

class i8086_cpu_device : public i8086_common_cpu_device
{
public:
	// construction/destruction
	i8086_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i8086_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int data_bus_size);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : NULL ); }

protected:
	virtual void execute_run();

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

class i80186_cpu_device : public i8086_common_cpu_device
{
public:
	// construction/destruction
	i80186_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i80186_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int data_bus_size);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : NULL ); }

protected:
	virtual void execute_run();

	address_space_config m_program_config;
	address_space_config m_io_config;
	static const UINT8 m_i80186_timing[200];
};

class i80188_cpu_device : public i80186_cpu_device
{
public:
	// construction/destruction
	i80188_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


#endif /* __I8086_H__ */
