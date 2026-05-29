// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl_timebase.h"

#define LOG_TIMEBASE (1U << 1)

#define VERBOSE     (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(GPL_TIMEBASE, gpl_timebase_device, "gpl_timebase", "Generalplus GPL162xx / GPL951xx System Timebase")

gpl_timebase_device::gpl_timebase_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GPL_TIMEBASE, tag, owner, clock),
	m_timebasetimer(*this, { "timebase_a", "timebase_b", "timebase_c" }),
	m_updateirqs_cb(*this)
{
}

void gpl_timebase_device::device_start()
{
	save_item(NAME(m_timebase_ctrl));
	save_item(NAME(m_timebase_reset));
}

void gpl_timebase_device::device_reset()
{
	m_timebase_ctrl[0] = 0x0000;
	m_timebase_ctrl[1] = 0x0000;
	m_timebase_ctrl[2] = 0x0000;
	m_timebase_reset = 0x0000;
}

attotime gpl_timebase_device::get_timer_period(int timer, int ctrlval)
{
	int which = (timer << 2) | ctrlval;

	if (which != 0) // timebase_a, fequency 0 is 'off'
	{
		int period = 1 << (which - 1);
		return attotime::from_hz(period);
	}

	return attotime::never;
}

// Timebase ('fixed' frequency timers)
// (each can select from 3 different frequencies, different for each timer)

template <u16 Timer>
TIMER_DEVICE_CALLBACK_MEMBER( gpl_timebase_device::timebase_cb )
{
	// sets bit 15 in m_timebase_ctrl[2], also visible (as read only) in P_INT_Status2, bit 8
	// uses IRQ7 (FIQ option not available)
	m_timebase_ctrl[Timer] |= 0x8000;
	m_updateirqs_cb(1);
}

// P_TimeBaseA_Ctrl
//
// 15  TMBAIF/C
// 14  TMBAIE
// 13  TMBAEN
// 12
//
// 11
// 10
//  9
//  8
//  7
//  6
//  5
//  4
//  3
//  2
//  1  TMBAS[1]
//  0  TMBAS[0]  00 = Reserved, 01 = 1Hz, 10 = 2Hz, 11 = 4Hz

template <u16 Timer>
u16 gpl_timebase_device::timebase_ctrl_r()
{
	LOGMASKED(LOG_TIMEBASE, "%s:sunplus_gcm394_base_device::timebase%c_ctrl_r\n", machine().describe_context(), 'a'+Timer);
	return m_timebase_ctrl[Timer];
}

template <u16 Timer>
void gpl_timebase_device::timebase_ctrl_w(u16 data)
{
	LOGMASKED(LOG_TIMEBASE, "%s:sunplus_gcm394_base_device::timebase%c_ctrl_w %04x\n", machine().describe_context(), 'a'+Timer, data);

	if (data & 0x8000)
	{
		m_timebase_ctrl[Timer] = data & 0x7fff;
		m_updateirqs_cb(1);
	}
	else
	{
		m_timebase_ctrl[Timer] = data;
	}

	if (m_timebase_ctrl[Timer] & 0x2000)
	{
		attotime period = get_timer_period(Timer, m_timebase_ctrl[Timer] & 0x0003);
		m_timebasetimer[Timer]->adjust(period, 0, period);
	}
	else
	{
		m_timebasetimer[Timer]->adjust(attotime::never);
	}
}

void gpl_timebase_device::timebase_reset_w(u16 data)
{
	LOGMASKED(LOG_TIMEBASE, "%s:sunplus_gcm394_base_device::timebase_reset_w %04x\n", machine().describe_context(), data);
	m_timebase_reset = data;
}

u16 gpl_timebase_device::timebasea_ctrl_r() { return timebase_ctrl_r<0>(); }
u16 gpl_timebase_device::timebaseb_ctrl_r() { return timebase_ctrl_r<1>(); }
u16 gpl_timebase_device::timebasec_ctrl_r() { return timebase_ctrl_r<2>(); }

void gpl_timebase_device::timebasea_ctrl_w(u16 data) { timebase_ctrl_w<0>(data); }
void gpl_timebase_device::timebaseb_ctrl_w(u16 data) { timebase_ctrl_w<1>(data); }
void gpl_timebase_device::timebasec_ctrl_w(u16 data) { timebase_ctrl_w<2>(data); }

void gpl_timebase_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, m_timebasetimer[0]).configure_generic(FUNC(gpl_timebase_device::timebase_cb<0>));
	TIMER(config, m_timebasetimer[1]).configure_generic(FUNC(gpl_timebase_device::timebase_cb<1>));
	TIMER(config, m_timebasetimer[2]).configure_generic(FUNC(gpl_timebase_device::timebase_cb<2>));
}
