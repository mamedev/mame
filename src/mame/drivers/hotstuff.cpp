// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* TAS 5 REEL system? by Olympic Video Gaming */

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc146818.h"
#include "machine/z80scc.h"
#include "emupal.h"
#include "screen.h"


class hotstuff_state : public driver_device
{
public:
	hotstuff_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bitmapram(*this, "bitmapram"),
		m_maincpu(*this, "maincpu"),
		m_rtc(*this, "rtc") { }

	void hotstuff(machine_config &config);

private:
	required_shared_ptr<uint16_t> m_bitmapram;
	virtual void video_start() override;
	uint32_t screen_update_hotstuff(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<mc146818_device> m_rtc;
	void hotstuff_map(address_map &map);
};


void hotstuff_state::video_start()
{
}

/* the first 0x20 bytes in every 0x200 (each line) of video ram are the colour data, providing a palette of 16 RGB444 colours for that line */

uint32_t hotstuff_state::screen_update_hotstuff(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count, y,yyy,x,xxx;
	uint16_t row_palette_data[0x10];
	rgb_t row_palette_data_as_rgb32_pen_data[0x10];

	yyy=512;xxx=512*2;

	count = 0;
	for (y = 0; y < yyy; y++)
	{
		// the current palette is stored in the first 0x20 bytes of each row!
		int p;

		for (p=0;p<0x10;p++)
		{
			row_palette_data[p] = m_bitmapram[count+p];

			row_palette_data_as_rgb32_pen_data[p] = rgb_t( (row_palette_data[p] & 0x0f00)>>4, (row_palette_data[p] & 0x00f0)>>0, (row_palette_data[p] & 0x000f)<<4  );

		}

		for(x = 0; x < xxx; x++)
		{
			{
				bitmap.pix32(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0xf000)>>12];
				x++;
				bitmap.pix32(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0x0f00)>>8];
				x++;
				bitmap.pix32(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0x00f0)>>4];
				x++;
				bitmap.pix32(y, x) = row_palette_data_as_rgb32_pen_data[(m_bitmapram[count] &0x000f)>>0];
			}

			count++;
		}
	}

	return 0;
}

void hotstuff_state::hotstuff_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x0fffff).noprw(); //ROM AM_REGION("data", 0)

	map(0x400000, 0x40ffff).ram();

	map(0x600000, 0x600003).rw("scc1", FUNC(z80scc_device::ba_cd_inv_r), FUNC(z80scc_device::ba_cd_inv_w));
	map(0x620000, 0x620003).rw("scc2", FUNC(z80scc_device::ba_cd_inv_r), FUNC(z80scc_device::ba_cd_inv_w));
	map(0x680000, 0x680001).lrw8("rtc_rw",
								 [this](address_space &space, offs_t offset, u8 mem_mask) {
									 return m_rtc->read(space, offset^1, mem_mask);
								 },
								 [this](address_space &space, offs_t offset, u8 data, u8 mem_mask) {
									 m_rtc->write(space, offset^1, data, mem_mask);
								 });

	map(0x980000, 0x9bffff).ram().share("bitmapram");
}

static INPUT_PORTS_START( hotstuff )
INPUT_PORTS_END

MACHINE_CONFIG_START(hotstuff_state::hotstuff)

	MCFG_DEVICE_ADD("maincpu", M68000, 16000000)
	MCFG_DEVICE_PROGRAM_MAP(hotstuff_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(128*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA((0x10*4)+8, 101*8-1, 0*8, 33*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(hotstuff_state, screen_update_hotstuff)

	MCFG_PALETTE_ADD("palette", 0x200)

	scc8530_device& scc1(SCC8530N(config, "scc1", 4915200));
	scc1.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_4);

	scc8530_device& scc2(SCC8530N(config, "scc2", 4915200));
	scc2.out_int_callback().set_inputline(m_maincpu, M68K_IRQ_5);

	MC146818(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set_inputline("maincpu", M68K_IRQ_1);
MACHINE_CONFIG_END



ROM_START( hotstuff )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "hot stuff game u6,68000.bin", 0x00000, 0x80000, CRC(65f6a72f) SHA1(3a6d489ec3bf351018e279605d42f10b0a2c61b1) )

	ROM_REGION( 0x80000, "data", 0 ) /* 68000 Data? */
	ROM_LOAD16_WORD_SWAP( "hot stuff symbol u8,68000.bin", 0x00000, 0x80000, CRC(f154a157) SHA1(92ae0fb977e2dcc0377487d768f95c6e447e990b) )
ROM_END

GAME( ????, hotstuff, 0, hotstuff, hotstuff, hotstuff_state, empty_init, ROT0, "Olympic Video Gaming", "Olympic Hot Stuff (TAS 5 Reel System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
