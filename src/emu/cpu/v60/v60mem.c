/****************************************************************/
/* Structure defining all callbacks for different architectures */
/****************************************************************/

struct cpu_info {
	UINT8  (*mr8) (address_space *space, offs_t address);
	void   (*mw8) (address_space *space, offs_t address, UINT8  data);
	UINT16 (*mr16)(address_space *space, offs_t address);
	void   (*mw16)(address_space *space, offs_t address, UINT16 data);
	UINT32 (*mr32)(address_space *space, offs_t address);
	void   (*mw32)(address_space *space, offs_t address, UINT32 data);
	UINT8  (*or8) (address_space *space, offs_t address);
	UINT16 (*or16)(address_space *space, offs_t address);
	UINT32 (*or32)(address_space *space, offs_t address);
	UINT32 start_pc;
};



/*****************************************************************/
/* Memory accesses for 16-bit data bus, 24-bit address bus (V60) */
/*****************************************************************/

static UINT8 memory_read_byte(address_space *space, offs_t address) { return space->read_byte(address); }
static void memory_write_byte(address_space *space, offs_t address, UINT8 data) { space->write_byte(address, data); }

#define MemRead8_16					memory_read_byte
#define MemWrite8_16				memory_write_byte

static UINT16 MemRead16_16(address_space *space, offs_t address)
{
	if (!(address & 1))
		return space->read_word(address);
	else
	{
		UINT16 result = space->read_byte(address);
		return result | space->read_byte(address + 1) << 8;
	}
}

static void MemWrite16_16(address_space *space, offs_t address, UINT16 data)
{
	if (!(address & 1))
		space->write_word(address, data);
	else
	{
		space->write_byte(address, data);
		space->write_byte(address + 1, data >> 8);
	}
}

static UINT32 MemRead32_16(address_space *space, offs_t address)
{
	if (!(address & 1))
	{
		UINT32 result = space->read_word(address);
		return result | (space->read_word(address + 2) << 16);
	}
	else
	{
		UINT32 result = space->read_byte(address);
		result |= space->read_word(address + 1) << 8;
		return result | space->read_byte(address + 3) << 24;
	}
}

static void MemWrite32_16(address_space *space, offs_t address, UINT32 data)
{
	if (!(address & 1))
	{
		space->write_word(address, data);
		space->write_word(address + 2, data >> 16);
	}
	else
	{
		space->write_byte(address, data);
		space->write_word(address + 1, data >> 8);
		space->write_byte(address + 3, data >> 24);
	}
}


/*****************************************************************/
/* Opcode accesses for 16-bit data bus, 24-bit address bus (V60) */
/*****************************************************************/

static UINT8 OpRead8_16(address_space *space, offs_t address)
{
	return space->direct().read_decrypted_byte(BYTE_XOR_LE(address));
}

static UINT16 OpRead16_16(address_space *space, offs_t address)
{
	return space->direct().read_decrypted_byte(BYTE_XOR_LE(address)) | (space->direct().read_decrypted_byte(BYTE_XOR_LE(address + 1)) << 8);
}

static UINT32 OpRead32_16(address_space *space, offs_t address)
{
	return space->direct().read_decrypted_byte(BYTE_XOR_LE(address)) | (space->direct().read_decrypted_byte(BYTE_XOR_LE(address + 1)) << 8) |
			(space->direct().read_decrypted_byte(BYTE_XOR_LE(address + 2)) << 16) | (space->direct().read_decrypted_byte(BYTE_XOR_LE(address + 3)) << 24);
}



/*****************************************************************/
/* Memory accesses for 32-bit data bus, 32-bit address bus (V70) */
/*****************************************************************/

#define MemRead8_32		memory_read_byte
#define MemWrite8_32	memory_write_byte

static UINT16 MemRead16_32(address_space *space, offs_t address)
{
	if (!(address & 1))
		return space->read_word(address);
	else
	{
		UINT16 result = space->read_byte(address);
		return result | space->read_byte(address + 1) << 8;
	}
}

static void MemWrite16_32(address_space *space, offs_t address, UINT16 data)
{
	if (!(address & 1))
		space->write_word(address, data);
	else
	{
		space->write_byte(address, data);
		space->write_byte(address + 1, data >> 8);
	}
}

static UINT32 MemRead32_32(address_space *space, offs_t address)
{
	if (!(address & 3))
		return space->read_dword(address);
	else if (!(address & 1))
	{
		UINT32 result = space->read_word(address);
		return result | (space->read_word(address + 2) << 16);
	}
	else
	{
		UINT32 result = space->read_byte(address);
		result |= space->read_word(address + 1) << 8;
		return result | space->read_byte(address + 3) << 24;
	}
}

static void MemWrite32_32(address_space *space, offs_t address, UINT32 data)
{
	if (!(address & 3))
		space->write_dword(address, data);
	else if (!(address & 1))
	{
		space->write_word(address, data);
		space->write_word(address + 2, data >> 16);
	}
	else
	{
		space->write_byte(address, data);
		space->write_word(address + 1, data >> 8);
		space->write_byte(address + 3, data >> 24);
	}
}



/*****************************************************************/
/* Opcode accesses for 32-bit data bus, 32-bit address bus (V60) */
/*****************************************************************/

static UINT8 OpRead8_32(address_space *space, offs_t address)
{
	return space->direct().read_decrypted_byte(BYTE4_XOR_LE(address));
}

static UINT16 OpRead16_32(address_space *space, offs_t address)
{
	return space->direct().read_decrypted_byte(BYTE4_XOR_LE(address)) | (space->direct().read_decrypted_byte(BYTE4_XOR_LE(address + 1)) << 8);
}

static UINT32 OpRead32_32(address_space *space, offs_t address)
{
	return space->direct().read_decrypted_byte(BYTE4_XOR_LE(address)) | (space->direct().read_decrypted_byte(BYTE4_XOR_LE(address + 1)) << 8) |
			(space->direct().read_decrypted_byte(BYTE4_XOR_LE(address + 2)) << 16) | (space->direct().read_decrypted_byte(BYTE4_XOR_LE(address + 3)) << 24);
}



/************************************************/
/* Structures pointing to various I/O functions */
/************************************************/

static const struct cpu_info v60_i =
{
	MemRead8_16,  MemWrite8_16,  MemRead16_16,  MemWrite16_16,  MemRead32_16,  MemWrite32_16,
	OpRead8_16,                  OpRead16_16,                   OpRead32_16,
	0xfffff0
};

static const struct cpu_info v70_i =
{
	MemRead8_32,  MemWrite8_32,  MemRead16_32,  MemWrite16_32,  MemRead32_32,  MemWrite32_32,
	OpRead8_32,                  OpRead16_32,                   OpRead32_32,
	0xfffffff0
};



/**************************************/
/* Macro shorthands for I/O functions */
/**************************************/

#define MemRead8    cpustate->info.mr8
#define MemWrite8   cpustate->info.mw8
#define MemRead16   cpustate->info.mr16
#define MemWrite16  cpustate->info.mw16
#define MemRead32   cpustate->info.mr32
#define MemWrite32  cpustate->info.mw32

#if defined(LSB_FIRST) && !defined(ALIGN_INTS)
#define OpRead8(s, a)	((s)->direct().read_decrypted_byte(a))
#define OpRead16(s, a)	((s)->direct().read_decrypted_word(a))
#define OpRead32(s, a)	((s)->direct().read_decrypted_dword(a))
#else
#define OpRead8     cpustate->info.or8
#define OpRead16    cpustate->info.or16
#define OpRead32    cpustate->info.or32
#endif
