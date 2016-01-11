// license:BSD-3-Clause
// copyright-holders:Tony La Porta
	/**************************************************************************\
	*                  Microchip PIC16C62X Emulator                            *
	*                                                                          *
	*                          Based On                                        *
	*                  Microchip PIC16C5X Emulator                             *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	\**************************************************************************/

#pragma once

#ifndef __PIC16C62X_H__
#define __PIC16C62X_H__




/**************************************************************************
 *  Internal Clock divisor
 *
 *  External Clock is divided internally by 4 for the instruction cycle
 *  times. (Each instruction cycle passes through 4 machine states). This
 *  is handled by the cpu execution engine.
 */

enum
{
	PIC16C62x_PC=1, PIC16C62x_STK0, PIC16C62x_STK1, PIC16C62x_STK2,
	PIC16C62x_STK3, PIC16C62x_STK4, PIC16C62x_STK5, PIC16C62x_STK6,
	PIC16C62x_STK7, PIC16C62x_FSR,  PIC16C62x_W,    PIC16C62x_ALU,
	PIC16C62x_STR,  PIC16C62x_OPT,  PIC16C62x_TMR0, PIC16C62x_PRTA,
	PIC16C62x_PRTB, PIC16C62x_WDT,  PIC16C62x_TRSA, PIC16C62x_TRSB,
	PIC16C62x_PSCL
};

#define PIC16C62x_T0        0


extern const device_type PIC16C620;
extern const device_type PIC16C620A;
//extern const device_type PIC16CR620A;
extern const device_type PIC16C621;
extern const device_type PIC16C621A;
extern const device_type PIC16C622;
extern const device_type PIC16C622A;


class pic16c62x_device : public cpu_device
{
public:
	// construction/destruction
	pic16c62x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, int program_width, int picmodel);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks + 4 - 1) / 4; }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 4); }
	virtual UINT32 execute_min_cycles() const override { return 1; }
	virtual UINT32 execute_max_cycles() const override { return 2; }
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual UINT32 execute_default_irq_vector() const override { return 0; }
	virtual void execute_run() override;

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override
	{
		return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : ( (spacenum == AS_DATA) ? &m_data_config : nullptr ) );
	}

	// device_state_interface overrides
	virtual void state_import(const device_state_entry &entry) override;
	virtual void state_export(const device_state_entry &entry) override;
	virtual void state_string_export(const device_state_entry &entry, std::string &str) override;

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const override { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const override { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options) override;

private:
	address_space_config m_program_config;
	address_space_config m_data_config;
	address_space_config m_io_config;

	/******************** CPU Internal Registers *******************/
	UINT16  m_PC;
	UINT16  m_PREVPC;     /* previous program counter */
	UINT8   m_W;
	UINT8   m_PCLATH;     /* 0a,8a */
	UINT8   m_OPTION;     /* 81 */
	UINT16  m_CONFIG;
	UINT8   m_ALU;
	UINT16  m_WDT;
	UINT8   m_TRISA;      /* 85 */
	UINT8   m_TRISB;      /* 86 */
	UINT16  m_STACK[8];
	UINT16  m_prescaler;  /* Note: this is really an 8-bit register */
	PAIR    m_opcode;
	UINT8   *m_internalram;

	int     m_icount;
	int     m_reset_vector;
	int     m_picmodel;
	int     m_delay_timer;
	UINT16  m_temp_config;
	UINT8   m_old_T0;
	INT8    m_old_data;
	UINT8   m_picRAMmask;
	int     m_inst_cycles;

	address_space *m_program;
	direct_read_data *m_direct;
	address_space *m_data;
	address_space *m_io;

	// For debugger
	int m_debugger_temp;

	/* opcode table entry */
	typedef void (pic16c62x_device::*pic16c62x_ophandler)();
	struct pic16c62x_opcode
	{
		UINT8   cycles;
		pic16c62x_ophandler function;
	};
	pic16c62x_opcode m_opcode_table[16384];

	/* instruction list entry */
	struct pic16c62x_instruction
	{
		char    *format;
		pic16c62x_ophandler function;
		UINT8   cycles;
	};
	static const pic16c62x_instruction s_instructiontable[];

	void update_internalram_ptr();
	void CALCULATE_Z_FLAG();
	void CALCULATE_ADD_CARRY();
	void CALCULATE_ADD_DIGITCARRY();
	void CALCULATE_SUB_CARRY();
	void CALCULATE_SUB_DIGITCARRY();
	UINT16 POP_STACK();
	void PUSH_STACK(UINT16 data);
	UINT8 GET_REGFILE(offs_t addr);
	void STORE_REGFILE(offs_t addr, UINT8 data);
	void STORE_RESULT(offs_t addr, UINT8 data);
	void illegal();
	void addwf();
	void addlw();
	void andwf();
	void andlw();
	void bcf();
	void bsf();
	void btfss();
	void btfsc();
	void call();
	void clrw();
	void clrf();
	void clrwdt();
	void comf();
	void decf();
	void decfsz();
	void goto_op();
	void incf();
	void incfsz();
	void iorlw();
	void iorwf();
	void movf();
	void movlw();
	void movwf();
	void nop();
	void option();
	void retlw();
	void returns();
	void retfie();
	void rlf();
	void rrf();
	void sleepic();
	void subwf();
	void sublw();
	void swapf();
	void tris();
	void xorlw();
	void xorwf();
	void build_opcode_table(void);
	void pic16c62x_reset_regs();
	void pic16c62x_soft_reset();
	void pic16c62x_set_config(int data);
	void pic16c62x_update_watchdog(int counts);
	void pic16c62x_update_timer(int counts);

};


class pic16c620_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c620_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class pic16c620a_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c620a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

/*
class pic16cr620a_device : public pic16c62x_device
{
public:
    // construction/destruction
    pic16cr620a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
}*/

class pic16c621_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c621_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class pic16c621a_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c621a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class pic16c622_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c622_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class pic16c622a_device : public pic16c62x_device
{
public:
	// construction/destruction
	pic16c622a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


#endif  /* __PIC16C62X_H__ */
