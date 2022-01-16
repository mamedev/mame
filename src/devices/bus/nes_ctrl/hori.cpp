// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer Hori Twin and 4 Players adapters

    In general these adapters work by reading pin 13 (the Famicom sees
    this in memory location $4016, bit 1) of their subports. It is not
    clear if other lines are pass-thru. For the moment we only emulate
    joypads connected to these ports, which only use pin 13 when read.

    The Hori Twin passes subport 1's input directly through to $4016
    bit 1. It reroutes subport 2's to $4017 bit 1. Incidentally, HAL
    released a device, Joypair, that should be functionally equivalent
    to the Hori Twin.

    The Hori 4 Players adapter has a 2/4 switch. In 2P mode it works
    just like the Hori Twin. In 4P mode it still passes through joypad
    reads to bit 1 of $4016 and $4017, but it operates like the NES Four
    Score (which in fact uses a Hori PCB). Subport 1 then subport 3
    inputs are passed serially to $4016 bit 1. Likewise subport 2 and 4
    go to $4017 bit 1. A signature byte can be read from both locations
    following this.

**********************************************************************/

#include "emu.h"
#include "hori.h"
#include "joypad.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_HORITWIN, nes_horitwin_device, "nes_horitwin", "FC Hori Twin Adapter")
DEFINE_DEVICE_TYPE(NES_HORI4P,   nes_hori4p_device,   "nes_hori4p",   "FC Hori 4 Players Adaptor")


static INPUT_PORTS_START( nes_hori4p )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x01, 0x00, "4 Players / 2 Players")
	PORT_CONFSETTING(  0x00, "2 Players" )
	PORT_CONFSETTING(  0x01, "4 Players" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor nes_hori4p_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_hori4p );
}


static void hori_adapter(device_slot_interface &device)
{
	device.option_add("joypad", NES_FCPAD_EXP);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_horitwin_device::device_add_mconfig(machine_config &config)
{
	for (int i = 0; i < 2; i++)
	{
		NES_CONTROL_PORT(config, m_subexp[i], hori_adapter, "joypad");
		if (m_port != nullptr)
			m_subexp[i]->set_screen_tag(m_port->m_screen);
	}
}

void nes_hori4p_device::device_add_mconfig(machine_config &config)
{
	for (int i = 0; i < 4; i++)
	{
		NES_CONTROL_PORT(config, m_subexp[i], hori_adapter, "joypad");
		if (m_port != nullptr)
			m_subexp[i]->set_screen_tag(m_port->m_screen);
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_horitwin_device - constructor
//-------------------------------------------------

nes_horitwin_device::nes_horitwin_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_HORITWIN, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_subexp(*this, "subexp%u", 0)
{
}

nes_hori4p_device::nes_hori4p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NES_HORI4P, tag, owner, clock)
	, device_nes_control_port_interface(mconfig, *this)
	, m_subexp(*this, "subexp%u", 0)
	, m_cfg(*this, "CONFIG")
{
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void nes_hori4p_device::device_start()
{
	save_item(NAME(m_count));
	save_item(NAME(m_sig));
	save_item(NAME(m_strobe));

	reset_regs();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 nes_horitwin_device::read_exp(offs_t offset)
{
	return m_subexp[offset]->read_exp(0);
}

u8 nes_hori4p_device::read_exp(offs_t offset)
{
	u8 ret = 0;
	int mode4p = m_cfg->read();

	if (m_count[offset] < 16 || !mode4p)  // read from P1/P2 for first byte, P3/P4 for second byte if in 4P mode
	{
		int port = 2 * (BIT(m_count[offset], 3) & mode4p) + offset;
		ret = m_subexp[port]->read_exp(0);
	}
	else    // excess reads return a distinct signature byte on both $4016 and $4017
	{
		ret = (m_sig[offset] & 1) << 1;
		m_sig[offset] >>= 1;
	}
	m_count[offset]++;

	return ret;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void nes_horitwin_device::write(u8 data)
{
	m_subexp[0]->write(data);
	m_subexp[1]->write(data);
}

void nes_hori4p_device::write(u8 data)
{
	for (int i = 0; i < 4; i++)
		m_subexp[i]->write(data);

	if (write_strobe(data))
		reset_regs();
}

void nes_hori4p_device::reset_regs()
{
	m_count[0] = 0;
	m_count[1] = 0;

	// signature third bytes returned in 4P mode, pair is flipped from what Four Score returns
	m_sig[0] = 0x04;
	m_sig[1] = 0x08;
}
