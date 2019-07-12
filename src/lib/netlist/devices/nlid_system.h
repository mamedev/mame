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

#include "netlist/analog/nlid_twoterm.h"
#include "netlist/nl_base.h"
#include "netlist/nl_setup.h"
#include "plib/putil.h"

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
		, m_use_deactivate(*this, "USE_DEACTIVATE", false)
		, m_startup_strategy(*this, "STARTUP_STRATEGY", 1)
		, m_mos_capmodel(*this, "DEFAULT_MOS_CAPMODEL", 2)
		, m_max_link_loops(*this, "MAX_LINK_RESOLVE_LOOPS", 100)
		{
		}
		NETLIB_UPDATEI() { }
		//NETLIB_RESETI() { }
		//NETLIB_UPDATE_PARAMI() { }
	public:
		param_logic_t m_use_deactivate;
		param_num_t<unsigned>   m_startup_strategy;
		param_num_t<unsigned>   m_mos_capmodel;
		//! How many times do we try to resolve links (connections)
		param_num_t<unsigned>   m_max_link_loops;
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
			m_Q.net().set_next_scheduled_time(netlist_time::zero());
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
			net.set_next_scheduled_time(exec().time() + m_inc);
		}

	public:
		logic_output_t m_Q;
		netlist_time m_inc;
	private:
		param_double_t m_freq;
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
		//NETLIB_RESETI();

		NETLIB_UPDATE_PARAMI()
		{
			m_inc = netlist_time::from_double(1.0 / (m_freq() * 2.0));
		}

		NETLIB_UPDATEI()
		{
			m_Q.push(!m_feedback(), m_inc);
		}

	private:
		logic_input_t m_feedback;
		logic_output_t m_Q;

		param_double_t m_freq;
		netlist_time m_inc;
	};

	// -----------------------------------------------------------------------------
	// varclock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(varclock)
	{
		NETLIB_CONSTRUCTOR(varclock)
		, m_feedback(*this, "FB")
		, m_Q(*this, "Q")
		, m_func(*this,"FUNC", "")
		, m_compiled(this->name() + ".FUNCC", this, this->state().run_state_manager())
		, m_funcparam({0.0})
		{
			if (m_func() != "")
				m_compiled.compile(std::vector<pstring>({{"T"}}), m_func());
			connect(m_feedback, m_Q);
		}
		//NETLIB_RESETI();
		//NETLIB_UPDATE_PARAMI()

		NETLIB_UPDATEI()
		{
			m_funcparam[0] = exec().time().as_double();
			const netlist_time m_inc = netlist_time::from_double(m_compiled.evaluate(m_funcparam));
			m_Q.push(!m_feedback(), m_inc);
		}

	private:
		logic_input_t m_feedback;
		logic_output_t m_Q;

		param_str_t m_func;
		plib::pfunction m_compiled;
		std::vector<double> m_funcparam;
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

			netlist_time base = netlist_time::from_double(1.0 / (m_freq()*2.0));
			std::vector<pstring> pat(plib::psplit(m_pattern(),","));
			m_off = netlist_time::from_double(m_offset());

			std::array<std::int64_t, 32> pati = { 0 };

			m_size = static_cast<std::uint8_t>(pat.size());
			netlist_time::mult_type total = 0;
			for (unsigned i=0; i<m_size; i++)
			{
				// FIXME: use pstonum_ne
				//pati[i] = plib::pstonum<decltype(pati[i])>(pat[i]);
				pati[i] = plib::pstonum<std::int64_t, true>(pat[i]);
				total += pati[i];
			}
			netlist_time ttotal = netlist_time::zero();
			auto sm1 = static_cast<uint8_t>(m_size - 1);
			for (unsigned i=0; i < sm1; i++)
			{
				m_inc[i] = base * pati[i];
				ttotal += m_inc[i];
			}
			m_inc[sm1] = base * total - ttotal;

		}

		NETLIB_UPDATEI();
		NETLIB_RESETI();
		//NETLIB_UPDATE_PARAMI();

		NETLIB_HANDLERI(clk2);
		NETLIB_HANDLERI(clk2_pow2);

	private:

		param_double_t m_freq;
		param_str_t m_pattern;
		param_double_t m_offset;

		logic_input_t m_feedback;
		logic_output_t m_Q;
		state_var_u8 m_cnt;
		std::uint8_t m_size;
		state_var<netlist_time> m_off;
		std::array<netlist_time, 32> m_inc;
	};

	// -----------------------------------------------------------------------------
	// Special support devices ...
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(logic_input)
	{
		NETLIB_CONSTRUCTOR(logic_input)
		, m_Q(*this, "Q")
		, m_IN(*this, "IN", false)
		/* make sure we get the family first */
		, m_FAMILY(*this, "FAMILY", "FAMILY(TYPE=TTL)")
		{
			set_logic_family(setup().family_from_model(m_FAMILY()));
			m_Q.set_logic_family(this->logic_family());
		}

		NETLIB_UPDATEI() { }
		NETLIB_RESETI() { m_Q.initial(0); }
		NETLIB_UPDATE_PARAMI() { m_Q.push(m_IN() & 1, netlist_time::from_nsec(1)); }

	private:
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

		NETLIB_UPDATEI() {  }
		NETLIB_RESETI() { m_Q.initial(0.0); }
		NETLIB_UPDATE_PARAMI() { m_Q.push(m_IN()); }

	private:
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

	NETLIB_OBJECT(dummy_input)
	{
	public:
		NETLIB_CONSTRUCTOR(dummy_input)
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

	NETLIB_OBJECT(frontier)
	{
	public:
		NETLIB_CONSTRUCTOR(frontier)
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
			m_RIN.set_G_V_I(1.0 / m_p_RIN(),0,0);
			m_ROUT.set_G_V_I(1.0 / m_p_ROUT(),0,0);
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
		, m_compiled(this->name() + ".FUNCC", this, this->state().run_state_manager())
		{
			std::vector<pstring> inps;
			for (int i=0; i < m_N(); i++)
			{
				pstring n = plib::pfmt("A{1}")(i);
				m_I.push_back(pool().make_poolptr<analog_input_t>(*this, n));
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
		std::vector<pool_owned_ptr<analog_input_t>> m_I;

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

		NETLIB_RESETI();
		//NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATEI();

		analog::NETLIB_SUB(R_base) m_R;
		logic_input_t m_I;
		param_double_t m_RON;
		param_double_t m_ROFF;

	private:

		state_var<netlist_sig_t> m_last_state;
	};

	// -----------------------------------------------------------------------------
	// power pins - not a device, but a helper
	// -----------------------------------------------------------------------------

	/**
	 * Power Pins are passive inputs. Delegate noop will silently ignore any
	 * updates.
	 */
	class nld_power_pins
	{
	public:
		nld_power_pins(device_t &owner, const char *sVCC = "VCC", const char *sGND = "GND", bool force_analog_input = false)
		{
			if (owner.setup().is_validation() || force_analog_input)
			{
				m_GND = plib::make_unique<analog_input_t>(owner, sGND, NETLIB_DELEGATE(power_pins, noop));
				m_VCC = plib::make_unique<analog_input_t>(owner, sVCC, NETLIB_DELEGATE(power_pins, noop));
			}
			else
			{
				owner.create_and_register_subdevice(sPowerDevRes, m_RVG);
				owner.register_subalias(sVCC, "_RVG.1");
				owner.register_subalias(sGND, "_RVG.2");
			}
		}

		/* FIXME: this will seg-fault if force_analog_input = false */
		nl_double VCC() const NL_NOEXCEPT { return m_VCC->Q_Analog(); }
		nl_double GND() const NL_NOEXCEPT { return m_GND->Q_Analog(); }

		NETLIB_SUBXX(analog, R) m_RVG; // dummy resistor between VCC and GND

	private:
		void noop() { }
		plib::unique_ptr<analog_input_t> m_VCC; // only used during validation or force_analog_input
		plib::unique_ptr<analog_input_t> m_GND; // only used during validation or force_analog_input
	};

} //namespace devices
} // namespace netlist

#endif /* NLD_SYSTEM_H_ */
