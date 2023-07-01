// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H19

    A smart terminal designed and manufactured by Heath Company. This
    is identical to the Zenith Data Systems Z-19.

****************************************************************************/

#include "emu.h"

#include "tlb.h"

namespace {

class h19_state : public driver_device
{
public:
	h19_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tlb(*this, "tlb")
	{
	}

	void h19(machine_config &config);
	void h19_superh19(machine_config &config);
	void h19_watzh19(machine_config &config);
	void h19_ultrah19(machine_config &config);

private:

	required_device<heath_tlb_device> m_tlb;

};

void h19_state::h19(machine_config &config)
{
	HEATH_TLB(config, m_tlb);
}

void h19_state::h19_superh19(machine_config &config)
{
	HEATH_SUPER19(config, m_tlb);
}

void h19_state::h19_watzh19(machine_config &config)
{
	HEATH_WATZ(config, m_tlb);
}

void h19_state::h19_ultrah19(machine_config &config)
{
	HEATH_ULTRA(config, m_tlb);
}

// ROM definition
ROM_START( h19 )
ROM_END

ROM_START( super19 )
ROM_END

ROM_START( watz19 )
ROM_END

ROM_START( ultra19 )
ROM_END
} // anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE       INPUT   CLASS      INIT        COMPANY          FULLNAME                         FLAGS
COMP( 1979, h19,     0,      0,      h19,             0,   h19_state, empty_init, "Heath Company", "Heathkit H-19",                 MACHINE_SUPPORTS_SAVE )
//Super-19 ROM - ATG Systems, Inc - Adv in Sextant Issue 4, Winter 1983. With the magazine lead-time, likely released late 1982.
COMP( 1982, super19, h19,    0,      h19_superh19,    0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ Super-19 ROM", MACHINE_SUPPORTS_SAVE )
// Watzman ROM - HUG p/n 885-1121, announced in REMark Issue 33, Oct. 1982
COMP( 1982, watz19,  h19,    0,      h19_watzh19,     0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ Watzman ROM",  MACHINE_SUPPORTS_SAVE )
// ULTRA ROM - Software Wizardry, Inc., (c) 1983 William G. Parrott, III
COMP( 1983, ultra19, h19,    0,      h19_ultrah19,    0,   h19_state, empty_init, "Heath Company", "Heathkit H-19 w/ ULTRA ROM",    MACHINE_SUPPORTS_SAVE )
