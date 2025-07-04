// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

C=65 / C=64DX (c) 1991 Commodore

TODO:
- DDR/port support from M4510;
- Complete memory model;
\- rom8 / roma / rome all causes bootstrap issues if hooked up (needs the CPU DDR port?)
\- Work RAM should really use a memory_share_creator, VIC-III can potentially access the full range.
\- CRAM is really just RAM that bitplane mode can access as-is;
- Complete interrupts;
- F011 FDC (1581 equivalent, does it implies IEC?);
- bios 0 detects an expansion RAM without checking REC first and no matter if there's
  effectively a RAM bank available or not, supposed to bus error or just buggy/hardwired code?

Notes:
- C=64 mode can be entered in two ways: with "GO64" at BASIC prompt, or by holding C= key during
boot;
- C=65 mode from C=64 BASIC is possible, as long as you do some pre-setup such as setting KEY in
VIC-III mode and disable CIA irqs;

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

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/vcs_ctrl/ctrl.h"
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

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	enum dma_state_t : u8 {
		COPY,
		MIX,
		SWAP,
		FILL,
		IDLE,
		FETCH_PARAMS
	};

	required_address_space m_space;

	u32 m_dmalist_address = 0;

	dma_state_t m_state;
	u32 m_src, m_dst, m_length, m_command, m_modulo;
	u8 m_src_mode, m_dst_mode;
	bool m_chained_transfer;

	emu_timer *m_dma_timer;

	TIMER_CALLBACK_MEMBER(execute_cb);

	void check_state(int next_cycles);
	void increment_src();
	void increment_dst();
};

ALLOW_SAVE_TYPE(dmagic_f018_device::dma_state_t)

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
	save_item(NAME(m_state));

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
		, m_nmis(*this, "nmis")
		, m_dma(*this, "dma")
		, m_joy(*this, "joy%u", 1U)
		, m_cram_view(*this, "cram_view")
		//, m_rom8_view(*this, "rom8_view")
		//, m_roma_view(*this, "roma_view")
		, m_romc_view(*this, "romc_view")
		//, m_rome_view(*this, "rome_view")
		//, m_loram_view(*this, "loram_view")
		//, m_hiram_view(*this, "hiram_view")
		, m_charen_view(*this, "charen_view")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_workram(*this, "work_ram", 0x20000, ENDIANNESS_LITTLE)
		, m_palred(*this, "redpal")
		, m_palgreen(*this, "greenpal")
		, m_palblue(*this, "bluepal")
		, m_cram(*this, "cram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_ipl_rom(*this, "ipl")
		, m_cart_exp(*this, "cart_exp")
		, m_exrom_view(*this, "exrom_view")
	{ }

	void init_c65();
	void init_c65pal();

	void c65(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;
private:
	required_device<m4510_device> m_maincpu;
	required_device_array<mos6526_device, 2> m_cia;
	required_device_array<mos6581_device, 2> m_sid;
	required_device<input_merger_device> m_irqs;
	required_device<input_merger_device> m_nmis;
	required_device<dmagic_f018_device> m_dma;
	required_device_array<vcs_control_port_device, 2> m_joy;

	memory_view m_cram_view;
	//memory_view m_rom8_view;
	//memory_view m_roma_view;
	memory_view m_romc_view;
	//memory_view m_rome_view;
	//memory_view m_loram_view;
	//memory_view m_hiram_view;
	memory_view m_charen_view;

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	memory_share_creator<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_palred;
	required_shared_ptr<uint8_t> m_palgreen;
	required_shared_ptr<uint8_t> m_palblue;
	required_shared_ptr<uint8_t> m_cram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_memory_region m_ipl_rom;
	required_device<generic_slot_device> m_cart_exp;
	memory_view m_exrom_view;

	uint8_t m_keyb_input[10]{};
	uint8_t m_keyb_c0_c7 = 0U;
	uint8_t m_keyb_c8_c9 = 0U;

	void vic4567_map(address_map &map) ATTR_COLD;
	void palette_red_w(offs_t offset, uint8_t data);
	void palette_green_w(offs_t offset, uint8_t data);
	void palette_blue_w(offs_t offset, uint8_t data);
	uint8_t uart_r(offs_t offset);
	void uart_w(offs_t offset, uint8_t data);
	uint8_t cia0_porta_r();
	void cia0_porta_w(uint8_t data);
	uint8_t cia0_portb_r();
	void cia0_portb_w(uint8_t data);
	void cia1_porta_w(uint8_t data);
	uint8_t cpu_r();
	void cpu_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette);

	void c65_map(address_map &map) ATTR_COLD;

	void irq_check(uint8_t irq_cause);

	template <unsigned StartBase> uint8_t ram_r(offs_t offset)
	{
		return m_workram[offset + StartBase];
	}

	template <unsigned StartBase> void ram_w(offs_t offset, uint8_t data)
	{
		m_workram[offset + StartBase] = data;
	}

	u16 m_vic_bank_base = 0U;

	// TODO: move to own device
	uint8_t m_irq_pending = 0U, m_irq_mask = 0U;
	uint8_t m_border_color = 0U;
	uint8_t m_vs_cb_base = 0U;
	uint8_t m_bk_color_clut[4]{};
	uint8_t m_sprite_enable = 0U;
	uint8_t m_control_a = 0U;
	uint8_t m_control_b = 0U;
	u16 m_rcr = 0xffU;
	bool m_ecm = false;
	bool m_bmm = false;
	bool m_blnk = false;
	u8 m_yscl = 0U;
	u8 m_xscl = 0U;
	bool m_csel = false;
	bool m_mcm = false;
	u8 m_ssc = 0U;
	u8 m_sbc = 0U;
	// handler helpers
	u16 m_vs_base_offset = 0U;
	u16 m_cb_base_offset = 0U;
	u8 m_gfxmode = 0U;
	u8* m_video_ptr = nullptr;
	u8* m_char_ptr = nullptr;

	// TODO: tracer bullet
	// this pointer fetch should happen as a bridge between VIC device and memory controller.
	void flush_cb_base()
	{
		m_video_ptr = &m_workram[m_vs_base_offset | m_vic_bank_base];
		// In bitmap modes only bit 3 has weight for CB base, cfr. isoccer
		if (m_gfxmode & 2)
		{
			m_char_ptr = &m_workram[(m_cb_base_offset & 0x2000) | m_vic_bank_base];
		}
		else
		{
			if ((m_cb_base_offset & 0x3000) == 0x1000 && !(BIT(m_vic_bank_base, 14)))
			{
				m_char_ptr = &m_ipl_rom->base()[((BIT(m_control_a, 6)) ? 0x9000 : 0xd000) + (m_cb_base_offset & 0x800)];
			}
			else
				m_char_ptr = &m_workram[m_cb_base_offset | m_vic_bank_base];
		}
	};
	struct {
		u16 x;
		u8 y;
		u8 clut;
	}m_sprite[16];
	u8 m_sprite_hi_xoffs;
	u8 m_sexx, m_sexy;
	u8 m_scm;
	u8 m_sprite_multicolor_clut[2];
	u8 m_bsp;

	bool m_blink_enable = false;
	bitmap_ind16 m_bitmap;
	emu_timer *m_scanline_timer;
	TIMER_CALLBACK_MEMBER(scanline_cb);
	std::tuple<u8, bool> get_tile_pixel(int y, int x);
	std::tuple<u8, u8, u8, bool> get_sprite_pixel(int y, int x);
	typedef std::tuple<u8, bool> (c65_state::*draw_tile_func)(u8 tile, u8 attr, int yi, int xi, int pixel_width);
	static const draw_tile_func draw_tile_table[8];
	std::tuple<u8, bool> draw_standard_char(u8 tile, u8 attr, int yi, int xi, int pixel_width);
	std::tuple<u8, bool> draw_multicolor_char(u8 tile, u8 attr, int yi, int xi, int pixel_width);
	std::tuple<u8, bool> draw_standard_bitmap(u8 tile, u8 attr, int yi, int xi, int pixel_width);
	std::tuple<u8, bool> draw_multicolor_bitmap(u8 tile, u8 attr, int yi, int xi, int pixel_width);
	std::tuple<u8, bool> draw_extended_background(u8 tile, u8 attr, int yi, int xi, int pixel_width);
	std::tuple<u8, bool> draw_invalid(u8 tile, u8 attr, int yi, int xi, int pixel_width);

	void palette_entry_flush(uint8_t offset);
};

// TODO: c65 bitplane mode
const c65_state::draw_tile_func c65_state::draw_tile_table[8] =
{
	&c65_state::draw_standard_char,
	&c65_state::draw_multicolor_char,
	&c65_state::draw_standard_bitmap,
	&c65_state::draw_multicolor_bitmap,
	&c65_state::draw_extended_background,
	&c65_state::draw_invalid,
	&c65_state::draw_invalid,
	&c65_state::draw_invalid
};


void c65_state::video_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
	m_scanline_timer = timer_alloc(FUNC(c65_state::scanline_cb), this);
}

void c65_state::video_reset()
{
	m_scanline_timer->adjust(m_screen->time_until_pos(m_screen->vpos() + 1, 0), m_screen->vpos() + 1);
	m_blnk = false;
	// vestigial, so to have a pointer in any case
	flush_cb_base();
}

void c65_state::vic4567_map(address_map &map)
{
//  53248/$d000 - 53263/$d00f S#X - S#Y Sprite X/Y
	map(0x00, 0x00).select(0xe).lrw8(
		NAME([this] (offs_t offset){
			return m_sprite[offset >> 1].x & 0xff;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sprite[offset >> 1].x &= 0x100;
			m_sprite[offset >> 1].x |= data;
		})
	);
	map(0x01, 0x01).select(0xe).lrw8(
		NAME([this] (offs_t offset){
			return m_sprite[offset >> 1].y;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sprite[offset >> 1].y = data;
		})
	);
//  53264/$d010 S#X8 bit 8 for Sprites X pos
	map(0x10, 0x10).lrw8(
		NAME([this] (offs_t offset) {
			return m_sprite_hi_xoffs;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sprite_hi_xoffs = data;
			for (int i = 0; i < 8; i++)
			{
				m_sprite[i].x &= 0xff;
				m_sprite[i].x |= BIT(data, i) << 8;
			}
		})
	);
/*
 * 53265/$d011
 * x--- ---- RC8 bit 8 of beam V
 * -x-- ---- ECM Extended Color Mode
 * --x- ---- BMM Bitmap Mode
 * ---x ---- BLNK Enable video output
 * ---- x--- RSEL 25/24 visible rows
 * ---- -xxx YSCL2-0 Screen Soft V Scroll
 */
	map(0x11, 0x11).lrw8(
		NAME([this] (offs_t offset) {
			return ((m_screen->vpos() & 0x100) >> 1) | (m_ecm << 6) | (m_bmm << 5) | (m_blnk << 4) | (m_yscl & 7);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_rcr = ((data & 0x80) << 1) | (m_rcr & 0xff);
			m_ecm = bool(BIT(data, 6));
			m_bmm = bool(BIT(data, 5));
			m_blnk = bool(BIT(data, 4));
			m_yscl = data & 7;
			logerror("VIC2: 53265 mode %02x\n", data);
			m_gfxmode = (m_ecm << 2) | (m_bmm << 1) | (m_mcm << 0);
			flush_cb_base();
		})
	);
//  53266/$d012 RC Raster CouNT
	map(0x12, 0x12).lrw8(
		NAME([this] (offs_t offset) {
			return (m_screen->vpos() & 0xff);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_rcr = data | (m_rcr & 0x100);
		})
	);
//  map(0x13, 0x13) 53267/$d013 LPX lightpen X
//  map(0x14, 0x14) 53268/$d014 LPY lightpen Y
	// 53269/$d015 SE# Sprite Enable
	map(0x15, 0x15).lrw8(
		NAME([this] (offs_t offset) {
			return m_sprite_enable;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sprite_enable = data;
		})
	);
/*
 *  53270/$d016
 * --x- ---- RST <unknown>, in c65 specs only?
 * ---x ---- MCM [Background] Multicolor Mode
 * ---- x--- CSEL 40/38 visible columns
 * ---- -xxx XSCL2-0 Screen Soft Scroll H
 */
	map(0x16, 0x16).lrw8(
		NAME([this] (offs_t offset) {
			return (m_mcm << 4) | (m_csel << 3) | (m_xscl & 7);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_mcm = bool(BIT(data, 4));
			m_csel = bool(BIT(data, 3));
			m_xscl = data & 7;
			m_gfxmode = (m_ecm << 2) | (m_bmm << 1) | (m_mcm << 0);
			flush_cb_base();
		})
	);
//  53271/$d017 SEXY# Sprite magnify V
	map(0x17, 0x17).lrw8(
		NAME([this] (offs_t offset) {
			return m_sexy;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sexy = data;
		})
	);
/*
 * 53272/$d018
 * xxxx ---- Screen RAM base (note bit 4 ignored in C=65 width 80)
 * ---- xxx- Character Set base
 */
	map(0x18, 0x18).lrw8(
		NAME([this] (offs_t offset) {
			return m_vs_cb_base;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_vs_cb_base = data;
			m_vs_base_offset = (m_vs_cb_base & 0xf0) << 6;
			m_cb_base_offset = (m_vs_cb_base & 0x0e) << 10;
			flush_cb_base();
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
			return m_irq_pending;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_irq_pending &= ~data;
			irq_check(0);
		})
	);
	map(0x1a, 0x1a).lrw8(
		NAME([this] (offs_t offset) {
			return m_irq_mask;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_irq_mask = data & 0xf;
			irq_check(0);
		})
	);
//  53275/$d01b BSP# Sprite-Background priority
	map(0x1b, 0x1b).lrw8(
		NAME([this] (offs_t offset) {
			return m_bsp;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_bsp = data;
		})
	);
//  53276/$d01c SCM# Sprite multicolor enable
	map(0x1c, 0x1c).lrw8(
		NAME([this] (offs_t offset) {
			return m_scm;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_scm = data;
		})
	);
//  53277/$d01d Sprite magnify X
	map(0x1d, 0x1d).lrw8(
		NAME([this] (offs_t offset) {
			return m_sexx;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sexx = data;
		})
	);
//  53278/$d01e Sprite-Sprite collision
	map(0x1e, 0x1e).lr8(
		NAME([this] (offs_t offset) {
			u8 res = m_ssc;
			if (!machine().side_effects_disabled())
				m_ssc = 0;
			return res;
		})
	);
//  53279/$d01f Sprite-background collision
	map(0x1f, 0x1f).lr8(
		NAME([this] (offs_t offset) {
			u8 res = m_sbc;
			if (!machine().side_effects_disabled())
				m_sbc = 0;
			return res;
		})
	);
//  53280/$d020 BORD border color
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) {
			return m_border_color;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// TODO: all 8-bits in C=65 mode
			m_border_color = data & 0xf;
			//m_screen->update_partial(m_screen->vpos());
		})
	);
//  53281/$d021 BK#C background clut BK0-BK3
	map(0x21, 0x24).lrw8(
		NAME([this] (offs_t offset) {
			return m_bk_color_clut[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_bk_color_clut[offset] = data & 0xf;
		})
	);
//  53285/$d025 - 53286/$d026 sprite multicolor clut
	map(0x25, 0x26).lrw8(
		NAME([this] (offs_t offset) {
			return m_sprite_multicolor_clut[offset];
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sprite_multicolor_clut[offset] = data & 0xf;
		})
	);
//  53287/$d027 - 53294/$d02e sprite color clut
	map(0x27, 0x2e).lrw8(
		NAME([this] (offs_t offset) {
			return m_sprite[offset].clut;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_sprite[offset].clut = data & 0xf;
		})
	);
/*
 * KEY register, handles vic-iii and vic-ii modes via two consecutive writes
 * 0xa5 -> 0x96 vic-iii mode
 * any other write vic-ii mode (changes base memory map)
 * vic-iv (MEGA65/C65GS) also has another KEY init sequence (0x47 -> 0x53)
 */
//  map(0x2f, 0x2f)
/*
 * x--- ---- overlay ROM at $e000
 * -x-- ---- Move CROM at $9000 (C=65), $d000 otherwise (C=64)
 * --x- ---- overlay ROM at $c000
 * ---x ---- overlay ROM at $a000
 * ---- x--- overlay ROM at $8000
 * ---- -x-- read PALette from [P]ROM
 * ---- --x- EXT SYNC
 * ---- ---x overlay CRAM at $dc00
 */
	map(0x30, 0x30).lrw8(
		NAME([this] (offs_t offset) {
			return m_control_a;
		}),
		NAME([this] (offs_t offset, u8 data) {
			if((data & 0xfe) != 0x64)
				logerror("CONTROL A %02x\n",data);
			m_control_a = data;
			// TODO: all the other bits
			m_cram_view.select(BIT(data, 0));
			//m_rom8_view.select(BIT(data, 3));
			//m_roma_view.select(BIT(data, 4));
			m_romc_view.select(BIT(data, 5));
			flush_cb_base();
			//m_rome_view.select(BIT(data, 7));
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
			return m_control_b;
		}),
		NAME([this] (offs_t offset, u8 data) {
			logerror("CONTROL B %02x\n", data);
			m_control_b = data;
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

std::tuple<u8, bool> c65_state::draw_standard_char(u8 tile, u8 attr, int yi, int xi, int pixel_width)
{
	const int xm = 7 - ((xi / pixel_width) & 7);
	const int ym = (yi & 7);
	const int foreground_color = attr & 0xf;
	const int background_color = m_bk_color_clut[0] & 0xf;
	u8 highlight_color = 0;

	u8 enable_dot = ((m_char_ptr[((tile << 3) + ym) & 0x3fff] >> xm) & 1);
	if ((attr & 0x80) && ym == 7) enable_dot = 1;
	if (attr & 0x40) highlight_color = 16;
	if (attr & 0x20) enable_dot = !enable_dot;
	if (attr & 0x10 && !m_blink_enable) enable_dot = 0;

	return std::make_tuple(highlight_color + ((enable_dot) ? foreground_color : background_color), enable_dot != 0);
}

std::tuple<u8, bool> c65_state::draw_multicolor_char(u8 tile, u8 attr, int yi, int xi, int pixel_width)
{
	if (!(attr & 8))
		return draw_standard_char(tile, attr, yi, xi, pixel_width);
	const int xm = 6 - ((xi / pixel_width) & 6);
	const int ym = (yi & 7);
	const u8 color11 = attr & 0xf;
	const std::array<u8, 4> color_map = { m_bk_color_clut[0], m_bk_color_clut[1], m_bk_color_clut[2], color11 };

	u8 highlight_color = 0;

	u8 enable_dot = ((m_char_ptr[((tile << 3) + ym) & 0x3fff] >> xm) & 3);
	if ((attr & 0x80) && ym == 7) enable_dot = 1;
	if (attr & 0x40) highlight_color = 16;
	if (attr & 0x20) enable_dot = !enable_dot;
	if (attr & 0x10 && !m_blink_enable) enable_dot = 0;

	return std::make_tuple(highlight_color + color_map[enable_dot], enable_dot & 2);
}

std::tuple<u8, bool> c65_state::draw_standard_bitmap(u8 tile, u8 attr, int yi, int xi, int pixel_width)
{
	const int xm = 7 - ((xi / pixel_width) & 7);
	const int ym = (yi & 7);
	const int foreground_color = tile >> 4;
	const int background_color = tile & 0xf;
	u8 highlight_color = 0;

	u8 enable_dot = ((m_char_ptr[((xi >> (3 + (pixel_width - 1))) * 8  + (yi >> 3) * 320 + ym) & 0x3fff] >> xm) & 1);
//  if ((attr & 0x80) && ym == 7) enable_dot = 1;
//  if (attr & 0x40) highlight_color = 16;
//  if (attr & 0x20) enable_dot = !enable_dot;
//  if (attr & 0x10 && !m_blink_enable) enable_dot = 0;

	return std::make_tuple(highlight_color + ((enable_dot) ? foreground_color : background_color), enable_dot != 0);
}

std::tuple<u8, bool> c65_state::draw_multicolor_bitmap(u8 tile, u8 attr, int yi, int xi, int pixel_width)
{
	const int xm = 6 - ((xi / pixel_width) & 6);
	const int ym = (yi & 7);
	const u8 color01 = tile >> 4;
	const u8 color10 = tile & 0xf;
	const u8 color11 = attr & 0xf;
	const std::array<u8, 4> color_map = { m_bk_color_clut[0], color01, color10, color11 };

	u8 highlight_color = 0;
	u8 enable_dot = ((m_char_ptr[((xi >> (3 + (pixel_width - 1))) * 8  + (yi >> 3) * 320 + ym) & 0x3fff] >> xm) & 3);
//  if ((attr & 0x80) && ym == 7) enable_dot = 1;
//  if (attr & 0x40) highlight_color = 16;
//  if (attr & 0x20) enable_dot = !enable_dot;
//  if (attr & 0x10 && !m_blink_enable) enable_dot = 0;

	return std::make_tuple(highlight_color + color_map[enable_dot], enable_dot & 2);
}

std::tuple<u8, bool> c65_state::draw_extended_background(u8 tile, u8 attr, int yi, int xi, int pixel_width)
{
	const int xm = 7 - ((xi / pixel_width) & 7);
	const int ym = (yi & 7);
	const u8 foreground_color = attr & 0xf;
	const u8 background_color = m_bk_color_clut[tile >> 6] & 0xf;

	u8 highlight_color = 0;
	u8 enable_dot = ((m_char_ptr[(((tile & 0x3f) << 3) + ym) & 0x3fff] >> xm) & 1);
	if ((attr & 0x80) && ym == 7) enable_dot = 1;
	if (attr & 0x40) highlight_color = 16;
	if (attr & 0x20) enable_dot = !enable_dot;
	if (attr & 0x10 && !m_blink_enable) enable_dot = 0;

	return std::make_tuple(highlight_color + ((enable_dot) ? foreground_color : background_color), enable_dot != 0);
}

// TODO: invalid modes essentially draws black but still weights for collision info
std::tuple<u8, bool> c65_state::draw_invalid(u8 tile, u8 attr, int yi, int xi, int pixel_width)
{
	return std::make_tuple(0, false);
}

std::tuple<u8, bool> c65_state::get_tile_pixel(int y, int x)
{
	// TODO: move width as a screen setup
	int pixel_width = (m_control_b & 0x80) ? 1 : 2;
	int columns = 80 / pixel_width;

	int xi = (x >> 3) / pixel_width;
	int yi = (y >> 3);
	uint8_t tile = m_video_ptr[(xi + yi * columns) & 0x3fff];
	uint8_t attr = m_cram[xi + yi * columns];

	return (this->*draw_tile_table[m_gfxmode])(tile, attr, y, x, pixel_width);
}

std::tuple<u8, u8, u8, bool> c65_state::get_sprite_pixel(int y, int x)
{
	if (!m_sprite_enable)
		return std::make_tuple(0, 0, 0, false);

	u8 enable_dot = 0;
	u8 sprite_mask = 0;
	u8 idx = 0;
	// TODO: move masking outside this function, bitplane mode should also affect this
	const u8 *sprite_ptr = &m_video_ptr[BIT(m_control_b, 7) ? 0x7f8 : 0x3f8];

	// sprite #7 < #6 < ... < #0
	for (int i = 7; i >= 0; i--)
	{
		if (!BIT(m_sprite_enable, i))
			continue;

		// NOTE: "0 +" intentional for 400i mode, eventually.
		const int y_width = 0 + BIT(m_sexy, i);
		const int ysize = 21 << y_width;
		const u16 yi = m_sprite[i].y;

		if (!(y >= yi && y < yi + ysize))
			continue;

		const int x_width = 1 + BIT(m_sexx, i);
		const int xsize = 24 << x_width;
		const u16 xi = m_sprite[i].x << 1;

		if (!(x >= xi && x < xi + xsize))
			continue;

		u32 sprite_offset = sprite_ptr[i] << 6;
		const int xm = (x - xi) >> x_width;
		const int ym = (y - yi) >> y_width;

		// TODO: not using m_workram here breaks isoccer boot in gfxmode=0, investigate
		u8 sprite_data = m_video_ptr[(ym * 3) + (xm >> 3) + sprite_offset];
		const bool is_multicolor = bool(BIT(m_scm, i));
		const u8 dot_mask = is_multicolor << 1 | 1;
		const u8 shift_mask = 7 - is_multicolor;
		const u8 color_shift = !is_multicolor;
		u8 sprite_dot = (sprite_data >> (shift_mask - (xm & shift_mask))) & dot_mask;
		if (sprite_dot)
		{
			const std::array<u8, 4> color_map = { 0, m_sprite_multicolor_clut[0], m_sprite[i].clut, m_sprite_multicolor_clut[1] };
			sprite_mask |= 1 << i;
			idx = i;
			enable_dot = color_map[sprite_dot << color_shift];
		}
	}
	return std::make_tuple(enable_dot, sprite_mask, idx, enable_dot != 0);
}

TIMER_CALLBACK_MEMBER(c65_state::scanline_cb)
{
	int y = param;
	uint16_t *p = &m_bitmap.pix(y);
	//popmessage("%02x %02x %02x", m_sprite_enable, m_sprite[0].x, m_sprite[0].y);

	const int border_left = 0;
	const int active_left = 24;
	const int active_right = 640 + active_left;
	const int border_right = active_right + active_left;
	const int active_top = 30;
	const int active_bottom = 200 + active_top;

	int x = border_left;

	if (!m_blnk)
	{
		// TODO: blank color
		for (x = border_left; x < border_right; x++)
			p[x] = 0;
	}
	else
	{
		if (y < active_top || y >= active_bottom)
		{
			// TODO: $3fff "opening the border" stuff
			for (x = border_left; x < border_right; x++)
				p[x] = m_border_color;
		}
		else
		{
			const int width80 = BIT(m_control_b, 7) ^ 1;
			for (x = border_left; x < active_left; x++)
				p[x] = m_border_color;
			for (;x < active_right; x++)
			{
				u8 tile_dot, sprite_dot, sprite_mask, sprite_active;
				bool is_foreground, is_sprite;
				// NOTE: VIC-II and VIC-III can switch mid-frame, but latches occur in 8 scanline steps
				// at least from/to a base C=64 tilemap mode, also cfr. "bad lines".
				std::tie(tile_dot, is_foreground) = get_tile_pixel(y - active_top, x - active_left - (m_xscl << width80));
				// HACK: are sprite positions in native coordinates from the border?
				std::tie(sprite_dot, sprite_mask, sprite_active, is_sprite) = get_sprite_pixel(y + 20, x + active_left);

				// TODO: collisions
				//m_ssc |= sprite_mask;
				// TODO: reduce this
				if (is_foreground && !(BIT(sprite_mask, sprite_active) & (BIT(m_bsp ^ 0xff, sprite_active))))
					p[x] = m_palette->pen(tile_dot);
				else
					p[x] = m_palette->pen(is_sprite ? sprite_dot : tile_dot);
			}
			for (;x < border_right; x++)
				p[x] = m_border_color;
		}
	}

	// HACK: need to compensate, cfr. princess
	if (y == m_rcr - 21)
		irq_check(1);

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

void c65_state::palette_entry_flush(uint8_t offset)
{
	m_palette->set_pen_color(offset, pal4bit(m_palred[offset]), pal4bit(m_palgreen[offset]), pal4bit(m_palblue[offset]));
}

void c65_state::palette_red_w(offs_t offset, uint8_t data)
{
	// TODO: bit 4 for FG/BG, superimposing?
	m_palred[offset] = data;
	palette_entry_flush(offset);
}

void c65_state::palette_green_w(offs_t offset, uint8_t data)
{
	m_palgreen[offset] = data;
	palette_entry_flush(offset);
}

void c65_state::palette_blue_w(offs_t offset, uint8_t data)
{
	m_palblue[offset] = data;
	palette_entry_flush(offset);
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
	uint8_t res = 0xff;

	// joystick
	uint8_t joy_b = m_joy[1]->read_joy();

	res &= (0xf0 | (joy_b & 0x0f));
	res &= ~(!BIT(joy_b, 5) << 4);

	return res;

}

uint8_t c65_state::cia0_portb_r()
{
	static const char *const c64ports[] = { "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7" };
	static const char *const c65ports[] = { "C8", "C9" };
	uint8_t res;

	res = 0xff;
	uint8_t joy_a = m_joy[0]->read_joy();

	res &= (0xf0 | (joy_a & 0x0f));
	res &= ~(!BIT(joy_a, 5) << 4);

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
	m_joy[1]->joy_w(data & 0x1f);
//  logerror("%02x\n",m_keyb_c0_c7);
}

void c65_state::cia0_portb_w(uint8_t data)
{
	m_joy[0]->joy_w(data & 0x1f);

}

/*
 * xx-- ---- serial bus input
 * --xx x--- serial bus output
 * ---- -x-- RS-232 TXD output
 * ---- --xx VIC bank base
 */
void c65_state::cia1_porta_w(uint8_t data)
{
	m_vic_bank_base = ~(data & 0x3) << 14;
	flush_cb_base();
}

void c65_state::c65_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x07fff).rw(FUNC(c65_state::ram_r<0x00000>), FUNC(c65_state::ram_w<0x00000>));

	map(0x08000, 0x0bfff).view(m_exrom_view);
	m_exrom_view[0](0x08000, 0x0bfff).r(m_cart_exp, FUNC(generic_slot_device::read_rom));
	m_exrom_view[1](0x08000, 0x09fff).rw(FUNC(c65_state::ram_r<0x08000>), FUNC(c65_state::ram_w<0x08000>));
	m_exrom_view[1](0x0a000, 0x0bfff).rom().region("ipl", 0x0a000);

	map(0x0c000, 0x0cfff).view(m_romc_view);
	m_romc_view[0](0x0c000, 0x0cfff).rw(FUNC(c65_state::ram_r<0x0c000>), FUNC(c65_state::ram_w<0x0c000>));
	m_romc_view[1](0x0c000, 0x0cfff).rom().region("ipl", 0x0c000);

	map(0x0d000, 0x0dfff).view(m_charen_view);
	m_charen_view[0](0x0d000, 0x0dfff).rom().region("ipl", 0x0d000);
	m_charen_view[1](0x0d000, 0x0d07f).m(*this, FUNC(c65_state::vic4567_map));
	// 0x0d080, 0x0d09f FDC
	m_charen_view[1](0x0d080, 0x0d09f).lr8(NAME([] (offs_t offset) { return 0; }));
	// 0x0d0a0, 0x0d0ff Ram Expansion Control (REC)
	m_charen_view[1](0x0d100, 0x0d1ff).ram().w(FUNC(c65_state::palette_red_w)).share("redpal");
	m_charen_view[1](0x0d200, 0x0d2ff).ram().w(FUNC(c65_state::palette_green_w)).share("greenpal");
	m_charen_view[1](0x0d300, 0x0d3ff).ram().w(FUNC(c65_state::palette_blue_w)).share("bluepal");
	// 0x0d400, 0x0d4*f Right SID
	// keyboard hold left shift will read to $d484 (?)
	m_charen_view[1](0x0d400, 0x0d41f).mirror(0x80).rw(m_sid[1], FUNC(mos6581_device::read), FUNC(mos6581_device::write));
	// 0x0d440, 0x0d4*f Left  SID
	m_charen_view[1](0x0d440, 0x0d45f).mirror(0x80).rw(m_sid[0], FUNC(mos6581_device::read), FUNC(mos6581_device::write));
	m_charen_view[1](0x0d600, 0x0d6ff).rw(FUNC(c65_state::uart_r), FUNC(c65_state::uart_w));
	// 0x0d700, 0x0d7** DMAgic
	m_charen_view[1](0x0d700, 0x0d703).m(m_dma, FUNC(dmagic_f018_device::map));
	// 0x0d800, 0x0d8** Color matrix
	m_charen_view[1](0x0d800, 0x0dfff).view(m_cram_view);
	// maps lower 1024 bytes regardless of the setting (essentially touches $dc00 as overlay)
	m_cram_view[0](0x0d800, 0x0dbff).lrw8(
		NAME([this] (offs_t offset) { return m_cram[offset]; }),
		NAME([this] (offs_t offset, u8 data) { m_cram[offset] = data; })
	);
	m_cram_view[0](0x0dc00, 0x0dc0f).rw(m_cia[0], FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	m_cram_view[0](0x0dd00, 0x0dd0f).rw(m_cia[1], FUNC(mos6526_device::read), FUNC(mos6526_device::write));
	// 0x0de00, 0x0de** Ext I/O Select 1 (IO1)
	// 0x0df00, 0x0df** Ext I/O Select 2 (IO2)
	m_cram_view[1](0x0d800, 0x0dfff).ram().share("cram");

	map(0x0e000, 0x0ffff).rom().region("ipl", 0x0e000);

	map(0x10000, 0x1f7ff).rw(FUNC(c65_state::ram_r<0x010000>), FUNC(c65_state::ram_w<0x10000>));
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
	m_control_a = 0;
	m_ssc = m_sbc = 0;
	m_cram_view.select(0);
	m_exrom_view.select(m_cart_exp->exists() ^ 1);
}


void c65_state::palette_init(palette_device &palette)
{
	// HACK: should read from an (undumped) PROM, and switch when bit 2 of control A is high
	static const u8 r_default[] = {
		0x00, 0x0f, 0x0f, 0x00,
		0x0f, 0x00, 0x00, 0x0f,
		0x0f, 0x0a, 0x0f, 0x05,
		0x08, 0x09, 0x09, 0x0b
	};
	static const u8 g_default[] = {
		0x00, 0x0f, 0x00, 0x0f,
		0x00, 0x0f, 0x00, 0x0f,
		0x06, 0x04, 0x07, 0x05,
		0x08, 0x0f, 0x09, 0x0b
	};
	static const u8 b_default[] = {
		0x00, 0x0f, 0x00, 0x0f,
		0x0f, 0x00, 0x0f, 0x00,
		0x00, 0x00, 0x07, 0x05,
		0x08, 0x09, 0x0f, 0x0b
	};
	int i;

	for (i = 0; i < 0x10; i++)
	{
		m_palred[i] = r_default[i];
		m_palgreen[i] = g_default[i];
		m_palblue[i] = b_default[i];
	}

	for (i = 0; i < 0x100; i++)
		palette_entry_flush(i);
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

void c65_state::irq_check(uint8_t irq_cause)
{
	m_irq_pending |= (irq_cause != 0) ? 0x80 : 0x00;
	m_irq_pending |= irq_cause;

	m_irqs->in_w<1>(m_irq_mask & m_irq_pending);
}

uint8_t c65_state::cpu_r()
{
	return 0x07;
}

void c65_state::cpu_w(uint8_t data)
{
//  m_loram = BIT(data, 0);
//  m_hiram = BIT(data, 1);
	m_charen_view.select(BIT(data, 2));
}


void c65_state::c65(machine_config &config)
{
	M4510(config, m_maincpu, MAIN_C65_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &c65_state::c65_map);
	m_maincpu->read_callback().set(FUNC(c65_state::cpu_r));
	m_maincpu->write_callback().set(FUNC(c65_state::cpu_w));
	m_maincpu->set_pulls(0x07, 0xc0);

	// TODO: multiply by x8 because with a more canonical x1 no transfer will complete in time.
	// is this thing a mixed burst/cycle steal really?
	DMAGIC_F018(config, m_dma, MAIN_C65_CLOCK * 8);
	m_dma->set_space(m_maincpu, AS_PROGRAM);

	input_merger_device &irq(INPUT_MERGER_ANY_HIGH(config, "irqs"));
	irq.output_handler().set_inputline(m_maincpu, M4510_IRQ_LINE);

	INPUT_MERGER_ANY_HIGH(config, m_nmis);
	m_nmis->output_handler().set_inputline(m_maincpu, M4510_NMI_LINE);

	MOS6526(config, m_cia[0], MAIN_C65_CLOCK);
	m_cia[0]->set_tod_clock(60);
	m_cia[0]->irq_wr_callback().set("irqs", FUNC(input_merger_device::in_w<0>));
	m_cia[0]->pa_rd_callback().set(FUNC(c65_state::cia0_porta_r));
	m_cia[0]->pa_wr_callback().set(FUNC(c65_state::cia0_porta_w));
	m_cia[0]->pb_rd_callback().set(FUNC(c65_state::cia0_portb_r));
	m_cia[0]->pb_wr_callback().set(FUNC(c65_state::cia0_portb_w));

	MOS6526(config, m_cia[1], MAIN_C65_CLOCK);
	m_cia[1]->set_tod_clock(60);
	m_cia[1]->irq_wr_callback().set(m_nmis, FUNC(input_merger_device::in_w<0>));
//  m_cia[1]->pa_rd_callback().set(FUNC(c65_state::c65_cia1_port_a_r));
	m_cia[1]->pa_wr_callback().set(FUNC(c65_state::cia1_porta_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(c65_state::screen_update));
	// TODO: stub parameters
	// C=64 / width 40 modes should actually be running in 320x200
	m_screen->set_raw(MAIN_C65_CLOCK*4, 910, 0, 640+24 * 2, 262, 0, 200+30*2);
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_c65);

	PALETTE(config, m_palette, FUNC(c65_state::palette_init), 0x100);

	SPEAKER(config, "speaker", 2).front();
	// 8580 SID
	MOS6581(config, m_sid[0], MAIN_C64_CLOCK);
	//m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	//m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid[0]->add_route(ALL_OUTPUTS, "speaker", 0.50, 0);

	MOS6581(config, m_sid[1], MAIN_C64_CLOCK);
	//m_sid->potx().set(FUNC(c64_state::sid_potx_r));
	//m_sid->poty().set(FUNC(c64_state::sid_poty_r));
	m_sid[1]->add_route(ALL_OUTPUTS, "speaker", 0.50, 1);

	VCS_CONTROL_PORT(config, m_joy[0], vcs_control_port_devices, "joy");
	//m_joy1->trigger_wr_callback().set(MOS6567_TAG, FUNC(mos6567_device::lp_w));
	VCS_CONTROL_PORT(config, m_joy[1], vcs_control_port_devices, "joy");

	GENERIC_CARTSLOT(config, m_cart_exp, generic_plain_slot, "c64_cart");

	SOFTWARE_LIST(config, "flop_list").set_original("c65_flop");
//    SOFTWARE_LIST(config, "cart_list").set_compatible("c64_cart");
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
