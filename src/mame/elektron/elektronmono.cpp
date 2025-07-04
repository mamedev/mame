// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    elektronmono.cpp - Elektron Machinedrum and Monomachine

    Skeleton driver by R. Belmont

    The Machinedrum and Monomachine are basically the same hardware
    platform, only the front panel is different. Both have two major
    hardware revisions with different features (MK 1 and MK 2), and a
    "+Drive" storage option/upgrade.

    There are also "user waveform" (UW) versions of the Machinedrum MK 1
    and MK 2, and a keyboard version of the Monomachine MK 1 only. All
    Monomachine MK 2 units support user waveforms, there's no separate
    UW version. The firmware covers all models.

    The same circuit boards are used in both Machinedrum and
    Monomachine, at least in MK 2. I haven't found photos of the top of MK 1
    circuit boards, so some of the memory configurations are guesses based
    on firmware code, and may not be accurate.

    MCU: Motorola Coldfire 5206e
    - Flash memory: 8 Mbyte for Machinedrum UW models and Monomachine MK 2,
    1 Mbyte for others?
    - Main memory: 1 Mbyte DRAM for MK 1, 1 MByte SRAM for MK 2
    - Battery backed "patch" memory: 512 kByte SRAM for Machinedrum MK 1, 1
    MByte SRAM for others

    DSP: 2x Motorola DSP56303
    - Memory for each DSP: 3x 64 kbyte SRAM + 3x 256 kbyte for MK 1, 3x 512
    kbyte SRAM for MK 2
    - Additional memory for DSP 2 in Machinedrum UW: 3x 512 kbyte DRAM for
    MK 1, 3x 512 kbyte SRAM for MK 2

    +Drive: Hynix H26M21001CAR 2 Gbyte flash?

    The Coldfire memory map is software definable, and differs between the
    bootstrap code and the main operating system.

    The +Drive isn't mapped to memory at all. Instead it's accessed through
    Coldfire port A. It's just a large flash chip connected via a 74 series
    chip with XOR gates, but I don't understand exactly how communication
    works. The only one I've seen in a photo is 2 Gbytes, which is curiously
    large considering it stores 128 snapshots of the patch memory, and 128
    snapshots of the user waveform memory when applicable. Probably even 512
    Mbytes would be wasteful.

    Memory map for Coldfire when bootstrap code runs:
    0-100000: Lower 1 Mbyte of flash memory
    100000-200000: Patch memory

    Memory map for Coldfire when operating system runs:
    700000-800000: Patch memory
    10000000-10800000: Flash memory

    Common memory map for Coldfire:
    200000-300000: Main memory
    300000: Coldfire SIM (system integration module - peripherals, etc)
    500000-500008: DSP 1 Hi08 host registers in order (ICR, CVR, ISR, IVR,
    0/unused, RXH/TXH, RXM/TXM, RXL/TXL)
    600000-600008: DSP 2 Hi08
    1000000-1002000?: Coldfire internal SRAM

    Memory map for flash:
    0-100000: Firmware (identical to previous reconstructed dump)
    100000-200000: User waveforms for the factory presets on Machinedrum UW
    and Monomachine MK 2
    200000-800000: Probably user waveforms on units without +Drive. Both the
    units I dumped have factory installed +Drives, and the area is empty
    except for some markers. Machinedrum UW has a big lookup table near the
    end as well. Side note: I think there's code in the firmware to recreate
    markers and lookup tables, so the upper 6 Mbytes may be superfluous for
    emulation.

    Memory map for MK 1 DSPs:
    100000-140000: DRAM
    140000-150000: SRAM
    180000-200000: Additional DRAM for DSP 2 in Machinedrum UW only

    Memory map for MK 2 DSPs:
    100000-180000: SRAM
    180000-200000: Additional SRAM for DSP 2 in Machinedrum UW only

    The DSPs seem to use the ESSI serial interface to communicate, and to
    interface with the AKM AK4626AVQ audio codec chip. On Machinedrum, DSP 2
    does the main synthesis (that's why it has additional memory in UW
    models), and DSP 1 does effects. I haven't looked much at the
    Monomachine DSP code, but both chips share most of the code, so the work
    is probably shared in a more symmetrical manner than on the Machinedrum.

    Coldfire peripherals:
    - UART 1: MIDI
    - UART 2: Buttons, knobs, LEDs, display
    - Port A: Bit 0 mirrors bit 2 on MK 2, bit 1 checks SRAM battery charge,
    bit 2 is used for +Drive address, bit 3 is something about +Drive, bits
    4-7 is data RX/TX for +Drive.

    Model detection:
    - MK 2 is detected by writing to port A bit 2 and reading the same value
    from bit 0. Machinedrum also checks patch SRAM size.
    - Machinedrum UW is detected by comparing the ID of the flash chip to
    certain 8 Mbyte chip IDs, and by DSP 2 which checks available memory.
    - +Drive is detected by communicating with it on port A.
    - I haven't looked into how the Monomachine keyboard is detected, or how
    it's connected to the Coldfire.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/mcf5206e.h"
#include "machine/mcf5206e.h"
#include "machine/intelfsh.h"
#include "speaker.h"


namespace {

class elekmono_state : public driver_device
{
public:
	elekmono_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void elektron(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void elektron_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void elekmono_state::machine_start()
{
}

void elekmono_state::machine_reset()
{
}

void elekmono_state::elektron_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom().region("maincpu", 0);
	map(0x00100000, 0x001fffff).ram(); // patch memory
	map(0x00200000, 0x002fffff).ram(); // main RAM
	// 00300000 = Coldfire SIM
	// 00400000 = DSP1 Hi08 host registers (ICR, CVR, ISR, IVR, unused, RXH/TXH, RXM/TXM, RXL/TXL)
	// 00500000 = DSP2 Hi08 host registers (same as DSP1)
	map(0x01000000, 0x01001fff).ram();
	map(0x10000000, 0x107fffff).rom().region("maincpu", 0);
}

void elekmono_state::elektron(machine_config &config)
{
	MCF5206E(config, m_maincpu, XTAL(25'447'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &elekmono_state::elektron_map);

	SPEAKER(config, "speaker", 2).front();
}

static INPUT_PORTS_START( elektron )
INPUT_PORTS_END

ROM_START( monomach )
	ROM_REGION(0x800000, "maincpu", 0)
	ROM_LOAD( "elektron_sfx6-60_os1.32b.bin", 0x000000, 0x800000, CRC(f90a8b0e) SHA1(11a37460a5f47fd1a4d911414288690e6e7da605) )
ROM_END

ROM_START( machdrum )
	ROM_REGION(0x800000, "maincpu", 0)
	ROM_LOAD( "elektron_sps1-1uw_os1.63.bin", 0x000000, 0x800000, CRC(3d552c99) SHA1(a872a2f3527063673d6ea6d3080c4c62ef0cadc1) )
ROM_END

} // anonymous namespace


CONS( 2004, monomach, 0, 0, elektron, elektron, elekmono_state, empty_init, "Elektron", "Monomachine SFX6 MK2",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
CONS( 2007, machdrum, 0, 0, elektron, elektron, elekmono_state, empty_init, "Elektron", "Machinedrum SPS-1 MK2", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
