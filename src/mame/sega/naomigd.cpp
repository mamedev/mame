// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "naomigd.h"

#include "bus/ata/gdrom.h"

#include "romload.h"

#include "multibyte.h"

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
                    b - 1 activates sdram command "load mode register", followed by write 01010101 to bank 10 offset 000110 or 000190 depending on cas latency
                        address bits 12-3 correspond to bits A9-A0 in the sdram chip address bus when sending the command
                    c - unk, set to 1 in VxWorks, 0 in 1.02
                    d - unk, checked for == 1 in 1.02

2A           8bit  possible DES decryption area size 8 MSB bits (16MB units number)
                   VxWorks firmwares set this to ((DIMMsize >> 24) - 1), 1.02 set it to FF

2C          32bit  DIMM SDRAM config
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

		temp = rotl_32(r, 1) ^ des_subkeys[subkey];
		l ^= DES_SBOX8[ (temp>>0)  & 0x3f ];
		l ^= DES_SBOX6[ (temp>>8)  & 0x3f ];
		l ^= DES_SBOX4[ (temp>>16) & 0x3f ];
		l ^= DES_SBOX2[ (temp>>24) & 0x3f ];
		subkey++;

		temp = rotr_32(r, 3) ^ des_subkeys[subkey];
		l ^= DES_SBOX7[ (temp>>0)  & 0x3f ];
		l ^= DES_SBOX5[ (temp>>8)  & 0x3f ];
		l ^= DES_SBOX3[ (temp>>16) & 0x3f ];
		l ^= DES_SBOX1[ (temp>>24) & 0x3f ];
		subkey++;
		if(decrypt)
			subkey -= 4;

		temp = rotl_32(l, 1) ^ des_subkeys[subkey];
		r ^= DES_SBOX8[ (temp>>0)  & 0x3f ];
		r ^= DES_SBOX6[ (temp>>8)  & 0x3f ];
		r ^= DES_SBOX4[ (temp>>16) & 0x3f ];
		r ^= DES_SBOX2[ (temp>>24) & 0x3f ];
		subkey++;

		temp = rotr_32(l, 3) ^ des_subkeys[subkey];
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

// For ide gdrom controller

DEFINE_DEVICE_TYPE(IDE_GDROM, idegdrom_device, "ide_gdrom", "ide gdrom controller")

idegdrom_device::idegdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const char *image_tag, const char *space_tag, int space_id)
	: idegdrom_device(mconfig, tag, owner, clock)
{
	space_owner_tag = space_tag;
	space_owner_id = space_id;
}

idegdrom_device::idegdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, IDE_GDROM, tag, owner, clock),
	m_ide(*this, "ide"),
	irq_cb(*this)
{
	set_ids(0x11db189d, 0, 0, 0); // 0x10221000 or 0x00011172 possible too
}

void idegdrom_device::device_start()
{
	pci_device::device_start();
	add_map(0x00000020, M_IO, FUNC(idegdrom_device::map_command));
	bank_infos[0].adr = 0x01c0;
	// pci system does not support base addresses not multiples of size
	add_map(0x00000020, M_IO | M_DISABLED, FUNC(idegdrom_device::map_control));
	bank_infos[1].adr = 0x03b0;
	add_map(0x00000010, M_IO, FUNC(idegdrom_device::map_dma));
	bank_infos[2].adr = 0xcc00;
	command = 0x0083;
}

void idegdrom_device::device_reset()
{
	pci_device::device_reset();
}

void idegdrom_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
	uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	io_space->install_device(io_offset + 0x03b0, io_offset + 0x03cf, *static_cast<idegdrom_device*>(this), &idegdrom_device::map_control);
}

static void gdrom_devices(device_slot_interface &device)
{
	device.option_add("gdrom", ATAPI_GDROM);
}


void idegdrom_device::ide_irq(int state)
{
	irq_cb(state);
}

void idegdrom_device::device_add_mconfig(machine_config &config)
{
	BUS_MASTER_IDE_CONTROLLER(config, m_ide).options(gdrom_devices, "gdrom", nullptr, true);
	m_ide->irq_handler().set(*this, FUNC(idegdrom_device::ide_irq));
	m_ide->set_bus_master_space(space_owner_tag, space_owner_id);
}

void idegdrom_device::map_command(address_map &map)
{
	map(0x0000, 0x001f).rw(FUNC(idegdrom_device::ide_cs0_r), FUNC(idegdrom_device::ide_cs0_w));
}

void idegdrom_device::map_control(address_map &map)
{
	map(0x0000, 0x001f).rw(FUNC(idegdrom_device::ide_cs1_r), FUNC(idegdrom_device::ide_cs1_w));
}

void idegdrom_device::map_dma(address_map &map)
{
	map(0x0000, 0x000f).rw("ide", FUNC(bus_master_ide_controller_device::bmdma_r), FUNC(bus_master_ide_controller_device::bmdma_w));
}

uint32_t idegdrom_device::ide_cs0_r(offs_t offset, uint32_t mem_mask)
{
	const int o = offset >> 2;
	const int r = (offset & 3) << 3;

	return m_ide->cs0_r(o, mem_mask << r) >> r;
}

uint32_t idegdrom_device::ide_cs1_r(offs_t offset, uint32_t mem_mask)
{
	const int o = offset >> 2;
	const int r = (offset & 3) << 3;

	return m_ide->cs1_r(o, mem_mask << r) >> r;
}

void idegdrom_device::ide_cs0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const int o = offset >> 2;
	const int r = (offset & 3) << 3;

	m_ide->cs0_w(o, data << r, mem_mask << r);
}

void idegdrom_device::ide_cs1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	const int o = offset >> 2;
	const int r = (offset & 3) << 3;

	m_ide->cs1_w(o, data << r, mem_mask << r);
}

// The board

static INPUT_PORTS_START(gdrom_board_ioports)
	PORT_START("DEBUG_ONLY")
	PORT_CONFNAME(0x01, 0x00, "Full emulation")
	PORT_CONFSETTING(0x01, "Enabled")
	PORT_CONFSETTING(0x00, "Disabled")
	PORT_CONFNAME(0x02, 0x02, "Initialized")
	PORT_CONFSETTING(0x02, "Yes")
	PORT_CONFSETTING(0x00, "No")
INPUT_PORTS_END

naomi_gdrom_board::naomi_gdrom_board(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	naomi_board(mconfig, NAOMI_GDROM_BOARD, tag, owner, clock),
	work_mode(0),
	m_maincpu(*this, "dimmcpu"),
	m_securitycpu(*this, "pic"),
	m_i2c0(*this, "i2c_0"),
	m_i2c1(*this, "i2c_1"),
	m_eeprom(*this, "eeprom"),
	m_315_6154(*this, "pci:00.0"),
	m_idegdrom(*this, "pci:01.0"),
	m_debug_dipswitches(*this, "DEBUG_ONLY"),
	picdata(*this, finder_base::DUMMY_TAG),
	dimm_command(0xffff),
	dimm_offsetl(0xffff),
	dimm_parameterl(0xffff),
	dimm_parameterh(0xffff),
	dimm_status(0xffff),
	dimm_control(0),
	sh4_unknown(0),
	dimm_des_key(0)
{
	image_tag = nullptr;
	picbus = 0;
	picbus_pullup = 0xf;
	picbus_io[0] = 0xf;
	picbus_io[1] = 0xf;
	picbus_used = false;
}

void naomi_gdrom_board::submap(address_map &map)
{
	naomi_board::submap(map);
	map(0x3c / 2, 0x3c / 2 + 1).rw(FUNC(naomi_gdrom_board::dimm_command_r), FUNC(naomi_gdrom_board::dimm_command_w));
	map(0x40 / 2, 0x40 / 2 + 1).rw(FUNC(naomi_gdrom_board::dimm_offsetl_r), FUNC(naomi_gdrom_board::dimm_offsetl_w));
	map(0x44 / 2, 0x44 / 2 + 1).rw(FUNC(naomi_gdrom_board::dimm_parameterl_r), FUNC(naomi_gdrom_board::dimm_parameterl_w));
	map(0x48 / 2, 0x48 / 2 + 1).rw(FUNC(naomi_gdrom_board::dimm_parameterh_r), FUNC(naomi_gdrom_board::dimm_parameterh_w));
	map(0x4c / 2, 0x4c / 2 + 1).rw(FUNC(naomi_gdrom_board::dimm_status_r), FUNC(naomi_gdrom_board::dimm_status_w));
}

void naomi_gdrom_board::sh4_map(address_map &map)
{
	map(0x00000000, 0x001fffff).mirror(0xa0000000).rom().region("bios", 0);
	map(0x04000000, 0x040000ff).rw(m_315_6154, FUNC(sega_315_6154_device::registers_r), FUNC(sega_315_6154_device::registers_w));
	map(0x0c000000, 0x0cffffff).ram();
	map(0x10000000, 0x103fffff).rw(FUNC(naomi_gdrom_board::shared_6154_sdram_r), FUNC(naomi_gdrom_board::shared_6154_sdram_w));
	map(0x14000000, 0x17ffffff).rw(m_315_6154, FUNC(sega_315_6154_device::aperture_r<0>), FUNC(sega_315_6154_device::aperture_w<0>));
	map(0x18000000, 0x1bffffff).rw(m_315_6154, FUNC(sega_315_6154_device::aperture_r<1>), FUNC(sega_315_6154_device::aperture_w<1>));
	map.unmap_value_high();
}

void naomi_gdrom_board::sh4_io_map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(naomi_gdrom_board::i2cmem_dimm_r), FUNC(naomi_gdrom_board::i2cmem_dimm_w));
}

void naomi_gdrom_board::pci_map(address_map &map)
{
	const char *t = tag();

	map(0x00000000, 0x00000003).rw(t, FUNC(naomi_gdrom_board::sh4_unknown_r), FUNC(naomi_gdrom_board::sh4_unknown_w));
	map(0x00000014, 0x00000017).rw(t, FUNC(naomi_gdrom_board::sh4_command_r), FUNC(naomi_gdrom_board::sh4_command_w));
	map(0x00000018, 0x0000001b).rw(t, FUNC(naomi_gdrom_board::sh4_offsetl_r), FUNC(naomi_gdrom_board::sh4_offsetl_w));
	map(0x0000001c, 0x0000001f).rw(t, FUNC(naomi_gdrom_board::sh4_parameterl_r), FUNC(naomi_gdrom_board::sh4_parameterl_w));
	map(0x00000020, 0x00000023).rw(t, FUNC(naomi_gdrom_board::sh4_parameterh_r), FUNC(naomi_gdrom_board::sh4_parameterh_w));
	map(0x00000024, 0x00000027).rw(t, FUNC(naomi_gdrom_board::sh4_status_r), FUNC(naomi_gdrom_board::sh4_status_w));
	map(0x00000028, 0x0000002b).rw(t, FUNC(naomi_gdrom_board::sh4_control_r), FUNC(naomi_gdrom_board::sh4_control_w));
	map(0x0000002c, 0x0000002f).rw(t, FUNC(naomi_gdrom_board::sh4_sdramconfig_r), FUNC(naomi_gdrom_board::sh4_sdramconfig_w));
	map(0x00000030, 0x00000033).rw(t, FUNC(naomi_gdrom_board::sh4_des_keyl_r), FUNC(naomi_gdrom_board::sh4_des_keyl_w));
	map(0x00000034, 0x00000037).rw(t, FUNC(naomi_gdrom_board::sh4_des_keyh_r), FUNC(naomi_gdrom_board::sh4_des_keyh_w));
	map(0x70000000, 0x70ffffff).rw(t, FUNC(naomi_gdrom_board::shared_sh4_sdram_r), FUNC(naomi_gdrom_board::shared_sh4_sdram_w));
	map(0x78000000, 0x783fffff).ram();
}

void naomi_gdrom_board::dimm_command_w(uint16_t data)
{
	dimm_command = data;
}

uint16_t naomi_gdrom_board::dimm_command_r()
{
	return dimm_command & 0xffff;
}

void naomi_gdrom_board::dimm_offsetl_w(uint16_t data)
{
	dimm_offsetl = data;
}

uint16_t naomi_gdrom_board::dimm_offsetl_r()
{
	return dimm_offsetl & 0xffff;
}

void naomi_gdrom_board::dimm_parameterl_w(uint16_t data)
{
	dimm_parameterl = data;
}

uint16_t naomi_gdrom_board::dimm_parameterl_r()
{
	return dimm_parameterl & 0xffff;
}

void naomi_gdrom_board::dimm_parameterh_w(uint16_t data)
{
	dimm_parameterh = data;
}

uint16_t naomi_gdrom_board::dimm_parameterh_r()
{
	return dimm_parameterh & 0xffff;
}

void naomi_gdrom_board::dimm_status_w(uint16_t data)
{
	dimm_status = data;
	if (dimm_status & 0x001)
		m_maincpu->set_input_line(SH4_IRL3, CLEAR_LINE);
	else
		m_maincpu->set_input_line(SH4_IRL3, ASSERT_LINE);
	if (dimm_status & 0x100)
		set_ext_irq(CLEAR_LINE);
	else
		set_ext_irq(ASSERT_LINE);
}

uint16_t naomi_gdrom_board::dimm_status_r()
{
	return dimm_status & 0xffff;
}

void naomi_gdrom_board::sh4_unknown_w(uint32_t data)
{
	sh4_unknown = data;
}

uint32_t naomi_gdrom_board::sh4_unknown_r()
{
	return sh4_unknown;
}

void naomi_gdrom_board::sh4_command_w(uint32_t data)
{
	dimm_command = data;
}

uint32_t naomi_gdrom_board::sh4_command_r()
{
	return dimm_command;
}

void naomi_gdrom_board::sh4_offsetl_w(uint32_t data)
{
	dimm_offsetl = data;
}

uint32_t naomi_gdrom_board::sh4_offsetl_r()
{
	return dimm_offsetl;
}

void naomi_gdrom_board::sh4_parameterl_w(uint32_t data)
{
	dimm_parameterl = data;
}

uint32_t naomi_gdrom_board::sh4_parameterl_r()
{
	return dimm_parameterl;
}

void naomi_gdrom_board::sh4_parameterh_w(uint32_t data)
{
	dimm_parameterh = data;
}

uint32_t naomi_gdrom_board::sh4_parameterh_r()
{
	return dimm_parameterh;
}

void naomi_gdrom_board::sh4_status_w(uint32_t data)
{
	dimm_status = data;
	if (dimm_status & 0x001)
		m_maincpu->set_input_line(SH4_IRL3, CLEAR_LINE);
	else
		m_maincpu->set_input_line(SH4_IRL3, ASSERT_LINE);
	if (dimm_status & 0x100)
		set_ext_irq(CLEAR_LINE);
	else
		set_ext_irq(ASSERT_LINE);
}

uint32_t naomi_gdrom_board::sh4_status_r()
{
	return dimm_status;
}

void naomi_gdrom_board::sh4_control_w(uint32_t data)
{
	uint32_t old = dimm_control;

	dimm_control = data;
	if (dimm_control & 2)
	{
		space_6154->unmap_readwrite(0x10000000, 0x10000000 + dimm_data_size - 1);
		logerror("Activated 'load mode register' command mode\n");
	}
	else
	{
		space_6154->install_ram(0x10000000, 0x10000000 + dimm_data_size - 1, dimm_des_data.get());
	}
	if (((old & 1) == 0) && ((dimm_control & 1) == 1))
		set_reset_out();
}

uint32_t naomi_gdrom_board::sh4_control_r()
{
	return dimm_control;
}

void naomi_gdrom_board::sh4_sdramconfig_w(uint32_t data)
{
	dimm_sdramconfig = data;
	logerror("Detected sdram dimm module size: %d megabytes\n", 4 * (1 << ((data >> 1) & 7)));
}

uint32_t naomi_gdrom_board::sh4_sdramconfig_r()
{
	return dimm_sdramconfig;
}

void naomi_gdrom_board::sh4_des_keyl_w(uint32_t data)
{
	dimm_des_key = (dimm_des_key & 0xffffffff00000000) | (uint64_t)data;
}

uint32_t naomi_gdrom_board::sh4_des_keyl_r()
{
	return (uint32_t)dimm_des_key;
}

void naomi_gdrom_board::sh4_des_keyh_w(uint32_t data)
{
	dimm_des_key = (dimm_des_key & 0xffffffff) | ((uint64_t)data << 32);
}

uint32_t naomi_gdrom_board::sh4_des_keyh_r()
{
	return (uint32_t)(dimm_des_key >> 32);
}

uint64_t naomi_gdrom_board::shared_6154_sdram_r(offs_t offset)
{
	return space_6154->read_qword(0x78000000 + (offset << 3));
}

void naomi_gdrom_board::shared_6154_sdram_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	space_6154->write_qword(0x78000000 + (offset << 3), data, mem_mask);
}

uint32_t naomi_gdrom_board::shared_sh4_sdram_r(offs_t offset)
{
	return space_sh4->read_dword(0x0c000000 + (offset << 2));
}

void naomi_gdrom_board::shared_sh4_sdram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	space_sh4->write_dword(0x0c000000 + (offset << 2), data, mem_mask);
}

uint64_t naomi_gdrom_board::i2cmem_dimm_r()
{
	uint8_t ret;

	ret = m_i2c0->read_sda();
	ret |= m_i2c1->read_sda();
	ret = ret << 1;
	if (picbus_used == true)
		ret |= ((picbus | picbus_pullup) & 0xf) << 2;
	else
		ret |= m_eeprom->do_read() << 5;
	return ret;
}

void naomi_gdrom_board::i2cmem_dimm_w(uint64_t data)
{
	if (data & 0x40000)
	{
		m_i2c0->write_sda((data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
		m_i2c1->write_sda((data & 0x2) ? ASSERT_LINE : CLEAR_LINE);
	}
	m_i2c0->write_scl((data & 0x1) ? ASSERT_LINE : CLEAR_LINE);
	m_i2c1->write_scl((data & 0x1) ? ASSERT_LINE : CLEAR_LINE);
	if (data & 0x0200)
	{
		picbus_used = true;
		picbus_io[0] = (uint8_t)(~data >> (16 + 5 * 2 - 3)) & 0x8; // clock only for now
		picbus = (data >> 2) & 0xf;
		picbus_pullup = (picbus_io[0] & picbus_io[1]) & 0xf; // high if both are inputs
		m_maincpu->abort_timeslice();
		machine().scheduler().perfect_quantum(attotime::from_msec(1));
	}
	else
	{
		picbus_used = false;
		m_eeprom->di_write((data & 0x4) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->cs_write((data & 0x10) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write((data & 0x8) ? ASSERT_LINE : CLEAR_LINE);
	}
}

uint8_t naomi_gdrom_board::pic_dimm_r()
{
	return picbus | picbus_pullup;
}

void naomi_gdrom_board::pic_dimm_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	picbus = data;
	m_securitycpu->abort_timeslice();

	picbus_io[1] = ~mem_mask; // for each bit specify direction, 0 out 1 in
	picbus_pullup = (picbus_io[0] & picbus_io[1]) & 0xf; // high if both are inputs
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
			file_start = get_u32le(&dir_sector[pos+2]);
			file_size =  get_u32le(&dir_sector[pos+10]);

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
	dimm_des_data = nullptr;
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

			// set data for security pic rom
			uint8_t *picrom = static_cast<uint8_t*>(m_securitycpu->memregion(DEVICE_SELF)->base());
			for(offs_t b=0;b<0x1000;b++)
				picrom[BYTE_XOR_LE(b)] = picdata[b];
		} else {
			// use extracted pic data
			// printf("This PIC key hasn't been converted to a proper PIC binary yet!\n");
			memcpy(name, &picdata[33], 7);
			memcpy(name+7, &picdata[25], 7);

			key = ((uint64_t(picdata[0x31]) << 56) |
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
		cdrom_file *gdromfile = new cdrom_file(machine().rom_load().get_disk_handle(image_tag));
		// primary volume descriptor
		// read frame 0xb06e (frame=sector+150)
		// dimm board firmware starts straight from this frame
		gdromfile->read_data((netpic ? 0 : 45000) + 16, buffer, cdrom_file::CD_TRACK_MODE1);
		uint32_t path_table = get_u32le(&buffer[0x8c]);
		// path table
		gdromfile->read_data(path_table, buffer, cdrom_file::CD_TRACK_MODE1);

		// directory
		uint8_t dir_sector[2048]{};
		// find data of file
		uint32_t file_start = 0, file_size = 0;

		if (netpic == 0) {
			uint32_t dir = get_u32le(&buffer[0x2]);

			gdromfile->read_data(dir, dir_sector, cdrom_file::CD_TRACK_MODE1);
			find_file(name, dir_sector, file_start, file_size);

			if (file_start && (file_size == 0x100)) {
				// read file
				gdromfile->read_data(file_start, buffer, cdrom_file::CD_TRACK_MODE1);
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
					uint32_t dir = get_u32le(&buffer[i + 2]);
					memcpy(name, "ROM.BIN", 7);
					gdromfile->read_data(dir, dir_sector, cdrom_file::CD_TRACK_MODE1);
					break;
				}
				i += buffer[i] + 8 + (buffer[i] & 1);
			}
		}

		find_file(name, dir_sector, file_start, file_size);

		if (file_start) {
			uint32_t file_rounded_size = (file_size + 2047) & -2048;
			for (dimm_data_size = 4096; dimm_data_size < file_rounded_size; dimm_data_size <<= 1);
			dimm_data = std::make_unique<uint8_t[]>(dimm_data_size);
			dimm_des_data = std::make_unique<uint8_t[]>(dimm_data_size);
			if (dimm_data_size != file_rounded_size)
				memset(&dimm_data[file_rounded_size], 0, dimm_data_size - file_rounded_size);

			// read encrypted data into dimm_des_data
			uint32_t sectors = file_rounded_size / 2048;
			for (uint32_t sec = 0; sec != sectors; sec++)
				gdromfile->read_data(file_start + sec, &dimm_des_data[2048 * sec], cdrom_file::CD_TRACK_MODE1);

			uint32_t des_subkeys[32];
			des_generate_subkeys(swapendian_int64(key), des_subkeys);

			// decrypt read data from dimm_des_data to dimm_data
			for (int i = 0; i < file_rounded_size; i += 8)
				put_u64le(&dimm_data[i], des_encrypt_decrypt(true, get_u64le(&dimm_des_data[i]), des_subkeys));
		}

		delete gdromfile;

		if(!dimm_data)
			throw emu_fatalerror("GDROM: Could not find the file to decrypt.");
	}

	space_sh4 = &m_maincpu->space(AS_PROGRAM);
	space_6154 = &m_315_6154->space(sega_315_6154_device::AS_PCI_MEMORY);

	save_item(NAME(dimm_cur_address));
	save_item(NAME(picbus));
	save_item(NAME(picbus_pullup));
	save_item(NAME(picbus_io));
	save_item(NAME(picbus_used));
	save_item(NAME(dimm_command));
	save_item(NAME(dimm_offsetl));
	save_item(NAME(dimm_parameterl));
	save_item(NAME(dimm_parameterh));
	save_item(NAME(dimm_status));
	save_item(NAME(dimm_control));
	save_item(NAME(sh4_unknown));
	save_item(NAME(dimm_des_key));
}

void naomi_gdrom_board::device_reset()
{
	int dips = m_debug_dipswitches->read();

	naomi_board::device_reset();

	if (dips & 1)
	{
		if (dips & 2)
			work_mode = 1; // real emulation, dimm ram contains valid game data
		else
			work_mode = 2; // real emulation, dimm ram not initialized
	}
	else
		work_mode = 0; // default cartridge-like mode
	logerror("Work mode is %d\n", work_mode);
	if (work_mode != 0)
	{
		dimm_command = 0;
		dimm_offsetl = 0;
		dimm_parameterl = 0;
		dimm_parameterh = 0;
		space_6154->install_ram(0x10000000, 0x10000000 + dimm_data_size - 1, dimm_des_data.get());
		if (work_mode == 2) // invalidate dimm memory contents by setting the first 2048 bytes to 0
			memset(dimm_des_data.get(), 0, 2048);
	}
	else
	{
		m_maincpu->set_disable();
		m_securitycpu->set_disable();
		space_6154->unmap_readwrite(0x10000000, 0x10000000 + dimm_data_size - 1);
	}

	dimm_cur_address = 0;
}

ioport_constructor naomi_gdrom_board::device_input_ports() const
{
	return INPUT_PORTS_NAME(gdrom_board_ioports);
}

void naomi_gdrom_board::board_setup_address(uint32_t address, bool is_dma)
{
	dimm_cur_address = address & (dimm_data_size - 1);
}

void naomi_gdrom_board::board_get_buffer(uint8_t *&base, uint32_t &limit)
{
	base = &dimm_data[dimm_cur_address];
	limit = dimm_data_size - dimm_cur_address;
}

void naomi_gdrom_board::board_advance(uint32_t size)
{
	dimm_cur_address += size;
	if(dimm_cur_address >= dimm_data_size)
		dimm_cur_address %= dimm_data_size;
}

#define CPU_CLOCK 200000000 // need to set the correct value here
#define PIC_CLOCK 20000000  // and here

void naomi_gdrom_board::device_add_mconfig(machine_config &config)
{
	SH4LE(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &naomi_gdrom_board::sh4_map);
	m_maincpu->set_addrmap(AS_IO, &naomi_gdrom_board::sh4_io_map);

	PCI_ROOT(config, "pci", 0);
	SEGA315_6154(config, m_315_6154, 0);
	m_315_6154->set_addrmap(sega_315_6154_device::AS_PCI_MEMORY, &naomi_gdrom_board::pci_map);
	IDE_GDROM(config, m_idegdrom, 0, image_tag, m_315_6154->tag(), sega_315_6154_device::AS_PCI_MEMORY);
	m_idegdrom->irq_callback().set_inputline(m_maincpu, SH4_IRL2);
	PIC16C622(config, m_securitycpu, PIC_CLOCK);
	m_securitycpu->read_b().set(FUNC(naomi_gdrom_board::pic_dimm_r));
	m_securitycpu->write_b().set(FUNC(naomi_gdrom_board::pic_dimm_w));
	m_securitycpu->set_config(0x3fff - 0x04);
	I2C_24C01(config, m_i2c0, 0);
	m_i2c0->set_e0(0);
	m_i2c0->set_wc(1);
	I2C_24C01(config, m_i2c1, 0);
	m_i2c1->set_e0(1);
	m_i2c1->set_wc(1);
	EEPROM_93C46_8BIT(config, m_eeprom, 0);
}

// DIMM firmwares:
//  FPR-23489C - 1.02 not VxWorks based, no network, can not be software updated to 2.xx+
// Net-DIMM firmwares:
// all VxWorkx based, can be updated up to 4.0x, actually 1MB in size, must have CRC32 FFFFFFFF, 1st MB of flash ROM contain stock version, 2nd MB have some updated version
//  ?          - 2.03 factory only, introduced ALL.net features, so far was seen only as stock firmware in 1st half of flash ROM, factory updated to some newer ver in 2nd ROM half
//  FPR23718   - 2.06 factory only, most common version of NAOMI Net-DIMMs, have stock 2.03, IC label need verification
//  ?            2.13 factory or update (NAOMI VF4)
//  ?            2.17 factory or update (NAOMI VF4 Evolution)
//  FPR23905C  - 3.01 factory, added network boot support, supports Triforce and Chihiro
//  FPR23905   - 3.03 factory or update (NAOMI WCCF)
//  ?            3.12 factory only
//  ?            3.17 latest known 3.xx version, factory or update (NAOMI VF4 Final Tuned or statndalone disks for Chihiro and Triforce)
// update only - 4.01 supports Compact Flash GD-ROM-replacement
//              "4.02" hack of 4.01 with CF card vendor check disabled

ROM_START( dimm )
	ROM_REGION( 0x200000, "segadimm", 0)
	// Altera FLEX EPF10K30 firmwares (implements PCI IDE controller)
	ROM_LOAD("315-6301.ic11", 0x000000, 0x01ff5b, CRC(cc7735c7) SHA1(1afb442b5918c0d60f98688ed0a7117b0d068722) ) // GD-only DIMM
	ROM_LOAD("315-6334.ic11", 0x000000, 0x01ff01, CRC(534c342d) SHA1(3e879f432c82305487922ab28c07107cf0f3c5cf) ) // Net-DIMM

	// unused and/or unknown security PICs
	// 253-5508-0352E 317-0352-EXP BFC.BIN, probably Sega Yonin Uchi Mahjong MJ (Export)
	ROM_LOAD("317-0352-exp.pic", 0x00, 0x4000, CRC(b216fbfc) SHA1(da2341003b35d1600d63fbe34d13ff3b42bdc939) )
	// 253-5508-0422J 317-0422-JPN BHE.BIN Quest of D undumped version, high likely 2.0x "Gofu no Keisyousya"
	ROM_LOAD("317-0422-jpn.pic", 0x00, 0x4000, CRC(54197fbf) SHA1(a18b5b7aec0498c7a62cacf9f2298ddefb7482c9) )
	// Sangokushi Taisen 2 satellite firmware update (CDV-10023) key, .BIN file name is unknown/incorrect.
	ROM_LOAD("317-unknown.pic",  0x00, 0x4000, CRC(7dc07733) SHA1(b223dc44718fa71e7b420c3b44ce4ab961445461) )

	// main firmwares
	ROM_REGION(0x200000, "bios", ROMREGION_64BIT)
	ROM_SYSTEM_BIOS(0, "fpr-23489c.ic14", "BIOS 0")
	ROMX_LOAD( "fpr-23489c.ic14", 0x000000, 0x200000, CRC(bc38bea1) SHA1(b36fcc6902f397d9749e9d02de1bbb7a5e29d468), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "203_203.bin", "BIOS 1")
	ROMX_LOAD( "203_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(6f55f1ae0606816a4eca6645ed36eb7f9c7ad9cf), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "fpr23718.ic36", "BIOS 2")
	ROMX_LOAD( "fpr23718.ic36",   0x000000, 0x200000, CRC(a738ea1c) SHA1(b7b5a55a6a4cf0aa2df1b3dff62ff67f864c55e8), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "213_203.bin", "BIOS 3")
	ROMX_LOAD( "213_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(17131f318632610b87bc095156ffad4597fed4ca), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "217_203.bin", "BIOS 4")
	ROMX_LOAD( "217_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(e5a229ae7ed48b2955cad63529fd938c6db555e5), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "3.01", "BIOS 5")
	ROMX_LOAD( "fpr23905c.ic36",  0x000000, 0x200000, CRC(ffffffff) SHA1(972b3b73aa1eabb1091e9096b57a7e5e1d0436d8), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "3.03", "BIOS 6")
	ROMX_LOAD( "fpr23905.ic36",   0x000000, 0x200000, CRC(ffffffff) SHA1(acade4362807c7571b1c2a48ed6067e4bddd404b), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "317_312.bin", "BIOS 7")
	ROMX_LOAD( "317_312.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(31d698cd659446ee09a2eeedec6e4bc6a19d05e8), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "401_203.bin", "BIOS 8")
	ROMX_LOAD( "401_203.bin",     0x000000, 0x200000, CRC(a738ea1c) SHA1(edb52597108462bcea8eb2a47c19e51e5fb60638), ROM_BIOS(8))

	// dynamically filled with data
	ROM_REGION(0x1000, "pic", ROMREGION_ERASE00)
	ROM_REGION(0x80, "i2c_0", ROMREGION_ERASE00)
	ROM_LOAD("dimmspd.bin", 0x00, 0x80, CRC(45dac6d7) SHA1(4548675f8d31348fa6828d5b4f247af1f072b62d))
	ROM_REGION(0x80, "i2c_1", ROMREGION_ERASE00)
	ROM_LOAD("dimmspd.bin", 0x00, 0x80, CRC(45dac6d7) SHA1(4548675f8d31348fa6828d5b4f247af1f072b62d))
	ROM_REGION(0x80, "eeprom", ROMREGION_ERASE00)
	ROM_LOAD("93c46.bin", 0x00, 0x80, CRC(daafbccd) SHA1(1e39983779a62ebc6801ec6f2a5138717a7a5259))
ROM_END

const tiny_rom_entry *naomi_gdrom_board::device_rom_region() const
{
	return ROM_NAME(dimm);
}
