// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Infrared step mats transmit either a preamble, pad bits and parity, or
    one of sixteen codewords. Later step-sensor software accepts the latter
    format alongside its separate serial receiver. The console panel uses
    a separate serial interface.

    The shared infrared generator owns transmission timing and saved state.

    TODO:
    - Measure physical transmitter edge timing and repetition.

***************************************************************************/

#include "emu.h"
#include "xavix_io_ir_step_mat.h"

#include "xavix_io_step_mat.h"

DEFINE_DEVICE_TYPE(XAVIX_IR_STEP_MAT, xavix_ir_step_mat_device, "xavix_ir_step_mat", "XaviX Infrared Step Mat")

xavix_ir_step_mat_device::xavix_ir_step_mat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, XAVIX_IR_STEP_MAT, tag, owner, clock)
	, m_ir(*this, "ir")
	, m_pads(*this, "STEP_SENSOR")
	, m_power(*this, "STEP_POWER")
	, m_protocol(protocol::JMAT)
{
}

ioport_constructor xavix_ir_step_mat_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(xavix_step_sensor);
}

void xavix_ir_step_mat_device::device_add_mconfig(machine_config &config)
{
	XAVIX_IR_REPORT(config, m_ir, DERIVED_CLOCK(1, 1));
	m_ir->set_report_count(1);
	m_ir->set_mode(xavix_ir_report_device::mode::SAMPLE_SEQUENCE);
	m_ir->sequence_callback<0>().set(*this, FUNC(xavix_ir_step_mat_device::mat_report_r));
}

void xavix_ir_step_mat_device::device_config_complete()
{
	// Transitions lie between the ROM's sampling windows; physical edge timing is unmeasured.
	if (m_protocol == protocol::JMAT)
	{
		m_ir.lookup()->set_report_bits<0>(16);
		m_ir.lookup()->set_sample_ticks(JMAT_SAMPLE_TICKS);
		m_ir.lookup()->set_bit_ticks(JMAT_SAMPLE_TICKS.back());
	}
	else
	{
		m_ir.lookup()->set_report_bits<0>(26);
		m_ir.lookup()->set_sample_ticks(AEROSTEP_SAMPLE_TICKS);
		m_ir.lookup()->set_bit_ticks(AEROSTEP_SAMPLE_TICKS.back());
	}
}

void xavix_ir_step_mat_device::device_start()
{
}

void xavix_ir_step_mat_device::vblank_w(int state)
{
	// Once-per-frame reports include released contacts; physical repetition is unmeasured.
	m_ir->vblank_w(state);
}

u16 xavix_ir_step_mat_device::jmat_report(u8 pads)
{
	u32 const enter = BIT(pads, 0);
	u32 const forward = BIT(pads, 1);
	u32 const back = BIT(pads, 2);
	u32 const cancel = BIT(pads, 3);

	// Each pad pair has an inverted-AND guard; four trailing bits check pairwise parity.
	u16 const report = 0x8c00 | (cancel << 9) | ((1 ^ (cancel & back)) << 8) | (back << 7)
			| (forward << 6) | ((1 ^ (forward & enter)) << 5) | (enter << 4)
			| ((1 ^ forward ^ cancel) << 3) | ((enter ^ cancel) << 2)
			| ((1 ^ forward ^ back) << 1) | (enter ^ back);
	return report ^ 0xffff;
}

u32 xavix_ir_step_mat_device::mat_report_r()
{
	if (!BIT(m_power->read(), 0))
		return xavix_ir_report_device::NO_SEQUENCE;

	u8 const pads = m_pads->read() & 0x0f;
	return m_protocol == protocol::JMAT ? jmat_report(pads) : (0x0100'0000 | AEROSTEP_REPORTS[pads]);
}
