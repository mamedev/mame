/*****************************************************************************
    8-bit memory accessors
 *****************************************************************************/

#ifdef I8086
#if (HAS_I8088||HAS_I80188)
static void configure_memory_8bit(void)
{
	I.mem.fetch_xor = 0;

	I.mem.rbyte = program_read_byte_8le;
	I.mem.rword = program_read_word_8le;
	I.mem.wbyte = program_write_byte_8le;
	I.mem.wword = program_write_word_8le;

	I.mem.rbyte_port = io_read_byte_8le;
	I.mem.rword_port = io_read_word_8le;
	I.mem.wbyte_port = io_write_byte_8le;
	I.mem.wword_port = io_write_word_8le;
}
#endif
#endif


/*****************************************************************************
    16-bit memory accessors
 *****************************************************************************/

static UINT16 read_word_16le(offs_t addr)
{
	if (!(addr & 1))
		return program_read_word_16le(addr);
	else
	{
		UINT16 result = program_read_byte_16le(addr);
		return result | (program_read_byte_16le(addr + 1) << 8);
	}
}

static void write_word_16le(offs_t addr, UINT16 data)
{
	if (!(addr & 1))
		program_write_word_16le(addr, data);
	else
	{
		program_write_byte_16le(addr, data);
		program_write_byte_16le(addr + 1, data >> 8);
	}
}

static UINT16 read_port_word_16le(offs_t addr)
{
	if (!(addr & 1))
		return io_read_word_16le(addr);
	else
	{
		UINT16 result = io_read_byte_16le(addr);
		return result | (io_read_byte_16le(addr + 1) << 8);
	}
}

static void write_port_word_16le(offs_t addr, UINT16 data)
{
	if (!(addr & 1))
		io_write_word_16le(addr, data);
	else
	{
		io_write_byte_16le(addr, data);
		io_write_byte_16le(addr + 1, data >> 8);
	}
}

static void configure_memory_16bit(void)
{
	I.mem.fetch_xor = BYTE_XOR_LE(0);

	I.mem.rbyte = program_read_byte_16le;
	I.mem.rword = read_word_16le;
	I.mem.wbyte = program_write_byte_16le;
	I.mem.wword = write_word_16le;

	I.mem.rbyte_port = io_read_byte_16le;
	I.mem.rword_port = read_port_word_16le;
	I.mem.wbyte_port = io_write_byte_16le;
	I.mem.wword_port = write_port_word_16le;
}
