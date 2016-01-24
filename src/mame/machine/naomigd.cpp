// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "naomigd.h"
#include "imagedev/chd_cd.h"

/*
    Dimm board registers (add more information if you find it):

    Name:                   Naomi   Dimm Bd.
    NAOMI_DIMM_COMMAND    = 5f703c  14000014 (16 bit):
        if bits all 1 no dimm board present and other registers not used
        bit 15: during an interrupt is 1 if the dimm board has a command to be executed
        bit 14-9: 6 bit command number (naomi bios understands 0 1 3 4 5 6 8 9 a)
        bit 7-0: higher 8 bits of 24 bit offset parameter
    NAOMI_DIMM_OFFSETL    = 5f7040  14000018 (16 bit):
        bit 15-0: lower 16 bits of 24 bit offset parameter
    NAOMI_DIMM_PARAMETERL = 5f7044  1400001c (16 bit)
    NAOMI_DIMM_PARAMETERH = 5f7048  14000020 (16 bit)
    NAOMI_DIMM_STATUS     = 5f704c  14000024 (16 bit):
        bit 0: when 0 signal interrupt from naomi to dimm board
        bit 8: when 0 signal interrupt from dimm board to naomi

*/

const device_type NAOMI_GDROM_BOARD = &device_creator<naomi_gdrom_board>;

const UINT32 naomi_gdrom_board::DES_LEFTSWAP[] = {
	0x00000000, 0x00000001, 0x00000100, 0x00000101, 0x00010000, 0x00010001, 0x00010100, 0x00010101,
	0x01000000, 0x01000001, 0x01000100, 0x01000101, 0x01010000, 0x01010001, 0x01010100, 0x01010101
};

const UINT32 naomi_gdrom_board::DES_RIGHTSWAP[] = {
	0x00000000, 0x01000000, 0x00010000, 0x01010000, 0x00000100, 0x01000100, 0x00010100, 0x01010100,
	0x00000001, 0x01000001, 0x00010001, 0x01010001, 0x00000101, 0x01000101, 0x00010101, 0x01010101,
};

const UINT32 naomi_gdrom_board::DES_SBOX1[] = {
	0x00808200, 0x00000000, 0x00008000, 0x00808202, 0x00808002, 0x00008202, 0x00000002, 0x00008000,
	0x00000200, 0x00808200, 0x00808202, 0x00000200, 0x00800202, 0x00808002, 0x00800000, 0x00000002,
	0x00000202, 0x00800200, 0x00800200, 0x00008200, 0x00008200, 0x00808000, 0x00808000, 0x00800202,
	0x00008002, 0x00800002, 0x00800002, 0x00008002, 0x00000000, 0x00000202, 0x00008202, 0x00800000,
	0x00008000, 0x00808202, 0x00000002, 0x00808000, 0x00808200, 0x00800000, 0x00800000, 0x00000200,
	0x00808002, 0x00008000, 0x00008200, 0x00800002, 0x00000200, 0x00000002, 0x00800202, 0x00008202,
	0x00808202, 0x00008002, 0x00808000, 0x00800202, 0x00800002, 0x00000202, 0x00008202, 0x00808200,
	0x00000202, 0x00800200, 0x00800200, 0x00000000, 0x00008002, 0x00008200, 0x00000000, 0x00808002
};

const UINT32 naomi_gdrom_board::DES_SBOX2[] = {
	0x40084010, 0x40004000, 0x00004000, 0x00084010, 0x00080000, 0x00000010, 0x40080010, 0x40004010,
	0x40000010, 0x40084010, 0x40084000, 0x40000000, 0x40004000, 0x00080000, 0x00000010, 0x40080010,
	0x00084000, 0x00080010, 0x40004010, 0x00000000, 0x40000000, 0x00004000, 0x00084010, 0x40080000,
	0x00080010, 0x40000010, 0x00000000, 0x00084000, 0x00004010, 0x40084000, 0x40080000, 0x00004010,
	0x00000000, 0x00084010, 0x40080010, 0x00080000, 0x40004010, 0x40080000, 0x40084000, 0x00004000,
	0x40080000, 0x40004000, 0x00000010, 0x40084010, 0x00084010, 0x00000010, 0x00004000, 0x40000000,
	0x00004010, 0x40084000, 0x00080000, 0x40000010, 0x00080010, 0x40004010, 0x40000010, 0x00080010,
	0x00084000, 0x00000000, 0x40004000, 0x00004010, 0x40000000, 0x40080010, 0x40084010, 0x00084000
};

const UINT32 naomi_gdrom_board::DES_SBOX3[] = {
	0x00000104, 0x04010100, 0x00000000, 0x04010004, 0x04000100, 0x00000000, 0x00010104, 0x04000100,
	0x00010004, 0x04000004, 0x04000004, 0x00010000, 0x04010104, 0x00010004, 0x04010000, 0x00000104,
	0x04000000, 0x00000004, 0x04010100, 0x00000100, 0x00010100, 0x04010000, 0x04010004, 0x00010104,
	0x04000104, 0x00010100, 0x00010000, 0x04000104, 0x00000004, 0x04010104, 0x00000100, 0x04000000,
	0x04010100, 0x04000000, 0x00010004, 0x00000104, 0x00010000, 0x04010100, 0x04000100, 0x00000000,
	0x00000100, 0x00010004, 0x04010104, 0x04000100, 0x04000004, 0x00000100, 0x00000000, 0x04010004,
	0x04000104, 0x00010000, 0x04000000, 0x04010104, 0x00000004, 0x00010104, 0x00010100, 0x04000004,
	0x04010000, 0x04000104, 0x00000104, 0x04010000, 0x00010104, 0x00000004, 0x04010004, 0x00010100
};

const UINT32 naomi_gdrom_board::DES_SBOX4[] = {
	0x80401000, 0x80001040, 0x80001040, 0x00000040, 0x00401040, 0x80400040, 0x80400000, 0x80001000,
	0x00000000, 0x00401000, 0x00401000, 0x80401040, 0x80000040, 0x00000000, 0x00400040, 0x80400000,
	0x80000000, 0x00001000, 0x00400000, 0x80401000, 0x00000040, 0x00400000, 0x80001000, 0x00001040,
	0x80400040, 0x80000000, 0x00001040, 0x00400040, 0x00001000, 0x00401040, 0x80401040, 0x80000040,
	0x00400040, 0x80400000, 0x00401000, 0x80401040, 0x80000040, 0x00000000, 0x00000000, 0x00401000,
	0x00001040, 0x00400040, 0x80400040, 0x80000000, 0x80401000, 0x80001040, 0x80001040, 0x00000040,
	0x80401040, 0x80000040, 0x80000000, 0x00001000, 0x80400000, 0x80001000, 0x00401040, 0x80400040,
	0x80001000, 0x00001040, 0x00400000, 0x80401000, 0x00000040, 0x00400000, 0x00001000, 0x00401040
};

const UINT32 naomi_gdrom_board::DES_SBOX5[] = {
	0x00000080, 0x01040080, 0x01040000, 0x21000080, 0x00040000, 0x00000080, 0x20000000, 0x01040000,
	0x20040080, 0x00040000, 0x01000080, 0x20040080, 0x21000080, 0x21040000, 0x00040080, 0x20000000,
	0x01000000, 0x20040000, 0x20040000, 0x00000000, 0x20000080, 0x21040080, 0x21040080, 0x01000080,
	0x21040000, 0x20000080, 0x00000000, 0x21000000, 0x01040080, 0x01000000, 0x21000000, 0x00040080,
	0x00040000, 0x21000080, 0x00000080, 0x01000000, 0x20000000, 0x01040000, 0x21000080, 0x20040080,
	0x01000080, 0x20000000, 0x21040000, 0x01040080, 0x20040080, 0x00000080, 0x01000000, 0x21040000,
	0x21040080, 0x00040080, 0x21000000, 0x21040080, 0x01040000, 0x00000000, 0x20040000, 0x21000000,
	0x00040080, 0x01000080, 0x20000080, 0x00040000, 0x00000000, 0x20040000, 0x01040080, 0x20000080
};

const UINT32 naomi_gdrom_board::DES_SBOX6[] = {
	0x10000008, 0x10200000, 0x00002000, 0x10202008, 0x10200000, 0x00000008, 0x10202008, 0x00200000,
	0x10002000, 0x00202008, 0x00200000, 0x10000008, 0x00200008, 0x10002000, 0x10000000, 0x00002008,
	0x00000000, 0x00200008, 0x10002008, 0x00002000, 0x00202000, 0x10002008, 0x00000008, 0x10200008,
	0x10200008, 0x00000000, 0x00202008, 0x10202000, 0x00002008, 0x00202000, 0x10202000, 0x10000000,
	0x10002000, 0x00000008, 0x10200008, 0x00202000, 0x10202008, 0x00200000, 0x00002008, 0x10000008,
	0x00200000, 0x10002000, 0x10000000, 0x00002008, 0x10000008, 0x10202008, 0x00202000, 0x10200000,
	0x00202008, 0x10202000, 0x00000000, 0x10200008, 0x00000008, 0x00002000, 0x10200000, 0x00202008,
	0x00002000, 0x00200008, 0x10002008, 0x00000000, 0x10202000, 0x10000000, 0x00200008, 0x10002008
};

const UINT32 naomi_gdrom_board::DES_SBOX7[] = {
	0x00100000, 0x02100001, 0x02000401, 0x00000000, 0x00000400, 0x02000401, 0x00100401, 0x02100400,
	0x02100401, 0x00100000, 0x00000000, 0x02000001, 0x00000001, 0x02000000, 0x02100001, 0x00000401,
	0x02000400, 0x00100401, 0x00100001, 0x02000400, 0x02000001, 0x02100000, 0x02100400, 0x00100001,
	0x02100000, 0x00000400, 0x00000401, 0x02100401, 0x00100400, 0x00000001, 0x02000000, 0x00100400,
	0x02000000, 0x00100400, 0x00100000, 0x02000401, 0x02000401, 0x02100001, 0x02100001, 0x00000001,
	0x00100001, 0x02000000, 0x02000400, 0x00100000, 0x02100400, 0x00000401, 0x00100401, 0x02100400,
	0x00000401, 0x02000001, 0x02100401, 0x02100000, 0x00100400, 0x00000000, 0x00000001, 0x02100401,
	0x00000000, 0x00100401, 0x02100000, 0x00000400, 0x02000001, 0x02000400, 0x00000400, 0x00100001
};

const UINT32 naomi_gdrom_board::DES_SBOX8[] = {
	0x08000820, 0x00000800, 0x00020000, 0x08020820, 0x08000000, 0x08000820, 0x00000020, 0x08000000,
	0x00020020, 0x08020000, 0x08020820, 0x00020800, 0x08020800, 0x00020820, 0x00000800, 0x00000020,
	0x08020000, 0x08000020, 0x08000800, 0x00000820, 0x00020800, 0x00020020, 0x08020020, 0x08020800,
	0x00000820, 0x00000000, 0x00000000, 0x08020020, 0x08000020, 0x08000800, 0x00020820, 0x00020000,
	0x00020820, 0x00020000, 0x08020800, 0x00000800, 0x00000020, 0x08020020, 0x00000800, 0x00020820,
	0x08000800, 0x00000020, 0x08000020, 0x08020000, 0x08020020, 0x08000000, 0x00020000, 0x08000820,
	0x00000000, 0x08020820, 0x00020020, 0x08000020, 0x08020000, 0x08000800, 0x08000820, 0x00000000,
	0x08020820, 0x00020800, 0x00020800, 0x00000820, 0x00000820, 0x00020020, 0x08000000, 0x08020800
};

const UINT32 naomi_gdrom_board::DES_MASK_TABLE[] = {
	0x24000000, 0x10000000, 0x08000000, 0x02080000, 0x01000000,
	0x00200000, 0x00100000, 0x00040000, 0x00020000, 0x00010000,
	0x00002000, 0x00001000, 0x00000800, 0x00000400, 0x00000200,
	0x00000100, 0x00000020, 0x00000010, 0x00000008, 0x00000004,
	0x00000002, 0x00000001, 0x20000000, 0x10000000, 0x08000000,
	0x04000000, 0x02000000, 0x01000000, 0x00200000, 0x00100000,
	0x00080000, 0x00040000, 0x00020000, 0x00010000, 0x00002000,
	0x00001000, 0x00000808, 0x00000400, 0x00000200, 0x00000100,
	0x00000020, 0x00000011, 0x00000004, 0x00000002
};

const UINT8 naomi_gdrom_board::DES_ROTATE_TABLE[16] = {
	1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

void naomi_gdrom_board::permutate(UINT32 &a, UINT32 &b, UINT32 m, int shift)
{
	UINT32 temp;
	temp = ((a>>shift) ^ b) & m;
	a ^= temp<<shift;
	b ^= temp;
}

void naomi_gdrom_board::des_generate_subkeys(const UINT64 key, UINT32 *subkeys)
{
	UINT32 l = key >> 32;
	UINT32 r = key;

	permutate(r, l, 0x0f0f0f0f, 4);
	permutate(r, l, 0x10101010, 0);

	l = (DES_LEFTSWAP[(l >> 0)  & 0xf] << 3) |
		(DES_LEFTSWAP[(l >> 8)  & 0xf] << 2) |
		(DES_LEFTSWAP[(l >> 16) & 0xf] << 1) |
		(DES_LEFTSWAP[(l >> 24) & 0xf] << 0) |
		(DES_LEFTSWAP[(l >> 5)  & 0xf] << 7) |
		(DES_LEFTSWAP[(l >> 13) & 0xf] << 6) |
		(DES_LEFTSWAP[(l >> 21) & 0xf] << 5) |
		(DES_LEFTSWAP[(l >> 29) & 0xf] << 4);

	r = (DES_RIGHTSWAP[(r >> 1)  & 0xf] << 3) |
		(DES_RIGHTSWAP[(r >> 9)  & 0xf] << 2) |
		(DES_RIGHTSWAP[(r >> 17) & 0xf] << 1) |
		(DES_RIGHTSWAP[(r >> 25) & 0xf] << 0) |
		(DES_RIGHTSWAP[(r >> 4)  & 0xf] << 7) |
		(DES_RIGHTSWAP[(r >> 12) & 0xf] << 6) |
		(DES_RIGHTSWAP[(r >> 20) & 0xf] << 5) |
		(DES_RIGHTSWAP[(r >> 28) & 0xf] << 4);

	l &= 0x0fffffff;
	r &= 0x0fffffff;


	for(int round = 0; round < 16; round++) {
		l = ((l << DES_ROTATE_TABLE[round]) | (l >> (28 - DES_ROTATE_TABLE[round]))) & 0x0fffffff;
		r = ((r << DES_ROTATE_TABLE[round]) | (r >> (28 - DES_ROTATE_TABLE[round]))) & 0x0fffffff;

		subkeys[round*2] =
			((l << 4)  & DES_MASK_TABLE[0]) |
			((l << 28) & DES_MASK_TABLE[1]) |
			((l << 14) & DES_MASK_TABLE[2]) |
			((l << 18) & DES_MASK_TABLE[3]) |
			((l << 6)  & DES_MASK_TABLE[4]) |
			((l << 9)  & DES_MASK_TABLE[5]) |
			((l >> 1)  & DES_MASK_TABLE[6]) |
			((l << 10) & DES_MASK_TABLE[7]) |
			((l << 2)  & DES_MASK_TABLE[8]) |
			((l >> 10) & DES_MASK_TABLE[9]) |
			((r >> 13) & DES_MASK_TABLE[10])|
			((r >> 4)  & DES_MASK_TABLE[11])|
			((r << 6)  & DES_MASK_TABLE[12])|
			((r >> 1)  & DES_MASK_TABLE[13])|
			((r >> 14) & DES_MASK_TABLE[14])|
			((r >> 0)  & DES_MASK_TABLE[15])|
			((r >> 5)  & DES_MASK_TABLE[16])|
			((r >> 10) & DES_MASK_TABLE[17])|
			((r >> 3)  & DES_MASK_TABLE[18])|
			((r >> 18) & DES_MASK_TABLE[19])|
			((r >> 26) & DES_MASK_TABLE[20])|
			((r >> 24) & DES_MASK_TABLE[21]);

		subkeys[round*2+1] =
			((l << 15) & DES_MASK_TABLE[22])|
			((l << 17) & DES_MASK_TABLE[23])|
			((l << 10) & DES_MASK_TABLE[24])|
			((l << 22) & DES_MASK_TABLE[25])|
			((l >> 2)  & DES_MASK_TABLE[26])|
			((l << 1)  & DES_MASK_TABLE[27])|
			((l << 16) & DES_MASK_TABLE[28])|
			((l << 11) & DES_MASK_TABLE[29])|
			((l << 3)  & DES_MASK_TABLE[30])|
			((l >> 6)  & DES_MASK_TABLE[31])|
			((l << 15) & DES_MASK_TABLE[32])|
			((l >> 4)  & DES_MASK_TABLE[33])|
			((r >> 2)  & DES_MASK_TABLE[34])|
			((r << 8)  & DES_MASK_TABLE[35])|
			((r >> 14) & DES_MASK_TABLE[36])|
			((r >> 9)  & DES_MASK_TABLE[37])|
			((r >> 0)  & DES_MASK_TABLE[38])|
			((r << 7)  & DES_MASK_TABLE[39])|
			((r >> 7)  & DES_MASK_TABLE[40])|
			((r >> 3)  & DES_MASK_TABLE[41])|
			((r << 2)  & DES_MASK_TABLE[42])|
			((r >> 21) & DES_MASK_TABLE[43]);
	}
}

UINT64 naomi_gdrom_board::des_encrypt_decrypt(bool decrypt, UINT64 src, const UINT32 *des_subkeys)
{
	UINT32 r = (src & 0x00000000ffffffffULL) >> 0;
	UINT32 l = (src & 0xffffffff00000000ULL) >> 32;

	permutate(l, r, 0x0f0f0f0f, 4);
	permutate(l, r, 0x0000ffff, 16);
	permutate(r, l, 0x33333333, 2);
	permutate(r, l, 0x00ff00ff, 8);
	permutate(l, r, 0x55555555, 1);

	int subkey;
	if(decrypt)
		subkey = 30;
	else
		subkey = 0;

	for(int i = 0; i < 32 ; i+=4) {
		UINT32 temp;

		temp = ((r<<1) | (r>>31)) ^ des_subkeys[subkey];
		l ^= DES_SBOX8[ (temp>>0)  & 0x3f ];
		l ^= DES_SBOX6[ (temp>>8)  & 0x3f ];
		l ^= DES_SBOX4[ (temp>>16) & 0x3f ];
		l ^= DES_SBOX2[ (temp>>24) & 0x3f ];
		subkey++;

		temp = ((r>>3) | (r<<29)) ^ des_subkeys[subkey];
		l ^= DES_SBOX7[ (temp>>0)  & 0x3f ];
		l ^= DES_SBOX5[ (temp>>8)  & 0x3f ];
		l ^= DES_SBOX3[ (temp>>16) & 0x3f ];
		l ^= DES_SBOX1[ (temp>>24) & 0x3f ];
		subkey++;
		if(decrypt)
			subkey -= 4;

		temp = ((l<<1) | (l>>31)) ^ des_subkeys[subkey];
		r ^= DES_SBOX8[ (temp>>0)  & 0x3f ];
		r ^= DES_SBOX6[ (temp>>8)  & 0x3f ];
		r ^= DES_SBOX4[ (temp>>16) & 0x3f ];
		r ^= DES_SBOX2[ (temp>>24) & 0x3f ];
		subkey++;

		temp = ((l>>3) | (l<<29)) ^ des_subkeys[subkey];
		r ^= DES_SBOX7[ (temp>>0)  & 0x3f ];
		r ^= DES_SBOX5[ (temp>>8)  & 0x3f ];
		r ^= DES_SBOX3[ (temp>>16) & 0x3f ];
		r ^= DES_SBOX1[ (temp>>24) & 0x3f ];
		subkey++;
		if(decrypt)
			subkey -= 4;
	}

	permutate(r, l, 0x55555555, 1);
	permutate(l, r, 0x00ff00ff, 8);
	permutate(l, r, 0x33333333, 2);
	permutate(r, l, 0x0000ffff, 16);
	permutate(r, l, 0x0f0f0f0f, 4);

	return (UINT64(r) << 32) | UINT64(l);
}

UINT64 naomi_gdrom_board::rev64(UINT64 src)
{
	UINT64 ret;

	ret = ((src & 0x00000000000000ffULL) << 56)
		| ((src & 0x000000000000ff00ULL) << 40)
		| ((src & 0x0000000000ff0000ULL) << 24)
		| ((src & 0x00000000ff000000ULL) << 8 )
		| ((src & 0x000000ff00000000ULL) >> 8 )
		| ((src & 0x0000ff0000000000ULL) >> 24)
		| ((src & 0x00ff000000000000ULL) >> 40)
		| ((src & 0xff00000000000000ULL) >> 56);

	return ret;
}

UINT64 naomi_gdrom_board::read_to_qword(const UINT8 *region)
{
	UINT64 ret = 0;

	for(int i=0;i<8;i++)
		ret |= UINT64(region[i]) << (56-(8*i));

	return ret;
}

void naomi_gdrom_board::write_from_qword(UINT8 *region, UINT64 qword)
{
	for(int i=0;i<8;i++)
		region[i] = qword >> (56-(i*8));
}

naomi_gdrom_board::naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: naomi_board(mconfig, NAOMI_GDROM_BOARD, "Sega NAOMI GDROM Board", tag, owner, clock, "naomi_gdrom_board", __FILE__)
{
	image_tag = nullptr;
	pic_tag = nullptr;
}

void naomi_gdrom_board::static_set_tags(device_t &device, const char *_image_tag, const char *_pic_tag)
{
	naomi_gdrom_board &dev = downcast<naomi_gdrom_board &>(device);
	dev.image_tag = _image_tag;
	dev.pic_tag = _pic_tag;
}

void naomi_gdrom_board::find_file(const char *name, const UINT8 *dir_sector, UINT32 &file_start, UINT32 &file_size)
{
	file_start = 0;
	file_size = 0;
	logerror("Looking for file [%s]\n", name);
	for(UINT32 pos = 0; pos < 2048; pos += dir_sector[pos]) {
		int fnlen = 0;
		if(!(dir_sector[pos+25] & 2)) {
			int len = dir_sector[pos+32];
//          printf("file: [%s]\n", &dir_sector[pos+33+fnlen]);
			for(fnlen=0; fnlen < FILENAME_LENGTH; fnlen++) {
				if((dir_sector[pos+33+fnlen] == ';') && (name[fnlen] == 0)) {
					fnlen = FILENAME_LENGTH+1;
					break;
				}
				if(dir_sector[pos+33+fnlen] != name[fnlen])
					break;
				if(fnlen == len) {
					if(name[fnlen] == 0)
						fnlen = FILENAME_LENGTH+1;
					else
						fnlen = FILENAME_LENGTH;
				}
			}
		}
		if(fnlen == FILENAME_LENGTH+1) {
			// start sector and size of file
			file_start = ((dir_sector[pos+2] << 0) |
							(dir_sector[pos+3] << 8) |
							(dir_sector[pos+4] << 16) |
							(dir_sector[pos+5] << 24));
			file_size =  ((dir_sector[pos+10] << 0) |
							(dir_sector[pos+11] << 8) |
							(dir_sector[pos+12] << 16) |
							(dir_sector[pos+13] << 24));

			logerror("start %08x size %08x\n", file_start, file_size);
			break;
		}
		if (dir_sector[pos] == 0)
			break;
	}
}

void naomi_gdrom_board::device_start()
{
	naomi_board::device_start();

	dimm_data = nullptr;
	dimm_data_size = 0;

	char name[128];
	memset(name,'\0',128);

	UINT64 key;
	UINT8 netpic = 0;

	const UINT8 *picdata = memregion(pic_tag)->base();

	if(picdata) {
		if(memregion(pic_tag)->bytes() >= 0x4000) {
			printf("Real PIC binary found\n");
			for(int i=0;i<7;i++)
				name[i] = picdata[0x7c0+i*2];
			for(int i=0;i<7;i++)
				name[i+7] = picdata[0x7e0+i*2];

			key = 0;
			for(int i=0;i<7;i++)
				key |= UINT64(picdata[0x780+i*2]) << (56 - i*8);

			key |= picdata[0x7a0];

			netpic = picdata[0x6ee];
		} else {
			// use extracted pic data
	//      printf("This PIC key hasn't been converted to a proper PIC binary yet!\n");
			memcpy(name, picdata+33, 7);
			memcpy(name+7, picdata+25, 7);

			key =((UINT64(picdata[0x31]) << 56) |
					(UINT64(picdata[0x32]) << 48) |
					(UINT64(picdata[0x33]) << 40) |
					(UINT64(picdata[0x34]) << 32) |
					(UINT64(picdata[0x35]) << 24) |
					(UINT64(picdata[0x36]) << 16) |
					(UINT64(picdata[0x37]) << 8)  |
					(UINT64(picdata[0x29]) << 0));
		}

		logerror("key is %08x%08x\n", (UINT32)((key & 0xffffffff00000000ULL)>>32), (UINT32)(key & 0x00000000ffffffffULL));

		UINT8 buffer[2048];
		cdrom_file *gdromfile = cdrom_open(machine().rom_load().get_disk_handle(image_tag));
		// primary volume descriptor
		// read frame 0xb06e (frame=sector+150)
		// dimm board firmware starts straight from this frame
		cdrom_read_data(gdromfile, (netpic ? 0 : 45000) + 16, buffer, CD_TRACK_MODE1);
		UINT32 path_table = ((buffer[0x8c+0] << 0) |
								(buffer[0x8c+1] << 8) |
								(buffer[0x8c+2] << 16) |
								(buffer[0x8c+3] << 24));
		// path table
		cdrom_read_data(gdromfile, path_table, buffer, CD_TRACK_MODE1);

		// directory
		UINT8 dir_sector[2048];
		// find data of file
		UINT32 file_start, file_size;

		if (netpic == 0) {
			UINT32 dir = ((buffer[0x2 + 0] << 0) |
				(buffer[0x2 + 1] << 8) |
				(buffer[0x2 + 2] << 16) |
				(buffer[0x2 + 3] << 24));

			cdrom_read_data(gdromfile, dir, dir_sector, CD_TRACK_MODE1);
			find_file(name, dir_sector, file_start, file_size);

			if (file_start && (file_size == 0x100)) {
				// read file
				cdrom_read_data(gdromfile, file_start, buffer, CD_TRACK_MODE1);
				// get "rom" file name
				memset(name, '\0', 128);
				memcpy(name, buffer + 0xc0, FILENAME_LENGTH - 1);
			}
		} else {
			UINT32 i = 0;
			while (i < 2048 && buffer[i] != 0)
			{
				if (buffer[i] == 3 && buffer[i + 8] == 'R' && buffer[i + 9] == 'O' && buffer[i + 10] == 'M')    // find ROM dir
				{
					UINT32 dir = ((buffer[i + 2] << 0) |
						(buffer[i + 3] << 8) |
						(buffer[i + 4] << 16) |
						(buffer[i + 5] << 24));
					memcpy(name, "ROM.BIN", 7);
					cdrom_read_data(gdromfile, dir, dir_sector, CD_TRACK_MODE1);
					break;
				}
				i += buffer[i] + 8 + (buffer[i] & 1);
			}
		}

		find_file(name, dir_sector, file_start, file_size);

		if (file_start) {
			UINT32 file_rounded_size = (file_size + 2047) & -2048;
			for (dimm_data_size = 4096; dimm_data_size < file_rounded_size; dimm_data_size <<= 1);
			dimm_data = auto_alloc_array(machine(), UINT8, dimm_data_size);
			if (dimm_data_size != file_rounded_size)
				memset(dimm_data + file_rounded_size, 0, dimm_data_size - file_rounded_size);

			// read encrypted data into dimm_data
			UINT32 sectors = file_rounded_size / 2048;
			for (UINT32 sec = 0; sec != sectors; sec++)
				cdrom_read_data(gdromfile, file_start + sec, dimm_data + 2048 * sec, CD_TRACK_MODE1);

			UINT32 des_subkeys[32];
			des_generate_subkeys(rev64(key), des_subkeys);

			for (int i = 0; i < file_rounded_size; i += 8)
				write_from_qword(dimm_data + i, rev64(des_encrypt_decrypt(true, rev64(read_to_qword(dimm_data + i)), des_subkeys)));
		}

		// decrypt loaded data
		cdrom_close(gdromfile);

		if(!dimm_data)
			throw emu_fatalerror("GDROM: Could not find the file to decrypt.");
	}

	save_item(NAME(dimm_cur_address));
}

void naomi_gdrom_board::device_reset()
{
	naomi_board::device_reset();

	dimm_cur_address = 0;
}

void naomi_gdrom_board::board_setup_address(UINT32 address, bool is_dma)
{
	dimm_cur_address = address & (dimm_data_size-1);
}

void naomi_gdrom_board::board_get_buffer(UINT8 *&base, UINT32 &limit)
{
	base = dimm_data + dimm_cur_address;
	limit = dimm_data_size - dimm_cur_address;
}

void naomi_gdrom_board::board_advance(UINT32 size)
{
	dimm_cur_address += size;
	if(dimm_cur_address >= dimm_data_size)
		dimm_cur_address %= dimm_data_size;
}
