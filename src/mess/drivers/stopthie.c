#include "emu.h"
#include "cpu/tms0980/tms0980.h"

/* Layout */
#include "stopthie.lh"


class stopthie_state : public driver_device
{
public:
	stopthie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

};



#define LOG 1

static INPUT_PORTS_START( stopthie )
INPUT_PORTS_END


static READ8_DEVICE_HANDLER( stopthie_read_k )
{
	UINT8 data = 0xFF;

	if (LOG)
		logerror( "stopthie_read_k\n" );

	return data;
}


static WRITE16_DEVICE_HANDLER( stopthie_write_o )
{
	if (LOG)
		logerror( "stopthie_write_o: write %02x\n", data );
}


static WRITE16_DEVICE_HANDLER( stopthie_write_r )
{
	if (LOG)
		logerror( "stopthie_write_r: write %04x\n", data );
}


static const tms0980_config stopthie_tms0980_config =
{
	{
		/* O output PLA configuration currently unknown */
		{ 0x01, 0x01 }, { 0x02, 0x02 }, { 0x03, 0x03 }, { 0x04, 0x04 },
		{ 0x05, 0x05 }, { 0x06, 0x06 }, { 0x07, 0x07 }, { 0x08, 0x08 },
		{ 0x09, 0x09 }, { 0x0a, 0x0a }, { 0x0b, 0x0b }, { 0x0c, 0x0c },
		{ 0x0d, 0x0d }, { 0x0e, 0x0e }, { 0x0f, 0x0f }, { 0x10, 0x10 },
		{ 0x11, 0x11 }, { 0x12, 0x12 }, { 0x13, 0x13 }, { 0x14, 0x14 }
	},
	stopthie_read_k,
	stopthie_write_o,
	stopthie_write_r
};


static MACHINE_CONFIG_START( stopthie, stopthie_state )
	MCFG_CPU_ADD( "maincpu", TMS0980, 5000000 )	/* Clock is wrong */
	MCFG_CPU_CONFIG( stopthie_tms0980_config )

	MCFG_DEFAULT_LAYOUT(layout_stopthie)
MACHINE_CONFIG_END

ROM_START( stopthie )
	ROM_REGION( 0x1000, "maincpu", 0 )
	/* Taken from patent 4341385, might have made mistakes when creating this rom */
	ROM_LOAD16_WORD( "stopthie.bin", 0x0000, 0x1000, BAD_DUMP CRC(49ef83ad) SHA1(407151f707aa4a62b7e034a1bcb957c42ea36707) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   INIT    COMPANY            FULLNAME      FLAGS */
CONS( 1979, stopthie,   0,      0,      stopthie,   stopthie, driver_device,  0,      "Parker Brothers", "Stop Thief", GAME_NOT_WORKING | GAME_NO_SOUND)

