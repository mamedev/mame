// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1656 bus expander emulation

*********************************************************************/

#include "emu.h"
#include "abc1656.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ABC1656, abc1656_device, "luxor_abc1656", "Luxor ABC 1656")


static DEVICE_INPUT_DEFAULTS_START( db4105 )
	DEVICE_INPUT_DEFAULTS( "5E", 0x3f, 0x26 )
DEVICE_INPUT_DEFAULTS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void abc1656_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_LOW(config, "pren").output_handler().set(FUNC(abc1656_device::pren_w));
	INPUT_MERGER_ANY_LOW(config, "trrq").output_handler().set(FUNC(abc1656_device::trrq_w));

	ABCBUS_SLOT(config, m_bus[0], DERIVED_CLOCK(1, 1), abc1600bus_cards, "4105");
	m_bus[0]->set_option_device_input_defaults("4105", DEVICE_INPUT_DEFAULTS_NAME(db4105));
	m_bus[0]->irq_callback().set(FUNC(abc1656_device::irq_w));
	m_bus[0]->pren_callback().set("pren", FUNC(input_merger_device::in_w<0>));
	m_bus[0]->trrq_callback().set("trrq", FUNC(input_merger_device::in_w<0>));

	ABCBUS_SLOT(config, m_bus[1], DERIVED_CLOCK(1, 1), abc1600bus_cards, nullptr);
	m_bus[1]->irq_callback().set(FUNC(abc1656_device::xint2_w));
	m_bus[1]->pren_callback().set("pren", FUNC(input_merger_device::in_w<1>));
	m_bus[1]->trrq_callback().set("trrq", FUNC(input_merger_device::in_w<1>));

	ABCBUS_SLOT(config, m_bus[2], DERIVED_CLOCK(1, 1), abc1600bus_cards, nullptr);
	m_bus[2]->irq_callback().set(FUNC(abc1656_device::xint3_w));
	m_bus[2]->pren_callback().set("pren", FUNC(input_merger_device::in_w<2>));
	m_bus[2]->trrq_callback().set("trrq", FUNC(input_merger_device::in_w<2>));

	ABCBUS_SLOT(config, m_bus[3], DERIVED_CLOCK(1, 1), abc1600bus_cards, nullptr);
	m_bus[3]->irq_callback().set(FUNC(abc1656_device::xint4_w));
	m_bus[3]->pren_callback().set("pren", FUNC(input_merger_device::in_w<3>));
	m_bus[3]->trrq_callback().set("trrq", FUNC(input_merger_device::in_w<3>));

	ABCBUS_SLOT(config, m_bus[4], DERIVED_CLOCK(1, 1), abc1600bus_cards, nullptr);
	m_bus[4]->irq_callback().set(FUNC(abc1656_device::xint5_w));
	m_bus[4]->pren_callback().set("pren", FUNC(input_merger_device::in_w<4>));
	m_bus[4]->trrq_callback().set("trrq", FUNC(input_merger_device::in_w<4>));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc1656_device - constructor
//-------------------------------------------------

abc1656_device::abc1656_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ABC1656, tag, owner, clock),
	device_abcbus_card_interface(mconfig, *this),
	m_bus(*this, "bus%u", 0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1656_device::device_start()
{
}


//-------------------------------------------------
//  abcbus_cs - card select
//-------------------------------------------------

void abc1656_device::abcbus_cs(uint8_t data)
{
	for (auto &slot : m_bus)
		slot->write_cs(data);
}


//-------------------------------------------------
//  abcbus_inp - input
//-------------------------------------------------

uint8_t abc1656_device::abcbus_inp()
{
	uint8_t data = 0xff;

	for (auto &slot : m_bus)
		data &= slot->read_inp();

	return data;
}


//-------------------------------------------------
//  abcbus_out - output
//-------------------------------------------------

void abc1656_device::abcbus_out(uint8_t data)
{
	for (auto &slot : m_bus)
		slot->write_out(data);
}


//-------------------------------------------------
//  abcbus_stat - status
//-------------------------------------------------

uint8_t abc1656_device::abcbus_stat()
{
	uint8_t data = 0xff;

	for (auto &slot : m_bus)
		data &= slot->read_stat();

	return data;
}


//-------------------------------------------------
//  abcbus_c1 - command 1
//-------------------------------------------------

void abc1656_device::abcbus_c1(uint8_t data)
{
	for (auto &slot : m_bus)
		slot->write_c1(data);
}


//-------------------------------------------------
//  abcbus_c2 - command 2
//-------------------------------------------------

void abc1656_device::abcbus_c2(uint8_t data)
{
	for (auto &slot : m_bus)
		slot->write_c2(data);
}


//-------------------------------------------------
//  abcbus_c3 - command 3
//-------------------------------------------------

void abc1656_device::abcbus_c3(uint8_t data)
{
	for (auto &slot : m_bus)
		slot->write_c3(data);
}


//-------------------------------------------------
//  abcbus_c4 - command 4
//-------------------------------------------------

void abc1656_device::abcbus_c4(uint8_t data)
{
	for (auto &slot : m_bus)
		slot->write_c4(data);
}


//-------------------------------------------------
//  abcbus_tren - transfer enable read
//-------------------------------------------------

uint8_t abc1656_device::abcbus_tren()
{
	uint8_t data = 0xff;

	for (auto &slot : m_bus)
		data &= slot->read_tren();

	return data;
}


//-------------------------------------------------
//  abcbus_tren - transfer enable write
//-------------------------------------------------

void abc1656_device::abcbus_tren(uint8_t data)
{
	for (auto &slot : m_bus)
		slot->write_tren(data);
}


//-------------------------------------------------
//  abcbus_prac -
//-------------------------------------------------

void abc1656_device::abcbus_prac(int state)
{
	for (auto &slot : m_bus)
		slot->prac_w(state);
}
