/*****************************************************************************
 *
 *   minc4510.h
 *   Base macros for 4510 CPU files
 *
 *****************************************************************************/


/* 4510 flags */
#define F_C	0x01
#define F_Z	0x02
#define F_I	0x04
#define F_D	0x08
#define F_B	0x10
#define F_E	0x20
#define F_V	0x40
#define F_N	0x80

/* some shortcuts for improved readability */
#define A	m4510.a
#define X	m4510.x
#define Y	m4510.y
#define P	m4510.p
#define Z	m4510.z
#define B	m4510.zp.b.h
#define SW	m4510.sp.w.l
#define SPL	m4510.sp.b.l
#define SPH	m4510.sp.b.h
#define SPD	m4510.sp.d

#define NZ	m4510.nz

#define EAL	m4510.ea.b.l
#define EAH	m4510.ea.b.h
#define EAW	m4510.ea.w.l
#define EAD	m4510.ea.d

#define ZPL	m4510.zp.b.l
#define ZPH	m4510.zp.b.h
#define ZPW	m4510.zp.w.l
#define ZPD	m4510.zp.d

#define PCL	m4510.pc.b.l
#define PCH	m4510.pc.b.h
#define PCW	m4510.pc.w.l
#define PCD	m4510.pc.d

#define PPC	m4510.ppc.d

#define IRQ_STATE	m4510.irq_state
#define AFTER_CLI	m4510.after_cli

#define M4510_MEM(addr)	(m4510.mem[(addr)>>13]+(addr))

#define CHANGE_PC	change_pc(M4510_MEM(PCD))

#define PEEK_OP()	cpu_readop(M4510_MEM(PCD))

#define RDMEM(addr)			program_read_byte_8le(M4510_MEM(addr)); m4510_ICount -= 1
#define WRMEM(addr,data)	program_write_byte_8le(M4510_MEM(addr),data); m4510_ICount -= 1

/***************************************************************
 *  RDOP    read an opcode
 ***************************************************************/
#undef RDOP
#define RDOP() m4510_cpu_readop(); m4510_ICount -= 1

/***************************************************************
 *  RDOPARG read an opcode argument
 ***************************************************************/
#undef RDOPARG
#define RDOPARG() m4510_cpu_readop_arg(); m4510_ICount -= 1
