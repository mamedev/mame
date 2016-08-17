// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpcsnd.c - Williams WPC pinball sound
 *   M6809E + YM2151 + HC55516 + DAC
 *
 *  Created on: 4/10/2013
 */

#include "wpcsnd.h"

#define LOG_WPCSND (0)

const device_type WPCSND = &device_creator<wpcsnd_device>;

wpcsnd_device::wpcsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig,WPCSND,"Williams WPC Sound",tag,owner,clock, "wpcsnd", __FILE__),
		m_cpu(*this,"bgcpu"),
		m_ym2151(*this,"ym2151"),
		m_hc55516(*this,"hc55516"),
		m_dac(*this,"dac"),
		m_cpubank(*this,"rombank"),
		m_fixedbank(*this,"fixed"),
		m_rom(*this),
		m_reply_cb(*this)
{
}

static ADDRESS_MAP_START( wpcsnd_map, AS_PROGRAM, 8, wpcsnd_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x03ff) AM_WRITE(rombank_w)
	AM_RANGE(0x2400, 0x2401) AM_MIRROR(0x03fe) AM_DEVREADWRITE("ym2151", ym2151_device, read, write)
	AM_RANGE(0x2800, 0x2800) AM_MIRROR(0x03ff) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x2c00, 0x2fff) AM_WRITE(bg_speech_digit_w)
	AM_RANGE(0x3000, 0x33ff) AM_READ(latch_r)
	AM_RANGE(0x3400, 0x37ff) AM_WRITE(bg_speech_clock_w)
	AM_RANGE(0x3800, 0x3bff) AM_WRITE(volume_w)
	AM_RANGE(0x3c00, 0x3fff) AM_WRITE(latch_w)
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("rombank")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("fixed")
ADDRESS_MAP_END

void wpcsnd_device::ctrl_w(UINT8 data)
{
	m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);
}

void wpcsnd_device::data_w(UINT8 data)
{
	m_latch = data;
	m_cpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
}

UINT8 wpcsnd_device::ctrl_r()
{
	return (m_reply_available) ? 0x01 : 0x00;
}

UINT8 wpcsnd_device::data_r()
{
	m_reply_available = false;
	m_reply_cb(m_cpu->space(AS_PROGRAM),0);
	return m_reply;
}

MACHINE_CONFIG_FRAGMENT( wpcsnd )
	MCFG_CPU_ADD("bgcpu", M6809E, XTAL_8MHz) // MC68B09E
	MCFG_CPU_PROGRAM_MAP(wpcsnd_map)
	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	MCFG_SPEAKER_STANDARD_MONO("bg")
	MCFG_YM2151_ADD("ym2151", 3580000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(wpcsnd_device, ym2151_irq_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)

	MCFG_SOUND_ADD("hc55516", HC55516, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "bg", 0.50)
MACHINE_CONFIG_END

machine_config_constructor wpcsnd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wpcsnd );
}

void wpcsnd_device::device_start()
{
	// resolve callback
	m_reply_cb.resolve_safe();
}

void wpcsnd_device::device_reset()
{
	UINT8* ROM = m_rom->base();
	m_cpubank->configure_entries(0, 0x80, &ROM[0], 0x8000);
	m_cpubank->set_entry(0);
	m_fixedbank->configure_entries(0, 1, &ROM[0x17c000], 0x4000);
	m_fixedbank->set_entry(0);

	// reset the CPU again, so that the CPU is starting with the right vectors (otherwise sound may die on reset)
	m_cpu->set_input_line(INPUT_LINE_RESET,PULSE_LINE);

	m_reply_available = false;
}

void wpcsnd_device::static_set_gfxregion(device_t &device, const char *tag)
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
	UINT8 bank = data & 0x0f;

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
