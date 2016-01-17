// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Deal 'Em */
/* Deal 'Em was designed as an enhanced gamecard, to fit into various existing MPU4 cabinets
It's an unoffical addon, and does all its work through the existing 6809 CPU.
Although given unofficial status, Barcrest's patent on the MPU4 Video hardware (GB1596363) describes
the Deal 'Em board design, rather than the one they ultimately used, suggesting some sort of licensing deal. */


//     - Deal 'Em lockouts vary on certain cabinets (normally connected to AUX2, but not there?)
//     - Deal 'Em has bad tiles (apostrophe, logo, bottom corner), black should actually be transparent
//                to give black on green. (Possibly colour 0 being used in place of colour 10?)


#include "emu.h"
#include "includes/mpu4.h"
#include "video/resnet.h"
#include "video/mc6845.h"


class mpu4dealem_state : public mpu4_state
{
public:
	mpu4dealem_state(const machine_config &mconfig, device_type type, std::string tag)
		: mpu4_state(mconfig, type, tag),
			m_dealem_videoram(*this, "dealem_videoram"),
		m_gfxdecode(*this, "gfxdecode")
	{
	}

	optional_shared_ptr<UINT8> m_dealem_videoram;
	DECLARE_MACHINE_RESET(dealem_vid);
	DECLARE_PALETTE_INIT(dealem);
	UINT32 screen_update_dealem(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(dealem_vsync_changed);
	required_device<gfxdecode_device> m_gfxdecode;
};



static const gfx_layout dealemcharlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 3*4, 2*4, 1*4, 0*4, 7*4, 6*4, 5*4, 4*4  },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


static GFXDECODE_START( dealem )
	GFXDECODE_ENTRY( "gfx1", 0x0000, dealemcharlayout, 0, 32 )
GFXDECODE_END



/***************************************************************************

  Convert the color PROMs into a more useable format.

  The palette PROM is connected to the RGB output this way:

  Red:      1K      Bit 0
            470R
            220R

  Green:    1K      Bit 3
            470R
            220R

  Blue:     470R
            220R    Bit 7

  Everything is also tied to a 1K pulldown resistor
***************************************************************************/


PALETTE_INIT_MEMBER(mpu4dealem_state,dealem)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i, len;
	static const int resistances_rg[3] = { 1000, 470, 220 };
	static const int resistances_b [2] = { 470, 220 };
	double weights_r[3], weights_g[3], weights_b[2];

	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  1000,   0,
			3,  resistances_rg, weights_g,  1000,   0,
			2,  resistances_b,  weights_b,  1000,   0);

	len = memregion("proms")->bytes();
	for (i = 0; i < len; i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = BIT(*color_prom,0);
		bit1 = BIT(*color_prom,1);
		bit2 = BIT(*color_prom,2);
		r = combine_3_weights(weights_r, bit0, bit1, bit2);
		/* green component */
		bit0 = BIT(*color_prom,3);
		bit1 = BIT(*color_prom,4);
		bit2 = BIT(*color_prom,5);
		g = combine_3_weights(weights_g, bit0, bit1, bit2);
		/* blue component */
		bit0 = BIT(*color_prom,6);
		bit1 = BIT(*color_prom,7);
		b = combine_2_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i,rgb_t(r,g,b));
		color_prom++;
	}
}


UINT32 mpu4dealem_state::screen_update_dealem(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y;
	int count = 0;

	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 40; x++)
		{
			int tile = m_dealem_videoram[count + 0x1000] | (m_dealem_videoram[count] << 8);
			count++;
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect,tile,0,0,0,x * 8,y * 8);
		}
	}

	return 0;
}


WRITE_LINE_MEMBER(mpu4dealem_state::dealem_vsync_changed)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static ADDRESS_MAP_START( dealem_memmap, AS_PROGRAM, 8, mpu4dealem_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)

/*  AM_RANGE(0x08e0, 0x08e7) AM_READWRITE(68681_duart_r,68681_duart_w) */ //Runs hoppers

	AM_RANGE(0x0900, 0x0907) AM_DEVREADWRITE("ptm_ic2", ptm6840_device, read, write)/* PTM6840 IC2 */

	AM_RANGE(0x0a00, 0x0a03) AM_DEVREADWRITE("pia_ic3", pia6821_device, read, write)        /* PIA6821 IC3 */
	AM_RANGE(0x0b00, 0x0b03) AM_DEVREADWRITE("pia_ic4", pia6821_device, read, write)        /* PIA6821 IC4 */
	AM_RANGE(0x0c00, 0x0c03) AM_DEVREADWRITE("pia_ic5", pia6821_device, read, write)        /* PIA6821 IC5 */
	AM_RANGE(0x0d00, 0x0d03) AM_DEVREADWRITE("pia_ic6", pia6821_device, read, write)        /* PIA6821 IC6 */
	AM_RANGE(0x0e00, 0x0e03) AM_DEVREADWRITE("pia_ic7", pia6821_device, read, write)        /* PIA6821 IC7 */
	AM_RANGE(0x0f00, 0x0f03) AM_DEVREADWRITE("pia_ic8", pia6821_device, read, write)        /* PIA6821 IC8 */

	AM_RANGE(0x1000, 0x2fff) AM_RAM AM_SHARE("dealem_videoram")
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_WRITENOP/* 64k  paged ROM (4 pages) */
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(mpu4dealem_state,dealem_vid)
{
	m_vfd->reset(); //for debug ports only

	m_lamp_strobe    = 0;
	m_lamp_strobe2   = 0;
	m_led_strobe     = 0;

	m_IC23GC    = 0;
	m_IC23GB    = 0;
	m_IC23GA    = 0;
	m_IC23G1    = 1;
	m_IC23G2A   = 0;
	m_IC23G2B   = 0;

	m_prot_col  = 0;
	m_chr_counter    = 0;
	m_chr_value     = 0;
}


/* machine driver for Zenitone Deal 'Em board */
static MACHINE_CONFIG_START( dealem, mpu4dealem_state )
	MCFG_MACHINE_START_OVERRIDE(mpu4dealem_state,mod2)                          /* main mpu4 board initialisation */
	MCFG_MACHINE_RESET_OVERRIDE(mpu4dealem_state,dealem_vid)

	MCFG_CPU_ADD("maincpu", M6809, MPU4_MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(dealem_memmap)

	MCFG_FRAGMENT_ADD(mpu4_common)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8913",AY8913, MPU4_MASTER_CLOCK/4)
	MCFG_AY8910_OUTPUT_TYPE(AY8910_SINGLE_OUTPUT)
	MCFG_AY8910_RES_LOADS(820, 0, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE((54+1)*8, (32+1)*8)                    /* Taken from 6845 init, registers 00 & 04. Normally programmed with (value-1) */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 31*8-1)      /* Taken from 6845 init, registers 01 & 06 */
	MCFG_SCREEN_REFRESH_RATE(56)                            /* Measured accurately from the flip-flop, but 6845 handles this */
	MCFG_SCREEN_UPDATE_DRIVER(mpu4dealem_state, screen_update_dealem)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dealem)

	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(mpu4dealem_state,dealem)

	MCFG_MC6845_ADD("crtc", HD6845, "screen", MPU4_MASTER_CLOCK / 4 / 8) /* HD68B45 */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(mpu4dealem_state, dealem_vsync_changed))
MACHINE_CONFIG_END



static INPUT_PORTS_START( dealem )
	PORT_START("ORANGE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ORANGE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN5) PORT_NAME("20p Token")PORT_IMPULSE(5)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p") PORT_CONDITION("DIL1",0x0f,EQUALS,0x04)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p") PORT_CONDITION("DIL1",0x0f,EQUALS,0x05)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p") PORT_CONDITION("DIL1",0x0f,EQUALS,0x06)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p") PORT_CONDITION("DIL1",0x0f,EQUALS,0x07)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p") PORT_CONDITION("DIL1",0x0f,EQUALS,0x08)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p") PORT_CONDITION("DIL1",0x0f,EQUALS,0x09)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BLACK1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Gamble")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2) PORT_NAME("Pontoon")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x01)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x09)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x03)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x04)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x05)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x06)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x07)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x08)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Test Button") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Rear Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("BLACK2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x00)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1) PORT_NAME("Hi-Lo") PORT_CONDITION("DIL1",0x0f,EQUALS,0x02)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Twist")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Lo")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Hi")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Stick")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Collect")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Deal")

	PORT_START("DIL1")
	PORT_DIPNAME( 0x0f, 0x00, "Cabinet Set Up Mode" ) PORT_DIPLOCATION("DIL1:01,02,03,04")
	PORT_DIPSETTING(    0x00, "Stop The Clock" )
	PORT_DIPSETTING(    0x01, "Hit the Top" )
	PORT_DIPSETTING(    0x02, "Way In" )
	PORT_DIPSETTING(    0x03, "Smash and Grab" )
	PORT_DIPSETTING(    0x04, "Ready Steady Go-1" )
	PORT_DIPSETTING(    0x05, "Ready Steady Go-2" )
	PORT_DIPSETTING(    0x06, "Top Gears-1" )
	PORT_DIPSETTING(    0x07, "Top Gears-2" )
	PORT_DIPSETTING(    0x08, "Nifty Fifty" )
	PORT_DIPSETTING(    0x09, "Super Tubes" )
	PORT_DIPNAME( 0x70, 0x00, "Target Payout Percentage" ) PORT_DIPLOCATION("DIL1:05,06,07")
	PORT_DIPSETTING(    0x00, "72%" )
	PORT_DIPSETTING(    0x10, "74%" )
	PORT_DIPSETTING(    0x20, "76%" )
	PORT_DIPSETTING(    0x30, "78%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x50, "82%" )
	PORT_DIPSETTING(    0x60, "84%" )
	PORT_DIPSETTING(    0x70, "86%" )
	PORT_DIPNAME( 0x80, 0x00, "Display Switch Settings on Monitor" ) PORT_DIPLOCATION("DIL1:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("DIL2")
	PORT_DIPNAME( 0x01, 0x00, "Payout Limit" ) PORT_DIPLOCATION("DIL2:01")
	PORT_DIPSETTING(    0x00, "200p (All Cash)")
	PORT_DIPSETTING(    0x01, "200p (Cash)+400p (Token)")
	PORT_DIPNAME( 0x02, 0x00, "10p Payout Priority" ) PORT_DIPLOCATION("DIL2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "Clear Credits and bank at power on?" ) PORT_DIPLOCATION("DIL2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x08, 0x00, "50p Payout Solenoid fitted?" ) PORT_DIPLOCATION("DIL2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x10, 0x00, "100p Payout Solenoid fitted?" ) PORT_DIPLOCATION("DIL2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin alarms active?" ) PORT_DIPLOCATION("DIL2:06")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes  ) )
	PORT_DIPNAME( 0x40, 0x00, "Price of Play" ) PORT_DIPLOCATION("DIL2:07")
	PORT_DIPSETTING(    0x00, "10p 1 Game" )
	PORT_DIPSETTING(    0x40, "10p 2 Games" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Entry" ) PORT_DIPLOCATION("DIL2:08")
	PORT_DIPSETTING(    0x00, "Multi" )
	PORT_DIPSETTING(    0x80, DEF_STR(Single))

	PORT_START("AUX1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("AUX2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5) PORT_CONDITION("DIL1",0x0f,EQUALS,0x00)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5) PORT_CONDITION("DIL1",0x0f,EQUALS,0x01)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5) PORT_CONDITION("DIL1",0x0f,EQUALS,0x02)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("10p")PORT_IMPULSE(5) PORT_CONDITION("DIL1",0x0f,EQUALS,0x03)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("20p")PORT_IMPULSE(5)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("50p")PORT_IMPULSE(5)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("100p")PORT_IMPULSE(5)
INPUT_PORTS_END


ROM_START( v4dealem )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "zenndlem.u6",    0x8000, 0x8000,  CRC(571e5c05) SHA1(89b4c331407a04eae34bb187b036791e0a671533) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "zenndlem.u24",   0x0000, 0x10000, CRC(3a1950c4) SHA1(7138346d4e8b3cffbd9751b4d7ebd367b9ad8da9) )    /* text layer */

	ROM_REGION( 0x020, "proms", 0 )
	ROM_LOAD( "zenndlem.u22",   0x000, 0x020, CRC(29988304) SHA1(42f61b8f9e1ee96b65db3b70833eb2f6e7a6ae0a) )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "zenndlem.u10",   0x000, 0x104, CRC(e3103c05) SHA1(91b7be75c5fb37025039ab54b484e46a033969b5) )
ROM_END

/*Deal 'Em was a conversion kit designed to make early MPU4 machines into video games by replacing the top glass
and reel assembly with this kit and a supplied monitor. This explains why the cabinet switch alters lamp data and buttons.
The original Deal 'Em ran on Summit Coin hardware, and was made by someone else.
Two further different releases were made, running on the Barcrest MPU4 Video, rather than this one. These are Deal 'Em Again and Deal 'Em 2000*/

GAME(  1987,v4dealem,   0,          dealem,     dealem, driver_device,      0,          ROT0, "Zenitone","Deal 'Em (MPU4 Conversion Kit, v7.0)",MACHINE_IMPERFECT_GRAPHICS )
