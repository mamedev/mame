// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-WSA1R

    A 1995 rack-mounted "Acoustic Modeling Synthesis" synthesizer module:
    64 notes over up to 32 parts, 256 preset sounds, 16 preset drum kits and
    128 preset combinations, a 320 x 240 dot LCD, two sets of MIDI IN/OUT/THRU
    and a built-in 3.5 inch floppy drive.

    Two TLCS-900/H processors share the work.  CPU 1 drives the panel, the
    floppy and MIDI; CPU 2 drives the tone generator and the modeling LSI.

    Hardware inventory below is taken from the SX-WSA1R service manual,
    ORDER NO. EMiD951604, (c) 1995 Matsushita Electric Industrial, which covers
    the rack module only.  The scan available here is photocopy grade: where
    the schematic sheets and the parts list give different spellings both are
    shown, and a designator that was not legible is marked (derived) or
    (unknown).

    A keyboard version of this instrument exists, the SX-WSA1, and it is not
    declared here: the only claim that it runs the same ROM set is the
    redistributor's, and no SX-WSA1 material was available to check it against.

    The four images below are not chip reads.  They are the firmware set that
    has been publicly redistributed for this machine; the copy used here came
    from dbwbp.com in August 2026, and its uploader states it was read from a
    rack SX-WSA1R.  They carry no dump-quality flag because nothing suggests
    the bytes are wrong -- what is second-hand is the provenance, not the
    integrity.  The set is self-consistent: three of the four images end with
    their own build tag ("wsaa_822", "wsac_230", "wsad_54"), matching the AX,
    CX and DX factory part numbers in the manual, and the firmware's ROM
    VERSION screen has exactly three slots, WSA-A/WSA-C/WSA-D, with no WSA-B
    line for the fourth image.

    TODO:
      - dump the six 16 Mbit wave mask ROMs, the AM29F400T flash, and the
        internal ROM of the control panel microcontroller
      - devices with no MAME implementation yet: the L7A1429 modeling LSI, the
        uPD6383GF-3BA DSP, the M37471M2196S panel MCU and the uPD72070 floppy
        disk controller
      - map the flash at 0xE80000 on CPU 2.  The firmware probes it with the
        AMD autoselect sequence (0xAAAA/0x5554 unlock, 0x90, then reads
        0xE80000 and 0xE80002) at prom_c 0xFC85BD, so the part is almost
        certainly the AM29F400T the parts list names.  MAME has AMD_29F400T
        but only as an 8-bit device, and the accepted-device table the
        firmware compares against has not been decoded yet.

***************************************************************************/

#include "emu.h"

#include "cpu/tlcs900/tmp95c061.h"



namespace {

class wsa1_state : public driver_device
{
public:
	wsa1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
	{ }

	void wsa1r(machine_config &config);

private:
	required_device<tmp95c061_device> m_cpu1;
	required_device<tmp95c061_device> m_cpu2;

	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;
};


// CPU 1 fetches prom_a and prom_b; CPU 2 fetches prom_c.  Which of them is IC1
// "MICROCOMPUTER (MAIN)" and which is IC2 "(SUB)" is not established, so the
// tags are neutral.
void wsa1_state::cpu1_map(address_map &map)
{
	// static RAM on CS1 (MSAR1 = 0x00 at InitMSAR1_CS1).  Boot clears from 0x000080
	// and the checksum furniture at 0x007FCA-0x007FD4 sizes the chip at 32 KiB.
	map(0x000080, 0x007fff).ram();

	// work DRAM on CS3 (MSAR3 = 0x60 at InitMSAR3_CS3; P6FC = 0x1F at InitP6FC_LCAS makes
	// the CS3 pin LCAS).  First stack is 0x60EB80.
	map(0x600000, 0x67ffff).ram();

	map(0xf00000, 0xf7ffff).rom().region("prom_ab", 0x000000);   // IC13
	map(0xf80000, 0xffffff).rom().region("prom_ab", 0x080000);   // IC12
}

void wsa1_state::cpu2_map(address_map &map)
{
	map(0x000080, 0x01ffff).ram();
	map(0xf00000, 0xf7ffff).rom().region("prom_d", 0);           // IC21, tone database
	map(0xf80000, 0xffffff).rom().region("prom_c", 0);           // IC28
}


static INPUT_PORTS_START(wsa1r)
INPUT_PORTS_END


void wsa1_state::wsa1r(machine_config &config)
{
	// fc = 28 MHz: the firmware stores it as a byte, prom_c[FcClockByte] = 0x1C
	// read at SerialDivisorFromFc, and computes its own serial divisor from it.
	TMP95C061(config, m_cpu1, 28_MHz_XTAL);
	m_cpu1->set_addrmap(AS_PROGRAM, &wsa1_state::cpu1_map);

	TMP95C061(config, m_cpu2, 28_MHz_XTAL);
	m_cpu2->set_addrmap(AS_PROGRAM, &wsa1_state::cpu2_map);
}


/***************************************************************************

    Hardware inventory, from the SX-WSA1R service manual, ORDER NO. EMiD951604,
    (c) 1995 Matsushita Electric Industrial, which covers the rack module only.
    The scan is photocopy grade: where the schematic and the parts list give
    different spellings both are shown, and a designator that was not legible
    is marked (derived) or (unknown).

    MAIN board

    IC1, IC2     TMP95C061AF    Toshiba TLCS-900/H, "MICROCOMPUTER (MAIN)" and
                                "(SUB)" respectively
    IC3          L7A1429        "MODELING LSI"
    IC4          TC183C230002   "TONE GENELATOR LSI" [sic] (schematic:
                                TC1830230002; designator derived)
    IC5, IC6,    D6383GF-3BA    NEC digital signal processor, three of them
    IC30                        (only IC30 is printed cleanly)
    IC7          SED1330FBA     LCD controller for the 320 x 240 panel;
                                src/devices/video/sed1330.h
    IC12         QSIGCWSA1AX    4 Mbit programmed EPROM, chip select PROMACS
    IC13         QSIGCWSA1BX    4 Mbit programmed EPROM, chip select PROMBCS
    IC28         QSIGCWSA1CX    4 Mbit programmed EPROM, chip select PROMCCS
    (derived)    QSIGCWSA1DX    4 Mbit programmed EPROM, chip select PROMDCS.
                                Parts-list row order puts it at IC21, and the
                                redistributed set names the file that way, but
                                the scan does not confirm it.
    (derived)    AM29F400T      4 Mbit flash memory; row order puts it at IC22
    IC14, IC15   M5256CFP70LL,  256 kbit static RAM and 4 Mbit dynamic RAM; the
                 M5M44170AJ7S   self-diagnostic calls the pair "RAM (IC14, 15)"
                                (parts list: M5M44170AN7S)
    IC23, IC31,  LC321664AJ80   1 Mbit dynamic RAM
    IC32, IC51,
    IC61
    IC27         D74HC139GS     decoder; generates PROMCCS and PROMDCS
    IC33, IC34   M5M44260AJ7S   4 Mbit dynamic RAM (parts list: M5M44260AJN7S)
    IC43         QSIGH3C16DT8   16 Mbit wave mask ROM (schematic: ...DT3)
    IC44         QSIGH3C16EA0   16 Mbit wave mask ROM (schematic: ...EA9)
    IC45         QSIGH3C16EA2   16 Mbit wave mask ROM (parts list: QSIGH38C...)
    IC47         QSIGH3C16DT7   16 Mbit wave mask ROM
    IC48         QSIGH3C16DT9   16 Mbit wave mask ROM
    IC49         QSIGH3C16EA1   16 Mbit wave mask ROM
    IC52 - IC54, PCM1702U       D/A converter, four of them
    IC59
    IC55 - IC58  M5218AFP       operational amplifier
    IC71         LH5P832N-10    256 kbit RAM (schematic: pseudo static)
    (unknown)    D72070GF3BE    NEC floppy disk controller, 3.5 inch 2HD
                                1.44 MB / 2DD 720 KB

    CONTROL PANEL 1 board

    (unknown)    M37471M2196S   Mitsubishi panel microcontroller, the same part
                                as the two MCUs in kn5000_cpanel.cpp.  Its mask
                                ROM is undumped and no region is declared for
                                it, because the manual does not give a capacity.
    (unknown)    HD74LS07P      hex buffer

    Capacities are the manual's own.  It never says what any of the four EPROMs
    holds; the roles in the ROM definitions below were read out of the images.
    Only devices with a legible part number are listed; discrete logic, the
    power supply and the SY-EW1 / SY-ES1 option boards are omitted.

***************************************************************************/

ROM_START(wsa1r)
	// The OS v2.0 set.  A v1 OS shipped and is not dumped; whether it used
	// different factory part numbers is not known, so the revision is carried
	// in the file names rather than left to the part number.
	// Regions are named after the chip-select nets, not after processors:
	// which of IC1 ("MAIN") and IC2 ("SUB") fetches which pair is not
	// established.

	// A and B share one address space.  B at PromB_JumpTable opens with a five entry
	// jump table and holds the user interface text (English, German, French)
	// and the service test screens.  A at PromC_Base puts its vector table at
	// PromC_Vectors, where a TMP95C061 fetches vectors; reset is PromA_Reset.
	ROM_REGION16_LE(0x100000, "prom_ab", 0)
	ROM_LOAD("qsigcwsa1bx_v2.ic13", 0x000000, 0x080000, CRC(f3f84441) SHA1(93adec2a04b7d93a2ec2bfb059227ff3959906e0)) // B, at 0xf00000
	ROM_LOAD("qsigcwsa1ax_v2.ic12", 0x080000, 0x080000, CRC(5f34af46) SHA1(90a2369f8e4d2fcdf26875272267624b07bc200d)) // A, at 0xf80000

	// C is the other processor's program, at PromC_Base in its own space, with
	// an independent vector table at PromC_Vectors and reset PromC_Reset.  It also
	// holds data: 0x000000-0x0165BF is a "ZZZZ" headed block with a 16 entry
	// category table and 128 combinations of 704 bytes; code runs 0x018000 to
	// 0x0621E4; boot code and vectors from 0x07F000.
	ROM_REGION16_LE(0x080000, "prom_c", 0)
	ROM_LOAD("qsigcwsa1cx_v2.ic28", 0x000000, 0x080000, CRC(855c8ac4) SHA1(9b2911e4b21a08d9744b91844630489f54dde856)) // at 0xf80000

	// D holds no executable content.  It is a tone bank: 32-bit image-relative
	// offsets at 0x000000, a 274 entry pointer directory at 0x000B80 (256
	// sounds then 18 drum kit records, each headed by a 16 byte printable
	// name), payload ending at 0x050B08.  A region of its own because it is a
	// separate chip on a separate chip select, IC27's 2Y2 = PROMDCS.  Sheet
	// II-11/II-12 draws IC21 QSIGCWSA1DX beside that decoder.
	ROM_REGION16_LE(0x080000, "prom_d", 0)
	ROM_LOAD("qsigcwsa1dx_v2.ic21", 0x000000, 0x080000, CRC(735ae465) SHA1(82df50816c20cd8f2d29551326d2633e7791f306))

	// Wave ROMs, undumped.  The self-diagnostic covers IC43-IC45 and IC47-IC49;
	// there is no IC46.  The manual gives 16 Mbit but not the organisation, and
	// which device sits on which tone-generator bus is not resolved, so each
	// gets its own region rather than being concatenated into a bank.
	ROM_REGION(0x200000, "waveform_ic43", 0)
	ROM_LOAD("qsigh3c16dt8.ic43", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic44", 0)
	ROM_LOAD("qsigh3c16ea0.ic44", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic45", 0)
	ROM_LOAD("qsigh3c16ea2.ic45", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic47", 0)
	ROM_LOAD("qsigh3c16dt7.ic47", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic48", 0)
	ROM_LOAD("qsigh3c16dt9.ic48", 0x000000, 0x200000, NO_DUMP)

	ROM_REGION(0x200000, "waveform_ic49", 0)
	ROM_LOAD("qsigh3c16ea1.ic49", 0x000000, 0x200000, NO_DUMP)

	// A 4 Mbit flash device that the block diagram puts on the same address
	// group as the program EPROMs.  Not held here, and the manual does not say
	// what it holds.  Named after the part number because its designator is
	// not legible either.
	ROM_REGION(0x080000, "flash", 0)
	ROM_LOAD("am29f400t.bin", 0x000000, 0x080000, NO_DUMP)
ROM_END

} // anonymous namespace


//   YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY     FULLNAME    FLAGS
SYST(1995, wsa1r, 0,      0,      wsa1r,   wsa1r, wsa1_state, empty_init, "Technics", "SX-WSA1R", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
