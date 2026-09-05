// license:BSD-3-Clause
// copyright-holders:

/*

MAIN BOARD REV 1.1

+---------------------------------------------------------------------------------+
|                 MOTHER BOARD REV  1.1                    TAB AUSTRIA            |
+--+                                                                              +--+
|  |                                                                               --|
|C |                                                         +-+                   --|
|O |                                                         |F|                   --|
|N |                                                         +-+                  +--+
|N |                                                                              |
|E |                                                                              +--+
|C |                                                     +-+      +-+              --|
|T |                                                     | | +-+  | | +-+          --|
|O |                                                     |A| |B|  |C| |H|          --|
|R |                                                     | | | |  | | | |          --|
+--+                                      +----------+   +-+ +-+  +-+ +-+          --|
|                                         |  ACTEL   |   +-+      +-+              --|
|                    +-+ +-+ +---+        |  A1020B  |   | | +-+  | | +-+          --|
|                    |D| |E| | X |        |PL84C 9637|   |A| |B|  |C| |H|          --|
|                    | | | | |TAL|        |          |   | | | |  | | | |          --|
|                    +-+ +-+ +---+        +----------+   +-+ +-+  +-+ +-+          --|
|                                                        +-+      +-+              --|
|     +-+  +------+   +----+ +----+ +------+             | | +-+  | | +-+          --|
+--+  |G|  |      |   |HY62| |HY62| |DALLAS|             |A| |B|  |A| |I|          --|
|C |  | |  |      |   |64AL| |64AL| |      |             | | | |  | | | |          --|
|O |  +-+  |      |   |P-10| |P-10| |DS1220|             +-+ +-+  +-+ +-+          --|
|N |       |      |   |    | |    | |Y-100 |             +-+      +-+              --|
|N |       |      |   +----+ +----+ +------+      +----+ | | +-+  | | +-+          --|
|E |       |MC6800|                               |EMPT| |A| |B|  |C| |H|          --|
|C |       |0P12  |   +----+ +----+ +----+ +----+ |Y SO| | | | |  | | | |          --|
|T |       |      |   |    | |    | |    | |    | |CKET| +-+ +-+  +-+ +-+          --|
|O |       |      |   |ROM1| |ROM2| |ROM3| |ROM4| |    | +-+      +-+              --|
|R |       |      |   |    | |    | |    | |    | |    | | | +-+  | | +-+          --|
|  |       |      |   |    | |    | |    | |    | |27C4| |A| |B|  |C| |H|          --|
+--+       |      |   |    | |    | |    | |    | |096 | | | | |  | | | |         +--+
|          +------+   +----+ +----+ +----+ +----+ +----+ +-+ +-+  +-+ +-+         |
+---------------------------------------------------------------------------------+

A = PC74HCT245P
B = 898-3-R330
C = PC74HCT563P
D = 898-3-R47 or 898-3-R330
E = PC74HCT4040P
F = TL7705ACP
G = AD557JN
H = TD62083AP or ULN2803A
I = 4116R-001-103

XTAL = 24.0000 MHz

Most boards only have EPROMS (27C040) on ROM-SOCKETS 1 & 3.
Some boards use 4x 27c020 on all four sockets.
The 40pin socket next to the rom-sockets is empty on all known boards. Pinout would fit for an 27c4096.


This system uses one mainboard, called MOTHERBOARD (by TAB AUSTRIA), and will need a Subboard
(called AUFSATZboard by TAB AUSTRIA) for video!

Known "Aufsatzboards" are:
- LOWRES (Rev1 and Rev2)
- MEDRES (Rev2)
- HIGHRES (Rev2)

HIGHRES has a VGA-Port and should have a resolution of 640x480.


-------------


LOWRES-Aufsatzboard Rev1

+--+-------------------------------+-------------------+-------------------------------+--+
|  |    CONNECTOR TO MAIN BOARD    |                   |    CONNECTOR TO MAIN BOARD    |  |
|  +-------------------------------+                   +-------------------------------+  |
|                                            +-+ +-+                                      |
|                                        +-+ | | | |                                      |
|                                        |H| |G| |G|      +------+ +-+ +-+ +-+            |
|  +-------------+   +-------------+     | | | | | |      |      | | | |C| |A|  LOWRES    |
|  |             |   |             |     +-+ +-+ +-+      |INMOS | |F| | | | |  AUFSATZ-  |
|  |             |   |             |                      |9222-G| | | +-+ +-+  BOARD     |
|  | HD63484CP8  |   |  HD63487CP  |         +-+ +-+      |      | | | +-+ +-+            |
|  |             |   |             |     +-+ | | | |      |IMSG17| +-+ |D| |B|  REV1      |
|  |             |   |             |     |H| |G| |G|      |6P-50 | +-+ | | | |            |
|  +-------------+   +-------------+     | | | | | |      +------+ |E| +-+ +-+            |
|                                        +-+ +-+ +-+               | |                    |
|                                                                  +-+            TAB     |
|                                                                                 AUSTRIA |
+-----------------------------------------------------------------------------------------+


A = PC74HCT04P
B = PC74HCT86P
C = PC74HCT73P
D = PC74HCT40103P
E = PC74HCT14P
F = PALCE "NLR 1.0"
G = HM514258AP-10
H = 898-3-R47


-------------


LOWRES-Aufsatzboard Rev2

+--+-------------------------------+-------------------+-------------------------------+--+
|  |    CONNECTOR TO MAIN BOARD    |                   |    CONNECTOR TO MAIN BOARD    |  |
|  +-------------------------------+                   +-------------------------------+  |
|    +---------+  +---------+          +-------------+                                    |
|    |    F    |  |    F    |          |             |                  LOWRES            |
|    +---------+  +---------+          |             |    +-------+     AUFSATZBOARD REV2 |
|    +-----E-----+   +-------------+   | HD63484CP98 |    |   C   |                       |
|                    |             |   |             |    +-------+  +---------------+    |
|    +-----E-----+   |             |   |             |    +-------+  | INMOS 9244-G  |    |
|                    |  HD63487CP  |   +-------------+    |   B   |  |  IMSG176P-66  |    |
|    +-----E-----+   |             |   +--------+         +-------+  +---------------+    |
|                    |             |   |   D    |         +-------+                       |
|    +-----E-----+   +-------------+   +--------+         |   A   |                       |
|                                      +------------+     +-------+                       |
|                                      |PALCE NLR1.0|                             TAB     |
|                                      +------------+                             AUSTRIA |
+-----------------------------------------------------------------------------------------+


A = PC74HCT40103P
B = 74HCT73N
C = 74HCT86N
D = 74HCT14N
E = KM44C258CZ-10
F = MDP1603 470G


-------------


MEDRES-Aufsatzboard Rev2
+--+-------------------------------+-------------------+-------------------------------+--+
|  |    CONNECTOR TO MAIN BOARD    |                   |    CONNECTOR TO MAIN BOARD    |  |
|  +-------------------------------+                   +-------------------------------+  |
|                                                                                         |
|                                     +-+   +-+   +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+         |
|     MEDRES-AUFSATZ   Rev.2          |B|   |B|   |A| |A| |A| |A| |A| |A| |A| |A|         |
|     ENTR.: 414.400.530              | |   | |   | | | | | | | | | | | | | | | |         |
|                                     +-+   +-+   +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+         |
|                                                                                         |
|       +-----+   +-------------+                                                         |
|       |     |   |             |                                                         |
|       |ADV47|   | HD63484CP8  |          +-------------+      +-------------+           |
|  +-+  |6KN66|   |             |          |    ACTEL    |      |    ACTEL    |           |
|  |B|  |     |   |             |          |             |   |  |             |           |
|  | |  |     |   |             |          |             |   C  |             |           |
|  +-+  |     |   +-------------+          |             |   |  |             |           |
|       +-----+                            |   MIC V1    |      |   VIC V1    |           |
| TAB AUSTRIA                              +-------------+      +-------------+           |
+-----------------------------------------------------------------------------------------+


A = TC514256AP-70
B = MDP1603 470G
C = two Solderpads => connect these pads and you will transform this board to LOWRES.


-------------


HIRES-Aufsatzboard Rev2

+----------------------------------------------+
|                                          TAB |
+--+                                    AUSTRIA|
|  |  +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+          |
|C |  | | | | | | | | | | | | | | | |  +-+   H |
|O |  |A| |A| |A| |A| |A| |A| |A| |A|  |B|   I |
|N |  | | | | | | | | | | | | | | | |  | |   R |
|N |  +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+  +-+   E |
|E |  +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+        S |
|C |  | | | | | | | | | | | | | | | |  +-+   | |
|T |  |A| |A| |A| |A| |A| |A| |A| |A|  |B|   A |
|O |  | | | | | | | | | | | | | | | |  | |   U |
|R |  +-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+  +-+   F |
+--+   +----------+   +----------+           S |
|      |          |   |          |           A |
|      |HD63486CP6|   |HD63486CP6|   REV.2   T |
|      |4         |   |4         |           Z |
|      |          |   |          | ETNR.:     +--+
|      +----------+   +----------+ 414.400.510| V|
|      +----------+   +----------+            | G|
+--+   |          |   |          |   +-+      | A|
|C |   |HD63484CP8|   |HD63485CP6|   |P|      +--+
|O |   |          |   |4         |   |A|       |
|N |   |          |   |          |   |L|       |
|N |   +----------+   +----------+   +-+       |
|E |                                           |
|C |     +----+                                |
|T |     |    | +-+                            |
|O |     |ADV4| |B|          +---+ +-+ +-+     |
|R |     |76KN| | |          | X | |C| |D|     |
|  |     |66  | +-+          |TAL| | | | |     |
+--+     +----+              +---+ +-+ +-+     |
|                                              |
+----------------------------------------------+

A = TC514256AP-70
B = MDP1603 470G
C = SN74HCT04N
D = SN74HCT125N

XTAL = 26.00000MHz


TODO:
- low res games seem to be slightly cut off on the right side of the screen
- luckyres bonus screen is cut off at the top
- verify inputs for all sets (the ones in the drivers were verified for royalpkr170l)
- hopper
- lamps
- NVRAM doesn't seem to reload correctly. Also machines take an incredibly long time to initialize,
  maybe related?
- code optionally supports an M68681 but it isn't to be found anywhere on the PCBs. What gives?
- reconstruct DS2401 / DS1220Y content for all regions (regions are determined by them, apart for a
  few hardcoded cases (Kärnten)). Only one region per game has been added thus far
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/clock.h"
#include "machine/ds2401.h"
//#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/hd63484.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class royalpkr_state : public driver_device
{
public:
	royalpkr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		//m_duart(*this, "duart"),
		m_id(*this, "id"),
		m_dac_clock(*this, "dacclock"),
		m_nvram(*this, "nvram")
	{ }

	void hires(machine_config &config) ATTR_COLD;
	void medres(machine_config &config) ATTR_COLD;
	void lowres(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	//required_device<mc68681_device> m_duart;
	required_device<ds2401_device> m_id;
	required_device<clock_device> m_dac_clock;

	required_shared_ptr<uint16_t> m_nvram;

	uint16_t m_clock_rate = 0;
	uint8_t m_irq_enable[2] {};

	void lowres_acrtc_display_pixels(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data, int screen_n);

	void irq_enable_w(offs_t offset, uint16_t data);
	INTERRUPT_GEN_MEMBER(periodic_irq);
	void vblank_w(int state);
	uint16_t irq_ack(offs_t offset);

	void clock_start_w(uint16_t data);

	void lamps_w(uint16_t data);
	void hopper_meters_w(uint16_t data);

	void program_map(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;
	void hd63484_lowres_map(address_map &map) ATTR_COLD;
	void hd63484_medres_map(address_map &map) ATTR_COLD;
	void hd63484_hires_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};


void royalpkr_state::lowres_acrtc_display_pixels(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data, int screen_n)
{
	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = (((screen_n + 1) & 3) << 4) | (data & 0x0f);
}

void royalpkr_state::machine_start()
{
	save_item(NAME(m_clock_rate));
	save_item(NAME(m_irq_enable));
}

void royalpkr_state::irq_enable_w(offs_t offset, uint16_t data)
{
	m_irq_enable[offset] = BIT(data, 0);

	if (!m_irq_enable[offset])
		m_maincpu->set_input_line(offset ? M68K_IRQ_3 : M68K_IRQ_4, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(royalpkr_state::periodic_irq)
{
	if (m_irq_enable[1])
		device.execute().set_input_line(M68K_IRQ_3, ASSERT_LINE);
}

void royalpkr_state::vblank_w(int state)
{
	if (state && m_irq_enable[0])
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
}

uint16_t royalpkr_state::irq_ack(offs_t offset)
{
	m_maincpu->set_input_line(offset + 1, CLEAR_LINE);

	//if (offset + 1 == 2)
		//return m_duart->get_irq_vector();
	//else
	return m68000_device::autovector(offset + 1);
}

void royalpkr_state::clock_start_w(uint16_t data)
{
	if (data != 0 && m_clock_rate != 0)
		m_dac_clock->set_clock_scale(1.0 / m_clock_rate);
	else
		m_dac_clock->set_clock_scale(0.0);
}

void royalpkr_state::lamps_w(uint16_t data)
{
	/*
	bit 0: Hopper out lamp
	bit 1: Button 1 lamp
	bit 2: Button 2 lamp
	bit 3: Button 3 lamp
	bit 4: Button 4 lamp
	bit 5: Button 5 lamp
	bit 6: Button 6 lamp
	bit 7: Button 7 lamp
	bit 8: Button 8 lamp
	bit 9: Button 9 lamp
	bit 10: Start lamp
	bit 11: Top lamp 1 - Jackpot
	bit 12: Top lamp 2 - Call attendant
	bit 13: Top lamp 3
	bit 14: Coin lamp
	bit 15: unused?
	*/
}

void royalpkr_state::hopper_meters_w(uint16_t data)
{
	/*
	bit 0: Meter A - Gesamt IN
	bit 1: Meter B - Gesamt OUT
	bit 2: Meter C - Extern IN
	bit 3: Meter D - Extern OUT
	bit 4: Meter E - Master IN
	bit 5: Meter F - Master OUT
	bit 6: Banknote enable
	bit 7: Hopper motor
	bit 8: Coin enable A
	bit 9: Coin enable B
	bit 10: Coin enable C
	bit 11: Coin enable D
	bit 12: Coin enable E
	bit 13: Sorter hopper-cash
	bit 14: Handle inhibit
	bit 15: Jackpot net
	*/
}


void royalpkr_state::program_map(address_map &map)
{
	map(0x000000, 0x2fffff).rom();
	map(0x300000, 0x303fff).ram();
	map(0x400000, 0x400fff).ram().lw16(NAME([this] (offs_t offset, uint16_t data) { m_nvram[offset] = data | 0xff00; })).share("nvram");
	map(0x500000, 0x500003).rw("acrtc", FUNC(hd63484_device::read16), FUNC(hd63484_device::write16));
	map(0x500020, 0x500020).lr8(NAME([] () -> uint8_t { return 0x01; }));   // board ID, bit 0 = Aufsatzboard rev
	map(0x500021, 0x500021).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x500023, 0x500023).rw("ramdac", FUNC(ramdac_device::pal_r), FUNC(ramdac_device::pal_w));
	map(0x500025, 0x500025).rw("ramdac", FUNC(ramdac_device::mask_r), FUNC(ramdac_device::mask_w));
	map(0x500027, 0x500027).w("ramdac", FUNC(ramdac_device::index_r_w));
	//map(0x500040, 0x50005f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x500060, 0x500061).lr16(NAME([this] () -> uint16_t { return m_id->read(); })).lw16(NAME([this] (uint16_t data) { m_id->write(BIT(data, 0)); }));
	map(0x600000, 0x600001).portr("IN0").w(FUNC(royalpkr_state::lamps_w));
	map(0x600002, 0x600003).portr("IN1").w(FUNC(royalpkr_state::hopper_meters_w));
	map(0x600004, 0x600005).portr("IN2");
	map(0x600004, 0x600004).w("dac", FUNC(dac_byte_interface::write));
	map(0x700000, 0x700001).w(FUNC(royalpkr_state::clock_start_w));
	map(0x700002, 0x700003).lw16(NAME([this] (uint16_t data) { m_clock_rate = data; }));
	map(0x700004, 0x700007).w(FUNC(royalpkr_state::irq_enable_w));
}

void royalpkr_state::cpu_space_map(address_map &map)
{
	map(0xfffff2, 0xffffff).r(FUNC(royalpkr_state::irq_ack));
}

void royalpkr_state::hd63484_lowres_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram().mirror(0xc0000);
}

void royalpkr_state::hd63484_medres_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram().mirror(0x80000);
}

void royalpkr_state::hd63484_hires_map(address_map &map)
{
	map(0x00000, 0xfffff).ram();
}

void royalpkr_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


static INPUT_PORTS_START( royalpkr )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // hopper out
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) // Button 1
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) // Button 2
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) // Button 3
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) // Button 4
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Black") // Button 5
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Bet / Take Score")// Button 6
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON7 ) // Button 7
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) // Button 8
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Red")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start / Double Up") // Button 10: Start
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN6 ) // banknote A
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN7 ) // banknote B
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN8 ) // banknote C
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN9 ) // banknote D
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // meters connected

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Hopper connected
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Hopper coin out
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Hopper full
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // External device error
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Coin to cashbox IN
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN11 ) // Remote IN B
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN10 ) // Remote IN A
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin IN A
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 ) // Coin IN B
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_COIN3 ) // Coin IN C
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_COIN4 ) // Coin IN D
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_COIN5 ) // Coin IN E
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Coin impulse type

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Machine Door") // Machine Door
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Switch 9 (unused?)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Switch 10(unused?)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Handle IN A
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Handle IN A
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused?
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Self test
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_DOOR ) PORT_NAME("Cashier Door") // Cashier Door
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4) // Push down (?)
	PORT_SERVICE( 0x0400, IP_ACTIVE_LOW ) // Testschalter
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // Aufsteller
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE ) // Wirt + Kellner 0
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Kellner 1
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Kellner 2
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Kellner 3
INPUT_PORTS_END


void royalpkr_state::hires(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &royalpkr_state::program_map);
	m_maincpu->set_periodic_int(FUNC(royalpkr_state::periodic_irq), attotime::from_hz(200));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &royalpkr_state::cpu_space_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1); // DS1220Y

	//MC68681(config, m_duart, 3'686'400);
	//m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_2, ASSERT_LINE);

	DS2401(config, m_id, 0);

	CLOCK(config, m_dac_clock, 1'500'000); // base rate derived from program code
	m_dac_clock->signal_handler().set_inputline(m_maincpu, M68K_IRQ_5, ASSERT_LINE);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(384, 280); // dummy values, reconfigured by the ACRTC
	screen.set_visarea(0, 384-1, 0, 280-1); // dummy values, reconfigured by the ACRTC
	screen.set_screen_update("acrtc", FUNC(hd63484_device::update_screen));
	screen.set_palette("palette");
	screen.screen_vblank().set(FUNC(royalpkr_state::vblank_w));

	hd63484_device &acrtc(HD63484(config, "acrtc"));
	acrtc.set_addrmap(0, &royalpkr_state::hd63484_hires_map);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", "palette"));
	ramdac.set_addrmap(0, &royalpkr_state::ramdac_map);
	ramdac.set_split_read(1);

	PALETTE(config, "palette").set_entries(0x100);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AD557(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void royalpkr_state::medres(machine_config &config)
{
	hires(config);

	subdevice<hd63484_device>("acrtc")->set_addrmap(0, &royalpkr_state::hd63484_medres_map);
}

void royalpkr_state::lowres(machine_config &config)
{
	hires(config);

	subdevice<hd63484_device>("acrtc")->set_addrmap(0, &royalpkr_state::hd63484_lowres_map);
	subdevice<hd63484_device>("acrtc")->set_display_callback(FUNC(royalpkr_state::lowres_acrtc_display_pixels));
}


ROM_START( royalpkr ) // Royal Poker V 2.10 Apr 06 1999 09:44:36
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop210.3", 0x000000, 0x80000, CRC(389158e4) SHA1(4675f2a3771e6126c713b15c6263429059a6e485) )
	ROM_LOAD16_BYTE( "rop210.1", 0x000001, 0x80000, CRC(a31d9f00) SHA1(e73ced49fe3907338ef6283cc82d759677168c41) )
	// ROM 2, 4 and 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "royalpkr.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "royalpkr.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END

ROM_START( royalpkr200l ) // Royal Poker V 2.00 Jul 21 1998 14:44:51
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop200.3", 0x000000, 0x80000, CRC(2f7106e5) SHA1(0e5d0815a0b68464a03555e5638dc5ca828c44e4) )
	ROM_LOAD16_BYTE( "rop200.1", 0x000001, 0x80000, CRC(6e006227) SHA1(f733c499986aa077021e509212b3a5573c5daaef) )
	// ROM 2, 4 and 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "royalpkr.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "royalpkr.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END

ROM_START( royalpkr185l ) // Royal Poker V 1.85 Oct 29 1996 12:20:07
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop185.3", 0x000000, 0x80000, CRC(e0c312b4) SHA1(57c64c82f723067b7b2f9bf3fdaf5aedeb4f9dc3) )
	ROM_LOAD16_BYTE( "rop185.1", 0x000001, 0x80000, CRC(fbe13fa8) SHA1(7c19b6b4d9a9935b6feb70b6261bafc6d9afb59f) )
	// ROM 2, 4 and 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "royalpkr.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "royalpkr.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END

ROM_START( royalpkr170l ) // Royal Poker V 1.70 May 31 1995 19:21:55
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop170.3", 0x000000, 0x40000, CRC(8d15263a) SHA1(e2e1f2383c1b13eb072d188f7e27b6f4ff6e8eaf) )
	ROM_LOAD16_BYTE( "rop170.1", 0x000001, 0x40000, CRC(5288d73c) SHA1(3cda4f881f261164741e51504f3dc97bd0a2395f) )
	ROM_LOAD16_BYTE( "rop170.4", 0x100000, 0x40000, CRC(86328aae) SHA1(476b589eb103bd758806920c41bc3f95dfae6d30) )
	ROM_LOAD16_BYTE( "rop170.2", 0x100001, 0x40000, CRC(04ea0f49) SHA1(999169d086a3501debf6f9409c2d725fe1970ebc) )
	// ROM 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "royalpkr.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "royalpkr.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END

ROM_START( royalpkr170h ) // Royal Poker V 1.70 May 31 1995 19:42:12
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "roh170.3", 0x000000, 0x80000, CRC(33b1c6a2) SHA1(7b50ce9e76bfaec0db8a2807206d8f76be0e0049) )
	ROM_LOAD16_BYTE( "roh170.1", 0x000001, 0x80000, CRC(3cfd6d9a) SHA1(35df8d7b8950b7ca354c62753d8b0275de4295f2) )
	ROM_LOAD16_BYTE( "roh170.4", 0x100000, 0x80000, CRC(87b619c5) SHA1(063d951c955251eba09df67021f5ba80fc1f3a10) )
	ROM_LOAD16_BYTE( "roh170.2", 0x100001, 0x80000, CRC(a48d1b05) SHA1(6a41d771dc97e0f460bb95067af2bfc566f08a42) )
	// ROM 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "royalpkr.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "royalpkr.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END

ROM_START( royalpkr154l ) // Royal Poker V 1.54 Nov 28 1994 14:22:54
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop154.3", 0x000000, 0x40000, CRC(1dd373c0) SHA1(ad16d362e036ea154a31b06bb72af7691c92015c) )
	ROM_LOAD16_BYTE( "rop154.1", 0x000001, 0x40000, CRC(b09d565a) SHA1(4c20b0fc1054e1b86f0b50eab6523d981ea8821f) )
	ROM_LOAD16_BYTE( "rop154.4", 0x100000, 0x40000, CRC(7721ba86) SHA1(8f31ef05924581064c05c491ac2487f2458a4909) )
	ROM_LOAD16_BYTE( "rop154.2", 0x100001, 0x40000, CRC(81e1bfdc) SHA1(e6b0a7432c84875ec43c606616a67ba504eaf49f) )
	// ROM 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "royalpkr.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "royalpkr.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END

ROM_START( royalpkr152l ) // Royal Poker V 1.52 Feb 08 1994 11:01:50
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "rop152.3", 0x000000, 0x40000, CRC(2870b6aa) SHA1(a8a55a62fab81f7138da92f14a4ac908b23ca4c8) )
	ROM_LOAD16_BYTE( "rop152.1", 0x000001, 0x40000, CRC(e79ba23d) SHA1(2f893ef7dc618d4229ddbbbe7411c04cff33c315) )
	ROM_LOAD16_BYTE( "rop152.4", 0x100000, 0x40000, CRC(7721ba86) SHA1(8f31ef05924581064c05c491ac2487f2458a4909) ) // same as V1.54
	ROM_LOAD16_BYTE( "rop152.2", 0x100001, 0x40000, CRC(81e1bfdc) SHA1(e6b0a7432c84875ec43c606616a67ba504eaf49f) ) // same as V1.54
	// ROM 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "royalpkr.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "royalpkr.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END


ROM_START( csmarino ) // Club San Marino V 1.10 Italy, Aug 10 1998 14:46:38
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "csm110.3", 0x000000, 0x80000, CRC(ce73b1d8) SHA1(d56b3e317f696cf03afe74d89da9cdb2f34861e9) )
	ROM_LOAD16_BYTE( "csm110.1", 0x000001, 0x80000, CRC(63dd956f) SHA1(0f692033bc116348db907653de8d1144516e976b) )
	// ROM 2, 4 and 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "csmarino.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "csmarino.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END

ROM_START( csmarino020l ) // Club San Marino V 0.20 Italy, Aug 07 1996 15:14:24
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "csm110.3", 0x000000, 0x80000, CRC(54ac214c) SHA1(4c2a4b437e4e04292204fcd47d4113a56c6d092a) )
	ROM_LOAD16_BYTE( "csm110.1", 0x000001, 0x80000, CRC(7739f667) SHA1(d6d137fe8b2a6d25a1ad71321c14c37d08171b77) )
	// ROM 2, 4 and 5 not populated

	// config: 54fe4500
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "csmarino.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "csmarino.nv", 0x0000, 0x1000, CRC(41b64de4) SHA1(08d03db5becbbe36164c7309da1ee56354fb6545) ) // with Konfigurationsnummer
ROM_END


ROM_START( jollypk2 ) // Double Joker K V 1.20 Mai 19 2000 12:00
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "dojk120.3", 0x000000, 0x80000, CRC(4cbe5dfb) SHA1(c81b12dfe584f59f7f52ccd7e67c6e4429ffd2b0) )
	ROM_LOAD16_BYTE( "dojk120.1", 0x000001, 0x80000, CRC(4eea28bf) SHA1(d7d0f7eba151ee62cd6bdaa9f1cce26d0d856cd7) )
	// ROM 2, 4 and 5 not populated

	// config: 29739118
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "jollypk2.bin", 0x00, 0x08, CRC(b19795e7) SHA1(5b4822fe60182a8bb0b19accc812bb48d68534e0) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000031d34341
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "jollypk2.nv", 0x0000, 0x1000, CRC(c67b7b3e) SHA1(a091619626ce16059e83d062c31946e25e635687) ) // with Konfigurationsnummer
ROM_END

ROM_START( jollypk2116l ) // Double Joker K V 1.16 Mai 19 2000 12:00 (weirdly same time-stamp as V 1.20)
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "dojk116.3", 0x000000, 0x80000, CRC(e02a9dbc) SHA1(a9affcb3ac0c83f6784e72c8e483b8a8940426e7) )
	ROM_LOAD16_BYTE( "dojk116.1", 0x000001, 0x80000, CRC(a52677d0) SHA1(e370ecca07ab9dbfbecec78154c8acc6813069bd) )
	// ROM 2, 4 and 5 not populated

	// config: 29739118
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "jollypk2.bin", 0x00, 0x08, CRC(b19795e7) SHA1(5b4822fe60182a8bb0b19accc812bb48d68534e0) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000031d34341
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "jollypk2.nv", 0x0000, 0x1000, CRC(c67b7b3e) SHA1(a091619626ce16059e83d062c31946e25e635687) ) // with Konfigurationsnummer
ROM_END


ROM_START( eurodraw ) // Euro Draw V 1.10, Jan 08 2002 13:39:29
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "edl110.3", 0x000000, 0x80000, CRC(f07303d1) SHA1(f1c9fb1072f8873b716efae27f69757e77c1e0be) )
	ROM_LOAD16_BYTE( "edl110.1", 0x000001, 0x80000, CRC(218c746b) SHA1(7790e75aba2665911f5d0bbd02911dc8ad150274) )
	// ROM 2, 4 and 5 not populated

	// config: 472c9d00
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "eurodraw.id", 0x00, 0x08, CRC(9a77afe9) SHA1(85b1f083d70787395974f7bf2dd2e003a9cc9d7c) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000027c8c16a
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "eurodraw.nv", 0x0000, 0x1000, CRC(d1878411) SHA1(a77d19419d39b0836e7b678852a651733610997c) ) // with Konfigurationsnummer
ROM_END


ROM_START( luckyfun ) // Lucky Fun V 2.20 Jun 02 1997 11:21:59
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fch220.3", 0x000000, 0x80000, CRC(fd555a29) SHA1(8970ef1f2618232e4bb206680292b63eb3190cbe) )
	ROM_LOAD16_BYTE( "fch220.1", 0x000001, 0x80000, CRC(c3087402) SHA1(a71c95ed1c5ba5b10d151db887cacf058ea7803a) )
	ROM_LOAD16_BYTE( "fch220.4", 0x100000, 0x80000, CRC(00a8eacd) SHA1(5cf5897c7d084337df120700f484ff2f02d85b9e) )
	ROM_LOAD16_BYTE( "fch220.2", 0x100001, 0x80000, CRC(15c909c3) SHA1(7e68bfe57f841625df2ad94d99376362259c33f7) )
	// ROM 5 not populated

	// config: 9cdfcb00
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "luckyfun.bin", 0x00, 0x08, CRC(d470f3e9) SHA1(bce77642a8f02cc838256dc6184f07186a5d2e17) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000016deb7d3
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "luckyfun.nv", 0x0000, 0x1000, CRC(458d1a03) SHA1(0dd5ca8af1f146507af59405f4ca88926023f838) ) // with Konfigurationsnummer
ROM_END


ROM_START( tripledr ) // Triple Draw V 1.09 Mar 28 1997 15:14:43
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tdh109.3", 0x000000, 0x80000, CRC(9885b79a) SHA1(29bf442a75ea14cd88e60c876bd71e189e331966) )
	ROM_LOAD16_BYTE( "tdh109.1", 0x000001, 0x80000, CRC(462a835f) SHA1(100f82bd5c7fe2b727fd0365b99e3fa096c7665e) )
	// ROM 2, 4 and 5 not populated

	// config: 748ddd10
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "tripledr.id", 0x00, 0x08, CRC(ce29e6dd) SHA1(f2c296f47c259f565dc407d527f9773d16da6d5a) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 00000108cbf18
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "tripledr.nv", 0x0000, 0x1000, CRC(feff6a35) SHA1(48e7951be2a124cf36e90c2235b06655d236e2f9) ) // with Konfigurationsnummer
ROM_END

ROM_START( tripledr106l ) // Triple Draw V 1.06 Nov 02 1995 10:31:36
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "tdl106.3", 0x000000, 0x80000, CRC(2304bb02) SHA1(29b6fb7b4c937bb68617917c058c2cf86dc13b0e) )
	ROM_LOAD16_BYTE( "tdl106.1", 0x000001, 0x80000, CRC(7407b5e6) SHA1(8821ee662b1fc5623431d07ee2261d55e9ddf886) )
	// ROM 2, 4 and 5 not populated

	// config: 748ddd10
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "tripledr.id", 0x00, 0x08, CRC(ce29e6dd) SHA1(f2c296f47c259f565dc407d527f9773d16da6d5a) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 00000108CBF18
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "tripledr.nv", 0x0000, 0x1000, CRC(feff6a35) SHA1(48e7951be2a124cf36e90c2235b06655d236e2f9) ) // with Konfigurationsnummer
ROM_END

ROM_START( gmachine ) // The Game Machine V 1.21 Mar 22 2002 10:08:44
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gm121.3", 0x000000, 0x80000, CRC(39051c4d) SHA1(1bb81b81c5ab708d29b8e8a1b7e4c48697262086) )
	ROM_LOAD16_BYTE( "gm121.1", 0x000001, 0x80000, CRC(ff8f0456) SHA1(78c57155a8957f7340f99219e9ad11c7f65bd271) )
	// ROM 2, 4 and 5 not populated

	// config: cbf39a1c
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "gmachine.id", 0x00, 0x08, CRC(daf5025f) SHA1(5b213d27b39e5e05848cf7ab95b093b13216764e) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000016deda0d
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "gmachine.nv", 0x0000, 0x1000, CRC(72d1c3de) SHA1(ab8bc078d94910a906b89d15aa39fc305dfa0154) ) // with Konfigurationsnummer
ROM_END

ROM_START( gmachine102m ) // The Game Machine Kärnten V 1.02 Sep 30 1999 12:00
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gmk102.3", 0x000000, 0x80000, CRC(3cde0223) SHA1(0cce22526c83dceb98d693bbe2fab01446626d7a) BAD_DUMP ) // corrupted vector area
	ROM_LOAD16_BYTE( "gmk102.1", 0x000001, 0x80000, CRC(639c0197) SHA1(0d7b06a07c33568086b61103d21fa2ccf6afb824) )
	// ROM 2, 4 and 5 not populated

	// config: cbf39a1c
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "gmachine.id", 0x00, 0x08, CRC(daf5025f) SHA1(5b213d27b39e5e05848cf7ab95b093b13216764e) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000016deda0d
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "gmachine.nv", 0x0000, 0x1000, CRC(72d1c3de) SHA1(ab8bc078d94910a906b89d15aa39fc305dfa0154) ) // with Konfigurationsnummer
ROM_END

ROM_START( gmachine010m ) // The Game Machine V 0.10 Jan 28 1997 15:07:58
	ROM_REGION( 0x300000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gm010.3", 0x000000, 0x80000, CRC(d0ce5b8a) SHA1(a1153f5e5c98601cb70227ff8883a7a6ff19e5a5) )
	ROM_LOAD16_BYTE( "gm010.1", 0x000001, 0x80000, CRC(854ecf02) SHA1(ad40a72d67f824485e2b080e0af0fff3d848289a) )
	// ROM 2, 4 and 5 not populated

	// config: cbf39a1c
	ROM_REGION( 0x08, "id", 0 )
	ROM_LOAD( "gmachine.id", 0x00, 0x08, CRC(daf5025f) SHA1(5b213d27b39e5e05848cf7ab95b093b13216764e) BAD_DUMP ) // AI - reversed, not a real dump

	// serial: 0000016deda0d
	ROM_REGION( 0x1000, "nvram", 0 )
	ROM_LOAD( "gmachine.nv", 0x0000, 0x1000, CRC(72d1c3de) SHA1(ab8bc078d94910a906b89d15aa39fc305dfa0154) ) // with Konfigurationsnummer
ROM_END

} // anonymous namespace


GAME( 1999, royalpkr,     0,        lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Royal Poker (V2.10, low resolution)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, royalpkr200l, royalpkr, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Royal Poker (V2.00, low resolution)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1996, royalpkr185l, royalpkr, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Royal Poker (V1.85, low resolution)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1995, royalpkr170l, royalpkr, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Royal Poker (V1.70, low resolution)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1995, royalpkr170h, royalpkr, hires,  royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Royal Poker (V1.70, high resolution)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1994, royalpkr154l, royalpkr, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Royal Poker (V1.54, low resolution)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1994, royalpkr152l, royalpkr, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Royal Poker (V1.52, low resolution)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1998, csmarino,     0,        lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Club San Marino (V1.10 Italy, low resolution)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1996, csmarino020l, csmarino, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Club San Marino (V0.20 Italy, low resolution)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 2000, jollypk2,     0,        lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Jolly Poker II (V1.20 Kaernten, low resolution)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 2000, jollypk2116l, jollypk2, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Jolly Poker II (V1.16 Kaernten, low resolution)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 2002, eurodraw,     0,        lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Euro Draw (V1.10, low resolution)",                    MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1997, luckyfun,     0,        hires,  royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Lucky Fun (V2.20 Schweiz, high resolution)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 1997, tripledr,     0,        hires,  royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Triple Draw (V1.09, high resolution)",                 MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1995, tripledr106l, tripledr, lowres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "Triple Draw (V1.06, low resolution)",                  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

GAME( 2002, gmachine,     0,        medres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "The Game Machine (V1.21, medium resolution)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, gmachine102m, gmachine, medres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "The Game Machine (V1.02 Kaernten, medium resolution)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
GAME( 1997, gmachine010m, gmachine, medres, royalpkr, royalpkr_state, empty_init, ROT0, "TAB Austria", "The Game Machine (V0.10, medium resolution)",          MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )
