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
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxrom(*this, "gfx"),
		m_screen(*this, "screen"),
		m_inputs(*this, "IN.%u", 0)
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
	required_ioport_array<1> m_inputs;

	void main_map(address_map &map);
	void io_map(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void io_w(offs_t offset, u8 data);
	u8 coin_r(offs_t offset);
	u8 busy_r();

	u8 m_regs[0x10];
	u8 m_vram[0x6000];
};

void getaway_state::machine_start()
{
	memset(m_regs, 0, sizeof(m_regs));
	memset(m_vram, 0, sizeof(m_vram));

	save_item(NAME(m_regs));
	save_item(NAME(m_vram));
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
	u8 n = offset >> 3;
	u8 bit = offset & 7;
	u8 mask = 1 << bit;
	data = (m_regs[n] & ~mask) | ((data & 1) ? mask : 0);

	if (n == 1 && ~m_regs[n] & data & 0x80)
	{
		// start gfx rom->vram transfer?
		u16 src = m_regs[6] << 8 | m_regs[5];
		//u8 smask = src >> 13;
		src &= 0x1fff;

		u16 dest = m_regs[4] << 8 | m_regs[3];
		u8 dmask = dest >> 13;
		dest &= 0x1fff;

		u8 bytes = m_regs[8];

		for (int count = 0; count < bytes; count++)
		{
			for (int i = 0; i < 3; i++)
				m_vram[i * 0x2000 + dest] = BIT(dmask, i) ? m_gfxrom[src] : 0;

			src = (src + 1) & 0x1fff;
			dest = (dest + 1) & 0x1fff;
		}
	}

	m_regs[n] = data;
}

u8 getaway_state::coin_r(offs_t offset)
{
	return BIT(m_inputs[0]->read(), offset);
}

u8 getaway_state::busy_r()
{
	// blitter busy?
	return 0;
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
	map(0x32, 0x35).r(FUNC(getaway_state::coin_r));
	map(0x36, 0x37).r(FUNC(getaway_state::busy_r));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( getaway )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
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
