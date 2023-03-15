// license:BSD-3-Clause
// copyright-holders:kmg, Fabio Priuli
/**********************************************************************

    Here we emulate two distinct mat controllers which share a common
    read/write interface with the hardware:

    Nintendo Family Computer - Bandai Family Trainer Mat
    Nintendo Family Computer - IGS Tap-tap Mat

**********************************************************************/

#include "emu.h"
#include "fcmat.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_FTRAINER,  nes_ftrainer_device,  "nes_famtrain",  "Bandai Family Trainer")
DEFINE_DEVICE_TYPE(NES_TAPTAPMAT, nes_taptapmat_device, "nes_taptapmat", "IGS Tap-tap Mat")


static INPUT_PORTS_START( nes_ftrainer )
	PORT_START("LAYOUT")
	PORT_CONFNAME( 0x01, 0x00, "Family Trainer Button Layout")
	PORT_CONFSETTING(    0x00, "Side A" )
	PORT_CONFSETTING(    0x01, "Side B" )

	// difference between the two sides is that we mirror the key mapping to match the real pad layout!
	PORT_START("MAT_COL.0")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                                                          PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 )  PORT_NAME("Family Trainer Mid1")  PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                                                          PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Family Trainer 12")    PORT_CODE(KEYCODE_M) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 )  PORT_NAME("Family Trainer 8")     PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 )  PORT_NAME("Family Trainer 4")     PORT_CODE(KEYCODE_U) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("MAT_COL.1")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Family Trainer Low1")  PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("Family Trainer Mid2")  PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )  PORT_NAME("Family Trainer Top1")  PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Family Trainer 11")    PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("Family Trainer 7")     PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )  PORT_NAME("Family Trainer 3")     PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("MAT_COL.2")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Family Trainer Low2")  PORT_CODE(KEYCODE_N) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 )  PORT_NAME("Family Trainer Mid3")  PORT_CODE(KEYCODE_H) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Family Trainer Top2")  PORT_CODE(KEYCODE_Y) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Family Trainer 10")    PORT_CODE(KEYCODE_B) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 )  PORT_NAME("Family Trainer 6")     PORT_CODE(KEYCODE_G) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Family Trainer 2")     PORT_CODE(KEYCODE_T) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)

	PORT_START("MAT_COL.3")
	// side A layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )                                                          PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 )  PORT_NAME("Family Trainer Mid4")  PORT_CODE(KEYCODE_J) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )                                                          PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x00)
	// side B layout
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 )  PORT_NAME("Family Trainer 9")     PORT_CODE(KEYCODE_V) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 )  PORT_NAME("Family Trainer 5")     PORT_CODE(KEYCODE_F) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )  PORT_NAME("Family Trainer 1")     PORT_CODE(KEYCODE_R) PORT_CONDITION("LAYOUT", 0x01, EQUALS, 0x01)
INPUT_PORTS_END


static INPUT_PORTS_START( nes_taptapmat )
	PORT_START("MAT_COL.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON12 ) PORT_NAME("Bottom Left (Frankenstein)")    PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON8 )  PORT_NAME("Middle Left (Bang!)")           PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 )  PORT_NAME("Top Left (Antlion)")            PORT_CODE(KEYCODE_R)

	PORT_START("MAT_COL.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Bottom Mid-Left (Bang!)")       PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON7 )  PORT_NAME("Middle Mid-Left (Caveman)")     PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )  PORT_NAME("Top Mid-Left (Bang!)")          PORT_CODE(KEYCODE_T)

	PORT_START("MAT_COL.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("Bottom Mid-Right (Shark)")      PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 )  PORT_NAME("Middle Mid-Right (Bang!)")      PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )  PORT_NAME("Top Mid-Right (Dinosaur)")      PORT_CODE(KEYCODE_Y)

	PORT_START("MAT_COL.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON9 )  PORT_NAME("Bottom Right (Bang!)")          PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 )  PORT_NAME("Middle Right (Mole)")           PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )  PORT_NAME("Top Right (Bang!)")             PORT_CODE(KEYCODE_U)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_ftrainer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_ftrainer );
}

ioport_constructor nes_taptapmat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_taptapmat );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  constructor
//-------------------------------------------------

nes_fcmat_device::nes_fcmat_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_mat(*this, "MAT_COL.%u", 0)
	, m_row_scan(0)
{
}

nes_ftrainer_device::nes_ftrainer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_fcmat_device(mconfig, NES_FTRAINER, tag, owner, clock)
{
}

nes_taptapmat_device::nes_taptapmat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_fcmat_device(mconfig, NES_TAPTAPMAT, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_fcmat_device::device_start()
{
	save_item(NAME(m_row_scan));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_fcmat_device::read_exp(offs_t offset)
{
	u8 ret = 0;

	if (offset == 1)    // $4017
	{
		for (int row = 0; row < 3; row++)  // bottom row first (buttons 9,10,11,12 on Family Trainer)
			if (!BIT(m_row_scan, row))
				for (int col = 0; col < 4; col++)  // left column first (Family Trainer side A and Tap-tap Mat)
					ret |= BIT(m_mat[col]->read(), row) << (col + 1);
		ret = ~ret & 0x1e;  // 0: pressed, 1: not pressed
	}

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_fcmat_device::write(u8 data)
{
	// select row to scan
	m_row_scan = data & 0x07;
}
