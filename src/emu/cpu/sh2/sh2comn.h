/*****************************************************************************
 *
 *   sh2common.h
 *
 *   SH-2 non-specific components
 *
 *****************************************************************************/

#pragma once

#ifndef __SH2COMN_H__
#define __SH2COMN_H__


#define USE_SH2DRC

#ifdef USE_SH2DRC
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"
#endif

#define SH2_CODE_XOR(a)		((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))

typedef struct _irq_entry irq_entry;
struct _irq_entry
{
	int irq_vector;
	int irq_priority;
};

enum
{
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

#define T	0x00000001
#define S	0x00000002
#define I	0x000000f0
#define Q	0x00000100
#define M	0x00000200

#define AM	0xc7ffffff

#define FLAGS	(M|Q|I|S|T)

#define Rn	((opcode>>8)&15)
#define Rm	((opcode>>4)&15)

#define CPU_TYPE_SH1	(0)
#define CPU_TYPE_SH2	(1)

#define REGFLAG_R(n)                                        (1 << (n))

/* register flags 1 */
#define REGFLAG_PR						(1 << 0)
#define REGFLAG_MACL						(1 << 1)
#define REGFLAG_MACH						(1 << 2)
#define REGFLAG_GBR						(1 << 3)
#define REGFLAG_VBR						(1 << 4)
#define REGFLAG_SR						(1 << 5)

#define CHECK_PENDING_IRQ(message)				\
do {											\
	int irq = -1;								\
	if (sh2->pending_irq & (1 <<  0)) irq =	0;	\
	if (sh2->pending_irq & (1 <<  1)) irq =	1;	\
	if (sh2->pending_irq & (1 <<  2)) irq =	2;	\
	if (sh2->pending_irq & (1 <<  3)) irq =	3;	\
	if (sh2->pending_irq & (1 <<  4)) irq =	4;	\
	if (sh2->pending_irq & (1 <<  5)) irq =	5;	\
	if (sh2->pending_irq & (1 <<  6)) irq =	6;	\
	if (sh2->pending_irq & (1 <<  7)) irq =	7;	\
	if (sh2->pending_irq & (1 <<  8)) irq =	8;	\
	if (sh2->pending_irq & (1 <<  9)) irq =	9;	\
	if (sh2->pending_irq & (1 << 10)) irq = 10;	\
	if (sh2->pending_irq & (1 << 11)) irq = 11;	\
	if (sh2->pending_irq & (1 << 12)) irq = 12;	\
	if (sh2->pending_irq & (1 << 13)) irq = 13;	\
	if (sh2->pending_irq & (1 << 14)) irq = 14;	\
	if (sh2->pending_irq & (1 << 15)) irq = 15;	\
	if ((sh2->internal_irq_level != -1) && (sh2->internal_irq_level > irq)) irq = sh2->internal_irq_level; \
	if (irq >= 0)								\
		sh2_exception(sh2,message,irq);			\
} while(0)

typedef struct
{
	UINT32	ppc;
	UINT32	pc;
	UINT32	pr;
	UINT32	sr;
	UINT32	gbr, vbr;
	UINT32	mach, macl;
	UINT32	r[16];
	UINT32	ea;
	UINT32	delay;
	UINT32	cpu_off;
	UINT32	dvsr, dvdnth, dvdntl, dvcr;
	UINT32	pending_irq;
	UINT32	test_irq;
	UINT32	pending_nmi;
	INT32  irqline;
	UINT32	evec;				// exception vector for DRC
	UINT32  irqsr;				// IRQ-time old SR for DRC
	UINT32 target;				// target for jmp/jsr/etc so the delay slot can't kill it
	irq_entry     irq_queue[16];

	int pcfsel;	    			// last pcflush entry set
	int maxpcfsel;				// highest valid pcflush entry
	UINT32 pcflushes[16];			// pcflush entries

	INT8	irq_line_state[17];
	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	const address_space *program;
	const address_space *internal;
	UINT32	*m;
	INT8  nmi_line_state;

	UINT16	frc;
	UINT16	ocra, ocrb, icr;
	UINT64	frc_base;

	int		frt_input;
	int 	internal_irq_level;
	int 	internal_irq_vector;

	emu_timer *timer;
	emu_timer *dma_timer[2];
	int     dma_timer_active[2];

	int     is_slave, cpu_type;
	int  (*dma_callback_kludge)(UINT32 src, UINT32 dst, UINT32 data, int size);

	void	(*ftcsr_read_callback)(UINT32 data);

#ifdef USE_SH2DRC
	drccache *			cache;			    	/* pointer to the DRC code cache */
	drcuml_state *		drcuml;					/* DRC UML generator state */
	drcfe_state *		drcfe;					/* pointer to the DRC front-end state */
	UINT32				drcoptions;			/* configurable DRC options */

	int				icount;

	/* internal stuff */
	UINT8				cache_dirty;		    	/* true if we need to flush the cache */

	/* parameters for subroutines */
	UINT64				numcycles;		    	/* return value from gettotalcycles */
	UINT32				arg0;			    	/* print_debug argument 1 */
	UINT32				arg1;			    	/* print_debug argument 2 */
	UINT32				irq;				/* irq we're taking */

	/* register mappings */
	drcuml_parameter	regmap[16];		    		/* parameter to register mappings for all 16 integer registers */

	drcuml_codehandle *	entry;			    		/* entry point */
	drcuml_codehandle *	read8;					/* read byte */
	drcuml_codehandle *	write8;					/* write byte */
	drcuml_codehandle *	read16;					/* read half */
	drcuml_codehandle *	write16;		    		/* write half */
	drcuml_codehandle *	read32;					/* read word */
	drcuml_codehandle *	write32;		    		/* write word */

	drcuml_codehandle *	interrupt;				/* interrupt */
	drcuml_codehandle *	nocode;					/* nocode */
	drcuml_codehandle *	out_of_cycles;				/* out of cycles exception handler */
#endif
} sh2_state;

void sh2_common_init(sh2_state *sh2, legacy_cpu_device *device, device_irq_callback irqcallback);
void sh2_recalc_irq(sh2_state *sh2);
void sh2_set_irq_line(sh2_state *sh2, int irqline, int state);
void sh2_exception(sh2_state *sh2, const char *message, int irqline);

#endif /* __SH2COMN_H__ */
