/***************************************************************************

    AT&T Unix PC series

    Skeleton driver

    Note: The 68k core needs restartable instruction support for this
    to have a chance to run.

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/ram.h"
#include "machine/wd17xx.h"
#include "imagedev/flopdrv.h"
#include "unixpc.lh"


/***************************************************************************
    DRIVER STATE
***************************************************************************/

class unixpc_state : public driver_device
{
public:
	unixpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_ram(*this, RAM_TAG),
		  m_wd2797(*this, "wd2797"),
		  m_floppy(*this, FLOPPY_0)
	,
		m_mapram(*this, "mapram"),
		m_videoram(*this, "videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<device_t> m_wd2797;
	required_device<device_t> m_floppy;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_reset();

	DECLARE_READ16_MEMBER( line_printer_r );
	DECLARE_WRITE16_MEMBER( misc_control_w );
	DECLARE_WRITE16_MEMBER( disk_control_w );
	DECLARE_WRITE16_MEMBER( romlmap_w );

	DECLARE_WRITE_LINE_MEMBER( wd2797_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( wd2797_drq_w );

	required_shared_ptr<UINT16> m_mapram;
	required_shared_ptr<UINT16> m_videoram;
};


/***************************************************************************
    MEMORY
***************************************************************************/

WRITE16_MEMBER( unixpc_state::romlmap_w )
{
	if (BIT(data, 15))
		space.install_ram(0x000000, 0x3fffff, m_ram->pointer());
	else
		space.install_rom(0x000000, 0x3fffff, space.machine().root_device().memregion("bootrom")->base());
}

void unixpc_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// force ROM into lower mem on reset
	romlmap_w(program, 0, 0, 0xffff);

	// reset cpu so that it can pickup the new values
	m_maincpu->reset();
}


/***************************************************************************
    MISC
***************************************************************************/

READ16_MEMBER( unixpc_state::line_printer_r )
{
	UINT16 data = 0;

	data |= 1; // no dial tone detected
	data |= 1 << 1; // no parity error
	data |= 0 << 2; // hdc intrq
	data |= wd17xx_intrq_r(m_wd2797) << 3;

	logerror("line_printer_r: %04x\n", data);

	return data;
}

WRITE16_MEMBER( unixpc_state::misc_control_w )
{
	logerror("misc_control_w: %04x\n", data);

	output_set_value("led_0", !BIT(data,  8));
	output_set_value("led_1", !BIT(data,  9));
	output_set_value("led_2", !BIT(data, 10));
	output_set_value("led_3", !BIT(data, 11));
}


/***************************************************************************
    FLOPPY
***************************************************************************/

WRITE16_MEMBER( unixpc_state::disk_control_w )
{
	logerror("disk_control_w: %04x\n", data);

	floppy_mon_w(m_floppy, !BIT(data, 5));

	// bit 6 = floppy selected / not selected
	wd17xx_set_drive(m_wd2797, 0);
}

WRITE_LINE_MEMBER( unixpc_state::wd2797_intrq_w )
{
	logerror("wd2797_intrq_w: %d\n", state);
}

WRITE_LINE_MEMBER( unixpc_state::wd2797_drq_w )
{
	logerror("wd2797_drq_w: %d\n", state);
}


/***************************************************************************
    VIDEO
***************************************************************************/

UINT32 unixpc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 348; y++)
		for (int x = 0; x < 720/16; x++)
			for (int b = 0; b < 16; b++)
				bitmap.pix16(y, x * 16 + b) = BIT(m_videoram[y * (720/16) + x], b);

	return 0;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( unixpc_mem, AS_PROGRAM, 16, unixpc_state )
	AM_RANGE(0x000000, 0x3fffff) AM_RAMBANK("bank1")
	AM_RANGE(0x400000, 0x4007ff) AM_RAM AM_SHARE("mapram")
	AM_RANGE(0x420000, 0x427fff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x470000, 0x470001) AM_READ(line_printer_r)
	AM_RANGE(0x4a0000, 0x4a0001) AM_WRITE(misc_control_w)
	AM_RANGE(0x4e0000, 0x4e0001) AM_WRITE(disk_control_w)
	AM_RANGE(0x800000, 0xbfffff) AM_MIRROR(0x7fc000) AM_ROM AM_REGION("bootrom", 0)
	AM_RANGE(0xe10000, 0xe10007) AM_DEVREADWRITE8_LEGACY("wd2797", wd17xx_r, wd17xx_w, 0x00ff)
	AM_RANGE(0xe43000, 0xe43001) AM_WRITE(romlmap_w)
ADDRESS_MAP_END


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( unixpc )
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static const floppy_interface unixpc_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	NULL,
	NULL
};

static const wd17xx_interface unixpc_wd17xx_intf =
{
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(unixpc_state, wd2797_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(unixpc_state, wd2797_drq_w),
	{ FLOPPY_0, NULL, NULL, NULL }
};

static MACHINE_CONFIG_START( unixpc, unixpc_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", M68010, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(unixpc_mem)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(unixpc_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_20MHz, 896, 0, 720, 367, 0, 348)
	// vsync should actually last 17264 pixels

	MCFG_DEFAULT_LAYOUT(layout_unixpc)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")
	MCFG_RAM_EXTRA_OPTIONS("2M")

	// floppy
	MCFG_WD2797_ADD("wd2797", unixpc_wd17xx_intf)
	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, unixpc_floppy_interface)
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

// ROMs were provided by Michael Lee und imaged by Philip Pemberton
ROM_START( 3b1 )
	ROM_REGION16_BE(0x400000, "bootrom", 0)
	ROM_LOAD16_BYTE("72-00617.15c", 0x000000, 0x002000, CRC(4e93ff40) SHA1(1a97c8d32ec862f7f5fa1032f1688b76ea0672cc))
	ROM_LOAD16_BYTE("72-00616.14c", 0x000001, 0x002000, CRC(c61f7ae0) SHA1(ab3ac29935a2a587a083c4d175a5376badd39058))
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT   INIT  COMPANY  FULLNAME  FLAGS
COMP( 1985, 3b1,  0,      0,      unixpc,  unixpc, driver_device, 0,    "AT&T",  "3B1",    GAME_NOT_WORKING | GAME_NO_SOUND )
