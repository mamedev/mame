// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Universal Parallel Interface Board


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


static constexpr u16 SELECT_ADDR_MASK = 0xF8;

sigmasoft_parallel_port::sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, H89BUS_SIGMASOFT_PARALLEL, tag, owner, clock),
	device_h89bus_left_card_interface(mconfig, *this),
	m_tlbc(*this, finder_base::DUMMY_TAG),
	m_jumpers(*this, "JUMPERS")
{
}

inline bool sigmasoft_parallel_port::card_selected(u8 select_lines, u16 offset)
{
	return m_enabled &&
		(select_lines & h89bus_device::H89_IO) &&
		((offset & SELECT_ADDR_MASK) == m_base_addr);
}

void sigmasoft_parallel_port::video_mem_w(u8 val)
{
	if (m_connected_tlbc)
	{
		m_tlbc->sigma_video_mem_w(val);
	}
}

void sigmasoft_parallel_port::io_lo_addr_w(u8 val)
{
	if (m_connected_tlbc)
	{
		m_tlbc->sigma_io_lo_addr_w(val);
	}
}

void sigmasoft_parallel_port::io_hi_addr_w(u8 val)
{
	if (m_connected_tlbc)
	{
		m_tlbc->sigma_io_hi_addr_w(val);
	}
}

void sigmasoft_parallel_port::window_lo_addr_w(u8 val)
{
	if (m_connected_tlbc)
	{
		m_tlbc->sigma_window_lo_addr_w(val);
	}
}

void sigmasoft_parallel_port::window_hi_addr_w(u8 val)
{
	if (m_connected_tlbc)
	{
		m_tlbc->sigma_window_hi_addr_w(val);
	}
}

void sigmasoft_parallel_port::ctrl_w(u8 val)
{
	if (m_connected_tlbc)
	{
		m_tlbc->sigma_ctrl_w(val);
	}
}

void sigmasoft_parallel_port::write(u8 select_lines, u16 offset, u8 data)
{
	if (!card_selected(select_lines, offset))
	{
		return;
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, offset, data);

	offset &= 0x07;

	switch (offset)
	{
	case 0:
		video_mem_w(data);
		break;
	case 1:
		io_lo_addr_w(data);
		break;
	case 2:
		io_hi_addr_w(data);
		break;
	case 3:
		window_lo_addr_w(data);
		break;
	case 4:
		window_hi_addr_w(data);
		break;
	case 5:
		// IGC Control
		ctrl_w(data);
		break;
	case 6:
		// TODO - Centronics interface
		break;
	case 7:
		// TODO - Centronics interface
		break;
	}
}

u8 sigmasoft_parallel_port::video_mem_r()
{
	// get video memory value from igc device
	return m_connected_tlbc ? m_tlbc->sigma_video_mem_r() : 0;
}

u8 sigmasoft_parallel_port::ctrl_r()
{
	// get control register from igc device
	return m_connected_tlbc ? m_tlbc->sigma_ctrl_r() : 0;
}

u8 sigmasoft_parallel_port::read(u8 select_lines, u16 offset)
{
	u8 value = 0;

	if (!card_selected(select_lines, offset))
	{
		return value;
	}

	offset &= 0x07;

	switch (offset)
	{
	case 0:
		value = video_mem_r();
		break;
	case 1:
		// TODO - Light Pen Low address
		break;
	case 2:
		// TODO - Light Pen High address
		break;
	case 3:
		// TODO - Left input device
		break;
	case 4:
		// TODO - Right input device
		break;
	case 5:
		// IGC Control Register
		value = ctrl_r();
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
}

void sigmasoft_parallel_port::device_reset()
{
	ioport_value const jumpers(m_jumpers->read());

	m_enabled = bool(jumpers & 0x20);

	m_base_addr = (jumpers & 0x1f) << 3;

	m_connected_tlbc = bool(jumpers & 0x40);
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

	PORT_CONFNAME(0x40, 0x40, "Connected to SigmaSoft IGC")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x40, DEF_STR( Yes ))

INPUT_PORTS_END


ioport_constructor sigmasoft_parallel_port::device_input_ports() const
{
	return INPUT_PORTS_NAME(sigmasoft_parallel_port_device);
}

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_SIGMASOFT_PARALLEL, device_h89bus_left_card_interface, sigmasoft_parallel_port, "sigmasoft_parallel_port", "SigmaSoft Universal Parallel Board");
