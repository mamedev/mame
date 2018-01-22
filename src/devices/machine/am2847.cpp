// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    AMD Am2847/Am2896 Quad 80/96-Bit Static Shift Registers

**********************************************************************/

#include "emu.h"
#include "am2847.h"

DEFINE_DEVICE_TYPE(AM2847,  am2847_device,  "am2847",  "AMD Am2847 80-bit Static Shift Register")
DEFINE_DEVICE_TYPE(AM2849,  am2849_device,  "am2849",  "AMD Am2849 96-bit Static Shift Register")
DEFINE_DEVICE_TYPE(TMS3409, tms3409_device, "tms3409", "TI TMS3409 80-bit Static Shift Register")

am2847_base_device::am2847_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t size)
	: device_t(mconfig, type, tag, owner, clock)
	, m_in(0)
	, m_out(0)
	, m_rc(0)
	, m_cp(0)
	, m_buffer_size(size)
{
}

am2847_device::am2847_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am2847_base_device(mconfig, AM2847, tag, owner, clock, 5)
{
}

am2849_device::am2849_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am2847_base_device(mconfig, AM2849, tag, owner, clock, 6)
{
}

tms3409_device::tms3409_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: am2847_base_device(mconfig, TMS3409, tag, owner, clock, 5)
{
}

void am2847_base_device::device_start()
{
	for (int bit = 0; bit < 4; bit++)
	{
		m_shifters[bit].alloc(m_buffer_size);
	}

	init();

	save_item(NAME(m_in));
	save_item(NAME(m_out));
}

void am2847_base_device::device_reset()
{
	init();
}

void am2847_base_device::init()
{
	m_in = 0;
	m_out = 0;
}

WRITE_LINE_MEMBER( am2847_base_device::in_a_w )
{
	m_in &= ~0x01;
	m_in |= state;
}

WRITE_LINE_MEMBER( am2847_base_device::in_b_w )
{
	m_in &= ~0x02;
	m_in |= state << 1;
}

WRITE_LINE_MEMBER( am2847_base_device::in_c_w )
{
	m_in &= ~0x04;
	m_in |= state << 2;
}

WRITE_LINE_MEMBER( am2847_base_device::in_d_w )
{
	m_in &= ~0x08;
	m_in |= state << 3;
}

void am2847_base_device::in_w(uint8_t data)
{
	m_in = data & 0x0f;
}

WRITE_LINE_MEMBER( am2847_base_device::rc_a_w )
{
	m_rc &= ~0x01;
	m_rc |= state;
}

WRITE_LINE_MEMBER( am2847_base_device::rc_b_w )
{
	m_rc &= ~0x02;
	m_rc |= state << 1;
}

WRITE_LINE_MEMBER( am2847_base_device::rc_c_w )
{
	m_rc &= ~0x04;
	m_rc |= state << 2;
}

WRITE_LINE_MEMBER( am2847_base_device::rc_d_w )
{
	m_rc &= ~0x08;
	m_rc |= state << 3;
}

void am2847_base_device::rc_w(uint8_t data)
{
	m_rc = data & 0x0f;
}

WRITE_LINE_MEMBER( am2847_base_device::cp_w )
{
	if (m_cp != state && state != 0)
	{
		shift();
	}
	m_cp = state;
}

void am2847_base_device::shift()
{
	for (int bit = 0; bit < 4; bit++)
	{
		m_out &= ~(1 << bit);
		m_out |= m_shifters[bit].shift(BIT(m_rc, bit) != 0, BIT(m_in, bit)) << bit;
	}
}

void am2847_base_device::shifter::alloc(size_t size)
{
	m_buffer_size = size;
	m_buffer = std::make_unique<uint16_t[]>(m_buffer_size);
	init();
}

void am2847_base_device::shifter::init()
{
	memset(&m_buffer[0], 0, m_buffer_size * 2);
}

int am2847_base_device::shifter::shift(bool recycle, int in)
{
	int out = m_buffer[m_buffer_size-1] & 1;
	for (int word = m_buffer_size - 1; word >= 1; word--)
	{
		m_buffer[word] >>= 1;
		m_buffer[word] |= (m_buffer[word - 1] & 1) << 15;
	}

	m_buffer[0] >>= 1;

	if (recycle)
		m_buffer[0] |= out << 15;
	else
		m_buffer[0] |= (in & 1) << 15;

	return out;
}
