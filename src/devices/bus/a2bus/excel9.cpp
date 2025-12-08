// license:BSD-3-Clause
// copyright-holders:R. Belmont, Rob Justice
/*********************************************************************

    excel9.cpp
    Implementation of the Seikou Excel-9 6809 card

    Address mapping as follows:
    6809 0x0000-0x0fff -> 6502 0x9000-0x9fff
    6809 0x1000-0x8fff -> 6502 0x1000-0x8fff
    6809 0x9000-0x9fff -> 6502 0x0000-0x0fff
    6809 0xa000-0xbfff -> 6502 0xc000-0xdfff
    6809 0xc000-0xdfff -> 6502 0xa000-0xbfff
    6809 0xe000-0xffff -> 6502 0xe000-0xffff

    Eproms/Proms and Schematic available here:
    http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/CPU/Seikou%20Excel-9/

    Todo:
    Add timer support, board has a CD4536B timer onboard that can generate interrupts

*********************************************************************/

#include "emu.h"
#include "excel9.h"
#include "cpu/m6809/m6809.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_EXCEL9, a2bus_excel9_device, "a2excel9", "Seikou Excel-9")

void a2bus_excel9_device::m6809_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(a2bus_excel9_device::dma_r), FUNC(a2bus_excel9_device::dma_w));
}

ROM_START( excel9 )
	ROM_REGION(0x2000, "m6809_rom", 0)
	ROM_LOAD( "e000.bin", 0x000000, 0x001000, CRC(f12f400d) SHA1(fde86ba8d1b9ffe08b13774a73b17a5f44990d7a) )
	ROM_LOAD( "f000.bin", 0x001000, 0x001000, CRC(52484440) SHA1(d2960851ce8f70df12d23021dfbe031ec5a8988f) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_excel9_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_6809, 1021800);   // 6809E runs at ~1 MHz
	m_6809->set_addrmap(AS_PROGRAM, &a2bus_excel9_device::m6809_mem);
}

//-------------------------------------------------
//  device_rom_region - device-specific ROMs
//-------------------------------------------------

const tiny_rom_entry *a2bus_excel9_device::device_rom_region() const
{
	return ROM_NAME( excel9 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_excel9_device::a2bus_excel9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_6809(*this, "m6809")
	, m_rom(*this, "m6809_rom")
	, m_bEnabled(false)
	, m_romenable(false)
	, m_enable_flipflop(false)
	, m_interrupttype_firq(false)
	, m_powerup(false)
	, m_status(0)
{
}

a2bus_excel9_device::a2bus_excel9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_excel9_device(mconfig, A2BUS_EXCEL9, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_excel9_device::device_start()
{
	save_item(NAME(m_bEnabled));
	save_item(NAME(m_romenable));
	save_item(NAME(m_enable_flipflop));
	save_item(NAME(m_status));
	save_item(NAME(m_interrupttype_firq));
	save_item(NAME(m_powerup));
}

void a2bus_excel9_device::device_reset()
{
	reset_from_bus();
}

void a2bus_excel9_device::reset_from_bus()
{
	m_bEnabled = true;
	m_romenable = true;
	m_enable_flipflop = false;
	m_status = 0;
	m_interrupttype_firq = false;
	m_powerup = true;
	raise_slot_dma();
	m_6809->reset();
}

uint8_t a2bus_excel9_device::read_c0nx(uint8_t offset)
{
	// Bit7 timer output, 1 = timer fired
	// Bit6
	// Bit5
	// Bit4 BA
	// Bit3 BS
	// Bit2 rom enable 0=enable 1=disable
	// Bit1 expansion connector pin34
	// Bit0 interrupttype 0=6502 IRQ & 6809 IRQ, 1=6809 FIRQ

	//Todo: add timer output, ba and bs
	return m_status;
}

void a2bus_excel9_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_enable_flipflop = false;
			break;

		case 1:
			m_enable_flipflop = true;
			break;

		case 2: // enable 6809
		case 3:
			raise_slot_dma();
			m_6809->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_bEnabled = true;
			break;

		case 4: // disable 6809
		case 5:
			m_6809->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			lower_slot_dma();
			m_bEnabled = false;
			break;

		case 6: // Set enable_flipflop state and trigger NMI
		case 7:
			if (m_enable_flipflop)
			{
				raise_slot_dma();
				m_6809->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
				m_6809->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
				m_bEnabled = true;
			}
			else
			{
				m_6809->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
				lower_slot_dma();
				if (m_powerup) // hack for initial 6502 RESET taking priority over the NMI
				{
					m_powerup = false;
				}
				else
				{
					raise_slot_nmi();
					lower_slot_nmi();
				}
				m_bEnabled = false;
			}
			break;

		case 8: // Timer control for CD4536B
		case 9:
				//to be added
			break;

		case 0xa: // status reg
		case 0xb:
			if (data & 0x04)
			{
				m_romenable=false;
				m_status |= 0x04;
			}
			else
			{
				m_romenable=true;
				m_status &= ~0x04;
			}

			if (data & 0x02) // this is an output to the expansion connector
			{
				m_status |= 0x02;
			}
			else
			{
				m_status &= ~0x02;
			}

			if (data & 0x01) // select interrupt type generated from timer
			{
				m_interrupttype_firq=true;
				m_status |= 0x01;
			}
			else
			{
				m_interrupttype_firq=false;
				m_status &= ~0x01;
			}
			break;

		default:
			logerror("Excel-9: %02x to unhandled c0n%x\n", data, offset);
			break;
	}
}

uint8_t a2bus_excel9_device::dma_r(offs_t offset)
{
	if (m_bEnabled)
	{
		if (offset <= 0x0fff)
		{
			return slot_dma_read(offset+0x9000);
		}
		else if (offset <= 0x8fff)
		{
			return slot_dma_read(offset);
		}
		else if (offset <= 0x9fff)
		{
			return slot_dma_read(offset&0xfff);
		}
		else if (offset <= 0xbfff)
		{
			return slot_dma_read(offset+0x2000);
		}
		else if (offset <= 0xdfff)
		{
			return slot_dma_read(offset-0x2000);
		}
		else
		{
			if (m_romenable)
			{
				return m_rom[offset & 0x1fff];
			}
			else
			{
				return slot_dma_read(offset);
			}
		}
	}
	return 0xff;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

void a2bus_excel9_device::dma_w(offs_t offset, uint8_t data)
{
	if (m_bEnabled)
	{
		if (offset <= 0x0fff)
		{
			slot_dma_write(offset+0x9000, data);
		}
		else if (offset <= 0x8fff)
		{
			slot_dma_write(offset, data);
		}
		else if (offset <= 0x9fff)
		{
			slot_dma_write(offset&0xfff, data);
		}
		else if (offset <= 0xbfff)
		{
			slot_dma_write(offset+0x2000, data);
		}
		else if (offset <= 0xdfff)
		{
			slot_dma_write(offset-0x2000, data);
		}
		else
		{
			if (!m_romenable)
			{
				slot_dma_write(offset, data);
			}
		}
	}
}
