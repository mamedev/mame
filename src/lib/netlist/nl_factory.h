// license:BSD-3-Clause
// copyright-holders:Couriersud

///
/// \file nl_factory.h
///

#ifndef NLFACTORY_H_
#define NLFACTORY_H_

#include "nltypes.h"

#include "plib/palloc.h"
#include "plib/pmempool.h"
#include "plib/psource.h"
#include "plib/ptypes.h"

#include <tuple>
#include <utility>
#include <vector>

#define NETLIB_DEVICE_IMPL_ALIAS(p_alias, chip, p_name, p_def_param)           \
	NETLIB_DEVICE_IMPL_BASE(devices, p_alias, chip, p_name, p_def_param)

#define NETLIB_DEVICE_IMPL(chip, p_name, p_def_param)                          \
	NETLIB_DEVICE_IMPL_NS(devices, chip, p_name, p_def_param)

#define NETLIB_DEVICE_IMPL_NS(ns, chip, p_name, p_def_param)                   \
	NETLIB_DEVICE_IMPL_BASE(ns, chip, chip, p_name, p_def_param)

#define NETLIB_DEVICE_IMPL_BASE(ns, p_alias, chip, p_name, p_def_param)        \
	static factory::element_t::uptr NETLIB_NAME(p_alias##_c)()                 \
	{                                                                          \
		using devtype = factory::device_element_t<ns ::NETLIB_NAME(chip)>;     \
		factory::properties sl(p_def_param, PSOURCELOC());                     \
		return devtype::create(p_name, std::move(sl));                         \
	}                                                                          \
																			   \
	extern factory::constructor_ptr_t decl_##p_alias;                          \
	factory::constructor_ptr_t        decl_##p_alias = NETLIB_NAME(p_alias##_c);

namespace netlist::factory
{
	enum class element_type
	{
		BUILTIN,
		MACRO
	};

	struct properties
	{
		properties(const pstring &         default_parameter,
				   plib::source_location &&location)
		: m_default_parameter(default_parameter)
		, m_location(std::move(location))
		, m_type(element_type::BUILTIN)
		{
		}

		~properties() = default;
		PCOPYASSIGNMOVE(properties, default)

		pstring default_parameter() const noexcept
		{
			return m_default_parameter;
		}

		plib::source_location source() const noexcept { return m_location; }

		element_type type() const noexcept { return m_type; }

		properties &set_type(element_type t) noexcept
		{
			m_type = t;
			return *this;
		}

	private:
		pstring               m_default_parameter;
		plib::source_location m_location;
		element_type          m_type;
	};

	// -------------------------------------------------------------------------
	// net_dev class factory
	// -------------------------------------------------------------------------

	class element_t
	{
	public:
		using dev_uptr = device_arena::unique_ptr<core_device_t>;
		using uptr = host_arena::unique_ptr<element_t>;
		using pointer = element_t *;

		element_t(const pstring &name, properties &&props);
		virtual ~element_t() = default;

		PCOPYASSIGNMOVE(element_t, default)

		virtual dev_uptr
		make_device(device_arena &pool, netlist_state_t &anetlist,
					const pstring &name)
			= 0;

		pstring name() const noexcept { return m_name; }
		pstring param_desc() const noexcept
		{
			return m_properties.default_parameter();
		}
		plib::source_location source() const noexcept
		{
			return m_properties.source();
		}
		element_type type() const noexcept { return m_properties.type(); }

	private:
		pstring    m_name;       ///< device name
		properties m_properties; ///< source file and other information and
								 ///< settings
	};

	template <class C, typename... Args>
	class device_element_t : public element_t
	{
	public:
		using constructor_data_t = typename C::constructor_data_t;

		device_element_t(const pstring &name, properties &&props,
						 Args &&...args)
		: element_t(name, std::move(props))
		, m_args(std::forward<Args>(args)...)
		{
		}

		template <std::size_t... Is>
		dev_uptr make_device(device_arena &pool, netlist_state_t &anetlist,
							 const pstring &name, std::tuple<Args...> &args,
							 std::index_sequence<Is...>)
		{
			return plib::make_unique<C>(
				pool, constructor_data_t{anetlist, name},
				std::forward<Args>(std::get<Is>(args))...);
		}

		dev_uptr make_device(device_arena &pool, netlist_state_t &anetlist,
							 const pstring &name, std::tuple<Args...> &args)
		{
			return make_device(pool, anetlist, name, args,
							   std::index_sequence_for<Args...>{});
		}

		dev_uptr make_device(device_arena &pool, netlist_state_t &anetlist,
							 const pstring &name) override
		{
			return make_device(pool, anetlist, name, m_args);
		}

		static uptr
		create(const pstring &name, properties &&props, Args &&...args)
		{
			return plib::make_unique<device_element_t<C, Args...>, host_arena>(
				name, std::move(props), std::forward<Args>(args)...);
		}

	private:
		std::tuple<Args...> m_args;
	};

	class list_t : public std::vector<element_t::uptr>
	{
	public:
		explicit list_t(log_type &alog);
		~list_t() = default;

		PCOPYASSIGNMOVE(list_t, delete)

		template <class device_class, typename... Args>
		void add(const pstring &name, properties &&props, Args &&...args)
		{
			add(device_element_t<device_class, Args...>::create(
				name, std::move(props), std::forward<Args>(args)...));
		}

		void add(element_t::uptr &&factory) noexcept(false);

		element_t::pointer
		factory_by_name(const pstring &devname) noexcept(false);

		template <class C>
		bool is_class(element_t::pointer f) noexcept
		{
			return bool(plib::dynamic_downcast<device_element_t<C> *>(f));
		}

		bool exists(const pstring &name) const noexcept;

	private:
		log_type &m_log;
	};

	// -------------------------------------------------------------------------
	// factory_creator_ptr_t
	// -------------------------------------------------------------------------

	using constructor_ptr_t = element_t::uptr (*const)();

	template <typename T>
	element_t::uptr constructor_t(const pstring &name, properties &&props)
	{
		return plib::make_unique<device_element_t<T>, host_arena>(
			name, std::move(props));
	}

	// -------------------------------------------------------------------------
	// library_element_t: factory class to wrap macro based chips/elements
	// -------------------------------------------------------------------------

	class library_element_t : public element_t
	{
	public:
		library_element_t(const pstring &name, properties &&props);

		dev_uptr make_device(device_arena &pool, netlist_state_t &anetlist,
							 const pstring &name) override;
	};

} // namespace netlist::factory

namespace netlist::devices
{
	void initialize_factory(factory::list_t &factory);
} // namespace netlist::devices

#endif // NLFACTORY_H_
