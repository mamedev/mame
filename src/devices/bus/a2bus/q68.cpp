// license:BSD-3-Clause
// copyright-holders:Rob Justice, R. Belmont
/*********************************************************************

    q68.cpp

    Implementation of the Stellation Q-68 68008 card

    Memory Map
    68008 0x00000-0x0ffff -> Apple II memory
    68008 0x10000-0x17fff -> Onboard EPROM, 8k (expandable to 32k)
    68008 0x18000-0x1ffff -> Onboard RAM, 8K (expandable to 32k?)
    68008 0x20000-0xfffff -> Open, for options via expansion connector

    Apple II Memory
    68008 0x00000-0x003ff -> 6502 0x0800-0x0bff
    68008 0x00400-0x007ff -> 6502 0x0400-0x07ff
    68008 0x00800-0x00bff -> 6502 0x0000-0x03ff
    68008 0x00c00-0x0ffff -> 6502 0x0c00-0xffff

    0xc0n1 - Turn card on
    0xc0n3 - Interrupt card
    0xc0n0 - Turn card off

    Also includes the Q-68 plus version made by nanja.info
    This has a full 1MB of ram on board and slight update to the memory map
    Memory Map
    68008 0x00000-0x0ffff -> Apple II memory
    68008 0x10000-0x11fff -> Onboard EPROM, 8k
    68008 0x12000-0x1ffff -> Onboard RAM, ~1MB

    Todo:
    Implement Watchdog timer. Only relevant to Q-68 and bus errors for nonexistent memory access
    Speed of 6502 should be slower when 68008 is enabled and accessing AppleII memory space
    Speed of 68008 should be slower when access the Apple II memory space

    (reference: http://mirrors.apple2.org.za/ftp.apple.asimov.net/unsorted/QPAK-68%20Users%20Guide.pdf )

*********************************************************************/

#include "emu.h"
#include "q68.h"

#include "cpu/m68000/m68008.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_Q68, a2bus_q68_device, "a2q68", "Stellation Two Q-68")
DEFINE_DEVICE_TYPE(A2BUS_Q68PLUS, a2bus_q68plus_device, "a2q68plus", "Stellation Two Q-68 Plus")

void a2bus_q68_device::m68008_mem(address_map &map)
{
	map(0x00000, 0x0ffff).rw(FUNC(a2bus_q68_device::dma_r), FUNC(a2bus_q68_device::dma_w));
	map(0x10000, 0x11fff).mirror(0x6000).rom(); // 8k debug rom
	map(0x18000, 0x19fff).mirror(0x6000).ram(); // 8k of ram
}

void a2bus_q68plus_device::m68008_mem(address_map &map)
{
	map(0x00000, 0x0ffff).rw(FUNC(a2bus_q68plus_device::dma_r), FUNC(a2bus_q68plus_device::dma_w));
	map(0x10000, 0x11fff).rom(); // 8k debug rom
	map(0x12000, 0xfffff).ram(); // 1 MB of RAM
}

ROM_START( q68 )
	ROM_REGION(0x100000, "m68008", 0)
	ROM_LOAD( "debug1.04-caf4.bin", 0x10000, 0x02000, CRC(6bead126) SHA1(e46d5e8256faece3723e9040f78fd1c8edb3ee0e) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_q68_device::device_add_mconfig(machine_config &config)
{
	M68008(config, m_m68008, 1021800*7); // M68008 runs at 7.16 MHz
	m_m68008->set_addrmap(AS_PROGRAM, &a2bus_q68_device::m68008_mem);
}

void a2bus_q68plus_device::device_add_mconfig(machine_config &config)
{
	M68008(config, m_m68008, 1021800*7); // M68008 runs at 7.16 MHz
	m_m68008->set_addrmap(AS_PROGRAM, &a2bus_q68plus_device::m68008_mem);
}

//-------------------------------------------------
//  device_rom_region - device-specific ROMs
//-------------------------------------------------

const tiny_rom_entry *a2bus_68k_device::device_rom_region() const
{
	return ROM_NAME( q68 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_68k_device::a2bus_68k_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_a2bus_card_interface(mconfig, *this)
	, m_m68008(*this, "m68008")
	, m_bEnabled(false)
{
}

a2bus_q68_device::a2bus_q68_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_68k_device(mconfig, A2BUS_Q68, tag, owner, clock)
{
}

a2bus_q68plus_device::a2bus_q68plus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_68k_device(mconfig, A2BUS_Q68PLUS, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_68k_device::device_start()
{
	save_item(NAME(m_bEnabled));
}

void a2bus_68k_device::device_reset()
{
	reset_from_bus();
}

void a2bus_68k_device::reset_from_bus()
{
	m_m68008->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_m68008->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_bEnabled=false;
}

uint8_t a2bus_68k_device::read_c0nx(uint8_t offset)
{
	switch (offset & 0x3)
	{
		case 0:
		case 2:
			m_m68008->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			m_m68008->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_bEnabled=false;
			break;

		case 1:
			m_m68008->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			m_m68008->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_bEnabled=true;
			break;

		case 3:
			m_m68008->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			m_m68008->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_bEnabled=true;
			m_m68008->pulse_input_line(M68K_IRQ_7, attotime::zero);  //work around, processor status lines + logic should clear nmi.
			break;
	}
	return 0;
}

void a2bus_68k_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset & 0x3)
	{
		case 0:
		case 2:
			m_m68008->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			m_m68008->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
			m_bEnabled=false;
			break;

		case 1:
			m_m68008->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			m_m68008->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_bEnabled=true;
			break;

		case 3:
			m_m68008->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			m_m68008->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_bEnabled=true;
			m_m68008->pulse_input_line(M68K_IRQ_7, attotime::zero);  //work around, processor status lines should clear nmi.
			break;
	}
}

uint8_t a2bus_68k_device::dma_r(offs_t offset)
{
	if (m_bEnabled)
	{
		if (offset <= 0x03ff)        //0x0000-0x03ff
		{
			return slot_dma_read(offset+0x800);
		}
		else if (offset <= 0x07ff)   //0x0400-0x07ff
		{
			return slot_dma_read(offset);
		}
		else if (offset <= 0x0bff)   //0x0800-0x0bff
		{
			return slot_dma_read(offset-0x800);
		}
		else if (offset <= 0xffff)   //0x0c00-0xffff
		{
			return slot_dma_read(offset);
		}
	}
	return 0xff;
}


//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

void a2bus_68k_device::dma_w(offs_t offset, uint8_t data)
{
	if (m_bEnabled)
	{
		if (offset <= 0x03ff)        //0x0000-0x03ff
		{
			return slot_dma_write(offset+0x800, data);
		}
		else if (offset <= 0x07ff)   //0x0400-0x07ff
		{
			return slot_dma_write(offset, data);
		}
		else if (offset <= 0x0bff)   //0x0800-0x0bff
		{
			return slot_dma_write(offset-0x800, data);
		}
		else if (offset <= 0xffff)   //0x0c00-0xffff
		{
			return slot_dma_write(offset, data);
		}
	}
}
