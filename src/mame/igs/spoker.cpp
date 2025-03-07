// license:BSD-3-Clause
// copyright-holders: Mirko Buffoni, Roberto Fresca

/***************************************************************************

  Super Poker (IGS)
  Driver by Mirko Buffoni
  Additional work by Roberto Fresca.

****************************************************************************

  NOTES:

  - Very similar to IGS009 driver, but without the reels stuff.
    Maybe both drivers can be merged at some point.

  - The Super Poker US/UA sets seem to be something in-between hardware
    emulated in igs/igspoker.cpp and the one in this driver. They lack of
    PPI 8255 devices, sporting instead an IGS026A custom (verified via decap
    as ULA for inputs/addressing).

****************************************************************************

  TODO:

  - Understand how to reset NVRAM
  - Verify / complete DSW for spk sets where operator mode doesn't help
     (every one but 100, 102u, 102ua, 114it, 130, 200, 200ua, 201ua, 203us)
  - Verify LEDs and coin counters (should be ok)
  - 3super8 randomly crashes
  - 3super8 doesn't have the 8x32 tilemap, change the video emulation accordingly
  - jinhulu2 stops at "system is connecting". Some type of link feature?

***************************************************************************/

#include "emu.h"

#include "cpu/z180/z180.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class spoker_state : public driver_device
{
public:
	spoker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_bg_tile_ram(*this, "bg_tile_ram"),
		m_fg_tile_ram(*this, "fg_tile_ram"),
		m_fg_color_ram(*this, "fg_color_ram"),
		m_dsw(*this, "DSW%u", 1U),
		m_leds(*this, "led%u", 0U)
	{ }

	void spoker(machine_config &config);
	void _3super8(machine_config &config);

	void init_spk100();
	void init_spk114it();
	void init_spk116it();
	void init_spk120in();
	void init_3super8();

	int hopper_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_shared_ptr<uint8_t> m_bg_tile_ram;
	tilemap_t *m_bg_tilemap = nullptr;

	required_shared_ptr<uint8_t> m_fg_tile_ram;
	required_shared_ptr<uint8_t> m_fg_color_ram;
	tilemap_t *m_fg_tilemap = nullptr;

	required_ioport_array<5> m_dsw;
	output_finder<7> m_leds;

	// common
	int m_nmi_ack = 0;
	uint8_t m_out[3]{};

	// spk116it and spk115it specific
	int m_video_enable = 0;
	int m_hopper = 0;
	uint8_t m_igs_magic[2]{};

	// common
	void bg_tile_w(offs_t offset, uint8_t data);
	void fg_tile_w(offs_t offset, uint8_t data);
	void fg_color_w(offs_t offset, uint8_t data);
	void nmi_and_coins_w(uint8_t data);
	void leds_w(uint8_t data);

	// spk116it and spk115it specific
	void video_and_leds_w(uint8_t data);
	void magic_w(offs_t offset, uint8_t data);
	uint8_t magic_r();

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void _3super8_portmap(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	void spoker_portmap(address_map &map) ATTR_COLD;
};

class spokeru_state : public spoker_state
{
public:
	using spoker_state::spoker_state;

	void spokeru(machine_config &config);

	void init_spokeru();

protected:
	virtual void video_start() override ATTR_COLD;

	void portmap(address_map &map) ATTR_COLD;

	void coins_w(uint8_t data);
	void nmi_video_leds_w(uint8_t data);

private:
	void program_map(address_map &map) ATTR_COLD;
};

class jinhulu2_state : public spokeru_state
{
public:
	jinhulu2_state(const machine_config &mconfig, device_type type, const char *tag) :
		spokeru_state(mconfig, type, tag)
	{ }

	void jinhulu2(machine_config &config);

	void init_jinhulu2();

private:
	void nmi_w(uint8_t data);

	void portmap(address_map &map) ATTR_COLD;
};

class jb_state : public spokeru_state
{
public:
	jb_state(const machine_config &mconfig, device_type type, const char *tag) :
		spokeru_state(mconfig, type, tag),
		m_reel_ram(*this, "reel_ram.%u", 0U),
		m_reel_scroll_ram(*this, "reel_scroll_ram.%u", 0U)
	{ }

	void jb(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr_array<uint8_t, 3> m_reel_ram;
	required_shared_ptr_array<uint8_t, 2> m_reel_scroll_ram;

	tilemap_t *m_reel_tilemap[3]{};

	template<uint8_t Reel> TILE_GET_INFO_MEMBER(get_reel_tile_info);
	template<uint8_t Reel> void reel_ram_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void portmap(address_map &map) ATTR_COLD;
};


/***************************************************************************
                                Video Hardware
***************************************************************************/

void spoker_state::bg_tile_w(offs_t offset, uint8_t data)
{
	m_bg_tile_ram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(spoker_state::get_bg_tile_info)
{
	int const code = m_bg_tile_ram[tile_index];
	tileinfo.set(1 + (tile_index & 3), code & 0xff, 0, 0);
}

TILE_GET_INFO_MEMBER(spoker_state::get_fg_tile_info)
{
	int const code = m_fg_tile_ram[tile_index] | (m_fg_color_ram[tile_index] << 8);
	tileinfo.set(0, code, (4 * (code >> 14) + 3), 0);
}

void spoker_state::fg_tile_w(offs_t offset, uint8_t data)
{
	m_fg_tile_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void spoker_state::fg_color_w(offs_t offset, uint8_t data)
{
	m_fg_color_ram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

template<uint8_t Reel>
TILE_GET_INFO_MEMBER(jb_state::get_reel_tile_info)
{
	int const code = m_reel_ram[Reel][tile_index];

	tileinfo.set(1,
			(code)+ (((tile_index + 1) & 0x3) * 0x100),
			(code & 0x80) ? 0xc : 0,
			0);
}

template<uint8_t Reel>
void jb_state::reel_ram_w(offs_t offset, uint8_t data)
{
	m_reel_ram[Reel][offset] = data;
	m_reel_tilemap[Reel]->mark_tile_dirty(offset);
}

void spoker_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spoker_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 32, 128, 8);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spoker_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 128, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

void spokeru_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(spokeru_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

void jb_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jb_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);

	m_reel_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jb_state::get_reel_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jb_state::get_reel_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);
	m_reel_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jb_state::get_reel_tile_info<2>)), TILEMAP_SCAN_ROWS, 8, 32, 64, 8);

	m_reel_tilemap[0]->set_scroll_cols(64);
	m_reel_tilemap[1]->set_scroll_cols(64);
	m_reel_tilemap[2]->set_scroll_cols(64);
}

uint32_t spoker_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	if (m_bg_tilemap)
		m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t jb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	for (int i = 0; i < 0x40; i++)
	{
		m_reel_tilemap[0]->set_scrolly(i, m_reel_scroll_ram[0][i + 0x000]);
		m_reel_tilemap[1]->set_scrolly(i, m_reel_scroll_ram[0][i + 0x040]);
		m_reel_tilemap[2]->set_scrolly(i, m_reel_scroll_ram[0][i + 0x080]);
	}

	int startclipmin = 0;
	const rectangle &visarea = screen.visible_area();

	for (int j = 0; j < 0x100 - 1; j++)
	{
		rectangle clip;
		int const rowenable = m_reel_scroll_ram[1][j];

		// draw top of screen
		clip.set(visarea.min_x, visarea.max_x, startclipmin, startclipmin + 1);

		if (rowenable == 0)
		{
			m_reel_tilemap[0]->draw(screen, bitmap, clip, 0, 0);
		}
		else if (rowenable == 1)
		{
			m_reel_tilemap[1]->draw(screen, bitmap, clip, 0, 0);
		}
		else if (rowenable == 2)
		{
			m_reel_tilemap[2]->draw(screen, bitmap, clip, 0, 0);
		}
		else if (rowenable == 3)
		{
		}

		startclipmin += 1;
	}

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}
/***************************************************************************
                               Misc Handlers
***************************************************************************/

int spoker_state::hopper_r()
{
	if (m_hopper) return !(m_screen->frame_number() % 10);
	return machine().input().code_pressed(KEYCODE_H);
}

static void show_out(running_machine &machine, uint8_t *out)
{
#ifdef MAME_DEBUG
	machine.popmessage("%02x %02x %02x", out[0], out[1], out[2]);
#endif
}

void spoker_state::nmi_and_coins_w(uint8_t data)
{
	if (data & 0x22)
	{
		logerror("PC %06X: nmi_and_coins = %02x\n", m_maincpu->pc(), data);
//      popmessage("%02x", data);
	}

	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));   // coin_a
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2));   // coin_c
	machine().bookkeeping().coin_counter_w(2, BIT(data, 3));   // key in
	machine().bookkeeping().coin_counter_w(3, BIT(data, 4));   // coin out mech

	m_leds[6] = BIT(data, 6);   // led for coin out / hopper active

	if (((m_nmi_ack & 0x80) == 0) && data & 0x80)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_nmi_ack = data & 0x80;     // nmi acknowledge, 0 -> 1

	m_out[0] = data;
	show_out(machine(), m_out);
}

void spokeru_state::coins_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);   // coin_a
	machine().bookkeeping().coin_counter_w(1, data & 0x04);   // coin_c
	machine().bookkeeping().coin_counter_w(2, data & 0x08);   // key in
	machine().bookkeeping().coin_counter_w(3, data & 0x10);   // coin out mech

	m_leds[6] = BIT(data, 6);   // led for coin out / hopper active

	m_out[0] = data;
	show_out(machine(), m_out);
}

void spoker_state::video_and_leds_w(uint8_t data)
{
	m_leds[4] = BIT(data, 0); // start?
	m_leds[5] = BIT(data, 2); // l_bet?

	m_video_enable = data & 0x40;
	m_hopper = (~data) & 0x80;

	m_out[1] = data;
	show_out(machine(), m_out);
}

void spokeru_state::nmi_video_leds_w(uint8_t data)
{
	m_leds[4] = BIT(data, 0); // start?
	m_leds[5] = BIT(data, 2); // l_bet?

	if (((m_nmi_ack & 0x20) == 0) && data & 0x20)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_nmi_ack = data & 0x20;     // NMI acknowledge, 0 -> 1

	m_video_enable = data & 0x40;
	m_hopper = (~data) & 0x80;

	m_out[1] = data;
	show_out(machine(), m_out);
}

void jinhulu2_state::nmi_w(uint8_t data)
{
	if (data & 0xef)
		logerror("nmi_w: %02x\n", data & 0xef);

	if (((m_nmi_ack & 0x10) == 0) && data & 0x10)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_nmi_ack = data & 0x10;     // NMI acknowledge, 0 -> 1

	// TODO: bit 5 is set often
}

void spoker_state::leds_w(uint8_t data)
{
	m_leds[0] = BIT(data, 0);  // stop_1
	m_leds[1] = BIT(data, 1);  // stop_2
	m_leds[2] = BIT(data, 2);  // stop_3
	m_leds[3] = BIT(data, 3);  // stop
	// data & 0x10?

	m_out[2] = data;
	show_out(machine(), m_out);
}

void spoker_state::magic_w(offs_t offset, uint8_t data)
{
	m_igs_magic[offset] = data;

	if (offset == 0)
		return;

	switch (m_igs_magic[0])
	{
		case 0x01:
			break;

		default:
//          popmessage("magic %x <- %04x", m_igs_magic[0], data);
			logerror("%06x: warning, writing to igs_magic %02x = %02x\n", m_maincpu->pc(), m_igs_magic[0], data);
	}
}

uint8_t spoker_state::magic_r()
{
	switch (m_igs_magic[0])
	{
		case 0x00:
			{
				uint8_t result = 0xff;
				if (BIT(~m_igs_magic[1], 0)) result &= m_dsw[0]->read();
				if (BIT(~m_igs_magic[1], 1)) result &= m_dsw[1]->read();
				if (BIT(~m_igs_magic[1], 2)) result &= m_dsw[2]->read();
				if (BIT(~m_igs_magic[1], 3)) result &= m_dsw[3]->read();
				if (BIT(~m_igs_magic[1], 4)) result &= m_dsw[4]->read();
				return result;
			}

		default:
			logerror("%06x: warning, reading with igs_magic = %02x\n", m_maincpu->pc(), m_igs_magic[0]);
	}

	return 0;
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

void spoker_state::program_map(address_map &map)
{
	map(0x00000, 0x0f3ff).rom();
	map(0x0f400, 0x0ffff).ram().share("nvram");
}

void spokeru_state::program_map(address_map &map)
{
	map(0x00000, 0x0efff).rom();
	map(0x0f000, 0x0ffff).ram().share("nvram");
}

void spoker_state::spoker_portmap(address_map &map)
{
	map(0x0000, 0x003f).ram(); // Z180 internal regs
	map(0x2000, 0x23ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2400, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x3000, 0x33ff).ram().w(FUNC(spoker_state::bg_tile_w)).share(m_bg_tile_ram);
	map(0x5000, 0x5fff).ram().w(FUNC(spoker_state::fg_tile_w)).share(m_fg_tile_ram);
	map(0x6480, 0x6483).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));    // NMI and coins (w), service (r), coins (r)
	map(0x6490, 0x6493).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));    // buttons 1 (r), video and leds (w), leds (w)
	map(0x64a0, 0x64a0).portr("BUTTONS2");
	map(0x64b0, 0x64b1).w("ymsnd", FUNC(ym2413_device::write));
	map(0x64c0, 0x64c0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x64d0, 0x64d1).rw(FUNC(spoker_state::magic_r), FUNC(spoker_state::magic_w));    // DSW1-5
	map(0x7000, 0x7fff).ram().w(FUNC(spoker_state::fg_color_w)).share(m_fg_color_ram);
}

void spokeru_state::portmap(address_map &map)
{
	map(0x0000, 0x003f).ram(); // Z180 internal regs
	map(0x2000, 0x23ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2400, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x4000, 0x4000).portr("DSW1");
	map(0x4001, 0x4001).portr("DSW2");
	map(0x4002, 0x4002).portr("DSW3");
	map(0x4003, 0x4003).portr("DSW4");
	map(0x4004, 0x4004).portr("DSW5");
	map(0x5080, 0x5080).portr("BUTTONS2");
	map(0x5081, 0x5081).portr("SERVICE");
	map(0x5082, 0x5082).portr("COINS");
	map(0x5083, 0x5083).portr("BUTTONS1");
	map(0x5090, 0x5090).w(FUNC(spokeru_state::coins_w));
	map(0x5091, 0x5091).w(FUNC(spokeru_state::leds_w));
	map(0x5092, 0x5092).w(FUNC(spokeru_state::nmi_video_leds_w));
	map(0x50b0, 0x50b1).w("ymsnd", FUNC(ym2413_device::write));
	map(0x50c0, 0x50c0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x7000, 0x77ff).ram().w(FUNC(spokeru_state::fg_tile_w)).share(m_fg_tile_ram);
	map(0x7800, 0x7fff).ram().w(FUNC(spokeru_state::fg_color_w)).share(m_fg_color_ram);
}

void jinhulu2_state::portmap(address_map &map)
{
	map(0x0000, 0x003f).ram(); // Z180 internal regs
	map(0x2000, 0x20ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x3000, 0x30ff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	// TODO: the following reads may be hooked to the wrong inputs
	map(0x4000, 0x4000).portr("DSW1");
	map(0x4001, 0x4001).portr("DSW2");
	map(0x4002, 0x4002).portr("DSW3");
	map(0x5001, 0x5001).portr("SERVICE");
	map(0x5010, 0x5010).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	//map(0x5030, 0x5030).w(FUNC()); // TODO: almost surely same protections as seen in igspoker.cpp and igs011.cpp. Probably the IGS003
	//map(0x5031, 0x5031).r(FUNC()); // TODO: "
	map(0x5031, 0x5031).w(FUNC(jinhulu2_state::nmi_w));
	map(0x7000, 0x77ff).ram().w(FUNC(jinhulu2_state::fg_tile_w)).share(m_fg_tile_ram);
	map(0x7800, 0x7fff).ram().w(FUNC(jinhulu2_state::fg_color_w)).share(m_fg_color_ram);
}

void jb_state::portmap(address_map &map)
{
	spokeru_state::portmap(map);

	map(0x1000, 0x10ff).ram().share(m_reel_scroll_ram[1]);
	map(0x6000, 0x60ff).ram().share(m_reel_scroll_ram[0]);
	map(0x6800, 0x69ff).ram().w(FUNC(jb_state::reel_ram_w<0>)).share(m_reel_ram[0]);
	map(0x6a00, 0x6bff).ram().w(FUNC(jb_state::reel_ram_w<1>)).share(m_reel_ram[1]);
	map(0x6c00, 0x6dff).ram().w(FUNC(jb_state::reel_ram_w<2>)).share(m_reel_ram[2]);
	map(0x6e00, 0x6fff).nopw(); // hardware seems to support a 4th reel, unused by the dumped game (only writes 0xff)
	map(0x8000, 0xffff).rom().region("maincpu", 0x10000);
}

void spoker_state::_3super8_portmap(address_map &map)
{
//  map(0x1000, 0x1fff).nopw();
	map(0x2000, 0x27ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0x2800, 0x2fff).ram().w(m_palette, FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x3000, 0x33ff).ram().w(FUNC(spoker_state::bg_tile_w)).share(m_bg_tile_ram);
	map(0x4000, 0x4000).portr("DSW1");
	map(0x4001, 0x4001).portr("DSW2");
	map(0x4002, 0x4002).portr("DSW3");
	map(0x4003, 0x4003).portr("DSW4");
	map(0x4004, 0x4004).portr("DSW5");
//  map(0x4000, 0x40ff).nopw();
	map(0x5000, 0x5fff).ram().w(FUNC(spoker_state::fg_tile_w)).share(m_fg_tile_ram);

//  The following one (0x6480) should be output. At beginning of code, there is a PPI initialization
//  setting O-I-I for ports ABC. Except these routines are just a leftover from the original game,
//  and now the I/O is handled by a CPLD or FPGA (as seen in goldstar and gp98).
	map(0x6480, 0x6480).portr("IN0");
	map(0x6490, 0x6490).portr("IN1");

	map(0x6491, 0x6491).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x64a0, 0x64a0).portr("IN2");
	map(0x64b0, 0x64b0).w(FUNC(spoker_state::leds_w));
	map(0x64c0, 0x64c0).nopr(); //irq ack?
	map(0x64f0, 0x64f0).w(FUNC(spoker_state::nmi_and_coins_w));
	map(0x7000, 0x7fff).ram().w(FUNC(spoker_state::fg_color_w)).share(m_fg_color_ram);
}


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( spoker ) // this has every hold key which also does another function (i. e. bet, take)
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/40 Credits" )
	PORT_DIPSETTING(    0x00, "1 Coin/50 Credits" )
	PORT_DIPNAME( 0x20, 0x20, "Card Type" )
	PORT_DIPSETTING(    0x20, "Cards" )
	PORT_DIPSETTING(    0x00, "Numbers" )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Min Bet" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x1c, 0x1c, "Key In/Out" )
	PORT_DIPSETTING(    0x1c, "10 Credits" )
	PORT_DIPSETTING(    0x18, "20 Credits" )
	PORT_DIPSETTING(    0x14, "40 Credits" )
	PORT_DIPSETTING(    0x10, "50 Credits" )
	PORT_DIPSETTING(    0x0c, "100 Credits" )
	PORT_DIPSETTING(    0x08, "200 Credits" )
	PORT_DIPSETTING(    0x04, "250 Credits" )
	PORT_DIPSETTING(    0x00, "500 Credits" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPNAME( 0x10, 0x10, "Credit Limit" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, "On (2000)" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Max Bet" )
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "40" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) // stats, memory
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(FUNC(spoker_state::hopper_r)) PORT_NAME("HPSW")   // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spk203us )
	PORT_INCLUDE(spoker)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME(    0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(       0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x02, 0x02, "Hold Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(       0x02, "Hold Win" )
	PORT_DIPSETTING(       0x00, "Auto Hold" )
	PORT_DIPNAME(    0x1c, 0x1c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(       0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(       0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(       0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(       0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(       0x0c, "1 Coin/20 Credits" )
	PORT_DIPSETTING(       0x08, "1 Coin/40 Credits" )
	PORT_DIPSETTING(       0x04, "1 Coin/50 Credits" )
	PORT_DIPSETTING(       0x00, "1 Coin/100 Credits" )
	PORT_DIPNAME(    0x60, 0x60, "Max Bet" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(       0x60, "8" )
	PORT_DIPSETTING(       0x40, "10" )
	PORT_DIPSETTING(       0x20, "50" )
	PORT_DIPSETTING(       0x00, "100" )
	PORT_DIPNAME(    0x80, 0x80, "Hopper" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(       0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(    0x03, 0x03, "Min Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(       0x03, "1" )
	PORT_DIPSETTING(       0x02, "5" )
	PORT_DIPSETTING(       0x01, "10" )
	PORT_DIPSETTING(       0x00, "20" )
	PORT_DIPNAME(    0x1c, 0x1c, "Key In" ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(       0x1c, "10 Credits" )
	PORT_DIPSETTING(       0x18, "20 Credits" )
	PORT_DIPSETTING(       0x14, "40 Credits" )
	PORT_DIPSETTING(       0x10, "50 Credits" )
	PORT_DIPSETTING(       0x0c, "100 Credits" )
	PORT_DIPSETTING(       0x08, "200 Credits" )
	PORT_DIPSETTING(       0x04, "250 Credits" )
	PORT_DIPSETTING(       0x00, "500 Credits" )
	PORT_DIPNAME(    0x60, 0x60, "Key Out" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(       0x60, "1 Credit" )
	PORT_DIPSETTING(       0x40, "10 Credits" )
	PORT_DIPSETTING(       0x20, "100 Credits" )
	PORT_DIPSETTING(       0x00, "500 Credits" )
	PORT_DIPNAME(    0x80, 0x80, "Payout Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(       0x80, "Manual" )
	PORT_DIPSETTING(       0x00, "Auto" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME(    0x01, 0x01, "System Ltd." ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(       0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, "7500" )
	PORT_DIPNAME(    0x02, 0x02, "Show Discard" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(       0x02, "Hold" )
	PORT_DIPSETTING(       0x00, "Discard" )
	PORT_DIPNAME(    0x04, 0x04, "Password" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(       0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x38, 0x38, "Main Rate" ) PORT_DIPLOCATION("SW3:4,5,6")
	PORT_DIPSETTING(       0x38, "50%" )
	PORT_DIPSETTING(       0x30, "60%" )
	PORT_DIPSETTING(       0x28, "65%" )
	PORT_DIPSETTING(       0x20, "70%" )
	PORT_DIPSETTING(       0x18, "75%" )
	PORT_DIPSETTING(       0x10, "80%" )
	PORT_DIPSETTING(       0x08, "85%" )
	PORT_DIPSETTING(       0x00, "90%" )
	PORT_DIPNAME(    0xc0, 0xc0, "W-Up Rate" ) PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(       0xc0, "75%" )
	PORT_DIPSETTING(       0x80, "80%" )
	PORT_DIPSETTING(       0x40, "85%" )
	PORT_DIPSETTING(       0x00, "90%" )

	PORT_MODIFY("DSW4")
	PORT_DIPNAME(    0x01, 0x01, "W-Up Game" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(       0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 ) PORT_DIPLOCATION("SW4:2") // from here on, not shown / listed in test mode
	PORT_DIPUNKNOWN( 0x04, 0x04 ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW4:8")

	PORT_MODIFY("DSW5") // present on PCB, but not shown / listed in test mode
	PORT_DIPUNKNOWN( 0x01, 0x01 ) PORT_DIPLOCATION("SW5:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 ) PORT_DIPLOCATION("SW5:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 ) PORT_DIPLOCATION("SW5:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 ) PORT_DIPLOCATION("SW5:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 ) PORT_DIPLOCATION("SW5:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 ) PORT_DIPLOCATION("SW5:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW5:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW5:8")
INPUT_PORTS_END

static INPUT_PORTS_START( spk201ua )
	PORT_INCLUDE(spk203us)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(    0x60, 0x60, "Key Out" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(       0x60, "1 Credit" )
	PORT_DIPSETTING(       0x40, "10 Credits" )
	PORT_DIPSETTING(       0x20, "100 Credits" )
	PORT_DIPSETTING(       0x00, "100 Credits (duplicate)" )
INPUT_PORTS_END

static INPUT_PORTS_START( spk200ua )
	PORT_INCLUDE(spk201ua)

	PORT_MODIFY("DSW4")
	PORT_DIPUNKNOWN( 0x01, 0x01 ) PORT_DIPLOCATION("SW4:1") // from here on, not shown / listed in test mode
INPUT_PORTS_END

static INPUT_PORTS_START( spk102ua )
	PORT_INCLUDE(spk200ua)

	PORT_MODIFY("DSW3")
	PORT_DIPUNKNOWN( 0x04, 0x04 ) PORT_DIPLOCATION("SW3:3") // from here on, not shown / listed in test mode
	PORT_DIPUNKNOWN( 0x08, 0x08 ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW3:8")
INPUT_PORTS_END

static INPUT_PORTS_START( spk114it ) // this has dedicated keys for every function
	PORT_INCLUDE(spoker)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no payout here

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) // Small
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) // Big

	PORT_MODIFY("DSW1")
	PORT_DIPNAME(    0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(       0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPNAME(    0x02, 0x02, "Hold Mode" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(       0x02, "Hold Win" )
	PORT_DIPSETTING(       0x00, "Auto Hold" )
	PORT_DIPNAME(    0x1c, 0x1c, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(       0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(       0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(       0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(       0x10, "1 Coin/10 Credits" )
	PORT_DIPSETTING(       0x0c, "1 Coin/20 Credits" )
	PORT_DIPSETTING(       0x08, "1 Coin/40 Credits" )
	PORT_DIPSETTING(       0x04, "1 Coin/50 Credits" )
	PORT_DIPSETTING(       0x00, "1 Coin/100 Credits" )
	PORT_DIPNAME(    0x20, 0x20, "Hopper" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(       0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW1:7") // not shown / listed in test mode
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW1:8") // not shown / listed in test mode

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(    0x03, 0x03, "Min Bet" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(       0x03, "1" )
	PORT_DIPSETTING(       0x02, "5" )
	PORT_DIPSETTING(       0x01, "10" )
	PORT_DIPSETTING(       0x00, "20" )
	PORT_DIPNAME(    0x1c, 0x1c, "Key In" ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(       0x1c, "10 Credits" )
	PORT_DIPSETTING(       0x18, "20 Credits" )
	PORT_DIPSETTING(       0x14, "40 Credits" )
	PORT_DIPSETTING(       0x10, "50 Credits" )
	PORT_DIPSETTING(       0x0c, "100 Credits" )
	PORT_DIPSETTING(       0x08, "200 Credits" )
	PORT_DIPSETTING(       0x04, "250 Credits" )
	PORT_DIPSETTING(       0x00, "500 Credits" )
	PORT_DIPNAME(    0x20, 0x20, "Key Out" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(       0x20, "10 Credits" )
	PORT_DIPSETTING(       0x00, "100 Credits" )
	PORT_DIPNAME(    0x40, 0x40, "W-Up Ltd." ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(       0x40, "3500" )
	PORT_DIPSETTING(       0x00, "5000" )
	PORT_DIPNAME(    0x80, 0x80, "Payout Mode" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(       0x80, "Manual" )
	PORT_DIPSETTING(       0x00, "Auto" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME(    0x03, 0x03, "W-Up B_Base" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(       0x03, "100" )
	PORT_DIPSETTING(       0x02, "200" )
	PORT_DIPSETTING(       0x01, "300" )
	PORT_DIPSETTING(       0x00, "400" )
	PORT_DIPNAME(    0x0c, 0x0c, "W-Up Pool" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(       0x0c, "300" )
	PORT_DIPSETTING(       0x08, "500" )
	PORT_DIPSETTING(       0x04, "800" )
	PORT_DIPSETTING(       0x00, "800 (duplicate)" )
	PORT_DIPNAME(    0x30, 0x30, "System Ltd." ) PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(       0x30, "20000" )
	PORT_DIPSETTING(       0x20, "30000" )
	PORT_DIPSETTING(       0x10, "40000" )
	PORT_DIPSETTING(       0x00, "50000" )
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW3:7") // not shown / listed in test mode
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW3:8") // not shown / listed in test mode

	PORT_MODIFY("DSW4")
	PORT_DIPNAME(    0x01, 0x01, "Payout Kind" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(       0x01, "Hopper" )
	PORT_DIPSETTING(       0x00, "Ticket" )
	PORT_DIPNAME(    0x06, 0x06, "Ticket Rate" ) PORT_DIPLOCATION("SW4:2,3")
	PORT_DIPSETTING(       0x06, "10:1" )
	PORT_DIPSETTING(       0x04, "20:1" )
	PORT_DIPSETTING(       0x02, "50:1" )
	PORT_DIPSETTING(       0x00, "100:1" )
	PORT_DIPUNKNOWN( 0x08, 0x08 ) PORT_DIPLOCATION("SW4:4") // not shown / listed in test mode// from here on, not shown / listed in test mode
	PORT_DIPUNKNOWN( 0x10, 0x10 ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW4:8")

	PORT_MODIFY("DSW5") // present on PCB, but not shown / listed in test mode
	PORT_DIPUNKNOWN( 0x01, 0x01 ) PORT_DIPLOCATION("SW5:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 ) PORT_DIPLOCATION("SW5:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 ) PORT_DIPLOCATION("SW5:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 ) PORT_DIPLOCATION("SW5:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 ) PORT_DIPLOCATION("SW5:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 ) PORT_DIPLOCATION("SW5:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW5:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW5:8")
INPUT_PORTS_END

static INPUT_PORTS_START( spk100 )
	PORT_INCLUDE(spk114it)

	PORT_MODIFY("BUTTONS1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet") // Only works as Hold 5 in test mode, but also works as bet in game

	PORT_MODIFY("BUTTONS2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) // this works as Bet in key test but not in game? Probably BTANB

	PORT_MODIFY("DSW4") // not shown / listed in test mode
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW4:3" )
INPUT_PORTS_END

static INPUT_PORTS_START( spk130 )
	PORT_INCLUDE(spk100)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME(    0x40, 0x40, "W-Up Ltd." ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(       0x40, "1750" )
	PORT_DIPSETTING(       0x00, "2500" )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME(    0x03, 0x03, "W-Up B_Base" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(       0x03, "50" )
	PORT_DIPSETTING(       0x02, "100" )
	PORT_DIPSETTING(       0x01, "150" )
	PORT_DIPSETTING(       0x00, "200" )
	PORT_DIPNAME(    0x0c, 0x0c, "W-Up Pool" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(       0x0c, "150" )
	PORT_DIPSETTING(       0x08, "250" )
	PORT_DIPSETTING(       0x04, "400" )
	PORT_DIPSETTING(       0x00, "400 (duplicate)" )
	PORT_DIPNAME(    0x30, 0x30, "System Ltd." ) PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(       0x30, "10000" )
	PORT_DIPSETTING(       0x20, "15000" )
	PORT_DIPSETTING(       0x10, "20000" )
	PORT_DIPSETTING(       0x00, "25000" )
INPUT_PORTS_END

static INPUT_PORTS_START( 3super8 )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("DSW4")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("DSW5")
	PORT_DIPUNKNOWN( 0xff, 0xff )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(FUNC(spoker_state::hopper_r)) PORT_NAME("HPSW")   // hopper sensor
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Statistics")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2   )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / High / Low")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Take")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / W-Up")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Red / Black")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END

// TODO: copied over from older versions in igs/igs009.cpp. Verify if dips are the same. Inputs are verified the same.
static INPUT_PORTS_START( jb )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "W-Up Bonus" )        PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Min Bet" )           PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x08, 0x08, "Spin Speed" )        PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Quick" )
	PORT_DIPNAME( 0x10, 0x00, "Strip Girl" )        PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Payout Mode" )       PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Auto" )
	PORT_DIPNAME( 0xc0, 0xc0, "Player's Panel" )    PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x00, "Type A" )
	PORT_DIPSETTING(    0xc0, "Type A" )
	PORT_DIPSETTING(    0x80, "Type B" )
	PORT_DIPSETTING(    0x40, "Type C" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Rate (%)" )    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "55" )
	PORT_DIPSETTING(    0x06, "60" )
	PORT_DIPSETTING(    0x05, "65" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x03, "75" )
	PORT_DIPSETTING(    0x02, "80" )
	PORT_DIPSETTING(    0x01, "85" )
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPNAME( 0x38, 0x38, "W-Up Chance (%)" )   PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(    0x38, "93" )
	PORT_DIPSETTING(    0x30, "94" )
	PORT_DIPSETTING(    0x28, "95" )
	PORT_DIPSETTING(    0x20, "96" )
	PORT_DIPSETTING(    0x18, "97" )
	PORT_DIPSETTING(    0x10, "98" )
	PORT_DIPSETTING(    0x08, "99" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "Key In Limit" )      PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(    0xc0, "1k" )
	PORT_DIPSETTING(    0x80, "3k" )
	PORT_DIPSETTING(    0x40, "5k" )
	PORT_DIPSETTING(    0x00, "10k" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x07, "Key In Rate" )       PORT_DIPLOCATION("DSW3:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "5" )
	PORT_DIPSETTING(    0x05, "10" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x03, "50" )
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x01, "200" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x38, 0x38, "Coin 1 Rate" )       PORT_DIPLOCATION("DSW3:4,5,6")
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x28, "5" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x10, "25" )
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0xc0, 0xc0, "System Limit" )      PORT_DIPLOCATION("DSW3:7,8")
	PORT_DIPSETTING(    0xc0, "5k" )
	PORT_DIPSETTING(    0x80, "10k" )
	PORT_DIPSETTING(    0x40, "30k" )
	PORT_DIPSETTING(    0x00, "Unlimited" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "Min Play For Fever" )    PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(    0x01, "8" )
	PORT_DIPSETTING(    0x00, "16" )
	PORT_DIPNAME( 0x02, 0x02, "Max Bet" )           PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x1c, 0x1c, "Coin 2 Rate" )       PORT_DIPLOCATION("DSW4:3,4,5")
	PORT_DIPSETTING(    0x1c, "1" )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x14, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x0c, "20" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x60, 0x60, "Key Out Rate" )      PORT_DIPLOCATION("DSW4:6,7")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x80, 0x80, "Play Line" )         PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(    0x80, "8" )
	PORT_DIPSETTING(    0x00, "16" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x03, 0x00, "Maximum Play" )      PORT_DIPLOCATION("DSW5:1,2")
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPSETTING(    0x01, "32" )
	PORT_DIPSETTING(    0x02, "16" )
	PORT_DIPSETTING(    0x03, "8" )
	PORT_DIPNAME( 0x04, 0x04, "Skill Stop" )        PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(    0x04, "On" )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPNAME( 0x08, 0x00, "Hands Count" )       PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(    0x08, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x30, 0x00, "Hands Coin Rate" )   PORT_DIPLOCATION("DSW5:5,6")
	PORT_DIPSETTING(    0x00, "25" )
	PORT_DIPSETTING(    0x20, "10" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPNAME( 0x40, 0x40, "Hands Coin Value" )  PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPNAME( 0x80, 0x80, "Unused" )            PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0x80, "Off" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )    // stats, memory
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM  ) PORT_READ_LINE_MEMBER(FUNC(spoker_state::hopper_r)) PORT_NAME("HPSW")   // hopper sensor
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Pay Out")
	PORT_SERVICE_NO_TOGGLE( 0x20, IP_ACTIVE_LOW )   // test (press during boot)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   PORT_NAME("Records")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Key Down")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SLOT_STOP1    )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SLOT_STOP2    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP3    )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP_ALL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("BUTTONS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1      ) PORT_NAME("Start / Half D-Up Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW  ) PORT_NAME("Small")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1     ) PORT_NAME("Left Bet / 2X D-Up Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2     ) PORT_NAME("Right Bet / D-Up Bet")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Big")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/***************************************************************************
                     Graphics Layout & Graphics Decode
***************************************************************************/

static const gfx_layout layout_8x8x6 =
{
	8, 8,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,2*8) },
	8*8*2
};

static const gfx_layout layout_8x32x6 =
{
	8, 32,
	RGN_FRAC(1, 3),
	6,
	{ RGN_FRAC(0,3)+8, RGN_FRAC(0,3)+0,
		RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+0,
		RGN_FRAC(2,3)+8, RGN_FRAC(2,3)+0 },
	{ STEP8(0,1) },
	{ STEP32(0,2*8) },
	8*32*2
};

static const gfx_layout layout3s8_8x8x6 =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3)+2*8, RGN_FRAC(0,3)+0, RGN_FRAC(1,3)+2*8, RGN_FRAC(1,3)+0,RGN_FRAC(2,3)+2*8, RGN_FRAC(2,3)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 4*8, 5*8, 8*8, 9*8, 12*8, 13*8 },
	16*8
};

static GFXDECODE_START( gfx_spoker )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_8x8x6,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, layout_8x32x6, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, layout_8x32x6, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, layout_8x32x6, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, layout_8x32x6, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_3super8 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout3s8_8x8x6, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, layout_8x32x6,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, layout_8x32x6,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x0c000, layout_8x32x6,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, layout_8x32x6,   0, 16 )
GFXDECODE_END

// TODO: probably correct but to be verified once it passes the connection check
static const gfx_layout layout_8x8x4 =
{
	8, 8,
	RGN_FRAC(1, 1),
	4,
	{ STEP4(24, -8) },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*4) },
	8*8*4
};

static GFXDECODE_START( gfx_jinhulu2 )
	GFXDECODE_ENTRY( "tiles", 0, layout_8x8x4, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_jb )
	GFXDECODE_ENTRY( "gfx1", 0x00000, layout_8x8x6,    0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, layout_8x32x6,   0, 16 )
GFXDECODE_END

/***************************************************************************
                           Machine Start & Reset
***************************************************************************/

void spoker_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_nmi_ack));
	save_item(NAME(m_out));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_hopper));
	save_item(NAME(m_igs_magic));
}

void spoker_state::machine_reset()
{
	m_nmi_ack = 0;
	m_hopper = 0;
	m_video_enable = 1;
}


/***************************************************************************
                              Machine Drivers
***************************************************************************/

void spoker_state::spoker(machine_config &config)
{
	// basic machine hardware
	HD64180RP(config, m_maincpu, 12_MHz_XTAL);   // HD64180RP8, 8 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &spoker_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &spoker_state::spoker_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(spoker_state::nmi_line_assert));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));  // Control 0x8b --> A:out; B:input; C:input.
	ppi0.out_pa_callback().set(FUNC(spoker_state::nmi_and_coins_w));
	ppi0.in_pb_callback().set_ioport("SERVICE");
	ppi0.in_pc_callback().set_ioport("COINS");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));  // Control 0x90 --> A:input; B:out; C:out.
	ppi1.in_pa_callback().set_ioport("BUTTONS1");
	ppi1.out_pb_callback().set(FUNC(spoker_state::video_and_leds_w));
	ppi1.out_pc_callback().set(FUNC(spoker_state::leds_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-16-1);
	m_screen->set_screen_update(FUNC(spoker_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_spoker);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x400);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.4);

	OKIM6295(config, "oki", XTAL(12'000'000) / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void spokeru_state::spokeru(machine_config &config)
{
	spoker(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &spokeru_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &spokeru_state::portmap);
	m_maincpu->set_clock(12.288_MHz_XTAL);

	config.device_remove("ppi8255_0");
	config.device_remove("ppi8255_1");
}


void jinhulu2_state::jinhulu2(machine_config &config)
{
	spokeru(config);

	m_maincpu->set_addrmap(AS_IO, &jinhulu2_state::portmap);

	m_gfxdecode->set_info(gfx_jinhulu2);
}


void jb_state::jb(machine_config &config)
{
	spokeru(config);

	m_maincpu->set_addrmap(AS_IO, &jb_state::portmap);

	m_screen->set_screen_update(FUNC(jb_state::screen_update));

	m_gfxdecode->set_info(gfx_jb);
}


void spoker_state::_3super8(machine_config &config)
{
	spoker(config);

	Z80(config.replace(), m_maincpu, 24_MHz_XTAL / 4);    // z840006, 24/4 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &spoker_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &spoker_state::_3super8_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(spoker_state::nmi_line_assert));
	m_maincpu->set_periodic_int(FUNC(spoker_state::irq0_line_hold), attotime::from_hz(120)); // this signal comes from the PIC

	config.device_remove("ppi8255_0");
	config.device_remove("ppi8255_1");

	m_gfxdecode->set_info(gfx_3super8);
	m_palette->set_entries(0x800);

	config.device_remove("ymsnd");
}


/***************************************************************************
                                ROMs load
***************************************************************************/

/* Super Poker (IGS)
   US & UA versions.

   Original IGS boards
   IGS PCB-0308-01

   HD64180RP8 (u25)
   IGS026a (u10)
   IGS001a (u30)
   IGS002  (u20)
   K668    (u41) - Oki M6295 clone
   U3567   (u42) - YM2413 clone

   1x 12.288 MHz Xtal. (next to HD64180RP8)

   Programs are encrypted.
   See the last 0x1000 of each one
   to see the patterns...
*/
ROM_START( spk306us )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v306_us.u27",   0x0000, 0x10000, CRC(a6c6359e) SHA1(768c53e6c9a4d453e5342a169932fcc30b10fd04) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1_mx28f2000p_v306_us.u33",  0x00000, 0x40000, BAD_DUMP CRC(7ae9b639) SHA1(eb29acf94e96b5a8446dab1e46675766da0538f9) )
	ROM_LOAD( "2_mx28f2000p_v306_us.u32",  0x40000, 0x40000, BAD_DUMP CRC(3a9fc765) SHA1(10ccacf4da189f41b1c0fdc8d943b24ac3464e17) )
	ROM_LOAD( "3_mx28f2000p_v306_us.u31",  0x80000, 0x40000, BAD_DUMP CRC(71f6ea7a) SHA1(f91735d79af153cbbbe82312ba2af789b89c43dd) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mx28f2000p_v306_ussp.u34",   0x0000, 0x40000, BAD_DUMP CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk205us )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v205_us.u27",   0x0000, 0x10000, CRC(4b743c73) SHA1(bd05db13e27dc441e0483b883b770365b2702254) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1_mx28f2000p_v205_us.u33",  0x00000, 0x40000, CRC(7ae9b639) SHA1(eb29acf94e96b5a8446dab1e46675766da0538f9) )
	ROM_LOAD( "2_mx28f2000p_v205_us.u32",  0x40000, 0x40000, CRC(3a9fc765) SHA1(10ccacf4da189f41b1c0fdc8d943b24ac3464e17) )
	ROM_LOAD( "3_mx28f2000p_v205_us.u31",  0x80000, 0x40000, CRC(71f6ea7a) SHA1(f91735d79af153cbbbe82312ba2af789b89c43dd) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mx28f2000p_v205_ussp.u34",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk203us )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v203_us.u27",   0x0000, 0x10000, CRC(41328b3d) SHA1(2a4cf0cfdb09e72dabbaf09901cff222847c195a) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1_mx28f2000p_v203_us.u33",  0x00000, 0x40000, CRC(b309e9cf) SHA1(2e3f81c9c654c859c0fd4c0953302c9283e7a4d8) )
	ROM_LOAD( "2_mx28f2000p_v203_us.u32",  0x40000, 0x40000, CRC(05048307) SHA1(38d5ba5522a60ae4f34731ea7bd3e2c16683125d) )
	ROM_LOAD( "3_mx28f2000p_v203_us.u31",  0x80000, 0x40000, CRC(beae217b) SHA1(9bfa69954c42ada88bedb7cedaceff841cb88a58) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mx28f2000p_v203_ussp.u34",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk201ua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v201_ua.u27",   0x0000, 0x10000, CRC(c9186a07) SHA1(b62459affa7ade023d7a5ea97289d1a1474ad966) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1_mx28f2000p_v200_ua.u33",  0x00000, 0x40000, CRC(b309e9cf) SHA1(2e3f81c9c654c859c0fd4c0953302c9283e7a4d8) )
	ROM_LOAD( "2_mx28f2000p_v200_ua.u32",  0x40000, 0x40000, CRC(05048307) SHA1(38d5ba5522a60ae4f34731ea7bd3e2c16683125d) )
	ROM_LOAD( "3_mx28f2000p_v200_ua.u31",  0x80000, 0x40000, CRC(beae217b) SHA1(9bfa69954c42ada88bedb7cedaceff841cb88a58) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mx28f2000p_v200_uasp.u34",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk200ua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v200_ua.u27",   0x0000, 0x10000, CRC(f4572b88) SHA1(b1f845b5340639eee1464acb8a40241868a21070) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1_mx28f2000p_v200_ua.u33",  0x00000, 0x40000, CRC(b309e9cf) SHA1(2e3f81c9c654c859c0fd4c0953302c9283e7a4d8) )
	ROM_LOAD( "2_mx28f2000p_v200_ua.u32",  0x40000, 0x40000, CRC(05048307) SHA1(38d5ba5522a60ae4f34731ea7bd3e2c16683125d) )
	ROM_LOAD( "3_mx28f2000p_v200_ua.u31",  0x80000, 0x40000, CRC(beae217b) SHA1(9bfa69954c42ada88bedb7cedaceff841cb88a58) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mx28f2000p_v200_uasp.u34",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk200 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u44",   0x0000, 0x10000, CRC(75dac1bd) SHA1(06cbed9cf9956af47aadb44e467c1952f16cc3e5) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "u25",  0x00000, 0x40000, CRC(7cf1327b) SHA1(afeaa975e63cff2f6ae52650cf77b125593efbbc) )
	ROM_LOAD( "u24",  0x40000, 0x40000, CRC(15095525) SHA1(480c17eec6079f7bf12026a9bb1c14fb52db8d45) )
	ROM_LOAD( "u23",  0x80000, 0x40000, CRC(b1538860) SHA1(0de12b91db25281025059429fac07d235071ea21) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u38",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk130 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u44",   0x0000, 0x10000, CRC(1a76b7f1) SHA1(6d83b0841991bc86179b98267ecb5949641932fe) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "u25",  0x00000, 0x40000, CRC(7cf1327b) SHA1(afeaa975e63cff2f6ae52650cf77b125593efbbc) )
	ROM_LOAD( "u24",  0x40000, 0x40000, CRC(15095525) SHA1(480c17eec6079f7bf12026a9bb1c14fb52db8d45) )
	ROM_LOAD( "u23",  0x80000, 0x40000, CRC(b1538860) SHA1(0de12b91db25281025059429fac07d235071ea21) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u38",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk102ua )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27c512_v102_ua.u27",   0x0000, 0x10000, CRC(ec5e9f6d) SHA1(5d7a86f8faef7a4b7a9dde040b00b987ffb09479) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1_mx28f2000p_v102_ua.u33",  0x00000, 0x40000, CRC(b309e9cf) SHA1(2e3f81c9c654c859c0fd4c0953302c9283e7a4d8) )
	ROM_LOAD( "2_mx28f2000p_v102_ua.u32",  0x40000, 0x40000, CRC(05048307) SHA1(38d5ba5522a60ae4f34731ea7bd3e2c16683125d) )
	ROM_LOAD( "3_mx28f2000p_v102_ua.u31",  0x80000, 0x40000, CRC(beae217b) SHA1(9bfa69954c42ada88bedb7cedaceff841cb88a58) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "mx28f2000p_v102_uasp.u34",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk102u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u44", 0x0000, 0x10000, CRC(51e392e7) SHA1(da76cdfbdc84234ee6d3dd360871d892a8a1c5e2) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "u25",  0x00000, 0x40000, CRC(67f2a1d6) SHA1(115655cf4718105e7ff054dd662d9d53e9ea91e0) )
	ROM_LOAD( "u24",  0x40000, 0x40000, CRC(fb9d8c09) SHA1(6cbefa90f6b866ee682f7981d6f5d30e9346a123) )
	ROM_LOAD( "u23",  0x80000, 0x40000, CRC(98b71478) SHA1(b5bd9eef91f5cc2e9628e5181fbbd6fd453487f0) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u38",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( spk100 ) // no labels on the ROMs. This version has also been found with GFX ROMs from the spk200 set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u43",   0x0000, 0x10000, CRC(7c17bf58) SHA1(dd16b9f52d8c08a61fe234978cc84b95c25c5dec) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "u25",  0x00000, 0x40000, CRC(67f2a1d6) SHA1(115655cf4718105e7ff054dd662d9d53e9ea91e0) )
	ROM_LOAD( "u24",  0x40000, 0x40000, CRC(fb9d8c09) SHA1(6cbefa90f6b866ee682f7981d6f5d30e9346a123) )
	ROM_LOAD( "u23",  0x80000, 0x40000, CRC(98b71478) SHA1(b5bd9eef91f5cc2e9628e5181fbbd6fd453487f0) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                              0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "u38",   0x0000, 0x40000, CRC(33e6089d) SHA1(cd1ad01e92c18bbeab3fe3ea9152f8b0a3eb1b29) )
ROM_END

ROM_START( jbell ) // IGS PCB-0308-01, same as the US Super Poker sets
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "jingle_v200us.u27",   0x00000, 0x10000, CRC(831782cb) SHA1(8e903569a0cd34222a7e5fd3e9ab21bf00802bb3) ) // 27C512
	ROM_LOAD( "jingle_v200us_7.u26", 0x10000, 0x10000, CRC(3160f443) SHA1(2930359961a618cbe356a7f9c8fd63bacba2a6b0) ) // 27C512, 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0xc0000, "gfx1", 0 ) // all 27C020
	ROM_LOAD( "jingle_v200us_4.u33",  0x00000, 0x40000, CRC(daa56ce5) SHA1(4f14a8efac16b03bd14dd26d586bcb8d5bef65c1) )
	ROM_LOAD( "jingle_v200us_5.u32",  0x40000, 0x40000, CRC(b10b38e1) SHA1(397b2d899e47c6249fbbb6e6262d0390d9b796e6) )
	ROM_LOAD( "jingle_v200us_6.u31",  0x80000, 0x40000, CRC(a3304b5a) SHA1(bf51cb1f728758d50ce27275aa19ef649f6b34b9) )

	ROM_REGION( 0x30000, "gfx2", 0 ) // all 27C512
	ROM_LOAD( "jingle_v200us_1.u23",  0x00000, 0x10000, CRC(cadd7910) SHA1(aa514ddb29c8c9a77478d56bea4ae71995fdd518) )
	ROM_LOAD( "jingle_v200us_2.u22",  0x10000, 0x10000, CRC(a9e1f5aa) SHA1(68d7f4e9e9a5bbce0904e406ee6fe82e9e52a9ba) )
	ROM_LOAD( "jingle_v200us_3.u21",  0x20000, 0x10000, CRC(865b7d3a) SHA1(c1dff3a27d747ee499aaee0c4468534f0249a3e5) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "jingle_v200ussp.u34", 0x00000, 0x20000, CRC(a42d73b1) SHA1(93157e9630d5c8bb34c71186415d0aa8c5d51951) ) // 27C010
ROM_END

/*
   Super Poker
   Italian sets...
*/

ROM_START( spk120in ) // IGS PCB NO-0102-6. Has a IGS009 instead of the IGS001/2 and PPI instead of the IGS026A.
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super_v120_8.u44", 0x00000, 0x10000, CRC(321c0971) SHA1(ee556b8d883c870919e54b4cfd82d5af1cac8718) ) // MX26C512
	// u43 not populated

	ROM_REGION( 0xc0000, "gfx1", 0 ) // all TMS27C020
	ROM_LOAD( "super_v120in_6.u23", 0x80000, 0x40000, CRC(55b54b11) SHA1(decf27d40ec842374af02c93d761375690be83a3) )
	ROM_LOAD( "super_v120in_5.u24", 0x40000, 0x40000, CRC(163f5b64) SHA1(5d3a5c2a64691ee9e2bb3a7c283aa9efa53fb35e) )
	ROM_LOAD( "super_v120in_4.u25", 0x00000, 0x40000, CRC(ec2c6ac3) SHA1(e0a38da26202d2b9a481060fe5b88a38e284201e) )

	ROM_REGION( 0x30000, "gfx2", 0 ) // all MX26C512
	ROM_LOAD( "super_v120_3.u15", 0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "super_v120_2.u16", 0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "super_v120_1.u17", 0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 ) // TMS27C020
	ROM_LOAD( "super_v120insp.u38", 0x00000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END

ROM_START( spk116it )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v.bin",   0x0000, 0x10000, CRC(e44e943a)  SHA1(78e32d07e2be9a452be10735641cbcf269068c55) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6_v116it.bin",  0x80000, 0x40000, CRC(55b54b11) SHA1(decf27d40ec842374af02c93d761375690be83a3) )
	ROM_LOAD( "5_v116it.bin",  0x40000, 0x40000, CRC(163f5b64) SHA1(5d3a5c2a64691ee9e2bb3a7c283aa9efa53fb35e) )
	ROM_LOAD( "4_v116it.bin",  0x00000, 0x40000, CRC(ec2c6ac3) SHA1(e0a38da26202d2b9a481060fe5b88a38e284201e) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3_v116it.bin",  0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "2_v116it.bin",  0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "1_v116it.bin",  0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7_v116it.bin",   0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END

ROM_START( spk116itmx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v_v116it_mx.bin",  0x0000, 0x10000, CRC(f84eefe3) SHA1(9b3acde4f8f5f635155ed132c10d11bf609da2dd) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6_v116itmx.bin",  0x80000, 0x40000, CRC(55b54b11) SHA1(decf27d40ec842374af02c93d761375690be83a3) )
	ROM_LOAD( "5_v116itmx.bin",  0x40000, 0x40000, CRC(163f5b64) SHA1(5d3a5c2a64691ee9e2bb3a7c283aa9efa53fb35e) )
	ROM_LOAD( "4_v116itmx.bin",  0x00000, 0x40000, CRC(ec2c6ac3) SHA1(e0a38da26202d2b9a481060fe5b88a38e284201e) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3_v116itmx.bin",  0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "2_v116itmx.bin",  0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "1_v116itmx.bin",  0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7_v116itmx.bin",   0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END

ROM_START( spk115it )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v_v115it.bin",   0x0000, 0x10000, CRC(df52997b) SHA1(72a76e84aeedfdebd4c6cb47809117a28b5d3892) ) // sldh

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6_v115it.bin",  0x80000, 0x40000, CRC(f9b027f8) SHA1(c4686a4024062482f9864e0445087e32899fc775) ) // sldh
	ROM_LOAD( "5_v115it.bin",  0x40000, 0x40000, CRC(baca51b6) SHA1(c97322c814729332378b6304a79062fea385ca97) ) // sldh
	ROM_LOAD( "4_v115it.bin",  0x00000, 0x40000, CRC(1172c790) SHA1(43f1d019ecae5c605722e3fe77ae2f022b01260b) ) // sldh

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_LOAD( "3_v115it.bin",  0x20000, 0x10000, CRC(5f18b012) SHA1(c9a96237eaf3138f136bbaffb29dde0ef568ce73) )
	ROM_LOAD( "2_v115it.bin",  0x10000, 0x10000, CRC(50fc3505) SHA1(ca1e4ee7e0bb59c3bd67727f65054a48000ae7fe) )
	ROM_LOAD( "1_v115it.bin",  0x00000, 0x10000, CRC(28ce630a) SHA1(9b597073d33841e7db2c68bbe9f30b734d7f7b41) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "7_v115it.bin",   0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END

ROM_START( spk114it )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "v_v114it.bin",  0x0000, 0x10000, CRC(471f5d97) SHA1(94781fb7bd8e9633fafca72de41113c3f21fbf4e) )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "6_v114it.bin",  0x80000, 0x40000, CRC(466cec74) SHA1(6e4fe56a55f1e230eb01266c67cc82a87f6aaec6) )
	ROM_LOAD( "5_v114it.bin",  0x40000, 0x40000, CRC(ae32a620) SHA1(d5238628269050e21da31d628b1d80b0f9b2d251) )
	ROM_LOAD( "4_v114it.bin",  0x00000, 0x40000, CRC(c2105b1c) SHA1(cd2a930e9c15d1fdcc02ce87c109b5b4107430fa) )

	ROM_REGION( 0x30000, "gfx2", 0 )
	ROM_FILL(                  0x0000, 0x30000, 0xff ) // filling the whole bank

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "7_v114it.bin",  0x0000, 0x40000, CRC(67789f1c) SHA1(1bef621b4d6399f76020c6310e2e1c2f861679de) )
ROM_END


/*

Produttore  ?Italy?
N.revisione
CPU

1x 24mhz osc
2x fpga
1x z840006
1x PIC16c65a-20/p
1x 6295 oki

ROMs

Note

4x 8 dipswitch
1x 4 dispwitch

*/

// All gfx / sound ROMs are bad.  They're definitely meant to have different data
//  in each half, and maybe even be twice the size.
//  In all cases the first half is missing (the sample table in the samples ROM for example)
//1.bin                                           1ST AND 2ND HALF IDENTICAL
//2.bin                                           1ST AND 2ND HALF IDENTICAL
//3.bin                                           1ST AND 2ND HALF IDENTICAL
//sound.bin                                       1ST AND 2ND HALF IDENTICAL

ROM_START( 3super8 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "prgrom.bin", 0x00000, 0x20000, CRC(37c85dfe) SHA1(56bd2fb859b17dda1e675a385b6bcd6867ecceb0)  )

	ROM_REGION( 0x1000, "pic", 0 )
	ROM_LOAD( "pic16c65a-20-p", 0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0xc0000, "gfx1", 0 )
	ROM_LOAD( "1.bin", 0x00000, 0x40000, BAD_DUMP CRC(d9d3e21e) SHA1(2f3f07ca427d9f56f0ff143d15d95cbf15255e33) ) // sldh
	ROM_LOAD( "2.bin", 0x40000, 0x40000, BAD_DUMP CRC(fbb50ab1) SHA1(50a7ef9219c38d59117c510fe6d53fb3ba1fa456) ) // sldh
	ROM_LOAD( "3.bin", 0x80000, 0x40000, BAD_DUMP CRC(545aa4e6) SHA1(3348d4b692900c9e9cd4a52b20922a84e596cd35) ) // sldh
	ROM_FILL( 0x00000 ,0x20000, 0x00 )
	ROM_FILL( 0x40000 ,0x20000, 0x00 )
	ROM_FILL( 0x80000 ,0x20000, 0x00 )

	ROM_REGION( 0x30000, "gfx2", ROMREGION_ERASE00 )

	ROM_REGION( 0xc0000, "rep_gfx", 0 ) // not real, taken from spk116it
	ROM_LOAD( "4.bin",  0x00000, 0x40000, BAD_DUMP CRC(ec2c6ac3) SHA1(e0a38da26202d2b9a481060fe5b88a38e284201e) )
	ROM_LOAD( "5.bin",  0x40000, 0x40000, BAD_DUMP CRC(163f5b64) SHA1(5d3a5c2a64691ee9e2bb3a7c283aa9efa53fb35e) )
	ROM_LOAD( "6.bin",  0x80000, 0x40000, BAD_DUMP CRC(55b54b11) SHA1(decf27d40ec842374af02c93d761375690be83a3) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sound.bin", 0x00000, 0x40000, BAD_DUMP CRC(230b31c3) SHA1(38c107325d3a4e9781912078b1317dc9ba3e1ced) )
ROM_END

/*********************************************************************************

Jin Hu Lu 2, IGS (year unknown, board dead/rotten, battery leaked EXTENSIVELY and killed it)
This is a Poker game.
Board is a mix of through-hole and surface mounted parts (lots of logic is SMD).
Some chips have year 1999 so this is 1999 or later.

PCB Layout
----------

IGS PCB No- 0202 - :
|--------------------------------------------|
|SW4    BATTERY  12MHz                 Z180  |
|                                            |
|1                                           |
|8       T518B                        PRG.U40|
|W                              PAL          |
|A                              PAL     6264 |
|Y          2149C               PAL          |
|                               PAL     SKT  |
|           IGS-003C                         |
|                                            |
|1                                           |
|0                                   IGS002  |
|W                                           |
|A                      IGS001A              |
|Y VOL     6264                        6264  |
|  UPC1242        M6295           27C4002.U39|
|7805    SP_U12.U12        SW1  SW2  SW3     |
|--------------------------------------------|
Notes:
      2149C - Might be an 8255 PPI. Definitely not a YM2149 ;-)
       Z180 - Zilog Z8018008PSC Z180 MPU. Chip rated at 8MHz so clock input likely to be 6MHz [12/2]
    IGS003C - In a socket but marked on PCB as 'ASIC3' (which is unusual) so could be a custom chip and not a ROM?
   IGS001/2 - Custom IGS Chip (QFP80)
       6264 - 8kBx8-bit SRAM
        SKT - Empty socket, missing PAL maybe?
      M6295 - Oki Sound Chip. Clock input possibly 1.000MHz [12/12]
      T518B - Reset Chip
    SW1/2/3 - 8-Position DIP Switch
 SP_U12.U12 - OKI Samples. Board printed '27C020' but actual chip is Intel FLASH P28F001

*********************************************************************************/

ROM_START( jinhulu2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "prg.u40", 0x00000, 0x10000, CRC(e7974ae0) SHA1(1240fabeb79bb820ba371b378ad51660170c6cd0) )

	ROM_REGION( 0x40000, "tiles", 0 )
	ROM_LOAD( "27c4002.u39", 0x00000, 0x40000, CRC(b933ec01) SHA1(72b541579551114f7c8649c2e9a839ef4128fc14) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                       0x40000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sp_u12.u12", 0x00000, 0x20000, CRC(1aeb078c) SHA1(9b8a256f51e66733c4ec30b451ca0711ed02318e) )
ROM_END

/***************************************************************************
                              Driver Init
***************************************************************************/

void spokeru_state::init_spokeru()
{
/*  The last 4K have the scheme/table for the whole encryption.
    Maybe a leftover...
*/
	uint8_t *rom = memregion("maincpu")->base();
	for (int a = 0; a < 0x10000; a++)
	{
		int b = ((a & 0x0fff) | 0xf000);
		rom[a] = rom[a] ^ rom[b];
	}
}

void jinhulu2_state::init_jinhulu2()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int a = 0; a < 0xf000; a++)
	{
		rom[a] ^= 0x01;
		if ((a & 0x0282) == 0x0282) rom[a] ^= 0x01;
		if ((a & 0x0940) == 0x0940) rom[a] ^= 0x02;
	}

	const int rom_size = memregion("tiles")->bytes();
	u8 * const gfxrom = memregion("tiles")->base();
	std::unique_ptr<u8[]> tmp = std::make_unique<u8[]>(rom_size);

	// address lines swap
	memcpy(tmp.get(), gfxrom, rom_size);
	for (int i = 0; i < rom_size; i++)
	{
		// TODO: may need some higher bits swapped, too. To be verified once it passes the connection check
		int addr = bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 6, 11, 8, 9, 10, 7, 12, 5, 4, 3, 2, 1, 0);
		gfxrom[i] = tmp[addr];
	}

}

void spoker_state::init_spk116it()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int a = 0; a < 0x10000; a++)
	{
		rom[a] ^= 0x02;
		if ((a & 0x0208) == 0x0208) rom[a] ^= 0x20;
		if ((a & 0x0228) == 0x0008) rom[a] ^= 0x20;
		if ((a & 0x04a0) == 0x04a0) rom[a] ^= 0x02;
		if ((a & 0x1208) == 0x1208) rom[a] ^= 0x01;
	}
}

void spoker_state::init_spk114it()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int a = 0; a < 0x10000; a++)
	{
		rom[a] ^= 0x02;
		if ((a & 0x0120) == 0x0020) rom[a] ^= 0x20;
		if ((a & 0x04a0) == 0x04a0) rom[a] ^= 0x02;
		if ((a & 0x1208) == 0x1208) rom[a] ^= 0x01;
	}
}

void spoker_state::init_spk100()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int a = 0; a < 0x10000; a++)
	{
		rom[a] ^= 0x22;
		if ((a & 0x0090) == 0x0080) rom[a] ^= 0x20;
		if ((a & 0x04a0) == 0x04a0) rom[a] ^= 0x02;
		if ((a & 0x1208) == 0x1208) rom[a] ^= 0x01;
	}
}

void spoker_state::init_spk120in()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int a = 0; a < 0x10000; a++)
	{
		rom[a] ^= 0x02;
		if ((a & 0x0208) == 0x0208) rom[a] ^= 0x20;
		if ((a & 0x0248) == 0x0008) rom[a] ^= 0x20;
		if ((a & 0x04a0) == 0x04a0) rom[a] ^= 0x02;
		if ((a & 0x1208) == 0x1208) rom[a] ^= 0x01;
	}
}

void spoker_state::init_3super8()
{
	uint8_t *rom = memregion("maincpu")->base();

	/* Decryption is probably done using one macrocell/output on an address decoding pal which we do not have a dump of
	   The encryption is quite awful actually, especially since the program ROM is entirely blank/0xFF but encrypted on its second half, exposing the entire function in plaintext
	   Input: A6, A7, A8, A9, A11; Output: D5 XOR
	   function: (A6&A8)&((!A7&A11)|(A9&!A11));
	   nor-reduced: !(!(!(!A6|!A8))|!(!(A7|!A11)|!(!A9|A11))); */
	for (int i = 0; i < 0x20000; i++)
	{
		uint8_t a6  = BIT(i, 6);
		uint8_t a7  = BIT(i, 7);
		uint8_t a8  = BIT(i, 8);
		uint8_t a9  = BIT(i, 9);
		uint8_t a11 = BIT(i, 11);
		uint8_t d5 = (a6 & a8) & ((~a7 & a11) | (a9 & ~a11));
		rom[i] ^= d5 * 0x20;
	}

	// cheesy hack: take gfx ROMs from spk116it and rearrange them for this game needs
	{
		uint8_t *src = memregion("rep_gfx")->base();
		uint8_t *dst = memregion("gfx1")->base();

		for (uint8_t x = 0; x < 3; x++)
		{
			for (int i = 0; i < 0x20000; i += 4)
			{
				dst[i + 0 + x * 0x40000] = src[i + 0 + x * 0x40000];
				dst[i + 1 + x * 0x40000] = src[i + 2 + x * 0x40000];
				dst[i + 2 + x * 0x40000] = src[i + 1 + x * 0x40000];
				dst[i + 3 + x * 0x40000] = src[i + 3 + x * 0x40000];
			}
		}
	}
}

} // anonymous namespace


/***************************************************************************
                              Game Drivers
***************************************************************************/

//    YEAR   NAME        PARENT    MACHINE   INPUT     STATE           INIT           ROT    COMPANY      FULLNAME                    FLAGS
GAME( 1996,  spk306us,   0,        spokeru,  spoker,   spokeru_state,  init_spokeru,  ROT0,  "IGS",       "Super Poker (v306US)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk205us,   spk306us, spokeru,  spoker,   spokeru_state,  init_spokeru,  ROT0,  "IGS",       "Super Poker (v205US)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk203us,   spk306us, spokeru,  spk203us, spokeru_state,  init_spokeru,  ROT0,  "IGS",       "Super Poker (v203US)",     MACHINE_SUPPORTS_SAVE ) // LS1. 8 203US in test mode
GAME( 1996,  spk201ua,   spk306us, spokeru,  spk201ua, spokeru_state,  init_spokeru,  ROT0,  "IGS",       "Super Poker (v201UA)",     MACHINE_SUPPORTS_SAVE ) // still shows 200UA in test mode
GAME( 1996,  spk200ua,   spk306us, spokeru,  spk200ua, spokeru_state,  init_spokeru,  ROT0,  "IGS",       "Super Poker (v200UA)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk200,     spk306us, spoker,   spk100,   spoker_state,   init_spk100,   ROT0,  "IGS",       "Super Poker (v200)",       MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk130,     spk306us, spoker,   spk130,   spoker_state,   init_spk100,   ROT0,  "IGS",       "Super Poker (v130)",       MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk120in,   spk306us, spoker,   spoker,   spoker_state,   init_spk120in, ROT0,  "IGS",       "Super Poker (v120IN)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk116it,   spk306us, spoker,   spoker,   spoker_state,   init_spk116it, ROT0,  "IGS",       "Super Poker (v116IT)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk116itmx, spk306us, spoker,   spoker,   spoker_state,   init_spk114it, ROT0,  "IGS",       "Super Poker (v116IT-MX)",  MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk115it,   spk306us, spoker,   spoker,   spoker_state,   init_spk116it, ROT0,  "IGS",       "Super Poker (v115IT)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk114it,   spk306us, spoker,   spk114it, spoker_state,   init_spk114it, ROT0,  "IGS",       "Super Poker (v114IT)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk102ua,   spk306us, spokeru,  spk102ua, spokeru_state,  init_spokeru,  ROT0,  "IGS",       "Super Poker (v102UA)",     MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk102u,    spk306us, spoker,   spk102ua, spoker_state,   init_spk100,   ROT0,  "IGS",       "Super Poker (v102U)",      MACHINE_SUPPORTS_SAVE )
GAME( 1996,  spk100,     spk306us, spoker,   spk100,   spoker_state,   init_spk100,   ROT0,  "IGS",       "Super Poker (v100)",       MACHINE_SUPPORTS_SAVE )
GAME( 1993?, 3super8,    0,        _3super8, 3super8,  spoker_state,   init_3super8,  ROT0,  "<unknown>", "3 Super 8 (Italy)",        MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // ROMs are badly dumped
GAME( 1997,  jbell,      0,        jb,       jb,       jb_state,       init_spokeru,  ROT0,  "IGS",       "Jingle Bell (v200US)",     MACHINE_SUPPORTS_SAVE )
GAME( 1995,  jinhulu2,   0,        jinhulu2, spoker,   jinhulu2_state, init_jinhulu2, ROT0,  "IGS",       "Jin Hu Lu 2 (v412GS)",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // tries to link to something?
