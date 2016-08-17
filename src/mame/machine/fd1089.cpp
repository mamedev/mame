// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Andreas Naive, Charles MacDonald
/***************************************************************************

    Hitachi FD1089A/FD1089B encryption emulation

****************************************************************************

    Copyright Nicola Salmoria, Andreas Naive, and Charles MacDonald.

    All rights reserved.

****************************************************************************

    Decryption tables provided by Charles MacDonald
    Decryption algorithm by Nicola Salmoria
    LCG algorithm by Andreas Naive

    The FD1089 is a 68000 with built-in encryption.
    It contains some battery-backed RAM, when the battery dies the CPU stops
    working.

    Both opcodes and data are encrypted, using different (but related) mappings.
    Decryption works on 16-bit words, but only 8 bits are affected, the other 8 are
    left untouched. A special value in internal RAM disables the encryption, this
    is necessary otherwise RAM would not work as expected (writing data and reading
    it back would return a different number).

    The FD1089A and FD1089B work in the same way, but the decryption tables are
    different. The internal RAM contains the 8-bit key to use at every address
    (only 12 bits of the address are used, so the encryption repeats).

    The FD1089 design is clearly derived from the MC8123. The MC8123 is a Z80 so it
    made sense to encrypt all 8 data bits and use 12 of the 16 address bits. It makes
    a lot less sense to encrypt only half of the 16 data bits; using 12 of the 24
    address bits might be ok, but not in the way it was done. The choice of address
    bits to use was probably dictated by the need to not encrypt data in certain areas
    of the address space, so they had to include the top 8 bits of the address.
    However this means that if you pick e.g. area 000000-00FFFF, where most of the
    program code resides, just 4 address bits affect the encryption, making it very
    weak when compared to the MC8123. Out of the 16KB of internal RAM, you need less
    than 128 bytes to decrypt a whole game - the rest is not used. A waste of space
    and security. Also, since only 8 of the 16 bits are encrypted, it is very easy
    to use the unencrypted ones to search for known sequences of code or data.

    Like for the MC8123, the contents of the internal RAM were generated using a
    linear congruential generator, so the whole key can be generated starting from
    a single 24-bit seed. Note however that the "don't decrypt" data sections need
    special treatment so it's not possible to derive the precise key without access
    to the CPU.

    static int rndseed;

    int rnd()
    {
        rndseed = rndseed * 0x290029;
        return (rndseed >> 16) & 0xff;
    }

    void generate_key(int seed)
    {
        int i;

        rndseed = seed;
        for (i = 0; i < 0x1000; ++i)
        {
            if ("we must encrypt this data table position")
            {
                UINT8 byteval;

                do
                {
                    byteval = rnd();
                } while (byteval == 0x40);

                opcode_key[i] = byteval;

                do
                {
                    byteval = rnd();
                } while (byteval == 0x40);

                data_key[i] = byteval;
            }
        }

        for (i = 0; i < 0x1000; ++i)
        {
            if ("we mustn't encrypt this data table position")
            {
                UINT8 byteval;

                do
                {
                    byteval = rnd();
                } while (byteval == 0x40);

                opcode_key[i] = byteval;
                data_key[i] = 0x40;
            }
        }
    }


    Note that when both FD1089A and FD1089B versions of a game exist, they use the
    same key.

    Known games that use this CPU:

    CPU #     Type  Status   Game              Seed   Unencrypted data range
    --------- ------- --- -------------------- ------ -----------------------------------
    317-0013A FD1089B [1] Enduro Racer         400001 030000-04ffff + 100000-1fffff
    317-0016  FD1089? ??? Fantasy Zone         400005 ?
    317-0018  FD1089A [1] Action Fighter       400003 400000-4fffff + 840000-8dffff + c00000-c4ffff + ff0000-ffffff
    317-0019  FD1089A [1] Outrun               400007 000000-03ffff
    317-0021  FD1089A [2] Alex Kidd            40000b ?
    317-0022  FD1089A [1] Dunk Shot            40000d 030000-ffffff
    317-0024  FD1089B [1] Time Scanner         40000f 000000-02ffff
    317-0027  FD1089B [1] SDI                  400011 000000-03ffff
    317-0028  FD1089A [2] Defense              400011 ?
    317-0033  FD1089A [1] Alien Syndrome       400013 030000-ffffff
    317-0037  FD1089B [2] Alien Syndrome       400013 030000-ffffff
    317-0034  FD1089B [1] Super Hang-On        400015 030000-06ffff + 100000-2fffff + ff0000-ffffff
    317-0086  FD1089A [2] Wonder Boy III       400043 ?
    317-0167  FD1089A [2] Aurail               400030 010000-ffffff
    317-0168  FD1089B [1] Aurail               400030 010000-ffffff
    317-5021  FD1089B [1] Sukeban Jansi Ryuko  40004b 000000-00ffff


    [1] Complete
    [2] Partial

***************************************************************************/

#include "emu.h"
#include "fd1089.h"
#include "segaic16.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// device type definition
const device_type FD1089A = &device_creator<fd1089a_device>;
const device_type FD1089B = &device_creator<fd1089b_device>;

// common base lookup table, shared between A and B variants
const UINT8 fd1089_base_device::s_basetable_fd1089[0x100] =
{
	0x00,0x1c,0x76,0x6a,0x5e,0x42,0x24,0x38,0x4b,0x67,0xad,0x81,0xe9,0xc5,0x03,0x2f,
	0x45,0x69,0xaf,0x83,0xe7,0xcb,0x01,0x2d,0x02,0x1e,0x78,0x64,0x5c,0x40,0x2a,0x36,
	0x32,0x2e,0x44,0x58,0xe4,0xf8,0x9e,0x82,0x29,0x05,0xcf,0xe3,0x93,0xbf,0x79,0x55,
	0x3f,0x13,0xd5,0xf9,0x85,0xa9,0x63,0x4f,0xb8,0xa4,0xc2,0xde,0x6e,0x72,0x18,0x04,
	0x0c,0x10,0x7a,0x66,0xfc,0xe0,0x86,0x9a,0x47,0x6b,0xa1,0x8d,0xbb,0x97,0x51,0x7d,
	0x17,0x3b,0xfd,0xd1,0xeb,0xc7,0x0d,0x21,0xa0,0xbc,0xda,0xc6,0x50,0x4c,0x26,0x3a,
	0x3e,0x22,0x48,0x54,0x46,0x5a,0x3c,0x20,0x25,0x09,0xc3,0xef,0xc1,0xed,0x2b,0x07,
	0x6d,0x41,0x87,0xab,0x89,0xa5,0x6f,0x43,0x1a,0x06,0x60,0x7c,0x62,0x7e,0x14,0x08,
	0x0a,0x16,0x70,0x6c,0xdc,0xc0,0xaa,0xb6,0x4d,0x61,0xa7,0x8b,0xf7,0xdb,0x11,0x3d,
	0x5b,0x77,0xbd,0x91,0xe1,0xcd,0x0b,0x27,0x80,0x9c,0xf6,0xea,0x56,0x4a,0x2c,0x30,
	0xb0,0xac,0xca,0xd6,0xee,0xf2,0x98,0x84,0x37,0x1b,0xdd,0xf1,0x95,0xb9,0x73,0x5f,
	0x39,0x15,0xdf,0xf3,0x9b,0xb7,0x71,0x5d,0xb2,0xae,0xc4,0xd8,0xec,0xf0,0x96,0x8a,
	0xa8,0xb4,0xd2,0xce,0xd0,0xcc,0xa6,0xba,0x1f,0x33,0xf5,0xd9,0xfb,0xd7,0x1d,0x31,
	0x57,0x7b,0xb1,0x9d,0xb3,0x9f,0x59,0x75,0x8c,0x90,0xfa,0xe6,0xf4,0xe8,0x8e,0x92,
	0x12,0x0e,0x68,0x74,0xe2,0xfe,0x94,0x88,0x65,0x49,0x8f,0xa3,0x99,0xb5,0x7f,0x53,
	0x35,0x19,0xd3,0xff,0xc9,0xe5,0x23,0x0f,0xbe,0xa2,0xc8,0xd4,0x4e,0x52,0x34,0x28,
};

// address decryption parameters, shared between A and B variants
const fd1089_base_device::decrypt_parameters fd1089_base_device::s_addr_params[16] =
{
	{ 0x23, 6,4,5,7,3,0,1,2 },
	{ 0x92, 2,5,3,6,7,1,0,4 },
	{ 0xb8, 6,7,4,2,0,5,1,3 },
	{ 0x74, 5,3,7,1,4,6,0,2 },
	{ 0xcf, 7,4,1,0,6,2,3,5 },
	{ 0xc4, 3,1,6,4,5,0,2,7 },
	{ 0x51, 5,7,2,4,3,1,6,0 },
	{ 0x14, 7,2,0,6,1,3,4,5 },
	{ 0x7f, 3,5,6,0,2,1,7,4 },
	{ 0x03, 2,3,4,0,6,7,5,1 },
	{ 0x96, 3,1,7,5,2,4,6,0 },
	{ 0x30, 7,6,2,3,0,4,5,1 },
	{ 0xe2, 1,0,3,7,4,5,2,6 },
	{ 0x72, 1,6,0,5,7,2,4,3 },
	{ 0xf5, 0,4,1,2,6,5,7,3 },
	{ 0x5b, 0,7,5,3,1,4,2,6 },
};

// data decryption parameters for the A variant
const fd1089_base_device::decrypt_parameters fd1089_base_device::s_data_params_a[16] =
{
	{ 0x55, 6,5,1,0,7,4,2,3 },
	{ 0x94, 7,6,4,2,0,5,1,3 },
	{ 0x8d, 1,4,2,3,0,6,7,5 },
	{ 0x9a, 4,3,5,6,0,2,1,7 },
	{ 0x72, 4,3,7,0,5,6,1,2 },
	{ 0xff, 1,7,2,3,6,4,5,0 },
	{ 0x06, 6,5,3,2,4,1,0,7 },
	{ 0xc5, 3,5,1,4,2,7,0,6 },
	{ 0xec, 4,7,5,1,6,0,2,3 },
	{ 0x89, 3,5,0,6,1,2,7,4 },
	{ 0x5c, 1,3,0,7,5,2,4,6 },
	{ 0x3f, 7,3,0,2,4,6,1,5 },
	{ 0x57, 6,4,7,2,1,5,3,0 },
	{ 0xf7, 6,3,7,0,5,4,2,1 },
	{ 0x3a, 6,1,3,2,7,4,5,0 },
	{ 0xac, 1,6,3,5,0,7,4,2 },
};

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 16, fd1094_device )
	AM_RANGE(0x00000, 0xfffff) AM_ROM AM_SHARE(":fd1089_decrypted_opcodes")
ADDRESS_MAP_END


//**************************************************************************
//  CORE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  fd1089_base_device - constructor
//-------------------------------------------------

fd1089_base_device::fd1089_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: m68000_device(mconfig, tag, owner, clock, shortname, source),
		m_region(*this, DEVICE_SELF),
		m_key(*this, "key"),
		m_decrypted_opcodes(*this, ":fd1089_decrypted_opcodes")
{
	// override the name after the m68000 initializes
	m_name.assign(name);

	// add the decrypted opcodes map
	m_address_map[AS_DECRYPTED_OPCODES] = ADDRESS_MAP_NAME(decrypted_opcodes_map);
}

fd1089a_device::fd1089a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: fd1089_base_device(mconfig, FD1089A, "FD1089A", tag, owner, clock, "fd1089a", __FILE__)
{
}

fd1089b_device::fd1089b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: fd1089_base_device(mconfig, FD1089B, "FD1089B", tag, owner, clock, "fd1089b", __FILE__)
{
}

//-------------------------------------------------
//  device_start - one-time device initialization
//-------------------------------------------------

void fd1089_base_device::device_start()
{
	// start the base device
	m68000_device::device_start();

	// get a pointer to the ROM region
	UINT16 *rombase = reinterpret_cast<UINT16 *>(m_region->base());

	// determine length and resize our internal buffers
	UINT32 romsize = m_region->bytes();
	m_plaintext.resize(romsize/2);

	// copy the plaintext
	memcpy(&m_plaintext[0], rombase, romsize);

	// decrypt it, overwriting original data with the decrypted data
	decrypt(0x000000, romsize, &m_plaintext[0], &m_decrypted_opcodes[0], rombase);
}



//**************************************************************************
//  INTERNAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  rearrange_key - shuffle bits in the table
//  based on whether this is an opcode or a data
//  decode
//-------------------------------------------------

UINT8 fd1089_base_device::rearrange_key(UINT8 table, bool opcode)
{
	if (!opcode)
	{
		table ^= (1<<4);
		table ^= (1<<5);
		table ^= (1<<6);

		if (BIT(~table,3))
			table ^= (1<<1);

		if (BIT(table,6))
			table ^= (1<<7);

		table = BITSWAP8(table,1,0,6,4,3,5,2,7);

		if (BIT(table,6))
			table = BITSWAP8(table,7,6,2,4,5,3,1,0);
	}
	else
	{
		table ^= (1<<2);
		table ^= (1<<3);
		table ^= (1<<4);

		if (BIT(~table,3))
			table ^= (1<<5);

		if (BIT(~table,7))
			table ^= (1<<6);

		table = BITSWAP8(table,5,6,7,4,2,3,1,0);

		if (BIT(table,6))
			table = BITSWAP8(table,7,6,5,3,2,4,1,0);
	}

	if (BIT(table,6))
	{
		if (BIT(table,5))
			table ^= (1<<4);
	}
	else
	{
		if (BIT(~table,4))
			table ^= (1<<5);
	}

	return table;
}


//-------------------------------------------------
//  decode_fd1089a - decode an 8-bit value
//  according to FD1089A rules
//-------------------------------------------------

UINT8 fd1089a_device::decode(UINT8 val, UINT8 key, bool opcode)
{
	// special case - don't decrypt
	if (key == 0x40)
		return val;

	UINT8 table = rearrange_key(key, opcode);

	const decrypt_parameters &p = s_addr_params[table >> 4];
	val = BITSWAP8(val, p.s7,p.s6,p.s5,p.s4,p.s3,p.s2,p.s1,p.s0) ^ p.xorval;

	if (BIT(table,3)) val ^= 0x01;
	if (BIT(table,0)) val ^= 0xb1;
	if (opcode) val ^= 0x34;
	if (opcode == 0)
		if (BIT(table,6))
			val ^= 0x01;

	val = s_basetable_fd1089[val];

	UINT8 family = table & 0x07;
	if (opcode == 0)
	{
		if (BIT(~table,6) & BIT(table,2)) family ^= 8;
		if (BIT(table,4)) family ^= 8;
	}
	else
	{
		if (BIT(table,6) & BIT(table,2)) family ^= 8;
		if (BIT(table,5)) family ^= 8;
	}

	if (BIT(table,0))
	{
		if (BIT(val,0)) val ^= 0xc0;
		if (BIT(~val,6) ^ BIT(val,4))
			val = BITSWAP8(val, 7,6,5,4,1,0,2,3);
	}
	else
	{
		if (BIT(~val,6) ^ BIT(val,4))
			val = BITSWAP8(val, 7,6,5,4,0,1,3,2);
	}
	if (BIT(~val,6))
		val = BITSWAP8(val, 7,6,5,4,2,3,0,1);

	const decrypt_parameters &q = s_data_params_a[family];

	val ^= q.xorval;
	val = BITSWAP8(val, q.s7,q.s6,q.s5,q.s4,q.s3,q.s2,q.s1,q.s0);

	return val;
}


//-------------------------------------------------
//  decode_fd1089b - decode an 8-bit value
//  according to FD1089B rules
//-------------------------------------------------

UINT8 fd1089b_device::decode(UINT8 val, UINT8 key, bool opcode)
{
	// special case - don't decrypt
	if (key == 0x40)
		return val;

	UINT8 table = rearrange_key(key, opcode);

	const decrypt_parameters &p = s_addr_params[table >> 4];
	val = BITSWAP8(val, p.s7,p.s6,p.s5,p.s4,p.s3,p.s2,p.s1,p.s0) ^ p.xorval;

	if (BIT(table,3)) val ^= 0x01;
	if (BIT(table,0)) val ^= 0xb1;
	if (opcode) val ^= 0x34;
	if (opcode == 0)
		if (BIT(table,6))
			val ^= 0x01;

	val = s_basetable_fd1089[val];

	UINT8 xorval = 0;
	if (opcode == 0)
	{
		if (BIT(~table,6) & BIT(table,2)) xorval ^= 0x01;
		if (BIT(table,4)) xorval ^= 0x01;
	}
	else
	{
		if (BIT(table,6) & BIT(table,2)) xorval ^= 0x01;
		if (BIT(table,5)) xorval ^= 0x01;
	}

	val ^= xorval;

	if (BIT(table,2))
	{
		val = BITSWAP8(val, 7,6,5,4,1,0,3,2);

		if (BIT(table,0) ^ BIT(table,1))
			val = BITSWAP8(val, 7,6,5,4,0,1,3,2);
	}
	else
	{
		val = BITSWAP8(val, 7,6,5,4,3,2,0,1);

		if (BIT(table,0) ^ BIT(table,1))
			val = BITSWAP8(val, 7,6,5,4,1,0,2,3);
	}

	return val;
}


//-------------------------------------------------
//  decrypt_one - decrypt a single 16-bit value
//  interpreted as being read at the given address
//  as either an opcode or as data
//-------------------------------------------------

UINT16 fd1089_base_device::decrypt_one(offs_t addr, UINT16 val, const UINT8 *key, bool opcode)
{
	// pick the translation table from bits ff022a of the address
	int tbl_num =   ((addr & 0x000002) >> 1) |
					((addr & 0x000008) >> 2) |
					((addr & 0x000020) >> 3) |
					((addr & 0x000200) >> 6) |
					((addr & 0xff0000) >> 12);

	UINT16 src =    ((val & 0x0008) >> 3) |
					((val & 0x0040) >> 5) |
					((val & 0xfc00) >> 8);

	src = decode(src, key[tbl_num + (opcode ? 0 : 1) * 0x1000], opcode);

	src =   ((src & 0x01) << 3) |
			((src & 0x02) << 5) |
			((src & 0xfc) << 8);

	return (val & ~0xfc48) | src;
}


//-------------------------------------------------
//  decrypt - decrypt a buffers' worth of opcodes
//  and data
//-------------------------------------------------

void fd1089_base_device::decrypt(offs_t baseaddr, UINT32 size, const UINT16 *srcptr, UINT16 *opcodesptr, UINT16 *dataptr)
{
	for (offs_t offset = 0; offset < size; offset += 2)
	{
		UINT16 src = srcptr[offset / 2];
		opcodesptr[offset / 2] = decrypt_one(baseaddr + offset, src, &m_key[0], true);
		dataptr[offset / 2] = decrypt_one(baseaddr + offset, src, &m_key[0], false);
	}
}
