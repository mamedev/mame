// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Videopac Service Test cartridge emulation

It's a standard 2KB ROM, the difference is that the PCB has a 7seg LED on it.
The program will still work fine if the 7seg is not hooked up (eg. O2_ROM_STD).

Test 1: press all joystick inputs
Test 4: type "NAGEL137"
(the other initial tests are automatic)

Hold UP to advance to next test.

******************************************************************************/

#include "emu.h"
#include "test.h"

DEFINE_DEVICE_TYPE(O2_ROM_TEST, o2_test_device, "o2_test", "Videopac Service Test Cartridge")


//-------------------------------------------------
//  o2_test_device - constructor
//-------------------------------------------------

o2_test_device::o2_test_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, O2_ROM_TEST, tag, owner, clock),
	device_o2_cart_interface(mconfig, *this),
	m_digit_out(*this, "tc_digit")
{ }

void o2_test_device::device_start()
{
	m_digit_out.resolve();

	save_item(NAME(m_control));
	save_item(NAME(m_bus_data));
}

void o2_test_device::cart_init()
{
	if (m_rom_size != 0x800)
		fatalerror("o2_test_device: ROM size must be 2KB\n");
}


//-------------------------------------------------
//  mapper specific handlers
//-------------------------------------------------

void o2_test_device::write_p1(u8 data)
{
	u8 output = m_digit_out;

	// P10: latch digit segments
	if (m_control & ~data & 1)
		output = ~m_bus_data;

	// P11: digit DP
	m_digit_out = (output & 0x7f) | (~data << 6 & 0x80);

	m_control = data;
}
