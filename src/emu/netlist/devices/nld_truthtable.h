// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_truthtable.h
 *
 *  Created on: 19 Jun 2014
 *      Author: andre
 */

#ifndef NLD_TRUTHTABLE_H_
#define NLD_TRUTHTABLE_H_

#include "../nl_base.h"

#define NETLIB_TRUTHTABLE(_name, _nIN, _nOUT, _state)                               \
	class NETLIB_NAME(_name) : public nld_truthtable_t<_nIN, _nOUT, _state>         \
	{                                                                               \
	public:                                                                         \
		NETLIB_NAME(_name)()                                                        \
		: nld_truthtable_t<_nIN, _nOUT, _state>(&m_ttbl, m_desc) { }                \
	private:                                                                        \
		static truthtable_t m_ttbl;                                                 \
		static const char *m_desc[];                                                \
	}


static inline UINT32 remove_first_bit(UINT32 v)
{
	for (int i=0; i<32; i++)
		if (v & (1<<i))
			return v & ~(1<<i);
	return v;
}

static inline int count_bits(UINT32 v)
{
	int ret = 0;
	for (int i=0; i<32; i++)
		if (v & (1<<i))
			ret++;
	return ret;
}

static inline UINT32 set_bits(UINT32 v, UINT32 b)
{
	UINT32 ret = 0;
	for (int i=0; i<32; i++)
		if (v & (1<<i))
		{
			ret |= ((b&1)<<i);
			b = b >> 1;
		}
	return ret;
}

template<int m_NI, int m_NO, int has_state>
class nld_truthtable_t : public netlist_device_t
{
public:

	static const int m_num_bits = m_NI + has_state * (m_NI + m_NO);
	static const int m_size = (1 << (m_num_bits));

	struct truthtable_t
	{
		truthtable_t() : m_initialized(false) {}
		UINT32 m_outs[m_size];
		UINT8  m_timing[m_size * m_NO];
		netlist_time m_timing_nt[16];
		bool m_initialized;
	};

	nld_truthtable_t(truthtable_t *ttbl, const char *desc[])
	: netlist_device_t(), m_last_state(0), m_ign(0), m_active(1), m_ttp(ttbl), m_desc(desc)
	{
	}

	ATTR_COLD virtual void start()
	{
		pstring ttline = pstring(m_desc[0]);

		nl_util::pstring_list io = nl_util::split(ttline,"|");
		// checks
		nl_assert_always(io.count() == 2, "too many '|'");
		nl_util::pstring_list inout = nl_util::split(io[0], ",");
		nl_assert_always(inout.count() == m_num_bits, "bitcount wrong");
		nl_util::pstring_list out = nl_util::split(io[1], ",");
		nl_assert_always(out.count() == m_NO, "output count wrong");

		for (int i=0; i < m_NI; i++)
		{
			inout[i] = inout[i].trim();
			register_input(inout[i], m_i[i]);
		}
		for (int i=0; i < m_NO; i++)
		{
			out[i] = out[i].trim();
			register_output(out[i], m_Q[i]);
		}
		{
			// Connect output "Q" to input "_Q" if this exists
			// This enables timed state without having explicit state ....
			for (int i=0; i < m_NO; i++)
			{
				pstring tmp = "_" + out[i];
				const int idx = inout.indexof(tmp);
				if (idx>=0)
				{
					//printf("connecting %s %d\n", out[i].cstr(), idx);
					connect(m_Q[i], m_i[idx]);
				}
			}
		}
		m_ign = 0;
		setup_tt();
		// FIXME: save state
	}

	ATTR_COLD void help(int cur, nl_util::pstring_list list,
			UINT64 state,UINT16 val, UINT8 timing_index[m_NO])
	{
		pstring elem = list[cur].trim();
		int start = 0;
		int end = 0;

		if (elem.equals("0"))
		{
			start = 0;
			end = 0;
		}
		else if (elem.equals("1"))
		{
			start = 1;
			end = 1;
		}
		else if (elem.equals("X"))
		{
			start = 0;
			end = 1;
		}
		else
			nl_assert_always(false, "unknown input value (not 0, 1, or X)");
		for (int i = start; i <= end; i++)
		{
			const UINT64 nstate = state | (i << cur);

			if (cur < m_num_bits - 1)
			{
				help(cur + 1, list, nstate, val, timing_index);
			}
			else
			{
				// cutoff previous inputs and outputs for ignore
				m_ttp->m_outs[nstate] = val;
				for (int j=0; j<m_NO; j++)
					m_ttp->m_timing[nstate * m_NO + j] = timing_index[j];
			}
		}
	}

	ATTR_COLD void setup_tt()
	{
		if (m_ttp->m_initialized)
			return;

		const char **truthtable = m_desc;
		pstring ttline = pstring(truthtable[0]);
		truthtable++;
		ttline = pstring(truthtable[0]);
		truthtable++;

		for (int j=0; j < m_size; j++)
			m_ttp->m_outs[j] = -1;

		for (int j=0; j < 16; j++)
			m_ttp->m_timing_nt[j] = netlist_time::zero;

		while (!ttline.equals(""))
		{
			nl_util::pstring_list io = nl_util::split(ttline,"|");
			// checks
			nl_assert_always(io.count() == 3, "io.count mismatch");
			nl_util::pstring_list inout = nl_util::split(io[0], ",");
			nl_assert_always(inout.count() == m_num_bits, "number of bits not matching");
			nl_util::pstring_list out = nl_util::split(io[1], ",");
			nl_assert_always(out.count() == m_NO, "output count not matching");
			nl_util::pstring_list times = nl_util::split(io[2], ",");
			nl_assert_always(times.count() == m_NO, "timing count not matching");

			UINT16 val = 0;
			UINT8 tindex[m_NO];
			for (int j=0; j<m_NO; j++)
			{
				pstring outs = out[j].trim();
				if (outs.equals("1"))
					val = val | (1 << j);
				else
					nl_assert_always(outs.equals("0"), "Unknown value (not 0 or 1");
				netlist_time t = netlist_time::from_nsec(times[j].trim().as_long());
				int k=0;
				while (m_ttp->m_timing_nt[k] != netlist_time::zero && m_ttp->m_timing_nt[k] != t)
					k++;
				m_ttp->m_timing_nt[k] = t;
				tindex[j] = k;
			}

			help(0, inout, 0 , val, tindex);
			ttline = pstring(truthtable[0]);
			truthtable++;
		}

		// ((nignore & ((1 << m_NI)-1)) << m_NO)

		// determine ignore
		UINT32 ign[m_size];

		for (int i=0; i<m_size; i++)
		{
#if 1
			UINT32 m_enable = 0;
			for (int j=0; j<m_size; j++)
			{
				if (m_ttp->m_outs[j] != m_ttp->m_outs[i])
				{
					m_enable |= (i ^ j);
				}
			}
			ign[i] = m_enable ^ (m_size - 1);
#else
			// this doesn't work in all cases and is dog slow
			UINT32 nign = 0;
			for (int j=0; j<m_NI; j++)
			{
				if (m_ttp->m_outs[i] == m_ttp->m_outs[i ^ (1 << j)])
					nign |= (1<<j);
			}
			/* Check all permutations of ign
			 */
			int t[1<<count_bits(nign)];

			for (UINT32 j=0; j<(1<<count_bits(nign)); j++)
			{
				UINT32 tign = set_bits(nign, j);
				t[j] = 0;
				for (UINT32 k=0; k<(1<<count_bits(tign)); k++)
				{
					UINT32 b=set_bits(tign, k);
					if (m_ttp->m_outs[i] != m_ttp->m_outs[(i & ~tign) | b])
						t[j] = 1;
				}
				//printf("x %d %d\n", j, tign);
			}
			UINT32 jb=0;
			UINT32 jm=0;
			for (UINT32 j=0; j<(1<<count_bits(nign)); j++)
			{
				int nb = count_bits(j);
				if ((t[j] == 0)&& (nb>jb))
				{
					jb = nb;
					jm = j;
				}
			}
			ign[i] = set_bits(nign, jm);
#endif
		}
		for (int i=0; i<m_size; i++)
		{
			m_ttp->m_outs[i] |= (ign[i] << m_NO);
		}
#if 0
		printf("%s\n", name().cstr());
		for (int j=0; j < m_size; j++)
			printf("%05x %04x %04x %04x\n", j, m_ttp->m_outs[j] & ((1 << m_NO)-1),
					m_ttp->m_outs[j] >> m_NO, m_ttp->m_timing[j][0]);
		for (int k=0; m_ttp->m_timing_nt[k] != netlist_time::zero; k++)
			printf("%d %f\n", k, m_ttp->m_timing_nt[k].as_double() * 1000000.0);
#endif
		m_ttp->m_initialized = true;

	}

	ATTR_COLD void reset()
	{
		m_active = 1;
		m_last_state = 0;
	}

	template<bool doOUT>
	ATTR_HOT inline void process()
	{

		netlist_time mt = netlist_time::zero;

		UINT32 state = 0;
		for (int i = 0; i < m_NI; i++)
		{
			if (!doOUT || (m_ign & (1<<i)) != 0)
				m_i[i].activate();
			state |= (INPLOGIC(m_i[i]) << i);
		}

		if (!doOUT)
		{
			for (int i = 0; i< m_NI; i++)
			{
				if (this->m_i[i].net().time() > mt)
					mt = this->m_i[i].net().time();
			}
		}

		const UINT32 nstate = state | (has_state ? (m_last_state << m_NI) : 0);
		const UINT32 out = m_ttp->m_outs[nstate] & ((1 << m_NO) - 1);
		m_ign = m_ttp->m_outs[nstate] >> m_NO;
		if (has_state)
			m_last_state = (state << m_NO) | out;

		for (int i = 0; i < m_NI; i++)
			if (m_ign & (1 << i))
				m_i[i].inactivate();

		const UINT32 timebase = nstate * m_NO;
		if (doOUT)
		{
			for (int i = 0; i < m_NO; i++)
				OUTLOGIC(m_Q[i], (out >> i) & 1, m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);
		}
		else
			for (int i = 0; i < m_NO; i++)
				m_Q[i].net().set_Q_time((out >> i) & 1, mt + m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);

	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		process<true>();
	}

	ATTR_HOT void inc_active()
	{
		nl_assert(netlist().use_deactivate());
		if (has_state == 0)
			if (++m_active == 1)
				process<false>();
	}

	ATTR_HOT void dec_active()
	{
		nl_assert(netlist().use_deactivate());
		if (has_state == 0)
			if (--m_active == 0)
				for (int i = 0; i< m_NI; i++)
					m_i[i].inactivate();
	}

	netlist_logic_input_t m_i[m_NI];
	netlist_logic_output_t m_Q[m_NO];

private:

	UINT32 m_last_state;
	UINT32 m_ign;
	INT32 m_active;

	truthtable_t *m_ttp;
	const char **m_desc;
};

#endif /* NLD_TRUTHTABLE_H_ */
