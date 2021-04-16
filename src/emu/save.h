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
	STATERR_NOT_FOUND,
	STATERR_ILLEGAL_REGISTRATIONS,
	STATERR_INVALID_HEADER,
	STATERR_READ_ERROR,
	STATERR_WRITE_ERROR,
	STATERR_DISABLED
};


//**************************************************************************
//  MACROS
//**************************************************************************

/*
TEST:
#define REGISTER_SAVE_1(a) .reg(a, #a)
#define REGISTER_SAVE_2(a,b) REGISTER_SAVE_1(a) REGISTER_SAVE_1(b)
#define REGISTER_SAVE_3(a,b,c) REGISTER_SAVE_1(a) REGISTER_SAVE_2(b,c)
#define REGISTER_SAVE_4(a,b,c,d) REGISTER_SAVE_1(a) REGISTER_SAVE_3(b,c,d)
#define REGISTER_SAVE_5(a,b,c,d,e) REGISTER_SAVE_1(a) REGISTER_SAVE_4(b,c,d,e)
#define REGISTER_SAVE_6(a,b,c,d,e,f) REGISTER_SAVE_1(a) REGISTER_SAVE_5(b,c,d,e,f)
#define REGISTER_SAVE_7(a,b,c,d,e,f,g) REGISTER_SAVE_1(a) REGISTER_SAVE_6(b,c,d,e,f,g)
#define REGISTER_SAVE_8(a,b,c,d,e,f,g,h) REGISTER_SAVE_1(a) REGISTER_SAVE_7(b,c,d,e,f,g,h)
#define REGISTER_SAVE_9(a,b,c,d,e,f,g,h,i) REGISTER_SAVE_1(a) REGISTER_SAVE_8(b,c,d,e,f,g,h,i)

#define GET_SAVE_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, NAME, ...) NAME
#define REGISTER_SAVE(...) GET_SAVE_N(__VA_ARGS__, REGISTER_SAVE_9, REGISTER_SAVE_8, REGISTER_SAVE_7, REGISTER_SAVE_6, REGISTER_SAVE_5, REGISTER_SAVE_4, REGISTER_SAVE_3, REGISTER_SAVE_2, REGISTER_SAVE_1)(__VA_ARGS__)
*/


// callback delegate for presave/postload
typedef named_delegate<void ()> save_prepost_delegate;


#define SAVE_TYPE_AS_INT(Type) \
	template<> struct save_registrar::is_signed_int_like<Type> { static constexpr bool value = true; };

#define SAVE_TYPE_AS_UINT(Type) \
	template<> struct save_registrar::is_unsigned_int_like<Type> { static constexpr bool value = true; };

#define SAVE_TYPE_AS_FLOAT(Type) \
	template<> struct save_registrar::is_floating_point_like<Type> { static constexpr bool value = true; };



// remove me
#define ALLOW_SAVE_TYPE(TYPE)
#define ALLOW_SAVE_TYPE_AND_VECTOR(TYPE)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class save_registered_item;
class zlib_streamer;
class ram_state;
class rewinder;


// ======================> save_zip_state

// this class manages the creation of a ZIP file containing a JSON with most of
// the save data, plus various binary files containing larger chunks of data
class save_zip_state
{
	// intenral constants
	static constexpr u32 JSON_EXPAND_CHUNK = 1024 * 1024;
	static constexpr u32 JSON_EXPAND_THRESH = 1024;

public:
	// the size threshold in bytes above which we will write an external file
	static constexpr u32 JSON_EXTERNAL_BINARY_THRESHOLD = 16 * 1024;

	// construction
	save_zip_state();

	// simpler getters
	char const *json_string() { m_json[m_json_offset] = 0; return &m_json[0]; }
	int json_length() const { return m_json_offset; }

	// append a character to the JSON stream
	save_zip_state &json_append(char ch) { 	m_json[m_json_offset++] = ch; return *this; }

	// append an end-of-line sequence to the JSON stream
	save_zip_state &json_append_eol() { return json_append(13).json_append(10); }

	// additional JSON output helpers
	save_zip_state &json_append(char const *buffer);
	save_zip_state &json_append_indent(int count);
	save_zip_state &json_append_name(char const *name);
	save_zip_state &json_append_signed(int64_t value);
	save_zip_state &json_append_unsigned(uint64_t value);
	save_zip_state &json_append_float(double value);

	// stage an item to be output as raw data
	char const *add_data_file(char const *proposed_name, save_registered_item &item, uintptr_t base);

	// commit the results to the given file
	bool commit(emu_file &output);

private:
	// check the reserve; if we're getting close, expand out one more chunk
	void json_check_reserve()
	{
		if (m_json_reserved - m_json_offset < JSON_EXPAND_THRESH)
		{
			m_json_reserved += JSON_EXPAND_CHUNK;
			m_json.resize(m_json_reserved);
		}
	}

	void create_end_of_central_directory(std::vector<u8> &header, u32 central_dir_entries, u64 central_dir_offset, u32 central_dir_size);
	void create_zip_file_header(std::vector<u8> &local, std::vector<u8> &central, char const *filename, u64 local_offset);
	void create_zip_file_footer(std::vector<u8> &local, std::vector<u8> &central, u32 filesize, u32 compressed, u32 crc);
	bool write_data_recursive(zlib_streamer &zlib, save_registered_item &item, uintptr_t base);

	// file_entry represents a single raw data file that will be written
	struct file_entry
	{
		file_entry(char const *name, save_registered_item &item, uintptr_t base) :
			m_item(item),
			m_name(name),
			m_base(base) { }

		save_registered_item &m_item;
		std::string m_name;
		uintptr_t m_base;
		std::vector<u8> m_central_directory;
	};

	// internal state
	std::list<file_entry> m_file_list;
	std::vector<char> m_json;
	util::crc32_creator m_file_crc_accum;
	u32 m_file_size_accum;
	u16 m_archive_date;
	u16 m_archive_time;
	u32 m_json_reserved;
	u32 m_json_offset;
};


// ======================> save_registered_item

// this class manages a single item node in the hierarchy of registered save items
class save_registered_item
{
	// generic types used as proxies for extracting pointers
	using generic_unique = std::unique_ptr<int> const;
	using generic_vector = std::vector<int> const;
	using generic_pointer = void * const;

public:
	// the various types supported
	enum save_type : uint32_t
	{
		TYPE_ARRAY = 0xffff0000, // array is relative, and contains 1 data item that is replicated
		TYPE_CONTAINER,		// container is absolute, and contains absolute data
		TYPE_POINTER,		// pointer is absolute, and contains 1 data item pointed to by the pointer
		TYPE_UNIQUE,		// unique is relative, and contains 1 data item pointed to by std::unique_ptr
		TYPE_VECTOR,		// vector is relative, and contains an array of items pointed to by std::vector
		TYPE_STRUCT,		// struct is relative, and contains a list of relative items
		TYPE_BOOL,			// bool is relative, size 1
		TYPE_INT,			// int is relative, same size as native
		TYPE_UINT,			// uint is relative, same size as native
		TYPE_FLOAT			// float is relative, same size as native
	};

	// root constructor
    save_registered_item();

	// constructor for a new item
    save_registered_item(uintptr_t ptr_offset, save_type type, uint32_t native_size, char const *name);

	// simple getters
	char const *name() const { return m_name.c_str(); }
	save_type type() const { return m_type; }
	bool is_struct_or_container() const { return (m_type == TYPE_STRUCT || m_type == TYPE_CONTAINER); }
	bool is_array() const { return (m_type < TYPE_ARRAY); }
	bool is_int() const { return (m_type == TYPE_INT || m_type == TYPE_UINT); }
	bool is_int_or_float() const { return (is_int() || m_type == TYPE_FLOAT); }
	std::list<save_registered_item> &subitems() { return m_items; }
	uint32_t native_size() const { return m_native_size; }
	uint32_t count() const { return is_array() ? m_type : 1; }
	uintptr_t ptr_offset() const { return m_ptr_offset; }

	// append a new item to the current one
    save_registered_item &append(uintptr_t ptr_offset, save_type type, uint32_t native_size, char const *name);

	// find an item by name
	save_registered_item *find(char const *name);

	// sort subitems by name and prune any empty items
	bool sort_and_prune();

	// update the object base and unwrap trivial items
	bool unwrap_and_update_objbase(uintptr_t &objbase) const;

	// compute the binary size by just saving with a null
	uint64_t compute_binary_size(uintptr_t objbase = 0) const { return save_binary(nullptr, 0, objbase); }

	// save this item and all owned items into a binary form
	uint64_t save_binary(uint8_t *ptr, uint64_t length, uintptr_t objbase = 0) const;

	// restore this item and all owned items from binary form
	uint64_t restore_binary(uint8_t const *ptr, uint64_t length, uintptr_t objbase = 0) const;

	// save this item into a JSON stream
	void save_json(save_zip_state &output, char const *nameprefix = "", int indent = 0, bool inline_form = false, uintptr_t objbase = 0);

	// restore this item from a JSON stream
	void restore_json(std::istringstream &input, uintptr_t objbase = 0);

	// read/write helpers
	uint64_t read_int_unsigned(uintptr_t objbase, int size);
	int64_t read_int_signed(uintptr_t objbase, int size);
	double read_float(uintptr_t objbase, int size);
	void write_int(uintptr_t objbase, int size, uint64_t data);
	void write_float(uintptr_t objbase, int size, double data);

private:
	// internal state
    std::list<save_registered_item> m_items; // list of embedded items
    uintptr_t m_ptr_offset;                  // pointer or offset
	save_type m_type;                        // type of native
    uint32_t m_native_size;                  // native size of item
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
	save_registrar(save_registrar &parent, save_registered_item::save_type type, uint32_t native_size, char const *name, void *baseptr = nullptr, void *containerbase = nullptr) :
		m_parent(parent.parent_item().append((baseptr == nullptr) ? 0 : uintptr_t(baseptr) - parent.m_baseptr, type, native_size, name)),
		m_baseptr((containerbase == nullptr) ? uintptr_t(baseptr) : uintptr_t(containerbase))
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

	// items are considered atoms if they fall into one of the three classes above
	template<typename T> struct is_atom { static constexpr bool value = (is_signed_int_like<T>::value || is_unsigned_int_like<T>::value || is_floating_point_like<T>::value); };

	// construct a container within parent
	save_registrar(save_registrar &parent, char const *name) :
		save_registrar(parent, save_registered_item::TYPE_CONTAINER, 0, name)
	{
	}

	// return a reference to the parent item
	save_registered_item &parent_item() const { return m_parent; }

	// append a bucket
	save_registrar &reg(save_registrar &src, char const *name)
	{
		save_registrar container(*this, save_registered_item::TYPE_CONTAINER, 0, name);
		auto &srcitems = src.parent_item().subitems();
		auto &dstitems = container.parent_item().subitems();
		dstitems.splice(dstitems.end(), srcitems);
        return *this;
	}

    // bool as a special case
    save_registrar &reg(bool &data, char const *name)
	{
		return register_internal(&data, save_registered_item::TYPE_BOOL, sizeof(data), name);
	}

    // signed integral types
    template<typename T>
    std::enable_if_t<is_signed_int_like<T>::value, save_registrar> &reg(T &data, char const *name)
	{
		static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
		return register_internal(&data, save_registered_item::TYPE_INT, sizeof(data), name);
	}

    // unsigned integral types
    template<typename T>
    std::enable_if_t<is_unsigned_int_like<T>::value, save_registrar> &reg(T &data, char const *name)
	{
		static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
		return register_internal(&data, save_registered_item::TYPE_UINT, sizeof(data), name);
	}

    // floating-point types
    template<typename T>
    std::enable_if_t<is_floating_point_like<T>::value, save_registrar> &reg(T &data, char const *name)
	{
		static_assert(sizeof(T) == 4 || sizeof(T) == 8);
		return register_internal(&data, save_registered_item::TYPE_FLOAT, sizeof(data), name);
	}

	// std::unique_ptrs -- these are containers with a single "unique" item within
	template<typename T>
	std::enable_if_t<!std::is_array<T>::value, save_registrar> &reg(std::unique_ptr<T> &data, char const *name)
	{
		if (data.get() == nullptr)
			throw emu_fatalerror("Passed null pointer to save state registration.");

		save_registrar container(*this, save_registered_item::TYPE_UNIQUE, sizeof(data), name, &data, data.get());
		container.reg(*data.get(), name);
		return *this;
	}

    // arrays -- these are containers with a single item representing the underlying data,
	// which is replicated across the whole array
    template<typename T, std::size_t N>
    save_registrar &reg(T (&data)[N], char const *name)
    {
		save_registrar container(*this, save_registered_item::save_type(N), uintptr_t(&data[1]) - uintptr_t(&data[0]), name, &data[0]);
        container.reg(data[0], "");
        return *this;
    }

    // std::arrays -- treat these identically to arrays
    template<typename T, std::size_t N>
    save_registrar &reg(std::array<T, N> &data, char const *name)
    {
		save_registrar container(*this, save_registered_item::save_type(N), uintptr_t(&data[1]) - uintptr_t(&data[0]), name, &data[0]);
        container.reg(data[0], "");
        return *this;
    }

	// pointers -- treat as an array with a specific count
	template<typename T>
	save_registrar &reg(T *data, char const *name, std::size_t count)
	{
		if (data == nullptr)
			throw emu_fatalerror("Passed null pointer to save state registration.");

		save_registrar container(*this, save_registered_item::save_type(count), uintptr_t(&data[1]) - uintptr_t(&data[0]), name, &data[0]);
		container.reg(data[0], "");
		return *this;
	}

    // std::vectors -- these are treated as arrays, but wrapped
    template<typename T>
    save_registrar &reg(std::vector<T> &data, char const *name)
    {
		// skip if nothing
		if (data.size() == 0)
			return *this;

		// create an outer container for the vector
		save_registrar container(*this, save_registered_item::TYPE_VECTOR, sizeof(data), name, &data, &data[0]);

		// then an array container within
		save_registrar subcontainer(container, save_registered_item::save_type(data.size()), uintptr_t(&data[1]) - uintptr_t(&data[0]), name, &data[0]);
		subcontainer.reg(data[0], "");
        return *this;
    }

	// std::unique_ptrs with arrays
	template<typename T>
	save_registrar &reg(std::unique_ptr<T[]> &data, char const *name, std::size_t count)
	{
		// skip if nothing
		if (count == 0)
			return *this;

		if (data.get() == nullptr)
			throw emu_fatalerror("Passed null pointer to save state registration.");

		save_registrar container(*this, save_registered_item::TYPE_UNIQUE, sizeof(data), name, &data, data.get());

		save_registrar subcontainer(container, save_registered_item::save_type(count), uintptr_t(&data[1]) - uintptr_t(&data[0]), name, &data[0]);
		subcontainer.reg(data[0], "");
		return *this;
	}

	// structures & unions (must have a register_save method)
    template<typename T, std::enable_if_t<(std::is_class<T>::value || std::is_union<T>::value) && !std::is_base_of<bitmap_t, T>::value && !is_atom<T>::value, bool> = true>
    save_registrar &reg(T &data, char const *name)
    {
		save_registrar container(*this, save_registered_item::TYPE_STRUCT, sizeof(data), name, &data);
        data.register_save(container);
        return *this;
    }

	// rectangle as a special case, since it's from an external library
    save_registrar &reg(rectangle &data, char const *name)
	{
		save_registrar container(*this, save_registered_item::TYPE_STRUCT, sizeof(data), name, &data);
		container.reg(data.min_x, "min_x")
			.reg(data.max_x, "max_x")
			.reg(data.min_y, "min_y")
			.reg(data.max_y, "max_y");
        return *this;
	}

	// bitmaps as a special case, since they're from an external library
	template<typename BitmapType>
	std::enable_if_t<std::is_base_of<bitmap_t, BitmapType>::value, save_registrar> &reg(BitmapType &data, char const *name)
	{
		save_registrar rows(*this, save_registered_item::save_type(data.height()), data.rowbytes(), name, data.raw_pixptr(0));
		save_registrar cols(rows, save_registered_item::save_type(data.width()), sizeof(BitmapType::pixel_t), "", data.raw_pixptr(0));
		cols.register_internal(data.raw_pixptr(0), save_registered_item::TYPE_UINT, sizeof(BitmapType::pixel_t), "");
		return *this;
	}

private:
	// internal constructor
    save_registrar(save_registered_item &parent, void *baseptr = nullptr) :
        m_parent(parent),
        m_baseptr(uintptr_t(baseptr))
    {
    }

	// internal registration
    save_registrar &register_internal(void *memptr, save_registered_item::save_type type, std::size_t itemsize, char const *itemname)
    {
        m_parent.append(uintptr_t(memptr) - m_baseptr, type, itemsize, itemname);
        return *this;
    }

	// internal state
    save_registered_item &m_parent;
    uintptr_t m_baseptr;
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
