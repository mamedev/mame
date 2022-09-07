// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Mitsubishi M50734 emulation (preliminary)

    This microcontroller contains no internal memory, RAM or ROM. The
    external bus allows P05 to be programmed as a strobe (DME) for a
    separable "data" memory space. On-chip peripherals include an ADC,
    UART, various general and special-purpose timers and a stepper
    motor controller.

**********************************************************************/

#include "emu.h"
#include "m50734.h"

// device type definition
DEFINE_DEVICE_TYPE(M50734, m50734_device, "m50734", "Mitsubishi M50734")

m50734_device::m50734_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m740_device(mconfig, M50734, tag, owner, clock)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_port_in_cb(*this)
	, m_port_out_cb(*this)
	, m_port_latch{0, 0, 0, 0}
	, m_port_3state{0, 0, 0, 0}
{
	program_config.m_internal_map = address_map_constructor(FUNC(m50734_device::internal_map), this);
}

device_memory_interface::space_config_vector m50734_device::memory_space_config() const
{
	if (has_configured_map(AS_DATA))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &program_config),
			std::make_pair(AS_DATA, &m_data_config)
		};
	else
		return m740_device::memory_space_config();
}

void m50734_device::device_resolve_objects()
{
	for (int n = 0; n < 4; n++)
		m_port_in_cb[n].resolve_safe(m_port_3state[n]);
	m_port_in_cb[4].resolve_safe(0);
	m_port_out_cb.resolve_all_safe();
}

void m50734_device::device_start()
{
	m740_device::device_start();

	space(has_space(AS_DATA) ? AS_DATA : AS_PROGRAM).specific(m_data);

	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_direction));
	save_item(NAME(m_p0_function));
	save_item(NAME(m_p2_p3_function));
}

void m50734_device::device_reset()
{
	m740_device::device_reset();

	std::fill(std::begin(m_port_direction), std::end(m_port_direction), 0x00);
	for (int n = 0; n < 4; n++)
		m_port_out_cb[n](m_port_3state[n]);
	m_p0_function = 0x00;
	m_p2_p3_function = 0x00;
}

void m50734_device::read_dummy(u16 adr)
{
	// M50734 outputs RD and WR strobes rather than R/W, so dummy accesses should do nothing
}

u8 m50734_device::read_data(u16 adr)
{
	if (BIT(m_p0_function, 5))
		return m_data.read_byte(adr);
	else
		return m740_device::read(adr);
}

void m50734_device::write_data(u16 adr, u8 val)
{
	if (BIT(m_p0_function, 5))
		m_data.write_byte(adr, val);
	else
		m740_device::write(adr, val);
}

template <int N>
u8 m50734_device::port_r(offs_t offset)
{
	if (BIT(offset, 0))
		return m_port_direction[N];
	else if (m_port_direction[0] == 0xff)
		return m_port_latch[N];
	else
		return (m_port_in_cb[N]() & ~m_port_direction[N]) | (m_port_latch[N] & m_port_direction[N]);
}

template <int N>
void m50734_device::port_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
	{
		if (m_port_direction[N] != data)
		{
			logerror("Port P%d direction = $%02X\n", N, data);
			m_port_direction[N] = data;
			m_port_out_cb[N]((data & m_port_latch[N]) | (m_port_3state[N] & ~m_port_direction[N]));
		}
	}
	else if (((std::exchange(m_port_latch[N], data) ^ data) & m_port_direction[N]) != 0)
		m_port_out_cb[N]((data & m_port_direction[N]) | (m_port_3state[N] & ~m_port_direction[N]));
}

u8 m50734_device::p4_r()
{
	// P4 has only 4 pins and no output drivers
	return m_port_in_cb[4]() & 0x0f;
}

u8 m50734_device::p0_function_r()
{
	return m_p0_function;
}

void m50734_device::p0_function_w(u8 data)
{
	logerror("Port P0 function = $%02X\n", data);
	m_p0_function = data;
}

u8 m50734_device::p2_p3_function_r()
{
	return m_p2_p3_function;
}

void m50734_device::p2_p3_function_w(u8 data)
{
	logerror("Port P2/P3 function = $%02X\n", data);
	m_p2_p3_function = data & 0xc7;
}

void m50734_device::internal_map(address_map &map)
{
	// TODO: timers, etc.
	map(0x00eb, 0x00eb).r(FUNC(m50734_device::p4_r));
	map(0x00ed, 0x00ed).rw(FUNC(m50734_device::p2_p3_function_r), FUNC(m50734_device::p2_p3_function_w));
	map(0x00ee, 0x00ef).rw(FUNC(m50734_device::port_r<3>), FUNC(m50734_device::port_w<3>));
	map(0x00f0, 0x00f1).rw(FUNC(m50734_device::port_r<2>), FUNC(m50734_device::port_w<2>));
	map(0x00f3, 0x00f4).rw(FUNC(m50734_device::port_r<1>), FUNC(m50734_device::port_w<1>));
	map(0x00f5, 0x00f5).rw(FUNC(m50734_device::p0_function_r), FUNC(m50734_device::p0_function_w));
	map(0x00f6, 0x00f7).rw(FUNC(m50734_device::port_r<0>), FUNC(m50734_device::port_w<0>));
}
