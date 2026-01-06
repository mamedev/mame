// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton device for Roland JX-8P and Super JX synthesizer boards.

****************************************************************************/

#include "emu.h"
#include "jx8p_synth.h"

DEFINE_DEVICE_TYPE(JX8P_SYNTH, jx8p_synth_device, "jx8p_synth", "Roland JX-8P Synthesizer Board")
DEFINE_DEVICE_TYPE(SUPERJX_SYNTH, superjx_synth_device, "superjx_synth", "Roland Super JX Synthesizer Board")


jx8p_synth_device::jx8p_synth_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_synthcpu(*this, "synthcpu")
	, m_ramio(*this, "ramio")
	, m_pit(*this, "pit%u", 1U)
{
}

jx8p_synth_device::jx8p_synth_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jx8p_synth_device(mconfig, JX8P_SYNTH, tag, owner, clock)
{
}

superjx_synth_device::superjx_synth_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: jx8p_synth_device(mconfig, SUPERJX_SYNTH, tag, owner, clock)
{
}

void jx8p_synth_device::device_start()
{
}


void jx8p_synth_device::prescale_w(u8 data)
{
	double clockin = clock() / (16.0 - (data & 0x0f));
	for (int n = 0; n < 2; n++)
	{
		m_pit[n]->set_clockin(0, clockin);
		m_pit[n]->set_clockin(1, clockin);
		m_pit[n]->set_clockin(2, clockin);
	}

	u32 timerin = clock() / (16 - ((data & 0xf0) >> 4));
	m_ramio->set_unscaled_clock(timerin);
}

void jx8p_synth_device::adc_w(offs_t offset, u8 data)
{
}

void jx8p_synth_device::prog_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x3fff).rom().region("program", 0);
}

void jx8p_synth_device::ext_map(address_map &map)
{
	map(0x0000, 0x00ff).mirror(0x3800).rw(m_ramio, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x0100, 0x0107).mirror(0x38f8).rw(m_ramio, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x0200, 0x0203).mirror(0x38fc).w(m_pit[0], FUNC(pit8253_device::write));
	map(0x0300, 0x0303).mirror(0x38fc).w(m_pit[1], FUNC(pit8253_device::write));
	map(0x0400, 0x0403).mirror(0x38fc).w(m_pit[2], FUNC(pit8253_device::write));
	map(0x0500, 0x0503).mirror(0x38fc).w(m_pit[3], FUNC(pit8253_device::write));
	map(0x0600, 0x0600).select(0xf0).mirror(0x380f).w(FUNC(jx8p_synth_device::adc_w));
}

void jx8p_synth_device::device_add_mconfig(machine_config &config)
{
	I8031(config, m_synthcpu, 12_MHz_XTAL); // 8051-319 (EA tied to GND)
	m_synthcpu->set_addrmap(AS_PROGRAM, &jx8p_synth_device::prog_map);
	m_synthcpu->set_addrmap(AS_DATA, &jx8p_synth_device::ext_map);

	I8155(config, m_ramio, 0); // µPD8155HC
	m_ramio->out_pa_callback().set(FUNC(jx8p_synth_device::prescale_w));

	for (auto &pit : m_pit)
		PIT8253(config, pit); // µPD8253C-2
}

void superjx_synth_device::device_add_mconfig(machine_config &config)
{
	I8031(config, m_synthcpu, 12_MHz_XTAL); // MBL8031AH-P-G
	m_synthcpu->set_addrmap(AS_PROGRAM, &superjx_synth_device::prog_map);
	m_synthcpu->set_addrmap(AS_DATA, &superjx_synth_device::ext_map);

	I8155(config, m_ramio, 0); // µPD8155HC-2
	m_ramio->out_pa_callback().set(FUNC(superjx_synth_device::prescale_w));

	for (auto &pit : m_pit)
		PIT8254(config, pit); // M5M82C54 P-6-D-1
}
