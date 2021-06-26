// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Motorola MVME-350 6U Intelligent Tape Controller driver
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
 *------------------------------------------------------------------------
 *  See mvme147.cpp
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
 *           0xffff5000   MVME350 - Streaming Tape Controller CLUN $04 - From MVME-166 installation manual
 *           0xffff5100   MVME350 - Streaming Tape Controller CLUN $05    probably base of shared RAM
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
 *  TODO:
 *  - Dump the ROMs (DONE)
 *  - Setup a working address map (STARTED)
 *  - Get documentation for VME interface
 *  - Add VME bus driver
 *  - Hook up the PITs correctly
 *  - Add a configurable shared memory window between local CPU and the VME bus
 *  - Hook up a CPU board that supports boot from tape (ie MVME-162, MVME 147)
 *  - Get a tape file with a bootable data on it.
 *
 ****************************************************************************/

#include "emu.h"
#include "vme_mvme350.h"

#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 //(LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_MVME350, vme_mvme350_card_device, "mvme350", "Motorola MVME-350 Intelligent Tape Controller")

#define MVME350_CPU_TAG "mvme350_cpu"
#define MVME350_ROM "mvme350_rom"

void vme_mvme350_card_device::mvme350_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x01ffff).rom().region(MVME350_ROM, 0);
	map(0x020000, 0x03ffff).ram();

#if 0
	map(0x040000, 0x040035).rw("pit", FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff); /* PIT ?*/
	map(0x060000, 0x06001f).ram(); /* Area is cleared on start */
	map(0x080000, 0x080035).rw("pit", FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff); /* PIT ?*/
#endif
//map(0x100000, 0xfeffff).rw(FUNC(vme_mvme350_card_device::vme_a24_r), FUNC(vme_mvme350_card_device::vme_a24_w)); /* VMEbus Rev B addresses (24 bits) - not verified */
//map(0xff0000, 0xffffff).rw(FUNC(vme_mvme350_card_device::vme_a16_r), FUNC(vme_mvme350_card_device::vme_a16_w)); /* VMEbus Rev B addresses (16 bits) - not verified */
}

ROM_START( mvme350 )
	ROM_REGION16_BE(0x20000, MVME350_ROM, 0)
	ROM_LOAD16_BYTE("mvme350u40v2.3.bin", 0x0000, 0x4000, CRC (bcef82ef) SHA1 (e6fdf26e4714cbaeb3e97d7b5acf02d64d8ad744))
	ROM_LOAD16_BYTE("mvme350u47v2.3.bin", 0x0001, 0x4000, CRC (582ce095) SHA1 (d0929dbfeb0cfda63df6b5bc29ee27fbf665def7))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void vme_mvme350_card_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	m68010_device &cpu(M68010(config, MVME350_CPU_TAG, XTAL(10'000'000)));
	cpu.set_addrmap(AS_PROGRAM, &vme_mvme350_card_device::mvme350_mem);
	/* PIT Parallel Interface and Timer device, assuming strapped for on board clock */
	PIT68230(config, "pit", XTAL(16'000'000) / 2);
}

const tiny_rom_entry *vme_mvme350_card_device::device_rom_region() const
{
	LOG("%s\n", FUNCNAME);
	return ROM_NAME( mvme350 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

vme_mvme350_card_device::vme_mvme350_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_vme_card_interface(mconfig, *this)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

vme_mvme350_card_device::vme_mvme350_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	vme_mvme350_card_device(mconfig, VME_MVME350, tag, owner, clock)
{
	LOG("%s %s\n", tag, FUNCNAME);
}

void vme_mvme350_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);

	/* Setup r/w handlers for shared memory area */
#if 0
	/* From MVME166 Single Board Computer Installation Guide:

	   Controller Type     First board    Second board
	                       CLUN Address   CLUN Address
	  ---------------------------------------------------
	   MVME350 - Streaming $04  $FFFF5000 $05  $FFFF5100
	   Tape Controller
	  ---------------------------------------------------
	*/
	uint32_t base = 0xFFFF5000;
	m_vme->install_device(base + 0, base + 1, // Channel B - Data
							 read8_delegate(FUNC(z80sio_device::db_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::db_w), subdevice<z80sio_device>("pit")), 0x00ff);
	m_vme->install_device(base + 2, base + 3, // Channel B - Control
							 read8_delegate(FUNC(z80sio_device::cb_r),  subdevice<z80sio_device>("pit")), write8_delegate(FUNC(z80sio_device::cb_w), subdevice<z80sio_device>("pit")), 0x00ff);
#endif

}

void vme_mvme350_card_device::device_reset()
{
	LOG("%s %s\n", tag(), FUNCNAME);
}

#if 0
uint16_t vme_mvme350_card_device::read16(){
	LOG("%s()\n", FUNCNAME);
	return (uint8_t) 0;
}

void vme_mvme350_card_device::write16(uint16_t data){
	LOG("%s()\n", FUNCNAME);
}
#endif
