// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_truthtable.c
 *
 */

#include  "nlid_truthtable.h"
#include "netlist/nl_setup.h"
#include "plib/palloc.h"
#include "plib/plists.h"

#include <bitset>

namespace netlist
{
	namespace devices
	{

	// ----------------------------------------------------------------------------------------
	// int compatible bitset ....
	// ----------------------------------------------------------------------------------------

	template <typename T>
	struct sbitset
	{
		using type = T;

		sbitset() : m_bs(0) { }
		/* explicit */ sbitset(T v) : m_bs(v) { }

		sbitset &set() { *this = all_bits(); return *this; }
		sbitset &set(const std::size_t bit) { m_bs |= (static_cast<T>(1) << bit); return *this; }
		sbitset &reset() { *this = no_bits(); return *this; }
		sbitset &reset(const std::size_t bit) { m_bs &= ~(static_cast<T>(1) << bit); return *this; }

		sbitset invert() const { return sbitset(~m_bs); }
		std::size_t count()
		{
			std::size_t ret = 0;
			for (T v = m_bs; v != 0; v = v >> 1)
			{
				ret += (v & 1);
			}
			return ret;
		}
		constexpr bool test(const std::size_t bit) const { return ((m_bs >> bit) & 1) == 1; }

		operator T&() { return m_bs; }
		operator const T&() const { return m_bs; }
		constexpr T as_uint() const { return m_bs; }

		constexpr bool all() const { return *this == all_bits(); }

		/*
		 * And all bits set with compressed bits from b
		 *
		 * Example: b = {b3,b2,b1,b0}
		 *          v = {v7, 0, v5, 0, v3, v2, 0, 0}
		 * Returns {v7 & b3, 0, v5 & b2, 0, v3 & b1, v2 & b0, 0, 0}
		 */

		sbitset expand_and(sbitset b) const
		{
			sbitset ret;
			T v( m_bs);

			for (size_t i = 0; v != 0; v = v >> 1, ++i)
			{
				if (v & 1)
				{
					if (b.test(0))
						ret.set(i);
					b = b >> 1;
				}
			}
			return ret;
		}

		static constexpr const sbitset all_bits() noexcept { return sbitset(~static_cast<T>(0)); }
		static constexpr const sbitset no_bits() noexcept{ return sbitset(static_cast<T>(0)); }
	private:
		T m_bs;
	};
	// ----------------------------------------------------------------------------------------
	// Truthtable parsing ....
	// ----------------------------------------------------------------------------------------

	//static const uint_least64_t all_set = ~(static_cast<uint_least64_t>(0));

	using tt_bitset = sbitset<uint_least64_t>;

	struct packed_int
	{
		packed_int(void *data, std::size_t bits)
		: m_data(data)
		, m_size(bits)
		{}

		void set(const size_t pos, const uint_least64_t val)
		{
			switch (m_size)
			{
				case 8: static_cast<uint_least8_t  *>(m_data)[pos] = static_cast<uint_least8_t>(val); break;
				case 16: static_cast<uint_least16_t *>(m_data)[pos] = static_cast<uint_least16_t>(val); break;
				case 32: static_cast<uint_least32_t *>(m_data)[pos] = static_cast<uint_least32_t>(val); break;
				case 64: static_cast<uint_least64_t *>(m_data)[pos] = static_cast<uint_least64_t>(val); break;
				default: { }
			}
		}

		uint_least64_t operator[] (size_t pos) const
		{
			switch (m_size)
			{
				case 8: return static_cast<uint_least8_t  *>(m_data)[pos];
				case 16: return static_cast<uint_least16_t *>(m_data)[pos];
				case 32: return static_cast<uint_least32_t *>(m_data)[pos];
				case 64: return static_cast<uint_least64_t *>(m_data)[pos];
				default:
					return 0; //should never happen
			}
		}

		uint_least64_t mask() const
		{
			switch (m_size)
			{
				case 8: return static_cast<uint_least8_t>(-1);
				case 16: return static_cast<uint_least16_t>(-1);
				case 32: return static_cast<uint_least32_t>(-1);
				case 64: return static_cast<uint_least64_t>(-1);
				default:
					return 0; //should never happen
			}
		}
	private:
		void *m_data;
		size_t m_size;
	};

	struct truthtable_parser
	{
		truthtable_parser(unsigned NO, unsigned NI, bool *initialized,
				packed_int outs, uint_least8_t *timing, netlist_time *timing_nt)
		: m_NO(NO), m_NI(NI),  m_initialized(initialized),
			m_out_state(outs), m_timing(timing), m_timing_nt(timing_nt),
			m_num_bits(m_NI),
			m_size(1 << (m_num_bits))
		{
		}

		void parse(const std::vector<pstring> &desc);

	private:
		void parseline(unsigned cur, std::vector<pstring> list,
				tt_bitset state, uint_least64_t val, std::vector<uint_least8_t> &timing_index);

		tt_bitset calculate_ignored_inputs(tt_bitset i);

		unsigned m_NO;
		unsigned m_NI;
		bool *m_initialized;
		packed_int m_out_state;
		uint_least8_t  *m_timing;
		netlist_time *m_timing_nt;

		/* additional values */

		const std::size_t m_num_bits;
		const std::size_t m_size;

	};

	// ----------------------------------------------------------------------------------------
	// Truthtable class ....
	// ----------------------------------------------------------------------------------------

	template<std::size_t m_NI, std::size_t m_NO>
	void NETLIB_NAME(truthtable_t)<m_NI, m_NO>::init(const std::vector<pstring> &desc)
	{
		set_hint_deactivate(true);

		pstring header = desc[0];

		std::vector<pstring> io(plib::psplit(header,"|"));
		// checks
		nl_assert_always(io.size() == 2, "too many '|'");
		std::vector<pstring> inout(plib::psplit(io[0], ","));
		nl_assert_always(inout.size() == m_num_bits, "bitcount wrong");
		std::vector<pstring> outputs(plib::psplit(io[1], ","));
		nl_assert_always(outputs.size() == m_NO, "output count wrong");

		for (std::size_t i=0; i < m_NI; i++)
		{
			inout[i] = plib::trim(inout[i]);
			m_I.emplace(i, *this, inout[i]);
		}
		for (std::size_t i=0; i < m_NO; i++)
		{
			outputs[i] = plib::trim(outputs[i]);
			m_Q.emplace(i, *this, outputs[i]);
			// Connect output "Q" to input "_Q" if this exists
			// This enables timed state without having explicit state ....
			pstring tmp = "_" + outputs[i];
			const std::size_t idx = plib::container::indexof(inout, tmp);
			if (idx != plib::container::npos)
				connect(m_Q[i], m_I[idx]);
		}

		m_ign = 0;
	}

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

		pool_owned_ptr<device_t> Create(netlist_state_t &anetlist, const pstring &name) override
		{
			using tt_type = nld_truthtable_t<m_NI, m_NO>;
			truthtable_parser desc_s(m_NO, m_NI, &m_ttbl.m_initialized,
					packed_int(m_ttbl.m_out_state.data(), sizeof(m_ttbl.m_out_state[0]) * 8),
					m_ttbl.m_timing_index.data(), m_ttbl.m_timing_nt.data());

			desc_s.parse(m_desc);

			/* update truthtable family definitions */
			if (m_family_name != "")
				m_family_desc = anetlist.setup().family_from_model(m_family_name);

			return pool().make_poolptr<tt_type>(anetlist, name, m_family_desc, m_ttbl, m_desc);
		}
	private:
		typename nld_truthtable_t<m_NI, m_NO>::truthtable_t m_ttbl;
	};

	tt_bitset truthtable_parser::calculate_ignored_inputs(tt_bitset state)
	{
		// Determine all inputs which may be ignored ...
		tt_bitset ignore = 0;
		for (std::size_t j=0; j<m_NI; j++)
		{
			// if changing the input directly doesn't change outputs we can ignore
			if (m_out_state[state] == m_out_state[tt_bitset(state).set(j)])
				ignore.set(j);
		}

		/* Check all permutations of ign
		 * We have to remove those where the ignored inputs
		 * may change the output
		 */

		tt_bitset bits = tt_bitset().set(ignore.count());

		std::vector<bool> t(bits);

		// loop over all combinations of bits set in ignore
		for (uint_least64_t  j=1; j < bits; j++)
		{
			tt_bitset tign = ignore.expand_and(j);
			t[j] = false;
			tt_bitset bitsk = tt_bitset().set(tign.count());

			// now loop over all combinations of the bits set currently set

			for (uint_least64_t k=0; k < bitsk; k++)
			{
				tt_bitset b = tign.expand_and(k);
				// will any of the inputs ignored change the output if changed?
				if (m_out_state[state] != m_out_state[(state & tign.invert()) | b])
				{
					t[j] = true;
					break;
				}
			}
		}

		/* find the ignore mask without potential for change with the most bits */

		size_t jb=0;
		tt_bitset jm=0;

		for (uint_least64_t j=1; j<bits; j++)
		{
			tt_bitset bj(j);
			size_t nb = bj.count();
			if ((t[j] == false) && (nb>jb))
			{
				jb = nb;
				jm = bj;
			}
		}
		return ignore.expand_and(jm);
	}

// ----------------------------------------------------------------------------------------
// parseline
// ----------------------------------------------------------------------------------------

void truthtable_parser::parseline(unsigned cur, std::vector<pstring> list,
		tt_bitset state, uint_least64_t val, std::vector<uint_least8_t> &timing_index)
{
	pstring elem = plib::trim(list[cur]);
	uint_least64_t start = 0;
	uint_least64_t end = 0;

	if (elem == "0")
	{
		start = 0;
		end = 0;
	}
	else if (elem == "1")
	{
		start = 1;
		end = 1;
	}
	else if (elem == "X")
	{
		start = 0;
		end = 1;
	}
	else
		nl_assert_always(false, "unknown input value (not 0, 1, or X)");
	for (uint_least64_t i = start; i <= end; i++)
	{
		tt_bitset nstate = state;
		if (i==1)
			nstate.set(cur);

		if (cur < m_num_bits - 1)
		{
			parseline(cur + 1, list, nstate, val, timing_index);
		}
		else
		{
			// cutoff previous inputs and outputs for ignore
			if (m_out_state[nstate] != m_out_state.mask() &&  m_out_state[nstate] != val)
				throw nl_exception(plib::pfmt("Error in truthtable: State {1:04} already set, {2} != {3}\n")
						.x(nstate.as_uint())(m_out_state[nstate])(val) );
			m_out_state.set(nstate, val);
			for (std::size_t j=0; j<m_NO; j++)
				m_timing[nstate * m_NO + j] = timing_index[j];
		}
	}
}

void truthtable_parser::parse(const std::vector<pstring> &truthtable)
{
	unsigned line = 0;

	if (*m_initialized)
		return;

	pstring ttline(truthtable[line]);
	line++;
	ttline = truthtable[line];
	line++;

	for (unsigned j=0; j < m_size; j++)
		m_out_state.set(j, tt_bitset::all_bits());

	for (int j=0; j < 16; j++)
		m_timing_nt[j] = netlist_time::zero();

	while (!(ttline == ""))
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

		tt_bitset val = 0;
		std::vector<uint_least8_t> tindex;

		/*
		 * FIXME: evaluation of outputs should be done in parseline to
		 *  enable the use of inputs for output values, i.e. "I1" or "~I1"
		 *  in addition to "0" and "1".
		 */
		for (unsigned j=0; j<m_NO; j++)
		{
			pstring outs = plib::trim(out[j]);
			if (outs == "1")
				val.set(j);
			else
				nl_assert_always(outs == "0", "Unknown value (not 0 or 1");
			// FIXME: error handling
			netlist_time t = netlist_time::from_nsec(plib::pstonum<std::int64_t>(plib::trim(times[j])));
			uint_least8_t k=0;
			while (m_timing_nt[k] != netlist_time::zero() && m_timing_nt[k] != t)
				k++;
			m_timing_nt[k] = t;
			tindex.push_back(k); //[j] = k;
		}

		parseline(0, inout, 0 , val, tindex);
		if (line < truthtable.size())
			ttline = truthtable[line];
		else
			ttline = "";
		line++;
	}

	// determine ignore mask by looping over all input combinations
	std::vector<tt_bitset> ign(m_size);
	for (tt_bitset &x : ign)
		x.set();

	for (uint_least64_t i=0; i < m_size; i++)
	{
		if (ign[i].all()) // not yet visited
		{
			tt_bitset tign = calculate_ignored_inputs(i);

			ign[i] = tign;

			/* don't need to recalculate similar ones */

			tt_bitset bitsk;
			bitsk.set(tign.count());

			for (uint_least64_t k=0; k < bitsk; k++)
			{
				tt_bitset b = tign.expand_and(k);
				ign[(i & tign.invert()) | b] = tign;
			}
		}
	}
	for (size_t i=0; i<m_size; i++)
	{
		if (m_out_state[i] == m_out_state.mask())
			throw nl_exception(plib::pfmt("truthtable: found element not set {1}\n").x(i) );
		m_out_state.set(i, m_out_state[i] | (ign[i] << m_NO));
	}
	*m_initialized = true;

}

netlist_base_factory_truthtable_t::netlist_base_factory_truthtable_t(const pstring &name, const pstring &classname,
		const pstring &def_param, const pstring &sourcefile)
: factory::element_t(name, classname, def_param, sourcefile), m_family_desc(family_TTL())
{
}

#define ENTRYY(n, m, s)    case (n * 100 + m): \
	{ using xtype = netlist_factory_truthtable_t<n, m>; \
		ret = plib::make_unique<xtype>(desc.name, desc.classname, desc.def_param, s); } break

#define ENTRY(n, s) ENTRYY(n, 1, s); ENTRYY(n, 2, s); ENTRYY(n, 3, s); \
					ENTRYY(n, 4, s); ENTRYY(n, 5, s); ENTRYY(n, 6, s); \
					ENTRYY(n, 7, s); ENTRYY(n, 8, s)

plib::unique_ptr<netlist_base_factory_truthtable_t> tt_factory_create(tt_desc &desc, const pstring &sourcefile)
{
	plib::unique_ptr<netlist_base_factory_truthtable_t> ret;

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
	ret->m_family_name = desc.family;

	return ret;
}

	} //namespace devices
} // namespace netlist
