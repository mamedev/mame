// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

UKNC (Educational Computer by Scientific Centre) PDP-11 clone.
Also known as Elektronika MS-0511.
RAM = 192K (CPU 1 = 128K, CPU 2 = 64K), ROM = 32K.
Graphics 640x288 pixels.

2009-05-12 Skeleton driver.

First half of each CPU's memory is mapped into address space for use by OS;
and second is reachable via register I/O, for use as VRAM.

CPU 1 memory is treated as bit plane 0, CPU 2 -- as planes 1 and 2.


****************************************************************************/

#include "emu.h"

#include "1515xm031.h"
#include "1801vp120.h"
#include "bus/qbus/qbus.h"
#include "bus/rs232/rs232.h"
#include "cpu/t11/t11.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/bankdev.h"
#include "machine/dl11.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "speaker.h"

#include "emupal.h"
#include "screen.h"


#define LOG_VRAM      (1U << 1)
#define LOG_DEBUG     (1U << 2)

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define LOGVRAM(...) LOGMASKED(LOG_VRAM, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


namespace {

// video parameters
static constexpr int UKNC_TOTAL_HORZ = 800;
static constexpr int UKNC_DISP_HORZ = 640;
static constexpr int UKNC_HORZ_START = 80;

static constexpr int UKNC_TOTAL_VERT = 312;
static constexpr int UKNC_DISP_VERT = 288; // 12 top sysline + 24*11 text area + 12 bottom sysline
static constexpr int UKNC_VERT_START = 19;
static constexpr int UKNC_MAX_VERT = 288 + 19;


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
		, m_channel(*this, "channel")
		, m_subchan(*this, "subchan")
		, m_keyboard(*this, "keyboard")
		, m_timer(*this, "timer")
		, m_qbus(*this, "qbus")
		, m_s2(*this, "s2")
		, m_rs232(*this, "rs232")
		, m_cart(*this, "cart")
		, m_cassette(*this, "cassette")
		, m_dac(*this, "dac")
		, m_yrgb(*this, "yrgb")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_ram(*this, "maincpu")
		, m_view_main(*this, "view_main")
		, m_view_cart(*this, "view_cart")
		, m_view_rom(*this, "view_rom")
	{ }

	void uknc(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void screen_vblank(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_callback);

	void uknc_mem(address_map &map) ATTR_COLD;
	void uknc_sub_mem(address_map &map) ATTR_COLD;

	void reset_w(int state);
	void reset_sub_w(int state);

	uint16_t trap_r(offs_t offset);
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
	std::unique_ptr<uknc_scanline[]> m_scanlines;
	bitmap_rgb32 m_tmpbmp;
	int m_odt_map;

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
	required_device<k1801vp120_pri_device> m_channel;
	required_device<k1801vp120_sub_device> m_subchan;
	required_device<k1515xm031_pri_device> m_keyboard;
	required_device<k1515xm031_sub_device> m_timer;
	required_device<qbus_device> m_qbus;
	optional_device<k1801vp065_device> m_s2;
	optional_device<rs232_port_device> m_rs232;
	required_device<qbus_device> m_cart;
	required_device<cassette_image_device> m_cassette;
	required_device<dac_bit_interface> m_dac;
	required_ioport m_yrgb;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_region_ptr<uint16_t> m_ram;

	memory_view m_view_main, m_view_cart, m_view_rom;
	std::unique_ptr<uint8_t[]> m_subram;
};


void uknc_state::uknc_mem(address_map &map)
{
	map(0000000, 0177777).view(m_view_main);
	// USER mode
	m_view_main[0](0000000, 0177777).rw(FUNC(uknc_state::trap_r), FUNC(uknc_state::trap_w));
	m_view_main[0](0000000, 0157777).ram().region("maincpu", 0);
	m_view_main[0](0176570, 0176577).rw(m_s2, FUNC(k1801vp065_device::read), FUNC(k1801vp065_device::write));
	m_view_main[0](0176640, 0176641).rw(FUNC(uknc_state::maincpu_vram_addr_r), FUNC(uknc_state::maincpu_vram_addr_w));
	m_view_main[0](0176642, 0176643).rw(FUNC(uknc_state::maincpu_vram_data_r), FUNC(uknc_state::maincpu_vram_data_w));
	m_view_main[0](0176644, 0176647).noprw(); // hardware breakpoint
	m_view_main[0](0176660, 0176667).rw(m_channel, FUNC(k1801vp120_pri_device::read<1>), FUNC(k1801vp120_pri_device::write<1>)); // printer
	m_view_main[0](0176670, 0176677).rw(m_channel, FUNC(k1801vp120_pri_device::read<2>), FUNC(k1801vp120_pri_device::write<2>)); // RPC
	m_view_main[0](0177560, 0177567).rw(m_channel, FUNC(k1801vp120_pri_device::read<0>), FUNC(k1801vp120_pri_device::write<0>)); // terminal emulator
	// HALT mode
	m_view_main[1](0000000, 0177777).ram().region("maincpu", 0);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "channel" },
//	{ "bp" },
	{ "s2" },
	{ "qbus" },
	{ nullptr }
};

void uknc_state::uknc_sub_mem(address_map &map)
{
	map(0000000, 0077777).bankrw("plane0bank0_3");

	map(0100000, 0117777).view(m_view_cart);
	m_view_cart[0](0100000, 0117777).bankr("plane0bank4").nopw();
	m_view_cart[1](0100000, 0117777).rom().region("subcpu", 0).nopw();
	m_view_cart[2](0100000, 0117777).bankrw("plane0bank4");
	m_view_cart[3](0100000, 0117777).rom().region("subcpu", 0).bankw("plane0bank4");

	map(0120000, 0176777).view(m_view_rom);
	for (int v = 0; v < 8; v++)
	{
		if (v & 1)
			m_view_rom[v](0120000, 0137777).rom().region("subcpu", 020000).bankw("plane0bank5");
		else
			m_view_rom[v](0120000, 0137777).rom().region("subcpu", 020000);
		if (v & 2)
			m_view_rom[v](0140000, 0157777).rom().region("subcpu", 040000).bankw("plane0bank6");
		else
			m_view_rom[v](0140000, 0157777).rom().region("subcpu", 040000);
		if (v & 4)
			m_view_rom[v](0160000, 0176777).rom().region("subcpu", 060000).bankw("plane0bank7");
		else
			m_view_rom[v](0160000, 0176777).rom().region("subcpu", 060000);
	}

	map(0177010, 0177011).rw(FUNC(uknc_state::vram_addr_r), FUNC(uknc_state::vram_addr_w));
	map(0177012, 0177013).rw(FUNC(uknc_state::vram_data_pl0_r), FUNC(uknc_state::vram_data_pl0_w));
	map(0177014, 0177015).rw(FUNC(uknc_state::vram_data_p12_r), FUNC(uknc_state::vram_data_p12_w));
	map(0177016, 0177027).rw(FUNC(uknc_state::sprite_r), FUNC(uknc_state::sprite_w));
	map(0177054, 0177055).rw(FUNC(uknc_state::bcsr_r), FUNC(uknc_state::bcsr_w));
	map(0177060, 0177103).rw(m_subchan, FUNC(k1801vp120_sub_device::read), FUNC(k1801vp120_sub_device::write));
	// 1515XM1-031 system controller
	map(0177700, 0177707).rw(m_keyboard, FUNC(k1515xm031_pri_device::read), FUNC(k1515xm031_pri_device::write));
	map(0177710, 0177715).rw(m_timer, FUNC(k1515xm031_sub_device::read), FUNC(k1515xm031_sub_device::write));
	map(0177716, 0177717).rw(FUNC(uknc_state::scsr_r), FUNC(uknc_state::scsr_w));
}

static const z80_daisy_config sub_daisy_chain[] =
{
//	{ "event" },
	{ "timer" },
	{ "keyboard" },
//	{ "reset" },
	{ "subchan" },
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
	m_odt_map = 1; m_view_main.select(1);
	bcsr_w(0, 01401, 0xffff);
	scsr_w(0, 0, 0xffff);
}

void uknc_state::machine_start()
{
	m_subram = make_unique_clear<uint8_t[]>(0200000);
	m_scanlines = std::make_unique<uknc_scanline[]>(UKNC_MAX_VERT);
	m_tmpbmp.allocate(UKNC_DISP_HORZ, UKNC_DISP_VERT);
	memset(&m_video, 0, sizeof(m_video));

	membank("plane0bank0_3")->set_base(&m_subram[0]);
	membank("plane0bank4")->set_base(&m_subram[0100000]);
	membank("plane0bank5")->set_base(&m_subram[0120000]);
	membank("plane0bank6")->set_base(&m_subram[0140000]);
	membank("plane0bank7")->set_base(&m_subram[0160000]);

	machine_reset();
}

void uknc_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_channel->reset();
		// breakpoint
		m_s2->reset();
		m_qbus->init_w();
	}
}

void uknc_state::reset_sub_w(int state)
{
	if (state == ASSERT_LINE)
	{
		// event
		m_timer->reset();
		m_keyboard->reset();
		// reset
		m_subchan->reset();
		m_cart->init_w();
	}
}


void uknc_state::uknc_palette(palette_device &palette) const
{
	// for debugging only, will be overwritten by firmware
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, rgb_t(pal3bit(i), pal3bit(i), pal3bit(i)));
	}
}


void uknc_state::screen_vblank(int state)
{
// FIXME not synced to vblank?
	if (!BIT(m_bcsr, 8))
		m_subcpu->set_input_line(t11_device::CP2_LINE, state);
	else
		m_subcpu->set_input_line(t11_device::CP2_LINE, CLEAR_LINE);
	if (!BIT(m_bcsr, 9))
		m_maincpu->set_input_line(t11_device::CP2_LINE, state);
	else
		m_maincpu->set_input_line(t11_device::CP2_LINE, CLEAR_LINE);
}

// VRAM access from maincpu

uint16_t uknc_state::maincpu_vram_addr_r()
{
	return m_video.maincpu_vram_addr;
}

void uknc_state::maincpu_vram_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_video.maincpu_vram_addr = data;
	m_video.maincpu_vram_data = m_ram[data];
}

uint16_t uknc_state::maincpu_vram_data_r()
{
	return m_video.maincpu_vram_data;
}

void uknc_state::maincpu_vram_data_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video.maincpu_vram_data);
	m_ram[m_video.maincpu_vram_addr] = m_video.maincpu_vram_data;
}

// VRAM access from subcpu

uint16_t uknc_state::vram_addr_r()
{
	return m_video.vram_addr;
}

void uknc_state::vram_addr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_video.vram_addr = data;
	m_video.vram_data_p12 = m_ram[data];
	m_video.vram_data_pl0 = m_subram[data];
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
	COMBINE_DATA(&m_video.vram_data_pl0);
	m_subram[m_video.vram_addr] = m_video.vram_data_pl0;
}

void uknc_state::vram_data_p12_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_video.vram_data_p12);
	m_ram[m_video.vram_addr] = m_video.vram_data_p12;
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
		{
			uint8_t vdata;

			// plane 0
			vdata = m_subram[m_video.vram_addr];
			m_video.background = BIT(vdata, 0);
			m_video.background |= BIT(vdata, 1) << 4;
			m_video.background |= BIT(vdata, 2) << 8;
			m_video.background |= BIT(vdata, 3) << 12;
			m_video.background |= BIT(vdata, 4) << 16;
			m_video.background |= BIT(vdata, 5) << 20;
			m_video.background |= BIT(vdata, 6) << 24;
			m_video.background |= BIT(vdata, 7) << 28;

			// plane 1
			vdata = m_ram[m_video.vram_addr];
			m_video.background |= BIT(vdata, 0) << 1;
			m_video.background |= BIT(vdata, 1) << 5;
			m_video.background |= BIT(vdata, 2) << 9;
			m_video.background |= BIT(vdata, 3) << 13;
			m_video.background |= BIT(vdata, 4) << 17;
			m_video.background |= BIT(vdata, 5) << 21;
			m_video.background |= BIT(vdata, 6) << 25;
			m_video.background |= BIT(vdata, 7) << 29;

			// plane 2
			vdata = m_ram[m_video.vram_addr] >> 8;
			m_video.background |= BIT(vdata, 0) << 2;
			m_video.background |= BIT(vdata, 1) << 6;
			m_video.background |= BIT(vdata, 2) << 10;
			m_video.background |= BIT(vdata, 3) << 14;
			m_video.background |= BIT(vdata, 4) << 18;
			m_video.background |= BIT(vdata, 5) << 22;
			m_video.background |= BIT(vdata, 6) << 26;
			m_video.background |= BIT(vdata, 7) << 30;
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
			m_subram[m_video.vram_addr] = vdata;
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
			m_ram[m_video.vram_addr] &= 0xff00;
			m_ram[m_video.vram_addr] |= vdata;
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
			m_ram[m_video.vram_addr] &= 0xff;
			m_ram[m_video.vram_addr] |= (vdata << 8);
		}
		break;

	case 4: // 177026
		m_video.plane_mask = data & 7;
		break;
	}
}

// system registers

uint16_t uknc_state::trap_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
	return 0xffff;
}

void uknc_state::trap_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero);
}

/*
 *	00	R	cassette data in.  if b2=1 and b3=0, interrupts at vector 0310 on data change.
 *	01	RW	cassette data out
 *	02	RW	cassette data/ext gate.  1: cassette  0: ext  (always reads as 1)
 *	03	RW	cassette data/ext/index gate.  0: cassette/ext  1: index
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

	return m_scsr | 4;
}

void uknc_state::scsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scsr);

	if ((m_scsr & 017400) == 0) m_dac->write(!BIT(m_scsr, 7));
	if (BIT(m_scsr, 2)) m_cassette->output(BIT(m_scsr, 1) ? -1 : 1);

	m_maincpu->set_input_line(t11_device::HLT_LINE, BIT(m_scsr, 4));

	if (BIT(m_scsr, 15) && m_maincpu->suspended(SUSPEND_REASON_DISABLE))
	{
		LOG("starting maincpu\n");
		m_maincpu->resume(SUSPEND_REASON_DISABLE);
	}
	else if (!BIT(m_scsr, 15) && !m_maincpu->suspended(SUSPEND_REASON_DISABLE))
	{
		LOG("stopping maincpu\n");
		m_maincpu->set_input_line(t11_device::PF_LINE, ASSERT_LINE);
	}
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

	bank1 = bitswap<2>(data, 4, 0);
	bank2 = (data >> 5) & 7;
	LOG("bcsr W banks %d,%d\n", bank1, bank2);

#if 0
	// order matters: ROM on device may unmap itself and we restore the mapping
	m_cart->ce0_ce3_w(data & 15);
	address_space *subspace = &m_subcpu->space(AS_PROGRAM);

	if (BIT(m_bcsr ^ data, 0) && BIT(data, 0))
	{
		subspace->unmap_readwrite(0100000, 0117777);
		subspace->install_readwrite_handler(0100000, 0117777,
			read16s_delegate(m_bankdev1, FUNC(address_map_bank_device::read16)),
			write16s_delegate(m_bankdev1, FUNC(address_map_bank_device::write16)));
	}
#endif
	m_view_cart.select(bank1);
	m_view_rom.select(bank2);

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

	for (int row = 0; row < UKNC_MAX_VERT; row++)
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

		if (row < 0)
		{
			LOGVRAM("dl row %d addr %06o px %06o wide %d\n", row, addr, scanline->addr, wide);
			if (wide) LOGVRAM("aux %08x (%s)\n", scanline->aux, wide == 1 ? "ctl" : "color");
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
	if (scanline->color)
	{
		m_video.palette = scanline->aux;
	}
	else if (scanline->control)
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

	if (!p) return;

	const pen_t *pen = m_palette->pens();
	int wide = (m_video.control >> 20) & 3;

	for (int i = 0; i < (80 >> wide); i++)
	{
		uint8_t bit_plane_0 = m_subram[scanline->addr + i];
		uint8_t bit_plane_1 = m_ram[scanline->addr + i];
		uint8_t bit_plane_2 = m_ram[scanline->addr + i] >> 8;

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
			int pen_selected = BIT(bit_plane_0, pixel_x) |
				BIT(bit_plane_1, pixel_x) << 1 | BIT(bit_plane_2, pixel_x) << 2;

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

	if (y < UKNC_VERT_START || y >= UKNC_MAX_VERT)
		draw_scanline(NULL, &m_scanlines[y]);
	else
		draw_scanline(&m_tmpbmp.pix(y - UKNC_VERT_START), &m_scanlines[y]);
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
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->out_reset().set(FUNC(uknc_state::reset_w));
#if 0 // not yet
	m_maincpu->out_bankswitch().set([this] (int state) {
		if (m_odt_map != state) { m_view_main.select(state); m_odt_map = state; }
		LOGDBG("%s HALT mode changed: %d\n", machine().describe_context(), state);
	});
#endif
	m_maincpu->set_disable();

	K1801VM2(config, m_subcpu, XTAL(12'500'000)/4);
	m_subcpu->set_addrmap(AS_PROGRAM, &uknc_state::uknc_sub_mem);
	m_subcpu->set_initial_mode(0160000);
	m_subcpu->set_daisy_config(sub_daisy_chain);
	m_subcpu->out_reset().set(FUNC(uknc_state::reset_sub_w));

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

	K1801VP120_PRI(config, m_channel, 0);
	m_channel->virq_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_channel->reset_wr_callback().set(m_subchan, FUNC(k1801vp120_sub_device::write_reset));
	m_channel->out_wr_callback<0>().set(m_subchan, FUNC(k1801vp120_sub_device::write_data<0>));
	m_channel->out_wr_callback<1>().set(m_subchan, FUNC(k1801vp120_sub_device::write_data<1>));
	m_channel->out_wr_callback<2>().set(m_subchan, FUNC(k1801vp120_sub_device::write_data<2>));
	m_channel->ack_wr_callback<0>().set(m_subchan, FUNC(k1801vp120_sub_device::write_ack<0>));
	m_channel->ack_wr_callback<1>().set(m_subchan, FUNC(k1801vp120_sub_device::write_ack<1>));

	K1801VP065(config, m_s2, XTAL(4'608'000));
	m_s2->set_rxc(9600);
	m_s2->set_txc(9600);
	m_s2->set_rxvec(0370);
	m_s2->set_txvec(0374);
	m_s2->txd_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_s2->rts_wr_callback().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_s2->txrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_s2->rxrdy_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);

	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->rxd_handler().set(m_s2, FUNC(k1801vp065_device::rx_w));

	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
	m_qbus->birq4().set_inputline(m_maincpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, nullptr);

	/*
	 * devices on the sub cpu
	 */

	K1515XM031_PRI(config, m_keyboard, 0);
	m_keyboard->virq_wr_callback().set_inputline(m_subcpu, t11_device::VEC_LINE);

	K1515XM031_SUB(config, m_timer, 0);
	m_timer->virq_wr_callback().set_inputline(m_subcpu, t11_device::VEC_LINE);

	K1801VP120_SUB(config, m_subchan, 0);
	m_subchan->virq_wr_callback().set_inputline(m_subcpu, t11_device::VEC_LINE);
	m_subchan->out_wr_callback<0>().set(m_channel, FUNC(k1801vp120_pri_device::write_data<0>));
	m_subchan->out_wr_callback<1>().set(m_channel, FUNC(k1801vp120_pri_device::write_data<1>));
	m_subchan->ack_wr_callback<0>().set(m_channel, FUNC(k1801vp120_pri_device::write_ack<0>));
	m_subchan->ack_wr_callback<1>().set(m_channel, FUNC(k1801vp120_pri_device::write_ack<1>));
	m_subchan->ack_wr_callback<2>().set(m_channel, FUNC(k1801vp120_pri_device::write_ack<2>));

	QBUS(config, m_cart, 0);
	m_cart->set_space(m_subcpu, AS_PROGRAM);
	m_cart->birq4().set_inputline(m_subcpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "cart" ":1", qbus_cards, "mz");
	QBUS_SLOT(config, "cart" ":2", qbus_cards, nullptr);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);

	// built-in piezo
	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}

/* ROM definition */
ROM_START( uknc )
	ROM_REGION16_LE(0400000, "maincpu", ROMREGION_ERASE00)
	ROM_REGION16_LE(0100000, "subcpu", ROMREGION_ERASE00)
	ROM_LOAD("uknc.rom", 0, 0100000, CRC(a1536994) SHA1(b3c7c678c41ffa9b37f654fbf20fef7d19e6407b))
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY             FULLNAME  FLAGS */
COMP( 1987, uknc, 0,      0,      uknc,    uknc,  uknc_state, empty_init, "Elektronika", "UKNC / MS-0511",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
