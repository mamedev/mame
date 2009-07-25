/**************************************************************************
 *
 * 39in1.c - bootleg MAME-based "39-in-1" arcade PCB
 * Skeleton by R. Belmont, thanks to the Guru
 * Decrypt by Andreas Naive
 *
 * CPU: Intel Xscale PXA255 series @ 200 MHz, configured little-endian
 * Xscale PXA consists of:
 *    ARMv5TE instruction set without the FPU
 *    ARM standard MMU
 *    ARM DSP extensions
 *    VGA-ish frame buffer with some 2D acceleration features
 *    AC97 stereo audio CODEC
 *
 * PCB also contains a custom ASIC, probably used for the decryption
 *
 * TODO:
 *   PXA255 peripherals
 *
 **************************************************************************/

#include "driver.h"
#include "video/generic.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#define VERBOSE_LEVEL ( 3 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine *machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%s: %s", cpuexec_describe_context(machine), buf );
	}
}

/*

  PXA255 DMA registers (placeholder)

  pg. 151 to 182, PXA255 Processor Developers Manual [278693-002].pdf

*/

#define PXA255_DMA_BASE_ADDR	(0x40000000)
#define PXA255_DCSR0			(PXA255_DMA_BASE_ADDR + 0x00000000)
#define PXA255_DCSR1			(PXA255_DMA_BASE_ADDR + 0x00000004)
#define PXA255_DCSR2			(PXA255_DMA_BASE_ADDR + 0x00000008)
#define PXA255_DCSR3			(PXA255_DMA_BASE_ADDR + 0x0000000c)
#define PXA255_DCSR4			(PXA255_DMA_BASE_ADDR + 0x00000010)
#define PXA255_DCSR5			(PXA255_DMA_BASE_ADDR + 0x00000014)
#define PXA255_DCSR6			(PXA255_DMA_BASE_ADDR + 0x00000018)
#define PXA255_DCSR7			(PXA255_DMA_BASE_ADDR + 0x0000001c)
#define PXA255_DCSR8			(PXA255_DMA_BASE_ADDR + 0x00000020)
#define PXA255_DCSR9			(PXA255_DMA_BASE_ADDR + 0x00000024)
#define PXA255_DCSR10			(PXA255_DMA_BASE_ADDR + 0x00000028)
#define PXA255_DCSR11			(PXA255_DMA_BASE_ADDR + 0x0000002c)
#define PXA255_DCSR12			(PXA255_DMA_BASE_ADDR + 0x00000030)
#define PXA255_DCSR13			(PXA255_DMA_BASE_ADDR + 0x00000034)
#define PXA255_DCSR14			(PXA255_DMA_BASE_ADDR + 0x00000038)
#define PXA255_DCSR15			(PXA255_DMA_BASE_ADDR + 0x0000003c)
/* More DMA registers TODO */

static READ32_HANDLER( pxa255_dma_r )
{
	verboselog( space->machine, 0, "pxa255_dma_r: Placeholder functionality.  Read: %08x & %08x\n", PXA255_DMA_BASE_ADDR | (offset << 2), mem_mask );
	/* TODO: Everything placeholder, DCSR3 just needs to flag that it's ready to accept data so everything boots. */
	switch(PXA255_DMA_BASE_ADDR | (offset << 2))
	{
		case PXA255_DCSR0:
		case PXA255_DCSR1:
		case PXA255_DCSR2:
		case PXA255_DCSR3:
		case PXA255_DCSR4:
		case PXA255_DCSR5:
		case PXA255_DCSR6:
		case PXA255_DCSR7:
		case PXA255_DCSR8:
		case PXA255_DCSR9:
		case PXA255_DCSR10:
		case PXA255_DCSR11:
		case PXA255_DCSR12:
		case PXA255_DCSR13:
		case PXA255_DCSR14:
		case PXA255_DCSR15:
			return 0x00000008; // Report channel as stopped
		default:
			break;
	}
	return 0;
}

/*

  PXA255 OS Timer registers

  pg. 138 to 142, PXA255 Processor Developers Manual [278693-002].pdf

*/

#define PXA255_OSTMR_BASE_ADDR	(0x40a00000)
#define PXA255_OSMR0			(PXA255_OSTMR_BASE_ADDR + 0x00000000)
#define PXA255_OSMR1			(PXA255_OSTMR_BASE_ADDR + 0x00000004)
#define PXA255_OSMR2			(PXA255_OSTMR_BASE_ADDR + 0x00000008)
#define PXA255_OSMR3			(PXA255_OSTMR_BASE_ADDR + 0x0000000c)
#define PXA255_OSCR				(PXA255_OSTMR_BASE_ADDR + 0x00000010)
#define PXA255_OSSR				(PXA255_OSTMR_BASE_ADDR + 0x00000014)
	#define PXA255_OSSR_M0		(0x00000001)
	#define PXA255_OSSR_M1		(0x00000002)
	#define PXA255_OSSR_M2		(0x00000004)
	#define PXA255_OSSR_M3		(0x00000008)
#define PXA255_OWER				(PXA255_OSTMR_BASE_ADDR + 0x00000018)
#define PXA255_OIER				(PXA255_OSTMR_BASE_ADDR + 0x0000001c)
	#define PXA255_OIER_E0		(0x00000001)
	#define PXA255_OIER_E1		(0x00000002)
	#define PXA255_OIER_E2		(0x00000004)
	#define PXA255_OIER_E3		(0x00000008)

typedef struct
{
	UINT32 osmr0;
	UINT32 osmr1;
	UINT32 osmr2;
	UINT32 osmr3;
	UINT32 oscr;
	UINT32 ossr;
	UINT32 ower;
	UINT32 oier;
} PXA255_OSTMR_Regs;

static PXA255_OSTMR_Regs ostimer_regs;

static READ32_HANDLER( pxa255_ostimer_r )
{
	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			verboselog( space->machine, 3, "pxa255_ostimer_r: OS Timer Match Register 0: %08x & %08x\n", ostimer_regs.osmr0, mem_mask );
			return ostimer_regs.osmr0;
		case PXA255_OSMR1:
			verboselog( space->machine, 3, "pxa255_ostimer_r: OS Timer Match Register 1: %08x & %08x\n", ostimer_regs.osmr1, mem_mask );
			return ostimer_regs.osmr1;
		case PXA255_OSMR2:
			verboselog( space->machine, 3, "pxa255_ostimer_r: OS Timer Match Register 2: %08x & %08x\n", ostimer_regs.osmr2, mem_mask );
			return ostimer_regs.osmr2;
		case PXA255_OSMR3:
			verboselog( space->machine, 3, "pxa255_ostimer_r: OS Timer Match Register 3: %08x & %08x\n", ostimer_regs.osmr3, mem_mask );
			return ostimer_regs.osmr3;
		case PXA255_OSCR:
			verboselog( space->machine, 4, "pxa255_ostimer_r: OS Timer Count Register: %08x & %08x\n", ostimer_regs.oscr, mem_mask );
			// free-running 3.something MHz counter.  this is a complete hack.
			ostimer_regs.oscr += 0x300;
			return ostimer_regs.oscr;
		case PXA255_OSSR:
			verboselog( space->machine, 3, "pxa255_ostimer_r: OS Timer Status Register: %08x & %08x\n", ostimer_regs.ossr, mem_mask );
			return ostimer_regs.ossr;
		case PXA255_OWER:
			verboselog( space->machine, 3, "pxa255_ostimer_r: OS Timer Watchdog Match Enable Register: %08x & %08x\n", ostimer_regs.ower, mem_mask );
			return ostimer_regs.ower;
		case PXA255_OIER:
			verboselog( space->machine, 3, "pxa255_ostimer_r: OS Timer Interrupt Enable Register: %08x & %08x\n", ostimer_regs.oier, mem_mask );
			return ostimer_regs.oier;
		default:
			verboselog( space->machine, 0, "pxa255_ostimer_r: Unknown address: %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}


static WRITE32_HANDLER( pxa255_ostimer_w )
{
	switch(PXA255_OSTMR_BASE_ADDR | (offset << 2))
	{
		case PXA255_OSMR0:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Match Register 0: %08x & %08x\n", data, mem_mask );
			ostimer_regs.osmr0 = data;
			break;
		case PXA255_OSMR1:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Match Register 1: %08x & %08x\n", data, mem_mask );
			ostimer_regs.osmr1 = data;
			break;
		case PXA255_OSMR2:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Match Register 2: %08x & %08x\n", data, mem_mask );
			ostimer_regs.osmr2 = data;
			break;
		case PXA255_OSMR3:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Match Register 3: %08x & %08x\n", data, mem_mask );
			ostimer_regs.osmr3 = data;
			break;
		case PXA255_OSCR:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Count Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs.oscr = data;
			break;
		case PXA255_OSSR:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Status Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs.ossr &= ~data;
			break;
		case PXA255_OWER:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Watchdog Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs.ower = data & 0x00000001;
			break;
		case PXA255_OIER:
			verboselog( space->machine, 3, "pxa255_ostimer_w: OS Timer Interrupt Enable Register: %08x & %08x\n", data, mem_mask );
			ostimer_regs.oier = data & 0x0000000f;
			break;
		default:
			verboselog( space->machine, 0, "pxa255_ostimer_w: Unknown address: %08x = %08x & %08x\n", PXA255_OSTMR_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 Interrupt registers

  pg. 124 to 132, PXA255 Processor Developers Manual [278693-002].pdf

*/

#define PXA255_INTC_BASE_ADDR	(0x40d00000)
#define PXA255_ICIP				(PXA255_INTC_BASE_ADDR + 0x00000000)
#define PXA255_ICMR				(PXA255_INTC_BASE_ADDR + 0x00000004)
#define PXA255_ICLR				(PXA255_INTC_BASE_ADDR + 0x00000008)
#define PXA255_ICFP				(PXA255_INTC_BASE_ADDR + 0x0000000c)
#define PXA255_ICPR				(PXA255_INTC_BASE_ADDR + 0x00000010)
#define PXA255_ICCR				(PXA255_INTC_BASE_ADDR + 0x00000014)

#define PXA255_INT_HUART		(1 << 7)
#define PXA255_INT_GPIO0		(1 << 8)
#define PXA255_INT_GPIO1		(1 << 9)
#define PXA255_INT_GPIO84_2		(1 << 10)
#define PXA255_INT_USB			(1 << 11)
#define PXA255_INT_PMU			(1 << 12)
#define PXA255_INT_I2S			(1 << 13)
#define PXA255_INT_AC97			(1 << 14)
#define PXA255_INT_NETWORK		(1 << 16)
#define PXA255_INT_LCD			(1 << 17)
#define PXA255_INT_I2C			(1 << 18)
#define PXA255_INT_ICP			(1 << 19)
#define PXA255_INT_STUART		(1 << 20)
#define PXA255_INT_BTUART		(1 << 21)
#define PXA255_INT_FFUART		(1 << 22)
#define PXA255_INT_MMC			(1 << 23)
#define PXA255_INT_SSP			(1 << 24)
#define PXA255_INT_DMA			(1 << 25)
#define PXA255_INT_OSTIMER0		(1 << 26)
#define PXA255_INT_OSTIMER1		(1 << 27)
#define PXA255_INT_OSTIMER2		(1 << 28)
#define PXA255_INT_OSTIMER3		(1 << 29)
#define PXA255_INT_RTC_HZ		(1 << 30)
#define PXA255_INT_RTC_ALARM	(1 << 31)

typedef struct
{
	UINT32 icip;
	UINT32 icmr;
	UINT32 iclr;
	UINT32 icfp;
	UINT32 icpr;
	UINT32 iccr;
} PXA255_INTC_Regs;

static PXA255_INTC_Regs intc_regs;

static void pxa255_update_interrupts(running_machine *machine)
{
	intc_regs.icfp = (intc_regs.icpr & intc_regs.icmr) & intc_regs.iclr;
	intc_regs.icip = (intc_regs.icpr & intc_regs.icmr) & (~intc_regs.iclr);
	cputag_set_input_line(machine, "maincpu", ARM7_FIRQ_LINE, intc_regs.icfp ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", ARM7_IRQ_LINE,  intc_regs.icip ? ASSERT_LINE : CLEAR_LINE);
}

static void pxa255_set_irq_line(running_machine *machine, UINT32 line, int state)
{
	intc_regs.icpr &= ~line;
	intc_regs.icpr |= state ? line : 0;
	pxa255_update_interrupts(machine);
}

static READ32_HANDLER( pxa255_intc_r )
{
	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			verboselog( space->machine, 3, "pxa255_intc_r: Interrupt Controller IRQ Pending Register: %08x & %08x\n", intc_regs.icip, mem_mask );
			return intc_regs.icip;
		case PXA255_ICMR:
			verboselog( space->machine, 3, "pxa255_intc_r: Interrupt Controller Mask Register: %08x & %08x\n", intc_regs.icmr, mem_mask );
			return intc_regs.icmr;
		case PXA255_ICLR:
			verboselog( space->machine, 3, "pxa255_intc_r: Interrupt Controller Level Register: %08x & %08x\n", intc_regs.iclr, mem_mask );
			return intc_regs.iclr;
		case PXA255_ICFP:
			verboselog( space->machine, 3, "pxa255_intc_r: Interrupt Controller FIQ Pending Register: %08x & %08x\n", intc_regs.icfp, mem_mask );
			return intc_regs.icfp;
		case PXA255_ICPR:
			verboselog( space->machine, 3, "pxa255_intc_r: Interrupt Controller Pending Register: %08x & %08x\n", intc_regs.icpr, mem_mask );
			return intc_regs.icpr;
		case PXA255_ICCR:
			verboselog( space->machine, 3, "pxa255_intc_r: Interrupt Controller Control Register: %08x & %08x\n", intc_regs.iccr, mem_mask );
			return intc_regs.iccr;
		default:
			verboselog( space->machine, 0, "pxa255_intc_r: Unknown address: %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

static WRITE32_HANDLER( pxa255_intc_w )
{
	switch(PXA255_INTC_BASE_ADDR | (offset << 2))
	{
		case PXA255_ICIP:
			verboselog( space->machine, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller IRQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICMR:
			verboselog( space->machine, 3, "pxa255_intc_w: Interrupt Controller Mask Register: %08x & %08x\n", data, mem_mask );
			intc_regs.icmr = data & 0xfffe7f00;
			break;
		case PXA255_ICLR:
			verboselog( space->machine, 3, "pxa255_intc_w: Interrupt Controller Level Register: %08x & %08x\n", data, mem_mask );
			intc_regs.iclr = data & 0xfffe7f00;
			break;
		case PXA255_ICFP:
			verboselog( space->machine, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller FIQ Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICPR:
			verboselog( space->machine, 3, "pxa255_intc_w: (Invalid Write) Interrupt Controller Pending Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_ICCR:
			verboselog( space->machine, 3, "pxa255_intc_w: Interrupt Controller Control Register: %08x & %08x\n", data, mem_mask );
			intc_regs.iccr = data & 0x00000001;
			break;
		default:
			verboselog( space->machine, 0, "pxa255_intc_w: Unknown address: %08x = %08x & %08x\n", PXA255_INTC_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 General-Purpose I/O registers

  pg. 105 to 124, PXA255 Processor Developers Manual [278693-002].pdf

*/

#define PXA255_GPIO_BASE_ADDR	(0x40E00000)
#define PXA255_GPLR0			(PXA255_GPIO_BASE_ADDR + 0x00000000)
#define PXA255_GPLR1			(PXA255_GPIO_BASE_ADDR + 0x00000004)
#define PXA255_GPLR2			(PXA255_GPIO_BASE_ADDR + 0x00000008)
#define PXA255_GPDR0			(PXA255_GPIO_BASE_ADDR + 0x0000000c)
#define PXA255_GPDR1			(PXA255_GPIO_BASE_ADDR + 0x00000010)
#define PXA255_GPDR2			(PXA255_GPIO_BASE_ADDR + 0x00000014)
#define PXA255_GPSR0			(PXA255_GPIO_BASE_ADDR + 0x00000018)
#define PXA255_GPSR1			(PXA255_GPIO_BASE_ADDR + 0x0000001c)
#define PXA255_GPSR2			(PXA255_GPIO_BASE_ADDR + 0x00000020)
#define PXA255_GPCR0			(PXA255_GPIO_BASE_ADDR + 0x00000024)
#define PXA255_GPCR1			(PXA255_GPIO_BASE_ADDR + 0x00000028)
#define PXA255_GPCR2			(PXA255_GPIO_BASE_ADDR + 0x0000002c)
#define PXA255_GRER0			(PXA255_GPIO_BASE_ADDR + 0x00000030)
#define PXA255_GRER1			(PXA255_GPIO_BASE_ADDR + 0x00000034)
#define PXA255_GRER2			(PXA255_GPIO_BASE_ADDR + 0x00000038)
#define PXA255_GFER0			(PXA255_GPIO_BASE_ADDR + 0x0000003c)
#define PXA255_GFER1			(PXA255_GPIO_BASE_ADDR + 0x00000040)
#define PXA255_GFER2			(PXA255_GPIO_BASE_ADDR + 0x00000044)
#define PXA255_GEDR0			(PXA255_GPIO_BASE_ADDR + 0x00000048)
#define PXA255_GEDR1			(PXA255_GPIO_BASE_ADDR + 0x0000004c)
#define PXA255_GEDR2			(PXA255_GPIO_BASE_ADDR + 0x00000050)
#define PXA255_GAFR0_L			(PXA255_GPIO_BASE_ADDR + 0x00000054)
#define PXA255_GAFR0_U			(PXA255_GPIO_BASE_ADDR + 0x00000058)
#define PXA255_GAFR1_L			(PXA255_GPIO_BASE_ADDR + 0x0000005c)
#define PXA255_GAFR1_U			(PXA255_GPIO_BASE_ADDR + 0x00000060)
#define PXA255_GAFR2_L			(PXA255_GPIO_BASE_ADDR + 0x00000064)
#define PXA255_GAFR2_U			(PXA255_GPIO_BASE_ADDR + 0x00000068)

typedef struct
{
	UINT32 gplr0; // GPIO Pin-Leve
	UINT32 gplr1;
	UINT32 gplr2;

	UINT32 gpdr0;
	UINT32 gpdr1;
	UINT32 gpdr2;

	UINT32 gpsr0;
	UINT32 gpsr1;
	UINT32 gpsr2;

	UINT32 gpcr0;
	UINT32 gpcr1;
	UINT32 gpcr2;

	UINT32 grer0;
	UINT32 grer1;
	UINT32 grer2;

	UINT32 gfer0;
	UINT32 gfer1;
	UINT32 gfer2;

	UINT32 gedr0;
	UINT32 gedr1;
	UINT32 gedr2;

	UINT32 gafr0l;
	UINT32 gafr0u;
	UINT32 gafr1l;
	UINT32 gafr1u;
	UINT32 gafr2l;
	UINT32 gafr2u;
} PXA255_GPIO_Regs;

static PXA255_GPIO_Regs gpio_regs;

static READ32_HANDLER( pxa255_gpio_r )
{
	switch(PXA255_GPIO_BASE_ADDR | (offset << 2))
	{
		case PXA255_GPLR0:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Pin-Level Register 0: %08x & %08x\n", gpio_regs.gplr0 | (1 << 1), mem_mask );
			return gpio_regs.gplr0 | (1 << 1); // Must be on.  Probably a DIP switch.
		case PXA255_GPLR1:
			verboselog( space->machine, 3, "pxa255_gpio_r: *Not Yet Implemented* GPIO Pin-Level Register 1: %08x & %08x\n", gpio_regs.gplr1, mem_mask );
			return gpio_regs.gplr1;
		case PXA255_GPLR2:
			verboselog( space->machine, 3, "pxa255_gpio_r: *Not Yet Implemented* GPIO Pin-Level Register 2: %08x & %08x\n", gpio_regs.gplr2, mem_mask );
			return gpio_regs.gplr2;
		case PXA255_GPDR0:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Pin Direction Register 0: %08x & %08x\n", gpio_regs.gpdr0, mem_mask );
			return gpio_regs.gpdr0;
		case PXA255_GPDR1:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Pin Direction Register 1: %08x & %08x\n", gpio_regs.gpdr1, mem_mask );
			return gpio_regs.gpdr1;
		case PXA255_GPDR2:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Pin Direction Register 2: %08x & %08x\n", gpio_regs.gpdr2, mem_mask );
			return gpio_regs.gpdr2;
		case PXA255_GPSR0:
			verboselog( space->machine, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 0: %08x & %08x\n", mame_rand(space->machine), mem_mask );
			return mame_rand(space->machine);
		case PXA255_GPSR1:
			verboselog( space->machine, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 1: %08x & %08x\n", mame_rand(space->machine), mem_mask );
			return mame_rand(space->machine);
		case PXA255_GPSR2:
			verboselog( space->machine, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Set Register 2: %08x & %08x\n", mame_rand(space->machine), mem_mask );
			return mame_rand(space->machine);
		case PXA255_GPCR0:
			verboselog( space->machine, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 0: %08x & %08x\n", mame_rand(space->machine), mem_mask );
			return mame_rand(space->machine);
		case PXA255_GPCR1:
			verboselog( space->machine, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 1: %08x & %08x\n", mame_rand(space->machine), mem_mask );
			return mame_rand(space->machine);
		case PXA255_GPCR2:
			verboselog( space->machine, 3, "pxa255_gpio_r: (Invalid Read) GPIO Pin Output Clear Register 2: %08x & %08x\n", mame_rand(space->machine), mem_mask );
			return mame_rand(space->machine);
		case PXA255_GRER0:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 0: %08x & %08x\n", gpio_regs.grer0, mem_mask );
			return gpio_regs.grer0;
		case PXA255_GRER1:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 1: %08x & %08x\n", gpio_regs.grer1, mem_mask );
			return gpio_regs.grer1;
		case PXA255_GRER2:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Rising Edge Detect Enable Register 2: %08x & %08x\n", gpio_regs.grer2, mem_mask );
			return gpio_regs.grer2;
		case PXA255_GFER0:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 0: %08x & %08x\n", gpio_regs.gfer0, mem_mask );
			return gpio_regs.gfer0;
		case PXA255_GFER1:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 1: %08x & %08x\n", gpio_regs.gfer1, mem_mask );
			return gpio_regs.gfer1;
		case PXA255_GFER2:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Falling Edge Detect Enable Register 2: %08x & %08x\n", gpio_regs.gfer2, mem_mask );
			return gpio_regs.gfer2;
		case PXA255_GEDR0:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Edge Detect Status Register 0: %08x & %08x\n", gpio_regs.gedr0, mem_mask );
			return gpio_regs.gedr0;
		case PXA255_GEDR1:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Edge Detect Status Register 1: %08x & %08x\n", gpio_regs.gedr1, mem_mask );
			return gpio_regs.gedr1;
		case PXA255_GEDR2:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Edge Detect Status Register 2: %08x & %08x\n", gpio_regs.gedr2, mem_mask );
			return gpio_regs.gedr2;
		case PXA255_GAFR0_L:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Alternate Function Register 0 Lower: %08x & %08x\n", gpio_regs.gafr0l, mem_mask );
			return gpio_regs.gafr0l;
		case PXA255_GAFR0_U:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Alternate Function Register 0 Upper: %08x & %08x\n", gpio_regs.gafr0u, mem_mask );
			return gpio_regs.gafr0u;
		case PXA255_GAFR1_L:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Alternate Function Register 1 Lower: %08x & %08x\n", gpio_regs.gafr1l, mem_mask );
			return gpio_regs.gafr1l;
		case PXA255_GAFR1_U:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Alternate Function Register 1 Upper: %08x & %08x\n", gpio_regs.gafr1u, mem_mask );
			return gpio_regs.gafr1u;
		case PXA255_GAFR2_L:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Alternate Function Register 2 Lower: %08x & %08x\n", gpio_regs.gafr2l, mem_mask );
			return gpio_regs.gafr2l;
		case PXA255_GAFR2_U:
			verboselog( space->machine, 3, "pxa255_gpio_r: GPIO Alternate Function Register 2 Upper: %08x & %08x\n", gpio_regs.gafr2u, mem_mask );
			return gpio_regs.gafr2u;
		default:
			verboselog( space->machine, 0, "pxa255_gpio_r: Unknown address: %08x\n", PXA255_GPIO_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

static WRITE32_HANDLER( pxa255_gpio_w )
{
	switch(PXA255_GPIO_BASE_ADDR | (offset << 2))
	{
		case PXA255_GPLR0:
			verboselog( space->machine, 3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_GPLR1:
			verboselog( space->machine, 3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 1: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_GPLR2:
			verboselog( space->machine, 3, "pxa255_gpio_w: (Invalid Write) GPIO Pin-Level Register 2: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_GPDR0:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Direction Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpdr0 = data;
			break;
		case PXA255_GPDR1:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Direction Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpdr1 = data;
			break;
		case PXA255_GPDR2:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Direction Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpdr2 = data;
			break;
		case PXA255_GPSR0:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Output Set Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpsr0 |= data & gpio_regs.gpdr0;
			break;
		case PXA255_GPSR1:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Output Set Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpsr1 |= data & gpio_regs.gpdr1;
			break;
		case PXA255_GPSR2:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Output Set Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpsr2 |= data & gpio_regs.gpdr2;
			break;
		case PXA255_GPCR0:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Output Clear Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpsr0 &= ~(data & gpio_regs.gpdr0);
			break;
		case PXA255_GPCR1:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Output Clear Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpsr1 &= ~(data & gpio_regs.gpdr1);
			break;
		case PXA255_GPCR2:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Pin Output Clear Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs.gpsr2 &= ~(data & gpio_regs.gpdr2);
			break;
		case PXA255_GRER0:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs.grer0 = data;
			break;
		case PXA255_GRER1:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs.grer1 = data;
			break;
		case PXA255_GRER2:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Rising Edge Detect Enable Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs.grer2 = data;
			break;
		case PXA255_GFER0:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 0: %08x & %08x\n", data, mem_mask );
			gpio_regs.gfer0 = data;
			break;
		case PXA255_GFER1:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 1: %08x & %08x\n", data, mem_mask );
			gpio_regs.gfer1 = data;
			break;
		case PXA255_GFER2:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Falling Edge Detect Enable Register 2: %08x & %08x\n", data, mem_mask );
			gpio_regs.gfer2 = data;
			break;
		case PXA255_GEDR0:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Edge Detect Status Register 0: %08x & %08x\n", gpio_regs.gedr0, mem_mask );
			gpio_regs.gedr0 &= ~data;
			break;
		case PXA255_GEDR1:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Edge Detect Status Register 1: %08x & %08x\n", gpio_regs.gedr1, mem_mask );
			gpio_regs.gedr1 &= ~data;
			break;
		case PXA255_GEDR2:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Edge Detect Status Register 2: %08x & %08x\n", gpio_regs.gedr2, mem_mask );
			gpio_regs.gedr2 &= ~data;
			break;
		case PXA255_GAFR0_L:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Alternate Function Register 0 Lower: %08x & %08x\n", gpio_regs.gafr0l, mem_mask );
			gpio_regs.gafr0l = data;
			break;
		case PXA255_GAFR0_U:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Alternate Function Register 0 Upper: %08x & %08x\n", gpio_regs.gafr0u, mem_mask );
			gpio_regs.gafr0u = data;
			break;
		case PXA255_GAFR1_L:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Alternate Function Register 1 Lower: %08x & %08x\n", gpio_regs.gafr1l, mem_mask );
			gpio_regs.gafr1l = data;
			break;
		case PXA255_GAFR1_U:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Alternate Function Register 1 Upper: %08x & %08x\n", gpio_regs.gafr1u, mem_mask );
			gpio_regs.gafr1u = data;
			break;
		case PXA255_GAFR2_L:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Alternate Function Register 2 Lower: %08x & %08x\n", gpio_regs.gafr2l, mem_mask );
			gpio_regs.gafr2l = data;
			break;
		case PXA255_GAFR2_U:
			verboselog( space->machine, 3, "pxa255_gpio_w: GPIO Alternate Function Register 2 Upper: %08x & %08x\n", gpio_regs.gafr2u, mem_mask );
			gpio_regs.gafr2u = data;
			break;
		default:
			verboselog( space->machine, 0, "pxa255_gpio_w: Unknown address: %08x = %08x & %08x\n", PXA255_GPIO_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

/*

  PXA255 LCD Controller

  pg. 265 to 310, PXA255 Processor Developers Manual [278693-002].pdf

*/

#define PXA255_LCD_BASE_ADDR	(0x44000000)
#define PXA255_LCCR0			(PXA255_LCD_BASE_ADDR + 0x00000000)
	#define PXA255_LCCR0_OUM	(0x00200000)
	#define PXA255_LCCR0_BM		(0x00100000)
	#define PXA255_LCCR0_PDD	(0x000ff000)
	#define PXA255_LCCR0_QDM	(0x00000800)
	#define PXA255_LCCR0_DIS	(0x00000400)
	#define PXA255_LCCR0_DPD	(0x00000200)
	#define PXA255_LCCR0_PAS	(0x00000080)
	#define PXA255_LCCR0_EFM	(0x00000040)
	#define PXA255_LCCR0_IUM	(0x00000020)
	#define PXA255_LCCR0_SFM	(0x00000010)
	#define PXA255_LCCR0_LDM	(0x00000008)
	#define PXA255_LCCR0_SDS	(0x00000004)
	#define PXA255_LCCR0_CMS	(0x00000002)
	#define PXA255_LCCR0_ENB	(0x00000001)
#define PXA255_LCCR1			(PXA255_LCD_BASE_ADDR + 0x00000004)
#define PXA255_LCCR2			(PXA255_LCD_BASE_ADDR + 0x00000008)
#define PXA255_LCCR3			(PXA255_LCD_BASE_ADDR + 0x0000000c)
#define PXA255_FBR0				(PXA255_LCD_BASE_ADDR + 0x00000020)
#define PXA255_FBR1				(PXA255_LCD_BASE_ADDR + 0x00000024)
#define PXA255_LCSR				(PXA255_LCD_BASE_ADDR + 0x00000038)
	#define PXA255_LCSR_LDD		(0x00000001)
	#define PXA255_LCSR_SOF		(0x00000002)
	#define PXA255_LCSR_BER		(0x00000004)
	#define PXA255_LCSR_ABC		(0x00000008)
	#define PXA255_LCSR_IUL		(0x00000010)
	#define PXA255_LCSR_IUU		(0x00000020)
	#define PXA255_LCSR_OU		(0x00000040)
	#define PXA255_LCSR_QD		(0x00000080)
	#define PXA255_LCSR_EOF		(0x00000100)
	#define PXA255_LCSR_BS		(0x00000200)
	#define PXA255_LCSR_SINT	(0x00000400)
#define PXA255_LIIDR			(PXA255_LCD_BASE_ADDR + 0x0000003c)
#define PXA255_TRGBR			(PXA255_LCD_BASE_ADDR + 0x00000040)
#define PXA255_TCR				(PXA255_LCD_BASE_ADDR + 0x00000044)
#define PXA255_FDADR0			(PXA255_LCD_BASE_ADDR + 0x00000200)
#define PXA255_FSADR0			(PXA255_LCD_BASE_ADDR + 0x00000204)
#define PXA255_FIDR0			(PXA255_LCD_BASE_ADDR + 0x00000208)
#define PXA255_LDCMD0			(PXA255_LCD_BASE_ADDR + 0x0000020c)
	#define PXA255_LDCMD_SOFINT	(0x00400000)
	#define PXA255_LDCMD_EOFINT	(0x00200000)
#define PXA255_FDADR1			(PXA255_LCD_BASE_ADDR + 0x00000210)
#define PXA255_FSADR1			(PXA255_LCD_BASE_ADDR + 0x00000214)
#define PXA255_FIDR1			(PXA255_LCD_BASE_ADDR + 0x00000218)
#define PXA255_LDCMD1			(PXA255_LCD_BASE_ADDR + 0x0000021c)

typedef struct
{
	UINT32 fdadr;
	UINT32 fsadr;
	UINT32 fidr;
	UINT32 ldcmd;
	emu_timer *eof;
} PXA255_LCD_DMA_Regs;

typedef struct
{
	UINT32 lccr0;
	UINT32 lccr1;
	UINT32 lccr2;
	UINT32 lccr3;

	UINT32 pad0[4];

	UINT32 fbr0;
	UINT32 fbr1;

	UINT32 pad1[4];

	UINT32 lcsr;
	UINT32 liidr;
	UINT32 trgbr;
	UINT32 tcr;

	UINT32 pad2[110];

	PXA255_LCD_DMA_Regs dma[2];
} PXA255_LCD_Regs;

static PXA255_LCD_Regs lcd_regs;

static void pxa255_lcd_initiate_dma(const address_space* space, UINT32 address, int channel)
{
	lcd_regs.dma[channel].fdadr = memory_read_dword_32le(space, address);
	lcd_regs.dma[channel].fsadr = memory_read_dword_32le(space, address + 0x04);
	lcd_regs.dma[channel].fidr  = memory_read_dword_32le(space, address + 0x08);
	lcd_regs.dma[channel].ldcmd = memory_read_dword_32le(space, address + 0x0c);
	verboselog( space->machine, 3, "pxa255_lcd_initiate_dma, address = %08x, channel = %d\n", address, channel);
	verboselog( space->machine, 3, "    DMA Frame Descriptor: %08x\n", lcd_regs.dma[channel].fdadr );
	verboselog( space->machine, 3, "    DMA Frame Source Address: %08x\n", lcd_regs.dma[channel].fsadr );
	verboselog( space->machine, 3, "    DMA Frame ID: %08x\n", lcd_regs.dma[channel].fidr );
	verboselog( space->machine, 3, "    DMA Command: %08x\n", lcd_regs.dma[channel].ldcmd );
}

static void pxa255_lcd_irq_check(running_machine *machine)
{
	if(((lcd_regs.lcsr & PXA255_LCSR_BS)  != 0 && (lcd_regs.lccr0 & PXA255_LCCR0_BM)  == 0) ||
	   ((lcd_regs.lcsr & PXA255_LCSR_EOF) != 0 && (lcd_regs.lccr0 & PXA255_LCCR0_EFM) == 0) ||
	   ((lcd_regs.lcsr & PXA255_LCSR_SOF) != 0 && (lcd_regs.lccr0 & PXA255_LCCR0_SFM) == 0))
	{
		pxa255_set_irq_line(machine, PXA255_INT_LCD, 1);
	}
	else
	{
		pxa255_set_irq_line(machine, PXA255_INT_LCD, 0);
	}
}

static READ32_HANDLER( pxa255_lcd_r )
{
	switch(PXA255_LCD_BASE_ADDR | (offset << 2))
	{
		case PXA255_LCCR0:		// 0x44000000
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Control 0: %08x & %08x\n", lcd_regs.lccr0, mem_mask );
			return lcd_regs.lccr0;
		case PXA255_LCCR1:		// 0x44000004
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Control 1: %08x & %08x\n", lcd_regs.lccr1, mem_mask );
			return lcd_regs.lccr1;
		case PXA255_LCCR2:		// 0x44000008
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Control 2: %08x & %08x\n", lcd_regs.lccr2, mem_mask );
			return lcd_regs.lccr2;
		case PXA255_LCCR3:		// 0x4400000c
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Control 3: %08x & %08x\n", lcd_regs.lccr3, mem_mask );
			return lcd_regs.lccr3;
		case PXA255_FBR0:		// 0x44000020
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Frame Branch Register 0: %08x & %08x\n", lcd_regs.fbr0, mem_mask );
			return lcd_regs.fbr0;
		case PXA255_FBR1:		// 0x44000024
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Frame Branch Register 1: %08x & %08x\n", lcd_regs.fbr1, mem_mask );
			return lcd_regs.fbr1;
		case PXA255_LCSR:		// 0x44000038
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Status Register: %08x & %08x\n", lcd_regs.lcsr, mem_mask );
			return lcd_regs.lcsr;
		case PXA255_LIIDR:		// 0x4400003c
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD Interrupt ID Register: %08x & %08x\n", lcd_regs.liidr, mem_mask );
			return lcd_regs.liidr;
		case PXA255_TRGBR:		// 0x44000040
			verboselog( space->machine, 3, "pxa255_lcd_r: TMED RGB Seed Register: %08x & %08x\n", lcd_regs.trgbr, mem_mask );
			return lcd_regs.trgbr;
		case PXA255_TCR:		// 0x44000044
			verboselog( space->machine, 3, "pxa255_lcd_r: TMED RGB Seed Register: %08x & %08x\n", lcd_regs.tcr, mem_mask );
			return lcd_regs.tcr;
		case PXA255_FDADR0:		// 0x44000200
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Frame Descriptor Address Register 0: %08x & %08x\n", lcd_regs.dma[0].fdadr, mem_mask );
			return lcd_regs.dma[0].fdadr;
		case PXA255_FSADR0:		// 0x44000204
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Frame Source Address Register 0: %08x & %08x\n", lcd_regs.dma[0].fsadr, mem_mask );
			return lcd_regs.dma[0].fsadr;
		case PXA255_FIDR0:		// 0x44000208
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Frame ID Register 0: %08x & %08x\n", lcd_regs.dma[0].fidr, mem_mask );
			return lcd_regs.dma[0].fidr;
		case PXA255_LDCMD0:		// 0x4400020c
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Command Register 0: %08x & %08x\n", lcd_regs.dma[0].ldcmd & 0xfff00000, mem_mask );
			return lcd_regs.dma[0].ldcmd & 0xfff00000;
		case PXA255_FDADR1:		// 0x44000210
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Frame Descriptor Address Register 1: %08x & %08x\n", lcd_regs.dma[1].fdadr, mem_mask );
			return lcd_regs.dma[1].fdadr;
		case PXA255_FSADR1:		// 0x44000214
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Frame Source Address Register 1: %08x & %08x\n", lcd_regs.dma[1].fsadr, mem_mask );
			return lcd_regs.dma[1].fsadr;
		case PXA255_FIDR1:		// 0x44000218
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Frame ID Register 1: %08x & %08x\n", lcd_regs.dma[1].fidr, mem_mask );
			return lcd_regs.dma[1].fidr;
		case PXA255_LDCMD1:		// 0x4400021c
			verboselog( space->machine, 3, "pxa255_lcd_r: LCD DMA Command Register 1: %08x & %08x\n", lcd_regs.dma[1].ldcmd & 0xfff00000, mem_mask );
			return lcd_regs.dma[1].ldcmd & 0xfff00000;
		default:
			verboselog( space->machine, 0, "pxa255_lcd_r: Unknown address: %08x\n", PXA255_LCD_BASE_ADDR | (offset << 2));
			break;
	}
	return 0;
}

static WRITE32_HANDLER( pxa255_lcd_w )
{
	switch(PXA255_LCD_BASE_ADDR | (offset << 2))
	{
		case PXA255_LCCR0:		// 0x44000000
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Control 0: %08x & %08x\n", data, mem_mask );
			lcd_regs.lccr0 = data & 0x00fffeff;
			break;
		case PXA255_LCCR1:		// 0x44000004
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Control 1: %08x & %08x\n", data, mem_mask );
			lcd_regs.lccr1 = data;
			break;
		case PXA255_LCCR2:		// 0x44000008
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Control 2: %08x & %08x\n", data, mem_mask );
			lcd_regs.lccr2 = data;
			break;
		case PXA255_LCCR3:		// 0x4400000c
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Control 3: %08x & %08x\n", data, mem_mask );
			lcd_regs.lccr3 = data;
			break;
		case PXA255_FBR0:		// 0x44000020
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Frame Branch Register 0: %08x & %08x\n", data, mem_mask );
			lcd_regs.fbr0 = data & 2;
			if(data & 1)
			{
				pxa255_lcd_initiate_dma(space, data & 0xfffffff0, 0);
				lcd_regs.fbr0 |= memory_read_dword_32le(space, data & 0xfffffff0) & 0xfffffff0;
				if(!(lcd_regs.lccr0 & PXA255_LCCR0_BM))
				{
					lcd_regs.lcsr |= PXA255_LCSR_BS;
					pxa255_lcd_irq_check(space->machine);
				}
			}
			break;
		case PXA255_FBR1:		// 0x44000024
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Frame Branch Register 1: %08x & %08x\n", data, mem_mask );
			lcd_regs.fbr1 = data & 2;
			if(data & 1)
			{
				pxa255_lcd_initiate_dma(space, data & 0xfffffff0, 1);
				lcd_regs.fbr1 |= memory_read_dword_32le(space, data & 0xfffffff0) & 0xfffffff0;
				if(!(lcd_regs.lccr0 & PXA255_LCCR0_BM))
				{
					lcd_regs.lcsr |= PXA255_LCSR_BS;
					pxa255_lcd_irq_check(space->machine);
				}
			}
			break;
		case PXA255_LCSR:		// 0x44000038
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Controller Status Register: %08x & %08x\n", data, mem_mask );
			lcd_regs.lcsr &= ~data;
			pxa255_lcd_irq_check(space->machine);
			break;
		case PXA255_LIIDR:		// 0x4400003c
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD Controller Interrupt ID Register: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_TRGBR:		// 0x44000040
			verboselog( space->machine, 3, "pxa255_lcd_w: TMED RGB Seed Register: %08x & %08x\n", data, mem_mask );
			lcd_regs.trgbr = data & 0x00ffffff;
			break;
		case PXA255_TCR:		// 0x44000044
			verboselog( space->machine, 3, "pxa255_lcd_w: TMED Control Register: %08x & %08x\n", data, mem_mask );
			lcd_regs.tcr = data & 0x00004fff;
			break;
		case PXA255_FDADR0:		// 0x44000200
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD DMA Frame Descriptor Address Register 0: %08x & %08x\n", data, mem_mask );
			pxa255_lcd_initiate_dma(space, data & 0xfffffff0, 0);
			break;
		case PXA255_FSADR0:		// 0x44000204
			verboselog( space->machine, 3, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame Source Address Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_FIDR0:		// 0x44000208
			verboselog( space->machine, 3, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame ID Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_LDCMD0:		// 0x4400020c
			verboselog( space->machine, 3, "pxa255_lcd_w: (Invalid Write) LCD DMA Command Register 0: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_FDADR1:		// 0x44000210
			verboselog( space->machine, 3, "pxa255_lcd_w: LCD DMA Frame Descriptor Address Register 1: %08x & %08x\n", data, mem_mask );
			pxa255_lcd_initiate_dma(space, data & 0xfffffff0, 1);
			break;
		case PXA255_FSADR1:		// 0x44000214
			verboselog( space->machine, 3, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame Source Address Register 1: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_FIDR1:		// 0x44000218
			verboselog( space->machine, 3, "pxa255_lcd_w: (Invalid Write) LCD DMA Frame ID Register 1: %08x & %08x\n", data, mem_mask );
			break;
		case PXA255_LDCMD1:		// 0x4400021c
			verboselog( space->machine, 3, "pxa255_lcd_w: (Invalid Write) LCD DMA Command Register 1: %08x & %08x\n", data, mem_mask );
			break;
		default:
			verboselog( space->machine, 0, "pxa255_lcd_w: Unknown address: %08x = %08x & %08x\n", PXA255_LCD_BASE_ADDR | (offset << 2), data, mem_mask);
			break;
	}
}

static TIMER_CALLBACK( pxa255_lcd_dma_eof )
{
	printf( "LCD EOF: %d\n", param );
	lcd_regs.liidr = lcd_regs.dma[param].fidr;
	lcd_regs.lcsr |= PXA255_LCSR_EOF;
	pxa255_lcd_irq_check(machine);
}

static INTERRUPT_GEN( pxa255_vblank_start )
{
	if(lcd_regs.lccr0 & PXA255_LCCR0_ENB)
	{
		// Mark the start of the current DMA frame as appropriate
		int channel = 0;
		for(channel = 0; channel < 2; channel++)
		{
			if(lcd_regs.dma[channel].fdadr != 0)
			{
				if(lcd_regs.dma[channel].ldcmd & PXA255_LDCMD_SOFINT)
				{
					printf( "LCD SOF: %d\n", channel );
					lcd_regs.liidr = lcd_regs.dma[channel].fidr;
					lcd_regs.lcsr |= PXA255_LCSR_SOF;
					pxa255_lcd_irq_check(device->machine);
				}
				if(lcd_regs.dma[channel].ldcmd & PXA255_LDCMD_EOFINT)
				{
					attotime period = attotime_mul(ATTOTIME_IN_HZ(200000000), lcd_regs.dma[channel].ldcmd & 0x000fffff);

					timer_adjust_oneshot(lcd_regs.dma[channel].eof, period, channel);
				}
			}
		}
	}
}

static ADDRESS_MAP_START( 39in1_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM
	AM_RANGE(0x40000000, 0x400002ff) AM_READ( pxa255_dma_r )
	AM_RANGE(0x40a00000, 0x40a0001f) AM_READWRITE( pxa255_ostimer_r, pxa255_ostimer_w )
	AM_RANGE(0x40d00000, 0x40d00017) AM_READWRITE( pxa255_intc_r, pxa255_intc_w )
	AM_RANGE(0x40e00000, 0x40e0006b) AM_READWRITE( pxa255_gpio_r, pxa255_gpio_w )
	AM_RANGE(0x44000000, 0x4400021f) AM_READWRITE( pxa255_lcd_r,  pxa255_lcd_w )
	AM_RANGE(0xa0000000, 0xa3ffffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( 39in1 )
INPUT_PORTS_END

static VIDEO_UPDATE( 39in1 )
{
	return 0;
}

/* To be moved to DEVICE_START( pxa255 ) upon completion */
static void pxa255_start(running_machine *machine)
{
    //pxa255_t* pxa255 = pxa255_get_safe_token( device );

    //pxa255->iface = device->static_config;

	ostimer_regs.osmr0 = ostimer_regs.osmr1 = ostimer_regs.osmr2 = ostimer_regs.osmr3 = 0;
	ostimer_regs.oscr = ostimer_regs.ossr = ostimer_regs.ower = ostimer_regs.oier = 0;

	intc_regs.icmr = intc_regs.iclr = intc_regs.iccr = intc_regs.icip = intc_regs.icfp = intc_regs.icpr = 0;

	lcd_regs.lccr0 = lcd_regs.lccr1 = lcd_regs.lccr2 = lcd_regs.lccr3 = 0;
	lcd_regs.dma[0].fdadr = lcd_regs.dma[1].fdadr = 0;
	lcd_regs.dma[0].fsadr = lcd_regs.dma[1].fsadr = 0;
	lcd_regs.dma[0].fidr  = lcd_regs.dma[1].fidr  = 0;
	lcd_regs.dma[0].ldcmd = lcd_regs.dma[1].ldcmd = 0;
	lcd_regs.dma[0].eof = timer_alloc(machine, pxa255_lcd_dma_eof, 0);
	lcd_regs.dma[1].eof = timer_alloc(machine, pxa255_lcd_dma_eof, 0);
	lcd_regs.fbr0 = lcd_regs.fbr1 = lcd_regs.lcsr = lcd_regs.liidr = 0;
	lcd_regs.trgbr = 0x00aa5500;
	lcd_regs.tcr = 0x0000754f;

    //pxa255_register_state_save(device);
}

static MACHINE_START(39in1)
{
	UINT8 *ROM = memory_region(machine, "maincpu");
	int i;

	for (i = 0; i < 0x80000; i += 2)
	{
		ROM[i] = BITSWAP8(ROM[i],7,2,5,6,0,3,1,4) ^ BITSWAP8((i>>3)&0xf, 3,2,4,1,4,4,0,4) ^ 0x90;
	}

	pxa255_start(machine);
}

static MACHINE_DRIVER_START( 39in1 )
	MDRV_CPU_ADD("maincpu", PXA255, 200000000)
	MDRV_CPU_PROGRAM_MAP(39in1_map)
	MDRV_CPU_VBLANK_INT("screen", pxa255_vblank_start)

	MDRV_PALETTE_LENGTH(32768)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(16777216/4, 308, 0,  240, 228, 0,  160)	// completely bogus for this h/w

	MDRV_MACHINE_START(39in1)

	MDRV_VIDEO_UPDATE(39in1)

	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_DRIVER_END

ROM_START( 39in1 )
	// main program, encrypted
	ROM_REGION( 0x80000, "maincpu", 0 )
        ROM_LOAD( "27c4096_plz-v001_ver.300.bin", 0x000000, 0x080000, CRC(9149dbc4) SHA1(40efe1f654f11474f75ae7fee1613f435dbede38) )

	// data ROM - contains a filesystem with ROMs, fonts, graphics, etc. in an unknown compressed format
	ROM_REGION32_LE( 0x200000, "data", 0 )
        ROM_LOAD( "16mflash.bin", 0x000000, 0x200000, CRC(a089f0f8) SHA1(e975eadd9176a8b9e416229589dfe3158cba22cb) )
ROM_END

GAME(2004, 39in1, 0, 39in1, 39in1, 0, ROT0, "????", "39 in 1 MAME bootleg", GAME_NOT_WORKING|GAME_NO_SOUND)
