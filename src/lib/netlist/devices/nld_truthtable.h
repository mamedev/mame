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
#include <cstdint>

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
	template<> struct uint_for_size<1> { typedef uint8_t type; };
	template<> struct uint_for_size<2> { typedef uint16_t type; };
	template<> struct uint_for_size<4> { typedef uint32_t type; };
	template<> struct uint_for_size<8> { typedef uint64_t type; };

	struct packed_int
	{
		template<typename C>
		packed_int(C *data)
		: m_data(data)
		, m_size(sizeof(C))
		{}

		void set(const size_t pos, const uint64_t val)
		{
			switch (m_size)
			{
				case 1: ((uint8_t *) m_data)[pos] = val; break;
				case 2: ((uint16_t *) m_data)[pos] = val; break;
				case 4: ((uint32_t *) m_data)[pos] = val; break;
				case 8: ((uint64_t *) m_data)[pos] = val; break;
				default: { }
			}
		}

		UINT64 operator[] (size_t pos) const
		{
			switch (m_size)
			{
				case 1: return ((uint8_t *) m_data)[pos]; break;
				case 2: return ((uint16_t *) m_data)[pos]; break;
				case 4: return ((uint32_t *) m_data)[pos]; break;
				case 8: return ((uint64_t *) m_data)[pos]; break;
				default:
					return 0; //should never happen
			}
		}

		uint64_t adjust(uint64_t val) const
		{
			switch (m_size)
			{
				case 1: return ((uint8_t) val); break;
				case 2: return ((uint16_t) val); break;
				case 4: return ((uint32_t) val); break;
				case 8: return ((uint64_t) val); break;
				default:
					return 0; //should never happen
			}
		}
	private:
		void *m_data;
		size_t m_size;
	};

	struct truthtable_desc_t
	{
		truthtable_desc_t(int NO, int NI, int has_state, bool *initialized,
				packed_int outs, UINT8 *timing, netlist_time *timing_nt)
		: m_NO(NO), m_NI(NI), /*m_has_state(has_state),*/ m_initialized(initialized),
			m_outs(outs), m_timing(timing), m_timing_nt(timing_nt),
			m_num_bits(m_NI + has_state * (m_NI + m_NO)),
			m_size(1 << (m_num_bits))
		{
		}

		void setup(const plib::pstring_vector_t &desc, UINT32 disabled_ignore);

	private:
		void help(unsigned cur, plib::pstring_vector_t list,
				uint64_t state, uint64_t val, std::vector<uint8_t> &timing_index);
		static unsigned count_bits(uint64_t v);
		static uint64_t set_bits(uint64_t v, uint64_t b);
		uint64_t get_ignored_simple(uint64_t i);
		uint64_t get_ignored_extended(uint64_t i);

		unsigned m_NO;
		unsigned m_NI;
		bool *m_initialized;
		packed_int m_outs;
		uint8_t  *m_timing;
		netlist_time *m_timing_nt;

		/* additional values */

		const std::size_t m_num_bits;
		const std::size_t m_size;

	};

	template<unsigned m_NI, unsigned m_NO, int m_has_state>
	NETLIB_OBJECT(truthtable_t)
	{
	private:
		family_setter_t m_fam;
	public:

		static const int m_num_bits = m_NI + m_has_state * (m_NI + m_NO);
		static const int m_size = (1 << (m_num_bits));

		struct truthtable_t
		{
			truthtable_t()
			: m_initialized(false)
			{}
			bool m_initialized;
			typename uint_for_size<need_bytes_for_bits<m_NO + m_NI>::value>::type m_outs[m_size];
			UINT8  m_timing[m_size * m_NO];
			netlist_time m_timing_nt[16];
		};

		template <class C>
		nld_truthtable_t(C &owner, const pstring &name, const logic_family_desc_t *fam,
				truthtable_t *ttp, const char *desc[])
		: device_t(owner, name)
		, m_fam(*this, fam)
		, m_last_state(0)
		, m_ign(0)
		, m_active(1)
		, m_ttp(ttp)
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
				truthtable_t *ttp, const plib::pstring_vector_t &desc)
		: device_t(owner, name)
		, m_fam(*this, fam)
		, m_last_state(0)
		, m_ign(0)
		, m_active(1)
		, m_ttp(ttp)
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

			for (std::size_t i=0; i < m_NI; i++)
			{
				inout[i] = inout[i].trim();
				m_I.emplace(i, *this, inout[i]);
			}
			for (std::size_t i=0; i < m_NO; i++)
			{
				out[i] = out[i].trim();
				m_Q.emplace(i, *this, out[i]);
			}
			// Connect output "Q" to input "_Q" if this exists
			// This enables timed state without having explicit state ....
			UINT32 disabled_ignore = 0;
			for (std::size_t i=0; i < m_NO; i++)
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

			truthtable_desc_t desc(m_NO, m_NI, m_has_state,
					&m_ttp->m_initialized, packed_int(m_ttp->m_outs), m_ttp->m_timing, m_ttp->m_timing_nt);

			desc.setup(m_desc, disabled_ignore * 0);

	#if 0
			printf("%s\n", name().cstr());
			for (int j=0; j < m_size; j++)
				printf("%05x %04x %04x %04x\n", j, m_ttp.m_outs[j] & ((1 << m_NO)-1),
						m_ttp.m_outs[j] >> m_NO, m_ttp.m_timing[j * m_NO + 0]);
			for (int k=0; m_ttp.m_timing_nt[k] != netlist_time::zero(); k++)
				printf("%d %f\n", k, m_ttp.m_timing_nt[k].as_double() * 1000000.0);
	#endif
			save(NLNAME(m_last_state));
			save(NLNAME(m_ign));
			save(NLNAME(m_active));
		}

		NETLIB_RESETI()
		{
			m_active = 0;
			m_ign = 0;
			for (std::size_t i = 0; i < m_NI; i++)
				m_I[i].activate();
			for (std::size_t i=0; i<m_NO;i++)
				if (this->m_Q[i].net().num_cons()>0)
					m_active++;
			m_last_state = 0;
		}

		NETLIB_UPDATEI()
		{
			process<true>();
		}

	public:
		void inc_active() override
		{
			nl_assert(netlist().use_deactivate());
			if (m_NI > 1 && m_has_state == 0)
				if (++m_active == 1)
				{
					process<false>();
				}
		}

		void dec_active() override
		{
			nl_assert(netlist().use_deactivate());
			/* FIXME:
			 * Based on current measurements there is no point to disable
			 * 1 input devices. This should actually be a parameter so that we
			 * can decide for each individual gate whether it is benefitial to
			 * ignore deactivation.
			 */
			if (m_NI > 1 && m_has_state == 0)
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

			uint64_t state = 0;
			if (m_NI > 1 && !m_has_state)
			{
				auto ign = m_ign;
				if (!doOUT)
					for (std::size_t i = 0; i < m_NI; i++)
					{
						m_I[i].activate();
						state |= (INPLOGIC(m_I[i]) << i);
						mt = std::max(this->m_I[i].net().time(), mt);
					}
				else
					for (std::size_t i = 0; i < m_NI; ign >>= 1, i++)
					{
						if ((ign & 1))
							m_I[i].activate();
						state |= (INPLOGIC(m_I[i]) << i);
					}
			}
			else
			{
				if (!doOUT)
					for (std::size_t i = 0; i < m_NI; i++)
					{
						state |= (INPLOGIC(m_I[i]) << i);
						mt = std::max(this->m_I[i].net().time(), mt);
					}
				else
					for (std::size_t i = 0; i < m_NI; i++)
						state |= (INPLOGIC(m_I[i]) << i);
			}
			auto nstate = state;

			if (m_has_state)
				nstate |= (m_last_state << m_NI);

			const auto outstate = m_ttp->m_outs[nstate];
			const auto out = outstate & ((1 << m_NO) - 1);

			m_ign = outstate >> m_NO;

			const auto timebase = nstate * m_NO;

			if (doOUT)
			{
				for (std::size_t i = 0; i < m_NO; i++)
					OUTLOGIC(m_Q[i], (out >> i) & 1, m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);
			}
			else
				for (std::size_t i = 0; i < m_NO; i++)
					m_Q[i].net().set_Q_time((out >> i) & 1, mt + m_ttp->m_timing_nt[m_ttp->m_timing[timebase + i]]);

			if (m_has_state)
				m_last_state = (state << m_NO) | out;

			if (m_NI > 1 && !m_has_state)
			{
				auto ign(m_ign);
				for (std::size_t i = 0; ign != 0; ign >>= 1, i++)
					if (ign & 1)
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

	plib::owned_ptr<netlist_base_factory_truthtable_t> nl_tt_factory_create(const unsigned ni, const unsigned no,
			const unsigned has_state,
			const pstring &name, const pstring &classname,
			const pstring &def_param);

	} //namespace devices
} // namespace netlist



#endif /* NLD_TRUTHTABLE_H_ */
