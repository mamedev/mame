// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-KN7000 and related MN10300-based keyboards

    This driver currently only declares the ROM contents of these machines.
    There is no emulation yet: MAME has a Panasonic MN10300 disassembler but
    no execution core, so no CPU device can be instantiated. The CPU is named
    in a commented-out line in each machine configuration below, so that the
    part is documented until a core is available.

    All five machines are built around a Panasonic MN103002A (MN10300 family,
    AM33 core), running Panasonic's "MILK" object framework.

    Hardware inventory below is taken from the manufacturer's service manuals:

        SX-KN7000  EMID0207013C0 (2002)
        SX-KN6000  EMID9908016C0 (1999)
        SX-KN6500  EMID0101001C0 (2001)
        SX-KN2400, SX-KN2600

    Each manual covers only its own model, except that the SX-KN2400 book's
    parts list reproduces the SX-KN2600's list of main-board integrated
    circuits verbatim - it names devices that appear on no SX-KN2400 schematic
    sheet or board silkscreen, and omits the floppy controller that all three
    SX-KN2400 sources show. The SX-KN2400 devices below are therefore taken
    from its schematics, block diagram and board assembly drawings only.

    Capacities were read from the schematics and parts lists, and confirmed by
    counting address pins. The manuals write ROM sizes in megabits: the KN6000
    manual prints "(64M BIT MASK ROM)" next to parts numbered QSIGX3C64004 and
    up, which fixes both the unit and the meaning of the digits in the part
    number.

    Clocking: the KN6000 and KN6500 manuals print a 32 MHz oscillator at X1,
    feeding a spread-spectrum clock generator at IC6 whose output drives the
    CPU. The KN7000, KN2400 and KN2600 use the same topology, but their
    manuals do not give the frequency of X1, so their core clocks are inferred
    from the KN6000 and KN6500 rather than documented.

    The KN7000's program and table ROMs are not chip reads. They are payloads
    from Panasonic's own firmware update disks, and they validate against the
    checksums that Panasonic ships alongside them: the update descriptor files
    carry a 32-bit sum over the whole payload plus 16-bit sums of each 256 KiB
    block, and every block matches.

    A note on the KN7000's IC16/IC17: these are one pair of 4 MiB flash devices
    on CPU address lines A2-A22, so together they span 8 MiB. Address line A22
    selects between the two regions declared below - the table data occupies
    the half where A22 is low, and the program the half where it is high.

    TODO:
      - MN10300 CPU core, which everything else depends on
      - dump the wave, rhythm and picture ROMs listed as NO_DUMP below

***************************************************************************/

#include "emu.h"



namespace {

class kn7000_state : public driver_device
{
public:
	kn7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void kn7000(machine_config &config);
	void kn6000(machine_config &config);
	void kn6500(machine_config &config);
	void kn2400(machine_config &config);
};


static INPUT_PORTS_START(kn7000)
INPUT_PORTS_END


void kn7000_state::kn7000(machine_config &config)
{
	// MN103002A(config, "maincpu", 32_MHz_XTAL);  // no MN10300 CPU core in MAME yet

	// The custom-data flash.  The firmware talks to it with the AMD command set
	// (unlock at +0xAAAA / +0x5554, autoselect 0x90, program 0xA0, sector erase
	// 0x80+0x30) and validates it against a table of accepted parts before it will
	// program anything, so the device IDs here are functional, not decorative.
	// KN7000 accepts MBM29LV160B, MX29LV160B or AT49BV16X4 (table at 0x485CF9E0).
}

void kn7000_state::kn6000(machine_config &config)
{
	// MN103002A(config, "maincpu", 32_MHz_XTAL);  // no MN10300 CPU core in MAME yet

	// The custom-data flash.  The firmware talks to it with the AMD command set
	// (unlock at +0xAAAA / +0x5554, autoselect 0x90, program 0xA0, sector erase
	// 0x80+0x30) and validates it against a table of accepted parts before it will
	// program anything, so the device IDs here are functional, not decorative.
	// KN6000 accepts MBM29LV160B or AT49BV16X4.

}

void kn7000_state::kn6500(machine_config &config)
{
	// MN103002A(config, "maincpu", 32_MHz_XTAL);  // no MN10300 CPU core in MAME yet

	// The custom-data flash.  The firmware talks to it with the AMD command set
	// (unlock at +0xAAAA / +0x5554, autoselect 0x90, program 0xA0, sector erase
	// 0x80+0x30) and validates it against a table of accepted parts before it will
	// program anything, so the device IDs here are functional, not decorative.
	// KN6500 accepts MBM29LV160B or AT49BV16X4.

}

void kn7000_state::kn2400(machine_config &config)
{
	// MN103002A(config, "maincpu", 32_MHz_XTAL);  // no MN10300 CPU core in MAME yet
}


/***************************************************************************

    SX-KN7000

    IC16, IC17   C3FBNG000016   32 Mbit flash, program + table (see note above)
    IC18         C3CBND000046   64 Mbit mask ROM, rhythm  (later production)
    IC20         C3FBMD000050   32 Mbit flash, rhythm      (earlier production,
                                same site, and half the capacity; the manual
                                states IC20 is not supplied as a spare part)
    IC19         C3CBMD000098   64 Mbit picture ROM
    IC21         C3FBMD000050   16 Mbit custom flash (user data).  The service
                                manual captions this "32M FLASH", but that is
                                copied from IC20: the firmware's flash device
                                table (0x485CF9E0) accepts only 16 Mbit parts,
                                so a 32 Mbit device would fail its autoselect
                                check.  It also builds a 0x200000 sector map,
                                and the board decodes a 2 MB window at
                                0x96800000.  Three independent reasons for
                                16 Mbit.
    IC203        C3CBQD000002  128 Mbit mask ROM, wave, main TG bank Y (AWAY)
    IC204        C3CBQD000001  128 Mbit mask ROM, wave, main TG bank X (AWAX)
    IC207        C3CBQD000004  128 Mbit mask ROM, wave, sub TG bank Y (BWAY)
    IC208        C3CBQD000003  128 Mbit mask ROM, wave, sub TG bank X (BWAX)

    The wave devices sit on two independent buses per tone generator, so they
    are declared as one region per bank rather than concatenated. The block
    diagram shows IC207 and IC208 the other way round, but the schematic gives
    BWAY on IC207 and BWAX on IC208 at pin level, the chip-enable groups agree
    with it, and the part numbers pair as Y = 000002/000004 against
    X = 000001/000003.
    IC414        C3FBKD000162    4 Mbit flash, SD card sub-CPU program

***************************************************************************/

ROM_START(kn7000)
	ROM_REGION32_LE(0x400000, "program", 0)
	ROM_LOAD32_WORD("kn7000_program_even.ic17", 0x000000, 0x200000, CRC(529b87ce) SHA1(f198fd9a9ea31a454acfe7be0eb935beca6771b1))
	ROM_LOAD32_WORD("kn7000_program_odd.ic16",  0x000002, 0x200000, CRC(a36e6222) SHA1(721d4469dc5f692f7a2c16c556b2e21115df19f6))

	ROM_REGION32_LE(0x400000, "table_data", 0)
	ROM_LOAD32_WORD("kn7000_table_even.ic17", 0x000000, 0x200000, CRC(005a6db2) SHA1(2f4112ea9b039b17b5ada6952b7646adae8d9dd6))
	ROM_LOAD32_WORD("kn7000_table_odd.ic16",  0x000002, 0x200000, CRC(7e1a312e) SHA1(435b597b926ebac56d4710bcae25b635a59a9ce5))

	ROM_REGION(0x1000000, "waveform_main_y", 0)
	ROM_LOAD("c3cbqd000002.ic203", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_main_x", 0)
	ROM_LOAD("c3cbqd000001.ic204", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_sub_y", 0)
	ROM_LOAD("c3cbqd000004.ic207", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_sub_x", 0)
	ROM_LOAD("c3cbqd000003.ic208", 0x000000, 0x1000000, NO_DUMP)

	ROM_REGION(0x800000, "rhythm_data", 0)
	ROM_LOAD("c3cbnd000046.ic18", 0x000000, 0x800000, NO_DUMP)

	ROM_REGION(0x800000, "picture", 0)
	ROM_LOAD("c3cbmd000098.ic19", 0x000000, 0x800000, NO_DUMP)

	// The custom flash holds user data, and is populated from a floppy rather than
	// programmed at the factory: the firmware inflates the CTMINI payload from an
	// "Initial Data Disk" and writes it verbatim to offset 0x20000, which is the top
	// 30 of the 64 KiB sectors.  Nothing is written below that, so the boot sectors
	// are left erased here.
	//
	// The images below are therefore not chip dumps.  Each is the exact content the
	// firmware places in the device for one published data set, so a part programmed
	// from that floppy reads back as declared.  They are offered as a BIOS choice
	// because a real instrument holds exactly one of them at a time.  Sectors 19..29
	// are byte-identical in all nine, so a little over a third of the region is an
	// invariant template rather than per-set data.
	ROM_REGION(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "ctmini",  "Initial Data Disk (factory default)")
	ROMX_LOAD("01ctmini.ic21", 0x020000, 0x1e0000, CRC(2a133ea7) SHA1(67b2a0fe8154c4d15557399a86bf0d0b49813ced), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "custm1",  "Custom Data: Blue Bayou")
	ROMX_LOAD("01custm1.ic21", 0x020000, 0x1e0000, CRC(c1c69b67) SHA1(4cd5fe37871984a4b470b1a12b835c9bc41f37f5), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "custm2",  "Custom Data: Piano Player")
	ROMX_LOAD("02custm2.ic21", 0x020000, 0x1e0000, CRC(8170c2d3) SHA1(6c85b616a9acc3a7ecc1b49a70f138c0d9442086), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "custm3",  "Custom Data: Jazz Organ Soloist")
	ROMX_LOAD("03custm3.ic21", 0x020000, 0x1e0000, CRC(5a7cc504) SHA1(d5722a1129134383bf643d20b4e0866c3089fc36), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "custm4",  "Custom Data: Bob's Band")
	ROMX_LOAD("04custm4.ic21", 0x020000, 0x1e0000, CRC(1f1c195e) SHA1(25822a3ad726df49882cdd0066424600fb70ea14), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "custm5",  "Custom Data: Traditional Melody")
	ROMX_LOAD("01custm5.ic21", 0x020000, 0x1e0000, CRC(ac264469) SHA1(11d32c3d293a23a511a76297647758046cd8ccc9), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "custm6",  "Custom Data: Jogeh 1")
	ROMX_LOAD("02custm6.ic21", 0x020000, 0x1e0000, CRC(174cea4c) SHA1(54f2bac0551a8d43dc1a87fe24a13c545169a326), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "custm7",  "Custom Data: Vibraphone")
	ROMX_LOAD("03custm7.ic21", 0x020000, 0x1e0000, CRC(aa14a0ed) SHA1(e1bdfbbbe78353003af5ed01866be87d799523bb), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "custm8",  "Custom Data: Italian Accordion 7")
	ROMX_LOAD("04custm8.ic21", 0x020000, 0x1e0000, CRC(18753233) SHA1(f4722d93ff3f31c6d8c25f757968d796e4eb57ca), ROM_BIOS(8))

	ROM_REGION(0x80000, "sdcard_cpu", 0)
	ROM_LOAD("c3fbkd000162.ic414", 0x000000, 0x80000, NO_DUMP)
ROM_END


/***************************************************************************

    SX-KN6000

    IC11, IC12   M29LV160B8TN   16 Mbit flash, program
    IC13         QSIGX3C16008   16 Mbit mask ROM, table data
    IC14         QSIGX3C16007   16 Mbit mask ROM, table data
    IC15         QSIGX3C32021   32 Mbit mask ROM, rhythm data
    IC18         A49BV161490T   16 Mbit flash, custom rhythm (user data)
    IC205        QSIGX3C64004   64 Mbit mask ROM, wave, bank Y (WAY)
    IC206        QSIGX3C64005   64 Mbit mask ROM, wave, bank X (WAX)
    IC207        QSIGX3C64006   64 Mbit mask ROM, wave, bank Y (WAY)
    IC208        QSIGX3C64007   64 Mbit mask ROM, wave, bank X (WAX)

***************************************************************************/

ROM_START(kn6000)
	ROM_REGION32_LE(0x400000, "program", 0)
	ROM_LOAD32_WORD("kn6000_program_even.ic12", 0x000000, 0x200000, CRC(56c2cfe3) SHA1(e15a4c73440f1dcdf06457f9956c96bf20d68b16))
	ROM_LOAD32_WORD("kn6000_program_odd.ic11",  0x000002, 0x200000, CRC(9d94da6c) SHA1(d73b4c8ebf0c67b6a2eeb5571d0273fc6efbfe4c))

	ROM_REGION32_LE(0x400000, "table_data", 0)
	ROM_LOAD32_WORD("qsigx3c16008.ic13", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD32_WORD("qsigx3c16007.ic14", 0x000002, 0x200000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_y", 0)
	ROM_LOAD("qsigx3c64004.ic205", 0x000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64006.ic207", 0x800000, 0x800000, NO_DUMP)

	ROM_REGION(0x1000000, "waveform_x", 0)
	ROM_LOAD("qsigx3c64005.ic206", 0x000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64007.ic208", 0x800000, 0x800000, NO_DUMP)

	ROM_REGION(0x400000, "rhythm_data", 0)
	ROM_LOAD("qsigx3c32021.ic15", 0x000000, 0x400000, NO_DUMP)

	// Populated from the IDD6000 Initial Data Disk, which serves both the KN6000 and
	// the KN6500; see the note in the KN7000 set above.  Not a chip dump.
	ROM_REGION(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_LOAD("01ctmini.ic18", 0x020000, 0x1e0000, CRC(f108e4c7) SHA1(8c6d62a8afab717a2b59e9242bbf897b01369416))
ROM_END


/***************************************************************************

    SX-KN6500

    IC11, IC12   M29LV160B8TN   16 Mbit flash, program
    IC13         C3FBMD000069   16 Mbit table data, supplied pre-programmed
    IC14         C3FBMD000068   16 Mbit table data, supplied pre-programmed
                                (the schematic legend reads "PROGRAMMED MASK
                                ROM", copied from the KN6000, but the pinout
                                drawn beside it - RESET, RY/BY, VPP, WE - is a
                                NOR flash, so no device type is asserted here)
    IC15         QSIGX3C32021   32 Mbit mask ROM, rhythm data
    IC18         M29LV160B8TN   16 Mbit flash, custom rhythm (user data).  The
                                schematic labels IC11, IC12 and IC18 with this
                                same part but three different descriptors, so
                                it is fitted at all three flash sites rather
                                than being a repeated parts-list row.  The
                                firmware will only program a device whose
                                autoselect response is in its table, and that
                                table holds MBM29LV160B and AT49BV16X4 - both
                                16 Mbit bottom boot, which is what fixes the
                                geometry here.  Which vendor's 29LV160B this
                                designation refers to is not established.
    IC205        QSIGX3C64004   64 Mbit mask ROM, wave, bank Y (WAY)
    IC206        QSIGX3C64005   64 Mbit mask ROM, wave, bank X (WAX)
    IC207        QSIGX3C64006   64 Mbit mask ROM, wave, bank Y (WAY)
    IC208        QSIGX3C64007   64 Mbit mask ROM, wave, bank X (WAX)
    IC209        QSIGX3C64020   64 Mbit mask ROM, wave, bank Y (WAY)
    IC210        QSIGX3C64019   64 Mbit mask ROM, wave, bank X (WAX)

***************************************************************************/

ROM_START(kn6500)
	ROM_REGION32_LE(0x400000, "program", 0)
	ROM_LOAD32_WORD("kn6500_program_even.ic12", 0x000000, 0x200000, CRC(f42a2fcf) SHA1(7cebf73bf623fd714ca455ed50b80da1d2186414))
	ROM_LOAD32_WORD("kn6500_program_odd.ic11",  0x000002, 0x200000, CRC(ca2a733f) SHA1(2484d3b76b62b05ded39e4194cdc74fd3c01bcbe))

	ROM_REGION32_LE(0x400000, "table_data", 0)
	ROM_LOAD32_WORD("c3fbmd000069.ic13", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD32_WORD("c3fbmd000068.ic14", 0x000002, 0x200000, NO_DUMP)

	ROM_REGION(0x1800000, "waveform_y", 0)
	ROM_LOAD("qsigx3c64004.ic205", 0x0000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64006.ic207", 0x0800000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64020.ic209", 0x1000000, 0x800000, NO_DUMP)

	ROM_REGION(0x1800000, "waveform_x", 0)
	ROM_LOAD("qsigx3c64005.ic206", 0x0000000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64007.ic208", 0x0800000, 0x800000, NO_DUMP)
	ROM_LOAD("qsigx3c64019.ic210", 0x1000000, 0x800000, NO_DUMP)

	ROM_REGION(0x400000, "rhythm_data", 0)
	ROM_LOAD("qsigx3c32021.ic15", 0x000000, 0x400000, NO_DUMP)

	// Same IDD6000 payload as the KN6000; not a chip dump.
	ROM_REGION(0x200000, "custom_data", ROMREGION_ERASEFF)
	ROM_LOAD("01ctmini.ic18", 0x020000, 0x1e0000, CRC(f108e4c7) SHA1(8c6d62a8afab717a2b59e9242bbf897b01369416))
ROM_END


/***************************************************************************

    SX-KN2400 and SX-KN2600

    One firmware image serves both models, selecting between them at run time,
    and the two boards carry an identical set of memory devices: the schematic
    sheet holding the tone generator and both wave ROMs is the same drawing in
    both manuals, and the board assembly drawings list the same designators.
    The models differ in storage and I/O only - the SX-KN2400 has a floppy
    drive and controller, the SX-KN2600 an SD card interface. They therefore
    share every ROM listed below except the SD sub-processor's program flash,
    which is fitted only on the SX-KN2600.

    IC12, IC13   C3FBNG000007   32 Mbit flash, program (see note below)
    IC14         C3ZBNG000023   64 Mbit flash, rhythm and other data
    IC302        C3ZBP0000003   64 Mbit flash, wave bank Y (AWAY bus)
    IC303        C3ZBP0000004   64 Mbit flash, wave bank X (AWAX bus)
    IC404        C3ZBK0000020    4 Mbit flash, SD sub-CPU program (KN2600 only)

    Neither board carries a table or font ROM: the schematics, the board
    assembly drawings and the block diagrams agree that the devices listed
    above are the only memories present, and the LCD controller at IC104 has
    no external memory attached.

    The block diagrams describe IC302 and IC303 as 128 Mbit parts addressed by
    WAY0-WAY22, but the schematics show only 22 address inputs, and the tone
    generator's WAY22 and WAY23 pins terminate unconnected. The 64 Mbit figure
    from the schematics is used here.

    A note on IC12/IC13: the schematics show 21 address inputs driven from CPU
    address lines A2-A22, making these 32 Mbit devices that together span
    8 MiB. The dumps below cover 4 MiB of that pair. The remainder has not been
    read, so it is not described here; on the KN7000 the equivalent pair holds
    the table data in the half that these dumps do not cover.

***************************************************************************/

#define KN2400_ROM_COMMON \
	ROM_REGION32_LE(0x400000, "program", 0) \
	ROM_LOAD32_WORD("kn2400_program_even.ic13", 0x000000, 0x200000, CRC(b94fc8a8) SHA1(86d5d9916afdb90f82de78064b1d76fce3a21d7b)) \
	ROM_LOAD32_WORD("kn2400_program_odd.ic12",  0x000002, 0x200000, CRC(73781cbc) SHA1(d90a3560561efd94322dca1a6710f2d5d3837cd2)) \
 \
	ROM_REGION(0x800000, "rhythm_data", 0) \
	ROM_LOAD("c3zbng000023.ic14", 0x000000, 0x800000, NO_DUMP) \
 \
	ROM_REGION(0x800000, "waveform_y", 0) \
	ROM_LOAD("c3zbp0000003.ic302", 0x000000, 0x800000, NO_DUMP) \
 \
	ROM_REGION(0x800000, "waveform_x", 0) \
	ROM_LOAD("c3zbp0000004.ic303", 0x000000, 0x800000, NO_DUMP)

ROM_START(kn2400)
	KN2400_ROM_COMMON
ROM_END

ROM_START(kn2600)
	KN2400_ROM_COMMON

	// the SD card interface is fitted only on the SX-KN2600
	ROM_REGION(0x80000, "sdcard_cpu", 0)
	ROM_LOAD("c3zbk0000020.ic404", 0x000000, 0x80000, NO_DUMP)
ROM_END

} // anonymous namespace


//   YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY     FULLNAME     FLAGS
CONS(1999, kn6000, 0,      0,      kn6000,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN6000", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
CONS(2000, kn2400, 0,      0,      kn2400,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN2400", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
CONS(2000, kn2600, kn2400, 0,      kn2400,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN2600", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
CONS(2001, kn6500, 0,      0,      kn6500,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN6500", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
CONS(2002, kn7000, 0,      0,      kn7000,  kn7000, kn7000_state, empty_init, "Technics", "SX-KN7000", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
