// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Terak 8510A

        23/02/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"

class terak_state : public driver_device
{
public:
	terak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ16_MEMBER(terak_fdc_status_r);
	DECLARE_WRITE16_MEMBER(terak_fdc_command_w);
	DECLARE_READ16_MEMBER(terak_fdc_data_r);
	DECLARE_WRITE16_MEMBER(terak_fdc_data_w);
	UINT8 m_unit;
	UINT8 m_cmd;
	UINT16 m_data;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_terak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};

READ16_MEMBER( terak_state::terak_fdc_status_r )
{
	logerror("terak_fdc_status_r\n");
	if (m_cmd==3) {
		logerror("cmd is 3\n");
		return 0xffff;
	}
	return 0;
}

WRITE16_MEMBER( terak_state::terak_fdc_command_w )
{
	m_unit = (data >> 8) & 0x03;
	m_cmd  = (data >> 1) & 0x07;
	logerror("terak_fdc_command_w %04x [%d %d]\n",data,m_unit,m_cmd);
}

READ16_MEMBER( terak_state::terak_fdc_data_r )
{
	logerror("terak_fdc_data_r\n");
	return 0;
}

WRITE16_MEMBER( terak_state::terak_fdc_data_w )
{
	logerror("terak_fdc_data_w %04x\n",data);
}

static ADDRESS_MAP_START(terak_mem, AS_PROGRAM, 16, terak_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000,  0xf5ff ) AM_RAM // RAM

	// octal
	AM_RANGE( 0173000, 0173177 ) AM_ROM // ROM
	AM_RANGE( 0177000, 0177001 ) AM_READWRITE(terak_fdc_status_r,terak_fdc_command_w)
	AM_RANGE( 0177002, 0177003 ) AM_READWRITE(terak_fdc_data_r,terak_fdc_data_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( terak )
INPUT_PORTS_END


void terak_state::machine_reset()
{
}

void terak_state::video_start()
{
}

UINT32 terak_state::screen_update_terak(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


static MACHINE_CONFIG_START( terak, terak_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",T11, XTAL_4MHz)
	MCFG_T11_INITIAL_MODE(6 << 13)
	MCFG_CPU_PROGRAM_MAP(terak_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(terak_state, screen_update_terak)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( terak )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "terak.rom", 0173000, 0x0080, CRC(fd654b8e) SHA1(273a9933b68a290c5aedcd6d69faa7b1d22c0344))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT  COMPANY   FULLNAME       FLAGS */
COMP( ????, terak,  0,      0,       terak,     terak, driver_device,   0,    "Terak", "Terak 8510A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
