// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file logic_family.h
///

#ifndef NL_CORE_LOGIC_FAMILY_H_
#define NL_CORE_LOGIC_FAMILY_H_

#include "../nltypes.h"
#include "../plib/pstring.h"

namespace netlist
{
	/// \brief Logic families descriptors are used to create proxy devices.
	///  The logic family describes the analog capabilities of logic devices,
	///  inputs and outputs.

	class logic_family_desc_t
	{
	public:
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, modernize-use-equals-default)
		logic_family_desc_t()
		{
		}

		logic_family_desc_t(const logic_family_desc_t &) = delete;
		logic_family_desc_t &operator=(const logic_family_desc_t &) = delete;

		// FOXME: Should be move constructible
		logic_family_desc_t(logic_family_desc_t &&) noexcept = delete;
		logic_family_desc_t &operator=(logic_family_desc_t &&) noexcept = delete;

		virtual ~logic_family_desc_t() noexcept = default;

		virtual device_arena::unique_ptr<devices::nld_base_d_to_a_proxy> create_d_a_proxy(netlist_state_t &anetlist, const pstring &name,
				const logic_output_t *proxied) const = 0;
		virtual device_arena::unique_ptr<devices::nld_base_a_to_d_proxy> create_a_d_proxy(netlist_state_t &anetlist, const pstring &name,
				const logic_input_t *proxied) const = 0;

		nl_fptype low_threshold_V(nl_fptype VN, nl_fptype VP) const noexcept{ return VN + (VP - VN) * m_low_threshold_PCNT; }
		nl_fptype high_threshold_V(nl_fptype VN, nl_fptype VP) const noexcept{ return VN + (VP - VN) * m_high_threshold_PCNT; }
		nl_fptype low_offset_V() const noexcept{ return m_low_VO; }
		nl_fptype high_offset_V() const noexcept{ return m_high_VO; }
		nl_fptype R_low() const noexcept{ return m_R_low; }
		nl_fptype R_high() const noexcept{ return m_R_high; }

		bool is_above_high_threshold_V(nl_fptype V, nl_fptype VN, nl_fptype VP) const noexcept
		{ return V > high_threshold_V(VN, VP); }

		bool is_below_low_threshold_V(nl_fptype V, nl_fptype VN, nl_fptype VP) const noexcept
		{ return V < low_threshold_V(VN, VP); }

		pstring vcc_pin() const { return pstring(m_vcc); }
		pstring gnd_pin() const { return pstring(m_gnd); }

		nl_fptype m_low_threshold_PCNT;   //!< low input threshold offset. If the input voltage is below this value times supply voltage, a "0" input is signalled
		nl_fptype m_high_threshold_PCNT;  //!< high input threshold offset. If the input voltage is above the value times supply voltage, a "0" input is signalled
		nl_fptype m_low_VO;               //!< low output voltage offset. This voltage is output if the ouput is "0"
		nl_fptype m_high_VO;              //!< high output voltage offset. The supply voltage minus this offset is output if the ouput is "1"
		nl_fptype m_R_low;                //!< low output resistance. Value of series resistor used for low output
		nl_fptype m_R_high;               //!< high output resistance. Value of series resistor used for high output
		const char *m_vcc;                //!< default power pin name for positive supply
		const char *m_gnd;                //!< default power pin name for negative supply
	};

	/// \brief Base class for devices, terminals, outputs and inputs which support
	///  logic families.
	///  This class is a storage container to store the logic family for a
	///  netlist object. You will not directly use it. Please refer to
	///  \ref NETLIB_FAMILY to learn how to define a logic family for a device.
	///
	/// All terminals inherit the family description from the device
	/// The default is the ttl family, but any device can override the family.
	/// For individual terminals, the family can be overwritten as well.
	///

	class logic_family_t
	{
	public:
		logic_family_t() : m_logic_family(nullptr) {}
		logic_family_t(const logic_family_desc_t *d) : m_logic_family(d) {}

		logic_family_t(const logic_family_t &) = delete;
		logic_family_t &operator=(const logic_family_t &) = delete;

		// FIXME: logic family can be move constructible.
		logic_family_t(logic_family_t &&) noexcept = delete;
		logic_family_t &operator=(logic_family_t &&) noexcept = delete;

		const logic_family_desc_t *logic_family() const noexcept { return m_logic_family; }
		void set_logic_family(const logic_family_desc_t *fam) noexcept { m_logic_family = fam; }

	protected:
		~logic_family_t() noexcept = default; // prohibit polymorphic destruction
	private:
		const logic_family_desc_t *m_logic_family;
	};

} // namespace netlist


#endif // NL_CORE_LOGIC_FAMILY_H_
