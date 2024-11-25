// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*

 This is a driver for the Aceex DM2814 28.8kbps modem containing the following chips

 U1-U3   JRC 4558D Dual op amps
 U4      SN74LS374N
 U5      SN74LS04N
 U7      Rockwell RC288DPi integrated modem chip marked Mexico (40.320MHz TXC crystal Y1 nearby)
 U8      HD74LS153P
 U9      SN75C189N
 U10,U12 SN75C188N
 U11     HD74LS244P
 U13     SN74LS374N
 U14     Winbond W78C31B which is a 8031 derivate (45.342720MHz TXC crystal Y2 nearby)
 U15     SN74LS138N (inside the socket of U14)
 U16     Macronix MX27C512 64KB EPROM
 U17     HD74LS32P (inside the socket of U16)
 U18     LM386N
 U19     CSI CAT93C46P 1KB eeproom
 U20     Winbond W24257 32KB static RAM
 U21     SN74LS373N
 LED1-11 Front leds
 SP1     Speaker
 SW1     On/Off switch

 +------------------------------------------------------+
 |                                                      |
 |    U1    U2    U3                                PHONE
 |      U4      U5                                      |
 |LED1                                               LINE
 |LED2                                                  = R
 |LED3           U7         U8             U9      U10  D S
 |LED4    Y1                U11     SP1                 2 2
 |LED5                      U13                    U12  5 3
 |LED6                                                  = 2
 |LED7      U14           U16       U18                 |
 |LED8                                                 PWR
 |LED9-11                 U20       U21                SW1
 +------------------------------------------------------+

 The U16 EPROM contains the following string

  COPYWRITE BY TSAI CHIH-HWA

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"


namespace {

class aceex2814_state : public driver_device
{
public:
	aceex2814_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		,m_maincpu(*this, "maincpu")
	{ }

	void aceex2814(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	void aceex2814_map(address_map &map) ATTR_COLD;
};

void aceex2814_state::aceex2814_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

static INPUT_PORTS_START( aceex2814 )
INPUT_PORTS_END

void aceex2814_state::machine_start()
{
}

void aceex2814_state::machine_reset()
{
}

#define Y1_CLOCK 40320000
#define Y2_CLOCK 45342720
void aceex2814_state::aceex2814(machine_config &config)
{

	/* basic machine hardware */
	I80C31(config, m_maincpu, Y2_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &aceex2814_state::aceex2814_map);
}

ROM_START( aceex2814 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dm2814u16-194.bin", 0x00000, 0x10000, CRC(36dc423d) SHA1(0f350b7c533eb5270a72587ab3e050e5fe453006) )
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY              FULLNAME      FLAGS
COMP( 1995, aceex2814, 0,      0,      aceex2814, aceex2814, aceex2814_state, empty_init, "Aceex Corporation", "Aceex 2814", MACHINE_IS_SKELETON )
