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

#define LOG_WPCSND (0)

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

ADDRESS_MAP_START(wpcsnd_device::wpcsnd_map)
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_WRITE(rombank_w)
	AM_RANGE(0x2400, 0x2401) AM_MIRROR(0x03fe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_DEVWRITE("dac", dac_byte_interface, write)
	AM_RANGE(0x2c00, 0x2fff) AM_WRITE(bg_speech_digit_w)
	AM_RANGE(0x3000, 0x33ff) AM_READ(latch_r)
	AM_RANGE(0x3400, 0x37ff) AM_WRITE(bg_speech_clock_w)
	AM_RANGE(0x3800, 0x3bff) AM_WRITE(volume_w)
	AM_RANGE(0x3c00, 0x3fff) AM_WRITE(latch_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("rombank")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("fixed")
ADDRESS_MAP_END

void wpcsnd_device::ctrl_w(uint8_t data)
{
	m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);
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

MACHINE_CONFIG_START(wpcsnd_device::device_add_mconfig)
	MCFG_CPU_ADD("bgcpu", MC6809E, XTAL(8'000'000) / 4) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(wpcsnd_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(wpcsnd_device, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.25)

	MCFG_SOUND_ADD("dac", AD7524, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)

	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5)
MACHINE_CONFIG_END


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
	m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);

	m_reply_available = false;
}

void wpcsnd_device::static_set_romregion(device_t &device, const char *tag)
{
	wpcsnd_device &cpuboard = downcast<wpcsnd_device &>(device);
	cpuboard.m_rom.set_tag(tag);
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

	if(LOG_WPCSND) logerror("WPCSND: Bank set to %02x\n",bank);
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
