// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 SCI Controller

   Lies at 0xfffffe00-0xfffffe0f

  TODO:
  - diserial;
  - CPU callbacks for RX and TX;

***************************************************************************/

#include "emu.h"
#include "sh7604_sci.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SH7604_SCI, sh7604_sci_device, "sh7604sci", "SH7604 SCI Controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

READ8_MEMBER(sh7604_sci_device::serial_mode_r)
{
	return m_smr;
}

WRITE8_MEMBER(sh7604_sci_device::serial_mode_w)
{
	m_smr = data;

	logerror("%s: serial mode set:\n",tag());
	logerror("\tCommunication Mode: %s mode\n",data & 0x80 ? "clocked synchronous" : "asynchronous");
	logerror("\tCharacter Length: %s mode\n",data & 0x40 ? "7-bit" : "8-bit");
	logerror("\tParity Enable: %s\n",data & 0x20 ? "yes" : "no");
	logerror("\tParity Mode: %s\n",data & 0x10 ? "Odd" : "Even");
	logerror("\tStop bits: %s\n",data & 0x08 ? "2" : "1");
	logerror("\tMultiprocessor mode: %s\n",data & 0x04 ? "yes" : "no");
	logerror("\tClock select: clock/%d\n",4 << ((data & 0x03)*2));
}

READ8_MEMBER(sh7604_sci_device::serial_control_r)
{
	return m_scr;
}

WRITE8_MEMBER(sh7604_sci_device::serial_control_w)
{
	m_scr = data;

	if(data & 0x30)
		throw emu_fatalerror("%s: enabled serial control %02x\n", tag(),data);
}

READ8_MEMBER(sh7604_sci_device::serial_status_r)
{
	return m_ssr;
}

WRITE8_MEMBER(sh7604_sci_device::serial_ack_w)
{
	// TODO: verify this
	m_ssr = (m_ssr & 0x06) | (m_ssr & data & 0xf9);
}

READ8_MEMBER(sh7604_sci_device::bitrate_r )
{
	return m_brr;
}

WRITE8_MEMBER(sh7604_sci_device::bitrate_w )
{
	m_brr = data;
}

READ8_MEMBER(sh7604_sci_device::transmit_data_r)
{
	// ...
	return 0;
}

WRITE8_MEMBER(sh7604_sci_device::transmit_data_w)
{
	// ...
}

READ8_MEMBER(sh7604_sci_device::receive_data_r)
{
	// ...
	return 0;
}

void sh7604_sci_device::sci_regs(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(sh7604_sci_device::serial_mode_r), FUNC(sh7604_sci_device::serial_mode_w));
	map(0x01, 0x01).rw(FUNC(sh7604_sci_device::bitrate_r), FUNC(sh7604_sci_device::bitrate_w));
	map(0x02, 0x02).rw(FUNC(sh7604_sci_device::serial_control_r), FUNC(sh7604_sci_device::serial_control_w));
	map(0x03, 0x03).rw(FUNC(sh7604_sci_device::transmit_data_r), FUNC(sh7604_sci_device::transmit_data_w));
	map(0x04, 0x04).rw(FUNC(sh7604_sci_device::serial_status_r), FUNC(sh7604_sci_device::serial_ack_w));
	map(0x05, 0x05).r(FUNC(sh7604_sci_device::receive_data_r));
}

//-------------------------------------------------
//  sh7604_sci_device - constructor
//-------------------------------------------------

sh7604_sci_device::sh7604_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7604_SCI, tag, owner, clock)

{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sh7604_sci_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sh7604_sci_device::device_reset()
{
	m_smr = 0;
	m_scr = 0;
	m_ssr = STATUS_TDRE|STATUS_TEND; //0x84;
	m_brr = 0xff;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ8_MEMBER( sh7604_sci_device::read )
{
	return space.read_byte(offset);
}

WRITE8_MEMBER( sh7604_sci_device::write )
{
	space.write_byte(offset,data);
}
