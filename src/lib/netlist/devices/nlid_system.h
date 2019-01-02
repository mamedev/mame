// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlid_system.h
 *
 * netlist devices defined in the core
 *
 * This file contains internal headers
 */

#ifndef NLID_SYSTEM_H_
#define NLID_SYSTEM_H_

#include "../nl_base.h"
#include "../nl_setup.h"
#include "../analog/nlid_twoterm.h"
#include "../plib/putil.h"

namespace netlist
{
	namespace devices
	{
	// -----------------------------------------------------------------------------
	// netlistparams
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(netlistparams)
	{
		NETLIB_CONSTRUCTOR(netlistparams)
		, m_use_deactivate(*this, "USE_DEACTIVATE", 0)
		{
		}
		NETLIB_UPDATEI() { }
		//NETLIB_RESETI() { }
		//NETLIB_UPDATE_PARAMI() { }
	public:
		param_logic_t m_use_deactivate;
	};

	// -----------------------------------------------------------------------------
	// mainclock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(mainclock)
	{
		NETLIB_CONSTRUCTOR(mainclock)
		, m_Q(*this, "Q")
		, m_freq(*this, "FREQ", 7159000.0 * 5)
		{
			m_inc = netlist_time::from_double(1.0 / (m_freq()*2.0));
		}

		NETLIB_RESETI()
		{
			m_Q.net().set_time(netlist_time::zero());
		}

		NETLIB_UPDATE_PARAMI()
		{
			m_inc = netlist_time::from_double(1.0 / (m_freq()*2.0));
		}

		NETLIB_UPDATEI()
		{
			logic_net_t &net = m_Q.net();
			// this is only called during setup ...
			net.toggle_new_Q();
			net.set_time(netlist().time() + m_inc);
		}

	public:
		logic_output_t m_Q;

		param_double_t m_freq;
		netlist_time m_inc;

		inline static void mc_update(logic_net_t &net);
	};

	// -----------------------------------------------------------------------------
	// clock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(clock)
	{
		NETLIB_CONSTRUCTOR(clock)
		, m_feedback(*this, "FB")
		, m_Q(*this, "Q")
		, m_freq(*this, "FREQ", 7159000.0 * 5.0)
		{
			m_inc = netlist_time::from_double(1.0 / (m_freq()*2.0));

			connect(m_feedback, m_Q);
		}
		NETLIB_UPDATEI();
		//NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();

	protected:
		logic_input_t m_feedback;
		logic_output_t m_Q;

		param_double_t m_freq;
		netlist_time m_inc;
	};

	// -----------------------------------------------------------------------------
	// extclock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(extclock)
	{
		NETLIB_CONSTRUCTOR(extclock)
		, m_freq(*this, "FREQ", 7159000.0 * 5.0)
		, m_pattern(*this, "PATTERN", "1,1")
		, m_offset(*this, "OFFSET", 0.0)
		, m_feedback(*this, "FB")
		, m_Q(*this, "Q")
		, m_cnt(*this, "m_cnt", 0)
		, m_off(*this, "m_off", netlist_time::zero())
		{
			m_inc[0] = netlist_time::from_double(1.0 / (m_freq() * 2.0));

			connect(m_feedback, m_Q);
			{
				netlist_time base = netlist_time::from_double(1.0 / (m_freq()*2.0));
				std::vector<pstring> pat(plib::psplit(m_pattern(),","));
				m_off = netlist_time::from_double(m_offset());

				unsigned long pati[32];
				for (int pI = 0; pI < 32; pI++)
				{
					pati[pI] = 0;
				}
				m_size = static_cast<std::uint8_t>(pat.size());
				unsigned long total = 0;
				for (unsigned i=0; i<m_size; i++)
				{
					pati[i] = static_cast<unsigned long>(pat[i].as_long());
					total += pati[i];
				}
				netlist_time ttotal = netlist_time::zero();
				for (unsigned i=0; i<m_size - 1; i++)
				{
					m_inc[i] = base * pati[i];
					ttotal += m_inc[i];
				}
				m_inc[m_size - 1] = base * total - ttotal;
			}
		}
		NETLIB_UPDATEI();
		NETLIB_RESETI();
		//NETLIB_UPDATE_PARAMI();
	protected:

		param_double_t m_freq;
		param_str_t m_pattern;
		param_double_t m_offset;

		logic_input_t m_feedback;
		logic_output_t m_Q;
		state_var_u8 m_cnt;
		std::uint8_t m_size;
		state_var<netlist_time> m_off;
		netlist_time m_inc[32];
	};

	// -----------------------------------------------------------------------------
	// Special support devices ...
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(logic_input)
	{
		NETLIB_CONSTRUCTOR(logic_input)
		, m_Q(*this, "Q")
		, m_IN(*this, "IN", 0)
		/* make sure we get the family first */
		, m_FAMILY(*this, "FAMILY", "FAMILY(TYPE=TTL)")
		{
			set_logic_family(setup().family_from_model(m_FAMILY()));
		}

		NETLIB_UPDATE_AFTER_PARAM_CHANGE()

		NETLIB_UPDATEI();
		NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();

	protected:
		logic_output_t m_Q;

		param_logic_t m_IN;
		param_model_t m_FAMILY;
	};

	NETLIB_OBJECT(analog_input)
	{
		NETLIB_CONSTRUCTOR(analog_input)
		, m_Q(*this, "Q")
		, m_IN(*this, "IN", 0.0)
		{
		}
		NETLIB_UPDATE_AFTER_PARAM_CHANGE()

		NETLIB_UPDATEI();
		NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();
	protected:
		analog_output_t m_Q;
		param_double_t m_IN;
	};

	// -----------------------------------------------------------------------------
	// nld_gnd
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(gnd)
	{
		NETLIB_CONSTRUCTOR(gnd)
		, m_Q(*this, "Q")
		{
		}
		NETLIB_UPDATEI()
		{
			m_Q.push(0.0);
		}
		NETLIB_RESETI() { }
	protected:
		analog_output_t m_Q;
	};

	// -----------------------------------------------------------------------------
	// nld_dummy_input
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(dummy_input, base_dummy)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(dummy_input, base_dummy)
		, m_I(*this, "I")
		{
		}

	protected:

		NETLIB_RESETI() { }
		NETLIB_UPDATEI() { }

	private:
		analog_input_t m_I;

	};

	// -----------------------------------------------------------------------------
	// nld_frontier
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(frontier, base_dummy)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(frontier, base_dummy)
		, m_RIN(*this, "m_RIN", true)
		, m_ROUT(*this, "m_ROUT", true)
		, m_I(*this, "_I")
		, m_Q(*this, "_Q")
		, m_p_RIN(*this, "RIN", 1.0e6)
		, m_p_ROUT(*this, "ROUT", 50.0)

		{
			register_subalias("I", m_RIN.m_P);
			register_subalias("G", m_RIN.m_N);
			connect(m_I, m_RIN.m_P);

			register_subalias("_OP", m_ROUT.m_P);
			register_subalias("Q", m_ROUT.m_N);
			connect(m_Q, m_ROUT.m_P);
		}

		NETLIB_RESETI()
		{
			m_RIN.set(1.0 / m_p_RIN(),0,0);
			m_ROUT.set(1.0 / m_p_ROUT(),0,0);
		}

		NETLIB_UPDATEI()
		{
			m_Q.push(m_I());
		}

	private:
		analog::NETLIB_NAME(twoterm) m_RIN;
		/* Fixme: only works if the device is time-stepped - need to rework */
		analog::NETLIB_NAME(twoterm) m_ROUT;
		analog_input_t m_I;
		analog_output_t m_Q;

		param_double_t m_p_RIN;
		param_double_t m_p_ROUT;
	};

	/* -----------------------------------------------------------------------------
	 * nld_function
	 *
	 * FIXME: Currently a proof of concept to get congo bongo working
	 * ----------------------------------------------------------------------------- */

	NETLIB_OBJECT(function)
	{
		NETLIB_CONSTRUCTOR(function)
		, m_N(*this, "N", 1)
		, m_func(*this, "FUNC", "A0")
		, m_Q(*this, "Q")
		, m_compiled(this->name() + ".FUNCC", this, this->netlist().state())
		{
			std::vector<pstring> inps;
			for (int i=0; i < m_N(); i++)
			{
				pstring n = plib::pfmt("A{1}")(i);
				m_I.push_back(plib::make_unique<analog_input_t>(*this, n));
				inps.push_back(n);
				m_vals.push_back(0.0);
			}
			m_compiled.compile(inps, m_func());
		}

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:

		param_int_t m_N;
		param_str_t m_func;
		analog_output_t m_Q;
		std::vector<std::unique_ptr<analog_input_t>> m_I;

		std::vector<double> m_vals;
		plib::pfunction m_compiled;
	};

	// -----------------------------------------------------------------------------
	// nld_res_sw
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(res_sw)
	{
	public:
		NETLIB_CONSTRUCTOR(res_sw)
		, m_R(*this, "_R")
		, m_I(*this, "I")
		, m_RON(*this, "RON", 1.0)
		, m_ROFF(*this, "ROFF", 1.0E20)
		, m_last_state(*this, "m_last_state", 0)
		{
			register_subalias("1", m_R.m_P);
			register_subalias("2", m_R.m_N);
		}

		analog::NETLIB_SUB(R_base) m_R;
		logic_input_t m_I;
		param_double_t m_RON;
		param_double_t m_ROFF;

		NETLIB_RESETI();
		//NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATEI();

	private:
		state_var<netlist_sig_t> m_last_state;
	};

	} //namespace devices
} // namespace netlist

#endif /* NLD_SYSTEM_H_ */
