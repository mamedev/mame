// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file state_var.h
///

#ifndef NL_CORE_STATE_VAR_H_
#define NL_CORE_STATE_VAR_H_

#include "../nltypes.h"
#include "../plib/pstring.h"

namespace netlist
{
	/// \brief A persistent variable template.
	///  Use the state_var template to define a variable whose value is saved.
	///  Within a device definition use
	///
	/// ```
	///      NETLIB_OBJECT(abc)
	///      {
	///          NETLIB_CONSTRUCTOR(abc)
	///          , m_var(*this, "myvar", 0)
	///          ...
	///          state_var<unsigned> m_var;
	///      }
	/// ```
	///
	template <typename T>
	struct state_var
	{
	public:

		using value_type = T;

		template <typename O>
		//! Constructor.
		state_var(O &owner,             //!< owner must have a netlist() method.
				const pstring &name,    //!< identifier/name for this state variable
				const T &value          //!< Initial value after construction
				);

		template <typename O>
		//! Constructor.
		state_var(O &owner,             //!< owner must have a netlist() method.
				const pstring &name     //!< identifier/name for this state variable
		);

		PMOVEASSIGN(state_var, delete)

		//! Destructor.
		~state_var() noexcept = default;

		//! Copy Constructor removed.
		constexpr state_var(const state_var &rhs) = delete;
		//! Assignment operator to assign value of a state var.
		constexpr state_var &operator=(const state_var &rhs) noexcept
		{
			if (this != &rhs)
				m_value = rhs.m_value;
			return *this;
		} // OSX doesn't like noexcept
		//! Assignment operator to assign value of type T.
		constexpr state_var &operator=(const T &rhs) noexcept { m_value = rhs; return *this; }
		//! Assignment move operator to assign value of type T.
		//constexpr state_var &operator=(T &&rhs) noexcept { std::swap(m_value, rhs); return *this; }
		constexpr state_var &operator=(T &&rhs) noexcept { m_value = std::move(rhs); return *this; }
		//! Return non-const value of state variable.
		constexpr operator T & () noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr operator const T & () const noexcept { return m_value; }
		//! Return non-const value of state variable.
		constexpr T & var() noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr const T & var() const noexcept { return m_value; }
		//! Return non-const value of state variable.
		constexpr T & operator ()() noexcept { return m_value; }
		//! Return const value of state variable.
		constexpr const T & operator ()() const noexcept { return m_value; }
		//! Access state variable by ->.
		constexpr T * operator->() noexcept { return &m_value; }
		//! Access state variable by const ->.
		constexpr const T * operator->() const noexcept{ return &m_value; }
		//! Access state variable by *.
		constexpr T * operator *() noexcept { return &m_value; }
		//! Access state variable by const *.
		constexpr const T * operator *() const noexcept{ return &m_value; }

	private:
		T m_value;
	};

	/// \brief A persistent array template.
	///  Use this state_var template to define an array whose contents are saved.
	///  Please refer to \ref state_var.
	///
	///  \tparam C container class to use.

	template <typename C>
	struct state_container : public C
	{
	public:
		using value_type = typename C::value_type;
		//! Constructor.
		template <typename O>
		state_container(O &owner,           //!< owner must have a netlist() method.
				const pstring &name,        //!< identifier/name for this state variable
				const value_type &value     //!< Initial value after construction
				);
		//! Constructor.
		template <typename O>
		state_container(O &owner,           //!< owner must have a netlist() method.
				const pstring &name,        //!< identifier/name for this state variable
				std::size_t n,              //!< number of elements to allocate
				const value_type &value     //!< Initial value after construction
				);
		//! Copy Constructor.
		state_container(const state_container &rhs) noexcept = default;
		//! Destructor.
		~state_container() noexcept = default;
		//! Move Constructor.
		state_container(state_container &&rhs) noexcept = default;
		state_container &operator=(const state_container &rhs) noexcept = default;
		state_container &operator=(state_container &&rhs) noexcept = default;
	};

	// -----------------------------------------------------------------------------
	// State variables - predefined and c++11 non-optional
	// -----------------------------------------------------------------------------

	/// \brief predefined state variable type for uint8_t
	using state_var_u8 = state_var<std::uint8_t>;
	/// \brief predefined state variable type for int8_t
	using state_var_s8 = state_var<std::int8_t>;

	/// \brief predefined state variable type for uint32_t
	using state_var_u32 = state_var<std::uint32_t>;
	/// \brief predefined state variable type for int32_t
	using state_var_s32 = state_var<std::int32_t>;
	/// \brief predefined state variable type for sig_t
	using state_var_sig = state_var<netlist_sig_t>;

	template <typename T>
	template <typename O>
	state_var<T>::state_var(O &owner, const pstring &name, const T &value)
	: m_value(value)
	{
		owner.state().save(owner, m_value, owner.name(), name);
	}

	template <typename T>
	template <typename O>
	state_var<T>::state_var(O &owner, const pstring &name)
	{
		owner.state().save(owner, m_value, owner.name(), name);
	}

	template <typename C>
	template <typename O>
	state_container<C>::state_container(O &owner, const pstring &name,
		const state_container<C>::value_type & value)
	{
		owner.state().save(owner, static_cast<C &>(*this), owner.name(), name);
		for (std::size_t i=0; i < this->size(); i++)
			(*this)[i] = value;
	}

	template <typename C>
	template <typename O>
	state_container<C>::state_container(O &owner, const pstring &name,
		std::size_t n, const state_container<C>::value_type & value)
	: C(n, value)
	{
		owner.state().save(owner, static_cast<C &>(*this), owner.name(), name);
	}

} // namespace netlist

namespace plib
{
	template <typename X>
	struct ptype_traits<netlist::state_var<X>> : ptype_traits<X>
	{
	};
} // namespace plib

#endif // NL_CORE_STATE_VAR_H_
