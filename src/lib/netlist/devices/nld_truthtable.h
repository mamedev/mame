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

#include "nl_base.h"
#include "nl_factory.h"

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

#define TRUTHTABLE_START(_name, _in, _out, _has_state, _def_params) \
	{ \
	netlist::devices::netlist_base_factory_truthtable_t *ttd = netlist::devices::nl_tt_factory_create(_in, _out, _has_state, \
			# _name, # _name, "+" _def_params);

#define TT_HEAD(_x) \
	ttd->m_desc.add(_x);

#define TT_LINE(_x) \
	ttd->m_desc.add(_x);

#define TT_FAMILY(_x) \
	ttd->m_family = setup.family_from_model(_x);

#define TRUTHTABLE_END() \
	setup.factory().register_device(ttd); \
	}

NETLIB_NAMESPACE_DEVICES_START()

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

	void setup(const pstring_list_t &desc, UINT32 disabled_ignore);

private:
	void help(unsigned cur, pstring_list_t list,
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
class nld_truthtable_t : public device_t
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
	: device_t(), m_last_state(0), m_ign(0), m_active(1), m_ttp(ttbl)
	{
		while (*desc != NULL && **desc != 0 )
			{
				m_desc.add(*desc);
				desc++;
			}

	}

	nld_truthtable_t(truthtable_t *ttbl, const pstring_list_t &desc)
	: device_t(), m_last_state(0), m_ign(0), m_active(1), m_ttp(ttbl)
	{
		m_desc = desc;
	}

	virtual void start() override
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
			register_input(inout[i], m_I[i]);
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
				connect_late(m_Q[i], m_I[idx]);
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
		save(NLNAME(m_last_state));
		save(NLNAME(m_ign));
		save(NLNAME(m_active));
	}

	void reset() override
	{
		m_active = 0;
		m_ign = 0;
		for (unsigned i = 0; i < m_NI; i++)
			m_I[i].activate();
		for (unsigned i=0; i<m_NO;i++)
			if (this->m_Q[i].net().num_cons()>0)
				m_active++;
		m_last_state = 0;
	}

	template<bool doOUT>
	inline void process()
	{
		netlist_time mt = netlist_time::zero;

		UINT32 state = 0;
		for (unsigned i = 0; i < m_NI; i++)
		{
			if (!doOUT || (m_ign & (1<<i)))
				m_I[i].activate();
		}
		for (unsigned i = 0; i < m_NI; i++)
		{
			state |= (INPLOGIC(m_I[i]) << i);
			if (!doOUT)
				if (this->m_I[i].net().time() > mt)
					mt = this->m_I[i].net().time();
		}

		const UINT32 nstate = state | (has_state ? (m_last_state << m_NI) : 0);
		const UINT32 outstate = m_ttp->m_outs[nstate];
		const UINT32 out = outstate & ((1 << m_NO) - 1);
		m_ign = outstate >> m_NO;
		if (has_state)
			m_last_state = (state << m_NO) | out;

#if 0
		for (int i = 0; i < m_NI; i++)
			if (m_ign & (1 << i))
				m_I[i].inactivate();
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

		if (m_NI > 1 || has_state)
		{
			for (unsigned i = 0; i < m_NI; i++)
				if (m_ign & (1 << i))
					m_I[i].inactivate();
		}
	}

	ATTR_HOT void update() override
	{
		process<true>();
	}

	ATTR_HOT void inc_active() override
	{
		nl_assert(netlist().use_deactivate());
		if (has_state == 0)
			if (++m_active == 1)
			{
				process<false>();
			}
	}

	ATTR_HOT void dec_active() override
	{
		nl_assert(netlist().use_deactivate());
		/* FIXME:
		 * Based on current measurements there is no point to disable
		 * 1 input devices. This should actually be a parameter so that we
		 * can decide for each individual gate whether it is benefitial to
		 * ignore deactivation.
		 */
		if (m_NI < 2)
			return;
		else if (has_state == 0)
		{
			if (--m_active == 0)
			{
				for (unsigned i = 0; i< m_NI; i++)
					m_I[i].inactivate();
				m_ign = (1<<m_NI)-1;
			}
		}
	}

	logic_input_t m_I[m_NI];
	logic_output_t m_Q[m_NO];

private:

	UINT32 m_last_state;
	UINT32 m_ign;
	INT32 m_active;

	truthtable_t *m_ttp;
	pstring_list_t m_desc;
};

class netlist_base_factory_truthtable_t : public base_factory_t
{
	P_PREVENT_COPYING(netlist_base_factory_truthtable_t)
public:
	netlist_base_factory_truthtable_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: base_factory_t(name, classname, def_param), m_family(netlist_family_TTL)
	{}

	virtual ~netlist_base_factory_truthtable_t()
	{
		if (!m_family->m_is_static)
			pfree(m_family);
	}

	pstring_list_t m_desc;
	logic_family_desc_t *m_family;
};


template<unsigned m_NI, unsigned m_NO, int has_state>
class netlist_factory_truthtable_t : public netlist_base_factory_truthtable_t
{
	P_PREVENT_COPYING(netlist_factory_truthtable_t)
public:
	netlist_factory_truthtable_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: netlist_base_factory_truthtable_t(name, classname, def_param) { }

	device_t *Create() override
	{
		typedef nld_truthtable_t<m_NI, m_NO, has_state> tt_type;
		device_t *r = palloc(tt_type(&m_ttbl, m_desc));
		r->set_logic_family(m_family);
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

NETLIB_NAMESPACE_DEVICES_END()



#endif /* NLD_TRUTHTABLE_H_ */
