// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 32 KiB Memory Expansion Card
    This is a fairly simple memory expansion for the TI-99/4A, adding
    unbuffered 32 KiB. Yet it was a very popular card since it was
    required for any kind of advanced programming beyond the console BASIC.

    As a peripheral box card, it is connected via the 8-bit mutiplexed data bus.
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
#include "peribox.h"
#include "ti_32kmem.h"

#define RAMREGION "ram32k"

ti_32k_expcard_device::ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI_32KMEM, "TI-99 32KiB memory expansion card", tag, owner, clock, "ti99_32k", __FILE__),
	m_ram(*this, RAMREGION)
{
}

READ8Z_MEMBER(ti_32k_expcard_device::readz)
{
	UINT8 val = 0;
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

WRITE8_MEMBER(ti_32k_expcard_device::write)
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


void ti_32k_expcard_device::device_start(void)
{
}

MACHINE_CONFIG_FRAGMENT( mem32k )
	MCFG_RAM_ADD(RAMREGION)
	MCFG_RAM_DEFAULT_SIZE("32k")
	MCFG_RAM_DEFAULT_VALUE(0)
MACHINE_CONFIG_END

machine_config_constructor ti_32k_expcard_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( mem32k );
}

const device_type TI_32KMEM = &device_creator<ti_32k_expcard_device>;
