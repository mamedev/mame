// license:BSD-3-Clause
// copyright-holders: David Haywood, Roberto Zandona'

/*************************************************************************************************

This driver covers Dyna games running on the DYNA CPU91A-011 custom CPU.
It is an encrypted NEC V25 or V35.
It has been seen on the following PCBs:
D9203
D9205 or D9205B (sub PCB)
D9304
D9401
D9701 (sub PCB)
D9702
D9805


  Cherry Bonus 2001  (c)2000/2001 Dyna


Produttore  Dyna
N.revisione
CPU

1x DYNA CPU91A-011-0016JK004 (QFP84) custom
1x DYNA DC3001-0051A (QFP128) custom
1x DYNA 22A078803 (DIP42) (similar to an I8255)
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

To enter input test, keep '9' pressed and press 'F3'.

TODO:
- correct / complete CPU decryption table. Most games run for a while before getting stuck.
  dynastye is the exception. It's a very early game which seems to use a rather different
  codebase and trips on not yet decrypted opcodes almost immediately;
- the DC3001 GFX custom is suspected to have internal ROM (seems used by scherrym and clones);
- dynastye doesn't seem to have the DC3001 GFX custom or at least uses a different GFX format;
- all games will need proper i/o once they fully work.

*************************************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/nec/v25.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_VIDEOREGS     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_VIDEOREGS)

#include "logmacro.h"

#define LOGVIDEOREGS(...)     LOGMASKED(LOG_VIDEOREGS,     __VA_ARGS__)


namespace {

class cb2001_state : public driver_device
{
public:
	cb2001_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram_fg(*this, "vrafg"),
		m_vram_bg(*this, "vrabg"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void cb2001(machine_config &config);
	void cb5(machine_config &config);
	void ndongmul2(machine_config &config);
	void scherrym(machine_config &config);
	void scherrymp(machine_config &config);

	void init_smaller_proms();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint16_t> m_vram_fg;
	required_shared_ptr<uint16_t> m_vram_bg;
	required_device<v35_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t m_videobank = 0;
	uint8_t m_videomode = 0;
	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_reel_tilemap[3]{};
	uint8_t m_other1 = 0;
	uint8_t m_other2 = 0;

	void vidctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vidctrl2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void bg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	void palette_init(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	uint8_t irq_ack_r();
	void io_map(address_map &map) ATTR_COLD;
	void cb5_io_map(address_map &map) ATTR_COLD;
	void scherrym_io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
};


#define xxxx 0x90 // Unknown

static const uint8_t cb2001_decryption_table[256] = {
	0xe8,xxxx,xxxx,xxxx,0x80,0xe4,0x12,0x2f, 0x3c,xxxx,xxxx,0x23,xxxx,xxxx,xxxx,0x5f, // 00
//    ssss                ---- **** pppp pppp  ssss           pppp                pppp
	0x86,xxxx,xxxx,0x27,0x1c,xxxx,xxxx,xxxx, 0x32,0x40,0xa0,0xd3,0x3a,0x14,0x89,0x1f, // 10
//    rrrr           **** pppp                 pppp pppp pppp pppp ppp? pppp pppp ssss
	xxxx,0x8e,xxxx,0x0f,xxxx,0x49,0xb5,xxxx, 0x56,xxxx,xxxx,0x75,0x33,0xb6,xxxx,0x39, // 20
//         ssss      ssss      pppp pppp       pppp           ssss pppp pppp      ****
	0x89,xxxx,xxxx,xxxx,xxxx,0x22,0x5b,xxxx, xxxx,xxxx,0x74,xxxx,xxxx,0xa6,xxxx,0x74, // 30
//    wwww                     **** pppp                 debu           pppp      ssss
	xxxx,0xea,xxxx,xxxx,0xd0,0xb0,0x5e,xxxx, xxxx,0xa2,xxxx,xxxx,0xa3,xxxx,xxxx,0xb3, // 40
//         ssss           **** pppp pppp            pppp           ssss           pppp
	0x13,xxxx,0x2c,xxxx,0x9d,xxxx,0x42,0xc0, 0x04,xxxx,0xb7,xxxx,0xeb,0xab,xxxx,xxxx, // 50
//    ????      ssss      ****      pppp pppp  ****      ****      ssss pppp
	xxxx,xxxx,xxxx,xxxx,0x0a,xxxx,xxxx,xxxx, 0xa1,0xa5,xxxx,xxxx,xxxx,0xbb,0xba,xxxx, // 60
//                        pppp                 ssss pppp                pppp ssss
	0xc3,0x53,0x02,0x58,xxxx,xxxx,0x24,xxxx, 0x72,xxxx,0xf3,xxxx,xxxx,0x43,xxxx,0x34, // 70
//    ssss pppp pppp ssss           pppp       ssss      pppp           ssss      ****
	0x26,xxxx,0xd1,xxxx,xxxx,0x3d,0xfb,0xf6, xxxx,xxxx,0x59,xxxx,0x73,xxxx,0x2a,xxxx, // 80
//    pppp      rrrr           pppp **** ssss            pppp      ssss      pppp
	xxxx,0x3d,0xe9,xxxx,xxxx,0xbe,0xf9,xxxx, xxxx,xxxx,0x57,xxxx,0xb9,xxxx,0xbf,xxxx, // 90
//         wwww pppp           pppp ****                 pppp      ssss      ssss
	0xc1,xxxx,0xe6,0x06,0xaa,0x9c,0xad,0xb8, 0x4e,xxxx,0x8d,0x50,0x51,0xa4,xxxx,0x1a, // A0
//    ****      pppp ssss pppp **** pppp ssss  pppp      ssss ssss pppp pppp      pppp
	0xac,xxxx,0xb4,xxxx,xxxx,0x83,xxxx,xxxx, xxxx,0x05,0x03,xxxx,0x1e,0x43,0x07,0xcf, // B0
//    pppp      ssss           pppp                 pppp pppp      ssss **** ssss ssss
	0xcb,0xec,0xee,xxxx,xxxx,0xe2,0x87,xxxx, xxxx,xxxx,0x76,0x61,0x48,xxxx,0x2e,xxxx, // C0
//    ssss ssss pppp           ssss pppp                 pppp **** ****      pppp
	xxxx,0xf2,0x46,xxxx,0x60,xxxx,0x4f,0x47, 0x88,xxxx,xxxx,0xff,xxxx,0xfa,0xc7,0x8b, // D0
//         pppp pppp      ****      pppp pppp  pppp           ssss      **** ssss pppp
	0x8a,0xb1,xxxx,0xc6,xxxx,0x5a,xxxx,0xb2, 0x9a,0x52,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, // E0
//    ssss gggg      ssss      ****      pppp  pppp ****
	xxxx,0xae,0xfe,xxxx,xxxx,xxxx,xxxx,0x3a, xxxx,xxxx,0x34,xxxx,0x81,xxxx,xxxx,xxxx, // F0
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


void cb2001_state::machine_start()
{
	save_item(NAME(m_videobank));
	save_item(NAME(m_videomode));
	save_item(NAME(m_other1));
	save_item(NAME(m_other2));
}


uint32_t cb2001_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	// render bg as 8x8 tilemaps
	if (m_other1 & 0x02)
	{
		if (!(m_other1 & 0x04))
			m_bg_tilemap->draw(screen, bitmap, cliprect);
		else
		{
			for (int i = 0; i < 64; i++)
			{
				uint16_t scroll = m_vram_bg[0xa00 / 2 + i / 2];
				if (i & 1)
					scroll >>= 8;
				scroll &= 0xff;

				m_reel_tilemap[1]->set_scrolly(i, scroll);

				scroll = m_vram_bg[0x800 / 2 + i / 2];
				if (i & 1)
					scroll >>= 8;
				scroll &= 0xff;

				m_reel_tilemap[0]->set_scrolly(i, scroll);

				scroll = m_vram_bg[0xc00 / 2 + i / 2];
				if (i & 1)
					scroll >>= 8;
				scroll &= 0xff;

				m_reel_tilemap[2]->set_scrolly(i, scroll);

			}

			// these areas are wrong
			const rectangle visible1(0*8, (14+48)*8-1,  3*8,  (3+7)*8-1);
			const rectangle visible2(0*8, (14+48)*8-1, 10*8, (10+7)*8-1);
			const rectangle visible3(0*8, (14+48)*8-1, 17*8, (17+7)*8-1);

			m_reel_tilemap[0]->draw(screen, bitmap, visible1, 0, 0);
			m_reel_tilemap[1]->draw(screen, bitmap, visible2, 0, 0);
			m_reel_tilemap[2]->draw(screen, bitmap, visible3, 0, 0);
		}
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect);

	LOGVIDEOREGS("%02x %02x %02x %02x\n", m_videobank, m_videomode, m_other1, m_other2);

	return 0;
}


/* these ports sometimes get written with similar values
 - they could be hooked up wrong, or subject to change it the code
   is being executed incorrectly */
void cb2001_state::vidctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15) // video control?
	{
		LOGVIDEOREGS("vidctrl_w %04x %04x\n", data, mem_mask);
		m_videobank = (data & 0x1c00) >> 10;
	}
	else // something else
		m_other1 = data & 0x00ff;
}

void cb2001_state::vidctrl2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15) // video control?
	{
		LOGVIDEOREGS("vidctrl2_w %04x %04x\n", data, mem_mask); // I think this switches to 'reels' mode
		m_videomode = (data >> 8) & 0x03; // which bit??
	}
	else // something else
		m_other2 = data & 0x00ff;

//      LOGVIDEOREGS("vidctrl2_w %04x %04x\n", data, mem_mask); // bank could be here instead
}


TILE_GET_INFO_MEMBER(cb2001_state::get_bg_tile_info)
{
	int const code = (m_vram_bg[tile_index] & 0x0fff) | (m_videobank * 0x1000);
	int const colour = (m_vram_bg[tile_index] & 0xf000) >> 12;

	tileinfo.set(0, code, colour, 0);
}

TILE_GET_INFO_MEMBER(cb2001_state::get_fg_tile_info)
{
	int const code = (m_vram_fg[tile_index] & 0x0fff) | (m_videobank * 0x1000);
	int const colour = (m_vram_fg[tile_index] & 0xf000) >> 12;

	tileinfo.set(0, code, colour, 0);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(cb2001_state::get_reel_tile_info)
{
	int code = m_vram_bg[(Which * 0x200 / 2) + tile_index / 2];

	if (tile_index & 1)
		code >>= 8;

	code &= 0xff;

	int const reel_bank = (m_other2 & 0x1c) << 8;

	int const colour = 0; //= (out_c & 0x7) + 8;

	tileinfo.set(1, code | reel_bank, colour, 0);
}


void cb2001_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_fg_tilemap->set_transparent_pen(0);

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cb2001_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel_tilemap[0]->set_scroll_cols(64);
	m_reel_tilemap[1]->set_scroll_cols(64);
	m_reel_tilemap[2]->set_scroll_cols(64);
}

void cb2001_state::bg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_bg[offset]);

	m_bg_tilemap->mark_tile_dirty(offset);

	// also used for the reel tilemaps in a different mode
/*
    if (offset < 0x200 / 2)
    {
        m_reel_tilemap[0]->mark_tile_dirty((offset & 0xff) / 2);
    }
    else if (offset < 0x400 / 2)
    {
        m_reel_tilemap[1]->mark_tile_dirty((offset & 0xff) / 2);
    }
    else if (offset < 0x600 / 2)
    {
        m_reel_tilemap[2]->mark_tile_dirty((offset & 0xff) / 2);
    }
    else if (offset < 0x800 / 2)
    {
    //  m_reel_tilemap[3]->mark_tile_dirty((offset & 0xff) / 2);
    }
*/
	m_reel_tilemap[0]->mark_all_dirty();
	m_reel_tilemap[1]->mark_all_dirty();
	m_reel_tilemap[2]->mark_all_dirty();
}

void cb2001_state::fg_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_vram_fg[offset]);

	m_fg_tilemap->mark_tile_dirty(offset);
}

void cb2001_state::program_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0x20fff).ram().w(FUNC(cb2001_state::fg_w)).share(m_vram_fg);
	map(0x21000, 0x21fff).ram().w(FUNC(cb2001_state::bg_w)).share(m_vram_bg);
	map(0xc0000, 0xfffff).rom().region("boot_prg", 0);
}

void cb2001_state::io_map(address_map &map)
{
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));   // Input ports
	map(0x10, 0x11).portr("DSW1-2");
	map(0x12, 0x13).portr("DSW3");
	map(0x21, 0x21).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x22, 0x23).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x30, 0x30).r(FUNC(cb2001_state::irq_ack_r));
	map(0x30, 0x31).w(FUNC(cb2001_state::vidctrl_w));
	map(0x32, 0x33).w(FUNC(cb2001_state::vidctrl2_w));
}

void cb2001_state::scherrym_io_map(address_map &map)
{
	map(0x00, 0x00).r(FUNC(cb2001_state::irq_ack_r));
	map(0x00, 0x01).w(FUNC(cb2001_state::vidctrl_w));
	map(0x02, 0x03).w(FUNC(cb2001_state::vidctrl2_w));
	map(0x10, 0x11).portr("DSW1-2");
	map(0x12, 0x13).portr("DSW3");
	map(0x21, 0x21).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x22, 0x23).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x30, 0x33).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));   // Input ports
}

void cb2001_state::cb5_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0000).r(FUNC(cb2001_state::irq_ack_r));
	map(0x0000, 0x0001).w(FUNC(cb2001_state::vidctrl_w));
	map(0x0002, 0x0003).w(FUNC(cb2001_state::vidctrl2_w));
	// this one has the main PCB hw mapped in the 0xb800 - 0xbfff area
	map(0xb800, 0xb803).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb810, 0xb813).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb820, 0xb823).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xb830, 0xb830).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xb840, 0xb840).w("aysnd", FUNC(ay8910_device::address_w));
	// map(0xb850, 0xb850).w // TODO: probably lamps
	// map(0xb860, 0xb860).w // TODO: probably lamps
	// map(0xb870, 0xb870).w // TODO: leftover from the SN76489 days?
}

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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin B
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)  // Coin D
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)  // Coin C
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Reel Speed" )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( High ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0000, "Main Game Pay Rate" )  PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "55%" )
	PORT_DIPSETTING(      0x0600, "60%" )
	PORT_DIPSETTING(      0x0500, "65%" )
	PORT_DIPSETTING(      0x0400, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0200, "80%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x0800, 0x0000, "Double Up Game Pay Rate" )  PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0800, "80%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Maximum Bet" )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x2000, "20" )
	PORT_DIPNAME( 0x4000, 0x0000, "Minimum Bet" )  PORT_DIPLOCATION("DSW2:7") // fixed at 1 in the service screen but shows 8 or 16 during attract
	PORT_DIPSETTING(      0x4000, "8" )
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( scherrymp )
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin B
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)  // Coin D
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)  // Coin C
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Reel Speed" )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( High ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0000, "Main Game Pay Rate" )  PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "55%" )
	PORT_DIPSETTING(      0x0600, "60%" )
	PORT_DIPSETTING(      0x0500, "65%" )
	PORT_DIPSETTING(      0x0400, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0200, "80%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x0800, 0x0000, "Double Up Game Pay Rate" )  PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0800, "80%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, "Maximum Bet" )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x0000, "10" )
	PORT_DIPSETTING(      0x2000, "20" )
	PORT_DIPNAME( 0x4000, 0x0000, "Minimum Bet" )  PORT_DIPLOCATION("DSW2:7") // fixed at 1 in the service screen but shows 8 or 16 during attract
	PORT_DIPSETTING(      0x4000, "8" )
	PORT_DIPSETTING(      0x0000, "16" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW6")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	 // test mode shows a 7th bank and it's read, but it wasn't populated on the dumped PCB. Leaving it here until the controls are done.
	PORT_START("DSW7")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cb5 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ndongmul2 )
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
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)  // Coin B
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)  // Coin D
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)  // Coin C
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)  // Coin A

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Out / Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Settings")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Stats")

	PORT_START("DSW1-2")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Reel Speed" )  PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( High ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0000, "Main Game Pay Rate" )  PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(      0x0700, "55%" )
	PORT_DIPSETTING(      0x0600, "60%" )
	PORT_DIPSETTING(      0x0500, "65%" )
	PORT_DIPSETTING(      0x0400, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0200, "80%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x1800, 0x0000, "Maximum Bet" )  PORT_DIPLOCATION("DSW2:4,5")
	PORT_DIPSETTING(      0x1800, "8" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0800, "32" )
	PORT_DIPSETTING(      0x0000, "64" )
	PORT_DIPNAME( 0x6000, 0x0000, "Minimum Bet" )  PORT_DIPLOCATION("DSW2:6,7")
	PORT_DIPSETTING(      0x6000, "8" )
	PORT_DIPSETTING(      0x4000, "8" ) // yes, same
	PORT_DIPSETTING(      0x2000, "16" )
	PORT_DIPSETTING(      0x0000, "32" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:4")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW6")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW6:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW7")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW7:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


INTERRUPT_GEN_MEMBER(cb2001_state::vblank_irq)
{
	m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, ASSERT_LINE);
}

uint8_t cb2001_state::irq_ack_r()
{
	m_maincpu->set_input_line(NEC_INPUT_LINE_INTP0, CLEAR_LINE);
	return 0xff;
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

static GFXDECODE_START( gfx_cb2001 )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout,   0x0, 32 )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout32, 0x0, 32 )
GFXDECODE_END

void cb2001_state::palette_init(palette_device &palette) const
{
	uint8_t const *const proms = memregion("proms")->base();

	for (int i = 0; i < 0x200; i++)
	{
		uint16_t dat = (proms[0x000 + i] << 8) | proms[0x200 + i];

		int const b = ((dat >> 1) & 0x1f) << 3;
		int const r = ((dat >> 6) & 0x1f) << 3;
		int const g = ((dat >> 11) & 0x1f) << 3;

		if (!(i & 0x20)) palette.set_pen_color((i & 0x1f) | ((i & ~0x3f) >> 1), rgb_t(r, g, b));
	}
}

void cb2001_state::cb2001(machine_config &config)
{
	V35(config, m_maincpu, 20'000'000); // CPU91A-011-0016JK004; encrypted CPU like NEC V25/35 used in some Irem games
	m_maincpu->set_decryption_table(cb2001_decryption_table);
	m_maincpu->set_addrmap(AS_PROGRAM, &cb2001_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &cb2001_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(cb2001_state::vblank_irq));

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("IN2");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cb2001);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(cb2001_state::screen_update));

	PALETTE(config, m_palette, FUNC(cb2001_state::palette_init), 0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", 1'500'000)); // wrong
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

void cb2001_state::cb5(machine_config &config)
{
	cb2001(config);

	m_maincpu->set_addrmap(AS_IO, &cb2001_state::cb5_io_map);
	m_maincpu->p0_in_cb().set_ioport("DSW5");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("IN2");
	ppi1.in_pb_callback().set_ioport("IN3");
	ppi1.in_pc_callback().set_ioport("DSW1");

	i8255_device &ppi2(I8255A(config, "ppi8255_2"));
	ppi2.in_pa_callback().set_ioport("DSW2");

	subdevice<ay8910_device>("aysnd")->port_b_read_callback().set_ioport("DSW3");
}

void cb2001_state::scherrym(machine_config &config)
{
	cb2001(config);

	m_maincpu->set_addrmap(AS_IO, &cb2001_state::scherrym_io_map);
}

void cb2001_state::scherrymp(machine_config &config)
{
	cb2001(config);

	m_maincpu->set_clock(24_MHz_XTAL);
	m_maincpu->p0_in_cb().set_ioport("DSW6");
	m_maincpu->p2_in_cb().set_ioport("DSW7");
}

void cb2001_state::ndongmul2(machine_config &config)
{
	scherrymp(config);

	I80C51(config, "mcu", 12_MHz_XTAL).set_disable(); // Actually an AT89C51, currently undumped so disabled
}


ROM_START( cb2001 ) // DYNA D9702 PCB; DYNA CO1 V1.1I in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "c01111.11f", 0x020000, 0x20000, CRC(ec6269f1) SHA1(f2428562a10e30192f2c95053f5ce448302e7cf5) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "c0111.12a", 0x000000, 0x80000, CRC(342b760e) SHA1(bc168bec384ccacd73543f604e3ab5b2b8f4f441) )

	ROM_REGION( 0x400, "proms", 0 ) // ?
	ROM_LOAD( "am27s29.9b",  0x000, 0x200, CRC(6c90f6a2) SHA1(f3f592954000d189ded0ed8c6c4444ace0b616a4) )
	ROM_LOAD( "am27s29.11b", 0x200, 0x200, CRC(e5aa3ec7) SHA1(675711dd6788b3d0c37573b49b6297cbcd8c8209) )
ROM_END

ROM_START( dynastye ) // DYNA D9203 PCB; DYNA DYN3 V5.1G in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "ds_51g.16f", 0x30000, 0x10000, CRC(ebc3397f) SHA1(8870dcf465757f1c4fedabe863fb41e4d42e4952) )

	ROM_REGION( 0x20000, "gfx", 0 )
	ROM_LOAD( "ds_3h.3h", 0x00000, 0x20000, CRC(3d1a7f92) SHA1(2bb85a6738a8ffe29a238f7276b7afd59c5ebafe) ) // D27C010

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.11e", 0x000, 0x100, CRC(b1d8c6b4) SHA1(ef9aa1627a5025be3cfaa188c2972ce81e57c474) )
	ROM_LOAD( "82s135.12e", 0x100, 0x100, CRC(b7fa3d99) SHA1(a8102d1637596a13733446dfdb37ec8c13185412) )
ROM_END

ROM_START( scherrym ) // DYNA D9304 PCB; DYNA SCM V5.2 in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "v5.2.11b", 0x20000, 0x10000, CRC(26417279) SHA1(a83b8c050f1a2ab379f69175f2416e6b0b43b940) )
	ROM_RELOAD(                  0x30000, 0x10000)

	ROM_REGION( 0x20000, "gfx", 0 )
	// this PCB has an empty socket near the D3001 custom GFX chip, but it works correctly.
	// Does the D3001 have a stock internal ROM which can be overridden by an external ROM chip?
	ROM_LOAD( "d3001", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.2d", 0x000, 0x100, CRC(e87ed5c9) SHA1(ecdfa9586f9daffdb366154b02febcdb535a1427) )
	ROM_LOAD( "82s135.3d", 0x100, 0x100, CRC(16af0d6d) SHA1(a2004091aec05ee85ae8b82766e7c3013ca87bc4) )
ROM_END

ROM_START( scherrym12 ) // DYNA D9304 PCB; DYNA SCM V1.2 in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "v1.2.11b", 0x20000, 0x10000, CRC(94d452c1) SHA1(a99b54f71318c82a9a5005ff4cc8efc17de6a327) )
	ROM_RELOAD(                  0x30000, 0x10000)

	ROM_REGION( 0x20000, "gfx", 0 )
	// this PCB has an empty socket near the D3001 custom GFX chip, but it works correctly.
	// Does the D3001 have a stock internal ROM which can be overridden by an external ROM chip?
	ROM_LOAD( "d3001", 0x00000, 0x20000, NO_DUMP )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.2d", 0x000, 0x100, CRC(e87ed5c9) SHA1(ecdfa9586f9daffdb366154b02febcdb535a1427) )
	ROM_LOAD( "82s135.3d", 0x100, 0x100, CRC(16af0d6d) SHA1(a2004091aec05ee85ae8b82766e7c3013ca87bc4) )
ROM_END

ROM_START( scherrymp ) // DYNA D9702 PCB; DYNA PLUS V1.6 in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "supercherrymaster_v1.6d.11f", 0x000000, 0x40000, CRC(8967f58d) SHA1(eb01a16b7d108f5fbe5de8f611b4f77869aedbf1) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "d9701.12c", 0x000000, 0x100000, CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(66ed363f) SHA1(65bd37842c441c2e712844b07c0cfe37ef16d0ef) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(a19821db) SHA1(62dda90dd67dfbc0b96f161f1f2b7a46a5805eae) )
ROM_END

ROM_START( scherrymp10u ) // DYNA D9702 PCB; DYNA PLUS V1.0U in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "m27c2001.bin", 0x000000, 0x40000, CRC(6e797b3f) SHA1(cc333e3dc2d416f1059559ce958bfe25a3869fc8) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "d9701.12c", 0x000000, 0x100000, CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "82s147.9b",  0x000, 0x200, CRC(dcf976d2) SHA1(73a08e4587f3516d694a8060b79470cf71df3925) )
	ROM_LOAD( "82s147.11b", 0x200, 0x200, CRC(a67e7a63) SHA1(b23e0eb9af13e57bbc8602ddc7fb381ba5c8267e) )
ROM_END

ROM_START( cb4 ) // Wing W4 board + DYNA D9205 subboard; DYNA CB4 V5.0 in bookkeeping screen.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "5mk.2g",  0x20000, 0x10000, CRC(ecc6f80e) SHA1(b6de63cd5231ef9481ee79d841a6ea591add7e4d) )
	ROM_RELOAD(                 0x30000, 0x10000)

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD16_BYTE( "cb4.4i", 0x000000, 0x040000, CRC(c1799150) SHA1(50e80607b93f6ee35e3e8ff5d854dc83afe76505) )
	ROM_LOAD16_BYTE( "cb4.4j", 0x000001, 0x040000, CRC(3a12cf69) SHA1(232eeca78cdabcd952825aba0ad397e3dde79747) )

	ROM_REGION( 0x400, "proms", 0 ) // not dumped yet
	ROM_LOAD( "82s147.9b",  0x000, 0x200, BAD_DUMP CRC(dcf976d2) SHA1(73a08e4587f3516d694a8060b79470cf71df3925) )
	ROM_LOAD( "82s147.11b", 0x200, 0x200, BAD_DUMP CRC(a67e7a63) SHA1(b23e0eb9af13e57bbc8602ddc7fb381ba5c8267e) )
ROM_END

ROM_START( cb4a ) // DYNA D9300 + DYNA D9205 subboard; DYNA CB4T V1.2 in bookkeeping screen.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "12fb.2g", 0x20000, 0x10000, CRC(2d0e519a) SHA1(a538e3003d8a008dd45ea9fd10b249744b87f3a6) )
	ROM_RELOAD(                 0x30000, 0x10000)

	ROM_REGION( 0x100000, "gfx", 0 ) // not dumped for this set
	ROM_LOAD16_BYTE( "cb4.4i", 0x000000, 0x040000, BAD_DUMP CRC(c1799150) SHA1(50e80607b93f6ee35e3e8ff5d854dc83afe76505) )
	ROM_LOAD16_BYTE( "cb4.4j", 0x000001, 0x040000, BAD_DUMP CRC(3a12cf69) SHA1(232eeca78cdabcd952825aba0ad397e3dde79747) )

	ROM_REGION( 0x400, "proms", 0 ) // not dumped yet
	ROM_LOAD( "82s147.9b",  0x000, 0x200, BAD_DUMP CRC(dcf976d2) SHA1(73a08e4587f3516d694a8060b79470cf71df3925) )
	ROM_LOAD( "82s147.11b", 0x200, 0x200, BAD_DUMP CRC(a67e7a63) SHA1(b23e0eb9af13e57bbc8602ddc7fb381ba5c8267e) )
ROM_END

ROM_START( cb5 ) // Wing W4 board + DYNA D9701 subboard; DYNA CB5 V1.3 in bookkeeping screen. Appears to be the missing link to igs/goldstar.cpp hw.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "cb5-131.1g", 0x020000, 0x20000, CRC(7d47192c) SHA1(bc65f0b3223789fbcd78a7f3ba4f1c0e2a1ee4da) )

	ROM_REGION( 0x100000, "gfx", 0 ) // not dumped for this set, but seems to work fine. Pics of another PCB show D9801 marked on the flash, so it could be different.
	ROM_LOAD( "flash", 0x000000, 0x100000, BAD_DUMP CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s135n.2b", 0x000, 0x100, CRC(502be98c) SHA1(4591d1d5cfe9e83032705139e630dfa5df79689a) )
	ROM_LOAD( "n82s135n.2d", 0x100, 0x100, CRC(bb1865c9) SHA1(58acf909dd6de519d9675482d130b697856e1bf4) )
ROM_END

ROM_START( cb5_11) // Wing W4 board + DYNA D9701 subboard; DYNA CB5 V1.1 in bookkeeping screen.
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "cb5.11d", 0x020000, 0x20000, CRC(ea99dad0) SHA1(eaa899583b199db140dcc0fe750d388996b111a5) )

	ROM_REGION( 0x100000, "gfx", 0 ) // not dumped for this set, but seems to work fine. Pics of another PCB show D9801 marked on the flash, so it could be different.
	ROM_LOAD( "flash", 0x000000, 0x100000, BAD_DUMP CRC(07d711a6) SHA1(6b5a4017eb1d31dc184831f85d786331f4a8e01f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s135n.2b", 0x000, 0x100, CRC(502be98c) SHA1(4591d1d5cfe9e83032705139e630dfa5df79689a) )
	ROM_LOAD( "n82s135n.2d", 0x100, 0x100, CRC(bb1865c9) SHA1(58acf909dd6de519d9675482d130b697856e1bf4) )
ROM_END

/* New DongmulDongmul 2 (뉴 동물동물 2, New AnimalAnimal 2) runs on slightly different hardware, but with same CPU, custom and I/O.
   Video from the real hardware: https://youtu.be/1K9e_7RzeiM
   _______________________________________________________________________________
  |    ________    ________    ________    __________    _________________     _  |
  |KM6264BLG-10L  |74HC00P| KM6264BLG-10L |29F1610ML|   |  HM27C4096     |    (_) <- Switch (SW8)
  |                                       |__U15____|   |_____U24________|        |
  |  _______________               ___________                                    |
  | | 27C020       |              | DYNA     |         __U23_____  __________     |
  | |___U10________|              | DC3001   |        |N82S147AN| |74HC374AN|  ___|
  |                               |          |                                |
  | __       ___________          |__________|         __U22_____  __________ |___
  ||SW7     | DYNA     |                     Xtal     |N82S147AN| |74HC374AN|   __|
  ||Unused  | CPU 91A  |                   24.000 MHz  __________               __|
  ||__|     |          |                              |74LS245N_|               __|
  |         |__________|                  ___U8_____   __________               __|
  | ________      _________              |PALCE16V8|  |74LS245N_|               __|
  ||74HC04AN     |__SW6___|               ___U7_____   __________               __|
  |          _________                   |GAL16V8D_|  |74LS245N_|               __|
  |         |__SW1___|          ____________________   __________               __|
  |          _________         | DYNA 22A078803    |  |74LS138N_|               __|
  |         |__SW2___|         |___________________|   __________   __________  __|
  |          _________          ____________________  |74LS273N_|  |ULN2003AN|  __|
  |         |__SW3___|         | JFC 95101         |   __________               __|
  |          _________         |___________________|  |74LS273N_|             ____|
  |         |__SW4___|                                                        |
  |          _________                             Xtal                       |
  |         |__SW5___|   _______U1___________   12.000 MHz                    |___
  |         __________  | AT89C51           |                                     |
  | ___     |MACH 110|  |___________________|                                     |
  ||   |    |AMD     |    _________   _________                                   |
  |TDA2009  |________|   |74LS245N_| |74LS245N_|                                  |
  ||___|     _________    ____ ____                                               |
  |         |__SW0___|   PC817 PC817                                              |
  |_______________________________________________________________________________|

*/
ROM_START( ndongmul2 ) // 뉴 동물동물 2 (bootleg MIA 94V-0 PCB; DYNA PLUS V1.2N in bookkeeping screen - based on the Super Cherry Master Plus codebase)
	ROM_REGION16_LE( 0x080000, "boot_prg", 0 ) // CPU91A-011-9915JK001
	ROM_LOAD16_WORD( "am27c020.u10", 0x000000, 0x040000, CRC(550e53e5) SHA1(a90ee66e7ae9b58005b6ed412669d86532c75156) )

	ROM_REGION( 0x001000, "mcu", 0)
	ROM_LOAD( "at89c51.u1",          0x000000, 0x001000, NO_DUMP ) // AT89C51, protected

	ROM_REGION( 0x400000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "hn27c4096.u24",       0x000000, 0x080000, CRC(d6d14e2a) SHA1(ee6d663f7c31fb76fa56d080aa2cf1c690da61b8) )
	ROM_LOAD( "mx29f1610ml.u15",     0x080000, 0x200000, NO_DUMP ) // TODO: or possibly the EEPROM is mapped over this to show the Korean specific stuff?

	ROM_REGION( 0x000400, "proms", 0 )
	ROM_LOAD( "n82s147an.u22",       0x000000, 0x000200, CRC(54b76f79) SHA1(d8eca94fda3436a204e71869a88fba5fc4daed18) )
	ROM_LOAD( "n82s147an.u23",       0x000200, 0x000200, CRC(2e3063c8) SHA1(b1b3d23063faabe7f588dfafe4a1439573d41cb4) )

	ROM_REGION( 0x000200, "plds", 0)
	ROM_LOAD( "palce16v8h-25.u8",    0x000000, 0x000117, CRC(f1de9b90) SHA1(3b2e76e1f6dc34d16fa1dded9bc8205683e59c0c) )
	ROM_LOAD( "gal16v8d.u7",         0x000000, 0x000117, CRC(55e39258) SHA1(4546fdbd343290c2a7953b4cd0f8db5aab2fad18) )
ROM_END

ROM_START( mystjb ) // DYNA D9702 PCB; DYNA MYST V1.3G in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD( "mjb_3g.11f", 0x20000, 0x20000, CRC(b67d1311) SHA1(7694bee009c5720dde65226ec19544c5e6e5077e) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "mjb_1g.12b", 0x00000, 0x80000, CRC(00244991) SHA1(952095011ff766018b9ad310afc9dbd3a4c8f5a8) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(777d4f6e) SHA1(6825514676ff1c2a2bef7c67819787d698eb32bf) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(ee087df2) SHA1(b60a1097e997459e788dcfafe15237c95a55bbed) )
ROM_END

ROM_START( tripjack ) // DYNA D9805 PCB; DYNA TRJ V1.6G in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "27c2001.11f", 0x00000, 0x40000, CRC(f7b6226a) SHA1(f01329eff728547c369075a1bc0c2455438499a5) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "27c4002.12b", 0x00000, 0x80000, CRC(5b6221a9) SHA1(b777cc4aace17969d0357ba5e8c75c9f8b596da4) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s135.9b",  0x000, 0x100, CRC(2d2237fb) SHA1(9b71801bd465d2a823f648f4d3c1823b5ba3340e) )
	ROM_LOAD( "82s135.11b", 0x100, 0x100, CRC(9940ef22) SHA1(42b0c6410d8db34e0316e95b7b7007abc3098341) )
ROM_END

ROM_START( crzybell ) // DYNA D9401 PCB; DYNA CRBL1 V1.2D in bookkeeping screen
	ROM_REGION16_LE( 0x40000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "cbl12d.6e", 0x20000, 0x20000, CRC(a5d43b00) SHA1(b89fd8eb7675b3dd4f47bf95326da1b881997a56) )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "cbl1d.2g", 0x00000, 0x80000, CRC(83c6a91f) SHA1(3f802777e4a5581f7efe86309992f0e7a79851ee) )

	ROM_REGION( 0x400, "proms", ROMREGION_ERASE00 ) // not dumped yet
	ROM_LOAD( "82s135.j5", 0x000, 0x100, BAD_DUMP CRC(2d2237fb) SHA1(9b71801bd465d2a823f648f4d3c1823b5ba3340e) )
	ROM_LOAD( "82s135.k5", 0x100, 0x100, BAD_DUMP CRC(9940ef22) SHA1(42b0c6410d8db34e0316e95b7b7007abc3098341) )
ROM_END

ROM_START( nmondop ) // DYNA D9702 PCB; DYNA NMP V0.6I in bookkeeping screen
	ROM_REGION16_LE( 0x040000, "boot_prg", 0 )
	ROM_LOAD16_WORD( "nmp12i.11f", 0x000000, 0x40000, CRC(a02d70f9) SHA1(26de0e09432fa5cccef502553bc6d65ec179c7a4) )

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "nmp1i.12a", 0x000000, 0x80000, CRC(291ca4d1) SHA1(404439c0e73098e253160af1d36f7ceb7f98f49d) )

	ROM_REGION( 0x400, "proms", 0 ) // not dumped yet
	ROM_LOAD( "82s147.9b",  0x000, 0x200, BAD_DUMP CRC(6c90f6a2) SHA1(f3f592954000d189ded0ed8c6c4444ace0b616a4) )
	ROM_LOAD( "82s147.11b", 0x200, 0x200, BAD_DUMP CRC(e5aa3ec7) SHA1(675711dd6788b3d0c37573b49b6297cbcd8c8209) )
ROM_END


void cb2001_state::init_smaller_proms()
{
	uint8_t *proms = memregion("proms")->base();
	std::vector<uint8_t> buffer(0x400);
	memcpy(&buffer[0], proms, 0x400);

	for (int i = 0; i < 0x400; i++)
	{
		if (!(i & 0x20))
			proms[i] = buffer[(i & 0x1f) | ((i & 0x3c0) >> 1)];
		else
			proms[i] = 0x00;
	}

	m_palette->update();
}

} // anonymous namespace


//    YEAR  NAME          PARENT     MACHINE    INPUT      CLASS         INIT                ROT   COMPANY  FULLNAME                            FLAGS
GAME( 2000, cb2001,       0,         cb2001,    cb2001,    cb2001_state, empty_init,         ROT0, "Dyna",  "Cherry Bonus 2001 (V1.1I)",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, ndongmul2,    0,         ndongmul2, ndongmul2, cb2001_state, empty_init,         ROT0, "Dyna",  "New DongmulDongmul 2 (V1.2N)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // goes into the weeds at various point, due to either missing MCU dump or incomplete decryption. Bad reels GFX.
GAME( 1992, dynastye ,    0,         scherrym,  cb2001,    cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Dynasty (1992, V5.1G)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, scherrym ,    0,         scherrym,  cb2001,    cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Super Cherry Master (V5.2)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, scherrym12 ,  scherrym,  scherrym,  cb2001,    cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Super Cherry Master (V1.2)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, scherrymp,    0,         scherrymp, scherrymp, cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Super Cherry Master Plus (V1.6)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, scherrymp10u, scherrymp, scherrymp, scherrymp, cb2001_state, empty_init,         ROT0, "Dyna",  "Super Cherry Master Plus (V1.0U)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, cb4,          0,         cb5,       cb5,       cb2001_state, empty_init,         ROT0, "Dyna",  "Cherry Bonus IV (V5.0)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, cb4a,         cb4,       cb5,       cb5,       cb2001_state, empty_init,         ROT0, "Dyna",  "Cherry Bonus IV (V1.2)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, cb5,          0,         cb5,       cb5,       cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Cherry Bonus V Five (V1.3)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1997, cb5_11,       cb5,       cb5,       cb5,       cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Cherry Bonus V Five (V1.1)",       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, mystjb,       0,         scherrymp, scherrymp, cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Mystery J & B (V1.3G)",            MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, tripjack,     0,         scherrymp, scherrymp, cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Triple Jack (V1.6G)",              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, crzybell,     0,         scherrym,  cb2001,    cb2001_state, init_smaller_proms, ROT0, "Dyna",  "Crazy Bell (V1.2D)",               MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, nmondop,      0,         cb2001,    cb2001,    cb2001_state, empty_init,         ROT0, "Dyna",  "New Mondo Plus (V0.6I)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
