// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Ryan Holtz

/***************************************************************************

    Ramtek Star Cruiser Driver

    (no known issues)

    Frank Palazzolo
    palazzol@home.com

    Netlist Audio by Ryan Holtz

***************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/netlist.h"
#include "netlist/nl_setup.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "nl_starcrus.h"


namespace {

class starcrus_state : public driver_device
{
public:
	starcrus_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_explode(*this, "sound_nl:explode%u", 1U),
		m_launch(*this, "sound_nl:launch%u", 1U),
		m_engine(*this, "sound_nl:engine%u", 1U),
		m_led(*this, "led2")
	{ }

	void starcrus(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void s1_x_w(uint8_t data);
	void s1_y_w(uint8_t data);
	void s2_x_w(uint8_t data);
	void s2_y_w(uint8_t data);
	void p1_x_w(uint8_t data);
	void p1_y_w(uint8_t data);
	void p2_x_w(uint8_t data);
	void p2_y_w(uint8_t data);
	void ship_parm_1_w(uint8_t data);
	void ship_parm_2_w(uint8_t data);
	void proj_parm_1_w(uint8_t data);
	void proj_parm_2_w(uint8_t data);
	uint8_t coll_det_r();

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_device_array<netlist_mame_logic_input_device, 2> m_explode;
	required_device_array<netlist_mame_logic_input_device, 2> m_launch;
	required_device_array<netlist_mame_logic_input_device, 2> m_engine;

	output_finder<> m_led;

	std::unique_ptr<bitmap_ind16> m_ship1_vid;
	std::unique_ptr<bitmap_ind16> m_ship2_vid;
	std::unique_ptr<bitmap_ind16> m_proj1_vid;
	std::unique_ptr<bitmap_ind16> m_proj2_vid;

	uint8_t m_s1_x = 0;
	uint8_t m_s1_y = 0;
	uint8_t m_s2_x = 0;
	uint8_t m_s2_y = 0;
	uint8_t m_p1_x = 0;
	uint8_t m_p1_y = 0;
	uint8_t m_p2_x = 0;
	uint8_t m_p2_y = 0;

	uint8_t m_p1_sprite = 0;
	uint8_t m_p2_sprite = 0;
	uint8_t m_s1_sprite = 0;
	uint8_t m_s2_sprite = 0;

	uint8_t m_collision_reg = 0;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int collision_check_s1s2();
	int collision_check_p1p2();
	int collision_check_s1p1p2();
	int collision_check_s2p1p2();

	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
};


/* The collision detection techniques used in this driver
   are well explained in the comments in the sprint2 driver */

void starcrus_state::s1_x_w(uint8_t data){ m_s1_x = data ^ 0xff; }
void starcrus_state::s1_y_w(uint8_t data){ m_s1_y = data ^ 0xff; }
void starcrus_state::s2_x_w(uint8_t data){ m_s2_x = data ^ 0xff; }
void starcrus_state::s2_y_w(uint8_t data){ m_s2_y = data ^ 0xff; }
void starcrus_state::p1_x_w(uint8_t data){ m_p1_x = data ^ 0xff; }
void starcrus_state::p1_y_w(uint8_t data){ m_p1_y = data ^ 0xff; }
void starcrus_state::p2_x_w(uint8_t data){ m_p2_x = data ^ 0xff; }
void starcrus_state::p2_y_w(uint8_t data){ m_p2_y = data ^ 0xff; }

void starcrus_state::video_start()
{
	m_ship1_vid = std::make_unique<bitmap_ind16>(16, 16);
	m_ship2_vid = std::make_unique<bitmap_ind16>(16, 16);

	m_proj1_vid = std::make_unique<bitmap_ind16>(16, 16);
	m_proj2_vid = std::make_unique<bitmap_ind16>(16, 16);

	save_item(NAME(m_s1_x));
	save_item(NAME(m_s1_y));
	save_item(NAME(m_s2_x));
	save_item(NAME(m_s2_y));
	save_item(NAME(m_p1_x));
	save_item(NAME(m_p1_y));
	save_item(NAME(m_p2_x));
	save_item(NAME(m_p2_y));
	save_item(NAME(m_p1_sprite));
	save_item(NAME(m_p2_sprite));
	save_item(NAME(m_s1_sprite));
	save_item(NAME(m_s2_sprite));
	save_item(NAME(m_collision_reg));
}

void starcrus_state::ship_parm_1_w(uint8_t data)
{
	m_s1_sprite = data & 0x1f;
	m_engine[0]->write_line(BIT(data, 5));
}

void starcrus_state::ship_parm_2_w(uint8_t data)
{
	m_s2_sprite = data & 0x1f;
	m_led = !BIT(data, 7); // game over lamp
	machine().bookkeeping().coin_counter_w(0, ((data & 0x40) >> 6) ^ 0x01); // coin counter
	m_engine[1]->write_line(BIT(data, 5));
}

void starcrus_state::proj_parm_1_w(uint8_t data)
{
	m_p1_sprite = data & 0x0f;
	m_launch[0]->write_line(BIT(data, 5));
	m_explode[0]->write_line(BIT(data, 4));
}

void starcrus_state::proj_parm_2_w(uint8_t data)
{
	m_p2_sprite = data & 0x0f;
	m_launch[1]->write_line(BIT(data, 5));
	m_explode[1]->write_line(BIT(data, 4));
}

int starcrus_state::collision_check_s1s2()
{
	rectangle clip(0, 15, 0, 15);

	m_ship1_vid->fill(0, clip);
	m_ship2_vid->fill(0, clip);

	// origin is with respect to ship1

	int const org_x = m_s1_x;
	int const org_y = m_s1_y;

	// Draw ship 1

	m_gfxdecode->gfx(8 + ((m_s1_sprite & 0x04) >> 2))->opaque(*m_ship1_vid,
			clip,
			(m_s1_sprite & 0x03) ^ 0x03,
			0,
			(m_s1_sprite & 0x08) >> 3, (m_s1_sprite & 0x10) >> 4,
			m_s1_x - org_x, m_s1_y - org_y);

	// Draw ship 2

	m_gfxdecode->gfx(10 + ((m_s2_sprite & 0x04) >> 2))->opaque(*m_ship2_vid,
			clip,
			(m_s2_sprite & 0x03) ^ 0x03,
			0,
			(m_s2_sprite & 0x08) >> 3, (m_s2_sprite & 0x10) >> 4,
			m_s2_x - org_x, m_s2_y - org_y);

	// Now check for collisions
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
		// Condition 1 - ship 1 = ship 2
		if ((m_ship1_vid->pix(sy, sx) == 1) && (m_ship2_vid->pix(sy, sx) == 1))
			return 1;

	return 0;
}

int starcrus_state::collision_check_p1p2()
{
	rectangle clip(0, 15, 0, 15);

	// if both are scores, return
	if (((m_p1_sprite & 0x08) == 0) &&
			((m_p2_sprite & 0x08) == 0))
	{
		return 0;
	}

	m_proj1_vid->fill(0, clip);
	m_proj2_vid->fill(0, clip);

	// origin is with respect to proj1

	int const org_x = m_p1_x;
	int const org_y = m_p1_y;

	if (m_p1_sprite & 0x08) // if p1 is a projectile
	{
		// Draw score/projectile 1

		m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->opaque(*m_proj1_vid,
				clip,
				(m_p1_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p1_x - org_x, m_p1_y - org_y);
	}

	if (m_p2_sprite & 0x08) // if p2 is a projectile
	{
		// Draw score/projectile 2

		m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->opaque(*m_proj2_vid,
				clip,
				(m_p2_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p2_x - org_x, m_p2_y - org_y);
	}

	// Now check for collisions
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
			// Condition 1 - proj 1 = proj 2
			if ((m_proj1_vid->pix(sy, sx) == 1) && (m_proj2_vid->pix(sy, sx) == 1))
				return 1;

	return 0;
}

int starcrus_state::collision_check_s1p1p2()
{
	rectangle clip(0, 15, 0, 15);

	// if both are scores, return
	if ((m_p1_sprite & 0x08) == 0 && (m_p2_sprite & 0x08) == 0)
	{
		return 0;
	}

	m_ship1_vid->fill(0, clip);
	m_proj1_vid->fill(0, clip);
	m_proj2_vid->fill(0, clip);

	// origin is with respect to ship1

	int const org_x = m_s1_x;
	int const org_y = m_s1_y;

	// Draw ship 1

	m_gfxdecode->gfx(8 + ((m_s1_sprite & 0x04) >> 2))->opaque(*m_ship1_vid,
			clip,
			(m_s1_sprite & 0x03) ^ 0x03,
			0,
			(m_s1_sprite & 0x08) >> 3, (m_s1_sprite & 0x10) >> 4,
			m_s1_x - org_x, m_s1_y - org_y);

	if (m_p1_sprite & 0x08) // if p1 is a projectile
	{
		// Draw projectile 1

		m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->opaque(*m_proj1_vid,
				clip,
				(m_p1_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p1_x - org_x, m_p1_y - org_y);
	}

	if (m_p2_sprite & 0x08) // if p2 is a projectile
	{
		// Draw projectile 2

		m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->opaque(*m_proj2_vid,
				clip,
				(m_p2_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p2_x - org_x, m_p2_y - org_y);
	}

	// Now check for collisions
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
			if (m_ship1_vid->pix(sy, sx) == 1)
			{
				// Condition 1 - ship 1 = proj 1
				if (m_proj1_vid->pix(sy, sx) == 1)
					return 1;
				// Condition 2 - ship 1 = proj 2
				if (m_proj2_vid->pix(sy, sx) == 1)
					return 1;
			}

	return 0;
}

int starcrus_state::collision_check_s2p1p2()
{
	rectangle clip(0, 15, 0, 15);

	// if both are scores, return
	if ((m_p1_sprite & 0x08) == 0 && (m_p2_sprite & 0x08) == 0)
	{
		return 0;
	}

	m_ship2_vid->fill(0, clip);
	m_proj1_vid->fill(0, clip);
	m_proj2_vid->fill(0, clip);

	// origin is with respect to ship2

	int const org_x = m_s2_x;
	int const org_y = m_s2_y;

	// Draw ship 2

	m_gfxdecode->gfx(10 + ((m_s2_sprite & 0x04) >> 2))->opaque(*m_ship2_vid,
			clip,
			(m_s2_sprite & 0x03) ^ 0x03,
			0,
			(m_s2_sprite & 0x08) >> 3, (m_s2_sprite & 0x10) >> 4,
			m_s2_x - org_x, m_s2_y - org_y);

	if (m_p1_sprite & 0x08) // if p1 is a projectile
	{
		// Draw projectile 1

		m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->opaque(*m_proj1_vid,
				clip,
				(m_p1_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p1_x - org_x, m_p1_y - org_y);
	}

	if (m_p2_sprite & 0x08) // if p2 is a projectile
	{
		// Draw projectile 2
		m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->opaque(*m_proj2_vid,
				clip,
				(m_p2_sprite & 0x03) ^ 0x03,
				0,
				0, 0,
				m_p2_x - org_x, m_p2_y - org_y);
	}

	// Now check for collisions
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
			if (m_ship2_vid->pix(sy, sx) == 1)
			{
				// Condition 1 - ship 2 = proj 1
				if (m_proj1_vid->pix(sy, sx) == 1)
					return 1;
				// Condition 2 - ship 2 = proj 2
				if (m_proj2_vid->pix(sy, sx) == 1)
					return 1;
			}

	return 0;
}

uint32_t starcrus_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// Draw ship 1
	m_gfxdecode->gfx(8 + ((m_s1_sprite & 0x04) >> 2))->transpen(bitmap,
			cliprect,
			(m_s1_sprite & 0x03) ^ 0x03,
			0,
			(m_s1_sprite & 0x08) >> 3, (m_s1_sprite & 0x10) >> 4,
			m_s1_x, m_s1_y,
			0);

	// Draw ship 2
	m_gfxdecode->gfx(10 + ((m_s2_sprite & 0x04) >> 2))->transpen(bitmap,
			cliprect,
			(m_s2_sprite & 0x03) ^ 0x03,
			0,
			(m_s2_sprite & 0x08) >> 3, (m_s2_sprite & 0x10) >> 4,
			m_s2_x, m_s2_y,
			0);

	// Draw score/projectile 1
	m_gfxdecode->gfx((m_p1_sprite & 0x0c) >> 2)->transpen(bitmap,
			cliprect,
			(m_p1_sprite & 0x03) ^ 0x03,
			0,
			0, 0,
			m_p1_x, m_p1_y,
			0);

	// Draw score/projectile 2
	m_gfxdecode->gfx(4 + ((m_p2_sprite & 0x0c) >> 2))->transpen(bitmap,
			cliprect,
			(m_p2_sprite & 0x03) ^ 0x03,
			0,
			0, 0,
			m_p2_x, m_p2_y,
			0);

	// Collision detection
	if (cliprect.max_y == screen.visible_area().max_y)
	{
		m_collision_reg = 0x00;

		// Check for collisions between ship1 and ship2
		if (collision_check_s1s2())
		{
			m_collision_reg |= 0x08;
		}
		// Check for collisions between ship1 and projectiles
		if (collision_check_s1p1p2())
		{
			m_collision_reg |= 0x02;
		}
		// Check for collisions between ship1 and projectiles
		if (collision_check_s2p1p2())
		{
			m_collision_reg |= 0x01;
		}
		// Check for collisions between ship1 and projectiles
		// Note: I don't think this is used by the game
		if (collision_check_p1p2())
		{
			m_collision_reg |= 0x04;
		}
	}

	return 0;
}

uint8_t starcrus_state::coll_det_r()
{
	return m_collision_reg ^ 0xff;
}


void starcrus_state::machine_start()
{
	m_led.resolve();
}

void starcrus_state::program_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x10ff).ram();
}

void starcrus_state::io_map(address_map &map)
{
	map(0x00, 0x00).portr("P1").w(FUNC(starcrus_state::s1_x_w));
	map(0x01, 0x01).portr("P2").w(FUNC(starcrus_state::s1_y_w));
	map(0x02, 0x02).rw(FUNC(starcrus_state::coll_det_r), FUNC(starcrus_state::s2_x_w));
	map(0x03, 0x03).portr("DSW").w(FUNC(starcrus_state::s2_y_w));
	map(0x04, 0x04).w(FUNC(starcrus_state::p1_x_w));
	map(0x05, 0x05).w(FUNC(starcrus_state::p1_y_w));
	map(0x06, 0x06).w(FUNC(starcrus_state::p2_x_w));
	map(0x07, 0x07).w(FUNC(starcrus_state::p2_y_w));
	map(0x08, 0x08).w(FUNC(starcrus_state::ship_parm_1_w));
	map(0x09, 0x09).w(FUNC(starcrus_state::ship_parm_2_w));
	map(0x0a, 0x0a).w(FUNC(starcrus_state::proj_parm_1_w));
	map(0x0b, 0x0b).w(FUNC(starcrus_state::proj_parm_2_w));
}


static INPUT_PORTS_START( starcrus )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY // ccw
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // engine
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY // cw
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) // torpedo
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) // phaser
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2) // ccw
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // engine
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2) // cw
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // torpedo
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // phaser
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x03, "60 secs" )
	PORT_DIPSETTING(    0x02, "90 secs" )
	PORT_DIPSETTING(    0x01, "120 secs" )
	PORT_DIPSETTING(    0x00, "150 secs" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x20, 0x20, "Mode" )
	PORT_DIPSETTING(    0x20, DEF_STR( Standard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Alternate ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("POT_1")
	PORT_ADJUSTER( 50, "Pot: Noise Level" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "noise_volume")

	PORT_START("POT_2")
	PORT_ADJUSTER( 50, "Pot: Volume" )  NETLIST_ANALOG_PORT_CHANGED("sound_nl", "volume")
INPUT_PORTS_END



static const gfx_layout spritelayout1 =
{
	16,16,    // 16x16 sprites
	4,          // 4 sprites
	1,      // 1 bits per pixel
	{ 0 },  // 1 chip
	{ 0*8+4,  0*8+4,  1*8+4,  1*8+4, 2*8+4, 2*8+4, 3*8+4, 3*8+4,
		4*8+4,  4*8+4,  5*8+4,  5*8+4, 6*8+4, 6*8+4, 7*8+4, 7*8+4 },
	{ 0, 0, 1*64, 1*64, 2*64, 2*64, 3*64, 3*64,
		4*64, 4*64, 5*64, 5*64, 6*64, 6*64, 7*64, 7*64 },
	1  // every sprite takes 1 consecutive bit
};
static const gfx_layout spritelayout2 =
{
	16,16,   // 16x16 sprites
	4,       // 4 sprites
	1,       // 1 bits per pixel
	{ 0 },   // 1 chip
	{ 0*8+4,  1*8+4,  2*8+4,  3*8+4, 4*8+4, 5*8+4, 6*8+4, 7*8+4,
		8*8+4,  9*8+4,  10*8+4,  11*8+4, 12*8+4, 13*8+4, 14*8+4, 15*8+4 },
	{ 0, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
		8*128, 9*128, 10*128, 11*128, 12*128, 13*128, 14*128, 15*128 },
	1 // every sprite takes 1 consecutive bytes
};

static GFXDECODE_START( gfx_starcrus )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0040, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x0080, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x00c0, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0040, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x0080, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0x00c0, spritelayout1, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, spritelayout2, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0100, spritelayout2, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0200, spritelayout2, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0x0300, spritelayout2, 0, 1 )
GFXDECODE_END


void starcrus_state::starcrus(machine_config &config)
{
	// basic machine hardware
	I8080(config, m_maincpu, 9'750'000 / 9); // 8224 chip is a divide by 9
	m_maincpu->set_addrmap(AS_PROGRAM, &starcrus_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &starcrus_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(starcrus_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(starcrus_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_starcrus);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48'000)
		.set_source(NETLIST_NAME(starcrus))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:explode1", "EXPLODE_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:explode2", "EXPLODE_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:launch1", "LAUNCH_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:launch2", "LAUNCH_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:engine1", "ENGINE_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:engine2", "ENGINE_2.IN", 0);
	NETLIST_ANALOG_INPUT(config, "sound_nl:noise_volume", "R23.DIAL");
	NETLIST_ANALOG_INPUT(config, "sound_nl:volume", "R75.DIAL");

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "R77.2").set_mult_offset(100000.0 / 32768.0, 0.0);
}

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( starcrus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "starcrus.j1",   0x0000, 0x0200, CRC(0ee60a50) SHA1(7419e7cb4c589da53d4a10ad129373502682464e) )
	ROM_LOAD( "starcrus.k1",   0x0200, 0x0200, CRC(a7bc3bc4) SHA1(0e38076e921856608b1dd712687bef1c2522b4b8) )
	ROM_LOAD( "starcrus.l1",   0x0400, 0x0200, CRC(10d233ec) SHA1(8933cf9fc51716a9e8f75a4444e7d7070cf5834d) )
	ROM_LOAD( "starcrus.m1",   0x0600, 0x0200, CRC(2facbfee) SHA1(d78fb38de49da938fce2b55c8decc244efee6f94) )
	ROM_LOAD( "starcrus.n1",   0x0800, 0x0200, CRC(42083247) SHA1(b32d67c914833f18e9955cd1c3cb1d948be0a7d5) )
	ROM_LOAD( "starcrus.p1",   0x0a00, 0x0200, CRC(61dfe581) SHA1(e1802fedf94541e9ccd9786b60e90890485f422f) )
	ROM_LOAD( "starcrus.r1",   0x0c00, 0x0200, CRC(010cdcfe) SHA1(ae76f1739b468e2987ce949470b36f1a873e061d) )
	ROM_LOAD( "starcrus.s1",   0x0e00, 0x0200, CRC(da4e276b) SHA1(3298f7cb259803f118a47292cbb413df253ef74d) )

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "starcrus.e6",   0x0000, 0x0200, CRC(54887a25) SHA1(562bf85cd063c2cc0a2f803095aaa6138dfb5bff) )

	ROM_REGION( 0x0200, "gfx2", 0 )
	ROM_LOAD( "starcrus.l2",   0x0000, 0x0200, CRC(54887a25) SHA1(562bf85cd063c2cc0a2f803095aaa6138dfb5bff) )

	ROM_REGION( 0x0400, "gfx3", 0 )
	ROM_LOAD( "starcrus.j4",   0x0000, 0x0200, CRC(25f15ae1) SHA1(7528edaa01ad5a167191c7e72394cb6009db1b27) )
	ROM_LOAD( "starcrus.g5",   0x0200, 0x0200, CRC(73b27f6e) SHA1(4a6cf9244556a2c2647d594c7a19fe1a374a57e6) )
ROM_END

} // anonymous namespace


GAME( 1977, starcrus, 0, starcrus, starcrus, starcrus_state, empty_init, ROT0, "Ramtek", "Star Cruiser", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
