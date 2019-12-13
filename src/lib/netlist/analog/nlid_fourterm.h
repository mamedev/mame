// license:GPL-2.0+
// copyright-holders:Couriersud

#ifndef NLID_FOURTERM_H_
#define NLID_FOURTERM_H_

///
/// \file nlid_fourterm.h
///

#include "netlist/nl_base.h"
#include "plib/putil.h"

namespace netlist {
namespace analog {

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
	NETLIB_OBJECT(VCCS)
	{
	public:
		NETLIB_CONSTRUCTOR(VCCS)
		, m_G(*this, "G", nlconst::one())
		, m_RI(*this, "RI", nlconst::magic(1e9))
		, m_OP(*this, "OP", &m_IP)
		, m_ON(*this, "ON", &m_IP)
		, m_IP(*this, "IP", &m_IN)   // <= this should be NULL and terminal be filtered out prior to solving...
		, m_IN(*this, "IN", &m_IP)   // <= this should be NULL and terminal be filtered out prior to solving...
		, m_OP1(*this, "_OP1", &m_IN)
		, m_ON1(*this, "_ON1", &m_IN)
		, m_gfac(nlconst::one())
		{
			connect(m_OP, m_OP1);
			connect(m_ON, m_ON1);
			m_gfac = nlconst::one();
		}

		NETLIB_RESETI();

		param_fp_t m_G;
		param_fp_t m_RI;

	protected:
		NETLIB_UPDATEI();
		NETLIB_UPDATE_PARAMI()
		{
			NETLIB_NAME(VCCS)::reset();
		}


		terminal_t m_OP;
		terminal_t m_ON;

		terminal_t m_IP;
		terminal_t m_IN;

		terminal_t m_OP1;
		terminal_t m_ON1;

		nl_fptype m_gfac;
	};

	// Limited Current source

	NETLIB_OBJECT_DERIVED(LVCCS, VCCS)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(LVCCS, VCCS)
		, m_cur_limit(*this, "CURLIM", nlconst::magic(1000.0))
		, m_vi(nlconst::zero())
		{
		}

		NETLIB_IS_DYNAMIC(true)

	protected:
		//NETLIB_UPDATEI();
		NETLIB_RESETI();
		NETLIB_UPDATE_PARAMI();
		NETLIB_UPDATE_TERMINALSI();

	private:
		param_fp_t m_cur_limit; // current limit
		nl_fptype m_vi;
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
	//         RI => G =>  I    IOut = (V(IP)-V(IN)) / RI  * G
	//         RI          I
	//         |           |
	//   IN ---+           +------< ON
	//
	//   G=1 ==> 1A ==> 1A
	//
	//   RI = 1
	//
	//   This needs high levels of accuracy to work with 1 Ohm RI.
	//

	NETLIB_OBJECT_DERIVED(CCCS, VCCS)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(CCCS, VCCS)
		{
			m_gfac = plib::reciprocal(m_RI());
		}

		NETLIB_RESETI();

	protected:
		//NETLIB_UPDATEI();
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

	NETLIB_OBJECT_DERIVED(VCVS, VCCS)
	{
	public:
		NETLIB_CONSTRUCTOR_DERIVED(VCVS, VCCS)
		, m_RO(*this, "RO", nlconst::one())
		, m_OP2(*this, "_OP2", &m_ON2)
		, m_ON2(*this, "_ON2", &m_OP2)
		{
			connect(m_OP2, m_OP1);
			connect(m_ON2, m_ON1);
		}

		NETLIB_RESETI();

		param_fp_t m_RO;

	private:
		//NETLIB_UPDATEI();
		//NETLIB_UPDATE_PARAMI();

		terminal_t m_OP2;
		terminal_t m_ON2;


	};

} // namespace analog
} // namespace netlist

#endif // NLD_FOURTERM_H_
