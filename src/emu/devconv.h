/***************************************************************************

    devconv.h

    Functions which help convert between different device handlers.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    ***** VERY IMPORTANT NOTICE *****

    These functions and macros are provided to facilitate the mapping
    of devices on cpus with different data bus widths.
    Devices should be implemented using their native data bus width,
    since that ensures that read/write operations are kept atomic.
    If we discover you have abused the functionality presented in this
    file, you *will* be publicly humiliated and your code submission
    will probably not be accepted. Seriously, please do not abuse these
    functions/macros.

****************************************************************************

    Conversions supported:

    CW = CPU Data Bus Width in bits
    CBO = CPU Byte Order
    DW = Device Data Bus Width in bits
    DBO = Device Byte Order

    CW | CBO    | DW | DBO    | Functions to use
    ---+--------+----+--------+-----------------------------------------------------------
    16 | Big    | 8  | N/A    | read16be_with_read8_device_handler,write16be_with_write8_device_handler
    16 | Little | 8  | N/A    | read16le_with_read8_device_handler,write16le_with_write8_device_handler
    ---+--------+----+--------+-----------------------------------------------------------
    32 | Big    | 8  | N/A    | read32be_with_read8_device_handler,write32be_with_write8_device_handler
    32 | Little | 8  | N/A    | read32le_with_read8_device_handler,write32le_with_write8_device_handler
    32 | Big    | 16 | Big    | read32be_with_16be_device_handler,write32be_with_16be_device_handler
    32 | Little | 16 | Little | read32le_with_16le_device_handler,write32le_with_16le_device_handler
    32 | Big    | 16 | Little | read32be_with_16le_device_handler,write32be_with_16le_device_handler
    32 | Little | 16 | Big    | read32le_with_16be_device_handler,write32le_with_16be_device_handler
    ---+--------+----+--------+-----------------------------------------------------------
    64 | Big    | 8  | N/A    | read64be_with_read8_device_handler,write64be_with_write8_device_handler
    64 | Little | 8  | N/A    | read64le_with_read8_device_handler,write64le_with_write8_device_handler
    64 | Big    | 16 | Big    | read64be_with_16be_device_handler,write64be_with_16be_device_handler
    64 | Little | 16 | Little | read64le_with_16le_device_handler,write64le_with_16le_device_handler
    64 | Big    | 16 | Little | read64be_with_16le_device_handler,write64be_with_16le_device_handler
    64 | Little | 16 | Big    | read64le_with_16be_device_handler,write64le_with_16be_device_handler
    64 | Big    | 32 | Big    | read64be_with_32be_device_handler,write64be_with_32be_device_handler
    64 | Little | 32 | Little | read64le_with_32le_device_handler,write64le_with_32le_device_handler
    64 | Big    | 32 | Little | read64be_with_32le_device_handler,write64be_with_32le_device_handler
    64 | Little | 32 | Big    | read64le_with_32be_device_handler,write64le_with_32be_device_handler

    You can also find at the bottom of this file a few convernient
    macros that will create the stub read and/or write handlers for
    the most common mappings, that will use the functions above.
    Here's an example on how to use them: Say you have a 8 bit device
    whose handlers are device8_r and device8_w, and you want to connect
    it to a 16 bit, big endian cpu. We'll say the device is mapped on
    the least significant byte of the data bus (LSB).

    In your driver, you would add:

    DEV_READWRITE8TO16BE_LSB( device16, device8_r, device8_w )

    which will create two 16 bit memory handlers, one for read, called
    device16_r, and one for write, called device16_w, with the proper
    mapping.

    then in the MEMORY_MAP you would specify:

    AM_RANGE(0x000000, 0x0000ff) AM_DEVREADWRITE( DEVICE, "device", device16_r, device16_w )

    And that is all. Your device should be mapped properly.
    If you need to do custom mappings, or a mapping that is not currently
    supported in this file, you can always write the stub yourself, and
    call the above functions to invoke the base handlers.

***************************************************************************/

/*************************************
 *
 *  16-bit BE using 8-bit handlers
 *
 *************************************/

INLINE UINT16 read16be_with_read8_device_handler(read8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 result = 0;
	if (ACCESSING_BITS_8_15)
		result |= ((UINT16)(*handler)(device, space, offset * 2 + 0, mem_mask >> 8)) << 8;
	if (ACCESSING_BITS_0_7)
		result |= ((UINT16)(*handler)(device, space, offset * 2 + 1, mem_mask >> 0)) << 0;
	return result;
}


INLINE void write16be_with_write8_device_handler(write8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
		(*handler)(device, space, offset * 2 + 0, data >> 8, mem_mask >> 8);
	if (ACCESSING_BITS_0_7)
		(*handler)(device, space, offset * 2 + 1, data >> 0, mem_mask >> 0);
}


/*************************************
 *
 *  16-bit LE using 8-bit handlers
 *
 *************************************/

INLINE UINT16 read16le_with_read8_device_handler(read8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 result = 0;
	if (ACCESSING_BITS_0_7)
		result |= ((UINT16) (*handler)(device, space, offset * 2 + 0, mem_mask >> 0)) << 0;
	if (ACCESSING_BITS_8_15)
		result |= ((UINT16) (*handler)(device, space, offset * 2 + 1, mem_mask >> 8)) << 8;
	return result;
}


INLINE void write16le_with_write8_device_handler(write8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		(*handler)(device, space, offset * 2 + 0, data >> 0, mem_mask >> 0);
	if (ACCESSING_BITS_8_15)
		(*handler)(device, space, offset * 2 + 1, data >> 8, mem_mask >> 8);
}


/*************************************
 *
 *  32-bit BE using 8-bit handlers
 *
 *************************************/

INLINE UINT32 read32be_with_read8_device_handler(read8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 mem_mask)
{
	UINT32 result = 0;
	if (ACCESSING_BITS_16_31)
		result |= read16be_with_read8_device_handler(handler, device, space, offset * 2 + 0, mem_mask >> 16) << 16;
	if (ACCESSING_BITS_0_15)
		result |= read16be_with_read8_device_handler(handler, device, space, offset * 2 + 1, mem_mask) << 0;
	return result;
}


INLINE void write32be_with_write8_device_handler(write8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_16_31)
		write16be_with_write8_device_handler(handler, device, space, offset * 2 + 0, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)
		write16be_with_write8_device_handler(handler, device, space, offset * 2 + 1, data, mem_mask);
}


/*************************************
 *
 *  32-bit LE using 8-bit handlers
 *
 *************************************/

INLINE UINT32 read32le_with_read8_device_handler(read8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 mem_mask)
{
	UINT32 result = 0;
	if (ACCESSING_BITS_0_15)
		result |= read16le_with_read8_device_handler(handler, device, space, offset * 2 + 0, mem_mask) << 0;
	if (ACCESSING_BITS_16_31)
		result |= read16le_with_read8_device_handler(handler, device, space, offset * 2 + 1, mem_mask >> 16) << 16;
	return result;
}


INLINE void write32le_with_write8_device_handler(write8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		write16le_with_write8_device_handler(handler, device, space, offset * 2 + 0, data, mem_mask);
	if (ACCESSING_BITS_16_31)
		write16le_with_write8_device_handler(handler, device, space, offset * 2 + 1, data >> 16, mem_mask >> 16);
}


/*************************************
 *
 *  32-bit BE using 16-bit BE handlers
 *
 *************************************/

INLINE UINT32 read32be_with_16be_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 mem_mask)
{
	UINT32 result = 0;
	if (ACCESSING_BITS_16_31)
		result |= (*handler)(device, space, offset * 2 + 0, mem_mask >> 16) << 16;
	if (ACCESSING_BITS_0_15)
		result |= (*handler)(device, space, offset * 2 + 1, mem_mask) << 0;
	return result;
}


INLINE void write32be_with_16be_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_16_31)
		(*handler)(device, space, offset * 2 + 0, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)
		(*handler)(device, space, offset * 2 + 1, data, mem_mask);
}


/*************************************
 *
 *  32-bit LE using 16-bit LE handlers
 *
 *************************************/

INLINE UINT32 read32le_with_16le_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 mem_mask)
{
	UINT32 result = 0;
	if (ACCESSING_BITS_0_15)
		result |= (*handler)(device, space, offset * 2 + 0, mem_mask) << 0;
	if (ACCESSING_BITS_16_31)
		result |= (*handler)(device, space, offset * 2 + 1, mem_mask >> 16) << 16;
	return result;
}


INLINE void write32le_with_16le_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	if (ACCESSING_BITS_0_15)
		(*handler)(device, space, offset * 2 + 0, data, mem_mask);
	if (ACCESSING_BITS_16_31)
		(*handler)(device, space, offset * 2 + 1, data >> 16, mem_mask >> 16);
}


/*************************************
 *
 *  32-bit BE using 16-bit LE handlers
 *
 *************************************/

INLINE UINT32 read32be_with_16le_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 mem_mask)
{
	UINT32 result = 0;
	mem_mask = FLIPENDIAN_INT32(mem_mask);
	result = read32le_with_16le_device_handler(handler, device, space, offset, mem_mask);
	return FLIPENDIAN_INT32(result);
}


INLINE void write32be_with_16le_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	data = FLIPENDIAN_INT32(data);
	mem_mask = FLIPENDIAN_INT32(mem_mask);
	write32le_with_16le_device_handler(handler, device, space, offset, data, mem_mask);
}


/*************************************
 *
 *  32-bit LE using 16-bit BE handlers
 *
 *************************************/

INLINE UINT32 read32le_with_16be_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 mem_mask)
{
	UINT32 result = 0;
	mem_mask = FLIPENDIAN_INT32(mem_mask);
	result = read32be_with_16be_device_handler(handler, device, space, offset, mem_mask);
	return FLIPENDIAN_INT32(result);
}


INLINE void write32le_with_16be_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	data = FLIPENDIAN_INT32(data);
	mem_mask = FLIPENDIAN_INT32(mem_mask);
	write32be_with_16be_device_handler(handler, device, space, offset, data, mem_mask);
}


/*************************************
 *
 *  64-bit BE using 8-bit handlers
 *
 *************************************/

INLINE UINT64 read64be_with_read8_device_handler(read8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read32be_with_read8_device_handler(handler, device, space, offset * 2 + 0, mem_mask >> 32) << 32;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read32be_with_read8_device_handler(handler, device, space, offset * 2 + 1, mem_mask) << 0;
	return result;
}


INLINE void write64be_with_write8_device_handler(write8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_32_63)
		write32be_with_write8_device_handler(handler, device, space, offset * 2 + 0, data >> 32, mem_mask >> 32);
	if (ACCESSING_BITS_0_31)
		write32be_with_write8_device_handler(handler, device, space, offset * 2 + 1, data, mem_mask);
}


/*************************************
 *
 *  64-bit LE using 8-bit handlers
 *
 *************************************/

INLINE UINT64 read64le_with_read8_device_handler(read8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read32le_with_read8_device_handler(handler, device, space, offset * 2 + 0, mem_mask >> 0) << 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read32le_with_read8_device_handler(handler, device, space, offset * 2 + 1, mem_mask >> 32) << 32;
	return result;
}


INLINE void write64le_with_write8_device_handler(write8_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_0_31)
		write32le_with_write8_device_handler(handler, device, space, offset * 2 + 0, data >> 0, mem_mask >> 0);
	if (ACCESSING_BITS_32_63)
		write32le_with_write8_device_handler(handler, device, space, offset * 2 + 1, data >> 32, mem_mask >> 32);
}


/*************************************
 *
 *  64-bit BE using 16-bit BE handlers
 *
 *************************************/

INLINE UINT32 read64be_with_16be_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read32be_with_16be_device_handler(handler, device, space, offset * 2 + 0, mem_mask >> 32) << 32;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read32be_with_16be_device_handler(handler, device, space, offset * 2 + 1, mem_mask >> 0) << 0;
	return result;
}


INLINE void write64be_with_16be_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_32_63)
		write32be_with_16be_device_handler(handler, device, space, offset * 2 + 0, data >> 32, mem_mask >> 32);
	if (ACCESSING_BITS_0_31)
		write32be_with_16be_device_handler(handler, device, space, offset * 2 + 1, data >> 0, mem_mask >> 0);
}


/*************************************
 *
 *  64-bit LE using 16-bit LE handlers
 *
 *************************************/

INLINE UINT32 read64le_with_16le_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read32le_with_16le_device_handler(handler, device, space, offset * 2 + 0, mem_mask >> 0) << 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read32le_with_16le_device_handler(handler, device, space, offset * 2 + 1, mem_mask >> 32) << 32;
	return result;
}


INLINE void write64le_with_16le_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_0_31)
		write32le_with_16le_device_handler(handler, device, space, offset * 2 + 0, data >> 0, mem_mask >> 0);
	if (ACCESSING_BITS_32_63)
		write32le_with_16le_device_handler(handler, device, space, offset * 2 + 1, data >> 32, mem_mask >> 32);
}


/*************************************
 *
 *  64-bit BE using 16-bit LE handlers
 *
 *************************************/

INLINE UINT32 read64be_with_16le_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read32be_with_16le_device_handler(handler, device, space, offset * 2 + 0, mem_mask >> 32) << 32;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read32be_with_16le_device_handler(handler, device, space, offset * 2 + 1, mem_mask >> 0) << 0;
	return result;
}


INLINE void write64be_with_16le_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_32_63)
		write32be_with_16le_device_handler(handler, device, space, offset * 2 + 0, data >> 32, mem_mask >> 32);
	if (ACCESSING_BITS_0_31)
		write32be_with_16le_device_handler(handler, device, space, offset * 2 + 1, data >> 0, mem_mask >> 0);
}


/*************************************
 *
 *  64-bit LE using 16-bit BE handlers
 *
 *************************************/

INLINE UINT32 read64le_with_16be_device_handler(read16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)read32le_with_16be_device_handler(handler, device, space, offset * 2 + 0, mem_mask >> 0) << 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)read32le_with_16be_device_handler(handler, device, space, offset * 2 + 1, mem_mask >> 32) << 32;
	return result;
}


INLINE void write64le_with_16be_device_handler(write16_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_0_31)
		write32le_with_16be_device_handler(handler, device, space, offset * 2 + 0, data >> 0, mem_mask >> 0);
	if (ACCESSING_BITS_32_63)
		write32le_with_16be_device_handler(handler, device, space, offset * 2 + 1, data >> 32, mem_mask >> 32);
}


/*************************************
 *
 *  64-bit BE using 32-bit BE handlers
 *
 *************************************/

INLINE UINT64 read64be_with_32be_device_handler(read32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)(*handler)(device, space, offset * 2 + 0, mem_mask >> 32) << 32;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)(*handler)(device, space, offset * 2 + 1, mem_mask >> 0) << 0;
	return result;
}


INLINE void write64be_with_32be_device_handler(write32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_32_63)
		(*handler)(device, space, offset * 2 + 0, data >> 32, mem_mask >> 32);
	if (ACCESSING_BITS_0_31)
		(*handler)(device, space, offset * 2 + 1, data >>  0, mem_mask >>  0);
}


/*************************************
 *
 *  64-bit LE using 32-bit LE handlers
 *
 *************************************/

INLINE UINT64 read64le_with_32le_device_handler(read32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result = 0;
	if (ACCESSING_BITS_0_31)
		result |= (UINT64)(*handler)(device, space, offset * 2 + 0, mem_mask >> 0) << 0;
	if (ACCESSING_BITS_32_63)
		result |= (UINT64)(*handler)(device, space, offset * 2 + 1, mem_mask >> 32) << 32;
	return result;
}


INLINE void write64le_with_32le_device_handler(write32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	if (ACCESSING_BITS_0_31)
		(*handler)(device, space, offset * 2 + 0, data >> 0, mem_mask >> 0);
	if (ACCESSING_BITS_32_63)
		(*handler)(device, space, offset * 2 + 1, data >> 32, mem_mask >> 32);
}


/*************************************
 *
 *  64-bit BE using 32-bit LE handlers
 *
 *************************************/

INLINE UINT64 read64be_with_32le_device_handler(read32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result;
	mem_mask = FLIPENDIAN_INT64(mem_mask);
	result = read64le_with_32le_device_handler(handler, device, space, offset, mem_mask);
	return FLIPENDIAN_INT64(result);
}


INLINE void write64be_with_32le_device_handler(write32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	data = FLIPENDIAN_INT64(data);
	mem_mask = FLIPENDIAN_INT64(mem_mask);
	write64le_with_32le_device_handler(handler, device, space, offset, data, mem_mask);
}


/*************************************
 *
 *  64-bit LE using 32-bit BE handlers
 *
 *************************************/

INLINE UINT64 read64le_with_32be_device_handler(read32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 mem_mask)
{
	UINT64 result;
	mem_mask = FLIPENDIAN_INT64(mem_mask);
	result = read64be_with_32be_device_handler(handler, device, space, offset, mem_mask);
	return FLIPENDIAN_INT64(result);
}


INLINE void write64le_with_32be_device_handler(write32_device_func handler, device_t *device, address_space &space, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	data = FLIPENDIAN_INT64(data);
	mem_mask = FLIPENDIAN_INT64(mem_mask);
	write64be_with_32be_device_handler(handler, device, space, offset, data, mem_mask);
}



/**************************************************************************

    Utility macros

**************************************************************************/

#define DEV_READ_TEMPLATE(bits, name, handler, func)				\
READ##bits##_DEVICE_HANDLER( name##_r )								\
{																	\
	return func(handler, device, space, offset, mem_mask);			\
}

#define DEV_READ_TEMPLATE_COND(bits, name, handler, func, cond)		\
READ##bits##_DEVICE_HANDLER( name##_r )								\
{																	\
	if (cond)														\
		return func(handler, device, space, offset, mem_mask);		\
	return 0;														\
}

#define DEV_WRITE_TEMPLATE(bits, name, handler, func)				\
WRITE##bits##_DEVICE_HANDLER( name##_w )							\
{																	\
	func(handler, device, space, offset, data, mem_mask);			\
}

#define DEV_WRITE_TEMPLATE_COND(bits, name, handler, func, cond)	\
WRITE##bits##_DEVICE_HANDLER( name##_w )							\
{																	\
	if (cond)														\
		func(handler, device, space, offset, data, mem_mask);		\
}



/**************************************************************************

    Generic conversions macros

**************************************************************************/


/*************************************
 * 8->16be, 1:1 mapping
 ************************************/

#define DEV_READ8TO16BE( name, read8 ) \
DEV_READ_TEMPLATE( 16, name, read8, read16be_with_read8_device_handler )


#define DEV_WRITE8TO16BE( name, write8 ) \
DEV_WRITE_TEMPLATE( 16, name, write8, write16be_with_write8_device_handler )


#define DEV_READWRITE8TO16BE( name, read8, write8 ) \
DEV_READ8TO16BE(name,read8) \
DEV_WRITE8TO16BE(name,write8)


/*************************************
 * 8->16le, 1:1 mapping
 ************************************/

#define DEV_READ8TO16LE( name, read8 ) \
DEV_READ_TEMPLATE( 16, name, read8, read16le_with_read8_device_handler )


#define DEV_WRITE8TO16LE( name, write8 ) \
DEV_WRITE_TEMPLATE( 16, name, write8, write16le_with_write8_device_handler )


#define DEV_READWRITE8TO16LE( name, read8, write8 ) \
DEV_READ8TO16LE(name,read8) \
DEV_WRITE8TO16LE(name,write8)


/*************************************
 * 8->16be, MSB mapping
 ************************************/

#define DEV_READ8TO16BE_MSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 16, name, read8, read16be_with_read8_device_handler, ACCESSING_BITS_8_15 )


#define DEV_WRITE8TO16BE_MSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 16, name, write8, write16be_with_write8_device_handler, ACCESSING_BITS_8_15 )


#define DEV_READWRITE8TO16BE_MSB( name, read8, write8 ) \
DEV_READ8TO16BE_MSB(name,read8) \
DEV_WRITE8TO16BE_MSB(name,write8)


/*************************************
 * 8->16le, MSB mapping
 ************************************/

#define DEV_READ8TO16LE_MSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 16, name, read8, read16le_with_read8_device_handler, ACCESSING_BITS_8_15 )


#define DEV_WRITE8TO16LE_MSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 16, name, write8, write16le_with_write8_device_handler, ACCESSING_BITS_8_15 )


#define DEV_READWRITE8TO16LE_MSB( name, read8, write8 ) \
DEV_READ8TO16LE_MSB(name,read8) \
DEV_WRITE8TO16LE_MSB(name,write8)


/*************************************
 * 8->16be, LSB mapping
 ************************************/

#define DEV_READ8TO16BE_LSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 16, name, read8, read16be_with_read8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_WRITE8TO16BE_LSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 16, name, write8, write16be_with_write8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_READWRITE8TO16BE_LSB( name, read8, write8 ) \
DEV_READ8TO16BE_LSB(name,read8) \
DEV_WRITE8TO16BE_LSB(name,write8)


/*************************************
 * 8->16le, LSB mapping
 ************************************/

#define DEV_READ8TO16LE_LSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 16, name, read8, read16le_with_read8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_WRITE8TO16LE_LSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 16, name, write8, write16le_with_write8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_READWRITE8TO16LE_LSB( name, read8, write8 ) \
DEV_READ8TO16LE_LSB(name,read8) \
DEV_WRITE8TO16LE_LSB(name,write8)


/*************************************
 * 8->32be, 1:1 mapping
 ************************************/

#define DEV_READ8TO32BE( name, read8 ) \
DEV_READ_TEMPLATE( 32, name, read8, read32be_with_read8_device_handler )


#define DEV_WRITE8TO32BE( name, write8 ) \
DEV_WRITE_TEMPLATE( 32, name, write8, write32be_with_write8_device_handler )


#define DEV_READWRITE8TO32BE( name, read8, write8 ) \
DEV_READ8TO32BE(name,read8) \
DEV_WRITE8TO32BE(name,write8)


/*************************************
 * 8->32le, 1:1 mapping
 ************************************/

#define DEV_READ8TO32LE( name, read8 ) \
DEV_READ_TEMPLATE( 32, name, read8, read32le_with_read8_device_handler )


#define DEV_WRITE8TO32LE( name, write8 ) \
DEV_WRITE_TEMPLATE( 32, name, write8, write32le_with_write8_device_handler )


#define DEV_READWRITE8TO32LE( name, read8, write8 ) \
DEV_READ8TO32LE(name,read8) \
DEV_WRITE8TO32LE(name,write8)


/*************************************
 * 8->32be, MSB mapping
 ************************************/

#define DEV_READ8TO32BE_MSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 32, name, read8, read32be_with_read8_device_handler, ACCESSING_BITS_24_31 )


#define DEV_WRITE8TO32BE_MSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write8, write32be_with_write8_device_handler, ACCESSING_BITS_24_31 )


#define DEV_READWRITE8TO32BE_MSB( name, read8, write8 ) \
DEV_READ8TO32BE_MSB(name,read8) \
DEV_WRITE8TO32BE_MSB(name,write8)


/*************************************
 * 8->32le, MSB mapping
 ************************************/

#define DEV_READ8TO32LE_MSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 32, name, read8, read32le_with_read8_device_handler, ACCESSING_BITS_24_31 )


#define DEV_WRITE8TO32LE_MSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write8, write32le_with_write8_device_handler, ACCESSING_BITS_24_31 )


#define DEV_READWRITE8TO32LE_MSB( name, read8, write8 ) \
DEV_READ8TO32LE_MSB(name,read8) \
DEV_WRITE8TO32LE_MSB(name,write8)


/*************************************
 * 8->32be, LSB mapping
 ************************************/

#define DEV_READ8TO32BE_LSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 32, name, read8, read32be_with_read8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_WRITE8TO32BE_LSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write8, write32be_with_write8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_READWRITE8TO32BE_LSB( name, read8, write8 ) \
DEV_READ8TO32BE_LSB(name,read8) \
DEV_WRITE8TO32BE_LSB(name,write8)


/*************************************
 * 8->32le, LSB mapping
 ************************************/

#define DEV_READ8TO32LE_LSB( name, read8 ) \
DEV_READ_TEMPLATE_COND( 32, name, read8, read32le_with_read8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_WRITE8TO32LE_LSB( name, write8 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write8, write32le_with_write8_device_handler, ACCESSING_BITS_0_7 )


#define DEV_READWRITE8TO32LE_LSB( name, read8, write8 ) \
DEV_READ8TO32LE_LSB(name,read8) \
DEV_WRITE8TO32LE_LSB(name,write8)


/*************************************
 * 8->64be, 1:1 mapping
 ************************************/

#define DEV_READ8TO64BE( name, read8 ) \
DEV_READ_TEMPLATE( 64, name, read8, read64be_with_read8_device_handler )


#define DEV_WRITE8TO64BE( name, write8 ) \
DEV_WRITE_TEMPLATE( 64, name, write8, write64be_with_write8_device_handler )


#define DEV_READWRITE8TO64BE( name, read8, write8 ) \
DEV_READ8TO64BE(name,read8) \
DEV_WRITE8TO64BE(name,write8)


/*************************************
 * 8->64le, 1:1 mapping
 ************************************/

#define DEV_READ8TO64LE( name, read8 ) \
DEV_READ_TEMPLATE( 64, name, read8, read64le_with_read8_device_handler )


#define DEV_WRITE8TO64LE( name, write8 ) \
DEV_WRITE_TEMPLATE( 64, name, write8, write64le_with_write8_device_handler )


#define DEV_READWRITE8TO64LE( name, read8, write8 ) \
DEV_READ8TO64LE(name,read8) \
DEV_WRITE8TO64LE(name,write8)


/*************************************
 *  16be->32be, 1:1 mapping
 *************************************/

#define DEV_READ16BETO32BE( name, read16 ) \
DEV_READ_TEMPLATE( 32, name, read16, read32be_with_16be_device_handler )


#define DEV_WRITE16BETO32BE( name, write16 ) \
DEV_WRITE_TEMPLATE( 32, name, write16, write32be_with_16be_device_handler )


#define DEV_READWRITE16BETO32BE( name, read16, write16 ) \
DEV_READ16BETO32BE(name,read16) \
DEV_WRITE16BETO32BE(name,write16)


/*************************************
 *  16le->32be, 1:1 mapping
 *************************************/

#define DEV_READ16LETO32BE( name, read16 ) \
DEV_READ_TEMPLATE( 32, name, read16, read32be_with_16le_device_handler )


#define DEV_WRITE16LETO32BE( name, write16 ) \
DEV_WRITE_TEMPLATE( 32, name, write16, write32be_with_16le_device_handler )


#define DEV_READWRITE16LETO32BE( name, read16, write16 ) \
DEV_READ16LETO32BE(name,read16) \
DEV_WRITE16LETO32BE(name,write16)


/*************************************
 *  16be->32le, 1:1 mapping
 *************************************/

#define DEV_READ16BETO32LE( name, read16 ) \
DEV_READ_TEMPLATE( 32, name, read16, read32le_with_16be_device_handler )


#define DEV_WRITE16BETO32LE( name, write16 ) \
DEV_WRITE_TEMPLATE( 32, name, write16, write32le_with_16be_device_handler )


#define DEV_READWRITE16BETO32LE( name, read16, write16 ) \
DEV_READ16BETO32LE(name,read16) \
DEV_WRITE16BETO32LE(name,write16)


/*************************************
 *  16le->32le, 1:1 mapping
 *************************************/

#define DEV_READ16LETO32LE( name, read16 ) \
DEV_READ_TEMPLATE( 32, name, read16, read32le_with_16le_device_handler )


#define DEV_WRITE16LETO32LE( name, write16 ) \
DEV_WRITE_TEMPLATE( 32, name, write16, write32le_with_16le_device_handler )


#define DEV_READWRITE16LETO32LE( name, read16, write16 ) \
DEV_READ16LETO32LE(name,read16) \
DEV_WRITE16LETO32LE(name,write16)


/*************************************
 *  16be->32be, MSW mapping
 *************************************/

#define DEV_READ16BETO32BE_MSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32be_with_16be_device_handler, ACCESSING_BITS_16_31 )


#define DEV_WRITE16BETO32BE_MSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32be_with_16be_device_handler, ACCESSING_BITS_16_31 )


#define DEV_READWRITE16BETO32BE_MSW( name, read16, write16 ) \
DEV_READ16BETO32BE_MSW(name,read16) \
DEV_WRITE16BETO32BE_MSW(name,write16)


/*************************************
 *  16le->32be, MSW mapping
 *************************************/

#define DEV_READ16LETO32BE_MSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32be_with_16le_device_handler, ACCESSING_BITS_16_31 )


#define DEV_WRITE16LETO32BE_MSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32be_with_16le_device_handler, ACCESSING_BITS_16_31 )


#define DEV_READWRITE16LETO32BE_MSW( name, read16, write16 ) \
DEV_READ16LETO32BE_MSW(name,read16) \
DEV_WRITE16LETO32BE_MSW(name,write16)


/*************************************
 *  16be->32le, MSW mapping
 *************************************/

#define DEV_READ16BETO32LE_MSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32le_with_16be_device_handler, ACCESSING_BITS_16_31 )


#define DEV_WRITE16BETO32LE_MSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32le_with_16be_device_handler, ACCESSING_BITS_16_31 )


#define DEV_READWRITE16BETO32LE_MSW( name, read16, write16 ) \
DEV_READ16BETO32LE_MSW(name,read16) \
DEV_WRITE16BETO32LE_MSW(name,write16)


/*************************************
 *  16le->32le, MSW mapping
 *************************************/

#define DEV_READ16LETO32LE_MSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32le_with_16le_device_handler, ACCESSING_BITS_16_31 )


#define DEV_WRITE16LETO32LE_MSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32le_with_16le_device_handler, ACCESSING_BITS_16_31 )


#define DEV_READWRITE16LETO32LE_MSW( name, read16, write16 ) \
DEV_READ16LETO32LE_MSW(name,read16) \
DEV_WRITE16LETO32LE_MSW(name,write16)

/*************************************
 *  16be->32be, LSW mapping
 *************************************/

#define DEV_READ16BETO32BE_LSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32be_with_16be_device_handler, ACCESSING_BITS_0_15 )


#define DEV_WRITE16BETO32BE_LSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32be_with_16be_device_handler, ACCESSING_BITS_0_15 )


#define DEV_READWRITE16BETO32BE_LSW( name, read16, write16 ) \
DEV_READ16BETO32BE_LSW(name,read16) \
DEV_WRITE16BETO32BE_LSW(name,write16)


/*************************************
 *  16le->32be, LSW mapping
 *************************************/

#define DEV_READ16LETO32BE_LSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32be_with_16le_device_handler, ACCESSING_BITS_0_15 )


#define DEV_WRITE16LETO32BE_LSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32be_with_16le_device_handler, ACCESSING_BITS_0_15 )


#define DEV_READWRITE16LETO32BE_LSW( name, read16, write16 ) \
DEV_READ16LETO32BE_LSW(name,read16) \
DEV_WRITE16LETO32BE_LSW(name,write16)


/*************************************
 *  16be->32le, LSW mapping
 *************************************/

#define DEV_READ16BETO32LE_LSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32le_with_16be_device_handler, ACCESSING_BITS_0_15 )


#define DEV_WRITE16BETO32LE_LSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32le_with_16be_device_handler, ACCESSING_BITS_0_15 )


#define DEV_READWRITE16BETO32LE_LSW( name, read16, write16 ) \
DEV_READ16BETO32LE_LSW(name,read16) \
DEV_WRITE16BETO32LE_LSW(name,write16)


/*************************************
 *  16le->32le, LSW mapping
 *************************************/

#define DEV_READ16LETO32LE_LSW( name, read16 ) \
DEV_READ_TEMPLATE_COND( 32, name, read16, read32le_with_16le_device_handler, ACCESSING_BITS_0_15 )


#define DEV_WRITE16LETO32LE_LSW( name, write16 ) \
DEV_WRITE_TEMPLATE_COND( 32, name, write16, write32le_with_16le_device_handler, ACCESSING_BITS_0_15 )


#define DEV_READWRITE16LETO32LE_LSW( name, read16, write16 ) \
DEV_READ16LETO32LE_LSW(name,read16) \
DEV_WRITE16LETO32LE_LSW(name,write16)

