#include "driver.h"
#include "sound/sn76477.h"
#include "sound/tms3615.h"
#include "includes/laserbat.h"

WRITE8_HANDLER( laserbat_csound1_w )
{
	laserbat_state *state = (laserbat_state *)space->machine->driver_data;
	state->csound1 = data;
}

WRITE8_HANDLER( laserbat_csound2_w )
{
	laserbat_state *state = (laserbat_state *)space->machine->driver_data;
	int ksound = 0;

	if (data & 0x01)
	{
		int noise_filter_res = 0, vco_res = 0;

		switch(state->csound1 & 0x07)
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

		sn76477_noise_filter_res_w(state->sn, noise_filter_res);
		sn76477_vco_res_w(state->sn, vco_res);

		sn76477_enable_w(state->sn, (state->csound1 & 0x08) ? 1 : 0); // AB SOUND
		sn76477_mixer_b_w(state->sn, (state->csound1 & 0x10) ? 1 : 0); // _VCO/NOISE

		state->degr = (state->csound1 & 0x20) ? 1 : 0;
		state->filt = (state->csound1 & 0x40) ? 1 : 0;
		state->a = (state->csound1 & 0x80) ? 1 : 0;
		state->us = 0; // sn76477 pin 12
	}

	sn76477_vco_w(state->sn, (data & 0x40) ? 0 : 1);

	switch((data & 0x1c) >> 2)
	{
	case 0x00:
		sn76477_slf_res_w(state->sn, RES_K(27));
		break;
	case 0x01:
		sn76477_slf_res_w(state->sn, RES_K(22));
		break;
	case 0x02:
		sn76477_slf_res_w(state->sn, RES_K(22));
		break;
	case 0x03:
		sn76477_slf_res_w(state->sn, RES_K(12));
		break;
	case 0x04: // not connected
		break;
	case 0x05: // SL1
		state->ksound1 = state->csound1;
		break;
	case 0x06: // SL2
		state->ksound2 = state->csound1;
		break;
	case 0x07: // SL3
		state->ksound3 = state->csound1;
		break;
	}

	ksound = ((data & 0x02) << 23) + (state->ksound3 << 16) + (state->ksound2 << 8) + state->ksound1;

	tms3615_enable_w(state->tms1, ksound & 0x1fff);
	tms3615_enable_w(state->tms2, (ksound >> 13) << 1);

	state->bit14 = (data & 0x20) ? 1 : 0;

	// (data & 0x80) = reset
}
