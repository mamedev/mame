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
lenghts.


Decoding is simpler.  The 128-samples waveform is decoded using the
adpcm data and mirroring/zeroing as needed in the voiced case.  The
pitch is taken into account by modulating a 1MHz (clock/4) signal at
the pitch frequency multiplied by 128.  pitch_vals in is practice this
modulation interval, hence its 128us base unit to compute the pitch
period.


        - Rom organization

The rom starts with a vector of 16-bits little endian values which are
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


typedef struct {
	const UINT8 *rom;
	device_t *device;
	sound_stream *stream;

	// Port/lines state
	UINT8 data, cs, cms, wr, intr;

	// Current decoding state
	UINT16 bpos, apos;
	UINT8 mode, cur_segment, cur_repeat, segments, repeats;
	UINT8 prev_pitch, pitch, pitch_pos;
	UINT8 stop_after, cur_dac, cur_bits;

	// Zero-range size
	UINT32 zero_count; // 0 for done

	// Waveform and current index in it
	UINT8 dac_index; // 128 for done
	INT16 dac[128];

} digitalker;

// Quantized intensity values, first index is the volume, second the
// intensity (positive half only, real value goes -8..7)
static const short pcm_levels[8][8] = {
	{  473,  945,  1418,  1890,  2363,  2835,  3308,  3781 },
	{  655, 1310,  1966,  2621,  3276,  3931,  4586,  5242 },
	{  925, 1851,  2776,  3702,  4627,  5553,  6478,  7404 },
	{ 1249, 2498,  3747,  4996,  6245,  7494,  8743,  9992 },
	{ 1638, 3276,  4914,  6552,  8190,  9828, 11466, 13104 },
	{ 2252, 4504,  6757,  9009, 11261, 13514, 15766, 18018 },
	{ 2989, 5979,  8968, 11957, 14947, 17936, 20925, 23915 },
	{ 4095, 8190, 12285, 16380, 20475, 24570, 28665, 32760 },
};

static const int delta1[16] = { -4, -4, -1, -1, -2, -2, 0, 0, 0, 0, 2, 2, 1, 1, 4, 4 };
static const int delta2[16] = { 0, -1, -2, -3, 1, 0, -1, -2, 2, 1, 0, -1, 3, 2, 1, 0 };

// Frequency quantizations, values are in units of 128us.

static const int pitch_vals[32] = {
	97, 95, 92, 89, 87, 84, 82, 80, 77, 75, 73, 71, 69, 67, 65, 63,
	61, 60, 58, 56, 55, 53, 52, 50, 49, 48, 46, 45, 43, 42, 41, 40
};


INLINE digitalker *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == DIGITALKER);
	return (digitalker *)downcast<digitalker_device *>(device)->token();
}


static void digitalker_write(digitalker *dg, UINT8 *adr, UINT8 vol, INT8 dac)
{
	INT16 v;
	dac &= 15;
	if(dac >= 9)
		v = -pcm_levels[vol][15-dac];
	else if(dac)
		v = pcm_levels[vol][dac-1];
	else
		v = 0;
	dg->dac[(*adr)++] = v;
}

static UINT8 digitalker_pitch_next(UINT8 val, UINT8 prev, int step)
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

static void digitalker_set_intr(digitalker *dg, UINT8 intr)
{
	dg->intr = intr;
}

static void digitalker_start_command(digitalker *dg, UINT8 cmd)
{
	dg->bpos = ((dg->rom[cmd*2] << 8) | dg->rom[cmd*2+1]) & 0x3fff;
	dg->cur_segment = dg->segments = dg->cur_repeat = dg->repeats = 0;
	dg->dac_index = 128;
	dg->zero_count = 0;
	digitalker_set_intr(dg, 0);
}

static void digitalker_step_mode_0(digitalker *dg)
{
	INT8 dac = 0;
	int i, k, l;
	UINT8 wpos = 0;
	UINT8 h = dg->rom[dg->apos];
	UINT16 bits = 0x80;
	UINT8 vol = h >> 5;
	UINT8 pitch_id = dg->cur_segment ? digitalker_pitch_next(h, dg->prev_pitch, dg->cur_repeat) : h & 0x1f;

	dg->pitch = pitch_vals[pitch_id];

	for(i=0; i<32; i++)
		dg->dac[wpos++] = 0;

	for(k=1; k != 9; k++) {
		bits |= dg->rom[dg->apos+k] << 8;
		for(l=0; l<4; l++) {
			dac += delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(dg, &wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(dg, &wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		bits = (bits << 8) | (k ? dg->rom[dg->apos+k] : 0x80);
		for(l=3; l>=0; l--) {
			dac -= delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(dg, &wpos, vol, dac);
		}
	}

	for(i=0; i<31; i++)
		dg->dac[wpos++] = 0;

	dg->cur_repeat++;
	if(dg->cur_repeat == dg->repeats) {
		dg->apos += 9;
		dg->prev_pitch = pitch_id;
		dg->cur_repeat = 0;
		dg->cur_segment++;
	}
}

static void digitalker_step_mode_1(digitalker *dg)
{
	logerror("Digitalker mode 1 unsupported");
	dg->zero_count = 1;
	dg->cur_segment = dg->segments;
}

static void digitalker_step_mode_2(digitalker *dg)
{
	INT8 dac = 0;
	int k, l;
	UINT8 wpos=0;
	UINT8 h = dg->rom[dg->apos];
	UINT16 bits = 0x80;
	UINT8 vol = h >> 5;
	UINT8 pitch_id = dg->cur_segment ? digitalker_pitch_next(h, dg->prev_pitch, dg->cur_repeat) : h & 0x1f;

	dg->pitch = pitch_vals[pitch_id];

	for(k=1; k != 9; k++) {
		bits |= dg->rom[dg->apos+k] << 8;
		for(l=0; l<4; l++) {
			dac += delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(dg, &wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(dg, &wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		int limit = k ? 0 : 1;
		bits = (bits << 8) | (k ? dg->rom[dg->apos+k] : 0x80);
		for(l=3; l>=limit; l--) {
			dac -= delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(dg, &wpos, vol, dac);
		}
	}

	digitalker_write(dg, &wpos, vol, dac);

	for(k=1; k != 9; k++) {
		int start = k == 1 ? 1 : 0;
		bits |= dg->rom[dg->apos+k] << 8;
		for(l=start; l<4; l++) {
			dac += delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(dg, &wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(dg, &wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		int limit = k ? 0 : 1;
		bits = (bits << 8) | (k ? dg->rom[dg->apos+k] : 0x80);
		for(l=3; l>=limit; l--) {
			dac -= delta1[(bits >> (6+2*l)) & 15];
			digitalker_write(dg, &wpos, vol, dac);
		}
	}

	dg->cur_repeat++;
	if(dg->cur_repeat == dg->repeats) {
		dg->apos += 9;
		dg->prev_pitch = pitch_id;
		dg->cur_repeat = 0;
		dg->cur_segment++;
	}
}

static void digitalker_step_mode_3(digitalker *dg)
{
	UINT8 h = dg->rom[dg->apos];
	UINT8 vol = h >> 5;
	UINT16 bits;
	UINT8 dac, apos, wpos;
	int k, l;

	dg->pitch = pitch_vals[h & 0x1f];
	if(dg->cur_segment == 0 && dg->cur_repeat == 0) {
		dg->cur_bits = 0x40;
		dg->cur_dac = 0;
	}
	bits = dg->cur_bits;
	dac = 0;

	apos = dg->apos + 1 + 32*dg->cur_segment;
	wpos = 0;
	for(k=0; k != 32; k++) {
		bits |= dg->rom[apos++] << 8;
		for(l=0; l<4; l++) {
			dac += delta2[(bits >> (6+2*l)) & 15];
			digitalker_write(dg, &wpos, vol, dac);
		}
		bits >>= 8;
	}

	dg->cur_bits = bits;
	dg->cur_dac = dac;

	dg->cur_segment++;
	if(dg->cur_segment == dg->segments) {
		dg->cur_segment = 0;
		dg->cur_repeat++;
	}
}

static void digitalker_step(digitalker *dg)
{
	if(dg->cur_segment == dg->segments || dg->cur_repeat == dg->repeats) {
		if(dg->stop_after == 0 && dg->bpos == 0xffff)
			return;
		if(dg->stop_after == 0) {
			UINT8 v1 = dg->rom[dg->bpos++];
			UINT8 v2 = dg->rom[dg->bpos++];
			UINT8 v3 = dg->rom[dg->bpos++];
			dg->apos = v2 | ((v3 << 8) & 0x3f00);
			dg->segments = (v1 & 15) + 1;
			dg->repeats = ((v1 >> 4) & 7) + 1;
			dg->mode = (v3 >> 6) & 3;
			dg->stop_after = (v1 & 0x80) != 0;

			dg->cur_segment = 0;
			dg->cur_repeat = 0;

			if(!dg->apos) {
				dg->zero_count = 40*128*dg->segments*dg->repeats;
				dg->segments = 0;
				dg->repeats = 0;
				return;
			}
		} else if(dg->stop_after == 1) {
			dg->bpos = 0xffff;
			dg->zero_count = 81920;
			dg->stop_after = 2;
			dg->cur_segment = 0;
			dg->cur_repeat = 0;
			dg->segments = 0;
			dg->repeats = 0;
		} else {
			dg->stop_after = 0;
			digitalker_set_intr(dg, 1);
		}
	}

	switch(dg->mode) {
	case 0: digitalker_step_mode_0(dg); break;
	case 1: digitalker_step_mode_1(dg); break;
	case 2: digitalker_step_mode_2(dg); break;
	case 3: digitalker_step_mode_3(dg); break;
	}
	if(!dg->zero_count)
		dg->dac_index = 0;
}

static STREAM_UPDATE(digitalker_update)
{
	digitalker *dg = (digitalker *)param;
	stream_sample_t *sout = outputs[0];
	int cpos = 0;
	while(cpos != samples) {
		if(dg->zero_count == 0 && dg->dac_index == 128)
			digitalker_step(dg);

		if(dg->zero_count) {
			int n = samples - cpos;
			int i;
			if(n > dg->zero_count)
				n = dg->zero_count;
			for(i=0; i != n; i++)
				sout[cpos++] = 0;
			dg->zero_count -= n;

		} else if(dg->dac_index != 128) {
			while(cpos != samples && dg->dac_index != 128) {
				short v = dg->dac[dg->dac_index];
				int pp = dg->pitch_pos;
				while(cpos != samples && pp != dg->pitch) {
					sout[cpos++] = v;
					pp++;
				}
				if(pp == dg->pitch) {
					pp = 0;
					dg->dac_index++;
				}
				dg->pitch_pos = pp;
			}

		} else {
			while(cpos != samples)
				sout[cpos++] = 0;
		}
	}
}

static void digitalker_cs_w(digitalker *dg, int line)
{
	UINT8 cs = line == ASSERT_LINE ? 1 : 0;
	if(cs == dg->cs)
		return;
	dg->cs = cs;
	if(cs)
		return;
	if(!dg->wr) {
		if(dg->cms)
			digitalker_set_intr(dg, 1);
		else
			digitalker_start_command(dg, dg->data);
	}
}

static void digitalker_cms_w(digitalker *dg, int line)
{
	dg->cms = line == ASSERT_LINE ? 1 : 0;
}

static void digitalker_wr_w(digitalker *dg, int line)
{
	UINT8 wr = line == ASSERT_LINE ? 1 : 0;
	if(wr == dg->wr)
		return;
	dg->wr = wr;
	if(wr || dg->cs)
		return;
	if(dg->cms)
		digitalker_set_intr(dg, 1);
	else
		digitalker_start_command(dg, dg->data);
}

static int digitalker_intr_r(digitalker *dg)
{
	return dg->intr ? ASSERT_LINE : CLEAR_LINE;
}

static void digitalker_register_for_save(digitalker *dg)
{
	dg->device->save_item(NAME(dg->data));
	dg->device->save_item(NAME(dg->cs));
	dg->device->save_item(NAME(dg->cms));
	dg->device->save_item(NAME(dg->wr));
	dg->device->save_item(NAME(dg->intr));
	dg->device->save_item(NAME(dg->bpos));
	dg->device->save_item(NAME(dg->apos));
	dg->device->save_item(NAME(dg->mode));
	dg->device->save_item(NAME(dg->cur_segment));
	dg->device->save_item(NAME(dg->cur_repeat));
	dg->device->save_item(NAME(dg->segments));
	dg->device->save_item(NAME(dg->repeats));
	dg->device->save_item(NAME(dg->prev_pitch));
	dg->device->save_item(NAME(dg->pitch));
	dg->device->save_item(NAME(dg->pitch_pos));
	dg->device->save_item(NAME(dg->stop_after));
	dg->device->save_item(NAME(dg->cur_dac));
	dg->device->save_item(NAME(dg->cur_bits));
	dg->device->save_item(NAME(dg->zero_count));
	dg->device->save_item(NAME(dg->dac_index));
	dg->device->save_item(NAME(dg->dac));
}

static DEVICE_START(digitalker)
{
	digitalker *dg = get_safe_token(device);
	dg->device = device;
	dg->rom = device->machine().root_device().memregion(device->tag())->base();
	dg->stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock()/4, dg, digitalker_update);
	dg->dac_index = 128;
	dg->data = 0xff;
	dg->cs = dg->cms = dg->wr = 1;
	dg->bpos = 0xffff;
	digitalker_set_intr(dg, 1);

	digitalker_register_for_save(dg);
}

DEVICE_GET_INFO(digitalker)
{
	switch(state) {
	case DEVINFO_INT_TOKEN_BYTES:	info->i = sizeof(digitalker); break;
	case DEVINFO_FCT_START:			info->start = DEVICE_START_NAME(digitalker); break;
	case DEVINFO_FCT_STOP:      	break;
	case DEVINFO_FCT_RESET:     	break;
	case DEVINFO_STR_NAME:      	strcpy(info->s, "Digitalker"); break;
	case DEVINFO_STR_FAMILY:	strcpy(info->s, "National Semiconductor"); break;
	case DEVINFO_STR_VERSION:	strcpy(info->s, "1.0"); break;
	case DEVINFO_STR_SOURCE_FILE:		strcpy(info->s, __FILE__); break;
	case DEVINFO_STR_CREDITS:	strcpy(info->s, "Copyright Olivier Galibert"); break;
	}
}

void digitalker_0_cs_w(device_t *device, int line)
{
	digitalker *dg = get_safe_token(device);
	digitalker_cs_w(dg, line);
}

void digitalker_0_cms_w(device_t *device, int line)
{
	digitalker *dg = get_safe_token(device);
	digitalker_cms_w(dg, line);
}

void digitalker_0_wr_w(device_t *device, int line)
{
	digitalker *dg = get_safe_token(device);
	digitalker_wr_w(dg, line);
}

int digitalker_0_intr_r(device_t *device)
{
	digitalker *dg = get_safe_token(device);
	return digitalker_intr_r(dg);
}

WRITE8_DEVICE_HANDLER( digitalker_data_w )
{
	digitalker *dg = get_safe_token(device);
	dg->data = data;
}


const device_type DIGITALKER = &device_creator<digitalker_device>;

digitalker_device::digitalker_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, DIGITALKER, "Digitalker", tag, owner, clock),
	  device_sound_interface(mconfig, *this)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(digitalker));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void digitalker_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void digitalker_device::device_start()
{
	DEVICE_START_NAME( digitalker )(this);
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void digitalker_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// should never get here
	fatalerror("sound_stream_update called; not applicable to legacy sound devices\n");
}


