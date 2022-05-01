// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca, David Haywood
/******************************************************************************

  MAGIC CARD - IMPERA
  -------------------

  Preliminary driver by Roberto Fresca, David Haywood & Angelo Salese


  TODO:
  - driver based off raw guesses (we don't have relevant key docs);
  - Device-ize 66470
    Handles video and CRTC, has annoying blitter ops (magicard text on
    playfield at very least, most likely service mode too);
  - Verify RAM config on PCBs;
  - I/Os;
  - UART;
  - magicardj & magicle: hook-up PIC;
  - magicardj: expects GFX pitch width to be 320 rather than 336 for
    title screen to draw correctly;
  - hotslots, quingo: sets up 68070 timer chs 1 & 2, currently unsupported;
  - magicardj: keeps reading timer 0 low byte, expects a live change?
  - bigdeal0: punts with an address error PC=0x60ea3a A2=$c71c38e3;
  - lucky7i, unkte06, magicardw: loops on i2c accesses;
  - Is int1_w unconnected? Doesn't seem to be enabled by games so far;
  - puzzleme: confirm it has a ssg (mapping matches hotslots);


  Games running on this hardware:

  * Magic Card (set 1),                         Impera, 199?.
  * Magic Card (set 2),                         Impera, 199?.
  * Magic Card (set 3),                         Impera, 199?.
  * Magic Card Export 94 (set 1),               Impera, 1994.
  * Magic Card Export 94 (set 2),               Impera, 1994.
  * Magic Export (V.211A),                      Impera, 1994.
  * Magic Card Jackpot (4.01),                  Impera, 1998.
  * Magic Card - Wien (Sicherheitsversion 1.2), Impera, 1993.
  * Magic Lotto Export (5.03),                  Impera, 2001.
  * Hot Slots (6.00),                           Impera, 2002.
  * Quingo Export (5.00),                       Impera, 1999.
  * Bel Slots Export (5.01),                    Impera, 1999.
  * Big Deal Belgien (5.04),                    Impera, 2001.
  * Puzzle Me!,                                 Impera, 199?.
  * unknown 'TE06',                             Impera, 199?.
  * Lucky 7 (Impera),                           Impera, 199?.
  * unknown Poker 'W',                          unknown, 1993.
  * Dallas Poker,                               unknown, 1993.
  * Kajot Card (Version 1.01, Wien Euro),       Amatic, 1993.


*******************************************************************************


  *** Hardware Notes ***

  These are actually the specs of the Philips CD-i console.

  Identified:

  - CPU:  1x Philips SCC 68070 CCA84 (16 bits Microprocessor, PLCC) @ 15 MHz
  - VSC:  1x Philips SCC 66470 CAB (Video and System Controller, QFP)

  - Protection: 1x Dallas TimeKey DS1207-1 (for book-keeping protection)

  - Crystals:   1x 30.0000 MHz.
                1x 19.6608 MHz.

  - PLDs:       1x PAL16L8ACN
                1x PALCE18V8H-25


*******************************************************************************


  *** General Notes ***

  Impera released "Magic Card" in a custom 16-bits PCB.
  The hardware was so expensive and they never have reached the expected sales,
  so... they ported the game to Impera/Funworld 8bits boards, losing part of
  graphics and sound/music quality. The new product was named "Magic Card II".


*******************************************************************************

  Impera boards...

  KNOWN REVS | Used with these games         | Differences to previews Revision
  ======================================================================================================
  V 1.04     | lucky7i                       | lowest known revision, does not have a socket for the PIC
  ------------------------------------------------------------------------------------------------------
  V 1.05     | unkte06, magicardw            | PIC16C54 + XTAL got added
  ------------------------------------------------------------------------------------------------------
  V 2.1      | puzzleme                      | ESI1, 24C02, YM2149F, RTC added
  ------------------------------------------------------------------------------------------------------
  V 2.2      | magicarde                     | 
  ------------------------------------------------------------------------------------------------------
  V 4.0      | magicardj, magicardf, magicle | ESI1 replaced by ALTERA MAX EPM7128SQC100
             |                               | YM2149F replaced by YMZ284-D, MX29F1610 added
  ------------------------------------------------------------------------------------------------------


*******************************************************************************

  For PCB layouts and extra info see the ROM Load of each game below.


*******************************************************************************/

#include "emu.h"
#include "machine/scc68070.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/saa1099.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define CLOCK_A XTAL(30'000'000)
#define CLOCK_B XTAL(8'000'000)
#define CLOCK_C XTAL(19'660'800)


class magicard_state : public driver_device
{
public:
	magicard_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_magicram(*this, "magicram")
		, m_magicramb(*this, "magicramb")
		, m_pcab_vregs(*this, "pcab_vregs")
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void magicard(machine_config &config);
	void hotslots(machine_config &config);

	void init_magicard();

private:
	//u16 m_vector;
	required_shared_ptr<uint16_t> m_magicram;
	required_shared_ptr<uint16_t> m_magicramb;
	required_shared_ptr<uint16_t> m_pcab_vregs;
	uint16_t test_r();
	uint16_t philips_66470_r(offs_t offset);
	void philips_66470_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_magicard(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(magicard_scanline_cb);
	required_device<scc68070_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	void hotslots_map(address_map &map);
	void magicard_map(address_map &map);
	void ramdac_map(address_map &map);
};


/*********************************************
*               Video Hardware               *
*********************************************/

/*
66470
video and system controller
19901219/wjvg
*/
/*
TODO: check this register,doesn't seem to be 100% correct.
1fffe0  csr = control and status register
    w 00...... ........ DM = slow timing speed, normal dram mode
    w 01...... ........ DM = fast timing speed, page dram mode
    w 10...... ........ DM = fast timing speed, nibble dram mode
    w 11...... ........ DM = slow timing speed, dual-port vram mode
    w ..1..... ........ TD = 256/64 k dram's
    w ...1.... ........ CG = enable character generator
    w ....1... ........ DD = rom data acknowledge delay
    w .....1.. ........ ED = early dtack
    w ......0. ........ not used
    w .......1 ........ BE  = enable bus error (watchdog timer)
   r  ........ 1....... DA  = vertical display active
   r  ........ .1...... FG  = set during frame grabbing (if fg in dcr set)
   r  ........ ..xxx... not used
   r  ........ .....1.. IT2 = intn active
   r  ........ ......1. IT1 = pixac free and intn active
   r  ........ .......1 BE  = bus error generated by watchdog timer
*/

// 63 at post test, 6d all the time.
#define SCC_CSR_VREG    (m_pcab_vregs[0x00/2] & 0xffff)
#define SCC_CG_VREG     ((SCC_CSR_VREG & 0x10)>>4)

/*
1fffe2  dcr = display command register
    w 1....... ........  DE = enable display
    w .00..... ........  CF = 20   MHz (or 19.6608 MHz)
    w .01..... ........  CF = 24   MHz
    w .10..... ........  CF = 28.5 MHz
    w .11..... ........  CF = 30   MHz
    w ...1.... ........  FD = 60/50 Hz frame duration
    w ....00.. ........  SM/SS = non-interlaced scan mode
    w ....01.. ........  SM/SS = double frequency scan mode
    w ....10.. ........  SM/SS = interlaced scan mode
    w ....11.. ........  SM/SS = interlaced field repeat scan mode
    w ......1. ........  LS = full screen/border
    w .......1 ........  CM = logical/physical screen
    w ........ 1.......  FG = 4/8 bits per pixel
    w ........ .1......  DF = enable frame grabbing
    w ........ ..00....  IC/DC = ICA and DCA inactive
    w ........ ..01....  IC/DC = ICA active, reduced DCA mode (DCA sz=16 byts)
    w ........ ..10....  IC/DC = ICA active, DCA inactive
    w ........ ..11....  IC/DC = ICA active, DCA active (DCA size=64 bytes)
    w ........ ....aaaa  VSR:H = video start address (MSB's)
*/

#define SCC_DCR_VREG    (m_pcab_vregs[0x02/2] & 0xffff)
#define SCC_DE_VREG     ((SCC_DCR_VREG & 0x8000)>>15)
#define SCC_FG_VREG     ((SCC_DCR_VREG & 0x0080)>>7)
#define SCC_VSR_VREG_H  ((SCC_DCR_VREG & 0xf)>>0)

/*
1fffe4  vsr = video start register
    w aaaaaaaa aaaaaaaa  VSR:L = video start address (LSB's)
*/

#define SCC_VSR_VREG_L  (m_pcab_vregs[0x04/2] & 0xffff)
#define SCC_VSR_VREG    ((SCC_VSR_VREG_H)<<16) | (SCC_VSR_VREG_L)

/*
1fffe6  bcr = border colour register
    w ........ nnnnnnnn  in 8 bit mode
    w ........ nnnn....  in 4 bit mode
*/
/*
(Note: not present on the original vreg listing)
1fffe8 dcr2 = display command register 2
    w x....... ........  not used
    w .nn..... ........  OM = lower port of the video mode (with CM)
    w ...1.... ........  ID = Indipendent DCA bit
    w ....nn.. ........  MF = Mosaic Factor (2,4,8,16)
    w ......nn ........  FT = File Type (0/1 = bitmap, 2 = RLE, 3 = Mosaic)
    w ........ xxxx....  not used
    w ........ ....aaaa  "data" (dunno the purpose...)
*/
#define SCC_DCR2_VREG  (m_pcab_vregs[0x08/2] & 0xffff)

/*
(Note: not present on the original vreg listing)
1fffea dcp = ???
    w aaaaaaaa aaaaaa--  "data" (dunno the purpose...)
    w -------- ------xx not used
*/

/*
1fffec  swm = selective write mask register
    w nnnnnnnn ........  mask
*/
/*
1fffee  stm = selective mask register
    w ........ nnnnnnnn  mask
*/
/*
1ffff0  a = source register a
    w nnnnnnnn nnnnnnnn  source
*/
#define SCC_SRCA_VREG  (m_pcab_vregs[0x10/2] & 0xffff)

/*
1ffff2  b = destination register b
   rw nnnnnnnn nnnnnnnn  destination
*/

#define SCC_DSTB_VREG  (m_pcab_vregs[0x12/2] & 0xffff)

/*
1ffff4  pcr = pixac command register
    w 1....... ........  4N  = 8/4 bits per pixel
    w .1....00 ....x00.  COL = enable colour2 function
    w .1....00 .....01.  COL = enable colour1 function
    w .1...0.. .....10.  COL = enable bcolour2 function
    w .1...0.. .....11.  COL = enable bcolour1 function
    w ..1..000 ....x00.  EXC = enable exchange function
    w ..1..000 .....01.  EXC = enable swap function
    w ..1..000 .....10.  EXC = enable inverted exchange function
    w ..1..000 .....11.  EXC = enable inverted swap function
    w ...1..0. ....x00.  CPY = enable copy type b function
    w ...1...0 ....x10.  CPY = enable copy type a function
    w ...1..0. .....01.  CPY = enable patch type b function
    w ...1...0 .....11.  CPY = enable patch type a function
    w ....1000 .....00.  CMP = enable compare function
    w ....1000 .....10.  CMP = enable compact function
    w .....1.. ........  RTL = manipulate right to left
    w ......1. ........  SHK = shrink picture by factor 2
    w .......1 ........  ZOM = zoom picture by factor 2
    w ........ nnnn....  LGF = logical function
    w ........ 0000....  LGF = d=r
    w ........ 0001....  LGF = d=~r
    w ........ 0010....  LGF = d=0
    w ........ 0011....  LGF = d=1
    w ........ 0100....  LGF = d=~(d^r)
    w ........ 0101....  LGF = d=d^r
    w ........ 0110....  LGF = d=d&r
    w ........ 0111....  LGF = d=~d&r
    w ........ 1000....  LGF = d=~d&~r
    w ........ 1001....  LGF = d=d&~r
    w ........ 1010....  LGF = d=~d|r
    w ........ 1011....  LGF = d=d|r
    w ........ 1100....  LGF = d=d|~r
    w ........ 1101....  LGF = d=~d|~r
    w ........ 1110....  LGF = d=d
    w ........ 1111....  LGF = d=~d
    w ........ ....1...  INV = invert transparancy state of source bits
    w ........ .....1..  BIT = copy:     enable copy type a
    w ........ .....1..  BIT = colour:   enable bcolour/colour
    w ........ .....1..  BIT = compare:  compact/compare
    w ........ ......1.  TT  = perform transparancy test
    w ........ .......0
*/

#define SCC_PCR_VREG  (m_pcab_vregs[0x14/2] & 0xffff)

/*
1ffff6  mask = mask register
    w ........ ....nnnn  mask nibbles/0
*/
/*
1ffff8  shift = shift register
    w ......nn ........  shift by .. during source alignment
*/
/*
1ffffa  index = index register
    w ........ ......nn  bcolour: use bit .. in the source word
    w ........ ......nn  compact: nibble .. will hold the result
*/
/*
1ffffc  fc/bc = foreground/background colour register
    w nnnnnnnn ........  FC = foreground colour
    w ........ nnnnnnnn  BC = background colour
*/
/*
1ffffe  tc = transparent colour register
    w nnnnnnnn ........  transparent colour
*/


void magicard_state::video_start()
{
}

uint32_t magicard_state::screen_update_magicard(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// TODO: border & genlock
	bitmap.fill(m_palette->black_pen(), cliprect);

	 // punt if display enable is off
	if(!(SCC_DE_VREG))
		return 0;

	uint32_t count = ((SCC_VSR_VREG) / 2);

	if(SCC_FG_VREG)  // 4bpp gfx
	{
		for(int y = 0; y < 300; y++)
		{
			for(int x = 0; x < 84; x++)
			{
				uint32_t color;

				color = ((m_magicram[count]) & 0x000f) >> 0;

				if(cliprect.contains((x * 4) + 3, y))
					bitmap.pix(y, (x * 4) + 3) = m_palette->pen(color);

				color = ((m_magicram[count]) & 0x00f0) >> 4;

				if(cliprect.contains((x * 4) + 2, y))
					bitmap.pix(y, (x * 4) + 2) = m_palette->pen(color);

				color = ((m_magicram[count]) & 0x0f00) >> 8;

				if(cliprect.contains((x * 4) + 1, y))
					bitmap.pix(y, (x * 4) + 1) = m_palette->pen(color);

				color = ((m_magicram[count]) & 0xf000) >> 12;

				if(cliprect.contains((x * 4) + 0, y))
					bitmap.pix(y, (x * 4) + 0) = m_palette->pen(color);

				count++;
			}
		}
	}
	else  // 8bpp gfx
	{
		for(int y = 0; y < 300; y++)
		{
			for(int x = 0; x < 168; x++)
			{
				uint32_t color;

				color = ((m_magicram[count]) & 0x00ff) >> 0;

				if(cliprect.contains((x * 2) + 1, y))
					bitmap.pix(y, (x * 2) + 1) = m_palette->pen(color);

				color = ((m_magicram[count]) & 0xff00) >> 8;

				if(cliprect.contains((x * 2) + 0, y))
					bitmap.pix(y, (x * 2) + 0) = m_palette->pen(color);

				count++;
			}
		}
	}

	return 0;
}


/*********************************************
*                R/W Handlers                *
*********************************************/

uint16_t magicard_state::test_r()
{
	return machine().rand();
}

uint16_t magicard_state::philips_66470_r(offs_t offset)
{
	switch(offset)
	{
		case 0/2:
		{
			uint8_t vdisp;
			vdisp = m_screen->vpos() < 256;

			// TODO: other bits
			return (m_pcab_vregs[offset] & 0xff7f) | vdisp << 7;
		}
	}

	return m_pcab_vregs[offset];
}

void magicard_state::philips_66470_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pcab_vregs[offset]);

//  if(offset == 0x10/2)
//  {
		//printf("%04x %04x %04x\n",data,m_pcab_vregs[0x12/2],m_pcab_vregs[0x14/2]);
		//m_pcab_vregs[0x12/2] = m_pcab_vregs[0x10/2];
//  }
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void magicard_state::magicard_map(address_map &map)
{
//  map.global_mask(0x1fffff);
	map(0x00000000, 0x001ffbff).mirror(0x00200000).ram().share("magicram");
	map(0x00600000, 0x007ffbff).ram().share("magicramb");
	// 001ffc00-001ffdff System I/O
	map(0x001ffc00, 0x001ffc01).mirror(0x7fe00000).portr("SYSTEM");
	map(0x001ffc40, 0x001ffc41).mirror(0x7fe00000).r(FUNC(magicard_state::test_r));
	map(0x001ffd01, 0x001ffd01).mirror(0x7fe00000).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x001ffd03, 0x001ffd03).mirror(0x7fe00000).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x001ffd05, 0x001ffd05).mirror(0x7fe00000).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x001ffd40, 0x001ffd43).mirror(0x7fe00000).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff);
	map(0x001ffd80, 0x001ffd81).mirror(0x7fe00000).r(FUNC(magicard_state::test_r));
	map(0x001ffd80, 0x001ffd81).mirror(0x7fe00000).nopw();
	map(0x001fff80, 0x001fffbf).mirror(0x7fe00000).ram();  // DRAM I/O, not accessed by this game, CD buffer?
	map(0x001fffe0, 0x001fffff).mirror(0x7fe00000).rw(FUNC(magicard_state::philips_66470_r), FUNC(magicard_state::philips_66470_w)).share("pcab_vregs");
}

// Different PAL mapping?
void magicard_state::hotslots_map(address_map &map)
{
//  map.global_mask(0x1fffff);
	// puzzleme sets $0080000a as default reset vector, magicardf sets $00800078
	// latter also will address error if we mirror with bank A by logic (i.e. .mirror(0x00a00000))
	// we currently map it to B bank for now
	map(0x00000000, 0x001ffbff).mirror(0x00200000).ram().share("magicram");
	map(0x00600000, 0x007ffbff).ram().share("magicramb");
	map(0x00800000, 0x009ffbff).ram().share("magicramb");
	map(0x001fff80, 0x001fffbf).mirror(0x7fe00000).ram();  // DRAM I/O, not accessed by this game, CD buffer?
	map(0x001fffe0, 0x001fffff).mirror(0x7fe00000).rw(FUNC(magicard_state::philips_66470_r), FUNC(magicard_state::philips_66470_w)).share("pcab_vregs");
	map(0x00400000, 0x00403fff).ram();  // ? bigdeal0, magicardj accesses this as scratchram
	map(0x00411000, 0x00411001).portr("SYSTEM");
	map(0x00414001, 0x00414001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x00414003, 0x00414003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x00414005, 0x00414005).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x00414007, 0x00414007).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x00415003, 0x00415003).r("ramdac", FUNC(ramdac_device::pal_r));
	map(0x00416001, 0x00416001).w("ssg", FUNC(ymz284_device::data_w));
	map(0x00417001, 0x00417001).w("ssg", FUNC(ymz284_device::address_w));
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( magicard )
	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01, 0x01, "SYSTEM0" )
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

	PORT_DIPNAME( 0x0100, 0x0100, "SYSTEM1" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	// Used by magicard to enter into gameplay
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	// coin sound in magicard (but no GFX is updated?)
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/******************************************
*          Machine Start & Reset          *
******************************************/

void magicard_state::machine_reset()
{
	// TODO: confirm reset state
	uint16_t *src    = (uint16_t*)memregion("maincpu")->base();
	uint16_t *dst    = m_magicram;
	memcpy (dst, src, 0x80000);
	memcpy (dst + 0x40000 * 1, src, 0x80000);
	memcpy (dst + 0x40000 * 2, src, 0x80000);
	memcpy (dst + 0x40000 * 3, src, 0x7fc00);
	dst = m_magicramb;
	memcpy (dst, src, 0x80000);
	memcpy (dst + 0x40000 * 1, src, 0x80000);
	memcpy (dst + 0x40000 * 2, src, 0x80000);
	memcpy (dst + 0x40000 * 3, src, 0x7fc00);
}


/*********************************************
*              Machine Drivers               *
*********************************************/

TIMER_DEVICE_CALLBACK_MEMBER(magicard_state::magicard_scanline_cb)
{
	int scanline = param;

	// hotslots and quingo definitely wants two irqs per frame,
	// reading vdisp as branch dispatch and setting a specific flag in RAM
	if (scanline == 256 || scanline == 0)
	{
		m_maincpu->int2_w(1);
		m_maincpu->int2_w(0);
	}
}

void magicard_state::ramdac_map(address_map &map)
{
	map(0x0000, 0x03ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


void magicard_state::magicard(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);  // SCC-68070 CCA84
	m_maincpu->set_addrmap(AS_PROGRAM, &magicard_state::magicard_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(magicard_state::magicard_scanline_cb), "screen", 0, 1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// TODO: has dynamic resolution, fill defaults and convert to set_raw
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(400, 300);
	m_screen->set_visarea(0, 320-1, 0, 256-1);
//  m_screen->screen_vblank().set(m_maincpu, FUNC(scc68070_device::int2_w));
	m_screen->set_screen_update(FUNC(magicard_state::screen_update_magicard));

	PALETTE(config, m_palette).set_entries(0x100);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &magicard_state::ramdac_map);

	SPEAKER(config, "mono").front_center();
	SAA1099(config, "saa", CLOCK_B).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void magicard_state::hotslots(machine_config &config)
{
	magicard(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &magicard_state::hotslots_map);

	config.device_remove("saa");
	YMZ284(config, "ssg", 4000000).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( magicard )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "magicorg.bin", 0x000000, 0x80000, CRC(810edf9f) SHA1(0f1638a789a4be7413aa019b4e198353ba9c12d9) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("mgorigee.bin",    0x0000, 0x0100, CRC(73522889) SHA1(3e10d6c1585c3a63cff717a0b950528d5373c781) )
ROM_END

ROM_START( magicarda )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "mcorigg2.bin", 0x00000, 0x20000, CRC(48546aa9) SHA1(23099a5e4c9f2c3386496f6d7f5bb7d435a6fb16) )
	ROM_RELOAD(                           0x40000, 0x20000 )
	ROM_LOAD16_WORD_SWAP( "mcorigg1.bin", 0x20000, 0x20000, CRC(c9e4a38d) SHA1(812e5826b27c7ad98142a0f52fbdb6b61a2e31d7) )
	ROM_RELOAD(                           0x40001, 0x20000 )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("mgorigee.bin",    0x0000, 0x0100, CRC(73522889) SHA1(3e10d6c1585c3a63cff717a0b950528d5373c781) )
ROM_END

ROM_START( magicardb )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "mg_8.bin", 0x00000, 0x80000, CRC(f5499765) SHA1(63bcf40b91b43b218c1f9ec1d126a856f35d0844) )

	/*bigger than the other sets?*/
	ROM_REGION( 0x20000, "other", 0 )  // unknown
	ROM_LOAD16_WORD_SWAP("mg_u3.bin",   0x00000, 0x20000, CRC(2116de31) SHA1(fb9c21ca936532e7c342db4bcaaac31c478b1a35) )
ROM_END

/*
   Magic Card Jackpot 4.01
  (Also Magic Lotto Export)
  -------------------------

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |       |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| | MAGIC |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| | CARD  |    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |JACKPOT|    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   4.01|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |11.7.98|    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                               ___________                                    ___|
  |    ___________________                            |RTC2421 A  |                                   ___|
  |   |   :::::::::::::   |                           |___________|                                  Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz.
  Xtal 2:  8.000 MHz.
  Xtal 3: 19.660 MHz.

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

  Silkscreened on the solder side:

  LEOTS.
  2800
  AT&S-F0 ML 94V-0

  IMPERA AUSTRIA          -------
  TEL: 0043/7242/27116     V 4.0
  FAX: 0043/7242/27053    -------

*/
ROM_START( magicardj )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(ab2ed583) SHA1(a2d7148b785a8dfce8cff3b15ada293d65561c98) ) // sldh

	ROM_REGION16_LE( 0x4280, "pic16f84", 0 )  // decapped and dumped
	ROM_LOAD("magicardj_4.01_pic16f84_code.bin",   0x0000, 0x0800, CRC(c6502436) SHA1(85c4126251bd60ec1f4e28615ec7f948ef8c088f) )
	/*
	{
	"conf_word": 0,
	"secure": true,
	"user_id0": 16256,
	"user_id1": 16262,
	"user_id2": 16265,
	"user_id3": 16264
	}
	*/
	// ID locations:
	ROM_FILL( 0x4000, 0x01, 0x80 )
	ROM_FILL( 0x4001, 0x01, 0x3f )
	ROM_FILL( 0x4002, 0x01, 0x86 )
	ROM_FILL( 0x4003, 0x01, 0x3f )
	ROM_FILL( 0x4004, 0x01, 0x89 )
	ROM_FILL( 0x4005, 0x01, 0x3f )
	ROM_FILL( 0x4006, 0x01, 0x88 )
	ROM_FILL( 0x4007, 0x01, 0x3f )
	// configuration word: all 0
	ROM_FILL( 0x400e, 0x01, 0x00 )
	ROM_FILL( 0x400f, 0x01, 0x00 )
	ROM_LOAD("magicardj_4.01_pic16f84_data.bin",   0x4200, 0x0080, CRC(40961fef) SHA1(8617ef78d50842ea89d81d4db3728b3f799d7530) )

	ROM_REGION( 0x200000, "other", 0 )  // unknown contents
	ROM_LOAD("29f1610mc.ic30",  0x000000, 0x200000, NO_DUMP )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("24c02c.ic26", 0x0000, 0x0100, CRC(b5c86862) SHA1(0debc0f7e7c506e5a4e2cae152548d80ad72fc2e) )
ROM_END

/*
  Magic Card Export 94
  International Ver. 2.11a (set 1)
  Vnr.29.07.94    CHECKSUM: A63D

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141
  1x ESI1 I9631
  1x MUSIC TR9C1710-11PCA SA121X/9617
  1x YAMAHA YM2149F 9614

  XTAL:

  Q1: 19.6608 Mhz
  Q2: 30.000 Mhz
  Q3: 3686.400  1Q08/95

*/
ROM_START( magicarde )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v2.11a_a63d.ic21", 0x00000, 0x80000, CRC(b5f24412) SHA1(73ff05c19132932a419fef0d5dc985440ce70e83) )

	ROM_REGION( 0x2000, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54.ic29",   0x0000, 0x1fff, CRC(9c225a49) SHA1(249c12d23d1a85de828652c55a1a19ef8ec378ef) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("st24c02.ic26",    0x0000, 0x0100, CRC(98287c67) SHA1(ad34e55c1ce4f77c27049dac88050ed3c94af1a0) )
ROM_END

/*
  Magic Card Export 94
  International Ver. 2.11a (set 2)
  Vnr.29.07.94    CHECKSUM: 9505

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141
  1x ESI1 I9631
  1x MUSIC TR9C1710-11PCA SA121X/9617
  1x YAMAHA YM2149F 9614

  XTAL:

  Q1: 19.6608 Mhz
  Q2: 30.000 Mhz
  Q3: 3686.400  1Q08/95

*/
ROM_START( magicardea )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v2.11a_9505.ic21", 0x00000, 0x80000, CRC(24c69c01) SHA1(0928800b9cfc2ae358f90b3f79c08acd2b2aa7d8) )

	ROM_REGION( 0x2000, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54.ic29",   0x0000, 0x1fff, CRC(9c225a49) SHA1(249c12d23d1a85de828652c55a1a19ef8ec378ef) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("st24c02.ic26",    0x0000, 0x0100, CRC(98287c67) SHA1(ad34e55c1ce4f77c27049dac88050ed3c94af1a0) )
ROM_END

/*
  Magic Card Export 94
  Clubversion Export v2.9a
  Vnr.02.08.94    CHECKSUM: 5B64

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141

  Other components are unreadable
  in the PCB picture.
  
*/
ROM_START( magicardeb )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v2.9a_5b64.ic21", 0x00000, 0x80000, CRC(81ad0437) SHA1(117e2681541f786874cd0bce7f8bfb2bffb0b548) )

	// PIC undumped
	// Serial EPROM undumped
ROM_END

/*
  Magic Card Export
  Version 4.01
  Vnr.07.03.98    CHECKSUM: AF18

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141

  Other components are unreadable
  in the PCB picture.
  
*/
ROM_START( magicardec )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v4.01_af18.ic21", 0x00000, 0x80000, CRC(7700fd22) SHA1(0555c08c82f56e6399a89f6408e52d9d0beba2ac) )

	// PIC undumped
	// Serial EPROM undumped
ROM_END

/*
  Magic Export.
  Ver 211A.

  1x Philips SCC66470CAB.
  1x Philips SCC68070 CCA84.
  1x MUSIC TR9C1710-11PCA.
  1x YAMAHA YMZ284-D.

  1x M27C4002 EPROM (dumped).
  1x 29F1610MC-12 Flash EEPROM (dumped).
  1x 24LC02 Serial EEPROM (dumped).

  1x Altera MAX EPM712xxxxx (unreadable).

  XTAL: 3x unknown frequency.

*/
ROM_START( magicardf )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(098258c0) SHA1(5f5dfe376c980ec88e68b324ba912022091e2426) )

	ROM_REGION( 0x200000, "other", 0 )  // Flash EEPROM
	ROM_LOAD("mx29f1610.ic30",  0x000000, 0x200000, CRC(c8ba9820) SHA1(fcae1e200c718b549b91d1110025595ffd7bdd51) )

	ROM_REGION( 0x0100, "sereeprom", 0 ) // Serial EEPROM
	ROM_LOAD("24lc02b.ic26",    0x0000, 0x0100, CRC(47c8b137) SHA1(6581e1f4ea65c833fa566c21c76dbe741af488f4) )
ROM_END

/*
  Magic Card - Wien
  Sicherheitsversion 1.2

*/
ROM_START( magicardw )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "am27c4096.bin", 0x00000, 0x80000, CRC(d9e2a4ec) SHA1(b3000ded242fa25709c90b9b2541c9d1d5cabebb) )

	ROM_REGION( 0x1fff, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54a.bin",   0x0000, 0x1fff, CRC(e777e814) SHA1(e0440be76fa1f3c7ae7d31e1b29a2ba73552231c) )
ROM_END


ROM_START( magicle )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(73328346) SHA1(fca5f8a93f25377e659c2b291674d706ca37400e) )

	ROM_REGION16_LE( 0x4280, "pic16f84", 0 )  // decapped and dumped
	ROM_LOAD("magicle_5.03_pic16f84_code.bin",   0x0000, 0x0800, CRC(22965864) SHA1(c421a9e9fac7c9c5dc01adda620dc8f5f16d94ba) )
	/*
{
	"conf_word": 0,
	"secure": true,
	"user_id0": 16256,
	"user_id1": 16263,
	"user_id2": 16265,
	"user_id3": 16265
}
	*/
	// ID locations:
	ROM_FILL( 0x4000, 0x01, 0x80 )
	ROM_FILL( 0x4001, 0x01, 0x3f )
	ROM_FILL( 0x4002, 0x01, 0x87 )
	ROM_FILL( 0x4003, 0x01, 0x3f )
	ROM_FILL( 0x4004, 0x01, 0x89 )
	ROM_FILL( 0x4005, 0x01, 0x3f )
	ROM_FILL( 0x4006, 0x01, 0x89 )
	ROM_FILL( 0x4007, 0x01, 0x3f )
	// configuration word: all 0
	ROM_FILL( 0x400e, 0x01, 0x00 )
	ROM_FILL( 0x400f, 0x01, 0x00 )
	ROM_LOAD("magicle_5.03_pic16f84_data.bin",   0x4200, 0x0080, CRC(b3cdf90f) SHA1(0afec6f78320e5fe653073769cdeb32918da061b) )

	ROM_REGION( 0x200000, "other", 0 )  // unknown contents
	ROM_LOAD("29f1610mc.ic30",  0x000000, 0x200000, NO_DUMP )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("24c04a.ic26", 0x0000, 0x0200, CRC(48c4f473) SHA1(5355313cc96f655096e13bfae78be3ba2dfe8a2d) )
ROM_END

/*
  Hot Slots Version 6.00

  Hardware PCB informations:
  E179465--A/02 LPL-CPU V4.0/MULTI GAME

  Eprom type AM27C4096
  Version 6.00
  vnr 15.04.02 Cksum (provided) 0D08

*/
ROM_START( hotslots )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "hot_slots_v600_15.04.02.bin", 0x00000, 0x80000, CRC(35677999) SHA1(7462eef3734b9b6087102901967a168a60ab7710) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("hot_slots_24c02.bin",          0x0000,  0x0100,  CRC(fcac71ad) SHA1(1bb31e9a2d847430dc0d011f672cf3726dc6280c) )
ROM_END

/*
  QUINGO EXPORT Version 5.00

  Hardware PCB informations : E179465--A/02 LPL-CPU V4.0/MULTI GAME 8603186

  Eprom type ST M27c4002
  Version 5.00
  vnr 27.07.99 Cksum (provided) 79C5

  Eeprom : 24c04A

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |       |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| |QUINGO |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| | EXPORT|    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |       |    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   5.00|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |270799 |    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                             ___________                                      ___|
  |    ___________________                          |RTC2421 A  |   °°°°°                             ___|
  |   |   :::::::::::::   |                         |___________|    CON5                            Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz.
  Xtal 2:  8.000 MHz.
  Xtal 3: 19.660 MHz.

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

*/
ROM_START( quingo )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "quingo_export_v500_27.07.99.bin", 0x00000, 0x80000, CRC(2cd89fe3) SHA1(bdd256d5114227166aff1c9f84b573e5f00530fd) )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("quingo_24c04a.bin", 0x0000, 0x0200, BAD_DUMP CRC(d5e82b49) SHA1(7dbdf7d539cbd59a3ac546b6f50861c4958abb3a) ) // all AA & 55
ROM_END

/*
  BIG DEAL BELGIEN Version 5.04

  Hardware PCB informations : E179465--A/02 LPL-CPU V4.0/MULTI GAME 8603186
  Eprom type ST M27c4002
  Version 5.04
  vnr 21.05.01 Cksum (provided) C4B7

  Eeprom : 24c04A

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |  BIG  |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| |  DEAL |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| |BELGIEN|    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |       |    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   5.04|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |210501 |    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                             ___________                                      ___|
  |    ___________________                          |RTC2421 A  |   °°°°°                             ___|
  |   |   :::::::::::::   |                         |___________|    CON5                            Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz.
  Xtal 2:  8.000 MHz.
  Xtal 3: 19.660 MHz.

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

*/
ROM_START( bigdeal0 )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "big_deal_belgien_v504_21.05.01.bin", 0x00000, 0x80000, CRC(3e3484db) SHA1(78bb655deacc57ad041a46de7ef153ce25922a8a) )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("big_deal_24c04a.bin", 0x0000, 0x0200, BAD_DUMP CRC(d5e82b49) SHA1(7dbdf7d539cbd59a3ac546b6f50861c4958abb3a) ) // all AA & 55
ROM_END

/*
  BEL SLOTS EXP. Version 5.01

  Hardware PCB informations : E179465--A/02 LPL-CPU V4.0/MULTI GAME 8603186
  Eprom type ST M27c4002
  Version 5.01
  vnr 01.12.99 Cksum (provided) F718

  Eeprom : 24c04A

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |  BEL  |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| | SLOTS |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| |  EXP. |    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |       |    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   5.01|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |011299 |    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                             ___________                                      ___|
  |    ___________________                          |RTC2421 A  |   °°°°°                             ___|
  |   |   :::::::::::::   |                         |___________|    CON5                            Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz.
  Xtal 2:  8.000 MHz.
  Xtal 3: 19.660 MHz.

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

*/
ROM_START( belslots )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "bel_slots_exp_v501_01.12.99.bin", 0x00000, 0x80000, CRC(bd0b97ff) SHA1(9431359f91fd059c61441f4cb4924500889552a9) )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("bel_slots_exp_24c04a.bin", 0x0000, 0x0200, BAD_DUMP CRC(d5e82b49) SHA1(7dbdf7d539cbd59a3ac546b6f50861c4958abb3a) ) // all AA & 55
ROM_END

/*
  Puzzle Me!
  Impera.

  PCB layout:
   ___________________________________________________________________________________________________________________________ 
  |                      ___    ___    ___    ___                                                                             |
  |    ___              |   |  |   |  |   |  |   |                                                                            |
  |   | B |             | A |  | A |  | A |  | A |    ______________________________________                                  |____ 
  |   |___|    _____    |   |  |   |  |   |  |   |   |                                      |                                   ___|
  |           |  _  |   |___|  |___|  |___|  |___|   |               YAMAHA                 |                                   ___|
  |  _______  |BATTE|                                |               YM2149F                |                                   ___|
  | |LTC695C| |RY   |    ___    ___    ___    ___    |                                  IC17|                                   ___|
  | |_______| |  +  |   | E |  | E |  | E |  | E |   |______________________________________|                                   ___|
  |           |_____|   | M |  | M |  | M |  | M |                                                                              ___|
  |                     | P |  | P |  | P |  | P |           ___________________________           ______________      _____    ___|
  |   ____    ____      | T |  | T |  | T |  | T |          |                           |         |   ULN2803A   |    | O O |   ___|
  |  |HY62|  |HY62|     | Y |  | Y |  | Y |  | Y |          |       KDA0476CN_66        |         |______________|    | O O |  ____| 
  |  |64AL|  |64AL|     |___|  |___|  |___|  |___|          |       KOREA    219    IC20|        ________________     | O O | |
  |  |J_10|  |J_10|                                         |___________________________|       |    74HC273N    |    | O O | |
  |  |    |  |    |                                                                             |________________|    | O O | |
  |  |____|  |____|                                        ___________           ____________    ________________     | O O | |
  |                                                       | 74HC04AP  |         |EMPTY SOCKET|  |    74HC245N    |    | O O | |
  |  _______   _______                                    |___________|         |____________|  |________________|    | O O | |
  | |       | |       |   XTAL1                                                                                       |_____| |____ 
  | |       | |       |    _________________                 XTAL3                                 ______________        CON3   ___|
  | | EMPTY | | EMPTY |   |    IMPERA 8     |        _____    _____________     _____________     |  TD62083AP   |              ___|
  | | SOCKET| | SOCKET|   |     209751      |       |24C02|  |  PIC16C54   |   | HCF40106BE  |    |______________|              ___|
  | |       | |       |   |   DfD9227I3 Y   |       |_____|  |_____________|   |_____________|   ________________               ___|
  | |       | |       |   |                 |          IC26             IC29                    |    74HC273N    |              ___|
  | |       | |       |   |  SCC 66470 CAB  |                                                   |________________|              ___|
  | |       | |       |   |      317360     |                                                                                   ___|
  | |       | |       |   |   DfD9501I3 Y   |                                                    ________________               ___|
  | |  IC22A| |  IC21A|   |_________________|         __________________                        |    74HC245N    |              ___|
  | |_______| |_______|                  IC19        |                  |                       |________________|              ___|
  |  _______   _______                               |                  |                                                       ___|
  | |       | |       |                              |       ESI 1      |                                                       ___|
  | |       | |       |   XTAL2                      |       I9349      |                        ________________               ___|
  | |       | |       |    _________________         |                  |                       |    74HC245N    |              ___|
  | | EMPTY | |       |   |    IMPERA 7     |        |                  |                       |________________|              ___|
  | | SOCKET| |27C4002|   |     204440      |        |              IC25|                                                       ___|
  | |       | |       |   |   DfD9231V3 Y   |        |__________________|                                                       ___|
  | |       | |       |   |                 |                                                                                   ___|
  | |       | |       |   | SCC 68070 CCA84 |                                                                                   ___|
  | |       | |       |   |     324320      |                                                                                   ___|
  | |       | |       |   |   DfD9501V3 Y   |                                                                                   ___|
  | |       | |       |   |_________________|                                                                                   ___|
  | |   IC21| |   IC21|                  IC1               ______________    _________                                         ____| 
  | |_______| |_______|                                   |  RTC 72421A  |  | DS1207  |                                       |
  | IMPERA BOARD REV V2.1                                 |______________|  |_________|                                       |
  |___________________________________________________________________________________________________________________________|

  XTAL1 = 30.000
  XTAL2 = 19.6608
  XTAL3 = 3686.400

  A = KM44C256CJ_6
  B = TL7705ACP

*/
ROM_START( puzzleme )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(cd3bc5a9) SHA1(682f62eba454f4f00212b2a8dabb05d6747f22fd) )

	ROM_REGION( 0x1fff, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54.ic29",   0x0000, 0x1fff, CRC(6dd2bd8e) SHA1(380f6b952ddd3183e9ab5404866c30be015b3773) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("x24c02p.ic26",    0x0000, 0x0100, CRC(bc940f53) SHA1(6b870019752ba5c446a5ad5155e4a81dfbf6e523) )
ROM_END


/*
  Unknown 'TE06'

  PCB layout:
   ________________________________________________________________________________________________________________
  |                                                                                                                |
  |      __________                          _____________               ___________        ___                    |                            
  |     |  74LS04  |                        |LC324256BP-70|             |     C     |      |   |     ___           |__ 
  |     |__________|                        |_____________|             |___________|      |EMP|    |. .|           __|           
  |                                          _____________                                 |TY |    |. .|           __|
  |            ____                         |LC324256BP-70|                                |   |    |. .|           __|
  |           | A  |                        |_____________|                                |SOC|    |. .|           __|
  |           |____|                         _____________                                 |KET|    |. .|           __|
  |                                         |LC324256BP-70|                                |___|    |. .|           __|
  |                                         |_____________|           __________________            |. .|           __|
  |                                          _____________           |   ADV476KN35E    |           |. .|           __|    
  |                                         |LC324256BP-70|          |                  |           |___|           __|
  | _______                                 |_____________|          |     OF19802.3    |                          |
  ||DS1207 |                                                         |__________________|                          |
  ||_______|                                                                                                       |                       
  |         ___                       XTAL2                              __________                                |   
  |        |   |        ________          ________________              | PIC16C54 |                               |        
  |   ___  |PC7|       |        |        |                |             |__________|                               |__
  |  |   | |4HC|       |        |        |    IMPERA 8    |                   XTAL3                                 __|
  |  |HEF| |273|       |HYUNDAI |        |                |                                                         __|
  |  |400| |P  |       |        |        |                |                                                         __|
  |  |98B| |   |       |HY6264AL|        |     209751     |                                                         __|
  |  |P  | |   |       |P_10    |        |                |                                                         __|
  |  |   | |   |       |        |        |  DfD0922713 Y  |                                                         __|
  |  |___| |___|       |        |        |                |                                ___   ___                __|
  |                    | 9218A  |        |________________|              _____________    |   | |   |               __|
  |   _______          |        |                                       |  74HC245N   |   |PC7| |ULN|               __|    
  |  |       |         | KOREA  |                                       |_____________|   |4HC| |280|               __| 
  |  |BATTERY|         |________|                                                         |273| |3A |               __|
  |  |       |                                                                            |P  | |   |               __|
  |  |_______|     ________   ________                                                    |   | |   |               __|
  |               |        | |        |                                                   |   | |   |               __|
  |               |        | |        |                                  _____________    |___| |___|               __|
  |   ___         |        | |        |     XTAL1                       |  74HC245N   |    ___   ___                __|                                
  |  | B |        |        | |        |    __________________           |_____________|   |   | |   |               __|
  |  |___|        |        | |        |   |                  |                            |PC7| |ULN|               __|
  |               | EMPTY  | |        |   |    IMPERA 7      |                            |4HC| |280|               __|
  |               | SOCKET | |27C4002 |   |                  |                            |273| |3A |               __|
  |               |        | |        |   |     230031       |           _____________    |P  | |   |               __|   
  |   ___   ___   |        | |        |   |                  |          |  74HC245N   |   |   | |   |               __|
  |  |   | |   |  |        | |        |   |   DfD9249V3 Y    |          |_____________|   |   | |   |               __|   
  |  |PAL| |PAL|  |        | |        |   |                  |                            |___| |___|               __|
  |  |CE | |CE |  |        | |        |   |                  |                             __________               __|
  |  |   | |   |  |        | |        |   |                  |                            | CNY 74-4 |              __|
  |  |   | |   |  |        | |        |   |__________________|           _____________    |__________|              __|                     
  |  |   | |   |  |        | |        |                                 |  74HC245N   |                             __|
  |  |   | |   |  |        | |        |                                 |_____________|                             __|
  |  |___| |___|  |________| |________|                                                    ___________              __|
  |                                                                                       |   DIP 1   |            |
  | IMPERA BOARD REV V1.05                                                                |___________|            |
  |________________________________________________________________________________________________________________|

  A = TL7705ACP
  B = DS1210
  C = Cover scratched - unreadable

  XTAL1 = 19.6608
  XTAL2 = 30.000
  XTAL3 = 3.686JB

*/
ROM_START( unkte06 )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "m27c4002.bin", 0x00000, 0x80000, CRC(229a504f) SHA1(8033e9b4cb55f2364bf4606375ef9ac05fc715fe) )

	ROM_REGION( 0x1fff, "pic16c56", 0 )  // decapped
	ROM_LOAD("pic16c56.bin",   0x0000, 0x1fff, CRC(b5655603) SHA1(d9126c36f3fca7e769ea60aaa711bb304b4b6a11) )
ROM_END

/*
  Lucky 7
  Impera

  PCB layout:
   ________________________________________________________________________________________________________________
  |                                                                                                                |
  |      __________                          _____________               ___________        ___                    |                            
  |     |  74LS04  |                        |HY51C4256S-10|             |     C     |      |904|     ___           |__ 
  |     |__________|                        |_____________|             |___________|      |9  |    |. .|           __|           
  |                                          _____________                                 |   |    |. .|           __|
  |            ____                         |HY51C4256S-10|                                |LT1|    |. .|           __|
  |           | A  |                        |_____________|                                |081|    |. .|           __|
  |           |____|                         _____________                                 |CN |    |. .|           __|
  |                                         |HY51C4256S-10|                                |___|    |. .|           __|
  |                                         |_____________|           __________________            |. .|           __|
  |                                          _____________           |   ADV476KN35E    |           |. .|           __|    
  |                                         |HY51C4256S-10|          |                  |           |___|           __|
  | _______                                 |_____________|          |     OF19802.3    |                          |
  ||DS1207 |                                                         |__________________|                          |
  ||_______|                                                                                                       |                       
  |         ___                       XTAL2                                                                        |   
  |        |   |        ________          ________________                                                         |        
  |   ___  |PC7|       |        |        |                |                                                        |__
  |  |   | |4HC|       |        |        |    IMPERA 8    |                                                         __|
  |  |HEF| |273|       |HYUNDAI |        |                |                                                         __|
  |  |400| |A  |       |        |        |                |                                                         __|
  |  |98B| |   |       |HY6264AL|        |     160710     |                                                         __|
  |  |P  | |   |       |P_10    |        |                |                                                         __|
  |  |   | |   |       |        |        |  DTD9105I1 Y   |                                                         __|
  |  |___| |___|       |        |        |                |                                ___   ___                __|
  |                    | 9218A  |        |________________|              _____________    |   | |   |               __|
  |   _______          |        |                                       |  74HC245AP  |   |PC7| |ULN|               __|    
  |  |       |         | KOREA  |                                       |_____________|   |4HC| |280|               __| 
  |  |BATTERY|         |________|                                                         |273| |3A |               __|
  |  |       |                                                                            |AP | |   |               __|
  |  |_______|     ________   ________                                                    |   | |   |               __|
  |               |        | |        |                                                   |   | |   |               __|
  |               |        | |        |                                  _____________    |___| |___|               __|
  |   ___         |        | |        |     XTAL1                       |  74HC245AP  |    ___   ___                __|                                
  |  | B |        |        | |        |    __________________           |_____________|   |   | |   |               __|
  |  |___|        |        | |        |   |                  |                            |PC7| |ULN|               __|
  |               |        | |        |   |    IMPERA 7      |                            |4HC| |280|               __|
  |               |D27C210 | |D27C210 |   |                  |                            |273| |3A |               __|
  |               |        | |        |   |     155200       |           _____________    |AP | |   |               __|   
  |   ___   ___   |        | |        |   |                  |          |  74HC245AP  |   |   | |   |               __|
  |  |   | |   |  |GAME-ROM| |        |   |   DfD9101V3 Y    |          |_____________|   |   | |   |               __|   
  |  |PAL| |PAL|  | Lucky 7| |        |   |                  |                            |___| |___|               __|
  |  |16L| |16L|  |        | |        |   |                  |                             __________               __|
  |  |8  | |8  |  |VNr03-07| |        |   |                  |                            |  PC849   |              __|
  |  |   | |   |  |Sum.D882| |        |   |__________________|           _____________    |__________|              __|                     
  |  |   | |   |  |        | |        |                                 |  74HC245AP  |                             __|
  |  |   | |   |  |        | |        |                                 |_____________|                             __|
  |  |___| |___|  |________| |________|                                                    ___________              __|
  |                                                                                       |   DIP 1   |            |
  | IMPERA BOARD REV V1.04                                                                |___________|            |
  |________________________________________________________________________________________________________________|

  A = TL7705ACP
  B = DS1210
  C = Cover scratched - unreadable

  XTAL1 = 19.6608
  XTAL2 = 30.000

*/
ROM_START( lucky7i )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c210.6", 0x00000, 0x20000, CRC(3a99e9f3) SHA1(b9b533378ce514662cbd85a37ee138a2df760ed4) )
	ROM_LOAD16_WORD_SWAP( "27c210.5", 0x20000, 0x20000, CRC(b4da8856) SHA1(a33158d75047561fa9674ceb6b22cc63b5b49aed) )
ROM_END


/*
  Unknown 'W'
  Poker Game.
  Version: 1.2 200/93

  PCB layout:

  +---------------------------------------------------------------------------------------+
  |                                                                                       |
  |        +--------+                   +----------+          +---------+  +--+           |
  |        |74LS14N |                   |KM44C256CP|          |SAA1099P |  |  |           |
  | +--+   +--------+                   +----------+          +---------+  |E |           +---+
  | |GA|                                +----------+                       |  |             --|
  | |L |       +----+                   |KM44C256CP|                       |  |             --|
  | |16|       | A  |                   +----------+                       +--+             --|
  | |V8|       +----+                   +----------+                                        --|
  | |B |                                |KM44C256CP|       +--------------+                 --|
  | |  |                                +----------+       | KDA0476CN-66 |                 --|
  | +--+                                +----------+       | KOREA    219 |                 --|
  |   +------+                          |KM44C256CP|       |              |                 --|
  |   |DS1207|                          +----------+       +--------------+               +---+
  |   +------+                                                                            |
  |                                                                                       |
  |           +--+                    +-------------+        +---------+                  |
  | +--+ +--+ |  |   +------+         |SCC 66470 CAB|        |PIC16C58 |                  |
  | |  | |  | |  |   |LH5164|         |206880       |        +---------+                  +---+  
  | |B | |C | |D |   |D-10L |         |DfD9210I3 Y  |                                       --|
  | |  | |  | |  |   |      |         |             |                                       --|
  | |  | |  | |  |   |      |         | PHILIPS 1988|                                       --|
  | +--+ +--+ |  |   |      |         |             |                     +--+              --|
  |           +--+   |      |         |             |        +----------+ |  | +--+         --|
  |                  |      |         |             |        |PC74HC245P| |  | |  |         --|
  |                  |      |         +-------------+        +----------+ |F | |G |         --|
  |                  |      |                                             |  | |  |         --|
  |                  +------+                                             |  | |  |         --|
  |               +------+ +------+                          +----------+ +--+ +--+         --|
  |               |      | |      |                          |PC74HC245P| +--+              --|
  |     +----+    |      | |  W   |    +-------------+       +----------+ |  | +--+         --|
  |     | H  |    |      | |      |    |SCC 68070 CCA|                    |  | |  |         --|
  |     +----+    |      | |      |    |           84|                    |F | |G |         --|
  |               |      | |      |    |268340       |       +----------+ |  | |  |         --|
  |     +--+ +--+ |EMPTY | |M     |    |DfD9349V3 Y  |       |PC74HC245P| |  | |  |         --|
  |+--+ |PA| |PA| |  SLOT| |2     |    |             |       +----------+ +--+ +--+         --|
  ||D | |L | |L | |      | |7     |    | PHILIPS 1988|                    +--------+        --|
  ||I | |  | |  | |      | |C     |    |             |                    | PC849  |        --|
  ||P | |16| |16| |      | |4     |    |             |       +----------+ +--------+        --|
  ||  | |L8| |L8| |      | |0     |    +-------------+       |PC74HC245P|                   --|
  ||2 | |  | |  | |      | |0     |                          +----------+                   --|
  |+--+ +--+ +--+ |      | |2     |                                                       +---+
  |               +------+ +------+                                       +---------+     |
  |                                                                       |  DIP 1  |     |
  |                                                                       +---------+     |
  +---------------------------------------------------------------------------------------+

  A: TL7705ACP
  B: CD4040BE
  C: HEF40098BP
  D: PC74HC273P
  E: LT1081CN
  F: PC74HC273P
  G: ULN2803A
  H: DS1210


  DIP 1:
  +-------------------------------+
  |O N                            |
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  || | | | |#| |#| |#| |#| |#| |#||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  ||#| |#| | | | | | | | | | | | ||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  | 1   2   3   4   5   6   7   8 |
  +-------------------------------+

  DIP 2:
  +-------------------------------+
  |O N                            |
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  ||#| |#| |#| |#| |#| | | |#| | ||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  || | | | | | | | | | |#| | | |#||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  | 1   2   3   4   5   6   7   8 |
  +-------------------------------+

*/
ROM_START( unkpkr_w )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "w.bin", 0x00000, 0x80000, CRC(28300427) SHA1(83ea014a818246f476d769ad06cb2eba1ce699e8) )
ROM_END

/*
  Dallas Poker

  PCB layout:
  +------------------------------------------------------------------------------+
  |                                                           +-------------+    |
  | +---------+    XTAL1                                      |    +------+S|    |
  | |  EMPTY  |                                               |    |AHF   |u|    |
  | +---------+       +-------------+                         |    |Automa|b|    +---+
  | +---------+       |SCC 66470 CAB|                         |    |tentec|b|      --|
  | |P21014-07|       |466006       |       +---------+       |    |hnik  |o|      --|
  | +---------+       |IfD9205I3 Y  |       |SAA1099P |       | X  |      |a|      --|
  | +---------+       |             |       +---------+       | T  |8430 L|r|      --|
  | |  EMPTY  |       | PHILIPS 1988|                         | A  |eibnit|d|      --|
  | +---------+       |             |                         | L  |z     | |      --|
  | +---------+       |             |                         | 3  |      | |      --|
  | |P21014-07|       |             |       +---------------+ +--+ |Tel. 0| |      --|
  | +---------+       +-------------+       |  ADV476KN35E  |    | |3452/3| |    +---+
  | +---------+                             |   03-24 0S    |    | |249   | |    |
  | |  EMPTY  |                             |   0F19802.3   |    | +------+ |    |
  | +---------+                             +---------------+    +----------+    |
  | +---------+         XTAL2                                                    |
  | |P21014-07|       +-------------+                                            |
  | +---------+       |SCC 68070 CBA|      +----+                                |
  | +---------+       |           84|      | F  |                                +---+
  | |  EMPTY  |       |             |      +----+                                  --|
  | +---------+       |   203590    |                                              --|
  | +---------+       | DfD9218V3 Y |                       +---------+            --|
  | |P21014-07|       |             |                       |ULN 2803A|            --|
  | +---------+       | PHILIPS 1988|                       +---------+            --|
  |                   |             |      +----+                                  --|
  |+--+ +--+ +--+     +-------------+      | F  |             +----+               --|
  ||  | |A | |B |                          +----+             | G  |               --|
  ||C | +--+ +--+                                             +----+               --|
  ||  |                                                                            --|
  |+--++--+                                                                        --|
  |    |  |  +--+                          +----+                                  --|
  |    |D |  |  |                          | F  |           +---------+            --|
  |    |  |  |E |                          +----+           |ULN 2803A|            --|
  |    |  |  |  | +--------------------+                    +---------+            --|
  |    +--+  |  | |DALLAS POKER CZ/V1 P|                                           --|
  |          +--+ |VNR:19-09-93        |                      +----+               --|
  |               |SUM:8F8A/w   D27C210|                      | G  |               --|
  |    +--+  +--+ +--------------------+   +----+             +----+               --|
  |    |PA|  |PA|                          | F  |                                  --|
  |    |L |  |L | +--------------------+   +----+            +--------+            --|
  |    |16|  |16| |DALLAS POKER CZ/V1 B|                     | PC849  |            --|
  |    |L8|  |L8| |VNR:19-09-93        |                     +--------+          +---+
  |    |AC|  |AC| |SUM:EB91/w   D27C210|                     +---------+         |
  |    |  |  |  | +--------------------+                     |  DIP 1  |         |
  |    +--+  +--+                                            +---------+         |
  +------------------------------------------------------------------------------+

  XTAL1: 30.0000
  XTAL2: 19.6608
  XTAL3: 16.000

  A: TL7705ACP
  B: DS1210
  C: DS1207
  D: HEF40098BP / 759690T / Hnn9210P3
  E: SN74LS14N
  F: HC245A
  G: PC74HC273T

  Under the "DALLAS POKER CZ/V1 B" chip is a PC74HC273T chip soldered on the PCB.
  Under the "DALLAS POKER CZ/V1 P" chip is a MB8464A-10L chip soldered on the PCB.

  Subboard: Looks like an 40PIN MCU or PIC...only four wires connect the subboard 
  with the mainboard. (GND & VCC and PIN21 and PIN22 from the 40pin-MCU/PIC)


  DIP 1:
  +-------------------------------+
  |O N                            |
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  || | |O| | | | | |O| |O| |O| |O||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  ||O| | | |O| |O| | | | | | | | ||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  | 1   2   3   4   5   6   7   8 |
  +-------------------------------+

*/
ROM_START( dallaspk )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "cz-v1-p.bin", 0x00000, 0x20000, CRC(ad575e3f) SHA1(4e22957c42610fec0a96bd85f4b766422b020d88) )
	ROM_LOAD16_WORD_SWAP( "cz-v1-b.bin", 0x20000, 0x20000, CRC(2595d346) SHA1(34f09931d82b5376e4f3922222645c796dad0440) )
ROM_END


/*
  Kajot Card
  Version 1.01, Wien Euro.

  Amatic.

  PCB layout:
   ___________________________________________________________________________________________________________________________ 
  |                      ___    ___    ___    ___                                                                             |
  |    ___              |   |  |   |  |   |  |   |                                                                            |
  |   | B |             | A |  | A |  | A |  | A |    ______________________________________                                  |____ 
  |   |___|    _____    |   |  |   |  |   |  |   |   |                                      |                                   ___|
  |           |  _  |   |___|  |___|  |___|  |___|   |               YAMAHA                 |                                   ___|
  |  _______  |BATTE|                                |               YM2149F                |                                   ___|
  | |LTC695C| |RY   |    ___    ___    ___    ___    |                                  IC17|                                   ___|
  | |_______| |  +  |   | E |  | E |  | E |  | E |   |______________________________________|                                   ___|
  |           |_____|   | M |  | M |  | M |  | M |                                                                              ___|
  |                     | P |  | P |  | P |  | P |           ___________________________           ______________      _____    ___|
  |   ____    ____      | T |  | T |  | T |  | T |          |                           |         |   ULN2803A   |    | O O |   ___|
  |  |HY62|  |HY62|     | Y |  | Y |  | Y |  | Y |          |       KDA0476CN_50        |         |______________|    | O O |  ____| 
  |  |64AL|  |64AL|     |___|  |___|  |___|  |___|          |       KOREA   332B    IC20|        ________________     | O O | |
  |  |J_10|  |J_10|                                         |___________________________|       |    74HC273N    |    | O O | |
  |  |    |  |    |                                                                             |________________|    | O O | |
  |  |____|  |____|                                        ___________           ____________    ________________     | O O | |
  |                                                       | 74HC04AP  |         |EMPTY SOCKET|  |    74HC245N    |    | O O | |
  |  _______   _______                                    |___________|         |____________|  |________________|    | O O | |
  | |       | |       |   XTAL1                                                                                       |_____| |____ 
  | |       | |       |    _________________                 XTAL3                                 ______________        CON3   ___|
  | | EMPTY | | EMPTY |   |    IMPERA 8     |        _____    _____________     _____________     | EMPTY SOCKET |              ___|
  | | SOCKET| | SOCKET|   |     209751      |       |24C02|  | EMPTY SOCKET|   | EMPTY SOCKET|    |______________|              ___|
  | |       | |       |   |   DfD9227I3 Y   |       |_____|  |_____________|   |_____________|   ________________               ___|
  | |       | |       |   |                 |          IC26             IC29                    |    74HC273N    |              ___|
  | |       | |       |   |                 |                                                   |________________|              ___|
  | |       | |       |   |                 |                                                                                   ___|
  | |       | |       |   |                 |                                                    ________________               ___|
  | |  IC22A| |  IC21A|   |_________________|         __________________                        |    74HC245N    |              ___|
  | |_______| |_______|                  IC19        |                  |                       |________________|              ___|
  |  _______   _______                               |                  |                                                       ___|
  | |       | |       |                              |       ESI 1      |                                                       ___|
  | |  02   | |  01   |   XTAL2                      |       I9407      |                        ________________               ___|
  | |       | |       |    _________________         |                  |                       |    74HC245N    |              ___|
  | |       | |       |   |                 |        |                  |                       |________________|              ___|
  | |27C4002| |27C4002|   |                 |        |              IC25|                                                       ___|
  | |       | |       |   |                 |        |__________________|                                                       ___|
  | |       | |       |   |                 |                                                                                   ___|
  | |       | |       |   | SCC 68070 CCA84 |                                                                                   ___|
  | |       | |       |   |     288571      |                                                                                   ___|
  | |       | |       |   |   DfD9414V3 Y   |                                                                                   ___|
  | |       | |       |   |_________________|                                                                                   ___|
  | |   IC22| |   IC21|                  IC1               ______________    _________                                         ____| 
  | |_______| |_______|                                   |  RTC 72421A  |  |  EMPTY  |                                       |
  | IMPERA BOARD REV V2.1                                 |______________|  |_________|                                       |
  |___________________________________________________________________________________________________________________________|

  XTAL1 = 30.000
  XTAL2 = 19.6608
  XTAL3 = 3686.400

  A = KM44C256CJ-7
  B = TL7705ACP

*/
ROM_START( kajotcrd )
	ROM_REGION( 0x100000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "01.ic21", 0x00000, 0x80000, CRC(674aa36e) SHA1(483eb09950ff7c43a7147378f2e68d113c856905) )
	ROM_LOAD16_WORD_SWAP( "02.ic22", 0x80000, 0x80000, CRC(ae52803e) SHA1(27f917b0f8b302bdab930e304b4977a4b8192cd5) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("x24c02.ic26",    0x0000, 0x0100, CRC(0f143d6f) SHA1(c293728a997cd0868705dced55955072c6ebf5c0) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

void magicard_state::init_magicard()
{
	//...
}


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME        PARENT    MACHINE   INPUT     STATE           INIT           ROT    COMPANY      FULLNAME                                     FLAGS
GAME( 199?, magicard,   0,        magicard, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card (set 1)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 199?, magicarda,  magicard, magicard, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card (set 2)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 199?, magicardb,  magicard, magicard, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card (set 3)",                         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1994, magicarde,  magicard, hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card Export 94 (v2.11a, set 1)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1994, magicardea, magicard, hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card Export 94 (v2.11a, set 2)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1994, magicardeb, magicard, hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card Export 94 (v2.9a)",               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, magicardec, magicard, hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card Export (v4.01)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1994, magicardf,  magicard, hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Export (V.211A)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, magicardj,  0,        hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card III Jackpot (4.01)",              MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1993, magicardw,  magicard, magicard, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Card - Wien (Sicherheitsversion 1.2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2001, magicle,    0,        magicard, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Magic Lotto Export (5.03)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2002, hotslots,   0,        hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Hot Slots (6.00)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, quingo,     0,        hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Quingo Export (5.00)",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, belslots,   0,        hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Bel Slots Export (5.01)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2001, bigdeal0,   0,        hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Big Deal Belgien (5.04)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 199?, puzzleme,   0,        hotslots, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Puzzle Me!",                                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 199?, unkte06,    0,        magicard, magicard, magicard_state, init_magicard, ROT0, "Impera",    "unknown Poker 'TE06'",                       MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // strings in ROM
GAME( 199?, lucky7i,    0,        magicard, magicard, magicard_state, init_magicard, ROT0, "Impera",    "Lucky 7 (Impera)",                           MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1993, unkpkr_w,   0,        magicard, magicard, magicard_state, init_magicard, ROT0, "<unknown>", "unknown Poker 'W'",                          MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1993, dallaspk,   0,        magicard, magicard, magicard_state, init_magicard, ROT0, "<unknown>", "Dallas Poker",                               MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1993, kajotcrd,   0,        hotslots, magicard, magicard_state, init_magicard, ROT0, "Amatic",    "Kajot Card (Version 1.01, Wien Euro)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
