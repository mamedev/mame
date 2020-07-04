// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * nld_74125.cpp
 *
 */

#include "nld_74125.h"
#include "nl_base.h"

#include <type_traits>

namespace netlist
{
	template <typename T>
	struct uptr : public device_arena::unique_ptr<T>
	{
		uptr() = default;

		using base_type = device_arena::unique_ptr<T>;

		template<typename O, typename... Args>
		uptr(O &owner, const pstring &name, Args&&... args)
		: device_arena::unique_ptr<T>(owner.template make_pool_object<T>(owner, name, std::forward<Args>(args)...))
		{ }

		constexpr auto operator ()() noexcept -> decltype((*device_arena::unique_ptr<T>::get())()) { return (*this->get())(); }
		constexpr auto operator ()() const noexcept -> const decltype((*device_arena::unique_ptr<T>::get())()) { return (*this->get())(); }
	};

	namespace devices
	{

	template <typename D>
	NETLIB_OBJECT(74125_base)
	{
		NETLIB_CONSTRUCTOR(74125_base)
		, m_TE(*this, "FORCE_TRISTATE_LOGIC", 0)
		, m_A(*this, "A", NETLIB_DELEGATE(A))
		, m_G(*this, pstring(D::invert_g::value ? "G" : "GQ"), NETLIB_DELEGATE(G))
		, m_Y(*this, "Y", m_TE())
		//, m_Y(*this, "Y")
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
			m_Y.set_tristate(m_G() ^ D::invert_g::value,
				D::ts_off_on::value(), D::ts_on_off::value());
			m_Y.push(m_A(), m_A() ? D::sig_off_on::value() : D::sig_on_off::value());
		}

		NETLIB_HANDLERI(A)
		{
			m_Y.push(m_A(), m_A() ? D::sig_off_on::value() : D::sig_on_off::value());
		}

		NETLIB_HANDLERI(G)
		{
			m_Y.set_tristate(m_G() ^ D::invert_g::value,
				D::ts_off_on::value(), D::ts_on_off::value());
		}

		param_logic_t      m_TE;
		logic_input_t      m_A;
		uptr<logic_input_t>      m_G;
		tristate_output_t  m_Y;
		nld_power_pins     m_power_pins;
	};

	struct desc_74125 : public desc_base
	{
		using invert_g = desc_const<0>;
		using ts_off_on = time_ns<11>;
		using ts_on_off = time_ns<13>;
		using sig_off_on = time_ns<8>;
		using sig_on_off = time_ns<12>;
	};

	struct desc_74126 : public desc_74125
	{
		using invert_g = desc_const<1>;
	};

	using NETLIB_NAME(74125) = NETLIB_NAME(74125_base)<desc_74125>;
	using NETLIB_NAME(74126) = NETLIB_NAME(74125_base)<desc_74126>;

	NETLIB_DEVICE_IMPL(74125,     "TTL_74125_GATE",     "")
	NETLIB_DEVICE_IMPL(74126,     "TTL_74126_GATE",     "")

	} //namespace devices
} // namespace netlist
