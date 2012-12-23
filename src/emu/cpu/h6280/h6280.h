/*****************************************************************************

    h6280.h Portable Hu6280 emulator interface

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.

******************************************************************************/

#pragma once

#ifndef __H6280_H__
#define __H6280_H__

#define LAZY_FLAGS  0

/***************************************************************************
    REGISTER ENUMERATION
***************************************************************************/

enum
{
	H6280_PC = 1,
	H6280_S,
	H6280_P,
	H6280_A,
	H6280_X,
	H6280_Y,
	H6280_IRQ_MASK,
	H6280_TIMER_STATE,
	H6280_NMI_STATE,
	H6280_IRQ1_STATE,
	H6280_IRQ2_STATE,
	H6280_IRQT_STATE,
	H6280_M1,
	H6280_M2,
	H6280_M3,
	H6280_M4,
	H6280_M5,
	H6280_M6,
	H6280_M7,
	H6280_M8
};

#define H6280_RESET_VEC	0xfffe
#define H6280_NMI_VEC	0xfffc
#define H6280_TIMER_VEC	0xfffa
#define H6280_IRQ1_VEC	0xfff8
#define H6280_IRQ2_VEC	0xfff6			/* Aka BRK vector */


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> h6280_device

// Used by core CPU interface
class h6280_device : public cpu_device
{
public:
	// construction/destruction
	h6280_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// public interfaces
	void set_irq_line(int irqline, int state);

	DECLARE_READ8_MEMBER( irq_status_r );
	DECLARE_WRITE8_MEMBER( irq_status_w );

	DECLARE_READ8_MEMBER( timer_r );
	DECLARE_WRITE8_MEMBER( timer_w );

	/* functions for use by the PSG and joypad port only! */
	UINT8 io_get_buffer();
	void io_set_buffer(UINT8);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_stop();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;
	virtual UINT32 execute_input_lines() const;
	virtual void execute_run();
	virtual void execute_set_input(int inputnum, int state);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	virtual bool memory_translate(address_spacenum spacenum, int intention, offs_t &address);

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const;
	virtual UINT32 disasm_max_opcode_bytes() const;
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);

	// device_state_interface overrides
	virtual void state_string_export(const device_state_entry &entry, astring &string);

	// opcode accessors
	UINT8 program_read8(offs_t addr);
	void program_write8(offs_t addr, UINT8 data);
	UINT8 program_read8z(offs_t addr);
	void program_write8z(offs_t addr, UINT8 data);
	UINT16 program_read16(offs_t addr);
	UINT16 program_read16z(offs_t addr);
	void push(UINT8 value);
	void pull(UINT8 &value);
	UINT8 read_opcode();
	UINT8 read_opcode_arg();

	// include the macros
	#include "h6280ops.h"

	// include the opcode macros and functions
	#include "tblh6280.c"

	// address spaces
	const address_space_config m_program_config;
	const address_space_config m_io_config;

	// CPU registers
	PAIR  m_ppc;			/* previous program counter */
    PAIR  m_pc;           	/* program counter */
    PAIR  m_sp;           	/* stack pointer (always 100 - 1FF) */
    PAIR  m_zp;           	/* zero page address */
    PAIR  m_ea;           	/* effective address */
    UINT8 m_a;            	/* Accumulator */
    UINT8 m_x;            	/* X index register */
    UINT8 m_y;            	/* Y index register */
    UINT8 m_p;            	/* Processor status */
    UINT8 m_mmr[8];       	/* Hu6280 memory mapper registers */
    UINT8 m_irq_mask;     	/* interrupt enable/disable */
    UINT8 m_timer_status; 	/* timer status */
	UINT8 m_timer_ack;		/* timer acknowledge */
    UINT8 m_clocks_per_cycle; /* 4 = low speed mode, 1 = high speed mode */
    INT32 m_timer_value;    /* timer interrupt */
    INT32 m_timer_load;		/* reload value */
    UINT8 m_nmi_state;
    UINT8 m_irq_state[3];
	UINT8 m_irq_pending;
#if LAZY_FLAGS
    INT32 m_nz;			/* last value (lazy N and Z flag) */
#endif
	UINT8 m_io_buffer;	/* last value written to the PSG, timer, and interrupt pages */

	// other internal states
    int m_icount;

	// address spaces
    address_space *m_program;
    address_space *m_io;
	direct_read_data *m_direct;

	typedef void (h6280_device::*ophandler)();

	ophandler m_opcode[256];

	static const ophandler s_opcodetable[256];
};

extern const device_type H6280;


CPU_DISASSEMBLE( h6280 );

#endif /* __H6280_H__ */
