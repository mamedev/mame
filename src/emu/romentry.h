// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Aaron Giles
/*********************************************************************

	romentry.h

	ROM loading functions.

*********************************************************************/

#ifndef MAME_EMU_ROMENTRY_H_
#define MAME_EMU_ROMENTRY_H_

#include <string>

#include "osdcomm.h"

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

#define ROM_BIOSFLAGSMASK           0xff000000          /* only loaded if value matches global bios value */
#define     ROM_BIOS(n)             (((n) & 255) << 24)

#define ROM_INHERITEDFLAGS          (ROM_GROUPMASK | ROM_SKIPMASK | ROM_REVERSEMASK | ROM_BITWIDTHMASK | ROM_BITSHIFTMASK | ROM_BIOSFLAGSMASK)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> rom_entry

class rom_entry
{
public:
	rom_entry(const char *name, const char *hashdata, UINT32 offset, UINT32 length, UINT32 flags)
		: m_name(name != nullptr ? name : "")
		, m_hashdata(hashdata != nullptr ? hashdata : "")
		, m_offset(offset)
		, m_length(length)
		, m_flags(flags) {}
	rom_entry(std::string &&name, std::string &&hashdata, UINT32 offset, UINT32 length, UINT32 flags)
		: m_name(std::move(name))
		, m_hashdata(std::move(hashdata))
		, m_offset(offset)
		, m_length(length)
		, m_flags(flags) {}
	rom_entry(rom_entry const &) = default;
	rom_entry(rom_entry &&) = default;
	rom_entry &operator=(rom_entry const &) = default;
	rom_entry &operator=(rom_entry &&) = default;

	// accessors
	const std::string &name() const { return m_name; }
	const std::string &hashdata() const { return m_hashdata; }
	UINT32 offset() const { return m_offset; }
	UINT32 length() const { return m_length; }
	UINT32 flags() const { return m_flags; }
	void set_flags(UINT32 flags) { m_flags = flags; }

private:
	std::string		m_name;
	std::string		m_hashdata;
	UINT32			m_offset;
	UINT32			m_length;
	UINT32			m_flags;
};


#endif // MAME_EMU_ROMENTRY_H_
