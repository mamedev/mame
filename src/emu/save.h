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

// callback delegate for presave/postload
typedef named_delegate<void ()> save_prepost_delegate;


/// \brief Declare a type as safe to automatically save/restore
///
/// Use this to declare that a given type is a simple, non-pointer type
/// that can be saved and restored.  In general, this should only be
/// be used for specific enum types that have fixed width integer types
/// as their storage classes.
/// \param TYPE The name of the type to declare safe to save.
/// \sa ALLOW_SAVE_TYPE_AND_VECTOR
#define ALLOW_SAVE_TYPE(TYPE) \
	template <> struct save_manager::is_atom<TYPE> : public std::true_type { };

/// \brief Declare a type as safe to automatically save/restore,
///   including in \c std::vector instances
///
/// Used the same way as #ALLOW_SAVE_TYPE, but also declares that
/// \c std::vector instances containing the type are safe to save.
/// that can be saved and restored.  This must not be used if
/// \c std::vector is specialised in an incompatible way.
/// \param TYPE The name of the type to declare safe to save.
/// \sa ALLOW_SAVE_TYPE
#define ALLOW_SAVE_TYPE_AND_VECTOR(TYPE) \
	ALLOW_SAVE_TYPE(TYPE) \
	template <> struct save_manager::is_vector_safe<TYPE> : public std::true_type { };

/// \brief Helper for referring to members of structures in arrays
///
/// Expands to the necessary reference, pointer to member and name to
/// refer to a structure member.
/// \param s Reference to a C array or \c std::array of structures.
/// \param m Name of the structure member to refer to.
#define STRUCT_MEMBER(s, m) s, &save_manager::pointer_unwrap<decltype(s)>::underlying_type::m, #s "." #m


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ram_state;
class rewinder;

class save_manager
{
	// stuff for working with arrays
	template <typename T> struct array_unwrap
	{
		using underlying_type = T;
		static constexpr std::size_t SAVE_COUNT = 1U;
		static constexpr std::size_t SIZE = sizeof(underlying_type);
		static underlying_type *ptr(T &value) { return &value; }
	};
	template <typename T, std::size_t N> struct array_unwrap<T [N]>
	{
		using underlying_type = typename array_unwrap<T>::underlying_type;
		static constexpr std::size_t SAVE_COUNT = N * array_unwrap<T>::SAVE_COUNT;
		static constexpr std::size_t SIZE = sizeof(underlying_type);
		static underlying_type *ptr(T (&value)[N]) { return array_unwrap<T>::ptr(value[0]); }
	};
	template <typename T, std::size_t N> struct array_unwrap<std::array<T, N> >
	{
		using underlying_type = typename array_unwrap<T>::underlying_type;
		static constexpr std::size_t SAVE_COUNT = N * array_unwrap<T>::SAVE_COUNT;
		static constexpr std::size_t SIZE = sizeof(underlying_type);
		static underlying_type *ptr(std::array<T, N> &value) { return array_unwrap<T>::ptr(value[0]); }
	};

	// set of templates to identify valid save types
	template <typename ItemType> struct is_atom : public std::false_type { };
	template <typename ItemType> struct is_vector_safe : public std::false_type { };

	class state_entry
	{
	public:
		// construction/destruction
		state_entry(void *data, std::string &&name, device_t *device, std::string &&module, std::string &&tag, int index, u8 size, u32 valcount, u32 blockcount, u32 stride);

		// helpers
		void flip_data();

		// state
		void *          m_data;                 // pointer to the memory to save/restore
		std::string     m_name;                 // full name
		device_t *      m_device;               // associated device, nullptr if none
		std::string     m_module;               // module name
		std::string     m_tag;                  // tag name
		int             m_index;                // index
		u8              m_typesize;             // size of the raw data type
		u32             m_typecount;            // number of items in each block
		u32             m_blockcount;           // number of blocks of items
		u32             m_stride;               // stride between blocks of items in units of item size
	};

	friend class ram_state;
	friend class rewinder;

public:
	// stuff to allow STRUCT_MEMBER to work with pointers
	template <typename T> struct pointer_unwrap { using underlying_type = typename array_unwrap<T>::underlying_type; };
	template <typename T> struct pointer_unwrap<T &> { using underlying_type = typename pointer_unwrap<std::remove_cv_t<T> >::underlying_type; };
	template <typename T> struct pointer_unwrap<T *> { using underlying_type = typename array_unwrap<T>::underlying_type; };
	template <typename T> struct pointer_unwrap<std::unique_ptr<T []> > { using underlying_type = typename array_unwrap<T>::underlying_type; };

	// construction/destruction
	save_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	rewinder *rewind() { return m_rewind.get(); }
	int registration_count() const { return m_entry_list.size(); }
	bool registration_allowed() const { return m_reg_allowed; }

	// registration control
	void allow_registration(bool allowed = true);
	const char *indexed_item(int index, void *&base, u32 &valsize, u32 &valcount, u32 &blockcount, u32 &stride) const;

	// function registration
	void register_presave(save_prepost_delegate func);
	void register_postload(save_prepost_delegate func);

	// callback dispatching
	void dispatch_presave();
	void dispatch_postload();

	// generic memory registration
	void save_memory(device_t *device, const char *module, const char *tag, u32 index, const char *name, void *val, u32 valsize, u32 valcount = 1, u32 blockcount = 1, u32 stride = 0);

	// templatized wrapper for general objects and arrays
	template <typename ItemType>
	std::enable_if_t<is_atom<typename array_unwrap<ItemType>::underlying_type>::value> save_item(device_t *device, const char *module, const char *tag, int index, ItemType &value, const char *valname)
	{
		static_assert(!std::is_pointer<ItemType>::value, "Called save_item on a pointer with no count!");
		save_memory(device, module, tag, index, valname, array_unwrap<ItemType>::ptr(value), array_unwrap<ItemType>::SIZE, array_unwrap<ItemType>::SAVE_COUNT);
	}

	// templatized wrapper for structure members
	template <typename ItemType, typename StructType, typename ElementType>
	void save_item(device_t *device, const char *module, const char *tag, int index, ItemType &value, ElementType StructType::*element, const char *valname)
	{
		static_assert(std::is_base_of<StructType, typename array_unwrap<ItemType>::underlying_type>::value, "Called save_item on a non-matching struct member pointer!");
		static_assert(!std::is_pointer<ElementType>::value, "Called save_item on a struct member pointer!");
		static_assert(is_atom<typename array_unwrap<ElementType>::underlying_type>::value, "Called save_item on a non-fundamental type!");
		save_memory(device, module, tag, index, valname, array_unwrap<ElementType>::ptr(array_unwrap<ItemType>::ptr(value)->*element), array_unwrap<ElementType>::SIZE, array_unwrap<ElementType>::SAVE_COUNT, array_unwrap<ItemType>::SAVE_COUNT, sizeof(typename array_unwrap<ItemType>::underlying_type));
	}

	// templatized wrapper for pointers
	template <typename ItemType>
	std::enable_if_t<is_atom<typename array_unwrap<ItemType>::underlying_type>::value> save_pointer(device_t *device, const char *module, const char *tag, int index, ItemType *value, const char *valname, u32 count)
	{
		save_memory(device, module, tag, index, valname, array_unwrap<ItemType>::ptr(value[0]), array_unwrap<ItemType>::SIZE, array_unwrap<ItemType>::SAVE_COUNT * count);
	}

	template <typename ItemType, typename StructType, typename ElementType>
	void save_pointer(device_t *device, const char *module, const char *tag, int index, ItemType *value, ElementType StructType::*element, const char *valname, u32 count)
	{
		static_assert(std::is_base_of<StructType, typename array_unwrap<ItemType>::underlying_type>::value, "Called save_pointer on a non-matching struct member pointer!");
		static_assert(!std::is_pointer<ElementType>::value, "Called save_pointer on a struct member pointer!");
		static_assert(is_atom<typename array_unwrap<ElementType>::underlying_type>::value, "Called save_pointer on a non-fundamental type!");
		save_memory(device, module, tag, index, valname, array_unwrap<ElementType>::ptr(array_unwrap<ItemType>::ptr(value[0])->*element), array_unwrap<ElementType>::SIZE, array_unwrap<ElementType>::SAVE_COUNT, array_unwrap<ItemType>::SAVE_COUNT * count, sizeof(typename array_unwrap<ItemType>::underlying_type));
	}

	// templatized wrapper for std::unique_ptr
	template <typename ItemType>
	std::enable_if_t<is_atom<typename array_unwrap<ItemType>::underlying_type>::value> save_pointer(device_t *device, const char *module, const char *tag, int index, const std::unique_ptr<ItemType []> &value, const char *valname, u32 count)
	{
		save_memory(device, module, tag, index, valname, array_unwrap<ItemType>::ptr(value[0]), array_unwrap<ItemType>::SIZE, array_unwrap<ItemType>::SAVE_COUNT * count);
	}

	template <typename ItemType, typename StructType, typename ElementType>
	void save_pointer(device_t *device, const char *module, const char *tag, int index, const std::unique_ptr<ItemType []> &value, ElementType StructType::*element, const char *valname, u32 count)
	{
		static_assert(std::is_base_of<StructType, typename array_unwrap<ItemType>::underlying_type>::value, "Called save_pointer on a non-matching struct member pointer!");
		static_assert(!std::is_pointer<ElementType>::value, "Called save_pointer on a struct member pointer!");
		static_assert(is_atom<typename array_unwrap<ElementType>::underlying_type>::value, "Called save_pointer on a non-fundamental type!");
		save_memory(device, module, tag, index, valname, array_unwrap<ElementType>::ptr(array_unwrap<ItemType>::ptr(value[0])->*element), array_unwrap<ElementType>::SIZE, array_unwrap<ElementType>::SAVE_COUNT, array_unwrap<ItemType>::SAVE_COUNT * count, sizeof(typename array_unwrap<ItemType>::underlying_type));
	}

	// templatized wrapper for std::vector
	template <typename ItemType>
	std::enable_if_t<is_vector_safe<typename array_unwrap<ItemType>::underlying_type>::value> save_item(device_t *device, const char *module, const char *tag, int index, std::vector<ItemType> &value, const char *valname)
	{
		save_pointer(device, module, tag, index, &value[0], valname, value.size());
	}

	// specializations for bitmaps
	void save_item(device_t *device, const char *module, const char *tag, int index, bitmap_ind8 &value, const char *valname)
	{
		save_memory(device, module, tag, index, valname, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
	}

	void save_item(device_t *device, const char *module, const char *tag, int index, bitmap_ind16 &value, const char *valname)
	{
		save_memory(device, module, tag, index, valname, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
	}

	void save_item(device_t *device, const char *module, const char *tag, int index, bitmap_ind32 &value, const char *valname)
	{
		save_memory(device, module, tag, index, valname, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
	}

	void save_item(device_t *device, const char *module, const char *tag, int index, bitmap_rgb32 &value, const char *valname)
	{
		save_memory(device, module, tag, index, valname, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
	}

	// specializations for attotimes
	template <typename ItemType>
	std::enable_if_t<std::is_same<typename save_manager::array_unwrap<ItemType>::underlying_type, attotime>::value> save_item(device_t *device, const char *module, const char *tag, int index, ItemType &value, const char *valname)
	{
		std::string tempstr;
		tempstr.assign(valname).append(".attoseconds");
		save_item(device, module, tag, index, value, &attotime::m_attoseconds, tempstr.c_str());
		tempstr.assign(valname).append(".seconds");
		save_item(device, module, tag, index, value, &attotime::m_seconds, tempstr.c_str());
	}

	template <typename ItemType>
	std::enable_if_t<std::is_same<typename save_manager::array_unwrap<ItemType>::underlying_type, attotime>::value> save_pointer(device_t *device, const char *module, const char *tag, int index, ItemType *value, const char *valname, u32 count)
	{
		std::string tempstr;
		tempstr.assign(valname).append(".attoseconds");
		save_item(device, module, tag, index, value, &attotime::m_attoseconds, tempstr.c_str(), count);
		tempstr.assign(valname).append(".seconds");
		save_item(device, module, tag, index, value, &attotime::m_seconds, tempstr.c_str(), count);
	}

	template <typename ItemType>
	std::enable_if_t<std::is_same<typename save_manager::array_unwrap<ItemType>::underlying_type, attotime>::value> save_pointer(device_t *device, const char *module, const char *tag, int index, const std::unique_ptr<ItemType []> &value, const char *valname, u32 count)
	{
		std::string tempstr;
		tempstr.assign(valname).append(".attoseconds");
		save_item(device, module, tag, index, value, &attotime::m_attoseconds, tempstr.c_str(), count);
		tempstr.assign(valname).append(".seconds");
		save_item(device, module, tag, index, value, &attotime::m_seconds, tempstr.c_str(), count);
	}

	// global memory registration
	template <typename ItemType>
	void save_item(ItemType &value, const char *valname, int index = 0)
	{ save_item(nullptr, "global", nullptr, index, value, valname); }
	template <typename ItemType, typename StructType, typename ElementType>
	void save_item(ItemType &value, ElementType StructType::*element, const char *valname, int index = 0)
	{ save_item(nullptr, "global", nullptr, index, value, element, valname); }
	template <typename ItemType>
	void save_pointer(ItemType &&value, const char *valname, u32 count, int index = 0)
	{ save_pointer(nullptr, "global", nullptr, index, std::forward<ItemType>(value), valname, count); }
	template <typename ItemType, typename StructType, typename ElementType>
	void save_pointer(ItemType &&value, ElementType StructType::*element, const char *valname, u32 count, int index = 0)
	{ save_pointer(nullptr, "global", nullptr, index, std::forward<ItemType>(value), element, valname, count); }

	// file processing
	static save_error check_file(running_machine &machine, util::core_file &file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...));
	save_error write_file(util::core_file &file);
	save_error read_file(util::core_file &file);

	save_error write_stream(std::ostream &str);
	save_error read_stream(std::istream &str);

	save_error write_buffer(void *buf, size_t size);
	save_error read_buffer(const void *buf, size_t size);

private:
	// state callback item
	class state_callback
	{
	public:
		// construction/destruction
		state_callback(save_prepost_delegate callback);

		save_prepost_delegate m_func;                 // delegate
	};

	// internal helpers
	template <typename T, typename U, typename V, typename W>
	save_error do_write(T check_space, U write_block, V start_header, W start_data);
	template <typename T, typename U, typename V, typename W>
	save_error do_read(T check_length, U read_block, V start_header, W start_data);
	u32 signature() const;
	void dump_registry() const;
	static save_error validate_header(const u8 *header, const char *gamename, u32 signature, void (CLIB_DECL *errormsg)(const char *fmt, ...), const char *error_prefix);

	// internal state
	running_machine &         m_machine;              // reference to our machine
	std::unique_ptr<rewinder> m_rewind;               // rewinder
	bool                      m_reg_allowed;          // are registrations allowed?
	s32                       m_illegal_regs;         // number of illegal registrations

	std::vector<std::unique_ptr<state_entry>>    m_entry_list;       // list of registered entries
	std::vector<std::unique_ptr<ram_state>>      m_ramstate_list;    // list of ram states
	std::vector<std::unique_ptr<state_callback>> m_presave_list;     // list of pre-save functions
	std::vector<std::unique_ptr<state_callback>> m_postload_list;    // list of post-load functions
};

class ram_state
{
	save_manager &     m_save;                        // reference to save_manager
	util::vectorstream m_data;                        // save data buffer

public:
	bool               m_valid;                       // can we load this state?
	attotime           m_time;                        // machine timestamp

	ram_state(save_manager &save);
	static size_t get_size(save_manager &save);
	save_error save();
	save_error load();
};

class rewinder
{
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

public:
	rewinder(save_manager &save);
	bool enabled() { return m_enabled; }
	void clamp_capacity();
	void invalidate();
	bool capture();
	bool step();
};


// template specializations to enumerate the fundamental atomic types you are allowed to save
ALLOW_SAVE_TYPE_AND_VECTOR(char)
ALLOW_SAVE_TYPE           (bool) // std::vector<bool> may be packed internally
ALLOW_SAVE_TYPE_AND_VECTOR(osd::s8)
ALLOW_SAVE_TYPE_AND_VECTOR(osd::u8)
ALLOW_SAVE_TYPE_AND_VECTOR(osd::s16)
ALLOW_SAVE_TYPE_AND_VECTOR(osd::u16)
ALLOW_SAVE_TYPE_AND_VECTOR(osd::s32)
ALLOW_SAVE_TYPE_AND_VECTOR(osd::u32)
ALLOW_SAVE_TYPE_AND_VECTOR(osd::s64)
ALLOW_SAVE_TYPE_AND_VECTOR(osd::u64)
ALLOW_SAVE_TYPE_AND_VECTOR(PAIR)
ALLOW_SAVE_TYPE_AND_VECTOR(PAIR64)
ALLOW_SAVE_TYPE_AND_VECTOR(float)
ALLOW_SAVE_TYPE_AND_VECTOR(double)
ALLOW_SAVE_TYPE_AND_VECTOR(endianness_t)
ALLOW_SAVE_TYPE_AND_VECTOR(rgb_t)


#endif // MAME_EMU_SAVE_H
