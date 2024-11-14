// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Taxi Driver  (c) 1984 Graphic Techno

XTAL:  10MHz, 8MHz
CPU:   3 * NEC D780C (clocks unknown)
SOUND: 2 * AY-3-8910 (clocks unknown)
OTHER: 5 * M5L8255AP

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class taxidriv_state : public driver_device
{
public:
	taxidriv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram%u", 0U),
		m_scroll(*this, "scroll"),
		m_servcoin(*this, "SERVCOIN")
	{ }

	void taxidriv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr_array<uint8_t, 8> m_vram;
	required_shared_ptr<uint8_t> m_scroll;

	required_ioport m_servcoin;

	int m_s1;
	int m_s2;
	int m_s3;
	int m_s4;
	int m_latchA;
	int m_latchB;
	int m_bghide;
	int m_spritectrl[9];

	uint8_t p0a_r();
	uint8_t p0c_r();
	void p0b_w(uint8_t data);
	void p0c_w(uint8_t data);
	uint8_t p1b_r();
	uint8_t p1c_r();
	void p1a_w(uint8_t data);
	void p1c_w(uint8_t data);
	uint8_t p8910_0a_r();
	uint8_t p8910_1a_r();
	void p8910_0b_w(uint8_t data);
	template <unsigned Offset> void spritectrl_w(uint8_t data);

	void taxidriv_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cpu2_map(address_map &map) ATTR_COLD;
	void cpu3_map(address_map &map) ATTR_COLD;
	void cpu3_port_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void taxidriv_state::machine_start()
{
	save_item(NAME(m_s1));
	save_item(NAME(m_s2));
	save_item(NAME(m_s3));
	save_item(NAME(m_s4));
	save_item(NAME(m_latchA));
	save_item(NAME(m_latchB));
	save_item(NAME(m_bghide));
	save_item(NAME(m_spritectrl));
}


uint8_t taxidriv_state::p0a_r()
{
	return m_latchA;
}

uint8_t taxidriv_state::p0c_r()
{
	return m_s1 << 7;
}

void taxidriv_state::p0b_w(uint8_t data)
{
	m_latchB = data;
}

void taxidriv_state::p0c_w(uint8_t data)
{
	m_s2 = data & 1;

	m_bghide = data & 2;

	/* bit 2 toggles during gameplay */

	flip_screen_set(data & 8);

//  popmessage("%02x",data&0x0f);
}

uint8_t taxidriv_state::p1b_r()
{
	return m_latchB;
}

uint8_t taxidriv_state::p1c_r()
{
	return (m_s2 << 7) | (m_s4 << 6) | ((m_servcoin->read() & 1) << 4);
}

void taxidriv_state::p1a_w(uint8_t data)
{
	m_latchA = data;
}

void taxidriv_state::p1c_w(uint8_t data)
{
	m_s1 = data & 1;
	m_s3 = (data & 2) >> 1;
}

uint8_t taxidriv_state::p8910_0a_r()
{
	return m_latchA;
}

uint8_t taxidriv_state::p8910_1a_r()
{
	return m_s3;
}

/* note that a lot of writes happen with port B set as input. I think this is a bug in the
   original, since it works anyway even if the communication is flawed. */
void taxidriv_state::p8910_0b_w(uint8_t data)
{
	m_s4 = data & 1;
}


template <unsigned Offset>
void taxidriv_state::spritectrl_w(uint8_t data)
{
	m_spritectrl[Offset] = data;
}


uint32_t taxidriv_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_bghide)
	{
		bitmap.fill(0, cliprect);


		/* kludge to fix scroll after death */
		m_scroll[0] = m_scroll[1] = m_scroll[2] = m_scroll[3] = 0;
		m_spritectrl[2] = m_spritectrl[5] = m_spritectrl[8] = 0;
	}
	else
	{
		for (int offs = 0;offs < 0x400;offs++)
		{
			int sx = offs % 32;
			int sy = offs / 32;

			m_gfxdecode->gfx(3)->opaque(bitmap,cliprect,
					m_vram[3][offs],
					0,
					0,0,
					(sx*8-m_scroll[0])&0xff,(sy*8-m_scroll[1])&0xff);
		}

		for (int offs = 0;offs < 0x400;offs++)
		{
			int sx = offs % 32;
			int sy = offs / 32;

			m_gfxdecode->gfx(2)->transpen(bitmap,cliprect,
					m_vram[2][offs]+256*m_vram[2][offs+0x400],
					0,
					0,0,
					(sx*8-m_scroll[2])&0xff,(sy*8-m_scroll[3])&0xff,0);
		}

		if (m_spritectrl[2] & 4)
		{
			for (int offs = 0;offs < 0x1000;offs++)
			{
				int sx = ((offs/2) % 64-m_spritectrl[0]-256*(m_spritectrl[2]&1))&0x1ff;
				int sy = ((offs/2) / 64-m_spritectrl[1]-128*(m_spritectrl[2]&2))&0x1ff;

				int color = (m_vram[5][offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						bitmap.pix(sy, sx) = color;
				}
			}
		}

		if (m_spritectrl[5] & 4)
		{
			for (int offs = 0;offs < 0x1000;offs++)
			{
				int sx = ((offs/2) % 64-m_spritectrl[3]-256*(m_spritectrl[5]&1))&0x1ff;
				int sy = ((offs/2) / 64-m_spritectrl[4]-128*(m_spritectrl[5]&2))&0x1ff;

				int color = (m_vram[6][offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						bitmap.pix(sy, sx) = color;
				}
			}
		}

		if (m_spritectrl[8] & 4)
		{
			for (int offs = 0;offs < 0x1000;offs++)
			{
				int sx = ((offs/2) % 64-m_spritectrl[6]-256*(m_spritectrl[8]&1))&0x1ff;
				int sy = ((offs/2) / 64-m_spritectrl[7]-128*(m_spritectrl[8]&2))&0x1ff;

				int color = (m_vram[7][offs/4]>>(2*(offs&3)))&0x03;
				if (color)
				{
					if (sx > 0 && sx < 256 && sy > 0 && sy < 256)
						bitmap.pix(sy, sx) = color;
				}
			}
		}

		for (int offs = 0;offs < 0x400;offs++)
		{
			int sx = offs % 32;
			int sy = offs / 32;

			m_gfxdecode->gfx(1)->transpen(bitmap,cliprect,
					m_vram[1][offs],
					0,
					0,0,
					sx*8,sy*8,0);
		}

		for (int offs = 0;offs < 0x2000;offs++)
		{
			int sx = (offs/2) % 64;
			int sy = (offs/2) / 64;

			int color = (m_vram[4][offs/4]>>(2*(offs&3)))&0x03;
			if (color)
			{
				bitmap.pix(sy, sx) = 2 * color;
			}
		}
	}

	for (int offs = 0;offs < 0x400;offs++)
	{
		int sx = offs % 32;
		int sy = offs / 32;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				m_vram[0][offs],
				0,
				0,0,
				sx*8,sy*8,0);
	}
	return 0;
}

void taxidriv_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram(); // ???
	map(0x9000, 0x9fff).ram(); // ???
	map(0xa000, 0xafff).ram(); // ???
	map(0xb000, 0xbfff).ram(); // ???
	map(0xc000, 0xc7ff).ram().share(m_vram[4]);       // radar bitmap
	map(0xc800, 0xcfff).writeonly().share(m_vram[5]); // "sprite1" bitmap
	map(0xd000, 0xd7ff).writeonly().share(m_vram[6]); // "sprite2" bitmap
	map(0xd800, 0xdfff).ram().share(m_vram[7]);       // "sprite3" bitmap
	map(0xe000, 0xe3ff).ram().share(m_vram[1]);       // car tilemap
	map(0xe400, 0xebff).ram().share(m_vram[2]);       // bg1 tilemap
	map(0xec00, 0xefff).ram().share(m_vram[0]);       // fg tilemap
	map(0xf000, 0xf3ff).ram().share(m_vram[3]);       // bg2 tilemap
	map(0xf400, 0xf403).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xf480, 0xf483).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));    // "sprite1" placement
	map(0xf500, 0xf503).rw("ppi8255_3", FUNC(i8255_device::read), FUNC(i8255_device::write));    // "sprite2" placement
	map(0xf580, 0xf583).rw("ppi8255_4", FUNC(i8255_device::read), FUNC(i8255_device::write));    // "sprite3" placement
	//map(0xf780, 0xf781).writeonly();    // more scroll registers?
	map(0xf782, 0xf787).writeonly().share("scroll");    // bg scroll (three copies always identical)
	map(0xf800, 0xffff).ram();
}

void taxidriv_state::cpu2_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x6000, 0x67ff).ram();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa003).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe000, 0xe000).portr("DSW0");
	map(0xe001, 0xe001).portr("DSW1");
	map(0xe002, 0xe002).portr("DSW2");
	map(0xe003, 0xe003).portr("P1");
	map(0xe004, 0xe004).portr("P2");
}

void taxidriv_state::cpu3_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2000).nopr(); /* irq ack? */
	map(0xfc00, 0xffff).ram();
}

void taxidriv_state::cpu3_port_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x01, 0x01).r("ay1", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x03, 0x03).r("ay2", FUNC(ay8910_device::data_r));
}


static INPUT_PORTS_START( taxidriv )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( Free_Play ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "255 (Cheat)")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x28, "6" )
	PORT_DIPSETTING(    0x30, "7" )
	PORT_DIPSETTING(    0x38, "8" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Fuel Consumption" )
	PORT_DIPSETTING(    0x00, "Slowest" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x05, "6" )
	PORT_DIPSETTING(    0x06, "7" )
	PORT_DIPSETTING(    0x07, "Fastest" )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x18, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x28, "6" )
	PORT_DIPSETTING(    0x30, "7" )
	PORT_DIPSETTING(    0x38, "8" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "40/30" )
	PORT_DIPSETTING(    0x40, "30/20" )
	PORT_DIPSETTING(    0x80, "20/15" )
	PORT_DIPSETTING(    0xc0, "10/10" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("SERVCOIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )   /* handled by p1c_r() */
INPUT_PORTS_END



static const gfx_layout charlayout2 =
{
	4,4,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 3*4, 2*4 },
	{ 0*16, 1*16, 2*16, 3*16 },
	16*4
};


static GFXDECODE_START( gfx_taxidriv )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x4_packed_lsb, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x4_packed_lsb, 0, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x4_packed_lsb, 0, 1 )
	GFXDECODE_ENTRY( "gfx4", 0, gfx_8x8x4_packed_lsb, 0, 1 )
	GFXDECODE_ENTRY( "gfx5", 0, charlayout2, 0, 1 )
GFXDECODE_END

void taxidriv_state::taxidriv_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	// TODO: resistors, 1k & 470
	for (int i = 0; i < 0x10; ++i)
	{
		int bit0, bit1;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		int const r = 0x55 * bit0 + 0xaa * bit1;
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 3);
		int const g = 0x55 * bit0 + 0xaa * bit1;
		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		int const b = 0x55 * bit0 + 0xaa * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void taxidriv_state::taxidriv(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);    /* 4 MHz ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &taxidriv_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(taxidriv_state::irq0_line_hold));

	z80_device &subcpu(Z80(config, "sub", 4000000));    /* 4 MHz ??? */
	subcpu.set_addrmap(AS_PROGRAM, &taxidriv_state::cpu2_map);
	subcpu.set_vblank_int("screen", FUNC(taxidriv_state::irq0_line_hold));   /* ??? */

	z80_device &audiocpu(Z80(config, "audiocpu", 4000000));   /* 4 MHz ??? */
	audiocpu.set_addrmap(AS_PROGRAM, &taxidriv_state::cpu3_map);
	audiocpu.set_addrmap(AS_IO, &taxidriv_state::cpu3_port_map);
	audiocpu.set_vblank_int("screen", FUNC(taxidriv_state::irq0_line_hold));   /* ??? */

	config.set_maximum_quantum(attotime::from_hz(6000));  /* 100 CPU slices per frame - a high value to ensure proper */
							/* synchronization of the CPUs */

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set(FUNC(taxidriv_state::p0a_r));
	ppi0.out_pb_callback().set(FUNC(taxidriv_state::p0b_w));
	ppi0.in_pc_callback().set(FUNC(taxidriv_state::p0c_r));
	ppi0.out_pc_callback().set(FUNC(taxidriv_state::p0c_w));

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.out_pa_callback().set(FUNC(taxidriv_state::p1a_w));
	ppi1.in_pb_callback().set(FUNC(taxidriv_state::p1b_r));
	ppi1.in_pc_callback().set(FUNC(taxidriv_state::p1c_r));
	ppi1.out_pc_callback().set(FUNC(taxidriv_state::p1c_w));

	i8255_device &ppi2(I8255A(config, "ppi8255_2"));
	ppi2.out_pa_callback().set(FUNC(taxidriv_state::spritectrl_w<0>));
	ppi2.out_pb_callback().set(FUNC(taxidriv_state::spritectrl_w<1>));
	ppi2.out_pc_callback().set(FUNC(taxidriv_state::spritectrl_w<2>));

	i8255_device &ppi3(I8255A(config, "ppi8255_3"));
	ppi3.out_pa_callback().set(FUNC(taxidriv_state::spritectrl_w<3>));
	ppi3.out_pb_callback().set(FUNC(taxidriv_state::spritectrl_w<4>));
	ppi3.out_pc_callback().set(FUNC(taxidriv_state::spritectrl_w<5>));

	i8255_device &ppi4(I8255A(config, "ppi8255_4"));
	ppi4.out_pa_callback().set(FUNC(taxidriv_state::spritectrl_w<6>));
	ppi4.out_pb_callback().set(FUNC(taxidriv_state::spritectrl_w<7>));
	ppi4.out_pc_callback().set(FUNC(taxidriv_state::spritectrl_w<8>));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 1*8, 27*8-1);
	screen.set_screen_update(FUNC(taxidriv_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_taxidriv);
	PALETTE(config, m_palette, FUNC(taxidriv_state::taxidriv_palette), 16);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 1250000));
	ay1.port_a_read_callback().set(FUNC(taxidriv_state::p8910_0a_r));
	ay1.port_b_write_callback().set(FUNC(taxidriv_state::p8910_0b_w));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay2(AY8910(config, "ay2", 1250000));
	ay2.port_a_read_callback().set(FUNC(taxidriv_state::p8910_1a_r));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.25);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( taxidriv )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.ic87",       0x0000, 0x2000, CRC(6b2424e9) SHA1(a65bb01da8f3b0649d945981cc4f1324b7fac5c7) )
	ROM_LOAD( "2.ic86",       0x2000, 0x2000, CRC(15111229) SHA1(0350918f9504b0e470684ebc94a823bb2513a54d) )
	ROM_LOAD( "3.ic85",       0x4000, 0x2000, CRC(a7782eee) SHA1(0f10b7876420f4237937b1b922aa410de3f79af1) )
	ROM_LOAD( "4.ic84",       0x6000, 0x2000, CRC(8eb0b16b) SHA1(a0015744373ee91bc505f077a04ab3546f8bb6fb) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "8.ic4",        0x0000, 0x2000, CRC(9f9a3865) SHA1(908cf4f2cc68c088649241997276ea25c27d9718) )
//  ROM_LOAD( "8.ic4",        0x0000, 0x2000, CRC(9835d517) SHA1(845f3efc54b64837c22dd06683c2950f2b8b03cb) ) // 0x1b5f = 0x04 instead of 0x03 from another set
	ROM_LOAD( "9.ic5",        0x2000, 0x2000, CRC(b28b766c) SHA1(21e08ef1e2671c8540380e3fa0858e8a4d821945) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "7.ic14",       0x0000, 0x2000, CRC(2b4cbfe6) SHA1(a2a900831116554d5aea1a81c93245d3bb424d48) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5.m.ic68",     0x0000, 0x2000, CRC(a3aa5f2f) SHA1(7e046e2a5d230c62d93a83f5a773e6e4d6e85961) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "6.1.ic35",     0x0000, 0x2000, CRC(bfddd550) SHA1(f528c2701c635bc61eda14fbe2cfe9b44cb75c20) )

	ROM_REGION( 0x6000, "gfx3", 0 )
	ROM_LOAD( "11.30.ic87",   0x0000, 0x2000, CRC(7485eaea) SHA1(8d69c61145470003cfeb33b11b81345c5e5e6503) )
	ROM_LOAD( "14.31.ic110",  0x2000, 0x2000, CRC(0d99a33e) SHA1(0df29464ea43aecd866ae322f4f7ca9152422023) )
	ROM_LOAD( "15.32.ic111",  0x4000, 0x2000, CRC(410fdf7c) SHA1(0957f335b84c4fbde983271786e7bf199fc22682) )

	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "10.40.ic99",   0x0000, 0x2000, CRC(c370b177) SHA1(4b3f73f764ff95cc7777fe01333558201658cead) )

	ROM_REGION( 0x4000, "gfx5", 0 ) /* not used?? */
	ROM_LOAD( "12.21.ic88",   0x0000, 0x2000, CRC(684b7bb0) SHA1(d83c45ff3adf94c649340227794020482231399f) )
	ROM_LOAD( "13.20.ic89",   0x2000, 0x2000, CRC(d1ef110e) SHA1(e34b6b4b70c783a8cf1296a05d3cec6af5820d0c) )

	ROM_REGION( 0x020, "proms", 0 )
	ROM_LOAD( "tbp18s030.ic2",  0x0000, 0x020, CRC(c366a9c5) SHA1(d38581e5c425cab4a4f216d99651e86d8034a7d2) ) // color prom located at edge of pcb
ROM_END

} // anonymous namespace


GAME( 1984, taxidriv,  0,        taxidriv, taxidriv, taxidriv_state, empty_init, ROT90, "Graphic Techno", "Taxi Driver", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
