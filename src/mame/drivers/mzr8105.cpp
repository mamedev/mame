// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Mizar VME8105 rev D 3U SBC board driver
 *
 *  19/08/2015
 *
 * I bought this board from http://www.retrotechnology.com without documentation.
 * It has a Motorola 68000 CPU @ 10MHz and two 27128 EPROMS with OS9 DEBUG labels
 * and not much more except 16 or so TTLs, 2 PALs and a VME P1 connector. It is a
 * 2 layered pcb so it shold be possible to trace the schematics quite easily.
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
 * Mizar's standard double-height (6U) processors provide additional features such
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
 * MZX 414
 * MZ 7122
 * MZ 7132
 * MZ 7300 Serial I/O Board (Z8530)
 * EMX 7320 Serial I/O Board
 * MZ 7400 Disk Controller Board (WD 2010; WD 1772)
 * MZ 7500 IEEE-488 (GPIB) Interface Board
 * EMX 7550 Ethernet Controller Board (AMD 7990 Lance) MAC: 00:80:F8 MIZAR, INC.
 * MZ 7772
 * MZ 7810 I/O Expansion Module (6681 DUART)
 * MZ 7831 SCSI Expansion Module (WD 33C93A)
 * MZ 7850 Ethernet Expansion Module (WD 83C690)
 * MZ 8000 ??
 * MZ 8105 3U 68000 CPU board
 * MZ 8115
 * MZ 8300 3U serial board, 2 NEC 7201 and 1 AMD CTS9513 5 x 16 bit Counter/Timer
 * MZ 8310 timing module 2 x AM9513
 * MZ 8505 IEEE-488 (GPIB) Interface Board"
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
 *  - understand what other device is expected in VME space ff0011-13
 *
 ****************************************************************************/
#include "emu.h"
#include "bus/vme/vme.h"
#include "bus/vme/vme_mzr8300.h"
#include "bus/vme/vme_mvme350.h"
#include "cpu/m68000/m68000.h"
#include "machine/clock.h"

class mzr8105_state : public driver_device
{
public:
mzr8105_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device (mconfig, type, tag)
	,m_maincpu (*this, "maincpu")
	{
	}

	void mzr8105(machine_config &config);

private:
	void mzr8105_mem(address_map &map);
	required_device<cpu_device> m_maincpu;
};

void mzr8105_state::mzr8105_mem(address_map &map)
{
	map.unmap_value_high();
/* The ROMs contains an OS9 bootloader. It is position independent but reset vector suggests that it sits flat on adress 0 (zero) */
	map(0x000000, 0x003fff).rom().region("roms", 0x000000); /* System EPROM Area 16Kb OS9 DEBUG - not verified     */
	map(0x004000, 0x01ffff).rom().region("roms", 0x004000);/* System EPROM Area 112Kb for System ROM - not verified    */
	map(0x020000, 0x03ffff).ram(); /* Not verified */
//  map(0x100000, 0xfeffff).rw(FUNC(mzr8105_state::vme_a24_r), FUNC(mzr8105_state::vme_a24_w)); /* VMEbus Rev B addresses (24 bits) - not verified */
//  map(0xff0000, 0xffffff).rw(FUNC(mzr8105_state::vme_a16_r), FUNC(mzr8105_state::vme_a16_w)); /* VMEbus Rev B addresses (16 bits) - not verified */
}

/* Input ports */
static INPUT_PORTS_START (mzr8105)
INPUT_PORTS_END

static void mzr8105_vme_cards(device_slot_interface &device)
{
	device.option_add("mzr8300", VME_MZR8300);
}

/*
 * Machine configuration
 */
void mzr8105_state::mzr8105(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(10'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &mzr8105_state::mzr8105_mem);
	VME(config, "vme", 0).use_owner_spaces();
	VME_SLOT(config, "slot1", mzr8105_vme_cards, "mzr8300", 1, "vme");
}

/* ROM definitions */
/* mzr8300 UPD7201 init sequence
 * :upd B Reg 02 <- 04 Interrupt vector
 * :upd B Reg 06 <- 00 Sync byte
 * :upd B Reg 07 <- 00 Sync byte
 * :upd B Reg 03 <- c1 Rx 8 bit chars + Rx enable
 * :upd B Reg 04 <- 44 16x clock, 1 stop bit ( == async mode), no parity
 * :upd B Reg 05 <- 68 Tx 8 bit chars + Tx enable
 */
ROM_START (mzr8105)
	ROM_REGION (0x20000, "roms", 0)
	ROM_LOAD16_BYTE ("mzros9lb.bin", 0x000000, 0x2000, CRC (7c6a354d) SHA1 (2721eb649c8046dbcb517a36a97dc0816cd133f2))
	ROM_LOAD16_BYTE ("mzros9hb.bin", 0x000001, 0x2000, CRC (d18e69a6) SHA1 (a00b68f4d649bcc09a29361f8692e52be12b3792))
ROM_END

/* Driver */
//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME         FLAGS
COMP( 1987, mzr8105, 0,      0,      mzr8105, mzr8105, mzr8105_state, empty_init, "Mizar Inc", "Mizar VME8105", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
