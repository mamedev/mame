// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#include "emu.h"
#include "debugger.h"
#include "swp30.h"
#include "cpu/drcumlsh.h"

// TODOs:
//   - 7 bits are still not understood in the MEG instructions
//   - in the lfo, the top of slot 9 is not understood
//   - slot b is not understood but used (and even read at times)
//   - some instruments from the demo don't work well (not sure which)
//   - there seems to be some saturation at times in the demo
//   - lots of control registers are not understood, in particular
//     the 5x ones, one of which is used in the talk mod effect
//   - the timing of communication between the meg registers and the
//     the environment is known but not implemented


//   The SWP30 is the combination of a rompler called AWM2 (Advanced
//   Wave Memory 2) and an effects DSP called MEG (Multiple Effects
//   Generator).  It also includes some routing/mixing capabilities,
//   moving data between AWM2, MEG and serial inputs (MELI) and
//   outputs (MELO) with volume management capabilities everywhere.
//   Its clock is 33.9MHz and the output is at 44100Hz stereo (768
//   cycles per sample pair) per dac output.

//   I/O wise, the chip has 8 generic audio serial inputs and 8
//   outputs for external plugins, and two dac outputs, all
//   stereo. The MU100 connects a stereo ADC to the first input, and
//   routes the third input and output to the plugin board.


//     Registers:

//   The chip interface presents 4096 16-bits registers in a 64x64 grid.
//   They are mostly read/write.  Some of this grid is for per-channel
//   values for AWM2, but parts are isolated and renumbered for MEG
//   registers or for general control functions.


//     AWM2:

//   The AWM2 is in charge of handling the individual channels.  It
//   manages reading the rom, decoding the samples, applying volume and
//   pitch envelopes and lfos and filtering the result.  Each channel is
//   then sent as a mono signal to the mixer for further processing.

//   It is composed of a number of blocks, please refer to the individual
//   documentations further in the file:
//   - streaming
//   - dual special filters
//   - dual iir1 filters
//   - envelope control
//   - lfo

//   The sound data can be four formats (8 bits, 12 bits, 16 bits, and
//   a 8-bits kinda-apdcm format).  The rom bus is 25 bits address and
//   32 bits data wide.  It applies four filters to the sample data in
//   two dual filter blocks.  The first block has two filters
//   configurable between iir1 and chamberlain, lpf, hpf, band or
//   notch, with our without configurable resonance.  The second block
//   is two free iir1 filters.  Envelopes are handled automatically,
//   and the final result is sent to the mixer for panning, volume
//   control and routing.  In addition lfo acts on the pitch and the
//   volume.


//     MEG:

//   The MEG is a DSP with 384 program steps connected to a reverb
//   samples ram.  It computes all the effects and sends to result to
//   the adcs and the serial outputs.


//     Mixer:

//   The mixer gets the outputs of the AWM2, the MEG (for the previous
//   sample) and the external inputs, attenuates and sums them
//   according to its mapping instructions, and pushes the results to
//   the MEG and the external outputs.


//--------------------------------------------------------------------------------
//
//    Memory map in rough numerical order with block indications
//
//   cccccc 000000  AMW2/Filters     mmmm .aaa aaaa aaaa                      Filter 1 mode and main parameter
//   cccccc 000001  AMW2/Filters     .xxx xxxx uuuu uuuu                      Bypass/dry level
//   cccccc 000010  AMW2/Filters     .... .ccc cccc cccc                      Filter 2 mode and main parameter
//   cccccc 000011  AMW2/Filters     .... .... vvvv vvvv                      Post-filter level
//   cccccc 000100  AMW2/Filters     bbbb b... .... ....                      Filters second parameter

//   cccccc 000101  AWM2/LFO         .... .... .aaa aaaa                      LFO amplitude depth
//   cccccc 000110  AWM2/Envelope    ssss ssss iiii iiii                      Attack speed and start volume
//   cccccc 000111  AWM2/Envelope    ssss ssss tttt tttt                      Decay 1 speed and target
//   cccccc 001000  AWM2/Envelope    ssss ssss tttt tttt                      Decay 2 speed and target
//   cccccc 001001  AWM2/Envelope    ssss ssss gggg gggg                      Release speed & global volume
//   cccccc 001010  AWM2/LFO         tt.s ssss mppp pppp                      LFO type, step, pitch mode, pitch depth
//   cccccc 001011   ?
//   cccccc 001100   ?
//   cccccc 001101   ?

//   000000 001110                   9100 at startup
//   000000 001111                   c002 at startup then c003
//   000001 001110  AWM2/Control     internal register address
//   000001 001111  AWM2/Control     (read) internal register value
//   000010 00111*  AWM2/Control     wave direct access address
//   000011 00111*  AWM2/Control     wave direct access size
//   000100 001110  AWM2/Control     wave direct access trigger (8000 = read sample from rom, 9000 = read sample from ram, 5000 = write sample to ram)
//   000100 001111  AWM2/Control     wave direct access status
//   000101 00111*  AWM2/Control     wave direct access data
//   00011* 00111*  AWM2/Control     keyon mask
//   001000 001110  AWM2/Control     keyon trigger
//   001101 001110                   1100 at startup then 0040
//   010000 001110  MEG/Control      .... .... .... ....    commit LFO increments on write
//   010000 001111  MEG/Control      .... ...a aaaa aaaa    program address
//   010001 00111*  MEG/Control      dddd dddd dddd dddd    program data 1/2
//   010010 00111*  MEG/Control      dddd dddd dddd dddd    program data 2/2
//   010011 001110   ? sy26
//   010011 001111   ? sy27
//   010100 001111                   00ff after part 1
//   010101 001110                   00ff after part 1
//   010101 001111                   00ff after part 1
//   011mmm 001110  MEG/Reverb       memory map
//   100000 001110  MEG/Reverb       ram memory map tlb enable (0=on, 1=off, bits 0-7)
//   100000 001111  MEG/Reverb       ram memory bank clear (1=trigger a clear)
//   100001 001110  MEG/Reverb       ram direct access status
//   100101 00111*  MEG/Reverb       ram direct access address
//   100110 00111*  MEG/Reverb       ram direct access data

//   101*** 00111*   ? sy5x

//   cccccc 010000   ?

//   cccccc 010001  AMW2/Streaming   ?-pp pppp pppp pppp                      Pitch
//   cccccc 01001*  AMW2/Streaming   ?Lll llll ssss ssss ssss ssss ssss ssss  Loop disable. Loop size adjust. Number of samples before the loop point
//   cccccc 01010*  AMW2/Streaming   bfff ffff ssss ssss ssss ssss ssss ssss  Backwards. Finetune. Number of samples in the loop
//   cccccc 01011*  AMW2/Streaming   ffSS Smma aaaa aaaa aaaa aaaa aaaa aaaa  Format, Scaling, Compressor mode, Sample address

//   cccccc 100000  AWM2/IIR         vvvv vvvv vvvv vvvv                      IIR1 a1
//   aaaaaa 100001  MEG/Data         cccc cccc cccc cccc                      constant index 6*a + 0
//   cccccc 100010  AWM2/IIR         vvvv vvvv vvvv vvvv                      IIR1 b1
//   aaaaaa 100011  MEG/Data         cccc cccc cccc cccc                      constant index 6*a + 1
//   cccccc 100100  AWM2/IIR         vvvv vvvv vvvv vvvv                      IIR1 a0
//   aaaaaa 100101  MEG/Data         cccc cccc cccc cccc                      constant index 6*a + 2
//   cccccc 100110  AWM2/IIR         vvvv vvvv vvvv vvvv                      IIR1 a1
//   aaaaaa 100111  MEG/Data         cccc cccc cccc cccc                      constant index 6*a + 3
//   cccccc 101000  AWM2/IIR         vvvv vvvv vvvv vvvv                      IIR1 b1
//   aaaaaa 101001  MEG/Data         cccc cccc cccc cccc                      constant index 6*a + 4
//   cccccc 101010  AWM2/IIR         vvvv vvvv vvvv vvvv                      IIR1 a0
//   aaaaaa 101011  MEG/Data         cccc cccc cccc cccc                      constant index 6*a + 5


//   aaaaaa 11000a  MEG/Data         oooo oooo oooo oooo                      offset index a
//   ssssss 110010  Mixer            llll llll rrrr rrrr                      Route attenuation left/right input s
//   ssssss 110011  Mixer            0000 0000 1111 1111                      Route attenuation slot 0/1   input s
//   ssssss 110100  Mixer            2222 2222 3333 3333                      Route attenuation slot 2/3   input s
//   ssssss 110101  Mixer            fedc ba98 7654 3210                      Route mode bit 2 input s output 0-f
//   ssssss 110110  Mixer            fedc ba98 7654 3210                      Route mode bit 1 input s output 0-f
//   ssssss 110111  Mixer            fedc ba98 7654 3210                      Route mode bit 0 input s output 0-f
//   ssssss 111000  Mixer            llll llll rrrr rrrr                      Route attenuation left/right input s+40
//   ssssss 111001  Mixer            0000 0000 1111 1111                      Route attenuation slot 0/1   input s+40
//   ssssss 111010  Mixer            2222 2222 3333 3333                      Route attenuation slot 2/3   input s+40
//   ssssss 111011  Mixer            fedc ba98 7654 3210                      Route mode bit 2 input s+40 output 0-f
//   ssssss 111100  Mixer            fedc ba98 7654 3210                      Route mode bit 1 input s+40 output 0-f
//   ssssss 111101  Mixer            fedc ba98 7654 3210                      Route mode bit 0 input s+40 output 0-f
//   aaaaaa 11111a  MEG/LFO          pppp ttss iiii iiii                      LFO index a, phase, type, shift, increment


//======================= AWM2 blocks ============================================

// Streaming block
//
//   cccccc 010001   ?-pp pppp pppp pppp                      Pitch
//   cccccc 01001*   ?Lll llll ssss ssss ssss ssss ssss ssss  Loop disable. Loop size adjust. Number of samples before the loop point
//   cccccc 01010*   bfff ffff ssss ssss ssss ssss ssss ssss  Backwards. Finetune. Number of samples in the loop
//   cccccc 01011*   ffSS Smma aaaa aaaa aaaa aaaa aaaa aaaa  Format, Scaling, Compressor mode, Sample address
//
// The streaming block manages reading and decoding samples from rom
// and/or dram at a given pitch and format.  The samples are
// interpolated for a better quality.
//
// Addresses 000000-ffffff are in rom (and maybe sram?),
// 1000000-1ffffff are in dram.
//
// The unknown bit in the 010001 slot tends to be set when reading a
// compressed sample and unset otherwise.  It does not seem to impact
// the result though.  The unknown bit in the 010010 slot doesn't seem
// to ever been set in the mu100 and does not seem to impact the
// result.
//
//   Sample formats
//
// Samples can be in one of four formats, 8 bits, 12
// bits, 16 bits and adaptive-delta-compression with 8 bits per
// sample.  Non-compressed samples are zero-extended on the right to
// get a almost-full-range 16-bits value.
//
// The compressed format uses a running delta and an accumulator.  The
// input byte is expanded into a 10-bit signed value through a fixed
// table, which is added to the current delta.  The delta is then
// added to the accumulator, which gives the current sample value.
// Then the current delta is, depending on the mode bits, multiplied
// by either 0.875 (7/8), 0.75 (3/4), 0.5 (1/2) or 0 (e.g. cleared).
//
// The multiplier on the delta is buggy and bias towards negative
// numbers, but it's not entirely clear how exactly.  Even worse, the
// multiplier results change depending on whether the scaling is zero
// or non-zero, and also has some kind of context or extra state bits
// hidden somewhere.
//
//
//   Sample scaling
//
// Samples just read are then shifted left by Scaling bits (0-7).
// While the scaling is in practice only used for compressed samples,
// the hardware applies it to any format.  The result is clamped
// between -0x8000 and a value depending on the amount of scaling
// (0x7fff for 0, 0x7ffe for 1, ..., 0x7f80 for 7).
//
//
//   Sample addressing, pitching and looping
//
// The chip has two 25-bits address, 16-bits data buses to the sample
// roms and drams.  The samples are interleaved between the two buses,
// looking as if the data bus was 32 bits wide.  It directly manages
// dram signals, so there must be a way somewhere for it to tell
// whether a sample address is in dram or not.  When in dual-chip
// configuration, the address and data lines of the buses are directly
// connected, so they have a way to arbitrate their accesses.  The
// amount of data needed at a given time varying depending on pitch
// and sample format, the design of the memory access controller must
// have been interesting.
//
// The current sample position is in signed 25.15 format.  The initial
// value of the sample position is minus the number of samples before
// the loop point (unsigned 24 bits, slots 010010 and 010011).  It is
// incremented by the unsigned 7.15 step value for each sample, until
// it reaches a positive value more or equal to the loop size
// (unsigned 24 bits, slots 010100 and 010101).  Then if looping is
// enabled the position is decreased by the loop size and incremented
// by the loop size adjust, otherwise the last sample value output is
// held and a maximum speed envelope release is triggered.
//
// The loop size adjust is a 0.6 unsigned value (e.g. between 0 and
// 0.984375).
//
// Looping is enabled when L=0 (slot 010010) and b=0 (slot 010110).
//
// The unsigned 7.15 step is computed from the pitch and the finetune
// values.  The base step value is 2**pitch with pitch encoded as a
// signed 4.10 value, e.g. giving a result between 1/256 and almost
// but not quite 256.  The exponentiation table has 13 bits of
// precision including the left 1 bit.  The finetune is a signed value
// between -64 and +63 that is added to the pitch once position 0 is
// reached.
//
// Backwards sample reading negates the sample position value before
// fetching.  Note that backwards reading disable looping.
//
// Samples are read with sample pos 0 corresponding to the bottom
// sample at the 25-bits address. It is important to note that four
// consecutive sample values are required for the interpolation block,
// and the hardware manages to provide the correct values even for
// compressed samples with large steps, requiring to compute and
// accumulate the deltas for all the bytes on the way.  The memory
// controller must be REALLY interesting.
//
// A pitch skip bigger than the loop size ends up with results
// somewhere between weird and utterly insane.  Don't do that.
//
//
//   Sample interpolation
//
// Samples go through an interpolator which uses two past samples and
// two future samples to compute the final value for a non-integer
// position, with a weight for each history sample.  The weights are
// computed from two polynoms:
//   f0(t) = (t - t**3)/6
//   f1(t) = t + (t**2 - t**3)/2
//
// The polynoms are used with the decimal part 'p' (as in phase) of
// the sample position.  The computation from the four samples s0..s3
// is:
//   s = - s0 * f0(1-p) + s1 * f1(1-p) + s2 * f1(p) - s3 * f0(p)
//
// f0(0) = f0(1) = f1(0) = 0 and f1(1) = 1, so when phase is 0 (sample
// streaming with no frequency shifting) the sample s1 is output.
//
// The implementation of the weights uses two tables with apparently
// 2048 entries and 10 bits precision (e.g. between 0 and 0.999), but
// are in reality 1023 entries.  Each entry of the 1023-entries tables
// goes to slots 2n-1 and 2n (n=1..1023), and slots 0 and 2047 are
// hardcoded to both 0 for f0 and 0/1.0 for f1. That way the tables
// can be used in both directions and the computation of 1-p consists
// of inverting all the bits.
//
// The two-past sample for the first position, the first pointed at by
// the streamer, is forced to zero.
//
// Post-interpolation, the output is a 16-bits signed value with no
// decimals.


// Dpcm delta expansion table
const std::array<s16, 256> swp30_device::streaming_block::dpcm_expand = []() {
	std::array<s16, 256> deltas;
	static const s16 offset[4] = { 0, 0x20, 0x60, 0xe0 };
	for(u32 i=0; i != 128; i++) {
		u32 e = i >> 5;
		s16 base = ((i & 0x1f) << e) + offset[e];
		deltas[i] = base;
		deltas[i+128] = -base;
	}
	deltas[0x80] = 0x88; // Not actually used by samples, but tested on hardware
	return deltas;
}();

// Pitch conversion table, 2**(i/1024) as 1.12
const std::array<u16, 0x400> swp30_device::streaming_block::pitch_base = []() {
	std::array<u16, 0x400> base;
	for(u32 i=0; i != 0x400; i++)
		base[i] = pow(2, i/1024.0) * 4096;
	return base;
}();

// Sample interpolation functions f0 and f1.  The second half of f1 is adjusted so that the combination is 1.0 (e.g. 0x400)
const std::array<std::array<s16, 0x800>, 2> swp30_device::streaming_block::interpolation_table = []() {
	std::array<std::array<s16, 0x800>, 2> result;

	// The exact way of doing the computations replicate the values
	// actually used by the chip (which are very probably a rom, of
	// course).

	for(u32 i=1; i != 1024; i++) {
		s16 f0 = (((i << 20) - i*i*i) / 6) >> 20;
		result[0][2*i-1] = f0;
		result[0][2*i  ] = f0;
	}
	for(u32 i=1; i != 513; i++) {
		s16 f1 = i + ((((i*i) << 10) - i*i*i) >> 21);
		result[1][2*i-1] = f1;
		result[1][2*i  ] = f1;
	}
	for(u32 i=513; i != 1024; i++) {
		u32 i1 = 2*i;
		u32 i2 = 2047 ^ i1;
		// When interpolating, f1 is added and f0 is subtracted, and the total must be 0x400
		s16 f1 = 0x400 + result[0][i1] + result[0][i2] - result[1][i2];
		result[1][2*i-1] = f1;
		result[1][2*i  ] = f1;
	}
	result[0][    0] = 0x000;
	result[0][0x7ff] = 0x000;
	result[1][    0] = 0x000;
	result[1][0x7ff] = 0x400;
	return result;
}();

const std::array<s32, 8> swp30_device::streaming_block::max_value = {
	0x7fff, 0x7ffe, 0x7ffc, 0x7ff8, 0x7ff0, 0x7fe0, 0x7fc0, 0x7f80
};

void swp30_device::streaming_block::clear()
{
	m_start = 0;
	m_loop = 0;
	m_address = 0;
	m_pitch = 0;
	m_loop_size = 0x400;
	m_pos = 0;
	m_pos_dec = 0;
	m_dpcm_s0 = m_dpcm_s1 = m_dpcm_s2 = m_dpcm_s3 = 0;
	m_dpcm_pos = 0;
	m_dpcm_delta = 0;
	m_first = false;
	m_done = false;
	m_last = 0;
}

void swp30_device::streaming_block::keyon()
{
	m_pos = -(m_start & 0xffffff) - 1;
	m_pos_dec = 0;
	m_dpcm_s0 = m_dpcm_s1 = m_dpcm_s2 = m_dpcm_s3 = 0;
	m_dpcm_pos = m_pos+1;
	m_dpcm_delta = 0;
	m_first = true;
	m_finetune_active = false;
	m_done = false;
}

void swp30_device::streaming_block::scale_and_clamp_one(s16 &val, u32 scale, s32 limit)
{
	s32 sval = val << scale;
	if(sval < -0x8000)
		sval = -0x8000;
	else if(sval > limit)
		sval = limit;
	val = sval;
}

void swp30_device::streaming_block::scale_and_clamp(s16 &val0, s16 &val1, s16 &val2, s16 &val3)
{
	u32 scale = (m_address >> 27) & 7;
	if(!scale)
		return;
	s32 limit = max_value[scale];
	scale_and_clamp_one(val0, scale, limit);
	scale_and_clamp_one(val1, scale, limit);
	scale_and_clamp_one(val2, scale, limit);
	scale_and_clamp_one(val3, scale, limit);
}

void swp30_device::streaming_block::read_16(memory_access<25, 2, -2, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1, s16 &val2, s16 &val3)
{
	s32 spos = m_loop & 0x80000000 ? -m_pos : m_pos;
	offs_t base_address = m_address & 0x1ffffff;
	offs_t adr = base_address + (spos >> 1);
	switch(spos & 1) {
	case 0: {
		// 32103210 32103210 32103210
		// bbbbaaaa ddddcccc ........
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr);
		val0 = l0;
		val1 = l0 >> 16;
		val2 = l1;
		val3 = l1 >> 16;
		break;
	}
	case 1: {
		// 32103210 32103210 32103210
		// aaaa.... ccccbbbb ....dddd
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr+1);
		u32 l2 = wave.read_dword(adr+2);
		val0 = l0 >> 16;
		val1 = l1;
		val2 = l1 >> 16;
		val3 = l2;
		break;
	}
	}
	scale_and_clamp(val0, val1, val2, val3);
}

void swp30_device::streaming_block::read_12(memory_access<25, 2, -2, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1, s16 &val2, s16 &val3)
{
	s32 spos = m_loop & 0x80000000 ? -m_pos : m_pos;
	offs_t base_address = m_address & 0x1ffffff;
	offs_t adr = base_address + (spos >> 3)*3;
	switch(spos & 7) {
	case 0: {
		// 10210210 02102102 21021021 10210210 10210210
		// ccbbbaaa ....dddc ........ ........ ........
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr+1);
		val0 =  (l0 & 0x00000fff) << 4;
		val1 =  (l0 & 0x00fff000) >> 8;
		val2 = ((l0 & 0xff000000) >> 20) | ((l1 & 0x0000000f) << 12);
		val3 =   l1 & 0x0000fff0;
		break;
	}
	case 1: {
		// 10210210 02102102 21021021 10210210 10210210
		// bbaaa... .dddcccb ........ ........ ........
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr+1);
		val0 =  (l0 & 0x00fff000) >> 8;
		val1 = ((l0 & 0xff000000) >> 20) | ((l1 & 0x0000000f) << 12);
		val2 =   l1 & 0x0000fff0;
		val3 =  (l1 & 0x0fff0000) >> 12;
		break;
	}
	case 2: {
		// 10210210 02102102 21021021 10210210 10210210
		// aa...... dcccbbba ......dd ........ ........
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr+1);
		u32 l2 = wave.read_dword(adr+2);
		val0 = ((l0 & 0xff000000) >> 20) | ((l1 & 0x0000000f) << 12);
		val1 =   l1 & 0x0000fff0;
		val2 =  (l1 & 0x0fff0000) >> 12;
		val3 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
		break;
	}
	case 3: {
		// 10210210 02102102 21021021 10210210 10210210
		// ........ cbbbaaa. ...dddcc ........ ........
		u32 l1 = wave.read_dword(adr+1);
		u32 l2 = wave.read_dword(adr+2);
		val0 =   l1 & 0x0000fff0;
		val1 =  (l1 & 0x0fff0000) >> 12;
		val2 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
		val3 =  (l2 & 0x000fff00) >> 4;
		break;
	}
	case 4: {
		// 10210210 02102102 21021021 10210210 10210210
		// ........ baaa.... dddcccbb ........ ........
		u32 l1 = wave.read_dword(adr+1);
		u32 l2 = wave.read_dword(adr+2);
		val0 =  (l1 & 0x0fff0000) >> 12;
		val1 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
		val2 =  (l2 & 0x000fff00) >> 4;
		val3 =  (l2 & 0xfff00000) >> 16;
		break;
	}
	case 5: {
		// 10210210 02102102 21021021 10210210 10210210
		// ........ a....... cccbbbaa .....ddd ........
		u32 l1 = wave.read_dword(adr+1);
		u32 l2 = wave.read_dword(adr+2);
		u32 l3 = wave.read_dword(adr+3);
		val0 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
		val1 =  (l2 & 0x000fff00) >> 4;
		val2 =  (l2 & 0xfff00000) >> 16;
		val3 =  (l3 & 0x00000fff) << 4;
		break;
	}
	case 6: {
		// 10210210 02102102 21021021 10210210 10210210
		// ........ ........ bbbaaa.. ..dddccc ........
		u32 l2 = wave.read_dword(adr+2);
		u32 l3 = wave.read_dword(adr+3);
		val0 =  (l2 & 0x000fff00) >> 4;
		val1 =  (l2 & 0xfff00000) >> 16;
		val2 =  (l3 & 0x00000fff) << 4;
		val3 =  (l3 & 0x00fff000) >> 8;
		break;
	}
	case 7: {
		// 10210210 02102102 21021021 10210210 10210210
		// ........ ........ aaa..... ddcccbbb .......d
		u32 l2 = wave.read_dword(adr+2);
		u32 l3 = wave.read_dword(adr+3);
		u32 l4 = wave.read_dword(adr+4);
		val0 =  (l2 & 0xfff00000) >> 16;
		val1 =  (l3 & 0x00000fff) << 4;
		val2 =  (l3 & 0x00fff000) >> 8;
		val3 = ((l3 & 0xff000000) >> 20) | ((l4 & 0x0000000f) << 12);
		break;
	}
	}
	scale_and_clamp(val0, val1, val2, val3);
}

void swp30_device::streaming_block::read_8(memory_access<25, 2, -2, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1, s16 &val2, s16 &val3)
{
	s32 spos = m_loop & 0x80000000 ? -m_pos : m_pos;
	offs_t base_address = m_address & 0x1ffffff;
	offs_t adr = base_address + (spos >> 2);
	switch(spos & 3) {
	case 0: {
		// 10101010 10101010
		// ddccbbaa ........
		u32 l0 = wave.read_dword(adr);
		val0 = (l0 & 0x000000ff) << 8;
		val1 =  l0 & 0x0000ff00;
		val2 = (l0 & 0x00ff0000) >> 8;
		val3 = (l0 & 0xff000000) >> 16;
		break;
	}
	case 1: {
		// 10101010 10101010
		// ccbbaa.. ......dd
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr+1);
		val0 =  l0 & 0x0000ff00;
		val1 = (l0 & 0x00ff0000) >> 8;
		val2 = (l0 & 0xff000000) >> 16;
		val3 = (l1 & 0x000000ff) << 8;
		break;
	}
	case 2: {
		// 10101010 10101010
		// bbaa.... ....ddcc
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr+1);
		val0 = (l0 & 0x00ff0000) >> 8;
		val1 = (l0 & 0xff000000) >> 16;
		val2 = (l1 & 0x000000ff) << 8;
		val3 =  l1 & 0x0000ff00;
		break;
	}
	case 3: {
		// 10101010 10101010
		// aa...... ..ddccbb
		u32 l0 = wave.read_dword(adr);
		u32 l1 = wave.read_dword(adr+1);
		val0 = (l0 & 0xff000000) >> 16;
		val1 = (l1 & 0x000000ff) << 8;
		val2 =  l1 & 0x0000ff00;
		val3 = (l1 & 0x00ff0000) >> 8;
		break;
	}
	}
	scale_and_clamp(val0, val1, val2, val3);
}

void swp30_device::streaming_block::dpcm_step(u8 input)
{
	u32 mode = (m_address >> 25) & 3;
	u32 scale = (m_address >> 27) & 7;
	s32 limit = max_value[scale];

	m_dpcm_s0 = m_dpcm_s1;
	m_dpcm_s1 = m_dpcm_s2;
	m_dpcm_s2 = m_dpcm_s3;

	s32 delta = m_dpcm_delta + dpcm_expand[input];
	s32 sample = m_dpcm_s3 + (delta << scale);

	if(sample < -0x8000) {
		sample = -0x8000;
		delta = 0;
	} else if(sample > limit) {
		sample = limit;
		delta = 0;
	}
	m_dpcm_s3 = sample;

	switch(mode) {
	case 0: delta = delta * 7 / 8; break;
	case 1: delta = delta * 3 / 4; break;
	case 2: delta = delta     / 2; break;
	case 3: delta = 0; break;
	}
	m_dpcm_delta = delta;
}

void swp30_device::streaming_block::read_8c(memory_access<25, 2, -2, ENDIANNESS_LITTLE>::cache &wave, s16 &val0, s16 &val1, s16 &val2, s16 &val3)
{
	offs_t base_address = m_address & 0x1ffffff;
	if(m_loop & 0x80000000) {
		abort();
	} else {
		s32 spos =  m_dpcm_pos;
		base_address += spos >> 2;
		u32 cv = wave.read_dword(base_address);
		while(spos != m_pos + 4) {
			u8 input = cv >> ((spos & 3) << 3);
			dpcm_step(input);
			spos++;
			if((spos & 3) == 0) {
				base_address ++;
				cv = wave.read_dword(base_address);
			}
		}
		m_dpcm_pos = spos;
	}

	val0 = m_dpcm_s0;
	val1 = m_dpcm_s1;
	val2 = m_dpcm_s2;
	val3 = m_dpcm_s3;
}

std::pair<s16, bool> swp30_device::streaming_block::step(memory_access<25, 2, -2, ENDIANNESS_LITTLE>::cache &wave, s32 pitch_lfo)
{
	if(m_done)
		return std::make_pair(m_last, false);

	s16 val0, val1, val2, val3;

	switch(m_address >> 30) {
	case 0: read_16(wave, val0, val1, val2, val3); break;
	case 1: read_12(wave, val0, val1, val2, val3); break;
	case 2: read_8 (wave, val0, val1, val2, val3); break;
	case 3: read_8c(wave, val0, val1, val2, val3); break;
	}
	if(m_first)
		val0 = 0;

	// Not perfectly exact, there are some rounding-like issues from
	// time to time
	s32 index = (m_pos_dec >> 4) & 2047;
	s16 result = (
				  - interpolation_table[0][index ^ 2047] * val0
				  + interpolation_table[1][index ^ 2047] * val1
				  + interpolation_table[1][index       ] * val2
				  - interpolation_table[0][index       ] * val3
				  ) >> 10;

	u32 pitch = m_pitch + pitch_lfo;
	if(m_finetune_active) {
		s32 ft = (m_loop >> 24) & 0x7f;
		if(ft & 0x40)
			ft -= 0x80;
		pitch += ft;
		if(pitch & 0x80000000)
			pitch = 0;
		if(pitch & 0x4000)
			pitch = 0x3fff;
	}
	u32 e = ((pitch >> 10) + 8) & 15;
	u32 m = pitch & 0x3ff;
	u32 step = (pitch_base[m] << 10) >> (15-e);

	m_pos_dec += step;
	if(m_pos_dec >= 0x8000) {
		m_first = false;
		m_pos += m_pos_dec >> 15;
		if(!m_finetune_active && m_pos >= 0)
			m_finetune_active = true;

		m_pos_dec &= 0x7fff;
		if(m_pos >= m_loop_size) {
			if(!((m_loop & 0x80000000) || (m_start & 0x40000000))) {
				m_pos -= m_loop_size;
				m_pos_dec += (m_start >> 15) & 0x7e00;
				if(m_pos_dec >= 0x8000)
					m_pos ++;
				m_pos_dec &= 0x7fff;
				m_dpcm_pos = 3;
			} else {
				m_done = true;
				m_last = result;
				return std::make_pair(m_last, true);
			}
		}
	}
	return std::make_pair(result, false);
}

void swp30_device::streaming_block::update_loop_size()
{
	m_loop_size = m_loop & 0x3ffffff;
	if(!m_loop_size && !((m_loop & 0x80000000) || (m_start & 0x40000000)))
		m_loop_size = 0x400;
}

void swp30_device::streaming_block::start_h_w(u16 data)
{
	m_start = (m_start & 0x0000ffff) | (data << 16);
	update_loop_size();
}

void swp30_device::streaming_block::start_l_w(u16 data)
{
	m_start = (m_start & 0xffff0000) | data;
}

void swp30_device::streaming_block::loop_h_w(u16 data)
{
	m_loop = (m_loop & 0x0000ffff) | (data << 16);
	update_loop_size();
}

void swp30_device::streaming_block::loop_l_w(u16 data)
{
	m_loop = (m_loop & 0xffff0000) | data;
	update_loop_size();
}

void swp30_device::streaming_block::address_h_w(u16 data)
{
	m_address = (m_address & 0x0000ffff) | (data << 16);
}

void swp30_device::streaming_block::address_l_w(u16 data)
{
	m_address = (m_address & 0xffff0000) | data;
}

void swp30_device::streaming_block::pitch_w(u16 data)
{
	m_pitch = data;
}

u16 swp30_device::streaming_block::start_h_r() const
{
	return m_start >> 16;
}

u16 swp30_device::streaming_block::start_l_r() const
{
	return m_start;
}

u16 swp30_device::streaming_block::loop_h_r() const
{
	return m_loop >> 16;
}

u16 swp30_device::streaming_block::loop_l_r() const
{
	return m_loop;
}

u16 swp30_device::streaming_block::address_h_r() const
{
	return m_address >> 16;
}

u16 swp30_device::streaming_block::address_l_r() const
{
	return m_address;
}

u16 swp30_device::streaming_block::pitch_r() const
{
	return m_pitch;
}

std::string swp30_device::streaming_block::describe() const
{
	std::string desc;
	desc = util::string_format("[%04x %08x %08x %08x] ", m_pitch, m_start, m_loop, m_address) + util::string_format("sample %06x-%06x @ %07x ", m_start & 0xffffff, m_loop & 0xffffff, m_address & 0x1ffffff);
	switch(m_address >> 30) {
	case 0: desc += "16"; break;
	case 1: desc += "12"; break;
	case 2: desc += "8 "; break;
	case 3: desc += util::string_format("c%x", (m_address >> 25) & 3); break;
	}
	if(m_address & 0x38000000)
		desc += util::string_format(" scale %x", (m_address >> 27) & 7);
	if(m_loop & 0x80000000)
		desc += " back";
	else if(m_start & 0x40000000)
		desc += " fwd ";
	else
		desc += " loop";
	if(m_start & 0x3f000000)
		desc += util::string_format(" loop-adjust %02x", (m_start >> 24) & 0x3f);
	if(m_loop & 0x7f000000) {
		if(m_loop & 0x40000000)
			desc += util::string_format(" loop-tune -%02x", 0x40 - ((m_loop >> 24) & 0x3f));
		else
			desc += util::string_format(" loop-tune +%02x", (m_loop >> 24) & 0x3f);
	}
	if(m_pitch & 0x2000) {
		u32 p = 0x4000 - (m_pitch & 0x3fff);
		desc += util::string_format(" pitch -%x.%03x", p >> 10, p & 0x3ff);
	} else if(m_pitch & 0x3fff)
		desc += util::string_format(" pitch +%x.%03x", (m_pitch >> 10) & 7, m_pitch & 0x3ff);

	return desc;
}


//--------------------------------------------------------------------------------

// Special filters block

//   cccccc 000000   mmmm .aaa aaaa aaaa                      Filter 1 mode and main parameter
//   cccccc 000001   .xxx xxxx uuuu uuuu                      Bypass/dry level
//   cccccc 000010   mmmm .aaa aaaa aaaa                      Filter 2 mode and main parameter
//   cccccc 000011   .... .... vvvv vvvv                      Post-filter level
//   cccccc 000100   bbbb b... .... ....                      Filters second parameter
//
// This block takes samples from the streaming block and applies two
// recursive filters to them.  The type of filter and its coefficient
// encoding depends on the 4-bits mode.  Filter 1 has encoded
// parameters a and b, filter 2 has c and d.  First parameter is 11
// bits and the second 5.  A first paramter of 0 disables the
// associated filter.
//
//
//
//   Volumes
//
// Three attenuations are used, in 4.4 format.
//
// Attenuation u in slot 000001 is the bypass/dry level, adding an
// attenuated version of the input directly the output.
//
// Attenuation v in slot 000011 is the filter 1 level, attenuating its
// output before summing to the block output.
//
//   Filter types
// 0: Chamberlin configuration low pass filter with fixed q=1
//
//    a is fp 3.8
//    k = ((0x101 + a.m) << a.e) / 65536
//
//    B(0) = L(0) = 0
//    H' = x0 - L - B
//    B' = B + k * H'
//    L' = L + k * B'
//    y0 = L'
//
// 1: Chamberlin configuration low pass filter
//
//    a is fp 3.8, (b+4) is fp 3.3
//    k = ((0x101 + a.m) << a.e) / 65536
//    q = ((0x10 - (b+4).m) << (4 - (b+4).e)) / 128
//
//    B(0) = L(0) = 0
//    H' = x0 - L - q*B
//    B' = B + k * H'
//    L' = L + k * B'
//    y0 = L'
//
// 2: order-1 lowpass IIR
//
//    a is fp 3.8, b is unused
//
//    a0 = ((0x101 + a.m) << a.e) / 65536
//    b1 = 1-a0
//    y0 = x0 * a0 + y1 * b1 = y1 + (x0 - y1) * a0
//
// 3: order-2 lowpass IIR
//
//    a is fp3.8, (b+4) is fp 2.3
//
//    dt = ((0x10 - (b+4).m) << (4 - (b+4).e)) / 128
//
//    a0 = ((0x101 + a.m) << a.e) / 65536
//    b1 = (2 - dt - a0)
//    b2 = dt - 1
//
//    y0 = x0 * a0 + y1 * (2 - dt - a0) + y2 * (dt - 1)
//       = (x0 - y1) * a0 + (y2 - y1) * dt + 2*y1 - y2
//
// 4: Chamberlin configuration band pass filter with fixed q=1
//
//    a is fp 3.8
//    k = ((0x101 + a.m) << a.e) / 65536
//
//    B(0) = L(0) = 0
//    B' = B + k * (x0 - L - B)
//    L' = L + k * B'
//    y0 = B'
//
// 5: Chamberlin configuration band pass filter
//
//    a is fp 3.8, (b+4) is fp 3.3
//    k = ((0x101 + a.m) << a.e) / 65536
//    q = ((0x10 - (b+4).m) << (4 - (b+4).e)) / 128
//
//    B(0) = L(0) = 0
//    B' = B + k * (x0 - L - q*B)
//    L' = L + k * B'
//    y0 = B'
//
// 6: order-1 ?pass IIR
//
//    a is fp 3.8, b is unused
//
//    a0 = ((0x101 + a.m) << a.e) / 65536
//    b1 = 1-a0
//    y0 = x0 - x1 + y1 * b1 = x0 - x1 + y1 - y1 * a0
//
// 8: Chamberlin configuration high pass filter with fixed q=1
//
//    a is fp 3.8
//    k = ((0x101 + a.m) << a.e) / 65536
//
//    B(0) = L(0) = 0
//    H' = x0 - L - B
//    B' = B + k * H'
//    L' = L + k * B'
//    y0 = B'
//
// 9: Chamberlin configuration high pass filter
//
//    a is fp 3.8, (b+4) is fp 3.3
//    k = ((0x101 + a.m) << a.e) / 65536
//    q = ((0x10 - (b+4).m) << (4 - (b+4).e)) / 128
//
//    B(0) = L(0) = 0
//    H' = x0 - L - q*B
//    B' = B + k * H'
//    L' = L + k * B'
//    y0 = H'
//
// c: Chamberlin configuration notch filter with fixed q=1
//
//    a is fp 3.8
//    k = ((0x101 + a.m) << a.e) / 65536
//
//    B(0) = L(0) = 0
//    H' = x0 - L - B
//    B' = B + k * H'
//    L' = L + k * B'
//    y0 = H' + L
//
// d: Chamberlin configuration notch filter
//
//    a is fp 3.8, (b+4) is fp 3.3
//    k = ((0x101 + a.m) << a.e) / 65536
//    q = ((0x10 - (b+4).m) << (4 - (b+4).e)) / 128
//
//    B(0) = L(0) = 0
//    H' = x0 - L - q*B
//    B' = B + k * H'
//    L' = L + k * B'
//    y0 = H' + L
//


void swp30_device::filter_block::clear()
{
	m_filter_1_a = 0;
	m_level_1 = 0;
	m_filter_2_a = 0;
	m_level_2 = 0;
	m_filter_b = 0;

	m_filter_1_p1 = 0;
	m_filter_2_p1 = 0;
	m_filter_p2 = 0;

	m_filter_1_y0 = 0;
	m_filter_1_y1 = 0;
	m_filter_1_x1 = 0;
	m_filter_1_x2 = 0;

	m_filter_1_h = 0;
	m_filter_1_b = 0;
	m_filter_1_l = 0;
	m_filter_1_n = 0;

	m_filter_2_y0 = 0;
	m_filter_2_y1 = 0;
	m_filter_2_x1 = 0;
	m_filter_2_x2 = 0;

	m_filter_2_h = 0;
	m_filter_2_b = 0;
	m_filter_2_l = 0;
	m_filter_2_n = 0;
}

void swp30_device::filter_block::keyon()
{
	m_filter_1_y0 = 0;
	m_filter_1_y1 = 0;
	m_filter_1_x1 = 0;
	m_filter_1_x2 = 0;

	m_filter_1_h = 0;
	m_filter_1_b = 0;
	m_filter_1_l = 0;
	m_filter_1_n = 0;

	m_filter_2_y0 = 0;
	m_filter_2_y1 = 0;
	m_filter_2_x1 = 0;
	m_filter_2_x2 = 0;

	m_filter_2_h = 0;
	m_filter_2_b = 0;
	m_filter_2_l = 0;
	m_filter_2_n = 0;
}

s32 swp30_device::filter_block::step(s16 input)
{
	s32 y0 = 0;
	if(m_filter_1_a & 0x7fff) {
		if(!BIT(m_filter_1_a, 13)) {
			m_filter_1_h = (input << 6) - m_filter_1_l - ((s64(m_filter_p2) * m_filter_1_b) >> 7);
			m_filter_1_b = m_filter_1_b + ((s64(m_filter_1_p1) * m_filter_1_h) >> 16);
			m_filter_1_n = m_filter_1_h + m_filter_1_l;
			m_filter_1_l = m_filter_1_l + ((s64(m_filter_1_p1) * m_filter_1_b) >> 16);

			switch(m_filter_1_a >> 14) {
			case 0x0: y0 = m_filter_1_l; break;
			case 0x1: y0 = m_filter_1_b; break;
			case 0x2: y0 = m_filter_1_h; break;
			case 0x3: y0 = m_filter_1_n; break;
			}
		} else {
			switch(m_filter_1_a >> 12) {
			case 0x2: y0 = m_filter_1_y0 + ((s64(m_filter_1_p1) * ((input << 6) - m_filter_1_y0)) >> 16); break;
			case 0x3: y0 = 2*m_filter_1_y0 - m_filter_1_y1 + ((s64(m_filter_1_p1) * ((input << 6) - m_filter_1_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_1_y1 - m_filter_1_y0)) >> 7); break;
			case 0x6: y0 = ((input - m_filter_1_x1) << 6) + m_filter_1_y0 + ((s64(m_filter_1_p1) * (0 - m_filter_1_y0)) >> 16); break;
			case 0x7: y0 = ((input - m_filter_1_x1) << 6) + 2*m_filter_1_y0 - m_filter_1_y1 + ((s64(m_filter_1_p1) * (0 - m_filter_1_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_1_y1 - m_filter_1_y0)) >> 7); break;
			case 0xa: y0 = ((input - 2*m_filter_1_x1 + m_filter_1_x2) << 6) + m_filter_1_y0 + ((s64(m_filter_1_p1) * (0 - m_filter_1_y0)) >> 16); break;
			case 0xb: y0 = ((input - 2*m_filter_1_x1 + m_filter_1_x2) << 6) + 2*m_filter_1_y0 - m_filter_1_y1 + ((s64(m_filter_1_p1) * (0 - m_filter_1_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_1_y1 - m_filter_1_y0)) >> 7); break;
			case 0xe: y0 = ((input - 2*m_filter_1_x1 + m_filter_1_x2) << 6) + m_filter_1_y0 + ((s64(m_filter_1_p1) * ((m_filter_1_x1 << 6) - m_filter_1_y0)) >> 16); break;
			case 0xf: y0 = ((input - 2*m_filter_1_x1 + m_filter_1_x2) << 6) + 2*m_filter_1_y0 - m_filter_1_y1 + ((s64(m_filter_1_p1) * ((m_filter_1_x1 << 6) - m_filter_1_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_1_y1 - m_filter_1_y0)) >> 7); break;
			}

			m_filter_1_x2 = m_filter_1_x1;
			m_filter_1_x1 = input;
			m_filter_1_y1 = m_filter_1_y0;
			m_filter_1_y0 = y0;
		}

		if(m_filter_2_a & 0x7fff) {
			if(!BIT(m_filter_2_a, 13)) {
				m_filter_2_h = y0 - m_filter_2_l - ((s64(m_filter_p2) * m_filter_2_b) >> 7);
				m_filter_2_b = m_filter_2_b + ((s64(m_filter_2_p1) * m_filter_2_h) >> 16);
				m_filter_2_n = m_filter_2_h + m_filter_2_l;
				m_filter_2_l = m_filter_2_l + ((s64(m_filter_2_p1) * m_filter_2_b) >> 16);

				switch(m_filter_2_a >> 14) {
				case 0x0: y0 = m_filter_2_l; break;
				case 0x1: y0 = m_filter_2_b; break;
				case 0x2: y0 = m_filter_2_h; break;
				case 0x3: y0 = m_filter_2_n; break;
				}
			} else {
				s32 y0_1 = y0;
				switch(m_filter_2_a >> 12) {
				case 0x2: y0 = m_filter_2_y0 + ((s64(m_filter_2_p1) * (y0 - m_filter_2_y0)) >> 16); break;
				case 0x3: y0 = 2*m_filter_2_y0 - m_filter_2_y1 + ((s64(m_filter_2_p1) * (y0 - m_filter_2_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_2_y1 - m_filter_2_y0)) >> 7); break;
				case 0x6: y0 = (y0 - m_filter_2_x1) + m_filter_2_y0 + ((s64(m_filter_2_p1) * (0 - m_filter_2_y0)) >> 16); break;
				case 0x7: y0 = (y0 - m_filter_2_x1) + 2*m_filter_2_y0 - m_filter_2_y1 + ((s64(m_filter_2_p1) * (0 - m_filter_2_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_2_y1 - m_filter_2_y0)) >> 7); break;
				case 0xa: y0 = (y0 - 2*m_filter_2_x1 + m_filter_2_x2) + m_filter_2_y0 + ((s64(m_filter_2_p1) * (0 - m_filter_2_y0)) >> 16); break;
				case 0xb: y0 = (y0 - 2*m_filter_2_x1 + m_filter_2_x2) + 2*m_filter_2_y0 - m_filter_2_y1 + ((s64(m_filter_2_p1) * (0 - m_filter_2_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_2_y1 - m_filter_2_y0)) >> 7); break;
				case 0xe: y0 = (y0 - 2*m_filter_2_x1 + m_filter_2_x2) + m_filter_2_y0 + ((s64(m_filter_2_p1) * ((m_filter_2_x1 << 6) - m_filter_2_y0)) >> 16); break;
				case 0xf: y0 = (y0 - 2*m_filter_2_x1 + m_filter_2_x2) + 2*m_filter_2_y0 - m_filter_2_y1 + ((s64(m_filter_2_p1) * ((m_filter_2_x1 << 6) - m_filter_2_y0)) >> 16) + ((s64(m_filter_p2) * (m_filter_2_y1 - m_filter_2_y0)) >> 7); break;
				}

				m_filter_2_x2 = m_filter_2_x1;
				m_filter_2_x1 = y0_1;
				m_filter_2_y1 = m_filter_2_y0;
				m_filter_2_y0 = y0;
			}
		}
	}

	s32 result = volume_apply(m_level_1, input << 6) + volume_apply(m_level_2, y0);
	if(result < -0x400000)
		result = -0x400000;
	else if(result > 0x3fffff)
		result = 0x3fffff;
	return result;
}

u16 swp30_device::filter_block::filter_1_a_r() const
{
	return m_filter_1_a;
}

u16 swp30_device::filter_block::level_1_r() const
{
	return m_level_1;
}

u16 swp30_device::filter_block::filter_2_a_r() const
{
	return m_filter_2_a;
}

u16 swp30_device::filter_block::level_2_r() const
{
	return m_level_2;
}

u16 swp30_device::filter_block::filter_b_r() const
{
	return m_filter_b;
}

void swp30_device::filter_block::filter_1_a_w(u16 data)
{
	m_filter_1_a = data;
	m_filter_1_p1 = (0x101 + (m_filter_1_a & 0xff)) << ((m_filter_1_a >> 8) & 7);
}

void swp30_device::filter_block::level_1_w(u16 data)
{
	m_level_1 = data;
}

void swp30_device::filter_block::filter_2_a_w(u16 data)
{
	m_filter_2_a = data;
	m_filter_2_p1 = (0x101 + (m_filter_2_a & 0xff)) << ((m_filter_2_a >> 8) & 7);
}

void swp30_device::filter_block::level_2_w(u16 data)
{
	m_level_2 = data;
}

void swp30_device::filter_block::filter_b_w(u16 data)
{
	m_filter_b = data;
	if(!BIT(m_filter_1_a, 12))
		m_filter_p2 = 0x80;

	else {
		u32 p2 = (m_filter_b >> 11) + 4;
		m_filter_p2 = (0x10 - (p2 & 7)) << (4 - (p2 >> 3));
	}
}


s32 swp30_device::filter_block::volume_apply(u8 level, s32 sample)
{
	// Level is 4.4 floating point positive, and represents an attenuation
	// Sample is 16.6 signed and the result is in the same format

	// ff seems to be hardcoded to 0 output
	if(level == 0xff)
		return 0;

	s32 e = level >> 4;
	s32 m = level & 0xf;
	return ((sample << 5) - (sample * m)) >> (e+5);
}


//--------------------------------------------------------------------------------

// IIR1 filters block
//
//   cccccc 100000   IIR1 a1
//   cccccc 100010   IIR1 b1
//   cccccc 100100   IIR1 a0
//   cccccc 100110   IIR2 b1
//   cccccc 101000   IIR2 a1
//   cccccc 101010   IIR2 a0
//
// This block takes samples from the filter block and applies two 3-point
// FIR filters.  The filter constants are encoded in signed 3.13
// format.
//
// Given two consecutive inputs x0, x1 (x1 being the oldest) and the
// previous output y1 a IIR1 filter computes the output y0 as:
//
//   y0 = a0 * x0 + a1 * x1 + b1 * y1
//
// It gets 16.6 and outputs 17.6 saturated values.

void swp30_device::iir1_block::clear()
{
	m_a[0][0] = 0;
	m_a[0][1] = 0;
	m_b[0]    = 0;
	m_a[1][0] = 0;
	m_a[1][1] = 0;
	m_b[1]    = 0;

	m_hx[0] = 0;
	m_hy[0] = 0;
	m_hx[1] = 0;
	m_hy[1] = 0;
}

void swp30_device::iir1_block::keyon()
{
	m_hx[0] = 0;
	m_hy[0] = 0;
	m_hx[1] = 0;
	m_hy[1] = 0;
}

s32 swp30_device::iir1_block::step(s32 input)
{
	s32 ya = std::clamp<s32>((s64(m_a[0][0]) * input + s64(m_a[0][1]) * m_hx[0] + s64(m_b[0]) * m_hy[0]) >> 13, -0x800000, 0x7fffff);
	s32 yb = std::clamp<s32>((s64(m_a[1][0]) * ya    + s64(m_a[1][1]) * m_hx[1] + s64(m_b[1]) * m_hy[1]) >> 13, -0x800000, 0x7fffff);

	m_hx[0] = input;
	m_hy[0] = ya;

	m_hx[1] = ya;
	m_hy[1] = yb;

	return yb;
}

template<u32 filter> u16 swp30_device::iir1_block::a0_r() const
{
	return m_a[filter][0];
}

template<u32 filter> u16 swp30_device::iir1_block::a1_r() const
{
	return m_a[filter][1];
}

template<u32 filter> u16 swp30_device::iir1_block::b1_r() const
{
	return m_b[filter];
}

template<u32 filter> void swp30_device::iir1_block::a0_w(u16 data)
{
	m_a[filter][0] = data;
}

template<u32 filter> void swp30_device::iir1_block::a1_w(u16 data)
{
	m_a[filter][1] = data;
}

template<u32 filter> void swp30_device::iir1_block::b1_w(u16 data)
{
	m_b[filter] = data;
}


//--------------------------------------------------------------------------------

// Envelope block
//
//
//   cccccc 000110   ssss ssss iiii iiii                      Attack speed and start volume
//   cccccc 000111   ssss ssss tttt tttt                      Decay 1 speed and target
//   cccccc 001000   ssss ssss tttt tttt                      Decay 2 speed and target
//   cccccc 001001   ssss ssss gggg gggg                      Release speed & global volume
//
// The envelope block manages the final volume of an awm2 voice and
// allows to automatically run it through four steps:
// - Attack, climbing up from a programmed value to global volume with a slowing down curve
// - Decay 1, going down to a target volume in a linear fashion
// - Decay 2, going up or down to another target in a linear fashion
// - Release, going down to silence in a linear fashion
//
// The global volume, though, is taken into account by adding it to
// the raw envelope volume.  Hence attack actually targets 0, and the
// levels reached by the decays are the given target plus the global
// volume.
//
// The volume itself is a 14-bit 4.10 attenuation which is manipulated
// as a single number by this block.  An idle voice has stage release
// and volume 3fff.  Start volume, targets and global volume are 4.4,
// hence just zero-extended on the right.
//
// Volume modification uses a concept of speed.  At each sample a
// value is added depending on the speed and a cycle:
// - Speed 78+:    value is always 7f
//
// - Speed 70..77: value alternates between 3f and 7f on a 8-samples
//                 cycle with seven 7f on speed 77, six on 76, etc up
//                 to only 3f on 70.
//
// - Speed 48..6f: same cycles with changing pair for every 8 speeds,
//                 going 1f/3f then f/1f all the way down to 1/3.
//
// - Speed 40..47: 16-samples cycles alternating 0 and 1, where on
//                 every two-sample block there is a 1 and then either a
//                 0 or a 1.  Speed 47 has one 0, 46 has two, all the way
//                 down to 40 which is 50% 0.
//
// - Speed 38..3f: cycles of size 2*16, where in each block of 2 cycles there
//                 may be one 1 depending on the cycle. Goes from
//                 fifteen 1 (out of thirty-two) on speed 3f to eight ones on
//                 speed 38.
//
// - Speed 00..37: same cycles with bigger blocks, going size 4 for 30..37 up
//                 to size 32 for 00..07.
//
// Phase on the cycles seems unpredictable.
//
// Decay 1, Decay 2 and Release use directly the speed as programmed
// (with Release inverting bit 7).  Speeds 80+ gives someone weird
// results, with steps still of 7f but sometimes acting on the target
// level.  Attack uses the given speed but adds to it bits 9..13 of
// the volume multiplied by 4, giving a fast curve at the start which
// decelerates when approaching maximum volume.
//
// Sequencing is automatic.  If release speed is non-zero at keyon
// then the chip will ride the attack from is start value to 0, then
// go to the two decay values then all the way to 3fff on release.  If
// release speed is zero, it will hold still when reaching the end of
// decay 2.  At any time if a non-zero value is written to release
// speed and the envelope is not yet in the release stage then the
// chip switches to release.
//
// A 16-bits readonly register gives the main cpu the current stage
// and raw envelope volume (without the global volume added), with
// stages numbered 0 to 3 in the two top bits and the volume in the
// bottom 14.

void swp30_device::envelope_block::clear()
{
	m_attack = 0;
	m_decay1 = 0;
	m_decay2 = 0;
	m_release_glo = 0;
	m_envelope_level = 0x3fff;
	m_envelope_mode = RELEASE;
}

void swp30_device::envelope_block::keyon()
{
	m_envelope_level = (m_attack & 0xff) << 6;
	if((m_attack & 0xff) == 0)
		m_envelope_level = 0x80 << 6;
	m_envelope_mode = ATTACK;
}

u16 swp30_device::envelope_block::status() const
{
	return (m_envelope_mode << 14) | m_envelope_level;
}

bool swp30_device::envelope_block::active() const
{
	return m_envelope_level != 0x3fff || m_envelope_mode != RELEASE;
}

u16 swp30_device::envelope_block::level_step(u32 level, u32 sample_counter)
{
	// Phase is incorrect, and very weird

	if(level >= 0x78)
		return 0x7f;

	u32 k0 = level >> 3;
	u32 k1 = level & 7;

	if(level >= 0x48) {
		k0 -= 9;
		u32 a = (4 << k0) - 1;
		u32 b = (2 << k0) - 1;
		static const u8 mx[8] = { 0x00, 0x20, 0x44, 0xa2, 0x55, 0x75, 0xee, 0xfe };
		return ((mx[k1] >> (sample_counter & 7)) & 1) ? a : b;
	}

	if(level >= 0x40) {
		if(sample_counter & 1)
			return 1;
		u32 s1 = (sample_counter & 0xe) >> 1;
		static const u8 mx[8] = { 0x00, 0x01, 0x22, 0xa8, 0x55, 0xab, 0x77, 0xfd };
		return (mx[k1] >> s1) & 1;
	}

	k0 = 8 - k0;

	if(sample_counter & util::make_bitmask<u32>(k0))
		return 0;

	static const u16 mx[8] = { 0x5555, 0x5557, 0x5757, 0x5777, 0x7777, 0x777f, 0x7f7f, 0x7fff };
	return (mx[k1] >> ((sample_counter >> k0) & 0xf)) & 1;
}

u16 swp30_device::envelope_block::step(u32 sample_counter)
{
	u16 result = m_envelope_level + ((m_release_glo & 0xff) << 6);
	switch(m_envelope_mode) {
	case ATTACK: {
		s32 level = m_envelope_level - level_step((m_attack >> 8) + ((m_envelope_level >> 9) << 2), sample_counter);
		if(level <= 0) {
			level = 0;
			m_envelope_mode = DECAY1;
		}
		m_envelope_level = level;
		if((m_attack & 0xff) == 0)
			result = (m_release_glo & 0xff) << 6;
		break;
	}

	case DECAY1: case DECAY2: {
		u16 reg = m_envelope_mode == DECAY1 ? m_decay1 : m_decay2;
		s32 limit = (reg & 0xff) << 6;
		s32 level = m_envelope_level;
		if(level < limit) {
			level += level_step(reg >> 8, sample_counter);
			if(level > limit)
				level = limit;
		} else if(level> limit) {
			level -= level_step(reg >> 8, sample_counter);
			if(level < limit)
				level = limit;
		}
		m_envelope_level = level;
		if(level == limit) {
			if(m_envelope_mode == DECAY1)
				m_envelope_mode = DECAY2;

			else if(m_release_glo & 0xff00)
				m_envelope_mode = RELEASE;
		}
		break;
	}

	case RELEASE: {
		s32 level = m_envelope_level + level_step((m_release_glo >> 8) ^ 0x80, sample_counter);
		if(level > 0x3fff)
			level = 0x3fff;
		m_envelope_level = level;
		break;
	}
	}
	return result;
}

u16 swp30_device::envelope_block::attack_r() const
{
	return m_attack;
}

void swp30_device::envelope_block::attack_w(u16 data)
{
	m_attack = data;
}

u16 swp30_device::envelope_block::decay1_r() const
{
	return m_decay1;
}

void swp30_device::envelope_block::decay1_w(u16 data)
{
	m_decay1 = data;
}

u16 swp30_device::envelope_block::decay2_r() const
{
	return m_decay2;
}

void swp30_device::envelope_block::decay2_w(u16 data)
{
	m_decay2 = data;
}

u16 swp30_device::envelope_block::release_glo_r() const
{
	return m_release_glo;
}

void swp30_device::envelope_block::release_glo_w(u16 data)
{
	m_release_glo = data;
	if(data & 0xff00)
		m_envelope_mode = RELEASE;
}

void swp30_device::envelope_block::trigger_release()
{
	m_release_glo |= 0xff00;
	m_envelope_mode = RELEASE;
}


//--------------------------------------------------------------------------------

// LFO block

//   cccccc 000101  .... .... .aaa aaaa       LFO amplitude depth
//   cccccc 001010  tt.s ssss mppp pppp       LFO type, step, pitch mode, pitch depth

// The LFO is a slow oscillator with a period between 0.2 and 6
// seconds (0.17 to 5.2Hz).  It starts with a 18-bits counter which is
// initialized to a random value on keyon.  At each sample the value
// of step is added to the counter.  When sep is zero, the counter is
// frozen.  In addition, somewhere in every 0x4000 block at a somewhat
// unpredictable time, the counter jumps by an extra 0x40.  The final
// period thus ends up being 0x3fc00/step cycles, or between 8423 and
// 261120 cycles.

// From the 18-bits counter a 12-bit state value is created.  How
// depends on the type:

//  Type 0 (saw);  state is bits 6-17 of the counter

//  Type 1 (triangle): state is bits 6-16 of the counter followed by a
//     zero if bit 17 = 0, inverted bits 6-16 followed by a zero if
//     bit 17 = 1

//  Type 2 (rectangle): state is 0 if bit 17 = 0, fff if bit 17 = 1

//  Type 3 (sample&hold): state is random, changes of value when bits
//     9-17 change

//  Amplitude LFO.  The current value of the state is multiplied by
//  the amplitude depth (zero hence makes it disabled) and divided by
//  0x20, giving a 14-bit, 4.10 attenuation (same format as for the
//  envelope).

//  Pitch LFO.  The value of the state minus 0x400 is multiplied by
//  the pitch depth.  It is then shifted by 8 in coarse mode (m=1) and
//  11 in fine mode (m=0).  The resulting signed value is added to the
//  pitch used by the streaming block.


void swp30_device::lfo_block::clear()
{
	m_counter = 0;
	m_state = 0;
	m_type = 0;
	m_step = 0;
	m_amplitude = 0;
	m_pitch_mode = false;
	m_pitch_depth = 0;
	m_r_type_step_pitch = 0;
	m_r_amplitude = 0;
}

void swp30_device::lfo_block::keyon(running_machine &machine)
{
	m_counter = machine.rand() & 0x3ffff;
	switch(m_type) {
	case 0: m_state = m_counter >> 6; break;
	case 1: m_state = m_counter & 0x20000 ? (~m_counter >> 5) & 0xffe : (m_counter >> 5) & 0xffe; break;
	case 2: m_state = m_counter & 0x20000 ? 0xfff : 0; break;
	case 3: m_state = machine.rand() & 0xfff; break;
	}
}

void swp30_device::lfo_block::step(running_machine &machine)
{
	u32 pc = m_counter;
	m_counter = (m_counter + m_step) & 0x3ffff;
	if((m_counter & 0x03fc0) == 0x02000)
		m_counter += 0x40;
	switch(m_type) {
	case 0: m_state = m_counter >> 6; break;
	case 1: m_state = m_counter & 0x20000 ? (~m_counter >> 5) & 0xffe : (m_counter >> 5) & 0xffe; break;
	case 2: m_state = m_counter & 0x20000 ? 0xfff : 0; break;
	case 3: if((pc ^ m_counter) & 0x3fe00) m_state = machine.rand() & 0xfff; break;
	}
}

u16 swp30_device::lfo_block::get_amplitude() const
{
	return (m_state * m_amplitude) >> 5;
}

s16 swp30_device::lfo_block::get_pitch() const
{
	s32 v = (m_state - 0x400) * m_pitch_depth;
	if(m_pitch_mode)
		return v >> 8;
	else
		return v >> 11;

}

void swp30_device::lfo_block::type_step_pitch_w(u16 data)
{
	m_r_type_step_pitch = data;
	m_type = data >> 14;
	m_step = (data >> 8) & 0x1f;
	m_pitch_mode = data & 0x80;
	m_pitch_depth = data & 0x7f;
}

void swp30_device::lfo_block::amplitude_w(u16 data)
{
	m_r_amplitude = data;
	m_amplitude = data & 0x7f;
}

u16 swp30_device::lfo_block::type_step_pitch_r()
{
	return m_r_type_step_pitch;
}

u16 swp30_device::lfo_block::amplitude_r()
{
	return m_r_amplitude;
}




s32 swp30_device::volume_apply(s32 level, s32 sample)
{
	// Level is 4.10 floating point positive, and represents an attenuation
	// Sample is 16.6 signed and the result is in the same format

	// Passed-in value may have overflowed
	if(level >= 0x3fff)
		return 0;

	s32 e = level >> 10;
	s32 m = level & 0x3ff;
	s64 mul = (0x4000000 - (m << 15)) >> e;
	return (sample * mul) >> 26;
}

void swp30_device::awm2_step(std::array<s32, 0x40> &samples_per_chan)
{
	for(int chan = 0; chan != 0x40; chan++) {
		if(!m_envelope[chan].active()) {
			samples_per_chan[chan] = 0;
			continue;
		}

		auto &lfo = m_lfo[chan];

		auto [sample1, trigger_release] = m_streaming[chan].step(m_wave_cache, lfo.get_pitch());
		if(trigger_release)
			m_envelope[chan].trigger_release();

		s32 sample2 = m_filter[chan].step(sample1);
		s32 sample3 = m_iir1[chan].step(sample2);
		s32 sample4 = volume_apply(m_envelope[chan].step(m_meg->m_sample_counter) + lfo.get_amplitude(), sample3);

		lfo.step(machine());
		samples_per_chan[chan] = sample4;
	}
}



swp30_device::swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, SWP30, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_program_config("meg_program", ENDIANNESS_LITTLE, 64, 9, -3, address_map_constructor(FUNC(swp30_device::meg_prg_map), this)),
	  m_wave_config("wave", ENDIANNESS_LITTLE, 32, 25, -2),
	  m_reverb_config("reverb_ram", ENDIANNESS_LITTLE, 16, 18, -1, address_map_constructor(FUNC(swp30_device::meg_reverb_map), this)),
	  m_sintab(*this, "sintab"),
	  m_drccache(32*1024*1024 + sizeof(meg_state))
{
}

void swp30_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_wave    = &space(AS_DATA);
	m_reverb  = &space(AS_REVERB);
	m_program->cache(m_program_cache);
	m_wave->cache(m_wave_cache);
	m_reverb->cache(m_reverb_cache);

	m_meg = static_cast<meg_state *>(m_drccache.alloc_near(sizeof(meg_state)));
	m_meg->m_swp = this;
	m_meg->reset();

	state_add(STATE_GENPC,     "GENPC",     m_meg->m_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_meg->m_pc).noshow();
	state_add(0,               "PC",        m_meg->m_pc);
	state_add(1,               "P",         m_meg->m_p);

	for(int i=1; i != 0x40; i++)
		state_add(i+1, util::string_format("m%02x", i).c_str(), m_meg->m_m[i]);

	m_drcuml = std::make_unique<drcuml_state>(*this, m_drccache, 0, 1, 9, 0);
	m_drcuml->symbol_add(&m_meg->m_pc,          sizeof(m_meg->m_pc),          "pc");
	m_drcuml->symbol_add(&m_meg->m_icount,      sizeof(m_meg->m_icount),      "icount");
	m_drcuml->symbol_add(&m_meg->m_program,     sizeof(m_meg->m_program),     "program");
	m_drcuml->symbol_add(&m_meg->m_const,       sizeof(m_meg->m_const),       "const");
	m_drcuml->symbol_add(&m_meg->m_offset,      sizeof(m_meg->m_offset),      "offset");
	m_drcuml->symbol_add(&m_meg->m_m,           sizeof(m_meg->m_m),           "m");
	m_drcuml->symbol_add(&m_meg->m_r,           sizeof(m_meg->m_r),           "r");
	m_drcuml->symbol_add(&m_meg->m_t,           sizeof(m_meg->m_t),           "t");
	m_drcuml->symbol_add(&m_meg->m_p,           sizeof(m_meg->m_p),           "p");
	m_drcuml->symbol_add(&m_meg->m_mw_value,    sizeof(m_meg->m_mw_value),    "mw");
	m_drcuml->symbol_add(&m_meg->m_rw_value,    sizeof(m_meg->m_rw_value),    "rw");
	m_drcuml->symbol_add(&m_meg->m_index_value, sizeof(m_meg->m_index_value), "index");
	m_drcuml->symbol_add(&m_meg->m_memw_value,  sizeof(m_meg->m_memw_value),  "memw");

	m_meg_drc_entry = m_drcuml->handle_alloc("entry");

	m_meg_program_changed = true;
	m_meg_drc_active = allow_drc();

	set_icountptr(m_meg->m_icount);

	// Separate the streams to avoid loops with plugins and dual-swp30 systems
	m_input_stream  = stream_alloc(16, 0, 44100, STREAM_SYNCHRONOUS);
	m_output_stream = stream_alloc(0, 20, 44100, STREAM_SYNCHRONOUS);

	save_item(NAME(m_keyon_mask));

	save_item(STRUCT_MEMBER(m_streaming, m_start));
	save_item(STRUCT_MEMBER(m_streaming, m_loop));
	save_item(STRUCT_MEMBER(m_streaming, m_address));
	save_item(STRUCT_MEMBER(m_streaming, m_pitch));
	save_item(STRUCT_MEMBER(m_streaming, m_loop_size));
	save_item(STRUCT_MEMBER(m_streaming, m_pos));
	save_item(STRUCT_MEMBER(m_streaming, m_pos_dec));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_s0));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_s1));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_s2));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_s3));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_pos));
	save_item(STRUCT_MEMBER(m_streaming, m_dpcm_delta));
	save_item(STRUCT_MEMBER(m_streaming, m_first));
	save_item(STRUCT_MEMBER(m_streaming, m_finetune_active));
	save_item(STRUCT_MEMBER(m_streaming, m_done));
	save_item(STRUCT_MEMBER(m_streaming, m_last));

	save_item(STRUCT_MEMBER(m_filter, m_filter_1_a));
	save_item(STRUCT_MEMBER(m_filter, m_level_1));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_a));
	save_item(STRUCT_MEMBER(m_filter, m_level_2));
	save_item(STRUCT_MEMBER(m_filter, m_filter_b));

	save_item(STRUCT_MEMBER(m_filter, m_filter_1_p1));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_p1));
	save_item(STRUCT_MEMBER(m_filter, m_filter_p2));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_x1));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_x2));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_y0));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_y1));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_h));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_b));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_n));
	save_item(STRUCT_MEMBER(m_filter, m_filter_1_l));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_x1));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_x2));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_y0));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_y1));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_h));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_b));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_n));
	save_item(STRUCT_MEMBER(m_filter, m_filter_2_l));

	save_item(STRUCT_MEMBER(m_iir1, m_a));
	save_item(STRUCT_MEMBER(m_iir1, m_b));
	save_item(STRUCT_MEMBER(m_iir1, m_hx));
	save_item(STRUCT_MEMBER(m_iir1, m_hy));

	save_item(STRUCT_MEMBER(m_envelope, m_attack));
	save_item(STRUCT_MEMBER(m_envelope, m_decay1));
	save_item(STRUCT_MEMBER(m_envelope, m_decay2));
	save_item(STRUCT_MEMBER(m_envelope, m_release_glo));
	save_item(STRUCT_MEMBER(m_envelope, m_envelope_level));
	save_item(STRUCT_MEMBER(m_envelope, m_envelope_mode));

	save_item(STRUCT_MEMBER(m_lfo, m_counter));
	save_item(STRUCT_MEMBER(m_lfo, m_state));
	save_item(STRUCT_MEMBER(m_lfo, m_type));
	save_item(STRUCT_MEMBER(m_lfo, m_step));
	save_item(STRUCT_MEMBER(m_lfo, m_amplitude));
	save_item(STRUCT_MEMBER(m_lfo, m_pitch_mode));
	save_item(STRUCT_MEMBER(m_lfo, m_pitch_depth));
	save_item(STRUCT_MEMBER(m_lfo, m_r_type_step_pitch));
	save_item(STRUCT_MEMBER(m_lfo, m_r_amplitude));

	save_item(NAME(m_internal_adr));

	save_item(NAME(m_wave_adr));
	save_item(NAME(m_wave_size));
	save_item(NAME(m_wave_access));
	save_item(NAME(m_wave_val));

	save_item(STRUCT_MEMBER(m_mixer, vol));
	save_item(STRUCT_MEMBER(m_mixer, route));
	save_item(NAME(m_melo));
	save_item(NAME(m_meli));
	save_item(NAME(m_adc));

	save_item(STRUCT_MEMBER(*m_meg, m_program));
	save_item(STRUCT_MEMBER(*m_meg, m_const));
	save_item(STRUCT_MEMBER(*m_meg, m_offset));
	save_item(STRUCT_MEMBER(*m_meg, m_lfo));
	save_item(STRUCT_MEMBER(*m_meg, m_lfo_counter));
	save_item(STRUCT_MEMBER(*m_meg, m_lfo_increment));
	save_item(STRUCT_MEMBER(*m_meg, m_map));
	save_item(STRUCT_MEMBER(*m_meg, m_ram_read));
	save_item(STRUCT_MEMBER(*m_meg, m_ram_write));
	save_item(STRUCT_MEMBER(*m_meg, m_ram_index));
	save_item(STRUCT_MEMBER(*m_meg, m_program_address));
	save_item(STRUCT_MEMBER(*m_meg, m_m));
	save_item(STRUCT_MEMBER(*m_meg, m_r));
	save_item(STRUCT_MEMBER(*m_meg, m_t));
	save_item(STRUCT_MEMBER(*m_meg, m_p));
	save_item(STRUCT_MEMBER(*m_meg, m_mw_value));
	save_item(STRUCT_MEMBER(*m_meg, m_mw_reg));
	save_item(STRUCT_MEMBER(*m_meg, m_rw_value));
	save_item(STRUCT_MEMBER(*m_meg, m_rw_reg));
	save_item(STRUCT_MEMBER(*m_meg, m_index_value));
	save_item(STRUCT_MEMBER(*m_meg, m_index_active));
	save_item(STRUCT_MEMBER(*m_meg, m_memw_value));
	save_item(STRUCT_MEMBER(*m_meg, m_memw_active));
	save_item(STRUCT_MEMBER(*m_meg, m_memr_value));
	save_item(STRUCT_MEMBER(*m_meg, m_memr_active));
	save_item(STRUCT_MEMBER(*m_meg, m_delay_3));
	save_item(STRUCT_MEMBER(*m_meg, m_delay_2));
	save_item(STRUCT_MEMBER(*m_meg, m_sample_counter));
	save_item(STRUCT_MEMBER(*m_meg, m_retval));
}

void swp30_device::meg_state::reset()
{
	std::fill(m_program.begin(),       m_program.end(),       0);
	std::fill(m_const.begin(),         m_const.end(),         0);
	std::fill(m_offset.begin(),        m_offset.end(),        0);
	std::fill(m_lfo.begin(),           m_lfo.end(),           0);
	std::fill(m_lfo_increment.begin(), m_lfo_increment.end(), 0);
	std::fill(m_lfo_counter.begin(),   m_lfo_counter.end(),   0);
	std::fill(m_map.begin(),           m_map.end(),           0);
	m_ram_read = 0;
	m_ram_write = 0;
	m_ram_index = 0;
	m_program_address = 0;
	m_pc = 0;
	std::fill(m_m.begin(), m_m.end(), 0);
	std::fill(m_r.begin(), m_r.end(), 0);
	std::fill(m_t.begin(), m_t.end(), 0);
	m_p = 0;
	std::fill(m_mw_value.begin(),     m_mw_value.end(),     0);
	std::fill(m_mw_reg.begin(),       m_mw_reg.end(),       0);
	std::fill(m_rw_value.begin(),     m_rw_value.end(),     0);
	std::fill(m_rw_reg.begin(),       m_rw_reg.end(),       0);
	std::fill(m_index_value.begin(),  m_index_value.end(),  false);
	std::fill(m_index_active.begin(), m_index_active.end(), 0);
	std::fill(m_memw_value.begin(),   m_memw_value.end(),   false);
	std::fill(m_memw_active.begin(),  m_memw_active.end(),  0);
	std::fill(m_memr_value.begin(),   m_memr_value.end(),   false);
	std::fill(m_memr_active.begin(),  m_memr_active.end(),  0);
	m_delay_3 = 0;
	m_delay_2 = 0;
	m_sample_counter = 0;
	m_retval = 0;
}

void swp30_device::device_reset()
{
	m_keyon_mask = 0;


	std::fill(m_mixer.begin(), m_mixer.end(), mixer_slot());

	for(auto &s : m_streaming)
		s.clear();
	for(auto &f : m_filter)
		f.clear();
	for(auto &i : m_iir1)
		i.clear();
	for(auto &e : m_envelope)
		e.clear();
	for(auto &l : m_lfo)
		l.clear();

	m_meg->reset();

	m_wave_adr = 0;
	m_wave_size = 0;
	m_wave_access = 0;
	m_wave_val = 0;
	m_revram_adr = 0;
	m_revram_data = 0;
	m_revram_enable = 0;

	std::fill(m_meli.begin(),  m_meli.end(),  0);
	std::fill(m_melo.begin(),  m_melo.end(),  0);
	std::fill(m_adc.begin(),   m_adc.end(),   0);
}

void swp30_device::map(address_map &map)
{
	map(0x0000, 0x1fff).w(FUNC(swp30_device::snd_w));

	rchan(map, 0x00).rw(FUNC(swp30_device::filter_1_a_r), FUNC(swp30_device::filter_1_a_w));
	rchan(map, 0x01).rw(FUNC(swp30_device::level_1_r), FUNC(swp30_device::level_1_w));
	rchan(map, 0x02).rw(FUNC(swp30_device::filter_2_a_r), FUNC(swp30_device::filter_2_a_w));
	rchan(map, 0x03).rw(FUNC(swp30_device::level_2_r), FUNC(swp30_device::level_2_w));
	rchan(map, 0x04).rw(FUNC(swp30_device::filter_b_r), FUNC(swp30_device::filter_b_w));
	rchan(map, 0x05).rw(FUNC(swp30_device::lfo_amplitude_r), FUNC(swp30_device::lfo_amplitude_w));
	rchan(map, 0x06).rw(FUNC(swp30_device::attack_r), FUNC(swp30_device::attack_w));
	rchan(map, 0x07).rw(FUNC(swp30_device::decay1_r), FUNC(swp30_device::decay1_w));
	rchan(map, 0x08).rw(FUNC(swp30_device::decay2_r), FUNC(swp30_device::decay2_w));
	rchan(map, 0x09).rw(FUNC(swp30_device::release_glo_r), FUNC(swp30_device::release_glo_w));
	rchan(map, 0x0a).rw(FUNC(swp30_device::lfo_type_step_pitch_r), FUNC(swp30_device::lfo_type_step_pitch_w));
	// 0b-0d missing
	// 10 missing
	rchan(map, 0x11).rw(FUNC(swp30_device::pitch_r), FUNC(swp30_device::pitch_w));
	rchan(map, 0x12).rw(FUNC(swp30_device::start_h_r), FUNC(swp30_device::start_h_w));
	rchan(map, 0x13).rw(FUNC(swp30_device::start_l_r), FUNC(swp30_device::start_l_w));
	rchan(map, 0x14).rw(FUNC(swp30_device::loop_h_r), FUNC(swp30_device::loop_h_w));
	rchan(map, 0x15).rw(FUNC(swp30_device::loop_l_r), FUNC(swp30_device::loop_l_w));
	rchan(map, 0x16).rw(FUNC(swp30_device::address_h_r), FUNC(swp30_device::address_h_w));
	rchan(map, 0x17).rw(FUNC(swp30_device::address_l_r), FUNC(swp30_device::address_l_w));
	rchan(map, 0x20).rw(FUNC(swp30_device::a1_r<0>), FUNC(swp30_device::a1_w<0>));
	rchan(map, 0x22).rw(FUNC(swp30_device::b1_r<0>), FUNC(swp30_device::b1_w<0>));
	rchan(map, 0x24).rw(FUNC(swp30_device::a0_r<0>), FUNC(swp30_device::a0_w<0>));
	rchan(map, 0x26).rw(FUNC(swp30_device::a1_r<1>), FUNC(swp30_device::a1_w<1>));
	rchan(map, 0x28).rw(FUNC(swp30_device::b1_r<1>), FUNC(swp30_device::b1_w<1>));
	rchan(map, 0x2a).rw(FUNC(swp30_device::a0_r<1>), FUNC(swp30_device::a0_w<1>));
	// 2c-2f missing

	// Control registers
	// These appear as channel slots 0x0e and 0x0f
	// 00-01 missing
	rctrl(map, 0x02).rw(FUNC(swp30_device::internal_adr_r), FUNC(swp30_device::internal_adr_w));
	rctrl(map, 0x03).r (FUNC(swp30_device::internal_r));
	rctrl(map, 0x04).rw(FUNC(swp30_device::wave_adr_r<1>), FUNC(swp30_device::wave_adr_w<1>));
	rctrl(map, 0x05).rw(FUNC(swp30_device::wave_adr_r<0>), FUNC(swp30_device::wave_adr_w<0>));
	rctrl(map, 0x06).rw(FUNC(swp30_device::wave_size_r<1>), FUNC(swp30_device::wave_size_w<1>));
	rctrl(map, 0x07).rw(FUNC(swp30_device::wave_size_r<0>), FUNC(swp30_device::wave_size_w<0>));
	rctrl(map, 0x08).rw(FUNC(swp30_device::wave_access_r), FUNC(swp30_device::wave_access_w));
	rctrl(map, 0x09).r (FUNC(swp30_device::wave_busy_r));
	rctrl(map, 0x0a).rw(FUNC(swp30_device::wave_val_r<1>), FUNC(swp30_device::wave_val_w<1>));
	rctrl(map, 0x0b).rw(FUNC(swp30_device::wave_val_r<0>), FUNC(swp30_device::wave_val_w<0>));
	rctrl(map, 0x0c).rw(FUNC(swp30_device::keyon_mask_r<3>), FUNC(swp30_device::keyon_mask_w<3>));
	rctrl(map, 0x0d).rw(FUNC(swp30_device::keyon_mask_r<2>), FUNC(swp30_device::keyon_mask_w<2>));
	rctrl(map, 0x0e).rw(FUNC(swp30_device::keyon_mask_r<1>), FUNC(swp30_device::keyon_mask_w<1>));
	rctrl(map, 0x0f).rw(FUNC(swp30_device::keyon_mask_r<0>), FUNC(swp30_device::keyon_mask_w<0>));
	rctrl(map, 0x10).rw(FUNC(swp30_device::keyon_r), FUNC(swp30_device::keyon_w));
	// 11-1f missing
	rctrl(map, 0x20).w (FUNC(swp30_device::meg_lfo_commit_w));
	rctrl(map, 0x21).rw(FUNC(swp30_device::meg_prg_address_r), FUNC(swp30_device::meg_prg_address_w));
	rctrl(map, 0x22).rw(FUNC(swp30_device::meg_prg_r<0>), FUNC(swp30_device::meg_prg_w<0>));
	rctrl(map, 0x23).rw(FUNC(swp30_device::meg_prg_r<1>), FUNC(swp30_device::meg_prg_w<1>));
	rctrl(map, 0x24).rw(FUNC(swp30_device::meg_prg_r<2>), FUNC(swp30_device::meg_prg_w<2>));
	rctrl(map, 0x25).rw(FUNC(swp30_device::meg_prg_r<3>), FUNC(swp30_device::meg_prg_w<3>));

	rctrl(map, 0x30).rw(FUNC(swp30_device::meg_map_r<0>), FUNC(swp30_device::meg_map_w<0>));
	rctrl(map, 0x32).rw(FUNC(swp30_device::meg_map_r<1>), FUNC(swp30_device::meg_map_w<1>));
	rctrl(map, 0x34).rw(FUNC(swp30_device::meg_map_r<2>), FUNC(swp30_device::meg_map_w<2>));
	rctrl(map, 0x36).rw(FUNC(swp30_device::meg_map_r<3>), FUNC(swp30_device::meg_map_w<3>));
	rctrl(map, 0x38).rw(FUNC(swp30_device::meg_map_r<4>), FUNC(swp30_device::meg_map_w<4>));
	rctrl(map, 0x3a).rw(FUNC(swp30_device::meg_map_r<5>), FUNC(swp30_device::meg_map_w<5>));
	rctrl(map, 0x3c).rw(FUNC(swp30_device::meg_map_r<6>), FUNC(swp30_device::meg_map_w<6>));
	rctrl(map, 0x3e).rw(FUNC(swp30_device::meg_map_r<7>), FUNC(swp30_device::meg_map_w<7>));
	rctrl(map, 0x40).w (FUNC(swp30_device::revram_enable_w));
	rctrl(map, 0x41).w (FUNC(swp30_device::revram_clear_w));
	rctrl(map, 0x42).r (FUNC(swp30_device::revram_status_r));
	rctrl(map, 0x4a).w (FUNC(swp30_device::revram_adr_w<1>));
	rctrl(map, 0x4b).w (FUNC(swp30_device::revram_adr_w<0>));
	rctrl(map, 0x4c).rw(FUNC(swp30_device::revram_data_r<1>), FUNC(swp30_device::revram_data_w<1>));
	rctrl(map, 0x4d).rw(FUNC(swp30_device::revram_data_r<0>), FUNC(swp30_device::revram_data_w<0>));

	// MEG registers
	rchan(map, 0x21).rw(FUNC(swp30_device::meg_const_r<0>), FUNC(swp30_device::meg_const_w<0>));
	rchan(map, 0x23).rw(FUNC(swp30_device::meg_const_r<1>), FUNC(swp30_device::meg_const_w<1>));
	rchan(map, 0x25).rw(FUNC(swp30_device::meg_const_r<2>), FUNC(swp30_device::meg_const_w<2>));
	rchan(map, 0x27).rw(FUNC(swp30_device::meg_const_r<3>), FUNC(swp30_device::meg_const_w<3>));
	rchan(map, 0x29).rw(FUNC(swp30_device::meg_const_r<4>), FUNC(swp30_device::meg_const_w<4>));
	rchan(map, 0x2b).rw(FUNC(swp30_device::meg_const_r<5>), FUNC(swp30_device::meg_const_w<5>));
	rchan(map, 0x30).rw(FUNC(swp30_device::meg_offset_r<0>), FUNC(swp30_device::meg_offset_w<0>));
	rchan(map, 0x31).rw(FUNC(swp30_device::meg_offset_r<1>), FUNC(swp30_device::meg_offset_w<1>));
	rchan(map, 0x3e).rw(FUNC(swp30_device::meg_lfo_r<0>), FUNC(swp30_device::meg_lfo_w<0>));
	rchan(map, 0x3f).rw(FUNC(swp30_device::meg_lfo_r<1>), FUNC(swp30_device::meg_lfo_w<1>));

	// Mixer registers
	rchan(map, 0x32).rw(FUNC(swp30_device::vol_r  <0x00|0>), FUNC(swp30_device::vol_w  <0x00|0>));
	rchan(map, 0x33).rw(FUNC(swp30_device::vol_r  <0x00|1>), FUNC(swp30_device::vol_w  <0x00|1>));
	rchan(map, 0x34).rw(FUNC(swp30_device::vol_r  <0x00|2>), FUNC(swp30_device::vol_w  <0x00|2>));
	rchan(map, 0x35).rw(FUNC(swp30_device::route_r<0x00|0>), FUNC(swp30_device::route_w<0x00|0>));
	rchan(map, 0x36).rw(FUNC(swp30_device::route_r<0x00|1>), FUNC(swp30_device::route_w<0x00|1>));
	rchan(map, 0x37).rw(FUNC(swp30_device::route_r<0x00|2>), FUNC(swp30_device::route_w<0x00|2>));
	rchan(map, 0x38).rw(FUNC(swp30_device::vol_r  <0x40|0>), FUNC(swp30_device::vol_w  <0x40|0>));
	rchan(map, 0x39).rw(FUNC(swp30_device::vol_r  <0x40|1>), FUNC(swp30_device::vol_w  <0x40|1>));
	rchan(map, 0x3a).rw(FUNC(swp30_device::vol_r  <0x40|2>), FUNC(swp30_device::vol_w  <0x40|2>));
	rchan(map, 0x3b).rw(FUNC(swp30_device::route_r<0x40|0>), FUNC(swp30_device::route_w<0x40|0>));
	rchan(map, 0x3c).rw(FUNC(swp30_device::route_r<0x40|1>), FUNC(swp30_device::route_w<0x40|1>));
	rchan(map, 0x3d).rw(FUNC(swp30_device::route_r<0x40|2>), FUNC(swp30_device::route_w<0x40|2>));
}

// Control registers
template<int sel> u16 swp30_device::keyon_mask_r()
{
	return m_keyon_mask >> (16*sel);
}

template<int sel> void swp30_device::keyon_mask_w(u16 data)
{
	m_keyon_mask = (m_keyon_mask & ~(u64(0xffff) << (16*sel))) | (u64(data) << (16*sel));
}

u16 swp30_device::keyon_r()
{
	return 0;
}

void swp30_device::keyon_w(u16)
{
	for(int chan=0; chan<64; chan++) {
		u64 mask = u64(1) << chan;
		if(m_keyon_mask & mask) {
			m_streaming[chan].keyon();
			m_filter   [chan].keyon();
			m_iir1     [chan].keyon();
			m_envelope [chan].keyon();
			m_lfo      [chan].keyon(machine());

			if(1)
				logerror("[%08d] keyon %02x %s\n", m_meg->m_sample_counter, chan, m_streaming[chan].describe());
		}
	}
	m_keyon_mask = 0;
}


u16 swp30_device::meg_state::prg_address_r()
{
	return m_program_address;
}

void swp30_device::meg_state::prg_address_w(u16 data)
{
	m_program_address = data;
	if(m_program_address >= 0x180)
		m_program_address = 0;
}

template<int sel> u16 swp30_device::meg_state::prg_r()
{
	constexpr offs_t shift = 48-16*sel;
	return m_program[m_program_address] >> shift;
}

template<int sel> void swp30_device::meg_state::prg_w(u16 data)
{
	constexpr offs_t shift = 48-16*sel;
	constexpr u64 mask = ~(u64(0xffff) << shift);
	m_program[m_program_address] = (m_program[m_program_address] & mask) | (u64(data) << shift);

	if(sel == 3) {
		m_program_address ++;
		if(m_program_address == 0x180)
			m_program_address = 0;
	}
}

template<int sel> u16 swp30_device::meg_state::map_r()
{
	return m_map[sel];
}

template<int sel> void swp30_device::meg_state::map_w(u16 data)
{
	m_map[sel] = data;
}



u16 swp30_device::meg_prg_address_r()
{
	return m_meg->prg_address_r();
}

void swp30_device::meg_prg_address_w(u16 data)
{
	m_meg->prg_address_w(data);
}

template<int sel> u16 swp30_device::meg_prg_r()
{
	return m_meg->prg_r<sel>();
}

template<int sel> void swp30_device::meg_prg_w(u16 data)
{
	m_meg->prg_w<sel>(data);
	m_meg_program_changed = true;
}


template<int sel> u16 swp30_device::meg_map_r()
{
	return m_meg->map_r<sel>();
}

template<int sel> void swp30_device::meg_map_w(u16 data)
{
	m_meg->map_w<sel>(data);
}


template<int sel> void swp30_device::wave_adr_w(u16 data)
{
	if(sel)
		m_wave_adr = (m_wave_adr & 0x0000ffff) | (data << 16);
	else
		m_wave_adr = (m_wave_adr & 0xffff0000) |  data;
	logerror("wave_adr_w %08x\n", m_wave_adr);
}

template<int sel> u16 swp30_device::wave_adr_r()
{
	return m_wave_adr >> (16*sel);
}

template<int sel> void swp30_device::wave_size_w(u16 data)
{
	if(sel)
		m_wave_size = (m_wave_size & 0x0000ffff) | (data << 16);
	else
		m_wave_size = (m_wave_size & 0xffff0000) |  data;
	logerror("wave_size_w %08x\n", m_wave_size);
}

template<int sel> u16 swp30_device::wave_size_r()
{
	return m_wave_size >> (16*sel);
}

void swp30_device::wave_access_w(u16 data)
{
	m_wave_access = data;
	logerror("wave_access_w %04x\n", m_wave_access);
	if(data == 0x8000) {
		m_wave_val = m_wave_cache.read_dword(m_wave_adr);
		logerror("wave read adr=%08x size=%08x -> %08x\n", m_wave_adr, m_wave_size, m_wave_val);
	}
}

u16 swp30_device::wave_access_r()
{
	return m_wave_access;
}

u16 swp30_device::wave_busy_r()
{
	return m_wave_size ? 0 : 0xffff;
}

template<int sel> u16 swp30_device::wave_val_r()
{
	return m_wave_val >> (16*sel);
}

template<int sel> void swp30_device::wave_val_w(u16 data)
{
	if(sel)
		m_wave_val = (m_wave_val & 0x0000ffff) | (data << 16);
	else
		m_wave_val = (m_wave_val & 0xffff0000) |  data;
	if(!sel) {
		//      logerror("wave_val_w %08x\n", m_wave_val);
		if(m_wave_access == 0x5000) {
			m_wave_cache.write_dword(m_wave_adr, m_wave_val);
			m_wave_adr ++;
			m_wave_size --;
		}
	}
}

// Encoding of the 27-bits sample values into 16-bits values to store
// and retrieve from the reverb ram.  Technically they're supposed to
// be 18-bits but the two low bits are never connected to anything.

u16 swp30_device::meg_state::revram_encode(u32 v)
{
	v &= 0x7ffffff;
	u32 s = 0;
	if(v & 0x4000000) {
		v ^= 0x7ffffff;
		s = 1;
	}
	u32 e = 15;
	while(e && !(v & (0x400 << e)))
		e --;
	u32 m = e ? (v >> (e-1)) & 0x7ff : v;
	return (e << 12) | (s << 11) | m;
}

u32 swp30_device::meg_state::revram_decode(u16 v)
{
	u32 e = (v >> 12) & 15;
	u32 s = (v >> 11) & 1;
	u32 m = v & 0x7ff;
	u32 vb = e ? (m | 0x800) << (e-1) : m;
	if(s)
		vb ^= e ? (0xffffffff << (e-1)) & 0xffffffff : 0xffffffe0;
	return vb;
}


void swp30_device::revram_enable_w(u16 data)
{
	logerror("revram enable = %04x\n", data);
	m_revram_enable = data;
}

void swp30_device::revram_clear_w(u16 data)
{
	logerror("revram clear = %04x\n", data);
}

u16 swp30_device::revram_status_r()
{
	return 0;
}

template<int sel> void swp30_device::revram_adr_w(u16 data)
{
	if(sel)
		m_revram_adr = (m_revram_adr & 0x0000ffff) | (data << 16);
	else
		m_revram_adr = (m_revram_adr & 0xffff0000) |  data;
}

template<int sel> void swp30_device::revram_data_w(u16 data)
{
	if(sel)
		m_revram_data = (m_revram_data & 0x0000ffff) | (data << 16);
	else
		m_revram_data = (m_revram_data & 0xffff0000) |  data;

	if(!sel)
		m_reverb->write_word(m_revram_adr, meg_state::revram_encode(m_revram_data >> 5));
}

template<int sel> u16 swp30_device::revram_data_r()
{
	if(sel)
		m_revram_data = meg_state::revram_decode(m_reverb->read_word(m_revram_adr)) << 5;

	return sel ? m_revram_data >> 16 : m_revram_data;
}



// Streaming block trampolines
u16 swp30_device::pitch_r(offs_t offset)
{
	return m_streaming[offset >> 6].pitch_r();
}

void swp30_device::pitch_w(offs_t offset, u16 data)
{
	m_streaming[offset >> 6].pitch_w(data);
}

u16 swp30_device::start_h_r(offs_t offset)
{
	return m_streaming[offset >> 6].start_h_r();
}

u16 swp30_device::start_l_r(offs_t offset)
{
	return m_streaming[offset >> 6].start_l_r();
}

void swp30_device::start_h_w(offs_t offset, u16 data)
{
	m_streaming[offset >> 6].start_h_w(data);
}

void swp30_device::start_l_w(offs_t offset, u16 data)
{
	m_streaming[offset >> 6].start_l_w(data);
}

u16 swp30_device::loop_h_r(offs_t offset)
{
	return m_streaming[offset >> 6].loop_h_r();
}

u16 swp30_device::loop_l_r(offs_t offset)
{
	return m_streaming[offset >> 6].loop_l_r();
}

void swp30_device::loop_h_w(offs_t offset, u16 data)
{
	m_streaming[offset >> 6].loop_h_w(data);
}

void swp30_device::loop_l_w(offs_t offset, u16 data)
{
	m_streaming[offset >> 6].loop_l_w(data);
}

u16 swp30_device::address_h_r(offs_t offset)
{
	return m_streaming[offset >> 6].address_h_r();
}

u16 swp30_device::address_l_r(offs_t offset)
{
	return m_streaming[offset >> 6].address_l_r();
}

void swp30_device::address_h_w(offs_t offset, u16 data)
{
	m_streaming[offset >> 6].address_h_w(data);
}

void swp30_device::address_l_w(offs_t offset, u16 data)
{
	m_streaming[offset >> 6].address_l_w(data);
}


// IIR block trampolines
u16 swp30_device::filter_1_a_r(offs_t offset)
{
	return m_filter[offset >> 6].filter_1_a_r();
}

void swp30_device::filter_1_a_w(offs_t offset, u16 data)
{
	m_filter[offset >> 6].filter_1_a_w(data);
}

u16 swp30_device::level_1_r(offs_t offset)
{
	return m_filter[offset >> 6].level_1_r();
}

void swp30_device::level_1_w(offs_t offset, u16 data)
{
	m_filter[offset >> 6].level_1_w(data);
}

u16 swp30_device::filter_2_a_r(offs_t offset)
{
	return m_filter[offset >> 6].filter_2_a_r();
}

void swp30_device::filter_2_a_w(offs_t offset, u16 data)
{
	m_filter[offset >> 6].filter_2_a_w(data);
}

u16 swp30_device::level_2_r(offs_t offset)
{
	return m_filter[offset >> 6].level_2_r();
}

void swp30_device::level_2_w(offs_t offset, u16 data)
{
	m_filter[offset >> 6].level_2_w(data);
}

u16 swp30_device::filter_b_r(offs_t offset)
{
	return m_filter[offset >> 6].filter_b_r();
}

void swp30_device::filter_b_w(offs_t offset, u16 data)
{
	m_filter[offset >> 6].filter_b_w(data);
}

// FIR block trampolines
template<u32 filter> u16 swp30_device::a0_r(offs_t offset)
{
	return m_iir1[offset >> 6].a0_r<filter>();
}

template<u32 filter> u16 swp30_device::a1_r(offs_t offset)
{
	return m_iir1[offset >> 6].a1_r<filter>();
}

template<u32 filter> u16 swp30_device::b1_r(offs_t offset)
{
	return m_iir1[offset >> 6].b1_r<filter>();
}

template<u32 filter> void swp30_device::a0_w(offs_t offset, u16 data)
{
	m_iir1[offset >> 6].a0_w<filter>(data);
}

template<u32 filter> void swp30_device::a1_w(offs_t offset, u16 data)
{
	m_iir1[offset >> 6].a1_w<filter>(data);
}

template<u32 filter> void swp30_device::b1_w(offs_t offset, u16 data)
{
	m_iir1[offset >> 6].b1_w<filter>(data);
}

// Envelope block trampolines
u16 swp30_device::attack_r(offs_t offset)
{
	return m_envelope[offset >> 6].attack_r();
}

void swp30_device::attack_w(offs_t offset, u16 data)
{
	m_envelope[offset >> 6].attack_w(data);
}

u16 swp30_device::decay1_r(offs_t offset)
{
	return m_envelope[offset >> 6].decay1_r();
}

void swp30_device::decay1_w(offs_t offset, u16 data)
{
	m_envelope[offset >> 6].decay1_w(data);
}

u16 swp30_device::decay2_r(offs_t offset)
{
	return m_envelope[offset >> 6].decay2_r();
}

void swp30_device::decay2_w(offs_t offset, u16 data)
{
	m_envelope[offset >> 6].decay2_w(data);
}

u16 swp30_device::release_glo_r(offs_t offset)
{
	return m_envelope[offset >> 6].release_glo_r();
}

void swp30_device::release_glo_w(offs_t offset, u16 data)
{
	m_envelope[offset >> 6].release_glo_w(data);
}




template<int sel> u16 swp30_device::vol_r(offs_t offset)
{
	return m_mixer[(sel & 0x40) | (offset >> 6)].vol[sel & 3];
}

template<int sel> void swp30_device::vol_w(offs_t offset, u16 data)
{
	m_mixer[(sel & 0x40) | (offset >> 6)].vol[sel & 3] = data;
}

template<int sel> u16 swp30_device::route_r(offs_t offset)
{
	return m_mixer[(sel & 0x40) | (offset >> 6)].route[sel & 3];
}

template<int sel> void swp30_device::route_w(offs_t offset, u16 data)
{
	m_mixer[(sel & 0x40) | (offset >> 6)].route[sel & 3] = data;
}

u16 swp30_device::lfo_type_step_pitch_r(offs_t offset)
{
	return m_lfo[offset >> 6].type_step_pitch_r();
}

void swp30_device::lfo_type_step_pitch_w(offs_t offset, u16 data)
{
	m_lfo[offset >> 6].type_step_pitch_w(data);
}

u16 swp30_device::lfo_amplitude_r(offs_t offset)
{
	return m_lfo[offset >> 6].amplitude_r();
}

void swp30_device::lfo_amplitude_w(offs_t offset, u16 data)
{
	m_lfo[offset >> 6].amplitude_w(data);
}

u16 swp30_device::internal_adr_r()
{
	return m_internal_adr;
}

void swp30_device::internal_adr_w(u16 data)
{
	m_internal_adr = data;
}

u16 swp30_device::internal_r()
{
	u8 chan = m_internal_adr & 0x3f;
	switch(m_internal_adr >> 8) {
	case 0:
		return m_envelope[chan].status();

	case 4:
		// used at 44c4
		// tests & 0x4000 only
		//      logerror("read %02x.4\n", chan);
		return 0x0000;

	case 6:
		return 0x8000;
	}

	logerror("%s internal_r port %x channel %02x sample %d\n", machine().time().to_string(), m_internal_adr >> 8, m_internal_adr & 0x1f, m_meg->m_sample_counter);

	return 0;
}


// Catch-all

void swp30_device::snd_w(offs_t offset, u16 data)
{
	int chan = (offset >> 6) & 0x3f;
	int slot = offset & 0x3f;

	if(slot == 0x0b)
		return;

	std::string preg = "-";
	if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
		preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
	else if(slot == 0x0e || slot == 0x0f)
		preg = util::string_format("sy%02x", (slot-0x0e) + 2*chan);
	else if(slot == 0x30 || slot == 0x31)
		preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
	else if(slot >= 0x38 && slot <= 0x3a)
		preg = util::string_format("mix[%x, %02x]", slot - 0x38, chan);
	else if(slot >= 0x3b && slot <= 0x3d)
		preg = util::string_format("route[%x, %02x]", slot - 0x3b, chan);
	else if(slot == 0x3e || slot == 0x3f)
		preg = util::string_format("lfo[%02x]", (slot-0x3e) + 2*chan);
	else
		preg = util::string_format("%02x.%02x", chan, slot);

	logerror("snd_w [%04x %04x] %-5s, %04x\n", offset, offset*2, preg, data);
}



// Synthesis and meg

uint32_t swp30_device::execute_min_cycles() const noexcept
{
	return 1;
}

uint32_t swp30_device::execute_max_cycles() const noexcept
{
	return 1;
}


void swp30_device::meg_prg_map(address_map &map)
{
	map(0x000, 0x17f).r(FUNC(swp30_device::meg_prg_map_r));
}

u64 swp30_device::meg_prg_map_r(offs_t address)
{
	return m_meg->m_program[address];
}

void swp30_device::meg_reverb_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}

u16 swp30_device::swp30d_const_r(u16 address) const
{
	return m_meg->m_const[address];
}

u16 swp30_device::swp30d_offset_r(u16 address) const
{
	return m_meg->m_offset[address];
}

device_memory_interface::space_config_vector swp30_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_wave_config),
		std::make_pair(AS_REVERB,  &m_reverb_config),
	};
}

std::unique_ptr<util::disasm_interface> swp30_device::create_disassembler()
{
	return std::make_unique<swp30_disassembler>(this);
}

void swp30_device::state_import(const device_state_entry &entry)
{
}

void swp30_device::state_export(const device_state_entry &entry)
{
}

void swp30_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
}

//======================= Mixer block ============================================

//   ssssss 110010  Mixer            llll llll rrrr rrrr                      Route attenuation left/right input s
//   ssssss 110011  Mixer            0000 0000 1111 1111                      Route attenuation slot 0/1   input s
//   ssssss 110100  Mixer            2222 2222 3333 3333                      Route attenuation slot 2/3   input s
//   ssssss 110101  Mixer            fedc ba98 7654 3210                      Route mode bit 2 input s output 0-f
//   ssssss 110110  Mixer            fedc ba98 7654 3210                      Route mode bit 1 input s output 0-f
//   ssssss 110111  Mixer            fedc ba98 7654 3210                      Route mode bit 0 input s output 0-f
//   ssssss 111000  Mixer            llll llll rrrr rrrr                      Route attenuation left/right input s+40
//   ssssss 111001  Mixer            0000 0000 1111 1111                      Route attenuation slot 0/1   input s+40
//   ssssss 111010  Mixer            2222 2222 3333 3333                      Route attenuation slot 2/3   input s+40
//   ssssss 111011  Mixer            fedc ba98 7654 3210                      Route mode bit 2 input s+40 output 0-f
//   ssssss 111100  Mixer            fedc ba98 7654 3210                      Route mode bit 1 input s+40 output 0-f
//   ssssss 111101  Mixer            fedc ba98 7654 3210                      Route mode bit 0 input s+40 output 0-f

// The mixer block ensures mixing and routing in the whole system,
// between the AM2, the MEG, and the MELI/MELO streams.  The values
// passing through are all 27-bits wide.

// It has 96 mono inputs:
//   - 64 outputs of the AWM2 block, numbered 0-63
//   - 16 outputs of the MEG, numbered 64-79, which are read from MEG
//     registers m20-m2f
//   - 16 inputs (8 stereo) on the MELI ports, numbered 80-95

// It has 16 stereo outputs:
//   - 8 outputs on the MELO ports, numbered 0-7
//   - 8 outputs to the MEG as 16 mono streams, numbered 8-15, which
//     are written to MEG registers m20-m2f

// Six 8-bit values provide attenuations, and three 16-bits values
// provide routing for each of the 96 inputs to each of the 16
// outputs.

// For a given source, target pair the three bits of routing target
// are interpreted following in the following way:

//          210
//       0: 000 - Not routed
//       1: 001 - No attenuation, add to both channels
//       2: 010 - No attenuation, add to left channel
//       3: 011 - No attenuation, add to right channel
//       4: 100 - Use attenuation slot 0
//       5: 101 - Use attenuation slot 1
//       6: 110 - Use attenuation slot 2
//       7: 111 - Use attenuation slot 3

// The attenuation slots are built from the six attenuation values.
// Attenuation for a given channel (left/right) and a slot (0-3) is
// the sum of the left/right attenuation and the slot attenuation.
// Final value is 4.4 with >= ff hardcoded to mute.

// There is space in the map for channels number 96-127.  The MUs
// never touch that space, it seems that it may have (mostly negative)
// impacts on the adc outputs (MEG registers m30-m33).


s32 swp30_device::mixer_att(s32 sample, s32 att)
{
	if(att >= 0xff)
		return 0;
	return (sample - ((sample * (att & 0xf)) >> 4)) >> (att >> 4);
}

void swp30_device::mixer_step(const std::array<s32, 0x40> &samples_per_chan)
{
	std::array<s32, 0x20> mixer_out;
	std::fill(mixer_out.begin(), mixer_out.end(), 0);

	for(int mix = 0; mix != 0x60; mix++) {
		u64 route = (u64(m_mixer[mix].route[0]) << 32) | (u64(m_mixer[mix].route[1]) << 16) | m_mixer[mix].route[2];
		if(route == 0)
			continue;

		s32 input;
		if(mix < 0x40)
			input = samples_per_chan[mix];
		else if(mix < 0x50)
			input = m_meg->m_m[0x20 | (mix & 0xf)];
		else
			input = m_meli[mix & 0xf];

		if(input == 0)
			continue;

		const std::array<u16, 3> &vol = m_mixer[mix].vol;
		for(int out = 0; out != 16; out++) {
			int mode = ((route >> (out+32-2)) & 4) | ((route >> (out+16-1)) & 2) | ((route >> (out+0-0)) & 1);
			switch(mode) {
			case 0: // No routing
				break;

			case 1: // No attenuation, add to both channels
				mixer_out[out*2  ] += input;
				mixer_out[out*2+1] += input;
				break;

			case 2: // No attenuation, add to left channel
				mixer_out[out*2  ] += input;
				break;

			case 3: // No attenuation, add to right channel
				mixer_out[out*2+1] += input;
				break;

			case 4: // Use attenuation slot 0
				mixer_out[out*2  ] += mixer_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
				mixer_out[out*2+1] += mixer_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
				break;

			case 5: // Use attenuation slot 1
				mixer_out[out*2  ] += mixer_att(input, (vol[0] >> 8)   + (vol[1] & 0xff));
				mixer_out[out*2+1] += mixer_att(input, (vol[0] & 0xff) + (vol[1] & 0xff));
				break;

			case 6: // Use attenuation slot 2
				mixer_out[out*2  ] += mixer_att(input, (vol[0] >> 8)   + (vol[2] >> 8));
				mixer_out[out*2+1] += mixer_att(input, (vol[0] & 0xff) + (vol[2] >> 8));
				break;

			case 7: // Use attenuation slot 3
				mixer_out[out*2  ] += mixer_att(input, (vol[0] >> 8)   + (vol[2] & 0xff));
				mixer_out[out*2+1] += mixer_att(input, (vol[0] & 0xff) + (vol[2] & 0xff));
				break;
			}
		}
	}
	std::copy(mixer_out.begin() + 0x00, mixer_out.begin() + 0x10, m_melo.begin());
	std::copy(mixer_out.begin() + 0x10, mixer_out.begin() + 0x20, m_meg->m_m.begin() + 0x20);
}


//     MEG:

//   010000 001110  MEG/Control      .... .... .... ....    commit LFO increments on write
//   010000 001111  MEG/Control      .... ...a aaaa aaaa    program address
//   010001 00111*  MEG/Control      dddd dddd dddd dddd    program data 1/2
//   010010 00111*  MEG/Control      dddd dddd dddd dddd    program data 2/2

//   aaaaaa 100001  MEG/Data         cccc cccc cccc cccc    constant index 6*a + 0
//   aaaaaa 100011  MEG/Data         cccc cccc cccc cccc    constant index 6*a + 1
//   aaaaaa 100101  MEG/Data         cccc cccc cccc cccc    constant index 6*a + 2
//   aaaaaa 100111  MEG/Data         cccc cccc cccc cccc    constant index 6*a + 3
//   aaaaaa 101001  MEG/Data         cccc cccc cccc cccc    constant index 6*a + 4
//   aaaaaa 101011  MEG/Data         cccc cccc cccc cccc    constant index 6*a + 5
//   aaaaaa 11000a  MEG/Data         oooo oooo oooo oooo    offset index a
//   aaaaaa 11111a  MEG/LFO          pppp ttss iiii iiii    LFO index a, phase, type, shift, increment



//      General structure

//   The MEG is a DSP with 384 program steps connected to a 0x40000
//   samples ram.  Instructions are 64 bits wide, and to each
//   instruction is associated a 1.15 fixed point signed value
//   (between -1 and 1), Every third instruction (pc multiple of 3)
//   can initiate a memory access to the reverb buffer which will be
//   completed two instructions later.  Each of those instructions is
//   associated to a 16-bits address offset value.

//   The main computation unit is a MAC cell which multiplies two
//   numbers and adds a third.

//   Every 44100th of a second the 384 program steps are run once in
//   order (no branches) to compute everything.


//      Registers

//   The DSP has multiple register sets:

//   - 127 standard registers (bank 'r') numbered 01-7f, with the extra
//     register number 00 being hardwired to 0 (like in mips)

//   - 63 mmio registers (bank 'm') numbered 01-3f with 00 wired to 0,
//     which are usable as normal registers but also are used for
//     communication

//   - 8 t(emporary) registers

//   - a p register to store the result of the MAC

//   - an index register that is optionally added to the memory address

//   - two external memory data ports, one holding the value to write,
//     one holding the latest one read

//   The registers from r and m are 24 bits each, signed.  The p
//   register (and the MAC block itself) is 42 bits, 27.15.

//   The m bank is used as the communication interface with the mixer
//   and adcs.  Once every sample the registers m20 to m2f are sent as
//   stereo values to the eight MELO ports.  Registers m30 to m33 are
//   sent to the two stereo DACs.  In addition the mixer outputs to
//   the MEG are loaded in registers m20 to m2f.  That bank also
//   communicates with lfos, with the prng and with the external
//   memory data ports.


//      LFO

//   24 LFO registers are available.  The LFO registers
//   internal counters are 22 bits wide.  The LSB of the register gives
//   the increment per sample, encoded in a special 3.5 format.
//   With scale = 3bits and v = 5bits,
//     step  = base[scale] + (v << shift[scale])
//     base  = { 0, 32, 64, 128, 256, 512, 1024, 2048 }
//     shift = { 0,  0,  1,   2,   3,   4,    5,    6 }

//   The top 17 bits of the counter are extracted.  They are shifted
//   up by 0-3 bits (depending on s) and truncated at the top, then
//   the phase p selects a value to add to the state.  That gives the
//   final 17-bits state which is shaped according to the type:

//   0: sine
//   1: triangle
//   2: saw upwards
//   3: saw downwards

//   The final 16 bits value is then shifted by 7 bits to generate the
//   final positive 23-bits value.

//   Writes to the MEG/LFO register changes phase, type and shift
//   immediatly but doesn't change the increment yet.  Writing then to
//   the commit register sets all the delayed increment changes
//   simultaneously.  This allows to keep the counters from
//   independant LFOs in sync.


//      Reverb ram access

 //   8 mappings can be setup, which allow to manage rotating buffers in
//   the samples ram easily by automating masking and offset adding.  The
//   register format is: pppppsss oooooooo.  'p' is the base pc/12 at
//   which the map starts to be used.  's' is the sub-buffer size,
//   defined as 1 << (10+s).  The base offset is o << 10.  There are no
//   alignment issues, e.g. you can have a buffer at 0x28000 which is
//   0x10000 samples long.


//      Instructions

//    33333333 33333333 22222222 22222222 11111111 11111111 00000000 00000000
//    fedcba98 76543210 fedcba98 76543210 fedcba98 76543210 fedcba98 76543210
//    ABCDEFFF Grrrrrrr HHHmmmmm m-II--J- KKLLMMNN OOPPQRrr rrrrrSmm mmmm----
//    +                               + +                                ++++ = bits set at least once in the mu100 programs

//    m = low is read port, high is write port, memory register
//    r = low is read port, high is write port, regular register

//    A = seems to disable writing to p and nothing else? Used for lo-fi variation only
//    B = set index to p
//    C = set mem write register to p
//    D = temp register write enable
//    E = temp register write source, 0=const, 1=p
//    F = temp register number
//    G = r register write source, 0 = p, 1 = r register
//    H = m register write source (0, 1, 3 unknown, 2 lfo, 4 mem read, 5 rand, 6 p, 7 m register)
//    I = memory mode, none/read/write/read+1
//    J = add index to address on memory access
//    K = saturation mode (0 = none, 1 = 24.15, 2 = 0 to max positive 24.15, 3 = abs then max positive 24.15)
//    L = shift left writing to p
//    M = adder mode (0 = add, 1 = sub, 2 = add abs, 3 = binary and)
//    N = a selector (0=p, 1=r, 2=m, 3=0)
//    O = multiplier mode (0=off, 1=m1, 2=m1*m2, 3=m2)
//    P = mul 1st input = 0,3=constant, 1,2=temp register (note that 2 and 3 seem never used)
//    Q = expand 1st input
//    R = mul 2nd input = 0=r, 1=m
//    S = disable dithering when copying from p

//   The instructions are VLIW, 64-bits wide.  The VLIW structure
//   means bits of the instruction are directly associated to
//   structures in the chip (muxes, etc) instead of the usual
//   instruction encoding of normal cpus.


//      Instruction execution

//            +--------+            +--------+                   +--------+
//            | t read |            | r read |                   | m read |
//            |  port  |            |  port  |                   |  port  |
//            +----+---+            +---+----+                   +---+----+
//                 |                 r<-+                            +->m
//           +-----+-------+            | +----------------------+---+
// Constant--+ m1 selector |            +-|-------------+        |
//           +-----+-------+     +------+-+----+      +-+--------+-+
//                 |             | m2 selector |      | a selector |
//                 |             +-+-----------+      +-+--------+-+
//                 |               |                    |        |
//            +----+---+   +-------+----+      +--------+--+     |
//            | expand +---+ multiplier +------+   adder   |     |
//            +--------+   +------------+      +-----+-----+     |
//                                                   |           |
//                                              +----+----+      |
//                                              | shifter |      |
//                                              +----+----+      |
//                                                   |           |
//                                             +-----+------+    |
//                                             | saturation |    |
//                                             +-----+------+    |
//                                                   |           |
//                                                 +-+-+         |
//                                                 | p +---------+
//                                                 +-+-+
//                                                   |
//                                              +----+---+
//                                              | dither |
//                                              +----+---+
//                                                   |
//            +------+-------------+-----------+-----+---+
//            |      |             |           |         |
//         r  |      | Constant    |           |         | m  lfo, prng, mem read
//         |  |      |     |       |           |         | |  |
//     +---+--+--+ +-+-----+-+ +---+---+ +-----+-----+ +-+-+--+--+
//     | r write | | t write | | index | | mem write | | m write |
//     |  port   | |  port   | |       | | register  | |  port   |
//     +---------+ +---------+ +-------+ +-----------+ +---------+


//   Read ports are always active, but for the r and m ports if the
//   register number is 0 then the result is 0.

//   Write ports for r and m are disabled when the register number is
//   0.  T, index and mem write have explicit enable bits (D, B and C
//   respectively).  P write is disabled though the multiplier mode
//   and the A bit.  P is passthrough, e.g. if it's written to and
//   read in the same instruction the read value is the written value.

//   The m1 selector (P) chooses between the instruction-associated
//   constant and a T register.  Optionally the value can be expanded
//   from floating-point to linear (Q).  The m2 selector chooses
//   between the r and m read ports (R).  m1 and m2 are combined in
//   the multiplier block, which can multiply them together, or pass
//   m1, or pass m2 (4th case disables the write to p).

//   The a selector (N) can choose between outputting 0, m, r, p, or
//   if r or m is selected but the register number is 0, then p >> 15.
//   Then the result of the multiplier and the a selector are combined
//   in the adder, through one of four operations: add a and m, sub a
//   from m, add m to abs(a) and binary and between m and a.  Then a
//   shifter left shifts the result by 0, 1, 2 or 4 bits. Finally a
//   saturation method may be applied (K) before writing to p.  After
//   p a dither is optionally applied (S) before the value is
//   distributed to the other registers.


//   MEG quarter-sine "ROM"

//   Pretty sure it's actually a computation given how imprecise it
//   actually is (a rom would have no reason not to be perfect).  But
//   guessing what calculation gives the correct pattern of
//   imprecision is not trivial.

ROM_START( swp30 )
	ROM_REGION16_LE( 0x10000, "sintab", 0 )
	ROM_LOAD( "sin-table.bin", 0, 0x10000, CRC(4305f63c) SHA1(ab3aeacc7a6261cd77019d2f3febd2c21986bf46) )
ROM_END

const tiny_rom_entry *swp30_device::device_rom_region() const
{
	return ROM_NAME( swp30 );
}

const std::array<u32, 256> swp30_device::meg_state::lfo_increment_table = []() {
	std::array<u32, 256> increments;
	static const int dt[8] = { 0, 32, 64, 128, 256, 512,  1024, 2048 };
	static const int sh[8] = { 0,  0,  1,   2,   3,   4,     5,    6 };

	for(u32 i=0; i != 256; i++) {
		int scale = (i >> 5) & 7;
		increments[i] = ((i & 31) << sh[scale]) + dt[scale];
	}
	return increments;
}();

swp30_disassembler::swp30_disassembler(info *inf) : m_info(inf)
{
}

u32 swp30_disassembler::opcode_alignment() const
{
	return 1;
}

std::string swp30_disassembler::gconst(offs_t address) const
{
	if(!m_info)
		return util::string_format("c%03x", address);
	s16 value = m_info->swp30d_const_r(address);
	return util::string_format("%g", value / 32768.0);
}

std::string swp30_disassembler::goffset(offs_t address) const
{
	return m_info ? util::string_format("%x", m_info->swp30d_offset_r(address)) : util::string_format("of%02x", address);
}

void swp30_disassembler::append(std::string &r, const std::string &e)
{
	if(r != "")
		r += " ; ";
	r += e;
}


u16 swp30_device::meg_state::const_r(offs_t offset)
{
	return m_const[offset];
}

void swp30_device::meg_state::const_w(offs_t offset, u16 data)
{
	m_const[offset] = data;
}

u16 swp30_device::meg_state::offset_r(offs_t offset)
{
	return m_offset[offset];
}

void swp30_device::meg_state::offset_w(offs_t offset, u16 data)
{
	m_offset[offset] =  data;
}

u16 swp30_device::meg_state::lfo_r(offs_t offset)
{
	return m_lfo[offset];
}

void swp30_device::meg_state::lfo_w(offs_t offset, u16 data)
{
	m_lfo[offset] = data;
}

void swp30_device::meg_state::lfo_commit_w()
{
	for(int i=0; i != 24; i++)
		m_lfo_increment[i] = lfo_increment_table[m_lfo[i] & 0xff];
}


template<int sel> u16 swp30_device::meg_const_r(offs_t offset)
{
	return m_meg->const_r((offset >> 6)*6 + sel);
}

template<int sel> void swp30_device::meg_const_w(offs_t offset, u16 data)
{
	//  logerror("meg const[%03x] = %04x / %f\n", (offset >> 6)*6 + sel, data, s16(data) / 32768.0);
	m_meg->const_w((offset >> 6)*6 + sel, data);
}

template<int sel> u16 swp30_device::meg_offset_r(offs_t offset)
{
	return m_meg->offset_r((offset >> 6)*2 + sel);
}

template<int sel> void swp30_device::meg_offset_w(offs_t offset, u16 data)
{
	//  logerror("meg offset[%02x/%03x] = %x / %d\n", (offset >> 6)*2 + sel, 3*((offset >> 6)*2 + sel), data, data);
	m_meg->offset_w((offset >> 6)*2 + sel, data);
}

template<int sel> u16 swp30_device::meg_lfo_r(offs_t offset)
{
	return m_meg->lfo_r((offset >> 6)*2 + sel);
}

template<int sel> void swp30_device::meg_lfo_w(offs_t offset, u16 data)
{
	if((offset >> 6)*2 + sel >= 0x18) {
		logerror("nolfo[%02x] = %04x\n", (offset >> 6)*2 + sel, data);
	}
	m_meg->lfo_w((offset >> 6)*2 + sel, data);
	m_meg_program_changed = true;
}

void swp30_device::meg_lfo_commit_w(u16)
{
	m_meg->lfo_commit_w();
	m_meg_program_changed = true;
}

void swp30_device::meg_state::lfo_step()
{
	for(int i = 0; i != 24; i++)
		m_lfo_counter[i] = (m_lfo_counter[i] + m_lfo_increment[i]) & 0x3fffff;
}

u32 swp30_device::meg_state::resolve_address(u16 pc, s32 offset)
{
	u16 key = (pc / 12) << 11;
	for(int i=0; i != 8; i++)
		if(i == 7 || m_map[i+1] <= m_map[i] || ((m_map[i+1] & 0xf800) > key)) {
			u32 mask = (1 << (10+BIT(m_map[i], 8, 3))) - 1;
			return (offset & mask) + (BIT(m_map[i], 0, 8) << 10);
		}
	return 0xffffffff;
}

u32 swp30_device::meg_state::get_lfo(int lfo)
{
	static const u32 offsets[16] = {
		0x00000, 0x02aaa, 0x04000, 0x05555,
		0x08000, 0x0aaaa, 0x0c000, 0x0d555,
		0x10000, 0x12aaa, 0x14000, 0x15555,
		0x18000, 0x1aaaa, 0x1c000, 0x1d555,
	};

	u32 base = (m_lfo_counter[lfo] >> 5);
	base = base << ((m_lfo[lfo] >> 8) & 3);
	base = base + offsets[m_lfo[lfo] >> 12];
	base = base & 0x1ffff;
	u32 res;
	switch((m_lfo[lfo] >> 10) & 3) {
	case 0: // sine
		if(base < 0x8000)
			res = m_swp->m_sintab[base];
		else if(base < 0x10000)
			res = m_swp->m_sintab[(base & 0x7fff) ^ 0x7fff];
		else if(base < 0x18000)
			res = m_swp->m_sintab[base & 0x7fff] ^ 0xffff;
		else
			res = m_swp->m_sintab[(base & 0x7fff) ^ 0x7fff] ^ 0xffff;
		break;

	case 1: // tri
		res = (base + 0x8000) & 0x1ffff;
		if(res & 0x10000)
			res ^= 0x1ffff;
		break;

	case 2: // saw up
		res = base >> 1;
		break;

	case 3: // saw down
		res = (base ^ 0x1ffff) >> 1;
		break;
	}
	return res << 7;
}

// Expand the first multiplier input

s16 swp30_device::meg_state::m1_expand(s16 v)
{
	if(v < 0)
		return 0;
	u32 s = v >> 12;
	v = 0x1000 | (v & 0xfff);
	return s == 5 ? v : s < 5 ? v >> (5-s) : v << (s-5);
}

offs_t swp30_disassembler::disassemble(std::ostream &stream, offs_t pc, const data_buffer &opcodes, const data_buffer &params)
{
	std::string r;
	u64 opcode = opcodes.r64(pc);

	int sm = BIT(opcode, 0x04, 6);
	int sr = BIT(opcode, 0x0b, 7);
	int dm = BIT(opcode, 0x27, 6);
	int dr = BIT(opcode, 0x30, 7);
	int t  = BIT(opcode, 0x38, 3);

	u32 mmode = BIT(opcode, 0x16, 2);
	if(mmode != 0 && !BIT(opcode, 0x3f)) {
		u32 m1t = BIT(opcode, 0x14, 2);
		std::string mul1 = m1t == 1 || m1t == 2 ? util::string_format("t%x", BIT(opcode, 0x38, 3)) : gconst(pc);
		if(BIT(opcode, 0x13))
			mul1 = util::string_format("exp(%s)", mul1);

		std::string mul2 = BIT(opcode, 0x12) ? sm ? util::string_format("m%02x", sm) : "0" : sr ? util::string_format("r%02x", sr) : "0";

		u32 at = BIT(opcode, 0x18, 2);
		std::string aop;
		switch(at) {
		case 0:
			aop = "p";
			break;

		case 1:
			if(sr)
				aop = util::string_format("r%02x", sr);
			else
				aop = "(p >> 15)";
			break;

		case 2:
			if(sm)
				aop = util::string_format("m%02x", sm);
			else
				aop = "(p >> 15)";
			break;
		}

		std::string op;
		switch(mmode) {
		case 1:
			op = util::string_format("(%s << 8)", mul1);
			break;
		case 2:
			op = util::string_format("%s * %s", mul1, mul2);
			break;
		case 3:
			op = util::string_format("%s", mul2);
			break;
		}

		std::string aopf;
		if(at != 3)
			switch(BIT(opcode, 0x1a, 2)) {
			case 0:
				aopf = util::string_format(" + %s", aop);
				break;
			case 1:
				aopf = util::string_format(" - %s", aop);
				break;
			case 2:
				aopf = util::string_format(" + abs(%s)", aop);
				break;
			case 3:
				aopf = util::string_format(" & %s", aop);
				break;
			}

		std::string o = op + aopf;
		u32 shift = BIT(opcode, 0x1c, 2);
		if(shift)
			o = util::string_format("(%s) << %d", o, shift == 3 ? 4 : shift);

		u32 sat = BIT(opcode, 0x1e, 2);
		static const char *const satmode[4] = { "=", "=s", "=_", "=a" };

		append(r, util::string_format("p %s %s", satmode[sat], o));
	}

	if(dm) {
		std::string dst = util::string_format("m%02x", dm);
		switch(BIT(opcode, 0x2d, 3)) {
		case 0: case 1: case 2: case 3:
			append(r, util::string_format("%s = lfo.%02x", dst, pc >> 4));
			break;
		case 4: append(r, util::string_format("%s = mr", dst)); break;
		case 5: append(r, util::string_format("%s = rand", dst)); break;
		case 6: append(r, util::string_format("%s = p", dst)); break;
		case 7: append(r, util::string_format("%s = %s", dst, sm ? util::string_format("m%02x", sm) : "0")); break;
		}
	}

	if(dr) {
		if(BIT(opcode, 0x37))
			append(r, util::string_format("r%02x = r%02x", dr, sr));
		else
			append(r, util::string_format("r%02x = p", dr));
	}

	if(BIT(opcode, 0x3d))
		append(r, util::string_format("mw = p"));

	if(BIT(opcode, 0x3e))
		append(r, util::string_format("idx = p"));

	if(BIT(opcode, 0x3b)) {
		if(BIT(opcode, 0x3c))
			append(r, util::string_format("t%x = p", t));
		else
			append(r, util::string_format("t%x = %s", t, gconst(pc)));
	}

	if(BIT(opcode, 0x0a))
		append(r, "nodither");

	u32 memmode = BIT(opcode, 0x24, 2);
	if(memmode) {
		static const char *modes[4] = { nullptr, "w", "r", "1r" };
		append(r, util::string_format("mem_%s +%s%s", modes[memmode], goffset(pc/3), BIT(opcode, 0x21) ? "+idx" : ""));
	}

	if(opcode == 0)
		append(r, "nop");

	stream << r;

	return 1 | SUPPORTED;
}

void swp30_device::meg_state::call_rand(void *ms)
{
	auto *ms1 = static_cast<meg_state *>(ms);
	ms1->m_retval = ms1->m_swp->machine().rand();
}

void swp30_device::meg_state::call_revram_encode(void *ms)
{
	auto *ms1 = static_cast<meg_state *>(ms);
	ms1->m_retval = revram_encode(ms1->m_retval);
}

void swp30_device::meg_state::call_revram_decode(void *ms)
{
	auto *ms1 = static_cast<meg_state *>(ms);
	ms1->m_retval = revram_decode(ms1->m_retval);
}

void swp30_device::meg_state::drc(drcuml_block &block, u16 pc)
{
	enum {
		L_ABS,     // abs value on p
		L_ABS2,    // abs value on the mac add branch
		L_M1_0,    // m1 expansion, value is < 0
		L_M1_DONE, // m1 expansion, end
		L_M1_M5,   // m1 expansion, exp < 5
		L_LFO1,    // lfo, first label
		L_LFO2,    // lfo, second label
	};

	UML_DEBUG(block, pc);

	u64 opcodep3 = m_program[(pc + 384 - 3) % 384];
	u64 opcodep2 = m_program[(pc + 384 - 2) % 384];
	u64 opcode   = m_program[pc];
	u64 opcode2  = m_program[(pc +       2) % 384];
	u32 index3 = pc % 3;
	u32 index2 = pc % 2;

	// Store the m register at the third instruction
	int delayed_md = BIT(opcodep3, 0x27, 6);
	if(delayed_md)
		UML_MOV(block, mem(&m_m[delayed_md]), mem(&m_mw_value[index3]));

	// Store the r register at the third instruction
	int delayed_rd = BIT(opcodep3, 0x30, 7);
	if(delayed_rd)
		UML_MOV(block, mem(&m_r[delayed_rd]), mem(&m_rw_value[index3]));

	// Store the index register at the third instruction
	if(BIT(opcodep3, 0x3e))
		UML_MOV(block, mem(&m_ram_index), mem(&m_index_value[index3]));

	// Store the memw register at the second instruction
	if(BIT(opcodep2, 0x3d))
		UML_MOV(block, mem(&m_ram_write), mem(&m_memw_value[index2]));

	// Store the memr register at the second instruction
	if(BIT(opcodep2, 0x25))
		UML_MOV(block, mem(&m_ram_read), mem(&m_memr_value[index2]));

	int sm = BIT(opcode, 0x04, 6);
	int sr = BIT(opcode, 0x0b, 7);
	int dm = BIT(opcode, 0x27, 6);
	int dr = BIT(opcode, 0x30, 7);
	int t  = BIT(opcode, 0x38, 3);

	u32 mmode = BIT(opcode, 0x16, 2);
	if(mmode != 0 && !BIT(opcode, 0x3f)) {
		u32 m1t = BIT(opcode, 0x14, 2);
		if(mmode != 3) {
			// Needs m1
			if(m1t == 1 || m1t == 2)
				UML_DLOADS(block, I1, m_t.data(), t, SIZE_WORD, SCALE_x2);
			else
				UML_DLOADS(block, I1, m_const.data(), pc, SIZE_WORD, SCALE_x2);
			if(BIT(opcode, 0x13)) {
				// m1_expand inline
				UML_DMOV(block, I0,  0x0000000000);
				UML_DCMP(block, I1, I0);
				UML_JMPc(block, COND_L, (pc << 4) | L_M1_0);     // If negative, clear
				UML_DSAR(block, I0, I1, 12);                     // exponent in I2
				UML_DAND(block, I1, I1, 0xfff);
				UML_DOR(block, I1, I1, 0x1000);                  // mantissa in I1
				UML_DMOV(block, I2, 5);                          // compare exponent with 5
				UML_DCMP(block, I0, I2);
				UML_JMPc(block, COND_E, (pc << 4) | L_M1_DONE);  // no shift if 5
				UML_JMPc(block, COND_L, (pc << 4) | L_M1_M5);

				UML_DSUB(block, I0, I0, I2);                     // shift left by exp-5 if >5
				UML_DSHL(block, I1, I1, I0);
				UML_JMP(block, (pc << 4) | L_M1_DONE);

				UML_LABEL(block, (pc << 4) | L_M1_M5);
				UML_DSUB(block, I0, I2, I0);                     // shift right by 5-exp if <5
				UML_DSAR(block, I1, I1, I0);
				UML_JMP(block, (pc << 4) | L_M1_DONE);

				UML_LABEL(block, (pc << 4) | L_M1_0);            // Clear (negative case), entered with I0=0
				UML_DMOV(block, I1, I0);
				UML_LABEL(block, (pc << 4) | L_M1_DONE);         // Exit
			}
		}

		if(mmode != 1) {
			// Needs m2
			if(BIT(opcode, 0x12)) {
				if(sm)
					UML_DLOADS(block, I2, m_m.data(), sm, SIZE_DWORD, SCALE_x4);
				else
					UML_DMOV(block, I2, 0);
			} else {
				if(sr)
					UML_DLOADS(block, I2, m_r.data(), sr, SIZE_DWORD, SCALE_x4);
				else
					UML_DMOV(block, I2, 0);
			}
		}

		switch(mmode) {
		case 1:
			UML_DSHL(block, I0, I1, 8+15);
			break;
		case 2:
			UML_DMULSLW(block, I0, I1, I2);
			break;
		case 3:
			UML_DSHL(block, I0, I2, 15);
			break;
		}

		bool a_is_zero = false;
		switch(BIT(opcode, 0x18, 2)) {
		case 0:
			UML_DMOV(block, I1, mem(&m_p));
			break;
		case 1:
			if(sr) {
				UML_DLOADS(block, I1, m_r.data(), sr, SIZE_DWORD, SCALE_x4);
				UML_DSHL(block, I1, I1, 15);
			} else
				UML_DSAR(block, I1, mem(&m_p), 15);
			break;
			break;
		case 2:
			if(sm) {
				UML_DLOADS(block, I1, m_m.data(), sm, SIZE_DWORD, SCALE_x4);
				UML_DSHL(block, I1, I1, 15);
			} else
				UML_DSAR(block, I1, mem(&m_p), 15);
			break;
		case 3:
			a_is_zero = true;
			break;
		}

		switch(BIT(opcode, 0x1a, 2)) {
		case 0:
			if(!a_is_zero)
				UML_DADD(block, I0, I0, I1);
			break;
		case 1:
			if(!a_is_zero)
				UML_DSUB(block, I0, I0, I1);
			break;
		case 2:
			if(!a_is_zero) {
				UML_DMOV(block, I2,  0x0000000000);
				UML_DCMP(block, I1, I2);
				UML_JMPc(block, COND_GE, (pc << 4) | L_ABS2);
				UML_DSUB(block, I1, I2, I1);
				UML_LABEL(block, (pc << 4) | L_ABS2);
				UML_DADD(block, I0, I0, I1);
			}
			break;
		case 3:
			if(!a_is_zero)
				UML_DAND(block, I0, I0, I1);
			else
				UML_DMOV(block, I0, 0);
			break;
		}

		// Shift and wrap to 42 bits
		switch(BIT(opcode, 0x1c, 2)) {
		case 0:
			UML_DSHL(block, I0, I0, 0 + (64-42));
			break;
		case 1:
			UML_DSHL(block, I0, I0, 1 + (64-42));
			break;
		case 2:
			UML_DSHL(block, I0, I0, 2 + (64-42));
			break;
		case 3:
			UML_DSHL(block, I0, I0, 4 + (64-42));
			break;
		}
		UML_DSAR(block, I0, I0, (64-42));

		// Clamp/saturate as requested
		switch(BIT(opcode, 0x1e, 2)) {
		case 0:
			break;
		case 1:
			UML_DMOV(block, I1, -0x4000000000);
			UML_DCMP(block, I0, I1);
			UML_DMOVc(block, COND_L, I0, I1);
			UML_DMOV(block, I1,  0x3fffffffff);
			UML_DCMP(block, I0, I1);
			UML_DMOVc(block, COND_G, I0, I1);
			break;
		case 2:
			UML_DMOV(block, I1,  0x0000000000);
			UML_DCMP(block, I0, I1);
			UML_DMOVc(block, COND_L, I0, I1);
			UML_DMOV(block, I1,  0x3fffffffff);
			UML_DCMP(block, I0, I1);
			UML_DMOVc(block, COND_G, I0, I1);
			break;
		case 3:
			UML_DMOV(block, I1,  0x0000000000);
			UML_DCMP(block, I0, I1);
			UML_JMPc(block, COND_GE, (pc << 4) | L_ABS);
			UML_DSUB(block, I0, I1, I0);
			UML_LABEL(block, (pc << 4) | L_ABS);
			UML_DMOV(block, I1,  0x3fffffffff);
			UML_DCMP(block, I0, I1);
			UML_DMOVc(block, COND_G, I0, I1);
			break;
		}

		UML_DMOV(block, mem(&m_p), I0);
	}

	if(dm) {
		switch(BIT(opcode, 0x2d, 3)) {
		case 0: case 1: case 2: case 3: {
			int lfo = pc >> 4;
			u16 info = m_lfo[lfo];
			UML_MOV(block, I0, mem(&m_lfo_counter[lfo]));
			UML_SAR(block, I0, I0, 5);
			if(info & 0xf300) {
				static const u32 offsets[16] = {
					0x00000, 0x02aaa, 0x04000, 0x05555,
					0x08000, 0x0aaaa, 0x0c000, 0x0d555,
					0x10000, 0x12aaa, 0x14000, 0x15555,
					0x18000, 0x1aaaa, 0x1c000, 0x1d555,
				};

				if(info & 0x0300)
					UML_SHL(block, I0, I0, BIT(info, 8, 2));
				if(info & 0xf000)
					UML_ADD(block, I0, I0, offsets[BIT(info, 12, 4)]);
				UML_AND(block, I0, I0, 0x1ffff);
			}

			switch((info >> 10) & 3) {
			case 0:
				UML_MOV(block, I1, I0);
				UML_AND(block, I0, I0, 0x7fff);
				UML_TEST(block, I1, 0x8000);
				UML_JMPc(block, COND_Z, (pc << 4) | L_LFO1);
				UML_XOR(block, I0, I0, 0x7fff);
				UML_LABEL(block, (pc << 4) | L_LFO1);
				UML_LOAD(block, I0, m_swp->m_sintab, I0, SIZE_WORD, SCALE_x2);
				UML_TEST(block, I1, 0x10000);
				UML_JMPc(block, COND_Z, (pc << 4) | L_LFO2);
				UML_XOR(block, I0, I0, 0xffff);
				UML_LABEL(block, (pc << 4) | L_LFO2);
				break;
			case 1:
				UML_ADD(block, I0, I0, 0x8000);
				UML_TEST(block, I0, 0x10000);
				UML_JMPc(block, COND_Z, (pc << 4) | L_LFO1);
				UML_XOR(block, I0, I0, 0xffff);
				UML_LABEL(block, (pc << 4) | L_LFO1);
				UML_AND(block, I0, I0, 0xffff);
				break;
			case 2:
				UML_SAR(block, I0, I0, 1);
				break;
			case 3:
				UML_XOR(block, I0, I0, 0x1ffff);
				UML_SAR(block, I0, I0, 1);
				break;
			}
			UML_SHL(block, mem(&m_mw_value[index3]), I0, 7);
			break;
		}
		case 4:
			UML_MOV(block, mem(&m_mw_value[index3]), mem(&m_ram_read));
			break;
		case 5:
			UML_CALLC(block, call_rand, this);
			UML_SHL(block, I0, mem(&m_retval), 8);
			UML_SAR(block, mem(&m_mw_value[index3]), I0, 8);
			break;
		case 6:
			UML_DMOV(block, I0, mem(&m_p));
			if(!BIT(opcode, 0x0a)) {
				UML_CALLC(block, call_rand, this);
				UML_AND(block, I1, mem(&m_retval), 0x07e0);
				UML_DADD(block, I0, I0, I1);
			}
			UML_DSAR(block, I0, I0, (15-8));
			UML_SAR(block, mem(&m_mw_value[index3]), I0, 8);
			break;
		case 7:
			UML_MOV(block, mem(&m_mw_value[index3]), mem(&m_m[sm]));
			break;
		}
	}

	if(dr) {
		if(BIT(opcode, 0x37)) {
			if(sr)
				UML_DMOV(block, mem(&m_rw_value[index3]), mem(&m_r[sr]));
			else
				UML_DMOV(block, mem(&m_rw_value[index3]), 0);
		} else {
			UML_DMOV(block, I0, mem(&m_p));
			if(!BIT(opcode, 0x0a)) {
				UML_CALLC(block, call_rand, this);
				UML_AND(block, I1, mem(&m_retval), 0x07e0);
				UML_DADD(block, I0, I0, I1);
			}
			UML_DSAR(block, I0, I0, (15-8));
			UML_SAR(block, mem(&m_rw_value[index3]), I0, 8);
		}
	}

	// T write lookups the p value from two cycles before
	if(BIT(opcode, 0x3b, 1)) {
		if(BIT(opcode, 0x3c))
			UML_LOADS(block, I0, m_t_value.data(), index2, SIZE_WORD, SCALE_x2);
		else
			UML_LOADS(block, I0, m_const.data(), pc, SIZE_WORD, SCALE_x2);
		UML_STORE(block, m_t.data(), t, I0, SIZE_WORD, SCALE_x2);
	}
	if(BIT(opcode2, 0x3b, 2) == 3) {
		if(BIT(opcode, 0x3e)) {
			UML_DSAR(block, I0, mem(&m_p), 8);
			UML_AND(block, I0, I0, 0x7fff);
			UML_STORE(block, m_t_value.data(), index2, I0, SIZE_WORD, SCALE_x2);
		} else {
			UML_DSAR(block, I0, mem(&m_p), 15+8);
			UML_STORE(block, m_t_value.data(), index2, I0, SIZE_WORD, SCALE_x2);
		}
	}

	if(BIT(opcode, 0x3d)) {
		UML_DSAR(block, I0, mem(&m_p), 15);
		UML_MOV(block, mem(&m_memw_value[index2]), I0);
	}

	if(BIT(opcode, 0x3e)) {
		UML_DSAR(block, I0, mem(&m_p), 15+8);
		UML_STORE(block, m_index_value.data(), index3, I0, SIZE_WORD, SCALE_x2);
	}

	// Memory access
	int amem = BIT(opcode, 0x24, 2);
	if(amem) {
		u16 key = (pc / 12) << 11;
		int bank;
		for(bank=0; bank != 7; bank++)
			if(m_map[bank+1] <= m_map[bank] || ((m_map[bank+1] & 0xf800) > key))
				break;
		u16 mapr = m_map[bank];
		u32 mask = (1 << (10+BIT(mapr, 8, 3))) - 1;
		u32 offset = BIT(mapr, 0, 8) << 10;
		if(amem == 3)
			offset ++;
		UML_LOAD(block, I0, m_offset.data(), pc/3, SIZE_WORD, SCALE_x2);
		UML_SUB(block, I0, I0, mem(&m_sample_counter));
		UML_ADD(block, I0, I0, offset);
		if(BIT(opcode, 0x21))
			UML_ADD(block, I0, I0, mem(&m_ram_index));
		UML_AND(block, I0, I0, mask);
		if(amem == 1) {
			UML_MOV(block, mem(&m_retval), mem(&m_ram_write));
			UML_CALLC(block, call_revram_encode, this);
			UML_MOV(block, I1, mem(&m_retval));
			UML_WRITE(block, I0, I1, SIZE_WORD, memory_space(swp30_device::AS_REVERB));
		} else {
			UML_READ(block, I1, I0, SIZE_WORD, memory_space(swp30_device::AS_REVERB));
			UML_MOV(block, mem(&m_retval), I1);
			UML_CALLC(block, call_revram_decode, this);
			UML_MOV(block, mem(&m_memr_value[index2]), mem(&m_retval));
		}
	}
}

void swp30_device::meg_state::step()
{
	m_swp->debugger_instruction_hook(m_pc);

	// All register writes are delayed by 3 cycles, probably a pipeline
	// Register 0 in both banks are wired to value 0
	if(m_mw_reg[m_delay_3])
		m_m[m_mw_reg[m_delay_3]] = m_mw_value[m_delay_3];

	if(m_rw_reg[m_delay_3])
		m_r[m_rw_reg[m_delay_3]] = m_rw_value[m_delay_3];

	// Index is similarly delayed
	if(m_index_active[m_delay_3])
		m_ram_index = m_index_value[m_delay_3];

	// Memory read and write ports are delayed by 2 cycles
	if(m_memw_active[m_delay_2]) {
		m_ram_write = m_memw_value[m_delay_2];
		m_memw_active[m_delay_2] = false;
	}
	if(m_memr_active[m_delay_2]) {
		m_ram_read = m_memr_value[m_delay_2];
		m_memr_active[m_delay_2] = false;
	}

	u64 opcode = m_swp->m_program_cache.read_qword(m_pc);

	int sm = BIT(opcode, 0x04, 6);
	int sr = BIT(opcode, 0x0b, 7);
	int dm = BIT(opcode, 0x27, 6);
	int dr = BIT(opcode, 0x30, 7);
	int t  = BIT(opcode, 0x38, 3);

	u32 mmode = BIT(opcode, 0x16, 2);
	if(mmode != 0) {
		u32 m1t = BIT(opcode, 0x14, 2);
		s64 m1 = m1t == 1 || m1t == 2 ? m_t[t] : m_const[m_pc];
		if(BIT(opcode, 0x13))
			m1 = m1_expand(m1);

		s64 m2 = BIT(opcode, 0x12) ? m_m[sm] : m_r[sr];

		s64 m;
		switch(mmode) {
		case 1:
			m = m1 << (8+15);
			break;
		case 2:
			m = m1 * m2;
			break;
		case 3:
			m = m2 << 15;
			break;
		}

		s64 a;
		switch(BIT(opcode, 0x18, 2)) {
		case 0: a = m_p; break;
		case 1: a = sr ? s64(m_r[sr]) << 15 : m_p >> 15; break;
		case 2: a = sm ? s64(m_m[sm]) << 15 : m_p >> 15; break;
		case 3: a = 0; break;
		}

		s64 r;
		switch(BIT(opcode, 0x1a, 2)) {
		case 0:
			r = m + a;
			break;
		case 1:
			r = m - a;
			break;
		case 2:
			r = m + (a < 0 ? -a : a);
			break;
		case 3:
			r = m & a;
			break;
		}

		int shift = BIT(opcode, 0x1c, 2);
		if(shift)
			r <<= shift == 3 ? 4 : shift;

		// wrap at 42 bits (27.15)
		r = util::sext(r, 42);

		switch(BIT(opcode, 0x1e, 2)) {
		case 0:
			break;
		case 1:
			r = std::clamp<s64>(r, -0x4000000000, 0x3fffffffff);
			break;
		case 2:
			r = std::clamp<s64>(r, 0, 0x3fffffffff);
			break;
		case 3:
			r = std::min<s64>(r < 0 ? -r : r, 0x3fffffffff);
			break;
		}

		m_p = r;
	}

	m_mw_reg[m_delay_3] = dm;
	if(dm) {
		u32 v;
		switch(BIT(opcode, 0x2d, 3)) {
		case 0: case 1: case 2: case 3:
			v = get_lfo(m_pc >> 4);
			break;
		case 4: v = m_ram_read; break;
		case 5: v = m_swp->machine().rand() & 0xffffff; if(v & 0x00800000) v |= 0xff000000; break;
		case 6: {
			s64 p = m_p;
			if(!BIT(opcode, 0x0a))
				p += m_swp->machine().rand() & 0x07e0;
			v = (p >> 15) & 0xffffff;
			if(v & 0x00800000)
				v |= 0xff000000;
			break;
		}
		case 7: v = m_m[sm]; break;
		}
		m_mw_value[m_delay_3] = v;
	}

	m_rw_reg[m_delay_3] = dr;
	if(dr) {
		u32 v;
		if(BIT(opcode, 0x37))
			v = m_r[sr];
		else {
			s64 p = m_p;
			if(!BIT(opcode, 0x0a))
				p += m_swp->machine().rand() & 0x07e0;
			v = (p >> 15) & 0xffffff;
			if(v & 0x00800000)
				v |= 0xff000000;
		}
		m_rw_value[m_delay_3] = v;
	}

	if(BIT(opcode, 0x3d)) {
		m_memw_active[m_delay_2] = true;
		m_memw_value[m_delay_2] = m_p >> 15;
	} else
		m_memw_active[m_delay_2] = false;

	if(BIT(opcode, 0x3e)) {
		m_index_active[m_delay_3] = true;
		m_index_value[m_delay_3] = m_p >> (15+8);
	} else
		m_index_active[m_delay_3] = false;

	// T write lookups the p value from two cycles before, but which
	// bits depends on the presence of index setting
	if(BIT(opcode, 0x3b, 1)) {
		if(BIT(opcode, 0x3c))
			m_t[t] = m_t_value[m_delay_2];
		else
			m_t[t] = m_const[m_pc];
	}
	m_t_value[m_delay_2] = BIT(opcode, 0x3e) ? (m_p >> 8) & 0x7fff : m_p >> (15+8);

	// Memory access
	switch(BIT(opcode, 0x24, 2)) {
	case 1: {
		u32 address = resolve_address(m_pc, m_offset[m_pc/3] + (BIT(opcode, 0x21) ? m_ram_index : 0) - m_sample_counter);
		if(address != 0xffffffff)
			m_swp->m_reverb_cache.write_word(address, revram_encode(m_ram_write));
		break;
	}
	case 2: {
		u32 address = resolve_address(m_pc, m_offset[m_pc/3] + (BIT(opcode, 0x21) ? m_ram_index : 0) - m_sample_counter);
		if(address != 0xffffffff) {
			u16 val = m_swp->m_reverb_cache.read_word(address);
			m_memr_value[m_delay_2] = revram_decode(val);
			m_memr_active[m_delay_2] = true;
		}
		break;
	}
	case 3: {
		u32 address = resolve_address(m_pc, m_offset[m_pc/3] + (BIT(opcode, 0x21) ? m_ram_index : 0) - m_sample_counter + 1);
		if(address != 0xffffffff) {
			u16 val = m_swp->m_reverb_cache.read_word(address);
			m_memr_value[m_delay_2] = revram_decode(val);
			m_memr_active[m_delay_2] = true;
		}
		break;
	}
	}

	m_delay_3 ++;
	if(m_delay_3 == 3)
		m_delay_3 = 0;

	m_delay_2 ++;
	if(m_delay_2 == 2)
		m_delay_2 = 0;

	m_pc ++;
	m_icount --;

	if(m_pc == 0x180)
		m_pc = 0;
}

void swp30_device::execute_run()
{
	if(m_meg_drc_active) {
		if(m_meg_program_changed) {
			m_drcuml->reset();
			m_meg_program_changed = false;
			drcuml_block &block(m_drcuml->begin_block(16384));
			UML_HANDLE(block, *m_meg_drc_entry);
			for(u16 pc = 0; pc != 384; pc++)
				m_meg->drc(block, pc);
			UML_EXIT(block, 0);
			block.end();
		}

		while(m_meg->m_icount > 0) {
			sample_step();
			m_drcuml->execute(*m_meg_drc_entry);
			m_meg->m_icount -= 384;
		}

	} else {
		m_meg_program_changed = false;

		while(m_meg->m_icount > 0) {
			if(m_meg->m_pc == 0)
				sample_step();
			m_meg->step();
		}
	}
}

void swp30_device::adc_step()
{
	for(int i=0; i != 4; i++)
		m_adc[i] = std::clamp<s32>(m_meg->m_m[0x30 + i] >> 4, -0x20000, +0x1ffff);
}

void swp30_device::sample_step()
{
	std::array<s32, 0x40> samples_per_chan;
	awm2_step(samples_per_chan);
	adc_step();
	mixer_step(samples_per_chan);
	m_meg->lfo_step();
	m_meg->m_sample_counter ++;
}

void swp30_device::sound_stream_update(sound_stream &stream)
{
	if(&stream == m_output_stream) {
		for(int i=0; i != 4; i++)
			stream.put_int_clamp(i, 0, m_adc[i], 1<<17);
		for(int i=0; i != 16; i++)
			stream.put_int_clamp(i+4, 0, m_melo[i], 1<<26);
	} else
		for(int i=0; i != 16; i++)
			m_meli[i] = stream.get(i, 0) * (1<<26);
}

DEFINE_DEVICE_TYPE(SWP30, swp30_device, "swp30", "Yamaha SWP30 sound chip")

