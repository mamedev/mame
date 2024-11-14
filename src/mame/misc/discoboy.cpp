// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
       Disco Boy

Similar to mitchell.cpp / egghunt.cpp .. clearly derived from that hardware

TODO:
- move sound HW into proper file (it's 99% IDENTICAL to yunsung8.cpp)
- ADPCM has sound volume issues, it's either too loud or too quiet;

PCB Layout
----------

SOFT ART CO. 1993 NO - 1021
PROGRAM NO-93-01-14-0024
+------------------------------------------+
| YM3014 YM3812 10MHz            5.u94     |
|       6116                    6.u124     |
|400kHz 1.u45                    7.u95     |
| M5205 2.u28                   8.u125     |
|J     Z8400B                          6116|
|A          6116          6116 TPC1020AFN  |
|M          6116                       6116|
|M    DSW2                                 |
|A DSW1                          6264      |
|      6264                       u80  u81 |
|12MHz  u2                       u50  u5   |
|      u18                        u78  u79 |
|    Z0840004PSC                 u46  u49  |
+------------------------------------------+

Notes:
  Zilog Z0840004PSC (Z80 CPU, main program CPU)
  Goldstar Z8400B PS (Z80 CPU, sound CPU)
  Yamaha YM3014/YM3812 (rebadged as 83142/5A12)
  OKI M5205
  TI TPC1020AFN-084C
  10.000MHz & 12.000MHz OSCs, 400KHz resonator

*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


// configurable logging
#define LOG_UNKWRITE     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_UNKWRITE)

#include "logmacro.h"

#define LOGUNKWRITE(...)     LOGMASKED(LOG_UNKWRITE,     __VA_ARGS__)


namespace {

class discoboy_state : public driver_device
{
public:
	discoboy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_adpcm_select(*this, "adpcm_select"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ram_att(*this, "att_ram"),
		m_palram(*this, "palram_%u", 1U, 0x800U, ENDIANNESS_LITTLE),
		m_spriteram(*this, "spriteram", 0x1000U, ENDIANNESS_LITTLE),
		m_tileram(*this, "tileram", 0x1000U, ENDIANNESS_LITTLE),
		m_main_rombank(*this, "main_rombank"),
		m_soundbank(*this, "soundbank"),
		m_palrambank(*this, "palrambank"),
		m_spritetilebank(*this, "spritetilebank") { }

	void discoboy(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// video-related
	uint8_t m_gfxbank = 0;
	uint8_t m_port_00 = 0;

	bool m_toggle = false;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<ls157_device> m_adpcm_select;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// memory
	required_shared_ptr<uint8_t> m_ram_att;
	memory_share_array_creator<uint8_t, 2> m_palram;
	memory_share_creator<uint8_t> m_spriteram;
	memory_share_creator<uint8_t> m_tileram;
	required_memory_bank m_main_rombank;
	required_memory_bank m_soundbank;
	required_memory_bank m_palrambank;
	required_memory_bank m_spritetilebank;

	void palrambank_select_w(uint8_t data);
	void port_00_w(uint8_t data);
	void port_01_w(uint8_t data);
	void port_06_w(uint8_t data);
	uint8_t port_06_r();
	void sound_bankswitch_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void adpcm_int(int state);
	void main_prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sound_prg_map(address_map &map) ATTR_COLD;
};

void discoboy_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int flipscreen = 0;

	for (int offs = 0x1000 - 0x40; offs >= 0; offs -= 0x20)
	{
		int code = m_spriteram[offs];
		int attr = m_spriteram[offs + 1];
		int color = attr & 0x0f;
		int sx = m_spriteram[offs + 3] + ((attr & 0x10) << 4);
		int sy = ((m_spriteram[offs + 2] + 8) & 0xff) - 8;
		code += (attr & 0xe0) << 3;

		if (code >= 0x400)
		{
			switch (m_gfxbank & 0x30)
			{
				case 0x00: code = 0x400 + (code & 0x3ff); break;
				case 0x10: code = 0x400 + (code & 0x3ff) + 0x400; break;
				case 0x20: code = 0x400 + (code & 0x3ff) + 0x800; break;
				case 0x30: code = 0x400 + (code & 0x3ff) + 0xc00; break;
				default: code = machine().rand();
			}
		}

		m_gfxdecode->gfx(0)->transpen(bitmap, cliprect,
					code,
					color,
					flipscreen, 0,
					sx, sy, 15);
	}
}


uint32_t discoboy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < 0x800; i += 2)
		{
			uint16_t pal = m_palram[j][i] | (m_palram[j][i + 1] << 8);

			int b = ((pal >> 0) & 0xf) << 4;
			int g = ((pal >> 4) & 0xf) << 4;
			int r = ((pal >> 8) & 0xf) << 4;

			m_palette->set_pen_color((i / 2) + j * 0x400, rgb_t(r, g, b));
		}
	}

	bitmap.fill(0x3ff, cliprect);

	for (uint8_t y = 0; y < 32; y++)
	{
		for (uint8_t x = 0; x < 64; x++)
		{
			uint16_t tileno = m_tileram[count] | (m_tileram[count + 1] << 8);

			if (tileno > 0x2000)
			{
				tileno = (tileno & 0x1fff) + ((m_gfxbank & 0xc0) << 7);

				m_gfxdecode->gfx(2)->opaque(bitmap, cliprect, tileno, m_ram_att[count / 2], 0, 0, x * 8, y * 8);
			}
			else
			{
				m_gfxdecode->gfx(1)->opaque(bitmap, cliprect, tileno, m_ram_att[count / 2], 0, 0, x * 8, y * 8);
			}

			count += 2;
		}
	}

	draw_sprites(bitmap, cliprect);

	return 0;
}

void discoboy_state::palrambank_select_w(uint8_t data)
{
	m_palrambank->set_entry(BIT(data, 5));

	if (data &= 0x83) LOGUNKWRITE("palrambank_select_w !!!!! %02x\n", data);
}

void discoboy_state::port_00_w(uint8_t data)
{
	if (data & 0xfe) LOGUNKWRITE("unk port_00_w %02x\n", data);
	m_spritetilebank->set_entry(BIT(data, 0));
}

void discoboy_state::port_01_w(uint8_t data)
{
	// 00 10 20 30 during gameplay  1,2,3 other times?? title screen bit 0x40 toggle
	LOGUNKWRITE("unk port_01_w %02x\n", data);
	// discoboy gfxbank
	m_gfxbank = data & 0xf0;

	m_main_rombank->set_entry(data & 0x07);
}

void discoboy_state::port_06_w(uint8_t data)
{
	if (data != 0) LOGUNKWRITE("port 06!!!! %02x\n",data);
}

void discoboy_state::main_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_main_rombank);
	map(0xc000, 0xc7ff).bankrw(m_palrambank);
	map(0xc800, 0xcfff).ram().share(m_ram_att);
	map(0xd000, 0xdfff).bankrw(m_spritetilebank);
	map(0xe000, 0xefff).ram();
	map(0xf000, 0xffff).ram();
}

uint8_t discoboy_state::port_06_r()
{
	return 0x00;
}

void discoboy_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSWA").w(FUNC(discoboy_state::port_00_w));
	map(0x01, 0x01).portr("SYSTEM").w(FUNC(discoboy_state::port_01_w));
	map(0x02, 0x02).portr("P1");
	map(0x03, 0x03).portr("P2").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x04, 0x04).portr("DSWB");
	map(0x06, 0x06).rw(FUNC(discoboy_state::port_06_r), FUNC(discoboy_state::port_06_w)); // ???
	map(0x07, 0x07).w(FUNC(discoboy_state::palrambank_select_w)); // 0x20 is palette bank bit.. others?
}

// Sound

void discoboy_state::sound_bankswitch_w(uint8_t data)
{
	// Note: this is bit 5 on yunsung8.cpp
	m_msm->reset_w((data & 0x08) >> 3);

	m_soundbank->set_entry(data & 0x07);

	if (data != (data & (~0x0f)))
		LOGUNKWRITE("%s: Bank %02X\n", machine().describe_context(), data);
}

void discoboy_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_soundbank);
	map(0xe000, 0xe000).w(FUNC(discoboy_state::sound_bankswitch_w));
	map(0xe400, 0xe400).w(m_adpcm_select, FUNC(ls157_device::ba_w));
	map(0xec00, 0xec01).w("ymsnd", FUNC(ym3812_device::write));
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf800).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( discoboy )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x08, "Every 150000" )
	PORT_DIPSETTING(    0x00, "Every 300000" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWA:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hardest ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSWB")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SWB:8" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SWB:2" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4,RGN_FRAC(1,2),  RGN_FRAC(0,2)+4,RGN_FRAC(0,2) },
	{ 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264,265,266,267  },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*32
};

static const gfx_layout tiles8x8_layout2 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(1,4), RGN_FRAC(0,4),RGN_FRAC(3,4),RGN_FRAC(2,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_discoboy )
	GFXDECODE_ENTRY( "sprites", 0, tiles8x8_layout, 0x000, 128 )
	GFXDECODE_ENTRY( "bgtiles", 0, tiles8x8_layout2, 0x000, 128 )
	GFXDECODE_ENTRY( "bgtiles_bank", 0, tiles8x8_layout2, 0x000, 128 )
GFXDECODE_END

void discoboy_state::machine_start()
{
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *audio = memregion("audiocpu")->base();

	m_main_rombank->configure_entries(0, 8, &rom[0x10000], 0x4000);
	m_main_rombank->set_entry(0);
	m_soundbank->configure_entries(0, 8, &audio[0x00000], 0x4000);
	m_soundbank->set_entry(0);

	m_palrambank->configure_entry(0, m_palram[0]);
	m_palrambank->configure_entry(1, m_palram[1]);

	m_spritetilebank->configure_entry(0, m_tileram);
	m_spritetilebank->configure_entry(1, m_spriteram);

	save_item(NAME(m_port_00));
	save_item(NAME(m_gfxbank));
	save_item(NAME(m_toggle));
}

void discoboy_state::machine_reset()
{
	m_port_00 = 0;
	m_gfxbank = 0;
	m_toggle = false;
}

void discoboy_state::adpcm_int(int state)
{
	if (!state)
		return;

	m_toggle = !m_toggle;
	m_adpcm_select->select_w(m_toggle);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, m_toggle);
}

void discoboy_state::discoboy(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000) / 2);     // 6 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &discoboy_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &discoboy_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(discoboy_state::irq0_line_hold));

	Z80(config, m_audiocpu, XTAL(10'000'000) / 2); // 5 MHz?
	m_audiocpu->set_addrmap(AS_PROGRAM, &discoboy_state::sound_prg_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea(8*8, 512-1-8*8, 0+8, 256-1-8);
	screen.set_screen_update(FUNC(discoboy_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_discoboy);
	PALETTE(config, m_palette).set_entries(0x1000);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, "soundlatch").data_pending_callback().set_inputline(m_audiocpu, 0);

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(10'000'000) / 4));   // 2.5 MHz?
	ymsnd.add_route(ALL_OUTPUTS, "lspeaker", 0.6);
	ymsnd.add_route(ALL_OUTPUTS, "rspeaker", 0.6);

	LS157(config, m_adpcm_select, 0);
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));

	MSM5205(config, m_msm, XTAL(400'000));
	m_msm->vck_legacy_callback().set(FUNC(discoboy_state::adpcm_int)); // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);      // 4KHz, 4 Bits
	m_msm->add_route(ALL_OUTPUTS, "lspeaker", 0.80);
	m_msm->add_route(ALL_OUTPUTS, "rspeaker", 0.80);
}


ROM_START( discoboy )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "u2",  0x00000, 0x10000, CRC(44a4fefa) SHA1(29b74bb739afffb7baefb5ed4da09cdb1559b011) )
	ROM_LOAD( "u18", 0x10000, 0x20000, CRC(88d1282d) SHA1(1f11dad0f577198c54a1dc182ba7502e398b998f) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "2.u28",  0x00000, 0x10000, CRC(7c2ed174) SHA1(ace209dc4cc7a4ffca062842defd84cefc5b10d2) )
	ROM_LOAD( "1.u45",  0x10000, 0x10000, CRC(c266c6df) SHA1(f76e38ded43f56a486cf6569c679ddb57a4165fb) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "5.u94",   0x00000, 0x10000, CRC(dbd20836) SHA1(d97651626b1dc16b93f8aed28bac19fd177e626f) )
	ROM_LOAD( "6.u124",  0x10000, 0x40000, CRC(e20d41f8) SHA1(792294a34840867072bc484d6f3cae3502c8bc28) )
	ROM_LOAD( "7.u95",   0x80000, 0x10000, CRC(1d5617a2) SHA1(6b6bd50c1984748dc8bf6600431d9bb6fe443873) )
	ROM_LOAD( "8.u125",  0x90000, 0x40000, CRC(30be1340) SHA1(e4765b75c8f774c6f7f7b5496a50c33ee3950550) )

	ROM_REGION( 0x40000, "bgtiles", ROMREGION_INVERT )
	ROM_LOAD( "u80",   0x00000, 0x10000, CRC(4cc642ae) SHA1(2a59ebc8ab27bf7c3c1aa389ea32fb01d5cfdce8) )
	ROM_LOAD( "u81",   0x10000, 0x10000, CRC(9e04274e) SHA1(70c28212b242335353e6dd48b7eb176146bec457) )
	ROM_LOAD( "u78",   0x20000, 0x10000, CRC(04571f70) SHA1(afdc7d84f7804c2ced413d13e6985a05f841e79e) )
	ROM_LOAD( "u79",   0x30000, 0x10000, CRC(646f0f83) SHA1(d5cd050872d4b8c2fc89c3c0f434b1d66e5f1c59) )

	ROM_REGION( 0x80000, "bgtiles_bank", ROMREGION_INVERT )
	ROM_LOAD( "u50",   0x00000, 0x20000, CRC(1557ca92) SHA1(5a0afbeede6f0ae1c75bdec446132c673aeb0fe7) )
	ROM_LOAD( "u5",    0x20000, 0x20000, CRC(a07df669) SHA1(7f09b2508b9bffed7a4cd191f707af3c0c2a1de2) )
	ROM_LOAD( "u46",   0x40000, 0x20000, CRC(764ffde4) SHA1(637df403a6ac73456892add3f2403a92afb67f19) )
	ROM_LOAD( "u49",   0x60000, 0x20000, CRC(0b6c0d8d) SHA1(820a12c84af4fd5a04e1eca3cbace0002d3024b6) )
ROM_END


ROM_START( discoboyp ) // all ROMs had PROMAT stickers but copyright in the game shows no indication of it
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "discob.u2",  0x00000, 0x10000, CRC(7f07afd1) SHA1(f81d38a45764289a78e9727934ccbda25933624c) )
	ROM_LOAD( "discob.u18", 0x10000, 0x20000, CRC(05f0daaf) SHA1(8691e0afff069a589a4601fe08f96f93c3773c7d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "discob.u28",  0x00000, 0x10000, CRC(7c2ed174) SHA1(ace209dc4cc7a4ffca062842defd84cefc5b10d2) )
	ROM_LOAD( "discob.u45",  0x10000, 0x10000, CRC(c266c6df) SHA1(f76e38ded43f56a486cf6569c679ddb57a4165fb) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "discob.u94",   0x00000, 0x10000, CRC(c436f1e5) SHA1(511e23b85f1b4fc732bd9648c0582848c20e6378) )
	ROM_LOAD( "discob.u124",  0x10000, 0x40000, CRC(0b0bf653) SHA1(ce609e9ee270eda6e74612ed5334fd6c3c81ef18) )
	ROM_LOAD( "discob.u95",   0x80000, 0x10000, CRC(ddea540e) SHA1(b69b94409bb15174f7780c637b183a2563c3d6c3) )
	ROM_LOAD( "discob.u125",  0x90000, 0x40000, CRC(fcac2cb8) SHA1(cb629b28acbb3ab42572b52ee85bf18a556b8e24) )

	ROM_REGION( 0x40000, "bgtiles", ROMREGION_INVERT )
	ROM_LOAD( "discob.u80",   0x000000, 0x10000, CRC(48a7ebdf) SHA1(942ddfdebb53f3f7f9256a4c8e8aa24887310775) )
	ROM_LOAD( "discob.u81",   0x010000, 0x10000, CRC(9ca358e1) SHA1(c711af363b466e40e514a7cec7a5098f178c3d9c) )
	ROM_LOAD( "discob.u78",   0x020000, 0x10000, CRC(2b39eb08) SHA1(c275041e54d53da58730e18f5b8f6443b4301cb2) )
	ROM_LOAD( "discob.u79",   0x030000, 0x10000, CRC(77ffc6bf) SHA1(15fd3b1d2b435d8ff7da5deaddbed7f3f9ccc609) )

	ROM_REGION( 0x100000, "bgtiles_bank", ROMREGION_INVERT )
	ROM_LOAD( "discob.u50",   0x000000, 0x40000, CRC(ea7231db) SHA1(8f42e39aeda97351a5ca5e926df544bc019f178f) )
	ROM_LOAD( "discob.u5",    0x040000, 0x40000, CRC(afeefecc) SHA1(9f85e0e5955876b85d91341a7fc355674c30a0bb) )
	ROM_LOAD( "discob.u46",   0x080000, 0x40000, CRC(835c513b) SHA1(eab400cb9af556b2762c4bbae55a2c04a1a7d8ba) )
	ROM_LOAD( "discob.u49",   0x0c0000, 0x40000, CRC(9f884db4) SHA1(fd916b0ac54961bbd9b3f23d3ee5d35d747cbf17) )
ROM_END

ROM_START( djgirl ) // all ROMs had PROMAT stickers but the game displays no copyright string. DJ Girl only used on title screen, still Disco Boy after coin-up and in-game.
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "djgirl.u2",  0x00000, 0x10000, CRC(95912ee3) SHA1(0e3dba93de550f2af7978af324b0ab514e44ebd1) )
	ROM_LOAD( "djgirl.u18", 0x10000, 0x20000, CRC(05f0daaf) SHA1(8691e0afff069a589a4601fe08f96f93c3773c7d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "djgirl.u28",  0x00000, 0x10000, CRC(7c2ed174) SHA1(ace209dc4cc7a4ffca062842defd84cefc5b10d2) )
	ROM_LOAD( "djgirl.u45",  0x10000, 0x10000, CRC(c266c6df) SHA1(f76e38ded43f56a486cf6569c679ddb57a4165fb) )

	ROM_REGION( 0x100000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "djgirl.u94",   0x00000, 0x10000, CRC(c436f1e5) SHA1(511e23b85f1b4fc732bd9648c0582848c20e6378) )
	ROM_LOAD( "djgirl.u124",  0x10000, 0x40000, CRC(0b0bf653) SHA1(ce609e9ee270eda6e74612ed5334fd6c3c81ef18) )
	ROM_LOAD( "djgirl.u95",   0x80000, 0x10000, CRC(ddea540e) SHA1(b69b94409bb15174f7780c637b183a2563c3d6c3) )
	ROM_LOAD( "djgirl.u125",  0x90000, 0x40000, CRC(fcac2cb8) SHA1(cb629b28acbb3ab42572b52ee85bf18a556b8e24) )

	ROM_REGION( 0x40000, "bgtiles", ROMREGION_INVERT )
	ROM_LOAD( "djgirl.u80",   0x000000, 0x10000, CRC(74f18c8b) SHA1(af6fb8927a8b8b85b65259bcd0d6f29e3c72b66b) )
	ROM_LOAD( "djgirl.u81",   0x010000, 0x10000, CRC(077ccd1c) SHA1(43fc03bf88f35804069be843f6df9be74ea905c7) )
	ROM_LOAD( "djgirl.u78",   0x020000, 0x10000, CRC(746bcf74) SHA1(597a8ce18c2a9dd04d9d3999fe0721d491726c38) )
	ROM_LOAD( "djgirl.u79",   0x030000, 0x10000, CRC(a17e04f8) SHA1(94bb0f958c83f8287b9a83858c153f009db109ef) )

	ROM_REGION( 0x100000, "bgtiles_bank", ROMREGION_INVERT )
	ROM_LOAD( "djgirl.u50",   0x000000, 0x40000, CRC(02a52a90) SHA1(b1c6f9b4f02f498109ea57f5bc37277f6cc4c11e) )
	ROM_LOAD( "djgirl.u5",    0x040000, 0x40000, CRC(39abacac) SHA1(3558334abb2b0d6cbccc2a07125c13e978ef9d26) )
	ROM_LOAD( "djgirl.u46",   0x080000, 0x40000, CRC(3853d794) SHA1(4746c660c57d44dc4fb2793c2660c30d3185efc4) )
	ROM_LOAD( "djgirl.u49",   0x0c0000, 0x40000, CRC(b3f580c5) SHA1(807380bed3773495de4b7c4b26a8aa97d4e83d64) )
ROM_END

} // anonymous namespace


GAME( 1993, discoboy,  0,        discoboy, discoboy, discoboy_state, empty_init, ROT270, "Soft Art Co.",          "Disco Boy",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, discoboyp, discoboy, discoboy, discoboy, discoboy_state, empty_init, ROT270, "Soft Art Co. / Promat", "Disco Boy (Promat license)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1993, djgirl,    discoboy, discoboy, discoboy, discoboy_state, empty_init, ROT270, "Soft Art Co. / Promat", "DJ Girl (Promat license)",    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
