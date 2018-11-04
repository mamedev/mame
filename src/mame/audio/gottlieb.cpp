// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    gottlieb.c

    Gottlieb 6502-based sound hardware implementations.

    Dedicated to Warren Davis, Jeff Lee, Tim Skelly & David Thiel

***************************************************************************/

#include "emu.h"
#include "audio/gottlieb.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"


#define SOUND1_CLOCK        XTAL(3'579'545)
#define SOUND2_CLOCK        XTAL(4'000'000)
#define SOUND2_SPEECH_CLOCK XTAL(3'120'000)


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_REV0,        gottlieb_sound_r0_device,             "gotsndr0",   "Gottlieb Sound rev. 0")
DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1,        gottlieb_sound_r1_device,             "gotsndr1",   "Gottlieb Sound rev. 1")
DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1_VOTRAX, gottlieb_sound_r1_with_votrax_device, "gotsndr1vt", "Gottlieb Sound rev. 1 with Votrax")
DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_REV2,        gottlieb_sound_r2_device,             "gotsndr2",   "Gottlieb Sound rev. 2")


//**************************************************************************
//  REV 0 SOUND BOARD: 6502 + 6530 + DAC
//**************************************************************************

//-------------------------------------------------
//  gottlieb_sound_r0_device - constructors
//-------------------------------------------------

gottlieb_sound_r0_device::gottlieb_sound_r0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GOTTLIEB_SOUND_REV0, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_audiocpu(*this, "audiocpu")
	, m_r6530(*this, "r6530")
	, m_sndcmd(0)
{
}


//-------------------------------------------------
//  read port -
//-------------------------------------------------

READ8_MEMBER( gottlieb_sound_r0_device::r6530b_r )
{
	return m_sndcmd;
}


//-------------------------------------------------
//  write - handle an external command write
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r0_device::write )
{
	// write the command data to the low 4 bits
	uint8_t pb0_3 = data ^ 15;
	uint8_t pb4_7 = ioport("SB0")->read() & 0x90;
	m_sndcmd = pb0_3 | pb4_7;
	m_r6530->write(space, offset, m_sndcmd);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

ADDRESS_MAP_START(gottlieb_sound_r0_device::gottlieb_sound_r0_map)
	ADDRESS_MAP_GLOBAL_MASK(0x0fff)
	AM_RANGE(0x0000, 0x003f) AM_RAM AM_MIRROR(0x1c0)
	AM_RANGE(0x0200, 0x020f) AM_DEVREADWRITE("r6530", mos6530_device, read, write)
	AM_RANGE(0x0400, 0x0fff) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  input ports
//-------------------------------------------------

INPUT_PORTS_START( gottlieb_sound_r0 )
	PORT_START("SB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_0) PORT_CHANGED_MEMBER(DEVICE_SELF, gottlieb_sound_r0_device, audio_nmi, nullptr)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Attract") PORT_CODE(KEYCODE_F1) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Music") PORT_CODE(KEYCODE_F2) PORT_TOGGLE
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( gottlieb_sound_r0_device::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(gottlieb_sound_r0_device::device_add_mconfig)
	// audio CPU
	MCFG_CPU_ADD("audiocpu", M6502, SOUND1_CLOCK/4) // M6503 - clock is a gate, a resistor and a capacitor. Freq unknown.
	MCFG_CPU_PROGRAM_MAP(gottlieb_sound_r0_map)

	// I/O configuration
	MCFG_DEVICE_ADD("r6530", MOS6530, SOUND1_CLOCK/4) // unknown - same as cpu
	MCFG_MOS6530_OUT_PA_CB(DEVWRITE8("dac", dac_byte_interface, write))
	MCFG_MOS6530_IN_PB_CB(READ8(gottlieb_sound_r0_device, r6530b_r))

	// sound devices
	MCFG_SOUND_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.25) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor gottlieb_sound_r0_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gottlieb_sound_r0 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gottlieb_sound_r0_device::device_start()
{
}


//**************************************************************************
//  REV 1 SOUND BOARD: 6502 + DAC
//**************************************************************************

//-------------------------------------------------
//  gottlieb_sound_r1_device - constructors
//-------------------------------------------------

gottlieb_sound_r1_device::gottlieb_sound_r1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gottlieb_sound_r1_device(mconfig, GOTTLIEB_SOUND_REV1, tag, owner, clock)
{
}

gottlieb_sound_r1_device::gottlieb_sound_r1_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_audiocpu(*this, "audiocpu")
	, m_riot(*this, "riot")
	, m_votrax(*this, "votrax")
	, m_last_speech_clock(0)
{
}


//-------------------------------------------------
//  write - handle an external command write
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r1_device::write )
{
	// write the command data to the low 6 bits, and the trigger to the upper bit
	uint8_t pa7 = (data & 0x0f) != 0xf;
	uint8_t pa0_5 = ~data & 0x3f;
	m_riot->porta_in_set(pa0_5 | (pa7 << 7), 0xbf);
}


//-------------------------------------------------
//  snd_interrupt - signal a sound interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( gottlieb_sound_r1_device::snd_interrupt )
{
	m_audiocpu->set_input_line(M6502_IRQ_LINE, state);
}


//-------------------------------------------------
//  r6532_portb_w - handle writes to the RIOT's
//  port B
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r1_device::r6532_portb_w )
{
	// unsure if this is ever used, but the NMI is connected to the RIOT's PB7
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}


//-------------------------------------------------
//  votrax_data_w - write data to the Votrax SC-01
//  speech chip
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r1_device::votrax_data_w )
{
	if (m_votrax != nullptr)
	{
		m_votrax->inflection_w(space, offset, data >> 6);
		m_votrax->write(space, offset, ~data & 0x3f);
	}
}


//-------------------------------------------------
//  speech_clock_dac_w - modify the clock driving
//  the Votrax SC-01 speech chip
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r1_device::speech_clock_dac_w )
{
	// prevent negative clock values (and possible crash)
	if (data < 0x65) data = 0x65;

	if (m_votrax != nullptr)
	{
		// nominal clock is 0xa0
		if (data != m_last_speech_clock)
		{
			osd_printf_debug("clock = %02X\n", data);

			// totally random guesswork; would like to get real measurements on a board
			if (m_votrax != nullptr)
				m_votrax->set_unscaled_clock(600000 + (data - 0xa0) * 10000);
			m_last_speech_clock = data;
		}
	}
}


//-------------------------------------------------
//  votrax_request - map the VOTRAX SC-01 request
//  line to the NMI pin on the sound chip
//-------------------------------------------------

WRITE_LINE_MEMBER( gottlieb_sound_r1_device::votrax_request )
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, state);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

ADDRESS_MAP_START(gottlieb_sound_r1_device::gottlieb_sound_r1_map)
	// A15 not decoded except in expansion socket
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0d80) AM_RAM
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0de0) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0fff) AM_DEVWRITE("dac", dac_byte_interface, write)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0fff) AM_WRITE(votrax_data_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0fff) AM_WRITE(speech_clock_dac_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  input ports
//-------------------------------------------------

INPUT_PORTS_START( gottlieb_sound_r1 )
	PORT_START("SB1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SB1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SB1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SB1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SB1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SB1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SB1:3" )
	PORT_DIPNAME( 0x40, 0x40, "Sound Test" )            PORT_DIPLOCATION("SB1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, 0x80, IPT_SPECIAL )
INPUT_PORTS_END

INPUT_PORTS_START( gottlieb_sound_r1_with_votrax )
	PORT_INCLUDE(gottlieb_sound_r1)
	PORT_MODIFY("SB1")
	PORT_BIT( 0x80, 0x80, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("votrax", votrax_sc01_device, request)
INPUT_PORTS_END


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(gottlieb_sound_r1_device::device_add_mconfig)
	// audio CPU
	MCFG_CPU_ADD("audiocpu", M6502, SOUND1_CLOCK/4) // the board can be set to /2 as well
	MCFG_CPU_PROGRAM_MAP(gottlieb_sound_r1_map)

	// I/O configuration
	MCFG_DEVICE_ADD("riot", RIOT6532, SOUND1_CLOCK/4)
	MCFG_RIOT6532_IN_PB_CB(IOPORT("SB1"))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gottlieb_sound_r1_device, r6532_portb_w))
	MCFG_RIOT6532_IRQ_CB(WRITELINE(gottlieb_sound_r1_device, snd_interrupt))

	// sound devices
	MCFG_SOUND_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.25) // unknown DAC
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor gottlieb_sound_r1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gottlieb_sound_r1 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gottlieb_sound_r1_device::device_start()
{
}



//**************************************************************************
//  REV 1 SOUND BOARD WITH VOTRAX
//**************************************************************************

//-------------------------------------------------
//  gottlieb_sound_r1_with_votrax_device -
//  constructor
//-------------------------------------------------

gottlieb_sound_r1_with_votrax_device::gottlieb_sound_r1_with_votrax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gottlieb_sound_r1_device(mconfig, GOTTLIEB_SOUND_REV1_VOTRAX, tag, owner, clock)
{
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(gottlieb_sound_r1_with_votrax_device::device_add_mconfig)
	gottlieb_sound_r1_device::device_add_mconfig(config);

	// add the VOTRAX
	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000)
	MCFG_VOTRAX_SC01_REQUEST_CB(DEVWRITELINE(DEVICE_SELF, gottlieb_sound_r1_device, votrax_request))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.5)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor gottlieb_sound_r1_with_votrax_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gottlieb_sound_r1_with_votrax );
}



//**************************************************************************
//  REV 2 SOUND BOARD: 6502 + 2 x DAC + 2 x AY-8913
//**************************************************************************

//-------------------------------------------------
//  gottlieb_sound_r2_device - constructor
//-------------------------------------------------

gottlieb_sound_r2_device::gottlieb_sound_r2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GOTTLIEB_SOUND_REV2, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_audiocpu(*this, "audiocpu"),
		m_speechcpu(*this, "speechcpu"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_sp0250(*this, "spsnd"),
		m_cobram3_mod(false),
		m_nmi_timer(nullptr),
		m_nmi_state(0),
		m_audiocpu_latch(0),
		m_speechcpu_latch(0),
		m_speech_control(0),
		m_last_command(0),
		m_psg_latch(0),
		m_psg_data_latch(0),
		m_sp0250_latch(0)
{
}


//-------------------------------------------------
//  static_enable_cobram3_mods - enable changes
//  for cobram3
//-------------------------------------------------

void gottlieb_sound_r2_device::static_enable_cobram3_mods(device_t &device)
{
	downcast<gottlieb_sound_r2_device &>(device).m_cobram3_mod = true;
}


//-------------------------------------------------
//  write - handle an external command write
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::write )
{
	// when data is not 0xff, the transparent latch at A3 allows it to pass through unmolested
	if (data != 0xff)
	{
		// latch data on a timer
		synchronize(TID_SOUND_LATCH_WRITE, data);

		// if the previous data was 0xff, clock an IRQ on each
		if (m_last_command == 0xff)
		{
			m_audiocpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
			m_speechcpu->set_input_line(M6502_IRQ_LINE, ASSERT_LINE);
		}
	}
	m_last_command = data;
}


//-------------------------------------------------
//  nmi_timer_adjust - adjust the NMI timer to
//  fire based on its configured rate
//-------------------------------------------------

inline void gottlieb_sound_r2_device::nmi_timer_adjust()
{
	// adjust timer to go off in the future based on the current rate
	m_nmi_timer->adjust(attotime::from_hz(SOUND2_CLOCK/16) * (256 * (256 - m_nmi_rate)));
}


//-------------------------------------------------
//  nmi_state_update - update the NMI state based
//  on the timer firing and the enable control
//-------------------------------------------------

inline void gottlieb_sound_r2_device::nmi_state_update()
{
	// update the NMI line state based on the enable and state
	m_speechcpu->set_input_line(INPUT_LINE_NMI, (m_nmi_state && (m_speech_control & 1)) ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  speech_data_r - read the input command latch
//  from the audio CPU
//-------------------------------------------------

READ8_MEMBER( gottlieb_sound_r2_device::audio_data_r )
{
	m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	return m_audiocpu_latch;
}


//-------------------------------------------------
//  speech_data_r - read the input command latch
//  from the speech CPU
//-------------------------------------------------

READ8_MEMBER( gottlieb_sound_r2_device::speech_data_r )
{
	m_speechcpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	return m_speechcpu_latch;
}


//-------------------------------------------------
//  signal_audio_nmi_w - signal an NMI from the
//  speech CPU to the audio CPU
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::signal_audio_nmi_w )
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


//-------------------------------------------------
//  nmi_rate_w - adjust the NMI rate on the speech
//  CPU
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::nmi_rate_w )
{
	// the new rate is picked up when the previous timer expires
	m_nmi_rate = data;
}


//-------------------------------------------------
//  speech_drq_custom_r - return the SP0250
//  request line as an input port bit
//-------------------------------------------------

CUSTOM_INPUT_MEMBER( gottlieb_sound_r2_device::speech_drq_custom_r )
{
	return m_sp0250->drq_r();
}


//-------------------------------------------------
//  speech_control_w - primary audio control
//  register on the speech board
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::speech_control_w )
{
	uint8_t previous = m_speech_control;
	m_speech_control = data;

	// bit 0 enables/disables the NMI line
	nmi_state_update();

	// bit 1 controls a LED on the sound board

	// bits 2-4 control the AY-8913, but act differently between the
	// standard sound board and the modified Cobra Command board
	if (!m_cobram3_mod)
	{
		// bit 2 goes to 8913 BDIR pin
		if ((previous & 0x04) != 0 && (data & 0x04) == 0)
		{
			// bit 3 selects which of the two 8913 to enable
			// bit 4 goes to the 8913 BC1 pin
			if ((data & 0x08) != 0)
				m_ay1->data_address_w(space, data >> 4, m_psg_latch);
			else
				m_ay2->data_address_w(space, data >> 4, m_psg_latch);
		}
	}
	else
	{
		if ( data & 0x10 )
		{
			m_psg_data_latch = m_psg_latch;
		}
		else
		{
			ay8913_device *ay = (data & 0x08) ? m_ay1 : m_ay2;
			ay->address_w(space, 0, m_psg_latch);
			ay->data_w(space, 0, m_psg_data_latch);
		}
	}

	// bit 5 goes to the speech chip DIRECT DATA TEST pin

	// bit 6 = speech chip DATA PRESENT pin; high then low to make the chip read data
	if ((previous & 0x40) == 0 && (data & 0x40) != 0)
		m_sp0250->write(space, 0, m_sp0250_latch);

	// bit 7 goes to the speech chip RESET pin
	if ((previous ^ data) & 0x80)
		m_sp0250->reset();
}


//-------------------------------------------------
//  psg_latch_w - store an 8-bit value in the PSG
//  latch register
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::psg_latch_w )
{
	m_psg_latch = data;
}


//-------------------------------------------------
//  psg_latch_w - store an 8-bit value in the
//  SP0250 latch register
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::sp0250_latch_w )
{
	m_sp0250_latch = data;
}


//-------------------------------------------------
//  sound CPU address map
//-------------------------------------------------

ADDRESS_MAP_START(gottlieb_sound_r2_device::gottlieb_sound_r2_map)
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x3c00) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x3ffe) AM_DEVWRITE("dacvol", dac_byte_interface, write)
	AM_RANGE(0x4001, 0x4001) AM_MIRROR(0x3ffe) AM_DEVWRITE("dac", dac_byte_interface, write)
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x3fff) AM_READ(audio_data_r)
	AM_RANGE(0xc000, 0xdfff) AM_MIRROR(0x2000) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  sppech CPU address map
//-------------------------------------------------

ADDRESS_MAP_START(gottlieb_sound_r2_device::gottlieb_speech_r2_map)
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x1fff) AM_WRITE(sp0250_latch_w)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x1fff) AM_WRITE(speech_control_w)
	AM_RANGE(0x6000, 0x6000) AM_MIRROR(0x1fff) AM_READ_PORT("SB2")
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1fff) AM_WRITE(psg_latch_w)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x07ff) AM_WRITE(nmi_rate_w)
	AM_RANGE(0xa800, 0xa800) AM_MIRROR(0x07ff) AM_READ(speech_data_r)
	AM_RANGE(0xb000, 0xb000) AM_MIRROR(0x07ff) AM_WRITE(signal_audio_nmi_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  input ports
//-------------------------------------------------

INPUT_PORTS_START( gottlieb_sound_r2 )
	PORT_START("SB2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SB2:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SB2:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SB2:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SB2:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SB2:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SB2:6")
	PORT_DIPNAME( 0x40, 0x40, "Sound Test" )            PORT_DIPLOCATION("SB2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, gottlieb_sound_r2_device, speech_drq_custom_r, nullptr)
INPUT_PORTS_END


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(gottlieb_sound_r2_device::device_add_mconfig)
	// audio CPUs
	MCFG_CPU_ADD("audiocpu", M6502, SOUND2_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(gottlieb_sound_r2_map)

	MCFG_CPU_ADD("speechcpu", M6502, SOUND2_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(gottlieb_speech_r2_map)

	// sound hardware
	MCFG_SOUND_ADD("dac", DAC_8BIT_R2R, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.075) // unknown DAC
	MCFG_SOUND_ADD("dacvol", DAC_8BIT_R2R, 0) // unknown DAC
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dacvol", 1.0, DAC_VREF_POS_INPUT)

	MCFG_SOUND_ADD("ay1", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.15)

	MCFG_SOUND_ADD("ay2", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.15)

	MCFG_SOUND_ADD("spsnd", SP0250, SOUND2_SPEECH_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0)
MACHINE_CONFIG_END


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor gottlieb_sound_r2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gottlieb_sound_r2 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gottlieb_sound_r2_device::device_start()
{
	// set up the NMI timer
	m_nmi_timer = timer_alloc(TID_NMI_GENERATE);
	m_nmi_rate = 0;
	nmi_timer_adjust();

	// disable the non-speech CPU for cobram3
	if (m_cobram3_mod)
		m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	// register for save states
	save_item(NAME(m_nmi_rate));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_speech_control));
	save_item(NAME(m_last_command));
}


//-------------------------------------------------
//  device_timer - handle timer-based behaviors
//-------------------------------------------------

void gottlieb_sound_r2_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TID_NMI_GENERATE:
			// update state
			m_nmi_state = 1;
			nmi_state_update();

			// set a timer to turn it off again on the next SOUND_CLOCK/16
			timer_set(attotime::from_hz(SOUND2_CLOCK/16), TID_NMI_CLEAR);

			// adjust the NMI timer for the next time
			nmi_timer_adjust();
			break;

		case TID_NMI_CLEAR:
			// update state
			m_nmi_state = 0;
			nmi_state_update();
			break;

		case TID_SOUND_LATCH_WRITE:
			// each CPU has its own latch
			m_audiocpu_latch = param;
			m_speechcpu_latch = param;
			break;
	}
}

