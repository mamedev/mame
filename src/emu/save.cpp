// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    save.c

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
//  INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  save_manager - constructor
//-------------------------------------------------

save_manager::save_manager(running_machine &machine)
	: m_machine(machine)
	, m_reg_allowed(true)
	, m_illegal_regs(0)
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
		// look for duplicates
		std::sort(m_entry_list.begin(), m_entry_list.end(),
			[](std::unique_ptr<state_entry> const& a, std::unique_ptr<state_entry> const& b) { return a->m_name < b->m_name; });

		int dupes_found = 0;
		for (int i = 0; i < m_entry_list.size() - 1; i++)
		{
			if (m_entry_list[i]->m_name == m_entry_list[i + 1]->m_name)
			{
				osd_printf_error("Duplicate save state registration entry (%s)\n", m_entry_list[i]->m_name);
				dupes_found++;
			}
		}

		if (dupes_found)
			fatalerror("%d duplicate save state entries found.\n", dupes_found);

		dump_registry();

		// everything is registered by now, evaluate the savestate size
		m_rewind->clamp_capacity();
	}
}


//-------------------------------------------------
//  indexed_item - return an item with the given
//  index
//-------------------------------------------------

const char *save_manager::indexed_item(int index, void *&base, u32 &valsize, u32 &valcount) const
{
	if (index >= m_entry_list.size() || index < 0)
		return nullptr;

	state_entry *entry = m_entry_list.at(index).get();
	base = entry->m_data;
	valsize = entry->m_typesize;
	valcount = entry->m_typecount;

	return entry->m_name.c_str();
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
//  save_memory - register an array of data in
//  memory
//-------------------------------------------------

void save_manager::save_memory(device_t *device, const char *module, const char *tag, u32 index, const char *name, void *val, u32 valsize, u32 valcount)
{
	assert(valsize == 1 || valsize == 2 || valsize == 4 || valsize == 8);

	// check for invalid timing
	if (!m_reg_allowed)
	{
		machine().logerror("Attempt to register save state entry after state registration is closed!\nModule %s tag %s name %s\n", module, tag, name);
		if (machine().system().flags & machine_flags::SUPPORTS_SAVE)
			fatalerror("Attempt to register save state entry after state registration is closed!\nModule %s tag %s name %s\n", module, tag, name);
		m_illegal_regs++;
		return;
	}

	// create the full name
	std::string totalname;
	if (tag != nullptr)
		totalname = string_format("%s/%s/%X/%s", module, tag, index, name);
	else
		totalname = string_format("%s/%X/%s", module, index, name);

	// insert us into the list
	m_entry_list.emplace_back(std::make_unique<state_entry>(val, totalname.c_str(), device, module, tag ? tag : "", index, valsize, valcount));
}


//-------------------------------------------------
//  check_file - check if a file is a valid save
//  state
//-------------------------------------------------

save_error save_manager::check_file(running_machine &machine, emu_file &file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...))
{
	// if we want to validate the signature, compute it
	u32 sig;
	sig = machine.save().signature();

	// seek to the beginning and read the header
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	u8 header[HEADER_SIZE];
	if (file.read(header, sizeof(header)) != sizeof(header))
	{
		if (errormsg != nullptr)
			(*errormsg)("Could not read %s save file header",emulator_info::get_appname());
		return STATERR_READ_ERROR;
	}

	// let the generic header check work out the rest
	return validate_header(header, gamename, sig, errormsg, "");
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
//  read_file - read the data from a file
//-------------------------------------------------

save_error save_manager::read_file(emu_file &file)
{
	// if we have illegal registrations, return an error
	if (m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// read the header and turn on compression for the rest of the file
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	u8 header[HEADER_SIZE];
	if (file.read(header, sizeof(header)) != sizeof(header))
		return STATERR_READ_ERROR;
	file.compress(FCOMPRESS_MEDIUM);

	// verify the header and report an error if it doesn't match
	u32 sig = signature();
	if (validate_header(header, machine().system().name, sig, nullptr, "Error: ")  != STATERR_NONE)
		return STATERR_INVALID_HEADER;

	// determine whether or not to flip the data when done
	bool flip = NATIVE_ENDIAN_VALUE_LE_BE((header[9] & SS_MSB_FIRST) != 0, (header[9] & SS_MSB_FIRST) == 0);

	// read all the data, flipping if necessary
	for (auto &entry : m_entry_list)
	{
		u32 totalsize = entry->m_typesize * entry->m_typecount;
		if (file.read(entry->m_data, totalsize) != totalsize)
			return STATERR_READ_ERROR;

		// handle flipping
		if (flip)
			entry->flip_data();
	}

	// call the post-load functions
	dispatch_postload();

	return STATERR_NONE;
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
//  write_file - writes the data to a file
//-------------------------------------------------

save_error save_manager::write_file(emu_file &file)
{
	// if we have illegal registrations, return an error
	if (m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// generate the header
	u8 header[HEADER_SIZE];
	memcpy(&header[0], STATE_MAGIC_NUM, 8);
	header[8] = SAVE_VERSION;
	header[9] = NATIVE_ENDIAN_VALUE_LE_BE(0, SS_MSB_FIRST);
	strncpy((char *)&header[0x0a], machine().system().name, 0x1c - 0x0a);
	u32 sig = signature();
	*(u32 *)&header[0x1c] = little_endianize_int32(sig);

	// write the header and turn on compression for the rest of the file
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	if (file.write(header, sizeof(header)) != sizeof(header))
		return STATERR_WRITE_ERROR;
	file.compress(FCOMPRESS_MEDIUM);

	// call the pre-save functions
	dispatch_presave();

	// then write all the data
	for (auto &entry : m_entry_list)
	{
		u32 totalsize = entry->m_typesize * entry->m_typecount;
		if (file.write(entry->m_data, totalsize) != totalsize)
			return STATERR_WRITE_ERROR;
	}
	return STATERR_NONE;
}


//-------------------------------------------------
//  signature - compute the signature, which
//  is a CRC over the structure of the data
//-------------------------------------------------

u32 save_manager::signature() const
{
	// iterate over entries
	u32 crc = 0;
	for (auto &entry : m_entry_list)
	{
		// add the entry name to the CRC
		crc = core_crc32(crc, (u8 *)entry->m_name.c_str(), entry->m_name.length());

		// add the type and size to the CRC
		u32 temp[2];
		temp[0] = little_endianize_int32(entry->m_typecount);
		temp[1] = little_endianize_int32(entry->m_typesize);
		crc = core_crc32(crc, (u8 *)&temp[0], sizeof(temp));
	}
	return crc;
}


//-------------------------------------------------
//  dump_registry - dump the registry to the
//  logfile
//-------------------------------------------------

void save_manager::dump_registry() const
{
	for (auto &entry : m_entry_list)
		LOG(("%s: %d x %d\n", entry->m_name.c_str(), entry->m_typesize, entry->m_typecount));
}


//-------------------------------------------------
//  validate_header - validate the data in the
//  header
//-------------------------------------------------

save_error save_manager::validate_header(const u8 *header, const char *gamename, u32 signature,
	void (CLIB_DECL *errormsg)(const char *fmt, ...), const char *error_prefix)
{
	// check magic number
	if (memcmp(header, STATE_MAGIC_NUM, 8))
	{
		if (errormsg != nullptr)
			(*errormsg)("%sThis is not a %s save file", error_prefix,emulator_info::get_appname());
		return STATERR_INVALID_HEADER;
	}

	// check save state version
	if (header[8] != SAVE_VERSION)
	{
		if (errormsg != nullptr)
			(*errormsg)("%sWrong version in save file (version %d, expected %d)", error_prefix, header[8], SAVE_VERSION);
		return STATERR_INVALID_HEADER;
	}

	// check gamename, if we were asked to
	if (gamename != nullptr && strncmp(gamename, (const char *)&header[0x0a], 0x1c - 0x0a))
	{
		if (errormsg != nullptr)
			(*errormsg)("%s'File is not a valid savestate file for game '%s'.", error_prefix, gamename);
		return STATERR_INVALID_HEADER;
	}

	// check signature, if we were asked to
	if (signature != 0)
	{
		u32 rawsig = *(u32 *)&header[0x1c];
		if (signature != little_endianize_int32(rawsig))
		{
			if (errormsg != nullptr)
				(*errormsg)("%sIncompatible save file (signature %08x, expected %08x)", error_prefix, little_endianize_int32(rawsig), signature);
			return STATERR_INVALID_HEADER;
		}
	}
	return STATERR_NONE;
}


//-------------------------------------------------
//  state_callback - constructor
//-------------------------------------------------

save_manager::state_callback::state_callback(save_prepost_delegate callback)
	: m_func(std::move(callback))
{
}


//-------------------------------------------------
//  ram_state - constructor
//-------------------------------------------------

ram_state::ram_state(save_manager &save)
	: m_save(save)
	, m_data()
	, m_valid(false)
	, m_time(m_save.machine().time())
{
	m_data.reserve(get_size(save));
	m_data.clear();
	m_data.rdbuf()->clear();
	m_data.seekp(0);
	m_data.seekg(0);
}


//-------------------------------------------------
//  get_size - utility function to get the
//  uncompressed size of a state
//-------------------------------------------------

size_t ram_state::get_size(save_manager &save)
{
	size_t totalsize = 0;

	for (auto &entry : save.m_entry_list)
	{
		totalsize += entry->m_typesize * entry->m_typecount;
	}

	return totalsize + HEADER_SIZE;
}


//-------------------------------------------------
//  save - write the current machine state to the
//  allocated stream
//-------------------------------------------------

save_error ram_state::save()
{
	// initialize
	m_valid = false;
	m_data.seekp(0);

	// if we have illegal registrations, return an error
	if (m_save.m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// generate the header
	u8 header[HEADER_SIZE];
	memcpy(&header[0], STATE_MAGIC_NUM, 8);
	header[8] = SAVE_VERSION;
	header[9] = NATIVE_ENDIAN_VALUE_LE_BE(0, SS_MSB_FIRST);
	strncpy((char *)&header[0x0a], m_save.machine().system().name, 0x1c - 0x0a);
	u32 sig = m_save.signature();
	*(u32 *)&header[0x1c] = little_endianize_int32(sig);

	// write the header
	m_data.write((char *)header, sizeof(header));

	// check for any errors
	if (!m_data)
		return STATERR_WRITE_ERROR;

	// call the pre-save functions
	m_save.dispatch_presave();

	// write all the data
	for (auto &entry : m_save.m_entry_list)
	{
		u32 totalsize = entry->m_typesize * entry->m_typecount;
		m_data.write((char *)entry->m_data, totalsize);

		// check for any errors
		if (!m_data)
			return STATERR_WRITE_ERROR;
	}

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
	// initialize
	m_data.seekg(0);

	// if we have illegal registrations, return an error
	if (m_save.m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// read the header
	u8 header[HEADER_SIZE];
	m_data.read((char *)header, sizeof(header));

	// check for any errors
	if (!m_data)
		return STATERR_READ_ERROR;

	// verify the header and report an error if it doesn't match
	u32 sig = m_save.signature();
	if (m_save.validate_header(header, m_save.machine().system().name, sig, nullptr, "Error: ") != STATERR_NONE)
		return STATERR_INVALID_HEADER;

	// determine whether or not to flip the data when done
	bool flip = NATIVE_ENDIAN_VALUE_LE_BE((header[9] & SS_MSB_FIRST) != 0, (header[9] & SS_MSB_FIRST) == 0);

	// read all the data, flipping if necessary
	for (auto &entry : m_save.m_entry_list)
	{
		u32 totalsize = entry->m_typesize * entry->m_typecount;
		m_data.read((char *)entry->m_data, totalsize);

		// check for any errors
		if (!m_data)
			return STATERR_READ_ERROR;

		// handle flipping
		if (flip)
			entry->flip_data();
	}

	// call the post-load functions
	m_save.dispatch_postload();

	return STATERR_NONE;
}


//-------------------------------------------------
//  rewinder - constuctor
//-------------------------------------------------

rewinder::rewinder(save_manager &save)
	: m_save(save)
	, m_enabled(save.machine().options().rewind())
	, m_capacity(save.machine().options().rewind_capacity())
	, m_current_index(REWIND_INDEX_NONE)
	, m_first_invalid_index(REWIND_INDEX_NONE)
	, m_first_time_warning(true)
	, m_first_time_note(true)
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
	const size_t single = ram_state::get_size(m_save);

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
	const size_t singlesize = ram_state::get_size(m_save);
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


//-------------------------------------------------
//  state_entry - constructor
//-------------------------------------------------

state_entry::state_entry(void *data, const char *name, device_t *device, const char *module, const char *tag, int index, u8 size, u32 count)
	: m_data(data)
	, m_name(name)
	, m_device(device)
	, m_module(module)
	, m_tag(tag)
	, m_index(index)
	, m_typesize(size)
	, m_typecount(count)
	, m_offset(0)
{
}


//-------------------------------------------------
//  flip_data - reverse the endianness of a
//  block of data
//-------------------------------------------------

void state_entry::flip_data()
{
	u16 *data16;
	u32 *data32;
	u64 *data64;
	int count;

	switch (m_typesize)
	{
		case 2:
			data16 = (u16 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data16[count] = swapendian_int16(data16[count]);
			break;

		case 4:
			data32 = (u32 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data32[count] = swapendian_int32(data32[count]);
			break;

		case 8:
			data64 = (u64 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data64[count] = swapendian_int64(data64[count]);
			break;
	}
}
