// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_truthtable.c
 *
 */

#include  "nlid_truthtable.h"
#include "../plib/plists.h"
#include "../nl_setup.h"
#include "../plib/palloc.h"

namespace netlist
{
	namespace devices
	{

	// ----------------------------------------------------------------------------------------
	// Truthtable description ....
	// ----------------------------------------------------------------------------------------

	struct packed_int
	{
		template<typename C>
		packed_int(C *data)
		: m_data(data)
		, m_size(sizeof(C))
		{}

		void set(const size_t pos, const uint_least64_t val)
		{
			switch (m_size)
			{
				case 1: static_cast<uint_least8_t  *>(m_data)[pos] = static_cast<uint_least8_t>(val); break;
				case 2: static_cast<uint_least16_t *>(m_data)[pos] = static_cast<uint_least16_t>(val); break;
				case 4: static_cast<uint_least32_t *>(m_data)[pos] = static_cast<uint_least32_t>(val); break;
				case 8: static_cast<uint_least64_t *>(m_data)[pos] = static_cast<uint_least64_t>(val); break;
				default: { }
			}
		}

		uint_least64_t operator[] (size_t pos) const
		{
			switch (m_size)
			{
				case 1: return static_cast<uint_least8_t  *>(m_data)[pos];
				case 2: return static_cast<uint_least16_t *>(m_data)[pos];
				case 4: return static_cast<uint_least32_t *>(m_data)[pos];
				case 8: return static_cast<uint_least64_t *>(m_data)[pos];
				default:
					return 0; //should never happen
			}
		}

		uint_least64_t adjust(uint_least64_t val) const
		{
			switch (m_size)
			{
				case 1: return static_cast<uint_least8_t >(val);
				case 2: return static_cast<uint_least16_t>(val);
				case 4: return static_cast<uint_least32_t>(val);
				case 8: return static_cast<uint_least64_t>(val);
				default:
					return 0; //should never happen
			}
		}
	private:
		void *m_data;
		size_t m_size;
	};

	static const uint_least64_t one64 = static_cast<uint_least64_t>(1);

	struct truthtable_desc_t
	{
		truthtable_desc_t(unsigned NO, unsigned NI, bool *initialized,
				packed_int outs, uint_least8_t *timing, netlist_time *timing_nt)
		: m_NO(NO), m_NI(NI),  m_initialized(initialized),
			m_outs(outs), m_timing(timing), m_timing_nt(timing_nt),
			m_num_bits(m_NI),
			m_size(1 << (m_num_bits))
		{
		}

		void setup(const std::vector<pstring> &desc, uint_least64_t disabled_ignore);

	private:
		void help(unsigned cur, std::vector<pstring> list,
				uint_least64_t state, uint_least64_t val, std::vector<uint_least8_t> &timing_index);
		static unsigned count_bits(uint_least64_t v);
		static uint_least64_t set_bits(uint_least64_t v, uint_least64_t b);
		uint_least64_t get_ignored_simple(uint_least64_t i);
		uint_least64_t get_ignored_extended(uint_least64_t i);

		unsigned m_NO;
		unsigned m_NI;
		bool *m_initialized;
		packed_int m_outs;
		uint_least8_t  *m_timing;
		netlist_time *m_timing_nt;

		/* additional values */

		const std::size_t m_num_bits;
		const std::size_t m_size;

	};

	template<unsigned m_NI, unsigned m_NO>
	template <class C>
	nld_truthtable_t<m_NI, m_NO>::nld_truthtable_t(C &owner, const pstring &name, const logic_family_desc_t *fam,
			truthtable_t *ttp, const pstring *desc)
	: device_t(owner, name)
	, m_fam(*this, fam)
	, m_ign(*this, "m_ign", 0)
	, m_active(*this, "m_active", 1)
	, m_ttp(ttp)
	{
		while (*desc != "" )
			{
				m_desc.push_back(*desc);
				desc++;
			}
		init();
	}

	template<unsigned m_NI, unsigned m_NO>
	void NETLIB_NAME(truthtable_t)<m_NI, m_NO>::init()
	{
		set_hint_deactivate(true);

		pstring header = m_desc[0];

		std::vector<pstring> io(plib::psplit(header,"|"));
		// checks
		nl_assert_always(io.size() == 2, "too many '|'");
		std::vector<pstring> inout(plib::psplit(io[0], ","));
		nl_assert_always(inout.size() == m_num_bits, "bitcount wrong");
		std::vector<pstring> out(plib::psplit(io[1], ","));
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
		uint_least64_t disabled_ignore = 0;
		for (std::size_t i=0; i < m_NO; i++)
		{
			pstring tmp = "_" + out[i];
			const std::size_t idx = plib::container::indexof(inout, tmp);
			if (idx != plib::container::npos)
			{
				connect(m_Q[i], m_I[idx]);
				// disable ignore for this inputs altogether.
				// FIXME: This shouldn't be necessary
				disabled_ignore |= (one64 << idx);
			}
		}

		m_ign = 0;

		truthtable_desc_t desc(m_NO, m_NI, &m_ttp->m_initialized,
				packed_int(m_ttp->m_outs),
				m_ttp->m_timing, m_ttp->m_timing_nt);

		desc.setup(m_desc, disabled_ignore * 0);
#if 0
		printf("%s\n", name().c_str());
		for (int j=0; j < m_size; j++)
			printf("%05x %04x %04x %04x\n", j, m_ttp->m_outs[j] & ((1 << m_NO)-1),
					m_ttp->m_outs[j] >> m_NO, m_ttp->m_timing[j * m_NO + 0]);
		for (int k=0; m_ttp->m_timing_nt[k] != netlist_time::zero(); k++)
			printf("%d %f\n", k, m_ttp->m_timing_nt[k].as_double() * 1000000.0);
#endif
	}

	// ----------------------------------------------------------------------------------------
	// Truthtable class ....
	// ----------------------------------------------------------------------------------------


	// ----------------------------------------------------------------------------------------
	// Truthtable factory ....
	// ----------------------------------------------------------------------------------------

	template<unsigned m_NI, unsigned m_NO>
	class netlist_factory_truthtable_t : public netlist_base_factory_truthtable_t
	{
	public:
		netlist_factory_truthtable_t(const pstring &name, const pstring &classname,
				const pstring &def_param, const pstring  &sourcefile)
		: netlist_base_factory_truthtable_t(name, classname, def_param, sourcefile)
		{ }

		plib::owned_ptr<device_t> Create(netlist_t &anetlist, const pstring &name) override
		{
			typedef nld_truthtable_t<m_NI, m_NO> tt_type;
			return plib::owned_ptr<device_t>::Create<tt_type>(anetlist, name, m_family, &m_ttbl, m_desc);
		}
	private:
		typename nld_truthtable_t<m_NI, m_NO>::truthtable_t m_ttbl;
	};

	static const uint_least64_t all_set = ~(static_cast<uint_least64_t>(0));

unsigned truthtable_desc_t::count_bits(uint_least64_t v)
{
	unsigned ret = 0;
	for (; v != 0; v = v >> 1)
	{
		ret += (v & 1);
	}
	return ret;
}

uint_least64_t truthtable_desc_t::set_bits(uint_least64_t v, uint_least64_t b)
{
	uint_least64_t ret = 0;
	for (size_t i = 0; v != 0; v = v >> 1, ++i)
	{
		if (v & 1)
		{
			ret |= ((b&1)<<i);
			b = b >> 1;
		}
	}
	return ret;
}

uint_least64_t truthtable_desc_t::get_ignored_simple(uint_least64_t i)
{
	uint_least64_t m_enable = 0;
	for (uint_least64_t j=0; j<m_size; j++)
	{
		if (m_outs[j] != m_outs[i])
		{
			m_enable |= (i ^ j);
		}
	}
	return m_enable ^ (m_size - 1);
}

uint_least64_t truthtable_desc_t::get_ignored_extended(uint_least64_t state)
{
	// Determine all inputs which may be ignored ...
	uint_least64_t ignore = 0;
	for (std::size_t j=0; j<m_NI; j++)
	{
		if (m_outs[state] == m_outs[state ^ (one64 << j)])
			ignore |= (one64 << j);
	}
	/* Check all permutations of ign
	 * We have to remove those where the ignored inputs
	 * may change the output
	 */
	uint_least64_t bits = (one64 << count_bits(ignore));
	std::vector<bool> t(bits);

	for (size_t j=1; j<bits; j++)
	{
		uint_least64_t tign = set_bits(ignore, j);
		t[j] = 0;
		uint_least64_t bitsk=(one64 << count_bits(tign));
		for (uint_least64_t k=0; k<bitsk; k++)
		{
			uint_least64_t b=set_bits(tign, k);
			if (m_outs[state] != m_outs[(state & ~tign) | b])
			{
				t[j] = 1;
				break;
			}
		}
	}
	size_t jb=0;
	size_t jm=0;
	for (size_t j=1; j<bits; j++)
	{
		size_t nb = count_bits(j);
		if ((t[j] == 0) && (nb>jb))
		{
			jb = nb;
			jm = j;
		}
	}
	return set_bits(ignore, jm);
}

// ----------------------------------------------------------------------------------------
// desc
// ----------------------------------------------------------------------------------------

void truthtable_desc_t::help(unsigned cur, std::vector<pstring> list,
		uint_least64_t state, uint_least64_t val, std::vector<uint_least8_t> &timing_index)
{
	pstring elem = list[cur].trim();
	uint_least64_t start = 0;
	uint_least64_t end = 0;

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
	for (uint_least64_t i = start; i <= end; i++)
	{
		const uint_least64_t nstate = state | (i << cur);

		if (cur < m_num_bits - 1)
		{
			help(cur + 1, list, nstate, val, timing_index);
		}
		else
		{
			// cutoff previous inputs and outputs for ignore
			if (m_outs[nstate] != m_outs.adjust(all_set) &&  m_outs[nstate] != val)
				nl_exception(plib::pfmt("Error in truthtable: State {1} already set, {2} != {3}\n")
						.x(nstate,"04")(m_outs[nstate])(val) );
			m_outs.set(nstate, val);
			for (unsigned j=0; j<m_NO; j++)
				m_timing[nstate * m_NO + j] = timing_index[j];
		}
	}
}

void truthtable_desc_t::setup(const std::vector<pstring> &truthtable, uint_least64_t disabled_ignore)
{
	unsigned line = 0;

	if (*m_initialized)
		return;

	pstring ttline = truthtable[line];
	line++;
	ttline = truthtable[line];
	line++;

	for (unsigned j=0; j < m_size; j++)
		m_outs.set(j, all_set);

	for (int j=0; j < 16; j++)
		m_timing_nt[j] = netlist_time::zero();

	while (!ttline.equals(""))
	{
		std::vector<pstring> io(plib::psplit(ttline,"|"));
		// checks
		nl_assert_always(io.size() == 3, "io.count mismatch");
		std::vector<pstring> inout(plib::psplit(io[0], ","));
		nl_assert_always(inout.size() == m_num_bits, "number of bits not matching");
		std::vector<pstring> out(plib::psplit(io[1], ","));
		nl_assert_always(out.size() == m_NO, "output count not matching");
		std::vector<pstring> times(plib::psplit(io[2], ","));
		nl_assert_always(times.size() == m_NO, "timing count not matching");

		uint_least64_t val = 0;
		std::vector<uint_least8_t> tindex;

		for (unsigned j=0; j<m_NO; j++)
		{
			pstring outs = out[j].trim();
			if (outs.equals("1"))
				val = val | (one64 << j);
			else
				nl_assert_always(outs.equals("0"), "Unknown value (not 0 or 1");
			netlist_time t = netlist_time::from_nsec(static_cast<unsigned long>(times[j].trim().as_long()));
			uint_least8_t k=0;
			while (m_timing_nt[k] != netlist_time::zero() && m_timing_nt[k] != t)
				k++;
			m_timing_nt[k] = t;
			tindex.push_back(k); //[j] = k;
		}

		help(0, inout, 0 , val, tindex);
		if (line < truthtable.size())
			ttline = truthtable[line];
		else
			ttline = "";
		line++;
	}

	// determine ignore
	std::vector<uint_least64_t> ign(m_size, all_set);

	for (uint_least64_t i=0; i<m_size; i++)
	{
		if (ign[i] == all_set)
		{
			uint_least64_t tign;
			if ((0))
			{
				tign = get_ignored_simple(i);
				ign[i] = tign;
			}
			else
			{
				tign = get_ignored_extended(i);

				ign[i] = tign;
				/* don't need to recalculate similar ones */
				uint_least64_t bitsk=(one64 << count_bits(tign));
				for (uint_least64_t k=0; k<bitsk; k++)
				{
					uint_least64_t b=set_bits(tign, k);
					ign[(i & ~tign) | b] = tign;
				}
			}
		}
	}
	for (size_t i=0; i<m_size; i++)
	{
		if (m_outs[i] == m_outs.adjust(all_set))
			throw nl_exception(plib::pfmt("truthtable: found element not set {1}\n").x(i) );
		m_outs.set(i, m_outs[i] | ((ign[i] & ~disabled_ignore)  << m_NO));;
	}
	*m_initialized = true;

}

netlist_base_factory_truthtable_t::netlist_base_factory_truthtable_t(const pstring &name, const pstring &classname,
		const pstring &def_param, const pstring &sourcefile)
: factory::element_t(name, classname, def_param, sourcefile), m_family(family_TTL())
{
}

netlist_base_factory_truthtable_t::~netlist_base_factory_truthtable_t()
{
}


#define ENTRYY(n, m, s)    case (n * 100 + m): \
	{ using xtype = netlist_factory_truthtable_t<n, m>; \
		ret = plib::palloc<xtype>(desc.name, desc.classname, desc.def_param, s); } break

#define ENTRY(n, s) ENTRYY(n, 1, s); ENTRYY(n, 2, s); ENTRYY(n, 3, s); \
					ENTRYY(n, 4, s); ENTRYY(n, 5, s); ENTRYY(n, 6, s)

void tt_factory_create(setup_t &setup, tt_desc &desc, const pstring &sourcefile)
{
	netlist_base_factory_truthtable_t *ret;

	switch (desc.ni * 100 + desc.no)
	{
		ENTRY(1, sourcefile);
		ENTRY(2, sourcefile);
		ENTRY(3, sourcefile);
		ENTRY(4, sourcefile);
		ENTRY(5, sourcefile);
		ENTRY(6, sourcefile);
		ENTRY(7, sourcefile);
		ENTRY(8, sourcefile);
		ENTRY(9, sourcefile);
		ENTRY(10, sourcefile);
		ENTRY(11, sourcefile);
		ENTRY(12, sourcefile);
		default:
			pstring msg = plib::pfmt("unable to create truthtable<{1},{2}>")(desc.ni)(desc.no);
			nl_assert_always(false, msg);
	}
	ret->m_desc = desc.desc;
	if (desc.family != "")
		ret->m_family = setup.family_from_model(desc.family);
	setup.factory().register_device(std::unique_ptr<netlist_base_factory_truthtable_t>(ret));
}

	} //namespace devices
} // namespace netlist
