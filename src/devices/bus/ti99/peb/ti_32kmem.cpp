// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 32 KiB Memory Expansion Card
    This is a fairly simple memory expansion for the TI-99/4A, adding
    unbuffered 32 KiB. Yet it was a very popular card since it was
    required for any kind of advanced programming beyond the console BASIC.

    As a peripheral box card, it is connected via the 8-bit multiplexed data bus.
    Later, modifications of the console became increasingly popular which
    avoided the bus multiplex so that the full 16bit access was possible.
    This helped to noticeably speed up the system.

    The memory is available on the addresses

    0x2000 - 0x3fff ("low memory")
    0xa000 - 0xffff ("high memory")

    The console TI BASIC is not able to access the memory expansion, but
    Extended Basic (available as a cartridge) makes use of it.

    Michael Zapf
    February 2012: Rewritten as class

    References
    [1] Michael L. Bunyard: Hardware Manual for the Texas Instruments 99/4A Home Computer, chapter 8

*****************************************************************************/

#include "emu.h"
#include "ti_32kmem.h"

DEFINE_DEVICE_TYPE(TI99_32KMEM, bus::ti99::peb::ti_32k_expcard_device, "ti99_32kmem", "TI-99 32KiB memory expansion card")

namespace bus::ti99::peb {

#define RAMREGION "ram32k"

ti_32k_expcard_device::ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_32KMEM, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_ram(*this, RAMREGION)
{
}

void ti_32k_expcard_device::readz(offs_t offset, uint8_t *value)
{
	uint8_t val = 0;
	/*
	    The problem for mapping the memory into the address space is that
	    we have a block at 2000-3FFF and another one at A000-FFFF. The trick
	    is to calculate a bank number in a way to get subsequent address
	    This is done in a PAL on the expansion board [1].

	                    S AB
	    0000: 000x  ->  0 xx
	    2000: 001x  ->  1 01
	    4000: 010x  ->  0 xx
	    6000: 011x  ->  0 xx
	    8000: 100x  ->  0 xx
	    A000: 101x  ->  1 00
	    C000: 110x  ->  1 11
	    E000: 111x  ->  1 10

	    select = A0*A1 + /A1*A2   (A0 = MSB)
	    A = A1
	    B = A0 nand A2
	*/

	bool select = ((offset & 0xfc000)==0x7c000) | ((offset & 0xf6000)==0x72000); // PAL output pin 14 [1]

	if (select)
	{
		// address = 0abx xxxx xxxx xxxx
		int bank = (offset & 0x4000) | ((((offset & 0x8000)>>2) & (offset & 0x2000)) ^ 0x2000);
		val = m_ram->pointer()[bank | (offset & 0x1fff)];

		// On powerup we find a FF00FF00... pattern in RAM
		if ((offset & 1)==0) val = ~val;
		*value = val;
	}
}

void ti_32k_expcard_device::write(offs_t offset, uint8_t data)
{
	bool select = ((offset & 0xfc000)==0x7c000) | ((offset & 0xf6000)==0x72000); // PAL output pin 14 [1]

	if (select)
	{
		// address = 0abx xxxx xxxx xxxx
		int bank = (offset & 0x4000) | ((((offset & 0x8000)>>2) & (offset & 0x2000)) ^ 0x2000);
		if ((offset & 1)==0) data = ~data;
		m_ram->pointer()[bank | (offset & 0x1fff)] = data;
	}
}


void ti_32k_expcard_device::device_start()
{
}

void ti_32k_expcard_device::device_add_mconfig(machine_config &config)
{
	RAM(config, m_ram, 0);
	m_ram->set_default_size("32k");
	m_ram->set_default_value(0);
}

} // end namespace bus::ti99::peb
