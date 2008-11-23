/****************************************************************/
/* Structure defining all callbacks for different architectures */
/****************************************************************/

struct cpu_info {
	UINT8  (*mr8) (const address_space *space, offs_t address);
	void   (*mw8) (const address_space *space, offs_t address, UINT8  data);
	UINT16 (*mr16)(const address_space *space, offs_t address);
	void   (*mw16)(const address_space *space, offs_t address, UINT16 data);
	UINT32 (*mr32)(const address_space *space, offs_t address);
	void   (*mw32)(const address_space *space, offs_t address, UINT32 data);
	UINT8  (*or8) (const address_space *space, offs_t address);
	UINT16 (*or16)(const address_space *space, offs_t address);
	UINT32 (*or32)(const address_space *space, offs_t address);
	UINT32 start_pc;
};



/*****************************************************************/
/* Memory accesses for 16-bit data bus, 24-bit address bus (V60) */
/*****************************************************************/

#define MemRead8_16		memory_read_byte_16le
#define MemWrite8_16	memory_write_byte_16le

static UINT16 MemRead16_16(const address_space *space, offs_t address)
{
	if (!(address & 1))
		return memory_read_word_16le(space, address);
	else
	{
		UINT16 result = memory_read_byte_16le(space, address);
		return result | memory_read_byte_16le(space, address + 1) << 8;
	}
}

static void MemWrite16_16(const address_space *space, offs_t address, UINT16 data)
{
	if (!(address & 1))
		memory_write_word_16le(space, address, data);
	else
	{
		memory_write_byte_16le(space, address, data);
		memory_write_byte_16le(space, address + 1, data >> 8);
	}
}

static UINT32 MemRead32_16(const address_space *space, offs_t address)
{
	if (!(address & 1))
	{
		UINT32 result = memory_read_word_16le(space, address);
		return result | (memory_read_word_16le(space, address + 2) << 16);
	}
	else
	{
		UINT32 result = memory_read_byte_16le(space, address);
		result |= memory_read_word_16le(space, address + 1) << 8;
		return result | memory_read_byte_16le(space, address + 3) << 24;
	}
}

static void MemWrite32_16(const address_space *space, offs_t address, UINT32 data)
{
	if (!(address & 1))
	{
		memory_write_word_16le(space, address, data);
		memory_write_word_16le(space, address + 2, data >> 16);
	}
	else
	{
		memory_write_byte_16le(space, address, data);
		memory_write_word_16le(space, address + 1, data >> 8);
		memory_write_byte_16le(space, address + 3, data >> 24);
	}
}


/*****************************************************************/
/* Opcode accesses for 16-bit data bus, 24-bit address bus (V60) */
/*****************************************************************/

static UINT8 OpRead8_16(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE_XOR_LE(address));
}

static UINT16 OpRead16_16(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE_XOR_LE(address+1)) << 8);
}

static UINT32 OpRead32_16(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE_XOR_LE(address+1)) << 8) |
			(memory_decrypted_read_byte(space, BYTE_XOR_LE(address+2)) << 16) | (memory_decrypted_read_byte(space, BYTE_XOR_LE(address+3)) << 24);
}



/*****************************************************************/
/* Memory accesses for 32-bit data bus, 32-bit address bus (V70) */
/*****************************************************************/

#define MemRead8_32		memory_read_byte_32le
#define MemWrite8_32	memory_write_byte_32le

static UINT16 MemRead16_32(const address_space *space, offs_t address)
{
	if (!(address & 1))
		return memory_read_word_32le(space, address);
	else
	{
		UINT16 result = memory_read_byte_32le(space, address);
		return result | memory_read_byte_32le(space, address + 1) << 8;
	}
}

static void MemWrite16_32(const address_space *space, offs_t address, UINT16 data)
{
	if (!(address & 1))
		memory_write_word_32le(space, address, data);
	else
	{
		memory_write_byte_32le(space, address, data);
		memory_write_byte_32le(space, address + 1, data >> 8);
	}
}

static UINT32 MemRead32_32(const address_space *space, offs_t address)
{
	if (!(address & 3))
		return memory_read_dword_32le(space, address);
	else if (!(address & 1))
	{
		UINT32 result = memory_read_word_32le(space, address);
		return result | (memory_read_word_32le(space, address + 2) << 16);
	}
	else
	{
		UINT32 result = memory_read_byte_32le(space, address);
		result |= memory_read_word_32le(space, address + 1) << 8;
		return result | memory_read_byte_32le(space, address + 3) << 24;
	}
}

static void MemWrite32_32(const address_space *space, offs_t address, UINT32 data)
{
	if (!(address & 3))
		memory_write_dword_32le(space, address, data);
	else if (!(address & 1))
	{
		memory_write_word_32le(space, address, data);
		memory_write_word_32le(space, address + 2, data >> 16);
	}
	else
	{
		memory_write_byte_32le(space, address, data);
		memory_write_word_32le(space, address + 1, data >> 8);
		memory_write_byte_32le(space, address + 3, data >> 24);
	}
}



/*****************************************************************/
/* Opcode accesses for 32-bit data bus, 32-bit address bus (V60) */
/*****************************************************************/

static UINT8 OpRead8_32(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE4_XOR_LE(address));
}

static UINT16 OpRead16_32(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE4_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE4_XOR_LE(address+1)) << 8);
}

static UINT32 OpRead32_32(const address_space *space, offs_t address)
{
	return memory_decrypted_read_byte(space, BYTE4_XOR_LE(address)) | (memory_decrypted_read_byte(space, BYTE4_XOR_LE(address+1)) << 8) |
			(memory_decrypted_read_byte(space, BYTE4_XOR_LE(address+2)) << 16) | (memory_decrypted_read_byte(space, BYTE4_XOR_LE(address+3)) << 24);
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

#define MemRead8    v60.info.mr8
#define MemWrite8   v60.info.mw8
#define MemRead16   v60.info.mr16
#define MemWrite16  v60.info.mw16
#define MemRead32   v60.info.mr32
#define MemWrite32  v60.info.mw32

#if defined(LSB_FIRST) && !defined(ALIGN_INTS)
#define OpRead8(s,a)	(memory_decrypted_read_byte(s,a))
#define OpRead16(s,a)	(memory_decrypted_read_word(s,a))
#define OpRead32(s,a)	(memory_decrypted_read_dword(s,a))
#else
#define OpRead8     v60.info.or8
#define OpRead16    v60.info.or16
#define OpRead32    v60.info.or32
#endif
