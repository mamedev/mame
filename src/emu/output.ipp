// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Vas Crabb
/***************************************************************************

    output.ipp

    General purpose output routines.

***************************************************************************/
#ifndef MAME_EMU_OUTPUT_IPP
#define MAME_EMU_OUTPUT_IPP

#include "devfind.h"
#include "mconfig.h"
#include "output.h"


class output_manager::item_creator_proxy
{
public:
	item_creator_proxy() = default;
	item_creator_proxy(item_creator_proxy const &) = delete;
	item_creator_proxy &operator=(item_creator_proxy const &) = delete;

	bool resolve(device_t &device, std::string_view name); // TODO: avoid keeping this visible to end users

	operator s32() const { return m_item->get(); }
	s32 operator=(s32 value) { m_item->set(value); return m_item->get(); }

private:
	item_impl const *m_item = nullptr;
};


template <typename X, unsigned... N>
class output_manager::output_finder : protected device_resolver_base
{
private:
	struct format_spec
	{
		char const *format;
		unsigned start_args[sizeof...(N)];
	};

	using resolver_func = bool (*)(output_finder &);
	using validator_func = bool (*)(output_finder &, validity_checker &);

	virtual bool findit(validity_checker *valid) override ATTR_COLD;
	virtual void end_configuration() override ATTR_COLD;

	template <unsigned A, unsigned C, typename T, typename... U>
	bool resolve_format(format_spec const &spec, T (&proxies)[C], U &&... i) ATTR_COLD;

	template <unsigned A, unsigned C, typename T, typename... U>
	bool validate_format(validity_checker &valid, format_spec const &spec, T (&proxies)[C], U &&... i) ATTR_COLD;

	template <typename T, typename U, unsigned C>
	bool resolve_array(T const &spec, U (&proxies)[C]) ATTR_COLD;

	template <typename T, typename U, unsigned C>
	bool validate_array(validity_checker &valid, T const &spec, U (&proxies)[C]) ATTR_COLD;

	static bool resolve_format(output_finder &obj) ATTR_COLD;

	static bool validate_format(output_finder &obj, validity_checker &valid) ATTR_COLD;

	template <typename T>
	static bool resolve_array(output_finder &obj) ATTR_COLD;

	template <typename T>
	static bool validate_array(output_finder &obj, validity_checker &valid) ATTR_COLD;

	std::reference_wrapper<device_t>    m_device;
	item_proxy_array_t<N...>            m_proxies;
	std::any                            m_spec;
	resolver_func                       m_resolver;
	validator_func                      m_validator;
	bool                                m_resolved;

public:
	template <typename... T>
	output_finder(device_t &device, char const *format, T &&... start_args)
		: device_resolver_base(device)
		, m_device(device)
		, m_spec(format_spec{ format, { std::forward<T>(start_args)... } })
		, m_resolver(&output_finder::resolve_format)
		, m_validator(&output_finder::validate_format)
		, m_resolved(false)
	{
		static_assert(sizeof...(N) == sizeof...(T));
	}

	template <typename T>
	output_finder(device_t &device, T const &names)
		: device_resolver_base(device)
		, m_device(device)
		, m_spec(std::cref(names))
		, m_resolver(&output_finder::resolve_array<T>)
		, m_validator(&output_finder::validate_array<T>)
		, m_resolved(false)
	{
	}

	template <typename... T>
	void set_names(char const *format, T &&... start_args)
	{
		static_assert(sizeof...(N) == sizeof...(T));
		assert(!m_resolved);
		assert(format);
		m_device = m_device.get().mconfig().current_device();
		m_spec = format_spec{ format, { std::forward<T>(start_args)... } };
		m_resolver = &output_finder::resolve_format;
		m_validator = &output_finder::validate_format;
	}

	template <typename T>
	void set_names(T const &names)
	{
		assert(!m_resolved);
		assert(std::size(names) == std::size(m_proxies));
		m_device = m_device.get().mconfig().current_device();
		m_spec = std::cref(names);
		m_resolver = &output_finder::resolve_array<T>;
		m_validator = &output_finder::validate_array<T>;
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
};

template <typename X, unsigned... N>
bool output_manager::output_finder<X, N...>::findit(validity_checker *valid)
{
	if (!valid)
	{
		assert(!m_resolved);
		m_resolved = true;
		return m_resolver(*this);
	}
	else
	{
		return m_validator(*this, *valid);
	}
}

template <typename X, unsigned... N>
void output_manager::output_finder<X, N...>::end_configuration()
{
	assert(!m_resolved);
}

template <typename X, unsigned... N>
template <unsigned A, unsigned C, typename T, typename... U>
bool output_manager::output_finder<X, N...>::resolve_format(format_spec const &spec, T (&proxies)[C], U &&... i)
{
	bool result = true;
	for (unsigned j = 0U; C > j; ++j)
	{
		if constexpr (std::is_same_v<T, item_creator_proxy>)
		{
			if (!proxies[j].resolve(m_device, util::string_format(spec.format, std::forward<U>(i)..., j + spec.start_args[A])))
				result = false;
		}
		else
		{
			if (!resolve_format<A + 1>(spec, proxies[j], std::forward<U>(i)..., j + spec.start_args[A]))
				result = false;
		}
	}
	return result;
}

template <typename X, unsigned... N>
template <unsigned A, unsigned C, typename T, typename... U>
bool output_manager::output_finder<X, N...>::validate_format(validity_checker &valid, format_spec const &spec, T (&proxies)[C], U &&... i)
{
	bool result = true;
	for (unsigned j = 0U; C > j; ++j)
	{
		if constexpr (std::is_same_v<T, item_creator_proxy>)
		{
			auto name = util::string_format(spec.format, std::forward<U>(i)..., j + spec.start_args[A]);
			if (!validate_name(m_device, name))
				result = false;
		}
		else
		{
			if (!validate_format<A + 1>(valid, spec, proxies[j], std::forward<U>(i)..., j + spec.start_args[A]))
				result = false;
		}
	}
	return result;
}

template <typename X, unsigned... N>
template <typename T, typename U, unsigned C>
bool output_manager::output_finder<X, N...>::resolve_array(T const &spec, U (&proxies)[C])
{
	assert(std::size(spec) == C);
	bool result = std::size(spec) == C;
	auto i = std::begin(spec);
	auto j = std::begin(proxies);
	while ((std::end(spec) != i) && (std::end(proxies) != j))
	{
		if constexpr (std::is_same_v<U, item_creator_proxy>)
		{
			if (!j->resolve(m_device, *i))
				result = false;
		}
		else
		{
			if (!resolve_array(*i, *j))
				result = false;
		}
		++i;
		++j;
	}
	return result;
}

template <typename X, unsigned... N>
template <typename T, typename U, unsigned C>
bool output_manager::output_finder<X, N...>::validate_array(validity_checker &valid, T const &spec, U (&proxies)[C])
{
	bool result = true;
	if (std::size(spec) != C)
	{
		result = false;
		osd_printf_error("Size of names %u does not match size of outputs %u\n",
				std::size(spec), C);
	}

	auto i = std::begin(spec);
	auto j = std::begin(proxies);
	while ((std::end(spec) != i) && (std::end(proxies) != j))
	{
		if constexpr (std::is_same_v<U, item_creator_proxy>)
		{
			if constexpr (std::is_pointer_v<std::remove_reference_t<decltype(*i)> >)
			{
				if (!*i)
				{
					result = false;
					osd_printf_error("Output name must not be a null pointer\n");
				}
				else if (!validate_name(m_device, *i))
				{
					result = false;
				}
			}
			else if (!validate_name(m_device, *i))
			{
				result = false;
			}
		}
		else
		{
			if (!validate_array(valid, *i, *j))
				result = false;
		}
		++i;
		++j;
	}
	return result;
}

template <typename X, unsigned... N>
bool output_manager::output_finder<X, N...>::resolve_format(output_finder &obj)
{
	return obj.resolve_format<0U>(*std::any_cast<format_spec>(&obj.m_spec), obj.m_proxies);
}

template <typename X, unsigned... N>
bool output_manager::output_finder<X, N...>::validate_format(output_finder &obj, validity_checker &valid)
{
	return obj.validate_format<0U>(valid, *std::any_cast<format_spec>(&obj.m_spec), obj.m_proxies);
}

template <typename X, unsigned... N>
template <typename T>
bool output_manager::output_finder<X, N...>::resolve_array(output_finder &obj)
{
	T const &names = *std::any_cast<std::reference_wrapper<T const> >(&obj.m_spec);
	return obj.resolve_array(names, obj.m_proxies);
}

template <typename X, unsigned... N>
template <typename T>
bool output_manager::output_finder<X, N...>::validate_array(output_finder &obj, validity_checker &valid)
{
	T const &names = *std::any_cast<std::reference_wrapper<T const> >(&obj.m_spec);
	return obj.validate_array(valid, names, obj.m_proxies);
}


template <typename X>
class output_manager::output_finder<X> : protected device_resolver_base
{
private:
	virtual bool findit(validity_checker *valid) override ATTR_COLD;
	virtual void end_configuration() override ATTR_COLD;

	std::reference_wrapper<device_t>    m_device;
	char const                          *m_name;
	item_creator_proxy                  m_proxy;
	bool                                m_resolved;

public:
	output_finder(device_t &device, char const *name)
		: device_resolver_base(device)
		, m_device(device)
		, m_name(name)
		, m_resolved(false)
	{
	}

	void set_name(char const *name)
	{
		assert(!m_resolved);
		assert(name);
		m_device = m_device.get().mconfig().current_device();
		m_name = name;
	}

	operator s32() const { return m_proxy; }
	s32 operator=(s32 value) { return m_proxy = value; }
};

template <typename X>
bool output_manager::output_finder<X>::findit(validity_checker *valid)
{
	if (!valid)
	{
		assert(!m_resolved);
		m_resolved = true;
		return m_proxy.resolve(m_device, m_name);
	}
	else if (!m_name)
	{
		osd_printf_error("Output name must not be a null pointer\n");
		return false;
	}
	else
	{
		return validate_name(m_device, m_name);
	}
}

template <typename X>
void output_manager::output_finder<X>::end_configuration()
{
	assert(!m_resolved);
}


class output_manager::output_proxy
{
public:
	output_proxy(output_proxy const &) = delete;
	output_proxy operator=(output_proxy const &) = delete;

	output_proxy() noexcept;
	output_proxy(output_proxy &&that) noexcept;
	output_proxy(device_t &device, std::string_view name);
	output_proxy &operator=(output_proxy &&that) noexcept;

	bool exists() const { return bool(m_item); }
	s32 get() const { return m_value; }
	void set(s32 value) { if (m_item) m_item->set(value); else m_local_value = value; }

private:
	item_impl const                     *m_item;
	std::reference_wrapper<s32 const>   m_value;
	s32                                 m_local_value;
};

#endif // MAME_EMU_OUTPUT_IPP
