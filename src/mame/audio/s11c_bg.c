// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * s11c_bg.c - Williams System 11C background music (M68B09E + YM2151 + HC55516 + DAC)
 *
 *  Created on: 2/10/2013
 *      Author: bsr
 */

#include "s11c_bg.h"


const device_type S11C_BG = &device_creator<s11c_bg_device>;

s11c_bg_device::s11c_bg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig,S11C_BG,"Williams System 11C Background Music",tag,owner,clock, "s11c_bg", __FILE__),
		m_cpu(*this,"bgcpu"),
		m_ym2151(*this,"ym2151"),
		m_hc55516(*this,"hc55516_bg"),
		m_dac1(*this,"dac1"),
		m_pia40(*this,"pia40"),
		m_cpubank(*this,"bgbank")
{
}

static ADDRESS_MAP_START( s11c_bg_map, AS_PROGRAM, 8, s11c_bg_device )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x2001) AM_MIRROR(0x1ffe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x4000, 0x4003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE("pia40", pia6821_device, read, write)
	AM_RANGE(0x6000, 0x67ff) AM_WRITE(bg_speech_digit_w)
	AM_RANGE(0x6800, 0x6fff) AM_WRITE(bg_speech_clock_w)
	AM_RANGE(0x7800, 0x7fff) AM_WRITE(bgbank_w)
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bgbank")
ADDRESS_MAP_END

WRITE8_MEMBER( s11c_bg_device::pia40_pa_w )
{
	m_dac1->write_unsigned8(data);
}

WRITE_LINE_MEMBER( s11c_bg_device::pia40_cb2_w)
{
//  m_pia34->cb1_w(state);  // To Widget MCB1 through CPU Data interface
}

WRITE8_MEMBER( s11c_bg_device::pia40_pb_w )
{
//  m_pia34->portb_w(data);
}

WRITE_LINE_MEMBER( s11c_bg_device::pia40_ca2_w)
{
	if(state == ASSERT_LINE)
		m_ym2151->reset();
}

void s11c_bg_device::ctrl_w(UINT8 data)
{
	m_pia40->cb1_w(data);
}

void s11c_bg_device::data_w(UINT8 data)
{
	m_pia40->portb_w(data);
}

MACHINE_CONFIG_FRAGMENT( s11c_bg )
	MCFG_CPU_ADD("bgcpu", M6809E, XTAL_8MHz) // MC68B09E (note: schematics show this as 8mhz/2, but games crash very quickly with that speed?)
	MCFG_CPU_PROGRAM_MAP(s11c_bg_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(s11c_bg_device, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.25)

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_SOUND_ADD("hc55516_bg", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_DEVICE_ADD("pia40", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(s11c_bg_device, pia40_pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(s11c_bg_device, pia40_pb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(s11c_bg_device, pia40_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(s11c_bg_device, pia40_cb2_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("bgcpu", m6809e_device, firq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("bgcpu", m6809e_device, nmi_line))
MACHINE_CONFIG_END

machine_config_constructor s11c_bg_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( s11c_bg );
}

void s11c_bg_device::device_start()
{
}

void s11c_bg_device::device_reset()
{
	UINT8* ROM;

	m_rom = memregion(m_regiontag);
	ROM = m_rom->base();
	m_cpubank->configure_entries(0, 8, &ROM[0x10000], 0x8000);
	m_cpubank->set_entry(0);
	// reset the CPU again, so that the CPU are starting with the right vectors (otherwise sound may die on reset)
	m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);
}

void s11c_bg_device::static_set_gfxregion(device_t &device, const char *tag)
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
	UINT8 bank = ((data & 0x04) >> 2) | ((data & 0x03) << 1);
	m_cpubank->set_entry(bank);
}
