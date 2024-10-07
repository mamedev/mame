// license:BSD-3-Clause
// copyright-holders:David Haywood

// The TV Board Game units have "Programmed by E.I. HK Development LTD." in the graphics

// To perform the hidden ROM check do Up + Button A while booting up, then on the black screen Down + Button B.
// This is probably impossible on the single button units using real hardware as the 'B' input isn't connected
// Currently these checksums fail in MAME due to the interrupt hack mapping over ROM, if you remove that hack they pass

#include "emu.h"
#include "elan_eu3a05_a.h"
#include "elan_eu3a05gpio.h"
#include "elan_ep3a19asys.h"
#include "elan_eu3a05vid.h"

#include "cpu/m6502/m6502.h"
#include "machine/bankdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class elan_ep3a19a_state : public driver_device
{
public:
	elan_ep3a19a_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sys(*this, "sys"),
		m_gpio(*this, "gpio"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_ram(*this, "ram"),
		m_sound(*this, "eu3a05sound"),
		m_vid(*this, "vid"),
		m_bank(*this, "bank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void elan_ep3a19a(machine_config &config);

	void init_tvbg();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<elan_ep3a19asys_device> m_sys;
	required_device<elan_eu3a05gpio_device> m_gpio;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);

	// for callback
	uint8_t read_full_space(offs_t offset);

	void elan_ep3a19a_bank_map(address_map &map) ATTR_COLD;
	void elan_ep3a19a_map(address_map &map) ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

	required_shared_ptr<uint8_t> m_ram;
	required_device<elan_eu3a05_sound_device> m_sound;
	required_device<elan_eu3a05vid_device> m_vid;
	required_device<address_map_bank_device> m_bank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	//void sound_end0(int state) { m_sys->generate_custom_interrupt(2); }
	//void sound_end1(int state) { m_sys->generate_custom_interrupt(3); }
	//void sound_end2(int state) { m_sys->generate_custom_interrupt(4); }
	//void sound_end3(int state) { m_sys->generate_custom_interrupt(5); }
	//void sound_end4(int state) { m_sys->generate_custom_interrupt(6); }
	//void sound_end5(int state) { m_sys->generate_custom_interrupt(7); }

	uint8_t nmi_vector_r(offs_t offset)
	{
		return 0xffd4 >> (offset * 8);
	}

};

void elan_ep3a19a_state::video_start()
{
}

uint32_t elan_ep3a19a_state::screen_update(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	return m_vid->screen_update(screen, bitmap, cliprect);
}

// sound callback
uint8_t elan_ep3a19a_state::read_full_space(offs_t offset)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	return fullbankspace.read_byte(offset);
}

void elan_ep3a19a_state::elan_ep3a19a_map(address_map &map)
{
	// can the addresses move around?
	map(0x0000, 0x3fff).ram().share("ram");
	map(0x4800, 0x49ff).rw(m_vid, FUNC(elan_eu3a05commonvid_device::palette_r), FUNC(elan_eu3a05commonvid_device::palette_w));

	map(0x5000, 0x5014).m(m_sys, FUNC(elan_ep3a19asys_device::map)); // including DMA controller
	map(0x5020, 0x503f).m(m_vid, FUNC(elan_eu3a05vid_device::map));

	// 504x GPIO area?
	map(0x5040, 0x5046).rw(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_r), FUNC(elan_eu3a05gpio_device::gpio_w));
	// 5047
	//map(0x5048, 0x504a).w(m_gpio, FUNC(elan_eu3a05gpio_device::gpio_unk_w));

	// 506x unknown
	//map(0x5060, 0x506d).ram(); // read/written by tetris (ADC?)

	// 508x sound
	map(0x5080, 0x50bf).m(m_sound, FUNC(elan_eu3a05_sound_device::map));

	//map(0x5000, 0x50ff).ram();
	map(0x6000, 0xdfff).m(m_bank, FUNC(address_map_bank_device::amap8));

	map(0xe000, 0xffff).rom().region("maincpu", 0x0000);
	// not sure how these work, might be a modified 6502 core instead.
	//map(0xfffa, 0xfffb).r(m_sys, FUNC(elan_eu3a05commonsys_device::nmi_vector_r)); // custom vectors handled with NMI for now
	map(0xfffa, 0xfffb).r(FUNC(elan_ep3a19a_state::nmi_vector_r)); // custom vectors handled with NMI for now

	//map(0xfffe, 0xffff).r(m_sys, FUNC(elan_eu3a05commonsys_device::irq_vector_r));  // allow normal IRQ for brk
}

void elan_ep3a19a_state::elan_ep3a19a_bank_map(address_map &map)
{
	map(0x000000, 0x3fffff).mirror(0xc00000).noprw();
	map(0x000000, 0x3fffff).mirror(0xc00000).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( tvbg_1button )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // the 6-in-1 units have a single button marked with both A and B (unless it depends which side of the button you press?)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tvbg_2button )
	PORT_INCLUDE( tvbg_1button )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Boggle uses 2 buttons for gameplay, other units do read this to enter secret test mode, but none of the games need it?
INPUT_PORTS_END


void elan_ep3a19a_state::machine_start()
{
}

void elan_ep3a19a_state::machine_reset()
{
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

INTERRUPT_GEN_MEMBER(elan_ep3a19a_state::interrupt)
{
	//m_sys->generate_custom_interrupt(9);
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void elan_ep3a19a_state::elan_ep3a19a(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(21'477'272)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &elan_ep3a19a_state::elan_ep3a19a_map);
	m_maincpu->set_vblank_int("screen", FUNC(elan_ep3a19a_state::interrupt));

	ADDRESS_MAP_BANK(config, "bank").set_map(&elan_ep3a19a_state::elan_ep3a19a_bank_map).set_options(ENDIANNESS_LITTLE, 8, 24, 0x8000);

	PALETTE(config, m_palette).set_entries(256);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(elan_ep3a19a_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_elan_eu3a05_fake);

	ELAN_EU3A05_GPIO(config, m_gpio, 0);
	m_gpio->read_0_callback().set_ioport("IN0");
	m_gpio->read_1_callback().set_ioport("IN1");
	m_gpio->read_2_callback().set_ioport("IN2");

	ELAN_EP3A19A_SYS(config, m_sys, 0);
	m_sys->set_cpu("maincpu");
	m_sys->set_addrbank("bank");

	ELAN_EU3A05_VID(config, m_vid, 0);
	m_vid->set_cpu("maincpu");
	m_vid->set_addrbank("bank");
	m_vid->set_palette("palette");
	m_vid->set_entries(256);
	m_vid->set_is_pvmilfin();
	m_vid->set_use_spritepages();
	m_vid->set_force_basic_scroll();

	/* sound hardware */
	SPEAKER(config, "mono").front_center();


	ELAN_EU3A05_SOUND(config, m_sound, 8000);
	m_sound->space_read_callback().set(FUNC(elan_ep3a19a_state::read_full_space));
	m_sound->add_route(ALL_OUTPUTS, "mono", 1.0);

	/*
	m_sound->sound_end_cb<0>().set(FUNC(elan_ep3a19a_state::sound_end0));
	m_sound->sound_end_cb<1>().set(FUNC(elan_ep3a19a_state::sound_end1));
	m_sound->sound_end_cb<2>().set(FUNC(elan_ep3a19a_state::sound_end2));
	m_sound->sound_end_cb<3>().set(FUNC(elan_ep3a19a_state::sound_end3));
	m_sound->sound_end_cb<4>().set(FUNC(elan_ep3a19a_state::sound_end4));
	m_sound->sound_end_cb<5>().set(FUNC(elan_ep3a19a_state::sound_end5));
	*/
}

ROM_START( tvbg6a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "candyland_hhh_silly6.bin", 0x00000, 0x200000, CRC(8b16d725) SHA1(06af509d03df0e5a2ca502743797af9f4a5dc6f1) )
	ROM_RELOAD(0x200000,0x200000)
ROM_END

ROM_START( tvbg6b )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bship_simon_mousetrap.bin", 0x00000, 0x200000, CRC(b0627a98) SHA1(6157e26916bb415037a4d122d3075cbfb8e61dcf) )
	ROM_RELOAD(0x200000,0x200000)
ROM_END

ROM_START( tvbg3a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hhhssp.bin", 0x00000, 0x100000, CRC(7e23a5a0) SHA1(2cd0f7572df30d2565b64fa0936715f71312ab1a) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_RELOAD(0x200000,0x100000)
	ROM_RELOAD(0x300000,0x100000)
ROM_END

ROM_START( tvbg3b )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "simonbship.bin", 0x00000, 0x100000, CRC(9b10a87a) SHA1(f2022ac07468d911cfb3d32887d6e59e60d48d51) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_RELOAD(0x200000,0x100000)
	ROM_RELOAD(0x300000,0x100000)
ROM_END

ROM_START( tvbg3c )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "boggle_connect4.bin", 0x00000, 0x100000, CRC(c2374eea) SHA1(c6971cb5108828bc72fd1cf7edeb53915d196db7) )
	ROM_RELOAD(0x100000,0x100000)
	ROM_RELOAD(0x200000,0x100000)
	ROM_RELOAD(0x300000,0x100000)
ROM_END

void elan_ep3a19a_state::init_tvbg()
{
	// is this swapping internal to the ep3a19a type ELAN, or external; ROM glob had standard TSOP pinout pads that were used for dumping.
	uint8_t* ROM = memregion("maincpu")->base();
	for (int i = 0; i < 0x400000; i++)
	{
		ROM[i] = bitswap<8>(ROM[i], 6, 5, 7, 0, 2, 3, 1, 4);
	}
}

} // anonymous namespace


CONS( 2007, tvbg6a, 0, 0, elan_ep3a19a, tvbg_1button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 6-in-1: Silly 6 Pins, Candy Land, Hungry Hungry Hippos, Match 'em, Mixin' Pics, Checkers", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=zajzQo47YYA
CONS( 2007, tvbg6b, 0, 0, elan_ep3a19a, tvbg_1button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 6-in-1: Simon, Battleship, Mouse Trap, Checkers, Link-a-Line, Roll Over", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=JbrR67kY8MI

CONS( 2007, tvbg3a, 0, 0, elan_ep3a19a, tvbg_2button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 3-in-1: Silly 6 Pins, Hungry Hungry Hippos, Match 'em", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, tvbg3b, 0, 0, elan_ep3a19a, tvbg_2button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 3-in-1: Simon, Battleship, Checkers", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=Q7nwKJfVavU
CONS( 2007, tvbg3c, 0, 0, elan_ep3a19a, tvbg_2button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 3-in-1: Boggle, Connect 4, Roll Over", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=SoKKIKSDGhY

// The back of the Silly 6 Pins 3-in-1 packaging suggests a Monopoly TV Board Game device was planned, but this does not appear to have been released.
