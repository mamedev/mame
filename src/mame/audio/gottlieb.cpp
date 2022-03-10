// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    gottlieb.cpp

    Gottlieb 6502-based sound hardware implementations.

    Dedicated to Warren Davis, Jeff Lee, Tim Skelly & David Thiel

***************************************************************************/

#include "emu.h"
#include "audio/gottlieb.h"

#include "machine/input_merger.h"


namespace {

constexpr XTAL SOUND1_CLOCK(3'579'545);
constexpr XTAL SOUND2_CLOCK(4'000'000);
constexpr XTAL SOUND2_SPEECH_CLOCK(3'120'000);

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_PIN2,        gottlieb_sound_p2_device,             "gotsndp2",   "Gottlieb Multi-mode Sound Board")
DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1,        gottlieb_sound_r1_device,             "gotsndr1",   "Gottlieb Sound rev. 1")
DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_REV1_VOTRAX, gottlieb_sound_r1_with_votrax_device, "gotsndr1vt", "Gottlieb Sound rev. 1 with Votrax")
DEFINE_DEVICE_TYPE(GOTTLIEB_SOUND_REV2,        gottlieb_sound_r2_device,             "gotsndr2",   "Gottlieb Sound rev. 2")


//**************************************************************************
//  REV 0 SOUND BOARD: 6502 + 6530 + DAC
//**************************************************************************

//-------------------------------------------------
//  gottlieb_sound_p2_device - constructors
//-------------------------------------------------

gottlieb_sound_p2_device::gottlieb_sound_p2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GOTTLIEB_SOUND_PIN2, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_audiocpu(*this, "audiocpu")
	, m_r6530(*this, "r6530")
	, m_sndcmd(0)
{
}


//-------------------------------------------------
//  read port -
//-------------------------------------------------

uint8_t gottlieb_sound_p2_device::r6530b_r()
{
	return m_sndcmd;
}


//-------------------------------------------------
//  write - handle an external command write
//-------------------------------------------------

void gottlieb_sound_p2_device::write(uint8_t data)
{
	// write the command data to bits 0-3 (also bit 6 used in system1 pinballs)
	uint8_t pb0_3 = ~data & 0x4f; // U7
	uint8_t pb4_7 = ioport("SB0")->read() & 0x90;
	m_sndcmd = pb0_3 | pb4_7;
	m_r6530->write(2, m_sndcmd);    // push to portB, but doesn't seem to be needed
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

void gottlieb_sound_p2_device::gottlieb_sound_p2_map(address_map &map)
{
	map.global_mask(0x0fff);
	map.unmap_value_high();
	map(0x0000, 0x017f).ram();
	map(0x0200, 0x03ff).rw(m_r6530, FUNC(mos6530_device::read), FUNC(mos6530_device::write));
	map(0x0400, 0x0fff).rom();
}


//-------------------------------------------------
//  input ports
//-------------------------------------------------

INPUT_PORTS_START( gottlieb_sound_p2 )
	PORT_START("SB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Sound Test") PORT_CODE(KEYCODE_7_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, gottlieb_sound_p2_device, audio_nmi, 0)
	PORT_DIPNAME( 0x80, 0x80, "Sound or Tones" )
	PORT_DIPSETTING(    0x80, "Sound" )
	PORT_DIPSETTING(    0x00, "Tones" )
	PORT_DIPNAME( 0x10, 0x00, "Attract Sound" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )  // Makes a sound every 6 minutes
INPUT_PORTS_END

// The sound test will only work if the 2 above dips are in opposing directions (one off and one on)
INPUT_CHANGED_MEMBER( gottlieb_sound_p2_device::audio_nmi )
{
	// Diagnostic button sends a pulse to NMI pin
	if (newval==CLEAR_LINE)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void gottlieb_sound_p2_device::device_add_mconfig(machine_config &config)
{
	// audio CPU
	M6502(config, m_audiocpu, 800'000); // M6503 - clock is a gate, a resistor and a capacitor. Freq 675-1000kHz.
	m_audiocpu->set_addrmap(AS_PROGRAM, &gottlieb_sound_p2_device::gottlieb_sound_p2_map);

	// I/O configuration
	MOS6530(config, m_r6530, 800'000); // same as cpu
	m_r6530->out_pa_callback().set("dac", FUNC(dac_byte_interface::data_w));
	m_r6530->in_pb_callback().set(FUNC(gottlieb_sound_p2_device::r6530b_r));

	// sound devices
	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.50); // SSS1408-6P
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor gottlieb_sound_p2_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gottlieb_sound_p2 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gottlieb_sound_p2_device::device_start()
{
	save_item(NAME(m_sndcmd));
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
	, m_dac(*this, "dac")
	, m_riot(*this, "riot")
{
}


//-------------------------------------------------
//  write - handle an external command write
//-------------------------------------------------

void gottlieb_sound_r1_device::write(u8 data)
{
	// write the command data to the low 6 bits, and the trigger to the upper bit
	uint8_t pa7 = (data & 0x0f) != 0xf;
	uint8_t pa0_5 = ~data & 0x3f;
	m_riot->porta_in_set(pa0_5 | (pa7 << 7), 0xbf);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

void gottlieb_sound_r1_device::gottlieb_sound_r1_map(address_map &map)
{
	// A15 not decoded except in expansion socket
	map.global_mask(0x7fff);
	map(0x0000, 0x007f).mirror(0x0d80).ram();
	map(0x0200, 0x021f).mirror(0x0de0).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
	map(0x1000, 0x1000).mirror(0x0fff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x6000, 0x7fff).rom();
}

void gottlieb_sound_r1_with_votrax_device::gottlieb_sound_r1_map(address_map &map)
{
	// A15 not decoded except in expansion socket
	gottlieb_sound_r1_device::gottlieb_sound_r1_map(map);
	map(0x2000, 0x2000).mirror(0x0fff).w(FUNC(gottlieb_sound_r1_with_votrax_device::votrax_data_w));
	map(0x3000, 0x3000).mirror(0x0fff).w(FUNC(gottlieb_sound_r1_with_votrax_device::speech_clock_dac_w));
}


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
	PORT_BIT( 0x80, 0x80, IPT_CUSTOM )
INPUT_PORTS_END

INPUT_PORTS_START( gottlieb_sound_r1_with_votrax )
	PORT_INCLUDE(gottlieb_sound_r1)
	PORT_MODIFY("SB1")
	PORT_BIT( 0x80, 0x80, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("votrax", votrax_sc01_device, request)
INPUT_PORTS_END


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void gottlieb_sound_r1_device::device_add_mconfig(machine_config &config)
{
	// audio CPU
	m6502_device &audiocpu(M6502(config, "audiocpu", SOUND1_CLOCK/4)); // the board can be set to /2 as well
	audiocpu.set_addrmap(AS_PROGRAM, &gottlieb_sound_r1_device::gottlieb_sound_r1_map);

	INPUT_MERGER_ANY_HIGH(config, "nmi").output_handler().set_inputline("audiocpu", INPUT_LINE_NMI);

	// I/O configuration
	RIOT6532(config, m_riot, SOUND1_CLOCK/4);
	m_riot->in_pb_callback().set_ioport("SB1");
	m_riot->out_pb_callback().set("nmi", FUNC(input_merger_device::in_w<0>)).bit(7).invert(); // unsure if this is ever used, but the NMI is connected to the RIOT's PB7
	m_riot->irq_callback().set_inputline("audiocpu", M6502_IRQ_LINE);

	// sound devices
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, *this, 0.25); // MC1408
}


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
	// register for save states
	save_item(NAME(m_dummy));
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
	, m_votrax(*this, "votrax")
	, m_last_speech_clock(0)
{
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void gottlieb_sound_r1_with_votrax_device::device_add_mconfig(machine_config &config)
{
	gottlieb_sound_r1_device::device_add_mconfig(config);

	m_dac->reset_routes();
	m_dac->add_route(ALL_OUTPUTS, *this, 0.20);

	// add the VOTRAX
	VOTRAX_SC01(config, m_votrax, 720000);
	m_votrax->ar_callback().set("nmi", FUNC(input_merger_device::in_w<1>));
	m_votrax->add_route(ALL_OUTPUTS, *this, 0.80);
}


//-------------------------------------------------
//  device_input_ports - return a pointer to
//  the device's I/O ports
//-------------------------------------------------

ioport_constructor gottlieb_sound_r1_with_votrax_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( gottlieb_sound_r1_with_votrax );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gottlieb_sound_r1_with_votrax_device::device_start()
{
	gottlieb_sound_r1_device::device_start();
	save_item(NAME(m_last_speech_clock));
}


void gottlieb_sound_r1_with_votrax_device::device_post_load()
{
	gottlieb_sound_r1_device::device_post_load();

	// totally random guesswork; would like to get real measurements on a board
	m_votrax->set_unscaled_clock(900000 + (m_last_speech_clock - 0xa0) * 9000);
}


//-------------------------------------------------
//  votrax_data_w - write data to the Votrax SC-01
//  speech chip
//-------------------------------------------------

void gottlieb_sound_r1_with_votrax_device::votrax_data_w(uint8_t data)
{
	m_votrax->inflection_w(data >> 6);
	m_votrax->write(~data & 0x3f);
}


//-------------------------------------------------
//  speech_clock_dac_w - modify the clock driving
//  the Votrax SC-01 speech chip
//-------------------------------------------------

void gottlieb_sound_r1_with_votrax_device::speech_clock_dac_w(uint8_t data)
{
	// prevent negative clock values (and possible crash)
	if (data < 0x60) data = 0x60;

	// nominal clock is 0xa0
	if (data != m_last_speech_clock)
	{
		logerror("clock = %02X\n", data);

		// totally random guesswork; would like to get real measurements on a board
		m_votrax->set_unscaled_clock(950000 + (data - 0xa0) * 5500);
		m_last_speech_clock = data;
	}
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
//  write - handle an external command write
//-------------------------------------------------

void gottlieb_sound_r2_device::write(u8 data)
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

uint8_t gottlieb_sound_r2_device::audio_data_r()
{
	if (!machine().side_effects_disabled())
		m_audiocpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	return m_audiocpu_latch;
}


//-------------------------------------------------
//  speech_data_r - read the input command latch
//  from the speech CPU
//-------------------------------------------------

uint8_t gottlieb_sound_r2_device::speech_data_r()
{
	if (!machine().side_effects_disabled())
		m_speechcpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	return m_speechcpu_latch;
}


//-------------------------------------------------
//  signal_audio_nmi_r - signal an NMI from the
//  speech CPU to the audio CPU
//-------------------------------------------------

uint8_t gottlieb_sound_r2_device::signal_audio_nmi_r()
{
	if (!machine().side_effects_disabled())
	{
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	return 0xff;
}


//-------------------------------------------------
//  signal_audio_nmi_w - signal an NMI from the
//  speech CPU to the audio CPU
//-------------------------------------------------

void gottlieb_sound_r2_device::signal_audio_nmi_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


//-------------------------------------------------
//  nmi_rate_w - adjust the NMI rate on the speech
//  CPU
//-------------------------------------------------

void gottlieb_sound_r2_device::nmi_rate_w(uint8_t data)
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

void gottlieb_sound_r2_device::speech_control_w(uint8_t data)
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
				m_ay1->data_address_w(data >> 4, m_psg_latch);
			else
				m_ay2->data_address_w(data >> 4, m_psg_latch);
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
			ay->address_w(m_psg_latch);
			ay->data_w(m_psg_data_latch);
		}
	}

	// bit 5 goes to the speech chip DIRECT DATA TEST pin

	// bit 6 = speech chip DATA PRESENT pin; high then low to make the chip read data
	if ((previous & 0x40) == 0 && (data & 0x40) != 0)
		m_sp0250->write(m_sp0250_latch);

	// bit 7 goes to the speech chip RESET pin
	if ((previous ^ data) & 0x80)
		m_sp0250->reset();
}


//-------------------------------------------------
//  psg_latch_w - store an 8-bit value in the PSG
//  latch register
//-------------------------------------------------

void gottlieb_sound_r2_device::psg_latch_w(uint8_t data)
{
	m_psg_latch = data;
}


//-------------------------------------------------
//  psg_latch_w - store an 8-bit value in the
//  SP0250 latch register
//-------------------------------------------------

void gottlieb_sound_r2_device::sp0250_latch_w(uint8_t data)
{
	m_sp0250_latch = data;
}


//-------------------------------------------------
//  sound CPU address map
//-------------------------------------------------

void gottlieb_sound_r2_device::gottlieb_sound_r2_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x3c00).ram();
	map(0x4000, 0x4000).mirror(0x3ffe).w("dacvol", FUNC(dac_byte_interface::data_w));
	map(0x4001, 0x4001).mirror(0x3ffe).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0x8000).mirror(0x3fff).r(FUNC(gottlieb_sound_r2_device::audio_data_r));
	map(0xc000, 0xdfff).mirror(0x2000).rom();
}


//-------------------------------------------------
//  speech CPU address map
//-------------------------------------------------

void gottlieb_sound_r2_device::gottlieb_speech_r2_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x1c00).ram();
	map(0x2000, 0x2000).mirror(0x1fff).w(FUNC(gottlieb_sound_r2_device::sp0250_latch_w));
	map(0x4000, 0x4000).mirror(0x1fff).w(FUNC(gottlieb_sound_r2_device::speech_control_w));
	map(0x6000, 0x6000).mirror(0x1fff).portr("SB2");
	map(0x8000, 0x8000).mirror(0x1fff).w(FUNC(gottlieb_sound_r2_device::psg_latch_w));
	map(0xa000, 0xa000).mirror(0x07ff).w(FUNC(gottlieb_sound_r2_device::nmi_rate_w));
	map(0xa800, 0xa800).mirror(0x07ff).r(FUNC(gottlieb_sound_r2_device::speech_data_r));
	map(0xb000, 0xb000).mirror(0x07ff).rw(FUNC(gottlieb_sound_r2_device::signal_audio_nmi_r), FUNC(gottlieb_sound_r2_device::signal_audio_nmi_w));
	map(0xc000, 0xffff).rom();
}


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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(gottlieb_sound_r2_device, speech_drq_custom_r)
INPUT_PORTS_END


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void gottlieb_sound_r2_device::device_add_mconfig(machine_config &config)
{
	// audio CPUs
	M6502(config, m_audiocpu, SOUND2_CLOCK/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &gottlieb_sound_r2_device::gottlieb_sound_r2_map);

	M6502(config, m_speechcpu, SOUND2_CLOCK/4);
	m_speechcpu->set_addrmap(AS_PROGRAM, &gottlieb_sound_r2_device::gottlieb_speech_r2_map);

	// sound hardware
	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, *this, 0.075); // AD7528
	DAC_8BIT_R2R(config, "dacvol", 0)
		.set_output_range(0, 1)
		.add_route(0, "dac", 1.0, DAC_INPUT_RANGE_HI)
		.add_route(0, "dac", -1.0, DAC_INPUT_RANGE_LO); // AD7528 dual DAC

	AY8913(config, m_ay1, SOUND2_CLOCK/2).add_route(ALL_OUTPUTS, *this, 0.15);

	AY8913(config, m_ay2, SOUND2_CLOCK/2).add_route(ALL_OUTPUTS, *this, 0.15);

	SP0250(config, m_sp0250, SOUND2_SPEECH_CLOCK).add_route(ALL_OUTPUTS, *this, 1.0);
}


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
	save_item(NAME(m_psg_latch));
	save_item(NAME(m_psg_data_latch));
	save_item(NAME(m_sp0250_latch));
}


//-------------------------------------------------
//  device_timer - handle timer-based behaviors
//-------------------------------------------------

void gottlieb_sound_r2_device::device_timer(emu_timer &timer, device_timer_id id, int param)
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

