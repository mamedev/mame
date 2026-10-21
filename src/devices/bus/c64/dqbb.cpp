// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Brown Boxes Double Quick Brown Box emulation

**********************************************************************/

#include "emu.h"
#include "dqbb.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_DQBB, c64_dqbb_cartridge_device, "c64_dqbb", "C64 Double Quick Brown Box 16K cartridge")
DEFINE_DEVICE_TYPE(C64_DQBB32, c64_dqbb32_cartridge_device, "c64_dqbb32", "C64 Double Quick Brown Box 32K cartridge")
DEFINE_DEVICE_TYPE(C64_DQBB64, c64_dqbb64_cartridge_device, "c64_dqbb64", "C64 Double Quick Brown Box 64K cartridge")
DEFINE_DEVICE_TYPE(C64_DQBB128, c64_dqbb128_cartridge_device, "c64_dqbb128", "C64 Double Quick Brown Box 128K cartridge")
DEFINE_DEVICE_TYPE(C64_DQBB256, c64_dqbb256_cartridge_device, "c64_dqbb256", "C64 Double Quick Brown Box 256K cartridge")


//-------------------------------------------------
//  INPUT_PORTS( c64_dqbb )
//-------------------------------------------------

static INPUT_PORTS_START( c64_dqbb )
	PORT_START("MODE")
	PORT_CONFNAME( 0x01, 0x00, "Mode" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(c64_dqbb_cartridge_device::mode_changed), 0)
	PORT_CONFSETTING(    0x00, "C64" )
	PORT_CONFSETTING(    0x01, "C128" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_dqbb_cartridge_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c64_dqbb );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_dqbb_cartridge_device - constructor
//-------------------------------------------------

c64_dqbb_cartridge_device::c64_dqbb_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t ram_size) :
	device_t(mconfig, type, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_mode(*this, "MODE"),
	m_ram_size(ram_size),
	m_cs(0),
	m_we(0),
	m_exrom_bit(false),
	m_bank(0)
{
}

c64_dqbb_cartridge_device::c64_dqbb_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_dqbb_cartridge_device(mconfig, C64_DQBB, tag, owner, clock, 0x4000)
{
}

c64_dqbb32_cartridge_device::c64_dqbb32_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_dqbb_cartridge_device(mconfig, C64_DQBB32, tag, owner, clock, 0x8000)
{
}

c64_dqbb64_cartridge_device::c64_dqbb64_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_dqbb_cartridge_device(mconfig, C64_DQBB64, tag, owner, clock, 0x10000)
{
}

c64_dqbb128_cartridge_device::c64_dqbb128_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_dqbb_cartridge_device(mconfig, C64_DQBB128, tag, owner, clock, 0x20000)
{
}

c64_dqbb256_cartridge_device::c64_dqbb256_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c64_dqbb_cartridge_device(mconfig, C64_DQBB256, tag, owner, clock, 0x40000)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_dqbb_cartridge_device::device_start()
{
	// allocate memory
	m_nvram = std::make_unique<uint8_t[]>(m_ram_size);
	save_pointer(NAME(m_nvram), m_ram_size);

	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_we));
	save_item(NAME(m_exrom_bit));
	save_item(NAME(m_bank));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_dqbb_cartridge_device::device_reset()
{
	m_game = 1;
	m_cs = 0;
	m_we = 0;
	m_exrom_bit = false;
	m_bank = 0;
	update_exrom();
}


//-------------------------------------------------
//  INPUT_CHANGED_MEMBER( mode_changed )
//-------------------------------------------------

INPUT_CHANGED_MEMBER( c64_dqbb_cartridge_device::mode_changed )
{
	update_exrom();
}


void c64_dqbb_cartridge_device::update_exrom()
{
	m_exrom = BIT(m_mode->read(), 0) && !m_exrom_bit;
}


void c64_dqbb_cartridge_device::nvram_default()
{
}


bool c64_dqbb_cartridge_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_nvram.get(), m_ram_size);
	return !err && (actual == m_ram_size);
}


bool c64_dqbb_cartridge_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_nvram.get(), m_ram_size);
	return !err;
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_dqbb_cartridge_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_cs && (!roml || !romh))
	{
		data = m_nvram[(m_bank << 14) | (offset & 0x3fff)];
	}

	return data;
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_dqbb_cartridge_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	if (!m_cs && m_we && (offset >= 0x8000 && offset < 0xc000))
	{
		m_nvram[(m_bank << 14) | (offset & 0x3fff)] = data;
	}
	else if (!io1)
	{
		/*

		    bit     description

		    0       BANK0
		    1       BANK1
		    2       GAME
		    3       BANK2
		    4       WE
		    5       BANK3
		    6       EXROM
		    7       _CS

		*/

		m_exrom_bit = BIT(data, 6);
		m_game = !BIT(data, 2);
		m_we = BIT(data, 4);
		m_cs = BIT(data, 7);
		m_bank = bitswap<4>(data, 5, 3, 1, 0) & ((m_ram_size >> 14) - 1);
		update_exrom();
	}
}
