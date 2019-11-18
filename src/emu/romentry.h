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

/* ----- type constants ----- */
#define ROMENTRY_TYPEMASK           0x0000000f          /* type of entry */
enum
{
	ROMENTRYTYPE_ROM = 0,       /* this entry is an actual ROM definition */
	ROMENTRYTYPE_REGION,        /* this entry marks the start of a region */
	ROMENTRYTYPE_END,           /* this entry marks the end of a region */
	ROMENTRYTYPE_RELOAD,        /* this entry reloads the previous ROM */
	ROMENTRYTYPE_CONTINUE,      /* this entry continues loading the previous ROM */
	ROMENTRYTYPE_FILL,          /* this entry fills an area with a constant value */
	ROMENTRYTYPE_COPY,          /* this entry copies data from another region/offset */
	ROMENTRYTYPE_CARTRIDGE,     /* this entry specifies a cartridge (MESS) */
	ROMENTRYTYPE_IGNORE,        /* this entry continues loading the previous ROM but throws the data away */
	ROMENTRYTYPE_SYSTEM_BIOS,   /* this entry specifies a bios */
	ROMENTRYTYPE_DEFAULT_BIOS,  /* this entry specifies a default bios */
	ROMENTRYTYPE_PARAMETER,     /* this entry specifies a per-game parameter */
	ROMENTRYTYPE_COUNT
};

/* ----- per-region constants ----- */
#define ROMREGION_WIDTHMASK         0x00000300          /* native width of region, as power of 2 */
#define     ROMREGION_8BIT          0x00000000          /*    (non-CPU regions only) */
#define     ROMREGION_16BIT         0x00000100
#define     ROMREGION_32BIT         0x00000200
#define     ROMREGION_64BIT         0x00000300

#define ROMREGION_ENDIANMASK        0x00000400          /* endianness of the region */
#define     ROMREGION_LE            0x00000000          /*    (non-CPU regions only) */
#define     ROMREGION_BE            0x00000400

#define ROMREGION_INVERTMASK        0x00000800          /* invert the bits of the region */
#define     ROMREGION_NOINVERT      0x00000000
#define     ROMREGION_INVERT        0x00000800

#define ROMREGION_ERASEMASK         0x00002000          /* erase the region before loading */
#define     ROMREGION_NOERASE       0x00000000
#define     ROMREGION_ERASE         0x00002000

#define ROMREGION_DATATYPEMASK      0x00004000          /* type of region (ROM versus disk) */
#define     ROMREGION_DATATYPEROM   0x00000000
#define     ROMREGION_DATATYPEDISK  0x00004000

#define ROMREGION_ERASEVALMASK      0x00ff0000          /* value to erase the region to */
#define     ROMREGION_ERASEVAL(x)   ((((x) & 0xff) << 16) | ROMREGION_ERASE)
#define     ROMREGION_ERASE00       ROMREGION_ERASEVAL(0)
#define     ROMREGION_ERASEFF       ROMREGION_ERASEVAL(0xff)


	/* ----- per-ROM constants ----- */
#define DISK_READONLYMASK           0x00000010          /* is the disk read-only? */
#define     DISK_READWRITE          0x00000000
#define     DISK_READONLY           0x00000010

#define ROM_OPTIONALMASK            0x00000020          /* optional - won't hurt if it's not there */
#define     ROM_REQUIRED            0x00000000
#define     ROM_OPTIONAL            0x00000020

#define ROM_REVERSEMASK             0x00000040          /* reverse the byte order within a group */
#define     ROM_NOREVERSE           0x00000000
#define     ROM_REVERSE             0x00000040

#define ROM_INHERITFLAGSMASK        0x00000080          /* inherit all flags from previous definition */
#define     ROM_INHERITFLAGS        0x00000080

#define ROM_GROUPMASK               0x00000f00          /* load data in groups of this size + 1 */
#define     ROM_GROUPSIZE(n)        ((((n) - 1) & 15) << 8)
#define     ROM_GROUPBYTE           ROM_GROUPSIZE(1)
#define     ROM_GROUPWORD           ROM_GROUPSIZE(2)
#define     ROM_GROUPDWORD          ROM_GROUPSIZE(4)

#define ROM_SKIPMASK                0x0000f000          /* skip this many bytes after each group */
#define     ROM_SKIP(n)             (((n) & 15) << 12)
#define     ROM_NOSKIP              ROM_SKIP(0)

#define ROM_BITWIDTHMASK            0x000f0000          /* width of data in bits */
#define     ROM_BITWIDTH(n)         (((n) & 15) << 16)
#define     ROM_NIBBLE              ROM_BITWIDTH(4)
#define     ROM_FULLBYTE            ROM_BITWIDTH(8)

#define ROM_BITSHIFTMASK            0x00f00000          /* left-shift count for the bits */
#define     ROM_BITSHIFT(n)         (((n) & 15) << 20)
#define     ROM_NOSHIFT             ROM_BITSHIFT(0)
#define     ROM_SHIFT_NIBBLE_LO     ROM_BITSHIFT(0)
#define     ROM_SHIFT_NIBBLE_HI     ROM_BITSHIFT(4)

#define ROM_BIOSFLAGSMASK           0xff000000          /* only loaded if value matches device bios value */
#define     ROM_BIOS(n)             ((((n) + 1) & 255) << 24)

#define ROM_INHERITEDFLAGS          (ROM_GROUPMASK | ROM_SKIPMASK | ROM_REVERSEMASK | ROM_BITWIDTHMASK | ROM_BITSHIFTMASK | ROM_BIOSFLAGSMASK)


/* ----- start/stop macros ----- */
#define ROM_NAME(name)                              rom_##name
#define ROM_START(name)                             static const tiny_rom_entry ROM_NAME(name)[] = {
#define ROM_END                                     { nullptr, nullptr, 0, 0, ROMENTRYTYPE_END } };


/* ----- ROM region macros ----- */
#define ROM_REGION(length,tag,flags)                { tag, nullptr, 0, length, ROMENTRYTYPE_REGION | (flags) },
#define ROM_REGION16_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_16BIT | ROMREGION_LE)
#define ROM_REGION16_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_16BIT | ROMREGION_BE)
#define ROM_REGION32_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_32BIT | ROMREGION_LE)
#define ROM_REGION32_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_32BIT | ROMREGION_BE)
#define ROM_REGION64_LE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_64BIT | ROMREGION_LE)
#define ROM_REGION64_BE(length,tag,flags)           ROM_REGION(length, tag, (flags) | ROMREGION_64BIT | ROMREGION_BE)


/* ----- core ROM loading macros ----- */
#define ROMX_LOAD(name,offset,length,hash,flags)    { name, hash, offset, length, ROMENTRYTYPE_ROM | (flags) },
#define ROM_LOAD(name,offset,length,hash)           ROMX_LOAD(name, offset, length, hash, 0)
#define ROM_LOAD_OPTIONAL(name,offset,length,hash)  ROMX_LOAD(name, offset, length, hash, ROM_OPTIONAL)


/* ----- specialized loading macros ----- */
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


/* ----- ROM_RELOAD related macros ----- */
#define ROM_RELOAD(offset,length)                   { nullptr, nullptr, offset, length, ROMENTRYTYPE_RELOAD | ROM_INHERITFLAGS },
#define ROM_RELOAD_PLAIN(offset,length)             { nullptr, nullptr, offset, length, ROMENTRYTYPE_RELOAD },

/* ----- additional ROM-related macros ----- */
#define ROM_CONTINUE(offset,length)                 { nullptr,  nullptr,                 (offset), (length), ROMENTRYTYPE_CONTINUE | ROM_INHERITFLAGS },
#define ROM_IGNORE(length)                          { nullptr,  nullptr,                 0,        (length), ROMENTRYTYPE_IGNORE | ROM_INHERITFLAGS },
#define ROM_FILL(offset,length,value)               { nullptr,  (const char *)(value),   (offset), (length), ROMENTRYTYPE_FILL },
#define ROMX_FILL(offset,length,value,flags)        { nullptr,  (const char *)(value),   (offset), (length), ROMENTRYTYPE_FILL | flags },
#define ROM_COPY(srctag,srcoffs,offset,length)      { (srctag), (const char *)(srcoffs), (offset), (length), ROMENTRYTYPE_COPY },


/* ----- system BIOS macros ----- */
#define ROM_SYSTEM_BIOS(value,name,description)     { name, description, 0, 0, ROMENTRYTYPE_SYSTEM_BIOS | ROM_BIOS(value) },
#define ROM_DEFAULT_BIOS(name)                      { name, nullptr,     0, 0, ROMENTRYTYPE_DEFAULT_BIOS },


/* ----- game parameter macro ----- */
#define ROM_PARAMETER(tag, value)                   { tag, value, 0, 0, ROMENTRYTYPE_PARAMETER },

/* ----- disk loading macros ----- */
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
