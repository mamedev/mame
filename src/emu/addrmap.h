// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    addrmap.h

    Macros and helper functions for handling address map definitions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_ADDRMAP_H
#define MAME_EMU_ADDRMAP_H

#include <type_traits>


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// address map handler types
enum map_handler_type
{
	AMH_NONE = 0,
	AMH_RAM,
	AMH_ROM,
	AMH_NOP,
	AMH_UNMAP,
	AMH_DEVICE_DELEGATE,
	AMH_DEVICE_DELEGATE_M,
	AMH_DEVICE_DELEGATE_S,
	AMH_DEVICE_DELEGATE_SM,
	AMH_DEVICE_DELEGATE_MO,
	AMH_DEVICE_DELEGATE_SMO,
	AMH_PORT,
	AMH_BANK,
	AMH_DEVICE_SUBMAP,
	AMH_VIEW
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// address map handler data
class map_handler_data
{
public:
	map_handler_data() { }

	map_handler_type    m_type = AMH_NONE;  // type of the handler
	u8                  m_bits = 0;         // width of the handler in bits, or 0 for default
	const char *        m_name = nullptr;   // name of the handler
	const char *        m_tag = nullptr;    // tag for I/O ports and banks
};



// ======================> address_map_entry

// address_map_entry is a linked list element describing one address range in a map
class address_map_entry
{
	friend class address_map;

	template <typename T, typename Ret, typename... Params>
	using is_addrmap_method = std::bool_constant<std::is_constructible_v<address_map_constructor, Ret (T::*)(Params...), const char *, T *> >;

	template <typename T, typename Ret, typename... Params>
	static std::enable_if_t<is_addrmap_method<T, Ret, Params...>::value, address_map_constructor> make_delegate(Ret (T::*func)(Params...), const char *name, T *obj)
	{ return address_map_constructor(func, name, obj); }

	template <typename T, bool Reqd>
	static device_t &find_device(const device_finder<T, Reqd> &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		device_t *device(target.first.subdevice(target.second));
		if (!device)
			throw emu_fatalerror("Device %s not found in %s\n", target.second, target.first.tag());
		return *device;
	}

	template <typename T, typename U>
	static T *make_pointer(U &obj)
	{
		if constexpr (std::is_convertible_v<std::add_pointer_t<U>, std::add_pointer_t<T> >)
			return &downcast<T &>(obj);
		else
			return &dynamic_cast<T &>(obj);
	}

	template <typename T> static std::enable_if_t<emu::detail::is_device_implementation<T>::value, const char *> get_tag(T &obj) { return obj.tag(); }
	template <typename T> static std::enable_if_t<emu::detail::is_device_interface<T>::value, const char *> get_tag(T &obj) { return obj.device().tag(); }

public:
	// construction/destruction
	address_map_entry(device_t &device, address_map &map, offs_t start, offs_t end);

	// getters
	address_map_entry *next() const { return m_next; }

	// simple inline setters
	address_map_entry &mirror(offs_t _mirror) { m_addrmirror = _mirror; return *this; }
	address_map_entry &select(offs_t _select) { m_addrselect = _select; return *this; }
	address_map_entry &region(const char *tag, offs_t offset) { m_region = tag; m_rgnoffs = offset; return *this; }
	address_map_entry &share(const char *tag) { m_share = tag; return *this; }

	// slightly less simple inline setters
	template<bool _reqd> address_map_entry &region(const memory_region_finder<_reqd> &finder, offs_t offset) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return region(target.second, offset);
	}
	template<typename _ptrt, bool _reqd> address_map_entry &region(const region_ptr_finder<_ptrt, _reqd> &finder, offs_t offset) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return region(target.second, offset);
	}
	template<typename _ptrt, bool _reqd> address_map_entry &share(const shared_ptr_finder<_ptrt, _reqd> &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return share(target.second);
	}
	template<typename _ptrt> address_map_entry &share(const memory_share_creator<_ptrt> &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return share(target.second);
	}

	address_map_entry &rom() { m_read.m_type = AMH_ROM; return *this; }
	address_map_entry &ram() { m_read.m_type = AMH_RAM; m_write.m_type = AMH_RAM; return *this; }
	address_map_entry &readonly() { m_read.m_type = AMH_RAM; return *this; }
	address_map_entry &writeonly() { m_write.m_type = AMH_RAM; return *this; }
	address_map_entry &unmaprw() { m_read.m_type = AMH_UNMAP; m_write.m_type = AMH_UNMAP; return *this; }
	address_map_entry &unmapr() { m_read.m_type = AMH_UNMAP; return *this; }
	address_map_entry &unmapw() { m_write.m_type = AMH_UNMAP; return *this; }
	address_map_entry &noprw() { m_read.m_type = AMH_NOP; m_write.m_type = AMH_NOP; return *this; }
	address_map_entry &nopr() { m_read.m_type = AMH_NOP; return *this; }
	address_map_entry &nopw() { m_write.m_type = AMH_NOP; return *this; }

	// address mask setting
	address_map_entry &mask(offs_t _mask);

	// unit mask setting
	address_map_entry &umask16(u16 _mask);
	address_map_entry &umask32(u32 _mask);
	address_map_entry &umask64(u64 _mask);

	// chip select width setting
	address_map_entry &cswidth(int _cswidth) { m_cswidth = _cswidth; return *this; }

	// I/O port configuration
	address_map_entry &portr(const char *tag) { m_read.m_type = AMH_PORT; m_read.m_tag = tag; return *this; }
	address_map_entry &portw(const char *tag) { m_write.m_type = AMH_PORT; m_write.m_tag = tag; return *this; }
	address_map_entry &portrw(const char *tag) { portr(tag); portw(tag); return *this; }

	// memory bank configuration
	address_map_entry &bankr(const char *tag) { m_read.m_type = AMH_BANK; m_read.m_tag = tag; return *this; }
	address_map_entry &bankw(const char *tag) { m_write.m_type = AMH_BANK; m_write.m_tag = tag; return *this; }
	address_map_entry &bankrw(const char *tag) { bankr(tag); bankw(tag); return *this; }

	address_map_entry &bankr(const memory_bank_creator &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return bankr(target.second);
	}
	address_map_entry &bankw(const memory_bank_creator &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return bankw(target.second);
	}
	address_map_entry &bankrw(const memory_bank_creator &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return bankrw(target.second);
	}

	template<bool _reqd> address_map_entry &bankr(const memory_bank_finder<_reqd> &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return bankr(target.second);
	}
	template<bool _reqd> address_map_entry &bankw(const memory_bank_finder<_reqd> &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return bankw(target.second);
	}
	template<bool _reqd> address_map_entry &bankrw(const memory_bank_finder<_reqd> &finder) {
		const std::pair<device_t &, const char *> target(finder.finder_target());
		assert(&target.first == &m_devbase);
		return bankrw(target.second);
	}

	// type setters
	address_map_entry &set_read_type(map_handler_type _type) { m_read.m_type = _type; return *this; }
	address_map_entry &set_write_type(map_handler_type _type) { m_write.m_type = _type; return *this; }

	// submap referencing
	address_map_entry &m(const char *tag, address_map_constructor func);
	address_map_entry &m(device_t *device, address_map_constructor func);

	// view initialization
	void view(memory_view &mv);

	// implicit base -> delegate converter
	template <typename T, typename Ret, typename... Params>
	address_map_entry &r(Ret (T::*read)(Params...), const char *read_name)
	{ return r(emu::detail::make_delegate(*make_pointer<T>(m_devbase), read, read_name)); }

	template <typename T, typename Ret, typename... Params>
	address_map_entry &w(Ret (T::*write)(Params...), const char *write_name)
	{ return w(emu::detail::make_delegate(*make_pointer<T>(m_devbase), write, write_name)); }

	template <typename T, typename RetR, typename... ParamsR, typename U, typename RetW, typename... ParamsW>
	address_map_entry &rw(RetR (T::*read)(ParamsR...), const char *read_name, RetW (U::*write)(ParamsW...), const char *write_name)
	{ return r(emu::detail::make_delegate(*make_pointer<T>(m_devbase), read, read_name)).w(emu::detail::make_delegate(*make_pointer<U>(m_devbase), write, write_name)); }

	template <typename T, typename Ret, typename... Params>
	address_map_entry &m(Ret (T::*map)(Params...), const char *map_name)
	{ return m(&m_devbase, make_delegate(map, map_name, make_pointer<T>(m_devbase))); }


	// device tag -> delegate converter
	template <typename T, typename Ret, typename... Params>
	address_map_entry &r(const char *tag, Ret (T::*read)(Params...), const char *read_name)
	{ return r(emu::detail::make_delegate(m_devbase, tag, read, read_name)); }

	template <typename T, typename Ret, typename... Params>
	address_map_entry &w(const char *tag, Ret (T::*write)(Params...), const char *write_name)
	{ return w(emu::detail::make_delegate(m_devbase, tag, write, write_name)); }

	template <typename T, typename RetR, typename... ParamsR, typename U, typename RetW, typename... ParamsW>
	address_map_entry &rw(const char *tag, RetR (T::*read)(ParamsR...), const char *read_name, RetW (U::*write)(ParamsW...), const char *write_name)
	{ return r(emu::detail::make_delegate(m_devbase, tag, read, read_name)).w(emu::detail::make_delegate(m_devbase, tag, write, write_name)); }

	template <typename T, typename Ret, typename... Params>
	address_map_entry &m(const char *tag, Ret (T::*map)(Params...), const char *map_name)
	{ return m(tag, make_delegate(map, map_name, static_cast<T *>(nullptr))); }


	// device reference -> delegate converter
	template <typename T, typename U, typename Ret, typename... Params>
	address_map_entry &r(T &obj, Ret (U::*read)(Params...), const char *read_name)
	{ return r(emu::detail::make_delegate(*make_pointer<U>(obj), read, read_name)); }

	template <typename T, typename U, typename Ret, typename... Params>
	address_map_entry &w(T &obj, Ret (U::*write)(Params...), const char *write_name)
	{ return w(emu::detail::make_delegate(*make_pointer<U>(obj), write, write_name)); }

	template <typename T, typename U, typename RetR, typename... ParamsR, typename V, typename RetW, typename... ParamsW>
	address_map_entry &rw(T &obj, RetR (U::*read)(ParamsR...), const char *read_name, RetW (V::*write)(ParamsW...), const char *write_name)
	{ return r(emu::detail::make_delegate(*make_pointer<U>(obj), read, read_name)).w(emu::detail::make_delegate(make_pointer<V>(obj), write, write_name)); }

	template <typename T, typename U, typename Ret, typename... Params>
	address_map_entry &m(T &obj, Ret (U::*map)(Params...), const char *map_name)
	{ return m(make_pointer<device_t>(obj), make_delegate(map, map_name, make_pointer<U>(obj))); }


	// device finder -> delegate converter
	template <typename T, bool Reqd, typename U, typename Ret, typename... Params>
	address_map_entry &r(device_finder<T, Reqd> &finder, Ret (U::*read)(Params...), const char *read_name) {
		device_t &device(find_device(finder));
		return r(emu::detail::make_delegate(device, DEVICE_SELF, read, read_name));
	}

	template <typename T, bool Reqd, typename U, typename Ret, typename... Params>
	address_map_entry &r(const device_finder<T, Reqd> &finder, Ret (U::*read)(Params...), const char *read_name) {
		device_t &device(find_device(finder));
		return r(emu::detail::make_delegate(device, DEVICE_SELF, read, read_name));
	}

	template <typename T, bool Reqd, typename U, typename Ret, typename... Params>
	address_map_entry &w(device_finder<T, Reqd> &finder, Ret (U::*write)(Params...), const char *write_name) {
		device_t &device(find_device(finder));
		return w(emu::detail::make_delegate(device, DEVICE_SELF, write, write_name));
	}

	template <typename T, bool Reqd, typename U, typename Ret, typename... Params>
	address_map_entry &w(const device_finder<T, Reqd> &finder, Ret (U::*write)(Params...), const char *write_name) {
		device_t &device(find_device(finder));
		return w(emu::detail::make_delegate(device, DEVICE_SELF, write, write_name));
	}

	template <typename T, bool Reqd, typename U, typename RetR, typename... ParamsR, typename V, typename RetW, typename... ParamsW>
	address_map_entry &rw(device_finder<T, Reqd> &finder, RetR (U::*read)(ParamsR...), const char *read_name, RetW (V::*write)(ParamsW...), const char *write_name) {
		device_t &device(find_device(finder));
		return r(emu::detail::make_delegate(device, DEVICE_SELF, read, read_name))
				.w(emu::detail::make_delegate(device, DEVICE_SELF, write, write_name));
	}

	template <typename T, bool Reqd, typename U, typename RetR, typename... ParamsR, typename V, typename RetW, typename... ParamsW>
	address_map_entry &rw(const device_finder<T, Reqd> &finder, RetR (U::*read)(ParamsR...), const char *read_name, RetW (V::*write)(ParamsW...), const char *write_name) {
		device_t &device(find_device(finder));
		return r(emu::detail::make_delegate(device, DEVICE_SELF, read, read_name))
				.w(emu::detail::make_delegate(device, DEVICE_SELF, write, write_name));
	}

	template <typename T, bool Reqd, typename U, typename Ret, typename... Params>
	address_map_entry &m(device_finder<T, Reqd> &finder, Ret (U::*map)(Params...), const char *map_name) {
		device_t &device(find_device(finder));
		return m(&device, make_delegate(map, map_name, make_pointer<U>(device)));
	}

	template <typename T, bool Reqd, typename U, typename Ret, typename... Params>
	address_map_entry &m(const device_finder<T, Reqd> &finder, Ret (U::*map)(Params...), const char *map_name) {
		device_t &device(find_device(finder));
		return m(&device, make_delegate(map, map_name, make_pointer<U>(device)));
	}


	// lambda -> delegate converter
	template <typename T> address_map_entry &lr8(T &&read, const char *name)
	{ return r(emu::detail::make_lr8_delegate(m_devbase, std::forward<T>(read), name)); }

	template <typename T> address_map_entry &lr16(T &&read, const char *name)
	{ return r(emu::detail::make_lr16_delegate(m_devbase, std::forward<T>(read), name)); }

	template <typename T> address_map_entry &lr32(T &&read, const char *name)
	{ return r(emu::detail::make_lr32_delegate(m_devbase, std::forward<T>(read), name)); }

	template <typename T> address_map_entry &lr64(T &&read, const char *name)
	{ return r(emu::detail::make_lr64_delegate(m_devbase, std::forward<T>(read), name)); }

	template <typename T> address_map_entry &lw8(T &&write, const char *name)
	{ return w(emu::detail::make_lw8_delegate(m_devbase, std::forward<T>(write), name)); }

	template <typename T> address_map_entry &lw16(T &&write, const char *name)
	{ return w(emu::detail::make_lw16_delegate(m_devbase, std::forward<T>(write), name)); }

	template <typename T> address_map_entry &lw32(T &&write, const char *name)
	{ return w(emu::detail::make_lw32_delegate(m_devbase, std::forward<T>(write), name)); }

	template <typename T> address_map_entry &lw64(T &&write, const char *name)
	{ return w(emu::detail::make_lw64_delegate(m_devbase, std::forward<T>(write), name)); }

	template <typename T, typename U> address_map_entry &lrw8(T &&read, const char *read_name, U &&write, const char *write_name)
	{ return r(emu::detail::make_lr8_delegate(m_devbase, std::forward<T>(read), read_name)).w(emu::detail::make_lw8_delegate(m_devbase, std::forward<U>(write), write_name)); }

	template <typename T, typename U> address_map_entry &lrw16(T &&read, const char *read_name, U &&write, const char *write_name)
	{ return r(emu::detail::make_lr16_delegate(m_devbase, std::forward<T>(read), read_name)).w(emu::detail::make_lw16_delegate(m_devbase, std::forward<U>(write), write_name)); }

	template <typename T, typename U> address_map_entry &lrw32(T &&read, const char *read_name, U &&write, const char *write_name)
	{ return r(emu::detail::make_lr32_delegate(m_devbase, std::forward<T>(read), read_name)).w(emu::detail::make_lw32_delegate(m_devbase, std::forward<U>(write), write_name)); }

	template <typename T, typename U> address_map_entry &lrw64(T &&read, const char *read_name, U &&write, const char *write_name)
	{ return r(emu::detail::make_lr64_delegate(m_devbase, std::forward<T>(read), read_name)).w(emu::detail::make_lw64_delegate(m_devbase, std::forward<U>(write), write_name)); }

	// public state
	address_map_entry *     m_next;                 // pointer to the next entry
	address_map &           m_map;                  // reference to our owning map
	device_t &              m_devbase;              // reference to "base" device for tag lookups

	// basic information
	offs_t                  m_addrstart;            // start address
	offs_t                  m_addrend;              // end address
	offs_t                  m_addrmirror;           // mirror bits
	offs_t                  m_addrmask;             // mask bits
	offs_t                  m_addrselect;           // select bits
	u64                     m_mask;                 // mask for which lanes apply
	int                     m_cswidth;              // chip select width override
	map_handler_data        m_read;                 // data for read handler
	map_handler_data        m_write;                // data for write handler
	const char *            m_share;                // tag of a shared memory block
	const char *            m_region;               // tag of region containing the memory backing this entry
	offs_t                  m_rgnoffs;              // offset within the region

	// handlers
	read8_delegate          m_rproto8;              // 8-bit read proto-delegate
	read16_delegate         m_rproto16;             // 16-bit read proto-delegate
	read32_delegate         m_rproto32;             // 32-bit read proto-delegate
	read64_delegate         m_rproto64;             // 64-bit read proto-delegate
	write8_delegate         m_wproto8;              // 8-bit write proto-delegate
	write16_delegate        m_wproto16;             // 16-bit write proto-delegate
	write32_delegate        m_wproto32;             // 32-bit write proto-delegate
	write64_delegate        m_wproto64;             // 64-bit write proto-delegate

	read8m_delegate         m_rproto8m;             // 8-bit read proto-delegate
	read16m_delegate        m_rproto16m;            // 16-bit read proto-delegate
	read32m_delegate        m_rproto32m;            // 32-bit read proto-delegate
	read64m_delegate        m_rproto64m;            // 64-bit read proto-delegate
	write8m_delegate        m_wproto8m;             // 8-bit write proto-delegate
	write16m_delegate       m_wproto16m;            // 16-bit write proto-delegate
	write32m_delegate       m_wproto32m;            // 32-bit write proto-delegate
	write64m_delegate       m_wproto64m;            // 64-bit write proto-delegate

	read8s_delegate         m_rproto8s;             // 8-bit read proto-delegate
	read16s_delegate        m_rproto16s;            // 16-bit read proto-delegate
	read32s_delegate        m_rproto32s;            // 32-bit read proto-delegate
	read64s_delegate        m_rproto64s;            // 64-bit read proto-delegate
	write8s_delegate        m_wproto8s;             // 8-bit write proto-delegate
	write16s_delegate       m_wproto16s;            // 16-bit write proto-delegate
	write32s_delegate       m_wproto32s;            // 32-bit write proto-delegate
	write64s_delegate       m_wproto64s;            // 64-bit write proto-delegate

	read8sm_delegate        m_rproto8sm;            // 8-bit read proto-delegate
	read16sm_delegate       m_rproto16sm;           // 16-bit read proto-delegate
	read32sm_delegate       m_rproto32sm;           // 32-bit read proto-delegate
	read64sm_delegate       m_rproto64sm;           // 64-bit read proto-delegate
	write8sm_delegate       m_wproto8sm;            // 8-bit write proto-delegate
	write16sm_delegate      m_wproto16sm;           // 16-bit write proto-delegate
	write32sm_delegate      m_wproto32sm;           // 32-bit write proto-delegate
	write64sm_delegate      m_wproto64sm;           // 64-bit write proto-delegate

	read8mo_delegate        m_rproto8mo;            // 8-bit read proto-delegate
	read16mo_delegate       m_rproto16mo;           // 16-bit read proto-delegate
	read32mo_delegate       m_rproto32mo;           // 32-bit read proto-delegate
	read64mo_delegate       m_rproto64mo;           // 64-bit read proto-delegate
	write8mo_delegate       m_wproto8mo;            // 8-bit write proto-delegate
	write16mo_delegate      m_wproto16mo;           // 16-bit write proto-delegate
	write32mo_delegate      m_wproto32mo;           // 32-bit write proto-delegate
	write64mo_delegate      m_wproto64mo;           // 64-bit write proto-delegate

	read8smo_delegate       m_rproto8smo;           // 8-bit read proto-delegate
	read16smo_delegate      m_rproto16smo;          // 16-bit read proto-delegate
	read32smo_delegate      m_rproto32smo;          // 32-bit read proto-delegate
	read64smo_delegate      m_rproto64smo;          // 64-bit read proto-delegate
	write8smo_delegate      m_wproto8smo;           // 8-bit write proto-delegate
	write16smo_delegate     m_wproto16smo;          // 16-bit write proto-delegate
	write32smo_delegate     m_wproto32smo;          // 32-bit write proto-delegate
	write64smo_delegate     m_wproto64smo;          // 64-bit write proto-delegate

	device_t               *m_submap_device;
	address_map_constructor m_submap_delegate;
	memory_view            *m_view;

	// information used during processing
	void *                  m_memory;               // pointer to memory backing this entry

	// handler setters for 8-bit delegates
	address_map_entry &r(read8_delegate func);
	address_map_entry &w(write8_delegate func);
	address_map_entry &r(read8m_delegate func);
	address_map_entry &w(write8m_delegate func);
	address_map_entry &r(read8s_delegate func);
	address_map_entry &w(write8s_delegate func);
	address_map_entry &r(read8sm_delegate func);
	address_map_entry &w(write8sm_delegate func);
	address_map_entry &r(read8mo_delegate func);
	address_map_entry &w(write8mo_delegate func);
	address_map_entry &r(read8smo_delegate func);
	address_map_entry &w(write8smo_delegate func);

	// handler setters for 16-bit delegates
	address_map_entry &r(read16_delegate func);
	address_map_entry &w(write16_delegate func);
	address_map_entry &r(read16m_delegate func);
	address_map_entry &w(write16m_delegate func);
	address_map_entry &r(read16s_delegate func);
	address_map_entry &w(write16s_delegate func);
	address_map_entry &r(read16sm_delegate func);
	address_map_entry &w(write16sm_delegate func);
	address_map_entry &r(read16mo_delegate func);
	address_map_entry &w(write16mo_delegate func);
	address_map_entry &r(read16smo_delegate func);
	address_map_entry &w(write16smo_delegate func);

	// handler setters for 32-bit delegates
	address_map_entry &r(read32_delegate func);
	address_map_entry &w(write32_delegate func);
	address_map_entry &r(read32m_delegate func);
	address_map_entry &w(write32m_delegate func);
	address_map_entry &r(read32s_delegate func);
	address_map_entry &w(write32s_delegate func);
	address_map_entry &r(read32sm_delegate func);
	address_map_entry &w(write32sm_delegate func);
	address_map_entry &r(read32mo_delegate func);
	address_map_entry &w(write32mo_delegate func);
	address_map_entry &r(read32smo_delegate func);
	address_map_entry &w(write32smo_delegate func);

	// handler setters for 64-bit delegates
	address_map_entry &r(read64_delegate func);
	address_map_entry &w(write64_delegate func);
	address_map_entry &r(read64m_delegate func);
	address_map_entry &w(write64m_delegate func);
	address_map_entry &r(read64s_delegate func);
	address_map_entry &w(write64s_delegate func);
	address_map_entry &r(read64sm_delegate func);
	address_map_entry &w(write64sm_delegate func);
	address_map_entry &r(read64mo_delegate func);
	address_map_entry &w(write64mo_delegate func);
	address_map_entry &r(read64smo_delegate func);
	address_map_entry &w(write64smo_delegate func);

private:
	// helper functions
	bool unitmask_is_appropriate(u8 width, u64 unitmask, const char *string) const;
};

// ======================> address_map

// address_map holds global map parameters plus the head of the list of entries
class address_map
{
public:
	// construction/destruction
	address_map(device_t &device, int spacenum);
	address_map(memory_view &view);
	address_map(device_t &device, address_map_entry *entry);
	address_map(const address_space &space, offs_t start, offs_t end, u64 unitmask, int cswidth, device_t &device, address_map_constructor submap_delegate);
	~address_map();

	// setters
	void global_mask(offs_t mask);
	void unmap_value_low() { m_unmapval = 0; }
	void unmap_value_high() { m_unmapval = ~0; }

	// add a new entry of the given type
	address_map_entry &operator()(offs_t start, offs_t end);

	// public data
	int                             m_spacenum;     // space number of the map
	device_t *                      m_device;       // associated device
	memory_view *                   m_view;         // view, when in one
	const address_space_config *    m_config;       // space configuration
	u8                              m_unmapval;     // unmapped memory value
	offs_t                          m_globalmask;   // global mask
	simple_list<address_map_entry>  m_entrylist;    // list of entries

	void import_submaps(running_machine &machine, device_t &owner, int data_width, endianness_t endian, int addr_shift);
	void map_validity_check(validity_checker &valid, int spacenum) const;
	const address_space_config &get_config() const;
};

#endif // MAME_EMU_ADDRMAP_H
