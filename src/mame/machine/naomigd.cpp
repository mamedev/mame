// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "naomigd.h"
#include "romload.h"
#include "imagedev/chd_cd.h"

/*

  GPIO pins(main board: EEPROM, DIMM SPDs, option board: PIC16, JPs)
   |
  SH4 <-> 315-6154 <-> PCI bus -> Sega 315-6322 -> Host system interface (NAOMI, Triforce, Chihiro)
   |         |                                  -> 2x DIMM RAM modules
  RAM       RAM                -> Altera (PCI IDE Bus Master Controller) -> IDE bus -> GD-ROM or CF
 16MB       4MB                -> PCnet-FAST III -> Ethernet

315-6154 - SH4 CPU to PCI bridge and SDRAM controller, also used in Sega Hikaru (2x)
315-6322 - DIMM SDRAM controller, DES decryption, host system communication

 SH4 address space
-------------------
00000000 - 001FFFFF Flash ROM (1st half - stock firmware, 2nd half - updated firmware)
04000000 - 040000FF memory/PCI bridge registers (Sega 315-6154)
0C000000 - 0CFFFFFF SH4 local RAM
10000000 - 103FFFFF memory/PCI controller RAM
14000000 - 1BFFFFFF 8x banked pages

internal / PCI memory space
-------------------
00000000 - 000000FF DIMM controller registers (Sega 315-6322)
10000000 - 4FFFFFFF DIMM memory, upto 1GB (if register 28 bit 1 is 0, otherwise some unknown MMIO)
70000000 - 70FFFFFF SH4 local RAM
78000000 - 783FFFFF 315-6154 PCI bridge RAM
C00001xx   IDE registers                 \
C00003xx   IDE registers                  | software configured in VxWorks, preconfigured or hardcoded in 1.02
C000CCxx   IDE Bus Master DMA registers  /
C1xxxxxx   Network registers

PCI configuration space (enabled using memctl 1C reg)
-------------------
00000000 - 00000FFF unknown, write 142 to reg 04 at very start
00001000 - 00001FFF PCI IDE controller (upper board Altera Flex) Vendor 11db Device 189d
00002000 - 00002FFF AMD AM79C973BVC PCnet-FAST III Network

DIMM controller registers
-------------------
14 5F703C |
18 5F7040 |
1C 5F7044 | 16bit  4x Communication registers
20 5F7048 |
24 5F704C   16bit  Interrupt register
                   -------c ---b---a
                    a - IRQ to DIMM (SH4 IRL3): 0 set / 1 clear
                    b - unk, mask of a ???
                    c - IRQ to NAOMI (HOLLY EXT 3): 0 set / 1 clear (write 0 from NAOMI seems ignored)

28          16bit  dddd---c ------ba
                    a - 0->1 NAOMI reset
                    b - 1 seems disable DIMM RAM access, followed by write 01010101 to bank 10 offset 000110 or 000190 (some MMIO?)
                    c - unk, set to 1 in VxWorks, 0 in 1.02
                    d - unk, checked for == 1 in 1.02

2A           8bit  possible DES decryption area size 8 MSB bits (16MB units number)
                   VxWorks firmwares set this to ((DIMMsize >> 24) - 1), 1.02 set it to FF

2C          32bit  SDRAM config
30          32bit  DES key low
34          32bit  DES key high

SH4 IO port A bits
-------------------
9 select input, 0 - main/lower board, 1 - option/upper board (IDE, Net, PIC)
     0             1
0 DIMM SPD clk   JP? 0 - enable IDE
1 DIMM SPD data  JP? 0 - enable Network
2 93C46 DI       PIC16 D0
3 93C46 CS       PIC16 D1
4 93C46 CLK      PIC16 D2
5 93C46 DO       PIC16 CLK



    Dimm board communication registers software level usage:

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

DEFINE_DEVICE_TYPE(NAOMI_GDROM_BOARD, naomi_gdrom_board, "segadimm", "Sega DIMM Board")

const uint32_t naomi_gdrom_board::DES_LEFTSWAP[] = {
	0x00000000, 0x00000001, 0x00000100, 0x00000101, 0x00010000, 0x00010001, 0x00010100, 0x00010101,
	0x01000000, 0x01000001, 0x01000100, 0x01000101, 0x01010000, 0x01010001, 0x01010100, 0x01010101
};

const uint32_t naomi_gdrom_board::DES_RIGHTSWAP[] = {
	0x00000000, 0x01000000, 0x00010000, 0x01010000, 0x00000100, 0x01000100, 0x00010100, 0x01010100,
	0x00000001, 0x01000001, 0x00010001, 0x01010001, 0x00000101, 0x01000101, 0x00010101, 0x01010101,
};

const uint32_t naomi_gdrom_board::DES_SBOX1[] = {
	0x00808200, 0x00000000, 0x00008000, 0x00808202, 0x00808002, 0x00008202, 0x00000002, 0x00008000,
	0x00000200, 0x00808200, 0x00808202, 0x00000200, 0x00800202, 0x00808002, 0x00800000, 0x00000002,
	0x00000202, 0x00800200, 0x00800200, 0x00008200, 0x00008200, 0x00808000, 0x00808000, 0x00800202,
	0x00008002, 0x00800002, 0x00800002, 0x00008002, 0x00000000, 0x00000202, 0x00008202, 0x00800000,
	0x00008000, 0x00808202, 0x00000002, 0x00808000, 0x00808200, 0x00800000, 0x00800000, 0x00000200,
	0x00808002, 0x00008000, 0x00008200, 0x00800002, 0x00000200, 0x00000002, 0x00800202, 0x00008202,
	0x00808202, 0x00008002, 0x00808000, 0x00800202, 0x00800002, 0x00000202, 0x00008202, 0x00808200,
	0x00000202, 0x00800200, 0x00800200, 0x00000000, 0x00008002, 0x00008200, 0x00000000, 0x00808002
};

const uint32_t naomi_gdrom_board::DES_SBOX2[] = {
	0x40084010, 0x40004000, 0x00004000, 0x00084010, 0x00080000, 0x00000010, 0x40080010, 0x40004010,
	0x40000010, 0x40084010, 0x40084000, 0x40000000, 0x40004000, 0x00080000, 0x00000010, 0x40080010,
	0x00084000, 0x00080010, 0x40004010, 0x00000000, 0x40000000, 0x00004000, 0x00084010, 0x40080000,
	0x00080010, 0x40000010, 0x00000000, 0x00084000, 0x00004010, 0x40084000, 0x40080000, 0x00004010,
	0x00000000, 0x00084010, 0x40080010, 0x00080000, 0x40004010, 0x40080000, 0x40084000, 0x00004000,
	0x40080000, 0x40004000, 0x00000010, 0x40084010, 0x00084010, 0x00000010, 0x00004000, 0x40000000,
	0x00004010, 0x40084000, 0x00080000, 0x40000010, 0x00080010, 0x40004010, 0x40000010, 0x00080010,
	0x00084000, 0x00000000, 0x40004000, 0x00004010, 0x40000000, 0x40080010, 0x40084010, 0x00084000
};

const uint32_t naomi_gdrom_board::DES_SBOX3[] = {
	0x00000104, 0x04010100, 0x00000000, 0x04010004, 0x04000100, 0x00000000, 0x00010104, 0x04000100,
	0x00010004, 0x04000004, 0x04000004, 0x00010000, 0x04010104, 0x00010004, 0x04010000, 0x00000104,
	0x04000000, 0x00000004, 0x04010100, 0x00000100, 0x00010100, 0x04010000, 0x04010004, 0x00010104,
	0x04000104, 0x00010100, 0x00010000, 0x04000104, 0x00000004, 0x04010104, 0x00000100, 0x04000000,
	0x04010100, 0x04000000, 0x00010004, 0x00000104, 0x00010000, 0x04010100, 0x04000100, 0x00000000,
	0x00000100, 0x00010004, 0x04010104, 0x04000100, 0x04000004, 0x00000100, 0x00000000, 0x04010004,
	0x04000104, 0x00010000, 0x04000000, 0x04010104, 0x00000004, 0x00010104, 0x00010100, 0x04000004,
	0x04010000, 0x04000104, 0x00000104, 0x04010000, 0x00010104, 0x00000004, 0x04010004, 0x00010100
};

const uint32_t naomi_gdrom_board::DES_SBOX4[] = {
	0x80401000, 0x80001040, 0x80001040, 0x00000040, 0x00401040, 0x80400040, 0x80400000, 0x80001000,
	0x00000000, 0x00401000, 0x00401000, 0x80401040, 0x80000040, 0x00000000, 0x00400040, 0x80400000,
	0x80000000, 0x00001000, 0x00400000, 0x80401000, 0x00000040, 0x00400000, 0x80001000, 0x00001040,
	0x80400040, 0x80000000, 0x00001040, 0x00400040, 0x00001000, 0x00401040, 0x80401040, 0x80000040,
	0x00400040, 0x80400000, 0x00401000, 0x80401040, 0x80000040, 0x00000000, 0x00000000, 0x00401000,
	0x00001040, 0x00400040, 0x80400040, 0x80000000, 0x80401000, 0x80001040, 0x80001040, 0x00000040,
	0x80401040, 0x80000040, 0x80000000, 0x00001000, 0x80400000, 0x80001000, 0x00401040, 0x80400040,
	0x80001000, 0x00001040, 0x00400000, 0x80401000, 0x00000040, 0x00400000, 0x00001000, 0x00401040
};

const uint32_t naomi_gdrom_board::DES_SBOX5[] = {
	0x00000080, 0x01040080, 0x01040000, 0x21000080, 0x00040000, 0x00000080, 0x20000000, 0x01040000,
	0x20040080, 0x00040000, 0x01000080, 0x20040080, 0x21000080, 0x21040000, 0x00040080, 0x20000000,
	0x01000000, 0x20040000, 0x20040000, 0x00000000, 0x20000080, 0x21040080, 0x21040080, 0x01000080,
	0x21040000, 0x20000080, 0x00000000, 0x21000000, 0x01040080, 0x01000000, 0x21000000, 0x00040080,
	0x00040000, 0x21000080, 0x00000080, 0x01000000, 0x20000000, 0x01040000, 0x21000080, 0x20040080,
	0x01000080, 0x20000000, 0x21040000, 0x01040080, 0x20040080, 0x00000080, 0x01000000, 0x21040000,
	0x21040080, 0x00040080, 0x21000000, 0x21040080, 0x01040000, 0x00000000, 0x20040000, 0x21000000,
	0x00040080, 0x01000080, 0x20000080, 0x00040000, 0x00000000, 0x20040000, 0x01040080, 0x20000080
};

const uint32_t naomi_gdrom_board::DES_SBOX6[] = {
	0x10000008, 0x10200000, 0x00002000, 0x10202008, 0x10200000, 0x00000008, 0x10202008, 0x00200000,
	0x10002000, 0x00202008, 0x00200000, 0x10000008, 0x00200008, 0x10002000, 0x10000000, 0x00002008,
	0x00000000, 0x00200008, 0x10002008, 0x00002000, 0x00202000, 0x10002008, 0x00000008, 0x10200008,
	0x10200008, 0x00000000, 0x00202008, 0x10202000, 0x00002008, 0x00202000, 0x10202000, 0x10000000,
	0x10002000, 0x00000008, 0x10200008, 0x00202000, 0x10202008, 0x00200000, 0x00002008, 0x10000008,
	0x00200000, 0x10002000, 0x10000000, 0x00002008, 0x10000008, 0x10202008, 0x00202000, 0x10200000,
	0x00202008, 0x10202000, 0x00000000, 0x10200008, 0x00000008, 0x00002000, 0x10200000, 0x00202008,
	0x00002000, 0x00200008, 0x10002008, 0x00000000, 0x10202000, 0x10000000, 0x00200008, 0x10002008
};

const uint32_t naomi_gdrom_board::DES_SBOX7[] = {
	0x00100000, 0x02100001, 0x02000401, 0x00000000, 0x00000400, 0x02000401, 0x00100401, 0x02100400,
	0x02100401, 0x00100000, 0x00000000, 0x02000001, 0x00000001, 0x02000000, 0x02100001, 0x00000401,
	0x02000400, 0x00100401, 0x00100001, 0x02000400, 0x02000001, 0x02100000, 0x02100400, 0x00100001,
	0x02100000, 0x00000400, 0x00000401, 0x02100401, 0x00100400, 0x00000001, 0x02000000, 0x00100400,
	0x02000000, 0x00100400, 0x00100000, 0x02000401, 0x02000401, 0x02100001, 0x02100001, 0x00000001,
	0x00100001, 0x02000000, 0x02000400, 0x00100000, 0x02100400, 0x00000401, 0x00100401, 0x02100400,
	0x00000401, 0x02000001, 0x02100401, 0x02100000, 0x00100400, 0x00000000, 0x00000001, 0x02100401,
	0x00000000, 0x00100401, 0x02100000, 0x00000400, 0x02000001, 0x02000400, 0x00000400, 0x00100001
};

const uint32_t naomi_gdrom_board::DES_SBOX8[] = {
	0x08000820, 0x00000800, 0x00020000, 0x08020820, 0x08000000, 0x08000820, 0x00000020, 0x08000000,
	0x00020020, 0x08020000, 0x08020820, 0x00020800, 0x08020800, 0x00020820, 0x00000800, 0x00000020,
	0x08020000, 0x08000020, 0x08000800, 0x00000820, 0x00020800, 0x00020020, 0x08020020, 0x08020800,
	0x00000820, 0x00000000, 0x00000000, 0x08020020, 0x08000020, 0x08000800, 0x00020820, 0x00020000,
	0x00020820, 0x00020000, 0x08020800, 0x00000800, 0x00000020, 0x08020020, 0x00000800, 0x00020820,
	0x08000800, 0x00000020, 0x08000020, 0x08020000, 0x08020020, 0x08000000, 0x00020000, 0x08000820,
	0x00000000, 0x08020820, 0x00020020, 0x08000020, 0x08020000, 0x08000800, 0x08000820, 0x00000000,
	0x08020820, 0x00020800, 0x00020800, 0x00000820, 0x00000820, 0x00020020, 0x08000000, 0x08020800
};

const uint32_t naomi_gdrom_board::DES_MASK_TABLE[] = {
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

const uint8_t naomi_gdrom_board::DES_ROTATE_TABLE[16] = {
	1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

void naomi_gdrom_board::permutate(uint32_t &a, uint32_t &b, uint32_t m, int shift)
{
	uint32_t temp;
	temp = ((a>>shift) ^ b) & m;
	a ^= temp<<shift;
	b ^= temp;
}

void naomi_gdrom_board::des_generate_subkeys(const uint64_t key, uint32_t *subkeys)
{
	uint32_t l = key >> 32;
	uint32_t r = key;

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

uint64_t naomi_gdrom_board::des_encrypt_decrypt(bool decrypt, uint64_t src, const uint32_t *des_subkeys)
{
	uint32_t r = (src & 0x00000000ffffffffULL) >> 0;
	uint32_t l = (src & 0xffffffff00000000ULL) >> 32;

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
		uint32_t temp;

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

	return (uint64_t(r) << 32) | uint64_t(l);
}

uint64_t naomi_gdrom_board::rev64(uint64_t src)
{
	uint64_t ret;

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

uint64_t naomi_gdrom_board::read_to_qword(const uint8_t *region)
{
	uint64_t ret = 0;

	for(int i=0;i<8;i++)
		ret |= uint64_t(region[i]) << (56-(8*i));

	return ret;
}

void naomi_gdrom_board::write_from_qword(uint8_t *region, uint64_t qword)
{
	for(int i=0;i<8;i++)
		region[i] = qword >> (56-(i*8));
}

naomi_gdrom_board::naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: naomi_board(mconfig, NAOMI_GDROM_BOARD, tag, owner, clock),
	picdata(*this, finder_base::DUMMY_TAG)
{
	image_tag = nullptr;
}

void naomi_gdrom_board::find_file(const char *name, const uint8_t *dir_sector, uint32_t &file_start, uint32_t &file_size)
{
	file_start = 0;
	file_size = 0;
	logerror("Looking for file [%s]\n", name);
	for(uint32_t pos = 0; pos < 2048; pos += dir_sector[pos]) {
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

	uint64_t key;
	uint8_t netpic = 0;


	if(picdata) {
		if(picdata.length() >= 0x4000) {
			printf("Real PIC binary found\n");
			for(int i=0;i<7;i++)
				name[i] = picdata[0x7c0+i*2];
			for(int i=0;i<7;i++)
				name[i+7] = picdata[0x7e0+i*2];

			key = 0;
			for(int i=0;i<7;i++)
				key |= uint64_t(picdata[0x780+i*2]) << (56 - i*8);

			key |= picdata[0x7a0];

			netpic = picdata[0x6ee];
		} else {
			// use extracted pic data
	//      printf("This PIC key hasn't been converted to a proper PIC binary yet!\n");
			memcpy(name, picdata+33, 7);
			memcpy(name+7, picdata+25, 7);

			key =((uint64_t(picdata[0x31]) << 56) |
					(uint64_t(picdata[0x32]) << 48) |
					(uint64_t(picdata[0x33]) << 40) |
					(uint64_t(picdata[0x34]) << 32) |
					(uint64_t(picdata[0x35]) << 24) |
					(uint64_t(picdata[0x36]) << 16) |
					(uint64_t(picdata[0x37]) << 8)  |
					(uint64_t(picdata[0x29]) << 0));
		}

		logerror("key is %08x%08x\n", (uint32_t)((key & 0xffffffff00000000ULL)>>32), (uint32_t)(key & 0x00000000ffffffffULL));

		uint8_t buffer[2048];
		cdrom_file *gdromfile = cdrom_open(machine().rom_load().get_disk_handle(image_tag));
		// primary volume descriptor
		// read frame 0xb06e (frame=sector+150)
		// dimm board firmware starts straight from this frame
		cdrom_read_data(gdromfile, (netpic ? 0 : 45000) + 16, buffer, CD_TRACK_MODE1);
		uint32_t path_table = ((buffer[0x8c+0] << 0) |
								(buffer[0x8c+1] << 8) |
								(buffer[0x8c+2] << 16) |
								(buffer[0x8c+3] << 24));
		// path table
		cdrom_read_data(gdromfile, path_table, buffer, CD_TRACK_MODE1);

		// directory
		uint8_t dir_sector[2048];
		// find data of file
		uint32_t file_start, file_size;

		if (netpic == 0) {
			uint32_t dir = ((buffer[0x2 + 0] << 0) |
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
			uint32_t i = 0;
			while (i < 2048 && buffer[i] != 0)
			{
				if (buffer[i] == 3 && buffer[i + 8] == 'R' && buffer[i + 9] == 'O' && buffer[i + 10] == 'M')    // find ROM dir
				{
					uint32_t dir = ((buffer[i + 2] << 0) |
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
			uint32_t file_rounded_size = (file_size + 2047) & -2048;
			for (dimm_data_size = 4096; dimm_data_size < file_rounded_size; dimm_data_size <<= 1);
			dimm_data = auto_alloc_array(machine(), uint8_t, dimm_data_size);
			if (dimm_data_size != file_rounded_size)
				memset(dimm_data + file_rounded_size, 0, dimm_data_size - file_rounded_size);

			// read encrypted data into dimm_data
			uint32_t sectors = file_rounded_size / 2048;
			for (uint32_t sec = 0; sec != sectors; sec++)
				cdrom_read_data(gdromfile, file_start + sec, dimm_data + 2048 * sec, CD_TRACK_MODE1);

			uint32_t des_subkeys[32];
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

void naomi_gdrom_board::board_setup_address(uint32_t address, bool is_dma)
{
	dimm_cur_address = address & (dimm_data_size-1);
}

void naomi_gdrom_board::board_get_buffer(uint8_t *&base, uint32_t &limit)
{
	base = dimm_data + dimm_cur_address;
	limit = dimm_data_size - dimm_cur_address;
}

void naomi_gdrom_board::board_advance(uint32_t size)
{
	dimm_cur_address += size;
	if(dimm_cur_address >= dimm_data_size)
		dimm_cur_address %= dimm_data_size;
}

// DIMM firmwares:
//  FPR-23489C - 1.02 not VxWorks based, no network, can not be software updated to 2.xx+
// Net-DIMM firmwares:
// all VxWorkx based, can be updated up to 4.0x, actually 1MB in size, must have CRC32 FFFFFFFF, 1st MB of flash ROM contain stock version, 2nd MB have some updated version
//  ?          - 2.03 factory only, introduced ALL.net features, so far was seen only as stock firmware in 1st half of flash ROM, factory updated to some newer ver in 2nd ROM half
//  FPR23718   - 2.06 factory only, most common version of NAOMI Net-DIMMs, have stock 2.03, IC label need verification
//  ?            2.13 factory or update (NAOMI VF4)
//  ?            2.17 factory or update (NAOMI VF4 Evolution)
//  ?          - 3.01 added network boot support, supports Triforce and Chihiro
//  FPR23905   - 3.03 factory or update (NAOMI WCCF)
//  ?            3.12 factory only
//  ?            3.17 latest known 3.xx version, factory or update (NAOMI VF4 Final Tuned or statndalone disks for Chihiro and Triforce)
// update only - 4.01 supports Compact Flash GD-ROM-replacement
//              "4.02" hack of 4.01 with CF card vendor check disabled

ROM_START( dimm )
	ROM_REGION( 0x200000, "segadimm", 0)
	// Altera FLEX EPF10K30 firmwares (implements PCI IDE controller)
	ROM_LOAD("315-6301.ic11", 0x000000, 0x01ff01, NO_DUMP ) // GD-only DIMM
	ROM_LOAD("315-6334.ic11", 0x000000, 0x01ff01, CRC(534c342d) SHA1(3e879f432c82305487922ab28c07107cf0f3c5cf) ) // Net-DIMM

	// unused and/or unknown security PICs
	// 253-5508-0352E 317-0352-EXP BFC.BIN unknown, presumable some mahjong game (MJ1 ?)
	ROM_LOAD("317-0352-exp.pic", 0x00, 0x4000, CRC(b216fbfc) SHA1(da2341003b35d1600d63fbe34d13ff3b42bdc939) )
	// 253-5508-0422J 317-0422-JPN BHE.BIN Quest of D undumped version, high likely 2.0x "Gofu no Keisyousya"
	ROM_LOAD("317-0422-jpn.pic", 0x00, 0x4000, CRC(54197fbf) SHA1(a18b5b7aec0498c7a62cacf9f2298ddefb7482c9) )
	// 253-5508-0456J 317-0456-JPN BEG.BIN WCCF 2005-2006 undumped Japan version
	ROM_LOAD("317-0456-jpn.pic", 0x00, 0x4000, CRC(cf3bd834) SHA1(6236cdb780260d34c02806478a39c9f3432a45e8) )
	// Sangokushi Taisen 2 satellite firmware update (CDV-10023) key, .BIN file name is unknown/incorrect.
	ROM_LOAD("317-unknown.pic",  0x00, 0x4000, CRC(7dc07733) SHA1(b223dc44718fa71e7b420c3b44ce4ab961445461) )

	// main firmwares
	ROM_LOAD16_WORD_SWAP( "fpr-23489c.ic14", 0x000000, 0x200000, CRC(bc38bea1) SHA1(b36fcc6902f397d9749e9d02de1bbb7a5e29d468) )
	ROM_LOAD16_WORD_SWAP( "203_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(6f55f1ae0606816a4eca6645ed36eb7f9c7ad9cf) )
	ROM_LOAD16_WORD_SWAP( "fpr23718.ic36",   0x000000, 0x200000, CRC(a738ea1c) SHA1(b7b5a55a6a4cf0aa2df1b3dff62ff67f864c55e8) )
	ROM_LOAD16_WORD_SWAP( "213_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(17131f318632610b87bc095156ffad4597fed4ca) )
	ROM_LOAD16_WORD_SWAP( "217_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(e5a229ae7ed48b2955cad63529fd938c6db555e5) )
	ROM_LOAD16_WORD_SWAP( "fpr23905.ic36",   0x000000, 0x200000, CRC(ffffffff) SHA1(acade4362807c7571b1c2a48ed6067e4bddd404b) )
	ROM_LOAD16_WORD_SWAP( "317_312.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(31d698cd659446ee09a2eeedec6e4bc6a19d05e8) )
	ROM_LOAD16_WORD_SWAP( "401_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(edb52597108462bcea8eb2a47c19e51e5fb60638) )
ROM_END

const tiny_rom_entry *naomi_gdrom_board::device_rom_region() const
{
	return ROM_NAME(dimm);
}
