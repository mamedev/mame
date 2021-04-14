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
//  INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  save_manager - constructor
//-------------------------------------------------

save_manager::save_manager(running_machine &machine)
	: m_machine(machine)
	, m_reg_allowed(true)
	, m_illegal_regs(0)
	, m_root_registrar(m_root_item)
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
				[] (std::unique_ptr<state_entry> const& a, std::unique_ptr<state_entry> const& b) { return a->m_name < b->m_name; });

		int dupes_found = 0;
		for (int i = 1; i < m_entry_list.size(); i++)
		{
			if (m_entry_list[i - 1]->m_name == m_entry_list[i]->m_name)
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

const char *save_manager::indexed_item(int index, void *&base, u32 &valsize, u32 &valcount, u32 &blockcount, u32 &stride) const
{
	if (index >= m_entry_list.size() || index < 0)
		return nullptr;

	state_entry *entry = m_entry_list.at(index).get();
	base = entry->m_data;
	valsize = entry->m_typesize;
	valcount = entry->m_typecount;
	blockcount = entry->m_blockcount;
	stride = entry->m_stride;

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
	return do_write(
			[] (size_t total_size) { return true; },
			[&file] (const void *data, size_t size) { return file.write(data, size) == size; },
			[&file] ()
			{
				file.compress(FCOMPRESS_NONE);
				file.seek(0, SEEK_SET);
				return true;
			},
			[&file] ()
			{
				file.compress(FCOMPRESS_MEDIUM);
				return true;
			});
}


//-------------------------------------------------
//  read_file - read the data from a file
//-------------------------------------------------

save_error save_manager::read_file(emu_file &file)
{
	return do_read(
			[] (size_t total_size) { return true; },
			[&file] (void *data, size_t size) { return file.read(data, size) == size; },
			[&file] ()
			{
				file.compress(FCOMPRESS_NONE);
				file.seek(0, SEEK_SET);
				return true;
			},
			[&file] ()
			{
				file.compress(FCOMPRESS_MEDIUM);
				return true;
			});
}


//-------------------------------------------------
//  write_stream - write the current machine state
//  to an output stream
//-------------------------------------------------

save_error save_manager::write_stream(std::ostream &str)
{
	return do_write(
			[] (size_t total_size) { return true; },
			[&str] (const void *data, size_t size)
			{
				return bool(str.write(reinterpret_cast<const char *>(data), size));
			},
			[] () { return true; },
			[] () { return true; });
}


//-------------------------------------------------
//  read_stream - restore the machine state from
//  an input stream
//-------------------------------------------------

save_error save_manager::read_stream(std::istream &str)
{
	return do_read(
			[] (size_t total_size) { return true; },
			[&str] (void *data, size_t size)
			{
				return bool(str.read(reinterpret_cast<char *>(data), size));
			},
			[] () { return true; },
			[] () { return true; });
}


//-------------------------------------------------
//  write_buffer - write the current machine state
//  to an allocated buffer
//-------------------------------------------------

save_error save_manager::write_buffer(void *buf, size_t size)
{
	return do_write(
			[size] (size_t total_size) { return size == total_size; },
			[ptr = reinterpret_cast<u8 *>(buf)] (const void *data, size_t size) mutable
			{
				memcpy(ptr, data, size);
				ptr += size;
				return true;
			},
			[] () { return true; },
			[] () { return true; });
}


//-------------------------------------------------
//  read_buffer - restore the machine state from a
//  buffer
//-------------------------------------------------

save_error save_manager::read_buffer(const void *buf, size_t size)
{
	const u8 *ptr = reinterpret_cast<const u8 *>(buf);
	const u8 *const end = ptr + size;
	return do_read(
			[size] (size_t total_size) { return size == total_size; },
			[&ptr, &end] (void *data, size_t size) -> bool
			{
				if ((ptr + size) > end)
					return false;
				memcpy(data, ptr, size);
				ptr += size;
				return true;
			},
			[] () { return true; },
			[] () { return true; });
}


//-------------------------------------------------
//  do_write - serialisation logic
//-------------------------------------------------

template <typename T, typename U, typename V, typename W>
inline save_error save_manager::do_write(T check_space, U write_block, V start_header, W start_data)
{
	// if we have illegal registrations, return an error
	if (m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// check for sufficient space
	size_t total_size = HEADER_SIZE;
	for (const auto &entry : m_entry_list)
		total_size += entry->m_typesize * entry->m_typecount * entry->m_blockcount;
	if (!check_space(total_size))
		return STATERR_WRITE_ERROR;

	// generate the header
	u8 header[HEADER_SIZE];
	memcpy(&header[0], STATE_MAGIC_NUM, 8);
	header[8] = SAVE_VERSION;
	header[9] = NATIVE_ENDIAN_VALUE_LE_BE(0, SS_MSB_FIRST);
	strncpy((char *)&header[0x0a], machine().system().name, 0x1c - 0x0a);
	u32 sig = signature();
	*(u32 *)&header[0x1c] = little_endianize_int32(sig);

	// write the header and turn on compression for the rest of the file
	if (!start_header() || !write_block(header, sizeof(header)) || !start_data())
		return STATERR_WRITE_ERROR;

	// call the pre-save functions
	dispatch_presave();

	// then write all the data
	for (auto &entry : m_entry_list)
	{
		const u32 blocksize = entry->m_typesize * entry->m_typecount;
		const u8 *data = reinterpret_cast<const u8 *>(entry->m_data);
		for (u32 b = 0; entry->m_blockcount > b; ++b, data += entry->m_stride)
			if (!write_block(data, blocksize))
				return STATERR_WRITE_ERROR;
	}
	return STATERR_NONE;
}


//-------------------------------------------------
//  do_read - deserialisation logic
//-------------------------------------------------

template <typename T, typename U, typename V, typename W>
inline save_error save_manager::do_read(T check_length, U read_block, V start_header, W start_data)
{
	// if we have illegal registrations, return an error
	if (m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// check for sufficient space
	size_t total_size = HEADER_SIZE;
	for (const auto &entry : m_entry_list)
		total_size += entry->m_typesize * entry->m_typecount * entry->m_blockcount;
	if (!check_length(total_size))
		return STATERR_READ_ERROR;

	// read the header and turn on compression for the rest of the file
	u8 header[HEADER_SIZE];
	if (!start_header() || !read_block(header, sizeof(header)) || !start_data())
		return STATERR_READ_ERROR;

	// verify the header and report an error if it doesn't match
	u32 sig = signature();
	if (validate_header(header, machine().system().name, sig, nullptr, "Error: ")  != STATERR_NONE)
		return STATERR_INVALID_HEADER;

	// determine whether or not to flip the data when done
	const bool flip = NATIVE_ENDIAN_VALUE_LE_BE((header[9] & SS_MSB_FIRST) != 0, (header[9] & SS_MSB_FIRST) == 0);

	// read all the data, flipping if necessary
	for (auto &entry : m_entry_list)
	{
		const u32 blocksize = entry->m_typesize * entry->m_typecount;
		u8 *data = reinterpret_cast<u8 *>(entry->m_data);
		for (u32 b = 0; entry->m_blockcount > b; ++b, data += entry->m_stride)
			if (!read_block(data, blocksize))
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
		u32 temp[4];
		temp[0] = little_endianize_int32(entry->m_typesize);
		temp[1] = little_endianize_int32(entry->m_typecount);
		temp[2] = little_endianize_int32(entry->m_blockcount);
		temp[3] = little_endianize_int32(entry->m_stride);
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
		LOG(("%s: %u x %u x %u (%u)\n", entry->m_name.c_str(), entry->m_typesize, entry->m_typecount, entry->m_blockcount, entry->m_stride));
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
		totalsize += entry->m_typesize * entry->m_typecount * entry->m_blockcount;

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

	// get the save manager to write state
	const save_error err = m_save.write_stream(m_data);
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
	// initialize
	m_data.seekg(0);

	// if we have illegal registrations, return an error
	if (m_save.m_illegal_regs > 0)
		return STATERR_ILLEGAL_REGISTRATIONS;

	// get the save manager to load state
	return m_save.read_stream(m_data);
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

save_manager::state_entry::state_entry(
		void *data,
		std::string &&name, device_t *device, std::string &&module, std::string &&tag, int index,
		u8 size, u32 valcount, u32 blockcount, u32 stride)
	: m_data(data)
	, m_name(std::move(name))
	, m_device(device)
	, m_module(std::move(module))
	, m_tag(std::move(tag))
	, m_index(index)
	, m_typesize(size)
	, m_typecount(valcount)
	, m_blockcount(blockcount)
	, m_stride(stride)
{
}


//-------------------------------------------------
//  flip_data - reverse the endianness of a
//  block of data
//-------------------------------------------------

void save_manager::state_entry::flip_data()
{
	u8 *data = reinterpret_cast<u8 *>(m_data);
	for (u32 b = 0; m_blockcount > b; ++b, data += m_stride)
	{
		u16 *data16;
		u32 *data32;
		u64 *data64;

		switch (m_typesize)
		{
		case 2:
			data16 = reinterpret_cast<u16 *>(data);
			for (u32 count = 0; count < m_typecount; count++)
				data16[count] = swapendian_int16(data16[count]);
			break;

		case 4:
			data32 = reinterpret_cast<u32 *>(data);
			for (u32 count = 0; count < m_typecount; count++)
				data32[count] = swapendian_int32(data32[count]);
			break;

		case 8:
			data64 = reinterpret_cast<u64 *>(data);
			for (u32 count = 0; count < m_typecount; count++)
				data64[count] = swapendian_int64(data64[count]);
			break;
		}
	}
}


//**************************************************************************
//  INITIALIZATION
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

static std::string type_string(save_registered_item::save_type type, uint32_t native_size)
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
printf("%s '%s': adding %s '%s' @ %llX, size %d\n", type_string(m_type, m_native_size).c_str(), m_name.c_str(), type_string(type, native_size).c_str(), name, ptr_offset, native_size);
	m_items.emplace_back(ptr_offset, type, native_size, name);
	return m_items.back();
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

void save_registered_item::save_json(save_zip_state &zipstate, int indent, bool inline_form, uintptr_t objbase)
{
	// update the base pointer and forward if a trivial unwrap
	if (unwrap_and_update_objbase(objbase))
		return m_items.front().save_json(zipstate, indent, inline_form, objbase);

	// output the name if present
	auto &output = zipstate.json();
	if (m_name.length() > 0)
		output << "\"" << m_name << "\": ";

	// switch off the type
	switch (m_type)
	{
		// boolean types
		case TYPE_BOOL:
			output << (*reinterpret_cast<bool const *>(objbase) ? "true" : "false");
			break;

		// signed integral types
		case TYPE_INT:
		{
			int64_t value = read_int_signed(objbase, m_native_size);
			char const *quote = (value == int64_t(double(value))) ? "" : "\"";
			output << quote << value << quote;
			break;
		}

		// unsigned integral types
		case TYPE_UINT:
		{
			uint64_t value = read_int_unsigned(objbase, m_native_size);
			char const *quote = (value == uint64_t(double(value))) ? "" : "\"";
			output << quote << "0x" << std::setw(m_native_size * 2) << std::setfill('0') << std::hex << value << quote;
			output << std::setw(0) << std::setfill(' ') << std::dec;
			break;
		}

		// float types
		case TYPE_FLOAT:
		{
			double value = read_float(objbase, m_native_size);
			output << value;
			break;
		}

		// structs and containers iterate over owned items
		case TYPE_CONTAINER:
		case TYPE_STRUCT:
			if (inline_form || compute_binary_size(objbase - m_ptr_offset) <= 16)
			{
				// inline form outputs everything on a single line
				output << "{ ";
				for (auto &item : m_items)
				{
					item.save_json(zipstate, indent, true, objbase);
					if (&item != &m_items.back())
						output << ", ";
				}
				output << " }";
			}
			else
			{
				// normal form outputs each item on its own line, indented
				output << "{" << std::endl;
				for (auto &item : m_items)
				{
					output << std::setw(indent + 1) << std::setfill('\t') << "" << std::setw(0);
					item.save_json(zipstate, indent + 1, false, objbase);
					if (&item != &m_items.back())
						output << ",";
					output << std::endl;
				}
				output << std::setw(indent) << std::setfill('\t') << "" << std::setw(0)
						<< "}";
			}
			break;

		// arrays are multiples of a single item
		default:
			if (m_type < TYPE_ARRAY)
			{
				auto &item = m_items.front();
				uint32_t item_size = item.compute_binary_size(objbase);
				if (inline_form || m_type * item_size <= 16)
				{
					// strictly inline form outputs everything on a single line
					output << "[ ";
					for (uint32_t rep = 0; rep < m_type; rep++)
					{
						item.save_json(zipstate, 0, true, objbase + rep * m_native_size);
						if (rep != m_type - 1)
							output << ",";
					}
					output << " ]";
				}
				else
				{
					// normal form outputs a certain number of items per row
					output << "[" << std::endl;
					uint32_t items_per_row = 0;
					if (item.m_type == TYPE_INT || item.m_type == TYPE_UINT || item.m_type == TYPE_FLOAT)
						items_per_row = 32 / item_size;
					if (items_per_row == 0)
						items_per_row = 1;

					// iterate over the items
					for (uint32_t rep = 0; rep < m_type; rep++)
					{
						if (rep % items_per_row == 0)
							output << std::setw(indent + 1) << std::setfill('\t') << "" << std::setw(0);
						item.save_json(zipstate, indent + 1, false, objbase + rep * m_native_size);
						if (rep != m_type - 1)
							output << ",";
						if (rep % items_per_row == items_per_row - 1)
							output << std::endl;
					}
					if (m_type % items_per_row != 0)
							output << std::endl;
					output << std::setw(indent) << std::setfill('\t') << "" << std::setw(0)
							<< "]";
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

void save_manager::test_dump()
{
	save_zip_state state;
	m_root_item.save_json(state);
	printf("%s\n", state.json().str().c_str());
}


save_zip_state::save_zip_state()
{
}

void save_zip_state::add_data_file(char const *name, void *base, uint32_t size)
{
	m_file_list.emplace_back(name, base, size);
}

void save_zip_state::commit(FILE &output)
{
	add_data_file("save.json", &m_json.str()[0], m_json.str().length());
}

save_zip_state::file_entry::file_entry(char const *name, void *base, uint32_t size) :
	m_name(name),
	m_base(base),
	m_size(size)
{
}
