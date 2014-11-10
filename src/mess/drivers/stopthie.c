#include "emu.h"
#include "cpu/tms0980/tms0980.h"

/* Layout */
#include "stopthie.lh"


class stopthie_state : public driver_device
{
public:
	stopthie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ8_MEMBER(stopthie_read_k);
	DECLARE_WRITE16_MEMBER(stopthie_write_o);
	DECLARE_WRITE16_MEMBER(stopthie_write_r);
	required_device<cpu_device> m_maincpu;
};



#define LOG 1

static INPUT_PORTS_START( stopthie )
INPUT_PORTS_END


READ8_MEMBER(stopthie_state::stopthie_read_k)
{
	UINT8 data = 0;

	if (LOG)
		logerror( "stopthie_read_k\n" );

	return data;
}


WRITE16_MEMBER(stopthie_state::stopthie_write_o)
{
	if (LOG)
		logerror( "stopthie_write_o: write %02x\n", data );
}


WRITE16_MEMBER(stopthie_state::stopthie_write_r)
{
	if (LOG)
		logerror( "stopthie_write_r: write %04x\n", data );
}


static const UINT16 stopthie_output_pla[0x20] =
{
	/* O output PLA configuration currently unknown */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};


static MACHINE_CONFIG_START( stopthie, stopthie_state )

	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", TMS0980, 500000 ) /* Clock is wrong */
	MCFG_TMS1XXX_OUTPUT_PLA( stopthie_output_pla )
	MCFG_TMS1XXX_READ_K( READ8( stopthie_state, stopthie_read_k ) )
	MCFG_TMS1XXX_WRITE_O( WRITE16( stopthie_state, stopthie_write_o ) )
	MCFG_TMS1XXX_WRITE_R( WRITE16( stopthie_state, stopthie_write_r ) )

	MCFG_DEFAULT_LAYOUT(layout_stopthie)
MACHINE_CONFIG_END

ROM_START( stopthie )
	ROM_REGION( 0x1000, "maincpu", 0 )
	/* Taken from patent 4341385, might have made mistakes when creating this rom */
	ROM_LOAD16_WORD( "stopthie.bin", 0x0000, 0x1000, BAD_DUMP CRC(63162ce9) SHA1(2ff88a139020c48869fcacc04b0786b27530a802) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   INIT    COMPANY            FULLNAME      FLAGS */
CONS( 1979, stopthie,   0,      0,      stopthie,   stopthie, driver_device,  0,      "Parker Brothers", "Stop Thief", GAME_NOT_WORKING | GAME_NO_SOUND)
