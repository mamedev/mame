// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Hewlett-Packard 95LX palmtop, code name Jaguar.

    NEC V20 CPU @ 5.37 MHz
    HP F1000-80054 "Hopper" ASIC
        Display controller
        Keyboard controler
        UART
        Interrupt controller (8259 core)
        Interval timer (8254 core)
        Real time clock
        PCMCIA 1.0 controller
        ADC (4 channels; used to measure battery voltages)
        DAC (1 channel; can be used as tone generator)
    512KB or 1MB of RAM
    1MB of BIOS ROM (banked)
        P/N 18-5301 ABD \\ HN62318BFC26
    LCD, 240x128 pixels (40x16 chars in MDA-compatible text mode)

    To do:
    - blue on green LCD palette
    - native keyboard
    - 1MB model
    - identify RTC core
    - variable refresh rate?
      When the AC adapter is plugged in, the LCD refresh rate is 73.14 Hz.
      When the AC adapter is not plugged in (ie, running off of batteries) the refresh rate is 56.8 Hz.
    - everything else

    Technical info:
    - http://web.archive.org/web/20071012040320/http://www.daniel-hertrich.de/download/95lx_devguide.zip
    - http://cd.textfiles.com/blackphilesii/PHILES/HP95/HP95DEV.ZIP

    Useful links:
    - https://hermocom.com/hplx/view-all-hp-palmtop-articles/41-95lx
    - ftp://netflora.demon.co.uk/main/TEXT/hp95lxfaq.txt
    - https://www.finseth.com/hpdata/hp95lx.php
        has keyboard layout
    - http://cfile3.uf.tistory.com/attach/152BF03E50054B2925840F
        Explorer's Guide to the HP 95LX
    - http://www.palmtoppaper.com/ptphtml/12/pt120055.htm
        Evolution of the HP Palmtops
    - http://web.archive.org/web/20150423014908/http://www.sp.uconn.edu/~mchem1/HPLX.shtml
        HPLX-L mailing list archive

    Software:
    - http://www.retroisle.com/others/hp95lx/software.php
    - http://www.mizj.com/
    - http://www.hp200lx.net/
    - http://www.nic.funet.fi/index/misc/hp95lx/Index
    - http://cd.textfiles.com/blackphilesii/PHILES/HP95/

***************************************************************************/


#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/nec/nec.h"
#include "machine/bankdev.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "sound/dac.h"

#include "screen.h"
#include "emupal.h"
#include "softlist.h"
#include "speaker.h"


#define LOG_KEYBOARD  (1U << 1)
#define LOG_DEBUG     (1U << 2)

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


namespace {

class hp95lx_state : public driver_device
{
public:
	hp95lx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev_c000(*this, "bankdev_c000")
		, m_bankdev_d000(*this, "bankdev_d000")
		, m_bankdev_e000(*this, "bankdev_e000")
		, m_bankdev_e400(*this, "bankdev_e400")
		, m_bankdev_e800(*this, "bankdev_e800")
		, m_bankdev_ec00(*this, "bankdev_ec00")
		, m_ram(*this, RAM_TAG)
		, m_nvram2(*this, "nvram2")
		, m_nvram3(*this, "nvram3")
		, m_isabus(*this, "isa")
		, m_pic8259(*this, "pic8259")
		, m_pit8254(*this, "pit8254")
		, m_dac(*this, "dac")
		, m_screen(*this, "screen")
		, m_p_videoram(*this, "video")
		, m_p_chargen(*this, "gfx1")
	{ }

	void d300_w(offs_t offset, uint8_t data);
	uint8_t d300_r(offs_t offset);
	void e300_w(offs_t offset, uint8_t data);
	uint8_t e300_r(offs_t offset);
	void f300_w(offs_t offset, uint8_t data);
	uint8_t f300_r(offs_t offset);

	void hp95lx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev_c000;
	required_device<address_map_bank_device> m_bankdev_d000;
	required_device<address_map_bank_device> m_bankdev_e000;
	required_device<address_map_bank_device> m_bankdev_e400;
	required_device<address_map_bank_device> m_bankdev_e800;
	required_device<address_map_bank_device> m_bankdev_ec00;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram2;
	optional_device<nvram_device> m_nvram3;
	required_device<isa8_device> m_isabus;
	required_device<pic8259_device> m_pic8259;
	required_device<pit8254_device> m_pit8254;
	required_device<dac_8bit_r2r_device> m_dac;
	required_device<screen_device> m_screen;

private:
	void hp95lx_palette(palette_device &palette) const;

	void keyboard_clock_w(int state);
	void keyboard_data_w(int state);
	uint8_t keyboard_r(offs_t offset);
	void keyboard_w(offs_t offset, uint8_t data);
	uint8_t video_r(offs_t offset);
	void video_w(offs_t offset, uint8_t data);
	void video_address_w(uint8_t data);
	uint8_t video_register_r();
	void video_register_w(uint8_t data);
	[[maybe_unused]] void debug_w(offs_t offset, uint8_t data);

	void hp95lx_io(address_map &map) ATTR_COLD;
	void hp95lx_map(address_map &map) ATTR_COLD;
	void hp95lx_romdos(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 m_mapper[32]{}, m_rtcram[2]{};

	// crtc
	u8 m_crtc[64]{}, m_cursor_start_ras = 0;
	u16 m_disp_start_addr = 0, m_cursor_addr = 0, m_window_start_addr = 0;
	int m_register_address_latch = 0;
	bool m_graphics_mode = false;

	// from pt68k4.cpp
	bool m_kclk = false;
	uint8_t m_kdata = 0;
	uint8_t m_scancode = 0;
	uint8_t m_kbdflag = 0;
	int m_kbit = 0;
};


void hp95lx_state::hp95lx_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xa0, 0xa8, 0xa0);
	palette.set_pen_color(1, 0x30, 0x38, 0x10);
}

uint32_t hp95lx_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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

		// screen memory is normal MDA 80x25, but only a scrollable 40x16 window is displayed
		for (int y = 0; y < 128; y++)
		{
			uint16_t *p = &bitmap.pix(y);
			int const offset = (y / 8) * 160 + m_window_start_addr;

			for (int x = offset; x < offset + 80; x += 2)
			{
				uint8_t const chr = m_p_videoram[x];
				uint8_t const attr = m_p_videoram[x + 1];

				uint16_t gfx = m_p_chargen[(chr)*8 + (y % 8)];

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

				case 0x70:
					gfx ^= 0xff;
					break;
				}

				for (int i = 5; i >= 0; i--)
				{
					*p++ = BIT(gfx, i);
				}
			}
		}
	}

	return 0;
}


void hp95lx_state::machine_start()
{
	save_item(NAME(m_mapper));
	save_item(NAME(m_rtcram));
	save_item(NAME(m_crtc));
	save_item(NAME(m_graphics_mode));

	//
	save_item(NAME(m_kclk));
	save_item(NAME(m_kdata));
	save_item(NAME(m_scancode));
	save_item(NAME(m_kbdflag));
	save_item(NAME(m_kbit));

	memcpy(m_p_chargen, memregion("romdos")->base() + 0xffa6e, 0x0400);
	memcpy(m_p_chargen + 0x0400, memregion("romdos")->base() + 0xfb200, 0x0400);
}

void hp95lx_state::machine_reset()
{
	std::fill(std::begin(m_mapper), std::end(m_mapper), 0);
	std::fill(std::begin(m_rtcram), std::end(m_rtcram), 0);

	std::fill(std::begin(m_crtc), std::end(m_crtc), 0);
	m_graphics_mode = false;
	m_cursor_start_ras = 7;
	m_disp_start_addr = 0;
	m_cursor_addr = 0;

	m_kclk = true;
	m_kbit = 0;
	m_scancode = 0;
	m_kbdflag = 0;

	m_dac->write(0x7f);
}


void hp95lx_state::d300_w(offs_t offset, uint8_t data)
{
	LOG("%s: IO %04x <- %02x\n", machine().describe_context(), 0xd300 + offset, data);

	switch (offset)
	{
	case 0:
		m_window_start_addr = ((data & 0xff) << 0) | (m_window_start_addr & 0xff00);
		break;
	case 1:
		m_window_start_addr = ((data & 0x3f) << 8) | (m_window_start_addr & 0x00ff);
		break;

	case 2:
		m_graphics_mode = !BIT(data, 4);
		break;

	case 6:
	case 7:
		m_rtcram[offset - 6] = data;
		break;
	}
}

uint8_t hp95lx_state::d300_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 0:
		data = m_window_start_addr;
		break;

	case 1:
		data = m_window_start_addr >> 8;
		break;

	case 5:
		data = 0x2;
		break;

	case 6:
	case 7:
		data = m_rtcram[offset - 6];
		break;
	}

	LOG("%s: IO %04x == %02x\n", machine().describe_context(), 0xd300 + offset, data);

	return data;
}

void hp95lx_state::e300_w(offs_t offset, uint8_t data)
{
	LOG("%s: IO %04x <- %02x\n", machine().describe_context(), 0xe300 + offset, data);

	switch (offset)
	{
	case 1: // b1 = 'IR interrupt enable' (per IR.DOC)
		break;

	case 2: // interrupt control (per snd.c), 0: disable all, 127: enable all
		break;

	case 5: // DAC out (per snd.c)
		m_dac->write(data);
		break;

	case 9: // b1 = 'a2d power' (per snd.c)
		break;

	case 10: // IRFMAT register (per IR.DOC)
		break;

	case 11: // IR transmit/receive register (per IR.DOC)
		break;

	case 13: // reset?
		break;

	case 14: // keyboard output?
	case 15:
		break;
	}
}

uint8_t hp95lx_state::e300_r(offs_t offset)
{
	uint8_t data = 0;

	switch (offset)
	{
	case 2: // b6 = low battery? (FD97Eh)
		break;

	case 4: // Card Detect Register (per chap5.txt)
		data = 0x50; // memory card inserted
		break;

	case 7:
		data = 0x20;
		break;

	case 9: // FF889h.  b0 = ADC ready? (per snd2.c)
		break;

	case 10: // IRFMAT register (per IR.DOC)
		break;

	case 11: // IR transmit/receive register (per IR.DOC)
		break;

	case 14: // keyboard status?
	case 15:
		return data;
		break;
	}

	LOG("%s: IO %04x == %02x\n", machine().describe_context(), 0xe300 + offset, data);

	return data;
}

void hp95lx_state::f300_w(offs_t offset, uint8_t data)
{
	address_map_bank_device *mapper;
	const char *mapname;

	LOG("%s: IO %04x <- %02x\n", machine().describe_context(), 0xf300 + offset, data);

	if (offset >= sizeof(m_mapper)) return;

	m_mapper[offset] = data;

	switch (offset)
	{
	case 0x11: // E0000 16K
		mapper = m_bankdev_e000;
		mapname = "E0000";
		break;

	case 0x13: // E4000 16K
		mapper = m_bankdev_e400;
		mapname = "E4000";
		break;

	case 0x15: // E8000 16K
		mapper = m_bankdev_e800;
		mapname = "E8000";
		break;

	case 0x17: // EC000 16K
		mapper = m_bankdev_ec00;
		mapname = "EC000";
		break;

	case 0x18: // C0000 64K
		mapper = m_bankdev_c000;
		mapname = "C0000";
		break;

	case 0x19: // D0000 64K
		mapper = m_bankdev_d000;
		mapname = "D0000";
		break;

	default:
		LOG("MAPPER %02x unknown\n", offset);
		return;
	}

	switch (offset)
	{
	case 0x11: case 0x13: case 0x15: case 0x17:
		switch (data)
		{
		case 0: // internal
			if (m_mapper[offset - 1])
			{
				LOG("MAPPER %s <- %05X\n", mapname, (m_mapper[offset - 1] & 127) * 0x2000);
				mapper->set_bank((m_mapper[offset - 1] >> 1) & 63);
			}
			break;

		case 1: // unknown
		case 2:
			break;

		case 4: // card slot
			LOG("MAPPER %s <- %06X (card)\n", mapname, (m_mapper[offset - 1]) * 0x2000);
			mapper->set_bank(0x1000 + (m_mapper[offset - 1] >> 1));
			break;

		case 7: // XXX unmap
			LOG("MAPPER %s <- unmap\n", mapname);
			mapper->set_bank(0x400);
			break;
		}
		break;

	case 0x18: case 0x19:
		switch (data & 7)
		{
		case 0:
			if ((data & 0x87) == 0x80)
			{
				LOG("MAPPER %s <- %X0000\n", mapname, (data >> 3) & 15);
				mapper->set_bank((data >> 3) & 15);
			}
			break;

		case 4: // XXX
			break;

		case 7: // XXX unmap
			LOG("MAPPER %s <- unmap\n", mapname);
			mapper->set_bank(0x40);
			break;
		}
	}
}

uint8_t hp95lx_state::f300_r(offs_t offset)
{
	uint8_t data = 0;

	LOG("%s: IO %04x == %02x\n", machine().describe_context(), 0xf300 + offset, data);

	return data;
}

/* keyboard HLE -- adapted from pt68k4.cpp */

uint8_t hp95lx_state::keyboard_r(offs_t offset)
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

void hp95lx_state::keyboard_w(offs_t offset, uint8_t data)
{
	LOGKBD("kbd: write %02X <= %02X\n", offset + 0x60, data);
	m_pic8259->ir1_w(CLEAR_LINE);
}

void hp95lx_state::keyboard_clock_w(int state)
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
			// FIXME: workaround, map F2..F10 to blue function keys
			if (m_scancode > 0x3B && m_scancode < 0x45) m_scancode += 0x36;
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

void hp95lx_state::keyboard_data_w(int state)
{
	LOGKBD("kbd: KDATA: %d\n", state ? 1 : 0);
	m_kdata = (state == ASSERT_LINE) ? 0x80 : 0x00;
}

/* video HLE */

uint8_t hp95lx_state::video_register_r()
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

void hp95lx_state::video_register_w(uint8_t data)
{
	switch (m_register_address_latch)
	{
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
		m_pic8259->ir2_w(ASSERT_LINE);
		break;
	}
}

void hp95lx_state::video_address_w(uint8_t data)
{
	m_register_address_latch = data & 0x3f;
}

uint8_t hp95lx_state::video_r(offs_t offset)
{
	int data = 0xff;

	switch (offset)
	{
	case 1: case 3: case 5: case 7:
		data = video_register_r();
		break;

	case 10:
		data = 0xf0; // video_status_r(offset):
		break;
	}

	return data;
}

void hp95lx_state::video_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: case 2: case 4: case 6:
		video_address_w(data);
		break;

	case 1: case 3: case 5: case 7:
		video_register_w(data);
		break;
	}
}

void hp95lx_state::debug_w(offs_t offset, uint8_t data)
{
	LOGDBG("%11.6f %s debug: port %02X <= %02X\n", machine().time().as_double(), machine().describe_context(), offset + 0x90, data);
}


void hp95lx_state::hp95lx_romdos(address_map &map)
{
	map(0x0000000, 0x00fffff).rom().region("romdos", 0);
	map(0x4000000, 0x41fffff).ram().share("nvram3");
}

void hp95lx_state::hp95lx_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x7ffff).ram().share("nvram2");
	map(0xa0000, 0xaffff).rom().region("romdos", 0xe0000); // OS functions (DOS, COMMAND.COM)
	map(0xb0000, 0xb0fff).ram().share("video");
	map(0xc0000, 0xcffff).rw(m_bankdev_c000, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xd0000, 0xdffff).rw(m_bankdev_d000, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xe0000, 0xe3fff).rw(m_bankdev_e000, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xe4000, 0xe7fff).rw(m_bankdev_e400, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xe8000, 0xebfff).rw(m_bankdev_e800, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xec000, 0xeffff).rw(m_bankdev_ec00, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	map(0xf0000, 0xfffff).rom().region("romdos", 0xf0000); // BIOS ROM and SysMgr
}

void hp95lx_state::hp95lx_io(address_map &map)
{
	map.unmap_value_low();
	map(0x0020, 0x002f).rw("pic8259", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x004f).rw("pit8254", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0063).rw(FUNC(hp95lx_state::keyboard_r), FUNC(hp95lx_state::keyboard_w));
//  map(0x0090, 0x009f).w(FUNC(hp95lx_state::debug_w));
	map(0x03b0, 0x03bf).rw(FUNC(hp95lx_state::video_r), FUNC(hp95lx_state::video_w));
//  map(0x0070, 0x007f) RTC
	map(0xd300, 0xd30f).rw(FUNC(hp95lx_state::d300_r), FUNC(hp95lx_state::d300_w));
	map(0xe300, 0xe30f).rw(FUNC(hp95lx_state::e300_r), FUNC(hp95lx_state::e300_w));
	map(0xf300, 0xf31f).rw(FUNC(hp95lx_state::f300_r), FUNC(hp95lx_state::f300_w));
}


void hp95lx_state::hp95lx(machine_config &config)
{
	V20(config, m_maincpu, XTAL(5'370'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &hp95lx_state::hp95lx_map);
	m_maincpu->set_addrmap(AS_IO, &hp95lx_state::hp95lx_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	ADDRESS_MAP_BANK(config, "bankdev_c000").set_map(&hp95lx_state::hp95lx_romdos).set_options(ENDIANNESS_LITTLE, 8, 32, 0x10000);
	ADDRESS_MAP_BANK(config, "bankdev_d000").set_map(&hp95lx_state::hp95lx_romdos).set_options(ENDIANNESS_LITTLE, 8, 32, 0x10000);
	ADDRESS_MAP_BANK(config, "bankdev_e000").set_map(&hp95lx_state::hp95lx_romdos).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "bankdev_e400").set_map(&hp95lx_state::hp95lx_romdos).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "bankdev_e800").set_map(&hp95lx_state::hp95lx_romdos).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);
	ADDRESS_MAP_BANK(config, "bankdev_ec00").set_map(&hp95lx_state::hp95lx_romdos).set_options(ENDIANNESS_LITTLE, 8, 32, 0x4000);

	PIT8254(config, m_pit8254, 0);
	m_pit8254->set_clk<0>(XTAL(14'318'181) / 12); /* heartbeat IRQ */
	m_pit8254->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8254->set_clk<1>(XTAL(14'318'181) / 12); /* misc IRQ */
	m_pit8254->out_handler<1>().set(m_pic8259, FUNC(pic8259_device::ir2_w));

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	ISA8(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);

	ISA8_SLOT(config, "board0", 0, "isa", pc_isa8_cards, "com", true);

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270));
	pc_kbdc.out_clock_cb().set(FUNC(hp95lx_state::keyboard_clock_w));
	pc_kbdc.out_data_cb().set(FUNC(hp95lx_state::keyboard_data_w));

	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0); // RAM
	NVRAM(config, "nvram3", nvram_device::DEFAULT_ALL_0); // card slot

	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC

	SCREEN(config, m_screen, SCREEN_TYPE_LCD, rgb_t::white());
	m_screen->set_screen_update(FUNC(hp95lx_state::screen_update));
	m_screen->set_raw(XTAL(5'370'000) / 2, 300, 0, 240, 180, 0, 128);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(hp95lx_state::hp95lx_palette), 2);

	RAM(config, RAM_TAG).set_default_size("512K");
}


ROM_START( hp95lx )
	ROM_REGION(0x100000, "romdos", 0)
	// Version A ... ROM BIOS Ver 2.14 ... 04/02/91
	ROM_LOAD("18-5301.abd.bin", 0, 0x100000, CRC(18121c48) SHA1(c3bfb45cbbf4f57ae67fb5659da40f371d8e5c54))

	ROM_REGION(0x800,"gfx1", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT   COMPAT  MACHINE  INPUT  CLASS          INIT         COMPANY             FULLNAME    FLAGS
COMP( 1991, hp95lx,  0,       0,      hp95lx,  0,     hp95lx_state,  empty_init,  "Hewlett-Packard",  "HP 95LX",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
