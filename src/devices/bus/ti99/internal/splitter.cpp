// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
     I/O port splitter

     The I/O port splitter connects to the TI-99/4(A) console at its I/O port
     and provides two I/O ports. That way, two Peripheral Expansion boxes or
     one PEB and a sidecar chain may be connected.

                            | Port 2
                            v
                          +---+
     +----------------+   |   |
     |   TI-99/4(A)   |---+   +--+
     +------------+---+ Splitter | <-- Port 1
     | oooooooooo |   |----------+
     | oooooooooo |   |
     +-----------------

     The splitter was designed 2015 by Jim Fetzner (as Tekumel Software)

     March 2025, Michael Zapf

*****************************************************************************/

#include "emu.h"
#include "ioport.h"
#include "splitter.h"
#include "bus/ti99/peb/peribox.h"

#define LOG_WARN        (1U << 1)   // Warnings
#define LOG_CONFIG      (1U << 2)   // Configuration
#define LOG_INT         (1U << 3)
#define LOG_READY       (1U << 4)

// #define VERBOSE (LOG_CONFIG | LOG_WARN)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_IOSPLIT, bus::ti99::internal::ioport_splitter_device, "ti99_iosplit", "TI-99 I/O Port Splitter")

namespace bus::ti99::internal {

#define PORT1 "port1"
#define PORT2 "port2"

ioport_splitter_device::ioport_splitter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	:   ioport_attached_device(mconfig, type, tag, owner, clock),
		m_port1(*this, PORT1),
		m_port2(*this, PORT2)
{
}

ioport_splitter_device::ioport_splitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ioport_splitter_device(mconfig, TI99_IOSPLIT, tag, owner, clock)
{
}

void ioport_splitter_device::readz(offs_t offset, uint8_t *value)
{
	// With a proper configuration, only one device on one of the branches
	// will respond.
	if (m_port1 != nullptr)
		m_port1->readz(offset, value);

	if (m_port2 != nullptr)
		m_port2->readz(offset, value);
}

void ioport_splitter_device::write(offs_t offset, uint8_t data)
{
	if (m_port1 != nullptr)
		m_port1->write(offset, data);

	if (m_port2 != nullptr)
		m_port2->write(offset, data);
}

void ioport_splitter_device::setaddress_dbin(offs_t offset, int state)
{
	if (m_port1 != nullptr)
		m_port1->setaddress_dbin(offset, state);

	if (m_port2 != nullptr)
		m_port2->setaddress_dbin(offset, state);
}

void ioport_splitter_device::crureadz(offs_t offset, uint8_t *value)
{
	if (m_port1 != nullptr)
		m_port1->crureadz(offset, value);

	if (m_port2 != nullptr)
		m_port2->crureadz(offset, value);
}

void ioport_splitter_device::cruwrite(offs_t offset, uint8_t data)
{
	if (m_port1 != nullptr)
		m_port1->cruwrite(offset, data);

	if (m_port2 != nullptr)
		m_port2->cruwrite(offset, data);
}

void ioport_splitter_device::memen_in(int state)
{
	if (m_port1 != nullptr)
		m_port1->memen_in(state);

	if (m_port2 != nullptr)
		m_port2->memen_in(state);
}

void ioport_splitter_device::msast_in(int state)
{
	if (m_port1 != nullptr)
		m_port1->msast_in(state);

	if (m_port2 != nullptr)
		m_port2->msast_in(state);
}

void ioport_splitter_device::clock_in(int state)
{
	if (m_port1 != nullptr)
		m_port1->clock_in(state);

	if (m_port2 != nullptr)
		m_port2->clock_in(state);
}

void ioport_splitter_device::reset_in(int state)
{
	if (m_port1 != nullptr)
		m_port1->reset_in(state);

	if (m_port2 != nullptr)
		m_port2->reset_in(state);
}

template<int port>
void ioport_splitter_device::extint(int state)
{
	LOGMASKED(LOG_INT, "propagating INTA from port %d to console: %d\n", port, state);
	if (state==ASSERT_LINE)
		m_inta_flag |= port;  // 1 or 2
	else
		m_inta_flag &= ~port;  // 1 or 2

	set_extint((m_inta_flag != 0)? ASSERT_LINE : CLEAR_LINE);
}

template<int port>
void ioport_splitter_device::ready(int state)
{
	LOGMASKED(LOG_READY, "Incoming READY=%d from port %d\n", state, port);
	// We store the inverse state
	if (state==CLEAR_LINE)
		m_ready_flag |= port;  // 1 or 2
	else
		m_ready_flag &= ~port;  // 1 or 2

	set_ready((m_ready_flag != 0)? CLEAR_LINE : ASSERT_LINE);
}

void ioport_splitter_device::device_start()
{
	LOG("Starting I/O Port Splitter\n");
}


void ioport_splitter_device::device_config_complete()
{
	m_inta_flag = 0;
	m_ready_flag = 0;
}

void ioport_splitter_device::device_add_mconfig(machine_config &config)
{
	TI99_IOPORT(config, m_port1, 1, ti99_ioport_options_evpc1, nullptr);
	TI99_IOPORT(config, m_port2, 2, ti99_ioport_options_evpc1, nullptr);

	m_port1->extint_cb().set(FUNC(ioport_splitter_device::extint<1>));
	m_port2->extint_cb().set(FUNC(ioport_splitter_device::extint<2>));
	m_port1->ready_cb().set(FUNC(ioport_splitter_device::ready<1>));
	m_port2->ready_cb().set(FUNC(ioport_splitter_device::ready<2>));
}

} // end namespace bus::ti99::internal
