// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_truthtable.h
 *
 *  Created on: 19 Jun 2014
 *      Author: andre
 */

#ifndef NLID_TRUTHTABLE_H_
#define NLID_TRUTHTABLE_H_

#include "netlist/devices/nlid_system.h"
#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "plib/putil.h"

namespace netlist
{
namespace devices
{

	template<unsigned bits>
	struct need_bytes_for_bits
	{
		enum { value =
			bits <= 8       ?   1 :
			bits <= 16      ?   2 :
			bits <= 32      ?   4 :
								8
		};
	};

	template<unsigned bits> struct uint_for_size;
	template<> struct uint_for_size<1> { using type = uint_least8_t; };
	template<> struct uint_for_size<2> { using type = uint_least16_t; };
	template<> struct uint_for_size<4> { using type = uint_least32_t; };
	template<> struct uint_for_size<8> { using type = uint_least64_t; };

	template<std::size_t NUM, typename R>
	struct aa
	{
		template<typename T>
		R f(T &arr, const R ign)
		{
			R r = aa<NUM-1, R>().f(arr, ign);
			if (ign & (1 << (NUM-1)))
				arr[NUM-1].activate();
			return r | (arr[NUM-1]() << (NUM-1));
		}
	};

	template<typename R>
	struct aa<1, R>
	{
		template<typename T>
		R f(T &arr, const R ign)
		{
			if ((ign & 1))
				arr[0].activate();
			return arr[0]();
		}
	};

	template<std::size_t m_NI, std::size_t m_NO>
	NETLIB_OBJECT(truthtable_t)
	{
	private:
		detail::family_setter_t m_fam;
	public:

		using type_t = typename uint_for_size<need_bytes_for_bits<m_NO + m_NI>::value>::type;

		static constexpr const std::size_t m_num_bits = m_NI;
		static constexpr const std::size_t m_size = (1 << (m_num_bits));
		static constexpr const type_t m_outmask = ((1 << m_NO) - 1);

		struct truthtable_t
		{
			truthtable_t()
			: m_timing_index{0}
			, m_initialized(false)
			{}

			std::array<type_t, m_size> m_out_state;
			std::array<uint_least8_t, m_size * m_NO> m_timing_index;
			std::array<netlist_time, 16> m_timing_nt;
			bool m_initialized;
		};

		template <class C>
		nld_truthtable_t(C &owner, const pstring &name,
				const logic_family_desc_t *fam,
				truthtable_t &ttp, const std::vector<pstring> &desc)
		: device_t(owner, name)
		, m_fam(*this, fam)
		, m_ign(*this, "m_ign", 0)
		, m_ttp(ttp)
		/* FIXME: the family should provide the names of the power-terminals! */
		, m_power_pins(*this)
		{
			init(desc);
		}

		void init(const std::vector<pstring> &desc);

		NETLIB_RESETI()
		{
			int active_outputs = 0;
			m_ign = 0;
			for (auto &i : m_I)
				i.activate();
			for (auto &q : m_Q)
				if (q.has_net() && q.net().num_cons() > 0)
					active_outputs++;
			set_active_outputs(active_outputs);
		}

		NETLIB_UPDATEI()
		{
			process<true>();
		}

		void inc_active() NL_NOEXCEPT override
		{
			process<false>();
		}

		void dec_active() NL_NOEXCEPT override
		{
			for (std::size_t i = 0; i< m_NI; i++)
				m_I[i].inactivate();
			m_ign = (1<<m_NI)-1;
		}

		plib::uninitialised_array_t<logic_input_t, m_NI> m_I;
		plib::uninitialised_array_t<logic_output_t, m_NO> m_Q;

	protected:

	private:

		template<bool doOUT>
		void process()
		{
			netlist_time mt(netlist_time::zero());

			type_t nstate(0);
			type_t ign(m_ign);
#if 1
			if (!doOUT)
				for (std::size_t i = 0; i < m_NI; i++)
				{
					m_I[i].activate();
					//nstate |= (m_I[i]() ? (1 << i) : 0);
					nstate |= (m_I[i]() << i);
					mt = std::max(this->m_I[i].net().next_scheduled_time(), mt);
				}
			else
				for (std::size_t i = 0; i < m_NI; i++)
				{
					if ((ign & 1))
						m_I[i].activate();
					//nstate |= (m_I[i]() ? (1 << i) : 0);
					nstate |= (m_I[i]() << i);
					ign >>= 1;
				}
#else
			if (!doOUT)
			{
				nstate = aa<m_NI, type_t>().f(m_I, ~0);
				for (std::size_t i = 0; i < m_NI; i++)
					mt = std::max(this->m_I[i].net().time(), mt);
			}
			else
				nstate = aa<m_NI, type_t>().f(m_I, ign);
#endif

			const type_t outstate(m_ttp.m_out_state[nstate]);
			type_t out(outstate & m_outmask);

			m_ign = outstate >> m_NO;

			const std::size_t timebase(nstate * m_NO);
			const auto *t(&m_ttp.m_timing_index[timebase]);
			const auto *tim = m_ttp.m_timing_nt.data();

			if (doOUT)
				for (std::size_t i = 0; i < m_NO; out >>= 1, ++i)
					m_Q[i].push(out & 1, tim[t[i]]);
			else
				for (std::size_t i = 0; i < m_NO; out >>= 1, ++i)
					m_Q[i].set_Q_time(out & 1, mt + tim[t[i]]);

			ign = m_ign;
			for (auto I = m_I.begin(); ign != 0; ign >>= 1, ++I)
				if (ign & 1)
					I->inactivate();
		}

		/* FIXME: check width */
		state_var<type_t>   m_ign;
		const truthtable_t &m_ttp;
		/* FIXME: the family should provide the names of the power-terminals! */
		nld_power_pins m_power_pins;
	};

	class netlist_base_factory_truthtable_t : public factory::element_t
	{
	public:
		netlist_base_factory_truthtable_t(const pstring &name, const pstring &classname,
				const pstring &def_param, const pstring &sourcefile);

		std::vector<pstring> m_desc;
		pstring m_family_name;
		const logic_family_desc_t *m_family_desc;
	};

	/* the returned element is still missing a pointer to the family ... */
	plib::unique_ptr<netlist_base_factory_truthtable_t> tt_factory_create(tt_desc &desc, const pstring &sourcefile);

} //namespace devices
} // namespace netlist



#endif /* NLID_TRUTHTABLE_H_ */
