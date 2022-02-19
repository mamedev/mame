// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    Nintendo Family Computer & Entertainment System controller ports
    and Family Computer expansion port emulation

    Here we emulate in fact 3 different kind of ports, which are
    connected to different bits of memory locations $4016 and $4017:
    - NES controller ports: these are hooked to bit 0,3,4 of the
      corresponding address ($4016 for port1, $4017 for port2)
    - FC controller ports: these are only hooked to bit 0 of the
      corresponding address (so that e.g. a NES Zapper could not
      be connected to a later FC AV model, because its inputs
      would not be detected)
    - FC expansion port: this is hooked to bit 1 of memory location
      $4016 and bits 0-4 of $4017
    To make things a little bit more complex, old FC models have
    controllers hardwired to the unit, and the P2 controllers are
    directly hooked to bit 2 of $4016 (for microphone inputs).

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
    the original FC microphone, we have a one-off read_bit2 handler
    called by the second controller.

**********************************************************************/

#include "emu.h"
#include "ctrl.h"
// slot devices
#include "4score.h"
#include "arkpaddle.h"
#include "bcbattle.h"
#include "dorepiano.h"
#include "fckeybrd.h"
#include "fcmat.h"
#include "hori.h"
#include "joypad.h"
#include "konamibag.h"
#include "konamihs.h"
#include "miracle.h"
#include "mjpanel.h"
#include "pachinko.h"
#include "partytap.h"
#include "powerpad.h"
#include "snesadapter.h"
#include "suborkey.h"
#include "turbofile.h"
#include "zapper.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NES_CONTROL_PORT, nes_control_port_device, "nes_control_port", "Nintendo NES/FC control port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_nes_control_port_interface - constructor
//-------------------------------------------------

device_nes_control_port_interface::device_nes_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "nesctrl")
	, m_strobe(0)
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

nes_control_port_device::nes_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, NES_CONTROL_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_nes_control_port_interface>(mconfig, *this),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_device(nullptr)
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
	m_device = get_card_device();
}


u8 nes_control_port_device::read_bit0()
{
	u8 data = 0;
	if (m_device)
		data = m_device->read_bit0();
	return data;
}

u8 nes_control_port_device::read_bit2()
{
	u8 data = 0;
	if (m_device)
		data = m_device->read_bit2();
	return data;
}

u8 nes_control_port_device::read_bit34()
{
	u8 data = 0;
	if (m_device)
		data = m_device->read_bit34();
	return data;
}

u8 nes_control_port_device::read_exp(offs_t offset)
{
	u8 data = 0;
	if (m_device)
		data = m_device->read_exp(offset);
	return data;
}

void nes_control_port_device::write(u8 data)
{
	if (m_device)
		m_device->write(data);
}



//-------------------------------------------------
//  SLOT_INTERFACE( nes_control_port_devices )
//-------------------------------------------------

void nes_control_port1_devices(device_slot_interface &device)
{
	device.option_add("joypad", NES_JOYPAD);
	device.option_add("zapper", NES_ZAPPER);
	device.option_add("4score_p1p3", NES_4SCORE_P1P3);
	device.option_add("miracle_piano", NES_MIRACLE);
	device.option_add("snes_adapter", NES_SNESADAPTER);
	device.option_add("vboy", NES_VBOYCTRL);
}

void nes_control_port2_devices(device_slot_interface &device)
{
	device.option_add("joypad", NES_JOYPAD);
	device.option_add("zapper", NES_ZAPPER);
	device.option_add("vaus", NES_ARKPADDLE);
	device.option_add("powerpad", NES_POWERPAD);
	device.option_add("4score_p2p4", NES_4SCORE_P2P4);
	device.option_add("snes_adapter", NES_SNESADAPTER);
	device.option_add("vboy", NES_VBOYCTRL);
}

void fc_control_port1_devices(device_slot_interface &device)
{
	device.option_add("joypad", NES_JOYPAD);
	device.option_add("ccpad_left", NES_CCPAD_LEFT);
}

void fc_control_port2_devices(device_slot_interface &device)
{
	device.option_add("joypad", NES_JOYPAD);
	device.option_add("joypad_old", NES_FCPAD_P2);
	device.option_add("ccpad_right", NES_CCPAD_RIGHT);
}

void fc_expansion_devices(device_slot_interface &device)
{
	device.option_add("joypad", NES_FCPAD_EXP);
	device.option_add("arcstick", NES_ARCSTICK);
	device.option_add("fc_keyboard", NES_FCKEYBOARD);
	device.option_add("zapper", NES_ZAPPER);
	device.option_add("bandaihs", NES_BANDAIHS);
	device.option_add("vaus", NES_ARKPADDLE_FC);
	device.option_add("family_trainer", NES_FTRAINER);
	device.option_add("konamibag", NES_KONAMIBAG);
	device.option_add("konamihs", NES_KONAMIHS);
	device.option_add("konami_piano", NES_DOREPIANO);
	device.option_add("mahjong", NES_MJPANEL);
	device.option_add("pachinko", NES_PACHINKO);
	device.option_add("partytap", NES_PARTYTAP);
	device.option_add("hori_twin", NES_HORITWIN);
	device.option_add("hori_4p", NES_HORI4P);
	device.option_add("barcode_battler", NES_BARCODE_BATTLER);
	device.option_add("subor_keyboard", NES_SUBORKEYBOARD);
	device.option_add("taptapmat", NES_TAPTAPMAT);
	device.option_add("turbofile", NES_TURBOFILE);
}

void famibox_control_port12_devices(device_slot_interface &device)
{
	device.option_add("joypad", NES_JOYPAD);
	device.option_add("zapper", NES_ZAPPER);
}

void famibox_control_port3_devices(device_slot_interface &device)
{
	device.option_add("zapper", NES_ZAPPER);
}
