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
             "6523" VIA (actually a custom-marked Rockwell R65NC22, not to be confused with the MOS 6523/65C23 TPI;
                         may also be sourced as VLSI VL65C22V-02PC)
             2MB DRAM
             2KB SRAM
             X2804 EEPROM (custom marked as 335-0022) [note that technically a 2808 or 2816 can go here and will work too]
             1MB ROM
             MMI67L401 64x4 FIFO, x2
             342-0440-A PIC (for ADB)

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
   |    +------+    +------+   | TC531000 | TC531000 |                   | CONN |       | VIA             |   |26LS30|      |     #
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

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/z80scc.h"

#include "screen.h"

#define LOG_VIDEO   (1U << 1)

//#define VERBOSE (LOG_COMMAND)
#include "logmacro.h"

namespace {

enum print_state {
	READING_CMD,
	WRITING_RESULT
};

class lwriter_state : public driver_device
{
public:
	lwriter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_scc(*this, "scc")
		, m_via(*this, "via")
		, m_screen(*this, "screen")
		, m_vram_offset(0)
		, m_dsw1(*this, "DSW1")
		, m_dram_ptr(*this, "dram")
		, m_sram_ptr(*this, "sram")
		, m_rom_ptr(*this, "rom")
		, m_overlay(1)
		, m_via_pb(0)
		, m_print_sc(1)
		, m_print_state(READING_CMD)
		, m_print_bit(0)
		, m_print_cmd(0)
		, m_print_result(0)
		, m_sbsy(0)
		, m_cbsy(0)
		, m_vsync(0)
		, m_fifo_count(0)
		, m_pb6_tick_count(0)
	{ }

	void lwriter(machine_config &config);
	void lwriter2nt(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// A guess based on not very much
	static constexpr unsigned FB_HEIGHT = 3434;
	static constexpr unsigned FB_WIDTH = 2520;

	uint16_t bankedarea_r(offs_t offset);
	void bankedarea_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t eeprom_r(offs_t offset);
	void eeprom_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	void dram_or_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void led_out_w(uint8_t data);
	void fifo_out_w(uint8_t data);
	uint8_t via_pa_r();
	void via_pa_w(uint8_t data);
	void via_pa_lw_w(uint8_t data);
	void via_ca2_w(int state);
	uint8_t via_pb_r();
	uint8_t via_pb_lw2nt_r();
	void write_dtr(int state);
	void via_pb_w(uint8_t data);
	void via_cb1_w(int state);
	void via_cb2_w(int state);
	void via_int_w(int state);
	emu_timer *m_pb6_timer;
	TIMER_CALLBACK_MEMBER(pb6_tick);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	//void scc_int(int state);
	void maincpu_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<scc8530_device> m_scc;
	required_device<via6522_device> m_via;
	optional_device<screen_device> m_screen;
	std::unique_ptr<u8[]> m_vram;
	size_t m_vram_offset;
	required_ioport m_dsw1;

	required_shared_ptr<uint16_t> m_dram_ptr;
	required_region_ptr<uint16_t> m_sram_ptr, m_rom_ptr;
	bool m_overlay;
	uint8_t m_via_pb;
	bool m_print_sc;
	print_state m_print_state;
	int m_print_bit;
	int m_print_cmd;
	int m_print_result;
	bool m_sbsy;
	bool m_cbsy;
	bool m_vsync;
	int m_fifo_count;
	int m_pb6_tick_count;
	int m_vbl_count;
	int m_reset_count;
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
	map(0x400000, 0x5fffff).ram().share(m_dram_ptr); // 2MB DRAM
	// DRAM appears to mirrored through 0x600000 but when written to does a bitwise-or with the existing data
	// instead of replacing it. This presumeably improves the performance of these operations by avoiding
	// an extra memory read.
	map(0x600000, 0x7fffff).readonly().share(m_dram_ptr).w(FUNC(lwriter_state::dram_or_w));
	map(0x800000, 0x800000).w(FUNC(lwriter_state::led_out_w)).mirror(0x1ffffe); // mirror is a guess given that the pals can only decode A18-A23
	map(0x800001, 0x800001).w(FUNC(lwriter_state::fifo_out_w)).mirror(0x1ffffe); // mirror is a guess given that the pals can only decode A18-A23
	map(0xc00001, 0xc00001).w(m_scc, FUNC(scc8530_device::cb_w)).mirror(0x1ffff8);
	map(0xc00005, 0xc00005).w(m_scc, FUNC(scc8530_device::db_w)).mirror(0x1ffff8);
	map(0xa00000, 0xa00000).r(m_scc, FUNC(scc8530_device::cb_r)).mirror(0x1ffff8);
	map(0xa00004, 0xa00004).r(m_scc, FUNC(scc8530_device::db_r)).mirror(0x1ffff8);

	map(0xc00003, 0xc00003).w(m_scc, FUNC(scc8530_device::ca_w)).mirror(0x1ffff8);
	map(0xc00007, 0xc00007).w(m_scc, FUNC(scc8530_device::da_w)).mirror(0x1ffff8);
	map(0xa00002, 0xa00002).r(m_scc, FUNC(scc8530_device::ca_r)).mirror(0x1ffff8);
	map(0xa00006, 0xa00006).r(m_scc, FUNC(scc8530_device::da_r)).mirror(0x1ffff8);

	map(0xe00000, 0xe0001f).m(m_via, FUNC(via6522_device::map)).umask16(0x00ff).mirror(0x17ffe0);

	// overlaps with SCC but only uses even addresses
	map(0xc00000, 0xc00400).rw(FUNC(lwriter_state::eeprom_r), FUNC(lwriter_state::eeprom_w)).umask16(0xff00);
}

static INPUT_PORTS_START( lwriter )
	PORT_START("DSW1")
	// Switch 1 | Switch 2 | switchsetting value | Meaning
	// Down       Down                         0   serial batch mode 1200 baud (0)
	// Up         Down                         1   serial batch mode 9600 baud (1)
	// Down       Up                           2   diablo emulation (special switch = 0)
	// Down       Up                           2   executive mode (special switch = 1)
	// Up         Up                           3   apple talk
	PORT_DIPNAME(0x60, 0x20, "Switch")
	PORT_DIPSETTING(  0x00, "Serial batch mode 1200 baud")
	PORT_DIPSETTING(  0x40, "Serial batch mode 9600 baud")
	PORT_DIPSETTING(  0x20, "Diablo emulation (special switch = 0) / Executive mode (special switch = 1)")
	PORT_DIPSETTING(  0x60, "AppleTalk")
INPUT_PORTS_END

// 300 dots/inch: 1,863,813 Hz (from the CX manual) / (8 bits per byte) / (48 bytes written per interrupt / 24 ticks per interrupt) = ~116488.3
#define PB6_CLK 116488

/* Start it up */
void lwriter_state::machine_start()
{
	m_vram = make_unique_clear<uint8_t []>(FB_WIDTH * FB_HEIGHT / 8);

	// do stuff here later on like setting up printer mechanisms HLE timers etc
	m_pb6_timer = timer_alloc(FUNC(lwriter_state::pb6_tick), this);

	m_pb6_timer->adjust(attotime::from_hz(PB6_CLK));
	// Initialize ca1 to 1 so that we don't miss the first interrupt/transition to 0
	m_via->write_ca1(1);
}

void lwriter_state::machine_reset()
{
}

/* Overlay area */
uint16_t lwriter_state::bankedarea_r(offs_t offset)
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
	if (!machine().side_effects_disabled()) { logerror("Attempt to read banked area (with overlay off) past end of SRAM from offset %08X! Returning 0xFFFF!\n",offset<<1); }
	return 0xFFFF;
}

void lwriter_state::bankedarea_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
	if (!machine().side_effects_disabled()) { logerror("Attempt to write banked area (with overlay off) with data %04X to offset %08X IGNORED!\n", data, offset<<1); }
}

void lwriter_state::dram_or_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_dram_ptr[offset] |= data & mem_mask;
}

uint8_t lwriter_state::eeprom_r(offs_t offset)
{
	uint8_t result = 0;
	// adjust offset to match real hardware mapping
	switch (offset)
	{
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83: {
			constexpr uint8_t signature[] = {0x7a, 0x53, 0xda, 0x71};
			result = signature[offset-0x80];
			break;
		}
		case 0xb0:
			result = 0x1; // disable printing the test page (dostartpage)
			break;
		case 0xf3:
			result = 0x1; // special switch procedure 1 to invoke executive (58 + 0xb9)
			break;
	}

	logerror("eeprom_r! %x %x\n", offset, result);
	return result;
}

void lwriter_state::eeprom_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	logerror("eeprom_w! %x %x %x\n", offset, data, mem_mask);
}

TIMER_CALLBACK_MEMBER(lwriter_state::pb6_tick)
{
	m_pb6_tick_count += 1;
	m_via->write_pb6(0);
	m_via->write_pb6(1);
	m_pb6_timer->adjust(attotime::from_hz(PB6_CLK));
}

/* 4 diagnostic LEDs, plus 4 i/o lines for the printer */
/* 0 - print cbsy
 * 1 - print prnt
 * 2 - print vsync
 * 3 - print cprdy
 * 4 - led 1
 * 5 - led 2
 * 6 - led 3
 * 7 - led 4
 */
void lwriter_state::led_out_w(uint8_t data)
{
	//popmessage("LED status: %02X\n", data&0xFF);
	logerror("LED status: %02X\n", data&0xFF);
	m_cbsy = data & 1;
	if (!m_vsync && (data & 4)) { // vsync
		LOGMASKED(LOG_VIDEO, "vsync\n");
		m_vbl_count = 0;
	}
	m_vsync = (data & 4);
	popmessage("LED status: %x %x %x %x %x %x %x %x\n", data&0x80, data&0x40, data&0x20, data&0x10, data&0x8, data&0x4, data&0x2, data&0x1);
}

/* FIFO to printer, 64 bytes long */
void lwriter_state::fifo_out_w(uint8_t data)
{
	m_fifo_count += 8;
	if (m_vbl_count >= FB_HEIGHT) {
		m_vbl_count = 0;
	}

	m_vram[m_vbl_count * FB_WIDTH/8 + m_vram_offset] = data;
	m_vram_offset++;
	if (m_vram_offset > (FB_WIDTH*FB_HEIGHT/8)) {
		LOGMASKED(LOG_VIDEO, "vram reset\n");
		m_vram_offset = 0;
	}
}
/* via port a bits */
/* 0 - ?
 * 1 - print sbsy
 * 2 - ?
 * 3 - ?
 * 4 - print vsreq
 * 5 - switch setting low
 * 6 - switch setting high
 * 7 - print (STATUS/COMMAND Message Line)
 */
uint8_t lwriter_state::via_pa_r()
{
	logerror(" VIA: Port A read!\n");
	uint8_t result = m_dsw1->read();
	if (m_sbsy)
	{
		result |= 2;
	}
	result |= (m_print_sc << 7);
	return result | 0x1C;
}

void lwriter_state::via_pa_w(uint8_t data)
{
	logerror(" VIA: Port A written with data of 0x%02x!\n", data);
}

void lwriter_state::via_pa_lw_w(uint8_t data)
{
	via_pa_w(data);
	m_cbsy = data & 1;
}

void lwriter_state::via_ca2_w(int state)
{
	logerror(" VIA: CA2 written with %d!\n", state);
}

/* via port b bits:
 * 0 - ?
 * 1 - print rdy
 * 2 - print pprdy
 * 3 - overlay
 * 4 - scc w/req
 * 5 - resetfifo
 * 6 - timer 2 clk (mroclk)
 * 7 - vbl
 */
uint8_t lwriter_state::via_pb_r()
{
	logerror(" VIA: Port B read!\n");
	return 0xFB;
}

// The 3rd bit (PPRDY) is used for talking with the print controller
// and is inverted on II NT
uint8_t lwriter_state::via_pb_lw2nt_r()
{
	return via_pb_r() ^ 0x4;
}

void lwriter_state::via_pb_w(uint8_t data)
{
	if ((data & 0x20) && ((m_via_pb & 0x20) == 0)) {
		m_reset_count++;
		LOGMASKED(LOG_VIDEO, "reset fifo %d pb6 %d\n", m_fifo_count, m_pb6_tick_count);
	}
	if ((data & 0x80) && ((m_via_pb & 0x80) == 0)) {
		m_vbl_count++;
		LOGMASKED(LOG_VIDEO, "vbl fifo:%d vbl:%d vram_off: %ld\n", m_fifo_count, m_vbl_count, m_vram_offset);
		m_vram_offset = 0;
	}

	logerror(" VIA: Port B written with data of 0x%02x!\n", data);
	/* Like early Mac models which had VIA A4 control overlaying, the
	 * LaserWriter II NT overlay is controlled by VIA B3 */
	m_overlay = BIT(data,3);
	m_via_pb = data;
}

void lwriter_state::via_cb1_w(int state)
{
	logerror(" VIA: CB1 written with %d!\n", state);
}

void lwriter_state::via_cb2_w(int state)
{
	logerror(" VIA: CB2 written with %d!\n", state);
}

void lwriter_state::via_int_w(int state)
{
	logerror(" VIA: INT output set to %d!\n", state);
	//TODO: this is likely wrong, the VPA pin which controls whether autovector is enabled or not is controlled by PAL U8D, which is not dumped.
	m_maincpu->set_input_line(M68K_IRQ_1, (state ? ASSERT_LINE : CLEAR_LINE));
}

/* scc stuff */
/*
void lwriter_state::scc_int(int state)
{
    logerror(" SCC: INT output set to %d!\n", state);
    //m_via->set_input_line(VIA_CA1, state ? ASSERT_LINE : CLEAR_LINE);
    m_via->write_ca1(state);
}*/

#define CPU_CLK (22.321_MHz_XTAL / 2) // Based on pictures form here: http://picclick.co.uk/Apple-Postscript-LaserWriter-IINT-Printer-640-4105-M6009-Mainboard-282160713108.html#&gid=1&pid=7
#define RXC_CLK ((CPU_CLK.value() - (87 * 16 * 70)) / 3) // Tuned to get 9600 baud according to manual, needs rework based on real hardware
/* These are from LBP-CX Series Video Interface Manual:
 * http://beefchicken.com/retro/laserwriter/LBP-CX%20Series%20Video%20Interface%20Service%20Manual.pdf
 */
#define SR0 1
#define SR1 2
#define SR2 4
#define SR4 8
#define SR5 0xb
#define EC0 0x40
#define EC1 0x43
#define EC2 0x45
#define EC3 0x46
#define EC4 0x49
#define EC5 0x4a
#define EC6 0x4c
#define EC7 0x4f
#define EC14 0x5d

void lwriter_state::write_dtr(int state)
{
	// DTR seems to be used as a clock for communication with
	// the print controller. We arbitrarily choose state == 1
	// to run our code
	if (state == 1 && (m_cbsy || m_sbsy))
	{
		logerror("pb line %d %d %d!\n", m_via_pb & 1, m_print_state, m_print_bit);
		switch (m_print_state)
		{
			case READING_CMD: {
				m_print_cmd <<= 1;
				m_print_cmd |= (m_via_pb & 1);
				m_print_bit += 1;
				if (m_print_bit == 8)
				{
					logerror("PRINT_CMD %x!\n", m_print_cmd);
					m_print_bit = 0;
					m_print_state = WRITING_RESULT;
					m_sbsy = true;
					// The status messages have odd parity in the high bit
					// right now we just manually make sure that's true.
					switch (m_print_cmd)
					{
						case SR1:
						case SR2:
						case SR4:
							m_print_result = 1 << 7; // parity
							break;
						case SR5: // paper size
							m_print_result = 1 << 6; // A4
							break;
						case SR0:
						default:
							m_print_result = 1 << 1; // print request
							break;
					}
					m_print_cmd = 0;
				}
				break;
			}
			case WRITING_RESULT: {
				m_print_sc = m_print_result & 0x1;
				m_print_result >>= 1;
				m_print_bit += 1;
				if (m_print_bit == 8)
				{
					logerror("PRINT_RESULT!\n");
					m_print_bit = 0;
					m_print_state = READING_CMD;
					m_sbsy = false;
				}
				break;
			}
		}
	}
}

uint32_t lwriter_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	auto const f = [] (auto x) { return x ? u32(0) : u32(0xffffffff); };
	for (int y = 0; y < FB_HEIGHT; y++) {
		for (int x = 0; x < FB_WIDTH; x += 8) {
			uint32_t *scanline = &bitmap.pix(y, x);
			uint8_t const pixels = m_vram[y * FB_WIDTH/8 + x/8];
			*scanline++ = f(BIT(pixels, 7));
			*scanline++ = f(BIT(pixels, 6));
			*scanline++ = f(BIT(pixels, 5));
			*scanline++ = f(BIT(pixels, 4));
			*scanline++ = f(BIT(pixels, 3));
			*scanline++ = f(BIT(pixels, 2));
			*scanline++ = f(BIT(pixels, 1));
			*scanline++ = f(BIT(pixels, 0));
		}
	}

	return 0;
}

void lwriter_state::lwriter2nt(machine_config &config)
{
	lwriter(config);
	m_via->readpb_handler().set(FUNC(lwriter_state::via_pb_lw2nt_r));
	m_via->writepa_handler().set(FUNC(lwriter_state::via_pa_w));
}

void lwriter_state::lwriter(machine_config &config)
{
	M68000(config, m_maincpu, CPU_CLK);
	m_maincpu->set_addrmap(AS_PROGRAM, &lwriter_state::maincpu_map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(30);
	m_screen->set_size(FB_WIDTH, FB_HEIGHT);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(FUNC(lwriter_state::screen_update));

	SCC8530N(config, m_scc, CPU_CLK);
	m_scc->configure_channels(RXC_CLK, 0, RXC_CLK, 0);
	/* Port A */
	m_scc->out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	m_scc->out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	/* Port B */
	m_scc->out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	m_scc->out_dtrb_callback().set(FUNC(lwriter_state::write_dtr));
	m_scc->out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	/* Interrupt */
	// The "CA1 Latch/Interrupt Control" bit in VIA PCR gets set to "negative
	// active edge" so we invert the SCC interrupt.
	m_scc->out_int_callback().set(m_via, FUNC(via6522_device::write_ca1)).invert();
	//m_scc->out_int_callback().set(FUNC(lwriter_state::scc_int));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, "terminal"));
	rs232a.rxd_handler().set(m_scc, FUNC(scc8530_device::rxa_w));
	rs232a.cts_handler().set(m_scc, FUNC(scc8530_device::ctsa_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, "terminal"));
	rs232b.rxd_handler().set(m_scc, FUNC(scc8530_device::rxb_w));
	rs232b.cts_handler().set(m_scc, FUNC(scc8530_device::ctsb_w));

	R65NC22(config, m_via, CPU_CLK/10); // 68000 E clock presumed
	m_via->readpa_handler().set(FUNC(lwriter_state::via_pa_r));
	m_via->readpb_handler().set(FUNC(lwriter_state::via_pb_r));
	m_via->writepa_handler().set(FUNC(lwriter_state::via_pa_lw_w));
	m_via->writepb_handler().set(FUNC(lwriter_state::via_pb_w));
	m_via->cb1_handler().set(FUNC(lwriter_state::via_cb1_w));
	m_via->ca2_handler().set(FUNC(lwriter_state::via_ca2_w));
	m_via->cb2_handler().set(FUNC(lwriter_state::via_cb2_w));
	m_via->irq_handler().set(FUNC(lwriter_state::via_int_w));
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


#define ROM_LOAD16_BYTE_BIOS(bios,name,offset,length,hash)     ROMX_LOAD(name, offset, length, hash, ROM_SKIP(1) | ROM_BIOS(bios))

ROM_START(lwriter)
	ROM_REGION16_BE( 0x200000, "rom", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS(0, "rev47", "PostScript Version 47.0")
	ROM_LOAD16_BYTE_BIOS(0, "342-0568a.rom", 0x000001, 0x10000, CRC (83341c75) SHA1 (d7c65d09abaaf862fef00ac4df7a094ddedd24c5)) // Label: "342-0568-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"
	ROM_LOAD16_BYTE_BIOS(0, "342-0569a.rom", 0x000000, 0x10000, CRC (47d33a6b) SHA1 (0e79fa9204f9be6539abcdb619a17a4ced912b13)) // Label: "342-0569-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"
	ROM_LOAD16_BYTE_BIOS(0, "342-0570a.rom", 0x020001, 0x10000, CRC (38753dd2) SHA1 (931eb3386fe0fff1de1311b2bc1cee8ee02ed599)) // Label: "342-0570-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"
	ROM_LOAD16_BYTE_BIOS(0, "342-0571a.rom", 0x020000, 0x10000, CRC (08888acd) SHA1 (f771306d8f876e6e4ed14f3c6e5b71dff75cf49e)) // Label: "342-0571-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"
	ROM_LOAD16_BYTE_BIOS(0, "342-0572a.rom", 0x040001, 0x10000, CRC (0a64af91) SHA1 (22cd61ed7c2f64bfd4ddbd7b5cde64311a3db5e6)) // Label: "342-0572-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"
	ROM_LOAD16_BYTE_BIOS(0, "342-0573a.rom", 0x040000, 0x10000, CRC (f8e529fe) SHA1 (8a4511a4c12eb24c731e1de747886aacfa2057d5)) // Label: "342-0573-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"
	ROM_LOAD16_BYTE_BIOS(0, "342-0574a.rom", 0x060001, 0x10000, CRC (bb694699) SHA1 (2e208b30e8d05725f7e8b469974b6357008fbb1d)) // Label: "342-0574-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"
	ROM_LOAD16_BYTE_BIOS(0, "342-0575a.rom", 0x060000, 0x10000, CRC (c21c1d22) SHA1 (9fc6cd059380c11588c182fb8ec6422e5db472e1)) // Label: "342-0575-A // (C) '87 ADOBE SYS // (C) 81 LINOTYPE // POSTSCRIPT"

	ROM_SYSTEM_BIOS(1, "rev2", "PostScript Version 38.0")
	ROM_LOAD16_BYTE_BIOS(1, "342-0081a_l0.bin", 0x000001, 0x10000, CRC (a76c91df) SHA1 (c62ef2ede8ce7ba92ec75b3fcb3ddbb288fe235b)) // Label: "342-0081A-L0 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(1, "342-0082a_h0.bin", 0x000000, 0x10000, CRC (3342d008) SHA1 (bc01749bd9a9bc129a4100ee64e09a428b0619c1)) // Label: "342-0082A-H0 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(1, "342-0083a_l1.bin", 0x020001, 0x10000, CRC (8569fb1e) SHA1 (0e004f649078949d5c70d6b92774e4696f3f3cd4)) // Label: "342-0083A-L1 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(1, "342-0084a_h1.bin", 0x020000, 0x10000, CRC (a4d939bf) SHA1 (1585d5e651349f2857d8934cfda85fc4012c2c91)) // Label: "342-0084A-H1 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(1, "342-0085a_l2.bin", 0x040001, 0x10000, CRC (a77e5efc) SHA1 (73b60da77d433d97ecbe9e28558836da8c1cc259)) // Label: "342-0085A-L2 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(1, "342-0086a_h2.bin", 0x040000, 0x10000, CRC (5cf037a1) SHA1 (1ea7177fa11ecdd02b794144c182de0836eb4110)) // Label: "342-0086A-H3 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(1, "342-0087a_l3.bin", 0x060001, 0x10000, CRC (8186bd91) SHA1 (4e3623efc4926be8d6182b702642bf634ae23f82)) // Label: "342-0087A-L4 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(1, "342-0088a_h3.bin", 0x060000, 0x10000, CRC (ecf425ec) SHA1 (353c957a16edf8d3a685c1c8bfe896e26d4a15ed)) // Label: "342-0088A-H4 // (C) '85 ADOBE SYS // POSTSCRIPT TM"

	ROM_REGION16_BE( 0x1000, "sram", ROMREGION_ERASEFF )
ROM_END

ROM_START(lwriterplus)
	ROM_REGION16_BE( 0x200000, "rom", ROMREGION_ERASEFF )

	ROM_SYSTEM_BIOS(0, "rev47", "PostScript Version 47.0")
	ROM_LOAD16_BYTE_BIOS(0, "342-0089a.l0", 0x000001, 0x10000, CRC (d5dc7d6e) SHA1 (b9a1f807facf6a6de92fae5887044df961d73ab1)) // Label: "342-0089-A+L0 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0090a.h0", 0x000000, 0x10000, CRC (32dc1f96) SHA1 (f3647b11c712979f6c5658a15a3e8647bd4d1a1d)) // Label: "342-0090-A+H0 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0091a.l1", 0x020001, 0x10000, CRC (a24dcb05) SHA1 (9edfb94a1e6723a7580caed629418ee1d2472a84)) // Label: "342-0091-A+L1 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0092a.h1", 0x020000, 0x10000, CRC (8600e85d) SHA1 (332308825f78a768e30eaa36f10f0ac1c5eacc19)) // Label: "342-0092-A+H1 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0093a.l2", 0x040001, 0x10000, CRC (3c8fd0f7) SHA1 (36315be1ed691b24c49471eab0cd93a4242d6e10)) // Label: "342-0093-A+L2 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0094a.h2", 0x040000, 0x10000, CRC (e1a9d862) SHA1 (ffdd96eb70f54c6bb10dfc94f49ce2d916a74ab6)) // Label: "342-0094-A+H2 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0095a.l3", 0x060001, 0x10000, CRC (47f637f3) SHA1 (d04548144f906a8d89b826692812acdcbfbed144)) // Label: "342-0095-A+L3 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0096a.h3", 0x060000, 0x10000, CRC (3213e057) SHA1 (94d2ac1849b48628004877521c882e0f828f97b3)) // Label: "342-0096-A+H3 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0097a.l4", 0x080001, 0x10000, CRC (9ecdc5fc) SHA1 (336ecaaf29c5396c30a11aaa86533f0598cb50b3)) // Label: "342-0097-A+L4 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0098a.h4", 0x080000, 0x10000, CRC (867657e3) SHA1 (0c9c29bac49fdfcd26f22a751be71508add0a25a)) // Label: "342-0098-A+H4 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0099a.l5", 0x0a0001, 0x10000, CRC (820c0f63) SHA1 (0f8d45fc886f996fbcb4103961810b673e9ab7e4)) // Label: "342-0099-A+L5 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0100a.h5", 0x0a0000, 0x10000, CRC (40aeb030) SHA1 (2a34280a6b2ab54d1c82145ec1a8aaac1f57ae15)) // Label: "342-0100-A+H5 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0101a.l6", 0x0c0001, 0x10000, CRC (aed532c4) SHA1 (39d7d3ae1d35d8b4ec33fd8c88a569c607628e2a)) // Label: "342-0101-A+L6 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0102a.h6", 0x0c0000, 0x10000, CRC (653979d1) SHA1 (bf56df6a7eaee2bc8edfbc45f78d1abb19e6807a)) // Label: "342-0102-A+H6 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0103b.l7", 0x0e0001, 0x10000, CRC (dbafb1ed) SHA1 (b9f5b65b04f8f804c473b62c292dc83d14b2ab33)) // Label: "342-0103-B+L7 // (C) '85 ADOBE SYS // POSTSCRIPT TM"
	ROM_LOAD16_BYTE_BIOS(0, "342-0104b.h7", 0x0e0000, 0x10000, CRC (50c72f89) SHA1 (bdbc0e282121f7dc4d701aa12f94296e436b504b)) // Label: "342-0104-B+H7 // (C) '85 ADOBE SYS // POSTSCRIPT TM"

	ROM_SYSTEM_BIOS(1, "rev42", "PostScript Version 42.2")
	ROM_LOAD16_BYTE_BIOS(1, "f0.bin", 0x000001, 0x10000, CRC (d971aeef) SHA1 (5d9a4cebdeab7bc87f6f1f821a95bfb5ad2ea252))
	ROM_LOAD16_BYTE_BIOS(1, "e0.bin", 0x000000, 0x10000, CRC (9df59887) SHA1 (ffb720cd1067cbbc3b2404bb9e77cc1ce768064a))
	ROM_LOAD16_BYTE_BIOS(1, "f1.bin", 0x020001, 0x10000, CRC (5216141e) SHA1 (6c53955cdc121ee5bee042a0582e31872c8707a9))
	ROM_LOAD16_BYTE_BIOS(1, "e1.bin", 0x020000, 0x10000, CRC (660a977b) SHA1 (c1257d87ddee6485dbf49b0c8da11b38947a0932))
	ROM_LOAD16_BYTE_BIOS(1, "f2.bin", 0x040001, 0x10000, CRC (8947a995) SHA1 (d7f4e2c1ce4f66cb55dd09c6fb876ac765ffca28))
	ROM_LOAD16_BYTE_BIOS(1, "e2.bin", 0x040000, 0x10000, CRC (3ae6e11b) SHA1 (a95e29abd808041bc00851a2dde1bb0951eb787d))
	ROM_LOAD16_BYTE_BIOS(1, "f3.bin", 0x060001, 0x10000, CRC (ca23252f) SHA1 (85714dd8716a1971c31a0609b8726a23f9d15cfc))
	ROM_LOAD16_BYTE_BIOS(1, "e3.bin", 0x060000, 0x10000, CRC (ae91d2a2) SHA1 (84b449d984ab539aef13ece4a0093fff041bd5e3))
	ROM_LOAD16_BYTE_BIOS(1, "f4.bin", 0x080001, 0x10000, CRC (5ad31e1b) SHA1 (90310fa158986ae88adec6de1d2a72a2ff161699))
	ROM_LOAD16_BYTE_BIOS(1, "e4.bin", 0x080000, 0x10000, CRC (987d6796) SHA1 (29a916eb76c953ec0b11b68b4b38dedb305d0c54))
	ROM_LOAD16_BYTE_BIOS(1, "f5.bin", 0x0a0001, 0x10000, CRC (c687c0ab) SHA1 (22e757fa46860b9ee4e97883b884084beb0f9f78))
	ROM_LOAD16_BYTE_BIOS(1, "e5.bin", 0x0a0000, 0x10000, CRC (b4ce7883) SHA1 (ef93d2ab821fe30ab78d749584656772fd95c42d))
	ROM_LOAD16_BYTE_BIOS(1, "f6.bin", 0x0c0001, 0x10000, CRC (3f60a380) SHA1 (d18cd39bb253054807c92f9fbbd756b460b7be5b))
	ROM_LOAD16_BYTE_BIOS(1, "e6.bin", 0x0c0000, 0x10000, CRC (0b2e9058) SHA1 (3ac9e02e70ef9bf1c732efe5912eac0a2fb58a35))
	ROM_LOAD16_BYTE_BIOS(1, "f7.bin", 0x0e0001, 0x10000, CRC (82478865) SHA1 (e6d56e04a586a646ef44bc15460572e8f7a4b602))
	ROM_LOAD16_BYTE_BIOS(1, "e7.bin", 0x0e0000, 0x10000, CRC (07a5548e) SHA1 (23b90b5e2dbaf5fa6a78929033e40c8ded919bad))

	ROM_REGION16_BE( 0x1000, "sram", ROMREGION_ERASEFF )
ROM_END

ROM_START(lwriter2nt)
	ROM_REGION16_BE( 0x200000, "rom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE("342-0545.l0", 0x000001, 0x20000, CRC (6431742d) SHA1 (040bd5b84b49b86f2b0fe9ece378bbc7a10a94ec)) // Label: "342-0545-A JAPAN // TC531000CP-F700 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L0
	ROM_LOAD16_BYTE("342-0546.h0", 0x000000, 0x20000, CRC (c592bfb7) SHA1 (b595ae225238f7fabd1566a3133ea6154e082e2d)) // Label: "342-0546-A JAPAN // TC531000CP-F701 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H0
	ROM_LOAD16_BYTE("342-0547.l1", 0x040001, 0x20000, CRC (205a5ea8) SHA1 (205fefbb5c67a07d57cb6184c69648321a34a8fe)) // Label: "342-0547-A JAPAN // TC531000CP-F702 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L1
	ROM_LOAD16_BYTE("342-0548.h1", 0x040000, 0x20000, CRC (f616e1c3) SHA1 (b9e2cd4d07990b2d1936be97b6e89ef21f06b462)) // Label: "342-0548-A JAPAN // TC531000CP-F703 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H1
	ROM_LOAD16_BYTE("342-0549.l2", 0x080001, 0x20000, CRC (0b0b051a) SHA1 (64a80085001570c3f99d9865031715bf49bd7698)) // Label: "342-0549-A JAPAN // TC531000CP-F704 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L2
	ROM_LOAD16_BYTE("342-0550.h2", 0x080000, 0x20000, CRC (82adcf85) SHA1 (e2ab728afdae802c0c67fc25c9ba278b9cb04e31)) // Label: "342-0550-A JAPAN // TC531000CP-F705 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H2
	ROM_LOAD16_BYTE("342-0551.l3", 0x0c0001, 0x20000, CRC (176b3346) SHA1 (eb8dfc7e44f2bc884097e51a47e2f10ee091c9e9)) // Label: "342-0551-A JAPAN // TC531000CP-F706 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @L3
	ROM_LOAD16_BYTE("342-0552.h3", 0x0c0000, 0x20000, CRC (69b175c6) SHA1 (a84c82be1ec7e373bb097ee74b941920a3b091aa)) // Label: "342-0552-A JAPAN // TC531000CP-F707 // (C) 87 APPLE 8940EAI // (C) 83-87 ADOBE V47.0 // (C) 81 LINOTYPE" TC531000 @H3

	ROM_REGION16_BE( 0x1000, "sram", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


/*    YEAR  NAME         PARENT  COMPAT  MACHINE     INPUT    STATE          INIT        COMPANY            FULLNAME             FLAGS */
CONS( 1985, lwriter,     0,      0,      lwriter,    lwriter, lwriter_state, empty_init, "Apple Computer",  "LaserWriter",       MACHINE_IS_SKELETON)
CONS( 1986, lwriterplus, 0,      0,      lwriter,    lwriter, lwriter_state, empty_init, "Apple Computer",  "LaserWriter Plus",  MACHINE_IS_SKELETON)
CONS( 1988, lwriter2nt,  0,      0,      lwriter2nt, lwriter, lwriter_state, empty_init, "Apple Computer",  "LaserWriter II NT", MACHINE_IS_SKELETON)
