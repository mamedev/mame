// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Deal 'Em */
/* Deal 'Em was designed as an enhanced gamecard, to fit into various existing MPU4 cabinets
It's an unoffical addon, and does all its work through the existing 6809 CPU.
Although given unofficial status, Barcrest's patent on the MPU4 Video hardware (GB1596363) describes
the Deal 'Em board design, rather than the one they ultimately used, suggesting some sort of licensing deal. */


//     - Deal 'Em lockouts vary on certain cabinets (normally connected to AUX2, but not there?)


#include "emu.h"
#include "mpu4.h"

#include "video/resnet.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class mpu4dealem_state : public mpu4_state
{
public:
	mpu4dealem_state(const machine_config &mconfig, device_type type, const char *tag)
		: mpu4_state(mconfig, type, tag)
		, m_dealem_videoram(*this, "dealem_videoram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
	{
	}

	void dealem(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	DECLARE_MACHINE_RESET(dealem_vid);
	void dealem_palette(palette_device &palette) const;
	uint32_t screen_update_dealem(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void dealem_memmap(address_map &map) ATTR_COLD;
	TILE_GET_INFO_MEMBER(tile_info);

	optional_shared_ptr<uint8_t> m_dealem_videoram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	tilemap_t *m_tilemap = nullptr;
};


void mpu4dealem_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mpu4dealem_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 40, 32);
}


TILE_GET_INFO_MEMBER(mpu4dealem_state::tile_info)
{
	// 7654----  background color
	// ----3---  unused
	// -----210  tile bits 10-8

	uint8_t data = m_dealem_videoram[0x1000 + tile_index];
	uint8_t attr = m_dealem_videoram[0x0000 + tile_index];

	uint16_t tile = ((attr & 0x07) << 8) | data;
	uint8_t color = (attr >> 4) & 0x0f;

	tileinfo.set(0, tile, color, 0);
}


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


static GFXDECODE_START( gfx_dealem )
	GFXDECODE_ENTRY( "gfx1", 0x0000, dealemcharlayout, 0, 16 )
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


void mpu4dealem_state::dealem_palette(palette_device &palette) const
{
	static constexpr int resistances_rg[3] = { 1000, 470, 220 };
	static constexpr int resistances_b [2] = { 470, 220 };

	double weights_r[3], weights_g[3], weights_b[2];
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  1000,   0,
			3,  resistances_rg, weights_g,  1000,   0,
			2,  resistances_b,  weights_b,  1000,   0);

	uint8_t const *color_prom = memregion("proms")->base();

	for (int i = 0; i < 16; i++)
	{
		int bit0, bit1, bit2;

		// red component
		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(weights_r, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(weights_g, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(weights_b, bit0, bit1);

		palette.set_indirect_color(i, rgb_t(r, g, b));
	}

	for (int i = 0; i < 256; i++)
	{
		if ((i % 16) == 0)
			// the background color is set to each of the 16 colors
			palette.set_pen_indirect(i, i / 16);
		else
			// the rest of the palette is just a copy
			palette.set_pen_indirect(i, i % 16);
	}
}


uint32_t mpu4dealem_state::screen_update_dealem(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->mark_all_dirty();
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mpu4dealem_state::dealem_memmap(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");

	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

/*  map(0x08e0, 0x08e7).rw(FUNC(mpu4dealem_state::68681_duart_r), FUNC(mpu4dealem_state::68681_duart_w)); */ //Runs hoppers

	map(0x0900, 0x0907).rw(m_6840ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));/* PTM6840 IC2 */

	map(0x0a00, 0x0a03).rw(m_pia3, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC3 */
	map(0x0b00, 0x0b03).rw(m_pia4, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC4 */
	map(0x0c00, 0x0c03).rw(m_pia5, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC5 */
	map(0x0d00, 0x0d03).rw(m_pia6, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC6 */
	map(0x0e00, 0x0e03).rw(m_pia7, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC7 */
	map(0x0f00, 0x0f03).rw(m_pia8, FUNC(pia6821_device::read), FUNC(pia6821_device::write));        /* PIA6821 IC8 */

	map(0x1000, 0x2fff).ram().share("dealem_videoram");
	map(0x8000, 0xffff).rom().nopw();/* 64k  paged ROM (4 pages) */
}

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
}


/* machine driver for Zenitone Deal 'Em board */
void mpu4dealem_state::dealem(machine_config &config)
{
	MCFG_MACHINE_START_OVERRIDE(mpu4dealem_state,mod2)                          /* main mpu4 board initialisation */
	MCFG_MACHINE_RESET_OVERRIDE(mpu4dealem_state,dealem_vid)

	MC6809(config, m_maincpu, MPU4_MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu4dealem_state::dealem_memmap);

	mpu4_common(config);

	AY8913(config, m_ay8913, MPU4_MASTER_CLOCK/4);
	m_ay8913->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay8913->set_resistors_load(820, 0, 0);
	m_ay8913->add_route(ALL_OUTPUTS, "mono", 1.0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MPU4_MASTER_CLOCK, 440, 0, 320, 279, 0, 248); // 6.88 MHz
	screen.set_screen_update(FUNC(mpu4dealem_state::screen_update_dealem));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_dealem);

	PALETTE(config, m_palette, FUNC(mpu4dealem_state::dealem_palette), 256, 16);

	hd6845s_device &crtc(HD6845S(config, "crtc", MPU4_MASTER_CLOCK / 8)); // 0.86 MHz
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
}



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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_CUSTOM)
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
	ROM_LOAD( "zenndlem.u22",   0x000, 0x020, CRC(29988304) SHA1(42f61b8f9e1ee96b65db3b70833eb2f6e7a6ae0a) ) // data duplicated twice

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "zenndlem.u10",   0x000, 0x104, CRC(e3103c05) SHA1(91b7be75c5fb37025039ab54b484e46a033969b5) )
ROM_END

} // anonymous namespace


/*Deal 'Em was a conversion kit designed to make early MPU4 machines into video games by replacing the top glass
and reel assembly with this kit and a supplied monitor. This explains why the cabinet switch alters lamp data and buttons.
The original Deal 'Em ran on Summit Coin hardware, and was made by someone else.
Two further different releases were made, running on the Barcrest MPU4 Video, rather than this one. These are Deal 'Em Again and Deal 'Em 2000*/

GAME( 1987, v4dealem, 0, dealem, dealem, mpu4dealem_state, empty_init, ROT0, "Zenitone","Deal 'Em (MPU4 Conversion Kit, v7.0)", 0 )
