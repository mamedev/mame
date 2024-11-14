// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* TAS 5 REEL system? by Olympic Video Gaming */

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "emupal.h"
#include "screen.h"


namespace {

class hotstuff_state : public driver_device
{
public:
	hotstuff_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bitmapram(*this, "bitmapram"),
		m_maincpu(*this, "maincpu")
	{ }

	void hotstuff(machine_config &config);

private:
	required_shared_ptr<uint16_t> m_bitmapram;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_hotstuff(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void hotstuff_map(address_map &map) ATTR_COLD;
};


void hotstuff_state::video_start()
{
}

/* the first 0x20 bytes in every 0x200 (each line) of video ram are the colour data, providing a palette of 16 RGB444 colours for that line */

uint32_t hotstuff_state::screen_update_hotstuff(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t row_palette_data[0x10];
	rgb_t row_palette_data_as_rgb32_pen_data[0x10];

	int yyy=512, xxx=512*2;

	int count = 0;
	for (int y = 0; y < yyy; y++)
	{
		// the current palette is stored in the first 0x20 bytes of each row!
		for (int p = 0; p < 0x10; p++)
		{
			row_palette_data[p] = m_bitmapram[count + p];
			row_palette_data_as_rgb32_pen_data[p] = rgb_t(
					pal4bit(row_palette_data[p] >> 8),
					pal4bit(row_palette_data[p] >> 4),
					pal4bit(row_palette_data[p] >> 0));
		}

		for (int x = 0; x < xxx; )
		{
			bitmap.pix(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0xf000)>>12];
			x++;
			bitmap.pix(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0x0f00)>>8];
			x++;
			bitmap.pix(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0x00f0)>>4];
			x++;
			bitmap.pix(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0x000f)>>0];
			x++;

			count++;
		}
	}

	return 0;
}

void hotstuff_state::hotstuff_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x0fffff).noprw(); //.rom().region("data", 0);

	map(0x400000, 0x40ffff).ram();

	map(0x600000, 0x600003).rw("scc1", FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0x620000, 0x620003).rw("scc2", FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0x680000, 0x680000).rw("rtc", FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w));
	map(0x680001, 0x680001).w("rtc", FUNC(mc146818_device::address_w));

	map(0x980000, 0x9bffff).ram().share("bitmapram");
}

static INPUT_PORTS_START( hotstuff )
INPUT_PORTS_END

void hotstuff_state::hotstuff(machine_config &config)
{
	M68000(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hotstuff_state::hotstuff_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(128*8, 64*8);
	screen.set_visarea((0x10*4)+8, 101*8-1, 0*8, 33*8-1);
	screen.set_screen_update(FUNC(hotstuff_state::screen_update_hotstuff));

	PALETTE(config, "palette").set_entries(0x200);

	scc8530_device &scc1(SCC8530N(config, "scc1", 4915200));
	scc1.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_4);

	scc8530_device &scc2(SCC8530N(config, "scc2", 4915200));
	scc2.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_5);

	mc146818_device &rtc(MC146818(config, "rtc", XTAL(32'768)));
	rtc.irq().set_inputline("maincpu", M68K_IRQ_1);
}



ROM_START( hotstuff )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "hot stuff game u6,68000.bin", 0x00000, 0x80000, CRC(65f6a72f) SHA1(3a6d489ec3bf351018e279605d42f10b0a2c61b1) )

	ROM_REGION( 0x80000, "data", 0 ) /* 68000 Data? */
	ROM_LOAD16_WORD_SWAP( "hot stuff symbol u8,68000.bin", 0x00000, 0x80000, CRC(f154a157) SHA1(92ae0fb977e2dcc0377487d768f95c6e447e990b) )
ROM_END

} // anonymous namespace


GAME( ????, hotstuff, 0, hotstuff, hotstuff, hotstuff_state, empty_init, ROT0, "Olympic Video Gaming", "Olympic Hot Stuff (TAS 5 Reel System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
