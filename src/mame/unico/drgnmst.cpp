// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Dragon Master (c)1994 Unico

the hardware seems to fall somewhere between the
hardware playmark commonly used and the hardware
unico used for zero point etc.

(also seems to be related to cps1 bootlegs?)

PCB Layout
----------

  DM1001    DM1002   6116   6116     DM1003
    6295 6295        6116   *PLCC84  DM1004
                     6116            DM1005
     PIC16C55                        DM1006

       6116            6116          DM1007
       6116            6116          DM1008
                       6116
                       6116
                                     TPC1020
DSW1 62256   62256                   (PLCC84)
    DM1000A DM1000B
DSW2  MC68000P12   TPC1020           62256
                   (PLCC84)          62256
12MHz 32MHz


Notes:
          *: Unknown PLCC84 chip (surface scratched)
      VSync: 60Hz
      HSync: 15.625kHz
  68K clock: 12MHz

***********************************************************

Master's Fury (c)1996 Unico

PCB Layout
+---------------------------------------------------------+
|               MF-01       MCM2018                       |
|                           MCM2018                M  M   |
|  VOL          M6295       MCM2018                F  F   |
|                           MCM2018                0  0   |
|    YM3012     YM2151                             4  5   |
|                                      6116               |
|                                      6116               |
|                 GM76C28K             6116               |
|J                GM76C28K             6116     +-------+ |
|A                                              |  AMD  | |
|M                                              |MACH210| |
|M                                              +-------+ |
|A     KM62256BPL KM62256BPL      +--------+  +--------+  |
|                                 |  ACTEL |  |  ACTEL |  |
|          MF-02   MF-03          | A1020B |  |A40MX04F|  |
| DIP-SW   +----------+           | PLCC84 |  | PLCC84 |  |
|          |MC68000P12|           +--------+  +--------+  |
| DIP-SW   +----------+                            M  M   |
|                                                  F  F   |
|LED      14.31818MHz   32.000MHz KM62256BPL       0  0   |
|                       20.000MHz KM62256BPL       6  7   |
+---------------------------------------------------------+

  CPU: MC68000P12
Sound: OKI M6295 & YM3012/YM2151 rebadged as 7105/BS901
  OSC: 14.31818MHz, 20.000MHz & 32.000MHz
  DSW: 8-switch dipswitch x 2
  VOL: Volume pot
  LED: Power on LED
Other: Actel A1020B (labeled LOGIC-A), Actel A40MX04-F (labeled LOGIC-B)
       AMD MACH210 (labeled LOGIC-C)

ROMS:
MF-01 - UNICO 1        - Sound sample       - 27C020
MF-02 - UNICO 2        - Program ROM (Even) - 27C040
MF-03 - UNICO 3        - Program ROM (Odd)  - 27C040
MF-04 - MF0032-1 UNICO - Sprites            - mask ROM read as 27C322
MF-05 - MF0032-2 UNICO - Sprites            - mask ROM read as 27C322
MF-06 - MF0016-3 UNICO - Background         - mask ROM read as 27C160
MF-07 - MF0016-4 UNICO - Background         - mask ROM read as 27C160

MF-0x are locations silkscreened on the PCB

*/

#include "emu.h"
#include "drgnmst.h"

#include "cpu/m68000/m68000.h"
#include "screen.h"
#include "speaker.h"


void drgnmst_base_state::coin_w(uint16_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x100);
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x400);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x800);
}

void drgnmst_pic_state::snd_command_w(uint8_t data)
{
	m_snd_command = data;
	m_maincpu->yield();
}

void drgnmst_pic_state::snd_flag_w(uint8_t data)
{
	/* Enables the following 68K write operation to latch through to the PIC */
	m_snd_flag = 1;
}


uint8_t drgnmst_pic_state::pic16c5x_port0_r()
{
	return m_pic16c5x_port0;
}

uint8_t drgnmst_pic_state::snd_command_r()
{
	uint8_t data = 0;

	switch (m_oki_control & 0x1f)
	{
		case 0x12:  data = (m_oki[1]->read() & 0x0f); break;
		case 0x16:  data = (m_oki[0]->read() & 0x0f); break;
		case 0x0b:
		case 0x0f:  data = m_snd_command; break;
		default:    break;
	}

	return data;
}

uint8_t drgnmst_pic_state::snd_flag_r()
{
	if (m_snd_flag)
	{
		m_snd_flag = 0;
		return 0x40;
	}

	return 0x00;
}

void drgnmst_pic_state::pcm_banksel_w(uint8_t data)
{
	/*  This is a 4 bit port.
	    Each pair of bits is used in part of the OKI PCM ROM bank selectors.
	    See the Port 2 write handler below (snd_control_w) for details.
	*/

	m_pic16c5x_port0 = data;
}

void drgnmst_pic_state::oki_w(uint8_t data)
{
	m_oki_command = data;
}

void drgnmst_pic_state::snd_control_w(uint8_t data)
{
	/*  This port controls communications to and from the 68K, both OKI
	    devices, and part of the OKI PCM ROM bank selection.

	    bit legend
	    7w  ROM bank select for OKI-1, bit 2. Bank bits 1 & 0 are on Port 0
	    6r  Flag from 68K to notify the PIC that a command is coming
	    5w  ROM bank select for OKI-0, bit 2. Bank bits 1 & 0 are on Port 0
	    4w  Set Port 1 to read sound to play command from 68K. (active low)
	    3w  OKI enable comms? (active low)
	    2w  OKI chip select? (0=OKI-1, 1=OKI-0)
	    1w  Latch write data to OKI? (active low)
	    0w  Activate read signal to OKI? (active low)

	    The PCM ROM bank selects are 3 bits wide.
	    2 bits for each OKI BANK selects are on Port 0, and the third most
	    significant bit is here. The MSb is written here immediately after
	    writing to Port 0 so we handle the bank switching here.
	    The PIC16C55 only supports bank selections for:
	     OKI0 from 1 to 5  (Each bank selection switches the $20000-3ffff area)
	     OKI1 from 0 to 7  (Each bank selection switches the entire $40000 area)
	    The OKI0 banks are pre-configured below in the driver init.
	*/

	int oki_new_bank;
	m_oki_control = data;


	oki_new_bank = ((m_pic16c5x_port0 & 0xc) >> 2) | ((m_oki_control & 0x80) >> 5);
	if (oki_new_bank != m_oki_bank[0])
	{
		m_oki_bank[0] = oki_new_bank;
		if (m_oki_bank[0])
			oki_new_bank--;
		m_oki1bank->set_entry(oki_new_bank);
	}

	oki_new_bank = ((m_pic16c5x_port0 & 0x3) >> 0) | ((m_oki_control & 0x20) >> 3);
	if (oki_new_bank != m_oki_bank[1])
	{
		m_oki_bank[1] = oki_new_bank;
		m_oki[1]->set_rom_bank(oki_new_bank);
	}

	switch (m_oki_control & 0x1f)
	{
	case 0x11:
//                  logerror("Writing %02x to OKI1", m_oki_command);
//                  logerror(", PortC=%02x, Code=%02x, Bank0=%01x, Bank1=%01x\n", m_oki_control, m_snd_command, m_oki_bank[0], m_oki_bank[1]);
		m_oki[1]->write(m_oki_command);
		break;
	case 0x15:
//                  logerror("Writing %02x to OKI0", m_oki_command);
//                  logerror(", PortC=%02x, Code=%02x, Bank0=%01x, Bank1=%01x\n", m_oki_control, m_snd_command, m_oki_bank[0], m_oki_bank[1]);
		m_oki[0]->write(m_oki_command);
		break;
	default:    break;
	}
}


/***************************** 68000 Memory Map *****************************/

void drgnmst_base_state::drgnmst_main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x800000, 0x800001).portr("P1_P2");
	map(0x800018, 0x800019).portr("SYSTEM");
	map(0x80001a, 0x80001b).portr("DSW1");
	map(0x80001c, 0x80001d).portr("DSW2");
	map(0x800030, 0x800031).w(FUNC(drgnmst_base_state::coin_w));
	map(0x800100, 0x800121).writeonly().share("vidregs");
	map(0x800154, 0x800155).writeonly().share("vidregs2"); // seems to be priority control
	map(0x800176, 0x800177).portr("EXTRA");
	map(0x8001e0, 0x8001e1).nopw();
	map(0x900000, 0x903fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x904000, 0x907fff).ram().w(FUNC(drgnmst_base_state::md_videoram_w)).share("md_videoram");
	map(0x908000, 0x90bfff).ram().w(FUNC(drgnmst_base_state::bg_videoram_w)).share("bg_videoram");
	map(0x90c000, 0x90ffff).ram().w(FUNC(drgnmst_base_state::fg_videoram_w)).share("fg_videoram");
	map(0x920000, 0x923fff).ram().share("rowscrollram"); // rowscroll ram
	map(0x930000, 0x9307ff).ram().share("spriteram");   // Sprites
	map(0xff0000, 0xffffff).ram();
}

void drgnmst_pic_state::drgnmst_main_map_with_pic(address_map& map)
{
	drgnmst_main_map(map);
	map(0x800181, 0x800181).w(FUNC(drgnmst_pic_state::snd_command_w));
	map(0x800189, 0x800189).w(FUNC(drgnmst_pic_state::snd_flag_w));
}

void drgnmst_ym_state::drgnmst_main_map_with_ym(address_map& map)
{
	drgnmst_main_map(map);
	map(0x800180, 0x800183).umask16(0x00ff).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x800189, 0x800189).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


void drgnmst_pic_state::drgnmst_oki1_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr("oki1bank");
}


static INPUT_PORTS_START( drgnmst )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") /* Test mode screen shows 3 sets of dipswitches */
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
/*  PORT_DIPSETTING(      0x0300, DEF_STR( Off ) ) */
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Two credits to start" )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR ( Free_Play ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Game Pause" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0700, 0x0400, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0700, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( Easier ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Harder ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Language ) )     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Korean ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Game_Time ) )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Short" )
	PORT_DIPNAME( 0x2000, 0x2000, "Stage Skip" )            PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Spit Color" )            PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "Grey" )
	PORT_DIPSETTING(      0x0000, "Red" )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("EXTRA")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout char8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 24,8,16, 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static const gfx_layout char16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 24, 8,16,0 },
	{ RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+1,RGN_FRAC(1,2)+2,RGN_FRAC(1,2)+3,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+5,RGN_FRAC(1,2)+6,RGN_FRAC(1,2)+7,
		0,1,2,3,4,5,6,7 },
	{ 0*32,1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32,9*32,10*32,11*32,12*32,13*32,14*32,15*32 },
	16*32
};



static const gfx_layout char32x32_layout =
{
	32,32,
	RGN_FRAC(1,2),
	4,
	{ 24,8, 16,0 },
	{   RGN_FRAC(1,2)+0,RGN_FRAC(1,2)+1,RGN_FRAC(1,2)+2,RGN_FRAC(1,2)+3,RGN_FRAC(1,2)+4,RGN_FRAC(1,2)+5,RGN_FRAC(1,2)+6,RGN_FRAC(1,2)+7,
		0,1,2,3,4,5,6,7,
		RGN_FRAC(1,2)+32,RGN_FRAC(1,2)+33,RGN_FRAC(1,2)+34,RGN_FRAC(1,2)+35,RGN_FRAC(1,2)+36,RGN_FRAC(1,2)+37,RGN_FRAC(1,2)+38,RGN_FRAC(1,2)+39,
		32,33,34,35,36,37,38,39 },

	{    0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64,
		16*64,17*64,18*64,19*64,20*64,21*64,22*64,23*64,
		24*64,25*64,26*64,27*64,28*64,29*64,30*64,31*64 },
	32*64
};


static GFXDECODE_START( gfx_drgnmst )
	GFXDECODE_ENTRY( "gfx1", 0, char16x16_layout,   0,     0x20 ) /* sprite tiles */
	GFXDECODE_ENTRY( "gfx2", 0, char8x8_layout,     0x200, 0x20 ) /* fg tiles */
	GFXDECODE_ENTRY( "gfx2", 0, char16x16_layout,   0x400, 0x20 ) /* md tiles */
	GFXDECODE_ENTRY( "gfx2", 0, char32x32_layout,   0x600, 0x20 ) /* bg tiles */
GFXDECODE_END


void drgnmst_base_state::machine_start()
{
}

void drgnmst_pic_state::machine_start()
{
	drgnmst_base_state::machine_start();
	save_item(NAME(m_snd_flag));
	save_item(NAME(m_oki_control));
	save_item(NAME(m_oki_command));
	save_item(NAME(m_pic16c5x_port0));
	save_item(NAME(m_oki_bank));
}

void drgnmst_base_state::machine_reset()
{
}

void drgnmst_pic_state::machine_reset()
{
	drgnmst_base_state::machine_reset();

	m_snd_flag = 0;
	m_oki_control = 0;
	m_oki_command = 0;
	m_pic16c5x_port0 = 0;
	m_oki_bank[1] = 0;
	m_oki_bank[0] = 0;
}


void drgnmst_base_state::drgnmst(machine_config &config)
{
	M68000(config, m_maincpu, 12_MHz_XTAL); /* Confirmed */
	m_maincpu->set_addrmap(AS_PROGRAM, &drgnmst_base_state::drgnmst_main_map);
	m_maincpu->set_vblank_int("screen", FUNC(drgnmst_base_state::irq2_line_hold));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_drgnmst);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8*8, 56*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(drgnmst_base_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));

	BUFFERED_SPRITERAM16(config, m_spriteram);

	PALETTE(config, m_palette).set_format(2, &drgnmst_base_state::drgnmst_IIIIRRRRGGGGBBBB, 0x2000);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

}

void drgnmst_pic_state::drgnmst_with_pic(machine_config& config)
{
	drgnmst(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &drgnmst_pic_state::drgnmst_main_map_with_pic);

	PIC16C55(config, m_audiocpu, 32_MHz_XTAL / 8);  /* 4MHz - Confirmed */
	m_audiocpu->read_a().set(FUNC(drgnmst_pic_state::pic16c5x_port0_r));
	m_audiocpu->write_a().set(FUNC(drgnmst_pic_state::pcm_banksel_w));
	m_audiocpu->read_b().set(FUNC(drgnmst_pic_state::snd_command_r));
	m_audiocpu->write_b().set(FUNC(drgnmst_pic_state::oki_w));
	m_audiocpu->read_c().set(FUNC(drgnmst_pic_state::snd_flag_r));
	m_audiocpu->write_c().set(FUNC(drgnmst_pic_state::snd_control_w));

	OKIM6295(config, m_oki[0], 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[0]->set_addrmap(0, &drgnmst_pic_state::drgnmst_oki1_map);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 1.00);

	OKIM6295(config, m_oki[1], 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 1.00);
}

void drgnmst_ym_state::drgnmst_ym(machine_config& config)
{
	drgnmst(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &drgnmst_ym_state::drgnmst_main_map_with_ym);

	ym2151_device &ym2151(YM2151(config, "ymsnd", XTAL(14'318'181)/4));  /* verified on pcb */
	ym2151.add_route(0, "mono", 0.05);
	ym2151.add_route(1, "mono", 0.05);

	OKIM6295(config, m_oki, 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.90);
}

ROM_START( mastfury )
	ROM_REGION( 0x100000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "master012.4m", 0x000000, 0x080000, CRC(020a3c50) SHA1(d6762b66f06fe91f3bff8cdcbff42c247df64671) )
	ROM_LOAD16_BYTE( "master013.4m", 0x000001, 0x080000, CRC(1e7dd287) SHA1(67764aa054731a0548f6c7d3b898597792d96eec) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "dmast96_voi1x", 0x00000, 0x40000, CRC(fc5161a1) SHA1(999e73e36df317aabefebf94444690f439d64559) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprites (16x16x4) */
	// Some versions of the game use 2x32MBit ROMs instead.
	// This set comes from a PCB marked "Dragon Master 96" but the PCB was missing program ROMs, was Dragon Master 96
	// an alt title for the Chinese market?
	ROM_LOAD16_BYTE( "dmast96_mvo1x", 0x000000, 0x080000, CRC(9a597497) SHA1(4f63e17629a00fa505e2165f7fa46f0c5ef2bc60) )
	ROM_CONTINUE(0x400000, 0x080000)
	ROM_CONTINUE(0x100000, 0x080000)
	ROM_CONTINUE(0x500000, 0x080000)
	ROM_LOAD16_BYTE( "dmast96_mvo3x", 0x000001, 0x080000, CRC(be01b829) SHA1(ab9858aadb0bba8415c674e27f9ea905de27871c) )
	ROM_CONTINUE(0x400001, 0x080000)
	ROM_CONTINUE(0x100001, 0x080000)
	ROM_CONTINUE(0x500001, 0x080000)
	ROM_LOAD16_BYTE( "dmast96_mvo2x", 0x200000, 0x080000, CRC(3eab296c) SHA1(d2add71e01aa6bd1b6539e72ed373bb71f65c437) )
	ROM_CONTINUE(0x600000, 0x080000)
	ROM_LOAD16_BYTE( "dmast96_mvo4x", 0x200001, 0x080000, CRC(d870b6ce) SHA1(e81c24eeaa5b857910436bfb6cac2b9fa05839e8) )
	ROM_CONTINUE(0x600001, 0x080000)

	ROM_REGION( 0x400000, "gfx2", 0 ) /* BG Tiles (8x8x4, 16x16x4 and 32x32x4) */
	ROM_LOAD16_BYTE( "mf0016-3", 0x000000, 0x200000, CRC(0946bc61) SHA1(8b10c7f76daf21afb2aa6961100d83b1f6ca89bb) )
	ROM_LOAD16_BYTE( "mf0016-4", 0x000001, 0x200000, CRC(8f5b7c82) SHA1(5947c015c8a13539a3125c7ffe07cca0691b4348) )
ROM_END



ROM_START( drgnmst )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "dm1000e", 0x00000, 0x80000, CRC(29467dac) SHA1(42ca42340ffd9b04be23853ca4e936d0528a66ee) )
	ROM_LOAD16_BYTE( "dm1000o", 0x00001, 0x80000, CRC(ba48e9cf) SHA1(1107f927424107918bb10ff23f40c50579b23836) )

	ROM_REGION( 0x400, "audiocpu", ROMREGION_ERASE00 ) /* PIC16C55 Code */
//  ROM_LOAD( "pic16c55", 0x0000, 0x400, CRC(531c9f8d) SHA1(8ec180b0566f2ce1e08f0347e5ad402c73b44049) )
	/* ROM will be copied here by the init code from the USER1 region */

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pic16c55.hex", 0x000, 0x0b7b, CRC(f17011e7) SHA1(8f3bd94ffb528f661eed77d89e5b772442d2f5a6) )

	ROM_REGION( 0x140000, "oki1", 0 ) /* OKI-0 Samples */
	ROM_LOAD( "dm1001", 0x00000, 0x100000, CRC(63566f7f) SHA1(0fe6cb67a5d99cd54e46e9889ea121097756b9ef) )

	ROM_REGION( 0x200000, "oki2", 0 ) /* OKI-1 Samples */
	ROM_LOAD( "dm1002", 0x00000, 0x200000, CRC(0f1a874e) SHA1(8efc39f8ff7e6e7138b19959bd083b9df002acca) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprites (16x16x4) */
	ROM_LOAD16_BYTE( "dm1003", 0x000000, 0x080000, CRC(0ca10e81) SHA1(abebd8437764110278c8b7e583d846db27e205ec) )
	ROM_CONTINUE(0x400000, 0x080000)
	ROM_CONTINUE(0x100000, 0x080000)
	ROM_CONTINUE(0x500000, 0x080000)
	ROM_LOAD16_BYTE( "dm1005", 0x000001, 0x080000, CRC(4c2b1db5) SHA1(35d799cd13540e2aca1d1164291fe4c9938ed0ce) )
	ROM_CONTINUE(0x400001, 0x080000)
	ROM_CONTINUE(0x100001, 0x080000)
	ROM_CONTINUE(0x500001, 0x080000)
	ROM_LOAD16_BYTE( "dm1004", 0x200000, 0x040000, CRC(1a9ac249) SHA1(c15c7399dcb24dcab05887e3711e5b31bb7f31e8) )
	ROM_CONTINUE(0x600000, 0x040000)
	ROM_LOAD16_BYTE( "dm1006", 0x200001, 0x040000, CRC(c46da6fc) SHA1(f2256f02c833bc1074681729bd2b95fa6f3350cf) )
	ROM_CONTINUE(0x600001, 0x040000)

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles (8x8x4, 16x16x4 and 32x32x4) */
	ROM_LOAD16_BYTE( "dm1007", 0x000001, 0x100000, CRC(d5ad81c4) SHA1(03df467b218682a02245a6e8f500ab83de382448) )
	ROM_LOAD16_BYTE( "dm1008", 0x000000, 0x100000, CRC(b8572be3) SHA1(29aab76821e0a56033cf06b0a1890b11804da8d8) )
ROM_END

ROM_START( drgnmst2 ) // only the maincpu ROMs were provided for this set
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "even", 0x00000, 0x80000, CRC(63eae56a) SHA1(24939923be09dea523d74ecd72d7d1587469b6dc) )
	ROM_LOAD16_BYTE( "odd", 0x00001, 0x80000, CRC(35734a49) SHA1(8e9b40ca68c1dd3b2d6c262b833f71333fb43209) )

	ROM_REGION( 0x400, "audiocpu", ROMREGION_ERASE00 ) /* PIC16C55 Code */
//  ROM_LOAD( "pic16c55", 0x0000, 0x400, CRC(531c9f8d) SHA1(8ec180b0566f2ce1e08f0347e5ad402c73b44049) )
	/* ROM will be copied here by the init code from the USER1 region */

	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "pic16c55.hex", 0x000, 0x0b7b, CRC(f17011e7) SHA1(8f3bd94ffb528f661eed77d89e5b772442d2f5a6) )

	ROM_REGION( 0x140000, "oki1", 0 ) /* OKI-0 Samples */
	ROM_LOAD( "dm1001", 0x00000, 0x100000, CRC(63566f7f) SHA1(0fe6cb67a5d99cd54e46e9889ea121097756b9ef) )

	ROM_REGION( 0x200000, "oki2", 0 ) /* OKI-1 Samples */
	ROM_LOAD( "dm1002", 0x00000, 0x200000, CRC(0f1a874e) SHA1(8efc39f8ff7e6e7138b19959bd083b9df002acca) )

	ROM_REGION( 0x800000, "gfx1", 0 ) /* Sprites (16x16x4) */
	ROM_LOAD16_BYTE( "dm1003", 0x000000, 0x080000, CRC(0ca10e81) SHA1(abebd8437764110278c8b7e583d846db27e205ec) )
	ROM_CONTINUE(0x400000, 0x080000)
	ROM_CONTINUE(0x100000, 0x080000)
	ROM_CONTINUE(0x500000, 0x080000)
	ROM_LOAD16_BYTE( "dm1005", 0x000001, 0x080000, CRC(4c2b1db5) SHA1(35d799cd13540e2aca1d1164291fe4c9938ed0ce) )
	ROM_CONTINUE(0x400001, 0x080000)
	ROM_CONTINUE(0x100001, 0x080000)
	ROM_CONTINUE(0x500001, 0x080000)
	ROM_LOAD16_BYTE( "dm1004", 0x200000, 0x040000, CRC(1a9ac249) SHA1(c15c7399dcb24dcab05887e3711e5b31bb7f31e8) )
	ROM_CONTINUE(0x600000, 0x040000)
	ROM_LOAD16_BYTE( "dm1006", 0x200001, 0x040000, CRC(c46da6fc) SHA1(f2256f02c833bc1074681729bd2b95fa6f3350cf) )
	ROM_CONTINUE(0x600001, 0x040000)

	ROM_REGION( 0x200000, "gfx2", 0 ) /* BG Tiles (8x8x4, 16x16x4 and 32x32x4) */
	ROM_LOAD16_BYTE( "dm1007", 0x000001, 0x100000, CRC(d5ad81c4) SHA1(03df467b218682a02245a6e8f500ab83de382448) )
	ROM_LOAD16_BYTE( "dm1008", 0x000000, 0x100000, CRC(b8572be3) SHA1(29aab76821e0a56033cf06b0a1890b11804da8d8) )
ROM_END

uint8_t drgnmst_pic_state::drgnmst_asciitohex( uint8_t data )
{
	/* Convert ASCII data to HEX */

	if ((data >= 0x30) && (data < 0x3a)) data -= 0x30;
	data &= 0xdf;           /* remove case sensitivity */
	if ((data >= 0x41) && (data < 0x5b)) data -= 0x37;

	return data;
}


void drgnmst_pic_state::init_drgnmst()
{
	uint8_t *drgnmst_PICROM_HEX = memregion("user1")->base();
	uint16_t *drgnmst_PICROM = (uint16_t *)memregion("audiocpu")->base();
	uint16_t  src_pos = 0;
	uint16_t  dst_pos = 0;

	/* Configure the OKI-0 PCM data into a MAME friendly bank format */
	/* $00000-1ffff is the same through all banks */
	/* $20000-3ffff in each bank is actually the switched area */

	m_oki1bank->configure_entries(0, 8, memregion("oki1")->base() + 0x20000, 0x20000);
	//m_oki1bank->configure_entries(0, 8, memregion("oki1")->base(), 0x20000); // TODO : Correct?

	/**** Convert the PIC16C55 ASCII HEX dump to pure HEX ****/
	do
	{
		if ((drgnmst_PICROM_HEX[src_pos + 0] == ':') &&
			(drgnmst_PICROM_HEX[src_pos + 1] == '1') &&
			(drgnmst_PICROM_HEX[src_pos + 2] == '0'))
		{
			src_pos += 9;

			for (int32_t offs = 0; offs < 32; offs += 4)
			{
				uint8_t data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 0]));
				uint8_t data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 1]));
				if ((data_hi <= 0x0f) && (data_lo <= 0x0f))
				{
					int32_t data =  (data_hi <<  4) | (data_lo << 0);
					data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 2]));
					data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 3]));

					if ((data_hi <= 0x0f) && (data_lo <= 0x0f))
					{
						data |= (data_hi << 12) | (data_lo << 8);
						drgnmst_PICROM[dst_pos] = data;
						dst_pos += 1;
					}
				}
			}
			src_pos += 32;
		}

		/* Get the PIC16C55 Config register data */

		if ((drgnmst_PICROM_HEX[src_pos + 0] == ':') &&
			(drgnmst_PICROM_HEX[src_pos + 1] == '0') &&
			(drgnmst_PICROM_HEX[src_pos + 2] == '2') &&
			(drgnmst_PICROM_HEX[src_pos + 3] == '1'))
		{
			src_pos += 9;

			uint8_t data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 0]));
			uint8_t data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 1]));
			int32_t data =  (data_hi <<  4) | (data_lo << 0);
			data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 2]));
			data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 3]));
			data |= (data_hi << 12) | (data_lo << 8);

			m_audiocpu->set_config(data);

			src_pos = 0x7fff;       /* Force Exit */
		}
		src_pos += 1;
	} while (src_pos < 0x0b7b);     /* 0x0b7b is the size of the HEX rom loaded */
}


GAME( 1994, drgnmst,        0, drgnmst_with_pic,  drgnmst, drgnmst_pic_state, init_drgnmst, ROT0, "Unico", "Dragon Master (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, drgnmst2, drgnmst, drgnmst_with_pic,  drgnmst, drgnmst_pic_state, init_drgnmst, ROT0, "Unico", "Dragon Master (set 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, mastfury,       0, drgnmst_ym,  drgnmst, drgnmst_ym_state, empty_init, ROT0, "Unico", "Master's Fury", MACHINE_IMPERFECT_GRAPHICS )
