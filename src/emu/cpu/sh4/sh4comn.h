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

#ifdef USE_SH4DRC
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"

class sh4_frontend;
#endif

#define CPU_TYPE_SH3	(2)
#define CPU_TYPE_SH4	(3)

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


struct sh4_state 
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
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *internal;
	address_space *program;
	direct_read_data *direct;
	address_space *io;

	// sh4 internal
	UINT32	*m;

	// timer regs handled manually for reuse
	UINT32 SH4_TSTR;
	UINT32 SH4_TCNT0;
	UINT32 SH4_TCNT1;
	UINT32 SH4_TCNT2;
	UINT32 SH4_TCR0;
	UINT32 SH4_TCR1;
	UINT32 SH4_TCR2;
	UINT32 SH4_TCOR0;
	UINT32 SH4_TCOR1;
	UINT32 SH4_TCOR2;
	UINT32 SH4_TOCR;
	UINT32 SH4_TCPR2;

	// INTC regs
	UINT32 SH4_IPRA;

	UINT32 SH4_IPRC;

	// DMAC regs
	UINT32 SH4_SAR0;
	UINT32 SH4_SAR1;
	UINT32 SH4_SAR2;
	UINT32 SH4_SAR3;

	UINT32 SH4_DAR0;
	UINT32 SH4_DAR1;
	UINT32 SH4_DAR2;
	UINT32 SH4_DAR3;

	UINT32 SH4_CHCR0;
	UINT32 SH4_CHCR1;
	UINT32 SH4_CHCR2;
	UINT32 SH4_CHCR3;

	UINT32 SH4_DMATCR0;
	UINT32 SH4_DMATCR1;
	UINT32 SH4_DMATCR2;
	UINT32 SH4_DMATCR3;

	UINT32 SH4_DMAOR;


	// sh3 internal
	UINT32  m_sh3internal_upper[0x3000/4];
	UINT32  m_sh3internal_lower[0x1000];

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
	UINT32	dma_source[4];
	UINT32	dma_destination[4];
	UINT32	dma_count[4];
	int		dma_wordsize[4];
	int		dma_source_increment[4];
	int		dma_destination_increment[4];
	int		dma_mode[4];

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

	int cpu_type;

#ifdef USE_SH4DRC
	int	icount;

	int pcfsel;	    			// last pcflush entry set
	int maxpcfsel;				// highest valid pcflush entry
	UINT32 pcflushes[16];		// pcflush entries

	drc_cache *			cache;		    	/* pointer to the DRC code cache */
	drcuml_state *		drcuml;				/* DRC UML generator state */
	sh4_frontend *		drcfe;				/* pointer to the DRC front-end class */
	UINT32				drcoptions;			/* configurable DRC options */

	/* internal stuff */
	UINT8				cache_dirty;    	/* true if we need to flush the cache */

	/* parameters for subroutines */
	UINT64				numcycles;		    /* return value from gettotalcycles */
	UINT32				arg0;			    /* print_debug argument 1 */
	UINT32				arg1;			    /* print_debug argument 2 */
	UINT32				irq;				/* irq we're taking */

	/* register mappings */
	uml::parameter	regmap[16];		    		/* parameter to register mappings for all 16 integer registers */

	uml::code_handle *	entry;			    		/* entry point */
	uml::code_handle *	read8;					/* read byte */
	uml::code_handle *	write8;					/* write byte */
	uml::code_handle *	read16;					/* read half */
	uml::code_handle *	write16;		    		/* write half */
	uml::code_handle *	read32;					/* read word */
	uml::code_handle *	write32;		    		/* write word */

	uml::code_handle *	interrupt;				/* interrupt */
	uml::code_handle *	nocode;					/* nocode */
	uml::code_handle *	out_of_cycles;				/* out of cycles exception handler */

	UINT32 prefadr;
	UINT32 target;
#endif
};

#ifdef USE_SH4DRC
class sh4_frontend : public drc_frontend
{
public:
	sh4_frontend(sh4_state &state, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);

protected:
	virtual bool describe(opcode_desc &desc, const opcode_desc *prev);

private:
	bool describe_group_0(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_2(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_3(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_4(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_6(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_8(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_12(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);
	bool describe_group_15(opcode_desc &desc, const opcode_desc *prev, UINT16 opcode);

	sh4_state &m_context;
};

INLINE sh4_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SH3LE || device->type() == SH3BE ||
		   device->type() == SH4LE || device->type() == SH4BE );
	return *(sh4_state **)downcast<legacy_cpu_device *>(device)->token();
}
#else
INLINE sh4_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == SH3LE || device->type() == SH3BE ||
		   device->type() == SH4LE || device->type() == SH4BE );
	return (sh4_state *)downcast<legacy_cpu_device *>(device)->token();
}
#endif


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

#define REGFLAG_R(n)                    (1 << (n))
#define REGFLAG_FR(n)                   (1 << (n))
#define REGFLAG_XR(n)                   (1 << (n))

/* register flags 1 */
#define REGFLAG_PR						(1 << 0)
#define REGFLAG_MACL					(1 << 1)
#define REGFLAG_MACH					(1 << 2)
#define REGFLAG_GBR						(1 << 3)
#define REGFLAG_VBR						(1 << 4)
#define REGFLAG_SR						(1 << 5)
#define REGFLAG_SGR						(1 << 6)
#define REGFLAG_FPUL					(1 << 7)
#define REGFLAG_FPSCR					(1 << 8)
#define REGFLAG_DBR						(1 << 9)
#define REGFLAG_SSR						(1 << 10)
#define REGFLAG_SPC						(1 << 11)

void sh4_exception_recompute(sh4_state *sh4); // checks if there is any interrupt with high enough priority
void sh4_exception_request(sh4_state *sh4, int exception); // start requesting an exception
void sh4_exception_unrequest(sh4_state *sh4, int exception); // stop requesting an exception
void sh4_exception_checkunrequest(sh4_state *sh4, int exception);
void sh4_exception(sh4_state *sh4, const char *message, int exception); // handle exception
void sh4_change_register_bank(sh4_state *sh4, int to);
void sh4_syncronize_register_bank(sh4_state *sh4, int to);
void sh4_swap_fp_registers(sh4_state *sh4);
void sh4_default_exception_priorities(sh4_state *sh4); // setup default priorities for exceptions
void sh4_parse_configuration(sh4_state *sh4, const struct sh4_config *conf);
void sh4_set_irq_line(sh4_state *sh4, int irqline, int state); // set state of external interrupt line
#ifdef LSB_FIRST
void sh4_swap_fp_couples(sh4_state *sh4);
#endif
void sh4_common_init(device_t *device);
UINT32 sh4_getsqremap(sh4_state *sh4, UINT32 address);
void sh4_handler_ipra_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask);

READ64_HANDLER( sh4_tlb_r );
WRITE64_HANDLER( sh4_tlb_w );


INLINE void sh4_check_pending_irq(sh4_state *sh4, const char *message) // look for highest priority active exception and handle it
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

