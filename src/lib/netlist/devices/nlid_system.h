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

#include "nl_setup.h"
#include "nl_base.h"
#include "nl_factory.h"
#include "analog/nld_twoterm.h"

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

			m_inc = netlist_time::from_hz(m_freq.Value()*2);
		}

		NETLIB_RESETI()
		{
			m_Q.net().set_time(netlist_time::zero());
		}

		NETLIB_UPDATE_PARAMI()
		{
			m_inc = netlist_time::from_hz(m_freq.Value()*2);
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

			m_inc = netlist_time::from_hz(m_freq.Value()*2);

			connect_late(m_feedback, m_Q);
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
		{
			m_inc[0] = netlist_time::from_hz(m_freq.Value()*2);

			connect_late(m_feedback, m_Q);
			{
				netlist_time base = netlist_time::from_hz(m_freq.Value()*2);
				plib::pstring_vector_t pat(m_pattern.Value(),",");
				m_off = netlist_time(m_offset.Value());

				int pati[256];
				m_size = pat.size();
				int total = 0;
				for (unsigned i=0; i<m_size; i++)
				{
					pati[i] = pat[i].as_long();
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
			save(NLNAME(m_cnt));
			save(NLNAME(m_off));
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
		unsigned m_cnt;
		unsigned m_size;
		netlist_time m_inc[32];
		netlist_time m_off;
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
			set_logic_family(netlist().setup().family_from_model(m_FAMILY.Value()));
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
			OUTANALOG(m_Q, 0.0);
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
		, m_RIN(*this, "m_RIN")
		, m_ROUT(*this, "m_ROUT", true)
		, m_I(*this, "_I")
		, m_Q(*this, "_Q")
		, m_p_RIN(*this, "RIN", 1.0e6)
		, m_p_ROUT(*this, "ROUT", 50.0)

		{
			register_subalias("I", m_RIN.m_P);
			register_subalias("G", m_RIN.m_N);
			connect_late(m_I, m_RIN.m_P);

			register_subalias("_OP", m_ROUT.m_P);
			register_subalias("Q", m_ROUT.m_N);
			connect_late(m_Q, m_ROUT.m_P);
		}

		NETLIB_RESETI()
		{
			m_RIN.set(1.0 / m_p_RIN.Value(),0,0);
			m_ROUT.set(1.0 / m_p_ROUT.Value(),0,0);
		}

		NETLIB_UPDATEI()
		{
			OUTANALOG(m_Q, INPANALOG(m_I));
		}

	private:
		NETLIB_NAME(twoterm) m_RIN;
		/* Fixme: only works if the device is time-stepped - need to rework */
		NETLIB_NAME(twoterm) m_ROUT;
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
		, m_N(*this, "N", 2)
		, m_func(*this, "FUNC", "")
		, m_Q(*this, "Q")
		{

			for (int i=0; i < m_N; i++)
				m_I.emplace(i, *this, plib::pfmt("A{1}")(i));

			plib::pstring_vector_t cmds(m_func.Value(), " ");
			m_precompiled.clear();

			for (std::size_t i=0; i < cmds.size(); i++)
			{
				pstring cmd = cmds[i];
				rpn_inst rc;
				if (cmd == "+")
					rc.m_cmd = ADD;
				else if (cmd == "-")
					rc.m_cmd = SUB;
				else if (cmd == "*")
					rc.m_cmd = MULT;
				else if (cmd == "/")
					rc.m_cmd = DIV;
				else if (cmd.startsWith("A"))
				{
					rc.m_cmd = PUSH_INPUT;
					rc.m_param = cmd.substr(1).as_long();
				}
				else
				{
					bool err = false;
					rc.m_cmd = PUSH_CONST;
					rc.m_param = cmd.as_double(&err);
					if (err)
						netlist().log().fatal("nld_function: unknown/misformatted token <{1}> in <{2}>", cmd, m_func.Value());
				}
				m_precompiled.push_back(rc);
			}


		}

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:

		enum rpn_cmd
		{
			ADD,
			MULT,
			SUB,
			DIV,
			PUSH_CONST,
			PUSH_INPUT
		};

		struct rpn_inst
		{
			rpn_inst() : m_cmd(ADD), m_param(0.0) { }
			rpn_cmd m_cmd;
			nl_double m_param;
		};

		param_int_t m_N;
		param_str_t m_func;
		analog_output_t m_Q;
		plib::uninitialised_array_t<analog_input_t, 10> m_I;

		plib::pvector_t<rpn_inst> m_precompiled;
	};

	// -----------------------------------------------------------------------------
	// nld_res_sw
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(res_sw)
	{
	public:
		NETLIB_CONSTRUCTOR(res_sw)
		, m_R(*this, "R")
		, m_I(*this, "I")
		, m_RON(*this, "RON", 1.0)
		, m_ROFF(*this, "ROFF", 1.0E20)
		, m_last_state(0)
		{
			register_subalias("1", m_R.m_P);
			register_subalias("2", m_R.m_N);

			save(NLNAME(m_last_state));
		}

		NETLIB_SUB(R) m_R;
		logic_input_t m_I;
		param_double_t m_RON;
		param_double_t m_ROFF;

		NETLIB_RESETI()
		{
			m_last_state = 0;
			m_R.set_R(m_ROFF.Value());
		}
		//NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATEI();

	private:

		UINT8 m_last_state;
	};

	// -----------------------------------------------------------------------------
	// nld_base_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(base_proxy)
	{
	public:
		nld_base_proxy(netlist_t &anetlist, const pstring &name, logic_t *inout_proxied, core_terminal_t *proxy_inout)
				: device_t(anetlist, name)
		{
			m_logic_family = inout_proxied->logic_family();
			m_term_proxied = inout_proxied;
			m_proxy_term = proxy_inout;
		}

		virtual ~nld_base_proxy() {}

		logic_t &term_proxied() const { return *m_term_proxied; }
		core_terminal_t &proxy_term() const { return *m_proxy_term; }

	protected:

		virtual const logic_family_desc_t &logic_family() const
		{
			return *m_logic_family;
		}

	private:
		const logic_family_desc_t *m_logic_family;
		logic_t *m_term_proxied;
		core_terminal_t *m_proxy_term;
	};

	// -----------------------------------------------------------------------------
	// nld_a_to_d_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(a_to_d_proxy, base_proxy)
	{
	public:
		nld_a_to_d_proxy(netlist_t &anetlist, const pstring &name, logic_input_t *in_proxied)
				: nld_base_proxy(anetlist, name, in_proxied, &m_I)
		, m_I(*this, "I")
		, m_Q(*this, "Q")
		{
		}

		virtual ~nld_a_to_d_proxy() {}

		analog_input_t m_I;
		logic_output_t m_Q;

	protected:

		NETLIB_RESETI()	{ }

		NETLIB_UPDATEI()
		{
			if (m_I.Q_Analog() > logic_family().m_high_thresh_V)
				OUTLOGIC(m_Q, 1, NLTIME_FROM_NS(1));
			else if (m_I.Q_Analog() < logic_family().m_low_thresh_V)
				OUTLOGIC(m_Q, 0, NLTIME_FROM_NS(1));
			else
			{
				// do nothing
			}
		}
	private:
	};

	// -----------------------------------------------------------------------------
	// nld_base_d_to_a_proxy
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT_DERIVED(base_d_to_a_proxy, base_proxy)
	{
	public:
		virtual ~nld_base_d_to_a_proxy() {}

		virtual logic_input_t &in() { return m_I; }

	protected:
		nld_base_d_to_a_proxy(netlist_t &anetlist, const pstring &name, logic_output_t *out_proxied, core_terminal_t &proxy_out)
		: nld_base_proxy(anetlist, name, out_proxied, &proxy_out)
		, m_I(*this, "I")
		{
		}

		logic_input_t m_I;

	private:
	};

	NETLIB_OBJECT_DERIVED(d_to_a_proxy, base_d_to_a_proxy)
	{
	public:
		nld_d_to_a_proxy(netlist_t &anetlist, const pstring &name, logic_output_t *out_proxied)
		: nld_base_d_to_a_proxy(anetlist, name, out_proxied, m_RV.m_P)
		, m_GNDHack(*this, "_Q")
		, m_RV(*this, "RV")
		, m_last_state(-1)
		, m_is_timestep(false)
		{
			//register_sub(m_RV);
			//register_term("1", m_RV.m_P);
			//register_term("2", m_RV.m_N);

			register_subalias("Q", m_RV.m_P);

			connect_late(m_RV.m_N, m_GNDHack);

			save(NLNAME(m_last_state));
		}

		virtual ~nld_d_to_a_proxy() {}

	protected:

		NETLIB_RESETI();
		NETLIB_UPDATEI();

	private:
		analog_output_t m_GNDHack;  // FIXME: Long term, we need to connect proxy gnd to device gnd
		NETLIB_SUB(twoterm) m_RV;
		int m_last_state;
		bool m_is_timestep;
	};


	class factory_lib_entry_t : public base_factory_t
	{
		P_PREVENT_COPYING(factory_lib_entry_t)
	public:

		factory_lib_entry_t(setup_t &setup, const pstring &name, const pstring &classname,
				const pstring &def_param)
		: base_factory_t(name, classname, def_param), m_setup(setup) {  }

		class wrapper : public device_t
		{
		public:
			wrapper(const pstring &devname, netlist_t &anetlist, const pstring &name)
			: device_t(anetlist, name), m_devname(devname)
			{
				anetlist.setup().namespace_push(name);
				anetlist.setup().include(m_devname);
				anetlist.setup().namespace_pop();
			}
		protected:
			NETLIB_RESETI() { }
			NETLIB_UPDATEI() { }

			pstring m_devname;
		};

		plib::owned_ptr<device_t> Create(netlist_t &anetlist, const pstring &name) override
		{
			return plib::owned_ptr<device_t>::Create<wrapper>(this->name(), anetlist, name);
		}

	private:
		setup_t &m_setup;
	};


	} //namespace devices
} // namespace netlist

#endif /* NLD_SYSTEM_H_ */
