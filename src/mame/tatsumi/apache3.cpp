// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Angelo Salese
/***************************************************************************

    Apache 3                                            ATF-011

    TODO:

    - verify the sky gradient mapping and the ground start line timing against hardware (both are
      inferred from the game's code)

***************************************************************************/

#include "emu.h"
#include "tatsumi.h"

#include "cpu/m68000/m68000.h"
#include "cpu/nec/nec.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

namespace {

class apache3_state : public tatsumi_state
{
public:
	apache3_state(const machine_config &mconfig, device_type type, const char *tag)
		: tatsumi_state(mconfig, type, tag)
		, m_subcpu(*this, "sub")
		, m_subcpu2(*this, "sub2")
		, m_apache3_g_ram(*this, "apache3_g_ram")
		, m_apache3_z80_ram(*this, "apache3_z80_ram")
		, m_apache3_prom(*this, "proms")
		, m_vr1(*this, "VR1")
	{
	}

	void apache3(machine_config &config);

	void init_apache3();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	uint16_t apache3_bank_r();
	void apache3_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void apache3_z80_ctrl_w(uint16_t data);
	uint16_t apache3_v30_v20_r(offs_t offset);
	void apache3_v30_v20_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t apache3_z80_r(offs_t offset);
	void apache3_z80_w(offs_t offset, uint16_t data);
	uint8_t apache3_vr1_r();
	void apache3_rotate_w(uint16_t data);
	void apache3_road_z_w(uint16_t data);
	void apache3_road_x_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void apache3_68000_reset(int state);

	void apache3_68000_map(address_map &map) ATTR_COLD;
	void apache3_v20_map(address_map &map) ATTR_COLD;
	void apache3_v30_map(address_map &map) ATTR_COLD;
	void apache3_z80_map(address_map &map) ATTR_COLD;

	void draw_ground(bitmap_rgb32 &dst, const rectangle &cliprect);

	required_device<m68000_base_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;

	required_shared_ptr<uint16_t> m_apache3_g_ram;
	required_shared_ptr<uint8_t> m_apache3_z80_ram;
	required_region_ptr<uint8_t> m_apache3_prom;

	required_ioport m_vr1;

	uint16_t m_apache3_rotate_ctrl[12]{};
	int m_apache3_rot_idx;
	int16_t m_apache3_road_x_ram[256]{};
	uint8_t m_apache3_road_z = 0;
};


void apache3_state::apache3_road_z_w(uint16_t data)
{
	m_apache3_road_z = data & 0xff;
}

void apache3_state::apache3_road_x_w(offs_t offset, uint8_t data)
{
	// The Z80 puts the value on the address bus and the ground row on the data bus: a 12-bit
	// signed horizontal offset per row of the ground pattern (address bits 12-14 look like its sign extension)
	m_apache3_road_x_ram[data] = util::sext(offset, 12);
}

/*
    Ground and sky

    The 68000 writes 12 words per frame to the TZ2213 accumulators (code at 0x8e06). They appear
    to form three 24-bit accumulators with 8 fractional bits, each stepped per pixel and per line
    (rN is the Nth word written):

      horizontal  start r0:r4 (low byte of r0 = bits 16-23, r4 = bits 0-15), +r2 per pixel, +r6 per line
      sky         start r1:r5 (r1 = bits 8-23, low byte of r5 = bits 0-7),   +r3 per pixel, +r7 per line
      ground      start r9:r10 (r9 = bits 8-23, low byte of r10 = bits 0-7), +r8 per pixel, +r11 per line

    The per-pixel and per-line steps appear to be the cos and sin of the bank angle (scaled 1.0 for
    the ground, by a game-set sky scale for the sky, and with an extra shear term for the horizontal
    accumulator). The ground accumulator is the distance below the horizon, which picks a row of
    the ground pattern RAM (512 pixels x 256 rows, 2 bits per pixel, pre-scaled for perspective and
    mirrored about the centre). Above the horizon the same accumulator goes negative and its low 7
    bits appear to address the sky gradient (palette entries 0x100-0x17f): one entry per line, the
    lightest at the horizon. The gradient keeps the same height whatever the pitch, while the words
    the game writes for the sky accumulator follow it, so they do not appear to select the gradient.
*/
void apache3_state::draw_ground(bitmap_rgb32 &dst, const rectangle &cliprect)
{
	// 24-bit accumulator: a 16-bit word in bits 8-23 plus the low byte of another word in bits 0-7
	auto const acc24 = [] (uint16_t hi, uint16_t lo) { return util::sext((uint32_t(hi) << 8) | (lo & 0xff), 24); };
	int32_t const h0 = util::sext((uint32_t(m_apache3_rotate_ctrl[0] & 0xff) << 16) | m_apache3_rotate_ctrl[4], 24);
	int32_t const g0 = acc24(m_apache3_rotate_ctrl[9], m_apache3_rotate_ctrl[10]);
	int32_t const h_dx = int16_t(m_apache3_rotate_ctrl[2]), h_dy = int16_t(m_apache3_rotate_ctrl[6]);
	int32_t const g_dx = int16_t(m_apache3_rotate_ctrl[8]), g_dy = int16_t(m_apache3_rotate_ctrl[11]);

	pen_t const *const pens = m_palette->pens();

	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		uint32_t *const dest = &dst.pix(y);
		int32_t h = h0 + y * h_dy + cliprect.left() * h_dx;
		int32_t g = g0 + y * g_dy + cliprect.left() * g_dx;
		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			int const row = g >> 8;
			if (row < 0)
			{
				// sky: one gradient entry per line above the horizon, stopping at the darkest
				dest[x] = pens[0x100 + std::max(row + 0x80, 0)];
			}
			else
			{
				int const r = std::min(row, 0xff);
				int hval = util::sext((h >> 8) + m_apache3_road_x_ram[r], 12);
				bool const side = (hval < 0);
				if (side)
					hval = ~hval;
				int pix = 0;
				if (hval < 0x200)
				{
					uint8_t const pixels = m_apache3_g_ram[(r << 7) | (hval >> 2)] & 0xff;
					pix = (pixels >> ((hval & 3) << 1)) & 3;
				}
				int const ln = ((m_apache3_prom[r] + m_apache3_road_z) >> 5) & 3;
				dest[x] = pens[0x200 + (side ? 0x10 : 0) + (pix << 2) + ln];
			}
			h += h_dx;
			g += g_dx;
		}
	}
}

void apache3_state::video_start()
{
	m_tx_layer = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(apache3_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64,64);

	m_tx_layer->set_transparent_pen(0);
}

uint32_t apache3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_sprites->update_cluts();

	m_tx_layer->set_scrollx(0,24);

	bitmap.fill(m_palette->pen(0), cliprect);
	screen.priority().fill(0, cliprect);
	m_sprites->draw_sprites(screen.priority(),cliprect,1,(m_sprite_control_ram[0xe0]&0x1000) ? 0x1000 : 0); // Alpha pass only
	draw_ground(bitmap, cliprect);
	apply_shadow_bitmap(bitmap,cliprect,screen.priority(), 0);
	m_sprites->draw_sprites(bitmap,cliprect,0, (m_sprite_control_ram[0x20]&0x1000) ? 0x1000 : 0);
	m_tx_layer->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

uint16_t apache3_state::apache3_bank_r()
{
	return m_control_word;
}

void apache3_state::apache3_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	/*
	    0x8000  - Set when accessing palette ram (not implemented, perhaps blank screen?)
	    0x0080  - Set when accessing IO cpu RAM/ROM (implemented as halt cpu)
	    0x0060  - IOP bank to access from main cpu (0x0 = RAM, 0x20 = lower ROM, 0x60 = upper ROM)
	    0x0010  - Set when accessing OBJ cpu RAM/ROM (implemented as halt cpu)
	    0x000f  - OBJ bank to access from main cpu (0x8 = RAM, 0x0 to 0x7 = ROM)
	*/

	COMBINE_DATA(&m_control_word);

	if (m_control_word & 0x7f00)
	{
		logerror("Unknown control Word: %04x\n",m_control_word);
		m_subcpu2->set_input_line(INPUT_LINE_HALT, CLEAR_LINE); // ?
	}

	if (m_control_word & 0x10)
		m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	if (m_control_word & 0x80)
		m_audiocpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	else
		m_audiocpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	m_last_control=m_control_word;
}

// D1 = /ZBREQ  - Z80 bus request
// D0 = /GRDACC - Allow 68000 access to road pattern RAM
void apache3_state::apache3_z80_ctrl_w(uint16_t data)
{
	m_subcpu2->set_input_line(INPUT_LINE_HALT, data & 2 ? ASSERT_LINE : CLEAR_LINE);
}

uint16_t apache3_state::apache3_v30_v20_r(offs_t offset)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	/* Each V20 byte maps to a V30 word */
	if ((m_control_word & 0xe0) == 0xe0)
		offset += 0xf8000; /* Upper half */
	else if ((m_control_word & 0xe0) == 0xc0)
		offset += 0xf0000;
	else if ((m_control_word & 0xe0) == 0x80)
		offset += 0x00000; // main ram
	else
		logerror("%08x: unmapped read z80 rom %08x\n", m_maincpu->pc(), offset);
	return 0xff00 | targetspace.read_byte(offset);
}

void apache3_state::apache3_v30_v20_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	address_space &targetspace = m_audiocpu->space(AS_PROGRAM);

	if ((m_control_word & 0xe0) != 0x80)
		logerror("%08x: write unmapped v30 rom %08x\n", m_maincpu->pc(), offset);

	/* Only 8 bits of the V30 data bus are connected - ignore writes to the other half */
	if (ACCESSING_BITS_0_7)
	{
		targetspace.write_byte(offset, data & 0xff);
	}
}

uint16_t apache3_state::apache3_z80_r(offs_t offset)
{
	return m_apache3_z80_ram[offset];
}

void apache3_state::apache3_z80_w(offs_t offset, uint16_t data)
{
	m_apache3_z80_ram[offset] = data & 0xff;
}

uint8_t apache3_state::apache3_vr1_r()
{
	return (uint8_t)((255./100) * (100 - m_vr1->read()));
}

/* Ground/sky rotation control
 *
 * There are 12 16-bit values that are
 * presumably loaded into the 8 TZ2213 custom
 * accumulators and counters.
 */
void apache3_state::apache3_rotate_w(uint16_t data)
{
	m_apache3_rotate_ctrl[m_apache3_rot_idx] = data;
	m_apache3_rot_idx = (m_apache3_rot_idx + 1) % 12;
}

void apache3_state::apache3_v30_map(address_map &map)
{
	map(0x00000, 0x03fff).ram();
	map(0x04000, 0x07fff).ram().share("nvram");
	map(0x08000, 0x08fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x0c000, 0x0dfff).ram().w(FUNC(apache3_state::text_w)).share("videoram");
	map(0x0e800, 0x0e803).w(FUNC(apache3_state::hd6445_crt_w)).umask16(0x00ff);
	map(0x0f000, 0x0f001).portr("DSW");
	map(0x0f000, 0x0f001).nopw(); // todo
	map(0x0f800, 0x0f801).rw(FUNC(apache3_state::apache3_bank_r), FUNC(apache3_state::apache3_bank_w));
	map(0x10000, 0x1ffff).rw(FUNC(apache3_state::apache3_v30_v20_r), FUNC(apache3_state::apache3_v30_v20_w));
	map(0x20000, 0x2ffff).rw(FUNC(apache3_state::tatsumi_v30_68000_r), FUNC(apache3_state::tatsumi_v30_68000_w));
	map(0x80000, 0xfffff).rom().region("master_rom", 0);
}

void apache3_state::apache3_68000_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("slave_rom", 0);
	map(0x80000, 0x83fff).ram().share("sharedram");
	map(0x90000, 0x93fff).ram().share("spriteram");
	map(0x9a000, 0x9a1ff).rw(FUNC(apache3_state::tatsumi_sprite_control_r), FUNC(apache3_state::tatsumi_sprite_control_w)).share("obj_ctrl_ram");
	map(0xa0000, 0xa0001).w(FUNC(apache3_state::apache3_rotate_w)); // /BNKCS
	map(0xb0000, 0xb0001).w(FUNC(apache3_state::apache3_z80_ctrl_w));
	map(0xc0000, 0xc0001).w(FUNC(apache3_state::apache3_road_z_w)); // /LINCS
	map(0xd0000, 0xdffff).ram().share("apache3_g_ram"); // /GRDCS
	map(0xe0000, 0xe7fff).rw(FUNC(apache3_state::apache3_z80_r), FUNC(apache3_state::apache3_z80_w));
}

void apache3_state::apache3_v20_map(address_map &map)
{
	map(0x00000, 0x01fff).ram();
	map(0x04000, 0x04003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x06000, 0x06001).portr("IN0"); // esw
	map(0x08000, 0x08001).rw(m_ym2151, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x0a000, 0x0a000).r(m_oki, FUNC(okim6295_device::read)).w(m_oki, FUNC(okim6295_device::write));
	map(0x0e000, 0x0e007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w));
	map(0xf0000, 0xfffff).rom().region("sound_rom", 0);
}

void apache3_state::apache3_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("apache3_z80_ram");
	map(0x8000, 0xffff).w(FUNC(apache3_state::apache3_road_x_w));
}


static INPUT_PORTS_START( apache3 )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Trigger" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Power" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Missile" )

	PORT_START("STICK_X")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("STICK_Y")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("THROTTLE")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Z ) PORT_SENSITIVITY(25) PORT_KEYDELTA(79)

	PORT_START("VR1")
	PORT_ADJUSTER(100, "VR1")

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x0000, "3" )
	PORT_DIPSETTING(    0x0001, "4" )
	PORT_DIPSETTING(    0x0002, "5" )
	PORT_DIPSETTING(    0x0003, "6" )
	PORT_DIPNAME( 0x000c, 0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x000c, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0010, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6") /* Listed as "Always On" */
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Use Throttle" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Disable Coins" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0700, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0700, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0800, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4") /* Manual only shows a 3-Way dip box, so 4-8 are unknown */
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Test ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static GFXDECODE_START( gfx_apache3 )
	GFXDECODE_ENTRY( "text",    0, gfx_8x8x3_planar, 768,  16)
GFXDECODE_END


void apache3_state::apache3_68000_reset(int state)
{
	m_subcpu2->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void apache3_state::machine_reset()
{
	m_subcpu2->set_input_line(INPUT_LINE_RESET, ASSERT_LINE); // TODO
}


void apache3_state::apache3(machine_config &config)
{
	/* basic machine hardware */
	V30(config, m_maincpu, apache3_state::CLOCK_1 / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &apache3_state::apache3_v30_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(apache3_state::v30_vector_r));

	M68000(config, m_subcpu, apache3_state::CLOCK_2 / 4);
	m_subcpu->set_addrmap(AS_PROGRAM, &apache3_state::apache3_68000_map);
	m_subcpu->set_vblank_int("screen", FUNC(apache3_state::irq4_line_hold));
	m_subcpu->reset_cb().set(FUNC(apache3_state::apache3_68000_reset));

	V20(config, m_audiocpu, apache3_state::CLOCK_1 / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &apache3_state::apache3_v20_map);

	Z80(config, m_subcpu2, apache3_state::CLOCK_2 / 8);
	m_subcpu2->set_addrmap(AS_PROGRAM, &apache3_state::apache3_z80_map);
	m_subcpu2->set_vblank_int("screen", FUNC(apache3_state::irq0_line_hold));

	config.set_maximum_quantum(attotime::from_hz(6000));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	m58990_device &adc(M58990(config, "adc", 1000000)); // unknown clock
	adc.in_callback<0>().set_ioport("STICK_X");
	adc.in_callback<1>().set_ioport("STICK_Y");
	adc.in_callback<2>().set_constant(0); // VSP1
	adc.in_callback<4>().set(FUNC(apache3_state::apache3_vr1_r));
	adc.in_callback<5>().set_ioport("THROTTLE");
	adc.in_callback<6>().set_constant(0); // RPSNC
	adc.in_callback<7>().set_constant(0); // LPSNC

	I8255(config, "ppi");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen"));
	// The game programs the HD6445 for 50 x 35 characters of 8 x 8 (400 x 280), 40 x 30 visible
	screen.set_raw(apache3_state::CLOCK_2 / 8, 400, 0, 320, 280, 0, 240); // TODO: Hook up CRTC
	screen.set_screen_update(FUNC(apache3_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 0);

	TZB215_SPRITES(config, m_sprites, 0, 0x800);
	m_sprites->set_sprite_palette_base(0);
	m_sprites->set_palette("sprites:palette_clut");
	m_sprites->set_basepalette(m_palette);
	m_sprites->set_spriteram(m_spriteram);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_apache3);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 2048); // 2048 real colours

	/* apache 3 schematics state
	bit 4:  250
	bit 3:  500
	bit 2:  1k
	bit 1:  2k
	bit 0:  3.9kOhm resistor
	*/

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	YM2151(config, m_ym2151, apache3_state::CLOCK_1 / 4);
	m_ym2151->irq_handler().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);
	m_ym2151->add_route(0, "speaker", 0.45, 0);
	m_ym2151->add_route(1, "speaker", 0.45, 1);

	OKIM6295(config, m_oki, apache3_state::CLOCK_1 / 4 / 2, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.75, 0);
	m_oki->add_route(ALL_OUTPUTS, "speaker", 0.75, 1);
}


ROM_START( apache3 )
	ROM_REGION16_LE( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ap-25f.125",   0x020001, 0x10000, CRC(3c7530f4) SHA1(9f7b58a3abddbdc3081ba9dfc1732406eb8c1752) )
	ROM_LOAD16_BYTE( "ap-26f.133",   0x020000, 0x10000, CRC(2955997f) SHA1(86e37def923d9cf4eb33e7979118ec6f1ef62678) )
	ROM_LOAD16_BYTE( "ap-23f.110",   0x060001, 0x10000, CRC(d7077149) SHA1(b08f5a9ee03641c20bdd5e5c9671a22c740150c6) )
	ROM_LOAD16_BYTE( "ap-24f.118",   0x060000, 0x10000, CRC(0bdef11b) SHA1(ed687600962ed2ca3a8e67cbd84fa5486778eade) )

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ap-19c.80",   0x000001, 0x10000, CRC(0908e468) SHA1(a2d725993bd4cd5425468736154fd3dd9dd7b060) )
	ROM_LOAD16_BYTE( "ap-21c.97",   0x000000, 0x10000, CRC(38a056fb) SHA1(67c8ae58670cebde0771854e1fb5fc2eb2543ecc) )
	ROM_LOAD16_BYTE( "ap-20a.89",   0x040001, 0x20000, CRC(92d24b5e) SHA1(1ea270d46a607e47b7e0961b532316aa05dc8f4e) )
	ROM_LOAD16_BYTE( "ap-22a.105",  0x040000, 0x20000, CRC(a8458a92) SHA1(43674731c2e9962c2bfbb73a85484cf03d6be223) )

	ROM_REGION( 0x10000, "sound_rom", 0 ) /* 64k code for sound V20 */
	ROM_LOAD( "ap-27d.151",   0x00000, 0x10000, CRC(294b4d79) SHA1(2b03418a12a2aaf3919b98161d8d0ce6ae29a2bb) )

	ROM_REGION( 0x100000, "sprites:sprites_l", 0)
	ROM_LOAD32_BYTE( "ap-00c.15",   0x000000, 0x20000, CRC(ad1ddc2b) SHA1(81f64663c4892ab5fb0e2dc99513dbfee73f15b8) )
	ROM_LOAD32_BYTE( "ap-01c.22",   0x000001, 0x20000, CRC(6286ff00) SHA1(920da4a3a441dbf54ad86c0f4fb6f47a867e9cda) )
	ROM_LOAD32_BYTE( "ap-04c.58",   0x000002, 0x20000, CRC(dc6d55e4) SHA1(9f48f8d6aa1a329a71913139a8d5a50d95a9b9e5) )
	ROM_LOAD32_BYTE( "ap-05c.65",   0x000003, 0x20000, CRC(2e6e495f) SHA1(af610f265da53735b20ddc6df1bda47fc54ee0c3) )
	ROM_LOAD32_BYTE( "ap-02c.34",   0x080000, 0x20000, CRC(af4ee7cb) SHA1(4fe2361b7431971b07671f145abf1ea5861d01db) )
	ROM_LOAD32_BYTE( "ap-03c.46",   0x080001, 0x20000, CRC(60ab495c) SHA1(18340d4fba550495b1e52f8023a0a2ec6349dfeb) )
	ROM_LOAD32_BYTE( "ap-06c.71",   0x080002, 0x20000, CRC(0ea90e55) SHA1(b16d6b8be4853797507d3e5c933a9dd1d451308e) )
	ROM_LOAD32_BYTE( "ap-07c.75",   0x080003, 0x20000, CRC(ba685543) SHA1(140a2b708d4e4de4d207fc2c4a96a5cab8639988) )

	ROM_REGION( 0x100000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "ap-08c.14",   0x000000, 0x20000, CRC(6437b580) SHA1(2b2ba42add18bbec04fbcf53645a8d44b972e26a) )
	ROM_LOAD32_BYTE( "ap-09c.21",   0x000001, 0x20000, CRC(54d18ef9) SHA1(40ebc6ea49b2a501fe843d60bec8c32d07f2d25d) )
	ROM_LOAD32_BYTE( "ap-12c.57",   0x000002, 0x20000, CRC(f95cf5cf) SHA1(ce373c648cbf3e4863bbc3a1175efe065c75eb13) )
	ROM_LOAD32_BYTE( "ap-13c.64",   0x000003, 0x20000, CRC(67a248c3) SHA1(cc945f7cfecaaab5075c1a3d202369b070d4c656) )
	ROM_LOAD32_BYTE( "ap-10c.33",   0x080000, 0x20000, CRC(74418df4) SHA1(cc1206b10afc2de919b2fb9899486122d27290a4) )
	ROM_LOAD32_BYTE( "ap-11c.45",   0x080001, 0x20000, CRC(195bf78e) SHA1(c3c472f3c4244545b89491b6ebec4f838a6bbb73) )
	ROM_LOAD32_BYTE( "ap-14c.70",   0x080002, 0x20000, CRC(58f7fe16) SHA1(a5b87b42b85808c226df0d2a7b7cdde12d474a41) )
	ROM_LOAD32_BYTE( "ap-15c.74",   0x080003, 0x20000, CRC(1ffd5496) SHA1(25efb568957fc9441a40a7d64cc6afe1a14b392b) )

	ROM_REGION( 0x18000, "text", 0 )
	ROM_LOAD( "ap-18d.73",   0x000000, 0x8000, CRC(55e664bf) SHA1(505bec8b5ff3f9fa2c5fb1213d54683347905be1) )
	ROM_LOAD( "ap-17d.68",   0x008000, 0x8000, CRC(6199afe4) SHA1(ad8c0ed6c33d984bb29c89f2e7fc7e5a923cefe3) )
	ROM_LOAD( "ap-16d.63",   0x010000, 0x8000, CRC(f115656d) SHA1(61798858dc0172192d89e666696b2c7642756899) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ap-28c.171",   0x000000, 0x20000, CRC(b349f0c2) SHA1(cb1ff1c0e784f669c87ab1eccd3b358950761b74) )
	ROM_LOAD( "ap-29c.176",   0x020000, 0x10000, CRC(b38fced3) SHA1(72f61a719f393957bcccf14687bfbb2e7a5f7aee) )

	ROM_REGION( 0x200, "proms", 0 ) /* Road stripe PROM */
	ROM_LOAD( "am27s29.ic41",   0x000, 0x200, CRC(c981f1e0) SHA1(7d8492d9f4033ab3734c09ee23016a0b210648b5) )
ROM_END

ROM_START( apache3a )
	ROM_REGION16_LE( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ap-25c.125",   0x020001, 0x10000, CRC(7bc496a6) SHA1(5491d06181d729407e975b85a8715fdc3b489c67) )
	ROM_LOAD16_BYTE( "ap-26c.133",   0x020000, 0x10000, CRC(9393a470) SHA1(00376f7a545629a83eb5a90b9d1685a68430e4ce) )
	ROM_LOAD16_BYTE( "ap-23e.110",   0x060001, 0x10000, CRC(3e465b8e) SHA1(dfd009221974eb43263dd3f4f80f39bb32c30ced) ) /* hand written label:  23E   DD65 */
	ROM_LOAD16_BYTE( "ap-24e.118",   0x060000, 0x10000, CRC(1ef746f2) SHA1(31d77bd954ea6cc00cac40a3a514281da371030b) ) /* hand written label:  24E   E2A2 */

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ap-19c.80",   0x000001, 0x10000, CRC(0908e468) SHA1(a2d725993bd4cd5425468736154fd3dd9dd7b060) )
	ROM_LOAD16_BYTE( "ap-21c.97",   0x000000, 0x10000, CRC(38a056fb) SHA1(67c8ae58670cebde0771854e1fb5fc2eb2543ecc) )
	ROM_LOAD16_BYTE( "ap-20a.89",   0x040001, 0x20000, CRC(92d24b5e) SHA1(1ea270d46a607e47b7e0961b532316aa05dc8f4e) )
	ROM_LOAD16_BYTE( "ap-22a.105",  0x040000, 0x20000, CRC(a8458a92) SHA1(43674731c2e9962c2bfbb73a85484cf03d6be223) )

	ROM_REGION( 0x10000, "sound_rom", 0 ) /* 64k code for sound V20 */
	ROM_LOAD( "ap-27d.151",   0x00000, 0x10000, CRC(294b4d79) SHA1(2b03418a12a2aaf3919b98161d8d0ce6ae29a2bb) )

	ROM_REGION( 0x100000, "sprites:sprites_l", 0)
	ROM_LOAD32_BYTE( "ap-00c.15",   0x000000, 0x20000, CRC(ad1ddc2b) SHA1(81f64663c4892ab5fb0e2dc99513dbfee73f15b8) )
	ROM_LOAD32_BYTE( "ap-01c.22",   0x000001, 0x20000, CRC(6286ff00) SHA1(920da4a3a441dbf54ad86c0f4fb6f47a867e9cda) )
	ROM_LOAD32_BYTE( "ap-04c.58",   0x000002, 0x20000, CRC(dc6d55e4) SHA1(9f48f8d6aa1a329a71913139a8d5a50d95a9b9e5) )
	ROM_LOAD32_BYTE( "ap-05c.65",   0x000003, 0x20000, CRC(2e6e495f) SHA1(af610f265da53735b20ddc6df1bda47fc54ee0c3) )
	ROM_LOAD32_BYTE( "ap-02c.34",   0x080000, 0x20000, CRC(af4ee7cb) SHA1(4fe2361b7431971b07671f145abf1ea5861d01db) )
	ROM_LOAD32_BYTE( "ap-03c.46",   0x080001, 0x20000, CRC(60ab495c) SHA1(18340d4fba550495b1e52f8023a0a2ec6349dfeb) )
	ROM_LOAD32_BYTE( "ap-06c.71",   0x080002, 0x20000, CRC(0ea90e55) SHA1(b16d6b8be4853797507d3e5c933a9dd1d451308e) )
	ROM_LOAD32_BYTE( "ap-07c.75",   0x080003, 0x20000, CRC(ba685543) SHA1(140a2b708d4e4de4d207fc2c4a96a5cab8639988) )

	ROM_REGION( 0x100000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "ap-08c.14",   0x000000, 0x20000, CRC(6437b580) SHA1(2b2ba42add18bbec04fbcf53645a8d44b972e26a) )
	ROM_LOAD32_BYTE( "ap-09c.21",   0x000001, 0x20000, CRC(54d18ef9) SHA1(40ebc6ea49b2a501fe843d60bec8c32d07f2d25d) )
	ROM_LOAD32_BYTE( "ap-12c.57",   0x000002, 0x20000, CRC(f95cf5cf) SHA1(ce373c648cbf3e4863bbc3a1175efe065c75eb13) )
	ROM_LOAD32_BYTE( "ap-13c.64",   0x000003, 0x20000, CRC(67a248c3) SHA1(cc945f7cfecaaab5075c1a3d202369b070d4c656) )
	ROM_LOAD32_BYTE( "ap-10c.33",   0x080000, 0x20000, CRC(74418df4) SHA1(cc1206b10afc2de919b2fb9899486122d27290a4) )
	ROM_LOAD32_BYTE( "ap-11c.45",   0x080001, 0x20000, CRC(195bf78e) SHA1(c3c472f3c4244545b89491b6ebec4f838a6bbb73) )
	ROM_LOAD32_BYTE( "ap-14c.70",   0x080002, 0x20000, CRC(58f7fe16) SHA1(a5b87b42b85808c226df0d2a7b7cdde12d474a41) )
	ROM_LOAD32_BYTE( "ap-15c.74",   0x080003, 0x20000, CRC(1ffd5496) SHA1(25efb568957fc9441a40a7d64cc6afe1a14b392b) )

	ROM_REGION( 0x18000, "text", 0 )
	ROM_LOAD( "ap-18e.73",   0x000000, 0x10000, CRC(d7861a26) SHA1(b1a1e089a293a5536d342c9edafbea303f4f128c) )
	ROM_LOAD( "ap-16e.63",   0x008000, 0x10000, CRC(d3251965) SHA1(aef4f58a6f773060434abda9d7f5f003693577bf) )
	ROM_LOAD( "ap-17e.68",   0x008000, 0x08000, CRC(4509c2ed) SHA1(97a6a6710e83aca212ce43d06c3f26c35f9782b8) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ap-28c.171",   0x000000, 0x20000, CRC(b349f0c2) SHA1(cb1ff1c0e784f669c87ab1eccd3b358950761b74) )
	ROM_LOAD( "ap-29c.176",   0x020000, 0x10000, CRC(b38fced3) SHA1(72f61a719f393957bcccf14687bfbb2e7a5f7aee) )

	ROM_REGION( 0x200, "proms", 0 ) /* Road stripe PROM */
	ROM_LOAD( "am27s29.ic41",   0x000, 0x200, CRC(c981f1e0) SHA1(7d8492d9f4033ab3734c09ee23016a0b210648b5) )
ROM_END

ROM_START( apache3b )
	ROM_REGION16_LE( 0x80000, "master_rom", 0 ) /* v30 main cpu */
	ROM_LOAD16_BYTE( "ap-25c.125",   0x020001, 0x10000, CRC(7bc496a6) SHA1(5491d06181d729407e975b85a8715fdc3b489c67) )
	ROM_LOAD16_BYTE( "ap-26c.133",   0x020000, 0x10000, CRC(9393a470) SHA1(00376f7a545629a83eb5a90b9d1685a68430e4ce) )
	ROM_LOAD16_BYTE( "ap-23g.110",   0x060001, 0x10000, CRC(0ab485e4) SHA1(d8d0695312732c31cedcb1c298810a6793835e80) ) /* Kana Corporation license program ROMs */
	ROM_LOAD16_BYTE( "ap-24g.118",   0x060000, 0x10000, CRC(6348e196) SHA1(6be537491a56a28b62981cae6db8dfc4eb2fece2) ) /* Kana Corporation license program ROMs */

	ROM_REGION16_BE( 0x80000, "slave_rom", 0 ) /* 68000 sub cpu */
	ROM_LOAD16_BYTE( "ap-19c.80",   0x000001, 0x10000, CRC(0908e468) SHA1(a2d725993bd4cd5425468736154fd3dd9dd7b060) )
	ROM_LOAD16_BYTE( "ap-21c.97",   0x000000, 0x10000, CRC(38a056fb) SHA1(67c8ae58670cebde0771854e1fb5fc2eb2543ecc) )
	ROM_LOAD16_BYTE( "ap-20a.89",   0x040001, 0x20000, CRC(92d24b5e) SHA1(1ea270d46a607e47b7e0961b532316aa05dc8f4e) )
	ROM_LOAD16_BYTE( "ap-22a.105",  0x040000, 0x20000, CRC(a8458a92) SHA1(43674731c2e9962c2bfbb73a85484cf03d6be223) )

	ROM_REGION( 0x10000, "sound_rom", 0 ) /* 64k code for sound V20 */
	ROM_LOAD( "ap-27d.151",   0x00000, 0x10000, CRC(294b4d79) SHA1(2b03418a12a2aaf3919b98161d8d0ce6ae29a2bb) )

	ROM_REGION( 0x100000, "sprites:sprites_l", 0)
	ROM_LOAD32_BYTE( "ap-00c.15",   0x000000, 0x20000, CRC(ad1ddc2b) SHA1(81f64663c4892ab5fb0e2dc99513dbfee73f15b8) )
	ROM_LOAD32_BYTE( "ap-01c.22",   0x000001, 0x20000, CRC(6286ff00) SHA1(920da4a3a441dbf54ad86c0f4fb6f47a867e9cda) )
	ROM_LOAD32_BYTE( "ap-04c.58",   0x000002, 0x20000, CRC(dc6d55e4) SHA1(9f48f8d6aa1a329a71913139a8d5a50d95a9b9e5) )
	ROM_LOAD32_BYTE( "ap-05c.65",   0x000003, 0x20000, CRC(2e6e495f) SHA1(af610f265da53735b20ddc6df1bda47fc54ee0c3) )
	ROM_LOAD32_BYTE( "ap-02c.34",   0x080000, 0x20000, CRC(af4ee7cb) SHA1(4fe2361b7431971b07671f145abf1ea5861d01db) )
	ROM_LOAD32_BYTE( "ap-03c.46",   0x080001, 0x20000, CRC(60ab495c) SHA1(18340d4fba550495b1e52f8023a0a2ec6349dfeb) )
	ROM_LOAD32_BYTE( "ap-06c.71",   0x080002, 0x20000, CRC(0ea90e55) SHA1(b16d6b8be4853797507d3e5c933a9dd1d451308e) )
	ROM_LOAD32_BYTE( "ap-07c.75",   0x080003, 0x20000, CRC(ba685543) SHA1(140a2b708d4e4de4d207fc2c4a96a5cab8639988) )

	ROM_REGION( 0x100000, "sprites:sprites_h", 0)
	ROM_LOAD32_BYTE( "ap-08c.14",   0x000000, 0x20000, CRC(6437b580) SHA1(2b2ba42add18bbec04fbcf53645a8d44b972e26a) )
	ROM_LOAD32_BYTE( "ap-09c.21",   0x000001, 0x20000, CRC(54d18ef9) SHA1(40ebc6ea49b2a501fe843d60bec8c32d07f2d25d) )
	ROM_LOAD32_BYTE( "ap-12c.57",   0x000002, 0x20000, CRC(f95cf5cf) SHA1(ce373c648cbf3e4863bbc3a1175efe065c75eb13) )
	ROM_LOAD32_BYTE( "ap-13c.64",   0x000003, 0x20000, CRC(67a248c3) SHA1(cc945f7cfecaaab5075c1a3d202369b070d4c656) )
	ROM_LOAD32_BYTE( "ap-10c.33",   0x080000, 0x20000, CRC(74418df4) SHA1(cc1206b10afc2de919b2fb9899486122d27290a4) )
	ROM_LOAD32_BYTE( "ap-11c.45",   0x080001, 0x20000, CRC(195bf78e) SHA1(c3c472f3c4244545b89491b6ebec4f838a6bbb73) )
	ROM_LOAD32_BYTE( "ap-14c.70",   0x080002, 0x20000, CRC(58f7fe16) SHA1(a5b87b42b85808c226df0d2a7b7cdde12d474a41) )
	ROM_LOAD32_BYTE( "ap-15c.74",   0x080003, 0x20000, CRC(1ffd5496) SHA1(25efb568957fc9441a40a7d64cc6afe1a14b392b) )

	ROM_REGION( 0x18000, "text", 0 )
	ROM_LOAD( "ap-18e.73",   0x000000, 0x10000, CRC(d7861a26) SHA1(b1a1e089a293a5536d342c9edafbea303f4f128c) )
	ROM_LOAD( "ap-16e.63",   0x008000, 0x10000, CRC(d3251965) SHA1(aef4f58a6f773060434abda9d7f5f003693577bf) )
	ROM_LOAD( "ap-17e.68",   0x008000, 0x08000, CRC(4509c2ed) SHA1(97a6a6710e83aca212ce43d06c3f26c35f9782b8) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "ap-28c.171",   0x000000, 0x20000, CRC(b349f0c2) SHA1(cb1ff1c0e784f669c87ab1eccd3b358950761b74) )
	ROM_LOAD( "ap-29c.176",   0x020000, 0x10000, CRC(b38fced3) SHA1(72f61a719f393957bcccf14687bfbb2e7a5f7aee) )

	ROM_REGION( 0x200, "proms", 0 ) /* Road stripe PROM */
	ROM_LOAD( "am27s29.ic41",   0x000, 0x200, CRC(c981f1e0) SHA1(7d8492d9f4033ab3734c09ee23016a0b210648b5) )
ROM_END


void apache3_state::init_apache3()
{
	init_tatsumi();

	m_apache3_rot_idx = 0;

	save_item(NAME(m_apache3_rot_idx));
	save_item(NAME(m_apache3_rotate_ctrl));
	save_item(NAME(m_apache3_road_x_ram));
	save_item(NAME(m_apache3_road_z));

	// TODO: ym2151_set_port_write_handler for CT1/CT2 outputs
}

} // anonymous namespace

GAME( 1988, apache3,   0,        apache3,   apache3,  apache3_state,  init_apache3,  ROT0, "Tatsumi", "Apache 3 (rev F)", MACHINE_IMPERFECT_GRAPHICS ) // Rev F CPU code
GAME( 1988, apache3a,  apache3,  apache3,   apache3,  apache3_state,  init_apache3,  ROT0, "Tatsumi", "Apache 3 (rev E)", MACHINE_IMPERFECT_GRAPHICS ) // Rev C & E CPU code
GAME( 1988, apache3b,  apache3,  apache3,   apache3,  apache3_state,  init_apache3,  ROT0, "Tatsumi (Kana Corporation license)", "Apache 3 (Kana Corporation license, rev G)", MACHINE_IMPERFECT_GRAPHICS ) // Rev C & G CPU code
