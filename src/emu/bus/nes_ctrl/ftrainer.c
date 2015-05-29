// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - Bandai Family Trainer Mat

**********************************************************************/

#include "ftrainer.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type NES_FTRAINER = &device_creator<nes_ftrainer_device>;


static INPUT_PORTS_START( nes_joypad )
	PORT_START("LAYOUT")
	PORT_CONFNAME( 0x01, 0x00, "Family Trainer Button Layout")
	PORT_CONFSETTING(  0x00, "Side A" )
	PORT_CONFSETTING(  0x01, "Side B" )

	// difference between the two sides is that we mirror the key mapping to match the real pad layout!
	PORT_START("FT_COL.0")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                                                        PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Mid1")  PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                                                        PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 12")    PORT_CODE(KEYCODE_M) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 8")     PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 4")     PORT_CODE(KEYCODE_U) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("FT_COL.1")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Low1")  PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Mid2")  PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Top1")  PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 11")    PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 7")     PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 3")     PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("FT_COL.2")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Low2")  PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Mid3")  PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Top2")  PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 10")    PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 6")     PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 2")     PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("FT_COL.3")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                                                        PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer Mid4")  PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                                                        PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 9")     PORT_CODE(KEYCODE_V) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 5")     PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Family Trainer 1")     PORT_CODE(KEYCODE_R) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_ftrainer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_joypad );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_ftrainer_device - constructor
//-------------------------------------------------

nes_ftrainer_device::nes_ftrainer_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, NES_FTRAINER, "Bandai Family Trainer", tag, owner, clock, "nes_famtrain", __FILE__),
					device_nes_control_port_interface(mconfig, *this),
					m_trainer(*this, "FT_COL")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_ftrainer_device::device_start()
{
	save_item(NAME(m_row_scan));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_ftrainer_device::device_reset()
{
	m_row_scan = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

UINT8 nes_ftrainer_device::read_exp(offs_t offset)
{
	UINT8 ret = 0;
	if (offset == 1)    //$4017
	{
		if (!BIT(m_row_scan, 0))
		{
			// read low line: buttons 9,10,11,12
			for (int i = 0; i < 4; i++)
				ret |= ((m_trainer[i]->read() & 0x01) << (1 + i));
		}
		else if (!BIT(m_row_scan, 1))
		{
			// read mid line: buttons 5,6,7,8
			for (int i = 0; i < 4; i++)
				ret |= ((m_trainer[i]->read() & 0x02) << (1 + i));
		}
		else if (!BIT(m_row_scan, 2))
		{
			// read high line: buttons 1,2,3,4
			for (int i = 0; i < 4; i++)
				ret |= ((m_trainer[i]->read() & 0x04) << (1 + i));
		}
	}
	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_ftrainer_device::write(UINT8 data)
{
	// select row to scan
	m_row_scan = data & 0x07;
}
