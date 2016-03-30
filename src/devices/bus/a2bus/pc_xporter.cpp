// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    pcxporter.cpp

    Implementation of the Applied Engineering PC Transporter card
    Preliminary version by R. Belmont

    The PC Transporter is basically a PC-XT on an Apple II card.
    Features include:
    - V30 CPU @ 4.77 MHz
    - 768K of RAM, which defines the V30 address space from 0x00000 to 0xBFFFF
      and is fully read/writable by the Apple's CPU.
    - Usual XT hardware, mostly inside custom ASICs.  There's a discrete
      NEC uPD71054 (i8254-compatible PIT) though.
    - CGA-compatible video, output to a separate CGA monitor or NTSC-compliant analog
      RGB monitor (e.g. the IIgs RGB monitor).
    - XT-compatible keyboard interface.
    - PC-style floppy controller: supports 360K 5.25" disks and 720K 3.5" disks
    - HDD controller which is redirected to a file on the Apple's filesystem

    The V30 BIOS is downloaded by the Apple; the Apple also writes text to the CGA screen prior to
    the V30's being launched.

    The board was developed by The Engineering Department, a company made up mostly of early Apple
    engineers including Apple /// designer Dr. Wendall Sander and ProDOS creator Dick Huston.

    Software and user documentation at:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/CPU/AE%20PC%20Transporter/

    Notes:
        Registers live at CFxx; access CFFF to clear C800 reservation,
        then read Cn00 to map C800-CFFF first.

        PC RAM from 0xA0000-0xAFFFF is where the V30 BIOS is downloaded,
        plus used for general storage by the system.
        RAM from 0xB0000-0xBFFFF is the CGA framebuffer as usual.

        CF00: PC memory pointer (bits 0-7)
        CF01: PC memory pointer (bits 8-15)
        CF02: PC memory pointer (bits 16-23)
        CF03: read/write PC memory at the pointer and increment the pointer
        CF04: read/write PC memory at the pointer and *don't* increment the pointer

    TODO:
        - A2 probably also can access the V30's I/O space: where's that at?  CF0E/CF0F
          are suspects...
        - There's likely A2 ROM at CnXX and C800-CBFF to support the "Slinky" memory
          expansion card emulation function inside one of the custom ASICs.  Need to
          dump this...

*********************************************************************/

#include "pc_xporter.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_PCXPORTER = &device_creator<a2bus_pcxporter_device>;

static MACHINE_CONFIG_FRAGMENT( pcxporter )
MACHINE_CONFIG_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_pcxporter_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcxporter );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_pcxporter_device::a2bus_pcxporter_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this)
{
}

a2bus_pcxporter_device::a2bus_pcxporter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_PCXPORTER, "Applied Engineering PC Transporter", tag, owner, clock, "a2pcxport", __FILE__),
	device_a2bus_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_pcxporter_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	memset(m_ram, 0, 768*1024);
	memset(m_regs, 0, 0x400);
	m_offset = 0;

	save_item(NAME(m_ram));
	save_item(NAME(m_regs));
	save_item(NAME(m_offset));
}

void a2bus_pcxporter_device::device_reset()
{
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_pcxporter_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		default:
			printf("Read c0n%x (PC=%x)\n", offset, space.device().safe_pc());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_pcxporter_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	switch (offset)
	{
		default:
			printf("Write %02x to c0n%x (PC=%x)\n", data, offset, space.device().safe_pc());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_pcxporter_device::read_cnxx(address_space &space, UINT8 offset)
{
	// read only to trigger C800?
	return 0xff;
}

void a2bus_pcxporter_device::write_cnxx(address_space &space, UINT8 offset, UINT8 data)
{
	printf("Write %02x to cn%02x (PC=%x)\n", data, offset, space.device().safe_pc());
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

UINT8 a2bus_pcxporter_device::read_c800(address_space &space, UINT16 offset)
{
//  printf("Read C800 at %x\n", offset + 0xc800);

	if (offset < 0x400)
	{
		return 0xff;
	}
	else
	{
		UINT8 rv;

		switch (offset)
		{
			case 0x700:
				return m_offset & 0xff;

			case 0x701:
				return (m_offset >> 8) & 0xff;

			case 0x702:
				return (m_offset >> 16) & 0xff;

			case 0x703: // read with increment
				rv = m_ram[m_offset];
				m_offset++;
				return rv;

			case 0x704: // read w/o increment
				rv = m_ram[m_offset];
				return rv;
		}

		return m_regs[offset];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_pcxporter_device::write_c800(address_space &space, UINT16 offset, UINT8 data)
{
	if (offset < 0x400)
	{
	}
	else
	{
		switch (offset)
		{
			case 0x700:
				m_offset &= ~0xff;
				m_offset |= data;
//              printf("offset now %x (PC=%x)\n", m_offset, space.device().safe_pc());
				break;

			case 0x701:
				m_offset &= ~0xff00;
				m_offset |= (data<<8);
//              printf("offset now %x (PC=%x)\n", m_offset, space.device().safe_pc());
				break;

			case 0x702:
				m_offset &= ~0xff0000;
				m_offset |= (data<<16);
//              printf("offset now %x (PC=%x)\n", m_offset, space.device().safe_pc());
				break;

			case 0x703: // write w/increment
				m_ram[m_offset] = data;
				m_offset++;
				break;

			case 0x704: // write w/o increment
				m_ram[m_offset] = data;
				break;

			default:
				printf("%02x to C800 at %x\n", data, offset + 0xc800);
				m_regs[offset] = data;
				break;
		}
	}
}
