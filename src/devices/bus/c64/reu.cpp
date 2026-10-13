// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1700/1750/1764 RAM Expansion Unit emulation

**********************************************************************/

#include "emu.h"
#include "reu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MOS8726R1_TAG   "u1"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_REU1700, c64_reu1700_cartridge_device, "c64_1700reu", "1700 RAM Expansion Unit")
DEFINE_DEVICE_TYPE(C64_REU1750, c64_reu1750_cartridge_device, "c64_1750reu", "1750 RAM Expansion Unit")
DEFINE_DEVICE_TYPE(C64_REU1764, c64_reu1764_cartridge_device, "c64_1764reu", "1764 RAM Expansion Unit")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_reu_cartridge_device::device_add_mconfig(machine_config &config)
{
	MOS8726(config, m_dmac, DERIVED_CLOCK(1, 1));
	m_dmac->irq_wr_callback().set(FUNC(c64_reu_cartridge_device::dmac_irq_w));
	m_dmac->dma_wr_callback().set(FUNC(c64_reu_cartridge_device::dmac_dma_w));
	m_dmac->c64_rd_callback().set(FUNC(c64_reu_cartridge_device::dmac_c64_r));
	m_dmac->c64_wr_callback().set(FUNC(c64_reu_cartridge_device::dmac_c64_w));
	m_dmac->reu_rd_callback().set(FUNC(c64_reu_cartridge_device::dmac_reu_r));
	m_dmac->reu_wr_callback().set(FUNC(c64_reu_cartridge_device::dmac_reu_w));

	GENERIC_SOCKET(config, m_eprom, generic_linear_slot, nullptr, "bin,rom");
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_reu_cartridge_device - constructor
//-------------------------------------------------

c64_reu_cartridge_device::c64_reu_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant, int jp1, size_t ram_size) :
	device_t(mconfig, type, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_dmac(*this, MOS8726R1_TAG),
	m_eprom(*this, "rom"),
	m_ram(*this, "ram", ram_size, ENDIANNESS_LITTLE),
	m_variant(variant),
	m_jp1(jp1),
	m_ram_size(ram_size),
	m_dd(0xff)
{
}

c64_reu1700_cartridge_device::c64_reu1700_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1700, tag, owner, clock, TYPE_1700, 0, 128 * 1024) { }

c64_reu1750_cartridge_device::c64_reu1750_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1750, tag, owner, clock, TYPE_1750, 1, 512 * 1024) { }

c64_reu1764_cartridge_device::c64_reu1764_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c64_reu_cartridge_device(mconfig, C64_REU1764, tag, owner, clock, TYPE_1764, 1, 256 * 1024) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_reu_cartridge_device::device_start()
{
	m_dmac->bs_w(m_jp1);

	save_item(NAME(m_dd));
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_reu_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_dmac->romsel_r(roml, romh))
	{
		data = m_eprom->read_rom(offset & 0x7fff);
	}
	else if (!io2 && !m_dmac->dma_active())
	{
		data = m_dmac->read(offset);
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_reu_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (m_dmac->dma_active())
		return;

	if (!io2)
	{
		m_dmac->write(offset, data);
	}
	else if (offset == 0xff00)
	{
		m_dmac->ff00_w();
	}
}


void c64_reu_cartridge_device::dmac_irq_w(int state)
{
	m_slot->irq_w(state);
}

void c64_reu_cartridge_device::dmac_dma_w(int state)
{
	m_slot->dma_w(state);
}

uint8_t c64_reu_cartridge_device::dmac_c64_r(offs_t offset)
{
	return m_slot->dma_cd_r(offset);
}

void c64_reu_cartridge_device::dmac_c64_w(offs_t offset, uint8_t data)
{
	m_slot->dma_cd_w(offset, data);
}

uint8_t c64_reu_cartridge_device::dmac_reu_r(offs_t offset)
{
	if (offset < m_ram_size)
		m_dd = m_ram[offset];

	return m_dd;
}

void c64_reu_cartridge_device::dmac_reu_w(offs_t offset, uint8_t data)
{
	m_dd = data;

	if (offset < m_ram_size)
		m_ram[offset] = data;
}
