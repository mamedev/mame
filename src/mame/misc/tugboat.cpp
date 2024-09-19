// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*******************************************************************************

Enter-Tech Tugboat (Moppet Video series, on improved El Grande hardware)
Moppet Video games were meant for young children. They made the games very simple.

6502 hooked up + preliminary video by Ryan Holtz

TODO:
- check how the score is displayed. I'm quite sure that score_w is supposed to
  access videoram scanning it by columns (like btime_mirrorvideoram_w), but
  the current implementation is a big kludge.
- colors might not be entirely accurate
  Suspect berenstn is using the wrong color PROM.
- convert to use the HD46505 device, it has two.

Be careful modifying IRQ timing, otherwise controls in tugboat won't work properly,
noticeable on the 2nd level.

--------------------------------------------------------------------------------

PCB notes:

Bottom board:
What appeared to be a cpu socket was a passthru to the top board.
4 other smaller chip sockets also passed thru to the top.

HD46505RP
4*SY2114-2
2*HD46821P
2*UPD5101L
AY-3-8912

10MHz

2532 u7
2532 u8
2532 u9

4716 u67
2532 u68
2532 u69
2532 u70

Top board:
Some ROM chips were labeled with a different location from the board.

SY6502
HD46505RP
4*SY2114-2

4716 u-168
4716 u-169
4716 u-170
4716 u-167

*******************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class tugboat_state : public driver_device
{
public:
	tugboat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram"),
		m_inputs(*this, "IN%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void tugboat(machine_config &config);
	void noahsark(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_vram;
	required_ioport_array<4> m_inputs;
	output_finder<2> m_lamps;

	u8 m_hd46505_regs[2][0x20] = { };
	u8 m_hd46505_reglatch[2] = { };
	u16 m_start_address[2] = { };
	u8 m_control[2] = { };
	u8 m_fine_x = 0;
	u8 m_fine_y = 0;

	template<int N> void hd46505_w(offs_t offset, u8 data);
	void score_w(offs_t offset, u8 data);
	u8 input_r();
	void control0_w(offs_t offset, u8 data, u8 mem_mask);
	void control1_w(offs_t offset, u8 data, u8 mem_mask);

	void tugboat_palette(palette_device &palette) const;
	void vblank_w(int state);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);

	void main_map(address_map &map) ATTR_COLD;
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void tugboat_state::machine_start()
{
	m_lamps.resolve();

	save_item(NAME(m_hd46505_regs));
	save_item(NAME(m_hd46505_reglatch));
	save_item(NAME(m_start_address));
	save_item(NAME(m_control));
	save_item(NAME(m_fine_x));
	save_item(NAME(m_fine_y));
}

void tugboat_state::video_start()
{
	m_gfxdecode->gfx(0)->set_granularity(8);
	m_gfxdecode->gfx(2)->set_granularity(8);
}



/*******************************************************************************
    Video Hardware
*******************************************************************************/

// there isn't the usual resistor array anywhere near the color PROM, just four 1k resistors.
void tugboat_state::tugboat_palette(palette_device &palette) const
{
	u8 const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < palette.entries(); i++)
	{
		int const brt = BIT(color_prom[i], 3) ? 0xff : 0x80;

		int const r = brt * BIT(color_prom[i], 0);
		int const g = brt * BIT(color_prom[i], 1);
		int const b = brt * BIT(color_prom[i], 2);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


// see mc6845.cpp, coarse scrolling is performed changing the start_addr register (0C/0D)
template<int N>
void tugboat_state::hd46505_w(offs_t offset, u8 data)
{
	if (offset == 0)
		m_hd46505_reglatch[N] = data & 0x1f;
	else
		m_hd46505_regs[N][m_hd46505_reglatch[N]] = data;
}

void tugboat_state::vblank_w(int state)
{
	if (state)
	{
		// latch display start address
		for (int i = 0; i < 2; i++)
			m_start_address[i] = m_hd46505_regs[i][0x0c] << 8 | m_hd46505_regs[i][0x0d];
	}
}


void tugboat_state::score_w(offs_t offset, u8 data)
{
	// to vram layer 1 column 29
	m_vram[0x91d + 32 * (offset ^ 8)] = ~data & 0xf;
}

void tugboat_state::draw_tilemap(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer)
{
	u16 addr = layer * 0x800 + (m_start_address[layer] & 0x3ff);

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			int attr = m_vram[addr | 0x400];
			int code = ((attr & 0x01) << 8) | m_vram[addr];
			int color = (attr & 0x3c) >> 2;
			int rgn = layer * 2 + ((attr & 0x02) >> 1);
			int transpen = -1, fx = 0, fy = 0;

			if (layer == 1)
			{
				transpen = m_gfxdecode->gfx(rgn)->depth() - 1;

				// score panel column doesn't scroll
				if (x != 29)
					fy = m_fine_y;
			}
			else
				fx = m_fine_x;

			m_gfxdecode->gfx(rgn)->transpen(bitmap,cliprect,
					code,
					color,
					0, 0,
					8 * x + fx,
					8 * y - fy,
					transpen);

			addr = (addr & 0x800) | ((addr + 1) & 0x3ff);
		}
	}
}

u32 tugboat_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	draw_tilemap(bitmap, cliprect, 0);
	draw_tilemap(bitmap, cliprect, 1);

	return 0;
}



/*******************************************************************************
    Misc. I/O
*******************************************************************************/

u8 tugboat_state::input_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 4; i++)
		if (BIT(m_control[1], i ^ 7))
			data &= m_inputs[i]->read();

	return data;
}

void tugboat_state::control0_w(offs_t offset, u8 data, u8 mem_mask)
{
	data |= ~mem_mask;
	data = ~data;

	// d1,d2: start lamps (it doesn't look like the default cabinet has them)
	m_lamps[0] = BIT(data, 1);
	m_lamps[1] = BIT(data, 2);

	// d5: coincounter
	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));

	// d6: clock layer 1 fine x scroll
	if (data & ~m_control[0] & 0x40)
		m_fine_x = (m_fine_x + 1) & 7;

	// d7: clear layer 1 fine x scroll
	if (data & 0x80)
		m_fine_x = 0;

	m_control[0] = data;
}

void tugboat_state::control1_w(offs_t offset, u8 data, u8 mem_mask)
{
	data |= ~mem_mask;
	data = ~data;

	// d2: clock layer 2 fine y scroll
	if (data & ~m_control[1] & 4)
		m_fine_y = (m_fine_y + 1) & 7;

	// d0: clear layer 2 fine y scroll
	if (data & 1)
		m_fine_y = 0;

	// d4-d7: input select
	m_control[1] = data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void tugboat_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).mirror(0x0100).ram();
	map(0x1060, 0x1061).w("aysnd", FUNC(ay8910_device::address_data_w));
	map(0x10a0, 0x10a1).w(FUNC(tugboat_state::hd46505_w<0>));
	map(0x10c0, 0x10c1).w(FUNC(tugboat_state::hd46505_w<1>));
	map(0x11e4, 0x11e7).rw(m_pia[0], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x11e8, 0x11eb).rw(m_pia[1], FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x18e0, 0x18ef).w(FUNC(tugboat_state::score_w)).nopr();
	map(0x2000, 0x2fff).ram().share(m_vram);
	map(0x4000, 0x7fff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( tugboat )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( noahsark )
	PORT_INCLUDE( tugboat )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
INPUT_PORTS_END

static INPUT_PORTS_START( berenstn )
	PORT_INCLUDE( noahsark )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*******************************************************************************
    GFX Layouts
*******************************************************************************/

static GFXDECODE_START( gfx_tugboat )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1,        0x80, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_8x8x3_planar, 0x80, 16 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_8x8x1,        0x00, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, gfx_8x8x3_planar, 0x00, 16 )
GFXDECODE_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void tugboat_state::tugboat(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 10_MHz_XTAL/8); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &tugboat_state::main_map);

	PIA6821(config, m_pia[0]);
	m_pia[0]->readpa_handler().set(FUNC(tugboat_state::input_r));
	m_pia[0]->writepb_handler().set(FUNC(tugboat_state::control0_w));

	PIA6821(config, m_pia[1]);
	m_pia[1]->readpa_handler().set_ioport("DSW");
	m_pia[1]->writepb_handler().set(FUNC(tugboat_state::control1_w));
	m_pia[1]->cb2_handler().set_nop();
	m_pia[1]->irqb_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_pia[1]->irqb_handler().append_inputline(m_maincpu, 0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(10_MHz_XTAL/2, 320, 8, 248, 264, 8, 240);
	m_screen->set_screen_update(FUNC(tugboat_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(m_pia[1], FUNC(pia6821_device::cb1_w));
	m_screen->screen_vblank().append(FUNC(tugboat_state::vblank_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tugboat);
	PALETTE(config, m_palette, FUNC(tugboat_state::tugboat_palette), 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8912(config, "aysnd", 10_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.5);
}


void tugboat_state::noahsark(machine_config &config)
{
	tugboat(config);

	// video hardware
	m_screen->screen_vblank().set(m_pia[1], FUNC(pia6821_device::cb1_w));
	m_screen->screen_vblank().append(FUNC(tugboat_state::vblank_w)).invert();
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( tugboat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tugboat_u-7.u7",     0x5000, 0x1000, CRC(e81d7581) SHA1(c76327e3b027a5a2af69f8cfafa1f828ad0ebdb1) )
	ROM_LOAD( "tugboat_u-8.u8",     0x6000, 0x1000, CRC(7525de06) SHA1(0722c7a0b89c55162227173679ffbe398ca350a2) )
	ROM_LOAD( "tugboat_u-9.u9",     0x7000, 0x1000, CRC(aa4ae687) SHA1(a212eed5d04d6197aa3484ff36059fd7998604a6) )

	ROM_REGION( 0x0800, "gfx1", ROMREGION_INVERT  )
	ROM_LOAD( "tugboat_u-67.u67",   0x0000, 0x0800, CRC(601c425b) SHA1(13ed54ba1307ba3f779293d88c19d0c0f2d91a96) )

	ROM_REGION( 0x3000, "gfx2", ROMREGION_INVERT  )
	ROM_LOAD( "tugboat_u-68.u68",   0x0000, 0x1000, CRC(d5835182) SHA1(f67c8f93e0d7dd1bf8e3a98756719d386c133d1c) )
	ROM_LOAD( "tugboat_u-69.u69",   0x1000, 0x1000, CRC(e6d25878) SHA1(de9096ef3108d031049be1e7f2c5e346d0bc0df1) )
	ROM_LOAD( "tugboat_u-70.u70",   0x2000, 0x1000, CRC(34ce2850) SHA1(8883126627ed8a1d2c3bed2a3d169ce35eafc8a3) )

	ROM_REGION( 0x0800, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "tugboat_u-167.u168", 0x0000, 0x0080, CRC(279042fd) SHA1(1361fff1bc532251bbd36b7b60776c2cc137cfba) )
	ROM_IGNORE( 0x0780 )

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_LOAD( "tugboat_u-168.u170", 0x0000, 0x0800, CRC(64d9f4d7) SHA1(3ff7fc099023512c33ec4583e91e6cbab903e7a8) )
	ROM_LOAD( "tugboat_u-169.u169", 0x0800, 0x0800, CRC(1a636296) SHA1(bcb18d714328ba3db2d16d74c47a985c16a0bbe2) )
	ROM_LOAD( "tugboat_u-170.u167", 0x1000, 0x0800, CRC(b9c9b4f7) SHA1(6685d580ae150d7c67bac2786ee4b7a2c28eddc3) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "nt2.u128",           0x0000, 0x0100, CRC(236672bf) SHA1(57482d0a23223ef7b211045ad28d3e41e90f961e) )
ROM_END


ROM_START( noahsark )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin", 0x4000, 0x1000, CRC(3579eeac) SHA1(f54435ac6b31cf81342de83965cf8a8503b26eb8) )
	ROM_LOAD( "u7.bin", 0x5000, 0x1000, CRC(64b0afae) SHA1(1fcc17490d1290565be38a817f783604bcefb8be) )
	ROM_LOAD( "u8.bin", 0x6000, 0x1000, CRC(02d53f62) SHA1(e51a583a548b4bdaf43d376d5d276325ee448d49) )
	ROM_LOAD( "u9.bin", 0x7000, 0x1000, CRC(d425b61c) SHA1(a8d9562435cc910916df4cd7e958468d88ff92e7) )

	ROM_REGION( 0x0800, "gfx1", ROMREGION_INVERT  )
	ROM_LOAD( "u67.bin",  0x0000, 0x0800, CRC(1a77605b) SHA1(8c25750f94895f5820ad4f1fa4ae1ea70ee0aee2) )

	ROM_REGION( 0x3000, "gfx2", ROMREGION_INVERT  )
	ROM_LOAD( "u68.bin", 0x0000, 0x1000, CRC(6a66eac8) SHA1(3a13c2f5ef45cdd8b8b5db07d8c1417a3304723a) )
	ROM_LOAD( "u69.bin", 0x1000, 0x1000, CRC(fa2c279c) SHA1(332fcfcfe605c4132114399c32932507b16752e5) )
	ROM_LOAD( "u70.bin", 0x2000, 0x1000, CRC(dcabc7c5) SHA1(68abfdedea518e3a5c90f9f72173e8c05e190535) )

	ROM_REGION( 0x0800, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "u168.bin", 0x0000, 0x0080, CRC(7fc7280f) SHA1(93bf46e421b580edf81177db85cb220073761c57) ) // labeled u-167
	ROM_IGNORE( 0x0780 )

	ROM_REGION( 0x3000, "gfx4", 0 )
	ROM_LOAD( "u170.bin", 0x0000, 0x1000, CRC(ba36641c) SHA1(df206dc4b6f2da7b60bdaa72c8175de928a630a4) ) // labeled u-168
	ROM_LOAD( "u169.bin", 0x1000, 0x1000, CRC(68c58207) SHA1(e09f9f8b5f1071fbf8a4883f75f296ec4bc0eca1) ) // labeled u-169
	ROM_LOAD( "u167.bin", 0x2000, 0x1000, CRC(76f16c5b) SHA1(a8a8f0ad7dcc57c2bf518fc5e2509ed8fb87f403) ) // labeled u-170

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "u128.bin", 0x0000, 0x0100, CRC(816784bd) SHA1(47181f4a6ab35c46796ca1d8c130b76f404c188d) )
ROM_END


ROM_START( berenstn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "u6.bin", 0x4000, 0x1000, CRC(e45275a2) SHA1(d788bc5a69b3cdb2596a3b354371ff88d39f6d46) )
	ROM_LOAD( "u7.bin", 0x5000, 0x1000, CRC(1984d787) SHA1(c13959c9be075400e9d1668b5404bc73f6db5fe4) )
	ROM_LOAD( "u8.bin", 0x6000, 0x1000, CRC(0c4d53b7) SHA1(45bd847fdb7bbfbe53d750003024ef3454faa6e6) )
	ROM_LOAD( "u9.bin", 0x7000, 0x1000, CRC(7e058e57) SHA1(e9506fa4ec693abf0dc4e4cbfd4b93bdbcfc9ba4) )

	ROM_REGION( 0x0800, "gfx1", ROMREGION_INVERT  )
	ROM_LOAD( "u67.bin",  0x0000, 0x0800, CRC(1a77605b) SHA1(8c25750f94895f5820ad4f1fa4ae1ea70ee0aee2) )

	ROM_REGION( 0x3000, "gfx2", ROMREGION_INVERT  )
	ROM_LOAD( "u68.bin", 0x0000, 0x1000, CRC(21bf375f) SHA1(52bc81a4f289a96edfab034445bcf639b1524ada) )
	ROM_LOAD( "u69.bin", 0x1000, 0x1000, CRC(9dc770f6) SHA1(5dc16fac72d68b521dbb415935f5e7f682c26d7f) )
	ROM_LOAD( "u70.bin", 0x2000, 0x1000, CRC(a810bd45) SHA1(8be531529174c5d4b4f164bd2397116b9d5350db) )

	ROM_REGION( 0x0800, "gfx3", ROMREGION_ERASEFF )
	ROM_LOAD( "u167.bin", 0x0000, 0x0080, CRC(7fc7280f) SHA1(93bf46e421b580edf81177db85cb220073761c57) )
	ROM_IGNORE( 0x0780 )

	ROM_REGION( 0x1800, "gfx4", 0 )
	ROM_LOAD( "u168.bin", 0x0000, 0x0800, CRC(af532ba3) SHA1(b196e294eaf4c25549278fd040b1dad2799e18d5) )
	ROM_LOAD( "u169.bin", 0x0800, 0x0800, CRC(07b6e660) SHA1(c755f63cc7c566e200fc11199bfac06a7e8f89e4) )
	ROM_LOAD( "u170.bin", 0x1000, 0x0800, CRC(73261eff) SHA1(19edd6957fceb3df12fd29cd5e156a5eb1c70710) )

	ROM_REGION( 0x0100, "proms", 0 ) // Not dumped, same label as Tugboat but is this actually correct?
	ROM_LOAD( "n.t.2-031j.24s10", 0x0000, 0x0100, BAD_DUMP CRC(236672bf) SHA1(57482d0a23223ef7b211045ad28d3e41e90f961e) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS          INIT        SCREEN  COMPANY             FULLNAME                                 FLAGS
GAME( 1982, tugboat,  0,      tugboat,  tugboat,  tugboat_state, empty_init, ROT90,  "Enter-Tech, Ltd.", "Tugboat",                               MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1983, noahsark, 0,      noahsark, noahsark, tugboat_state, empty_init, ROT90,  "Enter-Tech, Ltd.", "Noah's Ark",                            MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, berenstn, 0,      noahsark, berenstn, tugboat_state, empty_init, ROT90,  "Enter-Tech, Ltd.", "The Berenstain Bears in Bigpaw's Cave", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )
