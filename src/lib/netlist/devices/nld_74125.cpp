// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74125.c
 *
 */

#include "nld_74125.h"
#include "nl_base.h"

namespace netlist
{
	template <typename T>
	struct uptr : public unique_pool_ptr<T>
	{
		uptr() = default;

		using base_type = unique_pool_ptr<T>;

		template<typename O, typename... Args, class = typename std::enable_if<
		    std::is_base_of<core_device_t, O>::value>::type>
		uptr(O &owner, const pstring &name, Args&&... args)
		: unique_pool_ptr<T>(static_cast<core_device_t &>(owner).state().make_object<T>(owner, name, std::forward<Args>(args)...))
		{ }

		template<typename O, typename... Args>
		uptr(typename std::enable_if<
		    std::is_base_of<netlist_state_t, O>::value, O>::type &owner, const pstring &name, Args&&... args)
		: unique_pool_ptr<T>(static_cast<netlist_state_t>(owner).make_object<T>(owner, name, std::forward<Args>(args)...))
		{ }

		C14CONSTEXPR auto operator ()() noexcept -> decltype((*unique_pool_ptr<T>::get())()) { return (*this->get())(); }
		constexpr auto operator ()() const noexcept -> const decltype((*unique_pool_ptr<T>::get())()) { return (*this->get())(); }
	};

	namespace devices
	{

	template <netlist_sig_t INVERT_G>
	NETLIB_OBJECT(74125_base)
	{
		NETLIB_CONSTRUCTOR(74125_base)
		, m_A(*this, "A", NETLIB_DELEGATE(74125_base, A))
		, m_G(*this, "G", NETLIB_DELEGATE(74125_base, G))
		, m_Y(*this, "Y", NLTIME_FROM_NS(11), NLTIME_FROM_NS(13))
		, m_power_pins(*this)
		{
		}

	private:
		NETLIB_RESETI()
		{
		}

		NETLIB_UPDATEI()
		{
			// this one is only called during startup. Ensure all outputs
			// are in a consistent state.
			m_Y.set_tristate(m_G() ^ INVERT_G);
			m_Y.push(m_A(), m_A() ? NLTIME_FROM_NS(8) : NLTIME_FROM_NS(12));
		}

		NETLIB_HANDLERI(A)
		{
			m_Y.push(m_A(), m_A() ? NLTIME_FROM_NS(8) : NLTIME_FROM_NS(12));
		}

		NETLIB_HANDLERI(G)
		{
			m_Y.set_tristate(m_G() ^ INVERT_G);
		}

		logic_input_t      m_A;
		uptr<logic_input_t>      m_G;
		tristate_output_t  m_Y;
		nld_power_pins     m_power_pins;
	};

#if 0
	template <bool ASYNC>
	NETLIB_OBJECT(74125_dip_base)
	{
		NETLIB_CONSTRUCTOR(74125_dip_base)
		, A(*this, "A")
		{
			this->register_subalias("1", "A.CLRQ");
			this->register_subalias("2", "A.CLK");
			this->register_subalias("3", "A.A");
			this->register_subalias("4", "A.B");
			this->register_subalias("5", "A.C");
			this->register_subalias("6", "A.D");
			this->register_subalias("7", "A.ENP");
			this->register_subalias("8", "A.GND");

			this->register_subalias("9", "A.LOADQ");
			this->register_subalias("10", "A.ENT");
			this->register_subalias("11", "A.QD");
			this->register_subalias("12", "A.QC");
			this->register_subalias("13", "A.QB");
			this->register_subalias("14", "A.QA");
			this->register_subalias("15", "A.RC");
			this->register_subalias("16", "A.VCC");
		}
	private:
		NETLIB_SUB(74125_base)<ASYNC> A;
	};
	using NETLIB_NAME(74163) = NETLIB_NAME(74125_base)<false>;
	using NETLIB_NAME(74125_dip) = NETLIB_NAME(74125_dip_base)<true>;
	using NETLIB_NAME(74163_dip) = NETLIB_NAME(74125_dip_base)<false>;
	NETLIB_DEVICE_IMPL(74125_dip, "TTL_74125_DIP", "")

	NETLIB_DEVICE_IMPL(74163,     "TTL_74163",     "+CLK,+ENP,+ENT,+CLRQ,+LOADQ,+A,+B,+C,+D,@VCC,@GND")
	NETLIB_DEVICE_IMPL(74163_dip, "TTL_74163_DIP", "")
#endif
	using NETLIB_NAME(74125) = NETLIB_NAME(74125_base)<1>;
	using NETLIB_NAME(74126) = NETLIB_NAME(74125_base)<0>;

	NETLIB_DEVICE_IMPL(74125,     "TTL_74125",     "")
	NETLIB_DEVICE_IMPL(74126,     "TTL_74126",     "")

	} //namespace devices
} // namespace netlist
