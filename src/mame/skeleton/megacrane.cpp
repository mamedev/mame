// license:BSD-3-Clause
// copyright-holders:stonedDiscord
/****************************************************************************

    Skeleton driver for Elaut Megacrane

    This is the control board for a crane game.
    It plays music during gameplay and voice effects when inserting a coin or
    when the game is over.

    ________________________________________
    |  ELAUT PCB 0220 REV02                 |
    D  Relay                  L6203  BD243  |
    B  Relay    TBAB10S                     |
    1  Relay              J5                |
    5                                       |
    |  HEF4021  ISD1420P                    |
    D  HEF4021  4116R-1          L6506      |
    B     ULN2003  4094  PM99BH   YMZ284    |
    2       4094                 W27E512    |
    5   JP2                74HCT573 74HCT00 |
    |  HEF4021  HEF4010                XTAL |
    IE                        68HC11E1CFN2  |
    C   JP3          TLP521          DIPSW1 |
    10 HEF4511 HEF4094 FM25040  LED         |
    |   EUROGRIJPER REV02                   |
    ________________________________________|

****************************************************************************/

#include "emu.h"

#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/generic_spi_flash.h"
#include "sound/ay8910.h"

#include "speaker.h"

namespace {

class megacrane_state : public driver_device
{
public:
	megacrane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_spi(*this, "nvram")
		, m_ymz(*this, "ymz")
	{
	}

	void megacrane(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<generic_spi_flash_device> m_spi;
	required_device<ymz284_device> m_ymz;
};


void megacrane_state::mem_map(address_map &map)
{
	map(0x8000, 0xffff).rom().region("maincpu", 0x8000);

	map(0x8000, 0x8000).w(m_ymz, FUNC(ymz284_device::address_w));
	map(0x8001, 0x8001).w(m_ymz, FUNC(ymz284_device::data_w));
}


static INPUT_PORTS_START(megacrane)
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")   // (Port A)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN2")   // (Port D)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN) //MISO
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN) //MOSI
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN) //SCK
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN) //CS
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END


void megacrane_state::megacrane(machine_config &config)
{
	MC68HC11E1(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &megacrane_state::mem_map);
	m_maincpu->in_pa_callback().set_ioport("IN1");
	m_maincpu->in_pd_callback().set_ioport("IN2");
	m_maincpu->in_pe_callback().set_ioport("SW1");

	GENERIC_SPI_FLASH(config, m_spi, 0);

	SPEAKER(config, "mono").front_center();

	YMZ284(config, m_ymz, 4000000).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START(megacrane)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("elaut_2001_eu_mg_i_02.39.07.u5", 0x0000, 0x10000, CRC(feb5cfa1) SHA1(3c091543c0419ea15a5d66d2b9602668e7c35b10))

	ROM_REGION(0x2000, "voice", 0)
	ROM_LOAD("elaut_2001_sound_megacrane.u5", 0x0000, 0x2000, NO_DUMP) //ISD1420P

	ROM_REGION(0x200, "nvram", 0)
	ROM_LOAD("fm25040.u4", 0x0000, 0x200, CRC(b77297fe) SHA1(c404f7a254395412d8ee3a7090a2d67848923409))
ROM_END

} // anonymous namespace


GAME( 1997, megacrane, 0,     megacrane, megacrane, megacrane_state, empty_init, ROT0, "Elaut", "Megacrane", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
