// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nlid_proxy.cpp
 *
 */

#include "nlid_proxy.h"
#include "netlist/solver/nld_solver.h"

namespace netlist
{
	namespace devices
	{

	// -----------------------------------------------------------------------------
	// nld_base_proxy
	// -----------------------------------------------------------------------------

	nld_base_proxy::nld_base_proxy(netlist_state_t &anetlist, const pstring &name,
		const logic_t *inout_proxied)
		: device_t(anetlist, name)
		, m_tp(nullptr)
		, m_tn(nullptr)
	{
		set_logic_family(inout_proxied->logic_family());

		const std::vector<std::pair<pstring, pstring>> power_syms = { {"VCC", "VEE"}, {"VCC", "GND"}, {"VDD", "VSS"}};

		bool f = false;
		for (const auto & pwr_sym : power_syms)
		{
			pstring devname = inout_proxied->device().name();

			auto *tp_ct(anetlist.setup().find_terminal(devname + "." + pwr_sym.first,
					/*detail::terminal_type::INPUT,*/ false));
			auto *tp_cn(anetlist.setup().find_terminal(devname + "." + pwr_sym.second,
				/*detail::terminal_type::INPUT,*/ false));
			if (tp_ct && tp_cn)
			{
				if (tp_ct && !tp_ct->is_analog())
					throw nl_exception(plib::pfmt("Not an analog terminal: {1}")(tp_ct->name()));
				if (tp_cn && !tp_cn->is_analog())
					throw nl_exception(plib::pfmt("Not an analog terminal: {1}")(tp_cn->name()));

				auto *tp_t = static_cast<analog_t* >(tp_ct);
				auto *tn_t = static_cast<analog_t *>(tp_cn);
				if (f && (tp_t != nullptr && tn_t != nullptr))
					log().warning(MI_MULTIPLE_POWER_TERMINALS_ON_DEVICE(inout_proxied->device().name(),
						m_tp->name(), m_tn->name(),
						tp_t ? tp_t->name() : "",
						tn_t ? tn_t->name() : ""));
				else if (tp_t != nullptr && tn_t != nullptr)
				{
					m_tp = tp_t;
					m_tn = tn_t;
					f = true;
				}
			}
		}
		if (!f)
			log().error(MI_NO_POWER_TERMINALS_ON_DEVICE_2(name, anetlist.setup().de_alias(inout_proxied->device().name())));
		else
			log().verbose("D/A Proxy: Found power terminals on device {1}", inout_proxied->device().name());
	}

	// ----------------------------------------------------------------------------------------
	// nld_a_to_d_proxy
	// ----------------------------------------------------------------------------------------

	nld_base_a_to_d_proxy::nld_base_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name,
			const logic_input_t *in_proxied)
	: nld_base_proxy(anetlist, name, in_proxied)
	{
	}

	nld_a_to_d_proxy::nld_a_to_d_proxy(netlist_state_t &anetlist, const pstring &name, const logic_input_t *in_proxied)
	: nld_base_a_to_d_proxy(anetlist, name, in_proxied)
	, m_Q(*this, "Q")
	, m_I(*this, "I")
	{
	}

	NETLIB_RESET(a_to_d_proxy)
	{
	}

	NETLIB_UPDATE(a_to_d_proxy)
	{
		const auto v(m_I.Q_Analog());
		const auto vn(m_tn->net().Q_Analog());
		const auto vp(m_tp->net().Q_Analog());

		if (logic_family()->is_above_high_thresh_V(v, vn, vp))
			out().push(1, netlist_time::quantum());
		else if (logic_family()->is_below_low_thresh_V(v, vn, vp))
			out().push(0, netlist_time::quantum());
		else
		{
			// do nothing
		}
	}

	// ----------------------------------------------------------------------------------------
	// nld_d_to_a_proxy
	// ----------------------------------------------------------------------------------------

	nld_base_d_to_a_proxy::nld_base_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name,
			const logic_output_t *out_proxied)
	: nld_base_proxy(anetlist, name, out_proxied)
	{
	}

	nld_d_to_a_proxy::nld_d_to_a_proxy(netlist_state_t &anetlist, const pstring &name, const logic_output_t *out_proxied)
	: nld_base_d_to_a_proxy(anetlist, name, out_proxied)
	, m_I(*this, "I")
	, m_RP(*this, "RP")
	, m_RN(*this, "RN")
	, m_last_state(*this, "m_last_var", -1)
	{
		register_subalias("Q", m_RN.P());

		log().verbose("D/A Proxy: Found power terminals on device {1}", out_proxied->device().name());
		if (anetlist.is_extended_validation())
		{
			// During validation, don't connect to terminals found
			// This will cause terminals not connected to a rail net to
			// fail connection stage.
			connect(m_RN.N(), m_RP.P());
		}
		else
		{
			connect(m_RN.N(), *m_tn);
			connect(m_RP.P(), *m_tp);
		}
		connect(m_RN.P(), m_RP.N());
	}


	void nld_d_to_a_proxy::reset()
	{
		//m_Q.initial(0.0);
		m_last_state = -1;
		m_RN.reset();
		m_RP.reset();
		m_RN.set_G_V_I(plib::reciprocal(logic_family()->R_low()),
				logic_family()->low_offset_V(), nlconst::zero());
		m_RP.set_G_V_I(G_OFF,
			nlconst::zero(),
			nlconst::zero());
	}

	NETLIB_UPDATE(d_to_a_proxy)
	{
		const auto state = static_cast<int>(m_I());
		if (state != m_last_state)
		{
			// RN, RP are connected ...
			m_RN.change_state([this, &state]()
			{
				if (state)
				{
					m_RN.set_G_V_I(G_OFF,
						nlconst::zero(),
						nlconst::zero());
					m_RP.set_G_V_I(plib::reciprocal(logic_family()->R_high()),
							logic_family()->high_offset_V(), nlconst::zero());
				}
				else
				{
					m_RN.set_G_V_I(plib::reciprocal(logic_family()->R_low()),
							logic_family()->low_offset_V(), nlconst::zero());
					m_RP.set_G_V_I(G_OFF,
						nlconst::zero(),
						nlconst::zero());
				}
			});
			m_last_state = state;
		}
	}


	} //namespace devices
} // namespace netlist
