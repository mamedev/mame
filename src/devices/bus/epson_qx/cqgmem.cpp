// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * commodity quote graphics 1 megabyte memory expansion
 *
 *******************************************************************/

#include "emu.h"
#include "cqgmem.h"

//**************************************************************************
//  CQGMEM DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_CQGMEM, bus::epson_qx::cqgmem_device, "epson_qx_option_cqgmem", "Commodity Quote Graphics 1MB Memory Expansion")

namespace bus::epson_qx {

static INPUT_PORTS_START( cqgmem )
	PORT_START("IOBASE")
	PORT_CONFNAME(0xf0, 0xf0, "IO Base Address Selection")
	PORT_CONFSETTING(0x80, "&80")
	PORT_CONFSETTING(0x90, "&90")
	PORT_CONFSETTING(0xa0, "&A0")
	PORT_CONFSETTING(0xb0, "&B0")
	PORT_CONFSETTING(0xc0, "&C0")
	PORT_CONFSETTING(0xd0, "&D0")
	PORT_CONFSETTING(0xe0, "&E0")
	PORT_CONFSETTING(0xf0, "&F0")
INPUT_PORTS_END

//-------------------------------------------------
//  cqgmem_device - constructor
//-------------------------------------------------
cqgmem_device::cqgmem_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_CQGMEM, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	device_memory_interface(mconfig, *this),
	m_ram(*this, "xmem", 0x100000, ENDIANNESS_LITTLE),
	m_banks(*this, "bank%u", 0U),
	m_iobase(*this, "IOBASE"),
	m_space_config("xmem", ENDIANNESS_LITTLE, 8, 20, 0, address_map_constructor(FUNC(cqgmem_device::xmem_map), this)),
	m_installed(false)
{
}

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------
ioport_constructor cqgmem_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cqgmem );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void cqgmem_device::device_start()
{
	m_installed = false;

	for (int i = 0; i < m_banks.size(); ++i) {
		m_banks[i]->configure_entries(0, 128, m_ram, 0x2000);
	}

	save_item(NAME(m_installed));
	save_item(NAME(m_banks_enabled));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void cqgmem_device::device_reset()
{
	if (!m_installed) {
		address_space &space = m_bus->iospace();
		offs_t iobase = m_iobase->read() & 0xf0;
		space.install_device(iobase, iobase+0x07, *this, &cqgmem_device::io_map);
		m_installed = true;
	}

	m_banks_enabled = 0; // FIXME: should probably unmap any enabled banks before clearing this on reset

	for (int i = 0; i < m_banks.size(); ++i) {
		m_banks[i]->set_entry(0);
	}
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------
device_memory_interface::space_config_vector cqgmem_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void cqgmem_device::xmem_map(address_map &map)
{
	map(0x00000, 0xfffff).ram().share("xmem");
}

void cqgmem_device::write(offs_t offset, uint8_t data)
{
	memory_view::memory_view_entry &view = m_bus->memview();

	uint8_t const bank = offset & 0x07;
	uint8_t const page = data & 0x7f;
	uint8_t const enable = (m_banks_enabled & ~(0x80 >> bank)) | ((data & 0x80) >> bank);
	uint16_t const bank_addr = bank * 0x2000;

	m_banks[bank]->set_entry(page);

	if (data & 0x80) {
		if (enable != m_banks_enabled) {
			view.unmap_readwrite(bank_addr, bank_addr + 0x1fff);
			view.install_readwrite_bank(bank_addr, bank_addr + 0x1fff, m_banks[bank]);
		}
	} else {
		if (enable != m_banks_enabled) {
			view.unmap_readwrite(bank_addr, bank_addr + 0x1fff);
		}
	}
	m_banks_enabled = enable;
}

void cqgmem_device::io_map(address_map &map)
{
	map(0x00, 0x06).w(FUNC(cqgmem_device::write));
}

} // namespace bus::epson_qx
