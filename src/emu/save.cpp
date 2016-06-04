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
	: m_machine(machine),
		m_reg_allowed(true),
		m_illegal_regs(0)
{
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
		dump_registry();
}


//-------------------------------------------------
//  indexed_item - return an item with the given
//  index
//-------------------------------------------------

const char *save_manager::indexed_item(int index, void *&base, UINT32 &valsize, UINT32 &valcount) const
{
	state_entry *entry = m_entry_list.find(index);
	if (entry == nullptr)
		return nullptr;

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
	for (state_callback &cb : m_presave_list)
		if (cb.m_func == func)
			fatalerror("Duplicate save state function (%s/%s)\n", cb.m_func.name(), func.name());

	// allocate a new entry
	m_presave_list.append(*global_alloc(state_callback(func)));
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
	for (state_callback &cb : m_postload_list)
		if (cb.m_func == func)
			fatalerror("Duplicate save state function (%s/%s)\n", cb.m_func.name(), func.name());

	// allocate a new entry
	m_postload_list.append(*global_alloc(state_callback(func)));
}


//-------------------------------------------------
//  save_memory - register an array of data in
//  memory
//-------------------------------------------------

void save_manager::save_memory(device_t *device, const char *module, const char *tag, UINT32 index, const char *name, void *val, UINT32 valsize, UINT32 valcount)
{
	assert(valsize == 1 || valsize == 2 || valsize == 4 || valsize == 8);

	// check for invalid timing
	if (!m_reg_allowed)
	{
		machine().logerror("Attempt to register save state entry after state registration is closed!\nModule %s tag %s name %s\n", module, tag, name);
		if (machine().system().flags & MACHINE_SUPPORTS_SAVE)
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

	// look for duplicates and an entry to insert in front of
	state_entry *insert_after = nullptr;
	for (state_entry &entry : m_entry_list)
	{
		// stop when we find an entry whose name is after ours
		if (entry.m_name.compare(totalname)>0)
			break;
		insert_after = &entry;

		// error if we are equal
		if (entry.m_name.compare(totalname)==0)
			fatalerror("Duplicate save state registration entry (%s)\n", totalname.c_str());
	}

	// insert us into the list
	m_entry_list.insert_after(*global_alloc(state_entry(val, totalname.c_str(), device, module, tag ? tag : "", index, valsize, valcount)), insert_after);
}


//-------------------------------------------------
//  check_file - check if a file is a valid save
//  state
//-------------------------------------------------

save_error save_manager::check_file(running_machine &machine, emu_file &file, const char *gamename, void (CLIB_DECL *errormsg)(const char *fmt, ...))
{
	// if we want to validate the signature, compute it
	UINT32 sig;
	sig = machine.save().signature();

	// seek to the beginning and read the header
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	UINT8 header[HEADER_SIZE];
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
	for (state_callback &func : m_postload_list)
		func.m_func();
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
	UINT8 header[HEADER_SIZE];
	if (file.read(header, sizeof(header)) != sizeof(header))
		return STATERR_READ_ERROR;
	file.compress(FCOMPRESS_MEDIUM);

	// verify the header and report an error if it doesn't match
	UINT32 sig = signature();
	if (validate_header(header, machine().system().name, sig, nullptr, "Error: ")  != STATERR_NONE)
		return STATERR_INVALID_HEADER;

	// determine whether or not to flip the data when done
	bool flip = NATIVE_ENDIAN_VALUE_LE_BE((header[9] & SS_MSB_FIRST) != 0, (header[9] & SS_MSB_FIRST) == 0);

	// read all the data, flipping if necessary
	for (state_entry &entry : m_entry_list)
	{
		UINT32 totalsize = entry.m_typesize * entry.m_typecount;
		if (file.read(entry.m_data, totalsize) != totalsize)
			return STATERR_READ_ERROR;

		// handle flipping
		if (flip)
			entry.flip_data();
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
	for (state_callback &func : m_presave_list)
		func.m_func();
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
	UINT8 header[HEADER_SIZE];
	memcpy(&header[0], STATE_MAGIC_NUM, 8);
	header[8] = SAVE_VERSION;
	header[9] = NATIVE_ENDIAN_VALUE_LE_BE(0, SS_MSB_FIRST);
	strncpy((char *)&header[0x0a], machine().system().name, 0x1c - 0x0a);
	UINT32 sig = signature();
	*(UINT32 *)&header[0x1c] = LITTLE_ENDIANIZE_INT32(sig);

	// write the header and turn on compression for the rest of the file
	file.compress(FCOMPRESS_NONE);
	file.seek(0, SEEK_SET);
	if (file.write(header, sizeof(header)) != sizeof(header))
		return STATERR_WRITE_ERROR;
	file.compress(FCOMPRESS_MEDIUM);

	// call the pre-save functions
	dispatch_presave();

	// then write all the data
	for (state_entry &entry : m_entry_list)
	{
		UINT32 totalsize = entry.m_typesize * entry.m_typecount;
		if (file.write(entry.m_data, totalsize) != totalsize)
			return STATERR_WRITE_ERROR;
	}
	return STATERR_NONE;
}


//-------------------------------------------------
//  signature - compute the signature, which
//  is a CRC over the structure of the data
//-------------------------------------------------

UINT32 save_manager::signature() const
{
	// iterate over entries
	UINT32 crc = 0;
	for (state_entry &entry : m_entry_list)
	{
		// add the entry name to the CRC
		crc = core_crc32(crc, (UINT8 *)entry.m_name.c_str(), entry.m_name.length());

		// add the type and size to the CRC
		UINT32 temp[2];
		temp[0] = LITTLE_ENDIANIZE_INT32(entry.m_typecount);
		temp[1] = LITTLE_ENDIANIZE_INT32(entry.m_typesize);
		crc = core_crc32(crc, (UINT8 *)&temp[0], sizeof(temp));
	}
	return crc;
}


//-------------------------------------------------
//  dump_registry - dump the registry to the
//  logfile
//-------------------------------------------------

void save_manager::dump_registry() const
{
	for (state_entry &entry : m_entry_list)
		LOG(("%s: %d x %d\n", entry.m_name.c_str(), entry.m_typesize, entry.m_typecount));
}


//-------------------------------------------------
//  validate_header - validate the data in the
//  header
//-------------------------------------------------

save_error save_manager::validate_header(const UINT8 *header, const char *gamename, UINT32 signature,
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
		UINT32 rawsig = *(UINT32 *)&header[0x1c];
		if (signature != LITTLE_ENDIANIZE_INT32(rawsig))
		{
			if (errormsg != nullptr)
				(*errormsg)("%sIncompatible save file (signature %08x, expected %08x)", error_prefix, LITTLE_ENDIANIZE_INT32(rawsig), signature);
			return STATERR_INVALID_HEADER;
		}
	}
	return STATERR_NONE;
}


//-------------------------------------------------
//  state_callback - constructor
//-------------------------------------------------

save_manager::state_callback::state_callback(save_prepost_delegate callback)
	: m_next(nullptr),
		m_func(std::move(callback))
{
}


//-------------------------------------------------
//  state_entry - constructor
//-------------------------------------------------

state_entry::state_entry(void *data, const char *name, device_t *device, const char *module, const char *tag, int index, UINT8 size, UINT32 count)
	: m_next(nullptr),
		m_data(data),
		m_name(name),
		m_device(device),
		m_module(module),
		m_tag(tag),
		m_index(index),
		m_typesize(size),
		m_typecount(count),
		m_offset(0)
{
}


//-------------------------------------------------
//  flip_data - reverse the endianness of a
//  block of  data
//-------------------------------------------------

void state_entry::flip_data()
{
	UINT16 *data16;
	UINT32 *data32;
	UINT64 *data64;
	int count;

	switch (m_typesize)
	{
		case 2:
			data16 = (UINT16 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data16[count] = FLIPENDIAN_INT16(data16[count]);
			break;

		case 4:
			data32 = (UINT32 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data32[count] = FLIPENDIAN_INT32(data32[count]);
			break;

		case 8:
			data64 = (UINT64 *)m_data;
			for (count = 0; count < m_typecount; count++)
				data64[count] = FLIPENDIAN_INT64(data64[count]);
			break;
	}
}
