// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Pierpaolo Prazzoli
// thanks-to:HIGHWAYMAN
/*
    Wink    -   (c) 1985 Midcoin

    TODO:
    - better interrupts?
    - finish sound
    - fix protection properly
    - better handling of nvram? it loses the default values
    - I need a better comparison screenshot to be sure about the colors.
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class wink_state : public driver_device
{
public:
	wink_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram")
	{ }

	void wink(machine_config &config);

	void init_wink();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_sound_flag = 0U;
	uint8_t m_tile_bank = 0U;

	bool m_nmi_enable = false;

	void bgram_w(offs_t offset, uint8_t data);
	void nmi_clock_w(int state);
	void nmi_enable_w(int state);
	void player_mux_w(int state);
	void tile_banking_w(int state);
	template<int Player> void coin_counter_w(int state);
	uint8_t analog_port_r();
	uint8_t player_inputs_r();
	void sound_irq_w(uint8_t data);
	uint8_t prot_r();
	void prot_w(uint8_t data);
	uint8_t sound_r();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update_wink(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(wink_sound);
	void wink_io(address_map &map) ATTR_COLD;
	void wink_map(address_map &map) ATTR_COLD;
	void wink_sound_io(address_map &map) ATTR_COLD;
	void wink_sound_map(address_map &map) ATTR_COLD;
};


TILE_GET_INFO_MEMBER(wink_state::get_bg_tile_info)
{
	uint8_t *videoram = m_videoram;
	int code = videoram[tile_index];
	code |= 0x200 * m_tile_bank;

	// the 2 parts of the screen use different tile banking
	if(tile_index < 0x360)
	{
		code |= 0x100;
	}

	tileinfo.set(0, code, 0, 0);
}

void wink_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wink_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t wink_state::screen_update_wink(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void wink_state::bgram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void wink_state::nmi_clock_w(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void wink_state::nmi_enable_w(int state)
{
	m_nmi_enable = state;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void wink_state::player_mux_w(int state)
{
	//player_mux = state;
	//no mux / cocktail mode in the real pcb? strange...
}

void wink_state::tile_banking_w(int state)
{
	m_tile_bank = state;
	m_bg_tilemap->mark_all_dirty();
}

template<int Player>
void wink_state::coin_counter_w(int state)
{
	machine().bookkeeping().coin_counter_w(Player, state);
}

uint8_t wink_state::analog_port_r()
{
	return ioport(/* player_mux ? "DIAL2" : */ "DIAL1")->read();
}

uint8_t wink_state::player_inputs_r()
{
	return ioport(/* player_mux ? "INPUTS2" : */ "INPUTS1")->read();
}

void wink_state::sound_irq_w(uint8_t data)
{
	m_audiocpu->set_input_line(0, HOLD_LINE);
	//sync with sound cpu (but it still loses some soundlatches...)
	//machine().scheduler().synchronize();
}

void wink_state::wink_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x97ff).ram().share("nvram");
	map(0xa000, 0xa3ff).ram().w(FUNC(wink_state::bgram_w)).share("videoram");
}


uint8_t wink_state::prot_r()
{
	//take a0-a7 and do some math using the variable created from the upper address-lines,
	//put the result onto the databus.

/*
math

take 2 bytes:

byte1 = a8,a9,a10,a11,a12,a13,a14,a15
byte2 = a0,a2,a4,a6,a1,a3,a5,a7

add the 2 bytes together so that if the final value overflows past 255 it wraps to 0
the 8bit result is placed on the databus.

*/
	return 0x20; //hack to pass the jump calculated using this value
}

void wink_state::prot_w(uint8_t data)
{
	//take a9-a15 and stuff them in a variable for later use.
}

void wink_state::wink_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x1f).ram().w("palette", FUNC(palette_device::write8)).share("palette"); //0x10-0x1f is likely to be something else
	map(0x20, 0x27).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x40, 0x40).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x60, 0x60).w(FUNC(wink_state::sound_irq_w));
	map(0x80, 0x80).r(FUNC(wink_state::analog_port_r));
	map(0xa0, 0xa0).r(FUNC(wink_state::player_inputs_r));
	map(0xa4, 0xa4).portr("DSW1");   //dipswitch bank2
	map(0xa8, 0xa8).portr("DSW2");   //dipswitch bank1
//  map(0xac, 0xac).nopw();            //protection - loads video xor unit (written only once at startup)
	map(0xb0, 0xb0).portr("DSW3");   //unused inputs
	map(0xb4, 0xb4).portr("DSW4");   //dipswitch bank3
	map(0xc0, 0xdf).w(FUNC(wink_state::prot_w));       //load load protection-buffer from upper address bus
	map(0xc3, 0xc3).nopr();             //watchdog?
	map(0xe0, 0xff).r(FUNC(wink_state::prot_r));        //load math unit from buffer & lower address-bus
}

void wink_state::wink_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x8000, 0x8000).r("soundlatch", FUNC(generic_latch_8_device::read));
}

void wink_state::wink_sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x80, 0x80).w("aysnd", FUNC(ay8910_device::address_w));
}

static INPUT_PORTS_START( wink )
	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(3) PORT_REVERSE

	PORT_START("INPUTS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )    // right curve
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )    // left curve
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )    // slam

	PORT_START("DSW1")
	PORT_DIPNAME( 0x11, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x10, "60k/120k/240k/480k" )
	PORT_DIPSETTING(    0x01, "80k/160k/320k/640k" )
	PORT_DIPSETTING(    0x00, "100k/200k/400k/800k" )
	PORT_DIPSETTING(    0x11, DEF_STR( None ) )
	PORT_DIPNAME( 0x26, 0x26, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x26, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x24, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x22, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_5C ) )
	PORT_DIPNAME( 0xc8, 0xc8, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0xc8, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x48, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_5C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x11, 0x11, "Ball Save Barrier" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x11, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "1 Credit Award" )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x88, 0x80, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x88, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x60, 0x40, "Timer Speed" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x60, "Fast" )
	PORT_DIPSETTING(    0x20, "Very Fast" )

	PORT_START("DSW3")
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

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Summary" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x44, 0x44, "Credit Payout" )
	PORT_DIPSETTING(    0x44, "2.5%" )
	PORT_DIPSETTING(    0x40, "5%" )
	PORT_DIPSETTING(    0x04, "10%" )
	PORT_DIPSETTING(    0x00, "20%" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Reset Summary Stats" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( gfx_wink )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 4 )
GFXDECODE_END

uint8_t wink_state::sound_r()
{
	return m_sound_flag;
}

//AY portA is fed by an input clock at 15625 Hz
INTERRUPT_GEN_MEMBER(wink_state::wink_sound)
{
	m_sound_flag ^= 0x80;
}

void wink_state::machine_start()
{
	save_item(NAME(m_sound_flag));
	save_item(NAME(m_tile_bank));
	save_item(NAME(m_nmi_enable));
}

void wink_state::machine_reset()
{
	m_sound_flag = 0;
}

void wink_state::wink(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12000000 / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &wink_state::wink_map);
	m_maincpu->set_addrmap(AS_IO, &wink_state::wink_io);

	ls259_device &mainlatch(LS259(config, "mainlatch"));
	mainlatch.q_out_cb<0>().set(FUNC(wink_state::nmi_enable_w));
	mainlatch.q_out_cb<1>().set(FUNC(wink_state::player_mux_w)); //??? no mux on the pcb.
	mainlatch.q_out_cb<2>().set(FUNC(wink_state::tile_banking_w));
	mainlatch.q_out_cb<3>().set_nop();                //?
	mainlatch.q_out_cb<4>().set_nop();                //cab Knocker like in q-bert!
	mainlatch.q_out_cb<5>().set(FUNC(wink_state::coin_counter_w<0>));
	mainlatch.q_out_cb<6>().set(FUNC(wink_state::coin_counter_w<1>));
	mainlatch.q_out_cb<7>().set(FUNC(wink_state::coin_counter_w<2>));

	Z80(config, m_audiocpu, 12000000 / 8);
	m_audiocpu->set_addrmap(AS_PROGRAM, &wink_state::wink_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &wink_state::wink_sound_io);
	m_audiocpu->set_periodic_int(FUNC(wink_state::wink_sound), attotime::from_hz(15625));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(wink_state::screen_update_wink));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(wink_state::nmi_clock_w));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_wink);
	PALETTE(config, "palette").set_format(palette_device::xBRG_444, 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8912_device &aysnd(AY8912(config, "aysnd", 12000000 / 8));
	aysnd.port_a_read_callback().set(FUNC(wink_state::sound_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( wink )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "midcoin-wink00.rom", 0x0000, 0x4000, CRC(044f82d6) SHA1(4269333578c4fb14891b937c683aa5b105a193e7) )
	ROM_LOAD( "midcoin-wink01.rom", 0x4000, 0x4000, CRC(acb0a392) SHA1(428c24845a27b8021823a4a930071b3b47108f01) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "midcoin-wink05.rom", 0x0000, 0x2000, CRC(c6c9d9cf) SHA1(99984905282c2310058d1ce93aec68d8a920b2c0) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "midcoin-wink02.rom", 0x0000, 0x2000, CRC(d1cd9d06) SHA1(3b3ce61a0516cc94663f6d3aff3fea46aceb771f) )
	ROM_LOAD( "midcoin-wink03.rom", 0x2000, 0x2000, CRC(2346f50c) SHA1(a8535fcde0e9782ea61ad18443186fd5a6ebdc7d) )
	ROM_LOAD( "midcoin-wink04.rom", 0x4000, 0x2000, CRC(06dd229b) SHA1(9057cf10e9ec4119297c2d40b26f0ce0c1d7b86a) )
ROM_END

ROM_START( winka )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wink0.bin",    0x0000, 0x4000, CRC(554d86e5) SHA1(bf2de874a62d9137f79063d6ca1906b1ed0c87e6) )
	ROM_LOAD( "wink1.bin",    0x4000, 0x4000, CRC(9d8ad539) SHA1(77246df8195f7e3f3b06edc08d344801bf62e1ba) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "wink5.bin",    0x0000, 0x2000, CRC(c6c9d9cf) SHA1(99984905282c2310058d1ce93aec68d8a920b2c0) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "wink2.bin",    0x0000, 0x2000, CRC(d1cd9d06) SHA1(3b3ce61a0516cc94663f6d3aff3fea46aceb771f) )
	ROM_LOAD( "wink3.bin",    0x2000, 0x2000, CRC(2346f50c) SHA1(a8535fcde0e9782ea61ad18443186fd5a6ebdc7d) )
	ROM_LOAD( "wink4.bin",    0x4000, 0x2000, CRC(06dd229b) SHA1(9057cf10e9ec4119297c2d40b26f0ce0c1d7b86a) )
ROM_END

void wink_state::init_wink()
{
	uint8_t *ROM = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(0x8000);

	// protection module reverse engineered by HIGHWAYMAN

	memcpy(&buffer[0],ROM,0x8000);

	for (uint32_t i = 0x0000; i <= 0x1fff; i++)
		ROM[i] = buffer[bitswap<16>(i,15,14,13, 11,12, 7, 9, 8,10, 6, 4, 5, 1, 2, 3, 0)];

	for (uint32_t i = 0x2000; i <= 0x3fff; i++)
		ROM[i] = buffer[bitswap<16>(i,15,14,13, 10, 7,12, 9, 8,11, 6, 3, 1, 5, 2, 4, 0)];

	for (uint32_t i = 0x4000; i <= 0x5fff; i++)
		ROM[i] = buffer[bitswap<16>(i,15,14,13,  7,10,11, 9, 8,12, 6, 1, 3, 4, 2, 5, 0)];

	for (uint32_t i = 0x6000; i <= 0x7fff; i++)
		ROM[i] = buffer[bitswap<16>(i,15,14,13, 11,12, 7, 9, 8,10, 6, 4, 5, 1, 2, 3, 0)];

	for (uint32_t i = 0; i < 0x8000; i++)
		ROM[i] += bitswap<8>(i & 0xff, 7,5,3,1,6,4,2,0);
}

} // anonymous namespace


GAME( 1985, wink,  0,    wink, wink, wink_state, init_wink, ROT0, "Midcoin", "Wink (set 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1985, winka, wink, wink, wink, wink_state, init_wink, ROT0, "Midcoin", "Wink (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
