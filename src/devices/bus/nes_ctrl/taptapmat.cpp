// license:BSD-3-Clause
// copyright-holders:kmg, Fabio Priuli
/**********************************************************************

    Nintendo Family Computer - IGS Tap-tap Mat

**********************************************************************/

#include "emu.h"
#include "taptapmat.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_TAPTAPMAT, nes_taptapmat_device, "nes_taptapmat", "IGS Tap-tap Mat")


static INPUT_PORTS_START( nes_taptapmat )
	PORT_START("TTM_COL.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Bottom Left (Frankenstein)")    PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Middle Left (Bang!)")           PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Top Left (Antlion)")            PORT_CODE(KEYCODE_R)

	PORT_START("TTM_COL.1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Bottom Mid-Left (Bang!)")       PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Middle Mid-Left (Caveman)")     PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Top Mid-Left (Bang!)")          PORT_CODE(KEYCODE_T)

	PORT_START("TTM_COL.2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Bottom Mid-Right (Shark)")      PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Middle Mid-Right (Bang!)")      PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Top Mid-Right (Dinosaur)")      PORT_CODE(KEYCODE_Y)

	PORT_START("TTM_COL.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Bottom Right (Bang!)")          PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Middle Right (Mole)")           PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Top Right (Bang!)")             PORT_CODE(KEYCODE_U)
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_taptapmat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_taptapmat );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_taptapmat_device - constructor
//-------------------------------------------------

nes_taptapmat_device::nes_taptapmat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_TAPTAPMAT, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_mat(*this, "TTM_COL.%u", 0)
	, m_row_scan(0)
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_taptapmat_device::device_start()
{
	save_item(NAME(m_row_scan));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void nes_taptapmat_device::device_reset()
{
	m_row_scan = 0;
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_taptapmat_device::read_exp(offs_t offset)
{
	u8 ret = 0;

	if (offset == 1)    // $4017
	{
		for (int row = 0; row < 3; row++)  // bottom row first
			if (!BIT(m_row_scan, row))
				for (int col = 0; col < 4; col++)  // left column first
					ret |= BIT(m_mat[col]->read(), row) << (col + 1);
		ret = ~ret & 0x1e;  // 0: pressed, 1: not pressed
	}

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_taptapmat_device::write(u8 data)
{
	// select rows to scan
	m_row_scan = data & 0x07;
}
