// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    votrax.c

    Votrax SC01A simulation

***************************************************************************/

/*
  tp3 stb i1 i2 tp2
  1   1   o  o  white noise
  1   0   -  1  phone timing clock
  1   0   1  0  closure tick
  1   0   0  0  sram write pulse
  0   -   -  -  sram write pulse

i1.o = glottal impulse
i2.o = white noise

tp1 = phi clock (tied to f2q rom access)
*/

#include "emu.h"
#include "votrax.h"

#define LOG_PHONE  (1U << 1)
#define LOG_COMMIT (1U << 2)
#define LOG_INT    (1U << 3)
#define LOG_TICK   (1U << 4)
#define LOG_FILTER (1U << 5)

//#define VERBOSE (LOG_GENERAL | LOG_PHONE)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VOTRAX_SC01, votrax_sc01_device, "votrsc01", "Votrax SC-01")
DEFINE_DEVICE_TYPE(VOTRAX_SC01A, votrax_sc01a_device, "votrsc01a", "Votrax SC-01-A")

// ROM definition for the Votrax phone ROM
ROM_START( votrax_sc01 )
	ROM_REGION64_LE( 0x200, "internal", 0 )
	ROM_LOAD( "sc01.bin", 0x000, 0x200, CRC(528d1c57) SHA1(268b5884dce04e49e2376df3e2dc82e852b708c1) )
ROM_END

ROM_START( votrax_sc01a )
	ROM_REGION64_LE( 0x200, "internal", 0 )
	ROM_LOAD( "sc01a.bin", 0x000, 0x200, CRC(fc416227) SHA1(1d6da90b1807a01b5e186ef08476119a862b5e6d) )
ROM_END

// textual phone names for debugging
const char *const votrax_sc01_device::s_phone_table[64] =
{
	"EH3",  "EH2",  "EH1",  "PA0",  "DT",   "A1",   "A2",   "ZH",
	"AH2",  "I3",   "I2",   "I1",   "M",    "N",    "B",    "V",
	"CH",   "SH",   "Z",    "AW1",  "NG",   "AH1",  "OO1",  "OO",
	"L",    "K",    "J",    "H",    "G",    "F",    "D",    "S",
	"A",    "AY",   "Y1",   "UH3",  "AH",   "P",    "O",    "I",
	"U",    "Y",    "T",    "R",    "E",    "W",    "AE",   "AE1",
	"AW2",  "UH2",  "UH1",  "UH",   "O2",   "O1",   "IU",   "U1",
	"THV",  "TH",   "ER",   "EH",   "E1",   "AW",   "PA1",  "STOP"
};

// This waveform is built using a series of transistors as a resistor
// ladder.  There is first a transistor to ground, then a series of
// seven transistors one quarter the size of the first one, then it
// finishes by an active resistor to +9V.
//
// The terminal of the transistor to ground is used as a middle value.
// Index 0 is at that value. Index 1 is at 0V.  Index 2 to 8 start at
// just after the resistor down the latter.  Indices 9+ are the middle
// value again.
//
// For simplicity, we rescale the values to get the middle at 0 and
// the top at 1.  The final wave is very similar to the patent
// drawing.

const double votrax_sc01_device::s_glottal_wave[9] =
{
	0,
	-4/7.0,
	7/7.0,
	6/7.0,
	5/7.0,
	4/7.0,
	3/7.0,
	2/7.0,
	1/7.0
};

votrax_sc01_device::votrax_sc01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: votrax_sc01_device(mconfig, VOTRAX_SC01, tag, owner, clock)
{
}

// overridable type for subclass
votrax_sc01_device::votrax_sc01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_stream(nullptr),
	  m_rom(*this, "internal"),
	  m_ar_cb(*this)
{
}

votrax_sc01a_device::votrax_sc01a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: votrax_sc01_device(mconfig, VOTRAX_SC01A, tag, owner, clock)
{
}

void votrax_sc01_device::write(uint8_t data)
{
	// flush out anything currently processing
	m_stream->update();

	u8 prev = m_phone;

	// only 6 bits matter
	m_phone = data & 0x3f;

	if(m_phone != prev || m_phone != 0x3f)
		LOGMASKED(LOG_PHONE, "phone %02x.%d %s\n", m_phone, m_inflection, s_phone_table[m_phone]);

	m_ar_state = CLEAR_LINE;
	m_ar_cb(m_ar_state);

	// Schedule a commit/ar reset at roughly 0.1ms in the future (one
	// phi1 transition followed by the rom extra state in practice),
	// but only if there isn't already one on the fly.  It will
	// override an end-of-phone timeout if there's one pending, but
	// that's not a problem since stb does that anyway.
	if(m_timer->expire().is_never() || m_timer->param() != T_COMMIT_PHONE)
		m_timer->adjust(attotime::from_ticks(72, m_mainclock), T_COMMIT_PHONE);
}


//-------------------------------------------------
//  inflection_w - handle a write to the
//  inflection bits
//-------------------------------------------------

void votrax_sc01_device::inflection_w(uint8_t data)
{
	// only 2 bits matter
	data &= 3;
	if(m_inflection == data)
		return;

	m_stream->update();
	m_inflection = data;
}


//-------------------------------------------------
//  sound_stream_update - handle update requests
//  for our sound stream
//-------------------------------------------------

void votrax_sc01_device::sound_stream_update(sound_stream &stream)
{
	for(int i=0; i<stream.samples(); i++) {
		m_sample_count++;
		if(m_sample_count & 1)
			chip_update();
		stream.put(0, i, analog_calc());
	}
}



//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  rom_region - return a pointer to the device's
//  internal ROM region
//-------------------------------------------------

const tiny_rom_entry *votrax_sc01_device::device_rom_region() const
{
	return ROM_NAME( votrax_sc01 );
}

const tiny_rom_entry *votrax_sc01a_device::device_rom_region() const
{
	return ROM_NAME( votrax_sc01a );
}


//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void votrax_sc01_device::device_start()
{
	// initialize internal state
	m_mainclock = clock();
	m_sclock = m_mainclock / 18.0;
	m_cclock = m_mainclock / 36.0;
	m_stream = stream_alloc(0, 1, m_sclock);
	m_timer = timer_alloc(FUNC(votrax_sc01_device::phone_tick), this);

	// reset outputs
	m_ar_state = ASSERT_LINE;

	// save inputs
	save_item(NAME(m_inflection));
	save_item(NAME(m_phone));

	// save outputs
	save_item(NAME(m_ar_state));

	// save internal state
	save_item(NAME(m_rom_duration));
	save_item(NAME(m_rom_vd));
	save_item(NAME(m_rom_cld));
	save_item(NAME(m_rom_fa));
	save_item(NAME(m_rom_fc));
	save_item(NAME(m_rom_va));
	save_item(NAME(m_rom_f1));
	save_item(NAME(m_rom_f2));
	save_item(NAME(m_rom_f2q));
	save_item(NAME(m_rom_f3));
	save_item(NAME(m_rom_closure));
	save_item(NAME(m_rom_pause));
	save_item(NAME(m_cur_fa));
	save_item(NAME(m_cur_fc));
	save_item(NAME(m_cur_va));
	save_item(NAME(m_cur_f1));
	save_item(NAME(m_cur_f2));
	save_item(NAME(m_cur_f2q));
	save_item(NAME(m_cur_f3));
	save_item(NAME(m_filt_fa));
	save_item(NAME(m_filt_fc));
	save_item(NAME(m_filt_va));
	save_item(NAME(m_filt_f1));
	save_item(NAME(m_filt_f2));
	save_item(NAME(m_filt_f2q));
	save_item(NAME(m_filt_f3));
	save_item(NAME(m_phonetick));
	save_item(NAME(m_ticks));
	save_item(NAME(m_pitch));
	save_item(NAME(m_closure));
	save_item(NAME(m_update_counter));
	save_item(NAME(m_cur_closure));
	save_item(NAME(m_noise));
	save_item(NAME(m_cur_noise));
	save_item(NAME(m_voice_1));
	save_item(NAME(m_voice_2));
	save_item(NAME(m_voice_3));
	save_item(NAME(m_noise_1));
	save_item(NAME(m_noise_2));
	save_item(NAME(m_noise_3));
	save_item(NAME(m_noise_4));
	save_item(NAME(m_vn_1));
	save_item(NAME(m_vn_2));
	save_item(NAME(m_vn_3));
	save_item(NAME(m_vn_4));
	save_item(NAME(m_vn_5));
	save_item(NAME(m_vn_6));
	save_item(NAME(m_f1_a));
	save_item(NAME(m_f1_b));
	save_item(NAME(m_f2v_a));
	save_item(NAME(m_f2v_b));
	save_item(NAME(m_f2n_a));
	save_item(NAME(m_f2n_b));
	save_item(NAME(m_f3_a));
	save_item(NAME(m_f3_b));
	save_item(NAME(m_f4_a));
	save_item(NAME(m_f4_b));
	save_item(NAME(m_fx_a));
	save_item(NAME(m_fx_b));
	save_item(NAME(m_fn_a));
	save_item(NAME(m_fn_b));
}


//-------------------------------------------------
//  device_reset - handle device reset
//-------------------------------------------------

void votrax_sc01_device::device_reset()
{
	// Technically, there's no reset in this chip, and initial state
	// is random.  Still, it's a good idea to start it with something
	// sane.

	m_phone = 0x3f;
	m_inflection = 0;
	m_ar_state = ASSERT_LINE;
	m_ar_cb(m_ar_state);

	m_sample_count = 0;

	// Initialize the m_rom* values
	phone_commit();

	// Clear the interpolation sram
	m_cur_fa = m_cur_fc = m_cur_va = 0;
	m_cur_f1 = m_cur_f2 = m_cur_f2q = m_cur_f3 = 0;

	// Initialize the m_filt* values and the filter coefficients
	filters_commit(true);

	// Clear the rest of the internal digital state
	m_pitch = 0;
	m_closure = 0;
	m_update_counter = 0;
	m_cur_closure = true;
	m_noise = 0;
	m_cur_noise = false;

	// Clear the analog level histories
	memset(m_voice_1, 0, sizeof(m_voice_1));
	memset(m_voice_2, 0, sizeof(m_voice_2));
	memset(m_voice_3, 0, sizeof(m_voice_3));

	memset(m_noise_1, 0, sizeof(m_noise_1));
	memset(m_noise_2, 0, sizeof(m_noise_2));
	memset(m_noise_3, 0, sizeof(m_noise_3));
	memset(m_noise_4, 0, sizeof(m_noise_4));

	memset(m_vn_1, 0, sizeof(m_vn_1));
	memset(m_vn_2, 0, sizeof(m_vn_2));
	memset(m_vn_3, 0, sizeof(m_vn_3));
	memset(m_vn_4, 0, sizeof(m_vn_4));
	memset(m_vn_5, 0, sizeof(m_vn_5));
	memset(m_vn_6, 0, sizeof(m_vn_6));
}


//-------------------------------------------------
//  device_clock_changed - handle dynamic clock
//  changes by altering our output frequency
//-------------------------------------------------

void votrax_sc01_device::device_clock_changed()
{
	// lookup the new frequency of the master clock, and update if changed
	u32 newfreq = clock();
	if(newfreq != m_mainclock) {
		m_stream->update();

		if(!m_timer->expire().is_never()) {
			// determine how many clock ticks remained on the timer
			u64 remaining = m_timer->remaining().as_ticks(m_mainclock);

			// adjust the timer to the same number of ticks based on the new frequency
			m_timer->adjust(attotime::from_ticks(remaining, newfreq));
		}
		m_mainclock = newfreq;
		m_sclock = m_mainclock / 18.0;
		m_cclock = m_mainclock / 36.0;
		m_stream->set_sample_rate(m_sclock);
		filters_commit(true);
	}
}


//-------------------------------------------------
//  phone_tick - process transitions
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(votrax_sc01_device::phone_tick)
{
	m_stream->update();

	switch(param) {
	case T_COMMIT_PHONE:
		// strobe -> commit transition,
		phone_commit();
		m_timer->adjust(attotime::from_ticks(16*(m_rom_duration*4+1)*4*9+2, m_mainclock), T_END_OF_PHONE);
		break;

	case T_END_OF_PHONE:
		// end of phone
		m_ar_state = ASSERT_LINE;
		break;
	}

	m_ar_cb(m_ar_state);
}

void votrax_sc01_device::phone_commit()
{
	// Only these two counters are reset on phone change, the rest is
	// free-running.
	m_phonetick = 0;
	m_ticks = 0;

	// In the real chip, the rom is re-read all the time.  Since it's
	// internal and immutable, no point in not caching it though.
	for(int i=0; i<64; i++) {
		u64 val = reinterpret_cast<const u64 *>(m_rom->base())[i];
		if(m_phone == ((val >> 56) & 0x3f)) {
			m_rom_f1  = bitswap(val,  0,  7, 14, 21);
			m_rom_va  = bitswap(val,  1,  8, 15, 22);
			m_rom_f2  = bitswap(val,  2,  9, 16, 23);
			m_rom_fc  = bitswap(val,  3, 10, 17, 24);
			m_rom_f2q = bitswap(val,  4, 11, 18, 25);
			m_rom_f3  = bitswap(val,  5, 12, 19, 26);
			m_rom_fa  = bitswap(val,  6, 13, 20, 27);

			// These two values have their bit orders inverted
			// compared to everything else due to a bug in the
			// prototype (miswiring of the comparator with the ticks
			// count) they compensated in the rom.

			m_rom_cld = bitswap(val, 34, 32, 30, 28);
			m_rom_vd  = bitswap(val, 35, 33, 31, 29);

			m_rom_closure  = bitswap(val, 36);
			m_rom_duration = bitswap(~val, 37, 38, 39, 40, 41, 42, 43);

			// Hard-wired on the die, not an actual part of the rom.
			m_rom_pause = (m_phone == 0x03) || (m_phone == 0x3e);

			LOGMASKED(LOG_COMMIT, "commit fa=%x va=%x fc=%x f1=%x f2=%x f2q=%x f3=%x dur=%02x cld=%x vd=%d cl=%d pause=%d\n", m_rom_fa, m_rom_va, m_rom_fc, m_rom_f1, m_rom_f2, m_rom_f2q, m_rom_f3, m_rom_duration, m_rom_cld, m_rom_vd, m_rom_closure, m_rom_pause);

			// That does not happen in the sc01(a) rom, but let's
			// cover our behind.
			if(m_rom_cld == 0)
				m_cur_closure = m_rom_closure;

			return;
		}
	}
}

void votrax_sc01_device::interpolate(u8 &reg, u8 target)
{
	// One step of interpolation, adds one eight of the distance
	// between the current value and the target.
	reg = reg - (reg >> 3) + (target << 1);
}

void votrax_sc01_device::chip_update()
{
	// Phone tick counter update.  Stopped when ticks reach 16.
	// Technically the counter keeps updating, but the comparator is
	// disabled.
	if(m_ticks != 0x10) {
		m_phonetick++;
		// Comparator is with duration << 2, but there's a one-tick
		// delay in the path.
		if(m_phonetick == ((m_rom_duration << 2) | 1)) {
			m_phonetick = 0;
			m_ticks++;
			if(m_ticks == m_rom_cld)
				m_cur_closure = m_rom_closure;
		}
	}

	// The two update timing counters.  One divides by 16, the other
	// by 48, and they're phased so that the 208Hz counter ticks
	// exactly between two 625Hz ticks.
	m_update_counter++;
	if(m_update_counter == 0x30)
		m_update_counter = 0;

	bool tick_625 = !(m_update_counter & 0xf);
	bool tick_208 = m_update_counter == 0x28;

	// Formant update.  Die bug there: fc should be updated, not va.
	// The formants are frozen on a pause phone unless both voice and
	// noise volumes are zero.
	if(tick_208 && (!m_rom_pause || !(m_filt_fa || m_filt_va))) {
		// interpolate(m_cur_va,  m_rom_va);
		interpolate(m_cur_fc,  m_rom_fc);
		interpolate(m_cur_f1,  m_rom_f1);
		interpolate(m_cur_f2,  m_rom_f2);
		interpolate(m_cur_f2q, m_rom_f2q);
		interpolate(m_cur_f3,  m_rom_f3);
		LOGMASKED(LOG_INT, "int fa=%x va=%x fc=%x f1=%x f2=%02x f2q=%02x f3=%x\n", m_cur_fa >> 4, m_cur_va >> 4, m_cur_fc >> 4, m_cur_f1 >> 4, m_cur_f2 >> 3, m_cur_f2q >> 4, m_cur_f3 >> 4);
	}

	// Non-formant update. Same bug there, va should be updated, not fc.
	if(tick_625) {
		if(m_ticks >= m_rom_vd)
			interpolate(m_cur_fa, m_rom_fa);
		if(m_ticks >= m_rom_cld) {
			// interpolate(m_cur_fc, m_rom_fc);
			interpolate(m_cur_va, m_rom_va);
			LOGMASKED(LOG_INT, "int fa=%x va=%x fc=%x f1=%x f2=%02x f2q=%02x f3=%x\n", m_cur_fa >> 4, m_cur_va >> 4, m_cur_fc >> 4, m_cur_f1 >> 4, m_cur_f2 >> 3, m_cur_f2q >> 4, m_cur_f3 >> 4);
		}
	}

	// Closure counter, reset every other tick in theory when not
	// active (on the extra rom cycle).
	//
	// The closure level is immediately used in the analog path,
	// there's no pitch synchronization.

	if(!m_cur_closure && (m_filt_fa || m_filt_va))
		m_closure = 0;
	else if(m_closure != 7 << 2)
		m_closure ++;

	// Pitch counter.  Equality comparison, so it's possible to make
	// it miss by manipulating the inflection inputs, but it'll wrap.
	// There's a delay, hence the +2.

	// Intrinsically pre-divides by two, so we added one bit on the 7

	m_pitch = (m_pitch + 1) & 0xff;
	if(m_pitch == (0xe0 ^ (m_inflection << 5) ^ (m_filt_f1 << 1)) + 2)
		m_pitch = 0;

	// Filters are updated in index 1 of the pitch wave, which does
	// indeed mean four times in a row.
	if((m_pitch & 0xf9) == 0x08)
		filters_commit(false);

	// Noise shift register.  15 bits, with a nxor on the last two
	// bits for the loop.
	bool inp = (1||m_filt_fa) && m_cur_noise && (m_noise != 0x7fff);
	m_noise = ((m_noise << 1) & 0x7ffe) | inp;
	m_cur_noise = !(((m_noise >> 14) ^ (m_noise >> 13)) & 1);

	LOGMASKED(LOG_TICK, "%s tick %02x.%03x 625=%d 208=%d pitch=%02x.%x ns=%04x ni=%d noise=%d cl=%x.%x clf=%d/%d\n", machine().time().to_string(), m_ticks, m_phonetick, tick_625, tick_208, m_pitch >> 3, m_pitch & 7, m_noise, inp, m_cur_noise, m_closure >> 2, m_closure & 3, m_rom_closure, m_cur_closure);
}

void votrax_sc01_device::filters_commit(bool force)
{
	m_filt_fa = m_cur_fa >> 4;
	m_filt_fc = m_cur_fc >> 4;
	m_filt_va = m_cur_va >> 4;

	if(force || m_filt_f1 != m_cur_f1 >> 4) {
		m_filt_f1 = m_cur_f1 >> 4;

		build_standard_filter(m_f1_a, m_f1_b,
							  11247,
							  11797,
							  949,
							  52067,
							  2280 + bits_to_caps(m_filt_f1, { 2546, 4973, 9861, 19724 }),
							  166272);
	}

	if(force || m_filt_f2 != m_cur_f2 >> 3 || m_filt_f2q != m_cur_f2q >> 4) {
		m_filt_f2 = m_cur_f2 >> 3;
		m_filt_f2q = m_cur_f2q >> 4;

		build_standard_filter(m_f2v_a, m_f2v_b,
							  24840,
							  29154,
							  829 + bits_to_caps(m_filt_f2q, { 1390, 2965, 5875, 11297 }),
							  38180,
							  2352 + bits_to_caps(m_filt_f2, { 833, 1663, 3164, 6327, 12654 }),
							  34270);

		build_injection_filter(m_f2n_a, m_f2n_b,
							   29154,
							   829 + bits_to_caps(m_filt_f2q, { 1390, 2965, 5875, 11297 }),
							   38180,
							   2352 + bits_to_caps(m_filt_f2, { 833, 1663, 3164, 6327, 12654 }),
							   34270);
	}

	if(force || m_filt_f3 != m_cur_f3 >> 4) {
		m_filt_f3 = m_cur_f3 >> 4;
		build_standard_filter(m_f3_a, m_f3_b,
							  0,
							  17594,
							  868,
							  18828,
							  8480 + bits_to_caps(m_filt_f3, { 2226, 4485, 9056, 18111 }),
							  50019);
	}

	if(force) {
		build_standard_filter(m_f4_a, m_f4_b,
							  0,
							  28810,
							  1165,
							  21457,
							  8558,
							  7289);

		build_lowpass_filter(m_fx_a, m_fx_b,
							 1122,
							 23131);

		build_noise_shaper_filter(m_fn_a, m_fn_b,
								  15500,
								  14854,
								  8450,
								  9523,
								  14083);
	}

	if(m_filt_fa | m_filt_va | m_filt_fc | m_filt_f1 | m_filt_f2 | m_filt_f2q | m_filt_f3)
		LOGMASKED(LOG_FILTER, "filter fa=%x va=%x fc=%x f1=%x f2=%02x f2q=%x f3=%x\n", m_filt_fa, m_filt_va, m_filt_fc, m_filt_f1, m_filt_f2, m_filt_f2q, m_filt_f3);
}

sound_stream::sample_t votrax_sc01_device::analog_calc()
{
	// Voice-only path.
	// 1. Pick up the pitch wave

	double v = m_pitch >= (9 << 3) ? 0 : s_glottal_wave[m_pitch >> 3];

	// 2. Multiply by the initial amplifier.  It's linear on the die,
	// even if it's not in the patent.
	v = v * m_filt_va / 15.0;
	shift_hist(v, m_voice_1);

	// 3. Apply the f1 filter
	v = apply_filter(m_voice_1, m_voice_2, m_f1_a, m_f1_b);
	shift_hist(v, m_voice_2);

	// 4. Apply the f2 filter, voice half
	v = apply_filter(m_voice_2, m_voice_3, m_f2v_a, m_f2v_b);
	shift_hist(v, m_voice_3);

	// Noise-only path
	// 5. Pick up the noise pitch.  Amplitude is linear.  Base
	// intensity should be checked w.r.t the voice.
	double n = 1e4 * ((m_pitch & 0x40 ? m_cur_noise : false) ? 1 : -1);
	n = n * m_filt_fa / 15.0;
	shift_hist(n, m_noise_1);

	// 6. Apply the noise shaper
	n = apply_filter(m_noise_1, m_noise_2, m_fn_a, m_fn_b);
	shift_hist(n, m_noise_2);

	// 7. Scale with the f2 noise input
	double n2 = n * m_filt_fc / 15.0;
	shift_hist(n2, m_noise_3);

	// 8. Apply the f2 filter, noise half,
	n2 = apply_filter(m_noise_3, m_noise_4, m_f2n_a, m_f2n_b);
	shift_hist(n2, m_noise_4);

	// Mixed path
	// 9. Add the f2 voice and f2 noise outputs
	double vn = v + n2;
	shift_hist(vn, m_vn_1);

	// 10. Apply the f3 filter
	vn = apply_filter(m_vn_1, m_vn_2, m_f3_a, m_f3_b);
	shift_hist(vn, m_vn_2);

	// 11. Second noise insertion
	vn += n * (5 + (15^m_filt_fc))/20.0;
	shift_hist(vn, m_vn_3);

	// 12. Apply the f4 filter
	vn = apply_filter(m_vn_3, m_vn_4, m_f4_a, m_f4_b);
	shift_hist(vn, m_vn_4);

	// 13. Apply the glottal closure amplitude, also linear
	vn = vn * (7 ^ (m_closure >> 2)) / 7.0;
	shift_hist(vn, m_vn_5);

	// 13. Apply the final fixed filter
	vn = apply_filter(m_vn_5, m_vn_6, m_fx_a, m_fx_b);
	shift_hist(vn, m_vn_6);

	return vn*0.35;
}

/*
  Playing with analog filters, or where all the magic filter formulas are coming from.

  First you start with an analog circuit, for instance this one:

  |                     +--[R2]--+
  |                     |        |
  |                     +--|C2|--+<V1     +--|C3|--+
  |                     |        |        |        |
  |  Vi   +--[R1]--+    |  |\    |        |  |\    |
  |  -----+        +----+--+-\   |        +--+-\   |
  |       +--|C1|--+       |  >--+--[Rx]--+  |  >--+----- Vo
  |                |     0-++/             0-++/   |
  |                |       |/    +--[R0]--+  |/    |
  |                |             |        |        |
  |                |             |    /|  |        |
  |                |             |   /-+--+--[R0]--+
  |                +--[R4]-------+--<  |
  |                            V2^   \++-0
  |                                   \|

  It happens to be what most of the filters in the sc01a look like.

  You need to determine the transfer function H(s) of the circuit, which is
  defined as the ratio Vo/Vi.  To do that, you use some properties:

  - The intensity through an element is equal to the voltage
    difference through the element divided by the impedance

  - The impedance of a resistance is equal to its resistance

  - The impedance of a capacitor is 1/(s*C) where C is its capacitance

  - The impedance of elements in series is the sum of their impedances

  - The impedance of elements in parallel is the inverse of the sum of
    their inverses

  - The sum of all intensities flowing into a node is 0 (there's no
    charge accumulation in a wire)

  - An operational amplifier in looped mode is an interesting beast:
    the intensity at its two inputs is always 0, and the voltage is
    forced identical between the inputs.  In our case, since the '+'
    inputs are all tied to ground, that means that the '-' inputs are at
    voltage 0, intensity 0.

  From here we can build some equations.  Noting:
  X1 = 1/(1/R1 + s*C1)
  X2 = 1/(1/R2 + s*C2)
  X3 = 1/(s*C3)

  Then computing the intensity flow at each '-' input we have:
  Vi/X1 + V2/R4 + V1/X2 = 0
  V2/R0 + Vo/R0 = 0
  V1/Rx + Vo/X3 = 0

  Wrangling the equations, one eventually gets:
  |                            1 + s * C1*R1
  | Vo/Vi = H(s) = (R4/R1) * -------------------------------------------
  |                            1 + s * C3*Rx*R4/R2 + s^2 * C2*C3*Rx*R4

  To check the mathematics between the 's' stuff, check "Laplace
  transform".  In short, it's a nice way of manipulating derivatives
  and integrals without having to manipulate derivatives and
  integrals.

  With that transfer function, we first can compute what happens to
  every frequency in the input signal.  You just compute H(2i*pi*f)
  where f is the frequency, which will give you a complex number
  representing the amplitude and phase effect.  To get the usual dB
  curves, compute 20*log10(abs(v))).

  Now, once you have an analog transfer function, you can build a
  digital filter from it using what is called the bilinear transform.

  In our case, we have an analog filter with the transfer function:
  |                 1 + k[0]*s
  |        H(s) = -------------------------
  |                 1 + k[1]*s + k[2]*s^2

  We can always reintroduce the global multiplier later, and it's 1 in
  most of our cases anyway.

  The we pose:
  |                    z-1
  |        s(z) = zc * ---
  |                    z+1

  where zc = 2*pi*fr/tan(pi*fr/fs)
  with fs = sampling frequency
  and fr = most interesting frequency

  Then we rewrite H in function of negative integer powers of z.

  Noting m0 = zc*k[0], m1 = zc*k[1], m2=zc*zc*k[2],

  a little equation wrangling then gives:

  |                 (1+m0)    + (3+m0)   *z^-1 + (3-m0)   *z^-2 +    (1-m0)*z^-3
  |        H(z) = ----------------------------------------------------------------
  |                 (1+m1+m2) + (3+m1-m2)*z^-1 + (3-m1-m2)*z^-2 + (1-m1+m2)*z^-3

  That beast in the digital transfer function, of which you can
  extract response curves by posing z = exp(2*i*pi*f/fs).

  Note that the bilinear transform is an approximation, and H(z(f)) =
  H(s(f)) only at frequency fr.  And the shape of the filter will be
  better respected around fr.  If you look at the curves of the
  filters we're interested in, the frequency:
  fr = sqrt(abs(k[0]*k[1]-k[2]))/(2*pi*k[2])

  which is a (good) approximation of the filter peak position is a
  good choice.

  Note that terminology wise, the "standard" bilinear transform is
  with fr = fs/2, and using a different fr is called "pre-warping".

  So now we have a digital transfer function of the generic form:

  |                 a[0] + a[1]*z^-1 + a[2]*z^-2 + a[3]*z^-3
  |        H(z) = --------------------------------------------
  |                 b[0] + b[1]*z^-1 + b[2]*z^-2 + b[3]*z^-3

  The magic then is that the powers of z represent time in samples.
  Noting x the input stream and y the output stream, you have:
  H(z) = y(z)/x(z)

  or in other words:
  y*b[0]*z^0 + y*b[1]*z^-1 + y*b[2]*z^-2 + y*b[3]*z^-3 = x*a[0]*z^0 + x*a[1]*z^-1 + x*a[2]*z^-2 + x*a[3]*z^-3

  i.e.

  y*z^0 = (x*a[0]*z^0 + x*a[1]*z^-1 + x*a[2]*z^-2 + x*a[3]*z^-3 - y*b[1]*z^-1 - y*b[2]*z^-2 - y*b[3]*z^-3) / b[0]

  and powers of z being time in samples,

  y[0] = (x[0]*a[0] + x[-1]*a[1] + x[-2]*a[2] + x[-3]*a[3] - y[-1]*b[1] - y[-2]*b[2] - y[-3]*b[3]) / b[0]

  So you have a filter you can apply.  Note that this is why you want
  negative powers of z.  Positive powers would mean looking into the
  future (which is possible in some cases, in particular with x, and
  has some very interesting properties, but is not very useful in
  analog circuit simulation).

  Note that if you have multiple inputs, all this stuff is linear.
  Or, in other words, you just have to split it in multiple circuits
  with only one input connected each time and sum the results.  It
  will be correct.

  Also, since we're in practice in a dynamic system, for an amplifying
  filter (i.e. where things like r4/r1 is not 1), it's better to
  proceed in two steps:

  - amplify the input by the current value of the coefficient, and
    historize it
  - apply the now non-amplifying filter to the historized amplified
    input

  That way reduces the probability of the output bouncing all over the
  place.

  Except, we're not done yet.  Doing resistors precisely in an IC is
  very hard and/or expensive (you may have heard of "laser cut
  resistors" in DACs of the time).  Doing capacitors is easier, and
  their value is proportional to their surface.  So there are no
  resistors on the sc01 die (which is a lie, there are three, but not
  in the filter path.  They are used to scale the voltage in the pitch
  wave and to generate +5V from the +9V), but a magic thing called a
  switched capacitor.  Lookup patent 4,433,210 for details.  Using
  high frequency switching a capacitor can be turned into a resistor
  of value 1/(C*f) where f is the switching frequency (20Khz,
  main/36).  And the circuit is such that the absolute value of the
  capacitors is irrelevant, only their ratio is useful, which factors
  out the intrinsic capacity-per-surface-area of the IC which may be
  hard to keep stable from one die to another.  As a result all the
  capacitor values we use are actually surfaces in square micrometers.

  For the curious, it looks like the actual capacitance was around 25
  femtofarad per square micrometer.

*/

void votrax_sc01_device::build_standard_filter(double *a, double *b,
											   double c1t, // Unswitched cap, input, top
											   double c1b, // Switched cap, input, bottom
											   double c2t, // Unswitched cap, over first amp-op, top
											   double c2b, // Switched cap, over first amp-op, bottom
											   double c3,  // Cap between the two op-amps
											   double c4)  // Cap over second op-amp
{
	// First compute the three coefficients of H(s).  One can note
	// that there is as many capacitor values on both sides of the
	// division, which confirms that the capacity-per-surface-area
	// is not needed.
	double k0 = c1t / (m_cclock * c1b);
	double k1 = c4 * c2t / (m_cclock * c1b * c3);
	double k2 = c4 * c2b / (m_cclock * m_cclock * c1b * c3);

	// Estimate the filter cutoff frequency
	double fpeak = sqrt(fabs(k0*k1 - k2))/(2*M_PI*k2);

	// Turn that into a warp multiplier
	double zc = 2*M_PI*fpeak/tan(M_PI*fpeak / m_sclock);

	// Finally compute the result of the z-transform
	double m0 = zc*k0;
	double m1 = zc*k1;
	double m2 = zc*zc*k2;

	a[0] = 1+m0;
	a[1] = 3+m0;
	a[2] = 3-m0;
	a[3] = 1-m0;
	b[0] = 1+m1+m2;
	b[1] = 3+m1-m2;
	b[2] = 3-m1-m2;
	b[3] = 1-m1+m2;
}

/*
  Second filter type used once at the end, much simpler:

  |           +--[R1]--+
  |           |        |
  |           +--|C1|--+
  |           |        |
  |  Vi       |  |\    |
  |  ---[R0]--+--+-\   |
  |              |  >--+------ Vo
  |            0-++/
  |              |/


  Vi/R0 = Vo / (1/(1/R1 + s.C1)) = Vo (1/R1 + s.C1)
  H(s) = Vo/Vi = (R1/R0) * (1 / (1 + s.R1.C1))
*/

void votrax_sc01_device::build_lowpass_filter(double *a, double *b,
											  double c1t, // Unswitched cap, over amp-op, top
											  double c1b) // Switched cap, over amp-op, bottom
{
	// The caps values puts the cutoff at around 150Hz, put that's no good.
	// Recordings shows we want it around 4K, so fuzz it.

	// Compute the only coefficient we care about
	double k = c1b / (m_cclock * c1t) * (150.0/4000.0);

	// Compute the filter cutoff frequency
	double fpeak = 1/(2*M_PI*k);

	// Turn that into a warp multiplier
	double zc = 2*M_PI*fpeak/tan(M_PI*fpeak / m_sclock);

	// Finally compute the result of the z-transform
	double m = zc*k;

	a[0] = 1;
	b[0] = 1+m;
	b[1] = 1-m;
}

/*
  Used to shape the white noise

         +-------------------------------------------------------------------+
         |                                                                   |
         +--|C1|--+---------|C3|----------+--|C4|--+                         |
         |        |      +        +       |        |                         |
   Vi    |  |\    |     (1)      (1)      |        |       +        +        |
   -|R0|-+--+-\   |      |        |       |  |\    |      (1)      (1)       |
            |  >--+--(2)-+--|C2|--+---(2)-+--+-\   |       |        |        |
          0-++/          |                   |  >--+--(2)--+--|C5|--+---(2)--+
            |/          Vo                 0-++/
                                             |/
   Equivalent:

         +------------------|R5|-------------------+
         |                                         |
         +--|C1|--+---------|C3|----------+--|C4|--+
         |        |                       |        |
   Vi    |  |\    |                       |        |
   -|R0|-+--+-\   |                       |  |\    |
            |  >--+---------|R2|----------+--+-\   |
          0-++/   |                          |  >--+
            |/   Vo                        0-++/
                                             |/

  We assume r0 = r2
*/

void votrax_sc01_device::build_noise_shaper_filter(double *a, double *b,
												   double c1,  // Cap over first amp-op
												   double c2t, // Unswitched cap between amp-ops, input, top
												   double c2b, // Switched cap between amp-ops, input, bottom
												   double c3,  // Cap over second amp-op
												   double c4)  // Switched cap after second amp-op
{
	// Coefficients of H(s) = k1*s / (1 + k2*s + k3*s^2)
	double k0 = c2t*c3*c2b/c4;
	double k1 = c2t*(m_cclock * c2b);
	double k2 = c1*c2t*c3/(m_cclock * c4);

	// Estimate the filter cutoff frequency
	double fpeak = sqrt(1/k2)/(2*M_PI);

	// Turn that into a warp multiplier
	double zc = 2*M_PI*fpeak/tan(M_PI*fpeak / m_sclock);

	// Finally compute the result of the z-transform
	double m0 = zc*k0;
	double m1 = zc*k1;
	double m2 = zc*zc*k2;

	a[0] = m0;
	a[1] = 0;
	a[2] = -m0;
	b[0] = 1+m1+m2;
	b[1] = 2-2*m2;
	b[2] = 1-m1+m2;
}

/*
  Noise injection in f2

  |                     +--[R2]--+        +--[R1]-------- Vi
  |                     |        |        |
  |                     +--|C2|--+<V1     +--|C3|--+
  |                     |        |        |        |
  |                     |  |\    |        |  |\    |
  |                +----+--+-\   |        +--+-\   |
  |                |       |  >--+--[Rx]--+  |  >--+----- Vo
  |                |     0-++/             0-++/   |
  |                |       |/    +--[R0]--+  |/    |
  |                |             |        |        |
  |                |             |    /|  |        |
  |                |             |   /-+--+--[R0]--+
  |                +--[R4]-------+--<  |
  |                            V2^   \++-0
  |                                   \|

  We drop r0/r1 out of the equation (it factorizes), and we rescale so
  that H(infinity)=1.
*/

void votrax_sc01_device::build_injection_filter(double *a, double *b,
												double c1b, // Switched cap, input, bottom
												double c2t, // Unswitched cap, over first amp-op, top
												double c2b, // Switched cap, over first amp-op, bottom
												double c3,  // Cap between the two op-amps
												double c4)  // Cap over second op-amp
{
	// First compute the three coefficients of H(s) = (k0 + k2*s)/(k1 - k2*s)
	double k0 = m_cclock * c2t;
	double k1 = m_cclock * (c1b * c3 / c2t - c2t);
	double k2 = c2b;

	// Don't pre-warp
	double zc = 2*m_sclock;

	// Finally compute the result of the z-transform
	double m = zc*k2;

	a[0] = k0 + m;
	a[1] = k0 - m;
	b[0] = k1 - m;
	b[1] = k1 + m;

	// That ends up in a numerically unstable filter.  Neutralize it for now.
	a[0] = 0;
	a[1] = 0;
	b[0] = 1;
	b[1] = 0;
}
