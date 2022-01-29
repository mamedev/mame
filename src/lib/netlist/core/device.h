// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file device.h
///

#ifndef NL_DEVICE_H_
#define NL_DEVICE_H_

#include "core_device.h"
#include "param.h"

namespace netlist
{
	// -----------------------------------------------------------------------------
	// device_t
	// -----------------------------------------------------------------------------

	class device_t : public base_device_t,
					 public logic_family_t
	{
	public:
		device_t(netlist_state_t &owner, const pstring &name);
		device_t(netlist_state_t &owner, const pstring &name,
			const pstring &model);
		// only needed by proxies
		device_t(netlist_state_t &owner, const pstring &name,
			const logic_family_desc_t *desc);

		device_t(device_t &owner, const pstring &name);
		// pass in a default model - this may be overwritten by PARAM(DEVICE.MODEL, "XYZ(...)")
		device_t(device_t &owner, const pstring &name,
			const pstring &model);

		PCOPYASSIGNMOVE(device_t, delete)

		~device_t() noexcept override = default;

	protected:

		//NETLIB_UPDATE_TERMINALSI() { }
	private:
		param_model_t m_model;
	};

} // namespace netlist


#endif // NL_DEVICE_H_
