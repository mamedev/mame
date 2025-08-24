// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    krz2000.cpp - Kurzweil K2000 series

    Skeleton driver by R. Belmont
    Additional improvements by Peter Sobot

    Hardware in brief:
        TMP68301 CPU @ 16 MHz (with debug port on RXD2/TXD2)
        uPD72064 FDC
        85C30 SCSI (53C80 on schematic?)
        M37450 @ 10 MHz on I/O board to handle panel/display/keyboard scanning, LED control, LCD contrast, and the audio switching matrix
         (16K of ROM stored on U69, 384 bytes of RAM, 8 A/D inputs, 2 D/A outputs, 6 ports of I/O, and a 31.25Kbaud serial link to the maincpu)
        HD6303 on I/O board to manage reverb DSP program loading
        VLSI ASIC "Calvin" - Handles maincpu and PCM ROM/RAM addressing, as well as resampling (and possibly DRAM refresh)
         (on later K2000 and all later K-series units this is replaced by a more advanced VLSI ASIC "Janis")
         (service manual says "sample fetching tasks")
        2x VLSI ASIC "Hobbes" - specialized DSP processors for sound synthesis (oscillators, filters, etc)
         (service manual says "wave form generation and DSP functions")
         (each Hobbes chip handles a subset of available voices, and seems independently programmable)
        Digitech DSP256 Multi Effects VLSI Chip (reverb, chorus, delay, etc)
         (EPROM seems avaialble on eBay, but the chip itself is quite custom)

***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "cpu/m6502/m3745x.h"
#include "cpu/m68000/tmp68301.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/ncr5380.h"
#include "machine/upd765.h"
#include "video/t6963c.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

namespace {

class k2000_state : public driver_device
{
public:
	k2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		// , m_scanner(*this, "scanner")
		, m_1m_bank(*this, "bank1m")
		, m_mainram(*this, "mainram")
		// , m_midi(*this, "midi")
		// , m_fdc(*this, "fdc")
	{ }

	void k2000(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void k2000_map(address_map &map) ATTR_COLD;
	void bank_map_1m(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	// required_device<cpu_device> m_scanner;
	required_device<address_map_bank_device> m_1m_bank;
	required_shared_ptr<uint16_t> m_mainram;
	// optional_device<midi_port_device> m_midi;
	// required_device<upd765a_device> m_fdc;

	void swap_bit_ctrl_w(uint16_t data);
	uint16_t battery_voltage_and_keyscanner_version(offs_t offset);

	uint16_t calvin_read(offs_t offset);
	void calvin_write(offs_t offset, uint16_t data);

	uint16_t hobbes0_read(offs_t offset);
	void hobbes0_write(offs_t offset, uint16_t data);

	uint16_t hobbes1_read(offs_t offset);
	void hobbes1_write(offs_t offset, uint16_t data);

	void k2000_palette(palette_device &palette) const;

	TIMER_CALLBACK_MEMBER(boot_hack_timer);
	emu_timer *m_boot_hack_timer;
};

/**
 * From the K2000/K2000R service manual:
 *
 *  The 68301 processor's 16 megabits (sic) of address space are divided into 16 equal memory blocks,
 *  numbered 0-15. The first time the K2000 is powered up, Block 0 processes data for the boot
 *  EPROM and setup EPROM, while Block 1 processes data for the PSRAM. Many of the K2000's basic
 *  functions utilize the PSRAM, which accesses the higher address space of Block 1 in the CPU.
 *  This slows down the access time somewhat, since the address space of the CPU is accessed in
 *  sequence, beginning with Block 0.
 *
 *  To optimize the processing speed, the K2000 performs a one-time software operation when
 *  powered up for the first time. The memory management unit (MMU) in the gate array logic chip
 *  (GAL) located at U35 on the engine board executes a "swap bit" function which makes Block 0
 *  of the CPU responsible for the PSRAM, and Block 1 responsible for the Boot EPROM and Setup
 *  EPROM. Consequently the PSRAM functions are processed as early as possible.
 *
 * Note this "16 equal memory blocks" idea is really just the top hex digit of the address -
 * i.e.: 0x?FFFFF
 */
void k2000_state::swap_bit_ctrl_w(uint16_t data)
{
	data &= 0xff;
	logerror("%02x written to swap_bit_ctrl_w\n", data);
	// bit 4:
	//  0 = program ROM at 0x000000, work RAM    at 0x100000
	//  1 = work RAM    at 0x000000, program ROM at 0x100000
	m_1m_bank->set_bank((data >> 4) & 1);
}

uint16_t k2000_state::battery_voltage_and_keyscanner_version(offs_t offset)
{
	logerror("reading battery voltage and keyscanner version at %02x\n", offset);
	uint8_t battery_voltage = 0x36;
	uint8_t keyscanner_version = 0x12;
	return (((uint16_t) battery_voltage) << 8) | ((uint16_t) keyscanner_version);
}

uint16_t k2000_state::calvin_read(offs_t offset)
{
	logerror("Read from Calvin at %06x\n", offset);
	return 0;
}

void k2000_state::calvin_write(offs_t offset, uint16_t data)
{
	logerror("Write to Calvin at %06x = %04x\n", offset, data);
}

uint16_t k2000_state::hobbes0_read(offs_t offset)
{
	logerror("Read from Hobbes[0] at %06x\n", offset);
	return 0;
}

void k2000_state::hobbes0_write(offs_t offset, uint16_t data)
{
	logerror("Write to Hobbes[0] at %06x = %04x\n", offset, data);
}


uint16_t k2000_state::hobbes1_read(offs_t offset)
{
	logerror("Read from Hobbes[1] at %06x\n", offset);
	return 0;
}

void k2000_state::hobbes1_write(offs_t offset, uint16_t data)
{
	logerror("Write to Hobbes[1] at %06x = %04x\n", offset, data);
}

void k2000_state::machine_start()
{
	m_boot_hack_timer = timer_alloc(FUNC(k2000_state::boot_hack_timer), this);
	m_boot_hack_timer->adjust(attotime::from_seconds(2), 0, attotime::never);
}

void k2000_state::machine_reset()
{
	m_1m_bank->set_bank(0);
}

TIMER_CALLBACK_MEMBER(k2000_state::boot_hack_timer)
{
	logerror("Jumping to interrupt vector to continue boot sequence");
	m_maincpu->set_pc(m_1m_bank->read32(0x80));
}

void k2000_state::k2000_map(address_map &map)
{
	// word writes to 000000 region - unknown
	// word writes to 000090 region - unknown
	// word writes to 000180 region - unknown
	// word writes to 000240 region - unknown

	// The engine ROMs are mapped onto 0x0 when ENGROMCSb is low, which is IO3 of GODOT (pin 18)
	// The setup ROMs are mapped onto 0x100000 when SUROMCSb is low, which is the IO2 output (pin 17) of GODOT
	// This implies that IO3 of GODOT is low iff CA{20,23} are also low, and IO2 of GODOT is low iff CA{20,23} = 0x01

	// Block 0, Block 1: ROM and RAM (depending on bank switch)
	map(0x000000, 0x1fffff).m(m_1m_bank, FUNC(address_map_bank_device::amap16));

	// Lots of writes to 0x372B68 -> 0x372C8C

	// Block 4: PRAM?
	// "The K2000 comes standard with 116KB, and with the PRAM upgrade, totals 760KB."
	// Another source says:
	//  For the K2000 Series, the total memory allocated to all of these objects is
	//  120K expandable to 760K with the PRAM option. For the K2500 Series, it is 256k,
	//  expandable to 1280k with the PRAM option. For the K2600 Series, it is 486k,
	//  expandable to 1506k with the PRAM option.
	// These are pretty odd numbers - but it sounds like ~8KB of the RAM space is used
	// for OS functions, meaning the built-in RAM might only be 128KB in total.
	// The K2000 schematic shows 2x 1M "NVRAM" chips, one for the low byte, and one for
	// the high byte, which means we should (?) actually have 2 * 1024 * 1024 / 8 = 262,144 bytes?
	//
	// The boot routines seem to write to 0x400000 and compare the value saved there, so
	// this is probably the optional PRAM expansion.
	map(0x400000, 0x400000 + ((2 * 1024 * 1024) / 8) - 1).ram();

	// Init routine sets 0x4BAB44 -> 0x4BFFFA to 0x0000
	// Reads from 0x4BFFFC and 0x4BFFFE seem to happen on startup.
	// HOBBES0CSb = NAND(CS1', CA19)  = high unless CS1 is high and CA19 is high
	// HOBBES1CSb = NAND(CS1', !CA19) = high unless CS1 is high and CA19 is low
	// 0xFFFC00 is the Address Decoder area that controls which part of memory causes CS1 to be "enabled" (low)
	// At runtime,
	//  0xFFFC00 = 0x50 =
	//  0xFFFC01 = 0x3F
	//  0xFFFC02 = 0x00
	//  0xFFFC03 = 0x30
	//   For CS1:
	//  0xFFFC04 = 0x60 = 0b0110 0000 = Start address of the memory area for CS1 = A21 + A22
	//  0xFFFC05 = 0x3F = 0b0011 1111 = Size of memory area = M19 + M18 + M17 + M16 + M15-M9 + M8 = 1MB
	//  0xFFFC06 = 0x00
	//  0xFFFC07 = 0x30 = 0b0011 0000 = Area enabled, use external DTACK
	// By this logic, Hobbes chips are mapped to 0x100000, 0x200000, 0x400000, 0x600000
	// And their address mask is 0x000FFF00, which means the effective ranges are:
	// 0x100???00

	// Hobbes chip selection seems to look at CA19 and CS1

	// Internal sample ROM is switched on SBA20,SBA21, which corresponds to CA21,CA22 (one bit off)
	// meaning the internal address space for accessing the sample ROM is 0x200000 through 0x6fffff (?)
	// Blocks 2, 3, 4, 5, 6
	// map(0x200000, 0x2fffff).rom().region("pcm", 0);        // Block 2: Sample ROM
	// map(0x300000, 0x3fffff).rom().region("pcm", 0x100000); // Block 3: Sample ROM
	// map(0x400000, 0x4fffff).rom().region("pcm", 0x200000); // Block 4: Sample ROM
	// Writes to 0x4FFD82, 0x4FFD88, 0x4FFE24

	// Hypothesis: Calvin (or Janis) is mapped at 0x500000
	//  Writes to 0x500402, 0x500418, 0x50041A early in the boot process
	//  Initialization then writes zeros in a descending ladder:
	//  0x500400, 0x500402
	//  0x5003FC, 0x5003FE, ...
	//  down to 0x500004 (then 0x500006)

	// Calvin memory?
	map(0x500000, 0x5003ff).rw(FUNC(k2000_state::calvin_read), FUNC(k2000_state::calvin_write));
	// Calvin control registers?
	map(0x500400, 0x5004ff).rw(FUNC(k2000_state::calvin_read), FUNC(k2000_state::calvin_write));
	// Reads and writes to 0x580000

	// Hobbes (it seems) is connected in the 0x600000 range
	// The second Hobbes chip (?) seems connected at 0x680000
	// Initialization seems to:
	//  - write to 0x600300, followed by 0x600xxx where xxx is in 0x400 through 0x5FF
	//  - write to 0x6?0002, 0x6?0006, 0x6?0004, through 0x6?000F4
	//  - write to 0x600004, 0x600014, 0x600024... through 0x6002F4
	//    (with a repeating pattern of [0x02, 0x00, 0x26, 0x24, 0x4A, 0x48])
	//  - write to 0x680004, 0x680014, 0x680024... through 0x6802F4
	//    (with a repeating pattern of [0x80, 0x02, 0xA4, 0x26, 0xC6, 0x4A])
	// Each Hobbes chip seems to have two control registers, mapped at 0x600300/0x680300 and 0x600400/0x680400

	// Hobbes memory?
	map(0x600000, 0x6002ff).rw(FUNC(k2000_state::hobbes0_read), FUNC(k2000_state::hobbes0_write));
	// Hobbes control registers?
	map(0x600300, 0x6005ff).rw(FUNC(k2000_state::hobbes0_read), FUNC(k2000_state::hobbes0_write));

	// Hobbes memory?
	map(0x680000, 0x6802FF).rw(FUNC(k2000_state::hobbes1_read), FUNC(k2000_state::hobbes1_write));
	// Hobbes control registers?
	map(0x680300, 0x6805FF).rw(FUNC(k2000_state::hobbes1_read), FUNC(k2000_state::hobbes1_write));

	// SROM - Setup ROM?
	uint32_t srom_base = 0x280000;
	map(srom_base, srom_base + 0x40000 - 1).rom().region("maincpu", 0x100000);

	// Calvin chip has a 20-bit address bus with A0 tied low and A12-15 tied high, giving it access to 0x?0??[0-E]
	// Later models' Janis chips use 24-bit address buses (or at least they're all connected in the schematic)
	// Hobbes has a 10-bit address bus with A0 tied low, giving access to 0x0 through 0x3FF

	// Block 7: LCD control and bank switching
	map(0x700000, 0x700003).rw("lcd", FUNC(lm24014h_device::read), FUNC(lm24014h_device::write)).umask16(0x00ff);

	// Reads from 0x740000 (floppy?)
	// Writes to 0x760000
	// Writes to 0x7C0000

	// Single byte reads seem to happen at 0x740000, which seems to be FDCCTLCSb?
	// map(0x740000, 0x74000f).
	map(0x7e0000, 0x7e0001).w(FUNC(k2000_state::swap_bit_ctrl_w));

	// Some sort of hardware mapping - initial firmware boot expects a 4 at $78000B
	map(0x780000, 0x78ffff).ram();

	// When (CPUASb + GODOT I6) and (GODOT I/O0 + IOCSb) are low,
	// Address bits 17, 18, 19 get demultiplexed onto:
	// 0, 0, 0 => LCDCSb
	// 0, 0, 1 => FDCCSb
	// 0, 1, 0 => FDCCTLCSb
	// 0, 1, 1 => FDCOUTCSb
	// 1, 0, 0 => SCSICSb
	// 1, 0, 1 => SCSIDMACSb
	// 1, 1, 0 => CPUEXPCSb
	// 1, 1, 1 => MCTLREGCSb (OR'ed with WRITELOb to provide input into the Memory Management Control Register)

	// map(0x800000, 0x8fffff); // Block 8: ???
	// map(0x900000, 0x9fffff); // Block 9: ???
	// map(0xa00000, 0xffffff); // Blocks 10, 11, 12, 13, 14: Orchestral ROM
	// map(0x1000000, 0x18fffff); // Blocks 15, 16, 17, 18, 19: Contemporary ROM
}

void k2000_state::bank_map_1m(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x100000, 0x1fffff).ram().share("mainram");

	// The "stride" of this bank map is 0x200000, so this is the second "view" of the same data
	map(0x200000, 0x2fffff).ram().share("mainram");
	map(0x300000, 0x3fffff).rom().region("maincpu", 0);

	// Unsure if the battery voltage and keyscanner data is memory-mapped at these ranges (unlikely)
	// or if this data is written to some time during the boot process. If it's the latter,
	// this will have to change when loading different versions of the ROM (or someone has to figure
	// out where the battery voltage is actually read from).
	map(0x100000 + 0x1052, 0x100000 + 0x1053).r(FUNC(k2000_state::battery_voltage_and_keyscanner_version));
	map(0x200000 + 0x1052, 0x200000 + 0x1053).r(FUNC(k2000_state::battery_voltage_and_keyscanner_version));
}

void k2000_state::k2000(machine_config &config)
{
	TMP68301(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &k2000_state::k2000_map);

	// MIDI_PORT(config, m_midi, midiin_slot, "midiin");
	// TODO: Uncomment when the TMP68301 has serial/MIDI support, to wire up MIDI I/O and scanner.
	// MIDI_PORT(config, m_midi, midiout_slot, "midiout");

	// M37450(config, m_scanner, XTAL(10'000'000));
	// The "Scanner" sub-CPU is connected to main CPU via:
	//  - MIDI_FM_ENG, going to P34/RxD, which comes from SERTxD, which is INTMIDITxD == TxD1
	//  - 37450_TXD, coming from P35/TxD, which is NAND'd together with 6303_TXD
	//    and inverted to provide SERRxD to the engine (maincpu), which goes to INTMIDIRxD
	//    eventually ending up at RxD1
	// Essentially, the 68k's second built-in serial port runs at 31250Hz, which is MIDI baud rate,
	// which is how it talks to the scanner to tell it to operate the LEDs on the front of the case.
	// The external MIDI port(s) are connected directly to the 68K's first serial port, RxD0 and TxD0.
	// (RxD2 and TxD2 are hooked up to a "debug port" which the manual says requires additional
	// hardware - this is untested.)
	// The scanner's ROM comes from an NTE2764 (U69, 28-pin DIP) EPROM of which 16Kb are used.
	// (Note that on the K2500 and above, the scanner is an M37451M8-FP the ROM is 16KB, but stored on an M27256.)
	// The scanner ROM from the K2500 may be compatible with the K2000, and can be pulled from "U2"
	// on the K2500 Audio Scanner Board (Rev D)

	ADDRESS_MAP_BANK(config, "bank1m").set_map(&k2000_state::bank_map_1m).set_options(ENDIANNESS_BIG, 16, 24, 0x200000);

	// uPD765AC; clocked through SED9420C
	// UPD765A(config, m_fdc, 16_MHz_XTAL / 4, true, true);
	// m_fdc->intrq_wr_callback().set_inputline(m_maincpu, I6);
	// FLOPPY_CONNECTOR(config, "fdc:0", dss1_floppies, "35dd", floppy_image_device::default_floppy_formats).enable_sound(true);
	// FLOPPY_CONNECTOR(config, "fdc:1", dss1_floppies, nullptr, floppy_image_device::default_floppy_formats).enable_sound(true);

	PALETTE(config, "palette", FUNC(k2000_state::k2000_palette), 2);

	LM24014H(config, "lcd");

	SPEAKER(config, "speaker", 2).front();
}

void k2000_state::k2000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}


static INPUT_PORTS_START( k2000 )
INPUT_PORTS_END

ROM_START( k2000 )
	ROM_REGION(0x140000, "maincpu", 0)
	// note that since this firmware revision ends with a J, it is intended for a K2000 with the "Janis" Sample/Addressing ASIC (the later mainboard)
	// the earlier board has firmware revisions which end with a C, for the "Calvin" Sample/Addressing ASIC, on the older K2000 mainboard
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_hi__v2.0j_3b69__=c=_1993_yca.tms27c040.u6", 0x000000, 0x080000, CRC(35c17fc3) SHA1(b91deec0127669b46af05a2acaa212e29e49abfb) ) // TMS27C040 EPROM with sticker: "K2J-K2RJ ENG HI // V2.0J 3B69 // (C) 1993 YCA" @ U6
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_lo__v2.0j_0db0__=c=_1993_yca.tms27c040.u3", 0x000001, 0x080000, CRC(11c7f436) SHA1(c2afe84b58d71932f223097ea01812eb513bd740) ) // TMS27C040 EPROM with sticker: "K2J-K2RJ ENG LO // V2.0J 0DB0 // (C) 1993 YCA" @ U3
	ROM_LOAD16_BYTE( "k2j-k2rj_su_hi__v12ts_5e89__=c=_1993_yca.m27c1001.u5", 0x100000, 0x020000, CRC(16e0bdb7) SHA1(962fa10896f6a95210d752be28f02640869893a4) ) // MC27C1001 EPROM with sticker: "K2J-K2RJ SU HI // V12TS 5E89 // (C) 1993 YCA" @ U5
	ROM_LOAD16_BYTE( "k2j-k2rj_su_lo__v12ts_2f52__=c=_1993_yca.m27c1001.u2", 0x100001, 0x020000, CRC(cb11e837) SHA1(bcdf3d5abe8c53727a142008acb2755ed0ecc6ea) ) // MC27C1001 EPROM with sticker: "K2J-K2RJ SU LO // V12TS 2F52 // (C) 1993 YCA" @ U2

	ROM_REGION(0x2000000, "pcm", 0) // 23c080(?) 8MBit mask roms, byte-mode, in pairs. These are common on at least several members of the K2000 series; each has a unique number stored in the JEDEC id field.
	ROM_LOAD16_BYTE( "k2m1h_25da__830106-01__=c=1991_yca__japan_9322_d.ide6ed.u38", 0x000001, 0x100000, CRC(f110b0e7) SHA1(d8731b74b1ca6761f8fd3f6360bfe2f1cc3077bc) ) // Silkscreen: "K2M1H 25DA // 830106-01 // (C)1991 YCA // JAPAN 9322 D" @ U38; chip JEDEC id = e6 ed
	ROM_LOAD16_BYTE( "k2m1l_20fe__830107-01__=c=1991_yca__japan_9324_d.idb9a4.u43", 0x000000, 0x100000, CRC(00715fbe) SHA1(99f661096031b794de216c74ce9b780e9889d344) ) // Silkscreen: "K2M1L 20FE // 830107-01 // (C)1991 YCA // JAPAN 9324 D" @ U43; chip JEDEC id = b9 a4
	ROM_LOAD16_BYTE( "k2m2h_de34__830108-01__=c=1991_yca__japan_9324_d.id0000.u39", 0x200001, 0x100000, CRC(99aae00e) SHA1(7045fb6b19b046f3f068a3581b6498ee62603fb4) ) // Silkscreen: "K2M2H DE34 // 830108-01 // (C)1991 YCA // JAPAN 9324 D" @ U39; chip JEDEC id = 00 00
	ROM_LOAD16_BYTE( "k2m2l_1bdf__830109-01__=c=1991_yca__japan_9324_d.id836c.u44", 0x200000, 0x100000, CRC(b2acd497) SHA1(24d3e84016fa08a990ce4c39294ad47fb0cab3d0) ) // Silkscreen: "K2M2L 1BDF // 830109-01 // (C)1991 YCA // JAPAN 9324 D" @ U44; chip JEDEC id = 83 6c
	ROM_LOAD16_BYTE( "k2m3h_0e87__830110-01__=c=1991_yca__japan_9324_d.iddbfa.u40", 0x400001, 0x100000, CRC(f448694f) SHA1(484593d072c43fe442cd8cc6cc40cd24677b35cc) ) // Silkscreen: "K2M3H 0E87 // 830110-01 // (C)1991 YCA // JAPAN 9324 D" @ U40; chip JEDEC id = db fa
	ROM_LOAD16_BYTE( "k2m3l_3cde__830111-01__=c=1991_yca__japan_9329_d.ide8b5.u46", 0x400000, 0x100000, CRC(be8408f9) SHA1(fbeab2d690532d055d424be52d937e2729b3daac) ) // Silkscreen: "K2M3L 3CDE // 830111-01 // (C)1991 YCA // JAPAN 9329 D" @ U46; chip JEDEC id = e8 b5
	ROM_LOAD16_BYTE( "k2m4h_3f2d__830112-01__=c=1991_yca__japan_9324_d.iddbdb.u42", 0x600001, 0x100000, CRC(da8666f5) SHA1(4d0f306cad9a3a96cf1232b9b8df12fae044a1d6) ) // Silkscreen: "K2M4H 3F2D // 830112-01 // (C)1991 YCA // JAPAN 9324 D" @ U42; chip JEDEC id = db db
	ROM_LOAD16_BYTE( "k2m4l_2e6d__830113-01__=c=1991_yca__japan_9327_d.id3199.u47", 0x600000, 0x100000, CRC(6eb73185) SHA1(fe48fe44be90a856251974750b1eac7f5291e1e6) ) // Silkscreen: "K2M4L 2E6D // 830113-01 // (C)1991 YCA // JAPAN 9327 D" @ U47; chip JEDEC id = 31 99

	/* The K2000 can be expanded by installing up to two
	 *  "K2000 Sound Rom Babybd" P/N 331022-01 sound modules, onto a
	 *  "K2000 SOUND ROM DAUGHTERBOARD" RMB-k P/N 331021-03 daughterboard,
	 * as well as upgrading the board firmware to turn the unit into a
	 * K2000R. The shipping 'K2000R' model just needs this daughterboard
	 * to be added. (Did the K2000R come with the daughterboard populated?)
	 * http://tk386.com/k2000_innards/DSC_8139.jpg
	 *
	 * The 'babyboards' have two mezzanine connectors on them and plug into
	 * the daughterboard, which itself connects to the K2000 mainboard via
	 * connector J12 (a 3-row eurocard connector with 16 pins per row).
	 * The babyboard ROMs are 23c1610 16MBit mask ROMs, used in word mode.
	 *
	 * These same 4-ROM expansion modules can be used on the K2000, K2500,
	 * K2600, and is included in the K2661 from factory.
	 *
	 * Note that the K2600 uses a similar part called RMB-26 which has an
	 * additional 2 ROM slots intended for more dense single-chip ROMs.
	 * see http://kurzweil.com/accessory/rmb-26_soundblock_daughterboard/
	 */


	/* 'Orchestral' babyboard AKA "ROM 1" or 'Kurzweil RM-126':
	 * Note: There are several variations on the ROM markings for the ROMs on
	 * this babyboard, depending on what year it was produced.
	 * The contents are presumed (but not proven) to be the same.
	 * The older board variant presumably has a part number of K91B10162.
	 */
	// older board variant:
	// https://images.reverb.com/image/upload/s--DdCT5Kan--/a_180/f_auto,t_large/v1511800096/wiysqmlb9udd4hgxpppz.jpg
	// https://thumbs.worthpoint.com/zoom/images1/1/0716/15/kurzweil-rm1-orchestral-rom-block_1_3dca4ab8eb35682075d5e5cb77e4876c.jpg
	// Silkscreen: "KZK ROM 1-1 // 830114-01 N // (C) 1993 YCA" @ U1
	// Silkscreen: "KZK ROM 1-2 // 830115-01 N // (C) 1993 YCA" @ U2
	// Silkscreen: "KZK ROM 1-3 // 830116-01 N // (C) 1993 YCA" @ U3
	// Silkscreen: "KZK ROM 1-4 // 830117-01 N // (C) 1993 YCA" @ U4
	// newer board variant:
	// https://media.sweetwater.com/api/i/q-82__ha-8bb2b6f5cf9d25a9__hmac-757ef5825b7f607b540327a23b1e1ace1fe15df5/images/items/750/RM126-large.jpg
	ROM_LOAD( "k2xxrom1-1__830114-01n__2003kurzweil__0402.u1", 0x800000, 0x200000, CRC(39b55c26) SHA1(5937365835a9c1038dfd73d8624f4d7ab0ec48fa) ) // Silkscreen: "K2xxROM1-1 // 830114-01N // <square symbol>2003KURZWEIL // 0402" @ U1
	ROM_LOAD( "k2xxrom1-2__830115-01n__2003kurzweil__0402.u2", 0xa00000, 0x200000, CRC(fba51423) SHA1(4cfdb08c297dca15fddd13963bed0ef501b67c55) ) // Silkscreen: "K2xxROM1-2 // 830115-01N // <square symbol>2003KURZWEIL // 0402" @ U2
	ROM_LOAD( "k2xxrom1-3__830116-01n__2003kurzweil__0402.u3", 0xc00000, 0x200000, CRC(4efedf7f) SHA1(b6888d533a6338aae4d51d3b70f964eb96266069) ) // Silkscreen: "K2xxROM1-3 // 830116-01N // <square symbol>2003KURZWEIL // 0402" @ U3
	ROM_LOAD( "k2xxrom1-4__830117-01n__2003kurzweil__0402.u4", 0xe00000, 0x200000, CRC(4b83d510) SHA1(061a0e954779cb7214ca17bb3a879bb7bc36eddd) ) // Silkscreen: "K2xxROM1-4 // 830117-01N // <square symbol>2003KURZWEIL // 0402" @ U4


	// 'Contemporary' babyboard AKA "ROM 2" or 'Kurzweil RM-226':
	// see https://images.reverb.com/image/upload/s--t9U6Ex2u--/a_180/f_auto,t_large/v1511749743/r0fglivtamtoqrevhatt.jpg
	// and https://images.reverb.com/image/upload/s--6eU1RGQ4--/a_exif,c_limit,e_unsharp_mask:80,f_auto,fl_progressive,g_south,h_620,q_90,w_620/v1524633114/gxnbseznct6cptmspizt.jpg
	// Note there is a older version of contemporary with roms 830131-01
	// thru 830134-01; this older version is a VERY MUCH WANTED dump.
	// It only seems to have been produced during a short period in 1994-1995?
	// The old version may share some roms with the Kurzweil SP76 and SP88
	// Stage Pianos.
	// https://www.mobikin.com/images/android/rom.jpg

	// newer contemporary is 830146-01 4463, 830147-01 C9BE, 830148-01 93B2, 830134-01 F265
	ROM_LOAD( "k2xxrom2-1__830146-01n__2003kurzweil__0402.u1", 0x1000000, 0x200000, CRC(03628775) SHA1(1db7dd514c2a7b810d0e5fef6f614498d695879e) ) // Silkscreen: "K2xxROM2-1 // 830146-01N // <square symbol>2003KURZWEIL // 0402" @ U1; from alt ROM labels, sum16 is 4463
	ROM_LOAD( "k2xxrom2-2__830147-01n__2003kurzweil__0402.u2", 0x1200000, 0x200000, CRC(7b81c227) SHA1(954d41e9fce54eb4a4ce81b5095227a1478a6828) ) // Silkscreen: "K2xxROM2-2 // 830147-01N // <square symbol>2003KURZWEIL // 0402" @ U2; from alt ROM labels, sum16 is c9be
	ROM_LOAD( "k2xxrom2-3__830148-01n__2003kurzweil__0403.u3", 0x1400000, 0x200000, CRC(62af1ba7) SHA1(8910abfd33a939d8a20cd69576c94342f194e23a) ) // Silkscreen: "K2xxROM2-3 // 830148-01N // <square symbol>2003KURZWEIL // 0403" @ U3; from alt ROM labels, sum16 is 93b2
	ROM_LOAD( "k2xxrom2-4__830134-01n__2003kurzweil__0402.u4", 0x1600000, 0x200000, CRC(11f3dfb6) SHA1(a74e040b316f7a6042368c6ae9c2b0cda8656614) ) // Silkscreen: "K2xxROM2-4 // 830134-01N // <square symbol>2003KURZWEIL // 0402" @ U4; from alt ROM labels, sum16 is f265

	// Note1: the roms 830122-01 thru 830127-01 are from the Kurzweil MK-10
	// see https://images.reverb.com/image/upload/s--sBo5xoMU--/a_exif,c_limit,e_unsharp_mask:80,f_auto,fl_progressive,g_south,h_1600,q_80,w_1600/v1504391767/v7iopgveuh6z42kxzobo.jpg
	// Note2: the rom 830129-01 is from the Kurzweil RG200

	ROM_REGION(0x3000, "pals", 0)
	ROM_LOAD( "pseudo_v4d.u11.gal16v8b.jed", 0x000000, 0x000bd0, CRC(43561132) SHA1(a0c567c81022bc7fb83023d89556ccd5aa1ab36d) )
	ROM_LOAD( "sndram_v1.u50.gal16v8b.jed", 0x001000, 0x000bd0, CRC(cabc9335) SHA1(968fa5baa43c7589c901f09b12085437834aeb37) )
	// GODOT seems to take in the top 4 maincpu address lines (A20-A24) and multiplex/decode onto 11 (?) I/O lines
	// Effectively takes care of the "memory block" logic (MMU/bank switching) that the service manual describes
	ROM_LOAD( "godot_v5.u10.gal20v8a.jed", 0x002000, 0x00066f, CRC(c6517456) SHA1(b82530d46afdca5f6460e77ac11710cad55a6b89) )
	// There's an additional GAL20V8 for the MMU - exact logic unknown, but the MCTL[0-3] lines are used, which seem
	// like the "address space select" lines - so at least one or more of those address space areas map directly to SBA[22-25], which
	// seem to be the Sample Bank address lines.
ROM_END

} // anonymous namespace

SYST( 1990, k2000, 0, 0, k2000, k2000, k2000_state, empty_init, "Kurzweil Music Systems", "K2000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
