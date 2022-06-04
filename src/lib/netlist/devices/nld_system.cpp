// license:BSD-3-Clause
// copyright-holders:Couriersud
/*
 * nld_system.cpp
 *
 */

#include "solver/nld_solver.h"
#include "nlid_system.h"
#include "solver/nld_matrix_solver.h"

#include "plib/pstrutil.h"

namespace netlist::devices {

	// -----------------------------------------------------------------------------
	// extclock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(extclock)
	{
		NETLIB_CONSTRUCTOR(extclock)
		, m_freq(*this, "FREQ", nlconst::magic(7159000.0 * 5.0))
		, m_pattern(*this, "PATTERN", "1,1")
		, m_offset(*this, "OFFSET", nlconst::zero())
		, m_feedback(*this, "FB", NETLIB_DELEGATE(first))
		, m_Q(*this, "Q")
		, m_cnt(*this, "m_cnt", 0)
		, m_off(*this, "m_off", netlist_time::zero())
		{
			m_inc[0] = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));

			connect("FB", "Q");

			netlist_time base = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
			std::vector<pstring> pat(plib::psplit(m_pattern(),','));
			m_off = netlist_time::from_fp(m_offset());

			std::array<std::int64_t, 32> pati = { 0 };

			m_size = static_cast<std::uint8_t>(pat.size());
			netlist_time::mult_type total = 0;
			for (unsigned i=0; i<m_size; i++)
			{
				pati[i] = plib::pstonum<std::int64_t>(pat[i]);
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

		NETLIB_RESETI()
		{
			m_cnt = 0;
			m_off = netlist_time::from_fp<param_fp_t::value_type>(m_offset());
			m_feedback.set_delegate(NETLIB_DELEGATE(first));
		}
		//NETLIB_UPDATE_PARAMI();

	private:

		NETLIB_HANDLERI(clk2)
		{
			m_Q.push((m_cnt & 1) ^ 1, m_inc[m_cnt]);
			if (++m_cnt >= m_size)
				m_cnt = 0;
		}

		NETLIB_HANDLERI(clk2_pow2)
		{
			m_Q.push((m_cnt & 1) ^ 1, m_inc[m_cnt]);
			m_cnt = (++m_cnt) & (m_size-1);
		}

		NETLIB_HANDLERI(first)
		{
			m_Q.push((m_cnt & 1) ^ 1, m_inc[m_cnt] + m_off());
			m_off = netlist_time::zero();
			if (++m_cnt >= m_size)
				m_cnt = 0;

			// continue with optimized clock handlers ....

			if ((m_size & (m_size-1)) == 0) // power of 2?
				m_feedback.set_delegate(nl_delegate(&NETLIB_NAME(extclock)::clk2_pow2, this));
			else
				m_feedback.set_delegate(nl_delegate(&NETLIB_NAME(extclock)::clk2, this));
		}


		param_fp_t m_freq;
		param_str_t m_pattern;
		param_fp_t m_offset;

		logic_input_t m_feedback;
		logic_output_t m_Q;
		state_var_u8 m_cnt;
		std::uint8_t m_size;
		state_var<netlist_time> m_off;
		std::array<netlist_time, 32> m_inc;
	};

	NETLIB_DEVICE_IMPL(netlistparams,       "PARAMETER",              "")
	NETLIB_DEVICE_IMPL(nc_pin,              "NC_PIN",                 "")

	NETLIB_DEVICE_IMPL(frontier,            "FRONTIER_DEV",           "+I,+G,+Q")
	NETLIB_DEVICE_IMPL(function,            "AFUNC",                  "N,FUNC")
	NETLIB_DEVICE_IMPL(analog_input,        "ANALOG_INPUT",           "IN")
	NETLIB_DEVICE_IMPL(clock,               "CLOCK",                  "FREQ")
	NETLIB_DEVICE_IMPL(varclock,            "VARCLOCK",               "N,FUNC")
	NETLIB_DEVICE_IMPL(extclock,            "EXTCLOCK",               "FREQ,PATTERN")
	NETLIB_DEVICE_IMPL(sys_dsw1,            "SYS_DSW",                "+I,+1,+2")
	NETLIB_DEVICE_IMPL(sys_dsw2,            "SYS_DSW2",               "")
	NETLIB_DEVICE_IMPL(sys_compd,           "SYS_COMPD",              "")

	using NETLIB_NAME(sys_noise_mt_u) =
		NETLIB_NAME(sys_noise)<plib::mt19937_64, plib::uniform_distribution_t>;
	NETLIB_DEVICE_IMPL(sys_noise_mt_u,      "SYS_NOISE_MT_U",         "SIGMA")

	using NETLIB_NAME(sys_noise_mt_n) =
		NETLIB_NAME(sys_noise)<plib::mt19937_64, plib::normal_distribution_t>;
	NETLIB_DEVICE_IMPL(sys_noise_mt_n,      "SYS_NOISE_MT_N",         "SIGMA")

	NETLIB_DEVICE_IMPL(mainclock,           "MAINCLOCK",              "FREQ")
	NETLIB_DEVICE_IMPL(gnd,                 "GNDA",                   "")

	using NETLIB_NAME(logic_input8) = NETLIB_NAME(logic_inputN)<8>;
	NETLIB_DEVICE_IMPL(logic_input8,         "LOGIC_INPUT8",            "IN,MODEL")

	NETLIB_DEVICE_IMPL(logic_input,         "LOGIC_INPUT",            "IN,MODEL")
	NETLIB_DEVICE_IMPL_ALIAS(logic_input_ttl, logic_input, "TTL_INPUT", "IN")

} // namespace netlist::devices
