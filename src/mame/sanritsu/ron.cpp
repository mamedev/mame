// license:BSD-3-Clause
// copyright-holders:Angelo Salese, AJR
/***************************************************************************

    Futari Mahjong Ron II (c) 1981 Sanritsu

    TODO:
    - colors;
    - dip switches;
    - sound, especially f/f part;

============================================================================
Debug cheats:
0x8580-d player-1 tiles
0x8680-d player-2 tiles

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "debugger.h"


namespace {

// TBD
#define MAIN_CLOCK (XTAL(15'468'480) / 4)
#define VIDEO_CLOCK (XTAL(15'468'480) / 3)
#define SOUND_CLOCK XTAL(3'579'545)

class ron_state : public driver_device
{
public:
	ron_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ay(*this, "aysnd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_mj_ports1(*this, "PL1_%u", 1U),
		m_mj_ports2(*this, "PL2_%u", 1U),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_in3(*this, "IN3")
	{
	}

	void ron(machine_config &config);

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void ron_palette(palette_device &palette) const;
	void vblank_irq(int state);

	void output_w(uint8_t data);
	uint8_t p1_mux_r(offs_t offset);
	uint8_t p2_mux_r(offs_t offset);
	void mux_w(uint8_t data);
	void sound_cmd_w(uint8_t data);
	uint8_t audio_cmd_r();
	void audio_p1_w(uint8_t data);
	void audio_p2_w(uint8_t data);
	int audio_t1_r();
	void ay_pa_w(uint8_t data);

	void ron_audio_io(address_map &map) ATTR_COLD;
	void ron_audio_map(address_map &map) ATTR_COLD;
	void ron_io(address_map &map) ATTR_COLD;
	void ron_map(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<i8035_device> m_audiocpu;
	required_device<ay8910_device> m_ay;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_cram;
	required_ioport_array<8> m_mj_ports1;
	required_ioport_array<8> m_mj_ports2;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_in2;
	required_ioport m_in3;

	bool m_nmi_enable;
	uint8_t m_mux_data;
	uint8_t read_mux(bool which,bool side);
	uint8_t m_prev_p2;
	uint8_t m_sound_command;
	bool m_ay_address_sel;
};

void ron_state::video_start()
{
}

uint32_t ron_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int y,x;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			uint16_t tile = m_cram[count];

			gfx->opaque(bitmap,cliprect,tile,0,0,0,x*8,y*8);

			count++;
		}
	}

	gfx = m_gfxdecode->gfx(1);
	count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			uint16_t tile = m_vram[count];

			gfx->transpen(bitmap,cliprect,tile,0,0,0,x*8,y*8,0);

			count++;
		}
	}

	return 0;
}

void ron_state::output_w(uint8_t data)
{
	m_nmi_enable = (data & 0x10) == 0x10;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	if(data & 0xef)
		printf("%02x\n",data);
}

uint8_t ron_state::read_mux(bool which,bool side)
{
	uint8_t base_port = which == true ? 4 : 0;

	//uint8_t i,res;

	//  printf("%02x\n", m_mux_data);
	//res = 0;//ioport("COINS")->read();

	for(uint8_t i=0;i<4;i++)
	{
		if((~m_mux_data) & (1 << i))
			return (side == true ? m_mj_ports2[i+base_port] : m_mj_ports1[i+base_port])->read();
	}

	// TODO: check me
	return machine().rand();
}

uint8_t ron_state::p1_mux_r(offs_t offset)
{
	uint8_t res = offset == 0 ? (m_in0->read() & 0xcc) : (m_in1->read() & 0xfc);

	return read_mux(offset,false) | res;
}

uint8_t ron_state::p2_mux_r(offs_t offset)
{
	uint8_t res = (offset == 0 ? m_in2 : m_in3)->read();

	res &= 0xec;

	return (read_mux(offset,true) & 0x13) | res;
}


void ron_state::mux_w(uint8_t data)
{
	m_mux_data = data;
}

void ron_state::sound_cmd_w(uint8_t data)
{
	m_sound_command = data;
	m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
}

void ron_state::ron_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0x8000, 0x83ff).ram().share("vram");
	map(0x8400, 0x87ff).ram();
	map(0x8800, 0x8bff).ram().share("cram");
	map(0x8c00, 0x8fff).ram();
}

void ron_state::ron_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x01).r(FUNC(ron_state::p1_mux_r));
	map(0x02, 0x03).r(FUNC(ron_state::p2_mux_r));
	map(0x03, 0x03).w(FUNC(ron_state::mux_w));
	map(0x07, 0x07).w(FUNC(ron_state::sound_cmd_w));
	map(0x0a, 0x0a).w(FUNC(ron_state::output_w));
}

void ron_state::ron_audio_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void ron_state::ron_audio_io(address_map &map)
{

}

static INPUT_PORTS_START( ron )
	PORT_START("IN0")
	PORT_DIPNAME( 0x04, 0x04, "2-Players Mode" )
	PORT_DIPSETTING(    0x04, "Versus" )
	PORT_DIPSETTING(    0x00, "Alternates" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x04, 0x04, "IN1" )
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


	PORT_START("IN2")
	PORT_DIPNAME( 0x04, 0x04, "2P Coinage" ) // how many credits per 2p mode
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
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

	PORT_START("IN3")
	PORT_DIPNAME( 0x04, 0x04, "IN3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(1)

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(1)

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(1)

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(1)

	PORT_START("PL1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("PL1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("PL1_7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PL1_8")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)

	PORT_START("PL2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)

	PORT_START("PL2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)

	PORT_START("PL2_7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)

	PORT_START("PL2_8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x12, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout charlayout_2bpp =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2),RGN_FRAC(1,2) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_ron )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1,     0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout_2bpp,     4, 1 )
GFXDECODE_END


void ron_state::machine_start()
{
	save_item(NAME(m_sound_command));
}

void ron_state::machine_reset()
{
	m_sound_command = 0x80;
}


void ron_state::ron_palette(palette_device &palette) const
{
	// TODO: proms?
	palette.set_pen_color(0, rgb_t(pal1bit(0), pal1bit(0), pal1bit(0)));
	palette.set_pen_color(1, rgb_t(pal1bit(1), pal1bit(1), pal1bit(1)));
	palette.set_pen_color(2, rgb_t(pal1bit(0), pal1bit(0), pal1bit(0)));
	palette.set_pen_color(3, rgb_t(pal1bit(0), pal1bit(0), pal1bit(0)));

	palette.set_pen_color(4, rgb_t(pal1bit(0), pal1bit(0), pal1bit(0)));
	palette.set_pen_color(5, rgb_t(pal1bit(1), pal1bit(1), pal1bit(0)));
	palette.set_pen_color(6, rgb_t(pal1bit(0), pal1bit(0), pal1bit(1)));
	palette.set_pen_color(7, rgb_t(pal1bit(1), pal1bit(1), pal1bit(1)));
}


void ron_state::vblank_irq(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t ron_state::audio_cmd_r()
{
	return m_sound_command << 3;
}

void ron_state::audio_p1_w(uint8_t data)
{
	//address_space &space = m_audiocpu->space(AS_PROGRAM);

	//printf("p1 %02x %d\n",data,m_ay_address_sel);
	//machine().debug_break();

	if(m_ay_address_sel == true)
		m_ay->address_w(data);
	else
		m_ay->data_w(data);
}

void ron_state::audio_p2_w(uint8_t data)
{
	// TODO: guesswork, presumably f/f based
	// p2 ff
	// p2 3f
	if(data == 0xff)
		m_ay_address_sel = false;

	// p2 5f
	// p2 3f
	// p2 3f
	if(data == 0x5f)
		m_ay_address_sel = true;

	m_prev_p2 = data;

	//printf("p2 %02x\n",data);
//  machine().debug_break();
}

int ron_state::audio_t1_r()
{
	// TODO: what controls this?
	return !BIT(m_sound_command, 6);
}

void ron_state::ay_pa_w(uint8_t data)
{
}

void ron_state::ron(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &ron_state::ron_map);
	m_maincpu->set_addrmap(AS_IO, &ron_state::ron_io);

	I8035(config, m_audiocpu, SOUND_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &ron_state::ron_audio_map);
	m_audiocpu->set_addrmap(AS_IO, &ron_state::ron_audio_io);
	m_audiocpu->set_t0_clk_cb("aysnd", FUNC(device_t::set_unscaled_clock_int));
	m_audiocpu->p2_in_cb().set(FUNC(ron_state::audio_cmd_r));
	m_audiocpu->p1_out_cb().set(FUNC(ron_state::audio_p1_w));
	m_audiocpu->p2_out_cb().set(FUNC(ron_state::audio_p2_w));
	m_audiocpu->t1_in_cb().set(FUNC(ron_state::audio_t1_r));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(ron_state::screen_update));
	screen.set_raw(VIDEO_CLOCK, 320, 0, 256, 264, 0, 240);
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(ron_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_ron);

	PALETTE(config, "palette", FUNC(ron_state::ron_palette), 8);
	//PALETTE(config, "palette", FUNC(ron_state::ron_palette), 512, 32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	AY8910(config, m_ay, 0); // T0 CLK from I8035 (not verified)
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.30);
	m_ay->port_a_write_callback().set(FUNC(ron_state::ay_pa_w));
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( ron2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "r0__1.9h",     0x000000, 0x001000, CRC(82a55ad4) SHA1(96ff9da1d2c57bc916f3341416ed4eb4144a16eb) )
	ROM_LOAD( "r0__2.9k",     0x001000, 0x001000, CRC(6b654f8b) SHA1(145e7ea8c65c72b3858d22d3ce4c1a75672c9ed4) )
	ROM_LOAD( "r0__3.9l",     0x002000, 0x001000, CRC(6a8c1ef0) SHA1(9300a54102ca096fbf3d5056ede782fba0f5f970) )
	ROM_LOAD( "r0__4.9n",     0x003000, 0x001000, CRC(86340522) SHA1(0175dda90e9e4798e9f2ab7a0ab97aa397ca18b8) )
	ROM_LOAD( "r0__5.8h",     0x004000, 0x001000, CRC(3a28ad40) SHA1(872ced2d7515850cd86b84c81b14f200093746ad) )

	ROM_REGION( 0x10000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "r0_mu.4a",     0x0000, 0x000800, CRC(3491d8d5) SHA1(0aa0581350f4b3b81f3fa1f7c55a9bfb1f2c5f3b) )
	ROM_LOAD( "r0_v0.4c",     0x0800, 0x000800, CRC(4160eb7f) SHA1(1756937378cbabb2229129b794d8c5d955252ed4) )

	ROM_REGION( 0x0800, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "r0__b.4k",     0x0000, 0x0800, CRC(8a61cdde) SHA1(0a38573ed644f1ed897443187f5cb61a6eb499b2) )

	ROM_REGION( 0x1000, "gfx2", ROMREGION_ERASE00 )
	ROM_LOAD( "r0_a1.4n",     0x0000, 0x0800, CRC(2d6276f4) SHA1(432b7fe0a1f1e9fdc9e276bcf27f74a5aec6c940) )
	ROM_LOAD( "r0_a2.4l",     0x0800, 0x0800, CRC(2fe4a54f) SHA1(bb1d109851677ede58f875eff2588f60f979864e) )

	ROM_REGION( 0x040, "unk_proms", ROMREGION_ERASE00 ) // unknown, near sound roms
	ROM_LOAD( "82s123_1.6b",  0x000, 0x020, CRC(bd9bb647) SHA1(aad83eb295107cdc7ee96d78e81b0621ac351398) )
	ROM_LOAD( "82s123_2.6c",  0x020, 0x020, CRC(439109d6) SHA1(f0d79048bb27a63c641296b3b1b81f513df9b33d) )

	ROM_REGION( 0x020, "color_prom", ROMREGION_ERASE00 )
	ROM_LOAD( "82s123_5.1n",  0x000, 0x020, CRC(869784fa) SHA1(4bd0f26961d0bb54edb5eab5708d34468721d4c4) )

	ROM_REGION( 0x200, "clut_proms", ROMREGION_ERASE00 )
	ROM_LOAD( "82s129_3.2n",  0x000, 0x100, CRC(018ab2a0) SHA1(039c574d8fd3c1a8e9eca6a7c79fe92e8496b157) )
	ROM_LOAD( "82s129_4.2m",  0x100, 0x100, CRC(f3c05d59) SHA1(bd48963aa9f2bedaa0c1fd031d7c93089161d1d9) )
ROM_END

} // anonymous namespace


GAME( 1981, ron2,  0,   ron,  ron, ron_state, empty_init, ROT270, "Sanritsu", "Futari Mahjong Ron II", MACHINE_IMPERFECT_SOUND | MACHINE_WRONG_COLORS )
