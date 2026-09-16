// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/*********************************************************************

    romentry.h

    ROM loading functions.

*********************************************************************/

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_ROMENTRY_H
#define MAME_EMU_ROMENTRY_H

#include "emucore.h"
#include "osdcomm.h"

#include <string>


/***************************************************************************
CONSTANTS
***************************************************************************/

// ----- type constants -----
static constexpr u32 ROMENTRY_TYPEMASK      = 0x0000000f;       // type of entry
enum
{
	ROMENTRYTYPE_ROM = 0,       // starts loading a ROM file
	ROMENTRYTYPE_REGION,        // starts a new ROM region
	ROMENTRYTYPE_END,           // sentinel marking the end of a ROM definition
	ROMENTRYTYPE_RELOAD,        // starts loading the current ROM file from the beginning again
	ROMENTRYTYPE_CONTINUE,      // continues loading the current ROM file at a different offset
	ROMENTRYTYPE_FILL,          // fills an area with a constant value
	ROMENTRYTYPE_COPY,          // copies data from another region/offset
	ROMENTRYTYPE_IGNORE,        // ignores a portion of the current ROM file
	ROMENTRYTYPE_SYSTEM_BIOS,   // specifies a BIOS option
	ROMENTRYTYPE_DEFAULT_BIOS,  // specifies the default BIOS option
	ROMENTRYTYPE_PARAMETER,     // specifies a per-game parameter
	ROMENTRYTYPE_COUNT
};

// ----- per-region constants -----

constexpr u32 ROMREGION_WIDTHMASK    = 0x00000300;       // native width of region, as power of 2
constexpr u32 ROMREGION_8BIT         = 0x00000000;       //    (non-CPU regions only)
constexpr u32 ROMREGION_16BIT        = 0x00000100;
constexpr u32 ROMREGION_32BIT        = 0x00000200;
constexpr u32 ROMREGION_64BIT        = 0x00000300;

constexpr u32 ROMREGION_ENDIANMASK   = 0x00000400;       // endianness of the region
constexpr u32 ROMREGION_LE           = 0x00000000;       //    (non-CPU regions only)
constexpr u32 ROMREGION_BE           = 0x00000400;

constexpr u32 ROMREGION_INVERTMASK   = 0x00000800;       // invert the bits of the region
constexpr u32 ROMREGION_NOINVERT     = 0x00000000;
constexpr u32 ROMREGION_INVERT       = 0x00000800;

constexpr u32 ROMREGION_ERASEMASK    = 0x00002000;       // erase the region before loading
constexpr u32 ROMREGION_NOERASE      = 0x00000000;
constexpr u32 ROMREGION_ERASE        = 0x00002000;

constexpr u32 ROMREGION_DATATYPEMASK = 0x00004000;       // type of region (ROM versus disk)
constexpr u32 ROMREGION_DATATYPEROM  = 0x00000000;
constexpr u32 ROMREGION_DATATYPEDISK = 0x00004000;

constexpr u32 ROMREGION_ERASEVALMASK = 0x00ff0000;       // value to erase the region to
constexpr u32 ROMREGION_ERASEVAL(u8 x) { return (u32(x) << 16) | ROMREGION_ERASE; }
constexpr u32 ROMREGION_ERASE00      = ROMREGION_ERASEVAL(0);
constexpr u32 ROMREGION_ERASEFF      = ROMREGION_ERASEVAL(0xff);

// ----- per-ROM constants -----

constexpr u32 DISK_READONLYMASK      = 0x00000010;       // is the disk read-only?
constexpr u32 DISK_READWRITE         = 0x00000000;
constexpr u32 DISK_READONLY          = 0x00000010;

constexpr u32 ROM_OPTIONALMASK       = 0x00000020;       // optional - won't hurt if it's not there
constexpr u32 ROM_REQUIRED           = 0x00000000;
[[deprecated("use image devices, slot devices or BIOS options for truly optional files")]]
constexpr u32 ROM_OPTIONAL           = 0x00000020;

constexpr u32 ROM_REVERSEMASK        = 0x00000040;       // reverse the byte order within a group
constexpr u32 ROM_NOREVERSE          = 0x00000000;
constexpr u32 ROM_REVERSE            = 0x00000040;

constexpr u32 ROM_INHERITFLAGSMASK   = 0x00000080;       // inherit all flags from previous definition
constexpr u32 ROM_INHERITFLAGS       = 0x00000080;

constexpr u32 ROM_GROUPMASK          = 0x00000f00;       // load data in groups of this size + 1
constexpr u32 ROM_GROUPSIZE(unsigned n) { return ((n - 1) & 15) << 8; }
constexpr u32 ROM_GROUPBYTE          = ROM_GROUPSIZE(1);
constexpr u32 ROM_GROUPWORD          = ROM_GROUPSIZE(2);
constexpr u32 ROM_GROUPDWORD         = ROM_GROUPSIZE(4);

constexpr u32 ROM_SKIPMASK           = 0x0000f000;       // skip this many bytes after each group
constexpr u32 ROM_SKIP(unsigned n) { return (n & 15) << 12; }
constexpr u32 ROM_NOSKIP             = ROM_SKIP(0);

constexpr u32 ROM_BITWIDTHMASK       = 0x000f0000;       // width of data in bits
constexpr u32 ROM_BITWIDTH(unsigned n) { return u32(n & 15) << 16; }
constexpr u32 ROM_NIBBLE             = ROM_BITWIDTH(4);
constexpr u32 ROM_FULLBYTE           = ROM_BITWIDTH(8);

constexpr u32 ROM_BITSHIFTMASK       = 0x00f00000;       // left-shift count for the bits
constexpr u32 ROM_BITSHIFT(unsigned n) { return u32(n & 15) << 20; }
constexpr u32 ROM_NOSHIFT            = ROM_BITSHIFT(0);
constexpr u32 ROM_SHIFT_NIBBLE_LO    = ROM_BITSHIFT(0);
constexpr u32 ROM_SHIFT_NIBBLE_HI    = ROM_BITSHIFT(4);

constexpr u32 ROM_BIOSFLAGSMASK      = 0xff000000;       // only loaded if value matches device BIOS value
constexpr u32 ROM_BIOS(unsigned n) { return u32((n + 1) & 255) << 24; }

constexpr u32 ROM_INHERITEDFLAGS = ROM_GROUPMASK | ROM_SKIPMASK | ROM_REVERSEMASK | ROM_BITWIDTHMASK | ROM_BITSHIFTMASK | ROM_BIOSFLAGSMASK;


// ----- start/stop macros -----
#define ROM_NAME(name)                              rom_##name
#define ROM_START(name)                             static const tiny_rom_entry ROM_NAME(name)[] = {
#define ROM_END                                     { nullptr, nullptr, 0, 0, ROMENTRYTYPE_END } };


// ----- ROM region macros -----
#define ROM_REGION(length,tag,flags)                { tag, nullptr, 0, length, ROMENTRYTYPE_REGION | (flags) },
#define ROM_REGION16_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_16BIT | ROMREGION_LE)
#define ROM_REGION16_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_16BIT | ROMREGION_BE)
#define ROM_REGION32_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_32BIT | ROMREGION_LE)
#define ROM_REGION32_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_32BIT | ROMREGION_BE)
#define ROM_REGION64_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_64BIT | ROMREGION_LE)
#define ROM_REGION64_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_64BIT | ROMREGION_BE)


// ----- core ROM loading macros -----
#define ROMX_LOAD(name,offset,length,hash,flags)    { name, hash, offset, length, ROMENTRYTYPE_ROM | (flags) },
#define ROM_LOAD(name,offset,length,hash)           ROMX_LOAD(name, offset, length, hash, 0)
#define ROM_LOAD_OPTIONAL(name,offset,length,hash)  ROMX_LOAD(name, offset, length, hash, ROM_OPTIONAL)


// ----- specialized loading macros -----
#define ROM_LOAD_NIB_HIGH(name,offset,length,hash)      ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_HI)
#define ROM_LOAD_NIB_LOW(name,offset,length,hash)       ROMX_LOAD(name, offset, length, hash, ROM_NIBBLE | ROM_SHIFT_NIBBLE_LO)
#define ROM_LOAD16_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(1))
#define ROM_LOAD16_WORD(name,offset,length,hash)        ROM_LOAD(name, offset, length, hash)
#define ROM_LOAD16_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE)
#define ROM_LOAD32_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(3))
#define ROM_LOAD32_WORD(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(2))
#define ROM_LOAD32_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
#define ROM_LOAD32_DWORD(name,offset,length,hash)       ROMX_LOAD(name, offset, length, hash, ROM_GROUPDWORD)
#define ROM_LOAD64_BYTE(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_SKIP(7))
#define ROM_LOAD64_WORD(name,offset,length,hash)        ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6))
#define ROM_LOAD64_WORD_SWAP(name,offset,length,hash)   ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(6))
#define ROM_LOAD64_DWORD_SWAP(name,offset,length,hash)  ROMX_LOAD(name, offset, length, hash, ROM_GROUPDWORD | ROM_REVERSE | ROM_SKIP(4))


// ----- ROM_RELOAD related macros -----
#define ROM_RELOAD(offset,length)                   { nullptr, nullptr, offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS },
#define ROM_RELOAD_PLAIN(offset,length)             { nullptr, nullptr, offset, length, ROMENTRYTYPE_RELOAD },

// ----- additional ROM-related macros -----
#define ROM_CONTINUE(offset,length)                 { nullptr,  nullptr,                 (offset), (length), ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS },
#define ROM_IGNORE(length)                          { nullptr,  nullptr,                 0,        (length), ROMENTRYTYPE_IGNORE | ROM_INHERITFLAGS },
#define ROM_FILL(offset,length,value)               { nullptr,  (const char *)(value),   (offset), (length), ROMENTRYTYPE_FILL },
#define ROMX_FILL(offset,length,value,flags)        { nullptr,  (const char *)(value),   (offset), (length), ROMENTRYTYPE_FILL | flags },
#define ROM_COPY(srctag,srcoffs,offset,length)      { (srctag), (const char *)(srcoffs), (offset), (length), ROMENTRYTYPE_COPY },


// ----- system BIOS macros -----
#define ROM_SYSTEM_BIOS(value,name,description)     { name, description, 0, 0, ROMENTRYTYPE_SYSTEM_BIOS | ROM_BIOS(value) },
#define ROM_DEFAULT_BIOS(name)                      { name, nullptr,     0, 0, ROMENTRYTYPE_DEFAULT_BIOS },


// ----- game parameter macro -----
#define ROM_PARAMETER(tag, value)                   { tag, value, 0, 0, ROMENTRYTYPE_PARAMETER },

// ----- disk loading macros -----
#define DISK_REGION(tag)                            ROM_REGION(1, tag, ROMREGION_DATATYPEDISK)
#define DISK_IMAGE(name,idx,hash)                   ROMX_LOAD(name, idx, 0, hash, DISK_READWRITE)
#define DISK_IMAGE_READONLY(name,idx,hash)          ROMX_LOAD(name, idx, 0, hash, DISK_READONLY)
#define DISK_IMAGE_READONLY_OPTIONAL(name,idx,hash) ROMX_LOAD(name, idx, 0, hash, DISK_READONLY | ROM_OPTIONAL)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tiny_rom_entry

struct tiny_rom_entry
{
	char const *name;
	char const *hashdata;
	u32         offset;
	u32         length;
	u32         flags;

	constexpr u32 get_offset() const { return offset; }
	constexpr u32 get_length() const { return length; }
	constexpr u32 get_flags() const { return flags; }
};


// ======================> rom_entry

class rom_entry
{
public:
	rom_entry(const tiny_rom_entry &ent);
	rom_entry(std::string &&name, std::string &&hashdata, u32 offset, u32 length, u32 flags);
	rom_entry(rom_entry const &) = default;
	rom_entry(rom_entry &&) = default;
	rom_entry &operator=(rom_entry const &) = default;
	rom_entry &operator=(rom_entry &&) = default;

	// accessors
	std::string const &name() const { return m_name; }
	std::string const &hashdata() const { return m_hashdata; }
	u32 get_offset() const { return m_offset; }
	u32 get_length() const { return m_length; }
	u32 get_flags() const { return m_flags; }
	void set_flags(u32 flags) { m_flags = flags; }

private:
	std::string     m_name;
	std::string     m_hashdata;
	u32             m_offset;
	u32             m_length;
	u32             m_flags;
};


#endif // MAME_EMU_ROMENTRY_H
