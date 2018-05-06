// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    devcb.h

    Device callback interface helpers.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DEVCB_H
#define MAME_EMU_DEVCB_H

#include <functional>
#include <type_traits>
#include <utility>


namespace emu { namespace detail {

template <typename Delegate> struct devcb_delegate_initialiser
{
	devcb_delegate_initialiser(char const *tag, Delegate &&delegate) : m_base(nullptr), m_delegate(std::move(delegate))
	{
	}
	template <typename T>
	devcb_delegate_initialiser(T &device, std::enable_if_t<is_device_implementation<T>::value, Delegate &&> delegate) : m_base(&device), m_delegate(std::move(delegate))
	{
	}
	template <typename T>
	devcb_delegate_initialiser(T &interface, std::enable_if_t<is_device_interface<T>::value, Delegate &&> delegate) : m_base(&interface.device()), m_delegate(std::move(delegate))
	{
	}
	template <typename DeviceClass, bool Required>
	devcb_delegate_initialiser(device_finder<DeviceClass, Required> const &finder, Delegate &&delegate) : m_base(&finder.finder_target().first), m_delegate(std::move(delegate))
	{
	}

	device_t *m_base;
	Delegate &&m_delegate;
};

inline char const *devcb_delegate_get_tag(char const *tag) { return tag; }
template <typename T>
inline std::enable_if_t<is_device_implementation<T>::value, char const *> devcb_delegate_get_tag(T &device) { return DEVICE_SELF; }
template <typename T>
inline std::enable_if_t<is_device_interface<T>::value, char const *> devcb_delegate_get_tag(T &interface) { return DEVICE_SELF; }
template <typename DeviceClass, bool Required>
inline char const *devcb_delegate_get_tag(device_finder<DeviceClass, Required> const &finder) { return finder.finder_tag(); }

template <typename DescType> struct devcb_tag_desc_creator
{
	static DescType create(char const *tag)
	{
		return DescType{ nullptr, tag };
	}
	static DescType create(device_t &device)
	{
		return DescType{ &device, DEVICE_SELF };
	}
	static DescType create(device_interface &interface)
	{
		return DescType{ &interface.device(), DEVICE_SELF };
	}
	template <typename DeviceClass, bool Required>
	static DescType create(device_finder<DeviceClass, Required> const &finder)
	{
		std::pair<device_t &, char const *> const target(finder.finder_target());
		return DescType{ &target.first, target.second };
	}
};

template <typename DescType> struct devcb_line_desc_creator
{
	static DescType create(char const *tag, int inputnum)
	{
		return DescType{ nullptr, tag, inputnum };
	}
	static DescType create(device_t &device, int inputnum)
	{
		return DescType{ &device, DEVICE_SELF, inputnum };
	}
	static DescType create(device_interface &interface, int inputnum)
	{
		return DescType{ &interface.device(), DEVICE_SELF, inputnum };
	}
	template <typename DeviceClass, bool Required>
	static DescType create(device_finder<DeviceClass, Required> const &finder, int inputnum)
	{
		std::pair<device_t &, char const *> const target(finder.finder_target());
		return DescType{ &target.first, target.second, inputnum };
	}
};

} } // namespace emu::detail


//**************************************************************************
//  MACROS
//**************************************************************************

// wrappers for read callbacks into any tagged device
#define DEVCB_READLINE(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<read_line_delegate>((tag), read_line_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_READ8(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<read8_delegate>((tag), read8_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_READ16(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<read16_delegate>((tag), read16_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_READ32(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<read32_delegate>((tag), read32_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_READ64(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<read64_delegate>((tag), read64_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))

// wrappers for write callbacks into any tagged device
#define DEVCB_WRITELINE(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<write_line_delegate>((tag), write_line_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_WRITE8(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<write8_delegate>((tag), write8_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_WRITE16(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<write16_delegate>((tag), write16_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_WRITE32(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<write32_delegate>((tag), write32_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))
#define DEVCB_WRITE64(tag, _class, _func) (emu::detail::devcb_delegate_initialiser<write64_delegate>((tag), write64_delegate(&_class::_func, #_class "::" #_func, emu::detail::devcb_delegate_get_tag(tag), (_class *)nullptr)))

// machine config helpers to add shift, mask, or address space configuration
#define MCFG_DEVCB_RSHIFT(_shift) devcb->set_rshift(_shift);
#define MCFG_DEVCB_MASK(_mask) devcb->set_mask(_mask);
#define MCFG_DEVCB_BIT(_bit) devcb->set_rshift(-(_bit)).set_mask(1ULL << (_bit));
#define MCFG_DEVCB_XOR(_xor) devcb->set_xor(_xor);
#define MCFG_DEVCB_INVERT devcb->set_xor(~u64(0));
#define MCFG_DEVCB_ADDRESS_SPACE(_device, _spacenum) devcb->set_space(_device, _spacenum);

// machine config helpers for chaining callbacks
#define MCFG_DEVCB_CHAIN_INPUT(_desc) devcb = &downcast<devcb_read_base &>(*devcb).chain_alloc().set_callback(DEVCB_##_desc);
#define MCFG_DEVCB_CHAIN_OUTPUT(_desc) devcb = &downcast<devcb_write_base &>(*devcb).chain_alloc().set_callback(DEVCB_##_desc);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// base delegate type for a read_line/write_line
typedef device_delegate<int ()> read_line_delegate;
typedef device_delegate<void (int)> write_line_delegate;


// ======================> devcb_base

class devcb_base
{
protected:
	// enumerate the types of callbacks
	enum callback_type
	{
		CALLBACK_NONE,
		CALLBACK_LINE,
		CALLBACK_8,
		CALLBACK_16,
		CALLBACK_32,
		CALLBACK_64,
		CALLBACK_IOPORT,
		CALLBACK_MEMBANK,
		CALLBACK_OUTPUT,
		CALLBACK_LOG,
		CALLBACK_CONSTANT,
		CALLBACK_INPUTLINE,
		CALLBACK_ASSERTLINE,
		CALLBACK_CLEARLINE,
		CALLBACK_HOLDLINE
	};

	// construction/destruction
	devcb_base(device_t &device, u64 defmask);
	virtual ~devcb_base();

	template <callback_type Type> struct tag_desc
	{
		device_t *m_base;
		char const *m_tag;
	};

	template <callback_type Type> struct line_desc
	{
		device_t *m_base;
		char const *m_tag;
		int m_inputnum;
	};

public:
	// getters
	bool isnull() const { return (m_type == CALLBACK_NONE); }

	// additional configuration
	devcb_base &set_space(const char *device, int space = 0) { m_space_tag = device; m_space_num = space; return *this; }
	devcb_base &set_rshift(int rshift) { m_rshift = rshift; return *this; }
	devcb_base &set_mask(u64 mask) { m_mask = mask; return *this; }
	devcb_base &set_xor(u64 xorval) { m_xor = xorval; return *this; }

	// construction helper classes
	struct null_desc
	{
	};

	using ioport_desc = tag_desc<CALLBACK_IOPORT>;
	using membank_desc = tag_desc<CALLBACK_MEMBANK>;
	using output_desc = tag_desc<CALLBACK_OUTPUT>;

	struct constant_desc
	{
		u64 m_value;
	};

	struct logger_desc
	{
		const char *m_string;
	};

	using inputline_desc = line_desc<CALLBACK_INPUTLINE>;
	using assertline_desc = line_desc<CALLBACK_ASSERTLINE>;
	using clearline_desc = line_desc<CALLBACK_CLEARLINE>;
	using holdline_desc = line_desc<CALLBACK_HOLDLINE>;

	// shared callback setters
	devcb_base &set_callback(null_desc null) { reset(CALLBACK_NONE); return *this; }
	template <callback_type Type> devcb_base &set_callback(tag_desc<Type> desc)
	{
		if (desc.m_base) reset(*desc.m_base, Type);
		else reset(Type);
		m_target_tag = desc.m_tag;
		return *this;
	}
	devcb_base &set_callback(constant_desc constant) { reset(CALLBACK_CONSTANT); m_target_int = constant.m_value; return *this; }
	devcb_base &set_callback(logger_desc logger) { reset(CALLBACK_LOG); m_target_tag = logger.m_string; return *this; }
	void reset() { reset(m_owner, CALLBACK_NONE); }

protected:
	// internal helpers
	inline u64 shift_mask(u64 value) const { return ((m_rshift < 0) ? (value << -m_rshift) : (value >> m_rshift)) & m_mask; }
	inline u64 shift_mask_xor(u64 value) const { return (((m_rshift < 0) ? (value << -m_rshift) : (value >> m_rshift)) ^ m_xor) & m_mask; }
	inline u64 unshift_mask(u64 value) const { return (m_rshift < 0) ? ((value & m_mask) >> -m_rshift) : ((value & m_mask) << m_rshift); }
	inline u64 unshift_mask_xor(u64 value) const { return (m_rshift < 0) ? (((value ^ m_xor) & m_mask) >> -m_rshift) : (((value ^ m_xor) & m_mask) << m_rshift); }
	void reset(callback_type type);
	void reset(device_t &base, callback_type type);
	virtual void devcb_reset() = 0;
	void resolve_ioport();
	void resolve_membank();
	void resolve_output();
	void resolve_inputline();
	void resolve_space();

	// the callback target is going to be one of these
	union callback_target
	{
		void *              ptr;
		device_t *          device;
		ioport_port *       ioport;
		memory_bank *       membank;
		output_manager::output_item *item;
	};

	// configuration
	device_t &		    m_owner;                // reference to our owning device
	std::reference_wrapper<device_t> m_base;    // device to resolve relative to
	callback_type       m_type;                 // type of callback registered
	const char *        m_target_tag;           // tag of target object
	u64                 m_target_int;           // integer value of target object
	const char *        m_space_tag;            // tag of address space device
	int                 m_space_num;            // address space number of space device

	// derived state
	address_space *     m_space;                // target address space
	callback_target     m_target;               // resolved pointer to target object
	int                 m_rshift;               // right shift to apply to data read
	u64                 m_mask;                 // mask to apply to data read
	const u64           m_defmask;              // default mask
	u64                 m_xor;                  // XOR to apply to data read
};


// ======================> devcb_read_base

class devcb_read_base : public devcb_base
{
protected:
	// construction/destruction
	devcb_read_base(device_t &device, u64 defmask, bool chained = false);

	template <typename Delegate, typename Dummy> struct set_helper
	{
		static constexpr bool valid = false;
	};
	template <typename Dummy> struct set_helper<read_line_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_LINE;
		static void apply(devcb_read_base &devcb, read_line_delegate &&delegate) { devcb.m_readline = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<read8_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_8;
		static void apply(devcb_read_base &devcb, read8_delegate &&delegate) { devcb.m_read8 = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<read16_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_16;
		static void apply(devcb_read_base &devcb, read16_delegate &&delegate) { devcb.m_read16 = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<read32_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_32;
		static void apply(devcb_read_base &devcb, read32_delegate &&delegate) { devcb.m_read32 = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<read64_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_64;
		static void apply(devcb_read_base &devcb, read64_delegate &&delegate) { devcb.m_read64 = std::move(delegate); }
	};

public:
	// callback configuration
	using devcb_base::set_callback;
	template <typename Delegate>
	std::enable_if_t<set_helper<Delegate, void>::valid, devcb_read_base &> set_callback(emu::detail::devcb_delegate_initialiser<Delegate> &&desc)
	{
		if (desc.m_base) reset(*desc.m_base, set_helper<Delegate, void>::type);
		else reset(set_helper<Delegate, void>::type);
		set_helper<Delegate, void>::apply(*this, std::move(desc.m_delegate));
		return *this;
	}
	template <typename... Params> auto &chain(Params &&... args) { return chain_alloc().set_callback(std::forward<Params>(args)...); }
	devcb_read_base &chain_alloc();

	// resolution
	void resolve();
	void resolve_safe(u64 none_constant_value);

	// validity checking
	void validity_check(validity_checker &valid) const;

protected:
	// internal helpers
	virtual void devcb_reset() override;
	inline u64 read(address_space &space, offs_t offset, u64 mask);

private:
	// adapters
	u64 read_unresolved_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_line_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read8_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read16_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read32_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read64_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_ioport_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_logged_adapter(address_space &space, offs_t offset, u64 mask);
	u64 read_constant_adapter(address_space &space, offs_t offset, u64 mask);

	// configuration
	read_line_delegate  m_readline;             // copy of registered line reader
	read8_delegate      m_read8;                // copy of registered 8-bit reader
	read16_delegate     m_read16;               // copy of registered 16-bit reader
	read32_delegate     m_read32;               // copy of registered 32-bit reader
	read64_delegate     m_read64;               // copy of registered 64-bit reader

	// derived state
	typedef u64 (devcb_read_base::*adapter_func)(address_space &, offs_t, u64);
	adapter_func        m_adapter;              // actual callback to invoke
	std::unique_ptr<devcb_read_base> m_chain;   // next callback for chained input
};


// ======================> devcb_write_base

class devcb_write_base : public devcb_base
{
protected:
	// construction/destruction
	devcb_write_base(device_t &device, u64 defmask, bool chained = false);

	template <typename Delegate, typename Dummy> struct set_helper
	{
		static constexpr bool valid = false;
	};
	template <typename Dummy> struct set_helper<write_line_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_LINE;
		static void apply(devcb_write_base &devcb, write_line_delegate &&delegate) { devcb.m_writeline = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<write8_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_8;
		static void apply(devcb_write_base &devcb, write8_delegate &&delegate) { devcb.m_write8 = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<write16_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_16;
		static void apply(devcb_write_base &devcb, write16_delegate &&delegate) { devcb.m_write16 = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<write32_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_32;
		static void apply(devcb_write_base &devcb, write32_delegate &&delegate) { devcb.m_write32 = std::move(delegate); }
	};
	template <typename Dummy> struct set_helper<write64_delegate, Dummy>
	{
		static constexpr bool valid = true;
		static constexpr callback_type type = CALLBACK_64;
		static void apply(devcb_write_base &devcb, write64_delegate &&delegate) { devcb.m_write64 = std::move(delegate); }
	};

public:
	// callback configuration
	using devcb_base::set_callback;
	template <typename Delegate>
	std::enable_if_t<set_helper<Delegate, void>::valid, devcb_write_base &> set_callback(emu::detail::devcb_delegate_initialiser<Delegate> &&desc)
	{
		if (desc.m_base) reset(*desc.m_base, set_helper<Delegate, void>::type);
		else reset(set_helper<Delegate, void>::type);
		set_helper<Delegate, void>::apply(*this, std::move(desc.m_delegate));
		return *this;
	}
	template <callback_type Type> devcb_write_base &set_callback(line_desc<Type> desc)
	{
		if (desc.m_base) reset(*desc.m_base, Type);
		else reset(Type);
		m_target_tag = desc.m_tag;
		m_target_int = desc.m_inputnum;
		return *this;
	}
	template <typename... Params> auto &chain(Params &&... args) { return chain_alloc().set_callback(std::forward<Params>(args)...); }
	devcb_write_base &chain_alloc();

	// resolution
	void resolve();
	void resolve_safe();

	// validity checking
	void validity_check(validity_checker &valid) const;

protected:
	// internal helpers
	virtual void devcb_reset() override;
	inline void write(address_space &space, offs_t offset, u64 data, u64 mask);

private:
	// adapters
	void write_unresolved_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_line_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write8_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write16_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write32_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write64_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_ioport_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_membank_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_output_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_logged_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_noop_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_inputline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_assertline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_clearline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);
	void write_holdline_adapter(address_space &space, offs_t offset, u64 data, u64 mask);

	// configuration
	write_line_delegate m_writeline;            // copy of registered line writer
	write8_delegate     m_write8;               // copy of registered 8-bit writer
	write16_delegate    m_write16;              // copy of registered 16-bit writer
	write32_delegate    m_write32;              // copy of registered 32-bit writer
	write64_delegate    m_write64;              // copy of registered 64-bit writer

	// derived state
	typedef void (devcb_write_base::*adapter_func)(address_space &, offs_t, u64, u64);
	adapter_func        m_adapter;              // actual callback to invoke
	std::unique_ptr<devcb_write_base> m_chain;  // next callback for chained output
};


// ======================> devcb_read_line

class devcb_read_line : public devcb_read_base
{
public:
	devcb_read_line(device_t &device) : devcb_read_base(device, 1) { }
	int operator()() { return read(*m_space, 0, 0xffU) & 1; }
	int operator()(address_space &space) { return read((m_space_tag != nullptr) ? *m_space : space, 0, 0xffU) & 1; }
};


// ======================> devcb_read8

class devcb_read8 : public devcb_read_base
{
public:
	devcb_read8(device_t &device) : devcb_read_base(device, 0xff) { }
	u8 operator()(offs_t offset = 0, u8 mask = 0xff) { return read(*m_space, offset, mask) & mask; }
	u8 operator()(address_space &space, offs_t offset = 0, u8 mask = 0xff) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_read16

class devcb_read16 : public devcb_read_base
{
public:
	devcb_read16(device_t &device) : devcb_read_base(device, 0xffff) { }
	u16 operator()(offs_t offset = 0, u16 mask = 0xffff) { return read(*m_space, offset, mask) & mask; }
	u16 operator()(address_space &space, offs_t offset = 0, u16 mask = 0xffff) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_read32

class devcb_read32 : public devcb_read_base
{
public:
	devcb_read32(device_t &device) : devcb_read_base(device, 0xffffffff) { }
	u32 operator()(offs_t offset = 0, u32 mask = 0xffffffff) { return read(*m_space, offset, mask) & mask; }
	u32 operator()(address_space &space, offs_t offset = 0, u32 mask = 0xffffffff) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_read64

class devcb_read64 : public devcb_read_base
{
public:
	devcb_read64(device_t &device) : devcb_read_base(device, 0xffffffffffffffffU) { }
	u64 operator()(offs_t offset = 0, u64 mask = 0xffffffffffffffffU) { return read(*m_space, offset, mask) & mask; }
	u64 operator()(address_space &space, offs_t offset = 0, u64 mask = 0xffffffffffffffffU) { return read((m_space_tag != nullptr) ? *m_space : space, offset, mask) & mask; }
};


// ======================> devcb_write_line

class devcb_write_line : public devcb_write_base
{
public:
	devcb_write_line(device_t &device) : devcb_write_base(device, 1) { }
	void operator()(int state) { write(*m_space, 0, state & 1, 0xffU); }
	void operator()(address_space &space, int state) { write((m_space_tag != nullptr) ? *m_space : space, 0, state & 1, 0xffU); }
};


// ======================> devcb_write8

class devcb_write8 : public devcb_write_base
{
public:
	devcb_write8(device_t &device) : devcb_write_base(device, 0xff) { }
	void operator()(u8 data, u8 mask = 0xff) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u8 data, u8 mask = 0xff) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u8 data, u8 mask = 0xff) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb_write16

class devcb_write16 : public devcb_write_base
{
public:
	devcb_write16(device_t &device) : devcb_write_base(device, 0xffff) { }
	void operator()(u16 data, u16 mask = 0xffff) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u16 data, u16 mask = 0xffff) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u16 data, u16 mask = 0xffff) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb_write32

class devcb_write32 : public devcb_write_base
{
public:
	devcb_write32(device_t &device) : devcb_write_base(device, 0xffffffff) { }
	void operator()(u32 data, u32 mask = 0xffffffff) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u32 data, u32 mask = 0xffffffff) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u32 data, u32 mask = 0xffffffff) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};


// ======================> devcb_write64

class devcb_write64 : public devcb_write_base
{
public:
	devcb_write64(device_t &device) : devcb_write_base(device, 0xffffffffffffffffU) { }
	void operator()(u64 data, u64 mask = 0xffffffffffffffffU) { write(*m_space, 0, data, mask); }
	void operator()(offs_t offset, u64 data, u64 mask = 0xffffffffffffffffU) { write(*m_space, offset, data, mask); }
	void operator()(address_space &space, offs_t offset, u64 data, u64 mask = 0xffffffffffffffffU) { write((m_space_tag != nullptr) ? *m_space : space, offset, data, mask); }
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  read - generic read callback dispatch
//-------------------------------------------------

inline u64 devcb_read_base::read(address_space &space, offs_t offset, u64 mask)
{
	u64 result = (this->*m_adapter)(space, offset, mask);
	if (m_chain != nullptr)
		result |= m_chain->read(space, offset, mask);
	return result;
}


//-------------------------------------------------
//  write - generic write callback dispatch
//-------------------------------------------------

inline void devcb_write_base::write(address_space &space, offs_t offset, u64 data, u64 mask)
{
	(this->*m_adapter)(space, offset, data, mask);
	if (m_chain != nullptr)
		m_chain->write(space, offset, data, mask);
}


//-------------------------------------------------
//  wrappers for ioports, constants, and loggers
//-------------------------------------------------

#define DEVCB_NOOP devcb_base::null_desc{ }
template <typename... Params> inline devcb_base::ioport_desc DEVCB_IOPORT(Params &&... args) { return emu::detail::devcb_tag_desc_creator<devcb_base::ioport_desc>::create(std::forward<Params>(args)...); }
template <typename... Params> inline devcb_base::membank_desc DEVCB_MEMBANK(Params &&... args) { return emu::detail::devcb_tag_desc_creator<devcb_base::membank_desc>::create(std::forward<Params>(args)...); }
template <typename... Params> inline devcb_base::output_desc DEVCB_OUTPUT(Params &&... args) { return emu::detail::devcb_tag_desc_creator<devcb_base::output_desc>::create(std::forward<Params>(args)...); }
inline devcb_base::constant_desc DEVCB_CONSTANT(u64 value) { return devcb_base::constant_desc{ value }; }
inline devcb_base::logger_desc DEVCB_LOGGER(char const *string) { return devcb_base::logger_desc{ string }; }
template <typename... Params> inline devcb_base::inputline_desc DEVCB_INPUTLINE(Params &&... args) { return emu::detail::devcb_line_desc_creator<devcb_base::inputline_desc>::create(std::forward<Params>(args)...); }
template <typename... Params> inline devcb_base::assertline_desc DEVCB_ASSERTLINE(Params &&... args) { return emu::detail::devcb_line_desc_creator<devcb_base::assertline_desc>::create(std::forward<Params>(args)...); }
template <typename... Params> inline devcb_base::clearline_desc DEVCB_CLEARLINE(Params &&... args) { return emu::detail::devcb_line_desc_creator<devcb_base::clearline_desc>::create(std::forward<Params>(args)...); }
template <typename... Params> inline devcb_base::holdline_desc DEVCB_HOLDLINE(Params &&... args) { return emu::detail::devcb_line_desc_creator<devcb_base::holdline_desc>::create(std::forward<Params>(args)...); }
#define DEVCB_VCC DEVCB_CONSTANT(1)
#define DEVCB_GND DEVCB_CONSTANT(0)

#endif // MAME_EMU_DEVCB_H
