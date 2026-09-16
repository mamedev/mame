// license:LGPL-2.1+
// copyright-holders: Samuele Zannoli, R. Belmont, ElSemi, David Haywood, Angelo Salese, Olivier Galibert, MetalliC
/*

    dc.cpp - Sega Dreamcast hardware

    Misc interfacing to common DC buses over the various clients.

*/

#include "emu.h"
#include "dc.h"

#include "mie.h"

#include "cpu/sh/sh4.h"

//#include "debugger.h"

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

void dc_state::generic_dma(uint32_t main_adr, void *dma_ptr, uint32_t length, uint32_t size, bool to_mainram)
{
	sh4_ddt_dma ddt;
	if(to_mainram)
		ddt.destination = main_adr;
	else
		ddt.source = main_adr;
	ddt.buffer = dma_ptr;
	ddt.length = length;
	ddt.size = size;
	ddt.direction = to_mainram;
	ddt.channel = 0;
	ddt.mode = -1;
	m_maincpu->sh4_dma_ddt(&ddt);
}

void dc_state::g2_dma_end_w(offs_t channel, u8 state)
{
	dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_AICA << channel;
	dc_update_interrupt_status();
}

void dc_state::g2_dma_error_ia_w(offs_t channel, u8 state)
{
	dc_sysctrl_regs[SB_ISTERR] |= 0x8000 << channel;
	dc_update_interrupt_status();
}

void dc_state::g2_dma_error_ov_w(offs_t channel, u8 state)
{
	dc_sysctrl_regs[SB_ISTERR] |= 0x80000 << channel;
	dc_update_interrupt_status();
}

void dc_state::g1_irq(uint8_t data)
{
	switch(data) {
	case naomi_g1_device::DMA_GDROM_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_GDROM;
		break;
	}
	dc_update_interrupt_status();
}

void dc_state::pvr_irq(uint8_t data)
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

void dc_state::maple_irq(uint8_t data)
{
	switch(data) {
	case maple_dc_device::DMA_MAPLE_IRQ:
		dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_MAPLE;
		break;
	}
	dc_update_interrupt_status();
}

TIMER_CALLBACK_MEMBER(dc_state::ch2_dma_irq)
{
	dc_sysctrl_regs[SB_C2DLEN] = 0;
	dc_sysctrl_regs[SB_C2DST] = 0;
	dc_sysctrl_regs[SB_ISTNRM] |= IST_DMA_CH2;
	dc_update_interrupt_status();
}

// register decode helpers

// this accepts only 32-bit accesses
int dc_state::decode_reg32_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != 0xffffffff00000000U) && (mem_mask != 0x00000000ffffffffU))
	{
		osd_printf_verbose("%s:Wrong mask!\n", machine().describe_context());
		//machine().debug_break();
	}

	if (mem_mask == 0xffffffff00000000U)
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

// this accepts only 32 and 16 bit accesses
int dc_state::decode_reg3216_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 16&32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != 0x0000ffff00000000U) && (mem_mask != 0x000000000000ffffU) &&
		(mem_mask != 0xffffffff00000000U) && (mem_mask != 0x00000000ffffffffU))
	{
		osd_printf_verbose("%s:Wrong mask!\n", machine().describe_context());
		//machine().debug_break();
	}

	if (ACCESSING_BITS_32_47)
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

int dc_state::dc_compute_interrupt_level()
{
	uint32_t ln,lx,le;

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
	m_g2if->hw_irq_trigger_hs(
		dc_sysctrl_regs[SB_G2DTNRM] & dc_sysctrl_regs[SB_ISTNRM],
		dc_sysctrl_regs[SB_G2DTEXT] & dc_sysctrl_regs[SB_ISTEXT]
	);

	/* PVR-DMA HW trigger */
	if(m_powervr2->m_pvr_dma.flag && ((m_powervr2->m_pvr_dma.sel & 1) == 1))
	{
		if((dc_sysctrl_regs[SB_PDTNRM] & dc_sysctrl_regs[SB_ISTNRM]) || (dc_sysctrl_regs[SB_PDTEXT] & dc_sysctrl_regs[SB_ISTEXT]))
		{
			address_space &space = m_maincpu->space(AS_PROGRAM);

			logerror("PVR-DMA HW trigger\n");
			m_powervr2->pvr_dma_execute(space);
		}
	}
}

// TODO: convert SYSCTRL to device I/F (NAOMI2 needs two of these)
uint64_t dc_state::dc_sysctrl_r(offs_t offset, uint64_t mem_mask)
{
	int reg;
	uint64_t shift;

	reg = decode_reg32_64(offset, mem_mask, &shift);

	#if DEBUG_SYSCTRL
	if ((reg != 0x40) && (reg != 0x41) && (reg != 0x42) && (reg != 0x23) && (reg > 2))  // filter out IRQ status reads
	{
		osd_printf_verbose("SYSCTRL: [%08x] read %x @ %x (reg %x: %s), mask %x (PC=%x)\n", 0x5f6800+reg*4, dc_sysctrl_regs[reg], offset, reg, sysctrl_names[reg], mem_mask, m_maincpu->pc());
	}
	#endif

	return (uint64_t)dc_sysctrl_regs[reg] << shift;
}

void dc_state::dc_sysctrl_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	int reg;
	uint64_t shift;
	uint32_t old,dat;
	uint32_t address;
	struct sh4_ddt_dma ddtdata;

	reg = decode_reg32_64(offset, mem_mask, &shift);
	dat = (uint32_t)(data >> shift);
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
				m_ch2_dma_irq_timer->adjust(m_maincpu->cycles_to_attotime(ddtdata.length/4));
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
		osd_printf_verbose("SYSCTRL: write %x to %x (reg %x), mask %x\n", data>>shift, offset, reg, /*sysctrl_names[reg],*/ mem_mask);
	}
	#endif
}

uint64_t dc_state::dc_gdrom_r(offs_t offset, uint64_t mem_mask)
{
	uint32_t off;

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

void dc_state::dc_gdrom_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	uint32_t dat,off;

	if ((int)~mem_mask & 1)
	{
		dat=(uint32_t)(data >> 32);
		off=(offset << 1) | 1;
	}
	else
	{
		dat=(uint32_t)data;
		off=offset << 1;
	}

	osd_printf_verbose("GDROM: [%08x=%x]write %x to %x, mask %x\n", 0x5f7000+off*4, dat, data, offset, mem_mask);
}

int dc_state::decode_reg_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift)
{
	int reg = offset * 2;

	*shift = 0;

	// non 32-bit accesses have not yet been seen here, we need to know when they are
	if ((mem_mask != 0xffffffff00000000U) && (mem_mask != 0x00000000ffffffffU))
	{
		/*assume to return the lower 32-bits ONLY*/
		return reg & 0xffffffff;
	}

	if (mem_mask == 0xffffffff00000000U)
	{
		reg++;
		*shift = 32;
	}

	return reg;
}

uint64_t dc_state::dc_modem_r(offs_t offset, uint64_t mem_mask)
{
	int reg;
	uint64_t shift;

	reg = decode_reg32_64(offset, mem_mask, &shift);

	// from ElSemi: this makes Atomiswave do it's "verbose boot" with a Sammy logo and diagnostics instead of just running the cart.
	// our PVR emulation is apparently not good enough for that to work yet though.
	if (reg == 0x280/4)
	{
		return 0xffffffffffffffffU;
	}

	osd_printf_verbose("MODEM:  Unmapped read %08x\n", 0x600000+reg*4);
	return 0;
}

void dc_state::dc_modem_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	int reg;
	uint64_t shift;
	uint32_t dat;

	reg = decode_reg32_64(offset, mem_mask, &shift);
	dat = (uint32_t)(data >> shift);
	osd_printf_verbose("MODEM: [%08x=%x] write %x to %x, mask %x\n", 0x600000+reg*4, dat, data, offset, mem_mask);
}

void dc_state::machine_start()
{
	// dccons doesn't have a specific g1 device yet
	if(m_naomig1)
		m_naomig1->set_dma_cb(naomi_g1_device::dma_cb(&dc_state::generic_dma, this));

	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
	// TODO: repeated in dccons.cpp init_dc (NAOMI also uses double RAM)
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0cffffff, false, dc_ram);

	// save states
	save_pointer(NAME(dc_sysctrl_regs), 0x200/4);

	m_ch2_dma_irq_timer = timer_alloc(FUNC(dc_state::ch2_dma_irq), this);
}

void dc_state::machine_reset()
{
	/* halt the ARM7 */
	m_armrst = 1;
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	memset(dc_sysctrl_regs, 0, sizeof(dc_sysctrl_regs));

	dc_sysctrl_regs[SB_SBREV] = 0x0b;

	m_ch2_dma_irq_timer->adjust(attotime::never);
}

uint32_t dc_state::dc_aica_reg_r(offs_t offset, uint32_t mem_mask)
{
//  osd_printf_verbose("AICA REG: [%08x] read %x, mask %x\n", 0x700000+reg*4, (uint64_t)offset, mem_mask);

	if(offset == 0x2c00/4)
		return m_armrst;

	return m_aica->read(offset*2);
}

void dc_state::dc_aica_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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

	m_aica->write(offset*2, data, 0xffff);

//  osd_printf_verbose("AICA REG: [%08x=%x] write %x to %x, mask %x\n", 0x700000+reg*4, data, offset, mem_mask);
}

uint32_t dc_state::dc_arm_aica_r(offs_t offset)
{
	return m_aica->read(offset*2) & 0xffff;
}

void dc_state::dc_arm_aica_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_aica->write(offset*2, data, mem_mask&0xffff);
}

uint16_t dc_state::soundram_r(offs_t offset)
{
	return dc_sound_ram[offset];
}

void dc_state::soundram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&dc_sound_ram[offset]);
}

void dc_state::aica_irq(int state)
{
	m_soundcpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}

void dc_state::sh4_aica_irq(int state)
{
	if(state)
		dc_sysctrl_regs[SB_ISTEXT] |= IST_EXT_AICA;
	else
		dc_sysctrl_regs[SB_ISTEXT] &= ~IST_EXT_AICA;

	dc_update_interrupt_status();
}

void dc_state::external_irq(int state)
{
	if (state)
		dc_sysctrl_regs[SB_ISTEXT] |= IST_EXT_EXTERNAL;
	else
		dc_sysctrl_regs[SB_ISTEXT] &= ~IST_EXT_EXTERNAL;

	dc_update_interrupt_status();
}

MACHINE_RESET_MEMBER(dc_state,dc_console)
{
	dc_state::machine_reset();
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
}

TIMER_DEVICE_CALLBACK_MEMBER(dc_state::dc_scanline)
{
	m_powervr2->pvr_scanline_timer(param);
}

void dc_state::system_bus_config(machine_config &config, const char *cpu_tag)
{
	DC_G2IF(config, m_g2if, XTAL(25'000'000));
	m_g2if->set_host_space(cpu_tag, AS_PROGRAM);
	m_g2if->int_cb().set(FUNC(dc_state::g2_dma_end_w));
	m_g2if->error_ia_cb().set(FUNC(dc_state::g2_dma_error_ia_w));
	m_g2if->error_ov_cb().set(FUNC(dc_state::g2_dma_error_ov_w));
}
