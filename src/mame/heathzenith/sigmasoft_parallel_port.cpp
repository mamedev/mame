// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  SigmaSoft Universal Parallel Interface Board


****************************************************************************/

#include "emu.h"

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


DEFINE_DEVICE_TYPE(SIGMASOFT_PARALLEL_PORT, sigmasoft_parallel_port, "sigmasoft_parallel_port", "SigmaSoft Universal Parallel Board");

sigmasoft_parallel_port::sigmasoft_parallel_port(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, SIGMASOFT_PARALLEL_PORT, tag, owner, clock),
	m_ctrl_r(*this, 0x00),
	m_video_mem_r(*this, 0x00),
	m_video_mem_w(*this),
	m_io_lo_addr(*this),
	m_io_hi_addr(*this),
	m_window_lo_addr(*this),
	m_window_hi_addr(*this),
	m_ctrl_w(*this)
{
}

void sigmasoft_parallel_port::video_mem_w(u8 val)
{
	m_video_mem_w(val);
}

void sigmasoft_parallel_port::io_lo_addr_w(u8 val)
{
	m_io_lo_addr(val);
}

void sigmasoft_parallel_port::io_hi_addr_w(u8 val)
{
	m_io_hi_addr(val);
}

void sigmasoft_parallel_port::window_lo_addr_w(u8 val)
{
	m_window_lo_addr(val);
}

void sigmasoft_parallel_port::window_hi_addr_w(u8 val)
{
	m_window_hi_addr(val);
}

void sigmasoft_parallel_port::ctrl_w(u8 val)
{
	m_ctrl_w(val);
}

void sigmasoft_parallel_port::write(offs_t reg, u8 val)
{
	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, reg, val);

	switch (reg)
	{
	case 0:
		video_mem_w(val);
		break;
	case 1:
		io_lo_addr_w(val);
		break;
	case 2:
		io_hi_addr_w(val);
		break;
	case 3:
		window_lo_addr_w(val);
		break;
	case 4:
		window_hi_addr_w(val);
		break;
	case 5:
		ctrl_w(val);
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
	return m_video_mem_r();
}

u8 sigmasoft_parallel_port::ctrl_r()
{
	// get control register from igc device
	return m_ctrl_r();
}

u8 sigmasoft_parallel_port::read(offs_t reg)
{
	// default return for the h89
	u8 value = 0xff;

	switch (reg)
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
		// Control Register
		value = ctrl_r();
		break;
	case 6:
		// TODO - Centronics interface
		break;
	case 7:
		// TODO - Centronics interface
		break;
	}

	LOGFUNC("%s: reg: %d val: %d\n", FUNCNAME, reg, value);

	return value;
}

void sigmasoft_parallel_port::device_start()
{
}
