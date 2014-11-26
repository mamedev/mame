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
The main board also has a slot for a compact flash card. Primary storage media is DVD or HDD.
Both CF and HDD are locked and unreadable on a regular PC.

The familiar PIC is still present on the back of the system and likely decrypts the HDD and/or DVD.

On this red box the CPU is a Celeron D at 2.8GHz. RAM is 512M DDR PC3200
The box has Sega number 845-0001D-02


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
|34  |-------| 932S208DG           SIMM1(512M DDR PC3200)    |
|56  |6300ESB|                                               |
|78  |SL7XJ  |                     SIMM2                     |
|910 |       |                                               |
|1112|INTEL  |                     SIMM3                     |
|    |-------|                                               |
|32.768kHz         |---------|     SIMM4                     |
| 3V_BATT          |COMPACT  |                               |
|    MB_BIOS.3J7   |FLASH    |     IDE40                     |
|                  |SLOT     |     IDE40       ATX_POWER     |
|------------------|---------|-------------------------------|
Notes:
           CPU - Intel Celeron D 335 SL8HM 2.8GHz 256k L2 cache, 533MHz FSB
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
                 Revision C and E have been seen. There may be other revisions out there.
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

                        Sega Security
            Game        ID Number      Dongle Sticker
            -----------------------------------------
            Too Spicy   317-0490-COM   253-5508-0491


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
      RTL8201   - Realtek RTL8201 Single Chip Single Pport 10/100M Fast Ethernet IC (QFP48)
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
#include "video/gf6800gt.h"

class lindbergh_state : public driver_device
{
public:
	lindbergh_state(const machine_config &mconfig, device_type type, const char *tag);

	virtual void machine_start();
	virtual void machine_reset();
};

#if 0
static ADDRESS_MAP_START(lindbergh_map, AS_PROGRAM, 32, lindbergh_state)
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
										//  AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffffffff)
										//  AM_RANGE(0x000c0000, 0x000cffff) AM_ROM AM_REGION("vid_bios", 0)
//  0xd0000 - 0xdffff tested, wants 0x414d ("AM") in there
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROM AM_REGION("mb_bios", 0xf0000)
//  AM_RANGE(0xfd000000, 0xfd3fffff) AM_ROM AM_REGION("jvs_bios", 0)    /* Hack to see the data */
	AM_RANGE(0xfff00000, 0xffffffff) AM_ROM AM_REGION("mb_bios", 0)     /* System BIOS */
ADDRESS_MAP_END

static ADDRESS_MAP_START(lindbergh_io, AS_IO, 32, lindbergh_state)
//  AM_IMPORT_FROM(pcat32_io_common)

//  AM_RANGE(0x00e8, 0x00ef) AM_NOP
//  AM_RANGE(0x0cf8, 0x0cff) AM_DEVREADWRITE("pcibus", pci_bus_legacy_device, read, write)
ADDRESS_MAP_END
#endif

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
//  MCFG_CPU_ADD("maincpu", PENTIUM, 2800000000U) /* Actually Celeron D at 2,8 GHz */
	MCFG_CPU_ADD("maincpu", PENTIUM4, 28000000U*5) /* Actually Celeron D at 2,8 GHz */
//  MCFG_CPU_PROGRAM_MAP(lindbergh_map)
//  MCFG_CPU_IO_MAP(lindbergh_io)
//  MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

//  MCFG_FRAGMENT_ADD( pcat_common )
//  MCFG_FRAGMENT_ADD( pcvideo_vga )

//  MCFG_PCI_BUS_LEGACY_ADD("pcibus", 0)

	MCFG_PCI_ROOT_ADD(                ":pci")
	MCFG_I82875P_HOST_ADD(            ":pci:00.0",                        0x103382c0, ":maincpu", 512*1024*1024)
	MCFG_I82875P_AGP_ADD(             ":pci:01.0")
	MCFG_GEFORCE_6800GT_ADD(          ":pci:01.0:00.0",                   0x10de0204)
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
	MCFG_SATA_ADD(                    ":pci:1f.2",      0x808625a3, 0x02, 0x103382c0)
	MCFG_SMBUS_ADD(                   ":pci:1f.3",      0x808625a4, 0x02, 0x103382c0)
	MCFG_AC97_ADD(                    ":pci:1f.5",      0x808625a6, 0x02, 0x103382c0)
MACHINE_CONFIG_END

ROM_START(lindbios)
	ROM_REGION32_LE(0x100000, ":pci:1f.0", 0) // PC bios, location 3j7
	ROM_SYSTEM_BIOS(0, "bios0", "6.0.0010 alternate version")
	ROMX_LOAD("6.0.0010a.bin", 0x00000, 0x100000, CRC(10dd9b76) SHA1(1fdf1f921bc395846a7c3180fbdbc4ca287a9670), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "bios1", "6.0.0009")
	ROMX_LOAD("6.0.0009.bin", 0x00000, 0x100000, CRC(5ffdfbf8) SHA1(605bc4967b749b4e6d13fc2ebb845ba956a259a7), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS(2, "bios2", "6.0.0010")
	ROMX_LOAD("6.0.0010.bin", 0x00000, 0x100000, CRC(ea2bf888) SHA1(c9c5b6f0d4f4f36620939b15dd2f128a74347e37), ROM_BIOS(3) )


	ROM_REGION(0x400000, ":pci:1e.0:03.0", 0) // Baseboard MPC firmware
	ROM_LOAD("fpr-24370b.ic6", 0x000000, 0x400000, CRC(c3b021a4) SHA1(1b6938a50fe0e4ae813864649eb103838c399ac0))

	ROM_REGION32_LE(0x10000, ":pci:01.0:00.0", 0) // Geforce bios extension (custom or standard?)
	ROM_LOAD("vid_bios.u504", 0x00000, 0x10000, CRC(f78d14d7) SHA1(f129787e487984edd23bf344f2e9500c85052275))
ROM_END

GAME(1999, lindbios, 0, lindbergh, 0, driver_device, 0, ROT0, "Sega Lindbergh", "Sega Lindbergh Bios", GAME_IS_SKELETON)
