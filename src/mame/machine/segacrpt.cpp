// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/******************************************************************************

  Sega encryption emulation                                 by Nicola Salmoria


  Several Sega Z80 games have program ROMs encrypted using a common algorithm
  (but with a different key).
  The hardware used to implement this encryption is either a custom CPU, or an
  epoxy block which probably contains a standard Z80 + PALs.

  The encryption affects D3, D5, and D7, and depends on M1, A0, A4, A8 and A12.
  D0, D1, D3, D4 and D6 are always unaffected.

  The encryption consists of a permutation of the three bits, which can also be
  inverted. Therefore there are 3! * 2^3 = 48 different possible encryptions.

  For simplicity, the decryption is implemented using conversion tables.
  We need 32 of these tables, one for every possible combination of M1, A0, A4,
  A8 and A12. However, all the games currently known are full of repetitions
  and only use 6 different tables, the only exceptions being Pengo, Yamato and
  Spatter which have 7 (but one of them is the identity: { 0x00, 0x08, 0x20, 0x28 } ).
  This is most likely a limitation of the hardware.
  Some of the early games are even weaker: of the 6 different tables, they use
  3 for opcodes and 3 for data, and always coupled in the same way.

  In all games currently known, only bytes in the memory range 0x0000-0x7fff
  (A15 = 0) are encrypted. My guess is that this was done to allow games to
  copy code to RAM (in the memory range 0x8000-0xffff) and execute it from
  there without the CPU trying to decrypt it and messing everything up.
  However Zaxxon has RAM at 0x6000, and the CPU doesn't seem to interfere with
  it; but it doesn't execute code from there, so it's possible that the CPU is
  encrypting the data while writing it and decrypting it while reading (that
  would seem kind of strange though). Video and sprite RAM and memory mapped
  ports are all placed above 0x8000.

  Given its strict limitations, this encryption is reasonably easy to break,
  and very vulnerable to known plaintext attacks.



  Ninja Princess:

  there is a (bootleg?) board which has a standard Z80 + 2 bipolar PROMs
  instead of the custom CPU. The encryption table is different from the
  original Ninja Princess; it is actually the same as Flicky.

  The first PROM is 32x8 and contains the number (0..5) of the table to
  use depending on M1, A0, A4, A8, A12:

  00: 11 00 33 22 00 44 44 00 11 33 33 22 44 44 44 22
  10: 11 55 55 33 44 22 55 22 11 33 55 33 44 44 11 22

  The second PROM is 256x4 and contains the 6 different XOR tables:

       A  D  B  C  C  B  D  A
  00: 09 09 0A 0A 0A 0A 09 09
  08: 0E 08 0B 0D 0D 0B 08 0E
  10: 0A 0C 0A 0C 0C 0A 0C 0A
  18: 0B 0E 0E 0B 0B 0E 0E 0B
  20: 0C 0C 0F 0F 0F 0F 0C 0C
  28: 08 0D 0B 0E 0E 0B 0D 08
  [the remaining bytes are all 0F]
  bit 3 is not used.
  bits 0-2 is the XOR code inverted (0 = 0xa8, 1 = 0xa0 ... 6 = 0x08 7 = 0x00)

  Here is a diagram showing how it works:

  data to                             XOR
  decode                             value
                      A ---
  D7  --------------- 0|   |
  D3  --------------- 1|   |
  D5  --------------- 2| P |D
          A --- D      | R |0 ---|>--- D3
  M1  --- 0| P |0 --- 3| O |1 ---|>--- D5
  A0  --- 1| R |1 --- 4| M |2 ---|>--- D7
  A4  --- 2| O |2 --- 5| 2 |3 ---
  A8  --- 3| M |3 --- 6|   |
  A12 --- 4| 1 |4 --- 7|   |
            ---         ---


  My Hero:

  the bootleg does the decryption using a single 256x4 PROM, mapped in the
  obvious way:

  data to            XOR
  decode            value
          A ---
  D3  --- 0|   |
  D5  --- 1|   |D
  D7  --- 2| P |0 --- D3
  A0  --- 3| R |1 --- D5
  A4  --- 4| O |2 --- D7
  A8  --- 5| M |3 ---
  A12 --- 6|   |
  M1  --- 7|   |
            ---



  List of encrypted games currently known:

 CPU Part #         Game                   Comments
  315-5010      Pengo                   unencrypted version available
  315-5013      Super Zaxxon            used Zaxxon for known plaintext attack
  315-5014      Buck Rogers / Zoom 909  unencrypted version available
  315-5015      Super Locomotive
  315-5018      Yamato
  ???-????      Top Roller              same key as Yamato
  315-5028      Sindbad Mystery
  315-5030      Up'n Down &             unencrypted version available
  ???-???? M120 Razzmatazz
  315-5033      Regulus                 unencrypted version available
  315-5041 M140 Mister Viking
  315-5048      SWAT                    used Bull Fight for k.p.a.
  315-5051      Flicky &
                Ninja Princess (bootleg)
  315-5061      Future Spy
  315-5064      Water Match             used Mister Viking for k.p.a.
  315-5065      Bull Fight
  315-5069      Star Force              game by Tehkan; same key as Super Locomotive
  ???-????      Pinball Action          game by Tehkan; also has a simple bitswap on top
  ???-????      Spatter
  315-5084      Jongkyo                 TABLE INCOMPLETE game by Kiwako; also has a simple bitswap on top
  315-5093      Pitfall II
  315-5098      Ninja Princess          unencrypted version available; same key as Up'n Down
  315-5102      Sega Ninja              unencrypted version available
  315-5110      I'm Sorry               used My Hero for k.p.a.
  315-5114      ?? pcb 834-5492         same key as Regulus
  315-5115      TeddyBoy Blues
  315-5132      My Hero
  315-5135      Heavy Metal &
                Wonder Boy (set 1a & 3; bootlegs?)
  ???-????      Lovely Cards


  Some text found in the ROMs:

  Buck Rogers      SECULITY BY MASATOSHI,MIZUNAGA
  Super Locomotive SEGA FUKUMURA MIZUNAGA
  Yamato           SECULITY BY M,MIZUNAGA
  Regulus          SECULITY BY SYUICHI,KATAGI
  Up'n Down        19/SEP 1983   MASATOSHI,MIZUNAGA
  Mister Viking    SECURITY BY S.KATAGI  CONTROL CHIP M140
  SWAT             SECURITY BY S.KATAGI
  Flicky           SECURITY BY S.KATAGI
  Water Match      PROGRAMED BY KAWAHARA&NAKAGAWA
  Star Force       STAR FORCE TEHKAN. SECURITY BY SEGA ENTERPRISESE

******************************************************************************/

#include "emu.h"
#include "segacrpt.h"


#if 0
static void lfkp(int mask)
{
	int A;
	UINT8 *RAM = machine.root_device().memregion("maincpu")->base();


	for (A = 0x0000;A < 0x8000-14;A++)
	{
		static const char text[] = "INSERT COIN";
		int i;


		if (    (RAM[A+0] & mask) == (0x21 & mask) &&   /* LD HL,$xxxx */
				(RAM[A+3] & mask) == (0x11 & mask) &&   /* LD DE,$xxxx */
				(RAM[A+6] & mask) == (0x01 & mask))     /* LD BC,$xxxx */
		{
			if (    (RAM[A+ 9] & mask) == (0x36 & mask) &&  /* LD (HL),$xx */
					(RAM[A+11] & mask) == (0xed & mask) &&
					(RAM[A+12] & mask) == (0xb0 & mask))    /* LDIR */
				logerror("%04x: hl de bc (hl),xx ldir\n",A);

			if (    (RAM[A+ 9] & mask) == (0x77 & mask) &&  /* LD (HL),A */
					(RAM[A+10] & mask) == (0xed & mask) &&
					(RAM[A+11] & mask) == (0xb0 & mask))    /* LDIR */
				logerror("%04x: hl de bc (hl),a ldir\n",A);

			if (    (RAM[A+ 9] & mask) == (0xed & mask) &&
					(RAM[A+10] & mask) == (0xb0 & mask))    /* LDIR */
				logerror("%04x: hl de bc ldir\n",A);
		}

		/* the following can also be PUSH IX, PUSH IY - need better checking */
		if (    (RAM[A+0] & mask) == (0xf5 & mask) &&   /* PUSH AF */
				(RAM[A+1] & mask) == (0xc5 & mask) &&   /* PUSH BC */
				(RAM[A+2] & mask) == (0xd5 & mask) &&   /* PUSH DE */
				(RAM[A+3] & mask) == (0xe5 & mask))     /* PUSH HL */
			logerror("%04x: push af bc de hl\n",A);

		if (    (RAM[A+0] & mask) == (0xe1 & mask) &&   /* POP HL */
				(RAM[A+1] & mask) == (0xd1 & mask) &&   /* POP DE */
				(RAM[A+2] & mask) == (0xc1 & mask) &&   /* POP BC */
				(RAM[A+3] & mask) == (0xf1 & mask))     /* POP AF */
			logerror("%04x: pop hl de bc af\n",A);

		for (i = 0;i < strlen(text);i++)
			if ((RAM[A+i] & mask) != (text[i] & mask)) break;
		if (i == strlen(text))
			logerror("%04x: INSERT COIN\n",A);
	}
}

static void look_for_known_plaintext(void)
{
	lfkp(0x57);
}
#endif

void sega_decode(UINT8 *data, UINT8 *opcodes, int size, const UINT8 convtable[32][4], int bank_count, int bank_size)
{
	for (int A = 0x0000;A < size + bank_count*bank_size;A++)
	{
		int xorval = 0;

		UINT8 src = data[A];
		int adr;
		if(A < size || !bank_count)
			adr = A;
		else
			adr = size + ((A - size) % bank_size);

		/* pick the translation table from bits 0, 4, 8 and 12 of the address */
		int row = (adr & 1) + (((adr >> 4) & 1) << 1) + (((adr >> 8) & 1) << 2) + (((adr >> 12) & 1) << 3);

		/* pick the offset in the table from bits 3 and 5 of the source data */
		int col = ((src >> 3) & 1) + (((src >> 5) & 1) << 1);
		/* the bottom half of the translation table is the mirror image of the top */
		if (src & 0x80)
		{
			col = 3 - col;
			xorval = 0xa8;
		}

		/* decode the opcodes */
		opcodes[A] = (src & ~0xa8) | (convtable[2*row][col] ^ xorval);

		/* decode the data */
		data[A] = (src & ~0xa8) | (convtable[2*row+1][col] ^ xorval);

		if (convtable[2*row][col] == 0xff)  /* table incomplete! (for development) */
			opcodes[A] = 0xee;
		if (convtable[2*row+1][col] == 0xff)    /* table incomplete! (for development) */
			data[A] = 0xee;
	}
}
