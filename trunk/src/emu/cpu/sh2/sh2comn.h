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

// do we use a timer for the DMA, or have it in CPU_EXECUTE
#define USE_TIMER_FOR_DMA

#ifdef USE_SH2DRC
#include "cpu/drcfe.h"
#include "cpu/drcuml.h"
#include "cpu/drcumlsh.h"
class sh2_frontend;
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
	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *internal;
	UINT32	*m;
	INT8  nmi_line_state;

	UINT16	frc;
	UINT16	ocra, ocrb, icr;
	UINT64	frc_base;

	int		frt_input;
	int 	internal_irq_level;
	int 	internal_irq_vector;
	int				icount;

	emu_timer *timer;
	emu_timer *dma_current_active_timer[2];
	int     dma_timer_active[2];

	int active_dma_incs[2];
	int active_dma_incd[2];
	int active_dma_size[2];
	int active_dma_steal[2];
	UINT32 active_dma_src[2];
	UINT32 active_dma_dst[2];
	UINT32 active_dma_count[2];

	int     is_slave, cpu_type;
	int  (*dma_callback_kludge)(device_t *device, UINT32 src, UINT32 dst, UINT32 data, int size);
	int  (*dma_callback_fifo_data_available)(device_t *device, UINT32 src, UINT32 dst, UINT32 data, int size);

	void	(*ftcsr_read_callback)(UINT32 data);

#ifdef USE_SH2DRC
	drc_cache *			cache;			    	/* pointer to the DRC code cache */
	drcuml_state *		drcuml;					/* DRC UML generator state */
	sh2_frontend *		drcfe;					/* pointer to the DRC front-end state */
	UINT32				drcoptions;			/* configurable DRC options */

	/* internal stuff */
	UINT8				cache_dirty;		    	/* true if we need to flush the cache */

	/* parameters for subroutines */
	UINT64				numcycles;		    	/* return value from gettotalcycles */
	UINT32				arg0;			    	/* print_debug argument 1 */
	UINT32				arg1;			    	/* print_debug argument 2 */
	UINT32				irq;				/* irq we're taking */

	/* register mappings */
	uml::parameter		regmap[16];		    		/* parameter to register mappings for all 16 integer registers */

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
#endif
} sh2_state;

#ifdef USE_SH2DRC
class sh2_frontend : public drc_frontend
{
public:
	sh2_frontend(sh2_state &state, UINT32 window_start, UINT32 window_end, UINT32 max_sequence);

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

	sh2_state &m_context;
};
#endif

void sh2_common_init(sh2_state *sh2, legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback);
void sh2_recalc_irq(sh2_state *sh2);
void sh2_set_irq_line(sh2_state *sh2, int irqline, int state);
void sh2_exception(sh2_state *sh2, const char *message, int irqline);
void sh2_do_dma(sh2_state *sh2, int dma);
void sh2_notify_dma_data_available(device_t *device);

#endif /* __SH2COMN_H__ */
