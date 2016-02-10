// license:BSD-3-Clause
// copyright-holders:Bryan McPhail,Ernesto Corvi,Andrew Prime,Zsolt Vasvari
// thanks-to:Fuzz
/***************************************************************************

    Neo-Geo hardware

    Credits:
        * This driver was made possible by the research done by
          Charles MacDonald.  For a detailed description of the Neo-Geo
          hardware, please visit his page at:
          http://cgfm2.emuviews.com/txt/mvstech.txt
        * Presented to you by the Shin Emu Keikaku team.
        * The following people have all spent probably far
          too much time on this:
          AVDB
          Bryan McPhail
          Fuzz
          Ernesto Corvi
          Andrew Prime
          Zsolt Vasvari


    Known driver issues/to-do's:
    ============================

        * Fatal Fury 3 crashes during the ending - this doesn't occur if
          the language is set to Japanese, maybe the English endings
          are incomplete / buggy?
        * Graphical Glitches caused by incorrect timing?
          - Some raster effects are imperfect (off by a couple of lines)
        * Multi-cart support not implemented - the MVS can take up to
          6 cartridges depending on the board being used
        * 68000 waitstates on ROM region access, determined by jumpers on cart
          (garou train stage 3 background bug is probably related to this)


    Confirmed non-bugs:

        * Bad zooming in the Kof2003 bootlegs - this is what happens
          if you try and use the normal bios with a pcb set, it
          looks like the bootleggers didn't care.
        * Glitches at the edges of the screen - the real hardware
          can display 320x224 but most of the games seem designed
          to work with a width of 304, some less.
        * Distorted jumping sound in Nightmare in the Dark
        * Ninja Combat sometimes glitches


*****************************************************************************

    The Neo-Geo Multi Video System (MVS), is an arcade system board, being
    the first product in the Neo-Geo family, designed by Alpha Denshi(ADK)
    and released in 1990 by SNK. It was known to the coin-op industry, and
    offered arcade operators the ability to put up to 6 different arcade
    titles into a single cabinet, a key economic consideration for operators
    with limited floorspace (games for the Neo-Geo are cartridge based and are
    easily exchangeable). It comes in many different cabinets but basically
    consists of an add on board that can be linked to a standard Jamma system.
    The system was discontinued in 2004.
    Source (modified): http://en.wikipedia.org/wiki/Neo_Geo


    MVS motherboards were produced in 1 / 2 / 4 and 6 Slot versions.

    Known motherboards:
    ===================

    1 Slot:
    NEO-MVH MV1
    NEO-MVH MV1-1
    NEO-MVH MV1A
     . NEO-MVH MV1A CHX ??
    NEO-MVH MV1B (1996.1.19)
     . NEO-MVH MV1B CHX (1996.1.19) ??
    NEO-MVH MV1B1 (1998.6.17)
    NEO-MVH MV1C (1999.4.30)
    NEO-MVH MV1F
    NEO-MVH MV1FS
    NEO-MVH MV1FT
    NEO-MVH MV1FZ
    NEO-MVH MV1FZS

    2 Slot:
    NEO-MVH MV2
    NEO-MVH MV2F
    NEO-MVH MV2F-01

    4 Slot:
    NEO-MVH MV4
    NEO-MVH MV4F
    NEO-MVH MV4FS
    NEO-MVH MV4FT
    NEO-MVH MV4FT2

    6 Slot:
    NEO-MVH MV6
    NEO-MVH MV6F


    Neo-Geo Motherboard (info - courtesy of Guru):

          NEO-MVH MV1
          |---------------------------------------------------------------------|
          |       4558                                                          |
          |                                          HC04  HC32                 |
          |                      SP-S2.SP1  NEO-E0   000-L0.L0   LS244  AS04    |
          |             YM2610                                                  |
          | 4558                                                                |
          |       4558                        5814  HC259   SFIX.SFIX           |
          |                                                             NEO-I0  |
          | HA13001 YM3016                    5814                              |
          --|                                                                   |
            |     4558                                                          |
          --|                                                 SM1.SM1   LS32    |
          |                                                                     |
          |                           LSPC-A0         PRO-C0            LS244   |
          |                                                                     |
          |J              68000                                                 |
          |A                                                                    |
          |M                                                                    |
          |M                                                      NEO-ZMC2      |
          |A                                                                    |
          |   LS273  NEO-G0                          58256  58256     Z80A      |
          |                           58256  58256   58256  58256     6116      |
          |   LS273 5864                                                        |
          --| LS05  5864  PRO-B0                                                |
            |                                                                   |
          --|             LS06   HC32           D4990A    NEO-F0   24.000MHz    |
          |                      DSW1    BATT3.6V 32.768kHz       NEO-D0        |
          |                                           2003  2003                |
          |---------------------------------------------------------------------|


*****************************************************************************

    Neo-Geo game PCB infos:
    =======================

    The Neo-Geo games for AES (home) and MVS (arcade) systems are cartridge based.

    Each cartridge consists of two PCBs: CHA and PROG.
    .CHA PCB contains gfx data ('C' - rom), text layer data ('S' - rom) and sound driver ('M' - rom).
    .PROG PCB contains sample data ('V' - rom) and program code ('P' - rom).

    On most PCBs various custom/protection chips can also be found:
    (Custom chip detail information (modified) from: http://wiki.neogeodev.org)

    CHA:
    . NEO-273  (C and S-ROM address latch)
    . NEO-CMC 90G06CF7042 (NEO-273 logic / NEO-ZMC logic / C-ROM decryption / C and S-ROM multiplexer / S-ROM bankswitching)
    . NEO-CMC 90G06CF7050 (NEO-273 logic / NEO-ZMC logic / C-ROM decryption / M-ROM decryption / C and S-ROM multiplexer / S-ROM bankswitching)
    . NEO-ZMC  (Z80 memory controller)
    . NEO-ZMC2 (Z80 memory controller / Tile serializer)
    . PRO-CT0  (C-ROM serializer and multiplexer?; used on early AES-CHA boards)
    . SNK-9201 (C-ROM serializer and multiplexer?; used on early AES-CHA boards)

    PROG:
    . 0103 (QFP144) (Only found on Metal Slug X NEO-MVS PROGEOP board; function unknown)
    . ALTERA   (EPM7128SQC100-15) (P-ROM protection chip used for KOF98 NEO-MVS PROGSF1 board and Metal Slug X NEO-MVS PROGEOP board)
    . NEO-COMA (Microcontroller; used for MULTI PLAY MODE, boards and sets see below)
    . NEO-PCM2 (SNK 1999) (PCM functionality / V-ROM decryption / P-ROM decoding and bankswitching)
    . NEO-PCM2 (PLAYMORE 2002) (PCM functionality / V-ROM decryption / P-ROM decoding and bankswitching)
    . NEO-PVC  (P-ROM decryption and bankswitching) / RAM
    . NEO-SMA  (P-ROM decryption and bankswitching / RNG / Storage of 256kb game data)
    . PCM      (ADPCM bus latches / V-ROM multiplexer)
    . PRO-CT0  (On PROG board used for P-ROM protection -> Fatal Fury 2)
    . SNK-9201 (On PROG board used for P-ROM protection -> Fatal Fury 2)



    Known PCBs:
    ============

    MVS CHA:
    -- SNK --
    . NEO-MVS CHA-32
    . NEO-MVS CHA-8M
    . NEO-MVS CHA42G
    . NEO-MVS CHA42G-1
    . NEO-MVS CHA 42G-2
    . NEO-MVS CHA 42G-3
    . NEO-MVS CHA42G-3B
    . NEO-MVS CHA256
    . NEO-MVS CHA256B
    . NEO-MVS CHA512Y
    . NEO-MVS CHAFIO (1999.6.14) - used with NEO-CMC 90G06C7042 or NEO-CMC 90G06C7050
    . MVS CHAFIO REV1.0 (KOF-2001)
    . NEO-MVS CHAFIO (SNK 2002) - MADE IN KOREA
    -- SNKPLAYMORE --
    . NEO-MVS CHAFIO (2003.7.24) - used only with NEO-CMC 90G06C7050

    -- SNK development boards --
    . NEO-MVS CHAMC2

    MVS PROG:
    -- SNK --
    . NEO-MVS PROG-NAM
    . NEO-MVS PROG-HERO
    . NEO-MVS PROG-EP
    . NEO-MVS PROG-8MB
    . NEO-MVS PROGEP8M
    . NEO-MVS PROG8M42
    . NEO-MVS PROG16
    . NEO-MVS PROG42G
    . NEO-MVS PROG42G-COM
    . NEO-MVS PROG42G-1
    . NEO-MVS PROG-G2
    . NEO-MVS PROG 4096
    . NEO-MVS PROG 4096 B
    . NEO-MVS PROGGSC
    . NEO-MVS PROGSM
    . NEO-MVS PROGSS3
    . NEO-MVS PROGTOP
    . NEO-MVS PROGSF1 (1998.6.17)
    . NEO-MVS PROGSF1E (1998.6.18)
    . NEO-MVS PROGEOP (1999.2.2)
    . NEO-MVS PROGLBA (1999.4.12) - LBA-SUB (2000.2.24)
    . NEO-MVS PROGBK1 (1994)
    . NEO-MVS PROGBK1 (2001)
    . NEO-MVS PROGBK2 (2000.3.21) - used with NEO-PCM2 (1999 SNK) or NEO-PCM2 (2002 PLAYMORE)
    . MVS PROGBK2 REV1.0 (KOF-2001)
    . NEO-MVS PROGBK2 (SNK 2002) - MADE IN KOREA
    -- SNKPLAYMORE --
    . NEO-MVS PROGBK2R (2003.8.26) - NEO-HYCS (2003.9.29)
    . NEO-MVS PROGBK3R (2003.9.2) - NEO-HYCS (2003.9.29)
    . NEO-MVS PROGBK3S (2003.10.1)
    . NEO-MVS PROGBK2S (2003.10.18)

    -- SNK development boards --
    . NEO-MVS PROGMC2


    AES CHA:
    -- SNK --
    . NEO-AEG CHA-32
    . NEO-AEG CHA-8M
    . NEO-AEG CHA42G
    . NEO-AEG CHA42G-1
    . NEO-AEG CHA42G-2B
    . NEO-AEG CHA42G-3
    . NEO-AEG CHA42G-4
    . NEO-AEG CHA256
    . NEO-AEG CHA256 B
    . NEO-AEG CHA256[B]
    . NEO-AEG CHA256BY
    . NEO-AEG CHA256RY
    . NEO-AEG CHA512Y
    . NEO-AEG CHAFIO (1999.8.10) - used with NEO-CMC 90G06C7042 or NEO-CMC 90G06C7050
    -- SNKPLAYMORE --
    . NEO-AEG CHAFIO (2003.7.24) - used only with NEO-CMC 90G06C7050

    AES PROG:
    -- SNK --
    . NEO-AEG PROG-NAM
    . NEO-AEG PROG-HERO
    . NEO-AEG PROG-4A
    . NEO-AEG PROG-4B
    . NEO-AEG PROG 8M42
    . NEO-AEG PROG B
    . NEO-AEG PROG16
    . NEO-AEG PROG42G
    . NEO-AEG PROG42G-COM
    . NEO-AEG PROG42G-1
    . NEO-AEG PROG-G2
    . NEO-AEG PROG4096 B
    . NEO-AEG PROGGS
    . NEO-AEG PROGTOP2
    . NEO-AEG PROGTOP2Y
    . NEO-AEG PROGEOP (1999.4.2)
    . NEO-AEG PROGLBA (1999.7.6)
    . NEO-AEG PROGRK
    . NEO-AEG PROGRKB
    . NEO-AEG PROGBK1Y
    . NEO-AEG PROGBK1F
    -- PLAYMORE --
    . NEO-AEG PROGBK2 (2002.4.1) - used with NEO-PCM2 (1999 SNK) or NEO-PCM2 (2002 PLAYMORE)
    -- SNKPLAYMORE --
    . NEO-AEG PROGBK3R (2003.8.29) - NEO-HYCS (2003.9.29)
    . NEO-AEG PROGBK3S (2003.10.6)
    . NEO-AEG PROGBK2S (2003.10.16)



    Cartridge colours:
    ==================

    MVS cartridges were produced in different colours.

    Known cartridge colours:
    . Black
    . Blue
    . Green
    . Grey
    . Red
    . Transparent
    . Transparent Blue
    . Transparent Green
    . White
    . Yellow

    The above listed only covers SNK / PLAYMORE / SNKPLAYMORE PCBs. There also exists a
    wide range of 'bootleg' PCBs.


    Unofficial pcb's from NG:DEV.TEAM:

    MVS CHA:
    GIGA CHAR Board 1.0 Rev. A
    GIGA CHAR Board 1.5 Rev. 0
    GIGA CHAR Board 1.5 Rev. C

    MVS PROG:
    GIGA PROG Board 1.0 Rev. B
    GIGA PROG Board 1.5 Rev. A
    GIGA PROG Board 1.5 Rev. C


    Unofficial pcb's from NEOBITZ:

    MVS CHA:
    CHARBITZ1 2013.12.01

    MVS PROG:
    PROGBITZ1 2013.12.01


    Neo-Geo game PCB infos by Johnboy



    MVS cart pinout:
    ================

    Kindly submitted by Apollo69 (apollo69@columbus.rr.com)
    =================================================================
                CTRG1                            CTRG2
    =================================================================
         GND = 01A | 01B = GND            GND = 01A | 01B = GND
         GND = 02A | 02B = GND            GND = 02A | 02B = GND
          P0 = 03A | 03B = P1             GND = 03A | 03B = GND
          P2 = 04A | 04B = P3             GND = 04A | 04B = GND
          P4 = 05A | 05B = P5              D0 = 05A | 05B = A1
          P6 = 06A | 06B = P7              D1 = 06A | 06B = A2
          P8 = 07A | 07B = P9              D2 = 07A | 07B = A3
         P10 = 08A | 08B = P11             D3 = 08A | 08B = A4
         P12 = 09A | 09B = P13             D4 = 09A | 09B = A5
         P14 = 10A | 10B = P15             D5 = 10A | 10B = A6
         P16 = 11A | 11B = P17             D6 = 11A | 11B = A7
         P18 = 12A | 12B = P19             D7 = 12A | 12B = A8
         P20 = 13A | 13B = P21             D8 = 13A | 13B = A9
         P22 = 14A | 14B = P23             D9 = 14A | 14B = A10
       PCK1B = 15A | 15B = 24M            D10 = 15A | 15B = A11
       PCK2B = 16A | 16B = 12M            D11 = 16A | 16B = A12
         2H1 = 17A | 17B = 8M             D12 = 17A | 17B = A13
         CA4 = 18A | 18B = RESET          D13 = 18A | 18B = A14
         CR0 = 19A | 19B = CR1            D14 = 19A | 19B = A15
         CR2 = 20A | 20B = CR3            D15 = 20A | 20B = A16
         CR4 = 21A | 21B = CR5            R/W = 21A | 21B = A17
         CR6 = 22A | 22B = CR7             AS = 22A | 22B = A18
         CR8 = 23A | 23B = CR9         ROMOEU = 23A | 23B = A19
        CR10 = 24A | 24B = CR11        ROMOEL = 24A | 24B = 68KCLKB
        CR12 = 25A | 25B = CR13       PORTOEU = 25A | 25B = ROMWAIT
        CR14 = 26A | 26B = CR15       PORTOEL = 26A | 26B = PWAIT0
        CR16 = 27A | 27B = CR17       PORTWEU = 27A | 27B = PWAIT1
        CR18 = 28A | 28B = CR19       PORTWEL = 28A | 28B = PDTACT
         VCC = 29A | 29B = VCC            VCC = 29A | 29B = VCC
         VCC = 30A | 30B = VCC            VCC = 30A | 30B = VCC
         VCC = 31A | 31B = VCC            VCC = 31A | 31B = VCC
         VCC = 32A | 32B = VCC            VCC = 32A | 32B = VCC
        CR20 = 33A | 33B = CR21      PORTADRS = 33A | 33B = 4MB
        CR22 = 34A | 34B = CR23            NC = 34A | 34B = ROMOE
        CR24 = 35A | 35B = CR25            NC = 35A | 35B = RESET
        CR26 = 36A | 36B = CR27            NC = 36A | 36B = NC
        CR28 = 37A | 37B = CR29            NC = 37A | 37B = NC
        CR30 = 38A | 38B = CR31            NC = 38A | 38B = NC
          NC = 39A | 39B = FIX00           NC = 39A | 39B = NC
          NC = 40A | 40B = FIX01           NC = 40A | 40B = NC
          NC = 41A | 41B = FIX02           NC = 41A | 41B = SDPAD0
     SYSTEMB = 42A | 42B = FIX03      SYSTEMB = 42A | 42B = SDPAD1
        SDA0 = 43A | 43B = FIX04        SDPA8 = 43A | 43B = SDPAD2
        SDA1 = 44A | 44B = FIX05        SDPA9 = 44A | 44B = SDPAD3
        SDA2 = 45A | 45B = FIX06       SDPA10 = 45A | 45B = SDPAD4
        SDA3 = 46A | 46B = FIX07       SDPA11 = 46A | 46B = SDPAD5
        SDA4 = 47A | 47B = SDRD0       SDPMPX = 47A | 47B = SDPAD6
        SDA5 = 48A | 48B = SDRD1        SDPOE = 48A | 48B = SDPAD7
        SDA6 = 49A | 49B = SDROM        SDRA8 = 49A | 49B = SDRA00
        SDA7 = 50A | 50B = SDMRD        SDRA9 = 50A | 50B = SDRA01
        SDA8 = 51A | 51B = SDDO        SDRA20 = 51A | 51B = SDRA02
        SDA9 = 52A | 52B = SDD1        SDRA21 = 52A | 52B = SDRA03
       SDA10 = 53A | 53B = SDD2        SDRA22 = 53A | 53B = SDRA04
       SDA11 = 54A | 54B = SDD3        SDRA23 = 54A | 54B = SDRA05
       SDA12 = 55A | 55B = SDD4        SDRMPX = 55A | 55B = SDRA06
       SDA13 = 56A | 56B = SDD5         SDROE = 56A | 56B = SDRA07
       SDA14 = 57A | 57B = SDD6           GND = 57A | 57B = GND
       SDA15 = 58A | 58B = SDD7           GND = 58A | 58B = GND
         GND = 59A | 59B = GND            GND = 59A | 59B = GND
         GND = 60A | 60B = GND            GND = 60A | 60B = GND

    CTRG1 (CHA)  = Contains gfx data ('C' - rom), text layer data ('S' - rom) and sound driver ('M' - rom)
    CTRG2 (PROG) = Contains sample data ('V' - rom) and program code ('P' - rom)

    NOTE: On CTRG2-B, The "A" lines start at "A1". If you trace this on an
    actual cart, you will see that this is actually "A0" (A0 - A18).

    These are from a very hard to read copy of the schematics, so
    I hope that I got the pin names correct.

    Apollo69 10/19/99


*****************************************************************************

    Watchdog:
    =========

    The watchdog timer will reset the system after ~0.13 seconds.
    By cgfm's research, exactly 3,244,030 cycles (based on 24MHz clock).

    Newer games force a reset using the following code (this from kof99):
        009CDA  203C 0003 0D40             MOVE.L   #0x30D40,D0
        009CE0  5380                       SUBQ.L   #1,D0
        009CE2  64FC                       BCC.S    *-0x2 [0x9CE0]
    Note however that there is a valid code path after this loop.

    The watchdog is used as a form of protection on a number of games,
    previously this was implemented as a specific hack which locked a single
    address of SRAM.

    What actually happens is if the game doesn't find valid data in the
    backup ram it will initialize it, then sit in a loop.  The watchdog
    should then reset the system while it is in this loop.  If the watchdog
    fails to reset the system the code will continue and set a value in
    backup ram to indiate that the protection check has failed.


    Mahjong Panel notes (2009-03 FP):
    =================================

    * In Service Mode menu with mahjong panel active, controls are as
      follows:

        A = select / up (for options)
        B = down (for options)
        C = go to previous menu
        E = up (for menu entries)
        F = down (for menu entries)
        G = left (for options)
        H = right (for options)

    * These only work with Japanese BIOS, but I think it's not a bug: I
      doubt other BIOS were programmed to be compatible with mahjong panels

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/neogeo.h"
#include "machine/nvram.h"
#include "cpu/z80/z80.h"
#include "sound/2610intf.h"
#include "softlist.h"
#include "neogeo.lh"


#define LOG_VIDEO_SYSTEM         (0)
#define LOG_MAIN_CPU_BANKING     (0)
#define LOG_AUDIO_CPU_BANKING    (0)


/*************************************
 *
 *  Main CPU interrupt generation
 *
 *************************************/



// The display counter is automatically reloaded with the load register contents on scanline 224,
// 1146 mclks from the rising edge of /HSYNC.
#define NEOGEO_VBLANK_RELOAD_HTIM (attotime::from_ticks(1146, NEOGEO_MASTER_CLOCK))

#define IRQ2CTRL_ENABLE             (0x10)
#define IRQ2CTRL_LOAD_RELATIVE      (0x20)
#define IRQ2CTRL_AUTOLOAD_VBLANK    (0x40)
#define IRQ2CTRL_AUTOLOAD_REPEAT    (0x80)


void neogeo_state::adjust_display_position_interrupt_timer()
{
	attotime period = attotime::from_ticks((UINT64)m_display_counter + 1, NEOGEO_PIXEL_CLOCK);
	if (LOG_VIDEO_SYSTEM) logerror("adjust_display_position_interrupt_timer  current y: %02x  current x: %02x   target y: %x  target x: %x\n", m_screen->vpos(), m_screen->hpos(), (m_display_counter + 1) / NEOGEO_HTOTAL, (m_display_counter + 1) % NEOGEO_HTOTAL);

	m_display_position_interrupt_timer->adjust(period);
}


void neogeo_state::neogeo_set_display_position_interrupt_control( UINT16 data )
{
	m_display_position_interrupt_control = data;
}


void neogeo_state::neogeo_set_display_counter_msb( UINT16 data )
{
	m_display_counter = (m_display_counter & 0x0000ffff) | ((UINT32)data << 16);

	if (LOG_VIDEO_SYSTEM) logerror("PC %06x: set_display_counter %08x\n", m_maincpu->pc(), m_display_counter);
}


void neogeo_state::neogeo_set_display_counter_lsb( UINT16 data )
{
	m_display_counter = (m_display_counter & 0xffff0000) | data;

	if (LOG_VIDEO_SYSTEM) logerror("PC %06x: set_display_counter %08x\n", m_maincpu->pc(), m_display_counter);

	if (m_display_position_interrupt_control & IRQ2CTRL_LOAD_RELATIVE)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_RELATIVE ");
		adjust_display_position_interrupt_timer();
	}
}


void neogeo_state::update_interrupts()
{
	m_maincpu->set_input_line(3, m_irq3_pending ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(m_raster_level, m_display_position_interrupt_pending ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(m_vblank_level, m_vblank_interrupt_pending ? ASSERT_LINE : CLEAR_LINE);
}


void neogeo_state::neogeo_acknowledge_interrupt( UINT16 data )
{
	if (data & 0x01)
		m_irq3_pending = 0;
	if (data & 0x02)
		m_display_position_interrupt_pending = 0;
	if (data & 0x04)
		m_vblank_interrupt_pending = 0;

	update_interrupts();
}


TIMER_CALLBACK_MEMBER(neogeo_state::display_position_interrupt_callback)
{
	if (LOG_VIDEO_SYSTEM) logerror("--- Scanline @ %d,%d\n", m_screen->vpos(), m_screen->hpos());

	if (m_display_position_interrupt_control & IRQ2CTRL_ENABLE)
	{
		if (LOG_VIDEO_SYSTEM) logerror("*** Scanline interrupt (IRQ2) ***  y: %02x  x: %02x\n", m_screen->vpos(), m_screen->hpos());
		m_display_position_interrupt_pending = 1;

		update_interrupts();
	}

	if (m_display_position_interrupt_control & IRQ2CTRL_AUTOLOAD_REPEAT)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_REPEAT ");
		adjust_display_position_interrupt_timer();
	}
}


TIMER_CALLBACK_MEMBER(neogeo_state::display_position_vblank_callback)
{
	if (m_display_position_interrupt_control & IRQ2CTRL_AUTOLOAD_VBLANK)
	{
		if (LOG_VIDEO_SYSTEM) logerror("AUTOLOAD_VBLANK ");
		adjust_display_position_interrupt_timer();
	}

	/* set timer for next screen */
	m_display_position_vblank_timer->adjust(m_screen->time_until_pos(NEOGEO_VBSTART) + NEOGEO_VBLANK_RELOAD_HTIM);
}


TIMER_CALLBACK_MEMBER(neogeo_state::vblank_interrupt_callback)
{
	if (LOG_VIDEO_SYSTEM) logerror("+++ VBLANK @ %d,%d\n", m_screen->vpos(), m_screen->hpos());

	m_vblank_interrupt_pending = 1;
	update_interrupts();

	/* set timer for next screen */
	m_vblank_interrupt_timer->adjust(m_screen->time_until_pos(NEOGEO_VBSTART) + NEOGEO_VBLANK_IRQ_HTIM);
}


void neogeo_state::create_interrupt_timers()
{
	m_display_position_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(neogeo_state::display_position_interrupt_callback),this));
	m_display_position_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(neogeo_state::display_position_vblank_callback),this));
	m_vblank_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(neogeo_state::vblank_interrupt_callback),this));
}


void neogeo_state::start_interrupt_timers()
{
	m_vblank_interrupt_timer->adjust(m_screen->time_until_pos(NEOGEO_VBSTART) + NEOGEO_VBLANK_IRQ_HTIM);
	m_display_position_vblank_timer->adjust(m_screen->time_until_pos(NEOGEO_VBSTART) + NEOGEO_VBLANK_RELOAD_HTIM);
}



/*************************************
 *
 *  Audio CPU interrupt generation
 *
 *************************************/

void neogeo_state::audio_cpu_check_nmi()
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, (m_audio_cpu_nmi_enabled && m_audio_cpu_nmi_pending) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(neogeo_state::audio_cpu_enable_nmi_w)
{
	// out ($08) enables the nmi, out ($18) disables it
	m_audio_cpu_nmi_enabled = !(offset & 0x10);
	audio_cpu_check_nmi();
}



/*************************************
 *
 *  Input ports / Controllers
 *
 *************************************/

void neogeo_state::select_controller( UINT8 data )
{
	m_controller_select = data;
}


CUSTOM_INPUT_MEMBER(neogeo_state::multiplexed_controller_r)
{
	int port = (FPTR)param;

	static const char *const cntrl[2][2] =
	{
		{ "IN0-0", "IN0-1" }, { "IN1-0", "IN1-1" }
	};

	return read_safe(ioport(cntrl[port][m_controller_select & 0x01]), 0x00);
}

CUSTOM_INPUT_MEMBER(neogeo_state::kizuna4p_controller_r)
{
	int port = (FPTR)param;

	static const char *const cntrl[2][2] =
	{
		{ "IN0-0", "IN0-1" }, { "IN1-0", "IN1-1" }
	};

	int ret = read_safe(ioport(cntrl[port][m_controller_select & 0x01]), 0x00);
	if (m_controller_select & 0x04) ret &= ((m_controller_select & 0x01) ? ~0x20 : ~0x10);

	return ret;
}

CUSTOM_INPUT_MEMBER(neogeo_state::kizuna4p_start_r)
{
	return (ioport("START")->read() >> (m_controller_select & 0x01)) | ~0x05;
}

#if 1 // this needs to be added dynamically somehow
CUSTOM_INPUT_MEMBER(neogeo_state::mahjong_controller_r)
{
	UINT32 ret;

/*
cpu #0 (PC=00C18B9A): unmapped memory word write to 00380000 = 0012 & 00FF
cpu #0 (PC=00C18BB6): unmapped memory word write to 00380000 = 001B & 00FF
cpu #0 (PC=00C18D54): unmapped memory word write to 00380000 = 0024 & 00FF
cpu #0 (PC=00C18D6C): unmapped memory word write to 00380000 = 0009 & 00FF
cpu #0 (PC=00C18C40): unmapped memory word write to 00380000 = 0000 & 00FF
*/
	switch (m_controller_select)
	{
	default:
	case 0x00: ret = 0x0000; break; /* nothing? */
	case 0x09: ret = ioport("MAHJONG1")->read(); break;
	case 0x12: ret = ioport("MAHJONG2")->read(); break;
	case 0x1b: ret = ioport("MAHJONG3")->read(); break; /* player 1 normal inputs? */
	case 0x24: ret = ioport("MAHJONG4")->read(); break;
	}

	return ret;
}
#endif

WRITE8_MEMBER(neogeo_state::io_control_w)
{
	switch (offset)
	{
		case 0x00:
			select_controller(data);
			break;

		case 0x10:
			if (m_type == NEOGEO_MVS)
				if (m_cartslots[0]) set_slot_number(data);
			break;

		case 0x18:
			if (m_type == NEOGEO_MVS)
				set_output_latch(data);
			break;

		case 0x20:
			if (m_type == NEOGEO_MVS)
				set_output_data(data);
			break;

		case 0x28:
			if (m_type == NEOGEO_MVS)
			{
				m_upd4990a->data_in_w(data >> 0 & 1);
				m_upd4990a->clk_w(data >> 1 & 1);
				m_upd4990a->stb_w(data >> 2 & 1);
			}
			break;

//  case 0x30: break; // coin counters
//  case 0x31: break; // coin counters
//  case 0x32: break; // coin lockout
//  case 0x33: break; // coin lockout

		default:
			logerror("PC: %x  Unmapped I/O control write.  Offset: %x  Data: %x\n", space.device().safe_pc(), offset, data);
			break;
	}
}


/*************************************
 *
 *  Unmapped memory access
 *
 *************************************/

READ16_MEMBER(neogeo_state::neogeo_unmapped_r)
{
	UINT16 ret;

	/* unmapped memory returns the last word on the data bus, which is almost always the opcode
	   of the next instruction due to prefetch */

	/* prevent recursion */
	if (m_recurse)
		ret = 0xffff;
	else
	{
		m_recurse = true;
		ret = space.read_word(space.device().safe_pc());
		m_recurse = false;
	}
	return ret;
}



/*************************************
 *
 *  NVRAM (Save RAM)
 *
 *************************************/

void neogeo_state::set_save_ram_unlock( UINT8 data )
{
	m_save_ram_unlocked = data;
}


WRITE16_MEMBER(neogeo_state::save_ram_w)
{
	if (m_save_ram_unlocked)
		COMBINE_DATA(&m_save_ram[offset]);
}



/*************************************
 *
 *  Memory card
 *
 *************************************/

CUSTOM_INPUT_MEMBER(neogeo_state::get_memcard_status)
{
	// D0 and D1 are memcard 1 and 2 presence indicators, D2 indicates memcard
	// write protect status (we are always write enabled)
	return (m_memcard->present() == -1) ? 0x07 : 0x00;
}


READ16_MEMBER(neogeo_state::memcard_r)
{
	m_maincpu->eat_cycles(2); // insert waitstate

	UINT16 ret;

	if (m_memcard->present() != -1)
		ret = m_memcard->read(space, offset) | 0xff00;
	else
		ret = 0xffff;

	return ret;
}


WRITE16_MEMBER(neogeo_state::memcard_w)
{
	m_maincpu->eat_cycles(2); // insert waitstate

	if (ACCESSING_BITS_0_7)
	{
		if (m_memcard->present() != -1)
				m_memcard->write(space, offset, data);
	}
}

/*************************************
 *
 *  Inter-CPU communications
 *
 *************************************/

WRITE8_MEMBER(neogeo_state::audio_command_w)
{
	soundlatch_write(data);

	m_audio_cpu_nmi_pending = true;
	audio_cpu_check_nmi();

	/* boost the interleave to let the audio CPU read the command */
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50));
}


READ8_MEMBER(neogeo_state::audio_command_r)
{
	UINT8 ret = soundlatch_read();

	m_audio_cpu_nmi_pending = false;
	audio_cpu_check_nmi();

	return ret;
}


CUSTOM_INPUT_MEMBER(neogeo_state::get_audio_result)
{
	UINT8 ret = soundlatch_read(1); // soundlatch2_byte_r

	return ret;
}




void neogeo_state::neogeo_main_cpu_banking_init()
{
	/* create vector banks */
//  m_bank_vectors->configure_entry(1, m_region_maincpu->base());
//  m_bank_vectors->configure_entry(0, memregion("mainbios")->base());
//  m_bank_vectors->set_entry(0);
	m_use_cart_vectors = 0;

	if (m_type != NEOGEO_CD)
	{
		if (!m_cartslots[0]) m_banked_cart->init_banks();
	}
}


/*************************************
 *
 *  Audio CPU banking
 *
 *************************************/

READ8_MEMBER(neogeo_state::audio_cpu_bank_select_r)
{
	m_bank_audio_cart[offset & 3]->set_entry(offset >> 8);

	return 0;
}


void neogeo_state::neogeo_audio_cpu_banking_init(int set_entry)
{
	if (m_type == NEOGEO_CD) return;

	int region;
	int bank;
	UINT8 *rgn;
	UINT32 address_mask;

	rgn = memregion("audiocpu")->base();

	/* audio bios/cartridge selection */
	m_bank_audio_main->configure_entry(1, memregion("audiocpu")->base());
	if (memregion("audiobios"))
		m_bank_audio_main->configure_entry(0, memregion("audiobios")->base());
	else /* on hardware with no SM1 ROM, the cart ROM is always enabled */
		m_bank_audio_main->configure_entry(0, memregion("audiocpu")->base());

	m_bank_audio_main->set_entry(m_use_cart_audio);

	/* audio banking */
	m_bank_audio_cart[0] = membank("audio_f000");
	m_bank_audio_cart[1] = membank("audio_e000");
	m_bank_audio_cart[2] = membank("audio_c000");
	m_bank_audio_cart[3] = membank("audio_8000");

	address_mask = (memregion("audiocpu")->bytes() - 0x10000 - 1) & 0x3ffff;


	for (region = 0; region < 4; region++)
	{
		for (bank = 0xff; bank >= 0; bank--)
		{
			UINT32 bank_address = 0x10000 + ((bank << (11 + region)) & address_mask);
			m_bank_audio_cart[region]->configure_entry(bank, &rgn[bank_address]);
		}
	}

	// set initial audio banks - THIS IS A HACK
	// Z80 banking is handled by the NEO-ZMC chip in the cartridge
	// (in later cartridges, by multifunction banking/protection chips that implement the same bank scheme)
	// On the real chip, initial banks are all 0.
	// However, early cartridges with less than 64KB of Z80 code and data don't have ROM banking at all.
	// These initial bank settings are required so non-banked games will work until we identify them
	// and use a different Z80 address map for them.
	m_bank_audio_cart[0]->set_entry(0x1e);
	m_bank_audio_cart[1]->set_entry(0x0e);
	m_bank_audio_cart[2]->set_entry(0x06);
	m_bank_audio_cart[3]->set_entry(0x02);
}



/*************************************
 *
 *  System control register
 *
 *************************************/

WRITE8_MEMBER(neogeo_state::system_control_w)
{
	UINT8 bit = (offset >> 3) & 0x01;

	switch (offset & 0x07)
	{
		default:
		case 0x00:
			neogeo_set_screen_shadow(bit);
			break;

		case 0x01:
			if (m_type == NEOGEO_CD)
				printf("NeoCD: write to regular vector change address? %d\n", bit); // what IS going on with "neocdz doubledr" and why do games write here if it's hooked up to nothing?
			else
				//m_bank_vectors->set_entry(bit);
				m_use_cart_vectors = bit;
			break;

		case 0x05:
			if (m_type == NEOGEO_MVS)
			{
				m_use_cart_audio = bit;
				m_sprgen->neogeo_set_fixed_layer_source(bit);
				m_bank_audio_main->set_entry(m_use_cart_audio);
			}
			break;

		case 0x06:
			if (m_type == NEOGEO_MVS)
				set_save_ram_unlock(bit);
			break;

		case 0x07:
			neogeo_set_palette_bank(bit);
			break;

		case 0x02: // memory card 1: write enable/disable
		case 0x03: // memory card 2: write disable/enable
		case 0x04: // memory card: register select enable/set to normal (what does it mean?)
			logerror("PC: %x  Unmapped system control write.  Offset: %x  Data: %x\n", space.device().safe_pc(), offset & 0x07, bit);
			break;
	}

	if (LOG_VIDEO_SYSTEM && ((offset & 0x07) != 0x06)) logerror("PC: %x  System control write.  Offset: %x  Data: %x\n", space.device().safe_pc(), offset & 0x07, bit);
}



/*************************************
 *
 *  LEDs
 *
 *************************************/

void neogeo_state::set_outputs(  )
{
	static const UINT8 led_map[0x10] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x58,0x4c,0x62,0x69,0x78,0x00 };

	/* EL */
	output().set_digit_value(0, led_map[m_el_value]);

	/* LED1 */
	output().set_digit_value(1, led_map[m_led1_value >> 4]);
	output().set_digit_value(2, led_map[m_led1_value & 0x0f]);

	/* LED2 */
	output().set_digit_value(3, led_map[m_led2_value >> 4]);
	output().set_digit_value(4, led_map[m_led2_value & 0x0f]);
}


void neogeo_state::set_output_latch( UINT8 data )
{
	/* looks like the LEDs are set on the
	   falling edge */
	UINT8 falling_bits = m_output_latch & ~data;

	if (falling_bits & 0x08)
		m_el_value = 16 - (m_output_data & 0x0f);

	if (falling_bits & 0x10)
		m_led1_value = ~m_output_data;

	if (falling_bits & 0x20)
		m_led2_value = ~m_output_data;

	if (falling_bits & 0xc7)
		logerror("%s  Unmaped LED write.  Data: %x\n", machine().describe_context(), falling_bits);

	m_output_latch = data;

	set_outputs();
}


void neogeo_state::set_output_data( UINT8 data )
{
	m_output_data = data;
}



/*************************************
 *
 *  Machine initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(neogeo_state,neogeo)
{
	if (!m_cartslots[0]) m_banked_cart->install_banks(machine(), m_maincpu, m_region_maincpu->base(), m_region_maincpu->bytes());

	m_sprgen->m_fixed_layer_bank_type = 0;
}


void neogeo_state::neogeo_postload()
{
	m_bank_audio_main->set_entry(m_use_cart_audio);

	if (m_type == NEOGEO_MVS) set_outputs();
}


void neogeo_state::machine_start()
{
	m_type = NEOGEO_MVS;


	/* set the initial main CPU bank */
	neogeo_main_cpu_banking_init();

	/* set the initial audio CPU ROM banks */
	neogeo_audio_cpu_banking_init(1);

	create_interrupt_timers();

	/* irq levels for MVS / AES */
	m_vblank_level = 1;
	m_raster_level = 2;

	/* start with an IRQ3 - but NOT on a reset */
	m_irq3_pending = 1;

	// enable rtc and serial mode
	m_upd4990a->cs_w(1);
	m_upd4990a->oe_w(1);
	m_upd4990a->c0_w(1);
	m_upd4990a->c1_w(1);
	m_upd4990a->c2_w(1);

	/* register state save */
	save_item(NAME(m_display_position_interrupt_control));
	save_item(NAME(m_display_counter));
	save_item(NAME(m_vblank_interrupt_pending));
	save_item(NAME(m_display_position_interrupt_pending));
	save_item(NAME(m_irq3_pending));
	save_item(NAME(m_audio_cpu_nmi_enabled));
	save_item(NAME(m_audio_cpu_nmi_pending));
	save_item(NAME(m_controller_select));
	save_item(NAME(m_save_ram_unlocked));
	save_item(NAME(m_output_data));
	save_item(NAME(m_output_latch));
	save_item(NAME(m_el_value));
	save_item(NAME(m_led1_value));
	save_item(NAME(m_led2_value));

	save_item(NAME(m_use_cart_vectors));
	save_item(NAME(m_use_cart_audio));

	machine().save().register_postload(save_prepost_delegate(FUNC(neogeo_state::neogeo_postload), this));


	m_cartslots[0] = m_cartslot1;
	m_cartslots[1] = m_cartslot2;
	m_cartslots[2] = m_cartslot3;
	m_cartslots[3] = m_cartslot4;
	m_cartslots[4] = m_cartslot5;
	m_cartslots[5] = m_cartslot6;


	m_sprgen->set_screen(m_screen);
	m_sprgen->set_sprite_region(m_region_sprites->base(), m_region_sprites->bytes());
	m_sprgen->set_fixed_regions(m_region_fixed->base(), m_region_fixed->bytes(), m_region_fixedbios);

}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

void neogeo_state::set_slot_number(int slot)
{
	if (slot != m_currentslot)
	{
		m_currentslot = slot;

		address_space &space = m_maincpu->space(AS_PROGRAM);

		// unmap old handlers, some carts will have installed overlays on them, we need them to be cleared
		space.unmap_readwrite(0x000080, 0x0fffff);
		space.unmap_readwrite(0x200000, 0x2fffff);


		if (m_cartslots[m_currentslot]->get_sprites_size() == 0)
			return;

		// give the sprite chip pointers to the graphics for this slot from the slot device
		m_sprgen->set_sprite_region(m_cartslots[m_currentslot]->get_sprites_base(), m_cartslots[m_currentslot]->get_sprites_size());
		m_sprgen->set_fixed_regions(m_cartslots[m_currentslot]->get_fixed_base(), m_cartslots[m_currentslot]->get_fixed_size(), m_region_fixedbios);
		m_sprgen->set_optimized_sprite_data(m_cartslots[m_currentslot]->get_sprites_optimized(), m_cartslots[m_currentslot]->get_sprites_addrmask());
		m_sprgen->m_fixed_layer_bank_type = m_cartslots[m_currentslot]->get_fixed_bank_type();
		/*
		    Resetting a sound device causes the core to update() it and generate samples if it's not up to date.
		    Thus we preemptively reset it here while the old pointers are still valid so it's up to date and
		    doesn't generate samples below when we reset it for the new pointers.
		*/
		device_t* ym = machine().device("ymsnd");
		ym->reset();

		m_cartslots[m_currentslot]->setup_memory_banks(machine()); // setup basic pointers

		ym->reset(); // reset it again to get the new pointers

		// these could have changed, ensure the pointers are valid
		m_region_maincpu.findit();
		m_region_sprites.findit();
		m_region_fixed.findit();

		space.install_rom(0x000080, 0x0fffff, m_region_maincpu->base()+0x80); // reinstall the base program rom handler

		m_cartslots[m_currentslot]->activate_cart(machine(), m_maincpu, m_region_maincpu->base(), m_region_maincpu->bytes(), m_cartslots[m_currentslot]->get_fixed_base(), m_cartslots[m_currentslot]->get_fixed_size());
		//memcpy((UINT8*)m_cartslots[m_currentslot]->get_rom_base(),m_region_maincpu->base(), m_region_maincpu->bytes()); // hack- copy back any mods activate made (eh cthd2003)c

		neogeo_audio_cpu_banking_init(0); // should probably be responsibility of the cart
		m_audiocpu->reset(); // or some games like svc have no sounnd if in higher slots?

	}
}

void neogeo_state::machine_reset()
{
	offs_t offs;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* reset system control registers */
	for (offs = 0; offs < 8; offs++)
		system_control_w(space, offs, 0);

	// disable audiocpu nmi
	m_audio_cpu_nmi_enabled = false;
	m_audio_cpu_nmi_pending = false;
	audio_cpu_check_nmi();

	m_maincpu->reset();

	start_interrupt_timers();

	/* trigger the IRQ3 that was set by MACHINE_START */
	update_interrupts();

	m_recurse = false;

	if (m_cartslots[0]) // if thie system has cart slots then do some extra initialization
	{
		set_slot_number(0);
	}

}

READ16_MEMBER(neogeo_state::banked_vectors_r)
{
	if (!m_use_cart_vectors)
	{
		UINT16* bios = (UINT16*)memregion("mainbios")->base();
		return bios[offset];
	}
	else
	{
		UINT16* game = (UINT16*)m_region_maincpu->base();
		return game[offset];
	}

}

READ16_MEMBER(neogeo_state::neogeo_slot_rom_low_r)
{
	return m_cartslots[m_currentslot]->read_rom(space, offset+(0x80/2), mem_mask);
}

READ16_MEMBER(neogeo_state::neogeo_slot_rom_low_bectors_r)
{
	if (!m_use_cart_vectors)
	{
		UINT16* bios = (UINT16*)memregion("mainbios")->base();
		return bios[offset];
	}
	else
	{
		return m_cartslots[m_currentslot]->read_rom(space, offset, mem_mask);
	}

}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

ADDRESS_MAP_START( neogeo_main_map, AS_PROGRAM, 16, neogeo_state )

	AM_RANGE(0x100000, 0x10ffff) AM_MIRROR(0x0f0000) AM_RAM
	/* some games have protection devices in the 0x200000 region, it appears to map to cart space, not surprising, the ROM is read here too */
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01ff7e) AM_READ_PORT("P1/DSW")
	AM_RANGE(0x300080, 0x300081) AM_MIRROR(0x01ff7e) AM_READ_PORT("TEST")
	AM_RANGE(0x300000, 0x300001) AM_MIRROR(0x01fffe) AM_WRITE8(watchdog_reset_w, 0x00ff)
	AM_RANGE(0x320000, 0x320001) AM_MIRROR(0x01fffe) AM_READ_PORT("AUDIO/COIN") AM_WRITE8(audio_command_w, 0xff00)
	AM_RANGE(0x340000, 0x340001) AM_MIRROR(0x01fffe) AM_READ_PORT("P2")
	AM_RANGE(0x360000, 0x37ffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x380000, 0x380001) AM_MIRROR(0x01fffe) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x380000, 0x38007f) AM_MIRROR(0x01ff80) AM_WRITE8(io_control_w, 0x00ff)
	AM_RANGE(0x3a0000, 0x3a001f) AM_MIRROR(0x01ffe0) AM_READ(neogeo_unmapped_r) AM_WRITE8(system_control_w, 0x00ff)
	AM_RANGE(0x3c0000, 0x3c0007) AM_MIRROR(0x01fff8) AM_READ(neogeo_video_register_r)
	AM_RANGE(0x3c0000, 0x3c000f) AM_MIRROR(0x01fff0) AM_WRITE(neogeo_video_register_w)
	AM_RANGE(0x3e0000, 0x3fffff) AM_READ(neogeo_unmapped_r)
	AM_RANGE(0x400000, 0x401fff) AM_MIRROR(0x3fe000) AM_READWRITE(neogeo_paletteram_r, neogeo_paletteram_w)
	AM_RANGE(0x800000, 0x800fff) AM_READWRITE(memcard_r, memcard_w)
	AM_RANGE(0xc00000, 0xc1ffff) AM_MIRROR(0x0e0000) AM_ROM AM_REGION("mainbios", 0)
	AM_RANGE(0xd00000, 0xd0ffff) AM_MIRROR(0x0f0000) AM_RAM_WRITE(save_ram_w) AM_SHARE("saveram")
	AM_RANGE(0xe00000, 0xffffff) AM_READ(neogeo_unmapped_r)
ADDRESS_MAP_END




static ADDRESS_MAP_START( main_map_slot, AS_PROGRAM, 16, neogeo_state )
	AM_RANGE(0x000000, 0x00007f) AM_READ(neogeo_slot_rom_low_bectors_r)
	AM_RANGE(0x000080, 0x0fffff) AM_READ(neogeo_slot_rom_low_r)
	AM_RANGE(0x200000, 0x2fffff) AM_ROMBANK("cartridge")
//  AM_RANGE(0x2ffff0, 0x2fffff) AM_WRITE(main_cpu_bank_select_w)
	AM_IMPORT_FROM( neogeo_main_map )
ADDRESS_MAP_END

/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, neogeo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROMBANK("audio_main")
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("audio_8000")
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("audio_c000")
	AM_RANGE(0xe000, 0xefff) AM_ROMBANK("audio_e000")
	AM_RANGE(0xf000, 0xf7ff) AM_ROMBANK("audio_f000")
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Audio CPU port handlers
 *
 *************************************/

static ADDRESS_MAP_START( audio_io_map, AS_IO, 8, neogeo_state )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xff00) AM_READWRITE(audio_command_r, soundlatch_clear_byte_w)
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xff00) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0xff10) AM_MASK(0x0010) AM_WRITE(audio_cpu_enable_nmi_w)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xfff0) AM_MASK(0xff03) AM_READ(audio_cpu_bank_select_r)
	AM_RANGE(0x0c, 0x0c) AM_MIRROR(0xff00) AM_WRITE(soundlatch2_byte_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Standard Neo-Geo DIPs and
 *  input port definition
 *
 *************************************/

INPUT_PORTS_START( neogeo )
	PORT_START("P1/DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "Setting Mode" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "VS Mode" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Controller ) ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Joystick ) )
	PORT_DIPSETTING(      0x0000, "Mahjong Panel" )
	PORT_DIPNAME( 0x0018, 0x0018, "COMM Setting (Cabinet No.)" ) PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING(      0x0018, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0020, 0x0020, "COMM Setting (Link Enable)" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("P2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Next Game") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Previous Game") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, neogeo_state, get_memcard_status, NULL)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL ) /* Hardware type (AES=0, MVS=1). Some games check this and show a piracy warning screen if the hardware and BIOS don't match */

	PORT_START("AUDIO/COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN3 ) /* What is this? "us-e" BIOS uses it as a coin input; Universe BIOS uses it to detect MVS or AES hardware */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN4 ) /* What is this? "us-e" BIOS uses it as a coin input; Universe BIOS uses it to detect MVS or AES hardware */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL ) /* what is this? When ACTIVE_HIGH + IN4 bit 6 ACTIVE_LOW MVS-4 slot is detected */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("upd4990a", upd1990a_device, tp_r)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("upd4990a", upd1990a_device, data_out_r)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(DEVICE_SELF, neogeo_state,get_audio_result, NULL)

	PORT_START("TEST")
	PORT_BIT( 0x003f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* what is this? If ACTIVE_LOW, MVS-6 slot detected, when ACTIVE_HIGH MVS-1 slot (AES) detected */
	PORT_SERVICE_NO_TOGGLE( 0x0080, IP_ACTIVE_LOW )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( neogeo_6slot )
	PORT_INCLUDE( neogeo )

	PORT_MODIFY("TEST")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SPECIAL )
INPUT_PORTS_END




DRIVER_INIT_MEMBER(neogeo_state,mvs)
{
	DRIVER_INIT_CALL(neogeo);
}




/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_CONFIG_START( neogeo_base, neogeo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, NEOGEO_MAIN_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(neogeo_main_map)

	MCFG_CPU_ADD("audiocpu", Z80, NEOGEO_AUDIO_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_IO_MAP(audio_io_map)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_neogeo)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(NEOGEO_PIXEL_CLOCK, NEOGEO_HTOTAL, NEOGEO_HBEND, NEOGEO_HBSTART, NEOGEO_VTOTAL, NEOGEO_VBEND, NEOGEO_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(neogeo_state, screen_update_neogeo)

	/* 4096 colors * two banks * normal and shadow */
	MCFG_PALETTE_ADD_INIT_BLACK("palette", 4096*2*2)

	MCFG_DEVICE_ADD("spritegen", NEOGEO_SPRITE_OPTIMZIED, 0)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2610, NEOGEO_YM2610_CLOCK)
	MCFG_YM2610_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.28)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.28)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.98)
	MCFG_SOUND_ROUTE(2, "rspeaker", 0.98)
	MCFG_NEOGEO_BANKED_CART_ADD("banked_cart")

MACHINE_CONFIG_END


MACHINE_CONFIG_DERIVED( neogeo_arcade, neogeo_base )
	MCFG_WATCHDOG_TIME_INIT(attotime::from_ticks(3244030, NEOGEO_MASTER_CLOCK))
	MCFG_UPD4990A_ADD("upd4990a", XTAL_32_768kHz, NULL, NULL)
	MCFG_NVRAM_ADD_0FILL("saveram")
	MCFG_NEOGEO_MEMCARD_ADD("memcard")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( mvs, neogeo_arcade )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(main_map_slot)

	MCFG_NEOGEO_CARTRIDGE_ADD("cartslot1", neogeo_cart, nullptr)
	MCFG_NEOGEO_CARTRIDGE_ADD("cartslot2", neogeo_cart, nullptr)
	MCFG_NEOGEO_CARTRIDGE_ADD("cartslot3", neogeo_cart, nullptr)
	MCFG_NEOGEO_CARTRIDGE_ADD("cartslot4", neogeo_cart, nullptr)
	MCFG_NEOGEO_CARTRIDGE_ADD("cartslot5", neogeo_cart, nullptr)
	MCFG_NEOGEO_CARTRIDGE_ADD("cartslot6", neogeo_cart, nullptr)

	MCFG_SOFTWARE_LIST_ADD("cart_list","neogeo")
MACHINE_CONFIG_END







/* dummy entry for the dummy bios driver */
ROM_START( neogeo )
	NEOGEO_BIOS

	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_Y_ZOOM

	ROM_REGION( 0x20000, "fixed", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION( 0x10000, "ymsnd", ROMREGION_ERASEFF )

	NO_DELTAT_REGION

	ROM_REGION( 0x100000, "sprites", ROMREGION_ERASEFF )
ROM_END


/*    YEAR  NAME        PARENT    COMPAT    MACHINE   INPUT     INIT    */
CONS( 1990, neogeo,     0,        0,        mvs,      neogeo_6slot,   neogeo_state, mvs,  "SNK", "Neo-Geo", MACHINE_IS_BIOS_ROOT | MACHINE_SUPPORTS_SAVE )
