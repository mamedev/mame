#pragma once

#ifndef __SPC700_H__
#define __SPC700_H__

/* ======================================================================== */
/* ============================= Configuration ============================ */
/* ======================================================================== */

#ifndef INLINE
#define INLINE static
#endif

#undef uint
#define uint unsigned int

/* Turn on optimizations for SNES since it doesn't hook up the interrupt lines */
#define SPC700_OPTIMIZE_SNES 1


/* ======================================================================== */
/* ============================== PROTOTYPES ============================== */
/* ======================================================================== */

enum
{
	SPC700_PC=1, SPC700_S, SPC700_P, SPC700_A, SPC700_X, SPC700_Y
};

#define SPC700_INT_NONE			0
#define SPC700_INT_IRQ			1
#define SPC700_INT_NMI			2

/* CPU Structure */
typedef struct
{
	uint a;		/* Accumulator */
	uint x;		/* Index Register X */
	uint y;		/* Index Register Y */
	uint s;		/* Stack Pointer */
	uint pc;	/* Program Counter */
	uint ppc;	/* Previous Program Counter */
	uint flag_n;	/* Negative Flag */
	uint flag_z;	/* Zero flag */
	uint flag_v;	/* Overflow Flag */
	uint flag_p;	/* Direct Page Flag */
	uint flag_b;	/* BRK Instruction Flag */
	uint flag_h;	/* Half-carry Flag */
	uint flag_i;	/* Interrupt Mask Flag */
	uint flag_c;	/* Carry Flag */
	uint line_irq;	/* Status of the IRQ line */
	uint line_nmi;	/* Status of the NMI line */
	uint line_rst;	/* Status of the RESET line */
	uint ir;		/* Instruction Register */
	cpu_irq_callback int_ack;
	const device_config *device;
	const address_space *program;
	uint stopped;	/* stopped status */
	int ICount;
	uint source;
	uint destination;
	uint temp1, temp2, temp3;
	short spc_int16;
	int spc_int32;
} spc700i_cpu;


/* ======================================================================== */
/* ============================= INTERFACE API ============================ */
/* ======================================================================== */

/* This is the interface, not the implementation.  Please call the  */
/* implementation APIs below.                                       */


CPU_INIT( spc700 );

/* Pulse the RESET pin on the CPU */
CPU_RESET( spc700 );

/* Set the RESET line on the CPU */
void spc700_set_reset_line(spc700i_cpu *cpustate, int state, void* param);

/* Clean up after the emulation core - Not used in this core - */
CPU_EXIT( spc700 );

/* Get the current Program Counter */
unsigned spc700_get_pc(spc700i_cpu *cpustate);

/* Set the current Program Counter */
void spc700_set_pc(spc700i_cpu *cpustate,unsigned val);

/* Get the current Stack Pointer */
unsigned spc700_get_sp(spc700i_cpu *cpustate);

/* Set the current Stack Pointer */
void spc700_set_sp(spc700i_cpu *cpustate,unsigned val);

/* Get a register from the core */
unsigned spc700_get_reg(spc700i_cpu *cpustate,int regnum);

/* Set a register in the core */
void spc700_set_reg(spc700i_cpu *cpustate,int regnum, unsigned val);

/* Note about NMI:
 *   NMI is a one-shot trigger.  In order to trigger NMI again, you must
 *   clear NMI and then assert it again.
 */
void spc700_set_nmi_line(spc700i_cpu *cpustate,int state);

/* Assert or clear the IRQ pin */
void spc700_set_irq_line(spc700i_cpu *cpustate,int line, int state);

/* Set the callback that will be called when an interrupt is serviced */
void spc700_set_irq_callback(spc700i_cpu *cpustate,cpu_irq_callback callback);

/* Get a formatted string representing a register and its contents */
const char *spc700_info(spc700i_cpu *cpustate,void *context, int regnum);


/* Pulse the SO (Set Overflow) pin on the CPU */
void spc700_pulse_so(spc700i_cpu *cpustate);


/* ======================================================================== */
/* =================== Functions Implemented by the Host ================== */
/* ======================================================================== */

/* Read data from RAM */
unsigned int spc700_read_8(unsigned int address);

/* Read data from the direct page */
unsigned int spc700_read_8_direct(unsigned int address);

/* Read data from ROM */
unsigned int spc700_read_8_immediate(unsigned int address);
unsigned int spc700_read_8_instruction(unsigned int address);

/* Write data to RAM */
void spc700_write_8(unsigned int address, unsigned int value);
void spc700_write_8_direct(unsigned int address, unsigned int value);

void spc700_jumping(unsigned int new_pc);
void spc700_branching(unsigned int new_pc);



/* ======================================================================== */
/* ================================= MAME ================================= */
/* ======================================================================== */

#include "cpuintrf.h"

extern CPU_GET_INFO( spc700 );
#define CPU_SPC700 CPU_GET_INFO_NAME( spc700 )

#define spc700_read_8(addr) memory_read_byte_8le(cpustate->program,addr)
#define spc700_write_8(addr,data) memory_write_byte_8le(cpustate->program,addr,data)

#define spc700_read_8_direct(A)     spc700_read_8(A)
#define spc700_write_8_direct(A, V) spc700_write_8(A, V)
//#define spc700_read_instruction(A)    memory_decrypted_read_byte(cpustate->program,A)
//#define spc700_read_8_immediate(A)    memory_raw_read_byte(cpustate->program,A)
#define spc700_read_instruction(A)    memory_read_byte_8le(cpustate->program,A)
#define spc700_read_8_immediate(A)    memory_read_byte_8le(cpustate->program,A)
#define spc700_jumping(A)
#define spc700_branching(A)



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */

#endif /* __SPC700_H__ */
