// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/*********************************************************************

    romentry.cpp

    ROM loading functions.

*********************************************************************/

#include "emu.h"
#include "romentry.h"

#include "strformat.h"


/***************************************************************************
    HELPERS
***************************************************************************/

//-------------------------------------------------
//  hashdata_from_tiny_rom_entry - calculates the
//  proper hashdata string from the value in the
//  tiny_rom_entry
//-------------------------------------------------

static std::string hashdata_from_tiny_rom_entry(const tiny_rom_entry &ent)
{
	std::string result;
	switch (ent.flags & ROMENTRY_TYPEMASK)
	{
	case ROMENTRYTYPE_FILL:
	case ROMENTRYTYPE_COPY:
		// for these types, tiny_rom_entry::hashdata is an integer typecasted to a pointer
		result = string_format("0x%x", (unsigned)(uintptr_t)ent.hashdata);
		break;

	default:
		if (ent.hashdata != nullptr)
			result.assign(ent.hashdata);
		break;
	}
	return result;
}


/***************************************************************************
    ROM ENTRY
***************************************************************************/

//-------------------------------------------------
//  ctor (with move constructors)
//-------------------------------------------------

rom_entry::rom_entry(std::string &&name, std::string &&hashdata, u32 offset, u32 length, u32 flags)
	: m_name(std::move(name))
	, m_hashdata(std::move(hashdata))
	, m_offset(offset)
	, m_length(length)
	, m_flags(flags)
{
}


//-------------------------------------------------
//  ctor (with tiny_rom_entry)
//-------------------------------------------------

rom_entry::rom_entry(const tiny_rom_entry &ent)
	: m_name(ent.name != nullptr ? ent.name : "")
	, m_hashdata(hashdata_from_tiny_rom_entry(ent))
	, m_offset(ent.offset)
	, m_length(ent.length)
	, m_flags(ent.flags)
{
}
