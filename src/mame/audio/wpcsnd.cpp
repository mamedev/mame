// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpcsnd.c - Williams WPC pinball sound
 *   M6809E + YM2151 + HC55516 + DAC
 *
 *  Created on: 4/10/2013
 */

#include "emu.h"
#include "wpcsnd.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(WPCSND, wpcsnd_device, "wpcsnd", "Williams WPC Sound")

wpcsnd_device::wpcsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WPCSND, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "bgcpu")
	, m_ym2151(*this, "ym2151")
	, m_hc55516(*this, "hc55516")
	, m_cpubank(*this, "rombank")
	, m_fixedbank(*this, "fixed")
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_reply_cb(*this)
{
}

void wpcsnd_device::wpcsnd_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).mirror(0x03ff).w(FUNC(wpcsnd_device::rombank_w));
	map(0x2400, 0x2401).mirror(0x03fe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x2800, 0x2800).mirror(0x03ff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x2c00, 0x2fff).w(FUNC(wpcsnd_device::bg_speech_digit_w));
	map(0x3000, 0x33ff).r(FUNC(wpcsnd_device::latch_r));
	map(0x3400, 0x37ff).w(FUNC(wpcsnd_device::bg_speech_clock_w));
	map(0x3800, 0x3bff).w(FUNC(wpcsnd_device::volume_w));
	map(0x3c00, 0x3fff).w(FUNC(wpcsnd_device::latch_w));
	map(0x4000, 0xbfff).bankr("rombank");
	map(0xc000, 0xffff).bankr("fixed");
}

void wpcsnd_device::ctrl_w(uint8_t data)
{
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void wpcsnd_device::data_w(uint8_t data)
{
	m_latch = data;
	m_cpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
}

uint8_t wpcsnd_device::ctrl_r()
{
	return (m_reply_available) ? 0x01 : 0x00;
}

uint8_t wpcsnd_device::data_r()
{
	m_reply_available = false;
	m_reply_cb(m_cpu->space(AS_PROGRAM),0);
	return m_reply;
}

void wpcsnd_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, XTAL(8'000'000) / 4); // MC68B09E
	m_cpu->set_addrmap(AS_PROGRAM, &wpcsnd_device::wpcsnd_map);
	config.set_maximum_quantum(attotime::from_hz(50));

	YM2151(config, m_ym2151, 3580000);
	m_ym2151->irq_handler().set(FUNC(wpcsnd_device::ym2151_irq_w));
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.25);

	AD7524(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, *this, 0.5);
}


void wpcsnd_device::device_start()
{
	// resolve callback
	m_reply_cb.resolve_safe();
}

void wpcsnd_device::device_reset()
{
	uint8_t* ROM = m_rom->base();
	m_cpubank->configure_entries(0, 0x80, &ROM[0], 0x8000);
	m_cpubank->set_entry(0);
	m_fixedbank->configure_entries(0, 1, &ROM[0x17c000], 0x4000);
	m_fixedbank->set_entry(0);

	// reset the CPU again, so that the CPU is starting with the right vectors (otherwise sound may die on reset)
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	m_reply_available = false;
}

WRITE_LINE_MEMBER( wpcsnd_device::ym2151_irq_w)
{
	m_cpu->set_input_line(M6809_FIRQ_LINE,state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER( wpcsnd_device::bg_speech_clock_w )
{
	// pulses clock input?
	m_hc55516->clock_w(1);
	m_hc55516->clock_w(0);
}

WRITE8_MEMBER( wpcsnd_device::bg_speech_digit_w )
{
	m_hc55516->digit_w(data);
}

WRITE8_MEMBER( wpcsnd_device::rombank_w )
{
	uint8_t bank = data & 0x0f;

	switch((~data) & 0xe0)
	{
	case 0x80:
		bank |= 0x20;
		break;
	case 0x40:
		bank |= 0x10;
		break;
	case 0x20:
		break;
	}

	m_cpubank->set_entry(bank);

	LOG("WPCSND: Bank set to %02x\n",bank);
}

READ8_MEMBER(wpcsnd_device::latch_r)
{
	m_cpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
	return m_latch;
}

WRITE8_MEMBER(wpcsnd_device::latch_w)
{
	m_reply_available = true;
	m_reply = data;
	m_reply_cb(space,1);
}

WRITE8_MEMBER(wpcsnd_device::volume_w)
{
}
