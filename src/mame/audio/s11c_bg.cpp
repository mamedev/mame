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
{
}

void s11c_bg_device::s11c_bg_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2001).mirror(0x1ffe).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x4003).mirror(0x1ffc).rw("pia40", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x67ff).w(FUNC(s11c_bg_device::bg_speech_digit_w));
	map(0x6800, 0x6fff).w(FUNC(s11c_bg_device::bg_speech_clock_w));
	map(0x7800, 0x7fff).w(FUNC(s11c_bg_device::bgbank_w));
	map(0x8000, 0xffff).bankr("bgbank");
}

WRITE_LINE_MEMBER( s11c_bg_device::pia40_cb2_w)
{
//  m_pia34->cb1_w(state);  // To Widget MCB1 through CPU Data interface
}

WRITE8_MEMBER( s11c_bg_device::pia40_pb_w )
{
//  m_pia34->write_portb(data);
}

void s11c_bg_device::ctrl_w(uint8_t data)
{
	m_pia40->cb1_w(data);
}

void s11c_bg_device::data_w(uint8_t data)
{
	m_pia40->write_portb(data);
}

void s11c_bg_device::device_add_mconfig(machine_config &config)
{
	MC6809E(config, m_cpu, XTAL(8'000'000) / 4); // MC68B09E
	m_cpu->set_addrmap(AS_PROGRAM, &s11c_bg_device::s11c_bg_map);
	config.m_minimum_quantum = attotime::from_hz(50);

	YM2151(config, m_ym2151, XTAL(3'579'545)); // "3.58 MHz" on schematics and parts list
	m_ym2151->irq_handler().set(FUNC(s11c_bg_device::ym2151_irq_w));
	m_ym2151->add_route(ALL_OUTPUTS, *this, 0.25);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.25);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	HC55516(config, m_hc55516, 0).add_route(ALL_OUTPUTS, *this, 0.5);

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
}

void s11c_bg_device::device_reset()
{
	m_cpubank->configure_entries(0, 8, &m_rom[0x10000], 0x8000);
	m_cpubank->set_entry(0);
	// reset the CPU again, so that the CPU are starting with the right vectors (otherwise sound may die on reset)
	m_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

WRITE_LINE_MEMBER( s11c_bg_device::ym2151_irq_w)
{
	if(state == CLEAR_LINE)
		m_pia40->ca1_w(1);
	else
		m_pia40->ca1_w(0);
}

WRITE8_MEMBER( s11c_bg_device::bg_speech_clock_w )
{
	// pulses clock input?
	m_hc55516->clock_w(1);
	m_hc55516->clock_w(0);
}

WRITE8_MEMBER( s11c_bg_device::bg_speech_digit_w )
{
	m_hc55516->digit_w(data);
}

WRITE8_MEMBER( s11c_bg_device::bgbank_w )
{
	uint8_t bank = ((data & 0x04) >> 2) | ((data & 0x03) << 1);
	m_cpubank->set_entry(bank);
}
