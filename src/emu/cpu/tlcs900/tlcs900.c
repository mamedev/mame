/*******************************************************************

Toshiba TLCS-900/H emulation

This code only supports the 900/H mode which is needed for Neogeo
Pocket emulation. The 900 and 900/M modes are not supported yet.


TODO:
- review cycle counts
- implement the remaining internal mcu features
- add support for 900 and 900/M modes

*******************************************************************/

#include "emu.h"
#include "debugger.h"
#include "tlcs900.h"

typedef struct _tlcs900_state tlcs900_state;
struct _tlcs900_state
{
	const tlcs900_interface *intf;

	devcb_resolved_write8	to1;
	devcb_resolved_write8	to3;

	/* registers */
	PAIR	xwa[4];
	PAIR	xbc[4];
	PAIR	xde[4];
	PAIR	xhl[4];
	PAIR	xix;
	PAIR	xiy;
	PAIR	xiz;
	PAIR	xssp;
	PAIR	xnsp;
	PAIR	pc;
	PAIR	sr;
	PAIR	f2;	/* f' */
	/* DMA registers */
	PAIR	dmas[4];
	PAIR	dmad[4];
	PAIR	dmac[4];
	PAIR	dmam[4];

	/* Internal timers, irqs, etc */
	UINT8	reg[0x80];
	UINT32	timer_pre;
	UINT8	timer[6];
	UINT8	tff1;
	UINT8	tff3;
	int		timer_change[4];

	/* Current state of input levels */
	int		level[TLCS900_NUM_INPUTS];
	int		check_irqs;
	int		ad_cycles_left;
	int		nmi_state;

	/* used during execution */
	PAIR	dummy; /* for illegal register references */
	UINT8	op;
	PAIR	ea1, ea2;
	PAIR	imm1, imm2;
	int	cycles;
	UINT8	*p1_reg8, *p2_reg8;
	UINT16	*p1_reg16, *p2_reg16;
	UINT32	*p1_reg32, *p2_reg32;

	int halted;
	int icount;
	int regbank;
	device_irq_callback irqcallback;
	legacy_cpu_device *device;
	address_space *program;
};


/* Internal register defines */
#define P1			0x01
#define P1CR		0x02
#define P2			0x06
#define P2FC		0x09
#define P5			0x0d
#define P5CR		0x10
#define P5FC		0x11
#define P6			0x12
#define P7			0x13
#define P6FC		0x15
#define P7CR		0x16
#define P7FC		0x17
#define P8			0x18
#define P9			0x19
#define P8CR		0x1a
#define P8FC		0x1b
#define PA			0x1e
#define PB			0x1f
#define TRUN		0x20
#define TREG0		0x22
#define TREG1		0x23
#define T01MOD		0x24
#define TFFCR		0x25
#define TREG2		0x26
#define TREG3		0x27
#define T23MOD		0x28
#define TRDC		0x29
#define PACR		0x2c
#define PAFC		0x2d
#define PBCR		0x2e
#define PBFC		0x2f
#define TREG4L		0x30
#define TREG4H		0x31
#define TREG5L		0x32
#define TREG5H		0x33
#define CAP1L		0x34
#define CAP1H		0x35
#define CAP2L		0x36
#define CAP2H		0x37
#define T4MOD		0x38
#define T4FFCR		0x39
#define T45CR		0x3a
#define MSAR0		0x3c
#define MAMR0		0x3d
#define MSAR1		0x3e
#define MAMR1		0x3f
#define TREG6L		0x40
#define TREG6H		0x41
#define TREG7L		0x42
#define TREG7H		0x43
#define CAP3L		0x44
#define CAP3H		0x45
#define CAP4L		0x46
#define CAP4H		0x47
#define T5MOD		0x48
#define T5FFCR		0x49
#define PG0REG		0x4c
#define PG1REG		0x4d
#define PG01CR		0x4e
#define SC0BUF		0x50
#define SC0CR		0x51
#define SC0MOD		0x52
#define BR0CR		0x53
#define SC1BUF		0x54
#define SC1CR		0x55
#define SC1MOD		0x56
#define BR1CR		0x57
#define ODE			0x58
#define DREFCR		0x5a
#define DMEMCR		0x5b
#define MSAR2		0x5c
#define MAMR2		0x5d
#define MSAR3		0x5e
#define MAMR3		0x5f
#define ADREG0L		0x60
#define ADREG0H		0x61
#define ADREG1L		0x62
#define ADREG1H		0x63
#define ADREG2L		0x64
#define ADREG2H		0x65
#define ADREG3L		0x66
#define ADREG3H		0x67
#define B0CS		0x68
#define B1CS		0x69
#define B2CS		0x6a
#define B3CS		0x6b
#define BEXCS		0x6c
#define ADMOD		0x6d
#define WDMOD		0x6e
#define WDCR		0x6f
#define INTE0AD		0x70
#define INTE45		0x71
#define INTE67		0x72
#define INTET10		0x73
#define INTET32		0x74
#define INTET54		0x75
#define INTET76		0x76
#define INTES0		0x77
#define INTES1		0x78
#define INTETC10	0x79
#define INTETC32	0x7a
#define IIMC		0x7b
#define DMA0V		0x7c
#define DMA1V		0x7d
#define DMA2V		0x7e
#define DMA3V		0x7f


/* Flag defines */
#define FLAG_CF		0x01
#define FLAG_NF		0x02
#define FLAG_VF		0x04
#define FLAG_HF		0x10
#define FLAG_ZF		0x40
#define FLAG_SF		0x80


#define RDMEM(addr)			cpustate->program->read_byte( addr )
#define WRMEM(addr,data)	cpustate->program->write_byte( addr, data )
#define RDOP()				RDMEM( cpustate->pc.d ); cpustate->pc.d++
#define RDMEMW(addr)			( RDMEM(addr) | ( RDMEM(addr+1) << 8 ) )
#define RDMEML(addr)			( RDMEMW(addr) | ( RDMEMW(addr+2) << 16 ) )
#define WRMEMW(addr,data)		{ UINT16 dw = data; WRMEM(addr,dw & 0xff); WRMEM(addr+1,(dw >> 8 )); }
#define WRMEML(addr,data)		{ UINT32 dl = data; WRMEMW(addr,dl); WRMEMW(addr+2,(dl >> 16)); }


INLINE tlcs900_state *get_safe_token( device_t *device )
{
	assert( device != NULL );
	assert( device->type() == TLCS900H );

	return (tlcs900_state *) downcast<legacy_cpu_device *>(device)->token();
}


static CPU_INIT( tlcs900 )
{
	tlcs900_state *cpustate = get_safe_token(device);

	cpustate->intf = (const tlcs900_interface *)device->static_config();
	cpustate->irqcallback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space( AS_PROGRAM );

	cpustate->to1.resolve(cpustate->intf->to1, *device );
	cpustate->to3.resolve(cpustate->intf->to3, *device );

	device->save_item( NAME(cpustate->xwa) );
	device->save_item( NAME(cpustate->xbc) );
	device->save_item( NAME(cpustate->xde) );
	device->save_item( NAME(cpustate->xhl) );
	device->save_item( NAME(cpustate->xix) );
	device->save_item( NAME(cpustate->xiy) );
	device->save_item( NAME(cpustate->xiz) );
	device->save_item( NAME(cpustate->xssp) );
	device->save_item( NAME(cpustate->xnsp) );
	device->save_item( NAME(cpustate->pc) );
	device->save_item( NAME(cpustate->sr) );
	device->save_item( NAME(cpustate->f2) );
	device->save_item( NAME(cpustate->dmas) );
	device->save_item( NAME(cpustate->dmad) );
	device->save_item( NAME(cpustate->dmac) );
	device->save_item( NAME(cpustate->dmam) );
	device->save_item( NAME(cpustate->reg) );
	device->save_item( NAME(cpustate->timer_pre) );
	device->save_item( NAME(cpustate->timer) );
	device->save_item( NAME(cpustate->tff1) );
	device->save_item( NAME(cpustate->tff3) );
	device->save_item( NAME(cpustate->timer_change) );
	device->save_item( NAME(cpustate->level) );
	device->save_item( NAME(cpustate->check_irqs) );
	device->save_item( NAME(cpustate->ad_cycles_left) );
	device->save_item( NAME(cpustate->nmi_state) );
}


static CPU_RESET( tlcs900 )
{
	tlcs900_state *cpustate = get_safe_token(device);
	int i;

	cpustate->pc.b.l = RDMEM( 0xFFFF00 );
	cpustate->pc.b.h = RDMEM( 0xFFFF01 );
	cpustate->pc.b.h2 = RDMEM( 0xFFFF02 );
	cpustate->pc.b.h3 = 0;
	/* system mode, iff set to 111, max mode, register bank 0 */
	cpustate->sr.d = 0xF800;
	cpustate->regbank = 0;
	cpustate->xssp.d = 0x0100;
	cpustate->halted = 0;
	cpustate->check_irqs = 0;
	cpustate->ad_cycles_left = 0;
	cpustate->nmi_state = CLEAR_LINE;
	cpustate->timer_pre = 0;
	cpustate->timer_change[0] = 0;
	cpustate->timer_change[1] = 0;
	cpustate->timer_change[2] = 0;
	cpustate->timer_change[3] = 0;

	cpustate->reg[P1] = 0x00;
	cpustate->reg[P1CR] = 0x00;
	cpustate->reg[P2] = 0xff;
	cpustate->reg[P2FC] = 0x00;
	cpustate->reg[P5] = 0x3d;
	cpustate->reg[P5CR] = 0x00;
	cpustate->reg[P5FC] = 0x00;
	cpustate->reg[P6] = 0x3b;
	cpustate->reg[P6FC] = 0x00;
	cpustate->reg[P7] = 0xff;
	cpustate->reg[P7CR] = 0x00;
	cpustate->reg[P7FC] = 0x00;
	cpustate->reg[P8] = 0x3f;
	cpustate->reg[P8CR] = 0x00;
	cpustate->reg[P8FC] = 0x00;
	cpustate->reg[PA] = 0x0f;
	cpustate->reg[PACR] = 0x00;
	cpustate->reg[PAFC] = 0x00;
	cpustate->reg[PB] = 0xff;
	cpustate->reg[PBCR] = 0x00;
	cpustate->reg[PBFC] = 0x00;
	cpustate->reg[MSAR0] = 0xff;
	cpustate->reg[MSAR1] = 0xff;
	cpustate->reg[MSAR2] = 0xff;
	cpustate->reg[MSAR3] = 0xff;
	cpustate->reg[MAMR0] = 0xff;
	cpustate->reg[MAMR1] = 0xff;
	cpustate->reg[MAMR2] = 0xff;
	cpustate->reg[MAMR3] = 0xff;
	cpustate->reg[DREFCR] = 0x00;
	cpustate->reg[DMEMCR] = 0x80;
	cpustate->reg[T01MOD] = 0x00;
	cpustate->reg[T23MOD] = 0x00;
	cpustate->reg[TFFCR] = 0x00;
	cpustate->reg[TRUN] = 0x00;
	cpustate->reg[TRDC] = 0x00;
	cpustate->reg[T4MOD] = 0x20;
	cpustate->reg[T4FFCR] = 0x00;
	cpustate->reg[T5MOD] = 0x20;
	cpustate->reg[T5FFCR] = 0x00;
	cpustate->reg[T45CR] = 0x00;
	cpustate->reg[PG01CR] = 0x00;
	cpustate->reg[PG0REG] = 0x00;
	cpustate->reg[PG1REG] = 0x00;
	cpustate->reg[SC0MOD] = 0x00;
	cpustate->reg[SC0CR] = 0x00;
	cpustate->reg[BR0CR] = 0x00;
	cpustate->reg[SC1MOD] = 0x00;
	cpustate->reg[SC1CR] = 0x00;
	cpustate->reg[BR1CR] = 0x00;
	cpustate->reg[P8FC] = 0x00;
	cpustate->reg[ODE] = 0x00;
	cpustate->reg[ADMOD] = 0x00;
	cpustate->reg[ADREG0L] = 0x3f;
	cpustate->reg[ADREG1L] = 0x3f;
	cpustate->reg[ADREG2L] = 0x3f;
	cpustate->reg[ADREG3L] = 0x3f;
	cpustate->reg[WDMOD] = 0x80;

	for ( i = 0; i < TLCS900_NUM_INPUTS; i++ )
	{
		cpustate->level[i] = CLEAR_LINE;
	}
}


static CPU_EXIT( tlcs900 )
{
}


#include "900tbl.c"


#define NUM_MASKABLE_IRQS	22
static const struct {
	UINT8 reg;
	UINT8 iff;
	UINT8 vector;
} irq_vector_map[NUM_MASKABLE_IRQS] =
{
	{ INTETC32, 0x80, 0x80 },	/* INTTC3 */
	{ INTETC32, 0x08, 0x7c },	/* INTTC2 */
	{ INTETC10, 0x80, 0x78 },	/* INTTC1 */
	{ INTETC10, 0x08, 0x74 },	/* INTTC0 */
	{ INTE0AD, 0x80, 0x70 },	/* INTAD */
	{ INTES1, 0x80, 0x6c },		/* INTTX1 */
	{ INTES1, 0x08, 0x68 },		/* INTRX1 */
	{ INTES0, 0x80, 0x64 },		/* INTTX0 */
	{ INTES0, 0x08, 0x60 },		/* INTRX0 */
	{ INTET76, 0x80, 0x5c },	/* INTTR7 */
	{ INTET76, 0x08, 0x58 },	/* INTTR6 */
	{ INTET54, 0x80, 0x54 },	/* INTTR5 */
	{ INTET54, 0x08, 0x50 },	/* INTTR4 */
	{ INTET32, 0x80, 0x4c },	/* INTT3 */
	{ INTET32, 0x08, 0x48 },	/* INTT2 */
	{ INTET10, 0x80, 0x44 },	/* INTT1 */
	{ INTET10, 0x08, 0x40 },	/* INTT0 */
								/* 0x3c - reserved */
	{ INTE67, 0x80, 0x38 },		/* INT7 */
	{ INTE67, 0x08, 0x34 },		/* INT6 */
	{ INTE45, 0x80, 0x30 },		/* INT5 */
	{ INTE45, 0x08, 0x2c },		/* INT4 */
	{ INTE0AD, 0x08, 0x28 }		/* INT0 */
};


INLINE int tlcs900_process_hdma( tlcs900_state *cpustate, int channel )
{
	UINT8 vector = ( cpustate->reg[0x7c + channel] & 0x1f ) << 2;

	/* Check if any HDMA actions should be performed */
	if ( vector >= 0x28 && vector != 0x3C && vector < 0x74 )
	{
		int irq = 0;

		while( irq < NUM_MASKABLE_IRQS && irq_vector_map[irq].vector != vector )
			irq++;

		/* Check if our interrupt flip-flop is set */
		if ( irq < NUM_MASKABLE_IRQS && cpustate->reg[irq_vector_map[irq].reg] & irq_vector_map[irq].iff )
		{
			switch( cpustate->dmam[channel].b.l & 0x1f )
			{
			case 0x00:
				WRMEM( cpustate->dmad[channel].d, RDMEM( cpustate->dmas[channel].d ) );
				cpustate->dmad[channel].d += 1;
				cpustate->cycles += 8;
				break;
			case 0x01:
				WRMEMW( cpustate->dmad[channel].d, RDMEMW( cpustate->dmas[channel].d ) );
				cpustate->dmad[channel].d += 2;
				cpustate->cycles += 8;
				break;
			case 0x02:
				WRMEML( cpustate->dmad[channel].d, RDMEML( cpustate->dmas[channel].d ) );
				cpustate->dmad[channel].d += 4;
				cpustate->cycles += 12;
				break;
			case 0x04:
				WRMEM( cpustate->dmad[channel].d, RDMEM( cpustate->dmas[channel].d ) );
				cpustate->dmad[channel].d -= 1;
				cpustate->cycles += 8;
				break;
			case 0x05:
				WRMEMW( cpustate->dmad[channel].d, RDMEMW( cpustate->dmas[channel].d ) );
				cpustate->dmad[channel].d -= 2;
				cpustate->cycles += 8;
				break;
			case 0x06:
				WRMEML( cpustate->dmad[channel].d, RDMEML( cpustate->dmas[channel].d ) );
				cpustate->dmad[channel].d -= 4;
				cpustate->cycles += 12;
				break;
			case 0x08:
				WRMEM( cpustate->dmad[channel].d, RDMEM( cpustate->dmas[channel].d ) );
				cpustate->dmas[channel].d += 1;
				cpustate->cycles += 8;
				break;
			case 0x09:
				WRMEMW( cpustate->dmad[channel].d, RDMEMW( cpustate->dmas[channel].d ) );
				cpustate->dmas[channel].d += 2;
				cpustate->cycles += 8;
				break;
			case 0x0a:
				WRMEML( cpustate->dmad[channel].d, RDMEML( cpustate->dmas[channel].d ) );
				cpustate->dmas[channel].d += 4;
				cpustate->cycles += 12;
				break;
			case 0x0c:
				WRMEM( cpustate->dmad[channel].d, RDMEMW( cpustate->dmas[channel].d ) );
				cpustate->dmas[channel].d -= 1;
				cpustate->cycles += 8;
				break;
			case 0x0d:
				WRMEMW( cpustate->dmad[channel].d, RDMEMW( cpustate->dmas[channel].d ) );
				cpustate->dmas[channel].d -= 2;
				cpustate->cycles += 8;
				break;
			case 0x0e:
				WRMEML( cpustate->dmad[channel].d, RDMEML( cpustate->dmas[channel].d ) );
				cpustate->dmas[channel].d -= 4;
				cpustate->cycles += 12;
				break;
			case 0x10:
				WRMEM( cpustate->dmad[channel].d, RDMEMW( cpustate->dmas[channel].d ) );
				cpustate->cycles += 8;
				break;
			case 0x11:
				WRMEMW( cpustate->dmad[channel].d, RDMEMW( cpustate->dmas[channel].d ) );
				cpustate->cycles += 8;
				break;
			case 0x12:
				WRMEML( cpustate->dmad[channel].d, RDMEML( cpustate->dmas[channel].d ) );
				cpustate->cycles += 12;
				break;
			case 0x14:
				cpustate->dmas[channel].d += 1;
				cpustate->cycles += 5;
				break;
			}

			cpustate->dmac[channel].w.l -= 1;

			if ( cpustate->dmac[channel].w.l == 0 )
			{
				cpustate->reg[0x7c + channel] = 0;
				switch( channel )
				{
				case 0:
					cpustate->reg[INTETC10] |= 0x08;
					break;
				case 1:
					cpustate->reg[INTETC10] |= 0x80;
					break;
				case 2:
					cpustate->reg[INTETC32] |= 0x08;
					break;
				case 3:
					cpustate->reg[INTETC32] |= 0x80;
					break;
				}
			}

			/* Clear the interrupt flip-flop */
			cpustate->reg[irq_vector_map[irq].reg] &= ~irq_vector_map[irq].iff;

			return 1;
		}
	}
	return 0;
}


INLINE void tlcs900_check_hdma( tlcs900_state *cpustate )
{
	/* HDMA can only be performed if interrupts are allowed */
	if ( ( cpustate->sr.b.h & 0x70 ) != 0x70 )
	{
		if ( ! tlcs900_process_hdma( cpustate, 0 ) )
		{
			if ( ! tlcs900_process_hdma( cpustate, 1 ) )
			{
				if ( ! tlcs900_process_hdma( cpustate, 2 ) )
				{
					tlcs900_process_hdma( cpustate, 3 );
				}
			}
		}
	}
}


INLINE void tlcs900_check_irqs( tlcs900_state *cpustate )
{
	int irq_vectors[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	int level = 0;
	int irq = -1;
	int i;

	/* Check for NMI */
	if ( cpustate->nmi_state == ASSERT_LINE )
	{
		cpustate->xssp.d -= 4;
		WRMEML( cpustate->xssp.d, cpustate->pc.d );
		cpustate->xssp.d -= 2;
		WRMEMW( cpustate->xssp.d, cpustate->sr.w.l );
		cpustate->pc.d = RDMEML( 0xffff00 + 0x20 );
		cpustate->cycles += 18;

		cpustate->halted = 0;

		cpustate->nmi_state = CLEAR_LINE;

		return;
	}

	/* Check regular irqs */
	for( i = 0; i < NUM_MASKABLE_IRQS; i++ )
	{
		if ( cpustate->reg[irq_vector_map[i].reg] & irq_vector_map[i].iff )
		{
			switch( irq_vector_map[i].iff )
			{
			case 0x80:
				irq_vectors[ ( cpustate->reg[ irq_vector_map[i].reg ] >> 4 ) & 0x07 ] = i;
				break;
			case 0x08:
				irq_vectors[ cpustate->reg[ irq_vector_map[i].reg ] & 0x07 ] = i;
				break;
			}
		}
	}

	/* Check highest allowed priority irq */
	for ( i = MAX( 1, ( ( cpustate->sr.b.h & 0x70 ) >> 4 ) ); i < 7; i++ )
	{
		if ( irq_vectors[i] >= 0 )
		{
			irq = irq_vectors[i];
			level = i + 1;
		}
	}

	/* Take irq */
	if ( irq >= 0 )
	{
		UINT8 vector = irq_vector_map[irq].vector;

		cpustate->xssp.d -= 4;
		WRMEML( cpustate->xssp.d, cpustate->pc.d );
		cpustate->xssp.d -= 2;
		WRMEMW( cpustate->xssp.d, cpustate->sr.w.l );

		/* Mask off any lower priority interrupts  */
		cpustate->sr.b.h = ( cpustate->sr.b.h & 0x8f ) | ( level << 4 );

		cpustate->pc.d = RDMEML( 0xffff00 + vector );
		cpustate->cycles += 18;

		cpustate->halted = 0;

		/* Clear taken IRQ */
		cpustate->reg[ irq_vector_map[irq].reg ] &= ~ irq_vector_map[irq].iff;
	}
}


INLINE void tlcs900_handle_ad( tlcs900_state *cpustate )
{
	if ( cpustate->ad_cycles_left > 0 )
	{
		cpustate->ad_cycles_left -= cpustate->cycles;
		if ( cpustate->ad_cycles_left <= 0 )
		{
			/* Store A/D converted value */
			switch( cpustate->reg[ADMOD] & 0x03 )
			{
			case 0x00:	/* AN0 */
				cpustate->reg[ADREG0L] |= 0xc0;
				cpustate->reg[ADREG0H] = 0xff;
				break;
			case 0x01:	/* AN1 */
			case 0x02:	/* AN2 */
			case 0x03:	/* AN3 */
				break;
			}

			/* Clear BUSY flag, set END flag */
			cpustate->reg[ADMOD] &= ~ 0x40;
			cpustate->reg[ADMOD] |= 0x80;

			cpustate->reg[INTE0AD] |= 0x80;
			cpustate->check_irqs = 1;
		}
	}
}


enum ff_change
{
	FF_CLEAR,
	FF_SET,
	FF_INVERT
};


INLINE void tlcs900_change_tff( tlcs900_state *cpustate, int which, int change )
{
	switch( which )
	{
	case 1:
		switch( change )
		{
		case FF_CLEAR:
			cpustate->tff1 = 0;
			break;
		case FF_SET:
			cpustate->tff1 = 1;
			break;
		case FF_INVERT:
			cpustate->tff1 ^= 1;
			break;
		}
		if ( !cpustate->to1.isnull() )
			cpustate->to1(0, cpustate->tff1 );
		break;

	case 3:
		switch( change )
		{
		case FF_CLEAR:
			cpustate->tff3 = 0;
			break;
		case FF_SET:
			cpustate->tff3 = 1;
			break;
		case FF_INVERT:
			cpustate->tff3 ^= 1;
			break;
		}
		if ( !cpustate->to3.isnull() )
			cpustate->to3(0, cpustate->tff3 );
		break;
	}
}


INLINE void tlcs900_handle_timers( tlcs900_state *cpustate )
{
	UINT32	old_pre = cpustate->timer_pre;

	/* Is the pre-scaler active */
	if ( cpustate->reg[TRUN] & 0x80 )
		cpustate->timer_pre += cpustate->cycles;

	/* Timer 0 */
	if ( cpustate->reg[TRUN] & 0x01 )
	{
		switch( cpustate->reg[T01MOD] & 0x03 )
		{
		case 0x00:	/* TIO */
			break;
		case 0x01:	/* T1 */
			cpustate->timer_change[0] += ( cpustate->timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:	/* T4 */
			cpustate->timer_change[0] += ( cpustate->timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:	/* T16 */
			cpustate->timer_change[0] += ( cpustate->timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; cpustate->timer_change[0] > 0; cpustate->timer_change[0]-- )
		{
//printf("timer0 = %02x, TREG0 = %02x\n", cpustate->timer[0], cpustate->reg[TREG0] );
			cpustate->timer[0] += 1;
			if ( cpustate->timer[0] == cpustate->reg[TREG0] )
			{
				if ( ( cpustate->reg[T01MOD] & 0x0c ) == 0x00 )
				{
					cpustate->timer_change[1] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( cpustate->reg[T01MOD] & 0xc0 ) != 0x40 )
				{
					cpustate->timer[0] = 0;
					cpustate->reg[INTET10] |= 0x08;
				}
			}
		}
	}

	/* Timer 1 */
	if ( cpustate->reg[TRUN] & 0x02 )
	{
		switch( ( cpustate->reg[T01MOD] >> 2 ) & 0x03 )
		{
		case 0x00:	/* TO0TRG */
			break;
		case 0x01:	/* T1 */
			cpustate->timer_change[1] += ( cpustate->timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:	/* T16 */
			cpustate->timer_change[1] += ( cpustate->timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:	/* T256 */
			cpustate->timer_change[1] += ( cpustate->timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; cpustate->timer_change[1] > 0; cpustate->timer_change[1]-- )
		{
			cpustate->timer[1] += 1;
			if ( cpustate->timer[1] == cpustate->reg[TREG1] )
			{
				cpustate->timer[1] = 0;
				cpustate->reg[INTET10] |= 0x80;

				if ( cpustate->reg[TFFCR] & 0x02 )
				{
					tlcs900_change_tff( cpustate, 1, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 0 */
				if ( ( cpustate->reg[T01MOD] & 0xc0 ) == 0x40 )
				{
					cpustate->timer[0] = 0;
				}
			}
		}
	}

	/* Timer 2 */
	if ( cpustate->reg[TRUN] & 0x04 )
	{
		switch( cpustate->reg[T23MOD] & 0x03 )
		{
		case 0x00:	/* invalid */
		case 0x01:	/* T1 */
			cpustate->timer_change[2] += ( cpustate->timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:	/* T4 */
			cpustate->timer_change[2] += ( cpustate->timer_pre >> 9 ) - ( old_pre >> 9 );
			break;
		case 0x03:	/* T16 */
			cpustate->timer_change[2] += ( cpustate->timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		}

		for( ; cpustate->timer_change[2] > 0; cpustate->timer_change[2]-- )
		{
			cpustate->timer[2] += 1;
			if ( cpustate->timer[2] == cpustate->reg[TREG2] )
			{
				if ( ( cpustate->reg[T23MOD] & 0x0c ) == 0x00 )
				{
					cpustate->timer_change[3] += 1;
				}

				/* In 16bit timer mode the timer should not be reset */
				if ( ( cpustate->reg[T23MOD] & 0xc0 ) != 0x40 )
				{
					cpustate->timer[2] = 0;
					cpustate->reg[INTET32] |= 0x08;
				}
			}
		}
	}

	/* Timer 3 */
	if ( cpustate->reg[TRUN] & 0x08 )
	{
		switch( ( cpustate->reg[T23MOD] >> 2 ) & 0x03 )
		{
		case 0x00:	/* TO2TRG */
			break;
		case 0x01:	/* T1 */
			cpustate->timer_change[3] += ( cpustate->timer_pre >> 7 ) - ( old_pre >> 7 );
			break;
		case 0x02:	/* T16 */
			cpustate->timer_change[3] += ( cpustate->timer_pre >> 11 ) - ( old_pre >> 11 );
			break;
		case 0x03:	/* T256 */
			cpustate->timer_change[3] += ( cpustate->timer_pre >> 15 ) - ( old_pre >> 15 );
			break;
		}

		for( ; cpustate->timer_change[3] > 0; cpustate->timer_change[3]-- )
		{
			cpustate->timer[3] += 1;
			if ( cpustate->timer[3] == cpustate->reg[TREG3] )
			{
				cpustate->timer[3] = 0;
				cpustate->reg[INTET32] |= 0x80;

				if ( cpustate->reg[TFFCR] & 0x20 )
				{
					tlcs900_change_tff( cpustate, 3, FF_INVERT );
				}

				/* In 16bit timer mode also reset timer 2 */
				if ( ( cpustate->reg[T23MOD] & 0xc0 ) == 0x40 )
				{
					cpustate->timer[2] = 0;
				}
			}
		}
	}

	cpustate->timer_pre &= 0xffffff;
}


static CPU_EXECUTE( tlcs900 )
{
	tlcs900_state *cpustate = get_safe_token(device);

	do
	{
		const tlcs900inst *inst;

		cpustate->cycles = 0;

		if ( cpustate->check_irqs )
		{
			tlcs900_check_irqs( cpustate );
			cpustate->check_irqs = 0;
		}

		debugger_instruction_hook( device, cpustate->pc.d );

		if ( cpustate->halted )
		{
			cpustate->cycles += 8;
		}
		else
		{
			cpustate->op = RDOP();
			inst = &mnemonic[cpustate->op];
			prepare_operands( cpustate, inst );

			/* Execute the instruction */
			inst->opfunc( cpustate );
			cpustate->cycles += inst->cycles;
		}

		tlcs900_handle_ad( cpustate );

		tlcs900_handle_timers( cpustate );

		tlcs900_check_hdma( cpustate );

		cpustate->icount -= cpustate->cycles;
	} while ( cpustate->icount > 0 );
}


static void tlcs900_input_level_change( tlcs900_state *cpustate, int input, int level )
{
	switch( input )
	{
	case INPUT_LINE_NMI:
	case TLCS900_NMI:
		if ( cpustate->level[TLCS900_NMI] == CLEAR_LINE && level == ASSERT_LINE )
		{
			cpustate->nmi_state = level;
		}
		cpustate->level[TLCS900_NMI] = level;
		break;

	case TLCS900_INTWD:
		break;

	case TLCS900_INT0:
		/* Is INT0 functionality enabled? */
		if ( cpustate->reg[IIMC] & 0x04 )
		{
			if ( cpustate->reg[IIMC] & 0x02 )
			{
				/* Rising edge detect */
				if ( cpustate->level[TLCS900_INT0] == CLEAR_LINE && level == ASSERT_LINE )
				{
					/* Leave HALT state */
					cpustate->halted = 0;
					cpustate->reg[INTE0AD] |= 0x08;
				}
			}
			else
			{
				/* Level detect */
				if ( level == ASSERT_LINE )
					cpustate->reg[INTE0AD] |= 0x08;
				else
					cpustate->reg[INTE0AD] &= ~ 0x08;
			}
		}
		cpustate->level[TLCS900_INT0] = level;
		break;

	case TLCS900_INT4:
		if ( ! ( cpustate->reg[PBCR] & 0x01 ) )
		{
			if ( cpustate->level[TLCS900_INT4] == CLEAR_LINE && level == ASSERT_LINE )
			{
				cpustate->reg[INTE45] |= 0x08;
			}
		}
		cpustate->level[TLCS900_INT4] = level;
		break;

	case TLCS900_INT5:
		if ( ! ( cpustate->reg[PBCR] & 0x02 ) )
		{
			if ( cpustate->level[TLCS900_INT5] == CLEAR_LINE && level == ASSERT_LINE )
			{
				cpustate->reg[INTE45] |= 0x80;
			}
		}
		cpustate->level[TLCS900_INT5] = level;
		break;

	case TLCS900_TIO:	/* External timer input for timer 0 */
		if ( ( cpustate->reg[TRUN] & 0x01 ) && ( cpustate->reg[T01MOD] & 0x03 ) == 0x00 )
		{
			if ( cpustate->level[TLCS900_TIO] == CLEAR_LINE && level == ASSERT_LINE )
			{
				cpustate->timer_change[0] += 1;
			}
		}
		cpustate->level[TLCS900_TIO] = level;
		break;
	}
	cpustate->check_irqs = 1;
}


static READ8_HANDLER( tlcs900_internal_r )
{
	tlcs900_state *cpustate = get_safe_token( &space->device() );

	return cpustate->reg[ offset ];
}


static WRITE8_HANDLER( tlcs900_internal_w )
{
	tlcs900_state *cpustate = get_safe_token( &space->device() );

	switch ( offset )
	{
	case TRUN:
		if ( ! ( data & 0x01 ) )
		{
			cpustate->timer[0] = 0;
			cpustate->timer_change[0] = 0;
		}
		if ( ! ( data & 0x02 ) )
		{
			cpustate->timer[1] = 0;
			cpustate->timer_change[1] = 0;
		}
		if ( ! ( data & 0x04 ) )
		{
			cpustate->timer[2] = 0;
			cpustate->timer_change[2] = 0;
		}
		if ( ! ( data & 0x08 ) )
		{
			cpustate->timer[3] = 0;
			cpustate->timer_change[3] = 0;
		}
		if ( ! ( data & 0x10 ) )
			cpustate->timer[4] = 0;
		if ( ! ( data & 0x20 ) )
			cpustate->timer[5] = 0;
		break;

	case TFFCR:
		switch( data & 0x0c )
		{
		case 0x00:
			tlcs900_change_tff( cpustate, 1, FF_INVERT );
			break;
		case 0x04:
			tlcs900_change_tff( cpustate, 1, FF_SET );
			break;
		case 0x08:
			tlcs900_change_tff( cpustate, 1, FF_CLEAR );
			break;
		}
		switch( data & 0xc0 )
		{
		case 0x00:
			tlcs900_change_tff( cpustate, 3, FF_INVERT );
			break;
		case 0x40:
			tlcs900_change_tff( cpustate, 3, FF_SET );
			break;
		case 0x80:
			tlcs900_change_tff( cpustate, 3, FF_CLEAR );
			break;
		}
		break;
	case MSAR0:
	case MAMR0:
	case MSAR1:
	case MAMR1:
		break;

	case ADMOD:
		/* Preserve read-only bits */
		data = ( cpustate->reg[ADMOD] & 0xc0 ) | ( data & 0x3f );

		/* Check for A/D request start */
		if ( data & 0x04 )
		{
			data &= ~0x04;
			data |= 0x40;
			cpustate->ad_cycles_left = ( data & 0x08 ) ? 640 : 320;
		}
		break;

	case WDMOD:
	case WDCR:
		break;

	case INTE0AD:
	case INTE45:
	case INTE67:
	case INTET10:
	case INTET32:
	case INTET54:
	case INTET76:
	case INTES0:
	case INTES1:
	case INTETC10:
	case INTETC32:
		if ( data & 0x80 )
			data = ( data & 0x7f ) | ( cpustate->reg[offset] & 0x80 );
		if ( data & 0x08 )
			data = ( data & 0xf7 ) | ( cpustate->reg[offset] & 0x08 );
		break;

	case IIMC:
		break;

	default:
		break;
	}

	cpustate->check_irqs = 1;
	cpustate->reg[ offset ] = data;
}


static ADDRESS_MAP_START( tlcs900_mem, AS_PROGRAM, 8, legacy_cpu_device )
	AM_RANGE( 0x000000, 0x00007f ) AM_READWRITE_LEGACY( tlcs900_internal_r, tlcs900_internal_w )
ADDRESS_MAP_END


static CPU_SET_INFO( tlcs900 )
{
	tlcs900_state *cpustate = get_safe_token( device );

	switch ( state )
	{
	case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:
	case CPUINFO_INT_INPUT_STATE + TLCS900_NMI:
	case CPUINFO_INT_INPUT_STATE + TLCS900_INTWD:
	case CPUINFO_INT_INPUT_STATE + TLCS900_INT0:
	case CPUINFO_INT_INPUT_STATE + TLCS900_INT4:
	case CPUINFO_INT_INPUT_STATE + TLCS900_INT5:
	case CPUINFO_INT_INPUT_STATE + TLCS900_TIO:
		tlcs900_input_level_change( cpustate, state - CPUINFO_INT_INPUT_STATE, info->i ); break;
	}
}


CPU_GET_INFO( tlcs900h )
{
	tlcs900_state *cpustate = ( device != NULL && device->token() != NULL ) ? get_safe_token(device) : NULL;

	switch( state )
	{
	case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tlcs900_state); break;
	case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE; break;
	case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1; break;
	case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1; break;
	case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1; break;
	case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 7; break;	/* FIXME */
	case CPUINFO_INT_MIN_CYCLES:					info->i = 1; break; /* FIXME */
	case CPUINFO_INT_MAX_CYCLES:					info->i = 1; break; /* FIXME */
	case CPUINFO_INT_INPUT_LINES:					info->i = 1; break;

	case DEVINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 8; break;
	case DEVINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 24; break;
	case DEVINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:			info->i = 0; break;

	case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:
	case CPUINFO_INT_INPUT_STATE + TLCS900_NMI:		info->i = cpustate->level[TLCS900_NMI]; break;
	case CPUINFO_INT_INPUT_STATE + TLCS900_INTWD:	info->i = cpustate->level[TLCS900_INTWD]; break;
	case CPUINFO_INT_INPUT_STATE + TLCS900_INT0:	info->i = cpustate->level[TLCS900_INT0]; break;
	case CPUINFO_INT_INPUT_STATE + TLCS900_INT4:	info->i = cpustate->level[TLCS900_INT4]; break;
	case CPUINFO_INT_INPUT_STATE + TLCS900_INT5:	info->i = cpustate->level[TLCS900_INT5]; break;
	case CPUINFO_INT_INPUT_STATE + TLCS900_TIO:		info->i = cpustate->level[TLCS900_TIO]; break;

	case CPUINFO_INT_PC:							info->i = cpustate->pc.d; break;
	case CPUINFO_INT_REGISTER + TLCS900_PC:			info->i = cpustate->pc.d; break;
	case CPUINFO_INT_REGISTER + TLCS900_SR:			info->i = cpustate->sr.d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XWA0:		info->i = cpustate->xwa[0].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XBC0:		info->i = cpustate->xbc[0].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XDE0:		info->i = cpustate->xde[0].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XHL0:		info->i = cpustate->xhl[0].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XWA1:		info->i = cpustate->xwa[1].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XBC1:		info->i = cpustate->xbc[1].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XDE1:		info->i = cpustate->xde[1].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XHL1:		info->i = cpustate->xhl[1].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XWA2:		info->i = cpustate->xwa[2].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XBC2:		info->i = cpustate->xbc[2].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XDE2:		info->i = cpustate->xde[2].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XHL2:		info->i = cpustate->xhl[2].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XWA3:		info->i = cpustate->xwa[3].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XBC3:		info->i = cpustate->xbc[3].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XDE3:		info->i = cpustate->xde[3].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XHL3:		info->i = cpustate->xhl[3].d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XIX:		info->i = cpustate->xix.d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XIY:		info->i = cpustate->xiy.d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XIZ:		info->i = cpustate->xiz.d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XNSP:		info->i = cpustate->xnsp.d; break;
	case CPUINFO_INT_REGISTER + TLCS900_XSSP:		info->i = cpustate->xssp.d; break;

	case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(tlcs900); break;
	case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(tlcs900); break;
	case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tlcs900); break;
	case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(tlcs900); break;
	case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(tlcs900); break;
	case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(tlcs900); break;
	case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount; break;
	case DEVINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map8 = ADDRESS_MAP_NAME(tlcs900_mem); break;

	case CPUINFO_STR_REGISTER + TLCS900_PC:			sprintf( info->s, "PC:%08x", cpustate->pc.d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_SR:			sprintf( info->s, "SR:%c%d%c%d%c%c%c%c%c%c%c%c",
														cpustate->sr.w.l & 0x8000 ? 'S' : 'U',
														( cpustate->sr.w.l & 0x7000 ) >> 12,
														cpustate->sr.w.l & 0x0800 ? 'M' : 'N',
														( cpustate->sr.w.l & 0x0700 ) >> 8,
														cpustate->sr.w.l & 0x0080 ? 'S' : '.',
														cpustate->sr.w.l & 0x0040 ? 'Z' : '.',
														cpustate->sr.w.l & 0x0020 ? '1' : '.',
														cpustate->sr.w.l & 0x0010 ? 'H' : '.',
														cpustate->sr.w.l & 0x0008 ? '1' : '.',
														cpustate->sr.w.l & 0x0004 ? 'V' : '.',
														cpustate->sr.w.l & 0x0002 ? 'N' : '.',
														cpustate->sr.w.l & 0x0001 ? 'C' : '.' );
													break;
	case CPUINFO_STR_REGISTER + TLCS900_XWA0:		sprintf( info->s, "XWA0:%08x", cpustate->xwa[0].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XBC0:		sprintf( info->s, "XBC0:%08x", cpustate->xbc[0].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XDE0:		sprintf( info->s, "XDE0:%08x", cpustate->xde[0].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XHL0:		sprintf( info->s, "XHL0:%08x", cpustate->xhl[0].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XWA1:		sprintf( info->s, "XWA1:%08x", cpustate->xwa[1].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XBC1:		sprintf( info->s, "XBC1:%08x", cpustate->xbc[1].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XDE1:		sprintf( info->s, "XDE1:%08x", cpustate->xde[1].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XHL1:		sprintf( info->s, "XHL1:%08x", cpustate->xhl[1].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XWA2:		sprintf( info->s, "XWA2:%08x", cpustate->xwa[2].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XBC2:		sprintf( info->s, "XBC2:%08x", cpustate->xbc[2].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XDE2:		sprintf( info->s, "XDE2:%08x", cpustate->xde[2].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XHL2:		sprintf( info->s, "XHL2:%08x", cpustate->xhl[2].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XWA3:		sprintf( info->s, "XWA3:%08x", cpustate->xwa[3].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XBC3:		sprintf( info->s, "XBC3:%08x", cpustate->xbc[3].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XDE3:		sprintf( info->s, "XDE3:%08x", cpustate->xde[3].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XHL3:		sprintf( info->s, "XHL3:%08x", cpustate->xhl[3].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XIX:		sprintf( info->s, "XIX:%08x", cpustate->xix.d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XIY:		sprintf( info->s, "XIY:%08x", cpustate->xiy.d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XIZ:		sprintf( info->s, "XIZ:%08x", cpustate->xiz.d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XNSP:		sprintf( info->s, "XNSP:%08x", cpustate->xnsp.d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_XSSP:		sprintf( info->s, "XSSP:%08x", cpustate->xssp.d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAS0:		sprintf( info->s, "DMAS0:%08x", cpustate->dmas[0].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAD0:		sprintf( info->s, "DMAD0:%08x", cpustate->dmad[0].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAC0:		sprintf( info->s, "DMAC0:%04x", cpustate->dmac[0].w.l ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAM0:		sprintf( info->s, "DMAM0:%02x", cpustate->dmam[0].b.l ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAS1:		sprintf( info->s, "DMAS0:%08x", cpustate->dmas[1].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAD1:		sprintf( info->s, "DMAD0:%08x", cpustate->dmad[1].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAC1:		sprintf( info->s, "DMAC0:%04x", cpustate->dmac[1].w.l ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAM1:		sprintf( info->s, "DMAM0:%02x", cpustate->dmam[1].b.l ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAS2:		sprintf( info->s, "DMAS0:%08x", cpustate->dmas[2].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAD2:		sprintf( info->s, "DMAD0:%08x", cpustate->dmad[2].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAC2:		sprintf( info->s, "DMAC0:%04x", cpustate->dmac[2].w.l ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAM2:		sprintf( info->s, "DMAM0:%02x", cpustate->dmam[2].b.l ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAS3:		sprintf( info->s, "DMAS0:%08x", cpustate->dmas[3].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAD3:		sprintf( info->s, "DMAD0:%08x", cpustate->dmad[3].d ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAC3:		sprintf( info->s, "DMAC0:%04x", cpustate->dmac[3].w.l ); break;
	case CPUINFO_STR_REGISTER + TLCS900_DMAM3:		sprintf( info->s, "DMAM0:%02x", cpustate->dmam[3].b.l ); break;

	case DEVINFO_STR_NAME:							strcpy( info->s, "TLCS-900/H" ); break;
	case DEVINFO_STR_FAMILY:					strcpy( info->s, "Toshiba TLCS-900" ); break;
	case DEVINFO_STR_VERSION:					strcpy( info->s, "0.1" ); break;
	case DEVINFO_STR_SOURCE_FILE:						strcpy( info->s, __FILE__ ); break;
	case DEVINFO_STR_CREDITS:					strcpy( info->s, "Copyright Wilbert Pol" ); break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(TLCS900H, tlcs900h);
