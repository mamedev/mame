// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Ralph Allen 32K EPROM-RAM Card

    http://www.microtan.ukpc.net/6809/32RAM.pdf

**********************************************************************/


#include "emu.h"
#include "ra32k.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(TANBUS_RA32KRAM, tanbus_ra32kram_device, "tanbus_ra32kram", "Ralph Allen 32K EPROM-RAM Card (RAM)")
DEFINE_DEVICE_TYPE(TANBUS_RA32KROM, tanbus_ra32krom_device, "tanbus_ra32krom", "Ralph Allen 32K EPROM-RAM Card (RALBUG)")


//-------------------------------------------------
//  INPUT_PORTS( ra32k )
//-------------------------------------------------

INPUT_PORTS_START( ra32k )
	PORT_START("DSW1")
	PORT_DIPNAME(0x01, 0x01, "2K block $0800") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x01, "Enable")
	PORT_DIPNAME(0x02, 0x02, "2K block $1800") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x02, "Enable")
	PORT_DIPNAME(0x04, 0x04, "2K block $2800") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x04, "Enable")
	PORT_DIPNAME(0x08, 0x08, "2K block $3800") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x08, "Enable")
	PORT_DIPNAME(0x10, 0x10, "2K block $4800") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x10, "Enable")
	PORT_DIPNAME(0x20, 0x20, "2K block $5800") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x20, "Enable")
	PORT_DIPNAME(0x40, 0x40, "2K block $6800") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x40, "Enable")
	PORT_DIPNAME(0x80, 0x80, "2K block $7800") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x80, "Enable")

	PORT_START("DSW2")
	PORT_DIPNAME(0x01, 0x01, "2K block $0000") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x01, "Enable")
	PORT_DIPNAME(0x02, 0x02, "2K block $1000") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x02, "Enable")
	PORT_DIPNAME(0x04, 0x04, "2K block $2000") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x04, "Enable")
	PORT_DIPNAME(0x08, 0x08, "2K block $3000") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x08, "Enable")
	PORT_DIPNAME(0x10, 0x10, "2K block $4000") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x10, "Enable")
	PORT_DIPNAME(0x20, 0x20, "2K block $5000") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x20, "Enable")
	PORT_DIPNAME(0x40, 0x40, "2K block $6000") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x40, "Enable")
	PORT_DIPNAME(0x80, 0x80, "2K block $7000") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x00, "Disable")
	PORT_DIPSETTING(0x80, "Enable")

	PORT_START("DSW3")
	PORT_DIPNAME(0x0f, 0x0f, "Address Selection") PORT_DIPLOCATION("SW3:1,2,3,4")
	PORT_DIPSETTING(0x0f, "$0000-$7FFF")
	PORT_DIPSETTING(0x00, "$1000-$8FFF")
	PORT_DIPSETTING(0x04, "$2000-$9FFF")
	PORT_DIPSETTING(0x08, "$3000-$AFFF")
	PORT_DIPSETTING(0x0c, "$4000-$BFFF")
	PORT_DIPSETTING(0x01, "$5000-$CFFF")
	PORT_DIPSETTING(0x05, "$6000-$DFFF")
	PORT_DIPSETTING(0x09, "$7000-$EFFF")
	PORT_DIPSETTING(0x0d, "$8000-$FFFF")
	PORT_DIPSETTING(0x02, "$9000-$0FFF")
	PORT_DIPSETTING(0x06, "$A000-$1FFF")
	PORT_DIPSETTING(0x0a, "$B000-$2FFF")
	PORT_DIPSETTING(0x0e, "$C000-$3FFF")
	PORT_DIPSETTING(0x03, "$D000-$4FFF")
	PORT_DIPSETTING(0x07, "$E000-$5FFF")
	PORT_DIPSETTING(0x0b, "$F000-$6FFF")

	PORT_START("LNK1")
	PORT_CONFNAME(0x01, 0x00, "Block Enable")
	PORT_CONFSETTING(0x00, "Permanent")
	PORT_CONFSETTING(0x01, "Page selectable")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_ra32k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ra32k );
}

//-------------------------------------------------
//  ROM( ra32krom )
//-------------------------------------------------

ROM_START(ra32krom)
	ROM_REGION(0x8000, "rom", 0)
	ROM_LOAD("data_v1.4.rom",   0x2000, 0x0800, CRC(de61f11d) SHA1(3e6ddc5dba47d9136f58abc9475c3e73fc8cc0c2))
	ROM_LOAD("syntax_v1.4.rom", 0x2800, 0x0800, CRC(17438c4c) SHA1(46bf4e33544c19e142b380095736c5c3eb885ba0))
	ROM_LOAD("ascii_v1.4.rom",  0x3000, 0x0800, CRC(93244a81) SHA1(6149eec9904438a0d23529d2c2428a594d5b1fb9))
	ROM_LOAD("graph_v1.4.rom",  0x3800, 0x0800, CRC(159eb1f6) SHA1(5e49edda4b550506aca70d1f7e7de7a352f0359f))
ROM_END

const tiny_rom_entry *tanbus_ra32krom_device::device_rom_region() const
{
	return ROM_NAME(ra32krom);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_ra32k_device - constructor
//-------------------------------------------------

tanbus_ra32k_device::tanbus_ra32k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_dsw(*this, "DSW%u", 1)
	, m_link(*this, "LNK1")
{
}

tanbus_ra32kram_device::tanbus_ra32kram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_ra32k_device(mconfig, TANBUS_RA32KRAM, tag, owner, clock)
{
}

tanbus_ra32krom_device::tanbus_ra32krom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tanbus_ra32k_device(mconfig, TANBUS_RA32KROM, tag, owner, clock)
	, m_rom(*this, "rom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_ra32kram_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x8000);

	save_pointer(NAME(m_ram), 0x8000);
}

void tanbus_ra32krom_device::device_start()
{
}


bool tanbus_ra32k_device::block_enabled(offs_t offset, int inhrom, int inhram, int be)
{
	offs_t addr_start = ((bitswap<4>(m_dsw[2]->read(), 1, 0, 3, 2) << 12) + 0x1000) & 0xffff;
	offs_t addr_end = (addr_start + 0x7fff) & 0xffff;

	uint8_t block = (offset & 0x7800) >> 12;

	if (offset < addr_start || offset > addr_end)
		return false;

	if (offset & 0x0800)
	{
		/* switch SW1 */
		if (!BIT(m_dsw[0]->read(), block))
			return false;
	}
	else
	{
		/* switch SW2 */
		if (!BIT(m_dsw[1]->read(), block))
			return false;
	}

	/* recognise INHRAM signal */
	if (inhram)
		return false;

	/* recognise I/O signal (should be a Tanex output) */
	if ((offset & 0xfc00) == 0xbc00)
		return false;

	/* recognise Block Enable */
	if ((m_link->read() & 0x01) && !be)
		return false;

	return true;
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_ra32kram_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	if (block_enabled(offset, inhrom, inhram, be))
	{
		return m_ram[offset & 0x7fff];
	}
	return 0xff;
}

uint8_t tanbus_ra32krom_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	if (block_enabled(offset, inhrom, inhram, be))
	{
		return m_rom[offset & 0x7fff];
	}
	return 0xff;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_ra32kram_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	if (block_enabled(offset, inhrom, inhram, be))
	{
		m_ram[offset & 0x7fff] = data;
	}
}
