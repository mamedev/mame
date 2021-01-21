// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Universal Get A Way

Hardware notes:
- PCB label: UNIVERSAL 7812
- TMS9900 @ 3MHz
- 24KB ROM (6*TMM333P)
- 2KB SRAM(4*TMS4045), 24KB DRAM(48*TMS4030)
- discrete sound

TODO:
- roms have a lot of empty contents, but it's probably ok
- dipswitches and game inputs (reads from I/O, but don't know which is which yet),
  PCB has 12 dipswitches, game has a steering wheel and shifter
- video emulation, it looks like a custom blitter
- video timing is unknown, pixel clock XTAL is 10.816MHz
- sound emulation
- lamps and 7segs
- undumped proms?

******************************************************************************/

#include "emu.h"

#include "cpu/tms9900/tms9900.h"

#include "emupal.h"
#include "screen.h"
//#include "speaker.h"


namespace {

class getaway_state : public driver_device
{
public:
	getaway_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxrom(*this, "gfx")
		, m_screen(*this, "screen")
		, m_inputs(*this, "IN.%u", 0)
		, m_dsw(*this, "DSW.%u", 0)
	{ }

	// machine configs
	void getaway(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<tms9900_device> m_maincpu;
	required_region_ptr<u8> m_gfxrom;
	required_device<screen_device> m_screen;
	required_ioport_array<3> m_inputs;
	required_ioport_array<2> m_dsw;

	void main_map(address_map &map);
	void io_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void io_w(offs_t offset, u8 data);
	template <int i> u8 dsw_r(offs_t offset);
	template <int i> u8 input_r(offs_t offset);
	u8 busy_r();

	u8 m_regs[0x10];
	u8 m_vram[0x6000];
	u8 m_tvram[0x6000];
};

void getaway_state::machine_start()
{
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_vram, 0, sizeof(m_vram));
	memset(m_tvram, 0, sizeof(m_tvram));

	save_item(NAME(m_regs));
	save_item(NAME(m_vram));
	save_item(NAME(m_tvram));
}



/******************************************************************************
    Video
******************************************************************************/

uint32_t getaway_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			u8 r = m_vram[0x0000 | (x << 8 | y)];
			u8 g = m_vram[0x2000 | (x << 8 | y)];
			u8 b = m_vram[0x4000 | (x << 8 | y)];

			for (int i = 0; i < 8; i++)
				bitmap.pix(y, x << 3 | (i ^ 7)) = BIT(b, i) << 2 | BIT(g, i) << 1 | BIT(r, i);

			r = m_tvram[0x0000 | (x << 8 | y)];
			g = m_tvram[0x2000 | (x << 8 | y)];
			b = m_tvram[0x4000 | (x << 8 | y)];

			for (int i = 0; i < 8; i++)
			{
				u8 pix_data = BIT(b, i) << 2 | BIT(g, i) << 1 | BIT(r, i);
				if (pix_data != 0)
					bitmap.pix(y, x << 3 | (i ^ 7)) = pix_data;
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

//#include "debugger.h"

void getaway_state::io_w(offs_t offset, u8 data)
{
	u8 n = offset >> 3;
	u8 bit = offset & 7;
	u8 mask = 1 << bit;
	data = (m_regs[n] & ~mask) | ((data & 1) ? mask : 0);
	
	// [0x0a]
	// x--- ---- sound engine enable? (0)
	// ---x ---- sound filter?
	// ---- xxxx sound engine SFX strength?
//	popmessage("%02x %02x %02x", m_regs[0], m_regs[1], m_regs[2]);
//	popmessage("%02x %02x %02x %02x\n", m_regs[9], m_regs[0xa], m_regs[0xb], m_regs[0x0c]);

	if (n == 1 && ~m_regs[n] & data & 0x80)
	{
		// start gfx rom->vram transfer?
		u16 src = m_regs[6] << 8 | m_regs[5];
		u8 color_mask = src >> 13;
		src &= 0x1fff;

		// TODO: this can select through the full range
		// iiii ifff
		u16 dest = m_regs[4] << 8 | m_regs[3];
//		u8 dmask = dest >> 13;
		dest &= 0x1fff;

		u8 height = m_regs[8];
		// 0xff set on POST
		u8 width = m_regs[7] & 0x1f;

		const bool fill_mode = (m_regs[1] & 0x40) == 0x40;
		// 0x0b sprites, 0x03 score, 0x20 used on press start button (unknown meaning)
		// TODO: bit 3 is more likely a switch between RMW vs. regular replace
		u8 *vram = (m_regs[1] & 0x08) ? m_vram : m_tvram;
//		if (m_regs[7] & 0xe0)
		//if (m_regs[1] & 0x08)
		//	printf("|src=%04x dst=%04x|h:%02x w:%02x| sm:%02x dm:%02x|%02x %02x\n", src, dest, height, width, smask, dmask, (m_regs[7] & 0xe0) >> 5, m_regs[1]);
//		printf("%02x\n", m_regs[7]);
//		machine().debug_break();
		
		for (int x = 0; x < width; x++)
		{
			u16 x_ptr = dest;
			for (int y = 0; y < height; y++)
			{
				u8 src_data = fill_mode ? 0 : m_gfxrom[src];
				for (int i = 0; i < 3; i++)
					vram[i * 0x2000 + dest] = BIT(color_mask, i) ? src_data : 0;

				src = (src + 1) & 0x1fff;
				dest = (dest + 1) & 0x1fff;
			}
			dest = x_ptr - 0x100;
			dest &= 0x1fff;
		}
	}

	m_regs[n] = data;
}

template <int i> u8 getaway_state::input_r(offs_t offset)
{
	return BIT(m_inputs[i]->read(), offset);
}

u8 getaway_state::busy_r()
{
	// blitter busy?
	return 0;
}

template <int i> u8 getaway_state::dsw_r(offs_t offset)
{
	return BIT(m_dsw[i]->read(), offset);
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
	map.unmap_value_high();
	map(0x00, 0xff).w(FUNC(getaway_state::io_w));
	map(0x00, 0x09).r(FUNC(getaway_state::input_r<1>)); // shifter
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
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN.1")
	// TODO: positional/pedal, covers the full 0-0x1f range
	// (is all of it actually allowed?)
	PORT_DIPNAME( 0x01, 0x00, "Shifter" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )

	PORT_START("IN.2")
	// TODO: steering wheel, 0x00 is neutral, range seems to be 0xe0-0x1f?
	// (bit 7 -> left)
	PORT_DIPNAME( 0x01, 0x00, "SYSB" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	
	// dips are two banks, a regular 8 banks one
	// and a tiny 4. They are labeled, hard to read from the provided pic :=(

	// "D1S-8"?
	PORT_START("DSW.0")
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
	
	// "DNS04"?
	PORT_START("DSW.1")
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
	//m_screen->set_visarea(0*8, 32*8-1, 4*8, 28*8-1);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(getaway_state::screen_update));
	m_screen->screen_vblank().set(FUNC(getaway_state::vblank_irq));
	m_screen->set_palette("palette");
	PALETTE(config, "palette", palette_device::RGB_3BIT);

	/* sound hardware */
	// TODO
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

//    YEAR  NAME     PARENT  MACHINE  INPUT    CLASS          INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAME( 1979, getaway, 0,      getaway, getaway, getaway_state, empty_init, ROT270, "Universal", "Get A Way", MACHINE_SUPPORTS_SAVE | MACHINE_IS_SKELETON )
