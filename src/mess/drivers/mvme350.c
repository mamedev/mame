// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Motorola MVME-350 6U Intelligent Tape Controller driver, initially derived 
 *  from hk68v10.c
 *
 *  31/08/2015
 *
 * I baught this board from http://www.retrotechnology.com without documentation.
 * It has a Motorola 68010 CPU @ 10MHz, 128 Mb RAM and two 2764 EPROMS with 
 * QIC-02 tape controller firmware. The board also populates a 68230 PIT and loads
 * of descrete TTL components.
 *
 *                                                                                   
 *       ||                                                                          
 * ||    ||                                                                          
 * ||||--||                                                                          
 * ||||--||                                                                          
 * ||    ||__________________________________________________________    ___ 
 *       ||_______________________________________________________   |_|   |
 *       ||      |74F74 | xxxxx  |  xxxxx  | 74LS245   | 74ALS645|   | |   |
 *       ||      |______|________|_________|___________| ________|   | |   |
 *       ||      |74F74 | xxxxx  |  xxxxx  | 74LS245   | 74ALS645|   | |   |
 *       ||      |______|________|_________|___________| ________|   | |   |
 *       ||      |74LS02| 74LS08||Am25LS251| 74S38 |74LS20|Jumpers   | |   |
 *       ||      |______|_______||_________|_______|______|______|   | |VME|
 *       ||      |74F04 | 74LS32||74LS374  | 74LS374   |74S244   |   | |   |
 *       ||+---+ |______|_______||_________|___________|_________|   | |P1 |
 *       |||CON| |74F04 | 74LS85||74S244   | 74S240    |PAL      |   | |   |
 *       |||   | |______|_______||_________|___________|_________|   | |   |
 *       |||   | |74LS04|      |                       |74S244   |   | |   |
 *       |||   | |______|      |    PIT                |_________|   | |   |
 *       |||   | |74LS125      |  MC68230L10           |74LS145  |   | |   |
 *       |||   | |______|    __|_______________________|---------|   | |   |
 *       |||   | |74LS74|   |25LS251  | |    RAM       |74S244   |   |_|   |
 *       ||+---+ ---------- |---------- | HM6264P-12   |---------|     |___|
 *       ||      | PAL    | | 74245   | |______________|74S244   |     |  
 *       ||      +--------- |_________| | U40 27128    |---------|     |  
 *       ||     | 74F32 |   |          || System ROM   |74LS682  |     |  
 * Red   ||     +--------   | CPU      ||              |-+-------|     |  
 *  FAIL ||LED  | 74F138|   | MC68010  |+--------------+ |DIPSW__|     |  
 * Red   ||     +--------   |          ||    RAM       | |74S38  |     |  
 *  HALT ||LED  | 74F32 |   |__________|| HM6264P-12   | |______ |     |  
 * Green ||     +-------+   |74245     |+--------------+ |74F08  |     |  
 *  RUN  ||LED  |XTAL   |   |__________|| U47 27128    | |______ |     |___
 *       ||+---+|20MHz  |   |74244     || System ROM   | |74F00  |    _|   |
 *       |||CON|--------+___|__________|+--------------+_|_______|   | |   |
 *       |||   |74LS08  |74F74   |74LS148|  |PAL     | | PAL     |   | |   |
 *       |||   |________|________|_______|  |________|_|_________|   | |   |
 *       |||   | 74LS138|74F32   | PAL   |  |74F74   |Am29823    |   | |   |
 *       |||   |________|________|_______|  |________|-+---------|   | |VME|
 *       |||   | 74LS11 |74F04   |74LS374|  |74LS374 | |74S240   |   | |   |
 *       |||   |________|________|_______|  |________|_|---------|   | |P2 |
 *       |||   | 74F138 |BLANK   |74LS374|  |74LS374   |74S240   |   | |   |
 *       |||   |________|________|_______|  +----------+---------|   | |   |
 *       |||   | 74LS08 |74F32   |74LS11|74LS393|74LS393|resistors   | |   |
 *       |||   |________|________|______|______ |_______|________|   | |   |
 *       |||   |DM2585  |74F74   |DM2230| 74LS00| 74F02   |74F32 |   | |   |
 *       ||+---+--------+--------+------+-------+---------+------|   | |   |
 *       ||    |74LS74  |74F20   |74S260| 74S74 | 74F08   |74LS02|   | |   |
 *       ||    +------------------------------------------+------|   | |   |
 *       ||    |DM2353  |74F10   |74F32 | 74LS32| 74F08   |DM2353|   |_|   |
 *       ||    +------------------------------------------+------+-+   |___|
 * ||    ||------------------------------------------------------------+-+             
 * ||||--||                                                            
 * ||||--||                                                            
 * ||
 *
 * History of Motorola VME division (https://en.wikipedia.org/wiki/VMEbus)
 *---------------------------------
 * When Motorola released the 68000 processor 1979 the ambition of the deisgners
 * was also to standardize a versatile CPU bus to be able to build computer 
 * systems without constructing PCB:s from scratch. This become VersaBus but the
 * boards was really too big and the computer world already saw the systems shrink
 * in size. Motorola's design center in Munich proposed to use the smaller and
 * already used Euroboard form factor and call it Versabus-E. This later became
 * VME which was standardized in the VITA organization 1981 
 *
 * Misc links about Motorola VME division and this board: 
 * http://bitsavers.trailing-edge.com/pdf/motorola/_dataBooks/1987_Microcomputer_Systems_and_Components.pdf
 * 
 * Description
 * ------------
 * Streaming Tape Controller released 1984 with the following feature set
 *
 *      -  Double High (6U) VMEmodule
 *      -  QIC-02 Streaming Tape Interface
 *      -  Supports One 01C-02 compatible 1/4-inch Streaming Tape Drive
 *      -  Standard VMEbus Interface
 *      -  Supports 24- or 32-bit DMA Addressing/16-bit Data
 *      -  Generates Seven Levels of VMEbus Interrupts with Programmable Interrupt Vector
 *      -  10 MHz MC68010 Microprocessor
 *      -  90Kb/s Continuous Transfer Rate for QIC-02 Interface, 200Kb/s Burst rate
 *      -  Controls Tape Cartridges Offering 20Mb, 45Mb and 60Mb of Formatted Data Storage
 *      -  MC68230 PIT-based Timer
 *      -  16Kb of Static RAM Provides Buffer Storage and CPU Workspace
 *      -  Multitasking Kernel-based Firmware Package
 *      -  Buffered Pipe Communication Protocol Allows Multiple Hosts to Oueue Commands Without Interlock
 *      -  High Level Command/Status Packets offer efficient Operating System Support
 *      -  Permits Chaining of Host Command
 *
 * Address Map 
 * --------------------------------------------------------------------------
 * Local     to VME       Decscription
 * --------------------------------------------------------------------------
 * 0x000000               Up to 128Kb System ROM with RESET vector
 * 0x020000               RAM with vectors
 * 0x020500               RAM Top of stack
 * 0x040000               PIT device?
 * 0x060000               RAM?
 * 0x080000               PIT device?
 * --------------------------------------------------------------------------
 *
 * Interrupt sources MVME
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources                     
 *  
 * Off board Sources (other VME boards)
 *
 * ----------------------------------------------------------
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         M10            V10
 * ----------------------------------------------------------
 *
 *
 *  TODO:
 *  - Dump the ROMs (DONE)
 *  - Setup a working address map (STARTED)
 *  - Get documentation for VME interface
 *  - Add VME bus driver
 *  - Hook up a CPU board that supports boot from tape (ie MVME-162, MVME 147)
 *  - Get a tape file with a bootable data on it.
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"

#define LOG(x) x

class mvme350_state : public driver_device
{
public:
mvme350_state(const machine_config &mconfig, device_type type, const char *tag) :
        driver_device (mconfig, type, tag),
                m_maincpu (*this, "maincpu"),
                m_pit(*this, "pit")
{
}

DECLARE_READ16_MEMBER (vme_a24_r);
DECLARE_WRITE16_MEMBER (vme_a24_w);
DECLARE_READ16_MEMBER (vme_a16_r);
DECLARE_WRITE16_MEMBER (vme_a16_w);
virtual void machine_start ();
virtual void machine_reset ();
protected:

private:
        required_device<cpu_device> m_maincpu;
	required_device<pit68230_device> m_pit;

};

static ADDRESS_MAP_START (mvme350_mem, AS_PROGRAM, 16, mvme350_state)
ADDRESS_MAP_UNMAP_HIGH
        AM_RANGE (0x000000, 0x01ffff) AM_ROM /* 128 Mb ROM */
        AM_RANGE (0x020000, 0x03ffff) AM_RAM /* 128 Mb RAM */
#if 1
        AM_RANGE(0x040000,  0x040035) AM_DEVREADWRITE8("pit", pit68230_device, read, write, 0x00ff) /* PIT ?*/
        AM_RANGE(0x060000,  0x06001f) AM_RAM /* Area is cleared on start */
        AM_RANGE(0x080000,  0x080035) AM_DEVREADWRITE8("pit", pit68230_device, read, write, 0x00ff) /* PIT ?*/
#endif
//AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
//AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (mvme350)
INPUT_PORTS_END

/* Start it up */
void mvme350_state::machine_start ()
{
        LOG (logerror ("machine_start\n"));
}

void mvme350_state::machine_reset ()
{
        LOG (logerror ("machine_reset\n"));
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (mvme350_state::vme_a24_r){
        LOG (logerror ("vme_a24_r\n"));
        return (UINT16) 0;
}

WRITE16_MEMBER (mvme350_state::vme_a24_w){
        LOG (logerror ("vme_a24_w\n"));
}

READ16_MEMBER (mvme350_state::vme_a16_r){
        LOG (logerror ("vme_16_r\n"));
        return (UINT16) 0;
}

WRITE16_MEMBER (mvme350_state::vme_a16_w){
        LOG (logerror ("vme_a16_w\n"));
}
#endif

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (mvme350, mvme350_state)
        /* basic machine hardware */
        MCFG_CPU_ADD ("maincpu", M68010, XTAL_10MHz)
        MCFG_CPU_PROGRAM_MAP (mvme350_mem)
        /* PIT Parallel Interface and Timer device, assuming strapped for on board clock */
	MCFG_DEVICE_ADD("pit", PIT68230, XTAL_16MHz / 2)

MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (mvme350)
ROM_REGION (0x1000000, "maincpu", 0)

ROM_LOAD16_BYTE ("mvme350U40v2.3.bin", 0x0000, 0x4000, CRC (bcef82ef) SHA1 (e6fdf26e4714cbaeb3e97d7b5acf02d64d8ad744))
ROM_LOAD16_BYTE ("mvme350U47v2.3.bin", 0x0001, 0x4000, CRC (582ce095) SHA1 (d0929dbfeb0cfda63df6b5bc29ee27fbf665def7))

/*
 * System ROM information
 *
 * The ROMs known commands from different sources:
 *
 *  It communicates with the master through data buffers in shared memory and VME bus interrupts
 * as desribed in 
 * http://bitsavers.trailing-edge.com/pdf/motorola/_dataBooks/1987_Microcomputer_Systems_and_Components.pdf
 * 
 * The board is pretty boring as stand alone, it initializes everything and then executes a STOP instruction
 * awaiting a CPU on the VME bus to request its services. However, it enables boot from tape devices, we just 
 * need a MVME-131 and a dump of a VersaDOS or Motorola UNIX System V system tape and some work.
 */
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1984, mvme350,      0,      0,       mvme350,        mvme350, driver_device, 0,   "Motorola",   "MVME-350", MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
