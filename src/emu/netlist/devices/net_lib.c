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

NETLIB_START(logic_input)
{
	register_output("Q", m_Q);
}

NETLIB_UPDATE(logic_input)
{
}

NETLIB_START(analog_input)
{
	register_output("Q", m_Q);
}

NETLIB_UPDATE(analog_input)
{
}

NETLIB_START(nicMultiSwitch)
{
	static const char *sIN[8] = { "i1", "i2", "i3", "i4", "i5", "i6", "i7", "i8" };
	int i;

	m_position = 0;
	m_low.initial(0);

	for (i=0; i<8; i++)
	{
		register_input(sIN[i], m_I[i]);
		m_low.net().register_con(m_I[i]);
		//m_I[i].set_net(m_low.m_net);
	}
	register_param("POS", m_POS, 0);
	register_output("Q", m_Q);

	m_variable_input_count = true;
}

NETLIB_UPDATE(nicMultiSwitch)
{
	assert(m_position<8);
	OUTANALOG(m_Q, INPANALOG(m_I[m_position]), NLTIME_FROM_NS(1));
}

NETLIB_UPDATE_PARAM(nicMultiSwitch)
{
	m_position = m_POS.Value();
	//update();
}

NETLIB_START(nicMixer8)
{
	static const char *sI[8] = { "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8" };
	static const char *sR[8] = { "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8" };
	int i;

	m_low.initial(0);

	for (i=0; i<8; i++)
	{
		register_input(sI[i], m_I[i]);
		m_low.net().register_con(m_I[i]);
		//m_I[i].set_output(m_low);
		register_param(sR[i], m_R[i], 1e12);
	}
	register_output("Q", m_Q);

	m_variable_input_count = true;
}

NETLIB_UPDATE(nicMixer8)
{
	int i;
	double r = 0;

	for (i=0; i<8; i++)
	{
		r += m_w[i] * INPANALOG(m_I[i]);
	}
	OUTANALOG(m_Q, r, NLTIME_IMMEDIATE);
}

NETLIB_UPDATE_PARAM(nicMixer8)
{
	double t = 0;
	int i;

	for (i=0; i<8; i++)
		t += 1.0 / m_R[i].Value();
	t = 1.0 / t;

	for (i=0; i<8; i++)
		m_w[i] = t / m_R[i].Value();
}



NETLIB_START(nicRSFF)
{
	register_input("S", m_S);
	register_input("R", m_R);
	register_output("Q", m_Q);
	register_output("QQ", m_QQ);
	m_Q.initial(0);
	m_QQ.initial(1);
}

NETLIB_UPDATE(nicRSFF)
{
	if (INPLOGIC(m_S))
	{
		OUTLOGIC(m_Q,  1, NLTIME_FROM_NS(10));
		OUTLOGIC(m_QQ, 0, NLTIME_FROM_NS(10));
	}
	else if (INPLOGIC(m_R))
	{
		OUTLOGIC(m_Q,  0, NLTIME_FROM_NS(10));
		OUTLOGIC(m_QQ, 1, NLTIME_FROM_NS(10));
	}
}


NETLIB_START(nicNE555N_MSTABLE)
{
	register_input("TRIG", m_trigger);
	register_input("CV", m_CV);

	register_output("Q", m_Q);
	register_param("R", m_R, 0.0);
	register_param("C", m_C, 0.0);
	register_param("VS", m_VS, 5.0);
	register_param("VL", m_VL, 0.0 *5.0);

	m_THRESHOLD_OUT.init_object(*this, "THRESHOLD");
	register_link_internal(m_THRESHOLD, m_THRESHOLD_OUT, netlist_input_t::STATE_INP_ACTIVE);

	m_Q.initial(5.0 * 0.4);
	m_last = false;
}

inline double NETLIB_NAME(nicNE555N_MSTABLE)::nicNE555N_cv()
{
	return (m_CV.is_highz() ? 0.67 * m_VS.Value() : INPANALOG(m_CV));
}

inline double NETLIB_NAME(nicNE555N_MSTABLE)::nicNE555N_clamp(const double v, const double a, const double b)
{
	double ret = v;
	if (ret >  m_VS.Value() - a)
		ret = m_VS.Value() - a;
	if (ret < b)
		ret = b;
	return ret;
}

NETLIB_UPDATE_PARAM(nicNE555N_MSTABLE)
{
}

NETLIB_UPDATE(nicNE555N_MSTABLE)
{
	update_param(); // FIXME : m_CV should be on a sub device ...

	double vt = nicNE555N_clamp(nicNE555N_cv(), 0.7, 1.4);
	bool bthresh = (INPANALOG(m_THRESHOLD) > vt);
	bool btrig = (INPANALOG(m_trigger) > nicNE555N_clamp(nicNE555N_cv() * 0.5, 0.7, 1.4));
	bool out = m_last;

	if (!btrig)
	{
		out = true;
	}
	else if (bthresh)
	{
		out = false;
	}

	if (!m_last && out)
	{
		double vl = m_VL.Value();
		double time;

		// FIXME : m_CV should be on a sub device ...

		// TI datasheet states minimum pulse of 10 us
		if (vt<vl)
			time = 10;
		else
		{
			time = - log((m_VS.Value()-vt)/(m_VS.Value()-vl)) * m_R.Value() * m_C.Value() * 1.0e6; // in us
			if (time < 10.0)
				time = 10.0;
		}

		OUTANALOG(m_Q, m_VS.Value() * 0.7, NLTIME_FROM_NS(100));
		OUTANALOG(m_THRESHOLD_OUT, m_VS.Value(), NLTIME_FROM_US(time ));
	}
	else if (m_last && !out)
	{
		OUTANALOG(m_Q, 0.25, NLTIME_FROM_NS(100));
		OUTANALOG(m_THRESHOLD_OUT, 0.0, NLTIME_FROM_NS(1));
	}
	m_last = out;
}

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


NETLIB_START(nic7483)
{
	m_lastr = 0;

	register_input("A1", m_A1);
	register_input("A2", m_A2);
	register_input("A3", m_A3);
	register_input("A4", m_A4);
	register_input("B1", m_B1);
	register_input("B2", m_B2);
	register_input("B3", m_B3);
	register_input("B4", m_B4);
	register_input("CI", m_CI);

	register_output("SA", m_SA);
	register_output("SB", m_SB);
	register_output("SC", m_SC);
	register_output("SD", m_SD);
	register_output("CO", m_CO);
}

NETLIB_UPDATE(nic7483)
{
	UINT8 a = (INPLOGIC(m_A1) << 0) | (INPLOGIC(m_A2) << 1) | (INPLOGIC(m_A3) << 2) | (INPLOGIC(m_A4) << 3);
	UINT8 b = (INPLOGIC(m_B1) << 0) | (INPLOGIC(m_B2) << 1) | (INPLOGIC(m_B3) << 2) | (INPLOGIC(m_B4) << 3);

	UINT8 r = a + b + INPLOGIC(m_CI);

	if (r != m_lastr)
	{
		m_lastr = r;
		OUTLOGIC(m_SA, (r >> 0) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_SB, (r >> 1) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_SC, (r >> 2) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_SD, (r >> 3) & 1, NLTIME_FROM_NS(23));
		OUTLOGIC(m_CO, (r >> 4) & 1, NLTIME_FROM_NS(23));
	}
}

NETLIB_START(nic7490)
{
	m_cnt = 0;

	register_input("CLK", m_clk);
	register_input("R1",  m_R1);
	register_input("R2",  m_R2);
	register_input("R91", m_R91);
	register_input("R92", m_R92);

	register_output("QA", m_Q[0]);
	register_output("QB", m_Q[1]);
	register_output("QC", m_Q[2]);
	register_output("QD", m_Q[3]);
}

NETLIB_UPDATE(nic7490)
{
	if (INPLOGIC(m_R91) & INPLOGIC(m_R92))
	{
		m_cnt = 9;
		update_outputs();
	}
	else if (INPLOGIC(m_R1) & INPLOGIC(m_R2))
	{
		m_cnt = 0;
		update_outputs();
	}
	else if (INP_HL(m_clk))
	{
		m_cnt++;
		if (m_cnt >= 10)
			m_cnt = 0;
		update_outputs();
	}
}
#if 0
NETLIB_FUNC_VOID(nic7490, update_outputs)
{
	OUTLOGIC(m_QA, (m_cnt >> 0) & 1, NLTIME_FROM_NS(18));
	OUTLOGIC(m_QB, (m_cnt >> 1) & 1, NLTIME_FROM_NS(36));
	OUTLOGIC(m_QC, (m_cnt >> 2) & 1, NLTIME_FROM_NS(54));
	OUTLOGIC(m_QD, (m_cnt >> 3) & 1, NLTIME_FROM_NS(72));
}
#else
NETLIB_FUNC_VOID(nic7490, update_outputs, (void))
{
	const netlist_time delay[4] = { NLTIME_FROM_NS(18), NLTIME_FROM_NS(36), NLTIME_FROM_NS(54), NLTIME_FROM_NS(72) };
	for (int i=0; i<4; i++)
		OUTLOGIC(m_Q[i], (m_cnt >> i) & 1, delay[i]);
}
#endif
#if !USE_OLD7493
NETLIB_START(nic7493)
{
	register_sub(A, "A");
	register_sub(B, "B");
	register_sub(C, "C");
	register_sub(D, "D");

	register_subalias("CLKA", A.m_I);
	register_subalias("CLKB", B.m_I);
	register_input("R1",  m_R1);
	register_input("R2",  m_R2);

	register_subalias("QA", A.m_Q);
	register_subalias("QB", B.m_Q);
	register_subalias("QC", C.m_Q);
	register_subalias("QD", D.m_Q);

	register_link_internal(C, C.m_I, B.m_Q, netlist_input_t::STATE_INP_HL);
	register_link_internal(D, D.m_I, C.m_Q, netlist_input_t::STATE_INP_HL);
}

NETLIB_START(nic7493ff)
{
    register_input("CLK", m_I, netlist_input_t::STATE_INP_HL);
    register_output("Q", m_Q);
}

NETLIB_UPDATE(nic7493ff)
{
	if (m_reset == 0)
		OUTLOGIC(m_Q, !m_Q.net().new_Q(), NLTIME_FROM_NS(18));
}

NETLIB_UPDATE(nic7493)
{
	netlist_sig_t r = INPLOGIC(m_R1) & INPLOGIC(m_R2);

	if (r)
	{
		//printf("%s reset\n", name());
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 1;
		A.m_I.inactivate();
		B.m_I.inactivate();
		OUTLOGIC(A.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(B.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(C.m_Q, 0, NLTIME_FROM_NS(40));
		OUTLOGIC(D.m_Q, 0, NLTIME_FROM_NS(40));
	}
	else
	{
		A.m_reset = B.m_reset = C.m_reset = D.m_reset = 0;
		A.m_I.activate_hl();
		B.m_I.activate_hl();
		//printf("%s enable\n", name());
	}
}
#else

NETLIB_START(nic7493)
{
	m_cnt = 0;

	register_input("CLKA", m_CLK, netlist_input_t::STATE_INP_HL);
	register_input("CLKB", m_CLKB, netlist_input_t::STATE_INP_HL);
	register_input("R1",  m_R1);
	register_input("R2",  m_R2);

	register_output("QA", m_QA);
	register_output("QB", m_QB);
	register_output("QC", m_QC);
	register_output("QD", m_QD);
}

NETLIB_UPDATE(nic7493)
{
	//UINT8 old_clk = m_lastclk;
	//m_lastclk = INPLOGIC(m_clk);

	//printf("%s %d %d %d %d %d %d\n", name(), m_cnt, old_clk, m_QA.Q(), m_QB.Q(), m_QC.Q(), m_QD.Q());
	if (INPLOGIC(m_R1) & INPLOGIC(m_R2))
	{
		if (m_cnt > 0)
		{
			m_cnt = 0;
			OUTLOGIC(m_QA, 0, NLTIME_FROM_NS(40));
			OUTLOGIC(m_QB, 0, NLTIME_FROM_NS(40));
			OUTLOGIC(m_QC, 0, NLTIME_FROM_NS(40));
			OUTLOGIC(m_QD, 0, NLTIME_FROM_NS(40));
		}
		m_CLK.inactivate();
	}
	//else if (old_clk & !m_lastclk)
	else {
		m_CLK.activate_hl();
		if (INP_HL(m_CLK))
		{
			m_cnt++;
			m_cnt &= 0x0f;
			update_outputs();
		}
	}
}

NETLIB_FUNC_VOID(nic7493, update_outputs, (void))
{
	if (m_cnt & 1)
		OUTLOGIC(m_QA, 1, NLTIME_FROM_NS(16));
	else
	{
		OUTLOGIC(m_QA, 0, NLTIME_FROM_NS(16));
		switch (m_cnt)
		{
		case 0x00:
			OUTLOGIC(m_QD, 0, NLTIME_FROM_NS(70));
			OUTLOGIC(m_QC, 0, NLTIME_FROM_NS(48));
			OUTLOGIC(m_QB, 0, NLTIME_FROM_NS(34));
			break;
		case 0x02:
		case 0x06:
		case 0x0A:
		case 0x0E:
			OUTLOGIC(m_QB, 1, NLTIME_FROM_NS(34));
			break;
		case 0x04:
		case 0x0C:
			OUTLOGIC(m_QC, 1, NLTIME_FROM_NS(48));
			OUTLOGIC(m_QB, 0, NLTIME_FROM_NS(34));
			break;
		case 0x08:
			OUTLOGIC(m_QD, 1, NLTIME_FROM_NS(70));
			OUTLOGIC(m_QC, 0, NLTIME_FROM_NS(48));
			OUTLOGIC(m_QB, 0, NLTIME_FROM_NS(34));
			break;
		}
	}
}
#endif

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

NETLIB_START(nic9316)
{
	register_sub(sub, "sub");

	register_subalias("CLK", sub.m_clk);

	register_input("ENP", m_ENP);
	register_input("ENT", m_ENT);
	register_input("CLRQ", m_CLRQ);
	register_input("LOADQ", m_LOADQ);

	register_subalias("A", sub.m_A);
	register_subalias("B", sub.m_B);
	register_subalias("C", sub.m_C);
	register_subalias("D", sub.m_D);

	register_subalias("QA", sub.m_QA);
	register_subalias("QB", sub.m_QB);
	register_subalias("QC", sub.m_QC);
	register_subalias("QD", sub.m_QD);
	register_subalias("RC", sub.m_RC);

}

NETLIB_START(nic9316_sub)
{
    m_cnt = 0;
    m_loadq = 1;
    m_ent = 1;

    register_input("CLK", m_clk, netlist_input_t::STATE_INP_LH);

    register_input("A", m_A, netlist_input_t::STATE_INP_PASSIVE);
    register_input("B", m_B, netlist_input_t::STATE_INP_PASSIVE);
    register_input("C", m_C, netlist_input_t::STATE_INP_PASSIVE);
    register_input("D", m_D, netlist_input_t::STATE_INP_PASSIVE);

    register_output("QA", m_QA);
    register_output("QB", m_QB);
    register_output("QC", m_QC);
    register_output("QD", m_QD);
    register_output("RC", m_RC);

}

NETLIB_UPDATE(nic9316_sub)
{
	if (m_loadq)
	{
		m_cnt = ( m_cnt + 1) & 0x0f;
		update_outputs();
	}
	else
	{
		m_cnt = (INPLOGIC_PASSIVE(m_D) << 3) | (INPLOGIC_PASSIVE(m_C) << 2) | (INPLOGIC_PASSIVE(m_B) << 1) | (INPLOGIC_PASSIVE(m_A) << 0);
		update_outputs_all();
	}
	OUTLOGIC(m_RC, m_ent & (m_cnt == 0x0f), NLTIME_FROM_NS(20));
}

NETLIB_UPDATE(nic9316)
{
	sub.m_loadq = INPLOGIC(m_LOADQ);
	sub.m_ent = INPLOGIC(m_ENT);
	const netlist_sig_t clrq = INPLOGIC(m_CLRQ);

	if ((!sub.m_loadq || (sub.m_ent & INPLOGIC(m_ENP))) & clrq)
	{
		sub.m_clk.activate_lh();
	}
	else
	{
		sub.m_clk.inactivate();
		if (!clrq & (sub.m_cnt>0))
		{
			sub.m_cnt = 0;
			sub.update_outputs();
			OUTLOGIC(sub.m_RC, 0, NLTIME_FROM_NS(20));
			return;
		}
	}
	OUTLOGIC(sub.m_RC, sub.m_ent & (sub.m_cnt == 0x0f), NLTIME_FROM_NS(20));
}

NETLIB_FUNC_VOID(nic9316_sub, update_outputs_all, (void))
{
	const netlist_time out_delay = NLTIME_FROM_NS(20);
	OUTLOGIC(m_QA, (m_cnt >> 0) & 1, out_delay);
	OUTLOGIC(m_QB, (m_cnt >> 1) & 1, out_delay);
	OUTLOGIC(m_QC, (m_cnt >> 2) & 1, out_delay);
	OUTLOGIC(m_QD, (m_cnt >> 3) & 1, out_delay);
}

NETLIB_FUNC_VOID(nic9316_sub, update_outputs, (void))
{
	const netlist_time out_delay = NLTIME_FROM_NS(20);
#if 0
	OUTLOGIC(m_QA, (m_cnt >> 0) & 1, out_delay);
	OUTLOGIC(m_QB, (m_cnt >> 1) & 1, out_delay);
	OUTLOGIC(m_QC, (m_cnt >> 2) & 1, out_delay);
	OUTLOGIC(m_QD, (m_cnt >> 3) & 1, out_delay);
#else
	if ((m_cnt & 1) == 1)
		OUTLOGIC(m_QA, 1, out_delay);
	else
	{
		OUTLOGIC(m_QA, 0, out_delay);
		switch (m_cnt)
		{
		case 0x00:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 0, out_delay);
			OUTLOGIC(m_QD, 0, out_delay);
			break;
		case 0x02:
		case 0x06:
		case 0x0A:
		case 0x0E:
			OUTLOGIC(m_QB, 1, out_delay);
			break;
		case 0x04:
		case 0x0C:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 1, out_delay);
			break;
		case 0x08:
			OUTLOGIC(m_QB, 0, out_delay);
			OUTLOGIC(m_QC, 0, out_delay);
			OUTLOGIC(m_QD, 1, out_delay);
			break;
		}

	}
#endif
}

#define xstr(s) # s
#define ENTRY1(_nic, _name) m_list.add(new net_device_t_factory< _nic >( # _name, xstr(_nic) ));
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
    ENTRY(C,                    NETDEV_C)
    ENTRY(D,                    NETDEV_D)
    ENTRY(ttl_const,            NETDEV_TTL_CONST)
    ENTRY(analog_const,         NETDEV_ANALOG_CONST)
    ENTRY(logic_input,          NETDEV_LOGIC_INPUT)
    ENTRY(analog_input,         NETDEV_ANALOG_INPUT)
    ENTRY(log,                  NETDEV_LOG)
    ENTRY(clock,                NETDEV_CLOCK)
    ENTRY(mainclock,            NETDEV_MAINCLOCK)
    ENTRY(solver,               NETDEV_SOLVER)
    ENTRY(analog_callback,      NETDEV_CALLBACK)
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
    ENTRY(nic7483,              TTL_7483)
    ENTRY(nic7490,              TTL_7490)
    ENTRY(nic7493,              TTL_7493)
    ENTRY(nic74107,             TTL_74107)
    ENTRY(nic74107A,            TTL_74107A)
    ENTRY(nic74153,             TTL_74153)
    ENTRY(nic9316,              TTL_9316)
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
    fatalerror("Class %s not found!\n", classname.cstr());
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
    fatalerror("Class %s not found!\n", name.cstr());
    return NULL; // appease code analysis
}

