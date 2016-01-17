// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Tatsumi TX-1/Buggy Boy sound hardware

***************************************************************************/

#include "emu.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "includes/tx1.h"


/*************************************
 *
 *  TX-1
 *
 *************************************/

/* RC oscillator: 1785Hz */
#define TX1_NOISE_CLOCK     (1/(1000.0e-12 * 560000.0))
#define TX1_PIT_CLOCK       (TX1_PIXEL_CLOCK / 16)
#define TX1_FRAC            30

#define TX1_SHUNT           (250.0)
#define TX1_R0              (180000.0 + TX1_SHUNT)
#define TX1_R1              (56000.0  + TX1_SHUNT)
#define TX1_R2              (22000.0  + TX1_SHUNT)
#define TX1_R               (100000.0 + TX1_SHUNT)
#define TX1_RI              (180000.0)

static const double tx1_engine_gains[16] =
{
	-( TX1_R )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2 + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_R2 + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT ) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2 + 1.0/TX1_R0) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2 + 1.0/TX1_R1) )/TX1_RI,
	-( 1.0/(1.0/TX1_R + 1.0/TX1_SHUNT + 1.0/TX1_R2 + 1.0/TX1_R1 + 1.0/TX1_R0) )/TX1_RI
};


const device_type TX1 = &device_creator<tx1_sound_device>;

tx1_sound_device::tx1_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TX1, "TX-1 Audio Custom", tag, owner, clock, "tx1_sound", __FILE__),
		device_sound_interface(mconfig, *this)
{
}

tx1_sound_device::tx1_sound_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_freq_to_step(0),
		m_step0(0),
		m_step1(0),
		m_step2(0),
		m_ay_outputa(0),
		m_ay_outputb(0),
		m_pit0(0),
		m_pit1(0),
		m_pit2(0),
		m_noise_lfsra(0),
		m_noise_lfsrb(1),
		m_noise_lfsrc(0),
		m_noise_lfsrd(0),
		m_noise_counter(0),
		m_ym1_outputa(0),
		m_ym2_outputa(0),
		m_ym2_outputb(0)
{
}
//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tx1_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tx1_sound_device::device_start()
{
	static const int r0[4] = { static_cast<int>(390e3), static_cast<int>(180e3), static_cast<int>(180e3), static_cast<int>(180e3) };
	static const int r1[3] = { static_cast<int>(180e3), static_cast<int>(390e3), static_cast<int>(56e3) };
	static const int r2[3] = { static_cast<int>(390e3), static_cast<int>(390e3), static_cast<int>(180e3) };


	/* Allocate the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate());
	m_freq_to_step = (double)(1 << TX1_FRAC) / (double)machine().sample_rate();

	/* Compute the engine resistor weights */
	compute_resistor_weights(0, 10000, -1.0,
			4, &r0[0], m_weights0, 0, 0,
			3, &r1[0], m_weights1, 0, 0,
			3, &r2[0], m_weights2, 0, 0);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tx1_sound_device::device_reset()
{
	m_step0 = m_step1 = m_step2 = 0;
}


WRITE8_MEMBER( tx1_sound_device::pit8253_w )
{
	m_stream->update();

	if (offset < 3)
	{
		if (m_pit8253.idx[offset] == 0)
		{
			m_pit8253.counts[offset].as8bit.LSB = data;
			m_pit8253.idx[offset] = 1;
		}
		else
		{
			m_pit8253.counts[offset].as8bit.MSB = data;
			m_pit8253.idx[offset] = 0;
		}
	}
	else
	{
		int mode = (data >> 1) & 7;

		if (mode == 3)
		{
			int cntsel = (data >> 6) & 3;
			m_pit8253.idx[cntsel] = 0;
			m_pit8253.counts[cntsel].val = 0;
		}
		else
			osd_printf_debug("PIT8253: Unsupported mode %d.\n", mode);
	}
}

READ8_MEMBER( tx1_sound_device::pit8253_r )
{
	osd_printf_debug("PIT R: %x", offset);
	return 0;
}

/***************************************************************************

    AY-8910 port mappings:

    Port A                      Port B
    =======                     ======

    0: Engine 0 gain #0         0: Engine 0 gain #8
    1: Engine 0 gain #1         1: Engine 0 gain #9
    2: Engine 0 gain #2         2: Engine 0 gain #10
    3: Engine 0 gain #3         3: Engine 0 gain #11
    4: Engine 0 gain #4         4: /Enable AY on speaker CR
    5: Engine 0 gain #5         5: /Enable Engines 1/2 on speakers LR/RR/CF
    6: Engine 0 gain #6         6: /Skid 0 enable
    7: Engine 0 gain #7         7: /Skid 1 enable

***************************************************************************/

WRITE8_MEMBER( tx1_sound_device::ay8910_a_w )
{
	m_stream->update();

	/* All outputs inverted */
	m_ay_outputa = ~data;
}

WRITE8_MEMBER( tx1_sound_device::ay8910_b_w )
{
	double gain;

	m_stream->update();
	/* Only B3-0 are inverted */
	m_ay_outputb = data ^ 0xf;

	/* It'll do until we get quadrophonic speaker support! */
	gain = BIT(m_ay_outputb, 4) ? 1.5 : 2.0;
	device_sound_interface *sound;
	interface(sound);
	sound->set_output_gain(0, gain);
	sound->set_output_gain(1, gain);
	sound->set_output_gain(2, gain);
}

/***************************************************************************

    Engine sounds are produced by three of these 4013B chains:

                +------------------
                |  +---------+
        +-----+ |  | +-----+ |
      +-|D 1 Q|-+  +-|D 2 Q|-|-----
      | |C  /Q|-o----|C  /Q|-+
      | +-----+ |    +-----+
      +---------+

     +----------+
     |  +-----+ |    +-----+
     |  |C 3 Q|-+    |C 4 Q|-------
    !&--|D  /Q|------|D  /Q|-+
     |  +-----+      +-----+ |
     +-----------------------+

     Common clocks omitted for clarity (all driven from an 8253 output pin).

         Player: ES0, ES1, ES2
     Opponent 2: ES2

 ***************************************************************************/

static inline void update_engine(int eng[4])
{
	int p0 = eng[0];
	int p1 = eng[1];
	int p2 = eng[2];
	int p3 = eng[3];

	eng[0] = !p0;
	if (p0 && !eng[0]) eng[1] = !p1;
	eng[2] = !(p2 && !p3);
	eng[3] = !p2;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tx1_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	UINT32 step_0, step_1, step_2;
	double /*gain_0, gain_1,*/ gain_2, gain_3;

	stream_sample_t *fl = &outputs[0][0];
	stream_sample_t *fr = &outputs[1][0];

	/* Clear the buffers */
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	/* 8253 outputs for the player/opponent engine sounds. */
	step_0 = m_pit8253.counts[0].val ? (TX1_PIT_CLOCK / m_pit8253.counts[0].val) * m_freq_to_step : 0;
	step_1 = m_pit8253.counts[1].val ? (TX1_PIT_CLOCK / m_pit8253.counts[1].val) * m_freq_to_step : 0;
	step_2 = m_pit8253.counts[2].val ? (TX1_PIT_CLOCK / m_pit8253.counts[2].val) * m_freq_to_step : 0;

	//gain_0 = tx1_engine_gains[m_ay_outputa & 0xf];
	//gain_1 = tx1_engine_gains[m_ay_outputa >> 4];
	gain_2 = tx1_engine_gains[m_ay_outputb & 0xf];
	gain_3 = BIT(m_ay_outputb, 5) ? 1.0f : 1.5f;

	while (samples--)
	{
		if (m_step0 & ((1 << TX1_FRAC)))
		{
			update_engine(m_eng0);
			m_pit0 = combine_4_weights(m_weights0, m_eng0[0], m_eng0[1], m_eng0[2], m_eng0[3]);
			m_step0 &= ((1 << TX1_FRAC) - 1);
		}

		if (m_step1 & ((1 << TX1_FRAC)))
		{
			update_engine(m_eng1);
			m_pit1 = combine_3_weights(m_weights1, m_eng1[0], m_eng1[1], m_eng1[3]);
			m_step1 &= ((1 << TX1_FRAC) - 1);
		}

		if (m_step2 & ((1 << TX1_FRAC)))
		{
			update_engine(m_eng2);
			m_pit2 = combine_3_weights(m_weights2, m_eng2[0], m_eng2[1], m_eng2[3]);
			m_step2 &= ((1 << TX1_FRAC) - 1);
		}

		*fl++ = (m_pit0 + m_pit1)*gain_3 + 2*m_pit2*gain_2;
		*fr++ = (m_pit0 + m_pit1)*gain_3 + 2*m_pit2*gain_2;

		m_step0 += step_0;
		m_step1 += step_1;
		m_step2 += step_2;
	}
}


/*************************************
 *
 *  Buggy Boy
 *
 *************************************/

#define BUGGYBOY_PIT_CLOCK      (BUGGYBOY_ZCLK / 8)
#define BUGGYBOY_NOISE_CLOCK    (BUGGYBOY_PIT_CLOCK / 4)

#define BUGGYBOY_R1     47000.0
#define BUGGYBOY_R2     22000.0
#define BUGGYBOY_R3     10000.0
#define BUGGYBOY_R4     5600.0
#define BUGGYBOY_SHUNT  250.0

#define BUGGYBOY_R1S    (1.0/(1.0/BUGGYBOY_R1 + 1.0/BUGGYBOY_SHUNT))
#define BUGGYBOY_R2S    (1.0/(1.0/BUGGYBOY_R2 + 1.0/BUGGYBOY_SHUNT))
#define BUGGYBOY_R3S    (1.0/(1.0/BUGGYBOY_R3 + 1.0/BUGGYBOY_SHUNT))
#define BUGGYBOY_R4S    (1.0/(1.0/BUGGYBOY_R4 + 1.0/BUGGYBOY_SHUNT))

static const double bb_engine_gains[16] =
{
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1  + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2  + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3  + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4 ) + 1.0/100e3)/100e3,
	-1.0/(1.0/(BUGGYBOY_R1S + BUGGYBOY_R2S + BUGGYBOY_R3S + BUGGYBOY_R4S) + 1.0/100e3)/100e3,
};

const device_type BUGGYBOY = &device_creator<buggyboy_sound_device>;

buggyboy_sound_device::buggyboy_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: tx1_sound_device(mconfig, BUGGYBOY, "Buggy Boy Audio Custom", tag, owner, clock, "buggyboy_sound", __FILE__)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void buggyboy_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void buggyboy_sound_device::device_start()
{
	static const int resistors[4] = { 330000, 220000, 330000, 220000 };
	double aweights[4];
	int i;
	static const int tmp[16] =
	{
		0x0, 0x1, 0xe, 0xf, 0x8, 0x9, 0x6, 0x7, 0xc, 0xd, 0xe, 0xf, 0x4, 0x5, 0x6, 0x7
	};

	compute_resistor_weights(0, 16384,  -1.0,
							4,  &resistors[0], aweights, 0, 0,
							0, nullptr, nullptr, 0, 0,
							0, nullptr, nullptr, 0, 0 );

	for (i = 0; i < 16; i++)
		m_eng_voltages[i] = combine_4_weights(aweights, BIT(tmp[i], 0), BIT(tmp[i], 1), BIT(tmp[i], 2), BIT(tmp[i], 3));

	/* Allocate the stream */
	m_stream = machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate());
	m_freq_to_step = (double)(1 << 24) / (double)machine().sample_rate();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void buggyboy_sound_device::device_reset()
{
	m_step0 = m_step1 = 0;

	/* Reset noise LFSR */
	m_noise_lfsra = 0;
	m_noise_lfsrb = 1;
	m_noise_lfsrc = 0;
	m_noise_lfsrd = 0;
}

/***************************************************************************

    YM-2149, IC24 port mappings:

    Port A                      Port B
    =======                     ======

    0: Engine 1 gain (FR) #0    0: *Coin Counter 1
    1: Engine 1 gain (FR) #1    1: *Coin Counter 2
    2: Engine 1 gain (FR) #2    2: *Coin Counter 3 (Unused)
    3: Engine 1 gain (FR) #3    3: *Engine 0 gain
    4: Engine 1 gain (FL) #0    4: Skid 0 enable
    5: Engine 1 gain (FL) #1    5: Skid 1 enable
    6: Engine 1 gain (FL) #2    6: Enable YM IC24 output on RR
    7: Engine 1 gain (FL) #3    7: Enable YM IC19 output on RL

    (* Buggy Boy Junior only)

    The engine sounds are generated by an 8253. There are two channels.

    #0 is the player's buggy
    #1 is the opponents' buggies

              +------------> GAIN[1] +--> FL
              |                      +--> FR
    8255 #0 --+--> BL
              +--> BR

    8255 #1 --+--> GAIN[2] ---> FL
              +--> GAIN[3] ---> FR


    [1] is used to amplify sound during tunnel.
    [2] and [3] are stereo fades

***************************************************************************/

WRITE8_MEMBER( buggyboy_sound_device::ym1_a_w )
{
	m_stream->update();
	m_ym1_outputa = data ^ 0xff;
}

WRITE8_MEMBER( buggyboy_sound_device::ym2_a_w )
{
	m_stream->update();
	m_ym2_outputa = data ^ 0xff;
}

WRITE8_MEMBER( buggyboy_sound_device::ym2_b_w )
{
	device_t *ym1 = space.machine().device("ym1");
	device_t *ym2 = space.machine().device("ym2");
	double gain;

	m_stream->update();

	m_ym2_outputb = data ^ 0xff;

	if (!strcmp(space.machine().system().name, "buggyboyjr"))
	{
		space.machine().bookkeeping().coin_counter_w(0, data & 0x01);
		space.machine().bookkeeping().coin_counter_w(1, data & 0x02);
	}

	/*
	    Until we support > 2 speakers, double the gain of the front speakers

	    TODO: We do support more than 2 speakers but the output is downmixed to stereo.
	*/

	/* Rear left speaker */
	device_sound_interface *sound;
	ym1->interface(sound);
	gain = data & 0x80 ? 1.0 : 2.0;
	sound->set_output_gain(0, gain);
	sound->set_output_gain(1, gain);
	sound->set_output_gain(2, gain);

	/* Rear right speaker */
	ym2->interface(sound);
	gain = data & 0x40 ? 1.0 : 2.0;
	sound->set_output_gain(0, gain);
	sound->set_output_gain(1, gain);
	sound->set_output_gain(2, gain);
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void buggyboy_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	/* This is admittedly a bit of a hack job... */

	UINT32 step_0, step_1;
	int n1_en, n2_en;
	double gain0, gain1_l, gain1_r;

	stream_sample_t *fl = &outputs[0][0];
	stream_sample_t *fr = &outputs[1][0];

	/* Clear the buffers */
	memset(outputs[0], 0, samples * sizeof(*outputs[0]));
	memset(outputs[1], 0, samples * sizeof(*outputs[1]));

	/* 8253 outputs for the player/opponent buggy engine sounds. */
	step_0 = m_pit8253.counts[0].val ? (BUGGYBOY_PIT_CLOCK / m_pit8253.counts[0].val) * m_freq_to_step : 0;
	step_1 = m_pit8253.counts[1].val ? (BUGGYBOY_PIT_CLOCK / m_pit8253.counts[1].val) * m_freq_to_step : 0;

	if (!strcmp(machine().system().name, "buggyboyjr"))
		gain0 = BIT(m_ym2_outputb, 3) ? 1.0 : 2.0;
	else
		gain0 = BIT(m_ym1_outputa, 3) ? 1.0 : 2.0;

	n1_en = BIT(m_ym2_outputb, 4);
	n2_en = BIT(m_ym2_outputb, 5);

	gain1_l = bb_engine_gains[m_ym2_outputa >> 4] * 5;
	gain1_r = bb_engine_gains[m_ym2_outputa & 0xf] * 5;

	while (samples--)
	{
		int i;
		stream_sample_t pit0, pit1, n1, n2;
		pit0 = m_eng_voltages[(m_step0 >> 24) & 0xf];
		pit1 = m_eng_voltages[(m_step1 >> 24) & 0xf];

		/* Calculate the tyre screech noise source */
		for (i = 0; i < BUGGYBOY_NOISE_CLOCK / machine().sample_rate(); ++i)
		{
			/* CD4006 is a 4-4-1-4-4-1 shift register */
			int p13 = BIT(m_noise_lfsra, 3);
			int p12 = BIT(m_noise_lfsrb, 4);
			int p10 = BIT(m_noise_lfsrc, 3);
			int p8 = BIT(m_noise_lfsrd, 3);

			/* Update the register */
			m_noise_lfsra = p12 | ((m_noise_lfsra << 1) & 0xf);
			m_noise_lfsrb = (p8 ^ p12) | ((m_noise_lfsrb << 1) & 0x1f);
			m_noise_lfsrc = p13 | ((m_noise_lfsrc << 1) & 0xf);
			m_noise_lfsrd = p10 | ((m_noise_lfsrd << 1) & 0x1f);

			/* 4040 12-bit counter is clocked on the falling edge of Q13 */
			if ( !BIT(m_noise_lfsrc, 3) && p10 )
				m_noise_counter = (m_noise_counter + 1) & 0x0fff;
		}

		if (n1_en)
		{
			n1 = !BIT(m_noise_counter, 7-1) * 16000;
			if ( BIT(m_noise_counter, 11-1) ) n1 /=2;
		}
		else
			n1 = 8192;

		if (n2_en)
		{
			n2 = !BIT(m_noise_counter, 6-1) * 16000;
			if ( BIT(m_noise_counter, 11-1) ) n2 /=2;
		}
		else
			n2 = 8192;

		*fl++ = n1 + n2 + (pit0 * gain0) + (pit1 * gain1_l);
		*fr++ = n1 + n2 + (pit0 * gain0) + (pit1 * gain1_r);

		m_step0 += step_0;
		m_step1 += step_1;
	}
}
