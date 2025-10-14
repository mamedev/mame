// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Dan Boris
/* from Andrew Scott (ascott@utkux.utcc.utk.edu) */

/*
  updated by BUT
  - corrected music tempo (not confirmed Satan of Saturn and clone)
  - adjusted music freq (except Satan of Saturn and clone)
  - adjusted music waveform
  - support playing flag for music channel 0
  - support HD38880 speech by samples
*/


#include "emu.h"
#include "snk6502_a.h"

#include "speaker.h"

#ifndef M_LN2
#define M_LN2       0.69314718055994530942
#endif

#define TONE_VOLUME 50

#define SAMPLE_RATE (48000)
#define FRAC_BITS   16
#define FRAC_ONE    (1 << FRAC_BITS)
#define FRAC_MASK   (FRAC_ONE - 1)

static const char *const sasuke_sample_names[] =
{
	"*sasuke",

	// SN76477 and discrete
	"hit",
	"boss_start",
	"shot",
	"boss_attack",

	nullptr
};

static const char *const vanguard_sample_names[] =
{
	"*vanguard",

	// SN76477 and discrete
	"fire",
	"explsion",

	// HD38880 speech
	"vg_voi-0",
	"vg_voi-1",
	"vg_voi-2",
	"vg_voi-3",
	"vg_voi-4",
	"vg_voi-5",
	"vg_voi-6",
	"vg_voi-7",
	"vg_voi-8",
	"vg_voi-9",
	"vg_voi-a",
	"vg_voi-b",
	"vg_voi-c",
	"vg_voi-d",
	"vg_voi-e",
	"vg_voi-f",

	nullptr
};


static const char *const fantasy_sample_names[] =
{
	"*fantasy",

	// HD38880 speech
	"ft_voi-0",
	"ft_voi-1",
	"ft_voi-2",
	"ft_voi-3",
	"ft_voi-4",
	"ft_voi-5",
	"ft_voi-6",
	"ft_voi-7",
	"ft_voi-8",
	"ft_voi-9",
	"ft_voi-a",
	"ft_voi-b",

	nullptr
};


/************************************************************************
 * fantasy Sound System Analog emulation
 * July 2008, D. Renaud
 ************************************************************************/

static const discrete_op_amp_filt_info fantasy_filter =
{
	RES_K(10.5), 0, RES_K(33), 0, RES_K(470), CAP_U(.01), CAP_U(.01), 0, 0, 12, -12
};

#define FANTASY_BOMB_EN             NODE_01
#define FANTASY_NOISE_STREAM_IN     NODE_02
#define FANTASY_NOISE_LOGIC         NODE_03

static DISCRETE_SOUND_START( fantasy_discrete )

	DISCRETE_INPUT_LOGIC (FANTASY_BOMB_EN)
	DISCRETE_INPUT_STREAM(FANTASY_NOISE_STREAM_IN, 0)

	/* This is not the perfect way to discharge, but it is good enough for now */
	/* it does not take into acount that there is no discharge when noise is low */
	DISCRETE_RCDISC2(NODE_10, FANTASY_BOMB_EN, 0, RES_K(10) + RES_K(33), DEFAULT_TTL_V_LOGIC_1 - 0.5, RES_K(1), CAP_U(1))
	DISCRETE_CLAMP(FANTASY_NOISE_LOGIC, FANTASY_NOISE_STREAM_IN, 0, 1)
	DISCRETE_SWITCH(NODE_11, 1, FANTASY_NOISE_LOGIC, 0, NODE_10)

	DISCRETE_OP_AMP_FILTER(NODE_20, 1, NODE_11, 0, DISC_OP_AMP_FILTER_IS_BAND_PASS_1M, &fantasy_filter)
	DISCRETE_RCFILTER(NODE_21, NODE_20, RES_K(22), CAP_U(.01))
	DISCRETE_RCFILTER(NODE_22, NODE_21, RES_K(22) +  RES_K(22), CAP_P(2200))
	DISCRETE_RCFILTER(NODE_23, NODE_22, RES_K(22) + RES_K(22) +  RES_K(22), CAP_U(.001))

	DISCRETE_OUTPUT(NODE_23, 32760.0/12)
DISCRETE_SOUND_END


DEFINE_DEVICE_TYPE(SNK6502_SOUND,  snk6502_sound_device,  "snk6502_sound",  "SNK6502 Custom Sound")
DEFINE_DEVICE_TYPE(VANGUARD_SOUND, vanguard_sound_device, "vanguard_sound", "SNK Vanguard Sound")
DEFINE_DEVICE_TYPE(FANTASY_SOUND,  fantasy_sound_device,  "fantasy_sound",  "SNK Fantasy Sound")
DEFINE_DEVICE_TYPE(NIBBLER_SOUND,  nibbler_sound_device,  "nibbler_sound",  "Rock-Ola Nibbler Sound")
DEFINE_DEVICE_TYPE(PBALLOON_SOUND, pballoon_sound_device, "pballoon_sound", "SNK Pioneer Balloon Sound")
DEFINE_DEVICE_TYPE(SASUKE_SOUND,   sasuke_sound_device,   "sasuke_sound",   "SNK Sasuke Vs. Commander Sound")
DEFINE_DEVICE_TYPE(SATANSAT_SOUND, satansat_sound_device, "satansat_sound", "SNK Satan of Saturn Sound")


snk6502_sound_device::snk6502_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SNK6502_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_tone_clock_expire(0)
	, m_tone_clock(0)
	, m_tone_stream(nullptr)
	, m_samples(*this, "^samples")
	, m_rom(*this, DEVICE_SELF_OWNER)
	, m_sound0_stop_on_rollover(0)
	, m_hd38880_cmd(0)
	, m_hd38880_addr(0)
	, m_hd38880_data_bytes(0)
	, m_hd38880_speed(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void snk6502_sound_device::device_start()
{
	// adjusted
	set_music_freq(43000);

	// 38.99 Hz update (according to schematic)
	set_music_clock(M_LN2 * (RES_K(18) * 2 + RES_K(1)) * CAP_U(1));

	m_tone_stream = stream_alloc(0, 1, SAMPLE_RATE);

	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		save_item(NAME(m_tone_channels[i].mute), i);
		save_item(NAME(m_tone_channels[i].offset), i);
		save_item(NAME(m_tone_channels[i].base), i);
		save_item(NAME(m_tone_channels[i].mask), i);
		save_item(NAME(m_tone_channels[i].sample_step), i);
		save_item(NAME(m_tone_channels[i].sample_cur), i);
		save_item(NAME(m_tone_channels[i].form), i);
	}

	save_item(NAME(m_tone_clock));
	save_item(NAME(m_sound0_stop_on_rollover));
	save_item(NAME(m_hd38880_cmd));
	save_item(NAME(m_hd38880_addr));
	save_item(NAME(m_hd38880_data_bytes));
	save_item(NAME(m_hd38880_speed));
}

inline void snk6502_sound_device::validate_tone_channel(int channel)
{
	if (!m_tone_channels[channel].mute)
	{
		uint8_t romdata = m_rom->base()[m_tone_channels[channel].base + m_tone_channels[channel].offset];

		if (romdata != 0xff)
			m_tone_channels[channel].sample_step = m_tone_channels[channel].sample_rate / (256 - romdata);
		else
			m_tone_channels[channel].sample_step = 0;
	}
}

void snk6502_sound_device::sasuke_build_waveform(int mask)
{
	const int bit0 = BIT(mask, 0);
	const int bit1 = BIT(mask, 1);
	const int bit2 = 1;
	const int bit3 = BIT(mask, 2);
	const int base = (bit0 + bit1 + bit2 + bit3 + 1) / 2;

	for (int i = 0; i < 16; i++)
	{
		const int data = (bit0 & BIT(i, 0)) + (bit1 & BIT(i, 1)) + (bit2 & BIT(i, 2)) + (bit3 & BIT(i, 3));
		m_tone_channels[0].form[i] = data - base;
	}

	for (int i = 0; i < 16; i++)
		m_tone_channels[0].form[i] *= 65535 / 16;
}

void snk6502_sound_device::satansat_build_waveform(int mask)
{
	//logerror("1: wave form = %d\n", mask);
	const int bit0 = 1;
	const int bit1 = 1;
	const int bit2 = 1;
	const int bit3 = BIT(mask, 0);
	const int base = (bit0 + bit1 + bit2 + bit3 + 1) / 2;

	for (int i = 0; i < 16; i++)
	{
		const int data = (bit0 & BIT(i, 0)) + (bit1 & BIT(i, 1)) + (bit2 & BIT(i, 2)) + (bit3 & BIT(i, 3));
		m_tone_channels[1].form[i] = data - base;
	}

	for (int i = 0; i < 16; i++)
		m_tone_channels[1].form[i] *= 65535 / 16;
}

void snk6502_sound_device::build_waveform(int channel, int mask)
{
	//logerror("%d: wave form = %d\n", channel, mask);
	int bit0 = 0;
	int bit1 = 0;
	int bit2 = 0;
	int bit3 = 0;

	// bit 3
	if (BIT(mask, 0) || BIT(mask, 1))
		bit3 = 8;
	else if (BIT(mask, 2))
		bit3 = 4;
	else if (BIT(mask, 3))
		bit3 = 2;

	// bit 2
	if (BIT(mask, 2))
		bit2 = 8;
	else if (BIT(mask, 1) || BIT(mask, 3))
		bit2 = 4;

	// bit 1
	if (BIT(mask, 3))
		bit1 = 8;
	else if (BIT(mask, 2))
		bit1 = 4;
	else if (BIT(mask, 1))
		bit1 = 2;

	// bit 0
	bit0 = bit1 >> 1;

	if (bit0 + bit1 + bit2 + bit3 < 16)
	{
		bit0 <<= 1;
		bit1 <<= 1;
		bit2 <<= 1;
		bit3 <<= 1;
	}

	const int base = (bit0 + bit1 + bit2 + bit3 + 1) / 2;

	for (int i = 0; i < 16; i++)
	{
		/* special channel for fantasy */
		if (channel == 2)
		{
			m_tone_channels[channel].form[i] = BIT(i, 3) ? 7 : -8;
		}
		else
		{
			const int data = (BIT(i, 0) ? bit0 : 0) + (BIT(i, 1) ? bit1 : 0) + (BIT(i, 2) ? bit2 : 0) + (BIT(i, 3) ? bit3 : 0);
			m_tone_channels[channel].form[i] = data - base;
		}
	}

	for (int i = 0; i < 16; i++)
		m_tone_channels[channel].form[i] *= 65535 / 160;
}

void snk6502_sound_device::set_music_freq(int freq)
{
	for (int i = 0; i < NUM_CHANNELS; i++)
	{
		m_tone_channels[i].mute = 1;
		m_tone_channels[i].offset = 0;
		m_tone_channels[i].base = i * 0x800;
		m_tone_channels[i].mask = 0xff;
		m_tone_channels[i].sample_step = 0;
		m_tone_channels[i].sample_cur = 0;
		m_tone_channels[i].sample_rate = double(freq * 8) / SAMPLE_RATE * FRAC_ONE;

		build_waveform(i, 1);
	}
}

void snk6502_sound_device::set_music_clock(double clock_time)
{
	m_tone_clock_expire = clock_time * SAMPLE_RATE * FRAC_ONE;
	m_tone_clock = 0;
}

int snk6502_sound_device::music0_playing()
{
	return m_tone_channels[0].mute ? 1 : 0;
}

void snk6502_sound_device::set_channel_base(int channel, int base, int mask)
{
	m_tone_channels[channel].base = base;
	m_tone_channels[channel].mask = mask;
}

void snk6502_sound_device::mute_channel(int channel)
{
	m_tone_channels[channel].mute = 1;
	m_tone_channels[channel].offset = 0;
}

void snk6502_sound_device::unmute_channel(int channel)
{
	if (m_tone_channels[channel].mute)
		m_tone_channels[channel].offset = 0;
	m_tone_channels[channel].mute = 0;
}


/*
  Hitachi HD38880 speech synthesizer chip

    I heard that this chip uses PARCOR coefficients but I don't know ROM data format.
    How do I generate samples?
*/


/* HD38880 command */
#define HD38880_ADSET   2
#define HD38880_READ    3
#define HD38880_INT1    4
#define HD38880_INT2    6
#define HD38880_SYSPD   8
#define HD38880_STOP    10
#define HD38880_CONDT   11
#define HD38880_START   12
#define HD38880_SSTART  14

/* HD38880 control bits */
#define HD38880_CTP 0x10
#define HD38880_CMV 0x20
#define HD68880_SYBS    0x0f


void snk6502_sound_device::speech_w(uint8_t data, const uint16_t *table, int start)
{
	/*
	    bit description
	    0   SYBS1
	    1   SYBS2
	    2   SYBS3
	    3   SYBS4
	    4   CTP
	    5   CMV
	    6
	    7
	*/

	if ((data & HD38880_CTP) && (data & HD38880_CMV))
	{
		data &= HD68880_SYBS;

		switch (m_hd38880_cmd)
		{
		case 0:
			switch (data)
			{
			case HD38880_START:
				logerror("speech: START\n");

				if (m_hd38880_data_bytes == 5 && !m_samples->playing(0))
				{
					for (int i = 0; i < 16; i++)
					{
						if (table[i] && table[i] == m_hd38880_addr)
						{
							m_samples->start(0, start + i);
							break;
						}
					}
				}
				break;

			case HD38880_SSTART:
				logerror("speech: SSTART\n");
				break;

			case HD38880_STOP:
				m_samples->stop(0);
				logerror("speech: STOP\n");
				break;

			case HD38880_SYSPD:
				m_hd38880_cmd = data;
				break;

			case HD38880_CONDT:
				logerror("speech: CONDT\n");
				break;

			case HD38880_ADSET:
				m_hd38880_cmd = data;
				m_hd38880_addr = 0;
				m_hd38880_data_bytes = 0;
				break;

			case HD38880_READ:
				logerror("speech: READ\n");
				break;

			case HD38880_INT1:
				m_hd38880_cmd = data;
				break;

			case HD38880_INT2:
				m_hd38880_cmd = data;
				break;

			case 0:
				// ignore it
				break;

			default:
				logerror("speech: unknown command: 0x%x\n", data);
			}
			break;

		case HD38880_INT1:
			logerror("speech: INT1: 0x%x\n", data);

			if (data & 8)
				logerror("speech:   triangular waveform\n");
			else
				logerror("speech:   impulse waveform\n");

			logerror("speech:   %sable losing effect of vocal tract\n", data & 4 ? "en" : "dis");

			if ((data & 2) && (data & 8))
				logerror("speech:   use external pitch control\n");

			m_hd38880_cmd = 0;
			break;

		case HD38880_INT2:
			logerror("speech: INT2: 0x%x\n", data);

			logerror("speech:   %d bits / frame\n", data & 8 ? 48 : 96);
			logerror("speech:   %d ms / frame\n", data & 4 ? 20 : 10);
			logerror("speech:   %sable repeat\n", data & 2 ? "en" : "dis");
			logerror("speech:   %d operations\n", ((data & 8) == 0) || (data & 1) ? 10 : 8);

			m_hd38880_cmd = 0;
			break;

		case HD38880_SYSPD:
			m_hd38880_speed = ((double)(data + 1)) / 10.0;
			logerror("speech: SYSPD: %1.1f\n", m_hd38880_speed);
			m_hd38880_cmd = 0;
			break;

		case HD38880_ADSET:
			m_hd38880_addr |= (data << (m_hd38880_data_bytes++ * 4));
			if (m_hd38880_data_bytes == 5)
			{
				logerror("speech: ADSET: 0x%05x\n", m_hd38880_addr);
				m_hd38880_cmd = 0;
			}
			break;
		}
	}
}


/*
 vanguard/fantasy speech

 ROM data format (INT2 = 0xf):
  48 bits / frame
  20 ms / frame
  enable repeat
  10 operations
*/

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void snk6502_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < NUM_CHANNELS; i++)
		validate_tone_channel(i);

	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		int32_t data = 0;

		for (tone_t &voice : m_tone_channels)
		{
			int16_t const *const form = voice.form;

			if (!voice.mute && voice.sample_step)
			{
				const int cur_pos = voice.sample_cur + voice.sample_step;
				const int prev = form[(voice.sample_cur >> FRAC_BITS) & 15];
				const int cur = form[(cur_pos >> FRAC_BITS) & 15];

				/* interpolate */
				data += (int32_t(prev) * (FRAC_ONE - (cur_pos & FRAC_MASK))
						+ int32_t(cur) * (cur_pos & FRAC_MASK)) >> FRAC_BITS;

				voice.sample_cur = cur_pos;
			}
		}

		stream.put_int(0, sampindex, data, 3768);

		m_tone_clock += FRAC_ONE;
		if (m_tone_clock >= m_tone_clock_expire)
		{
			for (int i = 0; i < NUM_CHANNELS; i++)
			{
				m_tone_channels[i].offset++;
				m_tone_channels[i].offset &= m_tone_channels[i].mask;

				validate_tone_channel(i);
			}

			if (m_tone_channels[0].offset == 0 && m_sound0_stop_on_rollover)
				m_tone_channels[0].mute = 1;

			m_tone_clock -= m_tone_clock_expire;
		}
	}
}


vanguard_sound_device::vanguard_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VANGUARD_SOUND, tag, owner, clock)
	, m_custom(*this, "custom")
	, m_sn76477_2(*this, "sn76477.2")
	, m_samples(*this, "samples")
	, m_last_port1(0)
{
}

void vanguard_sound_device::sound_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		/*
		    bit description

		    0   MUSIC A10
		    1   MUSIC A9
		    2   MUSIC A8
		    3   LS05 PORT 1
		    4   LS04 PORT 2
		    5   SHOT A
		    6   SHOT B
		    7   BOMB
		*/

		/* select musical tune in ROM based on sound command byte */
		m_custom->set_channel_base(0, (data & 0x07) << 8);

		m_custom->set_sound0_stop_on_rollover(1);

		/* play noise samples requested by sound command byte */
		/* SHOT A */
		if (data & 0x20 && !(m_last_port1 & 0x20))
			m_samples->start(1, 0);
		else if (!(data & 0x20) && m_last_port1 & 0x20)
			m_samples->stop(1);

		/* BOMB */
		if (data & 0x80 && !(m_last_port1 & 0x80))
			m_samples->start(2, 1);

		if (data & 0x08)
			m_custom->mute_channel(0);

		if (data & 0x10)
			m_custom->unmute_channel(0);

		/* SHOT B */
		m_sn76477_2->enable_w((data & 0x40) ? 0 : 1);

		m_last_port1 = data;
		break;

	case 1:
		/*
		    bit description

		    0   MUSIC A10
		    1   MUSIC A9
		    2   MUSIC A8
		    3   LS04 PORT 3
		    4   EXTP A (HD38880 external pitch control A)
		    5   EXTP B (HD38880 external pitch control B)
		    6
		    7
		*/

		/* select tune in ROM based on sound command byte */
		m_custom->set_channel_base(1, 0x0800 | ((data & 0x07) << 8));

		if (data & 0x08)
			m_custom->unmute_channel(1);
		else
			m_custom->mute_channel(1);
		break;

	case 2:
		/*
		    bit description

		    0   AS 1    (sound0 waveform)
		    1   AS 2    (sound0 waveform)
		    2   AS 4    (sound0 waveform)
		    3   AS 3    (sound0 waveform)
		    4   AS 5    (sound1 waveform)
		    5   AS 6    (sound1 waveform)
		    6   AS 7    (sound1 waveform)
		    7   AS 8    (sound1 waveform)
		*/

		m_custom->build_waveform(0, (data & 0x3) | ((data & 4) << 1) | ((data & 8) >> 1));
		m_custom->build_waveform(1, data >> 4);
	}
}

void vanguard_sound_device::speech_w(uint8_t data)
{
	static const uint16_t vanguard_table[16] =
	{
		0x04000,
		0x04325,
		0x044a2,
		0x045b7,
		0x046ee,
		0x04838,
		0x04984,
		0x04b01,
		0x04c38,
		0x04de6,
		0x04f43,
		0x05048,
		0x05160,
		0x05289,
		0x0539e,
		0x054ce
	};

	m_custom->speech_w(data, vanguard_table, 2);
}

void vanguard_sound_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SNK6502_SOUND(config, m_custom, 0);
	m_custom->add_route(ALL_OUTPUTS, "mono", 0.50);

	SAMPLES(config, m_samples);
	m_samples->set_channels(3);
	m_samples->set_samples_names(vanguard_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.25);

	sn76477_device &sn76477_1(SN76477(config, "sn76477.1"));
	// SHOT A   GND: 2,9,26,27  +5V: 15,25
	sn76477_1.set_noise_params(RES_K(470), RES_M(1.5), CAP_P(220));
	sn76477_1.set_decay_res(0);
	sn76477_1.set_attack_params(0, 0);
	sn76477_1.set_amp_res(RES_K(47));
	sn76477_1.set_feedback_res(RES_K(4.7));
	sn76477_1.set_vco_params(0, 0, 0);
	sn76477_1.set_pitch_voltage(0);
	sn76477_1.set_slf_params(0, 0);
	sn76477_1.set_oneshot_params(0, 0);
	sn76477_1.set_vco_mode(0);
	sn76477_1.set_mixer_params(0, 1, 0);
	sn76477_1.set_envelope_params(1, 1);
	sn76477_1.set_enable(1);
	sn76477_1.add_route(ALL_OUTPUTS, "mono", 0.50);

	SN76477(config, m_sn76477_2);
	// SHOT B   GND: 1,2,26,27  +5V: 15,25,28
	m_sn76477_2->set_noise_params(RES_K(10), RES_K(30), 0);
	m_sn76477_2->set_decay_res(0);
	m_sn76477_2->set_attack_params(0, 0);
	m_sn76477_2->set_amp_res(RES_K(47));
	m_sn76477_2->set_feedback_res(RES_K(4.7));
	m_sn76477_2->set_vco_params(0, 0, 0);
	m_sn76477_2->set_pitch_voltage(0);
	m_sn76477_2->set_slf_params(0, 0);
	m_sn76477_2->set_oneshot_params(0, 0);
	m_sn76477_2->set_vco_mode(0);
	m_sn76477_2->set_mixer_params(0, 1, 0);
	m_sn76477_2->set_envelope_params(0, 1);
	m_sn76477_2->set_enable(1);
	m_sn76477_2->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void vanguard_sound_device::device_start()
{
	save_item(NAME(m_last_port1));
}

void vanguard_sound_device::device_reset()
{
	// 41.6 Hz update (measured)
	m_custom->set_music_clock(1 / 41.6);
}


fantasy_sound_device::fantasy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fantasy_sound_device(mconfig, FANTASY_SOUND, tag, owner, clock)
{
}

fantasy_sound_device::fantasy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_custom(*this, "custom")
	, m_discrete(*this, "discrete")
	, m_last_port1(0)
{
}

void fantasy_sound_device::sound_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		/*
		    bit description

		    0   MUSIC A10
		    1   MUSIC A9
		    2   MUSIC A8
		    3   LS04 PART 1
		    4   LS04 PART 2
		    5
		    6
		    7   BOMB
		*/

		/* select musical tune in ROM based on sound command byte */
		m_custom->set_channel_base(0, 0x0000 | ((data & 0x07) << 8));

		m_custom->set_sound0_stop_on_rollover(0);

		if (data & 0x08)
			m_custom->unmute_channel(0);
		else
			m_custom->mute_channel(0);

		if (data & 0x10)
			m_custom->unmute_channel(2);
		else
			m_custom->mute_channel(2);

		/* BOMB */
		m_discrete->write(FANTASY_BOMB_EN, data & 0x80);

		m_last_port1 = data;
		break;

	case 1:
		/*
		    bit description

		    0   MUSIC A10
		    1   MUSIC A9
		    2   MUSIC A8
		    3   LS04 PART 3
		    4   EXT PA (HD38880 external pitch control A)
		    5   EXT PB (HD38880 external pitch control B)
		    6
		    7
		*/

		/* select tune in ROM based on sound command byte */
		m_custom->set_channel_base(1, 0x0800 | ((data & 0x07) << 8));

		if (data & 0x08)
			m_custom->unmute_channel(1);
		else
			m_custom->mute_channel(1);
		break;

	case 2:
		/*
		    bit description

		    0   AS 1    (sound0 waveform)
		    1   AS 3    (sound0 waveform)
		    2   AS 2    (sound0 waveform)
		    3   AS 4    (sound0 waveform)
		    4   AS 5    (sound1 waveform)
		    5   AS 6    (sound1 waveform)
		    6   AS 7    (sound1 waveform)
		    7   AS 8    (sound1 waveform)
		*/

		m_custom->build_waveform(0, (data & 0x9) | ((data & 2) << 1) | ((data & 4) >> 1));
		m_custom->build_waveform(1, data >> 4);
		break;

	case 3:
		/*
		    bit description

		    0   BC 1
		    1   BC 2
		    2   BC 3
		    3   MUSIC A10
		    4   MUSIC A9
		    5   MUSIC A8
		    6
		    7   INV
		*/

		/* select tune in ROM based on sound command byte */
		m_custom->set_channel_base(2, 0x1000 | ((data & 0x70) << 4));
		break;
	}
}

void fantasy_sound_device::speech_w(uint8_t data)
{
	static const uint16_t fantasy_table[16] =
	{
		0x04000,
		0x04297,
		0x044b6,
		0x04682,
		0x04927,
		0x04be0,
		0x04cc2,
		0x04e36,
		0x05000,
		0x05163,
		0x052c9,
		0x053fd,
		0,
		0,
		0,
		0
	};

	m_custom->speech_w(data, fantasy_table, 0);
}

void fantasy_sound_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SNK6502_SOUND(config, m_custom, 0);
	m_custom->add_route(ALL_OUTPUTS, "mono", 0.50);

	samples_device &samples(SAMPLES(config, "samples"));
	samples.set_channels(1);
	samples.set_samples_names(fantasy_sample_names);
	samples.add_route(ALL_OUTPUTS, "mono", 0.5);

	sn76477_device &sn76477_1(SN76477(config, "sn76477.1"));
	// BOMB     GND:    2,9,26,27       +5V: 15,25
	sn76477_1.set_noise_params(RES_K(470), RES_M(1.5), CAP_P(220));
	sn76477_1.set_decay_res(0);
	sn76477_1.set_attack_params(0, 0);
	sn76477_1.set_amp_res(RES_K(470));
	sn76477_1.set_feedback_res(RES_K(4.7));
	sn76477_1.set_vco_params(0, 0, 0);
	sn76477_1.set_pitch_voltage(0);
	sn76477_1.set_slf_params(0, 0);
	sn76477_1.set_oneshot_params(0, 0);
	sn76477_1.set_vco_mode(0);
	sn76477_1.set_mixer_params(0, 1, 0);
	// schematic does not show pin 1 grounded, but it must be.
	// otherwise it is using the VCO for the envelope, but the VCO is not hooked up
	sn76477_1.set_envelope_params(0, 1);
	sn76477_1.set_enable(0);
	sn76477_1.add_route(0, "discrete", 1.0, 0);

	DISCRETE(config, m_discrete, fantasy_discrete);
	m_discrete->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void fantasy_sound_device::device_start()
{
	save_item(NAME(m_last_port1));
}

void fantasy_sound_device::device_reset()
{
	// 41.6 Hz update (measured)
	m_custom->set_music_clock(1 / 41.6);
}


nibbler_sound_device::nibbler_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fantasy_sound_device(mconfig, NIBBLER_SOUND, tag, owner, clock)
{
}

void nibbler_sound_device::device_add_mconfig(machine_config &config)
{
	fantasy_sound_device::device_add_mconfig(config);

	config.device_remove("samples");
}


pballoon_sound_device::pballoon_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fantasy_sound_device(mconfig, PBALLOON_SOUND, tag, owner, clock)
{
}

void pballoon_sound_device::device_add_mconfig(machine_config &config)
{
	fantasy_sound_device::device_add_mconfig(config);

	config.device_remove("samples");
}

void pballoon_sound_device::device_reset()
{
	// 40.3 Hz update (measured)
	m_custom->set_music_clock(1 / 40.3);
}


sasuke_sound_device::sasuke_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SASUKE_SOUND, tag, owner, clock)
	, m_custom(*this, "custom")
	, m_samples(*this, "samples")
	, m_last_port1(0)
{
}

void sasuke_sound_device::sound_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		/*
		    bit description

		    0   hit (ic52)
		    1   boss start (ic51)
		    2   shot
		    3   boss attack (ic48?)
		    4   ??
		    5
		    6
		    7   reset counter
		*/

		if (BIT(~data & m_last_port1, 0))
			m_samples->start(0, 0);
		if (BIT(~data & m_last_port1, 1))
			m_samples->start(1, 1);
		if (BIT(~data & m_last_port1, 2))
			m_samples->start(2, 2);
		if (BIT(~data & m_last_port1, 3))
			m_samples->start(3, 3);

		if (BIT(data & ~m_last_port1, 7))
		{
			m_custom->reset_offset(0); // TODO: why is this the only game that needs this when unmuting?
			m_custom->unmute_channel(0);
		}
		else if (BIT(~data & m_last_port1, 7))
			m_custom->mute_channel(0);

		m_last_port1 = data;
		break;

	case 1:
		/*
		    bit description

		    0
		    1   wave form
		    2   wave form
		    3   wave form
		    4   MUSIC A8
		    5   MUSIC A9
		    6   MUSIC A10
		    7
		*/

		/* select tune in ROM based on sound command byte */
		m_custom->set_channel_base(0, 0x0000 | ((data & 0x70) << 4));

		m_custom->set_sound0_stop_on_rollover(1);

		/* bit 1-3 sound0 waveform control */
		m_custom->sasuke_build_waveform((data & 0x0e) >> 1);
		break;
	}
}

void sasuke_sound_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SNK6502_SOUND(config, m_custom, 0);
	m_custom->add_route(ALL_OUTPUTS, "mono", 0.50);

	samples_device &samples(SAMPLES(config, "samples"));
	samples.set_channels(4);
	samples.set_samples_names(sasuke_sample_names);
	samples.add_route(ALL_OUTPUTS, "mono", 0.12);

	sn76477_device &sn76477_1(SN76477(config, "sn76477.1"));
	// ic48     GND: 2,22,26,27,28  +5V: 1,15,25
	sn76477_1.set_noise_params(RES_K(470), RES_K(150), CAP_P(4700));
	sn76477_1.set_decay_res(RES_K(22));
	sn76477_1.set_attack_params(CAP_U(10), RES_K(10));
	sn76477_1.set_amp_res(RES_K(100));
	sn76477_1.set_feedback_res(RES_K(47));
	sn76477_1.set_vco_params(0, 0, 0);
	sn76477_1.set_pitch_voltage(0);
	sn76477_1.set_slf_params(0, RES_K(10));
	sn76477_1.set_oneshot_params(CAP_U(2.2), RES_K(100));
	sn76477_1.set_vco_mode(0);
	sn76477_1.set_mixer_params(0, 1, 0);
	sn76477_1.set_envelope_params(1, 0);
	sn76477_1.set_enable(1);
	sn76477_1.add_route(ALL_OUTPUTS, "mono", 0.50);

	sn76477_device &sn76477_2(SN76477(config, "sn76477.2"));
	// ic51     GND: 2,26,27        +5V: 1,15,22,25,28
	sn76477_2.set_noise_params(RES_K(340), RES_K(47), CAP_P(100));
	sn76477_2.set_decay_res(RES_K(470));
	sn76477_2.set_attack_params(CAP_U(4.7), RES_K(10));
	sn76477_2.set_amp_res(RES_K(100));
	sn76477_2.set_feedback_res(RES_K(47));
	sn76477_2.set_vco_params(0, CAP_P(220), RES_K(1000));
	sn76477_2.set_pitch_voltage(0);
	sn76477_2.set_slf_params(0, RES_K(220));
	sn76477_2.set_oneshot_params(CAP_U(22), RES_K(47));
	sn76477_2.set_vco_mode(1);
	sn76477_2.set_mixer_params(0, 1, 0);
	sn76477_2.set_envelope_params(1, 1);
	sn76477_2.set_enable(1);
	sn76477_2.add_route(ALL_OUTPUTS, "mono", 0.50);

	sn76477_device &sn76477_3(SN76477(config, "sn76477.3"));
	// ic52     GND: 2,22,27,28     +5V: 1,15,25,26
	sn76477_3.set_noise_params(RES_K(330), RES_K(47), CAP_P(100));
	sn76477_3.set_decay_res(RES_K(1));
	sn76477_3.set_attack_params(0, RES_K(1));
	sn76477_3.set_amp_res(RES_K(100));
	sn76477_3.set_feedback_res(RES_K(47));
	sn76477_3.set_vco_params(0, CAP_P(1000), RES_K(1000));
	sn76477_3.set_pitch_voltage(0);
	sn76477_3.set_slf_params(CAP_U(1), RES_K(10));
	sn76477_3.set_oneshot_params(CAP_U(2.2), RES_K(150));
	sn76477_3.set_vco_mode(0);
	sn76477_3.set_mixer_params(1, 1, 0);
	sn76477_3.set_envelope_params(1, 0);
	sn76477_3.set_enable(1);
	sn76477_3.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void sasuke_sound_device::device_start()
{
	save_item(NAME(m_last_port1));
}

void sasuke_sound_device::device_reset()
{
	m_custom->set_music_clock(M_LN2 * (RES_K(18) + RES_K(1)) * CAP_U(1));

	// adjusted (measured through audio recording of pcb)
	m_custom->set_music_freq(35300);
}


satansat_sound_device::satansat_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SATANSAT_SOUND, tag, owner, clock)
	, m_custom(*this, "custom")
	, m_samples(*this, "samples")
	, m_last_port1(0)
{
}

void satansat_sound_device::sound_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0:
		/*
		    bit description

		*/

		/* bit 0 = analog sound trigger */

		/* bit 1 = to 76477 */

		/* bit 2 = analog sound trigger */
		if (data & 0x04 && !(m_last_port1 & 0x04))
			m_samples->start(0, 1);

		if (data & 0x08)
			m_custom->mute_channel(0);

		/* bit 4-6 sound0 waveform control */
		m_custom->sasuke_build_waveform((data & 0x70) >> 4);

		/* bit 7 sound1 waveform control */
		m_custom->satansat_build_waveform((data & 0x80) >> 7);

		m_last_port1 = data;
		break;

	case 1:
		/*
		    bit description

		*/

		/* select tune in ROM based on sound command byte */
		m_custom->set_channel_base(0, 0x0000 | ((data & 0x0e) << 7));
		m_custom->set_channel_base(1, 0x0800 | ((data & 0x60) << 4), 0x1ff);

		m_custom->set_sound0_stop_on_rollover(1);

		if (data & 0x01)
			m_custom->unmute_channel(0);

		if (data & 0x10)
			m_custom->unmute_channel(1);
		else
			m_custom->mute_channel(1);

		/* bit 7 = ? */
		break;
	}
}

void satansat_sound_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	SNK6502_SOUND(config, m_custom, 0);
	m_custom->add_route(ALL_OUTPUTS, "mono", 0.50);

	samples_device &samples(SAMPLES(config, "samples"));
	samples.set_channels(3);
	samples.set_samples_names(vanguard_sample_names);
	samples.add_route(ALL_OUTPUTS, "mono", 0.25);

	sn76477_device &sn76477_1(SN76477(config, "sn76477.1"));
	// ???      GND: 2,26,27        +5V: 15,25
	sn76477_1.set_noise_params(RES_K(470), RES_M(1.5), CAP_P(220));
	sn76477_1.set_decay_res(0);
	sn76477_1.set_attack_params(0, 0);
	sn76477_1.set_amp_res(RES_K(47));
	sn76477_1.set_feedback_res(RES_K(47));
	sn76477_1.set_vco_params(0, 0, 0);
	sn76477_1.set_pitch_voltage(0);
	sn76477_1.set_slf_params(0, 0);
	sn76477_1.set_oneshot_params(0, 0);
	sn76477_1.set_vco_mode(0);
	sn76477_1.set_mixer_params(0, 1, 0);
	sn76477_1.set_envelope_params(1, 1);
	sn76477_1.set_enable(1);
	sn76477_1.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void satansat_sound_device::device_start()
{
	save_item(NAME(m_last_port1));
}

void satansat_sound_device::device_reset()
{
	// same as sasuke (assumption?)
	// NOTE: this was set before sasuke was adjusted to a lower freq, please don't modify until measured/confirmed on pcb
	m_custom->set_music_freq(38000);
}
