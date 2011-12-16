/*

    dc.c - Sega Dreamcast hardware

*/

#include "emu.h"
#include "debugger.h"
#include "includes/dc.h"
#include "cpu/sh4/sh4.h"
#include "sound/aica.h"
#include "includes/naomi.h"
#include "machine/mie.h"

#define DEBUG_REGISTERS	(1)

#if DEBUG_REGISTERS

#define DEBUG_SYSCTRL	(0)
#define DEBUG_AICA_DMA (0)
#define DEBUG_PVRCTRL	(0)

#if DEBUG_SYSCTRL
static const char *const sysctrl_names[] =
{
	"CH2 DMA dest",
	"CH2 DMA length",
	"CH2 DMA start",
	"5f680c",
	"Sort DMA start link table addr",
	"Sort DMA link base addr",
	"Sort DMA link address bit width",
	"Sort DMA link address shift control",
	"Sort DMA start",
	"5f6824", "5f6828", "5f682c", "5f6830",
	"5f6834", "5f6838", "5f683c",
	"DBREQ # signal mask control",
	"BAVL # signal wait count",
	"DMA priority count",
	"CH2 DMA max burst length",
	"5f6850", "5f6854", "5f6858", "5f685c",
	"5f6860", "5f6864", "5f6868", "5f686c",
	"5f6870", "5f6874", "5f6878", "5f687c",
	"TA FIFO remaining",
	"TA texture memory bus select 0",
	"TA texture memory bus select 1",
	"FIFO status",
	"System reset", "5f6894", "5f6898",
	"System bus version",
	"SH4 root bus split enable",
	"5f68a4", "5f68a8", "5f68ac",
	"5f68b0", "5f68b4", "5f68b8", "5f68bc",
	"5f68c0", "5f68c4", "5f68c8", "5f68cc",
	"5f68d0", "5f68d4", "5f68d8", "5f68dc",
	"5f68e0", "5f68e4", "5f68e8", "5f68ec",
	"5f68f0", "5f68f4", "5f68f8", "5f68fc",
	"Normal IRQ status",
	"External IRQ status",
	"Error IRQ status", "5f690c",
	"Level 2 normal IRQ mask",
	"Level 2 external IRQ mask",
	"Level 2 error IRQ mask", "5f691c",
	"Level 4 normal IRQ mask",
	"Level 4 external IRQ mask",
	"Level 4 error IRQ mask", "5f692c",
	"Level 6 normal IRQ mask",
	"Level 6 external IRQ mask",
	"Level 6 error IRQ mask", "5f693c",
	"Normal IRQ PVR-DMA startup mask",
	"External IRQ PVR-DMA startup mask",
	"5f6948", "5f694c",
	"Normal IRQ G2-DMA startup mask",
	"External IRQ G2-DMA startup mask"
};
#endif

#endif

static TIMER_CALLBACK( aica_dma_irq )
{
	dc_state *state = machine.driver_data<dc_state>();

	state->m_wave_dma.start = state->g2bus_regs[SB_ADST] = 0;
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_AICA;
	dc_update_interrupt_status(machine);
}

static TIMER_CALLBACK( pvr_dma_irq )
{
	dc_state *state = machine.driver_data<dc_state>();

	state->m_pvr_dma.start = state->pvrctrl_regs[SB_PDST] = 0;
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_PVR;
	dc_update_interrupt_status(machine);
}

void naomi_g1_irq(running_machine &machine)
{
	dc_state *state = machine.driver_data<dc_state>();

	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_GDROM;
	dc_update_interrupt_status(machine);
}

static TIMER_CALLBACK( ch2_dma_irq )
{
	dc_state *state = machine.driver_data<dc_state>();

	state->dc_sysctrl_regs[SB_C2DLEN]=0;
	state->dc_sysctrl_regs[SB_C2DST]=0;
	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_CH2;
	dc_update_interrupt_status(machine);
}

static TIMER_CALLBACK( yuv_fifo_irq )
{
	dc_state *state = machine.driver_data<dc_state>();

	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_YUV;
	dc_update_interrupt_status(machine);
}

static void wave_dma_execute(address_space *space)
{
	dc_state *state = space->machine().driver_data<dc_state>();

	UINT32 src,dst,size;
	dst = state->m_wave_dma.aica_addr;
	src = state->m_wave_dma.root_addr;
	size = 0;

	/* 0 rounding size = 32 Mbytes */
	if(state->m_wave_dma.size == 0) { state->m_wave_dma.size = 0x200000; }

	if(state->m_wave_dma.dir == 0)
	{
		for(;size<state->m_wave_dma.size;size+=4)
		{
			space->write_dword(dst,space->read_dword(src));
			src+=4;
			dst+=4;
		}
	}
	else
	{
		for(;size<state->m_wave_dma.size;size+=4)
		{
			space->write_dword(src,space->read_dword(dst));
			src+=4;
			dst+=4;
		}
	}

	/* update the params*/
	state->m_wave_dma.aica_addr = state->g2bus_regs[SB_ADSTAG] = dst;
	state->m_wave_dma.root_addr = state->g2bus_regs[SB_ADSTAR] = src;
	state->m_wave_dma.size = state->g2bus_regs[SB_ADLEN] = 0;
	state->m_wave_dma.flag = (state->m_wave_dma.indirect & 1) ? 1 : 0;
	/* Note: if you trigger an instant DMA IRQ trigger, sfz3upper doesn't play any bgm. */
	/* TODO: timing of this */
	space->machine().scheduler().timer_set(attotime::from_usec(300), FUNC(aica_dma_irq));
}

static void pvr_dma_execute(address_space *space)
{
	dc_state *state = space->machine().driver_data<dc_state>();

	UINT32 src,dst,size;
	dst = state->m_pvr_dma.pvr_addr;
	src = state->m_pvr_dma.sys_addr;
	size = 0;

	/* used so far by usagui and sprtjam*/
	//printf("PVR-DMA start\n");
	//printf("%08x %08x %08x\n",state->m_pvr_dma.pvr_addr,state->m_pvr_dma.sys_addr,state->m_pvr_dma.size);
	//printf("src %s dst %08x\n",state->m_pvr_dma.dir ? "->" : "<-",state->m_pvr_dma.sel);

	/* 0 rounding size = 16 Mbytes */
	if(state->m_pvr_dma.size == 0) { state->m_pvr_dma.size = 0x100000; }

	if(state->m_pvr_dma.dir == 0)
	{
		for(;size<state->m_pvr_dma.size;size+=4)
		{
			space->write_dword(dst,space->read_dword(src));
			src+=4;
			dst+=4;
		}
	}
	else
	{
		for(;size<state->m_pvr_dma.size;size+=4)
		{
			space->write_dword(src,space->read_dword(dst));
			src+=4;
			dst+=4;
		}
	}
	/* Note: do not update the params, since this DMA type doesn't support it. */
	/* TODO: timing of this */
	space->machine().scheduler().timer_set(attotime::from_usec(250), FUNC(pvr_dma_irq));
}

// register decode helpers

// this accepts only 32-bit accesses
INLINE int decode_reg32_64(running_machine &machine, UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		mame_printf_verbose("%s:Wrong mask!\n", machine.describe_context());
//      debugger_break(machine);
	}

	if (mem_mask == U64(0xffffffff00000000))
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

// this accepts only 32 and 16 bit accesses
INLINE int decode_reg3216_64(running_machine &machine, UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 16&32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0x0000ffff00000000)) && (mem_mask != U64(0x000000000000ffff)) &&
	    (mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		mame_printf_verbose("%s:Wrong mask!\n", machine.describe_context());
//      debugger_break(machine);
	}

	if (mem_mask & U64(0x0000ffff00000000))
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

int dc_compute_interrupt_level(running_machine &machine)
{
	dc_state *state = machine.driver_data<dc_state>();
	UINT32 ln,lx,le;

	ln=state->dc_sysctrl_regs[SB_ISTNRM] & state->dc_sysctrl_regs[SB_IML6NRM];
	lx=state->dc_sysctrl_regs[SB_ISTEXT] & state->dc_sysctrl_regs[SB_IML6EXT];
	le=state->dc_sysctrl_regs[SB_ISTERR] & state->dc_sysctrl_regs[SB_IML6ERR];
	if (ln | lx | le)
	{
		return 6;
	}

	ln=state->dc_sysctrl_regs[SB_ISTNRM] & state->dc_sysctrl_regs[SB_IML4NRM];
	lx=state->dc_sysctrl_regs[SB_ISTEXT] & state->dc_sysctrl_regs[SB_IML4EXT];
	le=state->dc_sysctrl_regs[SB_ISTERR] & state->dc_sysctrl_regs[SB_IML4ERR];
	if (ln | lx | le)
	{
		return 4;
	}

	ln=state->dc_sysctrl_regs[SB_ISTNRM] & state->dc_sysctrl_regs[SB_IML2NRM];
	lx=state->dc_sysctrl_regs[SB_ISTEXT] & state->dc_sysctrl_regs[SB_IML2EXT];
	le=state->dc_sysctrl_regs[SB_ISTERR] & state->dc_sysctrl_regs[SB_IML2ERR];
	if (ln | lx | le)
	{
		return 2;
	}

	return 0;
}

void dc_update_interrupt_status(running_machine &machine)
{
	dc_state *state = machine.driver_data<dc_state>();
	int level;

	if (state->dc_sysctrl_regs[SB_ISTERR])
	{
		state->dc_sysctrl_regs[SB_ISTNRM] |= IST_ERROR;
	}
	else
	{
		state->dc_sysctrl_regs[SB_ISTNRM] &= ~IST_ERROR;
	}

	if (state->dc_sysctrl_regs[SB_ISTEXT])
	{
		state->dc_sysctrl_regs[SB_ISTNRM] |= IST_G1G2EXTSTAT;
	}
	else
	{
		state->dc_sysctrl_regs[SB_ISTNRM] &= ~IST_G1G2EXTSTAT;
	}

	level=dc_compute_interrupt_level(machine);
	sh4_set_irln_input(machine.device("maincpu"), 15-level);

	/* Wave DMA HW trigger */
	if(state->m_wave_dma.flag && ((state->m_wave_dma.sel & 2) == 2))
	{
		if((state->dc_sysctrl_regs[SB_G2DTNRM] & state->dc_sysctrl_regs[SB_ISTNRM]) || (state->dc_sysctrl_regs[SB_G2DTEXT] & state->dc_sysctrl_regs[SB_ISTEXT]))
		{
			address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

			printf("Wave DMA HW trigger\n");
			wave_dma_execute(space);
		}
	}

	/* PVR-DMA HW trigger */
	if(state->m_pvr_dma.flag && ((state->m_pvr_dma.sel & 1) == 1))
	{
		if((state->dc_sysctrl_regs[SB_PDTNRM] & state->dc_sysctrl_regs[SB_ISTNRM]) || (state->dc_sysctrl_regs[SB_PDTEXT] & state->dc_sysctrl_regs[SB_ISTEXT]))
		{
			address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

			printf("PVR-DMA HW trigger\n");
			pvr_dma_execute(space);
		}
	}
}

READ64_HANDLER( dc_sysctrl_r )
{
	dc_state *state = space->machine().driver_data<dc_state>();

	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine(), offset, mem_mask, &shift);

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x41) && (reg != 0x42) && (reg != 0x23) && (reg > 2))	// filter out IRQ status reads
	{
		mame_printf_verbose("SYSCTRL: [%08x] read %x @ %x (reg %x: %s), mask %" I64FMT "x (PC=%x)\n", 0x5f6800+reg*4, state->dc_sysctrl_regs[reg], offset, reg, sysctrl_names[reg], mem_mask, cpu_get_pc(&space->device()));
	}
	#endif

	return (UINT64)state->dc_sysctrl_regs[reg] << shift;
}

WRITE64_HANDLER( dc_sysctrl_w )
{
	dc_state *state = space->machine().driver_data<dc_state>();

	int reg;
	UINT64 shift;
	UINT32 old,dat;
	UINT32 address;
	struct sh4_ddt_dma ddtdata;

	reg = decode_reg32_64(space->machine(), offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = state->dc_sysctrl_regs[reg];
	state->dc_sysctrl_regs[reg] = dat; // 5f6800+off*4=dat
	switch (reg)
	{
		case SB_C2DST:
			if(((old & 1) == 0) && (dat & 1)) // 0 -> 1
			{
				address=(state->dc_sysctrl_regs[SB_C2DSTAT] & 0x03ffffe0) | 0x10000000;
				if(state->dc_sysctrl_regs[SB_C2DSTAT] & 0x1f)
					printf("C2DSTAT just used to reserved bits %02x\n",state->dc_sysctrl_regs[SB_C2DSTAT] & 0x1f);

				ddtdata.destination=address;
				/* 0 rounding size = 16 Mbytes */
				if(state->dc_sysctrl_regs[SB_C2DLEN] == 0)
					ddtdata.length = 0x1000000;
				else
					ddtdata.length = state->dc_sysctrl_regs[SB_C2DLEN];
				ddtdata.size=1;
				ddtdata.direction=0;
				ddtdata.channel=2;
				ddtdata.mode=25; //011001
				sh4_dma_ddt(space->machine().device("maincpu"),&ddtdata);
				#if DEBUG_SYSCTRL
				if ((address >= 0x11000000) && (address <= 0x11FFFFFF))
					if (state->dc_sysctrl_regs[SB_LMMODE0])
						printf("SYSCTRL: Ch2 direct display lists dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", state->dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, state->dc_sysctrl_regs[SB_C2DSTAT],state->dc_sysctrl_regs[SB_LMMODE0],state->dc_sysctrl_regs[SB_LMMODE1]); // 1
					else
						mame_printf_verbose("SYSCTRL: Ch2 direct textures dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", state->dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, state->dc_sysctrl_regs[SB_C2DSTAT],state->dc_sysctrl_regs[SB_LMMODE0],state->dc_sysctrl_regs[SB_LMMODE1]); // 0
				else if ((address >= 0x13000000) && (address <= 0x13FFFFFF))
					if (state->dc_sysctrl_regs[SB_LMMODE1])
						printf("SYSCTRL: Ch2 direct display lists dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", state->dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, state->dc_sysctrl_regs[SB_C2DSTAT],state->dc_sysctrl_regs[SB_LMMODE0],state->dc_sysctrl_regs[SB_LMMODE1]); // 1
					else
						mame_printf_verbose("SYSCTRL: Ch2 direct textures dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", state->dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, state->dc_sysctrl_regs[SB_C2DSTAT],state->dc_sysctrl_regs[SB_LMMODE0],state->dc_sysctrl_regs[SB_LMMODE1]); // 0
				else if ((address >= 0x10800000) && (address <= 0x10ffffff))
					printf("SYSCTRL: Ch2 YUV dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", state->dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, state->dc_sysctrl_regs[SB_C2DSTAT],state->dc_sysctrl_regs[SB_LMMODE0],state->dc_sysctrl_regs[SB_LMMODE1]);
				else if ((address >= 0x10000000) && (address <= 0x107fffff))
					mame_printf_verbose("SYSCTRL: Ch2 TA Display List dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", state->dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, state->dc_sysctrl_regs[SB_C2DSTAT],state->dc_sysctrl_regs[SB_LMMODE0],state->dc_sysctrl_regs[SB_LMMODE1]);
				else
					mame_printf_verbose("SYSCTRL: Ch2 unknown dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", state->dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, state->dc_sysctrl_regs[SB_C2DSTAT],state->dc_sysctrl_regs[SB_LMMODE0],state->dc_sysctrl_regs[SB_LMMODE1]);
				#endif

				if ((!(address & 0x01000000)))
					state->dc_sysctrl_regs[SB_C2DSTAT]=address;
				else //direct texture path
					state->dc_sysctrl_regs[SB_C2DSTAT]=address+ddtdata.length;

				/* 200 usecs breaks sfz3upper */
				space->machine().scheduler().timer_set(attotime::from_usec(50), FUNC(ch2_dma_irq));
				/* simulate YUV FIFO processing here */
				if((address & 0x1800000) == 0x0800000)
					space->machine().scheduler().timer_set(attotime::from_usec(500), FUNC(yuv_fifo_irq));
			}
			break;

		case SB_ISTNRM:
			state->dc_sysctrl_regs[SB_ISTNRM] = old & ~(dat | 0xC0000000); // bits 31,30 ro
			dc_update_interrupt_status(space->machine());
			break;

		case SB_ISTEXT:
			state->dc_sysctrl_regs[SB_ISTEXT] = old;
			dc_update_interrupt_status(space->machine());
			break;

		case SB_ISTERR:
			state->dc_sysctrl_regs[SB_ISTERR] = old & ~dat;
			dc_update_interrupt_status(space->machine());
			break;
		case SB_SDST:
			if(dat & 1)
			{
				// TODO: Sort-DMA routine goes here
				printf("Sort-DMA irq\n");

				state->dc_sysctrl_regs[SB_SDST] = 0;
				state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_SORT;
				dc_update_interrupt_status(space->machine());
			}
			break;
	}

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x42) && (reg > 2))	// filter out IRQ acks and ch2 dma
	{
		mame_printf_verbose("SYSCTRL: write %" I64FMT "x to %x (reg %x), mask %" I64FMT "x\n", data>>shift, offset, reg, /*sysctrl_names[reg],*/ mem_mask);
	}
	#endif
}

READ64_HANDLER( dc_gdrom_r )
{
//	dc_state *state = space->machine().driver_data<dc_state>();

	UINT32 off;

	if ((int)~mem_mask & 1)
	{
		off=(offset << 1) | 1;
	}
	else
	{
		off=offset << 1;
	}

	if (off*4 == 0x4c)
		return -1;
	if (off*4 == 8)
		return 0;

	return 0;
}

WRITE64_HANDLER( dc_gdrom_w )
{
//	dc_state *state = space->machine().driver_data<dc_state>();
	UINT32 dat,off;

	if ((int)~mem_mask & 1)
	{
		dat=(UINT32)(data >> 32);
		off=(offset << 1) | 1;
	}
	else
	{
		dat=(UINT32)data;
		off=offset << 1;
	}

	mame_printf_verbose("GDROM: [%08x=%x]write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x5f7000+off*4, dat, data, offset, mem_mask);
}

READ64_HANDLER( dc_g2_ctrl_r )
{
	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine(), offset, mem_mask, &shift);
	mame_printf_verbose("G2CTRL:  Unmapped read %08x\n", 0x5f7800+reg*4);
	return (UINT64)state->g2bus_regs[reg] << shift;
}

WRITE64_HANDLER( dc_g2_ctrl_w )
{
	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;
	UINT32 dat;
	UINT8 old;

	reg = decode_reg32_64(space->machine(), offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	state->g2bus_regs[reg] = dat; // 5f7800+reg*4=dat

	switch (reg)
	{
		/*AICA Address register*/
		case SB_ADSTAG: state->m_wave_dma.aica_addr = dat; break;
		/*Root address (work ram)*/
		case SB_ADSTAR:	state->m_wave_dma.root_addr = dat; break;
		/*DMA size (in dword units, bit 31 is "set dma initiation enable setting to 0"*/
		case SB_ADLEN:
			state->m_wave_dma.size = dat & 0x7fffffff;
			state->m_wave_dma.indirect = (dat & 0x80000000)>>31;
			break;
		/*0 = root memory to aica / 1 = aica to root memory*/
		case SB_ADDIR: state->m_wave_dma.dir = (dat & 1); break;
		/*dma flag (active HIGH, bug in docs)*/
		case SB_ADEN: state->m_wave_dma.flag = (dat & 1); break;
		/*
        SB_ADTSEL
        bit 1: (0) Wave DMA through SB_ADST flag (1) Wave DMA through irq trigger, defined by SB_G2DTNRM / SB_G2DTEXT
        */
		case SB_ADTSEL: state->m_wave_dma.sel = dat & 7; break;
		/*ready for dma'ing*/
		case SB_ADST:
			old = state->m_wave_dma.start & 1;
			state->m_wave_dma.start = dat & 1;

			#if DEBUG_AICA_DMA
			printf("AICA: G2-DMA start \n");
			printf("DST %08x SRC %08x SIZE %08x IND %02x\n",state->m_wave_dma.aica_addr,state->m_wave_dma.root_addr,state->m_wave_dma.size,state->m_wave_dma.indirect);
			printf("SEL %08x ST  %08x FLAG %08x DIR %02x\n",state->m_wave_dma.sel,state->m_wave_dma.start,state->m_wave_dma.flag,state->m_wave_dma.dir);
			#endif

			//mame_printf_verbose("SB_ADST data %08x\n",dat);
			if(((old & 1) == 0) && state->m_wave_dma.flag && state->m_wave_dma.start && ((state->m_wave_dma.sel & 2) == 0)) // 0 -> 1
				wave_dma_execute(space);
			break;

		case SB_E1ST:
		case SB_E2ST:
		case SB_DDST:
			if(dat & 1)
				printf("Warning: enabled G2 Debug / External DMA %08x\n",reg);
			break;

		case SB_ADSUSP:
		case SB_E1SUSP:
		case SB_E2SUSP:
		case SB_DDSUSP:
		case SB_G2APRO:
			break;

		default:
			/* might access the unhandled DMAs, so tell us if this happens. */
			//printf("Unhandled G2 register [%08x] -> %08x\n",reg,dat);
			break;
	}
}

INLINE int decode_reg_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		/*assume to return the lower 32-bits ONLY*/
		return reg & 0xffffffff;
	}

	if (mem_mask == U64(0xffffffff00000000))
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

READ64_HANDLER( pvr_ctrl_r )
{
	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;

	reg = decode_reg_64(offset, mem_mask, &shift);

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: [%08x] read %x @ %x (reg %x), mask %" I64FMT "x (PC=%x)\n", 0x5f7c00+reg*4, state->pvrctrl_regs[reg], offset, reg, mem_mask, cpu_get_pc(&space->device()));
	#endif

	return (UINT64)state->pvrctrl_regs[reg] << shift;
}

WRITE64_HANDLER( pvr_ctrl_w )
{
	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;
	UINT32 dat;
	UINT8 old;

	reg = decode_reg_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	switch (reg)
	{
		case SB_PDSTAP: state->m_pvr_dma.pvr_addr = dat; break;
		case SB_PDSTAR: state->m_pvr_dma.sys_addr = dat; break;
		case SB_PDLEN: state->m_pvr_dma.size = dat; break;
		case SB_PDDIR: state->m_pvr_dma.dir = dat & 1; break;
		case SB_PDTSEL:
			state->m_pvr_dma.sel = dat & 1;
			//if(state->m_pvr_dma.sel & 1)
			//  printf("Warning: Unsupported irq mode trigger PVR-DMA\n");
			break;
		case SB_PDEN: state->m_pvr_dma.flag = dat & 1; break;
		case SB_PDST:
			old = state->m_pvr_dma.start & 1;
			state->m_pvr_dma.start = dat & 1;

			if(((old & 1) == 0) && state->m_pvr_dma.flag && state->m_pvr_dma.start && ((state->m_pvr_dma.sel & 1) == 0)) // 0 -> 1
				pvr_dma_execute(space);
			break;
	}

	#if DEBUG_PVRCTRL
	mame_printf_verbose("PVRCTRL: [%08x=%x] write %" I64FMT "x to %x (reg %x), mask %" I64FMT "x\n", 0x5f7c00+reg*4, dat, data>>shift, offset, reg, mem_mask);
	#endif

//  state->pvrctrl_regs[reg] |= dat;
	state->pvrctrl_regs[reg] = dat;

}

READ64_HANDLER( dc_modem_r )
{
//	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(space->machine(), offset, mem_mask, &shift);

	// from ElSemi: this makes Atomiswave do it's "verbose boot" with a Sammy logo and diagnostics instead of just running the cart.
	// our PVR emulation is apparently not good enough for that to work yet though.
	if (reg == 0x280/4)
	{
		return U64(0xffffffffffffffff);
	}

	mame_printf_verbose("MODEM:  Unmapped read %08x\n", 0x600000+reg*4);
	return 0;
}

WRITE64_HANDLER( dc_modem_w )
{
//	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg32_64(space->machine(), offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	mame_printf_verbose("MODEM: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x600000+reg*4, dat, data, offset, mem_mask);
}

READ64_HANDLER( dc_rtc_r )
{
	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;

	reg = decode_reg3216_64(space->machine(), offset, mem_mask, &shift);
	mame_printf_verbose("RTC:  Unmapped read %08x\n", 0x710000+reg*4);

	return (UINT64)state->dc_rtcregister[reg] << shift;
}

WRITE64_HANDLER( dc_rtc_w )
{
	dc_state *state = space->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;
	UINT32 old,dat;

	reg = decode_reg3216_64(space->machine(), offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = state->dc_rtcregister[reg];
	state->dc_rtcregister[reg] = dat & 0xFFFF; // 5f6c00+off*4=dat
	switch (reg)
	{
	case RTC1:
		if (state->dc_rtcregister[RTC3])
			state->dc_rtcregister[RTC3] = 0;
		else
			state->dc_rtcregister[reg] = old;
		break;
	case RTC2:
		if (state->dc_rtcregister[RTC3] == 0)
			state->dc_rtcregister[reg] = old;
		else
			state->dc_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));
		break;
	case RTC3:
		state->dc_rtcregister[RTC3] &= 1;
		break;
	}
	mame_printf_verbose("RTC: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x710000 + reg*4, dat, data, offset, mem_mask);
}

static TIMER_CALLBACK(dc_rtc_increment)
{
	dc_state *state = machine.driver_data<dc_state>();

    state->dc_rtcregister[RTC2] = (state->dc_rtcregister[RTC2] + 1) & 0xFFFF;
    if (state->dc_rtcregister[RTC2] == 0)
        state->dc_rtcregister[RTC1] = (state->dc_rtcregister[RTC1] + 1) & 0xFFFF;
}

/* fill the RTC registers with the proper start-up values */
static void rtc_initial_setup(running_machine &machine)
{
	dc_state *state = machine.driver_data<dc_state>();
	static UINT32 current_time;
	static int year_count,cur_year,i;
	static const int month_to_day_conversion[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
	system_time systime;
	machine.base_datetime(systime);

	memset(state->dc_rtcregister, 0, sizeof(state->dc_rtcregister));

	/* put the seconds */
	current_time = systime.local_time.second;
	/* put the minutes */
	current_time+= systime.local_time.minute*60;
	/* put the hours */
	current_time+= systime.local_time.hour*60*60;
	/* put the days (note -1) */
	current_time+= (systime.local_time.mday-1)*60*60*24;
	/* take the current year here for calculating leaps */
	cur_year = (systime.local_time.year);

	/* take the months - despite popular beliefs, leap years aren't just evenly divisible by 4 */
	if(((((cur_year % 4) == 0) && ((cur_year % 100) != 0)) || ((cur_year % 400) == 0)) && systime.local_time.month > 2)
		current_time+= (month_to_day_conversion[systime.local_time.month]+1)*60*60*24;
	else
		current_time+= (month_to_day_conversion[systime.local_time.month])*60*60*24;

	/* put the years */
	year_count = (cur_year-1949);

	for(i=0;i<year_count-1;i++)
		current_time += (((((i+1950) % 4) == 0) && (((i+1950) % 100) != 0)) || (((i+1950) % 400) == 0)) ? 60*60*24*366 : 60*60*24*365;

	state->dc_rtcregister[RTC2] = current_time & 0x0000ffff;
	state->dc_rtcregister[RTC1] = (current_time & 0xffff0000) >> 16;

	state->dc_rtc_timer = machine.scheduler().timer_alloc(FUNC(dc_rtc_increment));
}

MACHINE_START( dc )
{
	dc_state *state = machine.driver_data<dc_state>();

	rtc_initial_setup(machine);

	// save states
	state_save_register_global_pointer(machine, state->dc_rtcregister, 4);
	state_save_register_global_pointer(machine, state->dc_sysctrl_regs, 0x200/4);
	state_save_register_global_pointer(machine, state->g2bus_regs, 0x100/4);
	state_save_register_global(machine, state->m_wave_dma.aica_addr);
	state_save_register_global(machine, state->m_wave_dma.root_addr);
	state_save_register_global(machine, state->m_wave_dma.size);
	state_save_register_global(machine, state->m_wave_dma.dir);
	state_save_register_global(machine, state->m_wave_dma.flag);
	state_save_register_global(machine, state->m_wave_dma.indirect);
	state_save_register_global(machine, state->m_wave_dma.start);
	state_save_register_global(machine, state->m_wave_dma.sel);
	state_save_register_global(machine, state->m_pvr_dma.pvr_addr);
	state_save_register_global(machine, state->m_pvr_dma.sys_addr);
	state_save_register_global(machine, state->m_pvr_dma.size);
	state_save_register_global(machine, state->m_pvr_dma.sel);
	state_save_register_global(machine, state->m_pvr_dma.dir);
	state_save_register_global(machine, state->m_pvr_dma.flag);
	state_save_register_global(machine, state->m_pvr_dma.start);
	state_save_register_global_pointer(machine,state->pvrta_regs,0x2000/4);
	state_save_register_global_pointer(machine,state->pvrctrl_regs,0x100/4);
	state_save_register_global(machine, state->debug_dip_status);
	state_save_register_global_pointer(machine,state->tafifo_buff,32);
	state_save_register_global(machine, state->scanline);
	state_save_register_global(machine, state->next_y);
	state_save_register_global_pointer(machine,state->dc_sound_ram,sizeof(state->dc_sound_ram));
}

MACHINE_RESET( dc )
{
	dc_state *state = machine.driver_data<dc_state>();

	/* halt the ARM7 */
	cputag_set_input_line(machine, "soundcpu", INPUT_LINE_RESET, ASSERT_LINE);

	memset(state->dc_sysctrl_regs, 0, sizeof(state->dc_sysctrl_regs));

	state->dc_rtc_timer->adjust(attotime::zero, 0, attotime::from_seconds(1));

	state->dc_sysctrl_regs[SB_SBREV] = 0x0b;
}

READ64_DEVICE_HANDLER( dc_aica_reg_r )
{
	//	dc_state *state = device->machine().driver_data<dc_state>();
	//int reg;
	UINT64 shift;

	/*reg = */decode_reg32_64(device->machine(), offset, mem_mask, &shift);

//  mame_printf_verbose("AICA REG: [%08x] read %" I64FMT "x, mask %" I64FMT "x\n", 0x700000+reg*4, (UINT64)offset, mem_mask);

	return (UINT64) aica_r(device, offset*2, 0xffff)<<shift;
}

WRITE64_DEVICE_HANDLER( dc_aica_reg_w )
{
	//	dc_state *state = device->machine().driver_data<dc_state>();
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg32_64(device->machine(), offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	if (reg == (0x2c00/4))
	{
		if (dat & 1)
		{
			/* halt the ARM7 */
			cputag_set_input_line(device->machine(), "soundcpu", INPUT_LINE_RESET, ASSERT_LINE);
		}
		else
		{
			/* it's alive ! */
			cputag_set_input_line(device->machine(), "soundcpu", INPUT_LINE_RESET, CLEAR_LINE);
		}
    }

	aica_w(device, offset*2, dat, shift ? ((mem_mask>>32)&0xffff) : (mem_mask & 0xffff));

//  mame_printf_verbose("AICA REG: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x700000+reg*4, dat, data, offset, mem_mask);
}

READ32_DEVICE_HANDLER( dc_arm_aica_r )
{
	return aica_r(device, offset*2, 0xffff) & 0xffff;
}

WRITE32_DEVICE_HANDLER( dc_arm_aica_w )
{
	aica_w(device, offset*2, data, mem_mask&0xffff);
}

