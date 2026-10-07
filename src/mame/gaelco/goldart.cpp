// license:BSD-3-Clause
// copyright-holders:David Haywood, Tomás García-Merás Capote (ClawGrip)

/*
  _____________________________________________________________________
  |                    _____________  _______DALLAS____________        |
  |                    |  ROM U6    | |                        |   __  |
  |                    |____________| |KM62256BLG-7L    BATT   |   | | |
  |                        ____       |       DS5002FP         |   |J| |
  |             ____       |U7 |      |________________________|   |P| |
  |             356N       |___|   _________      ____________     |6| |
  |                                74HCT132N|     CXK5814P-35L     |_| |
  |                      _________ _________                 _________ |
  |                      74HCT373N||74HC04B1|   _________    74HCT245N |
  |            ________  _____________          |        |   _________ |
  | __         |74F112N| | ROM U11    |    TPC1020BFN-084C   74HCT245N |
  | | |        _____     |____________|         |        |   _________ |
  | |J|        |XTAL|    _________ _________    |________|   74HCT273E |
  | |P|        |____|    |SN74F32N||74LS257_|                _________ |
  | |1|        ______    _________ _________  _____________  74HCT273E |
  | |_|        | U10 |   |ULN2003A||74LS257_| UM611024AK-20| _________ |
  |  ______    ______    ______________________    ______    74HCT273E |
  |  |FUSE_|   |_JP2_|   |_________JP3_________|   |_JP5_|             |
  |____________________________________________________________________|

 JP1 = Power (9 pins)
 JP2 = Serial DB9 (unused)
 JP3 = Darts board (20 pins)
 JP5 = Video out (6 pins)
 JP6 = Buttons (15 pins)

 XTAL = 32.000 MHz

 U7 = Oki
 U10 = Unpopulated socket for Max 202

 The UM611024AK-20 is a '128K X 8BIT HIGH SPEED CMOS SRAM' so likely the video RAM
 http://www.datasheetcatalog.com/datasheets_pdf/U/T/6/1/UT611024.shtml


 Hardware notes (worked out from the program code):

 The DS5002FP runs in partitioned mode (MCON = 0x78): program in SRAM 0x0000-0x6fff and
 data (variables, settings, bookkeeping) in SRAM 0x7000-0x7fff. The rest of the MOVX
 space is the expanded bus; the code sets RPCTL.EXBS whenever it needs to reach the
 expanded bus at 0x7000-0x7fff (and clears it to access its variables).
 Code at 0x6000-0x6fff is an overlay copied from data ROM bank 6 (the program
 temporarily switches the partition to 0x1000 with MCON = 0x18 to write it), so the
 contents of that area depend on what the machine was doing when the SRAM was dumped.

 Expanded bus:
   read  0000-ffef  data ROM, 64KB bank selected by P1.0-P1.2
   write 0000-fdff  video RAM: 384x288 bitmap, 4bpp + 4 bit palette select per pixel,
                    two pixels per address (high nibble = left pixel).
                    A nibble of 0xf is not written (transparency) and the palette
                    select for the written pixels comes from register fffe.
   write fe00-ffdf  palette: 15 palettes x 16 colours, xRRRRRGGGGGBBBBB, big endian
   fff3       w     window: unknown, always 0x60
   fff4-fff7  w     window: start address / 64 = ((offset & 3) << 8) | data
   fff8       w     window: first byte of each line
   fff9       w     window: number of lines
   fffa       w     window: last byte of each line + 1
   fffb       w     bit 0 = acknowledge vblank (INT0)
                    bit 1 = acknowledge window operation done (INT1)
                    bit 2 = restore window contents
                    bit 3 = save window contents (used for the pop up dialog boxes)
   fffc       r/w   OKI M6295
   fffd       r     dart board matrix (return lines, active low)
   fffd       w     OKI bank for 0x30000-0x3ffff
   fffe       r     buttons, coins and ultrasonic sensor
   fffe       w     palette select (high nibble) for video RAM writes

 Ports:
   P1.0-P1.2  data ROM bank / dart board matrix row select (with P1.3)
   P1.3       dart board matrix row select
   P1.4       selects which half of the 16 dart board return lines is read (2x 74LS257)
   P1.5       unknown output
   P1.6       coin counter (active low)
   P1.7       inertia sensor input (the program writes 0 to the latch, so it always reads 0)
   P3.2       INT0: vblank
   P3.3       INT1: window operation done
   P3.4       Test 1: Initialization menu (end game, clear credits)
   P3.5       Test 2: Test menu

 TODO:
 - the window save / restore probably uses the unused part of the 128KB video RAM, it's
   kept in a separate buffer here. The time it takes is unknown.
 - screen timings are unverified (assumed PAL, 8MHz pixel clock).

*/

#include "emu.h"

#include "cpu/mcs51/ds5002fp.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "dartboard.lh"

#include <algorithm>

#define LOG_REGS   (1U << 1)
#define LOG_WINDOW (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_REGS | LOG_WINDOW)

#include "logmacro.h"

#define LOGREGS(...)   LOGMASKED(LOG_REGS,   __VA_ARGS__)
#define LOGWINDOW(...) LOGMASKED(LOG_WINDOW, __VA_ARGS__)

/*
   Gaelco coin control with voltmeter used on Goldart machines.
   Includes a two-digit 7-segment display, but no other detail is known.
*/
class goldart_coincontrol_device : public device_t
{
public:
	goldart_coincontrol_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DEFINE_DEVICE_TYPE(GOLDART_COINCONTROL, goldart_coincontrol_device, "goldart_ctrl", "Gaelco Goldart Coin Control")

goldart_coincontrol_device::goldart_coincontrol_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GOLDART_COINCONTROL, tag, owner, clock)
{ }

void goldart_coincontrol_device::device_start()
{
}

void goldart_coincontrol_device::device_reset()
{
}

ROM_START(goldart_ctrl)
	ROM_REGION( 0x2000, "mcu", ROMREGION_ERASE00 )
	ROM_LOAD( "m-vg_17919_pic16c54.bin", 0x00000, 0x2000, CRC(9f27564b) SHA1(2a45188cbb6475a466c5813afb0eaabf070d90ec) )
ROM_END

const tiny_rom_entry *goldart_coincontrol_device::device_rom_region() const
{
	return ROM_NAME(goldart_ctrl);
}


namespace {

class goldart_state : public driver_device
{
public:
	goldart_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_oki(*this, "oki"),
		m_databank(*this, "databank"),
		m_okibank(*this, "okibank"),
		m_dart(*this, "DART%u", 0U)
	{ }

	void goldart(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr unsigned SCREEN_WIDTH = 384;
	static constexpr unsigned SCREEN_HEIGHT = 288;
	static constexpr unsigned VRAM_SIZE = 0x20000;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<okim6295_device> m_oki;
	memory_bank_creator m_databank;
	memory_bank_creator m_okibank;
	required_ioport_array<8> m_dart;

	std::unique_ptr<u8[]> m_vram;
	std::unique_ptr<u8[]> m_winbuf;
	u8 m_paletteram[0x1e0];
	u8 m_pal_select = 0;
	u8 m_port1 = 0;
	u8 m_window_unk = 0;
	u16 m_window_addr = 0;
	u8 m_window_left = 0;
	u8 m_window_lines = 0;
	u8 m_window_right = 0;
	emu_timer *m_window_timer = nullptr;

	void port1_w(u8 data);
	void vblank_w(int state);

	void vram_w(offs_t offset, u8 data);
	void palette_w(offs_t offset, u8 data);
	void window_unk_w(u8 data);
	void window_addr_w(offs_t offset, u8 data);
	void window_left_w(u8 data);
	void window_lines_w(u8 data);
	void window_right_w(u8 data);
	void control_w(u8 data);
	u8 dart_r();
	void okibank_w(u8 data);
	void pal_select_w(u8 data);
	void window_copy(bool restore);
	TIMER_CALLBACK_MEMBER(window_done);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_prgmap(address_map &map) ATTR_COLD;
	void main_datamap(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
};


void goldart_state::port1_w(u8 data)
{
	if ((data ^ m_port1) & 0x20)
		LOG("%s: P1.5 = %d\n", machine().describe_context(), BIT(data, 5));

	m_port1 = data;
	m_databank->set_entry(data & 0x07);
	machine().bookkeeping().coin_counter_w(0, BIT(~data, 6));
}

void goldart_state::vblank_w(int state)
{
	// held until acknowledged through fffb
	if (state)
		m_maincpu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
}

u32 goldart_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		u8 const *const src = &m_vram[y * SCREEN_WIDTH];
		u16 *const dst = &bitmap.pix(y);
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			dst[x] = src[x];
	}

	return 0;
}

void goldart_state::vram_w(offs_t offset, u8 data)
{
	// two pixels per address, a nibble of 0xf is not written
	u8 *const dst = &m_vram[offset << 1];

	if ((data & 0xf0) != 0xf0)
		dst[0] = m_pal_select | (data >> 4);

	if ((data & 0x0f) != 0x0f)
		dst[1] = m_pal_select | (data & 0x0f);
}

void goldart_state::palette_w(offs_t offset, u8 data)
{
	m_paletteram[offset] = data;

	u16 const entry = (m_paletteram[offset & ~1] << 8) | m_paletteram[offset | 1];
	m_palette->set_pen_color(offset >> 1, pal5bit(entry >> 10), pal5bit(entry >> 5), pal5bit(entry >> 0));
}

void goldart_state::window_unk_w(u8 data)
{
	LOGREGS("%s: window_unk_w %02x\n", machine().describe_context(), data);
	m_window_unk = data;
}

void goldart_state::window_addr_w(offs_t offset, u8 data)
{
	// the register offset provides the upper bits of the address
	m_window_addr = ((offset & 3) << 8) | data;
	LOGREGS("%s: window_addr_w %03x\n", machine().describe_context(), m_window_addr);
}

void goldart_state::window_left_w(u8 data)
{
	LOGREGS("%s: window_left_w %02x\n", machine().describe_context(), data);
	m_window_left = data;
}

void goldart_state::window_lines_w(u8 data)
{
	LOGREGS("%s: window_lines_w %02x\n", machine().describe_context(), data);
	m_window_lines = data;
}

void goldart_state::window_right_w(u8 data)
{
	LOGREGS("%s: window_right_w %02x\n", machine().describe_context(), data);
	m_window_right = data;
}

void goldart_state::window_copy(bool restore)
{
	offs_t const base = m_window_addr << 6;

	LOGWINDOW("%s: window %s address %05x x %02x-%02x lines %d (unk %02x)\n", machine().describe_context(),
			restore ? "restore" : "save", base, m_window_left, m_window_right, m_window_lines, m_window_unk);

	for (int y = 0; y < m_window_lines; y++)
	{
		for (int x = m_window_left; x < m_window_right; x++)
		{
			offs_t const pos = ((base + y * (SCREEN_WIDTH / 2) + x) & 0xffff) << 1;

			if (restore)
				std::copy_n(&m_winbuf[pos], 2, &m_vram[pos]);
			else
				std::copy_n(&m_vram[pos], 2, &m_winbuf[pos]);
		}
	}
}

TIMER_CALLBACK_MEMBER(goldart_state::window_done)
{
	m_maincpu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
}

void goldart_state::control_w(u8 data)
{
	if (BIT(data, 0))
		m_maincpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);

	if (BIT(data, 1))
		m_maincpu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);

	if (data & 0x0c)
	{
		window_copy(BIT(data, 2));

		// completion is signalled with INT1, the copy speed is a guess (one address per pixel clock)
		unsigned const width = (m_window_right > m_window_left) ? (m_window_right - m_window_left) : 0;
		m_window_timer->adjust(attotime::from_ticks((width * m_window_lines) + 1, 32_MHz_XTAL / 4));
	}

	if (data & 0xf0)
		LOG("%s: control_w unknown bits %02x\n", machine().describe_context(), data);
}

u8 goldart_state::dart_r()
{
	// P1.0-P1.3 drive the 4 row lines (through the ULN2003A), P1.4 selects which half of the 16 return lines is read
	u8 data = 0xff;

	for (int row = 0; row < 4; row++)
	{
		if (BIT(m_port1, row))
			data &= m_dart[(row << 1) | BIT(m_port1, 4)]->read();
	}

	return data;
}

void goldart_state::okibank_w(u8 data)
{
	m_okibank->set_entry(data & 0x07);
}

void goldart_state::pal_select_w(u8 data)
{
	m_pal_select = data & 0xf0;
}


void goldart_state::main_prgmap(address_map &map)
{
	map(0x00000, 0x07fff).readonly().share("sram");
}

void goldart_state::main_datamap(address_map &map)
{
	// expanded bus
	map(0x00000, 0x0ffff).bankr(m_databank);
	map(0x00000, 0x0fdff).w(FUNC(goldart_state::vram_w));
	map(0x0fe00, 0x0ffdf).w(FUNC(goldart_state::palette_w));
	map(0x0fff0, 0x0ffff).unmaprw();
	map(0x0fff3, 0x0fff3).w(FUNC(goldart_state::window_unk_w));
	map(0x0fff4, 0x0fff7).w(FUNC(goldart_state::window_addr_w));
	map(0x0fff8, 0x0fff8).w(FUNC(goldart_state::window_left_w));
	map(0x0fff9, 0x0fff9).w(FUNC(goldart_state::window_lines_w));
	map(0x0fffa, 0x0fffa).w(FUNC(goldart_state::window_right_w));
	map(0x0fffb, 0x0fffb).w(FUNC(goldart_state::control_w));
	map(0x0fffc, 0x0fffc).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x0fffd, 0x0fffd).rw(FUNC(goldart_state::dart_r), FUNC(goldart_state::okibank_w));
	map(0x0fffe, 0x0fffe).portr("IN0").w(FUNC(goldart_state::pal_select_w));

	// byte-wide bus
	map(0x10000, 0x17fff).ram().share("sram");
}

void goldart_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr(m_okibank);
}


static INPUT_PORTS_START( goldart )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Down")  // ABAJO
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Up")    // ARRIBA
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // strobe for an alternate coin mechanism mode which reads a coin code from bits 4-6, never enabled by this program
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ultrasonic Sensor") // SENSOR ULTRASONIDOS

	PORT_START("P1")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED ) // outputs, must read back as 1 (port pins read as latch & input)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Inertia Sensor") // SENSOR DE INERCIA

	PORT_START("P3")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED ) // serial port, INT0, INT1
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Test 1 (Initialization Menu)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Test 2 (Test Menu)")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) // expanded bus /WR, /RD

	// dart board matrix, DARTn = row line n / 2 (P1.0-P1.3), return lines 0-7 (n even) or 8-15 (n odd)
	// the dartboard layout finds the targets by these names
	PORT_START("DART0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 16")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 16")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 7")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 7")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 19")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 19")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 3")

	PORT_START("DART1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 17")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 17")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 16")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 7")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 19")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 17")

	PORT_START("DART2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 15")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 15")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 10")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 6")

	PORT_START("DART3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 13")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 13")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double Bull")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 15")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 10")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 13")

	PORT_START("DART4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 18")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 18")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 20")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 20")

	PORT_START("DART5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single Bull")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 18")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 20")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 5")

	PORT_START("DART6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 12")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 12")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 9")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 14")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 14")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 11")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 11")

	PORT_START("DART7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Double 8")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Single 8")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 12")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 9")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 14")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 11")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Dart Triple 8")
INPUT_PORTS_END


void goldart_state::machine_start()
{
	m_vram = make_unique_clear<u8[]>(VRAM_SIZE);
	m_winbuf = make_unique_clear<u8[]>(VRAM_SIZE);
	std::fill(std::begin(m_paletteram), std::end(m_paletteram), 0);

	m_databank->configure_entries(0, 8, memregion("data")->base(), 0x10000);
	m_okibank->configure_entries(0, 8, memregion("oki")->base(), 0x10000);
	m_okibank->set_entry(3);

	m_window_timer = timer_alloc(FUNC(goldart_state::window_done), this);

	save_pointer(NAME(m_vram), VRAM_SIZE);
	save_pointer(NAME(m_winbuf), VRAM_SIZE);
	save_item(NAME(m_paletteram));
	save_item(NAME(m_pal_select));
	save_item(NAME(m_port1));
	save_item(NAME(m_window_unk));
	save_item(NAME(m_window_addr));
	save_item(NAME(m_window_left));
	save_item(NAME(m_window_lines));
	save_item(NAME(m_window_right));
}

void goldart_state::machine_reset()
{
	m_port1 = 0xff;
	m_databank->set_entry(7);
	m_window_timer->adjust(attotime::never);
}

void goldart_state::goldart(machine_config &config)
{
	// basic machine hardware
	ds5002fp_device &maincpu(DS5002FP(config, m_maincpu, 32_MHz_XTAL / 2));
	maincpu.set_addrmap(AS_PROGRAM, &goldart_state::main_prgmap);
	maincpu.set_addrmap(AS_DATA, &goldart_state::main_datamap);
	maincpu.port_in_cb<1>().set_ioport("P1");
	maincpu.port_out_cb<1>().set(FUNC(goldart_state::port1_w));
	maincpu.port_in_cb<3>().set_ioport("P3");

	NVRAM(config, "sram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(32_MHz_XTAL / 4, 512, 0, SCREEN_WIDTH, 312, 0, SCREEN_HEIGHT); // unverified, PAL timings assumed
	screen.set_screen_update(FUNC(goldart_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(goldart_state::vblank_w));

	PALETTE(config, m_palette, palette_device::BLACK, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH); // clock frequency & pin 7 not verified
	m_oki->set_addrmap(0, &goldart_state::oki_map);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);

	// External coin control PCB
	GOLDART_COINCONTROL(config, "coin_ctrl");
}


ROM_START( goldart )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "u11_e_262.u11", 0x00000, 0x80000, CRC(325551e0) SHA1(4fe8d71d448de3f8a9b5751bad6e90d2e556cb8f) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u6_e.u6", 0x00000, 0x80000, CRC(dd9dc689) SHA1(11871ba815372c06f8b1367d2897c37953db7bdd) )
ROM_END

ROM_START( goldartfr )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "francia_dianas_794c_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(0d9c7d2c) SHA1(616652d5d07454293d00807a94c072f059528ed7) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dianas_so_fra_27c040.u6", 0x00000, 0x80000, CRC(727ce7b7) SHA1(533290aa97e33124a7697d72a9a108f0ab503ac5) )
ROM_END

ROM_START( goldartgr )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "alema_diana_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(f0119b2b) SHA1(f60c77e9352fdb8e6c00fd347d6af634da6f5ae3) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "dianas_son_aleman_15-2-95_27c4001.u6", 0x00000, 0x80000, CRC(fd494229) SHA1(41c2f9f185987510863116a95dc4f7cd6b6bb17c) )
ROM_END

ROM_START( goldartpt )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "p-262.u11", 0x00000, 0x80000, CRC(fa6537b0) SHA1(a4c3ac8f5139b18f0688beaa374c75a6f0aabcd2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "p-262.u6", 0x00000, 0x80000, CRC(4177e78b) SHA1(1099568b97a08c33a7da1bf46fc106f25af15e90) )
ROM_END

ROM_START( goldartuk )
	ROM_REGION( 0x8000, "sram", 0 ) // DS5002FP code
	ROM_LOAD( "ds5002fp_sram.bin", 0x00000, 0x8000, CRC(cd2bf151) SHA1(6f601cef86493fc2db181c93b17949b982149b0e) )

	ROM_REGION( 0x100, "maincpu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x80000, "data", 0 )
	ROM_LOAD( "g.b_diana_5017_26-2-96_27c040.u11", 0x00000, 0x80000, CRC(efd8bfc1) SHA1(d4d01a5d6d618ed2ecabc959a88eb14b1bbf6241) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "diana_so_uk_257a_26-7_27c040.u6", 0x00000, 0x80000, CRC(a93afb8b) SHA1(c7a5fc4e74a0743ffc729ec3214f318141a82cc0) )
ROM_END


} // Anonymous namespace

//    YEAR, NAME,       PARENT,  MACHINE,  INPUT,   CLASS,         INIT,       ROT,  COMPANY,             FULLNAME,                             FLAGS,                 LAYOUT

GAMEL( 1994, goldart,   0,       goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (Spain)",                    MACHINE_SUPPORTS_SAVE, layout_dartboard )
GAMEL( 1994, goldartfr, goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Jeutel",   "Goldart (France, Covielsa license)", MACHINE_SUPPORTS_SAVE, layout_dartboard )
GAMEL( 1994, goldartgr, goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (Germany)",                  MACHINE_SUPPORTS_SAVE, layout_dartboard )
GAMEL( 1994, goldartpt, goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (Portugal)",                 MACHINE_SUPPORTS_SAVE, layout_dartboard )
GAMEL( 1994, goldartuk, goldart, goldart,  goldart, goldart_state, empty_init, ROT0, "Gaelco / Covielsa", "Goldart (United Kingdom)",           MACHINE_SUPPORTS_SAVE, layout_dartboard )
