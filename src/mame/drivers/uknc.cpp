// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

UKNC (Educational Computer by Scientific Centre) PDP-11 clone.
Also known as Elektronika MS-0511.
RAM = 192K (CPU 1 = 64K, CPU 2 = 32K, Videoram = 96K), ROM = 32K.
Graphics 640x288 pixels.

2009-05-12 Skeleton driver.

Status: snapshot of work in progress.  needs real VM2 CPU core.

****************************************************************************/

#include "emu.h"

#include "bus/qbus/qbus.h"
#include "cpu/t11/t11.h"
#include "imagedev/cassette.h"
#include "machine/bankdev.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"


// video parameters
static constexpr int UKNC_TOTAL_HORZ = 800;
static constexpr int UKNC_DISP_HORZ = 640;
static constexpr int UKNC_HORZ_START = 80;

static constexpr int UKNC_TOTAL_VERT = 312;
static constexpr int UKNC_DISP_VERT = 288 + 18;
static constexpr int UKNC_VERT_START = 0;


typedef struct
{
	uint16_t addr;
	bool cursor;
	uint32_t aux;
	bool color;
	bool control;
	bool cursor_type;
	int cursor_color;
	int cursor_octet;
	int cursor_pixel;
} uknc_scanline;


class uknc_state : public driver_device
{
public:
	uknc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_bankdev0(*this, "bankdev0")
		, m_bankdev1(*this, "bankdev1")
		, m_bankdev2(*this, "bankdev2")
		, m_qbus(*this, "qbus")
		, m_subqbus(*this, "cart")
		, m_cassette(*this, "cassette")
		, m_yrgb(*this, "yrgb")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
	{ }

	void uknc(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);

	void uknc_mem(address_map &map);
	void uknc_sub_mem(address_map &map);
	void uknc_mem_banked(address_map &map);
	void uknc_cart_mem(address_map &map);
	void uknc_rom_mem(address_map &map);

	uint16_t trap_r();
	void trap_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t maincpu_vram_addr_r();
	void maincpu_vram_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t maincpu_vram_data_r();
	void maincpu_vram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t bcsr_r();
	void bcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t scsr_r();
	void scsr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t vram_addr_r();
	void vram_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vram_data_pl0_r();
	void vram_data_pl0_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vram_data_p12_r();
	void vram_data_p12_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sprite_r(offs_t offset);
	void sprite_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// subcpu registers
	uint16_t m_bcsr; // 177054
	uint16_t m_scsr; // 177716

	// video
	struct
	{
		uint16_t vram_addr;
		uint16_t vram_data_pl0;
		uint16_t vram_data_p12;
		uint16_t maincpu_vram_addr;
		uint16_t maincpu_vram_data;
		uint8_t foreground;  // 177016
		uint32_t background; // 177020
		uint8_t sprite_mask; // 177024
		uint8_t plane_mask;  // 177026
		uint32_t control;
		uint32_t palette;
	} m_video;
	std::unique_ptr<uint8_t[]> m_videoram;
	std::unique_ptr<uknc_scanline[]> m_scanlines;
	bitmap_rgb32 m_tmpbmp;

	void uknc_palette(palette_device &palette) const;
	void set_palette_yrgb(uint32_t aux);
	void set_palette_ygrb(uint32_t aux);
	void set_palette__rgb(uint32_t aux);
	void set_palette_gray(uint32_t aux);
	void update_displaylist();
	void draw_scanline(uint32_t *p, uknc_scanline *scanline);

protected:
	required_device<k1801vm2_device> m_maincpu;
	required_device<k1801vm2_device> m_subcpu;
	required_device<address_map_bank_device> m_bankdev0;
	required_device<address_map_bank_device> m_bankdev1;
	required_device<address_map_bank_device> m_bankdev2;
	required_device<qbus_device> m_qbus;
	required_device<qbus_device> m_subqbus;
	required_device<cassette_image_device> m_cassette;
	required_ioport m_yrgb;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};


void uknc_state::uknc_mem(address_map &map)
{
	map(0000000, 0177777).m(m_bankdev0, FUNC(address_map_bank_device::amap16));
}

void uknc_state::uknc_mem_banked(address_map &map)
{
// USER mode
	map(0000000, 0157777).ram().mirror(0200000);
//	map(0176560, 0176567).rw(m_dl11lan, FUNC(dl11_device::read), FUNC(dl11_device::write));
//	map(0176570, 0176577).rw(m_dl11com, FUNC(dl11_device::read), FUNC(dl11_device::write));
	map(0176640, 0176641).rw(FUNC(uknc_state::maincpu_vram_addr_r), FUNC(uknc_state::maincpu_vram_addr_w));
	map(0176642, 0176643).rw(FUNC(uknc_state::maincpu_vram_data_r), FUNC(uknc_state::maincpu_vram_data_w));
	map(0176644, 0176647).unmaprw(); // hardware breakpoint
//	map(0176660, 0176667).rw("channel", FUNC(vp1_120_pri_device::read1), FUNC(vp1_120_pri_device::write1));
//	map(0176670, 0176677).rw("channel", FUNC(vp1_120_pri_device::read2), FUNC(vp1_120_pri_device::write2));
//	map(0177560, 0177567).rw("channel", FUNC(vp1_120_pri_device::read0), FUNC(vp1_120_pri_device::write0)); // terminal emulator

// HALT mode
	map(0360000, 0377777).ram().share("sram");

// bus error handler
//	map(0000000, 0377777).rw(FUNC(uknc_state::trap_r), FUNC(uknc_state::trap_w));
}

static const z80_daisy_config daisy_chain[] =
{
//	{ "channel" },
//	{ "trap" },
//	{ "dl11com" },
	{ "qbus" },
	{ nullptr }
};

void uknc_state::uknc_sub_mem(address_map &map)
{
	map(0000000, 0077777).ram();
	map(0100000, 0117777).m(m_bankdev1, FUNC(address_map_bank_device::amap16)); // cart_mem
	map(0120000, 0176777).m(m_bankdev2, FUNC(address_map_bank_device::amap16)); // rom_mem
	map(0177010, 0177011).rw(FUNC(uknc_state::vram_addr_r), FUNC(uknc_state::vram_addr_w));
	map(0177012, 0177013).rw(FUNC(uknc_state::vram_data_pl0_r), FUNC(uknc_state::vram_data_pl0_w));
	map(0177014, 0177015).rw(FUNC(uknc_state::vram_data_p12_r), FUNC(uknc_state::vram_data_p12_w));
	map(0177016, 0177027).rw(FUNC(uknc_state::sprite_r), FUNC(uknc_state::sprite_w));
	map(0177054, 0177055).rw(FUNC(uknc_state::bcsr_r), FUNC(uknc_state::bcsr_w));
//	map(0177060, 0177077).rw("subchan", FUNC(vp1_120_sub_device::read), FUNC(vp1_120_sub_device::write));
//	map(0177100, 0177103).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write)).mask(0xffff);
//	map(0177700, 0177707).rw("keyboard", FUNC(xm1_031_kbd_device::read), FUNC(xm1_031_kbd_device::write));
//	map(0177710, 0177715).rw("timer", FUNC(xm1_031_timer_device::read), FUNC(xm1_031_timer_device::write));
	map(0177716, 0177717).rw(FUNC(uknc_state::scsr_r), FUNC(uknc_state::scsr_w));
}

/*
 *	bit 0 of map number = value of bit 0 in BCSR
 *	  0		mapping at 100000: 0 - read from VRAM plane 0, 1 - read from ROM (reset to 1)
 *	bit 1 of map number = value of bit 4 in BCSR (ROM banks replacement: 0 - writes ignored, 1 - writes go to VRAM plane 0)
 *	  4		100000..117777
 */
void uknc_state::uknc_cart_mem(address_map &map)
{
// map 0
	map(0000000, 0017777).bankr("plane0bank4");
// map 1 (default) -- read from system ROM, writes ignored
	map(0100000, 0117777).rom().region("subcpu", 0);
// map 2
	map(0200000, 0217777).bankrw("plane0bank4");
// map 3
	map(0300000, 0317777).rom().region("subcpu", 0).bankw("plane0bank4");
}

/*
 *	bits 0-2 of map number = value of bits 5-7 in BCSR (ROM banks replacement: 0 - writes ignored, 1 - writes go to VRAM plane 0)
 *	  5		120000..137777
 *	  6		140000..157777
 *	  7		160000..177777
 */
void uknc_state::uknc_rom_mem(address_map &map)
{
// map 0: all ROM, no VRAM
	map(0000000, 0057777).rom().region("subcpu", 020000);
// map 1
	map(0100000, 0117777).rom().region("subcpu", 020000).bankw("plane0bank5");
	map(0120000, 0157777).rom().region("subcpu", 040000);
// map 2
	map(0200000, 0217777).rom().region("subcpu", 020000);
	map(0220000, 0237777).rom().region("subcpu", 040000).bankw("plane0bank6");
	map(0240000, 0257777).rom().region("subcpu", 060000);
// map 3
	map(0300000, 0317777).rom().region("subcpu", 020000).bankw("plane0bank5");
	map(0320000, 0337777).rom().region("subcpu", 040000).bankw("plane0bank6");
	map(0340000, 0357777).rom().region("subcpu", 060000);
// map 4
	map(0400000, 0437777).rom().region("subcpu", 020000);
	map(0440000, 0457777).rom().region("subcpu", 060000).bankw("plane0bank7");
// map 5
	map(0500000, 0517777).rom().region("subcpu", 020000).bankw("plane0bank5");
	map(0520000, 0537777).rom().region("subcpu", 040000);
	map(0540000, 0557777).rom().region("subcpu", 060000).bankw("plane0bank7");
// map 6
	map(0600000, 0617777).rom().region("subcpu", 020000);
	map(0620000, 0637777).rom().region("subcpu", 040000).bankw("plane0bank6");
	map(0640000, 0657777).rom().region("subcpu", 060000).bankw("plane0bank7");
// map 7
	map(0700000, 0717777).rom().region("subcpu", 020000).bankw("plane0bank5");
	map(0720000, 0737777).rom().region("subcpu", 040000).bankw("plane0bank6");
	map(0740000, 0757777).rom().region("subcpu", 060000).bankw("plane0bank7");
}

static const z80_daisy_config sub_daisy_chain[] =
{
//	{ "event" },
//	{ "timer" },
//	{ "keyboard" },
//	{ "reset" },
//	{ "subchan" },
	{ "cart" },
	{ nullptr }
};

/* Input ports */

// some models had R and G outputs swapped and others had no Y (intensity) output
static INPUT_PORTS_START( uknc )
	PORT_START("yrgb")
	PORT_DIPNAME(0x03, 0x00, "variant")
	PORT_DIPSETTING(0x00, "Color, YRGB")
	PORT_DIPSETTING(0x01, "Color, YGRB")
	PORT_DIPSETTING(0x02, "Color, xRGB")
	PORT_DIPSETTING(0x03, "Grayscale")
INPUT_PORTS_END


void uknc_state::machine_reset()
{
	bcsr_w(0, 01401, 0xffff);
	scsr_w(0, 0, 0xffff);
}

void uknc_state::machine_start()
{
	// 3 bitplanes
	m_videoram = std::make_unique<uint8_t[]>(32768 * 3);
	m_scanlines = std::make_unique<uknc_scanline[]>(UKNC_DISP_VERT);
	m_tmpbmp.allocate(UKNC_DISP_HORZ, UKNC_DISP_VERT);

	membank("plane0bank4")->set_base(&m_videoram[0]);
	membank("plane0bank5")->set_base(&m_videoram[020000]);
	membank("plane0bank6")->set_base(&m_videoram[040000]);
	membank("plane0bank7")->set_base(&m_videoram[060000]);
}

void uknc_state::uknc_palette(palette_device &palette) const
{
	// for debugging only, will be overwritten by firmware
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, rgb_t(pal3bit(i), pal3bit(i), pal3bit(i)));
	}
}


WRITE_LINE_MEMBER(uknc_state::screen_vblank)
{
// requires real VM2 cpu core
#if 0
	if (!BIT(m_bcsr, 8))
		m_subcpu->set_input_line(INPUT_LINE_EVNT, ASSERT_LINE);
	if (!BIT(m_bcsr, 9))
		m_maincpu->set_input_line(INPUT_LINE_EVNT, ASSERT_LINE);
#endif
}

// VRAM access from maincpu

uint16_t uknc_state::maincpu_vram_addr_r()
{
	return m_video.maincpu_vram_addr;
}

void uknc_state::maincpu_vram_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_video.maincpu_vram_addr = data;
	if (data < 0100000)
	{
		m_video.maincpu_vram_data = m_bankdev0->space(AS_PROGRAM).read_word(0200000 + (data << 1));
	}
	else
	{
		m_video.maincpu_vram_data = m_videoram[data] | (m_videoram[data + 0100000] << 8);
	}
}

uint16_t uknc_state::maincpu_vram_data_r()
{
	return m_video.maincpu_vram_data;
}

void uknc_state::maincpu_vram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video.maincpu_vram_data);
	if (m_video.maincpu_vram_addr < 0100000)
	{
		m_bankdev0->space(AS_PROGRAM).write_word(0200000 + (m_video.maincpu_vram_addr << 1), m_video.maincpu_vram_data);
	}
	else
	{
		if (ACCESSING_BITS_0_7)
		{
			m_videoram[m_video.maincpu_vram_addr] = data & 0xff;
		}
		if (ACCESSING_BITS_8_15)
		{
			m_videoram[m_video.maincpu_vram_addr + 0100000] = (data >> 8) & 0xff;
		}
	}
}

// VRAM access from subcpu

uint16_t uknc_state::vram_addr_r()
{
	return m_video.vram_addr;
}

void uknc_state::vram_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_video.vram_addr = data;
	if (data < 0100000)
	{
		m_video.vram_data_p12 = m_bankdev0->space(AS_PROGRAM).read_word(0200000 + (data << 1));
	}
	else
	{
		m_video.vram_data_p12 = m_videoram[data] | (m_videoram[data + 0100000] << 8);
		m_video.vram_data_pl0 = m_videoram[data - 0100000];
	}
}

uint16_t uknc_state::vram_data_pl0_r()
{
	return m_video.vram_data_pl0;
}

uint16_t uknc_state::vram_data_p12_r()
{
	return m_video.vram_data_p12;
}

void uknc_state::vram_data_pl0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_videoram[m_video.vram_addr - 0100000] = data;
	COMBINE_DATA(&m_video.vram_data_pl0);
}

void uknc_state::vram_data_p12_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video.vram_data_p12);
	if (m_video.vram_addr < 0100000)
	{
		m_bankdev0->space(AS_PROGRAM).write_word(0200000 + (m_video.vram_addr << 1), m_video.vram_data_p12);
	}
	else
	{
		if (ACCESSING_BITS_0_7)
		{
			m_videoram[m_video.vram_addr] = data & 0xff;
		}
		if (ACCESSING_BITS_8_15)
		{
			m_videoram[m_video.vram_addr + 0100000] = (data >> 8) & 0xff;
		}
	}
}

uint16_t uknc_state::sprite_r(offs_t offset)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_video.foreground;
		break;

	case 1:
		data = m_video.background;
		break;

	case 2:
		data = (m_video.background >> 16);
		break;

	case 3:
		// vram read
		if (m_video.vram_addr >= 0100000)
		{
			uint8_t vdata;

			// plane 0
			vdata = m_videoram[m_video.vram_addr - 0100000];
			m_video.background = BIT(vdata, 0);
			m_video.background |= BIT(vdata, 1) << 4;
			m_video.background |= BIT(vdata, 2) << 8;
			m_video.background |= BIT(vdata, 3) << 12;
			m_video.background |= BIT(vdata, 4) << 16;
			m_video.background |= BIT(vdata, 5) << 20;
			m_video.background |= BIT(vdata, 6) << 24;
			m_video.background |= BIT(vdata, 7) << 28;

			// plane 1
			vdata = m_videoram[m_video.vram_addr];
			m_video.background |= BIT(vdata, 0) << 1;
			m_video.background |= BIT(vdata, 1) << 5;
			m_video.background |= BIT(vdata, 2) << 9;
			m_video.background |= BIT(vdata, 3) << 13;
			m_video.background |= BIT(vdata, 4) << 17;
			m_video.background |= BIT(vdata, 5) << 21;
			m_video.background |= BIT(vdata, 6) << 25;
			m_video.background |= BIT(vdata, 7) << 29;

			// plane 2
			vdata = m_videoram[m_video.vram_addr + 0100000];
			m_video.background |= BIT(vdata, 0) << 2;
			m_video.background |= BIT(vdata, 1) << 6;
			m_video.background |= BIT(vdata, 2) << 10;
			m_video.background |= BIT(vdata, 3) << 14;
			m_video.background |= BIT(vdata, 4) << 18;
			m_video.background |= BIT(vdata, 5) << 22;
			m_video.background |= BIT(vdata, 6) << 26;
			m_video.background |= BIT(vdata, 7) << 30;
		}
		else
		{
			logerror("sprite_r vram addr UNIMP: %06o\n", m_video.vram_addr);
		}
		break;

	case 4:
		data = m_video.plane_mask;
		break;
	}

	return data;
}

void uknc_state::sprite_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset)
	{
	case 0: // 177016
		m_video.foreground = data & 7;
		break;

	case 1: // 177020
		m_video.background &= ~0xffff;
		m_video.background |= data & 0x7777;
		break;

	case 2: // 177022
		m_video.background &= 0xffff;
		m_video.background |= (data & 0x7777) << 16;
		break;

	case 3: // 177024
		m_video.sprite_mask = data;
		if (!BIT(m_video.plane_mask, 0))
		{
			uint8_t vdata;

			vdata = (BIT(data, 0) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 0));
			vdata |= (BIT(data, 1) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 4)) << 1;
			vdata |= (BIT(data, 2) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 8)) << 2;
			vdata |= (BIT(data, 3) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 12)) << 3;
			vdata |= (BIT(data, 4) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 16)) << 4;
			vdata |= (BIT(data, 5) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 20)) << 5;
			vdata |= (BIT(data, 6) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 24)) << 6;
			vdata |= (BIT(data, 7) ? BIT(m_video.foreground, 0) : BIT(m_video.background, 28)) << 7;
			m_videoram[m_video.vram_addr - 0100000] = vdata;
		}
		if (!BIT(m_video.plane_mask, 1))
		{
			uint8_t vdata;

			vdata = (BIT(data, 0) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 1));
			vdata |= (BIT(data, 1) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 5)) << 1;
			vdata |= (BIT(data, 2) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 9)) << 2;
			vdata |= (BIT(data, 3) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 13)) << 3;
			vdata |= (BIT(data, 4) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 17)) << 4;
			vdata |= (BIT(data, 5) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 21)) << 5;
			vdata |= (BIT(data, 6) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 25)) << 6;
			vdata |= (BIT(data, 7) ? BIT(m_video.foreground, 1) : BIT(m_video.background, 29)) << 7;
			m_videoram[m_video.vram_addr] = vdata;
		}
		if (!BIT(m_video.plane_mask, 2))
		{
			uint8_t vdata;

			vdata = (BIT(data, 0) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 2));
			vdata |= (BIT(data, 1) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 6)) << 1;
			vdata |= (BIT(data, 2) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 10)) << 2;
			vdata |= (BIT(data, 3) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 14)) << 3;
			vdata |= (BIT(data, 4) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 18)) << 4;
			vdata |= (BIT(data, 5) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 22)) << 5;
			vdata |= (BIT(data, 6) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 26)) << 6;
			vdata |= (BIT(data, 7) ? BIT(m_video.foreground, 2) : BIT(m_video.background, 30)) << 7;
			m_videoram[m_video.vram_addr + 0100000] = vdata;
		}
		break;

	case 4: // 177026
		m_video.plane_mask = data & 7;
		break;
	}
}

// system registers

/*
 *	00	R	cassette data in
 *	01	RW	cassette data out
 *	02	RW	cassette data gate
 *	03	RW	cassette/external_event/index gate.  0: cassette  1: ???
 *	04	RW	maincpu USER/HALT mode. 0: USER, 1: HALT
 *	05	RW	maincpu DCLO pin
 *	06	-
 *	07	RW	sound data out, if 08-12 == 0
 *	12:08	sound frequency matrix
 *	13	RW	reserved
 *	14	-
 *	15	RW	maincpu ACLO pin
 */
uint16_t uknc_state::scsr_r()
{
	double tap_val = m_cassette->input();

	m_scsr = (m_scsr & ~1) | (tap_val < 0 ? 1 : 0);

	return m_scsr;
}

void uknc_state::scsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_scsr = data;

	if (BIT(data, 2)) m_cassette->output(BIT(data, 1) ? -1 : 1);

// requires real VM2 cpu core
#if 0
	m_maincpu->set_input_line(INPUT_LINE_1801HALT, BIT(data, 4) ? ASSERT_LINE : CLEAR_LINE);

	if (BIT(data, 15) && m_maincpu->suspended(SUSPEND_REASON_DISABLE))
	{
		m_maincpu->resume(SUSPEND_REASON_DISABLE);
	}
	else if (!BIT(data, 15) && !m_maincpu->suspended(SUSPEND_REASON_DISABLE))
	{
		m_maincpu->set_input_line(INPUT_LINE_ACLO, ASSERT_LINE);
	}
#endif
}

/*
 *	0-3	CE0..CE3 bus signals
 *	  CE0	mapping at 100000: 0 - read from VRAM plane 0, 1 - read from ROM (reset to 1)
 *	  CE1-2	ROM carts: bank selector
 *	  CE3	slot number
 *	4-7	ROM banks replacement: 0 - writes ignored, 1 - writes go to VRAM plane 0
 *	  4		100000..117777
 *	  5		120000..137777
 *	  6		140000..157777
 *	  7		160000..177777
 *	8	line clock interrupt (subcpu): 0 - enable
 *	9	line clock interrupt (maincpu): 0 - enable
 */
uint16_t uknc_state::bcsr_r()
{
	return m_bcsr;
}

void uknc_state::bcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int bank1, bank2;
	address_space *subspace = &m_subcpu->space(AS_PROGRAM);

	bank1 = (data & 1) | ((data >> 3) & 1);
	bank2 = (data >> 5) & 7;

	// order matters: ROM on device may unmap itself and we restore the mapping
//	m_subqbus->ce0_ce3_w(data & 15);

	if (BIT(m_bcsr ^ data, 0) && BIT(data, 0))
	{
		subspace->unmap_readwrite(0100000, 0117777);
		subspace->install_readwrite_handler(0100000, 0117777,
			read16s_delegate(m_bankdev1, FUNC(address_map_bank_device::read16)),
			write16s_delegate(m_bankdev1, FUNC(address_map_bank_device::write16)));
	}

	m_bankdev1->set_bank(bank1);
	m_bankdev2->set_bank(bank2);

	m_bcsr = data;
}

// video

#define YRGB(rgb, y, p) ((rgb)*42 * (3 + (p) + (y)*2))

void uknc_state::set_palette_yrgb(uint32_t aux)
{
	for (int i = 0; i < 8; i++)
	{
		m_palette->set_pen_color(i, rgb_t(
			YRGB(BIT(aux, i * 4 + 2), BIT(aux, i * 4 + 3), BIT(m_video.control, 18)),
			YRGB(BIT(aux, i * 4 + 1), BIT(aux, i * 4 + 3), BIT(m_video.control, 17)),
			YRGB(BIT(aux, i * 4    ), BIT(aux, i * 4 + 3), BIT(m_video.control, 16)))
		);
	}
}

void uknc_state::set_palette_ygrb(uint32_t aux)
{
	for (int i = 0; i < 8; i++)
	{
		m_palette->set_pen_color(i, rgb_t(
			YRGB(BIT(aux, i * 4 + 1), BIT(aux, i * 4 + 3), BIT(m_video.control, 17)),
			YRGB(BIT(aux, i * 4 + 2), BIT(aux, i * 4 + 3), BIT(m_video.control, 18)),
			YRGB(BIT(aux, i * 4    ), BIT(aux, i * 4 + 3), BIT(m_video.control, 16)))
		);
	}
}

void uknc_state::set_palette__rgb(uint32_t aux)
{
	for (int i = 0; i < 8; i++)
	{
		m_palette->set_pen_color(i, rgb_t(
			YRGB(BIT(aux, i * 4 + 2), !BIT(aux, i * 4 + 3), 1),
			YRGB(BIT(aux, i * 4 + 1), !BIT(aux, i * 4 + 3), 1),
			YRGB(BIT(aux, i * 4    ), !BIT(aux, i * 4 + 3), 1))
		);
	}
}

void uknc_state::set_palette_gray(uint32_t aux)
{
	for (int i = 0; i < 8; i++)
	{
		m_palette->set_pen_color(i, rgb_t(pal3bit(i), pal3bit(i), pal3bit(i)));
	}
}

#undef YGRB


void uknc_state::update_displaylist()
{
	int wide = 0;
	uint16_t addr = 0270, nextaddr;
	bool cursor = false, cursor_type = false;
	int cursor_color = 0, cursor_octet = 0, cursor_pixel = 0;
	uknc_scanline *scanline;

	for (int row = 0; row < UKNC_DISP_VERT; row++)
	{
		scanline = &m_scanlines[row];
		scanline->addr = m_subcpu->space(AS_PROGRAM).read_word(addr);
		nextaddr = m_subcpu->space(AS_PROGRAM).read_word(addr + 2);

		if (BIT(nextaddr, 0))
			cursor = !cursor;

		scanline->control = scanline->color = false;
		scanline->cursor = cursor;

		if (wide)
		{
			scanline->aux = m_subcpu->space(AS_PROGRAM).read_word(addr - 4) |
				(m_subcpu->space(AS_PROGRAM).read_word(addr - 2) << 16);
			if (wide == 1)
			{
				scanline->control = true;
				cursor_type = BIT(scanline->aux, 4);
				cursor_color = scanline->aux & 15;
				cursor_pixel = (scanline->aux >> 4) & 3;
				cursor_octet = (scanline->aux >> 8) & 127;
			}
			else
			{
				scanline->color = true;
			}
		}

		scanline->cursor_type = cursor_type;
		scanline->cursor_color = cursor_color;
		scanline->cursor_pixel = cursor_pixel;
		scanline->cursor_octet = cursor_octet;

		if (BIT(nextaddr, 1))
		{
			wide = BIT(nextaddr, 2) + 1;
			nextaddr &= ~7;
			nextaddr += 4;
		}
		else
		{
			wide = 0;
			nextaddr &= ~3;
		}

		addr = nextaddr;
	}
}

/*
 * aux bits, word 0:
 *
 * 0-3	cursor color, YRGB
 * 4	cursor type, 0: character, 1: graphics
 * 5-7	graphics cursor position in octet
 * 8-14	cursor position in line
 * 15	-
 *
 * aux bits, word 1:
 *
 * 16-18	RGB intensity
 * 19	-
 * 20-21	scaling
 */
void uknc_state::draw_scanline(uint32_t *p, uknc_scanline *scanline)
{
	if (scanline->addr < 0100000)
	{
		// TODO fetch pixel data from main RAM
		return;
	}

	if (scanline->color)
	{
		m_video.palette = scanline->aux;
	}
	if (scanline->control)
	{
		m_video.control = scanline->aux;
	}

	if (scanline->color || scanline->control)
	{
		switch (m_yrgb->read())
		{
		case 0:
			set_palette_yrgb(m_video.palette);
			break;

		case 1:
			set_palette_ygrb(m_video.palette);
			break;

		case 2:
			set_palette__rgb(m_video.palette);
			break;

		case 3:
			set_palette_gray(m_video.palette);
			break;
		}
	}

	const pen_t *pen = m_palette->pens();
	int wide = (m_video.control >> 20) & 3;
	int start = scanline->addr - 0100000;

	for (int i = 0; i < (80 >> wide); i++)
	{
		uint8_t bit_plane_0 = m_videoram[start + i];
		uint8_t bit_plane_1 = m_videoram[start + i + 32768];
		uint8_t bit_plane_2 = m_videoram[start + i + 65536];

		if (scanline->cursor && (i << wide) == scanline->cursor_octet)
		{
			// graphics cursor
			if (scanline->cursor_type)
			{
				bit_plane_0 &= ~(1 << scanline->cursor_pixel);
				bit_plane_1 &= ~(1 << scanline->cursor_pixel);
				bit_plane_2 &= ~(1 << scanline->cursor_pixel);
				bit_plane_0 |= (BIT(scanline->cursor_color, 0) << scanline->cursor_pixel);
				bit_plane_1 |= (BIT(scanline->cursor_color, 1) << scanline->cursor_pixel);
				bit_plane_2 |= (BIT(scanline->cursor_color, 2) << scanline->cursor_pixel);
			}
			// text cursor
			else
			{
				bit_plane_0 = (BIT(scanline->cursor_color, 0)) ? 255 : 0;
				bit_plane_1 = (BIT(scanline->cursor_color, 1)) ? 255 : 0;
				bit_plane_2 = (BIT(scanline->cursor_color, 2)) ? 255 : 0;
			}
		}

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			uint8_t pen_bit_0, pen_bit_1, pen_bit_2;
			uint8_t pen_selected;

			pen_bit_0 = (bit_plane_0 >> (pixel_x)) & 0x01;
			pen_bit_1 = (bit_plane_1 >> (pixel_x)) & 0x01;
			pen_bit_2 = (bit_plane_2 >> (pixel_x)) & 0x01;

			pen_selected = (pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0);

			*p++ = pen[pen_selected];
			if (wide) *p++ = pen[pen_selected];
			if (wide > 1) *p++ = pen[pen_selected];
			if (wide > 2) *p++ = pen[pen_selected];
		}
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(uknc_state::scanline_callback)
{
	uint16_t y = m_screen->vpos();

	if (y < UKNC_VERT_START) return;
	y -= UKNC_VERT_START;
	if (y >= UKNC_DISP_VERT) return;

	draw_scanline(&m_tmpbmp.pix(y), &m_scanlines[y]);
}

uint32_t uknc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmpbmp, 0, 0, UKNC_HORZ_START, UKNC_VERT_START, cliprect);
	update_displaylist();
	return 0;
}


void uknc_state::uknc(machine_config &config)
{
	K1801VM2(config, m_maincpu, XTAL(16'000'000)/4); // external clock is /2 + internal divider /2
	m_maincpu->set_addrmap(AS_PROGRAM, &uknc_state::uknc_mem);
	m_maincpu->set_initial_mode(0160000);
//	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_disable();

	ADDRESS_MAP_BANK(config, m_bankdev0).set_map(&uknc_state::uknc_mem_banked).set_options(ENDIANNESS_LITTLE, 16, 17, 0200000);

	K1801VM2(config, m_subcpu, XTAL(12'500'000)/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &uknc_state::uknc_sub_mem);
	m_subcpu->set_initial_mode(0160000);
//	m_maincpu->set_daisy_config(sub_daisy_chain);

	ADDRESS_MAP_BANK(config, m_bankdev1).set_map(&uknc_state::uknc_cart_mem).set_options(ENDIANNESS_LITTLE, 16, 18, 0100000);
	ADDRESS_MAP_BANK(config, m_bankdev2).set_map(&uknc_state::uknc_rom_mem).set_options(ENDIANNESS_LITTLE, 16, 18, 0100000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(uknc_state::screen_update));
	m_screen->set_raw(XTAL(12'500'000), UKNC_TOTAL_HORZ, UKNC_HORZ_START, UKNC_HORZ_START+UKNC_DISP_HORZ,
		UKNC_TOTAL_VERT, UKNC_VERT_START, UKNC_VERT_START+UKNC_DISP_VERT);
	m_screen->screen_vblank().set(FUNC(uknc_state::screen_vblank));

	TIMER(config, "scantimer").configure_scanline(FUNC(uknc_state::scanline_callback), "screen", 0, 1);

	PALETTE(config, m_palette, FUNC(uknc_state::uknc_palette), 8);

	/*
	 * devices on the main cpu
	 */

	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
//	m_qbus->birq4().set_inputline(m_maincpu, T11_IRQ0);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, nullptr);

	/*
	 * devices on the sub cpu
	 */

	QBUS(config, m_subqbus, 0);
	m_subqbus->set_space(m_subcpu, AS_PROGRAM);
//	m_subqbus->birq4().set_inputline(m_subcpu, T11_IRQ0);
	QBUS_SLOT(config, "cart" ":1", qbus_cards, nullptr);
	QBUS_SLOT(config, "cart" ":2", qbus_cards, nullptr);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
}

/* ROM definition */
ROM_START( uknc )
	ROM_REGION16_LE(0100000, "subcpu", ROMREGION_ERASE00)
	ROM_LOAD("uknc.rom", 0, 0100000, CRC(a1536994) SHA1(b3c7c678c41ffa9b37f654fbf20fef7d19e6407b))
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY             FULLNAME  FLAGS */
COMP( 1987, uknc, 0,      0,      uknc,    uknc,  uknc_state, empty_init, "Elektronika", "UKNC / MS-0511",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
