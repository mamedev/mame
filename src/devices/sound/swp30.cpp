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

// 3fb80: list of reverb times as text, 4chars/entry

// 84268: 7c 76 77 78
//        80 79 7a 7b
//        84 7e 7d 7f
//        88 81 82 83
//        8c 85 86 87
//        91 89 8a 8b


void f_62fcc(e0, r0, e1, r1l, er5) // 8 (11/12) 1 6 84268
{
	// 929b1
	// 00 00 00 00 00 00
	// 38 3c 40 43 49 3b
	// 46 4a 4e 51 55 43
	// 3e 3f 41 43 48 3c
	// 4b 51 59 5c 00 4e

	// 92560
	// 00 01 12 1c 23 28 2d 30 34 37 39 3c 3e 40 42 43
	// 45 46 48 49 4a 4c 4d 4e 4f 50 51 52 53 54 54 55
	// 56 57 58 58 59 5a 5a 5b 5c 5c 5d 5d 5e 5e 5f 60
	// 60 61 61 63 66 68 69 6b 6d 6e 70 71 72 75 77 79
	// 7b 7c 7e 7f 81 82

	l8 = 929b1 + 6*e1;
	r6 = r1l == 6 ? 0x15 : r1l == 4 ? 0xd : r1l == 3 ? 0x9 : e1;
	f_64a34(0x01, 01, t929a6[e0].b << 8, 0x40, 0, er5 + 2*r6); // 0101 3700 40 0  84292 - writes 1ff2/1ff1
	pr0 = r0;
	for(r3h=0; r3h<6; r3h++) {
		r0l = *l8++;
		if(r0l) {
			r6 = clamp(r0l + l92560[pr0] - 0x38, 0, 0xff);
			r6, e6 = l925a6[r6].l; // 13d2 5368
		} else
			r6, e6 = 0, 0;
		
	}
	er0 = *er5.w++;
	g214ca4[0x10 + er0].w = e6;
	lc = &g214ca4[0x20 + er0].w
	er0 = float(r6);
}

// 80914 = convert unsigned short to float

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
	m_keyon_mask = 0;
	m_active_mask = 0;

	memset(m_pre_size, 0, sizeof(m_pre_size));
	memset(m_post_size, 0, sizeof(m_post_size));
	memset(m_address, 0, sizeof(m_address));

	memset(m_sample_pos, 0, sizeof(m_sample_pos));
	memset(m_sample_history, 0, sizeof(m_sample_history));

	memset(m_volume, 0, sizeof(m_volume));
	memset(m_freq, 0, sizeof(m_freq));
	memset(m_pan, 0, sizeof(m_pan));
	memset(m_lpf_cutoff, 0, sizeof(m_lpf_cutoff));
	memset(m_lpf_reso, 0, sizeof(m_lpf_reso));
	memset(m_hpf_cutoff, 0, sizeof(m_hpf_cutoff));
	memset(m_eq_filter, 0, sizeof(m_eq_filter));

	memset(m_variation_delay, 0, sizeof(m_variation_delay));
	memset(m_variation_fb_level, 0, sizeof(m_variation_fb_level));
	memset(m_variation_final_level, 0, sizeof(m_variation_final_level));
	memset(m_variation_high_damp, 0, sizeof(m_variation_high_damp));
	memset(m_variation_inv_high_damp, 0, sizeof(m_variation_inv_high_damp));

	memset(m_insertion_delay, 0, sizeof(m_insertion_delay));
	memset(m_insertion_fb_level, 0, sizeof(m_insertion_fb_level));
	memset(m_insertion_final_level, 0, sizeof(m_insertion_final_level));
	memset(m_insertion_high_damp, 0, sizeof(m_insertion_high_damp));
	memset(m_insertion_inv_high_damp, 0, sizeof(m_insertion_inv_high_damp));
}

void swp30_device::rom_bank_updated()
{
	m_stream->update();
}

void swp30_device::map(address_map &map)
{
	map(0x0000, 0x1fff).rw(FUNC(swp30_device::snd_r), FUNC(swp30_device::snd_w));
	map(0x0000, 0x0001).select(0x1f80).rw(FUNC(swp30_device::lpf_cutoff_r), FUNC(swp30_device::lpf_cutoff_w));
	map(0x0004, 0x0005).select(0x1f80).rw(FUNC(swp30_device::hpf_cutoff_r), FUNC(swp30_device::hpf_cutoff_w));
	map(0x0008, 0x0009).select(0x1f80).rw(FUNC(swp30_device::lpf_reso_r), FUNC(swp30_device::lpf_reso_w));
	map(0x0012, 0x0013).select(0x1f80).rw(FUNC(swp30_device::volume_r), FUNC(swp30_device::volume_w));
	map(0x0022, 0x0023).select(0x1f80).rw(FUNC(swp30_device::freq_r), FUNC(swp30_device::freq_w));
	map(0x0024, 0x0027).select(0x1f80).rw(FUNC(swp30_device::pre_size_r), FUNC(swp30_device::pre_size_w));
	map(0x0028, 0x002b).select(0x1f80).rw(FUNC(swp30_device::post_size_r), FUNC(swp30_device::post_size_w));
	map(0x002c, 0x002f).select(0x1f80).rw(FUNC(swp30_device::address_r), FUNC(swp30_device::address_w));
	map(0x0040, 0x0041).select(0x1f80).rw(&swp30_device::eq_filter_r<0, 0>, "swp30_device::eq_filter_r<0, 0>", &swp30_device::eq_filter_w<0, 0>, "swp30_device::eq_filter_w<0, 0>");
	map(0x0044, 0x0045).select(0x1f80).rw(&swp30_device::eq_filter_r<0, 1>, "swp30_device::eq_filter_r<0, 1>", &swp30_device::eq_filter_w<0, 1>, "swp30_device::eq_filter_w<0, 1>");
	map(0x0048, 0x0049).select(0x1f80).rw(&swp30_device::eq_filter_r<0, 2>, "swp30_device::eq_filter_r<0, 2>", &swp30_device::eq_filter_w<0, 2>, "swp30_device::eq_filter_w<0, 2>");
	map(0x004c, 0x004d).select(0x1f80).rw(&swp30_device::eq_filter_r<1, 0>, "swp30_device::eq_filter_r<1, 0>", &swp30_device::eq_filter_w<1, 0>, "swp30_device::eq_filter_w<1, 0>");
	map(0x0050, 0x0051).select(0x1f80).rw(&swp30_device::eq_filter_r<1, 1>, "swp30_device::eq_filter_r<1, 1>", &swp30_device::eq_filter_w<1, 1>, "swp30_device::eq_filter_w<1, 1>");
	map(0x0054, 0x0055).select(0x1f80).rw(&swp30_device::eq_filter_r<1, 2>, "swp30_device::eq_filter_r<1, 2>", &swp30_device::eq_filter_w<1, 2>, "swp30_device::eq_filter_w<1, 2>");
	map(0x0064, 0x0065).select(0x1f80).rw(FUNC(swp30_device::pan_r), FUNC(swp30_device::pan_w));
	map(0x031c, 0x031d).               rw(FUNC(swp30_device::keyon_mask_r<3>), FUNC(swp30_device::keyon_mask_w<3>));
	map(0x031e, 0x031f).               rw(FUNC(swp30_device::keyon_mask_r<2>), FUNC(swp30_device::keyon_mask_w<2>));
	map(0x039c, 0x039d).               rw(FUNC(swp30_device::keyon_mask_r<1>), FUNC(swp30_device::keyon_mask_w<1>));
	map(0x039e, 0x039f).               rw(FUNC(swp30_device::keyon_mask_r<0>), FUNC(swp30_device::keyon_mask_w<0>));
	map(0x041c, 0x041d).               rw(FUNC(swp30_device::keyon_r), FUNC(swp30_device::keyon_w));

	map(0x1060, 0x1061).               rw(&swp30_device::insertion_delay_r<0, 0>, "swp30_device::insertion_delay_r<0, 0>", &swp30_device::insertion_delay_w<0, 0>, "swp30_device::insertion_delay_w<0, 0>");
	map(0x1062, 0x1063).               rw(&swp30_device::insertion_delay_r<0, 1>, "swp30_device::insertion_delay_r<0, 1>", &swp30_device::insertion_delay_w<0, 1>, "swp30_device::insertion_delay_w<0, 1>");
	map(0x10e0, 0x10e1).               rw(&swp30_device::insertion_delay_r<0, 2>, "swp30_device::insertion_delay_r<0, 2>", &swp30_device::insertion_delay_w<0, 2>, "swp30_device::insertion_delay_w<0, 2>");
	map(0x10e2, 0x10e3).               rw(&swp30_device::insertion_delay_r<0, 3>, "swp30_device::insertion_delay_r<0, 3>", &swp30_device::insertion_delay_w<0, 3>, "swp30_device::insertion_delay_w<0, 3>");
	map(0x1152, 0x1153).               rw(&swp30_device::insertion_inv_high_damp_r<0, 0>, "swp30_device::insertion_inv_high_damp_r<0, 0>", &swp30_device::insertion_inv_high_damp_w<0, 0>, "swp30_device::insertion_inv_high_damp_w<0, 0>");
	map(0x1156, 0x1157).               rw(&swp30_device::insertion_high_damp_r<0, 0>, "swp30_device::insertion_high_damp_r<0, 0>", &swp30_device::insertion_high_damp_w<0, 0>, "swp30_device::insertion_high_damp_w<0, 0>");
	map(0x11ca, 0x11cb).               rw(&swp30_device::insertion_inv_high_damp_r<0, 1>, "swp30_device::insertion_inv_high_damp_r<0, 1>", &swp30_device::insertion_inv_high_damp_w<0, 1>, "swp30_device::insertion_inv_high_damp_w<0, 1>");
	map(0x11ce, 0x11cf).               rw(&swp30_device::insertion_high_damp_r<0, 1>, "swp30_device::insertion_high_damp_r<0, 1>", &swp30_device::insertion_high_damp_w<0, 1>, "swp30_device::insertion_high_damp_w<0, 1>");
	map(0x1242, 0x1243).               rw(&swp30_device::insertion_fb_level_r<0, 0>, "swp30_device::insertion_fb_level_r<0, 0>", &swp30_device::insertion_fb_level_w<0, 0>, "swp30_device::insertion_fb_level_w<0, 0>");
	map(0x1246, 0x1247).               rw(&swp30_device::insertion_fb_level_r<0, 1>, "swp30_device::insertion_fb_level_r<0, 1>", &swp30_device::insertion_fb_level_w<0, 1>, "swp30_device::insertion_fb_level_w<0, 1>");
	map(0x124e, 0x124f).               rw(&swp30_device::insertion_fb_level_r<0, 2>, "swp30_device::insertion_fb_level_r<0, 2>", &swp30_device::insertion_fb_level_w<0, 2>, "swp30_device::insertion_fb_level_w<0, 2>");
	map(0x12d6, 0x12d7).               rw(&swp30_device::insertion_final_level_r<0, 0>, "swp30_device::insertion_final_level_r<0, 0>", &swp30_device::insertion_final_level_w<0, 0>, "swp30_device::insertion_final_level_w<0, 0>");
	map(0x1342, 0x1343).               rw(&swp30_device::insertion_final_level_r<0, 1>, "swp30_device::insertion_final_level_r<0, 1>", &swp30_device::insertion_final_level_w<0, 1>, "swp30_device::insertion_final_level_w<0, 1>");
	map(0x1346, 0x1347).               rw(&swp30_device::insertion_final_level_r<0, 2>, "swp30_device::insertion_final_level_r<0, 2>", &swp30_device::insertion_final_level_w<0, 2>, "swp30_device::insertion_final_level_w<0, 2>");
	map(0x134a, 0x134b).               rw(&swp30_device::insertion_final_level_r<0, 3>, "swp30_device::insertion_final_level_r<0, 3>", &swp30_device::insertion_final_level_w<0, 3>, "swp30_device::insertion_final_level_w<0, 3>");

	map(0x1460, 0x1461).               rw(&swp30_device::insertion_delay_r<1, 0>, "swp30_device::insertion_delay_r<1, 0>", &swp30_device::insertion_delay_w<1, 0>, "swp30_device::insertion_delay_w<1, 0>");
	map(0x1462, 0x1463).               rw(&swp30_device::insertion_delay_r<1, 1>, "swp30_device::insertion_delay_r<1, 1>", &swp30_device::insertion_delay_w<1, 1>, "swp30_device::insertion_delay_w<1, 1>");
	map(0x14e0, 0x14e1).               rw(&swp30_device::insertion_delay_r<1, 2>, "swp30_device::insertion_delay_r<1, 2>", &swp30_device::insertion_delay_w<1, 2>, "swp30_device::insertion_delay_w<1, 2>");
	map(0x14e2, 0x14e3).               rw(&swp30_device::insertion_delay_r<1, 3>, "swp30_device::insertion_delay_r<1, 3>", &swp30_device::insertion_delay_w<1, 3>, "swp30_device::insertion_delay_w<1, 3>");
	map(0x1552, 0x1553).               rw(&swp30_device::insertion_inv_high_damp_r<1, 0>, "swp30_device::insertion_inv_high_damp_r<1, 0>", &swp30_device::insertion_inv_high_damp_w<1, 0>, "swp30_device::insertion_inv_high_damp_w<1, 0>");
	map(0x1556, 0x1557).               rw(&swp30_device::insertion_high_damp_r<1, 0>, "swp30_device::insertion_high_damp_r<1, 0>", &swp30_device::insertion_high_damp_w<1, 0>, "swp30_device::insertion_high_damp_w<1, 0>");
	map(0x15ca, 0x15cb).               rw(&swp30_device::insertion_inv_high_damp_r<1, 1>, "swp30_device::insertion_inv_high_damp_r<1, 1>", &swp30_device::insertion_inv_high_damp_w<1, 1>, "swp30_device::insertion_inv_high_damp_w<1, 1>");
	map(0x15ce, 0x15cf).               rw(&swp30_device::insertion_high_damp_r<1, 1>, "swp30_device::insertion_high_damp_r<1, 1>", &swp30_device::insertion_high_damp_w<1, 1>, "swp30_device::insertion_high_damp_w<1, 1>");
	map(0x1642, 0x1643).               rw(&swp30_device::insertion_fb_level_r<1, 0>, "swp30_device::insertion_fb_level_r<1, 0>", &swp30_device::insertion_fb_level_w<1, 0>, "swp30_device::insertion_fb_level_w<1, 0>");
	map(0x1646, 0x1647).               rw(&swp30_device::insertion_fb_level_r<1, 1>, "swp30_device::insertion_fb_level_r<1, 1>", &swp30_device::insertion_fb_level_w<1, 1>, "swp30_device::insertion_fb_level_w<1, 1>");
	map(0x164e, 0x164f).               rw(&swp30_device::insertion_fb_level_r<1, 2>, "swp30_device::insertion_fb_level_r<1, 2>", &swp30_device::insertion_fb_level_w<1, 2>, "swp30_device::insertion_fb_level_w<1, 2>");
	map(0x16d6, 0x16d7).               rw(&swp30_device::insertion_final_level_r<1, 0>, "swp30_device::insertion_final_level_r<1, 0>", &swp30_device::insertion_final_level_w<1, 0>, "swp30_device::insertion_final_level_w<1, 0>");
	map(0x1742, 0x1743).               rw(&swp30_device::insertion_final_level_r<1, 1>, "swp30_device::insertion_final_level_r<1, 1>", &swp30_device::insertion_final_level_w<1, 1>, "swp30_device::insertion_final_level_w<1, 1>");
	map(0x1746, 0x1747).               rw(&swp30_device::insertion_final_level_r<1, 2>, "swp30_device::insertion_final_level_r<1, 2>", &swp30_device::insertion_final_level_w<1, 2>, "swp30_device::insertion_final_level_w<1, 2>");
	map(0x174a, 0x174b).               rw(&swp30_device::insertion_final_level_r<1, 3>, "swp30_device::insertion_final_level_r<1, 3>", &swp30_device::insertion_final_level_w<1, 3>, "swp30_device::insertion_final_level_w<1, 3>");

	map(0x18e0, 0x18e1).               rw(FUNC(swp30_device::variation_delay_r<0>), FUNC(swp30_device::variation_delay_w<0>));
	map(0x1960, 0x1961).               rw(FUNC(swp30_device::variation_delay_r<1>), FUNC(swp30_device::variation_delay_w<1>));
	map(0x19e0, 0x19e1).               rw(FUNC(swp30_device::variation_delay_r<2>), FUNC(swp30_device::variation_delay_w<2>));
	map(0x1ae0, 0x1ae1).               rw(FUNC(swp30_device::variation_delay_r<3>), FUNC(swp30_device::variation_delay_w<3>));
	map(0x1bd6, 0x1bd7).               rw(FUNC(swp30_device::variation_inv_high_damp_r<0>), FUNC(swp30_device::variation_inv_high_damp_w<0>));
	map(0x1c46, 0x1c47).               rw(FUNC(swp30_device::variation_high_damp_r<0>), FUNC(swp30_device::variation_high_damp_w<0>));
	map(0x1c52, 0x1c53).               rw(FUNC(swp30_device::variation_inv_high_damp_r<1>), FUNC(swp30_device::variation_inv_high_damp_w<1>));
	map(0x1cc2, 0x1cc3).               rw(FUNC(swp30_device::variation_high_damp_r<1>), FUNC(swp30_device::variation_high_damp_w<1>));
	map(0x1cce, 0x1ccf).               rw(FUNC(swp30_device::variation_fb_level_r<0>), FUNC(swp30_device::variation_fb_level_w<0>));
	map(0x1cd2, 0x1cd3).               rw(FUNC(swp30_device::variation_fb_level_r<1>), FUNC(swp30_device::variation_fb_level_w<1>));
	map(0x1d46, 0x1d47).               rw(FUNC(swp30_device::variation_fb_level_r<2>), FUNC(swp30_device::variation_fb_level_w<2>));
	map(0x1f4a, 0x1f4b).               rw(FUNC(swp30_device::variation_final_level_r<0>), FUNC(swp30_device::variation_final_level_w<0>));
	map(0x1f52, 0x1f53).               rw(FUNC(swp30_device::variation_final_level_r<1>), FUNC(swp30_device::variation_final_level_w<1>));
	map(0x1f56, 0x1f57).               rw(FUNC(swp30_device::variation_final_level_r<2>), FUNC(swp30_device::variation_final_level_w<2>));
	map(0x1fc6, 0x1fc7).               rw(FUNC(swp30_device::variation_final_level_r<3>), FUNC(swp30_device::variation_final_level_w<3>));

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

static int ncount = 0;
void swp30_device::keyon_w(u16)
{
	m_stream->update();
	for(int i=0; i<64; i++) {
		u64 mask = u64(1) << i;
		if((m_keyon_mask & mask) && !(m_active_mask & mask) && !(m_volume[i] & 0x8000)) {
			m_sample_pos[i] = -s32(m_pre_size[i] << 8);
			logerror("sample count %3d %02x %08x %08x %08x vol %04x pan %04x\n", ncount, i, m_pre_size[i], m_post_size[i], m_address[i], m_volume[i], m_pan[i]);
			m_active_mask |= mask;
			ncount++;
		}
	}
	m_keyon_mask = 0;
}

u16 swp30_device::lpf_cutoff_r(offs_t offset)
{
	return m_lpf_cutoff[offset >> 6];
}

void swp30_device::lpf_cutoff_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_lpf_cutoff[chan] != data)
		logerror("chan %02x lpf cutoff %04x\n", chan, data);
	m_lpf_cutoff[chan] = data;
}

u16 swp30_device::hpf_cutoff_r(offs_t offset)
{
	return m_hpf_cutoff[offset >> 6];
}

void swp30_device::hpf_cutoff_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_hpf_cutoff[chan] != data)
		logerror("chan %02x hpf cutoff %04x\n", chan, data);
	m_hpf_cutoff[chan] = data;
}

u16 swp30_device::lpf_reso_r(offs_t offset)
{
	return m_lpf_reso[offset >> 6];
}

void swp30_device::lpf_reso_w(offs_t offset, u16 data)
{
	m_stream->update();
	u8 chan = offset >> 6;
	if(m_lpf_reso[chan] != data)
		logerror("chan %02x lpf resonance %04x\n", chan, data);
	m_lpf_reso[chan] = data;
}

template<int filter, int coef> u16 swp30_device::eq_filter_r(offs_t offset)
{
	return m_eq_filter[offset >> 6][filter][coef];
}

template<int filter, int coef> void swp30_device::eq_filter_w(offs_t offset, u16 data)
{
	m_stream->update();
	m_eq_filter[offset >> 6][filter][coef] = data;
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

template<int id> void swp30_device::variation_delay_w(u16 data)
{
	m_variation_delay[id] = data;
	logerror("variation delay %d: %04x (%fms)\n", id, data, data/44.100);
}

template<int id> u16  swp30_device::variation_delay_r()
{
	return m_variation_delay[id];
}

template<int id> void swp30_device::variation_fb_level_w(u16 data)
{
	m_variation_fb_level[id] = data;
	logerror("variation feedback level %d: %04x (%+f)\n", id, data, s16(data)/500.0);
}

template<int id> u16  swp30_device::variation_fb_level_r()
{
	return m_variation_fb_level[id];
}

template<int id> void swp30_device::variation_final_level_w(u16 data)
{
	m_variation_final_level[id] = data;
	logerror("variation final level %s %s: %04x (%f)\n", id & 1 ? "wet" : "dry", id & 2 ? "left" : "right", data, data/16383.0);
}

template<int id> u16  swp30_device::variation_final_level_r()
{
	return m_variation_final_level[id];
}

template<int id> void swp30_device::variation_high_damp_w(u16 data)
{
	m_variation_high_damp[id] = data;
	logerror("variation high damp %d: %04x (%f)\n", id, data, 0.1+double(data-0x2000)/0x2000*0.9);
}

template<int id> u16  swp30_device::variation_high_damp_r()
{
	return m_variation_high_damp[id];
}

template<int id> void swp30_device::variation_inv_high_damp_w(u16 data)
{
	m_variation_inv_high_damp[id] = data;
	logerror("variation inverse high damp %d: %04x (%f)\n", id, data, 0.1+double(0x2000-data)/0x2000*0.9);
}

template<int id> u16  swp30_device::variation_inv_high_damp_r()
{
	return m_variation_inv_high_damp[id];
}


template<int ins, int id> void swp30_device::insertion_delay_w(u16 data)
{
	m_insertion_delay[ins][id] = data;
	logerror("insertion %d delay %d: %04x (%fms)\n", ins, id, data, data/44.100);
}

template<int ins, int id> u16  swp30_device::insertion_delay_r()
{
	return m_insertion_delay[ins][id];
}

template<int ins, int id> void swp30_device::insertion_fb_level_w(u16 data)
{
	m_insertion_fb_level[ins][id] = data;
	logerror("insertion %d feedback level %d: %04x (%+f)\n", ins, id, data, s16(data)/500.0);
}

template<int ins, int id> u16  swp30_device::insertion_fb_level_r()
{
	return m_insertion_fb_level[ins][id];
}

template<int ins, int id> void swp30_device::insertion_final_level_w(u16 data)
{
	m_insertion_final_level[ins][id] = data;
	logerror("insertion %d final level %s %s: %04x (%f)\n", ins, id & 1 ? "wet" : "dry", id & 2 ? "left" : "right", data, data/8191.0);
}

template<int ins, int id> u16  swp30_device::insertion_final_level_r()
{
	return m_insertion_final_level[ins][id];
}

template<int ins, int id> void swp30_device::insertion_high_damp_w(u16 data)
{
	m_insertion_high_damp[ins][id] = data;
	logerror("insertion %d high damp %d: %04x (%f)\n", ins, id, data, 0.1+double(data-0x4000)/0x4000*0.9);
}

template<int ins, int id> u16  swp30_device::insertion_high_damp_r()
{
	return m_insertion_high_damp[ins][id];
}

template<int ins, int id> void swp30_device::insertion_inv_high_damp_w(u16 data)
{
	m_insertion_inv_high_damp[ins][id] = data;
	logerror("insertion %s inverse high damp %d: %04x (%f)\n", ins, id, data, 0.1+double(0x4000-data)/0x4000*0.9);
}

template<int ins, int id> u16  swp30_device::insertion_inv_high_damp_r()
{
	return m_insertion_inv_high_damp[ins][id];
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
	if(offset == 0x4e)
		return;
	int chan = (offset >> 6) & 0x3f;
	int slot = offset & 0x3f;
	std::string preg = "-";
	if(slot >= 0x21 && slot <= 0x2b && (slot & 1))
		preg = util::string_format("%03x", (slot-0x21)/2 + 6*chan);
	logerror("snd_w [%04x %04x - %-4s] %02x.%02x, %04x\n", offset, offset*2, preg, chan, slot, data);
}

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
				// - missing lpf_cutoff, lpf_reso, hpf_cutoff

				// - eq lowpass
				s32 samp1 = (samp  * m_eq_filter[channel][0][2] + m_sample_history[channel][0][0] * m_eq_filter[channel][0][1] + m_sample_history[channel][0][1] * m_eq_filter[channel][0][0]) >> 13;
				m_sample_history[channel][0][1] = m_sample_history[channel][0][0];
				m_sample_history[channel][0][0] = samp;

				// - eq highpass
				s32 samp2 = (samp1 * m_eq_filter[channel][1][2] + m_sample_history[channel][1][0] * m_eq_filter[channel][1][1] + m_sample_history[channel][1][1] * m_eq_filter[channel][1][0]) >> 13;
				m_sample_history[channel][1][1] = m_sample_history[channel][1][0];
				m_sample_history[channel][1][0] = samp1;

				// - anything else?

				// Fourth, volume (disabled) and pan, clamp the attenuation at -96dB
				s32 sampl = samp2 * m_linear_attenuation[std::min(0xff, (m_volume[channel] & 0x00) + (m_pan[channel] >> 8))];
				s32 sampr = samp2 * m_linear_attenuation[std::min(0xff, (m_volume[channel] & 0x00) + (m_pan[channel] & 0xff))];

				// Fifth, add to the accumulators
				acc_left  += sampl;
				acc_right += sampr;

				// Missing: reverb, chorus, effects in general
			}

		// Samples are 16 bits, there are up to 64 of them, and the accumulators are fixed-point signed 48.16
		// Global EQ is missing

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
	}
}
