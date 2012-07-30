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


NETLIB_START(netdev_const)
{
	register_output("Q", &m_Q);
	register_param("CONST", &m_const, 0.0);
}

NETLIB_UPDATE(netdev_const)
{
}

NETLIB_UPDATE_PARAM(netdev_const)
{
	m_Q.setTo(m_const.ValueInt());
}

NETLIB_START(netdev_input)
{
	register_output("Q", &m_Q);
}

NETLIB_UPDATE(netdev_input)
{
}

NETLIB_START(netdev_delay_lh)
{
	register_input("CLK", &m_clk);
	register_input("D", &m_D, NET_INP_TYPE_PASSIVE);
	register_output("Q", &m_Q);

	m_lastclk = nst_LOW;
	m_Q.initial(1);
}

NETLIB_UPDATE(netdev_delay_lh)
{
	UINT8 oclk = m_lastclk;
	m_lastclk = INPVAL(m_clk);
	if (!oclk && m_lastclk)
		m_Q.setTo(INPVAL(m_D));
}

NETLIB_START(nicMultiSwitch)
{
	static const char *sIN[8] = { "i1", "i2", "i3", "i4", "i5", "i6", "i7", "i8" };
	int i;

	m_position = 0;
	m_low.initial(0);

	for (i=0; i<8; i++)
	{
		register_input(sIN[i], &m_I[i]);
		m_I[i].o = GETINPPTR(m_low);
	}
	register_param("POS", &m_POS);
	register_output("Q", &m_Q);

	m_variable_input_count = true;
}

NETLIB_UPDATE(nicMultiSwitch)
{
	m_Q.setTo(INPVAL(m_I[m_position]));
}

NETLIB_UPDATE_PARAM(nicMultiSwitch)
{
	m_position = m_POS.ValueInt();
	update();
}

NETLIB_START(nicRSFF)
{
	register_input("S", &m_S);
	register_input("R", &m_R);
	register_output("Q", &m_Q);
	register_output("QQ", &m_QQ);
}

NETLIB_UPDATE(nicRSFF)
{
	if (INPVAL(m_S))
	{
		m_Q.set();
		m_QQ.clear();
	}
	else if (INPVAL(m_R))
	{
		m_Q.clear();
		m_QQ.set();
	}
}


NETLIB_START(nicNE555N_MSTABLE)
{
	register_input("TRIG", &m_trigger);
	register_output("Q", &m_Q);
	register_param("R", &m_R);
	register_param("C", &m_C);
	register_param("VS", &m_VS, 5.0);
	register_param("VL", &m_VL, 0.0 *5.0);
	register_param("VT", &m_VT, 0.67*5.0);

	m_Q.initial(0);

	m_timer = m_setup->netlist().alloc_timer(this, 0);
}

NETLIB_UPDATE_PARAM(nicNE555N_MSTABLE)
{
	double vt = m_VT.Value();
	double vl = m_VL.Value();

	if (vt > m_VS.Value()-0.7)
		vt = m_VS.Value()-0.7;

	if (vt < 1.4)
		vt = 1.4;

	if (vt<vl)
		m_time = 0;
	else
		m_time = - log((m_VS.Value()-vt)/(m_VS.Value()-vl)) * m_R.Value() * m_C.Value();
}

NETLIB_UPDATE(nicNE555N_MSTABLE)
{
	if (!m_Q.Q())
	{
		if (m_last && !INPVAL(m_trigger))
		{
			m_fired = 0;
			m_timer->adjust_timer(m_time);
			m_Q.set();
		}
	}
	else if (m_fired)
	{
		if (INPVAL(m_trigger))
			m_Q.clear();
	}
	m_last = INPVAL(m_trigger);
}

NETLIB_TIMER_CALLBACK(nicNE555N_MSTABLE)
{
	if (INPVAL(m_trigger))
		m_Q.clear();
	m_fired = 1;
}


NETLIB_UPDATE(nic7400)
{
	UINT8 t = (INPVAL(m_i[0]) & INPVAL(m_i[1])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7402)
{
	UINT8 t = (INPVAL(m_i[0]) | INPVAL(m_i[1])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7404)
{
	UINT8 t = (INPVAL(m_i[0])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7410)
{
	UINT8 t = (INPVAL(m_i[0]) & INPVAL(m_i[1]) & INPVAL(m_i[2])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7420)
{
	UINT8 t = (INPVAL(m_i[0]) & INPVAL(m_i[1]) & INPVAL(m_i[2]) & INPVAL(m_i[3])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7425)
{
	UINT8 t = (INPVAL(m_i[0]) | INPVAL(m_i[1]) | INPVAL(m_i[2]) | INPVAL(m_i[3])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7427)
{
	UINT8 t = (INPVAL(m_i[0]) | INPVAL(m_i[1]) | INPVAL(m_i[2])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7430)
{
	UINT8 t = (INPVAL(m_i[0]) & INPVAL(m_i[1]) & INPVAL(m_i[2]) & INPVAL(m_i[3]) & INPVAL(m_i[4]) & INPVAL(m_i[5]) & INPVAL(m_i[6]) & INPVAL(m_i[7])) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

NETLIB_UPDATE(nic7486)
{
	UINT8 t = INPVAL(m_i[0]) ^ INPVAL(m_i[1]);
	m_Q.setTo(t, t ? 22 : 15  );
}

NETLIB_START(nic7448)
{
	register_input("A0", &m_A0);
	register_input("A1", &m_A1);
	register_input("A2", &m_A2);
	register_input("A3", &m_A3);
	register_input("LTQ", &m_LTQ);
	register_input("BIQ", &m_BIQ);
	register_input("RBIQ",&m_RBIQ);

	register_output("a", &m_a);
	register_output("b", &m_b);
	register_output("c", &m_c);
	register_output("d", &m_d);
	register_output("e", &m_e);
	register_output("f", &m_f);
	register_output("g", &m_g);
}

NETLIB_UPDATE(nic7448)
{
	UINT8 v;

	if (INPVAL(m_BIQ) && !INPVAL(m_LTQ))
		v = 8;
	else
	{
		v = (INPVAL(m_A0) << 0) | (INPVAL(m_A1) << 1) | (INPVAL(m_A2) << 2) | (INPVAL(m_A3) << 3);
		if (!INPVAL(m_BIQ) || (!INPVAL(m_RBIQ) && (v==0)))
			v = 15;
	}

	if (v != m_state)
	{
		m_a.setTo(tab7448[v][0]);
		m_b.setTo(tab7448[v][1]);
		m_c.setTo(tab7448[v][2]);
		m_d.setTo(tab7448[v][3]);
		m_e.setTo(tab7448[v][4]);
		m_f.setTo(tab7448[v][5]);
		m_g.setTo(tab7448[v][6]);
		m_state = v;
	}
}

const UINT8 nic7448::tab7448[16][7] =
{
		{	1, 1, 1, 1, 1, 1, 0 },  /* 00 - not blanked ! */
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

NETLIB_UPDATE(nic7450)
{
	UINT8 t1 = INPVAL(m_i[0]) & INPVAL(m_i[1]);
	UINT8 t2 = INPVAL(m_i[2]) & INPVAL(m_i[3]);
	UINT8 t =  (t1 | t2) ^ 1;
	m_Q.setTo(t, t ? 22 : 15);
}

INLINE void nic7474_newstate(UINT8 state, net_output_t &Q, net_output_t &QQ)
{
	if (state != Q.Q())
	{
		Q.setToNoCheck(state, state ? 40 : 25);
		QQ.setToNoCheck(!state, !state ? 40 : 25);
	}
}

NETLIB_UPDATE(nic7474)
{
	net_sig_t old_clk = m_lastclk;
	m_lastclk = INPVAL(m_clk);

	if (UNEXPECTED(!INPVAL(m_preQ)))
		nic7474_newstate(1, m_Q, m_QQ);
	else if (UNEXPECTED(!INPVAL(m_clrQ)))
		nic7474_newstate(0, m_Q, m_QQ);
	else if (!old_clk && m_lastclk)
		nic7474_newstate(INPVAL(m_D), m_Q, m_QQ);
}

NETLIB_START(nic7474)
{
	m_lastclk = 0;

	register_input("CLK",  &m_clk);
	register_input("D",    &m_D, NET_INP_TYPE_PASSIVE);
	register_input("CLRQ", &m_clrQ);
	register_input("PREQ", &m_preQ);

	register_output("Q",   &m_Q);
	register_output("QQ",  &m_QQ);

	m_Q.initial(0);
	m_QQ.initial(1);
}

NETLIB_START(nic7483)
{
	m_lastr = 0;

	register_input("A1", &m_A1);
	register_input("A2", &m_A2);
	register_input("A3", &m_A3);
	register_input("A4", &m_A4);
	register_input("B1", &m_B1);
	register_input("B2", &m_B2);
	register_input("B3", &m_B3);
	register_input("B4", &m_B4);
	register_input("CI", &m_CI);

	register_output("SA", &m_SA);
	register_output("SB", &m_SB);
	register_output("SC", &m_SC);
	register_output("SD", &m_SD);
	register_output("CO", &m_CO);
}

NETLIB_UPDATE(nic7483)
{
	UINT8 a = (INPVAL(m_A1) << 0) | (INPVAL(m_A2) << 1) | (INPVAL(m_A3) << 2) | (INPVAL(m_A4) << 3);
	UINT8 b = (INPVAL(m_B1) << 0) | (INPVAL(m_B2) << 1) | (INPVAL(m_B3) << 2) | (INPVAL(m_B4) << 3);

	UINT8 r = a + b + INPVAL(m_CI);

	if (r != m_lastr)
	{
		m_lastr = r;
		m_SA.setTo((r >> 0) & 1);
		m_SB.setTo((r >> 1) & 1);
		m_SC.setTo((r >> 2) & 1);
		m_SD.setTo((r >> 3) & 1);
		m_CO.setTo((r >> 4) & 1);
	}
}

NETLIB_START(nic7490)
{
	m_lastclk = 0;
	m_cnt = 0;

	register_input("CLK", &m_clk);
	register_input("R1",  &m_R1);
	register_input("R2",  &m_R2);
	register_input("R91", &m_R91);
	register_input("R92", &m_R92);

	register_output("QA", &m_QA);
	register_output("QB", &m_QB);
	register_output("QC", &m_QC);
	register_output("QD", &m_QD);
}

NETLIB_UPDATE(nic7490)
{
	UINT8 old_clk = m_lastclk;
	m_lastclk = INPVAL(m_clk);

	if (UNEXPECTED(INPVAL(m_R91) & INPVAL(m_R92)))
	{
		m_cnt = 9;
		update_outputs();
	}
	else if (UNEXPECTED(INPVAL(m_R1) & INPVAL(m_R2)))
	{
		m_cnt = 0;
		update_outputs();
	}
	else if (old_clk & !m_lastclk)
	{
		m_cnt++;
		if (m_cnt >= 10)
			m_cnt = 0;
		update_outputs();
	}
}

NETLIB_FUNC_VOID(nic7490, update_outputs)
{
	m_QA.setTo((m_cnt >> 0) & 1);
	m_QB.setTo((m_cnt >> 1) & 1);
	m_QC.setTo((m_cnt >> 2) & 1);
	m_QD.setTo((m_cnt >> 3) & 1);
}

NETLIB_START(nic7493)
{
	m_lastclk = 0;
	m_cnt = 0;

	register_input("CLK", &m_clk);
	register_input("R1",  &m_R1);
	register_input("R2",  &m_R2);

	register_output("QA", &m_QA);
	register_output("QB", &m_QB);
	register_output("QC", &m_QC);
	register_output("QD", &m_QD);
}

NETLIB_UPDATE(nic7493)
{
	UINT8 old_clk = m_lastclk;
	m_lastclk = INPVAL(m_clk);

	if (UNEXPECTED(INPVAL(m_R1) & INPVAL(m_R2)))
	{
		if (EXPECTED(m_cnt > 0))
		{
			m_cnt = 0;
			m_QA.setTo(0, 40);
			m_QB.setTo(0, 40);
			m_QC.setTo(0, 40);
			m_QD.setTo(0, 40);
		}
	}
	else if (EXPECTED(old_clk & !m_lastclk))
	{
		m_cnt++;
		m_cnt &= 0x0f;
		update_outputs();
	}
}

NETLIB_FUNC_VOID(nic7493, update_outputs)
{
	if (m_cnt & 1)
		m_QA.setToNoCheck(1, 16);
	else
	{
		m_QA.setToNoCheck(0, 16);
		switch (m_cnt)
		{
		case 0x00:
			m_QB.setToNoCheck(0, 34);
			m_QC.setToNoCheck(0, 48);
			m_QD.setToNoCheck(0, 70);
			break;
		case 0x02:
		case 0x06:
		case 0x0A:
		case 0x0E:
			m_QB.setToNoCheck(1, 34);
			break;
		case 0x04:
		case 0x0C:
			m_QB.setToNoCheck(0, 34);
			m_QC.setToNoCheck(1, 48);
			break;
		case 0x08:
			m_QB.setToNoCheck(0, 34);
			m_QC.setToNoCheck(0, 48);
			m_QD.setToNoCheck(1, 70);
			break;
		}
	}
}


NETLIB_START(nic74107A)
{
	register_input("CLK", &m_clk);
	register_input("J", &m_J, NET_INP_TYPE_PASSIVE);
	register_input("K", &m_K, NET_INP_TYPE_PASSIVE);
	register_input("CLRQ", &m_clrQ);
	register_output("Q", &m_Q);
	register_output("QQ", &m_QQ);

	m_lastclk = 0;
	m_Q.initial(0);
	m_QQ.initial(1);
}

INLINE void nic74107A_newstate(UINT8 state, net_output_t &Q, net_output_t &QQ)
{
	if (state != Q.Q())
	{
		Q.setToNoCheck(state, state ? 40 : 25);
		QQ.setToNoCheck(!state, state ? 25 : 40);
	}
}

NETLIB_UPDATE(nic74107A)
{
	UINT8 oclk = m_lastclk;
	m_lastclk = INPVAL(m_clk);

	if (!INPVAL(m_clrQ))
		nic74107A_newstate(0, m_Q, m_QQ);
	else if (oclk & !m_lastclk)
	{
		if (EXPECTED(INPVAL(m_J) & INPVAL(m_K)))
			nic74107A_newstate(!m_Q.Q(), m_Q, m_QQ);
		else if (!INPVAL(m_J) & INPVAL(m_K))
			nic74107A_newstate(0, m_Q, m_QQ);
		else if (INPVAL(m_J) & !INPVAL(m_K))
			nic74107A_newstate(1, m_Q, m_QQ);
	}
}

NETLIB_START(nic74153)
{
	register_input("A1", &m_I[0]);
	register_input("A2", &m_I[1]);
	register_input("A3", &m_I[2]);
	register_input("A4", &m_I[3]);
	register_input("A", &m_A);
	register_input("B", &m_B);
	register_input("GA", &m_GA);

	register_output("AY", &m_AY);
}

NETLIB_UPDATE(nic74153)
{
	if (!INPVAL(m_GA))
	{
		UINT8 chan = (INPVAL(m_A) | (INPVAL(m_B)<<1));
		m_AY.setTo(INPVAL(m_I[chan]));
	}
	else
		m_AY.clear();
}

NETLIB_START(nic9316)
{
	m_lastclk = 0;
	m_cnt = 0;

	register_input("CLK", &m_clk);
	register_input("ENP", &m_ENP, NET_INP_TYPE_PASSIVE);
	register_input("ENT", &m_ENT);
	register_input("CLRQ", &m_CLRQ);
	register_input("LOADQ", &m_LOADQ, NET_INP_TYPE_PASSIVE);
	register_input("A", &m_A, NET_INP_TYPE_PASSIVE);
	register_input("B", &m_B, NET_INP_TYPE_PASSIVE);
	register_input("C", &m_C, NET_INP_TYPE_PASSIVE);
	register_input("D", &m_D, NET_INP_TYPE_PASSIVE);

	register_output("QA", &m_QA);
	register_output("QB", &m_QB);
	register_output("QC", &m_QC);
	register_output("QD", &m_QD);
	register_output("RC", &m_RC);
}

NETLIB_UPDATE(nic9316)
{
	UINT8 old_clk = m_lastclk;
	m_lastclk = INPVAL(m_clk);

	if (EXPECTED(INPVAL(m_CLRQ)))
	{
		if (EXPECTED(!old_clk & m_lastclk))
		{
			if (EXPECTED(INPVAL(m_LOADQ)))
			{
				if (EXPECTED(INPVAL(m_ENP) & INPVAL(m_ENT)))
				{
							m_cnt = ( m_cnt + 1) & 0x0f;
							update_outputs();
				}
			}
			else
			{
				m_cnt = (INPVAL(m_D) << 3) | (INPVAL(m_C) << 2) | (INPVAL(m_B) << 1) | (INPVAL(m_A) << 0);
				update_outputs_all();
			}
		}
		m_RC.setTo((INPVAL(m_ENT) & (m_cnt == 0x0f)), 20);
	}
	else if (m_cnt>0)
	{
		m_cnt = 0;
		update_outputs();
		m_RC.setTo(0, 20);
	}
}

NETLIB_FUNC_VOID(nic9316, update_outputs_all)
{
	m_QA.setTo((m_cnt >> 0) & 1, 20);
	m_QB.setTo((m_cnt >> 1) & 1, 20);
	m_QC.setTo((m_cnt >> 2) & 1, 20);
	m_QD.setTo((m_cnt >> 3) & 1, 20);
}

NETLIB_FUNC_VOID(nic9316, update_outputs)
{
#if 0
	m_QA.setTo((m_cnt >> 0) & 1, 20);
	m_QB.setTo((m_cnt >> 1) & 1, 20);
	m_QC.setTo((m_cnt >> 2) & 1, 20);
	m_QD.setTo((m_cnt >> 3) & 1, 20);
#else
	if (m_cnt & 1)
		m_QA.setToNoCheck(1, 20);
	else
	{
		m_QA.setToNoCheck(0, 20);
		switch (m_cnt)
		{
		case 0x00:
			m_QB.setToNoCheck(0, 20);
			m_QC.setToNoCheck(0, 20);
			m_QD.setToNoCheck(0, 20);
			break;
		case 0x02:
		case 0x06:
		case 0x0A:
		case 0x0E:
			m_QB.setToNoCheck(1, 20);
			break;
		case 0x04:
		case 0x0C:
			m_QB.setToNoCheck(0, 20);
			m_QC.setToNoCheck(1, 20);
			break;
		case 0x08:
			m_QB.setToNoCheck(0, 20);
			m_QC.setToNoCheck(0, 20);
			m_QD.setToNoCheck(1, 20);
			break;
		}

	}
#endif
}

#define ENTRY(_nic, _name) new net_dev_t_factory< _nic >( # _name, # _nic ),

static net_dev_t_base_factory *netregistry[] =
{
	ENTRY(netdev_const, 		NETDEV_CONST)
	ENTRY(netdev_input, 		NETDEV_INPUT)
	ENTRY(netdev_callback,		NETDEV_CALLBACK)
	ENTRY(nicMultiSwitch,		NETDEV_SWITCH2)
	ENTRY(netdev_delay_lh,		NETDEV_DELAY_RAISE)
	ENTRY(nicRSFF,				NETDEV_RSFF)
	ENTRY(nic7400,				TTL_7400_NAND)
	ENTRY(nic7402,				TTL_7402_NOR)
	ENTRY(nic7404,				TTL_7404_INVERT)
	ENTRY(nic7410,				TTL_7410_NAND)
	ENTRY(nic7420,				TTL_7420_NAND)
	ENTRY(nic7425,				TTL_7425_NOR)
	ENTRY(nic7427,				TTL_7427_NOR)
	ENTRY(nic7430,				TTL_7430_NAND)
	ENTRY(nic7450,				TTL_7450_ANDORINVERT)
	ENTRY(nic7486,				TTL_7486_XOR)
	ENTRY(nic7448,				TTL_7448)
	ENTRY(nic7474,				TTL_7474)
	ENTRY(nic7483,				TTL_7483)
	ENTRY(nic7490,				TTL_7490)
	ENTRY(nic7493,				TTL_7493)
	ENTRY(nic74107,				TTL_74107)
	ENTRY(nic74107A,			TTL_74107A)
	ENTRY(nic74153,				TTL_74153)
	ENTRY(nic9316,				TTL_9316)
	ENTRY(nicNE555N_MSTABLE,	NE555N_MSTABLE)
	NULL
};

net_dev_t *net_create_device_by_classname(const char *classname, netlist_setup_t *setup, const char *icname)
{
	net_dev_t_base_factory **p = &netregistry[0];
	while (p != NULL)
	{
		if (strcmp((*p)->classname(), classname) == 0)
			return (*p)->Create(setup, icname);
		p++;
	}
	fatalerror("Class %s required for IC %s not found!", classname, icname);
}

net_dev_t *net_create_device_by_name(const char *name, netlist_setup_t *setup, const char *icname)
{
	net_dev_t_base_factory **p = &netregistry[0];
	while (p != NULL)
	{
		if (strcmp((*p)->name(), name) == 0)
			return (*p)->Create(setup, icname);
		p++;
	}
	fatalerror("Class %s required for IC %s not found!", name, icname);
}
