// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

    Itautec I7000

    driver by Felipe C. da S. Sanches <juca@members.fsf.org>
    with tech info provided by Alexandre Souza (a.k.a. Tabajara).

    The portuguese Wikipedia article available at
    http://pt.wikipedia.org/wiki/Itautec_I-7000
    also provides a technical overview of this machine:

    The I-7000 was the first computer manufactured by Itautec
    (http://www.itautec.com.br/pt-br/produtos). It was originally an 8 bit CP/M
    computer that became an IBM PC-XT clone in later hardware revisions which
    took the "I-7000 PC-XT" name.

    * Released in 1982
    * Operating System: SIM/M / BASIC
    * CPU: National NSC800 D-4 at 4,00 MHz
    * Memory: 64KB to 128KB
    * keyboards: 80 keys (with a reduced numerical keypad and function keys)
    * display:
     - 40 X 25 text
     - 80 X 25 text
     - 160 X 100 (8 colors)
     - 640 X 200 (monochrome, with an expansion board)
     - 320 X 200 (16 colors, with an expansion board)
    * Expansion slots:
     - 1 frontal cart slot
     - 4 internal expansion slots
    * Ports:
     - 1 composite video output for a color monitor
     - 2 cassete interfaces
     - 1 RS-232C serial port
     - 1 parallel interface
    * Storage:
     - Cassetes recorder
     - Up to 4 external floppy drives: 8" (FD/DD, 1,1MB) or 5" 1/4
     - Up to 1 external 10 MB hard-drive

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h" //CPU was actually a NSC800 (Z80 compatible)
#include "bus/generic/carts.h"

class i7000_state : public driver_device
{
public:
	i7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_card(*this, "cardslot"),
			m_videoram(*this, "videoram")
	{ }

	virtual void video_start();

	required_device<generic_slot_device> m_card;
	required_shared_ptr<UINT8> m_videoram;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT8 *m_char_rom;

//  DECLARE_READ8_MEMBER( i7000_io_r );
//  DECLARE_WRITE8_MEMBER( i7000_io_w );

	DECLARE_DRIVER_INIT(i7000);
	DECLARE_PALETTE_INIT(i7000);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( i7000_card );
};

DRIVER_INIT_MEMBER(i7000_state, i7000)
{
}

PALETTE_INIT_MEMBER(i7000_state, i7000)
{
	palette.set_pen_color(0, rgb_t(0x33, 0x33, 0x33));
	palette.set_pen_color(1, rgb_t(0xBB, 0xBB, 0xBB));
}

void i7000_state::video_start()
{
	// find memory regions
	m_char_rom = memregion("gfx1")->base();
}

UINT32 i7000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	offs_t addr = 0;

	for (int sy = 0; sy < 25; sy++)
	{
		for (int sx = 0; sx < 40; sx++)
		{
			UINT8 data = m_videoram[addr++];
			for (int y = 0; y < 8; y++)
			{
				int color = m_char_rom[data*8 + y];
				for (int x = 0; x < 8; x++)
				{
					bitmap.pix16(sy*8 + y, sx*8 + 7 - x) = (color & 1);
					color >>= 1;
				}
			}
		}
	}

	return 0;
}

/*FIXME: we still need to figure out the proper memory map
         for the maincpu and where the cartridge slot maps to. */
static ADDRESS_MAP_START(i7000_mem, AS_PROGRAM, 8, i7000_state)
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x1000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0xffff) AM_RAM AM_SHARE("videoram")
//    AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("cardslot", 0)
ADDRESS_MAP_END

/*
static ADDRESS_MAP_START( i7000_io , AS_IO, 8, i7000_state)
    ADDRESS_MAP_UNMAP_HIGH
    ADDRESS_MAP_GLOBAL_MASK (0xff)
    AM_RANGE(0x00, 0xff) AM_READWRITE(i7000_io_r, i7000_io_w)
ADDRESS_MAP_END
*/

DEVICE_IMAGE_LOAD_MEMBER( i7000_state, i7000_card )
{
	UINT32 size = m_card->common_get_size("rom");

	m_card->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_BIG);
	m_card->common_load_rom(m_card->get_rom_base(), size, "rom");

	return IMAGE_INIT_PASS;
}

#if 0
static const gfx_layout i7000_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                 /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( i7000 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, i7000_charlayout, 0, 8 )
GFXDECODE_END
#endif

static MACHINE_CONFIG_START( i7000, i7000_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NSC800, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(i7000_mem)
//  MCFG_CPU_IO_MAP(i7000_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(320, 200) /* 40x25 8x8 chars */
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)

	MCFG_SCREEN_UPDATE_DRIVER(i7000_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(i7000_state, i7000)

//  MCFG_GFXDECODE_ADD("gfxdecode", "palette", i7000)

	/* Cartridge slot */
	MCFG_GENERIC_CARTSLOT_ADD("cardslot", generic_romram_plain_slot, "i7000_card")
	MCFG_GENERIC_EXTENSIONS("rom")
	MCFG_GENERIC_LOAD(i7000_state, i7000_card)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("card_list", "i7000_card")
MACHINE_CONFIG_END

ROM_START( i7000 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "i7000_boot_v1_4r02_15_10_85_d52d.rom",  0x0000, 0x1000, CRC(622412e5) SHA1(bf187a095600fd46a739c35132a85b5f39b2f867) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "i7000_chargen.rom", 0x0000, 0x0800, CRC(7ba75183) SHA1(4af799f4a8bd385e1e4e5ece378df93e1133dc12) )

	ROM_REGION( 0x1000, "drive", 0 )
	ROM_LOAD( "i7000_drive_ci01.rom", 0x0000, 0x1000, CRC(d8d6e5c1) SHA1(93e7db42fbfaa8243973321c7fc8c51ed80780be) )

	ROM_REGION( 0x1000, "telex", 0 )
	ROM_LOAD( "i7000_telex_ci09.rom", 0x0000, 0x1000, CRC(c1c8fcc8) SHA1(cbf5fb600e587b998f190a9e3fb398a51d8a5e87) )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT                COMPANY    FULLNAME    FLAGS */
COMP( 1982, i7000,  0,      0,       i7000,     0,       i7000_state, i7000, "Itautec", "I-7000",   GAME_NOT_WORKING | GAME_NO_SOUND)
