// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
// thanks-to:Sergei Frolov, iret
/***************************************************************************

    Elektronika MK-98 palmtop prototype

    80C86 clone CPU @ ??? MHz
    ASIC 1
        8250 UART
        8254 timer
        8259 PIC (always runs in x86 mode)
    ASIC 2
        memory controller?
    ASIC 3
        video controller?
    128KB of RAM
    128KB of ROM
        self-tests
        monitor
        serial transfer
        memo pad
        spreadsheet
    LCD, 240x128 pixels (40x16 chars in text mode), two shades of gray (?)

    2 slots for battery-backed SRAM carts (10KB known to exist, max size 64KB).
    Carts are also compatible with MK-90 and MK-92 palmtops.

    To do:
    - native keyboard
    - carts
***************************************************************************/


#include "emu.h"

#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"

#include "screen.h"
#include "emupal.h"
#include "softlist.h"
#include "speaker.h"


#define LOG_KEYBOARD  (1U << 1)
#define LOG_DEBUG     (1U << 2)

#define VERBOSE (LOG_GENERAL|LOG_DEBUG)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


namespace {

class mk98_state : public driver_device
{
public:
	mk98_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_pic8259(*this, "pic8259")
		, m_screen(*this, "screen")
		, m_p_videoram(*this, "video")
	{ }

	void mk98(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<mk98pic_device> m_pic8259;
	required_device<screen_device> m_screen;

private:
	void mk98_palette(palette_device &palette) const;

	void keyboard_clock_w(int state);
	void keyboard_data_w(int state);
	uint8_t keyboard_r(offs_t offset);
	void keyboard_w(offs_t offset, uint8_t data);
	uint8_t serial_r(offs_t offset);
	void serial_w(offs_t offset, uint8_t data);
	uint8_t video_r(offs_t offset);
	void video_w(offs_t offset, uint8_t data);
	void video_address_w(uint8_t data);
	uint8_t video_register_r();
	void video_register_w(uint8_t data);

	void mk98_io(address_map &map) ATTR_COLD;
	void mk98_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_p_videoram;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// crtc
	u8 m_crtc[64]{}, m_cursor_start_ras = 0;
	u16 m_disp_start_addr = 0, m_cursor_addr = 0;
	int m_register_address_latch = 0, m_font_upload = 0;
	bool m_graphics_mode = false;

	// from pt68k4.cpp
	bool m_kclk = false;
	uint8_t m_kdata = 0;
	uint8_t m_scancode = 0;
	uint8_t m_kbdflag = 0;
	int m_kbit = 0;
	u8 m_p_chargen[0x800] = { };
};


void mk98_state::mk98_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xa0, 0xa8, 0xa0);
	palette.set_pen_color(1, 0x50, 0x58, 0x20);
	palette.set_pen_color(2, 0x03, 0x03, 0x01);
}

uint32_t mk98_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_graphics_mode)
	{
		for (int y = 0; y < 128; y++)
		{
			uint16_t *p = &bitmap.pix(y);
			int const offset = y * (240 / 8);

			for (int x = offset; x < offset + (240 / 8); x++)
			{
				uint16_t const gfx = m_p_videoram[x];

				for (int i = 7; i >= 0; i--)
				{
					*p++ = BIT(gfx, i);
				}
			}
		}
	}
	else
	{
		bool blink((m_screen->frame_number() % 10) > 4);

		// screen memory is normal MDA 80x25, but only a fixed 40x16 window is displayed
		for (int y = 0; y < 128; y++)
		{
			uint16_t *p = &bitmap.pix(y);
			int const offset = (y / 8) * 160;

			for (int x = offset; x < offset + 80; x += 2)
			{
				uint8_t const chr = m_p_videoram[x];
				uint8_t const attr = m_p_videoram[x + 1];

				uint16_t gfx = m_p_chargen[(chr)*8 + (y % 8) + 1];
				int fg = 1;

				if ((x >> 1) == m_cursor_addr && blink && (y % 8) >= m_cursor_start_ras)
				{
					gfx = 0xff;
				}

				switch (attr)
				{
				case 0x00:
					gfx = 0;
					break;

				case 0x01:
					if (y % 8 == 7) gfx = 0xff;
					break;

				case 0x0f:
					fg = 2;
					break;

				case 0x70:
					gfx ^= 0xff;
					break;

				case 0xf0:
					gfx ^= 0xff;
					fg = 2;
					break;
				}

				for (int i = 7; i >= 2; i--)
				{
					*p++ = BIT(gfx, i) ? fg : 0;
				}
			}
		}
	}

	return 0;
}


void mk98_state::machine_start()
{
	save_item(NAME(m_crtc));
	save_item(NAME(m_graphics_mode));

	//
	save_item(NAME(m_kclk));
	save_item(NAME(m_kdata));
	save_item(NAME(m_scancode));
	save_item(NAME(m_kbdflag));
	save_item(NAME(m_kbit));
}

void mk98_state::machine_reset()
{
	std::fill(std::begin(m_crtc), std::end(m_crtc), 0);
	m_graphics_mode = false;
	m_font_upload = 0;
	m_cursor_start_ras = 7;
	m_disp_start_addr = 0;
	m_cursor_addr = 0;

	m_kclk = true;
	m_kbit = 0;
	m_scancode = 0;
	m_kbdflag = 0;
}

uint8_t mk98_state::serial_r(offs_t offset)
{
	LOGDBG("aux: read  == %02X\n", m_crtc[0]);
	return m_crtc[0] | (m_crtc[0] & 1) << 7;
}

void mk98_state::serial_w(offs_t offset, uint8_t data)
{
	LOGDBG("aux: write <= %02X\n", data);
	m_crtc[0] = data & 0xe7;
}

/* keyboard HLE -- adapted from pt68k4.cpp */

uint8_t mk98_state::keyboard_r(offs_t offset)
{
	if (offset == 0)
	{
		LOGKBD("kbd: read  %02X == %02X\n", offset + 0x60, m_scancode);
		m_pic8259->ir1_w(CLEAR_LINE);
		return m_scancode;
	}
	else
		return 0;
}

void mk98_state::keyboard_w(offs_t offset, uint8_t data)
{
	LOGKBD("kbd: write %02X <= %02X\n", offset + 0x60, data);
	m_pic8259->ir1_w(CLEAR_LINE);
}

void mk98_state::keyboard_clock_w(int state)
{
	LOGKBD("kbd: KCLK: %d kbit: %d\n", state ? 1 : 0, m_kbit);

	if ((state == ASSERT_LINE) && (!m_kclk))
	{
		if (m_kbit >= 1 && m_kbit <= 8)
		{
			m_scancode >>= 1;
			m_scancode |= m_kdata;
		}

		// stop bit?
		if (m_kbit == 9)
		{
			m_scancode >>= 1;
			m_scancode |= m_kdata;
			// arrow keys
			switch (m_scancode)
			{
				case 0x48: m_scancode = 0x3c; break;
				case 0x50: m_scancode = 0x3e; break;
				case 0x4b: m_scancode = 0x3d; break;
				case 0x4d: m_scancode = 0x3f; break;
				case 0x53: m_scancode = 0x3b; break;
			}
			LOGKBD("kbd: scancode %02x\n", m_scancode);
			m_kbit = 0;
			m_pic8259->ir1_w(ASSERT_LINE);
		}
		else
		{
			m_kbit++;
		}
	}

	m_kclk = (state == ASSERT_LINE) ? true : false;
}

void mk98_state::keyboard_data_w(int state)
{
	LOGKBD("kbd: KDATA: %d\n", state ? 1 : 0);
	m_kdata = (state == ASSERT_LINE) ? 0x80 : 0x00;
}

/* video HLE */

uint8_t mk98_state::video_register_r()
{
	uint8_t ret = 0;

	switch (m_register_address_latch)
	{
	case 0x0e:
		ret = (m_cursor_addr >> 8) & 0xff;
		break;

	case 0x0f:
		ret = (m_cursor_addr >> 0) & 0xff;
		break;

	/* all other registers are write only and return 0 */
	default:
		break;
	}

	return ret;
}

void mk98_state::video_register_w(uint8_t data)
{
	switch (m_register_address_latch)
	{
	case 1:
		m_graphics_mode = BIT(data, 4);
		break;

	case 0x0a:
		m_cursor_start_ras = data & 0x7f;
		break;

	case 0x0c:
		m_disp_start_addr = ((data & 0x3f) << 8) | (m_disp_start_addr & 0x00ff);
		break;

	case 0x0d:
		m_disp_start_addr = ((data & 0xff) << 0) | (m_disp_start_addr & 0xff00);
		break;

	case 0x0e:
		m_cursor_addr = ((data & 0x3f) << 8) | (m_cursor_addr & 0x00ff);
		break;

	case 0x0f:
		m_cursor_addr = ((data & 0xff) << 0) | (m_cursor_addr & 0xff00);
		break;
	}
}

void mk98_state::video_address_w(uint8_t data)
{
	m_register_address_latch = data & 0x3f;
}

uint8_t mk98_state::video_r(offs_t offset)
{
	switch (offset)
	{
	case 0:
		return keyboard_r(0);

	case 5:
		return video_register_r();
	}

	return 0xff;
}

void mk98_state::video_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 2:
		m_font_upload++;
		if ((m_font_upload > 63) && (m_font_upload < 0x840))
			m_p_chargen[m_font_upload - 64] = data;
		break;

	case 4:
		video_address_w(data);
		break;

	case 5:
		video_register_w(data);
		break;
	}
}


void mk98_state::mk98_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).ram();
	map(0xb8000, 0xbdfff).ram().share("video");
	map(0xc0000, 0xdffff).noprw(); // ???
	map(0xe0000, 0xfffff).rom().region("romdos", 0);
}

void mk98_state::mk98_io(address_map &map)
{
	map.unmap_value_low();
//  map(0x0000, 0x000f).unmaprw();
	map(0x0020, 0x002f).rw(m_pic8259, FUNC(mk98pic_device::read), FUNC(mk98pic_device::write));
	map(0x0040, 0x004f).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0063).rw(FUNC(mk98_state::keyboard_r), FUNC(mk98_state::keyboard_w));
//  unidentified devices
//  map(0x00a0, 0x00a1).unmapw();
//  map(0x0110, 0x0111).unmapw();
//  map(0x0112, 0x0113).unmaprw();
//  map(0x0150, 0x0150).unmapw(); -- cart slot select
//  map(0x0170, 0x0170).unmapw();
	map(0x03d0, 0x03df).rw(FUNC(mk98_state::video_r), FUNC(mk98_state::video_w));
	map(0x03f8, 0x03fe).rw("uart0", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x03ff, 0x03ff).rw(FUNC(mk98_state::serial_r), FUNC(mk98_state::serial_w));
}


void mk98_state::mk98(machine_config &config)
{
	I8088(config, m_maincpu, XTAL(5'370'000)); // actually a 80C86 clone
	m_maincpu->set_addrmap(AS_PROGRAM, &mk98_state::mk98_map);
	m_maincpu->set_addrmap(AS_IO, &mk98_state::mk98_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(mk98pic_device::inta_cb));

	pit8254_device &pit8254(PIT8254(config, "pit8254", 0));
	pit8254.set_clk<0>(16000000/2/8); // FIXME unknown clock

	MK98PIC(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270));
	pc_kbdc.out_clock_cb().set(FUNC(mk98_state::keyboard_clock_w));
	pc_kbdc.out_data_cb().set(FUNC(mk98_state::keyboard_data_w));

	ins8250_device &uart0(INS8250(config, "uart0", XTAL(1'843'200)));
	uart0.out_tx_callback().set("serport0", FUNC(rs232_port_device::write_txd));
	uart0.out_dtr_callback().set("serport0", FUNC(rs232_port_device::write_dtr));
	uart0.out_rts_callback().set("serport0", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serport0(RS232_PORT(config, "serport0", default_rs232_devices, nullptr));
	serport0.rxd_handler().set(uart0, FUNC(ins8250_device::rx_w));
	serport0.dcd_handler().set(uart0, FUNC(ins8250_device::dcd_w));
	serport0.dsr_handler().set(uart0, FUNC(ins8250_device::dsr_w));
	serport0.ri_handler().set(uart0, FUNC(ins8250_device::ri_w));
	serport0.cts_handler().set(uart0, FUNC(ins8250_device::cts_w));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD, rgb_t::white());
	m_screen->set_screen_update(FUNC(mk98_state::screen_update));
	m_screen->set_raw(XTAL(5'370'000) / 2, 300, 0, 240, 180, 0, 128);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(mk98_state::mk98_palette), 3);

	RAM(config, RAM_TAG).set_default_size("128K");
}


ROM_START( mk98 )
	ROM_REGION(0x20000, "romdos", 0)
	ROM_LOAD("e0000.bin", 0, 0x20000, CRC(85785bd5) SHA1(b10811715f44cf8e2b41baea7b62a35082e04048))
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT   COMPAT  MACHINE  INPUT  CLASS        INIT         COMPANY         FULLNAME  FLAGS
COMP( 1998, mk98,  0,       0,      mk98,    0,     mk98_state,  empty_init,  "Elektronika",  "MK-98",  MACHINE_IS_SKELETON)
