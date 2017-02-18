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

#define NETLIB_TRUTHTABLE(cname, nIN, nOUT)                                     \
	class NETLIB_NAME(cname) : public nld_truthtable_t<nIN, nOUT>               \
	{                                                                           \
	public:                                                                     \
		template <class C>                                                      \
		NETLIB_NAME(cname)(C &owner, const pstring &name)                       \
		: nld_truthtable_t<nIN, nOUT>(owner, name, family_TTL(), &m_ttbl, m_desc) { }   \
	private:                                                                    \
		static truthtable_t m_ttbl;                                             \
		static const pstring m_desc[];                                         \
	}


namespace netlist
{
	namespace devices
	{

	template<unsigned bits>
	struct need_bytes_for_bits
	{
		enum { value =
			bits <= 8       ?  1 :
			bits <= 16      ?  2 :
			bits <= 32      ?  4 :
							   8
		};
	};

	template<unsigned bits> struct uint_for_size;
	template<> struct uint_for_size<1> { typedef uint_least8_t type; };
	template<> struct uint_for_size<2> { typedef uint_least16_t type; };
	template<> struct uint_for_size<4> { typedef uint_least32_t type; };
	template<> struct uint_for_size<8> { typedef uint_least64_t type; };

	template<unsigned m_NI, unsigned m_NO>
	NETLIB_OBJECT(truthtable_t)
	{
	private:
		detail::family_setter_t m_fam;
	public:

		static constexpr int m_num_bits = m_NI;
		static constexpr int m_size = (1 << (m_num_bits));

		struct truthtable_t
		{
			truthtable_t()
			: m_initialized(false)
			{}
			bool m_initialized;
			typename uint_for_size<need_bytes_for_bits<m_NO + m_NI>::value>::type m_outs[m_size];
			uint_least8_t m_timing[m_size * m_NO];
			netlist_time m_timing_nt[16];
		};

		template <class C>
		nld_truthtable_t(C &owner, const pstring &name, const logic_family_desc_t *fam,
				truthtable_t *ttp, const pstring *desc);

		template <class C>
		nld_truthtable_t(C &owner, const pstring &name, const logic_family_desc_t *fam,
				truthtable_t *ttp, const std::vector<pstring> &desc)
		: device_t(owner, name)
		, m_fam(*this, fam)
		, m_ign(*this, "m_ign", 0)
		, m_active(*this, "m_active", 1)
		, m_ttp(ttp)
		{
			m_desc = desc;
			init();
		}

		void init();

		NETLIB_RESETI()
		{
			m_active = 0;
			m_ign = 0;
			for (std::size_t i = 0; i < m_NI; i++)
				m_I[i].activate();
			for (std::size_t i=0; i<m_NO;i++)
				if (this->m_Q[i].has_net() && this->m_Q[i].net().num_cons()>0)
					m_active++;
		}

		NETLIB_UPDATEI()
		{
			process<true>();
		}

	public:
		void inc_active() NL_NOEXCEPT override
		{
			if (m_NI > 1)
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
			if (m_NI > 1)
				if (--m_active == 0)
				{
					for (std::size_t i = 0; i< m_NI; i++)
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
			netlist_time mt = netlist_time::zero();

			uint_least64_t state = 0;
			if (m_NI > 1)
			{
				auto ign = m_ign;
				if (!doOUT)
					for (std::size_t i = 0; i < m_NI; i++)
					{
						m_I[i].activate();
						state |= (m_I[i]() << i);
						mt = std::max(this->m_I[i].net().time(), mt);
					}
				else
					for (std::size_t i = 0; i < m_NI; ign >>= 1, i++)
					{
						if ((ign & 1))
							m_I[i].activate();
						state |= (m_I[i]() << i);
					}
			}
			else
			{
				if (!doOUT)
					for (std::size_t i = 0; i < m_NI; i++)
					{
						state |= (m_I[i]() << i);
						mt = std::max(this->m_I[i].net().time(), mt);
					}
				else
					for (std::size_t i = 0; i < m_NI; i++)
						state |= (m_I[i]() << i);
			}
			auto nstate = state;

			const auto outstate = m_ttp->m_outs[nstate];
			const auto out = outstate & ((1 << m_NO) - 1);

			m_ign = outstate >> m_NO;

			const auto timebase = nstate * m_NO;

			if (doOUT)
			{
				for (std::size_t i = 0; i < m_NO; i++)
					m_Q[i].push((out >> i) & 1, m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);
			}
			else
				for (std::size_t i = 0; i < m_NO; i++)
					m_Q[i].net().set_Q_time((out >> i) & 1, mt + m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);

			if (m_NI > 1)
			{
				auto ign(m_ign);
				for (std::size_t i = 0; ign != 0; ign >>= 1, i++)
					if (ign & 1)
						m_I[i].inactivate();
			}
		}

		/* FIXME: check width */
		state_var_u32       m_ign;
		state_var_s32       m_active;
		truthtable_t *      m_ttp;
		std::vector<pstring> m_desc;
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
