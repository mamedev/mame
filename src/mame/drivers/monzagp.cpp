// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/***************************************************************************

    Monza GP - Olympia


Upper board (MGP_02):
1x Intel P8035L (HMOS Single Component 8BIT Microcontroller - no internal ROM - 64x8 RAM)(main)
1x National DP8350N (Video Output Graphics Controller - CRT Controller)(main)
1x oscillator 10535

6x 2708 (10,11,12,13,14,15)
4x 2114 (4e,7e,6f,7f)

3x 10x2 legs connectors with flat cable to lower board
1x 8x2 dip switches


Lower board (MGP_01):
9x 2708 (1,2,3,4,5,6,7,8,9)(position 2 is empty, maybe my PCB is missing an EPROM)
5x MMI63S140N (1,3,4,5,6)
2x DM74S287N (2,7)

3x 10x2 legs connectors with flat cable to upper board
1x 6 legs connector (power supply)
1x 20x2 legs connector for flat cable
1x trimmer (TIME?????)

***************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/nvram.h"
#include "video/resnet.h"

#include "monzagp.lh"

class monzagp_state : public driver_device
{
public:
	monzagp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_nvram(*this, "nvram"),
		m_gfx1(*this, "gfx1"),
		m_gfx2(*this, "gfx2"),
		m_gfx3(*this, "gfx3"),
		m_tile_attr(*this, "unk1"),
		m_proms(*this, "proms"),
		m_steering_wheel(*this, "WHEEL"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_dsw(*this, "DSW")
		{ }

	DECLARE_READ8_MEMBER(port_r);
	DECLARE_WRITE8_MEMBER(port_w);
	DECLARE_WRITE8_MEMBER(port1_w);
	DECLARE_WRITE8_MEMBER(port2_w);
	DECLARE_READ8_MEMBER(port2_r);
	DECLARE_WRITE8_MEMBER(port3_w);
	virtual void video_start() override;
	TIMER_DEVICE_CALLBACK_MEMBER(time_tick_timer);
	DECLARE_PALETTE_INIT(monzagp);
	UINT32 screen_update_monzagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<nvram_device> m_nvram;
	required_memory_region m_gfx1;
	required_memory_region m_gfx2;
	required_memory_region m_gfx3;
	required_memory_region m_tile_attr;
	required_memory_region m_proms;
	required_ioport m_steering_wheel;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_dsw;

private:
	UINT8 m_p1;
	UINT8 m_p2;
	UINT8 m_video_ctrl[2][8];
	bool  m_time_tick;
	bool  m_cp_ruote;
	UINT8 m_collisions_ff;
	UINT8 m_collisions_clk;
	UINT8 m_mycar_pos;
	std::unique_ptr<UINT8[]> m_vram;
	std::unique_ptr<UINT8[]> m_score_ram;
};



TIMER_DEVICE_CALLBACK_MEMBER(monzagp_state::time_tick_timer)
{
	m_time_tick = !m_time_tick;
}

PALETTE_INIT_MEMBER(monzagp_state, monzagp)
{
	static const int r_resistances[3] = { 220, 1000, 3300 };
	static const int g_resistances[3] = { 100, 470 , 1500 };
	static const int b_resistances[3] = { 100, 470 , 1500 };
	double rweights[3], gweights[3], bweights[3];

	// compute the color output resistor weights
	compute_resistor_weights(0, 255, -1.0,
			3, &r_resistances[0], rweights, 0, 0,
			3, &g_resistances[0], gweights, 0, 0,
			3, &b_resistances[0], bweights, 0, 0);

	for (int i = 0; i < 0x100; i++)
	{
		int bit0 = 0, bit1 = 0, bit2 = 0;
		UINT8 d = m_proms->base()[0x400 + i] ^ 0x0f;

		if (d & 0x08)
		{
			bit1 = BIT(i, 0);
			bit2 = BIT(i, 1);
		}

		// red component
		bit0 = (d >> 2) & 0x01;
		int r = combine_3_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = (d >> 1) & 0x01;
		int g = combine_3_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = (d >> 0) & 0x01;
		int b = combine_3_weights(bweights, bit0, bit1, bit2);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void monzagp_state::video_start()
{
	m_vram = std::make_unique<UINT8[]>(0x800);
	m_score_ram = std::make_unique<UINT8[]>(0x100);
	m_time_tick = 0;
	m_cp_ruote = 0;
	m_mycar_pos = 0;
	m_collisions_ff = 0;
	m_collisions_clk = 0;
	save_pointer(NAME(m_vram.get()), 0x800);
	save_pointer(NAME(m_score_ram.get()), 0x100);

	m_nvram->set_base(m_score_ram.get(), 0x100);
}

UINT32 monzagp_state::screen_update_monzagp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/*
    for(int i=0;i<8;i++)
        printf("%02x ", m_video_ctrl[0][i]);
    printf("   ----   ");
    for(int i=0;i<8;i++)
        printf("%02x ", m_video_ctrl[1][i]);
    printf("\n");
*/

	bitmap.fill(0, cliprect);

	// background tilemap
	UINT8 *tile_table = m_proms->base() + 0x100;
	UINT8 *collisions_prom = m_proms->base() + 0x200;

	UINT8 start_tile = m_video_ctrl[0][0] ^ 0xff;
	UINT8 inv_counter = m_video_ctrl[0][1] ^ 0xff;
	UINT8 mycar_y = m_mycar_pos;
	bool inv = false;

	for(int y=0; y<=256; y++, start_tile += inv ? -1 : +1)
	{
		if (inv_counter++ == 0xff)
			inv = true;

		UINT16 start_x = (((m_video_ctrl[0][3] << 8) | m_video_ctrl[0][2]) ^ 0xffff);
		UINT8 mycar_x = m_video_ctrl[1][2];

		for(int x=0; x<=256; x++, start_x++)
		{
			UINT8 tile_attr = m_tile_attr->base()[((start_x >> 5) & 0x1ff) | ((m_video_ctrl[0][3] & 0x80) ? 0 : 0x200)];

			//if (tile_attr & 0x10)         printf("dark on\n");
			//if (tile_attr & 0x20)         printf("light on\n");
			//if (tile_attr & 0x40)         printf("bridge\n");

			int tile_idx = tile_table[((tile_attr & 0x0f) << 4) | (inv ? 0x08 : 0) | ((start_tile >> 5) & 0x07)];

			int bit_pos = 3 - (start_x & 3);
			UINT8 tile_data = m_gfx3->base()[(tile_idx << 8) | (((start_x << 3) & 0xe0) ^ 0x80) | (start_tile & 0x1f)];
			UINT8 tile_color = (BIT(tile_data, 4 + bit_pos) << 1) | BIT(tile_data, bit_pos);
			int color = (tile_idx << 2) | tile_color;


			// other cars sprites
			bool othercars = false;
			// TODO: other cars sprites


			// my car sprite
			bool mycar = false;
			int mycar_size = m_video_ctrl[1][3] & 0x20 ? 16 : 32;
			if ((m_video_ctrl[1][3] & 0x18) && x >= m_video_ctrl[1][2] && x < m_video_ctrl[1][2] + 4*8 && y > m_mycar_pos - mycar_size  && y < m_mycar_pos + mycar_size)
			{
				int hpos = x - mycar_x;
				int vpos = y > mycar_y ? ((y - mycar_y) ^ 0x1f) : mycar_y - y;
				int sprite_idx = (((m_video_ctrl[1][3] & 0x18)) << 2) | (hpos & 0x1c) | (((m_video_ctrl[1][3] & 0x06) >> 1) ^ 0x03);

				if (y <= mycar_y - 16  || y >= mycar_y + 16)            sprite_idx ^= 0x61;
				else if (m_cp_ruote && (m_video_ctrl[1][3] & 0x10))     sprite_idx ^= 0x01;

				int bitpos = 3 - (hpos & 3);
				UINT8 sprite_data = m_gfx2->base()[(sprite_idx << 5) | (vpos & 0x1f)];
				UINT8 sprite_color = (BIT(sprite_data, 4 + bitpos) << 1) | BIT(sprite_data, bitpos);

				if ((sprite_color & 3) != 3)
				{
					color = 0x100 + sprite_color;
					if (y > mycar_y - 16 && y < mycar_y + 16)
						mycar = true;
				}
			}

			if (cliprect.contains(x, y))
				bitmap.pix16(y, x) = color;

			// collisions
			UINT8 coll_prom_addr = BITSWAP8(tile_idx, 7, 6, 5, 4, 2, 0, 1, 3);
			UINT8 collisions = collisions_prom[((mycar && othercars) ? 0 : 0x80) | (inv ? 0x40 : 0) | (coll_prom_addr << 2) | (mycar ? 0 : 0x02) | (tile_color & 0x01)];
			m_collisions_ff |= ((m_collisions_clk ^ collisions) & collisions);
			m_collisions_clk = collisions;
		}
	}

	// characters
	for(int y=0;y<26;y++)
	{
		for(int x=0;x<40;x++)
		{
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				m_vram[y*40+x],
				0,
				0, 0,
				x*7,y*10,
				1);
		}
	}

	return 0;
}

static ADDRESS_MAP_START( monzagp_map, AS_PROGRAM, 8, monzagp_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END


READ8_MEMBER(monzagp_state::port_r)
{
	UINT8 data = 0xff;
	if (!(m_p1 & 0x01))             // 8350 videoram
	{
		//printf("ext 0 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
		int addr = ((m_p2 & 0x3f) << 5) | (offset & 0x1f);
		data = m_vram[addr];
	}
	if (!(m_p1 & 0x02))
	{
		printf("ext 1 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
	}
	if (!(m_p1 & 0x04))             // GFX
	{
		//printf("ext 2 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
		int addr = ((m_p2 & 0x7f) << 5) | (offset & 0x1f);
		data = m_gfx1->base()[addr];
	}
	if (!(m_p1 & 0x08))
	{
		//printf("ext 3 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
		data = m_in1->read();
	}
	if (!(m_p1 & 0x10))
	{
		//printf("ext 4 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
		data = (m_dsw->read() & 0x1f) | (m_in0->read() & 0xe0);
	}
	if (!(m_p1 & 0x20))
	{
		printf("ext 5 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
	}
	if (!(m_p1 & 0x40))             // digits
	{
		data = m_score_ram[BITSWAP8(offset, 3,2,1,0,7,6,5,4)];
		//printf("ext 6 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
	}
	if (!(m_p1 & 0x80))
	{
		//printf("ext 7 r P1:%02x P2:%02x %02x\n", m_p1, m_p2, offset);
		data = m_collisions_ff | (m_time_tick ? 0x10 : 0);
	}

	return data;
}

WRITE8_MEMBER(monzagp_state::port_w)
{
	if (!(m_p1 & 0x01))     // 8350 videoram
	{
		//printf("ext 0 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);

		int addr = ((m_p2 & 0x3f) << 5) | (offset & 0x1f);
		m_vram[addr] = data;
	}
	if (!(m_p1 & 0x02))
	{
		printf("ext 1 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);
	}
	if (!(m_p1 & 0x04))    // GFX
	{
		//printf("ext 2 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);
		int addr = ((m_p2 & 0x7f) << 5) | (offset & 0x1f);
		if (addr < 0x400)
		{
			static int pt[] = { 0x0e, 0x0c, 0x0d, 0x08, 0x09, 0x0a, 0x0b, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x0f };
			m_gfx1->base()[addr] = (pt[(data >> 4) & 0x0f] << 4) | pt[data & 0x0f];
			m_gfxdecode->gfx(0)->mark_dirty(addr >> 4);
		}
	}
	if (!(m_p1 & 0x08))
	{
		//printf("ext 3 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);
	}
	if (!(m_p1 & 0x10))
	{
		printf("ext 4 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);
	}
	if (!(m_p1 & 0x20))
	{
		//printf("ext 5 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);
	}
	if (!(m_p1 & 0x40))    // digits
	{
		//printf("ext 6 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);
		offs_t ram_offset = BITSWAP8(offset, 3,2,1,0,7,6,5,4);
		m_score_ram[ram_offset] = data & 0x0f;

		if ((ram_offset & 0x07) == 0)
		{
			// 74LS47 BCD-to-Seven-Segment Decoder
			static UINT8 bcd2hex[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x49, 0x78, 0x00 };
			output_set_digit_value(ram_offset >> 3, bcd2hex[data & 0x0f]);
		}
	}
	if (!(m_p1 & 0x80))
	{
		//printf("ext 7 w P1:%02x P2:%02x, %02x = %02x\n", m_p1, m_p2, offset, data);
		m_video_ctrl[0][(offset>>0) & 0x07] = data;
		m_video_ctrl[1][(offset>>3) & 0x07] = data;

		if (((offset>>0) & 0x07) == 0x04)           m_collisions_ff = 0;
		if (((offset>>3) & 0x07) == 0x04)           m_mycar_pos = 0xbf;

		if ((m_video_ctrl[1][3] & 1) == 0)
		{
			if (((offset>>3) & 0x07) == 0x00)       m_mycar_pos++;
			if (((offset>>3) & 0x07) == 0x01)       m_mycar_pos--;
		}

		if ((offset & 0x80) && (m_video_ctrl[1][3] & 0x01))
		{
			UINT8 steering_wheel = m_steering_wheel->read();
			if (steering_wheel & 0x01)              m_mycar_pos--;
			if (steering_wheel & 0x02)              m_mycar_pos++;
		}
	}
}

WRITE8_MEMBER(monzagp_state::port1_w)
{
//  printf("P1 %x = %x\n",space.device().safe_pc(),data);
	m_p1 = data;
}

READ8_MEMBER(monzagp_state::port2_r)
{
	return m_p2;
}

WRITE8_MEMBER(monzagp_state::port2_w)
{
//  printf("P2 %x = %x\n",space.device().safe_pc(),data);
	m_p2 = data;
}


static ADDRESS_MAP_START( monzagp_io, AS_IO, 8, monzagp_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(port_r, port_w)
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_WRITE(port1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(port2_r, port2_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( monzagp )
	PORT_START("WHEEL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )

	PORT_START("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Unused ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Unused ) )
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	7,10,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(1,1) },
	{ STEP16(0,8) },
	8*8*2
};

static const gfx_layout tile_layout =
{
	4, 32,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0,4) },
	{ STEP4(0,1) },
	{ STEP32(0,4*2) },
	32*4*2,
};

static const gfx_layout sprite_layout =
{
	4, 16,
	RGN_FRAC(1,1),
	2,
	{ STEP2(0,4) },
	{ STEP4(0,1) },
	{ STEP16(0,4*2) },
	16*4*2,
};


static GFXDECODE_START( monzagp )
	GFXDECODE_ENTRY( "gfx1", 0x0000, char_layout,   0, 8 )
	GFXDECODE_ENTRY( "gfx2", 0x0000, sprite_layout, 0, 8 )
	GFXDECODE_ENTRY( "gfx3", 0x0000, tile_layout,   0, 8 )
GFXDECODE_END

static MACHINE_CONFIG_START( monzagp, monzagp_state )
	MCFG_CPU_ADD("maincpu", I8035, 12000000/4) /* 400KHz ??? - Main board Crystal is 12MHz */
	MCFG_CPU_PROGRAM_MAP(monzagp_map)
	MCFG_CPU_IO_MAP(monzagp_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", monzagp_state, irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(monzagp_state, screen_update_monzagp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(monzagp_state, monzagp)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", monzagp)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("time_tick_timer", monzagp_state, time_tick_timer, attotime::from_hz(4))

	MCFG_NVRAM_ADD_NO_FILL("nvram")
MACHINE_CONFIG_END

ROM_START( monzagp )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "12.6a",       0x0000, 0x0400, CRC(35715718) SHA1(aa64cedf1f5898b109f643975722cf15a1c752ba) )
	ROM_LOAD( "13.7a",       0x0400, 0x0400, CRC(4e16bb68) SHA1(fb1d311a40145b3dccbd3d003a683c12898f43ff) )
	ROM_LOAD( "14.8a",       0x0800, 0x0400, CRC(16a1b36c) SHA1(6bc6bac37febb7c0fe18dc9b0a4e3a71ad1faafd) )
	ROM_LOAD( "15.9a",       0x0c00, 0x0400, CRC(ee6d9cc6) SHA1(0aa9efe812c1d4865fee2bbb1764a135dd642790) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "10.7d",       0x0400, 0x0400, CRC(d2bedd67) SHA1(9b75731d2701f5b9ce0446141c5cd55b05671ec1) )
	ROM_LOAD( "11.8d",       0x0800, 0x0400, CRC(47607a83) SHA1(91ce272c4af3e1994db71d2239b68879dd279347) )

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "9.10j",       0x0000, 0x0400, CRC(474ab63f) SHA1(6ba623d1768ed92b39e8f76c2f2eed7874955f1b) )
	ROM_LOAD( "6.4f",        0x0400, 0x0400, CRC(934d2070) SHA1(e742bfcb910e8747780d32ca66efd7e343190fb4) )
	ROM_LOAD( "7.3f",        0x0800, 0x0400, CRC(08f29f60) SHA1(9ca454e848aa986ff9eccaead3fec5076df2e4d3) )
	ROM_LOAD( "8.1f",        0x0c00, 0x0400, CRC(99ce2753) SHA1(f4540700ea909ba1be34ac2c33dafd8ec67a2bb7) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "5.9f",        0x0000, 0x0400, CRC(5abd1ef6) SHA1(1bc79225c1be2821930fdb8e821a70c7ac8683ab) )
	ROM_LOAD( "4.10f",       0x0400, 0x0400, CRC(a426a371) SHA1(d6023bebf6924d1820e631ee53896100e5b256a5) )
	ROM_LOAD( "3.12f",       0x0800, 0x0400, CRC(e5591074) SHA1(ac756ee605d932d7c1c3eddbe2b9c6f78dad6ce8) )
	ROM_LOAD( "2.13f",       0x0c00, 0x0400, BAD_DUMP CRC(1943122f) SHA1(3d343314fcb594560b4a280e795c8cea4a3200c9) ) /* missing, so use rom from below. Not confirmed to 100% same */

	ROM_REGION( 0x10000, "unk1", 0 )
	ROM_LOAD( "1.9c",        0x0000, 0x0400, CRC(005d5fed) SHA1(145a860751ef7d99129b7242aacac7a4e1e14a51) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "63s140.1",    0x0000, 0x0100, CRC(5123c83e) SHA1(d8ff06af421d3dae65bc9b0a081ed56249ef61ab) )
	ROM_LOAD( "74s287.2",    0x0100, 0x0100, CRC(a5488f72) SHA1(e7cd61a5577e2935b1ffa9dc17ca9da9b1196668) )
	ROM_LOAD( "63s140.3",    0x0200, 0x0100, CRC(eebbe52a) SHA1(14af033871cad4e35c391bce4435e7cf1ba146f7) )
	ROM_LOAD( "63s140.4",    0x0300, 0x0100, CRC(b89961a3) SHA1(99070a12e66764d21fd38ce4318ee0929daea465) )
	ROM_LOAD( "63s140.5",    0x0400, 0x0100, CRC(82c92620) SHA1(51d65156ebb592ff9e6375da7aa279325482fd5f) )
	ROM_LOAD( "63s140.6",    0x0500, 0x0100, CRC(8274f838) SHA1(c3518c668bda98759b1b1d4690062ced6c639efe) )
	ROM_LOAD( "74s287.7",    0x0600, 0x0100, CRC(3248ba56) SHA1(d449f4be8df1b4189afca55a4cf0cc2e19eb4dd4) )
ROM_END

// bootleg hardware seems identical, just bad quality pcb
ROM_START( monzagpb )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "m12c.6a",     0x0000, 0x0400, CRC(35715718) SHA1(aa64cedf1f5898b109f643975722cf15a1c752ba) )
	ROM_LOAD( "m13c.7a",     0x0400, 0x0400, CRC(4e16bb68) SHA1(fb1d311a40145b3dccbd3d003a683c12898f43ff) )
	ROM_LOAD( "m14.8a",      0x0800, 0x0400, CRC(16a1b36c) SHA1(6bc6bac37febb7c0fe18dc9b0a4e3a71ad1faafd) )
	ROM_LOAD( "m15bi.9a",    0x0c00, 0x0400, CRC(ee6d9cc6) SHA1(0aa9efe812c1d4865fee2bbb1764a135dd642790) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "m10.7d",      0x0400, 0x0400, CRC(19db00af) SHA1(c73da9c2fdbdb1b52a7354ba169af43b26fcb4cc) ) /* differs from above */
	ROM_LOAD( "m11.8d",      0x0800, 0x0400, CRC(5b4a7ffa) SHA1(50fa073437febe516065cd83fbaf85b596c4f3c8) ) /* differs from above */

	ROM_REGION( 0x1000, "gfx2", 0 )
	ROM_LOAD( "m9.10j",      0x0000, 0x0400, CRC(474ab63f) SHA1(6ba623d1768ed92b39e8f76c2f2eed7874955f1b) )
	ROM_LOAD( "m6.4f",       0x0400, 0x0400, CRC(934d2070) SHA1(e742bfcb910e8747780d32ca66efd7e343190fb4) )
	ROM_LOAD( "m7.3f",       0x0800, 0x0400, CRC(08f29f60) SHA1(9ca454e848aa986ff9eccaead3fec5076df2e4d3) )
	ROM_LOAD( "m8.1f",       0x0c00, 0x0400, CRC(99ce2753) SHA1(f4540700ea909ba1be34ac2c33dafd8ec67a2bb7) )

	ROM_REGION( 0x1000, "gfx3", 0 )
	ROM_LOAD( "m5.9f",       0x0000, 0x0400, CRC(5abd1ef6) SHA1(1bc79225c1be2821930fdb8e821a70c7ac8683ab) )
	ROM_LOAD( "m4.10f",      0x0400, 0x0400, CRC(a426a371) SHA1(d6023bebf6924d1820e631ee53896100e5b256a5) )
	ROM_LOAD( "m3.12f",      0x0800, 0x0400, CRC(e5591074) SHA1(ac756ee605d932d7c1c3eddbe2b9c6f78dad6ce8) )
	ROM_LOAD( "m2.13f",      0x0c00, 0x0400, CRC(1943122f) SHA1(3d343314fcb594560b4a280e795c8cea4a3200c9) )

	ROM_REGION( 0x10000, "unk1", 0 )
	ROM_LOAD( "m1.9c",       0x0000, 0x0400, CRC(005d5fed) SHA1(145a860751ef7d99129b7242aacac7a4e1e14a51) )

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "6300.1",      0x0000, 0x0100, CRC(5123c83e) SHA1(d8ff06af421d3dae65bc9b0a081ed56249ef61ab) )
	ROM_LOAD( "6300.2",      0x0100, 0x0100, CRC(8274f838) SHA1(c3518c668bda98759b1b1d4690062ced6c639efe) ) /* differs from above */
	ROM_LOAD( "6300.3",      0x0200, 0x0100, CRC(eebbe52a) SHA1(14af033871cad4e35c391bce4435e7cf1ba146f7) )
	ROM_LOAD( "6300.4",      0x0300, 0x0100, CRC(b89961a3) SHA1(99070a12e66764d21fd38ce4318ee0929daea465) )
	ROM_LOAD( "6300.5",      0x0400, 0x0100, CRC(82c92620) SHA1(51d65156ebb592ff9e6375da7aa279325482fd5f) )
	ROM_LOAD( "6300.6",      0x0500, 0x0100, CRC(8274f838) SHA1(c3518c668bda98759b1b1d4690062ced6c639efe) )
	ROM_LOAD( "6300.7",      0x0600, 0x0100, CRC(3248ba56) SHA1(d449f4be8df1b4189afca55a4cf0cc2e19eb4dd4) )
ROM_END


GAMEL( 1981, monzagp,  0,       monzagp, monzagp, driver_device, 0, ROT270, "Olympia", "Monza GP", MACHINE_NOT_WORKING|MACHINE_NO_SOUND, layout_monzagp )
GAMEL( 1981, monzagpb, monzagp, monzagp, monzagp, driver_device, 0, ROT270, "bootleg", "Monza GP (bootleg)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND, layout_monzagp )
