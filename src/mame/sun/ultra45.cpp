// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************
Skeleton driver for Sun Microsystems Ultra 25 and Ultra 45 Workstations.

Hardware info about Ultra 45:
 -Dual UltraSPARC IIIi processor.
 -PLX Technology PEX8532-BB25BI G (PCI Express Switch).
 -ALI ULI M1575 A1 (Super South Bridge).
 -Broadcom BCM5715CKPBG (Dual-Port Gigabit Ethernet controller with a PCI Express Host Interface).

***********************************************************************************************************/

#include "emu.h"
#include "cpu/sparc/sparc.h"


namespace {

class ultra45_state : public driver_device
{
public:
	ultra45_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void ultra45(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START(ultra45)
INPUT_PORTS_END

void ultra45_state::ultra45(machine_config &config)
{
	SPARCV8(config, m_maincpu, 20'000'000); // Actually a 1.6GHz UltraSPARC IIIi CPU with 1 MB integrated Level2 cache
	//SPARCV8(config, m_maincpu, 20'000'000); // Actually a 1.6GHz UltraSPARC IIIi CPU with 1 MB integrated Level2 cache (optional 2nd CPU)
}


// Probably all wrong
ROM_START(ultra45)
	ROM_REGION(0x400000, "maincpu", 0)
	ROM_LOAD( "chicago_4.25.9_cks_c1bd_s29al032d.u29", 0x000000, 0x400000, CRC(e08f3c4f) SHA1(87bd5c92f59121b5253c0c12d627d18401a01bf7) )

	ROM_REGION(0x400000, "extra1", 0)
	ROM_LOAD( "chicago_1.80.3_cks_8ee3_s29al032d.u81", 0x000000, 0x400000, CRC(0b18627e) SHA1(473fe575e0dad4467bb40cdb76234be354d67ac2) ) // checksum does not match with the one printed on the label, probably because a firmware update

	ROM_REGION(0x021000, "extra2", 0)
	ROM_LOAD( "d887_at45db011b.u77",                   0x000000, 0x021000, CRC(b6363183) SHA1(1769ffa968e0f79bbfa7bc147f8d6c179f5ca44b) )

	ROM_REGION(0x008000, "extra3", 0)
	ROM_LOAD( "ca4b_at25256an.u79",                    0x000000, 0x008000, CRC(a35303fb) SHA1(df1b4121c8ae40b7fb9a03cc57e61f0da12e1110) )

	/* Unprotected PIC16F777 near a 20 MHz xtal
	    ID0 = 2009h
	    ID1 = 0000h
	    ID2 = 2009h
	    ID3 = 0003h
	    Oscillator = High Speed
	    Watchdog = Disabled
	    Power-up timer = Disabled
	    MCLR pin = Disabled
	    Brown-out Reset enabled and always on */
	ROM_REGION(0x004000, "pic1", 0)
	ROM_LOAD("972a_pic16f777.u13",                     0x000000, 0x004000, CRC(a1d4b342) SHA1(564926990cf28a1f88a8a374f16ac172f7b8c8f7) )

	ROM_REGION(0x004300, "pic2", 0)
	ROM_LOAD("a4bb_pic12f629.u66",                     0x000000, 0x004300, CRC(258bd64e) SHA1(58f64710a3f4d184a3aaa776aa351d30144806f5) )

	ROM_REGION(0x0016cc, "pld", 0)
	ROM_LOAD("chicago_0309_4m_cks_6d7b_xc9572xl.u15",  0x000000, 0x0016cc, CRC(d5fbe610) SHA1(a41e136eeb6c115523814ca3774ff7a0a0604569) )
ROM_END

} // anonymous namespace

//    YEAR, NAME,    PARENT, COMPAT, MACHINE, INPUT,   CLASS,         INIT,       COMPANY,            FULLNAME,   FLAGS
COMP( 1996, ultra45, 0,      0,      ultra45, ultra45, ultra45_state, empty_init, "Sun Microsystems", "Ultra 45", MACHINE_IS_SKELETON )
