// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    TI-99 Myarc memory expansion MEXP-1

    The card features 128 KiB of RAM, not buffered. In the TI-99/4A address
    space, RAM is located at 2000-3fff and a000-ffff (32 KiB). Using the CRU
    interface, four banks of 32K each can be made visible in this area.

    We also emulate the 512 KiB version of this card; it works in the same way
    but offers two more CRU bits for a total of 16 banks of 32 KiB RAM.

    Beside the RAM, the card also contains ROM with a DSR (Device service
    routine, TI's term for the firmware). This 8 KiB ROM allows for
    testing and partitioning the card, and it introduces new devices which
    are make available to BASIC programs, like a RAMdisk device.

    There also was a 32 KiB version which did not contain a DSR; thus, it was
    equivalent to a standard TI 32 KiB memory expansion and is not emulated
    here.

    The firmware to be used with this card is a version that is tailored to
    work with Myarc Extended Basic II.

    Michael Zapf
    February 2012: rewritten as a class, adding DSR support

****************************************************************************/
#include "emu.h"
#include "myarcmem.h"

DEFINE_DEVICE_TYPE(TI99_MYARCMEM, bus::ti99::peb::myarc_memory_expansion_device, "ti99_myarcmem", "Myarc Memory expansion card MEXP-1")

namespace bus::ti99::peb {

/* This card has two CRU bases where it answers. */
#define MYARCMEM_CRU_BASE1 0x1000
#define MYARCMEM_CRU_BASE2 0x1900

enum
{
	SIZE_128,
	SIZE_512
};

myarc_memory_expansion_device::myarc_memory_expansion_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, TI99_MYARCMEM, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_ram(*this, RAM_TAG),
	m_dsrrom(nullptr), m_bank(0), m_size(0)
{
}

int myarc_memory_expansion_device::get_base(int offset)
{
	int base;
	if (m_size == SIZE_128)
	{
		base = ((m_bank & 0x03) << 15);
	}
	else
	{
		base = (m_bank << 15);
	}
	base |= (offset & 0x1fff);
	return base;
}

/*
    Memory read access.
    RAM is at 2000-3fff, a000-ffff;
    ROM is at 4000-5fff (if CRU bit 0 is set)
*/
void myarc_memory_expansion_device::readz(offs_t offset, uint8_t *value)
{
	int base = get_base(offset);

	switch((offset & 0xe000)>>13)
	{
	case 1:
		*value = m_ram->pointer()[base];
		break;
	case 2:
		if (m_selected) *value = m_dsrrom[offset & 0x1fff];
		break;
	case 5:
		*value = m_ram->pointer()[base | 0x2000];
		break;
	case 6:
		*value = m_ram->pointer()[base | 0x4000];
		break;
	case 7:
		*value = m_ram->pointer()[base | 0x6000];
		break;
	default:
		break;
	}
}

/*
    Memory write access. DSRROM does not allow writing.
*/
void myarc_memory_expansion_device::write(offs_t offset, uint8_t data)
{
	int base = get_base(offset);

	switch((offset & 0xe000)>>13)
	{
	case 1:
		m_ram->pointer()[base] = data;
		break;
	case 5:
		m_ram->pointer()[base | 0x2000] = data;
		break;
	case 6:
		m_ram->pointer()[base | 0x4000] = data;
		break;
	case 7:
		m_ram->pointer()[base | 0x6000] = data;
		break;
	default:
		break;
	}
}

/*
    CRU read. None here.
*/
void myarc_memory_expansion_device::crureadz(offs_t offset, uint8_t *value)
{
}

/*
    CRU write. Bit 0 turns on the DSR (firmware), bits 1-3 are used to select
    one of several 32K RAM banks.

    Select bits
        1000 = DSRROM seen on 4000-5fff (128, 512K)
        1002 = bit 0 of RAM bank value (128, 512K)
        1004 = bit 1 of RAM bank value (128, 512K)
        1006 = bit 2 of RAM bank value (512K)
        1008 = bit 3 of RAM bank value (512K)
*/
void myarc_memory_expansion_device::cruwrite(offs_t offset, uint8_t data)
{
	if (((offset & 0xff00)==MYARCMEM_CRU_BASE1)||((offset & 0xff00)==MYARCMEM_CRU_BASE2))
	{
		if ((offset & 0x000e)==0)
		{
			// Turn on/off DSR
			m_selected = (data!=0);
		}
		else
		{
			// xxxx xxxx xxxx 0010
			// xxxx xxxx xxxx 0100
			// xxxx xxxx xxxx 0110
			// xxxx xxxx xxxx 1000
			int bankbit = 1 << (((offset & 0x000e)>>1)-1);

			if (data==0)
				m_bank &= ~bankbit;
			else
				m_bank |= bankbit;
		}
	}
}

void myarc_memory_expansion_device::device_start()
{
	m_dsrrom = memregion(TI99_DSRROM)->base();
	save_item(NAME(m_bank));
	save_item(NAME(m_size));
}

void myarc_memory_expansion_device::device_reset()
{
	m_size = ioport("SIZE")->read();

	// Resetting values
	m_bank = 0;
	m_selected = false;
}

INPUT_PORTS_START( myarc_exp )
	PORT_START( "SIZE" )
	PORT_CONFNAME( 0x01, SIZE_512, "Myarc memory card size" )
		PORT_CONFSETTING( SIZE_128, "128 KiB")
		PORT_CONFSETTING( SIZE_512, "512 KiB")
INPUT_PORTS_END

ROM_START( myarc_exp )
	ROM_REGION(0x2000, TI99_DSRROM, 0)
	ROM_LOAD("myarc512k_xb2_dsr.bin", 0x0000, 0x2000, CRC(41fbb96d) SHA1(4dc7fdfa46842957bcbb0cf2c37764e4bb6d877a)) /* DSR for Ramdisk etc. */
ROM_END

void myarc_memory_expansion_device::device_add_mconfig(machine_config &config)
{
	RAM(config, RAM_TAG).set_default_size("512K").set_default_value(0);
}


const tiny_rom_entry *myarc_memory_expansion_device::device_rom_region() const
{
	return ROM_NAME( myarc_exp );
}

ioport_constructor myarc_memory_expansion_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(myarc_exp);
}

} // end namespace bus::ti99::peb
