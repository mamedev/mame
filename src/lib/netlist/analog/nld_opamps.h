// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_opamps.h
 *
 */

//#pragma once

#ifndef NLD_OPAMPS_H_
#define NLD_OPAMPS_H_

#include "nl_base.h"
#include "nld_twoterm.h"
#include "nld_fourterm.h"

// ----------------------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------------------

#ifndef NL_AUTO_DEVICES

#define OPAMP(name, model)                                                     \
		NET_REGISTER_DEV(OPAMP, name)                                          \
		NETDEV_PARAMI(name, MODEL, model)

#endif
// ----------------------------------------------------------------------------------------
// Devices ...
// ----------------------------------------------------------------------------------------

namespace netlist
{
	namespace analog
	{

	/*! Class representing the opamp model parameters.
	 *  The opamp model was designed based on designs from
	 *  http://www.ecircuitcenter.com/Circuits/opmodel1/opmodel1.htm.
	 *  Currently 2 different types are supported: Type 1 and Type 3. Type 1
	 *  is less complex and should run faster than Type 3.
	 *
	 *  This is an extension to the traditional SPICE approach which
	 *  assumes that you will be using an manufacturer model. These models may
	 *  have copyrights incompatible with the netlist license. Thus they may not
	 *  be suitable for certain implementations of netlist.
	 *
	 *  For the typical use cases in low frequency (< 100 KHz) applications at
	 *  which netlist is targeted, this model is certainly suitable. All parameters
	 *  can be determined from a typical opamp datasheet.
	 *
	 *   |Type|name  |parameter                                      |units|default| example|
	 *   |:--:|:-----|:----------------------------------------------|:----|------:|-------:|
	 *   |  3 |TYPE  |Model Type, 1 and 3 are supported              |     |       |        |
	 *   |1,3 |FPF   |frequency of first pole                        |Hz   |       |100     |
	 *   |  3 |SLEW  |unity gain slew rate                           |V/s  |       |       1|
	 *   |1,3 |RI    |input resistance                               |Ohm  |       |1M      |
	 *   |1,3 |RO    |output resistance                              |Ohm  |       |50      |
	 *   |1,3 |UGF   |unity gain frequency (transition frequency)    |Hz   |       |1000    |
	 *   |  3 |VLL   |low output swing minus low supply rail         |V    |       |1.5     |
	 *   |  3 |VLH   |high supply rail minus high output swing       |V    |       |1.5     |
	 *   |  3 |DAB   |Differential Amp Bias - total quiescent current|A    |       |0.001   |
	 */

	class opamp_model_t : public param_model_t
	{
	public:
		opamp_model_t(device_t &device, const pstring name, const pstring val)
		: param_model_t(device, name, val)
		, m_TYPE(*this, "TYPE")
		, m_FPF(*this, "FPF")
		, m_SLEW(*this, "SLEW")
		, m_RI(*this, "RI")
		, m_RO(*this, "RO")
		, m_UGF(*this, "UGF")
		, m_VLL(*this, "VLL")
		, m_VLH(*this, "VLH")
		, m_DAB(*this, "DAB")
		{}

		value_t m_TYPE;   //!< Model Type, 1 and 3 are supported
		value_t m_FPF;    //!< frequency of first pole
		value_t m_SLEW;   //!< unity gain slew rate
		value_t m_RI;     //!< input resistance
		value_t m_RO;     //!< output resistance
		value_t m_UGF;    //!< unity gain frequency (transition frequency)
		value_t m_VLL;    //!< low output swing minus low supply rail
		value_t m_VLH;    //!< high supply rail minus high output swing
		value_t m_DAB;    //!< Differential Amp Bias - total quiescent current
	};


	} //namespace analog
} // namespace netlist

#endif /* NLD_OPAMPS_H_ */
