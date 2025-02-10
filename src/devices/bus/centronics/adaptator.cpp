// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

"The Adaptator" a.k.a. DIY parallel port to 2x DE-9 Multitap adapter

Originally bundled with the Amiga/ST/DOS/C=64 versions of Dyna Blaster as a sort of mandatory
dongle (i.e. game menus needs joy 3 in Amiga version at least).

List of known supported games:
amigaocs_flop
- dynabls;
- kickoff2;
- gauntlt2;
- protent2;
- sskid;

TODO:
- DOS ct486 dynablst doesn't work, BIOS shenanigans?
- atarist (cracked only, loose) Dyna Blaster doesn't work either, needs select and data in routing;
- Untested on C=64;
- gauntlt2 seemingly requires a slightly different pinout according to the Super Skidmarks
  manual "connect pin 6 of joy 3 to pin 13 (?), pin 6 of joy 4 to pin 12";
- Anything that isn't Atari/Commodore single button joystick is uncharted waters at current time
  (read: no SW pretends to read a mouse or a MD pad with this);

References:
- https://www.aminet.net/package/util/misc/ControllerTest technical documentation;
- https://www.aminet.net/package/util/misc/VATestprogram MouseJoy test;
- Super Skidmarks manual, page 3;

**************************************************************************************************/

#include "emu.h"
#include "adaptator.h"

DEFINE_DEVICE_TYPE(ADAPTATOR_MULTITAP, adaptator_multitap_device, "adaptator_multitap", "The Adaptator 2x DE-9 Multitap")

adaptator_multitap_device::adaptator_multitap_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ADAPTATOR_MULTITAP, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this),
	m_joy(*this, "joy_p%u", 1U)
{ }


void adaptator_multitap_device::device_add_mconfig(machine_config &config)
{
	VCS_CONTROL_PORT(config, m_joy[0], vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, m_joy[1], vcs_control_port_devices, "joy");
}

void adaptator_multitap_device::device_start()
{
	save_item(NAME(m_ddr));
}

void adaptator_multitap_device::input_strobe(int state)
{
	// assume 1 -> 0, assume writing to the data port causes pullup
	// i.e. ControllerTest just writes a 0xff, at init time. ct486 do the same at POST.
	if (state)
		return;

	u8 p1_in = m_joy[0]->read_joy();
	u8 p2_in = m_joy[1]->read_joy();

	// route pin 13 -> joy port 3 pin 6
	output_select(BIT(p1_in, 5));
	// route pin 11 -> joy port 4 pin 6
	output_busy(BIT(p2_in, 5));
	// pins 18-22 -> pin 8 ground for both

	// NOTE: 2nd button hooks are possible but ControllerTest warns that ACK
	// "is not easily available to software without some fancy interrupt trickery"
	// so it doesn't support it.
	// route pin 12 (pout) -> joy port 3 pin 9
	//output_perror(BIT(p1_in, ?));
	// route pin 10 (ack) -> joy port 4 pin 9
	//output_ack(BIT(p2_in, ?));

	// route pins 2-5 -> joy port 3 pins 1-4
	output_data0(BIT(p1_in, 0));
	output_data1(BIT(p1_in, 1));
	output_data2(BIT(p1_in, 2));
	output_data3(BIT(p1_in, 3));
	// route pins 6-9 -> joy port 4 pins 1-4
	output_data4(BIT(p2_in, 0));
	output_data5(BIT(p2_in, 1));
	output_data6(BIT(p2_in, 2));
	output_data7(BIT(p2_in, 3));
}
