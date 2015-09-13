// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    DMAC

    DMA controller used in Amiga systems

BOARDS:
 CBM A590/A2091 HD controller: Prod=514/3($202/$3) (@$e90000 64K)
 CBM A2052/58. RAM I 590/2091.RAM Prod=514/10($202/$a) (@$200000 2meg mem)

***************************************************************************/

#include "dmac.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type DMAC = &device_creator<dmac_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dmac_device - constructor
//-------------------------------------------------

dmac_device::dmac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, DMAC, "DMAC DMA Controller", tag, owner, clock, "dmac", __FILE__),
	amiga_autoconfig(),
	m_cfgout_handler(*this),
	m_int_handler(*this),
	m_xdack_handler(*this),
	m_scsi_read_handler(*this),
	m_scsi_write_handler(*this),
	m_io_read_handler(*this),
	m_io_write_handler(*this),
	m_space(NULL),
	m_rom(NULL),
	m_ram(NULL),
	m_ram_size(-1),
	m_configured(false),
	m_rst(-1),
	m_cntr(0),
	m_istr(0),
	m_wtc(0),
	m_acr(0),
	m_dma_active(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dmac_device::device_start()
{
	// resolve callbacks
	m_cfgout_handler.resolve_safe();
	m_int_handler.resolve_safe();
	m_xdack_handler.resolve_safe();
	m_scsi_read_handler.resolve_safe(0);
	m_scsi_write_handler.resolve_safe();
	m_io_read_handler.resolve_safe(0);
	m_io_write_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dmac_device::device_reset()
{
	// fifo empty
	m_istr |= ISTR_FE_FLG;
}

void dmac_device::autoconfig_base_address(offs_t address)
{
	if (VERBOSE)
		logerror("%s('%s'): autoconfig_base_address received: 0x%06x\n", shortname(), basetag(), address);

	if (!m_configured && m_ram_size > 0)
	{
		if (VERBOSE)
			logerror("-> installing ram (%d bytes)\n", m_ram_size);

		// install access to the ram space
		if (address)
			m_space->install_ram(address, address + (m_ram_size - 1), m_ram);

		// prepare autoconfig for main device
		autoconfig_board_size(BOARD_SIZE_64K);
		autoconfig_product(0x03); // or 0x02 for rev 1
		autoconfig_rom_vector(0x2000);
		autoconfig_rom_vector_valid(true);
		autoconfig_link_into_memory(false);
		autoconfig_multi_device(false);

		// first device configured
		m_configured = true;
	}
	else
	{
		if (VERBOSE)
			logerror("-> installing dmac\n");

		// internal dmac registers
		m_space->install_readwrite_handler(address, address + 0xff,
			read16_delegate(FUNC(dmac_device::register_read), this),
			write16_delegate(FUNC(dmac_device::register_write), this), 0xffff);

		// install access to the rom space
		if (m_rom)
		{
			m_space->install_rom(address + 0x2000, address + 0x7fff, m_rom + 0x2000);
			m_space->install_rom(address + 0x8000, address + 0xffff, m_rom);
		}

		// stop responding to autoconfig
		m_space->unmap_readwrite(0xe80000, 0xe8007f);

		// we're done
		m_cfgout_handler(0);
	}
}

void dmac_device::check_interrupts()
{
	// interrupts enabled?
	if (m_cntr & CNTR_INTEN)
	{
		// any interrupts pending?
		if (m_istr & ISTR_INT_MASK)
			m_istr |= ISTR_INT_P;
	}
	else
		m_istr &= ~ISTR_INT_P;

	// finally update interrupt line
	m_int_handler((m_istr & ISTR_INT_P) ? 1 : 0);
}

void dmac_device::stop_dma()
{
	m_dma_active = false;
	m_istr &= ~ISTR_E_INT;
	check_interrupts();
}

void dmac_device::start_dma()
{
	m_dma_active = true;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ16_MEMBER( dmac_device::register_read )
{
	UINT16 data = 0xffff;

	// autoconfig handles this
	if (offset < 0x20)
		return autoconfig_read(space, offset, mem_mask);

	switch (offset)
	{
	case 0x20:
		data = m_istr;

		// reading clears fifo status (?)
		m_istr &= ~0x0f;
		check_interrupts();

		if (VERBOSE)
			logerror("%s('%s'): read istr %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		break;

	case 0x21:
		data = m_cntr;

		if (VERBOSE)
			logerror("%s('%s'): read cntr %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		break;

	case 0x48:
	case 0x49:
	case 0x50:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5e:
	case 0x5f:
		data = m_scsi_read_handler(offset);

		if (VERBOSE)
			logerror("%s('%s'): read scsi data @ %02x %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

		break;

	case 0x70:
		if (VERBOSE)
			logerror("%s('%s'): read dma start strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		start_dma();
		break;

	case 0x71:
		if (VERBOSE)
			logerror("%s('%s'): read dma stop strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		stop_dma();
		break;

	case 0x72:
		if (VERBOSE)
			logerror("%s('%s'): read clear irq strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		// clear all interrupts
		m_istr &= ~ISTR_INT_MASK;
		check_interrupts();
		break;

	case 0x74:
		if (VERBOSE)
			logerror("%s('%s'): read flush fifo strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		m_istr |= ISTR_FE_FLG;
		break;

	default:
		if (VERBOSE)
			logerror("%s('%s'): register_read %04x @ %02x [mask = %04x]\n", shortname(), basetag(), data, offset, mem_mask);
	}

	return data;
}

WRITE16_MEMBER( dmac_device::register_write )
{
	switch (offset)
	{
	case 0x21:
		if (VERBOSE)
			logerror("%s('%s'): write cntr %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		m_cntr = data;
		check_interrupts();
		break;

	case 0x40:
		if (VERBOSE)
			logerror("%s('%s'): write wtc hi %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		m_wtc &= 0x0000ffff;
		m_wtc |= ((UINT32) data) << 16;
		break;

	case 0x41:
		if (VERBOSE)
			logerror("%s('%s'): write wtc lo %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		m_wtc &= 0xffff0000;
		m_wtc |= data;
		break;

	case 0x42:
		if (VERBOSE)
			logerror("%s('%s'): write acr hi %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		m_acr &= 0x0000ffff;
		m_acr |= ((UINT32) data) << 16;
		break;

	case 0x43:
		if (VERBOSE)
			logerror("%s('%s'): write acr lo %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		m_acr &= 0xffff0000;
		m_acr |= data;
		break;

	case 0x47:
		if (VERBOSE)
			logerror("%s('%s'): write dawr %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);
		break;

	case 0x48:
	case 0x49:
	case 0x50:
	case 0x58:
	case 0x59:
	case 0x5a:
	case 0x5b:
	case 0x5c:
	case 0x5e:
	case 0x5f:
		if (VERBOSE)
			logerror("%s('%s'): write scsi data @ %02x %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

		m_scsi_write_handler(offset, data, 0xff);
		break;

	case 0x70:
		if (VERBOSE)
			logerror("%s('%s'): write dma start strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		start_dma();
		break;

	case 0x71:
		if (VERBOSE)
			logerror("%s('%s'): write dma stop strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		stop_dma();
		break;

	case 0x72:
		if (VERBOSE)
			logerror("%s('%s'): write clear irq strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		// clear all interrupts
		m_istr &= ~ISTR_INT_MASK;
		check_interrupts();
		break;

	case 0x74:
		if (VERBOSE)
			logerror("%s('%s'): write flush fifo strobe %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

		m_istr |= ISTR_FE_FLG;
		break;

	default:
		if (VERBOSE)
			logerror("%s('%s'): write %04x @ %02x [mask = %04x]\n", shortname(), basetag(), data, offset, mem_mask);
	}
}

// this signal tells us to expose our autoconfig values
WRITE_LINE_MEMBER( dmac_device::configin_w )
{
	if (VERBOSE)
		logerror("%s('%s'): configin_w (%d)\n", shortname(), basetag(), state);

	if (state == 0 && !m_configured)
	{
		// common autoconfig values
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_manufacturer(0x0202);
		autoconfig_serial(0x00000000);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true);

		// if we have ram, configure it first
		if (m_ram_size > 0)
		{
			// product id 10
			autoconfig_product(0x0a);

			// board size
			switch (m_ram_size)
			{
			case 0x080000: autoconfig_board_size(BOARD_SIZE_512K); break;
			case 0x100000: autoconfig_board_size(BOARD_SIZE_1M); break;
			case 0x200000: autoconfig_board_size(BOARD_SIZE_2M); break;
			}

			// no rom and link into free memory
			autoconfig_rom_vector_valid(false);
			autoconfig_link_into_memory(true);

			// the main device follows
			autoconfig_multi_device(true);
		}
		else
		{
			// just setup autoconfig for the main device
			autoconfig_board_size(BOARD_SIZE_64K);
			autoconfig_product(0x03); // or 0x02 for rev 1
			autoconfig_rom_vector(0x2000);
			autoconfig_rom_vector_valid(true);
			autoconfig_link_into_memory(false);

			// no more devices after this
			autoconfig_multi_device(false);
		}

		// install autoconfig handler
		m_space->install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(FUNC(amiga_autoconfig::autoconfig_read), static_cast<amiga_autoconfig *>(this)),
			write16_delegate(FUNC(amiga_autoconfig::autoconfig_write), static_cast<amiga_autoconfig *>(this)), 0xffff);
	}
}

// this sets the ram size depending on the line voltage
WRITE_LINE_MEMBER( dmac_device::ramsz_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ramsz_w (%d)\n", shortname(), basetag(), state);

	switch (state)
	{
	case 0: m_ram_size = 0x000000; break;
	case 1: m_ram_size = 0x080000; break;
	case 2: m_ram_size = 0x100000; break;
	case 3: m_ram_size = 0x200000; break;
	}
}

// reset the device
WRITE_LINE_MEMBER( dmac_device::rst_w )
{
	if (VERBOSE)
		logerror("%s('%s'): rst_w (%d)\n", shortname(), basetag(), state);

	if (m_rst == 1 && state == 0)
		device_reset();

	m_rst = state;
}

// external interrupt
WRITE_LINE_MEMBER( dmac_device::intx_w )
{
	if (VERBOSE)
		logerror("%s('%s'): intx_w (%d)\n", shortname(), basetag(), state);

	if (state)
		m_istr |= ISTR_INTS;
	else
		m_istr &= ~ISTR_INTS;

	check_interrupts();
}

// data request
WRITE_LINE_MEMBER( dmac_device::xdreq_w )
{
	if (VERBOSE)
		logerror("%s('%s'): xdreq_w (%d)\n", shortname(), basetag(), state);

	if (m_dma_active)
	{
	}
}
