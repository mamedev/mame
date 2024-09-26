// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Royal Gum

Main components
- Big Black Box in the middle of the PCB. Contains the main CPU (some kind of M6502) and maybe the 6845 (not seen anywhere on PCB)
- 24.000000 Mhz osc
- 2 x M82C55A
- AY38910A/P
- NEC UPD7759
- MK48Z08B-10
- 1 8-dip bank
- 1 4-dip bank


TODO:
- stuck at the play screen with 'attendere' (wait) message after coining up;
- Pressing "pin's switch" causes a "Micro Palline Err" (micro balls error),
  is this some kind of pachinko-like machine?
- some devices aren't mapped and others may be mapped wrong.
*/

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/upd7759.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class rgum_state : public driver_device
{
public:
	rgum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_maincpu(*this, "maincpu"),
		m_aysnd(*this, "aysnd"),
		m_upd(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	void rgum(machine_config &config);

	int heartbeat_r();

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_cram;
	required_device<cpu_device> m_maincpu;
	required_device<ay8910_device> m_aysnd;
	required_device<upd7759_device> m_upd;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	uint8_t m_hbeat = 0;
	bool m_aysnd_data_next = false;
	bool m_aysnd_toggle_enabled = false;

	void aysnd_2000_w(uint8_t data);
	void aysnd_2002_w(uint8_t data);
	void upd_data_w(uint8_t data);
	uint8_t upd_ready_r();
	uint8_t upd_busy_r();
	uint8_t upd_reset_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
};


void rgum_state::video_start()
{
	m_hbeat = 0;

	save_item(NAME(m_hbeat));
	save_item(NAME(m_aysnd_data_next));
	save_item(NAME(m_aysnd_toggle_enabled));
}

void rgum_state::machine_reset()
{
	m_aysnd_data_next = false;
	m_aysnd_toggle_enabled = false;
}

uint32_t rgum_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0;x < 66; x++)
		{
			int tile = m_vram[count] | ((m_cram[count] & 0xf) << 8);

			gfx->opaque(bitmap, cliprect, tile, 0, 0, 0, x * 8, y * 8);

			count++;
		}
	}

	return 0;
}

void rgum_state::aysnd_2000_w(uint8_t data)
{
	m_aysnd_toggle_enabled = true;
}

void rgum_state::aysnd_2002_w(uint8_t data)
{
	// AY-3-8910 interface is bizarre. Address is written to $2000 and then $2002 with successive instructions, then data is written to both $2000 and $2002.
	// Sound data contains many pairs where the address byte is $50. Do these control some other device?
	m_aysnd->address_data_w(m_aysnd_data_next, data);
	if (m_aysnd_toggle_enabled)
	{
		m_aysnd_data_next = !m_aysnd_data_next;
		m_aysnd_toggle_enabled = false;
	}
}

void rgum_state::upd_data_w(u8 data)
{
	m_upd->port_w(data);
	m_upd->start_w(1);
}

uint8_t rgum_state::upd_ready_r()
{
	if (!machine().side_effects_disabled())
	{
		m_upd->reset_w(1);
		m_upd->start_w(0);
	}
	return 0;
}

uint8_t rgum_state::upd_busy_r()
{
	return m_upd->busy_r() ? 0x80 : 0;
}

uint8_t rgum_state::upd_reset_r()
{
	if (!machine().side_effects_disabled())
		m_upd->reset_w(0);
	return 0;
}

void rgum_state::main_map(address_map &map) // TODO: map MK48Z08B-10
{
	map(0x0000, 0x07ff).ram(); // not all of it?

	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x2000, 0x2000).w(FUNC(rgum_state::aysnd_2000_w));
	map(0x2002, 0x2002).r("aysnd", FUNC(ay8910_device::data_r)).w(FUNC(rgum_state::aysnd_2002_w));

	map(0x2800, 0x2800).w(FUNC(rgum_state::upd_data_w));
	map(0x2801, 0x2801).r(FUNC(rgum_state::upd_ready_r));
	map(0x2802, 0x2802).r(FUNC(rgum_state::upd_busy_r));
	map(0x2803, 0x2803).r(FUNC(rgum_state::upd_reset_r));

	map(0x2c00, 0x2c03).w("ppi8255_1", FUNC(i8255_device::write));

	map(0x3000, 0x3003).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x4000, 0x47ff).ram().share(m_vram);
	map(0x5000, 0x57ff).ram().share(m_cram);

	map(0x8000, 0xffff).rom();
}


int rgum_state::heartbeat_r()
{
	return m_hbeat;
}


static INPUT_PORTS_START( rgum )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Stop Reel 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Stop Reel 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Stop Reel 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Stop Reel 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Pin's Switch")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(rgum_state, heartbeat_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Stop Reel 5")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Gum Switch")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // "PAY LOT"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) // "COIN SERV"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Game Level" ) PORT_DIPLOCATION("DSW1:1,2") // 'livello gioco' in test mode
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x02, "60" )
	PORT_DIPSETTING(    0x03, "80" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("DSW1:3") // seems buggy: Italian works as expected while French just puts some disclaimer strings in place of the winning combinations
	PORT_DIPSETTING(    0x00, DEF_STR( Italian ) )
	PORT_DIPSETTING(    0x04, DEF_STR( French ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:1") // must stay on at boot or the game will stop with 'switch 2 dip 1 off' message
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ 0, 1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_rgum )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


void rgum_state::rgum(machine_config &config)
{
	// basic machine hardware
	M65C02(config, m_maincpu, 24_MHz_XTAL / 16);  // divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &rgum_state::main_map);

	// NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // MK48Z08

	i8255_device &ppi(I8255A(config, "ppi8255_0"));
	ppi.in_pa_callback().set_ioport("IN0");
	ppi.in_pb_callback().set_ioport("DSW1");
	ppi.in_pc_callback().set_ioport("DSW2");
	ppi.out_pc_callback().set([this](uint8_t data) { m_hbeat = BIT(data, 0); });

	I8255A(config, "ppi8255_1");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(rgum_state::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 24_MHz_XTAL / 16));   // unknown clock & type, hand tuned to get ~50 fps (?)
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rgum);
	PALETTE(config, m_palette).set_entries(0x100);

	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 24_MHz_XTAL / 16));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50); // guessed to use the same xtal as the crtc
	aysnd.port_a_read_callback().set_ioport("COIN");
	aysnd.port_b_read_callback().set_ioport("IN1");

	UPD7759(config, "upd").add_route(ALL_OUTPUTS, "mono", 0.50);
}




ROM_START( rgum ) // PCB SL65 V2
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rgum.u5", 0x00000, 0x10000, CRC(9d2d1681) SHA1(1c1da0d970ea2cf58f7961417ab6986cc667da5c) ) // first half is empty

	ROM_REGION( 0x2000, "nvram", 0 ) // MK48Z08B-10
	ROM_LOAD( "rgum.u6", 0x0000, 0x2000, CRC(15a34117) SHA1(c7e0aef4007abfaaa533feb026148ba03230b79f) ) // near the data ROM, mostly empty

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "rgum.u16", 0x00000, 0x8000, CRC(2a2c8d78) SHA1(2ce335b900dccbc34ad8ae7ae02ec7c75ffcd559) ) // first half empty
	ROM_CONTINUE(0x00000,0x8000)
	ROM_LOAD( "rgum.u17", 0x08000,  0x8000, CRC(fae4e41a) SHA1(421aac2b567040c3a56e01aa70880c94450eaf76) ) // first half empty
	ROM_CONTINUE(0x08000,0x8000)
	ROM_LOAD( "rgum.u18", 0x10000, 0x8000, CRC(79b17da7) SHA1(31e1845261b0152df56135c212e55c4048b7496f) ) // first half empty
	ROM_CONTINUE(0x10000,0x8000)

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "rgum.u47", 0x00000, 0x20000, CRC(fe410eb9) SHA1(25180ba336269279f251be5483c210a581d27197) ) // 2nd half empty, almost identical to the one of ladygum in videosaa.cpp

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "n82s147n", 0x000, 0x200, NO_DUMP )

	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "gal16v8b-25lp", 0x000, 0x117, NO_DUMP )
ROM_END

} // Anonymous namespace


GAME( 1993, rgum, 0, rgum, rgum, rgum_state, empty_init, ROT0, "<unknown>", "Royal Gum (Italy)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
