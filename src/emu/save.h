// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    save.h

    Save state management functions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_SAVE_H
#define MAME_EMU_SAVE_H

#include <array>
#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <hashing.h>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum save_error
{
	STATERR_NONE,
	STATERR_MISMATCH_WARNING,
	STATERR_NOT_FOUND,
	STATERR_INVALID_FILE,
	STATERR_READ_ERROR,
	STATERR_MALFORMED_JSON,
	STATERR_MISSING_FILE,
	STATERR_INCOMPATIBLE_DATA,
	STATERR_WRITE_ERROR,
	STATERR_DISABLED
};


//**************************************************************************
//  MACROS
//**************************************************************************

// callback delegate for presave/postload
typedef named_delegate<void ()> save_prepost_delegate;

// use this macro to save a given type as a signed integer
#define SAVE_TYPE_AS_INT(Type) \
	template<> struct save_registrar::is_signed_int_like<Type> { static constexpr bool value = true; };

// use this macro to save a given type as an unsigned integer
#define SAVE_TYPE_AS_UINT(Type) \
	template<> struct save_registrar::is_unsigned_int_like<Type> { static constexpr bool value = true; };

// use this macro to save a given type as a floating-point value
#define SAVE_TYPE_AS_FLOAT(Type) \
	template<> struct save_registrar::is_floating_point_like<Type> { static constexpr bool value = true; };


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class save_zip_state;
class load_zip_state;
class zlib_streamer;
class ram_state;
class rewinder;


// ======================> save_registered_item

// this class manages a single item node in the hierarchy of registered save items
class save_registered_item
{
	// generic types used as proxies for extracting pointers
	using generic_unique = std::unique_ptr<int> const;
	using generic_vector = std::vector<int> const;
	using generic_pointer = void * const;

public:
	// various modes for restoring JSON data
	enum json_restore_mode
	{
		RESTORE_DATA,
		COMPARE_DATA,
		PARSE_ONLY
	};

	// the various types supported
	enum save_type : u32
	{
		// type                native_size           ptr_offset   items
		TYPE_BOOL,          // sizeof(bool)          relative     none
		TYPE_INT,           // sizeof(value)         relative     none
		TYPE_UINT,          // sizeof(value)         relative     none
		TYPE_FLOAT,         // sizeof(value)         relative     none

		TYPE_CONTAINER,     // 0                     0            list of contained items
		TYPE_STRUCT,        // sizeof(struct)        relative     list of contained items
		TYPE_STATIC_ARRAY,  // space-between-elems   relative     l
		TYPE_RUNTIME_ARRAY, // space-between-elems   relative

		TYPE_UNIQUE,        // sizeof(unique_ptr)    relative     item at pointer
		TYPE_VECTOR,        // sizeof(vector)        relative     TYPE_RUNTIME_ARRAY at pointer
	};

	// root constructor
    save_registered_item();

	// constructor for a new item
    save_registered_item(uintptr_t ptr_offset, save_type type, u32 native_size, char const *name, u32 count = 0);

	// simple getters
	char const *name() const { return m_name.c_str(); }
	save_type type() const { return save_type(m_type_count & 15); }
	u32 count() const { return m_type_count >> 4; }
	bool is_struct_or_container() const { return (type() == TYPE_STRUCT || type() == TYPE_CONTAINER); }
	bool is_array() const { return (type() == TYPE_STATIC_ARRAY || type() == TYPE_RUNTIME_ARRAY); }
	bool is_int() const { return (type() == TYPE_INT || type() == TYPE_UINT); }
	bool is_int_or_float() const { return (is_int() || type() == TYPE_FLOAT); }
	std::list<save_registered_item> &subitems() { return m_items; }
	u32 native_size() const { return m_native_size; }
	uintptr_t ptr_offset() const { return m_ptr_offset; }

	// append a new item to the current one
    save_registered_item &append(uintptr_t ptr_offset, save_type type, u32 native_size, char const *name, u32 count = 0);

	// find an item by name
	save_registered_item *find(char const *name);

	// is this item replicatable (i.e., can we replicate it for all elements in an array?)
	bool is_replicatable(bool parent_is_array) const;

	// sort subitems by name and prune any empty items
	bool sort_and_prune();

	// update the object base and unwrap special items
	bool unwrap_and_update_base(uintptr_t &objbase) const;

	// compute the binary size by just saving with a null
	u64 compute_binary_size(uintptr_t parentbase = 0) const { return save_binary(nullptr, 0, parentbase); }

	// save this item and all owned items into a binary form
	u64 save_binary(u8 *ptr, u64 length, uintptr_t parentbase = 0) const;

	// restore this item and all owned items from binary form
	u64 restore_binary(u8 const *ptr, u64 length, uintptr_t parentbase = 0) const;

	// save this item into a JSON stream
	void save_json(save_zip_state &output, char const *nameprefix = "", int indent = 0, bool inline_form = false, uintptr_t parentbase = 0);

	// restore this item from a JSON stream
	void restore_json(load_zip_state &input, char const *nameprefix = "", json_restore_mode mode = RESTORE_DATA, uintptr_t parentbase = 0);

	// read/write helpers for bools
	bool read_bool(uintptr_t objptr) const { return *reinterpret_cast<bool const *>(objptr); }
	void write_bool(uintptr_t objptr, bool data) const { *reinterpret_cast<bool *>(objptr) = data; }

	// read/write helpers for signed integers
	s64 read_int_signed(uintptr_t objptr, int size) const;
	bool write_int_signed(uintptr_t objptr, int size, s64 data) const;
	bool write_int_signed(uintptr_t objptr, int size, double data) const;

	// read/write helpers for unsigned integers
	u64 read_int_unsigned(uintptr_t objptr, int size) const;
	bool write_int_unsigned(uintptr_t objptr, int size, u64 data) const;
	bool write_int_unsigned(uintptr_t objptr, int size, double data) const;

	// read/write helpers for floats
	double read_float(uintptr_t objptr, int size) const;
	bool write_float(uintptr_t objptr, int size, double data) const;

private:
	// true if this item is an array of endpoints
	bool is_endpoint_array(u32 &total, u32 &unitsize) const;

	// parse out an external file spec from the JSON
	bool parse_external_data(load_zip_state &input, save_registered_item &baseitem, char const *localname, bool parseonly, uintptr_t parentbase);

	// internal state
    std::list<save_registered_item> m_items; // list of embedded items
    uintptr_t m_ptr_offset;                  // pointer or offset
	u32 m_type_count;                        // type and count
    u32 m_native_size;                       // native size of item
    std::string m_name;                      // name of item
};


// ======================> save_registrar

// this class is the public interface to registration; it contains the heavily
// templated registration helpers that do the right thing for all supported types
class save_registrar
{
	friend class save_manager;
	friend class device_t;

	// extended constructor for internal use only
	save_registrar(save_registrar &parent, void *baseptr, save_registered_item::save_type type, u32 native_size, char const *name, u32 count, void *regcontainerbase, u32 regcontainersize) :
		m_parent_item(parent.parent_item().append((baseptr == nullptr) ? 0 : uintptr_t(baseptr) - parent.m_regcontainerbase, type, native_size, name, count)),
		m_regcontainerbase(uintptr_t(regcontainerbase)),
		m_regcontainersize(regcontainersize)
	{
	}

	// internal constructor
    save_registrar(save_registered_item &parent, void *baseptr = nullptr) :
        m_parent_item(parent),
        m_regcontainerbase(uintptr_t(baseptr)),
		m_regcontainersize(0)
    {
    }

public:
	// items that are signed_int_like are interpreted as 8/16/32/64-bit signed integers; this includes
	// proper signed integral values and enums by default; additional types may be added via the
	// SAVE_TYPE_AS_INT macro
	template<typename T> struct is_signed_int_like { static constexpr bool value = ((std::is_integral<T>::value && std::is_signed<T>::value) || std::is_enum<T>::value); };

	// items that are unsigned_int_like are interpreted as 8/16/32/64-bit unsigned integers; this includes
	// proper unsigned integral values; additional types may be added via the SAVE_TYPE_AS_UINT macro
	template<typename T> struct is_unsigned_int_like { static constexpr bool value = (std::is_integral<T>::value && !std::is_signed<T>::value); };

	// items that are floating_point_like are interpreted as 32/64-bit IEEE floating point; this includes
	// proper float and double values; additional types may be added via the SAVE_TYPE_AS_FLOAT macro
	template<typename T> struct is_floating_point_like { static constexpr bool value = std::is_floating_point<T>::value; };

	// items are considered endpoints if they fall into one of the three classes above
	template<typename T> struct is_endpoint { static constexpr bool value = (is_signed_int_like<T>::value || is_unsigned_int_like<T>::value || is_floating_point_like<T>::value); };

	// construct a container within parent
	save_registrar(save_registrar &parent, char const *name) :
		save_registrar(parent, nullptr, save_registered_item::TYPE_CONTAINER, 0, name, 0, nullptr, 0)
	{
	}

	// return a reference to the parent item
	save_registered_item &parent_item() const { return m_parent_item; }

	// append a bucket by stealing its items
	save_registrar &reg(save_registrar &src, char const *name)
	{
		save_registrar container(*this, name);
		auto &srcitems = src.parent_item().subitems();
		auto &dstitems = container.parent_item().subitems();
		dstitems.splice(dstitems.end(), srcitems);
        return *this;
	}

    // bool as a special case
    save_registrar &reg(bool &data, char const *name)
	{
		return register_endpoint(&data, save_registered_item::TYPE_BOOL, sizeof(data), name);
	}

    // signed integral types
    template<typename T>
    std::enable_if_t<is_signed_int_like<T>::value, save_registrar> &reg(T &data, char const *name)
	{
		static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
		return register_endpoint(&data, save_registered_item::TYPE_INT, sizeof(data), name);
	}

    // unsigned integral types
    template<typename T>
    std::enable_if_t<is_unsigned_int_like<T>::value, save_registrar> &reg(T &data, char const *name)
	{
		static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
		return register_endpoint(&data, save_registered_item::TYPE_UINT, sizeof(data), name);
	}

    // floating-point types
    template<typename T>
    std::enable_if_t<is_floating_point_like<T>::value, save_registrar> &reg(T &data, char const *name)
	{
		static_assert(sizeof(T) == 4 || sizeof(T) == 8);
		return register_endpoint(&data, save_registered_item::TYPE_FLOAT, sizeof(data), name);
	}

	// std::unique_ptrs -- these are containers with a single "unique" item within
	template<typename T>
	std::enable_if_t<!std::is_array<T>::value, save_registrar> &reg(std::unique_ptr<T> &data, char const *name)
	{
		if (data.get() == nullptr)
			throw emu_fatalerror("Passed null pointer to save state registration.");

		save_registrar container(*this, &data, save_registered_item::TYPE_UNIQUE, sizeof(data), name, 0, data.get(), sizeof(T));
		container.reg(*data.get(), name);
		return *this;
	}

	// pointers with count -- treat as an array
	template<typename T>
	save_registrar &reg(T *data, char const *name, std::size_t count)
	{
		return register_array(data, save_registered_item::TYPE_RUNTIME_ARRAY, name, count);
	}

    // arrays -- these are containers with a single item representing the underlying data,
	// which is replicated across the whole array
    template<typename T, u32 N>
    save_registrar &reg(T (&data)[N], char const *name)
	{
		return register_array(&data[0], save_registered_item::TYPE_STATIC_ARRAY, name, N);
	}

    // std::arrays -- treat these identically to arrays
    template<typename T, u32 N>
    save_registrar &reg(std::array<T, N> &data, char const *name)
	{
		return register_array(&data[0], save_registered_item::TYPE_STATIC_ARRAY, name, N);
	}

    // std::vectors -- these are treated as arrays, but wrapped
    template<typename T>
    save_registrar &reg(std::vector<T> &data, char const *name)
    {
		// skip if no items
		if (data.size() == 0)
			return *this;

		// create an outer container for the vector, then a regular array container within
		save_registrar container(*this, &data, save_registered_item::TYPE_VECTOR, sizeof(data), name, 0, &data[0], sizeof(T));
		container.register_array(&data[0], save_registered_item::TYPE_RUNTIME_ARRAY, name, data.size());
		return *this;
    }

	// std::unique_ptrs with arrays
	template<typename T>
	save_registrar &reg(std::unique_ptr<T[]> &data, char const *name, std::size_t count)
	{
		// skip if no items
		if (count == 0)
			return *this;

		// create an outer container for the unique_ptr, then a regular array container within
		save_registrar container(*this, &data, save_registered_item::TYPE_UNIQUE, sizeof(data), name, 0, data.get(), sizeof(T));
		container.register_array(&data[0], save_registered_item::TYPE_RUNTIME_ARRAY, name, count);
		return *this;
	}

	// structures & unions (must have a register_save method)
    template<typename T, std::enable_if_t<(std::is_class<T>::value || std::is_union<T>::value) && !std::is_base_of<bitmap_t, T>::value && !is_endpoint<T>::value, bool> = true>
    save_registrar &reg(T &data, char const *name, std::size_t datasize = 0)
    {
		save_registrar container(*this, &data, save_registered_item::TYPE_STRUCT, sizeof(data), name, 0, &data, (datasize == 0) ? sizeof(T) : datasize);
        data.register_save(container);
        return *this;
    }

	// rectangle as a special case, since it's from an external library
    save_registrar &reg(rectangle &data, char const *name)
	{
		save_registrar container(*this, &data, save_registered_item::TYPE_STRUCT, sizeof(data), name, 0, &data, sizeof(data));
		container.reg(data.min_x, "min_x").reg(data.max_x, "max_x").reg(data.min_y, "min_y").reg(data.max_y, "max_y");
        return *this;
	}

	// bitmaps as a special case, since they're from an external library
	template<typename BitmapType>
	std::enable_if_t<std::is_base_of<bitmap_t, BitmapType>::value, save_registrar> &reg(BitmapType &data, char const *name)
	{
		void *pixbase = data.raw_pixptr(0);
		save_registrar rows(*this, pixbase, save_registered_item::TYPE_RUNTIME_ARRAY, data.rowbytes(), name, data.height(), pixbase, 0);
		save_registrar cols(rows, pixbase, save_registered_item::TYPE_RUNTIME_ARRAY, sizeof(BitmapType::pixel_t), "", data.width(), pixbase, 0);
		cols.register_endpoint(pixbase, save_registered_item::TYPE_UINT, sizeof(BitmapType::pixel_t), "");
		return *this;
	}

private:
	// register an endpoint item (containing no subitems)
    save_registrar &register_endpoint(void *memptr, save_registered_item::save_type type, std::size_t itemsize, char const *itemname)
    {
		uintptr_t delta = uintptr_t(memptr) - m_regcontainerbase;
		if (m_regcontainersize != 0 && delta >= m_regcontainersize)
			throw emu_fatalerror("Attempted to register item outside of parent element's bounds");
        m_parent_item.append(delta, type, itemsize, itemname);
        return *this;
    }

	// register an array item
	template<typename T>
	save_registrar &register_array(T *data, save_registered_item::save_type type, char const *name, std::size_t count)
	{
		// skip 0-length items
		if (count == 0)
			return *this;

		// error on null pointer
		if (data == nullptr)
			throw emu_fatalerror("Passed null pointer to save state registration.");

		// create a container and register the first item
		save_registrar container(*this, data, type, uintptr_t(&data[1]) - uintptr_t(&data[0]), name, count, &data[0], sizeof(T));
		container.reg(data[0], "");

		// if the first item was non-replicatable, register remaining items independently
		if (!container.m_parent_item.subitems().front().is_replicatable(true))
			for (int index = 1; index < count; index++)
			{
				container.m_regcontainerbase = uintptr_t(&data[index]);
				container.reg(data[index], "");
			}
		return *this;
	}

	// internal state
    save_registered_item &m_parent_item;
	uintptr_t m_regcontainerbase;
	u32 m_regcontainersize;
};

// these types are small structures/unions that embed an integral type; treat them as raw
// integral types for saving purposes
SAVE_TYPE_AS_UINT(rgb_t);
SAVE_TYPE_AS_UINT(PAIR);
SAVE_TYPE_AS_UINT(PAIR64);


// ======================> save_manager

class save_manager
{
	friend class ram_state;
	friend class rewinder;

public:
	// construction/destruction
	save_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	rewinder *rewind() { return m_rewind.get(); }
	bool registration_allowed() const { return m_reg_allowed; }
	save_registrar &root_registrar() { return m_root_registrar; }

	// registration control
	void allow_registration(bool allowed = true);

	// function registration
	void register_presave(save_prepost_delegate func);
	void register_postload(save_prepost_delegate func);

	// callback dispatching
	void dispatch_presave();
	void dispatch_postload();

	// binary file processing (internal)
	size_t binary_size() { return m_root_item.compute_binary_size(); }
	save_error save_binary(void *buf, size_t size);
	save_error save_binary(std::vector<u8> &buffer) { buffer.resize(binary_size()); return save_binary(&buffer[0], buffer.size()); }
	save_error load_binary(void *buf, size_t size);
	save_error load_binary(std::vector<u8> &buffer) { return load_binary(&buffer[0], buffer.size()); }

	// disk file processing (external)
	save_error save_file(emu_file &file);
	save_error load_file(emu_file &file);
	save_error compare_file(emu_file &file);

private:
	// state callback item
	class state_callback
	{
	public:
		// construction/destruction
		state_callback(save_prepost_delegate callback) : m_func(std::move(callback)) { }
		save_prepost_delegate m_func;                 // delegate
	};

	// internal state
	running_machine &         m_machine;              // reference to our machine
	std::unique_ptr<rewinder> m_rewind;               // rewinder
	bool                      m_reg_allowed;          // are registrations allowed?
	save_registered_item      m_root_item;            // the root item in the hierarchy
	save_registrar            m_root_registrar;       // a registrar for adding to the root item

	std::vector<std::unique_ptr<ram_state>>      m_ramstate_list;    // list of ram states
	std::vector<std::unique_ptr<state_callback>> m_presave_list;     // list of pre-save functions
	std::vector<std::unique_ptr<state_callback>> m_postload_list;    // list of post-load functions
};


// ======================> ram_state

class ram_state
{
public:
	bool               m_valid;                       // can we load this state?
	attotime           m_time;                        // machine timestamp

	ram_state(save_manager &save);
	save_error save();
	save_error load();

private:
	save_manager &     m_save;                        // reference to save_manager
	std::vector<u8>    m_data;                        // save data buffer
};


// ======================> rewinder

class rewinder
{
public:
	rewinder(save_manager &save);
	bool enabled() { return m_enabled; }
	void clamp_capacity();
	void invalidate();
	bool capture();
	bool step();

private:
	save_manager & m_save;                            // reference to save_manager
	bool           m_enabled;                         // enable rewind savestates
	size_t         m_capacity;                        // total memory rewind states can occupy (MB, limited to 1-2048 in options)
	s32            m_current_index;                   // where we are in time
	s32            m_first_invalid_index;             // all states before this one are guarateed to be valid
	bool           m_first_time_warning;              // keep track of warnings we report
	bool           m_first_time_note;                 // keep track of notes
	std::vector<std::unique_ptr<ram_state>> m_state_list; // rewinder's own ram states

	// load/save management
	enum class rewind_operation
	{
		SAVE,
		LOAD
	};

	enum
	{
		REWIND_INDEX_NONE = -1,
		REWIND_INDEX_FIRST
	};

	bool check_size();
	bool current_index_is_last() { return m_current_index == m_state_list.size() - 1; }
	void report_error(save_error type, rewind_operation operation);
};

#endif // MAME_EMU_SAVE_H
