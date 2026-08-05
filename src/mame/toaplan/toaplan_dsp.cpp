// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench, Stephane Humbert

#include "emu.h"
#include "toaplan_dsp.h"

#define LOG_DSP (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TOAPLAN_DSP, toaplan_dsp_device, "toaplan_dsp", "Toaplan TMS320C10 DSP Interface")

toaplan_dsp_device::toaplan_dsp_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TOAPLAN_DSP, tag, owner, clock)
	, m_dsp(*this, "dsp")
	, m_host_addr_cb(*this)
	, m_host_r_cb(*this)
	, m_host_w_cb(*this)
	, m_halt_cb(*this)
	, m_dsp_on(0)
	, m_dsp_bio(0)
	, m_dsp_execute(false)
	, m_dsp_addr_w(0)
	, m_main_ram_seg(0)
{
}

// DSP memory map

void toaplan_dsp_device::program_map(address_map &map)
{
	// Internal ROM size is actually 0x600 words (0xc00 bytes),
	// DSP ROM size of most bootlegs is 0x800 words (0x1000 bytes)
	map(0x000, 0x7ff).rom().region("dsp", 0);
}

// 0x00 - 0x8f  TMS320C10 Internal Data RAM in Data Address Space

void toaplan_dsp_device::io_map(address_map &map)
{
	map(0x0, 0x0).w(FUNC(toaplan_dsp_device::dsp_addrsel_w));
	map(0x1, 0x1).rw(FUNC(toaplan_dsp_device::dsp_r), FUNC(toaplan_dsp_device::dsp_w));
	map(0x3, 0x3).w(FUNC(toaplan_dsp_device::dsp_bio_w));
}


void toaplan_dsp_device::device_add_mconfig(machine_config &config)
{
	TMS320C10(config, m_dsp, DERIVED_CLOCK(1, 1));
	m_dsp->set_addrmap(AS_PROGRAM, &toaplan_dsp_device::program_map);
	m_dsp->set_addrmap(AS_IO, &toaplan_dsp_device::io_map);
	m_dsp->bio().set(FUNC(toaplan_dsp_device::bio_r));
}

void toaplan_dsp_device::device_start()
{
	m_host_addr_cb.resolve_safe();
	m_host_r_cb.resolve_safe(0);
	m_host_w_cb.resolve_safe(true);

	save_item(NAME(m_dsp_on));
	save_item(NAME(m_dsp_addr_w));
	save_item(NAME(m_main_ram_seg));
	save_item(NAME(m_dsp_bio));
	save_item(NAME(m_dsp_execute));
}

void toaplan_dsp_device::device_reset()
{
	m_dsp_addr_w = 0;
	m_main_ram_seg = 0;
	m_dsp_execute = false;
	m_dsp_bio = CLEAR_LINE;
}

void toaplan_dsp_device::device_post_load()
{
	dsp_int_w(m_dsp_on);
}


void toaplan_dsp_device::dsp_addrsel_w(u16 data)
{
	m_host_addr_cb(data, m_main_ram_seg, m_dsp_addr_w);
	LOGMASKED(LOG_DSP, "%s: dsp_addrsel_w %04x (%08x)\n", machine().describe_context(), data, m_main_ram_seg + m_dsp_addr_w);
}

u16 toaplan_dsp_device::dsp_r()
{
	// DSP can read data from main CPU RAM via DSP IO port 1

	const u16 input_data = m_host_r_cb(m_main_ram_seg, m_dsp_addr_w);
	if (!machine().side_effects_disabled())
		LOGMASKED(LOG_DSP, "%s: dsp_r %04x at %08x\n", machine().describe_context(), input_data, m_main_ram_seg + m_dsp_addr_w);
	return input_data;
}

void toaplan_dsp_device::dsp_w(u16 data)
{
	// Data written to main CPU RAM via DSP IO port 1

	m_dsp_execute = m_host_w_cb(m_main_ram_seg, m_dsp_addr_w, data);
	LOGMASKED(LOG_DSP, "%s: dsp_w %04x at %08x\n", machine().describe_context(), data, m_main_ram_seg + m_dsp_addr_w);
}

void toaplan_dsp_device::dsp_bio_w(u16 data)
{
	// data 0xffff  means inhibit BIO line to DSP and enable
	//              communication to main processor
	//              Actually only DSP data bit 15 controls this
	// data 0x0000  means set DSP BIO line active and disable
	//              communication to main processor

	LOGMASKED(LOG_DSP, "%s: dsp_bio_w %04x\n", machine().describe_context(), data);
	if (BIT(data, 15))
		m_dsp_bio = CLEAR_LINE;

	if (data == 0)
	{
		if (m_dsp_execute)
		{
			LOGMASKED(LOG_DSP, "Turning the host CPU on\n");
			m_halt_cb(CLEAR_LINE);
			m_dsp_execute = false;
		}
		m_dsp_bio = ASSERT_LINE;
	}
}

int toaplan_dsp_device::bio_r()
{
	return m_dsp_bio;
}

// host interface
void toaplan_dsp_device::dsp_int_w(int enable)
{
	m_dsp_on = enable;
	if (enable)
	{
		LOGMASKED(LOG_DSP, "Turning DSP on and the host CPU off\n");
		m_dsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_dsp->set_input_line(0, ASSERT_LINE); /* TMS32010 INT */
		m_halt_cb(ASSERT_LINE);
	}
	else
	{
		LOGMASKED(LOG_DSP, "Turning DSP off\n");
		m_dsp->set_input_line(0, CLEAR_LINE); /* TMS32010 INT */
		m_dsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}
