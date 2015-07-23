// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert, David Haywood, Samuele Zannoli, R. Belmont, ElSemi
/*

    dc.c - Sega Dreamcast hardware

*/

#include "emu.h"
#include "debugger.h"
#include "includes/dc.h"
#include "cpu/sh4/sh4.h"
#include "machine/mie.h"
#include "machine/naomig1.h"
#include "video/powervr2.h"

// for now, make buggy GCC/Mingw STFU about I64FMT
#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

#define DEBUG_REGISTERS (1)

#if DEBUG_REGISTERS

#define DEBUG_SYSCTRL   (0)
#define DEBUG_AICA_DMA (0)

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

void dc_state::generic_dma(UINT32 main_adr, void *dma_ptr, UINT32 length, UINT32 size, bool to_mainram)
{
	sh4_ddt_dma ddt;
	if(to_mainram)
		ddt.destination = main_adr;
	else
		ddt.source = main_adr;
	ddt.buffer = dma_ptr;
	ddt.length = length;
	ddt.size =size;
	ddt.direction = to_mainram;
	ddt.channel = 0;
	ddt.mode = -1;
	m_maincpu->sh4_dma_ddt(&ddt);
}

TIMER_CALLBACK_MEMBER(dc_state::g2_dma_irq)
{
	m_g2_dma[param].start = g2bus_regs[SB_ADST + (param * 8)] = 0;
	dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_AICA << param;
	dc_update_interrupt_status();
}

WRITE8_MEMBER(dc_state::g1_irq)
{
	switch(data) {
	case naomi_g1_device::DMA_GDROM_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_GDROM;
		break;
	}
	dc_update_interrupt_status();
}

WRITE8_MEMBER(dc_state::pvr_irq)
{
	switch(data) {
	case powervr2_device::EOXFER_YUV_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_YUV;
		break;

	case powervr2_device::EOXFER_OPLST_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_OPLST;
		break;

	case powervr2_device::EOXFER_OPMV_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_OPMV;
		break;

	case powervr2_device::EOXFER_TRLST_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_TRLST;
		break;

	case powervr2_device::EOXFER_TRMV_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_TRMV;
		break;

	case powervr2_device::EOXFER_PTLST_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOXFER_PTLST;
		break;

	case powervr2_device::VBL_IN_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_VBL_IN;
		break;

	case powervr2_device::VBL_OUT_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_VBL_OUT;
		break;

	case powervr2_device::HBL_IN_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_HBL_IN;
		break;

	case powervr2_device::EOR_VIDEO_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_VIDEO;
		break;

	case powervr2_device::EOR_TSP_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_TSP;
		break;

	case powervr2_device::EOR_ISP_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_EOR_ISP;
		break;

	case powervr2_device::DMA_PVR_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_PVR;
		break;

	case powervr2_device::ERR_ISP_LIMIT_IRQ:
		dc_sysctrl_regs[SB_ISTERR] |= IST_ERR_ISP_LIMIT;
		break;

	case powervr2_device::ERR_PVRIF_ILL_ADDR_IRQ:
		dc_sysctrl_regs[SB_ISTERR] |= IST_ERR_PVRIF_ILL_ADDR;
		break;
	}
	dc_update_interrupt_status();
}

void dc_maple_irq(running_machine &machine)
{
	dc_state *state = machine.driver_data<dc_state>();

	state->dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_MAPLE;
	state->dc_update_interrupt_status();
}

TIMER_CALLBACK_MEMBER(dc_state::ch2_dma_irq)
{
	dc_sysctrl_regs[SB_C2DLEN]=0;
	dc_sysctrl_regs[SB_C2DST]=0;
	dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_CH2;
	dc_update_interrupt_status();
}

void dc_state::g2_dma_execute(address_space &space, int channel)
{
	UINT32 src,dst,size;
	dst = m_g2_dma[channel].g2_addr;
	src = m_g2_dma[channel].root_addr;
	size = 0;

	/* 0 rounding size = 32 Mbytes */
	if (m_g2_dma[channel].size == 0) { m_g2_dma[channel].size = 0x200000; }

	if (m_g2_dma[channel].dir == 0)
	{
		for (; size<m_g2_dma[channel].size; size += 4)
		{
			space.write_dword(dst,space.read_dword(src));
			src+=4;
			dst+=4;
		}
	}
	else
	{
		for (; size<m_g2_dma[channel].size; size += 4)
		{
			space.write_dword(src,space.read_dword(dst));
			src+=4;
			dst+=4;
		}
	}

	/* update the params*/
	m_g2_dma[channel].g2_addr = g2bus_regs[SB_ADSTAG + (channel * 8)] = dst;
	m_g2_dma[channel].root_addr = g2bus_regs[SB_ADSTAR + (channel * 8)] = src;
	m_g2_dma[channel].size = g2bus_regs[SB_ADLEN + (channel * 8)] = 0;
	m_g2_dma[channel].flag = (m_g2_dma[channel].indirect & 1) ? 1 : 0;
	/* Note: if you trigger an instant DMA IRQ trigger, sfz3upper doesn't play any bgm. */
	/* TODO: timing of this */
	machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(m_g2_dma[channel].size / 4), timer_expired_delegate(FUNC(dc_state::g2_dma_irq), this), channel);
}

// register decode helpers

// this accepts only 32-bit accesses
int dc_state::decode_reg32_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		osd_printf_verbose("%s:Wrong mask!\n", machine().describe_context());
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
int dc_state::decode_reg3216_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 16&32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != U64(0x0000ffff00000000)) && (mem_mask != U64(0x000000000000ffff)) &&
		(mem_mask != U64(0xffffffff00000000)) && (mem_mask != U64(0x00000000ffffffff)))
	{
		osd_printf_verbose("%s:Wrong mask!\n", machine().describe_context());
//      debugger_break(machine);
	}

	if (mem_mask & U64(0x0000ffff00000000))
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

int dc_state::dc_compute_interrupt_level()
{
	UINT32 ln,lx,le;

	ln=dc_sysctrl_regs[SB_ISTNRM] & dc_sysctrl_regs[SB_IML6NRM];
	lx=dc_sysctrl_regs[SB_ISTEXT] & dc_sysctrl_regs[SB_IML6EXT];
	le=dc_sysctrl_regs[SB_ISTERR] & dc_sysctrl_regs[SB_IML6ERR];
	if (ln | lx | le)
	{
		return 6;
	}

	ln=dc_sysctrl_regs[SB_ISTNRM] & dc_sysctrl_regs[SB_IML4NRM];
	lx=dc_sysctrl_regs[SB_ISTEXT] & dc_sysctrl_regs[SB_IML4EXT];
	le=dc_sysctrl_regs[SB_ISTERR] & dc_sysctrl_regs[SB_IML4ERR];
	if (ln | lx | le)
	{
		return 4;
	}

	ln=dc_sysctrl_regs[SB_ISTNRM] & dc_sysctrl_regs[SB_IML2NRM];
	lx=dc_sysctrl_regs[SB_ISTEXT] & dc_sysctrl_regs[SB_IML2EXT];
	le=dc_sysctrl_regs[SB_ISTERR] & dc_sysctrl_regs[SB_IML2ERR];
	if (ln | lx | le)
	{
		return 2;
	}

	return 0;
}

void dc_state::dc_update_interrupt_status()
{
	int level;

	if (dc_sysctrl_regs[SB_ISTERR])
	{
		dc_sysctrl_regs[SB_ISTNRM] |= IST_ERROR;
	}
	else
	{
		dc_sysctrl_regs[SB_ISTNRM] &= ~IST_ERROR;
	}

	if (dc_sysctrl_regs[SB_ISTEXT])
	{
		dc_sysctrl_regs[SB_ISTNRM] |= IST_G1G2EXTSTAT;
	}
	else
	{
		dc_sysctrl_regs[SB_ISTNRM] &= ~IST_G1G2EXTSTAT;
	}

	level=dc_compute_interrupt_level();
	m_maincpu->sh4_set_irln_input(15-level);

	/* Wave DMA HW trigger */
	for (int i = 0; i < 4; i++)
	{
		if (m_g2_dma[i].flag && ((m_g2_dma[i].sel & 2) == 2))
		{
			if ((dc_sysctrl_regs[SB_G2DTNRM] & dc_sysctrl_regs[SB_ISTNRM]) || (dc_sysctrl_regs[SB_G2DTEXT] & dc_sysctrl_regs[SB_ISTEXT]))
			{
				address_space &space = m_maincpu->space(AS_PROGRAM);

				printf("Wave DMA HW trigger\n");
				g2_dma_execute(space, i);
			}
		}
	}

	/* PVR-DMA HW trigger */
	if(m_powervr2->m_pvr_dma.flag && ((m_powervr2->m_pvr_dma.sel & 1) == 1))
	{
		if((dc_sysctrl_regs[SB_PDTNRM] & dc_sysctrl_regs[SB_ISTNRM]) || (dc_sysctrl_regs[SB_PDTEXT] & dc_sysctrl_regs[SB_ISTEXT]))
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);

			printf("PVR-DMA HW trigger\n");
			m_powervr2->pvr_dma_execute(space);
		}
	}
}

READ64_MEMBER(dc_state::dc_sysctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(offset, mem_mask, &shift);

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x41) && (reg != 0x42) && (reg != 0x23) && (reg > 2))  // filter out IRQ status reads
	{
		osd_printf_verbose("SYSCTRL: [%08x] read %x @ %x (reg %x: %s), mask %" I64FMT "x (PC=%x)\n", 0x5f6800+reg*4, dc_sysctrl_regs[reg], offset, reg, sysctrl_names[reg], mem_mask, space.device().safe_pc());
	}
	#endif

	return (UINT64)dc_sysctrl_regs[reg] << shift;
}

WRITE64_MEMBER(dc_state::dc_sysctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 old,dat;
	UINT32 address;
	struct sh4_ddt_dma ddtdata;

	reg = decode_reg32_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	old = dc_sysctrl_regs[reg];
	dc_sysctrl_regs[reg] = dat; // 5f6800+off*4=dat
	switch (reg)
	{
		case SB_C2DST:
			if(((old & 1) == 0) && (dat & 1)) // 0 -> 1
			{
				address=(dc_sysctrl_regs[SB_C2DSTAT] & 0x03ffffe0) | 0x10000000;
				if(dc_sysctrl_regs[SB_C2DSTAT] & 0x1f)
					printf("C2DSTAT just used to reserved bits %02x\n",dc_sysctrl_regs[SB_C2DSTAT] & 0x1f);

				ddtdata.destination=address;
				/* 0 rounding size = 16 Mbytes */
				if(dc_sysctrl_regs[SB_C2DLEN] == 0)
					ddtdata.length = 0x1000000;
				else
					ddtdata.length = dc_sysctrl_regs[SB_C2DLEN];
				ddtdata.size=1;
				ddtdata.direction=0;
				ddtdata.channel=2;
				ddtdata.mode=25; //011001
				m_maincpu->sh4_dma_ddt(&ddtdata);
				#if DEBUG_SYSCTRL
				if ((address >= 0x11000000) && (address <= 0x11FFFFFF))
					if (dc_sysctrl_regs[SB_LMMODE0])
						printf("SYSCTRL: Ch2 direct display lists dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 1
					else
						osd_printf_verbose("SYSCTRL: Ch2 direct textures dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 0
				else if ((address >= 0x13000000) && (address <= 0x13FFFFFF))
					if (dc_sysctrl_regs[SB_LMMODE1])
						printf("SYSCTRL: Ch2 direct display lists dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 1
					else
						osd_printf_verbose("SYSCTRL: Ch2 direct textures dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]); // 0
				else if ((address >= 0x10800000) && (address <= 0x10ffffff))
					printf("SYSCTRL: Ch2 YUV dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]);
				else if ((address >= 0x10000000) && (address <= 0x107fffff))
					osd_printf_verbose("SYSCTRL: Ch2 TA Display List dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]);
				else
					osd_printf_verbose("SYSCTRL: Ch2 unknown dma %x from %08x to %08x (lmmode0=%d lmmode1=%d)\n", dc_sysctrl_regs[SB_C2DLEN], ddtdata.source-ddtdata.length, dc_sysctrl_regs[SB_C2DSTAT],dc_sysctrl_regs[SB_LMMODE0],dc_sysctrl_regs[SB_LMMODE1]);
				#endif

				if ((!(address & 0x01000000)))
					dc_sysctrl_regs[SB_C2DSTAT]=address;
				else //direct texture path
					dc_sysctrl_regs[SB_C2DSTAT]=address+ddtdata.length;

				/* TODO: timing is a guess */
				machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(ddtdata.length/4), timer_expired_delegate(FUNC(dc_state::ch2_dma_irq),this));
			}
			break;

		case SB_ISTNRM:
			dc_sysctrl_regs[SB_ISTNRM] = old & ~(dat | 0xC0000000); // bits 31,30 ro
			dc_update_interrupt_status();
			break;

		case SB_ISTEXT:
			dc_sysctrl_regs[SB_ISTEXT] = old;
			dc_update_interrupt_status();
			break;

		case SB_ISTERR:
			dc_sysctrl_regs[SB_ISTERR] = old & ~dat;
			dc_update_interrupt_status();
			break;
		case SB_SDST:
			if(dat & 1)
			{
				// TODO: Sort-DMA routine goes here
				printf("Sort-DMA irq\n");

				dc_sysctrl_regs[SB_SDST] = 0;
				dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_SORT;
				dc_update_interrupt_status();
			}
			break;
	}

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x42) && (reg > 2))    // filter out IRQ acks and ch2 dma
	{
		osd_printf_verbose("SYSCTRL: write %" I64FMT "x to %x (reg %x), mask %" I64FMT "x\n", data>>shift, offset, reg, /*sysctrl_names[reg],*/ mem_mask);
	}
	#endif
}

READ64_MEMBER(dc_state::dc_gdrom_r )
{
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

WRITE64_MEMBER(dc_state::dc_gdrom_w )
{
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

	osd_printf_verbose("GDROM: [%08x=%x]write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x5f7000+off*4, dat, data, offset, mem_mask);
}

READ64_MEMBER(dc_state::dc_g2_ctrl_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(offset, mem_mask, &shift);
	osd_printf_verbose("G2CTRL:  Unmapped read %08x\n", 0x5f7800+reg*4);
	return (UINT64)g2bus_regs[reg] << shift;
}

WRITE64_MEMBER(dc_state::dc_g2_ctrl_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;
	UINT8 old;

	reg = decode_reg32_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);

	g2bus_regs[reg] = dat; // 5f7800+reg*4=dat

	if (reg >= (0x80 / 4))
		return;
	int g2chan = reg >> 3;
	switch (reg & 7)
	{
		/*G2 Address register*/
		case SB_ADSTAG: m_g2_dma[g2chan].g2_addr = dat; break;
		/*Root address (work ram)*/
		case SB_ADSTAR: m_g2_dma[g2chan].root_addr = dat; break;
		/*DMA size (in dword units, bit 31 is "set dma initiation enable setting to 0"*/
		case SB_ADLEN:
			m_g2_dma[g2chan].size = dat & 0x7fffffff;
			m_g2_dma[g2chan].indirect = (dat & 0x80000000) >> 31;
			break;
		/*0 = root memory to aica / 1 = aica to root memory*/
		case SB_ADDIR: m_g2_dma[g2chan].dir = (dat & 1); break;
		/*dma flag (active HIGH, bug in docs)*/
		case SB_ADEN: m_g2_dma[g2chan].flag = (dat & 1); break;
		/*
		SB_ADTSEL
		bit 1: (0) Wave DMA through SB_ADST flag (1) Wave DMA through irq trigger, defined by SB_G2DTNRM / SB_G2DTEXT
		*/
		case SB_ADTSEL: m_g2_dma[g2chan].sel = dat & 7; break;
		/*ready for dma'ing*/
		case SB_ADST:
			old = m_g2_dma[g2chan].start & 1;
			m_g2_dma[g2chan].start = dat & 1;

			#if DEBUG_AICA_DMA
			printf("AICA: G2-DMA start \n");
			printf("DST %08x SRC %08x SIZE %08x IND %02x\n",m_g2_dma[g2chan].g2_addr,m_g2_dma[g2chan].root_addr,m_g2_dma[g2chan].size,m_g2_dma[g2chan].indirect);
			printf("SEL %08x ST  %08x FLAG %08x DIR %02x\n",m_g2_dma[g2chan].sel,m_g2_dma[g2chan].start,m_g2_dma[g2chan].flag,m_g2_dma[g2chan].dir);
			#endif

			//osd_printf_verbose("SB_ADST data %08x\n",dat);
			if (((old & 1) == 0) && m_g2_dma[g2chan].flag && m_g2_dma[g2chan].start && ((m_g2_dma[g2chan].sel & 2) == 0)) // 0 -> 1
				g2_dma_execute(space, g2chan);
			break;

		default:
			/* might access the unhandled DMAs, so tell us if this happens. */
			//printf("Unhandled G2 register [%08x] -> %08x\n",reg,dat);
			break;
	}
}

int dc_state::decode_reg_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift)
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

READ64_MEMBER(dc_state::dc_modem_r )
{
	int reg;
	UINT64 shift;

	reg = decode_reg32_64(offset, mem_mask, &shift);

	// from ElSemi: this makes Atomiswave do it's "verbose boot" with a Sammy logo and diagnostics instead of just running the cart.
	// our PVR emulation is apparently not good enough for that to work yet though.
	if (reg == 0x280/4)
	{
		return U64(0xffffffffffffffff);
	}

	osd_printf_verbose("MODEM:  Unmapped read %08x\n", 0x600000+reg*4);
	return 0;
}

WRITE64_MEMBER(dc_state::dc_modem_w )
{
	int reg;
	UINT64 shift;
	UINT32 dat;

	reg = decode_reg32_64(offset, mem_mask, &shift);
	dat = (UINT32)(data >> shift);
	osd_printf_verbose("MODEM: [%08x=%x] write %" I64FMT "x to %x, mask %" I64FMT "x\n", 0x600000+reg*4, dat, data, offset, mem_mask);
}

#define SAVE_G2DMA(x) \
	save_item(NAME(m_g2_dma[x].g2_addr)); \
	save_item(NAME(m_g2_dma[x].root_addr)); \
	save_item(NAME(m_g2_dma[x].size)); \
	save_item(NAME(m_g2_dma[x].dir)); \
	save_item(NAME(m_g2_dma[x].flag)); \
	save_item(NAME(m_g2_dma[x].indirect)); \
	save_item(NAME(m_g2_dma[x].start)); \
	save_item(NAME(m_g2_dma[x].sel));

void dc_state::machine_start()
{
	// dccons doesn't have a specific g1 device yet
	if(m_naomig1)
		m_naomig1->set_dma_cb(naomi_g1_device::dma_cb(FUNC(dc_state::generic_dma), this));

	// save states
	save_pointer(NAME(dc_sysctrl_regs), 0x200/4);
	save_pointer(NAME(g2bus_regs), 0x100/4);
	save_pointer(NAME(dc_sound_ram.target()),dc_sound_ram.bytes());
	SAVE_G2DMA(0)
	SAVE_G2DMA(1)
	SAVE_G2DMA(2)
	SAVE_G2DMA(3)
}

void dc_state::machine_reset()
{
	/* halt the ARM7 */
	m_armrst = 1;
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	memset(dc_sysctrl_regs, 0, sizeof(dc_sysctrl_regs));

	dc_sysctrl_regs[SB_SBREV] = 0x0b;
}

READ32_MEMBER(dc_state::dc_aica_reg_r)
{
//  osd_printf_verbose("AICA REG: [%08x] read %" I64FMT "x, mask %" I64FMT "x\n", 0x700000+reg*4, (UINT64)offset, mem_mask);

	if(offset == 0x2c00/4)
		return m_armrst;

	return m_aica->read(space, offset*2, 0xffff);
}

WRITE32_MEMBER(dc_state::dc_aica_reg_w)
{
	if (offset == (0x2c00/4))
	{
		if(ACCESSING_BITS_0_7)
		{
			m_armrst = data & 1;

			if (data & 1)
			{
				/* halt the ARM7 */
				m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}
			else
			{
				/* it's alive ! */
				m_soundcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
		}
	}

	m_aica->write(space, offset*2, data, 0xffff);

//  osd_printf_verbose("AICA REG: [%08x=%x] write %x to %x, mask %" I64FMT "x\n", 0x700000+reg*4, data, offset, mem_mask);
}

READ32_MEMBER(dc_state::dc_arm_aica_r)
{
	return m_aica->read(space, offset*2, 0xffff) & 0xffff;
}

WRITE32_MEMBER(dc_state::dc_arm_aica_w)
{
	m_aica->write(space, offset*2, data, mem_mask&0xffff);
}

TIMER_DEVICE_CALLBACK_MEMBER(dc_state::dc_scanline)
{
	m_powervr2->pvr_scanline_timer(param);
}
#if (defined(__MINGW32__) && (__GNUC__ >= 5))
#pragma GCC diagnostic pop
#endif

