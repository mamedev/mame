// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu, Klaus
/******************************************************************************

    Toshiba TC8802AF PARCOR speech synthesizer

    Overview
    ========

    The TC8802AF is the speech chip of the V-Tech Socrates speech module
    (and of its localized versions such as the Yeno Professor Weiss-Alles).
    It is a host-controlled member of Toshiba's PARCOR (= LPC with
    reflection coefficients) family and is pin and speech compatible with
    the older T6803; unlike the T6803 it can read its speech ROMs back to
    the host (DTRD command), which the Socrates BIOS needs to look up its
    phrase table.  No datasheet is known.  Everything below was worked out
    from the Socrates and Weiss-Alles BIOS speech drivers, from the speech
    data itself and from a recording of a real NTSC Socrates with the
    English module; what was verified which way is noted in each section.

    Socrates speech module
    ----------------------

        TC8802AF   speech chip with an 8 KiB customer mask ROM inside
        T6684F x4  16 KiB serial speech ROMs ("VSMs") on the external
                   speech ROM bus (the English module has three, the
                   German module one; empty sockets read as FF)
        800 kHz    clock, output sample rate clock / 80 = 10 kHz

    The two speech ROM spaces are independent:

        internal   8 KiB  (byte address 0x0000-0x1fff)
        external  64 KiB  (byte address 0x0000-0xffff: VSM 1 at 0x0000,
                           VSM 2 at 0x4000, VSM 3 at 0x8000, VSM 4 at
                           0xc000); the chip sends out 20 address bits,
                           so up to 1 MiB could be addressed

    Condition register bit 2 selects which space is used for reading
    (DTRD) and for speaking.  The serial protocol between the TC8802 and
    the T6684F is not known; MAME sees the external bus as a plain byte
    wide ROM space (device_rom_interface<20>).


    Host interface
    ==============

    The host writes one byte, D0-D5 = SEL0-SEL5, D6 = 64UP, D7 = START.
    A command is executed on the rising edge of START.  The Socrates BIOS
    always writes a command as three bytes:

        cmd & 0x7f     (set up SEL with START low)
        cmd | 0xc0     (START high, 64UP high: command executes)
        cmd & 0x7f     (START low again)

    With 64UP low the chip would play one of 64 fixed phrases from the
    internal ROM on its own (T6803 stand-alone mode).  The Socrates never
    does this, it is only logged.

    The host reads back (on the Socrates through I/O port 0x4x):

        D0-D3   the nibble latched by the last DTRD command
        busy    high from START until the end-of-speech code has been
                played (port 0x4x bit 7)

    Commands (SEL5..SEL0):

        00 nnnn  ADLD   load the next address nibble, least significant
                        nibble first; the BIOS always loads 5 nibbles
                        (20 bit byte address)
        01 nnnn  CNDT   load the next condition nibble, low nibble first,
                        2 nibbles in total
        10 xxxx  DTRD   read one nibble of speech ROM: the first DTRD after
                        any other command is a dummy read (it only turns the
                        bus around and returns the stale latch); every
                        further DTRD latches the next nibble, low nibble of
                        each byte first, and advances the pointer.  The
                        nibble is read while START is still high.
        11 0xxx  START  start speaking at the loaded address
        11 1xxx  RESET  stop speaking, clear the address, condition and the
                        nibble counters

    Any command other than ADLD (CNDT) restarts the ADLD (CNDT) nibble
    counter at the low nibble.


    How the Socrates BIOS uses the chip
    ===================================

    (traced from the BIOS; letters = command, digit = nibble, "=n" = nibble
    read back)

    1. Module detection.  Condition 00 (internal ROM), address 0, read the
       first three bytes and compare them with the signature:

           CNDT 0 0, ADLD 0 0 0 0 0, DTRD (dummy),
           DTRD =5 =A =3 =5 =F =4         -> A5 53 4F
           RESET

       The internal ROM of both known modules starts with
       A5 "SOCRATES,VTEL88".  If the signature does not match, the BIOS
       runs without speech.

    2. Phrase lookup.  The phrase table lives at the start of the external
       space (VSM 1), 3 bytes per phrase: start address (little endian
       byte address) and the condition byte to use.  For phrase n:

           CNDT 4 0, ADLD (3n, 5 nibbles), DTRD (dummy), 6 x DTRD, RESET

       e.g. phrase 62 of the English module (spoken by the title screen)
       reads 29 0E 4C = address 0x0E29, condition 0x4C.

    3. Speaking.

           CNDT (condition, 2 nibbles), ADLD (address, 5 nibbles), START

       then the BIOS waits for busy to go high and later for it to drop.

    Known tables: English 210 phrases (49 in the internal ROM, 161 in the
    VSMs, conditions 08/0C/48/4C), German 216 entries (146 internal, 70
    external, conditions 08/0C; phrases that do not exist in German point
    to the first internal phrase at 0x0010).

    Because the phrase table, the phrase data and the signature all live
    in the speech ROMs, another language module (e.g. French) works
    without any change to this device once its internal ROM and VSMs are
    dumped.


    Condition register
    ==================

    bit 0   98 bit frames (T6803 name; no known data uses it)
    bit 1   melody mode on the T6803 (never set)
    bit 2   0 = internal mask ROM, 1 = external VSM bus         (verified)
    bit 3   voiced excitation source; always set in known data
            (guess: B = glottal pulse; clear = A, triangle)
    bit 4   LPC-8 (8 filter stages); never set
    bit 5   unused, never set
    bit 6   1 = 10 ms frames, 0 = 20 ms frames
            (verified against a recording of a real Socrates)
    bit 7   50 bit frames (no k9/k10); never set


    Frame format
    ============

    Speech data is a bit stream, bits taken LSB first from each byte,
    every field stored LSB first.  A frame starts with a 7 bit header:

        bit 0     R   repeat: keep the filter coefficients
        bit 1     K   keep pitch: the frame has no pitch field
        bits 2-6  E   energy (5 bits)

    followed by, depending on the header:

        header 0x00          silence                       7 bits
        header 0x7f          end of speech                 7 bits
        R=1 K=0              P                            14 bits
        R=1 K=1              (nothing)                     7 bits
        R=0 K=0  P != 0      P, k1-k10                    56 bits
        R=0 K=0  P == 0      P, k1-k4 (unvoiced)          35 bits
        R=0 K=1              k1-k10 (pitch kept)          49 bits

        P    7 bits  pitch period in samples, modulo 128 (0 = unvoiced)
        k1   7 bits
        k2   5 bits
        k3   5 bits
        k4   4 bits
        k5   4 bits
        k6   4 bits
        k7   4 bits
        k8   3 bits
        k9   3 bits
        k10  3 bits

    Unvoiced frames set k5-k10 to zero.  The K bit is used heavily by the
    German data (about one frame in 14) and only rarely by the English
    data; with it every phrase of both modules parses exactly up to the
    start of the following phrase.  Ignoring it was the cause of the
    choppy German speech in earlier versions of this emulation.

    Pitch: real data uses the codes 45-127 and 1-16, i.e. the period wraps
    past 127.  Codes 1-31 are therefore taken as periods 129-159.  Pitch
    period = code samples at 10 kHz was verified by matching the pitch
    contours of the phrases against the recording (36 different phrases
    found, 165 occurrences).


    Quantizer tables
    ================

    The tables of the chip are not known.  They were measured:

    - The phrases found in the recording were aligned frame by frame.
    - For every frame the spectral envelope of the recording (cepstrally
      smoothed) was compared with the envelope of the lattice filter given
      by the frame's coefficients.  Per frame gain and the mean spectral
      difference (the response of the TV/recording chain) were removed.
    - Starting from the Toshiba T6721A tables, every table entry was
      optimized to minimize that difference, then the tables were made
      monotonic (weighted isotonic regression) and smoothed.

    All ten tables decrease with the code.  Spectral error, normalized:
    T6721A tables 0.56, T6721A with k4/k8-k10 reversed 0.35, final tables
    0.26 (cross-checked on frames not used for the fit).  Codes that are
    used fewer than 6 times in the recording keep interpolated values.

    The level of voiced frames follows energy x lattice gain, i.e. the
    energy scales the excitation (as on the TMS5100 family), not the
    output.  The energy steps are about 1.3 dB per code, measured over
    codes 16-30; the lower codes are under the noise of the recording and
    follow the same curve.


    Synthesis
    =========

    10 stage lattice filter, parameters interpolated linearly in 8 steps
    over each frame, voiced excitation is an impulse train shaped by a two
    pole glottal filter, unvoiced excitation a 15 bit LFSR.  Interpolation
    and excitation are not verified and follow the T6721A emulation.


    TODO
    ====

    - decap or a datasheet to get the real tables, interpolation and
      excitation
    - 98 bit frames, LPC-8/50 bit frames, melody and stand-alone mode
      (no known data uses them)
    - what the dummy DTRD returns on hardware

******************************************************************************/

#include "emu.h"
#include "tc8802.h"

#include <algorithm>
#include <cstdlib>
#include <iterator>

#define LOG_CMD    (1U << 1)
#define LOG_FRAME  (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_CMD | LOG_FRAME)
#include "logmacro.h"

#define LOGCMD(...)   LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGFRAME(...) LOGMASKED(LOG_FRAME, __VA_ARGS__)


DEFINE_DEVICE_TYPE(TC8802, tc8802_device, "tc8802", "Toshiba TC8802AF")

namespace {

constexpr unsigned K_BITS[10] = { 7, 5, 5, 4, 4, 4, 4, 3, 3, 3 };
constexpr unsigned K_OFFSET[10] = { 0, 128, 160, 192, 208, 224, 240, 256, 264, 272 };

// energy (frame header bits 2-6), 1.3 dB per step, measured against a
// recording of a real Socrates (see above); code 31 is never used
constexpr int16_t ENERGY[32] =
{
	0, 46, 53, 62, 72, 84, 97, 113, 131, 152, 177, 205, 238, 277, 322, 374,
	434, 504, 585, 680, 790, 917, 1065, 1237, 1437, 1669, 1938, 2251, 2614, 3036, 3527, 4096
};
constexpr int32_t ENERGY_MAX = 4096;

// reflection coefficients, value / 16384, measured (see above)
constexpr int16_t COEFFICIENTS[280] =
{
	// k1 (7 bits)
	2971, 2708, 2384, 2060, 1736, 1136, 464, 128, 128, 96, 32, -448, -747, -1045, -1344, -1643,
	-1941, -2240, -2456, -2511, -2566, -2621, -3530, -4106, -4260, -4331, -4589, -5185, -5615, -5650, -5805, -6226,
	-7209, -7537, -7864, -8192, -8192, -8289, -8500, -8847, -9216, -9592, -10039, -10486, -10486, -10525, -10564, -10628,
	-10855, -11082, -11310, -11484, -11578, -11673, -11758, -11777, -11796, -12386, -12679, -12748, -12817, -13105, -13435, -13521,
	-13607, -13693, -13780, -13857, -13902, -13946, -13991, -14036, -14196, -14425, -14654, -14731, -14772, -14814, -14855, -14896,
	-14931, -14967, -15002, -15038, -15073, -15073, -15401, -15401, -15401, -15458, -15515, -15583, -15656, -15729, -15729, -15729,
	-15729, -15729, -15733, -15745, -16032, -16032, -16056, -16060, -16064, -16064, -16064, -16072, -16088, -16128, -16192, -16224,
	-16225, -16230, -16236, -16241, -16247, -16253, -16259, -16265, -16271, -16276, -16282, -16288, -16294, -16300, -16302, -16302,
	// k2 (5 bits)
	15104, 14600, 14097, 13626, 13530, 13435, 11141, 11141, 10838, 10616, 10486, 8847, 8847, 8192, 6881, 5898,
	5424, 4915, 3932, 3932, 2621, 1311, 388, -535, -1147, -1700, -2252, -3909, -6259, -8608, -9320, -9664,
	// k3 (5 bits)
	9696, 7512, 3604, 3075, 2545, 2058, 1644, 1231, 801, 351, -98, -524, -918, -1311, -1311, -1311,
	-1545, -1638, -1638, -3277, -3490, -3702, -4352, -5243, -7209, -7537, -9175, -9175, -10158, -10813, -11141, -12452,
	// k4 (4 bits)
	9856, 5898, 5898, 5898, 4915, 4588, 4260, 2621, 2294, 328, -328, -1966, -3277, -4588, -4915, -5243,
	// k5 (4 bits)
	7209, 3932, 3343, 2786, 2267, 1564, 328, -328, -655, -1966, -2573, -3112, -3604, -3638, -3662, -3662,
	// k6 (4 bits)
	7864, 7209, 7209, 5571, 5571, 4260, 3932, 1638, 937, 236, -218, -665, -1112, -1132, -1132, -1132,
	// k7 (4 bits)
	3604, 3105, 2469, 1638, 1638, 1147, 706, 328, 0, -1966, -2298, -2515, -2621, -3604, -5376, -5376,
	// k8 (3 bits)
	3604, 1890, 959, 655, -1311, -1638, -1966, -5120,
	// k9 (3 bits)
	3604, 2621, 1311, -328, -655, -655, -2949, -4352,
	// k10 (3 bits)
	2737, 2464, 1966, 0, -359, -729, -1184, -1638
};

constexpr int32_t PULSE_AMP = 6523;

} // anonymous namespace


tc8802_device::tc8802_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TC8802, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_rom_interface(mconfig, *this),
	m_internal_rom(*this, "internal"),
	m_stream(nullptr)
{
}

void tc8802_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock() / 80);

	m_last_write = 0;

	save_item(NAME(m_last_write));
	save_item(NAME(m_condition));
	save_item(NAME(m_cndt_count));
	save_item(NAME(m_adld_count));
	save_item(NAME(m_address));
	save_item(NAME(m_read_nibble));
	save_item(NAME(m_dummy_read));
	save_item(NAME(m_data_latch));
	save_item(NAME(m_speaking));
	save_item(NAME(m_stopping));
	save_item(NAME(m_bit_address));
	save_item(NAME(m_pitch_code));
	save_item(NAME(m_sample));
	save_item(NAME(m_pitch_count));
	save_item(NAME(m_noise));
	save_item(NAME(m_glottal));
	save_item(NAME(m_kidx));
	save_item(NAME(m_kvalid));
	save_item(NAME(m_previous));
	save_item(NAME(m_target));
	save_item(NAME(m_delay));
}

void tc8802_device::device_reset()
{
	m_condition = 0;
	m_cndt_count = 0;
	m_adld_count = 0;
	m_address = 0;
	m_read_nibble = 0;
	m_dummy_read = true;
	m_data_latch = 0;
	m_noise = 1;
	stop_speech();
}

void tc8802_device::device_clock_changed()
{
	if (m_stream)
		m_stream->set_sample_rate(clock() / 80);
}

void tc8802_device::rom_bank_pre_change()
{
	if (m_stream)
		m_stream->update();
}


//-------------------------------------------------
//  host interface
//-------------------------------------------------

void tc8802_device::write(uint8_t data)
{
	bool const strobe = BIT(data, 7) && !BIT(m_last_write, 7);
	m_last_write = data;
	if (!strobe)
		return;

	if (!BIT(data, 6))
	{
		// stand-alone mode: speak fixed phrase SEL0-5
		logerror("stand-alone mode phrase %02X not supported\n", data & 0x3f);
		return;
	}

	execute_command(data & 0x3f);
}

void tc8802_device::execute_command(uint8_t sel)
{
	uint8_t const cmd = sel >> 4;

	// any other command ends a sequence of nibble loads/reads
	if (cmd != 0)
		m_adld_count = 0;
	if (cmd != 1)
		m_cndt_count = 0;
	if (cmd != 2)
		m_dummy_read = true;

	switch (cmd)
	{
	case 0: // ADLD
		if (m_adld_count < 5)
		{
			m_address = (m_address & ~(0xfU << (m_adld_count * 4))) | (uint32_t(sel & 0x0f) << (m_adld_count * 4));
			m_adld_count++;
		}
		m_read_nibble = m_address << 1;
		LOGCMD("ADLD %X (address %05X)\n", sel & 0x0f, m_address);
		break;

	case 1: // CNDT
		if (m_cndt_count < 2)
		{
			m_condition = (m_condition & ~(0xf << (m_cndt_count * 4))) | ((sel & 0x0f) << (m_cndt_count * 4));
			m_cndt_count++;
		}
		LOGCMD("CNDT %X (condition %02X)\n", sel & 0x0f, m_condition);
		break;

	case 2: // DTRD
		if (m_dummy_read)
		{
			// the first read only turns the bus around
			m_dummy_read = false;
		}
		else
		{
			m_data_latch = rom_nibble(m_read_nibble);
			m_read_nibble = (m_read_nibble + 1) & 0x1fffff;
		}
		LOGCMD("DTRD -> %X\n", m_data_latch);
		break;

	case 3:
		m_stream->update();
		if (BIT(sel, 3))
		{
			LOGCMD("RESET\n");
			stop_speech();
			m_condition = 0;
			m_address = 0;
			m_read_nibble = 0;
		}
		else
		{
			LOGCMD("START %s %05X condition %02X\n", (m_condition & CNDT_EXTROM) ? "external" : "internal", m_address, m_condition);
			if (m_condition & (CNDT_98BIT | CNDT_MELODY))
				logerror("unsupported condition %02X\n", m_condition);
			start_speech();
		}
		break;
	}
}

uint8_t tc8802_device::data_r()
{
	return m_data_latch;
}

int tc8802_device::busy_r()
{
	m_stream->update();
	return m_speaking ? 1 : 0;
}


//-------------------------------------------------
//  speech ROM access
//-------------------------------------------------

uint8_t tc8802_device::rom_nibble(uint32_t nibble_address)
{
	uint32_t const byte_address = nibble_address >> 1;
	uint8_t const data = (m_condition & CNDT_EXTROM)
			? read_byte(byte_address & 0xfffff)
			: m_internal_rom[byte_address & (m_internal_rom.length() - 1)];
	return BIT(nibble_address, 0) ? (data >> 4) : (data & 0x0f);
}

int tc8802_device::read_bit()
{
	int const bit = BIT(rom_nibble(m_bit_address >> 2), m_bit_address & 3);
	m_bit_address = (m_bit_address + 1) & 0x7fffff;
	return bit;
}

unsigned tc8802_device::read_bits(unsigned count)
{
	unsigned value = 0;
	for (unsigned i = 0; i < count; i++)
		value |= read_bit() << i;
	return value;
}


//-------------------------------------------------
//  speech synthesis
//-------------------------------------------------

void tc8802_device::start_speech()
{
	stop_speech();
	m_speaking = true;
	m_bit_address = m_address << 3;
	if (!parse_frame())
		m_stopping = true;
}

void tc8802_device::stop_speech()
{
	m_speaking = false;
	m_stopping = false;
	m_sample = 0;
	m_pitch_count = 0;
	m_pitch_code = 0;
	m_glottal[0] = m_glottal[1] = 0;
	std::fill(std::begin(m_kidx), std::end(m_kidx), 0);
	std::fill(std::begin(m_kvalid), std::end(m_kvalid), false);
	std::fill(std::begin(m_previous), std::end(m_previous), 0);
	std::fill(std::begin(m_target), std::end(m_target), 0);
	std::fill(std::begin(m_delay), std::end(m_delay), 0);
}

// read the next frame and make it the new interpolation target;
// returns false on end of speech
bool tc8802_device::parse_frame()
{
	std::copy(std::begin(m_target), std::end(m_target), std::begin(m_previous));
	m_sample = 0;

	unsigned const e = read_bits(7);
	if (e == 0x7f)
	{
		LOGFRAME("%05X: end of speech\n", m_bit_address >> 3);
		m_target[0] = 0;
		return false;
	}
	if (e == 0)
	{
		LOGFRAME("%05X: silence\n", m_bit_address >> 3);
		m_target[0] = 0;
		return true;
	}

	bool const repeat = BIT(e, 0);
	bool const keep_pitch = BIT(e, 1);
	if (!keep_pitch)
		m_pitch_code = read_bits(7);
	unsigned const p = m_pitch_code;
	m_target[0] = ENERGY[e >> 2];
	m_target[1] = p ? (((p - 32) & 0x7f) + 32) : 0;

	if (!repeat)
	{
		unsigned const count = !p ? 4 : stages();
		for (unsigned i = 0; i < STAGES; i++)
		{
			m_kvalid[i] = i < count;
			if (m_kvalid[i])
				m_kidx[i] = read_bits(K_BITS[i]);
		}
	}
	for (unsigned i = 0; i < STAGES; i++)
		m_target[i + 2] = m_kvalid[i] ? COEFFICIENTS[K_OFFSET[i] + m_kidx[i]] : 0;

	LOGFRAME("%05X: %s%c E=%2d P=%3d k=%d %d %d %d %d %d %d %d %d %d\n", m_bit_address >> 3,
			repeat ? "repeat  " : p ? "voiced  " : "unvoiced", keep_pitch ? '*' : ' ', e >> 2, p,
			m_kidx[0], m_kidx[1], m_kidx[2], m_kidx[3], m_kidx[4], m_kidx[5], m_kidx[6], m_kidx[7], m_kidx[8], m_kidx[9]);
	return true;
}

int32_t tc8802_device::next_sample()
{
	unsigned const samples = frame_samples();
	if (m_sample >= samples)
	{
		if (m_stopping)
		{
			stop_speech();
			return 0;
		}
		if (!parse_frame())
			m_stopping = true;
	}

	// interpolate in 8 steps over the frame
	unsigned const fraction = std::min(INTERP_STEPS, unsigned(m_sample) * INTERP_STEPS / samples);
	int32_t values[STAGES + 2];
	for (unsigned i = 0; i < STAGES + 2; i++)
		values[i] = (m_previous[i] * int(INTERP_STEPS - fraction) + m_target[i] * int(fraction)) / int(INTERP_STEPS);
	m_sample++;

	// excitation
	int32_t output;
	m_noise = (m_noise >> 1) | ((BIT(m_noise, 0) ^ BIT(m_noise, 1)) << 14);
	if (!m_target[1])
	{
		output = BIT(m_noise, 0) ? 4096 : -4096;
	}
	else
	{
		// don't sweep the pitch up from 0 at the start of a voiced segment
		unsigned const period = std::max(1, m_previous[1] ? values[1] : m_target[1]);
		m_pitch_count = (m_pitch_count + 1) % period;
		if (m_condition & CNDT_SOURCE_B)
		{
			int32_t const glottal = (m_pitch_count ? 0 : PULSE_AMP) + (m_glottal[0] * 778 - m_glottal[1] * 148) / 1024;
			m_glottal[1] = m_glottal[0];
			m_glottal[0] = glottal;
			output = glottal;
		}
		else
		{
			output = 4096 - std::abs(int(m_pitch_count * 16384 / period) - 8192);
		}
	}
	output = output * values[0] / ENERGY_MAX;

	// lattice filter
	int const count = stages();
	for (int i = count - 1; i >= 0; i--)
	{
		output = std::clamp(output - int32_t(int64_t(values[i + 2]) * m_delay[i] / 16384), -16384, 16383);
		if (i + 1 < count)
			m_delay[i + 1] = m_delay[i] + int32_t(int64_t(values[i + 2]) * output / 16384);
	}
	m_delay[0] = output;

	return output;
}

void tc8802_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
		stream.put_int(0, i, m_speaking ? (next_sample() & ~3) : 0, 16384);
}
