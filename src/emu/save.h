/***************************************************************************

    save.h

    Save state management functions.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __SAVE_H__
#define __SAVE_H__



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum save_error
{
	STATERR_NONE,
	STATERR_ILLEGAL_REGISTRATIONS,
	STATERR_INVALID_HEADER,
	STATERR_READ_ERROR,
	STATERR_WRITE_ERROR
};



//**************************************************************************
//  MACROS
//**************************************************************************

// callback delegate for presave/postload
typedef delegate<void ()> save_prepost_delegate;


// use this to declare a given type is a simple, non-pointer type that can be
// saved; in general, this is intended only to be used for specific enum types
// defined by your device
#define ALLOW_SAVE_TYPE(TYPE) template<> struct save_manager::type_checker<TYPE> { static const bool is_atom = true; static const bool is_pointer = false; }



// register items with explicit tags
#define state_save_register_item(_mach, _mod, _tag, _index, _val) \
	(_mach).save().save_item(_mod, _tag, _index, _val, #_val)

#define state_save_register_item_pointer(_mach, _mod, _tag, _index, _val, _count) \
	(_mach).save().save_pointer(_mod, _tag, _index, _val, #_val, _count)

#define state_save_register_item_array(_mach, _mod, _tag, _index, _val) \
	(_mach).save().save_item(_mod, _tag, _index, _val, #_val)

#define state_save_register_item_2d_array(_mach, _mod, _tag, _index, _val) \
	(_mach).save().save_item(_mod, _tag, _index, _val, #_val)

#define state_save_register_item_bitmap(_mach, _mod, _tag, _index, _val) \
	(_mach).save().save_item(_mod, _tag, _index, *(_val), #_val)



// register global items
#define state_save_register_global(_mach, _val) \
	(_mach).save().save_item(_val, #_val)

#define state_save_register_global_pointer(_mach, _val, _count) \
	(_mach).save().save_pointer(_val, #_val, _count)

#define state_save_register_global_array(_mach, _val) \
	(_mach).save().save_item(_val, #_val)

#define state_save_register_global_2d_array(_mach, _val) \
	(_mach).save().save_item(_val, #_val)

#define state_save_register_global_bitmap(_mach, _val) \
	(_mach).save().save_item(*(_val), #_val)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class save_manager
{
	// type_checker is a set of templates to identify valid save types
	template<typename _ItemType> struct type_checker { static const bool is_atom = false; static const bool is_pointer = false; };
	template<typename _ItemType> struct type_checker<_ItemType*> { static const bool is_atom = false; static const bool is_pointer = true; };

public:
	// construction/destruction
	save_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	int registration_count() const { return m_entry_list.count(); }
	bool registration_allowed() const { return m_reg_allowed; }

	// registration control
	void allow_registration(bool allowed = true);
	const char *indexed_item(int index, void *&base, UINT32 &valsize, UINT32 &valcount) const;

	// function registration
	void register_presave(save_prepost_delegate func);
	void register_postload(save_prepost_delegate func);

	// generic memory registration
	void save_memory(const char *module, const char *tag, UINT32 index, const char *name, void *val, UINT32 valsize, UINT32 valcount = 1);

	// templatized wrapper for general objects
	template<typename _ItemType>
	void save_item(const char *module, const char *tag, int index, _ItemType &value, const char *valname)
	{
		if (type_checker<_ItemType>::is_pointer) throw emu_fatalerror("Called save_item on a pointer with no count!");
		if (!type_checker<_ItemType>::is_atom) throw emu_fatalerror("Called save_item on a non-fundamental type!");
		save_memory(module, tag, index, valname, &value, sizeof(value));
	}

	// templatized wrapper for 1-dimensional arrays
	template<typename _ItemType, std::size_t N>
	void save_item(const char *module, const char *tag, int index, _ItemType (&value)[N], const char *valname)
	{
		if (!type_checker<_ItemType>::is_atom) throw emu_fatalerror("Called save_item on a non-fundamental type!");
		save_memory(module, tag, index, valname, &value[0], sizeof(value[0]), N);
	}

	// templatized wrapper for 2-dimensional arrays
	template<typename _ItemType, std::size_t M, std::size_t N>
	void save_item(const char *module, const char *tag, int index, _ItemType (&value)[M][N], const char *valname)
	{
		if (!type_checker<_ItemType>::is_atom) throw emu_fatalerror("Called save_item on a non-fundamental type!");
		save_memory(module, tag, index, valname, &value[0][0], sizeof(value[0][0]), M * N);
	}

	// templatized wrapper for pointers
	template<typename _ItemType>
	void save_pointer(const char *module, const char *tag, int index, _ItemType *value, const char *valname, UINT32 count)
	{
		if (!type_checker<_ItemType>::is_atom) throw emu_fatalerror("Called save_item on a non-fundamental type!");
		save_memory(module, tag, index, valname, value, sizeof(*value), count);
	}

	// global memory registration
	template<typename _ItemType>
	void save_item(_ItemType &value, const char *valname, int index = 0) { save_item("global", NULL, index, value, valname); }
	template<typename _ItemType>
	void save_pointer(_ItemType *value, const char *valname, UINT32 count, int index = 0) { save_pointer("global", NULL, index, value, valname, count); }

	// file processing
	static save_error check_file(running_machine &machine, emu_file &file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...));
	save_error write_file(emu_file &file);
	save_error read_file(emu_file &file);

private:
	// internal helpers
	UINT32 signature() const;
	void dump_registry() const;
	static save_error validate_header(const UINT8 *header, const char *gamename, UINT32 signature, void (CLIB_DECL *errormsg)(const char *fmt, ...), const char *error_prefix);

	// state callback item
	class state_callback
	{
	public:
		// construction/destruction
		state_callback(save_prepost_delegate callback);

		// getters
		state_callback *next() const { return m_next; }

		// state
		state_callback *    m_next;                 // pointer to next entry
		save_prepost_delegate m_func;               // delegate
	};

	class state_entry
	{
	public:
		// construction/destruction
		state_entry(void *data, const char *name, UINT8 size, UINT32 count);

		// getters
		state_entry *next() const { return m_next; }

		// helpers
		void flip_data();

		// state
		state_entry *       m_next;                 // pointer to next entry
		void *              m_data;                 // pointer to the memory to save/restore
		astring             m_name;                 // full name
		UINT8               m_typesize;             // size of the raw data type
		UINT32              m_typecount;            // number of items
		UINT32              m_offset;               // offset within the final structure
	};

	// internal state
	running_machine &       m_machine;              // reference to our machine
	bool                    m_reg_allowed;          // are registrations allowed?
	int                     m_illegal_regs;         // number of illegal registrations

	simple_list<state_entry> m_entry_list;          // list of reigstered entries
	simple_list<state_callback> m_presave_list;     // list of pre-save functions
	simple_list<state_callback> m_postload_list;    // list of post-load functions
};


// template specializations to enumerate the fundamental atomic types you are allowed to save
ALLOW_SAVE_TYPE(bool);
ALLOW_SAVE_TYPE(INT8);
ALLOW_SAVE_TYPE(UINT8);
ALLOW_SAVE_TYPE(INT16);
ALLOW_SAVE_TYPE(UINT16);
ALLOW_SAVE_TYPE(INT32);
ALLOW_SAVE_TYPE(UINT32);
ALLOW_SAVE_TYPE(INT64);
ALLOW_SAVE_TYPE(UINT64);
ALLOW_SAVE_TYPE(PAIR);
ALLOW_SAVE_TYPE(PAIR64);
ALLOW_SAVE_TYPE(float);
ALLOW_SAVE_TYPE(double);
ALLOW_SAVE_TYPE(endianness_t);



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  save_item - specialized save_item for bitmaps
//-------------------------------------------------

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, bitmap_ind8 &value, const char *name)
{
	save_memory(module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, bitmap_ind16 &value, const char *name)
{
	save_memory(module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, bitmap_ind32 &value, const char *name)
{
	save_memory(module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, bitmap_rgb32 &value, const char *name)
{
	save_memory(module, tag, index, name, &value.pix(0), value.bpp() / 8, value.rowpixels() * value.height());
}


//-------------------------------------------------
//  save_item - specialized save_item for attotimes
//-------------------------------------------------

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, attotime &value, const char *name)
{
	astring tempstr(name, ".attoseconds");
	save_memory(module, tag, index, tempstr, &value.attoseconds, sizeof(value.attoseconds));
	tempstr.cpy(name).cat(".seconds");
	save_memory(module, tag, index, tempstr, &value.seconds, sizeof(value.seconds));
}


//-------------------------------------------------
//  save_item - specialized save_item for
//  dynamic_arrays
//-------------------------------------------------

// surely there must be a syntax for doing this templated??
template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, dynamic_array<UINT8> &value, const char *name)
{
	save_memory(module, tag, index, name, &value[0], sizeof(UINT8), value.count());
}

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, dynamic_array<UINT16> &value, const char *name)
{
	save_memory(module, tag, index, name, &value[0], sizeof(UINT16), value.count());
}

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, dynamic_array<UINT32> &value, const char *name)
{
	save_memory(module, tag, index, name, &value[0], sizeof(UINT32), value.count());
}

template<>
inline void save_manager::save_item(const char *module, const char *tag, int index, dynamic_array<UINT64> &value, const char *name)
{
	save_memory(module, tag, index, name, &value[0], sizeof(UINT64), value.count());
}


#endif  /* __SAVE_H__ */
