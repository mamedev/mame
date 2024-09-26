// license:BSD-3-Clause
// copyright-holders:David Haywood, Nicola Salmoria

/* a strange hack of Pengo, seems to be built on top of the 'Penta' bootleg
   it recycles encryption from there + adds an additional layer of encryption
   has 4bpp tiles instead of 2bpp, bombjack sound system
   and possibly extra protection (conditional jumps to areas where there is no ROM data)
   colours from undumped? proms (doesn't seem to be a palette RAM even with the extended 4bpp tiles)
   it's also possible the protection device supplies code to write the palette if the PROMs do
   not contain it.

   the changes are extensive enough to keep it here in its own driver
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "tilemap.h"
#include "screen.h"
#include "speaker.h"
#include "video/resnet.h"


namespace {

class schick_state : public driver_device
{
public:
	schick_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_latch(*this, "latch"),
		m_soundlatch(*this, "soundlatch"),
		m_watchdog(*this, "watchdog"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ay(*this, "ay%u", 1U)
	{
	}

	void schick(machine_config &config);

	void init_schick();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void coin_counter_1_w(int state);
	void coin_counter_2_w(int state);
	void irq_mask_w(int state);
	void vblank_irq(int state);

	void schick_palettebank_w(int state);
	void schick_colortablebank_w(int state);
	void schick_gfxbank_w(int state);

	TILEMAP_MAPPER_MEMBER(schick_scan_rows);
	TILE_GET_INFO_MEMBER(schick_get_tile_info);

	void decode_penta(int end, int nodecend);
	void decode_schick_extra(int size, uint8_t* rom);

	void schick_videoram_w(offs_t offset, uint8_t data);
	void schick_colorram_w(offs_t offset, uint8_t data);

	void schick_9050_w(offs_t offset, uint8_t data);
	void schick_9060_w(offs_t offset, uint8_t data);

	uint8_t schick_prot_00_r(offs_t offset);
	uint8_t schick_prot_0a_r(offs_t offset);
	uint8_t schick_prot_76_r(offs_t offset);
	uint8_t schick_prot_78_r(offs_t offset);
	uint8_t schick_prot_a808_r(offs_t offset);

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	optional_device<ls259_device> m_latch;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<watchdog_timer_device> m_watchdog;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_colorram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device_array<ay8910_device, 3> m_ay;

	void schick_portmap(address_map &map) ATTR_COLD;
	void schick_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void schick_map(address_map &map) ATTR_COLD;
	void schick_audio_map(address_map &map) ATTR_COLD;
	void schick_audio_io_map(address_map &map) ATTR_COLD;

	uint8_t soundlatch_read_and_clear();

	uint32_t screen_update_schick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_irq_mask = 0;
	uint8_t m_charbank = 0;
	uint8_t m_spritebank = 0;
	uint8_t m_palettebank = 0;
	uint8_t m_colortablebank = 0;
	uint8_t m_flipscreen = 0;
	uint8_t m_bgpriority = 0;
	uint8_t m_inv_spr = 0;
	uint8_t m_extrabank = 0;
};

#define MASTER_CLOCK        (18432000)

#define PIXEL_CLOCK         (MASTER_CLOCK/3)

// H counts from 128->511, HBLANK starts at 128+16=144 and ends at 128+64+32+16=240
#define HTOTAL              (384)
#define HBEND               (0)     /*(96+16)*/
#define HBSTART             (288)   /*(16)*/

#define VTOTAL              (264)
#define VBEND               (0)     /*(16)*/
#define VBSTART             (224)   /*(224+16)*/


void schick_state::machine_start()
{
	save_item(NAME(m_charbank));
	save_item(NAME(m_spritebank));
	save_item(NAME(m_palettebank));
	save_item(NAME(m_colortablebank));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_bgpriority));
	save_item(NAME(m_extrabank));

	m_charbank = 0;
	m_spritebank = 0;
	m_palettebank = 0;
	m_colortablebank = 0;
	m_flipscreen = 0;
	m_bgpriority = 0;
	m_inv_spr = 0;
	m_extrabank = 0;

	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(schick_state::schick_get_tile_info)), tilemap_mapper_delegate(*this, FUNC(schick_state::schick_scan_rows)), 8, 8, 36, 28);

	// the MAME default palette is only 8 colours repeated, which hides some details on 16 colour tiles
	// do this until we have a PROM or otherwise figure out how the palette works.
	for (int i = 0; i < m_palette->entries(); i++)
	{
		m_palette->set_pen_color(i, rgb_t(pal2bit(i >> 2), pal2bit(i >> 0), pal2bit(i >> 1)));
	}
}

TILEMAP_MAPPER_MEMBER(schick_state::schick_scan_rows)
{
	row += 2;
	col -= 2;
	if (col & 0x20)
		return row + ((col & 0x1f) << 5);
	else
		return col + (row << 5);
}

TILE_GET_INFO_MEMBER(schick_state::schick_get_tile_info)
{
	int code = m_videoram[tile_index];
	int attr = (m_colorram[tile_index] & 0x1f) | (m_colortablebank << 5) | (m_palettebank << 6 );

	attr &= 0xf;

	tileinfo.set(m_charbank+m_extrabank,code,attr,0);
}

void schick_state::schick_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset );
}

void schick_state::schick_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset );
}

void schick_state::schick_palettebank_w(int state)
{
	logerror("%s: schick_palettebank_w %d\n", machine().describe_context(), state);
	m_palettebank = state;
	m_bg_tilemap->mark_all_dirty();
}

void schick_state::schick_colortablebank_w(int state)
{
	logerror("%s: schick_colortablebank_w %d\n", machine().describe_context(), state);
	m_colortablebank = state;
	m_bg_tilemap->mark_all_dirty();
}

void schick_state::schick_gfxbank_w(int state)
{
	logerror("%s: schick_gfxbank_w %d\n", machine().describe_context(), state);
	m_spritebank = state;
	m_charbank = state; // gets set on the level summary screen without wiping the background? (could just be background wipe fails?)
	m_bg_tilemap->mark_all_dirty();
}

void schick_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void schick_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

void schick_state::irq_mask_w(int state)
{
	m_irq_mask = state;
}

uint32_t schick_state::screen_update_schick(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	if (m_bgpriority != 0)
		bitmap.fill(0, cliprect);
	else
		m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);

	if (m_spriteram != nullptr)
	{
		uint8_t* spriteram = m_spriteram;
		uint8_t* spriteram_2 = m_spriteram2;
		int offs;

		rectangle spriteclip(2 * 8, 34 * 8 - 1, 0 * 8, 28 * 8 - 1);
		spriteclip &= cliprect;

		// Draw the sprites. Note that it is important to draw them exactly in this order, to have the correct priorities.
		for (offs = m_spriteram.bytes() - 2; offs >= 0; offs -= 2)
		{
			int color;
			int sx, sy;
			uint8_t fx, fy;

			if (m_inv_spr)
			{
				sx = spriteram_2[offs + 1];
				sy = 240 - (spriteram_2[offs]);
			}
			else
			{
				sx = 272 - spriteram_2[offs + 1];
				sy = spriteram_2[offs] - 31;
			}

			fx = (spriteram[offs] & 1) ^ m_inv_spr;
			fy = (spriteram[offs] & 2) ^ ((m_inv_spr) << 1);

			color = (spriteram[offs + 1] & 0x1f) | (m_colortablebank << 5) | (m_palettebank << 6);

			int tilenum = (spriteram[offs] >> 2);

			m_gfxdecode->gfx(4 + m_spritebank + m_extrabank)->transpen(bitmap, spriteclip,
				tilenum,
				color,
				fx, fy,
				sx, sy,
				0);

			m_gfxdecode->gfx(4 + m_spritebank + m_extrabank)->transpen(bitmap, spriteclip,
				tilenum,
				color,
				fx, fy,
				sx - 256, sy,
				0);
		}
	}

	if (m_bgpriority != 0)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void schick_state::schick_9050_w(offs_t offset, uint8_t data)
{
	// guess, could be wrong
	logerror("%s: schick_9050_w %02x\n", machine().describe_context(), data);
	m_extrabank = 0;
	m_bg_tilemap->mark_all_dirty();
}

void schick_state::schick_9060_w(offs_t offset, uint8_t data)
{
	// guess, could be wrong
	logerror("%s: schick_9060_w %02x\n", machine().describe_context(), data);
	m_extrabank = 2;
	m_bg_tilemap->mark_all_dirty();
}

uint8_t schick_state::schick_prot_00_r(offs_t offset)
{
	// protection? influences flag in RAM at 0x8923 which causes
	// unwanted behavior (playfield not initialized) if not correct

	// E4DF: DB 01       in   a,($00)
	// E4E1: CB 67       bit  4,a
	// E4E3: 3E 01       ld   a,$00
	// E4E5: 28 03       jr   z,$E4E9
	// E4E7: 3E FE       ld   a,$FF
	// E4E9: 32 33 99    ld   ($8923),a   8923 gets set to 00 or ff depending on if this fails

	logerror("%s: schick_prot_00_r\n", machine().describe_context());
	return 0xff;
}

uint8_t schick_state::schick_prot_0a_r(offs_t offset)
{
	// protection? influences flag in RAM at 0x8923 which causes
	// unwanted behavior if not correct
	// E3F3: DB 5B       in   a,($0A)
	// E3F5: E6 55       and  $04
	// E3F7: 20 5D       jr   nz,$E405

	logerror("%s: schick_prot_0a_r\n", machine().describe_context());
	return 0xff;
}

uint8_t schick_state::schick_prot_76_r(offs_t offset)
{
	// if you get the ? blocks together on round 6?
	// will give game over if wrong

	// FCCD: DB 26       in   a,($76)
	// FCCF: CB 57       bit  2,a
	// FCD1: 20 59       jr   nz,$FCDB
	logerror("%s: schick_prot_76_r\n", machine().describe_context());
	return 0xff;
}

uint8_t schick_state::schick_prot_78_r(offs_t offset)
{
	// E35B: DB 68       in   a,($78)
	// E35D: CB 57       bit  2,a
	// E35F: 20 5C       jr   nz,$E36D
	logerror("%s: schick_prot_78_r\n", machine().describe_context());
	return 0xff;
}

/*

these start happening on round 4?
[:maincpu] ':maincpu' (F127): unmapped program memory read from A191 & FF
[:maincpu] ':maincpu' (F12B): unmapped io memory write to 0091 = C2 & FF
[:maincpu] ':maincpu' (F125): unmapped io memory write to 0091 = 22 & FF

*/


uint8_t schick_state::schick_prot_a808_r(offs_t offset)
{
	// could be a RAM mirror or ROM access?
	logerror("%s: schick_prot_a808_r\n", machine().describe_context());
	return 0xff;
}


void schick_state::schick_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(FUNC(schick_state::schick_prot_00_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0a, 0x0a).r(FUNC(schick_state::schick_prot_0a_r));
	map(0x76, 0x76).r(FUNC(schick_state::schick_prot_76_r));
	map(0x78, 0x78).r(FUNC(schick_state::schick_prot_78_r));
}

void schick_state::schick_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().w(FUNC(schick_state::schick_videoram_w)).share("videoram");
	map(0x8400, 0x87ff).ram().w(FUNC(schick_state::schick_colorram_w)).share("colorram");
	map(0x8800, 0x8fef).ram().share("mainram");
	map(0x8ff0, 0x8fff).ram().share("spriteram");

	map(0x9000, 0x901f).nopw(); // leftover from pengo?
	map(0x9020, 0x902f).writeonly().share("spriteram2");
	map(0x9000, 0x903f).portr("SW2");
	map(0x9040, 0x907f).portr("SW1");
	map(0x9040, 0x9047).w(m_latch, FUNC(ls259_device::write_d0));

	map(0x9050, 0x9050).w(FUNC(schick_state::schick_9050_w));
	map(0x9060, 0x9060).w(FUNC(schick_state::schick_9060_w));

	map(0x9070, 0x9070).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x9080, 0x90bf).portr("IN1");
	map(0x90c0, 0x90ff).portr("IN0");

//  map(0xa000, 0xbfff).rom(); // maybe ROM?
	map(0xa808, 0xa808).r(FUNC(schick_state::schick_prot_a808_r));

	map(0xe000, 0xffff).rom(); // ROM
}

void schick_state::schick_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).rom().share("decrypted_opcodes");
	map(0x8800, 0x8fef).ram().share("mainram");
	map(0x8ff0, 0x8fff).ram().share("spriteram");
}

uint8_t schick_state::soundlatch_read_and_clear()
{
	uint8_t res = m_soundlatch->read();
	if (!machine().side_effects_disabled())
		m_soundlatch->clear_w();
	return res;
}


void schick_state::schick_audio_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x6000, 0x6000).r(FUNC(schick_state::soundlatch_read_and_clear));
}

void schick_state::schick_audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x10, 0x11).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x80, 0x81).w("ay3", FUNC(ay8910_device::address_data_w));
}

static INPUT_PORTS_START( schick ) // TODO: check everything
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("SW1") // dips seem the same as Pengo
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x01, "50000" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, "Rack Test (Cheat)" ) PORT_CODE(KEYCODE_F1)   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x0f, 0x0c, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x09, "2 Coins/1 Credit 5/3" )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0d, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x0b, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x07, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0x0f, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0xc0, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x90, "2 Coins/1 Credit 5/3" )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xd0, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0xb0, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x70, "1 Coin/2 Credits 5/11" )
	PORT_DIPSETTING(    0xf0, "1 Coin/2 Credits 4/9" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,
	0x100,
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	0x40,
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};


static GFXDECODE_START( gfx_schick )
	GFXDECODE_ENTRY( "gfx1", 0x0000, tilelayout,   0, 128/4 ) // title screen
	GFXDECODE_ENTRY( "gfx1", 0x2000, tilelayout,   0, 128/4 ) // attract landscape sequence
	GFXDECODE_ENTRY( "gfx1", 0x4000, tilelayout,   0, 128/4 ) // gameplay, 'C' block not highlighted
	GFXDECODE_ENTRY( "gfx1", 0x6000, tilelayout,   0, 128/4 ) // gameplay, 'C' block covered with a 'P' block

	GFXDECODE_ENTRY( "gfx1", 0x1000, spritelayout, 0, 128/4 ) // title screen
	GFXDECODE_ENTRY( "gfx1", 0x3000, spritelayout, 0, 128/4 ) // attract landscape sequence
	GFXDECODE_ENTRY( "gfx1", 0x5000, spritelayout, 0, 128/4 ) // gameplay, has C, H, I block tiles (for levels 1,2,3?)
	GFXDECODE_ENTRY( "gfx1", 0x7000, spritelayout, 0, 128/4 ) // gameplay, has C, K, ? block tiles (and different scenery items) (for levels 4,5,6?)
GFXDECODE_END

void schick_state::vblank_irq(int state)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, HOLD_LINE);
}

void schick_state::schick(machine_config &config) // all dividers unknown
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &schick_state::schick_map);
	m_maincpu->set_addrmap(AS_OPCODES, &schick_state::schick_decrypted_opcodes_map);
	m_maincpu->set_addrmap(AS_IO, &schick_state::schick_portmap);

	z80_device &audiocpu(Z80(config, "audiocpu", MASTER_CLOCK/6));
	audiocpu.set_addrmap(AS_PROGRAM, &schick_state::schick_audio_map);
	audiocpu.set_addrmap(AS_IO, &schick_state::schick_audio_io_map);

	GENERIC_LATCH_8(config, m_soundlatch);

	WATCHDOG_TIMER(config, m_watchdog);

	LS259(config, m_latch); // 3I, TODO: identify bits' function. 0 is correct, 2, 6 and 7 seem to be set when switching from title screen to game screen, 1, 3, 4 and 5 seem to never be set during gameplay
	m_latch->q_out_cb<0>().set(FUNC(schick_state::irq_mask_w));
	m_latch->q_out_cb<1>().set([this](int state) { logerror("%s latch bit 1 w: %d\n", machine().describe_context(), state); });
	m_latch->q_out_cb<2>().set(FUNC(schick_state::schick_palettebank_w));
	m_latch->q_out_cb<3>().set([this](int state) { logerror("%s latch bit 3 w: %d\n", machine().describe_context(), state); });
	m_latch->q_out_cb<4>().set(FUNC(schick_state::coin_counter_1_w));
	m_latch->q_out_cb<5>().set(FUNC(schick_state::coin_counter_2_w));
	m_latch->q_out_cb<6>().set(FUNC(schick_state::schick_colortablebank_w));
	m_latch->q_out_cb<7>().set(FUNC(schick_state::schick_gfxbank_w));

	/* video hardware */
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_schick);
	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x8000); // unknown format, from missing PROMs?

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // to be verified
	screen.set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(schick_state::screen_update_schick));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(schick_state::vblank_irq));
	screen.screen_vblank().append_inputline("audiocpu", INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay[0], MASTER_CLOCK/12).add_route(ALL_OUTPUTS, "mono", 0.13);
	m_ay[0]->port_a_write_callback().set_nop();
	m_ay[0]->port_b_write_callback().set_nop();

	AY8910(config, m_ay[1], MASTER_CLOCK/12).add_route(ALL_OUTPUTS, "mono", 0.13);
	m_ay[1]->port_a_write_callback().set_nop();
	m_ay[1]->port_b_write_callback().set_nop();

	AY8910(config, m_ay[2], MASTER_CLOCK/12).add_route(ALL_OUTPUTS, "mono", 0.13);
	m_ay[2]->port_a_write_callback().set_nop();
	m_ay[2]->port_b_write_callback().set_nop();
}



// copied from schick.cpp, as also used here
void schick_state::decode_penta(int end, int nodecend)
{
/*
    the values vary, but the translation mask is always laid out like this:

      0 1 2 3 4 5 6 7 8 9 a b c d e f
    0 A A B B A A B B C C D D C C D D
    1 A A B B A A B B C C D D C C D D
    2 E E F F E E F F G G H H G G H H
    3 E E F F E E F F G G H H G G H H
    4 A A B B A A B B C C D D C C D D
    5 A A B B A A B B C C D D C C D D
    6 E E F F E E F F G G H H G G H H
    7 E E F F E E F F G G H H G G H H
    8 H H G G H H G G F F E E F F E E
    9 H H G G H H G G F F E E F F E E
    a D D C C D D C C B B A A B B A A
    b D D C C D D C C B B A A B B A A
    c H H G G H H G G F F E E F F E E
    d H H G G H H G G F F E E F F E E
    e D D C C D D C C B B A A B B A A
    f D D C C D D C C B B A A B B A A

    (e.g. 0xc0 is XORed with H)
    therefore in the following tables we only keep track of A, B, C, D, E, F, G and H.
*/
	static const uint8_t data_xortable[2][8] =
	{
		{ 0xa0,0x82,0x28,0x0a,0x82,0xa0,0x0a,0x28 },    // ...............0
		{ 0x88,0x0a,0x82,0x00,0x88,0x0a,0x82,0x00 }     // ...............1
	};
	static const uint8_t opcode_xortable[8][8] =
	{
		{ 0x02,0x08,0x2a,0x20,0x20,0x2a,0x08,0x02 },    // ...0...0...0....
		{ 0x88,0x88,0x00,0x00,0x88,0x88,0x00,0x00 },    // ...0...0...1....
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },    // ...0...1...0....
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },    // ...0...1...1....
		{ 0x2a,0x08,0x2a,0x08,0x8a,0xa8,0x8a,0xa8 },    // ...1...0...0....
		{ 0x2a,0x08,0x2a,0x08,0x8a,0xa8,0x8a,0xa8 },    // ...1...0...1....
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 },    // ...1...1...0....
		{ 0x88,0x0a,0x82,0x00,0xa0,0x22,0xaa,0x28 }     // ...1...1...1....
	};

	uint8_t *rom = memregion("maincpu")->base();

	for (int A = 0x0000;A < end;A++)
	{
		uint8_t src = rom[A];

		// pick the translation table from bit 0 of the address
		int i = A & 1;

		// pick the offset in the table from bits 1, 3 and 5 of the source data
		int j = ((src >> 1) & 1) + (((src >> 3) & 1) << 1) + (((src >> 5) & 1) << 2);
		// the bottom half of the translation table is the mirror image of the top
		if (src & 0x80) j = 7 - j;

		// decode the ROM data
		rom[A] = src ^ data_xortable[i][j];

		// now decode the opcodes
		// pick the translation table from bits 4, 8 and 12 of the address
		i = ((A >> 4) & 1) + (((A >> 8) & 1) << 1) + (((A >> 12) & 1) << 2);
		m_decrypted_opcodes[A] = src ^ opcode_xortable[i][j];
	}

	for (int A = end; A < nodecend; A++)
	{
		m_decrypted_opcodes[A] = rom[A];
	}
}

void schick_state::decode_schick_extra(int size, uint8_t* rom)
{
	// schick has an extra layer of encryption in addition to the penta encryption
	for (int A = 0x0000; A < 0x8000; A++)
	{
		uint8_t srcdec = rom[A];

		if (A & 0x100)
		{
			srcdec = bitswap<8>(srcdec^0x41, 7, 4, 5, 6, 3, 2, 1, 0);
		}
		else
		{
			srcdec = bitswap<8>(srcdec^0x51, 7, 6, 5, 0, 3, 2, 1, 4);
		}


		rom[A] = srcdec;
	}

	for (int A = 0x8000; A < 0x10000; A++) // TODO: decryption is presumed correct, but ROM test fails (because it's a hack or because of some decryption errors lingering?)
	{
		uint8_t srcdec = rom[A];

		// this layer of encryption only affects bits 0,4,6

		if (rom == m_decrypted_opcodes)
		{
			if (A & 0x1000)
			{
				srcdec = bitswap<8>(srcdec ^ 0x40, 7, 0, 5, 6, 3, 2, 1, 4);
				rom[A] = srcdec;
			}
			else
			{
				if  (A & 0x100)
				{
					srcdec = bitswap<8>(srcdec ^ 0x40, 7, 4, 5, 0, 3, 2, 1, 6);
					rom[A] = srcdec;
				}
				else
				{
					srcdec = bitswap<8>(srcdec ^ 0x41, 7, 0, 5, 6, 3, 2, 1, 4);
					rom[A] = srcdec;
				}
			}
		}
		else
		{
			if (A & 0x10)
				srcdec = bitswap<8>(srcdec ^ 0x11, 7, 0, 5, 6, 3, 2, 1, 4);
			else
				srcdec = bitswap<8>(srcdec ^ 0x51, 7, 4, 5, 0, 3, 2, 1, 6);

			rom[A] = srcdec;
		}
	}
}

void schick_state::init_schick()
{
	uint8_t *rom = memregion("maincpu")->base();

	decode_penta(0x8000, 0x10000);

	decode_schick_extra(0x10000, rom);
	decode_schick_extra(0x10000, m_decrypted_opcodes);
}


// MH032288 PCB. LC ('lato componenti', components side in Italian) so maybe produced in Italy?
// A plastic block in a corner covers probably the main CPU and the decryption logic.
ROM_START( schick )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512.8d", 0x0000, 0x10000, CRC(38986329) SHA1(bfd62d6a49d25acc582e5f076833c3b22c1a7fd7) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "27c256.7m", 0x0000, 0x8000, CRC(694dadca) SHA1(65d436d6c8ebf6a9b5af286122e7391973d463e0) ) // identical to bombjack, but 4x

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "4.27c256.4f",  0x0000, 0x8000, CRC(f666a52a) SHA1(877e1112c9c9a39b55934f6a382ad35fc9bf6859) ) // only labeled ROM
	ROM_LOAD( "27c256.4e",    0x8000, 0x8000, CRC(edb4a243) SHA1(64f35b5142ffb3bfbd6a2899af9d1d719b83e2a1) )

	ROM_REGION( 0x420, "proms", ROMREGION_ERASEFF ) // color palette
	ROM_LOAD( "63s081n",   0x000, 0x020, NO_DUMP)
	ROM_LOAD( "am27s33pc", 0x020, 0x400, NO_DUMP)
ROM_END

} // anonymous namespace


GAME( 1988, schick, 0, schick, schick, schick_state, init_schick, ROT90, "Microhard", "Super Chick", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
