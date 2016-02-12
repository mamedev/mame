// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#include "emu.h"
#include "audio/zaccaria.h"

#include "cpu/m6800/m6800.h"
#include "machine/clock.h"
#include "sound/dac.h"


device_type const ZACCARIA_1B11142 = &device_creator<zac1b11142_audio_device>;


/*
 * slave sound cpu, produces music and sound effects
 * mapping:
 * A15 A14 A13 A12 A11 A10 A09 A08 A07 A06 A05 A04 A03 A02 A01 A00
 *  0   0   0   0   0   0   0   0   0   *   *   *   *   *   *   *  RW 6802 internal ram
 *  0   0   0   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus (for area that doesn't overlap ram)
 *  0   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
 *  0   1   0   x   x   x   x   x   x   x   x   x   0   0   x   x  Open bus
 *  0   1   0   x   x   x   x   x   x   x   x   x   0   1   x   x  Open bus
 *  0   1   0   x   x   x   x   x   x   x   x   x   1   0   x   x  Open bus
 *  0   1   0   x   x   x   x   x   x   x   x   x   1   1   *   *  RW 6821 PIA @ 4I
 *  0   1   1   x   x   x   x   x   x   x   x   x   x   x   x   x  Open bus
 *  1   0   %   %   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS4A: Enable ROM 13
 *  1   1   %   %   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS5A: Enable ROM 9
 *  note that the % bits go to pins 2 (6802 A12) and 26 (6802 A13) of the roms
 *  monymony and jackrabt both use 2764 roms, which use pin 2 as A12 and pin 26 as N/C don't care
 *  hence for actual chips used, the mem map is:
 *  1   0   x   *   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS4A: Enable ROM 13
 *  1   1   x   *   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS5A: Enable ROM 9
 *
 * 6821 PIA:
 * CA1 comes from the master sound cpu's latch bit 7 (which is also connected to the AY chip at 4G's IOB1)
 * CB1 comes from the 6802's clock divided by 4096*2 (about 437Hz)
 * CA2 and CB2 are not connected
 * PA0-7 connect to the data busses of the AY-3-8910 chips
 * PB0 and PB1 connect to the BC1 and BDIR pins of the AY chip at 4G
 * PB2 and PB3 connect to the BC1 and BDIR pins of the AY chip at 4H.
 */
static ADDRESS_MAP_START(zac1b11142_melody_map, AS_PROGRAM, 8, zac1b11142_audio_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM // 6802 internal RAM
	AM_RANGE(0x400c, 0x400f) AM_MIRROR(0x1ff0) AM_DEVREADWRITE("pia_4i", pia6821_device, read, write)
	AM_RANGE(0x8000, 0x9fff) AM_MIRROR(0x2000) AM_ROM // rom 13
	AM_RANGE(0xc000, 0xdfff) AM_MIRROR(0x2000) AM_ROM // rom 9
ADDRESS_MAP_END


/*
 * master sound cpu, controls speech directly
 * mapping:
 * A15 A14 A13 A12 A11 A10 A09 A08 A07 A06 A05 A04 A03 A02 A01 A00
 *  0   0   0   0   0   0   0   0   0   *   *   *   *   *   *   *  RW 6802 internal ram
 *  x   0   0   0   x   x   x   x   1   x   x   0   x   x   *   *  Open bus (test mode writes as if there was another PIA here)
 *  x   0   0   0   x   x   x   x   1   x   x   1   x   x   *   *  RW 6821 PIA @ 1I
 *  x   0   0   1   0   0   x   x   x   x   x   x   x   x   x   x   W MC1408 DAC
 *  x   x   0   1   0   1   x   x   x   x   x   x   x   x   x   x   W Command to slave melody cpu
 *  x   x   0   1   1   0   x   x   x   x   x   x   x   x   x   x  R  Command read latch from z80
 *  x   x   0   1   1   1   x   x   x   x   x   x   x   x   x   x  Open bus
 *  %   %   1   0   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS1A: Enable ROM 8
 *  %   %   1   1   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS0A: Enable ROM 7
 *  note that the % bits go to pins 2 (6802 A14) and 26 (6802 A15) of the roms
 *  monymony and jackrabt both use 2764 roms, which use pin 2 as A12 and pin 26 as N/C don't care
 *  hence for actual chips used, the mem map is:
 *  x   *   1   0   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS1A: Enable ROM 8
 *  x   *   1   1   *   *   *   *   *   *   *   *   *   *   *   *  R  /CS0A: Enable ROM 7
 *
 * 6821 PIA:
 * PA0-7, PB0-1, CA2 and CB1 connect to the TMS5200
 * CA1 and CB2 are not connected, though the test mode assumes there's something connected to CB2 (possibly another LED like the one connected to PB4)
 * PB3 connects to 'ACS' which goes to the Z80
 */
static ADDRESS_MAP_START(zac1b11142_audio_map, AS_PROGRAM, 8, zac1b11142_audio_device)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM // 6802 internal RAM
	AM_RANGE(0x0090, 0x0093) AM_MIRROR(0x8f6c) AM_DEVREADWRITE("pia_1i", pia6821_device, read, write)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x83ff) AM_DEVWRITE("dac_1f", dac_device, write_unsigned8) // MC1408
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0xc3ff) AM_WRITE(melody_command_w)
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0xc3ff) AM_READ(host_command_r)
	AM_RANGE(0x2000, 0x2fff) AM_MIRROR(0x8000) AM_ROM // ROM 8 with A12 low
	AM_RANGE(0x3000, 0x3fff) AM_MIRROR(0x8000) AM_ROM // ROM 7 with A12 low
	AM_RANGE(0x6000, 0x6fff) AM_MIRROR(0x8000) AM_ROM // ROM 8 with A12 high
	AM_RANGE(0x7000, 0x7fff) AM_MIRROR(0x8000) AM_ROM // ROM 7 with A12 high
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT(zac1b11142_config)
	MCFG_CPU_ADD("melodycpu", M6802, XTAL_3_579545MHz) // verified on pcb
	MCFG_CPU_PROGRAM_MAP(zac1b11142_melody_map)

	MCFG_DEVICE_ADD("timebase", CLOCK, XTAL_3_579545MHz/4096/2) // CPU clock divided using 4040 and half of 74LS74
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("pia_4i", pia6821_device, cb1_w))

	MCFG_DEVICE_ADD("pia_4i", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(zac1b11142_audio_device, pia_4i_porta_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(zac1b11142_audio_device, pia_4i_porta_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(zac1b11142_audio_device, pia_4i_portb_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("melodycpu", m6802_cpu_device, nmi_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("melodycpu", m6802_cpu_device, irq_line))

	MCFG_SOUND_ADD("ay_4g", AY8910, XTAL_3_579545MHz/2) // CPU clock divided using 4040
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(zac1b11142_audio_device, ay_4g_porta_w))
	MCFG_AY8910_PORT_B_READ_CB(READ8(zac1b11142_audio_device, ay_4g_portb_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.15)

	MCFG_SOUND_ADD("ay_4h", AY8910, XTAL_3_579545MHz/2) // CPU clock divided using 4040
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(zac1b11142_audio_device, ay_4h_porta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(zac1b11142_audio_device, ay_4h_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.15)

	MCFG_CPU_ADD("audiocpu", M6802, XTAL_3_579545MHz) // verified on pcb
	MCFG_CPU_PROGRAM_MAP(zac1b11142_audio_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(zac1b11142_audio_device, input_poll, 60)

	MCFG_DEVICE_ADD("pia_1i", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(DEVREAD8("speech", tms5220_device, status_r))
	MCFG_PIA_WRITEPA_HANDLER(DEVWRITE8("speech", tms5220_device, data_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(zac1b11142_audio_device, pia_1i_portb_w))

	MCFG_DAC_ADD("dac_1f")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.80)

	// There is no xtal, the clock is obtained from a RC oscillator as shown in the TMS5220 datasheet (R=100kOhm C=22pF)
	// 162kHz measured on pin 3 20 minutes after power on, clock would then be 162.3*4=649.2kHz
	MCFG_SOUND_ADD("speech", TMS5200, 649200) // ROMCLK pin measured at 162.3Khz, OSC is exactly *4 of that)
	MCFG_TMS52XX_IRQ_HANDLER(DEVWRITELINE("pia_1i", pia6821_device, cb1_w))
	MCFG_TMS52XX_READYQ_HANDLER(DEVWRITELINE("pia_1i", pia6821_device, ca2_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.80)
MACHINE_CONFIG_END


INPUT_PORTS_START(zac1b11142_ioports)
	PORT_START("1B11142")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("P1")
INPUT_PORTS_END


zac1b11142_audio_device::zac1b11142_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ZACCARIA_1B11142, "Zaccaria 1B11142 Sound Board", tag, owner, clock, "zac1b11142", __FILE__)
	, device_mixer_interface(mconfig, *this, 1)
	, m_acs_cb(*this)
	, m_melodycpu(*this, "melodycpu")
	, m_pia_4i(*this, "pia_4i")
	, m_ay_4g(*this, "ay_4g")
	, m_ay_4h(*this, "ay_4h")
	, m_audiocpu(*this, "audiocpu")
	, m_pia_1i(*this, "pia_1i")
	, m_speech(*this, "speech")
	, m_inputs(*this, "1B11142")
	, m_host_command(0)
	, m_melody_command(0)
{
}

WRITE8_MEMBER(zac1b11142_audio_device::hs_w)
{
	m_host_command = data;
	m_audiocpu->set_input_line(INPUT_LINE_IRQ0, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

READ_LINE_MEMBER(zac1b11142_audio_device::acs_r)
{
	return (~m_pia_1i->b_output() >> 3) & 0x01;
}

WRITE_LINE_MEMBER(zac1b11142_audio_device::ressound_w)
{
	// TODO: there is a pulse-stretching network attached that should be simulated
	m_melodycpu->set_input_line(INPUT_LINE_RESET, state);
	// TODO: holds the reset line of m_pia_4i - can't implement this in MAME at this time
	// TODO: holds the reset line of m_ay_4g - can't implement this in MAME at this time
	// TODO: holds the reset line of m_ay_4h - can't implement this in MAME at this time
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state);
	// TODO: holds the reset line of m_pia_1i - can't implement this in MAME at this time
	// TODO: does some funky stuff with the VDD and VSS lines on the speech chip
}

READ8_MEMBER(zac1b11142_audio_device::pia_4i_porta_r)
{
	UINT8 const control = m_pia_4i->b_output();
	UINT8 data = 0xff;

	if (0x01 == (control & 0x03))
		data &= m_ay_4g->data_r(space, 0);

	if (0x04 == (control & 0x0c))
		data &= m_ay_4h->data_r(space, 0);

	return data;
}

WRITE8_MEMBER(zac1b11142_audio_device::pia_4i_porta_w)
{
	UINT8 const control = m_pia_4i->b_output();

	if (control & 0x02)
		m_ay_4g->data_address_w(space, (control >> 0) & 0x01, data);

	if (control & 0x08)
		m_ay_4h->data_address_w(space, (control >> 2) & 0x01, data);
}

WRITE8_MEMBER(zac1b11142_audio_device::pia_4i_portb_w)
{
	if (data & 0x02)
		m_ay_4g->data_address_w(space, (data >> 0) & 0x01, m_pia_4i->a_output());

	if (data & 0x08)
		m_ay_4h->data_address_w(space, (data >> 2) & 0x01, m_pia_4i->a_output());
}

WRITE8_MEMBER(zac1b11142_audio_device::ay_4g_porta_w)
{
	// TODO: (data & 0x07) controls tromba mix volume
	// TODO: (data & 0x08) controls cassa gate
	// TODO: (data & 0x10) controls rullante gate
}

READ8_MEMBER(zac1b11142_audio_device::ay_4g_portb_r)
{
	return m_melody_command;
}

WRITE8_MEMBER(zac1b11142_audio_device::ay_4h_porta_w)
{
	// TODO: data & 0x01 controls LEVEL
	// TODO: data & 0x02 controls LEVELT
}

WRITE8_MEMBER(zac1b11142_audio_device::ay_4h_portb_w)
{
	// TODO: data & 0x01 controls ANAL3 filter
}

READ8_MEMBER(zac1b11142_audio_device::host_command_r)
{
	return m_host_command;
}

WRITE8_MEMBER(zac1b11142_audio_device::melody_command_w)
{
	m_melody_command = data;
	m_pia_4i->ca1_w((data >> 7) & 0x01);
}

WRITE8_MEMBER(zac1b11142_audio_device::pia_1i_portb_w)
{
	m_speech->rsq_w((data >> 0) & 0x01);
	m_speech->wsq_w((data >> 1) & 0x01);
	m_acs_cb((~data >> 3) & 0x01);
	// TODO: a LED output().set_led_value(0, (data >> 4) & 0x01);
}

INTERRUPT_GEN_MEMBER(zac1b11142_audio_device::input_poll)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_inputs->read() & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

machine_config_constructor zac1b11142_audio_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(zac1b11142_config);
}

ioport_constructor zac1b11142_audio_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(zac1b11142_ioports);
}

void zac1b11142_audio_device::device_config_complete()
{
}

void zac1b11142_audio_device::device_start()
{
	m_acs_cb.resolve_safe();

	save_item(NAME(m_host_command));
	save_item(NAME(m_melody_command));
}

void zac1b11142_audio_device::device_reset()
{
	m_host_command = 0;
	m_melody_command = 0;
}
