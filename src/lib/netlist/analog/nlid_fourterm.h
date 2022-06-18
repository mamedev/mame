// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NLID_FOURTERM_H_
#define NLID_FOURTERM_H_

///
/// \file nlid_fourterm.h
///

#include "nl_base.h"

#include "plib/putil.h"

namespace netlist::analog
{

	// ----------------------------------------------------------------------------------------
	// nld_VCCS
	// ----------------------------------------------------------------------------------------

	//
	//   Voltage controlled current source
	//
	//   IP ---+           +------> OP
	//         |           |
	//         RI          I
	//         RI => G =>  I    IOut = (V(IP)-V(IN)) * G
	//         RI          I
	//         |           |
	//   IN ---+           +------< ON
	//
	//   G=1 ==> 1V ==> 1A
	//
	//   RI = 1 / NETLIST_GMIN
	//
	class nld_VCCS : public base_device_t
	{
	public:
		nld_VCCS(constructor_param_t data, nl_fptype ri = nlconst::magic(1e9))
		: base_device_t(data)
		, m_G(*this, "G", nlconst::one())
		, m_RI(*this, "RI", ri)
		, m_OP(*this, "OP", &m_IP, {&m_ON, &m_IN},
			   NETLIB_DELEGATE(terminal_handler))
		, m_ON(*this, "ON", &m_IP, {&m_OP, &m_IN},
			   NETLIB_DELEGATE(terminal_handler))
		, m_IP(*this, "IP", &m_IN, {&m_OP, &m_ON},
			   NETLIB_DELEGATE(terminal_handler))
		, m_IN(*this, "IN", &m_IP, {&m_OP, &m_ON},
			   NETLIB_DELEGATE(terminal_handler))
		, m_OP1(*this, "_OP1", &m_IN, NETLIB_DELEGATE(terminal_handler))
		, m_ON1(*this, "_ON1", &m_IN, NETLIB_DELEGATE(terminal_handler))
		, m_gfac(nlconst::one())
		{
			connect(m_OP, m_OP1);
			connect(m_ON, m_ON1);
		}

		NETLIB_RESETI();

		param_fp_t m_G;
		param_fp_t m_RI;

	protected:
		NETLIB_HANDLERI(terminal_handler);
		NETLIB_UPDATE_PARAMI() { NETLIB_NAME(VCCS)::reset(); }

		void set_gfac(nl_fptype g) noexcept { m_gfac = g; }

		nl_fptype get_gfac() const noexcept { return m_gfac; }

		terminal_t m_OP;
		terminal_t m_ON;

		terminal_t m_IP;
		terminal_t m_IN;

		terminal_t m_OP1;
		terminal_t m_ON1;

	private:
		nl_fptype m_gfac;
	};

	// Limited Current source

	class nld_LVCCS : public nld_VCCS
	{
	public:
		nld_LVCCS(constructor_param_t data)
		: nld_VCCS(data)
		, m_cur_limit(*this, "CURLIM", nlconst::magic(1000.0))
		, m_vi(nlconst::zero())
		{
		}

		NETLIB_IS_DYNAMIC(true)

	protected:
		NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATE_TERMINALSI();

	private:
		param_fp_t m_cur_limit; // current limit
		nl_fptype  m_vi;
	};

	// ----------------------------------------------------------------------------------------
	// nld_CCCS
	// ----------------------------------------------------------------------------------------

	//
	//   Current controlled current source
	//
	//   IP ---+           +------> OP
	//         |           |
	//         RI          I
	//         RI => G =>  I    IOut = -(V(IP)-V(IN)) / RI  * G
	//         RI          I
	//         |           |
	//   IN ---+           +------< ON
	//
	//   G=1 ==> 1A ==> 1A
	//
	//   RI = 1
	//
	//   If current flows from IP to IN than output current flows from OP to ON
	//
	//   This needs high levels of accuracy to work with 1 Ohm RI.
	//

	class nld_CCCS : public nld_VCCS
	{
	public:
		nld_CCCS(constructor_param_t data)
		: nld_VCCS(data, nlconst::one())
		{
			set_gfac(-plib::reciprocal(m_RI()));
		}

		NETLIB_RESETI();

	protected:
		NETLIB_UPDATE_PARAMI();
	};

	// ----------------------------------------------------------------------------------------
	// nld_VCVS
	// ----------------------------------------------------------------------------------------

	//
	//   Voltage controlled voltage source
	//
	//   Parameters:
	//     G        Default: 1
	//     RO       Default: 1  (would be typically 50 for an op-amp
	//
	//   IP ---+           +--+---- OP
	//         |           |  |
	//         RI          I  RO
	//         RI => G =>  I  RO              V(OP) - V(ON) = (V(IP)-V(IN)) * G
	//         RI          I  RO
	//         |           |  |
	//   IN ---+           +--+---- ON
	//
	//   G=1 ==> 1V ==> 1V
	//
	//   RI = 1 / NETLIST_GMIN
	//
	//   Internal GI = G / RO
	//

	class nld_VCVS : public nld_VCCS
	{
	public:
		nld_VCVS(constructor_param_t data)
		: nld_VCCS(data)
		, m_RO(*this, "RO", nlconst::one())
		, m_OP2(*this, "_OP2", &m_ON2, NETLIB_DELEGATE(terminal_handler))
		, m_ON2(*this, "_ON2", &m_OP2, NETLIB_DELEGATE(terminal_handler))
		{
			connect(m_OP2, m_OP1);
			connect(m_ON2, m_ON1);
		}

		NETLIB_RESETI();

		param_fp_t m_RO;

	private:
		// NETLIB_UPDATE_PARAMI();
		NETLIB_HANDLERI(terminal_handler)
		{
			NETLIB_NAME(VCCS)::terminal_handler();
		}

		terminal_t m_OP2;
		terminal_t m_ON2;
	};

	// ----------------------------------------------------------------------------------------
	// nld_CCVS
	// ----------------------------------------------------------------------------------------

	//
	//   Voltage controlled voltage source
	//
	//   Parameters:
	//     G        Default: 1
	//     RO       Default: 1  (would be typically 50 for an op-amp
	//
	//   IP ---+           +--+---- OP
	//         |           |  |
	//         RI          I  RO
	//         RI => G =>  I  RO   V(OP) - V(ON) = (V(IP)-V(IN)) / RI * G
	//         RI          I  RO
	//         |           |  |
	//   IN ---+           +--+---- ON
	//
	//   G=1 ==> 1A ==> 1V
	//
	//   RI = 1
	//
	//   Internal GI = G / RO
	//

	class nld_CCVS : public nld_VCCS
	{
	public:
		nld_CCVS(constructor_param_t data)
		: nld_VCCS(data, nlconst::one())
		, m_RO(*this, "RO", nlconst::one())
		, m_OP2(*this, "_OP2", &m_ON2, NETLIB_DELEGATE(terminal_handler))
		, m_ON2(*this, "_ON2", &m_OP2, NETLIB_DELEGATE(terminal_handler))
		{
			connect(m_OP2, m_OP1);
			connect(m_ON2, m_ON1);
		}

		NETLIB_RESETI();

		param_fp_t m_RO;

	private:
		// NETLIB_UPDATE_PARAMI();

		NETLIB_HANDLERI(terminal_handler)
		{
			NETLIB_NAME(VCCS)::terminal_handler();
		}

		terminal_t m_OP2;
		terminal_t m_ON2;
	};

} // namespace netlist::analog

#endif // NLD_FOURTERM_H_
