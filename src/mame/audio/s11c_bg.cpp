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
{
}

ADDRESS_MAP_START(s11c_bg_device::s11c_bg_map)
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("pia40", pia6821_device, read, write)
	AM_RANGE(0x6000, 0x67ff) AM_WRITE(bg_speech_digit_w)
	AM_RANGE(0x6800, 0x6fff) AM_WRITE(bg_speech_clock_w)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(bgbank_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bgbank")
ADDRESS_MAP_END

WRITE_LINE_MEMBER( s11c_bg_device::pia40_cb2_w)
{
//  m_pia34->cb1_w(state);  // To Widget MCB1 through CPU Data interface
}

WRITE8_MEMBER( s11c_bg_device::pia40_pb_w )
{
//  m_pia34->portb_w(data);
}

void s11c_bg_device::ctrl_w(uint8_t data)
{
	m_pia40->cb1_w(data);
}

void s11c_bg_device::data_w(uint8_t data)
{
	m_pia40->portb_w(data);
}

MACHINE_CONFIG_START(s11c_bg_device::device_add_mconfig)
	MCFG_CPU_ADD("bgcpu", MC6809E, XTAL(8'000'000) / 4) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(s11c_bg_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	MCFG_YM2151_ADD("ym2151", XTAL(3'579'545)) // "3.58 MHz" on schematics and parts list
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(s11c_bg_device, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.25)

	MCFG_SOUND_ADD("dac", MC1408, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	MCFG_SOUND_ADD("hc55516_bg", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5)

	MCFG_DEVICE_ADD("pia40", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("dac", dac_byte_interface, write))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s11c_bg_device, pia40_pb_w))
	MCFG_PIA_CA2_HANDLER(DEVWRITELINE("ym2151", ym2151_device, reset_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s11c_bg_device, pia40_cb2_w))
	MCFG_PIA_IRQA_HANDLER(INPUTLINE("bgcpu", M6809_FIRQ_LINE))
	MCFG_PIA_IRQB_HANDLER(INPUTLINE("bgcpu", INPUT_LINE_NMI))
MACHINE_CONFIG_END

void s11c_bg_device::device_start()
{
}

void s11c_bg_device::device_reset()
{
	uint8_t* ROM;

	m_rom = memregion(m_regiontag);
	ROM = m_rom->base();
	m_cpubank->configure_entries(0, 8, &ROM[0x10000], 0x8000);
	m_cpubank->set_entry(0);
	// reset the CPU again, so that the CPU are starting with the right vectors (otherwise sound may die on reset)
	m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);
}

void s11c_bg_device::static_set_romregion(device_t &device, const char *tag)
{
	s11c_bg_device &cpuboard = downcast<s11c_bg_device &>(device);
	cpuboard.m_regiontag = tag;
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
