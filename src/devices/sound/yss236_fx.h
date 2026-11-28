// license:BSD-3-Clause
// copyright-holders:Wouter van Nifterick

// Yamaha FS1R effects: the reverb, variation and insertion blocks and the
// master EQ, one small float model per algorithm.
//
// The VOP3 effect microprogram is not decoded. The reverb tank, the early
// reflections, the compressor and the enhancer follow Yamaha's later SWP70
// versions of those algorithms. The others are minimal models of what the
// FS1R Data List describes. Parameter words and encodings follow the Data
// List: word k of a block is its parameter k + 1.

#ifndef MAME_SOUND_YSS236_FX_H
#define MAME_SOUND_YSS236_FX_H

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace yss236_fx {

constexpr float PI = 3.14159265f;

// parameter decoding

inline float freq(int i) { return 20.0f * std::exp2(std::clamp(i, 0, 60) / 6.0f); }   // 20 Hz-20 kHz, sixth octaves
inline float db(int v) { return (v >= 52 && v <= 76) ? float(v - 64) : 0.0f; }
inline float bip(int v) { return (std::clamp(v, 1, 127) - 64) / 64.0f; }
inline float unit(int v) { return std::clamp(v, 0, 127) / 127.0f; }
inline float q(int v) { return std::max(1, v) / 10.0f; }
inline float ms(int w) { return std::clamp(w, 1, 13650) * 0.1f; }
inline float ms_lin(int v, float max) { return 0.1f + std::clamp(v, 0, 127) * (max - 0.1f) / 127.0f; }
inline float phase(int v) { return (std::clamp(v, 4, 124) - 64) / 120.0f; }
inline float out_lvl(int v) { return 4.0f * unit(v) * unit(v); }
inline float coef(float ms, float sr) { return std::exp(-1000.0f / (std::max(ms, 0.01f) * sr)); }
inline float lvl(int n) { n = std::clamp(n, 1, 64) - 1; return n * n / 3969.0f; }   // ER/Rev, Dry/Wet: 64 is both full
inline float attack_ms(int i) { static constexpr uint8_t t[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 23, 26, 30, 35, 40 }; return t[std::clamp(i, 0, 19)]; }
inline float release_ms(int i) { static constexpr uint16_t t[] = { 10, 15, 25, 35, 45, 55, 65, 75, 85, 100, 115, 140, 170, 230, 340, 680 }; return t[std::clamp(i, 0, 15)]; }
inline float ratio(int i) { static constexpr float r[] = { 1, 1.5f, 2, 3, 5, 7, 10, 20 }; return r[std::clamp(i, 0, 7)]; }

inline float ofs_ms(int i)
{
	i = std::clamp(i, 0, 127);
	return i <= 100 ? i * 0.1f : i <= 105 ? 10.0f + (i - 100) * 1.1f : 17.1f + (i - 106) * 1.5625f;
}

inline float rev_time(int i)
{
	i = std::clamp(i, 0, 69);
	return i < 48 ? 0.3f + i * 0.1f : i < 58 ? 5.5f + (i - 48) * 0.5f : i < 68 ? 11.0f + (i - 58) : i == 68 ? 25.0f : 30.0f;
}

// 48000 / 2^20 Hz per step, the step doubling at each break
inline float lfo_hz(int i)
{
	static constexpr int brk[] = { 64, 76, 88, 101, 113, 125, 127 };
	i = std::clamp(i, 0, 127);
	int n = 0;
	for (int s = 0, b = 0; i > b; b = brk[s++])
		n += (std::min(i, brk[s]) - b) << s;
	return n * (48000.0f / 1048576.0f);
}

// building blocks

struct line
{
	std::vector<float> b;
	unsigned m = 0, w = 0;

	void init(float n) { unsigned s = 2; while (s < n + 2) s <<= 1; b.assign(s, 0.0f); m = s - 1; w = 0; }
	void clear() { std::fill(b.begin(), b.end(), 0.0f); }
	void put(float x) { b[w = (w + 1) & m] = x; }
	float tap(int d) const { return b[(w - unsigned(std::clamp(d, 0, int(m)))) & m]; }
	float read(float d) const
	{
		d = std::clamp(d, 0.0f, float(m - 1));
		int const i = int(d);
		return b[(w - i) & m] + (b[(w - i - 1) & m] - b[(w - i) & m]) * (d - i);
	}
};

// (z^-d - k) / (1 - k z^-d)
inline float allpass(line &l, int d, float k, float x) { float const z = l.tap(d), v = x + k * z; l.put(v); return z - k * v; }

// Bilinear sections, w = tan(pi f / fs), two channels. First-order shelves, as in the SWP70.
struct filt
{
	enum { THRU, LP1, HP1, LS1, HS1, LP2, HP2, BP2, PEAK };
	float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, s[2][4] = { };

	void reset() { std::memset(s, 0, sizeof(s)); }
	void set(int kind, float sr, float f, float qv = 0.7f, float gdb = 0.0f)
	{
		float const w = std::tan(PI * std::min(f, 0.45f * sr) / sr), w2 = w * w, al = w / qv;
		float const g = std::pow(10.0f, gdb / 20.0f), up = std::max(g, 1.0f), dn = std::min(g, 1.0f);
		b0 = 1; b1 = b2 = a1 = a2 = 0;
		if (kind == LP1 || kind == HP1)
		{
			b0 = kind == LP1 ? w / (1 + w) : 1 / (1 + w); b1 = kind == LP1 ? b0 : -b0; a1 = (1 - w) / (1 + w);
		}
		else if (kind == LS1)
		{
			float const z = w * up, p = w / dn;
			b0 = (z + 1) / (p + 1); b1 = (z - 1) / (p + 1); a1 = (1 - p) / (p + 1);
		}
		else if (kind == HS1)
		{
			float const p = 1 / dn;
			b0 = (up + w) / (p + w); b1 = (w - up) / (p + w); a1 = (p - w) / (p + w);
		}
		else if (kind != THRU)
		{
			float const n = kind == PEAK ? al * up : al, d = kind == PEAK ? al / dn : al, D = 1 + w2 + d;
			a1 = (2 - 2 * w2) / D; a2 = (d - 1 - w2) / D;
			if (kind == LP2) { b0 = b2 = w2 / D; b1 = 2 * b0; }
			else if (kind == HP2) { b0 = b2 = 1 / D; b1 = -2 * b0; }
			else if (kind == BP2) { b0 = al / D; b2 = -b0; }
			else { b0 = (1 + w2 + n) / D; b1 = -a1; b2 = (1 + w2 - n) / D; }
		}
	}
	float run(int c, float x)
	{
		float *z = s[c], y = b0 * x + b1 * z[0] + b2 * z[1] + a1 * z[2] + a2 * z[3];
		z[1] = z[0]; z[0] = x; z[3] = z[2]; z[2] = y;
		return y;
	}
};

// unipolar, phase in cycles; the triangle starts at 0.5 rising, like the sine
struct lfo
{
	float p = 0, inc = 0;
	void set(float hz, float sr) { inc = hz / sr; }
	void step() { p += inc; p -= std::floor(p); }
	float tri(float o = 0) const { float const x = p + o + 0.25f; return 1.0f - std::fabs(2.0f * (x - std::floor(x)) - 1.0f); }
	float sine(float o = 0) const { return 0.5f + 0.5f * std::sin(2.0f * PI * (p + o)); }
};

// SWP70 compressor: peak detector with a two-stage release, (s / thr)^(1/R - 1)
// above the threshold, the gain smoothed by the attack
struct comp
{
	float a = 0, r = 0, thr = 1, ex = 0, p = 0, s = 0, g = 1, gl = 1;

	void set(float sr, float att, float rel, float thr_db, float ratio)
	{
		a = coef(att, sr); r = coef(rel / 2.2f, sr); thr = std::pow(10.0f, thr_db / 20.0f); ex = 1.0f / ratio - 1.0f;
	}
	float run(float e)
	{
		p = std::max(e, r * p);
		s = std::max(e, r * s + (1 - r) * p);
		float const t = s > thr ? std::pow(s / thr, ex) : 1.0f, out = g;
		g = a * g + (1 - a) * 0.5f * (t + gl);
		gl = t;
		return out;
	}
};

// two taps half a window apart, crossfaded by a triangle so each is silent as it wraps
struct shifter
{
	float p = 0;
	float run(line const &l, float base, float ratio, float win)
	{
		p += 1.0f - ratio;
		p -= win * std::floor(p / win);
		float const q2 = p < 0.5f * win ? p + 0.5f * win : p - 0.5f * win, g = 1.0f - std::fabs(2.0f * p / win - 1.0f);
		return l.read(base + p) * g + l.read(base + q2) * (1.0f - g);
	}
};

// soft clip, harder as edge goes from 0 to 1, peaking at 1
inline float clip(float x, float edge)
{
	float const c = (1 - edge) / 3.0f + edge / 16.0f;
	x = std::clamp(x, -1.0f, 1.0f);
	return (x - c * x * x * x) / (1 - c);
}

// SPX reverb tank. Two predelay lanes with four early taps each. Their mono sum
// goes through two allpasses into six damped combs; each lane adds a multitap of
// every comb and leaves through two more allpasses. Times in 10 us.
struct tank
{
	struct room { uint16_t loop[6], tap[6], ap[6], k[6], et[4], eb[4], ea[4]; float shelf; };
	static constexpr room rooms[4] = {
		{ { 10363, 8890, 7474, 6564, 5112, 9306 }, { 8262, 6776, 5374, 3149, 2086, 4915 }, { 2848, 2214, 1880, 1544, 1910, 1574 },        // hall
		  { 21561, 22938, 23298, 23233, 23167, 23069 }, { 1537, 2774, 4357, 6149 }, { 9830, 8749, 8356, 6095 }, { 9830, 8749, 8356, 6095 }, 0 },
		{ { 6000, 5064, 4330, 3758, 3266, 6736 }, { 5300, 4398, 3730, 2390, 2666, 5912 }, { 1101, 1368, 481, 743, 510, 730 },             // room
		  { 19923, 18350, 20382, 20546, 19792, 20709 }, { 1500, 2774, 3325, 4357 }, { 8847, 7864, 4424, 7668 }, { 8847, 7864, 4424, 7668 }, 2 },
		{ { 8182, 7692, 7306, 6540, 5511, 8921 }, { 7294, 5746, 4522, 3156, 1630, 7919 }, { 3325, 2851, 1871, 2411, 1921, 2430 },         // stage
		  { 22643, 23331, 22151, 20349, 21922, 20283 }, { 1400, 1700, 0, 0 }, { 6291, 5112, 0, 0 }, { 6291, 0, 5505, 0 }, 0 },
		{ { }, { }, { 1101, 1368, 481, 743, 510, 730 },                                                                                   // space: combs from W/H/D
		  { 19923, 18350, 20382, 20546, 19792, 20709 }, { 1750, 2350, 3650, 4250 }, { 6252, 0, 7373, 0 }, { 6252, 6881, 0, 8356 }, 0 } };
	static constexpr uint16_t diff_ticks[11] = { 0, 4, 9, 16, 23, 34, 45, 68, 91, 181, 288 };
	static constexpr uint8_t damp_idx[11] = { 60, 46, 48, 50, 51, 52, 53, 54, 55, 58, 60 };
	static constexpr uint8_t dens_mask[5] = { 0x00, 0x28, 0x3c, 0x3e, 0x3f };

	line pre[2], ap[6], comb[6];
	filt hp, lp, shelf;
	int loop[6] = { }, mt[2][6] = { }, apd[6] = { }, er[2][4] = { }, feed = 1, mask = 0;
	float g[6] = { }, inj[6] = { }, k[6] = { }, ge[2][4] = { }, grev = 1, fb = 0, b0 = 1, a1 = 0, h[6][2] = { };

	void init(float sr) { for (int i = 0; i < 6; i++) { pre[i & 1].init(0.5f * sr); ap[i].init(0.04f * sr); comb[i].init(0.16f * sr); } }
	void clear()
	{
		for (int i = 0; i < 6; i++) { pre[i & 1].clear(); ap[i].clear(); comb[i].clear(); }
		hp.reset(); lp.reset(); shelf.reset();
		std::memset(h, 0, sizeof(h));
	}
	// v: hall, room, stage, space
	void set(int v, float sr, int const *p)
	{
		auto const smp = [sr] (float ticks) { return int(ticks * sr * 1e-5f); };
		room const &r = rooms[v];
		int const dd = smp(diff_ticks[std::clamp(p[1], 0, 10)]), init = int(ms_lin(p[2], 200.0f) * 1e-3f * sr), bal = p[12] ? p[12] : 64;
		float const t = rev_time(p[0]);
		filt d;
		d.set(filt::LP1, sr, freq(damp_idx[std::clamp(p[13], 0, 10)]));
		b0 = d.b0; a1 = d.a1;
		hp.set(p[3] ? filt::HP1 : filt::THRU, sr, freq(p[3]));
		lp.set(p[4] < 60 ? filt::LP1 : filt::THRU, sr, freq(p[4]));
		shelf.set(r.shelf ? filt::LS1 : filt::THRU, sr, 100.0f, 0.7f, r.shelf);
		feed = init + int(ms_lin(p[10], 99.3f) * 1e-3f * sr);
		mask = dens_mask[std::clamp(p[11], 0, 4)];
		fb = (std::clamp(p[14], 1, 127) - 64) / 65.5f;
		grev = lvl(bal);
		for (int i = 0; i < 6; i++)
		{
			// space: width, height, depth (0.5 m steps) feed two combs each, the second longer by Wall Vary
			float const n = std::clamp(p[5 + i / 2], 0, 104) + ((i & 1) ? std::clamp(p[8], 0, 30) : 0);
			loop[i] = v == 3 ? smp(145.0f + 72.5f * n + 0.1f * n * n) : smp(r.loop[i]);
			int const tap = v == 3 ? loop[i] / 2 : smp(r.tap[i]);
			g[i] = std::pow(10.0f, -3.0f * loop[i] / ((v == 3 && i >= 4 ? 1.2f : 1.0f) * t * sr));
			inj[i] = 0.204f * std::sqrt(1 - g[i] * g[i]);
			mt[i & 1][i] = tap;
			mt[~i & 1][i] = std::max(1, tap - dd);
			apd[i] = smp(r.ap[(dd || i < 4) ? i : i - 2]) + 1;
			k[i] = (r.k[i] & ~1) / 32768.0f;
		}
		for (int j = 0; j < 4; j++)
		{
			int const e = smp(r.et[j]) + 1 + init;
			er[0][j] = e + ((j & 1) ? dd : 0);
			er[1][j] = e + ((j & 1) ? 0 : dd);
			ge[0][j] = r.eb[j] / 65536.0f * lvl(128 - bal);
			ge[1][j] = r.ea[j] / 65536.0f * lvl(128 - bal);
		}
	}
	float stage(int i, float x) { return (mask >> i & 1) ? allpass(ap[i], apd[i], k[i], x) : x; }
	void run(float const *in, float *out)
	{
		float s = 0;
		for (int c = 0; c < 2; c++)
		{
			float const d = pre[c].tap(feed);
			pre[c].put(lp.run(c, hp.run(c, in[c])) + fb * d);
			s += 0.5f * d;
			out[c] = 0;
			for (int j = 0; j < 4; j++)
				out[c] += ge[c][j] * pre[c].tap(er[c][j]);
		}
		s = stage(1, stage(0, shelf.run(0, s)));
		for (int i = 0; i < 6; i++)
		{
			float const y = comb[i].tap(loop[i]), o = g[i] * b0 * (y + h[i][0]) + a1 * h[i][1];
			h[i][0] = y; h[i][1] = o;
			out[0] += grev * comb[i].tap(mt[0][i]);
			out[1] += grev * comb[i].tap(mt[1][i]);
			comb[i].put(o + inj[i] * s);
		}
		out[0] = 2 * stage(3, stage(2, out[0]));
		out[1] = 2 * stage(5, stage(4, out[1]));
	}
};

// Early reflections, gate and reverse. A mono feed through two allpasses into a
// line with damped feedback; eighteen taps alternate between the lanes, which
// cross-feed by the diffusion delay and leave through one allpass each. 10 us.
struct early
{
	static constexpr int16_t rows[8][36] = {
		{ 0, 255, 110, 122, 180, 93, 260, 83, 550, -95, 610, 68, 670, 63, 850, 100, 1060, -83, 1170, 70, 1410, -60, 1580, 79,       // S-Hall
		  1760, 68, 2010, 59, 2270, -62, 2640, 58, 2840, 56, 3080, -53 },
		{ 0, 255, 220, 132, 460, 108, 730, -100, 1100, 92, 1580, -88, 1970, -84, 2430, 78, 2700, -74, 3210, 70, 3700, 66, 4470, 64,  // L-Hall
		  4850, -60, 5120, 62, 5620, 60, 6030, 58, 6780, 56, 7710, 54 },
		{ 0, 255, 1050, 112, 1650, 92, 2550, -116, 3050, 78, 3750, 84, 4310, -72, 5000, -88, 5570, -64, 6250, -70, 7290, -78,        // Random, gate A
		  7980, 70, 8510, 60, 8900, 56, 9380, -60, 10220, -62, 10830, 56, 11290, 52 },
		{ 760, 16, 1690, -18, 2380, -19, 2830, -22, 3390, 24, 3950, 26, 4240, -28, 4550, -30, 4880, 34, 5220, 38, 5660, 42,         // Reverse, reverse gate B
		  5920, -48, 6100, 50, 6450, 56, 6790, 65, 7020, -93, 7270, 63, 7697, 255 },
		{ 40, 48, 60, 53, 100, 52, 140, 57, 170, 58, 200, 56, 220, 66, 300, 69, 340, 72, 380, 80, 420, 78, 500, 88, 540, 99,       // Plate
		  610, 103, 710, 123, 780, 140, 870, 255, 921, 122 },
		{ 150, 22, 450, 24, 970, -30, 1110, 38, 1380, 42, 1700, 48, 1910, -56, 2300, 68, 2510, 60, 2790, -72, 3140, 78,             // Spring
		  3400, 117, 3680, 126, 4030, 255, 4260, 114, 4530, 90, 4840, -68, 5124, -66 },
		{ 0, 255, 1050, 112, 1650, 92, 2550, -116, 3050, 78, 3750, 84, 4310, -72, 5000, -88, 5570, -64, 6250, -70, 7290, -78,        // gate B
		  7980, 80, 8510, 68, 8900, 98, 9380, -77 },
		{ 595, 13, 737, -14, 1392, -15, 1548, -17, 2064, 18, 2673, 20, 3058, -22, 3275, -24, 3756, 26, 4450, 28, 4824, 31,           // reverse gate A
		  5149, -34, 5643, 37, 6039, 41, 6450, 46, 6930, -51, 7482, 58, 8484, 255 } };
	static constexpr int16_t aps[4][8] = {   // allpass delay (samples at 44.1 kHz), k * 10000
		{ 1257, 5620, 772, 6125, 539, 6478, 715, 6245 }, { 865, 4362, 254, 4938, 381, 5487, 386, 5487 },
		{ 865, 4362, 937, 5217, 627, 6478, 631, 6478 }, { 254, 4938, 486, 4796, 309, 5217, 316, 4938 } };
	static constexpr uint8_t live[11] = { 88, 98, 108, 116, 122, 128, 138, 148, 160, 173, 188 };
	static constexpr uint8_t cross_smp[11] = { 0, 2, 4, 7, 10, 15, 20, 30, 40, 80, 127 };

	line l, ap[4], x[2];
	filt hp, lp, damp;
	int d[18] = { }, apd[4] = { }, cross = 2, mask = 0;
	float g[18] = { }, k[4] = { }, fb = 0;

	void init(float sr) { l.init(sr); x[0].init(256); x[1].init(256); for (line &a : ap) a.init(0.04f * sr); }
	void clear() { l.clear(); x[0].clear(); x[1].clear(); for (line &a : ap) a.clear(); hp.reset(); lp.reset(); damp.reset(); }
	// v: early reflection, gate, reverse gate
	void set(int v, float sr, int const *p)
	{
		static constexpr uint8_t er_ap[6] = { 3, 2, 1, 3, 1, 1 }, dens[4] = { 0, 12, 14, 15 };
		int const type = std::clamp(p[0], 0, 5), init = int(ms_lin(p[3], 200.0f) * 1e-3f * sr), diff = std::clamp(p[2], 0, 10);
		int const row = v == 0 ? type : v == 1 ? (p[0] ? 6 : 2) : (p[0] ? 3 : 7), aprow = v == 0 ? er_ap[type] : v == 1 ? 0 : 1;
		float const size = (1 + 199 * std::clamp(p[1], 0, 127) / 127.0f) * 0.1f;
		for (int i = 0; i < 18; i++)
		{
			int const tg = rows[row][2 * i + 1], idx = std::clamp(std::abs(tg) + live[std::clamp(p[10], 0, 10)] - 128, 0, 255);
			d[i] = int(rows[row][2 * i] * sr * 1e-5f * size) + init + 1;
			g[i] = (v == 1 && i >= 15) || !tg ? 0 : (tg < 0 ? -0.5f : 0.5f) * std::pow(10.0f, -3.158f * std::exp(-0.04048f * idx));
		}
		for (int i = 0; i < 4; i++)
		{
			apd[i] = int(aps[aprow][2 * ((diff || i < 3) ? i : 2)] * sr / 44100.0f);
			k[i] = aps[aprow][2 * i + 1] / 10000.0f;
		}
		mask = dens[std::clamp(p[11], 0, 3)];
		cross = cross_smp[diff] + 2;
		fb = (std::clamp(p[4], 1, 127) - 64) / 65.5f;
		hp.set(p[5] ? filt::HP1 : filt::THRU, sr, freq(p[5]));
		lp.set(p[6] < 60 ? filt::LP1 : filt::THRU, sr, freq(p[6]));
		damp.set(filt::LP1, sr, freq(tank::damp_idx[std::clamp(p[12], 0, 10)]));
	}
	float stage(int i, float v) { return (mask >> i & 1) ? allpass(ap[i], apd[i], k[i], v) : v; }
	void run(float in, float *out)
	{
		l.put(stage(1, stage(0, lp.run(0, hp.run(0, in)))) + fb * damp.run(0, l.tap(d[17])));
		float s[2] = { };
		for (int i = 0; i < 18; i++)
			s[i & 1] += g[i] * l.tap(d[i]);
		for (int c = 0; c < 2; c++)
			out[c] = x[c].tap(0) + x[c ^ 1].tap(cross);
		for (int c = 0; c < 2; c++)
			x[c].put(s[c]), out[c] = stage(2 + c, out[c]);
	}
};

// one effect block

struct fx_type { uint8_t algo, v = 0; };

struct FxBlock
{
	enum slot_t { REVERB, VARIATION, INSERTION };
	enum algo_t
	{
		THRU, TANK, ER, CHORUS, CELESTE, SYMPH, FLANGER, PHASER1, PHASER2, PITCH, DETUNE, ROTARY, ROTARY2, TREMOLO, AUTOPAN,
		AMBIENCE, WAH, WAHDRIVE, LOFI, EQ3, ENHANCER, GATE, COMP, COMPDRIVE, DRIVE, AMP, LCR, LR, ECHO, CROSS, KARAOKE
	};
	enum { OD = 1, TOUCH = 2, DLY = 4 };   // drive family variants; tank and ER variants are their rows

	static constexpr fx_type rev_types[17] = {
		{ THRU }, { TANK, 0 }, { TANK, 0 }, { TANK, 1 }, { TANK, 1 }, { TANK, 1 }, { TANK, 2 }, { TANK, 2 }, { TANK, 2 },
		{ TANK, 3 }, { TANK, 3 }, { TANK, 3 }, { TANK, 3 }, { LCR }, { LR }, { ECHO }, { CROSS } };
	static constexpr fx_type var_types[29] = {
		{ THRU }, { CHORUS }, { CELESTE }, { FLANGER }, { SYMPH }, { PHASER1 }, { PHASER2 }, { DETUNE }, { ROTARY }, { TREMOLO },
		{ AUTOPAN }, { WAH }, { WAH, TOUCH }, { EQ3 }, { ENHANCER }, { GATE }, { COMP }, { DRIVE }, { DRIVE, OD }, { AMP }, { LCR },
		{ LR }, { ECHO }, { CROSS }, { KARAOKE }, { TANK, 0 }, { TANK, 1 }, { TANK, 2 }, { TANK, 2 } };
	static constexpr fx_type ins_types[41] = {
		{ THRU }, { CHORUS }, { CELESTE }, { FLANGER }, { SYMPH }, { PHASER1 }, { PHASER2 }, { PITCH }, { DETUNE }, { ROTARY },
		{ ROTARY2 }, { TREMOLO }, { AUTOPAN }, { AMBIENCE }, { WAHDRIVE }, { WAHDRIVE, OD }, { WAHDRIVE, TOUCH },
		{ WAHDRIVE, TOUCH | OD }, { WAHDRIVE, TOUCH | DLY }, { WAHDRIVE, TOUCH | DLY | OD }, { LOFI }, { EQ3 }, { ENHANCER },
		{ GATE }, { COMP }, { COMPDRIVE }, { COMPDRIVE, DLY }, { COMPDRIVE, DLY | OD }, { DRIVE }, { DRIVE, DLY }, { DRIVE, OD },
		{ DRIVE, DLY | OD }, { AMP }, { LCR }, { LR }, { ECHO }, { CROSS }, { ER, 0 }, { ER, 0 }, { ER, 1 }, { ER, 2 } };
	// insertion types without Dry/Wet: Thru, 2WayRotary, Tremolo, Auto Pan, 3-Band EQ, HM Enhancer, Noise Gate, Compressor
	static constexpr uint64_t no_dry_wet = 0x1e01c01;

	float sr = 48000.0f, dry = 0, wet = 1, env = 0, gate = 0, pan = 0, hold[2] = { }, z[2][16] = { }, dgain = 1, dedge = 0, dout = 1;
	int slot = -1, type = -1, algo = THRU, v = 0, p[16] = { }, count = 0;
	line dl[3];
	filt f[8];   // 0-2 output EQ low/mid/high, 3 LPF, 4-5 drive EQ, 6 HPF, 7 crossover
	lfo lf[2];
	comp cp, hc;
	shifter sh[2];
	tank tk;
	early er;

	void init(float rate) { sr = rate; for (line &l : dl) l.init(1.4f * sr); tk.init(sr); er.init(sr); }
	void clear()
	{
		for (line &l : dl) l.clear();
		for (filt &x : f) x.reset();
		tk.clear(); er.clear();
		std::memset(z, 0, sizeof(z));
		env = gate = pan = hold[0] = hold[1] = 0;
		cp = hc = comp();
	}
	float smp(float ms) const { return ms * 1e-3f * sr; }
	int dsmp(int w) const { return int(smp(ms(w))); }
	void eq(int lf_, int lg, int hf, int hg) { f[0].set(filt::LS1, sr, freq(lf_), 0.7f, db(lg)); f[2].set(filt::HS1, sr, freq(hf), 0.7f, db(hg)); }
	void lpf(int i, float qv = 0.7f) { f[3].set(i < 60 ? filt::LP2 : filt::THRU, sr, freq(i), qv); }

	void configure(int s, int t, int const *words)
	{
		t = std::clamp(t, 0, s == REVERB ? 16 : s == VARIATION ? 28 : 40);
		if (s != slot || t != type)
			clear();
		fx_type const e = (s == REVERB ? rev_types : s == VARIATION ? var_types : ins_types)[t];
		slot = s; type = t; algo = e.algo; v = e.v;
		std::copy_n(words, 16, p);
		for (filt &x : f) x.set(filt::THRU, sr, 1000.0f);
		bool const ins = s == INSERTION, dw = ins && !(no_dry_wet >> t & 1);
		dry = dw ? lvl(128 - p[9]) : 0.0f;
		wet = dw ? lvl(p[9]) : 1.0f;
		lf[0].set(lfo_hz(p[0]), sr);
		lf[1].set(lfo_hz(p[0]) * 0.85f, sr);
		f[7].set(filt::LP1, sr, algo == ROTARY2 ? freq(p[10]) : 800.0f);

		switch (algo)
		{
		case TANK: tk.set(v, sr, p); break;
		case ER: er.set(v, sr, p); break;
		case DETUNE: eq(p[10], p[11], p[12], p[13]); break;
		case LCR: case LR: case ECHO: case CROSS: eq(p[12], p[13], p[14], p[15]); break;
		case COMP: cp.set(sr, attack_ms(p[0]), release_ms(p[1]), p[2] - 127.0f, ratio(p[3])); break;
		case LOFI: lpf(p[3] < 10 ? 60 : p[3], q(p[5])); break;
		case KARAOKE: f[6].set(p[2] ? filt::HP1 : filt::THRU, sr, freq(p[2])); lpf(p[3]); break;
		case EQ3:
			f[0].set(filt::LS1, sr, freq(p[5]), 0.7f, db(p[0]));
			f[1].set(filt::PEAK, sr, freq(p[1]), q(p[3]), db(p[2]));
			f[2].set(filt::HS1, sr, freq(p[6]), 0.7f, db(p[4]));
			break;
		case ENHANCER:
			f[6].set(filt::HP2, sr, freq(p[0]));
			f[5].set(filt::HP1, sr, freq(p[0]));
			hc.set(sr, 1.0f, 45.0f, -48.0f, 2.0f);
			break;
		case WAHDRIVE: case COMPDRIVE: case DRIVE: case AMP:
		{
			// word of drive, output, DS low gain, DS mid gain, LPF, edge
			static constexpr int8_t layout[5][6] = { { 10, 14, 11, 12, 13, -1 }, { 3, 4, 5, 6, -1, -1 }, { 5, 6, 7, 8, -1, -1 }, { 0, 4, 2, 7, 3, 10 }, { 0, 3, -1, -1, 2, 10 } };
			bool const dly = v & DLY;
			int8_t const *w = layout[algo == AMP ? 4 : algo == WAHDRIVE && !dly ? 0 : dly ? (algo == DRIVE ? 2 : 1) : 3];
			bool const own = w[5] >= 0 && algo != AMP;   // Distortion, Overdrive, Comp+Dist set their EQ frequencies
			dgain = std::exp2(unit(p[w[0]]) * ((v & OD) ? 5.0f : 7.0f));
			dedge = w[5] < 0 ? ((v & OD) ? 0.0f : 0.6f) : unit(p[w[5]]);
			dout = out_lvl(p[w[1]]);
			f[4].set(w[2] < 0 ? filt::THRU : filt::LS1, sr, own ? freq(p[1]) : 250.0f, 0.7f, w[2] < 0 ? 0 : db(p[w[2]]));
			f[5].set(w[3] < 0 ? filt::THRU : filt::PEAK, sr, own ? freq(p[6]) : 1600.0f, own ? q(p[8]) : 1.0f, w[3] < 0 ? 0 : db(p[w[3]]));
			f[6].set(algo == AMP && p[1] ? filt::HP1 : filt::THRU, sr, 50.0f * float(1 << std::clamp(p[1], 0, 3)));
			lpf(w[4] < 0 ? 60 : p[w[4]]);
			if (algo == WAHDRIVE && !dly)
				eq(p[5], p[6], p[7], p[8]);
			if (algo == COMPDRIVE)
				cp.set(sr, attack_ms(p[dly ? 10 : 11]), release_ms(p[dly ? 11 : 12]), p[dly ? 12 : 13] - 127.0f, ratio(p[dly ? 13 : 14]));
			break;
		}
		case ROTARY2: eq(p[5], p[6], p[7], p[8]); break;
		default:   // modulation types: EQ low/high, insertion EQ mid
			eq(p[5], p[6], p[7], p[8]);
			if (ins && (algo <= FLANGER || algo == ROTARY || algo == TREMOLO || algo == AUTOPAN))
				f[1].set(filt::PEAK, sr, freq(p[10]), q(p[12]), db(p[11]));
			break;
		}
	}

	float drive(int c, float x) { return f[3].run(c, f[5].run(c, f[4].run(c, clip(x * dgain, dedge)))) * dout; }
	float damp(int c, float x) { return z[c][11] += std::clamp(p[algo == LCR ? 6 : algo == LR ? 5 : 4], 1, 10) * 0.1f * (x - z[c][11]); }
	float delay(int c, float x, int d, int fb, int mix) { float const y = dl[c].tap(dsmp(d)); dl[c].put(x + bip(fb) * y); return x + unit(mix) * y; }
	// state variable bandpass, swept up to three octaves above the cutoff by m
	float wah(int c, float x, float m, int cut, int res)
	{
		float const fc = std::min(200.0f * std::exp2(unit(cut) * 4.0f + 3.0f * m), 0.2f * sr), g = 2.0f * std::sin(PI * fc / sr), r = 1.0f / q(res);
		z[c][12] += g * z[c][13];
		z[c][13] += g * (x - z[c][12] - r * z[c][13]);
		return 2.0f * z[c][13] * std::sqrt(r);
	}

	void process(float inl, float inr, float &outl, float &outr)
	{
		float const in[2] = { inl, inr }, m = 0.5f * (inl + inr);
		float o[2] = { inl, inr };
		switch (algo)
		{
		case THRU: if (slot != INSERTION) o[0] = o[1] = 0; break;   // a send block with no effect returns nothing
		case TANK: tk.run(in, o); break;
		case ER: er.run(m, o); break;
		case CHORUS: case CELESTE: case SYMPH:
		{
			// U5mMod: one read per side and a third, at 1.5 times the offset, shared
			float const base = smp(ofs_ms(p[algo == SYMPH ? 2 : 3])), span = p[1] * 8.0f * sr / 44100.0f, fb = algo == SYMPH ? 0 : bip(p[2]);
			lf[0].step();
			float const c = dl[0].read(1.5f * base + span * lf[0].tri(algo == CELESTE ? 0.8f : 2 / 3.0f));
			o[0] = 0.5f * (dl[0].read(base + span * lf[0].tri(algo == CELESTE ? 0.6f : 1 / 3.0f)) + c);
			o[1] = 0.5f * (dl[1].read(base + span * lf[0].tri()) + c);
			for (int i = 0; i < 2; i++) dl[i].put(in[i] + fb * o[i]);
			break;
		}
		case FLANGER:
			lf[0].step();
			for (int c = 0; c < 2; c++)
			{
				o[c] = dl[c].read(smp(ofs_ms(p[3]) + 0.1f + 4.0f * unit(p[1]) * lf[0].tri(c ? phase(p[13]) : 0)));
				dl[c].put(in[c] + bip(p[2]) * o[c]);
			}
			break;
		case PHASER1: case PHASER2:
		{
			// first-order allpasses swept over about six octaves, mono or stereo
			bool const st = algo == PHASER2;
			int const n = std::min(st ? 2 * std::clamp(p[10], 3, 6) : std::clamp(p[10], 4, 12), 12);
			lf[0].step();
			for (int c = 0; c < (st ? 2 : 1); c++)
			{
				float const fc = std::min(100.0f * std::exp2(5.0f * unit(p[2]) + 4.0f * unit(p[1]) * (lf[0].sine(c ? phase(p[12]) : 0) - 0.5f)), 0.4f * sr);
				float const w = std::tan(PI * fc / sr), a = (w - 1) / (w + 1);
				float x = (st ? in[c] : m) + 0.9f * bip(p[3]) * z[c][14];
				for (int i = 0; i < n; i++) { float const y = a * x + z[c][i]; z[c][i] = x - a * y; x = y; }
				o[c] = z[c][14] = x;
			}
			if (!st) o[1] = p[11] ? -o[0] : o[0];
			break;
		}
		case PITCH:
		{
			float const semi = std::clamp(p[0], 40, 88) - 64, base = smp(ms_lin(p[1], 400.0f)), win = 0.0286f * sr;
			float y[2];
			for (int i = 0; i < 2; i++)
				y[i] = sh[i].run(dl[0], base, std::exp2((semi + (std::clamp(p[2 + i], 14, 114) - 64) / 100.0f) / 12), win);
			dl[0].put(m + bip(p[4]) * 0.5f * (y[0] + y[1]));
			for (int c = 0; c < 2; c++)
			{
				o[c] = 0;
				for (int i = 0; i < 2; i++)
				{
					float const pan = (std::clamp(p[10 + 2 * i], 1, 127) - 1) / 126.0f;
					o[c] += 2 * y[i] * unit(p[11 + 2 * i]) * (c ? pan : 1 - pan);
				}
			}
			break;
		}
		case DETUNE:
		{
			float const r = std::exp2((std::clamp(p[0], 14, 114) - 64) / 1200.0f);
			dl[0].put(m);
			for (int c = 0; c < 2; c++) o[c] = sh[c].run(dl[0], smp(ofs_ms(p[1 + c])), c ? 1 / r : r, 0.0286f * sr);
			break;
		}
		case ROTARY: case ROTARY2:
		{
			// horn above the crossover with Doppler and AM, drum below with AM
			bool const two = algo == ROTARY2;
			float const d = two ? 0.8f : unit(p[1]), mic = two ? std::clamp(p[11], 0, 180) / 360.0f : 0.25f;
			float lo = f[7].run(0, m), hi = m - lo;
			if (two)
			{
				lo = clip(lo * (1 + 4 * unit(p[1])), 0) * lvl(128 - p[3]);
				hi = clip(hi * (1 + 4 * unit(p[2])), 0) * lvl(p[3]);
			}
			lf[0].step(); lf[1].step();
			dl[0].put(hi);
			for (int c = 0; c < 2; c++)
			{
				float const h = lf[0].sine(c * mic), r = lf[1].sine(c * mic);
				o[c] = dl[0].read(smp(0.3f + 0.6f * d * h)) * (1 - 0.6f * d * (1 - h)) + lo * (1 - 0.3f * d * (1 - r));
			}
			break;
		}
		case TREMOLO:
			lf[0].step();
			for (int c = 0; c < 2; c++)
			{
				float const s = lf[0].sine(c ? phase(p[13]) + (p[14] ? 0.5f : 0.0f) : 0);
				dl[c].put(in[c]);
				o[c] = dl[c].read(smp(unit(p[2])) * (1 + s)) * (1 - unit(p[1]) * (1 - s));
			}
			break;
		case AUTOPAN:
		{
			// equal-power pan, position from the LFO as PAN Dir says, F/R depth as level
			lf[0].step();
			float const ph = lf[0].p, s = std::sin(2 * PI * ph), fr = 1 - 0.25f * unit(p[2]) * (1 + std::cos(2 * PI * ph));
			float const pos[6] = { s, 2 * ph - 1, 1 - 2 * ph, -s, s, ph < 0.5f ? -1.0f : 1.0f };
			pan += (pos[std::clamp(p[3], 0, 5)] - pan) * 0.005f;
			for (int c = 0; c < 2; c++) o[c] = in[c] * std::sqrt(1 + (c ? 1 : -1) * pan * unit(p[1])) * fr;
			break;
		}
		case AMBIENCE:
			dl[0].put(m);
			o[0] = dl[0].tap(int(smp(ofs_ms(p[0]))));
			o[1] = p[1] ? -o[0] : o[0];
			break;
		case WAH: case WAHDRIVE:
		{
			bool const touch = v & TOUCH, dd = v & DLY;
			int const cut = dd ? p[11] : touch ? p[1] : p[2], res = dd ? p[12] : touch ? p[2] : p[3], rel = p[dd ? 13 : 15];
			float mod;
			if (touch)
			{
				env = std::max(std::fabs(m), env * coef(algo == WAH ? 100.0f : 10 + 670 * unit(rel) * unit(rel), sr));
				mod = std::min(1.0f, env * (1 + 20 * unit(p[dd ? 10 : 0])));
			}
			else
			{
				lf[0].step();
				mod = unit(p[1]) * lf[0].sine();
			}
			for (int c = 0; c < 2; c++)
			{
				o[c] = wah(c, in[c], mod, cut, res);
				if (algo == WAHDRIVE) o[c] = drive(c, o[c]);
				if (dd) o[c] = delay(c, o[c], p[0], p[1], p[2]);
			}
			break;
		}
		case LOFI:
		{
			float const step = std::min(0.25f, std::sqrt(float(std::max(1, p[1]))) * std::exp2(2.0f * std::clamp(p[6], 0, 6) - 14.0f));
			if (++count >= std::max(1, int(sr * (std::clamp(p[0], 0, 127) + 1) / 48000.0f + 0.5f)))
			{
				count = 0;
				for (int c = 0; c < 2; c++) hold[c] = std::round(in[c] / step) * step;
			}
			for (int c = 0; c < 2; c++) o[c] = f[3].run(c, hold[c]) * std::pow(10.0f, (std::clamp(p[2], 0, 42) - 6) / 20.0f);
			break;
		}
		case ENHANCER:
		{
			// SWP70: high-passed drive into an 8x + 10gx^2 shaper, a fixed compressor holding the square term level
			float x[2];
			for (int c = 0; c < 2; c++) x[c] = std::clamp(p[1] / 63.5f * f[6].run(c, 0.5f * in[c]), -1.0f, 1.0f);
			float const g = 10.0f * hc.run(std::min(1.0f, 0.5f * std::fabs(x[0] + x[1])));
			for (int c = 0; c < 2; c++) o[c] = in[c] + 4.0f * unit(p[2]) * f[5].run(c, std::clamp(8 * x[c] + g * x[c] * x[c], -1.0f, 1.0f));
			break;
		}
		case GATE:
		{
			float const kr = coef(release_ms(p[1]), sr);
			env = std::max(std::max(std::fabs(inl), std::fabs(inr)), env * kr);
			gate = env > std::pow(10.0f, (p[2] - 127) / 20.0f) ? 1 - coef(attack_ms(p[0]), sr) * (1 - gate) : gate * kr;
			for (int c = 0; c < 2; c++) o[c] = in[c] * gate * out_lvl(p[3]);
			break;
		}
		case COMP: case COMPDRIVE:
		{
			float const g = cp.run(std::fabs(m)) * (algo == COMP ? out_lvl(p[4]) : 1.0f);
			for (int c = 0; c < 2; c++)
			{
				o[c] = in[c] * g;
				if (algo == COMPDRIVE) o[c] = drive(c, o[c]);
				if (v & DLY) o[c] = delay(c, o[c], p[0], p[1], p[2]);
			}
			break;
		}
		case DRIVE: case AMP:
			if (v & DLY)
			{
				// Dist+Delay, Odrv+Delay: mono drive into L and R taps with one feedback tap
				float const y = drive(0, m), t = dl[0].tap(dsmp(p[2]));
				dl[0].put(y + bip(p[3]) * t);
				for (int c = 0; c < 2; c++) o[c] = y + unit(p[4]) * dl[0].tap(dsmp(p[c]));
			}
			else
				for (int c = 0; c < 2; c++) o[c] = drive(c, f[6].run(c, in[c]));
			break;
		case LCR:
		{
			dl[0].put(m + bip(p[4]) * damp(0, dl[0].tap(dsmp(p[3]))));
			float const c = unit(p[5]) * dl[0].tap(dsmp(p[2]));
			for (int i = 0; i < 2; i++) o[i] = dl[0].tap(dsmp(p[i])) + c;
			break;
		}
		case LR:
			for (int c = 0; c < 2; c++)
			{
				dl[c].put(in[c] + bip(p[4]) * damp(c, dl[c].tap(dsmp(p[2 + c]))));
				o[c] = dl[c].tap(dsmp(p[c]));
			}
			break;
		case ECHO:
			for (int c = 0; c < 2; c++)
			{
				float const y = dl[c].tap(dsmp(p[2 * c]));
				dl[c].put(in[c] + bip(p[2 * c + 1]) * damp(c, y));
				o[c] = y + unit(p[7]) * dl[c].tap(dsmp(p[5 + c]));
			}
			break;
		case CROSS:
		{
			// the L>R line comes out on the right, R>L on the left, feedback crossed
			float const y[2] = { dl[0].tap(dsmp(p[0])), dl[1].tap(dsmp(p[1])) };
			int const sel = std::clamp(p[3], 0, 2);
			for (int c = 0; c < 2; c++)
			{
				dl[c].put((sel == 2 || sel == c ? in[c] : 0) + bip(p[2]) * damp(c, y[c ^ 1]));
				o[c] = y[c ^ 1];
			}
			break;
		}
		case KARAOKE:
		{
			float const y = dl[0].tap(int(smp(ms_lin(p[0], 400.0f))));
			dl[0].put(f[3].run(0, f[6].run(0, m)) + bip(p[1]) * y);
			o[0] = o[1] = y;
			break;
		}
		}
		for (int c = 0; c < 2; c++)
			o[c] = dry * in[c] + wet * f[2].run(c, f[1].run(c, f[0].run(c, o[c])));
		outl = o[0];
		outr = o[1];
	}
};

// The section: 112 performance effect bytes. Reverb words 0-7 at 0x00, its bytes
// 8-15 at 0x10, variation words at 0x18, insertion words at 0x38, the types at
// 0x58, 0x5b and 0x5f, the master EQ at 0x64-0x6e.
struct FxSection
{
	FxBlock rev, var, ins;
	filt eq[3];
	float sr = 48000.0f;
	uint8_t cur[112];

	void init(float rate)
	{
		sr = rate;
		rev.init(sr); var.init(sr); ins.init(sr);
		uint8_t const off[112] = { };
		std::memset(cur, 0xff, sizeof(cur));
		configure(off);
	}
	void configure(uint8_t const *fx)
	{
		if (!std::memcmp(cur, fx, sizeof(cur)))
			return;
		std::memcpy(cur, fx, sizeof(cur));
		int w[3][16];
		for (int k = 0; k < 16; k++)
		{
			w[0][k] = k < 8 ? fx[2 * k] << 7 | fx[2 * k + 1] : fx[8 + k];
			w[1][k] = fx[0x18 + 2 * k] << 7 | fx[0x19 + 2 * k];
			w[2][k] = fx[0x38 + 2 * k] << 7 | fx[0x39 + 2 * k];
		}
		rev.configure(FxBlock::REVERB, fx[0x58], w[0]);
		var.configure(FxBlock::VARIATION, fx[0x5b], w[1]);
		ins.configure(FxBlock::INSERTION, fx[0x5f], w[2]);
		eq[0].set(fx[0x67] ? filt::PEAK : filt::LS1, sr, freq(fx[0x65]), q(fx[0x66]), db(fx[0x64]));
		eq[1].set(filt::PEAK, sr, freq(fx[0x69]), q(fx[0x6a]), db(fx[0x68]));
		eq[2].set(fx[0x6e] ? filt::PEAK : filt::HS1, sr, freq(fx[0x6c]), q(fx[0x6d]), db(fx[0x6b]));
	}
	void master(float &l, float &r) { for (filt &x : eq) { l = x.run(0, l); r = x.run(1, r); } }

	// One sample of the whole section. in: stereo dry, insertion, variation
	// send and reverb send buses. The insertion output goes to the mix by
	// its dry level and to the other blocks by its sends; the variation
	// returns and sends to the reverb; both return by level and pan.
	void process(float const *in, float &l, float &r)
	{
		// level, then balance: 64 is centre, 1 and 127 hard left and right
		auto const add = [&l, &r] (float g, int pan, float xl, float xr)
		{
			pan = std::clamp(pan, 1, 127);
			l += g * xl * std::min(1.0f, (127 - pan) / 63.0f);
			r += g * xr * std::min(1.0f, (pan - 1) / 63.0f);
		};
		float il, ir, vl, vr, rl, rr;
		ins.process(in[2], in[3], il, ir);
		float const iv = unit(cur[0x62]), irv = unit(cur[0x61]), vrv = unit(cur[0x5e]);
		var.process(in[4] + iv * il, in[5] + iv * ir, vl, vr);
		rev.process(in[6] + irv * il + vrv * vl, in[7] + irv * ir + vrv * vr, rl, rr);
		l = in[0]; r = in[1];
		add(unit(cur[0x63]), cur[0x60], il, ir);
		add(cur[0x5d] / 64.0f, cur[0x5c], vl, vr);
		add(cur[0x5a] / 64.0f, cur[0x59], rl, rr);
		master(l, r);
	}
};

} // namespace yss236_fx

#endif // MAME_SOUND_YSS236_FX_H
