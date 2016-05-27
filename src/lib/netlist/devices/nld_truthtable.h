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

#include <new>

#include "nl_base.h"
#include "nl_factory.h"
#include "plib/plists.h"

#define NETLIB_TRUTHTABLE(cname, nIN, nOUT, state)                               \
	class NETLIB_NAME(cname) : public nld_truthtable_t<nIN, nOUT, state>         \
	{                                                                           \
	public:                                                                     \
		template <class C>                                                      \
		NETLIB_NAME(cname)(C &owner, const pstring &name)                        \
		: nld_truthtable_t<nIN, nOUT, state>(owner, name, nullptr, &m_ttbl, m_desc) { }   \
	private:                                                                    \
		static truthtable_t m_ttbl;                                             \
		static const char *m_desc[];                                            \
	}

#define TRUTHTABLE_START(cname, in, out, has_state, def_params) \
	{ \
	auto ttd = netlist::devices::nl_tt_factory_create(in, out, has_state, \
			# cname, # cname, "+" def_params);

#define TT_HEAD(x) \
	ttd->m_desc.push_back(x);

#define TT_LINE(x) \
	ttd->m_desc.push_back(x);

#define TT_FAMILY(x) \
	ttd->m_family = setup.family_from_model(x);

#define TRUTHTABLE_END() \
	setup.factory().register_device(std::move(ttd)); \
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

	void setup(const plib::pstring_vector_t &desc, UINT32 disabled_ignore);

private:
	void help(unsigned cur, plib::pstring_vector_t list,
			UINT64 state,UINT16 val, plib::array_t<UINT8> &timing_index);
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
NETLIB_OBJECT(truthtable_t)
{
private:
	family_setter_t m_fam;
public:

	static const int m_num_bits = m_NI + has_state * (m_NI + m_NO);
	static const int m_size = (1 << (m_num_bits));

	struct truthtable_t
	{
		truthtable_t()
		: m_initialized(false)
		, m_desc(m_NO, m_NI, has_state, &m_initialized, m_outs, m_timing, m_timing_nt) {}
		bool m_initialized;
		UINT32 m_outs[m_size];
		UINT8  m_timing[m_size * m_NO];
		netlist_time m_timing_nt[16];
		truthtable_desc_t m_desc;
	};

	template <class C>
	nld_truthtable_t(C &owner, const pstring &name, const logic_family_desc_t *fam,
			truthtable_t *ttbl, const char *desc[])
	: device_t(owner, name)
	, m_fam(*this, fam)
	, m_last_state(0)
	, m_ign(0)
	, m_active(1)
	, m_ttp(ttbl)
	{
		while (*desc != nullptr && **desc != 0 )
			{
				m_desc.push_back(*desc);
				desc++;
			}
		startxx();
	}

	template <class C>
	nld_truthtable_t(C &owner, const pstring &name, const logic_family_desc_t *fam,
			truthtable_t *ttbl, const plib::pstring_vector_t &desc)
	: device_t(owner, name)
	, m_fam(*this, fam)
	, m_last_state(0)
	, m_ign(0)
	, m_active(1)
	, m_ttp(ttbl)
	{
		m_desc = desc;
		startxx();
	}

	void startxx()
	{
		pstring header = m_desc[0];

		plib::pstring_vector_t io(header,"|");
		// checks
		nl_assert_always(io.size() == 2, "too many '|'");
		plib::pstring_vector_t inout(io[0], ",");
		nl_assert_always(inout.size() == m_num_bits, "bitcount wrong");
		plib::pstring_vector_t out(io[1], ",");
		nl_assert_always(out.size() == m_NO, "output count wrong");

		for (unsigned i=0; i < m_NI; i++)
		{
			new (&m_I[i]) logic_input_t();
			inout[i] = inout[i].trim();
			enregister(inout[i], m_I[i]);
		}
		for (unsigned i=0; i < m_NO; i++)
		{
			new (&m_Q[i]) logic_output_t();
			out[i] = out[i].trim();
			enregister(out[i], m_Q[i]);
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

	NETLIB_RESETI()
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

	NETLIB_UPDATEI()
	{
		process<true>();
	}

public:
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
		if (m_NI > 1 && has_state == 0)
			if (--m_active == 0)
			{
				for (unsigned i = 0; i< m_NI; i++)
					m_I[i].inactivate();
				m_ign = (1<<m_NI)-1;
			}
	}

	//logic_input_t m_I[m_NI];
	//logic_output_t m_Q[m_NO];
	plib::uninitialised_array_t<logic_input_t, m_NI> m_I;
	plib::uninitialised_array_t<logic_output_t, m_NO> m_Q;

protected:

private:

	template<bool doOUT>
	inline void process()
	{
		netlist_time mt = netlist_time::zero;

		UINT32 state = 0;
		if (m_NI > 1 && !has_state)
			for (unsigned i = 0; i < m_NI; i++)
			{
				if (!doOUT || (m_ign & (1<<i)))
					m_I[i].activate();
			}

		if (!doOUT)
			for (unsigned i = 0; i < m_NI; i++)
			{
				state |= (INPLOGIC(m_I[i]) << i);
				mt = std::max(this->m_I[i].net().time(), mt);
			}
		else
			for (unsigned i = 0; i < m_NI; i++)
				state |= (INPLOGIC(m_I[i]) << i);

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

		if (m_NI > 1 && !has_state)
		{
			for (unsigned i = 0; i < m_NI; i++)
				if (m_ign & (1 << i))
					m_I[i].inactivate();
		}
	}

	UINT32 m_last_state;
	UINT32 m_ign;
	INT32 m_active;

	truthtable_t *m_ttp;
	plib::pstring_vector_t m_desc;
};

class netlist_base_factory_truthtable_t : public base_factory_t
{
	P_PREVENT_COPYING(netlist_base_factory_truthtable_t)
public:
	netlist_base_factory_truthtable_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: base_factory_t(name, classname, def_param), m_family(family_TTL())
	{}

	virtual ~netlist_base_factory_truthtable_t()
	{
	}

	plib::pstring_vector_t m_desc;
	const logic_family_desc_t *m_family;
};


template<unsigned m_NI, unsigned m_NO, int has_state>
class netlist_factory_truthtable_t : public netlist_base_factory_truthtable_t
{
	P_PREVENT_COPYING(netlist_factory_truthtable_t)
public:
	netlist_factory_truthtable_t(const pstring &name, const pstring &classname,
			const pstring &def_param)
	: netlist_base_factory_truthtable_t(name, classname, def_param) { }

	plib::owned_ptr<device_t> Create(netlist_t &anetlist, const pstring &name) override
	{
		typedef nld_truthtable_t<m_NI, m_NO, has_state> tt_type;
		return plib::owned_ptr<device_t>::Create<tt_type>(anetlist, name, m_family, &m_ttbl, m_desc);
	}
private:
	typename nld_truthtable_t<m_NI, m_NO, has_state>::truthtable_t m_ttbl;
};

plib::owned_ptr<netlist_base_factory_truthtable_t> nl_tt_factory_create(const unsigned ni, const unsigned no,
		const unsigned has_state,
		const pstring &name, const pstring &classname,
		const pstring &def_param);

NETLIB_NAMESPACE_DEVICES_END()



#endif /* NLD_TRUTHTABLE_H_ */
