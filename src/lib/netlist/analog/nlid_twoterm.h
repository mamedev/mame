// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLID_TWOTERM_H_
#define NLID_TWOTERM_H_

///
/// \file nlid_twoterm.h
///
/// Devices with two terminals ...
///
///
///       (k)
///  +-----T-----+
///  |     |     |
///  |  +--+--+  |
///  |  |     |  |
///  |  R     |  |
///  |  R     |  |
///  |  R     I  |
///  |  |     I  |  Device n
///  |  V+    I  |
///  |  V     |  |
///  |  V-    |  |
///  |  |     |  |
///  |  +--+--+  |
///  |     |     |
///  +-----T-----+
///       (l)
///
///  This is a resistance in series to a voltage source and paralleled by a
///  current source. This is suitable to model voltage sources, current sources,
///  resistors, capacitors, inductances and diodes.
///
//

#include "../nl_setup.h"
#include "nl_base.h"
#include "nld_generic_models.h"

#include "solver/nld_solver.h"

#include "plib/pfunction.h"

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

namespace netlist::analog
{

	// -------------------------------------------------------------------------
	// nld_two_terminal
	// -------------------------------------------------------------------------

	class nld_two_terminal : public base_device_t
	{
	public:
		nld_two_terminal(constructor_param_t data)
		: base_device_t(data)
		, m_P(*this, "1", &m_N, NETLIB_DELEGATE(terminal_handler))
		, m_N(*this, "2", &m_P, NETLIB_DELEGATE(terminal_handler))
		{
		}

		// This constructor covers the case in which the terminals are "owned"
		// by the device using a two_terminal. In this case it passes
		// the terminal handler on to the terminals.

		nld_two_terminal(base_device_t &owner, const pstring &name,
						 nl_delegate owner_delegate)
		: base_device_t(
			constructor_data_t{owner.state(), owner.name() + "." + name})
		, m_P(owner, name + ".1", &m_N, owner_delegate)
		, m_N(owner, name + ".2", &m_P, owner_delegate)
		{
		}

		// NETLIB_UPDATE_TERMINALSI() { }
		// NETLIB_RESETI() {}

	public:
		NETLIB_HANDLERI(terminal_handler);

		solver::matrix_solver_t *solver() const noexcept;

		void solve_now() const;

		template <typename F>
		void change_state(F f) const
		{
			auto *solv(solver());
			if (solv)
				solv->change_state(f);
		}

		void set_G_V_I(nl_fptype G, nl_fptype V, nl_fptype I) const noexcept
		{
			//               GO, GT,        I
			m_P.set_go_gt_I( -G,  G, (  V) * G - I);
			m_N.set_go_gt_I( -G,  G, ( -V) * G + I);
		}

		nl_fptype deltaV() const noexcept
		{
			return m_P.net().Q_Analog() - m_N.net().Q_Analog();
		}

		nl_fptype V1P() const noexcept { return m_P.net().Q_Analog(); }

		nl_fptype V2N() const noexcept { return m_N.net().Q_Analog(); }

		void set_mat(nl_fptype a11, nl_fptype a12, nl_fptype rhs1, //
					 nl_fptype a21, nl_fptype a22, nl_fptype rhs2  //
		) const noexcept
		{
			//               GO,  GT,     I
			m_P.set_go_gt_I(a12, a11, rhs1);
			m_N.set_go_gt_I(a21, a22, rhs2);
		}

		void set_mat(const std::array<std::array<nl_fptype,3>,2> &a) const noexcept
		{
			//                 GO,      GT,       I
			m_P.set_go_gt_I(a[0][1], a[0][0], a[0][2]);
			m_N.set_go_gt_I(a[1][0], a[1][1], a[1][2]);
		}

		void clear_mat() const noexcept
		{
			const auto z = nlconst::zero();
			//               GO,  GT,     I
			m_P.set_go_gt_I(z, z, z);
			m_N.set_go_gt_I(z, z, z);
		}

		/// \brief Get a const reference to the m_P terminal
		///
		/// This is typically called during initialization to connect
		/// terminals.
		///
		/// \returns Reference to m_P terminal.
		const terminal_t &P() const noexcept { return m_P; }

		/// \brief Get a const reference to the m_N terminal
		///
		/// This is typically called during initialization to connect
		/// terminals.
		///
		/// \returns Reference to m_N terminal.
		const terminal_t &N() const noexcept { return m_N; }

		/// \brief Get a reference to the m_P terminal
		///
		/// This call is only allowed from the core. Device code should never
		/// need to call this.
		///
		/// \returns Reference to m_P terminal.
		terminal_t &setup_P() noexcept { return m_P; }

		/// \brief Get a reference to the m_N terminal
		///
		/// This call is only allowed from the core. Device code should never
		/// need to call this.
		///
		/// \returns Reference to m_P terminal.
		terminal_t &setup_N() noexcept { return m_N; }

	private:
		terminal_t m_P;
		terminal_t m_N;
	};

	// -------------------------------------------------------------------------
	// nld_R
	// -------------------------------------------------------------------------

	class nld_R_base : public nld_two_terminal
	{
	public:
		nld_R_base(constructor_param_t data)
		: nld_two_terminal(data)
		{
		}

		void set_R(nl_fptype R) const noexcept
		{
			const nl_fptype G = plib::reciprocal(R);
			set_mat(G, -G, nlconst::zero(), //
					-G, G, nlconst::zero());
		}

		void set_G(nl_fptype G) const noexcept
		{
			set_mat(G, -G, nlconst::zero(), //
					-G, G, nlconst::zero());
		}

		// NETLIB_RESETI();

	protected:
		// NETLIB_UPDATEI();
	};

	class nld_R : public nld_R_base
	{
	public:
		nld_R(constructor_param_t data)
		: nld_R_base(data)
		, m_R(*this, "R", nlconst::magic(1e9))
		{
		}

	protected:
		NETLIB_RESETI() { set_R(std::max(m_R(), exec().gmin())); }

		NETLIB_UPDATE_PARAMI()
		{
			// FIXME: We only need to update the net first if this is a time
			// stepping net
			change_state([this]() { set_R(std::max(m_R(), exec().gmin())); });
		}

	private:
		param_fp_t m_R;
		// protect set_R ... it's a recipe to disaster when used to bypass the
		// parameter
		using nld_R_base::set_G;
		using nld_R_base::set_R;
	};

	// -------------------------------------------------------------------------
	// nld_POT
	// -------------------------------------------------------------------------

	class nld_POT : public base_device_t
	{
	public:
		nld_POT(constructor_param_t data)
		: base_device_t(data)
		, m_R1(*this, "_R1")
		, m_R2(*this, "_R2")
		, m_R(*this, "R", 10000)
		, m_Dial(*this, "DIAL", nlconst::half())
		, m_DialIsLog(*this, "DIALLOG", false)
		, m_Reverse(*this, "REVERSE", false)
		{
			register_sub_alias("1", m_R1().P());
			register_sub_alias("2", m_R1().N());
			register_sub_alias("3", m_R2().N());

			connect(m_R2().P(), m_R1().N());
		}

		// NETLIB_UPDATEI();
		NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();

	private:
		NETLIB_SUB_NS(analog, R_base) m_R1;
		NETLIB_SUB_NS(analog, R_base) m_R2;

		param_fp_t    m_R;
		param_fp_t    m_Dial;
		param_logic_t m_DialIsLog;
		param_logic_t m_Reverse;
	};

	class nld_POT2 : public base_device_t
	{
	public:
		nld_POT2(constructor_param_t data)
		: base_device_t(data)
		, m_R1(*this, "_R1")
		, m_R(*this, "R", nlconst::magic(10000.0))
		, m_Dial(*this, "DIAL", nlconst::half())
		, m_DialIsLog(*this, "DIALLOG", false)
		, m_Reverse(*this, "REVERSE", false)
		{
			register_sub_alias("1", m_R1().P());
			register_sub_alias("2", m_R1().N());
		}

		// NETLIB_UPDATEI();
		NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();

	private:
		NETLIB_SUB_NS(analog, R_base) m_R1;

		param_fp_t    m_R;
		param_fp_t    m_Dial;
		param_logic_t m_DialIsLog;
		param_logic_t m_Reverse;
	};

	// -------------------------------------------------------------------------
	// nld_C
	// -------------------------------------------------------------------------

#if 1
	class nld_C : public nld_two_terminal
	{
	public:
		nld_C(constructor_param_t data)
		: nld_two_terminal(data)
		, m_C(*this, "C", nlconst::magic(1e-6))
		, m_cap(*this, "m_cap")
		{
		}

		NETLIB_IS_TIMESTEP(true)
		NETLIB_TIMESTEPI()
		{
			if (ts_type == time_step_type::FORWARD)
			{
				// G, Ieq
				const auto      res(m_cap.time_step(m_C(), deltaV(), step));
				const nl_fptype G = res.first;
				const nl_fptype I = res.second;
				set_mat(G, -G, -I, //
						-G, G, I);
			}
			else
				m_cap.restore_state();
		}

		NETLIB_RESETI() { m_cap.set_parameters(exec().gmin()); }

		/// \brief Set capacitance
		///
		/// This call will set the capacitance. The typical use case are
		/// are components like BJTs which use this component to model
		/// internal capacitances. Typically called during initialization.
		///
		/// \param val Capacitance value
		///
		void set_cap_embedded(nl_fptype val) { m_C.set(val); }

	protected:
		// NETLIB_UPDATEI();
		// FIXME: should be able to change
		NETLIB_UPDATE_PARAMI() {}

	private:
		param_fp_t              m_C;
		generic_capacitor_const m_cap;
	};

#else
	// Code preserved as a basis for a current/voltage controlled capacitor
	class nld_C : public nld_two_terminal)
	{
	public:
		nld_C(constructor_param_t data)
		: nld_two_terminal(data)
		, m_C(*this, "C", nlconst::magic(1e-6))
		, m_cap(*this, "m_cap")
		{
		}

		NETLIB_IS_TIMESTEP(true)
		NETLIB_TIMESTEPI()
		{
			m_cap.time_step(m_C(), deltaV(), step);
			if (m_cap.type() == capacitor_e::CONSTANT_CAPACITY)
			{
				const nl_fptype I = m_cap.Ieq(m_C(), deltaV());
				const nl_fptype G = m_cap.G(m_C());
				set_mat(G, -G, -I, //
						-G, G, I);
			}
		}

		NETLIB_IS_DYNAMIC(m_cap.type() == capacitor_e::VARIABLE_CAPACITY)
		NETLIB_UPDATE_TERMINALSI()
		{
			const nl_fptype I = m_cap.Ieq(m_C(), deltaV());
			const nl_fptype G = m_cap.G(m_C());
			set_mat(G, -G, -I, //
					-G, G, I);
		}

		param_fp_t m_C;
		NETLIB_RESETI() { m_cap.set_parameters(exec().gmin()); }

	protected:
		// NETLIB_UPDATEI();
		// FIXME: should be able to change
		NETLIB_UPDATE_PARAMI() {}

	private:
		// generic_capacitor<capacitor_e::VARIABLE_CAPACITY> m_cap;
		generic_capacitor<capacitor_e::CONSTANT_CAPACITY> m_cap;
	};
#endif

	// -------------------------------------------------------------------------
	// nld_L
	// -------------------------------------------------------------------------

	class nld_L : public nld_two_terminal
	{
	public:
		nld_L(constructor_param_t data)
		: nld_two_terminal(data)
		, m_L(*this, "L", nlconst::magic(1e-6))
		, m_gmin(nlconst::zero())
		, m_G(*this, "m_G", nlconst::zero())
		, m_I(*this, "m_I", nlconst::zero())
		, m_last_I(*this, "m_last_I", nlconst::zero())
		, m_last_G(*this, "m_last_G", nlconst::zero())
		{
			// register_term("1", m_P);
			// register_term("2", m_N);
		}

		NETLIB_IS_TIMESTEP(true)
		NETLIB_TIMESTEPI();
		NETLIB_RESETI();

	protected:
		// NETLIB_UPDATEI();
		NETLIB_UPDATE_PARAMI();

	private:
		param_fp_t m_L;

		nl_fptype            m_gmin;
		state_var<nl_fptype> m_G;
		state_var<nl_fptype> m_I;
		state_var<nl_fptype> m_last_I;
		state_var<nl_fptype> m_last_G;
	};

	/// \brief Class representing the diode model parameters.
	///
	///  This is the model representation of the diode model. Typically, SPICE
	///  uses the following parameters. A "Y" in the first column indicates that
	///  the parameter is actually used in netlist.
	///
	///  NBV, BV and IBV are only used in the ZDIODE model. It is assumed
	///  that DIODEs are not modeled up to their breakdown voltage.
	///
	///   |NL? |name  |parameter                        |units|default| example|area  |
	///   |:--:|:-----|:--------------------------------|:----|------:|-------:|:----:|
	///   | Y  |IS    |saturation current               |A    |1.0e-14| 1.0e-14|   *  |
	///   |    |RS    |ohmic resistance                 |Ohm  |      0|      10|   *  |
	///   | Y  |N     |emission coefficient             |-    |      1|       1|      |
	///   |    |TT    |transit-time                     |sec  |      0|   0.1ns|      |
	///   |    |CJO   |zero-bias junction capacitance   |F    |      0|     2pF|   *  |
	///   |    |VJ    |junction potential               |V    |      1|     0.6|      |
	///   |    |M     |grading coefficient              |-    |    0.5|     0.5|      |
	///   |    |EG    |band-gap energy                  |eV   |   1.11| 1.11 Si|      |
	///   |    |XTI   |saturation-current temp.exp      |-    |      3|3.0 pn. 2.0 Schottky| |
	///   |    |KF    |flicker noise coefficient        |-    |      0|        |      |
	///   |    |AF    |flicker noise exponent           |-    |      1|        |      |
	///   |    |FC    |coefficient for forward-bias depletion capacitance formula|-|0.5|| |
	///   | Y  |NBV   |reverse emission coefficient     |-    |      3|       1|      |
	///   | Y  |BV    |reverse breakdown voltage        |V    |infinite|     40|      |
	///   | Y  |IBV   |current at breakdown voltage     |A    |  0.001|        |      |
	///   |    |TNOM  |parameter measurement temperature|deg C|     27|      50|      |
	///
	class diode_model_t
	{
	public:
		diode_model_t(param_model_t &model)
		: m_IS(model, "IS")
		, m_N(model, "N")
		{
		}

		param_model_t::value_t m_IS; //!< saturation current.
		param_model_t::value_t m_N;  //!< emission coefficient.
	};

	class zdiode_model_t : public diode_model_t
	{
	public:
		zdiode_model_t(param_model_t &model)
		: diode_model_t(model)
		, m_NBV(model, "NBV")
		, m_BV(model, "BV")
		, m_IBV(model, "IBV")
		{
		}

		param_model_t::value_t m_NBV; //!< reverse emission coefficient.
		param_model_t::value_t m_BV;  //!< reverse breakdown voltage.
		param_model_t::value_t m_IBV; //!< current at breakdown voltage.
	};

	// -------------------------------------------------------------------------
	// nld_D
	// -------------------------------------------------------------------------

	class nld_D : public nld_two_terminal
	{
	public:
		nld_D(constructor_param_t data, const pstring &model = "D")
		: nld_two_terminal(data)
		, m_model(*this, "MODEL", model)
		, m_modacc(m_model)
		, m_D(*this, "m_D")
		{
			register_sub_alias("A", P());
			register_sub_alias("K", N());
		}

		NETLIB_IS_DYNAMIC(true)
		NETLIB_UPDATE_TERMINALSI();
		NETLIB_RESETI();

	protected:
		// NETLIB_UPDATEI();
		NETLIB_UPDATE_PARAMI();

	private:
		param_model_t                   m_model;
		diode_model_t                   m_modacc;
		generic_diode<diode_e::BIPOLAR> m_D;
	};

	// -------------------------------------------------------------------------
	// nld_Z - Zener Diode
	// -------------------------------------------------------------------------

	class nld_Z : public nld_two_terminal
	{
	public:
		nld_Z(constructor_param_t data, const pstring &model = "D")
		: nld_two_terminal(data)
		, m_model(*this, "MODEL", model)
		, m_modacc(m_model)
		, m_D(*this, "m_D")
		, m_R(*this, "m_R")
		{
			register_sub_alias("A", P());
			register_sub_alias("K", N());
		}

		NETLIB_IS_DYNAMIC(true)
		NETLIB_UPDATE_TERMINALSI();
		NETLIB_RESETI();

	protected:
		// NETLIB_UPDATEI();
		NETLIB_UPDATE_PARAMI();

	private:
		param_model_t                   m_model;
		zdiode_model_t                  m_modacc;
		generic_diode<diode_e::BIPOLAR> m_D;
		// REVERSE diode
		generic_diode<diode_e::BIPOLAR> m_R;
	};

	// -------------------------------------------------------------------------
	// nld_VS - Voltage source
	//
	// netlist voltage source must have inner resistance
	// -------------------------------------------------------------------------

	class nld_VS : public nld_two_terminal
	{
	public:
		nld_VS(constructor_param_t data)
		: nld_two_terminal(data)
		, m_t(*this, "m_t", nlconst::zero())
		, m_R(*this, "RI", nlconst::magic(0.1))
		, m_V(*this, "V", nlconst::zero())
		, m_func(*this, "FUNC", "")
		, m_compiled(*this, "m_compiled")
		, m_funcparam({nlconst::zero()})
		{
			register_sub_alias("P", P());
			register_sub_alias("N", N());
			if (!m_func().empty())
				m_compiled->compile(m_func(),
									std::vector<pstring>({{pstring("T")}}));
		}

		NETLIB_IS_TIMESTEP(!m_func().empty())

		NETLIB_TIMESTEPI()
		{
			if (ts_type == time_step_type::FORWARD)
			{
				m_t += step;
				m_funcparam[0] = m_t;
				this->set_G_V_I(plib::reciprocal(m_R()),
								m_compiled->evaluate(m_funcparam),
								nlconst::zero());
			}
			else
				m_t -= step; // only need to restore state, will be called again
		}

	protected:
		NETLIB_RESETI()
		{
			nld_two_terminal::reset();
			this->set_G_V_I(plib::reciprocal(m_R()), m_V(), nlconst::zero());
		}

	private:
		state_var<nl_fptype>                  m_t;
		param_fp_t                            m_R;
		param_fp_t                            m_V;
		param_str_t                           m_func;
		state_var<plib::pfunction<nl_fptype>> m_compiled;
		std::vector<nl_fptype>                m_funcparam;
	};

	// -------------------------------------------------------------------------
	// nld_CS - Current source
	// -------------------------------------------------------------------------

	class nld_CS : public nld_two_terminal
	{
	public:
		nld_CS(constructor_param_t data)
		: nld_two_terminal(data)
		, m_t(*this, "m_t", nlconst::zero())
		, m_I(*this, "I", nlconst::one())
		, m_func(*this, "FUNC", "")
		, m_compiled(*this, "m_compiled")
		, m_funcparam({nlconst::zero()})
		{
			register_sub_alias("P", "1");
			register_sub_alias("N", "2");
			if (!m_func().empty())
				m_compiled->compile(m_func(),
									std::vector<pstring>({{pstring("T")}}));
		}

		NETLIB_IS_TIMESTEP(!m_func().empty())
		NETLIB_TIMESTEPI()
		{
			if (ts_type == time_step_type::FORWARD)
			{
				m_t += step;
				m_funcparam[0] = m_t;
				const nl_fptype I = m_compiled->evaluate(m_funcparam);
				const auto      zero(nlconst::zero());
				set_mat(zero, zero, -I, //
						zero, zero, I);
			}
			else
				m_t -= step;
		}

	protected:
		NETLIB_RESETI()
		{
			nld_two_terminal::reset();
			const auto zero(nlconst::zero());
			set_mat(zero, zero, -m_I(), //
					zero, zero, m_I());
		}

		NETLIB_UPDATE_PARAMI()
		{
			// FIXME: We only need to update the net first if this is a time
			// stepping net
			// FIXME: works only for CS without function
			change_state(
				[this]()
				{
					const auto zero(nlconst::zero());
					set_mat(zero, zero, -m_I(), //
							zero, zero, m_I());
				});
		}

	private:
		state_var<nl_fptype>                  m_t;
		param_fp_t                            m_I;
		param_str_t                           m_func;
		state_var<plib::pfunction<nl_fptype>> m_compiled;
		std::vector<nl_fptype>                m_funcparam;
	};

} // namespace netlist::analog

#endif // NLD_TWOTERM_H_
