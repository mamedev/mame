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

		device_t(const device_t &) = delete;
		device_t &operator=(const device_t &) = delete;
		device_t(device_t &&) noexcept = delete;
		device_t &operator=(device_t &&) noexcept = delete;

		~device_t() noexcept override = default;

	protected:
		template <typename T1, typename T2>
		void push_two(T1 &term1, netlist_sig_t newQ1, const netlist_time &delay1,
			T2 &term2, netlist_sig_t newQ2, const netlist_time &delay2) noexcept
		{
			if (delay2 < delay1)
			{
				term1.push(newQ1, delay1);
				term2.push(newQ2, delay2);
			}
			else
			{
				term2.push(newQ2, delay2);
				term1.push(newQ1, delay1);
			}
		}


		//NETLIB_UPDATE_TERMINALSI() { }
	private:
		param_model_t m_model;
	};

} // namespace netlist


#endif // NL_DEVICE_H_
