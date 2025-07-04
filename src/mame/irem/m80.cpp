// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Tamtex Shisensho / Match It

driver by Nicola Salmoria


IREM M80-A-A PCB
+-----------------------------------------------------+
|  VR1 MB3730A  D780C-1    PAL16L8.IC37               |
|          ML324N          24.000MHz                  |
|              SIS_A-22-.IC22             C2H.IC50    |
|       Y3014B                KNA91H014   C2L.IC51    |
|J     YM2151  TMM2063AP-70               C0H.IC52*   |
|A                         PAL16L8.IC30   C0L.IC53    |
|M    SD.IC14                                         |
|M SW1                                                |
|A SW2         TMM2063AP-70                           |
|              SIS_A-27-I.IC27   TMM2063AP-70         |
|              D780C-1                                |
|                                                     |
|                3579.545KHz      PAL16R4.IC48        |
+-----------------------------------------------------+

  CPU: NEC D780C-1 Z80 x 2
Sound: Yamaha YM2151, Y3014B DAC
       Fujitsu MB3730A Low-Frequency High-Power AMP
       LM324N Low-Power Quad-Operational AMP
Video: Nanao KNA91H014 QFP60
  OSC: 24.000MHz, 3579.545KHz (AKA 3.579545MHz)
  RAM: TMM2063AP-70 64K SRAM x 3
Other: VR1 - Volume pot
       SW1 & SW2 - 8 switch dipswitch
       TIBPAL16L8-25CN @ IC37
       TIBPAL16L8-25CN @ IC30
       PAL16R4ACN @ IC48

EPROMs:
 IC27 - main program ROM, SIS A-27-I  27C101
 IC22 - sound program ROM, SIS A-22-  27C512
ROMs:
 IC50 - NEC D23C2001C 115 mask ROM silkscreened C2H
 IC51 - NEC D23C2001C 114 mask ROM silkscreened C2L
 IC52 - Unpopulated, silkscreened C0H - 2M ROM
 IC53 - Toshiba TC534000P N001 mask ROM silkscreened C0L
 IC14 - NEC D23C2001C 116 mask ROM silkscreened SD

***************************************************************************/

#include "emu.h"

#include "m72_a.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/rstbuf.h"
#include "machine/timer.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class shisen_state : public driver_device
{
public:
	shisen_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_paletteram(*this, "paletteram"),
		m_videoram(*this, "videoram"),
		m_mainbank(*this, "mainbank"),
		m_dsw2(*this, "DSW2")
	{ }

	void shisen(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_videoram;
	required_memory_bank m_mainbank;

	required_ioport m_dsw2;

	uint8_t m_gfxbank = 0;
	tilemap_t *m_bg_tilemap = nullptr;

	void coin_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void bankswitch_w(uint8_t data);
	void paletteram_w(offs_t offset, uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(sound_nmi);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_prg_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_prg_map(address_map &map) ATTR_COLD;
};


void shisen_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

void shisen_state::bankswitch_w(uint8_t data)
{
	if (data & 0xc0) logerror("bank switch %02x\n", data);

	// bits 0-2 select ROM bank
	m_mainbank->set_entry(data & 0x07);

	// bits 3-5 select gfx bank
	int bank = (data & 0x38) >> 3;

	if (m_gfxbank != bank)
	{
		m_gfxbank = bank;
		machine().tilemap().mark_all_dirty();
	}

	// bits 6-7 unknown
}

void shisen_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_paletteram[offset] = data;

	offset &= 0xff;

	m_palette->set_pen_color(offset, pal5bit(m_paletteram[offset + 0x000]), pal5bit(m_paletteram[offset + 0x100]), pal5bit(m_paletteram[offset + 0x200]));
}

TILE_GET_INFO_MEMBER(shisen_state::get_bg_tile_info)
{
	int offs = tile_index * 2;
	int code = m_videoram[offs] + ((m_videoram[offs + 1] & 0x0f) << 8) + (m_gfxbank << 12);
	int color = (m_videoram[offs + 1] & 0xf0) >> 4;

	tileinfo.set(0, code, color, 0);
}

void shisen_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(shisen_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_mainbank->configure_entries(0, 8, memregion("maincpu")->base(), 0x4000);

	save_item(NAME(m_gfxbank));
}

TIMER_DEVICE_CALLBACK_MEMBER(shisen_state::sound_nmi)
{
	m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

uint32_t shisen_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// on Irem boards, screen flip is handled in both hardware and software.
	// this game doesn't have cocktail mode so if there's software control we don't know where it is mapped.
	flip_screen_set(~m_dsw2->read() & 1);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void shisen_state::main_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_mainbank);
	map(0xc800, 0xcaff).ram().w(FUNC(shisen_state::paletteram_w)).share(m_paletteram);
	map(0xd000, 0xdfff).ram().w(FUNC(shisen_state::videoram_w)).share(m_videoram);
	map(0xe000, 0xffff).ram();
}


void shisen_state::coin_w(uint8_t data)
{
	if ((data & 0xf9) != 0x01) logerror("coin ctrl = %02x\n",data);

	machine().bookkeeping().coin_counter_w(0, data & 0x02);
	machine().bookkeeping().coin_counter_w(1, data & 0x04);
}

void shisen_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1").w(FUNC(shisen_state::coin_w));
	map(0x01, 0x01).portr("DSW2").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x02, 0x02).portr("P1").w(FUNC(shisen_state::bankswitch_w));
	map(0x03, 0x03).portr("P2");
	map(0x04, 0x04).portr("COIN");
}


void shisen_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0xfd00, 0xffff).ram();
}

void shisen_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x80, 0x80).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).w(m_audio, FUNC(m72_audio_device::shisen_sample_addr_w));
	map(0x82, 0x82).w(m_audio, FUNC(m72_audio_device::sample_w));
	map(0x83, 0x83).w("soundlatch", FUNC(generic_latch_8_device::acknowledge_w));
	map(0x84, 0x84).r(m_audio, FUNC(m72_audio_device::sample_r));
}


static INPUT_PORTS_START( shisen )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Timer" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coinage ) ) PORT_CONDITION("DSW2",0x04,EQUALS,0x04) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0xa0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) ) PORT_CONDITION("DSW2",0x04,NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) ) PORT_CONDITION("DSW2",0x04,NOTEQUALS,0x04) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x08, 0x08, "Nude Pictures" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Women Select" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	// In stop mode, press 2 to stop and 1 to restart
	PORT_DIPNAME( 0x20, 0x20, "Stop Mode (Cheat)") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Play Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "1 Player" )
	PORT_DIPSETTING(    0x40, "2 Player" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( matchit )
	PORT_INCLUDE( shisen )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	4096*8,
	4,
	{ 0, 4, 0x80000*8+0, 0x80000*8+4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};


static GFXDECODE_START( gfx_shisen )
	GFXDECODE_ENTRY( "tiles", 0x00000, charlayout,  0, 16 )
GFXDECODE_END



void shisen_state::shisen(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 3.579545_MHz_XTAL ); // Verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &shisen_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &shisen_state::main_io_map);

	z80_device &soundcpu(Z80(config, m_soundcpu, 3.579545_MHz_XTAL )); // Verified on PCB
	soundcpu.set_addrmap(AS_PROGRAM, &shisen_state::sound_prg_map);
	soundcpu.set_addrmap(AS_IO, &shisen_state::sound_io_map);
								// IRQs are generated by main Z80 and YM2151
	soundcpu.set_irq_acknowledge_callback("soundirq", FUNC(rst_neg_buffer_device::inta_cb));

	TIMER(config, "v1").configure_scanline(FUNC(shisen_state::sound_nmi), "screen", 1, 2);  // clocked by V1? (Vigilante)

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(24_MHz_XTAL / 2, 96*8, 0*8, 64*8, 284, 0, 256); // sync rates not verified
	screen.set_screen_update(FUNC(shisen_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_shisen);
	PALETTE(config, m_palette).set_entries(256);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	generic_latch_8_device &soundlatch(GENERIC_LATCH_8(config, "soundlatch"));
	soundlatch.data_pending_callback().set("soundirq", FUNC(rst_neg_buffer_device::rst18_w));
	soundlatch.set_separate_acknowledge(true);

	RST_NEG_BUFFER(config, "soundirq").int_callback().set_inputline(m_soundcpu, 0);

	IREM_M72_AUDIO(config, m_audio);
	m_audio->set_dac_tag("dac");

	ym2151_device &ymsnd(YM2151(config, "ymsnd", 3.579545_MHz_XTAL )); // Verified on PCB
	ymsnd.irq_handler().set("soundirq", FUNC(rst_neg_buffer_device::rst28_w));
	ymsnd.add_route(0, "speaker", 0.5, 0);
	ymsnd.add_route(1, "speaker", 0.5, 1);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 0).add_route(ALL_OUTPUTS, "speaker", 0.25, 1); // Y3014B DAC
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( matchit ) // IREM M80-A-A hardware
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sis_a-27-i.ic27", 0x00000, 0x20000, CRC(c7190125) SHA1(f4615a5dff0931d8f0e417e7dd6d0f87c7f573a4) ) // 27C101 EPROM

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sis_a-22-.ic22", 0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) ) // 27C512 EPROM

	ROM_REGION( 0x100000, "tiles", 0 ) // mask ROMs soldered in place
	ROM_LOAD( "c2l.ic51", 0x000000, 0x40000, CRC(e19276a9) SHA1(6350f61225e9b1bdb60554807553d08eb3b3d626) ) // NEC D23C2001C mask ROM
	ROM_LOAD( "c2h.ic50", 0x040000, 0x40000, CRC(e1dc099e) SHA1(3089b51150e13486eb4f233e598a7cc75d97375a) ) // NEC D23C2001C mask ROM
	ROM_LOAD( "c0l.ic53", 0x080000, 0x80000, CRC(10fde9fb) SHA1(6d69ddbe3e90679a9b6f70c030a7d7e7db8c03ca) ) // Toshiba TC534000P mask ROM

	ROM_REGION( 0x40000, "m72", 0 ) // samples -  mask ROM soldered in place
	ROM_LOAD( "sd.ic14", 0x00000, 0x40000, CRC(80448f72) SHA1(99cd7160e9bf1f6f99b2ed8fe3900c5ac24f3acc) ) // NEC D23C2001C mask ROM

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "tibpal16l8-25cn.ic30", 0x0000, 0x0104, CRC(7cf017b3) SHA1(5de606cc679d564edb796832646771ed2bae0b7f) ) // unprotected
	ROM_LOAD( "tibpal16l8-25cn.ic37", 0x0200, 0x0104, CRC(6121d077) SHA1(dfc449f4c8cb4ae8f916c4dd32d69a7d62d54b9a) ) // unprotected
	ROM_LOAD( "pal16r4acn.ic48",      0x0400, 0x0104, CRC(09c6c744) SHA1(24fc275cc49d4407a14941e036404c2108b52402) ) // unprotected
ROM_END

ROM_START( shisen ) // IREM M80-A-A hardware
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sis_a-27-d.ic27", 0x00000, 0x20000, CRC(feaf8b13) SHA1(7d98fdda42aa203073679e5e441e835350a2342b) ) // 27C101 EPROM

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sis_a-22-.ic22", 0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) ) // 27C512 EPROM

	ROM_REGION( 0x100000, "tiles", 0 ) // mask ROMs soldered in place
	ROM_LOAD( "c2l.ic51", 0x000000, 0x40000, CRC(e19276a9) SHA1(6350f61225e9b1bdb60554807553d08eb3b3d626) ) // NEC D23C2001C mask ROM
	ROM_LOAD( "c2h.ic50", 0x040000, 0x40000, CRC(e1dc099e) SHA1(3089b51150e13486eb4f233e598a7cc75d97375a) ) // NEC D23C2001C mask ROM
	ROM_LOAD( "c0l.ic53", 0x080000, 0x80000, CRC(10fde9fb) SHA1(6d69ddbe3e90679a9b6f70c030a7d7e7db8c03ca) ) // Toshiba TC534000P mask ROM

	ROM_REGION( 0x40000, "m72", 0 ) // samples -  mask ROM soldered in place
	ROM_LOAD( "sd.ic14", 0x00000, 0x40000, CRC(80448f72) SHA1(99cd7160e9bf1f6f99b2ed8fe3900c5ac24f3acc) ) // NEC D23C2001C mask ROM

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "tibpal16l8-25cn.ic30", 0x0000, 0x0104, CRC(7cf017b3) SHA1(5de606cc679d564edb796832646771ed2bae0b7f) ) // unprotected
	ROM_LOAD( "tibpal16l8-25cn.ic37", 0x0200, 0x0104, CRC(6121d077) SHA1(dfc449f4c8cb4ae8f916c4dd32d69a7d62d54b9a) ) // unprotected
	ROM_LOAD( "pal16r4acn.ic48",      0x0400, 0x0104, CRC(09c6c744) SHA1(24fc275cc49d4407a14941e036404c2108b52402) ) // unprotected
ROM_END

ROM_START( shisena ) // IREM M80-A-A hardware
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sis_a-27-a.ic27", 0x00000, 0x20000, CRC(de2ecf05) SHA1(7256c5587f92db10a52c43001e3236f3be3df5df) ) // 27C101 EPROM

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sis_a-22-.ic22", 0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) ) // 27C512 EPROM

	ROM_REGION( 0x100000, "tiles", 0 ) // mask ROMs soldered in place
	ROM_LOAD( "c2l.ic51", 0x000000, 0x40000, CRC(e19276a9) SHA1(6350f61225e9b1bdb60554807553d08eb3b3d626) ) // NEC D23C2001C mask ROM
	ROM_LOAD( "c2h.ic50", 0x040000, 0x40000, CRC(e1dc099e) SHA1(3089b51150e13486eb4f233e598a7cc75d97375a) ) // NEC D23C2001C mask ROM
	ROM_LOAD( "c0l.ic53", 0x080000, 0x80000, CRC(10fde9fb) SHA1(6d69ddbe3e90679a9b6f70c030a7d7e7db8c03ca) ) // Toshiba TC534000P mask ROM

	ROM_REGION( 0x40000, "m72", 0 ) // samples -  mask ROM soldered in place
	ROM_LOAD( "sd.ic14", 0x00000, 0x40000, CRC(80448f72) SHA1(99cd7160e9bf1f6f99b2ed8fe3900c5ac24f3acc) ) // NEC D23C2001C mask ROM

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "tibpal16l8-25cn.ic30", 0x0000, 0x0104, CRC(7cf017b3) SHA1(5de606cc679d564edb796832646771ed2bae0b7f) ) // unprotected
	ROM_LOAD( "tibpal16l8-25cn.ic37", 0x0200, 0x0104, CRC(6121d077) SHA1(dfc449f4c8cb4ae8f916c4dd32d69a7d62d54b9a) ) // unprotected
	ROM_LOAD( "pal16r4acn.ic48",      0x0400, 0x0104, CRC(09c6c744) SHA1(24fc275cc49d4407a14941e036404c2108b52402) ) // unprotected
ROM_END


/*
Match It (bootleg hardware)
Tamtex, 1989


CPU   : Z80A
SOUND : Z80A, YM2151, YM3012, LM324, GL4558
DIPSW : 8 position (x2)
XTAL  : 3.579545MHz, 24.000MHz
RAM   : GM76C88-12D (=6264, x3), 6116 (x3)
PROMs : None
PALs  : PAL16L8 (x3)

ROMs  : (All ROMs type 27C512)

1.2C     Sound Program

2.11D  \
3.11C  / Main Program

4.3J   \
5.4J   |
6.5J   |
7.6J   |
8.1L   |
9.2L   |
10.3L  |
11.5L  | GFX
12.6L  |
13.7L  |
14.8L  |
15.10L |
16.11L |
17.12L |
18.13L |
19.14L /
*/

ROM_START( matchitb ) // bootleg
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "2.11d",  0x00000, 0x10000, CRC(299815f7) SHA1(dd25f69d3c825e12e5c2e24b5bbfda9c39400345) ) // == 1/2 sis_a-27-i.ic27
	ROM_LOAD( "3.11c",  0x10000, 0x10000, CRC(0350f6e2) SHA1(c683571969c0e4c66eb316a1bc580759db02bbfc) ) // == 2/2 sis_a-27-i.ic27 / sis_a-27-d.ic27 / sis_a-27-a.ic27

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.2c",   0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) ) // == sis_a-22-.ic22

	ROM_REGION( 0x100000, "tiles", 0 ) // same data as original sets, just in 64K chunks
	ROM_LOAD( "4.3j",   0x00000, 0x10000, CRC(1c0e221c) SHA1(87561f7dabf25309be784e797ac237aa3956ea1c) )
	ROM_LOAD( "5.4j",   0x10000, 0x10000, CRC(8a7d8284) SHA1(56b5d352b506c5bfab24102b11c877dd28c8ad36) )
	ROM_LOAD( "8.1l",   0x20000, 0x10000, CRC(48e1d043) SHA1(4fbd409aff593c0b27fc58c218a470adf48ee0b7) )
	ROM_LOAD( "9.2l",   0x30000, 0x10000, CRC(3feff3f2) SHA1(2e87e4fb158379486de5d13feff5bf1965690e14) )
	ROM_LOAD( "10.3l",  0x40000, 0x10000, CRC(b76a517d) SHA1(6dcab31ecc127c2fdc6912802cddfa62161d83a2) )
	ROM_LOAD( "11.5l",  0x50000, 0x10000, CRC(8ff5ee7a) SHA1(c8f4d374a43fcc818378c4e73af2c03238a93ad0) )
	ROM_LOAD( "12.6l",  0x60000, 0x10000, CRC(64e5d837) SHA1(030f9704dfdb06cd3ac583b857ce9239e1409f60) )
	ROM_LOAD( "13.7l",  0x70000, 0x10000, CRC(02c1b2c4) SHA1(6e4fe801189766559859eb0d628f3ae65c05ad16) )
	ROM_LOAD( "14.8l",  0x80000, 0x10000, CRC(f5a8370e) SHA1(b270af116ee15b5de048fc218215bf91a2ce6b46) )
	ROM_LOAD( "15.10l", 0x90000, 0x10000, CRC(7a9b7671) SHA1(ddb9b412b494f2f259c56a163e5511353cf4ebb5) )
	ROM_LOAD( "16.11l", 0xa0000, 0x10000, CRC(7fb396ad) SHA1(b245bb9a6a4c1ec518906c59c3a3da5e412369ab) )
	ROM_LOAD( "17.12l", 0xb0000, 0x10000, CRC(fb83c652) SHA1(3d124e5c751732e8055bbb7b6853b6e64435a85d) )
	ROM_LOAD( "18.13l", 0xc0000, 0x10000, CRC(d8b689e9) SHA1(14db7ba2246f12db9b6b5b4dcb4a648e4a65ace4) )
	ROM_LOAD( "19.14l", 0xd0000, 0x10000, CRC(e6611947) SHA1(9267dad097b318174706d51fb94d613208b04f60) )
	ROM_LOAD( "6.5j",   0xe0000, 0x10000, CRC(473b349a) SHA1(9f5d08e07c8175bc7ec3854499177af2c398bd76) )
	ROM_LOAD( "7.6j",   0xf0000, 0x10000, CRC(d9a60285) SHA1(f8ef211e022e9c8ea25f6d8fb16266867656a591) )

	ROM_REGION( 0x40000, "m72", 0 ) // samples - same as original data, just in 64K chunks
	ROM_LOAD( "2.7b", 0x00000, 0x10000, CRC(92f0093d) SHA1(530b924aa991283045577d03524dfc7eacf1be49) )
	ROM_LOAD( "3.6c", 0x10000, 0x10000, CRC(116a049c) SHA1(656c0d1d7f945c5f5637892721a58421b682fd01) )
	ROM_LOAD( "4.7c", 0x20000, 0x10000, CRC(6840692b) SHA1(f6f7b063ecf7206e172843515be38376f8845b42) )
	ROM_LOAD( "5.9c", 0x30000, 0x10000, CRC(92ffe22a) SHA1(19dcaf6e25bb7498d4ab19fa0a63f3326b9bff8f) )
ROM_END

ROM_START( sichuan2 ) // bootleg / hack
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "6.11d", 0x00000, 0x10000, CRC(98a2459b) SHA1(42102cf2921f80be7600b11aba63538e3b3858ec) )
	ROM_LOAD( "7.11c", 0x10000, 0x10000, CRC(0350f6e2) SHA1(c683571969c0e4c66eb316a1bc580759db02bbfc) ) // == 2/2 sis_a-27-i.ic27 / sis_a-27-d.ic27 / sis_a-27-a.ic27

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.2c", 0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) ) // == sis_a-22-.ic22

	ROM_REGION( 0x100000, "tiles", 0 ) // same data as original sets, just in 64K chunks
	ROM_LOAD( "8.3j",   0x00000, 0x10000, CRC(1c0e221c) SHA1(87561f7dabf25309be784e797ac237aa3956ea1c) )
	ROM_LOAD( "9.4j",   0x10000, 0x10000, CRC(8a7d8284) SHA1(56b5d352b506c5bfab24102b11c877dd28c8ad36) )
	ROM_LOAD( "12.1l",  0x20000, 0x10000, CRC(48e1d043) SHA1(4fbd409aff593c0b27fc58c218a470adf48ee0b7) )
	ROM_LOAD( "13.2l",  0x30000, 0x10000, CRC(3feff3f2) SHA1(2e87e4fb158379486de5d13feff5bf1965690e14) )
	ROM_LOAD( "14.3l",  0x40000, 0x10000, CRC(b76a517d) SHA1(6dcab31ecc127c2fdc6912802cddfa62161d83a2) )
	ROM_LOAD( "15.5l",  0x50000, 0x10000, CRC(8ff5ee7a) SHA1(c8f4d374a43fcc818378c4e73af2c03238a93ad0) )
	ROM_LOAD( "16.6l",  0x60000, 0x10000, CRC(64e5d837) SHA1(030f9704dfdb06cd3ac583b857ce9239e1409f60) )
	ROM_LOAD( "17.7l",  0x70000, 0x10000, CRC(02c1b2c4) SHA1(6e4fe801189766559859eb0d628f3ae65c05ad16) )
	ROM_LOAD( "18.8l",  0x80000, 0x10000, CRC(f5a8370e) SHA1(b270af116ee15b5de048fc218215bf91a2ce6b46) )
	ROM_LOAD( "19.10l", 0x90000, 0x10000, CRC(7a9b7671) SHA1(ddb9b412b494f2f259c56a163e5511353cf4ebb5) )
	ROM_LOAD( "20.11l", 0xa0000, 0x10000, CRC(7fb396ad) SHA1(b245bb9a6a4c1ec518906c59c3a3da5e412369ab) )
	ROM_LOAD( "21.12l", 0xb0000, 0x10000, CRC(fb83c652) SHA1(3d124e5c751732e8055bbb7b6853b6e64435a85d) )
	ROM_LOAD( "22.13l", 0xc0000, 0x10000, CRC(d8b689e9) SHA1(14db7ba2246f12db9b6b5b4dcb4a648e4a65ace4) )
	ROM_LOAD( "23.14l", 0xd0000, 0x10000, CRC(e6611947) SHA1(9267dad097b318174706d51fb94d613208b04f60) )
	ROM_LOAD( "11.5j",  0xe0000, 0x10000, CRC(473b349a) SHA1(9f5d08e07c8175bc7ec3854499177af2c398bd76) )
	ROM_LOAD( "10.6j",  0xf0000, 0x10000, CRC(d9a60285) SHA1(f8ef211e022e9c8ea25f6d8fb16266867656a591) )

	ROM_REGION( 0x40000, "m72", 0 ) // samples - same as original data, just in 64K chunks
	ROM_LOAD( "2.7b", 0x00000, 0x10000, CRC(92f0093d) SHA1(530b924aa991283045577d03524dfc7eacf1be49) )
	ROM_LOAD( "3.6c", 0x10000, 0x10000, CRC(116a049c) SHA1(656c0d1d7f945c5f5637892721a58421b682fd01) )
	ROM_LOAD( "4.7c", 0x20000, 0x10000, CRC(6840692b) SHA1(f6f7b063ecf7206e172843515be38376f8845b42) )
	ROM_LOAD( "5.9c", 0x30000, 0x10000, CRC(92ffe22a) SHA1(19dcaf6e25bb7498d4ab19fa0a63f3326b9bff8f) )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "1.1f",  0x000, 0x104, NO_DUMP ) // TIBPAL16R4
	ROM_LOAD( "2.6e",  0x200, 0x104, NO_DUMP ) // TIBPAL16L8
	ROM_LOAD( "3.14f", 0x400, 0x104, NO_DUMP ) // TIBPAL16L8
ROM_END

ROM_START( sichuan2a ) // bootleg / hack
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic06.a6", 0x00000, 0x10000, CRC(f8ac05ef) SHA1(cd20e5239d73264f1323ba6b1e35934685852ba1) )
	ROM_LOAD( "ic07.03", 0x10000, 0x10000, CRC(0350f6e2) SHA1(c683571969c0e4c66eb316a1bc580759db02bbfc) ) // == 2/2 sis_a-27-i.ic27 / sis_a-27-d.ic27 / sis_a-27-a.ic27

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "ic01.01", 0x00000, 0x10000, CRC(51b0a26c) SHA1(af2482cfe8d395848c8e1bf07bf1049ffc6ee69b) ) // == sis_a-22-.ic22

	ROM_REGION( 0x100000, "tiles", 0 ) // same data as original sets, just in 64K chunks
	ROM_LOAD( "ic08.04", 0x00000, 0x10000, CRC(1c0e221c) SHA1(87561f7dabf25309be784e797ac237aa3956ea1c) )
	ROM_LOAD( "ic09.05", 0x10000, 0x10000, CRC(8a7d8284) SHA1(56b5d352b506c5bfab24102b11c877dd28c8ad36) )
	ROM_LOAD( "ic12.08", 0x20000, 0x10000, CRC(48e1d043) SHA1(4fbd409aff593c0b27fc58c218a470adf48ee0b7) )
	ROM_LOAD( "ic13.09", 0x30000, 0x10000, CRC(3feff3f2) SHA1(2e87e4fb158379486de5d13feff5bf1965690e14) )
	ROM_LOAD( "ic14.10", 0x40000, 0x10000, CRC(b76a517d) SHA1(6dcab31ecc127c2fdc6912802cddfa62161d83a2) )
	ROM_LOAD( "ic15.11", 0x50000, 0x10000, CRC(8ff5ee7a) SHA1(c8f4d374a43fcc818378c4e73af2c03238a93ad0) )
	ROM_LOAD( "ic16.12", 0x60000, 0x10000, CRC(64e5d837) SHA1(030f9704dfdb06cd3ac583b857ce9239e1409f60) )
	ROM_LOAD( "ic17.13", 0x70000, 0x10000, CRC(02c1b2c4) SHA1(6e4fe801189766559859eb0d628f3ae65c05ad16) )
	ROM_LOAD( "ic18.14", 0x80000, 0x10000, CRC(f5a8370e) SHA1(b270af116ee15b5de048fc218215bf91a2ce6b46) )
	ROM_LOAD( "ic19.15", 0x90000, 0x10000, CRC(7a9b7671) SHA1(ddb9b412b494f2f259c56a163e5511353cf4ebb5) )
	ROM_LOAD( "ic20.16", 0xa0000, 0x10000, CRC(7fb396ad) SHA1(b245bb9a6a4c1ec518906c59c3a3da5e412369ab) )
	ROM_LOAD( "ic21.17", 0xb0000, 0x10000, CRC(fb83c652) SHA1(3d124e5c751732e8055bbb7b6853b6e64435a85d) )
	ROM_LOAD( "ic22.18", 0xc0000, 0x10000, CRC(d8b689e9) SHA1(14db7ba2246f12db9b6b5b4dcb4a648e4a65ace4) )
	ROM_LOAD( "ic23.19", 0xd0000, 0x10000, CRC(e6611947) SHA1(9267dad097b318174706d51fb94d613208b04f60) )
	ROM_LOAD( "ic10.06", 0xe0000, 0x10000, CRC(473b349a) SHA1(9f5d08e07c8175bc7ec3854499177af2c398bd76) )
	ROM_LOAD( "ic11.07", 0xf0000, 0x10000, CRC(d9a60285) SHA1(f8ef211e022e9c8ea25f6d8fb16266867656a591) )

	ROM_REGION( 0x40000, "m72", 0 ) // samples - same as original data, just in 64K chunks
	ROM_LOAD( "ic02.02", 0x00000, 0x10000, CRC(92f0093d) SHA1(530b924aa991283045577d03524dfc7eacf1be49) )
	ROM_LOAD( "ic03.03", 0x10000, 0x10000, CRC(116a049c) SHA1(656c0d1d7f945c5f5637892721a58421b682fd01) )
	ROM_LOAD( "ic04.04", 0x20000, 0x10000, CRC(6840692b) SHA1(f6f7b063ecf7206e172843515be38376f8845b42) )
	ROM_LOAD( "ic05.05", 0x30000, 0x10000, CRC(92ffe22a) SHA1(19dcaf6e25bb7498d4ab19fa0a63f3326b9bff8f) )
ROM_END

} // anonymous namespace

// Original IREM M80-A-A PCB
GAME( 1989, matchit,   0,         shisen,   matchit, shisen_state, empty_init, ROT0, "Tamtex",  "Match It",                                MACHINE_SUPPORTS_SAVE ) // program ROM SIS A-27-I
GAME( 1989, shisen,    matchit,   shisen,   shisen,  shisen_state, empty_init, ROT0, "Tamtex",  "Shisensho - Joshiryo-Hen (Japan, set 1)", MACHINE_SUPPORTS_SAVE ) // program ROM SIS A-27-D
GAME( 1989, shisena,   matchit,   shisen,   shisen,  shisen_state, empty_init, ROT0, "Tamtex",  "Shisensho - Joshiryo-Hen (Japan, set 2)", MACHINE_SUPPORTS_SAVE ) // program ROM SIS A-27-A

// Bootleg hardware
GAME( 1989, matchitb,  matchit,   shisen,   matchit, shisen_state, empty_init, ROT0, "bootleg", "Match It (bootleg)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1989, sichuan2,  matchit,   shisen,   shisen,  shisen_state, empty_init, ROT0, "bootleg", "Sichuan II (bootleg, set 1)",             MACHINE_SUPPORTS_SAVE )
GAME( 1989, sichuan2a, matchit,   shisen,   shisen,  shisen_state, empty_init, ROT0, "bootleg", "Sichuan II (bootleg, set 2)",             MACHINE_SUPPORTS_SAVE )
