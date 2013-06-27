/***************************************************************************

    net_lib.h

    Discrete netlist implementation.

****************************************************************************

    Couriersud reserves the right to license the code under a less restrictive
    license going forward.

    Copyright Nicola Salmoria and the MAME team
    All rights reserved.

    Redistribution and use of this code or any derivative works are permitted
    provided that the following conditions are met:

    * Redistributions may not be sold, nor may they be used in a commercial
    product or activity.

    * Redistributions that are modified from the original source must include the
    complete source code, including the source code for all components used by a
    binary built from the modified sources. However, as a special exception, the
    source code distributed need not include anything that is normally distributed
    (in either source or binary form) with the major components (compiler, kernel,
    and so on) of the operating system on which the executable runs, unless that
    component itself accompanies the executable.

    * Redistributions must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.


****************************************************************************/

#ifndef NET_LIB_H
#define NET_LIB_H

#include "netlist.h"

// ----------------------------------------------------------------------------------------
// Special chips
// ----------------------------------------------------------------------------------------

#define NETDEV_CLOCK(_name)                                                         \
		NET_REGISTER_DEV(netdev_clock, _name)
#define NETDEV_LOGIC_INPUT(_name)                                                   \
		NET_REGISTER_DEV(netdev_logic_input, _name)
#define NETDEV_ANALOG_INPUT(_name)                                                  \
		NET_REGISTER_DEV(netdev_analog_input, _name)
#define NETDEV_CALLBACK(_name, _IN)                                                 \
		NET_REGISTER_DEV(netdev_callback, _name)                                    \
		NET_CONNECT(_name, IN, _IN)
#define NETDEV_SWITCH2(_name, _i1, _i2)                                             \
		NET_REGISTER_DEV(nicMultiSwitch, _name)                                     \
		NET_CONNECT(_name, i1, _i1)                                                 \
		NET_CONNECT(_name, i2, _i2)
#define NETDEV_DELAY_RISE(_name, _CLK, _D)                                          \
		NET_REGISTER_DEV(netdev_delay_lh, _name)                                    \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, D, _D)
#define NETDEV_RSFF(_name, _S, _R)                                                  \
		NET_REGISTER_DEV(nicRSFF, _name)                                            \
		NET_CONNECT(_name, S, _S)                                                   \
		NET_CONNECT(_name, R, _R)

// ----------------------------------------------------------------------------------------
// TTL Logic chips
// ----------------------------------------------------------------------------------------

#define TTL_7400_NAND(_name, _I1, _I2)                                              \
		NET_REGISTER_DEV(nic7400, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)
#define TTL_7402_NOR(_name, _I1, _I2)                                               \
		NET_REGISTER_DEV(nic7402, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)
#define TTL_7404_INVERT(_name, _I1)                                                 \
		NET_REGISTER_DEV(nic7404, _name)                                            \
		NET_CONNECT(_name, I1, _I1)
#define TTL_7410_NAND(_name, _I1, _I2, _I3)                                         \
		NET_REGISTER_DEV(nic7410, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)
#define TTL_7420_NAND(_name, _I1, _I2, _I3, _I4)                                    \
		NET_REGISTER_DEV(nic7420, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)                                                 \
		NET_CONNECT(_name, I4, _I4)
#define TTL_7425_NOR(_name, _I1, _I2, _I3, _I4)                                     \
		NET_REGISTER_DEV(nic7425, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)                                                 \
		NET_CONNECT(_name, I4, _I4)
#define TTL_7427_NOR(_name, _I1, _I2, _I3)                                          \
		NET_REGISTER_DEV(nic7427, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)
#define TTL_7430_NAND(_name, _I1, _I2, _I3, _I4, _I5, _I6, _I7, _I8)                \
		NET_REGISTER_DEV(nic7430, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)                                                 \
		NET_CONNECT(_name, I4, _I4)                                                 \
		NET_CONNECT(_name, I5, _I5)                                                 \
		NET_CONNECT(_name, I6, _I6)                                                 \
		NET_CONNECT(_name, I7, _I7)                                                 \
		NET_CONNECT(_name, I8, _I8)
#define TTL_7450_ANDORINVERT(_name, _I1, _I2, _I3, _I4)                             \
		NET_REGISTER_DEV(nic7450, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)                                                 \
		NET_CONNECT(_name, I4, _I4)
#define TTL_7486_XOR(_name, _I1, _I2)                                               \
		NET_REGISTER_DEV(nic7486, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)
#define TTL_7448(_name, _A0, _A1, _A2, _A3, _LTQ, _BIQ, _RBIQ)                      \
		NET_REGISTER_DEV(nic7448, _name)                                            \
		NET_CONNECT(_name, A0, _A0)                                                 \
		NET_CONNECT(_name, A1, _A1)                                                 \
		NET_CONNECT(_name, A2, _A2)                                                 \
		NET_CONNECT(_name, A3, _A3)                                                 \
		NET_CONNECT(_name, LTQ, _LTQ)                                               \
		NET_CONNECT(_name, BIQ, _BIQ)                                               \
		NET_CONNECT(_name, RBIQ, _RBIQ)
#define TTL_7474(_name, _CLK, _D, _CLRQ, _PREQ)                                     \
		NET_REGISTER_DEV(nic7474, _name)                                            \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, D,  _D)                                                  \
		NET_CONNECT(_name, CLRQ,  _CLRQ)                                            \
		NET_CONNECT(_name, PREQ,  _PREQ)
#define TTL_7483(_name, _A1, _A2, _A3, _A4, _B1, _B2, _B3, _B4, _CI)                \
		NET_REGISTER_DEV(nic7483, _name)                                            \
		NET_CONNECT(_name, A1, _A1)                                                 \
		NET_CONNECT(_name, A2, _A2)                                                 \
		NET_CONNECT(_name, A3, _A3)                                                 \
		NET_CONNECT(_name, A4, _A4)                                                 \
		NET_CONNECT(_name, B1, _B1)                                                 \
		NET_CONNECT(_name, B2, _B2)                                                 \
		NET_CONNECT(_name, B3, _B3)                                                 \
		NET_CONNECT(_name, B4, _B4)                                                 \
		NET_CONNECT(_name, CI, _CI)
#define TTL_7490(_name, _CLK, _R1, _R2, _R91, _R92)                                 \
		NET_REGISTER_DEV(nic7490, _name)                                            \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, R1,  _R1)                                                \
		NET_CONNECT(_name, R2,  _R2)                                                \
		NET_CONNECT(_name, R91, _R91)                                               \
		NET_CONNECT(_name, R92, _R92)
#define TTL_7493(_name, _CLKA, _CLKB, _R1, _R2)                                     \
		NET_REGISTER_DEV(nic7493, _name)                                            \
		NET_CONNECT(_name, CLKA, _CLKA)                                             \
		NET_CONNECT(_name, CLKB, _CLKB)                                             \
		NET_CONNECT(_name, R1,  _R1)                                                \
		NET_CONNECT(_name, R2,  _R2)
#define TTL_74107A(_name, _CLK, _J, _K, _CLRQ)                                      \
		NET_REGISTER_DEV(nic74107A, _name)                                          \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, J,  _J)                                                  \
		NET_CONNECT(_name, K,  _K)                                                  \
		NET_CONNECT(_name, CLRQ,  _CLRQ)
#define TTL_74107(_name, _CLK, _J, _K, _CLRQ)                                       \
		TTL_74107A(_name, _CLK, _J, _K, _CLRQ)

#define TTL_74153(_name, _A1, _A2, _A3, _A4, _A, _B, _GA)                           \
		NET_REGISTER_DEV(nic74153, _name)                                           \
		NET_CONNECT(_name, A1, _A1)                                                 \
		NET_CONNECT(_name, A2, _A2)                                                 \
		NET_CONNECT(_name, A3, _A3)                                                 \
		NET_CONNECT(_name, A4, _A4)                                                 \
		NET_CONNECT(_name, A, _A)                                                   \
		NET_CONNECT(_name, B, _B)                                                   \
		NET_CONNECT(_name, GA, _GA)
#define TTL_9316(_name, _CLK, _ENP, _ENT, _CLRQ, _LOADQ, _A, _B, _C, _D)            \
		NET_REGISTER_DEV(nic9316, _name)                                            \
		NET_CONNECT(_name, CLK, _CLK)                                               \
		NET_CONNECT(_name, ENP,  _ENP)                                              \
		NET_CONNECT(_name, ENT,  _ENT)                                              \
		NET_CONNECT(_name, CLRQ, _CLRQ)                                             \
		NET_CONNECT(_name, LOADQ,_LOADQ)                                            \
		NET_CONNECT(_name, A,    _A)                                                \
		NET_CONNECT(_name, B,    _B)                                                \
		NET_CONNECT(_name, C,    _C)                                                \
		NET_CONNECT(_name, D,    _D)

#define NE555N_MSTABLE(_name, _TRIG, _CV)                                           \
		NET_REGISTER_DEV(nicNE555N_MSTABLE, _name)                                  \
		NET_CONNECT(_name, TRIG, _TRIG)                                             \
		NET_CONNECT(_name, CV, _CV)
#define NETDEV_MIXER3(_name, _I1, _I2, _I3)                                         \
		NET_REGISTER_DEV(nicMixer8, _name)                                          \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)
// ----------------------------------------------------------------------------------------
// Special support devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE(netdev_logic_input,
	ttl_output_t m_Q;
);

NETLIB_DEVICE(netdev_analog_input,
	analog_output_t m_Q;
);




// ----------------------------------------------------------------------------------------
// Special devices ...
// ----------------------------------------------------------------------------------------


NETLIB_DEVICE_WITH_PARAMS(netdev_clock,
	ttl_input_t m_feedback;
	ttl_output_t m_Q;

	net_param_t m_freq;
	netlist_time m_inc;
);

NETLIB_DEVICE_WITH_PARAMS(nicMultiSwitch,
	analog_input_t m_I[8];

	analog_output_t m_Q;
	analog_output_t m_low;

	net_param_t m_POS;

	int m_position;
);

NETLIB_DEVICE(nicRSFF,
	ttl_input_t m_S;
	ttl_input_t m_R;

	ttl_output_t m_Q;
	ttl_output_t m_QQ;
);

NETLIB_DEVICE_WITH_PARAMS(nicMixer8,
	analog_input_t m_I[8];

	analog_output_t m_Q;
	analog_output_t m_low;

	net_param_t m_R[8];

	double m_w[8];
);

// ----------------------------------------------------------------------------------------
// Standard devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE_WITH_PARAMS(nicNE555N_MSTABLE,

	//ATTR_HOT void timer_cb(INT32 timer_id);

	analog_input_t m_trigger;
	analog_input_t m_CV;
	analog_input_t m_THRESHOLD; /* internal */

	bool m_last;

	analog_output_t m_Q;
	analog_output_t m_THRESHOLD_OUT; /* internal */

	//netlist_base_timer_t *m_timer;
	net_param_t m_R;
	net_param_t m_C;
	net_param_t m_VS;
	net_param_t m_VL;
);

NETLIB_SIGNAL(nic7400, 2, 0)
NETLIB_SIGNAL(nic7402, 2, 1)
NETLIB_SIGNAL(nic7410, 3, 0)
NETLIB_SIGNAL(nic7420, 4, 0)
NETLIB_SIGNAL(nic7425, 4, 1)
NETLIB_SIGNAL(nic7427, 3, 1)
NETLIB_SIGNAL(nic7430, 8, 0)

NETLIB_DEVICE(nic7404,
	ttl_input_t m_I;
	ttl_output_t m_Q;
);

NETLIB_DEVICE(nic7450,
	ttl_input_t m_I0;
	ttl_input_t m_I1;
	ttl_input_t m_I2;
	ttl_input_t m_I3;
	ttl_output_t m_Q;
);

#if 0
NETLIB_DEVICE(nic7474,
	ttl_input_t m_clk;
	ttl_input_t m_D;
	ttl_input_t m_clrQ;
	ttl_input_t m_preQ;

	net_sig_t m_lastclk;

	ttl_output_t m_Q;
	ttl_output_t m_QQ;
);

#else
NETLIB_SUBDEVICE(nic7474sub,
	ttl_input_t m_clk;

	UINT8 m_nextD;
	ttl_output_t m_Q;
	ttl_output_t m_QQ;
);

NETLIB_DEVICE(nic7474,
	nic7474sub sub;

	ttl_input_t m_D;
	ttl_input_t m_clrQ;
	ttl_input_t m_preQ;
);
#endif

NETLIB_DEVICE(nic7486,
		ttl_input_t m_I0;
		ttl_input_t m_I1;
		ttl_output_t m_Q;
);

/* 74107 does latch data during high !
 * For modelling purposes, we assume 74107 and 74107A are the same
 */


NETLIB_SUBDEVICE(nic74107Asub,
	ttl_input_t m_clk;

	UINT8 m_Q1;
	UINT8 m_Q2;
	UINT8 m_F;

	ttl_output_t m_Q;
	ttl_output_t m_QQ;

);

NETLIB_DEVICE(nic74107A,
	nic74107Asub sub;

	ttl_input_t m_J;
	ttl_input_t m_K;
	ttl_input_t m_clrQ;

);

class nic74107 : public nic74107A
{
public:
	nic74107()
	:   nic74107A() {}

};


NETLIB_SUBDEVICE(nic7493ff,
	ttl_input_t m_I;
	ttl_output_t m_Q;
	UINT8 m_active;
);

#if 1
NETLIB_DEVICE(nic7493,
	ttl_input_t m_R1;
	ttl_input_t m_R2;

	nic7493ff A;
	nic7493ff B;
	nic7493ff C;
	nic7493ff D;

);
#else

NETLIB_DEVICE(nic7493,
	ATTR_HOT void update_outputs();

	ttl_input_t m_clk;
	ttl_input_t m_R1;
	ttl_input_t m_R2;

	ttl_output_t m_QA;
	ttl_output_t m_QB;
	ttl_output_t m_QC;
	ttl_output_t m_QD;

	UINT8 m_cnt;
);
#endif

NETLIB_DEVICE(nic7490,
	ATTR_HOT void update_outputs();

	ttl_input_t m_R1;
	ttl_input_t m_R2;
	ttl_input_t m_R91;
	ttl_input_t m_R92;
	ttl_input_t m_clk;

	UINT8 m_cnt;

	ttl_output_t m_QA;
	ttl_output_t m_QB;
	ttl_output_t m_QC;
	ttl_output_t m_QD;
);

/* ripple-carry counter on low-high clock transition */

NETLIB_SUBDEVICE(nic9316sub,
	ATTR_HOT void update_outputs_all();
	ATTR_HOT void update_outputs();

	ttl_input_t m_clk;

	ttl_input_t m_LOADQ;
	ttl_input_t m_ENT;
	ttl_input_t m_A;
	ttl_input_t m_B;
	ttl_input_t m_C;
	ttl_input_t m_D;

	UINT8 m_cnt;

	ttl_output_t m_QA;
	ttl_output_t m_QB;
	ttl_output_t m_QC;
	ttl_output_t m_QD;
	ttl_output_t m_RC;
);

NETLIB_DEVICE(nic9316,
	nic9316sub sub;
	ttl_input_t m_ENP;
	ttl_input_t m_CLRQ;
);

NETLIB_DEVICE(nic7483,
	ttl_input_t m_CI;
	ttl_input_t m_A1;
	ttl_input_t m_A2;
	ttl_input_t m_A3;
	ttl_input_t m_A4;
	ttl_input_t m_B1;
	ttl_input_t m_B2;
	ttl_input_t m_B3;
	ttl_input_t m_B4;
	ttl_input_t m_clk;

	UINT8 m_lastr;

	ttl_output_t m_SA;
	ttl_output_t m_SB;
	ttl_output_t m_SC;
	ttl_output_t m_SD;
	ttl_output_t m_CO;

);

/* one half of a nic74153 */

NETLIB_DEVICE(nic74153,
	ttl_input_t m_I[4];
	ttl_input_t m_A;
	ttl_input_t m_B;
	ttl_input_t m_GA;

	ttl_output_t m_AY;
);

NETLIB_DEVICE(nic7448,
	static const UINT8 tab7448[16][7];

	ttl_input_t m_A0;
	ttl_input_t m_A1;
	ttl_input_t m_A2;
	ttl_input_t m_A3;
	ttl_input_t m_LTQ;
	ttl_input_t m_RBIQ;
	ttl_input_t m_BIQ;

	UINT8 m_state;

	ttl_output_t m_a;
	ttl_output_t m_b;
	ttl_output_t m_c;
	ttl_output_t m_d;
	ttl_output_t m_e;
	ttl_output_t m_f;
	ttl_output_t m_g;
);



#endif
