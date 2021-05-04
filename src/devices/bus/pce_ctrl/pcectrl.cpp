// license:BSD-3-Clause
// copyright-holders:cam900
/**********************************************************************

    NEC PC Engine/TurboGrafx-16 controller port emulation

    Based on SMS controller port emulation (devices\bus\sms_ctrl\*.*)
	by Fabio Priuli,
	PC engine emulation (mame\*\pce.*)
	by Charles MacDonald, Wilbert Pol, Angelo Salese

    Controller port interface:
    DIN-8 interface for TurboGrafx-16:

       /-------------\
      //-------------\\
     //       2       \\
    ||   5         4   ||
    ||                 ||
    || 3      8      1 ||
    ||                 ||
    ||    7       6    ||
     \\      ---      //
      \\----/   \----//
       \-------------/

    Mini DIN-8 interface for others:

       /-------------\
      //----|   |----\\
     //     |---|     \\
    ||  6     7     8  ||
    ||                 ||
    ||  3    4      5  ||
    ||                 ||
    ||--|  1     2  |--||
     \  |           |  /
      \ |-----------| /
       \-------------/

    1: +5V
    2: O0 (Read bit 0)
    3: O1 (Read bit 1)
    4: O2 (Read bit 2)
    5: O3 (Read bit 3)
    6: Clock (Write bit 0)
    7: Reset (Write bit 1)
    8: GND

**********************************************************************/

#include "emu.h"
#include "screen.h"
#include "pcectrl.h"

// slot devices
#include "joypad2.h"
#include "joypad6.h"
#include "multitap.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(PCE_CONTROL_PORT, pce_control_port_device, "pce_control_port", "NEC PC Engine/TurboGrafx-16 controller port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_pce_control_port_interface - constructor
//-------------------------------------------------

device_pce_control_port_interface::device_pce_control_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "pcectrl")
{
	m_port = dynamic_cast<pce_control_port_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_pce_control_port_interface - destructor
//-------------------------------------------------

device_pce_control_port_interface::~device_pce_control_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pce_control_port_device - constructor
//-------------------------------------------------

pce_control_port_device::pce_control_port_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, PCE_CONTROL_PORT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_device(nullptr)
{
}


//-------------------------------------------------
//  pce_control_port_device - destructor
//-------------------------------------------------

pce_control_port_device::~pce_control_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pce_control_port_device::device_start()
{
	m_device = dynamic_cast<device_pce_control_port_interface *>(get_card_device());
}


//-------------------------------------------------
//  port_r - controller port read
//-------------------------------------------------

u8 pce_control_port_device::port_r()
{
	u8 data = 0xf;
	if (m_device)
		data = m_device->peripheral_r() & 0xf; // 4 bit
	return data;
}


//-------------------------------------------------
//  clk_w - Clock pin write
//-------------------------------------------------

void pce_control_port_device::clk_w(int state)
{
	if (m_device)
		m_device->clk_w(state);
}


//-------------------------------------------------
//  rst_w - Reset pin write
//-------------------------------------------------

void pce_control_port_device::rst_w(int state)
{
	if (m_device)
		m_device->rst_w(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( pce_control_port_devices )
//-------------------------------------------------

void pce_control_port_devices(device_slot_interface &device)
{
	device.option_add("joypad2",  PCE_JOYPAD2);
	device.option_add("joypad6",  PCE_JOYPAD6);
	device.option_add("multitap", PCE_MULTITAP);
	// 3 Button Joypad/Joysticks (ex: Avenue Pad 3)
	// Pachinko Controller (CJPC-101)
	// PC Engine Mouse (PI-PD10)
	// Memory Base 128 (PI-AD19)
	// etc...
}
