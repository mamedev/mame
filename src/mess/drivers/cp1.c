/***************************************************************************

        Kosmos CP-1

        06/03/2012 Skeleton driver.

        on board there is also 8155
        KEYBOARD Membrane keyboard, 57 keys
        6 * 7 seg led display

****************************************************************************/
#include "emu.h"
#include "cpu/mcs48/mcs48.h"

class cp1_state : public driver_device
{
public:
	cp1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	DECLARE_READ8_MEMBER(getp1);
	DECLARE_READ8_MEMBER(getp2);
	DECLARE_READ8_MEMBER(getbus);
	DECLARE_READ8_MEMBER(t0_r);
	DECLARE_READ8_MEMBER(t1_r);
	DECLARE_WRITE8_MEMBER(putp1);
	DECLARE_WRITE8_MEMBER(putp2);
	DECLARE_WRITE8_MEMBER(putbus);
};

READ8_MEMBER(cp1_state::getp1)
{
	logerror("getp1\n");
	return 0;
}
READ8_MEMBER(cp1_state::getp2)
{
	logerror("getp2\n");
	return 0;
}
READ8_MEMBER(cp1_state::getbus)
{
	logerror("getbus\n");
	return 0;
}
READ8_MEMBER(cp1_state::t0_r)
{
	logerror("t0_r\n");
	return 0;
}
READ8_MEMBER(cp1_state::t1_r)
{
	logerror("t1_r\n");
	return 0;
}
WRITE8_MEMBER(cp1_state::putp1)
{
	logerror("putp1\n");
}
WRITE8_MEMBER(cp1_state::putp2)
{
	logerror("putp2\n");
}
WRITE8_MEMBER(cp1_state::putbus)
{
	logerror("putbus\n");
}

static ADDRESS_MAP_START(cp1_mem, AS_PROGRAM, 8, cp1_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cp1_io , AS_IO, 8, cp1_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( MCS48_PORT_P1,    MCS48_PORT_P1)  AM_READWRITE( getp1, putp1 )
	AM_RANGE( MCS48_PORT_P2,    MCS48_PORT_P2)  AM_READWRITE( getp2, putp2 )
	AM_RANGE( MCS48_PORT_BUS,   MCS48_PORT_BUS) AM_READWRITE( getbus, putbus )
	AM_RANGE( MCS48_PORT_T0,    MCS48_PORT_T0)  AM_READ( t0_r )
	AM_RANGE( MCS48_PORT_T1,    MCS48_PORT_T1)  AM_READ( t1_r )
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( cp1 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( cp1, cp1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8048, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(cp1_mem)
	MCFG_CPU_IO_MAP(cp1_io)
MACHINE_CONFIG_END

/* ROM definition */
/*
  KOSMOS B
  <Mitsubishi Logo> M5L8049-136P-6
  JAPAN 83F301
*/

ROM_START( cp1 )
	ROM_REGION( 0x0800, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "b", "b" )
	ROMX_LOAD( "cp1-kosmos-b.rom", 0x0000, 0x0800, CRC(fea8a2b2) SHA1(c987b79a7b90fcbd58b66a69e95913f2655a1f0d), ROM_BIOS(1))
	// This is from 2716 eprom that was on board with I8039 instead of I8049
	ROM_SYSTEM_BIOS( 1, "2716", "2716" )
	ROMX_LOAD( "cp1-2716.bin",     0x0000, 0x0800, CRC(3a2caf0e) SHA1(ff4befcf82a664950186d3af1843fdef70d2209f), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1980, cp1,  0,       0,   cp1,    cp1, driver_device,  0,   "Kosmos",   "CP1 / Computer Praxis",      GAME_NOT_WORKING | GAME_NO_SOUND)
