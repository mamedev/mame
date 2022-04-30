// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    devcb.h

    Device callback interface helpers.

***************************************************************************/

#ifndef MAME_EMU_DEVCB_H
#define MAME_EMU_DEVCB_H

#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>



//**************************************************************************
//  DETECT PROBLEMATIC COMPILERS
//**************************************************************************

#if defined(__GNUC__) && !defined(__clang__)
#if (__GNUC__ >= 8)
#define MAME_DEVCB_GNUC_BROKEN_FRIEND 1
#endif // (__GNUC__ >= 8) && !defined(__clang__)
#endif // defined(__GNUC__)

#if defined(__clang__)
#if (__clang_major__ == 8)
#define MAME_DEVCB_GNUC_BROKEN_FRIEND 1
#endif // (__clang_major__ == 8)
#endif // defined(__clang__)

//**************************************************************************
//  DELEGATE TYPES
//**************************************************************************

typedef device_delegate<int ()> read_line_delegate;
typedef device_delegate<void (int)> write_line_delegate;

namespace emu::detail {

template <typename T> struct rw_delegate_type<T, void_t<rw_device_class_t<read_line_delegate, std::remove_reference_t<T> > > > { using type = read_line_delegate; using device_class = rw_device_class_t<type, std::remove_reference_t<T> >; };
template <typename T> struct rw_delegate_type<T, void_t<rw_device_class_t<write_line_delegate, std::remove_reference_t<T> > > > { using type = write_line_delegate; using device_class = rw_device_class_t<type, std::remove_reference_t<T> >; };

} // namespace emu::detail


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/// \brief Base callback helper
///
/// Provides utilities for supporting multiple read/write/transform
/// signatures, and the base exclusive-or/mask transform methods.
class devcb_base
{
public:
	virtual void validity_check(validity_checker &valid) const = 0;

protected:
	// This is in C++17 but not C++14
	template <typename... T> struct void_wrapper { using type = void; };
	template <typename... T> using void_t = typename void_wrapper<T...>::type;

	// Intermediate is larger of input and output, mask is forced to unsigned
	template <typename T, typename U, typename Enable = void> struct intermediate;
	template <typename T, typename U> struct intermediate<T, U, std::enable_if_t<sizeof(T) >= sizeof(U)> > { using type = T; };
	template <typename T, typename U> struct intermediate<T, U, std::enable_if_t<sizeof(T) < sizeof(U)> > { using type = U; };
	template <typename T, typename U> using intermediate_t = typename intermediate<T, U>::type;
	template <typename T, typename U> using mask_t = std::make_unsigned_t<intermediate_t<T, U> >;

	// Detecting candidates for transform functions
	template <typename Input, typename Result, typename Func> using is_transform_form3 = std::is_invocable_r<Result, Func, offs_t &, Input, std::make_unsigned_t<Input> &>;
	template <typename Input, typename Result, typename Func> using is_transform_form4 = std::is_invocable_r<Result, Func, offs_t &, Input>;
	template <typename Input, typename Result, typename Func> using is_transform_form6 = std::is_invocable_r<Result, Func, Input>;
	template <typename Input, typename Result, typename Func> using is_transform = std::bool_constant<is_transform_form3<Input, Result, Func>::value || is_transform_form4<Input, Result, Func>::value || is_transform_form6<Input, Result, Func>::value>;

	// Determining the result type of a transform function
	template <typename Input, typename Result, typename Func, typename Enable = void> struct transform_result;
	template <typename Input, typename Result, typename Func> struct transform_result<Input, Result, Func, std::enable_if_t<is_transform_form3<Input, Result, Func>::value> > { using type = std::invoke_result_t<Func, offs_t &, Input, std::make_unsigned_t<Input> &>; };
	template <typename Input, typename Result, typename Func> struct transform_result<Input, Result, Func, std::enable_if_t<is_transform_form4<Input, Result, Func>::value> > { using type = std::invoke_result_t<Func, offs_t &, Input>; };
	template <typename Input, typename Result, typename Func> struct transform_result<Input, Result, Func, std::enable_if_t<is_transform_form6<Input, Result, Func>::value> > { using type = std::invoke_result_t<Func, Input>; };
	template <typename Input, typename Result, typename Func> using transform_result_t = typename transform_result<Input, Result, Func>::type;

	// Mapping method types to delegate types
	template <typename T> using delegate_type_t = emu::detail::rw_delegate_type_t<T>;
	template <typename T> using delegate_device_class_t = emu::detail::rw_delegate_device_class_t<T>;

	// Invoking transform callbacks
	template <typename Input, typename Result, typename T> static std::enable_if_t<is_transform_form3<Input, Result, T>::value, mask_t<transform_result_t<Input, Result, T>, Result> > invoke_transform(T const &cb, offs_t &offset, Input data, std::make_unsigned_t<Input> &mem_mask) { return std::make_unsigned_t<transform_result_t<Input, Result, T> >(cb(offset, data, mem_mask)); }
	template <typename Input, typename Result, typename T> static std::enable_if_t<is_transform_form4<Input, Result, T>::value, mask_t<transform_result_t<Input, Result, T>, Result> > invoke_transform(T const &cb, offs_t &offset, Input data, std::make_unsigned_t<Input> &mem_mask) { return std::make_unsigned_t<transform_result_t<Input, Result, T> >(cb(offset, data)); }
	template <typename Input, typename Result, typename T> static std::enable_if_t<is_transform_form6<Input, Result, T>::value, mask_t<transform_result_t<Input, Result, T>, Result> > invoke_transform(T const &cb, offs_t &offset, Input data, std::make_unsigned_t<Input> &mem_mask) { return std::make_unsigned_t<transform_result_t<Input, Result, T> >(cb(data)); }

	// Working with devices and interfaces
	template <typename T> static std::enable_if_t<emu::detail::is_device_implementation<T>::value, const char *> get_tag(T &obj) { return obj.tag(); }
	template <typename T> static std::enable_if_t<emu::detail::is_device_interface<T>::value, const char *> get_tag(T &obj) { return obj.device().tag(); }
	template <typename T, typename U> static T &cast_reference(U &obj)
	{
		if constexpr (std::is_convertible_v<std::add_pointer_t<U>, std::add_pointer_t<T> >)
			return downcast<T &>(obj);
		else
			return dynamic_cast<T &>(obj);
	}

	/// \brief Base transform helper
	///
	/// Provides member functions for setting exclusive-or, mask and
	/// shifts.  Exclusive-or and mask values are stored; it's assumed
	/// that the implementation supports lamba transforms to allow
	/// shifts.
	template <typename T, typename Impl>
	class transform_base
	{
	public:
		Impl &exor(std::make_unsigned_t<T> val) { m_exor ^= val; return static_cast<Impl &>(*this); }
		Impl &mask(std::make_unsigned_t<T> val) { m_mask = m_inherited_mask ? val : (m_mask & val); m_inherited_mask = false; return static_cast<Impl &>(*this); }
		Impl &invert() { return exor(~std::make_unsigned_t<T>(0)); }

		auto rshift(unsigned val)
		{
			auto trans(static_cast<Impl &>(*this).transform([val] (offs_t offset, T data, std::make_unsigned_t<T> &mem_mask) { mem_mask >>= val; return data >> val; }));
			if (inherited_mask())
				return trans;
			else
				return std::move(trans.mask(m_mask >> val));
		}
		auto lshift(unsigned val)
		{
			auto trans(static_cast<Impl &>(*this).transform([val] (offs_t offset, T data, std::make_unsigned_t<T> &mem_mask) { mem_mask <<= val; return data << val; }));
			if (inherited_mask())
				return trans;
			else
				return std::move(trans.mask(m_mask << val));
		}
		auto bit(unsigned val) { return std::move(rshift(val).mask(T(1U))); }

		constexpr std::make_unsigned_t<T> exor() const { return m_exor & m_mask; }
		constexpr std::make_unsigned_t<T> mask() const { return m_mask; }

	protected:
		constexpr bool need_exor() const { return std::make_unsigned_t<T>(0) != (m_exor & m_mask); }
		constexpr bool need_mask() const { return std::make_unsigned_t<T>(~std::make_unsigned_t<T>(0)) != m_mask; }

		constexpr bool inherited_mask() const { return m_inherited_mask; }

		constexpr transform_base(std::make_unsigned_t<T> mask) : m_mask(mask) { }
		constexpr transform_base(transform_base const &) = default;
		transform_base(transform_base &&) = default;
		transform_base &operator=(transform_base const &) = default;
		transform_base &operator=(transform_base &&) = default;

	private:
		std::make_unsigned_t<T> m_exor = std::make_unsigned_t<T>(0);
		std::make_unsigned_t<T> m_mask;
		bool m_inherited_mask = true;
	};

	/// \brief Callback array helper
	///
	/// Simplifies construction and resolution of arrays of callbacks.
	template <typename T, unsigned Count>
	class array : public std::array<T, Count>
	{
	private:
		template <unsigned... V>
		array(device_t &owner, std::integer_sequence<unsigned, V...> const &)
			: std::array<T, Count>{{ { make_one<V>(owner) }... }}
		{
		}

		template <unsigned N> device_t &make_one(device_t &owner) { return owner; }

	public:
		using std::array<T, Count>::array;

		array(device_t &owner) : array(owner, std::make_integer_sequence<unsigned, Count>()) { }

		void resolve_all()
		{
			for (T &elem : *this)
				elem.resolve();
		}
	};

	devcb_base(device_t &owner);
	~devcb_base();

	device_t &owner() const { return m_owner; }

private:
	device_t &m_owner;
};


/// \brief Read callback utilities
///
/// Helpers that don't need to be templated on callback type.
class devcb_read_base : public devcb_base
{
protected:
	// Detecting candidates for read functions
	template <typename Result, typename Func> using is_read_form1 = std::is_invocable_r<Result, Func, offs_t, Result>;
	template <typename Result, typename Func> using is_read_form2 = std::is_invocable_r<Result, Func, offs_t>;
	template <typename Result, typename Func> using is_read_form3 = std::is_invocable_r<Result, Func>;
	template <typename Result, typename Func> using is_read = std::bool_constant<is_read_form1<Result, Func>::value || is_read_form2<Result, Func>::value || is_read_form3<Result, Func>::value>;

	// Determining the result type of a read function
	template <typename Result, typename Func, typename Enable = void> struct read_result;
	template <typename Result, typename Func> struct read_result<Result, Func, std::enable_if_t<is_read_form1<Result, Func>::value> > { using type = std::invoke_result_t<Func, offs_t, std::make_unsigned_t<Result>>; };
	template <typename Result, typename Func> struct read_result<Result, Func, std::enable_if_t<is_read_form2<Result, Func>::value> > { using type = std::invoke_result_t<Func, offs_t>; };
	template <typename Result, typename Func> struct read_result<Result, Func, std::enable_if_t<is_read_form3<Result, Func>::value> > { using type = std::invoke_result_t<Func>; };
	template <typename Result, typename Func> using read_result_t = typename read_result<Result, Func>::type;

	// Detecting candidates for read delegates
	template <typename T, typename Enable = void> struct is_read_method : public std::false_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read8s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read16s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read32s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read64s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read8sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read16sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read32sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read64sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read8smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read16smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read32smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read64smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_read_method<T, void_t<emu::detail::rw_device_class_t<read_line_delegate, std::remove_reference_t<T> > > > : public std::true_type { };

	// Invoking read callbacks
	template <typename Result, typename T> static std::enable_if_t<is_read_form1<Result, T>::value, mask_t<read_result_t<Result, T>, Result> > invoke_read(T const &cb, offs_t offset, std::make_unsigned_t<Result> mem_mask) { return std::make_unsigned_t<read_result_t<Result, T> >(cb(offset, mem_mask)); }
	template <typename Result, typename T> static std::enable_if_t<is_read_form2<Result, T>::value, mask_t<read_result_t<Result, T>, Result> > invoke_read(T const &cb, offs_t offset, std::make_unsigned_t<Result> mem_mask) { return std::make_unsigned_t<read_result_t<Result, T> >(cb(offset)); }
	template <typename Result, typename T> static std::enable_if_t<is_read_form3<Result, T>::value, mask_t<read_result_t<Result, T>, Result> > invoke_read(T const &cb, offs_t offset, std::make_unsigned_t<Result> mem_mask) { return std::make_unsigned_t<read_result_t<Result, T> >(cb()); }

	// Delegate characteristics
	template <typename T, typename Dummy = void> struct delegate_traits;
	template <typename Dummy> struct delegate_traits<read8s_delegate, Dummy> { static constexpr u8 default_mask = ~u8(0); };
	template <typename Dummy> struct delegate_traits<read16s_delegate, Dummy> { static constexpr u16 default_mask = ~u16(0); };
	template <typename Dummy> struct delegate_traits<read32s_delegate, Dummy> { static constexpr u32 default_mask = ~u32(0); };
	template <typename Dummy> struct delegate_traits<read64s_delegate, Dummy> { static constexpr u64 default_mask = ~u64(0); };
	template <typename Dummy> struct delegate_traits<read8sm_delegate, Dummy> { static constexpr u8 default_mask = ~u8(0); };
	template <typename Dummy> struct delegate_traits<read16sm_delegate, Dummy> { static constexpr u16 default_mask = ~u16(0); };
	template <typename Dummy> struct delegate_traits<read32sm_delegate, Dummy> { static constexpr u32 default_mask = ~u32(0); };
	template <typename Dummy> struct delegate_traits<read64sm_delegate, Dummy> { static constexpr u64 default_mask = ~u64(0); };
	template <typename Dummy> struct delegate_traits<read8smo_delegate, Dummy> { static constexpr u8 default_mask = ~u8(0); };
	template <typename Dummy> struct delegate_traits<read16smo_delegate, Dummy> { static constexpr u16 default_mask = ~u16(0); };
	template <typename Dummy> struct delegate_traits<read32smo_delegate, Dummy> { static constexpr u32 default_mask = ~u32(0); };
	template <typename Dummy> struct delegate_traits<read64smo_delegate, Dummy> { static constexpr u64 default_mask = ~u64(0); };
	template <typename Dummy> struct delegate_traits<read_line_delegate, Dummy> { static constexpr unsigned default_mask = 1U; };

	using devcb_base::devcb_base;
	~devcb_read_base();
};


/// \brief Write callback utilities
///
/// Helpers that don't need to be templated on callback type.
class devcb_write_base : public devcb_base
{
protected:
	// Detecting candidates for write functions
	template <typename Input, typename Func> using is_write_form1 = std::is_invocable<Func, offs_t, Input, std::make_unsigned_t<Input> >;
	template <typename Input, typename Func> using is_write_form2 = std::is_invocable<Func, offs_t, Input>;
	template <typename Input, typename Func> using is_write_form3 = std::is_invocable<Func, Input>;
	template <typename Input, typename Func> using is_write = std::bool_constant<is_write_form1<Input, Func>::value || is_write_form2<Input, Func>::value || is_write_form3<Input, Func>::value>;

	// Detecting candidates for write delegates
	template <typename T, typename Enable = void> struct is_write_method : public std::false_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write8s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write16s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write32s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write64s_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write8sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write16sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write32sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write64sm_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write8smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write16smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write32smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write64smo_delegate, std::remove_reference_t<T> > > > : public std::true_type { };
	template <typename T> struct is_write_method<T, void_t<emu::detail::rw_device_class_t<write_line_delegate, std::remove_reference_t<T> > > > : public std::true_type { };

	// Invoking write callbacks
	template <typename Input, typename T> static std::enable_if_t<is_write_form1<Input, T>::value> invoke_write(T const &cb, offs_t &offset, Input data, std::make_unsigned_t<Input> mem_mask) { return cb(offset, data, mem_mask); }
	template <typename Input, typename T> static std::enable_if_t<is_write_form2<Input, T>::value> invoke_write(T const &cb, offs_t &offset, Input data, std::make_unsigned_t<Input> mem_mask) { return cb(offset, data); }
	template <typename Input, typename T> static std::enable_if_t<is_write_form3<Input, T>::value> invoke_write(T const &cb, offs_t &offset, Input data, std::make_unsigned_t<Input> mem_mask) { return cb(data); }

	// Delegate characteristics
	template <typename T, typename Dummy = void> struct delegate_traits;
	template <typename Dummy> struct delegate_traits<write8s_delegate, Dummy> { using input_t = u8; static constexpr u8 default_mask = ~u8(0); };
	template <typename Dummy> struct delegate_traits<write16s_delegate, Dummy> { using input_t = u16; static constexpr u16 default_mask = ~u16(0); };
	template <typename Dummy> struct delegate_traits<write32s_delegate, Dummy> { using input_t = u32; static constexpr u32 default_mask = ~u32(0); };
	template <typename Dummy> struct delegate_traits<write64s_delegate, Dummy> { using input_t = u64; static constexpr u64 default_mask = ~u64(0); };
	template <typename Dummy> struct delegate_traits<write8sm_delegate, Dummy> { using input_t = u8; static constexpr u8 default_mask = ~u8(0); };
	template <typename Dummy> struct delegate_traits<write16sm_delegate, Dummy> { using input_t = u16; static constexpr u16 default_mask = ~u16(0); };
	template <typename Dummy> struct delegate_traits<write32sm_delegate, Dummy> { using input_t = u32; static constexpr u32 default_mask = ~u32(0); };
	template <typename Dummy> struct delegate_traits<write64sm_delegate, Dummy> { using input_t = u64; static constexpr u64 default_mask = ~u64(0); };
	template <typename Dummy> struct delegate_traits<write8smo_delegate, Dummy> { using input_t = u8; static constexpr u8 default_mask = ~u8(0); };
	template <typename Dummy> struct delegate_traits<write16smo_delegate, Dummy> { using input_t = u16; static constexpr u16 default_mask = ~u16(0); };
	template <typename Dummy> struct delegate_traits<write32smo_delegate, Dummy> { using input_t = u32; static constexpr u32 default_mask = ~u32(0); };
	template <typename Dummy> struct delegate_traits<write64smo_delegate, Dummy> { using input_t = u64; static constexpr u64 default_mask = ~u64(0); };
	template <typename Dummy> struct delegate_traits<write_line_delegate, Dummy> { using input_t = int; static constexpr unsigned default_mask = 1U; };

	using devcb_base::devcb_base;
	~devcb_write_base();
};


/// \brief Read callback helper
///
/// Allows binding a variety of signatures, composing a result from
/// multiple callbacks, and chained arbitrary transforms.  Transforms
/// may modify the offset and mask.
template <typename Result, std::make_unsigned_t<Result> DefaultMask = std::make_unsigned_t<Result>(~std::make_unsigned_t<Result>(0))>
class devcb_read : public devcb_read_base
{
private:
	using func_t = std::function<Result (offs_t, std::make_unsigned_t<Result>)>;

	class creator
	{
	public:
		using ptr = std::unique_ptr<creator>;

		virtual ~creator() { }
		virtual void validity_check(validity_checker &valid) const = 0;
		virtual func_t create() = 0;

		std::make_unsigned_t<Result> mask() const { return m_mask; }

	protected:
		creator(std::make_unsigned_t<Result> mask) : m_mask(mask) { }

		std::make_unsigned_t<Result> m_mask;
	};

	template <typename T>
	class creator_impl : public creator
	{
	public:
		creator_impl(T &&builder) : creator(builder.mask()), m_builder(std::move(builder)) { }

		virtual void validity_check(validity_checker &valid) const override { m_builder.validity_check(valid); }

		virtual func_t create() override
		{
			func_t result;
			m_builder.build([&result] (auto &&f) { result = [cb = std::move(f)] (offs_t offset, typename T::input_mask_t mem_mask) { return cb(offset, mem_mask); }; });
			return result;
		}

	private:
		T m_builder;
	};

	class log_creator : public creator
	{
	public:
		log_creator(device_t &devbase, std::string &&message) : creator(0U), m_devbase(devbase), m_message(std::move(message)) { }

		virtual void validity_check(validity_checker &valid) const override { }

		virtual func_t create() override
		{
			return
					[&devbase = m_devbase, message = std::move(m_message)] (offs_t offset, std::make_unsigned_t<Result> mem_mask)
					{
						devbase.logerror("%s: %s\n", devbase.machine().describe_context(), message);
						return Result(0);
					};
		}

	private:
		device_t &m_devbase;
		std::string m_message;
	};

	template <typename Source, typename Func> class transform_builder; // workaround for MSVC

	class builder_base
	{
	protected:
		template <typename T, typename U> friend class transform_builder; // workaround for MSVC

		builder_base(devcb_read &target, bool append) : m_target(target), m_append(append) { }
		builder_base(builder_base const &) = delete;
		builder_base(builder_base &&) = default;
		~builder_base() { assert(m_consumed); }
		builder_base &operator=(builder_base const &) = delete;
		builder_base &operator=(builder_base &&) = default;

#ifdef MAME_DEVCB_GNUC_BROKEN_FRIEND
	public:
#endif
		void consume() { m_consumed = true; }
#ifdef MAME_DEVCB_GNUC_BROKEN_FRIEND
	protected:
#endif
		void built() { assert(!m_built); m_built = true; }

		template <typename T>
		void register_creator()
		{
			if (!m_consumed)
			{
				if (!m_append)
					m_target.m_creators.clear();
				consume();
				m_target.m_creators.emplace_back(std::make_unique<creator_impl<T> >(std::move(static_cast<T &>(*this))));
			}
		}

		devcb_read &m_target;
		bool const m_append;
		bool m_consumed = false;
		bool m_built = false;
	};

	template <typename Source, typename Func>
	class transform_builder : public builder_base, public transform_base<mask_t<transform_result_t<typename Source::output_t, Result, Func>, Result>, transform_builder<Source, Func> >
	{
	public:
		template <typename T, typename U> friend class transform_builder;

		using input_t = typename Source::output_t;
		using output_t = mask_t<transform_result_t<input_t, Result, Func>, Result>;
		using input_mask_t = mask_t<input_t, typename Source::input_mask_t>;

		template <typename T>
		transform_builder(devcb_read &target, bool append, Source &&src, T &&cb, output_t mask)
			: builder_base(target, append)
			, transform_base<output_t, transform_builder>(mask)
			, m_src(std::move(src))
			, m_cb(std::forward<T>(cb))
		{ m_src.consume(); }
		transform_builder(transform_builder &&that)
			: builder_base(std::move(that))
			, transform_base<output_t, transform_builder>(std::move(that))
			, m_src(std::move(that.m_src))
			, m_cb(std::move(that.m_cb))
		{
			m_src.consume();
			that.consume();
			that.built();
		}
		~transform_builder() { this->template register_creator<transform_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<output_t, Result, T>::value, transform_builder<transform_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			output_t const m(this->mask());
			if (this->inherited_mask())
				this->mask(std::make_unsigned_t<transform_result_t<typename Source::output_t, Result, Func> >(~std::make_unsigned_t<transform_result_t<typename Source::output_t, Result, Func> >(0)));
			return transform_builder<transform_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, std::move(*this), std::forward<T>(cb), m);
		}

		void validity_check(validity_checker &valid) const { m_src.validity_check(valid); }

		template <typename T>
		void build(T &&chain)
		{
			assert(this->m_consumed);
			auto wrap([this, c = std::forward<T>(chain)] (auto &&f) mutable { this->build(std::move(c), std::move(f)); });
			m_src.build(std::move(wrap));
		}

	private:
		transform_builder(transform_builder const &) = delete;
		transform_builder &operator=(transform_builder const &) = delete;
		transform_builder &operator=(transform_builder &&that) = delete;

		template <typename T, typename U>
		void build(T &&chain, U &&f)
		{
			assert(this->m_consumed);
			this->built();
			chain(
					[src = std::forward<U>(f), cb = std::move(m_cb), exor = this->exor(), mask = this->mask()] (offs_t &offset, input_mask_t &mem_mask)
					{
						typename Source::input_mask_t source_mask(mem_mask);
						auto const data(src(offset, source_mask));
						mem_mask = source_mask & mask;
						return (devcb_read::invoke_transform<input_t, Result>(cb, offset, data, mem_mask) ^ exor) & mask;
					});
		}

		Source m_src;
		Func m_cb;
	};

	template <typename Func>
	class functoid_builder : public builder_base, public transform_base<mask_t<read_result_t<Result, Func>, Result>, functoid_builder<Func> >
	{
	public:
		template <typename T, typename U> friend class transform_builder;

		using output_t = mask_t<read_result_t<Result, Func>, Result>;
		using input_mask_t = std::make_unsigned_t<Result>;

		template <typename T>
		functoid_builder(devcb_read &target, bool append, T &&cb)
			: builder_base(target, append)
			, transform_base<output_t, functoid_builder>(DefaultMask)
			, m_cb(std::forward<T>(cb))
		{ }
		functoid_builder(functoid_builder &&that)
			: builder_base(std::move(that))
			, transform_base<output_t, functoid_builder>(std::move(that))
			, m_cb(std::move(that.m_cb))
		{
			that.consume();
			that.built();
		}
		~functoid_builder() { this->template register_creator<functoid_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<output_t, Result, T>::value, transform_builder<functoid_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			output_t const m(this->mask());
			if (this->inherited_mask())
				this->mask(std::make_unsigned_t<read_result_t<Result, Func> >(~std::make_unsigned_t<read_result_t<Result, Func> >(0)));
			return transform_builder<functoid_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, std::move(*this), std::forward<T>(cb), m);
		}

		void validity_check(validity_checker &valid) const { }

		template <typename T>
		void build(T &&chain)
		{
			assert(this->m_consumed);
			this->built();
			chain(
					[cb = std::move(m_cb), exor = this->exor(), mask = this->mask()] (offs_t offset, input_mask_t mem_mask)
					{ return (devcb_read::invoke_read<Result>(cb, offset, mem_mask & mask) ^ exor) & mask; });
		}

	private:
		functoid_builder(functoid_builder const &) = delete;
		functoid_builder &operator=(functoid_builder const &) = delete;
		functoid_builder &operator=(functoid_builder &&that) = delete;

		Func m_cb;
	};

	template <typename Delegate>
	class delegate_builder : public builder_base, public transform_base<mask_t<read_result_t<Result, Delegate>, Result>, delegate_builder<Delegate> >
	{
	public:
		template <typename T, typename U> friend class transform_builder;

		using output_t = mask_t<read_result_t<Result, Delegate>, Result>;
		using input_mask_t = std::make_unsigned_t<Result>;

		template <typename T>
		delegate_builder(devcb_read &target, bool append, device_t &devbase, char const *tag, T &&func, char const *name)
			: builder_base(target, append)
			, transform_base<output_t, delegate_builder>(DefaultMask & delegate_traits<Delegate>::default_mask)
			, m_delegate(devbase, tag, std::forward<T>(func), name)
		{ }
		template <typename T>
		delegate_builder(devcb_read &target, bool append, device_t &devbase, devcb_read::delegate_device_class_t<T> &obj, T &&func, char const *name)
			: builder_base(target, append)
			, transform_base<output_t, delegate_builder>(DefaultMask & delegate_traits<Delegate>::default_mask)
			, m_delegate(obj, std::forward<T>(func), name)
		{ }
		delegate_builder(delegate_builder &&that)
			: builder_base(std::move(that))
			, transform_base<output_t, delegate_builder>(std::move(that))
			, m_delegate(std::move(that.m_delegate))
		{
			that.consume();
			that.built();
		}
		~delegate_builder() { this->template register_creator<delegate_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<output_t, Result, T>::value, transform_builder<delegate_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			output_t const m(this->mask());
			if (this->inherited_mask())
				this->mask(delegate_traits<Delegate>::default_mask);
			return transform_builder<delegate_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, std::move(*this), std::forward<T>(cb), m);
		}

		void validity_check(validity_checker &valid) const
		{
			auto const target(m_delegate.finder_target());
			if (target.second && !target.first.subdevice(target.second))
				osd_printf_error("Read callback bound to non-existent object tag %s (%s)\n", target.first.subtag(target.second), m_delegate.name());
		}

		template <typename T>
		void build(T &&chain)
		{
			assert(this->m_consumed);
			this->built();
			m_delegate.resolve();
			chain(
					[cb = std::move(m_delegate), exor = this->exor(), mask = this->mask()] (offs_t offset, input_mask_t mem_mask)
					{ return (devcb_read::invoke_read<Result>(cb, offset, mem_mask & mask) ^ exor) & mask; });
		}

	private:
		delegate_builder(delegate_builder const &) = delete;
		delegate_builder &operator=(delegate_builder const &) = delete;
		delegate_builder &operator=(delegate_builder &&that) = delete;

		Delegate m_delegate;
	};

	class ioport_builder : public builder_base, public transform_base<mask_t<ioport_value, Result>, ioport_builder>
	{
	public:
		template <typename T, typename U> friend class transform_builder;

		using output_t = mask_t<ioport_value, Result>;
		using input_mask_t = std::make_unsigned_t<Result>;

		ioport_builder(devcb_read &target, bool append, device_t &devbase, std::string &&tag)
			: builder_base(target, append)
			, transform_base<output_t, ioport_builder>(DefaultMask)
			, m_devbase(devbase)
			, m_tag(std::move(tag))
		{ }
		ioport_builder(ioport_builder &&that)
			: builder_base(std::move(that))
			, transform_base<output_t, ioport_builder>(std::move(that))
			, m_devbase(that.m_devbase)
			, m_tag(std::move(that.m_tag))
		{
			that.consume();
			that.built();
		}
		~ioport_builder() { this->template register_creator<ioport_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<output_t, Result, T>::value, transform_builder<ioport_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			output_t const m(this->mask());
			if (this->inherited_mask())
				this->mask(std::make_unsigned_t<ioport_value>(~std::make_unsigned_t<ioport_value>(0)));
			return transform_builder<ioport_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, std::move(*this), std::forward<T>(cb), m);
		}

		void validity_check(validity_checker &valid) const { }

		template <typename T>
		void build(T &&chain)
		{
			assert(this->m_consumed);
			this->built();
			ioport_port *const ioport(m_devbase.ioport(m_tag));
			if (!ioport)
				throw emu_fatalerror("Read callback bound to non-existent I/O port %s of device %s (%s)\n", m_tag, m_devbase.tag(), m_devbase.name());
			chain(
					[&port = *ioport, exor = this->exor(), mask = this->mask()] (offs_t offset, input_mask_t mem_mask)
					{ return (port.read() ^ exor) & mask; });
		}

	private:
		ioport_builder(ioport_builder const &) = delete;
		ioport_builder &operator=(ioport_builder const &) = delete;
		ioport_builder &operator=(ioport_builder &&that) = delete;

		device_t &m_devbase;
		std::string m_tag;
	};

	class binder
	{
	public:
		binder(devcb_read &target) : m_target(target) { }
		binder(binder const &) = delete;
		binder(binder &&that) : m_target(that.m_target), m_append(that.m_append), m_used(that.m_used) { that.m_used = true; }
		binder &operator=(binder const &) = delete;
		binder &operator=(binder &&) = delete;

		template <typename T>
		std::enable_if_t<is_read<Result, T>::value, functoid_builder<std::remove_reference_t<T> > > set(T &&cb)
		{
			set_used();
			return functoid_builder<std::remove_reference_t<T> >(m_target, m_append, std::forward<T>(cb));
		}

		template <typename T>
		std::enable_if_t<is_read_method<T>::value, delegate_builder<delegate_type_t<T> > > set(T &&func, char const *name)
		{
			set_used();
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, m_target.owner().mconfig().current_device(), DEVICE_SELF, std::forward<T>(func), name);
		}

		template <typename T>
		std::enable_if_t<is_read_method<T>::value, delegate_builder<delegate_type_t<T> > > set(char const *tag, T &&func, char const *name)
		{
			set_used();
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, m_target.owner().mconfig().current_device(), tag, std::forward<T>(func), name);
		}

		template <typename T, typename U>
		std::enable_if_t<is_read_method<T>::value, delegate_builder<delegate_type_t<T> > > set(U &obj, T &&func, char const *name)
		{
			set_used();
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, m_target.owner(), devcb_read::cast_reference<delegate_device_class_t<T> >(obj), std::forward<T>(func), name);
		}

		template <typename T, typename U, bool R>
		std::enable_if_t<is_read_method<T>::value, delegate_builder<delegate_type_t<T> > > set(device_finder<U, R> &finder, T &&func, char const *name)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, target.first, target.second, std::forward<T>(func), name);
		}

		template <typename T, typename U, bool R>
		std::enable_if_t<is_read_method<T>::value, delegate_builder<delegate_type_t<T> > > set(device_finder<U, R> const &finder, T &&func, char const *name)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, target.first, target.second, std::forward<T>(func), name);
		}

		template <typename... Params>
		auto append(Params &&... args)
		{
			m_append = true;
			return set(std::forward<Params>(args)...);
		}

		template <typename... Params>
		ioport_builder set_ioport(Params &&... args)
		{
			set_used();
			return ioport_builder(m_target, m_append, m_target.owner().mconfig().current_device(), std::string(std::forward<Params>(args)...));
		}

		template <bool R>
		ioport_builder set_ioport(ioport_finder<R> &finder)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return ioport_builder(m_target, m_append, target.first, std::string(target.second));
		}

		template <bool R>
		ioport_builder set_ioport(ioport_finder<R> const &finder)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return ioport_builder(m_target, m_append, target.first, std::string(target.second));
		}

		template <typename... Params>
		ioport_builder append_ioport(Params &&... args)
		{
			m_append = true;
			return set_ioport(std::forward<Params>(args)...);
		}

		template <typename... Params>
		void set_log(device_t &devbase, Params &&... args)
		{
			set_used();
			if (!m_append)
				m_target.m_creators.clear();
			m_target.m_creators.emplace_back(std::make_unique<log_creator>(devbase, std::string(std::forward<Params>(args)...)));
		}

		template <typename T, typename... Params>
		std::enable_if_t<emu::detail::is_device_implementation<std::remove_reference_t<T> >::value> set_log(T &devbase, Params &&... args)
		{
			set_log(static_cast<device_t &>(devbase), std::forward<Params>(args)...);
		}

		template <typename T, typename... Params>
		std::enable_if_t<emu::detail::is_device_interface<std::remove_reference_t<T> >::value> set_log(T &devbase, Params &&... args)
		{
			set_log(devbase.device(), std::forward<Params>(args)...);
		}

		template <typename... Params>
		void set_log(Params &&... args)
		{
			set_log(m_target.owner().mconfig().current_device(), std::forward<Params>(args)...);
		}

		template <typename... Params>
		void append_log(Params &&... args)
		{
			m_append = true;
			set_log(std::forward<Params>(args)...);
		}

		auto set_constant(Result val) { return set([val] () { return val; }); }
		auto append_constant(Result val) { return append([val] () { return val; }); }

	private:
		void set_used() { assert(!m_used); m_used = true; }

		devcb_read &m_target;
		bool m_append = false;
		bool m_used = false;
	};

	std::vector<func_t> m_functions;
	std::vector<typename creator::ptr> m_creators;

public:
	template <unsigned Count>
	class array : public devcb_read_base::array<devcb_read<Result, DefaultMask>, Count>
	{
	public:
		using devcb_read_base::array<devcb_read<Result, DefaultMask>, Count>::array;

		void resolve_all_safe(Result dflt)
		{
			for (devcb_read<Result, DefaultMask> &elem : *this)
				elem.resolve_safe(dflt);
		}
	};

	devcb_read(device_t &owner);

	binder bind();
	void reset();

	virtual void validity_check(validity_checker &valid) const override;

	void resolve();
	void resolve_safe(Result dflt);

	Result operator()(offs_t offset, std::make_unsigned_t<Result> mem_mask = DefaultMask);
	Result operator()();

	bool isnull() const { return m_functions.empty() && m_creators.empty(); }
	explicit operator bool() const { return !m_functions.empty(); }
};

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
devcb_read<Result, DefaultMask>::devcb_read(device_t &owner)
	: devcb_read_base(owner)
{
}

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
typename devcb_read<Result, DefaultMask>::binder devcb_read<Result, DefaultMask>::bind()
{
	return binder(*this);
}

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
void devcb_read<Result, DefaultMask>::reset()
{
	assert(m_functions.empty());
	m_creators.clear();
}

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
void devcb_read<Result, DefaultMask>::validity_check(validity_checker &valid) const
{
	assert(m_functions.empty());
	for (typename std::vector<typename creator::ptr>::const_iterator i = m_creators.begin(); m_creators.end() != i; ++i)
	{
		(*i)->validity_check(valid);
		std::make_unsigned_t<Result> const m((*i)->mask());
		for (typename std::vector<typename creator::ptr>::const_iterator j = std::next(i); m_creators.end() != j; ++j)
		{
			std::make_unsigned_t<Result> const n((*j)->mask());
			if (m & n)
				osd_printf_error("Read callback masks %lX and %lX overlap\n", static_cast<unsigned long>(m), static_cast<unsigned long>(n)); // FIXME: doesn't work with u64
		}
	}
}

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
void devcb_read<Result, DefaultMask>::resolve()
{
	assert(m_functions.empty());
	m_functions.reserve(m_creators.size());
	for (typename creator::ptr const &c : m_creators)
		m_functions.emplace_back(c->create());
	m_creators.clear();
}

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
void devcb_read<Result, DefaultMask>::resolve_safe(Result dflt)
{
	resolve();
	if (m_functions.empty())
		m_functions.emplace_back([dflt] (offs_t offset, std::make_unsigned_t<Result> mem_mask) { return dflt; });
}

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
Result devcb_read<Result, DefaultMask>::operator()(offs_t offset, std::make_unsigned_t<Result> mem_mask)
{
	assert(m_creators.empty() && !m_functions.empty());
	typename std::vector<func_t>::const_iterator it(m_functions.begin());
	std::make_unsigned_t<Result> result((*it)(offset, mem_mask));
	while (m_functions.end() != ++it)
		result |= (*it)(offset, mem_mask);
	return result;
}

template <typename Result, std::make_unsigned_t<Result> DefaultMask>
Result devcb_read<Result, DefaultMask>::operator()()
{
	return this->operator()(0U, DefaultMask);
}


/// \brief Write callback helper
///
/// Allows binding a variety of signatures, sending the value to
/// multiple callbacks, and chained arbitrary transforms.  Transforms
/// may modify the offset and mask.
template <typename Input, std::make_unsigned_t<Input> DefaultMask = std::make_unsigned_t<Input>(~std::make_unsigned_t<Input>(0))>
class devcb_write : public devcb_write_base
{
private:
	using func_t = std::function<void (offs_t, Input, std::make_unsigned_t<Input>)>;

	class creator
	{
	public:
		using ptr = std::unique_ptr<creator>;

		virtual ~creator() { }
		virtual void validity_check(validity_checker &valid) const = 0;
		virtual func_t create() = 0;
	};

	template <typename T>
	class creator_impl : public creator
	{
	public:
		creator_impl(T &&builder) : m_builder(std::move(builder)) { }

		virtual void validity_check(validity_checker &valid) const override { m_builder.validity_check(valid); }

		virtual func_t create() override
		{
			return [cb = m_builder.build()] (offs_t offset, Input data, std::make_unsigned_t<Input> mem_mask) { cb(offset, data, mem_mask); };
		}

	private:
		T m_builder;
	};

	class nop_creator : public creator
	{
	public:
		virtual void validity_check(validity_checker &valid) const override { }
		virtual func_t create() override { return [] (offs_t offset, Input data, std::make_unsigned_t<Input> mem_mask) { }; }
	};

	template <typename Source, typename Func> class transform_builder; // workaround for MSVC
	template <typename Sink, typename Func> class first_transform_builder; // workaround for MSVC
	template <typename Func> class functoid_builder; // workaround for MSVC

	class builder_base
	{
	protected:
		template <typename T, typename U> friend class transform_builder; // workaround for MSVC
		template <typename T, typename U> friend class first_transform_builder; // workaround for MSVC
		template <typename Func> friend class functoid_builder; // workaround for MSVC

		builder_base(devcb_write &target, bool append) : m_target(target), m_append(append) { }
		builder_base(builder_base const &) = delete;
		builder_base(builder_base &&) = default;
		~builder_base() { assert(m_consumed); }
		builder_base &operator=(builder_base const &) = delete;
		builder_base &operator=(builder_base &&) = default;

#ifdef MAME_DEVCB_GNUC_BROKEN_FRIEND
	public:
#endif
		void consume() { m_consumed = true; }
#ifdef MAME_DEVCB_GNUC_BROKEN_FRIEND
	protected:
#endif
		void built() { assert(!m_built); m_built = true; }

		template <typename T>
		void register_creator()
		{
			if (!m_consumed)
			{
				if (!m_append)
					m_target.m_creators.clear();
				consume();
				m_target.m_creators.emplace_back(std::make_unique<creator_impl<T> >(std::move(static_cast<T &>(*this))));
			}
		}

		devcb_write &m_target;
		bool const m_append;
		bool m_consumed = false;
		bool m_built = false;
	};

	template <typename Source, typename Func>
	class transform_builder : public builder_base, public transform_base<mask_t<transform_result_t<typename Source::output_t, typename Source::output_t, Func>, typename Source::output_t>, transform_builder<Source, Func> >
	{
	public:
		template <typename T, typename U> friend class transform_builder;

		using input_t = typename Source::output_t;
		using output_t = mask_t<transform_result_t<typename Source::output_t, typename Source::output_t, Func>, typename Source::output_t>;

		template <typename T>
		transform_builder(devcb_write &target, bool append, Source &&src, T &&cb, output_t mask)
			: builder_base(target, append)
			, transform_base<output_t, transform_builder>(mask)
			, m_src(std::move(src))
			, m_cb(std::forward<T>(cb))
		{ m_src.consume(); }
		transform_builder(transform_builder &&that)
			: builder_base(std::move(that))
			, transform_base<output_t, transform_builder>(std::move(that))
			, m_src(std::move(that.m_src))
			, m_cb(std::move(that.m_cb))
		{
			m_src.consume();
			that.consume();
			that.built();
		}
		~transform_builder() { this->template register_creator<transform_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<output_t, output_t, T>::value, transform_builder<transform_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			output_t const m(this->mask());
			if (this->inherited_mask())
				this->mask(output_t(~output_t(0)));
			return transform_builder<transform_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, std::move(*this), std::forward<T>(cb), m);
		}

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			return m_src.build(
					[cb = std::move(m_cb), exor = this->exor(), mask = this->mask()] (offs_t &offset, input_t data, std::make_unsigned_t<input_t> &mem_mask)
					{
						auto const trans(devcb_write::invoke_transform<input_t, output_t>(cb, offset, data, mem_mask));
						mem_mask &= mask;
						return (trans ^ exor) & mask;
					});
		}

		void validity_check(validity_checker &valid) const { m_src.validity_check(valid); }

	private:
		transform_builder(transform_builder const &) = delete;
		transform_builder &operator=(transform_builder const &) = delete;
		transform_builder &operator=(transform_builder &&that) = delete;

		template <typename T>
		auto build(T &&chain)
		{
			assert(this->m_consumed);
			this->built();
			return m_src.build(
					[f = std::move(chain), cb = std::move(m_cb), exor = this->exor(), mask = this->mask()] (offs_t &offset, input_t data, std::make_unsigned_t<input_t> &mem_mask)
					{
						auto const trans(devcb_write::invoke_transform<input_t, output_t>(cb, offset, data, mem_mask));
						output_t out_mask(mem_mask & mask);
						return f(offset, (trans ^ exor) & mask, out_mask);
					});
		}

		Source m_src;
		Func m_cb;
	};

	template <typename Sink, typename Func>
	class first_transform_builder : public builder_base, public transform_base<mask_t<transform_result_t<typename Sink::input_t, typename Sink::input_t, Func>, typename Sink::input_t>, first_transform_builder<Sink, Func> >
	{
	public:
		template <typename T, typename U> friend class transform_builder;

		using input_t = typename Sink::input_t;
		using output_t = mask_t<transform_result_t<typename Sink::input_t, typename Sink::input_t, Func>, typename Sink::input_t>;

		template <typename T>
		first_transform_builder(devcb_write &target, bool append, Sink &&sink, T &&cb, std::make_unsigned_t<Input> in_exor, std::make_unsigned_t<Input> in_mask, std::make_unsigned_t<output_t> mask)
			: builder_base(target, append)
			, transform_base<output_t, first_transform_builder>(mask)
			, m_sink(std::move(sink))
			, m_cb(std::forward<T>(cb))
			, m_in_exor(in_exor & in_mask)
			, m_in_mask(in_mask)
		{ m_sink.consume(); }
		first_transform_builder(first_transform_builder &&that)
			: builder_base(std::move(that))
			, transform_base<output_t, first_transform_builder>(std::move(that))
			, m_sink(std::move(that.m_sink))
			, m_cb(std::move(that.m_cb))
			, m_in_exor(that.m_in_exor)
			, m_in_mask(that.m_in_mask)
		{
			m_sink.consume();
			that.consume();
			that.built();
		}
		~first_transform_builder() { this->template register_creator<first_transform_builder>(); }

		void validity_check(validity_checker &valid) const { m_sink.validity_check(valid); }

		template <typename T>
		std::enable_if_t<is_transform<output_t, output_t, T>::value, transform_builder<first_transform_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			output_t const m(this->mask());
			if (this->inherited_mask())
				this->mask(output_t(~output_t(0)));
			return transform_builder<first_transform_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, std::move(*this), std::forward<T>(cb), m);
		}

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			return
					[sink = m_sink.build(), cb = std::move(m_cb), in_exor = m_in_exor, in_mask = m_in_mask, exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{
						data = (data ^ in_exor) & in_mask;
						mem_mask &= in_mask;
						auto const trans(devcb_write::invoke_transform<input_t, output_t>(cb, offset, data, mem_mask));
						mem_mask &= mask;
						sink(offset, (trans ^ exor) & mask, mem_mask);
					};
		}

	private:
		first_transform_builder(first_transform_builder const &) = delete;
		first_transform_builder operator=(first_transform_builder const &) = delete;
		first_transform_builder operator=(first_transform_builder &&that) = delete;

		template <typename T>
		auto build(T &&chain)
		{
			assert(this->m_consumed);
			this->built();
			return
					[f = std::move(chain), sink = m_sink.build(), cb = std::move(m_cb), in_exor = m_in_exor, in_mask = m_in_mask, exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{
						data = (data ^ in_exor) & in_mask;
						mem_mask &= in_mask;
						auto const trans_1(devcb_write::invoke_transform<input_t, output_t>(cb, offset, data, mem_mask));
						output_t out_mask(mem_mask & mask);
						auto const trans_n(f(offset, (trans_1 ^ exor) & mask, out_mask));
						sink(offset, trans_n, out_mask);
					};
		}

		Sink m_sink;
		Func m_cb;
		std::make_unsigned_t<Input> const m_in_exor, m_in_mask;
	};

	template <typename Func>
	class functoid_builder : public builder_base, public transform_base<std::make_unsigned_t<Input>, functoid_builder<Func> >
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = Input;

			wrapped_builder(functoid_builder &&that) : builder_base(std::move(that)), m_cb(std::move(that.m_cb)) { that.consume(); that.built(); }
			wrapped_builder(wrapped_builder &&that) : builder_base(std::move(that)), m_cb(std::move(that.m_cb)) { that.consume(); that.built(); }

			void validity_check(validity_checker &valid) const { }

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				return
						[cb = std::move(m_cb)] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ devcb_write::invoke_write<Input>(cb, offset, data, mem_mask); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			Func m_cb;
		};

		functoid_builder(functoid_builder const &) = delete;
		functoid_builder &operator=(functoid_builder const &) = delete;
		functoid_builder &operator=(functoid_builder &&that) = delete;

		Func m_cb;

	public:
		using input_t = Input;

		template <typename T>
		functoid_builder(devcb_write &target, bool append, T &&cb)
			: builder_base(target, append)
			, transform_base<std::make_unsigned_t<Input>, functoid_builder>(DefaultMask)
			, m_cb(std::forward<T>(cb))
		{ }
		functoid_builder(functoid_builder &&that)
			: builder_base(std::move(that))
			, transform_base<std::make_unsigned_t<Input>, functoid_builder>(std::move(that))
			, m_cb(std::move(that.m_cb))
		{
			that.consume();
			that.built();
		}
		~functoid_builder() { this->template register_creator<functoid_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), this->mask(), DefaultMask);
		}

		void validity_check(validity_checker &valid) const { }

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			return
					[cb = std::move(m_cb), exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ devcb_write::invoke_write<Input>(cb, offset, (data ^ exor) & mask, mem_mask & mask); };
		}
	};

	template <typename Delegate>
	class delegate_builder : public builder_base, public transform_base<mask_t<Input, typename delegate_traits<Delegate>::input_t>, delegate_builder<Delegate> >
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = intermediate_t<Input, typename delegate_traits<Delegate>::input_t>;

			wrapped_builder(delegate_builder &&that)
				: builder_base(std::move(that))
				, m_delegate(std::move(that.m_delegate))
			{
				that.consume();
				that.built();
			}
			wrapped_builder(wrapped_builder &&that)
				: builder_base(std::move(that))
				, m_delegate(std::move(that.m_delegate))
			{
				that.consume();
				that.built();
			}

			void validity_check(validity_checker &valid) const
			{
				auto const target(m_delegate.finder_target());
				if (target.second && !target.first.subdevice(target.second))
					osd_printf_error("Write callback bound to non-existent object tag %s (%s)\n", target.first.subtag(target.second), m_delegate.name());
			}

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				m_delegate.resolve();
				return
						[cb = std::move(m_delegate)] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ devcb_write::invoke_write<Input>(cb, offset, data, mem_mask); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			Delegate m_delegate;
		};

		friend class wrapped_builder; // workaround for MSVC

		delegate_builder(delegate_builder const &) = delete;
		delegate_builder &operator=(delegate_builder const &) = delete;
		delegate_builder &operator=(delegate_builder &&that) = delete;

		Delegate m_delegate;

	public:
		using input_t = intermediate_t<Input, typename delegate_traits<Delegate>::input_t>;

		template <typename T>
		delegate_builder(devcb_write &target, bool append, device_t &devbase, char const *tag, T &&func, char const *name)
			: builder_base(target, append)
			, transform_base<mask_t<Input, typename delegate_traits<Delegate>::input_t>, delegate_builder>(DefaultMask & delegate_traits<Delegate>::default_mask)
			, m_delegate(devbase, tag, std::forward<T>(func), name)
		{ }
		template <typename T>
		delegate_builder(devcb_write &target, bool append, device_t &devbase, devcb_write::delegate_device_class_t<T> &obj, T &&func, char const *name)
			: builder_base(target, append)
			, transform_base<mask_t<Input, typename delegate_traits<Delegate>::input_t>, delegate_builder>(DefaultMask & delegate_traits<Delegate>::default_mask)
			, m_delegate(obj, std::forward<T>(func), name)
		{ }
		delegate_builder(delegate_builder &&that)
			: builder_base(std::move(that))
			, transform_base<mask_t<Input, typename delegate_traits<Delegate>::input_t>, delegate_builder>(std::move(that))
			, m_delegate(std::move(that.m_delegate))
		{
			that.consume();
			that.built();
		}
		~delegate_builder() { this->template register_creator<delegate_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			std::make_unsigned_t<Input> const in_mask(this->inherited_mask() ? DefaultMask : this->mask());
			mask_t<Input, typename delegate_traits<Delegate>::input_t> const out_mask(DefaultMask & delegate_traits<Delegate>::default_mask);
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), in_mask, out_mask);
		}

		void validity_check(validity_checker &valid) const
		{
			auto const target(m_delegate.finder_target());
			if (target.second && !target.first.subdevice(target.second))
				osd_printf_error("Write callback bound to non-existent object tag %s (%s)\n", target.first.subtag(target.second), m_delegate.name());
		}

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			m_delegate.resolve();
			return
					[cb = std::move(m_delegate), exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ devcb_write::invoke_write<Input>(cb, offset, (data ^ exor) & mask, mem_mask & mask); };
		}
	};

	class inputline_builder : public builder_base, public transform_base<mask_t<Input, int>, inputline_builder>
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = intermediate_t<Input, int>;

			wrapped_builder(inputline_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(that.m_tag)
				, m_exec(that.m_exec)
				, m_linenum(that.m_linenum)
			{
				that.consume();
				that.built();
			}
			wrapped_builder(wrapped_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(that.m_tag)
				, m_exec(that.m_exec)
				, m_linenum(that.m_linenum)
			{
				that.consume();
				that.built();
			}

			void validity_check(validity_checker &valid) const
			{
				if (!m_exec)
				{
					device_t *const device(m_devbase.subdevice(m_tag));
					if (!device)
						osd_printf_error("Write callback bound to non-existent object tag %s\n", m_tag);
					else if (!dynamic_cast<device_execute_interface *>(device))
						osd_printf_error("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
				}
			}

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				if (!m_exec)
				{
					device_t *const device(m_devbase.subdevice(m_tag));
					if (!device)
						throw emu_fatalerror("Write callback bound to non-existent object tag %s\n", m_tag);
					m_exec = dynamic_cast<device_execute_interface *>(device);
					if (!m_exec)
						throw emu_fatalerror("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
				}
				return
						[&exec = *m_exec, linenum = m_linenum] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ exec.set_input_line(linenum, data); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			device_t &m_devbase;
			char const *const m_tag;
			device_execute_interface *m_exec;
			int const m_linenum;
		};

		friend class wrapped_builder; // workaround for MSVC

		inputline_builder(inputline_builder const &) = delete;
		inputline_builder &operator=(inputline_builder const &) = delete;
		inputline_builder &operator=(inputline_builder &&that) = delete;

		device_t &m_devbase;
		char const *const m_tag;
		device_execute_interface *m_exec;
		int const m_linenum;

	public:
		using input_t = intermediate_t<Input, int>;

		inputline_builder(devcb_write &target, bool append, device_t &devbase, char const *tag, int linenum)
			: builder_base(target, append)
			, transform_base<mask_t<Input, int>, inputline_builder>(1U)
			, m_devbase(devbase)
			, m_tag(tag)
			, m_exec(nullptr)
			, m_linenum(linenum)
		{ }
		inputline_builder(devcb_write &target, bool append, device_execute_interface &exec, int linenum)
			: builder_base(target, append)
			, transform_base<mask_t<Input, int>, inputline_builder>(1U)
			, m_devbase(exec.device())
			, m_tag(exec.device().tag())
			, m_exec(&exec)
			, m_linenum(linenum)
		{ }
		inputline_builder(inputline_builder &&that)
			: builder_base(std::move(that))
			, transform_base<mask_t<Input, int>, inputline_builder>(std::move(that))
			, m_devbase(that.m_devbase)
			, m_tag(that.m_tag)
			, m_exec(that.m_exec)
			, m_linenum(that.m_linenum)
		{
			that.consume();
			that.built();
		}
		~inputline_builder() { this->template register_creator<inputline_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			std::make_unsigned_t<Input> const in_mask(this->inherited_mask() ? DefaultMask : this->mask());
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), in_mask, 1U);
		}

		void validity_check(validity_checker &valid) const
		{
			if (!m_exec)
			{
				device_t *const device(m_devbase.subdevice(m_tag));
				if (!device)
					osd_printf_error("Write callback bound to non-existent object tag %s\n", m_tag);
				else if (!dynamic_cast<device_execute_interface *>(device))
					osd_printf_error("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
			}
		}

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			if (!m_exec)
			{
				device_t *const device(m_devbase.subdevice(m_tag));
				if (!device)
					throw emu_fatalerror("Write callback bound to non-existent object tag %s\n", m_tag);
				m_exec = dynamic_cast<device_execute_interface *>(device);
				if (!m_exec)
					throw emu_fatalerror("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
			}
			return
					[&exec = *m_exec, linenum = m_linenum, exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ exec.set_input_line(linenum, (data ^ exor) & mask); };
		}
	};

	class latched_inputline_builder : public builder_base, public transform_base<std::make_unsigned_t<Input>, latched_inputline_builder>
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = Input;

			wrapped_builder(latched_inputline_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(that.m_tag)
				, m_exec(that.m_exec)
				, m_linenum(that.m_linenum)
				, m_value(that.m_value)
			{
				that.consume();
				that.built();
			}
			wrapped_builder(wrapped_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(that.m_tag)
				, m_exec(that.m_exec)
				, m_linenum(that.m_linenum)
				, m_value(that.m_value)
			{
				that.consume();
				that.built();
			}

			void validity_check(validity_checker &valid) const
			{
				if (!m_exec)
				{
					device_t *const device(m_devbase.subdevice(m_tag));
					if (!device)
						osd_printf_error("Write callback bound to non-existent object tag %s\n", m_tag);
					else if (!dynamic_cast<device_execute_interface *>(device))
						osd_printf_error("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
				}
			}

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				if (!m_exec)
				{
					device_t *const device(m_devbase.subdevice(m_tag));
					if (!device)
						throw emu_fatalerror("Write callback bound to non-existent object tag %s\n", m_tag);
					m_exec = dynamic_cast<device_execute_interface *>(device);
					if (!m_exec)
						throw emu_fatalerror("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
				}
				return
						[&exec = *m_exec, linenum = m_linenum, value = m_value] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ if (data) exec.set_input_line(linenum, value); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			device_t &m_devbase;
			char const *const m_tag;
			device_execute_interface *m_exec;
			int const m_linenum;
			int const m_value;
		};

		friend class wrapped_builder; // workaround for MSVC

		latched_inputline_builder(latched_inputline_builder const &) = delete;
		latched_inputline_builder &operator=(latched_inputline_builder const &) = delete;
		latched_inputline_builder &operator=(latched_inputline_builder &&that) = delete;

		device_t &m_devbase;
		char const *const m_tag;
		device_execute_interface *m_exec;
		int const m_linenum;
		int const m_value;

	public:
		using input_t = Input;

		latched_inputline_builder(devcb_write &target, bool append, device_t &devbase, char const *tag, int linenum, int value)
			: builder_base(target, append)
			, transform_base<std::make_unsigned_t<Input>, latched_inputline_builder>(DefaultMask)
			, m_devbase(devbase)
			, m_tag(tag)
			, m_exec(nullptr)
			, m_linenum(linenum)
			, m_value(value)
		{ }
		latched_inputline_builder(devcb_write &target, bool append, device_execute_interface &exec, int linenum, int value)
			: builder_base(target, append)
			, transform_base<std::make_unsigned_t<Input>, latched_inputline_builder>(DefaultMask)
			, m_devbase(exec.device())
			, m_tag(exec.device().tag())
			, m_exec(&exec)
			, m_linenum(linenum)
			, m_value(value)
		{ }
		latched_inputline_builder(latched_inputline_builder &&that)
			: builder_base(std::move(that))
			, transform_base<std::make_unsigned_t<Input>, latched_inputline_builder>(std::move(that))
			, m_devbase(that.m_devbase)
			, m_tag(that.m_tag)
			, m_exec(that.m_exec)
			, m_linenum(that.m_linenum)
			, m_value(that.m_value)
		{
			that.consume();
			that.built();
		}
		~latched_inputline_builder() { this->template register_creator<latched_inputline_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), this->mask(), DefaultMask);
		}

		void validity_check(validity_checker &valid) const
		{
			if (!m_exec)
			{
				device_t *const device(m_devbase.subdevice(m_tag));
				if (!device)
					osd_printf_error("Write callback bound to non-existent object tag %s\n", m_tag);
				else if (!dynamic_cast<device_execute_interface *>(device))
					osd_printf_error("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
			}
		}

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			if (!m_exec)
			{
				device_t *const device(m_devbase.subdevice(m_tag));
				if (!device)
					throw emu_fatalerror("Write callback bound to non-existent object tag %s\n", m_tag);
				m_exec = dynamic_cast<device_execute_interface *>(device);
				if (!m_exec)
					throw emu_fatalerror("Write callback bound to device %s (%s) that does not implement device_execute_interface\n", device->tag(), device->name());
			}
			return
					[&exec = *m_exec, linenum = m_linenum, value = m_value, exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ if ((data ^ exor) & mask) exec.set_input_line(linenum, value); };
		}
	};

	class ioport_builder : public builder_base, public transform_base<mask_t<Input, ioport_value>, ioport_builder>
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = intermediate_t<Input, ioport_value>;

			wrapped_builder(ioport_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(std::move(that.m_tag))
			{
				that.consume();
				that.built();
			}
			wrapped_builder(wrapped_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(std::move(that.m_tag))
			{
				that.consume();
				that.built();
			}

			void validity_check(validity_checker &valid) const { }

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				ioport_port *const ioport(m_devbase.ioport(m_tag));
				if (!ioport)
					throw emu_fatalerror("Write callback bound to non-existent I/O port %s of device %s (%s)\n", m_tag, m_devbase.tag(), m_devbase.name());
				return
						[&port = *ioport] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ port.write(data); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			device_t &m_devbase;
			std::string m_tag;
		};

		friend class wrapped_builder; // workaround for MSVC

		ioport_builder(ioport_builder const &) = delete;
		ioport_builder &operator=(ioport_builder const &) = delete;
		ioport_builder &operator=(ioport_builder &&that) = delete;

		device_t &m_devbase;
		std::string m_tag;

	public:
		using input_t = intermediate_t<Input, ioport_value>;

		ioport_builder(devcb_write &target, bool append, device_t &devbase, std::string &&tag)
			: builder_base(target, append)
			, transform_base<mask_t<Input, ioport_value>, ioport_builder>(DefaultMask)
			, m_devbase(devbase)
			, m_tag(std::move(tag))
		{ }
		ioport_builder(ioport_builder &&that)
			: builder_base(std::move(that))
			, transform_base<mask_t<Input, ioport_value>, ioport_builder>(std::move(that))
			, m_devbase(that.m_devbase)
			, m_tag(std::move(that.m_tag))
		{
			that.consume();
			that.built();
		}
		~ioport_builder() { this->template register_creator<ioport_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), this->mask(), DefaultMask);
		}

		void validity_check(validity_checker &valid) const { }

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			ioport_port *const ioport(m_devbase.ioport(m_tag));
			if (!ioport)
				throw emu_fatalerror("Write callback bound to non-existent I/O port %s of device %s (%s)\n", m_tag, m_devbase.tag(), m_devbase.name());
			return
					[&port = *ioport, exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ port.write((data ^ exor) & mask); };
		}
	};

	class membank_builder : public builder_base, public transform_base<mask_t<Input, int>, membank_builder>
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = intermediate_t<Input, int>;

			wrapped_builder(membank_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(std::move(that.m_tag))
			{
				that.consume();
				that.built();
			}
			wrapped_builder(wrapped_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(std::move(that.m_tag))
			{
				that.consume();
				that.built();
			}

			void validity_check(validity_checker &valid) const { }

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				memory_bank *const bank(m_devbase.membank(m_tag));
				if (!bank)
					throw emu_fatalerror("Write callback bound to non-existent memory bank %s of device %s (%s)\n", m_tag, m_devbase.tag(), m_devbase.name());
				return
						[&membank = *bank] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ membank.set_entry(data); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			device_t &m_devbase;
			std::string m_tag;
		};

		friend class wrapped_builder; // workaround for MSVC

		membank_builder(membank_builder const &) = delete;
		membank_builder &operator=(membank_builder const &) = delete;
		membank_builder &operator=(membank_builder &&that) = delete;

		device_t &m_devbase;
		std::string m_tag;

	public:
		using input_t = intermediate_t<Input, int>;

		membank_builder(devcb_write &target, bool append, device_t &devbase, std::string &&tag)
			: builder_base(target, append)
			, transform_base<mask_t<Input, int>, membank_builder>(DefaultMask)
			, m_devbase(devbase)
			, m_tag(std::move(tag))
		{ }
		membank_builder(membank_builder &&that)
			: builder_base(std::move(that))
			, transform_base<mask_t<Input, int>, membank_builder>(std::move(that))
			, m_devbase(that.m_devbase)
			, m_tag(std::move(that.m_tag))
		{
			that.consume();
			that.built();
		}
		~membank_builder() { this->template register_creator<membank_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), this->mask(), DefaultMask);
		}

		void validity_check(validity_checker &valid) const { }

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			memory_bank *const bank(m_devbase.membank(m_tag));
			if (!bank)
				throw emu_fatalerror("Write callback bound to non-existent memory bank %s of device %s (%s)\n", m_tag, m_devbase.tag(), m_devbase.name());
			return
					[&membank = *bank, exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ membank.set_entry((data ^ exor) & mask); };
		}
	};

	class output_builder : public builder_base, public transform_base<mask_t<Input, s32>, output_builder>
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = intermediate_t<Input, s32>;

			wrapped_builder(output_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(std::move(that.m_tag))
			{
				that.consume();
				that.built();
			}
			wrapped_builder(wrapped_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_tag(std::move(that.m_tag))
			{
				that.consume();
				that.built();
			}

			void validity_check(validity_checker &valid) const { }

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				return
						[&item = m_devbase.machine().output().find_or_create_item(m_tag, 0)] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ item.set(data); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			device_t &m_devbase;
			std::string m_tag;
		};

		friend class wrapped_builder; // workaround for MSVC

		output_builder(output_builder const &) = delete;
		output_builder &operator=(output_builder const &) = delete;
		output_builder &operator=(output_builder &&that) = delete;

		device_t &m_devbase;
		std::string m_tag;

	public:
		using input_t = intermediate_t<Input, s32>;

		output_builder(devcb_write &target, bool append, device_t &devbase, std::string &&tag)
			: builder_base(target, append)
			, transform_base<mask_t<Input, s32>, output_builder>(DefaultMask)
			, m_devbase(devbase)
			, m_tag(std::move(tag))
		{ }
		output_builder(output_builder &&that)
			: builder_base(std::move(that))
			, transform_base<mask_t<Input, s32>, output_builder>(std::move(that))
			, m_devbase(that.m_devbase)
			, m_tag(std::move(that.m_tag))
		{
			that.consume();
			that.built();
		}
		~output_builder() { this->template register_creator<output_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), this->mask(), DefaultMask);
		}

		void validity_check(validity_checker &valid) const { }

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			return
					[&item = m_devbase.machine().output().find_or_create_item(m_tag, 0), exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ item.set((data ^ exor) & mask); };
		}
	};

	class log_builder : public builder_base, public transform_base<std::make_unsigned_t<Input>, log_builder>
	{
	private:
		class wrapped_builder : public builder_base
		{
		public:
			template <typename T, typename U> friend class first_transform_builder;

			using input_t = Input;

			wrapped_builder(log_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_message(std::move(that.m_message))
			{
				that.consume();
				that.built();
			}
			wrapped_builder(wrapped_builder &&that)
				: builder_base(std::move(that))
				, m_devbase(that.m_devbase)
				, m_message(std::move(that.m_message))
			{
				that.consume();
				that.built();
			}

			void validity_check(validity_checker &valid) const { }

			auto build()
			{
				assert(this->m_consumed);
				this->built();
				return
						[&devbase = m_devbase, message = std::move(m_message)] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
						{ if (data) devbase.logerror("%s: %s\n", devbase.machine().describe_context(), message); };
			}

		private:
			wrapped_builder(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder const &) = delete;
			wrapped_builder operator=(wrapped_builder &&that) = delete;

			device_t &m_devbase;
			std::string m_message;
		};

		friend class wrapped_builder; // workaround for MSVC

		log_builder(log_builder const &) = delete;
		log_builder &operator=(log_builder const &) = delete;
		log_builder &operator=(log_builder &&that) = delete;

		device_t &m_devbase;
		std::string m_message;

	public:
		using input_t = Input;

		log_builder(devcb_write &target, bool append, device_t &devbase, std::string &&message)
			: builder_base(target, append)
			, transform_base<std::make_unsigned_t<Input>, log_builder>(DefaultMask)
			, m_devbase(devbase)
			, m_message(std::move(message))
		{ }
		log_builder(log_builder &&that)
			: builder_base(std::move(that))
			, transform_base<std::make_unsigned_t<Input>, log_builder>(std::move(that))
			, m_devbase(that.m_devbase)
			, m_message(std::move(that.m_message))
		{
			that.consume();
			that.built();
		}
		~log_builder() { this->template register_creator<log_builder>(); }

		template <typename T>
		std::enable_if_t<is_transform<input_t, input_t, T>::value, first_transform_builder<wrapped_builder, std::remove_reference_t<T> > > transform(T &&cb)
		{
			return first_transform_builder<wrapped_builder, std::remove_reference_t<T> >(this->m_target, this->m_append, wrapped_builder(std::move(*this)), std::forward<T>(cb), this->exor(), this->mask(), DefaultMask);
		}

		void validity_check(validity_checker &valid) const { }

		auto build()
		{
			assert(this->m_consumed);
			this->built();
			return
					[&devbase = m_devbase, message = std::move(m_message), exor = this->exor(), mask = this->mask()] (offs_t offset, input_t data, std::make_unsigned_t<input_t> mem_mask)
					{ if ((data ^ exor) & mask) devbase.logerror("%s: %s\n", devbase.machine().describe_context(), message); };
		}
	};
	class binder
	{
	public:
		binder(devcb_write &target) : m_target(target) { }
		binder(binder const &) = delete;
		binder(binder &&that) : m_target(that.m_target), m_append(that.m_append), m_used(that.m_used) { that.m_used = true; }
		binder &operator=(binder const &) = delete;
		binder &operator=(binder &&) = delete;

		template <typename T>
		std::enable_if_t<is_write<Input, T>::value, functoid_builder<std::remove_reference_t<T> > > set(T &&cb)
		{
			set_used();
			return functoid_builder<std::remove_reference_t<T> >(m_target, m_append, std::forward<T>(cb));
		}

		template <typename T>
		std::enable_if_t<is_write_method<T>::value, delegate_builder<delegate_type_t<T> > > set(T &&func, char const *name)
		{
			set_used();
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, m_target.owner().mconfig().current_device(), DEVICE_SELF, std::forward<T>(func), name);
		}

		template <typename T>
		std::enable_if_t<is_write_method<T>::value, delegate_builder<delegate_type_t<T> > > set(char const *tag, T &&func, char const *name)
		{
			set_used();
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, m_target.owner().mconfig().current_device(), tag, std::forward<T>(func), name);
		}

		template <typename T, typename U>
		std::enable_if_t<is_write_method<T>::value, delegate_builder<delegate_type_t<T> > > set(U &obj, T &&func, char const *name)
		{
			set_used();
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, m_target.owner(), devcb_write::cast_reference<delegate_device_class_t<T> >(obj), std::forward<T>(func), name);
		}

		template <typename T, typename U, bool R>
		std::enable_if_t<is_write_method<T>::value, delegate_builder<delegate_type_t<T> > > set(device_finder<U, R> &finder, T &&func, char const *name)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, target.first, target.second, std::forward<T>(func), name);
		}

		template <typename T, typename U, bool R>
		std::enable_if_t<is_write_method<T>::value, delegate_builder<delegate_type_t<T> > > set(device_finder<U, R> const &finder, T &&func, char const *name)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return delegate_builder<delegate_type_t<T> >(m_target, m_append, target.first, target.second, std::forward<T>(func), name);
		}

		template <typename... Params>
		auto append(Params &&... args)
		{
			m_append = true;
			return set(std::forward<Params>(args)...);
		}

		inputline_builder set_inputline(char const *tag, int linenum)
		{
			set_used();
			return inputline_builder(m_target, m_append, m_target.owner().mconfig().current_device(), tag, linenum);
		}

		latched_inputline_builder set_inputline(char const *tag, int linenum, int value)
		{
			set_used();
			return latched_inputline_builder(m_target, m_append, m_target.owner().mconfig().current_device(), tag, linenum, value);
		}

		inputline_builder set_inputline(device_execute_interface &obj, int linenum)
		{
			set_used();
			return inputline_builder(m_target, m_append, obj, linenum);
		}

		latched_inputline_builder set_inputline(device_execute_interface &obj, int linenum, int value)
		{
			set_used();
			return latched_inputline_builder(m_target, m_append, obj, linenum, value);
		}

		template <typename T, bool R>
		inputline_builder set_inputline(device_finder<T, R> const &finder, int linenum)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return inputline_builder(m_target, m_append, target.first, target.second, linenum);
		}

		template <typename T, bool R>
		latched_inputline_builder set_inputline(device_finder<T, R> const &finder, int linenum, int value)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return latched_inputline_builder(m_target, m_append, target.first, target.second, linenum, value);
		}

		template <typename... Params>
		auto append_inputline(Params &&... args)
		{
			m_append = true;
			return set_inputline(std::forward<Params>(args)...);
		}

		template <typename... Params>
		ioport_builder set_ioport(Params &&... args)
		{
			set_used();
			return ioport_builder(m_target, m_append, m_target.owner().mconfig().current_device(), std::string(std::forward<Params>(args)...));
		}

		template <bool R>
		ioport_builder set_ioport(ioport_finder<R> &finder)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return ioport_builder(m_target, m_append, target.first, std::string(target.second));
		}

		template <bool R>
		ioport_builder set_ioport(ioport_finder<R> const &finder)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return ioport_builder(m_target, m_append, target.first, std::string(target.second));
		}

		template <typename... Params>
		ioport_builder append_ioport(Params &&... args)
		{
			m_append = true;
			return set_ioport(std::forward<Params>(args)...);
		}

		template <typename... Params>
		membank_builder set_membank(Params &&... args)
		{
			set_used();
			return membank_builder(m_target, m_append, m_target.owner().mconfig().current_device(), std::string(std::forward<Params>(args)...));
		}

		template <bool R>
		membank_builder set_membank(memory_bank_finder<R> &finder)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return membank_builder(m_target, m_append, target.first, std::string(target.second));
		}

		template <bool R>
		membank_builder set_membank(memory_bank_finder<R> const &finder)
		{
			set_used();
			std::pair<device_t &, char const *> const target(finder.finder_target());
			return membank_builder(m_target, m_append, target.first, std::string(target.second));
		}

		template <typename... Params>
		membank_builder append_membank(Params &&... args)
		{
			m_append = true;
			return set_membank(std::forward<Params>(args)...);
		}

		template <typename... Params>
		output_builder set_output(Params &&... args)
		{
			set_used();
			return output_builder(m_target, m_append, m_target.owner().mconfig().current_device(), std::string(std::forward<Params>(args)...));
		}

		template <typename... Params>
		output_builder append_output(Params &&... args)
		{
			m_append = true;
			return set_output(std::forward<Params>(args)...);
		}

		template <typename... Params>
		log_builder set_log(device_t &devbase, Params &&... args)
		{
			set_used();
			return log_builder(m_target, m_append, devbase, std::string(std::forward<Params>(args)...));
		}

		template <typename T, typename... Params>
		std::enable_if_t<emu::detail::is_device_implementation<std::remove_reference_t<T> >::value, log_builder> set_log(T &devbase, Params &&... args)
		{
			return set_log(static_cast<device_t &>(devbase), std::forward<Params>(args)...);
		}

		template <typename T, typename... Params>
		std::enable_if_t<emu::detail::is_device_interface<std::remove_reference_t<T> >::value, log_builder> set_log(T &devbase, Params &&... args)
		{
			return set_log(devbase.device(), std::forward<Params>(args)...);
		}

		template <typename... Params>
		log_builder set_log(Params &&... args)
		{
			return set_log(m_target.owner().mconfig().current_device(), std::forward<Params>(args)...);
		}

		template <typename... Params>
		log_builder append_log(Params &&... args)
		{
			m_append = true;
			return set_log(std::forward<Params>(args)...);
		}

		void set_nop()
		{
			set_used();
			m_target.m_creators.clear();
			m_target.m_creators.emplace_back(std::make_unique<nop_creator>());
		}

	private:
		void set_used() { assert(!m_used); m_used = true; }

		devcb_write &m_target;
		bool m_append = false;
		bool m_used = false;
	};

	std::vector<func_t> m_functions;
	std::vector<typename creator::ptr> m_creators;

public:
	template <unsigned Count>
	class array : public devcb_write_base::array<devcb_write<Input, DefaultMask>, Count>
	{
	public:
		using devcb_write_base::array<devcb_write<Input, DefaultMask>, Count>::array;

		void resolve_all_safe()
		{
			for (devcb_write<Input, DefaultMask> &elem : *this)
				elem.resolve_safe();
		}
	};

	devcb_write(device_t &owner);

	binder bind();
	void reset();

	virtual void validity_check(validity_checker &valid) const override;

	void resolve();
	void resolve_safe();

	void operator()(offs_t offset, Input data, std::make_unsigned_t<Input> mem_mask = DefaultMask);
	void operator()(Input data);

	bool isnull() const { return m_functions.empty() && m_creators.empty(); }
	explicit operator bool() const { return !m_functions.empty(); }
};

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
devcb_write<Input, DefaultMask>::devcb_write(device_t &owner)
	: devcb_write_base(owner)
{
}

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
typename devcb_write<Input, DefaultMask>::binder devcb_write<Input, DefaultMask>::bind()
{
	return binder(*this);
}

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
void devcb_write<Input, DefaultMask>::reset()
{
	assert(m_functions.empty());
	m_creators.clear();
}

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
void devcb_write<Input, DefaultMask>::validity_check(validity_checker &valid) const
{
	assert(m_functions.empty());
	for (typename creator::ptr const &c : m_creators)
		c->validity_check(valid);
}

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
void devcb_write<Input, DefaultMask>::resolve()
{
	assert(m_functions.empty());
	m_functions.reserve(m_creators.size());
	for (typename creator::ptr const &c : m_creators)
		m_functions.emplace_back(c->create());
	m_creators.clear();
}

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
void devcb_write<Input, DefaultMask>::resolve_safe()
{
	resolve();
	if (m_functions.empty())
		m_functions.emplace_back([] (offs_t offset, Input data, std::make_unsigned_t<Input> mem_mask) { });
}

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
void devcb_write<Input, DefaultMask>::operator()(offs_t offset, Input data, std::make_unsigned_t<Input> mem_mask)
{
	assert(m_creators.empty() && !m_functions.empty());
	typename std::vector<func_t>::const_iterator it(m_functions.begin());
	(*it)(offset, data, mem_mask);
	while (m_functions.end() != ++it)
		(*it)(offset, data, mem_mask);
}

template <typename Input, std::make_unsigned_t<Input> DefaultMask>
void devcb_write<Input, DefaultMask>::operator()(Input data)
{
	this->operator()(0U, data, DefaultMask);
}

using devcb_read8 = devcb_read<u8>;
using devcb_read16 = devcb_read<u16>;
using devcb_read32 = devcb_read<u32>;
using devcb_read64 = devcb_read<u64>;
using devcb_read_line = devcb_read<int, 1U>;

using devcb_write8 = devcb_write<u8>;
using devcb_write16 = devcb_write<u16>;
using devcb_write32 = devcb_write<u32>;
using devcb_write64 = devcb_write<u64>;
using devcb_write_line = devcb_write<int, 1U>;


//**************************************************************************
//  TEMPLATE INSTANTIATIONS
//**************************************************************************

extern template class devcb_read<u8>;
extern template class devcb_read<u16>;
extern template class devcb_read<u32>;
extern template class devcb_read<u64>;
extern template class devcb_read<int, 1U>;

extern template class devcb_read8::delegate_builder<read8s_delegate>;
extern template class devcb_read8::delegate_builder<read16s_delegate>;
extern template class devcb_read8::delegate_builder<read32s_delegate>;
extern template class devcb_read8::delegate_builder<read64s_delegate>;
extern template class devcb_read8::delegate_builder<read8sm_delegate>;
extern template class devcb_read8::delegate_builder<read16sm_delegate>;
extern template class devcb_read8::delegate_builder<read32sm_delegate>;
extern template class devcb_read8::delegate_builder<read64sm_delegate>;
extern template class devcb_read8::delegate_builder<read8smo_delegate>;
extern template class devcb_read8::delegate_builder<read16smo_delegate>;
extern template class devcb_read8::delegate_builder<read32smo_delegate>;
extern template class devcb_read8::delegate_builder<read64smo_delegate>;
extern template class devcb_read8::delegate_builder<read_line_delegate>;

extern template class devcb_read16::delegate_builder<read8s_delegate>;
extern template class devcb_read16::delegate_builder<read16s_delegate>;
extern template class devcb_read16::delegate_builder<read32s_delegate>;
extern template class devcb_read16::delegate_builder<read64s_delegate>;
extern template class devcb_read16::delegate_builder<read8sm_delegate>;
extern template class devcb_read16::delegate_builder<read16sm_delegate>;
extern template class devcb_read16::delegate_builder<read32sm_delegate>;
extern template class devcb_read16::delegate_builder<read64sm_delegate>;
extern template class devcb_read16::delegate_builder<read8smo_delegate>;
extern template class devcb_read16::delegate_builder<read16smo_delegate>;
extern template class devcb_read16::delegate_builder<read32smo_delegate>;
extern template class devcb_read16::delegate_builder<read64smo_delegate>;
extern template class devcb_read16::delegate_builder<read_line_delegate>;

extern template class devcb_read32::delegate_builder<read8s_delegate>;
extern template class devcb_read32::delegate_builder<read16s_delegate>;
extern template class devcb_read32::delegate_builder<read32s_delegate>;
extern template class devcb_read32::delegate_builder<read64s_delegate>;
extern template class devcb_read32::delegate_builder<read8sm_delegate>;
extern template class devcb_read32::delegate_builder<read16sm_delegate>;
extern template class devcb_read32::delegate_builder<read32sm_delegate>;
extern template class devcb_read32::delegate_builder<read64sm_delegate>;
extern template class devcb_read32::delegate_builder<read8smo_delegate>;
extern template class devcb_read32::delegate_builder<read16smo_delegate>;
extern template class devcb_read32::delegate_builder<read32smo_delegate>;
extern template class devcb_read32::delegate_builder<read64smo_delegate>;
extern template class devcb_read32::delegate_builder<read_line_delegate>;

extern template class devcb_read64::delegate_builder<read8s_delegate>;
extern template class devcb_read64::delegate_builder<read16s_delegate>;
extern template class devcb_read64::delegate_builder<read32s_delegate>;
extern template class devcb_read64::delegate_builder<read64s_delegate>;
extern template class devcb_read64::delegate_builder<read8sm_delegate>;
extern template class devcb_read64::delegate_builder<read16sm_delegate>;
extern template class devcb_read64::delegate_builder<read32sm_delegate>;
extern template class devcb_read64::delegate_builder<read64sm_delegate>;
extern template class devcb_read64::delegate_builder<read8smo_delegate>;
extern template class devcb_read64::delegate_builder<read16smo_delegate>;
extern template class devcb_read64::delegate_builder<read32smo_delegate>;
extern template class devcb_read64::delegate_builder<read64smo_delegate>;
extern template class devcb_read64::delegate_builder<read_line_delegate>;

extern template class devcb_read_line::delegate_builder<read8s_delegate>;
extern template class devcb_read_line::delegate_builder<read16s_delegate>;
extern template class devcb_read_line::delegate_builder<read32s_delegate>;
extern template class devcb_read_line::delegate_builder<read64s_delegate>;
extern template class devcb_read_line::delegate_builder<read8sm_delegate>;
extern template class devcb_read_line::delegate_builder<read16sm_delegate>;
extern template class devcb_read_line::delegate_builder<read32sm_delegate>;
extern template class devcb_read_line::delegate_builder<read64sm_delegate>;
extern template class devcb_read_line::delegate_builder<read8smo_delegate>;
extern template class devcb_read_line::delegate_builder<read16smo_delegate>;
extern template class devcb_read_line::delegate_builder<read32smo_delegate>;
extern template class devcb_read_line::delegate_builder<read64smo_delegate>;
extern template class devcb_read_line::delegate_builder<read_line_delegate>;

extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read8s_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read16s_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read32s_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read64s_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read8sm_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read16sm_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read32sm_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read64sm_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read8smo_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read16smo_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read32smo_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read64smo_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::delegate_builder<read_line_delegate> >;
extern template class devcb_read8::creator_impl<devcb_read8::ioport_builder>;

extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read8s_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read16s_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read32s_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read64s_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read8sm_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read16sm_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read32sm_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read64sm_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read8smo_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read16smo_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read32smo_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read64smo_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::delegate_builder<read_line_delegate> >;
extern template class devcb_read16::creator_impl<devcb_read16::ioport_builder>;

extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read8s_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read16s_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read32s_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read64s_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read8sm_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read16sm_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read32sm_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read64sm_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read8smo_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read16smo_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read32smo_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read64smo_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::delegate_builder<read_line_delegate> >;
extern template class devcb_read32::creator_impl<devcb_read32::ioport_builder>;

extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read8s_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read16s_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read32s_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read64s_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read8sm_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read16sm_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read32sm_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read64sm_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read8smo_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read16smo_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read32smo_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read64smo_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::delegate_builder<read_line_delegate> >;
extern template class devcb_read64::creator_impl<devcb_read64::ioport_builder>;

extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read8s_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read16s_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read32s_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read64s_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read8sm_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read16sm_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read32sm_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read64sm_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read8smo_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read16smo_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read32smo_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read64smo_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::delegate_builder<read_line_delegate> >;
extern template class devcb_read_line::creator_impl<devcb_read_line::ioport_builder>;

extern template class devcb_write<u8>;
extern template class devcb_write<u16>;
extern template class devcb_write<u32>;
extern template class devcb_write<u64>;
extern template class devcb_write<int, 1U>;

extern template class devcb_write8::delegate_builder<write8s_delegate>;
extern template class devcb_write8::delegate_builder<write16s_delegate>;
extern template class devcb_write8::delegate_builder<write32s_delegate>;
extern template class devcb_write8::delegate_builder<write64s_delegate>;
extern template class devcb_write8::delegate_builder<write8sm_delegate>;
extern template class devcb_write8::delegate_builder<write16sm_delegate>;
extern template class devcb_write8::delegate_builder<write32sm_delegate>;
extern template class devcb_write8::delegate_builder<write64sm_delegate>;
extern template class devcb_write8::delegate_builder<write8smo_delegate>;
extern template class devcb_write8::delegate_builder<write16smo_delegate>;
extern template class devcb_write8::delegate_builder<write32smo_delegate>;
extern template class devcb_write8::delegate_builder<write64smo_delegate>;
extern template class devcb_write8::delegate_builder<write_line_delegate>;

extern template class devcb_write16::delegate_builder<write8s_delegate>;
extern template class devcb_write16::delegate_builder<write16s_delegate>;
extern template class devcb_write16::delegate_builder<write32s_delegate>;
extern template class devcb_write16::delegate_builder<write64s_delegate>;
extern template class devcb_write16::delegate_builder<write8sm_delegate>;
extern template class devcb_write16::delegate_builder<write16sm_delegate>;
extern template class devcb_write16::delegate_builder<write32sm_delegate>;
extern template class devcb_write16::delegate_builder<write64sm_delegate>;
extern template class devcb_write16::delegate_builder<write8smo_delegate>;
extern template class devcb_write16::delegate_builder<write16smo_delegate>;
extern template class devcb_write16::delegate_builder<write32smo_delegate>;
extern template class devcb_write16::delegate_builder<write64smo_delegate>;
extern template class devcb_write16::delegate_builder<write_line_delegate>;

extern template class devcb_write32::delegate_builder<write8s_delegate>;
extern template class devcb_write32::delegate_builder<write16s_delegate>;
extern template class devcb_write32::delegate_builder<write32s_delegate>;
extern template class devcb_write32::delegate_builder<write64s_delegate>;
extern template class devcb_write32::delegate_builder<write8sm_delegate>;
extern template class devcb_write32::delegate_builder<write16sm_delegate>;
extern template class devcb_write32::delegate_builder<write32sm_delegate>;
extern template class devcb_write32::delegate_builder<write64sm_delegate>;
extern template class devcb_write32::delegate_builder<write8smo_delegate>;
extern template class devcb_write32::delegate_builder<write16smo_delegate>;
extern template class devcb_write32::delegate_builder<write32smo_delegate>;
extern template class devcb_write32::delegate_builder<write64smo_delegate>;
extern template class devcb_write32::delegate_builder<write_line_delegate>;

extern template class devcb_write64::delegate_builder<write8s_delegate>;
extern template class devcb_write64::delegate_builder<write16s_delegate>;
extern template class devcb_write64::delegate_builder<write32s_delegate>;
extern template class devcb_write64::delegate_builder<write64s_delegate>;
extern template class devcb_write64::delegate_builder<write8sm_delegate>;
extern template class devcb_write64::delegate_builder<write16sm_delegate>;
extern template class devcb_write64::delegate_builder<write32sm_delegate>;
extern template class devcb_write64::delegate_builder<write64sm_delegate>;
extern template class devcb_write64::delegate_builder<write8smo_delegate>;
extern template class devcb_write64::delegate_builder<write16smo_delegate>;
extern template class devcb_write64::delegate_builder<write32smo_delegate>;
extern template class devcb_write64::delegate_builder<write64smo_delegate>;
extern template class devcb_write64::delegate_builder<write_line_delegate>;

extern template class devcb_write_line::delegate_builder<write8s_delegate>;
extern template class devcb_write_line::delegate_builder<write16s_delegate>;
extern template class devcb_write_line::delegate_builder<write32s_delegate>;
extern template class devcb_write_line::delegate_builder<write64s_delegate>;
extern template class devcb_write_line::delegate_builder<write8sm_delegate>;
extern template class devcb_write_line::delegate_builder<write16sm_delegate>;
extern template class devcb_write_line::delegate_builder<write32sm_delegate>;
extern template class devcb_write_line::delegate_builder<write64sm_delegate>;
extern template class devcb_write_line::delegate_builder<write8smo_delegate>;
extern template class devcb_write_line::delegate_builder<write16smo_delegate>;
extern template class devcb_write_line::delegate_builder<write32smo_delegate>;
extern template class devcb_write_line::delegate_builder<write64smo_delegate>;
extern template class devcb_write_line::delegate_builder<write_line_delegate>;

extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write8s_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write16s_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write32s_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write64s_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write8sm_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write16sm_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write32sm_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write64sm_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write8smo_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write16smo_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write32smo_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write64smo_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::delegate_builder<write_line_delegate> >;
extern template class devcb_write8::creator_impl<devcb_write8::inputline_builder>;
extern template class devcb_write8::creator_impl<devcb_write8::latched_inputline_builder>;
extern template class devcb_write8::creator_impl<devcb_write8::ioport_builder>;
extern template class devcb_write8::creator_impl<devcb_write8::membank_builder>;
extern template class devcb_write8::creator_impl<devcb_write8::output_builder>;
extern template class devcb_write8::creator_impl<devcb_write8::log_builder>;

extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write8s_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write16s_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write32s_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write64s_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write8sm_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write16sm_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write32sm_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write64sm_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write8smo_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write16smo_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write32smo_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write64smo_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::delegate_builder<write_line_delegate> >;
extern template class devcb_write16::creator_impl<devcb_write16::inputline_builder>;
extern template class devcb_write16::creator_impl<devcb_write16::latched_inputline_builder>;
extern template class devcb_write16::creator_impl<devcb_write16::ioport_builder>;
extern template class devcb_write16::creator_impl<devcb_write16::membank_builder>;
extern template class devcb_write16::creator_impl<devcb_write16::output_builder>;
extern template class devcb_write16::creator_impl<devcb_write16::log_builder>;

extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write8s_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write16s_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write32s_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write64s_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write8sm_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write16sm_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write32sm_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write64sm_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write8smo_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write16smo_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write32smo_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write64smo_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::delegate_builder<write_line_delegate> >;
extern template class devcb_write32::creator_impl<devcb_write32::inputline_builder>;
extern template class devcb_write32::creator_impl<devcb_write32::latched_inputline_builder>;
extern template class devcb_write32::creator_impl<devcb_write32::ioport_builder>;
extern template class devcb_write32::creator_impl<devcb_write32::membank_builder>;
extern template class devcb_write32::creator_impl<devcb_write32::output_builder>;
extern template class devcb_write32::creator_impl<devcb_write32::log_builder>;

extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write8s_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write16s_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write32s_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write64s_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write8sm_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write16sm_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write32sm_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write64sm_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write8smo_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write16smo_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write32smo_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write64smo_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::delegate_builder<write_line_delegate> >;
extern template class devcb_write64::creator_impl<devcb_write64::inputline_builder>;
extern template class devcb_write64::creator_impl<devcb_write64::latched_inputline_builder>;
extern template class devcb_write64::creator_impl<devcb_write64::ioport_builder>;
extern template class devcb_write64::creator_impl<devcb_write64::membank_builder>;
extern template class devcb_write64::creator_impl<devcb_write64::output_builder>;
extern template class devcb_write64::creator_impl<devcb_write64::log_builder>;

extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write8s_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write16s_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write32s_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write64s_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write8sm_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write16sm_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write32sm_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write64sm_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write8smo_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write16smo_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write32smo_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write64smo_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::delegate_builder<write_line_delegate> >;
extern template class devcb_write_line::creator_impl<devcb_write_line::inputline_builder>;
extern template class devcb_write_line::creator_impl<devcb_write_line::latched_inputline_builder>;
extern template class devcb_write_line::creator_impl<devcb_write_line::ioport_builder>;
extern template class devcb_write_line::creator_impl<devcb_write_line::membank_builder>;
extern template class devcb_write_line::creator_impl<devcb_write_line::output_builder>;
extern template class devcb_write_line::creator_impl<devcb_write_line::log_builder>;

#endif // MAME_EMU_DEVCB_H
