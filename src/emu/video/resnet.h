// license:BSD-3-Clause
// copyright-holders:Couriersud
/*****************************************************************************

    resnet.h

    Compute weights for resistors networks.

*****************************************************************************/

#ifndef MAME_EMU_VIDEO_RESNET_H
#define MAME_EMU_VIDEO_RESNET_H

#pragma once

/**********************************************************************
 *      Rbias
 * Vbias >-ZZZ-.    .-----------------------------------------> Out0
 *             |    |                           Vcc
 *          R0 |    |                            |
 *   In0 >-ZZZ-+----+               Vcc          Z
 *             |    |                |           Z
 *          R1 |    |               /            Z
 *   In1 >-ZZZ-+    +----+----ZZZ--|   NPN       +------------> Out1
 *             :         |          >           <
 *             :         |           +----+----|  PNP
 *          R8 |         |           Z    |     \
 *   In8 >-ZZZ-+         |           Z    |      |
 *             |         |           Z    |     Gnd
 *             Z         |           |    '-------------------> Out2
 *             Z Rgnd    |          Gnd
 *             Z         |   |-----------------------|
 *             |         `---| max(vmin,min(sig-vcut)|--------> Out3
 *            Gnd            |-----------------------|
 *
 *********************************************************************/

/* Amplifier stage per channel but may be specified globally as default */

#define RES_NET_AMP_USE_GLOBAL      0x0000
#define RES_NET_AMP_NONE            0x0001      //Out0
#define RES_NET_AMP_DARLINGTON      0x0002      //Out1
#define RES_NET_AMP_EMITTER         0x0003      //Out2
#define RES_NET_AMP_CUSTOM          0x0004      //Out3
#define RES_NET_AMP_MASK            0x0007

/* VCC prebuilds - Global */

#define RES_NET_VCC_5V              0x0000
#define RES_NET_VCC_CUSTOM          0x0008
#define RES_NET_VCC_MASK            0x0008

/* VBias prebuilds - per channel but may be specified globally as default */

#define RES_NET_VBIAS_USE_GLOBAL    0x0000
#define RES_NET_VBIAS_5V            0x0010
#define RES_NET_VBIAS_TTL           0x0020
#define RES_NET_VBIAS_CUSTOM        0x0030
#define RES_NET_VBIAS_MASK          0x0030

/* Input Voltage levels - Global */

#define RES_NET_VIN_OPEN_COL        0x0000
#define RES_NET_VIN_VCC             0x0100
#define RES_NET_VIN_TTL_OUT         0x0200
#define RES_NET_VIN_CUSTOM          0x0300
#define RES_NET_VIN_MASK            0x0300

/* Monitor options */

// Just invert the signal
#define RES_NET_MONITOR_INVERT      0x1000
// SANYO_EZV20 / Nintendo with inverter circuit
#define RES_NET_MONITOR_SANYO_EZV20 0x2000
// Electrohome G07 Series
// 5.6k input impedance
#define RES_NET_MONITOR_ELECTROHOME_G07 0x3000

#define RES_NET_MONITOR_MASK        0x3000

/* General defines */

#define RES_NET_CHAN_RED            0x00
#define RES_NET_CHAN_GREEN          0x01
#define RES_NET_CHAN_BLUE           0x02

/* Some aliases */

#define RES_NET_VIN_MB7051          RES_NET_VIN_TTL_OUT
#define RES_NET_VIN_MB7052          RES_NET_VIN_TTL_OUT
#define RES_NET_VIN_MB7053          RES_NET_VIN_TTL_OUT
#define RES_NET_VIN_28S42           RES_NET_VIN_TTL_OUT

/* Structures */

struct res_net_channel_info {
	// per channel options
	u32     options;
	// Pullup resistor value in Ohms
	double  rBias;
	// Pulldown resistor value in Ohms
	double  rGnd;
	// Number of inputs connected to resistors
	int     num;
	// Resistor values
	// - Least significant bit first
	double  R[8];
	// Minimum output voltage
	// - Applicable if output is routed through a complementary
	// - darlington circuit
	// - typical value ~ 0.9V
	double  minout;
	// Cutoff output voltage
	// - Applicable if output is routed through 1:1 transistor amplifier
	// - Typical value ~ 0.7V
	double  cut;
	// Voltage at the pullup resistor
	// - Typical voltage ~5V
	double  vBias;
};

struct res_net_info {
	// global options
	u32     options;
	// The three color channels
	res_net_channel_info rgb[3];
	// Supply Voltage
	// - Typical value 5V
	double  vcc;
	// High Level output voltage
	// - TTL : 3.40V
	// - CMOS: 4.95V (@5v vcc)
	double  vOL;
	// Low Level output voltage
	// - TTL : 0.35V
	// - CMOS: 0.05V (@5v vcc)
	double  vOH;
	// Open Collector flag
	u8      OpenCol;
};

#define RES_NET_MAX_COMP    3

struct res_net_decode_info {
	int numcomp;
	int start;
	int end;
	u16 offset[3 * RES_NET_MAX_COMP];
	s16 shift[3 * RES_NET_MAX_COMP];
	u16 mask[3 * RES_NET_MAX_COMP];
};

/* return a single value for one channel */

int compute_res_net(int inputs, int channel, const res_net_info &di);

/* compute all values */

void compute_res_net_all(std::vector<rgb_t> &rgb, const u8 *prom, const res_net_decode_info &rdi, const res_net_info &di);


/* legacy interface */

namespace emu::detail {

template <std::size_t I, typename T, std::size_t N, typename U>
constexpr auto combine_weights(T const (&tab)[N], U w) { return tab[I] * w; }

template <std::size_t I, typename T, std::size_t N, typename U, typename... V>
constexpr auto combine_weights(T const (&tab)[N], U w0, V... w) { return (tab[I] * w0) + combine_weights<I + 1>(tab, w...); }

} // namespace emu::detail

double compute_resistor_weights(
		int minval, int maxval, double scaler,
		int count_1, const int * resistances_1, double * weights_1, int pulldown_1, int pullup_1,
		int count_2, const int * resistances_2, double * weights_2, int pulldown_2, int pullup_2,
		int count_3, const int * resistances_3, double * weights_3, int pulldown_3, int pullup_3);

template <typename T = int, typename U, std::size_t N, typename... V>
constexpr T combine_weights(U const (&tab)[N], V... w)
{
	return T(emu::detail::combine_weights<0U>(tab, w...) + 0.5);
}



/* this should be moved to one of the core files */

#define MAX_NETS 3
#define MAX_RES_PER_NET 18




/* for the open collector outputs PROMs */

double compute_resistor_net_outputs(
		int minval, int maxval, double scaler,
		int count_1, const int * resistances_1, double * outputs_1, int pulldown_1, int pullup_1,
		int count_2, const int * resistances_2, double * outputs_2, int pulldown_2, int pullup_2,
		int count_3, const int * resistances_3, double * outputs_3, int pulldown_3, int pullup_3 );



#endif /* MAME_EMU_VIDEO_RESNET_H */
