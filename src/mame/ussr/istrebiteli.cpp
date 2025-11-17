// license:BSD-3-Clause
// copyright-holders:MetalliC
/*************************************************************************

    Istrebiteli driver by MetalliC

    TODO:
      hardware-like noise sound generation
      accurate sprite collision

    how to play:
      insert one or more coins, each coin gives 2 minutes of play time, then press 1 or 2 player game start
      hit enemy 15 or more times to get bonus game

    test mode:
      insert 12 or more coins then press 2 player start

    notes:
      dumped PCB is early game version, has several bugs, possible test/prototype.
      later version was seen in St.Petersburg arcade museum, CPU board has single 8Kx8 ROM.

**************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#define I8080_TAG   "maincpu"

/////////////////////////////////////////////////////////////

class istrebiteli_sound_device : public device_t, public device_sound_interface
{
public:
	template <typename T> istrebiteli_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&region_tag)
		: istrebiteli_sound_device(mconfig, tag, owner, clock)
	{
		m_rom.set_tag(std::forward<T>(region_tag));
	}

	istrebiteli_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void sound_w(uint8_t data);

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// internal state
	sound_stream *m_channel;
	required_region_ptr<uint8_t> m_rom;
	uint16_t m_rom_cnt;
	uint8_t m_rom_incr;
	uint8_t m_sample_num;
	bool m_cnt_reset;
	bool m_rom_out_en;
	uint8_t m_prev_data;
};

DECLARE_DEVICE_TYPE(ISTREBITELI_SOUND, istrebiteli_sound_device)

//////////////////////////////////////////////////////////////

DEFINE_DEVICE_TYPE(ISTREBITELI_SOUND, istrebiteli_sound_device, "istrebiteli_sound", "Istrebiteli Sound")

istrebiteli_sound_device::istrebiteli_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISTREBITELI_SOUND, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_channel(nullptr),
		m_rom(*this, finder_base::DUMMY_TAG),
		m_rom_cnt(0),
		m_rom_incr(0),
		m_sample_num(0),
		m_cnt_reset(true),
		m_rom_out_en(false),
		m_prev_data(0)
{
}

void istrebiteli_sound_device::device_start()
{
	m_channel = stream_alloc(0, 1, clock() / 2);

	save_item(NAME(m_rom_cnt));
	save_item(NAME(m_rom_incr));
	save_item(NAME(m_sample_num));
	save_item(NAME(m_cnt_reset));
	save_item(NAME(m_rom_out_en));
	save_item(NAME(m_prev_data));
}

void istrebiteli_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int sampindex = 0; sampindex < stream.samples(); sampindex++)
	{
		int smpl = 0;
		if (m_rom_out_en)
			smpl = (m_rom[m_rom_cnt] >> m_sample_num) & 1;

		// below is huge guess
		if ((m_prev_data & 0x40) == 0)              // b6 noise enable ?
			smpl &= machine().rand() & 1;
		smpl *= (m_prev_data & 0x80) ? 1000 : 4000; // b7 volume ?

		stream.put_int(0, sampindex, smpl, 32768);
		m_rom_cnt = (m_rom_cnt + m_rom_incr) & 0x1ff;
	}
}

void istrebiteli_sound_device::sound_w(uint8_t data)
{
	m_cnt_reset = (data & 2) ? true : false;
	m_sample_num = (data >> 2) & 7;
	m_rom_out_en = (data & 0x20) ? false : true;

	if (m_cnt_reset)
	{
		m_rom_cnt = 0;
		m_rom_incr = 0;
	}
	else
		m_rom_incr = 1;
//  if (m_prev_data != data)
//      printf("sound %02X rescnt %d sample %d outen %d b6 %d b7 %d\n", data, (data >> 1) & 1, (data >> 2) & 7, (data >> 5) & 1, (data >> 6) & 1, (data >> 7) & 1);
	m_prev_data = data;
}

//////////////////////////////////////////////////////////////

namespace {

class istrebiteli_state : public driver_device
{
public:
	istrebiteli_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8080_TAG)
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_gfxdecode(*this, "gfxdecode")
		, m_sound_dev(*this, "custom")
	{
	}

	void init_istreb();
	void init_moto();

	void istreb(machine_config &config);
	void motogonki(machine_config &config);

	template <int ID> int collision_r();
	ioport_value coin_r();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inc);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void istrebiteli_palette(palette_device &palette) const;
	void motogonki_palette(palette_device &palette) const;
	uint8_t ppi0_r(offs_t offset);
	void ppi0_w(offs_t offset, uint8_t data);
	uint8_t ppi1_r(offs_t offset);
	void ppi1_w(offs_t offset, uint8_t data);
	void sound_w(uint8_t data);
	void spr0_ctrl_w(uint8_t data);
	void spr1_ctrl_w(uint8_t data);
	void spr_xy_w(offs_t offset, uint8_t data);
	void moto_spr_xy_w(offs_t offset, uint8_t data);
	void tileram_w(offs_t offset, uint8_t data);
	void moto_tileram_w(offs_t offset, uint8_t data);
	void road_ctrl_w(uint8_t data);
	DECLARE_VIDEO_START(moto);

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<istrebiteli_sound_device> m_sound_dev;

	TILE_GET_INFO_MEMBER(get_tile_info);
	tilemap_t *m_tilemap = nullptr;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t moto_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t coin_count = 0;
	uint8_t m_spr_ctrl[2]{};
	uint8_t m_spr_collision[2]{};
	uint8_t m_spr_xy[8]{};
	uint8_t m_tileram[16]{};
	uint8_t m_road_scroll = 0;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void moto_io_map(address_map &map) ATTR_COLD;
	void moto_mem_map(address_map &map) ATTR_COLD;
};

void istrebiteli_state::machine_start()
{
	save_item(NAME(coin_count));
	save_item(NAME(m_spr_ctrl));
	save_item(NAME(m_spr_collision));
	save_item(NAME(m_spr_xy));
	save_item(NAME(m_tileram));
}

void istrebiteli_state::machine_reset()
{
	coin_count = 0;
	memset(m_spr_ctrl, 0, sizeof(m_spr_ctrl));
	memset(m_spr_collision, 0, sizeof(m_spr_collision));
	memset(m_spr_xy, 0, sizeof(m_spr_xy));
	memset(m_tileram, 0, sizeof(m_tileram));
}

static const rgb_t istreb_palette[4] = {
		rgb_t(0x00, 0x00, 0x00),
		rgb_t(0x00, 0x00, 0xff),
		rgb_t(0xff, 0xff, 0xff),
		rgb_t(0x00, 0x00, 0xff) };

void istrebiteli_state::istrebiteli_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, istreb_palette);
}

static const rgb_t moto_palette[4] = {
		rgb_t(0x00, 0x00, 0x00),
		rgb_t(0x00, 0x00, 0xff),
		rgb_t(0xff, 0xff, 0xff),
		rgb_t(0x00, 0x00, 0x00) };

void istrebiteli_state::motogonki_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, moto_palette);
}

TILE_GET_INFO_MEMBER(istrebiteli_state::get_tile_info)
{
	tileinfo.set(0, m_tileram[tile_index] & 0x1f, 0, 0);
}

void istrebiteli_state::init_istreb()
{
	uint8_t *gfx = memregion("sprite")->base();
	uint8_t temp[64];

	for (int offs = 0; offs < 0x200; offs += 0x40)
	{
		memset(&temp[0], 0, sizeof(temp));
		for (int spnum = 0; spnum < 8; spnum++)
			for (int dot = 0; dot < 64; dot++)
				temp[(dot >> 3) + spnum * 8] |= ((gfx[offs + dot] >> spnum) & 1) << (dot & 7);
		memcpy(&gfx[offs], &temp[0], sizeof(temp));
	}
}

void istrebiteli_state::init_moto()
{
	uint8_t *gfx = memregion("sprite")->base();
	uint8_t temp[256];

	for (int offs = 0; offs < 0x600; offs += 0x100)
	{
		memset(&temp[0], 0, sizeof(temp));
		for (int spnum = 0; spnum < 8; spnum++)
			for (int dot = 0; dot < 256; dot++)
				temp[(dot >> 3) + spnum * 32] |= ((gfx[offs + dot] >> spnum) & 1) << (dot & 7);
		memcpy(&gfx[offs], &temp[0], sizeof(temp));
	}
}

void istrebiteli_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(istrebiteli_state::get_tile_info)), TILEMAP_SCAN_ROWS,
		8, 16, 16, 1);
	m_tilemap->set_scrolldx(96, 96);
}

VIDEO_START_MEMBER(istrebiteli_state, moto)
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(istrebiteli_state::get_tile_info)), TILEMAP_SCAN_ROWS,
		8, 16, 16, 1);
	m_tilemap->set_scrolldx(96, 96);
	m_tilemap->set_scrolldy(8, 8);
}

uint32_t istrebiteli_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(1);

	rectangle rect = cliprect;
	rect.offset(32, 64);
	rect.set_size(16*8, 16);
	m_tilemap->draw(screen, bitmap, rect, 0, 0);

	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, (m_spr_ctrl[0] & 0x40) ? 5 : 7, 0, 0, 0, m_spr_xy[4], m_spr_xy[5], 1);
	m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, (m_spr_ctrl[1] & 0x40) ? 4 : 6, 0, 0, 0, m_spr_xy[6], m_spr_xy[7], 1);

	int spritecode;

	spritecode = (m_spr_ctrl[0] & 0x1f) + ((m_spr_ctrl[0] & 0x80) >> 2);
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, spritecode, 0, 0, 0, m_spr_xy[0], m_spr_xy[1], 1);
	spritecode = (m_spr_ctrl[1] & 0x1f) + ((m_spr_ctrl[1] & 0x80) >> 2);
	m_gfxdecode->gfx(2)->transpen(bitmap, cliprect, spritecode, 0, 0, 0, m_spr_xy[2], m_spr_xy[3], 1);

	return 0;
}

uint32_t istrebiteli_state::moto_screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0);

	// port 38 m_spr_ctrl[0] bits
	//   0123 - sprite 1 (player) idx
	//   4    - road row scroll enable
	//   5    - ? 1 during game mode, 0 in atract |
	//   6    - ? 1 during atract mode, 0 in game | one of these is coin lock, other unknown
	//   7    - sprites/road enable
	// port 39 m_spr_ctrl[1] bits
	//   0123 - sprite 2 idx
	//   4567 - sprite 3 idx
	// port 3d road row scroll decrement, 1 or 2 (starts at m_spr_xy[5] line, during 16 lines which makes road curve)

	// temp / debug code
	// out of road space should be filled with 1pix grey 1pix black pattern, road area is grey, mid road lines is white, all sprites is black.
	rectangle rect = cliprect;
	rect.offsetx(64 - (s8)m_spr_xy[4]);
	rect.set_width(64);
	bitmap.fill(1, rect);

	int spritecode0 = ((m_spr_ctrl[0] >> 1) & 7) + ((m_spr_ctrl[0] << 3) & 8);
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, spritecode0, 0, 0, 0, m_spr_xy[0], m_spr_xy[1], 1);
	int spritecode1 = ((m_spr_ctrl[1] >> 1) & 7) + ((m_spr_ctrl[1] << 3) & 8);
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, spritecode1 + 16, 0, 0, 0, m_spr_xy[2], m_spr_xy[3], 1);
	int spritecode2 = ((m_spr_ctrl[1] >> 5) & 7) + ((m_spr_ctrl[1] >> 1) & 8);
	m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, spritecode2 + 32, 0, 0, 0, m_spr_xy[6], m_spr_xy[7], 1);

	rect = cliprect;
	rect.set_height(45);
	bitmap.fill(0, rect);

	rect = cliprect;
	rect.offset(32, 8);
	rect.set_size(16 * 8, 16);
	m_tilemap->draw(screen, bitmap, rect, 0, 0);

	//printf("PL %03d:%03d %X SP1 %03d:%03d %X SP2 %03d:%03d %X Road %03d:%03d %01X %d\n", m_spr_xy[0], m_spr_xy[1], spritecode0, m_spr_xy[2], m_spr_xy[3], spritecode1, m_spr_xy[6], m_spr_xy[7], spritecode2, m_spr_xy[4], m_spr_xy[5], (m_spr_ctrl[0] >> 4) & 7, m_road_scroll);
	return 0;
}

void istrebiteli_state::tileram_w(offs_t offset, uint8_t data)
{
	offset ^= 15;
	m_tileram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void istrebiteli_state::moto_tileram_w(offs_t offset, uint8_t data)
{
	m_tileram[offset] = data ^ 0xff;
	m_tilemap->mark_tile_dirty(offset);
}

void istrebiteli_state::road_ctrl_w(uint8_t data)
{
	m_road_scroll = data;
}

uint8_t istrebiteli_state::ppi0_r(offs_t offset)
{
	return m_ppi0->read(offset ^ 3) ^ 0xff;
}
void istrebiteli_state::ppi0_w(offs_t offset, uint8_t data)
{
	m_ppi0->write(offset ^ 3, data ^ 0xff);
}
uint8_t istrebiteli_state::ppi1_r(offs_t offset)
{
	return m_ppi1->read(offset ^ 3) ^ 0xff;
}
void istrebiteli_state::ppi1_w(offs_t offset, uint8_t data)
{
	m_ppi1->write(offset ^ 3, data ^ 0xff);
}

void istrebiteli_state::sound_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, data & 1);
	if (data & 1)
		coin_count = 0;
	m_sound_dev->sound_w(data);
}

void istrebiteli_state::spr0_ctrl_w(uint8_t data)
{
	m_spr_ctrl[0] = data;
	if (data & 0x80)
		m_spr_collision[0] = 0;
}

void istrebiteli_state::spr1_ctrl_w(uint8_t data)
{
	m_spr_ctrl[1] = data;
	if (data & 0x80)
		m_spr_collision[1] = 0;
}

void istrebiteli_state::spr_xy_w(offs_t offset, uint8_t data)
{
	m_spr_xy[offset ^ 7] = data;
}

void istrebiteli_state::moto_spr_xy_w(offs_t offset, uint8_t data)
{
	m_spr_xy[offset] = data;
}

void istrebiteli_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
}

void istrebiteli_state::moto_mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram(); // KR537RU8 16Kbit SRAM, only half used ?
}

void istrebiteli_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xb0, 0xbf).w(FUNC(istrebiteli_state::tileram_w));
	map(0xc0, 0xc3).rw(FUNC(istrebiteli_state::ppi0_r), FUNC(istrebiteli_state::ppi0_w));
	map(0xc4, 0xc7).rw(FUNC(istrebiteli_state::ppi1_r), FUNC(istrebiteli_state::ppi1_w));
	map(0xc8, 0xcf).w(FUNC(istrebiteli_state::spr_xy_w));
}

void istrebiteli_state::moto_io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x30, 0x37).w(FUNC(istrebiteli_state::moto_spr_xy_w));
	map(0x38, 0x3b).rw(m_ppi0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x3a, 0x3a).w(FUNC(istrebiteli_state::sound_w));
	map(0x3c, 0x3f).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x4f).w(FUNC(istrebiteli_state::moto_tileram_w));
}

template <int ID>
int istrebiteli_state::collision_r()
{
	// piece of HACK
	// real hardware does per-pixel sprite collision detection

	if ((m_spr_ctrl[ID] & 0x80) == 0)
	{
		int sx = m_spr_xy[0 + ID * 2];
		int sy = m_spr_xy[1 + ID * 2];
		int px = m_spr_xy[6 - ID * 2] + 3;
		int py = m_spr_xy[7 - ID * 2] + 3;

		if (sx > 56 && px >= sx && px < (sx + 8) && py >= sy && py < (sy + 8))
			m_spr_collision[ID] |= 1;
	}
	return m_spr_collision[ID];
}

ioport_value istrebiteli_state::coin_r()
{
	return coin_count;
}

INPUT_CHANGED_MEMBER(istrebiteli_state::coin_inc)
{
	if (oldval == 0 && newval == 1)
		++coin_count;
}

static INPUT_PORTS_START( istreb )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(istrebiteli_state::collision_r<1>))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(istrebiteli_state::collision_r<0>))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0x3c, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(istrebiteli_state::coin_r))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::hblank))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(istrebiteli_state::coin_inc), 0)
INPUT_PORTS_END

static INPUT_PORTS_START( moto )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) // handle left
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  // handle right
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    // speed 30
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  // speed 80
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1)        // speed 120, 3 above bits encode 0 30 50 80 100 120 speeds
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2)       // skip RAM test
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3)        // handle full left/right
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4)        // brake

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START1)  // coin, TODO check why it is locked
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON5) // collision ?
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::hblank)) // guess, seems unused
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,16,
	32,
	1,
	{ 0 },
	{ 7,6,5,4,3,2,1,0 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	8*16
};

static const gfx_layout sprite_layout =
{
	8,8,
	64,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 7*8,6*8,5*8,4*8,3*8,2*8,1*8,0*8 },
	8*8
};

static const gfx_layout moto_sprite_layout =
{
	16,16,
	16*3,
	1,
	{ 0 },
	{ 8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7 },
	{ 15*16,14*16,13*16,12*16,11*16,10*16,9*16,8*16,7*16,6*16,5*16,4*16,3*16,2*16,1*16,0*16 },
	16*16
};

static const gfx_layout projectile_layout =
{
	16,16,
	8,
	1,
	{ 0 },
	{ 15*8,14*8,13*8,12*8,11*8,10*8,9*8,8*8,7*8,6*8,5*8,4*8,3*8,2*8,1*8,0*8 },
	{ 15*128,14*128,13*128,12*128,11*128,10*128,9*128,8*128,7*128,6*128,5*128,4*128,3*128,2*128,1*128,0*128 },
	1
};

static GFXDECODE_START( gfx_istrebiteli )
	GFXDECODE_ENTRY( "chars", 0x0000, char_layout, 0, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0000, sprite_layout, 2, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0000, sprite_layout, 0, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0200, projectile_layout, 0, 2 )
GFXDECODE_END

static GFXDECODE_START( gfx_motogonki )
	GFXDECODE_ENTRY( "chars", 0x0000, char_layout, 2, 2 )
	GFXDECODE_ENTRY( "sprite", 0x0000, moto_sprite_layout, 2, 2 )
GFXDECODE_END

void istrebiteli_state::istreb(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(8'000'000) / 4);       // KR580VM80A
	m_maincpu->set_addrmap(AS_PROGRAM, &istrebiteli_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &istrebiteli_state::io_map);

	i8255_device &ppi0(I8255A(config, "ppi0"));
	ppi0.in_pa_callback().set_ioport("IN1");
	ppi0.in_pb_callback().set_ioport("IN0");
	ppi0.out_pc_callback().set(FUNC(istrebiteli_state::sound_w));

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.out_pa_callback().set(FUNC(istrebiteli_state::spr0_ctrl_w));
	ppi1.out_pb_callback().set(FUNC(istrebiteli_state::spr1_ctrl_w));
	ppi1.in_pc_callback().set_ioport("IN2");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(8'000'000) / 2, 256, 64, 256, 312, 0, 256);
	screen.set_screen_update(FUNC(istrebiteli_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_istrebiteli);
	PALETTE(config, "palette", FUNC(istrebiteli_state::istrebiteli_palette), 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ISTREBITELI_SOUND(config, m_sound_dev, XTAL(8'000'000) / 2 / 256, "soundrom").add_route(ALL_OUTPUTS, "mono", 1.00);
}

void istrebiteli_state::motogonki(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, XTAL(15'700'000) / 9);       // KR580VM80A
	m_maincpu->set_addrmap(AS_PROGRAM, &istrebiteli_state::moto_mem_map);
	m_maincpu->set_addrmap(AS_IO, &istrebiteli_state::moto_io_map);

	i8255_device &ppi0(I8255A(config, "ppi0"));
	ppi0.out_pa_callback().set(FUNC(istrebiteli_state::spr0_ctrl_w));
	ppi0.out_pb_callback().set(FUNC(istrebiteli_state::spr1_ctrl_w));

	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.in_pa_callback().set_ioport("IN0");
	ppi1.out_pb_callback().set(FUNC(istrebiteli_state::road_ctrl_w));
	ppi1.in_pc_callback().set_ioport("IN1");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(8'000'000) / 2, 256, 64, 256, 312, 0, 256);
	screen.set_screen_update(FUNC(istrebiteli_state::moto_screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_motogonki);
	PALETTE(config, "palette", FUNC(istrebiteli_state::motogonki_palette), 4);
	MCFG_VIDEO_START_OVERRIDE(istrebiteli_state, moto)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ISTREBITELI_SOUND(config, m_sound_dev, XTAL(8'000'000) / 2 / 256, "soundrom").add_route(ALL_OUTPUTS, "mono", 1.00);
}

ROM_START( istreb )
	ROM_REGION( 0x1000, I8080_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "002-ia12.bin",   0x000, 0x200, CRC(de0bce75) SHA1(ca284e8220d0d55c1a4dd3e951b53404f40fc873) )
	ROM_LOAD( "002-ia9.bin",    0x200, 0x200, CRC(e9a93ee7) SHA1(63c2001140d2b30657fceca97a639b1acbf614c2) )
	ROM_LOAD( "002-ib11-2.bin", 0x400, 0x200, CRC(4bb8b875) SHA1(230193e08586f4585fe98b2b31c4c8aa57a83e70) )
	ROM_LOAD( "002-ib9.bin",    0x600, 0x200, CRC(4eb948b5) SHA1(e9926591d1b0c528630b54956993c01139c58913) )
	ROM_LOAD( "002-ib13.bin",   0x800, 0x200, CRC(4fec5b14) SHA1(72b01c28882d567cad6924e05849438e5fe7a133) )

	ROM_REGION( 0x200, "chars", 0 )
	ROM_LOAD( "003-g8.bin", 0x000, 0x200, CRC(5cd7ad47) SHA1(2142711c8a3640b7aa258a2059cfb0f14297a5ac) )

	ROM_REGION( 0x1000, "sprite", 0 )
	ROM_LOAD( "001-g4.bin",  0x000, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
	ROM_LOAD( "001-g9.bin",  0x000, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
	ROM_LOAD( "001-a11.bin", 0x200, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )
	ROM_LOAD( "001-b11.bin", 0x200, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )

	ROM_REGION(0x200, "soundrom", 0)
	ROM_LOAD( "003-w3.bin", 0x000, 0x200, CRC(54eb4893) SHA1(c7a4724045c645ab728074ed7fef1882d9776005) )
ROM_END

// hardware is similar to Istrebiteli, but bigger ROM, RAM location moved, CPU A/D buses is not inverted unlike Istrebiteli PCB
// test mode: PPI1 port A bits 1,6,7 must be active low (currently left+btn3+btn4), then insert coin (press start)
ROM_START( motogonki )
	ROM_REGION( 0x2000, I8080_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "005_mb3.b2",   0x000, 0x2000, CRC(4dd35ed6) SHA1(6a0ee9e370634e501b6ee15a9747a491b745a205) )

	ROM_REGION( 0x200, "chars", 0 )
	ROM_LOAD( "003_ig8.g8", 0x000, 0x200, CRC(9af1e9de) SHA1(4bc89bc0c1f229ca3ebee983ae2fb3910d8ca599) )

	ROM_REGION( 0x1000, "sprite", 0 )
	ROM_LOAD( "006_b1.b1",  0x000, 0x200, CRC(ae9820fb) SHA1(7727d20e314aee670ba36ca6ea7ca5a4da0fc1cd) ) // player bike sprites, 16 16x16
	ROM_LOAD( "006_02.b5",  0x200, 0x200, CRC(e5c17daf) SHA1(1b6ffeba7dd98da11e5eb953280dd53f0f77fa7f) ) // opponents, road signs, etc sprites, 16 16x16
	ROM_LOAD( "006_03.b7",  0x400, 0x200, CRC(e1731d8d) SHA1(744fd768754a65a66bfcdb1959b4d6796bff4fcb) ) // more opponents, road signs, etc sprites, 16 16x16

	ROM_REGION(0x200, "soundrom", 0)
	ROM_LOAD( "003_iw3.w3", 0x000, 0x200, CRC(814854ba) SHA1(2cbfd60df01f00d7659393efa58547de660bf201) )

	ROM_REGION( 0x300, "proms", 0 ) // KR556RT4 256x4 ROMs
	ROM_LOAD( "006_05.b3",  0x000, 0x100, CRC(7dc4f9c9) SHA1(8a40f9f021b1662b1c638c7fdcefead1687ca4f1) ) // road tilemap ?
	ROM_LOAD( "006_01.d3",  0x100, 0x100, CRC(b53b83c9) SHA1(8f9733c827cc9aacc7c182585dcbc5da01357468) ) // sprite generators outputs combine prom
	ROM_LOAD( "006_04.w13", 0x200, 0x100, CRC(e43a500c) SHA1(c9a90b54587d0dc9d7d66c419790627088f2546e) ) // ports 30-37 address decoder prom
ROM_END

} // Anonymous namespace


GAME( 198?, istreb,    0, istreb,    istreb, istrebiteli_state, init_istreb, ROT0, "Terminal", "Istrebiteli", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE)
GAME( 198?, motogonki, 0, motogonki, moto,   istrebiteli_state, init_moto,   ROT0, "Terminal", "Motogonki", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
