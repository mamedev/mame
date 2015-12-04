// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                 Texas Instruments TMS32010 DSP Emulator                  *
	*                                                                          *
	*                  Copyright Tony La Porta                                 *
	*      You are not allowed to distribute this software commercially.       *
	*                      Written for the MAME project.                       *
	*                                                                          *
	*                                                                          *
	*      Note :  This is a word based microcontroller, with addressing       *
	*              architecture based on the Harvard addressing scheme.        *
	*                                                                          *
	\**************************************************************************/

#pragma once

#ifndef __TMS32010_H__
#define __TMS32010_H__




/****************************************************************************
 * Use this in the I/O port address fields of your driver for the BIO pin
 * i.e,
 *  AM_RANGE(TMS32010_BIO, TMS32010_BIO) AM_READ(twincobr_bio_line_r)
 */

#define TMS32010_BIO            0x10        /* BIO input */


#define TMS32010_INT_PENDING    0x80000000
#define TMS32010_INT_NONE       0


enum
{
	TMS32010_PC=1, TMS32010_SP,   TMS32010_STR,  TMS32010_ACC,
	TMS32010_PREG, TMS32010_TREG, TMS32010_AR0,  TMS32010_AR1,
	TMS32010_STK0, TMS32010_STK1, TMS32010_STK2, TMS32010_STK3
};


/****************************************************************************
 *  Public Functions
 */


class tms32010_device : public cpu_device
{
public:
	// construction/destruction
	tms32010_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms32010_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int addr_mask);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 3; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const { return (clocks + 4 - 1) / 4; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const { return (cycles * 4); }

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ) ); }

	// device_state_interface overrides
	void state_string_export(const device_state_entry &entry, std::string &str);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 4; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	typedef void ( tms32010_device::*opcode_func ) ();
	struct tms32010_opcode
	{
		UINT8       cycles;
		opcode_func function;
	};
	static const tms32010_opcode s_opcode_main[256];
	static const tms32010_opcode s_opcode_7F[32];

	/******************** CPU Internal Registers *******************/
	UINT16  m_PC;
	UINT16  m_PREVPC;     /* previous program counter */
	UINT16  m_STR;
	PAIR    m_ACC;
	PAIR    m_ALU;
	PAIR    m_Preg;
	UINT16  m_Treg;
	UINT16  m_AR[2];
	UINT16  m_STACK[4];

	PAIR    m_opcode;
	int     m_INTF;       /* Pending Interrupt flag */
	int     m_icount;
	PAIR    m_oldacc;
	UINT16  m_memaccess;
	int     m_addr_mask;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	inline void CLR(UINT16 flag);
	inline void SET_FLAG(UINT16 flag);
	inline void CALCULATE_ADD_OVERFLOW(INT32 addval);
	inline void CALCULATE_SUB_OVERFLOW(INT32 subval);
	inline UINT16 POP_STACK();
	inline void PUSH_STACK(UINT16 data);
	inline void UPDATE_AR();
	inline void UPDATE_ARP();
	inline void getdata(UINT8 shift,UINT8 signext);
	inline void putdata(UINT16 data);
	inline void putdata_sar(UINT8 data);
	inline void putdata_sst(UINT16 data);
	void opcodes_7F();
	void illegal();
	void abst();
	void add_sh();
	void addh();
	void adds();
	void and_();
	void apac();
	void br();
	void banz();
	void bgez();
	void bgz();
	void bioz();
	void blez();
	void blz();
	void bnz();
	void bv();
	void bz();
	void cala();
	void call();
	void dint();
	void dmov();
	void eint();
	void in_p();
	void lac_sh();
	void lack();
	void lar_ar0();
	void lar_ar1();
	void lark_ar0();
	void lark_ar1();
	void larp_mar();
	void ldp();
	void ldpk();
	void lst();
	void lt();
	void lta();
	void ltd();
	void mpy();
	void mpyk();
	void nop();
	void or_();
	void out_p();
	void pac();
	void pop();
	void push();
	void ret();
	void rovm();
	void sach_sh();
	void sacl();
	void sar_ar0();
	void sar_ar1();
	void sovm();
	void spac();
	void sst();
	void sub_sh();
	void subc();
	void subh();
	void subs();
	void tblr();
	void tblw();
	void xor_();
	void zac();
	void zalh();
	void zals();
	inline int add_branch_cycle();
	int Ext_IRQ();

};


class tms32015_device : public tms32010_device
{
public:
	// construction/destruction
	tms32015_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms32016_device : public tms32010_device
{
public:
	// construction/destruction
	tms32016_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type TMS32010;
extern const device_type TMS32015;
extern const device_type TMS32016;


#endif  /* __TMS32010_H__ */
