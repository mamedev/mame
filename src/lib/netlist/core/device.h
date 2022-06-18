// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file device.h
///

#ifndef NL_DEVICE_H_
#define NL_DEVICE_H_

#include "core_device.h"
#include "logic_family.h"
#include "param.h"

namespace netlist
{

	// -------------------------------------------------------------------------
	// device_t construction parameters
	// -------------------------------------------------------------------------

	using device_data_t = base_device_data_t;
	// The type use to pass data on
	using device_param_t = const device_data_t &;

	// -------------------------------------------------------------------------
	// device_t
	// -------------------------------------------------------------------------

	class device_t : public base_device_t, public logic_family_t
	{
	public:
		using constructor_data_t = device_data_t;
		using constructor_param_t = device_param_t;

		device_t(device_param_t data);

		device_t(device_param_t data, const pstring &model);
		// only needed by proxies
		device_t(device_param_t data, const logic_family_desc_t *desc);

		device_t(const device_t &) = delete;
		device_t &operator=(const device_t &) = delete;
		device_t(device_t &&) noexcept = delete;
		device_t &operator=(device_t &&) noexcept = delete;

		~device_t() noexcept override = default;

	protected:
		template <typename T1, typename T2>
		void push_two(T1 &term1, netlist_sig_t newQ1,
			const netlist_time &delay1, T2 &term2, netlist_sig_t newQ2,
			const netlist_time &delay2) noexcept
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

		// NETLIB_UPDATE_TERMINALSI() { }
	private:
		param_model_t m_model;
	};

	// -------------------------------------------------------------------------
	// FIXME: Rename
	// -------------------------------------------------------------------------

	template <typename CX>
	struct sub_device_wrapper
	{
		using constructor_data_t = typename CX::constructor_data_t;
		using constructor_param_t = typename CX::constructor_param_t;

		template <typename... Args>
		sub_device_wrapper(base_device_t &owner, const pstring &name,
			Args &&...args)
		{
			// m_dev = owner.state().make_pool_object<CX>(owner, name,
			// std::forward<Args>(args)...);
			m_dev = owner.state().make_pool_object<CX>(
				constructor_data_t{owner.state(), owner.name() + "." + name},
				std::forward<Args>(args)...);
			owner.state().register_device(m_dev->name(),
				device_arena::owned_ptr<core_device_t>(m_dev.get(), false));
		}
		template <typename... Args>
		sub_device_wrapper(device_t &owner, const pstring &name, Args &&...args)
		{
			// m_dev = owner.state().make_pool_object<CX>(owner, name,
			// std::forward<Args>(args)...);
			m_dev = owner.state().make_pool_object<CX>(
				constructor_data_t{owner.state(), owner.name() + "." + name},
				std::forward<Args>(args)...);
			owner.state().register_device(m_dev->name(),
				device_arena::owned_ptr<core_device_t>(m_dev.get(), false));
		}
		CX &      operator()() { return *m_dev; }
		const CX &operator()() const { return *m_dev; }

	private:
		device_arena::unique_ptr<CX> m_dev;
	};

} // namespace netlist

#endif // NL_DEVICE_H_
