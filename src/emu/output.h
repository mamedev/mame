// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Vas Crabb
/***************************************************************************

    output.h

    General purpose output routines.

***************************************************************************/
#ifndef MAME_EMU_OUTPUT_H
#define MAME_EMU_OUTPUT_H

#pragma once

#include "interface/output.h"

#include <any>
#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class output_manager
{
private:
	template <typename Input, std::make_unsigned_t<Input> DefaultMask> friend class devcb_write;

	using notifier_func = void (*)(void *param, osd::output_item const &item, s32 seconds, s64 attoseconds);

	class output_notify
	{
	public:
		output_notify(notifier_func callback, void *param)
			: m_notifier(callback)
			, m_param(param)
		{
		}

		void operator()(attotime const &when, osd::output_item const &item) const
		{ m_notifier(m_param, item, when.seconds(), when.attoseconds()); }

	private:
		notifier_func   m_notifier;       // callback to call
		void            *m_param;         // parameter to pass the callback
	};
	using notify_vector = std::vector<output_notify>;

	class item_impl
	{
	public:
		item_impl(output_manager &manager, device_t &device, std::string_view name);

		osd::output_item const &data() const { return m_data; }
		device_t &device() const { return m_device; }
		std::string_view const &name() const { return m_data.name(); }
		std::string_view const &device_tag() const { return m_data.device_tag(); }
		std::string const &qualified_name() const { return m_data.qualified_name(); }
		s32 const &get() const { return m_data.value(); }
		void set(s32 value) const { if (m_data.value() != value) { notify(value); } }
		void notify(s32 value) const;

		void add_notifier(notifier_func callback, void *param) const { m_notifylist.emplace_back(callback, param); }

	private:
		class item_data : public osd::output_item
		{
		public:
			item_data(std::string_view n, std::string_view d) : osd::output_item(std::move(n), d) { }
			void set(s32 value) { m_value = value; }
		};

		output_manager          &m_manager;     // parent output manager
		device_t                &m_device;      // associated device
		mutable item_data       m_data;         // base data
		mutable notify_vector   m_notifylist;   // list of notifier callbacks
	};

	struct item_hash
	{
		using is_transparent = void;

		std::size_t operator()(item_impl const &a) const
		{
			auto const x = std::hash<device_t *>()(&a.device());
			auto const y = std::hash<std::string_view>()(a.name());
			return x ^ (y << 1);
		}

		std::size_t operator()(std::pair<device_t &, std::string_view> const &a) const
		{
			auto const x = std::hash<device_t *>()(&a.first);
			auto const y = std::hash<std::string_view>()(a.second);
			return x ^ (y << 1);
		}
	};

	struct item_equal
	{
		using is_transparent = void;

		bool operator()(item_impl const &a, item_impl const &b) const
		{ return (&a.device() == &b.device()) && (a.name() == b.name()); }

		bool operator()(item_impl const &a, std::pair<device_t &, std::string_view> const &b) const
		{ return (&a.device() == &b.first) && (a.name() == b.second); }

		bool operator()(std::pair<device_t &, std::string_view> const &a, item_impl const &b) const
		{ return (&a.first == &b.device()) && (a.second == b.name()); }
	};

	struct qualified_name_hash : protected std::hash<std::string_view>
	{
		using is_transparent = void;

		using std::hash<std::string_view>::operator();

		std::size_t operator()(item_impl const &a) const
		{ return std::hash<std::string_view>::operator()(a.qualified_name()); }
	};

	struct qualified_name_equal
	{
		using is_transparent = void;

		bool operator()(item_impl const &a, item_impl const &b) const
		{ return a.qualified_name() == b.qualified_name(); }

		bool operator()(item_impl const &a, std::string_view b) const
		{ return a.qualified_name() == b; }

		bool operator()(std::string_view a, item_impl const &b) const
		{ return a == b.qualified_name(); }
	};

	struct unqualified_name_hash : protected std::hash<std::string_view>
	{
		using is_transparent = void;

		using std::hash<std::string_view>::operator();

		std::size_t operator()(item_impl const &a) const
		{ return std::hash<std::string_view>::operator()(a.name()); }
	};

	struct unqualified_name_equal
	{
		using is_transparent = void;

		bool operator()(item_impl const &a, item_impl const &b) const
		{ return a.name() == b.name(); }

		bool operator()(item_impl const &a, std::string_view b) const
		{ return a.name() == b; }

		bool operator()(std::string_view a, item_impl const &b) const
		{ return a == b.name(); }
	};

	using item_reference = std::reference_wrapper<item_impl const>;
	using item_set = std::unordered_set<item_impl, item_hash, item_equal>;
	using qualified_name_set = std::unordered_set<item_reference, qualified_name_hash, qualified_name_equal>;
	using unqualified_name_set = std::unordered_set<item_reference, unqualified_name_hash, unqualified_name_equal>;

	class item_creator_proxy;
	template <unsigned M, unsigned... N> struct item_proxy_array { typedef typename item_proxy_array<N...>::type type[M]; };
	template <unsigned N> struct item_proxy_array<N> { typedef item_creator_proxy type[N]; };
	template <unsigned... N> using item_proxy_array_t = typename item_proxy_array<N...>::type;

	item_impl const *find_item(device_t &device, std::string_view name);
	item_impl const &find_or_create_item(device_t &device, std::string_view name);

	// event handlers
	void presave() ATTR_COLD;
	void postload() ATTR_COLD;

	static bool validate_name(device_t &device, std::string_view name);

	// internal state
	running_machine             &m_machine;
	item_set                    m_itemtable;
	qualified_name_set          m_qualified;
	unqualified_name_set        m_unqualified;
	notify_vector               m_global_notifylist;
	std::vector<item_reference> m_save_order;
	std::unique_ptr<s32 []>     m_save_data;

public:
	template <typename X, unsigned... N> class output_finder;
	template <typename X> class output_finder<X>;
	class output_proxy;

	// construction/destruction
	output_manager(running_machine &machine);

	// register for save states
	void register_save() ATTR_COLD;

	// getters
	running_machine &machine() const { return m_machine; }

	// set a notifier on a particular output
	void add_notifier(device_t &device, std::string_view name, notifier_func callback, void *param);

	// add a notifier globally
	void add_global_notifier(notifier_func callback, void *param);

	// immediately call a notifier for all outputs
	template <typename T> void notify_all(T &&notifier) const
	{
		for (auto const &item : m_itemtable)
			notifier(item.data());
	}
};

template <unsigned... N> using output_finder = output_manager::output_finder<void, N...>;
using output_proxy = output_manager::output_proxy;

#endif // MAME_EMU_OUTPUT_H
