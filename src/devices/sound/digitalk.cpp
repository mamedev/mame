// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "digitalk.h"

/*
  National Semiconductor's Digitalker, also known as MM54104.

This is a sample decompression chip where the codec is very
specialized for speech.

        - Driver history

The history of this driver is a little strange.  The real
reverse-engineering work has been done by Kevin Horton
(single-stepping the chip and everything) with assistance by Lord
Nightmare who had done the work (with help from Mr. Horton) on the tsi
s14001a, a predecessor of the digitalker.  Mr. Horton was not
interested in publishing his findings, but provided full-rate
resynthesized samples for the game scorpion.  This driver is the
result of analyzing these samples.


        - The Chip

Pinout from chipdir, added there by Agustin Yado.  Added rompwr.
Package is DIP40.  Standard osc is 4MHz, maximum is 5Mhz.

            +--()--+
     osc in | 1  40| vdd
    osc out | 2  39| speech out
         cs | 3  38| adr 13
         wr | 4  37| adr 12
     rompwr | 5  36| adr 11
       intr | 6  35| adr 10
        cms | 7  34| adr 9
         d0 | 8  33| adr 8
         d1 | 9  32| adr 7
         d2 |10  31| adr 6
         d3 |11  30| adr 5
         d4 |12  29| adr 4
         d5 |13  28| adr 3
         d6 |14  27| adr 2
         d7 |15  26| adr 1
    rdata 0 |16  25| adr 0
    rdata 1 |17  24| rdata 7
    rdata 2 |18  23| rdata 6
    rdata 3 |19  22| rdata 5
        vss |20  21| rdata 4
            +------+

Pin functions, excerpt from
http://www.ski.org/Rehab/sktf/vol06no1Winter1985.html, slightly modified
"Smith-Kettlewell Technical File, Vol 6, No 1, winter 1985"

    On the controller chip, pin 40 is VCC, while pin 20 is ground. VCC
    for this chip is between 7 and 11 VDC, and pin 40 is bypassed to
    pin 20 by 0.1uF. Maximum current is listed at 45mA.

    Pin 3 is called "Chip Select Not," and can be taken high to "open"
    the input address and control lines. This is used in cases where
    the Digitalker is connected to a computer bus, and the address
    lines need to be floated while the bus is doing something else. In
    other words, taking pin 3 high makes the Digitalker turn a deaf
    ear to all of its inputs.

    Pin 4 is "Write Not," and, as mentioned before, is brought low to
    load an address into the controller, then brought high again to
    start speech. In other words, this is the pin by which you
    "trigger" the Digitalker.

    Pin 5 is "Not ROM-Power Enable," an output which can be used to
    control the power to the ROM's. This is used in cases of battery
    supply where current drain is important; the ROM's will have their
    power controlled by the controller.

    Pin 6 is the "Interrupt Output," (equivalent to the "Busy Line" of
    the old TSI Speech Board); this line goes low when an address is
    loaded into the chip, then goes high again when speech is
    finished. This signal can be used to control the driver circuitry
    (or other controlling device), in which case it tells the driver
    to "Hold the phone!" while the speech is running. Pin 7 is called
    "CMS," and its state controls the action of the "Write Not"
    line. With pin 7 low, the operation of pin 4 is as described. If
    pin 7 is brought high, raising pin 4 high after loading an address
    serves only to reset the interrupt and does not start speech. This
    facility is probably intended for use where the interrupt line
    really controls the hardware interrupt of a computer, and where
    the program taking care of the interrupt may not have another word
    to say every time the Digitalker is finished. I have found no
    particular use for pin 7, and I simply ground it for normal
    operation.

    Pins 8 through 15 are the eight input address lines, with pin 8
    being the most significant BIT and pin 15 being least
    significant. These address lines are "active high." They should
    never be left open. They are TTL-compatible; this means that logic
    low is ground and logic high is plus 5VDC. (Actually, being MOS
    inputs, you can take them as high as the VCC on the controller,
    but a 5V supply is required for the ROM's anyhow -- it's there if
    you want to use 5V.)

    Pins 16 through 24 are the eight data lines which bring data from
    the ROMs to the controller, with pin 16 being called "ROM Data
    1," and pin 24 being "ROM Data 8."

    Pins 25 through 38 are the fourteen address lines which select
    location in the ROM's to be read by the controller. Pin 25 is
    "Address 0," pin 38 is "Address 13."


        - Codec

The codec stems from the standard model for voiced speech generation:
a stream of impulses at the pitch frequency followed by an
articulation filter.  Both of those are considered slowly varying.

        pitch         filter  voiced sound
        ||||||||||  * /\/\  = ~~~~

The first compression effect is by forcing the filter to be
zero-phase.  That makes the periods perfectly symmetrical around the
pitch pulse.  The voiced speech is as a result extracted as a number
of symmetric periods, centered on the pitch pulses.

Following that, two quantizations are done.  First, the pitch
frequency is quantized to one of 32 values (see pitch_vals), going
from ~80 to 200Hz.  Then the volume is selected among 8 possible
values in an exponential scale, and the amplitudes are quantized as a
4-bit signed value.  The period is time-warped to make it exactly 128
samples long.

The next step of the compression is to select which harnomics will be
kept.  The choices are to keep only the even ones or only the odd
ones.  Dropping half the harmonics allow to encode the period in only
32 samples, using the fact that a period, for a zero-phase-at-center,
half-harmonics signal, looks like:

   even harmonics: /\/\     odd harmonics:  _/\_

Where / = block of 32 samples
      \ = same block reversed
      _ = 32 zeroes

So we're left with 32 4-bit samples to encode, which is done using a
2-bit adpcm.  The adaptative part is done by using a fixed 16-deltas
table indexed by the current and the previous encoded value.

Added to all that is the possibility of repeating such a period while
increasing or decreasing the pitch frequency.


For non-voiced speech or non-speech an alternative mode is available
where an equivalent period cutting, frequency and amplitude
quantization is done, but the whole 128 samples are adpcm-encoded.


Finally, silent zones are compressed specifically by storing their
lengths.


Decoding is simpler.  The 128-samples waveform is decoded using the
adpcm data and mirroring/zeroing as needed in the voiced case.  The
pitch is taken into account by modulating a 1MHz (clock/4) signal at
the pitch frequency multiplied by 128.  pitch_vals in is practice this
modulation interval, hence its 128us base unit to compute the pitch
period.


        - Rom organization

The rom starts with a vector of 16-bits big endian values which are
the addresses of the segments table for the samples.  The segments data
is a vector of 24-bits little-endian values organized as such:

      adr+2    adr+1    adr
      MMAAAAAA AAAAAAAA ERRRSSSS

  M: Segment base waveforms compression mode (0-3)
  A: Segment base waveforms data address (0-16383)
  R: Repeat count (1-8)
  S: Number of waveforms (1-16)
  E: Last segment of the sample (flag)

Decoding stops after having decoded a segment with the E bit set.  A
final 8.192ms silence is systematically added.

A == 0 means silence.  Duration is 5.12ms*(R+1)*(S+1), or in other
terms a full decode of all-zero waveforms at maximal pitch frequency
(pitch code 31).


A != 0 means sound.  The sound data starts at that offset.  The
encoding method is selected with M:

  0: odd-harmonics voiced mode
  2: even-harmonics voiced mode
  3: unvoiced/non-speech mode

Mode 1 is not supported because it is not present in the available
samples, hence unknown.


  Voiced mode (9 bytes/waveform):

        VVVPPPPP AAAAAAAAx8     - First waveform
        VVVDCCCC AAAAAAAAx8     - Following waveforms

V: Volume (first index in pcm_levels)
P: Pitch index
A: adpcm data
D: Pitch index change direction (0=increase, 1=decrease)
C: Pitch index maximum change

The waveforms are encoded with a 2-bit adpcm, lowest pair of bits
first.  Deltas are a size-16 vector, indexed with the previous adpcm
value in bits 0&1 and the current in bits 2&3.  Voiced speech modes
use table delta1 and initial "previous" value 2.

Each waveform is repeated R times at volume V.  First waveform has
fixed pitch P.  Subsequent waveforms change the pitch index by 1 every
repeat (including the first) up to a change of C.  D indicates whether
it's an increment or a decrement.


  Unvoiced mode (33 bytes/waveform):

        VVVPPPPP AAAAAAAAx32    - All waveforms

V: Volume (first index in pcm_levels)
P: Pitch index
A: adpcm data

Adpcm encoding is identical but using delta2 table and an initial
value of 1.  Every waveform is played consecutively and the adpcm
previous value or dac level is not reset between waveforms.  The
complete set of waveforms is repeated R times.

*/

// Quantized intensity values, first index is the volume, second the
// intensity (positive half only, real value goes -8..7)
static constexpr short pcm_levels[8][8] = {
	{  473,  945,  1418,  1890,  2363,  2835,  3308,  3781 },
	{  655, 1310,  1966,  2621,  3276,  3931,  4586,  5242 },
	{  925, 1851,  2776,  3702,  4627,  5553,  6478,  7404 },
	{ 1249, 2498,  3747,  4996,  6245,  7494,  8743,  9992 },
	{ 1638, 3276,  4914,  6552,  8190,  9828, 11466, 13104 },
	{ 2252, 4504,  6757,  9009, 11261, 13514, 15766, 18018 },
	{ 2989, 5979,  8968, 11957, 14947, 17936, 20925, 23915 },
	{ 4095, 8190, 12285, 16380, 20475, 24570, 28665, 32760 },
};

static constexpr int delta1[16] = { -4, -4, -1, -1, -2, -2, 0, 0, 0, 0, 2, 2, 1, 1, 4, 4 };
static constexpr int delta2[16] = { 0, -1, -2, -3, 1, 0, -1, -2, 2, 1, 0, -1, 3, 2, 1, 0 };

// Frequency quantizations, values are in units of 128us.
static constexpr int pitch_vals[32] = {
	97, 95, 92, 89, 87, 84, 82, 80, 77, 75, 73, 71, 69, 67, 65, 63,
	61, 60, 58, 56, 55, 53, 52, 50, 49, 48, 46, 45, 43, 42, 41, 40
};


DEFINE_DEVICE_TYPE(DIGITALKER, digitalker_device, "digitalker", "MM54104 Digitalker")

digitalker_device::digitalker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DIGITALKER, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_rom(*this, DEVICE_SELF),
		m_stream(nullptr),
		m_data(0),
		m_cs(0),
		m_cms(0),
		m_wr(0),
		m_intr(0),
		m_bpos(0),
		m_apos(0),
		m_mode(0),
		m_cur_segment(0),
		m_cur_repeat(0),
		m_segments(0),
		m_repeats(0),
		m_prev_pitch(0),
		m_pitch(0),
		m_pitch_pos(0),
		m_stop_after(0),
		m_cur_dac(0),
		m_cur_bits(0),
		m_zero_count(0),
		m_dac_index(0)
{
}


void digitalker_device::digitalker_write(uint8_t *adr, uint8_t vol, int8_t dac)
{
	int16_t v;
	dac &= 15;
	if(dac >= 9)
		v = -pcm_levels[vol][15-dac];
	else if(dac)
		v = pcm_levels[vol][dac-1];
	else
		v = 0;
	m_dac[(*adr)++] = v;
}

uint8_t digitalker_device::digitalker_pitch_next(uint8_t val, uint8_t prev, int step)
{
	int delta, nv;

	delta = val & 0xf;
	if(delta > step + 1)
		delta = step + 1;
	if(val & 0x10)
		delta = -delta;

	nv = prev + delta;
	if(nv < 0)
		nv = 0;
	else if(nv > 31)
		nv = 31;
	return nv;
}

void digitalker_device::digitalker_set_intr(uint8_t intr)
{
	m_intr = intr;
}

void digitalker_device::digitalker_start_command(uint8_t cmd)
{
	m_bpos = ((m_rom[cmd*2] << 8) | m_rom[cmd*2+1]) & 0x3fff;
	m_cur_segment = m_segments = m_cur_repeat = m_repeats = 0;
	m_dac_index = 128;
	m_zero_count = 0;
	digitalker_set_intr(0);
}

void digitalker_device::digitalker_step_mode_0()
{
	int8_t dac = 0;
	int i, k, l;
	uint8_t wpos = 0;
	uint8_t h = m_rom[m_apos];
	uint16_t bits = 0x80;
	uint8_t vol = h >> 5;
	uint8_t pitch_id = m_cur_segment ? digitalker_pitch_next(h, m_prev_pitch, m_cur_repeat) : h & 0x1f;

	m_pitch = pitch_vals[pitch_id];

	for(i=0; i<32; i++)
		m_dac[wpos++] = 0;

	for(k=1; k != 9; k++) {
		bits |= m_rom[m_apos+k] << 8;
		for(l=0; l<4; l++) {
			dac += delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(&wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		bits = (bits << 8) | (k ? m_rom[m_apos+k] : 0x80);
		for(l=3; l>=0; l--) {
			dac -= delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(&wpos, vol, dac);
		}
	}

	for(i=0; i<31; i++)
		m_dac[wpos++] = 0;

	m_cur_repeat++;
	if(m_cur_repeat == m_repeats) {
		m_apos += 9;
		m_prev_pitch = pitch_id;
		m_cur_repeat = 0;
		m_cur_segment++;
	}
}

void digitalker_device::digitalker_step_mode_1()
{
	logerror("Digitalker mode 1 unsupported\n");
	m_zero_count = 1;
	m_cur_segment = m_segments;
}

void digitalker_device::digitalker_step_mode_2()
{
	int8_t dac = 0;
	int k, l;
	uint8_t wpos=0;
	uint8_t h = m_rom[m_apos];
	uint16_t bits = 0x80;
	uint8_t vol = h >> 5;
	uint8_t pitch_id = m_cur_segment ? digitalker_pitch_next(h, m_prev_pitch, m_cur_repeat) : h & 0x1f;

	m_pitch = pitch_vals[pitch_id];

	for(k=1; k != 9; k++) {
		bits |= m_rom[m_apos+k] << 8;
		for(l=0; l<4; l++) {
			dac += delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(&wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		int limit = k ? 0 : 1;
		bits = (bits << 8) | (k ? m_rom[m_apos+k] : 0x80);
		for(l=3; l>=limit; l--) {
			dac -= delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(&wpos, vol, dac);
		}
	}

	digitalker_write(&wpos, vol, dac);

	for(k=1; k != 9; k++) {
		int start = k == 1 ? 1 : 0;
		bits |= m_rom[m_apos+k] << 8;
		for(l=start; l<4; l++) {
			dac += delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(&wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		int limit = k ? 0 : 1;
		bits = (bits << 8) | (k ? m_rom[m_apos+k] : 0x80);
		for(l=3; l>=limit; l--) {
			dac -= delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(&wpos, vol, dac);
		}
	}

	m_cur_repeat++;
	if(m_cur_repeat == m_repeats) {
		m_apos += 9;
		m_prev_pitch = pitch_id;
		m_cur_repeat = 0;
		m_cur_segment++;
	}
}

void digitalker_device::digitalker_step_mode_3()
{
	uint8_t h = m_rom[m_apos];
	uint8_t vol = h >> 5;
	uint16_t bits;
	uint8_t dac, apos, wpos;
	int k, l;

	m_pitch = pitch_vals[h & 0x1f];
	if(m_cur_segment == 0 && m_cur_repeat == 0) {
		m_cur_bits = 0x40;
		m_cur_dac = 0;
	}
	bits = m_cur_bits;
	dac = 0;

	apos = m_apos + 1 + 32*m_cur_segment;
	wpos = 0;
	for(k=0; k != 32; k++) {
		bits |= m_rom[apos++] << 8;
		for(l=0; l<4; l++) {
			dac += delta2[(bits >> (6+2*l)) & 15];
			digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	m_cur_bits = bits;
	m_cur_dac = dac;

	m_cur_segment++;
	if(m_cur_segment == m_segments) {
		m_cur_segment = 0;
		m_cur_repeat++;
	}
}

void digitalker_device::digitalker_step()
{
	if(m_cur_segment == m_segments || m_cur_repeat == m_repeats) {
		if(m_stop_after == 0 && m_bpos == 0xffff)
			return;
		if(m_stop_after == 0) {
			uint8_t v1 = m_rom[m_bpos++];
			uint8_t v2 = m_rom[m_bpos++];
			uint8_t v3 = m_rom[m_bpos++];
			m_apos = v2 | ((v3 << 8) & 0x3f00);
			m_segments = (v1 & 15) + 1;
			m_repeats = ((v1 >> 4) & 7) + 1;
			m_mode = (v3 >> 6) & 3;
			m_stop_after = (v1 & 0x80) != 0;

			m_cur_segment = 0;
			m_cur_repeat = 0;

			if(!m_apos) {
				m_zero_count = 40*128*m_segments*m_repeats;
				m_segments = 0;
				m_repeats = 0;
				return;
			}
		} else if(m_stop_after == 1) {
			digitalker_set_intr(1);
			m_bpos = 0xffff;
			m_zero_count = 81920;
			m_stop_after = 2;
			m_cur_segment = 0;
			m_cur_repeat = 0;
			m_segments = 0;
			m_repeats = 0;
		} else {
			m_stop_after = 0;
		}
	}

	switch(m_mode) {
	case 0: digitalker_step_mode_0(); break;
	case 1: digitalker_step_mode_1(); break;
	case 2: digitalker_step_mode_2(); break;
	case 3: digitalker_step_mode_3(); break;
	}
	if(!m_zero_count)
		m_dac_index = 0;
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void digitalker_device::sound_stream_update(sound_stream &stream)
{
	int cpos = 0;
	while(cpos != stream.samples()) {
		if(m_zero_count == 0 && m_dac_index == 128)
			digitalker_step();

		if(m_zero_count) {
			int n = stream.samples() - cpos;
			if(n > m_zero_count)
				n = m_zero_count;
			stream.fill(0, 0, cpos, n);
			cpos += n;
			m_zero_count -= n;

		} else if(m_dac_index != 128) {
			while(cpos != stream.samples() && m_dac_index != 128) {
				s32 v = m_dac[m_dac_index];
				int pp = m_pitch_pos;
				while(cpos != stream.samples() && pp != m_pitch) {
					stream.put_int(0, cpos++, v, 32768);
					pp++;
				}
				if(pp == m_pitch) {
					pp = 0;
					m_dac_index++;
				}
				m_pitch_pos = pp;
			}

		}
	}
}

void digitalker_device::digitalker_cs_w(int line)
{
	uint8_t cs = line == ASSERT_LINE ? 1 : 0;
	if(cs == m_cs)
		return;
	m_cs = cs;
	if(cs)
		return;
	if(!m_wr) {
		if(m_cms)
			digitalker_set_intr(1);
		else
			digitalker_start_command(m_data);
	}
}

void digitalker_device::digitalker_cms_w(int line)
{
	m_cms = line == ASSERT_LINE ? 1 : 0;
}

void digitalker_device::digitalker_wr_w(int line)
{
	uint8_t wr = line == ASSERT_LINE ? 1 : 0;
	if(wr == m_wr)
		return;
	m_wr = wr;
	if(wr || m_cs)
		return;
	if(m_cms)
		digitalker_set_intr(1);
	else
		digitalker_start_command(m_data);
}

int digitalker_device::digitalker_intr_r()
{
	return m_intr ? ASSERT_LINE : CLEAR_LINE;
}

void digitalker_device::digitalker_register_for_save()
{
	save_item(NAME(m_data));
	save_item(NAME(m_cs));
	save_item(NAME(m_cms));
	save_item(NAME(m_wr));
	save_item(NAME(m_intr));
	save_item(NAME(m_bpos));
	save_item(NAME(m_apos));
	save_item(NAME(m_mode));
	save_item(NAME(m_cur_segment));
	save_item(NAME(m_cur_repeat));
	save_item(NAME(m_segments));
	save_item(NAME(m_repeats));
	save_item(NAME(m_prev_pitch));
	save_item(NAME(m_pitch));
	save_item(NAME(m_pitch_pos));
	save_item(NAME(m_stop_after));
	save_item(NAME(m_cur_dac));
	save_item(NAME(m_cur_bits));
	save_item(NAME(m_zero_count));
	save_item(NAME(m_dac_index));
	save_item(NAME(m_dac));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void digitalker_device::device_start()
{
	m_stream = stream_alloc(0, 1, clock()/4);
	m_dac_index = 128;
	m_data = 0xff;
	m_cs = m_cms = m_wr = 1;
	m_bpos = 0xffff;
	digitalker_set_intr(1);

	digitalker_register_for_save();
}

void digitalker_device::digitalker_0_cs_w(int line)
{
	digitalker_cs_w(line);
}

void digitalker_device::digitalker_0_cms_w(int line)
{
	digitalker_cms_w(line);
}

void digitalker_device::digitalker_0_wr_w(int line)
{
	digitalker_wr_w(line);
}

int digitalker_device::digitalker_0_intr_r()
{
	return digitalker_intr_r();
}

void digitalker_device::digitalker_data_w(uint8_t data)
{
	m_data = data;
}
