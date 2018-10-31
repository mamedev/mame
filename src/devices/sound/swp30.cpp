// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Yamaha SWP30/30B, rompler/dsp combo

#include "emu.h"
#include "swp30.h"

/*
  mode hall 1 reverb 0.3/2.0/2.1/30.0
    5ab/b56: 1ff2
    5e1/bc2: 1ff1
    5e3/bc6, 521/a42, 527/a4e, 563/ac6, 56b/ad6, 5a7/b4e, 5e3/bc6: 1d
    529/a52:          1a21/13d2/134f/ 70c
    4e9/9d2, 4eb/9d6:    5/14d0/1585/1ec3
    565/aca:          1a21/12cb/1247/ 684
    523/a46, 525/a4a:   10/1631/16d4/1eef
    5a1/b42:          1a21/11c2/113d/ 606
    561/ac2, 52b/ac2:   a2/176f/1801/1f15
    5a9/b5a:          1a20/10fb/1077/ 5ae
    567/ace, 569/ad2:   4d/1847/18cd/1f2e
    5e5/bca:          1a1f/ f74/ ef5/ 50b
    5a3/b46, 5a5/b46:   d2/19c2/1a31/1f58
    623/c46:          1a21/130d/1289/ 6a5
    5ab/b56, 5e1/bc2:    c/15dc/1683/1ee4


72814: move e0 to reg r0

62fcc: compute stuff

*/
#if 0
// 929a6: 00 2e 30 32 33 34 35 36 37 3a 3c

void f_62fcc(e0, r0, e1, r1l, er5) // 8 (11/12) 1 6 84268
{
	l8 = 929b1 + 6*e1;
	r6 = r1l == 6 ? 0x15 : r1l == 4 ? 0xd : r1l == 3 ? 0x9 : e1;
	f_64a34(0x01, 01, t929a6[e0].b << 8, 0x40, 0, er5 + 2*r6); // 0101 3700 40 0  84292
}

void write_regs(r0b, er1) // 728c0, r0=3, er1=84292
{
	if(!g214c98)
		return;
	for(r5=0; r5<r0b; r5++) {
		r6 = er1[r5].w; // 89 8a 8b
		e5 = (r6 % 6) + 0x21;
		r0 = r6 / 6;
		write_reg(r6/6, (r6 % 6)*2 + 0x21, g214cc2[r6].w);
	}
}
#endif


DEFINE_DEVICE_TYPE(SWP30, swp30_device, "swp30", "Yamaha SWP30 sound chip")

swp30_device::swp30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SWP30, tag, owner, clock),
	  device_sound_interface(mconfig, *this),
	  device_rom_interface(mconfig, *this, 25+2, ENDIANNESS_LITTLE, 32)
{	
}

void swp30_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);

	// Attenuantion for panning is logarithmic on one byte where 0x10
	// means divide by two.  It means 0 to -96dB with steps of
	// 0.375dB.  Since it's a nice scale, we assume it's the same for
	// other attenuation values.
	for(int i=0; i<256; i++)
		m_linear_attenuation[i] = 65536.0/pow(2, i/16.0);

	// Relative playback frequency of a sample is encoded on signed 14
	// bits.  The scale is logarithmic, with 0x400 = 1 octave (e.g. *2
	// or /2).
	for(int i=-0x20000; i<0x2000; i++)
		m_sample_increment[i & 0x3fff] = 256 * pow(2, i/1024.0);

	// Log to linear 8-bits sample decompression.  Statistics say
	// that's what it should look like.  Note that 0 can be encoded
	// both as 0x00 and 0x80, and as it happens 0x80 is never used in
	// these samples.

	//  Rescale so that it's roughly 16 bits.  Range ends up being +/- 78c0.

	for(int i=0; i<32; i++) {
		m_sample_log8[     i] =  i << 0;
		m_sample_log8[0x20|i] = (i << 1) + 0x21;
		m_sample_log8[0x40|i] = (i << 2) + 0x62;
		m_sample_log8[0x60|i] = (i << 3) + 0xe3;
	}
	for(int i=0; i<128; i++) {
		m_sample_log8[i] =  m_sample_log8[i] << 6;
		m_sample_log8[i | 0x80] = -m_sample_log8[i];
	}

}

void swp30_device::device_reset()
{
	memset(m_pre_size, 0, sizeof(m_pre_size));
	memset(m_post_size, 0, sizeof(m_post_size));
	memset(m_address, 0, sizeof(m_address));
	memset(m_volume, 0, sizeof(m_volume));
	memset(m_freq, 0, sizeof(m_freq));
	memset(m_pan, 0, sizeof(m_pan));

	m_keyon_mask = 0;
	m_active_mask = 0;
}

void swp30_device::rom_bank_updated()
{
	m_stream->update();
}

void swp30_device::map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(swp30_device::snd_r), FUNC(swp30_device::snd_w));
	map(0x0012, 0x0013).select(0x1f80).rw(FUNC(swp30_device::volume_r), FUNC(swp30_device::volume_w));
	map(0x0022, 0x0023).select(0x1f80).rw(FUNC(swp30_device::freq_r), FUNC(swp30_device::freq_w));
	map(0x0024, 0x0027).select(0x1f80).rw(FUNC(swp30_device::pre_size_r), FUNC(swp30_device::pre_size_w));
	map(0x0028, 0x002b).select(0x1f80).rw(FUNC(swp30_device::post_size_r), FUNC(swp30_device::post_size_w));
	map(0x002c, 0x002f).select(0x1f80).rw(FUNC(swp30_device::address_r), FUNC(swp30_device::address_w));
	map(0x0064, 0x0065).select(0x1f80).rw(FUNC(swp30_device::pan_r), FUNC(swp30_device::pan_w));
	map(0x031c, 0x031d).               rw(FUNC(swp30_device::keyon_mask_r<3>), FUNC(swp30_device::keyon_mask_w<3>));
	map(0x031e, 0x031f).               rw(FUNC(swp30_device::keyon_mask_r<2>), FUNC(swp30_device::keyon_mask_w<2>));
	map(0x039c, 0x039d).               rw(FUNC(swp30_device::keyon_mask_r<1>), FUNC(swp30_device::keyon_mask_w<1>));
	map(0x039e, 0x039f).               rw(FUNC(swp30_device::keyon_mask_r<0>), FUNC(swp30_device::keyon_mask_w<0>));
	map(0x041c, 0x041d).               rw(FUNC(swp30_device::keyon_r), FUNC(swp30_device::keyon_w));
}


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
	m_stream->update();
	static int count = 0;
	for(int i=0; i<64; i++) {
		u64 mask = u64(1) << i;
		if((m_keyon_mask & mask) && !(m_active_mask & mask) && !(m_volume[i] & 0x8000)) {
			m_sample_pos[i] = -s32(m_pre_size[i] << 8);
			logerror("sample count %3d %02x %08x %08x %08x vol %04x pan %04x\n", count, i, m_pre_size[i], m_post_size[i], m_address[i], m_volume[i], m_pan[i]);
			m_active_mask |= mask;
			count++;
		}
	}
	m_keyon_mask = 0;
}

u16 swp30_device::volume_r(offs_t offset)
{
	int chan = offset >> 6;
	return m_volume[chan];
}

void swp30_device::volume_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_volume[chan] != data)
		logerror("snd chan %02x volume %02x %02x\n", chan, data >> 8, data & 0xff);
	m_volume[chan] = data;
	if(data & 0x8000)
		m_active_mask &= ~(u64(1) << chan);
}


u16 swp30_device::pan_r(offs_t offset)
{
	return m_pan[offset >> 6];
}

void swp30_device::pan_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	double p1 = pow(2, -(data >> 8)/16.0);
	double p2 = pow(2, -(data & 0xff)/16.0);
	if(m_pan[chan] != data)
		logerror("snd chan %02x pan l %02x r %02x (%g %g %g)\n", chan, data >> 8, data & 0xff, p1, p2, sqrt(p1*p1+p2*p2));
	m_pan[chan] = data;
}

u16 swp30_device::freq_r(offs_t offset)
{
	return m_freq[offset >> 6];
}

void swp30_device::freq_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	//	delta is 4*256 per octave, positive means higher freq, e.g 4.10 format.
	s16 v = data & 0x2000 ? data | 0xc000 : data;
	if(m_freq[chan] != data)
		logerror("snd chan %02x freq %c%c %d.%03x\n", chan, data & 0x8000 ? '#' : '.', data & 0x4000 ? '#' : '.', v / 1024, (v < 0 ? -v : v) & 0x3ff);
	m_freq[chan] = data;
}

u16 swp30_device::pre_size_r(offs_t offset)
{
	if(offset & 1)
		return m_pre_size[offset >> 6];
	else
		return m_pre_size[offset >> 6] >> 16;
}

void swp30_device::pre_size_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(offset & 1) {
		m_pre_size[chan] = (m_pre_size[chan] & 0xffff0000) | data;
		logerror("snd chan %02x pre-size %02x %06x\n", chan, m_pre_size[chan] >> 24, m_pre_size[chan] & 0xffffff);
	} else
		m_pre_size[chan] = (m_pre_size[chan] & 0x0000ffff) | (data << 16);
}

u16 swp30_device::post_size_r(offs_t offset)
{
	if(offset & 1)
		return m_post_size[offset >> 6];
	else
		return m_post_size[offset >> 6] >> 16;
}

void swp30_device::post_size_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(offset & 1) {
		m_post_size[chan] = (m_post_size[chan] & 0xffff0000) | data;
		logerror("snd chan %02x post-size %02x %06x\n", chan, m_post_size[chan] >> 24, m_post_size[chan] & 0xffffff);
	} else
		m_post_size[chan] = (m_post_size[chan] & 0x0000ffff) | (data << 16);
}

u16 swp30_device::address_r(offs_t offset)
{
	if(offset & 1)
		return m_address[offset >> 6];
	else
		return m_address[offset >> 6] >> 16;
}

void swp30_device::address_w(offs_t offset, u16 data)
{
	u8 chan = offset >> 6;
	if(offset & 1) {
		// The address may be 25 bits
		static const char *const formats[4] = { "l16", "l12", "l8", "x8" };
		m_address[chan] = (m_address[chan] & 0xffff0000) | data;
		logerror("snd chan %02x format %s flags %02x address %06x\n", chan, formats[m_address[chan] >> 30], (m_address[chan] >> 24) & 0x3f, m_address[chan] & 0xffffff);
	} else
		m_address[chan] = (m_address[chan] & 0x0000ffff) | (data << 16);
}

static u16 rr[0x40*0x40];

u16 swp30_device::snd_r(offs_t offset)
{
	if(0) {
		int chan = (offset >> 6) & 0x3f;
		int slot = offset & 0x3f;
		std::string preg = "-";
		if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
			preg = util::string_format("%03x", (slot-0x21)/2 + 6*chan);
		logerror("snd_r [%04x %04x - %-4s] %02x.%02x  %04x\n", offset, offset*2, preg, chan, slot, rr[offset]);
	}
	if(offset == 0x080f)
		return rr[offset] & ~8;
	//	return chan == 0x20 && slot == 0xf ? 0 : 0xffff;
	return rr[offset];
}

void swp30_device::snd_w(offs_t offset, u16 data)
{
	if(rr[offset] == data)
		return;
	if(offset == 0x4e && (data ^ rr[offset]) == 0x400) {
		rr[offset] = data;
		return;
	}
	rr[offset] = data;
	int chan = (offset >> 6) & 0x3f;
	int slot = offset & 0x3f;
	std::string preg = "-";
	if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
		preg = util::string_format("%03x", (slot-0x21)/2 + 6*chan);
	logerror("snd_w [%04x %04x - %-4s] %02x.%02x, %04x\n", offset, offset*2, preg, chan, slot, data);
}

/*
[:swp30] sample 00 46a86f [3] -5848 0180  00ff 3e06
[:swp30] sample 01 4a2a69 [1] -5789 0400  001b 0122
[:swp30] sample 02 42a89b [3] -4110 0100  00ff 0a3a
[:swp30] sample 03 46ce62 [3] -6727 0380  00ff 3e06
[:swp30] sample 08 427193 [3] -4190 0540  00ff 0a3a
#[:swp30] sample 09 94ba25 [0] -11267 0000  0026 0830
[:swp30] sample 0b 41b5c1 [3]  +617 fe80  003c 0a3a
#[:swp30] sample 20 455235 [3] +19583 0580  002f 3e06
#[:swp30] sample 24 4117b1 [3] +18465 fa80  0034 0a3a
#[:swp30] sample 2a 41f4bf [3] +19969 ff40  0039 0a3a
[:swp30] sample 2d 467938 [3] +7441 ff40  00ff 3e06
[:swp30] sample 30 435fd3 [3] -5882 fdc0  0039 0a3a
[:swp30] sample 33 431a0a [3] +1960 0340  0043 0a3a
#[:swp30] sample 36 94ba25 [0] -2174 0000  0047 081c
[:swp30] sample 39 42e187 [3] +4221 ffc0  00ff 0a3a
[:swp30] sample 3a 46f798 [3] +2522 0000  00ff 3e06

[:swp30] sample id  1226729 085f 0402

[:swp30] sample 00 46a86f [3] -5847 00c0  00ff 3e06
[:swp30] sample 01 4a2a69 [1] -5788 3010  001b 0122
[:swp30] sample 02 42a89b [3] -4109 00c0  00ff 0a3a
[:swp30] sample 03 46ce62 [3] -6726 0000  00ff 3e06
[:swp30] sample 08 427193 [3] -4189 03c0  00ff 0a3a
#[:swp30] sample 09 94ba25 [0] -11267 0000  0026 0830
[:swp30] sample 0b 41b5c1 [3]  +618 0180  003c 0a3a
#[:swp30] sample 20 455235 [3] +19583 0580  002f 3e06
#[:swp30] sample 24 4117b1 [3] +18465 fa80  0034 0a3a
#[:swp30] sample 2a 41f4bf [3] +19970 fe40  0039 0a3a
[:swp30] sample 2d 467938 [3] +7442 ff00  00ff 3e06
[:swp30] sample 30 435fd3 [3] -5882 fdc0  0039 0a3a
[:swp30] sample 33 431a0a [3] +1961 0040  0043 0a3a
#[:swp30] sample 36 94ba25 [0] -2173 0000  0047 081c
[:swp30] sample 39 42e187 [3] +4222 02c0  00ff 0a3a
[:swp30] sample 3a 46f798 [3] +2523 ff80  00ff 3e06

[:swp30] sample id  1226730 0ea3 2e06
*/

 //static int sid = 0;

void swp30_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	// Loop first on the samples and not on the channels otherwise
	// effects will be impossible to implement.

	for(int sample = 0; sample < samples; sample++) {
		// Accumulate on 64 bits, shift/clamp at the end
		s64 acc_left = 0, acc_right = 0;

		// Loop on channels
		for(int channel = 0; channel < 64; channel++)
			if(m_active_mask & (u64(1) << channel)) {
				// First, read the sample

				// - Find the base sample index and base address
				s32 spos = m_sample_pos[channel] >> 8;
				offs_t base_address = (m_address[channel] & 0x1ffffff) << 2;
				// - Read/decompress the sample
				s16 samp = 0;
				switch(m_address[channel] >> 30) {
				case 0: { // 16-bits linear
					offs_t adr = base_address + (spos << 1);
					samp = read_word(adr);
					break;
				}

				case 1: { // 12-bits linear
					offs_t adr = base_address + (spos >> 2)*6;
					switch(spos & 3) {
					case 0: { // .abc .... ....
						u16 w0 = read_word(adr);
						samp = (w0 & 0x0fff) << 4;
						break;
					}
					case 1: { // C... ..AB ....
						u16 w0 = read_word(adr);
						u16 w1 = read_word(adr+2);
						samp = ((w0 & 0xf000) >> 8) | ((w1 & 0x00ff) << 8);
						break;
					}
					case 2: { // .... bc.. ...a
						u16 w0 = read_word(adr+2);
						u16 w1 = read_word(adr+4);
						samp = ((w0 & 0xff00) >> 4) | ((w1 & 0x000f) << 12);
						break;
					}
					case 3: { // .... .... ABC.
						u16 w1 = read_word(adr+4);
						samp = w1 & 0xfff0;
						break;
					}
					}
					break;
				}

				case 2:   // 8-bits linear
					samp = read_byte(base_address + spos) << 8;
					break;

				case 3:   // 8-bits logarithmic
					samp = m_sample_log8[read_byte(base_address + spos)];
					break;
				}
				
				//				logerror("sample %02x %06x [%d] %+5d %04x  %04x %04x\n", channel, base_address >> 2, m_address[channel] >> 30, spos, samp & 0xffff, m_volume[channel], m_pan[channel]);

				// Second, step the sample pos, loop/deactivate as needed
				m_sample_pos[channel] += m_sample_increment[m_freq[channel] & 0x3fff];
				s32 loop_size = m_post_size[channel] << 8;
				if(m_sample_pos[channel] >= loop_size) {
					// We reached the loop point, stop if loop size is zero,
					// otherwise loop
					if(!loop_size)
						m_active_mask &= ~((u64(1) << channel));
					else
						do
							m_sample_pos[channel] -= loop_size;
						while(m_sample_pos[channel] >= loop_size);
				}

				// Third, filter the sample

				// - no idea what's needed there

				// Fourth, volume and pan, clamp the attenuation at -96dB
				s32 sampl = samp * m_linear_attenuation[std::min(0xff, (m_volume[channel] & 0x00) + (m_pan[channel] >> 8))];
				s32 sampr = samp * m_linear_attenuation[std::min(0xff, (m_volume[channel] & 0x00) + (m_pan[channel] & 0xff))];

				// Fifth, add to the accumulators
				acc_left  += sampl;
				acc_right += sampr;

				// Missing: reverb, chorus, effects in general
			}

		// Samples are 16 bits, there are up to 64 of them, and the accumulators are fixed-point signed 48.16
		acc_left >>= (16+6);
		if(acc_left < -0x8000)
			acc_left = -0x8000;
		else if(acc_left > 0x7fff)
			acc_left = 0x7fff;
		outputs[0][sample] = acc_left;

		acc_right >>= (16+6);
		if(acc_right < -0x8000)
			acc_right = -0x8000;
		else if(acc_right > 0x7fff)
			acc_right = 0x7fff;
		outputs[1][sample] = acc_right;

		//		logerror("sample id %8d %04x %04x %016x [%08x]\n", sid, acc_right & 0xffff, acc_left & 0xffff, m_active_mask, m_address[4]);
		//		sid++;
	}
}
