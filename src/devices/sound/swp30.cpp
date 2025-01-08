// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#include "emu.h"
#include "debugger.h"
#include "swp30.h"

static int scount = 0;

/*
  The SWP30 is the combination of a rompler called AWM2 (Advanced Wave
  Memory 2) and an effects DSP called MEG (Multiple Effects
  Generator).  It also includes some routing/mixing capabilities,
  moving data between AWM2, MEG and serial inputs and outputs with
  volume management capabilities everywhere.  Its clock is 33.9MHz and
  the output is at 44100Hz stereo (768 cycles per sample pair) per dac
  output.

  I/O wise, the chip has 8 generic audio serial inputs and 8 outputs
  for external plugins, and two dac outputs.  The DAC outputs are
  stereo, and so is the first generic input.  It's unclear whether the
  outputs and the other inputs are stereo.  The MU100 connects a
  stereo ADC to the first input, and routes the third input and output
  to the plugin boards, but not the left/right input clock, arguing
  for mono.


    Registers:

  The chip interface presents 4096 16-bits registers in a 64x64 grid.
  They all seem to be read/write.  Some of this grid is for
  per-channel values for AWM2, but parts are isolated and renumbered
  for MEG regisrers or for general control functions.

  Names we'll use in the rest of the text:
  - reg(y, x) is the register at address 2*(y*0x40 + x)
  - ch<nn>  is reg(channel, nn) for a given AWG2 channel
  - sy<nn>  is reg(nn/2, 0xe + (nn % 2))
  - fp<nnn> is reg(nn/6, 0x21 + 2*(nn % 6))
  - of<nn>  is reg(nn/2, 0x30 + (nn % 2))
  - lfo<nn> is reg(nn/2, 0x3e + (nn % 2)) for nn = 0..17


    AWM2:

  The AWM2 is in charge of handling the individual channels.  It
  manages reading the rom, decoding the samples, applying volume and
  pitch envelopes and lfos and filtering the result.  Each channel is
  then sent to the mixer for further processing.

  The sound data can be four formats (8 bits, 12 bits, 16 bits, and a
  8-bits log format with roughly 10 bits of dynamic).  The rom bus is
  25 bits address and 32 bits data wide.  It applies four filters to
  the sample data, two of fixed type (low pass then highpass) and two
  free 3-point FIR filters (used for yet another lowpass and
  highpass).  Envelopes are handled semi-automatically, and the final
  panned result is sent to the mixer.


  ch00       fixed LPF frequency cutoff index
  ch01       fixed LPF frequency cutoff index increment?
  ch02       fixed HPF frequency cutoff
  ch03       40ff at startup, 5010 always afterwards?
  ch04       fixed LPF resonance level
  ch05       unknown
  ch06       attack, bit 14-8 = step, bit 7 = mode
  ch07       decay1, bit 14-8 = step, bit 7-0 = target attenuation (top 8 bits)
  ch08       decay2, bit 14-8 = step, bit 7-0 = target attenuation (top 8 bits)
  ch09       base volume  bit 15 = activate decay2, bit 14-8 unknown, bit 7-0 = initial attenuation

  ch0a-0d    unknown, probably something to do with pitch eg
  ch10       unknown
  ch11       bit 15 = compressed 8-bits mode, 13-0 channel replay frequency, signed 3.10 fixed point,
             log2 scale, positive is higher resulting frequency.

  ch12-13    bit 31 unknown, 30 unknown, 29-0 = number of samples before the loop point
  ch14-15    bit 31 = play sample backwards, 30-0 = number of samples in the loop
  ch16-17    bit 31-30 = sample format, 29-25 = loop samples decimal part, 24-0 = loop start address in rom
  ch20,22,24 first FIR coefficients
  ch26,28,2a second FIR coefficients
  ch2c-2f    unknown
  ch32       pan left/right, 2x8 bits of attenuation

  sy02       internal register selector, msb = 0 or 6, lsb = channel
  sy03       internal register read port, used for envelope/keyoff management, 6 seems to be current volume
  sy0c-0f    keyon mask
  sy10       write something to trigger a keyon according to the mask


  The current attenuation (before panning) is on 26 bits, in 4.22
  floating point format, of which only probably the top 8 are used for
  actual volume computations (see the Mixer part).  The steps are in
  4.3 floating-point format, e.g. the value converts to linear as:

     step = (8 + bit 2..0) << (bit 7..4)

  giving a value between 8 and 0x78000.  This value is added or
  substracted after each sample.


    MEG:

  The MEG is a DSP with 384 program steps connected to a 0x40000
  samples ram.  Instructions are 64 bits wide, and to each instruction
  is associated a 2.14 fixed point value, Every third instruction (pc
  multiple of 3) can initiate a memory access to the reverb buffer
  which will be completed two instructions later.  Each of those
  instructions is associated to a 16-bits address offset value.

  The DSP also sports 256 rotating registers (e.g. register 1 at run
  <n> becomes register 0 at run <n+1>) and 64 fixed registers.  The
  fixed registers are used to store the results of reading the samples
  ram and also communicate with the mixer.

  Every 44100th of a second the 384 program steps are run once in
  order (no branches) to compute everything.

  24 LFO registers are available.  The LFO registers
  internal counters are 22 bits wide.  The LSB of the register gives
  the increment per sample, encoded in a special 3.5 format.
  With scale = 3bits and v = 5bits,
    step  = base[scale] + (v << shift[scale])
    base  = { 0, 32, 64, 128, 256, 512,  1024, 2048 }
    shift = { 0,  0,  1,   2,   3,   4,     5,    6 }

  The 21th bit of the counter inverts bits 20-0 on read, those are
  interpreted as a 0-1 value, giving a sawtooth wave.  When an
  instruction uses the lfo, which one is selected by using pc/16.

  8 mappings can be setup, which allow to manage rotating buffers in
  the samples ram easily by automating masking and offset adding.  The
  register format is: pppppsss oooooooo.  'p' is the base pc/12 at
  which the map starts to be used.  's' is the sub-buffer size,
  defined as 1 << (10+s).  The base offset is o << 10.  There are no
  alignment issues, e.g. you can have a buffer at 0x28000 which is
  0x10000 samples long.


  fp<nnn>    fixed point 2.14 value associated with instruction nnn
  of<nn>     16-bits offset associated with instruction 3*nn
  lfo<nn>    LFO registers

  sy21       MEG program write address
  sy22-25    MEG program opcode, msb-first, writing to 25 triggers an auto-increment
  sy30-3e    even slots only, MEG buffer mappings


    Mixer:

  The mixer gets the outputs of the AWM2, the MEG (for the previous
  sample) and the external inputs, attenuates and sums them according
  to its mapping instructions, and pushes the results to the MEG, the
  DACs and the external outputs.  The attenuations are 8-bits values
  in 4.4 floating point format (multiplies by (1-mant/2)*2**(-exp)).
  The routing is indicated through triplets of 16-bits values.

  ch33       dry (msb) and reverb (lsb) attenuation for an AWM2 channel
  ch34       chorus (msb) and variation (lsb) atternuation
  ch35-37    routing for an AWM2 channel


*/


DEFINE_DEVICE_TYPE(SWP30, swp30_device, "swp30", "Yamaha SWP30 sound chip")

bool swp30_device::istep(s32 &value, s32 limit, s32 step)
{
	//  fprintf(stderr, "istep(%x, %x, %x)\n", value, limit, step);
	if(value < limit) {
		value += step;
		if(value >= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	if(value > limit) {
		value -= step;
		if(value <= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	return true;
}

s32 swp30_device::fpadd(s32 value, s32 step)
{
	s32 e = value >> 24;
	s32 m = value & 0xffffff;

	m += step << e;
	if(m & 0xfe000000)
		return 0xfffffff;

	while(m & 0x01000000) {
		m <<= 1;
		e ++;
	}
	if(e >= 16)
		return 0xfffffff;
	return (e << 24) | (m & 0xffffff);
}

s32 swp30_device::fpsub(s32 value, s32 step)
{
	s32 e = value >> 24;
	s32 m = (value & 0xffffff) | 0xfe000000;
	m = e < 0xc ? m - (step << e) : (m >> (e - 0xb)) - (step << 0xb);
	if(m >= 0)
		return 0;
	if(e >= 0xc)
		e = 0xb;
	while(m < 0xfe000000) {
		if(!e)
			return 0;
		e --;
		m >>= 1;
	}
	while(e != 0xf && (m >= 0xff000000)) {
		e ++;
		m <<= 1;
	}

	return (e << 24) | (m & 0xffffff);
}

bool swp30_device::fpstep(s32 &value, s32 limit, s32 step)
{
	// value, limit and step are 4.24 but step has its exponent and
	// top four bits zero

	if(value == limit)
		return true;
	if(value < limit) {
		value = fpadd(value, step);
		if(value >= limit) {
			value = limit;
			return true;
		}
		return false;
	}

	value = fpsub(value, step);
	if(value <= limit) {
		value = limit;
		return true;
	}
	return false;
}

// sample is signed 24.8
s32 swp30_device::fpapply(s32 value, s32 sample)
{
	if(value >= 0x10000000)
		return 0;
	return (s64(sample) - ((s64(sample) * ((value >> 9) & 0x7fff)) >> 16)) >> (value >> 24);
}

// sample is signed 24.8
s32 swp30_device::lpffpapply(s32 value, s32 sample)
{
	return ((((value >> 7) & 0x7fff) | 0x8000) * s64(sample)) >> (31 - (value >> 22));
}

// Some tables we picked up from the swp00.  May be different, may not be.

const std::array<s32, 0x80> swp30_device::attack_linear_step = {
	0x00027, 0x0002b, 0x0002f, 0x00033, 0x00037, 0x0003d, 0x00042, 0x00048,
	0x0004d, 0x00056, 0x0005e, 0x00066, 0x0006f, 0x0007a, 0x00085, 0x00090,
	0x0009b, 0x000ac, 0x000bd, 0x000cc, 0x000de, 0x000f4, 0x00109, 0x00120,
	0x00135, 0x00158, 0x00179, 0x00199, 0x001bc, 0x001e7, 0x00214, 0x00240,
	0x0026b, 0x002af, 0x002f2, 0x00332, 0x00377, 0x003d0, 0x0042c, 0x00480,
	0x004dc, 0x0055e, 0x005e9, 0x0066e, 0x006f4, 0x007a4, 0x00857, 0x0090b,
	0x009c3, 0x00acb, 0x00bd6, 0x00ce6, 0x00e00, 0x00f5e, 0x010d2, 0x01234,
	0x0139e, 0x015d0, 0x017f3, 0x01a20, 0x01c4a, 0x01f52, 0x02232, 0x0250f,
	0x027ff, 0x02c72, 0x03109, 0x0338b, 0x039c4, 0x04038, 0x04648, 0x04c84,
	0x05262, 0x05c1c, 0x065af, 0x06f5c, 0x07895, 0x0866f, 0x09470, 0x0a19e,
	0x0ae4c, 0x0c566, 0x0db8d, 0x0f00f, 0x10625, 0x12937, 0x14954, 0x16c17,
	0x1886e, 0x1c71c, 0x20000, 0x239e1, 0x2647c, 0x2aaab, 0x2ecfc, 0x3241f,
	0x35e51, 0x3a83b, 0x40000, 0x4325c, 0x47dc1, 0x4c8f9, 0x50505, 0x55555,
	0x58160, 0x5d174, 0x60606, 0x62b2e, 0x67b24, 0x6a63c, 0x6d3a0, 0x6eb3e,
	0x71c72, 0x73616, 0x75075, 0x76b98, 0x78788, 0x78788, 0x7a44c, 0x7a44c,
	0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c, 0x7a44c,
};

const std::array<s32, 0x20> swp30_device::decay_linear_step = {
	0x15083, 0x17ad2, 0x1a41a, 0x1cbe7, 0x1f16d, 0x22ef1, 0x26a44, 0x2a1e4,
	0x2da35, 0x34034, 0x3a197, 0x40000, 0x45b82, 0x4b809, 0x51833, 0x57262,
	0x5d9f7, 0x6483f, 0x6b15c, 0x71c72, 0x77976, 0x7d119, 0x83127, 0x88889,
	0x8d3dd, 0x939a8, 0x991f2, 0x9d89e, 0xa0a0a, 0xa57eb, 0xa72f0, 0xac769,
};

// Pitch conversion table, 2**(31 + i/0x400)
const std::array<u32, 0x400> swp30_device::pitch_base = {
	0x80000000, 0x8016302f, 0x802c6436, 0x80429c17, 0x8058d7d2, 0x806f1768, 0x80855ad9, 0x809ba226,
	0x80b1ed4f, 0x80c83c56, 0x80de8f3b, 0x80f4e5ff, 0x810b40a1, 0x81219f24, 0x81380188, 0x814e67cc,
	0x8164d1f3, 0x817b3ffd, 0x8191b1ea, 0x81a827ba, 0x81bea170, 0x81d51f0b, 0x81eba08c, 0x820225f4,
	0x8218af43, 0x822f3c7a, 0x8245cd9a, 0x825c62a4, 0x8272fb97, 0x82899876, 0x82a0393f, 0x82b6ddf5,
	0x82cd8698, 0x82e43329, 0x82fae3a7, 0x83119814, 0x83285071, 0x833f0cbf, 0x8355ccfd, 0x836c912c,
	0x8383594e, 0x839a2563, 0x83b0f56c, 0x83c7c969, 0x83dea15b, 0x83f57d43, 0x840c5d21, 0x842340f6,
	0x843a28c3, 0x84511489, 0x84680447, 0x847ef800, 0x8495efb3, 0x84aceb61, 0x84c3eb0b, 0x84daeeb2,
	0x84f1f656, 0x850901f8, 0x85201198, 0x85372538, 0x854e3cd8, 0x85655879, 0x857c781c, 0x85939bc0,
	0x85aac367, 0x85c1ef12, 0x85d91ec1, 0x85f05275, 0x86078a2f, 0x861ec5ef, 0x863605b5, 0x864d4984,
	0x8664915b, 0x867bdd3b, 0x86932d25, 0x86aa811a, 0x86c1d919, 0x86d93525, 0x86f0953d, 0x8707f963,
	0x871f6196, 0x8736cdd8, 0x874e3e2a, 0x8765b28c, 0x877d2afe, 0x8794a783, 0x87ac2819, 0x87c3acc2,
	0x87db357f, 0x87f2c251, 0x880a5337, 0x8821e834, 0x88398146, 0x88511e70, 0x8868bfb2, 0x8880650c,
	0x88980e80, 0x88afbc0e, 0x88c76db6, 0x88df237a, 0x88f6dd5a, 0x890e9b57, 0x89265d72, 0x893e23ab,
	0x8955ee03, 0x896dbc7b, 0x89858f13, 0x899d65cc, 0x89b540a7, 0x89cd1fa5, 0x89e502c6, 0x89fcea0b,
	0x8a14d575, 0x8a2cc504, 0x8a44b8ba, 0x8a5cb096, 0x8a74ac9a, 0x8a8cacc6, 0x8aa4b11c, 0x8abcb99b,
	0x8ad4c645, 0x8aecd71a, 0x8b04ec1b, 0x8b1d0548, 0x8b3522a3, 0x8b4d442c, 0x8b6569e4, 0x8b7d93cc,
	0x8b95c1e3, 0x8badf42c, 0x8bc62aa7, 0x8bde6554, 0x8bf6a434, 0x8c0ee748, 0x8c272e91, 0x8c3f7a10,
	0x8c57c9c4, 0x8c701daf, 0x8c8875d2, 0x8ca0d22d, 0x8cb932c1, 0x8cd1978f, 0x8cea0098, 0x8d026ddb,
	0x8d1adf5b, 0x8d335517, 0x8d4bcf11, 0x8d644d49, 0x8d7ccfc0, 0x8d955677, 0x8dade16e, 0x8dc670a6,
	0x8ddf0420, 0x8df79bdc, 0x8e1037dc, 0x8e28d820, 0x8e417ca9, 0x8e5a2577, 0x8e72d28c, 0x8e8b83e7,
	0x8ea4398b, 0x8ebcf377, 0x8ed5b1ac, 0x8eee742b, 0x8f073af5, 0x8f20060b, 0x8f38d56c, 0x8f51a91b,
	0x8f6a8117, 0x8f835d62, 0x8f9c3dfc, 0x8fb522e6, 0x8fce0c21, 0x8fe6f9ae, 0x8fffeb8c, 0x9018e1bd,
	0x9031dc43, 0x904adb1c, 0x9063de4b, 0x907ce5d0, 0x9095f1ab, 0x90af01de, 0x90c81669, 0x90e12f4e,
	0x90fa4c8b, 0x91136e24, 0x912c9417, 0x9145be67, 0x915eed13, 0x9178201d, 0x91915785, 0x91aa934c,
	0x91c3d373, 0x91dd17fb, 0x91f660e3, 0x920fae2e, 0x9228ffdc, 0x924255ed, 0x925bb062, 0x92750f3d,
	0x928e727d, 0x92a7da24, 0x92c14632, 0x92dab6a9, 0x92f42b88, 0x930da4d2, 0x93272285, 0x9340a4a4,
	0x935a2b2f, 0x9373b626, 0x938d458b, 0x93a6d95e, 0x93c071a0, 0x93da0e52, 0x93f3af75, 0x940d5509,
	0x9426ff0f, 0x9440ad88, 0x945a6075, 0x947417d6, 0x948dd3ac, 0x94a793f8, 0x94c158bb, 0x94db21f6,
	0x94f4efa8, 0x950ec1d4, 0x9528987a, 0x9542739a, 0x955c5336, 0x9576374e, 0x95901fe3, 0x95aa0cf5,
	0x95c3fe86, 0x95ddf497, 0x95f7ef27, 0x9611ee38, 0x962bf1cb, 0x9645f9e1, 0x96600679, 0x967a1795,
	0x96942d37, 0x96ae475d, 0x96c8660a, 0x96e2893f, 0x96fcb0fb, 0x9716dd3f, 0x97310e0e, 0x974b4366,
	0x97657d49, 0x977fbbb9, 0x9799feb5, 0x97b4463e, 0x97ce9255, 0x97e8e2fc, 0x98033832, 0x981d91f9,
	0x9837f051, 0x9852533b, 0x986cbab9, 0x988726c9, 0x98a1976f, 0x98bc0caa, 0x98d6867b, 0x98f104e2,
	0x990b87e2, 0x99260f7a, 0x99409bab, 0x995b2c77, 0x9975c1dd, 0x99905bdf, 0x99aafa7d, 0x99c59db9,
	0x99e04593, 0x99faf20b, 0x9a15a324, 0x9a3058dc, 0x9a4b1337, 0x9a65d233, 0x9a8095d2, 0x9a9b5e15,
	0x9ab62afc, 0x9ad0fc89, 0x9aebd2bb, 0x9b06ad95, 0x9b218d16, 0x9b3c7140, 0x9b575a14, 0x9b724791,
	0x9b8d39b9, 0x9ba8308d, 0x9bc32c0e, 0x9bde2c3c, 0x9bf93118, 0x9c143aa4, 0x9c2f48df, 0x9c4a5bcb,
	0x9c657368, 0x9c808fb7, 0x9c9bb0ba, 0x9cb6d670, 0x9cd200db, 0x9ced2ffc, 0x9d0863d3, 0x9d239c61,
	0x9d3ed9a7, 0x9d5a1ba6, 0x9d75625e, 0x9d90add1, 0x9dabfdff, 0x9dc752e9, 0x9de2ac90, 0x9dfe0af5,
	0x9e196e18, 0x9e34d5fb, 0x9e50429e, 0x9e6bb401, 0x9e872a27, 0x9ea2a50f, 0x9ebe24bb, 0x9ed9a92b,
	0x9ef53260, 0x9f10c05b, 0x9f2c531d, 0x9f47eaa6, 0x9f6386f8, 0x9f7f2814, 0x9f9acdf9, 0x9fb678a9,
	0x9fd22825, 0x9feddc6d, 0xa0099583, 0xa0255367, 0xa041161b, 0xa05cdd9e, 0xa078a9f2, 0xa0947b17,
	0xa0b0510f, 0xa0cc2bda, 0xa0e80b7a, 0xa103efee, 0xa11fd938, 0xa13bc758, 0xa157ba50, 0xa173b221,
	0xa18faeca, 0xa1abb04d, 0xa1c7b6ac, 0xa1e3c1e5, 0xa1ffd1fc, 0xa21be6ef, 0xa23800c1, 0xa2541f72,
	0xa2704303, 0xa28c6b74, 0xa2a898c7, 0xa2c4cafc, 0xa2e10215, 0xa2fd3e11, 0xa3197ef3, 0xa335c4ba,
	0xa3520f68, 0xa36e5efe, 0xa38ab37c, 0xa3a70ce3, 0xa3c36b34, 0xa3dfce70, 0xa3fc3698, 0xa418a3ac,
	0xa43515ae, 0xa4518c9e, 0xa46e087d, 0xa48a894c, 0xa4a70f0c, 0xa4c399be, 0xa4e02962, 0xa4fcbdfa,
	0xa5195786, 0xa535f608, 0xa552997f, 0xa56f41ed, 0xa58bef53, 0xa5a8a1b1, 0xa5c55909, 0xa5e2155c,
	0xa5fed6a9, 0xa61b9cf3, 0xa6386839, 0xa655387d, 0xa6720dc0, 0xa68ee803, 0xa6abc745, 0xa6c8ab89,
	0xa6e594cf, 0xa7028319, 0xa71f7665, 0xa73c6eb7, 0xa7596c0e, 0xa7766e6c, 0xa79375d1, 0xa7b0823e,
	0xa7cd93b4, 0xa7eaaa35, 0xa807c5c0, 0xa824e656, 0xa8420bfa, 0xa85f36aa, 0xa87c6669, 0xa8999b38,
	0xa8b6d516, 0xa8d41405, 0xa8f15806, 0xa90ea11a, 0xa92bef41, 0xa949427d, 0xa9669ace, 0xa983f836,
	0xa9a15ab4, 0xa9bec24b, 0xa9dc2efa, 0xa9f9a0c3, 0xaa1717a7, 0xaa3493a7, 0xaa5214c2, 0xaa6f9afb,
	0xaa8d2652, 0xaaaab6c9, 0xaac84c5f, 0xaae5e716, 0xab0386ef, 0xab212bea, 0xab3ed609, 0xab5c854d,
	0xab7a39b5, 0xab97f344, 0xabb5b1fa, 0xabd375d8, 0xabf13edf, 0xac0f0d0f, 0xac2ce06a, 0xac4ab8f1,
	0xac6896a4, 0xac867985, 0xaca46194, 0xacc24ed1, 0xace0413f, 0xacfe38de, 0xad1c35af, 0xad3a37b3,
	0xad583eea, 0xad764b55, 0xad945cf7, 0xadb273ce, 0xadd08fdd, 0xadeeb124, 0xae0cd7a4, 0xae2b035e,
	0xae493452, 0xae676a83, 0xae85a5f0, 0xaea3e69b, 0xaec22c84, 0xaee077ad, 0xaefec816, 0xaf1d1dc0,
	0xaf3b78ad, 0xaf59d8dc, 0xaf783e50, 0xaf96a908, 0xafb51906, 0xafd38e4b, 0xaff208d8, 0xb01088ad,
	0xb02f0dcb, 0xb04d9834, 0xb06c27e8, 0xb08abce8, 0xb0a95736, 0xb0c7f6d1, 0xb0e69bbc, 0xb10545f6,
	0xb123f581, 0xb142aa5e, 0xb161648e, 0xb1802411, 0xb19ee8e8, 0xb1bdb315, 0xb1dc8299, 0xb1fb5773,
	0xb21a31a6, 0xb2391132, 0xb257f618, 0xb276e059, 0xb295cff5, 0xb2b4c4ef, 0xb2d3bf46, 0xb2f2befc,
	0xb311c412, 0xb330ce88, 0xb34fde60, 0xb36ef39a, 0xb38e0e38, 0xb3ad2e3a, 0xb3cc53a1, 0xb3eb7e6e,
	0xb40aaea2, 0xb429e43e, 0xb4491f43, 0xb4685fb2, 0xb487a58c, 0xb4a6f0d2, 0xb4c64185, 0xb4e597a5,
	0xb504f333, 0xb5245432, 0xb543baa0, 0xb5632681, 0xb58297d3, 0xb5a20e99, 0xb5c18ad3, 0xb5e10c82,
	0xb60093a8, 0xb6202044, 0xb63fb259, 0xb65f49e7, 0xb67ee6ee, 0xb69e8971, 0xb6be316f, 0xb6dddeea,
	0xb6fd91e3, 0xb71d4a5a, 0xb73d0851, 0xb75ccbc9, 0xb77c94c2, 0xb79c633e, 0xb7bc373d, 0xb7dc10c1,
	0xb7fbefca, 0xb81bd459, 0xb83bbe70, 0xb85bae0f, 0xb87ba337, 0xb89b9de9, 0xb8bb9e27, 0xb8dba3f0,
	0xb8fbaf47, 0xb91bc02b, 0xb93bd69f, 0xb95bf2a2, 0xb97c1437, 0xb99c3b5d, 0xb9bc6816, 0xb9dc9a63,
	0xb9fcd245, 0xba1d0fbc, 0xba3d52ca, 0xba5d9b70, 0xba7de9ae, 0xba9e3d86, 0xbabe96f9, 0xbadef607,
	0xbaff5ab2, 0xbb1fc4fa, 0xbb4034e0, 0xbb60aa66, 0xbb81258d, 0xbba1a655, 0xbbc22cbf, 0xbbe2b8cd,
	0xbc034a7e, 0xbc23e1d6, 0xbc447ed3, 0xbc652178, 0xbc85c9c5, 0xbca677bb, 0xbcc72b5b, 0xbce7e4a7,
	0xbd08a39f, 0xbd296844, 0xbd4a3297, 0xbd6b0299, 0xbd8bd84b, 0xbdacb3af, 0xbdcd94c4, 0xbdee7b8c,
	0xbe0f6809, 0xbe305a3b, 0xbe515222, 0xbe724fc1, 0xbe935317, 0xbeb45c27, 0xbed56af1, 0xbef67f75,
	0xbf1799b6, 0xbf38b9b4, 0xbf59df6f, 0xbf7b0aea, 0xbf9c3c24, 0xbfbd731f, 0xbfdeafdd, 0xbffff25d,
	0xc0213aa1, 0xc04288ab, 0xc063dc7a, 0xc0853610, 0xc0a6956e, 0xc0c7fa95, 0xc0e96586, 0xc10ad642,
	0xc12c4cca, 0xc14dc91f, 0xc16f4b42, 0xc190d333, 0xc1b260f5, 0xc1d3f488, 0xc1f58ded, 0xc2172d25,
	0xc238d231, 0xc25a7d12, 0xc27c2dc8, 0xc29de456, 0xc2bfa0bc, 0xc2e162fc, 0xc3032b15, 0xc324f909,
	0xc346ccda, 0xc368a687, 0xc38a8613, 0xc3ac6b7e, 0xc3ce56c9, 0xc3f047f5, 0xc4123f04, 0xc4343bf6,
	0xc4563ecc, 0xc4784787, 0xc49a5629, 0xc4bc6ab2, 0xc4de8523, 0xc500a57e, 0xc522cbc3, 0xc544f7f4,
	0xc5672a11, 0xc589621b, 0xc5aba014, 0xc5cde3fd, 0xc5f02dd6, 0xc6127da1, 0xc634d35e, 0xc6572f0f,
	0xc67990b5, 0xc69bf851, 0xc6be65e3, 0xc6e0d96d, 0xc70352f0, 0xc725d26c, 0xc74857e4, 0xc76ae358,
	0xc78d74c8, 0xc7b00c37, 0xc7d2a9a4, 0xc7f54d12, 0xc817f681, 0xc83aa5f2, 0xc85d5b66, 0xc88016de,
	0xc8a2d85c, 0xc8c59fe0, 0xc8e86d6c, 0xc90b40ff, 0xc92e1a9d, 0xc950fa45, 0xc973dff8, 0xc996cbb8,
	0xc9b9bd86, 0xc9dcb562, 0xc9ffb34f, 0xca22b74c, 0xca45c15a, 0xca68d17c, 0xca8be7b2, 0xcaaf03fd,
	0xcad2265e, 0xcaf54ed6, 0xcb187d66, 0xcb3bb20f, 0xcb5eecd3, 0xcb822db2, 0xcba574ae, 0xcbc8c1c7,
	0xcbec14fe, 0xcc0f6e56, 0xcc32cdcd, 0xcc563367, 0xcc799f23, 0xcc9d1104, 0xccc08909, 0xcce40734,
	0xcd078b86, 0xcd2b1600, 0xcd4ea6a3, 0xcd723d71, 0xcd95da6a, 0xcdb97d8f, 0xcddd26e2, 0xce00d664,
	0xce248c15, 0xce4847f6, 0xce6c0a0a, 0xce8fd250, 0xceb3a0ca, 0xced77579, 0xcefb505e, 0xcf1f317a,
	0xcf4318cf, 0xcf67065c, 0xcf8afa24, 0xcfaef428, 0xcfd2f468, 0xcff6fae5, 0xd01b07a2, 0xd03f1a9e,
	0xd06333da, 0xd0875359, 0xd0ab791b, 0xd0cfa521, 0xd0f3d76c, 0xd1180ffd, 0xd13c4ed6, 0xd16093f7,
	0xd184df62, 0xd1a93117, 0xd1cd8918, 0xd1f1e766, 0xd2164c02, 0xd23ab6ec, 0xd25f2827, 0xd2839fb3,
	0xd2a81d91, 0xd2cca1c3, 0xd2f12c49, 0xd315bd25, 0xd33a5457, 0xd35ef1e1, 0xd38395c4, 0xd3a84001,
	0xd3ccf099, 0xd3f1a78d, 0xd41664df, 0xd43b288e, 0xd45ff29e, 0xd484c30d, 0xd4a999df, 0xd4ce7713,
	0xd4f35aab, 0xd51844a8, 0xd53d350c, 0xd5622bd6, 0xd5872909, 0xd5ac2ca5, 0xd5d136ac, 0xd5f6471f,
	0xd61b5dfe, 0xd6407b4b, 0xd6659f08, 0xd68ac934, 0xd6aff9d1, 0xd6d530e1, 0xd6fa6e65, 0xd71fb25d,
	0xd744fcca, 0xd76a4daf, 0xd78fa50b, 0xd7b502e1, 0xd7da6731, 0xd7ffd1fc, 0xd8254343, 0xd84abb08,
	0xd870394c, 0xd895be0f, 0xd8bb4954, 0xd8e0db1b, 0xd9067364, 0xd92c1232, 0xd951b786, 0xd9776360,
	0xd99d15c2, 0xd9c2cead, 0xd9e88e21, 0xda0e5421, 0xda3420ad, 0xda59f3c7, 0xda7fcd6f, 0xdaa5ada6,
	0xdacb946f, 0xdaf181c9, 0xdb1775b6, 0xdb3d7038, 0xdb63714f, 0xdb8978fd, 0xdbaf8742, 0xdbd59c20,
	0xdbfbb797, 0xdc21d9aa, 0xdc480259, 0xdc6e31a6, 0xdc946791, 0xdcbaa41b, 0xdce0e747, 0xdd073114,
	0xdd2d8185, 0xdd53d899, 0xdd7a3653, 0xdda09ab4, 0xddc705bc, 0xdded776e, 0xde13efc9, 0xde3a6ecf,
	0xde60f482, 0xde8780e2, 0xdeae13f1, 0xded4adb0, 0xdefb4e1f, 0xdf21f541, 0xdf48a316, 0xdf6f579f,
	0xdf9612de, 0xdfbcd4d4, 0xdfe39d82, 0xe00a6ce9, 0xe031430a, 0xe0581fe6, 0xe07f037f, 0xe0a5edd6,
	0xe0ccdeec, 0xe0f3d6c2, 0xe11ad559, 0xe141dab2, 0xe168e6cf, 0xe18ff9b1, 0xe1b71359, 0xe1de33c8,
	0xe2055aff, 0xe22c8900, 0xe253bdcc, 0xe27af963, 0xe2a23bc7, 0xe2c984fa, 0xe2f0d4fc, 0xe3182bce,
	0xe33f8972, 0xe366ede9, 0xe38e5934, 0xe3b5cb55, 0xe3dd444c, 0xe404c41a, 0xe42c4ac2, 0xe453d843,
	0xe47b6ca0, 0xe4a307d9, 0xe4caa9ef, 0xe4f252e5, 0xe51a02ba, 0xe541b971, 0xe5697709, 0xe5913b86,
	0xe5b906e7, 0xe5e0d92e, 0xe608b25c, 0xe6309273, 0xe6587973, 0xe680675e, 0xe6a85c34, 0xe6d057f8,
	0xe6f85aaa, 0xe720644c, 0xe74874df, 0xe7708c63, 0xe798aada, 0xe7c0d046, 0xe7e8fca8, 0xe8113000,
	0xe8396a50, 0xe861ab99, 0xe889f3dd, 0xe8b2431c, 0xe8da9958, 0xe902f692, 0xe92b5acb, 0xe953c605,
	0xe97c3840, 0xe9a4b17e, 0xe9cd31c0, 0xe9f5b908, 0xea1e4756, 0xea46dcac, 0xea6f790a, 0xea981c73,
	0xeac0c6e7, 0xeae97868, 0xeb1230f7, 0xeb3af095, 0xeb63b743, 0xeb8c8502, 0xebb559d4, 0xebde35ba,
	0xec0718b6, 0xec3002c8, 0xec58f3f1, 0xec81ec33, 0xecaaeb8f, 0xecd3f207, 0xecfcff9b, 0xed26144d,
	0xed4f301e, 0xed785310, 0xeda17d22, 0xedcaae58, 0xedf3e6b1, 0xee1d2630, 0xee466cd5, 0xee6fbaa2,
	0xee990f98, 0xeec26bb7, 0xeeebcf03, 0xef15397b, 0xef3eab20, 0xef6823f5, 0xef91a3fb, 0xefbb2b32,
	0xefe4b99b, 0xf00e4f39, 0xf037ec0d, 0xf0619017, 0xf08b3b58, 0xf0b4edd3, 0xf0dea788, 0xf1086879,
	0xf13230a7, 0xf15c0013, 0xf185d6be, 0xf1afb4aa, 0xf1d999d8, 0xf2038649, 0xf22d79ff, 0xf25774fa,
	0xf281773c, 0xf2ab80c6, 0xf2d5919a, 0xf2ffa9b8, 0xf329c923, 0xf353efda, 0xf37e1de1, 0xf3a85337,
	0xf3d28fde, 0xf3fcd3d7, 0xf4271f24, 0xf45171c6, 0xf47bcbbe, 0xf4a62d0d, 0xf4d095b5, 0xf4fb05b7,
	0xf5257d15, 0xf54ffbce, 0xf57a81e6, 0xf5a50f5c, 0xf5cfa433, 0xf5fa406c, 0xf624e407, 0xf64f8f07,
	0xf67a416c, 0xf6a4fb38, 0xf6cfbc6c, 0xf6fa8509, 0xf7255510, 0xf7502c84, 0xf77b0b65, 0xf7a5f1b4,
	0xf7d0df73, 0xf7fbd4a2, 0xf826d145, 0xf851d55a, 0xf87ce0e5, 0xf8a7f3e6, 0xf8d30e5e, 0xf8fe3050,
	0xf92959bb, 0xf9548aa1, 0xf97fc305, 0xf9ab02e6, 0xf9d64a46, 0xfa019927, 0xfa2cef8a, 0xfa584d70,
	0xfa83b2db, 0xfaaf1fcb, 0xfada9443, 0xfb061042, 0xfb3193cc, 0xfb5d1ee0, 0xfb88b181, 0xfbb44baf,
	0xfbdfed6c, 0xfc0b96ba, 0xfc374799, 0xfc63000b, 0xfc8ec011, 0xfcba87ac, 0xfce656de, 0xfd122da9,
	0xfd3e0c0c, 0xfd69f20b, 0xfd95dfa6, 0xfdc1d4dd, 0xfdedd1b4, 0xfe19d62b, 0xfe45e243, 0xfe71f5fd,
	0xfe9e115c, 0xfeca3460, 0xfef65f0a, 0xff22915d, 0xff4ecb59, 0xff7b0cff, 0xffa75652, 0xffd3a751,
};

// Actual shape of the lfos unknown, since the hardware accepts 4 and
// 3 are in use (0, 1 and 3) and no recording are currently available

const std::array<u32, 4> swp30_device::lfo_shape_centered_saw = { 0x00000000, 0x00000000, 0xfff00000, 0xfff00000 }; // --////--
const std::array<u32, 4> swp30_device::lfo_shape_centered_tri = { 0x00000000, 0x0007ffff, 0xfff7ffff, 0xfff00000 }; // --/\/\--
const std::array<u32, 4> swp30_device::lfo_shape_offset_saw   = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 }; // __////__
const std::array<u32, 4> swp30_device::lfo_shape_offset_tri   = { 0x00000000, 0x00000000, 0x000fffff, 0x000fffff }; // __/\/\__

const std::array<u8, 4> swp30_device::dpcm_offset = { 7, 6, 4, 0 };

swp30_device::swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, SWP30, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  m_program_config("meg_program", ENDIANNESS_LITTLE, 64, 9, -3, address_map_constructor(FUNC(swp30_device::meg_prg_map), this)),
	  m_rom_config("sample_rom", ENDIANNESS_LITTLE, 32, 25, -2),
	  m_reverb_config("reverb_ram", ENDIANNESS_LITTLE, 16, 18, -1, address_map_constructor(FUNC(swp30_device::meg_reverb_map), this))
{
}

void swp30_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_rom     = &space(AS_DATA);
	m_reverb  = &space(AS_REVERB);
	m_rom->cache(m_rom_cache);
	m_reverb->cache(m_reverb_cache);

	state_add(STATE_GENPC,     "GENPC",     m_meg_pc).noshow();
	state_add(STATE_GENPCBASE, "CURPC",     m_meg_pc).noshow();
	state_add(0,               "PC",        m_meg_pc);

	set_icountptr(m_icount);

	m_stream = stream_alloc(0, 2, 44100, STREAM_SYNCHRONOUS);

	for(int i=0; i != 128; i++) {
		u32 v = 0;
		switch(i >> 3) {
		default:  v = ((i & 7) + 8) << (1 + (i >> 3)); break;
		case 0xb: v = ((i & 7) + 4) << 13; break;
		case 0xc: v = ((i & 6) + 6) << 14; break;
		case 0xd: v = ((i & 4) + 7) << 15; break;
		case 0xe: v = 15 << 15; break;
		case 0xf: v = 31 << 15; break;
		}
		m_global_step[i] = v;
	}

	// Delta-packed samples decompression.

	for(int i=0; i<128; i++) {
		s16 base = ((i & 0x1f) << (3+(i >> 5))) + (((1 << (i >> 5))-1) << 8);
		m_dpcm[i | 0x80] = - base;
		m_dpcm[i]        = + base;
	}

	save_item(NAME(m_keyon_mask));

	save_item(NAME(m_sample_start));
	save_item(NAME(m_sample_end));
	save_item(NAME(m_sample_address));
	save_item(NAME(m_pitch));

	save_item(NAME(m_release_glo));

	save_item(NAME(m_lfo_step_pmod));
	save_item(NAME(m_lfo_amod));

	save_item(NAME(m_attack));
	save_item(NAME(m_decay1));
	save_item(NAME(m_decay2));

	save_item(NAME(m_lfo_phase));
	save_item(NAME(m_sample_pos));
	save_item(NAME(m_envelope_level));
	save_item(NAME(m_envelope_on_timer));
	save_item(NAME(m_envelope_timer));
	save_item(NAME(m_decay2_done));
	save_item(NAME(m_envelope_mode));
	save_item(NAME(m_glo_level_cur));

	save_item(NAME(m_dpcm_current));
	save_item(NAME(m_dpcm_next));
	save_item(NAME(m_dpcm_address));
	save_item(NAME(m_dpcm_sum));

	save_item(NAME(m_sample_history));

	save_item(NAME(m_lpf_cutoff));
	save_item(NAME(m_lpf_cutoff_inc));
	save_item(NAME(m_lpf_reso));
	save_item(NAME(m_hpf_cutoff));
	save_item(NAME(m_eq_filter));

	save_item(NAME(m_internal_adr));

	save_item(NAME(m_meg_program_address));
	save_item(NAME(m_waverom_adr));
	save_item(NAME(m_waverom_mode));
	save_item(NAME(m_waverom_access));
	save_item(NAME(m_waverom_val));

	save_item(NAME(m_meg_program));
	save_item(NAME(m_meg_const));
	save_item(NAME(m_meg_offset));
	save_item(NAME(m_meg_lfo));
	save_item(NAME(m_meg_map));

	save_item(STRUCT_MEMBER(m_mixer, vol));
	save_item(STRUCT_MEMBER(m_mixer, route));
}

void swp30_device::device_reset()
{
	m_keyon_mask = 0;

	std::fill(m_sample_start.begin(), m_sample_start.end(), 0);
	std::fill(m_sample_end.begin(), m_sample_end.end(), 0);
	std::fill(m_sample_address.begin(), m_sample_address.end(), 0);
	std::fill(m_pitch.begin(), m_pitch.end(), 0);

	std::fill(m_release_glo.begin(), m_release_glo.end(), 0);

	std::fill(m_lfo_step_pmod.begin(), m_lfo_step_pmod.end(), 0);
	std::fill(m_lfo_amod.begin(), m_lfo_amod.end(), 0);

	std::fill(m_attack.begin(), m_attack.end(), 0);
	std::fill(m_decay1.begin(), m_decay1.end(), 0);
	std::fill(m_decay2.begin(), m_decay2.end(), 0);

	std::fill(m_lfo_phase.begin(), m_lfo_phase.end(), 0);
	std::fill(m_sample_pos.begin(), m_sample_pos.end(), 0);
	std::fill(m_envelope_level.begin(), m_envelope_level.end(), 0);
	std::fill(m_envelope_timer.begin(), m_envelope_timer.end(), 0);
	std::fill(m_envelope_on_timer.begin(), m_envelope_on_timer.end(), false);
	std::fill(m_decay2_done.begin(), m_decay2_done.end(), false);
	std::fill(m_envelope_mode.begin(), m_envelope_mode.end(), IDLE);
	std::fill(m_glo_level_cur.begin(), m_glo_level_cur.end(), 0);

	std::fill(m_dpcm_current.begin(), m_dpcm_current.end(), false);
	std::fill(m_dpcm_next.begin(), m_dpcm_next.end(), false);
	std::fill(m_dpcm_address.begin(), m_dpcm_address.end(), false);
	std::fill(m_dpcm_sum.begin(), m_dpcm_sum.end(), 0);

	std::fill(m_meg_program.begin(), m_meg_program.end(), 0);
	std::fill(m_meg_const.begin(), m_meg_const.end(), 0);
	std::fill(m_meg_offset.begin(), m_meg_offset.end(), 0);
	std::fill(m_meg_lfo.begin(), m_meg_lfo.end(), 0);
	std::fill(m_meg_map.begin(), m_meg_map.end(), 0);

	std::fill(m_mixer.begin(), m_mixer.end(), mixer_slot());

	memset(m_sample_history, 0, sizeof(m_sample_history));

	memset(m_lpf_cutoff, 0, sizeof(m_lpf_cutoff));
	memset(m_lpf_cutoff_inc, 0, sizeof(m_lpf_cutoff_inc));
	memset(m_lpf_reso, 0, sizeof(m_lpf_reso));
	memset(m_hpf_cutoff, 0, sizeof(m_hpf_cutoff));
	memset(m_eq_filter, 0, sizeof(m_eq_filter));

	m_meg_program_address = 0;
	m_waverom_adr = 0;
	m_waverom_mode = 0;
	m_waverom_access = 0;
	m_waverom_val = 0;
}

void swp30_device::map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(swp30_device::snd_r), FUNC(swp30_device::snd_w));

	rchan(map, 0x00).rw(FUNC(swp30_device::lpf_cutoff_r), FUNC(swp30_device::lpf_cutoff_w));
	rchan(map, 0x01).rw(FUNC(swp30_device::lpf_cutoff_inc_r), FUNC(swp30_device::lpf_cutoff_inc_w));
	rchan(map, 0x02).rw(FUNC(swp30_device::hpf_cutoff_r), FUNC(swp30_device::hpf_cutoff_w));
	// 03 seems to always get 5010 except at startup where it's 40ff
	rchan(map, 0x04).rw(FUNC(swp30_device::lpf_reso_r), FUNC(swp30_device::lpf_reso_w));
	rchan(map, 0x05).rw(FUNC(swp30_device::lfo_amod_r), FUNC(swp30_device::lfo_amod_w));
	rchan(map, 0x06).rw(FUNC(swp30_device::attack_r), FUNC(swp30_device::attack_w));
	rchan(map, 0x07).rw(FUNC(swp30_device::decay1_r), FUNC(swp30_device::decay1_w));
	rchan(map, 0x08).rw(FUNC(swp30_device::decay2_r), FUNC(swp30_device::decay2_w));
	rchan(map, 0x09).rw(FUNC(swp30_device::release_glo_r), FUNC(swp30_device::release_glo_w));
	rchan(map, 0x0a).rw(FUNC(swp30_device::lfo_step_pmod_r), FUNC(swp30_device::lfo_step_pmod_w));
	// 0b-0d missing
	// 10 missing
	rchan(map, 0x11).rw(FUNC(swp30_device::pitch_r), FUNC(swp30_device::pitch_w));
	rchan(map, 0x12).rw(FUNC(swp30_device::sample_start_h_r), FUNC(swp30_device::sample_start_h_w));
	rchan(map, 0x13).rw(FUNC(swp30_device::sample_start_l_r), FUNC(swp30_device::sample_start_l_w));
	rchan(map, 0x14).rw(FUNC(swp30_device::sample_end_h_r), FUNC(swp30_device::sample_end_h_w));
	rchan(map, 0x15).rw(FUNC(swp30_device::sample_end_l_r), FUNC(swp30_device::sample_end_l_w));
	rchan(map, 0x16).rw(FUNC(swp30_device::sample_address_h_r), FUNC(swp30_device::sample_address_h_w));
	rchan(map, 0x17).rw(FUNC(swp30_device::sample_address_l_r), FUNC(swp30_device::sample_address_l_w));
	rchan(map, 0x20).rw(FUNC(swp30_device::eq_filter_r<0>), FUNC(swp30_device::eq_filter_w<0>));
	rchan(map, 0x22).rw(FUNC(swp30_device::eq_filter_r<1>), FUNC(swp30_device::eq_filter_w<1>));
	rchan(map, 0x24).rw(FUNC(swp30_device::eq_filter_r<2>), FUNC(swp30_device::eq_filter_w<2>));
	rchan(map, 0x26).rw(FUNC(swp30_device::eq_filter_r<3>), FUNC(swp30_device::eq_filter_w<3>));
	rchan(map, 0x28).rw(FUNC(swp30_device::eq_filter_r<4>), FUNC(swp30_device::eq_filter_w<4>));
	rchan(map, 0x2a).rw(FUNC(swp30_device::eq_filter_r<5>), FUNC(swp30_device::eq_filter_w<5>));
	// 2c-2f missing

	// Control registers
	// These appear as channel slots 0x0e and 0x0f
	// 00-01 missing
	rctrl(map, 0x02).rw(FUNC(swp30_device::internal_adr_r), FUNC(swp30_device::internal_adr_w));
	rctrl(map, 0x03).r (FUNC(swp30_device::internal_r));
	rctrl(map, 0x04).rw(FUNC(swp30_device::waverom_adr_r<1>), FUNC(swp30_device::waverom_adr_w<1>));
	rctrl(map, 0x05).rw(FUNC(swp30_device::waverom_adr_r<0>), FUNC(swp30_device::waverom_adr_w<0>));
	rctrl(map, 0x06).rw(FUNC(swp30_device::waverom_mode_r<1>), FUNC(swp30_device::waverom_mode_w<1>));
	rctrl(map, 0x07).rw(FUNC(swp30_device::waverom_mode_r<0>), FUNC(swp30_device::waverom_mode_w<0>));
	rctrl(map, 0x08).rw(FUNC(swp30_device::waverom_access_r), FUNC(swp30_device::waverom_access_w));
	rctrl(map, 0x09).r (FUNC(swp30_device::waverom_busy_r));
	rctrl(map, 0x0a).r (FUNC(swp30_device::waverom_val_r<1>));
	rctrl(map, 0x0b).r (FUNC(swp30_device::waverom_val_r<0>));
	rctrl(map, 0x0c).rw(FUNC(swp30_device::keyon_mask_r<3>), FUNC(swp30_device::keyon_mask_w<3>));
	rctrl(map, 0x0d).rw(FUNC(swp30_device::keyon_mask_r<2>), FUNC(swp30_device::keyon_mask_w<2>));
	rctrl(map, 0x0e).rw(FUNC(swp30_device::keyon_mask_r<1>), FUNC(swp30_device::keyon_mask_w<1>));
	rctrl(map, 0x0f).rw(FUNC(swp30_device::keyon_mask_r<0>), FUNC(swp30_device::keyon_mask_w<0>));
	rctrl(map, 0x10).rw(FUNC(swp30_device::keyon_r), FUNC(swp30_device::keyon_w));
	// 11-20 missing
	rctrl(map, 0x21).rw(FUNC(swp30_device::meg_prg_address_r), FUNC(swp30_device::meg_prg_address_w));
	rctrl(map, 0x22).rw(FUNC(swp30_device::meg_prg_r<0>), FUNC(swp30_device::meg_prg_w<0>));
	rctrl(map, 0x23).rw(FUNC(swp30_device::meg_prg_r<1>), FUNC(swp30_device::meg_prg_w<1>));
	rctrl(map, 0x24).rw(FUNC(swp30_device::meg_prg_r<2>), FUNC(swp30_device::meg_prg_w<2>));
	rctrl(map, 0x25).rw(FUNC(swp30_device::meg_prg_r<3>), FUNC(swp30_device::meg_prg_w<3>));
	// 26-7f missing
	rctrl(map, 0x30).rw(FUNC(swp30_device::meg_map_r<0>), FUNC(swp30_device::meg_map_w<0>));
	rctrl(map, 0x32).rw(FUNC(swp30_device::meg_map_r<1>), FUNC(swp30_device::meg_map_w<1>));
	rctrl(map, 0x34).rw(FUNC(swp30_device::meg_map_r<2>), FUNC(swp30_device::meg_map_w<2>));
	rctrl(map, 0x36).rw(FUNC(swp30_device::meg_map_r<3>), FUNC(swp30_device::meg_map_w<3>));
	rctrl(map, 0x38).rw(FUNC(swp30_device::meg_map_r<4>), FUNC(swp30_device::meg_map_w<4>));
	rctrl(map, 0x3a).rw(FUNC(swp30_device::meg_map_r<5>), FUNC(swp30_device::meg_map_w<5>));
	rctrl(map, 0x3c).rw(FUNC(swp30_device::meg_map_r<6>), FUNC(swp30_device::meg_map_w<6>));
	rctrl(map, 0x3e).rw(FUNC(swp30_device::meg_map_r<7>), FUNC(swp30_device::meg_map_w<7>));

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
			m_sample_pos[chan] = -(m_sample_start[chan] & 0xffffff) << 8;
			if(m_release_glo[chan] & 0x8000) {
				m_envelope_level[chan] = 0;
				m_envelope_on_timer[chan] = false;
				m_envelope_mode[chan] = RELEASE;
			} else if(m_attack[chan] & 0x80) {
				m_envelope_level[chan] = 0x8000000;
				m_envelope_on_timer[chan] = false;
				m_envelope_mode[chan] = ATTACK;
			} else {
				m_envelope_level[chan] = 0;
				m_envelope_on_timer[chan] = true;
				m_envelope_timer[chan] = 0x8000000;
				m_envelope_mode[chan] = ATTACK;
			}

			m_decay2_done[chan] = false;
			m_glo_level_cur[chan] = (m_release_glo[chan] & 0xff) << 4;

			m_dpcm_current[chan] = 0;
			m_dpcm_next[chan] = 0;
			s32 dt = m_sample_start[chan] & 0xffffff;
			if(m_sample_end[chan] & 0x80000000)
				dt = -dt;
			m_dpcm_address[chan] = ((m_sample_address[chan] & 0xffffff) << 2) - dt;
			m_dpcm_sum[chan] = 0;

			m_lfo_phase[chan] = 0;

			if(1)
				logerror("[%08d] keyon %02x %08x %08x %08x vol %04x env %04x %04x %04x pitch %04x pmod %04x\n", scount, chan, m_sample_start[chan], m_sample_end[chan], m_sample_address[chan], m_release_glo[chan], m_attack[chan], m_decay1[chan], m_decay2[chan], m_pitch[chan], m_lfo_step_pmod[chan]);
		}
	}
	m_keyon_mask = 0;
}


u16 swp30_device::meg_prg_address_r()
{
	return m_meg_program_address;
}

void swp30_device::meg_prg_address_w(u16 data)
{
	m_meg_program_address = data;
	if(m_meg_program_address >= 0x180)
		m_meg_program_address = 0;
}

template<int sel> u16 swp30_device::meg_prg_r()
{
	constexpr offs_t shift = 48-16*sel;
	return m_meg_program[m_meg_program_address] >> shift;
}

template<int sel> void swp30_device::meg_prg_w(u16 data)
{
	constexpr offs_t shift = 48-16*sel;
	constexpr u64 mask = ~(u64(0xffff) << shift);
	m_meg_program[m_meg_program_address] = (m_meg_program[m_meg_program_address] & mask) | (u64(data) << shift);

	if(sel == 3) {
		if(0)
			logerror("program %03x %016x\n", m_meg_program_address, m_meg_program[m_meg_program_address]);
		m_meg_program_address ++;
		if(m_meg_program_address == 0x180)
			m_meg_program_address = 0;
	}
}


template<int sel> u16 swp30_device::meg_map_r()
{
	return m_meg_map[sel];
}

template<int sel> void swp30_device::meg_map_w(u16 data)
{
	m_meg_map[sel] = data;
	logerror("map %x  pc = %03x  base = %05x  size = %05x\n", sel, 12*(data >> 11), (data & 0xff) << 10, 0x400 << ((data >> 8) & 7));
}


template<int sel> void swp30_device::waverom_adr_w(u16 data)
{
	if(sel)
		m_waverom_adr = (m_waverom_adr & 0x0000ffff) | (data << 16);
	else
		m_waverom_adr = (m_waverom_adr & 0xffff0000) |  data;
}

template<int sel> u16 swp30_device::waverom_adr_r()
{
	return m_waverom_adr >> (16*sel);
}

template<int sel> void swp30_device::waverom_mode_w(u16 data)
{
	if(sel)
		m_waverom_mode = (m_waverom_mode & 0x0000ffff) | (data << 16);
	else
		m_waverom_mode = (m_waverom_mode & 0xffff0000) |  data;
}

template<int sel> u16 swp30_device::waverom_mode_r()
{
	return m_waverom_mode >> (16*sel);
}

void swp30_device::waverom_access_w(u16 data)
{
	m_waverom_access = data;
	if(data == 0x8000) {
		m_waverom_val = m_rom_cache.read_dword(m_waverom_adr);
		logerror("waverom read adr=%08x mode=%08x -> %08x\n", m_waverom_adr, m_waverom_mode, m_waverom_val);
	}
}

u16 swp30_device::waverom_access_r()
{
	return m_waverom_access;
}

u16 swp30_device::waverom_busy_r()
{
	// 0 = busy reading the rom, non-0 = finished
	return 0xffff;
}

template<int sel> u16 swp30_device::waverom_val_r()
{
	return m_waverom_val >> (16*sel);
}


// AWM2 per-channel registers
u16 swp30_device::lpf_cutoff_r(offs_t offset)
{
	return m_lpf_cutoff[offset >> 6];
}

void swp30_device::lpf_cutoff_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_lpf_cutoff[chan] != data)
		logerror("chan %02x lpf cutoff %04x\n", chan, data);
	m_lpf_cutoff[chan] = data;
}

u16 swp30_device::lpf_cutoff_inc_r(offs_t offset)
{
	return m_lpf_cutoff_inc[offset >> 6];
}

void swp30_device::lpf_cutoff_inc_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_lpf_cutoff_inc[chan] != data)
		logerror("chan %02x lpf cutoff increment %04x\n", chan, data);
	m_lpf_cutoff_inc[chan] = data;
}

u16 swp30_device::hpf_cutoff_r(offs_t offset)
{
	return m_hpf_cutoff[offset >> 6];
}

void swp30_device::hpf_cutoff_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_hpf_cutoff[chan] != data)
		logerror("chan %02x hpf cutoff %04x\n", chan, data);
	m_hpf_cutoff[chan] = data;
}

u16 swp30_device::lpf_reso_r(offs_t offset)
{
	return m_lpf_reso[offset >> 6];
}

void swp30_device::lpf_reso_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(0 && m_lpf_reso[chan] != data)
		logerror("chan %02x lpf resonance %04x\n", chan, data);
	m_lpf_reso[chan] = data;
}

template<int coef> u16 swp30_device::eq_filter_r(offs_t offset)
{
	return m_eq_filter[offset >> 6][coef];
}

template<int coef> void swp30_device::eq_filter_w(offs_t offset, u16 data)
{
	m_eq_filter[offset >> 6][coef] = data;
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

u16 swp30_device::release_glo_r(offs_t offset)
{
	return m_release_glo[offset >> 6];
}

void swp30_device::release_glo_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(1 && m_release_glo[chan] != data)
		logerror("snd chan %02x rel/glo %02x %02x\n", chan, data >> 8, data & 0xff);
	m_release_glo[chan] = data;
	if((data & 0x8000) && m_envelope_mode[chan] != IDLE && m_envelope_mode[chan] != RELEASE)
		m_envelope_mode[chan] = RELEASE;
}

u16 swp30_device::pitch_r(offs_t offset)
{
	return m_pitch[offset >> 6];
}

void swp30_device::pitch_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	//  delta is 4*256 per octave, positive means higher freq, e.g 4.10 format.
	s16 v = data & 0x2000 ? data | 0xc000 : data;
	if(0 && m_pitch[chan] != data)
		logerror("snd chan %02x pitch %c%c %d.%03x\n", chan, data & 0x8000 ? '#' : '.', data & 0x4000 ? '#' : '.', v / 1024, (v < 0 ? -v : v) & 0x3ff);
	m_pitch[chan] = data;
}

u16 swp30_device::attack_r(offs_t offset)
{
	return m_attack[offset >> 6];
}

void swp30_device::attack_w(offs_t offset, u16 data)
{
	if(data != m_attack[offset >> 6])
		logerror("attack[%02x] = %04x\n", offset >> 6, data);
	m_attack[offset >> 6] = data;
}

u16 swp30_device::decay1_r(offs_t offset)
{
	return m_decay1[offset >> 6];
}

void swp30_device::decay1_w(offs_t offset, u16 data)
{
	logerror("decay1[%02x] = %04x\n", offset >> 6, data);
	m_decay1[offset >> 6] = data;
}

u16 swp30_device::decay2_r(offs_t offset)
{
	return m_decay2[offset >> 6];
}

void swp30_device::decay2_w(offs_t offset, u16 data)
{
	logerror("decay2[%02x] = %04x\n", offset >> 6, data);
	m_decay2[offset >> 6] = data;
}

u16 swp30_device::lfo_step_pmod_r(offs_t offset)
{
	return m_lfo_step_pmod[offset >> 6];
}

void swp30_device::lfo_step_pmod_w(offs_t offset, u16 data)
{
	//  logerror("lfo_step_pmod[%02x] = %04x\n", offset >> 6, data);
	m_lfo_step_pmod[offset >> 6] = data;
}

u16 swp30_device::lfo_amod_r(offs_t offset)
{
	return m_lfo_amod[offset >> 6];
}

void swp30_device::lfo_amod_w(offs_t offset, u16 data)
{
	//  logerror("lfo_amod[%02x] = %04x\n", offset >> 6, data);
	m_lfo_amod[offset >> 6] = data;
}

u16 swp30_device::sample_start_h_r(offs_t offset)
{
	return m_sample_start[offset >> 6] >> 16;
}

u16 swp30_device::sample_start_l_r(offs_t offset)
{
	return m_sample_start[offset >> 6];
}

void swp30_device::sample_start_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_start[chan] = (m_sample_start[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::sample_start_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_start[chan] = (m_sample_start[chan] & 0xffff0000) | data;
}

u16 swp30_device::sample_end_h_r(offs_t offset)
{
	return m_sample_end[offset >> 6] >> 16;
}

u16 swp30_device::sample_end_l_r(offs_t offset)
{
	return m_sample_end[offset >> 6];
}

void swp30_device::sample_end_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_end[chan] = (m_sample_end[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::sample_end_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_end[chan] = (m_sample_end[chan] & 0xffff0000) | data;
	if(0)
		logerror("snd chan %02x post-size %02x %06x\n", chan, m_sample_end[chan] >> 24, m_sample_end[chan] & 0xffffff);
}

u16 swp30_device::sample_address_h_r(offs_t offset)
{
	return m_sample_address[offset >> 6] >> 16;
}

u16 swp30_device::sample_address_l_r(offs_t offset)
{
	return m_sample_address[offset >> 6];
}

void swp30_device::sample_address_h_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	m_sample_address[chan] = (m_sample_address[chan] & 0x0000ffff) | (data << 16);
}

void swp30_device::sample_address_l_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	static const char *const formats[4] = { "l16", "l12", "l8", "x8" };
	m_sample_address[chan] = (m_sample_address[chan] & 0xffff0000) | data;
	if(0)
		logerror("snd chan %02x format %s flags %02x address %06x\n", chan, formats[m_sample_address[chan] >> 30], (m_sample_address[chan] >> 24) & 0x3f, m_sample_address[chan] & 0xffffff);
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
		// Not certain about the two top bits though, the code seems to only care about 0/non-0
		return m_envelope_mode[chan] == IDLE ? 0xffff : ((m_envelope_mode[chan] - 1) << 14) | (m_envelope_level[chan] >> (28-14));

	case 4:
		// used at 44c4
		// tests & 0x4000 only
		//      logerror("read %02x.4\n", chan);
		return 0x0000;

	case 6:
		return m_decay2_done[chan] ? 0x0000 : 0x8000;
	}

	logerror("%s internal_r port %x channel %02x sample %d\n", machine().time().to_string(), m_internal_adr >> 8, m_internal_adr & 0x1f, scount);
	machine().debug_break();

	return 0;
}


// MEG registers

template<int sel> u16 swp30_device::meg_const_r(offs_t offset)
{
	return m_meg_const[(offset >> 6)*6 + sel];
}

template<int sel> void swp30_device::meg_const_w(offs_t offset, u16 data)
{
	m_meg_const[(offset >> 6)*6 + sel] = data;
}

template<int sel> u16 swp30_device::meg_offset_r(offs_t offset)
{
	return m_meg_offset[(offset >> 6)*2 + sel];
}

template<int sel> void swp30_device::meg_offset_w(offs_t offset, u16 data)
{
	m_meg_offset[(offset >> 6)*2 + sel] =  data;
}

template<int sel> u16 swp30_device::meg_lfo_r(offs_t offset)
{
	return m_meg_lfo[(offset >> 6)*2 + sel];
}

template<int sel> void swp30_device::meg_lfo_w(offs_t offset, u16 data)
{
	int slot = (offset >> 6)*2 + sel;
	m_meg_lfo[slot] = data;

	static const int dt[8] = { 0, 32, 64, 128, 256, 512,  1024, 2048 };
	static const int sh[8] = { 0,  0,  1,   2,   3,   4,     5,    6 };

	int scale = (data >> 5) & 7;
	int step = ((data & 31) << sh[scale]) + dt[scale];
	logerror("lfo_w %02x %04x freq=%5.2f phase=%6.4f\n", slot, m_meg_lfo[slot], step * 44100.0/4194304, (data >> 8)/256.0);
}



// Catch-all

static u16 rr[0x40*0x40];

u16 swp30_device::snd_r(offs_t offset)
{
	if(0) {
		int chan = (offset >> 6) & 0x3f;
		int slot = offset & 0x3f;
		std::string preg = "-";
		if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
			preg = util::string_format("fp%03x", (slot-0x21)/2 + 6*chan);
		else if(slot == 0x30 || slot == 0x31)
			preg = util::string_format("dt%02x", (slot-0x30) + 2*chan);
		else if(slot == 0x0e || slot == 0x0f)
			preg = util::string_format("ct%02x", (slot-0x0e) + 2*chan);
		else
			preg = util::string_format("%02x.%02x", chan, slot);
		logerror("snd_r [%04x %04x] %-5s, %04x\n", offset, offset*2, preg, rr[offset]);
	}
	if(offset == 0x080f)
		return 0;
	return rr[offset];
}

void swp30_device::snd_w(offs_t offset, u16 data)
{
	if(rr[offset] == data)
		return;

	rr[offset] = data;

	int chan = (offset >> 6) & 0x3f;
	int slot = offset & 0x3f;

	if(offset == 0x04e)
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
	map(0x000, 0x1bf).r(FUNC(swp30_device::meg_prg_map_r));
}

u64 swp30_device::meg_prg_map_r(offs_t address)
{
	return m_meg_program[address];
}

void swp30_device::meg_reverb_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
}

u16 swp30_device::swp30d_const_r(u16 address) const
{
	return m_meg_const[address];
}

u16 swp30_device::swp30d_offset_r(u16 address) const
{
	return m_meg_offset[address];
}

device_memory_interface::space_config_vector swp30_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_rom_config),
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

void swp30_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	outputs[0].put_int_clamp(0, m_meg_output[0], 32768);
	outputs[1].put_int_clamp(0, m_meg_output[1], 32768);
}

void swp30_device::change_mode_attack_decay1(int chan)
{
	m_envelope_mode[chan] = DECAY1;
	m_envelope_timer[chan] = 0x8000000;
	m_envelope_on_timer[chan] = (m_decay1[chan] & 0xff) == 0;
}

void swp30_device::change_mode_decay1_decay2(int chan)
{
	m_envelope_mode[chan] = DECAY2;
	m_envelope_timer[chan] = 0x8000000;
	m_envelope_on_timer[chan] = (m_decay2[chan] & 0xff) == (m_decay1[chan] & 0xff);
}

s32 swp30_device::meg_att(s32 sample, s32 att)
{
	if(att >= 0xff)
		return 0;
	return (sample - ((sample * (att & 0xf)) >> 4)) >> (att >> 4);

}

void swp30_device::execute_run()
{
	while(m_icount >= 0) {
		if(m_meg_pc == 0) {
			scount++;
			if(0) {
				static std::array<mixer_slot, 0x80> mixer;
				if(memcmp(mixer.data(), m_mixer.data(), sizeof(mixer))) {
					mixer = m_mixer;
					for(int i=0; i != 0x20; i++) {
						logerror("mixer %02x %04x.%04x.%04x %04x.%04x.%04x  %02x %04x.%04x.%04x %04x.%04x.%04x  %02x %04x.%04x.%04x %04x.%04x.%04x  %02x %04x.%04x.%04x %04x.%04x.%04x\n",
								 0x00 | i, m_mixer[0x00|i].vol[0], m_mixer[0x00|i].vol[1], m_mixer[0x00|i].vol[2], m_mixer[0x00|i].route[0], m_mixer[0x00|i].route[1], m_mixer[0x00|i].route[2],
								 0x20 | i, m_mixer[0x20|i].vol[0], m_mixer[0x20|i].vol[1], m_mixer[0x20|i].vol[2], m_mixer[0x20|i].route[0], m_mixer[0x20|i].route[1], m_mixer[0x20|i].route[2],
								 0x40 | i, m_mixer[0x40|i].vol[0], m_mixer[0x40|i].vol[1], m_mixer[0x40|i].vol[2], m_mixer[0x40|i].route[0], m_mixer[0x40|i].route[1], m_mixer[0x40|i].route[2],
								 0x60 | i, m_mixer[0x60|i].vol[0], m_mixer[0x60|i].vol[1], m_mixer[0x60|i].vol[2], m_mixer[0x60|i].route[0], m_mixer[0x60|i].route[1], m_mixer[0x60|i].route[2]);
					}
				}
			}


			//  AWM2 synthesis
			s32 samples_per_chan[0x40];
			for(int chan = 0; chan != 0x40; chan++) {
				if(m_envelope_mode[chan] == IDLE) {
					samples_per_chan[chan] = 0;
					continue;
				}

				// There actually are three shapes (0000, 4000 and c000) but
				// we're not sure what they are

				u32 lfo_phase = m_lfo_phase[chan] >> 7;
				s32 lfo_p_phase = lfo_phase ^ (m_lfo_step_pmod[chan] & 0xc000 ? lfo_shape_centered_tri : lfo_shape_centered_saw)[lfo_phase >> 18];
				s32 lfo_a_phase = lfo_phase ^ (m_lfo_step_pmod[chan] & 0xc000 ? lfo_shape_offset_tri   : lfo_shape_offset_saw  )[lfo_phase >> 18];

				lfo_p_phase = lfo_a_phase = 0;

				// First, read the sample

				// - Find the base sample index and base address
				s32 sample_pos = m_sample_pos[chan];
				if(m_sample_end[chan] & 0x80000000)
					sample_pos = -sample_pos;

				s32 spos = sample_pos >> 8;
				offs_t base_address = m_sample_address[chan] & 0x1ffffff;

				// - Read/decompress the sample
				s16 val0, val1;
				switch(m_sample_address[chan] >> 30) {
				case 0: { // 16-bits linear
					offs_t adr = base_address + (spos >> 1);
					switch(spos & 1) {
					case 0: { // ABCDabcd ........
						u32 l0 = m_rom_cache.read_dword(adr);
						val0 = l0;
						val1 = l0 >> 16;
						break;
					}
					case 1: { // abcd.... ....ABCD
						u32 l0 = m_rom_cache.read_dword(adr);
						u32 l1 = m_rom_cache.read_dword(adr+1);
						val0 = l0 >> 16;
						val1 = l1;
						break;
					}
					}
					break;
				}

				case 1: { // 12-bits linear
					offs_t adr = base_address + (spos >> 3)*3;
					switch(spos & 7) {
					case 0: { // ..ABCabc ........ ........ ........
						u32 l0 = m_rom_cache.read_dword(adr);
						val0 =  (l0 & 0x00000fff) << 4;
						val1 =  (l0 & 0x00fff000) >> 8;
						break;
					}
					case 1: { // BCabc... .......A ........ ........
						u32 l0 = m_rom_cache.read_dword(adr);
						u32 l1 = m_rom_cache.read_dword(adr+1);
						val0 =  (l0 & 0x00fff000) >> 8;
						val1 = ((l0 & 0xff000000) >> 20) | ((l1 & 0x0000000f) << 12);
						break;
					}
					case 2: { // bc...... ....ABCa ........ ........
						u32 l0 = m_rom_cache.read_dword(adr);
						u32 l1 = m_rom_cache.read_dword(adr+1);
						val0 = ((l0 & 0xff000000) >> 20) | ((l1 & 0x0000000f) << 12);
						val1 =   l1 & 0x0000fff0;
						break;
					}
					case 3: { // ........ .ABCabc. ........ ........
						u32 l1 = m_rom_cache.read_dword(adr+1);
						val0 =   l1 & 0x0000fff0;
						val1 =  (l1 & 0x0fff0000) >> 12;
						break;
					}
					case 4: { // ........ Cabc.... ......AB ........
						u32 l1 = m_rom_cache.read_dword(adr+1);
						u32 l2 = m_rom_cache.read_dword(adr+2);
						val0 =  (l1 & 0x0fff0000) >> 12;
						val1 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
						break;
					}
					case 5: { // ........ c....... ...ABCab ........
						u32 l1 = m_rom_cache.read_dword(adr+1);
						u32 l2 = m_rom_cache.read_dword(adr+2);
						val0 = ((l1 & 0xf0000000) >> 24) | ((l2 & 0x000000ff) << 8);
						val1 =  (l2 & 0x000fff00) >> 4;
						break;
					}
					case 6: { // ........ ........ ABCabc.. ........
						u32 l2 = m_rom_cache.read_dword(adr+2);
						val0 =  (l2 & 0x000fff00) >> 4;
						val1 =  (l2 & 0xfff00000) >> 16;
						break;
					}
					case 7: { // ........ ........ abc..... .....ABC
						u32 l2 = m_rom_cache.read_dword(adr+2);
						u32 l3 = m_rom_cache.read_dword(adr+3);
						val0 =  (l2 & 0xfff00000) >> 16;
						val1 =  (l3 & 0x00000fff) << 4;
						break;
					}
					}
					break;
				}

				case 2: { // 8-bits linear
					offs_t adr = base_address + (spos >> 2);
					switch(spos & 3) {
					case 0: { // ....ABab ........
						u32 l0 = m_rom_cache.read_dword(adr);
						val0 = (l0 & 0x000000ff) << 8;
						val1 =  l0 & 0x0000ff00;
						break;
					}
					case 1: { // ..ABab.. ........
						u32 l0 = m_rom_cache.read_dword(adr);
						val0 =  l0 & 0x0000ff00;
						val1 = (l0 & 0x00ff0000) >> 8;
						break;
					}
					case 2: { // ABab.... ........
						u32 l0 = m_rom_cache.read_dword(adr);
						val0 = (l0 & 0x00ff0000) >> 8;
						val1 = (l0 & 0xff000000) >> 16;
						break;
					}
					case 3: { // ab...... ......AB
						u32 l0 = m_rom_cache.read_dword(adr);
						u32 l1 = m_rom_cache.read_dword(adr+1);
						val0 = (l0 & 0xff000000) >> 16;
						val1 = (l1 & 0x000000ff) << 8;
						break;
					}
					}
					break;
				}

				case 3: { // 8-bits delta-pcm
					u8 offset = dpcm_offset[(m_sample_address[chan] >> 25) & 3];
					u8 scale = (m_sample_address[chan] >> 27) & 7;
					offs_t adr = m_dpcm_address[chan];
					if(m_sample_end[chan] & 0x80000000) {
						u32 target_address = (base_address << 2) + spos - 1;
						while(adr >= target_address) {
							m_dpcm_current[chan] = m_dpcm_next[chan];
							m_dpcm_sum[chan] += m_dpcm[(m_rom_cache.read_dword(adr >> 2) >> (8*(adr & 3))) & 0xff] - offset;
							s32 sample = (m_dpcm_sum[chan] << scale) >> 3;
							adr --;
							if(sample < -0x8000)
								sample = -0x8000;
							else if(sample > 0x7fff)
								sample = 0x7fff;
							m_dpcm_next[chan] = sample;
						}
					} else {
						u32 target_address = (base_address << 2) + spos + 1;
						while(adr <= target_address) {
							m_dpcm_current[chan] = m_dpcm_next[chan];
							m_dpcm_sum[chan] += m_dpcm[(m_rom_cache.read_dword(adr >> 2) >> (8*(adr & 3))) & 0xff] - offset;
							s32 sample = (m_dpcm_sum[chan] << scale) >> 3;
							//                          logerror("## +  sample %08x %02x %d\n", adr, (m_rom_cache.read_dword(adr >> 2) >> (8*(adr & 3))) & 0xff, sample);
							adr ++;
							if(sample < -0x8000)
								sample = -0x8000;
							else if(sample > 0x7fff)
								sample = 0x7fff;
							m_dpcm_next[chan] = sample;
						}
					}
					m_dpcm_address[chan] = adr;
					val0 = m_dpcm_current[chan];
					val1 = m_dpcm_next[chan];
					break;
				}
				}

				s32 mul = sample_pos & 0xff;
				s32 sample = val1 * mul + val0 * (0x100 - mul);

#if 0
				// Third, filter the sample
				// - missing lpf_cutoff, lpf_reso, hpf_cutoff

				// - eq lowpass
				s32 samp1 = (samp  * m_eq_filter[chan][2] + m_sample_history[chan][0][0] * m_eq_filter[chan][1] + m_sample_history[chan][0][1] * m_eq_filter[chan][0]) >> 13;
				m_sample_history[chan][0][1] = m_sample_history[chan][0][0];
				m_sample_history[chan][0][0] = samp;

				// - eq highpass
				s32 samp2 = (samp1 * m_eq_filter[chan][5] + m_sample_history[chan][1][0] * m_eq_filter[chan][4] + m_sample_history[chan][1][1] * m_eq_filter[chan][3]) >> 13;
				m_sample_history[chan][1][1] = m_sample_history[chan][1][0];
				m_sample_history[chan][1][0] = samp1;

#endif

				s32 tremolo_level = (lfo_a_phase * (m_lfo_amod[chan] & 0x1f)) << ((m_lfo_step_pmod[chan] & 0xc000) ? 3 : 2);

				samples_per_chan[chan] = fpapply(m_envelope_level[chan] + (m_glo_level_cur[chan] << 16) + tremolo_level, sample) >> 8;

				istep(m_glo_level_cur[chan], (m_release_glo[chan] & 0x00ff) << 4, 1);

				m_lfo_phase[chan] = (m_lfo_phase[chan] + m_global_step[0x20 + ((m_lfo_step_pmod[chan] >> 8) & 0x3f)]) & 0x7ffffff;

				u32 sample_increment = pitch_base[m_pitch[chan] & 0x3ff] >> (23 - ((s16(m_pitch[chan] << 2) >> 12)));
				m_sample_pos[chan] += (sample_increment * (0x800 + ((lfo_p_phase * (m_lfo_step_pmod[chan] & 0xff)) >> (m_lfo_step_pmod[chan] & 0xc000 ? 18 : 19)))) >> 11;
				if((m_sample_pos[chan] >> 8) >= (m_sample_end[chan] & 0xffffff)) {
					if(!(m_sample_end[chan] & 0xffffff))
						m_envelope_mode[chan] = IDLE;
					else {
						s32 prev = m_sample_pos[chan];
						do
							m_sample_pos[chan] -= (m_sample_end[chan] & 0xffffff) << 8;
						while((m_sample_pos[chan] >> 8) >= (m_sample_end[chan] & 0xffffff));
						if(m_sample_end[chan] & 0x80000000)
							m_dpcm_address[chan] -= (m_sample_pos[chan] >> 8) - (prev >> 8);
						else
							m_dpcm_address[chan] += (m_sample_pos[chan] >> 8) - (prev >> 8);
						m_dpcm_sum[chan] = 0;
					}
				}

				switch(m_envelope_mode[chan]) {
				case ATTACK:
					if(m_envelope_on_timer[chan]) {
						if(istep(m_envelope_timer[chan], 0, m_global_step[(m_attack[chan] >> 8) & 0x7f] << 1))
							change_mode_attack_decay1(chan);
					} else {
						if(fpstep(m_envelope_level[chan], 0, attack_linear_step[(m_attack[chan] >> 8) & 0x7f]))
							change_mode_attack_decay1(chan);
					}
					break;

				case DECAY1:
					if(m_envelope_on_timer[chan]) {
						if(istep(m_envelope_timer[chan], 0, m_global_step[(m_decay1[chan] >> 8) & 0x7f] << 1))
							change_mode_decay1_decay2(chan);
					} else if((m_decay1[chan] & 0x6000) == 0x6000) {
						if(fpstep(m_envelope_level[chan], (m_decay1[chan] & 0xff) << 20, decay_linear_step[(m_decay1[chan] >> 8) & 0x1f]))
							change_mode_decay1_decay2(chan);
					} else {
						if(fpstep(m_envelope_level[chan], (m_decay1[chan] & 0xff) << 20, m_global_step[(m_decay1[chan] >> 8) & 0x7f]))
							change_mode_decay1_decay2(chan);
					}
					break;

				case DECAY2:
					if(m_envelope_on_timer[chan])
						m_decay2_done[chan] = istep(m_envelope_timer[chan], 0, m_global_step[(m_decay1[chan] >> 8) & 0x7f] << 1);
					else if((m_decay2[chan] & 0x6000) == 0x6000)
						m_decay2_done[chan] = fpstep(m_envelope_level[chan], (m_decay2[chan] & 0xff) << 20, decay_linear_step[(m_decay2[chan] >> 8) & 0x1f]);
					else
						m_decay2_done[chan] = fpstep(m_envelope_level[chan], (m_decay2[chan] & 0xff) << 20, m_global_step[(m_decay2[chan] >> 8) & 0x7f]);
					break;

				case RELEASE:
					if((m_release_glo[chan] & 0x6000) == 0x6000) {
						if(fpstep(m_envelope_level[chan], 0x8000000, decay_linear_step[(m_release_glo[chan] >> 8) & 0x1f]))
							m_envelope_mode[chan] = IDLE;
					} else {
						if(fpstep(m_envelope_level[chan], 0x8000000, m_global_step[(m_release_glo[chan] >> 8) & 0x7f]))
							m_envelope_mode[chan] = IDLE;
					}
					break;
				}
			}

			// Mixer
			std::array<s32, 0x10> out_samples;
			std::copy(m_meg_m.begin() + 0x20, m_meg_m.begin() + 0x30, out_samples.begin());
			std::fill(m_meg_m.begin() + 0x20, m_meg_m.begin() + 0x30, 0);
			std::fill(m_melo.begin(), m_melo.end(), 0);
			std::fill(m_meg_output.begin(), m_meg_output.end(), 0);

			for(int mix = 0; mix != 0x60; mix++) {
				s32 input;
				if(mix  < 0x40)
					input = samples_per_chan[mix];
				else if(mix < 0x50)
					input = out_samples[mix & 0xf];
				else
					input = 0; // Audio input not yet supported in Mame (meli 0-7)

				if(input == 0)
					continue;

				u64 route = (u64(m_mixer[mix].route[0]) << 32) | (u64(m_mixer[mix].route[1]) << 16) | m_mixer[mix].route[2];
				const std::array<u16, 3> &vol = m_mixer[mix].vol;

				// It looks like this could be turned into something generic, but not 100% clear
				// routes 000100010001, 000200020002 etc seem to target the melo ports
				switch(route) {
				case 0x000000000000:
					// Incorrect, the program writes the outputs to
					// m30/m31, but right now the program doesn't run.
					m_meg_output[0] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_output[1] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					break;

				case 0x000100000000: // Used by the mu90, which does not write to 30/31
					m_meg_output[0] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_output[1] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					break;

				case 0x010000000000:
					m_meg_m[0x20] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x21] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					break;

				case 0x020000000000:
					m_meg_m[0x22] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x23] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					break;

				case 0x050000000400:
					m_meg_m[0x20] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x21] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					m_meg_m[0x24] += meg_att(input, (vol[0] >> 8)   + (vol[1] & 0xff));
					m_meg_m[0x25] += meg_att(input, (vol[0] & 0xff) + (vol[1] & 0xff));
					break;


				case 0x0d0008000400:
					m_meg_m[0x20] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x21] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					m_meg_m[0x24] += meg_att(input, (vol[0] >> 8)   + (vol[1] & 0xff));
					m_meg_m[0x25] += meg_att(input, (vol[0] & 0xff) + (vol[1] & 0xff));
					m_meg_m[0x26] += meg_att(input, (vol[0] >> 8)   + (vol[2] >> 8));
					m_meg_m[0x27] += meg_att(input, (vol[0] & 0xff) + (vol[2] >> 8));
					break;

				case 0x100010001000:
					m_meg_m[0x28] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x29] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					break;

				case 0x200020002000:
					m_meg_m[0x2a] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x2b] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					break;

				case 0x400040004000:
					m_meg_m[0x2c] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x2d] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					break;

				case 0x4d0048004400:
					m_meg_m[0x20] += meg_att(input, (vol[0] >> 8)   + (vol[1] >> 8));
					m_meg_m[0x21] += meg_att(input, (vol[0] & 0xff) + (vol[1] >> 8));
					m_meg_m[0x24] += meg_att(input, (vol[0] >> 8)   + (vol[1] & 0xff));
					m_meg_m[0x25] += meg_att(input, (vol[0] & 0xff) + (vol[1] & 0xff));
					m_meg_m[0x26] += meg_att(input, (vol[0] >> 8)   + (vol[2] >> 8));
					m_meg_m[0x27] += meg_att(input, (vol[0] & 0xff) + (vol[2] >> 8));
					m_meg_m[0x2c] += meg_att(input, (vol[0] >> 8)   + (vol[2] & 0xff));
					m_meg_m[0x2d] += meg_att(input, (vol[0] & 0xff) + (vol[2] & 0xff));
					break;

				default:
					logerror("Unhandled route %012x\n", route);
					break;
				}
			}
		}

		debugger_instruction_hook(m_meg_pc);
		m_icount --;
		m_meg_pc ++;
		if(m_meg_pc == 0x180)
			m_meg_pc = 0;
	}
}
