// license:BSD-3-Clause
// copyright-holders:hap
/*

The ChessMachine DR by Tasc

8-bit ISA card, 2nd version of The ChessMachine.
see chessmachine_device for technical notes

*/

#include "emu.h"
#include "chessmdr.h"


DEFINE_DEVICE_TYPE(ISA8_CHESSMDR, isa8_chessmdr_device, "isa_chessmdr", "The ChessMachine DR")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

isa8_chessmdr_device::isa8_chessmdr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA8_CHESSMDR, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_chessm(*this, "chessm")
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_chessmdr_device::device_start()
{
	set_isa_device();
	m_installed = false;

	save_item(NAME(m_installed));
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_chessmdr_device::device_reset()
{
	if (!m_installed)
	{
		// MAME doesn't allow reading ioport at device_start
		u16 port = ioport("DSW")->read() * 0x40 + 0x10;
		m_isa->install_device(port, port+1, read8_delegate(*this, FUNC(isa8_chessmdr_device::chessmdr_r)), write8_delegate(*this, FUNC(isa8_chessmdr_device::chessmdr_w)));

		m_installed = true;
	}
}



//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( chessmdr )
	PORT_START("DSW") // DIP switch on the ISA card PCB, installer shows range 0x110-0x3D0
	PORT_DIPNAME( 0x0f, 0x09, "I/O Port Address" ) PORT_DIPLOCATION("CMDR_SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, "0x010 (Invalid)" )
	PORT_DIPSETTING(    0x01, "0x050 (Invalid)" )
	PORT_DIPSETTING(    0x02, "0x090 (Invalid)" )
	PORT_DIPSETTING(    0x03, "0x0D0 (Invalid)" )
	PORT_DIPSETTING(    0x04, "0x110" )
	PORT_DIPSETTING(    0x05, "0x150" )
	PORT_DIPSETTING(    0x06, "0x190" )
	PORT_DIPSETTING(    0x07, "0x1D0" )
	PORT_DIPSETTING(    0x08, "0x210" )
	PORT_DIPSETTING(    0x09, "0x250" )
	PORT_DIPSETTING(    0x0a, "0x290" )
	PORT_DIPSETTING(    0x0b, "0x2D0" )
	PORT_DIPSETTING(    0x0c, "0x310" )
	PORT_DIPSETTING(    0x0d, "0x350" )
	PORT_DIPSETTING(    0x0e, "0x390" )
	PORT_DIPSETTING(    0x0f, "0x3D0" )
INPUT_PORTS_END

ioport_constructor isa8_chessmdr_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(chessmdr);
}



//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_chessmdr_device::device_add_mconfig(machine_config &config)
{
	CHESSMACHINE(config, m_chessm, 15'000'000);
}



/******************************************************************************
    I/O
******************************************************************************/

READ8_MEMBER(isa8_chessmdr_device::chessmdr_r)
{
	if (offset == 1)
		return m_chessm->data_r() ? 0 : 0x80;
	else
		return 0xff;
}

WRITE8_MEMBER(isa8_chessmdr_device::chessmdr_w)
{
	if (offset == 0)
	{
		m_chessm->data0_w(data & 1);
		m_chessm->data1_w(data & 0x80);
		m_chessm->reset_w(data & 2);
	}
}

