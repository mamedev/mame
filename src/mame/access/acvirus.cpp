// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    acvirus.cpp - Access Virus series

    Skeleton driver by R. Belmont

    Hardware in brief:
        Virus A: SAB 80C535-N    (12 MHz), DSP56303 @ 66 MHz
        Virus B: SAB 80C535-N    (12 MHz), DSP56311 @ ??? MHz (illegible on PCB photo I've seen)
        Virus C: SAF 80C515-L24N (24 MHz), DSP56362 @ 120 MHz

        Virus Rack is same h/w as B, Rack XL is the same h/w as C.
        Virus Classic is supposed to be the same h/w as B but not proven.

    The various 80C5xx chips are i8051-based SoCs with additional I/O ports,
    256 bytes of internal RAM like the 8052, and an analog/digital converter.

    The top 4 bits of port P5 select the bank at 0x8000.  P5 is not implemented in
    any of the MCS-51 variants we support yet.

    Hardware Notes:
    The DSP has three SRAM chips, probably 128 kbyte each
    for a total of 128 kwords, mapped to address 0x20000. All three DSP
    buses (P, X, Y) point to the same external memory. There's another 128
    kbyte of battery backed SRAM for the 8051.

    The firmware image fits exactly in an AM29F040-120PC flash chip, and is
    bank switched into the 8051 program address space. The lower 0x8000
    bytes of the address space always points to the first 0x8000 bytes of
    flash (except during firmware upgrade, as I assume the programming
    routine has do run from RAM). The upper 0x8000 bytes of the address
    space can point to any 0x8000 sized bank in flash. A bank switch routine
    is at 0x64B8, and will switch to e.g. bank 2 (offset 0x10000) when A =
    0x20. The low nibble is usually zero, but not always, and I don't know
    how it's interpreted.

    Banks 0-2 contain OS code and data, banks 3-6 contain DSP code and data,
    and banks 8-14 seem to contain factory default settings. There are flash
    programming routines at the beginning of banks 7 and 15, and two at the
    end of bank 6. Not sure why there are so many, and not all are
    identical, so there's probably additional bank switching logic to match.
    All display a charming "DO NOT TOUCH ME" message while programming. :)

    The same bank switching also seems to affect external memory, but I'm
    not sure how the smaller SRAM is mapped. Some external memory locations
    are used for other tasks, like communicating with the DSP.

    The initial DSP program and data upload routine is at 0x1FAA. After
    setting up the bus, it churns out all the 24-bit words in banks 3-6
    (except for headers) as one stream. The DSP will interpret the first
    word as a length, the second as address, and the following "length"
    words will be stored at that address in program memory before execution
    starts there. This is just a very short bootstrap program, which takes
    care of receiving the remaining words in chunks. Each chunks starts with
    three words - a command, an address, and optionally length. Commands 0-2
    store data in P, X, or Y memory respectively. Command 3 splits each
    24-bit word into two 12-bit values and store each of them as a 24-bit
    word in Y memory. Command 4 starts execution at the specified address,
    and doesn't have a length.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/intelfsh.h"
#include "speaker.h"


namespace {

class acvirus_state : public driver_device
{
public:
	acvirus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rombank(*this, "rombank")
	{ }

	void virus(machine_config &config);

	void init_virus();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void virus_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
};

void acvirus_state::machine_start()
{
	m_rombank->configure_entries(0, 15, memregion("maincpu")->base(), 0x8000);
	m_rombank->set_entry(3);
}

void acvirus_state::machine_reset()
{
}

void acvirus_state::virus_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0); // fixed 32K of flash image
	map(0x8000, 0xffff).bankr("rombank");
}

void acvirus_state::virus(machine_config &config)
{
	SAB80C535(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &acvirus_state::virus_map);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

static INPUT_PORTS_START( virus )
INPUT_PORTS_END

ROM_START( virusa )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "virus_a_28.bin", 0x000000, 0x080000, CRC(087cd808) SHA1(fe3310a165c208473822455c75ee5b2a6de34bc8) )
ROM_END

ROM_START( virusb )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "virus_bt_490x049.bin", 0x000000, 0x080000, CRC(4ffc928a) SHA1(ee4b83e2eb1f01c73e37e2ff1d2edd653a0dcf5b) )
ROM_END

ROM_START( virusc )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "virus_c_650x352.bin", 0x000000, 0x080000, CRC(d44a9468) SHA1(fad9b896b39a43a1d46acb1d780b78b775a609b8) )
ROM_END

ROM_START( virusrck )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "virus_rt_210x071.bin", 0x000000, 0x080000, CRC(62b2bcc1) SHA1(241467bcb563736472a6e61f6c9c532590664500) )
ROM_END

ROM_START( virusrckxl )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "virus_xl_650x079.bin", 0x000000, 0x080000, CRC(d0721c46) SHA1(b7c292b66ba3690a4a50592e17321b9c4147621d) )
ROM_END

ROM_START( viruscl )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "virus_cl_061_release.bin", 0x000000, 0x080000, CRC(a202e443) SHA1(33d5f4ebbacc817ab1e5dd572e8dc755f6c5e253) )
ROM_END

} // anonymous namespace


CONS( 1997, virusa,     0, 0, virus, virus, acvirus_state, empty_init, "Access", "Virus A", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
CONS( 1999, virusb,     0, 0, virus, virus, acvirus_state, empty_init, "Access", "Virus B (Ver. T)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
CONS( 2002, virusc,     0, 0, virus, virus, acvirus_state, empty_init, "Access", "Virus C", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
CONS( 2001, virusrck,   0, 0, virus, virus, acvirus_state, empty_init, "Access", "Virus Rack (Ver. T)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
CONS( 2002, virusrckxl, 0, 0, virus, virus, acvirus_state, empty_init, "Access", "Virus Rack XL", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
CONS( 2004, viruscl,    0, 0, virus, virus, acvirus_state, empty_init, "Access", "Virus Classic", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
