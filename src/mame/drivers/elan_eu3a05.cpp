// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
    Radica Games 6502 based 'TV Game' hardware

    These use a 6502 derived CPU under a glob
    The CPU die is marked 'ELAN EU3A05'

    There is a second glob surrounded by TSOP48 pads
    this contains the ROM

    Space Invaders uses a 3rd glob marked
    AMIC (C) (M) 1998-1 AM3122A
    this is presumably for the bitmap layer on Qix

    --
    Known games on this hardware

    Tetris
    Space Invaders
	ABL Air-Blaster Joystick

    ---
    XaviX plug and play units almost always have a XaviX logo on the external packaging
	while the ones for this driver (and SunPlus etc.) don't seem to have any specific
	markings.

    Notes:
    
	Tetris - RAM 0xa0 and 0xa1 contain the ACD0 and AD1 values and player 2 controls if
	between certain values? probably read via serial (or ADC abuse?)

	Internal Test Menus:
	
	Tetris - hold P1 Down + P1 Anticlockwise (Button 2) on boot
    Space Invaders - hold P1 Down + P1 Button 1 on boot
	ABL Air-Blaster - none?

	----

    Flaws (NOT emulation bugs, happen on hardware):
    --

    In QIX the sprites lag behind the line drawing, so you see the line infront of your player until you stop moving

    In Space Invaders the UFO can sometimes glitch for a frame when appearing, and wraps around at the edges
      (even if the hardware supports having higher priority tiles to prevent this, as used by Lunar Rescue, it isn't
       used here)

    Colony 7 has a typo in the instructions

    The fake 'colour band' effect does not apply to the thruster (and several other elements) in Lunar Rescue

    Enemies in Phoenix are rendered above the score panel

    The 200pt right facing bird on the Phoenix score table is corrupt

    Uncertain (to check)

    Space Invaders seems to be using a darker than expected palette, there are lighter colours in the palette but
    they don't seem to be used.  It's difficult to judge from hardware videos, although it definitely isn't as
    white as the menu, so this might also be a non-bug.


*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
//#include "cpu/m6502/m65c02.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"
#include "audio/elan_eu3a05.h"
#include "machine/elan_eu3a05gpio.h"
#include "machine/elan_eu3a05sys.h"
#include "video/elan_eu3a05vid.h"

class elan_eu3a05_state : public driver_device
{
public:
	elan_eu3a05_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ram(*this, "ram"),
		m_gpio(*this, "gpio"),
		m_sys(*this, "sys"),
		m_sound(*this, "eu3a05sound"),
		m_vid(*this, "vid"),
		m_pixram(*this, "pixram"),
		m_bank(*this, "bank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void elan_eu3a05(machine_config &config);
	void airblsjs(machine_config& config);

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// system
	DECLARE_READ8_MEMBER(elan_eu3a05_5003_r);
	DECLARE_READ8_MEMBER(elan_eu3a05_pal_ntsc_r);
	DECLARE_WRITE8_MEMBER(elan_eu3a05_500b_unk_w);

	INTERRUPT_GEN_MEMBER(interrupt);

	// for callback
	DECLARE_READ8_MEMBER(read_full_space);

	void elan_eu3a05_bank_map(address_map &map);
	void elan_eu3a05_map(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint8_t> m_ram;
	required_device<elan_eu3a05gpio_device> m_gpio;
	required_device<elan_eu3a05sys_device> m_sys;
	required_device<elan_eu3a05_sound_device> m_sound;
	required_device<elan_eu3a05vid_device> m_vid;
	required_shared_ptr<uint8_t> m_pixram;
	required_device<address_map_bank_device> m_bank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE_LINE_MEMBER(sound_end0) { m_sys->generate_custom_interrupt(2); }
	DECLARE_WRITE_LINE_MEMBER(sound_end1) { m_sys->generate_custom_interrupt(3); }
	DECLARE_WRITE_LINE_MEMBER(sound_end2) { m_sys->generate_custom_interrupt(4); }
	DECLARE_WRITE_LINE_MEMBER(sound_end3) { m_sys->generate_custom_interrupt(5); }
	DECLARE_WRITE_LINE_MEMBER(sound_end4) { m_sys->generate_custom_interrupt(6); }
	DECLARE_WRITE_LINE_MEMBER(sound_end5) { m_sys->generate_custom_interrupt(7); }
};

void elan_eu3a05_state::video_start()
{
}


uint32_t elan_eu3a05_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	return m_vid->screen_update(screen, bitmap, cliprect);
}

// System control

READ8_MEMBER(elan_eu3a05_state::elan_eu3a05_pal_ntsc_r)
{
	// how best to handle this, we probably need to run the PAL machine at 50hz
	// the text under the radica logo differs between regions
	logerror("%s: elan_eu3a05_pal_ntsc_r (region + more?)\n", machine().describe_context());
	return 0xff; // NTSC
	//return 0x00; // PAL
}

READ8_MEMBER(elan_eu3a05_state::elan_eu3a05_5003_r)
{
	/* masked with 0x0f, 0x01 or 0x03 depending on situation..

	  I think it might just be an RNG because if you return 0x00
	  your shots can never hit the stage 3 enemies in Phoenix and
	  if you return 0xff they always hit.  On the real hardware it
	  seems to be random.  Could also just be a crude frame counter
	  used for the same purpose.

	*/

	logerror("%s: elan_eu3a05_5003_r (RNG?)\n", machine().describe_context());

	return machine().rand();
}



WRITE8_MEMBER(elan_eu3a05_state::elan_eu3a05_500b_unk_w)
{
	// this is the PAL / NTSC flag when read, so what are writes?
	logerror("%s: elan_eu3a05_500b_unk_w %02x\n", machine().describe_context(), data);
}

// sound callback
READ8_MEMBER(elan_eu3a05_state::read_full_space)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	return fullbankspace.read_byte(offset);
}

// code at 8bc6 in Air Blaster makes unwanted reads, why, bug in code, flow issue?
//[:maincpu] ':maincpu' (8BC6): unmapped program memory read from 5972 & FF


void elan_eu3a05_state::elan_eu3a05_map(address_map &map)
{
	// can the addresses move around?
	map(0x0000, 0x3fff).ram().share("ram");
	map(0x4800, 0x49ff).rw(m_vid, FUNC(elan_eu3a05commonvid_device::palette_r), FUNC(elan_eu3a05commonvid_device::palette_w));

	// 500x system regs?
	map(0x5000, 0x5000).ram();
	map(0x5001, 0x5001).ram();
	// 5002
	map(0x5003, 0x5003).r(FUNC(elan_eu3a05_state::elan_eu3a05_5003_r));
	map(0x5004, 0x5004).ram();
	// 5005
	map(0x5006, 0x5006).ram();
	map(0x5007, 0x5008).rw(m_sys, FUNC(elan_eu3a05commonsys_device::intmask_r), FUNC(elan_eu3a05commonsys_device::intmask_w));
	map(0x5009, 0x5009).ram();
	map(0x500a, 0x500a).ram();
	map(0x500b, 0x500b).rw(FUNC(elan_eu3a05_state::elan_eu3a05_pal_ntsc_r), FUNC(elan_eu3a05_state::elan_eu3a05_500b_unk_w)); // PAL / NTSC flag at least
	map(0x500c, 0x500d).rw(m_sys, FUNC(elan_eu3a05commonsys_device::elan_eu3a05_rombank_r), FUNC(elan_eu3a05commonsys_device::elan_eu3a05_rombank_w));

	// 501x DMA controller
	map(0x500f, 0x5015).rw(m_sys, FUNC(elan_eu3a05sys_device::dma_param_r), FUNC(elan_eu3a05sys_device::dma_param_w));
	map(0x5016, 0x5016).rw(m_sys, FUNC(elan_eu3a05sys_device::elan_eu3a05_dmatrg_r), FUNC(elan_eu3a05sys_device::elan_eu3a05_dmatrg_w));

	// 502x - 503x video regs area?
	map(0x5020, 0x5026).ram(); // unknown, space invaders sets these to fixed values, tetris has them as 00
	map(0x5027, 0x5027).rw(m_vid, FUNC(elan_eu3a05vid_device::elan_eu3a05_vidctrl_r), FUNC(elan_eu3a05vid_device::elan_eu3a05_vidctrl_w));
	map(0x5028, 0x5028).ram();
	map(0x5029, 0x5029).rw(m_vid, FUNC(elan_eu3a05vid_device::tile_gfxbase_lo_r), FUNC(elan_eu3a05vid_device::tile_gfxbase_lo_w)); // tilebase
	map(0x502a, 0x502a).rw(m_vid, FUNC(elan_eu3a05vid_device::tile_gfxbase_hi_r), FUNC(elan_eu3a05vid_device::tile_gfxbase_hi_w)); // tilebase
	map(0x502b, 0x502b).rw(m_vid, FUNC(elan_eu3a05vid_device::sprite_gfxbase_lo_r), FUNC(elan_eu3a05vid_device::sprite_gfxbase_lo_w)); // tilebase (spr?)
	map(0x502c, 0x502c).rw(m_vid, FUNC(elan_eu3a05vid_device::sprite_gfxbase_hi_r), FUNC(elan_eu3a05vid_device::sprite_gfxbase_hi_w)); // tilebase (spr?)
	map(0x502d, 0x502e).rw(m_vid, FUNC(elan_eu3a05vid_device::splitpos_r), FUNC(elan_eu3a05vid_device::splitpos_w)); // split position
	map(0x502f, 0x5036).rw(m_vid, FUNC(elan_eu3a05vid_device::tile_scroll_r), FUNC(elan_eu3a05vid_device::tile_scroll_w)); // there are 4 scroll values in here, x scroll, y scroll, xscroll1 for split, xscroll2 for split (eu3a14 can do split too)
	// 5037
	map(0x5038, 0x5038).ram();

	// 504x GPIO area?
	map(0x5040, 0x5046).rw(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_r), FUNC(elan_eu3a05gpio_device::gpio_w));
	// 5047
	map(0x5048, 0x504a).w(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_unk_w));

	// 506x unknown
	map(0x5060, 0x506d).ram(); // read/written by tetris (ADC?)

	// 508x sound
	map(0x5080, 0x50a9).rw(m_sound, FUNC(elan_eu3a05_sound_device::read), FUNC(elan_eu3a05_sound_device::write));

	//map(0x5000, 0x50ff).ram();
	map(0x6000, 0xdfff).m(m_bank, FUNC(address_map_bank_device::amap8));

	map(0xe000, 0xffff).rom().region("maincpu", 0x3f8000);
	// not sure how these work, might be a modified 6502 core instead.
	map(0xfffa, 0xfffb).r(m_sys, FUNC(elan_eu3a05commonsys_device::nmi_vector_r)); // custom vectors handled with NMI for now
	//map(0xfffe, 0xffff).r(m_sys, FUNC(elan_eu3a05commonsys_device::irq_vector_r));  // allow normal IRQ for brk
}


void elan_eu3a05_state::elan_eu3a05_bank_map(address_map &map)
{
	map(0x000000, 0xffffff).noprw(); // shut up any logging when video params are invalid
	map(0x000000, 0x3fffff).rom().region("maincpu", 0);
	map(0x400000, 0x40ffff).ram(); // ?? only ever cleared maybe a mirror of below?
	map(0x800000, 0x80ffff).ram().share("pixram"); // Qix writes here and sets the tile base here instead of ROM so it can have a pixel layer
}

static INPUT_PORTS_START( rad_sinv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // MENU
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( rad_tetr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Anticlockwise
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Clockwise
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // and Select
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 2 inputs must be read via serial or similar
	   the game doesn't read them directly, or even let
	   you select player 2 mode by default
	*/

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END


static INPUT_PORTS_START( airblsjs )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pause")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )	PORT_NAME("Start")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Trigger")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Missile")
INPUT_PORTS_END



void elan_eu3a05_state::machine_start()
{
}

void elan_eu3a05_state::machine_reset()
{
	/* the 6502 core sets the default stack value to 0x01bd
	   and Tetris does not initialize it to anything else

	   Tetris stores the playfield data at 0x100 - 0x1c7 and
	   has a clear routine that will erase that range and
	   trash the stack

	   It seems likely this 6502 sets it to 0x1ff by default
	   at least.

	   According to
	   http://mametesters.org/view.php?id=6486
	   this isn't right for known 6502 types either
	*/
	m_maincpu->set_state_int(M6502_S, 0x1ff);
}

static const gfx_layout helper_4bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ STEP8(0,4) },
	{ 0 },
	8 * 4
};

static const gfx_layout helper_8bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,8) },
	{ 0 },
	8 * 8
};


// these are fake just to make looking at the texture pages easier
static const uint32_t texlayout_xoffset_8bpp[256] = { STEP256(0,8) };
static const uint32_t texlayout_yoffset_8bpp[256] = { STEP256(0,256*8) };
static const gfx_layout texture_helper_8bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*8,
	texlayout_xoffset_8bpp,
	texlayout_yoffset_8bpp
};

static const uint32_t texlayout_xoffset_4bpp[256] = { STEP256(0,4) };
static const uint32_t texlayout_yoffset_4bpp[256] = { STEP256(0,256*4) };
static const gfx_layout texture_helper_4bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*4,
	texlayout_xoffset_4bpp,
	texlayout_yoffset_4bpp
};

static GFXDECODE_START( gfx_elan_eu3a05_fake )
	GFXDECODE_ENTRY( "maincpu", 0, helper_4bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_4bpp_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, helper_8bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_8bpp_layout,  0x0, 1  )
GFXDECODE_END



INTERRUPT_GEN_MEMBER(elan_eu3a05_state::interrupt)
{
	m_sys->generate_custom_interrupt(9);
}


/* Tetris (PAL version)      has XTAL of 21.281370
   Air Blaster (PAL version) has XTAL of 21.2813
   what are the NTSC clocks?

   not confirmed on Space Invaders, actual CPU clock unknown.
   21281370 is the same value as a PAL SNES

   game logic speed (but not level scroll speed) in Air Blaster appears to be limited entirely by CPU speed (and therefore needs to be around 2-3mhz
   at most to match hardware) - a divider of 8 gives something close to original hardware
   it is unclear exactly what limits the clock speed (maybe video / sound causes waitstates? - dma in progress could also slow / stop the CPU
   and is not going to be 'instant' on hardware)
  
   using a low clock speed also helps with the badly programmed controls in Tetris as that likewise seems to run the game logic 'as fast as possible'
   there don't appear to be any kind of blanking bits being checked.
*/

void elan_eu3a05_state::elan_eu3a05(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(21'281'370)/8); // wrong, this is the PAL clock
	m_maincpu->set_addrmap(AS_PROGRAM, &elan_eu3a05_state::elan_eu3a05_map);
	m_maincpu->set_vblank_int("screen", FUNC(elan_eu3a05_state::interrupt));

	ADDRESS_MAP_BANK(config, "bank").set_map(&elan_eu3a05_state::elan_eu3a05_bank_map).set_options(ENDIANNESS_LITTLE, 8, 24, 0x8000);

	PALETTE(config, m_palette).set_entries(256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(elan_eu3a05_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_elan_eu3a05_fake);

	ELAN_EU3A05_GPIO(config, m_gpio, 0);
	m_gpio->read_0_callback().set_ioport("IN0");
	m_gpio->read_1_callback().set_ioport("IN1");
	m_gpio->read_2_callback().set_ioport("IN2");

	ELAN_EU3A05_SYS(config, m_sys, 0);
	m_sys->set_cpu("maincpu");
	m_sys->set_addrbank("bank");

	ELAN_EU3A05_VID(config, m_vid, 0);
	m_vid->set_cpu("maincpu");
	m_vid->set_addrbank("bank");
	m_vid->set_palette("palette");
	m_vid->set_entries(256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ELAN_EU3A05_SOUND(config, m_sound, 8000);
	m_sound->space_read_callback().set(FUNC(elan_eu3a05_state::read_full_space));
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.0);
	/* just causes select sound to loop in Tetris for now!
	m_sound->sound_end_cb<0>().set(FUNC(elan_eu3a05_state::sound_end0));
	m_sound->sound_end_cb<1>().set(FUNC(elan_eu3a05_state::sound_end1));
	m_sound->sound_end_cb<2>().set(FUNC(elan_eu3a05_state::sound_end2));
	m_sound->sound_end_cb<3>().set(FUNC(elan_eu3a05_state::sound_end3));
	m_sound->sound_end_cb<4>().set(FUNC(elan_eu3a05_state::sound_end4));
	m_sound->sound_end_cb<5>().set(FUNC(elan_eu3a05_state::sound_end5));
	*/
}

void elan_eu3a05_state::airblsjs(machine_config& config)
{
	elan_eu3a05(config);
	m_screen->set_refresh_hz(50);
}


ROM_START( rad_tetr )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "tetrisrom.bin", 0x000000, 0x100000, CRC(40538e08) SHA1(1aef9a2c678e39243eab8d910bb7f9f47bae0aee) )
	ROM_RELOAD(0x100000, 0x100000)
	ROM_RELOAD(0x200000, 0x100000)
	ROM_RELOAD(0x300000, 0x100000)
ROM_END

ROM_START( rad_sinv )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "spaceinvadersrom.bin", 0x000000, 0x100000, CRC(5ffb2c8f) SHA1(9bde42ec5c65d9584a802de7d7c8b842ebf8cbd8) )
	ROM_RELOAD(0x100000, 0x100000)
	ROM_RELOAD(0x200000, 0x100000)
	ROM_RELOAD(0x300000, 0x100000)
ROM_END

ROM_START( airblsjs )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "airblsjs.bin", 0x000000, 0x400000, BAD_DUMP CRC(d10a6a84) SHA1(fa65f06e7da229006ddaffb245eef2cc4f90a66d) ) // ROM probably ok, but needs verification pass
ROM_END


CONS( 2004, rad_sinv, 0, 0, elan_eu3a05, rad_sinv, elan_eu3a05_state, empty_init, "Radica (licensed from Taito)",                      "Space Invaders [Lunar Rescue, Colony 7, Qix, Phoenix] (Radica, Arcade Legends TV Game)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // "5 Taito games in 1"

CONS( 2004, rad_tetr, 0, 0, elan_eu3a05, rad_tetr, elan_eu3a05_state, empty_init, "Radica (licensed from Elorg / The Tetris Company)", "Tetris (Radica, Arcade Legends TV Game)", MACHINE_NOT_WORKING ) // "5 Tetris games in 1"

// ROM contains the string "Credit:XiAn Hummer Software Studio(CHINA) Tel:86-29-84270600 Email:HummerSoft@126.com"  PCB has datecode of "050423" (23rd April 2005)
CONS( 2005, airblsjs, 0, 0, airblsjs, airblsjs, elan_eu3a05_state, empty_init, "Advance Bright Ltd", "Air-Blaster Joystick (AB1500, PAL)", MACHINE_NOT_WORKING )
