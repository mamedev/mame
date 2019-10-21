// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, David Widel
/***************************************************************************

NEC MC-8123 encryption emulation

Decryption tables provided by David Widel
Decryption algorithm by Nicola Salmoria

The NEC MC-8123 is a Z80 with built-in encryption.
It contains 0x2000 bytes of battery-backed RAM, when the battery dies the
game stops working. It's possible that the CPU would work as a normal Z80
in that case, this hasn't been verified.

When the CPU reads a byte from memory, it uses 12 bits of the address and
the information of whether the byte is an opcode or not to select a byte in
the internal RAM. The byte controls how the decryption works.

A special value in the internal RAM instructs the CPU to not apply the decryption
algorithm at all. This is necessary for the CPU to be able to use RAM areas
normally.

In theory, the size of the key would make the encryption quite secure.

In practice, the contents of the internal RAM were generated using a linear
congruential generator, so the whole key can be generated starting from a single
24-bit seed.

The LCG is the same that was used to generate the FD1094 keys.
Note that the algorithm skips the special value that would instruct the CPU to
not decrypt at that address.

void generate_key(uint8_t (&key)[0x2000], uint32_t seed, unsigned upper_bound)
{
    unsigned i;

    for (i = 0; i < upper_bound; ++i)
    {
        uint8_t byteval;

        do
        {
            seed *= 0x29;
            seed += seed << 16;

            byteval = (~seed >> 16) & 0xff;
        }
        while (byteval == 0xff);

        key[i] = byteval;
    }

    for ( ; i < 0x2000; ++i)
        key[i] = 0xff;
}


The known games that use this CPU are:

CPU #    Game                      Notes              Seed   Upper Limit
-------- ------------------------- ------------------ ------ -----
317-0029 Block Gal                 NEC MC-8123B 651   091755  1800?
317-0030 Perfect Billiard                             9451EC  1C00
317-0040 Toki no Senshi - Chrono Soldier              068713  1c00
317-0042 Opa Opa                                      B31FD0  1C00
317-0043 Wonder Boy Monster Land                      640708  1C00
317-0054 Shinobi (sound CPU)       NEC MC-8123B 652   088992  1800
317-0057 Fantasy Zone 2                               ADFACE  1800
317-0064 Ufo Senshi Yohko Chan                        880603  1C00
317-0066 Altered Beast (sound CPU) NEC MC-8123B 704   ED8600  1800
317-5002 Gigas 1 & 2               NEC MC-8123 638    234567  1C00
317-5004 Eagle Writer              Not Dumped
317-5005 Astro Flash               Not Dumped
317-5008 Perfect Billiard                             9451EC  1C00
317-5012 Ganbare Chinsan Ooshoubu  NEC MC-8123A       804B54  1C00
317-5014 DakkoChan Jansoh          NEC MC-8123B       206850  1C00
317-5??? Ninja Kid II (sound CPU)  NEC MC-8123A 646   27998D  1800
317-???? Center Court (sound CPU)  NEC MC-8123B 703   640506  1800
317-???? Omega                                        861226  1C00?

***************************************************************************/

#include "emu.h"
#include "mc8123.h"


DEFINE_DEVICE_TYPE(MC8123, mc8123_device, "mc8123", "MC-8123")

//-------------------------------------------------
//  mc8123_device - constructor
//-------------------------------------------------

mc8123_device::mc8123_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: z80_device(mconfig, MC8123, tag, owner, clock)
	, m_key(*this, "key", 0x2000)
{
}


namespace {

template <typename T> constexpr u8 BITS(T b) { return u8(1) << b; }
template <typename T, typename... U> constexpr u8 BITS(T b, U... c) { return (u8(1) << b) | BITS(c...); }

} // anonymous namespace

u8 mc8123_device::decrypt_type0(u8 val, u8 param, unsigned swap)
{
	if (swap == 0) val = bitswap<8>(val,7,5,3,1,2,0,6,4);
	if (swap == 1) val = bitswap<8>(val,5,3,7,2,1,0,4,6);
	if (swap == 2) val = bitswap<8>(val,0,3,4,6,7,1,5,2);
	if (swap == 3) val = bitswap<8>(val,0,7,3,2,6,4,1,5);

	if (BIT(param,3) && BIT(val,7))
		val ^= BITS(5,3,0);

	if (BIT(param,2) && BIT(val,6))
		val ^= BITS(7,2,1);

	if (BIT(val,6)) val ^= BITS(7);

	if (BIT(param,1) && BIT(val,7))
		val ^= BITS(6);

	if (BIT(val,2)) val ^= BITS(5,0);

	val ^= BITS(4,3,1);

	if (BIT(param,2)) val ^= BITS(5,2,0);
	if (BIT(param,1)) val ^= BITS(7,6);
	if (BIT(param,0)) val ^= BITS(5,0);

	if (BIT(param,0)) val = bitswap<8>(val,7,6,5,1,4,3,2,0);

	return val;
}


u8 mc8123_device::decrypt_type1a(u8 val, u8 param, unsigned swap)
{
	if (swap == 0) val = bitswap<8>(val,4,2,6,5,3,7,1,0);
	if (swap == 1) val = bitswap<8>(val,6,0,5,4,3,2,1,7);
	if (swap == 2) val = bitswap<8>(val,2,3,6,1,4,0,7,5);
	if (swap == 3) val = bitswap<8>(val,6,5,1,3,2,7,0,4);

	if (BIT(param,2)) val = bitswap<8>(val,7,6,1,5,3,2,4,0);

	if (BIT(val,1)) val ^= BITS(0);
	if (BIT(val,6)) val ^= BITS(3);
	if (BIT(val,7)) val ^= BITS(6,3);
	if (BIT(val,2)) val ^= BITS(6,3,1);
	if (BIT(val,4)) val ^= BITS(7,6,2);

	if (BIT(val,7) ^ BIT(val,2))
		val ^= BITS(4);

	val ^= BITS(6,3,1,0);

	if (BIT(param,3)) val ^= BITS(7,2);
	if (BIT(param,1)) val ^= BITS(6,3);

	if (BIT(param,0)) val = bitswap<8>(val,7,6,1,4,3,2,5,0);

	return val;
}

u8 mc8123_device::decrypt_type1b(u8 val, u8 param, unsigned swap)
{
	if (swap == 0) val = bitswap<8>(val,1,0,3,2,5,6,4,7);
	if (swap == 1) val = bitswap<8>(val,2,0,5,1,7,4,6,3);
	if (swap == 2) val = bitswap<8>(val,6,4,7,2,0,5,1,3);
	if (swap == 3) val = bitswap<8>(val,7,1,3,6,0,2,5,4);

	if (BIT(val,2) && BIT(val,0))
		val ^= BITS(7,4);

	if (BIT(val,7)) val ^= BITS(2);
	if (BIT(val,5)) val ^= BITS(7,2);
	if (BIT(val,1)) val ^= BITS(5);
	if (BIT(val,6)) val ^= BITS(1);
	if (BIT(val,4)) val ^= BITS(6,5);
	if (BIT(val,0)) val ^= BITS(6,2,1);
	if (BIT(val,3)) val ^= BITS(7,6,2,1,0);

	val ^= BITS(6,4,0);

	if (BIT(param,3)) val ^= BITS(4,1);
	if (BIT(param,2)) val ^= BITS(7,6,3,0);
	if (BIT(param,1)) val ^= BITS(4,3);
	if (BIT(param,0)) val ^= BITS(6,2,1,0);

	return val;
}

u8 mc8123_device::decrypt_type2a(u8 val, u8 param, unsigned swap)
{
	if (swap == 0) val = bitswap<8>(val,0,1,4,3,5,6,2,7);
	if (swap == 1) val = bitswap<8>(val,6,3,0,5,7,4,1,2);
	if (swap == 2) val = bitswap<8>(val,1,6,4,5,0,3,7,2);
	if (swap == 3) val = bitswap<8>(val,4,6,7,5,2,3,1,0);

	if (BIT(val,3) || (BIT(param,1) && BIT(val,2)))
		val = bitswap<8>(val,6,0,7,4,3,2,1,5);

	if (BIT(val,5)) val ^= BITS(7);
	if (BIT(val,6)) val ^= BITS(5);
	if (BIT(val,0)) val ^= BITS(6);
	if (BIT(val,4)) val ^= BITS(3,0);
	if (BIT(val,1)) val ^= BITS(2);

	val ^= BITS(7,6,5,4,1);

	if (BIT(param,2)) val ^= BITS(4,3,2,1,0);

	if (BIT(param,3))
	{
		if (BIT(param,0))
			val = bitswap<8>(val,7,6,5,3,4,1,2,0);
		else
			val = bitswap<8>(val,7,6,5,1,2,4,3,0);
	}
	else if (BIT(param,0))
	{
		val = bitswap<8>(val,7,6,5,2,1,3,4,0);
	}

	return val;
}

u8 mc8123_device::decrypt_type2b(u8 val, u8 param, unsigned swap)
{
	// only 0x20 possible encryptions for this method - all others have 0x40
	// this happens because BIT(param,2) cancels the other three

	if (swap == 0) val = bitswap<8>(val,1,3,4,6,5,7,0,2);
	if (swap == 1) val = bitswap<8>(val,0,1,5,4,7,3,2,6);
	if (swap == 2) val = bitswap<8>(val,3,5,4,1,6,2,0,7);
	if (swap == 3) val = bitswap<8>(val,5,2,3,0,4,7,6,1);

	if (BIT(val,7) && BIT(val,3))
		val ^= BITS(6,4,0);

	if (BIT(val,7)) val ^= BITS(2);
	if (BIT(val,5)) val ^= BITS(7,3);
	if (BIT(val,1)) val ^= BITS(5);
	if (BIT(val,4)) val ^= BITS(7,5,3,1);

	if (BIT(val,7) && BIT(val,5))
		val ^= BITS(4,0);

	if (BIT(val,5) && BIT(val,1))
		val ^= BITS(4,0);

	if (BIT(val,6)) val ^= BITS(7,5);
	if (BIT(val,3)) val ^= BITS(7,6,5,1);
	if (BIT(val,2)) val ^= BITS(3,1);

	val ^= BITS(7,3,2,1);

	if (BIT(param,3)) val ^= BITS(6,3,1);
	if (BIT(param,2)) val ^= BITS(7,6,5,3,2,1); // same as the other three combined
	if (BIT(param,1)) val ^= BITS(7);
	if (BIT(param,0)) val ^= BITS(5,2);

	return val;
}

u8 mc8123_device::decrypt_type3a(u8 val, u8 param, unsigned swap)
{
	if (swap == 0) val = bitswap<8>(val,5,3,1,7,0,2,6,4);
	if (swap == 1) val = bitswap<8>(val,3,1,2,5,4,7,0,6);
	if (swap == 2) val = bitswap<8>(val,5,6,1,2,7,0,4,3);
	if (swap == 3) val = bitswap<8>(val,5,6,7,0,4,2,1,3);

	if (BIT(val,2)) val ^= BITS(7,5,4);
	if (BIT(val,3)) val ^= BITS(0);

	if (BIT(param,0)) val = bitswap<8>(val,7,2,5,4,3,1,0,6);

	if (BIT(val,1)) val ^= BITS(6,0);
	if (BIT(val,3)) val ^= BITS(4,2,1);

	if (BIT(param,3)) val ^= BITS(4,3);

	if (BIT(val,3)) val = bitswap<8>(val,5,6,7,4,3,2,1,0);

	if (BIT(val,5)) val ^= BITS(2,1);

	val ^= BITS(6,5,4,3);

	if (BIT(param,2)) val ^= BITS(7);
	if (BIT(param,1)) val ^= BITS(4);
	if (BIT(param,0)) val ^= BITS(0);

	return val;
}

u8 mc8123_device::decrypt_type3b(u8 val, u8 param, unsigned swap)
{
	if (swap == 0) val = bitswap<8>(val,3,7,5,4,0,6,2,1);
	if (swap == 1) val = bitswap<8>(val,7,5,4,6,1,2,0,3);
	if (swap == 2) val = bitswap<8>(val,7,4,3,0,5,1,6,2);
	if (swap == 3) val = bitswap<8>(val,2,6,4,1,3,7,0,5);

	if (BIT(val,2)) val ^= BITS(7);

	if (BIT(val,7)) val = bitswap<8>(val,7,6,3,4,5,2,1,0);

	if (BIT(param,3)) val ^= BITS(7);

	if (BIT(val,4)) val ^= BITS(6);
	if (BIT(val,1)) val ^= BITS(6,4,2);

	if (BIT(val,7) && BIT(val,6))
		val ^= BITS(1);

	if (BIT(val,7)) val ^= BITS(1);

	if (BIT(param,3)) val ^= BITS(7);
	if (BIT(param,2)) val ^= BITS(0);

	if (BIT(param,3)) val = bitswap<8>(val,4,6,3,2,5,0,1,7);

	if (BIT(val,4)) val ^= BITS(1);
	if (BIT(val,5)) val ^= BITS(4);
	if (BIT(val,7)) val ^= BITS(2);

	val ^= BITS(5,3,2);

	if (BIT(param,1)) val ^= BITS(7);
	if (BIT(param,0)) val ^= BITS(3);

	return val;
}

u8 mc8123_device::decrypt_internal(u8 val, u8 key, bool opcode)
{
	unsigned type = 0;
	unsigned swap = 0;
	u8 param = 0;

	key ^= 0xff;

	// no encryption
	if (key == 0x00)
		return val;

	type ^= BIT(key,0) << 0;
	type ^= BIT(key,2) << 0;
	type ^= BIT(key,0) << 1;
	type ^= BIT(key,1) << 1;
	type ^= BIT(key,2) << 1;
	type ^= BIT(key,4) << 1;
	type ^= BIT(key,4) << 2;
	type ^= BIT(key,5) << 2;

	swap ^= BIT(key,0) << 0;
	swap ^= BIT(key,1) << 0;
	swap ^= BIT(key,2) << 1;
	swap ^= BIT(key,3) << 1;

	param ^= BIT(key,0) << 0;
	param ^= BIT(key,0) << 1;
	param ^= BIT(key,2) << 1;
	param ^= BIT(key,3) << 1;
	param ^= BIT(key,0) << 2;
	param ^= BIT(key,1) << 2;
	param ^= BIT(key,6) << 2;
	param ^= BIT(key,1) << 3;
	param ^= BIT(key,6) << 3;
	param ^= BIT(key,7) << 3;

	if (!opcode)
	{
		param ^= BITS(0);
		type ^= BITS(0);
	}

	switch (type)
	{
		default:
		case 0: return decrypt_type0(val, param, swap);
		case 1: return decrypt_type0(val, param, swap);
		case 2: return decrypt_type1a(val, param, swap);
		case 3: return decrypt_type1b(val, param, swap);
		case 4: return decrypt_type2a(val, param, swap);
		case 5: return decrypt_type2b(val, param, swap);
		case 6: return decrypt_type3a(val, param, swap);
		case 7: return decrypt_type3b(val, param, swap);
	}
}


u8 mc8123_device::decrypt(offs_t addr, u8 val, bool opcode)
{
	// pick the translation table from bits fd57 of the address
	offs_t const tbl_num = bitswap<12>(addr,15,14,13,12,11,10,8,6,4,2,1,0);

	return decrypt_internal(val, m_key[tbl_num | (opcode ? 0x0000 : 0x1000)], opcode);
}


void mc8123_device::decode(u8 *rom, u8 *opcodes, unsigned length)
{
	for (unsigned i = 0; i < length; i++)
	{
		unsigned const adr = (i >= 0xc000) ? ((i & 0x3fff) | 0x8000) : i;
		u8 const src = rom[i];

		// decode the opcodes
		opcodes[i] = decrypt(adr, src, true);

		// decode the data
		rom[i] = decrypt(adr, src, false);
	}
}
