// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2alfsm2.c

    Implementation of the ALF Apple Music II card (originally marketed as the "MC1")
    The AE Super Music Synthesizer is a superset of this card (4x76489 instead of 3)

*********************************************************************/

#include "emu.h"
#include "a2alfam2.h"
#include "sound/sn76496.h"
#include "speaker.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_ALFAM2, a2bus_alfam2_device, "a2alfam2", "ALF MC1 / Apple Music II")
DEFINE_DEVICE_TYPE(A2BUS_AESMS,  a2bus_aesms_device,  "a2aesms",  "Applied Engineering Super Music Synthesizer")

#define SN1_TAG         "sn76489_1" // left
#define SN2_TAG         "sn76489_2" // center
#define SN3_TAG         "sn76489_3" // right
#define SN4_TAG         "sn76489_4" // center?


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_sn76489_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "alf_l").front_left();
	SPEAKER(config, "alf_r").front_right();

	SN76489(config, m_sn1, 1020484);
	m_sn1->add_route(ALL_OUTPUTS, "alf_l", 0.50);
	SN76489(config, m_sn2, 1020484);
	m_sn2->add_route(ALL_OUTPUTS, "alf_l", 0.50);
	m_sn2->add_route(ALL_OUTPUTS, "alf_r", 0.50);
	SN76489(config, m_sn3, 1020484);
	m_sn3->add_route(ALL_OUTPUTS, "alf_r", 0.50);
}

void a2bus_aesms_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "alf_l").front_left();
	SPEAKER(config, "alf_r").front_right();

	SN76489(config, m_sn1, 1020484);
	m_sn1->add_route(ALL_OUTPUTS, "alf_l", 0.50);

	SN76489(config, m_sn2, 1020484);
	m_sn2->add_route(ALL_OUTPUTS, "alf_l", 0.50);
	m_sn2->add_route(ALL_OUTPUTS, "alf_r", 0.50);

	SN76489(config, m_sn3, 1020484);
	m_sn3->add_route(ALL_OUTPUTS, "alf_r", 0.50);

	SN76489(config, m_sn4, 1020484);
	m_sn4->add_route(ALL_OUTPUTS, "alf_l", 0.50);
	m_sn4->add_route(ALL_OUTPUTS, "alf_r", 0.50);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_sn76489_device::a2bus_sn76489_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_sn1(*this, SN1_TAG),
	m_sn2(*this, SN2_TAG),
	m_sn3(*this, SN3_TAG),
	m_sn4(*this, SN4_TAG), m_latch0(0), m_latch1(0), m_latch2(0), m_latch3(0), m_has4thsn(false)
{
}

a2bus_alfam2_device::a2bus_alfam2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_sn76489_device(mconfig, A2BUS_ALFAM2, tag, owner, clock)
{
	m_has4thsn = false;
}

a2bus_aesms_device::a2bus_aesms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_sn76489_device(mconfig, A2BUS_AESMS, tag, owner, clock)
{
	m_has4thsn = true;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_sn76489_device::device_start()
{
	m_latch0 = m_latch1 = m_latch2 = m_latch3 = 0;

	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
	save_item(NAME(m_latch2));
	save_item(NAME(m_latch3));
}

void a2bus_sn76489_device::device_reset()
{
	m_latch0 = m_latch1 = m_latch2 = m_latch3 = 0;
}

uint8_t a2bus_sn76489_device::read_c0nx(uint8_t offset)
{
	// SN76489 can't be read, it appears from the schematics this is what happens
	switch (offset)
	{
		case 0:
			return m_latch0;

		case 1:
			return m_latch1;

		case 2:
			return m_latch2;

		case 3:
			return m_latch3;
	}

	return 0xff;
}

void a2bus_sn76489_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_sn1->write(data);
			m_latch0 = data;
			break;

		case 1:
			m_sn2->write(data);
			m_latch1 = data;
			break;

		case 2:
			m_sn3->write(data);
			m_latch2 = data;
			break;

		case 3:
			if (m_has4thsn)
			{
				m_sn4->write(data);
				m_latch3 = data;
			}
			break;
	}
}

bool a2bus_sn76489_device::take_c800()
{
	return false;
}
