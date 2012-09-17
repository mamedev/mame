/*****************************************************************************
 *
 *   mincce02.h
 *   Base macros for 65ce02 CPU files
 *
 *****************************************************************************/


/* 65ce02 flags */
#define F_C	0x01
#define F_Z	0x02
#define F_I	0x04
#define F_D	0x08
#define F_B	0x10
#define F_E	0x20
#define F_V	0x40
#define F_N	0x80

/* some shortcuts for improved readability */
#define A	cpustate->a
#define X	cpustate->x
#define Y	cpustate->y
#define P	cpustate->p
#define Z	cpustate->z
#define B	cpustate->zp.b.h
#define SW	cpustate->sp.w.l
#define SPL	cpustate->sp.b.l
#define SPH	cpustate->sp.b.h
#define SPD	cpustate->sp.d

#define NZ	cpustate->nz

#define EAL	cpustate->ea.b.l
#define EAH	cpustate->ea.b.h
#define EAW	cpustate->ea.w.l
#define EAD	cpustate->ea.d

#define ZPL	cpustate->zp.b.l
#define ZPH	cpustate->zp.b.h
#define ZPW	cpustate->zp.w.l
#define ZPD	cpustate->zp.d

#define PCL	cpustate->pc.b.l
#define PCH	cpustate->pc.b.h
#define PCW	cpustate->pc.w.l
#define PCD	cpustate->pc.d

#define PPC	cpustate->ppc.d

#define RDMEM_ID(a)		cpustate->rdmem_id(*cpustate->space, a)
#define WRMEM_ID(a,d)	cpustate->wrmem_id(*cpustate->space, a, d)

#define IRQ_STATE	cpustate->irq_state
#define AFTER_CLI	cpustate->after_cli

/***************************************************************
 *  RDOP    read an opcode
 ***************************************************************/
#define RDOP()	cpustate->direct->read_decrypted_byte(PCW++); cpustate->icount -= 1

/***************************************************************
 *  RDOPARG read an opcode argument
 ***************************************************************/
#define RDOPARG()	cpustate->direct->read_raw_byte(PCW++); cpustate->icount -= 1

#define PEEK_OP()	cpustate->direct->read_decrypted_byte(PCW)

#define RDMEM(addr)			cpustate->space->read_byte(addr); cpustate->icount -= 1
#define WRMEM(addr,data)	cpustate->space->write_byte(addr,data); cpustate->icount -= 1
