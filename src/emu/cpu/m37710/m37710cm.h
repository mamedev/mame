#pragma once

#ifndef __M37710CM_H__
#define __M37710CM_H__

#define m37710i_branching(A)
#define m37710i_jumping(A)


/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include <limits.h>
#include "m37710.h"


/* ======================================================================== */
/* ================================= MAME ================================= */
/* ======================================================================== */

#undef M37710_CALL_DEBUGGER

#define M37710_CALL_DEBUGGER(x)         debugger_instruction_hook(cpustate->device, x)
#define m37710_read_8(addr)             cpustate->program->read_byte(addr)
#define m37710_write_8(addr,data)       cpustate->program->write_byte(addr,data)
#define m37710_read_8_immediate(A)      cpustate->program->read_byte(A)
#define m37710_read_16(addr)            cpustate->program->read_word(addr)
#define m37710_write_16(addr,data)      cpustate->program->write_word(addr,data)
#define m37710_jumping(A)
#define m37710_branching(A)


/* ======================================================================== */
/* ================================ GENERAL =============================== */
/* ======================================================================== */

/* This should be set to the default size of your processor (min 16 bit) */
#undef uint
#define uint unsigned int

#undef uint8
#define uint8 unsigned char

#undef int8

/* Allow for architectures that don't have 8-bit sizes */
#if UCHAR_MAX == 0xff
#define int8 char
#define MAKE_INT_8(A) (int8)((A)&0xff)
#else
#define int8   int
INLINE int MAKE_INT_8(int A) {return (A & 0x80) ? A | ~0xff : A & 0xff;}
#endif /* UCHAR_MAX == 0xff */

#define MAKE_UINT_8(A) ((A)&0xff)
#define MAKE_UINT_16(A) ((A)&0xffff)
#define MAKE_UINT_24(A) ((A)&0xffffff)

/* Bits */
#define BIT_0       0x01
#define BIT_1       0x02
#define BIT_2       0x04
#define BIT_3       0x08
#define BIT_4       0x10
#define BIT_5       0x20
#define BIT_6       0x40
#define BIT_7       0x80

/* ======================================================================== */
/* ================================== CPU ================================= */
/* ======================================================================== */

/* CPU Structure */
struct m37710i_cpu_struct
{
	uint a;         /* Accumulator */
	uint b;         /* holds high byte of accumulator */
	uint ba;        /* Secondary Accumulator */
	uint bb;        /* holds high byte of secondary accumulator */
	uint x;         /* Index Register X */
	uint y;         /* Index Register Y */
	uint xh;        /* holds high byte of x */
	uint yh;        /* holds high byte of y */
	uint s;         /* Stack Pointer */
	uint pc;        /* Program Counter */
	uint ppc;       /* Previous Program Counter */
	uint pb;        /* Program Bank (shifted left 16) */
	uint db;        /* Data Bank (shifted left 16) */
	uint d;         /* Direct Register */
	uint flag_e;        /* Emulation Mode Flag */
	uint flag_m;        /* Memory/Accumulator Select Flag */
	uint flag_x;        /* Index Select Flag */
	uint flag_n;        /* Negative Flag */
	uint flag_v;        /* Overflow Flag */
	uint flag_d;        /* Decimal Mode Flag */
	uint flag_i;        /* Interrupt Mask Flag */
	uint flag_z;        /* Zero Flag (inverted) */
	uint flag_c;        /* Carry Flag */
	uint line_irq;      /* Bitmask of pending IRQs */
	uint ipl;       /* Interrupt priority level (top of PSW) */
	uint ir;        /* Instruction Register */
	uint im;        /* Immediate load value */
	uint im2;       /* Immediate load target */
	uint im3;       /* Immediate load target */
	uint im4;       /* Immediate load target */
	uint irq_delay;     /* delay 1 instruction before checking irq */
	uint irq_level;     /* irq level */
	int ICount;     /* cycle count */
	uint source;        /* temp register */
	uint destination;   /* temp register */
	device_irq_acknowledge_callback int_ack;
	legacy_cpu_device *device;
	address_space *program;
	address_space *io;
	uint stopped;       /* Sets how the CPU is stopped */
	void (*const *opcodes)(m37710i_cpu_struct *cpustate);       /* opcodes with no prefix */
	void (*const *opcodes42)(m37710i_cpu_struct *cpustate); /* opcodes with 0x42 prefix */
	void (*const *opcodes89)(m37710i_cpu_struct *cpustate); /* opcodes with 0x89 prefix */
	uint (*get_reg)(m37710i_cpu_struct *cpustate, int regnum);
	void (*set_reg)(m37710i_cpu_struct *cpustate, int regnum, uint val);
	void (*set_line)(m37710i_cpu_struct *cpustate, int line, int state);
	int  (*execute)(m37710i_cpu_struct *cpustate, int cycles);

	// on-board peripheral stuff
	UINT8 m37710_regs[128];
	attotime reload[8];
	emu_timer *timers[8];
};

INLINE m37710i_cpu_struct *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == M37710 ||
			device->type() == M37702);
	return (m37710i_cpu_struct *)downcast<legacy_cpu_device *>(device)->token();
}

extern uint m37710i_adc_tbl[];
extern uint m37710i_sbc_tbl[];

extern void (*const *const m37710i_opcodes[])(m37710i_cpu_struct *cpustate);
extern void (*const *const m37710i_opcodes2[])(m37710i_cpu_struct *cpustate);
extern void (*const *const m37710i_opcodes3[])(m37710i_cpu_struct *cpustate);
extern uint (*const m37710i_get_reg[])(m37710i_cpu_struct *cpustate,int regnum);
extern void (*const m37710i_set_reg[])(m37710i_cpu_struct *cpustate,int regnum, uint val);
extern void (*const m37710i_set_line[])(m37710i_cpu_struct *cpustate,int line, int state);
extern int (*const m37710i_execute[])(m37710i_cpu_struct *cpustate, int cycles);

#define REG_A           cpustate->a     /* Accumulator */
#define REG_B           cpustate->b     /* Accumulator hi byte */
#define REG_BA          cpustate->ba        /* Secondary Accumulator */
#define REG_BB          cpustate->bb        /* Secondary Accumulator hi byte */
#define REG_X           cpustate->x     /* Index X Register */
#define REG_Y           cpustate->y     /* Index Y Register */
#define REG_XH          cpustate->xh        /* X high byte */
#define REG_YH          cpustate->yh        /* Y high byte */
#define REG_S           cpustate->s     /* Stack Pointer */
#define REG_PC          cpustate->pc        /* Program Counter */
#define REG_PPC         cpustate->ppc       /* Previous Program Counter */
#define REG_PB          cpustate->pb        /* Program Bank */
#define REG_DB          cpustate->db        /* Data Bank */
#define REG_D           cpustate->d     /* Direct Register */
#define FLAG_M          cpustate->flag_m    /* Memory/Accumulator Select Flag */
#define FLAG_X          cpustate->flag_x    /* Index Select Flag */
#define FLAG_N          cpustate->flag_n    /* Negative Flag */
#define FLAG_V          cpustate->flag_v    /* Overflow Flag */
#define FLAG_D          cpustate->flag_d    /* Decimal Mode Flag */
#define FLAG_I          cpustate->flag_i    /* Interrupt Mask Flag */
#define FLAG_Z          cpustate->flag_z    /* Zero Flag (inverted) */
#define FLAG_C          cpustate->flag_c    /* Carry Flag */
#define LINE_IRQ        cpustate->line_irq  /* Status of the IRQ line */
#define REG_IR          cpustate->ir        /* Instruction Register */
#define REG_IM          cpustate->im        /* Immediate load value */
#define REG_IM2         cpustate->im2       /* Immediate load target */
#define REG_IM3         cpustate->im3       /* Immediate load target */
#define REG_IM4         cpustate->im4       /* Immediate load target */
#define INT_ACK         cpustate->int_ack   /* Interrupt Acknowledge function pointer */
#define CLOCKS          cpustate->ICount        /* Clock cycles remaining */
#define IRQ_DELAY       cpustate->irq_delay /* Delay 1 instruction before checking IRQ */
#define CPU_STOPPED     cpustate->stopped   /* Stopped status of the CPU */

#define FTABLE_GET_REG  cpustate->get_reg
#define FTABLE_SET_REG  cpustate->set_reg
#define FTABLE_SET_LINE cpustate->set_line

#define SRC         cpustate->source        /* Source Operand */
#define DST         cpustate->destination   /* Destination Operand */

#define STOP_LEVEL_WAI  1
#define STOP_LEVEL_STOP 2

#define EXECUTION_MODE_M0X0 0
#define EXECUTION_MODE_M0X1 1
#define EXECUTION_MODE_M1X0 2
#define EXECUTION_MODE_M1X1 3

INLINE void m37710i_set_execution_mode(m37710i_cpu_struct *cpustate, uint mode)
{
	cpustate->opcodes = m37710i_opcodes[mode];
	cpustate->opcodes42 = m37710i_opcodes2[mode];
	cpustate->opcodes89 = m37710i_opcodes3[mode];
	FTABLE_GET_REG = m37710i_get_reg[mode];
	FTABLE_SET_REG = m37710i_set_reg[mode];
	FTABLE_SET_LINE = m37710i_set_line[mode];
	cpustate->execute = m37710i_execute[mode];
}

/* ======================================================================== */
/* ================================= CLOCK ================================ */
/* ======================================================================== */

#define CLK_OP          1
#define CLK_R8          1
#define CLK_R16         2
#define CLK_R24         3
#define CLK_W8          1
#define CLK_W16         2
#define CLK_W24         3
#define CLK_RMW8        3
#define CLK_RMW16       5

#define CLK_IMPLIED     1
#define CLK_IMPLIED     1
#define CLK_RELATIVE_8  1
#define CLK_RELATIVE_16 2
#define CLK_IMM         0
#define CLK_AI          4
#define CLK_AXI         4
#define CLK_A           2
#define CLK_AL          3
#define CLK_ALX         3
#define CLK_AX          2
#define CLK_AY          2
#define CLK_D           1
#define CLK_DI          3
#define CLK_DIY         3
#define CLK_DLI         4
#define CLK_DLIY        4
#define CLK_DX          2
#define CLK_DXI         4
#define CLK_DY          2
#define CLK_S           2
#define CLK_SIY         5

/* AX and AY addressing modes take 1 extra cycle when writing */
#define CLK_W_IMM       0
#define CLK_W_AI        4
#define CLK_W_AXI       4
#define CLK_W_A         2
#define CLK_W_AL        3
#define CLK_W_ALX       3
#define CLK_W_AX        3
#define CLK_W_AY        3
#define CLK_W_D         1
#define CLK_W_DI        3
#define CLK_W_DIY       3
#define CLK_W_DLI       4
#define CLK_W_DLIY      4
#define CLK_W_DX        2
#define CLK_W_DXI       4
#define CLK_W_DY        2
#define CLK_W_S         2
#define CLK_W_SIY       5

#define CLK(A)          CLOCKS -= (A)
#define USE_ALL_CLKS()  CLOCKS = 0


/* ======================================================================== */
/* ============================ STATUS REGISTER =========================== */
/* ======================================================================== */

/* Flag positions in Processor Status Register */
/* common */
#define FLAGPOS_N       BIT_7   /* Negative         */
#define FLAGPOS_V       BIT_6   /* Overflow         */
#define FLAGPOS_D       BIT_3   /* Decimal Mode     */
#define FLAGPOS_I       BIT_2   /* Interrupt Mask   */
#define FLAGPOS_Z       BIT_1   /* Zero             */
#define FLAGPOS_C       BIT_0   /* Carry            */
/* emulation */
#define FLAGPOS_R       BIT_5   /* Reserved         */
#define FLAGPOS_B       BIT_4   /* BRK Instruction  */
/* native */
#define FLAGPOS_M       BIT_5   /* Mem/Reg Select   */
#define FLAGPOS_X       BIT_4   /* Index Select     */

#define EFLAG_SET       1
#define EFLAG_CLEAR     0
#define MFLAG_SET       FLAGPOS_M
#define MFLAG_CLEAR     0
#define XFLAG_SET       FLAGPOS_X
#define XFLAG_CLEAR     0
#define NFLAG_SET       0x80
#define NFLAG_CLEAR     0
#define VFLAG_SET       0x80
#define VFLAG_CLEAR     0
#define DFLAG_SET       FLAGPOS_D
#define DFLAG_CLEAR     0
#define IFLAG_SET       FLAGPOS_I
#define IFLAG_CLEAR     0
#define BFLAG_SET       FLAGPOS_B
#define BFLAG_CLEAR     0
#define ZFLAG_SET       0
#define ZFLAG_CLEAR     1
#define CFLAG_SET       0x100
#define CFLAG_CLEAR     0

/* Codition code tests */
#define COND_CC()       (!(FLAG_C&0x100))   /* Carry Clear */
#define COND_CS()       (FLAG_C&0x100)      /* Carry Set */
#define COND_EQ()       (!FLAG_Z)           /* Equal */
#define COND_NE()       FLAG_Z              /* Not Equal */
#define COND_MI()       (FLAG_N&0x80)       /* Minus */
#define COND_PL()       (!(FLAG_N&0x80))    /* Plus */
#define COND_VC()       (!(FLAG_V&0x80))    /* Overflow Clear */
#define COND_VS()       (FLAG_V&0x80)       /* Overflow Set */

/* Set Overflow flag in math operations */
#define VFLAG_ADD_8(S, D, R)    ((S^R) & (D^R))
#define VFLAG_ADD_16(S, D, R)   (((S^R) & (D^R))>>8)
#define VFLAG_SUB_8(S, D, R)    ((S^D) & (R^D))
#define VFLAG_SUB_16(S, D, R)   (((S^D) & (R^D))>>8)

#define CFLAG_8(A)      (A)
#define CFLAG_16(A)     ((A)>>8)
#define NFLAG_8(A)      (A)
#define NFLAG_16(A)     ((A)>>8)

#define CFLAG_AS_1()    ((FLAG_C>>8)&1)

/* update IRQ state (internal use only) */
void m37710i_update_irqs(m37710i_cpu_struct *cpustate);

/* ======================================================================== */
/* ================================== CPU ================================= */
/* ======================================================================== */
#endif /* __M37710CM_H__ */
