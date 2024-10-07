// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 * The 4490 CNC Mill Control unit
 *
 * History of Terco
 *------------------
 * Terco, founded 1963, is a privately held company in Sweden that develops and distribute equipment for
 * technical vocational educations worldwide. In the mid 80s they had a number of state of the art
 * products for educations on CNC machines, both mill and lathe, all based on Motorola 8-bit CPU:s.
 *
 * Known products
 * --------------
 * T4426 - CNC Programming Station, a ruggedized Tandy Color Computer + monitor in a metal case (see coco12.cpp)
 * T4490 - CNC Control System, a complete system including a small milling machine, can be programmed by T4426
 * T???? - A CAD/CAM sold in small numbers, an office computer based on a Tandy Color Computer PCB + two floppy drives
 *
 * Misc links about the machines supported by this driver.
 *--------------------------------------------------------
 * https://www.westauction.com/auction/712/item/terco-cnc-programming-system-37047
 * https://kiertonet.fi/tyokalut-ja-koneet/koneet-ja-laitteet/cnc-pora-45-terco-4403
 * http://www.repair--parts.com/Repair-Electronics-/Rectifiers-/Terco-table-top-cnc-mill.php5
 *
 *  TODO:
 * ------
 *  - Display
 *  - Clickable Artwork
 *  - Serial communication for printer, plotter, paper tape and download from the T4426
 *  - Identify expansion bus
 *  - Keyboard Controller
 *  - Dump keyboard ROM
 *  - Cassette i/o
 */

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
//#include "machine/kb3600.h"
//#include "machine/mc14411.h"

#define LOG_SETUP   (1U << 1)
#define LOG_READ    (1U << 2)
#define LOG_BCD     (1U << 3)

//#define VERBOSE (LOG_BCD|LOG_SETUP)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGBCD(...)   LOGMASKED(LOG_BCD,     __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif
/*
 * Identified Chips
 * ----------------
 * 11442   - 7-seg driver
 * 14411   - Bit Rate Generator
 * 1488    - Quad Line EIA-232D Driver
 * 1489    - Quad Line EIA-232D Receivers
 * 15005   - Reed relay
 * 2114    - 4 x 1024 bits of data
 * 2732    - 4KB EPROM
 * 4527    - BCD Rate Multiplexer
 * 555     - analog timer circuit
 * 6800    - 8 bit CPU
 * 6821    - PIA parallel interface
 * 6850    - ACIA serial interface
 * 7400    - Quad 2 input NAND gates
 * 7402    - Quad 2 input NOR gates
 * 7408    - Quad 2 input AND gates
 * 7414    - 6 hex schmitt trigger inverters
 * 7420    - Dual 4 input NAND gates
 * 7421    - Dual 4 input AND gates
 * 7476    - Dual JK flip flops with preset and clear
 * 7490    - Decade Counters, divide by 2 and divide by 5 outputs
 * 74116   - Dual 4-bit latches
 * 74138   - 3 to 8 line decoder/demultiplexer
 * 74139   - Dual 2 to 4 line decoder/demultiplexer
 * 74154   - 4 line to 16 line decoders/multiplexers
 * 74164   - 8 bit paralell output serial shift register
 * 74174   - 6 D-type flip flops
 * 74240   - Inverted 3-state outputs
 * 74244   - Non-inverted 3-state outputs
 * AY3600  - Keyboard Controller
 * CA339   - Quad Voltage Comparators
 * CD4050B - CMOS Hex Non-Inverting Buffer and Converter
 * CIC2414 - CMOS RAM suspects
 * DS1210  - NVRAM controller, inhibits access and supply battery power on power down
 * MC6875  - Phase clock generator
 *
 * Board Layouts
 * -------------
 *
 *          T4490 CPU board
 *        ______________________________________________________________
 *      |\                              ..                 .---.      _o|__
 *      ||o+----------+  +---+ +---+ .------. +-------+    |470|     | |   |
 *      || | 2732     |  |CIC| |CIC| |battery |74LS240|    | uF|     | |   |
 *    ///> | 4490-F   |  2414E 2414E |      | +-------+    |   |     | |   |
 *       | +----------+  |   | |   | |      | +-------+    |   |     | |   |
 *       |               |   | |   | |      | |74LS240|    |   |     | |   |
 *       | +----------+  +---+ +---+ |      | +-------+    .___.     | |   |
 *       | | 2732     |              .______.                o       | |   |
 *       | | 4490-E   |     +-----+  +-------+                       | |   |
 *       | +----------+     |DS1210  |4527BE |    +-----------+      | |   |
 *       |                  +-----+  +-------+    |           |      | |   |
 *       | +----------+                           | 74116N    |      | |   |
 *       | | 2732     |                           +-----------+      | |   |
 *       | | 4490-D   |    +-------+ +-------+                       | |   |
 *       | +----------+    |74LS90N| |4527BE |                       | |   |
 *       |                 +-------+ +-------+                       | |   |
 *       | +----------+                           +-----------+      |_|   |
 *       | | 2732     |   +-------+  +-------+    |           |        |___|
 *       | | 4490-C   |   | 7476  |  |4527BE |    | 74116N    |        o|
 *       | +----------+   +-------+  +-------+    +-----------+         |
 *       |                                                              |
 *       | R2 varistor                                                  |
 *       |  +---+ R1     +---------+ +-------+                          |
 *       |  |555| 3.9K   | 74LS244 | |74LS00 |                          |
 *       |  +---+        +---------+ +-------+                          |
 *       | C1 0.01uF                                                    |
 *       | C2 0.01uF                                                   o|__
 *       |  +----+                                                    _|   |
 *       | -|XTAL|                                                   | |   |
 *       | -|8MHz|                                                   | |   |
 *       |  +----+       +-----------------+                         | |   |
 *       |               |   CPU           |            +-------+    | |   |
 *       | +-------+     |   68B00         |            |74LS139|    | |   |
 *       | | 6875L |     +-----------------+            +-------+    | |   |
 *       | +-------+                                                 | |   |
 *       |                                                           | |   |
 *       | +-------+         +--------+ +--------+      +-------+    | |   |
 *       | | 74LS08|         |74LS244 | |74LS244 |      |74LS138|    | |   |
 *       | +-------+         +--------+ +--------+      +-------+    | |   |
 *       |                                                           | |   |
 *       |                                                           | |   |
 *       | +-------+  +-------++-------+ +-------+      +-------+    | |   |
 *       | |74LS164|  |74LS14 ||74LS02N| |74LS20N|      |74LS138|    | |   |
 *    \\\> +-------+  +-------++-------+ +-------+      +-------+    |_|   |
 *      ||                                                             |___|
 *      ||o                                                             o|
 *      |/---------------------------------------------------------------+
 *
 *
 *       T4490 I/O board
 *      . ______________________________________________________________
 *      |\                                    o  o                    _o|__
 *      ||o                      96..+------+.----. +-------+        | |   |
 *      ||                       72..|BRG   ||XTAL| +74LS240|        | |   |
 *    ///>            +-------+  48..|14411 ||1.84| +-------+        | |   |
 *    +-+|O|+------+  |CA339A |  36..|      || 320|                  | |   |
 *  +-|    ||15005B|  +-------+  24..|      |.____. +-------+        | |   |
 *  | |  ==|+------+             18..|      |       |74LS240|        | |   |
 *  | |  ==|                     12..|      |       +-------+        | |   |
 *  | |  ==|                      6..+------+                        | |   |
 *  | |  ==|          +-------+   3.. .----------.                   | |   |
 *  | |  ==|          |1488P  |   2..o| 470uF    |o +-------+        | |   |
 *  | |  ==|          +-------+   1.5 .__________.  |74LS244|        | |   |
 *  | |  ==|                      1.1 +----------+  +-------+        | |   |
 *  | |  ==|                          | 2732A    |                   | |   |
 *  +-|    |+-------+ +-------+       | 4490-B   |                   | |   |
 *    +-+==||CD4050BE |1489P  |       +----------+ +-------+         | |   |
 *  \\   |O|+-------+ +-------+                    |74LS244|         |_|   |
 *   \\---++------+ +------+                       +-------+           |___|
 *  --+   O|PIA   | |PIA   |   +------+  +-----------+                 o|
 *    |   ||68B21P| |68B21P|   |ACIA  |  | 2732A     |                  |
 *    |   ||      | |      |   |68B50P|  | 4490-A    |                  |
 *    +-+ ||      | |      |   |      |  +-----------+                  |
 *    --| ||      | |      |   |      |                                 |
 *    +-+ ||      | |      |   |      |  +-----------+  +-------+       |
 *    |   ||      | |      |   +------+  | 2732A     |  |74LS139|       |
 *    |   ||      | |      |             | 4490-3    |  +-------+       |
 *    |   |+------+ +------+             +-----------+                 o|__
 *  --+   O                                             +-------+     _|   |
 *   //---+                                             |74LS00N|    | |   |
 *  // ==o==                                            +-------+    | |   |
 *     ==o==                                                         | |   |
 *       |  +-------+  +-------+   +-------+  +-------+ +-------+    | |   |
 *       |  | 2114  |  | 2114  |   | 2114  |  | 2114  | |74LS21N|    | |   |
 *       |  +-------+  +-------+   +-------+  +-------+ +-------+    | |   |
 *       |                                                           | |   |
 *       |  +-------+  +-------+   +-------+  +-------+              | |   |
 *       |  | 2114  |  | 2114  |   | 2114  |  | 2114  |              | |   |
 *       |  +-------+  +-------+   +-------+  +-------+  +-------+   | |   |
 *       |                                               |74LS08N|   | |   |
 *       |  +-------+  +-------+   +-------+  +-------+  +-------+   | |   |
 *       |  | 2114  |  | 2114  |   | 2114  |  | 2114  |              | |   |
 *       |  +-------+  +-------+   +-------+  +-------+              | |   |
 *       |                                                           | |   |
 *    \\\>  +-------+  +-------+   +-------+  +-------+  +-------+   |_|   |
 *      ||  | 2114  |  | 2114  |   | 2114  |  | 2114  |  |74LS138|     |___|
 *      ||o +-------+  +-------+   +-------+  +-------+  +-------+      o|
 *      |/---------------------------------------------------------------+
 *
 *
 *      T4490 Front Panel PCB
 * .--------------------------------------------------------------------------------------------------------------------.
 * |                                                                                                          +--------+|
 * |+-----+-----+-----+-----+-----+-----+-----+-----+-----+   +-----+   +-----+-----+                         |        ||
 * ||11442|11442|11442|11442|11442|11442|11442|11442|11442|   |11442|   |11442|11442|                         |74116N  ||
 * |+-----+-----+-----+-----+-----+-----+-----+-----+-----+   +-----+   +-----+-----+    POSITION REGISTER    +--------+|
 * | +---+---+---+     +---+---+     +---+---+---+     +---+    +---+     +---+---+  +------+  +---+---+---+---+---+---+|
 * N |7sg|7sg|7sg|   G |7sg|7sg|   F |7sg|7sg|7sg|   S |7sg|  T |7sg|   M |7sg|7sg|  |sparse|  |7sg|7sg|7sg|7sg|7sg|7sg||
 * | |BCD|BCD|BCD|     |BCD|BCD|     |BCD|BCD|BCD|     |BCD|    |BCD|     |BCD|BCD|  |DotLed|  |BCD|BCD|BCD|BCD|BCD|BCD||
 * | +---+---+---+     +---+---+     +---+---+---+     +---+    +---+     +---+---+  |matrix|  +---+---+---+---+---+---+|
 * |   BLOCK NBR        G-FUNCT       FEED-SPEED+-------+SPIN-  TOOL   +-------+ F-  +------++---+---+---+---+---+---+  |
 * |PWR  RUN  MAN  EXA  LOAD DUMP BBB  FULL PAR | 74116 | DLE    NBR   | 74116 | FUNCT       |114|114|114|114|114|114|  |
 * | O    O    O    O    O    O    O    O    O  +-------+              +-------+             | 42| 42| 42| 42| 42| 42|  |
 * | +------+  +--------+ +--------+ SYSTEM STATUS                                           |   |   |   |   |   |   |  |
 * | |74LS00|  |74LS240N| |74LS240N|                                                         +---+---+---+---+---+---+  |
 * | +------+  +--------+ +--------+                                                                                    |
 * ----------------------------------------------------------------------------------------------------------------------
 * O                                    O                                    O                                          O
 * ----------------------------------------------------------------------------------------------------------------------
 * |.----.----.----.----.----.----.----.----.        .----.----.----.----.----.----.----.----.            +-------+     |
 * ||STOP|STRT|PROG|EXAM|LOAD|DUMP|BLK |TO  |        |RST |SET |READ|CHNG|INS |ERA |END |END |            |74154J |     |
 * ||    |    |    |    |TAPE|TAPE|BBLK| REF|        |    | REF|    |    | BLK| BLK| BLK| PRG|      DATA  +-------+     |
 * |'----'----'----'----'----'----'----'----'        '----'----'----'----'----'----'----'----'        .----.----.----.  |
 * |                +--------+ SYSTEM CONTROL         PROGRAMMING CONTROL                             |    |    |    |  |
 * |                |74LS244N|                                                                        | 7  | 8  | 9  |  |
 * |                +--------+                                                                        +----+----+----+  |
 * |                                                                                                  |    |    |    |  |
 * |          JOG CONTROL                                 FUNCTION             COORDINATE             | 4  | 5  | 6  |  |
 * |.----.----.----.  +--------+                      .----.----.----.      .----.----.----.          +----+----+----+  |
 * ||    |    |    |  |74LS174N|                      |    |    |    |      |    |    |    |          |    |    |    |  |
 * || X+ | Y+ | Z+ |  +--------+.--.                  |CTRL| G  | F  |      | X  | Y  | Z  |          | 1  | 2  | 3  |  |
 * |+----+----+----+           |    |                 +----+----+----+      +----+----+----+          +----+----+----+  |
 * ||    |    |    |  +--------+'--'                  |    |    |    |      |    |    |    |          |    |    |    |  |
 * || X- | Y- | Z- |  |74LS240N|                      | S  | T  | M  |      | I  | J  | K  |          | +  | 0  | -  |  |
 * |+----+----+----+  +--------+                      +----+---++----+      +----+----+----+          '----'----'----'  |
 * |                                                       +---+                                                        |
 * |                  .-------------------.                |74 |                                                        |
 * |                  |       o    o      |                |240|                                                        |
 * |                  | +----+o    o+----+|+-----+         |   |              .-----------------------------------------'
 * |                  | |AY-5|o    o|    |||PIA  |         +---+              |
 * |                  | |3600|o    o|2732|||68B21|                            |
 * |                  | |PRO-|o    o|TBORD||     |     FEED OVERRIDE %        |
 * |                  | | 50 |o    o|    |||     |                            |
 * |                  | |    |o    o+----+||     |          .--.              |
 * |                  | |    |o    o      ||     |         |    |             |
 * |                  | +----+o    o      |+-----+          '--'              |
 * |                  '-------------------'                                   |
 * ---------------------------------------------------------------------------'------------------------------------------
 * O                                    O                                    O                                          O
 * ----------------------------------------------------------------------------------------------------------------------
 */


namespace {

/* Terco CNC Control Station 4490 */
class t4490_state : public driver_device
{
public:
	t4490_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	,m_maincpu(*this, "maincpu")
	,m_pia1(*this, "pia1")
	,m_pia2(*this, "pia2")
	//,m_pia3(*this, "pia3")
	//,m_acia(*this, "acia")
	//,m_brg(*this, "brg")
	//,m_ay3600(*this, "ay3600")
	{ }

	void t4490(machine_config &config);

private:
	void t4490_map(address_map &map) ATTR_COLD;
	required_device<m6800_cpu_device> m_maincpu;
  //    virtual void machine_reset() override { m_maincpu->reset(); LOG("--->%s()\n", FUNCNAME); };
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	//required_device<pia6821_device> m_pia3;
	//required_device<acia6850_device> m_acia;
	//required_device<mc14411_device> m_brg;
	//required_device<ay3600_device> m_ay3600;
};

void t4490_state::t4490_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x3000, 0x3fff).rom().region("maincpu", 0x3000);
	map(0x9500, 0x95ff).ram();
	map(0x9030, 0x9031).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x9034, 0x9037).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x9038, 0x903b).rw(m_pia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xa000, 0xffff).rom().region("maincpu", 0xa000);
}

/* Input ports */
static INPUT_PORTS_START( t4490 )
INPUT_PORTS_END

void t4490_state::t4490(machine_config &config)
{
	M6800(config, m_maincpu, XTAL(8'000'000)/4); // divided by a MC6875
	m_maincpu->set_addrmap(AS_PROGRAM, &t4490_state::t4490_map);

	/* devices */
	PIA6821(config, m_pia1);
	PIA6821(config, m_pia2);
	ACIA6850(config, "acia", 0);
}

ROM_START( t4490 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "terco4490-3861104.bin", 0x3000, 0x1000, CRC(d5fd17cc) SHA1(9a3564fa69b897ec51b49ad34f2d2696cb78ee9b) )
	ROM_LOAD( "terco4490-a861104.bin", 0xa000, 0x1000, CRC(65b8e7d0) SHA1(633217fc4aa301d87790bb8744b72ef030a4c262) )
	ROM_LOAD( "terco4490-b861104.bin", 0xb000, 0x1000, CRC(5a0ce3f2) SHA1(7ec455b9075454ce5943011a1dfb5725857168f5) )
	ROM_LOAD( "terco4490-c861104.bin", 0xc000, 0x1000, CRC(0627c68c) SHA1(bf733d3ffad3f1e75684e833afc9d10d33ca870f) )
	ROM_LOAD( "terco4490-d861104.bin", 0xd000, 0x1000, CRC(2156476d) SHA1(0d70c6285541746ef15cad0d47b2d752e228abfc) )
	ROM_LOAD( "terco4490-e861104.bin", 0xe000, 0x1000, CRC(b317fa37) SHA1(a2e037a3a88b5d780067a86e52c6f7c103711a98) )
	ROM_LOAD( "terco4490-f861104.bin", 0xf000, 0x1000, CRC(a45bc3e7) SHA1(e12efa9a4c72e4bce1d59ad359ee66d7c3babfa6) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME                       FLAGS
COMP( 1986, t4490, 0,      0,      t4490,   t4490, t4490_state, empty_init, "Terco AB", "Terco 4490 Mill CNC Control", MACHINE_IS_SKELETON )
