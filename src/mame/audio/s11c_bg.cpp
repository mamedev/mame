// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s11c_bg.c - Williams System 11C background music (M68B09E + YM2151 + HC55516 + DAC)
 *
 *  Created on: 2/10/2013
 *      Author: bsr
 */

#include "emu.h"
#include "s11c_bg.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"


DEFINE_DEVICE_TYPE(S11C_BG, s11c_bg_device, "s11c_bg", "Williams System 11C Background Music")

s11c_bg_device::s11c_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, S11C_BG,tag,owner,clock)
	, device_mixer_interface(mconfig, *this)
	, m_cpu(*this, "bgcpu")
	, m_ym2151(*this, "ym2151")
	, m_hc55516(*this, "hc55516_bg")
	, m_pia40(*this, "pia40")
	, m_cpubank(*this, "bgbank")
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_cb2_cb(*this)
	, m_pb_cb(*this)
{
}

void s11c_bg_device::s11c_bg_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x2001).mirror(0x1ffe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x4003).mirror(0x1ffc).rw("pia40", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x6000).mirror(0x07ff).w(FUNC(s11c_bg_device::bg_cvsd_clock_set_w));
	map(0x6800, 0x6800).mirror(0x07ff).w(FUNC(s11c_bg_device::bg_cvsd_digit_clock_clear_w));
	map(0x7800, 0x7800).mirror(0x07ff).w(FUNC(s11c_bg_device::bgbank_w));
	map(0x8000, 0xffff).bankr("bgbank");
}

TIMER_CALLBACK_MEMBER(s11c_bg_device::deferred_cb2_w)
{
	if (!m_cb2_cb.isnull())
		m_cb2_cb(param);
	else
		logerror("S11C_BG CB2 writeback called, but callback is not registered!\n");
}

TIMER_CALLBACK_MEMBER(s11c_bg_device::deferred_pb_w)
{
	if (!m_pb_cb.isnull())
		m_pb_cb(param);
	else
		logerror("S11C_BG PB writeback called, but callback is not registered!\n");
}


WRITE_LINE_MEMBER( s11c_bg_device::pia40_cb2_w)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(s11c_bg_device::deferred_cb2_w),this), state);
}

void s11c_bg_device::pia40_pb_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(s11c_bg_device::deferred_pb_w),this), data);
}


WRITE_LINE_MEMBER( s11c_bg_device::extra_w )
{
	m_pia40->cb2_w(state);
}

WRITE_LINE_MEMBER( s11c_bg_device::ctrl_w )
{
	m_pia40->cb1_w(state);
}

void s11c_bg_device::data_w(uint8_t data)
{
	m_pia40->portb_w(data);
}

void s11c_bg_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, XTAL(8'000'000) / 4); // MC68B09E
	m_cpu->set_addrmap(AS_PROGRAM, &s11c_bg_device::s11c_bg_map);
	config.set_maximum_quantum(attotime::from_hz(50));

	YM2151(config, m_ym2151, XTAL(3'579'545)); // "3.58 MHz" on schematics and parts list
	m_ym2151->irq_handler().set(m_pia40, FUNC(pia6821_device::ca1_w)).invert(); // IRQ is not true state
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.1);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, *this, 0.6);

	PIA6821(config, m_pia40, 0);
	m_pia40->writepa_handler().set("dac", FUNC(dac_byte_interface::data_w));
	m_pia40->writepb_handler().set(FUNC(s11c_bg_device::pia40_pb_w));
	m_pia40->ca2_handler().set(m_ym2151, FUNC(ym2151_device::reset_w));
	m_pia40->cb2_handler().set(FUNC(s11c_bg_device::pia40_cb2_w));
	m_pia40->irqa_handler().set_inputline(m_cpu, M6809_FIRQ_LINE);
	m_pia40->irqb_handler().set_inputline(m_cpu, INPUT_LINE_NMI);
}

void s11c_bg_device::device_start()
{
	/* resolve lines */
	m_cb2_cb.resolve();
	m_pb_cb.resolve();
}

void s11c_bg_device::device_reset()
{
	m_cpubank->configure_entries(0, 8, &m_rom[0x10000], 0x8000);
	m_cpubank->set_entry(0);
	// reset the CPU again, so that the CPU are starting with the right vectors (otherwise sound may die on reset)
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void s11c_bg_device::bg_cvsd_clock_set_w(uint8_t data)
{
	m_hc55516->clock_w(1);
}

void s11c_bg_device::bg_cvsd_digit_clock_clear_w(uint8_t data)
{
	m_hc55516->clock_w(0);
	m_hc55516->digit_w(data&1);
}

void s11c_bg_device::bgbank_w(uint8_t data)
{
	uint8_t bank = ((data & 0x04) >> 2) | ((data & 0x03) << 1);
	m_cpubank->set_entry(bank);
}
