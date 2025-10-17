// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, R. Belmont
/***************************************************************************

    SEI80BU Z80 ROM decryptor

    Its clock is synced to Z80 CPU that has encrypted ROM, and
    has separated opcode/data decryption method determined by M1 signal.

    Mainly used for decrypt sound Z80 ROM, but seibu/mustache.cpp is
    used it for decrypt main Z80 ROM.

***************************************************************************/

#include "emu.h"
#include "sei80bu.h"


/*
    Games using encrypted sound cpu:

    Air Raid         1987   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Cabal            1988   "Michel/Seibu    sound 11/04/88"
    Dead Angle       1988?  "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Dynamite Duke    1989   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Toki             1989   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."
    Raiden (alt)     1990   "START UP PROGRAM V1.02 (C)1986 SEIBU KAIHATSU INC."

    raiden and the decrypted raidena are not identical, there are vast sections of different data.
    However, there are a few different bytes in the middle of identical data, suggesting a possible
    error in the decryption scheme: they all require an additional XOR with 0x20 and are located at
    similar addresses.
    00002422: 03 23
    000024A1: 00 20
    000024A2: 09 29
    00002822: 48 68
    000028A1: 06 26
    00002A21: 17 37
    00002A22: 00 20
    00002AA1: 12 32
    00002C21: 02 22
    00002CA1: 02 22
    00002CA2: 17 37
*/

DEFINE_DEVICE_TYPE(SEI80BU, sei80bu_device, "sei80bu", "SEI80BU Encrypted Z80 Interface")

sei80bu_device::sei80bu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SEI80BU, tag, owner, clock)
	, device_rom_interface(mconfig, *this)
{
}

u8 sei80bu_device::data_r(offs_t offset)
{
	u16 const a = offset;
	u8 src = read_byte(offset);

	if (BIT( a, 9) & BIT( a, 8))               src ^= 0x80;
	if (BIT( a,11) & BIT( a, 4) &  BIT( a, 1)) src ^= 0x40;
	if (BIT( a,11) & BIT(~a, 8) &  BIT( a, 1)) src ^= 0x04;
	if (BIT( a,13) & BIT(~a, 6) &  BIT( a, 4)) src ^= 0x02;
	if (BIT(~a,11) & BIT( a, 9) &  BIT( a, 2)) src ^= 0x01;

	if (BIT( a,13) & BIT( a, 4)) src = bitswap<8>(src,7,6,5,4,3,2,0,1);
	if (BIT( a, 8) & BIT( a, 4)) src = bitswap<8>(src,7,6,5,4,2,3,1,0);

	return src;
}

u8 sei80bu_device::opcode_r(offs_t offset)
{
	u16 const a = offset;
	u8 src = read_byte(offset);

	if (BIT( a, 9) & BIT( a, 8))               src ^= 0x80;
	if (BIT( a,11) & BIT( a, 4) &  BIT( a, 1)) src ^= 0x40;
	if (BIT(~a,13) & BIT( a,12))               src ^= 0x20;
	if (BIT(~a, 6) & BIT( a, 1))               src ^= 0x10;
	if (BIT(~a,12) & BIT( a, 2))               src ^= 0x08;
	if (BIT( a,11) & BIT(~a, 8) &  BIT( a, 1)) src ^= 0x04;
	if (BIT( a,13) & BIT(~a, 6) &  BIT( a, 4)) src ^= 0x02;
	if (BIT(~a,11) & BIT( a, 9) &  BIT( a, 2)) src ^= 0x01;

	if (BIT( a,13) & BIT( a, 4)) src = bitswap<8>(src,7,6,5,4,3,2,0,1);
	if (BIT( a, 8) & BIT( a, 4)) src = bitswap<8>(src,7,6,5,4,2,3,1,0);
	if (BIT( a,12) & BIT( a, 9)) src = bitswap<8>(src,7,6,4,5,3,2,1,0);
	if (BIT( a,11) & BIT(~a, 6)) src = bitswap<8>(src,6,7,5,4,3,2,1,0);

	return src;
}
