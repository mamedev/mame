// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    segasnd.c

    Sound boards for early Sega G-80 based games.

***************************************************************************/

#include "emu.h"
#include "segasnd.h"

#include "sound/sp0250.h"
#include "includes/segag80r.h"
#include "includes/segag80v.h"

#include <cmath>


#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SPEECH_MASTER_CLOCK 3120000

#define USB_MASTER_CLOCK    6000000
#define USB_2MHZ_CLOCK      (USB_MASTER_CLOCK/3)
#define USB_PCS_CLOCK       (USB_2MHZ_CLOCK/2)
#define USB_GOS_CLOCK       (USB_2MHZ_CLOCK/16/4)
#define MM5837_CLOCK        100000

#define SAMPLE_RATE         (USB_2MHZ_CLOCK/8)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline void usb_sound_device::g80_filter_state::configure(double r, double c)
{
	capval = 0.0;
	exponent = 1.0 - std::exp(-1.0 / (r * c * SAMPLE_RATE));
}


inline double usb_sound_device::g80_filter_state::step_rc(double input)
{
	return capval += (input - capval) * exponent;
}


inline double usb_sound_device::g80_filter_state::step_cr(double input)
{
	double const result = input - capval;
	capval += result * exponent;
	return result;
}



/***************************************************************************
    SPEECH BOARD
***************************************************************************/

DEFINE_DEVICE_TYPE(SEGASPEECH, speech_sound_device, "sega_speech_sound", "Sega Speech Sound Board")

#define SEGASPEECH_REGION "speech"

speech_sound_device::speech_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEGASPEECH, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_int_cb(*this),
		m_speech(*this, SEGASPEECH_REGION),
		m_drq(0),
		m_latch(0),
		m_t0(0),
		m_p2(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void speech_sound_device::device_start()
{
	m_int_cb.resolve();

	save_item(NAME(m_latch));
	save_item(NAME(m_t0));
	save_item(NAME(m_p2));
	save_item(NAME(m_drq));
}


/*************************************
 *
 *  i8035 port accessors
 *
 *************************************/



READ_LINE_MEMBER( speech_sound_device::t0_r )
{
	return m_t0;
}

READ_LINE_MEMBER( speech_sound_device::t1_r )
{
	return m_drq;
}

READ8_MEMBER( speech_sound_device::p1_r )
{
	return m_latch & 0x7f;
}

READ8_MEMBER( speech_sound_device::rom_r )
{
	return m_speech->base()[0x100 * (m_p2 & 0x3f) + offset];
}

WRITE8_MEMBER( speech_sound_device::p1_w )
{
	if (!(data & 0x80))
		m_t0 = 0;
}

WRITE8_MEMBER( speech_sound_device::p2_w )
{
	m_p2 = data;
}



/*************************************
 *
 *  i8035 port accessors
 *
 *************************************/

WRITE_LINE_MEMBER(speech_sound_device::drq_w)
{
	m_drq = (state == ASSERT_LINE);
}



/*************************************
 *
 *  External access
 *
 *************************************/

TIMER_CALLBACK_MEMBER( speech_sound_device::delayed_speech_w )
{
	int data = param;
	u8 old = m_latch;

	/* all 8 bits are latched */
	m_latch = data;

	/* the high bit goes directly to the INT line */
	m_int_cb((data & 0x80) ? CLEAR_LINE : ASSERT_LINE);

	/* a clock on the high bit clocks a 1 into T0 */
	if (!(old & 0x80) && (data & 0x80))
		m_t0 = 1;
}


WRITE8_MEMBER( speech_sound_device::data_w )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(speech_sound_device::delayed_speech_w), this), data);
}


WRITE8_MEMBER( speech_sound_device::control_w )
{
	LOG("Speech control = %X\n", data);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void speech_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
}

/*************************************
 *
 *  Speech board functions
 *
 *************************************/

WRITE_LINE_MEMBER(segag80snd_common::segaspeech_int_w)
{
	m_audiocpu->set_input_line(0, state);
}

/*************************************
 *
 *  Speech board address maps
 *
 *************************************/

void segag80snd_common::speech_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).rom();
}


void segag80snd_common::speech_portmap(address_map &map)
{
	map(0x00, 0xff).r("segaspeech", FUNC(speech_sound_device::rom_r));
	map(0x00, 0xff).w("speech", FUNC(sp0250_device::write));
}


/*************************************
 *
 *  Speech board machine drivers
 *
 *************************************/

void segag80snd_common::sega_speech_board(machine_config &config)
{
	/* CPU for the speech board */
	i8035_device &audiocpu(I8035(config, m_audiocpu, SPEECH_MASTER_CLOCK));        /* divide by 15 in CPU */
	audiocpu.set_addrmap(AS_PROGRAM, &segag80snd_common::speech_map);
	audiocpu.set_addrmap(AS_IO, &segag80snd_common::speech_portmap);
	audiocpu.p1_in_cb().set("segaspeech", FUNC(speech_sound_device::p1_r));
	audiocpu.p1_out_cb().set("segaspeech", FUNC(speech_sound_device::p1_w));
	audiocpu.p2_out_cb().set("segaspeech", FUNC(speech_sound_device::p2_w));
	audiocpu.t0_in_cb().set("segaspeech", FUNC(speech_sound_device::t0_r));
	audiocpu.t1_in_cb().set("segaspeech", FUNC(speech_sound_device::t1_r));

	/* sound hardware */
	SEGASPEECH(config, "segaspeech", 0).int_cb().set(FUNC(segag80snd_common::segaspeech_int_w));
	sp0250_device &speech(SP0250(config, "speech", SPEECH_MASTER_CLOCK));
	speech.drq().set("segaspeech", FUNC(speech_sound_device::drq_w));
	speech.add_route(ALL_OUTPUTS, "speaker", 1.0);
}



/***************************************************************************
    UNIVERSAL SOUND BOARD
***************************************************************************/

DEFINE_DEVICE_TYPE(SEGAUSB, usb_sound_device, "segausb", "Sega Universal Sound Board")

usb_sound_device::usb_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_ourcpu(*this, "ourcpu"),
		m_maincpu(*this, finder_base::DUMMY_TAG),
		m_stream(nullptr),
		m_in_latch(0),
		m_out_latch(0),
		m_last_p2_value(0),
		m_program_ram(*this, "pgmram"),
		m_work_ram(*this, "workram"),
		m_work_ram_bank(0),
		m_t1_clock(0),
		m_t1_clock_mask(0),
		m_noise_shift(0),
		m_noise_state(0),
		m_noise_subcount(0)
{
}

usb_sound_device::usb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: usb_sound_device(mconfig, SEGAUSB, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void usb_sound_device::device_start()
{
	/* create a sound stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 1, SAMPLE_RATE);

	/* initialize state */
	m_noise_shift = 0x15555;

	for (timer8253 &g : m_timer_group)
	{
		g.chan_filter[0].configure(10e3, 1e-6);
		g.chan_filter[1].configure(10e3, 1e-6);
		g.gate1.configure(100e3, 0.01e-6);
		g.gate2.configure(2 * 100e3, 0.01e-6);
	}

	g80_filter_state temp;
	temp.configure(100e3, 0.01e-6);
	m_gate_rc1_exp[0] = temp.exponent;
	temp.configure(1e3, 0.01e-6);
	m_gate_rc1_exp[1] = temp.exponent;
	temp.configure(2 * 100e3, 0.01e-6);
	m_gate_rc2_exp[0] = temp.exponent;
	temp.configure(2 * 1e3, 0.01e-6);
	m_gate_rc2_exp[1] = temp.exponent;

	m_noise_filters[0].configure(2.7e3 + 2.7e3, 1.0e-6);
	m_noise_filters[1].configure(2.7e3 + 1e3, 0.30e-6);
	m_noise_filters[2].configure(2.7e3 + 270, 0.15e-6);
	m_noise_filters[3].configure(2.7e3 + 0, 0.082e-6);
	m_noise_filters[4].configure(33e3, 0.1e-6);

	m_final_filter.configure(100e3, 4.7e-6);

	/* register for save states */
	save_item(NAME(m_in_latch));
	save_item(NAME(m_out_latch));
	save_item(NAME(m_last_p2_value));
	save_item(NAME(m_work_ram_bank));
	save_item(NAME(m_t1_clock));

	for (int tgroup = 0; tgroup < 3; tgroup++)
	{
		timer8253 *group = &m_timer_group[tgroup];
		for (int tchan = 0; tchan < 3; tchan++)
		{
			timer8253::channel *channel = &group->chan[tchan];
			save_item(NAME(channel->holding), tgroup * 3 + tchan);
			save_item(NAME(channel->latchmode), tgroup * 3 + tchan);
			save_item(NAME(channel->latchtoggle), tgroup * 3 + tchan);
			save_item(NAME(channel->clockmode), tgroup * 3 + tchan);
			save_item(NAME(channel->bcdmode), tgroup * 3 + tchan);
			save_item(NAME(channel->output), tgroup * 3 + tchan);
			save_item(NAME(channel->lastgate), tgroup * 3 + tchan);
			save_item(NAME(channel->gate), tgroup * 3 + tchan);
			save_item(NAME(channel->subcount), tgroup * 3 + tchan);
			save_item(NAME(channel->count), tgroup * 3 + tchan);
			save_item(NAME(channel->remain), tgroup * 3 + tchan);
		}
		save_item(NAME(group->env), tgroup);
		save_item(NAME(group->chan_filter[0].capval), tgroup);
		save_item(NAME(group->chan_filter[1].capval), tgroup);
		save_item(NAME(group->gate1.capval), tgroup);
		save_item(NAME(group->gate2.capval), tgroup);
		save_item(NAME(group->config), tgroup);
	}

	save_item(NAME(m_timer_mode));
	save_item(NAME(m_noise_shift));
	save_item(NAME(m_noise_state));
	save_item(NAME(m_noise_subcount));
	save_item(NAME(m_final_filter.capval));
	save_item(NAME(m_noise_filters[0].capval));
	save_item(NAME(m_noise_filters[1].capval));
	save_item(NAME(m_noise_filters[2].capval));
	save_item(NAME(m_noise_filters[3].capval));
	save_item(NAME(m_noise_filters[4].capval));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void usb_sound_device::device_reset()
{
	/* halt the USB CPU at reset time */
	m_ourcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	/* start the clock timer */
	m_t1_clock_mask = 0x10;
}

/*************************************
 *
 *  Initialization/reset
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER( usb_sound_device::increment_t1_clock_timer_cb )
{
	/* only increment if it is not being forced clear */
	if (!(m_last_p2_value & 0x80))
		m_t1_clock++;
}

/*************************************
 *
 *  External access
 *
 *************************************/

READ8_MEMBER( usb_sound_device::status_r )
{
	LOG("%s:usb_data_r = %02X\n", machine().describe_context(), (m_out_latch & 0x81) | (m_in_latch & 0x7e));

	m_maincpu->adjust_icount(-200);

	/* only bits 0 and 7 are controlled by the I8035; the remaining */
	/* bits 1-6 reflect the current input latch values */
	return (m_out_latch & 0x81) | (m_in_latch & 0x7e);
}


TIMER_CALLBACK_MEMBER( usb_sound_device::delayed_usb_data_w )
{
	int data = param;

	/* look for rising/falling edges of bit 7 to control the RESET line */
	m_ourcpu->set_input_line(INPUT_LINE_RESET, (data & 0x80) ? ASSERT_LINE : CLEAR_LINE);

	/* if the CLEAR line is set, the low 7 bits of the input are ignored */
	if ((m_last_p2_value & 0x40) == 0)
		data &= ~0x7f;

	/* update the effective input latch */
	m_in_latch = data;
}


WRITE8_MEMBER( usb_sound_device::data_w )
{
	LOG("%s:usb_data_w = %02X\n", machine().describe_context(), data);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(usb_sound_device::delayed_usb_data_w), this), data);

	/* boost the interleave so that sequences can be sent */
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(250));
}


READ8_MEMBER( usb_sound_device::ram_r )
{
	return m_program_ram[offset];
}


WRITE8_MEMBER( usb_sound_device::ram_w )
{
	if (m_in_latch & 0x80)
		m_program_ram[offset] = data;
	else
		LOG("%s:sega_usb_ram_w(%03X) = %02X while /LOAD disabled\n", machine().describe_context(), offset, data);
}



/*************************************
 *
 *  I8035 port accesses
 *
 *************************************/

READ8_MEMBER( usb_sound_device::p1_r )
{
	/* bits 0-6 are inputs and map to bits 0-6 of the input latch */
	if ((m_in_latch & 0x7f) != 0)
		LOG("%s: P1 read = %02X\n", machine().describe_context(), m_in_latch & 0x7f);
	return m_in_latch & 0x7f;
}


WRITE8_MEMBER( usb_sound_device::p1_w )
{
	/* bit 7 maps to bit 0 on the output latch */
	m_out_latch = (m_out_latch & 0xfe) | (data >> 7);
	LOG("%s: P1 write = %02X\n", machine().describe_context(), data);
}


WRITE8_MEMBER( usb_sound_device::p2_w )
{
	u8 old = m_last_p2_value;
	m_last_p2_value = data;

	/* low 2 bits control the bank of work RAM we are addressing */
	m_work_ram_bank = data & 3;

	/* bit 6 controls the "ready" bit output to the host */
	/* it also clears the input latch from the host (active low) */
	m_out_latch = ((data & 0x40) << 1) | (m_out_latch & 0x7f);
	if ((data & 0x40) == 0)
		m_in_latch = 0;

	/* bit 7 controls the reset on the upper counter at U33 */
	if ((old & 0x80) && !(data & 0x80))
		m_t1_clock = 0;

	LOG("%s: P2 write -> bank=%d ready=%d clock=%d\n", machine().describe_context(), data & 3, (data >> 6) & 1, (data >> 7) & 1);
}


READ_LINE_MEMBER( usb_sound_device::t1_r )
{
	/* T1 returns 1 based on the value of the T1 clock; the exact */
	/* pattern is determined by one or more jumpers on the board. */
	return (m_t1_clock & m_t1_clock_mask) != 0;
}



/*************************************
 *
 *  Sound generation
 *
 *************************************/

inline void usb_sound_device::timer8253::channel::clock()
{
	u8 const old_lastgate = lastgate;

	/* update the gate */
	lastgate = gate;

	/* if we're holding, skip */
	if (holding)
		return;

	/* switch off the clock mode */
	switch (clockmode)
	{
		/* oneshot; waits for trigger to restart */
		case 1:
			if (!old_lastgate && gate)
			{
				output = 0;
				remain = count;
			}
			else
			{
				if (--remain == 0)
					output = 1;
			}
			break;

		/* square wave: counts down by 2 and toggles output */
		case 3:
			remain = (remain - 1) & ~1;
			if (remain == 0)
			{
				output ^= 1;
				remain = count;
			}
			break;
	}
}


/*************************************
 *
 *  USB timer and envelope controls
 *
 *************************************/

void usb_sound_device::timer_w(int which, u8 offset, u8 data)
{
	timer8253 *g = &m_timer_group[which];
	timer8253::channel *ch;
	int was_holding;

	m_stream->update();

	/* switch off the offset */
	switch (offset)
	{
		case 0:
		case 1:
		case 2:
			ch = &g->chan[offset];
			was_holding = ch->holding;

			/* based on the latching mode */
			switch (ch->latchmode)
			{
				case 1: /* low word only */
					ch->count = data;
					ch->holding = false;
					break;

				case 2: /* high word only */
					ch->count = data << 8;
					ch->holding = false;
					break;

				case 3: /* low word followed by high word */
					if (ch->latchtoggle == 0)
					{
						ch->count = (ch->count & 0xff00) | (data & 0x00ff);
						ch->latchtoggle = 1;
					}
					else
					{
						ch->count = (ch->count & 0x00ff) | (data << 8);
						ch->holding = false;
						ch->latchtoggle = 0;
					}
					break;
			}

			/* if we're not holding, load the initial count for some modes */
			if (was_holding && !ch->holding)
				ch->remain = 1;
			break;

		case 3:
			/* break out the components */
			if (((data & 0xc0) >> 6) < 3)
			{
				ch = &g->chan[(data & 0xc0) >> 6];

				/* extract the bits */
				ch->holding = true;
				ch->latchmode = (data >> 4) & 3;
				ch->clockmode = (data >> 1) & 7;
				ch->bcdmode = (data >> 0) & 1;
				ch->latchtoggle = 0;
				ch->output = (ch->clockmode == 1);
			}
			break;
	}
}


void usb_sound_device::env_w(int which, u8 offset, u8 data)
{
	timer8253 *g = &m_timer_group[which];

	m_stream->update();

	if (offset < 3)
		g->env[offset] = (double)data;
	else
		g->config = data & 1;
}



/*************************************
 *
 *  USB work RAM access
 *
 *************************************/

READ8_MEMBER( usb_sound_device::workram_r )
{
	offset += 256 * m_work_ram_bank;
	return m_work_ram[offset];
}


WRITE8_MEMBER( usb_sound_device::workram_w )
{
	offset += 256 * m_work_ram_bank;
	m_work_ram[offset] = data;

	/* writes to the low 32 bytes go to various controls */
	switch (offset & ~3)
	{
		case 0x00:  /* CTC0 */
			timer_w(0, offset & 3, data);
			break;

		case 0x04:  /* ENV0 */
			env_w(0, offset & 3, data);
			break;

		case 0x08:  /* CTC1 */
			timer_w(1, offset & 3, data);
			break;

		case 0x0c:  /* ENV1 */
			env_w(1, offset & 3, data);
			break;

		case 0x10:  /* CTC2 */
			timer_w(2, offset & 3, data);
			break;

		case 0x14:  /* ENV2 */
			env_w(2, offset & 3, data);
			break;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void usb_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *dest = outputs[0];

	/* iterate over samples */
	while (samples--)
	{
		double noiseval;
		double sample = 0;
		int group;
		int step;


		/*----------------
		    Noise Source
		  ----------------

		                 RC
		   MM5837 ---> FILTER ---> CR FILTER ---> 3.2x AMP ---> NOISE
		               LADDER
		*/

		/* update the noise source */
		for (step = USB_2MHZ_CLOCK / SAMPLE_RATE; step >= m_noise_subcount; step -= m_noise_subcount)
		{
			m_noise_shift = (m_noise_shift << 1) | (((m_noise_shift >> 13) ^ (m_noise_shift >> 16)) & 1);
			m_noise_state = (m_noise_shift >> 16) & 1;
			m_noise_subcount = USB_2MHZ_CLOCK / MM5837_CLOCK;
		}
		m_noise_subcount -= step;

		/* update the filtered noise value -- this is just an approximation to the pink noise filter */
		/* being applied on the PCB, but it sounds pretty close */
		m_noise_filters[0].capval = 0.99765 * m_noise_filters[0].capval + m_noise_state * 0.0990460;
		m_noise_filters[1].capval = 0.96300 * m_noise_filters[1].capval + m_noise_state * 0.2965164;
		m_noise_filters[2].capval = 0.57000 * m_noise_filters[2].capval + m_noise_state * 1.0526913;
		noiseval = m_noise_filters[0].capval + m_noise_filters[1].capval + m_noise_filters[2].capval + m_noise_state * 0.1848;

		/* final output goes through a CR filter; the scaling factor is arbitrary to get the noise to the */
		/* correct relative volume */
		noiseval = m_noise_filters[4].step_cr(noiseval);
		noiseval *= 0.075;

		/* there are 3 identical groups of circuits, each with its own 8253 */
		for (group = 0; group < 3; group++)
		{
			timer8253 *g = &m_timer_group[group];
			double chan0, chan1, chan2, mix;


			/*-------------
			    Channel 0
			  -------------

			    8253        CR                   AD7524
			    OUT0 ---> FILTER ---> BUFFER--->  VRef  ---> 100k ---> mix
			*/

			/* channel 0 clocks with the PCS clock */
			for (step = USB_2MHZ_CLOCK / SAMPLE_RATE; step >= g->chan[0].subcount; step -= g->chan[0].subcount)
			{
				g->chan[0].subcount = USB_2MHZ_CLOCK / USB_PCS_CLOCK;
				g->chan[0].gate = 1;
				g->chan[0].clock();
			}
			g->chan[0].subcount -= step;

			/* channel 0 is mixed in with a resistance of 100k */
			chan0 = g->chan_filter[0].step_cr(g->chan[0].output) * g->env[0] * (1.0/100.0);


			/*-------------
			    Channel 1
			  -------------

			    8253        CR                   AD7524
			    OUT1 ---> FILTER ---> BUFFER--->  VRef  ---> 100k ---> mix
			*/

			/* channel 1 clocks with the PCS clock */
			for (step = USB_2MHZ_CLOCK / SAMPLE_RATE; step >= g->chan[1].subcount; step -= g->chan[1].subcount)
			{
				g->chan[1].subcount = USB_2MHZ_CLOCK / USB_PCS_CLOCK;
				g->chan[1].gate = 1;
				g->chan[1].clock();
			}
			g->chan[1].subcount -= step;

			/* channel 1 is mixed in with a resistance of 100k */
			chan1 = g->chan_filter[1].step_cr(g->chan[1].output) * g->env[1] * (1.0/100.0);


			/*-------------
			    Channel 2
			  -------------

			  If timer_mode == 0:

			               SWITCHED                                  AD7524
			    NOISE --->    RC   ---> 1.56x AMP ---> INVERTER --->  VRef ---> 33k ---> mix
			                FILTERS

			  If timer mode == 1:

			                             AD7524                                    SWITCHED
			    NOISE ---> INVERTER --->  VRef ---> 33k ---> mix ---> INVERTER --->   RC   ---> 1.56x AMP ---> finalmix
			                                                                        FILTERS
			*/

			/* channel 2 clocks with the 2MHZ clock and triggers with the GOS clock */
			for (step = 0; step < USB_2MHZ_CLOCK / SAMPLE_RATE; step++)
			{
				if (g->chan[2].subcount-- == 0)
				{
					g->chan[2].subcount = USB_2MHZ_CLOCK / USB_GOS_CLOCK / 2 - 1;
					g->chan[2].gate = !g->chan[2].gate;
				}
				g->chan[2].clock();
			}

			/* the exponents for the gate filters are determined by channel 2's output */
			g->gate1.exponent = m_gate_rc1_exp[g->chan[2].output];
			g->gate2.exponent = m_gate_rc2_exp[g->chan[2].output];

			/* based on the envelope mode, we do one of two things with source 2 */
			if (g->config == 0)
			{
				chan2 = g->gate2.step_rc(g->gate1.step_rc(noiseval)) * -1.56 * g->env[2] * (1.0/33.0);
				mix = chan0 + chan1 + chan2;
			}
			else
			{
				chan2 = -noiseval * g->env[2] * (1.0/33.0);
				mix = chan0 + chan1 + chan2;
				mix = g->gate2.step_rc(g->gate1.step_rc(-mix)) * 1.56;
			}

			/* accumulate the sample */
			sample += mix;
		}


		/*-------------
		    Final mix
		  -------------

		  INPUTS
		  EQUAL ---> 1.2x INVERTER ---> CR FILTER ---> out
		  WEIGHT

		*/
		*dest++ = 4000 * m_final_filter.step_cr(sample);
	}
}


/*************************************
 *
 *  USB address maps
 *
 *************************************/

void usb_sound_device::usb_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("pgmram");
}

void usb_sound_device::usb_portmap(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(usb_sound_device::workram_r), FUNC(usb_sound_device::workram_w)).share("workram");
}


//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void usb_sound_device::device_add_mconfig(machine_config &config)
{
	/* CPU for the usb board */
	I8035(config, m_ourcpu, USB_MASTER_CLOCK);     /* divide by 15 in CPU */
	m_ourcpu->set_addrmap(AS_PROGRAM, &usb_sound_device::usb_map);
	m_ourcpu->set_addrmap(AS_IO, &usb_sound_device::usb_portmap);
	m_ourcpu->p1_in_cb().set(FUNC(usb_sound_device::p1_r));
	m_ourcpu->p1_out_cb().set(FUNC(usb_sound_device::p1_w));
	m_ourcpu->p2_out_cb().set(FUNC(usb_sound_device::p2_w));
	m_ourcpu->t1_in_cb().set(FUNC(usb_sound_device::t1_r));

	TIMER(config, "usb_timer", 0).configure_periodic(
			FUNC(usb_sound_device::increment_t1_clock_timer_cb),
			attotime::from_hz(USB_2MHZ_CLOCK / 256));
}


DEFINE_DEVICE_TYPE(SEGAUSBROM, usb_rom_sound_device, "segausbrom", "Sega Universal Sound Board with ROM")

usb_rom_sound_device::usb_rom_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: usb_sound_device(mconfig, SEGAUSBROM, tag, owner, clock)
{
}

void usb_sound_device::usb_map_rom(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(":usbcpu", 0);
}

void usb_rom_sound_device::device_add_mconfig(machine_config &config)
{
	usb_sound_device::device_add_mconfig(config);

	/* CPU for the usb board */
	m_ourcpu->set_addrmap(AS_PROGRAM, &usb_rom_sound_device::usb_map_rom);
}
