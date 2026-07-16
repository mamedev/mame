// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

PC-9801-03 cassette interface

i8251 + glue logic + cassette port

TODO:
- gather what's the difference with -13 (looks same-ish implementation);
- make it work (looks a very basic UART to cassette deck connection);

**************************************************************************************************/

#include "emu.h"

#include "pc9801_03.h"
#include "speaker.h"


#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

// device type definition
DEFINE_DEVICE_TYPE(PC9801_03, pc9801_03_device, "pc9801_03", "NEC PC-9801-03 CMT cassette interface card")

pc9801_03_device::pc9801_03_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PC9801_03, tag, owner, clock)
	, device_pc98_cbus_slot_interface(mconfig, *this)
	, m_uart(*this, "uart")
	, m_cassette(*this, "cassette")
{
}

void pc9801_03_device::device_add_mconfig(machine_config &config)
{
	// TODO: connect to speaker(s)
//	SPEAKER(config, "speaker", 2).front();

	// TODO: clock, derived from C-Bus SCKL1?
	I8251(config, m_uart, 1021800);

	CASSETTE(config, m_cassette);
//  m_cassette->set_formats(pc98_cassette_formats);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
//  m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void pc9801_03_device::device_validity_check(validity_checker &valid) const
{
}

void pc9801_03_device::device_start()
{
}

void pc9801_03_device::device_reset()
{
}

void pc9801_03_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		m_bus->install_device(0x0000, 0x3fff, *this, &pc9801_03_device::io_map);
	}
}

// TODO: only bit 5 meaning known
void pc9801_03_device::control_port_w(offs_t offset, u8 data)
{
	LOG("Control port %02x\n", data);
	m_cassette->change_state(BIT(data, 5) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
}

void pc9801_03_device::io_map(address_map &map)
{
	map(0x0090, 0x0093).rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0xff00);
	map(0x0094, 0x0097).w(FUNC(pc9801_03_device::control_port_w)).umask16(0xff00);
}
