/********************************************************************************

    Pinball
    NSM (Lowen) : Hot Fire Birds

*********************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9900l.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"

class nsm_state : public driver_device
{
public:
	nsm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(ff_r);
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(nsm);
};


static ADDRESS_MAP_START( nsm_map, AS_PROGRAM, 8, nsm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf0fb) AM_READ_LEGACY(tms9995_internal1_r)
	AM_RANGE(0xffec, 0xffed) AM_DEVWRITE_LEGACY("ay1", ay8910_address_data_w)
	AM_RANGE(0xffee, 0xffef) AM_DEVWRITE_LEGACY("ay2", ay8910_address_data_w)
	AM_RANGE(0xfffc, 0xffff) AM_READ_LEGACY(tms9995_internal2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( nsm_io_map, AS_IO, 8, nsm_state )
	AM_RANGE(0x0000, 0x0001) AM_READ(ff_r)
	AM_RANGE(0x0010, 0x0011) AM_READNOP
	AM_RANGE(0x0060, 0x0061) AM_READNOP
	AM_RANGE(0x0f70, 0x0f7d) AM_WRITENOP
	AM_RANGE(0x0fe4, 0x0fff) AM_READNOP
	AM_RANGE(0x7f80, 0x7fd1) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( nsm )
INPUT_PORTS_END

READ8_MEMBER( nsm_state::ff_r ) { return 0xff; }

void nsm_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(nsm_state,nsm)
{
}

static MACHINE_CONFIG_START( nsm, nsm_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", TMS9995L, 11052000)
	MCFG_CPU_PROGRAM_MAP(nsm_map)
	MCFG_CPU_IO_MAP(nsm_io_map)

	/* Sound */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ay1", AY8912, 11052000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_SOUND_ADD("ay2", AY8912, 11052000/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Cosmic Flash (1985)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Hot Fire Birds (1985)
/-------------------------------------------------------------------*/
ROM_START(firebird)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nsmf02.764", 0x0000, 0x2000, CRC(236b5780) SHA1(19ef6e1fc900e5d94f615a4316f0383ed5ee939c))
	ROM_LOAD("nsmf03.764", 0x2000, 0x2000, CRC(d88c6ef5) SHA1(00edeefaab7e1141741aa132e6f7e56a911573be))
	ROM_LOAD("nsmf04.764", 0x4000, 0x2000, CRC(38a8add4) SHA1(74f781edc31aad07411feacad53c5f6cc73d09f4))
ROM_END

/*-------------------------------------------------------------------
/ Tag-Team Pinball (1986)
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ The Games (1985)
/-------------------------------------------------------------------*/

GAME(1985,  firebird,  0,  nsm,  nsm, nsm_state,  nsm,  ROT0,  "NSM",    "Hot Fire Birds",     GAME_NOT_WORKING | GAME_NO_SOUND | GAME_MECHANICAL)
