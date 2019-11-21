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


// use this to declare a given type is a simple, non-pointer type that can be
// saved; in general, this is intended only to be used for specific enum types
// defined by your device
#define ALLOW_SAVE_TYPE(TYPE) \
	template<> struct save_manager::type_checker<TYPE> { static const bool is_atom = true; static const bool is_pointer = false; }

// use this as above, but also to declare that std::vector<TYPE> is safe as well
#define ALLOW_SAVE_TYPE_AND_ARRAY(TYPE) \
	ALLOW_SAVE_TYPE(TYPE); \
	template<> inline void save_manager::save_item(device_t *device, const char *module, const char *tag, int index, std::vector<TYPE> &value, const char *name) { save_memory(device, module, tag, index, name, &value[0], sizeof(TYPE), value.size()); }


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class state_entry
{
public:
	// construction/destruction
	state_entry(void *data, const char *name, device_t *device, const char *module, const char *tag, int index, u8 size, u32 count);

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
	u32             m_typecount;            // number of items
	u32             m_offset;               // offset within the final structure
};

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
	template <typename T, std::size_t N> struct array_unwrap<T[N]>
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

	// type_checker is a set of templates to identify valid save types
	template<typename ItemType> struct type_checker { static const bool is_atom = false; static const bool is_pointer = false; };
	template<typename ItemType> struct type_checker<ItemType*> { static const bool is_atom = false; static const bool is_pointer = true; };

	friend class ram_state;
	friend class rewinder;

public:
	// construction/destruction
	save_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	rewinder *rewind() { return m_rewind.get(); }
	int registration_count() const { return m_entry_list.size(); }
	bool registration_allowed() const { return m_reg_allowed; }

	// registration control
	void allow_registration(bool allowed = true);
	const char *indexed_item(int index, void *&base, u32 &valsize, u32 &valcount) const;

	// function registration
	void register_presave(save_prepost_delegate func);
	void register_postload(save_prepost_delegate func);

	// callback dispatching
	void dispatch_presave();
	void dispatch_postload();

	// generic memory registration
	void save_memory(device_t *device, const char *module, const char *tag, u32 index, const char *name, void *val, u32 valsize, u32 valcount = 1);

	// templatized wrapper for general objects and arrays
	template<typename ItemType>
	void save_item(device_t *device, const char *module, const char *tag, int index, ItemType &value, const char *valname)
	{
		if (type_checker<ItemType>::is_pointer)
			throw emu_fatalerror("Called save_item on a pointer with no count!");
		if (!type_checker<typename array_unwrap<ItemType>::underlying_type>::is_atom)
			throw emu_fatalerror("Called save_item on a non-fundamental type, name %s!", valname);
		save_memory(device, module, tag, index, valname, array_unwrap<ItemType>::ptr(value), array_unwrap<ItemType>::SIZE, array_unwrap<ItemType>::SAVE_COUNT);
	}

	// templatized wrapper for pointers
	template<typename ItemType>
	void save_pointer(device_t *device, const char *module, const char *tag, int index, ItemType *value, const char *valname, u32 count)
	{
		if (!type_checker<typename array_unwrap<ItemType>::underlying_type>::is_atom)
			throw emu_fatalerror("Called save_item on a non-fundamental type, name %s!", valname);
		save_memory(device, module, tag, index, valname, array_unwrap<ItemType>::ptr(value[0]), array_unwrap<ItemType>::SIZE, array_unwrap<ItemType>::SAVE_COUNT * count);
	}

	// templatized wrapper for std::unique_ptr
	template<typename ItemType>
	void save_pointer(device_t *device, const char *module, const char *tag, int index, std::unique_ptr<ItemType[]> &value, const char *valname, u32 count)
	{
		if (!type_checker<typename array_unwrap<ItemType>::underlying_type>::is_atom)
			throw emu_fatalerror("Called save_item on a non-fundamental type, name %s!", valname);
		save_memory(device, module, tag, index, valname, array_unwrap<ItemType>::ptr(value[0]), array_unwrap<ItemType>::SIZE, array_unwrap<ItemType>::SAVE_COUNT * count);
	}

	// global memory registration
	template<typename ItemType>
	void save_item(ItemType &value, const char *valname, int index = 0) { save_item(nullptr, "global", nullptr, index, value, valname); }
	template<typename ItemType>
	void save_pointer(ItemType *value, const char *valname, u32 count, int index = 0) { save_pointer(nullptr, "global", nullptr, index, value, valname, count); }

	// file processing
	static save_error check_file(running_machine &machine, emu_file &file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...));
	save_error write_file(emu_file &file);
	save_error read_file(emu_file &file);

private:
	// internal helpers
	u32 signature() const;
	void dump_registry() const;
	static save_error validate_header(const u8 *header, const char *gamename, u32 signature, void (CLIB_DECL *errormsg)(const char *fmt, ...), const char *error_prefix);

	// state callback item
	class state_callback
	{
	public:
		// construction/destruction
		state_callback(save_prepost_delegate callback);

		save_prepost_delegate m_func;                 // delegate
	};

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
ALLOW_SAVE_TYPE_AND_ARRAY(char)
ALLOW_SAVE_TYPE          (bool); // std::vector<bool> may be packed internally
ALLOW_SAVE_TYPE_AND_ARRAY(osd::s8)
ALLOW_SAVE_TYPE_AND_ARRAY(osd::u8)
ALLOW_SAVE_TYPE_AND_ARRAY(osd::s16)
ALLOW_SAVE_TYPE_AND_ARRAY(osd::u16)
ALLOW_SAVE_TYPE_AND_ARRAY(osd::s32)
ALLOW_SAVE_TYPE_AND_ARRAY(osd::u32)
ALLOW_SAVE_TYPE_AND_ARRAY(osd::s64)
ALLOW_SAVE_TYPE_AND_ARRAY(osd::u64)
ALLOW_SAVE_TYPE_AND_ARRAY(PAIR)
ALLOW_SAVE_TYPE_AND_ARRAY(PAIR64)
ALLOW_SAVE_TYPE_AND_ARRAY(float)
ALLOW_SAVE_TYPE_AND_ARRAY(double)
ALLOW_SAVE_TYPE_AND_ARRAY(endianness_t)
ALLOW_SAVE_TYPE_AND_ARRAY(rgb_t)



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  save_item - specialized save_item for bitmaps
//-------------------------------------------------

template<>
inline void save_manager::save_item(device_t *device, const char *module, const char *tag, int index, bitmap_ind8 &value, const char *name)
{
	save_memory(device, module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}

template<>
inline void save_manager::save_item(device_t *device, const char *module, const char *tag, int index, bitmap_ind16 &value, const char *name)
{
	save_memory(device, module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}

template<>
inline void save_manager::save_item(device_t *device, const char *module, const char *tag, int index, bitmap_ind32 &value, const char *name)
{
	save_memory(device, module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}

template<>
inline void save_manager::save_item(device_t *device, const char *module, const char *tag, int index, bitmap_rgb32 &value, const char *name)
{
	save_memory(device, module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}


//-------------------------------------------------
//  save_item - specialized save_item for attotimes
//-------------------------------------------------

template<>
inline void save_manager::save_item(device_t *device, const char *module, const char *tag, int index, attotime &value, const char *name)
{
	std::string tempstr = std::string(name).append(".attoseconds");
	save_memory(device, module, tag, index, tempstr.c_str(), &value.m_attoseconds, sizeof(value.m_attoseconds));
	tempstr.assign(name).append(".seconds");
	save_memory(device, module, tag, index, tempstr.c_str(), &value.m_seconds, sizeof(value.m_seconds));
}


#endif  /* MAME_EMU_SAVE_H */
