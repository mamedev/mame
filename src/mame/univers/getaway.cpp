// license:BSD-3-Clause
// copyright-holders:hap, Angelo Salese
/******************************************************************************

Universal Get A Way

Top-down arcade racing game inspired from Taito (Super) Speed Race.

Hardware notes:
- PCB label: UNIVERSAL 7812
- TMS9900 @ 3MHz
- 24KB ROM (6*TMM333P)
- 2KB SRAM(4*TMS4045), 24KB DRAM(48*TMS4030)
- discrete sound

Japanese-language flyers show an upright cabinet with a start button, steering
wheel and a control lever marked LOW at the upper end of the range and HIGH at
the lower end.  It appears to be spring-returned to the LOW position.  The
cabinet appears to have the position where an accelerator pedal would be
located covered with a panel.  The dumped set seems to be designed for this
cabinet.

English-language flyers show a sit-down cabinet with a gear shift lever,
accelerator pedal, and digital displays for high scores.  The set dumped set
doesn't have support for the additional I/O.

TODO:
- unknown DIP switches, and verify factory defaults;
- several unknowns in the video emulation:
  - score layer is a simplification hack, it is unknown how it should really
    cope RMW-wise against main layer. It also has wrong colors (different color
    base or overlay artwork, with extra bit output for taking priority?).
    The score background color should change from white(or is it cyan?) to red
    after Extended Play, the score digits themselves should always be black;
  - According to flyers, screen sides should have a green background color,
    it can't be an artwork overlay since it only occurs when the trees are
    on screen. However, the German flyer contains a cabinet photo and there
    is no green background. Other flyers could be hand-drawn pictures?;
  - do we need to offset X by 1 char-wise? Fills starts from 0x1f;
  - video timing is unknown, pixel clock XTAL is 10.816MHz;
  - blitter busy flag;
  - miscellanea, cfr. in documentation;
- sound emulation;
- lamps;
- undumped PROMs?

******************************************************************************/

#include "emu.h"

#include "cpu/tms9900/tms9900.h"

#include "emupal.h"
#include "screen.h"


namespace {

class getaway_state : public driver_device
{
public:
	getaway_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxrom(*this, "gfx")
		, m_screen(*this, "screen")
		, m_inputs(*this, "IN.%u", 0)
		, m_dsw(*this, "DSW.%u", 0)
		, m_wheel(*this, "WHEEL")
	{ }

	// machine configs
	void getaway(machine_config &config);

	// input functions
	ioport_value read_wheel() { return (m_wheel->read() - 0x08) & 0xff; }

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<tms9900_device> m_maincpu;
	required_region_ptr<u8> m_gfxrom;
	required_device<screen_device> m_screen;
	required_ioport_array<3> m_inputs;
	required_ioport_array<2> m_dsw;
	required_ioport m_wheel;

	void main_map(address_map &map);
	void io_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void io_w(offs_t offset, u8 data);
	template <unsigned N> u8 dsw_r(offs_t offset);
	template <unsigned N> u8 input_r(offs_t offset);
	u8 busy_r();

	u8 m_regs[0x10];
	u8 m_vram[0x6000];
	u8 m_score_vram[0x6000];
};

void getaway_state::machine_start()
{
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_vram, 0, sizeof(m_vram));
	memset(m_score_vram, 0, sizeof(m_score_vram));

	save_item(NAME(m_regs));
	save_item(NAME(m_vram));
	save_item(NAME(m_score_vram));
}



/******************************************************************************
    Video
******************************************************************************/

u32 getaway_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// apparently score overlay covers only the rightmost 3 columns
	const int x_overlay = 29*8;

	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x+=8)
		{
			u16 xi = x >> 3;
			u8 b = m_vram[0x0000 | (xi << 8 | y)];
			u8 g = m_vram[0x2000 | (xi << 8 | y)];
			u8 r = m_vram[0x4000 | (xi << 8 | y)];

			for (int i = 0; i < 8; i++)
				bitmap.pix(y, x + i) = BIT(r, i) << 2 | BIT(g, i) << 1 | BIT(b, i);

			if (x < x_overlay)
				continue;

			b = m_score_vram[0x0000 | (xi << 8 | y)];
			g = m_score_vram[0x2000 | (xi << 8 | y)];
			r = m_score_vram[0x4000 | (xi << 8 | y)];

			for (int i = 0; i < 8; i++)
			{
				u8 pix_data = BIT(r, i) << 2 | BIT(g, i) << 1 | BIT(b, i);
				if (pix_data != 0)
					bitmap.pix(y, x + i) = pix_data;
			}
		}
	}

	return 0;
}



/******************************************************************************
    I/O
******************************************************************************/

WRITE_LINE_MEMBER(getaway_state::vblank_irq)
{
	if (state)
		m_maincpu->pulse_input_line(INT_9900_INTREQ, 2 * m_maincpu->minimum_quantum_time());
}

void getaway_state::io_w(offs_t offset, u8 data)
{
	// writes 1 bit at a time
	u8 n = offset >> 3;
	u8 prev = m_regs[n];
	u8 iomask = 1 << (offset & 7);
	m_regs[n] = (m_regs[n] & ~iomask) | ((data & 1) ? iomask : 0);

	/*
	[0x00]
	x--- ---- coin counter or coin SFX (more likely former?)
	---- x--- coin lockout?
	-??? -??? outputs? sounds?

	[0x01]
	x--- ---- blitter trigger (0->1)
	-x-- ---- fill mode (1) / RMW (0)
	--?- ---- 1 on press start screen (lamp or blitter related)
	---- x--- destination VRAM select, (1) normal VRAM (0) score VRAM
	---- --?? unknown, (11) mostly, flips with (10) when starting from the right lane.

	[0x02]
	???? ????

	[0x03]
	yyyy yyyy y destination offset

	[0x04]
	xxxx xxxx x destination offset

	[0x05]
	ssss ssss source GFX ROM lower address

	[0x06]
	ccc- ---- color mask
	---S SSSS source GFX ROM upper address

	[0x07]
	???w wwww transfer width, in 8 pixel units
	        Notice that 0xff is set on POST, either full clear or NOP

	[0x08]
	hhhh hhhh transfer height, in scanline units

	[0x09]
	---- ---? 1 triggered when explosion occurs

	[0x0a]
	x--- ---- sound engine enable? (0)
	---x ---- sound filter?
	---- xxxx sound engine SFX strength?

	[0x0b]
	???? ???? (game writes 0x30)

	[0x0c]
	---- ---? (game writes 1)
	*/

	//popmessage("%02x %02x %02x|%02x %02x %02x %02x", m_regs[0], m_regs[1] & 0x37, m_regs[2], m_regs[9], m_regs[0xa], m_regs[0xb], m_regs[0x0c]);

	// start gfx rom->vram transfer
	if (n == 1 && ~prev & m_regs[1] & 0x80)
	{
		u16 src = m_regs[6] << 8 | m_regs[5];
		// several valid entries are drawn with color=0 cfr. tyres
		// flyer shows them as white so definitely xor-ed
		// TODO: may be applied at palette init time instead + score layer colors doesn't match flyer.
		u8 color_mask = (src >> 13) ^ 7;
		src &= 0x1fff;
		src <<= 3;

		u16 x = m_regs[4];
		u16 y = m_regs[3];

		u8 height = m_regs[8];
		u8 width = m_regs[7] & 0x1f;

		const bool fill_mode = (m_regs[1] & 0x40) == 0x40;
		const bool layer_bank = BIT(m_regs[1], 3);
		u8 *vram = (layer_bank) ? m_vram : m_score_vram;

		for (int count = 0; count < width; count ++)
		{
			for (int yi = 0; yi < height; yi++)
			{
				for (int xi = 0; xi < 8; xi++)
				{
					u16 x_offs = xi + x;
					u16 dest = ((x_offs >> 3) & 0x1f) << 8;
					dest += (yi + y) & 0xff;
					u8 pen_mask = 1 << (x_offs & 7);

					if (fill_mode)
					{
						for (int i = 0; i < 3; i++)
						{
							// reversed for score VRAM?
							if (layer_bank)
								vram[i * 0x2000 + dest] &= ~pen_mask;
							else
								vram[i * 0x2000 + dest] |= pen_mask;
						}
					}
					else
					{
						u8 src_data = (m_gfxrom[src >> 3] >> ((7-src) & 7)) & 1;

						for (int i = 0; i < 3; i++)
							if (BIT(color_mask, i))
							{
								u16 out_bank = i * 0x2000 + dest;
								if (src_data)
									vram[out_bank] |= pen_mask;
								else
									vram[out_bank] &= ~pen_mask;
							}
					}

					src = (src + 1) & 0xffff;
				}
			}

			x -= 8;
			x &= 0xff;
		}
	}
}

u8 getaway_state::busy_r()
{
	// TODO: blitter busy?
	return 0;
}

template <unsigned N> u8 getaway_state::input_r(offs_t offset)
{
	return BIT(m_inputs[N]->read(), offset);
}

template <unsigned N> u8 getaway_state::dsw_r(offs_t offset)
{
	return BIT(m_dsw[N]->read(), offset);
}


/******************************************************************************
    Address Maps
******************************************************************************/

void getaway_state::main_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x3800, 0x3fff).ram();
}

void getaway_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).w(FUNC(getaway_state::io_w));
	map(0x00, 0x09).r(FUNC(getaway_state::input_r<1>)); // accelerator
	map(0x0a, 0x19).r(FUNC(getaway_state::input_r<2>)); // steering wheel
	map(0x1a, 0x21).r(FUNC(getaway_state::dsw_r<1>));
	map(0x22, 0x2f).r(FUNC(getaway_state::dsw_r<0>));
	map(0x32, 0x35).r(FUNC(getaway_state::input_r<0>)); // coin + start
	map(0x36, 0x37).r(FUNC(getaway_state::busy_r));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( getaway )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN.1")
	PORT_BIT( 0x1f, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(15)

	PORT_START("IN.2")
	// steering wheel, signed byte, absolute values larger than 8 ignored
	PORT_BIT( 0xff, 0x00, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(getaway_state, read_wheel)

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x08, IPT_PADDLE ) PORT_MINMAX(0x00, 0x10) PORT_SENSITIVITY(5) PORT_KEYDELTA(15)

	PORT_START("DSW.0") // DTS-8 DIP switch @ location k6
	// TODO: defaults for these two, assume they have different quotas?
	PORT_DIPNAME( 0x07, 0x02, "Extended Play" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPSETTING(    0x03, "4000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x05, "6000" )
	PORT_DIPSETTING(    0x06, "7000" )
	PORT_DIPSETTING(    0x07, "8000" )
	PORT_DIPNAME( 0x38, 0x28, "Extra Play" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPSETTING(    0x08, "2000" )
	PORT_DIPSETTING(    0x10, "3000" )
	PORT_DIPSETTING(    0x18, "4000" )
	PORT_DIPSETTING(    0x20, "5000" )
	PORT_DIPSETTING(    0x28, "6000" )
	PORT_DIPSETTING(    0x30, "7000" )
	PORT_DIPSETTING(    0x38, "8000" )
	PORT_DIPNAME( 0x40, 0x00, "Language" )
	PORT_DIPSETTING(    0x00, "English" )
	PORT_DIPSETTING(    0x40, "Japanese" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW.1") // DNS04 DIP switch @ location m7
	// credit display is shown if both extended plays are on "None"
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit (again)" )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void getaway_state::getaway(machine_config &config)
{
	/* basic machine hardware */
	TMS9900(config, m_maincpu, 48_MHz_XTAL/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &getaway_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &getaway_state::io_map);
	m_maincpu->intlevel_cb().set_constant(2);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 30*8-1);
	m_screen->set_screen_update(FUNC(getaway_state::screen_update));
	m_screen->screen_vblank().set(FUNC(getaway_state::vblank_irq));
	m_screen->set_palette("palette");
	PALETTE(config, "palette", palette_device::BGR_3BIT);

	/* sound hardware */
	// TODO: discrete
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( getaway )
	ROM_REGION16_BE(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE("3901.a16", 0x0000, 0x1000, CRC(d7596a61) SHA1(6afdfadc4f13f8dc2abcc967536f70a999dd00ef))
	ROM_LOAD16_BYTE("3902.a14", 0x0001, 0x1000, CRC(67e67b65) SHA1(3a1d82acc05318c52b9ce1d71df1a9471fb1ffe7))
	ROM_LOAD16_BYTE("3903.a15", 0x2000, 0x1000, CRC(ad43edff) SHA1(cd52bd1984d7d10bdda39fa850ee6c164cc4316c))
	ROM_LOAD16_BYTE("3904.a13", 0x2001, 0x1000, CRC(2728304f) SHA1(45f5485b4036a211bb6c16283c2710aa8b8a0212))

	ROM_REGION(0x2000, "gfx", 0)
	ROM_LOAD("3905.f6", 0x0000, 0x1000, CRC(90546543) SHA1(c52e4e59aebd4a37ce2cfd077f85c36d49878493))
	ROM_LOAD("3906.f8", 0x1000, 0x1000, CRC(fd878838) SHA1(b161791d505f79578102148934a9f11dd9c4f4fe))
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        SCREEN  COMPANY      FULLNAME               FLAGS
GAME( 1979, getaway, 0,      getaway, getaway, getaway_state, empty_init, ROT270, "Universal", "Get A Way (upright)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
