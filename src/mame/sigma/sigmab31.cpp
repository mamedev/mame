// license:BSD-3-Clause
// copyright-holders:
/*

SIGMA B31 SYSTEM
Seems a predecessor (or only mechanical version) of the hardware in sigmab52.cpp

Board silkscreend on top    sigma,inc. B31-1E
                B31-1E      GTP
                MECH-SLOT   (MS)
                1ST REEL
                2ND REEL
                3RD REEL
                4TH REEL
                5TH REEL
                sigma, inc.

Board stickered on top x2   microware
                            (c)MICROWARE SYSTEMS CORPORATION
                            IND09LI

9 jumpers on back that look like it came that way, not re-worked.

.57 27256   stickered   M-SLOT
                        03-00
                        L89-1625

.70 27512   stickered   B325107-084
                        CAPTAIN LUCKY
                        L93-0091


chip at location ic69   24 pin dip  Stamped     Motorola logo?  MC68B50P    stickered   MODS
                                                T6A J9007                               COMPLETE

chip at location ic62   28 pin dip  Stamped     Hitachi logo? OJ1 R
                                                HD63B40P

chip at location ic3    28 pin dip  Stamped     Fujitsu logo?   JAPAN
                                                8464A-15L
                                                9034 T98

chip at location ic9    28 pin dip  Stamped     Fujitsu logo?   JAPAN
                                                8464A-15L
                                                9034 T98

chip at location ic65   28 pin dip  Stamped     Fujitsu logo?   JAPAN
                                                8464A-15L
                                                9047 T01

chip at location ic65   28 pin dip  Stamped     NEC JAPAN
                                                D43256AC-10L
                                                9103AD053

chip at location JP6    20 pin dip switch       stickered   DENOMINATION
                                                            SET

chip at location ic80   7 pin sip           stamped     Fujitsu logo? JAPAN
                                                        MB3730
                                                        9041 M40            <- might be missing some since the bolt of the heat sink blocks it

    sw1 off
    sw2 off
    sw3 on
    sw4 on
    sw5 on
    sw6 off
    sw7 off
    sw8 off
    sw9 off
    sw10    off

Oscillator at x1    stamped     KX0-01-1
                                8.0000MHZ
                                9051 KYOCERA

Oscillator at x2    stamped     KX0-HC1-T
                                3.579545MHZ
                                9103 KYOCERA

missing chip at ic56    silkscreen says 6809

chip at location ic43   24 pin dip  stamped     YAMAHA
                                                YM3812
                                                9036 EADB

chip at location ic50   28 pin dip  stamped     Hitachi logo? 1A1 R
                                                HD63B40P
                                                JAPAN

*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/nvram.h"
#include "sound/ymopl.h"
#include "speaker.h"


namespace {

class sigmab31_state : public driver_device
{
public:
	sigmab31_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void sigmab31(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void prg_map(address_map &map) ATTR_COLD;

	required_device<cpu_device>     m_maincpu;
};


void sigmab31_state::prg_map(address_map &map)
{
	map(0x6000, 0xf6ff).rom();
	map(0xf800, 0xffff).rom();
}


static INPUT_PORTS_START( cptlucky )
	PORT_START("IN0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // TODO: should be 10 switches
	PORT_DIPNAME( 0x01, 0x01, "DSW1-1" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1-2" )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1-3" )        PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1-4" )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1-5" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1-6" )        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1-7" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1-8" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void sigmab31_state::machine_start()
{
}

void sigmab31_state::machine_reset()
{
}


void sigmab31_state::sigmab31(machine_config &config)
{
	MC6809(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sigmab31_state::prg_map);

	PTM6840(config, "6840ptm_1", 8_MHz_XTAL / 8);

	PTM6840(config, "6840ptm_2", 8_MHz_XTAL / 8);

	ACIA6850(config, "acia");

	//NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	SPEAKER(config, "mono").front_center();

	YM3812(config, "ymsnd", 3.579545_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( cptlucky )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b325107-084_captain_lucky_l93-0091.70", 0x00000, 0x10000, CRC(84c2ab4e) SHA1(3d388ba1c8e4718ca95df45f59d0315887385a27) )

	ROM_REGION( 0x8000, "opl", 0 )
	ROM_LOAD( "m-slot_03-00_l89-1625.57", 0x00000, 0x8000, CRC(268c8a7c) SHA1(90903428d6c0af3ebdcb462e80a7c28dc4ee7af2) )
ROM_END

} // anonymous namespace


GAME( 1988, cptlucky, 0, sigmab31, cptlucky, sigmab31_state, empty_init, ROT0, "Sigma", "Captain Lucky", MACHINE_IS_SKELETON_MECHANICAL ) // 1988 copyright in main CPU ROM
