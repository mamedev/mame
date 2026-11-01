// license:BSD-3-Clause
// copyright-holders:buffi
/***************************************************************************

    Puri Puri Station (Konami, 1996) - business card / photo sticker machine.

    Konami game code 657, program build date Oct 22 1996.

    A photo business card vending machine: the user picks a card design,
    has their face taken by a camera, types a name, address or title and
    a phone number, and a sheet of six colour cards is printed.

    Hardware info from the operator's manual:
    - Camera: Hitachi VK-C255 CCD
    - Printer: Alps Electric AM-1000 "arcade printer" connected to the
      parallel port.

    CPU board GQ657 PWB(A)302134A:
    - MC68EC020FG25 main CPU
    - MC68EC000FN8 sound CPU
    - 50.000 MHz oscillator
    - 657 A01/A02: HN27C4096 program EPROMs
    - 657 B03/B04: 16Mbit mask ROMs
    - 657 A05: 16Mbit sample ROM
    - 657 A06: 27C240 EPROM
    - 2x MB8464 RAM
    - YMZ280B-F with a 16.9344 MHz crystal
    - NPC SM5875 DAC
    - Konami 058232 output filter
    - Konami 056879 (I/O)
    - MACH111 (002799)
    - 2x PALCE16V8 (002800, 002801)
    - Two 8-way DIP switch blocks

    Video board GQ657 PWB(B)302135B:
    - 2x Konami 001642 "GAMA" (the two video chips; konami/djmain.cpp notes the
      same chip, also marked KS10101, as sprites + reset/clock source)
    - 32.000 MHz oscillator
    - 657 A07/A08: 16Mbit tile ROMs
    - 16x CY7C109 (128Kx8) and 8x CY7C199 (32Kx8) SRAM
    - Bt812 video decoder (NTSC/PAL capture for the camera)
    - TL16C550 UART with an ADM232L RS-232 driver on a 6-pin connector
    - PALCE16V8 (002400)

    Memory map (main CPU, from the program):
      000000-0fffff ROM
      100000-17ffff work RAM
      340000        watchdog kick (bit 7 toggled) + IRQ enables (bits 0, 4, 5)
      340004        DIP switches (16-bit, active low): DIP SW 1 on bits 0-7,
                    DIP SW 2 on bits 8-15, switch 1 = bit 0
      340008        inputs (16-bit, active low), joystick on bits 8-11
      360000        status byte from the sound CPU
      380000        output latch on D31-24 (the OUTPUT CHECK screen names them):
                    bit 24 coin counter, 25 paper out (print) counter (both
                    pulsed 8 frames on / 8 off), 26 OK lamp, 27 Cancel lamp,
                    28 coin blocker, 29 error lamp, 30 side light
      3a0000        second output latch: the low byte shows the self test
                    stage / error code (diagnostic LEDs?), bits 8-10 are set
                    together by the video code
      3c0000-3c000f parallel printer port with the PC register layout (data,
                    status with BUSY/PE/nERROR, control with STROBE/AUTOFD/
                    INIT/SELIN/IRQ/direction), see printer_r. Also used to
                    read the captured frame back. Mocked here
      3e0000        command word to the sound CPU
      400000-7fffff DATA ROMs, b03 on D31-16 and b04 on D15-0
      a00000-afffff "PRINT RAM" (16-bit on D31-16; the print code reads
                    RGB555 pixels from a19000 in rows of 0x800 bytes)
      c00000-cfffff "CCD RAM 1" (16-bit on D31-16)
      d00000-dfffff "CCD RAM 2" (16-bit on D31-16)
      e00000-e3ffff video chip A
      e40000-e7ffff video chip B
      e80000        video mixer / camera control (level 6 handler state machine):
                    bit 28 camera video on, bit 29 set from the first capture on
      f00000-f4001f ? 16-bit registers written at reset and once per frame

    Sound CPU:
      000000-07ffff ROM
      080000-083fff RAM
      100000        status byte to the main CPU
      180000        interrupt enable / acknowledge latch (level 6 is the
                    command interrupt; the handler only acknowledges it, the
                    main loop polls the command word)
      200000        ? (write only): 0x490f at reset, 0x4d3f once initialised,
                    then 0x4d00 | low byte of each command taken
      400000-400003 YMZ280B
      500000        command word from the main CPU

    The sound program also has level 3 and 4 handlers (level 4 checks the
    command word too); their sources are unknown and not hooked up.

    The main CPU's level 4 interrupt is the frame interrupt.

    The printer interface at 3c0000 carries both directions of image data:
    the captured camera frame is read back from it 8KB at a time (main loop
    messages 4-6, into video chip B's memory window at e43800) and the three
    colour passes of a print are sent through it (message 1). The interface
    is mocked just far enough for the handshakes to complete: frame reads
    return a colour-bar test pattern and print data is discarded.

    The camera video itself is mixed in behind the graphics by the register
    at e80000 while bit 28 is set (the photo screen and the attract demo of
    it); the driver shows a test pattern there.

    The operator menu is on the Test button.

    TODO:
    - which registers select the visible tilemap pages, scrolling (e03070-76)
    - picture written into chip B's memory window is not used;
      a test pattern stands in for the camera (Bt812) video
    - real screen timings, the registers at e03030-e0303a look like a CRTC
    - some sort of printer output
    - the DIP SW 2 picture settings (brightness, colour level) only select
      camera register values and have no effect on the stand-in picture

***************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "machine/gen_latch.h"
#include "sound/ymz280b.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "multibyte.h"

#include <algorithm>


namespace {

class ppstation_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::CAMERA | feature::PRINTER; }

	ppstation_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_ymz(*this, "ymz"),
		m_soundlatch(*this, "soundlatch"),
		m_soundstatus(*this, "soundstatus"),
		m_vidram(*this, "vidram_%u", 0U),
		m_colorram(*this, "colorram_%u", 0U),
		m_gfxrom(*this, { "gfx_a", "gfx_b" }),
		m_inputs(*this, "IN%u", 0U),
		m_ok_lamp(*this, "ok_lamp"),
		m_cancel_lamp(*this, "cancel_lamp"),
		m_error_lamp(*this, "error_lamp"),
		m_side_light(*this, "side_light")
	{ }

	void ppstation(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static constexpr u16 PATTERN_PEN = 2048; // pens of the stand-in camera picture

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_w(int state);

	void irqctrl_w(offs_t offset, u16 data, u16 mem_mask);
	void outputs_w(u8 data);
	u32 mixer_r();
	void mixer_w(offs_t offset, u32 data, u32 mem_mask);
	template <int Chip> void colorram_w(offs_t offset, u32 data, u32 mem_mask);
	u16 vidreg(offs_t address) const { return m_vidregs[BIT(address, 18)][(address & 0xfe) >> 1]; } // by CPU address, either chip
	template <int Chip> u16 vidregs_r(offs_t offset);
	template <int Chip> void vidregs_w(offs_t offset, u16 data, u16 mem_mask);
	template <int Chip> u32 gfxrom_window_r(offs_t offset);

	u8 printer_r(offs_t offset);
	void printer_w(offs_t offset, u8 data);

	void draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, u32 base, int rows, int palbase);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<m68ec020_device> m_maincpu;
	required_device<m68000_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<ymz280b_device> m_ymz;
	required_device<generic_latch_16_device> m_soundlatch; // command word, main -> sound
	required_device<generic_latch_8_device> m_soundstatus; // status byte, sound -> main
	required_shared_ptr_array<u32, 2> m_vidram;
	required_shared_ptr_array<u32, 2> m_colorram;
	required_memory_region_array<2> m_gfxrom;
	required_ioport_array<2> m_inputs;
	output_finder<> m_ok_lamp;
	output_finder<> m_cancel_lamp;
	output_finder<> m_error_lamp;
	output_finder<> m_side_light;

	u16 m_irqctrl = 0;
	u16 m_vidregs[2][0x80] = { };

	// mocked printer interface at 3c0000
	bool m_printer_busy = false;
	u32 m_capture_count = 0;

	u32 m_mixer = 0; // e80000
};


void ppstation_state::machine_start()
{
	save_item(NAME(m_irqctrl));
	save_item(NAME(m_vidregs));
	save_item(NAME(m_printer_busy));
	save_item(NAME(m_capture_count));
	save_item(NAME(m_mixer));

	// mock for the camera: colour bars with a grey ramp underneath
	static constexpr rgb_t BARS[8] = {
		rgb_t(0xc0, 0xc0, 0xc0), rgb_t(0xc0, 0xc0, 0x00), rgb_t(0x00, 0xc0, 0xc0), rgb_t(0x00, 0xc0, 0x00),
		rgb_t(0xc0, 0x00, 0xc0), rgb_t(0xc0, 0x00, 0x00), rgb_t(0x00, 0x00, 0xc0), rgb_t(0x10, 0x10, 0x10) };
	for (int i = 0; i < 8; i++)
	{
		m_palette->set_pen_color(PATTERN_PEN + i, BARS[i]);
		m_palette->set_pen_color(PATTERN_PEN + 8 + i, rgb_t(i * 36, i * 36, i * 36));
	}
}

void ppstation_state::machine_reset()
{
	m_irqctrl = 0;
	m_maincpu->set_input_line(4, CLEAR_LINE);
	m_maincpu->set_input_line(5, CLEAR_LINE);
	m_maincpu->set_input_line(6, CLEAR_LINE);
	m_printer_busy = false;
	m_capture_count = 0;
	m_mixer = 0;
}


/***************************************************************************
    Video
***************************************************************************/

template <int Chip>
void ppstation_state::colorram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_colorram[Chip][offset]);
	u32 const v = m_colorram[Chip][offset];
	m_palette->set_pen_color(Chip * 1024 + offset, pal5bit(v >> 16), pal5bit(v >> 8), pal5bit(v));
}

template <int Chip>
u16 ppstation_state::vidregs_r(offs_t offset)
{
	return m_vidregs[Chip][offset & 0x7f];
}

template <int Chip>
void ppstation_state::vidregs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vidregs[Chip][offset & 0x7f]);
}

// the self test selects one of 1024 2KB banks with e03028 / e43028 and reads the tile ROM back here
template <int Chip>
u32 ppstation_state::gfxrom_window_r(offs_t offset)
{
	u8 const *const rom = m_gfxrom[Chip]->base();
	u32 const base = (((vidreg(0xe03028 + (Chip * 0x40000)) & 0x3ff) * 0x800) + (offset * 4)) & (m_gfxrom[Chip]->bytes() - 1);
	return get_u32be(&rom[base]);
}

void ppstation_state::draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int chip, u32 base, int rows, int palbase)
{
	gfx_element *const gfx = m_gfxdecode->gfx(chip);
	u32 const *const vram = &m_vidram[chip][base / 4];

	// The monitor shows the camera picture wherever chip A's colour RAM entry has its flag
	// bit set, like a video keyer: pen 0 of every palette is black with the flag, and the
	// photo windows are simply left undrawn. The black text panels use a plain black pen
	// and stay opaque.
	u32 masks[32];
	for (int pal = 0; pal < 32; pal++)
	{
		masks[pal] = 0;
		for (int pen = 0; pen < 16; pen++)
		{
			if (BIT(m_colorram[chip][((palbase + pal) * 16) + pen], 23))
				masks[pal] |= 1 << pen;
		}
	}

	for (int ty = 0; ty < rows; ty++)
	{
		for (int tx = 0; tx < 64; tx++)
		{
			u32 const entry = vram[(ty * 64) + tx];
			u32 const code = entry & 0xffff;
			u32 const attr = entry >> 16;
			u32 const color = palbase + (attr & 0x1f);
			bool const flipx = BIT(attr, 5);
			bool const flipy = BIT(attr, 6);

			gfx->transmask(bitmap, cliprect, code, color, flipx, flipy, tx * 8, ty * 8, masks[attr & 0x1f]);
		}
	}
}

/*
    Sprites live in video chip B's object RAM (e41000-e417ff, 128 entries of
    16 bytes, latched by writing 1 to register e43024 once e43026 reads 0).
    The program sorts them by priority and fills the entries as follows:

      +0  16-bit sprite code (in 16x16 cells, four 8x8 tiles each)
      +2  bit 7 enable, bit 2 flip X, bit 3 flip Y, bits 0-1 size:
          0 = 16x16, 1 = 32x32, 2 = 64x64, 3 = 128x128 pixels
      +3  draw order, lower values are in front
      +4  Y centre, +6 X centre
      +8  X zoom, +a Y zoom: 0x40 is 1:1, smaller values enlarge (the
          program scales the pieces' X offsets with the value it stores at
          +8 and their Y offsets with the one at +a; as in konami/djmain.cpp)
      +c  colour (palette in the low 6 bits)

    Larger sprites use the usual Konami interleaved cell order. The list is
    terminated by entries whose byte +2 is 0 and byte +3 is 0xff.
*/
void ppstation_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static constexpr u8 XOFFSET[8] = { 0, 1, 4, 5, 16, 17, 20, 21 };
	static constexpr u8 YOFFSET[8] = { 0, 2, 8, 10, 32, 34, 40, 42 };

	gfx_element *const gfx = m_gfxdecode->gfx(1);
	u32 const *const objram = &m_vidram[1][0x1000 / 4];

	// collect the enabled entries and draw them back to front
	struct entry { u8 order; int index; };
	entry list[128];
	int count = 0;
	for (int i = 0; i < 128; i++)
	{
		u32 const w0 = objram[i * 4];
		u8 const flags = (w0 >> 8) & 0xff;
		if (!BIT(flags, 7))
			continue;
		list[count].order = w0 & 0xff;
		list[count].index = i;
		count++;
	}
	std::stable_sort(list, list + count, [] (entry const &a, entry const &b) { return a.order > b.order; });

	for (int n = 0; n < count; n++)
	{
		u32 const *const e = &objram[list[n].index * 4];
		u16 const code = e[0] >> 16;
		u8 const flags = (e[0] >> 8) & 0xff;
		s16 const ypos = e[1] >> 16;
		s16 const xpos = e[1] & 0xffff;
		u16 const zoomx = e[2] >> 16;
		u16 const zoomy = e[2] & 0xffff;
		u32 const color = (e[3] >> 16) & 0x3f;
		int const cells = 1 << (flags & 3);
		bool const flipx = BIT(flags, 2);
		bool const flipy = BIT(flags, 3);

		// zoom values are 6.? fixed point steps: 0x40 is 1:1
		u32 const scalex = zoomx ? (0x40 << 16) / zoomx : 0x10000;
		u32 const scaley = zoomy ? (0x40 << 16) / zoomy : 0x10000;
		int const width = (cells * 16 * scalex) >> 16;
		int const height = (cells * 16 * scaley) >> 16;
		int const left = xpos - width / 2;
		int const top = ypos - height / 2;

		for (int cy = 0; cy < cells; cy++)
		{
			for (int cx = 0; cx < cells; cx++)
			{
				u32 const cell = (code + XOFFSET[cx] + YOFFSET[cy]) & 0xffff;
				int const dcx = flipx ? (cells - 1 - cx) : cx;
				int const dcy = flipy ? (cells - 1 - cy) : cy;
				for (int ty = 0; ty < 2; ty++)
				{
					for (int tx = 0; tx < 2; tx++)
					{
						u32 const tile = (cell * 4) + (ty * 2) + tx;
						int const dtx = flipx ? (1 - tx) : tx;
						int const dty = flipy ? (1 - ty) : ty;
						int const sx = left + ((((dcx * 16) + (dtx * 8)) * scalex) >> 16);
						int const sy = top + ((((dcy * 16) + (dty * 8)) * scaley) >> 16);
						gfx->zoom_transpen(bitmap, cliprect, tile, color, flipx, flipy, sx, sy, scalex, scaley, 0);
					}
				}
			}
		}
	}
}

// pen of the stand-in camera picture at a given position
inline u16 test_pattern_pen(int x, int y, int width, int height, int base)
{
	if (y >= height - height / 6)
		return base + 8 + std::min(7, (x * 8) / width);
	return base + std::min(7, (x * 8) / width);
}

u32 ppstation_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// The tilemap RAM is addressed linearly with 64 entries per row. The program draws its
	// "layer 2" (panels, frames, keyboards) at 0x8000 (64x128 tiles) and its "layer 1" (text,
	// the title logo) at 0x10000 (64x256 tiles) on top of it; chip B's "layer 0" at 0x10000
	// is cleared but never drawn to. How the registers select the pages is not understood.
	//
	// The camera picture is at the back, visible wherever both layers are undrawn (the
	// photo windows); the sprites go in front of everything. Bit 28 of the mixer register
	// switches the camera video in (a test pattern here). Bit 29 stays set from the first
	// capture on, through the attract mode, so it is probably not a display enable.
	if (BIT(m_mixer, 28))
	{
		int const width = screen.visible_area().width();
		int const height = screen.visible_area().height();
		for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
		{
			u16 *const dst = &bitmap.pix(y);
			for (int x = cliprect.left(); x <= cliprect.right(); x++)
				dst[x] = test_pattern_pen(x, y, width, height, PATTERN_PEN);
		}
	}
	else
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
	}
	// On the card screens the program loads a second set of 32 palettes into colour RAM
	// entries 512-1023 and sets bit 1 of e03050: the text layer then takes its colours
	// from that set (the customer's name is white in the first set, black in the second).
	draw_layer(bitmap, cliprect, 0, 0x08000, 48, 0);
	draw_layer(bitmap, cliprect, 0, 0x10000, 48, BIT(vidreg(0xe03050), 1) ? 32 : 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}

void ppstation_state::vblank_w(int state)
{
	if (!state)
		return;

	// level 4 is the frame interrupt. The sources of levels 5 and 6 are unknown
	// (level 6 drives a state machine around the e80000 registers, probably the
	// camera); both are raised once per frame here when enabled.
	if (BIT(m_irqctrl, 4))
		m_maincpu->set_input_line(4, ASSERT_LINE);
	if (BIT(m_irqctrl, 5))
		m_maincpu->set_input_line(5, ASSERT_LINE);
	if (BIT(m_irqctrl, 0))
		m_maincpu->set_input_line(6, ASSERT_LINE);
}


/***************************************************************************
    Main CPU
***************************************************************************/

void ppstation_state::irqctrl_w(offs_t offset, u16 data, u16 mem_mask)
{
	// bit 7 is toggled continuously (watchdog kick)
	// bits 4, 5 and 0 are cleared and set again by the level 4, 5 and 6 handlers (enable / acknowledge)
	COMBINE_DATA(&m_irqctrl);
	if (!BIT(m_irqctrl, 4))
		m_maincpu->set_input_line(4, CLEAR_LINE);
	if (!BIT(m_irqctrl, 5))
		m_maincpu->set_input_line(5, CLEAR_LINE);
	if (!BIT(m_irqctrl, 0))
		m_maincpu->set_input_line(6, CLEAR_LINE);
}

// output latch at 380000 (D31-24), bit assignment from the OUTPUT CHECK screen
void ppstation_state::outputs_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // coin counter
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1)); // paper out (print) counter
	m_ok_lamp = BIT(data, 2);
	m_cancel_lamp = BIT(data, 3);
	machine().bookkeeping().coin_lockout_w(0, !BIT(data, 4)); // coin blocker: the game keeps it set while it accepts coins
	m_error_lamp = BIT(data, 5);
	m_side_light = BIT(data, 6);
}

// video mixer / camera control at e80000, see screen_update
u32 ppstation_state::mixer_r()
{
	return m_mixer;
}

void ppstation_state::mixer_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_mixer);
}

/*
    Printer interface at 3c0000 (byte registers on D31-24, PC parallel port
    register layout). This is a mock that satisfies the program's handshakes;
    the real device is an Alps AM-1000 Micro Dry colour printer.

      3c0000  data (read/write)
      3c0004  status: bit 7 busy / data not ready, bit 5 error (paper out?),
              bit 3 ready. The program drops into an error screen unless
              bit 3 is set and bit 5 is clear.
      3c0008  control: 0x38 requests a byte (busy clears, the program reads
              3c0000 and then waits for busy to rise again before writing 0x39),
              0x05/0x04/0x05/0x15 strobes a byte written to 3c0000, 0x11 is
              written once before a frame is read back.
      3c000c  secondary status, read by the level 6 handler (bit 4 tested)
*/
u8 ppstation_state::printer_r(offs_t offset)
{
	switch (offset)
	{
	case 0: // data: the captured frame comes back here, 16-bit pixels in rows of 1024
	{
		u32 const pixel = m_capture_count >> 1;
		int const x = pixel & 0x3ff;
		int const y = (pixel >> 10) & 0x3ff;
		rgb_t const c = m_palette->pen_color(test_pattern_pen(x, y, 1024, 1024, PATTERN_PEN));
		u16 const rgb565 = ((c.r() >> 3) << 11) | ((c.g() >> 2) << 5) | (c.b() >> 3);
		u8 const data = BIT(m_capture_count, 0) ? (rgb565 & 0xff) : (rgb565 >> 8);
		if (!machine().side_effects_disabled())
		{
			m_printer_busy = true;
			m_capture_count++;
		}
		return data;
	}
	case 1: // status
		return (m_printer_busy ? 0x80 : 0x00) | 0x08;
	case 3: // secondary status
		return 0x00;
	}
	return 0;
}

void ppstation_state::printer_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: // print data, discarded
		break;
	case 2: // control
		if (data == 0x38)
			m_printer_busy = false;
		else if (data == 0x11) // written before a frame is read back
			m_capture_count = 0;
		break;
	}
}

void ppstation_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom().region("maincpu", 0);
	map(0x100000, 0x17ffff).ram();
	map(0x340000, 0x340001).w(FUNC(ppstation_state::irqctrl_w));
	map(0x340004, 0x340007).lr32(NAME([this] () -> u32 { return u32(m_inputs[1]->read()) << 16; })); // DIP switches
	map(0x340008, 0x34000b).lr32(NAME([this] () -> u32 { return u32(m_inputs[0]->read()) << 16; }));
	map(0x360000, 0x360003).umask32(0xff00'0000).r(m_soundstatus, FUNC(generic_latch_8_device::read)); // status byte from the sound CPU
	map(0x380000, 0x380003).umask32(0xff00'0000).w(FUNC(ppstation_state::outputs_w)); // lamps and counters
	map(0x3a0000, 0x3a0003).nopw(); // self test stage / error code on the low byte, bits 8-10 set by the video code
	map(0x3c0000, 0x3c000f).umask32(0xff00'0000).rw(FUNC(ppstation_state::printer_r), FUNC(ppstation_state::printer_w));
	map(0x3e0000, 0x3e0003).umask32(0xffff'0000).w(m_soundlatch, FUNC(generic_latch_16_device::write)); // command word to the sound CPU, polled by its main loop
	map(0x400000, 0x7fffff).rom().region("data", 0); // b03 on D31-16, b04 on D15-0
	map(0xa00000, 0xafffff).umask32(0xffff'0000).ram(); // "PRINT RAM", the self test covers bytes 0 and 1 of each long word
	map(0xc00000, 0xcfffff).umask32(0xffff'0000).ram(); // "CCD RAM 1"
	map(0xd00000, 0xdfffff).umask32(0xffff'0000).ram(); // "CCD RAM 2"

	// video chip A
	map(0xe00000, 0xe3ffff).ram().share(m_vidram[0]);
	map(0xe02000, 0xe02fff).ram().w(FUNC(ppstation_state::colorram_w<0>)).share(m_colorram[0]);
	map(0xe03000, 0xe030ff).rw(FUNC(ppstation_state::vidregs_r<0>), FUNC(ppstation_state::vidregs_w<0>));
	map(0xe03800, 0xe03fff).r(FUNC(ppstation_state::gfxrom_window_r<0>));

	// video chip B
	map(0xe40000, 0xe7ffff).ram().share(m_vidram[1]);
	map(0xe42000, 0xe42fff).ram().w(FUNC(ppstation_state::colorram_w<1>)).share(m_colorram[1]);
	map(0xe43000, 0xe430ff).rw(FUNC(ppstation_state::vidregs_r<1>), FUNC(ppstation_state::vidregs_w<1>));
	map(0xe43800, 0xe43fff).r(FUNC(ppstation_state::gfxrom_window_r<1>));

	map(0xe80000, 0xe80003).rw(FUNC(ppstation_state::mixer_r), FUNC(ppstation_state::mixer_w)); // video mixer / camera control
	map(0xe80004, 0xe8001f).noprw();
	map(0xf00000, 0xf0001f).nopw(); // camera interfaces? 16-bit registers, written at reset and once per frame
	map(0xf40000, 0xf4001f).nopw();
	map(0xfc0000, 0xfc0003).noprw();
}


/***************************************************************************
    Sound CPU (guesswork: only the ROM and RAM ranges are confirmed)
***************************************************************************/

void ppstation_state::sound_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("soundcpu", 0);
	map(0x080000, 0x083fff).ram();
	map(0x100000, 0x100001).umask16(0xff00).w(m_soundstatus, FUNC(generic_latch_8_device::write)); // status byte to the main CPU
	map(0x180000, 0x180001).umask16(0xff00).w(m_soundlatch, FUNC(generic_latch_16_device::acknowledge_w)); // interrupt enable / acknowledge latch: the level 6 handler writes 0x4c4c then 0x6c6c
	map(0x200000, 0x200001).nopw(); // ? 0x490f at reset, 0x4d3f once initialised, then 0x4d00 | low byte of each command
	map(0x400000, 0x400003).umask16(0x00ff).rw(m_ymz, FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	map(0x500000, 0x500001).r(m_soundlatch, FUNC(generic_latch_16_device::read)); // command word from the main CPU, polled by the main loop
}


static INPUT_PORTS_START( ppstation )
	PORT_START("IN0") // read at 340008
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_COIN1 ) // counted on release, held for 5 seconds -> coin selector error
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 ) // adds a credit
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW ) // Test: operator menu
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset") // re-initialises the game without the self test
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // order as in the INPUT CHECK screen
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("OK")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Cancel")
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// Names and settings as in the operator's manual and the DIP SW CHECK screen.
	// Read at 340004, decoded at reset (tables at 206d8); the factory settings are all OFF.
	PORT_START("IN1")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DIP SW 1:1,2,3") // "COIN SETTINGS", 100 yen coins
	PORT_DIPSETTING(      0x0001, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0018, 0x0018, "Plays per Credit" ) PORT_DIPLOCATION("DIP SW 1:4,5") // "CREDIT SETTINGS"
	PORT_DIPSETTING(      0x0018, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x0060, 0x0060, "Photo Retakes" ) PORT_DIPLOCATION("DIP SW 1:6,7") // "RETAKE SETTINGS"
	PORT_DIPSETTING(      0x0060, "3" )
	PORT_DIPSETTING(      0x0040, "5" )
	PORT_DIPSETTING(      0x0020, "7" )
	PORT_DIPSETTING(      0x0000, "Unlimited" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "DIP SW 1:8" )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DIP SW 2:1,2") // "SOUND IN ATTRACT MODE"
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, "Once Every 4 Cycles" )
	PORT_DIPSETTING(      0x0200, "Once Every 2 Cycles" )
	PORT_DIPSETTING(      0x0300, "All the Time" )
	PORT_DIPNAME( 0x1c00, 0x1c00, "Camera Brightness" ) PORT_DIPLOCATION("DIP SW 2:3,4,5") // "BRIGHT & CONTRAST SETTINGS", selects a pair of camera register values (table at 1ed00)
	PORT_DIPSETTING(      0x0000, "-40%" )
	PORT_DIPSETTING(      0x0400, "-30%" )
	PORT_DIPSETTING(      0x0800, "-20%" )
	PORT_DIPSETTING(      0x0c00, "-10%" )
	PORT_DIPSETTING(      0x1c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x1800, "+10%" )
	PORT_DIPSETTING(      0x1400, "+20%" )
	PORT_DIPSETTING(      0x1000, "+30%" )
	PORT_DIPNAME( 0x6000, 0x6000, "Camera Colour Level" ) PORT_DIPLOCATION("DIP SW 2:6,7") // "COLOR LEVEL SETTINGS" (table at 1ed10)
	PORT_DIPSETTING(      0x6000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x4000, "Dark" )
	PORT_DIPSETTING(      0x2000, "Light" )
	PORT_DIPSETTING(      0x0000, "Monotone" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP SW 2:8") // not in the manual; with OK held when Test is pressed, selects a different operator mode
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_ppstation )
	GFXDECODE_ENTRY( "gfx_a", 0, gfx_8x8x4_packed_lsb, 0,    64 )
	GFXDECODE_ENTRY( "gfx_b", 0, gfx_8x8x4_packed_lsb, 1024, 64 )
GFXDECODE_END


void ppstation_state::ppstation(machine_config &config)
{
	// basic machine hardware
	M68EC020(config, m_maincpu, 50_MHz_XTAL / 2); // 25 MHz rated part, 50 MHz oscillator; divider is a guess
	m_maincpu->set_addrmap(AS_PROGRAM, &ppstation_state::main_map);

	M68000(config, m_soundcpu, 50_MHz_XTAL / 6); // 68EC000FN8, divider is a guess
	m_soundcpu->set_addrmap(AS_PROGRAM, &ppstation_state::sound_map);

	GENERIC_LATCH_16(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_soundcpu, 6);
	m_soundlatch->set_separate_acknowledge(true); // the sound CPU polls the command and acknowledges the interrupt at 180000

	GENERIC_LATCH_8(config, m_soundstatus);

	// video hardware
	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60); // 32 MHz oscillator on the video board, real timings unknown
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(512, 408);
	m_screen->set_visarea(0, 496 - 1, 0, 384 - 1); // the tilemap frames and the sprites need this much; registers e03030-e0303a (1a1, 17f, 306, 149, f8, 180a) are probably the real timings
	m_screen->set_screen_update(FUNC(ppstation_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set(FUNC(ppstation_state::vblank_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ppstation);
	PALETTE(config, m_palette).set_entries(2048 + 16); // the last 16 are the stand-in camera picture

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YMZ280B(config, m_ymz, 16.9344_MHz_XTAL);
	m_ymz->add_route(0, "speaker", 1.0, 0);
	m_ymz->add_route(1, "speaker", 1.0, 1);
}


ROM_START( ppstation )
	ROM_REGION( 0x100000, "maincpu", 0 ) // HN27C4096 EPROMs on the CPU board
	ROM_LOAD32_WORD_SWAP( "657a01.12c", 0x000000, 0x080000, CRC(eb42a415) SHA1(3f03868a0d152c20705b94ea1143499f4f8646f7) )
	ROM_LOAD32_WORD_SWAP( "657a02.14c", 0x000002, 0x080000, CRC(5cafe0a7) SHA1(cdc22cec9b760257623347cba4cbf3e9036415a7) )

	ROM_REGION( 0x80000, "soundcpu", 0 ) // 27C240
	ROM_LOAD16_WORD_SWAP( "657a06.23n", 0x000000, 0x080000, CRC(b970de57) SHA1(4c6b761ea518a6d9b8d32689ecbdd445101f1d15) )

	ROM_REGION( 0x200000, "gfx_a", 0 ) // tiles for video chip A (monitor), mask ROM on the video board
	ROM_LOAD( "657a08.6n", 0x000000, 0x200000, CRC(38497880) SHA1(6ba0e4a0e57c529f655e9fc9fcf5c390323baa86) )

	ROM_REGION( 0x200000, "gfx_b", 0 ) // sprites for video chip B, mask ROM on the video board
	ROM_LOAD( "657a07.4n", 0x000000, 0x200000, CRC(62aa2a05) SHA1(88b5ba5c6c43d6f84fb52496b7b14b6d72ef4c87) )

	ROM_REGION32_BE( 0x400000, "data", 0 ) // tilemap data (card designs, text): b03 on the upper word, b04 on the lower word (the self test checksums confirm the lanes)
	ROM_LOAD32_WORD_SWAP( "657b03.18g", 0x000000, 0x200000, CRC(83647332) SHA1(c0202b63bf384b60d4536ae38093b483482ab4cd) )
	ROM_LOAD32_WORD_SWAP( "657b04.18h", 0x000002, 0x200000, CRC(c67a3d3f) SHA1(30a4d121c76d3531b5ed7babf3cfe833f5b9b2db) )

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "657a05.9n", 0x000000, 0x200000, CRC(d603e144) SHA1(8d1775bf1d167ec91e962f23b80d17fd4a610f7f) )

	ROM_REGION( 0xa00, "plds", 0 )
	ROM_LOAD( "002799.19k", 0x000, 0x400, NO_DUMP ) // MACH111, CPU board
	ROM_LOAD( "002800.27n", 0x400, 0x117, NO_DUMP ) // PALCE16V8, CPU board
	ROM_LOAD( "002801.25n", 0x600, 0x117, NO_DUMP ) // PALCE16V8, CPU board
	ROM_LOAD( "002400.5d",  0x800, 0x117, NO_DUMP ) // PALCE16V8, video board
ROM_END

} // anonymous namespace


GAME( 1996, ppstation, 0, ppstation, ppstation, ppstation_state, empty_init, ROT0, "Konami", "Puri Puri Station", MACHINE_NOT_WORKING ) // requires camera and printer emulation
