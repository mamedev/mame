// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*
BMC Bowling (c) 1994.05 BMC, Ltd

- TS 2004.10.22 - analog[at]op.pl

  Game is almost playable, especially with NVRAM_HACK (undefine it
  to get real nvram  access, but sometimes it's impossible to get back to
  title screen ).

 Controls:

 press START(1) OR BUTTON1 to start game , also START(1) or BUTTON1 to bowl / start
        ( 5 to insert coin(s) , B to bet , D to pay out (?)  etc...)

 press ANALIZER(0) durning boot to enter test menu, then :
            STOP1+STOP2 - sound test menu
        BIG(G) - cycle options ,
        DOUBLE(H) - play
            STOP1(X),STOP2(C) - change
            TAKE(A) - color test
            START(1) - exit
        BET(B)+START(1) - other tests
        START(1) - next test

 press START(1)+HP(S) durning boot to see stats

 press CONFIRM(N) durning boot, to enter    settings
        BET(B) - change page
        STOP1(X)/STOP3(V) - modify
        START(1)/SMALL(F) - move
        KEY DOWN(D) - default ?

TODO:

 - scroll (writes to $91800 and VIA port A - not used in game (only in test mode))
 - VIA 6522(ports)
 - Crt
 - interrupts
 - missing gfx elements

---

Chips:
MC68000P10
Goldstar GM68B45S (same as Hitachi HD6845 CTR Controller)*
Winbond WF19054 (same as AY3-8910)
MK28 (appears to be a AD-65, AKA OKI6295) next to rom 10
Synertek SY6522 VIA
9325-AG (Elliptical Filter)
KDA0476BCN-66 (RAMDAC)
part # scratched 64 pin PLCC next to 7EX & 8 EX

Ram:
Goldstar GM76C28A (2Kx8 SRAM) x3
HM62256LP-12 x6

OSC:
GREAT1 21.47727
13.3M
3.579545

DIPS:
Place for 4 8 switch dips
dips 1 & 3 are all connected via resitors
dips 2 & 4 are standard 8 switch dips

EEPROM       Label         Use
----------------------------------------
ST M27C1001  bmc_8ex.bin - 68K code 0x00
ST M27C1001  bmc_7ex.bin - 68K code 0x01
ST M27C512   bmc_3.bin\
ST M27C512   bmc_4.bin | Graphics
ST M27C512   bmc_5.bin |
ST M27C512   bmc_6.bin/
HM27C101AG   bmc_10.bin - Sound samples

BrianT

Top board:
          --- Edge Connection ---
GS9403      7EX    8EX   Part #      Misc HD74LS374p
                         Scratched   HM62256   HM62256
                         PLC
  OKI                                ST T74L6245BI
                  PEEl 18CV8P
          10



Main board:
          --- Edge Connection ---              JAMMA Connection
  68K     GM76C28A         3   5   GMC76C28A
          GM76C28A         4   6   GM68B45S   BATTERY

21.47727MHz                                 WF19054
                                                      DIP1
13.3Mhz                                               DIP2
  SY6522            HM62256 x4  3.579545MHz 9325-AG   DIP3
                                   KDA0476BCN-66      DIP4
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "video/ramdac.h"

#define NVRAM_HACK


class bmcbowl_state : public driver_device
{
public:
	bmcbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_stats_ram(*this, "nvram", 16),
		m_vid1(*this, "vid1"),
		m_vid2(*this, "vid2"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT8> m_stats_ram;
	required_shared_ptr<UINT16> m_vid1;
	required_shared_ptr<UINT16> m_vid2;
	required_device<palette_device> m_palette;
	int m_bmc_input;
	DECLARE_READ16_MEMBER(bmc_random_read);
	DECLARE_READ16_MEMBER(bmc_protection_r);
	DECLARE_WRITE16_MEMBER(scroll_w);
	DECLARE_READ8_MEMBER(via_b_in);
	DECLARE_WRITE8_MEMBER(via_a_out);
	DECLARE_WRITE8_MEMBER(via_b_out);
	DECLARE_WRITE_LINE_MEMBER(via_ca2_out);
	DECLARE_READ8_MEMBER(dips1_r);
	DECLARE_WRITE8_MEMBER(input_mux_w);
	DECLARE_DRIVER_INIT(bmcbowl);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_bmcbowl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void init_stats(const UINT8 *table, int table_len, int address);
};




void bmcbowl_state::video_start()
{
}

UINT32 bmcbowl_state::screen_update_bmcbowl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/*
      280x230,4 bitmap layers, 8bpp,
        missing scroll and priorities   (maybe fixed ones)
*/

	int x,y,z,pixdat;
	bitmap.fill(m_palette->black_pen(), cliprect);

	z=0;
	for (y=0;y<230;y++)
	{
		for (x=0;x<280;x+=2)
		{
			pixdat = m_vid2[0x8000+z];

			if(pixdat&0xff)
				bitmap.pix16(y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				bitmap.pix16(y, x) = (pixdat>>8);

			pixdat = m_vid2[z];

			if(pixdat&0xff)
				bitmap.pix16(y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				bitmap.pix16(y, x) = (pixdat>>8);

			pixdat = m_vid1[0x8000+z];

			if(pixdat&0xff)
				bitmap.pix16(y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				bitmap.pix16(y, x) = (pixdat>>8);

			pixdat = m_vid1[z];

			if(pixdat&0xff)
				bitmap.pix16(y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				bitmap.pix16(y, x) = (pixdat>>8);

			z++;
		}
	}
	return 0;
}

READ16_MEMBER(bmcbowl_state::bmc_random_read)
{
	return machine().rand();
}

READ16_MEMBER(bmcbowl_state::bmc_protection_r)
{
	switch(space.device().safe_pcbase())
	{
		case 0xca68:
			switch(space.device().state().state_int(M68K_D2))
			{
				case 0:          return 0x37<<8;
				case 0x1013: return 0;
				default:         return 0x46<<8;
			}
	}
	logerror("Protection read @ %X\n",space.device().safe_pcbase());
	return machine().rand();
}

WRITE16_MEMBER(bmcbowl_state::scroll_w)
{
	//TODO - scroll
}


READ8_MEMBER(bmcbowl_state::via_b_in)
{
	return ioport("IN3")->read();
}


WRITE8_MEMBER(bmcbowl_state::via_a_out)
{
	// related to video hw ? BG scroll ?
}

WRITE8_MEMBER(bmcbowl_state::via_b_out)
{
	//used
}

WRITE_LINE_MEMBER(bmcbowl_state::via_ca2_out)
{
	//used
}


// 'working' NVRAM

#ifdef NVRAM_HACK
static const UINT8 bmc_nv1[]=
{
	0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x13,0x88,0x46,0xDD,0x0F,0xA0,
	0x5A,0xF5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x6E,0x55,
	0x55,0x55,0x3B,0x00,0x00,0x00,0x06,0x55,0x55,0x55,0x53,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,
	0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x6E,0x55,0x55,0x55,0x3B,0x00,0x00,0x00,0x06,0x55,0x55,0x55,0x53,0x00,
	0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,
	0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0xFF,0x00,0x0A,0x00,0x0A,
	0x00,0x32,0x00,0x02,0x28,0x32,0x5C,0x0A,0x03,0x03,0xD6,0x66,0xFF,0xFF,0xFF,0xFF,0x5D,0xED,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x25,0xD5,0x25,0x1C,0x00,0x00,0x00,0x00,
	0x00,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6E,
	0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0xDC,0x00,0xFF,0xFF,0xFF,0xFF
};

static const UINT8 bmc_nv2[]=
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x03,0x00,0x09,0x00,0x00,0x2B,0xF1,
	0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};

static const UINT8 bmc_nv3[]=
{
	0xFA,0xFF,0x01,0x02,0x04,0x0A,0x1E,0xC8,0x02,0x01,0xFF,0xFF,0xFF,0xFF,0xFF
};


void bmcbowl_state::init_stats(const UINT8 *table, int table_len, int address)
{
	for (int i = 0; i < table_len; i++)
		m_stats_ram[address+2*i]=table[i];
}
#endif

void bmcbowl_state::machine_reset()
{
#ifdef NVRAM_HACK
	for (int i = 0; i < m_stats_ram.bytes(); i++)
		m_stats_ram[i] = 0xff;

	init_stats(bmc_nv1,ARRAY_LENGTH(bmc_nv1),0);
	init_stats(bmc_nv2,ARRAY_LENGTH(bmc_nv2),0x3b0);
	init_stats(bmc_nv3,ARRAY_LENGTH(bmc_nv3),0xfe2);
#endif
}

static ADDRESS_MAP_START( bmcbowl_mem, AS_PROGRAM, 16, bmcbowl_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM

	AM_RANGE(0x090000, 0x090001) AM_DEVWRITE8("ramdac", ramdac_device, index_w, 0x00ff)
	AM_RANGE(0x090002, 0x090003) AM_DEVWRITE8("ramdac", ramdac_device, pal_w, 0x00ff)
	AM_RANGE(0x090004, 0x090005) AM_DEVWRITE8("ramdac", ramdac_device, mask_w, 0x00ff)

	AM_RANGE(0x090800, 0x090803) AM_WRITENOP
	AM_RANGE(0x091000, 0x091001) AM_WRITENOP
	AM_RANGE(0x091800, 0x091801) AM_WRITE(scroll_w)

	AM_RANGE(0x092000, 0x09201f) AM_DEVREADWRITE8("via6522_0", via6522_device, read, write, 0x00ff)

	AM_RANGE(0x093000, 0x093003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0x00ff)
	AM_RANGE(0x092800, 0x092803) AM_DEVWRITE8("aysnd", ay8910_device, data_address_w, 0xff00)
	AM_RANGE(0x092802, 0x092803) AM_DEVREAD8("aysnd", ay8910_device, data_r, 0xff00)
	AM_RANGE(0x093802, 0x093803) AM_READ_PORT("IN0")
	AM_RANGE(0x095000, 0x095fff) AM_RAM AM_SHARE("nvram") /* 8 bit */
	AM_RANGE(0x097000, 0x097001) AM_READNOP
	AM_RANGE(0x140000, 0x1bffff) AM_ROM
	AM_RANGE(0x1c0000, 0x1effff) AM_RAM AM_SHARE("vid1")
	AM_RANGE(0x1f0000, 0x1fffff) AM_RAM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM AM_SHARE("vid2")

	AM_RANGE(0x28c000, 0x28c001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff00)

	/* protection device*/
	AM_RANGE(0x30c000, 0x30c001) AM_WRITENOP
	AM_RANGE(0x30c040, 0x30c041) AM_WRITENOP
	AM_RANGE(0x30c080, 0x30c081) AM_WRITENOP
	AM_RANGE(0x30c0c0, 0x30c0c1) AM_WRITENOP
	AM_RANGE(0x30c100, 0x30c101) AM_READ(bmc_protection_r)
	AM_RANGE(0x30c140, 0x30c141) AM_WRITENOP
	AM_RANGE(0x30ca00, 0x30ca01) AM_READ(bmc_random_read) AM_WRITENOP
ADDRESS_MAP_END


static INPUT_PORTS_START( bmcbowl )
	PORT_START("IN0")   /* DSW 1 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Note")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Analizer")

	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 1") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 2") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 3") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Bet") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Confirm") PORT_CODE(KEYCODE_N)


	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Start")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Take") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("HP") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key Down") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Small") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Big") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double") PORT_CODE(KEYCODE_H)

	PORT_START("IN1")   /* DSW 2 */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, "1 COIN 10 CREDITS" )
	PORT_DIPSETTING(    0x00, "2 COINS 10 CREDITS" )

	PORT_DIPNAME( 0x01, 0x00, "DSW2 8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2 7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2 6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2 5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x00, "DSW2 2" )
	PORT_DIPSETTING(    0x040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2 1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")   /* DSW 4 */
	PORT_DIPNAME( 0x01, 0x00, "DSW4 8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW4 7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW4 6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW4 5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW4 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW4 3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW4 2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW4 1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

INPUT_PORTS_END

READ8_MEMBER(bmcbowl_state::dips1_r)
{
	switch(m_bmc_input)
	{
			case 0x00:  return  ioport("IN1")->read();
			case 0x40:  return  ioport("IN2")->read();
	}
	logerror("%s:unknown input - %X\n",machine().describe_context(),m_bmc_input);
	return 0xff;
}


WRITE8_MEMBER(bmcbowl_state::input_mux_w)
{
	m_bmc_input=data;
}

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, bmcbowl_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb666_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( bmcbowl, bmcbowl_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_21_4772MHz / 2 )
	MCFG_CPU_PROGRAM_MAP(bmcbowl_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bmcbowl_state, irq2_line_hold)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(35*8, 30*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 35*8-1, 0*8, 29*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bmcbowl_state, screen_update_bmcbowl)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz )  // guessed chip type, clock not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_SOUND_ADD("aysnd", AY8910, XTAL_3_579545MHz / 2)
	MCFG_AY8910_PORT_A_READ_CB(READ8(bmcbowl_state, dips1_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(bmcbowl_state, input_mux_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_OKIM6295_ADD("oki", 1122000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	/* via */
	MCFG_DEVICE_ADD("via6522_0", VIA6522, 1000000)
	MCFG_VIA6522_READPB_HANDLER(READ8(bmcbowl_state,via_b_in))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(bmcbowl_state, via_a_out))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(bmcbowl_state, via_b_out))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(bmcbowl_state, via_ca2_out))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m68000_device, write_irq4))
MACHINE_CONFIG_END

ROM_START( bmcbowl )
	ROM_REGION( 0x200000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "bmc_8ex.bin", 0x000000, 0x10000, CRC(8b1aa5db) SHA1(879df950bedf2c163ba89d983ca4a0691b01c46e) )
	ROM_LOAD16_BYTE( "bmc_7ex.bin", 0x000001, 0x10000, CRC(7726d47a) SHA1(8438c3345847c2913c640a29145ec8502f6b01e7) )

	ROM_LOAD16_BYTE( "bmc_4.bin", 0x140000, 0x10000, CRC(f43880d6) SHA1(9e73a29baa84d417ff88026896d852567a38e714) )
	ROM_RELOAD(0x160000,0x10000)
	ROM_LOAD16_BYTE( "bmc_3.bin", 0x140001, 0x10000, CRC(d1af9410) SHA1(e66b3ddd9d9e3c567fdb140c4c8972c766f2b975) )
	ROM_RELOAD(0x160001,0x10000)

	ROM_LOAD16_BYTE( "bmc_6.bin", 0x180000, 0x20000, CRC(7b9e0d77) SHA1(1ec1c92c6d4c512f7292b77e9663518085684ba9) )
	ROM_LOAD16_BYTE( "bmc_5.bin", 0x180001, 0x20000, CRC(708b6f8b) SHA1(4a910126d87c11fed99f44b61d51849067eddc02) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "bmc_10.bin", 0x00000, 0x20000,  CRC(f840c17f) SHA1(82891a85c8dc60f727b5a8c8e8ab09e8e4bd8af4) )

ROM_END

DRIVER_INIT_MEMBER(bmcbowl_state,bmcbowl)
{
	save_item(NAME(m_bmc_input));
}

GAME( 1994, bmcbowl,    0, bmcbowl,    bmcbowl, bmcbowl_state,    bmcbowl, ROT0,  "BMC", "Konkyuu no Hoshi", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)
