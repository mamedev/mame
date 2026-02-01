// license:BSD-3-Clause
// copyright-holders:Rob Justice, R. Belmont
/*********************************************************************

    softcard3.cpp

    Implementation of the Microsoft SoftCard /// Z-80 card

    Best guess at the moment based on disassembly of the boot disk
    and some manual checks with the real hardware

*********************************************************************/

#include "emu.h"
#include "softcard3.h"
#include "cpu/z80/z80.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_SOFTCARD3, a2bus_softcard3_device, "softcard3", "Microsoft SoftCard ///")

void a2bus_softcard3_device::z80_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(a2bus_softcard3_device::dma_r), FUNC(a2bus_softcard3_device::dma_w));
}

ROM_START( softcard3 )
	ROM_REGION(0x100, "mapping_prom", 0)
	ROM_LOAD("softcard3.rom", 0x0000, 0x0100, CRC(9d4433b2) SHA1(aff45dd8850641b4616b61750c104e7ee45a99a4))
ROM_END

void a2bus_softcard3_device::z80_io(address_map &map)
{
	map(0x00, 0x00).w(FUNC(a2bus_softcard3_device::z80_io_w));
}

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_softcard3_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_z80, 1021800*2);   // Z80 runs at 2M based on comment in the manual
	m_z80->set_addrmap(AS_PROGRAM, &a2bus_softcard3_device::z80_mem);
	m_z80->set_addrmap(AS_IO, &a2bus_softcard3_device::z80_io);
	TIMER(config, "timer").configure_generic(FUNC(a2bus_softcard3_device::timercallback));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_softcard3_device::a2bus_softcard3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_z80(*this, "z80"),
	m_prom(*this, "mapping_prom"),
	m_timer(*this, "timer"),
	m_bEnabled(false),
	m_reset(false),
	m_enable_fffx(false)
{
}

a2bus_softcard3_device::a2bus_softcard3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_softcard3_device(mconfig, A2BUS_SOFTCARD3, tag, owner, clock)
{
}

const tiny_rom_entry *a2bus_softcard3_device::device_rom_region() const
{
	return ROM_NAME(softcard3);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_softcard3_device::device_start()
{
	save_item(NAME(m_bEnabled));
	save_item(NAME(m_reset));
	save_item(NAME(m_enable_fffx));
}

void a2bus_softcard3_device::device_reset()
{
	reset_from_bus();
}

void a2bus_softcard3_device::reset_from_bus()
{
	m_bEnabled = false;
	m_reset = false;
	m_enable_fffx = false;
	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_timer->adjust(attotime::never);
}

uint8_t a2bus_softcard3_device::read_c0nx(uint8_t offset)
{
	return 0x50; //software is looking for 0x50, needs to be checked if this can return other flags
}

void a2bus_softcard3_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (data)
	{
		case 0x01: // reset
			m_reset = true; // flag we need a reset when we enable the Z80
			break;

		case 0x10: // enable the card fffc & fffd memory mapped address to appear
			m_enable_fffx = true;
			break;

		default:
			//printf("Softcard3: %02x to unhandled c0n%x\n", data, offset);
			break;
	}
}

// read to fffc halts 6502 and enables z80
uint8_t a2bus_softcard3_device::read_inh_rom(uint16_t offset)
{
	if (offset == 0xfffc)
	{
		m_bEnabled = true;
		raise_slot_dma();
		m_z80->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		if (m_reset)
		{
			m_z80->reset();
		}
		m_timer->adjust(attotime::from_hz(5.0)); //start supervision timer
		//printf("timer start\n");
	}

	if (offset == 0xfffd)
	{
		m_timer->adjust(attotime::never); //disable supervision timer
		//printf("disable timer\n");
	}

	return 0xff;
}

// returns if we want to /INH a read or write to a specific address
bool a2bus_softcard3_device::inh_check(u16 offset, bool bIsWrite)
{
	if (!m_enable_fffx)
	{
		return false;
	}

	if (!(bIsWrite) && (offset == 0xfffc)) //only a read to fffc is mapped in
	{
		return true;
	}

	if (!(bIsWrite) && (offset == 0xfffd)) //only a read to fffd is mapped in
	{
		return true;
	}

	return false;
}

// this io write halts the z80 and returns to the 6502
void a2bus_softcard3_device::z80_io_w(offs_t offset, uint8_t data)
{
	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	lower_slot_dma();
	m_bEnabled = false;
	m_enable_fffx = false;
	m_reset = false;
	m_timer->adjust(attotime::never);
	//printf("z80 io timer stop\n");
}

// fires if Z80 has not handed back to the 6502 in this timer period
// this ensures 6502 can run and service console i/o
//  actual time to be checked on real hw
TIMER_DEVICE_CALLBACK_MEMBER(a2bus_softcard3_device::timercallback)
{
	m_z80->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	lower_slot_dma();
	m_bEnabled = false;
	m_enable_fffx = false;
	m_reset = false;
	//printf("timer callback stop\n");
}

//memory mapping based on a dump of the mapping rom 256x8
//maps the top 8 address lines
uint8_t a2bus_softcard3_device::dma_r(offs_t offset)
{
	if (m_bEnabled)
	{
		return slot_dma_read((offset & 0xff) + (m_prom[offset >> 8] << 8));
	}

	return 0xff;
}

//-------------------------------------------------
//  dma_w -
//-------------------------------------------------

void a2bus_softcard3_device::dma_w(offs_t offset, uint8_t data)
{
	if (m_bEnabled)
	{
		slot_dma_write((offset & 0xff) + (m_prom[offset >> 8] << 8), data);
	}
}
