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

*************************************************************************************************/

#include "driver.h"
#include "cpu/nec/nec.h"
#include "sound/ay8910.h"
#include "machine/8255ppi.h"

#define xxxx 0x90 /* Unknown */

static const UINT8 cb2001_decryption_table[256] = {
	0xe8,xxxx,0x94,xxxx,0x80,0x61,0x12,xxxx, 0x3c,xxxx,xxxx,0x23,xxxx,xxxx,xxxx,xxxx, /* 00 */
//    pppp      ????      pppp ???? pppp       pppp           p?p?
	xxxx,xxxx,xxxx,0x27,0x1c,xxxx,xxxx,xxxx, 0x32,xxxx,0xa0,0xd3,0x3a,0x14,0x89,xxxx, /* 10 */
//                   pppp pppp                 pppp      ???? pppp pppp pppp pppp ????
	xxxx,0x8e,xxxx,0x0f,xxxx,0x49,0xb5,xxxx, xxxx,xxxx,xxxx,0x75,xxxx,xxxx,xxxx,xxxx, /* 20 */
//         !!!!      pppp      ???? pppp                      pppp
	0x9d,xxxx,xxxx,xxxx,xxxx,xxxx,0xbe,xxxx, xxxx,xxxx,0x74,xxxx,xxxx,0xa6,0xbf,0x74, /* 30 */
//    ????                          pppp                 ????           ???? ???? pppp
	xxxx,0xea,xxxx,xxxx,xxxx,0xb0,xxxx,xxxx, xxxx,0xa2,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* 40 */
//         !!!!                gggg                 pppp
	xxxx,xxxx,0x2c,xxxx,xxxx,xxxx,0x42,0xc0, xxxx,xxxx,xxxx,xxxx,0xeb,xxxx,xxxx,xxxx, /* 50 */
//              pppp                ???? pppp                      pppp
	xxxx,xxxx,xxxx,xxxx,0x22,xxxx,xxxx,xxxx, xxxx,0xa5,xxxx,xxxx,xxxx,0xbb,0xba,xxxx, /* 60 */
//                        pppp                      ????                pppp gggg
	0xc3,xxxx,0x02,xxxx,xxxx,xxxx,0x24,xxxx, 0x72,xxxx,0xf2,xxxx,xxxx,0x43,xxxx,xxxx, /* 70 */
//    pppp      pppp                pppp       pppp      ????           pppp
	xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x34, xxxx,xxxx,0x59,xxxx,0x73,xxxx,0x2a,xxxx, /* 80 */
//                                       ????            pppp      pppp      ????
	xxxx,xxxx,0xe9,xxxx,xxxx,0xbe,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,0xb9,xxxx,xxxx,xxxx, /* 90 */
//              ????           p?p?                                pppp
	xxxx,xxxx,xxxx,0x06,0xaa,0x9c,xxxx,0xb8, xxxx,xxxx,0xfc,xxxx,0x51,xxxx,xxxx,0x1a, /* A0 */
//                   ???? ???? ????      !!!!            ????      pppp           pppp
	0x75,xxxx,0xb4,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,0x03,xxxx,xxxx,xxxx,0x07,0xcf, /* B0 */
//    ????      pppp                                     pppp      ????      ???? ????
	xxxx,0xec,0xee,xxxx,xxxx,0xe2,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,0x2e,xxxx, /* C0 */
//         pppp ????           pppp                                          pppp
	xxxx,xxxx,0x46,xxxx,0x60,xxxx,xxxx,0x47, 0x88,xxxx,xxxx,xxxx,xxxx,0xfa,0xc7,0x8b, /* D0 */
//              pppp      ????           pppp  pppp                     ???? !!!! pppp
	0x8a,xxxx,xxxx,0xc6,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* E0 */
//    pppp           !!!!
	xxxx,xxxx,0xfe,xxxx,xxxx,xxxx,xxxx,xxxx, xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx,xxxx, /* F0 */
//              pppp
};

/* robiza's notes:

cmast91 and cmv4 seems similar to this cb2001:

cmast91:                      cb2001:
0067 ld a,$07                 e0130 mov al,7h
0069 out ($23),a              e0132 mov dw,23h
                              e0135 out dw,al

006b ld a,$3f                 e0136 mov al,3fh
006d out ($22),a              e0138 mov dw,22h
                              e013b out dw,al

006f ld a,$9b                 e0124 mov al,0ffh
0071 out ($13),a              e0126 mov dw,13h
                              e0129 out dw,al

-------------------------------------------------

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
02a7 call $2b2d               e0240 call 0e32a6h
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

  2b40-2b4b                     e32bf-e32d1              (06 -> 12) (d7 -> 4f dec iy or 47 inc iy)
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
  4ab1 ex de,hl                 e66f2 mov bw,w ptr ss[bw](df -> 8b)prefix not sure about prefix
  4ab2 xor a                    .
  4ab3 ld ($d618),a             e66f5 mov b ptr[72dh],ah (d8 -> 88)
  4ab6 ld ($d619),a             e66f9 mov b ptr[72eh],ah
  4ab9 ld ($d61a),hl            e66fd mov w ptr[72fh],bw (1e -> 89)
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
  4acc ld a,(hl)                e671b mov al,b ptr ps[bw](ce -> 2e) (e0 -> 8a) not sure about the prefix
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

56 -> ????

5c -> conditional jmp for sure

aa -> ????
92 -> e9 (probably)
1c ????
d8 ????
dd -> fa (di)

guessed:
45 -> b0 (mov al,#value)
6e -> ba (mov dw,#value)
c2 -> ee (out dw,al)

probably:

2b -> conditional jmp for sure (75)
36 -> be
9c -> it's a counter (like mov cw,#value) -> not sure the register (cw,bw,....) -> b9 (cw)
c5 -> 75 (loop?)

very probably:
00 -> e8 (call)
41 -> ea (jmp_far)
70 -> c3 (ret)

checked against gussun and quizf1 (start up code):
21 -> 8e
a7 -> b8
de -> c7
e3 -> c6

opcodes: 36,9c,00,18,d8,d2,c5,70 probably:
e1af1 36 62 06 mov ix,0662
      9c 04 00 mov cw,0004
      00 94 17 call e328e

e328e 18 c0 xor al,al
      d8 04 mov byte ptr [ix],al
      d2    inc ix
      c5 fb dbnz e3290
      70    ret
*/


static VIDEO_START(cb2001)
{

}

static VIDEO_UPDATE(cb2001)
{
	return 0;
}

static ADDRESS_MAP_START( cb2001_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0xbffff) AM_RAM
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cb2001_io, ADDRESS_SPACE_IO, 16 )
//  ADDRESS_MAP_GLOBAL_MASK(0x00ff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE8("ppi8255_0", ppi8255_r, ppi8255_w, 0xffff)	/* Input Ports */
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE8("ppi8255_1", ppi8255_r, ppi8255_w, 0xffff)	/* DIP switches */
	AM_RANGE(0x20, 0x21) AM_DEVREAD8("aysnd", ay8910_r, 0x00ff)
	AM_RANGE(0x22, 0x23) AM_DEVWRITE8("aysnd", ay8910_data_address_w, 0xffff)
ADDRESS_MAP_END

static INPUT_PORTS_START( cb2001 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop 2 / Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop 1 / D-UP")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL ) PORT_NAME("Stop All / Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop 3 / Small / Info")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)	/* Coin B */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(2)	/* Coin D */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)	/* Coin C */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)	/* Coin A */

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
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )	PORT_DIPLOCATION("DSW1:1")	/* OK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out Switch" )	PORT_DIPLOCATION("DSW1:2")	/* OK */
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Mode" )		PORT_DIPLOCATION("DSW1:3")	/* OK */
	PORT_DIPSETTING(    0x04, "Payout Switch" )
	PORT_DIPSETTING(    0x00, "Automatic" )
	PORT_DIPNAME( 0x08, 0x00, "W-UP '7'" )			PORT_DIPLOCATION("DSW1:4")	/* not checked */
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x00, "W-UP Pay Rate" )		PORT_DIPLOCATION("DSW1:5")	/* OK */
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "W-UP Game" )			PORT_DIPLOCATION("DSW1:6")	/* OK */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bet Max" )			PORT_DIPLOCATION("DSW1:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPSETTING(    0x40, "32" )
	PORT_DIPSETTING(    0x80, "64" )
	PORT_DIPSETTING(    0xc0, "96" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )	PORT_DIPLOCATION("DSW2:1,2,3")	/* OK */
	PORT_DIPSETTING(    0x07, "35%" )
	PORT_DIPSETTING(    0x06, "40%" )
	PORT_DIPSETTING(    0x05, "45%" )
	PORT_DIPSETTING(    0x04, "50%" )
	PORT_DIPSETTING(    0x03, "55%" )
	PORT_DIPSETTING(    0x02, "60%" )
	PORT_DIPSETTING(    0x01, "65%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )			PORT_DIPLOCATION("DSW2:4,5")	/* OK */
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )		PORT_DIPLOCATION("DSW2:6")	/* not checked */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Key-In Type" )			PORT_DIPLOCATION("DSW2:7")	/* OK */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )	PORT_DIPLOCATION("DSW2:8")	/* related with DSW 4-6 */
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("DSW3:1,2")	/* OK */
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) /* A-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )     PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) /* B-Type */
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" )  PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("DSW3:3,4")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("DSW3:5,6")	/* OK */
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) /* C-Type */
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) /* D-Type */
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("DSW3:7,8")	/* OK */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Credit Limit" )            PORT_DIPLOCATION("DSW4:1,2,3")	/* not checked */
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
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" )          PORT_DIPLOCATION("DSW4:5")	/* OK */
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("DSW4:6")	/* OK */
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" )              PORT_DIPLOCATION("DSW4:7")	/* OK */
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" )    PORT_DIPLOCATION("DSW4:8")	/* not checked */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x00, "Display Of Doll On Demo" )          PORT_DIPLOCATION("DSW5:1")	/* not working */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" )                    PORT_DIPLOCATION("DSW5:2,3")	/* not checked */
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" )    PORT_DIPLOCATION("DSW5:4,5")	/* not checked */
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x00, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("DSW5:6")	/* not checked */
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )                 PORT_DIPLOCATION("DSW5:7")	/* listed as unused */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" )      PORT_DIPLOCATION("DSW5:8")	/* not working */
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INTERRUPT_GEN( vblank_irq )
{
//  cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x08/4);
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
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout,   0x0, 32  )
	GFXDECODE_ENTRY( "gfx", 0, cb2001_layout32, 0x0, 32 )
GFXDECODE_END

static PALETTE_INIT(cb2001)
{
	int i;
	for (i = 0; i < 0x200; i++)
	{
		int r,g,b;

		UINT8*proms = memory_region(machine, "proms");
		UINT16 dat;

		dat = (proms[0x000+i] << 8) | proms[0x200+i];


		b = ((dat >> 1) & 0x1f)<<3;
		r = ((dat >> 6 )& 0x1f)<<3;
		g = ((dat >> 11 ) & 0x1f)<<3;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static const ppi8255_interface cb2001_ppi8255_intf[2] =
{
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("IN0"),	/* Port A read */
		DEVCB_INPUT_PORT("IN1"),	/* Port B read */
		DEVCB_INPUT_PORT("IN2"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	},
	{	/* A, B & C set as input */
		DEVCB_INPUT_PORT("DSW1"),	/* Port A read */
		DEVCB_INPUT_PORT("DSW2"),	/* Port B read */
		DEVCB_INPUT_PORT("DSW3"),	/* Port C read */
		DEVCB_NULL,					/* Port A write */
		DEVCB_NULL,					/* Port B write */
		DEVCB_NULL					/* Port C write */
	}
};

static const ay8910_interface cb2001_ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_INPUT_PORT("DSW4"),
	DEVCB_INPUT_PORT("DSW5"),
	DEVCB_NULL,
	DEVCB_NULL
};

static const nec_config cb2001_config = { cb2001_decryption_table, };
static MACHINE_DRIVER_START( cb2001 )
	MDRV_CPU_ADD("maincpu", V30, 20000000) // CPU91A-011-0016JK004; encrypted cpu like nec v25/35 used in some irem game
	MDRV_CPU_CONFIG(cb2001_config)
	MDRV_CPU_PROGRAM_MAP(cb2001_map)
	MDRV_CPU_IO_MAP(cb2001_io)
	MDRV_CPU_VBLANK_INT("screen", vblank_irq)

	MDRV_PPI8255_ADD( "ppi8255_0", cb2001_ppi8255_intf[0] )
	MDRV_PPI8255_ADD( "ppi8255_1", cb2001_ppi8255_intf[1] )

	MDRV_GFXDECODE(cb2001)

	MDRV_PALETTE_INIT( cb2001 )

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 64*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(cb2001)
	MDRV_VIDEO_UPDATE(cb2001)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, 1500000) // wrong
	MDRV_SOUND_CONFIG(cb2001_ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END


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

	ROM_REGION( 0x080000, "gfx", 0 )
	ROM_LOAD( "12a.bin", 0x000000, 0x80000,NO_DUMP ) // missing on PCB

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "n82s135-1.bin", 0x000, 0x100, CRC(66ed363f) SHA1(65bd37842c441c2e712844b07c0cfe37ef16d0ef) )
	ROM_LOAD( "n82s135-2.bin", 0x200, 0x100, CRC(a19821db) SHA1(62dda90dd67dfbc0b96f161f1f2b7a46a5805eae) )
ROM_END

GAME( 2001, cb2001,    0,      cb2001,      cb2001,   0, ROT0,  "Dyna", "Cherry Bonus 2001", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 2001, scherrym,  0,      cb2001,      cb2001,   0, ROT0,  "Dyna", "Super Cherry Master", GAME_NOT_WORKING|GAME_NO_SOUND ) // 2001 version? (we have bootlegs running on z80 hw of a 1996 version)

