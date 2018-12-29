// license:BSD-3-Clause
// copyright-holders:hap
/*

  TMS1000 family - TMS0270

*/

#include "emu.h"
#include "tms0270.h"
#include "debugger.h"

// TMS0270 is a TMS0980 with earrings and a new hat. The new changes look like a quick afterthought, almost hacky
// - RAM, ROM, and main instructions PLAs is the same as TMS0980
// - 64-term microinstructions PLA between the RAM and ROM, similar to TMS0980,
//   plus optional separate lines for custom opcode handling
// - 48-term output PLA above the RAM (rotate opla 90 degrees)
DEFINE_DEVICE_TYPE(TMS0270, tms0270_cpu_device, "tms0270", "Texas Instruments TMS0270") // 40-pin DIP, 16 O pins, 8+ R pins (some R pins are internally hooked up to support more I/O)
// newer TMS0270 chips (eg. Speak & Math) have 42 pins

// TMS0260 is same or similar?


// device definitions
tms0270_cpu_device::tms0270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms0980_cpu_device(mconfig, TMS0270, tag, owner, clock, 16 /* o pins */, 16 /* r pins */, 7 /* pc bits */, 9 /* byte width */, 4 /* x width */, 11 /* prg width */, address_map_constructor(FUNC(tms0270_cpu_device::program_11bit_9), this), 8 /* data width */, address_map_constructor(FUNC(tms0270_cpu_device::data_144x4), this))
	, m_read_ctl(*this)
	, m_write_ctl(*this)
	, m_write_pdc(*this)
{
}


// machine configs
void tms0270_cpu_device::device_add_mconfig(machine_config &config)
{
	// main opcodes PLA, microinstructions PLA, output PLA
	PLA(config, "ipla", 9, 22, 24).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "mpla", 6, 22, 64).set_format(pla_device::FMT::BERKELEY);
	PLA(config, "opla", 6, 16, 48).set_format(pla_device::FMT::BERKELEY);
}


// device_start/reset
void tms0270_cpu_device::device_start()
{
	// common init
	tms1k_base_device::device_start();

	m_read_ctl.resolve_safe(0);
	m_write_ctl.resolve_safe();
	m_write_pdc.resolve_safe();

	// zerofill
	m_r_prev = 0;
	m_chipsel = 0;
	m_ctl_dir = 0;
	m_ctl_out = 0;
	m_pdc = -1; // !

	m_o_latch_low = 0;
	m_o_latch = 0;
	m_o_latch_prev = 0;

	// register for savestates
	save_item(NAME(m_r_prev));
	save_item(NAME(m_chipsel));
	save_item(NAME(m_ctl_dir));
	save_item(NAME(m_ctl_out));
	save_item(NAME(m_pdc));

	save_item(NAME(m_o_latch_low));
	save_item(NAME(m_o_latch));
	save_item(NAME(m_o_latch_prev));
}

void tms0270_cpu_device::device_reset()
{
	// common reset
	tms0980_cpu_device::device_reset();

	m_o_latch_low = 0;
	m_o_latch = 0;
	m_o_latch_prev = 0;
}


// i/o handling
void tms0270_cpu_device::dynamic_output()
{
	// R11: TMS5100 CTL port direction (0=read from TMS5100, 1=write to TMS5100)
	m_ctl_dir = m_r >> 11 & 1;

	// R12: chip select (off=display via OPLA, on=TMS5100 via ACC/CKB)
	m_chipsel = m_r >> 12 & 1;

	if (m_chipsel)
	{
		// ACC via SEG G,B,C,D: TMS5100 CTL pins
		if (m_ctl_dir && m_a != m_ctl_out)
		{
			m_ctl_out = m_a;
			m_write_ctl(0, m_ctl_out, 0xff);
		}

		// R10 via SEG E: TMS5100 PDC pin
		if (m_pdc != (m_r >> 10 & 1))
		{
			m_pdc = m_r >> 10 & 1;
			m_write_pdc(m_pdc);
		}
	}
	else
	{
		// standard O-output
		if (m_o_latch != m_o_latch_prev)
		{
			write_o_output(m_o_latch);
			m_o_latch_prev = m_o_latch;
		}
	}

	// standard R-output
	if (m_r != m_r_prev)
	{
		m_write_r(0, m_r & m_r_mask, 0xffff);
		m_r_prev = m_r;
	}
}

u8 tms0270_cpu_device::read_k_input()
{
	// external: TMS5100 CTL port via SEG G,B,C,D
	if (m_chipsel)
		return (m_ctl_dir) ? m_ctl_out : m_read_ctl(0, 0xff) & 0xf;

	// standard K-input otherwise
	u8 k = m_read_k(0, 0xff) & 0x1f;
	return (k & 0x10) ? 0xf : k; // the TMS0270 KF line asserts all K-inputs
}


// opcode deviations
void tms0270_cpu_device::op_setr()
{
	// same as default, but handle write to output in dynamic_output
	m_r = m_r | (1 << m_y);
}

void tms0270_cpu_device::op_rstr()
{
	// same as default, but handle write to output in dynamic_output
	m_r = m_r & ~(1 << m_y);
}

void tms0270_cpu_device::op_tdo()
{
	// TDO: transfer data out
	if (m_status)
		m_o_latch_low = m_a;
	else
		m_o_latch = m_o_latch_low | (m_a << 4 & 0x30);

	// write to output is done in dynamic_output
}
