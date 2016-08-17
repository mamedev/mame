// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    esqmr.c - Ensoniq MR-61, MR-76, and MR-Rack

    Skeleton driver by R. Belmont

    Hardware:
        CPU: 68340 MCU
        Sound: 2xES5506
        Effects: ES5511

    Memory map:

    0x000000-0x0FFFFF   OS ROM

    MR Rack
    Ensoniq, 1995

    This is a 64-voice expandable synth module made by Ensoniq in 1995.
    It is shipped with 12MB of 16-bit 44.1kHz wave data and can be expanded
    with up to 3 ENSONIQ EXP Series Wave Expansion Boards containing up to a
    maximum of 84MB of additional wave data.


    PCB Layout
    ----------

    PN: 4001028501 REV C
    |--------------------------------------------------------|
    | J6  J7          J12  J13               J21   J23    J22|
    |                                                      J5|
    |       4565  4565                          HP_6N138     |
    |                                                        |
    |                                      BATTERY   7407    |
    |     4565                                               |
    |                                                        |
    |          OTTOR2  OTTOR2                                |
    |AD1861                                                  |
    |AD1861                                                  |
    |AD1861                                                  |
    |AD1861                                    |--J3-----|   |
    | HC04                                     |EXPANSION|   |
    |                                          |BOARD #3 |   |
    |                                          |         |   |
    |            1370001501                    |         |   |
    |                                          |         |   |
    |                                          |--J8-----|   |
    |     ESP2                         ROM2                  |
    |                                          |--J2-----|   |
    |                                          |EXPANSION|   |
    |                                  ROM0    |BOARD #2 |   |
    |                                          |         |   |
    | D43256               IDT7130             |         |   |
    | D43256  35MHz  16MHz             ROM1    |         |   |
    | D43256               J14                 |--J9-----|   |
    | D43256     22.5792MHz                            ADM691|
    | D43256             MC68340               |--J1-----|   |
    | D43256                                   |EXPANSION|   |
    |                    6MHz                  |BOARD #1 |   |
    |                          EPROM_UP        |         |   |
    |         MC68HC705C4A                KM681000       |   |
    |J4                        EPROM_LO   KM681000       |J11|
    |       J19        J18                     |--J10----|   |
    |--------------------------------------------------------|
    Notes:
          J4/J18/J19   - Connectors to front panel buttons, LCD etc
          J1/J10       - Connectors for expansion board #1
          J2/J9        - Connectors for expansion board #2
          J3/J8        - Connectors for expansion board #3
          J11          - Memory card connector
          J14          - JTAG connector
          J6           - Main left/mono jack
          J7           - Main right jack
          J12          - Aux left/mono jack
          J13          - Aux right jack
          J21          - MIDI in connector
          J23          - MIDI out connector
          J22          - MIDI thru connector
          J5           - Power input connector
          1370001501   - Unknown TQFP144 IC stamped with the Ensoniq logo. Possibly CPLD?
          ESP2         - Ensoniq ESP2 sound chip
          OTTOR2       - Ensoniq OTTOR2 sound chip
          ROM*         - 4M x8-bit SOP44 mask ROM
          D43256       - NEC D43256 32k x8-bit Static RAM
          HP_6N138     - HP/Agilent HP 6N138 Low Input Current High Gain Optocoupler
          4565         - JRC4565 Dual Operational Amplifier
          AD1861       - Analog Devices AD1861 16-bit/18-bit PCM Audio DAC
          MC68HC705C4A - Motorola MC68HC705C4A Microcontroller. Clock input is tied to the TQFP CPLD
          MC68340      - Motorola MC68340PV16E 68000-compatible 32-bit processor with on-board peripherals. Clock input 6.000MHz
          EPROM*       - 27C4001 EPROM
          IDT7130      - IDT7130 High Speed 1k x8-bit Dual Port Static RAM
          KM681000     - Samsung KM681000 128k x8-bit Static RAM
          ADM691       - Analog Devices ADM691 Microprocessor Supervisory Circuit with Automatic Battery Backup Switching


    Additional notes from the manual:
    ---------------------------------

    The MR-Rack can play special demonstration songs to give you an idea of how terrific it sounds.

    To Play the MR-Rack Main Demo
    1. Press the Audition button, and hold it down.
    2. While still holding Audition, press the Save button.
    3. Let go of both buttons.

    In an unexpanded MR-Rack, the display shows:

    Hit ENTER to Play:
    MAINDEMO:MR Internal

    If you've installed any Expansion boards or a ROM card containing MAINDEMO-type
    demonstration songs, your display will differ. Turn the Value knob counter-clockwise
    until the display looks as it does above.

    Note: When MR-Rack demos are being viewed or playing, MIDI In is disabled.

    4. Press Enter to play the demo.
    5. Press Enter again to stop the demo.
    6. When you're done listening to the demo song, press Exit to return to normal MR-Rack


    The Version Number of Your MR-Rack Operating System:
    You can easily find out what operating system (or O.S.) your MR-Rack is currently using.
    To Find the Operating System:
    1. Press the Save button and hold it down.
    2. While still holding the Save button, press the System button.
    The display briefly shows your current Operating System:

    ENSONIQ MR-RACK
    O.S. Version: #.##



    ENSONIQ EXP Series Wave Expansion Boards
    ----------------------------------------

    These are small plug-in boards containing a 256k x8-bit EPROM (27C020) and from 1 to 6
    SOP44 mask ROMs. These add additional digital sound waves. These can be used with the MR
    Rack, the full-size MR Keyboards and a few other models.

    PCB Layout
    ----------

    PN: 4001033401 REV B
    |------------------|
    |                  |
    |    U6       U7   |
    |-                 |
    ||                 |
    ||            U1   |
    ||    U2          -|
    ||                ||
    ||                ||
    ||    U3          ||
    ||                ||
    ||             U8 ||
    ||    U4          ||
    ||                ||
    ||                ||
    ||    U5          ||
    |-                -|
    |------------------|
    Notes:
          U1       - AC138 logic chip
          U2 to U7 - 4M x 8-bit or 2M x 8-bit SOP44 mask ROM
          U8       - 27C020 EPROM

    The EXP boards dumped so far are....

                             MROM  # of
    Name            Version  Size  MROMs
    -----------------------------------
    Piano           V1.00    4M    4 (multisampled pianos, aka "The Perfect Piano")
    The Real World  V1.01    4M    6 (non-Western percusson and other "world music" sounds)
    Drum            V1.00    2M    1 (additional drum kits that are built in to the MR keyboards)


    Additional notes from the manual:
    ---------------------------------

    The MR-Rack provides three displays which can identify any EXP Wave Expansion boards
    you have installed.

    To Identify an Installed Expansion Board
    1. Press the System button.
    2. Turn the Parameter knob until the display shows:

    System parameters:
    WaveEXP1:xxxxxxxxxxx

    When an Expansion Board is installed, this read-only display will show the name of the
    Expansion Board located in the first slot.

    3. Turning the Parameter knob two more times will reveal two more displays which show
    the names of the Expansion Boards in Wave EXP Slots 2 and 3 (if they're installed).
    If there are no Expansion Boards installed, the display will show "WaveEXP1= **EMPTY**."

***************************************************************************/

#include "emu.h"
#include "machine/68340.h"
#include "sound/es5506.h"

#include "machine/esqvfd.h"

class esqmr_state : public driver_device
{
public:
	esqmr_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sq1vfd(*this, "sq1vfd")
	{ }

	required_device<m68340cpu_device> m_maincpu;
	required_device<esq2x40_sq1_t> m_sq1vfd;

	virtual void machine_reset() override;

public:
	DECLARE_DRIVER_INIT(mr);
	DECLARE_WRITE_LINE_MEMBER(esq5506_otto_irq);
	DECLARE_READ16_MEMBER(esq5506_read_adc);
};

void esqmr_state::machine_reset()
{
}

static ADDRESS_MAP_START( mr_map, AS_PROGRAM, 32, esqmr_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM AM_REGION("maincpu", 0)
//  AM_RANGE(0x200000, 0x20003f) AM_DEVREADWRITE8("ensoniq", es5506_device, read, write, 0xffffffff)
//  AM_RANGE(0x240000, 0x24003f) AM_DEVREADWRITE8("ensoniq2", es5506_device, read, write, 0xffffffff)
//    AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("osram")
ADDRESS_MAP_END

WRITE_LINE_MEMBER(esqmr_state::esq5506_otto_irq)
{
}

READ16_MEMBER(esqmr_state::esq5506_read_adc)
{
	return 0;
}

static MACHINE_CONFIG_START( mr, esqmr_state )
	MCFG_CPU_ADD("maincpu", M68340, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(mr_map)

	MCFG_ESQ2x40_SQ1_ADD("sq1vfd")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5506, XTAL_16MHz)
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(1)          /* channels */
	MCFG_ES5506_IRQ_CB(WRITELINE(esqmr_state, esq5506_otto_irq)) /* irq */
	MCFG_ES5506_READ_PORT_CB(READ16(esqmr_state, esq5506_read_adc))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)
	MCFG_SOUND_ADD("ensoniq2", ES5506, XTAL_16MHz)
	MCFG_ES5506_REGION0("waverom")  /* Bank 0 */
	MCFG_ES5506_REGION1("waverom2") /* Bank 1 */
	MCFG_ES5506_REGION2("waverom3") /* Bank 0 */
	MCFG_ES5506_REGION3("waverom4") /* Bank 1 */
	MCFG_ES5506_CHANNELS(1)          /* channels */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)
MACHINE_CONFIG_END

static INPUT_PORTS_START( mr )
INPUT_PORTS_END

ROM_START( mr61 )
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "mrw-osf-11af-2.10.bin",  0x000000, 0x080000, CRC(5854314e) SHA1(8fb2e2ee2f5fb12eae8ea33cb18f757efaec6780) )
	ROM_LOAD16_WORD_SWAP( "mrw-romc-32ef-1.20.bin", 0x080000, 0x080000, CRC(68321347) SHA1(56cb96943ba42c35ba2787a49b5f4adf7c8dffb8) )

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)

	ROM_REGION(0x400000, "waverom2", ROMREGION_ERASE00)

	ROM_REGION(0x400000, "waverom3", ROMREGION_ERASE00)

	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

ROM_START( mrrack )
	// 68340 main MCU
	ROM_REGION(0x100000, "maincpu", 0)
	ROM_LOAD16_BYTE( "mr_r_ec51_lo_1.50.u36", 0x000001, 0x080000, CRC(b29988a1) SHA1(986c2def11de27fa2b9be55ac32f7fec0c414bca) )
	ROM_LOAD16_BYTE( "mr_r_9dac_up_1.50.u35", 0x000000, 0x080000, CRC(71511692) SHA1(54744f16f1db1ac5abb2f70b6e04aebf1e0e029d) )

	// 68705 display/front panel MCU
	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD( "68hc705.u40",  0x000000, 0x002000, CRC(7b0291a7) SHA1(c92c19ce9289b7b21dbc915475cdff8930e3c677) )

	ROM_REGION(0x400000, "waverom", ROMREGION_ERASE00)
	ROM_LOAD( "1351000901_h-rom0.u5", 0x000000, 0x400000, CRC(89654b42) SHA1(4bdffd8060eb20cdb01f6178222aeb32fdbfd703) )

	ROM_REGION(0x400000, "waverom2", ROMREGION_ERASE00)
	ROM_LOAD( "1351000902_h-rom1.u23", 0x000000, 0x400000, CRC(4a19e517) SHA1(e819f1e0b50c4911c4855ad95ed505998a2bbe86) )

	ROM_REGION(0x400000, "waverom3", ROMREGION_ERASE00)
	ROM_LOAD( "1351000903_h-rom2.u24", 0x000000, 0x400000, CRC(c9ab1214) SHA1(92f48b068bbe49eacbffd03e428599e3ab21b8ec) )

	ROM_REGION(0x200000, "waverom4", ROMREGION_ERASE00)
ROM_END

DRIVER_INIT_MEMBER(esqmr_state, mr)
{
}

CONS( 1996, mr61,   0, 0, mr, mr, esqmr_state, mr, "Ensoniq", "MR-61 Workstation", MACHINE_NOT_WORKING )
CONS( 1996, mrrack, 0, 0, mr, mr, esqmr_state, mr, "Ensoniq", "MR-Rack", MACHINE_NOT_WORKING )
