// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Atari/Midway Vegas hardware games

    driver by Aaron Giles

    Games supported:
        * Gauntlet Legends [200MHz R5000, 8MB RAM, Vegas + Vegas SIO + Voodoo 2, 2-TMU * 4MB]
        * War: Final Assault [250MHz R5000, 8MB RAM, Vegas + Vegas SIO + Voodoo 2, 2-TMU * 4MB]
        * NBA on NBC
        * Tenth Degree/ Juko Threat
        * NBA Showtime Gold + NFL Blitz 2000 Gold

    Durango PCB (uses an RM7000 or RM5271 @ 250MHz):
        * Gauntlet Dark Legacy [Atari, 200MHz]
        * Road Burners [250MHz QED5271, 32MB RAM, Durango + DSIO + Voodoo 2, 2-TMU * 4MB]
        * San Francisco Rush 2049 [250MHz RM7000, 32MB RAM, Durango + Denver + Voodoo 3, 16MB]
        * San Francisco Rush 2049 Tournament Edition (PIC ID = 348)
        * CART Fury

    Known bugs:
        * not working yet

***************************************************************************

    Interrupt summary:

                        __________
    UART clear-to-send |          |
    -------(0x2000)--->|          |
                       |          |
    UART data ready    |          |
    -------(0x1000)--->|          |                     __________
                       |          |   VSYNC            |          |
    Main-to-sound empty|  IOASIC  |   -------(0x20)--->|          |
    -------(0x0080)--->|          |                    |          |
                       |          |   Ethernet         |          |   SIO Summary
    Sound-to-main full |          |   -------(0x10)--->|   SIO    |--------------+
    -------(0x0040)--->|          |                    |          |              |
                       |          |                    |          |              |
    Sound FIFO empty   |          |   IOASIC Summary   |          |              |
    -------(0x0008)--->|          |----------(0x04)--->|          |              |
                       |__________|                    |__________|              |
                                                                                 |
 +-------------------------------------------------------------------------------+
 |
 |                      __________                      __________
 |  IDE Controller     |          |                    |          |
 |  -------(0x0800)--->|          |                    |          |
 |                     |          |----------(IRQ5)--->|          |
 |  SIO Summary        |          |                    |          |
 +---------(0x0400)--->|          |----------(IRQ4)--->|          |
                       |          |                    |          |
    Timer 2            |          |----------(IRQ3)--->|   CPU    |
    -------(0x0040)--->|   NILE   |                    |          |
                       |          |----------(IRQ2)--->|          |
    Timer 3            |          |                    |          |
    -------(0x0020)--->|          |----------(IRQ1)--->|          |
                       |          |                    |          |
    UART Transmit      |          |----------(IRQ0)--->|          |
    -------(0x0010)--->|          |                    |__________|
                       |__________|


***************************************************************************

 PCB Summary:

    Three boards per game. CPU board, sound I/O board, and a video board.

    CPU boards:
        Vegas    - R5000 @ 200-250MHz, 8MB RAM
        Vegas32  - R5000 @ 200-250MHz, 32MB RAM
        Durango  - RM7000 or RM5271 @ 250-300MHz, 8-32MB RAM

    Sound I/O boards:
        Vegas SIO  - ADSP2104 @ 16MHz, boot ROM, 4MB RAM
        Deluxe SIO - ADSP2181 @ 32MHz, no ROM, 4MB RAM
        Denver SIO - ADSP2181 @ 33MHz, no ROM, 4MB RAM

    Video boards:
        Voodoo 2
        Voodoo Banshee
        Voodoo 3


***************************************************************************

 Gauntlet Legends info:


 CPU PCB:  Vegas CPU  Midway no.5770-15563-06
 --------------------------------------------

 U16   33.3333MHz Oscillator (to U2 uPD82157N7-002)
 U9   100.0000MHz Oscillator (to U10 MPC948)

 U1   79RV5000-200      IDT 64-bit Processor     TBGA
 U2   uPD82157N7-002    NEC                      TBGA (huge)
 U3   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U4   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U5   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U6   uPD4516161ag5-A10 NEC 1Megx16 SDRAM        TSOP
 U13  EPM7032LC44-15T   Altera PLD               Midway no.A-22544
 U11  93LC46B           Microchip EEPROM         Midway no.A-22545
 U18  27C4001           Boot ROM
 U10  MPC948            Motorola low voltage clock distribution chip
 U21  PC1646U2          CMD EIDE Controller


 Sound I/O PCB: Vegas 7-7-7 SIO-4 PCB  Midway no.5770-15534-04
 -------------------------------------------------------------

 U1   33.3333MHz  Oscillator
 Y1   20.0000MHz  XTAL       (to U24 SMC91C94QFP)
 Y2    4.0000MHz  XTAL       (to U37 PIC micro)
 Y3   16.0000MHz  XTAL       (to U14 ADSP)

 U28  M4T28-BR12SH1    ST Timer Keeper Snap Hat RAM
 U14  ADSP2104         Analog Devices DSP
 U8   EPF6016TC144-2   Altera FLEX 6000 PLD    144 pin TQFP
 U11  5410-14589-00    Midway Custom           164 pin QFP
 U34  5410-14590-00    Midway Custom            80 pin PQFP
 U18  ADC0848CCN       NSC ADC
 U24  SMC91C94QFP      SMC ISA/PCMCIA Ethernet & Modem Controller
 U5   AD1866R          Analog Devices dual 16-bit audio DAC
 U31  IS61C256AH-15J   ISSI 32kx8 SRAM
 U32  IS61C256AH-15J   ISSI 32kx8 SRAM
 U33  IS61C256AH-15J   ISSI 32kx8 SRAM
 U25  TMS418160ADZ     TI 1048576x16 DRAM
 U26  TMS418160ADZ     TI 1048576x16 DRAM
 U37  PIC16C57C        Microchip PIC  Atari no.322 Gauntlet 27"
 U44  27C256           Vegas SIO Audio Boot ROM
 U1   AM7201-35JC      AMD opamp
 U2   AM7201-35JC      AMD opamp


 GFX Card: Quantum 3D - Obsidian2 PCI (3DFx Voodoo2)
 ---------------------------------------------------

 U1    500-0009-01      3DFX Pixel processor    256 pin PQFP
 U26   500-0010-01      3DFX Texture processor  208 pin PQFP
 U2    500-0010-01      3DFX Texture processor  208 pin PQFP
 U20   ICS5342-3        ICS DAC
 U21   XC9572           Xilinx CPLD Firmware 546-0014-02
 U53   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U54   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U55   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U56   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U37   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U38   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U39   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U40   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U45   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U46   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U47   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U48   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U49   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U50   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U51   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U52   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U57   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U58   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U59   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg
 U60   SM81C256K16CJ-25 Silicon Magic 100MHz EDO RAM, 4Meg


***************************************************************************

 Gauntlet Dark Legacy Info:


 CPU board: Durango 5770-15982-00    sticker with RM5271 on it
 --------------------------------
 1x RM5271 CPU(assuming this is correct...don't want to remove the heatsink from my good board)
 1x medium sized BGA, has heatsink
 1x Atmel ATF1500A  PLD  A-22912
 4x MT48LC1M16AT RAM
 1x 93clc46b       label A-22911   config eeprom
 1x texas instruments 8CA00YF (don't know what it is)
 1x motorolla MPC948 clock distribution chip
 100MHz crystal
 1x CMDPCI646U2 IDE controller
 1x 7segment LED display (cycles IOASIC if you try to load a game that doesnt match the PIC, spins during normal play)
 1x 232ACBN serial port controller
 other misc 74xxx parts
 Boot ROM 1.7

 Connectors:
   P1 4 pin marked "Reset/Interrupt"
   P2 20 pin marked "PCI Extend"
   P8B Standard PCI connector found on PC motherboards (although is custom mounted on the side of the PCB) Voodoo connects here
   P3 DB15 labeled "Video In" Voodoo connects here, but is not required if you connect directly to the Voodoo card.
   P4 Standard IDE connector marked "IDE HDD"
   P5 9 pin marked "Serial"
   Non-designated connector marked "EXP CONN" Standard PCI connector found on PC motherboards(custom mounted on the side of the PCB), SIO connects here



 SIO board: Vegas777 5770-15534-05
 ---------------------------------
 1x Midway 5410-14589-00 IO chip
 1x ADSP-2104  (16MHz crystal attached to clock pins)
 1x ADC0848CCN analog input chip
 1x ST M4t28-br12sh1  timekeeper
 1x Altera Flex epf6016tc144   pld
 1x Midway security PIC Gauntlet DL 27" 346xxxxxx (4MHz crystal attached)
 2x KM416C1200CJ-6  RAM
 1x SMSC LAN91C94 ethernet
 1x Midway 5410-14590-00 ???
 3x CY7C199-15VC  32Kx8 SRAM
 1x AD1866 DAC
 20MHz crystal attached to LAN chip
 other misc parts
 SIO ROM 1.0



 Voodoo card: Quantum3D Obsidian2 PCI 650-0818-03D
 -------------------------------------------------
 (SLI pin holes filled with solder, no connector, has no VGA input/passthrough connector, only VGA out)

 1x XC9572 label 546-0014-02
 20x silicon magic 100MHz   sm81c256k16cj-25   RAM (8MB for texture RAM, 2MB for framebuffer?)
 2x 3dfx 500-0010-01 texelfx
 1x 3dfx 500-0009-01 pixelfx
 1x 14.318MHz crystal
 1 x 3384q 10bit bus switch(i think)
 1x ICS GenDAC ICS5342-3
 other misc IC's


***************************************************************************

 Gauntlet Legends versus Gauntlet Dark Legacy


 CPU board: Vegas 5770-15563-06
 ------------------------------
 4x NEC D4516161A65-A10-9NF 8MB RAM  (other difference here...this PCB only has spots for these chips, the Durango has alternate positions for RAM, I assume for larger chips)
 1x 33.3333MHz crystal (in addition to the 100MHz crystal, on Durango this spot is present but unpopulated)
 1x Atmel ATF1500A  PLD A-22560 (replaces A-22912 on Durango)
 1x 93lc46b (A-22545 config EEPROM, dump is mostly FF and 00)
 Boot ROM "Gauntlet Update Boot v1.5 A-5343-30022-7"



 SIO board: Vegas777 5770-15534-05 (same as Legacy)
 --------------------------------------------------
 Security PIC "Gauntlet 27" 322xxxxxx"
 Sound ROM "Gauntlet 3D U44 Sound 1.0"
 2x TMS418160ADZ RAM instead of 2x KM416C1200CJ-6 on Legacy
 20MHz crystal attached to LAN chip
 1x Valor  SF1012 ethernet physical interface(also on other board)

 Connectors:
   P1 5 pin marked "Snd Line In"
   P5 5 pin marked "Snd Line Out"
   unmarked 4 pin PC style power connector(for hard drive)
   standard JAMMA connection
   P7 14 pin marked "PLYR4"
   P14 14 pin marked "PLYR3"
   P23 10 pin marked "Coin DR"
   P10 unmarked 10 pin connector (no idea)
   P21 99 pin unmarked -right on top of PCI edge connection to CPU board
   P15 20 pin marked "Aux Latched Out"
   P8 14 pin unmarked
   P2 11 pin marked "Gun 2 I/O"
   P4 11 pin marked "Gun 1 I/O"
   P 18 standard ethernet connector


**************************************************************************/

#include "emu.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/mips/mips3.h"
#include "audio/dcs.h"
#include "machine/timekpr.h"
#include "machine/idectrl.h"
#include "machine/midwayic.h"
#include "machine/smc91c9x.h"
#include "video/voodoo.h"


/*************************************
 *
 *  Debugging constants
 *
 *************************************/

#define LOG_NILE            (0)
#define LOG_NILE_IRQS       (0)
#define LOG_PCI             (0)
#define LOG_TIMERS          (0)
#define LOG_TIMEKEEPER      (0)
#define LOG_SIO             (0)
#define LOG_DYNAMIC         (0)
#define PRINTF_SERIAL       (1)



/*************************************
 *
 *  Core constants
 *
 *************************************/

#define SYSTEM_CLOCK        100000000
#define TIMER_PERIOD        attotime::from_hz(SYSTEM_CLOCK)

#define MAX_DYNAMIC_ADDRESSES   32

#define NOP_HANDLER         read32_delegate()



/*************************************
 *
 *  NILE constants
 *
 *************************************/

/* NILE 4 registers 0x000-0x0ff */
#define NREG_SDRAM0         (0x000/4)
#define NREG_SDRAM1         (0x008/4)
#define NREG_DCS2           (0x010/4)   /* SIO misc */
#define NREG_DCS3           (0x018/4)   /* ADC */
#define NREG_DCS4           (0x020/4)   /* CMOS */
#define NREG_DCS5           (0x028/4)   /* SIO */
#define NREG_DCS6           (0x030/4)   /* IOASIC */
#define NREG_DCS7           (0x038/4)   /* ethernet */
#define NREG_DCS8           (0x040/4)
#define NREG_PCIW0          (0x060/4)
#define NREG_PCIW1          (0x068/4)
#define NREG_INTCS          (0x070/4)
#define NREG_BOOTCS         (0x078/4)
#define NREG_CPUSTAT        (0x080/4)
#define NREG_INTCTRL        (0x088/4)
#define NREG_INTSTAT0       (0x090/4)
#define NREG_INTSTAT1       (0x098/4)
#define NREG_INTCLR         (0x0A0/4)
#define NREG_INTPPES        (0x0A8/4)
#define NREG_PCIERR         (0x0B8/4)
#define NREG_MEMCTRL        (0x0C0/4)
#define NREG_ACSTIME        (0x0C8/4)
#define NREG_CHKERR         (0x0D0/4)
#define NREG_PCICTRL        (0x0E0/4)
#define NREG_PCIARB         (0x0E8/4)
#define NREG_PCIINIT0       (0x0F0/4)
#define NREG_PCIINIT1       (0x0F8/4)

/* NILE 4 registers 0x100-0x1ff */
#define NREG_LCNFG          (0x100/4)
#define NREG_LCST2          (0x110/4)
#define NREG_LCST3          (0x118/4)
#define NREG_LCST4          (0x120/4)
#define NREG_LCST5          (0x128/4)
#define NREG_LCST6          (0x130/4)
#define NREG_LCST7          (0x138/4)
#define NREG_LCST8          (0x140/4)
#define NREG_DCSFN          (0x150/4)
#define NREG_DCSIO          (0x158/4)
#define NREG_BCST           (0x178/4)
#define NREG_DMACTRL0       (0x180/4)
#define NREG_DMASRCA0       (0x188/4)
#define NREG_DMADESA0       (0x190/4)
#define NREG_DMACTRL1       (0x198/4)
#define NREG_DMASRCA1       (0x1A0/4)
#define NREG_DMADESA1       (0x1A8/4)
#define NREG_T0CTRL         (0x1C0/4)
#define NREG_T0CNTR         (0x1C8/4)
#define NREG_T1CTRL         (0x1D0/4)
#define NREG_T1CNTR         (0x1D8/4)
#define NREG_T2CTRL         (0x1E0/4)
#define NREG_T2CNTR         (0x1E8/4)
#define NREG_T3CTRL         (0x1F0/4)
#define NREG_T3CNTR         (0x1F8/4)

/* NILE 4 registers 0x200-0x2ff */
#define NREG_VID            (0x200/4)
#define NREG_DID            (0x202/4)
#define NREG_PCICMD         (0x204/4)
#define NREG_PCISTS         (0x206/4)
#define NREG_REVID          (0x208/4)
#define NREG_CLASS          (0x209/4)
#define NREG_CLSIZ          (0x20C/4)
#define NREG_MLTIM          (0x20D/4)
#define NREG_HTYPE          (0x20E/4)
#define NREG_BIST           (0x20F/4)
#define NREG_BARC           (0x210/4)
#define NREG_BAR0           (0x218/4)
#define NREG_BAR1           (0x220/4)
#define NREG_CIS            (0x228/4)
#define NREG_SSVID          (0x22C/4)
#define NREG_SSID           (0x22E/4)
#define NREG_ROM            (0x230/4)
#define NREG_INTLIN         (0x23C/4)
#define NREG_INTPIN         (0x23D/4)
#define NREG_MINGNT         (0x23E/4)
#define NREG_MAXLAT         (0x23F/4)
#define NREG_BAR2           (0x240/4)
#define NREG_BAR3           (0x248/4)
#define NREG_BAR4           (0x250/4)
#define NREG_BAR5           (0x258/4)
#define NREG_BAR6           (0x260/4)
#define NREG_BAR7           (0x268/4)
#define NREG_BAR8           (0x270/4)
#define NREG_BARB           (0x278/4)

/* NILE 4 registers 0x300-0x3ff */
#define NREG_UARTRBR        (0x300/4)
#define NREG_UARTTHR        (0x300/4)
#define NREG_UARTIER        (0x308/4)
#define NREG_UARTDLL        (0x300/4)
#define NREG_UARTDLM        (0x308/4)
#define NREG_UARTIIR        (0x310/4)
#define NREG_UARTFCR        (0x310/4)
#define NREG_UARTLCR        (0x318/4)
#define NREG_UARTMCR        (0x320/4)
#define NREG_UARTLSR        (0x328/4)
#define NREG_UARTMSR        (0x330/4)
#define NREG_UARTSCR        (0x338/4)

/* NILE 4 interrupts */
#define NINT_CPCE           (0)
#define NINT_CNTD           (1)
#define NINT_MCE            (2)
#define NINT_DMA            (3)
#define NINT_UART           (4)
#define NINT_WDOG           (5)
#define NINT_GPT            (6)
#define NINT_LBRTD          (7)
#define NINT_INTA           (8)
#define NINT_INTB           (9)
#define NINT_INTC           (10)
#define NINT_INTD           (11)
#define NINT_INTE           (12)
#define NINT_RESV           (13)
#define NINT_PCIS           (14)
#define NINT_PCIE           (15)


struct dynamic_address
{
	offs_t          start;
	offs_t          end;
	read32_delegate   read;
	write32_delegate write;
};

class vegas_state : public driver_device
{
public:
	vegas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_timekeeper(*this, "timekeeper") ,
		m_ide(*this, "ide"),
		m_ethernet(*this, "ethernet"),
		m_rambase(*this, "rambase"),
		m_nile_regs(*this, "nile_regs"),
		m_rombase(*this, "rombase"),
		m_dcs(*this, "dcs"),
		m_ioasic(*this, "ioasic"),
		m_io_analog(*this, "AN")
	{ }

	required_device<mips3_device> m_maincpu;
	required_device<m48t37_device> m_timekeeper;
	required_device<bus_master_ide_controller_device> m_ide;
	required_device<smc91c94_device> m_ethernet;
	required_shared_ptr<UINT32> m_rambase;
	required_shared_ptr<UINT32> m_nile_regs;
	required_shared_ptr<UINT32> m_rombase;
	required_device<dcs_audio_device> m_dcs;
	required_device<midway_ioasic_device> m_ioasic;
	optional_ioport_array<8> m_io_analog;

	UINT16 m_nile_irq_state;
	UINT16 m_ide_irq_state;
	UINT32 m_pci_bridge_regs[0x40];
	UINT32 m_pci_ide_regs[0x40];
	UINT32 m_pci_3dfx_regs[0x40];
	emu_timer *m_timer[4];
	UINT8 m_vblank_state;
	UINT8 m_sio_data[4];
	UINT8 m_sio_irq_clear;
	UINT8 m_sio_irq_enable;
	UINT8 m_sio_irq_state;
	UINT8 m_sio_led_state;
	UINT8 m_pending_analog_read;
	UINT8 m_cmos_unlocked;
	device_t *m_voodoo;
	UINT8 m_dcs_idma_cs;
	int m_count;
	int m_dynamic_count;
	dynamic_address m_dynamic[MAX_DYNAMIC_ADDRESSES];
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	DECLARE_WRITE_LINE_MEMBER(vblank_assert);
	DECLARE_DRIVER_INIT(gauntleg);
	DECLARE_DRIVER_INIT(cartfury);
	DECLARE_DRIVER_INIT(tenthdeg);
	DECLARE_DRIVER_INIT(nbashowt);
	DECLARE_DRIVER_INIT(warfa);
	DECLARE_DRIVER_INIT(roadburn);
	DECLARE_DRIVER_INIT(sf2049te);
	DECLARE_DRIVER_INIT(gauntdl);
	DECLARE_DRIVER_INIT(nbanfl);
	DECLARE_DRIVER_INIT(sf2049);
	DECLARE_DRIVER_INIT(sf2049se);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_vegas(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(nile_timer_callback);
	void remap_dynamic_addresses();
	void update_nile_irqs();
	void update_sio_irqs();
	inline void _add_dynamic_address(offs_t start, offs_t end, read32_delegate read, write32_delegate write);

	DECLARE_WRITE32_MEMBER( cmos_unlock_w );
	DECLARE_WRITE32_MEMBER(timekeeper_w);
	DECLARE_READ32_MEMBER(timekeeper_r);
	DECLARE_READ32_MEMBER( pci_bridge_r );
	DECLARE_WRITE32_MEMBER( pci_bridge_w );
	DECLARE_READ32_MEMBER( pci_ide_r );
	DECLARE_WRITE32_MEMBER( pci_ide_w );
	DECLARE_READ32_MEMBER( pci_3dfx_r );
	DECLARE_WRITE32_MEMBER( pci_3dfx_w );
	DECLARE_READ32_MEMBER( nile_r );
	DECLARE_WRITE32_MEMBER( nile_w );
	DECLARE_READ32_MEMBER( sio_irq_clear_r );
	DECLARE_WRITE32_MEMBER( sio_irq_clear_w );
	DECLARE_READ32_MEMBER( sio_irq_enable_r );
	DECLARE_WRITE32_MEMBER( sio_irq_enable_w );
	DECLARE_READ32_MEMBER( sio_irq_cause_r );
	DECLARE_READ32_MEMBER( sio_irq_status_r );
	DECLARE_WRITE32_MEMBER( sio_led_w );
	DECLARE_READ32_MEMBER( sio_led_r );
	DECLARE_WRITE32_MEMBER( sio_w );
	DECLARE_READ32_MEMBER( sio_r );
	DECLARE_READ32_MEMBER( analog_port_r );
	DECLARE_WRITE32_MEMBER( analog_port_w );
	DECLARE_WRITE32_MEMBER( vegas_watchdog_w );
	DECLARE_WRITE32_MEMBER( asic_fifo_w );
	DECLARE_READ32_MEMBER( ide_main_r );
	DECLARE_WRITE32_MEMBER( ide_main_w );
	DECLARE_READ32_MEMBER( ide_alt_r );
	DECLARE_WRITE32_MEMBER( ide_alt_w );
	DECLARE_READ32_MEMBER( ide_bus_master32_r );
	DECLARE_WRITE32_MEMBER( ide_bus_master32_w );
	DECLARE_READ32_MEMBER( ethernet_r );
	DECLARE_WRITE32_MEMBER( ethernet_w );
	DECLARE_WRITE32_MEMBER( dcs3_fifo_full_w );
	DECLARE_WRITE_LINE_MEMBER(ethernet_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ioasic_irq);
};


/*************************************
 *
 *  Video start and update
 *
 *************************************/

UINT32 vegas_state::screen_update_vegas(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return voodoo_update(m_voodoo, bitmap, cliprect) ? 0 : UPDATE_HAS_NOT_CHANGED;
}


/*************************************
 *
 *  Machine init
 *
 *************************************/

void vegas_state::machine_start()
{
	m_voodoo = machine().device("voodoo");

	/* allocate timers for the NILE */
	m_timer[0] = machine().scheduler().timer_alloc(FUNC_NULL);
	m_timer[1] = machine().scheduler().timer_alloc(FUNC_NULL);
	m_timer[2] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vegas_state::nile_timer_callback),this));
	m_timer[3] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vegas_state::nile_timer_callback),this));

	/* identify our sound board */
	if (machine().device("dsio") != nullptr)
		m_dcs_idma_cs = 6;
	else if (machine().device("denver") != nullptr)
		m_dcs_idma_cs = 7;
	else
		m_dcs_idma_cs = 0;

	/* set the fastest DRC options, but strict verification */
	m_maincpu->mips3drc_set_options(MIPS3DRC_FASTEST_OPTIONS + MIPS3DRC_STRICT_VERIFY + MIPS3DRC_FLUSH_PC);

	/* configure fast RAM regions */
	m_maincpu->add_fastram(0x00000000, m_rambase.bytes() - 1, FALSE, m_rambase);
	m_maincpu->add_fastram(0x1fc00000, 0x1fc7ffff, TRUE, m_rombase);

	/* register for save states */
	save_item(NAME(m_nile_irq_state));
	save_item(NAME(m_ide_irq_state));
	save_item(NAME(m_pci_bridge_regs));
	save_item(NAME(m_pci_ide_regs));
	save_item(NAME(m_pci_3dfx_regs));
	save_item(NAME(m_vblank_state));
	save_item(NAME(m_sio_data));
	save_item(NAME(m_sio_irq_clear));
	save_item(NAME(m_sio_irq_enable));
	save_item(NAME(m_sio_irq_state));
	save_item(NAME(m_sio_led_state));
	save_item(NAME(m_pending_analog_read));
	save_item(NAME(m_cmos_unlocked));
	machine().save().register_postload(save_prepost_delegate(FUNC(vegas_state::remap_dynamic_addresses), this));
}


void vegas_state::machine_reset()
{
	/* reset dynamic addressing */
	memset(m_nile_regs, 0, 0x1000);
	memset(m_pci_ide_regs, 0, sizeof(m_pci_ide_regs));
	memset(m_pci_3dfx_regs, 0, sizeof(m_pci_3dfx_regs));

	/* reset the DCS system if we have one */
	if (machine().device("dcs") != nullptr)
	{
		m_dcs->reset_w(1);
		m_dcs->reset_w(0);
	}

	/* initialize IRQ states */
	m_ide_irq_state = 0;
	m_nile_irq_state = 0;
	m_sio_irq_state = 0;
}


/*************************************
 *
 *  Timekeeper access
 *
 *************************************/

WRITE32_MEMBER( vegas_state::cmos_unlock_w )
{
	m_cmos_unlocked = 1;
}


WRITE32_MEMBER( vegas_state::timekeeper_w )
{
	if (m_cmos_unlocked)
	{
		if ((mem_mask & 0x000000ff) != 0)
			m_timekeeper->write(space, offset * 4 + 0, data >> 0, 0xff);
		if ((mem_mask & 0x0000ff00) != 0)
			m_timekeeper->write(space, offset * 4 + 1, data >> 8, 0xff);
		if ((mem_mask & 0x00ff0000) != 0)
			m_timekeeper->write(space, offset * 4 + 2, data >> 16, 0xff);
		if ((mem_mask & 0xff000000) != 0)
			m_timekeeper->write(space, offset * 4 + 3, data >> 24, 0xff);
		if (offset*4 >= 0x7ff0)
			if (LOG_TIMEKEEPER) logerror("timekeeper_w(%04X & %08X) = %08X\n", offset*4, mem_mask, data);
		m_cmos_unlocked = 0;
	}
	else
		logerror("%08X:timekeeper_w(%04X,%08X & %08X) without CMOS unlocked\n", safe_pc(), offset, data, mem_mask);
}


READ32_MEMBER( vegas_state::timekeeper_r )
{
	UINT32 result = 0xffffffff;
	if ((mem_mask & 0x000000ff) != 0)
		result = (result & ~0x000000ff) | (m_timekeeper->read(space, offset * 4 + 0, 0xff) << 0);
	if ((mem_mask & 0x0000ff00) != 0)
		result = (result & ~0x0000ff00) | (m_timekeeper->read(space, offset * 4 + 1, 0xff) << 8);
	if ((mem_mask & 0x00ff0000) != 0)
		result = (result & ~0x00ff0000) | (m_timekeeper->read(space, offset * 4 + 2, 0xff) << 16);
	if ((mem_mask & 0xff000000) != 0)
		result = (result & ~0xff000000) | (m_timekeeper->read(space, offset * 4 + 3, 0xff) << 24);
	if (offset*4 >= 0x7ff0)
		if (LOG_TIMEKEEPER) logerror("timekeeper_r(%04X & %08X) = %08X\n", offset*4, mem_mask, result);
	return result;
}



/*************************************
 *
 *  PCI bridge accesses
 *
 *************************************/

READ32_MEMBER( vegas_state::pci_bridge_r )
{
	UINT32 result = m_pci_bridge_regs[offset];

	switch (offset)
	{
		case 0x00:      /* ID register: 0x005a = VRC 5074, 0x1033 = NEC */
			result = 0x005a1033;
			break;

		case 0x02:      /* revision ID register */
			result = 0x00000004;
			break;
	}

	if (LOG_PCI)
		logerror("%06X:PCI bridge read: reg %d = %08X\n", safe_pc(), offset, result);
	return result;
}


WRITE32_MEMBER( vegas_state::pci_bridge_w )
{
	m_pci_bridge_regs[offset] = data;
	if (LOG_PCI)
		logerror("%06X:PCI bridge write: reg %d = %08X\n", safe_pc(), offset, data);
}



/*************************************
 *
 *  PCI IDE accesses
 *
 *************************************/

READ32_MEMBER( vegas_state::pci_ide_r )
{
	UINT32 result = m_pci_ide_regs[offset];

	switch (offset)
	{
		case 0x00:      /* ID register: 0x0646 = 646 EIDE controller, 0x1095 = CMD */
			result = 0x06461095;
			break;

		case 0x14:      /* interrupt pending */
			result &= 0xffffff00;
			if (m_ide_irq_state)
				result |= 4;
			break;
	}

	if (LOG_PCI)
		logerror("%06X:PCI IDE read: reg %d = %08X\n", safe_pc(), offset, result);
	return result;
}


WRITE32_MEMBER( vegas_state::pci_ide_w )
{
	m_pci_ide_regs[offset] = data;

	switch (offset)
	{
		case 0x04:      /* address register */
			m_pci_ide_regs[offset] &= 0xfffffff0;
			remap_dynamic_addresses();
			break;

		case 0x05:      /* address register */
			m_pci_ide_regs[offset] &= 0xfffffffc;
			remap_dynamic_addresses();
			break;

		case 0x08:      /* address register */
			m_pci_ide_regs[offset] &= 0xfffffff0;
			remap_dynamic_addresses();
			break;

		case 0x14:      /* interrupt pending */
			if (data & 4)
				ide_interrupt(0);
			break;
	}
	if (LOG_PCI)
		logerror("%06X:PCI IDE write: reg %d = %08X\n", safe_pc(), offset, data);
}



/*************************************
 *
 *  PCI 3dfx accesses
 *
 *************************************/

READ32_MEMBER( vegas_state::pci_3dfx_r )
{
	int voodoo_type = voodoo_get_type(m_voodoo);
	UINT32 result = m_pci_3dfx_regs[offset];

	switch (offset)
	{
		case 0x00:      /* ID register: 0x0002 = SST-2, 0x121a = 3dfx */
			if (voodoo_type == TYPE_VOODOO_2)
				result = 0x0002121a;
			else
				result = 0x0003121a;
			break;

		case 0x02:      /* revision ID register */
			result = 0x00000002;
			break;

		case 0x10:      /* fab ID register?? */
			result = 0x00044000;
			break;

		case 0x15:      /* ???? -- gauntleg wants 0s in the bits below */
			result &= 0xf000ffff;
			break;
	}

	if (LOG_PCI)
		logerror("%06X:PCI 3dfx read: reg %d = %08X\n", safe_pc(), offset, result);
	return result;
}


WRITE32_MEMBER( vegas_state::pci_3dfx_w )
{
	int voodoo_type = voodoo_get_type(m_voodoo);

	m_pci_3dfx_regs[offset] = data;

	switch (offset)
	{
		case 0x04:      /* address register */
			if (voodoo_type == TYPE_VOODOO_2)
				m_pci_3dfx_regs[offset] &= 0xff000000;
			else
				m_pci_3dfx_regs[offset] &= 0xfe000000;
			remap_dynamic_addresses();
			break;

		case 0x05:      /* address register */
			if (voodoo_type >= TYPE_VOODOO_BANSHEE)
			{
				m_pci_3dfx_regs[offset] &= 0xfe000000;
				remap_dynamic_addresses();
			}
			break;

		case 0x06:      /* I/O register */
			if (voodoo_type >= TYPE_VOODOO_BANSHEE)
			{
				m_pci_3dfx_regs[offset] &= 0xffffff00;
				remap_dynamic_addresses();
			}
			break;

		case 0x0c:      /* romBaseAddr register */
			if (voodoo_type >= TYPE_VOODOO_BANSHEE)
			{
				m_pci_3dfx_regs[offset] &= 0xffff0000;
				remap_dynamic_addresses();
			}
			break;

		case 0x10:      /* initEnable register */
			voodoo_set_init_enable(m_voodoo, data);
			break;

	}
	if (LOG_PCI)
		logerror("%06X:PCI 3dfx write: reg %d = %08X\n", safe_pc(), offset, data);
}



/*************************************
 *
 *  nile timers & interrupts
 *
 *************************************/

void vegas_state::update_nile_irqs()
{
	UINT32 intctll = m_nile_regs[NREG_INTCTRL+0];
	UINT32 intctlh = m_nile_regs[NREG_INTCTRL+1];
	UINT8 irq[6];
	int i;

	/* check for UART transmit IRQ enable and synthsize one */
	if (m_nile_regs[NREG_UARTIER] & 2)
		m_nile_irq_state |= 0x0010;
	else
		m_nile_irq_state &= ~0x0010;

	irq[0] = irq[1] = irq[2] = irq[3] = irq[4] = irq[5] = 0;
	m_nile_regs[NREG_INTSTAT0+0] = 0;
	m_nile_regs[NREG_INTSTAT0+1] = 0;
	m_nile_regs[NREG_INTSTAT1+0] = 0;
	m_nile_regs[NREG_INTSTAT1+1] = 0;

	/* handle the lower interrupts */
	for (i = 0; i < 8; i++)
		if (m_nile_irq_state & (1 << i))
			if ((intctll >> (4*i+3)) & 1)
			{
				int vector = (intctll >> (4*i)) & 7;
				if (vector < 6)
				{
					irq[vector] = 1;
					m_nile_regs[NREG_INTSTAT0 + vector/2] |= 1 << (i + 16*(vector&1));
				}
			}

	/* handle the upper interrupts */
	for (i = 0; i < 8; i++)
		if (m_nile_irq_state & (1 << (i+8)))
			if ((intctlh >> (4*i+3)) & 1)
			{
				int vector = (intctlh >> (4*i)) & 7;
				if (vector < 6)
				{
					irq[vector] = 1;
					m_nile_regs[NREG_INTSTAT0 + vector/2] |= 1 << (i + 8 + 16*(vector&1));
				}
			}

	/* push out the state */
	if (LOG_NILE_IRQS) logerror("NILE IRQs:");
	for (i = 0; i < 6; i++)
	{
		if (irq[i])
		{
			if (LOG_NILE_IRQS) logerror(" 1");
			m_maincpu->set_input_line(MIPS3_IRQ0 + i, ASSERT_LINE);
		}
		else
		{
			if (LOG_NILE_IRQS) logerror(" 0");
			m_maincpu->set_input_line(MIPS3_IRQ0 + i, CLEAR_LINE);
		}
	}
	if (LOG_NILE_IRQS) logerror("\n");
}


TIMER_CALLBACK_MEMBER(vegas_state::nile_timer_callback)
{
	int which = param;
	UINT32 *regs = &m_nile_regs[NREG_T0CTRL + which * 4];
	if (LOG_TIMERS) logerror("timer %d fired\n", which);

	/* adjust the timer to fire again */
	{
		UINT32 scale = regs[0];
		if (regs[1] & 2)
			logerror("Unexpected value: timer %d is prescaled\n", which);
		if (scale != 0)
			m_timer[which]->adjust(TIMER_PERIOD * scale, which);
	}

	/* trigger the interrupt */
	if (which == 2)
		m_nile_irq_state |= 1 << 6;
	if (which == 3)
		m_nile_irq_state |= 1 << 5;

	update_nile_irqs();
}



/*************************************
 *
 *  Nile system controller
 *
 *************************************/

READ32_MEMBER( vegas_state::nile_r )
{
	UINT32 result = m_nile_regs[offset];
	int logit = 1, which;

	switch (offset)
	{
		case NREG_CPUSTAT+0:    /* CPU status */
		case NREG_CPUSTAT+1:    /* CPU status */
			if (LOG_NILE) logerror("%08X:NILE READ: CPU status(%03X) = %08X\n", safe_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTCTRL+0:    /* Interrupt control */
		case NREG_INTCTRL+1:    /* Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt control(%03X) = %08X\n", safe_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTSTAT0+0:   /* Interrupt status 0 */
		case NREG_INTSTAT0+1:   /* Interrupt status 0 */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt status 0(%03X) = %08X\n", safe_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTSTAT1+0:   /* Interrupt status 1 */
		case NREG_INTSTAT1+1:   /* Interrupt status 1 */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt status 1/enable(%03X) = %08X\n", safe_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTCLR+0:     /* Interrupt clear */
		case NREG_INTCLR+1:     /* Interrupt clear */
			if (LOG_NILE) logerror("%08X:NILE READ: interrupt clear(%03X) = %08X\n", safe_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_INTPPES+0:    /* PCI Interrupt control */
		case NREG_INTPPES+1:    /* PCI Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE READ: PCI interrupt control(%03X) = %08X\n", safe_pc(), offset*4, result);
			logit = 0;
			break;

		case NREG_PCIERR+0:     /* PCI error */
		case NREG_PCIERR+1:     /* PCI error */
		case NREG_PCICTRL+0:    /* PCI control */
		case NREG_PCICTRL+1:    /* PCI arbiter */
		case NREG_PCIINIT0+0:   /* PCI master */
		case NREG_PCIINIT0+1:   /* PCI master */
		case NREG_PCIINIT1+0:   /* PCI master */
		case NREG_PCIINIT1+1:   /* PCI master */
			logit = 0;
			break;

		case NREG_T0CNTR:       /* SDRAM timer control (counter) */
		case NREG_T1CNTR:       /* bus timeout timer control (counter) */
		case NREG_T2CNTR:       /* general purpose timer control (counter) */
		case NREG_T3CNTR:       /* watchdog timer control (counter) */
			which = (offset - NREG_T0CTRL) / 4;
			if (m_nile_regs[offset - 1] & 1)
			{
				if (m_nile_regs[offset] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				result = m_nile_regs[offset + 1] = m_timer[which]->remaining().as_double() * (double)SYSTEM_CLOCK;
			}

			if (LOG_TIMERS) logerror("%08X:NILE READ: timer %d counter(%03X) = %08X\n", safe_pc(), which, offset*4, result);
			logit = 0;
			break;

		case NREG_UARTIIR:          /* serial port interrupt ID */
			if (m_nile_regs[NREG_UARTIER] & 2)
				result = 0x02;          /* transmitter buffer IRQ pending */
			else
				result = 0x01;          /* no IRQ pending */
			break;

		case NREG_UARTLSR:          /* serial port line status */
			result = 0x60;
			logit = 0;
			break;

		case NREG_VID:
		case NREG_PCICMD:
		case NREG_REVID:
		case NREG_CLSIZ:
		case NREG_BARC:
		case NREG_BAR0:
		case NREG_BAR1:
		case NREG_CIS:
		case NREG_SSVID:
		case NREG_ROM:
		case NREG_INTLIN:
		case NREG_BAR2:
		case NREG_BAR3:
		case NREG_BAR4:
		case NREG_BAR5:
		case NREG_BAR6:
		case NREG_BAR7:
		case NREG_BAR8:
		case NREG_BARB:
			result = pci_bridge_r(space, offset & 0x3f, mem_mask);
			break;

	}

	if (LOG_NILE && logit)
		logerror("%06X:nile read from offset %03X = %08X\n", safe_pc(), offset*4, result);
	return result;
}


WRITE32_MEMBER( vegas_state::nile_w )
{
	UINT32 olddata = m_nile_regs[offset];
	int logit = 1, which;

	COMBINE_DATA(&m_nile_regs[offset]);

	switch (offset)
	{
		case NREG_CPUSTAT+0:    /* CPU status */
		case NREG_CPUSTAT+1:    /* CPU status */
			if (LOG_NILE) logerror("%08X:NILE WRITE: CPU status(%03X) = %08X & %08X\n", safe_pc(), offset*4, data, mem_mask);
			logit = 0;
			break;

		case NREG_INTCTRL+0:    /* Interrupt control */
		case NREG_INTCTRL+1:    /* Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt control(%03X) = %08X & %08X\n", safe_pc(), offset*4, data, mem_mask);
			logit = 0;
			update_nile_irqs();
			break;

		case NREG_INTSTAT0+0:   /* Interrupt status 0 */
		case NREG_INTSTAT0+1:   /* Interrupt status 0 */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt status 0(%03X) = %08X & %08X\n", safe_pc(), offset*4, data, mem_mask);
			logit = 0;
			update_nile_irqs();
			break;

		case NREG_INTSTAT1+0:   /* Interrupt status 1 */
		case NREG_INTSTAT1+1:   /* Interrupt status 1 */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt status 1/enable(%03X) = %08X & %08X\n", safe_pc(), offset*4, data, mem_mask);
			logit = 0;
			update_nile_irqs();
			break;

		case NREG_INTCLR+0:     /* Interrupt clear */
		case NREG_INTCLR+1:     /* Interrupt clear */
			if (LOG_NILE) logerror("%08X:NILE WRITE: interrupt clear(%03X) = %08X & %08X\n", safe_pc(), offset*4, data, mem_mask);
			logit = 0;
			m_nile_irq_state &= ~(m_nile_regs[offset] & ~0xf00);
			update_nile_irqs();
			break;

		case NREG_INTPPES+0:    /* PCI Interrupt control */
		case NREG_INTPPES+1:    /* PCI Interrupt control */
			if (LOG_NILE) logerror("%08X:NILE WRITE: PCI interrupt control(%03X) = %08X & %08X\n", safe_pc(), offset*4, data, mem_mask);
			logit = 0;
			break;

		case NREG_PCIERR+0:     /* PCI error */
		case NREG_PCIERR+1:     /* PCI error */
		case NREG_PCICTRL+0:    /* PCI control */
		case NREG_PCICTRL+1:    /* PCI arbiter */
		case NREG_PCIINIT0+0:   /* PCI master */
		case NREG_PCIINIT0+1:   /* PCI master */
		case NREG_PCIINIT1+1:   /* PCI master */
			logit = 0;
			break;

		case NREG_PCIINIT1+0:   /* PCI master */
			if (((olddata & 0xe) == 0xa) != ((m_nile_regs[offset] & 0xe) == 0xa))
				remap_dynamic_addresses();
			logit = 0;
			break;

		case NREG_T0CTRL+1:     /* SDRAM timer control (control bits) */
		case NREG_T1CTRL+1:     /* bus timeout timer control (control bits) */
		case NREG_T2CTRL+1:     /* general purpose timer control (control bits) */
		case NREG_T3CTRL+1:     /* watchdog timer control (control bits) */
			which = (offset - NREG_T0CTRL) / 4;
			if (LOG_NILE) logerror("%08X:NILE WRITE: timer %d control(%03X) = %08X & %08X\n", safe_pc(), which, offset*4, data, mem_mask);
			logit = 0;

			/* timer just enabled? */
			if (!(olddata & 1) && (m_nile_regs[offset] & 1))
			{
				UINT32 scale = m_nile_regs[offset + 1];
				if (m_nile_regs[offset] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				if (scale != 0)
					m_timer[which]->adjust(TIMER_PERIOD * scale, which);
				if (LOG_TIMERS) logerror("Starting timer %d at a rate of %d Hz\n", which, (int)ATTOSECONDS_TO_HZ((TIMER_PERIOD * (m_nile_regs[offset + 1] + 1)).attoseconds()));
			}

			/* timer disabled? */
			else if ((olddata & 1) && !(m_nile_regs[offset] & 1))
			{
				if (m_nile_regs[offset] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				m_nile_regs[offset + 1] = m_timer[which]->remaining().as_double() * SYSTEM_CLOCK;
				m_timer[which]->adjust(attotime::never, which);
			}
			break;

		case NREG_T0CNTR:       /* SDRAM timer control (counter) */
		case NREG_T1CNTR:       /* bus timeout timer control (counter) */
		case NREG_T2CNTR:       /* general purpose timer control (counter) */
		case NREG_T3CNTR:       /* watchdog timer control (counter) */
			which = (offset - NREG_T0CTRL) / 4;
			if (LOG_TIMERS) logerror("%08X:NILE WRITE: timer %d counter(%03X) = %08X & %08X\n", safe_pc(), which, offset*4, data, mem_mask);
			logit = 0;

			if (m_nile_regs[offset - 1] & 1)
			{
				if (m_nile_regs[offset - 1] & 2)
					logerror("Unexpected value: timer %d is prescaled\n", which);
				m_timer[which]->adjust(TIMER_PERIOD * m_nile_regs[offset], which);
			}
			break;

		case NREG_UARTTHR:      /* serial port output */
			if (PRINTF_SERIAL) osd_printf_debug("%c", data & 0xff);
			logit = 0;
			break;
		case NREG_UARTIER:      /* serial interrupt enable */
			update_nile_irqs();
			break;

		case NREG_VID:
		case NREG_PCICMD:
		case NREG_REVID:
		case NREG_CLSIZ:
		case NREG_BARC:
		case NREG_BAR0:
		case NREG_BAR1:
		case NREG_CIS:
		case NREG_SSVID:
		case NREG_ROM:
		case NREG_INTLIN:
		case NREG_BAR2:
		case NREG_BAR3:
		case NREG_BAR4:
		case NREG_BAR5:
		case NREG_BAR6:
		case NREG_BAR7:
		case NREG_BAR8:
		case NREG_BARB:
			pci_bridge_w(space, offset & 0x3f, data, mem_mask);
			break;

		case NREG_DCS2:
		case NREG_DCS3:
		case NREG_DCS4:
		case NREG_DCS5:
		case NREG_DCS6:
		case NREG_DCS7:
		case NREG_DCS8:
		case NREG_PCIW0:
		case NREG_PCIW1:
			remap_dynamic_addresses();
			break;
	}

	if (LOG_NILE && logit)
		logerror("%06X:nile write to offset %03X = %08X & %08X\n", safe_pc(), offset*4, data, mem_mask);
}



/*************************************
 *
 *  IDE interrupts
 *
 *************************************/

WRITE_LINE_MEMBER(vegas_state::ide_interrupt)
{
	m_ide_irq_state = state;
	if (state)
		m_nile_irq_state |= 0x800;
	else
		m_nile_irq_state &= ~0x800;
	update_nile_irqs();
}



/*************************************
 *
 *  SIO interrupts
 *
 *************************************/

void vegas_state::update_sio_irqs()
{
	if (m_sio_irq_state & m_sio_irq_enable)
		m_nile_irq_state |= 0x400;
	else
		m_nile_irq_state &= ~0x400;
	update_nile_irqs();
}


WRITE_LINE_MEMBER(vegas_state::vblank_assert)
{
	if (!m_vblank_state && state)
	{
		m_sio_irq_state |= 0x20;
		update_sio_irqs();
	}
	m_vblank_state = state;

	/* if we have stalled DMA, restart */
//  if (dma_pending_on_vblank[0]) { perform_dma(0); }
//  if (dma_pending_on_vblank[1]) { perform_dma(1); }
//  if (dma_pending_on_vblank[2]) { perform_dma(2); }
//  if (dma_pending_on_vblank[3]) { perform_dma(3); }
}


WRITE_LINE_MEMBER(vegas_state::ioasic_irq)
{
	if (state)
		m_sio_irq_state |= 0x04;
	else
		m_sio_irq_state &= ~0x04;
	update_sio_irqs();
}

WRITE_LINE_MEMBER(vegas_state::ethernet_interrupt)
{
	if (state)
		m_sio_irq_state |= 0x10;
	else
		m_sio_irq_state &= ~0x10;
	update_sio_irqs();
}


READ32_MEMBER( vegas_state::sio_irq_clear_r )
{
	return m_sio_irq_clear;
}


WRITE32_MEMBER( vegas_state::sio_irq_clear_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_sio_irq_clear = data;

		/* bit 0x01 seems to be used to reset the IOASIC */
		if (!(data & 0x01))
		{
			m_ioasic->ioasic_reset();
			m_dcs->reset_w(data & 0x01);
		}

		/* they toggle bit 0x08 low to reset the VBLANK */
		if (!(data & 0x08))
		{
			m_sio_irq_state &= ~0x20;
			update_sio_irqs();
		}
	}
}


READ32_MEMBER( vegas_state::sio_irq_enable_r )
{
	return m_sio_irq_enable;
}


WRITE32_MEMBER( vegas_state::sio_irq_enable_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_sio_irq_enable = data;
		update_sio_irqs();
	}
}


READ32_MEMBER( vegas_state::sio_irq_cause_r )
{
	return m_sio_irq_state & m_sio_irq_enable;
}


READ32_MEMBER( vegas_state::sio_irq_status_r )
{
	return m_sio_irq_state;
}


WRITE32_MEMBER( vegas_state::sio_led_w )
{
	if (ACCESSING_BITS_0_7)
		m_sio_led_state = data;
}


READ32_MEMBER( vegas_state::sio_led_r )
{
	return m_sio_led_state;
}



/*************************************
 *
 *  SIO FPGA accesses
 *
 *************************************/

WRITE32_MEMBER( vegas_state::sio_w )
{
	if (ACCESSING_BITS_0_7) offset += 0;
	if (ACCESSING_BITS_8_15) offset += 1;
	if (ACCESSING_BITS_16_23) offset += 2;
	if (ACCESSING_BITS_24_31) offset += 3;
	if (LOG_SIO && offset != 0)
		logerror("%08X:sio write to offset %X = %02X\n", safe_pc(), offset, data >> (offset*8));
	if (offset < 4)
		m_sio_data[offset] = data >> (offset*8);
	if (offset == 1)
		m_sio_data[2] = (m_sio_data[2] & ~0x02) | ((m_sio_data[1] & 0x01) << 1) | (m_sio_data[1] & 0x01);
}


READ32_MEMBER( vegas_state::sio_r )
{
	UINT32 result = 0;
	if (ACCESSING_BITS_0_7) offset += 0;
	if (ACCESSING_BITS_8_15) offset += 1;
	if (ACCESSING_BITS_16_23) offset += 2;
	if (ACCESSING_BITS_24_31) offset += 3;
	if (offset < 4)
		result = m_sio_data[0] | (m_sio_data[1] << 8) | (m_sio_data[2] << 16) | (m_sio_data[3] << 24);
	if (LOG_SIO && offset != 2)
		logerror("%08X:sio read from offset %X = %02X\n", safe_pc(), offset, result >> (offset*8));
	return result;
}



/*************************************
 *
 *  Analog input handling
 *
 *************************************/

READ32_MEMBER( vegas_state::analog_port_r )
{
	return m_pending_analog_read;
}


WRITE32_MEMBER( vegas_state::analog_port_w )
{
	if (data < 8 || data > 15)
		logerror("%08X:Unexpected analog port select = %08X\n", safe_pc(), data);
	m_pending_analog_read = m_io_analog[data & 7] ? m_io_analog[data & 7]->read() : 0;
}



/*************************************
 *
 *  Misc accesses
 *
 *************************************/

WRITE32_MEMBER( vegas_state::vegas_watchdog_w )
{
	space.device().execute().eat_cycles(100);
}


WRITE32_MEMBER( vegas_state::asic_fifo_w )
{
	m_ioasic->fifo_w(data);
}


READ32_MEMBER( vegas_state::ide_main_r )
{
	return m_ide->read_cs0(space, offset, mem_mask);
}


WRITE32_MEMBER( vegas_state::ide_main_w )
{
	m_ide->write_cs0(space, offset, data, mem_mask);
}


READ32_MEMBER( vegas_state::ide_alt_r )
{
	return m_ide->read_cs1(space, offset + 1, mem_mask);
}


WRITE32_MEMBER( vegas_state::ide_alt_w )
{
	m_ide->write_cs1(space, offset + 1, data, mem_mask);
}


READ32_MEMBER( vegas_state::ide_bus_master32_r )
{
	return m_ide->bmdma_r(space, offset, mem_mask);
}


WRITE32_MEMBER( vegas_state::ide_bus_master32_w )
{
	m_ide->bmdma_w(space, offset, data, mem_mask);
}


READ32_MEMBER( vegas_state::ethernet_r )
{
	UINT32 result = 0;
	if (ACCESSING_BITS_0_15)
		result |= m_ethernet->read(space, offset * 2 + 0, mem_mask);
	if (ACCESSING_BITS_16_31)
		result |= m_ethernet->read(space, offset * 2 + 1, mem_mask >> 16) << 16;
	return result;
}


WRITE32_MEMBER( vegas_state::ethernet_w )
{
	if (ACCESSING_BITS_0_15)
		m_ethernet->write(space, offset * 2 + 0, data, mem_mask);
	if (ACCESSING_BITS_16_31)
		m_ethernet->write(space, offset * 2 + 1, data >> 16, mem_mask >> 16);
}


WRITE32_MEMBER( vegas_state::dcs3_fifo_full_w )
{
	m_ioasic->fifo_full_w(data);
}



/*************************************
 *
 *  Dynamic addressing
 *
 *************************************/

#define add_dynamic_address(s,e,r,w)         _add_dynamic_address(s,e,r,w)


inline void vegas_state::_add_dynamic_address(offs_t start, offs_t end, read32_delegate read, write32_delegate write)
{
	dynamic_address *dynamic = m_dynamic;
	dynamic[m_dynamic_count].start = start;
	dynamic[m_dynamic_count].end = end;
	dynamic[m_dynamic_count].read = read;
	dynamic[m_dynamic_count].write = write;
	m_dynamic_count++;
}

void vegas_state::remap_dynamic_addresses()
{
	dynamic_address *dynamic = m_dynamic;
	int voodoo_type = voodoo_get_type(m_voodoo);
	offs_t base;
	int addr;

	/* unmap everything we know about */
	for (addr = 0; addr < m_dynamic_count; addr++)
		m_maincpu->space(AS_PROGRAM).unmap_readwrite(dynamic[addr].start, dynamic[addr].end);

	/* the build the list of stuff */
	m_dynamic_count = 0;

	/* DCS2 */
	base = m_nile_regs[NREG_DCS2] & 0x1fffff00;
	if (base >= m_rambase.bytes())
	{
		add_dynamic_address(base + 0x0000, base + 0x0003, read32_delegate(FUNC(vegas_state::sio_irq_clear_r), this), write32_delegate(FUNC(vegas_state::sio_irq_clear_w), this));
		add_dynamic_address(base + 0x1000, base + 0x1003, read32_delegate(FUNC(vegas_state::sio_irq_enable_r), this), write32_delegate(FUNC(vegas_state::sio_irq_enable_w), this));
		add_dynamic_address(base + 0x2000, base + 0x2003, read32_delegate(FUNC(vegas_state::sio_irq_cause_r), this), write32_delegate());
		add_dynamic_address(base + 0x3000, base + 0x3003, read32_delegate(FUNC(vegas_state::sio_irq_status_r), this), write32_delegate());
		add_dynamic_address(base + 0x4000, base + 0x4003, read32_delegate(FUNC(vegas_state::sio_led_r), this), write32_delegate(FUNC(vegas_state::sio_led_w), this));
		add_dynamic_address(base + 0x5000, base + 0x5007, NOP_HANDLER, write32_delegate());
		add_dynamic_address(base + 0x6000, base + 0x6003, read32_delegate(), write32_delegate(FUNC(vegas_state::cmos_unlock_w), this));
		add_dynamic_address(base + 0x7000, base + 0x7003, read32_delegate(), write32_delegate(FUNC(vegas_state::vegas_watchdog_w), this));
	}

	/* DCS3 */
	base = m_nile_regs[NREG_DCS3] & 0x1fffff00;
	if (base >= m_rambase.bytes())
		add_dynamic_address(base + 0x0000, base + 0x0003, read32_delegate(FUNC(vegas_state::analog_port_r), this), write32_delegate(FUNC(vegas_state::analog_port_w), this));

	/* DCS4 */
	base = m_nile_regs[NREG_DCS4] & 0x1fffff00;
	if (base >= m_rambase.bytes())
		add_dynamic_address(base + 0x0000, base + 0x7fff, read32_delegate(FUNC(vegas_state::timekeeper_r), this), write32_delegate(FUNC(vegas_state::timekeeper_w), this));

	/* DCS5 */
	base = m_nile_regs[NREG_DCS5] & 0x1fffff00;
	if (base >= m_rambase.bytes())
		add_dynamic_address(base + 0x0000, base + 0x0003, read32_delegate(FUNC(vegas_state::sio_r), this), write32_delegate(FUNC(vegas_state::sio_w), this));

	/* DCS6 */
	base = m_nile_regs[NREG_DCS6] & 0x1fffff00;
	if (base >= m_rambase.bytes())
	{
		add_dynamic_address(base + 0x0000, base + 0x003f, read32_delegate(FUNC(midway_ioasic_device::packed_r),(midway_ioasic_device*)m_ioasic), write32_delegate(FUNC(midway_ioasic_device::packed_w),(midway_ioasic_device*)m_ioasic));
		add_dynamic_address(base + 0x1000, base + 0x1003, read32_delegate(), write32_delegate(FUNC(vegas_state::asic_fifo_w), this));
		if (m_dcs_idma_cs != 0)
			add_dynamic_address(base + 0x3000, base + 0x3003, read32_delegate(), write32_delegate(FUNC(vegas_state::dcs3_fifo_full_w), this));
		if (m_dcs_idma_cs == 6)
		{
			add_dynamic_address(base + 0x5000, base + 0x5003, read32_delegate(), write32_delegate(FUNC(dcs_audio_device::dsio_idma_addr_w),(dcs_audio_device*)m_dcs));
			add_dynamic_address(base + 0x7000, base + 0x7003, read32_delegate(FUNC(dcs_audio_device::dsio_idma_data_r),(dcs_audio_device*)m_dcs), write32_delegate(FUNC(dcs_audio_device::dsio_idma_data_w),(dcs_audio_device*)m_dcs));
		}
	}

	/* DCS7 */
	base = m_nile_regs[NREG_DCS7] & 0x1fffff00;
	if (base >= m_rambase.bytes())
	{
		add_dynamic_address(base + 0x1000, base + 0x100f, read32_delegate(FUNC(vegas_state::ethernet_r), this), write32_delegate(FUNC(vegas_state::ethernet_w), this));
		if (m_dcs_idma_cs == 7)
		{
			add_dynamic_address(base + 0x5000, base + 0x5003, read32_delegate(), write32_delegate(FUNC(dcs_audio_device::dsio_idma_addr_w),(dcs_audio_device*)m_dcs));
			add_dynamic_address(base + 0x7000, base + 0x7003, read32_delegate(FUNC(dcs_audio_device::dsio_idma_data_r),(dcs_audio_device*)m_dcs), write32_delegate(FUNC(dcs_audio_device::dsio_idma_data_w),(dcs_audio_device*)m_dcs));
		}
	}

	/* PCI config space */
	if ((m_nile_regs[NREG_PCIINIT1] & 0xe) == 0xa)
	{
		base = m_nile_regs[NREG_PCIW1] & 0x1fffff00;
		if (base >= m_rambase.bytes())
		{
			add_dynamic_address(base + (1 << (21 + 4)) + 0x0000, base + (1 << (21 + 4)) + 0x00ff, read32_delegate(FUNC(vegas_state::pci_3dfx_r), this),  write32_delegate(FUNC(vegas_state::pci_3dfx_w), this));
			add_dynamic_address(base + (1 << (21 + 5)) + 0x0000, base + (1 << (21 + 5)) + 0x00ff, read32_delegate(FUNC(vegas_state::pci_ide_r), this),  write32_delegate(FUNC(vegas_state::pci_ide_w), this));
		}
	}

	/* PCI real space */
	else
	{
		/* IDE controller */
		base = m_pci_ide_regs[0x04] & 0xfffffff0;
		if (base >= m_rambase.bytes() && base < 0x20000000)
			add_dynamic_address(base + 0x0000, base + 0x000f, read32_delegate(FUNC(vegas_state::ide_main_r), this),  write32_delegate(FUNC(vegas_state::ide_main_w), this));

		base = m_pci_ide_regs[0x05] & 0xfffffffc;
		if (base >= m_rambase.bytes() && base < 0x20000000)
			add_dynamic_address(base + 0x0000, base + 0x0003, read32_delegate(FUNC(vegas_state::ide_alt_r), this), write32_delegate(FUNC(vegas_state::ide_alt_w), this));

		base = m_pci_ide_regs[0x08] & 0xfffffff0;
		if (base >= m_rambase.bytes() && base < 0x20000000)
			add_dynamic_address(base + 0x0000, base + 0x0007, read32_delegate(FUNC(vegas_state::ide_bus_master32_r), this), write32_delegate(FUNC(vegas_state::ide_bus_master32_w), this));

		/* 3dfx card */
		base = m_pci_3dfx_regs[0x04] & 0xfffffff0;
		if (base >= m_rambase.bytes() && base < 0x20000000)
		{
			if (voodoo_type == TYPE_VOODOO_2)
				add_dynamic_address(base + 0x000000, base + 0xffffff, read32_delegate(FUNC(voodoo_device::voodoo_r),(voodoo_device*)m_voodoo), write32_delegate(FUNC(voodoo_device::voodoo_w),(voodoo_device*)m_voodoo));
			else
				add_dynamic_address(base + 0x000000, base + 0x1ffffff, read32_delegate(FUNC(voodoo_banshee_device::banshee_r),(voodoo_banshee_device*)m_voodoo), write32_delegate(FUNC(voodoo_banshee_device::banshee_w),(voodoo_banshee_device*)m_voodoo));
		}

		if (voodoo_type >= TYPE_VOODOO_BANSHEE)
		{
			base = m_pci_3dfx_regs[0x05] & 0xfffffff0;
			if (base >= m_rambase.bytes() && base < 0x20000000)
				add_dynamic_address(base + 0x0000000, base + 0x1ffffff, read32_delegate(FUNC(voodoo_banshee_device::banshee_fb_r),(voodoo_banshee_device*)m_voodoo), write32_delegate(FUNC(voodoo_banshee_device::banshee_fb_w),(voodoo_banshee_device*)m_voodoo));

			base = m_pci_3dfx_regs[0x06] & 0xfffffff0;
			if (base >= m_rambase.bytes() && base < 0x20000000)
				add_dynamic_address(base + 0x0000000, base + 0x00000ff, read32_delegate(FUNC(voodoo_banshee_device::banshee_io_r),(voodoo_banshee_device*)m_voodoo), write32_delegate(FUNC(voodoo_banshee_device::banshee_io_w),(voodoo_banshee_device*)m_voodoo));

			base = m_pci_3dfx_regs[0x0c] & 0xffff0000;
			if (base >= m_rambase.bytes() && base < 0x20000000)
				add_dynamic_address(base + 0x0000000, base + 0x000ffff, read32_delegate(FUNC(voodoo_banshee_device::banshee_rom_r),(voodoo_banshee_device*)m_voodoo), write32_delegate());
		}
	}

	/* now remap everything */
	if (LOG_DYNAMIC) logerror("remap_dynamic_addresses:\n");
	address_space &space = m_maincpu->space(AS_PROGRAM);
	for (addr = 0; addr < m_dynamic_count; addr++)
	{
		if (LOG_DYNAMIC) logerror("  installing: %08X-%08X \n", dynamic[addr].start, dynamic[addr].end);

		if (dynamic[addr].read == NOP_HANDLER)
			space.nop_read(dynamic[addr].start, dynamic[addr].end);
		else if (!dynamic[addr].read.isnull())
			space.install_read_handler(dynamic[addr].start, dynamic[addr].end, 0, 0, dynamic[addr].read);
		if (!dynamic[addr].write.isnull())
			space.install_write_handler(dynamic[addr].start, dynamic[addr].end, 0, 0, dynamic[addr].write);
	}

	if (LOG_DYNAMIC)
	{
		++m_count;
		popmessage("Remaps = %d", m_count);
	}
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( vegas_map_8mb, AS_PROGRAM, 32, vegas_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x1fa00000, 0x1fa00fff) AM_READWRITE(nile_r, nile_w) AM_SHARE("nile_regs")

	AM_RANGE(0x01700000, 0x017fffff) AM_ROM AM_REGION("update", 0)

	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("rombase")
ADDRESS_MAP_END


static ADDRESS_MAP_START( vegas_map_32mb, AS_PROGRAM, 32, vegas_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x01ffffff) AM_RAM AM_SHARE("rambase")
	AM_RANGE(0x1fa00000, 0x1fa00fff) AM_READWRITE(nile_r, nile_w) AM_SHARE("nile_regs")


	AM_RANGE(0x01700000, 0x017fffff) AM_ROM AM_REGION("update", 0)

	AM_RANGE(0x1fc00000, 0x1fc7ffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("rombase")
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( vegas_common )
	PORT_START("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "Unknown0001" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, "Unknown0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown0040" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown0080" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Unknown0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown4000" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown8000" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT )
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( gauntleg )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "SIO Rev" )
	PORT_DIPSETTING(      0x0800, "1 or later")
	PORT_DIPSETTING(      0x0000, "0")
	PORT_DIPNAME( 0x1000, 0x1000, "Harness" )
	PORT_DIPSETTING(      0x1000, "JAMMA" )
	PORT_DIPSETTING(      0x0000, "Midway" )
	PORT_DIPNAME( 0x2000, 0x2000, "Joysticks" )
	PORT_DIPSETTING(      0x2000, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )
INPUT_PORTS_END


static INPUT_PORTS_START( gauntdl )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0002, 0x0002, "Quantum 3dfx card rev" )
	PORT_DIPSETTING(      0x0002, "4 or later" )
	PORT_DIPSETTING(      0x0000, "3 or earlier" )
	PORT_DIPNAME( 0x0004, 0x0004, "DRAM" )
	PORT_DIPSETTING(      0x0004, "8MB" )
	PORT_DIPSETTING(      0x0000, "32MB" )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "SIO Rev" )
	PORT_DIPSETTING(      0x0800, "1 or later")
	PORT_DIPSETTING(      0x0000, "0")
	PORT_DIPNAME( 0x1000, 0x1000, "Harness" )
	PORT_DIPSETTING(      0x1000, "JAMMA" )
	PORT_DIPSETTING(      0x0000, "Midway" )
	PORT_DIPNAME( 0x2000, 0x2000, "Joysticks" )
	PORT_DIPSETTING(      0x2000, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )
INPUT_PORTS_END


static INPUT_PORTS_START( tenthdeg )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)   /* P1 roundhouse */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)   /* P1 fierce */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)   /* P1 forward */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)   /* P2 roundhouse */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)   /* P2 forward */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)   /* P2 fierce */
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( warfa )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0001, "PM Dump" )
	PORT_DIPSETTING(      0x0001, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0002, 0x0002, "Quantum 3dfx card rev" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "?" )
	PORT_DIPNAME( 0x0040, 0x0040, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0x4000, "Resolution" )
	PORT_DIPSETTING(      0xc000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x4000, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START("AN.0")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN.1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN.2")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( roadburn )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0002, 0x0002, "Boot ROM Test" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Quantum 3dfx card rev" )
	PORT_DIPSETTING(      0x0040, "4" )
	PORT_DIPSETTING(      0x0000, "?" )
	PORT_DIPNAME( 0x0080, 0x0080, "PM Dump" )
	PORT_DIPSETTING(      0x0080, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0300, 0x0300, "Resolution" )
	PORT_DIPSETTING(      0x0300, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x0200, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0000, "VGA Res 640x480" )

	PORT_START("AN.0")   /* Steer */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("AN.1")   /* Accel */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("AN.2")   /* Brake */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(100)

	PORT_START("AN.3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( nbashowt )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0001, 0x0000, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x000e, 0x000e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x000e, "Mode 1" )
	PORT_DIPSETTING(      0x0008, "Mode 2" )
	PORT_DIPSETTING(      0x0009, "Mode 3" )
	PORT_DIPSETTING(      0x0002, "Mode 4" )
	PORT_DIPSETTING(      0x000c, "Mode ECA" )
//  PORT_DIPSETTING(      0x0004, "Not Used 1" )        /* Marked as Unused in the manual */
//  PORT_DIPSETTING(      0x0008, "Not Used 2" )        /* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0030, 0x0030, "Curency Type" )
	PORT_DIPSETTING(      0x0030, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( German ) )
//  PORT_DIPSETTING(      0x0000, "Not Used" )      /* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Unknown ) )  /* Marked as Unused in the manual */
	PORT_DIPSETTING(      0x0040, "0" )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPNAME( 0x0080, 0x0080, "Game" )
	PORT_DIPSETTING(      0x0080, "NBA Showtime" )
	PORT_DIPSETTING(      0x0000, "Blitz 99" )
	PORT_DIPNAME( 0x0100, 0x0100, "Joysticks" )
	PORT_DIPSETTING(      0x0100, "8-Way" )
	PORT_DIPSETTING(      0x0000, "49-Way" )
	PORT_DIPNAME( 0x0600, 0x0200, "Graphics Mode" )
	PORT_DIPSETTING(      0x0200, "512x385 @ 25KHz" )
	PORT_DIPSETTING(      0x0400, "512x256 @ 15KHz" )
//  PORT_DIPSETTING(      0x0600, "0" )         /* Marked as Unused in the manual */
//  PORT_DIPSETTING(      0x0000, "3" )         /* Marked as Unused in the manual */
	PORT_DIPNAME( 0x1800, 0x1800, "Graphics Speed" )
	PORT_DIPSETTING(      0x0000, "45 MHz" )
	PORT_DIPSETTING(      0x0800, "47 MHz" )
	PORT_DIPSETTING(      0x1000, "49 MHz" )
	PORT_DIPSETTING(      0x1800, "51 MHz" )
	PORT_DIPNAME( 0x2000, 0x0000, "Number of Players" )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x4000, 0x0000, "Power On Self Test" )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Test Switch" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sf2049 )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")
	PORT_DIPNAME( 0x0003, 0x0003, "Test Mode" )
	PORT_DIPSETTING(      0x0003, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0002, "Disk-based Test" )
	PORT_DIPSETTING(      0x0001, "EPROM-based Test" )
	PORT_DIPSETTING(      0x0000, "Interactive Diagnostics" )
	PORT_DIPNAME( 0x0080, 0x0080, "PM Dump" )
	PORT_DIPSETTING(      0x0080, "Watchdog resets only" )
	PORT_DIPSETTING(      0x0000, "All resets" )
	PORT_DIPNAME( 0x0100, 0x0200, "Resolution" )
	PORT_DIPSETTING(      0x0000, "Standard Res 512x256" )
	PORT_DIPSETTING(      0x0200, "Medium Res 512x384" )
	PORT_DIPSETTING(      0x0300, "VGA Res 640x480" )

	PORT_START("AN.0")   /* Steer */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("AN.1")   /* Accel */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("AN.2")   /* Brake */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(100)

	PORT_START("AN.3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( sf2049se )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")

	PORT_START("AN.0")   /* Steer */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("AN.1")   /* Accel */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("AN.2")   /* Brake */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(100)

	PORT_START("AN.3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( sf2049te )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")

	PORT_START("AN.0")   /* Steer */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("AN.1")   /* Accel */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("AN.2")   /* Brake */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(100)

	PORT_START("AN.3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END


static INPUT_PORTS_START( cartfury )
	PORT_INCLUDE(vegas_common)

	PORT_MODIFY("DIPS")

	PORT_START("AN.0")   /* Steer */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(5)

	PORT_START("AN.1")   /* Accel */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("AN.2")   /* Brake */
	PORT_BIT( 0xff, 0x80, IPT_PEDAL2 ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(100)

	PORT_START("AN.3")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.4")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.5")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.6")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )

	PORT_START("AN.7")
	PORT_BIT( 0xff, 0x80, IPT_SPECIAL )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( vegascore, vegas_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", R5000LE, SYSTEM_CLOCK*2)
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)
	MCFG_MIPS3_SYSTEM_CLOCK(SYSTEM_CLOCK)
	MCFG_CPU_PROGRAM_MAP(vegas_map_8mb)

	MCFG_M48T37_ADD("timekeeper")

	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, true)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(vegas_state, ide_interrupt))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE("maincpu", AS_PROGRAM)

	MCFG_SMC91C94_ADD("ethernet")
	MCFG_SMC91C94_IRQ_CALLBACK(WRITELINE(vegas_state, ethernet_interrupt))

	MCFG_DEVICE_ADD("voodoo", VOODOO_2, STD_VOODOO_2_CLOCK)
	MCFG_VOODOO_FBMEM(2)
	MCFG_VOODOO_TMUMEM(4,4)
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("maincpu")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(vegas_state,vblank_assert))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(vegas_state, screen_update_vegas)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vegas, vegascore )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vegas250, vegascore )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(SYSTEM_CLOCK*2.5)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vegas32m, vegascore )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(vegas_map_32mb)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vegasban, vegascore )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(vegas_map_32mb)

	MCFG_DEVICE_REMOVE("voodoo")
	MCFG_DEVICE_ADD("voodoo", VOODOO_BANSHEE, STD_VOODOO_BANSHEE_CLOCK)
	MCFG_VOODOO_FBMEM(16)
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("maincpu")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(vegas_state,vblank_assert))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( vegasv3, vegas32m )
	MCFG_CPU_REPLACE("maincpu", RM7000LE, SYSTEM_CLOCK*2.5)
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)
	MCFG_MIPS3_SYSTEM_CLOCK(SYSTEM_CLOCK)
	MCFG_CPU_PROGRAM_MAP(vegas_map_8mb)

	MCFG_DEVICE_REMOVE("voodoo")
	MCFG_DEVICE_ADD("voodoo", VOODOO_3, STD_VOODOO_3_CLOCK)
	MCFG_VOODOO_FBMEM(16)
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("maincpu")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(vegas_state,vblank_assert))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( denver, vegascore )
	MCFG_CPU_REPLACE("maincpu", RM7000LE, SYSTEM_CLOCK*2.5)
	MCFG_MIPS3_ICACHE_SIZE(16384)
	MCFG_MIPS3_DCACHE_SIZE(16384)
	MCFG_MIPS3_SYSTEM_CLOCK(SYSTEM_CLOCK)
	MCFG_CPU_PROGRAM_MAP(vegas_map_32mb)

	MCFG_DEVICE_REMOVE("voodoo")
	MCFG_DEVICE_ADD("voodoo", VOODOO_3, STD_VOODOO_3_CLOCK)
	MCFG_VOODOO_FBMEM(16)
	MCFG_VOODOO_SCREEN_TAG("screen")
	MCFG_VOODOO_CPU_TAG("maincpu")
	MCFG_VOODOO_VBLANK_CB(WRITELINE(vegas_state,vblank_assert))
MACHINE_CONFIG_END

// Per driver configs

static MACHINE_CONFIG_DERIVED( gauntleg, vegas )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2104, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)
	MCFG_DCS2_AUDIO_POLLING_OFFSET(0x0b5d)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_CALSPEED)
	MCFG_MIDWAY_IOASIC_UPPER(340/* 340=39", 322=27", others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gauntdl, vegas )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2104, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)
	MCFG_DCS2_AUDIO_POLLING_OFFSET(0x0b5d)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_GAUNTDL)
	MCFG_MIDWAY_IOASIC_UPPER(346/* 347, others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( warfa, vegas250 )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2104, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)
	MCFG_DCS2_AUDIO_POLLING_OFFSET(0x0b5d)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_MACE)
	MCFG_MIDWAY_IOASIC_UPPER(337/* others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tenthdeg, vegas )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2104, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)
	MCFG_DCS2_AUDIO_POLLING_OFFSET(0x0afb)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_GAUNTDL)
	MCFG_MIDWAY_IOASIC_UPPER(330/* others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( roadburn, vegas32m )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DSIO, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)
	MCFG_DCS2_AUDIO_POLLING_OFFSET(0) /* no place to hook :-( */

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_STANDARD)
	MCFG_MIDWAY_IOASIC_UPPER(325/* others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nbashowt, vegasban )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2104, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_MACE)
	MCFG_MIDWAY_IOASIC_UPPER(528/* or 478 or 487 */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( nbanfl, vegasban )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2104, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_BLITZ99)
	MCFG_MIDWAY_IOASIC_UPPER(498/* or 478 or 487 */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sf2049 , denver )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DENVER, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(8)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_STANDARD)
	MCFG_MIDWAY_IOASIC_UPPER(336/* others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sf2049se, denver )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DENVER, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(8)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_SFRUSHRK)
	MCFG_MIDWAY_IOASIC_UPPER(336/* others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sf2049te, denver )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_DENVER, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(8)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_SFRUSHRK)
	MCFG_MIDWAY_IOASIC_UPPER(348/* others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( cartfury, vegasv3 )
	MCFG_DEVICE_ADD("dcs", DCS2_AUDIO_2104, 0)
	MCFG_DCS2_AUDIO_DRAM_IN_MB(4)

	MCFG_DEVICE_ADD("ioasic", MIDWAY_IOASIC, 0)
	MCFG_MIDWAY_IOASIC_SHUFFLE(MIDWAY_IOASIC_CARNEVIL)
	MCFG_MIDWAY_IOASIC_UPPER(495/* others? */)
	MCFG_MIDWAY_IOASIC_YEAR_OFFS(80)
	MCFG_MIDWAY_IOASIC_IRQ_CALLBACK(WRITELINE(vegas_state, ioasic_irq))
	MCFG_MIDWAY_IOASIC_AUTO_ACK(1)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

	// there is a socket next to the main bios roms for updates, this is what the update region is.


ROM_START( gauntleg )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "legend15.bin", 0x000000, 0x80000, CRC(a8372d70) SHA1(d8cd4fd4d7007ee38bb58b5a818d0f83043d5a48) ) // EPROM Boot code. Version: Nov 17 1998 19:18:28 / 1.5 Nov 17 1998 19:21:49

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )

	DISK_REGION( "ide:0:hdd:image" )    /* Guts 1.5 1/14/1999 Game 1/14/1999 */
	DISK_IMAGE( "gauntleg", 0, SHA1(66eb70e2fba574a7abe54be8bd45310654b24b08) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( gauntleg12 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "legend13.bin", 0x000000, 0x80000, CRC(34674c5f) SHA1(92ec1779f3ab32944cbd953b6e1889503a57794b) ) //  EPROM Boot code. Version: Sep 25 1998 18:34:43 / 1.3 Sep 25 1998 18:33:45
	ROM_LOAD( "legend14.bin", 0x000000, 0x80000, CRC(66869402) SHA1(bf470e0b9198b80f8baf8b9432a7e1df8c7d18ca) ) //  EPROM Boot code. Version: Oct 30 1998 17:48:21 / 1.4 Oct 30 1998 17:44:29

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "noupdate",       "No Update Rom" )

	ROM_SYSTEM_BIOS( 1, "up16_1",       "Disk Update 1.2 to 1.6 Step 1 of 3" )
	ROMX_LOAD("12to16.1.bin", 0x000000, 0x100000, CRC(253c6bf2) SHA1(5e129576afe2bc4c638242e010735655d269a747), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "up16_2",       "Disk Update 1.2 to 1.6 Step 2 of 3" )
	ROMX_LOAD("12to16.2.bin", 0x000000, 0x100000, CRC(15b1fe78) SHA1(532c4937b55befcc3a8cb25b0282d63e206fba47), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "up16_3",       "Disk Update 1.2 to 1.6 Step 3 of 3" )
	ROMX_LOAD("12to16.3.bin", 0x000000, 0x100000, CRC(1027e54f) SHA1(a841f5cc5b022ddfaf70c97a64d1582f0a2ca70e), ROM_BIOS(4))



	DISK_REGION( "ide:0:hdd:image" )    /* Guts 1.4 10/22/1998 Main 10/23/1998 */
	DISK_IMAGE( "gauntl12", 0, SHA1(62917fbd692d004bc391287349041ebe669385cf) ) // compressed with -chs 4969,16,63 (which is apparently correct for a Quantum FIREBALL 2.5 GB and allows the update program to work)

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( gauntdl )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* EPROM 1.7 12/14/1999 */
	ROM_LOAD( "gauntdl.bin", 0x000000, 0x80000, CRC(3d631518) SHA1(d7f5a3bc109a19c9c7a711d607ff87e11868b536) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )    /* Guts: 1.9 3/17/2000 Game 5/9/2000 */
	DISK_IMAGE( "gauntdl", 0, SHA1(ba3af48171e727c2f7232c06dcf8411cbcf14de8) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( gauntdl24 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* EPROM 1.7 12/14/1999 */
	ROM_LOAD( "gauntdl.bin", 0x000000, 0x80000, CRC(3d631518) SHA1(d7f5a3bc109a19c9c7a711d607ff87e11868b536) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )    /* Guts: 1.9 3/17/2000 Game 3/19/2000 */
	DISK_IMAGE( "gauntd24", 0, SHA1(3e055794d23d62680732e906cfaf9154765de698) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( warfa )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* EPROM 1.9 3/25/1999 */
	ROM_LOAD( "warboot.v19", 0x000000, 0x80000, CRC(b0c095cd) SHA1(d3b8cccdca83f0ecb49aa7993864cfdaa4e5c6f0) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )    /* Guts 1.3 4/20/1999 Game 4/20/1999 */
	DISK_IMAGE( "warfa", 0, SHA1(87f8a8878cd6be716dbd6c68fb1bc7f564ede484) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "warsnd.106", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END

ROM_START( warfaa )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* EPROM 1.6 Jan 14 1999 */
	ROM_LOAD( "warboot.v16", 0x000000, 0x80000, CRC(1c44b3a3) SHA1(e81c15d7c9bc19078787d39c7f5e48eab003c5f4) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )    /* GUTS 1.1 Mar 16 1999, GAME Mar 16 1999 */
	DISK_IMAGE( "warfaa", 0, SHA1(b443ba68003f8492e5c20156e0d3091fe51e9224) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "warsnd.106", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( tenthdeg )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "tenthdeg.bio", 0x000000, 0x80000, CRC(1cd2191b) SHA1(a40c48f3d6a9e2760cec809a79a35abe762da9ce) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )    /* Guts 5/26/1998 Main 8/25/1998 */
	DISK_IMAGE( "tenthdeg", 0, SHA1(41a1a045a2d118cf6235be2cc40bf16dbb8be5d1) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "tenthdeg.snd", 0x000000, 0x8000, CRC(1c75c1c1) SHA1(02ac1419b0fd4acc3f39676e7dce879e926d998b) )
ROM_END


ROM_START( roadburn )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* EPROM 2.6 4/22/1999 */
	ROM_LOAD( "rbmain.bin", 0x000000, 0x80000, CRC(060e1aa8) SHA1(2a1027d209f87249fe143500e721dfde7fb5f3bc) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )    /* Guts 4/22/1999 Game 4/22/1999 */
	DISK_IMAGE( "roadburn", 0, SHA1(a62870cceafa6357d7d3505aca250c3f16087566) )
ROM_END


ROM_START( nbashowt )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "showtime_mar15_1999.u27", 0x000000, 0x80000, CRC(ff5d620d) SHA1(8f07567929f40a2269a42495dfa9dd5edef688fe) ) // 16:09:14 Mar 15 1999 BIOS FOR SHOWTIME USING BANSHEE / 16:09:01 Mar 15 1999. POST FOR SHOWTIME USING BANSHEE

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )
	// various strings from this image
	// SHOWTIME REV 2.0
	// BUILD DATE: Apr 25 1999 (diag.exe?)
	// BUILD DATE: Apr 21 1999 (game?)
	DISK_IMAGE( "nbashowt", 0, SHA1(f7c56bc3dcbebc434de58034986179ae01127f87) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( nbanfl )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "blitz00_sep22_1999.u27", 0x000000, 0x80000, CRC(6a9bd382) SHA1(18b942df6af86ea944c24166dbe88148334eaff9) ) // 16:00:32 Sep 22 1999 BIOS FOR BLITZ00 USING BANSHEE / 16:00:26 Sep 22 1999 POST FOR BLITZ00 USING BANSHEE
//  ROM_LOAD( "bootnflnba.bin", 0x000000, 0x80000, CRC(3def7053) SHA1(8f07567929f40a2269a42495dfa9dd5edef688fe) ) // 1 byte different to above (0x51b95 is 0x1b instead of 0x18)
	ROM_LOAD( "blitz00_nov30_1999.u27", 0x000000, 0x80000, CRC(4242bf14) SHA1(c1fcec67d7463df5f41afc89f22c3b4484279534) ) // 15:10:49 Nov 30 1999 BIOS FOR BLITZ00 USING BANSHEE / 15:10:43 Nov 30 1999 POST FOR BLITZ00 USING BANSHEE

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )
	// various strings from this image
	//NBA SHOWTIME 2.1
	//BUILD DATE: Sep 22 1999 (diag.exe?)
	//BUILD DATE: Sep 21 1999 (game?)
	DISK_IMAGE( "nbanfl", 0, SHA1(f60c627f85f1bf58f2ea674063736a1e516e7e9e) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END

// I'm not sure if NBA Showtime: NBA on NBC Gold was a standalone release, or the version with NBA Showtime: NBA on NBC Gold is actually 'Sports Station'
// it's possible the boot rom and CHD are mismatched here
ROM_START( nbagold )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "nbagold_jan10_2000.u27", 0x000000, 0x80000, CRC(6768e802) SHA1(d994e3efe14f57e261841134ddd1489fa67d418b) ) // 11:29:11 Jan 10 2000. BIOS FOR NBAGOLD USING BANSHEE / 11:23:58 Jan 10 2000. POST FOR NBAGOLD USING BANSHEE

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )
	// various strings from this image
	//NBA SHOWTIME GOLD 3.00
	//BUILD DATE Feb 18 2000 (diag.exe)
	//BUILD DATE:Feb 17 2000 (game?)
	//BUILD DATE:Feb 10 2000 (something else?)
	DISK_IMAGE( "nbanfl3", 0,  SHA1(19a51346ce5ae4e06e8dff3eb4bed59ec1ee855f))
	// these both contain the same strings / build dates, same thing with different user data / drive sizes?
//  DISK_IMAGE( "nbanfl27", 0, SHA1(da371d27e2fbceec493e2203055e0c1399eaf3b9) )
//  DISK_IMAGE( "sportstn", 0, SHA1(9442feefaeb5ae4a090422e937615f8a2d8e8f31) )


	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* Vegas SIO boot ROM */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )

	// also a PIC?
ROM_END


ROM_START( cartfury )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "cart_mar8_2000.u27", 0x000000, 0x80000, CRC(c44550a2) SHA1(ad30f1c3382ff2f5902a4cbacbb1f0c4e37f42f9) ) // 10:40:17 Mar  8 2000 BIOS FOR CART USING VOODOO3 / 10:39:55 Mar  8 2000 POST FOR CART USING VOODOO3

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "cartfury", 0, SHA1(4c5bc2803297ea9a191bbd8b002d0e46b4ae1563) )

	ROM_REGION16_LE( 0x10000, "dcs", 0 )    /* ADSP-2105 data */
	ROM_LOAD16_BYTE( "vegassio.bin", 0x000000, 0x8000, CRC(d1470e23) SHA1(f6e8405cfa604528c0224401bc374a6df9caccef) )
ROM_END


ROM_START( sf2049 )
	ROM_REGION32_LE( 0x80000, "user1", 0 )  /* EPROM 1.02 7/9/1999 */
	ROM_LOAD( "sf2049.u27", 0x000000, 0x80000, CRC(174ba8fe) SHA1(baba83b811eca659f00514a008a86ef0ac9680ee) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )    /* Guts 1.03 9/3/1999 Game 9/8/1999 */
	DISK_IMAGE( "sf2049", 0, SHA1(9e0661b8566a6c78d18c59c11cd3a6628d025405) )
ROM_END


ROM_START( sf2049se )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "sf2049se.u27", 0x000000, 0x80000, CRC(da4ecd9c) SHA1(2574ff3d608ebcc59a63cf6dea13ee7650ae8921) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "sf2049se", 0, SHA1(7b27a8ce2a953050ce267548bb7160b41f3e8054) )
ROM_END


ROM_START( sf2049te )
	ROM_REGION32_LE( 0x80000, "user1", 0 )
	ROM_LOAD( "sf2049te.u27", 0x000000, 0x80000, CRC(cc7c8601) SHA1(3f37dbd1b32b3ac5caa300725468e8e426f0fb83) )

	ROM_REGION32_LE( 0x100000, "update", ROMREGION_ERASEFF )


	DISK_REGION( "ide:0:hdd:image" )
	DISK_IMAGE( "sf2049te", 0, SHA1(625aa36436587b7bec3e7db1d19793b760e2ea51) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

DRIVER_INIT_MEMBER(vegas_state,gauntleg)
{
	/* speedups */
	m_maincpu->mips3drc_add_hotspot(0x80015430, 0x8CC38060, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x80015464, 0x3C09801E, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x800C8918, 0x8FA2004C, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x800C8890, 0x8FA20024, 250);     /* confirmed */
}


DRIVER_INIT_MEMBER(vegas_state,gauntdl)
{
	/* speedups */
	m_maincpu->mips3drc_add_hotspot(0x800158B8, 0x8CC3CC40, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x800158EC, 0x3C0C8022, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x800D40C0, 0x8FA2004C, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x800D4038, 0x8FA20024, 250);     /* confirmed */
}


DRIVER_INIT_MEMBER(vegas_state,warfa)
{
	/* speedups */
	m_maincpu->mips3drc_add_hotspot(0x8009436C, 0x0C031663, 250);     /* confirmed */
}


DRIVER_INIT_MEMBER(vegas_state,tenthdeg)
{
	/* speedups */
	m_maincpu->mips3drc_add_hotspot(0x80051CD8, 0x0C023C15, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x8005E674, 0x3C028017, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x8002DBCC, 0x8FA2002C, 250);     /* confirmed */
	m_maincpu->mips3drc_add_hotspot(0x80015930, 0x8FC20244, 250);     /* confirmed */
}


DRIVER_INIT_MEMBER(vegas_state,roadburn)
{
}


DRIVER_INIT_MEMBER(vegas_state,nbashowt)
{
}


DRIVER_INIT_MEMBER(vegas_state,nbanfl)
{
}


DRIVER_INIT_MEMBER(vegas_state,sf2049)
{
}


DRIVER_INIT_MEMBER(vegas_state,sf2049se)
{
}


DRIVER_INIT_MEMBER(vegas_state,sf2049te)
{
}


DRIVER_INIT_MEMBER(vegas_state,cartfury)
{
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* Vegas + Vegas SIO + Voodoo 2 */
GAME( 1998, gauntleg,   0,        gauntleg,    gauntleg, vegas_state, gauntleg, ROT0, "Atari Games",  "Gauntlet Legends (version 1.6)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, gauntleg12, gauntleg, gauntleg,    gauntleg, vegas_state, gauntleg, ROT0, "Atari Games",  "Gauntlet Legends (version 1.2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1998, tenthdeg, 0,        tenthdeg,    tenthdeg, vegas_state, tenthdeg, ROT0, "Atari Games",  "Tenth Degree (prototype)", MACHINE_SUPPORTS_SAVE )

/* Durango + Vegas SIO + Voodoo 2 */
GAME( 1999, gauntdl,  0,        gauntdl,    gauntdl, vegas_state,  gauntdl,  ROT0, "Midway Games", "Gauntlet Dark Legacy (version DL 2.52)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, gauntdl24,gauntdl,  gauntdl,    gauntdl, vegas_state,  gauntdl,  ROT0, "Midway Games", "Gauntlet Dark Legacy (version DL 2.4)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, warfa,    0,        warfa, warfa, vegas_state,    warfa,    ROT0, "Atari Games",  "War: The Final Assault (EPROM 1.9 Mar 25 1999, GUTS 1.3 Apr 20 1999, GAME Apr 20 1999)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1999, warfaa,   warfa,    warfa, warfa, vegas_state,    warfa,    ROT0, "Atari Games",  "War: The Final Assault (EPROM 1.6 Jan 14 1999, GUTS 1.1 Mar 16 1999, GAME Mar 16 1999)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )


/* Durango + DSIO + Voodoo 2 */
GAME( 1999, roadburn, 0,        roadburn, roadburn, vegas_state, roadburn, ROT0, "Atari Games",  "Road Burners", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

/* Durango + DSIO? + Voodoo banshee */
GAME( 1998, nbashowt, 0,        nbashowt, nbashowt, vegas_state, nbashowt, ROT0, "Midway Games", "NBA Showtime: NBA on NBC (ver 2.0)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1999, nbanfl,   0,        nbanfl, nbashowt, vegas_state, nbanfl,   ROT0, "Midway Games", "NBA Showtime / NFL Blitz 2000 (ver 2.1)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 2000, nbagold , 0,        nbanfl, nbashowt, vegas_state, nbanfl,   ROT0, "Midway Games", "NBA Showtime Gold / NFL Blitz 2000 (ver 3.0) (Sports Station?)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )


/* Durango + Denver SIO + Voodoo 3 */
GAME( 1998, sf2049,   0,        sf2049,   sf2049, vegas_state,   sf2049,   ROT0, "Atari Games",  "San Francisco Rush 2049", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1998, sf2049se, sf2049,   sf2049se,   sf2049se, vegas_state, sf2049se, ROT0, "Atari Games",  "San Francisco Rush 2049: Special Edition", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1998, sf2049te, sf2049,   sf2049te,   sf2049te, vegas_state, sf2049te, ROT0, "Atari Games",  "San Francisco Rush 2049: Tournament Edition", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE)

/* Durango + Vegas SIO + Voodoo 3 */
GAME( 2000, cartfury, 0,        cartfury,  cartfury, vegas_state, cartfury, ROT0, "Midway Games", "Cart Fury", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
