// license:BSD-3-Clause
// copyright-holders:David Haywood

/*
Scorpion 6 uses an Actel A3P250 FPGA
paired with a Coldfire MCF53014
".sc6 files contain a *full* image of the used part of the flash (apart from boot loader)"
to update a machine they should be copied to a USB
"appears to be aes128-encrypted though, which is not really breakable"
there is a hacked version of the platform from Reflex Gaming which uses ".rgf" files and apparently has an unencrypted bootloader
--
A version with video exists, although all dumps just seem to be loose files (to be copied to a card?)
--
Many games existed on both Scorpion 5 and Scorpion 6 hardware
*/


#include "emu.h"
#include "speaker.h"


namespace {

class bfm_sc6_state : public driver_device
{
public:
	bfm_sc6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void bfm_sc6(machine_config &config);

private:
};

static INPUT_PORTS_START( bfm_sc6 )
INPUT_PORTS_END

void bfm_sc6_state::bfm_sc6(machine_config &config)
{
	// nothing yet
}

#define SC6_ROM_COMMON \
	ROM_REGION( 0x10000, "bootloader", ROMREGION_ERASE00 ) \
	ROM_LOAD( "bootloader.bin", 0x00000, 0x10000, NO_DUMP ) /* unknown size - is this common to the platform, or different for each game? */ \
	/* there is also onboard flash */


ROM_START( sc6dndem )
	SC6_ROM_COMMON
	ROM_REGION( 0x1000000, "update", ROMREGION_ERASE00 )
	ROM_LOAD( "dnem612n9561082.sc6", 0x00000, 0xe32200, CRC(d71f9100) SHA1(cef09f8046e70c61599cf1050146eefb9ddb2bf8) )
ROM_END

ROM_START( sc6dndema )
	SC6_ROM_COMMON
	ROM_REGION( 0x1000000, "update", ROMREGION_ERASE00 )
	ROM_LOAD( "dnem612p9561082.sc6", 0x00000, 0xe32200, CRC(e38fb341) SHA1(fd72d79c761161611e966bfa61562e09fda5929c) )
ROM_END

ROM_START( sc6dndemb )
	SC6_ROM_COMMON
	ROM_REGION( 0x1000000, "update", ROMREGION_ERASE00 )
	ROM_LOAD( "dnem611n9560933.sc6", 0x00000, 0xe32200, CRC(8d820ee9) SHA1(de88328bec54fb8cb8b8a3a924e503d58bc5d9c6) )
ROM_END

ROM_START( sc6dndemc )
	SC6_ROM_COMMON
	ROM_REGION( 0x1000000, "update", ROMREGION_ERASE00 )
	ROM_LOAD( "dnem611p9560933.sc6", 0x00000, 0xe32200, CRC(2f8b7a8d) SHA1(09dd59ad844909c5e99d196ace9599a70de6d300) )
ROM_END


} // anonymous namespace

GAME( 201?, sc6dndem,  0,         bfm_sc6, bfm_sc6, bfm_sc6_state, empty_init, ROT0, "BFM", "Deal or No Deal Easy Money (Scorpion 6, 9561082)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 201?, sc6dndema, sc6dndem,  bfm_sc6, bfm_sc6, bfm_sc6_state, empty_init, ROT0, "BFM", "Deal or No Deal Easy Money (Scorpion 6, 9561082, protocol)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 201?, sc6dndemb, sc6dndem,  bfm_sc6, bfm_sc6, bfm_sc6_state, empty_init, ROT0, "BFM", "Deal or No Deal Easy Money (Scorpion 6, 9560933)",           MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
GAME( 201?, sc6dndemc, sc6dndem,  bfm_sc6, bfm_sc6, bfm_sc6_state, empty_init, ROT0, "BFM", "Deal or No Deal Easy Money (Scorpion 6, 9560933, protocol)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
