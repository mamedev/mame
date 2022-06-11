// license:BSD-3-Clause
// copyright-holders:Couriersud

#ifndef NL_CORE_DEVICES_H_
#define NL_CORE_DEVICES_H_

///
/// \file devices.h
///
/// The core is accessing members or type definitions of devices defined
/// here directly (e.g. nld_nc_pin).
///

#include "analog.h"
#include "device.h"
#include "device_macros.h"
#include "logic.h"
#include "param.h"

//============================================================
// Namespace starts
//============================================================

namespace netlist::devices
{
	// -----------------------------------------------------------------------------
	// main clock
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(mainclock)
	{
		NETLIB_CONSTRUCTOR(mainclock)
		, m_Q(*this, "Q")
		, m_freq(*this, "FREQ", nlconst::magic(7159000.0 * 5))
		{
			m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
		}

		NETLIB_RESETI()
		{
			m_Q.net().set_next_scheduled_time(exec().time());
		}

		NETLIB_UPDATE_PARAMI()
		{
			m_inc = netlist_time::from_fp(plib::reciprocal(m_freq()*nlconst::two()));
		}

	public:
		logic_output_t m_Q; // NOLINT: needed in core
		netlist_time m_inc; // NOLINT: needed in core
	private:
		param_fp_t m_freq;
	};

	// -----------------------------------------------------------------------------
	// power pins - not a device, but a helper
	// -----------------------------------------------------------------------------

	/// \brief Power pins class.
	///
	/// Power Pins are passive inputs. Delegate noop will silently ignore any
	/// updates.

	class nld_power_pins
	{
	public:
		using this_type = nld_power_pins;

		explicit nld_power_pins(device_t &owner)
		: m_VCC(owner, owner.logic_family()->vcc_pin(), NETLIB_DELEGATE(noop))
		, m_GND(owner, owner.logic_family()->gnd_pin(), NETLIB_DELEGATE(noop))
		{
		}

		explicit nld_power_pins(device_t &owner, nl_delegate delegate)
		: m_VCC(owner, owner.logic_family()->vcc_pin(), delegate)
		, m_GND(owner, owner.logic_family()->gnd_pin(), delegate)
		{
		}

		// Some devices like the 74LS629 have two pairs of supply pins.
		explicit nld_power_pins(device_t &owner,
			const pstring &vcc, const pstring &gnd)
		: m_VCC(owner, vcc, NETLIB_DELEGATE(noop))
		, m_GND(owner, gnd, NETLIB_DELEGATE(noop))
		{
		}

		// Some devices like the 74LS629 have two pairs of supply pins.
		explicit nld_power_pins(device_t &owner,
			const pstring &vcc, const pstring &gnd,
			nl_delegate delegate)
		: m_VCC(owner, vcc, delegate)
		, m_GND(owner, gnd, delegate)
		{
		}

		const analog_input_t &VCC() const noexcept
		{
			return m_VCC;
		}
		const analog_input_t &GND() const noexcept
		{
			return m_GND;
		}

	private:
		void noop() { }
		analog_input_t m_VCC;
		analog_input_t m_GND;
	};

	// -----------------------------------------------------------------------------
	// netlist parameters
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(netlistparams)
	{
		NETLIB_CONSTRUCTOR(netlistparams)
		, m_use_deactivate(*this, "USE_DEACTIVATE", false)
		, m_startup_strategy(*this, "STARTUP_STRATEGY", 0)
		, m_mos_cap_model(*this, "DEFAULT_MOS_CAPMODEL", 2)
		, m_max_link_loops(*this, "MAX_LINK_RESOLVE_LOOPS", 100)
		{
		}
		//NETLIB_RESETI() {}
		//NETLIB_UPDATE_PARAMI() { }
	public:
		param_logic_t m_use_deactivate;
		param_num_t<unsigned>   m_startup_strategy;
		param_num_t<unsigned>   m_mos_cap_model;
		//! How many times do we try to resolve links (connections)
		param_num_t<unsigned>   m_max_link_loops;
	};

	// -----------------------------------------------------------------------------
	// nld_nc_pin
	//
	// FIXME: This needs to optimized
	//        The input can be in de-activated state.
	// -----------------------------------------------------------------------------

	NETLIB_OBJECT(nc_pin)
	{
	public:
		NETLIB_CONSTRUCTOR(nc_pin)
		, m_I(*this, "I", NETLIB_DELEGATE_NOOP())
		{
		}

	protected:
		//NETLIB_RESETI() {}

	private:
		analog_input_t m_I;

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

		NETLIB_UPDATE_PARAMI()
		{
			m_Q.push(nlconst::zero());
		}

		//NETLIB_RESETI() {}
	protected:
		analog_output_t m_Q;
	};

} // namespace netlist::devices

#endif // NL_CORE_DEVICES_H_
