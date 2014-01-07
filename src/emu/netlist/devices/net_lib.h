// license:GPL-2.0+
// copyright-holders:Couriersud
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

#include "../nl_base.h"
#include "nld_signal.h"
#include "nld_system.h"

#include "nld_7400.h"
#include "nld_7402.h"
#include "nld_7404.h"
#include "nld_7410.h"
#include "nld_7420.h"
#include "nld_7425.h"
#include "nld_7427.h"
#include "nld_7430.h"
#include "nld_7474.h"
#include "nld_7483.h"
#include "nld_7486.h"
#include "nld_7490.h"
#include "nld_7493.h"
#include "nld_74107.h"
#include "nld_74153.h"
#include "nld_9316.h"

#include "nld_ne555.h"

#include "nld_log.h"

#include "../analog/nld_bjt.h"
#include "../analog/nld_fourterm.h"
#include "../analog/nld_solver.h"
#include "../analog/nld_switches.h"
#include "../analog/nld_twoterm.h"


#include "nld_legacy.h"

// ----------------------------------------------------------------------------------------
// TTL Logic chips
// ----------------------------------------------------------------------------------------

#define TTL_7450_ANDORINVERT(_name, _I1, _I2, _I3, _I4)                             \
		NET_REGISTER_DEV(nic7450, _name)                                            \
		NET_CONNECT(_name, I1, _I1)                                                 \
		NET_CONNECT(_name, I2, _I2)                                                 \
		NET_CONNECT(_name, I3, _I3)                                                 \
		NET_CONNECT(_name, I4, _I4)

#define TTL_7448(_name, _A0, _A1, _A2, _A3, _LTQ, _BIQ, _RBIQ)                      \
		NET_REGISTER_DEV(nic7448, _name)                                            \
		NET_CONNECT(_name, A0, _A0)                                                 \
		NET_CONNECT(_name, A1, _A1)                                                 \
		NET_CONNECT(_name, A2, _A2)                                                 \
		NET_CONNECT(_name, A3, _A3)                                                 \
		NET_CONNECT(_name, LTQ, _LTQ)                                               \
		NET_CONNECT(_name, BIQ, _BIQ)                                               \
		NET_CONNECT(_name, RBIQ, _RBIQ)



// ----------------------------------------------------------------------------------------
// Standard devices ...
// ----------------------------------------------------------------------------------------

NETLIB_DEVICE(nic7450,
	netlist_ttl_input_t m_I0;
	netlist_ttl_input_t m_I1;
	netlist_ttl_input_t m_I2;
	netlist_ttl_input_t m_I3;
	netlist_ttl_output_t m_Q;
);


NETLIB_SUBDEVICE(nic7448_sub,
	ATTR_HOT void update_outputs(UINT8 v);
	static const UINT8 tab7448[16][7];

	netlist_ttl_input_t m_A0;
	netlist_ttl_input_t m_A1;
	netlist_ttl_input_t m_A2;
	netlist_ttl_input_t m_A3;
	netlist_ttl_input_t m_RBIQ;

	UINT8 m_state;

	netlist_ttl_output_t m_a;
	netlist_ttl_output_t m_b;
	netlist_ttl_output_t m_c;
	netlist_ttl_output_t m_d;
	netlist_ttl_output_t m_e;
	netlist_ttl_output_t m_f;
	netlist_ttl_output_t m_g;
);

NETLIB_DEVICE(nic7448,

	NETLIB_NAME(nic7448_sub) sub;

	netlist_ttl_input_t m_LTQ;
	netlist_ttl_input_t m_BIQ;
);

#endif
