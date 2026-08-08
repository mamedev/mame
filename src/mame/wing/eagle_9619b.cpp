// license:BSD-3-Clause
// copyright-holders:

/*
Eagle 9619-B PCB

Main components:

HD647180X0P8 (internal ROM not used)
8 MHz XTAL
36.8640MHz XTAL
24.57600 MHz XTAL
Z0840006PSC
D72001C-11 MPSC
MSM6242-compatible RTC (chip type non readable on pic)
093062 square 208-pin custom
09D074 square 80-pin custom
Sanyo LC83020 DSP (?)
Oki M6295
unpopulated space marked for YM3438
2x bank of 8 switches

TODO:
- reel tilemap
- DSP
- one of the custom does sound? see rom 6. Although it doesn't seem to be tested in test mode.
- 10 buttons panel type
- hopper
- lamps / outputs
- NVRAM
- decent key assignments
*/


#include "emu.h"

#include "cpu/z180/hd647180x.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/msm6242.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class eagle_9619b_state : public driver_device
{
public:
	eagle_9619b_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_oki(*this, "oki"),
		m_tileram(*this, "tileram%u", 0U),
		m_attrram(*this, "attrram%u", 0U),
		m_colorram(*this, "colorram%u", 0U)
	{ }

	void trfruits(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<okim6295_device> m_oki;

	required_shared_ptr_array<uint8_t, 2> m_tileram;
	required_shared_ptr_array<uint8_t, 2> m_attrram;
	required_shared_ptr_array<uint8_t, 2> m_colorram;

	tilemap_t *m_tilemap[2] {};

	uint8_t m_dsp_siak = 0;

	template <uint8_t Which> TILE_GET_INFO_MEMBER(get_tile_info);
	template <uint8_t Which> void tileram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void attrram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> void colorram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void counters_w(uint8_t data);

	void sound_reset_w(uint8_t data);
	void sound_ctrl_w(uint8_t data);
	uint8_t dsp_status_r();

	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
	void audio_program_map(address_map &map) ATTR_COLD;
	void audio_io_map(address_map &map) ATTR_COLD;
};


void eagle_9619b_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(eagle_9619b_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(eagle_9619b_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 128, 64);

	m_tilemap[1]->set_transparent_pen(0);
}

template<uint8_t Which>
void eagle_9619b_state::tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template<uint8_t Which>
void eagle_9619b_state::attrram_w(offs_t offset, uint8_t data)
{
	m_attrram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template<uint8_t Which>
void eagle_9619b_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <uint8_t Which>
TILE_GET_INFO_MEMBER(eagle_9619b_state::get_tile_info)
{
	int const tile = (m_tileram[Which][tile_index] | (m_attrram[Which][tile_index] << 8)) & 0x7fff;
	int const color = m_colorram[Which][tile_index] & 0x1f;

	tileinfo.set(0, tile, color, 0);
}

uint32_t eagle_9619b_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	// m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void eagle_9619b_state::machine_start()
{
	save_item(NAME(m_dsp_siak));
}

void eagle_9619b_state::counters_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // coin in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1)); // coin out
	machine().bookkeeping().coin_counter_w(2, BIT(data, 2)); // drop coin
	machine().bookkeeping().coin_counter_w(3, BIT(data, 3)); // games
	machine().bookkeeping().coin_counter_w(4, BIT(data, 4)); // attendant out 1/1
	machine().bookkeeping().coin_counter_w(5, BIT(data, 5)); // lockout
	machine().bookkeeping().coin_counter_w(6, BIT(data, 6)); // jackpot lock
	machine().bookkeeping().coin_counter_w(7, BIT(data, 7)); // attendant out 1/10
}

void eagle_9619b_state::sound_reset_w(uint8_t data)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
}

void eagle_9619b_state::sound_ctrl_w(uint8_t data)
{
	m_oki->set_rom_bank(BIT(data, 0));

	m_dsp_siak = BIT(data, 6);

}

uint8_t eagle_9619b_state::dsp_status_r()
{
	return 0xfe | m_dsp_siak;   // bit 0 = /SIAK
}


void eagle_9619b_state::main_program_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x0e000, 0x0ffff).ram(); // NVRAM
	map(0x40000, 0x4001f).ram(); // video regs?
	map(0x40800, 0x4083f).ram(); // some regs?
	map(0x41000, 0x417ff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0x44000, 0x45fff).ram(); // ??
	map(0x48000, 0x49fff).ram().w(FUNC(eagle_9619b_state::tileram_w<0>)).share(m_tileram[0]);
	map(0x50000, 0x51fff).ram().w(FUNC(eagle_9619b_state::attrram_w<0>)).share(m_attrram[0]);
	map(0x58000, 0x59fff).ram().w(FUNC(eagle_9619b_state::colorram_w<0>)).share(m_colorram[0]);
	map(0x60800, 0x6083f).ram(); // some regs?
	map(0x64000, 0x65fff).ram(); // ??
	map(0x68000, 0x69fff).ram().w(FUNC(eagle_9619b_state::tileram_w<1>)).share(m_tileram[1]);
	map(0x70000, 0x71fff).ram().w(FUNC(eagle_9619b_state::attrram_w<1>)).share(m_attrram[1]);
	map(0x78000, 0x79fff).ram().w(FUNC(eagle_9619b_state::colorram_w<1>)).share(m_colorram[1]);
}

void eagle_9619b_state::main_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x00, 0x7f).noprw();
	map(0x80, 0x8f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0xb0, 0xb0).portr("IN0");
	map(0xb1, 0xb1).portr("IN1");
	map(0xb2, 0xb2).portr("IN2");
	map(0xb3, 0xb3).portr("IN3");
	map(0xb4, 0xb4).portr("IN4").w(FUNC(eagle_9619b_state::counters_w));
	map(0xb5, 0xb5).portr("IN5"); // .w() //  bit 0 and 3 coin enable, bit 2 coin diverter
	// map(0xb6, 0xb6).w() // bit 0 hopper motor, bit 1 jackpot bell, bit 6 t-tilt lamp, bit 7 t-change lamp
	map(0xb7, 0xb7).portr("BATTERY").w(FUNC(eagle_9619b_state::sound_reset_w));
	map(0xb8, 0xb8).portr("DSW1");
	map(0xb9, 0xb9).portr("DSW2").w("soundlatch", FUNC(generic_latch_8_device::write));
}

void eagle_9619b_state::audio_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xf800, 0xffff).ram();
}

void eagle_9619b_state::audio_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x7c, 0x7c).w(FUNC(eagle_9619b_state::sound_ctrl_w));
	map(0x7d, 0x7d).r(FUNC(eagle_9619b_state::dsp_status_r));
	map(0x80, 0x80).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc0, 0xc0).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( trfruits )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) // spin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Max Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("Change")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Drop Coin")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin A In
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("Coin A Error")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("Main Door")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("Drop Door")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("Logic Door")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("Hopper Coin")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_PLAYER(1) PORT_NAME("Hopper Overflow")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_PLAYER(1)  PORT_NAME("Hopper Empty")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) // Coin B Up
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_PLAYER(1) PORT_NAME("Coin B Down")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_PLAYER(1) PORT_NAME("Top Door")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_PLAYER(1) PORT_NAME("Bill Door")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no effect shown in I/O test
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_PLAYER(1) PORT_NAME("Bet 100")

	PORT_START("IN5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_PLAYER(1) PORT_NAME("Bet 10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_PLAYER(1) PORT_NAME("Bet 25")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_PLAYER(1) PORT_NAME("Bet 50")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("Bet 75")

	// DIP definitions according to test mode, not verified in-game
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Key In" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "5" )
	PORT_DIPSETTING(    0x06, "10" )
	PORT_DIPSETTING(    0x05, "25" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_100C ) )
	PORT_DIPNAME( 0x40, 0x40, "Hopper Limit" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "400" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x80, 0x80, "Panel Type" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Full Buttons" )
	PORT_DIPSETTING(    0x00, "10 Buttons" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Hopper Coin Switch Polarity" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x06, 0x06, "P/O" ) PORT_DIPLOCATION("SW2:2,3") // ??
	PORT_DIPSETTING(    0x06, "Type 1" )
	PORT_DIPSETTING(    0x04, "Type 2" )
	PORT_DIPSETTING(    0x02, "Type 3" )
	PORT_DIPSETTING(    0x00, "Type 4" )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_50C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_100C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BATTERY")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("soundlatch", FUNC(generic_latch_8_device::pending_r))
	PORT_CONFNAME( 0x02, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x02, "Low Battery" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED ) // or so it seems
INPUT_PORTS_END


static GFXDECODE_START( gfx_trfruits )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x5_planar, 0, 32 )
GFXDECODE_END


void eagle_9619b_state::trfruits(machine_config &config)
{
	HD647180X(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &eagle_9619b_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &eagle_9619b_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(eagle_9619b_state::irq0_line_hold));

	Z80(config, m_audiocpu, 24.576_MHz_XTAL / 4); // clock / divider not verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &eagle_9619b_state::audio_program_map);
	m_audiocpu->set_addrmap(AS_IO, &eagle_9619b_state::audio_io_map);

	MSM6242(config, "rtc", 32.768_kHz_XTAL); // TODO: identify exact chip

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	// D72001C, although there's no sign of it being used by the dumped game

	// TODO: everything
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128*8, 64*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 50*8-1);
	screen.set_screen_update(FUNC(eagle_9619b_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_trfruits);

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 0x800);

	SPEAKER(config, "mono").front_center();

	// LC83020

	OKIM6295(config, m_oki, 1'056'000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}


ROM_START( trfruits )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "tropicalfruits_9619__v11.01.e1", 0x00000, 0x20000, CRC(6c713a05) SHA1(8bc2e32e8b14b82cbf52f48ac33093b61205f085) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "8.j5", 0x00000, 0x10000, CRC(98086355) SHA1(991c2d336c0e6eb6e8d1de4bbbf2198213bcbd0d) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x140000, "tiles", 0 )
	ROM_LOAD( "1.j1", 0x000000, 0x40000, CRC(7d24fa30) SHA1(ff3c13cb56fb4ea3dccd4f4d2daa17721594c59c) )
	ROM_LOAD( "2.f1", 0x040000, 0x40000, CRC(836d5afd) SHA1(d4853af7b7ef6966225f2e6663f179e7f3a6d3aa) )
	ROM_LOAD( "3.j2", 0x080000, 0x40000, CRC(6ea67001) SHA1(09b07a31e1fb8dc60153801aa0c442f30d6c6e93) )
	ROM_LOAD( "4.f2", 0x0c0000, 0x40000, CRC(f2cc9e1d) SHA1(735630a2f1d471ca1b7377b7aed53525088cd04a) )
	ROM_LOAD( "5.e2", 0x100000, 0x40000, CRC(29ce45fe) SHA1(0884a582f402e830b316fc9f624c80a6dab444b1) )

	ROM_REGION( 0x80000, "samples", 0 )
	ROM_LOAD( "6.m5", 0x00000, 0x80000, CRC(03a76cd7) SHA1(de6872c0953db617e593d3bcd3d429022269dff5) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "7.k5", 0x00000, 0x80000, CRC(dd56e2c3) SHA1(b184a901f80095a12c299e94e70a800978d0af80) )
ROM_END

} // anonymous namespace


GAME( 200?, trfruits, 0, trfruits, trfruits, eagle_9619b_state, empty_init, ROT0, "Eagle", "Tropical Fruits (v11.01)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
