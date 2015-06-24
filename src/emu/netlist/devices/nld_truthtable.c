// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_truthtable.c
 *
 */

#include "nld_truthtable.h"
#include "../plib/plists.h"

NETLIB_NAMESPACE_DEVICES_START()

unsigned truthtable_desc_t::count_bits(UINT32 v)
{
	unsigned ret = 0;
	for (; v != 0; v = v >> 1)
	{
		if (v & 1)
			ret++;
	}
	return ret;
}

UINT32 truthtable_desc_t::set_bits(UINT32 v, UINT32 b)
{
	UINT32 ret = 0;
	int i = 0;
	for (; v != 0; v = v >> 1)
	{
		if (v & 1)
		{
			ret |= ((b&1)<<i);
			b = b >> 1;
		}
		i++;
	}
	return ret;
}

UINT32 truthtable_desc_t::get_ignored_simple(UINT32 i)
{
	UINT32 m_enable = 0;
	for (UINT32 j=0; j<m_size; j++)
	{
		if (m_outs[j] != m_outs[i])
		{
			m_enable |= (i ^ j);
		}
	}
	return m_enable ^ (m_size - 1);
}

UINT32 truthtable_desc_t::get_ignored_extended(UINT32 i)
{
	// Determine all inputs which may be ignored ...
	UINT32 nign = 0;
	for (unsigned j=0; j<m_NI; j++)
	{
		if (m_outs[i] == m_outs[i ^ (1 << j)])
			nign |= (1<<j);
	}
	/* Check all permutations of ign
	 * We have to remove those where the ignored inputs
	 * may change the output
	 */
	UINT32 bits = (1<<count_bits(nign));
	parray_t<int> t(bits);

	for (UINT32 j=1; j<bits; j++)
	{
		UINT32 tign = set_bits(nign, j);
		t[j] = 0;
		UINT32 bitsk=(1<<count_bits(tign));
		for (UINT32 k=0; k<bitsk; k++)
		{
			UINT32 b=set_bits(tign, k);
			if (m_outs[i] != m_outs[(i & ~tign) | b])
			{
				t[j] = 1;
				break;
			}
		}
	}
	UINT32 jb=0;
	UINT32 jm=0;
	for (UINT32 j=1; j<bits; j++)
	{
		unsigned nb = count_bits(j);
		if ((t[j] == 0)&& (nb>jb))
		{
			jb = nb;
			jm = j;
		}
	}
	return set_bits(nign, jm);
}

// ----------------------------------------------------------------------------------------
// desc
// ----------------------------------------------------------------------------------------

void truthtable_desc_t::help(unsigned cur, pstring_list_t list,
		UINT64 state,UINT16 val, UINT8 *timing_index)
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
			if (m_outs[nstate] != ~0U &&  m_outs[nstate] != val)
				fatalerror_e("Error in truthtable: State %04x already set, %d != %d\n",
						(UINT32) nstate, m_outs[nstate], val);
			m_outs[nstate] = val;
			for (unsigned j=0; j<m_NO; j++)
				m_timing[nstate * m_NO + j] = timing_index[j];
		}
	}
}

void truthtable_desc_t::setup(const pstring_list_t &truthtable, UINT32 disabled_ignore)
{
	unsigned line = 0;

	if (*m_initialized)
		return;

	pstring ttline = truthtable[line];
	line++;
	ttline = truthtable[line];
	line++;

	for (unsigned j=0; j < m_size; j++)
		m_outs[j] = ~0L;

	for (int j=0; j < 16; j++)
		m_timing_nt[j] = netlist_time::zero;

	while (!ttline.equals(""))
	{
		pstring_list_t io(ttline,"|");
		// checks
		nl_assert_always(io.size() == 3, "io.count mismatch");
		pstring_list_t inout(io[0], ",");
		nl_assert_always(inout.size() == m_num_bits, "number of bits not matching");
		pstring_list_t out(io[1], ",");
		nl_assert_always(out.size() == m_NO, "output count not matching");
		pstring_list_t times(io[2], ",");
		nl_assert_always(times.size() == m_NO, "timing count not matching");

		UINT16 val = 0;
		parray_t<UINT8> tindex(m_NO);

		for (unsigned j=0; j<m_NO; j++)
		{
			pstring outs = out[j].trim();
			if (outs.equals("1"))
				val = val | (1 << j);
			else
				nl_assert_always(outs.equals("0"), "Unknown value (not 0 or 1");
			netlist_time t = netlist_time::from_nsec(times[j].trim().as_long());
			int k=0;
			while (m_timing_nt[k] != netlist_time::zero && m_timing_nt[k] != t)
				k++;
			m_timing_nt[k] = t;
			tindex[j] = k;
		}

		help(0, inout, 0 , val, tindex.data());
		ttline = truthtable[line];
		line++;
	}

	// determine ignore
	parray_t<UINT32> ign(m_size);

	for (UINT32 j=0; j < m_size; j++)
		ign[j] = ~0U;

	for (UINT32 i=0; i<m_size; i++)
	{
		if (ign[i] == ~0U)
		{
			int tign;
			if (0)
			{
				tign = get_ignored_simple(i);
				ign[i] = tign;
			}
			else
			{
				tign = get_ignored_extended(i);

				ign[i] = tign;
				/* don't need to recalculate similar ones */
				UINT32 bitsk=(1<<count_bits(tign));
				for (UINT32 k=0; k<bitsk; k++)
				{
					UINT32 b=set_bits(tign, k);
					ign[(i & ~tign) | b] = tign;
				}
			}
		}
	}
	for (UINT32 i=0; i<m_size; i++)
	{
		if (m_outs[i] == ~0U)
			fatalerror_e("truthtable: found element not set %04x\n", i);
		m_outs[i] |= ((ign[i] & ~disabled_ignore)  << m_NO);
	}
	*m_initialized = true;

}

#define ENTRYX(_n,_m,_h)    case (_n * 1000 + _m * 10 + _h): \
	{ typedef netlist_factory_truthtable_t<_n,_m,_h> xtype; \
		return palloc(xtype,name,classname,def_param); } break

#define ENTRYY(_n,_m)   ENTRYX(_n,_m,0); ENTRYX(_n,_m,1)

#define ENTRY(_n) ENTRYY(_n, 1); ENTRYY(_n, 2); ENTRYY(_n, 3); ENTRYY(_n, 4); ENTRYY(_n, 5); ENTRYY(_n, 6)

netlist_base_factory_truthtable_t *nl_tt_factory_create(const unsigned ni, const unsigned no,
		const unsigned has_state,
		const pstring &name, const pstring &classname,
		const pstring &def_param)
{
	switch (ni * 1000 + no * 10 + has_state)
	{
		ENTRY(1);
		ENTRY(2);
		ENTRY(3);
		ENTRY(4);
		ENTRY(5);
		ENTRY(6);
		ENTRY(7);
		ENTRY(8);
		ENTRY(9);
		ENTRY(10);
		default:
			pstring msg = pstring::sprintf("unable to create truthtable<%d,%d,%d>", ni, no, has_state);
			nl_assert_always(false, msg.cstr());
	}
	return NULL;
}

NETLIB_NAMESPACE_DEVICES_END()
