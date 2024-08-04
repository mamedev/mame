// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

C=65 / C=64DX (c) 1991 Commodore

TODO:
- Fails interpreting BASIC commands, CPU core bug?
- DDR/port support from M4510;
- Complete memory model;
- Complete interrupts;
- F011 FDC (1581 equivalent, does it implies IEC?);
- C=64 mode (with "GO64" at BASIC prompt, or holding C= key during boot);
- bios 0 detects an expansion RAM without checking REC first and no matter if there's
  effectively a RAM bank available or not, supposed to bus error or just buggy/hardwired code?

===================================================================================================
References:

Hardware infos can be found at:
http://www.zimmers.net/cbmpics/cbm/c65/c65manual.txt
http://www.zimmers.net/cbmpics/cbm/c65/c65faq20.txt

Hardware pics:
http://www.zimmers.net/cbmpics/cbm/c65/c65-2b-lhs.JPG
http://www.zimmers.net/cbmpics/cbm/c65/c65-2b-rhs.JPG

Schematics:
http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/C65%20Rev%202A%20Schematic.pdf
http://www.zimmers.net/anonftp/pub/cbm/schematics/computers/C64DX_aka_C65_System_Specifications_Preliminary_(1991_Mar).pdf

**************************************************************************************************/

#include "emu.h"
#include "cpu/m6502/m4510.h"
#include "machine/input_merger.h"
#include "machine/mos6526.h"
#include "sound/mos6581.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#define MAIN_C65_CLOCK XTAL(28'375'160) / 8
#define MAIN_C64_CLOCK XTAL(14'318'181) / 14 // TODO: unconfirmed

// TODO: move to own file, subclass with f018a and f018b
// using device_execute_interface will tank performance, avoid it for now.
class dmagic_f018_device : public device_t
{
public:
	// construction/destruction
	dmagic_f018_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }

	void map(address_map &map);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_address_space m_space;

	u32 m_dmalist_address = 0;

	TIMER_CALLBACK_MEMBER(execute_cb);
	typedef enum {
		COPY,
		MIX,
		SWAP,
		FILL,
		IDLE,
		FETCH_PARAMS
	} dma_state_t;

	dma_state_t m_state;
	u32 m_src, m_dst, m_length, m_command, m_modulo;
	u8 m_src_mode, m_dst_mode;
	bool m_chained_transfer;

	emu_timer *m_dma_timer;
	void check_state(int next_cycles);
	void increment_src();
	void increment_dst();
};

// CSG 390957-01
DEFINE_DEVICE_TYPE(DMAGIC_F018, dmagic_f018_device, "dmagic_f018", "DMAgic F018 Gate Array")

dmagic_f018_device::dmagic_f018_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DMAGIC_F018, tag, owner, clock)
	, m_space(*this, finder_base::DUMMY_TAG, -1)
{
}

void dmagic_f018_device::device_start()
{
	save_item(NAME(m_dmalist_address));
	//save_item(NAME(m_state));

	m_dma_timer = timer_alloc(FUNC(dmagic_f018_device::execute_cb), this);
}

void dmagic_f018_device::device_reset()
{
	m_dma_timer->adjust(attotime::never);
	m_state = dma_state_t::IDLE;
	m_chained_transfer = false;
	m_dmalist_address = 0;
}

void dmagic_f018_device::check_state(int next_cycles)
{
	if (m_length != 0)
		m_dma_timer->adjust(attotime::from_ticks((next_cycles), clock()));
	else
	{
		m_chained_transfer = bool(BIT(m_command, 2));
		if (m_chained_transfer)
			popmessage("DMAgic: untested Chain Mode");
		m_state = m_chained_transfer ? FETCH_PARAMS : IDLE;
		m_dma_timer->adjust(m_chained_transfer ? attotime::from_ticks(12, clock()) : attotime::never);
	}
}

void dmagic_f018_device::increment_src()
{
	const int src_dir = BIT(m_src_mode, 2) ? -1 : +1;
	m_src = ((m_src + src_dir) & 0xffff) | (m_src & 0xf'0000);
}

void dmagic_f018_device::increment_dst()
{
	const int dst_dir = BIT(m_dst_mode, 2) ? -1 : +1;
	m_dst = ((m_dst + dst_dir) & 0xffff) | (m_dst & 0xf'0000);
}


TIMER_CALLBACK_MEMBER(dmagic_f018_device::execute_cb)
{
	switch(m_state)
	{
		case dma_state_t::IDLE:
			return;

		case dma_state_t::FETCH_PARAMS: {
			static const char *const dma_cmd_string[] =
			{
				"COPY",
				"MIX",
				"SWAP",
				"FILL"
			};

			u32 address = m_dmalist_address;
			logerror("%05x ", address);
			m_command = m_space->read_byte(address++);

			m_length = m_space->read_byte(address++);
			m_length |=(m_space->read_byte(address++)<<8);

			if (m_length == 0)
				m_length = 0x10000;

			m_src = m_space->read_byte(address++);
			m_src |=(m_space->read_byte(address++)<<8);
			m_src |=(m_space->read_byte(address++)<<16);

			m_dst = m_space->read_byte(address++);
			m_dst |=(m_space->read_byte(address++)<<8);
			m_dst |=(m_space->read_byte(address++)<<16);

			m_modulo = m_space->read_byte(address++);
			m_modulo |=(m_space->read_byte(address++)<<8);

			m_dmalist_address = address;
			m_state = (dma_state_t)(m_command & 3);

			m_src_mode = m_src >> 20;
			m_dst_mode = m_dst >> 20;

			logerror("DMAgic %s [%06x] -> [%06x] length = %04x modulo = %02x (CHAIN=%s)\n"
				, dma_cmd_string[m_command & 3]
				, m_src
				, m_dst
				, m_length
				, m_modulo
				, m_command & 4 ? "yes" : "no"
			);

			if (m_command & 0xf8)
				popmessage("DMAgic: unhandled command %02x", m_command);

			// x--- I/O select
			// -x-- DIR
			//      \- decrements instead of increment, used by "LIST"
			// --x- MOD
			// ---x HOLD
			if ((m_src_mode & 0xb || m_dst_mode & 0xb) && m_length != 1)
			{
				popmessage("DMAgic: unhandled source %06x dst %06x length %05x", m_src, m_dst, m_length);
			}

			m_dma_timer->adjust(attotime::from_ticks((1), clock()));
			break;
		}

		// TODO: very limited use by the BIOS, not extensively tested
		// generally uses length == 1 (for copying BASIC strings from work RAM)
		// or length == 50 (for scrolling vertical text, cfr. c64dx)
		case dma_state_t::COPY: {
			m_space->write_byte(m_dst & 0xf'ffff, m_space->read_byte(m_src & 0xf'ffff));
			increment_src();
			increment_dst();
			m_length --;
			check_state(2);
			break;
		}

		case dma_state_t::MIX:
			popmessage("DMAgic: unimplemented MIX");
			break;

		case dma_state_t::SWAP: {
			popmessage("DMAgic: untested SWAP");
			const u8 tmp_src = m_space->read_byte(m_src & 0xf'ffff);
			const u8 tmp_dst = m_space->read_byte(m_dst & 0xf'ffff);
			m_space->write_byte(m_src & 0xf'ffff, tmp_dst);
			m_space->write_byte(m_dst & 0xf'ffff, tmp_src);
			increment_src();
			increment_dst();
			m_length --;
			check_state(4);
			break;
		}

		case dma_state_t::FILL:
			m_space->write_byte(m_dst & 0xf'ffff, m_src & 0xff);
			increment_dst();
			m_length --;
			check_state(1);
			break;
	}
}


void dmagic_f018_device::map(address_map &map)
{
	map(0, 0).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_dmalist_address &= 0xffff00;
			m_dmalist_address |= data;
			m_state = FETCH_PARAMS;
			m_dma_timer->adjust(attotime::from_ticks((12), clock()));
		})
	);
	map(1, 1).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_dmalist_address &= 0xff00ff;
			m_dmalist_address |= data << 8;
		})
	);
	map(2, 2).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_dmalist_address &= 0x00ffff;
			m_dmalist_address |= data << 16;
		})
	);
	map(3, 3).lr8(
		NAME([this] (offs_t offset) {
			return (m_state != IDLE) << 7 | (m_chained_transfer << 0);
		})
	);
}

namespace {

class c65_state : public driver_device
{
public:
	c65_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cia(*this, "cia_%u", 0U)
		, m_sid(*this, "sid_%u", 0U)
		, m_irqs(*this, "irqs")
		, m_dma(*this, "dma")
		, m_cram_view(*this, "cram_view")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_workram(*this, "work_ram")
		, m_palred(*this, "redpal")
		, m_palgreen(*this, "greenpal")
		, m_palblue(*this, "bluepal")
		, m_cram(*this, "cram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_ipl_rom(*this, "ipl")
	{ }

	// devices
	required_device<m4510_device> m_maincpu;
	required_device_array<mos6526_device, 2> m_cia;
	required_device_array<mos6581_device, 2> m_sid;
	required_device<input_merger_device> m_irqs;
	required_device<dmagic_f018_device> m_dma;
	memory_view m_cram_view;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_palred;
	required_shared_ptr<uint8_t> m_palgreen;
	required_shared_ptr<uint8_t> m_palblue;
	required_shared_ptr<uint8_t> m_cram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_region m_ipl_rom;

	uint8_t m_keyb_input[10]{};
	uint8_t m_keyb_c0_c7 = 0U;
	uint8_t m_keyb_c8_c9 = 0U;

	void vic4567_map(address_map &map);
	void PalRed_w(offs_t offset, uint8_t data);
	void PalGreen_w(offs_t offset, uint8_t data);
	void PalBlue_w(offs_t offset, uint8_t data);
	uint8_t uart_r(offs_t offset);
	void uart_w(offs_t offset, uint8_t data);
	uint8_t cia0_porta_r();
	void cia0_porta_w(uint8_t data);
	uint8_t cia0_portb_r();
	void cia0_portb_w(uint8_t data);

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette);
	void init_c65();
	void init_c65pal();

	void c65(machine_config &config);
	void c65_map(address_map &map);
protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;
	virtual void video_reset() override;
private:
	// TODO: move to own device
	uint8_t m_VIC2_IRQPend = 0U, m_VIC2_IRQMask = 0U;
	uint8_t m_VIC2_EXTColor = 0U;
	uint8_t m_VIC2_VS_CB_Base = 0U;
	uint8_t m_VIC2_BK0_Color = 0U;
	uint8_t m_sprite_enable = 0U;
	uint8_t m_VIC3_ControlA = 0U;
	uint8_t m_VIC3_ControlB = 0U;
	bool m_video_enable = false;

	bool m_blink_enable = false;
	bitmap_ind16 m_bitmap;
	emu_timer *m_scanline_timer;
	TIMER_CALLBACK_MEMBER(scanline_cb);
	std::tuple<u8, bool> get_tile_pixel(int y, int x);
	void PalEntryFlush(uint8_t offset);
	void IRQCheck(uint8_t irq_cause);
};

void c65_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
	m_scanline_timer = timer_alloc(FUNC(c65_state::scanline_cb), this);
}

void c65_state::video_reset()
{
	m_scanline_timer->adjust(m_screen->time_until_pos(m_screen->vpos() + 1, 0), m_screen->vpos() + 1);
	m_video_enable = false;
}

void c65_state::vic4567_map(address_map &map)
{
//  map(0x00, 0x0f) Sprite X/Y
//  map(0x10, 0x10) bit 8 for Sprites X pos
	/*
	 * x--- ---- bit 8 of beam V
	 * -x-- ---- Extended Color Mode
	 * --x- ---- Bitmap Mode
	 * ---x ---- Enable video output
	 * ---- x--- 25/24 visible rows
	 * ---- -xxx Screen Soft V Scroll
	 */
	map(0x11, 0x11).lrw8(
		NAME([this] (offs_t offset) {
			return (m_screen->vpos() & 0x100) >> 1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_video_enable = bool(BIT(data, 4));
			logerror("VIC2 $11 mode %02x\n", data);
		})
	);
	// TODO: writes for rasterline irq trigger
	map(0x12, 0x12).lr8(
		NAME([this] (offs_t offset) {
			return (m_screen->vpos() & 0xff);
		})
	);
	map(0x15, 0x15).lrw8(
		NAME([this] (offs_t offset) {
			return m_sprite_enable;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sprite_enable = data;
		})
	);
	/*
	 * ---x ---- Multicolor Mode
	 * ---- x--- 40/38 visible columns
	 * ---- -xxx Screen Soft Scroll H
	 */
//  map(0x16, 0x16)
//  map(0x17, 0x17) Sprite magnify V
	/*
	 * xxxx ---- Screen RAM base (note bit 4 ignored in C=65 width 80)
	 * ---- xxx- Character Set base
	 */
	map(0x18, 0x18).lrw8(
		NAME([this] (offs_t offset) {
			return m_VIC2_VS_CB_Base;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_VIC2_VS_CB_Base = data;
		})
	);
	/*
	 * x--- ---- Latches high if any IRQ taken below (valid for pending reg only)
	 * ---- x--- Lightpen input IRQ
	 * ---- -x-- Sprite-Sprite collision IRQ
	 * ---- --x- Sprite-Background collision IRQ
	 * ---- ---x rasterline IRQ
	 */
	map(0x19, 0x19).lrw8(
		NAME([this] (offs_t offset) {
			return m_VIC2_IRQPend;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_VIC2_IRQPend &= ~data;
			IRQCheck(0);
		})
	);
	map(0x1a, 0x1a).lrw8(
		NAME([this] (offs_t offset) {
			return m_VIC2_IRQMask;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_VIC2_IRQMask = data & 0xf;
			IRQCheck(0);
		})
	);
//  map(0x1b, 0x1b) Sprite-Background priority
//  map(0x1c, 0x1c) Sprite multicolor enable
//  map(0x1d, 0x1d) Sprite magnify X
//  map(0x1e, 0x1e) Sprite-Sprite collision
//  map(0x1f, 0x1f) Spirte-background collision
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) {
			return m_VIC2_EXTColor;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: all 8-bits in C=65 mode
			m_VIC2_EXTColor = data & 0xf;
			//m_screen->update_partial(m_screen->vpos());
		})
	);
	map(0x21, 0x21).lrw8(
		NAME([this] (offs_t offset) {
			return m_VIC2_BK0_Color;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_VIC2_BK0_Color = data & 0xf;
		})
	);
//  map(0x21, 0x24) background clut
//  map(0x25, 0x26) sprite multicolor clut
//  map(0x27, 0x2e) sprite color clut
	/*
	 * KEY register, handles vic-iii and vic-ii modes via two consecutive writes
	 * 0xa5 -> 0x96 vic-iii mode
	 * any other write vic-ii mode (changes base memory map)
	 * vic-iv (MEGA65/C65GS) also has another KEY init sequence
	 */
//  map(0x2f, 0x2f)
	/*
	 * x--- ---- overlay ROM at $e000
	 * -x-- ---- Move CROM at @9000
	 * --x- ---- overlay ROM at $c000
	 * ---x ---- overlay ROM at $a000
	 * ---- x--- overlay ROM at $8000
	 * ---- -x-- read PALette from [P]ROM
	 * ---- --x- EXT SYNC
	 * ---- ---x overlay CRAM at $dc00
	 */
	map(0x30, 0x30).lrw8(
		NAME([this] (offs_t offset) {
			return m_VIC3_ControlA;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if((data & 0xfe) != 0x64)
				logerror("CONTROL A %02x\n",data);
			m_VIC3_ControlA = data;
			// TODO: all the other bits
			m_cram_view.select(BIT(data, 0));
			logerror("\tROM @ 8000 %d\n", BIT(data, 3));
			logerror("\tROM @ a000 %d\n", BIT(data, 4));
			logerror("\tROM @ c000 %d\n", BIT(data, 5));
			logerror("\tROM @ e000 %d\n", BIT(data, 7));
		})
	);
	/*
	 * x--- ---- H640
	 * -x-- ---- FAST
	 * --x- ---- ATTR
	 * ---x ---- BPM Bitplane Mode
	 * ---- x--- V400
	 * ---- -x-- H1280
	 * ---- --x- MONO
	 * ---- ---x INT
	 */
	map(0x31, 0x31).lrw8(
		NAME([this] (offs_t offset) {
			return m_VIC3_ControlB;
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("CONTROL B %02x\n", data);
			m_VIC3_ControlB = data;
			// FAST mode
			const XTAL clock = BIT(data, 6) ? MAIN_C65_CLOCK : MAIN_C64_CLOCK;
			m_maincpu->set_unscaled_clock(clock);
			m_cia[0]->set_unscaled_clock(clock);
			m_cia[1]->set_unscaled_clock(clock);
		})
	);
//  map(0x32, 0x32) Bitplane enable
//  map(0x33, 0x3a) Bitplane addresses
//  map(0x3b, 0x3b) BP COMP
//  map(0x3c, 0x3c) Bitplane X
//  map(0x3d, 0x3d) Bitplane Y
//  map(0x3e, 0x3e) Horizontal [screen?] position
//  map(0x3f, 0x3f) Vertical [screen?] position
//  map(0x40, 0x47) DAT Bitplane ports
}

std::tuple<u8, bool> c65_state::get_tile_pixel(int y, int x)
{
    // TODO: move width as a screen setup
	int pixel_width = (m_VIC3_ControlB & 0x80) ? 1 : 2;
	int columns = 80 / pixel_width;

    // TODO: Move init of these two in handlers
	uint8_t *cptr = &m_ipl_rom->base()[((m_VIC3_ControlA & 0x40) ? 0x9000: 0xd000) + ((m_VIC2_VS_CB_Base & 0x2) << 10)];
	const u32 base_offs = (m_VIC2_VS_CB_Base & 0xf0) << 6;

	int xi = (x >> 3) / pixel_width;
	int yi = (y >> 3);
	int xm = 7 - ((x / pixel_width) & 7);
	int ym = (y & 7);
	uint8_t tile = m_workram[xi + yi * columns + base_offs];
	uint8_t attr = m_cram[xi + yi * columns];
	int foreground_color = attr & 0xf;
	int background_color = m_VIC2_BK0_Color & 0xf;
	int highlight_color = 0;

	int enable_dot = ((cptr[(tile << 3) + ym] >> xm) & 1);

	if ((attr & 0x80) && ym == 7) enable_dot = 1;
	if (attr & 0x40) highlight_color = 16;
	if (attr & 0x20) enable_dot = !enable_dot;
	if (attr & 0x10 && !m_blink_enable) enable_dot = 0;

	return std::make_tuple(highlight_color + ((enable_dot) ? foreground_color : background_color), enable_dot != 0);
}

TIMER_CALLBACK_MEMBER(c65_state::scanline_cb)
{
	int y = param;
	uint16_t *p = &m_bitmap.pix(y);

	const int border_left = 0;
	const int active_left = 32;
	const int active_right = 640 + active_left;
	const int border_right = active_right + active_left;
	const int active_top = 17;
	const int active_bottom = 200 + active_top;

	int x = border_left;

	if (!m_video_enable)
	{
		// TODO: blank color
		for (x = border_left; x < border_right; x++)
			p[x] = 0;
	}
	else
	{
		if (y < active_top || y >= active_bottom)
		{
			for (x = border_left; x < border_right; x++)
				p[x] = m_VIC2_EXTColor;
		}
		else
		{
			for (x = border_left; x < active_left; x++)
				p[x] = m_VIC2_EXTColor;
			for (;x < active_right; x++)
			{
				u8 tile_dot;
				bool is_foreground;
				// TODO: functional depending on mode
				// NOTE: VIC-II and VIC-III can switch mid-frame, but latches occur in 8 scanline steps
                // at least from/to a base C=64 tilemap mode.
				std::tie(tile_dot, is_foreground) = get_tile_pixel(y - active_top, x - active_left);
				// TODO: sprites fetch here
				p[x] = m_palette->pen(tile_dot);
			}
			for (;x < border_right; x++)
				p[x] = m_VIC2_EXTColor;
		}
	}

	// TODO: raster irq position
	if (y == 0xff)
		IRQCheck(1);

	y += 1;
	y %= 262;
	if (y == 0)
		m_blink_enable = bool((m_screen->frame_number() & 4) == 4);
	m_scanline_timer->adjust(m_screen->time_until_pos(y, 0), y);
}

uint32_t c65_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

void c65_state::PalEntryFlush(uint8_t offset)
{
	m_palette->set_pen_color(offset, pal4bit(m_palred[offset]), pal4bit(m_palgreen[offset]), pal4bit(m_palblue[offset]));
}

void c65_state::PalRed_w(offs_t offset, uint8_t data)
{
	// TODO: bit 4 for FG/BG, superimposing?
	m_palred[offset] = data;
	PalEntryFlush(offset);
}

void c65_state::PalGreen_w(offs_t offset, uint8_t data)
{
	m_palgreen[offset] = data;
	PalEntryFlush(offset);
}

void c65_state::PalBlue_w(offs_t offset, uint8_t data)
{
	m_palblue[offset] = data;
	PalEntryFlush(offset);
}

uint8_t c65_state::uart_r(offs_t offset)
{
	switch (offset)
	{
		case 7:
			return ioport("CAPS")->read();
	}
	return 0xff;
}

void c65_state::uart_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 7:
			m_keyb_c8_c9 = (~data >> 1) & 3;
			break;
		case 8:
			// ddr?
			break;
	}
}

uint8_t c65_state::cia0_porta_r()
{
	return 0xff;
}

uint8_t c65_state::cia0_portb_r()
{
	static const char *const c64ports[] = { "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7" };
	static const char *const c65ports[] = { "C8", "C9" };
	uint8_t res;

	res = 0xff;
	for(int i=0;i<8;i++)
	{
		m_keyb_input[i] = ioport(c64ports[i])->read();

		if(m_keyb_c0_c7 & 1 << (i))
			res &= m_keyb_input[i];
	}

	for(int i=0;i<2;i++)
	{
		m_keyb_input[i+8] = ioport(c65ports[i])->read();

		if(m_keyb_c8_c9 & 1 << (i))
			res &= m_keyb_input[i+8];
	}

	return res;
}

void c65_state::cia0_porta_w(uint8_t data)
{
	m_keyb_c0_c7 = ~data;
//  logerror("%02x\n",m_keyb_c0_c7);
}

void c65_state::cia0_portb_w(uint8_t data)
{
}


void c65_state::c65_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x07fff).ram().share("work_ram");
	map(0x08000, 0x0bfff).rom().region("ipl", 0x08000);
	map(0x0c000, 0x0cfff).rom().region("ipl", 0x0c000);
	map(0x0d000, 0x0d07f).m(*this, FUNC(c65_state::vic4567_map));
	// 0x0d080, 0x0d09f FDC
	map(0x0d080, 0x0d09f).lr8(NAME([] (offs_t offset) { return 0; }));
	// 0x0d0a0, 0x0d0ff Ram Expansion Control (REC)
	map(0x0d100, 0x0d1ff).ram().w(FUNC(c65_state::PalRed_w)).share("redpal");
	map(0x0d200, 0x0d2ff).ram().w(FUNC(c65_state::PalGreen_w)).share("greenpal");
	map(0x0d300, 0x0d3ff).ram().w(FUNC(c65_state::PalBlue_w)).share("bluepal");
	// 0x0d400, 0x0d4*f Right SID
	// keyboard hold left shift will read to $d484 (?)
	map(0x0d400, 0x0d41f).mirror(0x80).rw(m_sid[1], FUNC(mos6581_device::read), FUNC(mos6581_device::write));
	// 0x0d440, 0x0d4*f Left  SID
	map(0x0d440, 0x0d45f).mirror(0x80).rw(m_sid[0], FUNC(mos6581_device::read), FUNC(mos6581_device::write));
	map(0x0d600, 0x0d6ff).rw(FUNC(c65_state::uart_r), FUNC(c65_state::uart_w));
	// 0x0d700, 0x0d7** DMAgic
	map(0x0d700, 0x0d703).m(m_dma, FUNC(dmagic_f018_device::map));
	// 0x0d800, 0x0d8** Color matrix
	map(0x0d800, 0x0dfff).view(m_cram_view);
	// maps lower 1024 bytes regardless of the setting (essentially touches $dc00 as overlay)
	m_cram_view[0](0x0d800, 0x0dbff).lrw8(
		NAME([this] (offs_t offset) { return m_cram[offset]; }),
		NAME([this] (offs_t offset, u8 data) { m_cram[offset] = data; })
	);

	m_cram_view[0](0x0dc00, 0x0dc0f).rw(m_cia[0], FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	m_cram_view[0](0x0dd00, 0x0dd0f).rw(m_cia[1], FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	// 0x0de00, 0x0de** Ext I/O Select 1
	// 0x0df00, 0x0df** Ext I/O Select 2 (RAM window?)
	m_cram_view[1](0x0d800, 0x0dfff).ram().share("cram");
	map(0x0e000, 0x0ffff).rom().region("ipl", 0x0e000);
	map(0x10000, 0x1f7ff).ram();
	map(0x1f800, 0x1ffff).ram().share("cram");
	map(0x20000, 0x3ffff).rom().region("ipl", 0);
//  0x40000, 0x7ffff cart expansion
//  0x80000, 0xfffff RAM expansion
//  map(0x80000, 0xfffff).ram();
}



static INPUT_PORTS_START( c65 )
	PORT_START("C0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("INST DEL") PORT_CODE(KEYCODE_BACKSPACE)       PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER)             PORT_CHAR(13)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Right Left") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)                                    PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)                                    PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)                                    PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)                                    PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Crsr Down Up") PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("C1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)         PORT_CHAR('W')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)         PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)         PORT_CHAR('Z')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)         PORT_CHAR('S')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)         PORT_CHAR('E')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Left)") PORT_CODE(KEYCODE_LSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("C2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)         PORT_CHAR('R')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)         PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)         PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)         PORT_CHAR('F')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)         PORT_CHAR('T')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)         PORT_CHAR('X')

	PORT_START("C3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)         PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)         PORT_CHAR('G')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)         PORT_CHAR('B')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)         PORT_CHAR('H')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)         PORT_CHAR('U')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)         PORT_CHAR('V')

	PORT_START("C4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)         PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)         PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)         PORT_CHAR('0')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)         PORT_CHAR('M')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)         PORT_CHAR('K')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)         PORT_CHAR('O')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)         PORT_CHAR('N')

	PORT_START("C5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('+')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)         PORT_CHAR('P')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)         PORT_CHAR('L')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('-')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(':') PORT_CHAR('[')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('<')

	PORT_START("C6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)                        PORT_CHAR(0xA3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)                        PORT_CHAR('*')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)                             PORT_CHAR(';') PORT_CHAR(']')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CLR HOME") PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift (Right)") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)                         PORT_CHAR('=')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x91  Pi") PORT_CODE(KEYCODE_DEL) PORT_CHAR(0x2191) PORT_CHAR(0x03C0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)                             PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("C7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)                                 PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("\xE2\x86\x90") PORT_CODE(KEYCODE_TILDE)   PORT_CHAR(0x2190)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LCONTROL)                          PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)                                 PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)                             PORT_CHAR(' ')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CBM") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)                                 PORT_CHAR('Q')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RUN STOP") PORT_CODE(KEYCODE_HOME)

	PORT_START("C8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NO SCROLL") PORT_CODE(KEYCODE_SCRLOCK)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("HELP") PORT_CODE(KEYCODE_F9)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)                                    PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6)                                    PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7)                                    PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)

	PORT_START("C9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("linefeed?")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("cursor left?")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("UNKNOWN8")

	PORT_START("CAPS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_F8) PORT_TOGGLE
INPUT_PORTS_END


void c65_state::machine_start()
{
	save_pointer(NAME(m_cram.target()), 0x800);
}

void c65_state::machine_reset()
{
	m_VIC3_ControlA = 0;
	m_cram_view.select(0);
}


void c65_state::palette_init(palette_device &palette)
{
	for (int i = 0; i < 0x100; i++)
		PalEntryFlush(i);
}

// debug
static const gfx_layout charlayout =
{
	8,8,
	0x1000/8,
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_c65 )
	GFXDECODE_ENTRY( "ipl", 0xd000, charlayout,     0, 16 )
	// almost identical to above
	GFXDECODE_ENTRY( "ipl", 0x9000, charlayout,     0, 16 )
GFXDECODE_END

void c65_state::IRQCheck(uint8_t irq_cause)
{
	m_VIC2_IRQPend |= (irq_cause != 0) ? 0x80 : 0x00;
	m_VIC2_IRQPend |= irq_cause;

	m_irqs->in_w<1>(m_VIC2_IRQMask & m_VIC2_IRQPend);
}

void c65_state::c65(machine_config &config)
{
	/* basic machine hardware */
	M4510(config, m_maincpu, MAIN_C65_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &c65_state::c65_map);

	// TODO: multiply by x8 because with a more canonical x1 no transfer will complete in time.
	// is this thing a mixed burst/cycle steal really?
	DMAGIC_F018(config, m_dma, MAIN_C65_CLOCK * 8);
	m_dma->set_space(m_maincpu, AS_PROGRAM);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irqs"));
	irq.output_handler().set_inputline(m_maincpu, M4510_IRQ_LINE);

	MOS6526(config, m_cia[0], MAIN_C65_CLOCK);
	m_cia[0]->set_tod_clock(60);
	m_cia[0]->irq_wr_callback().set("irqs", FUNC(input_merger_device::in_w<0>));
	m_cia[0]->pa_rd_callback().set(FUNC(c65_state::cia0_porta_r));
	m_cia[0]->pa_wr_callback().set(FUNC(c65_state::cia0_porta_w));
	m_cia[0]->pb_rd_callback().set(FUNC(c65_state::cia0_portb_r));
	m_cia[0]->pb_wr_callback().set(FUNC(c65_state::cia0_portb_w));

	MOS6526(config, m_cia[1], MAIN_C65_CLOCK);
	m_cia[1]->set_tod_clock(60);
//  m_cia[1]->irq_wr_callback().set(FUNC(c65_state::cia1_irq)); // NMI
//  m_cia[1]->pa_rd_callback().set(FUNC(c65_state::c65_cia1_port_a_r));
//  m_cia[1]->pa_wr_callback().set(FUNC(c65_state::c65_cia1_port_a_w));


	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(c65_state::screen_update));
	// TODO: stub parameters
	// C=64 / width 40 modes should actually be running in 320x200
	m_screen->set_raw(MAIN_C65_CLOCK*4, 910, 0, 640+32 * 2, 262, 0, 200+17*2);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_c65);

	PALETTE(config, m_palette, FUNC(c65_state::palette_init), 0x100);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	// 8580 SID
	MOS6581(config, m_sid[0], MAIN_C64_CLOCK);
	//m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	//m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid[0]->add_route(ALL_OUTPUTS, "lspeaker", 0.50);

	MOS6581(config, m_sid[1], MAIN_C64_CLOCK);
	//m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	//m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid[1]->add_route(ALL_OUTPUTS, "rspeaker", 0.50);


	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("c65_flop");
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( c65 )
	ROM_REGION( 0x20000, "ipl", 0 )
	ROM_SYSTEM_BIOS( 0, "910111", "V0.9.910111" ) // sum16 CAFF, this shows up on the picture from a spare, unused rom on the 20171102 c64dx auction as "390488-02 CAFF" with the 02 scratched off on the chip and 03 written in pen, unclear what the "correct" label is.
	ROMX_LOAD( "910111.bin", 0x0000, 0x20000, CRC(c5d8d32e) SHA1(71c05f098eff29d306b0170e2c1cdeadb1a5f206), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "910523", "V0.9.910523" ) // sum16 B96B
	ROMX_LOAD( "910523.bin", 0x0000, 0x20000, CRC(e8235dd4) SHA1(e453a8e7e5b95de65a70952e9d48012191e1b3e7), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "910626", "V0.9.910626" ) // sum16 888C
	ROMX_LOAD( "910626.bin", 0x0000, 0x20000, CRC(12527742) SHA1(07c185b3bc58410183422f7ac13a37ddd330881b), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "910828", "V0.9.910828" ) // sum16 C9CD
	ROMX_LOAD( "910828.bin", 0x0000, 0x20000, CRC(3ee40b06) SHA1(b63d970727a2b8da72a0a8e234f3c30a20cbcb26), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "911001", "V0.9.911001" ) // sum16 4BCF
	ROMX_LOAD( "911001.bin", 0x0000, 0x20000, CRC(0888b50f) SHA1(129b9a2611edaebaa028ac3e3f444927c8b1fc5d), ROM_BIOS(4) )
ROM_END

ROM_START( c64dx )
	ROM_REGION( 0x20000, "ipl", 0 ) // "v0.90.910429", sum16 E96A
	ROM_LOAD( "910429.bin", 0x0000, 0x20000, CRC(b025805c) SHA1(c3b05665684f74adbe33052a2d10170a1063ee7d) )
ROM_END

void c65_state::init_c65()
{
//  m_dma.version = 2;
//  c65_common_driver_init();
}

void c65_state::init_c65pal()
{
//  m_dma.version = 1;
//  c65_common_driver_init();
//  m_pal = 1;
}

} // anonymous namespace

COMP( 1991, c65,   0,   0, c65, c65, c65_state, init_c65,    "Commodore Business Machines", "Commodore 65 Development System (Prototype, NTSC)",          MACHINE_NOT_WORKING )
COMP( 1991, c64dx, c65, 0, c65, c65, c65_state, init_c65pal, "Commodore Business Machines", "Commodore 64DX Development System (Prototype, PAL, German)", MACHINE_NOT_WORKING )
