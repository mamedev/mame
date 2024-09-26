// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*******************************************************************************************

ZG board (c) 1991 Alba

Notes:
-The name of this hardware is "Alba ZG board",a newer revision of the
 "Alba ZC board" used by Hanaroku (seta/albazc.cpp driver). Test mode says clearly that this is
 from 1991.

TODO:
-Player-2 inputs are unemulated;
-"Custom RAM" emulation: might be a (weak) protection device or related to the "Back-up RAM NG"
 msg that pops up at every start-up.
-Video emulation requires a major conversion to the HD46505SP C.R.T. chip (MC6845 clone),
 there's an heavy x offsetting with the flip screen right now due of that (sets register
 0x0d to 0x80 when the screen is upside-down)
-You can actually configure the coin chutes / coin lockout active high/low (!), obviously
 MAME framework isn't really suitable for it at the current time;

PCB:
- HD46505SP-2 / HD68B45SP Japan
- Mostek MK3880P CPU, Z80 clone
- NEC D8255AC-2
- AY38910A/P
- X1-009 (labeled 8732K5), X1-0198 (or X1-019B, can't read)
- X2-004, X2-003, AX-014 (all with epoxy modules apparently)
- X1-007
- CR-203 lithium battery, near X1-009 and X1-0198. There is also a switch near it
- Xtal 12 MHz at top right corner

*******************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/i8255.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PROTRAM (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PROTRAM)

#include "logmacro.h"

#define LOGPROTRAM(...) LOGMASKED(LOG_PROTRAM, __VA_ARGS__)

namespace {

class albazg_state : public driver_device
{
public:
	albazg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_custom_ram(*this, "custom_ram")
		, m_video_ram(*this, "video_ram")
		, m_attr_ram(*this, "attr_ram")
		, m_rombank(*this, "rombank")
		, m_gfxdecode(*this, "gfxdecode")
		, m_key_in(*this, "P1_IN%u", 0U)
		, m_coin_in(*this, "COIN")
	{ }

	virtual void yumefuda(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void vram_w(offs_t offset, uint8_t data);
	void attr_w(offs_t offset, uint8_t data);
	uint8_t custom_ram_r(offs_t offset);
	void custom_ram_w(offs_t offset, uint8_t data);
	void prot_lock_w(uint8_t data);
	uint8_t key_matrix_r();
	void key_matrix_w(uint8_t data);
	void output_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	tilemap_t *m_tilemap = nullptr;
	uint8_t m_port_select = 0;
	uint8_t m_prot_lock = 0;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_custom_ram;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_attr_ram;
	required_memory_bank m_rombank;
	required_device<gfxdecode_device> m_gfxdecode;

	required_ioport_array<6> m_key_in;
	required_ioport m_coin_in;
};

TILE_GET_INFO_MEMBER(albazg_state::get_tile_info)
{
	const u8 attr = m_attr_ram[tile_index];
	const u16 code = m_video_ram[tile_index] | ((attr & 0xf8) << 3);

	tileinfo.set(
		0,
		code,
		attr & 0x7,
		0
	);
}


void albazg_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(albazg_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t albazg_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/***************************************************************************************/

static GFXDECODE_START( gfx_yumefuda )
	GFXDECODE_ENTRY( "tiles", 0x0000, gfx_8x8x4_planar, 0, 8 )
GFXDECODE_END


void albazg_state::vram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void albazg_state::attr_w(offs_t offset, uint8_t data)
{
	m_attr_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

// Custom RAM (Thrash Protection)
uint8_t albazg_state::custom_ram_r(offs_t offset)
{
	LOGPROTRAM("Custom RAM read at %02x PC = %x\n", offset + 0xaf80, m_maincpu->pc());
	return m_custom_ram[offset];// ^ 0x55;
}

void albazg_state::custom_ram_w(offs_t offset, uint8_t data)
{
	LOGPROTRAM("Custom RAM write at %02x : %02x PC = %x\n", offset + 0xaf80, data, m_maincpu->pc());
	if (m_prot_lock)
		m_custom_ram[offset] = data;
}

// this might be used as NVRAM commands btw
void albazg_state::prot_lock_w(uint8_t data)
{
	LOGPROTRAM("PC %04x Prot lock value written %02x\n", m_maincpu->pc(), data);
	m_prot_lock = data;
}

uint8_t albazg_state::key_matrix_r()
{
	u8 res = (m_coin_in->read() & 0xf) | 0xf0;

	for (int i = 0; i < 6; i++)
	{
		// TODO: unverified (and likely not working) multi select
		// (both games just access this one bit at a time, selects with 0x00 for accessing coin section above)
		if (BIT(m_port_select, i))
		{
			res &= 0xf;
			res |= m_key_in[i]->read() & 0xf0;
		}
	}

	return res;
}

void albazg_state::key_matrix_w(uint8_t data)
{
	//0x10000 "Learn Mode"
	//0x12000 gameplay
	//0x14000 bonus game
	//0x16000 ?
	m_rombank->set_entry((data & 0xc0) >> 6);

	m_port_select = data & ~0xc0;
}

void albazg_state::output_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(~data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(~data, 1));
	machine().bookkeeping().coin_lockout_global_w(data & 1);
	//BIT(data, 4) hopper-c (active LOW)
	//BIT(data, 3) divider (active HIGH)
	flip_screen_set(BIT(~data, 5));
}

/***************************************************************************************/

void albazg_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr(m_rombank);
	map(0xa7fc, 0xa7fc).w(FUNC(albazg_state::prot_lock_w));
	map(0xa7ff, 0xa7ff).portw("EEPROMOUT");
	map(0xaf80, 0xafff).rw(FUNC(albazg_state::custom_ram_r), FUNC(albazg_state::custom_ram_w)).share(m_custom_ram);
	map(0xb000, 0xb07f).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xb080, 0xb0ff).ram().w("palette", FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0xc000, 0xc3ff).ram().w(FUNC(albazg_state::vram_w)).share(m_video_ram);
	map(0xd000, 0xd3ff).ram().w(FUNC(albazg_state::attr_w)).share(m_attr_ram);
	map(0xe000, 0xffff).ram();
}

void albazg_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("crtc", FUNC(mc6845_device::address_w));
	map(0x01, 0x01).w("crtc", FUNC(mc6845_device::register_w));
	map(0x40, 0x40).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x40, 0x41).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x80, 0x83).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc0, 0xc0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
}

/***************************************************************************************/

static INPUT_PORTS_START( yumefuda )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset SW") //doesn't work?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Meter SW")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Coin Out")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay Out")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Init SW")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Flip-Flop")  PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coupon") PORT_IMPULSE(2) //coupon
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Note") PORT_IMPULSE(2)  //note
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_IN0")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 BET Button") PORT_CODE(KEYCODE_3) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start") PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_START("P1_IN1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_HANAFUDA_C ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_B ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_A ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_HANAFUDA_D ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_START("P1_IN2")
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("P1 Start") PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 BET Button") PORT_CODE(KEYCODE_3) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_F ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_E ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	// Some bits of these three are actually used if you use the Royal Panel type
	PORT_START("P1_IN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_IN4")
	PORT_BIT( 0x9f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_HANAFUDA_NO ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_HANAFUDA_YES ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_START("P1_IN5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	// Added by translating the manual (both Yumefuda and Hana Awase 6 Part II have the same DIPs)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Learn Mode" ) PORT_DIPLOCATION ("DSW1:!8") //SW Dip-Switches
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x02, IP_ACTIVE_LOW ) PORT_DIPLOCATION ("DSW1:!7")
	PORT_DIPNAME( 0x04, 0x04, "Hopper Payout" ) PORT_DIPLOCATION ("DSW1:!6")
	PORT_DIPSETTING(    0x04, "Hanafuda Type" ) //hanaawase
	PORT_DIPSETTING(    0x00, "Royal Type" )
	PORT_DIPNAME( 0x08, 0x08, "Panel Type" ) PORT_DIPLOCATION ("DSW1:!5")
	PORT_DIPSETTING(    0x08, "Hanafuda Panel" ) //hanaawase
	PORT_DIPSETTING(    0x00, "Royal Panel" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION ("DSW1:!4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION ("DSW1:!3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION ("DSW1:!2") //Screen Orientation
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION ("DSW1:!1") //Screen Flip
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) ) //pressing Flip-Flop button makes the screen flip

	// Unused, on the PCB there's just one bank
	PORT_START("DSW2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

/***************************************************************************************/

void albazg_state::machine_start()
{
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x2000);

	save_item(NAME(m_port_select));
	save_item(NAME(m_prot_lock));
}

void albazg_state::machine_reset()
{
	m_port_select = 0;
	m_prot_lock = 0;
}

void albazg_state::yumefuda(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 2); // unknown divider
	m_maincpu->set_addrmap(AS_PROGRAM, &albazg_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &albazg_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(albazg_state::irq0_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 8); // timing is unknown

	i8255_device &ppi(I8255A(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(albazg_state::key_matrix_w));
	ppi.in_pb_callback().set_ioport("SYSTEM");
	ppi.in_pc_callback().set(FUNC(albazg_state::key_matrix_r));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(albazg_state::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", 12_MHz_XTAL / 16)); // hand tuned to get ~60 fps
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_yumefuda);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x80);

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 12_MHz_XTAL / 16)); // guessed to use the same xtal as the crtc
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.port_a_write_callback().set(FUNC(albazg_state::output_w));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}

/***************************************************************************************/


ROM_START( yumefuda )
	ROM_REGION( 0x10000, "maincpu", 0 ) // code
	ROM_LOAD("zg004y02.u43", 0x0000, 0x8000, CRC(974c543c) SHA1(56aeb318cb00445f133246dfddc8c24bb0c23f2d))
	ROM_LOAD("zg004y01.u42", 0x8000, 0x8000, CRC(ae99126b) SHA1(4ae2c1c804bbc505a013f5e3d98c0bfbb51b747a))

	ROM_REGION( 0x10000, "tiles", 0 )
	ROM_LOAD("zg001003.u3", 0x0000, 0x4000, CRC(5822ff27) SHA1(d40fa0790de3c912f770ef8f610bd8c42bc3500f))
	ROM_LOAD("zg001004.u4", 0x4000, 0x4000, CRC(d8676435) SHA1(9b6df5378948f492717e1a4d9c833ddc5a9e8225))
	ROM_LOAD("zg001005.u5", 0x8000, 0x4000, CRC(158b6cde) SHA1(3e335b7dc1bbae2edb02722025180f32ab91f69f))
	ROM_LOAD("zg001006.u6", 0xc000, 0x4000, CRC(a5df443c) SHA1(a6c088a463c05e43a7b559c5d0afceddc88ef476))

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD("zg1-007.u13", 0x000, 0x104, NO_DUMP ) // PAL
ROM_END

// 花合せ・６ Part Ⅱ
// P0-066A PCB
ROM_START( hana6pt2 )
	ROM_REGION( 0x10000, "maincpu", 0 ) // code
	ROM_LOAD("zg006p11.u42", 0x00000, 0x10000, CRC(4e455ac5) SHA1(df3a327acd2eb8f566ba6b86342d8fb6f7e89560))
	// u43 empty on this PCB

	ROM_REGION( 0x10000, "tiles", 0 ) // same GFX as Yumefuda
	ROM_LOAD("zg001003.u3", 0x0000, 0x4000, CRC(5822ff27) SHA1(d40fa0790de3c912f770ef8f610bd8c42bc3500f))
	ROM_LOAD("zg001004.u4", 0x4000, 0x4000, CRC(d8676435) SHA1(9b6df5378948f492717e1a4d9c833ddc5a9e8225))
	ROM_LOAD("zg001005.u5", 0x8000, 0x4000, CRC(158b6cde) SHA1(3e335b7dc1bbae2edb02722025180f32ab91f69f))
	ROM_LOAD("zg001006.u6", 0xc000, 0x4000, CRC(a5df443c) SHA1(a6c088a463c05e43a7b559c5d0afceddc88ef476))

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD("zg2-007.u13", 0x000, 0x104, NO_DUMP ) // PAL
ROM_END

} // anonymous namespace


GAME( 1991, yumefuda, 0, yumefuda, yumefuda, albazg_state, empty_init, ROT0, "Alba", "Yumefuda",             MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, hana6pt2, 0, yumefuda, yumefuda, albazg_state, empty_init, ROT0, "Alba", "Hana Awase 6 Part II", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
