// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Vas Crabb
/***************************************************************************

    output.h

    General purpose output routines.
***************************************************************************/
#ifndef MAME_EMU_OUTPUT_H
#define MAME_EMU_OUTPUT_H

#pragma once

#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> output_manager

class output_manager
{
private:
	template <typename Input, std::make_unsigned_t<Input> DefaultMask> friend class devcb_write;

	typedef void (*notifier_func)(const char *outname, s32 value, void *param);

	class output_notify
	{
	public:
		output_notify(notifier_func callback, void *param)
			: m_notifier(callback)
			, m_param(param)
		{
		}

		void operator()(char const *outname, s32 value) const { m_notifier(outname, value, m_param); }

	private:
		notifier_func   m_notifier;       // callback to call
		void *          m_param;          // parameter to pass the callback
	};
	using notify_vector = std::vector<output_notify>;

	class output_item
	{
	public:
		output_item(output_item &&) = delete;
		output_item(output_item const &) = delete;
		output_item &operator=(output_item &&) = delete;
		output_item &operator=(output_item const &) = delete;

		output_item(
				output_manager &manager,
				std::string &&name,
				u32 id,
				s32 value);

		std::string const &name() const { return m_name; }
		u32 id() const { return m_id; }
		s32 get() const { return m_value; }
		void set(s32 value) { if (m_value != value) { notify(value); } }
		void notify(s32 value);

		void set_notifier(notifier_func callback, void *param) { m_notifylist.emplace_back(callback, param); }

	private:
		output_manager      &m_manager;     // parent output manager
		std::string const   m_name;         // string name of the item
		u32 const           m_id;           // unique ID for this item
		s32                 m_value;        // current value
		notify_vector       m_notifylist;   // list of notifier callbacks
	};

	class item_proxy
	{
	public:
		item_proxy() = default;
		void resolve(device_t &device, std::string_view name);
		operator s32() const { return m_item->get(); }
		s32 operator=(s32 value) { m_item->set(value); return m_item->get(); }
	private:
		output_item *m_item = nullptr;
	};
	template <unsigned M, unsigned... N> struct item_proxy_array { typedef typename item_proxy_array<N...>::type type[M]; };
	template <unsigned N> struct item_proxy_array<N> { typedef item_proxy type[N]; };
	template <unsigned... N> using item_proxy_array_t = typename item_proxy_array<N...>::type;

public:
	template <typename X, unsigned... N> class output_finder
	{
	public:
		template <typename... T> output_finder(device_t &device, std::string &&format, T &&... start_args)
			: m_device(device)
			, m_format(std::move(format))
			, m_start_args{ std::forward<T>(start_args)... }
		{
		}

		auto &operator[](unsigned n) { return m_proxies[n]; }
		auto &operator[](unsigned n) const { return m_proxies[n]; }

		auto size() const { return std::size(m_proxies); }
		auto begin() { return std::begin(m_proxies); }
		auto end() { return std::end(m_proxies); }
		auto begin() const { return std::begin(m_proxies); }
		auto end() const { return std::end(m_proxies); }
		auto cbegin() const { return std::begin(m_proxies); }
		auto cend() const { return std::end(m_proxies); }

		void resolve() { resolve<0U>(m_proxies); }

	private:
		template <unsigned A, unsigned C, typename... T>
		void resolve(item_proxy (&proxies)[C], T &&... i)
		{
			for (unsigned j = 0U; C > j; ++j)
				proxies[j].resolve(m_device, util::string_format(m_format, std::forward<T>(i)..., j + m_start_args[A]));
		}

		template <unsigned A, unsigned C, unsigned D, typename T, typename... U>
		void resolve(T (&proxies)[C][D], U &&... i)
		{
			for (unsigned j = 0U; C > j; ++j)
				resolve<A + 1>(proxies[j], std::forward<U>(i)..., j + m_start_args[A]);
		}

		device_t                    &m_device;
		std::string const           m_format;
		unsigned const              m_start_args[sizeof...(N)];
		item_proxy_array_t<N...>    m_proxies;
	};

	template <typename X> class output_finder<X>
	{
	public:
		output_finder(device_t &device, std::string &&format)
			: m_device(device)
			, m_format(std::move(format))
		{
		}

		operator s32() const { return m_proxy; }
		s32 operator=(s32 value) { return m_proxy = value; }

		void resolve() { m_proxy.resolve(m_device, m_format); }

	private:
		device_t            &m_device;
		std::string const   m_format;
		item_proxy          m_proxy;
	};

	// construction/destruction
	output_manager(running_machine &machine);

	// register for save states
	void register_save() ATTR_COLD;

	// getters
	running_machine &machine() const { return m_machine; }

	// set the value for a given output
	void set_value(std::string_view outname, s32 value);

	// return the current value for a given output
	s32 get_value(std::string_view outname);

	// set a notifier on a particular output
	void set_notifier(std::string_view outname, notifier_func callback, void *param);

	// set a notifier globally
	void set_global_notifier(notifier_func callback, void *param);

	// immdediately call a notifier for all outputs
	template <typename T> void notify_all(T &&notifier) const
	{
		for (auto const &item : m_itemtable)
			notifier(item.second.name().c_str(), item.second.get());
	}

	// map a name to a unique ID
	u32 name_to_id(std::string_view outname);

	// map a unique ID back to a name
	const char *id_to_name(u32 id);

private:
	output_item *find_item(std::string_view string);
	output_item &create_new_item(std::string_view outname, s32 value);
	output_item &find_or_create_item(std::string_view outname, s32 value);

	// event handlers
	void pause();
	void resume();
	void presave() ATTR_COLD;
	void postload() ATTR_COLD;

	// internal state
	running_machine &m_machine;                  // reference to our machine
	std::unordered_map<std::string, output_item> m_itemtable;
	notify_vector m_global_notifylist;
	std::vector<std::reference_wrapper<output_item> > m_save_order;
	std::unique_ptr<s32 []> m_save_data;
	u32 m_uniqueid;
};

template <unsigned... N> using output_finder = output_manager::output_finder<void, N...>;

#endif // MAME_EMU_OUTPUT_H
