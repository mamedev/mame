// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    decstation.cpp: MIPS-based DECstation family

	WANTED: all boot ROM dumps except 5000/133, all TURBOchannel card ROM dumps

	Machine types:
		DECstation 3100 (PMAX/KN01):
			16.67 MHz R2000 with FPU and MMU
			24 MiB max RAM
			Serial: DEC "DZ" quad-UART (DC7085 gate array)
			SCSI: DEC "SII" SCSI interface (DC7061 gate array)
			Ethernet: AMD7990 "LANCE" controller
			Monochrome or color video on-board
		PMIN/KN01: 
			Cheaper PMAX, 12.5 MHz R2000, same as PMAX

		Personal DECstation 5000/xx (MAXine/KN02BA):
			20, 25, or 33 MHz R3000 or 100 MHz R4000
			40 MiB max RAM
			Serial: DEC "DZ" quad-UART for keyboard/mouse, SCC8530 for modem/printer
			SCSI: NCR53C94
			Ethernet: AMD7990 "LANCE" controller
			Audio: AMD AM79C30
			Color 1024x768 8bpp video on-board
			2 TURBOchannel slots

		DECstation 5000/1xx: (3MIN/KN02DA):
			20, 25, or 33 MHz R3000 or 100 MHz R4000
			128 MiB max RAM
			Serial: 2x SCC8530
			SCSI: NCR53C94
			Ethernet: AMD7990 "LANCE" controller
			No on-board video
			3 TURBOchannel slots

		DECstation 5000/200: (3MAX/KN02):
			25 MHz R3000
			480 MiB max RAM
			Serial: DEC "DZ" quad-UART
			SCSI: NCR53C94
			Ethernet: AMD7990 "LANCE" controllor

		DECstation 5000/240, 5000/261 (3MAX+/KN03)
			40 MHz R3400, or 120 MHz R4400.
			480 MiB max RAM
			Serial: 2x SCC8530
			SCSI: NCR53C94
			Ethernet: AMD7990 "LANCE" controller

****************************************************************************/

#include "emu.h"
#include "cpu/mips/r3000.h"
#include "machine/decioga.h"

class decstation_state : public driver_device
{
public:
	decstation_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_ioga(*this, "ioga")
		{ }

	void kn02da(machine_config &config);

	void init_decstation();

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;
	required_device<dec_ioga_device> m_ioga;
	void decstation_map(address_map &map);
};

/***************************************************************************
    VIDEO HARDWARE
***************************************************************************/

void decstation_state::video_start()
{
}

/***************************************************************************
    MACHINE FUNCTIONS
***************************************************************************/

void decstation_state::machine_start()
{
}

void decstation_state::machine_reset()
{
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void decstation_state::decstation_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).ram(); 	// full 128 MB
	map(0x1c000000, 0x1dffffff).m(m_ioga, FUNC(dec_ioga_device::map));
	map(0x1fc00000, 0x1fc3ffff).rom().region("user1", 0);
}

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

MACHINE_CONFIG_START(decstation_state::kn02da)
	MCFG_DEVICE_ADD( "maincpu", R3041, 33000000 ) // FIXME: Should be R2000
	MCFG_R3000_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_DEVICE_PROGRAM_MAP( decstation_map )

	MCFG_DEVICE_ADD("ioga", DECSTATION_IOGA, XTAL(12'500'000))
MACHINE_CONFIG_END

static INPUT_PORTS_START( decstation )
	PORT_START("UNUSED") // unused IN0
	PORT_BIT(0xffff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

void decstation_state::init_decstation()
{
}

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( ds5k133 )
	ROM_REGION32_LE( 0x40000, "user1", 0 )
	ROM_LOAD( "ds5000-133_005eb.bin", 0x000000, 0x040000, CRC(76a91d29) SHA1(140fcdb4fd2327daf764a35006d05fabfbee8da6) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY                 FULLNAME                FLAGS
COMP( 1992, ds5k133, 0,      0,      kn02da, decstation, decstation_state, init_decstation, "Digital Equipment Corporation", "DECstation 5000/133", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
