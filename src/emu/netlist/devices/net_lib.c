// license:GPL-2.0+
// copyright-holders:Couriersud
/***************************************************************************

    netlib.c

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

#include "net_lib.h"
#include "nld_system.h"


NETLIB_START(nic7448)
{
	register_sub(sub, "sub");

	register_subalias("A0", sub.m_A0);
	register_subalias("A1", sub.m_A1);
	register_subalias("A2", sub.m_A2);
	register_subalias("A3", sub.m_A3);
	register_input("LTQ", m_LTQ);
	register_input("BIQ", m_BIQ);
	register_subalias("RBIQ",sub.m_RBIQ);

	register_subalias("a", sub.m_a);
	register_subalias("b", sub.m_b);
	register_subalias("c", sub.m_c);
	register_subalias("d", sub.m_d);
	register_subalias("e", sub.m_e);
	register_subalias("f", sub.m_f);
	register_subalias("g", sub.m_g);
}

NETLIB_RESET(nic7448)
{
    sub.do_reset();
}

NETLIB_UPDATE(nic7448)
{
	if (INPLOGIC(m_BIQ) && !INPLOGIC(m_LTQ))
	{
		sub.update_outputs(8);
	}
	else if (!INPLOGIC(m_BIQ))
	{
		sub.update_outputs(15);
	}

	if (!INPLOGIC(m_BIQ) || (INPLOGIC(m_BIQ) && !INPLOGIC(m_LTQ)))
	{
		sub.m_A0.inactivate();
		sub.m_A1.inactivate();
		sub.m_A2.inactivate();
		sub.m_A3.inactivate();
		sub.m_RBIQ.inactivate();
	} else {
		sub.m_RBIQ.activate();
		sub.m_A3.activate();
		sub.m_A2.activate();
		sub.m_A1.activate();
		sub.m_A0.activate();
		sub.update();
	}

}

NETLIB_START(nic7448_sub)
{
	register_input("A0", m_A0);
	register_input("A1", m_A1);
	register_input("A2", m_A2);
	register_input("A3", m_A3);
	register_input("RBIQ", m_RBIQ);

	register_output("a", m_a);
	register_output("b", m_b);
	register_output("c", m_c);
	register_output("d", m_d);
	register_output("e", m_e);
	register_output("f", m_f);
	register_output("g", m_g);

	save(NAME(m_state));
}

NETLIB_RESET(nic7448_sub)
{
    m_state = 0;
}

NETLIB_UPDATE(nic7448_sub)
{
	UINT8 v;

	v = (INPLOGIC(m_A0) << 0) | (INPLOGIC(m_A1) << 1) | (INPLOGIC(m_A2) << 2) | (INPLOGIC(m_A3) << 3);
	if ((!INPLOGIC(m_RBIQ) && (v==0)))
			v = 15;
	update_outputs(v);
}

NETLIB_FUNC_VOID(nic7448_sub, update_outputs, (UINT8 v))
{
	assert(v<16);
	if (v != m_state)
	{
	    // max transfer time is 100 NS */

		OUTLOGIC(m_a, tab7448[v][0], NLTIME_FROM_NS(100));
		OUTLOGIC(m_b, tab7448[v][1], NLTIME_FROM_NS(100));
		OUTLOGIC(m_c, tab7448[v][2], NLTIME_FROM_NS(100));
		OUTLOGIC(m_d, tab7448[v][3], NLTIME_FROM_NS(100));
		OUTLOGIC(m_e, tab7448[v][4], NLTIME_FROM_NS(100));
		OUTLOGIC(m_f, tab7448[v][5], NLTIME_FROM_NS(100));
		OUTLOGIC(m_g, tab7448[v][6], NLTIME_FROM_NS(100));
		m_state = v;
	}
}

const UINT8 NETLIB_NAME(nic7448_sub)::tab7448[16][7] =
{
		{   1, 1, 1, 1, 1, 1, 0 },  /* 00 - not blanked ! */
		{   0, 1, 1, 0, 0, 0, 0 },  /* 01 */
		{   1, 1, 0, 1, 1, 0, 1 },  /* 02 */
		{   1, 1, 1, 1, 0, 0, 1 },  /* 03 */
		{   0, 1, 1, 0, 0, 1, 1 },  /* 04 */
		{   1, 0, 1, 1, 0, 1, 1 },  /* 05 */
		{   0, 0, 1, 1, 1, 1, 1 },  /* 06 */
		{   1, 1, 1, 0, 0, 0, 0 },  /* 07 */
		{   1, 1, 1, 1, 1, 1, 1 },  /* 08 */
		{   1, 1, 1, 0, 0, 1, 1 },  /* 09 */
		{   0, 0, 0, 1, 1, 0, 1 },  /* 10 */
		{   0, 0, 1, 1, 0, 0, 1 },  /* 11 */
		{   0, 1, 0, 0, 0, 1, 1 },  /* 12 */
		{   1, 0, 0, 1, 0, 1, 1 },  /* 13 */
		{   0, 0, 0, 1, 1, 1, 1 },  /* 14 */
		{   0, 0, 0, 0, 0, 0, 0 },  /* 15 */
};

NETLIB_START(nic7450)
{
	register_input("I1", m_I0);
	register_input("I2", m_I1);
	register_input("I3", m_I2);
	register_input("I4", m_I3);
	register_output("Q", m_Q);
}

NETLIB_RESET(nic7450)
{
}

NETLIB_UPDATE(nic7450)
{
	m_I0.activate();
	m_I1.activate();
	m_I2.activate();
	m_I3.activate();
	UINT8 t1 = INPLOGIC(m_I0) & INPLOGIC(m_I1);
	UINT8 t2 = INPLOGIC(m_I2) & INPLOGIC(m_I3);
#if 0
	UINT8 t =  (t1 | t2) ^ 1;
	OUTLOGIC(m_Q, t, t ? NLTIME_FROM_NS(22) : NLTIME_FROM_NS(15));
#else
	const netlist_time times[2] = { NLTIME_FROM_NS(22), NLTIME_FROM_NS(15) };

	UINT8 res = 0;
	if (t1 ^ 1)
	{
		if (t2 ^ 1)
		{
			res = 1;
		}
		else
		{
			m_I0.inactivate();
			m_I1.inactivate();
		}
	} else {
		if (t2 ^ 1)
		{
			m_I2.inactivate();
			m_I3.inactivate();
		}
	}
	OUTLOGIC(m_Q, res, times[1 - res]);// ? 22000 : 15000);

#endif
}




#define xstr(s) # s
#define ENTRY1(_nic, _name, _defparam) register_device<_nic>( # _name, xstr(_nic), _defparam );
#define ENTRY(_nic, _name, _defparam) ENTRY1(NETLIB_NAME(_nic), _name, _defparam)

netlist_factory_t::netlist_factory_t()
{
}

netlist_factory_t::~netlist_factory_t()
{
	for (net_device_t_base_factory * const *e = m_list.first(); e != NULL; e = m_list.next(e))
	{
		net_device_t_base_factory *p = *e;
		delete p;
	}
	m_list.reset();
}

void netlist_factory_t::initialize()
{
	ENTRY(R,                    NETDEV_R,               "R")
	ENTRY(POT,                  NETDEV_POT,             "R")
	ENTRY(C,                    NETDEV_C,               "C")
	ENTRY(D,                    NETDEV_D,               "model")
	ENTRY(VCVS,                 NETDEV_VCVS,            "-")
	ENTRY(VCCS,                 NETDEV_VCCS,            "-")
    ENTRY(QBJT_EB,              NETDEV_QBJT_EB,         "model")
	ENTRY(QBJT_switch,          NETDEV_QBJT_SW,         "model")
	ENTRY(ttl_input,            NETDEV_TTL_INPUT,       "IN")
	ENTRY(analog_input,         NETDEV_ANALOG_INPUT,    "IN")
	ENTRY(log,                  NETDEV_LOG,             "+I")
	ENTRY(logD,                 NETDEV_LOGD,            "+I,I2")
	ENTRY(clock,                NETDEV_CLOCK,           "-")   // FIXME
	ENTRY(mainclock,            NETDEV_MAINCLOCK,       "-")   // FIXME
	ENTRY(solver,               NETDEV_SOLVER,          "-")   // FIXME
    ENTRY(gnd,                  NETDEV_GND,             "-")
	ENTRY(switch2,              NETDEV_SWITCH2,         "+i1,i2")
	ENTRY(nicRSFF,              NETDEV_RSFF,            "+S,R")
	ENTRY(7400,                 TTL_7400_NAND,          "+A,B")
	ENTRY(7402,                 TTL_7402_NOR,           "+A,B")
	ENTRY(nic7404,              TTL_7404_INVERT,        "+A")
	ENTRY(7410,                 TTL_7410_NAND,          "+A,B,C")
	ENTRY(7420,                 TTL_7420_NAND,          "+A,B,C,D")
	ENTRY(7425,                 TTL_7425_NOR,           "+A,B,C,D")
	ENTRY(7427,                 TTL_7427_NOR,           "+A,B,C")
	ENTRY(7430,                 TTL_7430_NAND,          "+A,B,C,D,E,F,G,H")
	ENTRY(nic7450,              TTL_7450_ANDORINVERT,   "+I1,I2,I3,I4")
	ENTRY(7486,                 TTL_7486_XOR,           "+A,B")
	ENTRY(nic7448,              TTL_7448,               "+A0,A1,A2,A3,LTQ,BIQ,RBIQ")
	ENTRY(7474,                 TTL_7474,               "+CLK,D,CLRQ,PREQ")
	ENTRY(7483,                 TTL_7483,               "+A1,A2,A3,A4,B1,B2,B3,B4,C0")
	ENTRY(7490,                 TTL_7490,               "+CLK,R1,R2,R91,R92")
	ENTRY(7493,                 TTL_7493,               "+CLKA,CLKB,R1,R2")
	ENTRY(nic74107,             TTL_74107,              "+CLK,J,K,CLRQ")
	ENTRY(nic74107A,            TTL_74107A,             "+CLK,J,K,CLRQ")
	ENTRY(nic74153,             TTL_74153,              "+C0,C1,C2,C3,A,B,G")
	ENTRY(9316,                 TTL_9316,               "+CLK,ENP,ENT,CLRQ,LOADQ,A,B,C,D")
	ENTRY(NE555,                NETDEV_NE555,           "-")
}

netlist_device_t *netlist_factory_t::new_device_by_classname(const pstring &classname, netlist_setup_t &setup) const
{
	for (net_device_t_base_factory * const *e = m_list.first(); e != NULL; e = m_list.next(e))
	{
		net_device_t_base_factory *p = *e;
		if (strcmp(p->classname(), classname) == 0)
		{
			netlist_device_t *ret = p->Create();
			return ret;
		}
		p++;
	}
	setup.netlist().error("Class %s not found!\n", classname.cstr());
	return NULL; // appease code analysis
}

netlist_device_t *netlist_factory_t::new_device_by_name(const pstring &name, netlist_setup_t &setup) const
{
    net_device_t_base_factory *f = factory_by_name(name, setup);
    return f->Create();
}

net_device_t_base_factory * netlist_factory_t::factory_by_name(const pstring &name, netlist_setup_t &setup) const
{
    for (net_device_t_base_factory * const *e = m_list.first(); e != NULL; e = m_list.next(e))
    {
        net_device_t_base_factory *p = *e;
        if (strcmp(p->name(), name) == 0)
        {
            return p;
        }
        p++;
    }
    setup.netlist().error("Class %s not found!\n", name.cstr());
    return NULL; // appease code analysis
}
