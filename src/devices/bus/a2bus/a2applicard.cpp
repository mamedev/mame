// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2applicard.c

    Implementation of the PCPI AppliCard Z-80 card

    Unlike the SoftCard and clones, this has its own 64k of RAM on board
    and the Z80 runs completely independently of the host's 6502.

*********************************************************************/

#include "emu.h"
#include "a2applicard.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_APPLICARD, a2bus_applicard_device, "a2aplcrd", "PCPI Applicard")

#define Z80_TAG         "z80"
#define Z80_ROM_REGION  "z80_rom"

void a2bus_applicard_device::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(a2bus_applicard_device::dma_r), FUNC(a2bus_applicard_device::dma_w));
}

void a2bus_applicard_device::z80_io(address_map &map)
{
	map(0x00, 0x60).mirror(0xff00).rw(FUNC(a2bus_applicard_device::z80_io_r), FUNC(a2bus_applicard_device::z80_io_w));
}

ROM_START( a2applicard )
	ROM_REGION(0x800, Z80_ROM_REGION, 0)
	ROM_LOAD( "applicard-v9.bin", 0x000000, 0x000800, CRC(1d461000) SHA1(71d633be864b6084362e85108a4e600cbe6e44fe) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_applicard_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 6000000); // Z80 runs at 6 MHz
	m_z80->set_addrmap(AS_PROGRAM, &a2bus_applicard_device::z80_mem);
	m_z80->set_addrmap(AS_IO, &a2bus_applicard_device::z80_io);
}

//-------------------------------------------------
//  device_rom_region - device-specific ROMs
//-------------------------------------------------

const tiny_rom_entry *a2bus_applicard_device::device_rom_region() const
{
	return ROM_NAME( a2applicard );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_applicard_device::a2bus_applicard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_z80(*this, Z80_TAG), m_bROMAtZ80Zero(false), m_z80stat(false), m_6502stat(false), m_toz80(0), m_to6502(0), m_z80rom(nullptr)
{
}

a2bus_applicard_device::a2bus_applicard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_applicard_device(mconfig, A2BUS_APPLICARD, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_applicard_device::device_start()
{
	// locate Z80 ROM
	m_z80rom = device().machine().root_device().memregion(this->subtag(Z80_ROM_REGION).c_str())->base();

	save_item(NAME(m_bROMAtZ80Zero));
	save_item(NAME(m_z80stat));
	save_item(NAME(m_6502stat));
	save_item(NAME(m_toz80));
	save_item(NAME(m_to6502));
	save_item(NAME(m_z80ram));

	memset(m_z80ram, 0, 64*1024);
}

void a2bus_applicard_device::device_reset()
{
	m_bROMAtZ80Zero = true;
	m_z80stat = false;
}

uint8_t a2bus_applicard_device::read_c0nx(uint8_t offset)
{
	switch (offset & 0xf)
	{
		case 0:
			m_6502stat = false;
			return m_to6502;

		case 1:
			return m_toz80;

		case 2:
			if (m_z80stat)
			{
				return 0x80;
			}
			return false;

		case 3:
			if (m_6502stat)
			{
				return 0x80;
			}
			return false;

		case 5:
			m_bROMAtZ80Zero = true;
			m_toz80 = false;
			m_to6502 = false;
			m_z80->reset();
			break;

		case 6: // IRQ on Z80 via CTC channel 3 (CP/M doesn't use the CTC or IRQs)
			fatalerror("Applicard: Z80 IRQ not supported yet\n");

		case 7: // NMI on Z80 (direct)
			m_z80->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			break;

	}
	return 0xff;
}

void a2bus_applicard_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset & 0xf)
	{
		case 0: // are these legal to write?
		case 2:
		case 3:
			break;

		case 1:
			m_z80stat = true;
			m_toz80 = data;
			break;

		case 5:
		case 6:
		case 7:
			read_c0nx(offset);   // let the read handler take care of these
			break;
	}
}

READ8_MEMBER( a2bus_applicard_device::z80_io_r )
{
	uint8_t tmp = 0;

	switch (offset)
	{
		case 0:
			return m_to6502;

		case 0x20:
			m_z80stat = false;
			return m_toz80;

		case 0x40:
			if (m_z80stat)
			{
				tmp |= 0x80;
			}
			if (m_6502stat)
			{
				tmp |= 1;
			}
			return tmp;

		case 0x60:
			break;
	}
	return 0xff;
}

WRITE8_MEMBER( a2bus_applicard_device::z80_io_w )
{
	switch (offset)
	{
		case 0:
			m_to6502 = data;
			m_6502stat = true;
			break;

		case 0x60:
			if (data & 1)
			{
				m_bROMAtZ80Zero = true;
			}
			else
			{
				m_bROMAtZ80Zero = false;
			}
			break;
	}
}

//-------------------------------------------------
//  dma_r -
//-------------------------------------------------

READ8_MEMBER( a2bus_applicard_device::dma_r )
{
	if (offset < 0x8000)
	{
		if (m_bROMAtZ80Zero)
		{
			return m_z80rom[offset & 0x7ff];
		}
		else
		{
			return m_z80ram[offset];
		}
	}
	else
	{
		return m_z80ram[offset];
	}
	// never executed
	//return 0xff;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

WRITE8_MEMBER( a2bus_applicard_device::dma_w )
{
	if (offset < 0x8000)
	{
		// writing only works if ROM not mapped from 0-7fff
		if (!m_bROMAtZ80Zero)
		{
			m_z80ram[offset] = data;
		}
	}
	else
	{
		m_z80ram[offset] = data;
	}
}

bool a2bus_applicard_device::take_c800()
{
	return false;
}
