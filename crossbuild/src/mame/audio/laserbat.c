#include "driver.h"
#include "sound/sn76477.h"
#include "sound/tms3615.h"

static int csound1;
static int ksound1, ksound2, ksound3;
static int degr, filt, a, us, bit14;

WRITE8_HANDLER( laserbat_csound1_w )
{
	csound1 = data;
}

WRITE8_HANDLER( laserbat_csound2_w )
{
	int ksound = 0;

	if (data & 0x01)
	{
		int noise_filter_res = 0, vco_res = 0;

		switch(csound1 & 0x07)
		{
		case 0x00:
			noise_filter_res = RES_K(270);
			vco_res = RES_K(47);
			break;
		case 0x01:
			noise_filter_res = RES_K(220);
			vco_res = RES_K(27);
			break;
		case 0x02:
			noise_filter_res = RES_K(150);
			vco_res = RES_K(22);
			break;
		case 0x03:
			noise_filter_res = RES_K(120);
			vco_res = RES_K(15);
			break;
		case 0x04:
			noise_filter_res = RES_K(82);
			vco_res = RES_K(12);
			break;
		case 0x05:
			noise_filter_res = RES_K(68);
			vco_res = RES_K(8.2);
			break;
		case 0x06:
			noise_filter_res = RES_K(47);
			vco_res = RES_K(6.8);
			break;
		case 0x07:
			noise_filter_res = RES_K(33);
			vco_res = RES_K(4.7);
			break;
		}

		SN76477_noise_filter_res_w(0, noise_filter_res);
		SN76477_vco_res_w(0, vco_res);

		SN76477_enable_w(0, (csound1 & 0x08) ? 1 : 0); // AB SOUND
		SN76477_mixer_b_w(0, (csound1 & 0x10) ? 1 : 0); // _VCO/NOISE

		degr = (csound1 & 0x20) ? 1 : 0;
		filt = (csound1 & 0x40) ? 1 : 0;
		a = (csound1 & 0x80) ? 1 : 0;
		us = 0; // sn76477 pin 12
	}

	SN76477_vco_w(0, (data & 0x40) ? 0 : 1);

	switch((data & 0x1c) >> 2)
	{
	case 0x00:
		SN76477_slf_res_w(0, RES_K(27));
		break;
	case 0x01:
		SN76477_slf_res_w(0, RES_K(22));
		break;
	case 0x02:
		SN76477_slf_res_w(0, RES_K(22));
		break;
	case 0x03:
		SN76477_slf_res_w(0, RES_K(12));
		break;
	case 0x04: // not connected
		break;
	case 0x05: // SL1
		ksound1 = csound1;
		break;
	case 0x06: // SL2
		ksound2 = csound1;
		break;
	case 0x07: // SL3
		ksound3 = csound1;
		break;
	}

	ksound = ((data & 0x02) << 23) + (ksound3 << 16) + (ksound2 << 8) + ksound1;

	tms3615_enable_w(0, ksound & 0x1fff);
	tms3615_enable_w(1, (ksound >> 13) << 1);

	bit14 = (data & 0x20) ? 1 : 0;

	// (data & 0x80) = reset
}
