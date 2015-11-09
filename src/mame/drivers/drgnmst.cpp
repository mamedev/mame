// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Dragon Master (c)1994 Unico

the hardware seems to fall somewhere between the
hardware playmark commonly used and the hardware
unico used for zero point etc.

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
DSW2    68000      TPC1020           62256
                   (PLCC84)          62256
12MHz 32MHz


Notes:
          *: Unknown PLCC84 chip (surface scratched)
      VSync: 60Hz
      HSync: 15.625kHz
  68K clock: 16MHz

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "includes/drgnmst.h"


WRITE16_MEMBER(drgnmst_state::drgnmst_coin_w)
{
	coin_counter_w(machine(), 0, data & 0x100);
	coin_lockout_w(machine(), 0, ~data & 0x400);
	coin_lockout_w(machine(), 1, ~data & 0x800);
}

WRITE16_MEMBER(drgnmst_state::drgnmst_snd_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_snd_command = (data & 0xff);
		space.device().execute().yield();
	}
}

WRITE16_MEMBER(drgnmst_state::drgnmst_snd_flag_w)
{
	/* Enables the following 68K write operation to latch through to the PIC */
	if (ACCESSING_BITS_0_7)
		m_snd_flag = 1;
}


READ8_MEMBER(drgnmst_state::pic16c5x_port0_r)
{
	return m_pic16c5x_port0;
}

READ8_MEMBER(drgnmst_state::drgnmst_snd_command_r)
{
	int data = 0;

	switch (m_oki_control & 0x1f)
	{
		case 0x12:  data = (m_oki_2->read(space, 0) & 0x0f); break;
		case 0x16:  data = (m_oki_1->read(space, 0) & 0x0f); break;
		case 0x0b:
		case 0x0f:  data = m_snd_command; break;
		default:    break;
	}

	return data;
}

READ8_MEMBER(drgnmst_state::drgnmst_snd_flag_r)
{
	if (m_snd_flag)
	{
		m_snd_flag = 0;
		return 0x40;
	}

	return 0x00;
}

WRITE8_MEMBER(drgnmst_state::drgnmst_pcm_banksel_w)
{
	/*  This is a 4 bit port.
	    Each pair of bits is used in part of the OKI PCM ROM bank selectors.
	    See the Port 2 write handler below (drgnmst_snd_control_w) for details.
	*/

	m_pic16c5x_port0 = data;
}

WRITE8_MEMBER(drgnmst_state::drgnmst_oki_w)
{
	m_oki_command = data;
}

WRITE8_MEMBER(drgnmst_state::drgnmst_snd_control_w)
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
	if (oki_new_bank != m_oki0_bank)
	{
		m_oki0_bank = oki_new_bank;
		if (m_oki0_bank)
			oki_new_bank--;
		m_oki_1->set_bank_base(oki_new_bank * 0x40000);
	}

	oki_new_bank = ((m_pic16c5x_port0 & 0x3) >> 0) | ((m_oki_control & 0x20) >> 3);
	if (oki_new_bank != m_oki1_bank)
	{
		m_oki1_bank = oki_new_bank;
		m_oki_2->set_bank_base(oki_new_bank * 0x40000);
	}

	switch (m_oki_control & 0x1f)
	{
		case 0x11:
//                  logerror("Writing %02x to OKI1", m_oki_command);
//                  logerror(", PortC=%02x, Code=%02x, Bank0=%01x, Bank1=%01x\n", m_oki_control, m_snd_command, m_oki0_bank, m_oki1_bank);
					m_oki_2->write(space, 0, m_oki_command);
					break;
		case 0x15:
//                  logerror("Writing %02x to OKI0", m_oki_command);
//                  logerror(", PortC=%02x, Code=%02x, Bank0=%01x, Bank1=%01x\n", m_oki_control, m_snd_command, m_oki0_bank, m_oki1_bank);
					m_oki_1->write(space, 0, m_oki_command);
					break;
		default:    break;
	}
}


READ_LINE_MEMBER(drgnmst_state::PIC16C5X_T0_clk_r)
{
	return 0;
}


/***************************** 68000 Memory Map *****************************/

static ADDRESS_MAP_START( drgnmst_main_map, AS_PROGRAM, 16, drgnmst_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x800018, 0x800019) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x80001a, 0x80001b) AM_READ_PORT("DSW1")
	AM_RANGE(0x80001c, 0x80001d) AM_READ_PORT("DSW2")
	AM_RANGE(0x800030, 0x800031) AM_WRITE(drgnmst_coin_w)
	AM_RANGE(0x800100, 0x80011f) AM_WRITEONLY AM_SHARE("vidregs")
	AM_RANGE(0x800120, 0x800121) AM_WRITENOP
	AM_RANGE(0x80014a, 0x80014b) AM_WRITENOP
	AM_RANGE(0x800154, 0x800155) AM_WRITEONLY AM_SHARE("vidregs2") // seems to be priority control
	AM_RANGE(0x800176, 0x800177) AM_READ_PORT("EXTRA")
	AM_RANGE(0x800180, 0x800181) AM_WRITE(drgnmst_snd_command_w)
	AM_RANGE(0x800188, 0x800189) AM_WRITE(drgnmst_snd_flag_w)
	AM_RANGE(0x8001e0, 0x8001e1) AM_WRITENOP
	AM_RANGE(0x900000, 0x903fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x904000, 0x907fff) AM_RAM_WRITE(drgnmst_md_videoram_w) AM_SHARE("md_videoram")
	AM_RANGE(0x908000, 0x90bfff) AM_RAM_WRITE(drgnmst_bg_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0x90c000, 0x90ffff) AM_RAM_WRITE(drgnmst_fg_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0x920000, 0x923fff) AM_RAM AM_SHARE("rowscrollram") // rowscroll ram
	AM_RANGE(0x930000, 0x9307ff) AM_RAM AM_SHARE("spriteram")   // Sprites
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


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


static const gfx_layout drgnmst_char8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 24,8,16, 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*32
};


static const gfx_layout drgnmst_char16x16_layout =
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



static const gfx_layout drgnmst_char32x32_layout =
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


static GFXDECODE_START( drgnmst )
	GFXDECODE_ENTRY( "gfx1", 0, drgnmst_char16x16_layout,   0,      0x200  ) /* sprite tiles */
	GFXDECODE_ENTRY( "gfx2", 0, drgnmst_char8x8_layout,     0x200,  0x200  ) /* fg tiles */
	GFXDECODE_ENTRY( "gfx2", 0, drgnmst_char16x16_layout,   0x0400, 0x200  ) /* md tiles */
	GFXDECODE_ENTRY( "gfx2", 0, drgnmst_char32x32_layout,   0x0600, 0x200  ) /* bg tiles */
GFXDECODE_END


void drgnmst_state::machine_start()
{
	save_item(NAME(m_snd_flag));
	save_item(NAME(m_snd_command));
	save_item(NAME(m_oki_control));
	save_item(NAME(m_oki_command));
	save_item(NAME(m_pic16c5x_port0));
	save_item(NAME(m_oki1_bank));
	save_item(NAME(m_oki0_bank));
}

void drgnmst_state::machine_reset()
{
	m_snd_flag = 0;
	m_snd_command = 0;
	m_oki_control = 0;
	m_oki_command = 0;
	m_pic16c5x_port0 = 0;
	m_oki1_bank = 0;
	m_oki0_bank = 0;
}

static MACHINE_CONFIG_START( drgnmst, drgnmst_state )

	MCFG_CPU_ADD("maincpu", M68000, 12000000) /* Confirmed */
	MCFG_CPU_PROGRAM_MAP(drgnmst_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", drgnmst_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C55, 32000000/8)  /* Confirmed */
	MCFG_PIC16C5x_READ_A_CB(READ8(drgnmst_state, pic16c5x_port0_r))
	MCFG_PIC16C5x_WRITE_A_CB(WRITE8(drgnmst_state, drgnmst_pcm_banksel_w))
	MCFG_PIC16C5x_READ_B_CB(READ8(drgnmst_state, drgnmst_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(drgnmst_state, drgnmst_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(drgnmst_state, drgnmst_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(drgnmst_state, drgnmst_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(drgnmst_state, PIC16C5X_T0_clk_r))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", drgnmst)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 56*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(drgnmst_state, screen_update_drgnmst)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x2000)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", 32000000/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)

	MCFG_OKIM6295_ADD("oki2", 32000000/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END


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


UINT8 drgnmst_state::drgnmst_asciitohex( UINT8 data )
{
	/* Convert ASCII data to HEX */

	if ((data >= 0x30) && (data < 0x3a)) data -= 0x30;
	data &= 0xdf;           /* remove case sensitivity */
	if ((data >= 0x41) && (data < 0x5b)) data -= 0x37;

	return data;
}


DRIVER_INIT_MEMBER(drgnmst_state,drgnmst)
{
	UINT8 *drgnmst_PICROM_HEX = memregion("user1")->base();
	UINT16 *drgnmst_PICROM = (UINT16 *)memregion("audiocpu")->base();
	UINT8 *drgnmst_PCM = memregion("oki1")->base();
	INT32   offs, data;
	UINT16  src_pos = 0;
	UINT16  dst_pos = 0;
	UINT8   data_hi, data_lo;

	/* Configure the OKI-0 PCM data into a MAME friendly bank format */
	/* $00000-1ffff is the same through all banks */
	/* $20000-3ffff in each bank is actually the switched area */

	for (offs = 0x1ffff; offs >= 0; offs--)
	{
		drgnmst_PCM[0x120000 + offs] = drgnmst_PCM[0xa0000 + offs];
		drgnmst_PCM[0x100000 + offs] = drgnmst_PCM[0x00000 + offs];
		drgnmst_PCM[0x0e0000 + offs] = drgnmst_PCM[0x80000 + offs];
		drgnmst_PCM[0x0c0000 + offs] = drgnmst_PCM[0x00000 + offs];
		drgnmst_PCM[0x0a0000 + offs] = drgnmst_PCM[0x60000 + offs];
		drgnmst_PCM[0x080000 + offs] = drgnmst_PCM[0x00000 + offs];
		drgnmst_PCM[0x060000 + offs] = drgnmst_PCM[0x40000 + offs];
		drgnmst_PCM[0x040000 + offs] = drgnmst_PCM[0x00000 + offs];
	}

	/**** Convert the PIC16C55 ASCII HEX dump to pure HEX ****/
	do
	{
		if ((drgnmst_PICROM_HEX[src_pos + 0] == ':') &&
			(drgnmst_PICROM_HEX[src_pos + 1] == '1') &&
			(drgnmst_PICROM_HEX[src_pos + 2] == '0'))
		{
			src_pos += 9;

			for (offs = 0; offs < 32; offs += 4)
			{
				data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 0]));
				data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 1]));
				if ((data_hi <= 0x0f) && (data_lo <= 0x0f)) {
					data =  (data_hi <<  4) | (data_lo << 0);
					data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 2]));
					data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + offs + 3]));

					if ((data_hi <= 0x0f) && (data_lo <= 0x0f)) {
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

			data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 0]));
			data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 1]));
			data =  (data_hi <<  4) | (data_lo << 0);
			data_hi = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 2]));
			data_lo = drgnmst_asciitohex((drgnmst_PICROM_HEX[src_pos + 3]));
			data |= (data_hi << 12) | (data_lo << 8);

			m_audiocpu->pic16c5x_set_config(data);

			src_pos = 0x7fff;       /* Force Exit */
		}
		src_pos += 1;
	} while (src_pos < 0x0b7b);     /* 0x0b7b is the size of the HEX rom loaded */
}


GAME( 1994, drgnmst, 0, drgnmst,  drgnmst, drgnmst_state, drgnmst, ROT0, "Unico", "Dragon Master", MACHINE_SUPPORTS_SAVE )
