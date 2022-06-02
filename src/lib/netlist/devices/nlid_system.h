// license:BSD-3-Clause
// copyright-holders:Couriersud
#ifndef NLID_SYSTEM_H_
#define NLID_SYSTEM_H_

///
/// \file nlid_system.h
///

#include "analog/nlid_twoterm.h"
#include "nl_base.h"
#include "nl_factory.h"
#include "plib/prandom.h"
#include "plib/pstonum.h"
#include "plib/putil.h"

#include <random>

namespace netlist::devices {

	// -----------------------------------------------------------------------------
	// clock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(clock)
	{
		NETLIB_CONSTRUCTOR(clock)
		, m_feedback(*this, "FB", NETLIB_DELEGATE(fb))
		, m_Q(*this, "Q")
		, m_freq(*this, "FREQ", nlconst::magic(7159000.0 * 5.0))
		, m_supply(*this)
		{
			m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));

			connect("FB", "Q");
		}

		NETLIB_UPDATE_PARAMI()
		{
			m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
		}

		NETLIB_HANDLERI(fb)
		{
			m_Q.push(m_feedback() ^ 1, m_inc);
		}

	private:
		logic_input_t m_feedback;
		logic_output_t m_Q;

		param_fp_t m_freq;
		netlist_time m_inc;

		NETLIB_NAME(power_pins) m_supply;
};

	// -----------------------------------------------------------------------------
	// variable clock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(varclock)
	{
		NETLIB_CONSTRUCTOR(varclock)
		, m_N(*this, "N", 1)
		, m_func(*this,"FUNC", "T")
		, m_feedback(*this, "FB", NETLIB_DELEGATE(fb))
		, m_Q(*this, "Q")
		, m_compiled(*this, "m_compiled")
		, m_supply(*this)
		{
			if (!m_func().empty())
			{
				std::vector<pstring> inputs;

				m_I.reserve(m_N());
				inputs.reserve(m_N() + 1);
				m_vals.reserve(m_N() + 1);

				// add time parameter to the front
				inputs.emplace_back("T");
				m_vals.push_back(nlconst::zero());

				for (std::uint64_t i=0; i < m_N(); i++)
				{
					pstring input_name = plib::pfmt("A{1}")(i);
					m_I.push_back(owner.template make_pool_object<analog_input_t>(*this, input_name, NETLIB_DELEGATE(fb)));
					inputs.push_back(input_name);
					m_vals.push_back(nlconst::zero());
				}
				m_compiled->compile(m_func(), inputs);
			}
			connect("FB", "Q");
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATE_PARAMI()

	private:
		NETLIB_HANDLERI(fb)
		{
			m_vals[0] = exec().time().as_fp<nl_fptype>();
			for (std::size_t i = 0; i < static_cast<unsigned>(m_N()); i++)
			{
				m_vals[i+1] = (*m_I[i])();
			}
			const netlist_time m_inc = netlist_time::from_fp(m_compiled->evaluate(m_vals));
			m_Q.push(m_feedback() ^ 1, m_inc);
		}

		using pf_type = plib::pfunction<nl_fptype>;
		param_num_t<std::uint64_t> m_N;
		param_str_t m_func;
		logic_input_t m_feedback;
		logic_output_t m_Q;
		std::vector<device_arena::unique_ptr<analog_input_t>> m_I;

		pf_type::values_container m_vals;
		state_var<pf_type> m_compiled;

		NETLIB_NAME(power_pins) m_supply;
	};

	// -----------------------------------------------------------------------------
	// Special support devices ...
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(logic_input)
	{
		NETLIB_CONSTRUCTOR(logic_input)
		, m_Q(*this, "Q")
		, m_IN(*this, "IN", false)
		, m_supply(*this)
		{
		}

		NETLIB_RESETI() { m_Q.initial(0); }
		NETLIB_UPDATE_PARAMI()
		{
			//printf("%s %d\n", name().c_str(), m_IN());
			m_Q.push(m_IN() & 1, netlist_time::from_nsec(1));
		}

	private:
		logic_output_t m_Q;

		param_logic_t m_IN;
		NETLIB_NAME(power_pins) m_supply;
	};

	template<std::size_t N>
	NETLIB_OBJECT(logic_inputN)
	{
		NETLIB_CONSTRUCTOR(logic_inputN)
		, m_Q(*this, "Q{}")
		, m_IN(*this, "IN", 0)
		, m_supply(*this)
		{
		}

		NETLIB_RESETI() { for (auto &q : m_Q) q.initial(0); }
		NETLIB_UPDATE_PARAMI()
		{
			//printf("%s %d\n", name().c_str(), m_IN());
			for (std::size_t i=0; i<N; i++)
				m_Q[i].push((m_IN()>>i) & 1, netlist_time::from_nsec(1));
		}

	private:
		object_array_t<logic_output_t, N> m_Q;

		param_int_t m_IN;
		NETLIB_NAME(power_pins) m_supply;
	};

	NETLIB_OBJECT(analog_input)
	{
		NETLIB_CONSTRUCTOR(analog_input)
		, m_Q(*this, "Q")
		, m_IN(*this, "IN", nlconst::zero())
		{
		}

		NETLIB_RESETI() { m_Q.initial(nlconst::zero()); }
		NETLIB_UPDATE_PARAMI() { m_Q.push(m_IN()); }

	private:
		analog_output_t m_Q;
		param_fp_t m_IN;
	};


	// -----------------------------------------------------------------------------
	// nld_frontier
	// -----------------------------------------------------------------------------


	/// \brief Frontiers divides a netlist into sub netlist
	///
	/// Example:
	///
	/// Consider the following mixing stage
	///
	///                 R1
	///      I1 >-----1RRRR2---------+
	///                              |
	///                 R2           |
	///      I2 >-----1RRRR2---------+----------> Out
	///                              |
	///                              R
	///                           R3 R
	///                              R
	///                              |
	///                             GND
	///
	/// With OPTIMIZE_FRONTIER(R2.1, R2, RX) where RX is the impedance of the
	/// output connected to I2 this becomes:
	///
	///                 R1
	///      I1 >-----1RRRR2--------------------------------+
	///                                                     |
	///               ##########################            |
	///               #                  RX    #     R2     |
	///      I2 >----->--+-AnIn AnOut>--RRRR--->---1RRRR2---+----------> Out
	///               #  |                     #            |
	///               #  R                     #            R
	///               #  R R2                  #         R3 R
	///               #  R                     #            R
	///               #  |                     #            |
	///               # GND          Frontier  #           GND
	///               #                        #
	///               ##########################
	///
	/// As a result, provided there are no other connections between the parts
	/// generating S1 and S2 the "S2 part" will now have a separate solver.
	///
	/// The size (aka number of nets) of the solver for I1 will be smaller.
	/// The size of the solver for I2 and the rest of the circuit will be smaller
	/// as well.
	///

	NETLIB_OBJECT(frontier)
	{
		NETLIB_CONSTRUCTOR(frontier)
		, m_RIN(*this, "m_RIN", NETLIB_DELEGATE(input)) // FIXME: does not look right
		, m_ROUT(*this, "m_ROUT", NETLIB_DELEGATE(input)) // FIXME: does not look right
		, m_I(*this, "_I", NETLIB_DELEGATE(input))
		, m_Q(*this, "_Q")
		, m_p_RIN(*this, "RIN", nlconst::magic(1.0e6))
		, m_p_ROUT(*this, "ROUT", nlconst::magic(50.0))

		{
			register_sub_alias("I", "m_RIN.1");
			register_sub_alias("G", "m_RIN.2");
			connect("_I", "m_RIN.1");

			register_sub_alias("_OP", "m_ROUT.1");
			register_sub_alias("Q", "m_ROUT.2");
			connect("_Q", "m_ROUT.1");
		}

	private:
		NETLIB_RESETI()
		{
			//printf("%s: in %f out %f\n", name().c_str(), m_p_RIN(), m_p_ROUT());
			m_RIN.set_G_V_I(plib::reciprocal(m_p_RIN()),0,0);
			m_ROUT.set_G_V_I(plib::reciprocal(m_p_ROUT()),0,0);
		}

		NETLIB_HANDLERI(input)
		{
			m_Q.push(m_I());
		}

		analog::NETLIB_NAME(two_terminal) m_RIN;
		analog::NETLIB_NAME(two_terminal) m_ROUT;
		analog_input_t m_I;
		analog_output_t m_Q;

		param_fp_t m_p_RIN;
		param_fp_t m_p_ROUT;
	};

	// -----------------------------------------------------------------------------
	// nld_function
	// ----------------------------------------------------------------------------- */

	NETLIB_OBJECT(function)
	{
		NETLIB_CONSTRUCTOR(function)
		, m_N(*this, "N", 1)
		, m_function(*this, "FUNC", "A0")
		, m_thresh(*this, "THRESH", nlconst::zero())
		, m_Q(*this, "Q")
		, m_compiled(*this, "m_compiled")
		, m_last(*this, "m_last")
		{
			std::vector<pstring> inputs;

			m_I.reserve(m_N());
			inputs.reserve(m_N());
			m_values.reserve(m_N());

			for (uint64_t i=0; i < m_N(); i++)
			{
				pstring input_name = plib::pfmt("A{1}")(i);
				m_I.push_back(owner.template make_pool_object<analog_input_t>(*this, input_name, NETLIB_DELEGATE(inputs)));
				inputs.push_back(input_name);
				m_values.push_back(nlconst::zero());
			}
			m_compiled->compile(m_function(), inputs);
		}

	protected:
		NETLIB_RESETI()
		{
			//m_Q.initial(0.0);
		}

		NETLIB_HANDLERI(inputs)
		{
			for (std::size_t i = 0; i < static_cast<unsigned>(m_N()); i++)
			{
				m_values[i] = (*m_I[i])();
			}
			auto result = m_compiled->evaluate(m_values);
			if (plib::abs(m_last - result) >= m_thresh)
			{
				m_Q.push(result);
				m_last = result;
			}
		}

	private:
		using pf_type = plib::pfunction<nl_fptype>;
		param_num_t<std::uint64_t> m_N;
		param_str_t m_function;
		param_fp_t m_thresh;
		analog_output_t m_Q;
		std::vector<device_arena::unique_ptr<analog_input_t>> m_I;

		pf_type::values_container m_values;
		state_var<pf_type> m_compiled;
		state_var<nl_fptype> m_last;

	};

	// -----------------------------------------------------------------------------
	// nld_sys_dsw1
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(sys_dsw1)
	{
		NETLIB_CONSTRUCTOR(sys_dsw1)
		, m_RON(*this, "RON", nlconst::one())
		, m_ROFF(*this, "ROFF", nlconst::magic(1.0E20))
		, m_R(*this, "_R")
		, m_I(*this, "I", NETLIB_DELEGATE(input))
		, m_last_state(*this, "m_last_state", 0)
		{
			register_sub_alias("1", "_R.1");
			register_sub_alias("2", "_R.2");
		}

		NETLIB_RESETI()
		{
			m_last_state = 0;
			m_R.set_R(m_ROFF());
		}

		//NETLIB_UPDATE_PARAMI();

		//FIXME: used by 74123

		const terminal_t &P() const noexcept { return m_R.P(); }
		const terminal_t &N() const noexcept { return m_R.N(); }
		const logic_input_t &I() const noexcept { return m_I; }

		param_fp_t m_RON;
		param_fp_t m_ROFF;

	private:
		NETLIB_HANDLERI(input)
		{
			const netlist_sig_t state = m_I();
			if (state != m_last_state)
			{
				m_last_state = state;
				const nl_fptype R = (state != 0) ? m_RON() : m_ROFF();

				m_R.change_state([this, &R]()
				{
					m_R.set_R(R);
				});
			}
		}

		analog::NETLIB_SUB(R_base) m_R;
		logic_input_t m_I;

		state_var<netlist_sig_t> m_last_state;
	};

	// -----------------------------------------------------------------------------
	// nld_sys_dsw2
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(sys_dsw2)
	{
		NETLIB_CONSTRUCTOR(sys_dsw2)
		, m_R1(*this, "_R1")
		, m_R2(*this, "_R2")
		, m_I(*this, "I", NETLIB_DELEGATE(input))
		, m_GON(*this, "GON", nlconst::magic(1e9)) // FIXME: all switches should have some on value
		, m_GOFF(*this, "GOFF", nlconst::cgmin())
		, m_power_pins(*this)
		{
			// connect and register pins
			register_sub_alias("1", "_R1.1");
			register_sub_alias("2", "_R1.2");
			register_sub_alias("3", "_R2.2");
			connect("_R1.2", "_R2.1");
		}

	private:
		NETLIB_RESETI()
		{
			m_R1.set_G(m_GOFF());
			m_R2.set_G(m_GON());
		}

		//NETLIB_UPDATE_PARAMI();

		NETLIB_HANDLERI(input)
		{
			const netlist_sig_t state = m_I();

			//printf("Here %d\n", state);
			const nl_fptype G1 = (state != 0) ? m_GON() : m_GOFF();
			const nl_fptype G2 = (state != 0) ? m_GOFF() : m_GON();
			if (m_R1.solver() == m_R2.solver())
			{
				m_R1.change_state([this, &G1, &G2]()
				{
					m_R1.set_G(G1);
					m_R2.set_G(G2);
				});
			}
			else
			{
				m_R1.change_state([this, &G1]()
				{
					m_R1.set_G(G1);
				});
				m_R2.change_state([this, &G2]()
				{
					m_R2.set_G(G2);
				});
			}
		}

		analog::NETLIB_SUB(R_base) m_R1;
		analog::NETLIB_SUB(R_base) m_R2;
		logic_input_t m_I;
		param_fp_t m_GON;
		param_fp_t m_GOFF;

		nld_power_pins m_power_pins;
	};


	// -----------------------------------------------------------------------------
	// nld_sys_comp
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(sys_compd)
	{
		NETLIB_CONSTRUCTOR(sys_compd)
		, m_IP(*this, "IP", NETLIB_DELEGATE(inputs))
		, m_IN(*this, "IN", NETLIB_DELEGATE(inputs))
		, m_Q(*this, "Q")
		, m_QQ(*this, "QQ")
		, m_power_pins(*this)
		, m_last_state(*this, "m_last_state", 2) // ensure first execution
		{
		}

	private:
		NETLIB_RESETI()
		{
			m_last_state = 0;
		}

		//NETLIB_UPDATE_PARAMI();

		NETLIB_HANDLERI(inputs)
		{
			const netlist_sig_t state = (m_IP() > m_IN());
			if (state != m_last_state)
			{
				m_last_state = state;
				// FIXME: make timing a parameter
				m_Q.push(state, NLTIME_FROM_NS(10));
				m_QQ.push(!state, NLTIME_FROM_NS(10));
			}
		}

		analog_input_t m_IP;
		analog_input_t m_IN;
		logic_output_t m_Q;
		logic_output_t m_QQ;
		nld_power_pins m_power_pins;

		state_var<netlist_sig_t> m_last_state;
	};

	///
	/// \brief  noise source
	///
	/// An externally clocked noise source. The noise acts as a voltage source
	/// with internal resistance RI.
	///
	/// Since a new random value is used on each state change on I the effective
	/// frequency is clock source frequency times two!
	///
	/// Typical application:
	///
	///             VCC
	///              |
	///              R
	///              R
	///              R
	///              |
	///              +-----> Output
	///              |
	///         +-------+
	///         |    1  |
	///     --->| I     |
	///         |    2  |
	///         +-------+
	///              |
	///              R
	///              R
	///              R
	///              |
	///             GND
	///
	// -----------------------------------------------------------------------------
	template <typename E, template<class> class D>
	NETLIB_OBJECT(sys_noise)
	{
	public:

		using engine = E;
		using distribution = D<nl_fptype>;

		NETLIB_CONSTRUCTOR(sys_noise)
		, m_T(*this, "m_T")
		, m_I(*this, "I", NETLIB_DELEGATE(input))
		, m_RI(*this, "RI", nlconst::magic(0.1))
		, m_sigma(*this, "SIGMA", nlconst::zero())
		, m_mt(*this, "m_mt")
		, m_dis(*this, "m_dis",m_sigma())
		{

			register_sub_alias("1", "m_T.1");
			register_sub_alias("2", "m_T.2");
		}

	private:
		NETLIB_HANDLERI(input)
		{
			nl_fptype val = m_dis()(m_mt());
			m_T.change_state([this, val]()
			{
				m_T.set_G_V_I(plib::reciprocal(m_RI()), val, nlconst::zero());
			});
		}

		NETLIB_RESETI()
		{
			m_T.set_G_V_I(plib::reciprocal(m_RI()), nlconst::zero(), nlconst::zero());
		}

		analog::NETLIB_SUB(two_terminal) m_T;
		logic_input_t m_I;
		param_fp_t m_RI;
		param_fp_t m_sigma;
		state_var<engine> m_mt;
		state_var<distribution> m_dis;
	};

} // namespace netlist::devices

#endif // NLD_SYSTEM_H_
