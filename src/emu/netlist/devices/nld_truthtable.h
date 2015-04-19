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
		UINT8  m_timing[m_size][m_NO];
		netlist_time m_timing_nt[16];
		bool m_initialized;
	};

	nld_truthtable_t(truthtable_t *ttbl, const char *desc[])
	: netlist_device_t(), m_last_state(0), m_active(1), m_ttp(ttbl), m_desc(desc)
	{
	}

	ATTR_COLD virtual void start()
	{
		pstring ttline = pstring(m_desc[0]);
		{
			nl_util::pstring_list io = nl_util::split(ttline,"|");
			// checks
			nl_assert_always(io.count() == 2, "too many '|'");
			nl_util::pstring_list inout = nl_util::split(io[0], ",");
			nl_assert_always(inout.count() == m_num_bits, "bitcount wrong");
			nl_util::pstring_list out = nl_util::split(io[1], ",");
			nl_assert_always(out.count() == m_NO, "output count wrong");

			for (int i=0; i < m_NI; i++)
			{
				register_input(inout[i].trim(), m_i[i]);
			}
			for (int i=0; i < m_NO; i++)
			{
				register_output(out[i].trim(), m_Q[i]);
			}
		}
		setup_tt();
		// FIXME: save state
	}

	ATTR_COLD void help(int cur, nl_util::pstring_list list,
			UINT64 state, UINT64 ignore, UINT16 val, UINT8 timing_index[m_NO])
	{
		pstring elem = list[cur];
		int start = 0;
		int end = 0;
		int ign = 0;

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
			ign = 1;
		}
		for (int i = start; i <= end; i++)
		{
			const UINT64 nstate = state | (i << cur);
			const UINT64 nignore = ignore | (ign << cur);

			if (cur < m_num_bits - 1)
			{
				help(cur + 1, list, nstate, nignore, val, timing_index);
			}
			else
			{
				// cutoff previous inputs and outputs for ignore
				m_ttp->m_outs[nstate] = val | ((nignore & ((1 << m_NI)-1)) << m_NO);
				for (int j=0; j<m_NO; j++)
					m_ttp->m_timing[nstate][j] = timing_index[j];
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
			nl_assert(io.count() == 3);
			nl_util::pstring_list inout = nl_util::split(io[0], ",");
			nl_assert(inout.count() == m_num_bits);
			nl_util::pstring_list out = nl_util::split(io[1], ",");
			nl_assert(out.count() == m_NO);
			nl_util::pstring_list times = nl_util::split(io[2], ",");
			nl_assert(times.count() == m_NO);

			UINT16 val = 0;
			UINT8 tindex[m_NO];
			for (int j=0; j<m_NO; j++)
			{
				if (out[j].equals("1"))
					val = val | (1 << j);
				netlist_time t = netlist_time::from_nsec(times[j].as_long());
				int k=0;
				while (m_ttp->m_timing_nt[k] != netlist_time::zero && m_ttp->m_timing_nt[k] != t)
					k++;
				m_ttp->m_timing_nt[k] = t;
				tindex[j] = k;
			}

			help(0, inout, 0 , 0 , val, tindex);
			ttline = pstring(truthtable[0]);
			truthtable++;
		}
		for (int j=0; j < m_size; j++)
			printf("%05x %04x %04x %04x\n", j, m_ttp->m_outs[j] & ((1 << m_NO)-1),
					m_ttp->m_outs[j] >> m_NO, m_ttp->m_timing[j][0]);
		for (int k=0; m_ttp->m_timing_nt[k] != netlist_time::zero; k++)
			printf("%d %f\n", k, m_ttp->m_timing_nt[k].as_double() * 1000000.0);

		m_ttp->m_initialized = true;

	}

	ATTR_COLD void reset()
	{
		//m_Q.initial(1);
		m_active = 1;
		m_last_state = 0;
	}

	ATTR_HOT ATTR_ALIGN void update()
	{
		// FIXME: this check is needed because update is called during startup as well
		//if (m_active == 0 && UNEXPECTED(netlist().use_deactivate()))
		//	return;

		UINT32 state = 0;
		for (int i = 0; i < m_NI; i++)
		{
			m_i[i].activate();
			state |= (INPLOGIC(m_i[i]) << i);
		}

		const UINT32 nstate = state | (has_state ? (m_last_state << m_NI) : 0);
		const UINT32 out = m_ttp->m_outs[nstate] & ((1 << m_NO) - 1);
		const UINT32 ign = m_ttp->m_outs[nstate] >> m_NO;
		if (has_state)
			m_last_state = (state << m_NO) | out;

		for (int i = 0; i < m_NI; i++)
			if (ign & (1 << i))
				m_i[i].inactivate();

		for (int i = 0; i < m_NO; i++)
			OUTLOGIC(m_Q[i], (out >> i) & 1, m_ttp->m_timing_nt[m_ttp->m_timing[nstate][i]]);
	}


	ATTR_HOT void inc_active()
	{
		nl_assert(netlist().use_deactivate());
		if (++m_active == 1)
		{
			update();
		}
	}

	ATTR_HOT void dec_active()
	{
		nl_assert(netlist().use_deactivate());
		if (--m_active == 0)
		{
			for (int i = 0; i< m_NI; i++)
				m_i[i].inactivate();
		}
	}

	netlist_ttl_input_t m_i[m_NI];
	netlist_ttl_output_t m_Q[m_NO];

private:

	UINT32 m_last_state;
	INT32 m_active;

	truthtable_t *m_ttp;
	const char **m_desc;
};

#endif /* NLD_TRUTHTABLE_H_ */
