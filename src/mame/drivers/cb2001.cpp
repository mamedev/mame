// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Zandona'
/*************************************************************************************************

  Cherry Bonus 2001  (c)2000/2001 Dyna


Produttore  Dyna
N.revisione
CPU

1x DYNA CPU91A-011-0016JK004 (QFP84) custom
1x DYNA DC3001-0051A (QFP128) custom
1x DYNA 22A078803 (DIP42) (I think it's an I/O)
1x WINBOND WF19054 (equivalent to AY-3-8910)
1x oscillator 24.000MHz

ROMs

1x M27C4002 (12a)
1x M27C1001 (11f)
2x AM27S29PC (9b,11b)
2x GAL16V8D (not dumped)


Note

1x 36x2 edge connector
1x 10x2 edge connector
1x trimmer (volume)
1x pushbutton (sw8)
1x battery
7x 8x switches dip (sw1-7)
------------------------------

In title screen (c) is 2001
In test mode (c) is 2000

------------------------------

this seems more like 8-bit hardware, maybe it should be v25, not v35...

*************************************************************************************************/

#include "emu.h"
#include "cpu/nec/v25.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"


class cb2001_state : public driver_device
{
public:
	cb2001_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_fg(*this, "vrafg"),
		m_vram_bg(*this, "vrabg"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT16> m_vram_fg;
	required_shared_ptr<UINT16> m_vram_bg;
	int m_videobank;
	int m_videomode;
	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;
	int m_other1;
	int m_other2;
	DECLARE_WRITE16_MEMBER(cb2001_vidctrl_w);
	DECLARE_WRITE16_MEMBER(cb2001_vidctrl2_w);
	DECLARE_WRITE16_MEMBER(cb2001_bg_w);
	TILE_GET_INFO_MEMBER(get_cb2001_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_cb2001_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_cb2001_reel3_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(cb2001);
	UINT32 screen_update_cb2001(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


#define xxxx 0x90 /* Unknown */

static const UINT8 cb2001_decryption_table[256] = {
	0xe8,xxxx,xxxx,xxxx,0x80,0xe4,0x12,0x2f, 0x3c,xxxx,xxxx,0x23,xxxx,xxxx,xxxx,0x5f, /* 00 */
//    ssss                ---- **** pppp pppp  ssss           pppp                pppp
	0x86,xxxx,xxxx,0x27,0x1c,xxxx,xxxx,xxxx, 0x32,0x40,0xa0,0xd3,0x3a,0x14,0x89,0x1f, /* 10 */
//    rrrr           **** pppp                 pppp pppp pppp pppp ppp? pppp pppp ssss
	xxxx,0x8e,xxxx,0x0f,xxxx,0x49,0xb5,xxxx, 0x56,xxxx,xxxx,0x75,0x33,0xb6,xxxx,0x39, /* 20 */
//         ssss      ssss      pppp pppp       pppp           ssss pppp pppp      ****
	0x89,xxxx,xxxx,xxxx,xxxx,0x22,0x5b,xxxx, xxxx,xxxx,0x74,xxxx,xxxx,0xa6,xxxx,0x74, /* 30 */
//    wwww                     **** pppp                 debu           pppp      ssss
	xxxx,0xea,xxxx,xxxx,0xd0,0xb0,0x5e,xxxx, xxxx,0xa2,xxxx,xxxx,0xa3,xxxx,xxxx,0xb3, /* 40 */
//         ssss           **** pppp pppp            pppp           ssss           pppp
	0x13,xxxx,0x2c,xxxx,0x9d,xxxx,0x42,0xc0, 0x04,xxxx,0xb7,xxxx,0xeb,0xab,xxxx,xxxx, /* 50 */
//    ????      ssss      ****      pppp pppp  ****      ****      ssss pppp
	xxxx,xxxx,xxxx,xxxx,0x0a,xxxx,xxxx,xxxx, 0xa1,0xa5,xxxx,xxxx,xxxx,0xbb,0xba,xxxx, /* 60 */
//                        pppp                 ssss pppp                pppp ssss
	0xc3,0x53,0x02,0x58,xxxx,xxxx,0x24,xxxx, 0x72,xxxx,0xf3,xxxx,xxxx,0x43,xxxx,0x34, /* 70 */
//    ssss pppp pppp ssss           pppp       ssss      pppp           ssss      ****
	0x26,xxxx,0xd1,xxxx,xxxx,0x3d,0xfb,0xf6, xxxx,xxxx,0x59,xxxx,0x73,xxxx,0x2a,xxxx, /* 80 */
//    pppp      rrrr           pppp **** ssss            pppp      ssss      pppp
	xxxx,0x3d,0xe9,xxxx,xxxx,0xbe,0xf9,xxxx, xxxx,xxxx,0x57,xxxx,0xb9,xxxx,0xbf,xxxx, /* 90 */
//         wwww pppp           pppp ****                 pppp      ssss      ssss
	0xc1,xxxx,0xe6,0x06,0xaa,0x9c,0xad,0xb8, 0x4e,xxxx,0x8d,0x50,0x51,0xa4,xxxx,0x1a, /* A0 */
//    ****      pppp ssss pppp **** pppp ssss  pppp      ssss ssss pppp pppp      pppp
	0xac,xxxx,0xb4,xxxx,xxxx,0x83,xxxx,xxxx, xxxx,0x05,0x03,xxxx,0x1e,0x43,0x07,0xcf, /* B0 */
//    pppp      ssss           pppp                 pppp pppp      ssss **** ssss ssss
	0xcb,0xec,0xee,xxxx,xxxx,0xe2,0x87,xxxx, xxxx,xxxx,0x76,0x61,0x48,xxxx,0x2e,xxxx, /* C0 */
//    ssss ssss pppp           ssss pppp                 pppp **** ****      pppp
	xxxx,0xf2,0x46,xxxx,0x60,xxxx,0x4f,0x47, 0x88,xxxx,xxxx,0xff,xxxx,0xfa,0xc7,0x8b, /* D0 */
//         pppp pppp      ****      pppp pppp  pppp           ssss      **** ssss pppp
	0x8a,0xb1,xxxx,0xc6,xxxx,0x5a,xxxx,0xb2, 0x9a,0x52,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* E0 */
//    ssss gggg      ssss      ****      pppp  pppp ****
	xxxx,0xae,0xfe,xxxx,xxxx,xxxx,xxxx,0x3a, xxxx,xxxx,0x34,xxxx,0x81,xxxx,xxxx,xxxx, /* F0 */
//         pppp ssss                     ppp?            wwww      pppp
};

/* robiza's notes:

f7 -> 3a very probable, but 1c -> 3a
d1 f1 -> same effect of "z80 cpir" opcode (repne cmpmb) d1 -> f2, f1 -> ae (cmast91 from 2682, cb2001 from e2156)

e0022 a5         push psw ?
e0023 d4         push r ?
e0024 bc         push ds0                 (bc -> 1e)
e0025 a3         push ds1                 (a3 -> 06)
e0026 dd         di ?
e0027 a7 00 00   mov aw,0
e002a 21 d8      mov ds0,aw
e002c 21 c0      mov ds1,aw
e002e 18 c0      xor al,al
e0030 49 67 07   mov [767],al
e0033 45 01      mov al,1
e0035 49 d3 06   mov [6d3],al
...
e00a6 be         pop ds1                  (be -> 07)
e00a7 1f         pop ds0                  (1f -> 1f)
e00a8 05 30      in al, 30 ?
e00aa cb         pop r ?
e00ab 54         pop psw ?
e00ac 23 92      fint                     (23 -> 0f)
e00ae bf         reti

cmast91 and cmv4 seems similar to this cb2001:

cmast91:                                  cb2001:

0089 call 2a9d                            e0038 call e30a2
  2a9d ld hl,d0b3                           e30a2 lea bw,[04a6]
  2aa0 inc (hl)                             e30a5 inc b ptr[bw]
  2aa1 inc hl                               e30a8 inc bw
  2aa2 inc (hl)                             e30a9 inc b ptr[bw]
  2aa3 ld a,(hl)                            e30ab mov al,b ptr[bw]
  2aa4 cp 3c                                e30ad cmp al,3c
  2aa6 ret c                                e30af bc e30b7

0c4f ld a,(d44d)                            e003b mov al,[512]
0c52 or a,a                                 e003e and al,al
0c53 ret nz                                 e0040 be e0083
0c54 ld hl,d461                             .
0c57 ld a,(d476)                            e0042 mov aw,[4b9]              (68 -> a1)
0c5a or a                                   e0045 and aw,aw
0c5b jr nz,0c6d                             e0047 be e0083
0c5d ld a,(hl)                              .
0c5e or a                                   e0049 and al,al
0c5f jr nz,0c71                             e004b be e0054
0c61 inc hl                                 e004d dec al
0c62 inc (hl)                               e004f mov [4b9],aw              (4c -> a3)
0c63 ld a,(hl)                              .
.                                           e0052 br e0083
.                                           e0054 and ah,ah
.                                           e0056 be e007b
0c64 cp 3c                                  e0058 mov al,3c
0c66 jr c,c71                               .
0c68 dec hl                                 e005a dec ah
0c69 ld (hl),ff                             e005c mov [4b9],aw
0c6b jr c71                                 .
0c6d xor a                                  .
0c6e ld (hl),a                              .
0c6f inc (hl)                               .
0c70 ld (hl),a                              .
0c71 ld a,(d463)                            e005f mov aw,[4bb]
0c74 or a                                   e0062 and aw,aw
0c75 ret nz                                 e0064 bne e006b
.                                           e0066 mov aw,1
.                                           e0069 br e006e

0067 ld a,$07                 e0130 mov al,7h
0069 out ($23),a              e0132 mov dw,23h
                              e0135 out dw,al

006b ld a,$3f                 e0136 mov al,3fh
006d out ($22),a              e0138 mov dw,22h
                              e013b out dw,al

006f ld a,$9b                 e0124 mov al,0ffh
0071 out ($13),a              e0126 mov dw,13h
                              e0129 out dw,al

-----------------

cmv4                          cb2001                     (en -> de)

.                             e025f mov dw,2h
02b2 in a,($06)               e0262 in al,dw             (c1 -> ec)
.                             e0263 xor al,d0h
.                             e0265 and l,c0h
02b4 bit 6,a                  e0267 test1 al,6h          (23 -> 0f)
02b6 jp z,$41e2               e026b be e0270
.                             e026d br e39fe
02b9 bit 7,a                  e0270 test1 al,7h
02bb jp z,$41e2               e0274 be e0279
02be call $0ab2               .
.                             e0276 br e6120
02c1 ld bc,$8000              e0279 mov cw,8000h

029f ld b,$fc
02a1 call $0c38               e0239 call 0e30b8h
02a4 ld hl,$d023              e023d mov ix,90h           (36 -> be)
02a7 call $2b2d               e0240 call 0e32a6h         (00 -> e8)
  2b2d ld a,$01                 e32a6 mov al,1h
  2b2f or a                     e32a8 and al,al
  2b30 add a,(hl)               e32aa add al,b ptr [ix]
  2b31 daa                      e32ac daa                (13 -> 27) not sure
  2b32 ld (hl),a                e32ad mov b ptr [ix],al
  2b33 push bc                  e32af push cw            (ac -> 51)
  2b34 ld b,$03                 e32b0 mov cw,3h          (9c -> b9)
  2b36 dec hl                   e32b3                               inc ix or dec ix?
  2b37 ld a,(hl)                e32b4 mov al,b ptr [ix]  (e0 -> 8a)
  2b38 adc a,$00                e32b6 adc al,0h          (1d -> 14)
  2b3a daa                      e32b8 daa
  2b3b ld (hl),a                e32b9 mov b ptr [ix],al
  2b3c djnz $2b36               e32bb dbnz e32b3h        (c5 -> e2)
  2b3e pop bc                   e32bd pop cw             (8a -> 59)
  2b3f ret                      e32be ret

  2b40-2b4b                     e32bf-e32d1              (06 -> 12) (07 -> 27 not sure) (d7 -> 4f dec iy or 47 inc iy)
  2b4c-2b57                     e32d2-e32e4              (af -> 1a)
  .                             e32e5-                   (8e -> 2a) (14 -> 1c)

02aa ld a,$01                 e0243 mov al,1h
02ac ld ($d618),a             e0245 mov [72dh],al
02af call $4a8e               e0248 call 0e66cbh
  4a8e ld a,($d618)             e66cb mov al,[72dh]      (1a -> a0)
  4a91 and a                    e66ce and al,al          (64 -> 22)
  4a92 jr z,$4abe               e66d0 be e6703           (3f -> 74)
  4a94 cp $02                   e66d2 cmp al,2h          (08 -> 3c)
  4a96 jr nz,$4aa6              e66d4 bne e66e9          (2b -> 75)
  4a98 ld a,$0d                 e66d6 mov al,0dh         (45 -> b0)
  .                             e66d8 mov dw,23h         (6e -> ba)
  4a9a out ($03),a              e66db out dw,al          (c2 -> ee)
  4a9c ld a,$00                 e66dc mov al,00h
  .                             e66de mov dw,22h
  4a9e out ($02),a              e66e1 out dw,al
  4aa0 xor a                    e66e2 xor al,al          (18 -> 32)
  4aa1 ld ($d618),al            e66e4 mov [72dh],al      (49 -> a2)
  4aa4 jr $4abe                 e66e7 br 0e6703h         (5c -> eb)

  4aa6 add a,a                  e66e9 add al,al          (72 -> 02)
  4aa7 ld e,a                   .
  4aa8 ld d,$00                 e66eb mov ah,00h         (b2 -> b4)
  4aaa ld hl,$4b62              e66ed mov bw,67cbh       (6d -> bb)
  4aad add lh,de                e66f0 add bw,aw          (ba -> 03)
  4aae ld e,(hl)                .
  4aaf inc hl                   .
  4abo ld d,(hl)                .
  4ab1 ex de,hl                 e66f2 mov bw,w ptr ss[bw](df -> 8b)
  4ab2 xor a                    .
  4ab3 ld ($d618),a             e66f5 mov b ptr[72dh],ah (d8 -> 88)
  4ab6 ld ($d619),a             e66f9 mov b ptr[72eh],ah
  4ab9 ld ($d61a),hl            e66fd mov w ptr[72fh],bw
  4abc jr $4ac9                 e6701 br 0e6712h

  4abe ld a,($d619)             e6703 mov al,[72eh]
  4ac1 and a                    e6706 and al,al
  4ac2 jr z,$4ac9               e6708 be 0e6712h
  4ac4 dec a                    e670a dec al             (f2 -> fe)
  4ac5 ld ($d619),a             e670c mov [72eh],al
  .                             e670f be 0e6712h
  4ac8 ret nz                   e6711 ret                (70 -> c3)

  4ac9 ld hl,($d61a)            e6712 mov bw,w ptr[72fh]
  .                             e6716 or bw,bw                      ??? (0b -> 23) ???
  .                             e6718 bne 0e671bh
  .                             e671a ret
  4acc ld a,(hl)                e671b mov al,b ptr ps[bw](ce -> 2e) (e0 -> 8a)
  4acd inc hl                   e671e inc bw             (7d -> 43)
  4ace cp $f0                   e671f cmp al,0f0h
  4ad0 jr nc,$4b14              e6721 bnc 0e676fh        (8c -> 73)

  4add sub $50                  e672f sub al,50h         (52 -> 2c)
  4adf cp $50                   e6731 cmp al,50h
  4ae1 jr c,$4ae7               e6733 bc 0e6739          (78 -> 72)
  4ae3 ld b,$04                 e6735 mov ch,4h          (26 -> b5)
  4ae5 sub $50                  e6737 sub al,50h
  4ae7 ld c,a                   e6739 mov cl,al
  4ae8 and $0f                  e673b and al,0fh         (76 -> 24)
  4aea add a,a                  e673d add al,al
  4aeb ld e,a                   .
  4aec ld d,$00                 e673f mov ah,0h
  4aee ld hl,$4b42              e6741 mov bw,67ab
  4af1 add hl,de                e6744 add bw,aw
  4af2 ld e,(hl)                e6746 mov bw,w ptr ps[bw]           not sure the prefix
  4af3 inc hl                   .
  4af4 ld d,(hl)                .
  4af5 ld a,c                   .
  4af6 and $f0                  e6749 and cl,f0h         (04 -> 80)
  4af8 jr z,$4b05               e674c be 0e6753
  4afa rrca                     .
  4afb rrca                     .
  4afc rrca                     .
  4afd rrca                     e674e ror cl,4h          (57 -> c0)
  4afe srl d                    .
  4b00 rr e                     e6751 shr bw,cl          (1b -> d3)
  4b02 dec a                    .
  4b03 jr nz,$4afe              .
  4b05 ld a,b                   e6753 mov al,ch
  .                             e6755 mov dw,23h
  4b06 out ($03),a              e6758 out dw,al

-------------------------------------------------

56 -> 42 (inc dw or dec dw)

92 -> e9 (probably)
dd -> fa (di) (probably)

checked against gussun (from 10000) and quizf1 (start up code):
41 -> ea (jmp_far)
21 -> 8e
a7 -> b8
de -> c7
e3 -> c6

*/


UINT32 cb2001_state::screen_update_cb2001(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count,x,y;
	bitmap.fill(m_palette->black_pen(), cliprect);

	count = 0x0000;

	// render bg as 8x8 tilemaps
	if (m_other1 & 0x02)
	{
		if (!(m_other1 & 0x04))
		{
			for (y=0;y<32;y++)
			{
				for (x=0;x<64;x++)
				{
					int tile;
					int colour;

					tile = (m_vram_bg[count] & 0x0fff);
					colour = (m_vram_bg[count] & 0xf000)>>12;
					tile += m_videobank*0x2000;


					m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,tile,colour,0,0,x*8,y*8);

					count++;
				}
			}
		}
		else
		{
			int i;

			for (i= 0;i < 64;i++)
			{
				UINT16 scroll;

				scroll = m_vram_bg[0xa00/2 + i/2];
				if (i&1)
					scroll >>=8;
				scroll &=0xff;

				m_reel2_tilemap->set_scrolly(i, scroll);

				scroll = m_vram_bg[0x800/2 + i/2];
				if (i&1)
					scroll >>=8;
				scroll &=0xff;

				m_reel1_tilemap->set_scrolly(i, scroll);

				scroll = m_vram_bg[0xc00/2 + i/2];
				if (i&1)
					scroll >>=8;
				scroll &=0xff;

				m_reel3_tilemap->set_scrolly(i, scroll);

			}

			// these areas are wrong
			const rectangle visible1(0*8, (14+48)*8-1,  3*8,  (3+7)*8-1);
			const rectangle visible2(0*8, (14+48)*8-1, 10*8, (10+7)*8-1);
			const rectangle visible3(0*8, (14+48)*8-1, 17*8, (17+7)*8-1);

			m_reel1_tilemap->draw(screen, bitmap, visible1, 0, 0);
			m_reel2_tilemap->draw(screen, bitmap, visible2, 0, 0);
			m_reel3_tilemap->draw(screen, bitmap, visible3, 0, 0);
		}
	}

	count = 0x0000;

	for (y=0;y<32;y++)
	{
		for (x=0;x<64;x++)
		{
			int tile;
			int colour;

			tile = (m_vram_fg[count] & 0x0fff);
			colour = (m_vram_fg[count] & 0xf000)>>12;
			tile += m_videobank*0x2000;

			if (m_other2 & 0x4)
			{
				tile += 0x1000;
			}

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,tile,colour,0,0,x*8,y*8,0);
			count++;
		}
	}

	popmessage("%02x %02x %02x %02x\n",m_videobank,m_videomode, m_other1, m_other2);

	return 0;
}


/* these ports sometimes get written with similar values
 - they could be hooked up wrong, or subject to change it the code
   is being executed incorrectly */
WRITE16_MEMBER(cb2001_state::cb2001_vidctrl_w)
{
	if (mem_mask&0xff00) // video control?
	{
		printf("cb2001_vidctrl_w %04x %04x\n", data, mem_mask);
		m_videobank = (data & 0x0800)>>11;
	}
	else // something else
		m_other1 = data & 0x00ff;
}

WRITE16_MEMBER(cb2001_state::cb2001_vidctrl2_w)
{
	if (mem_mask&0xff00) // video control?
	{
		printf("cb2001_vidctrl2_w %04x %04x\n", data, mem_mask); // i think this switches to 'reels' mode
		m_videomode = (data>>8) & 0x03; // which bit??
	}
	else // something else
		m_other2 = data & 0x00ff;

//      printf("cb2001_vidctrl2_w %04x %04x\n", data, mem_mask); // bank could be here instead
}


TILE_GET_INFO_MEMBER(cb2001_state::get_cb2001_reel1_tile_info)
{
	int code = m_vram_bg[(0x0000/2) + tile_index/2];

	if (tile_index&1)
		code >>=8;

	code &=0xff;

	int colour = 0;//= (cb2001_out_c&0x7) + 8;

	SET_TILE_INFO_MEMBER(1,
			code+0x800,
			colour,
			0);
}

TILE_GET_INFO_MEMBER(cb2001_state::get_cb2001_reel2_tile_info)
{
	int code = m_vram_bg[(0x0200/2) + tile_index/2];

	if (tile_index&1)
		code >>=8;

	code &=0xff;

	int colour = 0;//(cb2001_out_c&0x7) + 8;

	SET_TILE_INFO_MEMBER(1,
			code+0x800,
			colour,
			0);
}


TILE_GET_INFO_MEMBER(cb2001_state::get_cb2001_reel3_tile_info)
{
	int code = m_vram_bg[(0x0400/2) + tile_index/2];
	int colour = 0;//(cb2001_out_c&0x7) + 8;

	if (tile_index&1)
		code >>=8;

	code &=0xff;

	SET_TILE_INFO_MEMBER(1,
			code+0x800,
			colour,
			0);
}


void cb2001_state::video_start()
{
	m_reel1_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cb2001_state::get_cb2001_reel1_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel2_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cb2001_state::get_cb2001_reel2_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel3_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(cb2001_state::get_cb2001_reel3_tile_info),this),TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel1_tilemap->set_scroll_cols(64);
	m_reel2_tilemap->set_scroll_cols(64);
	m_reel3_tilemap->set_scroll_cols(64);
}

WRITE16_MEMBER(cb2001_state::cb2001_bg_w)
{
	COMBINE_DATA(&m_vram_bg[offset]);

	// also used for the reel tilemaps in a different mode
/*
    if (offset<0x200/2)
    {
        m_reel1_tilemap->mark_tile_dirty((offset&0xff)/2);
    }
    else if (offset<0x400/2)
    {
        m_reel2_tilemap->mark_tile_dirty((offset&0xff)/2);
    }
    else if (offset<0x600/2)
    {
        m_reel3_tilemap->mark_tile_dirty((offset&0xff)/2);
    }
    else if (offset<0x800/2)
    {
    //  reel4_tilemap->mark_tile_dirty((offset&0xff)/2);
    }
*/
	m_reel1_tilemap->mark_all_dirty();
	m_reel2_tilemap->mark_all_dirty();
	m_reel3_tilemap->mark_all_dirty();


}

static ADDRESS_MAP_START( cb2001_map, AS_PROGRAM, 16, cb2001_state )
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0x20000, 0x20fff) AM_RAM AM_SHARE("vrafg")
	AM_RANGE(0x21000, 0x21fff) AM_RAM_WRITE(cb2001_bg_w) AM_SHARE("vrabg")
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cb2001_io, AS_IO, 16, cb2001_state )
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE8("ppi8255_0", i8255_device, read, write, 0xffff)   /* Input Ports */
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE8("ppi8255_1", i8255_device, read, write, 0xffff)   /* DIP switches */
	AM_RANGE(0x20, 0x21) AM_DEVREAD8("aysnd", ay8910_device, data_r, 0xff00)
	AM_RANGE(0x22, 0x23) AM_DEVWRITE8("aysnd", ay8910_device, data_address_w, 0xffff)

	AM_RANGE(0x30, 0x31) AM_WRITE(cb2001_vidctrl_w)
	AM_RANGE(0x32, 0x33) AM_WRITE(cb2001_vidctrl2_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( cb2001 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  /* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)  /* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)  /* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  /* Coin A */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")  /* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" ) PORT_DIPLOCATION("DSW1:2")  /* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )       PORT_DIPLOCATION("DSW1:3")  /* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )          PORT_DIPLOCATION("DSW1:4")  /* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )     PORT_DIPLOCATION("DSW1:5")  /* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )         PORT_DIPLOCATION("DSW1:6")  /* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )           PORT_DIPLOCATION("DSW1:7,8")    /* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )    PORT_DIPLOCATION("DSW2:1,2,3")  /* OK */
	PORT_DIPSETTING(    0x07, "35%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "45%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )          PORT_DIPLOCATION("DSW2:4,5")    /* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )        PORT_DIPLOCATION("DSW2:6")  /* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )           PORT_DIPLOCATION("DSW2:7")  /* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )  PORT_DIPLOCATION("DSW2:8")  /* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")  /* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")  /* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")  /* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")    /* not checked */
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x08, "Display Of Payout Limit" ) PORT_DIPLOCATION("DSW4:4") /* not working */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")    /* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")    /* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")    /* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")    /* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")   /* not working */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3") /* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5") /* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")   /* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")   /* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")   /* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(cb2001_state::vblank_irq)
{
	generic_pulse_irq_line(device.execute(), NEC_INPUT_LINE_INTP0, 1);
}

static const gfx_layout cb2001_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28, 16,20 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};


static const gfx_layout cb2001_layout32 =
	{
	8,32,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 8,12,0,4,24,28, 16,20 },
	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32,14*32,15*32,16*32,17*32,18*32,19*32,20*32,21*32,22*32,23*32,24*32,25*32,26*32,27*32,28*32,29*32,30*32,31*32 },
	32*32
};

static GFXDECODE_START( cb2001 )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout,   0x0, 32 )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout32, 0x0, 32 )
GFXDECODE_END

PALETTE_INIT_MEMBER(cb2001_state, cb2001)
{
	int i;
	for (i = 0; i < 0x200; i++)
	{
		int r,g,b;

		UINT8*proms = memregion("proms")->base();
		int length = memregion("proms")->bytes();
		UINT16 dat;

		dat = (proms[0x000+i] << 8) | proms[0x200+i];


		b = ((dat >> 1) & 0x1f)<<3;
		r = ((dat >> 6 )& 0x1f)<<3;
		g = ((dat >> 11 ) & 0x1f)<<3;

		if (length==0x400) // are the cb2001 proms dumped incorrectly?
		{
			if (!(i&0x20)) palette.set_pen_color((i&0x1f) | ((i&~0x3f)>>1), rgb_t(r, g, b));
		}
		else
		{
			palette.set_pen_color(i, rgb_t(r, g, b));
		}
	}
}

static MACHINE_CONFIG_START( cb2001, cb2001_state )
	MCFG_CPU_ADD("maincpu", V35, 20000000) // CPU91A-011-0016JK004; encrypted cpu like nec v25/35 used in some irem game
	MCFG_V25_CONFIG(cb2001_decryption_table)
	MCFG_CPU_PROGRAM_MAP(cb2001_map)
	MCFG_CPU_IO_MAP(cb2001_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cb2001_state,  vblank_irq)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("DSW1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("DSW2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("DSW3"))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cb2001)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(cb2001_state, screen_update_cb2001)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(cb2001_state, cb2001)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 1500000) // wrong
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW4"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW5"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( cb2001 )
	ROM_REGION( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "c01111.11f", 0x020000, 0x20000, CRC(ec6269f1) SHA1(f2428562a10e30192f2c95053f5ce448302e7cf5) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "c0111.12a", 0x000000, 0x80000, CRC(342b760e) SHA1(bc168bec384ccacd73543f604e3ab5b2b8f4f441) )

	ROM_REGION( 0x400, "proms", 0 ) // ?
	ROM_LOAD( "am27s29.9b",  0x000, 0x200, CRC(6c90f6a2) SHA1(f3f592954000d189ded0ed8c6c4444ace0b616a4) )
	ROM_LOAD( "am27s29.11b", 0x200, 0x200, CRC(e5aa3ec7) SHA1(675711dd6788b3d0c37573b49b6297cbcd8c8209) )
ROM_END

ROM_START( scherrym )
	ROM_REGION( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "f11.bin", 0x000000, 0x40000, CRC(8967f58d) SHA1(eb01a16b7d108f5fbe5de8f611b4f77869aedbf1) )

	ROM_REGION( 0x080000, "gfx", ROMREGION_ERASEFF )
	ROM_LOAD( "gfx.12c", 0x000000, 0x80000,NO_DUMP ) // this board uses an unmarked MASK rom at 12c, 12a is unpopulated.  Size unknown.

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "n82s135-1.bin", 0x000, 0x100, CRC(66ed363f) SHA1(65bd37842c441c2e712844b07c0cfe37ef16d0ef) )
	ROM_LOAD( "n82s135-2.bin", 0x200, 0x100, CRC(a19821db) SHA1(62dda90dd67dfbc0b96f161f1f2b7a46a5805eae) )
ROM_END

GAME( 2001, cb2001,    0,      cb2001,      cb2001, driver_device,   0, ROT0,  "Dyna", "Cherry Bonus 2001", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
GAME( 2001, scherrym,  0,      cb2001,      cb2001, driver_device,   0, ROT0,  "Dyna", "Super Cherry Master", MACHINE_NOT_WORKING|MACHINE_NO_SOUND ) // 2001 version? (we have bootlegs running on z80 hw of a 1996 version)
