// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    save.cpp

    Save state management functions.

****************************************************************************

    Save state file format:

    00..07  'MAMESAVE'
    08      Format version (this is format 2)
    09      Flags
    0A..1B  Game name padded with \0
    1C..1F  Signature
    20..end Save game data (compressed)

    Data is always written as native-endian.
    Data is converted from the endiannness it was written upon load.

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "coreutil.h"

#include <iomanip>


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) machine().logerror x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int SAVE_VERSION      = 2;
const int HEADER_SIZE       = 32;

// Available flags
enum
{
	SS_MSB_FIRST = 0x02
};

#define STATE_MAGIC_NUM         "MAMESAVE"


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  json_append - append a string to the JSON
//  stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append(char const *buffer)
{
	json_check_reserve();
	while (*buffer != 0)
		json_append(*buffer++);
	return *this;
}


//-------------------------------------------------
//  json_append_indent - append an indentation of
//  the given depth to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_indent(int count)
{
	for (int index = 0; index < count; index++)
		json_append('\t');
	return *this;
}


//-------------------------------------------------
//  json_append_name - append a string-ified name
//  to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_name(char const *name)
{
	if (name == nullptr || name[0] == 0)
		return *this;
	return json_append('"').json_append(name).json_append('"').json_append(':');
}


//-------------------------------------------------
//  json_append_signed - append a signed integer
//  value to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_signed(int64_t value)
{
	json_check_reserve();

	// quote values that don't fit into a double
	bool quote = (int64_t(double(value)) != value);
	if (quote)
		json_append('"');

	// just use sprintf -- is there a faster way?
	char buffer[20];
	sprintf(buffer, "%lld", value);
	json_append(buffer);

	// end quotes
	if (quote)
		json_append('"');
	return *this;
}


//-------------------------------------------------
//  json_append_unsigned - append an unsigned
//  integer value to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_unsigned(uint64_t value)
{
	json_check_reserve();

	// quote values that don't fit into a double
	bool quote = (uint64_t(double(value)) != value);
	if (quote)
		json_append('"');

	// just use sprintf -- is there a faster way?
	char buffer[20];
	sprintf(buffer, "%llu", value);
	json_append(buffer);

	// end quotes
	if (quote)
		json_append('"');
	return *this;
}


//-------------------------------------------------
//  json_append_float - append a floating-point
//  value to the JSON stream
//-------------------------------------------------

save_zip_state &save_zip_state::json_append_float(double value)
{
	json_check_reserve();
	char buffer[20];
	sprintf(buffer, "%g", value);
	return json_append(buffer);
}



//**************************************************************************
//  SAVE REGISTERED ITEM
//**************************************************************************

//-------------------------------------------------
//  save_registered_item - constructor
//-------------------------------------------------

save_registered_item::save_registered_item() :
	m_ptr_offset(0),
	m_type(TYPE_CONTAINER),
	m_native_size(0)
{
}

// constructor for a new item
save_registered_item::save_registered_item(uintptr_t ptr_offset, save_type type, uint32_t native_size, char const *name) :
	m_ptr_offset(ptr_offset),
	m_type(type),
	m_native_size(native_size),
	m_name(name)
{
	// cleanup names a bit
	if (m_name[0] == '*')
		m_name.erase(0, 1);
	if (m_name[0] == 'm' && m_name[1] == '_')
		m_name.erase(0, 2);
}


//-------------------------------------------------
//  append - append a new item to the current one
//-------------------------------------------------

std::string type_string(save_registered_item::save_type type, uint32_t native_size)
{
	switch (type)
	{
	case save_registered_item::TYPE_CONTAINER:	return "CONTAINER";
	case save_registered_item::TYPE_POINTER:	return "POINTER";
	case save_registered_item::TYPE_UNIQUE:		return "UNIQUE";
	case save_registered_item::TYPE_VECTOR:		return "VECTOR";
	case save_registered_item::TYPE_STRUCT:		return "STRUCT";
	case save_registered_item::TYPE_BOOL:		return "BOOL";
	case save_registered_item::TYPE_INT:		return string_format("INT%d", 8 * native_size);
	case save_registered_item::TYPE_UINT:		return string_format("UINT%d", 8 * native_size);
	case save_registered_item::TYPE_FLOAT:		return string_format("FLOAT%d", 8 * native_size);
	default:				return string_format("ARRAY[%d]", int(type));
	}
}

save_registered_item &save_registered_item::append(uintptr_t ptr_offset, save_type type, uint32_t native_size, char const *name)
{
	// make sure there are no duplicates
	if (find(name) != nullptr)
		throw emu_fatalerror("Duplicate save state registration '%s'\n", name);

//printf("%s '%s': adding %s '%s' @ %llX, size %d\n", type_string(m_type, m_native_size).c_str(), m_name.c_str(), type_string(type, native_size).c_str(), name, ptr_offset, native_size);

	// add the item to the back of the list
	m_items.emplace_back(ptr_offset, type, native_size, name);
	return m_items.back();
}


//-------------------------------------------------
//  find - find a subitem by name
//-------------------------------------------------

save_registered_item *save_registered_item::find(char const *name)
{
	// blank names can't be found this way
	if (name[0] == 0)
		return nullptr;

	// make sure there are no duplicates
	for (auto &item : m_items)
		if (strcmp(item.name(), name) == 0)
			return &item;
	return nullptr;
}


//-------------------------------------------------
//  sort_and_prune - prune empty subitems and
//  sort them by name
//-------------------------------------------------

bool save_registered_item::sort_and_prune()
{
	// only applies to arrays, structs, and containers; don't prune anything else
	if (m_type >= TYPE_ARRAY && m_type != TYPE_STRUCT && m_type != TYPE_CONTAINER)
		return false;

	// first prune any empty items
	for (auto it = m_items.begin(); it != m_items.end(); )
	{
		if (it->sort_and_prune())
			it = m_items.erase(it);
		else
			++it;
	}

	// then sort the rest if we have more than 1
	if (m_items.size() > 1)
		m_items.sort([] (auto const &x, auto const &y) { return (std::strcmp(x.name(), y.name()) < 0); });

	// return true if we have nothing
	return (m_items.size() == 0);
}


//-------------------------------------------------
//  unwrap_and_update_objbase - unwrap trivial
//  type and update the object base
//-------------------------------------------------

bool save_registered_item::unwrap_and_update_objbase(uintptr_t &objbase) const
{
	// update the base pointer with our local base/offset
	objbase += m_ptr_offset;

	// switch off the type
	switch (m_type)
	{
		// unique ptrs retrieve the pointer from their container
		case TYPE_UNIQUE:
			objbase = reinterpret_cast<uintptr_t>(reinterpret_cast<generic_unique *>(objbase)->get());
			return true;

		// vectors retrieve the pointer from their container
		case TYPE_VECTOR:
			objbase = reinterpret_cast<uintptr_t>(&(*reinterpret_cast<generic_vector *>(objbase))[0]);
			return true;

		// pointers just extract the pointer directly
		case TYPE_POINTER:
			objbase = reinterpret_cast<uintptr_t>(*reinterpret_cast<generic_pointer *>(objbase));
			return true;

		// containers are always based at 0
		case TYPE_CONTAINER:
			objbase = 0;
			return false;

		// everything else is as-is
		default:
			return false;
	}
}


//-------------------------------------------------
//  save_binary - save this item and all owned
//  items into a binary form
//-------------------------------------------------

uint64_t save_registered_item::save_binary(uint8_t *ptr, uint64_t length, uintptr_t objbase) const
{
	// update the base pointer and forward if a trivial unwrap
	if (unwrap_and_update_objbase(objbase))
		return m_items.front().save_binary(ptr, length, objbase);

	// switch off the type
	uint64_t offset = 0;
	switch (m_type)
	{
		// boolean types save as a single byte
		case TYPE_BOOL:
			if (offset + 1 <= length)
				ptr[offset] = *reinterpret_cast<bool const *>(objbase) ? 1 : 0;
			offset++;
			break;

		// integral/float types save as their native size
		case TYPE_INT:
		case TYPE_UINT:
		case TYPE_FLOAT:
			if (offset + m_native_size <= length)
				memcpy(&ptr[offset], reinterpret_cast<void const *>(objbase), m_native_size);
			offset += m_native_size;
			break;

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			for (auto &item : m_items)
				offset += item.save_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase);
			break;

		// arrays are multiples of a single item
		default:
			if (m_type < TYPE_ARRAY)
			{
				auto &item = m_items.front();
				for (uint32_t rep = 0; rep < m_type; rep++)
					offset += item.save_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase + rep * m_native_size);
			}
			break;
	}
	return offset;
}


//-------------------------------------------------
//  restore_binary - restore this item and all
//  owned items from binary form
//-------------------------------------------------

uint64_t save_registered_item::restore_binary(uint8_t const *ptr, uint64_t length, uintptr_t objbase) const
{
	// update the base pointer and forward if a trivial unwrap
	if (unwrap_and_update_objbase(objbase))
		return m_items.front().restore_binary(ptr, length, objbase);

	// switch off the type
	uint64_t offset = 0;
	switch (m_type)
	{
		// boolean types save as a single byte
		case TYPE_BOOL:
			if (offset + 1 <= length)
				*reinterpret_cast<bool *>(objbase) = (ptr[offset] != 0);
			offset++;
			break;

		// integral/float types save as their native size
		case TYPE_INT:
		case TYPE_UINT:
		case TYPE_FLOAT:
			if (offset + m_native_size <= length)
				memcpy(reinterpret_cast<void *>(objbase), &ptr[offset], m_native_size);
			offset += m_native_size;
			break;

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			for (auto &item : m_items)
				offset += item.restore_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase);
			break;

		// arrays are multiples of a single item
		default:
			if (m_type < TYPE_ARRAY)
			{
				auto &item = m_items.front();
				for (uint32_t rep = 0; rep < m_type; rep++)
					offset += item.restore_binary(&ptr[offset], (offset < length) ? length - offset : 0, objbase + rep * m_native_size);
			}
			break;
	}
	return offset;
}


//-------------------------------------------------
//  save_json - save this item into a JSON stream
//-------------------------------------------------

void save_registered_item::save_json(save_zip_state &zipstate, char const *nameprefix, int indent, bool inline_form, uintptr_t objbase)
{
	// update the base pointer and forward if a trivial unwrap
	if (unwrap_and_update_objbase(objbase))
		return m_items.front().save_json(zipstate, nameprefix, indent, inline_form, objbase);

	// update the name prefix
	std::string localname = nameprefix;
	if (m_name.length() != 0)
	{
		if (localname.length() != 0)
			localname += ".";
		localname += m_name;
	}

	// output the name if present
	zipstate.json_append_name(m_name.c_str());

	// switch off the type
	switch (m_type)
	{
		// boolean types
		case TYPE_BOOL:
			zipstate.json_append(*reinterpret_cast<bool const *>(objbase) ? "true" : "false");
			break;

		// signed integral types
		case TYPE_INT:
			zipstate.json_append_signed(read_int_signed(objbase, m_native_size));
			break;

		// unsigned integral types
		case TYPE_UINT:
			zipstate.json_append_unsigned(read_int_unsigned(objbase, m_native_size));
			break;

		// float types
		case TYPE_FLOAT:
			zipstate.json_append_float(read_float(objbase, m_native_size));
			break;

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			if (inline_form || compute_binary_size(objbase - m_ptr_offset) <= 16)
			{
				// inline form outputs everything on a single line
				zipstate.json_append('{');
				for (auto &item : m_items)
				{
					item.save_json(zipstate, localname.c_str(), indent, true, objbase);
					if (&item != &m_items.back())
						zipstate.json_append(',');
				}
				zipstate.json_append('}');
			}
			else
			{
				// normal form outputs each item on its own line, indented
				zipstate.json_append('{').json_append_eol();
				for (auto &item : m_items)
				{
					zipstate.json_append_indent(indent + 1);
					item.save_json(zipstate, localname.c_str(), indent + 1, false, objbase);
					if (&item != &m_items.back())
						zipstate.json_append(',');
					zipstate.json_append_eol();
				}
				zipstate.json_append_indent(indent).json_append('}');
			}
			break;

		// arrays are multiples of a single item
		default:
			if (m_type < TYPE_ARRAY)
			{
				auto &item = m_items.front();

				// look for large arrays of ints/floats
				save_registered_item *inner = &item;
				u32 total = count();
				while (inner->type() < TYPE_ARRAY)
				{
					total *= inner->count();
					inner = &inner->m_items.front();
				}
				if ((inner->type() == TYPE_INT || inner->type() == TYPE_UINT || inner->type() == TYPE_FLOAT) && total * inner->m_native_size >= save_zip_state::JSON_EXTERNAL_BINARY_THRESHOLD)
				{
					std::string filename = localname;
					for (int index = 0; index < filename.length(); )
						if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-.", filename[index]) == nullptr)
						{
							if (index != 0 && filename[index - 1] != '.')
								filename[index++] = '.';
							else
								filename.erase(index, 1);
						}
						else
							index++;

					zipstate.json_append('[').json_append('{');
					zipstate.json_append_name("external_file");
					zipstate.json_append('"').json_append(filename.c_str()).json_append('"').json_append(',');
					zipstate.json_append_name("unit");
					zipstate.json_append_signed(inner->m_native_size).json_append(',');
					zipstate.json_append_name("count");
					zipstate.json_append_signed(total).json_append(',');
					zipstate.json_append_name("little_endian");
					zipstate.json_append((ENDIANNESS_NATIVE == ENDIANNESS_LITTLE) ? "true" : "false");
					zipstate.json_append('}').json_append(']');

					zipstate.add_data_file(filename.c_str(), item, reinterpret_cast<void *>(objbase));
				}
				else
				{
					uint32_t item_size = item.compute_binary_size(objbase);
					if (inline_form || m_type * item_size <= 16)
					{
						// strictly inline form outputs everything on a single line
						zipstate.json_append('[');
						for (uint32_t rep = 0; rep < m_type; rep++)
						{
							item.save_json(zipstate, localname.c_str(), 0, true, objbase + rep * m_native_size);
							if (rep != m_type - 1)
								zipstate.json_append(',');
						}
						zipstate.json_append(']');
					}
					else
					{
						// normal form outputs a certain number of items per row
						zipstate.json_append('[').json_append_eol();
						uint32_t items_per_row = 0;
						if (item.m_type == TYPE_INT || item.m_type == TYPE_UINT || item.m_type == TYPE_FLOAT)
							items_per_row = 32 / item_size;
						if (items_per_row == 0)
							items_per_row = 1;

						// iterate over the items
						for (uint32_t rep = 0; rep < m_type; rep++)
						{
							if (rep % items_per_row == 0)
								zipstate.json_append_indent(indent + 1);
							item.save_json(zipstate, localname.c_str(), indent + 1, false, objbase + rep * m_native_size);
							if (rep != m_type - 1)
								zipstate.json_append(',');
							if (rep % items_per_row == items_per_row - 1)
								zipstate.json_append_eol();
						}
						if (m_type % items_per_row != 0)
							zipstate.json_append_eol();
						zipstate.json_append_indent(indent).json_append(']');
					}
				}
			}
			break;
	}
}


//-------------------------------------------------
//  read_int_unsigned - read an unsigned integer
//  of the given size
//-------------------------------------------------

uint64_t save_registered_item::read_int_unsigned(uintptr_t objbase, int size)
{
	switch (size)
	{
		case 1:	return *reinterpret_cast<uint8_t const *>(objbase);
		case 2:	return *reinterpret_cast<uint16_t const *>(objbase);
		case 4:	return *reinterpret_cast<uint32_t const *>(objbase);
		case 8:	return *reinterpret_cast<uint64_t const *>(objbase);
	}
	return 0;
}


//-------------------------------------------------
//  read_int_signed - read a signed integer of the
//  given size
//-------------------------------------------------

int64_t save_registered_item::read_int_signed(uintptr_t objbase, int size)
{
	switch (size)
	{
		case 1:	return *reinterpret_cast<int8_t const *>(objbase);
		case 2:	return *reinterpret_cast<int16_t const *>(objbase);
		case 4:	return *reinterpret_cast<int32_t const *>(objbase);
		case 8:	return *reinterpret_cast<int64_t const *>(objbase);
	}
	return 0;
}


//-------------------------------------------------
//  read_float - read a floating-point value of the
//  given size
//-------------------------------------------------

double save_registered_item::read_float(uintptr_t objbase, int size)
{
	switch (size)
	{
		case 4:	return *reinterpret_cast<float const *>(objbase);
		case 8:	return *reinterpret_cast<double const *>(objbase);
	}
	return 0;
}


//-------------------------------------------------
//  write_int - write an integer of the given size
//-------------------------------------------------

void save_registered_item::write_int(uintptr_t objbase, int size, uint64_t data)
{
	switch (size)
	{
		case 1:	*reinterpret_cast<uint8_t *>(objbase) = uint8_t(data); break;
		case 2:	*reinterpret_cast<uint16_t *>(objbase) = uint16_t(data); break;
		case 4:	*reinterpret_cast<uint32_t *>(objbase) = uint32_t(data); break;
		case 8:	*reinterpret_cast<uint64_t *>(objbase) = uint64_t(data); break;
	}
}


//-------------------------------------------------
//  write_float - write a floating-point value of
//  the given size
//-------------------------------------------------

void save_registered_item::write_float(uintptr_t objbase, int size, double data)
{
	switch (size)
	{
		case 4:	*reinterpret_cast<float *>(objbase) = float(data); break;
		case 8:	*reinterpret_cast<double *>(objbase) = double(data); break;
	}
}



//**************************************************************************
//  SAVE MANAGER
//**************************************************************************

//-------------------------------------------------
//  save_manager - constructor
//-------------------------------------------------

save_manager::save_manager(running_machine &machine) :
	m_machine(machine),
	m_reg_allowed(true),
	m_root_registrar(m_root_item)
{
	m_rewind = std::make_unique<rewinder>(*this);
}


//-------------------------------------------------
//  allow_registration - allow/disallow
//  registrations to happen
//-------------------------------------------------

void save_manager::allow_registration(bool allowed)
{
	// allow/deny registration
	m_reg_allowed = allowed;
	if (!allowed)
	{
		// prune and sort
		m_root_item.sort_and_prune();

		// dump out a sample JSON
		{
			save_zip_state state;
			m_root_item.save_json(state);
			printf("%s\n", state.json_string());
		}

		// everything is registered by now, evaluate the savestate size
		m_rewind->clamp_capacity();
	}
}


//-------------------------------------------------
//  register_presave - register a pre-save
//  function callback
//-------------------------------------------------

void save_manager::register_presave(save_prepost_delegate func)
{
	// check for invalid timing
	if (!m_reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!\n");

	// scan for duplicates and push through to the end
	for (auto &cb : m_presave_list)
		if (cb->m_func == func)
			fatalerror("Duplicate save state function (%s/%s)\n", cb->m_func.name(), func.name());

	// allocate a new entry
	m_presave_list.push_back(std::make_unique<state_callback>(func));
}


//-------------------------------------------------
//  state_save_register_postload -
//  register a post-load function callback
//-------------------------------------------------

void save_manager::register_postload(save_prepost_delegate func)
{
	// check for invalid timing
	if (!m_reg_allowed)
		fatalerror("Attempt to register callback function after state registration is closed!\n");

	// scan for duplicates and push through to the end
	for (auto &cb : m_postload_list)
		if (cb->m_func == func)
			fatalerror("Duplicate save state function (%s/%s)\n", cb->m_func.name(), func.name());

	// allocate a new entry
	m_postload_list.push_back(std::make_unique<state_callback>(func));
}


//-------------------------------------------------
//  dispatch_postload - invoke all registered
//  postload callbacks for updates
//-------------------------------------------------

void save_manager::dispatch_postload()
{
	for (auto &func : m_postload_list)
		func->m_func();
}


//-------------------------------------------------
//  dispatch_presave - invoke all registered
//  presave callbacks for updates
//-------------------------------------------------

void save_manager::dispatch_presave()
{
	for (auto &func : m_presave_list)
		func->m_func();
}


//-------------------------------------------------
//  save_binary - invoke all registered presave
//  callbacks for updates and then generate the
//  data in binary form
//-------------------------------------------------

save_error save_manager::save_binary(void *buf, size_t size)
{
	// call the pre-save functions
	dispatch_presave();

	// write the output
	u64 finalsize = m_root_item.save_binary(reinterpret_cast<u8 *>(buf), size);
	if (finalsize != size)
		return STATERR_WRITE_ERROR;

	return STATERR_NONE;
}


//-------------------------------------------------
//  load_binary - restore all data and then call
//  the postload callbacks
//-------------------------------------------------

save_error save_manager::load_binary(void *buf, size_t size)
{
	// read the input
	u64 finalsize = m_root_item.restore_binary(reinterpret_cast<u8 *>(buf), size);
	if (finalsize != size)
		return STATERR_READ_ERROR;

	// call the post-load functions
	dispatch_postload();
	return STATERR_NONE;
}


//-------------------------------------------------
//  save_file - invoke all registered presave
//  callbacks for updates and then generate the
//  data in JSON/ZIP form
//-------------------------------------------------

save_error save_manager::save_file(emu_file &file)
{
	// call the pre-save functions
	dispatch_presave();

	// create the JSON and target all the output files
	save_zip_state state;
	m_root_item.save_json(state);

	// write the output
	__debugbreak();

	return STATERR_NONE;
}


//-------------------------------------------------
//  load_file - restore all data and then call
//  the postload callbacks
//-------------------------------------------------

save_error save_manager::load_file(emu_file &file)
{
	__debugbreak();

	// call the post-load functions
	dispatch_postload();
	return STATERR_NONE;
}



//**************************************************************************
//  RAM STATE
//**************************************************************************

//-------------------------------------------------
//  ram_state - constructor
//-------------------------------------------------

ram_state::ram_state(save_manager &save) :
	m_valid(false),
	m_time(m_save.machine().time()),
	m_save(save)
{
}


//-------------------------------------------------
//  save - write the current machine state to the
//  allocated stream
//-------------------------------------------------

save_error ram_state::save()
{
	// initialize
	m_valid = false;

	// get the save manager to write state
	const save_error err = m_save.save_binary(m_data);
	if (err != STATERR_NONE)
		return err;

	// final confirmation
	m_valid = true;
	m_time = m_save.machine().time();

	return STATERR_NONE;
}


//-------------------------------------------------
//  load - restore the machine state from the
//  stream
//-------------------------------------------------

save_error ram_state::load()
{
	// get the save manager to load state
	return m_save.load_binary(m_data);
}



//**************************************************************************
//  REWINDER
//**************************************************************************

//-------------------------------------------------
//  rewinder - constuctor
//-------------------------------------------------

rewinder::rewinder(save_manager &save) :
	m_save(save),
	m_enabled(save.machine().options().rewind()),
	m_capacity(save.machine().options().rewind_capacity()),
	m_current_index(REWIND_INDEX_NONE),
	m_first_invalid_index(REWIND_INDEX_NONE),
	m_first_time_warning(true),
	m_first_time_note(true)
{
}


//-------------------------------------------------
//  clamp_capacity - safety checks for commandline
//  override
//-------------------------------------------------

void rewinder::clamp_capacity()
{
	if (!m_enabled)
		return;

	const size_t total = m_capacity * 1024 * 1024;
	const size_t single = m_save.binary_size();

	// can't set below zero, but allow commandline to override options' upper limit
	if (total < 0)
		m_capacity = 0;

	// if capacity is below savestate size, can't save anything
	if (total < single)
	{
		m_enabled = false;
		m_save.machine().logerror("Rewind has been disabled, because rewind capacity is smaller than savestate size.\n");
		m_save.machine().logerror("Rewind buffer size: %d bytes. Savestate size: %d bytes.\n", total, single);
		m_save.machine().popmessage("Rewind has been disabled. See error.log for details");
	}
}


//-------------------------------------------------
//  invalidate - mark all the future states as
//  invalid to prevent loading them, as the
//  current input might have changed
//-------------------------------------------------

void rewinder::invalidate()
{
	if (!m_enabled)
		return;

	// is there anything to invalidate?
	if (!current_index_is_last())
	{
		// all states starting from the current one will be invalid
		m_first_invalid_index = m_current_index;

		// actually invalidate
		for (auto it = m_state_list.begin() + m_first_invalid_index; it < m_state_list.end(); ++it)
			it->get()->m_valid = false;
	}
}


//-------------------------------------------------
//  capture - record a single state, returns true
//  on success
//-------------------------------------------------

bool rewinder::capture()
{
	if (!m_enabled)
	{
		report_error(STATERR_DISABLED, rewind_operation::SAVE);
		return false;
	}

	if (current_index_is_last())
	{
		// we need to create a new state
		std::unique_ptr<ram_state> state = std::make_unique<ram_state>(m_save);
		const save_error error = state->save();

		// validate the state
		if (error == STATERR_NONE)
			// it's safe to append
			m_state_list.push_back(std::move(state));
		else
		{
			// internal error, complain and evacuate
			report_error(error, rewind_operation::SAVE);
			return false;
		}
	}
	else
	{
		// invalidate the future states
		invalidate();

		// update the existing state
		ram_state *state = m_state_list.at(m_current_index).get();
		const save_error error = state->save();

		// validate the state
		if (error != STATERR_NONE)
		{
			// internal error, complain and evacuate
			report_error(error, rewind_operation::SAVE);
			return false;
		}
	}

	// make sure we will fit in
	if (!check_size())
		// the list keeps growing
		m_current_index++;

	// update first invalid index
	if (current_index_is_last())
		m_first_invalid_index = REWIND_INDEX_NONE;
	else
		m_first_invalid_index = m_current_index + 1;

	// success
	report_error(STATERR_NONE, rewind_operation::SAVE);
	return true;
}


//-------------------------------------------------
//  step - single step back in time, returns true
//  on success
//-------------------------------------------------

bool rewinder::step()
{
	if (!m_enabled)
	{
		report_error(STATERR_DISABLED, rewind_operation::LOAD);
		return false;
	}

	// do we have states to load?
	if (m_current_index <= REWIND_INDEX_FIRST || m_first_invalid_index == REWIND_INDEX_FIRST)
	{
		// no valid states, complain and evacuate
		report_error(STATERR_NOT_FOUND, rewind_operation::LOAD);
		return false;
	}

	// prepare to load the last valid index if we're too far ahead
	if (m_first_invalid_index > REWIND_INDEX_NONE && m_current_index > m_first_invalid_index)
		m_current_index = m_first_invalid_index;

	// step back and obtain the state pointer
	ram_state *state = m_state_list.at(--m_current_index).get();

	// try to load and report the result
	const save_error error = state->load();
	report_error(error, rewind_operation::LOAD);

	if (error == save_error::STATERR_NONE)
		return true;

	return false;
}


//-------------------------------------------------
//  check_size - shrink the state list if it is
//  about to hit the capacity. returns true if
//  the list got shrank
//-------------------------------------------------

bool rewinder::check_size()
{
	if (!m_enabled)
		return false;

	// state sizes in bytes
	const size_t singlesize = m_save.binary_size();
	size_t totalsize = m_state_list.size() * singlesize;

	// convert our limit from megabytes
	const size_t capsize = m_capacity * 1024 * 1024;

	// safety check that shouldn't be allowed to trigger
	if (totalsize > capsize)
	{
		// states to remove
		const u32 count = (totalsize - capsize) / singlesize;

		// drop everything that's beyond capacity
		m_state_list.erase(m_state_list.begin(), m_state_list.begin() + count);
	}

	// update before new check
	totalsize = m_state_list.size() * singlesize;

	// check if capacity will be hit by the newly captured state
	if (totalsize + singlesize >= capsize)
	{
		// check if we have spare states ahead
		if (!current_index_is_last())
			// no need to move states around
			return false;

		// we can now get the first state and invalidate it
		std::unique_ptr<ram_state> first(std::move(m_state_list.front()));
		first->m_valid = false;

		// move it to the end for future use
		m_state_list.push_back(std::move(first));
		m_state_list.erase(m_state_list.begin());

		if (m_first_time_note)
		{
			m_save.machine().logerror("Rewind note: Capacity has been reached. Old savestates will be erased.\n");
			m_save.machine().logerror("Capacity: %d bytes. Savestate size: %d bytes. Savestate count: %d.\n",
				totalsize, singlesize, m_state_list.size());
			m_first_time_note = false;
		}

		return true;
	}

	return false;
}


//-------------------------------------------------
//  report_error - report rewind results
//-------------------------------------------------

void rewinder::report_error(save_error error, rewind_operation operation)
{
	const char *const opname = (operation == rewind_operation::LOAD) ? "load" : "save";
	switch (error)
	{
	// internal saveload failures
	case STATERR_ILLEGAL_REGISTRATIONS:
		m_save.machine().logerror("Rewind error: Unable to %s state due to illegal registrations.", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;

	case STATERR_INVALID_HEADER:
		m_save.machine().logerror("Rewind error: Unable to %s state due to an invalid header. "
			"Make sure the save state is correct for this machine.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;

	case STATERR_READ_ERROR:
		m_save.machine().logerror("Rewind error: Unable to %s state due to a read error.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;

	case STATERR_WRITE_ERROR:
		m_save.machine().logerror("Rewind error: Unable to %s state due to a write error.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;

	// external saveload failures
	case STATERR_NOT_FOUND:
		if (operation == rewind_operation::LOAD)
		{
			m_save.machine().logerror("Rewind error: No rewind state to load.\n");
			m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		}
		break;

	case STATERR_DISABLED:
		if (operation == rewind_operation::LOAD)
		{
			m_save.machine().logerror("Rewind error: Rewind is disabled.\n");
			m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		}
		break;

	// success
	case STATERR_NONE:
		{
			const u64 supported = m_save.machine().system().flags & MACHINE_SUPPORTS_SAVE;
			const char *const warning = supported || !m_first_time_warning ? "" :
				"Rewind warning: Save states are not officially supported for this machine.\n";
			const char *const opnamed = (operation == rewind_operation::LOAD) ? "loaded" : "captured";

			// for rewinding outside of debugger, give some indication that rewind has worked, as screen doesn't update
			m_save.machine().popmessage("Rewind state %i %s.\n%s", m_current_index + 1, opnamed, warning);
			if (m_first_time_warning && operation == rewind_operation::LOAD && !supported)
			{
				m_save.machine().logerror(warning);
				m_first_time_warning = false;
			}
		}
		break;

	// something that shouldn't be allowed to happen
	default:
		m_save.machine().logerror("Error: Unknown error during state %s.\n", opname);
		m_save.machine().popmessage("Rewind error occured. See error.log for details.");
		break;
	}
}


void save_manager::test_dump()
{
	save_zip_state state;
	m_root_item.save_json(state);
	printf("%s\n", state.json_string());
}


save_zip_state::save_zip_state() :
	m_json_reserved(0),
	m_json_offset(0)
{
	json_check_reserve();
}

void save_zip_state::commit(FILE &output)
{
//	add_data_file("save.json", &m_json.str()[0], m_json.str().length());
}
