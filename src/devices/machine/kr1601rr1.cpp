// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    KR1601RR1 1024x4 bit EAROM

    Same geometry as GI ER2401, but not pin-compatible.

    To do:
    - realistic timing esp. for ERASE_ALL
    - alternate wirings

***************************************************************************/

#include "emu.h"
#include "kr1601rr1.h"

#include <algorithm>


#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"


DEFINE_DEVICE_TYPE(KR1601RR1, kr1601rr1_device, "kr1601rr1", "KR1601RR1 EAROM")

//-------------------------------------------------
//  ctor
//-------------------------------------------------

kr1601rr1_device::kr1601rr1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, KR1601RR1, tag, owner, clock)
	, device_nvram_interface(mconfig, *this)
{
}

uint8_t kr1601rr1_device::read(offs_t offset)
{
	assert(EAROM_SIZE > offset);
	if (m_earom_mode == EAROM_READ)
	{
		LOG("earom R %03x == %x\n", offset, m_earom[offset] & 15);
		return m_earom[offset] & 15;
	}
	else return 0;
}

/*
 * wiring hardcoded for ms6102:
 *
 * b7..b4 = CS _PR _ER RD
 * b3..b0 = data
 */
void kr1601rr1_device::write(offs_t offset, uint8_t data)
{
	assert(EAROM_SIZE > offset);

	switch (data >> 4)
	{
	case 0x0: case 0x1: case 0x2: case 0x3:
	case 0x4: case 0x5: case 0x6: case 0x7:
		m_earom_mode = EAROM_IDLE;
		break;

	case 0x8:
		m_earom_mode = EAROM_ERASE;
		break;

	case 0xa:
		m_earom_mode = EAROM_WRITE;
		break;

	case 0xc:
		m_earom_mode = EAROM_ERASE_ALL;
		break;

	case 0xf:
		m_earom_mode = EAROM_READ;
		break;
	}
	LOG("earom new mode = %u (from %02X)\n", m_earom_mode, data);

	switch (m_earom_mode)
	{
	case EAROM_WRITE:
		LOG("earom W %03x <- %x\n", offset, data & 15);
		m_earom[offset] |= (data & 15);
		break;

	case EAROM_ERASE:
		LOG("earom erase %03x\n", offset);
		m_earom[offset] = 0;
		break;

	case EAROM_ERASE_ALL:
		LOG("earom erase all\n");
		std::fill(std::begin(m_earom), std::end(m_earom), 0);
		break;

	default:
		break;
	}
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void kr1601rr1_device::nvram_default()
{
	std::fill(std::begin(m_earom), std::end(m_earom), 0);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool kr1601rr1_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_earom, EAROM_SIZE, actual) && actual == EAROM_SIZE;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool kr1601rr1_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_earom, EAROM_SIZE, actual) && actual == EAROM_SIZE;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void kr1601rr1_device::device_start()
{
	/* register for save states */
	save_item(NAME(m_earom));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void kr1601rr1_device::device_reset()
{
	m_earom_mode = EAROM_IDLE;
}
