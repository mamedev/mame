/***************************************************************************

    Skeleton driver for Husky Hunter 2

    TODO:
    - Everything - this is just a skeleton


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/hd61830.h"
#include "rendlay.h"

class hunter2_state : public driver_device
{
public:
	hunter2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(hunter2);
	DECLARE_WRITE8_MEMBER(porte0_w);
	DECLARE_PALETTE_INIT(hunter2);

private:
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(hunter2_mem, AS_PROGRAM, 8, hunter2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE(0x4000, 0x7fff) AM_READ_BANK("bankr1") AM_WRITE_BANK("bankw1")
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bankr2") AM_WRITE_BANK("bankw2")
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(hunter2_io, AS_IO, 8, hunter2_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("lcdc", hd61830_device, data_r, data_w)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("lcdc", hd61830_device, status_r, control_w)
	AM_RANGE(0xe0, 0xe0) AM_WRITE(porte0_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( hunter2 )
INPUT_PORTS_END

/*
data   bank0    bank1    bank2
00     00       01       02
01     00       01       03
02     00       01       04
....
09     00       01       11
80     16       17       18
....
8F     61       62       63
*/
WRITE8_MEMBER( hunter2_state::porte0_w )
{
	if (data < 0x0a)
	{
		membank("bankr0")->set_entry(0);
		membank("bankr1")->set_entry(0);
		membank("bankr2")->set_entry(data);
		membank("bankw0")->set_entry(0);
		membank("bankw1")->set_entry(0);
		membank("bankw2")->set_entry(0);
	}
	else
	if ((data >= 0x80) && (data <= 0x8f))
	{
		data -= 0x70;
		membank("bankr0")->set_entry(data);
		membank("bankr1")->set_entry(data);
		membank("bankr2")->set_entry(data);
		membank("bankw0")->set_entry(data);
		membank("bankw1")->set_entry(data);
		membank("bankw2")->set_entry(data);
	}
}

void hunter2_state::machine_reset()
{
	membank("bankr0")->set_entry(0);
	membank("bankr1")->set_entry(0);
	membank("bankr2")->set_entry(0);
	membank("bankw0")->set_entry(0);
	membank("bankw1")->set_entry(0);
	membank("bankw2")->set_entry(0);
}

// it is presumed that writing to rom will go nowhere
DRIVER_INIT_MEMBER( hunter2_state, hunter2 )
{
	UINT8 *rom = memregion("roms")->base();
	UINT8 *ram = memregion("rams")->base();
	membank("bankr0")->configure_entries( 0, 10, &rom[0x00000], 0x0000);
	membank("bankr0")->configure_entries(16, 16, &ram[0x00000], 0xc000);
	membank("bankr1")->configure_entries( 0, 10, &rom[0x04000], 0x0000);
	membank("bankr1")->configure_entries(16, 16, &ram[0x04000], 0xc000);
	membank("bankr2")->configure_entries( 0, 10, &rom[0x08000], 0x4000);
	membank("bankr2")->configure_entries(16, 16, &ram[0x08000], 0xc000);
	membank("bankw0")->configure_entries( 0, 10, &ram[0xc0000], 0x0000);
	membank("bankw0")->configure_entries(16, 16, &ram[0x00000], 0xc000);
	membank("bankw1")->configure_entries( 0, 10, &ram[0xc0000], 0x0000);
	membank("bankw1")->configure_entries(16, 16, &ram[0x04000], 0xc000);
	membank("bankw2")->configure_entries( 0, 10, &ram[0xc0000], 0x0000);
	membank("bankw2")->configure_entries(16, 16, &ram[0x08000], 0xc000);
}

PALETTE_INIT_MEMBER(hunter2_state, hunter2)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static MACHINE_CONFIG_START( hunter2, hunter2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NSC800, 4000000)
	MCFG_CPU_PROGRAM_MAP(hunter2_mem)
	MCFG_CPU_IO_MAP(hunter2_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(80)
	MCFG_SCREEN_UPDATE_DEVICE("lcdc", hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(240, 128)
	MCFG_SCREEN_VISIBLE_AREA(0, 239, 0, 63)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(hunter2_state, hunter2)
	MCFG_DEVICE_ADD("lcdc", HD61830, XTAL_4_9152MHz/2/2) // unknown clock
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( hunter2 )
	ROM_REGION(0x30000, "roms", ROMREGION_ERASEFF) // board has space for 6 roms, but only 2 are populated normally
	ROM_LOAD( "tr032kx8mrom0.ic50", 0x0000, 0x8000, CRC(694d252c) SHA1(b11dbf24faf648596d92b1823e25a8e4fb7f542c) )
	ROM_LOAD( "tr032kx8mrom1.ic51", 0x8000, 0x8000, CRC(82901642) SHA1(d84f2bbd2e9e052bd161a313c240a67918f774ad) )

	// 48 x 4k blocks plus 1 sinkhole
	ROM_REGION(0xc4000, "rams", ROMREGION_ERASEVAL(0xa5)) // board can have up to 736k of ram, but 192k is standard
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE           INIT      COMPANY   FULLNAME       FLAGS */
COMP( 1981, hunter2, 0,      0,       hunter2,   hunter2, hunter2_state,  hunter2,  "Husky", "Hunter 2", GAME_IS_SKELETON )
