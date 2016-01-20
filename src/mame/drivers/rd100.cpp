// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

    Data RD100

    2015-10-02 Skeleton [Robbbert]

    Nothing is known about this system, except that it uses a 6809 CPU.
    No manuals, schematic or circuit description have been found.

*********************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"



class rd100_state : public driver_device
{
public:
	rd100_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(rd100);
	DECLARE_MACHINE_RESET(rd100);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( rd100_mem, AS_PROGRAM, 8, rd100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0xffff) AM_ROM AM_REGION("roms", 0x800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( rd100_io, AS_IO, 8, rd100_state )
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( rd100 )
INPUT_PORTS_END

UINT32 rd100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
//  for (int y = 0; y < 32*8; y++)
//  {
//      offs_t offset = (y / 8) * 128;

//      for (int sx = 0; sx < 64; sx++)
//      {
//          UINT8 code = m_video_ram[offset++];
//          UINT8 attr = m_video_ram[offset++];

//          offs_t char_offs = ((code & 0x7f) << 3) | (y & 0x07);
//          if (BIT(code, 7)) char_offs = ((code & 0x7f) << 3) | ((y >> 1) & 0x07);

//          UINT8 data = m_char_rom->base()[char_offs];

//          rgb_t fg = m_palette->pen_color(attr & 0x07);
//          rgb_t bg = m_palette->pen_color((attr >> 3) & 0x07);

//          for (int x = 0; x < 6; x++)
//          {
//              bitmap.pix32(y, (sx * 6) + x) = BIT(data, 7) ? fg : bg;

//              data <<= 1;
//          }
//      }
//  }

	return 0;
}

DRIVER_INIT_MEMBER( rd100_state, rd100 )
{
}

MACHINE_RESET_MEMBER( rd100_state, rd100 )
{
}

static MACHINE_CONFIG_START( rd100, rd100_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)  // freq unknown
	MCFG_CPU_PROGRAM_MAP(rd100_mem)
	MCFG_CPU_IO_MAP(rd100_io)

	MCFG_MACHINE_RESET_OVERRIDE(rd100_state, rd100)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(250))
	MCFG_SCREEN_UPDATE_DRIVER(rd100_state, screen_update)
	MCFG_SCREEN_SIZE(64*6, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*6-1, 0, 32*8-1)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")
	//MCFG_GFXDECODE_ADD("gfxdecode", "palette", rd100)
MACHINE_CONFIG_END

ROM_START( rd100 )
	ROM_REGION( 0x8000, "roms", 0 )
	ROM_LOAD( "pak3-01.bin",  0x0000, 0x8000, CRC(cf5bbf01) SHA1(0673f4048d700b84c30781af23fbeabe0b994306) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   CLASS          INIT    COMPANY FULLNAME       FLAGS */
COMP( 1989, rd100,  0,      0,       rd100,     rd100,  rd100_state,   rd100,  "Data", "RD100", MACHINE_IS_SKELETON )
