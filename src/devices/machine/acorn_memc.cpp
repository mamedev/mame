// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller, Sandro Ronco
/**************************************************************************************************

    Acorn RISC Machine Memory Controller (MEMC)

    TODO:
    - VIDC DMA interface needs to be cleaned up.
    - Slave mode.

**************************************************************************************************/

#include "emu.h"
#include "acorn_memc.h"

#include "debug/debugcon.h"
#include "debug/debugcmd.h"
#include "debugger.h"

#include <functional>

//#define VERBOSE 1
#include "logmacro.h"

DEFINE_DEVICE_TYPE(ACORN_MEMC, acorn_memc_device, "memc", "Acorn MEMC")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

acorn_memc_device::acorn_memc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ACORN_MEMC, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_vidc(*this, finder_base::DUMMY_TAG)
	, m_space_config("memc", ENDIANNESS_LITTLE, 32, 26, 0)
	, m_abort_w(*this)
	, m_sirq_w(*this)
	, m_output_dram_rowcol(false)
{
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector acorn_memc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void acorn_memc_device::memc_map_debug_commands(const std::vector<std::string> &params)
{
	uint64_t offset;
	if (params.size() != 1 || !machine().debugger().commands().validate_number_parameter(params[0], offset))
		return;

	// figure out the page number and offset in the page
	uint32_t pagesize = m_page_sizes[m_pagesize];
	uint32_t page = offset / pagesize;
	uint32_t poffs = offset % pagesize;

	machine().debugger().console().printf("0x%08lx == ", offset);
	if (offset >= 0x02000000)
		machine().debugger().console().printf("physical\n");
	else if (m_pages[page] == -1)
		machine().debugger().console().printf("unmapped\n");
	else
		machine().debugger().console().printf("0x%08lx (PPL %x)\n", 0x02000000 | ((m_pages[page] * pagesize) + poffs), m_pages_ppl[page]);
}

void acorn_memc_device::device_resolve_objects()
{
	m_abort_w.resolve_safe();
	m_sirq_w.resolve_safe();
}

void acorn_memc_device::device_start()
{
	m_space = &space();

	save_item(NAME(m_spvmd));
	save_item(NAME(m_pagesize));
	save_item(NAME(m_latchrom));
	save_item(NAME(m_video_dma_on));
	save_item(NAME(m_sound_dma_on));
	save_item(NAME(m_cursor_enabled));
	save_item(NAME(m_os_mode));
	save_item(NAME(m_vidinit));
	save_item(NAME(m_vidstart));
	save_item(NAME(m_vidend));
	save_item(NAME(m_vidcur));
	save_item(NAME(m_cinit));
	save_item(NAME(m_sndstart));
	save_item(NAME(m_sndend));
	save_item(NAME(m_sndcur));
	save_item(NAME(m_sndendcur));
	save_item(NAME(m_pages));
	save_item(NAME(m_pages_ppl));

	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		using namespace std::placeholders;
		machine().debugger().console().register_command("memc_map", CMDFLAG_NONE, 1, 1, std::bind(&acorn_memc_device::memc_map_debug_commands, this, _1));
	}
}

void acorn_memc_device::device_reset()
{
	m_latchrom       = true;    // map in the boot ROM
	m_pagesize       = 0;
	m_video_dma_on   = false;
	m_sound_dma_on   = false;
	m_cursor_enabled = false;
	m_os_mode        = false;
	m_vidinit        = 0;
	m_vidstart       = 0;
	m_vidend         = 0;
	m_vidcur         = 0;
	m_cinit          = 0;
	m_sndstart       = 0;
	m_sndend         = 0;
	m_sndcur         = 0;
	m_sndendcur      = 0;
	m_spvmd          = ASSERT_LINE;

	// kill all MEMC mappings
	std::fill(std::begin(m_pages), std::end(m_pages), -1);   // indicate unmapped
	std::fill(std::begin(m_pages_ppl), std::end(m_pages_ppl), 0);
}

uint32_t acorn_memc_device::invalid_access(bool is_write, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (!machine().side_effects_disabled())
	{
		if (is_write)
			logerror("abort W 0x%08x 0x%08x (0x%08x)\n", offset << 2, data, mem_mask);
		else
			logerror("abort R 0x%08x (0x%08x)\n", offset << 2, mem_mask);

		m_abort_w(ASSERT_LINE);
	}

	return 0xdeadbeef;
}

bool acorn_memc_device::is_valid_access(int page, bool write)
{
	if (m_pages[page] != -1)
	{
		if (m_spvmd || machine().side_effects_disabled())
			return true;

		switch (m_pages_ppl[page])
		{
		case 0:         return true;
		case 1:         return m_os_mode || (write == false);
		case 2:         return m_os_mode && (write == false);
		case 3:         return m_os_mode && (write == false);
		}
	}

	return false;
}

void acorn_memc_device::registers_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// is it a register?
	if ((data & 0x03e00000) != 0x03600000)
		return;

	LOG("%s: MEMC W %02x = %04x\n", machine().describe_context(), (data >> 17) & 7, data & 0xffff);

	switch ((data >> 17) & 7)
	{
	case 0: // Video init
		m_vidinit = ((data >> 2) & 0x7fff) * 16;
		break;

	case 1: // Video start
		m_vidstart = ((data >> 2) & 0x7fff) * 16;
		break;

	case 2: // Video end
		m_vidend = ((data >> 2) & 0x7fff) * 16;
		break;

	case 3: // Cursor init
		m_cursor_enabled = true;
		if (m_vidc.found())
			m_vidc->set_cursor_enable(m_cursor_enabled);

		m_cinit = ((data >> 2) & 0x7fff) * 16;
		break;

	case 4: // Sound start
		m_sirq_w(CLEAR_LINE);
		m_sndstart = ((data >> 2) & 0x7fff) * 16;
		break;

	case 5: // Sound end
		// end buffer is actually +16 bytes wrt sound start
		// TODO: it actually don't apply for ertictac and poizone?
		m_sndend = ((data >> 2) & 0x7fff) * 16;
		break;

	case 6: // Sound pointer
		m_sndcur = m_sndstart;
		m_sndendcur = m_sndend;
		m_sirq_w(ASSERT_LINE);
		break;

	case 7: // Control
		// --x- ---- ---- ---- Test Mode
		// ---x ---- ---- ---- OS Mode
		// ---- x--- ---- ---- Sound DMA
		// ---- -x-- ---- ---- Video DMA
		// ---- --xx ---- ---- DRAM refresh config
		// ---- ---- xx-- ---- High ROM access time
		// ---- ---- --xx ---- Low ROM access time
		// ---- ---- ---- xx-- Page size
		// ---- ---- ---- --xx Not used

		m_pagesize = BIT(data, 2, 2);
		m_video_dma_on = BIT(data, 10);
		m_sound_dma_on = BIT(data, 11);
		m_os_mode = BIT(data, 12);

		LOG("%s MEMC: %x to Control (page size %d, %s, %s)\n", machine().describe_context(), data & 0x1ffc, m_page_sizes[m_pagesize], m_video_dma_on ? "Video DMA on" : "Video DMA off", m_sound_dma_on ? "Sound DMA on" : "Sound DMA off");

		if (m_video_dma_on)
		{
			m_vidcur = 0;
			// TODO: update internally
		}
		else
		{
			m_cursor_enabled = false;
			if (m_vidc.found())
				m_vidc->set_cursor_enable(m_cursor_enabled);
		}

		if (m_vidc.found())
			m_vidc->update_sound_mode(m_sound_dma_on);

		if (m_sound_dma_on)
		{
			//logerror("MEMC: Starting audio DMA at %d uSec, buffer from %x to %x\n", ((m_regs[0xc0]&0xff)-2)*8, m_sndstart, m_sndend);
			//logerror("MEMC: audio DMA start, sound freq %d, sndhz = %f\n", (m_regs[0xc0] & 0xff)-2, sndhz);

			m_sndcur = m_sndstart;
			m_sndendcur = m_sndend;
		}
		break;
	default:
		logerror("MEMC: %06x to unknown reg %d\n", data & 0x1ffff, (data >> 17) & 7);
		break;
	}
}


//**************************************************************************
//
//              22 2222 1111 1111 1100 0000 0000
//              54 3210 9876 5432 1098 7654 3210
//    4k  page: 11 1LLL LLLL LLLL LLAA MPPP PPPP
//    8k  page: 11 1LLL LLLL LLLM LLAA MPPP PPPP
//    16k page: 11 1LLL LLLL LLxM LLAA MPPP PPPP
//    32k page: 11 1LLL LLLL LxxM LLAA MPPP PPPP
//           3   8    2   9    0    f    f
//
//    L - logical page
//    P - physical page
//    A - access permissions
//    M - MEMC number (for machines with multiple MEMCs)
//
//    The logical page is encoded with bits 11+10 being the most significant bits
//    (in that order), and the rest being bit 22 down.
//
//    The physical page is encoded differently depending on the page size :
//
//    4k  page:   bits 6-0 being bits 6-0
//    8k  page:   bits 6-1 being bits 5-0, bit 0 being bit 6
//    16k page:   bits 6-2 being bits 4-0, bits 1-0 being bits 6-5
//    32k page:   bits 6-3 being bits 4-0, bit 0 being bit 4, bit 2 being bit 5, bit 1 being bit 6
//
//**************************************************************************

void acorn_memc_device::page_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t logaddr = 0;
	uint32_t phyaddr = 0;
	uint32_t memc = 0;

	switch (m_pagesize)
	{
	case 0:
		phyaddr = BIT(data, 0, 7);
		logaddr = BIT(data, 12, 11) | (BIT(data, 10, 2) << 11);
		memc    = BIT(data, 7);
		break;

	case 1:
		phyaddr = BIT(data, 1, 6) | (BIT(data, 0) << 6);
		logaddr = BIT(data, 13, 10) | (BIT(data, 10, 2) << 10);
		memc    = BIT(data, 7) | (BIT(data, 12) << 1);
		break;

	case 2:
		phyaddr = BIT(data, 2, 5) | (BIT(data, 0, 2) << 5);
		logaddr = BIT(data, 14, 9) | (BIT(data, 10, 2) << 9);
		memc    = BIT(data, 7) | (BIT(data, 12) << 1);
		break;

	case 3:
		phyaddr = BIT(data, 3, 4) | (BIT(data, 0) << 4) | (BIT(data, 1) << 6) | (BIT(data, 2) << 5);
		logaddr = BIT(data, 15, 8) | (BIT(data, 10, 2) << 8);
		memc    = BIT(data, 7) | (BIT(data, 12) << 1);
		break;
	}

	// always make sure ROM mode is disconnected when this occurs
	m_latchrom = false;

	phyaddr += memc * 0x80;

	// unmap all logical pages that resolve to the same physical address
	for (int i=0; i < 0x2000; i++)
		if (m_pages[i] == phyaddr)
			m_pages[i] = -1;

	// now go ahead and set the mapping in the page table
	m_pages[logaddr] = phyaddr;
	m_pages_ppl[logaddr] = BIT(data, 8, 2);

	LOG("%s = MEMC_PAGE(%d): W %08x: logaddr %08x to phyaddr %08x, MEMC %d, perms %d\n", machine().describe_context(), m_pages[logaddr], data, logaddr * m_page_sizes[m_pagesize], phyaddr * m_page_sizes[m_pagesize], memc, m_pages_ppl[logaddr]);
}


// TODO: what type of DMA this is, burst or cycle steal? Docs doesn't explain it (4 usec is the DRAM refresh). */
// TODO: Erotictac and Poizone sets up vidinit register AFTER vidend, for double buffering? (fixes Poizone "Eterna" logo display on attract)
// TODO: understand how to make quazer to work (sets video DMA param in-flight)
void acorn_memc_device::do_video_dma()
{
	uint32_t size = (m_vidend - m_vidstart + 0x10) & 0x1fffff;
	uint32_t offset_ptr = m_vidinit;

	if (offset_ptr >= m_vidend + 0x10) // TODO: correct?
		offset_ptr = m_vidstart;

	//popmessage("%08x %08x %08x",m_vidstart, m_vidinit, m_vidend);

	if (m_vidc.found())
	{
		for (m_vidcur = 0; m_vidcur < size; m_vidcur++)
		{
			m_vidc->write_vram(m_vidcur, m_space->read_byte(dram_address((offset_ptr))));
			offset_ptr++;
			if (offset_ptr >= m_vidend + 0x10) // TODO: correct?
				offset_ptr = m_vidstart;
		}

		if (m_cursor_enabled)
		{
			uint16_t ccur_size = m_vidc->get_cursor_size() & 0x1ff;

			for (int ccur = 0; ccur < ccur_size; ccur++)
				m_vidc->write_cram(ccur, m_space->read_byte(dram_address((m_cinit + ccur))));
		}
	}
}

void acorn_memc_device::do_sound_dma()
{
	if (m_vidc.found())
	{
		for (int ch = 0; ch < 8; ch++)
			m_vidc->write_dac(ch, m_space->read_byte(dram_address(m_sndcur + ch)));
	}

	m_sndcur += 8;

	if (m_sndcur >= m_sndendcur)
	{
		m_sirq_w(ASSERT_LINE);

		// TODO: nuke this implementation detail, repeated below
		if (m_vidc.found())
			m_vidc->update_sound_mode(m_sound_dma_on);

		if (m_sound_dma_on)
		{
			//logerror("Chaining to next: start %x end %x\n", m_sndstart, m_sndend);
			m_sndcur = m_sndstart;
			m_sndendcur = m_sndend;
		}
		else if (m_vidc.found())
		{
			for (int ch=0; ch<8; ch++)
				m_vidc->clear_dac(ch);
		}
	}
}

WRITE_LINE_MEMBER(acorn_memc_device::spvmd_w)
{
	m_spvmd = state;
	m_abort_w(CLEAR_LINE);
}

WRITE_LINE_MEMBER(acorn_memc_device::sndrq_w)
{
	if (state && m_sound_dma_on)
		do_sound_dma();
}


WRITE_LINE_MEMBER(acorn_memc_device::vidrq_w)
{
	if (state && m_video_dma_on)
		do_video_dma();
}

uint32_t acorn_memc_device::dram_address(uint32_t address)
{
	if (m_output_dram_rowcol)
	{
		// The correct DRAM row / column for every page size is shown in Appendix A of the Acorn MEMC datasheet
		// xx-- ---- ---- ---- ---- ---- MEMC (for systems with multiple MEMC)
		// --xx xxxx xxxx ---- ---- ---- DRAM row
		// ---- ---- ---- xxxx xxxx xx-- DRAM column
		// ---- ---- ---- ---- ---- --xx CAS

		switch (m_pagesize)
		{
		// Page size                               MEMC      Unused          DRAM row         Unused           DRAM column            CAS    Mask unused
		case 0:    address = bitswap<24>(address, 23, 22,    21,20,        11,10,9,8,7,6,5,4,   19,      18,17,16,15,14,13,12,3,2,    1,0) & 0xcff7ff;  break;
		case 1:    address = bitswap<24>(address, 23, 22,    21,        12,11,10,9,8,7,6,5,4,   20,      18,17,16,15,14,13,19,3,2,    1,0) & 0xdff7ff;  break;
		case 2:    address = bitswap<24>(address, 23, 22,    21,        12,11,10,9,8,7,6,5,4,         20,18,17,16,15,14,13,19,3,2,    1,0) & 0xdfffff;  break;
		case 3:    address = bitswap<24>(address, 23, 22,            13,12,11,10,9,8,7,6,5,4,         20,18,17,16,15,14,21,19,3,2,    1,0) & 0xffffff;  break;
		}
	}

	return 0x02000000 | address;
}

uint32_t acorn_memc_device::logical_r(offs_t offset, uint32_t mem_mask)
{
	// are we mapping in the boot ROM?
	if (m_latchrom)
		return m_space->read_dword(0x3800000 | ((offset & 0x1fffff) << 2), mem_mask);

	// figure out the page number and offset in the page
	uint32_t pagesize = m_page_sizes[m_pagesize];
	uint32_t page = (offset << 2) / pagesize;
	uint32_t poffs = (offset << 2) % pagesize;

	if (is_valid_access(page, false))
		return m_space->read_dword(dram_address(m_pages[page] * pagesize + poffs), mem_mask);
	else
		return invalid_access(false, offset, 0, mem_mask);
}


void acorn_memc_device::logical_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// if the boot ROM is mapped, ignore writes
	if (m_latchrom)
		return;

	// figure out the page number and offset in the page
	uint32_t pagesize = m_page_sizes[m_pagesize];
	uint32_t page = (offset << 2) / pagesize;
	uint32_t poffs = (offset << 2) % pagesize;

	if (is_valid_access(page, true))
		m_space->write_dword(dram_address(m_pages[page] * pagesize + poffs), data, mem_mask);
	else
		invalid_access(true, offset, data, mem_mask);
}


uint32_t acorn_memc_device::high_mem_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t addr = offset << 2;
	if (!machine().side_effects_disabled())
		m_latchrom = false;

	if (!m_spvmd)
		return invalid_access(false, addr, 0, mem_mask);
	else if (addr < 0x1000000)   // DRAM
		return m_space->read_dword(dram_address(addr), mem_mask);
	else
		return m_space->read_dword(0x2000000 | addr, mem_mask);
}

void acorn_memc_device::high_mem_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t addr = offset << 2;
	m_latchrom = false;

	if (!m_spvmd)
		invalid_access(true, addr, data, mem_mask);
	else if (addr < 0x1000000)   // DRAM
		m_space->write_dword(dram_address(addr), data, mem_mask);
	else
		m_space->write_dword(0x2000000 | addr, data, mem_mask);
}
