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

DEFINE_DEVICE_TYPE(TANBUS_RA32K, tanbus_ra32k_device, "tanbus_ra32k", "Ralph Allen 32K EPROM-RAM Card")


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
	PORT_CONFNAME(0x03, 0x00, "Block Enable")
	PORT_CONFSETTING(0x00, "RAM (permanent)")
	PORT_CONFSETTING(0x01, "RAM (page selectable)")
	PORT_CONFSETTING(0x02, "EPROM (page selectable)")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor tanbus_ra32k_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ra32k );
}

//-------------------------------------------------
//  ROM( ra32k )
//-------------------------------------------------

ROM_START(ra32k)
	ROM_REGION(0x8000, "rom", 0)
	ROM_LOAD("data_v1.4.rom", 0x0000, 0x0800, CRC(de61f11d) SHA1(3e6ddc5dba47d9136f58abc9475c3e73fc8cc0c2))
	ROM_LOAD("syntax_v1.4.rom", 0x0800, 0x0800, CRC(17438c4c) SHA1(46bf4e33544c19e142b380095736c5c3eb885ba0))
	ROM_LOAD("ascii_v1.4.rom", 0x1000, 0x0800, CRC(93244a81) SHA1(6149eec9904438a0d23529d2c2428a594d5b1fb9))
	ROM_LOAD("graph_v1.4.rom", 0x1800, 0x0800, CRC(159eb1f6) SHA1(5e49edda4b550506aca70d1f7e7de7a352f0359f))
ROM_END

const tiny_rom_entry *tanbus_ra32k_device::device_rom_region() const
{
	return ROM_NAME(ra32k);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tanbus_ra32k_device - constructor
//-------------------------------------------------

tanbus_ra32k_device::tanbus_ra32k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TANBUS_RA32K, tag, owner, clock)
	, device_tanbus_interface(mconfig, *this)
	, m_rom(*this, "rom")
	, m_dsw(*this, "DSW%u", 1)
	, m_link(*this, "LNK1")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tanbus_ra32k_device::device_start()
{
	m_ram = std::make_unique<uint8_t[]>(0x8000);

	save_pointer(NAME(m_ram), 0x8000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tanbus_ra32k_device::device_reset()
{
}

//-------------------------------------------------
//  read - card read
//-------------------------------------------------

uint8_t tanbus_ra32k_device::read(offs_t offset, int inhrom, int inhram, int be)
{
	uint8_t data = 0xff;

	switch (m_link->read() & 0x02)
	{
	case 0x00:
		/* 32K dynamic RAM */
		if (block_enabled(offset, inhrom, inhram, be))
		{
			//logerror("ram read %04x\n", offset);
			data = m_ram[offset & 0x7fff];
		}
		break;
	case 0x02:
		/* ROM selected */
		if (block_enabled(offset, inhrom, inhram, be))
		{
			//logerror("rom read %04x\n", offset);
			data = m_rom->base()[(offset - m_addr_start) & 0x7fff];
		}
		break;
	}

	return data;
}

//-------------------------------------------------
//  write - card write
//-------------------------------------------------

void tanbus_ra32k_device::write(offs_t offset, uint8_t data, int inhrom, int inhram, int be)
{
	switch (m_link->read() & 0x02)
	{
	case 0x00:
		/* 32K dynamic RAM */
		if (block_enabled(offset, inhrom, inhram, be))
		{
			//logerror("ram write %04x %02x\n", offset, data);
			m_ram[offset & 0x7fff] = data;
		}
		break;
	case 0x02:
		/* ROM selected */
		break;
	}
}

bool tanbus_ra32k_device::block_enabled(offs_t offset, int inhrom, int inhram, int be)
{
	m_addr_start = ((bitswap<4>(m_dsw[2]->read(), 1, 0, 3, 2) << 12) + 0x1000) & 0xffff;
	m_addr_end = (m_addr_start + 0x7fff) & 0xffff;

	//uint8_t block_start = (bitswap<4>(m_dsw[2]->read(), 1, 0, 3, 2) + 1) & 0x0f;
	uint8_t block = offset >> 12;
	//logerror("%04x start %04x current %04x\n", offset, block_start, block);

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
