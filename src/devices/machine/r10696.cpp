// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/**********************************************************************

    Rockwell 10696 General Purpose Input/Output (I/O)

    REGISTER DESCRIPTION

    HEX    Address   Select     Names
    -------------------------------------------------------
    A      x x x x   1 0 1 0    Read Group A
    9      x x x x   1 0 0 1    Read Group B
    3      x x x x   0 0 1 1    Read Group C
    0      x x x x   0 0 0 0    Read Groups A | B | C
    1      x x x x   0 0 0 1    Read Groups B | C
    2      x x x x   0 0 1 0    Read Groups A | C
    8      x x x x   1 0 0 0    Read Groups A | B

    E      x x x x   1 1 1 0    Set Group A
    D      x x x x   1 1 0 1    Set Group B
    7      x x x x   0 1 1 1    Set Group C
    4      x x x x   0 1 0 0    Set Groups A, B and C
    5      x x x x   0 1 0 1    Set Groups B and C
    6      x x x x   0 1 1 0    Set Groups A and C
    C      x x x x   1 1 0 0    Set Groups A and B

    Notes:
    Any of the I/O chips may be used to read or set any group
    (A, B, C) or combination of groups.
**********************************************************************/

#include "emu.h"
#include "r10696.h"

//#define VERBOSE 1
#include "logmacro.h"


/*************************************
 *
 *  Device interface
 *
 *************************************/

DEFINE_DEVICE_TYPE(R10696, r10696_device, "r10696", "Rockwell 10696 GPIO")

r10696_device::r10696_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, R10696, tag, owner, clock)
	, m_io_a(0), m_io_b(0), m_io_c(0)
	, m_iord(*this, 0), m_iowr(*this)
{
}

/**
 * @brief r10696_device::device_start device-specific startup
 */
void r10696_device::device_start()
{
	save_item(NAME(m_io_a));
	save_item(NAME(m_io_b));
	save_item(NAME(m_io_c));
}

/**
 * @brief r10696_device::device_reset device-specific reset
 */
void r10696_device::device_reset()
{
	m_io_a = 0;
	m_io_b = 0;
	m_io_c = 0;
}

/*************************************
 *
 *  Command access handlers
 *
 *************************************/

void r10696_device::io_w(offs_t offset, uint8_t data)
{
	assert(offset < 16);
	const uint8_t io_a = m_io_a;
	const uint8_t io_b = m_io_b;
	const uint8_t io_c = m_io_c;
	switch (offset)
	{
	case 0x0A: // Read Group A
	case 0x09: // Read Group B
	case 0x03: // Read Group C
	case 0x00: // Read Groups A | B | C
	case 0x01: // Read Groups B | C
	case 0x02: // Read Groups A | C
	case 0x08: // Read Groups A | B
		break;

	case 0x0E: // Set Group A
		m_io_a = data & 0x0f;
		break;
	case 0x0D: // Set Group B
		m_io_b = data & 0x0f;
		break;
	case 0x07: // Set Group C
		m_io_c = data & 0x0f;
		break;
	case 0x04: // Set Groups A, B and C
		m_io_a = m_io_b = m_io_c = data & 0x0f;
		break;
	case 0x05: // Set Groups B and C
		m_io_b = m_io_c = data & 0x0f;
		break;
	case 0x06: // Set Groups A and C
		m_io_a = m_io_c = data & 0x0f;
		break;
	case 0x0C: // Set Groups A and B
		m_io_a = m_io_b = data & 0x0f;
		break;
	}
	if (io_a != m_io_a)
		m_iowr(0, m_io_a, 0x0f);
	if (io_b != m_io_b)
		m_iowr(1, m_io_b, 0x0f);
	if (io_c != m_io_c)
		m_iowr(2, m_io_c, 0x0f);
}


uint8_t r10696_device::io_r(offs_t offset)
{
	assert(offset < 16);
	uint8_t io_a, io_b, io_c;
	uint8_t data = 0xf;
	switch (offset)
	{
	case 0x0A: // Read Group A
		io_a = m_iord(0);
		data = io_a & 0x0f;
		break;
	case 0x09: // Read Group B
		io_b = m_iord(1);
		data = io_b & 0x0f;
		break;
	case 0x03: // Read Group C
		io_c = m_iord(2);
		data = io_c & 0x0f;
		break;
	case 0x00: // Read Groups A | B | C
		io_a = m_iord(0);
		io_b = m_iord(1);
		io_c = m_iord(2);
		data = (io_a | io_b | io_c) & 0x0f;
		break;
	case 0x01: // Read Groups B | C
		io_b = m_iord(1);
		io_c = m_iord(2);
		data = (io_b | io_c) & 0x0f;
		break;
	case 0x02: // Read Groups A | C
		io_a = m_iord(0);
		io_c = m_iord(2);
		data = (io_a | io_c) & 0x0f;
		break;
	case 0x08: // Read Groups A | B
		io_a = m_iord(0);
		io_b = m_iord(1);
		data = (io_a | io_b) & 0x0f;
		break;

	case 0x0E: // Set Group A
	case 0x0D: // Set Group B
	case 0x07: // Set Group C
	case 0x04: // Set Groups A, B and C
	case 0x05: // Set Groups B and C
	case 0x06: // Set Groups A and C
	case 0x0C: // Set Groups A and B
		break;
	}
	return data;
}
