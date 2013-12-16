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
    m_state = 0;

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





NETLIB_START(nic74107Asub)
{
    register_input("CLK", m_clk, netlist_input_t::STATE_INP_HL);
    register_output("Q", m_Q);
    register_output("QQ", m_QQ);

    m_Q.initial(0);
    m_QQ.initial(1);
}

NETLIB_START(nic74107A)
{
    register_sub(sub, "sub");

    register_subalias("CLK", sub.m_clk);
    register_input("J", m_J);
    register_input("K", m_K);
    register_input("CLRQ", m_clrQ);
    register_subalias("Q", sub.m_Q);
    register_subalias("QQ", sub.m_QQ);

    sub.m_Q.initial(0);
    sub.m_QQ.initial(1);
}

ATTR_HOT inline void NETLIB_NAME(nic74107Asub)::newstate(const netlist_sig_t state)
{
    const netlist_time delay[2] = { NLTIME_FROM_NS(40), NLTIME_FROM_NS(25) };
#if 1
    OUTLOGIC(m_Q, state, delay[state ^ 1]);
    OUTLOGIC(m_QQ, state ^ 1, delay[state]);
#else
    if (state != Q.new_Q())
    {
        Q.setToNoCheck(state, delay[1-state]);
        QQ.setToNoCheck(1-state, delay[state]);
    }
#endif
}

NETLIB_UPDATE(nic74107Asub)
{
    {
        const netlist_sig_t t = m_Q.net().new_Q();
        newstate((!t & m_Q1) | (t & m_Q2) | m_F);
        if (!m_Q1)
            m_clk.inactivate();
    }
}

NETLIB_UPDATE(nic74107A)
{
    if (INPLOGIC(m_J) & INPLOGIC(m_K))
    {
        sub.m_Q1 = 1;
        sub.m_Q2 = 0;
        sub.m_F  = 0;
    }
    else if (!INPLOGIC(m_J) & INPLOGIC(m_K))
    {
        sub.m_Q1 = 0;
        sub.m_Q2 = 0;
        sub.m_F  = 0;
    }
    else if (INPLOGIC(m_J) & !INPLOGIC(m_K))
    {
        sub.m_Q1 = 0;
        sub.m_Q2 = 0;
        sub.m_F  = 1;
    }
    else
    {
        sub.m_Q1 = 0;
        sub.m_Q2 = 1;
        sub.m_F  = 0;
        sub.m_clk.inactivate();
    }
    if (!INPLOGIC(m_clrQ))
    {
        sub.m_clk.inactivate();
        sub.newstate(0);
    }
    else if (!sub.m_Q2)
        sub.m_clk.activate_hl();
    //if (!sub.m_Q2 & INPLOGIC(m_clrQ))
    //  sub.m_clk.activate_hl();
}

NETLIB_START(nic74153)
{
    register_input("A1", m_I[0]);
    register_input("A2", m_I[1]);
    register_input("A3", m_I[2]);
    register_input("A4", m_I[3]);
    register_input("A", m_A);
    register_input("B", m_B);
    register_input("GA", m_GA);

    register_output("AY", m_AY);
}

NETLIB_UPDATE(nic74153)
{
    const netlist_time delay[2] = { NLTIME_FROM_NS(23), NLTIME_FROM_NS(18) };
    if (!INPLOGIC(m_GA))
    {
        UINT8 chan = (INPLOGIC(m_A) | (INPLOGIC(m_B)<<1));
        UINT8 t = INPLOGIC(m_I[chan]);
        OUTLOGIC(m_AY, t, delay[t] ); /* data to y only, FIXME */
    }
    else
    {
        OUTLOGIC(m_AY, 0, delay[0]);
    }
}


#define xstr(s) # s
#define ENTRY1(_nic, _name) register_device<_nic>( # _name, xstr(_nic) );
#define ENTRY(_nic, _name) ENTRY1(NETLIB_NAME(_nic), _name)

netlist_factory::netlist_factory()
{

}

netlist_factory::~netlist_factory()
{
    for (list_t::entry_t *e = m_list.first(); e != NULL; e = m_list.next(e))
    {
        net_device_t_base_factory *p = e->object();
        delete p;
    }
    m_list.reset();
}

void netlist_factory::initialize()
{
    ENTRY(R,                    NETDEV_R)
    ENTRY(POT,                  NETDEV_POT)
    ENTRY(C,                    NETDEV_C)
    ENTRY(D,                    NETDEV_D)
    ENTRY(VCVS,                 NETDEV_VCVS)
    ENTRY(VCCS,                 NETDEV_VCCS)
    ENTRY(QPNP_switch,          NETDEV_QPNP)
    ENTRY(QNPN_switch,          NETDEV_QNPN)
    ENTRY(ttl_const,            NETDEV_TTL_CONST)
    ENTRY(analog_const,         NETDEV_ANALOG_CONST)
    ENTRY(logic_input,          NETDEV_LOGIC_INPUT)
    ENTRY(analog_input,         NETDEV_ANALOG_INPUT)
    ENTRY(log,                  NETDEV_LOG)
    ENTRY(logD,                 NETDEV_LOGD)
    ENTRY(clock,                NETDEV_CLOCK)
    ENTRY(mainclock,            NETDEV_MAINCLOCK)
    ENTRY(solver,               NETDEV_SOLVER)
    ENTRY(nicMultiSwitch,       NETDEV_SWITCH2)
    ENTRY(nicRSFF,              NETDEV_RSFF)
    ENTRY(nicMixer8,            NETDEV_MIXER)
    ENTRY(7400,                 TTL_7400_NAND)
    ENTRY(7402,                 TTL_7402_NOR)
    ENTRY(nic7404,              TTL_7404_INVERT)
    ENTRY(7410,                 TTL_7410_NAND)
    ENTRY(7420,                 TTL_7420_NAND)
    ENTRY(7425,                 TTL_7425_NOR)
    ENTRY(7427,                 TTL_7427_NOR)
    ENTRY(7430,                 TTL_7430_NAND)
    ENTRY(nic7450,              TTL_7450_ANDORINVERT)
    ENTRY(7486,                 TTL_7486_XOR)
    ENTRY(nic7448,              TTL_7448)
    ENTRY(7474,                 TTL_7474)
    ENTRY(7483,                 TTL_7483)
    ENTRY(7490,                 TTL_7490)
    ENTRY(7493,                 TTL_7493)
    ENTRY(nic74107,             TTL_74107)
    ENTRY(nic74107A,            TTL_74107A)
    ENTRY(nic74153,             TTL_74153)
    ENTRY(9316,                 TTL_9316)
    ENTRY(NE555,                NETDEV_NE555)
    ENTRY(nicNE555N_MSTABLE,    NE555N_MSTABLE)
}

netlist_device_t *netlist_factory::new_device_by_classname(const pstring &classname, netlist_setup_t &setup) const
{
    for (list_t::entry_t *e = m_list.first(); e != NULL; e = m_list.next(e))
    {
        net_device_t_base_factory *p = e->object();
        if (strcmp(p->classname(), classname) == 0)
        {
            netlist_device_t *ret = p->Create();
            return ret;
        }
        p++;
    }
    setup.netlist().xfatalerror("Class %s not found!\n", classname.cstr());
    return NULL; // appease code analysis
}

netlist_device_t *netlist_factory::new_device_by_name(const pstring &name, netlist_setup_t &setup) const
{
    for (list_t::entry_t *e = m_list.first(); e != NULL; e = m_list.next(e))
    {
        net_device_t_base_factory *p = e->object();
        if (strcmp(p->name(), name) == 0)
        {
            netlist_device_t *ret = p->Create();
            return ret;
        }
        p++;
    }
    setup.netlist().xfatalerror("Class %s not found!\n", name.cstr());
    return NULL; // appease code analysis
}

