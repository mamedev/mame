// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    Sega Lindbergh skeleton driver

    TODO:
    - tests area 0xd0000 - 0xd000f, wants an undumped ROM in there?
    - Apparently there's no way to avoid a dead lock at 0xfd085, perhaps
      tied to the aforementioned?

***************************************************************************

Lindbergh
Sega 2005-2009

This is a "PC-based" arcade system. Different configurations have different colored boxes.
The version documented here is the red box. The PC part of it is mostly just the CPU,
Intel North/South-bridge chipset and AGP/PCI card slots etc. The main board is still
a typically custom-made Sega arcade PCB using a custom nVIDIA GeForce video card.
The main board also has a slot for a compact flash card. Primary storage media is HDD.
Games are installed from a DVD. Both the CF and HDD are locked and unreadable on a regular PC.

The familiar PIC is still present on the back of the system and likely decrypts the HDD and/or DVD.

On this red box the CPU is a Celeron D at 2.8GHz. RAM is 512M DDR PC3200
The box has Sega number 845-0001D-02


Security
--------

The security seems to work in multiple steps.  The information here
is a combination of our research and things found on the internet.

- At boot, the bios unlocks the CF card through an IDE command.  There
  is also a hardware heartbeat signal on the IDE bus to avoid
  hotswapping, and making it hard to dump the card outside of a Lindberg
  motherboard.

- The system boots on the CF which holds a customized Montavista linux.

- The CF system can either install the game (from the DVD) or start it
  (on the HD) through the "/usr/sbin/segaboot" executable in the second
  partition.

- The DVD includes an ISO-9660 filesystem at a (game-dependant)
  offset. It has a handful of files, all encrypted.  Of specific
  interest and the su[0-3].dat files which are system updates, and the
  frontend file which handles the setup of all the other files for the
  game.

- The PIC includes an AES-CBC engine and has as data an IV, a key,
  some game-specific identification information, and two pre and
  post-whitening values.  Everything but the key is dumpable through
  commands, but the key seems well-protected.  It's not realistic to
  decrypt very large amounts of data through it though, the bandwidth
  would be way too low.

- The CF decrypts the dvd/hd files with a custom crypto system which
  is keyed by the result of decrypting 16 times 0x00, 16 times 0x01,
  ..., 16 times 0x0b through the PIC, giving a 176 bytes secondary key.
  segaboot (in the second partition) and lxdecrypt_hard (in the first
  partition's initrd) take care of that.

- The HD is unlocked by the CF with lxunlock.hdb in the first
  partition's initrd.  The method varies depending on the HD model.
  That code is also capable of unlocking the CF (but don't forget
  the hardware hearbeat there).


Lindbergh Game List
-------------------
                                              Security          Sega Part#
Game                                          Dongle Sticker    printed on PIC  DVD Code
------------------------------------------------------------------------------------------
2 Spicy                                       253-5508-0491     317-0491-COM   ^DVP-0027A
After Burner Climax (EXPORT)                  253-5508-0440A   ^317-0440-COM    DVP-0009
After Burner Climax CE                        ?                 ?               DVP-0031A
Ami-Gyo                                       ?                 ?               ?
Answer X Answer                               253-5508-0618J    317-0618-JPN    ?
Answer X Answer 1.1                           ?                 ?               ?
Answer X Answer DX                            ?                 ?               ?
Answer X Answer Premium                       ?                 ?               ?
Club Majesty Extend                           ?                 ?               ?
Club Majesty Formal                           ?                 ?               ?
Derby Owners Club 2008: Feel the Rush         ?                 ?               DVP-0047A
Derby Owners Club 2008: Feel the Rush V2.0    ?                 ?               ?
Ghost Squad Evolution                         ?                 ?              ^DVP-0029A
Harley Davidson: King of the Road             ?                 ?               ?
Hummer Extreme                                253-5508-????    ^317-????-COM    ?
Initial D Arcade Stage 4                      253-5508-0620     317-0620-COM    DVP-0019
Initial D Arcade Stage 4 (rev A)              253-5508-0620     317-0620-COM    DVP-0019A
Initial D Arcade Stage 4 (rev B)              253-5508-0620     317-0620-COM    DVP-0019B
Initial D Arcade Stage 4 (rev C)              253-5508-0620     317-0620-COM   ^DVP-0019C
Initial D Arcade Stage 4 (rev D)              253-5508-0620     317-0620-COM    DVP-0019D
Initial D Arcade Stage 4 (rev G)              253-5508-0620     317-0620-COM    DVP-0019G
Initial D4                                    253-5508-0486E    317-0486-COM    DVP-0030
Initial D4 (rev B)                            253-5508-0486E    317-0486-COM    DVP-0030B
Initial D4 (rev C)                            253-5508-0486E    317-0486-COM   ^DVP-0030C
Initial D4 (rev D)                            253-5508-0486E    317-0486-COM   ^DVP-0030D
Initial D Arcade Stage 5                      ?                 ?               ?
Let's Go Jungle (EXPORT)                      253-5508-0442     317-0442-COM    DVP-0011
MJ4                                           ?                 ?               ?
OutRun 2 Special Tours (EXPORT)               253-5508-0452     317-0452-COM    ?
OutRun 2 SP SDX                               ?                 ?               DVP-0015A
Primeval Hunt                                 253-5508-0512     317-0512-COM   ^DVP-0048A
R-Tuned: Ultimate Street Racing               ?                 ?               DVP-0060
Rambo (EXPORT)                                253-5508-0540    ^317-0540-COM   ^DVP-0069
SEGA Network Taisen Mahjong MJ4 (rev A)       ?                 ?               DVP-0049A
SEGA Network Taisen Mahjong MJ4 (rev F)       ?                 ?               DVP-0049F
SEGA Network Taisen Mahjong MJ4 (rev G)       ?                 ?               DVP-0049G
SEGA-Race TV (EXPORT)                         253-5508-0504    ^317-0504-COM   ^DVP-0044
StarHorse 2: Fifth Expansion (rev D)          ?                 ?               DVP-0082D
StarHorse 2: Fifth Expansion (rev E)          ?                 ?               DVP-0082E
The House Of The Dead 4 (EXPORT)              253-5508-0427    ^317-0427-COM   ^DVP-0003A
The House Of The Dead EX (JAPAN)              253-5508-0550    ^317-0550-JPN    ?
The House Of the Dead 4 Special               ?                 ?               ?
VBIOS Update                                  ?                 ?              ^DVP-0021B
VBIOS Update [For VTF]                        ?                 ?               DVP-0023A
Virtua Fighter 5 (EXPORT)                     253-5508-0438     317-0438-COM    DVP-0008E
Virtua Tennis 3 (Power Smash 3)               ?                 ?               DVP-0005
Virtua Tennis 3 (Power Smash 3) (EXPORT)      253-5508-0434    ^317-0434-COM    DVP-0005A
Virtua Tennis 3 (JAPAN)                       253-5508-0506     317-0506-JPN   ^DVP-0005C
WCC Football Intercontinental Clubs 2006-2007 ?                 ?               ?
WCC Football Intercontinental Clubs 2007-2008 ?                 ?               ?
WCC Football Intercontinental Clubs 2008-2009 ?                 ?               ?
WCC Football Intercontinental Clubs 2009-2010 ?                 ?               ?

^ denotes these parts are archived.
This list is not necessarily correct or complete.
Corrections and additions to the above are welcome.


Mainboard
---------

838-14487
Sticker: 838-14673
                   |----|     |-----|
                   |USB ||---||1/8  | |-------|
                   |USB ||USB||AUDIO| |SERIAL1|  SECURITY
-------------------|RJ45||USB||JACKS|-|SERIAL2|--CONNECTOR---|
|OSC(D245L6I)                                                |
|      VIA                   OSC(D250L6I)                    |
|      VT1616          |-------|                             |
|                      |82541PI|  12V_POWER                  |
|                      |INTEL  |                             |
|  P  P  P  P          |-------|              |-----------|  |
|  C  C  C  C                        ISL6556B |           |  |
|  I  I  I  I         A          |-------|    |           |  |
|  4  3  2  1         G          |JG82875|    |           |  |
|                     P          |SL8DB  |    |    CPU    |  |
|                                |       |    |           |  |
|                                |INTEL  |    |           |  |
|          14.31818MHz           |-------|    |           |  |
|JP                                           |-----------|  |
|12                                                          |
|34  |-------| 932S208DG           SIMM1                     |
|56  |6300ESB|                                               |
|78  |SL7XJ  |                     SIMM2(not used)           |
|910 |       |                                               |
|1112|INTEL  |                     SIMM3(not used)           |
|    |-------|                                               |
|32.768kHz         |---------|     SIMM4(not used)           |
| 3V_BATT          |COMPACT  |                               |
|    MB_BIOS.3J7   |FLASH    |     IDE40                     |
|                  |SLOT     |     IDE40       ATX_POWER     |
|------------------|---------|-------------------------------|
Notes:
           CPU - Lindbergh RED: Intel Celeron D 335 SL8HM 2.8GHz 256k L2 cache, 533MHz FSB. 
                 Lindbergh YELLOW: Intel Pentium 4 3.00GHz/1M/800 SL8JZ 
         SIMM1 - Lindbergh RED: 512M DDR PC3200
                 Lindbergh YELLOW: 1GB DDR PC3200
       82541PI - Intel Gigabit Ethernet Controller
       6300ESB - Intel Southbridge IC
       JG82875 - Intel Northbridge IC
      ISL6556B - Intersil ISL6556B Optimized Multiphase PWM Controller with 6-Bit
                 DAC and Programmable Internal Temperature Compensation
      932S208DG- IDT 932S208DG Programmable PLL Clock synthesizer
        VT1616 - VIA VT1616 6-channel AC97 codec sound IC
            JP - Jumpers....
                 1-3 Normal (SET ON)
                 1-2 CMOS
                 3-4 PASSWORD
                 5-6 PCBL
                 7-8 BIOS
                 9-10 CF SLAVE
                 11-12 CF MASTER (SET ON)
         IDE40 - ATA133 IDE connector(s)
                 A 40GB HDD is plugged in via an 80-pin flat cable
                 This game is 'Too Spicy'. The hard drive is a Hitachi Deskstar
                 model HDS728040PLAT20. Capacity is 41GB. C/H/S 16383/16/63
                 LBA 80,418,240 sectors. In the model number 8040 means 80GB full
                 capacity but only 40GB is actually available
                 P/N: 0A30209 BA17730E6B
                 Serial: EETNGM0G
       CF SLOT - Accepts a compact flash card. The card is required to boot the system.
                 Revision C and E have been seen. StarHorse 2 has it's own special card.
                 There may be other revisions out there.
                 Sticker: LINDBERGH
                          MDA-C0004A
                          REV. C


Rear Board incorporating Security Board (plugged into main board security connector)
---------------------------------------

This board has the power input connectors on it and holes for access to the mainboard
I/O conections (LAN/COM/USB/speakers etc). Lower left is where the security board is plugged in.

837-14520R
171-8322C
839-1275
Sticker 839-1275R
|----------------------------------|
|                                  |
|                                  |
|       POWER       POWER          |
|                           CN5    |
|                                  |
| LED LED                          |
|                                  |
| DSW(8)                           |
|                                  |
|                                  |
|      SW2  SW1                    |
|                    C/W           |
|                                  |
|     CN2                          |
|-------------|COM1  SPK_REAR  LAN |
|             |                USB4|
|   DIP18.IC1 |                USB3|
|             |COM2  SPK_FR    USB2|
|SECURITY_CONN|                USB1|
|-------------|--------------------|
DSW(8)    - OFF,OFF,OFF,ON,ON,OFF,OFF,ON
DIP18.IC1 - DIP18 socket for protection PIC16F648A


Video Card (plugged into AGP slot)
----------

nVIDIA 180-10508-0000-A00
Sticker: BIOS VERSION 5.73.22.55.08
Sticker: 900-10508-2304-000 D 032 Made In China
Sticker: 600-10508-0004-000 J
Sticker: GeForce 7600 GS 0325206031558
 |----------------------------------------|
 |  VRAM *VRAM                VRAM *VRAM  |-|
 |                                        | |POWER_CONN
 | *VRAM  VRAM |---------|   *VRAM  VRAM  | |
|-V            |NVIDIA   |                |-|
| G            |U611B269 |                |
|-A            |0646B1S  |                |
 |       27MHz |NA6105.00P                |
 |             |G73-N-B1 |                |
 |             |---------|                |
 |                                        |
|-D  *RT9173C             |-------| 25MHz |
| V                       |NVIDIA |       |
| I      *VID_BIOS.U504   |HSI-N-A4       |
|-                        |4MJHT07B30 0607|
 |                        |-------|       |
 |    |----------|     AGP     |----------|
 |----|          |-------------|
Notes:
       * - These parts on the other side of the PCB
    VRAM - QIMONDA HYB18T256161AFL25 WVV10017 256Mbit x16 DDR2 SDRAM (P-TFBGA-84)
VID_BIOS - SST 25VF512 512Kbit serial flash ROM (video BIOS) at location U504 (SOIC8)


JVS I/O Card (plugged into PCI slot #4)
------------

Sega 2004
171-8300C
837-14472R
Sticker: 837-14472R91
|-----------------------------------------|
|                                  3V_BATT|
|        FLASH.IC6            DS14185     |
|                   D442012               |
|                                         |
|LED                                      |
|    DS485                                |
|USB                                0.1F  |
|                                   5.5v  |
|          EDS1232 |--------|       SUPERCAP
|   RTL8201        |FREESCALE             |
|                  |MPC8248 |             |
|RJ45   25MHz      |        |48MHz        |
|                  |        |58.9824MHz   |
| ISP1106          |--------|             |
|mUSB                            PQ070XZ1H|
|  |--------|      PCI      |-------------|
|--|        |---------------|
Notes:
      FLASH.IC6 - Spansion S29AL032D70 32Mbit flash ROM labelled 'FPR-24370B' (TSOP48)
      DS14185   - National Semiconductor DS14185 EIA/TIA-232 3 Driver x 5 Receiver (SOIC10)
      DS485     - National Semiconductor DS485 Low Power RS-485/RS-422 Multipoint Transceiver (SOIC8)
      MPC8248   - Freescale MPC8248 PowerQUICC II Family Multi-Channel Controller (PBGA516)
      EDS1232   - Elpida EDS1232AATA-75-E 128Mbit SDRAM (4M word x 32bit)
      D442012   - NEC D442012AGY-BB70 2Mbit CMOS Static RAM (128k-word x 16bit)
      ISP1106   - NXP Semiconductor ISP1106 Advanced Universal Serial Bus transceiver (SSOIC16)
      RTL8201   - Realtek RTL8201 Single Chip Single Port 10/100M Fast Ethernet IC (QFP48)
      mUSB      - Mini USB connector


JVS I/O Board (connects arcade machine controls etc to PC)
-------------

Sega 2005
837-14572
171-8357B
|------------------------|
| USB_B USB_A CN6 CN7 CN8|
| GH6-2F                 |
| DS485   14.745MHz      |
|      UPC393            |
|           315-6414  CN9|
|                        |
|CN1 CN2       CN3       |
|------------------------|

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"
#include "machine/i82875p.h"
#include "machine/i6300esb.h"
#include "machine/pci-usb.h"
#include "machine/pci-apic.h"
#include "machine/pci-sata.h"
#include "machine/pci-smbus.h"
#include "machine/i82541.h"
#include "machine/segabb.h"
#include "sound/pci-ac97.h"
#include "sound/sb0400.h"
#include "video/gf7600gs.h"

class lindbergh_state : public driver_device
{
public:
	lindbergh_state(const machine_config &mconfig, device_type type, const char *tag);

	virtual void machine_start();
	virtual void machine_reset();
};

lindbergh_state::lindbergh_state(const machine_config &mconfig, device_type type, const char *tag) : driver_device(mconfig, type, tag)
{
}

void lindbergh_state::machine_start()
{
}

void lindbergh_state::machine_reset()
{
}

static MACHINE_CONFIG_START(lindbergh, lindbergh_state)
	MCFG_CPU_ADD("maincpu", PENTIUM4, 28000000U*5) /* Actually Celeron D at 2,8 GHz */

	MCFG_PCI_ROOT_ADD(                ":pci")
	MCFG_I82875P_HOST_ADD(            ":pci:00.0",                        0x103382c0, ":maincpu", 512*1024*1024)
	MCFG_I82875P_AGP_ADD(             ":pci:01.0")
	MCFG_GEFORCE_7600GS_ADD(          ":pci:01.0:00.0",                   0x10de02e1)
	MCFG_I82875P_OVERFLOW_ADD(        ":pci:06.0",                        0x103382c0)
	MCFG_PCI_BRIDGE_ADD(              ":pci:1c.0",      0x808625ae, 0x02)
	MCFG_I82541PI_ADD(                ":pci:1c.0:00.0",                   0x103382c0)
	MCFG_USB_UHCI_ADD(                ":pci:1d.0",      0x808625a9, 0x02, 0x103382c0)
	MCFG_USB_UHCI_ADD(                ":pci:1d.1",      0x808625aa, 0x02, 0x103382c0)
	MCFG_I6300ESB_WATCHDOG_ADD(       ":pci:1d.4",                        0x103382c0)
	MCFG_APIC_ADD(                    ":pci:1d.5",      0x808625ac, 0x02, 0x103382c0)
	MCFG_USB_EHCI_ADD(                ":pci:1d.7",      0x808625ad, 0x02, 0x103382c0)
	MCFG_PCI_BRIDGE_ADD(              ":pci:1e.0",      0x8086244e, 0x0a)
	MCFG_SB0400_ADD(                  ":pci:1e.0:02.0",                   0x11021101)
	MCFG_SEGA_LINDBERGH_BASEBOARD_ADD(":pci:1e.0:03.0")
	MCFG_I6300ESB_LPC_ADD(            ":pci:1f.0")
	MCFG_LPC_ACPI_ADD(                ":pci:1f.0:acpi")
	MCFG_LPC_RTC_ADD(                 ":pci:1f.0:rtc")
	MCFG_LPC_PIT_ADD(                 ":pci:1f.0:pit")
	MCFG_SATA_ADD(                    ":pci:1f.2",      0x808625a3, 0x02, 0x103382c0)
	MCFG_SMBUS_ADD(                   ":pci:1f.3",      0x808625a4, 0x02, 0x103382c0)
	MCFG_AC97_ADD(                    ":pci:1f.5",      0x808625a6, 0x02, 0x103382c0)
MACHINE_CONFIG_END

#define LINDBERGH_BIOS \
	ROM_REGION32_LE(0x100000, ":pci:1f.0", 0) /* PC bios, location 3j7 */ \
	ROM_SYSTEM_BIOS(0, "bios0", "6.0.0010 alternate version") \
	ROMX_LOAD("6.0.0010a.bin", 0x00000, 0x100000, CRC(10dd9b76) SHA1(1fdf1f921bc395846a7c3180fbdbc4ca287a9670), ROM_BIOS(1) ) \
	ROM_SYSTEM_BIOS(1, "bios1", "6.0.0009") \
	ROMX_LOAD("6.0.0009.bin",  0x00000, 0x100000, CRC(5ffdfbf8) SHA1(605bc4967b749b4e6d13fc2ebb845ba956a259a7), ROM_BIOS(2) ) \
	ROM_SYSTEM_BIOS(2, "bios2", "6.0.0010") \
	ROMX_LOAD("6.0.0010.bin",  0x00000, 0x100000, CRC(ea2bf888) SHA1(c9c5b6f0d4f4f36620939b15dd2f128a74347e37), ROM_BIOS(3) ) \
\
	ROM_REGION(0x400000, ":pci:1e.0:03.0", 0) /* Baseboard MPC firmware */ \
	ROM_LOAD("fpr-24370b.ic6", 0x000000, 0x400000, CRC(c3b021a4) SHA1(1b6938a50fe0e4ae813864649eb103838c399ac0)) \
\
	ROM_REGION32_LE(0x10000, ":pci:01.0:00.0", 0) /* Geforce bios extension (custom for the card) */ \
	ROM_LOAD("vid_bios.u504", 0x00000, 0x10000, CRC(f78d14d7) SHA1(f129787e487984edd23bf344f2e9500c85052275)) \
	DISK_REGION("cf") \
	DISK_IMAGE_READONLY("mda-c0004a_revb_lindyellow_v2.4.20_mvl31a_boot_2.01", 0, SHA1(e13da5f827df852e742b594729ee3f933b387410))


ROM_START(lindbios)
	LINDBERGH_BIOS
ROM_END

ROM_START(hotd4)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0427 / 317-0427-COM
	ROM_LOAD("317-0427-com.bin", 0, 0x2000, CRC(ef4a120c) SHA1(fcc0386fa708af9e010e40e1d259a6bd95e8b9e2))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0003a", 0, SHA1(46544e28735f55418dd78bd19446093874438264))
ROM_END

ROM_START(vf5)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0438 / 317-0438-COM
	ROM_LOAD("317-0438-com.bin", 0, 0x2000, CRC(9aeb15d3) SHA1(405ddc44b2b40b72cfe2a081a0d5e43ceb9a380e))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0008e", 0, NO_DUMP)
ROM_END

ROM_START(abclimax)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0440 / 317-0440-COM
	ROM_LOAD("317-0440-com.bin", 0, 0x2000, CRC(8d09e717) SHA1(6b25982f7042541874115d33ea5d0c028140a962))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0009", 0, NO_DUMP)
ROM_END

ROM_START(letsgoju)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0442 / 317-0442-COM
	ROM_LOAD("317-0442-com.bin", 0, 0x2000, CRC(b706efbb) SHA1(97c2b65e521113c5201f0b588fcb37a39148a637))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0011", 0, NO_DUMP)
ROM_END

ROM_START(outr2sdx)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0452 / 317-0452-COM (to verify, may be the one for OutRun 2 Special Tours)
	ROM_LOAD("317-0452-com.bin", 0, 0x2000, CRC(f5b7bb3f) SHA1(6b179b255b3d29e5ce61902eeae4da07177a2943))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0015a", 0, NO_DUMP)
ROM_END

ROM_START(psmash3)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0434 / 317-0434-COM
	ROM_LOAD("317-0434-com.bin", 0, 0x2000, CRC(70e3b202) SHA1(4925a288f937d54529abe6ef467c9c23674e47f0))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0005a", 0, NO_DUMP)
ROM_END

ROM_START(vtennis3)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0506 / 317-0506-JPN
	ROM_LOAD("317-0506-jpn.bin", 0, 0x2000, NO_DUMP)

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0005c", 0, SHA1(1fd689753c4b70dff0286cb7f623ee7fd439db62))
ROM_END

ROM_START(2spicy)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0491 / 317-0491-COM
	ROM_LOAD("317-0491-com.bin", 0, 0x2000, NO_DUMP)

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0027a", 0, SHA1(da1aacee9e32e813844f4d434981e69cc5c80682))
ROM_END

ROM_START(ghostsev)
	LINDBERGH_BIOS

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0029a", 0, SHA1(256d9e8a6d61e1bcf65b17b8ed70fbc58796f7b1))
ROM_END

ROM_START(initiad4)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0486E / 317-0486-COM
	ROM_LOAD("317-0486-com.bin", 0, 0x2000, NO_DUMP)

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0030d", 0, SHA1(e43e6d22fab4eceb81db8309e4634e049d9c41e6))
ROM_END

ROM_START(initiad4c)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0486E / 317-0486-COM
	ROM_LOAD("317-0486-com.bin", 0, 0x2000, NO_DUMP)

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0030c", 0, SHA1(b1919f28539afec4c4bc52357e5210a090b5ae32))
ROM_END

ROM_START(segartv)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0504 / 317-0504-COM
	ROM_LOAD("317-0504-com.bin", 0, 0x2000, CRC(ae7eaea8) SHA1(187e417e0b5543d95245364b547925426aa9f80e))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0044", 0, SHA1(914aa23ece8aaf0f1942f77272b3a87d10f7a7db))
ROM_END

ROM_START(hotdex)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0550 / 317-0550-JPN
	ROM_LOAD("317-0550-jpn.bin", 0, 0x2000, CRC(7e247f13) SHA1(d416b0e7742b32eb31443967e84ef93fc9e56dfb))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("hotdex", 0, NO_DUMP)
ROM_END

ROM_START(primevah)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0512 / 317-0512-COM
	ROM_LOAD("317-0512-com.bin", 0, 0x2000, NO_DUMP)

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0048a", 0, SHA1(0c3b87b7309cf67ece54fc5cd5bbcfc7dc04083f))
ROM_END

ROM_START(rambo)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security 253-5508-0540 / 317-0540-COM
	ROM_LOAD("317-0540-com.bin", 0, 0x2000, CRC(fd9a7bc0) SHA1(140b05573e25a41c1237c7a96c8e099efbfd75b8))

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0069", 0, SHA1(1f3401b652c45db2b843360aff9cda862c2832c0))
ROM_END

ROM_START(hummerxt)
	LINDBERGH_BIOS

	ROM_REGION(0x2000, ":pic", 0) // PIC security id unknown
	ROM_LOAD("hummerextreme.bin", 0, 0x2000, CRC(524bc69a) SHA1(c79b6bd384196c169e40e623f4c80c8b9eb11f81))
ROM_END

ROM_START(lbvbiosu)
	LINDBERGH_BIOS

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY("dvp-0021b", 0, SHA1(362ac028ba19ba4762678953a033034a5ee8ad53))
ROM_END

GAME(1999, lindbios,         0, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Sega Lindbergh Bios",                      MACHINE_IS_BIOS_ROOT)
GAME(2005, hotd4,     lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "House of the Dead 4 (Export)",             MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2005, vf5,       lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Virtua Fighter 5 (Export)",                MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2006, abclimax,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "After Burner Climax (Export)",             MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2006, letsgoju,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Let's Go Jungle (Export)",                 MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2006, outr2sdx,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "OutRun 2 SP SDX",                          MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2006, psmash3,   lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Power Smash 3 / Virtua Tennis 3 (Export)", MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2006, vtennis3,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Virtua Tennis 3 (Japan)",                  MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2007, 2spicy,    lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "2 Spicy",                                  MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2007, ghostsev,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Ghost Squad Evolution",                    MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2007, initiad4,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Initial D4 (Rev D)",                       MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2007, initiad4c, initiad4, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Initial D4 (Rev C)",                       MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2007, segartv,   lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Sega Race-TV (Export)",                    MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2008, hotdex,    lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "House of the Dead EX (Japan)",             MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2008, primevah,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Primeval Hunt",                            MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2008, rambo,     lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Rambo (Export)",                           MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(2009, hummerxt,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "Hummer Extreme",                           MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
GAME(200?, lbvbiosu,  lindbios, lindbergh, 0, driver_device, 0, ROT0, "Sega", "VBios updater",                            MACHINE_NOT_WORKING|MACHINE_UNEMULATED_PROTECTION|MACHINE_NO_SOUND)
