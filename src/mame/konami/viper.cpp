// license:BSD-3-Clause
// copyright-holders:Ville Linde, Angelo Salese

/*
    Konami Viper System

    Driver by Ville Linde

    Software notes (as per Police 911)
    -- VL - 01.06.2011

    IRQs:

    IRQ0: ???               (Task 4)
    IRQ1: unused
    IRQ2: ???               Possibly UART? Accesses registers at 0xffe00008...f
    IRQ3: Sound             (Task 5)
    IRQ4: Voodoo3           Currently only for User Interrupt Command, maybe a more extensive handler gets installed later?

    I2C:  ???               (no task switch) what drives this? network? U13 (ADC838) test fails if I2C doesn't work
    DMA0: unused
    DMA1: unused
    IIVPR3: unused

    Memory:

    0x000001E0:             Current task
    0x000001E1:             Current FPU task
    0x000001E4:             Scheduled tasks bitvector (bit 31 = task0, etc.)
    0x00000A00...BFF:       Task structures
                            0x00-03:    unknown
                            0x04:       unknown
                            0x05:       if non-zero, this task uses FPU
                            0x06-07:    unknown
                            0x08:       unknown mem pointer, task stack pointer?
                            0x0c:       pointer to task PC (also top of stack?)

    Sound:
    0x00001320:             A flag that's used when sound effects(?) are being played
    0x00001324:             Pointer to the data cache buffer to be used for loading and mixing BGM/SE.
                            Each buffer is 0x800 bytes in size and the game will switch between the two every IRQ3(?).
                            The original audio typically seems to be ADPCM which is then decoded and mixed in software.
    0x00001330:             L/R channel PCM data when a sound effect is played? Seems to be the last result when mixing down buffers.


    0x00000310:             Global timer 0 IRQ handler
    0x00000320:             Global timer 1 IRQ handler
    0x00000330:             Global timer 2 IRQ handler
    0x00000340:             Global timer 3 IRQ handler
    0x00000350:             IRQ0 handler
    0x00000360:             IRQ1 handler
    0x00000370:             IRQ2 handler
    0x00000380:             IRQ3 handler
    0x00000390:             IRQ4 handler
    0x000003a0:             I2C IRQ handler
    0x000003b0:             DMA0 IRQ handler
    0x000003c0:             DMA1 IRQ handler
    0x000003d0:             Message Unit IRQ handler

    0x000004e4:             Global timer 0 IRQ handler function ptr
    0x000004e8:             Global timer 1 IRQ handler function ptr
    0x000004ec:             Global timer 2 IRQ handler function ptr
    0x000004f0:             Global timer 3 IRQ handler function ptr


    IRQ0:       Vector 0x0004e020       Stack 0x000d4fa4
    IRQ1:       Vector 0x0000a5b8       Stack 0x0001323c    (dummy)
    IRQ2:       Vector 0x000229bc       Stack 0x000d4fa4
    IRQ3:       Vector 0x006a02f4       Stack 0x006afeb0
    IRQ4:       Vector 0x0068c354       Stack 0x0068cc54
    I2C:        Vector 0x00023138       Stack 0x000d4fa4


    Functions of interest:

    0x0000f7b4:     SwitchTask()
    0x0000c130:     ScheduleTask()
    0x00009d00:     LoadProgram(): R3 = ptr to filename

    TODO:
    - needs a proper way to dump security dongles, anything but p9112 has placeholder ROM for
      ds2430.
    - Voodoo 3 has issues with LOD minimums, cfr. mocapglf where card check don't display a bar
      near the percentage;
    - convert i2c to be a real i2c-complaint device;
    - hookup adc0838, reads from i2c;
    - convert epic to be a device, make it input_merger/irq_callback complaint;
    - (more intermediate steps for proper PCI conversions here)
    - xtrial: hangs when coined up;
    - gticlub2: throws NETWORK ERROR after course select;
    - jpark3: attract mode demo play acts weird, the dinosaur gets submerged
      and camera doesn't really know what to do, CPU core bug?
    - jpark3: crashes during second attract cycle;
    - sscopex, thrild2: attract mode black screens (coin still works), sogeki/sscopefh are unaffected;
    - thrild2: no BGMs;
    - wcombat: black screen when entering service mode;
    - mocapglf, sscopefh, sscopex: implement 2nd screen output, controlled by IP90C63A;
    \- sscopex/sogeki desyncs during gameplay intro, leaves heavy trails in gameplay;
    - ppp2nd: hangs when selecting game mode from service (manages to save);
    - code1db: crashes when selecting single course type;
    - wcombatj: gets stuck on network check;
    - thrild2c: blue screen;
    - thrild2ac: black screen;
    - all games needs to be verified against factory settings
      (game options, coin options & sound options often don't match "green colored" defaults)

    Other notes:
    - "Distribution error" means there's a region mismatch.
    - Hold TEST while booting (from the very start) to initialize the RTC for most games.
    - It seems that p911 has 3 unique regional images: U/E, K/A, and J. If you try booting, for example, U region on a K/A image, it won't find some files and will error out with "distribution error".
    - mocapglf: enable "show diag" at boot then disable it once the diag text appears.
      This will allow game to bypass the I/O SENSOR error later on.

    Game status (potentially outdated, to be moved on top):
        boxingm             Goes in-game. Controllers are not emulated. Various graphical glitches.
        jpark3              Goes in-game. Controllers are not emulated. Various graphical glitches.
        mocapb,j            Goes in-game. Controllers are not emulated. Various graphical glitches. Random crashes.
        ppp2nd,a            Fully playable with graphical glitches. No network or DVD support. Crashes when returning to game mode from test menu.
        p911(all)           Goes in-game. Controllers are not emulated. Various graphical glitches.
        tsurugi,j           Goes in-game. Controllers are not emulated. Various graphical glitches.
        p9112               Goes in-game. Controllers are not emulated. Various graphical glitches.

        gticlub2,ea         Attract mode works. Coins up. Hangs in various places. Will crash with "network error" after stage is selected.
        thrild2,a           Attract mode with partial graphics. Coins up. Hangs in various places.

        sscopefh            Graphics heavily glitched. Gun controller is not emulated. Sensor error and hopper error stop it from working.

        mfightc,c           Requires touch panel emulation. Gets stuck at "Waiting for central monitor, checking serial...".
        xtrial              Hangs at "Please set the time for the bookkeeping" message.

        code1d,b,a          Can boot but crashes randomly and quickly so it's hard to do anything.

        mocapglf            Gets stuck at "SENSOR I/O ERROR" though test menu can still be entered.
        sscopex,sogeki      Graphics very heavily glitched. Gun controller is not emulated.

        wcombat             Can boot into a test menu by using a combination of dipswitches, but it says "serial check bad". Can't boot normally.
        wcombatu            Bootable when dipsw 4 is set to on. Controls not implemented so it's not possible to pass nickname selection screen. Freezes when test button is pressed.
        thrild2c,ac         Inf loop on blue screen



===================================================================================================

Konami Viper Hardware Overview (last updated 5th June 2011 10:56pm)

Games on this hardware include:

Konami
Game ID  Year    Game
--------------------------------------------------------------------------------------------------------------------
GK922    2000    Code One Dispatch
G????    2001    ParaParaParadise 2nd Mix
GM941    2000    Driving Party: Racing in Italy (World) / GTI Club: Corso Italiano (Japan) / GTI Club 2 (USA?)
G?A00    2000    Police 911 (USA) / Police 24/7 (World) / The Keisatsukan: Shinjuku 24-ji (Asia/Japan/Korea)
GKA13    2001    Silent Scope EX (USA/World) / Sogeki (Japan)
G?A29    2001    Mocap Boxing
G?A30    2002    Blade of Honor (USA) / Tsurugi (World/Japan)
GMA41    2001    Thrill Drive 2
G?A45    2001    Boxing Mania
G*B11    2001    Police 911 2 (USA) / Police 24/7 2 (World) / The Keisatsukan 2: Zenkoku Daitsuiseki Special (Japan)
G?B33    2001    Mocap Golf
G?B41    2001    Jurassic Park III
G?B4x    2002    Xtrial Racing
G?C09    2002    Mahjong Fight Club
G?C22    2002    World Combat (USA/Japan/Korea) / Warzaid (Europe)

PCB Layout
----------
Early revision - GM941-PWB(A)B (CN13/15/16 not populated and using 941A01 BIOS)
Later revision - GM941-PWB(A)C (with 941B01 BIOS)
Copyright 1999 KONAMI
  |----------------------------------------------------------|
  |            LA4705      6379AL                            |
|-|  TD62064                               14.31818MHz   CN15|
|                  3793-A                                    |
|     |------|                            |--------|         |
|J    |056879|                            |3DFX    |MB81G163222-80
|A    |      |PQR0RV21                    |355-0024|         |
|M    |------|          XC9572XL          |-030    |MB81G163222-80
|M                                        |--------|         |
|A                                    MB81G163222-80         |
|    ADC0838           |------------|          MB81G163222-80|
|    LM358             |MOTOROLA    |                        |
|                      XPC8240LZU200E  33.868MHz         CN13|
|-| PC16552            |            |                        |
  |           PQ30RV21 |            |        CY7C199         |
|-|                    |            |                        |
|                      |------------|        XCS10XL         |
|                 48LC2M32B2   48LC2M32B2                    |
|2                                                           |
|8                                       CN17                |
|W       XC9536(1)  XC9536(2)        |-------------|         |
|A                                   |  DUAL       |         |
|Y                                   |  PCMCIA     |         |
|                     M48T58Y.U39    |  SLOTS      |         |
|                                    |             |         |
|           29F002                   |             |     CN16|
|-|                       DS2430.U37 |             |         |
  | DIP(4)  CN4 CN5 CN7         CN9  |             |  CN12   |
  |----------------------------------|-------------|---------|
Notes:
XPC8240LZU200E - Motorola XPC8240LZU200E MPC8420 PPC603e-based CPU (TBGA352 @ U38). Clock input is 33.868MHz
                Chip rated at 200MHz so likely clock is 33.868 x6 = 203.208MHz
         3DFX - 3DFX Voodoo III 3500 graphics chip with heatsink (BGA @ U54). Clock input 14.31818MHz
                Full markings: 355-0024-030 F26664.10C 0025 20005 TAIWAN 1301
   48LC2M32B2 - Micron Technology 48LC2M32B2-6 2M x32-bit (512k x 32 x 4 banks = 64MB) 166MHz Synchronous
                DRAM (TSOP86 @ U28 & U45)
MB81G163222-80 - Fujitsu MB81G163222-80 256k x 32-bit x 2 banks Synchronous Graphics DRAM (TQFP100 @ U53, U56, U59 & U60)
      CY7C199 - Cypress Semiconductor CY7C199-15VC 32k x8 SRAM (SOJ28 @ U57)
      PC16552 - National Semiconductor PC16552D Dual Universal Asynchronous Receiver/Transmitter with FIFO's (PLCC44 @ U7)
    XC9536(1) - Xilinx XC9536 In-System Programmable CPLD stamped 'M941A1' (PLCC44 @ U17)
    XC9536(2) - Xilinx XC9536 In-System Programmable CPLD stamped 'M941A2' (PLCC44 @ U24)
     XC9572XL - Xilinx XC9572XL High Performance CPLD stamped 'M941A3A' (PLCC44 @ U29)
      XCS10XL - Xilinx XCS10XL Spartan-XL FPGA (TQFP100 @ U55)
       056879 - Konami 056879 custom IC (QFP120 @ U15)
     PQ30RV21 - Sharp PQ30RV21 low-power voltage regulator (5 Volt to 3 Volt)
       LA4705 - Sanyo LA4705 15W 2-channel power amplifier (SIP18)
        LM358 - National Semiconductor LM358 low power dual operational amplifier (SOIC8 @ U14)
       6379AL - NEC uPC6379AL 2-channel 16-bit D/A converter (SOIC8 @ U30)
      ADC0838 - National Semiconductor ADC0838 Serial I/O 8-Bit A/D Converters with Multiplexer Options (SOIC20 @ U13)
       DS2430 - Dallas DS2430 256-bits 1-Wire EEPROM.
      M48T58Y - ST Microelectronics M48T58Y Timekeeper RAM (DIP28 @ U39).
       29F002 - Fujitsu 29F002 256k x8 EEPROM stamped '941B01' (PLCC44 @ U25). Earlier revision stamped '941A01'
      CN4/CN5 - RCA-type network connection jacks
          CN7 - 80 pin connector (unused in all games?)
          CN9 - DIN5 socket for dongle. Dongle is a DIN5 male plug containing a standard DS2430 wired to
                DIN pins 2, 3 & 4. Pin 1 NC, Pin 2 GND, Pin 3 DATA, Pin 4 NC, Pin 5 NC. If the dongle is
                required and plugged in it overrides the DS2430 on the main board. Without the (on-board)
                DS2430 the PCB will complain after the CF check with HARDWARE ERROR. If the DS2430 is not
                correct for the game the error given is RTC BAD even if the RTC is correct. Most games don't require
                a dongle and accept any DS2430 on the main board.
         CN12 - 4 pin connector (possibly stereo audio output?)
         CN13 - Power connector for plug-in daughterboard
    CN15/CN16 - Multi-pin IDC connectors for plug-in daughterboard (see detail below)
         CN17 - Dual PCMCIA slots. Usually only one slot is used containing a PCMCIA to CF adapter. The entire game
                software resides on the CF card. Games use 32M, 64M and 128M CF cards. In many cases a different
                CF card version of the same game can be swapped and the existing RTC works but sometimes the RTC data
                needs to be re-initialised to factory defaults by entering test mode. Sometimes the game will not boot
                and gives error RTC BAD meaning the RTC is not compatible with the version or the dongle is required.
                See DS2430 below for more info.
       28-WAY - Edge connector used for connecting special controls such as guns etc.
       DIP(4) - 4-position DIP switch. Switch 1 skips the CF check for a faster boot-up. The others appear unused?

[DS2430] Has 256 bits x8 EEPROM (32 bytes), 64 bits x8 (8 bytes)
    one-time programmable application register and unique factory-lasered and tested 64-bit
    registration number (8-bit family code + 48-bit serial number + 8-bit CRC) (TO-92 @ U37)
    The OTP application register on the common DS2430 and the Police 911 2 DS2430 are not programmed
    (application register reads all 0xFF and the status register reads back 0xFF), so it's probably safe
    to assume they're not used on any of them.
    It appears the DS2430 is not protected from reading and the unique silicon serial number is
    included in the 40 byte dump. This serial number is used as a check to verify the NVRAM and DS2430.
    In the Police 911 2 NVRAM dump the serial number of the DS2430 is located at 0x002A and 0x1026
    If the serial number in the NVRAM and DS2430 match then they are paired and the game accepts the NVRAM.
    If they don't match the game requires an external DS2430 (i.e. dongle) and flags the NVRAM as 'BAD'
    The serial number is not present in the CF card (2 different Police 911 2 cards of the same version
    were dumped and matched).
    When the lasered ROM is read from the DS2430, it comes out from LSB to MSB (family code, LSB of
    S/N->MSB of S/N, CRC)
    For Police 911 2 that is 0x14 0xB2 0xB7 0x4A 0x00 0x00 0x00 0x83
    Family code=0x14
    S/N=0x0000004AB7B2
    CRC=0x83
    In a DS2430 dump, the first 32 bytes is the EEPROM and the lasered ROM is 8 bytes and starts at 0x20h
    For Police 911 2 that is....
    00000000h CB 9B 56 EC A0 4C 87 53 51 46 28 E7 00 00 00 74
    00000010h 30 A9 C7 76 B9 85 A3 43 87 53 50 42 1A E7 FA CF
    00000020h 14 B2 B7 4A 00 00 00 83
    It may be possible to hand craft a DS2430 for a dongle-protected version of a game simply by using
    one of the existing DS2430 dumps and adjusting the serial number found in a dump of the NVRAM to pair them
    or adjusting the serial number in the NVRAM to match the serial number found in one of the dumped DS2430s.
    This Police 911 2 board was upgraded from Police 911 by plugging in the dongle and changing the CF card.
    The NVRAM had previously died and the board was dead. Normally for a Viper game that is fatal. Using
    the NVRAM from Police 911 allowed it to boot and then the NVRAM upgraded itself with some additional
    data (the original data remained untouched). This means the dongle does more than just protect the game.
    Another interesting fact about this upgrade is it has been discovered that the PCB can write to the
    external DS2430 in the dongle. This has been proven because the serial number of the DS2430 soldered
    on the PCB is present in the EEPROM area of the Police 911 2 DS2430.
    Here is a dump of the DS2430 from Police 911. Note the EEPROM area is empty and the serial number (from 0x20 onwards)
    is present in the above Police 911 2 DS2430 dump at locations 0x11, 0x10 and 0x0F
    00000000h FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
    00000010h FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
    00000020h 14 A9 30 74 00 00 00 E7
    This proves that the EEPROM area in the DS2430 is unused by an unprotected game and in fact the on-board
    DS2430 is completely unused by an unprotected game. That is why any unprotected game will work on any
    Viper PCB regardless of the on-board DS2430 serial number.
    The existing DS2430 'common' dump used in the unprotected games was actually from a (dongle-protected)
    Mahjong Fight Club PCB but that PCB was used to test and run all of the unprotected Viper games.

[M48T58Y] When this dies (after 10 year lifespan)
    the game will complain with error RTC BAD then reset. The data inside the RTC can not be hand created
    (yet) so to revive the PCB the correct RTC data must be re-programmed to a new RTC and replaced
    on the PCB.
    Regarding the RTC and protection-related checks....
    "RTC OK" checks 0x0000->0x0945 (i.e. I can clear the contents after 0x0945 and the game will still
    happily boot). The NVRAM contents are split into chunks, each of which are checksummed.  It is a 16-bit checksum,
    computed by summing two consecutive bytes as a 16-bit integer, where the final sum must add up to 0xFFFF (mod
    65536).  The last two bytes in the chunk are used to make the value 0xFFFF.  There doesn't appear to be a
    complete checksum over all the chunks (I can pick and choose chunks from various NVRAMs, as long as each chunk
    checksum checks out). The important chunks for booting are the first two.
    The first chunk goes from 0x0000-0x000F.  This seems to be a game/region identifier, and doesn't like its
    contents changed (I didn't try changing every byte, but several of the bytes would throw RTC errors, even with a
    fixed checksum).  I'd guess that the CF verifies this value, since it's different for every game (i.e. Mocap
    Boxing NVRAM would have a correct checksum, but shouldn't pass Police 911 checks).
    The second chunk goes from 0x0010-0x0079.  This seems to be a board identifier.  This has (optionally)
    several fields, each of which are 20 bytes long.  I'm unsure of the first 6 bytes, the following 6
    bytes are the DS2430A S/N, and the last 8 bytes are a game/region/dongle identifier.  If running
    without a dongle, only the first 20 byte field is present.  With a dongle, a second 20 byte field will
    be present.  Moving this second field into the place of the first field (and fixing the checksum)
    doesn't work, and the second field will be ignored if the first field is valid for the game (and in
    which case the dongle will be ignored).  For example, Police 911 will boot with a valid first field,
    with or without the second field, and with or without the dongle plugged in.  If you have both fields,
    and leave the dongle plugged in, you can switch between Police 911 and Police 911/2 by simply swapping
    CF cards.

The PCB pinout is JAMMA but the analog controls (pots for driving games mostly) connect to pins on the JAMMA connector.
The 2 outer pins of each pot connect to +5V and GND. If the direction of control is opposite to what is expected simply
reverse the wires.
The centre pin of each pot joins to the following pins on the JAMMA connector.....
Pin 25 Parts side  - GAS POT
Pin 25 Solder side - STEERING POT
Pin 26 Parts side  - HANDBRAKE POT (if used, for example Xtrial Racing)
Pin 26 Solder side - BRAKE POT

For the gun games (Jurassic Park III and Warzaid) the gun connects to the 28 way connector like this......
Pin 1 Parts side        - Gun optical input
Pin 2 Parts side        - Ground
Pin 3 Parts side        - +5V
Jamma pin 22 parts side - Gun trigger

Player 2 gun connects to the same pin numbers on the solder side.

Jurassic Park III also uses 2 additional buttons for escaping left and right. These are wired to buttons on the Jamma
connector.

Additionally on the 28-WAY connector is...
Pin 7 parts side       - Serial TX
Pin 7 solder side      - Serial RX
Pin 8 solder side      - GND (used by serial)

Pin 9 parts side       - SP_LP (outputs to SP-F, front speaker)
Pin 9 solder side      - SP_LN
Pin 9 parts side       - SP_RP (output splits into SP-BL and SP-BR, rear speaker(s))
Pin 9 solder side      - SP_RN


Measurements
------------
X1    - 33.86803MHz
X2    - 14.31700MHz
HSync - 24.48700kHz
VSync - 58.05630Hz


Additional PCBs
---------------

GQA13-PWB(D)
Copyright 2000 KONAMI
          |--------------------|
          | MB81G163222-80     |
          |               40MHz|
          |                 CN4|
          | IP90C63A           |
          |----|            CN2|
               |               |
               |               |
               |               |
               |            CN3|
               |               |
|--------------|               |
|                            |-|
|        IP90C63A            |
|                            |-|
| MB81G163222-80               |
|----------|     XC9536XL  CN1 |
           |----------------|  |
                            |  |
                            |  |
                            | *|
                            |  |
                            |  |
                            |  |
                            |  |
                            |  |
                            |  |
                            |  |
                            |CN5
                            |--|
Notes:
     This PCB is used with Mocap Golf only and drives the 2 external monitors.
     An almost identical PCB is used with Silent Scope EX but the sticker says '15KHz x2' and the
     CPLD is likely different. This most likely drives the small monitor inside the gun sight.

     XC9536XL - Xilinx XC9536 In-System Programmable CPLD stamped 'QB33A1' (PLCC44)
            * - sticker '24KHz x2'
MB81G163222-80 - Fujitsu MB81G163222-80 256k x 32-bit x 2 banks Synchronous Graphics DRAM (TQFP100)
     IP90C63A - i-Chips IP90C63A Video Controller chip (QFP144)
          CN1 - Power connector, plugs into CN13 on main board
      CN2/CN3 - Video output connector to external monitors
      CN4/CN5 - Multi-pin IDC connectors joining to main board CN15/CN16


I/O Board for Mocap Golf
----------------------------------
Board#: OMZ-3DCPU
The I/O board and hook-up is very similar to the gun board used on
House Of The Dead 2 (NAOMI).
The I/O board talks to the main board via a MAX232 TX/RX connection.
The MCU is a ROM-less NEC D784031GC.
EPROM is common 27C512 EPROM. End of the ROM shows plain text
'COPYRIGHT(C) OHMIC 1997-1998'
The rest of the board just contains lots of opamps, logic, a DC-DC converter,
resistors/caps/diodes/inductors, LM7805 5V regulator, MAX232, 2MHz XTAL,
some pots, 2-position dipsw (both on, pulls MCU pins 77 & 78 to ground),
VCO (2x BA7042), dual frequency synthesizer (BU2630) and a bunch of
connectors.
The monitor has a lot of LED sensors around the edge. The sensors
are the same type used by Sega gun games like Too Spicy, HOTD2 etc.
The sensors are linked together (like Christmas tree lights) and plug into the
I/O board. The gun/golf club plugs into the gun connectors.
There are 2 gun connectors on the board but as far as I know Mocap Golf is
a single player game, although network/e-Amusement might be possible.
The golf club acts like a LED gun. PCB power input is 12V.
*/

#include "emu.h"

#include "cpu/powerpc/ppc.h"
#include "cpu/upd78k/upd78k4.h"
#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "machine/ds2430a.h"
#include "machine/ins8250.h"
#include "machine/k056230.h"
#include "machine/lpci.h"
#include "machine/timekpr.h"
#include "machine/timer.h"
#include "sound/dmadac.h"
#include "video/voodoo_banshee.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// configurable logging
//#define LOG_WARN  (1U << 1)
#define LOG_I2C     (1U << 2)
#define LOG_IRQ     (1U << 3)
#define LOG_TIMER   (1U << 4)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGI2C(...)     LOGMASKED(LOG_I2C,     __VA_ARGS__)
#define LOGIRQ(...)     LOGMASKED(LOG_IRQ,     __VA_ARGS__)
#define LOGTIMER(...)   LOGMASKED(LOG_TIMER,   __VA_ARGS__)

namespace {

#define PCI_CLOCK           (XTAL(33'868'800))
#define SDRAM_CLOCK         (PCI_CLOCK * 3) // Main SDRAMs run at 100 MHz
#define TIMER_CLOCK         (SDRAM_CLOCK / 8)

class viper_state : public driver_device
{
public:
	viper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_voodoo(*this, "voodoo"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_duart_com(*this, "duart_com"),
		m_lanc(*this, "lanc"),
		m_ata(*this, "ata"),
		m_lpci(*this, "pcibus"),
		m_ds2430(*this, "ds2430"),
		m_ds2430_ext(*this, "ds2430_ext"),
		m_workram(*this, "workram"),
		m_io_ports(*this, "IN%u", 0U),
		m_analog_input(*this, "AN%u", 0U),
		m_gun_input(*this, "GUN%u", 0U),
		m_io_ppp_sensors(*this, "SENSOR%u", 1U),
		m_dmadac(*this, { "dacr", "dacl" })
	{
	}

	void viper(machine_config &config);
	void viper_ppp(machine_config &config);
	void viper_omz(machine_config &config);
	void viper_fullbody(machine_config &config);
	void viper_fbdongle(machine_config &config);

	void init_viper();
	void init_vipercf();
	void init_viperhd();

	int ds2430_combined_r();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<voodoo_3_device> m_voodoo;
private:
	void mpc8240_soc_map(address_map &map);

	void unk2_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t voodoo3_io_r(offs_t offset, uint64_t mem_mask = ~0);
	void voodoo3_io_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t voodoo3_r(offs_t offset, uint64_t mem_mask = ~0);
	void voodoo3_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t voodoo3_lfb_r(offs_t offset, uint64_t mem_mask = ~0);
	void voodoo3_lfb_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint8_t input_r(offs_t offset);
	void output_w(offs_t offset, uint8_t data);
	uint8_t ds2430_r();
	void ds2430_w(uint8_t data = 0);
	uint8_t ds2430_ext_r();
	void ds2430_ext_w(uint8_t data = 0);
	void unk1a_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	void unk1b_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);

	uint64_t pci_config_addr_r();
	void pci_config_addr_w(uint64_t data);
	uint64_t pci_config_data_r();
	void pci_config_data_w(uint64_t data);
	uint64_t cf_card_data_r(offs_t offset, uint64_t mem_mask = ~0);
	void cf_card_data_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t cf_card_r(offs_t offset, uint64_t mem_mask = ~0);
	void cf_card_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t ata_r(offs_t offset, uint64_t mem_mask = ~0);
	void ata_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);
	uint64_t unk_serial_r(offs_t offset, uint64_t mem_mask = ~0);
	void unk_serial_w(offs_t offset, uint64_t data, uint64_t mem_mask = ~0);

	uint16_t ppp_sensor_r(offs_t offset);

	void lanc_int(int state);
	void uart_int(int state);

	void voodoo_vblank(int state);
	void voodoo_pciint(int state);

	//the following two arrays need to stay public til the legacy PCI bus is removed
	uint32_t m_voodoo3_pci_reg[0x100];
	uint32_t m_mpc8240_regs[256/4];

	void viper_map(address_map &map);
	void viper_ppp_map(address_map &map);
	void omz3d_map(address_map &map);

	TIMER_CALLBACK_MEMBER(epic_global_timer_callback);
	TIMER_CALLBACK_MEMBER(i2c_timer_callback);

	int m_cf_card_ide = 0;
	int m_unk_serial_bit_w = 0;
	uint16_t m_unk_serial_cmd = 0U;
	uint16_t m_unk_serial_data = 0U;
	uint16_t m_unk_serial_data_r = 0U;
	uint8_t m_unk_serial_regs[0x80]{};
	uint32_t m_sound_buffer_offset = 0U;
	bool m_sound_irq_enabled = false;

	TIMER_DEVICE_CALLBACK_MEMBER(sound_timer_callback);

	// MPC8240 EPIC, to be device-ified
	enum
	{
		MPC8240_IRQ0 = 0,
		MPC8240_IRQ1,
		MPC8240_IRQ2,
		MPC8240_IRQ3,
		MPC8240_IRQ4,
		MPC8240_IRQ5,
		MPC8240_IRQ6,
		MPC8240_IRQ7,
		MPC8240_IRQ8,
		MPC8240_IRQ9 ,
		MPC8240_IRQ10,
		MPC8240_IRQ11,
		MPC8240_IRQ12,
		MPC8240_IRQ13,
		MPC8240_IRQ14,
		MPC8240_IRQ15,
		MPC8240_I2C_IRQ,
		MPC8240_DMA0_IRQ,
		MPC8240_DMA1_IRQ,
		MPC8240_MSG_IRQ,
		MPC8240_GTIMER0_IRQ,
		MPC8240_GTIMER1_IRQ,
		MPC8240_GTIMER2_IRQ,
		MPC8240_GTIMER3_IRQ,
		MPC8240_NUM_INTERRUPTS
	};

	enum
	{
		I2C_STATE_ADDRESS_CYCLE = 1,
		I2C_STATE_DATA_TRANSFER
	};

	struct MPC8240_IRQ
	{
		uint32_t vector = 0U;
		int priority = 0;
		int destination = 0;
		int active = 0;
		int pending = 0;
		int mask = 0;
	};

	struct MPC8240_GLOBAL_TIMER
	{
		uint32_t base_count = 0U;
		int enable = 0;
		emu_timer *timer = nullptr;
	};

	struct MPC8240_EPIC
	{
		uint32_t iack = 0U;
		uint32_t eicr = 0U;
		uint32_t svr = 0U;

		uint8_t pctpr = 0x0U;

		int active_irq = 0;

		MPC8240_IRQ irq[MPC8240_NUM_INTERRUPTS];

		MPC8240_GLOBAL_TIMER global_timer[4];
	};

	MPC8240_EPIC m_epic{};

	void epic_update_interrupts();
	void mpc8240_interrupt(int irq);
	void mpc8240_epic_init();
	void mpc8240_epic_reset(void);

	struct MPC8240_I2C {
		uint8_t adr = 0U;
		int fdr = 0, dffsr = 0;
		uint8_t cr = 0U;
		uint8_t sr = 0U;
		int state = 0;
		uint8_t addr_latch = 0U;
		bool rw = 0;
		emu_timer *timer = nullptr;
	};

	MPC8240_I2C m_i2c;
	uint8_t i2cdr_r(offs_t offset);
	void i2cdr_w(offs_t offset, uint8_t data);

	required_device<ppc_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<pc16552_device> m_duart_com;
	required_device<k056230_device> m_lanc;
	required_device<ata_interface_device> m_ata;
	required_device<pci_bus_legacy_device> m_lpci;
	required_device<ds2430a_device> m_ds2430;
	optional_device<ds2430a_device> m_ds2430_ext;
	required_shared_ptr<uint64_t> m_workram;
	required_ioport_array<8> m_io_ports;
	required_ioport_array<4> m_analog_input;
	required_ioport_array<4> m_gun_input;
	optional_ioport_array<4> m_io_ppp_sensors;
	required_device_array<dmadac_sound_device, 2> m_dmadac;

	uint32_t mpc8240_pci_r(int function, int reg, uint32_t mem_mask);
	void mpc8240_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
	uint32_t voodoo3_pci_r(int function, int reg, uint32_t mem_mask);
	void voodoo3_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask);
};

class viper_subscreen_state : public viper_state
{
public:
	viper_subscreen_state(const machine_config &mconfig, device_type type, const char *tag)
		: viper_state(mconfig, type, tag)
	{}

protected:
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual void video_start() override;
private:
	std::unique_ptr<bitmap_rgb32> m_voodoo_buf;
	std::unique_ptr<bitmap_rgb32> m_ttl_buf;
};

uint32_t viper_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_voodoo->update(bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}

void viper_subscreen_state::video_start()
{
	m_voodoo_buf = std::make_unique<bitmap_rgb32>(1024, 1024);
	m_ttl_buf = std::make_unique<bitmap_rgb32>(1024, 1024);
}

// TODO: multiscreen games enables TV out in Voodoo core specifically for these games,
// then drives what to actually draw thru overlay regs.
// [:voodoo] ':maincpu' (000205C8):internal_io_w(vidInFormat) = 00008000 & FFFFFFFF
// sscopex (sub screen 320x240, current m_ttl_buf cuts off picture)
// [:voodoo] ':maincpu' (0002A424):internal_io_w(vidOverlayStartCoords) = 00000000 & FFFFFFFF
// [:voodoo] ':maincpu' (0002A424):internal_io_w(vidOverlayEndScreenCoord) = 0017F3FF & FFFFFFFF
// [:voodoo] ':maincpu' (0002A424):internal_io_w(vidOverlayDudxOffsetSrcWidth) = 20000000 & FFFFFFFF
// [:voodoo] ':maincpu' (0002A424):internal_io_w(vidDesktopOverlayStride) = 00080008 & FFFFFFFF
// [:voodoo] ':maincpu' (000200DC):internal_io_w(vidOverlayStartCoords) = 0000004E & FFFFFFFF
// [:voodoo] ':maincpu' (000200DC):internal_io_w(vidOverlayEndScreenCoord) = 000FF289 & FFFFFFFF
// [:voodoo] ':maincpu' (000200DC):internal_io_w(vidOverlayDudxOffsetSrcWidth) = 11E00000 & FFFFFFFF
// [:voodoo] ':maincpu' (000200DC):internal_io_w(vidDesktopOverlayStride) = 00050008 & FFFFFFFF
// mocapglf (sub screen 512x384, ROT90 like main)
// [:voodoo] ':maincpu' (0102A4AC):internal_io_w(vidOverlayStartCoords) = 00000000 & FFFFFFFF
// [:voodoo] ':maincpu' (0102A4AC):internal_io_w(vidOverlayEndScreenCoord) = 0017F3FF & FFFFFFFF
// [:voodoo] ':maincpu' (0102A4AC):internal_io_w(vidOverlayDudxOffsetSrcWidth) = 20000000 & FFFFFFFF
// [:voodoo] ':maincpu' (0102A4AC):internal_io_w(vidDesktopOverlayStride) = 00080008 & FFFFFFFF
// [:voodoo] ':maincpu' (010200DC):internal_io_w(vidOverlayStartCoords) = 00000000 & FFFFFFFF
// [:voodoo] ':maincpu' (010200DC):internal_io_w(vidOverlayEndScreenCoord) = 0017F3FF & FFFFFFFF
// [:voodoo] ':maincpu' (010200DC):internal_io_w(vidOverlayDudxOffsetSrcWidth) = 20000000 & FFFFFFFF
// [:voodoo] ':maincpu' (010200DC):internal_io_w(vidDesktopOverlayStride) = 00080008 & FFFFFFFF
// Stereo video seems disabled (no writes to rightOverlayBuf) so a 30 Hz TTL demuxer may still be
// used here.

// TODO: we need to read the secondary TV out for nothing atm, otherwise sscopefh (at least) will hang (???)
uint32_t viper_subscreen_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_voodoo->update(screen.frame_number() & 1 ? *m_voodoo_buf : *m_ttl_buf, cliprect);

	copybitmap(bitmap, *m_voodoo_buf, 0, 0, cliprect.min_x, cliprect.min_y, cliprect);
	return 0;
}

static inline uint64_t read64be_with_32sle_device_handler(read32s_delegate handler, offs_t offset, uint64_t mem_mask)
{
	mem_mask = swapendian_int64(mem_mask);
	uint64_t result = 0;
	if (ACCESSING_BITS_0_31)
		result = (uint64_t)(handler)(offset * 2, mem_mask & 0xffffffff);
	if (ACCESSING_BITS_32_63)
		result |= (uint64_t)(handler)(offset * 2 + 1, mem_mask >> 32) << 32;
	return swapendian_int64(result);
}


static inline void write64be_with_32sle_device_handler(write32s_delegate handler, offs_t offset, uint64_t data, uint64_t mem_mask)
{
	data = swapendian_int64(data);
	mem_mask = swapendian_int64(mem_mask);
	if (ACCESSING_BITS_0_31)
		handler(offset * 2, data & 0xffffffff, mem_mask & 0xffffffff);
	if (ACCESSING_BITS_32_63)
		handler(offset * 2 + 1, data >> 32, mem_mask >> 32);
}

/*****************************************************************************/

uint32_t viper_state::mpc8240_pci_r(int function, int reg, uint32_t mem_mask)
{
	switch (reg)
	{
	}
	return m_mpc8240_regs[reg/4];
}

void viper_state::mpc8240_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_mpc8240_regs[reg/4]);
}


uint64_t viper_state::pci_config_addr_r()
{
	return m_lpci->read_64be(0, 0xffffffff00000000U);
}

void viper_state::pci_config_addr_w(uint64_t data)
{
	m_lpci->write_64be(0, data, 0xffffffff00000000U);
}

uint64_t viper_state::pci_config_data_r()
{
	return m_lpci->read_64be(1, 0x00000000ffffffffU) << 32;
}

void viper_state::pci_config_data_w(uint64_t data)
{
	m_lpci->write_64be(1, data >> 32, 0x00000000ffffffffU);
}



/*****************************************************************************/
// MPC8240 Embedded Programmable Interrupt Controller (EPIC)

// TODO: timing calculation, comes from fdr / dffsr
// most if not all games in the driver sets fdr = 0x27 = 512, dffsr = 0x21
#define I2C_TIMER_FREQ (SDRAM_CLOCK / 512) / 10

uint8_t viper_state::i2cdr_r(offs_t offset)
{
	u8 res = 0;
	if (m_i2c.cr & 0x80 && !machine().side_effects_disabled())     // only do anything if the I2C module is enabled
	{
		if (m_i2c.state == I2C_STATE_ADDRESS_CYCLE)
		{
			LOGI2C("I2C address cycle read\n");

			m_i2c.state = I2C_STATE_DATA_TRANSFER;

			m_i2c.timer->adjust(attotime::from_hz(I2C_TIMER_FREQ));
		}
		else if (m_i2c.state == I2C_STATE_DATA_TRANSFER)
		{
			LOGI2C("I2C data read\n");

			m_i2c.state = I2C_STATE_ADDRESS_CYCLE;

			// set transfer complete in status register
			m_i2c.sr |= 0x80;

			if (m_i2c.rw)
			{
				if ((m_i2c.addr_latch & 0xf0) == 0x10)
				{
					// TODO: hackish direct read
					// What should really happen here is that i2c initiates a transfer with
					// connected devices in serial form, cycling thru the various devices.
					// The hard part is to drive the adc (which has 4 write and 2 read lines)
					// with only sda/scl, and assuming it is really adc and the Guru note doesn't
					// refer to boxingm instead.

					// 0x1c: voltage, assume 5v
					if (m_i2c.addr_latch == 0x1c)
						return 0x80;
					const u16 adc_value = m_analog_input[m_i2c.addr_latch & 0x3]->read();
					// FIXME: upper nibble is currently discarded in port defs
					// is it expecting 7 bits of data and 1 of parity?
					// cfr. input tests returning different values for each nibble when both are equal.
					const u8 adc_nibble = BIT(m_i2c.addr_latch, 2) ? 0 : 8;

					res = (adc_value) >> adc_nibble;
				}
				else
					LOG("I2C: unmapped read access %02x\n", m_i2c.addr_latch);
			}
			else
				LOG("I2C: read access %02x in write mode!\n", m_i2c.addr_latch);

			// generate interrupt if interrupt are enabled
			/*if (m_i2c.cr & 0x40)
			{
			    printf("I2C interrupt\n");
			    mpc8240_interrupt(MPC8240_I2C_IRQ);

			    // set interrupt flag in status register
			    m_i2c.sr |= 0x2;
			}*/
		}
	}

	return res;
}

void viper_state::i2cdr_w(offs_t offset, uint8_t data)
{
	if (m_i2c.cr & 0x80)     // only do anything if the I2C module is enabled
	{
		if (m_i2c.state == I2C_STATE_ADDRESS_CYCLE)          // waiting for address cycle
		{
			//int rw = data & 1;
			m_i2c.rw = bool(data & 1);
			m_i2c.addr_latch = (data >> 1) & 0x7f;
			LOGI2C("I2C address cycle %s, addr = %02X \n"
				, m_i2c.rw ? "read" : "write"
				, m_i2c.addr_latch
			);
			m_i2c.state = I2C_STATE_DATA_TRANSFER;

			m_i2c.timer->adjust(attotime::from_hz(I2C_TIMER_FREQ));
		}
		else if (m_i2c.state == I2C_STATE_DATA_TRANSFER)     // waiting for data transfer
		{
			LOGI2C("I2C data transfer, data = %02x\n", data);
			m_i2c.state = I2C_STATE_ADDRESS_CYCLE;

			m_i2c.timer->adjust(attotime::from_hz(I2C_TIMER_FREQ));
		}
	}
}

TIMER_CALLBACK_MEMBER(viper_state::i2c_timer_callback)
{
	// set transfer complete in status register
	m_i2c.sr |= 0x80;

	// generate interrupt if interrupt are enabled
	if (m_i2c.cr & 0x40)
	{
		LOGI2C("I2C interrupt\n");
		mpc8240_interrupt(MPC8240_I2C_IRQ);

		// set interrupt flag in status register
		m_i2c.sr |= 0x2;
	}
}

// NOTE: swapendian_int*/8-bit ports used as a temp measure
// handling with endianness can be done thru new PCI model later on (the EPIC is natively LE)

// NOTE: not everything is "EPIC" but rather is space from the SoC that includes the EPIC.
// Also a subset of I2O/DMAC/ATU/data path diags are mappable thru a 0x1000 window PCSRBAR,
// while this full range thru EUMBBAR.
void viper_state::mpc8240_soc_map(address_map &map)
{
//  map(0x00000, 0x00fff) I2O
//  map(0x01000, 0x01fff) DMAC
//  map(0x02000, 0x02fff) ATU Address Translation Unit
	// I2C
	map(0x03000, 0x03000).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2c.adr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			LOGI2C("I2CADR %02x\n", data);
			m_i2c.adr = data;
		})
	);
	map(0x03004, 0x03004).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2c.fdr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_i2c.fdr = data & 0x3f;
			LOGI2C("I2CFDR FDR %02x\n", m_i2c.fdr);
		})
	);
	map(0x03005, 0x03005).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2c.dffsr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_i2c.dffsr = data & 0x3f;
			LOGI2C("I2CFDR DFFSR %02x\n", m_i2c.dffsr);
		})
	);
	map(0x03008, 0x03008).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2c.cr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if ((m_i2c.cr & 0x80) == 0 && (data & 0x80) != 0)
			{
				m_i2c.state = I2C_STATE_ADDRESS_CYCLE;
			}
			if ((m_i2c.cr & 0x10) != (data & 0x10))
			{
				m_i2c.state = I2C_STATE_ADDRESS_CYCLE;
			}
			m_i2c.cr = data;
			LOGI2C("I2CCR %02x\n", data);
		})
	);
	map(0x0300c, 0x0300c).lrw8(
		NAME([this] (offs_t offset) {
			return m_i2c.sr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: very wrong, only bits 4 & 2 writeable
			m_i2c.sr = data;
			LOGI2C("I2CSR %02x\n", data);
		})
	);
	map(0x03010, 0x03010).rw(FUNC(viper_state::i2cdr_r), FUNC(viper_state::i2cdr_w));
//  map(0x04000, 0x3ffff) <reserved>

//  map(0x40000, 0x7ffff) EPIC
	map(0x41030, 0x41033).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			data = swapendian_int32(data);
			m_epic.eicr = data;
			LOG("EICR %08x\n", data);
			if (BIT(data, 27))
				throw emu_fatalerror("EPIC: serial interrupts mode not implemented\n");
		})
	);
	map(0x41080, 0x41083).lr32(
		NAME([this] (offs_t offset) {
			if (!machine().side_effects_disabled())
				LOG("EVI read\n");
			// step = 1, device_id = 0, vendor_id = 0
			return swapendian_int32(0x00010000);
		})
	);
	map(0x410e0, 0x410e0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_epic.svr = data;
			LOGIRQ("SVR %02x\n", data);
		})
	);

	map(0x41110, 0x41113).select(0xc0).lw32(
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			int timer_num = offset >> 4;
			data = swapendian_int32(data);

			m_epic.global_timer[timer_num].enable = (data & 0x80000000) ? 0 : 1;
			m_epic.global_timer[timer_num].base_count = data & 0x7fffffff;

			if (m_epic.global_timer[timer_num].enable && m_epic.global_timer[timer_num].base_count > 0)
			{
				attotime timer_duration = attotime::from_hz(TIMER_CLOCK / m_epic.global_timer[timer_num].base_count);
				m_epic.global_timer[timer_num].timer->adjust(timer_duration, timer_num);

				LOGTIMER("EPIC GTIMER%d: next in %s\n", timer_num, (timer_duration / 8).as_string() );
			}
			else
			{
				m_epic.global_timer[timer_num].timer->reset();
			}
		})
	);

	map(0x41120, 0x41123).select(0xc0).lrw32(
		NAME([this] (offs_t offset) {
			u32 ret = 0;
			int timer_num = offset >> 4;

			ret |= m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].mask ? 0x80000000 : 0;
			ret |= m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].priority << 16;
			ret |= m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].vector;
			ret |= m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].active ? 0x40000000 : 0;
			return swapendian_int32(ret);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			int timer_num = offset >> 4;

			data = swapendian_int32(data);

			m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].mask = (data & 0x80000000) ? 1 : 0;
			m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].priority = (data >> 16) & 0xf;
			m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].vector = data & 0xff;

			LOGIRQ("GTVPR%d %08x\n", timer_num, data);

			if (!machine().side_effects_disabled())
				epic_update_interrupts();
		})
	);
	map(0x41130, 0x41130).select(0xc0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			int timer_num = offset >> 4;

			m_epic.irq[MPC8240_GTIMER0_IRQ + timer_num].destination = data & 0x1;

			if (data)
				throw emu_fatalerror("GTDR%d in P1 mode", timer_num, data & 1);
		})
	);

	map(0x50200, 0x50203).select(0x1e0).lrw32(
		NAME([this] (offs_t offset) {
			u32 ret = 0;
			int irq = offset >> 3;

			ret |= m_epic.irq[MPC8240_IRQ0 + irq].mask ? 0x80000000 : 0;
			ret |= m_epic.irq[MPC8240_IRQ0 + irq].priority << 16;
			ret |= m_epic.irq[MPC8240_IRQ0 + irq].vector;
			ret |= m_epic.irq[MPC8240_IRQ0 + irq].active ? 0x40000000 : 0;
			return swapendian_int32(ret);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			int irq = offset >> 3;

			data = swapendian_int32(data);
			m_epic.irq[MPC8240_IRQ0 + irq].mask = (data & 0x80000000) ? 1 : 0;
			m_epic.irq[MPC8240_IRQ0 + irq].priority = (data >> 16) & 0xf;
			m_epic.irq[MPC8240_IRQ0 + irq].vector = data & 0xff;

			LOGIRQ("IVPR%d %08x\n", irq, data);

			if (!machine().side_effects_disabled())
				epic_update_interrupts();
		})
	);
	map(0x50210, 0x50210).select(0x1e0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			int irq = offset >> 3;

			m_epic.irq[MPC8240_IRQ0 + irq].destination = data & 0x1;

			if (data)
				throw emu_fatalerror("IDR%d in P1 mode", irq, data & 1);
		})
	);
	map(0x51020, 0x51023).lrw32(
		NAME([this] (offs_t offset) {
			u32 ret = 0;

			ret |= m_epic.irq[MPC8240_I2C_IRQ].mask ? 0x80000000 : 0;
			ret |= m_epic.irq[MPC8240_I2C_IRQ].priority << 16;
			ret |= m_epic.irq[MPC8240_I2C_IRQ].vector;
			ret |= m_epic.irq[MPC8240_I2C_IRQ].active ? 0x40000000 : 0;
			return swapendian_int32(ret);
		}),
		NAME([this] (offs_t offset, u32 data, u32 mem_mask) {
			data = swapendian_int32(data);

			m_epic.irq[MPC8240_I2C_IRQ].mask = (data & 0x80000000) ? 1 : 0;
			m_epic.irq[MPC8240_I2C_IRQ].priority = (data >> 16) & 0xf;
			m_epic.irq[MPC8240_I2C_IRQ].vector = data & 0xff;

			LOGIRQ("IIVPR0 %08x\n", data);

			if (!machine().side_effects_disabled())
				epic_update_interrupts();
		})
	);
	map(0x51030, 0x51030).lw8(
		NAME([this] (offs_t, u8 data) {
			m_epic.irq[MPC8240_I2C_IRQ].destination = data & 0x1;
			if (data)
				throw emu_fatalerror("I2C IRQ in P1 mode");
			// epic_update_interrupts();
		})
	);
	map(0x60080, 0x60080).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_epic.pctpr = data & 0xf;
			epic_update_interrupts();
		})
	);
	// IACK
	map(0x600a0, 0x600a0).lr8(
		NAME([this] (offs_t offset) {
			u8 ret = 0;
			if (!machine().side_effects_disabled())
				epic_update_interrupts();
			// spurious vector register is returned if no pending interrupts
			ret = (m_epic.active_irq >= 0) ? m_epic.iack : m_epic.svr;
			return ret;
		})
	);
	// w/o strobe
	map(0x600b0, 0x600b0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			// spammy
			//LOGIRQ("EOI IRQ%d ACK\n", m_epic.active_irq);

			m_epic.irq[m_epic.active_irq].active = 0;
			m_epic.active_irq = -1;

			epic_update_interrupts();
		})
	);
//  map(0x80000, 0xfefff) <reserved>
//  map(0xff000, 0xff017) data path diags
//  map(0xff018, 0xff048) data path diags watchpoints
//  map(0xff04d, 0xfffff) <reserved>
}

TIMER_CALLBACK_MEMBER(viper_state::epic_global_timer_callback)
{
	int timer_num = param;

	if (m_epic.global_timer[timer_num].enable && m_epic.global_timer[timer_num].base_count > 0)
	{
		attotime timer_duration = attotime::from_hz(TIMER_CLOCK / m_epic.global_timer[timer_num].base_count);
		m_epic.global_timer[timer_num].timer->adjust(timer_duration, timer_num);

		LOGTIMER("EPIC GTIMER%d: next in %s\n", timer_num, timer_duration.as_string() );
	}
	else
	{
		m_epic.global_timer[timer_num].timer->reset();
	}

	mpc8240_interrupt(MPC8240_GTIMER0_IRQ + timer_num);
}


void viper_state::epic_update_interrupts()
{
	int i;

	int irq = -1;
	int priority = -1;

	// Do not change the state until current irq is fully serviced.
	if (m_epic.active_irq >= 0)
		return;

	// find the highest priority pending interrupt
	for (i=MPC8240_NUM_INTERRUPTS-1; i >= 0; i--)
	{
		if (m_epic.irq[i].pending)
		{
			// pending interrupt can only be serviced if its mask is enabled
			// and priority is above PCTPR (> 1 for Konami Viper)
			if (m_epic.irq[i].mask == 0 && m_epic.irq[i].priority > m_epic.pctpr)
			{
				if (m_epic.irq[i].priority > priority)
				{
					irq = i;
					priority = m_epic.irq[i].priority;
				}
			}
		}
	}

	if (irq >= 0 && m_epic.active_irq == -1)
	{
		m_epic.active_irq = irq;
		m_epic.irq[m_epic.active_irq].pending = 0;
		m_epic.irq[m_epic.active_irq].active = 1;

		m_epic.iack = m_epic.irq[m_epic.active_irq].vector;

		//if (irq > 4 && irq < 20)
			LOGIRQ("EPIC IRQ%d taken vector = %02X\n", irq, m_epic.iack);

		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

void viper_state::mpc8240_interrupt(int irq)
{
	m_epic.irq[irq].pending = 1;
	epic_update_interrupts();
}

void viper_state::mpc8240_epic_init()
{
	m_epic = MPC8240_EPIC();
	m_epic.global_timer[0].timer = timer_alloc(FUNC(viper_state::epic_global_timer_callback), this);
	m_epic.global_timer[1].timer = timer_alloc(FUNC(viper_state::epic_global_timer_callback), this);
	m_epic.global_timer[2].timer = timer_alloc(FUNC(viper_state::epic_global_timer_callback), this);
	m_epic.global_timer[3].timer = timer_alloc(FUNC(viper_state::epic_global_timer_callback), this);
}

void viper_state::mpc8240_epic_reset(void)
{
	int i;

	for (i=0; i < MPC8240_NUM_INTERRUPTS; i++)
	{
		m_epic.irq[i].mask = 1;
	}

	for (auto &gt : m_epic.global_timer)
	{
		gt.enable = 0;
		gt.timer->reset();
	}

	m_epic.active_irq = -1;
	m_epic.pctpr = 0xf;
}

/*****************************************************************************/


static const uint8_t cf_card_tuples[] =
{
	0x01,       // Device Tuple
	0x01,       // Tuple size
	0xd0,       // Device Type Func Spec

	0x1a,       // Config Tuple
	0xff,       // Tuple size (last?)
	0x03,       // CCR base size
	0x00,       // last config index?
	0x00, 0x01, 0x00, 0x00,     // CCR base (0x00000100)
};

uint64_t viper_state::cf_card_data_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_16_31)
	{
		switch (offset & 0xf)
		{
			case 0x8:   // Duplicate Even RD Data
			{
				r |= m_ata->cs0_r(0, mem_mask >> 16) << 16;
				break;
			}

			default:
			{
				throw emu_fatalerror("%s:cf_card_data_r: IDE reg %02X\n", machine().describe_context().c_str(), offset & 0xf);
			}
		}
	}
	return r;
}

void viper_state::cf_card_data_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		switch (offset & 0xf)
		{
			case 0x8:   // Duplicate Even RD Data
			{
				m_ata->cs0_w(0, data >> 16, mem_mask >> 16);
				break;
			}

			default:
			{
				throw emu_fatalerror("%s:cf_card_data_w: IDE reg %02X, %04X\n", machine().describe_context().c_str(), offset & 0xf, (uint16_t)(data >> 16));
			}
		}
	}
}

uint64_t viper_state::cf_card_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_16_31)
	{
		if (m_cf_card_ide)
		{
			switch (offset & 0xf)
			{
				case 0x0:   // Even RD Data
				case 0x1:   // Error
				case 0x2:   // Sector Count
				case 0x3:   // Sector No.
				case 0x4:   // Cylinder Low
				case 0x5:   // Cylinder High
				case 0x6:   // Select Card/Head
				case 0x7:   // Status
				{
					r |= m_ata->cs0_r(offset & 7, mem_mask >> 16) << 16;
					break;
				}

				//case 0x8: // Duplicate Even RD Data
				//case 0x9: // Duplicate Odd RD Data

				case 0xd:   // Duplicate Error
				{
					r |= m_ata->cs0_r(1, mem_mask >> 16) << 16;
					break;
				}
				case 0xe:   // Alt Status
				case 0xf:   // Drive Address
				{
					r |= m_ata->cs1_r(offset & 7, mem_mask >> 16) << 16;
					break;
				}

				default:
				{
					printf("%s:compact_flash_r: IDE reg %02X\n", machine().describe_context().c_str(), offset & 0xf);
				}
			}
		}
		else
		{
			int reg = offset;

			logerror("cf_r: %04X\n", reg);

			if ((reg >> 1) < sizeof(cf_card_tuples))
			{
				r |= cf_card_tuples[reg >> 1] << 16;
			}
			else
			{
				throw emu_fatalerror("%s:compact_flash_r: reg %02X\n", machine().describe_context().c_str(), reg);
			}
		}
	}
	return r;
}

void viper_state::cf_card_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	//logerror("%s:compact_flash_w: %08X%08X, %08X, %08X%08X\n", machine().describe_context(), (uint32_t)(data>>32), (uint32_t)(data), offset, (uint32_t)(mem_mask >> 32), (uint32_t)(mem_mask));

	if (ACCESSING_BITS_16_31)
	{
		if (offset < 0x10)
		{
			switch (offset & 0xf)
			{
				case 0x0:   // Even WR Data
				case 0x1:   // Features
				case 0x2:   // Sector Count
				case 0x3:   // Sector No.
				case 0x4:   // Cylinder Low
				case 0x5:   // Cylinder High
				case 0x6:   // Select Card/Head
				case 0x7:   // Command
				{
					m_ata->cs0_w(offset & 7, data >> 16, mem_mask >> 16);
					break;
				}

				//case 0x8: // Duplicate Even WR Data
				//case 0x9: // Duplicate Odd WR Data

				case 0xd:   // Duplicate Features
				{
					m_ata->cs0_w(1, data >> 16, mem_mask >> 16);
					break;
				}
				case 0xe:   // Device Ctl
				case 0xf:   // Reserved
				{
					m_ata->cs1_w(offset & 7, data >> 16, mem_mask >> 16);
					break;
				}

				default:
				{
					throw emu_fatalerror("%s:compact_flash_w: IDE reg %02X, data %04X\n", machine().describe_context().c_str(), offset & 0xf, (uint16_t)((data >> 16) & 0xffff));
				}
			}
		}
		else if (offset >= 0x100)
		{
			switch (offset)
			{
				case 0x100:
				{
					if ((data >> 16) & 0x80)
					{
						m_cf_card_ide = 1;

						m_ata->reset();
					}
					break;
				}
				default:
				{
					throw emu_fatalerror("%s:compact_flash_w: reg %02X, data %04X\n", machine().describe_context().c_str(), offset, (uint16_t)((data >> 16) & 0xffff));
				}
			}
		}
	}
}

void viper_state::unk2_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_56_63)
	{
		m_cf_card_ide = 0;
	}
}




uint64_t viper_state::ata_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;

	if (ACCESSING_BITS_16_31)
	{
		int reg = (offset >> 4) & 0x7;

		switch(offset & 0x80)
		{
		case 0x00:
			r |= m_ata->cs0_r(reg, mem_mask >> 16) << 16;
			break;
		case 0x80:
			r |= m_ata->cs1_r(reg, mem_mask >> 16) << 16;
			break;
		}
	}

	return r;
}

void viper_state::ata_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		int reg = (offset >> 4) & 0x7;

		switch(offset & 0x80)
		{
		case 0x00:
			m_ata->cs0_w(reg, data >> 16, mem_mask >> 16);
			break;
		case 0x80:
			m_ata->cs1_w(reg, data >> 16, mem_mask >> 16);
			break;
		}
	}
}

uint32_t viper_state::voodoo3_pci_r(int function, int reg, uint32_t mem_mask)
{
	switch (reg)
	{
		case 0x00:      // PCI Vendor ID (0x121a = 3dfx), Device ID (0x0005 = Voodoo 3)
		{
			return 0x0005121a;
		}
		case 0x08:      // Device class code
		{
			return 0x03000000;
		}
		case 0x10:      // memBaseAddr0
		{
			return m_voodoo3_pci_reg[0x10/4];
		}
		case 0x14:      // memBaseAddr1
		{
			return m_voodoo3_pci_reg[0x14/4];
		}
		case 0x18:      // memBaseAddr1
		{
			return m_voodoo3_pci_reg[0x18/4];
		}
		case 0x40:      // fabId
		{
			return m_voodoo3_pci_reg[0x40/4];
		}
		case 0x50:      // cfgScratch
		{
			return m_voodoo3_pci_reg[0x50/4];
		}

		default:
			throw emu_fatalerror("voodoo3_pci_r: %08X at %08X\n", reg, m_maincpu->pc());
	}
}

void viper_state::voodoo3_pci_w(int function, int reg, uint32_t data, uint32_t mem_mask)
{
//  printf("voodoo3_pci_w: %08X, %08X\n", reg, data);

	switch (reg)
	{
		case 0x04:      // Command register
		{
			m_voodoo3_pci_reg[0x04/4] = data;
			break;
		}
		case 0x10:      // memBaseAddr0
		{
			if (data == 0xffffffff)
			{
				m_voodoo3_pci_reg[0x10/4] = 0xfe000000;
			}
			else
			{
				m_voodoo3_pci_reg[0x10/4] = data;
			}
			break;
		}
		case 0x14:      // memBaseAddr1
		{
			if (data == 0xffffffff)
			{
				m_voodoo3_pci_reg[0x14/4] = 0xfe000008;
			}
			else
			{
				m_voodoo3_pci_reg[0x14/4] = data;
			}
			break;
		}
		case 0x18:      // ioBaseAddr
		{
			if (data == 0xffffffff)
			{
				m_voodoo3_pci_reg[0x18/4] = 0xffffff01;
			}
			else
			{
				m_voodoo3_pci_reg[0x18/4] = data;
			}
			break;
		}
		case 0x3c:      // InterruptLine
		{
			break;
		}
		case 0x40:      // fabId
		{
			m_voodoo3_pci_reg[0x40/4] = data;
			break;
		}
		case 0x50:      // cfgScratch
		{
			m_voodoo3_pci_reg[0x50/4] = data;
			break;
		}

		default:
			throw emu_fatalerror("voodoo3_pci_w: %08X, %08X at %08X\n", data, reg, m_maincpu->pc());
	}
}

uint64_t viper_state::voodoo3_io_r(offs_t offset, uint64_t mem_mask)
{
	return read64be_with_32sle_device_handler(read32s_delegate(*m_voodoo, FUNC(voodoo_3_device::read_io)), offset, mem_mask);
}
void viper_state::voodoo3_io_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
//  printf("voodoo3_io_w: %08X%08X, %08X at %08X\n", (uint32_t)(data >> 32), (uint32_t)(data), offset, m_maincpu->pc());

	write64be_with_32sle_device_handler(write32s_delegate(*m_voodoo, FUNC(voodoo_3_device::write_io)), offset, data, mem_mask);
}

uint64_t viper_state::voodoo3_r(offs_t offset, uint64_t mem_mask)
{
	return read64be_with_32sle_device_handler(read32s_delegate(*m_voodoo, FUNC(voodoo_3_device::read)), offset, mem_mask);
}
void viper_state::voodoo3_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
//  printf("voodoo3_w: %08X%08X, %08X at %08X\n", (uint32_t)(data >> 32), (uint32_t)(data), offset, m_maincpu->pc());

	write64be_with_32sle_device_handler(write32s_delegate(*m_voodoo, FUNC(voodoo_3_device::write)), offset, data, mem_mask);
}

uint64_t viper_state::voodoo3_lfb_r(offs_t offset, uint64_t mem_mask)
{
	return read64be_with_32sle_device_handler(read32s_delegate(*m_voodoo, FUNC(voodoo_3_device::read_lfb)), offset, mem_mask);
}
void viper_state::voodoo3_lfb_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
//  printf("voodoo3_lfb_w: %08X%08X, %08X at %08X\n", (uint32_t)(data >> 32), (uint32_t)(data), offset, m_maincpu->pc());

	write64be_with_32sle_device_handler(write32s_delegate(*m_voodoo, FUNC(voodoo_3_device::write_lfb)), offset, data, mem_mask);
}


uint8_t viper_state::input_r(offs_t offset)
{
#if 0
	uint64_t r = 0;
	//return 0;//0x0000400000000000U;

	r |= 0xffff00000000ffffU;

	if (ACCESSING_BITS_40_47)
	{
		uint64_t reg = 0;
		reg |= (m_ds2430->data_r() << 5);
		reg |= 0x40;        // if this bit is 0, loads a disk copier instead
		//r |= 0x04;    // screen flip
		reg |= 0x08;      // memory card check (1 = enable)

		r |= reg << 40;

		//r |= (uint64_t)(m_ds2430->data_r() << 5) << 40;
		//r |= 0x0000400000000000U;

		//r |= 0x0000040000000000U; // screen flip
		//r |= 0x0000080000000000U; // memory card check (1 = enable)
	}
	if (ACCESSING_BITS_32_39)
	{
		uint64_t reg = ioport("IN0")->read();
		r |= reg << 32;
	}
	if (ACCESSING_BITS_24_31)
	{
		uint64_t reg = ioport("IN1")->read();
		r |= reg << 24;
	}
	if (ACCESSING_BITS_16_23)
	{
		uint64_t reg = 0;
		//reg |= 0x80;                  // memory card check for boxingm
		//reg |= 0x40;                  // memory card check for tsurugi
		reg |= 0x3f;

		r |= reg << 16;
	}

	return r;
#else
	return (m_io_ports[offset & 7])->read();
#endif
}

void viper_state::output_w(offs_t offset, uint8_t data)
{
	/*
	 * -11- ---- always enabled, bit 6 first then bit 5 (sound engine control?)
	 * ---1 ---- enabled in tsurugi/mocapglf
	 * ---- x--- output 1
	 *           \- start button lamp for sscopex
	 *           \- rotating light for mocapb
	 * ---- -x-- output 0
	 *           \- start button lamp for mocapglf
	 *           \- coin lockout for mfightc
	 *           \- scope enable for sscopex
	 *           \- start button lamp for mocapb
	 * ---- --xx coin counters
	 *           \- sscopefh sends signals depending on the coin type
	 */
	if (offset == 0)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
		machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
		m_sound_irq_enabled = bool(BIT(data, 5));
		return;
	}
	LOG("output_w %02x -> %02x\n", offset, data);
}

uint8_t viper_state::ds2430_r()
{
	if (!machine().side_effects_disabled())
		m_ds2430->data_w(0);

	return 0;
}

void viper_state::ds2430_w(uint8_t data)
{
	m_ds2430->data_w(1);
}

uint8_t viper_state::ds2430_ext_r()
{
	if (m_ds2430_ext.found() && !machine().side_effects_disabled())
		m_ds2430_ext->data_w(0);

	return 0;
}

void viper_state::ds2430_ext_w(uint8_t data)
{
	if (m_ds2430_ext.found())
		m_ds2430_ext->data_w(1);
}

void viper_state::unk1a_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_56_63)
	{
	//  printf("%s unk1a_w: %08X%08X, %08X (mask %08X%08X) at %08X\n", machine().describe_context().c_str(), (uint32_t)(data >> 32), (uint32_t)data, offset, (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
	}
}

void viper_state::unk1b_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_56_63)
	{
		// HACK: put DS2430A in reset state (probably side effect of enabling initial output on a GPIO pin)
		m_ds2430->data_w(0);
		if (m_ds2430_ext.found())
			m_ds2430_ext->data_w(0);
	//  printf("%s unk1b_w: %08X%08X, %08X (mask %08X%08X) at %08X\n", machine().describe_context().c_str(), (uint32_t)(data >> 32), (uint32_t)data, offset, (uint32_t)(mem_mask >> 32), (uint32_t)mem_mask);
	}
}

uint64_t viper_state::unk_serial_r(offs_t offset, uint64_t mem_mask)
{
	uint64_t r = 0;
	if (ACCESSING_BITS_16_31)
	{
		int bit = m_unk_serial_data_r & 0x1;
		m_unk_serial_data_r >>= 1;
		r |= bit << 17;
	}
	return r;
}

void viper_state::unk_serial_w(offs_t offset, uint64_t data, uint64_t mem_mask)
{
	if (ACCESSING_BITS_16_31)
	{
		if (data & 0x10000)
		{
			int bit = (data & 0x20000) ? 1 : 0;
			if (m_unk_serial_bit_w < 8)
			{
				if (m_unk_serial_bit_w > 0)
					m_unk_serial_cmd <<= 1;
				m_unk_serial_cmd |= bit;
			}
			else
			{
				if (m_unk_serial_bit_w > 8)
					m_unk_serial_data <<= 1;
				m_unk_serial_data |= bit;
			}
			m_unk_serial_bit_w++;

			if (m_unk_serial_bit_w == 8)
			{
				if ((m_unk_serial_cmd & 0x80) == 0)     // register read
				{
					int reg = m_unk_serial_cmd & 0x7f;
					uint8_t data = m_unk_serial_regs[reg];

					m_unk_serial_data_r = ((data & 0x1) << 7) | ((data & 0x2) << 5) | ((data & 0x4) << 3) | ((data & 0x8) << 1) | ((data & 0x10) >> 1) | ((data & 0x20) >> 3) | ((data & 0x40) >> 5) | ((data & 0x80) >> 7);

					logerror("unk_serial read reg %02X: %04X\n", reg, data);
				}
			}
			if (m_unk_serial_bit_w == 16)
			{
				if (m_unk_serial_cmd & 0x80)                // register write
				{
					int reg = m_unk_serial_cmd & 0x7f;
					m_unk_serial_regs[reg] = m_unk_serial_data;
					logerror("unk_serial write reg %02X: %04X\n", reg, m_unk_serial_data);
				}

				m_unk_serial_bit_w = 0;
				m_unk_serial_cmd = 0;
				m_unk_serial_data = 0;
			}
		}
	}
}

/*****************************************************************************/

void viper_state::viper_map(address_map &map)
{
//  map.unmap_value_high();
	map(0x00000000, 0x00ffffff).mirror(0x1000000).ram().share("workram");
	map(0x80000000, 0x800fffff).m(*this, FUNC(viper_state::mpc8240_soc_map));
	map(0x82000000, 0x83ffffff).rw(FUNC(viper_state::voodoo3_r), FUNC(viper_state::voodoo3_w));
	map(0x84000000, 0x85ffffff).rw(FUNC(viper_state::voodoo3_lfb_r), FUNC(viper_state::voodoo3_lfb_w));
	// I/O space, Voodoo 3 sets 0x00800001 as BAR2
	map(0xfe800000, 0xfe8000ff).rw(FUNC(viper_state::voodoo3_io_r), FUNC(viper_state::voodoo3_io_w));
	map(0xfec00000, 0xfedfffff).rw(FUNC(viper_state::pci_config_addr_r), FUNC(viper_state::pci_config_addr_w));
	map(0xfee00000, 0xfeefffff).rw(FUNC(viper_state::pci_config_data_r), FUNC(viper_state::pci_config_data_w));
	// 0xff000000, 0xff000fff - cf_card_data_r/w (installed in DRIVER_INIT(vipercf))
	// 0xff200000, 0xff200fff - cf_card_r/w (installed in DRIVER_INIT(vipercf))
	// 0xff300000, 0xff300fff - ata_r/w (installed in DRIVER_INIT(viperhd))
//  map(0xff400xxx, 0xff400xxx) ppp2nd sense device
	map(0xffe00000, 0xffe0000f).rw(m_duart_com, FUNC(pc16552_device::read), FUNC(pc16552_device::write));
	map(0xffe08000, 0xffe08007).nopw(); // timestamp? watchdog?
	map(0xffe10000, 0xffe10007).rw(FUNC(viper_state::input_r), FUNC(viper_state::output_w));
	map(0xffe20000, 0xffe20007).nopw(); // motor k-type for deluxe force feedback (xtrial, gticlub2, jpark3)
	map(0xffe28000, 0xffe28007).nopw(); // ppp2nd/boxingm extended leds
	map(0xffe28000, 0xffe28007).nopr(); // sscopex busy flag for secondary screen?
	// boxingm reads and writes here to read the pad sensor values, 2nd adc?
	// $10 bit 7 (w) clk_write, $18 bit 7 (r) do_read
//  map(0xffe28008, 0xffe2801f).noprw();
	map(0xffe30000, 0xffe31fff).rw("m48t58", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0xffe40000, 0xffe40007).noprw(); // JTAG? 0x00 on normal operation, other values on POST,
										 // 0xa8/0xa9 for unexpected irq (namely irq1)
	map(0xffe50000, 0xffe50007).w(FUNC(viper_state::unk2_w));
	map(0xffe60000, 0xffe60007).noprw();
	map(0xffe70000, 0xffe70000).rw(FUNC(viper_state::ds2430_r), FUNC(viper_state::ds2430_w));
	map(0xffe78000, 0xffe78000).rw(FUNC(viper_state::ds2430_ext_r), FUNC(viper_state::ds2430_ext_w));
	map(0xffe80000, 0xffe80007).w(FUNC(viper_state::unk1a_w));
	map(0xffe88000, 0xffe88007).w(FUNC(viper_state::unk1b_w));
	map(0xffe98000, 0xffe98007).m(m_lanc, FUNC(k056230_viper_device::regs_map));
	map(0xffe9a000, 0xffe9bfff).rw(m_lanc, FUNC(k056230_viper_device::ram_r), FUNC(k056230_viper_device::ram_w));
	map(0xffea0000, 0xffea0007).lr8(
		NAME([this] (offs_t offset) {
			const u8 res = m_gun_input[offset >> 1]->read() >> ((offset & 1) ? 0 : 8);
			return res;
		})
	).nopw(); // Gun sensor? Read heavily by p9112
	map(0xffea8000, 0xffea8007).nopw(); // sound DMA trigger for block request?
	map(0xfff00000, 0xfff3ffff).rom().region("user1", 0);       // Boot ROM
}

void viper_state::viper_ppp_map(address_map &map)
{
	viper_map(map);
	map(0xff400108, 0xff40012f).nopw(); // ppp2nd lamps
	map(0xff400200, 0xff40023f).r(FUNC(viper_state::ppp_sensor_r));
}

/*****************************************************************************/

int viper_state::ds2430_combined_r()
{
	// Inactive DS2430A is held in reset state
	return m_ds2430->data_r() || m_ds2430_ext->data_r();
}

static INPUT_PORTS_START( viper )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP2" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP1" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("ds2430", ds2430a_device, data_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // if this bit is 0, loads a disk copier instead
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x20, 0x20, "3-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "3-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "3-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "4-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "4-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "4-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "4-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// following bits controls screen mux in Mocap Golf?
	PORT_DIPNAME( 0x10, 0x10, "4-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "4-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "4-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "4-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "5-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "5-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "5-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "5-3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "5-4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "5-5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "5-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "5-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUN3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN3")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( ppp2nd )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x01, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DIP3" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DIP2" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DIP1" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("OK Button")

	PORT_MODIFY("IN4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Button")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Button")

	PORT_MODIFY("IN5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // another OK button

	PORT_START("SENSOR1")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_BUTTON3 )     // Sensor 0, 1, 2  (Sensor bar 1)
	PORT_BIT( 0x0038, IP_ACTIVE_HIGH, IPT_BUTTON4 )     // Sensor 3, 4, 5  (Sensor bar 2)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_BUTTON5 )     // Sensor 6, 7, 8  (Sensor bar 3)

	PORT_START("SENSOR2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON5 )     // Sensor 6, 7, 8  (Sensor bar 3)
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_BUTTON6 )     // Sensor 9, 10,11 (Sensor bar 4)

	PORT_START("SENSOR3")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_BUTTON7 )     // Sensor 12,13,14 (Sensor bar 5)
	PORT_BIT( 0x0038, IP_ACTIVE_HIGH, IPT_BUTTON8 )     // Sensor 15,16,17 (Sensor bar 6)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_BUTTON9 )     // Sensor 18,19,20 (Sensor bar 7)

	PORT_START("SENSOR4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 )     // Sensor 18,19,20 (Sensor bar 7)
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_BUTTON10 )    // Sensor 21,22,23 (Sensor bar 8)
INPUT_PORTS_END

INPUT_PORTS_START( thrild2 )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x00, "Calibrate Controls On Boot" ) PORT_DIPLOCATION("SW:2") // Game crashes during boot when this is on
	PORT_DIPSETTING( 0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shift Down")

	PORT_MODIFY("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shift Up")

	// TODO: normal type steering wheel (non-K type)
	PORT_MODIFY("AN0")
	PORT_BIT( 0xfff, 0x000, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x800,0x7ff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Gas Pedal") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE

	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_NAME("Brake Pedal") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE
INPUT_PORTS_END

INPUT_PORTS_START( gticlub2 )
	PORT_INCLUDE( thrild2 )

	// K-Type steering wheel
	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_REVERSE

	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL3 ) PORT_NAME("Handbrake Lever") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_REVERSE
INPUT_PORTS_END

INPUT_PORTS_START( gticlub2ea )
	PORT_INCLUDE( gticlub2 )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x02, 0x00, "DIP3" ) PORT_DIPLOCATION("SW:3") // This needs to be on or it asks for a password, parent doesn't care
	PORT_DIPSETTING( 0x02, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

INPUT_PORTS_START( boxingm )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x04, "Calibrate Pads On Boot" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )

	PORT_MODIFY("IN4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select L")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select R")
	// as attract claims, following two are for standing up on KO count
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("BodyPad L")

	PORT_MODIFY("IN5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("BodyPad R")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // memory card check for boxingm (actually comms enable?)

	// TODO: non-i2c analog ports
INPUT_PORTS_END

INPUT_PORTS_START( jpark3 )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("IN4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Gun Trigger") PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Gun Trigger") PORT_PLAYER(2)

	PORT_MODIFY("IN5")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Right Escape button")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Left Escape button")

	PORT_MODIFY("GUN0")
	PORT_BIT( 0x07ff, 0x2f8, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x00e0, 0x0510 ) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_MODIFY("GUN1")
	PORT_BIT( 0x01ff, 0x0e7, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x0020, 0x01af) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_MODIFY("GUN2")
	PORT_BIT( 0x07ff, 0x2f8, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x00e0, 0x0510 ) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(2)

	PORT_MODIFY("GUN3")
	PORT_BIT( 0x01ff, 0x0e7, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0x0020, 0x01af) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(2)
INPUT_PORTS_END

INPUT_PORTS_START( p911 )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x04, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gun Trigger")

	PORT_MODIFY("IN5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // P2 SHT2 (checks and fails serial if pressed)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: corrupted GFXs on calibration screen
	PORT_MODIFY("GUN0")
	PORT_BIT( 0x07ff, 0x2f8, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX( 0x00e0, 0x0510 ) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_MODIFY("GUN1")
	PORT_BIT( 0x01ff, 0x0e7, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x0020, 0x01af) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_MODIFY("GUN2")
	PORT_BIT( 0x07ff, 0x2f8, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX( 0x00e0, 0x0510 ) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(2)

	PORT_MODIFY("GUN3")
	PORT_BIT( 0x01ff, 0x0e7, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x0020, 0x01af) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_PLAYER(2)
INPUT_PORTS_END

INPUT_PORTS_START( p9112 )
	PORT_INCLUDE( p911 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(viper_state, ds2430_combined_r)
INPUT_PORTS_END

INPUT_PORTS_START( mfightc )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x01, DEF_STR( No ) )

	PORT_MODIFY("IN4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // If off, will get stuck after RTC OK

	PORT_MODIFY("IN5")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // if off tries to check UART

	// TODO: touchscreen
INPUT_PORTS_END

INPUT_PORTS_START( mocapglf )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Select Up")

	PORT_MODIFY("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Select Down")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select Left")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select Right")
	PORT_DIPNAME( 0x40, 0x40, "Show Diagnostics On Boot" ) // Shows UART status, lamp status, and accelerometer values
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x40, DEF_STR( No ) )

	// TODO: placeholder, can be tested thru I/O check -> G-Sensor check
	// (is there missing GFXs for pitch/roll angle displays?)
	PORT_MODIFY("GUN0")
	PORT_BIT( 0xffff, 0x8000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0xffff ) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_NAME("Pitch Angle X")

	PORT_MODIFY("GUN1")
	PORT_BIT( 0xffff, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x0000, 0xffff) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_NAME("Pitch Angle Y")

	PORT_MODIFY("GUN2")
	PORT_BIT( 0xffff, 0x8000, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0xffff ) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_NAME("Roll Angle X")

	PORT_MODIFY("GUN3")
	PORT_BIT( 0xffff, 0x8000, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX( 0x0000, 0xffff ) PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_NAME("Roll Angle Y")
INPUT_PORTS_END

INPUT_PORTS_START( mocapb )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Left Button")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Right Button")

	PORT_MODIFY("IN5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // P2 SHT2 (checks and fails serial if pressed)

	// TODO: placeholders, not really IPT_PEDALs and not really PORT_PLAYER(2)
	PORT_MODIFY("AN0")
	PORT_BIT( 0xfff, 0x00, IPT_PEDAL ) PORT_NAME("Left Glove Rear-Front") PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE

	PORT_MODIFY("AN1")
	PORT_BIT( 0xfff, 0x00, IPT_PEDAL2 ) PORT_NAME("Left Glove Left-Right") PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE
	// TODO: Left Glove Bottom-Top

	PORT_MODIFY("AN2")
	PORT_BIT( 0xfff, 0x00, IPT_PEDAL ) PORT_NAME("Right Glove Rear-Front") PORT_MINMAX(0x00,0xfff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE PORT_PLAYER(2)

	PORT_MODIFY("AN3")
	PORT_BIT( 0xfff, 0x00, IPT_PEDAL2 ) PORT_NAME("Right Glove Left-Right") PORT_MINMAX(0x00,0xfff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE PORT_PLAYER(2)
	// TODO: Right Glove Bottom-Top
INPUT_PORTS_END

INPUT_PORTS_START( sscopefh )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x00, "DIP2" ) PORT_DIPLOCATION("SW:2") // Without this switched on, the screen will be static
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Refill Key")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Hopper") // causes hopper errors if pressed, TBD

	PORT_MODIFY("IN4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Credit 2 Pounds") // Currency probably changes between regions
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Credit 1 Pound") // Can be used in refill mode to insert coins into the hopper
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Credit 0.50 Pounds")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gun Trigger")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_NAME("Credit 0.20 Pounds")

	PORT_MODIFY("IN5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // P2 SHT2 (checks and fails serial if pressed)
INPUT_PORTS_END

INPUT_PORTS_START( sogeki )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN3")
	PORT_DIPNAME( 0x20, 0x00, "Cabinet Type" ) // must stay on E-Amusement for game to boot
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "E-Amusement" )

	PORT_MODIFY("IN4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gun Trigger")
INPUT_PORTS_END

INPUT_PORTS_START( sscopex )
	PORT_INCLUDE( sogeki )

	PORT_MODIFY("IN3")
	PORT_DIPNAME( 0x20, 0x20, "Cabinet Type" ) // must stay on Normal for game to boot
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "E-Amusement" )
INPUT_PORTS_END

INPUT_PORTS_START( tsurugi )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN4")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shot Button")

	PORT_MODIFY("IN5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Foot Pedal")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // deluxe ID? memory card check?
												// if off tries to check UART & "lampo"/bleeder at POST
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Sensor Grip")
INPUT_PORTS_END

INPUT_PORTS_START( wcombat )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Mirror Screen X" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x02, DEF_STR( No ) )
	PORT_DIPNAME( 0x04, 0x04, "Mirror Screen Y" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x00, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x04, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("IN4")
	PORT_DIPNAME( 0x04, 0x04, "Diag test enable" ) // possibly PL1 IPT_BUTTON4 sense
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // X flip screen

	// TODO: whatever it reads from the i2c analog ports (needs service mode)
INPUT_PORTS_END

// twin cab version?
INPUT_PORTS_START( wcombatj )
	PORT_INCLUDE( wcombat )

	// TODO: check if DIP2 ID selects side as stated by the manual

	// Specifically works in this version only
	PORT_MODIFY("IN5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

INPUT_PORTS_START( xtrial )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x04, 0x00, "Calibrate Controls On Boot" ) PORT_DIPLOCATION("SW:2") // Game crashes during boot when this is on
	PORT_DIPSETTING( 0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1") // Crashes at 45% when card checks are enabled
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shift Down")

	PORT_MODIFY("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shift Up")

	// virtually identical to gticlub
	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_REVERSE

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Gas Pedal") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE

	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_NAME("Brake Pedal") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE

	PORT_MODIFY("AN3")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL3 ) PORT_NAME("Handbrake Lever") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(100) PORT_KEYDELTA(25) PORT_REVERSE
INPUT_PORTS_END

INPUT_PORTS_START( code1d )
	PORT_INCLUDE( viper )

	PORT_MODIFY("IN2")
	// Unknown, but without this set the game won't display anything besides a blue screen
	PORT_DIPNAME( 0x01, 0x00, "DIP4" ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING( 0x01, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	// needs it to be on otherwise analog inputs won't work in gameplay
	PORT_DIPNAME( 0x04, 0x04, "Calibrate Controls On Boot" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING( 0x04, DEF_STR( Yes ) )
	PORT_DIPSETTING( 0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x08, 0x00, "Memory Card Check On Boot" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING( 0x08, DEF_STR( On ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )

	PORT_MODIFY("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Action Button")

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_NAME("Steering Wheel") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_REVERSE

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_NAME("Gas Pedal") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE

	PORT_MODIFY("AN2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_NAME("Brake Pedal") PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_REVERSE
INPUT_PORTS_END

/*****************************************************************************/

void viper_state::lanc_int(int state)
{
	if (state)
		mpc8240_interrupt(MPC8240_IRQ1);
}

void viper_state::uart_int(int state)
{
	if (state)
		mpc8240_interrupt(MPC8240_IRQ2);
}

void viper_state::voodoo_vblank(int state)
{
	if (state)
		mpc8240_interrupt(MPC8240_IRQ0);
}

void viper_state::voodoo_pciint(int state)
{
	if (state)
		mpc8240_interrupt(MPC8240_IRQ4);
}

TIMER_DEVICE_CALLBACK_MEMBER(viper_state::sound_timer_callback)
{
	if (!m_sound_irq_enabled)
	{
		// If IRQ3 is triggered too soon into the boot process then it'll freeze on the blue boot screen.
		return;
	}

	mpc8240_interrupt(MPC8240_IRQ3);

	// Get samples from memory
	int32_t* samplePtr = (int32_t*)(m_workram + (m_sound_buffer_offset >> 3));
	for (int i = 0; i < 2; i++)
	{
		m_dmadac[i]->transfer(
			i,
			1,
			2,
			0x800 / 4 / 2, // Each buffer is 0x800 bytes in size, containing stereo 32-bit audio
			samplePtr
		);
	}

	m_sound_buffer_offset ^= 0x800;
}

void viper_state::machine_start()
{
	mpc8240_epic_init();

	m_i2c.timer = timer_alloc(FUNC(viper_state::i2c_timer_callback), this);

	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x00ffffff, false, m_workram);

	save_item(NAME(m_voodoo3_pci_reg));
	save_item(NAME(m_mpc8240_regs));
	save_item(NAME(m_cf_card_ide));
	save_item(NAME(m_unk_serial_bit_w));
	save_item(NAME(m_unk_serial_cmd));
	save_item(NAME(m_unk_serial_data));
	save_item(NAME(m_unk_serial_data_r));
	save_item(NAME(m_unk_serial_regs));
	save_item(NAME(m_sound_buffer_offset));
	save_item(NAME(m_sound_irq_enabled));

	save_item(NAME(m_epic.iack));
	save_item(NAME(m_epic.eicr)); // written but never used
	save_item(NAME(m_epic.svr));
	save_item(NAME(m_epic.active_irq));

	save_item(NAME(m_i2c.adr));
	save_item(NAME(m_i2c.fdr));
	save_item(NAME(m_i2c.dffsr));
	save_item(NAME(m_i2c.cr));
	save_item(NAME(m_i2c.sr));
	save_item(NAME(m_i2c.state));

	save_item(STRUCT_MEMBER(m_epic.irq, vector));
	save_item(STRUCT_MEMBER(m_epic.irq, priority));
	save_item(STRUCT_MEMBER(m_epic.irq, destination)); // written but never read
	save_item(STRUCT_MEMBER(m_epic.irq, active));
	save_item(STRUCT_MEMBER(m_epic.irq, pending));
	save_item(STRUCT_MEMBER(m_epic.irq, mask));

	save_item(STRUCT_MEMBER(m_epic.global_timer, base_count));
	save_item(STRUCT_MEMBER(m_epic.global_timer, enable));

	m_unk_serial_bit_w = 0;
	std::fill(std::begin(m_unk_serial_regs), std::end(m_unk_serial_regs), 0);

	std::fill(std::begin(m_voodoo3_pci_reg), std::end(m_voodoo3_pci_reg), 0);
	std::fill(std::begin(m_mpc8240_regs), std::end(m_mpc8240_regs), 0);
}

void viper_state::machine_reset()
{
	mpc8240_epic_reset();

	m_i2c.state = I2C_STATE_ADDRESS_CYCLE;
	m_i2c.timer->reset();

	ide_hdd_device *hdd = m_ata->subdevice<ata_slot_device>("0")->subdevice<ide_hdd_device>("hdd");
	uint16_t *identify_device = hdd->identify_device_buffer();

	// Viper expects these settings or the BIOS fails
	identify_device[51] = 0x0200;           /* 51: PIO data transfer cycle timing mode */
	identify_device[67] = 0x00f0;           /* 67: minimum PIO transfer cycle time without flow control */

	m_sound_buffer_offset = 0xfff800; // The games swap between 0xfff800 and 0xfff000 every IRQ3 call
	m_sound_irq_enabled = false;

	for (int i = 0; i < 2; i++)
	{
		m_dmadac[i]->initialize_state();
		m_dmadac[i]->set_frequency(44100);
		m_dmadac[i]->enable(1);
	}

	m_ds2430->data_w(1);
	if (m_ds2430_ext.found())
		m_ds2430_ext->data_w(1);
}

void viper_state::viper(machine_config &config)
{
	/* basic machine hardware */
	MPC8240(config, m_maincpu, PCI_CLOCK * 6); // 200 Mhz
	m_maincpu->set_bus_frequency(PCI_CLOCK * 2); // TODO: x2 for AGP, Epic gets x1
	m_maincpu->set_addrmap(AS_PROGRAM, &viper_state::viper_map);

	DS2430A(config, m_ds2430);

	pci_bus_legacy_device &pcibus(PCI_BUS_LEGACY(config, "pcibus", 0, 0));
	pcibus.set_device( 0, FUNC(viper_state::mpc8240_pci_r), FUNC(viper_state::mpc8240_pci_w));
	pcibus.set_device(12, FUNC(viper_state::voodoo3_pci_r), FUNC(viper_state::voodoo3_pci_w));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, true);

	PC16552D(config, "duart_com", 0);
	// TODO: unverified clocks and channel types, likely connects to sensor motion based games
	NS16550(config, "duart_com:chan0", XTAL(19'660'800));
	NS16550(config, "duart_com:chan1", XTAL(19'660'800)).out_int_callback().set(FUNC(viper_state::uart_int));

	K056230_VIPER(config, m_lanc);
	m_lanc->irq_cb().set(FUNC(viper_state::lanc_int));

	VOODOO_3(config, m_voodoo, voodoo_3_device::NOMINAL_CLOCK);
	m_voodoo->set_fbmem(8); // TODO: should be 16, implement VMI_DATA_5 strapping pin in Voodoo 3 core instead
	m_voodoo->set_screen("screen");
	m_voodoo->set_cpu("maincpu");
	m_voodoo->set_status_cycles(1000); // optimization to consume extra cycles when polling status
	m_voodoo->vblank_callback().set(FUNC(viper_state::voodoo_vblank));
	m_voodoo->pciint_callback().set(FUNC(viper_state::voodoo_pciint));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	// Screeen size and timing is re-calculated later in voodoo card
	screen.set_refresh_hz(60);
	screen.set_size(1024, 768);
	screen.set_visarea(0, 1024 - 1, 0, 768 - 1);
	screen.set_screen_update(FUNC(viper_state::screen_update));

	PALETTE(config, "palette").set_entries(65536);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	DMADAC(config, "dacl").add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	DMADAC(config, "dacr").add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	M48T58(config, "m48t58", 0);

	// Each IRQ3 will update the data buffers with 256 samples, and the playback rate is always 44100hz.
	// The frequency is picked such that the DMADAC buffer should never overflow or underflow.
	// Note that adjusting this value has gameplay consequences for ppp2nd: the gameplay's note and animation timings are tied directly to values updated using IRQ3,
	// so having IRQ3 trigger too quickly or too slowly will mean that the gameplay will either be too fast or too slow.
	TIMER(config, "sound_timer").configure_periodic(FUNC(viper_state::sound_timer_callback), attotime::from_hz(44100.0 / 256));
}

void viper_state::viper_ppp(machine_config &config)
{
	viper(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &viper_state::viper_ppp_map);
}

// mocapb / p911 alt speaker config
void viper_state::viper_fullbody(machine_config &config)
{
	viper(config);
	config.device_remove("lspeaker");
	config.device_remove("rspeaker");
	SPEAKER(config, "front").front_center();
	SPEAKER(config, "rear").rear_center();
	DMADAC(config.replace(), "dacl").add_route(ALL_OUTPUTS, "front", 1.0);
	DMADAC(config.replace(), "dacr").add_route(ALL_OUTPUTS, "rear", 1.0);
}

void viper_state::viper_fbdongle(machine_config &config)
{
	viper_fullbody(config);
	DS2430A(config, m_ds2430_ext);
}

void viper_state::omz3d_map(address_map &map)
{
	map(0x00000, 0x0ffff).rom().region("ioboard", 0);
}

void viper_state::viper_omz(machine_config &config)
{
	viper(config);

	upd784031_device &omz3dcpu(UPD784031(config, "omz3dcpu", 12000000));
	omz3dcpu.set_addrmap(AS_PROGRAM, &viper_state::omz3d_map);
}

/*****************************************************************************/

void viper_state::init_viper()
{
//  m_maincpu->space(AS_PROGRAM).install_legacy_readwrite_handler( *ide, 0xff200000, 0xff207fff, FUNC(hdd_r), FUNC(hdd_w) ); //TODO
}

void viper_state::init_viperhd()
{
	init_viper();

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xff300000, 0xff300fff, read64s_delegate(*this, FUNC(viper_state::ata_r)), write64s_delegate(*this, FUNC(viper_state::ata_w)));
}

void viper_state::init_vipercf()
{
	init_viper();

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xff000000, 0xff000fff, read64s_delegate(*this, FUNC(viper_state::cf_card_data_r)), write64s_delegate(*this, FUNC(viper_state::cf_card_data_w)));
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xff200000, 0xff200fff, read64s_delegate(*this, FUNC(viper_state::cf_card_r)), write64s_delegate(*this, FUNC(viper_state::cf_card_w)));

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xff300000, 0xff300fff, read64s_delegate(*this, FUNC(viper_state::unk_serial_r)), write64s_delegate(*this, FUNC(viper_state::unk_serial_w)));
}

uint16_t viper_state::ppp_sensor_r(offs_t offset)
{
	switch(offset)
	{
		case 0x06: return m_io_ppp_sensors[0]->read();
		case 0x0e: return m_io_ppp_sensors[1]->read();
		case 0x16: return m_io_ppp_sensors[2]->read();
		case 0x1e: return m_io_ppp_sensors[3]->read();
	}

	return 0;
}

/*****************************************************************************/

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

#define VIPER_BIOS \
	ROM_REGION64_BE(0x40000, "user1", 0)    /* Boot ROM */ \
	ROM_SYSTEM_BIOS(0, "bios0", "GM941B01 (01/15/01)") \
		ROM_LOAD_BIOS(0, "941b01.u25", 0x00000, 0x40000, CRC(233e5159) SHA1(66ff268d5bf78fbfa48cdc3e1b08f8956cfd6cfb)) \
	ROM_SYSTEM_BIOS(1, "bios1", "GM941A01 (03/10/00)") \
		ROM_LOAD_BIOS(1, "941a01.u25", 0x00000, 0x40000, CRC(df6f88d6) SHA1(2bc10e4fbec36573aa8b6878492d37665f074d87))

ROM_START(kviper)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	// presumably doesn't belong here
	ROM_LOAD("ds2430.u3", 0x00, 0x28, CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
ROM_END


/* Viper games with hard disk */
ROM_START(ppp2nd)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(ef0e7caa) SHA1(02fef7465445d33f0288c49a8998a2759ad70823))
	// byte 0x1e (0) JAA (1) AAA
	// byte 0x1f (1) rental

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "ppp2nd", 0, SHA1(b8b90483d515c83eac05ffa617af19612ea990b0))
ROM_END

ROM_START(ppp2nda)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430-aaa.u3", 0x00, 0x28, BAD_DUMP CRC(76906d8f) SHA1(ceea4addc881975cfd6b8e2283b9aecb6080bd99))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "ppp2nd", 0, SHA1(b8b90483d515c83eac05ffa617af19612ea990b0))
ROM_END

/* Viper games with Compact Flash card */
ROM_START(boxingm) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a45jaa_nvram.u39", 0x00000, 0x2000, CRC(c24e29fc) SHA1(efb6ecaf25cbdf9d8dfcafa85e38a195fa5ff6c4))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a45a02", 0, SHA1(9af2481f53de705ae48fad08d8dd26553667c2d0) )
ROM_END

ROM_START(code1d) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* game-specific DS2430 on PCB */
	ROM_LOAD("ds2430_code1d2.u3", 0x00, 0x28, BAD_DUMP CRC(817e725f) SHA1(0c36ddf1e0c4dc6f6b46ec73d3e86eb58247fa42))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("m48t58_uad.u39", 0x00000, 0x2000, CRC(22ef677d) SHA1(10b1e68d409edeca5af70aff1146b7373eeb3864) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "922d02", 0, SHA1(01f35e324c9e8567da0f51b3e68fff1562c32116) )
ROM_END

ROM_START(code1db) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* game-specific DS2430 on PCB */
	ROM_LOAD("ds2430_code1d.u3", 0x00, 0x28, BAD_DUMP CRC(fada04dd) SHA1(49bd4e87d48f0404a091a79354bbc09cde739f5c))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("m48t58_uab.u39", 0x00000, 0x2000, CRC(6059cdad) SHA1(67f9d9239c3e3ef8c967f26c45fa9201981ad848) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "922b02", 0, SHA1(4d288b5dcfab3678af662783e7083a358eee99ce) )
ROM_END

ROM_START(gticlub2) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	// both with non-default settings (check sound options for instance)
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, BAD_DUMP CRC(d0604e84) SHA1(18d1183f1331af3e655a56692eb7ab877b4bc239))
	ROM_LOAD("941jab_nvram.u39", 0x00000, 0x2000, BAD_DUMP CRC(6c4a852f) SHA1(2753dda42cdd81af22dc6780678f1ddeb3c62013))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "941b02", 0,  SHA1(943bc9b1ea7273a8382b94c8a75010dfe296df14) )
ROM_END

ROM_START(gticlub2ea) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, NO_DUMP )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("941eaa_nvram.u39", 0x00000, 0x2000, BAD_DUMP CRC(5ee7004d) SHA1(92e0ce01049308f459985d466fbfcfac82f34a47))

	DISK_REGION( "ata:0:hdd" ) // 32 MB Memory Card labeled 941 EA A02
	DISK_IMAGE( "941a02", 0,  SHA1(dd180ad92dd344b38f160e31833077e342cee38d) ) // with ATA id included
ROM_END

/* This CF card has sticker B41C02 */
ROM_START(jpark3) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b41ebc_nvram.u39", 0x00000, 0x2000, CRC(55d1681d) SHA1(26868cf0d14f23f06b81f2df0b4186924439bb43))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "b41c02", 0, SHA1(fb6b0b43a6f818041d644bcd711f6a727348d3aa) )
ROM_END

/* This CF card has sticker B41C02 */
ROM_START(jpark3u) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b41 ua rtc.u39", 0x00000, 0x1ff8, CRC(75fdda39) SHA1(6292ce0d32afdf6bde33ac7f1f07655fa17282f6))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "b41c02", 0, SHA1(fb6b0b43a6f818041d644bcd711f6a727348d3aa) )
ROM_END

/* This CF card has sticker B33A02 */
ROM_START(mocapglf) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, CRC(4d9d7178) SHA1(97215aa13136c1393363a0ebd1e5b885ca602293))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b33uaa_nvram.u39", 0x00000, 0x2000, CRC(5eece882) SHA1(945e5e9882bd16513a2947f6823b985d51501fad))

	ROM_REGION(0x10000, "ioboard", 0) // OMZ-3DCPU PCB
	ROM_LOAD("kzkn1.bin", 0x00000, 0x10000, CRC(b87780d8) SHA1(bae84785d218daa9666143f08e2632ca1b7a4f72))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "b33a02", 0, SHA1(819d8fac5d2411542c1b989105cffe38a5545fc2) )
ROM_END

ROM_START(mocapb) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a29aaa_nvram.u39", 0x000000, 0x2000, CRC(14b9fe68) SHA1(3c59e6df1bb46bc1835c13fd182b1bb092c08759)) //supposed to be aab version?

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a29b02", 0, SHA1(f0c04310caf2cca804fde20805eb30a44c5a6796) ) //missing bootloader
ROM_END

ROM_START(mocapbj) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a29jaa_nvram.u39", 0x000000, 0x2000, CRC(2f7cdf27) SHA1(0b69d8728be12909e235268268a312982f81d46a))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a29a02", 0, SHA1(00afad399737652b3e17257c70a19f62e37f3c97) )
ROM_END

ROM_START(p911) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00aae_nvram.u39", 0x000000, 0x2000, BAD_DUMP CRC(9ecd75a3) SHA1(f9db35b91d4ef7fd61f21382fc62a6428d0b0c52))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00uad02", 0, SHA1(6acb8dc41920e7025b87034a3a62b185ef0109d9) ) // Actually is AAE/KAE
ROM_END

ROM_START(p911k) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00kae_nvram.u39", 0x000000, 0x2000, BAD_DUMP CRC(157e0361) SHA1(a4e301f1c73d148b3c18c9c02b67692ffdd6a664))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00uad02", 0, SHA1(6acb8dc41920e7025b87034a3a62b185ef0109d9) ) // Actually is AAE/KAE
ROM_END

ROM_START(p911ac) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00aac_nvram.u39", 0x000000, 0x2000, BAD_DUMP CRC(d65742ce) SHA1(20055c0b701c62b0f01cfe619d07bd9532cc3b45))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00uac02", 0, SHA1(b268789416dbf8886118a634b911f0ee254970de) ) // a00uac02 and a00kac02 are the same image
ROM_END

ROM_START(p911kc) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00kac_nvram.u39", 0x000000, 0x2000,  CRC(8ddc921c) SHA1(901538da237679fc74966a301278b36d1335671f) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00kac02", 0, SHA1(b268789416dbf8886118a634b911f0ee254970de) )
ROM_END

ROM_START(p911ud) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00uad_nvram.u39", 0x000000, 0x2000,  BAD_DUMP CRC(c4f44a70) SHA1(d7946606bf72ca7a6f391c4832205ae6fb1ebd95) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00eaa02", 0, SHA1(81565a2dce2e2b0a7927078a784354948af1f87c) ) // Is actually UAD/EAD
ROM_END

ROM_START(p911ed) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00ead_nvram.u39", 0x000000, 0x2000,  BAD_DUMP CRC(0314fc96) SHA1(cbf421bb37f0a122944fbccf8f4c80380c89e094) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00eaa02", 0, SHA1(81565a2dce2e2b0a7927078a784354948af1f87c) ) // Is actually UAD/EAD
ROM_END

ROM_START(p911ea)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00eaa_nvram.u39", 0x000000, 0x2000,  CRC(4f3497b6) SHA1(3045c54f98dff92cdf3a1fc0cd4c76ba82d632d7) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00eaa02_ea", 0, SHA1(fa057bf17f4c0fb9b9a09b820ff7a101e44fab7d) )
ROM_END

ROM_START(p911j) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a00jaa_nvram.u39", 0x000000, 0x2000, CRC(9ecf70dc) SHA1(4769a99b0cc28563e219860b8d480f32d1e21f60))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a00jac02", 0, SHA1(d962d3a8ea84c380767d0fe336296911c289c224) )
ROM_END

ROM_START(p9112) /* dongle-protected version */
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430_ext", ROMREGION_ERASE00)       /* plug-in male DIN5 dongle containing a DS2430. The sticker on the dongle says 'GCB11-UA' */
	ROM_LOAD("ds2430_p9112.u3", 0x00, 0x28, CRC(d745c6ee) SHA1(065c9d0df1703b3bbb53a07f4923fdee3b16f80e))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b11uad_nvram.u39", 0x000000, 0x2000, CRC(cda37033) SHA1(a94524824f21a0106928b4fe01d86f967bd5aa82))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "b11a02", 0, SHA1(57665664321b78c1913d01f0d2c0b8d3efd42e04) )
ROM_END

ROM_START(sscopex)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, CRC(427a65ef) SHA1(745e951715ece9f60898b7ed4809e69558145d2d))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a13uaa_nvram.u39", 0x000000, 0x2000, CRC(7b0e1ac8) SHA1(1ea549964539e27f87370e9986bfa44eeed037cd))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a13c02", 0, SHA1(d740784fa51a3f43695ea95e23f92ef05f43284a) )
ROM_END

//TODO: sscopexb + many nvram clone versions.

ROM_START(sogeki) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, CRC(771d8256) SHA1(afd89ae2d196fe40174bba46581d1eb5c2302932) )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(2f325c55) SHA1(0bc44f40f981a815c8ce64eae95ae55db510c565))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a13b02", 0, SHA1(c25a61b76d365794c2da4a9e7de88a5519e944ec) )
ROM_END

ROM_START(sscopefh)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, CRC(9271c24f) SHA1(f194fea15969b322c96cce8f0335dccd3475a3e6) )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, CRC(2dd07bdf) SHA1(dadc189625e11c98f68afd988700a842c78b0ca7) )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "ccca02", 0, SHA1(ec0d9a1520f17c73750de71dba8b31bc8c9d0409) )
ROM_END

ROM_START(thrild2) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41ebb_nvram.u39", 0x00000, 0x2000, CRC(22f59ac0) SHA1(e14ea2ba95b72edf0a3331ab82c192760bfdbce3))
//  a41eba_nvram == a41ebb_nvram

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a41b02", 0, SHA1(0426f4bb9001cf457f44e2c22e3d7575b8049aa3) )
ROM_END

ROM_START(thrild2j) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41jaa_nvram.u39", 0x00000, 0x2000, CRC(d56226d5) SHA1(085f40816befde993069f56fdd5f8bd6ccfcf301))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a41a02", 0, SHA1(bbb71e23bddfa07dfa30b6565a35befd82b055b8) ) // same as Asian version
ROM_END

ROM_START(thrild2a) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41aaa_nvram.u39", 0x00000, 0x2000, CRC(d5de9b8e) SHA1(768bcd46a6ad20948f60f5e0ecd2f7b9c2901061))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a41a02", 0, SHA1(bbb71e23bddfa07dfa30b6565a35befd82b055b8) )
ROM_END

ROM_START(thrild2ab)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41aaa_nvram.u39", 0x00000, 0x2000, CRC(d5de9b8e) SHA1(768bcd46a6ad20948f60f5e0ecd2f7b9c2901061))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a41a02_alt", 0, SHA1(7a9cfdab7000765ffdd9198b209f7a74741248f2) )
ROM_END

ROM_START(thrild2ac)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a41aaa_nvram.u39", 0x00000, 0x2000, CRC(d5de9b8e) SHA1(768bcd46a6ad20948f60f5e0ecd2f7b9c2901061))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a41a02_alt2", 0, SHA1(c8bfbac4f5a1a2241df7417ad2f9eba7d9e9a9df) )
ROM_END

/* This CF card has sticker 941EAA02 */
ROM_START(thrild2c) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("941eaa_nvram.u39", 0x00000, 0x2000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a41c02", 0, SHA1(ab3020e8709768c0fd2467573e92b679a05944e5) )
ROM_END

ROM_START(tsurugi) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a30eab_nvram.u39", 0x00000, 0x2000, CRC(c123342c) SHA1(55416767608fe0311a362854a16b214b04435a31))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a30b02", 0, SHA1(d2be83b7323c365ba445de7697c3fb8eb83d0212) )
ROM_END

ROM_START(tsurugij) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("a30jac_nvram.u39", 0x00000, 0x2000, BAD_DUMP CRC(0e2c0e61) SHA1(d77670e214f618652e67fa91e644750894a0c5c7))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a30c02", 0, SHA1(533b5669b00884a800df9ba29651777a76559862) )
ROM_END

ROM_START(tsurugie)
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, NO_DUMP )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x000000, 0x2000, NO_DUMP )

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "a30eab02", 0, SHA1(fcc5b69f89e246f26ca4b8546cc409d3488bbdd9) ) // Incomplete dump? Is half the size of the other dumps
ROM_END

/* This CF card has sticker C22D02 */
ROM_START(wcombat) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("wcombat_nvram.u39", 0x00000, 0x2000, CRC(4f8b5858) SHA1(68066241c6f9db7f45e55b3c5da101987f4ce53c))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "c22d02", 0, SHA1(69a24c9e36b073021d55bec27d89fcc0254a60cc) ) // chs 978,8,32
ROM_END

ROM_START(wcombatb) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("wcombat_nvram.u39", 0x00000, 0x2000, CRC(4f8b5858) SHA1(68066241c6f9db7f45e55b3c5da101987f4ce53c))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "c22d02_alt", 0, SHA1(772e3fe7910f5115ec8f2235bb48ba9fcac6950d) ) // chs 978,8,32
ROM_END

ROM_START(wcombatk) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, NO_DUMP )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("wcombatk_nvram.u39", 0x00000, 0x2000, CRC(ebd4d645) SHA1(2fa7e2c6b113214f3eb1900c8ceef4d5fcf0bb76))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "c22c02", 0, BAD_DUMP SHA1(8bd1dfbf926ad5b28fa7dafd7e31c475325ec569) )
ROM_END

ROM_START(wcombatu) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, NO_DUMP )

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("warzaid u39 c22d02", 0x00000, 0x2000, CRC(71744990) SHA1(19ed07572f183e7b3a712704ebddf7a848c48a78) )

	DISK_REGION( "ata:0:hdd" )
	// CHD image provided had evidence of being altered by Windows, probably was put in a Windows machine without write protection hardware (bad idea)
	// label was the same as this, so this should be a clean and correct version.
	DISK_IMAGE( "c22d02", 0, SHA1(69a24c9e36b073021d55bec27d89fcc0254a60cc) ) // chs 978,8,32
ROM_END

/* This CF card has sticker C22A02 */
ROM_START(wcombatj) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("wcombatj_nvram.u39", 0x00000, 0x2000, CRC(bd8a6640) SHA1(2d409197ef3fb07d984d27fa943f29c7a711d715))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "c22a02", 0, SHA1(7200c7c436491fd8027d6d7139a80ee3b984697b) ) // chs 978,8,32
ROM_END

ROM_START(xtrial) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("b4xjab_nvram.u39", 0x00000, 0x2000, CRC(33708a93) SHA1(715968e3c9c15edf628fa6ac655dc0864e336c6c))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "b4xb02", 0, SHA1(d8d54f3f16b762bf0187fe29b2f8696015c0a940) )
ROM_END

/* Viper Satellite Terminal games */

/*
Mahjong Fight Club (Konami Viper h/w)
Konami, 2002
78,8,3)
PCB number - GM941-PWB(A)C Copyright 1999 Konami Made In Japan

Mahjong Fight Club is a multi player Mahjong battle game for up to 8 players. A
single PCB will not boot unless all of the other units are connected and powered
on, although how exactly they're connected is unknown. There is probably a
master unit that talks to all of the 8 satellite units. At the moment I have
only 2 of the 8 satellite units so I can't confirm that.
However, I don't have access to the main unit anyway as it was not included in
the auction we won :(

The Viper hardware can accept additional PCBs inside the metal box depending on
the game. For Mahjong Fight Club, no additional PCBs are present or required.

The main CPU is a Motorola XPC8240LZU200E
The main graphics chip is heatsinked. It's a BGA chip, and might be something
like a Voodoo chip? Maybe :-)
There's 1 Konami chip stamped 056879
There's also a bunch of video RAMs and several PLCC FPGAs or CPLDs
There's also 1 PLCC44 chip stamped PC16552

Files
-----
c09jad04.bin is a 64M Compact Flash card. The image was simply copied from the
card as it is PC readable. The card contains only 1 file named c09jad04.bin

941b01.u25 is the BIOS, held in a 2MBit PLCC32 Fujitsu MBM29F002 EEPROM and
surface mounted at location U25. The BIOS is common to ALL Viper games.

nvram.u39 is a ST M48T58Y Timekeeper NVRAM soldered-in at location U39. The
codes at the start of the image (probably just the first 16 or 32 bytes) are
used as a simple (and very weak) protection check to stop game swaps. The
contents of the NVRAM is different for ALL games on this hardware.

Some games use a dongle and swapping games won't work unless the dongle is also provided.
The following games comes with a dongle....
Mahjong Fight Club

For non-dongled games, I have verified the following games will work when the
CF card and NVRAM are swapped....
*/

/* This CF card has sticker C09JAD04 */
ROM_START(mfightc) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("nvram.u39", 0x00000, 0x2000, CRC(9fb551a5) SHA1(a33d185e186d404c3bf62277d7e34e5ad0000b09)) //likely non-default settings
	ROM_LOAD("c09jad_nvram.u39", 0x00000, 0x2000, CRC(33e960b7) SHA1(a9a249e68c89b18d4685f1859fe35dc21df18e14))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "c09d04", 0, SHA1(7395b7a33e953f65827aea44461e49f8388464fb) )
ROM_END

/* This CF card has sticker C09JAC04 */
ROM_START(mfightcc) //*
	VIPER_BIOS

	ROM_REGION(0x28, "ds2430", ROMREGION_ERASE00)       /* DS2430 */
	ROM_LOAD("ds2430.u3", 0x00, 0x28, BAD_DUMP CRC(f1511505) SHA1(ed7cd9b2763b3e377df9663943160f9871f65105))

	ROM_REGION(0x2000, "m48t58", ROMREGION_ERASE00)     /* M48T58 Timekeeper NVRAM */
	ROM_LOAD("c09jac_nvram.u39", 0x00000, 0x2000, BAD_DUMP CRC(2d100e2b) SHA1(209764130ec3279fe17fe98de6cd0780b80c148f))

	DISK_REGION( "ata:0:hdd" )
	DISK_IMAGE( "c09c04", 0, SHA1(bf5f7447d74399d34edd4eb6dfcca7f6fc2154f2) )
ROM_END


} // Anonymous namespace


/*****************************************************************************/

/* Viper BIOS */
GAME(1999, kviper,    0,         viper,     viper,      viper_state, init_viper,    ROT0,  "Konami", "Konami Viper BIOS", MACHINE_IS_BIOS_ROOT)

GAME(2001, ppp2nd,    kviper,    viper_ppp, ppp2nd,     viper_state, init_viperhd,  ROT0,  "Konami", "ParaParaParadise 2nd Mix (JAA)", MACHINE_NOT_WORKING)
GAME(2001, ppp2nda,   ppp2nd,    viper_ppp, ppp2nd,     viper_state, init_viperhd,  ROT0,  "Konami", "ParaParaParadise 2nd Mix (AAA)", MACHINE_NOT_WORKING)

GAME(2001, boxingm,   kviper,    viper,     boxingm,    viper_state, init_vipercf,  ROT0,  "Konami", "Boxing Mania: Ashita no Joe (ver JAA)", MACHINE_NOT_WORKING)
GAME(2000, code1d,    kviper,    viper,     code1d,     viper_state, init_vipercf,  ROT0,  "Konami", "Code One Dispatch Ver 1.21 (ver UAD)", MACHINE_NOT_WORKING)
GAME(2000, code1db,   code1d,    viper,     code1d,     viper_state, init_vipercf,  ROT0,  "Konami", "Code One Dispatch Ver 1.16 (ver UAB)", MACHINE_NOT_WORKING)
GAME(2000, gticlub2,  kviper,    viper,     gticlub2,   viper_state, init_vipercf,  ROT0,  "Konami", "GTI Club: Corso Italiano (ver JAB)", MACHINE_NOT_WORKING)
GAME(2000, gticlub2ea,gticlub2,  viper,     gticlub2ea, viper_state, init_vipercf,  ROT0,  "Konami", "Driving Party: Racing in Italy (ver EAA)", MACHINE_NOT_WORKING)
GAME(2001, jpark3,    kviper,    viper,     jpark3,     viper_state, init_vipercf,  ROT0,  "Konami", "Jurassic Park III (ver EBC)", MACHINE_NOT_WORKING)
GAME(2001, jpark3u,   jpark3,    viper,     jpark3,     viper_state, init_vipercf,  ROT0,  "Konami", "Jurassic Park III (ver UBC)", MACHINE_NOT_WORKING)
GAME(2001, mocapglf,  kviper,    viper_omz, mocapglf,   viper_subscreen_state, init_vipercf,  ROT90, "Konami", "Mocap Golf (ver EAA:B)", MACHINE_NOT_WORKING)
GAME(2001, mocapb,    kviper,    viper_fullbody, mocapb,     viper_state, init_vipercf,  ROT90, "Konami", "Mocap Boxing (ver AAB)", MACHINE_NOT_WORKING)
GAME(2001, mocapbj,   mocapb,    viper_fullbody, mocapb,     viper_state, init_vipercf,  ROT90, "Konami", "Mocap Boxing (ver JAA)", MACHINE_NOT_WORKING)
GAME(2000, p911,      kviper,    viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "The Keisatsukan: Shinjuku 24-ji (ver AAE)", MACHINE_NOT_WORKING)
GAME(2000, p911k,     p911,      viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "The Keisatsukan: Shinjuku 24-ji (ver KAE)", MACHINE_NOT_WORKING)
GAME(2000, p911ac,    p911,      viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "The Keisatsukan: Shinjuku 24-ji (ver AAC)", MACHINE_NOT_WORKING)
GAME(2000, p911kc,    p911,      viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "The Keisatsukan: Shinjuku 24-ji (ver KAC)", MACHINE_NOT_WORKING)
GAME(2000, p911ud,    p911,      viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "Police 911 (ver UAD)", MACHINE_NOT_WORKING)
GAME(2000, p911ed,    p911,      viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "Police 24/7 (ver EAD)", MACHINE_NOT_WORKING)
GAME(2000, p911ea,    p911,      viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "Police 24/7 (ver EAD, alt)", MACHINE_NOT_WORKING)
GAME(2000, p911j,     p911,      viper_fullbody,     p911,       viper_state, init_vipercf,  ROT90, "Konami", "The Keisatsukan: Shinjuku 24-ji (ver JAE)", MACHINE_NOT_WORKING)
GAME(2001, p9112,     kviper,    viper_fbdongle,     p9112,      viper_state, init_vipercf,  ROT90, "Konami", "Police 911 2 (VER. UAA:B)", MACHINE_NOT_WORKING)
GAME(2001, sscopex,   kviper,    viper,     sscopex,    viper_subscreen_state, init_vipercf,  ROT0,  "Konami", "Silent Scope EX (ver UAA)", MACHINE_NOT_WORKING)
GAME(2001, sogeki,    sscopex,   viper,     sogeki,     viper_subscreen_state, init_vipercf,  ROT0,  "Konami", "Sogeki (ver JAA)", MACHINE_NOT_WORKING)
GAME(2002, sscopefh,  kviper,    viper,     sscopefh,   viper_subscreen_state, init_vipercf,  ROT0,  "Konami", "Silent Scope Fortune Hunter (ver EAA)", MACHINE_NOT_WORKING) // UK only?
GAME(2001, thrild2,   kviper,    viper,     thrild2,    viper_state, init_vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver EBB)", MACHINE_NOT_WORKING)
GAME(2001, thrild2j,  thrild2,   viper,     gticlub2,   viper_state, init_vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver JAA)", MACHINE_NOT_WORKING)
GAME(2001, thrild2a,  thrild2,   viper,     gticlub2,   viper_state, init_vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver AAA)", MACHINE_NOT_WORKING)
GAME(2001, thrild2ab, thrild2,   viper,     gticlub2,   viper_state, init_vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver AAA, alt)", MACHINE_NOT_WORKING)
GAME(2001, thrild2ac, thrild2,   viper,     thrild2,    viper_state, init_vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver AAA, alt 2)", MACHINE_NOT_WORKING)
GAME(2001, thrild2c,  thrild2,   viper,     thrild2,    viper_state, init_vipercf,  ROT0,  "Konami", "Thrill Drive 2 (ver EAA)", MACHINE_NOT_WORKING)
GAME(2002, tsurugi,   kviper,    viper,     tsurugi,    viper_state, init_vipercf,  ROT0,  "Konami", "Tsurugi (ver EAB)", MACHINE_NOT_WORKING)
GAME(2002, tsurugie,  tsurugi,   viper,     tsurugi,    viper_state, init_vipercf,  ROT0,  "Konami", "Tsurugi (ver EAB, alt)", MACHINE_NOT_WORKING)
GAME(2002, tsurugij,  tsurugi,   viper,     tsurugi,    viper_state, init_vipercf,  ROT0,  "Konami", "Tsurugi (ver JAC)", MACHINE_NOT_WORKING)
GAME(2002, wcombat,   kviper,    viper,     wcombat,    viper_state, init_vipercf,  ROT0,  "Konami", "World Combat (ver AAD:B)", MACHINE_NOT_WORKING)
GAME(2002, wcombatb,  wcombat,   viper,     wcombat,    viper_state, init_vipercf,  ROT0,  "Konami", "World Combat (ver AAD:B, alt)", MACHINE_NOT_WORKING)
GAME(2002, wcombatk,  wcombat,   viper,     wcombat,    viper_state, init_vipercf,  ROT0,  "Konami", "World Combat (ver KBC:B)", MACHINE_NOT_WORKING)
GAME(2002, wcombatu,  wcombat,   viper,     wcombat,    viper_state, init_vipercf,  ROT0,  "Konami", "World Combat / Warzaid (ver UCD:B)", MACHINE_NOT_WORKING)
GAME(2002, wcombatj,  wcombat,   viper,     wcombatj,   viper_state, init_vipercf,  ROT0,  "Konami", "World Combat (ver JAA)", MACHINE_NOT_WORKING)
GAME(2002, xtrial,    kviper,    viper,     xtrial,     viper_state, init_vipercf,  ROT0,  "Konami", "Xtrial Racing (ver JAB)", MACHINE_NOT_WORKING)

GAME(2002, mfightc,   kviper,    viper,     mfightc,    viper_state, init_vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver JAD)", MACHINE_NOT_WORKING)
GAME(2002, mfightcc,  mfightc,   viper,     mfightc,    viper_state, init_vipercf,  ROT0,  "Konami", "Mahjong Fight Club (ver JAC)", MACHINE_NOT_WORKING)
