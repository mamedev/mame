// license:BSD-3-Clause
// copyright-holders:QUFB
/******************************************************************************

    Sega Ferie (c) 1994 Sega

    TODO:

    - Fix input press state, e.g. World Travel test program tablet check not passed, unresponsive menu lists
    - LCD effects, in particular when to clear/show dots, e.g. Kitten test program credits are jumbled
    - Most tablet commands aren't parsed, e.g. pen callibration positions

    Hardware
    --------

    Ferie Kitten (first model, just named "Ferie"):

    - PCB Revision: SB1P01B, 171-6896, 837-11123
    - IC1 (CPU): Toshiba T6A84
    - IC2 (Mask ROM): Sega MPR-17062-T
    - IC3 (Static RAM): Sony CXK58257AM-10L
    - IC4 (Touchpad Driver): Sega 315-5832

    Ferie Puppy:

    - PCB Revision: MB-TPD338-1.0
    - U1 (CPU): Chip-on-board
    - U2 (ROM): Chip-on-board
    - U3 (RAM): Chip-on-board
    - U4 (Touchpad Driver): Sega 315-5832
    - LCD Driver: Toshiba T6A04

    Ferie World Travel:

    - PCB Revision: SB1P02A
    - U1 (CPU): Toshiba T6A84
    - U2 (Mask ROM): Sega MPR-18080A
    - U3 (Static RAM): Sony CXK58257AM-10L
    - U4 (Touchpad Driver): Sega 315-5889

    Mask ROM pinouts are identical to MPR-18201-S: https://www.smspower.org/Development/MaskROMs

    Test Program
    ------------

    Ferie Kitten stores it in page 7 (ROM offset 0x70000). Currently unknown how to
    normally load this program. It requires writing "TEST" at RAM address 0x70.
    To force loading after reset, run "do pc = 0x1ea9" in the debugger.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/t6a84.h"
#include "machine/ram.h"
#include "machine/timer.h"

#include "crsshair.h"
#include "emupal.h"
#include "screen.h"

#include "ferie.lh"

#define LOG_MEM    (1U << 1)
#define LOG_LCD    (1U << 2)
#define LOG_TABLET (1U << 3)

//#define VERBOSE (LOG_MEM | LOG_LCD | LOG_TABLET)
#include "logmacro.h"

namespace {

class sega_ferie_state : public driver_device
{
public:
	sega_ferie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_ram(*this, RAM_TAG)
		, m_io_buttons(*this, "BUTTONS")
		, m_io_pen_x(*this, "PEN_X")
		, m_io_pen_y(*this, "PEN_Y")
		, m_io_pen_y_rescale(*this, "PEN_Y_RESCALE")
	{ }

	void sega_ferie(machine_config &config);

	DECLARE_CROSSHAIR_MAPPER_MEMBER(pen_y_mapper);
	DECLARE_CUSTOM_INPUT_MEMBER(pen_y_rescale_r);
	ioport_value pen_target_r();

private:
	static inline constexpr uint16_t LCD_W = 120;
	static inline constexpr uint16_t LCD_H = 64;

	static inline constexpr uint8_t RST1 = 0x08;
	static inline constexpr uint8_t RST3 = 0x18;

	static inline constexpr uint8_t HIGH_BATTERY = 0x40;

	// Data is sent in sequences of writes to I/O port 0xe5,
	// filling lines of dots in a configurable direction. An internal index
	// is increment after each line, and gets reset when it reaches a given threshold.
	enum lcd_fill_pattern : uint8_t
	{
		FILL_8_DOTS_Y, // Column-wise
		FILL_8_DOTS_X  // Row-wise
	};

	/*
	    Commands for parsing and configuring tablet variables. Both
	    data for requests and replies is shifted one bit at a time:

	    ROM_00::086a 4f              LD         C,data
	    ROM_00::086b 06 08           LD         i,0x8
	    ROM_00::086d cd 91 11        CALL       set_data_page_8
	    ROM_00::0870 36 00           LD         (reply),0x0
	                            LAB_ROM_00__0872
	    ROM_00::0872 00              NOP
	    ROM_00::0873 00              NOP
	    ROM_00::0874 00              NOP
	    ROM_00::0875 00              NOP
	    ROM_00::0876 79              LD         data,C
	    ROM_00::0877 e6 01           AND        0x1
	    ROM_00::0879 d3 f6           OUT        (TABLET_CTRL),data
	    ROM_00::087b 00              NOP
	    ; ...
	    ROM_00::0887 00              NOP
	    ROM_00::0888 79              LD         data,C
	    ROM_00::0889 e6 01           AND        0x1
	    ROM_00::088b f6 02           OR         0x2
	    ROM_00::088d d3 f6           OUT        (TABLET_CTRL),data
	    ROM_00::088f cb 39           SRL        C
	    ROM_00::0891 db f7           IN         data,(TABLET_DATA)
	    ROM_00::0893 07              RLCA
	    ROM_00::0894 cb 1e           RR         (reply)
	    ROM_00::0896 10 da           DJNZ       LAB_ROM_00__0872
	*/
	enum tablet_command_request : uint8_t
	{
		READ = 0,
		EFFECTIVE_PEN_X_Y = 0x0a,
		RAW_PEN_X_Y = 0x15, // Only used for callibration?
	};

	// Replies are read via one or more sequences of READ commands.
	enum tablet_command_reply_state : uint16_t
	{
		REPLY_0x06 = 0,
		EFFECTIVE_PEN_STATUS = 0x0a00,
		EFFECTIVE_PEN_X = 0x0a01,
		EFFECTIVE_PEN_Y = 0x0a02,
		RAW_PEN_STATUS = 0x1500,
		RAW_PEN_X = 0x1501,
		RAW_PEN_Y = 0x1502,
	};

	enum input_state : uint8_t
	{
		INPUT_RELEASE = 0,
		INPUT_PRESS = 0x10,
		INPUT_HOLD = 0x11,
	};

	enum pen_target : uint8_t
	{
		PEN_TARGET_LCD = 0,
		PEN_TARGET_PANEL = 1,
	};

	virtual void machine_start() override;
	virtual void machine_reset() override;

	void ferie_io_map(address_map &map);

	TIMER_DEVICE_CALLBACK_MEMBER(irq);

	void update_rtc();

	void update_crosshair(screen_device &screen);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lcd_ctrl_w(uint8_t data);
	void lcd_data_w(uint8_t data);
	uint8_t get_lcd_dots(uint16_t x, uint16_t y);

	uint8_t code_r(offs_t offset);
	uint8_t data_r(offs_t offset);
	void data_w(offs_t offset, uint8_t data);
	uint8_t stack_r(offs_t offset);
	void stack_w(offs_t offset, uint8_t data);
	uint8_t get_page_data(uint32_t address);

	void tablet_ctrl_w(uint8_t data);
	uint8_t tablet_data_r();

	static constexpr float rescale(float x, float min_x, float max_x, float a, float b)
	{
		// Rescaling (min-max normalization) from [min_x..max_x] to [a..b].
		return a + (((x - min_x) * (b - a)) / (max_x - min_x));
	}

	required_device<t6a84_device> m_maincpu;
	optional_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram;

	required_ioport m_io_buttons;
	required_ioport m_io_pen_x;
	required_ioport m_io_pen_y;
	required_ioport m_io_pen_y_rescale;

	uint8_t *m_rom;

	uint8_t m_irq_ctrl;
	uint8_t m_irq_status;
	uint8_t m_irq_vector;

	uint8_t m_rtc;

	std::unique_ptr<uint8_t[]> m_lcd_dots;
	uint8_t m_lcd_x;
	uint8_t m_lcd_y;
	uint8_t m_lcd_i;
	uint8_t m_lcd_mode;

	uint8_t m_tablet_cmd;
	uint8_t m_tablet_cmd_prev;
	uint8_t m_tablet_data_i;
	uint16_t m_tablet_reply_state;
	bool m_is_tablet_cmd_bit_set;

	uint8_t m_button_state;
	uint8_t m_pen_state;
	uint8_t m_pen_target;
};

void sega_ferie_state::ferie_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe4, 0xe4).lr8(NAME([]() { return 0; })).w(FUNC(sega_ferie_state::lcd_ctrl_w));
	map(0xe5, 0xe5).w(FUNC(sega_ferie_state::lcd_data_w));
	map(0xeb, 0xeb).lrw8(NAME([] () { return 0; /* TODO: On/Off button state */ }), NAME([this] (uint8_t data) { m_irq_ctrl = data; }));
	map(0xf4, 0xf4).lr8(NAME([this] () { return m_rtc; }));
	map(0xf5, 0xf5).lr8(NAME([this] () { return m_io_buttons->read() & 0x3f; }));
	map(0xf6, 0xf6).w(FUNC(sega_ferie_state::tablet_ctrl_w));
	map(0xf7, 0xf7).r(FUNC(sega_ferie_state::tablet_data_r));
}

void sega_ferie_state::machine_start()
{
	uint8_t *rom = memregion("mask_rom")->base();
	m_rom = reinterpret_cast<uint8_t *>(rom);
	uint8_t *ram = m_ram->pointer();

	m_maincpu->space(AS_PROGRAM).install_rom(0x00000, 0x7ffff, rom);
	m_maincpu->space(AS_PROGRAM).install_ram(0x80000, 0x87fff, ram);
	m_maincpu->space(AS_DATA).install_ram(0x00000, 0x7ffff, rom);
	m_maincpu->space(AS_DATA).install_ram(0x80000, 0x87fff, ram);
	m_maincpu->space(t6a84_device::AS_STACK).install_ram(0x00000, 0x7ffff, rom);
	m_maincpu->space(t6a84_device::AS_STACK).install_ram(0x80000, 0x87fff, ram);

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0000, 0xffff,
			read8sm_delegate(*this, FUNC(sega_ferie_state::code_r)));
	m_maincpu->space(AS_DATA).install_readwrite_handler(0x0000, 0xffff,
			read8sm_delegate(*this, FUNC(sega_ferie_state::data_r)),
			write8sm_delegate(*this, FUNC(sega_ferie_state::data_w)));
	m_maincpu->space(t6a84_device::AS_STACK).install_readwrite_handler(0x0000, 0xffff,
			read8sm_delegate(*this, FUNC(sega_ferie_state::stack_r)),
			write8sm_delegate(*this, FUNC(sega_ferie_state::stack_w)));

	save_item(NAME(m_irq_ctrl));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_irq_vector));

	save_item(NAME(m_rtc));

	m_lcd_dots = make_unique_clear<uint8_t[]>(LCD_W * LCD_H);
	save_pointer(NAME(m_lcd_dots), LCD_W * LCD_H);
	save_item(NAME(m_lcd_mode));
	save_item(NAME(m_lcd_x));
	save_item(NAME(m_lcd_y));
	save_item(NAME(m_lcd_i));

	save_item(NAME(m_tablet_cmd));
	save_item(NAME(m_tablet_cmd_prev));
	save_item(NAME(m_tablet_data_i));
	save_item(NAME(m_tablet_reply_state));
	save_item(NAME(m_is_tablet_cmd_bit_set));

	save_item(NAME(m_button_state));
	save_item(NAME(m_pen_state));
	save_item(NAME(m_pen_target));
}

void sega_ferie_state::machine_reset()
{
	m_irq_ctrl = 0;
	m_irq_status = 0;
	m_irq_vector = RST3;

	m_rtc = 0;

	memset(m_lcd_dots.get(), 0, LCD_W * LCD_H);
	m_lcd_mode = FILL_8_DOTS_Y;
	m_lcd_x = 0;
	m_lcd_y = 0;
	m_lcd_i = 0;

	m_tablet_cmd = READ;
	m_tablet_cmd_prev = READ;
	m_tablet_data_i = 0;
	m_tablet_reply_state = REPLY_0x06;
	m_is_tablet_cmd_bit_set = false;

	m_button_state = INPUT_RELEASE;
	m_pen_state = INPUT_RELEASE;
	m_pen_target = PEN_TARGET_LCD;
}

CROSSHAIR_MAPPER_MEMBER(sega_ferie_state::pen_y_mapper)
{
	// Parameter `linear_value` is ignored, since we will read the input port directly
	// for adjustments, just need to return that value in the expected range [0.0f..1.0f].
	return (float) pen_y_rescale_r() / 0xff;
}

CUSTOM_INPUT_MEMBER(sega_ferie_state::pen_y_rescale_r)
{
	/*
	    There are two distinct areas that can be interacted with the pen:

	    - LCD screen visible area: 8 tiles = 64 pixels;
	    - Bottom panel: 3 tiles = 24 pixels;

	    In order to transparently map coordinates between each area, we split
	    the value across these areas, but rescaled to the input port's full range.
	*/
	const int16_t io_pen_y_min = m_io_pen_y->field(0xff)->minval();
	const int16_t io_pen_y_max = m_io_pen_y->field(0xff)->maxval();
	const int16_t screen_y_max = io_pen_y_max * 0.75f;
	int16_t adjusted_value = m_io_pen_y->read();
	if (adjusted_value > screen_y_max) {
		adjusted_value = rescale(adjusted_value, screen_y_max, io_pen_y_max, io_pen_y_min, io_pen_y_max);
		m_pen_target = PEN_TARGET_PANEL;
	} else {
		adjusted_value = rescale(adjusted_value, io_pen_y_min, screen_y_max, io_pen_y_min, io_pen_y_max);
		m_pen_target = PEN_TARGET_LCD;
	}

	return adjusted_value;
}

ioport_value sega_ferie_state::pen_target_r()
{
	return m_pen_target;
}

TIMER_DEVICE_CALLBACK_MEMBER(sega_ferie_state::irq)
{
	bool is_button_pressed = (m_io_buttons->read() & 0x7f) != 0;
	if (is_button_pressed && m_button_state == INPUT_RELEASE) {
		m_button_state = INPUT_PRESS;
	}
	if (m_button_state == INPUT_PRESS) {
		m_irq_vector = (m_irq_ctrl == 0xf7) ? RST1 : RST3;
		m_button_state = INPUT_HOLD;
	}
	if (!is_button_pressed && m_button_state == INPUT_HOLD) {
		m_irq_vector = RST3;
		m_button_state = INPUT_RELEASE;
	}

	m_maincpu->set_input_line_vector(0, 0xc7 | m_irq_vector);
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_irq_status ? ASSERT_LINE : CLEAR_LINE);
	m_irq_status ^= 1;
}

uint8_t sega_ferie_state::code_r(offs_t offset)
{
	return get_page_data(m_maincpu->code_address(offset));
}

uint8_t sega_ferie_state::data_r(offs_t offset)
{
	return get_page_data(m_maincpu->data_address(offset));
}

void sega_ferie_state::data_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MEM, "DATA w [%06x] = %02x\n", offset, data);
	if (offset > 0x7fff) {
		osd_printf_warning("DATA w OOB @ %06x\n", m_maincpu->pc());
	} else {
		m_ram->pointer()[offset] = data;
	}
}

uint8_t sega_ferie_state::stack_r(offs_t offset)
{
	return get_page_data(m_maincpu->stack_address(offset));
}

void sega_ferie_state::stack_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_MEM, "STACK w [%06x] = %02x\n", offset, data);
	if (offset > 0x7fff) {
		osd_printf_warning("STACK w OOB @ %06x\n", m_maincpu->pc());
	} else {
		m_ram->pointer()[offset] = data;
	}
}

uint8_t sega_ferie_state::get_page_data(uint32_t address)
{
	uint8_t data = 0;
	if (((int32_t) address) - 0x80000 > 0x7fff) {
		osd_printf_warning("get_page_data OOB @ %06x\n", m_maincpu->pc());
	} else {
		data = (address < 0x80000) ? m_rom[address] : m_ram->pointer()[address - 0x80000];
	}
	LOGMASKED(LOG_MEM, "get_page_data [%06x] = %02x\n", address, data);

	return data;
}

uint8_t sega_ferie_state::tablet_data_r()
{
	if (machine().side_effects_disabled() || !m_is_tablet_cmd_bit_set) {
		return HIGH_BATTERY;
	}

	if (m_tablet_data_i == 0) {
		if (m_tablet_reply_state == REPLY_0x06) {
			switch (m_tablet_cmd_prev) {
				case EFFECTIVE_PEN_X_Y:
					m_tablet_reply_state = EFFECTIVE_PEN_STATUS;
					LOGMASKED(LOG_TABLET, "tablet_data_r EFFECTIVE_PEN_X_Y -> EFFECTIVE_PEN_STATUS\n");
					break;
				case RAW_PEN_X_Y:
					m_tablet_reply_state = RAW_PEN_STATUS;
					LOGMASKED(LOG_TABLET, "tablet_data_r RAW_PEN_X_Y -> RAW_PEN_STATUS\n");
					break;
			}
		}

		if (BIT(m_io_buttons->read(), 6)) {
			if (m_pen_state == INPUT_RELEASE) {
				m_pen_state = INPUT_PRESS;
				LOGMASKED(LOG_TABLET, "tablet_data_r INPUT_RELEASE -> INPUT_PRESS\n");
			} else if (m_pen_state == INPUT_PRESS) {
				m_pen_state = INPUT_HOLD;
				LOGMASKED(LOG_TABLET, "tablet_data_r INPUT_PRESS -> INPUT_HOLD\n");
			}
		} else if (m_pen_state != INPUT_RELEASE) {
			m_pen_state = INPUT_RELEASE;
			LOGMASKED(LOG_TABLET, "tablet_data_r INPUT_RELEASE\n");
		}
	}

	int16_t io_pen_x_min = m_io_pen_x->field(0xff)->minval();
	int16_t io_pen_x_max = m_io_pen_x->field(0xff)->maxval();
	int16_t io_pen_y_min = m_io_pen_y->field(0xff)->minval();
	int16_t io_pen_y_max = m_io_pen_y->field(0xff)->maxval();
	int16_t io_pen_x_pos = m_io_pen_x->read();
	int16_t io_pen_y_pos = pen_y_rescale_r();
	uint8_t reply;
	switch (m_tablet_reply_state) {
		case EFFECTIVE_PEN_STATUS:
			// Fallthrough
		case RAW_PEN_STATUS:
			reply = m_pen_state;
			break;
		case EFFECTIVE_PEN_X:
			reply = rescale(io_pen_x_pos, io_pen_x_min, io_pen_x_max, 0x00, 0x80);
			break;
		case EFFECTIVE_PEN_Y:
			reply = (m_pen_target == PEN_TARGET_LCD)
					? rescale(io_pen_y_pos, io_pen_y_min, io_pen_y_max, 0x00, 0x40)
					: rescale(io_pen_y_pos, io_pen_y_min, io_pen_y_max, 0x40, 0x40 + 3 * 8);
			break;
		case RAW_PEN_X:
			reply = io_pen_x_pos;
			break;
		case RAW_PEN_Y:
			reply = io_pen_y_pos;
			break;
		default:
			reply = 0x06;
	}

	uint8_t data = BIT(reply, m_tablet_data_i) << 7;

	LOGMASKED(LOG_TABLET, "tablet_data_r i = %02x\n", m_tablet_data_i);
	m_tablet_data_i++;
	if (m_tablet_data_i == 8) {
		LOGMASKED(LOG_TABLET, "tablet_data_r cmd = %02x, rpy = %02x @ %06x\n", m_tablet_cmd, reply, m_maincpu->pc());
		m_tablet_data_i = 0;
		if (m_tablet_cmd != READ) {
			m_tablet_cmd_prev = m_tablet_cmd;
			m_tablet_reply_state = REPLY_0x06;
		}
		m_tablet_cmd = READ;
		switch (m_tablet_reply_state) {
			case EFFECTIVE_PEN_STATUS:
				if (m_pen_state == INPUT_HOLD) {
					m_tablet_reply_state = EFFECTIVE_PEN_X;
					LOGMASKED(LOG_TABLET, "tablet_data_r EFFECTIVE_PEN_STATUS -> EFFECTIVE_PEN_X\n");
				}
				break;
			case EFFECTIVE_PEN_X:
				m_tablet_reply_state = EFFECTIVE_PEN_Y;
				LOGMASKED(LOG_TABLET, "tablet_data_r EFFECTIVE_PEN_X -> EFFECTIVE_PEN_Y\n");
				break;
			case RAW_PEN_STATUS:
				if (m_pen_state == INPUT_HOLD) {
					m_tablet_reply_state = RAW_PEN_X;
					LOGMASKED(LOG_TABLET, "tablet_data_r RAW_PEN_STATUS -> RAW_PEN_X\n");
				}
				break;
			case RAW_PEN_X:
				m_tablet_reply_state = RAW_PEN_Y;
				LOGMASKED(LOG_TABLET, "tablet_data_r RAW_PEN_X -> RAW_PEN_Y\n");
				break;
			default:
				m_tablet_reply_state = REPLY_0x06;
		}
	}

	m_is_tablet_cmd_bit_set = false;

	return data | HIGH_BATTERY;
}

void sega_ferie_state::tablet_ctrl_w(uint8_t data)
{
	if ((data & 2) != 0) {
		LOGMASKED(LOG_TABLET, "tablet_ctrl_w i = %02x\n", m_tablet_data_i);
		m_tablet_cmd |= ((data & 1) << m_tablet_data_i);
		m_is_tablet_cmd_bit_set = true;
	}
}

void sega_ferie_state::update_rtc()
{
	system_time systime;
	machine().current_datetime(systime);
	m_rtc = systime.time;
}

void sega_ferie_state::update_crosshair(screen_device &screen)
{
	// Either screen crosshair or layout view's cursor should be visible at a time.
	machine().crosshair().get_crosshair(0).set_screen(m_pen_target ? CROSSHAIR_SCREEN_NONE : &screen);
}

void sega_ferie_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(188, 190, 144));  // Background
	palette.set_pen_color(1, rgb_t(176, 180, 134));  // LCD pixel off
	palette.set_pen_color(2, rgb_t( 38,  60,  80));  // LCD pixel on
}

uint32_t sega_ferie_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	update_rtc();
	update_crosshair(screen);

	// Render LCD
	for (size_t y = cliprect.min_y; y <= cliprect.max_y; y++) {
		for (size_t x = cliprect.min_x; x <= cliprect.max_x / 8; x++) {
			uint8_t data = get_lcd_dots(x, y);
			for (size_t bit_i = 0; bit_i < 8; bit_i++) {
				// TODO: Adjust based on configured contrast
				bitmap.pix(y, (8 * x) + bit_i) = BIT(data, 7 - bit_i) + 1;
			}
		}
	}

	return 0;
}

uint8_t sega_ferie_state::get_lcd_dots(uint16_t x, uint16_t y)
{
	return m_lcd_dots[y * LCD_W + x];
}

void sega_ferie_state::lcd_ctrl_w(uint8_t data)
{
	LOGMASKED(LOG_LCD, "lcd_ctrl_w @ %06x = %02x\n", m_maincpu->pc(), data);
	if ((data & 0x20) != 0) {
		m_lcd_x = data & 0x1f;
	} else if ((data & 0x80) != 0) {
		m_lcd_y = data & 0x7f;
		m_lcd_i = 0;
	} else if (data == 0x7) {
		LOGMASKED(LOG_LCD, "lcd_ctrl_w FILL_8_DOTS_X\n");
		m_lcd_mode = FILL_8_DOTS_X;
	} else if (data == 0x3) {
		LOGMASKED(LOG_LCD, "lcd_ctrl_w FILL_8_DOTS_Y\n");
		m_lcd_mode = FILL_8_DOTS_Y;
	}
}

void sega_ferie_state::lcd_data_w(uint8_t data)
{
	const uint16_t offset = (m_lcd_mode == FILL_8_DOTS_Y)
			? (m_lcd_y + m_lcd_i) * LCD_W + m_lcd_x
			: m_lcd_y * LCD_W + (m_lcd_x + m_lcd_i);

	LOGMASKED(LOG_LCD, "lcd_data_w[y=%02x,x=%02x,i=%02x => %08x] = %02x\n", m_lcd_y, m_lcd_x, m_lcd_i, offset, data);
	if (offset > LCD_W * LCD_H - 1) {
		osd_printf_warning("lcd_data_w OOB @ %06x\n", m_maincpu->pc());
	} else {
		m_lcd_dots[offset] = data;
	}

	m_lcd_i += 1;
	if (m_lcd_mode == FILL_8_DOTS_Y) {
		if (m_lcd_i == 0x8) {
			m_lcd_i = 0;
		}
	} else if (m_lcd_mode == FILL_8_DOTS_X) {
		if (m_lcd_i == 0xf) {
			m_lcd_i = 0;
			m_lcd_y += 1;
		}
	}
}

static INPUT_PORTS_START( sega_ferie )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("Return")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("On/Off")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("Pen Down")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(sega_ferie_state, pen_target_r)

	PORT_START("PEN_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Pen X")

	PORT_START("PEN_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_MINMAX(0, 255) PORT_PLAYER(1) PORT_NAME("Pen Y") PORT_CROSSHAIR_MAPPER_MEMBER(DEVICE_SELF, sega_ferie_state, pen_y_mapper)

	PORT_START("PEN_Y_RESCALE")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(sega_ferie_state, pen_y_rescale_r)
INPUT_PORTS_END

void sega_ferie_state::sega_ferie(machine_config &config)
{
	T6A84(config, m_maincpu, XTAL(4'000'000)); // High frequency external crystal, connected to HXIN/HXOUT pins
	m_maincpu->set_addrmap(AS_IO, &sega_ferie_state::ferie_io_map);

	RAM(config, RAM_TAG).set_default_size("32K").set_default_value(0x00);

	// FIXME: Guessed timings
	TIMER(config, "irq").configure_periodic(FUNC(sega_ferie_state::irq), attotime::from_hz(200));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(LCD_W, LCD_H);
	m_screen->set_visarea(0, LCD_W - 1, 0, LCD_H - 1);
	m_screen->set_screen_update(FUNC(sega_ferie_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(sega_ferie_state::palette), 3);

	config.set_default_layout(layout_ferie);
}

ROM_START( ferieki )
	ROM_REGION16_LE( 0x80000, "mask_rom", 0 )
	ROM_LOAD( "mpr-17062-t.ic2", 0x00000, 0x80000, CRC(c693c9f6) SHA1(491e23902f5ef0dea9156f244f5aa2a21ab68505) )
ROM_END

ROM_START( feriepu )
	ROM_REGION16_LE( 0x80000, "mask_rom", 0 )
	ROM_LOAD( "rom.u2", 0x00000, 0x80000, CRC(895bfe47) SHA1(28ca98471a4a4b5084884b39eccbc74bff1cf4c6) )
ROM_END

ROM_START( feriewt )
	ROM_REGION16_LE( 0x80000, "mask_rom", 0 )
	ROM_LOAD( "mpr-18080a.u2", 0x00000, 0x80000, CRC(117aea09) SHA1(73ff933ba2bacd485cbc0580b023341fffac692f) )
ROM_END

} // anonymous namespace


//    year, name,     parent,  compat, machine,    input,      class,            init,       company, fullname,             flags
CONS( 1994, ferieki,  0,       0,      sega_ferie, sega_ferie, sega_ferie_state, empty_init, "Sega",  "Ferie Kitten",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
CONS( 1995, feriepu,  0,       0,      sega_ferie, sega_ferie, sega_ferie_state, empty_init, "Sega",  "Ferie Puppy", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
CONS( 1995, feriewt,  0,       0,      sega_ferie, sega_ferie, sega_ferie_state, empty_init, "Sega",  "Ferie World Travel", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
