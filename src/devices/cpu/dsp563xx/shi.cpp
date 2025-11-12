// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Serial Host Interface for DSP56362/DSP56364

***************************************************************************/

#include "emu.h"
#include "shi.h"

#include "dsp563xx.h"

// device type definition
DEFINE_DEVICE_TYPE(DSP5636X_SHI, dsp5636x_shi_device, "dsp5636x_shi", "DSP5636x Serial Host Interface")


dsp5636x_shi_device::dsp5636x_shi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DSP5636X_SHI, tag, owner, clock)
	, m_cpu(*this, DEVICE_SELF_OWNER)
	, m_iosr(0)
	, m_hrx{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	, m_htx(0)
{
}

void dsp5636x_shi_device::device_start()
{
	save_item(NAME(m_iosr));
	save_item(NAME(m_hsar));
	save_item(NAME(m_hckr));
	save_item(NAME(m_hcsr));
	save_item(NAME(m_hrx));
	save_item(NAME(m_htx));
}

void dsp5636x_shi_device::device_reset()
{
	m_hsar = 0xb00000; // default slave address = 1011x0x
	m_hckr = 0x000002; // CPHA set, all other bits cleared
	m_hcsr = 0x008200; // HIDLE & HTDE set, all other bits cleared
}


void dsp5636x_shi_device::map(address_map &map)
{
	map(0 * 4 + 0, 0 * 4 + 3).rw(FUNC(dsp5636x_shi_device::hckr_r), FUNC(dsp5636x_shi_device::hckr_w));
	map(1 * 4 + 0, 1 * 4 + 3).rw(FUNC(dsp5636x_shi_device::hcsr_r), FUNC(dsp5636x_shi_device::hcsr_w));
	map(2 * 4 + 0, 2 * 4 + 3).rw(FUNC(dsp5636x_shi_device::hsar_r), FUNC(dsp5636x_shi_device::hsar_w));
	map(3 * 4 + 0, 3 * 4 + 3).w(FUNC(dsp5636x_shi_device::htx_w));
	map(4 * 4 + 0, 4 * 4 + 3).r(FUNC(dsp5636x_shi_device::hrx_r));
}

u32 dsp5636x_shi_device::hckr_r()
{
	return m_hckr;
}

void dsp5636x_shi_device::hckr_w(u32 data)
{
	logerror("%s: HCKR = $%06X\n", machine().describe_context(), data);
	m_hckr = data & 0x0037ff;
}

u32 dsp5636x_shi_device::hcsr_r()
{
	return m_hcsr;
}

void dsp5636x_shi_device::hcsr_w(u32 data)
{
	logerror("%s: HCSR = $%06X\n", machine().describe_context(), data);
	m_hcsr = (m_hcsr & 0xffc000) | (data & 0x003fff);
}

u32 dsp5636x_shi_device::hsar_r()
{
	return m_hsar;
}

void dsp5636x_shi_device::hsar_w(u32 data)
{
	logerror("%s: HSAR = $%06X\n", machine().describe_context(), data);
	m_hsar = data & 0xf40000;
}

void dsp5636x_shi_device::htx_w(u32 data)
{
	logerror("%s: Writing $%06X to HTX\n", machine().describe_context(), data);
}

u32 dsp5636x_shi_device::hrx_r()
{
	u32 data = 0;
	if (!machine().side_effects_disabled())
		logerror("%s: Reading $%06X from HRX\n", machine().describe_context(), data);
	return data;
}
