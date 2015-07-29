// license:BSD-3-Clause
// copyright-holders:David Haywood, Tomasz Slanina
/*
  Dwarfs Den by Electro-Sport
  http://www.arcadeflyers.net/?page=flyerdb&subpage=thumbs&id=3993

  driver by
   David Haywood
   Tomasz Slanina

 TODO:

 - convert driver to use i8275 CRT device emulation
 - fix gfx glitches and banking
 - correct colors
 - DIPs
 - layout(lamps)
 - NVRAM

Notes: at least on dwarf's den hardware, the current card set/dwarf set can be swapped
by pushing the buttons in this order: 1-4-1-4-2-1-3-5 ('zvzvxzcb' in mame)
Doing this with the default dipswitches results in an error; The code needs to be investigated.
Setting all dipswitches active and dipswitch 10 inactive allows game/gfx switching to work, but gfx
are all corrupt/scrambled.
Also the dipswitches are active low/inverted, since on the machine this was tested on dipswitches 2,3,5,6,8 were ON, and 1,4 and 7 were OFF.
There are only 8 dipswitches populated on the Dwarf's den boards seen so far.



DD hardware is weird and complicated. There's no dedicated DMA controller
for CRT.  When serial output pin of cpu is in high state, all reads from
work RAM are redirected to CRT:

005B: 3E C0      mvi  a,$c0
005D: 30         sim
005E: E1         pop  h ; 40 times "pop h" = transfer 80 bytes (line) from ram to crt fifo
...
0087: 0F         rrc
0088: 30         sim

$4c00-$4fff part of work RAM is addressed in different way than other blocks. No idea why
(this part of the pcb is a bit damaged).
Gfx decode is strange - may be wrong.
Gfx rom bank (6d/6b or 6a/6c pair) is switched by timer circuit - few 74161 counters connected to xtal.
Current implementations is just a guess, and doesn't work on test screen.


  __________________________________________   ___________   _______________________________________
  |         1         2         3         4 |_|||||||||||||_| 6         7         8         9      |
  |                                                                                      ________  |
  ||| L                                                   SN7445N74  74LS224N  7404-PC   |9L    |  |
D |||                                                                                    |______|  |
  |||                                                                   _____________    ________  |
  ||| K  ULN2803                BATTERY         16-1-471  7400-PC       | M5L8085AP |    |9K    |  |
  |                                                                     |___________|    |______|  |
  |                                                                                      ________  |
  ||| J  ULN2803  74LS273NA                     MC14011   MC14011              74LS373N  |9J    |  |
C |||                                                                                    |______|  |
  |||                                                                _______             ________  |
  ||| H  ULN2803  74LS273NA  SN7442AN  74107N   74161N    SN7432N    |7H   |   MDP1603   |9H    |  |
  |                                                                                      |______|  |
  |                                                                                                |
  ||| F  ULN2803  74LS273NA  7408N     7486N    OSC       7404                 MM2114N   MM2114N   |
P |||                                                                                              |
  |||                                  _____________                                               |
  ||| E                      SN7442AN  |iP8275     |      7400       74LS244N  MM2114N   MM2114N   |
  |                                    |___________|                                               |
  |                                    _____________      ________                                 |
  ||| D 16-1-471  MDP1603    7414      |AY-3-8910  |      |6D    |   74LS245   MM2114N   MM2114N   |
B |||                                  |___________|      |______|                                 |
  |||                                                     ________   _______                       |
  ||| C 16-1-471  MDP1603    7414      7414     7400      |6C    |   |7C   |   M3-6514-9 M3-6514-9 |
  |                                                       |______|                                 |
  |                                                       ________   __________________            |
  ||| B SN74175N             74174N    74LS374N 74LS374N  |6B    |   |        (74LS257N) 6-1-471   |
A |||                                                     |______|   |                |            |
  |||                        _______                      ________   |    ______      |            |
  ||| A DN74505N             |3A   |   74153N   74153N    |6A    |   |    LM324N      |  DIP-SW    |
  |                                                       |______|   |                |            |
  |                                                                  |________________|            |
  |________________________________________________________________________________________________|

  OSC = 10.595 MHz
  A = 20-pin connector for video (r,g,b,hsync,vsync,csync on pins 5,7,9,11,13,15, the even pins are gnd, others are n/c)
  B = 26-pin connector for bet/hold buttons and button lights (connects to one of the darlingtons)
  P = 12-pin connector for power, see pinout below
  C = 20-pin connector for win lamps (connects to darlingtons at 1J and 1K)
  D = 20-pin connector for the 3 coin slots, the test switch and service switch, as well as lockout coil and GI lamps
  Edge connector (top) is not JAMMA (connects to cpu pins; debug connector?)
  3A = (63S080N or 74S188) dumped as 82S(1)23
  7C = non populated on original Dwarf's den, would be a prom if present.
  7H = (M3-7602-5 or 74S188) dumped as 82S(1)23

  Power connector pinout:
  ...-------------pcb edge----------------...
         1 2 3 4 5 6 7 8 9 10 11 12
1,2 = +5VDC
3,4 = LAMP POWER A and B (tied together, only used by the four ULN2803 Darlington Arrays and connectors C and D)
5,6 = N/C
7 = SENSE/RESET (PSU puts +20VDC? here, goes to darlington at 1F)
8,10 = GND
9 = AUDIO OUTPUT (the audio amplifier is part of the PSU)
11,12 = LAMP GROUND (tied together, only used by the four ULN2803 Darlington Arrays and connectors C and indirectly D)


=====================================================================================================
(quarterh, quarterhb)

----------------------------------------
Quarterhorse by Electro-Sport Inc (1983)
----------------------------------------

Dumped by Ruben Panossian




Location    Device         File ID            Checksum
------------------------------------------------------
CPU 9L       2732    9l_qh_10-2402_7193.bin     5B36    [ CPU ROM 1 Disc No.* ]
CPU 9K       2732    9k_qh_10-2401_8193.bin     0EE6    [ CPU ROM 2 Disc No.* ]
CPU 9J       2732    9j_qh_10-2400_9193.bin     6119    [ CPU ROM 3 Disc No.* ]
CPU 9H       2732    9h_qh_50-2399_10193.bin    8D7C    [ CPU ROM 4 Disc No.* ]

CPU 9L       2732    9l_qh_60-2402_7193.bin     8731    [ CPU ROM 1 Disc No.* ]
CPU 9K       2732    9k_qh_60-2401_8193.bin     3C36    [ CPU ROM 2 Disc No.* ]
CPU 9J       2732    9j_qh_60-2400_9193.bin     5165    [ CPU ROM 3 Disc No.* ]
CPU 9H       2732    9h_qh_80-2399_10193.bin    DB83    [ CPU ROM 4 Disc No.* ]

CPU 6D       2716    6d_qh_01-2398_11193.bin    9AA3    [    Character B2     ]
CPU 6C       2716    6c_qh_01-2397_12193.bin    33C3    [    Character A2     ]
CPU 6B       2716    6b_qh_01-2396_13193.bin    A95A    [    Character B1     ]
CPU 6A       2716    6a_qh_01-2395_14193.bin    43C0    [    Character A1     ]
CPU 3A      82S23    3a_bprom.bin               1005
CPU 7H      82S23    7H_bprom.bin               1EB4
LDP 2D       2716    -G_L-.bin                  0699    [      LDP Ctrl       ]


* Disc ID: Rodessch & Associates, INC. Model QAB, Serial 01292 Pioneer Double sided
  Disc ID: Rodessch & Associates, INC. Model QAA, Serial 00857 Pioneer Double sided



Notes:  CPU - Main PCB                     QH Processor Board 5/5/83    13-637 B
        LDP - LDP CTRL, Audio Input PCB    LDP Control                  13-909 A
        VID - Video overlay PCB            ESI Video Switcher PCB       13-2321
        DEC - NTSC Decoder PCB             ESI NTSC Decoder BD          13-2133
        VLT - Voltage Monitor              ESI Voltage/Static Monitor   13-1586

        Uses either a Pioneer PR-8210 or Magnavox VC-8010 Laserdisc player
        The laserdisc player is modified - has a custom communication & audio cable



Brief Hardware Overview
-----------------------

CPU 5F  Y1            10.595MHz Xtal
CPU 7K  uPD8085AC     8085 CPU
CPU 4E  P8275         Prog CRT Controller, Video Output Graphics Controller
CPU 4D  AY-3-8910     Programmable Sound Generator
CPU 9F  2114          1024x4 SRAM
CPU 9E  2114          1024x4 SRAM
CPU 9D  2114          1024x4 SRAM
CPU 9C  M3-6514C      1024x4 CMOS SRAM, Battery Backed
CPU 8F  2114          1024x4 SRAM
CPU 8E  2114          1024x4 SRAM
CPU 8D  2114          1024x4 SRAM
CPU 8C  M3-6514C      1024x4 CMOS SRAM, Battery Backed

=====================================================================================
(quarterhb)

----------------------------------------
Quarterhorse by Electro-Sport Inc (1983)
----------------------------------------

Dumped by Grull Osgo

Location    Device   File ID    Checksum
----------------------------------------
CPU 10A      27C128  a1.bin     9f26    [ CPU ROM   ]

CPU 6H       27C128  a2.bin     7ccc    [ Character ]
CPU 6K       27C128  a3.bin     ba34    [ Character ]

LDP O7       27c128  a4.bin     2120    [ LDP Ctrl  ] (Add-On Board "7-50A" on O7-O9-010 IC Sockets)


Notes:  CPU - Main Board includes NTSC decoder, video Switch & Audio control.
    No Model or Serial number on PCB.
        Uses a Pioneer LD-V2000 Laserdisc player.
        The laserdisc player is modified - has a custom communication & audio cable (10 Wires Flat Cable).

Brief Hardware Overview
-----------------------

CPU 5E  X-TAL         10.00MHz Xtal
CPU 8B  M5L8085AC     8085 CPU
CPU 4F  P8275         Prog CRT Controller, Video Output Graphics Controller
CPU 4G  AY-3-8910     Programmable Sound Generator
CPU 10B TMM2016       2048x8 SRAM, Battery Backed
CPU 10D UM6116        2048X8 SRAM
CPU 3K  82S123        32X8   TTL PROM
CPU 4N  uPC1325       NTSC Decoder
CPU 3A  NEC C1182H    Audio Amplifier
CPU 10K Dip-SW        8 x Dip Switch bank.


Quarter Horse LD (Pioneer)

It has oly one side recorded. The other side has a video with a single slide ad. that says
"The recorded  material is on the other side"

Disk 1
------
Sticker:

    09-251  A

    HORSE RACE I

    QUARTER HORSE

    (C) 1981 DALE FRANK RODESCH
    SAN DIEGO, CA USA

Stamp on disk:

    09-251A1-15



Disk 2
------
Sticker:


    09251   1

    QUARTER HORSE

    VIDEO DISK

    (C) 1981 DALE FRANK RODESCH


Stamp on disk:

    09-251A1-01

    09-251    A


Disk 1
------
Sticker:

    09-251  A

    HORSE RACE I

    QUARTER HORSE

    (C) 1981 DALE FRANK RODESCH

Stamp on disk:

    09-251A1-06


=====================================================================================
Quater Horse Classic

Board silkscreend   QUARTERHORSE
            (C) ARJAY EXPORT CO.,INC/PRESTIGE GAMES> -1995 REV-C-95
            HYANNIS,MA.02601--MADE IN USA

.A10 - 27256    stickered   QUARTERHORSE CLASSIC
                10A - V1.1
                (c)1995 ARJAY EXPORT - PRESTIGE GAMES


.H6 - 27128 stickered   QUARTERHORSE CLASSIC
                CGR - 6H - V1.0
                (c)1995 ARJAY EXPORT - PRESTIGE GAMES

.K6 - 27128 stickered   QUARTERHORSE CLASSIC
                CGR - 6K - V1.0
                (c)1995 ARJAY EXPORT - PRESTIGE GAMES
.U9 - Lattice GAL22V10  on daughter board

P8085 @ B8
P8275 @ F4
AY8910 @ G4
uPC1352C @ N3
6116 @ B10 and C10

*/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"
#include "video/i8275.h"

class dwarfd_state : public driver_device
{
public:
	dwarfd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_palette(*this, "palette"),
		m_crtc(*this, "i8275"),
		m_charmap(*this, "gfx1"),
		m_dsw2(*this, "DSW2")
		{ }

	/* video-related */
	int m_crt_access;
	bool m_back_color;

	/* memory */
	UINT8    m_dw_ram[0x1000];

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<i8275_device> m_crtc;
	required_region_ptr<UINT16> m_charmap;
	required_ioport m_dsw2;

	DECLARE_READ8_MEMBER(dwarfd_ram_r);
	DECLARE_WRITE8_MEMBER(dwarfd_ram_w);
	DECLARE_WRITE8_MEMBER(output1_w);
	DECLARE_WRITE8_MEMBER(output2_w);
	DECLARE_READ8_MEMBER(qc_b8_r);
	DECLARE_WRITE_LINE_MEMBER(dwarfd_sod_callback);
	DECLARE_WRITE_LINE_MEMBER(drq_w);
	DECLARE_DRIVER_INIT(qc);
	DECLARE_DRIVER_INIT(dwarfd);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_PALETTE_INIT(dwarfd);
	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	I8275_DRAW_CHARACTER_MEMBER(qc_display_pixels);
};


READ8_MEMBER(dwarfd_state::dwarfd_ram_r)
{
	if (m_crt_access == 0)
	{
		return m_dw_ram[offset];
	}
	else
	{
		m_crtc->dack_w(space, 0, m_dw_ram[offset], mem_mask);
		return m_dw_ram[offset];
	}
}

WRITE8_MEMBER(dwarfd_state::dwarfd_ram_w)
{
	m_dw_ram[offset] = data;
}

WRITE8_MEMBER(dwarfd_state::output1_w)
{
/*
 bits:
     0 - pp lamp
     1 - start lamp
     2 - ?
     3 - ?
     4 - unzap lamp
     5 - z1 lamp
     6 - z2 lamp
     7 - z3 lamp
*/
}

WRITE8_MEMBER(dwarfd_state::output2_w)
{
/*
 bits:
     0 - z4
     1 - z5
     2 - coin counter?
     3 - ?
     4 - ?
     5 - ?
     6 - ?
     7 - ?
*/
}


READ8_MEMBER(dwarfd_state::qc_b8_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, dwarfd_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x4fff) AM_READWRITE(dwarfd_ram_r, dwarfd_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, dwarfd_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)

	AM_RANGE(0x20, 0x21) AM_DEVREADWRITE("i8275", i8275_device, read, write)
	AM_RANGE(0x40, 0x40) AM_WRITENOP // unknown
	AM_RANGE(0x60, 0x60) AM_WRITE(output1_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(output2_w)
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("DSW1")
	AM_RANGE(0xc1, 0xc1) AM_READ_PORT("DSW2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( qc_map, AS_PROGRAM, 8, dwarfd_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_READWRITE(dwarfd_ram_r, dwarfd_ram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( qc_io_map, AS_IO, 8, dwarfd_state )
	AM_IMPORT_FROM( io_map )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xb8, 0xb8) AM_READ(qc_b8_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( dwarfd )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, "Games Per Coin for slots 1/2/3" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "2/2/8" )
	PORT_DIPSETTING(    0x02, "1/2/4" )
	PORT_DIPSETTING(    0x01, "1/1/4" )
	PORT_DIPSETTING(    0x00, ".5/.5/2" )
	PORT_DIPNAME( 0x04, 0x00, "Multiple Coins Accepted Per Game" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Operator Settings Mode 1/2" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Changes Allowed/Preset #2" )
	PORT_DIPSETTING(    0x00, "Changes Locked/Preset #1" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/*PORT_DIPNAME( 0x18, 0x00, "Operator Settings Mode" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "Changes Allowed" )
	PORT_DIPSETTING(    0x10, "Preset #1" )
	PORT_DIPSETTING(    0x08, "Preset #2" )
	PORT_DIPSETTING(    0x00, "Changes locked" )
	PORT_DIPNAME( 0x20, 0x00, "Dwarf's Den Gfx" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Large Character Gfx" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Full Card Gfx" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )*/

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Operator Settings Mode 2/2" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x01, "Changes Allowed/Preset #1" )
	PORT_DIPSETTING(    0x00, "Changes Locked/Preset #2" )
	// note for these: dwarfd should have switch 6 and 8 on, dwarfda should have 7 and 8 on
	PORT_DIPNAME( 0x02, 0x00, "Dwarf's Den Gfx" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Large Character Gfx" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Full Card Gfx" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Zap 1") //z1 zap 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Zap 2") //z2 zap 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Zap 3") //z3 zap 3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_NAME("Zap 4") //z4 zap 4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) PORT_NAME("Zap 5") //z5 zap 5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	//PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_SERVICE( 0x40, IP_ACTIVE_HIGH )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) //PORT_NAME("Play Credit") PORT_CODE(KEYCODE_A) //pp (play credit)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) // (deal)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_DEAL ) //PORT_NAME("Replace") PORT_CODE(KEYCODE_F) //rp replace (draw)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) //tk take (stand)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Unzap") //uz unzap (cancel)
INPUT_PORTS_END

static INPUT_PORTS_START( quarterh )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Zap 1") //z1 zap 1
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Zap 2") //z2 zap 2
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Zap 3") //z3 zap 3
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_NAME("Zap 4") //z4 zap 4
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 ) PORT_NAME("Zap 5") //z5 zap 5
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_BET ) PORT_NAME("Bet x1") //x1 bet x1
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_D_UP ) PORT_NAME("Bet x2") //x2 bet x2

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("PP ?") PORT_CODE(KEYCODE_A) //pp
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Replace") PORT_CODE(KEYCODE_F) //rp replace
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) //tk take
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Unzap") //uz unzap
INPUT_PORTS_END


I8275_DRAW_CHARACTER_MEMBER(dwarfd_state::display_pixels)
{
	int i;
	int bank = ((gpa & 2) ? 0 : 4) + (gpa & 1) + ((m_dsw2->read() & 4) >> 1);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT16 pixels = m_charmap[(linecount & 7) + ((charcode + (bank * 128)) << 3)];
	if(!x)
		m_back_color = false;

	//if(!linecount)
	//  logerror("%d %d %02x %02x %02x %02x %02x %02x %02x\n", x/8, y/8, charcode, lineattr, lten, rvv, vsp, gpa, hlgt);

	for(i=0;i<8;i+=2)
	{
		UINT8 pixel = (pixels >> (i * 2)) & 0xf;
		UINT8 value = (pixel >> 1) | (rvv << 4) | (vsp << 3);
		bitmap.pix32(y, x + i) = palette[value];
		bitmap.pix32(y, x + i + 1) = palette[(pixel & 1) ? 0 : value];
		if(m_back_color)
			bitmap.pix32(y, x + i - 1) = palette[value];
		m_back_color = pixel & 1;
	}
}

I8275_DRAW_CHARACTER_MEMBER(dwarfd_state::qc_display_pixels)
{
	int i;
	int bank = gpa;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT16 pixels = m_charmap[(linecount & 7) + ((charcode + (bank * 128)) << 3)];
	if(!x)
		m_back_color = false;

	//if(!linecount)
	//  logerror("%d %d %02x %02x %02x %02x %02x %02x %02x\n", x/8, y/8, charcode, lineattr, lten, rvv, vsp, gpa, hlgt);

	for(i=0;i<8;i+=2)
	{
		UINT8 pixel = (pixels >> (i * 2)) & 0xf;
		UINT8 value = (pixel >> 1) | (rvv << 4) | (vsp << 3);
		bitmap.pix32(y, x + i) = palette[value];
		bitmap.pix32(y, x + i + 1) = palette[(pixel & 1) ? 0 : value];
		if(m_back_color)
			bitmap.pix32(y, x + i - 1) = palette[value];
		m_back_color = pixel & 1;
	}
}

WRITE_LINE_MEMBER(dwarfd_state::dwarfd_sod_callback)
{
	m_crt_access = state;
}

WRITE_LINE_MEMBER(dwarfd_state::drq_w)
{
	if(state && !m_crt_access)
		m_maincpu->set_input_line(I8085_RST65_LINE, ASSERT_LINE);
	else if(!state)
		m_maincpu->set_input_line(I8085_RST65_LINE, CLEAR_LINE);

}

#if 0
static const gfx_layout tiles8x8_layout =
{
	4,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 0, 8, 16, 24 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};
#endif

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ STEP8(0, 4) },
//  {12,8,4,0,28,24,20,16 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ STEP8(0, 32) },
	8*32
};


static const gfx_layout tiles8x8_layout0 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 8, 16, 24 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ STEP8(0, 32) },
	8*32
};

static const gfx_layout tiles8x8_layout1 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 1 },
	{ 0, 8, 16, 24 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ STEP8(0, 32) },
	8*32
};

static const gfx_layout tiles8x8_layout2 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 2 },
	{ 0, 8, 16, 24 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ STEP8(0, 32) },
	8*32
};

static const gfx_layout tiles8x8_layout3 =
{
	4,8,
	RGN_FRAC(1,1),
	1,
	{ 3 },
	{ 0, 8, 16, 24 },
	//{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	{ STEP8(0, 32) },
	8*32
};
/*
static const gfx_layout tiles8x8_layout =
{
    8,8,
    RGN_FRAC(1,1),
    2,
    { 1,1},
    {6,6,2,2,14,14,10,10 },
    //{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
    { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
    8*16
};
*/

static GFXDECODE_START( dwarfd )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout0, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout1, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout2, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout3, 0, 16 )
GFXDECODE_END

PALETTE_INIT_MEMBER(dwarfd_state, dwarfd)
{
	UINT8 rgb[3];
	int i,j;
	UINT8 *prom = memregion("proms")->base();

	for (i = 0; i < 32; i++)
	{
		// what are the top 2 bits?
		rgb[0] = ((prom[i] & 0x08) >> 2) | (prom[i] & 1);
		rgb[1] = ((prom[i] & 0x10) >> 3) | ((prom[i] & 2) >> 1);
		rgb[2] = ((prom[i] & 0x20) >> 4) | ((prom[i] & 4) >> 2);
		for(j = 0; j < 3; j++)
			rgb[j] |= (rgb[j] << 6) | (rgb[j] << 4) | (rgb[j] << 2);

		palette.set_pen_color(i,rgb_t(rgb[0], rgb[1], rgb[2]));
	}
}

void dwarfd_state::machine_start()
{
	save_item(NAME(m_crt_access));
}

void dwarfd_state::machine_reset()
{
	m_crt_access = 0;
	m_back_color = false;
}

static MACHINE_CONFIG_START( dwarfd, dwarfd_state )

	/* basic machine hardware */
	/* FIXME: The 8085A had a max clock of 6MHz, internally divided by 2! */
	MCFG_CPU_ADD("maincpu", I8085A, 10595000/3*2)        /* ? MHz */
	MCFG_I8085A_SOD(WRITELINE(dwarfd_state,dwarfd_sod_callback))
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(272*2, 200+4*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 272*2-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_DEVICE("i8275", i8275_device, screen_update)

	MCFG_DEVICE_ADD("i8275", I8275, 10595000/3)
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(dwarfd_state, display_pixels)
	MCFG_I8275_IRQ_CALLBACK(INPUTLINE("maincpu", I8085_RST55_LINE))
	MCFG_I8275_DRQ_CALLBACK(WRITELINE(dwarfd_state, drq_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dwarfd)
	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(dwarfd_state, dwarfd)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN2"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( qc, dwarfd )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(qc_map)
	MCFG_CPU_IO_MAP(qc_io_map)

	MCFG_DEVICE_MODIFY("i8275")
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(dwarfd_state, qc_display_pixels)
MACHINE_CONFIG_END

/* Dwarfs den PROM explanation:
   The proms used in Dwarfs den are 74S188 (82s23 equivalent, 32x8 open collector)

    The prom at 7H controls the 8085 memory map:
        The inputs are such:
        A0 - 8085 A10
        A1 - 8085 A11
        A2 - 8085 A12
        A3 - 8085 A13
        A4 - 8085 A14
        /CE - ? (maybe 8085 ALE * !8085 A15)? need to trace again
        Outputs are as such:
        O1 - /CE on ROM at 9L (also test point 36)
        O2 - /CE on ROM at 9K (also test point 35)
        O3 - /CE on ROM at 9J (also test point 34)
        O4 - /CE on ROM at 9H (also test point 33)
        O5 - /CE on 2114 SRAMs at 8F and 9F
        O6 - /CE on 2114 SRAMs at 8E and 9E
        O7 - /CE on 2114 SRAMs at 8D and 9D
        O8 - indirectly, /CE on the 2114 srams at 8C and 9C, due to the fact that those two are battery-backed

    The prom at 3A controls the color palette:
        TODO: finish me!
*/


ROM_START( dwarfd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9l_pd_50-3196_m5l2732k.bin", 0x0000, 0x1000, CRC(34e942ae) SHA1(d4f0ee7f29e1c1a93b4b30b950023dbf60596100) )
	ROM_LOAD( "9k_pd_50-3193_hn462732g.bin",0x1000, 0x1000, CRC(78f0c260) SHA1(d6c3b8b3ef4ce99a811e291f1396a47106683df9) )
	ROM_LOAD( "9j_pd_50-3192_mbm2732.bin",  0x2000, 0x1000, CRC(9c66ee6e) SHA1(49c20fa276508b3c7b0134909295ae04ee46890f) )
	ROM_LOAD( "9h_pd_50-3375_2732.bin",     0x3000, 0x1000, CRC(daf5551d) SHA1(933e3453c9e74ca6695137c9f6b1abc1569ad019) )

	ROM_REGION16_LE( 0x4000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "6a_pd_50_1991_2732.bin"      ,0x0000, 0x1000, CRC(6da494bc) SHA1(0323eaa5f81e3b8561225ccdd4654c9a11f2167c) )
	ROM_LOAD16_BYTE( "6b_pd_50-1992_tms2732ajl.bin",0x2000, 0x1000, CRC(69208e1a) SHA1(8706f8f0d2dfeba5cebc71985ea46a67de13bc7d) )
	ROM_LOAD16_BYTE( "6c_pd_50-1993_tms2732ajl.bin",0x0001, 0x1000, CRC(cd8e5e54) SHA1(0961739d72d80e0ac00e6cbf9643bcebfe74830d) )
	ROM_LOAD16_BYTE( "6d_pd_50-1994_tms2732ajl.bin",0x2001, 0x1000, CRC(ef52b88c) SHA1(3405152da3194a71f6dac6492f275c746e781ee7) )

	ROM_REGION( 0x4000*2, "gfx2", 0 )
	ROM_FILL(0,  0x4000*2, 0)

	ROM_REGION( 0x40, "proms", 0 )
	/* ??? colors */
	ROM_LOAD( "3a_50-1381_63s080n.bin",0x00, 0x20, CRC(451d0a72) SHA1(9ff6e2c5bd2b57bd607cb33e60e7ed25bea164b3) )
	/* memory map */
	ROM_LOAD( "7h_7602.bin",0x20, 0x20, CRC(d5457333) SHA1(5872c868638c08faef7365d9c6e41dc3f070bd97) )
ROM_END

ROM_START( dwarfda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9l_pd_50-3196_m5l2732k.bin", 0x0000, 0x1000, CRC(34e942ae) SHA1(d4f0ee7f29e1c1a93b4b30b950023dbf60596100) )
	ROM_LOAD( "9k_pd_50-3193_hn462732g.bin",0x1000, 0x1000, CRC(78f0c260) SHA1(d6c3b8b3ef4ce99a811e291f1396a47106683df9) )
	ROM_LOAD( "9j_pd_50-3192_mbm2732.bin",  0x2000, 0x1000, CRC(9c66ee6e) SHA1(49c20fa276508b3c7b0134909295ae04ee46890f) )
	ROM_LOAD( "9h_pd_50-3375_2732.bin",     0x3000, 0x1000, CRC(daf5551d) SHA1(933e3453c9e74ca6695137c9f6b1abc1569ad019) )

	ROM_REGION16_LE( 0x4000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "6a_pd_50_1991_2732.bin"      ,0x0000, 0x1000, CRC(6da494bc) SHA1(0323eaa5f81e3b8561225ccdd4654c9a11f2167c) )
	ROM_LOAD16_BYTE( "50-1814-tms2732ajl.6b",0x2000, 0x1000, CRC(baa78a2e) SHA1(f7b61bae8919ed58c12d9f80f4133a722df08ac4) )
	ROM_LOAD16_BYTE( "6c_pd_50-1993_tms2732ajl.bin",0x0001, 0x1000, CRC(cd8e5e54) SHA1(0961739d72d80e0ac00e6cbf9643bcebfe74830d) )
	ROM_LOAD16_BYTE( "50-1815-tms2732ajl.6d",0x2001, 0x1000, CRC(303d2d16) SHA1(885df57f253d92f96692256325ffcf2ca71dc64f) )

	ROM_REGION( 0x4000*2, "gfx2", 0 )
	ROM_FILL(0,  0x4000*2, 0)

	ROM_REGION( 0x40, "proms", 0 )
	/* ??? colors */
	ROM_LOAD( "74s188n.3a",0x00, 0x20, CRC(9951e47a) SHA1(d06da09af25da06ac6bd0ee1fc99f7690b36b550) )
	/* memory map */
	ROM_LOAD( "74s188n.7h",0x20, 0x20, CRC(c9618de2) SHA1(d5636546dbc57e6aab01dab79b2ead1dfef8fa5c) )
ROM_END

/*
bp 32f9
do pc=3319
*/
ROM_START( quarterh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9l_qh_60-2402_7193.bin",  0x0000, 0x1000, CRC(6a097741) SHA1(f42de58743f79447c8b2b7e3f2d1aa87da663231) )
	ROM_LOAD( "9k_qh_60-2401_8193.bin",  0x1000, 0x1000, CRC(d958078c) SHA1(66d9cfaafe06042c93a4adb16480b4655857beeb) )
	ROM_LOAD( "9j_qh_60-2400_9193.bin",  0x2000, 0x1000, CRC(90efa26e) SHA1(aa29e2d90692cd97a9d18c93d8a2ea13ef1eab71) )
	ROM_LOAD( "9h_qh_80-2399_10193.bin", 0x3000, 0x1000, CRC(20be3f2f) SHA1(b6a67b664cc899e997742fb4350e3c8c1e23664a) )

	ROM_REGION16_LE( 0x4000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "6a_qh_01-2395_14193.bin",0x1000, 0x0800, CRC(98b8e844) SHA1(c8a2ec3cb61d6cdc3e8fadba23a9850afd8db05b) )
	ROM_LOAD16_BYTE( "6b_qh_01-2396_13193.bin",0x0000, 0x0800, CRC(03a21561) SHA1(0f6d8d13d81712e3e1971fe41e48ce5dff888dfd) )
	ROM_LOAD16_BYTE( "6c_qh_01-2397_12193.bin",0x1001, 0x0800, CRC(b0306417) SHA1(d8322009f39c937b6dc8fe3f591734f06213a9a3) )
	ROM_LOAD16_BYTE( "6d_qh_01-2398_11193.bin",0x0001, 0x0800, CRC(1db80656) SHA1(5cbfc2e4ba0c8028ff5e0ba2ec6220d8afb8cfc2) )

	ROM_REGION( 0x4000*2, "gfx2", 0 )
	ROM_FILL(0,  0x4000*2, 0)

	ROM_REGION( 0x800, "ld_data", 0 )
	ROM_LOAD( "-g_l-.bin",  0x0000, 0x0800, CRC(05c09fa6) SHA1(92ec4c225e477194d2c134403d9ebf922149b51c) )

	ROM_REGION( 0x40, "proms", 0 )
	/* ??? colors */
	ROM_LOAD( "3a_bprom.bin",0x00, 0x20, CRC(d4febd88) SHA1(37abb6508b375784f35d3eedc75ec7df4ef86048) )
	/* memory map */
	ROM_LOAD( "7h_bprom.bin",0x20, 0x20, CRC(c9618de2) SHA1(d5636546dbc57e6aab01dab79b2ead1dfef8fa5c) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "quarterh", 0, NO_DUMP )
ROM_END

ROM_START( quarterha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9l_qh_10-2402_7193.bin",  0x0000, 0x1000, CRC(4a688031) SHA1(ba5801dee72f28366d44e4ff80a4c8c93893617d) )
	ROM_LOAD( "9k_qh_10-2401_8193.bin",  0x1000, 0x1000, CRC(f5239ec6) SHA1(822522ac2bb9221f25eaad88751216acf3fe6d41) )
	ROM_LOAD( "9j_qh_10-2400_9193.bin",  0x2000, 0x1000, CRC(0b90860f) SHA1(6e5f79e1a7e1b477da8e0483f75e23129604564c) )
	ROM_LOAD( "9h_qh_50-2399_10193.bin", 0x3000, 0x1000, CRC(7d96c776) SHA1(e97080b0b0f524c3f313d5f7b7f3b093fb071bf9) )

	ROM_REGION16_LE( 0x4000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "6a_qh_01-2395_14193.bin",0x1000, 0x0800, CRC(98b8e844) SHA1(c8a2ec3cb61d6cdc3e8fadba23a9850afd8db05b) )
	ROM_LOAD16_BYTE( "6b_qh_01-2396_13193.bin",0x0000, 0x0800, CRC(03a21561) SHA1(0f6d8d13d81712e3e1971fe41e48ce5dff888dfd) )
	ROM_LOAD16_BYTE( "6c_qh_01-2397_12193.bin",0x1001, 0x0800, CRC(b0306417) SHA1(d8322009f39c937b6dc8fe3f591734f06213a9a3) )
	ROM_LOAD16_BYTE( "6d_qh_01-2398_11193.bin",0x0001, 0x0800, CRC(1db80656) SHA1(5cbfc2e4ba0c8028ff5e0ba2ec6220d8afb8cfc2) )

	ROM_REGION( 0x4000*2, "gfx2", 0 )
	ROM_FILL(0,  0x4000*2, 0)

	ROM_REGION( 0x800, "ld_data", 0 )
	ROM_LOAD( "-g_l-.bin",  0x0000, 0x0800, CRC(05c09fa6) SHA1(92ec4c225e477194d2c134403d9ebf922149b51c) )

	ROM_REGION( 0x40, "proms", 0 )
	/* ??? colors */
	ROM_LOAD( "3a_bprom.bin",0x00, 0x20, CRC(d4febd88) SHA1(37abb6508b375784f35d3eedc75ec7df4ef86048) )
	/* memory map */
	ROM_LOAD( "7h_bprom.bin",0x20, 0x20, CRC(c9618de2) SHA1(d5636546dbc57e6aab01dab79b2ead1dfef8fa5c) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "quarterh", 0, NO_DUMP )
ROM_END

ROM_START( quarterhb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a1.bin",  0x0000, 0x4000, CRC(9eace6a3) SHA1(285945034b73ff660a5a138d7be2fa431c0872e1) )

	ROM_REGION16_LE( 0x8000, "gfx_data", 0 )
	ROM_LOAD16_BYTE( "a2.bin",0x0001, 0x4000, CRC(b8cf5e27) SHA1(a5b451ab94ea1f2dda18a2d8ef9b8e0e46621420) ) // - oversized dumps perhaps?
	ROM_LOAD16_BYTE( "a3.bin",0x0000, 0x4000, CRC(8b5296b1) SHA1(9d27d85f2edb44b96acce3c3f3e611217dcef70d) ) // /

	ROM_REGION16_LE( 0x4000, "gfx1", 0 )
	ROM_COPY("gfx_data", 0x0000, 0x1000, 0x0800 )
	ROM_COPY("gfx_data", 0x0800, 0x0000, 0x0800 )
	ROM_COPY("gfx_data", 0x1000, 0x1800, 0x0800 )
	ROM_COPY("gfx_data", 0x1800, 0x0800, 0x0800 )

	ROM_REGION( 0x4000*2, "gfx2", 0 )
	ROM_FILL(0,  0x4000*2, 0)

	ROM_REGION( 0x800, "ld_data", ROMREGION_ERASEFF )

	ROM_REGION( 0x40, "proms", 0 )
	/* ??? colors */
	ROM_LOAD( "3a_50-1381_63s080n.bin",0x00, 0x20, CRC(451d0a72) SHA1(9ff6e2c5bd2b57bd607cb33e60e7ed25bea164b3) )
	/* memory map */
//  ROM_LOAD( "7h_7602.bin",0x20, 0x20, BAD_DUMP CRC(451d0a72) SHA1(9ff6e2c5bd2b57bd607cb33e60e7ed25bea164b3) )
	ROM_LOAD( "7h_bprom.bin",0x20, 0x20, BAD_DUMP CRC(c9618de2) SHA1(d5636546dbc57e6aab01dab79b2ead1dfef8fa5c) ) //taken from the other set, might be bad

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "quarterh", 0, NO_DUMP )
ROM_END

ROM_START( qc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qc.a10", 0x0000, 0x8000, CRC(4e0327de) SHA1(543d89f2e808e48041c6c10ad4686c7f7113ed88) )

	ROM_REGION16_LE( 0x8000, "gfx_data", 0 )
	ROM_LOAD16_BYTE( "qc.h6", 0x0001, 0x4000, CRC(a091526e) SHA1(58507414ae0d02c6adee80987f66fb8894e169b0) )
	ROM_LOAD16_BYTE( "qc.k6", 0x0000, 0x4000, CRC(eb583b44) SHA1(c23ad0037472c5bcb30fb030e4d13a6e5fde4b30) )

	ROM_REGION16_LE( 0x4000, "gfx1", 0 )
	ROM_COPY("gfx_data", 0x6000, 0x1000, 0x800 )
	ROM_COPY("gfx_data", 0x6800, 0x0000, 0x800 )
	ROM_COPY("gfx_data", 0x7000, 0x1800, 0x800 )
	ROM_COPY("gfx_data", 0x7800, 0x0800, 0x800 )
	ROM_COPY("gfx1", 0x0000, 0x2000, 0x2000 )

	ROM_REGION( 0x4000*2, "gfx2", 0 )
	ROM_FILL(0,  0x4000*2, 0)

	// borrowed from above and slightly edited
	ROM_REGION( 0x40, "proms", 0 )
	ROM_LOAD( "colors.bin",0x00, 0x20, BAD_DUMP CRC(3adeee7c) SHA1(f118ee62f84b0384316c12fc22356d43b2cfd876) )
ROM_END

DRIVER_INIT_MEMBER(dwarfd_state,dwarfd)
{
	/* expand gfx roms */
	UINT8 *dst = memregion("gfx2")->base();

	for (int i = 0; i < 0x4000/2; i++)
	{
		dst[i * 4 + 0] = (m_charmap[i] & 0x000f) << 4;

		dst[i * 4 + 1] = (m_charmap[i] & 0x00f0) >> 0;

		dst[i * 4 + 2] = (m_charmap[i] & 0x0f00) >> 4;

		dst[i * 4 + 3] = (m_charmap[i] & 0xf000) >> 8;
	}

	/* use low bit as 'interpolation' bit */
	for (int i = 0; i < 0x8000; i++)
	{
		if (dst[i] & 0x10)
		{
			dst[i] = (dst[i] & 0xe0) >> 1;
	//      dst[i] |= ((dst[(i + 1) & 0x7fff] & 0xe0) >> 4);

		}
		else
		{
			dst[i] = (dst[i] & 0xe0) >> 1;
			dst[i] |= (dst[i] >> 4);

		}
	//      dst[i] = dst[i] & 0xe0;
	}

	save_item(NAME(m_dw_ram));

	memset(m_dw_ram, 0, sizeof(m_dw_ram));

}

DRIVER_INIT_MEMBER(dwarfd_state,qc)
{
	DRIVER_INIT_CALL(dwarfd);

	// hacks for program to proceed
	memregion("maincpu")->base()[0x6564] = 0x00;
	memregion("maincpu")->base()[0x6565] = 0x00;

	memregion("maincpu")->base()[0x59b2] = 0x00;
	memregion("maincpu")->base()[0x59b3] = 0x00;
	memregion("maincpu")->base()[0x59b4] = 0x00;

}

/*    YEAR  NAME      PARENT     MACHINE INPUT   INIT    ORENTATION,         COMPANY           FULLNAME            FLAGS */
GAME( 1981, dwarfd,   0,         dwarfd, dwarfd, dwarfd_state, dwarfd, 0, "Electro-Sport", "Draw Poker III / Dwarfs Den (Dwarf Gfx)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1981, dwarfda,   dwarfd,   dwarfd, dwarfd, dwarfd_state, dwarfd, 0, "Electro-Sport", "Draw Poker III / Dwarfs Den (Card Gfx)",            MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
GAME( 1983, quarterh, 0,         dwarfd, quarterh, dwarfd_state, dwarfd, 0, "Electro-Sport", "Quarter Horse (set 1, Pioneer PR-8210)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1983, quarterha, quarterh, dwarfd, quarterh, dwarfd_state, dwarfd, 0, "Electro-Sport", "Quarter Horse (set 2, Pioneer PR-8210)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1983, quarterhb, quarterh, dwarfd, quarterh, dwarfd_state, dwarfd, 0, "Electro-Sport", "Quarter Horse (set 3, Pioneer LD-V2000)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1995, qc,       0,         qc,     quarterh, dwarfd_state, qc,     0, "ArJay Exports/Prestige Games", "Quarter Horse Classic", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
