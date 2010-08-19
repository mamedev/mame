/*****************************************************************************
    8-bit memory accessors
 *****************************************************************************/

static UINT8 memory_read_byte(address_space *space, offs_t address) { return space->read_byte(address); }
static void memory_write_byte(address_space *space, offs_t address, UINT8 data) { space->write_byte(address, data); }

#ifdef I8086

static UINT16 memory_read_word(address_space *space, offs_t address) { return space->read_word(address); }
static void memory_write_word(address_space *space, offs_t address, UINT16 data) { space->write_word(address, data); }

static void configure_memory_8bit(i8086_state *cpustate)
{
	cpustate->mem.fetch_xor = 0;

	cpustate->mem.rbyte = memory_read_byte;
	cpustate->mem.rword = memory_read_word;
	cpustate->mem.wbyte = memory_write_byte;
	cpustate->mem.wword = memory_write_word;
}
#endif


/*****************************************************************************
    16-bit memory accessors
 *****************************************************************************/

static UINT16 read_word_16le(address_space *space, offs_t addr)
{
	if (!(addr & 1))
		return space->read_word(addr);
	else
	{
		UINT16 result = space->read_byte(addr);
		return result | (space->read_byte(addr + 1) << 8);
	}
}

static void write_word_16le(address_space *space, offs_t addr, UINT16 data)
{
	if (!(addr & 1))
		space->write_word(addr, data);
	else
	{
		space->write_byte(addr, data);
		space->write_byte(addr + 1, data >> 8);
	}
}

static void configure_memory_16bit(i8086_state *cpustate)
{
	cpustate->mem.fetch_xor = BYTE_XOR_LE(0);

	cpustate->mem.rbyte = memory_read_byte;
	cpustate->mem.rword = read_word_16le;
	cpustate->mem.wbyte = memory_write_byte;
	cpustate->mem.wword = write_word_16le;
}
