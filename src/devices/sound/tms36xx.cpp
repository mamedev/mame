// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
#include "emu.h"
#include "tms36xx.h"

#define VERBOSE 1

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* the frequencies are later adjusted by "* clock / FSCALE" */
#define FSCALE  1024

#define C(n)    (int)((FSCALE<<(n-1))*1.18921)  /* 2^(3/12) */
#define Cx(n)   (int)((FSCALE<<(n-1))*1.25992)  /* 2^(4/12) */
#define D(n)    (int)((FSCALE<<(n-1))*1.33484)  /* 2^(5/12) */
#define Dx(n)   (int)((FSCALE<<(n-1))*1.41421)  /* 2^(6/12) */
#define E(n)    (int)((FSCALE<<(n-1))*1.49831)  /* 2^(7/12) */
#define F(n)    (int)((FSCALE<<(n-1))*1.58740)  /* 2^(8/12) */
#define Fx(n)   (int)((FSCALE<<(n-1))*1.68179)  /* 2^(9/12) */
#define G(n)    (int)((FSCALE<<(n-1))*1.78180)  /* 2^(10/12) */
#define Gx(n)   (int)((FSCALE<<(n-1))*1.88775)  /* 2^(11/12) */
#define A(n)    (int)((FSCALE<<n))              /* A */
#define Ax(n)   (int)((FSCALE<<n)*1.05946)      /* 2^(1/12) */
#define B(n)    (int)((FSCALE<<n)*1.12246)      /* 2^(2/12) */

/*
 * Alarm sound?
 * It is unknown what this sound is like. Until somebody manages
 * trigger sound #1 of the Phoenix PCB sound chip I put just something
 * 'alarming' in here.
 */
static const int tune1[96*6] = {
	C(3),   0,      0,      C(2),   0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      C(4),   0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      C(2),   0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      C(4),   0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
	C(3),   0,      0,      0,      0,      0,
	G(3),   0,      0,      0,      0,      0,
};

/*
 * Fuer Elise, Beethoven
 * (Excuse my non-existent musical skill, Mr. B ;-)
 */
static const int tune2[96*6] = {
	D(3),   D(4),   D(5),   0,      0,      0,
	Cx(3),  Cx(4),  Cx(5),  0,      0,      0,
	D(3),   D(4),   D(5),   0,      0,      0,
	Cx(3),  Cx(4),  Cx(5),  0,      0,      0,
	D(3),   D(4),   D(5),   0,      0,      0,
	A(2),   A(3),   A(4),   0,      0,      0,
	C(3),   C(4),   C(5),   0,      0,      0,
	Ax(2),  Ax(3),  Ax(4),  0,      0,      0,
	G(2),   G(3),   G(4),   0,      0,      0,
	D(1),   D(2),   D(3),   0,      0,      0,
	G(1),   G(2),   G(3),   0,      0,      0,
	Ax(1),  Ax(2),  Ax(3),  0,      0,      0,

	D(2),   D(3),   D(4),   0,      0,      0,
	G(2),   G(3),   G(4),   0,      0,      0,
	A(2),   A(3),   A(4),   0,      0,      0,
	D(1),   D(2),   D(3),   0,      0,      0,
	A(1),   A(2),   A(3),   0,      0,      0,
	D(2),   D(3),   D(4),   0,      0,      0,
	Fx(2),  Fx(3),  Fx(4),  0,      0,      0,
	A(2),   A(3),   A(4),   0,      0,      0,
	Ax(2),  Ax(3),  Ax(4),  0,      0,      0,
	D(1),   D(2),   D(3),   0,      0,      0,
	G(1),   G(2),   G(3),   0,      0,      0,
	Ax(1),  Ax(2),  Ax(3),  0,      0,      0,

	D(3),   D(4),   D(5),   0,      0,      0,
	Cx(3),  Cx(4),  Cx(5),  0,      0,      0,
	D(3),   D(4),   D(5),   0,      0,      0,
	Cx(3),  Cx(4),  Cx(5),  0,      0,      0,
	D(3),   D(4),   D(5),   0,      0,      0,
	A(2),   A(3),   A(4),   0,      0,      0,
	C(3),   C(4),   C(5),   0,      0,      0,
	Ax(2),  Ax(3),  Ax(4),  0,      0,      0,
	G(2),   G(3),   G(4),   0,      0,      0,
	D(1),   D(2),   D(3),   0,      0,      0,
	G(1),   G(2),   G(3),   0,      0,      0,
	Ax(1),  Ax(2),  Ax(3),  0,      0,      0,

	D(2),   D(3),   D(4),   0,      0,      0,
	G(2),   G(3),   G(4),   0,      0,      0,
	A(2),   A(3),   A(4),   0,      0,      0,
	D(1),   D(2),   D(3),   0,      0,      0,
	A(1),   A(2),   A(3),   0,      0,      0,
	D(2),   D(3),   D(4),   0,      0,      0,
	Ax(2),  Ax(3),  Ax(4),  0,      0,      0,
	A(2),   A(3),   A(4),   0,      0,      0,
	0,      0,      0,      G(2),   G(3),   G(4),
	D(1),   D(2),   D(3),   0,      0,      0,
	G(1),   G(2),   G(3),   0,      0,      0,
	0,      0,      0,      0,      0,      0
};

/*
 * The theme from Phoenix, a sad little tune.
 * Gerald Coy:
 *   The starting song from Phoenix comes from an old French movie and
 *   it's called : "Jeux interdits" which means "unallowed games"  ;-)
 * Mirko Buffoni:
 *   It's called "Sogni proibiti" in Italian, by Anonymous.
 * Magic*:
 *   This song is a classical piece called "ESTUDIO" from M.A.Robira.
 */
static const int tune3[96*6] = {
	A(2),   A(3),   A(4),   D(1),    D(2),    D(3),
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	A(2),   A(3),   A(4),   A(1),    A(2),    A(3),
	0,      0,      0,      0,       0,       0,
	G(2),   G(3),   G(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	F(2),   F(3),   F(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	F(2),   F(3),   F(4),   F(1),    F(2),    F(3),
	0,      0,      0,      0,       0,       0,
	E(2),   E(3),   E(4),   F(1),    F(2),    F(3),
	0,      0,      0,      0,       0,       0,
	D(2),   D(3),   D(4),   F(1),    F(2),    F(3),
	0,      0,      0,      0,       0,       0,

	D(2),   D(3),   D(4),   A(1),    A(2),    A(3),
	0,      0,      0,      0,       0,       0,
	F(2),   F(3),   F(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	D(3),   D(4),   D(5),   D(1),    D(2),    D(3),
	0,      0,      0,      0,       0,       0,
	0,      0,      0,      D(1),    D(2),    D(3),
	0,      0,      0,      F(1),    F(2),    F(3),
	0,      0,      0,      A(1),    A(2),    A(3),
	0,      0,      0,      D(2),    D(2),    D(2),

	D(3),   D(4),   D(5),   D(1),    D(2),    D(3),
	0,      0,      0,      0,       0,       0,
	C(3),   C(4),   C(5),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	Ax(2),  Ax(3),  Ax(4),  0,       0,       0,
	0,      0,      0,      0,       0,       0,

	Ax(2),  Ax(3),  Ax(4),  Ax(1),   Ax(2),   Ax(3),
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	G(2),   G(3),   G(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	G(2),   G(3),   G(4),   G(1),    G(2),    G(3),
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	Ax(2),  Ax(3),  Ax(4),  0,       0,       0,
	0,      0,      0,      0,       0,       0,

	A(2),   A(3),   A(4),   A(1),    A(2),    A(3),
	0,      0,      0,      0,       0,       0,
	Ax(2),  Ax(3),  Ax(4),  0,       0,       0,
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	Cx(3),  Cx(4),  Cx(5),  A(1),    A(2),    A(3),
	0,      0,      0,      0,       0,       0,
	Ax(2),  Ax(3),  Ax(4),  0,       0,       0,
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	A(2),   A(3),   A(4),   F(1),    F(2),    F(3),
	0,      0,      0,      0,       0,       0,
	G(2),   G(3),   G(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	F(2),   F(3),   F(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	F(2),   F(3),   F(4),   D(1),    D(2),    D(3),
	0,      0,      0,      0,       0,       0,
	E(2),   E(3),   E(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	D(2),   D(3),   D(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	E(2),   E(3),   E(4),   E(1),    E(2),    E(3),
	0,      0,      0,      0,       0,       0,
	E(2),   E(3),   E(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	E(2),   E(3),   E(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,

	E(2),   E(3),   E(4),   Ax(1),   Ax(2),   Ax(3),
	0,      0,      0,      0,       0,       0,
	F(2),   F(3),   F(4),   0,       0,       0,
	0,      0,      0,      0,       0,       0,
	E(2),   E(3),   E(4),   F(1),    F(2),    F(3),
	0,      0,      0,      0,       0,       0,

	D(2),   D(3),   D(4),   D(1),    D(2),    D(3),
	0,      0,      0,      0,       0,       0,
	F(2),   F(3),   F(4),   A(1),    A(2),    A(3),
	0,      0,      0,      0,       0,       0,
	A(2),   A(3),   A(4),   F(1),    F(2),    F(3),
	0,      0,      0,      0,       0,       0,

	D(3),   D(4),   D(5),   D(1),    D(2),    D(3),
	0,      0,      0,      0,       0,       0,
	0,      0,      0,      0,       0,       0,
	0,      0,      0,      0,       0,       0,
	0,      0,      0,      0,       0,       0,
	0,      0,      0,      0,       0,       0
};

/* This is used to play single notes for the TMS3615/TMS3617 */
static const int tune4[13*6] = {
/*  16'     8'      5 1/3'  4'      2 2/3'  2'      */
	B(0),   B(1),   Dx(2),  B(2),   Dx(3),  B(3),
	C(1),   C(2),   E(2),   C(3),   E(3),   C(4),
	Cx(1),  Cx(2),  F(2),   Cx(3),  F(3),   Cx(4),
	D(1),   D(2),   Fx(2),  D(3),   Fx(3),  D(4),
	Dx(1),  Dx(2),  G(2),   Dx(3),  G(3),   Dx(4),
	E(1),   E(2),   Gx(2),  E(3),   Gx(3),  E(4),
	F(1),   F(2),   A(2),   F(3),   A(3),   F(4),
	Fx(1),  Fx(2),  Ax(2),  Fx(3),  Ax(3),  Fx(4),
	G(1),   G(2),   B(2),   G(3),   B(3),   G(4),
	Gx(1),  Gx(2),  C(3),   Gx(3),  C(4),   Gx(4),
	A(1),   A(2),   Cx(3),  A(3),   Cx(4),  A(4),
	Ax(1),  Ax(2),  D(3),   Ax(3),  D(4),   Ax(4),
	B(1),   B(2),   Dx(3),  B(3),   Dx(4),  B(4)
};

static const int *const tunes[] = {nullptr,tune1,tune2,tune3,tune4};

#define DECAY(voice)                                            \
	if( m_vol[voice] > TMS36XX_VMIN )                                   \
	{                                                           \
		/* decay of first voice */                              \
		m_vol_counter[voice] -= m_decay[voice];                 \
		while( m_vol_counter[voice] <= 0 )                      \
		{                                                       \
			m_vol_counter[voice] += samplerate;                 \
			if( m_vol[voice]-- <= TMS36XX_VMIN )                        \
			{                                                   \
				m_frequency[voice] = 0;                         \
				m_vol[voice] = TMS36XX_VMIN;                            \
				break;                                          \
			}                                                   \
		}                                                       \
	}

#define RESTART(voice)                                          \
	if( tunes[m_tune_num][m_tune_ofs*6+voice] )                 \
	{                                                           \
		m_frequency[m_shift+voice] =                            \
			tunes[m_tune_num][m_tune_ofs*6+voice] *             \
			(m_basefreq << m_octave) / FSCALE;                  \
		m_vol[m_shift+voice] = TMS36XX_VMAX;                            \
	}

#define TONE(voice)                                             \
	if( (m_enable & (1<<voice)) && m_frequency[voice] )         \
	{                                                           \
		/* first note */                                        \
		m_counter[voice] -= m_frequency[voice];                 \
		while( m_counter[voice] <= 0 )                          \
		{                                                       \
			m_counter[voice] += samplerate;                     \
			m_output ^= 1 << voice;                             \
		}                                                       \
		if (m_output & m_enable & (1 << voice))                 \
			sum += m_vol[voice];                                \
	}



// device type definition
const device_type TMS36XX = &device_creator<tms36xx_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tms36xx_device - constructor
//-------------------------------------------------

tms36xx_device::tms36xx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TMS36XX, "TMS36XX", tag, owner, clock, "tms36xx", __FILE__),
		device_sound_interface(mconfig, *this),
		m_subtype(nullptr),
		m_channel(nullptr),
		m_samplerate(0),
		m_basefreq(0),
		m_octave(0),
		m_speed(0),
		m_tune_counter(0),
		m_note_counter(0),
		m_voices(0),
		m_shift(0),
		m_output(0),
		m_enable(0),
		m_tune_num(0),
		m_tune_ofs(0),
		m_tune_max(0)
{
	memset(m_vol, 0, sizeof(int)*12);
	memset(m_vol_counter, 0, sizeof(int)*12);
	memset(m_decay, 0, sizeof(int)*12);
	memset(m_counter, 0, sizeof(int)*12);
	memset(m_frequency, 0, sizeof(int)*12);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms36xx_device::device_start()
{
	int enable = 0;

	m_channel = stream_alloc(0, 1, clock() * 64);
	m_samplerate = clock() * 64;
	m_basefreq = clock();

	for (int j = 0; j < 6; j++)
	{
		if (m_decay_time[j] > 0)
		{
			m_decay[j+0] = m_decay[j+6] = TMS36XX_VMAX / m_decay_time[j];
			enable |= 0x41 << j;
		}
	}
	tms3617_enable(enable);

	LOG(("TMS36xx samplerate    %d\n", m_samplerate));
	LOG(("TMS36xx basefreq      %d\n", m_basefreq));
	LOG(("TMS36xx decay         %d,%d,%d,%d,%d,%d\n",
		m_decay[0], m_decay[1], m_decay[2],
		m_decay[3], m_decay[4], m_decay[5]));
	LOG(("TMS36xx speed         %d\n", m_speed));

	save_item(NAME(m_octave));
	save_item(NAME(m_tune_counter));
	save_item(NAME(m_note_counter));
	save_item(NAME(m_voices));
	save_item(NAME(m_shift));
	save_item(NAME(m_vol));
	save_item(NAME(m_vol_counter));
	save_item(NAME(m_counter));
	save_item(NAME(m_frequency));
	save_item(NAME(m_output));
	save_item(NAME(m_enable));
	save_item(NAME(m_tune_num));
	save_item(NAME(m_tune_ofs));
	save_item(NAME(m_tune_max));
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void tms36xx_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	int samplerate = m_samplerate;
	stream_sample_t *buffer = outputs[0];

	/* no tune played? */
	if( !tunes[m_tune_num] || m_voices == 0 )
	{
		while (--samples >= 0)
			buffer[samples] = 0;
		return;
	}

	while( samples-- > 0 )
	{
		int sum = 0;

		/* decay the twelve voices */
		DECAY( 0) DECAY( 1) DECAY( 2) DECAY( 3) DECAY( 4) DECAY( 5)
		DECAY( 6) DECAY( 7) DECAY( 8) DECAY( 9) DECAY(10) DECAY(11)

		/* musical note timing */
		m_tune_counter -= m_speed;
		if( m_tune_counter <= 0 )
		{
			int n = (-m_tune_counter / samplerate) + 1;
			m_tune_counter += n * samplerate;

			if( (m_note_counter -= n) <= 0 )
			{
				m_note_counter += TMS36XX_VMAX;
				if (m_tune_ofs < m_tune_max)
				{
					/* shift to the other 'bank' of voices */
					m_shift ^= 6;
					/* restart one 'bank' of voices */
					RESTART(0) RESTART(1) RESTART(2)
					RESTART(3) RESTART(4) RESTART(5)
					m_tune_ofs++;
				}
			}
		}

		/* update the twelve voices */
		TONE( 0) TONE( 1) TONE( 2) TONE( 3) TONE( 4) TONE( 5)
		TONE( 6) TONE( 7) TONE( 8) TONE( 9) TONE(10) TONE(11)

		*buffer++ = sum / m_voices;
	}
}


//-------------------------------------------------
//  MM6221AA interface functions
//-------------------------------------------------

void tms36xx_device::mm6221aa_tune_w(int tune)
{
	/* which tune? */
	tune &= 3;
	if( tune == m_tune_num )
		return;

	LOG(("%s tune:%X\n", m_subtype, tune));

	/* update the stream before changing the tune */
	m_channel->update();

	m_tune_num = tune;
	m_tune_ofs = 0;
	m_tune_max = 96; /* fixed for now */
}


//-------------------------------------------------
//  TMS3615/17 interface functions
//-------------------------------------------------

void tms36xx_device::tms36xx_note_w(int octave, int note)
{
	octave &= 3;
	note &= 15;

	if (note > 12)
		return;

	LOG(("%s octave:%X note:%X\n", m_subtype, octave, note));

	/* update the stream before changing the tune */
	m_channel->update();

	/* play a single note from 'tune 4', a list of the 13 tones */
	tms36xx_reset_counters();
	m_octave = octave;
	m_tune_num = 4;
	m_tune_ofs = note;
	m_tune_max = note + 1;
}


//-------------------------------------------------
//  TMS3617 interface functions
//-------------------------------------------------

void tms36xx_device::tms3617_enable_w(int enable)
{
	tms3617_enable(enable);
}


//-------------------------------------------------
//  Locals
//-------------------------------------------------

void tms36xx_device::tms36xx_reset_counters()
{
	m_tune_counter = 0;
	m_note_counter = 0;
	memset(m_vol_counter, 0, sizeof(m_vol_counter));
	memset(m_counter, 0, sizeof(m_counter));
}


void tms36xx_device::tms3617_enable(int enable)
{
	int i, bits = 0;

	/* duplicate the 6 voice enable bits */
	enable = (enable & 0x3f) | ((enable & 0x3f) << 6);
	if (enable == m_enable)
		return;

	/* update the stream before changing the tune */
	m_channel->update();

	LOG(("%s enable voices", m_subtype));
	for (i = 0; i < 6; i++)
	{
		if (enable & (1 << i))
		{
			bits += 2;  /* each voice has two instances */

			switch (i)
			{
			case 0: LOG((" 16'")); break;
			case 1: LOG((" 8'")); break;
			case 2: LOG((" 5 1/3'")); break;
			case 3: LOG((" 4'")); break;
			case 4: LOG((" 2 2/3'")); break;
			case 5: LOG((" 2'")); break;
			}
		}
	}
	/* set the enable mask and number of active voices */
	m_enable = enable;
	m_voices = bits;
	LOG(("%s\n", bits ? "" : " none"));
}
