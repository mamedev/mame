// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/* TAS 5 REEL system? by Olympic Video Gaming */

#include "emu.h"
#include "cpu/m68000/m68000.h"


class hotstuff_state : public driver_device
{
public:
	hotstuff_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_bitmapram(*this, "bitmapram"),
		m_maincpu(*this, "maincpu") { }

	required_shared_ptr<UINT16> m_bitmapram;
	struct
	{
		UINT8 index;
	}m_ioboard;
	DECLARE_READ8_MEMBER(ioboard_status_r);
	DECLARE_READ8_MEMBER(ioboard_unk_r);
	DECLARE_WRITE8_MEMBER(ioboard_data_w);
	DECLARE_WRITE8_MEMBER(ioboard_reg_w);
	virtual void video_start() override;
	UINT32 screen_update_hotstuff(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


void hotstuff_state::video_start()
{
}

/* the first 0x20 bytes in every 0x200 (each line) of video ram are the colour data, providing a palette of 16 RGB444 colours for that line */

UINT32 hotstuff_state::screen_update_hotstuff(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count, y,yyy,x,xxx;
	UINT16 row_palette_data[0x10];
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

/* TODO: identify this ... */
READ8_MEMBER(hotstuff_state::ioboard_status_r)
{
	UINT8 res;

	printf("STATUS R\n");

	switch(m_ioboard.index)
	{
		case 0x0c: res = 0x80|0x10; break;
		default: res = 0; break;//machine().rand(); break;
	}

	return res;
}

READ8_MEMBER(hotstuff_state::ioboard_unk_r)
{
	printf("UNK R\n");

	return 0xff;
}

WRITE8_MEMBER(hotstuff_state::ioboard_data_w)
{
	printf("DATA %02x\n",data);
}

WRITE8_MEMBER(hotstuff_state::ioboard_reg_w)
{
	m_ioboard.index = data;
	printf("REG %02x\n",data);
}

static ADDRESS_MAP_START( hotstuff_map, AS_PROGRAM, 16, hotstuff_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x0fffff) AM_NOP //ROM AM_REGION("data", 0)

	AM_RANGE(0x400000, 0x40ffff) AM_RAM

	AM_RANGE(0x680000, 0x680001) AM_READWRITE8(ioboard_status_r,ioboard_data_w,0xff00)
	AM_RANGE(0x680000, 0x680001) AM_READWRITE8(ioboard_unk_r,ioboard_reg_w,0x00ff)

	AM_RANGE(0x980000, 0x9bffff) AM_RAM AM_SHARE("bitmapram")
ADDRESS_MAP_END

static INPUT_PORTS_START( hotstuff )
INPUT_PORTS_END

static MACHINE_CONFIG_START( hotstuff, hotstuff_state )

	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(hotstuff_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", hotstuff_state,  irq1_line_hold)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(128*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA((0x10*4)+8, 101*8-1, 0*8, 33*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(hotstuff_state, screen_update_hotstuff)

	MCFG_PALETTE_ADD("palette", 0x200)

MACHINE_CONFIG_END



ROM_START( hotstuff )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_WORD_SWAP( "hot stuff game u6 (68000).bin", 0x00000, 0x80000, CRC(65f6a72f) SHA1(3a6d489ec3bf351018e279605d42f10b0a2c61b1) )

	ROM_REGION( 0x80000, "data", 0 ) /* 68000 Data? */
	ROM_LOAD16_WORD_SWAP( "hot stuff symbol u8 (68000).bin", 0x00000, 0x80000, CRC(f154a157) SHA1(92ae0fb977e2dcc0377487d768f95c6e447e990b) )
ROM_END

GAME( ????, hotstuff,    0,        hotstuff,    hotstuff, driver_device,    0, ROT0,  "Olympic Video Gaming", "Olympic Hot Stuff (TAS 5 Reel System)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
