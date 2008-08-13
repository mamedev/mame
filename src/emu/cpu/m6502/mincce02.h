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
#define A	m65ce02.a
#define X	m65ce02.x
#define Y	m65ce02.y
#define P	m65ce02.p
#define Z	m65ce02.z
#define B	m65ce02.zp.b.h
#define SW	m65ce02.sp.w.l
#define SPL	m65ce02.sp.b.l
#define SPH	m65ce02.sp.b.h
#define SPD	m65ce02.sp.d

#define NZ	m65ce02.nz

#define EAL	m65ce02.ea.b.l
#define EAH	m65ce02.ea.b.h
#define EAW	m65ce02.ea.w.l
#define EAD	m65ce02.ea.d

#define ZPL	m65ce02.zp.b.l
#define ZPH	m65ce02.zp.b.h
#define ZPW	m65ce02.zp.w.l
#define ZPD	m65ce02.zp.d

#define PCL	m65ce02.pc.b.l
#define PCH	m65ce02.pc.b.h
#define PCW	m65ce02.pc.w.l
#define PCD	m65ce02.pc.d

#define PPC	m65ce02.ppc.d

#define RDMEM_ID	m65ce02.rdmem_id
#define WRMEM_ID	m65ce02.wrmem_id

#define IRQ_STATE	m65ce02.irq_state
#define AFTER_CLI	m65ce02.after_cli

#define CHANGE_PC	change_pc(PCD)

/***************************************************************
 *  RDOP    read an opcode
 ***************************************************************/
#define RDOP()	cpu_readop(PCW++); m65ce02_ICount -= 1

/***************************************************************
 *  RDOPARG read an opcode argument
 ***************************************************************/
#define RDOPARG()	cpu_readop_arg(PCW++); m65ce02_ICount -= 1

#define PEEK_OP()	cpu_readop(PCW)

#define RDMEM(addr)			program_read_byte_8le(addr); m65ce02_ICount -= 1
#define WRMEM(addr,data)	program_write_byte_8le(addr,data); m65ce02_ICount -= 1
