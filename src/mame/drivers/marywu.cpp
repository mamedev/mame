// license:GPL2+
// copyright-holders:Felipe Sanches
/*************************************************************************
  
  This is a driver for a gambling board with a yet unknown name.
  The PCB is labeled with: WU- MARY-1A
  And there's a text string in the ROM that says: "Music by: SunKiss Chen"

  Driver by Felipe Sanches

  TODO:
  * Figure out where exactly all devices are mapped to (the devices are
    2 sound chips, the KC8279 controller the 2kb SRAM, the 8bit dipswitches,
    31 LEDs, 13 modules of double-digit 7-seg displays and 4 push-buttons).
  * we may also have user inputs from the coin slot and from the
    cabinet buttons, for making bets.
**************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ay8910.h"

class marywu_state : public driver_device
{
public:
    marywu_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
    { }
};

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, marywu_state )
    AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, marywu_state )
    AM_RANGE(0x8000, 0x87ff) AM_RAM //HM6116: 2kbytes of Static RAM /* This is a guess. Maybe it is also mirrored? */
    //AM_RANGE(0xB000, 0xB001) AM_NOP /*maybe this is where the keyboard/display "KC8279" controller is hooked up ? */
    AM_RANGE(0x9001, 0x9002) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_data_w) /* This is a guess */
    AM_RANGE(0x9003, 0x9004) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_data_w) /* This is a guess */
ADDRESS_MAP_END

static MACHINE_CONFIG_START( marywu , marywu_state )
    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu", I80C31, XTAL_10_738635MHz) //actual CPU is a Winbond w78c31b-24
    MCFG_CPU_PROGRAM_MAP(program_map)
    MCFG_CPU_IO_MAP(io_map)

    /* sound hardware */
    MCFG_SPEAKER_STANDARD_MONO("mono")
    MCFG_SOUND_ADD("ay1", AY8910, XTAL_10_738635MHz) /* should it be perhaps a fraction of the XTAL clock ? */
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
    
    MCFG_SOUND_ADD("ay2", AY8910, XTAL_10_738635MHz) /* should it be perhaps a fraction of the XTAL clock ? */
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

ROM_START( marywu )
	ROM_REGION( 0x8000, "maincpu", 0 )
    ROM_LOAD( "marywu_sunkiss_chen.rom", 0x0000, 0x8000, CRC(11f67c7d) SHA1(9c1fd1a5cc6e2b0d675f0217aa8ff21c30609a0c) )
ROM_END

/*    YEAR  NAME       PARENT   MACHINE   INPUT     STATE          INIT   ROT    COMPANY       FULLNAME          FLAGS  */
GAME( ????, marywu,    0,       marywu,   0,        driver_device, 0,     ROT0, "<unknown>", "<unknown> Labeled 'WU- MARY-1A' Music by: SunKiss Chen", MACHINE_NOT_WORKING )
