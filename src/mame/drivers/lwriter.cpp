// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom, Jonathan Gevaryahu
/******************************************************************************

    Apple LaserWriter II NT driver

        0x000000 - 0x1fffff     SRAM/ROM (switches based on overlay)
        0x200000 - 0x3fffff     ROM
        0x400000 - 0x5fffff     RAM
        0x600000 - 0x7fffff     ??? more RAM?
        0x800000 - 0x9fffff     LED/Printer Controls(MSB), FIFO to print mechanism(LSB)
        0xa00000 - 0xbfffff     Zilog 8530 SCC (Serial Control Chip) Read
        0xc00000 - 0xdfffff     Zilog 8530 SCC (Serial Control Chip) Write
        0xe00000 - 0xefffff     Rockwell 6522 VIA
        0xf00000 - 0xffffef     ??? (the ROM appears to be accessing here)
        0xfffff0 - 0xffffff     ???Auto Vector??

    TODO:
    - Get the board to pass its self test, it fails long before it even bothers reading the dipswitches
    - Hook up the rest of the VIA pins to a canon printer HLE stub
    - Hook up ADB bitbang device to the VIA CB1, CB2 and PortA pins
    - Hook up VIA Port A, bits 5 and 6 to the SW1 and SW2 panel switches
    - Everything else
    DONE:
    - Hook up SCC and VIA interrupt pins to the 68k
    Future:
    - Let the board identify itself to a emulated mac driver so it displays the printer icon on the desktop

    Self Test LEDs at 0x800000-800001 most significant nybble:
    0x8 - <Passes> cpu check? (displayed before the overlay is disabled)
    0xF - <Passes> 200000-3fffff ROM checksum
    0xE - <Passes> 400000-400007 Low half of DRAM individual bit tests (walking ones and zeroes)
    0xD - <Passes> 5ffff8-5fffff High half of DRAM individual bit tests (walking ones and zeroes)
    0xC - <Passes> 400000-5fffff comprehensive DRAM data test
    0xB - <Passes?> Unknown test
    0xA - <Passes> dies if 600000-7fffff doesn't mirror 400000-5fffff ?
    0x8 - <Passes> SRAM test 000000-000FFF (2e3616-2e3664)
    0x0 - main loop? starts at 2e0002 (2e2fd0... 2db764 is the end of the 'clear ram 41d53f down to 400000' loop...),
    touches the via and fires an int, but runs off into the weeds
    If one of the self tests fails, the uppermost bit will oscillate (c000 4000 c000 4000 etc) forever

******************************************************************************/
/*
 * Hardware: 68000@11.16 MHz
             8530 SCC
             65C22 VIA on newer pcbs (older pcbs might have a 6523/6525 TPI but the pinout is completely different?)
             2MB DRAM
             2KB SRAM
             X2804 EEPROM (custom marked as 335-0022) [note that technically a 2808 or 2816 can go here and will work too]
             1MB ROM
             MMI67L401 64x4 FIFO, x2

   +------------------------------------------------------------------------------------------------------------------------+=====+
   |      1           2            3           4            5          6          7          8        9        10     11    |     #
   |    +------+    +------+   +---------+                                                +-------+  +------+         +-+   |     #
   |A   |511000|    | F257 |   |335-0022 |                                                | 0296  |  |22.3210         | | J2|     #
   |    +------+    +------+   |EEPROM   |                     +-------+                  +-------+  |XTAL  |         | |   |     #
   |B   |511000|    | RP2B |   +---------+                     |       |                             +------+         | |   |     #
   |    +------+    +------+                                   | 68000 |                  +-------+  +-------+        +-+   |     #
   |C   |511000|    | F257 |                                   |       |                  | 0558  |  | F175  |        +-+   |     #
   |    +------+    +------+   +---------+                     |       |                  +-------+  +-------+        | |   |     #
   |D   |511000|    +------+   |Am9128-10|                     |       |        +-------+ +-------+  +-------+        | |   |     #
   |    +------+    | F257 |   |SRAM     |                     |       |        | 0559  | | 0557  |  | LS393 |        | |   |     #
   |E   |511000|    +------+   +---------+                     |       |        +-------+ +-------+  +-------+        +-+   |  F  #
   |    +------+    +------+   +---------+                     |       |                                      +------+      |     #
   |F   |511000|    | F138 |   |Am9128-10|                     |       |                +-----------------+   |26LS32|    J3|  R  #
   |    +------+    +------+   |SRAM     |                     |       |                | Z8530B1C        |   +------+      |     #
   |G   |511000|               +--------++                     |       |                | SCC             |   |26LS32|      |  O  #
   |    +------+    +------+   | F244   |                      |       |     +----+     +-----------------+   +------+      |     #
   |H   |511000|    | RP2H |   +--------+                      +-------+     |7705|                           +------+      |  N  #
   |    +------+    +------+   | F244   |                                    +----+     +-----------------+   |26LS30|      |     #
   |J   |511000|    | 0259 |   +--------+-+----------+                   o------o       | 338-6523        |   +------+      |  T  #
   |    +------+    +------+   | TC531000 | TC531000 |                   | CONN |       | TPI             |   |26LS30|      |     #
   |K   |511000|    +------+   |  ROM H3  |  ROM L3  |          +------+ |   == |       +-----------------+   +------+      |     #
   |    +------+    | RP2L |   +----------+----------+          | F02  | |   == |             +-------+                     |     #
   |L   |511000|    +------+   +----------+----------+          +------+ |   == |             | LS14  |                     |     #
   |    +------+    | 0259 |   | TC531000 | TC531000 |  +------++------+ |   == |             +-------+                     |     #
   |M   |511000|    +------+   |  ROM H2  |  ROM L2  |  |67L401|| LS166| |   == |             | LS14  |                     |     #
   |    +------+    +------+   +----------+----------+  +------++------+ |   == |             +-------+ +------+            |     #
   |N   |511000|    | F245 |   +----------+----------+  +------++------+ |   == |                       | 0440A|            |     #
   |    +------+    +------+   | TC531000 | TC531000 |  |67L401|| LS00 | |   == |                       +------+            |     #
   |P   |511000|    +------+   |  ROM H1  |  ROM L1  |  +------++------+ o------o      +------+                             |     #
   |    +------+    | F245 |   +----------+----------+         +-------+               |TL497 |                             |     #
   |R   |511000|    +------+   +----------+----------+         | LS273 |               +------+                             |     #
   |    +------+    +------+   | TC531000 | TC531000 |         +-------+                                                    |     #
   |S   |511000|    | RP2S |   |  ROM H0  |  ROM L0  |         | LS05 |                                                     |     #
   |    +------+    +------+   +----------+----------+         +------+  (c)1987                                          J4|     #
   |     DRAM                       LASERWRITER II NT                    Apple Computer               640-4105              |     #
   +------------------------------------------------------------------------------------------------------------------------+=====+
 */

#define TPI 0  //The TPI is used on the original M6009 board but the first dump is from a newer that uses a VIA

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/z80scc.h"

#if TPI
#include "machine/6525tpi.h"
#else
#include "machine/6522via.h"
#endif

class lwriter_state : public driver_device
{
public:
	lwriter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_scc(*this, "scc")
		, m_via(*this, "via")
		, m_overlay(1)
	{ }

	void lwriter(machine_config &config);

private:
	DECLARE_READ16_MEMBER(bankedarea_r);
	DECLARE_WRITE16_MEMBER(bankedarea_w);
	DECLARE_WRITE8_MEMBER(led_out_w);
	DECLARE_WRITE8_MEMBER(fifo_out_w);
	DECLARE_READ8_MEMBER(via_pa_r);
	DECLARE_WRITE8_MEMBER(via_pa_w);
	DECLARE_WRITE_LINE_MEMBER(via_ca2_w);
	DECLARE_READ8_MEMBER(via_pb_r);
	DECLARE_WRITE8_MEMBER(via_pb_w);
	DECLARE_WRITE_LINE_MEMBER(via_cb1_w);
	DECLARE_WRITE_LINE_MEMBER(via_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(via_int_w);
	//DECLARE_WRITE_LINE_MEMBER(scc_int);
	virtual void machine_start () override;
	virtual void machine_reset () override;
	void maincpu_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<scc8530_device> m_scc;

#if TPI
#define tpi6523_device tpi6525_device/* TODO: define a type for the TPI6523 of its own in the device header file */
	required_device<tpi6523_device> m_tpi;
#else
	required_device<via6522_device> m_via;
#endif

	uint16_t *m_dram_ptr, *m_sram_ptr, *m_rom_ptr;
	bool m_overlay;
};

/*
Address maps (x = ignored; * = selects address within this range)
68k address map:
a23 a22 a21 a20 a19 a18 a17 a16 a15 a14 a13 a12 a11 a10 a9  a8  a7  a6  a5  a4  a3  a2  a1  (a0 via UDS/LDS)
*   *   *                                                                                           PAL16R6 U80
*   *   *   *   *   *                                                                               decoded by pals
Overlay ON:
0   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROM
0   0   0   0   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN1
0   0   0   0   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN2
0   0   0   0   1   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN3
0   0   0   0   1   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN4
0   0   0   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x       OPEN BUS
Overlay OFF:
0   0   0   ?   ?  ?0?  x   x   x   x   x   x   *   *   *   *   *   *   *   *   *   *   *   *       RW  SRAM
Unknown:
?   ?   ?   ?   ?   ?   x   x   x   x   x   x  (*) (*)  *   *   *   *   *   *   *   *   *   1       RW  2804 EEPROM
  (technically a10 and a11 are ignored, but if a 2808 or 2816 is put in this spot the address lines do connect to the appropriate pins)
Common:
0   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROM
0   0   1   0   0   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN1
0   0   1   0   0   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN2
0   0   1   0   1   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN3
0   0   1   0   1   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       R   ROMEN4
0   0   1   1   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x       OPEN BUS
0   1   0   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW  DRAM
0   1   1   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *       RW ???? DRAM mirror?
1   0   0   ?   ?   ?   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   0       W   64x8 FIFO
1   0   0   ?   ?   ?   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   1       W   Status LEDs and mech
1   0   1   ?   ?   ?   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *   *   1       R   8530 SCC Read
1   1   0   ?   ?   ?   x   x   x   x   x   x   x   x   x   x   x   x   x   x   x   *   *   0       W   8530 SCC Write
1   1   1  ?x? ?0?  ?   x   x   x   x   x   x   x   x   x   x   x   x   x   *   *   *   *   0       RW  65C22 VIA
1   1   1  ?x? ?1?  ?   x   x   x   x   x   x   x   x   x   x   x   x   x   *   *   *   *   0       RW  debugger rom/pod area
              |               |               |               |               |
map when overlay is set:
000000-1fffff ROM (second half is open bus)
map when overlay is clear:
000000-03ffff SRAM(?)
040000-1fffff ????
200000-3fffff ROM (second half is open bus)
400000-5fffff DRAM
600000-7fffff DRAM mirror
800000-83ffff LEDs and status bits to printer mechanism, FIFO
840000-9fffff unknown
a00000-a3ffff SCC read
a40000-bfffff unknown
c00000-c3ffff SCC write
c40000-dfffff unknown
e00000-e3ffff VIA
e40000-f7ffff unknown
f80000-fbffff debug area (first read must be 0xAAAA5555, then 68k will jump to address of second read)
fc0000-ffffff unknown

The ADB bit-bang transceiver MCU connects to the VIA CB1 (adbclk) and CB2 (adbdata) pins,
as well as PA0 (ST1), PA2 (ST2) and PA3 (ADB /INT)
*/

void lwriter_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).rw(FUNC(lwriter_state::bankedarea_r), FUNC(lwriter_state::bankedarea_w));
	map(0x200000, 0x2fffff).rom().region("rom", 0); // 1MB ROM
	//map(0x300000, 0x3fffff) // open bus?
	map(0x400000, 0x5fffff).ram().region("dram", 0).mirror(0x200000); // 2MB DRAM
	map(0x800000, 0x800000).w(FUNC(lwriter_state::led_out_w)).mirror(0x1ffffe); // mirror is a guess given that the pals can only decode A18-A23
	map(0x800001, 0x800001).w(FUNC(lwriter_state::fifo_out_w)).mirror(0x1ffffe); // mirror is a guess given that the pals can only decode A18-A23
	map(0xc00001, 0xc00001).w(m_scc, FUNC(scc8530_device::ca_w)).mirror(0x1ffff8);
	map(0xc00005, 0xc00005).w(m_scc, FUNC(scc8530_device::da_w)).mirror(0x1ffff8);
	map(0xa00000, 0xa00000).r(m_scc, FUNC(scc8530_device::ca_r)).mirror(0x1ffff8);
	map(0xa00004, 0xa00004).r(m_scc, FUNC(scc8530_device::da_r)).mirror(0x1ffff8);

	map(0xc00003, 0xc00003).w(m_scc, FUNC(scc8530_device::cb_w)).mirror(0x1ffff8);
	map(0xc00007, 0xc00007).w(m_scc, FUNC(scc8530_device::db_w)).mirror(0x1ffff8);
	map(0xa00002, 0xa00002).r(m_scc, FUNC(scc8530_device::cb_r)).mirror(0x1ffff8);
	map(0xa00006, 0xa00006).r(m_scc, FUNC(scc8530_device::db_r)).mirror(0x1ffff8);

#if TPI
	map(0xe00010, 0xe0001f).rw("tpi", FUNC(tpi6523_device::read), FUNC(tpi6523_device::write)).umask16(0x00ff).mirror(0x17ffe0); // Used on older boards, needs proper mapping
#else
	map(0xe00000, 0xe0001f).m(m_via, FUNC(via6522_device::map)).umask16(0x00ff).mirror(0x17ffe0);
#endif
}

static INPUT_PORTS_START( lwriter )
INPUT_PORTS_END

/* Start it up */
void lwriter_state::machine_start()
{
	m_rom_ptr = (uint16_t*)memregion("rom")->base();
	m_dram_ptr = (uint16_t*)memregion("dram")->base();
	m_sram_ptr = (uint16_t*)memregion("sram")->base();
	// do stuff here later on like setting up printer mechanisms HLE timers etc
}

void lwriter_state::machine_reset()
{
	/* Reset the VIA */
	m_via->reset();
}

/* Overlay area */
READ16_MEMBER(lwriter_state::bankedarea_r)
{
	if (m_overlay)
	{
		return m_rom_ptr[offset];
	}
	else if (offset <= 0x01ffff)
	{
		if ((offset > 0x7ff) && !machine().side_effects_disabled()) { logerror("Attempt to read banked area (with overlay off) past end of SRAM from offset %08X!\n",offset<<1); }
		return m_sram_ptr[offset&0x7FF];
	}
	if(!machine().side_effects_disabled()) { logerror("Attempt to read banked area (with overlay off) past end of SRAM from offset %08X! Returning 0xFFFF!\n",offset<<1); }
	return 0xFFFF;
}

WRITE16_MEMBER(lwriter_state::bankedarea_w)
{
	if (m_overlay)
	{
		if(!machine().side_effects_disabled()) { logerror("Attempt to write banked area (with overlay ON) with data %04X to offset %08X IGNORED!\n",data, offset<<1); }
		return;
	}
	else if (offset <= 0x01ffff)
	{
		if ((offset > 0x7ff) && !machine().side_effects_disabled()) { logerror("Attempt to write banked area (with overlay off) with data %04X to offset %08X!\n",data, offset<<1); }
		COMBINE_DATA(&m_sram_ptr[offset&0x7FF]);
		return;
	}
	if(!machine().side_effects_disabled()) { logerror("Attempt to write banked area (with overlay off) with data %04X to offset %08X IGNORED!\n", data, offset<<1); }
}

/* 4 diagnostic LEDs, plus 4 i/o lines for the printer */
WRITE8_MEMBER(lwriter_state::led_out_w)
{
	//popmessage("LED status: %02X\n", data&0xFF);
	logerror("LED status: %02X\n", data&0xFF);
	popmessage("LED status: %x %x %x %x %x %x %x %x\n", data&0x80, data&0x40, data&0x20, data&0x10, data&0x8, data&0x4, data&0x2, data&0x1);
}

/* FIFO to printer, 64 bytes long */
WRITE8_MEMBER(lwriter_state::fifo_out_w)
{
	/** TODO: actually emulate this */
	logerror("FIFO written with: %02X\n", data&0xFF);
}

/* via stuff */
READ8_MEMBER(lwriter_state::via_pa_r)
{
	logerror(" VIA: Port A read!\n");
	return 0xFF;
}

WRITE8_MEMBER(lwriter_state::via_pa_w)
{
	logerror(" VIA: Port A written with data of 0x%02x!\n", data);
}

WRITE_LINE_MEMBER(lwriter_state::via_ca2_w)
{
	logerror(" VIA: CA2 written with %d!\n", state);
}

READ8_MEMBER(lwriter_state::via_pb_r)
{
	logerror(" VIA: Port B read!\n");
	return 0xFF;
}

WRITE8_MEMBER(lwriter_state::via_pb_w)
{
	logerror(" VIA: Port B written with data of 0x%02x!\n", data);
	/* Like early Mac models which had VIA A4 control overlaying, the
	 * LaserWriter II NT overlay is controlled by VIA B3 */
	m_overlay = BIT(data,3);
}

WRITE_LINE_MEMBER (lwriter_state::via_cb1_w)
{
	logerror(" VIA: CB1 written with %d!\n", state);
}

WRITE_LINE_MEMBER(lwriter_state::via_cb2_w)
{
	logerror(" VIA: CB2 written with %d!\n", state);
}

WRITE_LINE_MEMBER(lwriter_state::via_int_w)
{
	logerror(" VIA: INT output set to %d!\n", state);
	//TODO: this is likely wrong, the VPA pin which controls whether autovector is enabled or not is controlled by PAL U8D, which is not dumped.
	m_maincpu->set_input_line(M68K_IRQ_1, (state ? ASSERT_LINE : CLEAR_LINE));
}

/* scc stuff */
/*
WRITE_LINE_MEMBER(lwriter_state::scc_int)
{
    logerror(" SCC: INT output set to %d!\n", state);
    //m_via->set_input_line(VIA_CA1, state ? ASSERT_LINE : CLEAR_LINE);
    m_via->write_ca1(state);
}*/

#define CPU_CLK (22.321_MHz_XTAL / 2) // Based on pictures form here: http://picclick.co.uk/Apple-Postscript-LaserWriter-IINT-Printer-640-4105-M6009-Mainboard-282160713108.html#&gid=1&pid=7
#define RXC_CLK ((CPU_CLK.value() - (87 * 16 * 70)) / 3) // Tuned to get 9600 baud according to manual, needs rework based on real hardware

void lwriter_state::lwriter(machine_config &config)
{
	M68000(config, m_maincpu, CPU_CLK);
	m_maincpu->set_addrmap(AS_PROGRAM, &lwriter_state::maincpu_map);

	SCC8530N(config, m_scc, CPU_CLK);
	m_scc->configure_channels(RXC_CLK, 0, RXC_CLK, 0);
	/* Port A */
	m_scc->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	/* Port B */
	m_scc->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	/* Interrupt */
	m_scc->out_int_callback().set("via", FUNC(via6522_device::write_ca1));
	//m_scc->out_int_callback().set(FUNC(lwriter_state::scc_int));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set("scc", FUNC(scc8530_device::rxa_w));
	rs232a.cts_handler().set("scc", FUNC(scc8530_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, "terminal"));
	rs232b.rxd_handler().set("scc", FUNC(scc8530_device::rxb_w));
	rs232b.cts_handler().set("scc", FUNC(scc8530_device::ctsb_w));

#if TPI
	TPI6525(config, "tpi", 0);
#else
	VIA6522(config, m_via, CPU_CLK/10); // 68000 E clock presumed
	m_via->readpa_handler().set(FUNC(lwriter_state::via_pa_r));
	m_via->readpb_handler().set(FUNC(lwriter_state::via_pb_r));
	m_via->writepa_handler().set(FUNC(lwriter_state::via_pa_w));
	m_via->writepb_handler().set(FUNC(lwriter_state::via_pb_w));
	m_via->cb1_handler().set(FUNC(lwriter_state::via_cb1_w));
	m_via->ca2_handler().set(FUNC(lwriter_state::via_ca2_w));
	m_via->cb2_handler().set(FUNC(lwriter_state::via_cb2_w));
	m_via->irq_handler().set(FUNC(lwriter_state::via_int_w));
#endif
}

/* SCC init sequence
 * :scc B Reg 09 <- c0 - Master Interrupt Control - Device reset
 * -
 * :scc A Reg 0f <- 00 - External/Status Control Bits - Disable all
 * :scc B Reg 05 <- 02 - Tx setup: 5 bits, Tx disable, RTS:1 DTR:0
 * :scc B Reg 05 <- 00 - Tx setup: 5 bits, Tx disable, RTS:0 DTR:0
 * -
 * :scc A Reg 09 <- c0 - Master Interrupt Control - Device reset
 *
 * -
 * :scc A Reg 0f <- 00 - External/Status Control Bits - Disable all
 * :scc A Reg 04 <- 4c - Setting up Asynchrounous mode: 2 Stop bits, No parity, 16x clock
 * :scc A Reg 0b <- 50 - Clock Mode Control - TTL clk on RTxC, Rx and Tx clks from BRG, TRxC is input
 * :scc A Reg 0e <- 00 - Misc Control Bits - BRG clk is RTxC, BRG is disabled
 * :scc A Reg 0c <- 0a - Low byte of baudrate generator constant
 * :scc A Reg 0d <- 00 - Hi byte of baudrate generator constant
 * :scc A Reg 0e <- 01 - BRG enabled with external clk from RTxC
 * :scc A Reg 0a <- 00 - Synchronous parameters, all turned off
 * :scc A Reg 03 <- c1 - Rx setup: 8 bits, Rx enabled
 * :scc A Reg 05 <- 6a - Tx setup: 8 bits, Tx enable, RTS:1 DTR:0
 * -
 * :scc A Reg 01 <- 00 - Rx interrupt disabled
 * :scc A Reg 01 <- 30 - Wait/Ready on receive, Rx int an all characters, parity affect vector
 * :scc A Reg 00 <- 30 - Error Reset command
 * -
 * :scc A Reg 01 <- 01 - External interrupt enabled, Rx ints disabled
 * :scc A Reg 00 <- 30 - Error Reset command
 * :scc A Reg 00 <- 30 - Error Reset command
 * - last three loops
*/


ROM_START(lwriter)
	ROM_REGION16_BE( 0x200000, "rom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE("342-0545.l0", 0x000001, 0x20000, CRC (6431742d) SHA1 (040bd5b84b49b86f2b0fe9ece378bbc7a10a94ec)) // Label: "342-0545-A JAPAN // TC531000CP-F700 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L0
	ROM_LOAD16_BYTE("342-0546.h0", 0x000000, 0x20000, CRC (c592bfb7) SHA1 (b595ae225238f7fabd1566a3133ea6154e082e2d)) // Label: "342-0546-A JAPAN // TC531000CP-F701 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H0
	ROM_LOAD16_BYTE("342-0547.l1", 0x040001, 0x20000, CRC (205a5ea8) SHA1 (205fefbb5c67a07d57cb6184c69648321a34a8fe)) // Label: "342-0547-A JAPAN // TC531000CP-F702 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L1
	ROM_LOAD16_BYTE("342-0548.h1", 0x040000, 0x20000, CRC (f616e1c3) SHA1 (b9e2cd4d07990b2d1936be97b6e89ef21f06b462)) // Label: "342-0548-A JAPAN // TC531000CP-F703 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H1
	ROM_LOAD16_BYTE("342-0549.l2", 0x080001, 0x20000, CRC (0b0b051a) SHA1 (64a80085001570c3f99d9865031715bf49bd7698)) // Label: "342-0549-A JAPAN // TC531000CP-F704 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L2
	ROM_LOAD16_BYTE("342-0550.h2", 0x080000, 0x20000, CRC (82adcf85) SHA1 (e2ab728afdae802c0c67fc25c9ba278b9cb04e31)) // Label: "342-0550-A JAPAN // TC531000CP-F705 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H2
	ROM_LOAD16_BYTE("342-0551.l3", 0x0c0001, 0x20000, CRC (176b3346) SHA1 (eb8dfc7e44f2bc884097e51a47e2f10ee091c9e9)) // Label: "342-0551-A JAPAN // TC531000CP-F706 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L3
	ROM_LOAD16_BYTE("342-0552.h3", 0x0c0000, 0x20000, CRC (69b175c6) SHA1 (a84c82be1ec7e373bb097ee74b941920a3b091aa)) // Label: "342-0552-A JAPAN // TC531000CP-F707 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H3
	ROM_REGION( 0x200000, "dram", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000, "sram", ROMREGION_ERASEFF )

ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    STATE          INIT        COMPANY            FULLNAME                    FLAGS */
CONS( 1988, lwriter, 0,      0,      lwriter, lwriter, lwriter_state, empty_init, "Apple Computer",  "Apple Laser Writer II NT", MACHINE_IS_SKELETON)
