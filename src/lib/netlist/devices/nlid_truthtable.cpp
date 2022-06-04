// license:BSD-3-Clause
// copyright-holders:Couriersud

#include "plib/pstonum.h"
#include "plib/pstrutil.h"

#include "nl_base.h"
#include "nl_factory.h"
#include "nlid_truthtable.h"

#include <bitset>
#include <cstdint>
#include <vector>

namespace netlist::devices {

	template<std::size_t m_NI, std::size_t m_NO>
	class NETLIB_NAME(truth_table_t) : public device_t
	{
	public:

		using type_t = typename plib::fast_type_for_bits<m_NO + m_NI>::type;

		static constexpr const std::size_t m_num_bits = m_NI;
		static constexpr const std::size_t m_size = (1 << (m_num_bits));
		static constexpr const type_t m_outmask = ((1 << m_NO) - 1);

		struct truth_table_t
		{
			truth_table_t()
			: m_timing_index{0}
			{}

			std::array<type_t, m_size> m_out_state;
			std::array<uint_least8_t, m_size * m_NO> m_timing_index;
			std::array<netlist_time, 16> m_timing_nt;
		};

		template <class C>
		nld_truth_table_t(C &owner, const pstring &name,
				const pstring &model,
				truth_table_t &ttp, const std::vector<pstring> &desc)
		: device_t(owner, name, model)
#if USE_TT_ALTERNATIVE
		, m_state(*this, "m_state", 0)
#endif
		, m_ign(*this, "m_ign", 0)
		, m_ttp(ttp)
		/* FIXME: the family should provide the names of the power-terminals! */
		, m_power_pins(*this)
		{
			m_activate = activate_delegate(& NETLIB_NAME(truth_table_t) :: incdec_active, this);
			set_hint_deactivate(true);
			init(desc);
		}

	private:
		void init(const std::vector<pstring> &desc);

		NETLIB_RESETI()
		{
			int active_outputs = 0;
			m_ign = 0;
#if USE_TT_ALTERNATIVE
			m_state = 0;
#endif
			for (std::size_t i = 0; i < m_NI; ++i)
			{
				m_I[i].activate();
#if USE_TT_ALTERNATIVE
				m_state |= (m_I[i]() << i);
#endif
			}
			for (auto &q : m_Q)
				if (q.has_net() && !exec().nl_state().core_terms(q.net()).empty())
					active_outputs++;
			set_active_outputs(active_outputs);
		}

		NETLIB_HANDLERI(inputs)
		{
#if USE_TT_ALTERNATIVE
			m_state = 0;
			for (std::size_t i = 0; i < m_NI; ++i)
			{
				m_state |= (m_I[i]() << i);
			}
#endif
			process<true>();
		}

#if USE_TT_ALTERNATIVE
		template <std::size_t N>
		void update_N() noexcept
		{
			m_state &= ~(1<<N);
			m_state |= (m_I[N]() << N);
			process<true>();
		}
#endif

		void incdec_active(bool a) noexcept
		{
			if (a)
			{
				process<false>();
			}
			else
			{
				for (std::size_t i = 0; i< m_NI; i++)
					m_I[i].inactivate();
				m_ign = (1<<m_NI)-1;
			}
		}

		template<bool doOUT>
		void process() noexcept
		{
			netlist_time_ext mt(netlist_time_ext::zero());
			type_t new_state(0);
			type_t ign(m_ign);

			if (doOUT)
			{
#if !USE_TT_ALTERNATIVE
				for (auto I = m_I.begin(); ign != 0; ign >>= 1, ++I)
					if (ign & 1)
						I->activate();
				for (std::size_t i = 0; i < m_NI; i++)
					new_state |= (m_I[i]() << i);
#else
				new_state = m_state;
				for (std::size_t i = 0; ign != 0; ign >>= 1, ++i)
				{
					if (ign & 1)
					{
						new_state &= ~(1 << i);
						m_I[i].activate();
						new_state |= (m_I[i]() << i);
					}
				}
#endif
			}
			else
				for (std::size_t i = 0; i < m_NI; i++)
				{
					m_I[i].activate();
					new_state |= (m_I[i]() << i);
					mt = std::max(this->m_I[i].net().next_scheduled_time(), mt);
				}

			const type_t output_state(m_ttp.m_out_state[new_state]);
			type_t out(output_state & m_outmask);

			m_ign = output_state >> m_NO;

			const auto *t(&m_ttp.m_timing_index[new_state * m_NO]);

			if (doOUT)
				//for (std::size_t i = 0; i < m_NO; ++i)
				//  m_Q[i].push((out >> i) & 1, tim[t[i]]);
				this->push(out, t);
			else
			{
				const auto *tim = m_ttp.m_timing_nt.data();
				for (std::size_t i = 0; i < m_NO; ++i)
					m_Q[i].set_Q_time((out >> i) & 1, mt + tim[t[i]]);
			}

			ign = m_ign;
			for (auto I = m_I.begin(); ign != 0; ign >>= 1, ++I)
				if (ign & 1)
					I->inactivate();
#if USE_TT_ALTERNATIVE
			m_state = new_state;
#endif
		}

		template<typename T>
		void push(const T &v, const std::uint_least8_t * t)
		{
			if (m_NO >= 1) m_Q[0].push((v >> 0) & 1, m_ttp.m_timing_nt[t[0]]);
			if (m_NO >= 2) m_Q[1].push((v >> 1) & 1, m_ttp.m_timing_nt[t[1]]);
			if (m_NO >= 3) m_Q[2].push((v >> 2) & 1, m_ttp.m_timing_nt[t[2]]);
			if (m_NO >= 4) m_Q[3].push((v >> 3) & 1, m_ttp.m_timing_nt[t[3]]);
			if (m_NO >= 5) m_Q[4].push((v >> 4) & 1, m_ttp.m_timing_nt[t[4]]);
			if (m_NO >= 6) m_Q[5].push((v >> 5) & 1, m_ttp.m_timing_nt[t[5]]);
			if (m_NO >= 7) m_Q[6].push((v >> 6) & 1, m_ttp.m_timing_nt[t[6]]);
			if (m_NO >= 8) m_Q[7].push((v >> 7) & 1, m_ttp.m_timing_nt[t[7]]);
			for (std::size_t i = 8; i < m_NO; i++)
				m_Q[i].push((v >> i) & 1, m_ttp.m_timing_nt[t[i]]);
		}


		plib::static_vector<logic_input_t, m_NI> m_I;
		plib::static_vector<logic_output_t, m_NO> m_Q;

#if USE_TT_ALTERNATIVE
		state_var<type_t>   m_state;
#endif
		state_var<type_t>   m_ign;
		const truth_table_t  m_ttp;
		/* FIXME: the family should provide the names of the power-terminals! */
		nld_power_pins m_power_pins;
	};


	// ----------------------------------------------------------------------------------------
	// int compatible bitset ....
	// ----------------------------------------------------------------------------------------

	template <typename T>
	struct pbitset
	{
		using type = T;

		pbitset() noexcept : m_bs(0) { }
		pbitset(T v) noexcept : m_bs(v) { }

		pbitset &set() noexcept { *this = all_bits(); return *this; }
		pbitset &set(std::size_t bit) noexcept { m_bs |= (static_cast<T>(1) << bit); return *this; }
		pbitset &reset() noexcept { *this = no_bits(); return *this; }
		pbitset &reset(std::size_t bit) noexcept { m_bs &= ~(static_cast<T>(1) << bit); return *this; }

		pbitset flip() const noexcept { return pbitset(~m_bs); }
		pbitset flip(std::size_t bit) const noexcept { return pbitset(m_bs ^ (static_cast<T>(1) << bit)); }

		std::size_t count() const noexcept
		{
			std::size_t ret(0);
			for (T v = m_bs; v != 0; v = v >> 1)
			{
				ret += (v & 1);
			}
			return ret;
		}
		constexpr bool test(const std::size_t bit) const { return ((m_bs >> bit) & 1) == 1; }

		operator T&() noexcept  { return m_bs; }
		operator const T&() const noexcept { return m_bs; }
		constexpr T as_uint() const noexcept { return m_bs; }

		constexpr bool all() const noexcept { return *this == all_bits(); }

		/// \brief And all bits set with compressed bits from b
		///
		/// Example:
		///
		/// \code
		/// b = {b3,b2,b1,b0}
		/// v = {v7, 0, v5, 0, v3, v2, 0, 0}
		/// return {v7 & b3, 0, v5 & b2, 0, v3 & b1, v2 & b0, 0, 0}
		/// \endcode
		///
		/// \returns pbitset
		///
		pbitset expand_and(pbitset b) const noexcept
		{
			pbitset ret;
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

		static constexpr pbitset all_bits() noexcept { return pbitset(~static_cast<T>(0)); }
		static constexpr pbitset no_bits() noexcept{ return pbitset(static_cast<T>(0)); }
	private:
		T m_bs;
	};

	// ----------------------------------------------------------------------------------------
	// Truth table parsing ....
	// ----------------------------------------------------------------------------------------

	using tt_bitset = pbitset<std::uint_least64_t>;

	struct packed_int
	{
		packed_int(void *data, std::size_t bits) noexcept
		: m_data(data)
		, m_size(bits)
		{}

		void set(size_t pos, std::uint_least64_t val) noexcept
		{
			switch (m_size)
			{
				case 8: static_cast<std::uint_least8_t  *>(m_data)[pos] = static_cast<std::uint_least8_t>(val); break;
				case 16: static_cast<std::uint_least16_t *>(m_data)[pos] = static_cast<std::uint_least16_t>(val); break;
				case 32: static_cast<std::uint_least32_t *>(m_data)[pos] = static_cast<std::uint_least32_t>(val); break;
				case 64: static_cast<std::uint_least64_t *>(m_data)[pos] = static_cast<std::uint_least64_t>(val); break;
				default: { }
			}
		}

		std::uint_least64_t operator[] (size_t pos) const noexcept
		{
			switch (m_size)
			{
				case 8: return static_cast<std::uint_least8_t  *>(m_data)[pos];
				case 16: return static_cast<std::uint_least16_t *>(m_data)[pos];
				case 32: return static_cast<std::uint_least32_t *>(m_data)[pos];
				case 64: return static_cast<std::uint_least64_t *>(m_data)[pos];
				default:
					return 0; //should never happen
			}
		}

		std::uint_least64_t mask() const noexcept
		{
			switch (m_size)
			{
				case 8: return static_cast<std::uint_least8_t>(-1);
				case 16: return static_cast<std::uint_least16_t>(-1);
				case 32: return static_cast<std::uint_least32_t>(-1);
				case 64: return static_cast<std::uint_least64_t>(-1);
				default:
					return 0; //should never happen
			}
		}
	private:
		void *m_data;
		size_t m_size;
	};

	struct truth_table_parser
	{
		truth_table_parser(unsigned NO, unsigned NI,
				packed_int outs, uint_least8_t *timing, netlist_time *timing_nt)
		: m_NO(NO)
		, m_NI(NI)
		, m_out_state(outs)
		, m_timing(timing)
		, m_timing_nt(timing_nt)
		, m_num_bits(m_NI)
		, m_size(1 << (m_num_bits))
		{
		}

		void parse(const std::vector<pstring> &truthtable);

	private:
		void parse_line(unsigned cur, std::vector<pstring> list,
				tt_bitset state, std::uint_least64_t val, std::vector<uint_least8_t> &timing_index);

		tt_bitset calculate_ignored_inputs(tt_bitset state) const;

		unsigned m_NO;
		unsigned m_NI;
		packed_int m_out_state;
		uint_least8_t  *m_timing;
		netlist_time *m_timing_nt;

		const std::size_t m_num_bits;
		const std::size_t m_size;

	};

	// ----------------------------------------------------------------------------------------
	// Truth table class ....
	// ----------------------------------------------------------------------------------------

	template<std::size_t m_NI, std::size_t m_NO>
	void NETLIB_NAME(truth_table_t)<m_NI, m_NO>::init(const std::vector<pstring> &desc)
	{
		pstring header = desc[0];

		std::vector<pstring> io(plib::psplit(header,'|'));
		// checks
		nl_assert_always(io.size() == 2, "too many '|'");
		std::vector<pstring> inout(plib::psplit(io[0], ','));
		nl_assert_always(inout.size() == m_num_bits, "bit count wrong");
		std::vector<pstring> outputs(plib::psplit(io[1], ','));
		nl_assert_always(outputs.size() == m_NO, "output count wrong");

#if !USE_TT_ALTERNATIVE
		for (std::size_t i=0; i < m_NI; i++)
		{
			inout[i] = plib::trim(inout[i]);
			m_I.emplace_back(*this, inout[i], nl_delegate(&NETLIB_NAME(truth_table_t)<m_NI, m_NO> :: inputs, this));
		}
#else
		for (std::size_t i=0; i < m_NI; i++)
		{
			inout[i] = plib::trim(inout[i]);
		}
		if (0 < m_NI) m_I.emplace(0, *this, inout[0]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<0>, this));
		if (1 < m_NI) m_I.emplace(1, *this, inout[1]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<1>, this));
		if (2 < m_NI) m_I.emplace(2, *this, inout[2]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<2>, this));
		if (3 < m_NI) m_I.emplace(3, *this, inout[3]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<3>, this));
		if (4 < m_NI) m_I.emplace(4, *this, inout[4]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<4>, this));
		if (5 < m_NI) m_I.emplace(5, *this, inout[5]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<5>, this));
		if (6 < m_NI) m_I.emplace(6, *this, inout[6]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<6>, this));
		if (7 < m_NI) m_I.emplace(7, *this, inout[7]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<7>, this));
		if (8 < m_NI) m_I.emplace(8, *this, inout[8]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<8>, this));
		if (9 < m_NI) m_I.emplace(9, *this, inout[9]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<9>, this));
		if (10 < m_NI) m_I.emplace(10, *this, inout[10]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<10>, this));
		if (11 < m_NI) m_I.emplace(11, *this, inout[11]); //# nldelegate(&nld_truthtable_t<m_NI, m_NO>::update_N<11>, this));
#endif
		for (std::size_t i=0; i < m_NO; i++)
		{
			outputs[i] = plib::trim(outputs[i]);
			m_Q.emplace_back(*this, outputs[i]);
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
	// Truth table factory ....
	// ----------------------------------------------------------------------------------------

	template<unsigned m_NI, unsigned m_NO>
	class netlist_factory_truth_table_t : public factory::truth_table_base_element_t
	{
	public:
		netlist_factory_truth_table_t(const pstring &name,
			factory::properties &&props)
		: truth_table_base_element_t(name, std::move(props))
		{ }

		device_arena::unique_ptr<core_device_t> make_device(device_arena &pool, netlist_state_t &anetlist, const pstring &name) override
		{
			using tt_type = nld_truth_table_t<m_NI, m_NO>;

			if (!m_ttbl)
			{
				m_ttbl = plib::make_unique<typename nld_truth_table_t<m_NI, m_NO>::truth_table_t>(pool);
				truth_table_parser desc_s(m_NO, m_NI,
						packed_int(m_ttbl->m_out_state.data(), sizeof(m_ttbl->m_out_state[0]) * 8),
						m_ttbl->m_timing_index.data(), m_ttbl->m_timing_nt.data());

				desc_s.parse(m_desc);
			}

			return plib::make_unique<tt_type>(pool, anetlist, name, m_family_name, *m_ttbl, m_desc);
		}
	private:
		device_arena::unique_ptr<typename nld_truth_table_t<m_NI, m_NO>::truth_table_t> m_ttbl;
	};

	tt_bitset truth_table_parser::calculate_ignored_inputs(tt_bitset state) const
	{
		// Determine all inputs which may be ignored ...
		tt_bitset ignore = 0;
		for (std::size_t j=0; j<m_NI; j++)
		{
			// if changing the input directly doesn't change outputs we can ignore
			if (m_out_state[state] == m_out_state[tt_bitset(state).flip(j)])
				ignore.set(j);
		}

		// Check all permutations of ign
		// We have to remove those where the ignored inputs
		// may change the output
		//
		tt_bitset bits = tt_bitset().set(ignore.count());

		std::vector<bool> t(bits);

		// loop over all combinations of bits set in ignore
		for (std::uint_least64_t j = 1; j < bits; j++)
		{
			tt_bitset tign = ignore.expand_and(j);
			t[j] = false;
			tt_bitset bitsk(tt_bitset().set(tign.count()));

			// now loop over all combinations of the bits set currently set

			for (std::uint_least64_t k=0; k < bitsk; k++)
			{
				tt_bitset b = tign.expand_and(k);
				// will any of the inputs ignored change the output if changed?
				if (m_out_state[state] != m_out_state[state ^ b])
				{
					t[j] = true;
					break;
				}
			}
		}

		// find the ignore mask without potential for change with the most bits

		size_t jb(0);
		tt_bitset jm(0);

		for (std::uint_least64_t j=1; j<bits; j++)
		{
			tt_bitset bj(j);
			size_t nb(bj.count());
			if (!t[j] && (nb > jb))
			{
				jb = nb;
				jm = bj;
			}
		}
		return ignore.expand_and(jm);
	}

	// ----------------------------------------------------------------------------------------
	// parse line
	// ----------------------------------------------------------------------------------------

	void truth_table_parser::parse_line(unsigned cur, std::vector<pstring> list,
			tt_bitset state, std::uint_least64_t val, std::vector<uint_least8_t> &timing_index)
	{
		pstring elem = plib::trim(list[cur]);
		std::uint_least64_t start = 0;
		std::uint_least64_t end = 0;

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
		for (std::uint_least64_t i = start; i <= end; i++)
		{
			tt_bitset new_state = state;
			if (i==1)
				new_state.set(cur);

			if (cur < m_num_bits - 1)
			{
				parse_line(cur + 1, list, new_state, val, timing_index);
			}
			else
			{
				// cutoff previous inputs and outputs for ignore
				if (m_out_state[new_state] != m_out_state.mask() &&  m_out_state[new_state] != val)
					throw nl_exception(plib::pfmt("Error in truth table: State {1:04} already set, {2} != {3}\n")
							.x(new_state.as_uint())(m_out_state[new_state])(val) );
				m_out_state.set(new_state, val);
				for (std::size_t j=0; j<m_NO; j++)
					m_timing[new_state * m_NO + j] = timing_index[j];
			}
		}
	}

	void truth_table_parser::parse(const std::vector<pstring> &truthtable)
	{
		unsigned line = 0;

		pstring ttline(truthtable[line]);
		line++;
		ttline = truthtable[line];
		line++;

		for (unsigned j=0; j < m_size; j++)
			m_out_state.set(j, tt_bitset::all_bits());

		for (int j=0; j < 16; j++)
			m_timing_nt[j] = netlist_time::zero();

		while (!ttline.empty())
		{
			std::vector<pstring> io(plib::psplit(ttline,'|'));
			// checks
			nl_assert_always(io.size() == 3, "io.count mismatch");
			std::vector<pstring> inout(plib::psplit(io[0], ','));
			nl_assert_always(inout.size() == m_num_bits, "number of bits not matching");
			std::vector<pstring> out(plib::psplit(io[1], ','));
			nl_assert_always(out.size() == m_NO, "output count not matching");
			std::vector<pstring> times(plib::psplit(io[2], ','));
			nl_assert_always(times.size() == m_NO, "timing count not matching");

			tt_bitset val = 0;
			std::vector<uint_least8_t> tindex;

			//
			// FIXME: evaluation of outputs should be done in parse_line to
			//        enable the use of inputs for output values, i.e. "I1" or "~I1"
			//  in addition to "0" and "1".

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

			parse_line(0, inout, 0 , val, tindex);
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

		for (std::uint_least64_t i=0; i < m_size; i++)
		{
			if (ign[i].all()) // not yet visited
			{
				tt_bitset tign = calculate_ignored_inputs(i);

				ign[i] = tign;

				// don't need to recalculate similar ones
				tt_bitset bitsk;
				bitsk.set(tign.count());

				for (std::uint_least64_t k=0; k < bitsk; k++)
				{
					tt_bitset b = tign.expand_and(k);
					ign[(i & tign.flip()) | b] = tign;
				}
			}
		}

		for (size_t i=0; i<m_size; i++)
		{
			if (m_out_state[i] == m_out_state.mask())
				throw nl_exception(plib::pfmt("truth table: found element not set {1}\n").x(i) );
			m_out_state.set(i, m_out_state[i] | (ign[i] << m_NO));
		}
	}

} // namespace netlist::devices

namespace netlist::factory {

	truth_table_base_element_t::truth_table_base_element_t(const pstring &name,
		properties &&props)
	: factory::element_t(name, std::move(props))
	, m_family_name(config::DEFAULT_LOGIC_FAMILY())
	{
	}

	#define ENTRYY(n, m, s)    case (n * 100 + m): \
		{ using xtype = devices::netlist_factory_truth_table_t<n, m>; \
			auto cs=s; \
			ret = plib::make_unique<xtype, host_arena>(desc.name, std::move(cs)); } \
			break

	#define ENTRY(n, s) ENTRYY(n, 1, s); ENTRYY(n, 2, s); ENTRYY(n, 3, s); \
						ENTRYY(n, 4, s); ENTRYY(n, 5, s); ENTRYY(n, 6, s); \
						ENTRYY(n, 7, s); ENTRYY(n, 8, s); ENTRYY(n, 9, s); \
						ENTRYY(n, 10, s)

	host_arena::unique_ptr<truth_table_base_element_t> truth_table_create(tt_desc &desc, properties &&props)
	{
		host_arena::unique_ptr<truth_table_base_element_t> ret;

		switch (desc.ni * 100 + desc.no)
		{
			ENTRY(1, props);
			ENTRY(2, props);
			ENTRY(3, props);
			ENTRY(4, props);
			ENTRY(5, props);
			ENTRY(6, props);
			ENTRY(7, props);
			ENTRY(8, props);
			ENTRY(9, props);
			ENTRY(10, props);
			ENTRY(11, props);
			ENTRY(12, props);
			default:
				pstring msg = plib::pfmt("unable to create truth table<{1},{2}>")(desc.ni)(desc.no);
				nl_assert_always(false, putf8string(msg).c_str());
		}
		ret->m_desc = desc.desc;
		ret->m_family_name = (!desc.family.empty() ? desc.family : pstring(config::DEFAULT_LOGIC_FAMILY()));

		return ret;
	}

} // namespace netlist::factory
