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
#include "ti32kmem.h"

#define RAMREGION "ram"

ti_32k_expcard_device::ti_32k_expcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI_32KMEM, "TI-99 32KiB memory expansion card", tag, owner, clock)
{
	m_shortname = "ti99_32k";
}

READ8Z_MEMBER(ti_32k_expcard_device::readz)
{
	switch((offset & 0xe000)>>13)
	{
	case 1:
		*value = m_ram_ptr[offset & 0x1fff];
		break;
	case 5:
		*value = m_ram_ptr[(offset & 0x1fff) | 0x2000];
		break;
	case 6:
		*value = m_ram_ptr[(offset & 0x1fff) | 0x4000];
		break;
	case 7:
		*value = m_ram_ptr[(offset & 0x1fff) | 0x6000];
		break;
	default:
		break;
	}
}

WRITE8_MEMBER(ti_32k_expcard_device::write)
{
	switch((offset & 0xe000)>>13)
	{
	case 1:
		m_ram_ptr[offset & 0x1fff] = data;
		break;
	case 5:
		m_ram_ptr[(offset & 0x1fff) | 0x2000] = data;
		break;
	case 6:
		m_ram_ptr[(offset & 0x1fff) | 0x4000] = data;
		break;
	case 7:
		m_ram_ptr[(offset & 0x1fff) | 0x6000] = data;
		break;
	default:
		break;
	}
}

/*
    32K memory expansion does not have a CRU interface
*/
void ti_32k_expcard_device::crureadz(offs_t offset, UINT8 *value)
{
	return;
}

void ti_32k_expcard_device::cruwrite(offs_t offset, UINT8 value)
{
	return;
}

void ti_32k_expcard_device::device_start(void)
{
	m_ram_ptr = memregion(RAMREGION)->base();
	m_cru_base = 0;
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
