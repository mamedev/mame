// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Taito Field Goal driver

    set #1 / orig Taito PCB / sticker "AFN00004" / Field Goal
    set #2 / orig Taito PCB / sticker "MFN00001" / Field Goal

Differences between these sets include

    - ball speed
    - paddle color and position
    - scoring of bonus points
    - when bonus score reaches 1000...
        set #1: paddle gets sticky
        set #2: paddle reflects the ball vertically upward

***************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6800.h"
#include "machine/mb14241.h"

#include "emupal.h"
#include "screen.h"


namespace {

class fgoal_state : public driver_device
{
public:
	fgoal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mb14241(*this, "mb14241"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_video_ram(*this, "video_ram"),
		m_in1(*this, "IN1"),
		m_paddle(*this, "PADDLE%u", 0U)
	{ }

	void fgoal(machine_config &config);

	int _80_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<mb14241_device> m_mb14241;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// memory pointers
	required_shared_ptr<uint8_t> m_video_ram;

	// I/O ports
	required_ioport m_in1;
	required_ioport_array<2> m_paddle;

	// video-related
	bitmap_ind16 m_bgbitmap{};
	bitmap_ind16 m_fgbitmap{};
	uint8_t m_xpos = 0U;
	uint8_t m_ypos = 0U;
	uint8_t m_current_color = 0U;

	// misc
	uint8_t m_player = 0U;
	uint8_t m_row = 0U;
	uint8_t m_col = 0U;
	uint8_t m_prev_coin = 0U;
	emu_timer *m_interrupt_timer = nullptr;

	uint8_t analog_r();
	uint8_t nmi_reset_r();
	uint8_t irq_reset_r();
	uint8_t row_r();
	void row_w(uint8_t data);
	void col_w(uint8_t data);
	uint8_t address_hi_r();
	uint8_t address_lo_r();
	uint8_t shifter_reverse_r();
	void sound1_w(uint8_t data);
	void sound2_w(uint8_t data);
	void color_w(uint8_t data);
	void ypos_w(uint8_t data);
	void xpos_w(uint8_t data);

	TIMER_CALLBACK_MEMBER(interrupt_callback);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	static int intensity(int bits);
	unsigned video_ram_address();

	void cpu_map(address_map &map) ATTR_COLD;
};


void fgoal_state::color_w(uint8_t data)
{
	m_current_color = data & 3;
}


void fgoal_state::ypos_w(uint8_t data)
{
	m_ypos = data;
}


void fgoal_state::xpos_w(uint8_t data)
{
	m_xpos = data;
}


void fgoal_state::video_start()
{
	m_screen->register_screen_bitmap(m_fgbitmap);
	m_screen->register_screen_bitmap(m_bgbitmap);

	save_item(NAME(m_fgbitmap));
	save_item(NAME(m_bgbitmap));
}


uint32_t fgoal_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t const *VRAM = m_video_ram;

	// draw color overlay foreground and background

	if (m_player == 1 && (m_in1->read() & 0x40))
	{
		m_gfxdecode->gfx(0)->zoom_opaque(m_fgbitmap, cliprect,
			0, (m_player << 2) | m_current_color,
			1, 1,
			0, 16,
			0x40000,
			0x40000);

		m_gfxdecode->gfx(1)->zoom_opaque(m_bgbitmap, cliprect,
			0, 0,
			1, 1,
			0, 16,
			0x40000,
			0x40000);
	}
	else
	{
		m_gfxdecode->gfx(0)->zoom_opaque(m_fgbitmap, cliprect,
			0, (m_player << 2) | m_current_color,
			0, 0,
			0, 0,
			0x40000,
			0x40000);

		m_gfxdecode->gfx(1)->zoom_opaque(m_bgbitmap, cliprect,
			0, 0,
			0, 0,
			0, 0,
			0x40000,
			0x40000);
	}

	// the ball has a fixed color

	for (int y = m_ypos; y < m_ypos + 8; y++)
	{
		for (int x = m_xpos; x < m_xpos + 8; x++)
		{
			if (y < 256 && x < 256)
			{
				m_fgbitmap.pix(y, x) = 128 + 16;
			}
		}
	}

	// draw bitmap layer

	for (int y = 0; y < 256; y++)
	{
		uint16_t *const p = &bitmap.pix(y);

		uint16_t const *const FG = &m_fgbitmap.pix(y);
		uint16_t const *const BG = &m_bgbitmap.pix(y);

		for (int x = 0; x < 256; x += 8)
		{
			uint8_t v = *VRAM++;

			for (int n = 0; n < 8; n++)
			{
				if (v & (1 << n))
				{
					p[x + n] = FG[x + n];
				}
				else
				{
					p[x + n] = BG[x + n];
				}
			}
		}
	}
	return 0;
}


int fgoal_state::intensity(int bits)
{
	int v = 0;

	// contrary to the schematics pull-up resistors are 270 and not 390

	if (1)
		v += 0x2e; // 100 + 270

	if (bits & 1)
		v += 0x27; // 100 + 330

	if (bits & 2)
		v += 0xaa; // 100

	return v;
}


void fgoal_state::palette(palette_device &palette) const
{
	// for B/W screens PCB can be jumpered to use lower half of PROM

	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 128; i++)
	{
		uint8_t const color = color_prom[0x80 | i] & 63;
		palette.set_pen_color(i, intensity(color >> 4), intensity(color >> 2), intensity(color >> 0));
	}

	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(128 + 0 * 8 + i, rgb_t(0x2e, 0x80, 0x2e));
		palette.set_pen_color(128 + 1 * 8 + i, rgb_t(0x2e, 0x2e, 0x2e));
	}

	// ball is a fixed color
	palette.set_pen_color(128 + 16, intensity(0x38 >> 4), intensity(0x38 >> 2), intensity(0x38 >> 0));
}


TIMER_CALLBACK_MEMBER(fgoal_state::interrupt_callback)
{
	int coin = (m_in1->read() & 2);

	m_maincpu->set_input_line(0, ASSERT_LINE);

	if (!coin && m_prev_coin)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	m_prev_coin = coin;

	int scanline = m_screen->vpos() + 128;

	if (scanline > 256)
		scanline = 0;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline));
}


unsigned fgoal_state::video_ram_address()
{
	return 0x4000 | (m_row << 5) | (m_col >> 3);
}


uint8_t fgoal_state::analog_r()
{
	return m_paddle[m_player]->read(); // PCB can be jumpered to use a single dial
}


int fgoal_state::_80_r()
{
	return BIT(m_screen->vpos(), 7);
}

uint8_t fgoal_state::nmi_reset_r()
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0;
}


uint8_t fgoal_state::irq_reset_r()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}


uint8_t fgoal_state::row_r()
{
	return m_row;
}


void fgoal_state::row_w(uint8_t data)
{
	m_row = data;
	m_mb14241->shift_data_w(0);
}

void fgoal_state::col_w(uint8_t data)
{
	m_col = data;
	m_mb14241->shift_count_w(data);
}

uint8_t fgoal_state::address_hi_r()
{
	return video_ram_address() >> 8;
}

uint8_t fgoal_state::address_lo_r()
{
	return video_ram_address() & 0xff;
}

uint8_t fgoal_state::shifter_reverse_r()
{
	uint8_t v = m_mb14241->shift_result_r();

	return bitswap<8>(v, 0, 1, 2, 3, 4, 5, 6, 7);
}


void fgoal_state::sound1_w(uint8_t data)
{
	// BIT0 => SX2
	// BIT1 => SX1
	// BIT2 => SX1
	// BIT3 => SX1
	// BIT4 => SX1
	// BIT5 => SX1
	// BIT6 => SX1
	// BIT7 => SX1
}


void fgoal_state::sound2_w(uint8_t data)
{
	// BIT0 => CX0
	// BIT1 => SX6
	// BIT2 => N/C
	// BIT3 => SX5
	// BIT4 => SX4
	// BIT5 => SX3
	m_player = data & 1;
}


void fgoal_state::cpu_map(address_map &map)
{

	map(0x0000, 0x00ef).ram();

	map(0x00f0, 0x00f0).r(FUNC(fgoal_state::row_r));
	map(0x00f1, 0x00f1).r(FUNC(fgoal_state::analog_r));
	map(0x00f2, 0x00f2).portr("IN0");
	map(0x00f3, 0x00f3).portr("IN1");
	map(0x00f4, 0x00f4).r(FUNC(fgoal_state::address_hi_r));
	map(0x00f5, 0x00f5).r(FUNC(fgoal_state::address_lo_r));
	map(0x00f6, 0x00f6).r(m_mb14241, FUNC(mb14241_device::shift_result_r));
	map(0x00f7, 0x00f7).r(FUNC(fgoal_state::shifter_reverse_r));
	map(0x00f8, 0x00fb).r(FUNC(fgoal_state::nmi_reset_r));
	map(0x00fc, 0x00ff).r(FUNC(fgoal_state::irq_reset_r));

	map(0x00f0, 0x00f0).w(FUNC(fgoal_state::row_w));
	map(0x00f1, 0x00f1).w(FUNC(fgoal_state::col_w));
	map(0x00f2, 0x00f2).w(FUNC(fgoal_state::row_w));
	map(0x00f3, 0x00f3).w(FUNC(fgoal_state::col_w));
	map(0x00f4, 0x00f7).w(m_mb14241, FUNC(mb14241_device::shift_data_w));
	map(0x00f8, 0x00fb).w(FUNC(fgoal_state::sound1_w));
	map(0x00fc, 0x00ff).w(FUNC(fgoal_state::sound2_w));

	map(0x0100, 0x03ff).ram();
	map(0x4000, 0x7fff).ram().share(m_video_ram);

	map(0x8000, 0x8000).w(FUNC(fgoal_state::ypos_w));
	map(0x8001, 0x8001).w(FUNC(fgoal_state::xpos_w));
	map(0x8002, 0x8002).w(FUNC(fgoal_state::color_w));

	map(0xa000, 0xbfff).rom();
	map(0xd000, 0xffff).rom();
}


static INPUT_PORTS_START( fgoal )
	PORT_START("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )
	PORT_DIPNAME( 0x40, 0x40, "Display Coinage Settings" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x18, 0x18, "Options" ) // bit #4 comes from a jumper
	PORT_DIPSETTING(    0x00, "Clear All Helmets" )
	PORT_DIPSETTING(    0x08, "No Extra Ball" )
	PORT_DIPSETTING(    0x10, "No Extra Credit" )
	PORT_DIPSETTING(    0x18, "Default" )
	PORT_DIPNAME( 0x07, 0x05, "Initial Extra Credit Score" )
	PORT_DIPSETTING(    0x00, "9000" )
	PORT_DIPSETTING(    0x01, "17000" )
	PORT_DIPSETTING(    0x02, "28000" )
	PORT_DIPSETTING(    0x03, "39000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x05, "65000" )
	PORT_DIPSETTING(    0x06, "79000" )
	PORT_DIPSETTING(    0x07, "93000" )
	// extra credit score changes depending on player's performance

	PORT_START("IN1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(fgoal_state, _80_r) // 128V
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ))
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ))
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Language ))
	PORT_DIPSETTING(    0x00, DEF_STR( Japanese ))
	PORT_DIPSETTING(    0x10, DEF_STR( English ))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	// game freezes when analog controls read $00 or $ff
	PORT_START("PADDLE0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(1, 254) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("PADDLE1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(1, 254) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)
INPUT_PORTS_END

static const uint32_t gfxlayout_xoffset[64] =
{
	0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
	0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078,
	0x080, 0x088, 0x090, 0x098, 0x0a0, 0x0a8, 0x0b0, 0x0b8,
	0x0c0, 0x0c8, 0x0d0, 0x0d8, 0x0e0, 0x0e8, 0x0f0, 0x0f8,
	0x100, 0x108, 0x110, 0x118, 0x120, 0x128, 0x130, 0x138,
	0x140, 0x148, 0x150, 0x158, 0x160, 0x168, 0x170, 0x178,
	0x180, 0x188, 0x190, 0x198, 0x1a0, 0x1a8, 0x1b0, 0x1b8,
	0x1c0, 0x1c8, 0x1d0, 0x1d8, 0x1e0, 0x1e8, 0x1f0, 0x1f8
};

static const uint32_t gfxlayout_yoffset[64] =
{
	0x0000, 0x0200, 0x0400, 0x0600, 0x0800, 0x0a00, 0x0c00, 0x0e00,
	0x1000, 0x1200, 0x1400, 0x1600, 0x1800, 0x1a00, 0x1c00, 0x1e00,
	0x2000, 0x2200, 0x2400, 0x2600, 0x2800, 0x2a00, 0x2c00, 0x2e00,
	0x3000, 0x3200, 0x3400, 0x3600, 0x3800, 0x3a00, 0x3c00, 0x3e00,
	0x4000, 0x4200, 0x4400, 0x4600, 0x4800, 0x4a00, 0x4c00, 0x4e00,
	0x5000, 0x5200, 0x5400, 0x5600, 0x5800, 0x5a00, 0x5c00, 0x5e00,
	0x6000, 0x6200, 0x6400, 0x6600, 0x6800, 0x6a00, 0x6c00, 0x6e00,
	0x7000, 0x7200, 0x7400, 0x7600, 0x7800, 0x7a00, 0x7c00, 0x7e00
};

static const gfx_layout gfxlayout =
{
	64, 64,
	1,
	4,
	{ 4, 5, 6, 7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0,
	gfxlayout_xoffset,
	gfxlayout_yoffset
};


static GFXDECODE_START( gfx_fgoal )
	GFXDECODE_ENTRY( "tiles", 0, gfxlayout, 0x00, 8 ) // foreground
	GFXDECODE_ENTRY( "tiles", 0, gfxlayout, 0x80, 1 ) // background
GFXDECODE_END



void fgoal_state::machine_start()
{
	m_interrupt_timer = timer_alloc(FUNC(fgoal_state::interrupt_callback), this);

	save_item(NAME(m_xpos));
	save_item(NAME(m_ypos));
	save_item(NAME(m_current_color));
	save_item(NAME(m_player));
	save_item(NAME(m_row));
	save_item(NAME(m_col));
	save_item(NAME(m_prev_coin));
}

void fgoal_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(0));

	m_xpos = 0;
	m_ypos = 0;
	m_current_color = 0;
	m_player = 0;
	m_row = 0;
	m_col = 0;
	m_prev_coin = 0;
}

void fgoal_state::fgoal(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 10065000 / 10); // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &fgoal_state::cpu_map);

	// add shifter
	MB14241(config, m_mb14241);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(256, 263);
	m_screen->set_visarea(0, 255, 16, 255);
	m_screen->set_screen_update(FUNC(fgoal_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_fgoal);
	PALETTE(config, m_palette, FUNC(fgoal_state::palette), 128 + 16 + 1);

	// sound hardware
	// TODO: netlist
}


ROM_START( fgoal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tf04.m28", 0xa000, 0x0800, CRC(45fd7b03) SHA1(adc75a7fff6402c5c668ac28aec5d7c31c67c948) )
	ROM_RELOAD(           0xe000, 0x0800 )
	ROM_LOAD( "tf03.m31", 0xa800, 0x0800, CRC(01891c32) SHA1(013480dc970da83bda969506b2bd8865753a78ad) )
	ROM_RELOAD(           0xe800, 0x0800 )
	ROM_LOAD( "tf02.m38", 0xb000, 0x0800, CRC(c297d509) SHA1(a180e5203008db6b358dceee7349682ae3675c20) )
	ROM_RELOAD(           0xf000, 0x0800 )
	ROM_LOAD( "tf01.m46", 0xb800, 0x0800, CRC(1b0bfa5c) SHA1(768e14f08063cc022d7e18a9cb2197d64a9e1b8d) )
	ROM_RELOAD(           0xf800, 0x0800 )

	ROM_REGION( 0x1000, "tiles", 0 ) // overlay PROMs
	ROM_LOAD( "tf05.m11", 0x0000, 0x0400, CRC(925b78ab) SHA1(97d6e572658715dc4f6c37b98ba5352643fc8e27) )
	ROM_LOAD( "tf06.m4",  0x0400, 0x0400, CRC(3d2f007b) SHA1(7f4b6f3f08be8c886af3e2ccd3c0d93ae54d4649) )
	ROM_LOAD( "tf07.m12", 0x0800, 0x0400, CRC(0b1d01c4) SHA1(8680602fecd412e5136e1107618a2e0a59b37d08) )
	ROM_LOAD( "tf08.m5",  0x0c00, 0x0400, CRC(5cbc7dfd) SHA1(1a054dc72d25615ea6f903f6da8108033514fd1f) )

	ROM_REGION( 0x0100, "proms", ROMREGION_INVERT )
	ROM_LOAD_NIB_LOW ( "tf09.m13", 0x0000, 0x0100, CRC(b0fc4b80) SHA1(c6029f6d912275aa65302ca97281e10ccbf63159) )
	ROM_LOAD_NIB_HIGH( "tf10.m6",  0x0000, 0x0100, CRC(7b30b15d) SHA1(e9826a107b209e18d891ead341eda3d4523ce195) )
ROM_END


ROM_START( fgoala )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mf04.m28", 0xa000, 0x0800, CRC(acba21bc) SHA1(4a82e88555491883628a07f905d130380d5274f1) )
	ROM_RELOAD(           0xe000, 0x0800 )
	ROM_LOAD( "mf03.m31", 0xa800, 0x0800, CRC(4ce7462d) SHA1(ff02b4a831967c4e75e1d42e0679224b107d61bd) )
	ROM_RELOAD(           0xe800, 0x0800 )
	ROM_LOAD( "mf02.m38", 0xb000, 0x0800, CRC(5cd889b9) SHA1(7c8d810fed6d5e57c9b6a00e699f5b1d1253e84e) )
	ROM_RELOAD(           0xf000, 0x0800 )
	ROM_LOAD( "mf01.m46", 0xb800, 0x0800, CRC(9b9f5faa) SHA1(f944fff2c07e70f86fdd28fa5c9dc6c75ea2028b) )
	ROM_RELOAD(           0xf800, 0x0800 )
	ROM_LOAD( "mf05.m22", 0xd800, 0x0800, CRC(58082b8b) SHA1(72cd4153f7939cd33fc69ba82b44391fc19ae152) )

	ROM_REGION( 0x1000, "tiles", 0 ) // overlay PROMs
	ROM_LOAD( "tf05.m11", 0x0000, 0x0400, CRC(925b78ab) SHA1(97d6e572658715dc4f6c37b98ba5352643fc8e27) )
	ROM_LOAD( "tf06.m4",  0x0400, 0x0400, CRC(3d2f007b) SHA1(7f4b6f3f08be8c886af3e2ccd3c0d93ae54d4649) )
	ROM_LOAD( "tf07.m12", 0x0800, 0x0400, CRC(0b1d01c4) SHA1(8680602fecd412e5136e1107618a2e0a59b37d08) )
	ROM_LOAD( "tf08.m5",  0x0c00, 0x0400, CRC(5cbc7dfd) SHA1(1a054dc72d25615ea6f903f6da8108033514fd1f) )

	ROM_REGION( 0x0100, "proms", ROMREGION_INVERT )
	ROM_LOAD_NIB_LOW ( "tf09.m13", 0x0000, 0x0100, CRC(b0fc4b80) SHA1(c6029f6d912275aa65302ca97281e10ccbf63159) )
	ROM_LOAD_NIB_HIGH( "tf10.m6",  0x0000, 0x0100, CRC(7b30b15d) SHA1(e9826a107b209e18d891ead341eda3d4523ce195) )
ROM_END

} // anonymous namespace


GAME( 1979, fgoal,  0,     fgoal, fgoal, fgoal_state, empty_init, ROT90, "Taito", "Field Goal (set 1)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, fgoala, fgoal, fgoal, fgoal, fgoal_state, empty_init, ROT90, "Taito", "Field Goal (set 2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
