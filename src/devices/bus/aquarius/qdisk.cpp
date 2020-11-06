// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Radofin Aquarius Quick Disk

    Specifications:
      Capacity            64K byte per side
      Read/Write time     64K/8sec
      Transmitting speed  101K [BPS]
      Recording density   4410 [BPI]
      Track density       59 [TPI]
      Number of track     1 (spiral)
      Recoding system     MFM
      Rotation speed      423 [RPM]
      Disk                2.8"
      Number of heads     1

    TODO:

    - floppy support (I/O 0xe6-0xe7 = drive 1, 0xea-0xeb = drive 2)

**********************************************************************/


#include "emu.h"
#include "qdisk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AQUARIUS_QDISK, aquarius_qdisk_device, "aquarius_qdisk", "Aquarius Quick Disk")


//-------------------------------------------------
//  INPUT_PORTS( qdisk )
//-------------------------------------------------

static INPUT_PORTS_START(qdisk)
	PORT_START("SW1")
	PORT_DIPNAME(0x01, 0x00, "Drive Selection")
	PORT_DIPSETTING(0x00, "1")
	PORT_DIPSETTING(0x01, "2")
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor aquarius_qdisk_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(qdisk);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void aquarius_qdisk_device::device_add_mconfig(machine_config &config)
{
	MC6852(config, m_ssda, 6.5_MHz_XTAL);

	AQUARIUS_CARTRIDGE_SLOT(config, m_exp, DERIVED_CLOCK(1,1), aquarius_cartridge_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(aquarius_cartridge_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(aquarius_cartridge_slot_device::nmi_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  aquarius_qdisk_device - constructor
//-------------------------------------------------

aquarius_qdisk_device::aquarius_qdisk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, AQUARIUS_QDISK, tag, owner, clock)
	, device_aquarius_cartridge_interface(mconfig, *this)
	, m_ssda(*this, "mc6852")
	, m_exp(*this, "exp")
	, m_sw1(*this, "SW1")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void aquarius_qdisk_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t aquarius_qdisk_device::mreq_r(offs_t offset)
{
	return m_exp->mreq_r(offset);
}

void aquarius_qdisk_device::mreq_w(offs_t offset, uint8_t data)
{
	m_exp->mreq_w(offset, data);
}


uint8_t aquarius_qdisk_device::mreq_ce_r(offs_t offset)
{
	uint8_t data = get_rom_base()[offset & 0x1fff];

	data &= m_exp->mreq_ce_r(offset);

	return data;
}

void aquarius_qdisk_device::mreq_ce_w(offs_t offset, uint8_t data)
{
	m_exp->mreq_ce_w(offset, data);
}


uint8_t aquarius_qdisk_device::iorq_r(offs_t offset)
{
	uint8_t data = m_exp->iorq_r(offset);

	switch (offset & 0xfe)
	{
	case 0xe6: case 0xea:
		if (BIT(offset, 3) == m_sw1->read())
		{
			data = m_ssda->read(offset & 0x01);
		}
		logerror("iorq_r: %02x = %02x\n", offset, data);
		break;
	}

	data &= m_exp->iorq_r(offset);

	return data;
}

void aquarius_qdisk_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xfe)
	{
	case 0xe6: case 0xea:
		if (BIT(offset, 3) == m_sw1->read())
		{
			m_ssda->write(offset & 0x01, data);
		}
		logerror("iorq_w: %02x = %02x\n", offset, data);
		break;
	}

	m_exp->iorq_w(offset, data);
}
