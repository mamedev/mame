// license:BSD-3-Clause
// copyright-holders:David Haywood
/*** DRIVER INFORMATION & NOTES ***********************************************

Super Slams - Driver by David Haywood
   Sound Information from R. Belmont
   DSWs corrected by Stephh

TODO :

sprite offset control?
priorities (see hi-score table during attract mode)
unknown reads / writes (also KONAMI chip ..?)

WORKING NOTES :

68k interrupts
lev 1 : 0x64 : 0000 0406 - vblank?
lev 2 : 0x68 : 0000 06ae - x
lev 3 : 0x6c : 0000 06b4 - x
lev 4 : 0x70 : 0000 06ba - x
lev 5 : 0x74 : 0000 06c0 - x
lev 6 : 0x78 : 0000 06c6 - x
lev 7 : 0x7c : 0000 06cc - x

******************************************************************************/


/*** README INFORMATION *******************************************************

Super Slam
(C) 1995 Banpresto

(C) 1995 Banpresto / Toei Animation

PCB: VSEP-26
CPU: TMP68HC000P16 (68000, 64 pin DIP)
SND: Z8400B (Z80B, 40 pin DIP), YM2610, YM3016
OSC: 14.318180 MHz (Near VS920D & 053936), 24.000000 MHz, 32.000 MHz (Both near VS920F & EB26IC66)
Other Chips:
            Fujitsu CG10103 145 9520 Z14    (160 Pin PQFP)
            VS9210 4L06F1056 JAPAN 9525EAI  (176 Pin PQFP)
            VS920F 4L01F1435 JAPAN 9524EAI  (100 Pin PQFP)
            VS920E 4L06F1057 JAPAN 9533EAI  (176 pin PQFP)
            VS9209 4L01F1429 JAPAN 9523EAI  (80 pin PQFP)
            VS920D 4L04F1689 JAPAN 9524EAI  (160 pin PQFP)
            KONAMI KS10011-PF 053936 PSAC2 9522 Z02 (80 pin PQFP)

RAM:
            LGS GM76C28K-10 x 1 (Connected/Near Z80B)
            LGS GM76C28K-10 x 2 (Connected/Near 053936)
            SEC KM6264BLS-7 x 2 (Connected/Near VS920D)
            SEC KM6264BLS-7 x 2 (Connected/Near VS9210)
            UM61256FK-15 x 2 (Connected/Near VS9210)
            CY7C195-25PC x 1     -\
            UM61256FK-15 x 4       > (Connected/Near Fujitsu CG10103 145 9520 Z14)
            SEC KM6264BLS-7 x 4  -/
            LGS GM76C28K-10 x 2 (Connected/Near VS920F)
            LGS GM76C28K-10 x 2 (Connected/Near VS920E)
            UM61256FK-15 x 2 (Connected/Near 68000 & VS9209)

PALs: (4 total, not dumped, 2 located near 68000, 1 near Z80B, 1 near VS9210)

DIPs: 8 position x 3 (ALL DIPs linked to VS9209)

Info taken from sheet supplied with PCB, no info for SW3 (which is never read?).

ROMs: (on ALL ROMs is written only "EB26")

EB26_100.BIN    16M Mask    \
EB26_101.BIN    16M Mask    |
EB26IC09.BIN    16M Mask    |  GFX (near VS9210, 053936 & VS920D)
EB26IC10.BIN    16M Mask    |
EB26IC12.BIN    16M Mask    /
EB26IC36.BIN    16M Mask
EB26IC43.BIN    16M Mask       GFX (Near VS920F & VS920E)
EB26IC59.BIN    8M Mask        Sound (Near YM2610)
EB26IC66.BIN    16M Mask       Sound (Near YM2610)
EB26IC38.BIN    27C1001            Sound Program (Near Z80B)
EB26IC47.BIN    27C240      \
EB26IC73.BIN    27C240      /  Main Program

******************************************************************************/

#include "emu.h"

#include "vs9209.h"
#include "vsystem_spr.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"
#include "sound/ymopn.h"
#include "video/k053936.h"

#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class suprslam_state : public driver_device
{
public:
	suprslam_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k053936(*this, "k053936"),
		m_spr(*this, "vsystem_spr"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch"),
		m_screen_videoram(*this, "screen_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_sp_videoram(*this, "sp_videoram"),
		m_spriteram(*this, "spriteram"),
		m_screen_vregs(*this, "screen_vregs"),
		m_soundbank(*this, "soundbank")
	{ }

	void suprslam(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053936_device> m_k053936;
	required_device<vsystem_spr_device> m_spr;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	// memory pointers
	required_shared_ptr<uint16_t> m_screen_videoram;
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_sp_videoram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_screen_vregs;
	required_memory_bank m_soundbank;

	// video-related
	tilemap_t *m_screen_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint16_t m_screen_bank = 0;
	uint16_t m_bg_bank = 0;
	uint8_t m_spr_ctrl = 0;

	uint32_t tile_callback(uint32_t code);
	void sh_bankswitch_w(uint8_t data);
	void screen_videoram_w(offs_t offset, uint16_t data);
	void bg_videoram_w(offs_t offset, uint16_t data);
	void bank_w(uint16_t data);
	void spr_ctrl_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


// FG 'SCREEN' LAYER

void suprslam_state::screen_videoram_w(offs_t offset, uint16_t data)
{
	m_screen_videoram[offset] = data;
	m_screen_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(suprslam_state::get_tile_info)
{
	int tileno = m_screen_videoram[tile_index] & 0x0fff;
	int colour = m_screen_videoram[tile_index] & 0xf000;

	tileno += m_screen_bank;
	colour = colour >> 12;

	tileinfo.set(0, tileno, colour, 0);
}


// BG LAYER
void suprslam_state::bg_videoram_w(offs_t offset, uint16_t data)
{
	m_bg_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(suprslam_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index] & 0x0fff;
	int colour = m_bg_videoram[tile_index] & 0xf000;

	tileno += m_bg_bank;
	colour = colour >> 12;

	tileinfo.set(1, tileno, colour, 0);
}


uint32_t suprslam_state::tile_callback(uint32_t code)
{
	return m_sp_videoram[code];
}



void suprslam_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(suprslam_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_screen_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(suprslam_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_spr_ctrl = 0;
	m_screen_tilemap->set_transparent_pen(15);
}

void suprslam_state::spr_ctrl_w(uint8_t data)
{
	m_spr_ctrl = data;
}

uint32_t suprslam_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_screen_tilemap->set_scrollx(0, m_screen_vregs[0x04 / 2] );

	bitmap.fill(m_palette->black_pen(), cliprect);
	m_k053936->zoom_draw(screen, bitmap, cliprect, m_bg_tilemap, 0, 0, 1);
	if (!(m_spr_ctrl & 8))
		m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect);
	m_screen_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	if (m_spr_ctrl & 8)
		m_spr->draw_sprites(m_spriteram, m_spriteram.bytes(), screen, bitmap, cliprect);
	return 0;
}

void suprslam_state::bank_w(uint16_t data)
{
	uint16_t old_screen_bank = m_screen_bank;
	uint16_t old_bg_bank = m_bg_bank;

	m_screen_bank = data & 0xf000;
	m_bg_bank = (data & 0x0f00) << 4;

	if (m_screen_bank != old_screen_bank)
		m_screen_tilemap->mark_all_dirty();
	if (m_bg_bank != old_bg_bank)
		m_bg_tilemap->mark_all_dirty();
}


/*** SOUND *******************************************************************/

void suprslam_state::sh_bankswitch_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x03);
}

/*** MEMORY MAPS *************************************************************/

void suprslam_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0xfb0000, 0xfb1fff).ram().share(m_spriteram);
	map(0xfc0000, 0xfcffff).ram().share(m_sp_videoram);
	map(0xfd0000, 0xfdffff).ram();
	map(0xfe0000, 0xfe0fff).ram().w(FUNC(suprslam_state::screen_videoram_w)).share(m_screen_videoram);
	map(0xff0000, 0xff1fff).ram().w(FUNC(suprslam_state::bg_videoram_w)).share(m_bg_videoram);
	map(0xff2000, 0xff203f).ram().share(m_screen_vregs);
	map(0xff3000, 0xff3001).nopw(); // sprite buffer trigger?
	map(0xff8000, 0xff8fff).rw(m_k053936, FUNC(k053936_device::linectrl_r), FUNC(k053936_device::linectrl_w));
	map(0xff9001, 0xff9001).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0xffa000, 0xffafff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xffd000, 0xffd01f).w(m_k053936, FUNC(k053936_device::ctrl_w));
	map(0xffe000, 0xffe001).w(FUNC(suprslam_state::bank_w));
	map(0xfff000, 0xfff01f).rw("io", FUNC(vs9209_device::read), FUNC(vs9209_device::write)).umask16(0x00ff);
}

void suprslam_state::sound_map(address_map &map)
{
	map(0x0000, 0x77ff).rom();
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void suprslam_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(suprslam_state::sh_bankswitch_w));
	map(0x04, 0x04).rw(m_soundlatch, FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::acknowledge_w));
	map(0x08, 0x0b).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
}

/*** INPUT PORTS *************************************************************/

static INPUT_PORTS_START( suprslam )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )                // Only in "test mode"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )             // "Test"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Coin Slots" )
	PORT_DIPSETTING(    0x01, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x0e, 0x0e, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Play Time" )
	PORT_DIPSETTING(    0x08, "2:00" )
	PORT_DIPSETTING(    0x0c, "3:00" )
	PORT_DIPSETTING(    0x04, "4:00" )
	PORT_DIPSETTING(    0x00, "5:00" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x80, 0x00, "Country" )
	PORT_DIPSETTING(    0x80, DEF_STR( Japan ) )
	PORT_DIPSETTING(    0x00, DEF_STR( World ) )
INPUT_PORTS_END

/*** GFX DECODE **************************************************************/

static GFXDECODE_START( gfx_suprslam )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_packed_lsb,   0x000, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_16x16x4_packed_msb, 0x100, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_suprslam_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_16x16x4_packed_msb, 0x200, 16 )
GFXDECODE_END


/*** MACHINE DRIVER **********************************************************/

void suprslam_state::machine_start()
{
	save_item(NAME(m_screen_bank));
	save_item(NAME(m_bg_bank));
	save_item(NAME(m_spr_ctrl));

	m_soundbank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x8000);
}

void suprslam_state::machine_reset()
{
	m_screen_bank = 0;
	m_bg_bank = 0;
}

void suprslam_state::suprslam(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &suprslam_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(suprslam_state::irq1_line_hold));

	Z80(config, m_audiocpu, 32_MHz_XTAL / 8); // 4 MHz ??? not verified
	m_audiocpu->set_addrmap(AS_PROGRAM, &suprslam_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &suprslam_state::sound_io_map);

	vs9209_device &io(VS9209(config, "io", 0));
	io.porta_input_cb().set_ioport("P1");
	io.portb_input_cb().set_ioport("P2");
	io.portc_input_cb().set_ioport("SYSTEM");
	io.portd_input_cb().set_ioport("DSW1");
	io.porte_input_cb().set_ioport("DSW2");
	io.porth_output_cb().set(FUNC(suprslam_state::spr_ctrl_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_suprslam);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_UPDATE_AFTER_VBLANK);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2300)); // hand-tuned
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 40*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(suprslam_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xGBR_555, 0x800);

	VSYSTEM_SPR(config, m_spr, 0, m_palette, gfx_suprslam_spr);
	m_spr->set_tile_indirect_cb(FUNC(suprslam_state::tile_callback));

	K053936(config, m_k053936, 0);
	m_k053936->set_wrap(1);
	m_k053936->set_offsets(-45, -21);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 32_MHz_XTAL / 4)); // not verified
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);
}

/*** ROM LOADING *************************************************************/

ROM_START( suprslam )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 Code
	ROM_LOAD16_WORD_SWAP( "eb26ic47.bin", 0x000000, 0x080000, CRC(8d051fd8) SHA1(1820209306116e5b09cc10a8b3661d232c688b24) )
	ROM_LOAD16_WORD_SWAP( "eb26ic73.bin", 0x080000, 0x080000, CRC(ca4ad383) SHA1(143ee475761fa54d5b3a9f4e3fb3acc8408972fd) )

	ROM_REGION( 0x030000, "audiocpu", 0 ) // Z80 Code
	ROM_LOAD( "eb26ic38.bin", 0x000000, 0x020000, CRC(153f2c50) SHA1(b70f248cfb18239fcd26e36fb36159f219debf2c) )

	ROM_REGION( 0x200000, "ymsnd:adpcma", 0 ) // Samples
	ROM_LOAD( "eb26ic66.bin", 0x000000, 0x200000, CRC(8cb33682) SHA1(0e6189ef0673227d35b9a154e333cc6cf9b65df6) )

	ROM_REGION( 0x100000, "ymsnd:adpcmb", 0 ) // Samples
	ROM_LOAD( "eb26ic59.bin", 0x000000, 0x100000, CRC(4ae4095b) SHA1(62b0600b18febb6cecb6370b03a2d6b7756840a2) )

	ROM_REGION( 0x200000, "fgtiles", 0 ) // 8x8x4 'Screen' Layer GFX
	ROM_LOAD( "eb26ic43.bin", 0x000000, 0x200000, CRC(9dfb0959) SHA1(ba479192a422a55efcf8aa7ff995c914525b4a56) )

	ROM_REGION( 0x800000, "sprites", 0 ) // 16x16x4 Sprites GFX
	ROM_LOAD16_WORD_SWAP( "eb26ic09.bin", 0x000000, 0x200000, CRC(5a415365) SHA1(a59a4ab231980b0540e9a8356a02530217779dbd) )
	ROM_LOAD16_WORD_SWAP( "eb26ic10.bin", 0x200000, 0x200000, CRC(a04f3140) SHA1(621ff823d93fecdde801912064ac951727b71677) )
	ROM_LOAD16_WORD_SWAP( "eb26_100.bin", 0x400000, 0x200000, CRC(c2ee5eb6) SHA1(4b61e77a0d0f38b542d5e32fa25799a4c85bf651) )
	ROM_LOAD16_WORD_SWAP( "eb26_101.bin", 0x600000, 0x200000, CRC(7df654b7) SHA1(3a5ed6ee7cc31566e908b835a065e9bce60389fb) )

	ROM_REGION( 0x400000, "bgtiles", 0 ) // 16x16x4 BG GFX
	ROM_LOAD16_WORD_SWAP( "eb26ic12.bin", 0x000000, 0x200000, CRC(14561bd7) SHA1(5f69f68a305aba9acb21b844c8aa5b1de60f89ff) )
	ROM_LOAD16_WORD_SWAP( "eb26ic36.bin", 0x200000, 0x200000, CRC(92019d89) SHA1(dbf6f8384341707996e4b9e07a3d4f536cf4905b) )
ROM_END

} // anonymous namespace


/*** GAME DRIVERS ************************************************************/

GAME( 1995, suprslam, 0, suprslam, suprslam, suprslam_state, empty_init, ROT0, "Banpresto / Toei Animation / Video System Co.", "From TV Animation Slam Dunk - Super Slams", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // Video System credited in ending screen
