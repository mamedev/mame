// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2mockingboard.c

    Implementation of the Sweet Micro Systems Mockingboard card
    and friends.

*********************************************************************/

#include "emu.h"
#include "a2mockingboard.h"
#include "speaker.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define VIA1_TAG "mockbd_via1"
#define VIA2_TAG "mockbd_via2"
#define AY1_TAG "mockbd_ay1"
#define AY2_TAG "mockbd_ay2"
#define AY3_TAG "mockbd_ay3"
#define AY4_TAG "mockbd_ay4"
#define E2P_TMS_TAG "tms5220"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_MOCKINGBOARD, a2bus_mockingboard_device, "a2mockbd", "Sweet Micro Systems Mockingboard")
DEFINE_DEVICE_TYPE(A2BUS_PHASOR,       a2bus_phasor_device,       "a2phasor", "Applied Engineering Phasor")
DEFINE_DEVICE_TYPE(A2BUS_ECHOPLUS,     a2bus_echoplus_device,     "a2echop",  "Street Electronics Echo Plus")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ayboard_device::add_common_devices(machine_config &config)
{
	VIA6522(config, m_via1, 1022727);
	m_via1->writepa_handler().set(FUNC(a2bus_ayboard_device::via1_out_a));
	m_via1->writepb_handler().set(FUNC(a2bus_ayboard_device::via1_out_b));
	m_via1->irq_handler().set(FUNC(a2bus_ayboard_device::via1_irq_w));

	VIA6522(config, m_via2, 1022727);
	m_via2->writepa_handler().set(FUNC(a2bus_ayboard_device::via2_out_a));
	m_via2->writepb_handler().set(FUNC(a2bus_ayboard_device::via2_out_b));
	m_via2->irq_handler().set(FUNC(a2bus_ayboard_device::via2_irq_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	AY8913(config, m_ay1, 1022727);
	m_ay1->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
}

void a2bus_ayboard_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	AY8913(config, m_ay2, 1022727);
	m_ay2->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
}

void a2bus_phasor_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	m_via1->writepb_handler().set(FUNC(a2bus_phasor_device::via1_out_b));
	m_via2->writepb_handler().set(FUNC(a2bus_phasor_device::via2_out_b));

	SPEAKER(config, "lspeaker2").front_left();
	SPEAKER(config, "rspeaker2").front_right();
	AY8913(config, m_ay2, 1022727);
	AY8913(config, m_ay3, 1022727);
	AY8913(config, m_ay4, 1022727);
	m_ay2->add_route(ALL_OUTPUTS, "lspeaker2", 1.0);
	m_ay3->add_route(ALL_OUTPUTS, "rspeaker", 1.0);
	m_ay4->add_route(ALL_OUTPUTS, "rspeaker2", 1.0);
}

void a2bus_echoplus_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	AY8913(config, m_ay2, 1022727);
	m_ay2->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	SPEAKER(config, "echosp").front_center();
	TMS5220(config, m_tms, 640000);
	// echo+ has a TSP5220C soldered down on it
	m_tms->add_route(ALL_OUTPUTS, "echosp", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_ayboard_device::a2bus_ayboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_via1(*this, VIA1_TAG),
	m_via2(*this, VIA2_TAG),
	m_ay1(*this, AY1_TAG),
	m_ay2(*this, AY2_TAG),
	m_porta1(0),
	m_porta2(0)
{
}

a2bus_mockingboard_device::a2bus_mockingboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ayboard_device(mconfig, A2BUS_MOCKINGBOARD, tag, owner, clock)
{
}

a2bus_phasor_device::a2bus_phasor_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ayboard_device(mconfig, A2BUS_PHASOR, tag, owner, clock),
	m_ay3(*this, AY3_TAG),
	m_ay4(*this, AY4_TAG),
	m_native(false)
{
}

a2bus_echoplus_device::a2bus_echoplus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ayboard_device(mconfig, A2BUS_ECHOPLUS, tag, owner, clock),
	m_tms(*this, E2P_TMS_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_ayboard_device::device_start()
{
	save_item(NAME(m_porta1));
	save_item(NAME(m_porta2));
}

void a2bus_ayboard_device::device_reset()
{
	m_porta1 = m_porta2 = 0;
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_ayboard_device::read_cnxx(uint8_t offset)
{
	if (offset <= 0x10)
		return m_via1->read(offset & 0xf);
	else if (offset >= 0x80 && offset <= 0x90)
		return m_via2->read(offset & 0xf);

	return 0;
}

uint8_t a2bus_phasor_device::read_cnxx(uint8_t offset)
{
	uint8_t ret = 0;
	int via_sel;

	if (m_native)
		via_sel = ((offset & 0x80) >> 6) | ((offset & 0x10) >> 4);
	else
		via_sel = (offset & 0x80) ? 2 : 1;

	if ((offset <= 0x20) || (offset >= 0x80 && offset <= 0xa0))
	{
		if (BIT(via_sel, 0))
			ret |= m_via1->read(offset & 0xf);

		if (BIT(via_sel, 1))
			ret |= m_via2->read(offset & 0xf);
	}

	return ret;
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_ayboard_device::write_cnxx(uint8_t offset, uint8_t data)
{
	if (offset <= 0x10)
		m_via1->write(offset & 0xf, data);
	else if (offset >= 0x80 && offset <= 0x90)
		m_via2->write(offset & 0xf, data);
	else
		logerror("Mockingboard(%d): unk write %02x to Cn%02X (%s)\n", slotno(), data, offset, machine().describe_context());
}

void a2bus_phasor_device::write_cnxx(uint8_t offset, uint8_t data)
{
	if ((offset <= 0x20) || (offset >= 0x80 && offset <= 0xa0))
	{
		int via_sel;

		if (m_native)
			via_sel = ((offset & 0x80) >> 6) | ((offset & 0x10) >> 4);
		else
			via_sel = (offset & 0x80) ? 2 : 1;

		if (BIT(via_sel, 0))
			m_via1->write(offset & 0xf, data);

		if (BIT(via_sel, 1))
			m_via2->write(offset & 0xf, data);
	}
}

WRITE_LINE_MEMBER( a2bus_ayboard_device::via1_irq_w )
{
	if (state)
		raise_slot_irq();
	else
		lower_slot_irq();
}

WRITE_LINE_MEMBER( a2bus_ayboard_device::via2_irq_w )
{
	if (state)
		raise_slot_irq();
	else
		lower_slot_irq();
}

WRITE8_MEMBER( a2bus_ayboard_device::via1_out_a )
{
	m_porta1 = data;
}

WRITE8_MEMBER( a2bus_ayboard_device::via1_out_b )
{
	if (!BIT(data, 2))
	{
		m_ay1->reset_w();
	}
	else
	{
		switch (data & 3)
		{
			case 0: // BDIR=0, BC1=0 (inactive)
				break;

			case 1: // BDIR=0, BC1=1 (read PSG)
				m_porta1 = m_ay1->data_r();
				break;

			case 2: // BDIR=1, BC1=0 (write PSG)
				m_ay1->data_w(m_porta1);
				break;

			case 3: // BDIR=1, BC1=1 (latch)
				m_ay1->address_w(m_porta1);
				break;
		}
	}
}

WRITE8_MEMBER( a2bus_phasor_device::via1_out_b )
{
	if (!(data & 4))
	{
		m_ay1->reset_w();
		if (m_native)
		{
			m_ay2->reset_w();
		}
	}
	else
	{
		int chip_sel;

		if (m_native)
			chip_sel = (~(data >> 3) & 3);
		else
			chip_sel = 1;

//      logerror("Phasor: %02x to AY1/2 CS %02x (BDIR/BC1 %02x, data %02x)\n", m_porta1, chipSel, data & 3, data);
		switch (data & 3)
		{
			case 0: // BDIR=0, BC1=0 (inactive)
				break;

			case 1: // BDIR=0, BC1=1 (read PSG)
				if (BIT(chip_sel, 0))
					m_porta1 = m_ay1->data_r();
				if (BIT(chip_sel, 1))
					m_porta1 = m_ay2->data_r();
				break;

			case 2: // BDIR=1, BC1=0 (write PSG)
				if (BIT(chip_sel, 0))
					m_ay1->data_w(m_porta1);
				if (BIT(chip_sel, 1))
					m_ay2->data_w(m_porta1);
				break;

			case 3: // BDIR=1, BC1=1 (latch)
				if (BIT(chip_sel, 0))
					m_ay1->address_w(m_porta1);
				if (BIT(chip_sel, 1))
					m_ay2->address_w(m_porta1);
				break;
		}
	}
}

WRITE8_MEMBER( a2bus_ayboard_device::via2_out_a )
{
	m_porta2 = data;
}

WRITE8_MEMBER( a2bus_ayboard_device::via2_out_b )
{
	if (!BIT(data, 2))
	{
		m_ay2->reset_w();
	}
	else
	{
		switch (data & 3)
		{
			case 0: // BDIR=0, BC1=0 (inactive)
				break;

			case 1: // BDIR=0, BC1=1 (read PSG)
				m_porta2 = m_ay2->data_r();
				break;

			case 2: // BDIR=1, BC1=0 (write PSG)
				m_ay2->data_w(m_porta2);
				break;

			case 3: // BDIR=1, BC1=1 (latch)
				m_ay2->data_w(m_porta2);
				break;
		}
	}
}

WRITE8_MEMBER( a2bus_phasor_device::via2_out_b )
{
	if (!BIT(data, 2))
	{
		if (m_native)
		{
			m_ay3->reset_w();
			m_ay4->reset_w();
		}
		else
		{
			m_ay2->reset_w();
		}
	}
	else
	{
		int chip_sel;

		if (m_native)
			chip_sel = (~(data >> 3) & 3);
		else
			chip_sel = 1;

//      logerror("Phasor: %02x to AY3/4 CS %02x (BDIR/BC1 %02x, data %02x)\n", m_porta2, chipSel, data & 3, data);
		switch (data & 3)
		{
			case 0: // BDIR=0, BC1=0 (inactive)
				break;

			case 1: // BDIR=0, BC1=1 (read PSG)
				if (BIT(chip_sel, 0))
					m_porta2 = m_ay3->data_r();
				if (BIT(chip_sel, 1))
					m_porta2 = m_ay4->data_r();
				break;

			case 2: // BDIR=1, BC1=0 (write PSG)
				if (BIT(chip_sel, 0))
					m_ay3->data_w(m_porta2);
				if (BIT(chip_sel, 1))
					m_ay4->data_w(m_porta2);
				break;

			case 3: // BDIR=1, BC1=1 (latch)
				if (BIT(chip_sel, 0))
					m_ay3->address_w(m_porta2);
				if (BIT(chip_sel, 1))
					m_ay4->address_w(m_porta2);
				break;
		}
	}
}


uint8_t a2bus_phasor_device::read_c0nx(uint8_t offset)
{
	m_native = BIT(offset, 0);
	return 0xff;
}

void a2bus_phasor_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_native = BIT(offset, 0);
}

uint8_t a2bus_echoplus_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return 0x1f | m_tms->status_r();
	}

	return 0;
}

void a2bus_echoplus_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_tms->data_w(data);
			break;
	}
}
