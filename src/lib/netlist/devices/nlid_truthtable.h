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

#include "../nl_setup.h"
#include "../nl_base.h"
#include "../plib/putil.h"

#define NETLIB_TRUTHTABLE(cname, nIN, nOUT)                                    \
	class NETLIB_NAME(cname) : public nld_truthtable_t<nIN, nOUT>              \
	{                                                                          \
	public:                                                                    \
		template <class C>                                                     \
		NETLIB_NAME(cname)(C &owner, const pstring &name)                      \
		: nld_truthtable_t<nIN, nOUT>(owner, name, family_TTL(), &m_ttbl, m_desc) { }   \
	private:                                                                   \
		static truthtable_t m_ttbl;                                            \
		static std::vector<pstring> m_desc;                                    \
	}


namespace netlist
{
	namespace devices
	{

#if 0
	template<unsigned bits> struct uint_for_size { typedef uint_least32_t type; };
	template<unsigned bits>
	struct need_bytes_for_bits
	{
		enum { value = 4 };
	};
#else
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
	template<> struct uint_for_size<1> { typedef uint_least8_t  type; };
	template<> struct uint_for_size<2> { typedef uint_least16_t type; };
	template<> struct uint_for_size<4> { typedef uint_least32_t type; };
	template<> struct uint_for_size<8> { typedef uint_least64_t type; };
#endif

	template<std::size_t m_NI, std::size_t m_NO>
	NETLIB_OBJECT(truthtable_t)
	{
	private:
		detail::family_setter_t m_fam;
	public:

		typedef typename uint_for_size<need_bytes_for_bits<m_NO + m_NI>::value>::type type_t;

		static constexpr const std::size_t m_num_bits = m_NI;
		static constexpr const std::size_t m_size = (1 << (m_num_bits));
		static constexpr const type_t m_outmask = ((1 << m_NO) - 1);
		static constexpr const std::size_t m_min_devices_for_deactivate = 2;

		struct truthtable_t
		{
			truthtable_t()
			: m_initialized(false)
			{}
			bool m_initialized;
			type_t m_outs[m_size];
			uint_least8_t m_timing[m_size * m_NO];
			netlist_time m_timing_nt[16];
		};

		template <class C>
		nld_truthtable_t(C &owner, const pstring &name,
				const logic_family_desc_t *fam,
				truthtable_t &ttp, const std::vector<pstring> &desc)
		: device_t(owner, name)
		, m_fam(*this, fam)
		, m_ign(*this, "m_ign", 0)
		, m_active(*this, "m_active", 1)
		, m_ttp(ttp)
		{
			init(desc);
		}

		void init(const std::vector<pstring> &desc);

		NETLIB_RESETI()
		{
			m_active = 0;
			m_ign = 0;
			for (auto &i : m_I)
				i.activate();
			for (auto &q : m_Q)
				if (q.has_net() && q.net().num_cons() > 0)
					m_active++;
		}

		NETLIB_UPDATEI()
		{
			process<true>();
		}

		void inc_active() NL_NOEXCEPT override
		{
			if (m_NI >= m_min_devices_for_deactivate)
				if (++m_active == 1)
				{
					process<false>();
				}
		}

		void dec_active() NL_NOEXCEPT override
		{
			/* FIXME:
			 * Based on current measurements there is no point to disable
			 * 1 input devices. This should actually be a parameter so that we
			 * can decide for each individual gate whether it is beneficial to
			 * ignore deactivation.
			 */
			if (m_NI >= m_min_devices_for_deactivate)
				if (--m_active == 0)
				{
					for (std::size_t i = 0; i< m_NI; i++)
						m_I[i].inactivate();
					m_ign = (1<<m_NI)-1;
				}
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
			if (m_NI >= m_min_devices_for_deactivate)
			{
				type_t ign(m_ign);
				if (!doOUT)
					for (std::size_t i = 0; i < m_NI; i++)
					{
						m_I[i].activate();
						nstate |= (m_I[i]() ? (1 << i) : 0);
						mt = std::max(this->m_I[i].net().time(), mt);
					}
				else
					for (std::size_t i = 0; i < m_NI; i++)
					{
						if ((ign & 1))
							m_I[i].activate();
						nstate |= (m_I[i]() ? (1 << i) : 0);
						ign >>= 1;
					}
			}
			else
			{
				if (!doOUT)
					for (std::size_t i = 0; i < m_NI; i++)
					{
						nstate |= (m_I[i]() ? (1 << i) : 0);
						mt = std::max(this->m_I[i].net().time(), mt);
					}
				else
					for (std::size_t i = 0; i < m_NI; i++)
						nstate |= (m_I[i]() ? (1 << i) : 0);
			}

			const type_t outstate(m_ttp.m_outs[nstate]);
			type_t out(outstate & m_outmask);

			m_ign = outstate >> m_NO;

			const std::size_t timebase(nstate * m_NO);
			const auto *t(&m_ttp.m_timing[timebase]);
			const auto *tim = m_ttp.m_timing_nt;

			if (doOUT)
				for (std::size_t i = 0; i < m_NO; out >>= 1, ++i)
					m_Q[i].push(out & 1, tim[t[i]]);
			else
				for (std::size_t i = 0; i < m_NO; out >>= 1, ++i)
					m_Q[i].set_Q_time(out & 1, mt + tim[t[i]]);

			if (m_NI >= m_min_devices_for_deactivate)
			{
				type_t ign(m_ign);
				for (auto I = m_I.begin(); ign != 0; ign >>= 1, ++I)
					if (ign & 1)
						I->inactivate();
			}
		}

		/* FIXME: check width */
		state_var<type_t>   m_ign;
		state_var_s32       m_active;
		const truthtable_t &m_ttp;
	};

	class netlist_base_factory_truthtable_t : public factory::element_t
	{
	public:
		netlist_base_factory_truthtable_t(const pstring &name, const pstring &classname,
				const pstring &def_param, const pstring &sourcefile);

		virtual ~netlist_base_factory_truthtable_t();

		std::vector<pstring> m_desc;
		const logic_family_desc_t *m_family;
	};

	void tt_factory_create(setup_t &setup, tt_desc &desc, const pstring &sourcefile);

	} //namespace devices
} // namespace netlist



#endif /* NLID_TRUTHTABLE_H_ */
