// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/****************************************************************************
 *
 * Sound hardware for Pleiades, Naughty Boy and Pop Flamer.
 *
 * If you find errors or have suggestions, please mail me.
 * Juergen Buchmueller <pullmoll@t-online.de>
 *
 ****************************************************************************/
#include "emu.h"
#include "pleiads.h"

//#define VERBOSE 1
#include "logmacro.h"

#define VMIN    0
#define VMAX    32767

/* fixed 8kHz clock */
#define TONE1_CLOCK  8000


DEFINE_DEVICE_TYPE(PLEIADS_SOUND, pleiads_sound_device, "pleiads_sound", "Pleiads Custom Sound")

pleiads_sound_device::pleiads_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pleiads_sound_device(mconfig, PLEIADS_SOUND, tag, owner, clock)
{
}

pleiads_sound_device::pleiads_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_tms(*this, ":tms"),
		m_channel(nullptr),
		m_sound_latch_a(0),
		m_sound_latch_b(0),
		m_sound_latch_c(0),
		m_poly18(nullptr),
		m_polybit(0),
		m_pa5_resistor(0),
		m_pc5_resistor(0),
		m_polybit_resistor(0),
		m_opamp_resistor(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pleiads_sound_device::device_start()
{
	/* The real values are _unknown_!
	* I took the ones from Naughty Boy / Pop Flamer
	*/

	/* charge 10u?? (C??) through 330K?? (R??) -> 3.3s */
	m_pa5.charge_time = 3.3;

	/* discharge 10u?? (C??) through 220k?? (R??) -> 2.2s */
	m_pa5.discharge_time = 2.2;

	/* charge 2.2uF?? through 330?? -> 0.000726s */
	m_pa6.charge_time = 0.000726;

	/* discharge 2.2uF?? through 10k?? -> 0.22s */
	m_pa6.discharge_time = 0.022;

	/* 10k and 10uF */
	m_pb4.charge_time = 0.1;
	m_pb4.discharge_time = 0.1;

	/* charge C49 (22u?) via R47 (2k?) and R48 (1k)
	 * time constant (1000+2000) * 22e-6 = 0.066s */
	m_pc4.charge_time = 0.066;

	/* discharge C49 (22u?) via R48 (1k) and diode D1
	 * time constant 1000 * 22e-6 = 0.022s */
	m_pc4.discharge_time = 0.022;

	/* charge 10u?? through 330 -> 0.0033s */
	m_pc5.charge_time = 0.0033;

	/* discharge 10u?? through ??k (R??) -> 0.1s */
	m_pc5.discharge_time = 0.1;

	/* both in K */
	m_pa5_resistor = 33;
	m_pc5_resistor = 47;

	/* upper 556 upper half: Ra=10k??, Rb=200k??, C=0.01uF?? -> 351Hz */
	m_tone2.max_freq = 351;

	/* upper 556 lower half: Ra=47k??, Rb=100k??, C=0.01uF?? -> 582Hz */
	m_tone3.max_freq = 582;

	/* lower 556 upper half: Ra=33k??, Rb=100k??, C=0.0047uF??
	   freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	m_tone4.max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	m_polybit_resistor = 47;
	m_opamp_resistor = 20;

	/* lower 556 lower half: Ra=100k??, Rb=1k??, C=0.01uF??
	  freq = 1.44 / ((100000+2*1000) * 0.01e-6) = approx. 1412 Hz */
	m_noise.freq = 1412; /* higher noise rate than popflame/naughtyb??? */

	common_start();
}

DEFINE_DEVICE_TYPE(NAUGHTYB_SOUND, naughtyb_sound_device, "naughtyb_sound", "Naughty Boy Custom Sound")

naughtyb_sound_device::naughtyb_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pleiads_sound_device(mconfig, NAUGHTYB_SOUND, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void naughtyb_sound_device::device_start()
{
		/* charge 10u??? through 330K (R??) -> 3.3s */
	m_pa5.charge_time = 3.3;

	/* discharge 10u through 220k (R??) -> 2.1s */
	m_pa5.discharge_time = 2.2;

	/* charge 2.2uF through 330 -> 0.000726s */
	m_pa6.charge_time = 0.000726;

	/* discharge 2.2uF through 10K -> 0.022s */
	m_pa6.discharge_time = 0.022;

	/* 10k and 10uF */
	m_pb4.charge_time = 0.1;
	m_pb4.discharge_time = 0.1;

	/* charge 10uF? (C??) via 3k?? (R??) and 2k?? (R28?)
	 * time constant (3000+2000) * 10e-6 = 0.05s */
	m_pc4.charge_time = 0.05 * 10;

	/* discharge 10uF? (C??) via 2k?? R28??  and diode D?
	 * time constant 2000 * 10e-6 = 0.02s */
	m_pc4.discharge_time = 0.02 * 10;

	/* charge 10u through 330 -> 0.0033s */
	m_pc5.charge_time = 0.0033;

	/* discharge 10u through ??k (R??) -> 0.1s */
	m_pc5.discharge_time = 0.1;

	/* both in K */
	m_pa5_resistor = 100;
	m_pc5_resistor = 78;

	/* upper 556 upper half: 10k, 200k, 0.01uF -> 351Hz */
	m_tone2.max_freq = 351;

	/* upper 556 lower half: 47k, 200k, 0.01uF -> 322Hz */
	m_tone3.max_freq = 322;

	/* lower 556 upper half: Ra=33k, Rb=100k, C=0.0047uF
	   freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	m_tone4.max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	m_polybit_resistor = 47;
	m_opamp_resistor = 20;

	/* lower 556 lower half: Ra=200k, Rb=1k, C=0.01uF
	  freq = 1.44 / ((200000+2*1000) * 0.01e-6) = approx. 713 Hz */
	m_noise.freq = 713;

	common_start();
}

DEFINE_DEVICE_TYPE(POPFLAME_SOUND, popflame_sound_device, "popflame_sound", "Pop Flamer Custom Sound")

popflame_sound_device::popflame_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pleiads_sound_device(mconfig, POPFLAME_SOUND, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void popflame_sound_device::device_start()
{
		/* charge 10u (C63 in Pop Flamer) through 330K -> 3.3s */
	m_pa5.charge_time = 3.3;

	/* discharge 10u (C63 in Pop Flamer) through 220k -> 2.2s */
	m_pa5.discharge_time = 2.2;

	/* charge 2.2uF through 330 -> 0.000726s */
	m_pa6.charge_time = 0.000726;

	/* discharge 2.2uF through 10K -> 0.022s */
	m_pa6.discharge_time = 0.022;

	/* 2k and 10uF */
	m_pb4.charge_time = 0.02;
	m_pb4.discharge_time = 0.02;

	/* charge 2.2uF (C49?) via R47 (100) and R48 (1k)
	 * time constant (100+1000) * 2.2e-6 = 0.00242 */
	m_pc4.charge_time = 0.000242;

	/* discharge 2.2uF (C49?) via R48 (1k) and diode D1
	 * time constant 1000 * 22e-6 = 0.0022s */
	m_pc4.discharge_time = 0.00022;

	/* charge 22u (C52 in Pop Flamer) through 10k -> 0.22s */
	m_pc5.charge_time = 0.22;

	/* discharge 22u (C52 in Pop Flamer) through ??k (R??) -> 0.1s */
	m_pc5.discharge_time = 0.1;

	/* both in K */
	m_pa5_resistor = 33;
	m_pc5_resistor = 47;

	/* upper 556 upper half: Ra=10k, Rb=100k, C=0.01uF -> 1309Hz */
	m_tone2.max_freq = 1309;

	/* upper 556 lower half: Ra=10k??, Rb=120k??, C=0.01uF -> 1108Hz */
	m_tone3.max_freq = 1108;

	/* lower 556 upper half: Ra=33k, Rb=100k, C=0.0047uF
	   freq = 1.44 / ((33000+2*100000) * 0.0047e-6) = approx. 1315 Hz */
	m_tone4.max_freq = 1315;

	/* how to divide the V/C voltage for tone #4 */
	m_polybit_resistor = 20;
	m_opamp_resistor = 20;

	/* lower 556 lower half: Ra=200k, Rb=1k, C=0.01uF
	  freq = 1.44 / ((200000+2*1000) * 0.01e-6) = approx. 713 Hz */
	m_noise.freq = 713;

	common_start();
}

/*****************************************************************************
 * Tone #1 is a fixed 8 kHz signal divided by 1 to 15.
 *****************************************************************************/
inline int pleiads_sound_device::tone1(int samplerate)
{
	if( (m_sound_latch_a & 15) != 15 )
	{
		m_tone1.counter -= TONE1_CLOCK;
		while( m_tone1.counter <= 0 )
		{
			m_tone1.counter += samplerate;
			if( ++m_tone1.max_freq == 16 )
			{
				m_tone1.max_freq = m_sound_latch_a & 15;
				m_tone1.output ^= 1;
			}
		}
	}
	return m_tone1.output ? VMAX : -VMAX;
}

/*****************************************************************************
 * Tones #2 and #3 are coming from the upper 556 chip
 * It's labelled IC96 in Pop Flamer, 4D(??) in Naughty Boy.
 * C68 controls the frequencies of tones #2 and #3 (V/C inputs)
 *****************************************************************************/
inline int pleiads_sound_device::update_pb4(int samplerate)
{
	/* bit 4 of latch B: charge 10uF (C28/C68) through 10k (R19/R25) */
	if( m_sound_latch_b & 0x10 )
	{
		if( m_pb4.level < VMAX )
		{
			m_pb4.counter -= (int)((VMAX - m_pb4.level) / m_pb4.charge_time);
			if( m_pb4.counter <= 0 )
			{
				int n = (-m_pb4.counter / samplerate) + 1;
				m_pb4.counter += n * samplerate;
				if( (m_pb4.level += n) > VMAX )
					m_pb4.level = VMAX;
			}
		}
	}
	else
	{
		if( m_pb4.level > VMIN )
		{
			m_pb4.counter -= (int)((m_pb4.level - VMIN) / m_pb4.discharge_time);
			if( m_pb4.counter <= 0 )
			{
				int n = (-m_pb4.counter / samplerate) + 1;
				m_pb4.counter += n * samplerate;
				if( (m_pb4.level -= n) < VMIN)
					m_pb4.level = VMIN;
			}
		}
	}
	return m_pb4.level;
}

inline int pleiads_sound_device::tone23(int samplerate)
{
	int level = VMAX - update_pb4(samplerate);
	int sum = 0;

	/* bit 5 = low: tone23 disabled */
	if( (m_sound_latch_b & 0x20) == 0 )
		return sum;

	/* modulate timers from the upper 556 with the voltage on Cxx on PB4. */
	if( level < VMAX )
	{
		m_tone2.counter -= m_tone2.max_freq * level / 32768;
		if( m_tone2.counter <= 0 )
		{
			int n = (-m_tone2.counter / samplerate) + 1;
			m_tone2.counter += n * samplerate;
			m_tone2.output = (m_tone2.output + n) & 1;
		}

		m_tone3.counter -= m_tone3.max_freq*1/3 + m_tone3.max_freq*2/3 * level / 33768;
		if( m_tone3.counter <= 0 )
		{
			int n = (-m_tone2.counter / samplerate) + 1;
			m_tone3.counter += samplerate;
			m_tone3.output = (m_tone3.output + n) & 1;
		}
	}

	sum += (m_tone2.output) ? VMAX : -VMAX;
	sum += (m_tone3.output) ? VMAX : -VMAX;

	return sum / 2;
}

/*****************************************************************************
 * Tone #4 comes from upper half of the lower 556 (IC98 in Pop Flamer)
 * It's modulated by the voltage at C49, which is then divided between
 * 0V or 5V, depending on the polynomial output bit.
 * The tone signal gates two signals (bits 5 of latches A and C), but
 * these are also swept between two levels (C52 and C53 in Pop Flamer).
 *****************************************************************************/
inline int pleiads_sound_device::update_c_pc4(int samplerate)
{
	#define PC4_MIN (int)(VMAX * 7 / 50)

	/* bit 4 of latch C: (part of videoreg_w) hi? */
	if (m_sound_latch_c & 0x10)
	{
		if (m_pc4.level < VMAX)
		{
			m_pc4.counter -= (int)((VMAX - m_pc4.level) / m_pc4.charge_time);
			if( m_pc4.counter <= 0 )
			{
				int n = (-m_pc4.counter / samplerate) + 1;
				m_pc4.counter += n * samplerate;
				if( (m_pc4.level += n) > VMAX )
					m_pc4.level = VMAX;
			}
		}
	}
	else
	{
		if (m_pc4.level > PC4_MIN)
		{
			m_pc4.counter -= (int)((m_pc4.level - PC4_MIN) / m_pc4.discharge_time);
			if( m_pc4.counter <= 0 )
			{
				int n = (-m_pc4.counter / samplerate) + 1;
				m_pc4.counter += n * samplerate;
				if( (m_pc4.level -= n) < PC4_MIN )
					m_pc4.level = PC4_MIN;
			}
		}
	}
	return m_pc4.level;
}

inline int pleiads_sound_device::update_c_pc5(int samplerate)
{
	/* bit 5 of latch C: charge or discharge C52 */
	if (m_sound_latch_c & 0x20)
	{
		if (m_pc5.level < VMAX)
		{
			m_pc5.counter -= (int)((VMAX - m_pc5.level) / m_pc5.charge_time);
			if( m_pc5.counter <= 0 )
			{
				int n = (-m_pc5.counter / samplerate) + 1;
				m_pc5.counter += n * samplerate;
				if( (m_pc5.level += n) > VMAX )
					m_pc5.level = VMAX;
			}
		}
	}
	else
	{
		if (m_pc5.level > VMIN)
		{
			m_pc5.counter -= (int)((m_pc5.level - VMIN) / m_pc5.discharge_time);
			if( m_pc5.counter <= 0 )
			{
				int n = (-m_pc5.counter / samplerate) + 1;
				m_pc5.counter += samplerate;
				if( (m_pc5.level -= n) < VMIN )
					m_pc5.level = VMIN;
			}
		}
	}
	return m_pc5.level;
}

inline int pleiads_sound_device::update_c_pa5(int samplerate)
{
	/* bit 5 of latch A: charge or discharge C63 */
	if (m_sound_latch_a & 0x20)
	{
		if (m_pa5.level < VMAX)
		{
			m_pa5.counter -= (int)((VMAX - m_pa5.level) / m_pa5.charge_time);
			if( m_pa5.counter <= 0 )
			{
				int n = (-m_pa5.counter / samplerate) + 1;
				m_pa5.counter += n * samplerate;
				if( (m_pa5.level += n) > VMAX )
					m_pa5.level = VMAX;
			}
		}
	}
	else
	{
		if (m_pa5.level > VMIN)
		{
			m_pa5.counter -= (int)((m_pa5.level - VMIN) / m_pa5.discharge_time);
			if( m_pa5.counter <= 0 )
			{
				int n = (-m_pa5.counter / samplerate) + 1;
				m_pa5.counter += samplerate;
				if( (m_pa5.level -= n) < VMIN )
					m_pa5.level = VMIN;
			}
		}
	}
	return m_pa5.level;
}

inline int pleiads_sound_device::tone4(int samplerate)
{
	int level = update_c_pc4(samplerate);
	int vpc5 = update_c_pc5(samplerate);
	int vpa5 = update_c_pa5(samplerate);
	int sum = 0;

	/* Two resistors divide the output voltage of the op-amp between
	 * polybit = 0: 0V and level: x * opamp_resistor / (opamp_resistor + polybit_resistor)
	 * polybit = 1: level and 5V: x * polybit_resistor / (opamp_resistor + polybit_resistor)
	 */
	if (m_polybit)
		level = level + (VMAX - level) * m_opamp_resistor / (m_opamp_resistor + m_polybit_resistor);
	else
		level = level * m_polybit_resistor / (m_opamp_resistor + m_polybit_resistor);

	m_tone4.counter -= m_tone4.max_freq * level / 32768;
	if( m_tone4.counter <= 0 )
	{
		int n = (-m_tone4.counter / samplerate) + 1;
		m_tone4.counter += n * samplerate;
		m_tone4.output = (m_tone4.output + n) & 1;
	}

	/* mix the two signals */
	sum = vpc5 * m_pa5_resistor / (m_pa5_resistor + m_pc5_resistor) +
			vpa5 * m_pc5_resistor / (m_pa5_resistor + m_pc5_resistor);

	return (m_tone4.output) ? sum : -sum;
}

/*****************************************************************************
 * Noise comes from a shift register (4006) hooked up just like in Phoenix.
 * Difference: the clock frequency is toggled between two values only by
 * bit 4 of latch A. The output of the first shift register can be zapped(?)
 * by some control line (IC87 in Pop Flamer: not yet implemented)
 *****************************************************************************/
inline int pleiads_sound_device::update_c_pa6(int samplerate)
{
	/* bit 6 of latch A: charge or discharge C63 */
	if (m_sound_latch_a & 0x40)
	{
		if (m_pa6.level < VMAX)
		{
			m_pa6.counter -= (int)((VMAX - m_pa6.level) / m_pa6.charge_time);
			if( m_pa6.counter <= 0 )
			{
				int n = (-m_pa6.counter / samplerate) + 1;
				m_pa6.counter += n * samplerate;
				if( (m_pa6.level += n) > VMAX )
					m_pa6.level = VMAX;
			}
		}
	}
	else
	{
		/* only discharge of poly bit is active */
		if (m_polybit && m_pa6.level > VMIN)
		{
			/* discharge 10uF through 10k -> 0.1s */
			m_pa6.counter -= (int)((m_pa6.level - VMIN) / 0.1);
			if( m_pa6.counter <= 0 )
			{
				int n = (-m_pa6.counter / samplerate) + 1;
				m_pa6.counter += n * samplerate;
				if( (m_pa6.level -= n) < VMIN )
					m_pa6.level = VMIN;
			}
		}
	}
	return m_pa6.level;
}


inline int pleiads_sound_device::noise(int samplerate)
{
	int c_pa6_level = update_c_pa6(samplerate);
	int sum = 0;

	/*
	 * bit 4 of latch A: noise counter rate modulation?
	 * CV2 input of lower 556 is connected via 2k resistor
	 */
	if ( m_sound_latch_a & 0x10 )
		m_noise.counter -= m_noise.freq * 2 / 3; /* ????? */
	else
		m_noise.counter -= m_noise.freq * 1 / 3; /* ????? */

	if( m_noise.counter <= 0 )
	{
		int n = (-m_noise.counter / samplerate) + 1;
		m_noise.counter += n * samplerate;
		m_noise.polyoffs = (m_noise.polyoffs + n) & 0x3ffff;
		m_polybit = (m_poly18[m_noise.polyoffs>>5] >> (m_noise.polyoffs & 31)) & 1;
	}

	/* The polynomial output bit is used to gate bits 6 + 7 of
	 * sound latch A through the upper half of a 4066 chip.
	 * Bit 6 is sweeping a capacitor between 0V and 4.7V
	 * while bit 7 is connected directly to the 4066.
	 * Both outputs are then filtered, bit 7 even twice,
	 * but it's beyond me what the filters there are doing...
	 */
	if (m_polybit)
	{
		sum += c_pa6_level;
		/* bit 7 is connected directly */
		if (m_sound_latch_a & 0x80)
			sum += VMAX;
	}
	else
	{
		sum -= c_pa6_level;
		/* bit 7 is connected directly */
		if (m_sound_latch_a & 0x80)
			sum -= VMAX;
	}

	return sum / 2;
}

void pleiads_sound_device::control_a_w(uint8_t data)
{
	if (data == m_sound_latch_a)
		return;

	LOG("pleiads_sound_control_a_w $%02x\n", data);

	m_channel->update();
	m_sound_latch_a = data;
}

void pleiads_sound_device::control_b_w(uint8_t data)
{
	/*
	 * pitch selects one of 4 possible clock inputs
	 * (actually 3, because IC2 and IC3 are tied together)
	 * write note value to TMS3615; voice b1 & b2
	 */
	int note = data & 15;
	int pitch = (data >> 6) & 3;

	if (data == m_sound_latch_b)
		return;

	LOG("pleiads_sound_control_b_w $%02x\n", data);

	if (pitch == 3)
		pitch = 2;  /* 2 and 3 are the same */

	m_tms->tms36xx_note_w(pitch, note);

	m_channel->update();
	m_sound_latch_b = data;
}

/* two bits (4 + 5) from the videoreg_w latch go here */
void pleiads_sound_device::control_c_w(uint8_t data)
{
	if (data == m_sound_latch_c)
		return;

	LOG("pleiads_sound_control_c_w $%02x\n", data);
	m_channel->update();
	m_sound_latch_c = data;
}

void pleiads_sound_device::common_start()
{
	int i, j;
	uint32_t shiftreg;

	m_pc4.level = PC4_MIN;
	m_poly18 = make_unique_clear<uint32_t[]>(1ul << (18-5));

	shiftreg = 0;
	for( i = 0; i < (1ul << (18-5)); i++ )
	{
		uint32_t bits = 0;
		for( j = 0; j < 32; j++ )
		{
			bits = (bits >> 1) | (shiftreg << 31);
			if( ((shiftreg >> 16) & 1) == ((shiftreg >> 17) & 1) )
				shiftreg = (shiftreg << 1) | 1;
			else
				shiftreg <<= 1;
		}
		m_poly18[i] = bits;
	}

	m_channel = stream_alloc(0, 1, machine().sample_rate());

	save_item(NAME(m_sound_latch_a));
	save_item(NAME(m_sound_latch_b));
	save_item(NAME(m_sound_latch_c));
	save_item(NAME(m_polybit));
	save_item(NAME(m_pa5_resistor));
	save_item(NAME(m_pc5_resistor));
	save_item(NAME(m_polybit_resistor));
	save_item(NAME(m_opamp_resistor));
	save_item(NAME(m_tone1.counter));
	save_item(NAME(m_tone1.output));
	save_item(NAME(m_tone1.max_freq));
	save_item(NAME(m_tone2.counter));
	save_item(NAME(m_tone2.output));
	save_item(NAME(m_tone2.max_freq));
	save_item(NAME(m_tone3.counter));
	save_item(NAME(m_tone3.output));
	save_item(NAME(m_tone3.max_freq));
	save_item(NAME(m_tone4.counter));
	save_item(NAME(m_tone4.output));
	save_item(NAME(m_tone4.max_freq));
	save_item(NAME(m_pa5.counter));
	save_item(NAME(m_pa5.level));
	save_item(NAME(m_pa5.charge_time));
	save_item(NAME(m_pa5.discharge_time));
	save_item(NAME(m_pa6.counter));
	save_item(NAME(m_pa6.level));
	save_item(NAME(m_pa6.charge_time));
	save_item(NAME(m_pa6.discharge_time));
	save_item(NAME(m_pb4.counter));
	save_item(NAME(m_pb4.level));
	save_item(NAME(m_pb4.charge_time));
	save_item(NAME(m_pb4.discharge_time));
	save_item(NAME(m_pc4.counter));
	save_item(NAME(m_pc4.level));
	save_item(NAME(m_pc4.charge_time));
	save_item(NAME(m_pc4.discharge_time));
	save_item(NAME(m_pc5.counter));
	save_item(NAME(m_pc5.level));
	save_item(NAME(m_pc5.charge_time));
	save_item(NAME(m_pc5.discharge_time));
	save_item(NAME(m_noise.counter));
	save_item(NAME(m_noise.polyoffs));
	save_item(NAME(m_noise.freq));
	save_pointer(NAME(m_poly18), (1ul << (18-5)));
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void pleiads_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &buffer = outputs[0];
	int rate = buffer.sample_rate();

	for (int sampindex = 0; sampindex < buffer.samples(); sampindex++)
	{
		int sum = tone1(rate)/2 + tone23(rate)/2 + tone4(rate) + noise(rate);
		buffer.put_int_clamp(sampindex, sum, 32768);
	}
}

void naughtyb_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	pleiads_sound_device::sound_stream_update(stream, inputs, outputs);
}

void popflame_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	pleiads_sound_device::sound_stream_update(stream, inputs, outputs);
}
