// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Mertec Compact Companion

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Mertec_CompactComp.html

    TODO:
    - Fix userport, by somehow passing lines into joyport
    - Not sure whether any 1MHz bus devices should work on the 2MHz bus
    - No idea how the 6821 is connected/used

**********************************************************************/


#include "emu.h"
#include "mertec.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_MERTEC, bbc_mertec_device, "bbc_mertec", "Mertec Compact Companion");


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(mertec)
	ROM_REGION(0x8000, "ext_rom", 0)
	ROM_LOAD("mertec-companion-v0.99.rom", 0x0000, 0x8000, CRC(af8ff8d7) SHA1(0c4017ffbb480168e54c6b153da257ec5ea29d4e))
ROM_END

const tiny_rom_entry *bbc_mertec_device::device_rom_region() const
{
	return ROM_NAME(mertec);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_mertec_device::device_add_mconfig(machine_config &config)
{
	PIA6821(config, m_pia, DERIVED_CLOCK(1, 8));
	//m_pia->readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	//m_pia->writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	//m_pia->irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	/* adc */
	UPD7002(config, m_upd7002, DERIVED_CLOCK(1, 8));
	m_upd7002->set_get_analogue_callback(FUNC(bbc_mertec_device::get_analogue_input), this);
	m_upd7002->set_eoc_callback(FUNC(bbc_mertec_device::upd7002_eoc), this);

	/* analogue port */
	BBC_ANALOGUE_SLOT(config, m_analog, bbc_analogue_devices, nullptr);

	/* user port */
	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	//m_userport->cb1_handler().set(m_pia, FUNC(via6522_device::write_cb1));
	//m_userport->cb2_handler().set(m_pia, FUNC(via6522_device::write_cb2));

	/* 2mhz bus port */
	BBC_1MHZBUS_SLOT(config, m_2mhzbus, DERIVED_CLOCK(1, 4), bbc_1mhzbus_devices, nullptr);
	m_2mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::irq_w));
	m_2mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(bbc_exp_slot_device::nmi_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_mertec_device - constructor
//-------------------------------------------------

bbc_mertec_device::bbc_mertec_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_MERTEC, tag, owner, clock)
	, device_bbc_exp_interface(mconfig, *this)
	, m_pia(*this, "pia")
	, m_upd7002(*this, "upd7002")
	, m_analog(*this, "analogue")
	, m_userport(*this, "userport")
	, m_2mhzbus(*this, "2mhzbus")
	, m_ext_rom(*this, "ext_rom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_mertec_device::device_start()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

int bbc_mertec_device::get_analogue_input(int channel_number)
{
	return ((0xff - m_analog->ch_r(channel_number)) << 8);
}

void bbc_mertec_device::upd7002_eoc(int data)
{
	//m_via6522_0->write_cb1(data);
}

READ8_MEMBER(bbc_mertec_device::fred_r)
{
	return m_2mhzbus->fred_r(space, offset);
}

WRITE8_MEMBER(bbc_mertec_device::fred_w)
{
	m_2mhzbus->fred_w(space, offset, data);
}

READ8_MEMBER(bbc_mertec_device::jim_r)
{
	return m_2mhzbus->jim_r(space, offset);
}

WRITE8_MEMBER(bbc_mertec_device::jim_w)
{
	m_2mhzbus->jim_w(space, offset, data);
}

READ8_MEMBER(bbc_mertec_device::sheila_r)
{
	uint8_t data = 0xfe;

	if (offset >= 0x18 && offset < 0x20)
	{
		data = m_upd7002->read(space, offset & 0x03);
	}

	return data;
}

WRITE8_MEMBER(bbc_mertec_device::sheila_w)
{
	if (offset >= 0x18 && offset < 0x20)
	{
		m_upd7002->write(space, offset & 0x03, data);
	}
}

READ8_MEMBER(bbc_mertec_device::pb_r)
{
	return m_userport->pb_r(space, 0);
}

WRITE8_MEMBER(bbc_mertec_device::pb_w)
{
	m_userport->pb_w(space, 0, data);
}
