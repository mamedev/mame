// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Mizar VME8105 rev D 3U SBC driver, initially derived from force68k.c
 *
 *  19/08/2015
 *
 * I baught this board from http://www.retrotechnology.com without documentation.
 * It has a Motorola 68000 CPU @ 10MHz and two 27128 EPROMS with OS9 DEBUG labels
 * and not much more except 16 or so TTLs, 2 PALs and a VME P1 connector. It is a
 * 2 sided design so it shold be possible to trace the schematics quite easily.
 * There is a date on the P1 Connector: "Feb 20 1987"
 *
 *       ||
 *       ||
 *       ||
 *       ||
 *       ||____________________________________________________________   ___
 *       ||    --------------K2|U6-7432|U7-7474|U14-7405|U21-74645  |  |_|   |
 *       ||   | 2764 - low   |-+-------+-------+--------+-----------+  | |   |
 *       ||   | OS9 DEBUG  U4|  +---------+    +--------+---------+    | |   |
 *       ||    --------------   |         |    |U13-7474|U20-74645|    | |   |
 *       ||    ______________   |         |    +--------+---------+    | |   |
 *       ||   |              |  |         |    ______________K4____    | |   |
 *       ||   |            U3|  |         |    |U12-7438|U19-74244|    | |VME|
 *       ||    --------------   |         |+------K6---------K3---+    | |   |
 *       ||    ______________   |         ||U11-PAL20L10|U18-74244|    | |P1 |
 *       ||   | 2764 - high  |  |         |+-----021102-+---------+    | |   |
 *       ||   | OS9 DEBUG  U2|  |MC68000L10+------------+---------+    | |   |
 *       ||   |______________|  |         ||U10-PAL14L4 |U17-74244|    | |   |
 *       ||                     |         |+-----021001-+---------+    | |   |
 *       ||    --------------   |         |+----+_______|U16-74244|+--+| |   |
 *       ||   |              |  |         |XTAL |U9-7410+---------+|  || |   |
 *       ||K1 |            U1|  |         |FOX100-------+---------+|K5||_|   |
 *       ||    --------------   |_________|10MHz|U8-7404|U15-74148||  || |___|
 *   ||--||--------------------------------+----+-------+---------++--+
 *   ||--||
 *       ||
 *
 * PAL:s
 *------
 * The board has two PAL:s, a PAL20L10 and a PAL14L4.
 *
 *                        _____   _____
 *              OD/CLK 1 |*    \_/     | 20 VCC
 *     CPU A13    I0   2 |             | 19 O0/LR  A23
 *  U9 7410p12    I1   3 |             | 18 O1/A0  A22
 *     CPU *AS    I2   4 |             | 17 02/A1  U2/U4 2764p22 *OE
 *     GND        I3   5 |             | 16 03/A2  U11 PAL20L10p6 I4
 *     CPU A14    I4   6 |     U10     | 15 A0/O0  U11 PAL20L10p7 I5
 *     CPU A15    I5   7 |   PAL14L4   | 14 A1/O1  U19 74LS244p11 I0b
 *     CPU A16    I6   8 |             | 13 A2/O2  A18
 *     CPU A17    I7   9 |             | 12 LR/O3  A20
 *                GND 10 |_____________| 11 CLK/OD A21
 *
 *                        _____   _____
 *              OD/CLK 1 |*    \_/     | 24 VCC
 *                I0   2 |             | 23 O0/LR
 *     CPU *AS    I1   3 |             | 22 O1/A0
 * U10 PAL14L4p3  I2   4 |             | 21 02/A1
 *                I3   5 |             | 20 03/A2
 * U10 PAL14L4p16 I4   6 |     U10     | 19 04/NC
 * U10 PAL14L4p15 I5   7 |   PAL20L10  | 18 NC/O0
 *                I6   8 |             | 17 A2/O1
 *                I7   9 |             | 16 A1/O2
 *                I8  10 |             | 15 A0/O3
 *                I9  11 |             | 14 LR/O4
 *                GND 12 |_____________| 13 CLK/OD
 *
 *  Trace is not fully completed and validated.
 *
 * History of Mizar
 *-------------------
 * Mizar was one of the first companies that released VME boards in 1982-3 together with
 * Force Computers and Motorola. Their systems was often supported by OS9/68000 from Microware.
 * In 1993 Mizar discontinued their OS9/CPU board product range and focused entirely on DSP
 * boards. RTSI was founded by Mizar employees preserving the knowledge in OS9 as consultants.
 * In 1998 Blue Wave resulted from the merger of Mizar and Loughborough Sound Images.
 * Not much tech information availabe but I found some quotes about Mizar on the internet:
 *
 * From http://archive.org/stream/68micro-vol-11-num-02/V11N02_Feb1989_djvu.txt :
 *--------------------------------------------------------------------------------
 * " Mizar provides complete OS-9 solutions for the VMEbus. Mizar's VME CPUs
 * offer (he functions and performance your application demands. Our single
 * height (3U) VME processors are uniquely configurable computing engines,
 * Through Mizar's unique MXbus expansion interface, standard and custom side
 * modules can be added to basic processors to create double-height (6U) boards
 * tor specific applications, 3U CPU options include 68010, 66020, and 63030
 * microprocessors, up to one MB of DRAM, serial I/O, real-time clock, and
 * mailbox interrupt support. Standard MXbus side modules include additional DRAM.
 * SRAM, and I/O.
 *
 * Mizar's standard double- height (6U) processors provide additional features such
 * as a high-speed cache to enhance 68030 performance, floating, point coprocessor
 * support, up to four MB dual ported DRAM, VSB memory interface, Ethernet, and SCSI.
 *
 * Mizar also supports OS-9 with completely configured OS9 development systems and
 * OS-9 application server systems. For more information, call Mizar today
 *
 * 800-635-0200 MIZAR 1419 Dunn Drive  CarrolHon, TX 75006  214-446-2664"
 *
 * Known boards from Mizar:
 *
 *-Comp.Os.Vxworks diguest mailing list May 1992: VxWorks Drivers Available from Mizar:
 * EMX 7320 Serial I/O Board
 * EMX 7550 Ethernet Controller Board (AMD 7990 Lance) MAC: 00:80:F8 MIZAR, INC.
 * MZ 7300 Serial I/O Board (Z8530)
 * MZ 7400 Disk Controller Board (WD 2010; WD 1772)
 * MZ 7500 IEEE-488 (GPIB) Interface Board
 * MZ 7810 I/O Expansion Module (6681 DUART)
 * MZ 7831 SCSI Expansion Module (WD 33C93A)
 * MZ 7850 Ethernet Expansion Module (WD 83C690)
 * MZ 8505 IEEE-488 (GPIB) Interface Board"
 *- Other sources:
 * MZ 8000 ??
 * MZ 8105 3U 68000 CPU board
 * MZ 8300 3U serial board, 2 NEC 7201 (Z80 DART) and 1 AMD CTS9513 5 x 16 bit Counter/Timer
 *
 * From http://www.megalextoria.com/forum2/index.php?t=msg&goto=73945&
 *--------------------------------------------------------------------
 * Aug 20 1984 a report by James Jones from the OS/9 conferance in Des Moines:
 * "...
 * Mizar: is selling VME bus 68010 boards, running OS-9/68000; they also make
 * various memory and I/O boards, disk controller boards, and and a NEC 7220-based
 * graphics controller board...."
 *
 * Misc links about Mizar:
 * http://www.vita.com/History
 * http://www.verycomputer.com/154_e272024804bbe203_1.htm
 * https://www.aihitdata.com/company/00814595/RTSI/overview
 *
 * Address Map
 * --------------------------------------------------------------------------
 * Address Range         Description/Assumption
 * --------------------------------------------------------------------------
 * 0x000000 0x01ffff   ROM because of fixed vectors
 * 0x020000 0x03ffff   RAM because bootvector stack starts at 0x21000
 * -- VME adresses---
 * 0xff0000 0xff0003   Bootstrap expects to find a UPD7201 serial device here
 * --------------------------------------------------------------------------
 *
 * Interrupt sources
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 *  TBD
 * Off board Sources (other VME boards)
 *  TBD
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Dump the ROMs (DONE)
 *  - Setup a working address map (DONE)
 *  - Add VME bus driver (Faked one)
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/z80dart.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "softlist.h"

#define CARDSLOT 0

#if CARDSLOT // Awaiting info on how to boot the user eproms
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#endif

#define LOG(x) x


/* These values are borrowed just to get the terminal going and should be replaced
 * once a proper serial board hardware (ie MZ 8300) is found and emulated. */
#define BAUDGEN_CLOCK XTAL_19_6608MHz /* fake */
#define UPD_CLOCK (BAUDGEN_CLOCK / 128) /* This will give prompt */

class mzr8105_state : public driver_device
{
public:
mzr8105_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device (mconfig, type, tag),
				m_maincpu (*this, "maincpu")
				,m_updterm(*this, "upd")
//        ,m_cart(*this, "exp_rom1")
{
}

DECLARE_READ16_MEMBER (vme_a24_r);
DECLARE_WRITE16_MEMBER (vme_a24_w);
DECLARE_READ16_MEMBER (vme_a16_r);
DECLARE_WRITE16_MEMBER (vme_a16_w);
virtual void machine_start ();
DECLARE_WRITE_LINE_MEMBER (write_updterm_clock);

#if CARDSLOT
// User EPROM/SRAM slot(s)
int mzr8105_load_cart(device_image_interface &image, generic_slot_device *slot);
DECLARE_DEVICE_IMAGE_LOAD_MEMBER (exp1_load) { return mzr8105_load_cart(image, m_cart); }
DECLARE_READ16_MEMBER (read16_rom);
#endif

protected:

private:
required_device<cpu_device> m_maincpu;
required_device<upd7201_device> m_updterm;

// Pointer to System ROMs needed by bootvect_r
// UINT16  *m_sysrom;

#if CARDSLOT
UINT16  *m_usrrom;
required_device<generic_slot_device> m_cart;
#endif
};

static ADDRESS_MAP_START (mzr8105_mem, AS_PROGRAM, 16, mzr8105_state)
ADDRESS_MAP_UNMAP_HIGH
//AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ (bootvect_r)       /* Not verified */
AM_RANGE (0x000000, 0x003fff) AM_ROM /* System EPROM Area 16Kb OS9 DEBUG - not verified     */
AM_RANGE (0x004000, 0x01ffff) AM_ROM /* System EPROM Area 112Kb for System ROM - not verified    */
AM_RANGE (0x020000, 0x03ffff) AM_RAM /* Not verified */
//AM_RANGE (0x0a0000, 0x0bffff) AM_ROM /* User EPROM/SRAM Area, max 128Kb mapped by a cartslot  */
//AM_RANGE (0x0c0080, 0x0c0081) AM_DEVREADWRITE8 ("aciaterm", acia6850_device, status_r, control_w, 0xff00)
//AM_RANGE (0x0c0082, 0x0c0083) AM_DEVREADWRITE8 ("aciaterm", acia6850_device, data_r, data_w, 0xff00)
AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
//AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
// Faking a Mizar 8300 SIO BOARD in VME A16 adress space
		AM_RANGE (0xFF0000, 0xFF0001) AM_DEVREADWRITE8("upd", upd7201_device, da_r, da_w, 0x00ff) /* Dual serial port NEC uPD7201 */
		AM_RANGE (0xFF0002, 0xFF0003) AM_DEVREADWRITE8("upd", upd7201_device, ca_r, ca_w, 0x00ff) /* Dual serial port NEC uPD7201 */
		AM_RANGE (0xFF0004, 0xFF0005) AM_DEVREADWRITE8("upd", upd7201_device, db_r, db_w, 0x00ff) /* Dual serial port NEC uPD7201 */
		AM_RANGE (0xFF0006, 0xFF0007) AM_DEVREADWRITE8("upd", upd7201_device, cb_r, cb_w, 0x00ff) /* Dual serial port NEC uPD7201 */

ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (mzr8105)
INPUT_PORTS_END

/* Start it up */
void mzr8105_state::machine_start ()
{
		LOG (logerror ("machine_start\n"));

#if CARDSLOT
		/* Map user ROM/RAM socket(s) */
		if (m_cart->exists())
		{
				m_usrrom = (UINT16*)m_cart->get_rom_base();
				m_maincpu->space(AS_PROGRAM).install_read_handler(0xa0000, 0xbffff, read16_delegate(FUNC(generic_slot_device::read16_rom), (generic_slot_device*)m_cart));
		}
#endif
}

/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (mzr8105_state::vme_a24_r){
		LOG (logerror ("vme_a24_r\n"));
		return (UINT16) 0;
}

WRITE16_MEMBER (mzr8105_state::vme_a24_w){
		LOG (logerror ("vme_a24_w\n"));
}

READ16_MEMBER (mzr8105_state::vme_a16_r){
		LOG (logerror ("vme_16_r\n"));
		return (UINT16) 0;
}

WRITE16_MEMBER (mzr8105_state::vme_a16_w){
		LOG (logerror ("vme_a16_w\n"));
}

#if CARDSLOT
/*
 * 4. The USER EPROM Area
 */
// Implementation of static 2 x 64K EPROM in sockets U1/U3 as 16 bit wide cartridge for easier
// software handling. TODO: make configurable according to table above.
static MACHINE_CONFIG_FRAGMENT( mzr8105_eprom_sockets )
		MCFG_GENERIC_CARTSLOT_ADD("exp_rom1", generic_plain_slot, "mzr8105_cart")
		MCFG_GENERIC_EXTENSIONS("bin,rom")
		MCFG_GENERIC_WIDTH(GENERIC_ROM16_WIDTH)
		MCFG_GENERIC_ENDIAN(ENDIANNESS_BIG)
		MCFG_GENERIC_LOAD(mzr8105_state, exp1_load)
//      MCFG_SOFTWARE_LIST_ADD("cart_list", "mzr8105_cart")
MACHINE_CONFIG_END

/***************************
   Rom loading functions
****************************/
int mzr8105_state::mzr8105_load_cart(device_image_interface &image, generic_slot_device *slot)
{
		UINT32 size = slot->common_get_size("rom");

		if (size > 0x20000) // Max 128Kb - not verified
		{
				LOG( printf("Cartridge size exceeding max size (128Kb): %d\n", size) );
				image.seterror(IMAGE_ERROR_UNSPECIFIED, "Cartridge size exceeding max size (128Kb)");
				return IMAGE_INIT_FAIL;
		}

		slot->rom_alloc(size, GENERIC_ROM16_WIDTH, ENDIANNESS_BIG);
		slot->common_load_rom(slot->get_rom_base(), size, "rom");

		return IMAGE_INIT_PASS;
}
#endif

WRITE_LINE_MEMBER (mzr8105_state::write_updterm_clock){
		m_updterm->txca_w (state);
		m_updterm->rxca_w (state);
}

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (mzr8105, mzr8105_state)
/* basic machine hardware */
MCFG_CPU_ADD ("maincpu", M68000, XTAL_10MHz)
MCFG_CPU_PROGRAM_MAP (mzr8105_mem)


/* Terminal Port config */
MCFG_UPD7201_ADD("upd", XTAL_4MHz, 0, 0, 0, 0 )
MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_txd))
MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_dtr))
MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232trm", rs232_port_device, write_rts))

MCFG_RS232_PORT_ADD ("rs232trm", default_rs232_devices, "terminal")
MCFG_RS232_RXD_HANDLER (DEVWRITELINE ("upd", upd7201_device, rxa_w))
MCFG_RS232_CTS_HANDLER (DEVWRITELINE ("upd", upd7201_device, ctsa_w))

MCFG_DEVICE_ADD ("updterm_clock", CLOCK, UPD_CLOCK)
MCFG_CLOCK_SIGNAL_HANDLER (WRITELINE (mzr8105_state, write_updterm_clock))

#if CARDSLOT

// EPROM sockets
MCFG_FRAGMENT_ADD(mzr8105_eprom_sockets)
#endif

MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (mzr8105)
ROM_REGION (0x1000000, "maincpu", 0)

ROM_LOAD16_BYTE ("mzros9LB.bin", 0x000001, 0x2000, CRC (7c6a354d) SHA1 (2721eb649c8046dbcb517a36a97dc0816cd133f2))
ROM_LOAD16_BYTE ("mzros9HB.bin", 0x000000, 0x2000, CRC (d18e69a6) SHA1 (a00b68f4d649bcc09a29361f8692e52be12b3792))

/*
 * System ROM information
 *
 * The ROMs contains an OS9 bootloader. It is position independent but reset vector suggests that it sits flat on adress 0 (zero)
 *
 * We got a prompt but the command reportoair are very sparse to non OS9 users so investigation needed.
 *
 */
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1987, mzr8105,      0,      0,       mzr8105,        mzr8105, driver_device, 0,   "Mizar Inc",             "Mizar VME8105", MACHINE_IS_SKELETON )
