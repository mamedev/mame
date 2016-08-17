// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System controller ports
    and Family Computer expansion port emulation

    Here we emulate in fact 3 different kind of ports, which are
    connected to different bis of memory locations $4016 and $4017:
    - NES controller ports: these are hooked to bit 0,3,4 of the
      corresponding address ($4016 for port1, $4017 for port2)
    - FC controller ports: these are only hooked to bit 0 of the
      corresponding address (so that e.g. a NES Zapper could not
      be connected to a later FC AV model, because its inputs
      would not be detected)
    - FC expansion port: this is hooked to bits 0-4 of both addresses
    To make things a little bit more complex, old FC models have the
    controller hardwired to the unit, and the P2 controllers are
    directly hooked also to one of the expansion port lines (namely,
    microphone inputs from P2 go to $4016 bit 2)

    Even if the controller port and the expansion port are
    physically different (the FC expansion is a 15pin port, while
    the controller ports are 7pin), we emulate them as variants of a
    common device, exposing the following handlers:
    - read_bit0: for bit0 reads, which are typically used for serial
      inputs from controllers
    - read_bit34: for bit3,4 reading, expected to be at the correct
      offset (but we don't currently check for read_bit34 & 0xf8==0)
    - read_exp: for reads going through the expansion, with a offset
      parameter to decide whether we are reading from $4016 and $4017
    - write: to acknowledge writes to $4016

    The driver emulation will take care to only call the correct
    handlers they have hooks for: Basic usage is that the expansion
    port calls read_exp, FC ctrl ports call read_bit0, and NES ctrl
    ports call both read_bit0 and read_bit34. However, to cope with
    the original FC microphone, we will have the second controller
    port calling read_exp too.

**********************************************************************/

#include "ctrl.h"
// slot devices
#include "4score.h"
#include "arkpaddle.h"
#include "bcbattle.h"
#include "ftrainer.h"
#include "fckeybrd.h"
#include "hori.h"
#include "joypad.h"
#include "konamihs.h"
#include "miracle.h"
#include "mjpanel.h"
#include "pachinko.h"
#include "partytap.h"
#include "powerpad.h"
#include "suborkey.h"
#include "zapper.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NES_CONTROL_PORT = &device_creator<nes_control_port_device>;



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_nes_control_port_interface - constructor
//-------------------------------------------------

device_nes_control_port_interface::device_nes_control_port_interface(const machine_config &mconfig, device_t &device) :
									device_slot_card_interface(mconfig, device)
{
	m_port = dynamic_cast<nes_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_nes_control_port_interface - destructor
//-------------------------------------------------

device_nes_control_port_interface::~device_nes_control_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nes_control_port_device - constructor
//-------------------------------------------------

nes_control_port_device::nes_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
						device_t(mconfig, NES_CONTROL_PORT, "Nintendo NES/FC control port", tag, owner, clock, "nes_control_port", __FILE__),
						device_slot_interface(mconfig, *this), m_device(nullptr)
{
}


//-------------------------------------------------
//  nes_control_port_device - destructor
//-------------------------------------------------

nes_control_port_device::~nes_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nes_control_port_device::device_start()
{
	m_device = dynamic_cast<device_nes_control_port_interface *>(get_card_device());
	m_brightpixel_cb.bind_relative_to(*owner());
}


UINT8 nes_control_port_device::read_bit0()
{
	UINT8 data = 0;
	if (m_device)
		data = m_device->read_bit0();
	return data;
}

UINT8 nes_control_port_device::read_bit34()
{
	UINT8 data = 0;
	if (m_device)
		data = m_device->read_bit34();
	return data;
}

UINT8 nes_control_port_device::read_exp(offs_t offset)
{
	UINT8 data = 0;
	if (m_device)
		data = m_device->read_exp(offset);
	return data;
}

void nes_control_port_device::write(UINT8 data)
{
	if (m_device)
		m_device->write(data);
}



//-------------------------------------------------
//  SLOT_INTERFACE( nes_control_port_devices )
//-------------------------------------------------

SLOT_INTERFACE_START( nes_control_port1_devices )
	SLOT_INTERFACE("joypad", NES_JOYPAD)
	SLOT_INTERFACE("zapper", NES_ZAPPER)
	SLOT_INTERFACE("4score_p1p3", NES_4SCORE_P1P3)
	SLOT_INTERFACE("miracle_piano", NES_MIRACLE)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( nes_control_port2_devices )
	SLOT_INTERFACE("joypad", NES_JOYPAD)
	SLOT_INTERFACE("zapper", NES_ZAPPER)
	SLOT_INTERFACE("vaus", NES_ARKPADDLE)
	SLOT_INTERFACE("powerpad", NES_POWERPAD)
	SLOT_INTERFACE("4score_p2p4", NES_4SCORE_P2P4)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( fc_control_port1_devices )
	SLOT_INTERFACE("joypad", NES_JOYPAD)
	SLOT_INTERFACE("ccpad_left", NES_CCPAD_LEFT)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( fc_control_port2_devices )
	SLOT_INTERFACE("joypad", NES_JOYPAD)
	SLOT_INTERFACE("joypad_old", NES_FCPAD_P2)
	SLOT_INTERFACE("ccpad_right", NES_CCPAD_RIGHT)
SLOT_INTERFACE_END

SLOT_INTERFACE_START( fc_expansion_devices )
	SLOT_INTERFACE("joypad", NES_JOYPAD)
	SLOT_INTERFACE("arcstick", NES_ARCSTICK)
	SLOT_INTERFACE("fc_keyboard", NES_FCKEYBOARD)
	SLOT_INTERFACE("zapper", NES_ZAPPER)
	SLOT_INTERFACE("vaus", NES_ARKPADDLE_FC)
	SLOT_INTERFACE("family_trainer", NES_FTRAINER)
	SLOT_INTERFACE("konamihs", NES_KONAMIHS)
	SLOT_INTERFACE("mj_panel", NES_MJPANEL)
	SLOT_INTERFACE("pachinko", NES_PACHINKO)
	SLOT_INTERFACE("partytap", NES_PARTYTAP)
	SLOT_INTERFACE("hori_twin", NES_HORITWIN)
	SLOT_INTERFACE("hori_4p", NES_HORI4P)
	SLOT_INTERFACE("barcode_battler", NES_BARCODE_BATTLER)
	SLOT_INTERFACE("subor_keyboard", NES_SUBORKEYBOARD)
SLOT_INTERFACE_END
