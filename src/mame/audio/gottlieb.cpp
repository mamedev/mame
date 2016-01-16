// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    gottlieb.c

    Gottlieb 6502-based sound hardware implementations.

    Dedicated to Warren Davis, Jeff Lee, Tim Skelly & David Thiel

***************************************************************************/

#include "audio/gottlieb.h"


#define SOUND1_CLOCK        XTAL_3_579545MHz
#define SOUND2_CLOCK        XTAL_4MHz
#define SOUND2_SPEECH_CLOCK XTAL_3_12MHz


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const device_type GOTTLIEB_SOUND_REV0 = &device_creator<gottlieb_sound_r0_device>;
extern const device_type GOTTLIEB_SOUND_REV1 = &device_creator<gottlieb_sound_r1_device>;
extern const device_type GOTTLIEB_SOUND_REV1_WITH_VOTRAX = &device_creator<gottlieb_sound_r1_with_votrax_device>;
extern const device_type GOTTLIEB_SOUND_REV2 = &device_creator<gottlieb_sound_r2_device>;



//**************************************************************************
//  OLD CRAPPY SAMPLE PLAYER
//**************************************************************************

#if USE_FAKE_VOTRAX

void gottlieb_sound_r1_device::trigger_sample(UINT8 data)
{
	/* Reactor samples */
	if (strcmp(machine().system().name, "reactor") == 0)
	{
		switch (data)
		{
			case 55:
			case 56:
			case 57:
			case 59:
				m_samples->start(0, data - 53);
				break;

			case 31:
				m_score_sample = 7;
				break;

			case 39:
				m_score_sample++;
				if (m_score_sample < 20)
					m_samples->start(0, m_score_sample);
				break;
		}
	}

	/* Q*Bert samples */
	else
	{
		switch (data)
		{
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
				m_samples->start(0, (data - 17) * 8 + m_random_offset);
				m_random_offset = (m_random_offset + 1) & 7;
				break;

			case 22:
				m_samples->start(0,40);
				break;

			case 23:
				m_samples->start(0,41);
				break;
		}
	}
}

void gottlieb_sound_r1_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

void gottlieb_sound_r1_device::fake_votrax_data_w(UINT8 data)
{
	static const char *const PhonemeTable[0x40] =
	{
		"EH3", "EH2", "EH1", "PA0", "DT" , "A1" , "A2" , "ZH",
		"AH2", "I3" , "I2" , "I1" , "M"  , "N"  , "B"  , "V",
		"CH" , "SH" , "Z"  , "AW1", "NG" , "AH1", "OO1", "OO",
		"L"  , "K"  , "J"  , "H"  , "G"  , "F"  , "D"  , "S",
		"A"  , "AY" , "Y1" , "UH3", "AH" , "P"  , "O"  , "I",
		"U"  , "Y"  , "T"  , "R"  , "E"  , "W"  , "AE" , "AE1",
		"AW2", "UH2", "UH1", "UH" , "O2" , "O1" , "IU" , "U1",
		"THV", "TH" , "ER" , "EH" , "E1" , "AW" , "PA1", "STOP"
	};

	data ^= 0xff;

logerror("Votrax: intonation %d, phoneme %02x %s\n",data >> 6,data & 0x3f,PhonemeTable[data & 0x3f]);

	m_votrax_queue[m_votrax_queuepos++] = data;

	if ((data & 0x3f) == 0x3f)
	{
		if (m_votrax_queuepos > 1)
		{
			int last = -1;
			int i;
			char phonemes[200];

			phonemes[0] = 0;
			for (i = 0;i < m_votrax_queuepos-1;i++)
			{
				static const char *const inf[4] = { "[0]", "[1]", "[2]", "[3]" };
				int phoneme = m_votrax_queue[i] & 0x3f;
				int inflection = m_votrax_queue[i] >> 6;
				if (inflection != last) strcat(phonemes, inf[inflection]);
				last = inflection;
				if (phoneme == 0x03 || phoneme == 0x3e) strcat(phonemes," ");
				else strcat(phonemes,PhonemeTable[phoneme]);
			}

			osd_printf_debug("Votrax played '%s'\n", phonemes);

			if (strcmp(phonemes, "[0] HEH3LOOW     AH1EH3I3YMTERI2NDAHN") == 0)   /* Q-Bert & Tylz - Hello, I am turned on */
								m_samples->start(0, 42);
			else if (strcmp(phonemes, "[0]BAH1EH1Y") == 0)                            /* Q-Bert - Bye, bye */
				m_samples->start(0, 43);
			else if (strcmp(phonemes, "[0]A2YHT LEH2FTTH") == 0)                      /* Reactor - Eight left */
				m_samples->start(0, 0);
			else if (strcmp(phonemes, "[0]SI3KS DTYN LEH2FTTH") == 0)                 /* Reactor - Sixteen left */
				m_samples->start(0, 1);
			else if (strcmp(phonemes, "[0]WO2RNYNG KO2R UH1NSDTABUH1L") == 0)         /* Reactor - Warning core unstable */
				m_samples->start(0, 5);
			else if (strcmp(phonemes, "[0]CHAMBERR   AE1EH2KTI1VA1I3DTEH1DT ") == 0) /* Reactor - Chamber activated */
				m_samples->start(0, 7);
		}

		m_votrax_queuepos = 0;
	}

	/* generate a NMI after a while to make the CPU continue to send data */
	timer_set(attotime::from_usec(50));
}

static const char *const reactor_sample_names[] =
{
	"*reactor",
	"fx_53", /* "8 left" */
	"fx_54", /* "16 left" */
	"fx_55", /* "24 left" */
	"fx_56", /* "32 left" */
	"fx_57", /* "40 left" */
	"fx_58", /* "warning, core unstable" */
	"fx_59", /* "bonus" */
	"fx_31", /* "chamber activated" */
	"fx_39a", /* "2000" */
	"fx_39b", /* "5000" */
	"fx_39c", /* "10000" */
	"fx_39d", /* "15000" */
	"fx_39e", /* "20000" */
	"fx_39f", /* "25000" */
	"fx_39g", /* "30000" */
	"fx_39h", /* "35000" */
	"fx_39i", /* "40000" */
	"fx_39j", /* "45000" */
	"fx_39k", /* "50000" */
	"fx_39l", /* "55000" */
	nullptr   /* end of array */
};

static const char *const qbert_sample_names[] =
{
	"*qbert",
	"fx_17a", /* random speech, voice clock 255 */
	"fx_17b", /* random speech, voice clock 255 */
	"fx_17c", /* random speech, voice clock 255 */
	"fx_17d", /* random speech, voice clock 255 */
	"fx_17e", /* random speech, voice clock 255 */
	"fx_17f", /* random speech, voice clock 255 */
	"fx_17g", /* random speech, voice clock 255 */
	"fx_17h", /* random speech, voice clock 255 */
	"fx_18a", /* random speech, voice clock 176 */
	"fx_18b", /* random speech, voice clock 176 */
	"fx_18c", /* random speech, voice clock 176 */
	"fx_18d", /* random speech, voice clock 176 */
	"fx_18e", /* random speech, voice clock 176 */
	"fx_18f", /* random speech, voice clock 176 */
	"fx_18g", /* random speech, voice clock 176 */
	"fx_18h", /* random speech, voice clock 176 */
	"fx_19a", /* random speech, voice clock 128 */
	"fx_19b", /* random speech, voice clock 128 */
	"fx_19c", /* random speech, voice clock 128 */
	"fx_19d", /* random speech, voice clock 128 */
	"fx_19e", /* random speech, voice clock 128 */
	"fx_19f", /* random speech, voice clock 128 */
	"fx_19g", /* random speech, voice clock 128 */
	"fx_19h", /* random speech, voice clock 128 */
	"fx_20a", /* random speech, voice clock 96 */
	"fx_20b", /* random speech, voice clock 96 */
	"fx_20c", /* random speech, voice clock 96 */
	"fx_20d", /* random speech, voice clock 96 */
	"fx_20e", /* random speech, voice clock 96 */
	"fx_20f", /* random speech, voice clock 96 */
	"fx_20g", /* random speech, voice clock 96 */
	"fx_20h", /* random speech, voice clock 96 */
	"fx_21a", /* random speech, voice clock 62 */
	"fx_21b", /* random speech, voice clock 62 */
	"fx_21c", /* random speech, voice clock 62 */
	"fx_21d", /* random speech, voice clock 62 */
	"fx_21e", /* random speech, voice clock 62 */
	"fx_21f", /* random speech, voice clock 62 */
	"fx_21g", /* random speech, voice clock 62 */
	"fx_21h", /* random speech, voice clock 62 */
	"fx_22", /* EH2 with decreasing voice clock */
	"fx_23", /* O1 with varying voice clock */
	"fx_28",
	"fx_36",
	nullptr   /* end of array */
};

MACHINE_CONFIG_FRAGMENT( reactor_samples )
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(reactor_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( qbert_samples )
	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(1)
	MCFG_SAMPLES_NAMES(qbert_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

#endif


//**************************************************************************
//  REV 0 SOUND BOARD: 6502 + 6530 + DAC
//**************************************************************************

//-------------------------------------------------
//  gottlieb_sound_r0_device - constructors
//-------------------------------------------------

gottlieb_sound_r0_device::gottlieb_sound_r0_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GOTTLIEB_SOUND_REV1, "Gottlieb Sound rev. 0", tag, owner, clock, "gotsndr0", __FILE__)
	, device_mixer_interface(mconfig, *this)
	, m_audiocpu(*this, "audiocpu")
	, m_r6530(*this, "r6530")
	, m_dac(*this, "dac")
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
	UINT8 pb0_3 = data ^ 15;
	UINT8 pb4_7 = ioport("SB0")->read() & 0x90;
	m_sndcmd = pb0_3 | pb4_7;
	m_r6530->write(space, offset, m_sndcmd);
}


//-------------------------------------------------
//  audio CPU map
//-------------------------------------------------

static ADDRESS_MAP_START( gottlieb_sound_r0_map, AS_PROGRAM, 8, gottlieb_sound_r0_device )
	ADDRESS_MAP_GLOBAL_MASK(0x0fff)
	AM_RANGE(0x0000, 0x003f) AM_RAM AM_MIRROR(0x1c0)
	AM_RANGE(0x0200, 0x020f) AM_DEVREADWRITE("r6530", mos6530_device, read, write)
	AM_RANGE(0x0400, 0x0fff) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( gottlieb_sound_r0 )
	// audio CPU
	MCFG_CPU_ADD("audiocpu", M6502, SOUND1_CLOCK/4) // M6503 - clock is a gate, a resistor and a capacitor. Freq unknown.
	MCFG_CPU_PROGRAM_MAP(gottlieb_sound_r0_map)

	// I/O configuration
	MCFG_DEVICE_ADD("r6530", MOS6530, SOUND1_CLOCK/4) // unknown - same as cpu
	MCFG_MOS6530_OUT_PA_CB(DEVWRITE8("dac", dac_device, write_unsigned8))
	MCFG_MOS6530_IN_PB_CB(READ8(gottlieb_sound_r0_device, r6530b_r))

	// sound devices
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.50)
MACHINE_CONFIG_END


//-------------------------------------------------
//  input ports
//-------------------------------------------------

INPUT_PORTS_START( gottlieb_sound_r0 )
	PORT_START("SB0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_0) PORT_CHANGED_MEMBER(DEVICE_SELF, gottlieb_sound_r0_device, audio_nmi, 1)
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
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor gottlieb_sound_r0_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gottlieb_sound_r0 );
}


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

gottlieb_sound_r1_device::gottlieb_sound_r1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GOTTLIEB_SOUND_REV1, "Gottlieb Sound rev. 1", tag, owner, clock, "gotsndr1", __FILE__),
		device_mixer_interface(mconfig, *this),
		m_audiocpu(*this, "audiocpu"),
		m_riot(*this, "riot"),
		m_dac(*this, "dac"),
		m_votrax(*this, "votrax"),
		//m_populate_votrax(false),
		m_last_speech_clock(0)
#if USE_FAKE_VOTRAX
		, m_samples(*this, ":samples"),
		m_score_sample(0),
		m_random_offset(0),
		m_votrax_queuepos(0)
#endif
{
}

gottlieb_sound_r1_device::gottlieb_sound_r1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock, bool populate_votrax)
	: device_t(mconfig, GOTTLIEB_SOUND_REV1, "Gottlieb Sound rev. 1", tag, owner, clock, "gotsndr1", __FILE__),
		device_mixer_interface(mconfig, *this),
		m_audiocpu(*this, "audiocpu"),
		m_riot(*this, "riot"),
		m_dac(*this, "dac"),
		m_votrax(*this, "votrax"),
		//m_populate_votrax(populate_votrax),
		m_last_speech_clock(0)
#if USE_FAKE_VOTRAX
		, m_samples(*this, ":samples"),
		m_score_sample(0),
		m_random_offset(0),
		m_votrax_queuepos(0)
#endif
{
}


//-------------------------------------------------
//  write - handle an external command write
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r1_device::write )
{
	// write the command data to the low 6 bits, and the trigger to the upper bit
	UINT8 pa7 = (data & 0x0f) != 0xf;
	UINT8 pa0_5 = ~data & 0x3f;
	m_riot->porta_in_set(pa0_5 | (pa7 << 7), 0xbf);

#if USE_FAKE_VOTRAX
	if (pa7 && m_samples != nullptr)
		trigger_sample(pa0_5);
#endif
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

#if USE_FAKE_VOTRAX
	fake_votrax_data_w(data);
#endif
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

static ADDRESS_MAP_START( gottlieb_sound_r1_map, AS_PROGRAM, 8, gottlieb_sound_r1_device )
	// A15 not decoded except in expansion socket
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0d80) AM_RAM
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0de0) AM_DEVREADWRITE("riot", riot6532_device, read, write)
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x0fff) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x0fff) AM_WRITE(votrax_data_w)
	AM_RANGE(0x3000, 0x3000) AM_MIRROR(0x0fff) AM_WRITE(speech_clock_dac_w)
	AM_RANGE(0x6000, 0x7fff) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( gottlieb_sound_r1 )
	// audio CPU
	MCFG_CPU_ADD("audiocpu", M6502, SOUND1_CLOCK/4) // the board can be set to /2 as well
	MCFG_CPU_PROGRAM_MAP(gottlieb_sound_r1_map)

	// I/O configuration
	MCFG_DEVICE_ADD("riot", RIOT6532, SOUND1_CLOCK/4)
	MCFG_RIOT6532_IN_PB_CB(IOPORT("SB1"))
	MCFG_RIOT6532_OUT_PB_CB(WRITE8(gottlieb_sound_r1_device, r6532_portb_w))
	MCFG_RIOT6532_IRQ_CB(WRITELINE(gottlieb_sound_r1_device, snd_interrupt))

	// sound devices
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.50)
MACHINE_CONFIG_END

MACHINE_CONFIG_FRAGMENT( gottlieb_sound_r1_with_votrax )
	MCFG_FRAGMENT_ADD(gottlieb_sound_r1)

	// add the VOTRAX
	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000)
	MCFG_VOTRAX_SC01_REQUEST_CB(DEVWRITELINE(DEVICE_SELF, gottlieb_sound_r1_device, votrax_request))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.50)
MACHINE_CONFIG_END


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
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor gottlieb_sound_r1_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gottlieb_sound_r1 );
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
}



//**************************************************************************
//  REV 1 SOUND BOARD WITH VOTRAX
//**************************************************************************

//-------------------------------------------------
//  gottlieb_sound_r1_with_votrax_device -
//  constructor
//-------------------------------------------------

gottlieb_sound_r1_with_votrax_device::gottlieb_sound_r1_with_votrax_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: gottlieb_sound_r1_device(mconfig, tag, owner, clock, true)
{
}


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor gottlieb_sound_r1_with_votrax_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gottlieb_sound_r1_with_votrax );
}


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

gottlieb_sound_r2_device::gottlieb_sound_r2_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, GOTTLIEB_SOUND_REV2, "Gottlieb Sound rev. 2", tag, owner, clock, "gotsndr2", __FILE__),
		device_mixer_interface(mconfig, *this),
		m_audiocpu(*this, "audiocpu"),
		m_speechcpu(*this, "speechcpu"),
		m_dac(*this, "dac"),
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
//  dac_w - write to one of the two DACs on the
//  board
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::dac_w )
{
	// dual DAC; the first DAC serves as the reference voltage for the
	// second, effectively scaling the output
	m_dac_data[offset] = data;
	m_dac->write_unsigned16(m_dac_data[0] * m_dac_data[1]);
}


//-------------------------------------------------
//  speech_control_w - primary audio control
//  register on the speech board
//-------------------------------------------------

WRITE8_MEMBER( gottlieb_sound_r2_device::speech_control_w )
{
	UINT8 previous = m_speech_control;
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

static ADDRESS_MAP_START( gottlieb_sound_r2_map, AS_PROGRAM, 8, gottlieb_sound_r2_device )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x3c00) AM_RAM
	AM_RANGE(0x4000, 0x4001) AM_MIRROR(0x3ffe) AM_WRITE(dac_w)
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x3fff) AM_READ(audio_data_r)
	AM_RANGE(0xe000, 0xffff) AM_MIRROR(0x2000) AM_ROM
ADDRESS_MAP_END


//-------------------------------------------------
//  sppech CPU address map
//-------------------------------------------------

static ADDRESS_MAP_START( gottlieb_speech_r2_map, AS_PROGRAM, 8, gottlieb_sound_r2_device )
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
//  machine configuration
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( gottlieb_sound_r2 )
	// audio CPUs
	MCFG_CPU_ADD("audiocpu", M6502, SOUND2_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(gottlieb_sound_r2_map)

	MCFG_CPU_ADD("speechcpu", M6502, SOUND2_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(gottlieb_speech_r2_map)

	// sound hardware
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.15)

	MCFG_SOUND_ADD("ay1", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.15)

	MCFG_SOUND_ADD("ay2", AY8913, SOUND2_CLOCK/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 0.15)

	MCFG_SOUND_ADD("spsnd", SP0250, SOUND2_SPEECH_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0)
MACHINE_CONFIG_END


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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, gottlieb_sound_r2_device, speech_drq_custom_r, NULL)
INPUT_PORTS_END


//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor gottlieb_sound_r2_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gottlieb_sound_r2 );
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

	// reset the DACs
	m_dac_data[0] = m_dac_data[1] = 0xff;

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
