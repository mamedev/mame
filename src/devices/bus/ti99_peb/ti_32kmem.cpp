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

*****************************************************************************/

#include "emu.h"
#include "peribox.h"
#include "ti_32kmem.h"

#define RAMREGION "ram"

ti_32k_expcard_device::ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI_32KMEM, "TI-99 32KiB memory expansion card", tag, owner, clock, "ti99_32k", __FILE__), m_ram_ptr(nullptr)
{
}

READ8Z_MEMBER(ti_32k_expcard_device::readz)
{
	UINT8 val = 0;
	bool access = true;
	switch((offset & 0xe000)>>13)
	{
	case 1:
		val = m_ram_ptr[offset & 0x1fff];
		break;
	case 5:
		val = m_ram_ptr[(offset & 0x1fff) | 0x2000];
		break;
	case 6:
		val = m_ram_ptr[(offset & 0x1fff) | 0x4000];
		break;
	case 7:
		val = m_ram_ptr[(offset & 0x1fff) | 0x6000];
		break;
	default:
		access = false;
		break;
	}
	if (access)
	{
		// There is no evidence for an inverted write on the even addresses;
		// we assume that the FF00 byte sequence in this memory is a power-on
		// artifact.

		/* if ((offset&1)!=1) *value = ~val;
		else */
		*value = val;
	}
}

WRITE8_MEMBER(ti_32k_expcard_device::write)
{
	UINT8 data1 = data;
	// if ((offset&1)!=1) data1 = ~data;
	switch((offset & 0xe000)>>13)
	{
	case 1:
		m_ram_ptr[offset & 0x1fff] = data1;
		break;
	case 5:
		m_ram_ptr[(offset & 0x1fff) | 0x2000] = data1;
		break;
	case 6:
		m_ram_ptr[(offset & 0x1fff) | 0x4000] = data1;
		break;
	case 7:
		m_ram_ptr[(offset & 0x1fff) | 0x6000] = data1;
		break;
	default:
		break;
	}
}

void ti_32k_expcard_device::device_start(void)
{
	m_ram_ptr = memregion(RAMREGION)->base();
	m_cru_base = 0;
	// See above. Preset the memory with FF00
	// ROM_FILL does not seem to allow filling with an alternating pattern
	for (int i=0; i < 0x8000; i+=2)
	{
		m_ram_ptr[i] = (UINT8)0xff;
	}
}

ROM_START( ti_exp_32k )
	ROM_REGION(0x8000, RAMREGION, 0)
	ROM_FILL(0x0000, 0x8000, 0x00)
ROM_END

const rom_entry *ti_32k_expcard_device::device_rom_region() const
{
	return ROM_NAME( ti_exp_32k );
}

const device_type TI_32KMEM = &device_creator<ti_32k_expcard_device>;
