#include "emu.h"
#include "cpu/tms0980/tms0980.h"

/* Layout */
#include "merlin.lh"


class merlin_state : public driver_device
{
public:
	merlin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	DECLARE_READ8_MEMBER(read_k);
	DECLARE_WRITE16_MEMBER(write_o);
	DECLARE_WRITE16_MEMBER(write_r);
};



#define LOG 1

static INPUT_PORTS_START( merlin )
INPUT_PORTS_END


READ8_MEMBER(merlin_state::read_k)
{
	UINT8 data = 0xFF;

	if (LOG)
		logerror( "read_k\n" );

	return data;
}


WRITE16_MEMBER(merlin_state::write_o)
{
	if (LOG)
		logerror( "write_o: write %02x\n", data );
}


WRITE16_MEMBER(merlin_state::write_r)
{
	if (LOG)
		logerror( "write_r: write %04x\n", data );
}


static const tms0980_config merlin_tms0980_config =
{
	{
		/* O output PLA configuration currently unknown */
		{ 0x01, 0x01 }, { 0x02, 0x02 }, { 0x03, 0x03 }, { 0x04, 0x04 },
		{ 0x05, 0x05 }, { 0x06, 0x06 }, { 0x07, 0x07 }, { 0x08, 0x08 },
		{ 0x09, 0x09 }, { 0x0a, 0x0a }, { 0x0b, 0x0b }, { 0x0c, 0x0c },
		{ 0x0d, 0x0d }, { 0x0e, 0x0e }, { 0x0f, 0x0f }, { 0x10, 0x10 },
		{ 0x11, 0x11 }, { 0x12, 0x12 }, { 0x13, 0x13 }, { 0x14, 0x14 }
	},
	DEVCB_DRIVER_MEMBER(merlin_state, read_k),
	DEVCB_DRIVER_MEMBER16(merlin_state, write_o),
	DEVCB_DRIVER_MEMBER16(merlin_state, write_r)
};


static MACHINE_CONFIG_START( merlin, merlin_state )
	MCFG_CPU_ADD( "maincpu", TMS1100, 5000000 )	/* Clock is wrong */
	MCFG_CPU_CONFIG( merlin_tms0980_config )

	MCFG_DEFAULT_LAYOUT(layout_merlin)
MACHINE_CONFIG_END

ROM_START( merlin )
	ROM_REGION( 0x800, "maincpu", 0 )
	ROM_LOAD( "mp3404", 0x0000, 0x800, CRC(9362d9f9) SHA1(266d2a4a98cc33944a4fc7ed073ba9321bba8e05) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   INIT    COMPANY            FULLNAME      FLAGS */
CONS( 1978, merlin,     0,      0,      merlin,     merlin, driver_device,  0,      "Parker Brothers", "Merlin", GAME_IS_SKELETON )


