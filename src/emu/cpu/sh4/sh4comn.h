/*****************************************************************************
 *
 *   sh4comn.h
 *
 *   SH-4 non-specific components
 *
 *****************************************************************************/

#pragma once

#ifndef __SH4COMN_H__
#define __SH4COMN_H__

//#define USE_SH4DRC

/* speed up delay loops, bail out of tight loops */
#define BUSY_LOOP_HACKS 	0

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define EXPPRI(pl,po,p,n)	(((4-(pl)) << 24) | ((15-(po)) << 16) | ((p) << 8) | (255-(n)))
#define NMIPRI()			EXPPRI(3,0,16,SH4_INTC_NMI)
#define INTPRI(p,n)			EXPPRI(4,2,p,n)

#define FP_RS(r) sh4->fr[(r)] // binary representation of single precision floating point register r
#define FP_RFS(r) *( (float  *)(sh4->fr+(r)) ) // single precision floating point register r
#define FP_RFD(r) *( (double *)(sh4->fr+(r)) ) // double precision floating point register r
#define FP_XS(r) sh4->xf[(r)] // binary representation of extended single precision floating point register r
#define FP_XFS(r) *( (float  *)(sh4->xf+(r)) ) // single precision extended floating point register r
#define FP_XFD(r) *( (double *)(sh4->xf+(r)) ) // double precision extended floating point register r
#ifdef LSB_FIRST
#define FP_RS2(r) sh4->fr[(r) ^ sh4->fpu_pr]
#define FP_RFS2(r) *( (float  *)(sh4->fr+((r) ^ sh4->fpu_pr)) )
#define FP_XS2(r) sh4->xf[(r) ^ sh4->fpu_pr]
#define FP_XFS2(r) *( (float  *)(sh4->xf+((r) ^ sh4->fpu_pr)) )
#endif


typedef struct
{
	UINT32	ppc;
	UINT32	pc, spc;
	UINT32	pr;
	UINT32	sr, ssr;
	UINT32	gbr, vbr;
	UINT32	mach, macl;
	UINT32	r[16], rbnk[2][8], sgr;
	UINT32  fr[16], xf[16];
	UINT32	ea;
	UINT32	delay;
	UINT32	cpu_off;
	UINT32	pending_irq;
	UINT32  test_irq;
	UINT32  fpscr;
	UINT32  fpul;
	UINT32  dbr;

	UINT32	exception_priority[128];
	int		exception_requesting[128];

	INT8	irq_line_state[17];
	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	const address_space *internal;
	const address_space *program;
	const address_space *io;
	UINT32	*m;
	INT8	nmi_line_state;

	UINT8 sleep_mode;

	int		frt_input;
	int		irln;
	int 	internal_irq_level;
	int 	internal_irq_vector;

	emu_timer *dma_timer[4];
	emu_timer *refresh_timer;
	emu_timer *rtc_timer;
	emu_timer *timer[3];
	UINT32	refresh_timer_base;
	int     dma_timer_active[4];

	int 	sh4_icount;
	int     is_slave;
	int		cpu_clock, bus_clock, pm_clock;
	int		fpu_sz, fpu_pr;
	int		ioport16_pullup, ioport16_direction;
	int		ioport4_pullup, ioport4_direction;

	void	(*ftcsr_read_callback)(UINT32 data);


	/* This MMU simulation is good for the simple remap used on Naomi GD-ROM SQ access *ONLY* */
	UINT32 sh4_tlb_address[64];
	UINT32 sh4_tlb_data[64];
	UINT8 sh4_mmu_enabled;

} SH4;

enum
{
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

/* Bits in SR */
#define T	0x00000001
#define S	0x00000002
#define I	0x000000f0
#define Q	0x00000100
#define M	0x00000200
#define FD	0x00008000
#define BL	0x10000000
#define sRB	0x20000000
#define MD	0x40000000

/* 29 bits */
#define AM	0x1fffffff

#define FLAGS	(MD|sRB|BL|FD|M|Q|I|S|T)

/* Bits in FPSCR */
#define RM  0x00000003
#define DN  0x00040000
#define PR  0x00080000
#define SZ  0x00100000
#define FR  0x00200000

#define Rn	((opcode>>8)&15)
#define Rm	((opcode>>4)&15)

void sh4_exception_recompute(SH4 *sh4); // checks if there is any interrupt with high enough priority
void sh4_exception_request(SH4 *sh4, int exception); // start requesting an exception
void sh4_exception_unrequest(SH4 *sh4, int exception); // stop requesting an exception
void sh4_exception_checkunrequest(SH4 *sh4, int exception);
void sh4_exception(SH4 *sh4, const char *message, int exception); // handle exception
void sh4_change_register_bank(SH4 *sh4, int to);
void sh4_syncronize_register_bank(SH4 *sh4, int to);
void sh4_swap_fp_registers(SH4 *sh4);
void sh4_default_exception_priorities(SH4 *sh4); // setup default priorities for exceptions
void sh4_parse_configuration(SH4 *sh4, const struct sh4_config *conf);
void sh4_set_irq_line(SH4 *sh4, int irqline, int state); // set state of external interrupt line
#ifdef LSB_FIRST
void sh4_swap_fp_couples(SH4 *sh4);
#endif
void sh4_common_init(running_device *device);

INLINE void sh4_check_pending_irq(SH4 *sh4, const char *message) // look for highest priority active exception and handle it
{
	int a,irq,z;

	irq = 0;
	z = -1;
	for (a=0;a <= SH4_INTC_ROVI;a++)
	{
		if (sh4->exception_requesting[a])
		{
			if ((int)sh4->exception_priority[a] > z)
			{
				z = sh4->exception_priority[a];
				irq = a;
			}
		}
	}
	if (z >= 0)
	{
		sh4_exception(sh4, message, irq);
	}
}

#endif /* __SH4COMN_H__ */

