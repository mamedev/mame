// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 BUS Controller

  Lies at 0xffffffe0-0xffffffff


  TODO:
  - Host CPU setter (is_slave and clock are needed);
  - timer clock emulation;
  - fix fatalerrors;
  - bus control stuff, someday;

***************************************************************************/

#include "emu.h"
#include "sh7604_bus.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SH7604_BUS, sh7604_bus_device, "sh7604bus", "SH7604 BUS Controller")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

uint16_t sh7604_bus_device::bus_control_1_r()
{
	return (m_bcr1 & 0x1ff7) | (m_is_slave == true ? 0x8000 : 0);
}

void sh7604_bus_device::bus_control_1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr1);
	if(m_bcr1 & 0x1000) // ENDIAN
		throw emu_fatalerror("%s: enabled little endian for Area 2\n", tag());
	if(m_bcr1 & 0x0800) // PSHR
		throw emu_fatalerror("%s: enabled partial space share mode\n", tag());
}

uint16_t sh7604_bus_device::bus_control_2_r() { return m_bcr2 & 0x00fc; }
void sh7604_bus_device::bus_control_2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr2);
	if(m_bcr2 != 0x00fc)
		throw emu_fatalerror("%s: unexpected bus size register set %04x\n", tag(),data);
}

uint16_t sh7604_bus_device::wait_control_r() { return m_wcr; }
void sh7604_bus_device::wait_control_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_wcr); }

uint16_t sh7604_bus_device::memory_control_r() { return m_mcr & 0xfefc; }
void sh7604_bus_device::memory_control_w(offs_t offset, uint16_t data, uint16_t mem_mask) { COMBINE_DATA(&m_mcr); }

uint16_t sh7604_bus_device::refresh_timer_status_r()
{
	return m_rtcsr & 0x00f8;
}

void sh7604_bus_device::refresh_timer_control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcsr);

	if(m_rtcsr & 0x40)
		throw emu_fatalerror("%s: enabled timer irq register with clock setting = %02x\n",tag(),data & 0x38);
}

uint16_t sh7604_bus_device::refresh_timer_counter_r()
{
	throw emu_fatalerror("%s: reading timer counter!\n",tag());
	return 0;
}

void sh7604_bus_device::refresh_timer_counter_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	throw emu_fatalerror("%s: writing timer counter %04x\n",tag(),data);
	//COMBINE_DATA(&m_rtcnt);
}

uint16_t sh7604_bus_device::refresh_timer_constant_r()
{
	return m_rtcor & 0xff;
}

void sh7604_bus_device::refresh_timer_constant_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcor);
}

void sh7604_bus_device::bus_regs(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(sh7604_bus_device::bus_control_1_r), FUNC(sh7604_bus_device::bus_control_1_w));
	map(0x02, 0x03).rw(FUNC(sh7604_bus_device::bus_control_2_r), FUNC(sh7604_bus_device::bus_control_2_w));
	map(0x04, 0x05).rw(FUNC(sh7604_bus_device::wait_control_r), FUNC(sh7604_bus_device::wait_control_w));
	map(0x06, 0x07).rw(FUNC(sh7604_bus_device::memory_control_r), FUNC(sh7604_bus_device::memory_control_w));
	map(0x08, 0x09).rw(FUNC(sh7604_bus_device::refresh_timer_status_r), FUNC(sh7604_bus_device::refresh_timer_control_w));
	map(0x0a, 0x0b).rw(FUNC(sh7604_bus_device::refresh_timer_counter_r), FUNC(sh7604_bus_device::refresh_timer_counter_w));
	map(0x0c, 0x0d).rw(FUNC(sh7604_bus_device::refresh_timer_constant_r), FUNC(sh7604_bus_device::refresh_timer_constant_w));
//  map(0x0e, 0x0f) unmapped, mirror?
}

//-------------------------------------------------
//  sh7604_bus_device - constructor
//-------------------------------------------------

sh7604_bus_device::sh7604_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7604_BUS, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sh7604_bus_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sh7604_bus_device::device_reset()
{
	m_bcr1 = 0x03f0;
	m_bcr2 = 0x00fc;
	m_wcr = 0xaaff;
	m_mcr = 0x0000;
	m_rtcsr = 0x0000;
	m_rtcor = 0x0000;
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint32_t sh7604_bus_device::read(address_space &space, offs_t offset)
{
	// 16 bit access only, TODO
	return space.read_word(offset) & 0xffff;
}

void sh7604_bus_device::write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// TODO: 8 bit access is invalid
	// if accessing bits 16-31, one must write ID = 0xa55a
	if(ACCESSING_BITS_16_31)
	{
		// throw fatalerror if something trips it, presumably the write is going to be ignored
		if((data & 0xffff0000) != 0xa55a0000)
			throw emu_fatalerror("%s: making bus write with ID signature = %04x!\n", tag(),data >> 16);
	}

	space.write_word(offset,data & 0xffff);
}
