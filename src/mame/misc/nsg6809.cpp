// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for 6809-based mechanical games by National Sports
    Games, Inc. (a division of Skee-Ball Inc.):

    * Pitch Hitter - Baseball Challenge
    * Super Shot (undumped)
    * UB-QB - Football Challenge (undumped)

    Memory map for "Universal Game PCB" in Super Shot schematics:

                2K SRAM             8K SRAM
           JP5-A JP6-B JP7-B   JP5-B JP6-A JP7-A

    *AUX3     $6000-$7FFF         $6000-$7FFF
    *AUX2     $2000-$3FFF         $2000-$27FF
    *AUX1     $4000-$5FFF         $4000-$5FFF
    *ASIC     $1000-$17FF         $3000-$37FF
    *VIA      $0800-$0FFF         $2800-$2FFF
    *SRAM     $0000-$07FF         $0000-$1FFF
    DEADMAN   $1E00-$1FFF         $3E00-$3FFF
    *IN-D     $1E00-$1FFF         $3E00-$3FFF
    *OUT-C    $1C00-$1DFF         $3C00-$3DFF
    *IN-C     $1C00-$1DFF         $3C00-$3DFF
    *OUT-B    $1A00-$1BFF         $3A00-$3BFF
    *IN-B     $1A00-$1BFF         $3A00-$3BFF
    *OUT-A    $1800-$19FF         $3800-$39FF
    *IN-A     $1800-$19FF         $3800-$39FF

    "ASIC" is a rather odd thing to call a 6551...

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/mos6551.h"
#include "machine/watchdog.h"


namespace {

class nsg6809_state : public driver_device
{
public:
	nsg6809_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void pitchhit(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;

	required_device<mc6809_device> m_maincpu;
};


void nsg6809_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2800, 0x280f).m("via", FUNC(via6522_device::map));
	map(0x3000, 0x3003).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
	map(0x3e00, 0x3e00).w("deadman", FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( pitchhit )
INPUT_PORTS_END

void nsg6809_state::pitchhit(machine_config &config)
{
	MC6809(config, m_maincpu, XTAL(4'000'000)); // clock buffered through 74HC4060
	m_maincpu->set_addrmap(AS_PROGRAM, &nsg6809_state::main_map);

	via6522_device &via(MOS6522(config, "via", XTAL(4'000'000) / 4));
	via.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));

	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(XTAL(1'843'200));
	acia.irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	WATCHDOG_TIMER(config, "deadman", 0);

	input_merger_device &merger(INPUT_MERGER_ANY_HIGH(config, "mainirq"));
	merger.output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);
}


/*

Pitch Hitter Eprom 27c256 by National Sports Games (aka Skeeball)
location U-14

IC label "PH101C"  and  "CK# $A790"

*/

ROM_START( pitchhit )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "ph101c__ck_a790.u14", 0x0000, 0x8000, CRC(1d306ab7) SHA1(13faf7c6dca8e5482672b2d09a99616896c4ae55) )
ROM_END

} // anonymous namespace


GAME(1993, pitchhit, 0, pitchhit, pitchhit, nsg6809_state, empty_init, ROT0, "National Sports Games", "Pitch Hitter - Baseball Challenge", MACHINE_IS_SKELETON_MECHANICAL )
