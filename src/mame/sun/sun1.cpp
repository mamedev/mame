// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Sun-1 Models
        ------------

    Sun-1

        Processor(s):   68000
        P/N:            270-0001
        Notes:          Large black desktop boxes with 17" monitors.
                        Uses the original Stanford-designed video board
                        and a parallel microswitch keyboard (type 1) and
                        parallel mouse (Sun-1).

    100
        Processor(s):   68000 @ 10MHz
        Bus:            Multibus, serial
        Notes:          Uses a design similar to original SUN (Stanford
                        University Network) CPU. The version 1.5 CPU can
                        take larger RAMs.

    100U
        Processor(s):   68010 @ 10MHz
        CPU:            501-1007
        Bus:            Multibus, serial
        Notes:          "Brain transplant" for 100 series. Replaced CPU
                        and memory boards with first-generation Sun-2
                        CPU and memory boards so original customers
                        could run SunOS 1.x. Still has parallel kb/mouse
                        interface so type 1 keyboards and Sun-1 mice
                        could be connected.

    170
        Processor(s):   68010?
        Bus:            Multibus?
        Chassis type:   rackmount
        Notes:          Server. Slightly different chassis design than
                        2/170's


        Documentation:
            http://www.bitsavers.org/pdf/sun/sun1/800-0345_Sun-1_System_Ref_Man_Jul82.pdf
            (page 39,40 of pdf contain memory map)

        This "Draft Version 1.0" reference claims a 10MHz clock for the
        MC68000 and a 5MHz clock for the Am9513; though the original design
        may have specified a 10MHz CPU, and though this speed may have been
        realized in later models, schematics suggest the system's core
        devices actually run at 8/4MHz (divided from a 16MHz XTAL), which
        lets the 1.0 monitor ROM's Am9513 configuration generate a more
        plausible baud rate.

        04/12/2009 Skeleton driver.

        04/04/2011 Modernised, added terminal keyboard.

 TODO:
  - graphic cards
  - network cards
  - storage cards

****************************************************************************/

#include "emu.h"

#include "sun1_cpu.h"

#include "bus/multibus/multibus.h"

namespace {

class sun1_state : public driver_device
{
public:
	sun1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_bus(*this, "bus")
	{
	}

	void sun1(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<multibus_device> m_bus;
};

void sun1_state::machine_start()
{
}

void sun1_state::machine_reset()
{
}

static void sun1_cards(device_slot_interface &device)
{
	device.option_add("sun1cpu", MULTIBUS_SUN1CPU);
}

void sun1_state::sun1(machine_config &config)
{
	// FIXME: CPU board can optionally drive Multibus clocks
	MULTIBUS(config, m_bus, 19.6608_MHz_XTAL / 2);

	// slot 1 is physically located at top, only slots 1-3 have P2
	MULTIBUS_SLOT(config, "slot1", m_bus, sun1_cards, nullptr, false); // memory expansion 2
	MULTIBUS_SLOT(config, "slot2", m_bus, sun1_cards, nullptr, false); // memory expansion 1
	MULTIBUS_SLOT(config, "slot3", m_bus, sun1_cards, "sun1cpu", false); // processor
	MULTIBUS_SLOT(config, "slot4", m_bus, sun1_cards, nullptr, false); // bus master 1
	MULTIBUS_SLOT(config, "slot5", m_bus, sun1_cards, nullptr, false); // bus master 2
	MULTIBUS_SLOT(config, "slot6", m_bus, sun1_cards, nullptr, false); // multibus memory, optional
	MULTIBUS_SLOT(config, "slot7", m_bus, sun1_cards, nullptr, false); // graphics
}

ROM_START(sun1)
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY             FULLNAME  FLAGS
COMP( 1982, sun1, 0,      0,      sun1,    0,     sun1_state, empty_init, "Sun Microsystems", "Sun-1",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
