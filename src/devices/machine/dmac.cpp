// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    DMAC

    DMA controller used in Amiga systems

    Notes:
    - Part numbers: 390563-01 and 390563-02
    - Emulated is the old variant

    TODO:
    - SCSI
    - Support newer variant
    - DAWR
    - Data corruption when installing WB31
    - FIFO?

***************************************************************************/

#include "emu.h"
#include "dmac.h"

#define LOG_REGS (1 << 1)
#define LOG_INT  (1 << 2)
#define LOG_DMA  (1 << 3)

#define VERBOSE (LOG_GENERAL | LOG_REGS | LOG_INT)

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_DMAC_REV1, amiga_dmac_rev1_device, "amiga_dmac_rev1", "Amiga DMAC Rev. 1 DMA Controller")
DEFINE_DEVICE_TYPE(AMIGA_DMAC_REV2, amiga_dmac_rev2_device, "amiga_dmac_rev2", "Amiga DMAC Rev. 2 DMA Controller")

amiga_dmac_device::amiga_dmac_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool rev1) :
	device_t(mconfig, type, tag, owner, clock),
	amiga_autoconfig(),
	m_cfgout_cb(*this),
	m_int_cb(*this),
	m_css_read_cb(*this, 0),
	m_css_write_cb(*this),
	m_csx0_read_cb(*this, 0),
	m_csx0_write_cb(*this),
	m_csx1_read_cb(*this, 0),
	m_csx1_write_cb(*this),
	m_sdack_read_cb(*this, 0),
	m_sdack_write_cb(*this),
	m_xdack_read_cb(*this, 0),
	m_xdack_write_cb(*this),
	m_rom(*this, finder_base::DUMMY_TAG),
	m_space(nullptr),
	m_ram(nullptr),
	m_ram_size(-1),
	m_rev1(rev1),
	m_cntr(0),
	m_istr(0),
	m_wtc(0),
	m_acr(0),
	m_intx(false),
	m_sdreq(false),
	m_xdreq(false),
	m_autoconfig_dmac_done(false),
	m_autoconfig_ram_done(false),
	m_dmac_address(0),
	m_ram_address(0)
{
}

amiga_dmac_rev1_device::amiga_dmac_rev1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	amiga_dmac_device(mconfig, AMIGA_DMAC_REV1, tag, owner, clock, true)
{
}

amiga_dmac_rev2_device::amiga_dmac_rev2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	amiga_dmac_device(mconfig, AMIGA_DMAC_REV2, tag, owner, clock, false)
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void amiga_dmac_device::map(address_map &map)
{
	map(0x0000, 0x003f).rw(FUNC(amiga_dmac_device::autoconfig_r), FUNC(amiga_dmac_device::autoconfig_w));
	map(0x0040, 0x0041).r(FUNC(amiga_dmac_device::istr_r));
	map(0x0042, 0x0043).rw(FUNC(amiga_dmac_device::cntr_r), FUNC(amiga_dmac_device::cntr_w));
	map(0x0080, 0x0081).w(FUNC(amiga_dmac_device::wtc_hi_w)); // read?
	map(0x0082, 0x0083).w(FUNC(amiga_dmac_device::wtc_lo_w)); // read?
	map(0x0084, 0x0085).w(FUNC(amiga_dmac_device::acr_hi_w));
	map(0x0086, 0x0087).w(FUNC(amiga_dmac_device::acr_lo_w));
	map(0x008e, 0x008f).w(FUNC(amiga_dmac_device::dawr_w));
	map(0x0090, 0x0093).rw(FUNC(amiga_dmac_device::css_r), FUNC(amiga_dmac_device::css_w)).umask16(0x00ff);
	map(0x00a0, 0x00a7).rw(FUNC(amiga_dmac_device::csx0_r), FUNC(amiga_dmac_device::csx0_w)).umask16(0x00ff);
	map(0x00c0, 0x00c7).rw(FUNC(amiga_dmac_device::csx1_r), FUNC(amiga_dmac_device::csx1_w)).umask16(0x00ff);
	map(0x00e0, 0x00e1).rw(FUNC(amiga_dmac_device::st_dma_r), FUNC(amiga_dmac_device::st_dma_w));
	map(0x00e2, 0x00e3).rw(FUNC(amiga_dmac_device::sp_dma_r), FUNC(amiga_dmac_device::sp_dma_w));
	map(0x00e4, 0x00e5).rw(FUNC(amiga_dmac_device::cint_r), FUNC(amiga_dmac_device::cint_w));
	map(0x00e8, 0x00e9).rw(FUNC(amiga_dmac_device::flush_r), FUNC(amiga_dmac_device::flush_w));
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void amiga_dmac_device::device_start()
{
	m_dma_timer = timer_alloc(FUNC(amiga_dmac_device::update_dma), this);

	// register for save states
	save_item(NAME(m_ram_size));
	save_item(NAME(m_cntr));
	save_item(NAME(m_istr));
	save_item(NAME(m_wtc));
	save_item(NAME(m_acr));
	save_item(NAME(m_intx));
	save_item(NAME(m_sdreq));
	save_item(NAME(m_xdreq));
	save_item(NAME(m_autoconfig_dmac_done));
	save_item(NAME(m_autoconfig_ram_done));
	save_item(NAME(m_dmac_address));
	save_item(NAME(m_ram_address));
}

void amiga_dmac_device::update_interrupts()
{
	// check external interrupt line
	if (m_intx)
		m_istr |= ISTR_INTS;
	else
		m_istr &= ~ISTR_INTS;

	if (m_istr & ISTR_INT_MASK)
	{
		LOGMASKED(LOG_INT, "Found active interrupt: %02x\n", m_istr);

		m_istr |= ISTR_INT_F;
		m_istr |= ISTR_INT_P;
	}
	else
	{
		LOGMASKED(LOG_INT, "No active interrupt\n");

		m_istr &= ~ISTR_INT_F;
		m_istr &= ~ISTR_INT_P;
	}

	// clear pending if interrupts disabled
	if (!(m_cntr & CNTR_INTEN))
	{
		LOGMASKED(LOG_INT, "Interrupts are disabled\n");

		m_istr &= ~ISTR_INT_P;
	}

	// finally update interrupt line
	m_int_cb((m_istr & ISTR_INT_P) ? 1 : 0);
}

TIMER_CALLBACK_MEMBER(amiga_dmac_device::update_dma)
{
	if (m_sdreq && (m_cntr & CNTR_PDMD))
	{
		LOGMASKED(LOG_DMA, "scsi dma, acr=%08x, wtc=%08x\n", m_acr, m_wtc);
	}
	else if (m_xdreq && !(m_cntr & CNTR_PDMD))
	{
		LOGMASKED(LOG_DMA, "xt dma, acr=%08x, wtc=%08x\n", m_acr, m_wtc);

		if (m_cntr & CNTR_DDIR)
		{
			// host -> peripheral
			uint8_t data = m_space->read_byte(m_acr);
			LOGMASKED(LOG_DMA, "Write to device: %02x\n", data);
			m_xdack_write_cb(data);
		}
		else
		{
			// peripheral -> host
			uint8_t data = m_xdack_read_cb();
			LOGMASKED(LOG_DMA, "Read from device: %02x\n", data);
			m_space->write_byte(m_acr, data);
		}

		m_acr++;

		if (m_rev1 && (m_cntr & CNTR_TCEN))
		{
			// we count words
			if ((m_acr & 1) == 0)
			{
				if (--m_wtc == 0)
				{
					LOGMASKED(LOG_DMA, "Terminal count\n");

					m_istr |= ISTR_E_INT;
					update_interrupts();
				}
			}
		}
	}
}

void amiga_dmac_device::stop_dma()
{
	m_dma_timer->adjust(attotime::never);

	m_istr &= ~ISTR_E_INT;
	update_interrupts();
}

void amiga_dmac_device::start_dma()
{
	m_dma_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));
}

uint16_t amiga_dmac_device::autoconfig_r(offs_t offset, uint16_t mem_mask)
{
	return autoconfig_read(*m_space, offset, mem_mask);
}

void amiga_dmac_device::autoconfig_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	autoconfig_write(*m_space, data, mem_mask);
}

uint16_t amiga_dmac_device::istr_r(offs_t offset, uint16_t mem_mask)
{
	return m_istr;
}

uint16_t amiga_dmac_device::cntr_r(offs_t offset, uint16_t mem_mask)
{
	return m_cntr;
}

void amiga_dmac_device::cntr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS, "cntr_w: %04x & %04x\n", data, mem_mask);

	COMBINE_DATA(&m_cntr);
	update_interrupts();
}

void amiga_dmac_device::wtc_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS, "wtc_hi_w: %04x & %04x\n", data, mem_mask);

	if (m_rev1)
	{
		m_wtc &= (~(uint32_t) mem_mask) << 16 | 0x0000ffff;
		m_wtc |= ((uint32_t) data & mem_mask) << 16;
	}
}

void amiga_dmac_device::wtc_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS, "wtc_lo_w: %04x & %04x\n", data, mem_mask);

	if (m_rev1)
	{
		m_wtc &= 0xffff0000 & (~mem_mask);
		m_wtc |= data & mem_mask;
	}
}

void amiga_dmac_device::acr_hi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS, "acr_hi_w: %04x & %04x\n", data, mem_mask);

	m_acr &= (~(uint32_t) mem_mask) << 16 | 0x0000ffff;
	m_acr |= ((uint32_t) data & mem_mask) << 16;
}

void amiga_dmac_device::acr_lo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS, "acr_lo_w: %04x & %04x\n", data, mem_mask);

	m_acr &= 0xffff0000 & (~mem_mask);
	m_acr |= (data & mem_mask) & (m_rev1 ? 0xfffc : 0xfffe);
}

void amiga_dmac_device::dawr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS, "dawr_w: %04x & %04x\n", data, mem_mask);
}

// chip select lines
uint8_t amiga_dmac_device::css_r(offs_t offset) { return m_css_read_cb(offset); }
void amiga_dmac_device::css_w(offs_t offset, uint8_t data) { m_css_write_cb(offset, data); }
uint8_t amiga_dmac_device::csx0_r(offs_t offset) { return m_csx0_read_cb(offset); }
void amiga_dmac_device::csx0_w(offs_t offset, uint8_t data) { m_csx0_write_cb(offset, data); }
uint8_t amiga_dmac_device::csx1_r(offs_t offset) { return m_csx1_read_cb(offset); }
void amiga_dmac_device::csx1_w(offs_t offset, uint8_t data) { m_csx1_write_cb(offset, data); }

void amiga_dmac_device::st_dma()
{
	LOGMASKED(LOG_DMA, "st_dma\n");
	start_dma();
}

void amiga_dmac_device::sp_dma()
{
	LOGMASKED(LOG_DMA, "sp_dma\n");
	stop_dma();
}

void amiga_dmac_device::cint()
{
	LOGMASKED(LOG_DMA, "cint\n");
	m_istr = 0;
	update_interrupts();
}

void amiga_dmac_device::flush()
{
	LOGMASKED(LOG_DMA, "flush\n");

	if (!m_rev1)
	{
		m_istr &= ~ISTR_FF_FLG;
		m_istr |= ISTR_FE_FLG;
	}
}

void amiga_dmac_device::ramsz_w(int state)
{
	LOG("ramsz_w: %d\n", state);

	switch (state)
	{
		case 0: m_ram_size = 0x000000; break;
		case 1: m_ram_size = 0x080000; break;
		case 2: m_ram_size = 0x100000; break;
		case 3: m_ram_size = 0x200000; break;
	}
}

void amiga_dmac_device::rst_w(int state)
{
	if (state == 0)
	{
		if (m_autoconfig_dmac_done)
		{
			LOG("unmapping dmac, base = %06x\n", m_dmac_address);
			m_space->unmap_readwrite(m_dmac_address, m_dmac_address + 0xffff);

			m_autoconfig_dmac_done = false;
		}

		if (m_autoconfig_ram_done)
		{
			LOG("unmapping ram, base = %06x, size = %06x\n", m_ram_address, m_ram_size);
			m_space->unmap_readwrite(m_ram_address, m_ram_address + m_ram_size - 1);

			m_ram_size = 0;
			m_autoconfig_ram_done = false;
		}

		m_istr = 0;
		m_istr |= ISTR_FE_FLG; // fifo empty
		m_cntr = 0;
	}
}

void amiga_dmac_device::intx_w(int state)
{
	LOGMASKED(LOG_INT, "intx_w: %d\n", state);

	m_intx = bool(state);
	update_interrupts();
}

void amiga_dmac_device::sdreq_w(int state)
{
	LOGMASKED(LOG_DMA, "sdreq_w: %d\n", state);

	m_sdreq = bool(state);
}

void amiga_dmac_device::xdreq_w(int state)
{
	LOGMASKED(LOG_DMA, "xdreq_w: %d\n", state);

	m_xdreq = bool(state);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void amiga_dmac_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);

	if (!m_autoconfig_ram_done && m_ram_size > 0)
	{
		LOG("-> installing dmac memory, size = %06x\n", m_ram_size);

		m_space->install_ram(address, address + m_ram_size - 1, m_ram);

		m_ram_address = address;
		m_autoconfig_ram_done = true;

		// configure next
		configin_w(0);
	}
	else
	{
		LOG("-> installing dmac registers\n");

		m_space->install_device(address, address + 0x7fff, *this, &amiga_dmac_device::map);

		if (m_rom)
		{
			m_space->install_rom(address + 0x2000, address + 0x7fff, m_rom->base() + 0x2000);
			m_space->install_rom(address + 0x8000, address + 0xffff, m_rom->base());
		}

		m_space->unmap_readwrite(0xe80000, 0xe8007f);

		m_dmac_address = address;
		m_autoconfig_dmac_done = true;

		// we're done
		m_cfgout_cb(0);
	}
}

void amiga_dmac_device::configin_w(int state)
{
	if (state != 0)
		return;

	if (!m_autoconfig_ram_done && m_ram_size > 0)
	{
		LOG("autoconfig for memory\n");

		// setup autoconfig for memory
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		switch (m_ram_size)
		{
			case 0x080000: autoconfig_board_size(BOARD_SIZE_512K); break;
			case 0x100000: autoconfig_board_size(BOARD_SIZE_1M); break;
			case 0x200000: autoconfig_board_size(BOARD_SIZE_2M); break;
		}
		autoconfig_link_into_memory(true);
		autoconfig_rom_vector_valid(false);
		autoconfig_multi_device(true);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true);
		autoconfig_product(10);
		autoconfig_manufacturer(514);
		autoconfig_serial(0x00000000);
	}
	else
	{
		LOG("autoconfig for registers\n");

		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector(0x2000);
		autoconfig_rom_vector_valid(true);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true);
		autoconfig_product(m_rev1 ? 2 : 3);
		autoconfig_manufacturer(514);
		autoconfig_serial(0x00000000);
	}

	// install autoconfig handler
	m_space->install_readwrite_handler(0xe80000, 0xe8007f,
		read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
		write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
}
