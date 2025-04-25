// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Utility functions for dynamic recompiling backends.

***************************************************************************/

#include "emu.h"
#include "drcbeut.h"

#include <algorithm>


namespace drc {

using namespace uml;


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define LOG_RECOVER         (0)



//**************************************************************************
//  DRC HASH TABLE
//**************************************************************************

//-------------------------------------------------
//  drc_hash_table - constructor
//-------------------------------------------------

drc_hash_table::drc_hash_table(drc_cache &cache, uint32_t modes, uint8_t addrbits, uint8_t ignorebits) :
	m_cache(cache),
	m_modes(modes),
	m_nocodeptr(nullptr),
	m_l1bits((addrbits - ignorebits) / 2),
	m_l2bits((addrbits - ignorebits) - m_l1bits),
	m_l1shift(ignorebits + m_l2bits),
	m_l2shift(ignorebits),
	m_l1mask((1 << m_l1bits) - 1),
	m_l2mask((1 << m_l2bits) - 1),
	m_base(reinterpret_cast<drccodeptr ***>(cache.alloc(modes * sizeof(**m_base)))),
	m_emptyl1(nullptr),
	m_emptyl2(nullptr)
{
	reset();
}


//-------------------------------------------------
//  reset - flush existing hash tables and create
//  new ones
//-------------------------------------------------

bool drc_hash_table::reset()
{
	// allocate an empty l2 hash table
	m_emptyl2 = (drccodeptr *)m_cache.alloc_temporary(sizeof(drccodeptr) << m_l2bits);
	if (m_emptyl2 == nullptr)
		return false;

	// populate it with pointers to the recompile_exit code
	for (int entry = 0; entry < (1 << m_l2bits); entry++)
		m_emptyl2[entry] = m_nocodeptr;

	// allocate an empty l1 hash table
	m_emptyl1 = (drccodeptr **)m_cache.alloc_temporary(sizeof(drccodeptr *) << m_l1bits);
	if (m_emptyl1 == nullptr)
		return false;

	// populate it with pointers to the empty l2 table
	for (int entry = 0; entry < (1 << m_l1bits); entry++)
		m_emptyl1[entry] = m_emptyl2;

	// reset the hash tables
	for (int modenum = 0; modenum < m_modes; modenum++)
		m_base[modenum] = m_emptyl1;

	return true;
}


//-------------------------------------------------
//  block_begin - note the beginning of a block
//-------------------------------------------------

void drc_hash_table::block_begin(drcuml_block &block, const uml::instruction *instlist, uint32_t numinst)
{
	// before generating code, pre-allocate any hash entries; we do this by setting dummy hash values
	for (int inum = 0; inum < numinst; inum++)
	{
		const uml::instruction &inst = instlist[inum];

		// if the opcode is a hash, verify that it makes sense and then set a nullptr entry
		if (inst.opcode() == OP_HASH)
		{
			assert(inst.numparams() == 2);

			// if we fail to allocate, we must abort the block
			if (!set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), nullptr))
				block.abort();
		}

		// if the opcode is a hashjmp to a fixed location, make sure we preallocate the tables
		if (inst.opcode() == OP_HASHJMP && inst.param(0).is_immediate() && inst.param(1).is_immediate())
		{
			// if we fail to allocate, we must abort the block
			drccodeptr code = get_codeptr(inst.param(0).immediate(), inst.param(1).immediate());
			if (!set_codeptr(inst.param(0).immediate(), inst.param(1).immediate(), code))
				block.abort();
		}
	}
}


//-------------------------------------------------
//  block_end - note the end of a block
//-------------------------------------------------

void drc_hash_table::block_end(drcuml_block &block)
{
	// nothing to do here, yet
}


//-------------------------------------------------
//  set_default_codeptr - change the default
//  codeptr
//-------------------------------------------------

void drc_hash_table::set_default_codeptr(drccodeptr nocodeptr)
{
	// nothing to do if the same
	drccodeptr old = m_nocodeptr;
	if (old == nocodeptr)
		return;
	m_nocodeptr = nocodeptr;

	// update the empty L2 table first
	for (int l2entry = 0; l2entry < (1 << m_l2bits); l2entry++)
		m_emptyl2[l2entry] = nocodeptr;

	// now scan all existing hashtables for entries
	for (int modenum = 0; modenum < m_modes; modenum++)
		if (m_base[modenum] != m_emptyl1)
			for (int l1entry = 0; l1entry < (1 << m_l1bits); l1entry++)
				if (m_base[modenum][l1entry] != m_emptyl2)
					for (int l2entry = 0; l2entry < (1 << m_l2bits); l2entry++)
						if (m_base[modenum][l1entry][l2entry] == old)
							m_base[modenum][l1entry][l2entry] = nocodeptr;
}


//-------------------------------------------------
//  set_codeptr - set the codeptr for the given
//  mode/pc
//-------------------------------------------------

bool drc_hash_table::set_codeptr(uint32_t mode, uint32_t pc, drccodeptr code)
{
	// copy-on-write for the l1 hash table
	assert(mode < m_modes);
	if (m_base[mode] == m_emptyl1)
	{
		drccodeptr **newtable = (drccodeptr **)m_cache.alloc_temporary(sizeof(drccodeptr *) << m_l1bits);
		if (newtable == nullptr)
			return false;
		memcpy(newtable, m_emptyl1, sizeof(drccodeptr *) << m_l1bits);
		m_base[mode] = newtable;
	}

	// copy-on-write for the l2 hash table
	uint32_t l1 = (pc >> m_l1shift) & m_l1mask;
	if (m_base[mode][l1] == m_emptyl2)
	{
		drccodeptr *newtable = (drccodeptr *)m_cache.alloc_temporary(sizeof(drccodeptr) << m_l2bits);
		if (newtable == nullptr)
			return false;
		memcpy(newtable, m_emptyl2, sizeof(drccodeptr) << m_l2bits);
		m_base[mode][l1] = newtable;
	}

	// set the new entry
	uint32_t l2 = (pc >> m_l2shift) & m_l2mask;
	m_base[mode][l1][l2] = code;
	return true;
}



//**************************************************************************
//  DRC MAP VARIABLES
//**************************************************************************

//-------------------------------------------------
//  drc_map_variables - constructor
//-------------------------------------------------

drc_map_variables::drc_map_variables(drc_cache &cache, uint64_t uniquevalue) :
	m_cache(cache),
	m_uniquevalue(uniquevalue)
{
	std::fill(std::begin(m_mapvalue), std::end(m_mapvalue), 0);
}


//-------------------------------------------------
//  ~drc_map_variables - destructor
//-------------------------------------------------

drc_map_variables::~drc_map_variables()
{
}


//-------------------------------------------------
//  block_begin - note the beginning of a block
//-------------------------------------------------

void drc_map_variables::block_begin(drcuml_block &block)
{
	// release any remaining live entries
	m_entry_list.clear();
	m_entry_list.reserve(128);

	// reset the variable values
	std::fill(std::begin(m_mapvalue), std::end(m_mapvalue), 0);
}


//-------------------------------------------------
//  block_end - note the end of a block
//-------------------------------------------------

void drc_map_variables::block_end(drcuml_block &block)
{
	// only process if we have data
	if (m_entry_list.empty())
		return;

	// begin "code generation" aligned to an 8-byte boundary
	drccodeptr *top = m_cache.begin_codegen(sizeof(uint64_t) + sizeof(uint32_t) + 2 * sizeof(uint32_t) * m_entry_list.size());
	if (!top)
		block.abort();
	auto dest = reinterpret_cast<uint32_t *>((uintptr_t(*top) + 7) & ~uintptr_t(7));

	// store the cookie first
	*(uint64_t *)dest = m_uniquevalue;
	dest += 2;

	// get the pointer to the first item and store an initial backwards offset
	drccodeptr lastptr = m_entry_list.front().codeptr;
	*dest = drccodeptr(dest) - lastptr;
	dest++;

	// now iterate over entries and store them
	std::array<uint32_t, MAPVAR_COUNT> curvalue;
	std::array<bool, MAPVAR_COUNT> changed;
	std::fill(curvalue.begin(), curvalue.end(), 0);
	std::fill(changed.begin(), changed.end(), false);
	auto entry = m_entry_list.begin();
	while (m_entry_list.end() != entry)
	{
		// update the current value of the variable and detect changes
		if (curvalue[entry->mapvar] != entry->newval)
		{
			curvalue[entry->mapvar] = entry->newval;
			changed[entry->mapvar] = true;
		}

		// if the next code pointer is different, or if we're at the end, flush changes
		auto const next = std::next(entry);
		if ((m_entry_list.end() == next) || (next->codeptr != entry->codeptr))
		{
			// build a mask of changed variables
			int numchanged = 0;
			uint32_t varmask = 0;
			for (int varnum = 0; varnum < std::size(changed); varnum++)
			{
				if (changed[varnum])
				{
					changed[varnum] = false;
					varmask |= 1 << varnum;
					numchanged++;
				}
			}

			// if nothing really changed, skip it
			if (numchanged)
			{
				// first word is a code delta plus mask of changed variables
				uint32_t codedelta = entry->codeptr - lastptr;
				while (codedelta > 0xffff)
				{
					*dest++ = uint32_t(0xffff) << 16;
					codedelta -= 0xffff;
				}
				*dest++ = (codedelta << 16) | (varmask << 4) | numchanged;

				// now output updated variable values
				for (int varnum = 0; varnum < changed.size(); varnum++)
				{
					if (BIT(varmask, varnum))
						*dest++ = curvalue[varnum];
				}

				// remember our lastptr
				lastptr = entry->codeptr;
			}
		}
		entry = next;
	}

	// add a terminator
	*dest++ = 0;

	// complete codegen
	*top = (drccodeptr)dest;
	m_cache.end_codegen();
}


//-------------------------------------------------
//  set_value - set a map value for the given
//  code pointer
//-------------------------------------------------

void drc_map_variables::set_value(drccodeptr codebase, uint32_t mapvar, uint32_t newvalue)
{
	assert((mapvar >= MAPVAR_M0) && (mapvar < MAPVAR_END));

	// if this value isn't different, skip it
	if (m_mapvalue[mapvar - MAPVAR_M0] == newvalue)
		return;

	// allocate a new entry and fill it in
	auto &entry = m_entry_list.emplace_back();
	entry.codeptr = codebase;
	entry.mapvar = mapvar - MAPVAR_M0;
	entry.newval = newvalue;

	// update our state in the table as well
	m_mapvalue[mapvar - MAPVAR_M0] = newvalue;
}


//-------------------------------------------------
//  get_value - return a map value for the given
//  code pointer
//-------------------------------------------------

uint32_t drc_map_variables::get_value(drccodeptr codebase, uint32_t mapvar) const
{
	assert((mapvar >= MAPVAR_M0) && (mapvar < MAPVAR_END));
	mapvar -= MAPVAR_M0;

	// get an aligned pointer to start scanning
	auto curscan = reinterpret_cast<uint64_t const *>((uintptr_t(codebase) | 7) + 1);
	auto const endscan = reinterpret_cast<uint64_t const *>(m_cache.top());

	// look for the signature
	while ((curscan < endscan) && (*curscan++ != m_uniquevalue)) { }
	if (curscan >= endscan)
		return 0;

	// switch to 32-bit pointers for processing the rest
	auto data = reinterpret_cast<uint32_t const *>(curscan);

	// first get the 32-bit starting offset to the code
	drccodeptr curcode = (drccodeptr)data - *data;
	data++;

	// now loop until we advance past our target
	uint32_t const varmask = 0x10 << mapvar;
	uint32_t result = 0;
	while (true)
	{
		// a 0 is a terminator
		uint32_t controlword = *data++;
		if (controlword == 0)
			break;

		// update the codeptr; if this puts us past the end, we're done
		curcode += (controlword >> 16) & 0xffff;
		if (curcode > codebase)
			break;

		// if our mapvar has changed, process this word
		if (controlword & varmask)
		{
			// count how many words precede the one we care about
			int dataoffs = 0;
			for (uint32_t skipmask = (controlword & (varmask - 1)) >> 4; skipmask != 0; skipmask = skipmask & (skipmask - 1))
				dataoffs++;

			// fetch the one we want
			result = data[dataoffs];
		}

		// low 4 bits contain the total number of words of data
		data += controlword & 0x0f;
	}
	if (LOG_RECOVER)
		printf("recover %d @ %p = %08X\n", mapvar, codebase, result);
	return result;
}



//-------------------------------------------------
//  get_last_value - return the most recently set
//  map value
//-------------------------------------------------

uint32_t drc_map_variables::get_last_value(uint32_t mapvar)
{
	assert(mapvar >= MAPVAR_M0 && mapvar < MAPVAR_END);
	return m_mapvalue[mapvar - MAPVAR_M0];
}



//**************************************************************************
//  DRC LABEL LIST
//**************************************************************************

//-------------------------------------------------
//  drc_label_list - constructor
//-------------------------------------------------

drc_label_list::drc_label_list(drc_cache &cache) :
	m_cache(cache),
	m_oob_callback_delegate(&drc_label_list::oob_callback, this)
{
}


//-------------------------------------------------
//  ~drc_label_list - destructor
//-------------------------------------------------

drc_label_list::~drc_label_list()
{
}


//-------------------------------------------------
//  block_begin - note the beginning of a block
//-------------------------------------------------

void drc_label_list::block_begin(drcuml_block &block)
{
	// make sure the label list is clear, but don't fatalerror
	reset(false);
}


//-------------------------------------------------
//  block_end - note the end of a block
//-------------------------------------------------

void drc_label_list::block_end(drcuml_block &block)
{
	// can't free until the cache is clean of our OOB requests
	assert(!m_cache.generating_code());

	// free all of the pending fixup requests
	m_free_fixups.splice(m_free_fixups.begin(), m_fixup_list);

	// make sure the label list is clear, and fatalerror if we missed anything
	reset(true);
}


//-------------------------------------------------
//  get_codeptr - find or allocate a new label;
//  returns nullptr and requests an OOB callback if
//  undefined
//-------------------------------------------------

drccodeptr drc_label_list::get_codeptr(uml::code_label label, drc_label_fixup_delegate const &callback, void *param)
{
	label_entry &curlabel = find_or_allocate(label);

	// if no code pointer, request an OOB callback
	if (!curlabel.codeptr && !callback.isnull())
	{
		label_fixup_list::iterator fixup = m_free_fixups.begin();
		if (m_free_fixups.end() == fixup)
			fixup = m_fixup_list.emplace(m_fixup_list.end());
		else
			m_fixup_list.splice(m_fixup_list.end(), m_free_fixups, fixup);
		fixup->label = &curlabel;
		fixup->callback = callback;
		m_cache.request_oob_codegen(drc_oob_delegate(m_oob_callback_delegate), &*fixup, param);
	}

	return curlabel.codeptr;
}


//-------------------------------------------------
//  set_codeptr - set the pointer to a new label
//-------------------------------------------------

void drc_label_list::set_codeptr(uml::code_label label, drccodeptr codeptr)
{
	// set the code pointer
	label_entry &curlabel = find_or_allocate(label);
	assert(!curlabel.codeptr);
	curlabel.codeptr = codeptr;
}


//-------------------------------------------------
//  reset - reset a label list (add all entries to
//  the free list)
//-------------------------------------------------

void drc_label_list::reset(bool fatal_on_leftovers)
{
	// loop until out of labels
	while (!m_list.empty())
	{
		// fatal if we were a leftover
		if (fatal_on_leftovers && !m_list.front().codeptr)
			throw emu_fatalerror("Label %08X never defined!\n", m_list.front().label.label());

		// free the label
		m_free_labels.splice(m_free_labels.begin(), m_list, m_list.begin());
	}
}


//-------------------------------------------------
//  find_or_allocate - look up a label and
//  allocate a new one if not found
//-------------------------------------------------

drc_label_list::label_entry &drc_label_list::find_or_allocate(uml::code_label label)
{
	// find the label, or else allocate a new one
	for (auto curlabel = m_list.begin(); m_list.end() != curlabel; ++curlabel)
	{
		if (curlabel->label == label)
			return *curlabel;
	}

	// if none found, allocate
	auto newlabel = m_free_labels.begin();
	if (m_free_labels.end() == newlabel)
		newlabel = m_list.emplace(m_list.end());
	else
		m_list.splice(m_list.end(), m_free_labels, newlabel);
	newlabel->label = label;
	newlabel->codeptr = nullptr;
	return *newlabel;
}


//-------------------------------------------------
//  oob_callback - out-of-band codegen callback
//  for labels
//-------------------------------------------------

void drc_label_list::oob_callback(drccodeptr *codeptr, void *param1, void *param2)
{
	label_fixup *fixup = reinterpret_cast<label_fixup *>(param1);
	fixup->callback(param2, fixup->label->codeptr);
}



//**************************************************************************
//  RESOLVED MEMORY ACCESSORS
//**************************************************************************

//-------------------------------------------------
//  set - bind to address space
//-------------------------------------------------

void resolved_memory_accessors::set(address_space &space)
{
	read_byte          .set(space, static_cast<u8  (address_space::*)(offs_t)     >(&address_space::read_byte));
	read_byte_masked   .set(space, static_cast<u8  (address_space::*)(offs_t, u8) >(&address_space::read_byte));
	read_word          .set(space, static_cast<u16 (address_space::*)(offs_t)     >(&address_space::read_word));
	read_word_masked   .set(space, static_cast<u16 (address_space::*)(offs_t, u16)>(&address_space::read_word));
	read_dword         .set(space, static_cast<u32 (address_space::*)(offs_t)     >(&address_space::read_dword));
	read_dword_masked  .set(space, static_cast<u32 (address_space::*)(offs_t, u32)>(&address_space::read_dword));
	read_qword         .set(space, static_cast<u64 (address_space::*)(offs_t)     >(&address_space::read_qword));
	read_qword_masked  .set(space, static_cast<u64 (address_space::*)(offs_t, u64)>(&address_space::read_qword));

	write_byte         .set(space, static_cast<void (address_space::*)(offs_t, u8)      >(&address_space::write_byte));
	write_byte_masked  .set(space, static_cast<void (address_space::*)(offs_t, u8, u8)  >(&address_space::write_byte));
	write_word         .set(space, static_cast<void (address_space::*)(offs_t, u16)     >(&address_space::write_word));
	write_word_masked  .set(space, static_cast<void (address_space::*)(offs_t, u16, u16)>(&address_space::write_word));
	write_dword        .set(space, static_cast<void (address_space::*)(offs_t, u32)     >(&address_space::write_dword));
	write_dword_masked .set(space, static_cast<void (address_space::*)(offs_t, u32, u32)>(&address_space::write_dword));
	write_qword        .set(space, static_cast<void (address_space::*)(offs_t, u64)     >(&address_space::write_qword));
	write_qword_masked .set(space, static_cast<void (address_space::*)(offs_t, u64, u64)>(&address_space::write_qword));

	if (
			!read_byte  || !read_byte_masked  || !write_byte  || !write_byte_masked  ||
			!read_word  || !read_word_masked  || !write_word  || !write_word_masked  ||
			!read_dword || !read_dword_masked || !write_dword || !write_dword_masked ||
			!read_qword || !read_qword_masked || !write_qword || !write_qword_masked)
	{
		throw emu_fatalerror("Error resolving address space accessor member functions!\n");
	}
}

} // namespace drc
