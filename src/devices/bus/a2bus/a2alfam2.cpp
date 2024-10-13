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

#include <algorithm>


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_alfam2_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	a2bus_alfam2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_alfam2_device(mconfig, A2BUS_ALFAM2, tag, owner, clock)
	{
	}

protected:
	// construction/destruction
	a2bus_alfam2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override { return false; }

	required_device<sn76489_device> m_sn1;
	required_device<sn76489_device> m_sn2;
	required_device<sn76489_device> m_sn3;
	optional_device<sn76489_device> m_sn4;

private:
	uint8_t m_latch[4];
};

class a2bus_aesms_device : public a2bus_alfam2_device
{
public:
	a2bus_aesms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
		a2bus_alfam2_device(mconfig, A2BUS_AESMS, tag, owner, clock)
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

/***************************************************************************
    PARAMETERS
***************************************************************************/

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

void a2bus_alfam2_device::device_add_mconfig(machine_config &config)
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

a2bus_alfam2_device::a2bus_alfam2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_sn1(*this, SN1_TAG),
	m_sn2(*this, SN2_TAG),
	m_sn3(*this, SN3_TAG),
	m_sn4(*this, SN4_TAG),
	m_latch{ 0, 0, 0, 0 }
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_alfam2_device::device_start()
{
	save_item(NAME(m_latch));
}

void a2bus_alfam2_device::device_reset()
{
	std::fill(std::begin(m_latch), std::end(m_latch), 0);
}

uint8_t a2bus_alfam2_device::read_c0nx(uint8_t offset)
{
	// SN76489 can't be read, it appears from the schematics this is what happens
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			return m_latch[offset];
	}

	return 0xff;
}

void a2bus_alfam2_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_sn1->write(data);
			m_latch[0] = data;
			break;

		case 1:
			m_sn2->write(data);
			m_latch[1] = data;
			break;

		case 2:
			m_sn3->write(data);
			m_latch[2] = data;
			break;

		case 3:
			if (m_sn4)
			{
				m_sn4->write(data);
				m_latch[3] = data;
			}
			break;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ALFAM2, device_a2bus_card_interface, a2bus_alfam2_device, "a2alfam2", "ALF MC1 / Apple Music II")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_AESMS,  device_a2bus_card_interface, a2bus_aesms_device,  "a2aesms",  "Applied Engineering Super Music Synthesizer")
