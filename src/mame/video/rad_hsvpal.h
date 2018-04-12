// license:BSD-3-Clause
// copyright-holders:David Haywood

// this is based on emupal.h, style etc. (including use of statics) copied where possible.  If considered generic enough could probably be merged?

#ifndef MAME_VIDEO_RAD_HSVPAL_H
#define MAME_VIDEO_RAD_HSVPAL_H

class radica_hsvpal_device : public palette_device
{
public:
	radica_hsvpal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<int _HueMask, int _SaturationMask, int _ValueMask, int _HueShift, int _SaturationShift, int _ValueShift, int _V_max>
	static rgb_t hsv_decoder(u32 raw)
	{
		int h_raw = (raw >> _HueShift) & _HueMask;
		int s_raw = (raw >> _SaturationShift) & _SaturationMask;
		int v_raw = (raw >> _ValueShift) & _ValueMask;

		// might want to pass maximum values as params for scaling
		//if (h_raw > 24)
		//  logerror("hraw >24 (%02x)\n", h_raw);

		if (v_raw > _V_max)
		{
			v_raw = _V_max;
		}

		//if (s_raw > 7)
		//  logerror("s_raw >5 (%02x)\n", sl_raw);

		double h = (double)h_raw / 24.0f;
		double s = (double)s_raw / 7.0f;
		double v = (double)v_raw / (double)_V_max;

		double r, g, b;

		if (s == 0) {
			r = g = b = v; // greyscale
		}
		else {
			double q = v < 0.5f ? v * (1 + s) : v + s - v * s;
			double p = 2 * v - q;
			r = hue2rgb(p, q, h + 1 / 3.0f);
			g = hue2rgb(p, q, h);
			b = hue2rgb(p, q, h - 1 / 3.0f);
		}

		int r_real = r * 255.0f;
		int g_real = g * 255.0f;
		int b_real = b * 255.0f;

		return rgb_t(r_real, g_real, b_real);
	};

private:
	static double hue2rgb(double p, double q, double t);
};

#define PALETTE_FORMAT_XXXVVVVVSSSHHHHH_XAVIX raw_to_rgb_converter(2, &radica_hsvpal_device::hsv_decoder<0x1f, 0x7, 0x1f, 0, 5, 8, 17>) // or is upper V bit something else?
#define PALETTE_FORMAT_XXXHHHHHVVVVVSSS_RADICA raw_to_rgb_converter(2, &radica_hsvpal_device::hsv_decoder<0x1f, 0x7, 0x1f, 8, 0, 3, 31>)


DECLARE_DEVICE_TYPE(RADICA_HSVPAL, radica_hsvpal_device)

#endif // MAME_VIDEO_RAD_HSVPAL_H
