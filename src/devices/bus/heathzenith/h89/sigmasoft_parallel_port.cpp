// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Universal Parallel Interface Board

  TODO
    - add support for hard/floppy drive controller

****************************************************************************/

#include "emu.h"

#include "bus/heathzenith/h19/tlb.h"

#include "sigmasoft_parallel_port.h"

//
// Logging defines
//
#define LOG_REG  (1U << 1)   // Shows register setup
#define LOG_FUNC (1U << 2)   // Function calls

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREG(...)        LOGMASKED(LOG_REG, __VA_ARGS__)
#define LOGFUNC(...)       LOGMASKED(LOG_FUNC, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


static constexpr u16 SELECT_ADDR_MASK = 0xf8;

/**
 * The SigmaSoft Parallel Port
 */
sigmasoft_parallel_port::sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sigmasoft_parallel_port(mconfig, H89BUS_SIGMASOFT_PARALLEL, tag, owner, clock)
{
}

sigmasoft_parallel_port::sigmasoft_parallel_port(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_h89bus_left_card_interface(mconfig, *this)
	, m_jumpers(*this, "JUMPERS")
{
}

void sigmasoft_parallel_port::igc_w(u8 offset, u8 val)
{
}

void sigmasoft_parallel_port::write(offs_t offset, u8 data)
{
	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, data);

	offset &= 0x07;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			igc_w(offset, data);
			break;
		case 6:
			// TODO - Centronics interface
			break;
		case 7:
			// TODO - Centronics interface
			break;
	}
}

u8 sigmasoft_parallel_port::igc_r(u8 offset)
{
	return 0;
}

u8 sigmasoft_parallel_port::read(offs_t offset)
{
	u8 value = 0;

	offset &= 0x07;

	switch (offset)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			value = igc_r(offset);
			break;
		case 6:
			// TODO - Centronics interface
			break;
		case 7:
			// TODO - Centronics interface
			break;
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, value);

	return value;
}

void sigmasoft_parallel_port::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
}

void sigmasoft_parallel_port::device_reset()
{
	ioport_value const jumpers(m_jumpers->read());

	m_enabled = bool(jumpers & 0x20);

	m_base_addr = (jumpers & 0x1f) << 3;

	if (!m_installed)
	{
		if (m_enabled)
		{
			h89bus().install_io_device(m_base_addr, m_base_addr + 7,
				read8sm_delegate(*this, FUNC(sigmasoft_parallel_port::read)),
				write8sm_delegate(*this, FUNC(sigmasoft_parallel_port::write)));
		}

		m_installed = true;
	}
}

static INPUT_PORTS_START( sigmasoft_parallel_port_device )
	PORT_START("JUMPERS")
	PORT_CONFNAME(0x01, 0x01, "Port Address Selection a3" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x01, "1")
	PORT_CONFNAME(0x02, 0x00, "Port Address Selection a4" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x02, "1")
	PORT_CONFNAME(0x04, 0x00, "Port Address Selection a5" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x04, "1")
	PORT_CONFNAME(0x08, 0x00, "Port Address Selection a6" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x08, "1")
	PORT_CONFNAME(0x10, 0x00, "Port Address Selection a7" )
	PORT_CONFSETTING(   0x00, "0")
	PORT_CONFSETTING(   0x10, "1")

	PORT_CONFNAME(0x20, 0x20, "Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x20, DEF_STR( Yes ))
INPUT_PORTS_END

ioport_constructor sigmasoft_parallel_port::device_input_ports() const
{
	return INPUT_PORTS_NAME(sigmasoft_parallel_port_device);
}


/**
 * The SigmaSoft Parallel Port connected to a SigmaSoft IGC board
 */
sigmasoft_parallel_port_igc::sigmasoft_parallel_port_igc(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	sigmasoft_parallel_port(mconfig, H89BUS_SIGMASOFT_PARALLEL_IGC, tag, owner, clock),
	m_tlbc(*this, finder_base::DUMMY_TAG)
{
}

void sigmasoft_parallel_port_igc::igc_w(u8 offset, u8 val)
{
	m_tlbc->sigma_w(offset, val);
}

u8 sigmasoft_parallel_port_igc::igc_r(u8 offset)
{
	return m_tlbc->sigma_r(offset);
}

void sigmasoft_parallel_port_igc::device_start()
{
	sigmasoft_parallel_port::device_start();
}

void sigmasoft_parallel_port_igc::device_reset()
{
	sigmasoft_parallel_port::device_reset();
}


DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_SIGMASOFT_PARALLEL,     device_h89bus_left_card_interface, sigmasoft_parallel_port,     "sigmasoft_parallel_port",     "SigmaSoft Universal Parallel Board");
DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_SIGMASOFT_PARALLEL_IGC, device_h89bus_left_card_interface, sigmasoft_parallel_port_igc, "sigmasoft_parallel_port_igc", "SigmaSoft Universal Parallel Board connected to SigmaSoft IGC");
