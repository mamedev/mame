// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    TUG Combo Card / TUG EPROM Storage Card

    http://www.microtan.ukpc.net/pageProducts.html#ROM

    Usage:
    G EFFC

**********************************************************************/


#include "emu.h"
#include "tugcombo.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_TUGCOMBO2716, tanbus_tugcombo2716_device, "tanbus_tugcombo2716", "TUG Combo Card (2716)")
DEFINE_DEVICE_TYPE(TANBUS_TUGCOMBO2732, tanbus_tugcombo2732_device, "tanbus_tugcombo2732", "TUG Combo Card (2732)")
DEFINE_DEVICE_TYPE(TANBUS_TUGCOMBO6116, tanbus_tugcombo6116_device, "tanbus_tugcombo6116", "TUG Combo Card (6116)")
DEFINE_DEVICE_TYPE(TANBUS_TUGESC2716, tanbus_tugesc2716_device, "tanbus_tugesc2716", "TUG EPROM Storage Card (2716)")
DEFINE_DEVICE_TYPE(TANBUS_TUGESC2732, tanbus_tugesc2732_device, "tanbus_tugesc2732", "TUG EPROM Storage Card (2732)")


//-------------------------------------------------
//  INPUT_PORTS( tugcombo )
//-------------------------------------------------

INPUT_PORTS_START(tugcombo)
	PORT_START("LINKS")
	PORT_DIPNAME(0x88, 0x00, "I/O Address")
	PORT_DIPSETTING(0x00, "$BD00 - $BD07")
	PORT_DIPSETTING(0x08, "$BD08 - $BD0F")
	PORT_DIPSETTING(0x80, "$BD80 - $BD87")
	PORT_DIPSETTING(0x88, "$BD88 - $BD8F")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_tugcombo_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tugcombo);
}


void tanbus_tugcombo2716_device::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x8000).r(m_rom[0], FUNC(generic_slot_device::read_rom));
	map(0x0800, 0x0fff).mirror(0x8000).r(m_rom[1], FUNC(generic_slot_device::read_rom));
	map(0x1000, 0x17ff).mirror(0x8000).r(m_rom[2], FUNC(generic_slot_device::read_rom));
	map(0x1800, 0x1fff).mirror(0x8000).r(m_rom[3], FUNC(generic_slot_device::read_rom));
	map(0x2000, 0x27ff).mirror(0x8000).r(m_rom[4], FUNC(generic_slot_device::read_rom));
	map(0x2800, 0x2fff).mirror(0x8000).r(m_rom[5], FUNC(generic_slot_device::read_rom));
	map(0x3000, 0x37ff).mirror(0x8000).r(m_rom[6], FUNC(generic_slot_device::read_rom));
	map(0x3800, 0x3fff).mirror(0x8000).r(m_rom[7], FUNC(generic_slot_device::read_rom));
	map(0x4000, 0x47ff).mirror(0x8000).r(m_rom[8], FUNC(generic_slot_device::read_rom));
	map(0x4800, 0x4fff).mirror(0x8000).r(m_rom[9], FUNC(generic_slot_device::read_rom));
	map(0x5000, 0x57ff).mirror(0x8000).r(m_rom[10], FUNC(generic_slot_device::read_rom));
	map(0x5800, 0x5fff).mirror(0x8000).r(m_rom[11], FUNC(generic_slot_device::read_rom));
	map(0x6000, 0x67ff).mirror(0x8000).r(m_rom[12], FUNC(generic_slot_device::read_rom));
	map(0x6800, 0x6fff).mirror(0x8000).r(m_rom[13], FUNC(generic_slot_device::read_rom));
	map(0x7000, 0x77ff).mirror(0x8000).r(m_rom[14], FUNC(generic_slot_device::read_rom));
	map(0x7800, 0x7fff).mirror(0x8000).r(m_rom[15], FUNC(generic_slot_device::read_rom));
}

void tanbus_tugcombo2732_device::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).r(m_rom[0], FUNC(generic_slot_device::read_rom));
	map(0x1000, 0x1fff).r(m_rom[1], FUNC(generic_slot_device::read_rom));
	map(0x2000, 0x2fff).r(m_rom[2], FUNC(generic_slot_device::read_rom));
	map(0x3000, 0x3fff).r(m_rom[3], FUNC(generic_slot_device::read_rom));
	map(0x4000, 0x4fff).r(m_rom[4], FUNC(generic_slot_device::read_rom));
	map(0x5000, 0x5fff).r(m_rom[5], FUNC(generic_slot_device::read_rom));
	map(0x6000, 0x6fff).r(m_rom[6], FUNC(generic_slot_device::read_rom));
	map(0x7000, 0x7fff).r(m_rom[7], FUNC(generic_slot_device::read_rom));
	map(0x8000, 0x8fff).r(m_rom[8], FUNC(generic_slot_device::read_rom));
	map(0x9000, 0x9fff).r(m_rom[9], FUNC(generic_slot_device::read_rom));
	map(0xa000, 0xafff).r(m_rom[10], FUNC(generic_slot_device::read_rom));
	map(0xb000, 0xbfff).r(m_rom[11], FUNC(generic_slot_device::read_rom));
	map(0xc000, 0xcfff).r(m_rom[12], FUNC(generic_slot_device::read_rom));
	map(0xd000, 0xdfff).r(m_rom[13], FUNC(generic_slot_device::read_rom));
	map(0xe000, 0xefff).r(m_rom[14], FUNC(generic_slot_device::read_rom));
	map(0xf000, 0xffff).r(m_rom[15], FUNC(generic_slot_device::read_rom));
}

void tanbus_tugcombo6116_device::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).mirror(0x8000).ram().share("nvram");
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tanbus_tugcombo_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia[0]);
	m_pia[0]->writepa_handler().set([this](uint8_t data) { m_addr = (m_addr & 0xff00) | data; });
	m_pia[0]->writepb_handler().set([this](uint8_t data) { m_addr = (m_addr & 0x00ff) | (data << 8); });

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpb_handler().set([this]() { return m_space->read_byte(m_addr); });
	m_pia[1]->writepb_handler().set([this](uint8_t data) { m_space->write_byte(m_addr, data); });
}

void tanbus_tugcombo2716_device::device_add_mconfig(machine_config &config)
{
	tanbus_tugcombo_device::device_add_mconfig(config);

	for (int i = 0; i < 16; i++)
		GENERIC_SOCKET(config, m_rom[i], generic_linear_slot, "rom_esc2716", "bin,rom");
}

void tanbus_tugcombo2732_device::device_add_mconfig(machine_config &config)
{
	tanbus_tugcombo_device::device_add_mconfig(config);

	for (int i = 0; i < 16; i++)
		GENERIC_SOCKET(config, m_rom[i], generic_linear_slot, "rom_esc2732", "bin,rom");
}

void tanbus_tugcombo6116_device::device_add_mconfig(machine_config &config)
{
	tanbus_tugcombo_device::device_add_mconfig(config);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_tugcombo_device - constructor
//-------------------------------------------------

tanbus_tugcombo_device::tanbus_tugcombo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_tanbus_interface(mconfig, *this)
	, m_space(nullptr)
	, m_links(*this, "LINKS")
	, m_pia(*this, "pia%u", 0)
	, m_addr(0)
{
}


tanbus_tugcombo2716_device::tanbus_tugcombo2716_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo_device(mconfig, type, tag, owner, clock)
	, m_rom(*this, "rom%u", 0)
{
	m_space_config = address_space_config("memory", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(tanbus_tugcombo2716_device::mem_map), this));
}

tanbus_tugcombo2716_device::tanbus_tugcombo2716_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo2716_device(mconfig, TANBUS_TUGCOMBO2716, tag, owner, clock)
{
}

tanbus_tugesc2716_device::tanbus_tugesc2716_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo2716_device(mconfig, TANBUS_TUGESC2716, tag, owner, clock)
{
}


tanbus_tugcombo2732_device::tanbus_tugcombo2732_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo_device(mconfig, type, tag, owner, clock)
	, m_rom(*this, "rom%u", 0)
{
	m_space_config = address_space_config("memory", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(tanbus_tugcombo2732_device::mem_map), this));
}

tanbus_tugcombo2732_device::tanbus_tugcombo2732_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo2732_device(mconfig, TANBUS_TUGCOMBO2732, tag, owner, clock)
{
}

tanbus_tugesc2732_device::tanbus_tugesc2732_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo2732_device(mconfig, TANBUS_TUGESC2732, tag, owner, clock)
{
}


tanbus_tugcombo6116_device::tanbus_tugcombo6116_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo_device(mconfig, type, tag, owner, clock)
{
}

tanbus_tugcombo6116_device::tanbus_tugcombo6116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_tugcombo6116_device(mconfig, TANBUS_TUGCOMBO6116, tag, owner, clock)
{
	m_space_config = address_space_config("memory", ENDIANNESS_LITTLE, 8, 16, 0, address_map_constructor(FUNC(tanbus_tugcombo6116_device::mem_map), this));
}


device_memory_interface::space_config_vector tanbus_tugcombo_device::memory_space_config() const
{
	return space_config_vector{ std::make_pair(0, &m_space_config) };
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_tugcombo_device::device_start()
{
	m_space = &space(0);

	save_item(NAME(m_addr));
}


//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_tugcombo_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	offs_t addr = 0xbd00 | m_links->read();

	if (addr == (offset & 0xfff8))
	{
		switch (offset & 0x04)
		{
		case 0x00:
			data = m_pia[0]->read(offset & 3);
			break;
		case 0x04:
			data = m_pia[1]->read(offset & 3);
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_tugcombo_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	offs_t addr = 0xbd00 | m_links->read();

	if (addr == (offset & 0xfff8))
	{
		switch (offset & 0x04)
		{
		case 0x00:
			m_pia[0]->write(offset & 3, data);
			break;
		case 0x04:
			m_pia[1]->write(offset & 3, data);
			break;
		}
	}
}
