// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcbeut.h

    Utility functions for dynamic recompiling backends.

***************************************************************************/

#ifndef MAME_CPU_DRCBEUT_H
#define MAME_CPU_DRCBEUT_H

#pragma once

#include "drcuml.h"

#include "mfpresolve.h"

#include <utility>
#include <vector>


namespace drc {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> drc_hash_table

// common hash table management
class drc_hash_table
{
public:
	// construction/destruction
	drc_hash_table(drc_cache &cache, uint32_t modes, uint8_t addrbits, uint8_t ignorebits);

	// getters
	drccodeptr ***base() const { return m_base; }
	uint8_t l1bits() const { return m_l1bits; }
	uint8_t l2bits() const { return m_l2bits; }
	uint8_t l1shift() const { return m_l1shift; }
	uint8_t l2shift() const { return m_l2shift; }
	offs_t l1mask() const { return m_l1mask; }
	offs_t l2mask() const { return m_l2mask; }
	bool is_mode_populated(uint32_t mode) const { return m_base[mode] != m_emptyl1; }

	// set up and configuration
	bool reset();
	void set_default_codeptr(drccodeptr code);

	// block begin/end
	void block_begin(drcuml_block &block, const uml::instruction *instlist, uint32_t numinst);
	void block_end(drcuml_block &block);

	// code pointer access
	bool set_codeptr(uint32_t mode, uint32_t pc, drccodeptr code);
	drccodeptr get_codeptr(uint32_t mode, uint32_t pc) { assert(mode < m_modes); return m_base[mode][(pc >> m_l1shift) & m_l1mask][(pc >> m_l2shift) & m_l2mask]; }
	bool code_exists(uint32_t mode, uint32_t pc) { return get_codeptr(mode, pc) != m_nocodeptr; }

private:
	// internal state
	drc_cache &     m_cache;                // cache where allocations come from
	uint32_t        m_modes;                // number of modes supported

	drccodeptr      m_nocodeptr;            // pointer to code which will handle missing entries

	uint8_t         m_l1bits;               // bits worth of entries in l1 hash tables
	uint8_t         m_l2bits;               // bits worth of entries in l2 hash tables
	uint8_t         m_l1shift;              // shift to apply to the PC to get the l1 hash entry
	uint8_t         m_l2shift;              // shift to apply to the PC to get the l2 hash entry
	offs_t          m_l1mask;               // mask to apply after shifting
	offs_t          m_l2mask;               // mask to apply after shifting

	drccodeptr ***  m_base;                 // pointer to the l1 table for each mode
	drccodeptr **   m_emptyl1;              // pointer to empty l1 hash table
	drccodeptr *    m_emptyl2;              // pointer to empty l2 hash table
};


// ======================> drc_map_variables

// common map variable management
class drc_map_variables
{
public:
	// construction/destruction
	drc_map_variables(drc_cache &cache, uint64_t uniquevalue);
	~drc_map_variables();

	// block begin/end
	void block_begin(drcuml_block &block);
	void block_end(drcuml_block &block);

	// get/set values
	void set_value(drccodeptr codebase, uint32_t mapvar, uint32_t newvalue);
	uint32_t get_value(drccodeptr codebase, uint32_t mapvar) const;
	uint32_t get_last_value(uint32_t mapvar);

	// static accessors to be called directly by generated code
	static uint32_t static_get_value(drc_map_variables &map, drccodeptr codebase, uint32_t mapvar);

private:
	// internal state
	drc_cache &         m_cache;            // pointer to the cache
	uint64_t            m_uniquevalue;      // unique value used to find the table
	uint32_t            m_mapvalue[uml::MAPVAR_END - uml::MAPVAR_M0]; // array of current values

	// list of entries
	struct map_entry
	{
		map_entry *next() const { return m_next; }
		map_entry *     m_next;             // pointer to next map entry
		drccodeptr      m_codeptr;          // pointer to the relevant code
		uint32_t        m_mapvar;           // map variable id
		uint32_t        m_newval;           // value of the variable starting at codeptr
	};
	simple_list<map_entry> m_entry_list;    // list of entries
};


// ======================> drc_label_list

typedef delegate<void (void *, drccodeptr)> drc_label_fixup_delegate;

// structure holding a live list of labels
class drc_label_list
{
public:
	// construction/destruction
	drc_label_list(drc_cache &cache);
	~drc_label_list();

	// block begin/end
	void block_begin(drcuml_block &block);
	void block_end(drcuml_block &block);

	// get/set values
	drccodeptr get_codeptr(uml::code_label label, drc_label_fixup_delegate const &fixup, void *param);
	void set_codeptr(uml::code_label label, drccodeptr codeptr);

private:
	struct label_entry
	{
		label_entry *next() const { return m_next; }
		label_entry *       m_next;         // pointer to next label
		uml::code_label     m_label;        // the label specified
		drccodeptr          m_codeptr;      // pointer to the relevant code
	};

	struct label_fixup
	{
		label_fixup *next() const { return m_next; }
		label_fixup *       m_next;         // pointer to the next oob
		label_entry *       m_label;        // the label in question
		drc_label_fixup_delegate m_callback; // callback
	};

	// internal helpers
	void reset(bool fatal_on_leftovers);
	label_entry *find_or_allocate(uml::code_label label);
	void oob_callback(drccodeptr *codeptr, void *param1, void *param2);

	// internal state
	drc_cache &         m_cache;            // pointer to the cache
	simple_list<label_entry> m_list;        // head of the live list
	simple_list<label_fixup> m_fixup_list;  // list of pending oob fixups
	drc_oob_delegate    m_oob_callback_delegate; // pre-computed delegate
};


// ======================> resolved_member_function

struct resolved_member_function
{
	uintptr_t obj = uintptr_t(nullptr);
	uint8_t *func = nullptr;

	explicit operator bool() const noexcept
	{
		return bool(func);
	}

	template <typename C, typename F>
	void set(C &&instance, F &&mfp) noexcept
	{
		auto const [entrypoint, adjusted] = util::resolve_member_function(std::forward<F>(mfp), std::forward<C>(instance));
		obj = adjusted;
		func = reinterpret_cast<uint8_t *>(entrypoint);
	}
};


// ======================> resolved_memory_accessors

struct resolved_memory_accessors
{
	resolved_member_function read_byte;
	resolved_member_function read_byte_masked;
	resolved_member_function read_word;
	resolved_member_function read_word_masked;
	resolved_member_function read_dword;
	resolved_member_function read_dword_masked;
	resolved_member_function read_qword;
	resolved_member_function read_qword_masked;

	resolved_member_function write_byte;
	resolved_member_function write_byte_masked;
	resolved_member_function write_word;
	resolved_member_function write_word_masked;
	resolved_member_function write_dword;
	resolved_member_function write_dword_masked;
	resolved_member_function write_qword;
	resolved_member_function write_qword_masked;

	void set(address_space &space);
};

using resolved_memory_accessors_vector = std::vector<resolved_memory_accessors>;

} // namespace drc


#endif // MAME_CPU_DRCBEUT_H
