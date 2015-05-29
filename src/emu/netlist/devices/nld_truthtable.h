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
#include "../nl_factory.h"

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

#if 0
static inline UINT32 remove_first_bit(UINT32 v)
{
	for (int i=0; i<32; i++)
		if (v & (1<<i))
			return v & ~(1<<i);
	return v;
}
#endif

struct truthtable_desc_t
{
	truthtable_desc_t(int NO, int NI, int has_state, bool *initialized,
			UINT32 *outs, UINT8 *timing, netlist_time *timing_nt)
	: m_NO(NO), m_NI(NI), /*m_has_state(has_state),*/ m_initialized(initialized),
		m_outs(outs), m_timing(timing), m_timing_nt(timing_nt),
		m_num_bits(m_NI + has_state * (m_NI + m_NO)),
		m_size(1 << (m_num_bits))
	{
	}

	ATTR_COLD void setup(const pstring_list_t &desc, UINT32 disabled_ignore);

private:
	ATTR_COLD void help(unsigned cur, pstring_list_t list,
			UINT64 state,UINT16 val, UINT8 *timing_index);
	static unsigned count_bits(UINT32 v);
	static UINT32 set_bits(UINT32 v, UINT32 b);
	UINT32 get_ignored_simple(UINT32 i);
	UINT32 get_ignored_extended(UINT32 i);

	unsigned m_NO;
	unsigned m_NI;
	//int m_has_state;
	bool *m_initialized;
	UINT32 *m_outs;
	UINT8  *m_timing;
	netlist_time *m_timing_nt;

	/* additional values */

	const UINT32 m_num_bits;
	const UINT32 m_size;

};

template<unsigned m_NI, unsigned m_NO, int has_state>
class nld_truthtable_t : public netlist_device_t
{
public:

	static const int m_num_bits = m_NI + has_state * (m_NI + m_NO);
	static const int m_size = (1 << (m_num_bits));

	struct truthtable_t
	{
		truthtable_t() : m_initialized(false),
				m_desc(m_NO, m_NI, has_state, &m_initialized, m_outs, m_timing, m_timing_nt) {}
		bool m_initialized;
		UINT32 m_outs[m_size];
		UINT8  m_timing[m_size * m_NO];
		netlist_time m_timing_nt[16];
		truthtable_desc_t m_desc;
	};

	nld_truthtable_t(truthtable_t *ttbl, const char *desc[])
	: netlist_device_t(), m_last_state(0), m_ign(0), m_active(1), m_ttp(ttbl)
	{
		while (*desc != NULL && **desc != 0 )
			{
				m_desc.add(*desc);
				desc++;
			}

	}

	nld_truthtable_t(truthtable_t *ttbl, const pstring_list_t &desc)
	: netlist_device_t(), m_last_state(0), m_ign(0), m_active(1), m_ttp(ttbl)
	{
		m_desc = desc;
	}

	/* ATTR_COLD */ virtual void start()
	{
		pstring header = m_desc[0];

		pstring_list_t io(header,"|");
		// checks
		nl_assert_always(io.size() == 2, "too many '|'");
		pstring_list_t inout(io[0], ",");
		nl_assert_always(inout.size() == m_num_bits, "bitcount wrong");
		pstring_list_t out(io[1], ",");
		nl_assert_always(out.size() == m_NO, "output count wrong");

		for (unsigned i=0; i < m_NI; i++)
		{
			inout[i] = inout[i].trim();
			register_input(inout[i], m_i[i]);
		}
		for (unsigned i=0; i < m_NO; i++)
		{
			out[i] = out[i].trim();
			register_output(out[i], m_Q[i]);
		}
		// Connect output "Q" to input "_Q" if this exists
		// This enables timed state without having explicit state ....
		UINT32 disabled_ignore = 0;
		for (unsigned i=0; i < m_NO; i++)
		{
			pstring tmp = "_" + out[i];
			const int idx = inout.indexof(tmp);
			if (idx>=0)
			{
				//printf("connecting %s %d\n", out[i].cstr(), idx);
				connect(m_Q[i], m_i[idx]);
				// disable ignore for this inputs altogether.
				// FIXME: This shouldn't be necessary
				disabled_ignore |= (1<<idx);
			}
		}

		m_ign = 0;

		m_ttp->m_desc.setup(m_desc, disabled_ignore * 0);

#if 0
		printf("%s\n", name().cstr());
		for (int j=0; j < m_size; j++)
			printf("%05x %04x %04x %04x\n", j, m_ttp->m_outs[j] & ((1 << m_NO)-1),
					m_ttp->m_outs[j] >> m_NO, m_ttp->m_timing[j * m_NO + 0]);
		for (int k=0; m_ttp->m_timing_nt[k] != netlist_time::zero; k++)
			printf("%d %f\n", k, m_ttp->m_timing_nt[k].as_double() * 1000000.0);
#endif
		// FIXME: save state
		save(NLNAME(m_last_state));
		save(NLNAME(m_ign));
		save(NLNAME(m_active));

	}

	ATTR_COLD void reset()
	{
		m_active = 0;
		for (unsigned i=0; i<m_NO;i++)
			if (this->m_Q[i].net().num_cons()>0)
				m_active++;
		m_last_state = 0;
	}

	template<bool doOUT>
	ATTR_HOT inline void process()
	{
		netlist_time mt = netlist_time::zero;

		UINT32 state = 0;
		for (unsigned i = 0; i < m_NI; i++)
		{
			if (!doOUT || (m_ign & (1<<i)) != 0)
				m_i[i].activate();
		}
		for (unsigned i = 0; i < m_NI; i++)
		{
			state |= (INPLOGIC(m_i[i]) << i);
			if (!doOUT)
				if (this->m_i[i].net().time() > mt)
					mt = this->m_i[i].net().time();
		}

		const UINT32 nstate = state | (has_state ? (m_last_state << m_NI) : 0);
		const UINT32 out = m_ttp->m_outs[nstate] & ((1 << m_NO) - 1);
		m_ign = m_ttp->m_outs[nstate] >> m_NO;
		if (has_state)
			m_last_state = (state << m_NO) | out;

#if 0
		for (int i = 0; i < m_NI; i++)
			if (m_ign & (1 << i))
				m_i[i].inactivate();
#endif
		const UINT32 timebase = nstate * m_NO;
		if (doOUT)
		{
			for (unsigned i = 0; i < m_NO; i++)
				OUTLOGIC(m_Q[i], (out >> i) & 1, m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);
		}
		else
			for (unsigned i = 0; i < m_NO; i++)
				m_Q[i].net().set_Q_time((out >> i) & 1, mt + m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);

		for (unsigned i = 0; i < m_NI; i++)
			if (m_ign & (1 << i))
				m_i[i].inactivate();

	}

	ATTR_HOT void update()
	{
		process<true>();
	}

	ATTR_HOT void inc_active()
	{
		nl_assert(netlist().use_deactivate());
		if (has_state == 0)
			if (++m_active == 1)
			{
				process<false>();
			}
	}

	ATTR_HOT void dec_active()
	{
		nl_assert(netlist().use_deactivate());
		if (has_state == 0)
			if (--m_active == 0)
			{
				for (unsigned i = 0; i< m_NI; i++)
					m_i[i].inactivate();
			}
	}

	netlist_logic_input_t m_i[m_NI];
	netlist_logic_output_t m_Q[m_NO];

private:

	UINT32 m_last_state;
	UINT32 m_ign;
	INT32 m_active;

	truthtable_t *m_ttp;
	pstring_list_t m_desc;
};

class netlist_base_factory_truthtable_t : public netlist_base_factory_t
{
	NETLIST_PREVENT_COPYING(netlist_base_factory_truthtable_t)
public:
	ATTR_COLD netlist_base_factory_truthtable_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: netlist_base_factory_t(name, classname, def_param)
	{}
	pstring_list_t m_desc;
};


template<unsigned m_NI, unsigned m_NO, int has_state>
class netlist_factory_truthtable_t : public netlist_base_factory_truthtable_t
{
	NETLIST_PREVENT_COPYING(netlist_factory_truthtable_t)
public:
	ATTR_COLD netlist_factory_truthtable_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: netlist_base_factory_truthtable_t(name, classname, def_param) { }

	ATTR_COLD netlist_device_t *Create()
	{
		typedef nld_truthtable_t<m_NI, m_NO, has_state> tt_type;
		netlist_device_t *r = palloc(tt_type, &m_ttbl, m_desc);
		//r->init(setup, name);
		return r;
	}
private:
	typename nld_truthtable_t<m_NI, m_NO, has_state>::truthtable_t m_ttbl;
};

netlist_base_factory_truthtable_t *nl_tt_factory_create(const unsigned ni, const unsigned no,
		const unsigned has_state,
		const pstring &name, const pstring &classname,
		const pstring &def_param);

#define TRUTHTABLE_START(_name, _in, _out, _has_state, _def_params) \
	{ \
	netlist_base_factory_truthtable_t *ttd = nl_tt_factory_create(_in, _out, _has_state, \
			# _name, # _name, "+" _def_params);

#define TT_HEAD(_x) \
	ttd->m_desc.add(_x);

#define TT_LINE(_x) \
	ttd->m_desc.add(_x);

#define TRUTHTABLE_END() \
	setup.factory().register_device(ttd); \
	}


#endif /* NLD_TRUTHTABLE_H_ */
