// license:BSD-3-Clause
// copyright-holders:Carl
/*
    SCN2672 - Programmable Video Timing Controller (PVTC)
    SCN2674 - Advanced Video Display Controller (AVDC)
*/

#include "emu.h"
#include "scn2674.h"

#include "screen.h"

#define LOG_IR      (1 << 0)
#define LOG_COMMAND (1 << 1)
#define LOG_INTR    (1 << 2)
#define LOG_READ    (1 << 3)
#define VERBOSE     (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SCN2672, scn2672_device, "scn2672", "Signetics SCN2672 PVTC")
DEFINE_DEVICE_TYPE(SCN2674, scn2674_device, "scn2674", "Signetics SCN2674 AVDC")


// default address map
void scn2674_device::scn2674_vram(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, (1 << space_config(0)->addr_width()) - 1).noprw();
}

scn2672_device::scn2672_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: scn2674_device(mconfig, SCN2672, tag, owner, clock, false)
{
}

scn2674_device::scn2674_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: scn2674_device(mconfig, SCN2674, tag, owner, clock, true)
{
}

scn2674_device::scn2674_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool extend_addressing)
	: device_t(mconfig, type, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_intr_cb(*this)
	, m_breq_cb(*this)
	, m_mbc_cb(*this)
	, m_mbc_char_cb(*this)
	, m_mbc_attr_cb(*this)
	, m_IR_pointer(0)
	, m_screen1_address(0), m_screen2_address(0)
	, m_cursor_address(0)
	, m_irq_register(0), m_status_register(0), m_irq_mask(0)
	, m_gfx_enabled(false)
	, m_display_enabled(false), m_dadd_enabled(false), m_display_enabled_field(false), m_display_enabled_scanline(false)
	, m_cursor_enabled(false)
	, m_hpixels_per_column(0)
	, m_double_ht_wd(false)
	, m_scanline_per_char_row(0)
	, m_csync_select(false)
	, m_buffer_mode_select(0)
	, m_interlace_enable(false)
	, m_equalizing_constant(0)
	, m_use_row_table(false)
	, m_horz_sync_width(0), m_horz_back_porch(0)
	, m_vert_front_porch(0), m_vert_back_porch(0)
	, m_rows_per_screen(0)
	, m_character_blink_rate_divisor(0)
	, m_character_per_row(0)
	, m_cursor_first_scanline(0), m_cursor_last_scanline(0)
	, m_cursor_underline_position(0)
	, m_cursor_rate_divisor(0)
	, m_cursor_blink(false)
	, m_vsync_width(0)
	, m_display_buffer_first_address(0)
	, m_display_buffer_last_address(0)
	, m_display_pointer_address(0)
	, m_reset_scanline_counter_on_scrollup(false)
	, m_reset_scanline_counter_on_scrolldown(false)
	, m_scroll_start(false), m_scroll_end(false)
	, m_scroll_lines(0)
	, m_split_register{0, 0}
	, m_double{0, 0}
	, m_spl{false, false}
	, m_dbl1(0)
	, m_char_buffer(0), m_attr_buffer(0)
	, m_linecounter(0), m_address(0), m_start1change(0)
	, m_display_cb(*this)
	, m_scanline_timer(nullptr)
	, m_breq_timer(nullptr)
	, m_vblank_timer(nullptr)
	, m_char_space(nullptr), m_attr_space(nullptr)
	, m_char_space_config("charram", ENDIANNESS_LITTLE, 8, extend_addressing ? 16 : 14, 0, address_map_constructor(FUNC(scn2674_device::scn2674_vram), this))
	, m_attr_space_config("attrram", ENDIANNESS_LITTLE, 8, extend_addressing ? 16 : 14, 0)
{
}

device_memory_interface::space_config_vector scn2674_device::memory_space_config() const
{
	return has_configured_map(1)
			? space_config_vector{ std::make_pair(0, &m_char_space_config), std::make_pair(1, &m_attr_space_config) }
			: space_config_vector{ std::make_pair(0, &m_char_space_config) };
}

void scn2674_device::device_start()
{
	// resolve callbacks
	m_display_cb.resolve();
	m_intr_cb.resolve_safe();
	m_breq_cb.resolve_safe();
	m_mbc_cb.resolve_safe();
	m_mbc_char_cb.resolve();
	m_mbc_attr_cb.resolve();
	m_scanline_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scn2674_device::scanline_timer), this));
	m_breq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scn2674_device::breq_timer), this));
	m_vblank_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(scn2674_device::vblank_timer), this));
	screen().register_screen_bitmap(m_bitmap);

	m_char_space = &space(0);
	if (has_space(1))
		m_attr_space = &space(1);

	save_item(NAME(m_address));
	save_item(NAME(m_linecounter));
	save_item(NAME(m_screen1_address));
	save_item(NAME(m_screen2_address));
	save_item(NAME(m_cursor_address));
	save_item(NAME(m_IR_pointer));
	save_item(NAME(m_irq_register));
	save_item(NAME(m_status_register));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_gfx_enabled));
	save_item(NAME(m_display_enabled));
	save_item(NAME(m_dadd_enabled));
	save_item(NAME(m_display_enabled_field));
	save_item(NAME(m_display_enabled_scanline));
	save_item(NAME(m_cursor_enabled));
	save_item(NAME(m_double_ht_wd));
	save_item(NAME(m_scanline_per_char_row));
	save_item(NAME(m_csync_select));
	save_item(NAME(m_buffer_mode_select));
	save_item(NAME(m_interlace_enable));
	save_item(NAME(m_equalizing_constant));
	save_item(NAME(m_use_row_table));
	save_item(NAME(m_horz_sync_width));
	save_item(NAME(m_horz_back_porch));
	save_item(NAME(m_vert_front_porch));
	save_item(NAME(m_vert_back_porch));
	save_item(NAME(m_rows_per_screen));
	save_item(NAME(m_character_blink_rate_divisor));
	save_item(NAME(m_character_per_row));
	save_item(NAME(m_cursor_first_scanline));
	save_item(NAME(m_cursor_last_scanline));
	save_item(NAME(m_cursor_underline_position));
	save_item(NAME(m_cursor_rate_divisor));
	save_item(NAME(m_cursor_blink));
	save_item(NAME(m_vsync_width));
	save_item(NAME(m_display_buffer_first_address));
	save_item(NAME(m_display_buffer_last_address));
	save_item(NAME(m_display_pointer_address));
	save_item(NAME(m_reset_scanline_counter_on_scrollup));
	save_item(NAME(m_reset_scanline_counter_on_scrolldown));
	save_item(NAME(m_scroll_start));
	save_item(NAME(m_scroll_end));
	save_item(NAME(m_scroll_lines));
	save_item(NAME(m_split_register));
	save_item(NAME(m_double));
	save_item(NAME(m_spl));
	save_item(NAME(m_dbl1));
	save_item(NAME(m_char_buffer));
	if (m_attr_space != nullptr)
		save_item(NAME(m_attr_buffer));
	save_item(NAME(m_start1change));
}

void scn2674_device::device_reset()
{
	m_screen1_address = 0;
	m_screen2_address = 0;
	m_cursor_address = 0;
	m_irq_register = 0;
	m_status_register = 0;
	m_irq_mask = 0;
	m_gfx_enabled = false;
	m_display_enabled = false;
	m_dadd_enabled = false;
	m_display_enabled_field = false;
	m_display_enabled_scanline = false;
	m_cursor_enabled = false;
	m_double_ht_wd = false;
	m_scanline_per_char_row = 1;
	m_csync_select = false;
	m_buffer_mode_select = 0;
	m_interlace_enable = false;
	m_equalizing_constant = 0;
	m_use_row_table = false;
	m_horz_sync_width = 0;
	m_horz_back_porch = 0;
	m_vert_front_porch = 0;
	m_vert_back_porch = 0;
	m_rows_per_screen = 0;
	m_character_blink_rate_divisor = 0;
	m_character_per_row = 0;
	m_cursor_first_scanline = 0;
	m_cursor_last_scanline = 0;
	m_cursor_underline_position = 0;
	m_cursor_rate_divisor = 0;
	m_cursor_blink = false;
	m_vsync_width = 3; // fixed for SCN2672
	m_display_buffer_first_address = 0;
	m_display_buffer_last_address = 0;
	m_display_pointer_address = 0;
	m_reset_scanline_counter_on_scrollup = false;
	m_reset_scanline_counter_on_scrolldown = false;
	m_scroll_start = false;
	m_scroll_end = false;
	m_scroll_lines = 0;
	m_split_register[0] = m_split_register[1] = 0;
	m_double[0] = m_double[1] = 0;
	m_spl[0] = m_spl[1] = false;
	m_dbl1 = 0;
	m_char_buffer = m_attr_buffer = 0;
	m_linecounter = 0;
	m_IR_pointer = 0;
	m_address = 0;
	m_start1change = 0;

	m_intr_cb(CLEAR_LINE);
}

// 11 Initialization Registers (8-bit each)
void scn2672_device::write_init_regs(uint8_t data)
{
	//LOGMASKED(LOG_COMMAND, "scn2674_write_init_regs %02x %02x\n",m_IR_pointer,data);

	bool parameters_changed = false;
	switch (m_IR_pointer)
	{
	case 0:
		parameters_changed = m_scanline_per_char_row != ((data & 0x78) >> 3) + 1;

		m_double_ht_wd = BIT(data, 7);
		m_scanline_per_char_row = ((data & 0x78) >> 3) + 1;
		m_csync_select = BIT(data, 2);
		m_buffer_mode_select = data & 0x03;

		LOGMASKED(LOG_IR, "IR0 - Scanlines per Character Row %02i\n", m_scanline_per_char_row);//value+1 = scanlines
		LOGMASKED(LOG_IR, "IR0 - %s Output Selected\n", m_csync_select ? "CSYNC" : "VSYNC");
		switch (m_buffer_mode_select)
		{
		case 0:
			LOGMASKED(LOG_IR, "IR0 - Independent Buffer Mode\n");
			break;

		case 1:
			LOGMASKED(LOG_IR, "IR0 - Transparent Buffer Mode\n");
			break;

		case 2:
			LOGMASKED(LOG_IR, "IR0 - Shared Buffer Mode\n");
			break;

		case 3:
			LOGMASKED(LOG_IR, "IR0 - Row Buffer Mode\n");
			break;
		}
		break;

	case 1:
		parameters_changed = m_equalizing_constant != (data & 0x7f) + 1;

		m_interlace_enable = BIT(data, 7);
		m_equalizing_constant = (data & 0x7f) + 1;

		LOGMASKED(LOG_IR, "IR1 - Interlace %s\n", m_interlace_enable ? "on" : "off");
		LOGMASKED(LOG_IR, "IR1 - Equalizing Constant %02i CCLKs\n", m_equalizing_constant);
		break;

	case 2:
		parameters_changed = m_horz_sync_width != (((data & 0x78) >> 3) * 2) + 2;

		m_horz_sync_width = (((data & 0x78) >> 3) * 2) + 2;
		m_horz_back_porch = ((data & 0x07) * 4) + 1;

		LOGMASKED(LOG_IR, "IR2 - Horizontal Sync Width %02i CCLKs\n", m_horz_sync_width);
		LOGMASKED(LOG_IR, "IR2 - Horizontal Back Porch %02i CCLKs\n", m_horz_back_porch);
		break;

	case 3:
		parameters_changed = m_vert_front_porch != (((data & 0xe0) >> 5) * 4) + 4
			|| m_vert_back_porch != ((data & 0x1f) * 2) + 4;

		m_vert_front_porch = (((data & 0xe0) >> 5) * 4) + 4;
		m_vert_back_porch = ((data & 0x1f) * 2) + 4;

		LOGMASKED(LOG_IR, "IR3 - Vertical Front Porch %02i Lines\n", m_vert_front_porch);
		LOGMASKED(LOG_IR, "IR3 - Vertical Back Porch %02i Lines\n", m_vert_back_porch);
		break;

	case 4:
		parameters_changed = m_rows_per_screen != (data & 0x7f) + 1;

		m_rows_per_screen = (data & 0x7f) + 1;
		m_character_blink_rate_divisor = BIT(data, 7) ? 32 : 16;

		LOGMASKED(LOG_IR, "IR4 - Rows Per Screen %02i\n", m_rows_per_screen);
		LOGMASKED(LOG_IR, "IR4 - Character Blink Rate = 1/%02i\n", m_character_blink_rate_divisor);
		break;

	case 5:
		parameters_changed = m_character_per_row != data + 1;

		m_character_per_row = data + 1;
		LOGMASKED(LOG_IR, "IR5 - Active Characters Per Row %02i\n", m_character_per_row);
		break;

	case 6:
		m_cursor_last_scanline = data & 0x0f;
		m_cursor_first_scanline = (data & 0xf0) >> 4;
		LOGMASKED(LOG_IR, "IR6 - First Line of Cursor %02i\n", m_cursor_first_scanline);
		LOGMASKED(LOG_IR, "IR6 - Last Line of Cursor %02i\n", m_cursor_last_scanline);
		break;

	case 7:
		m_cursor_underline_position = data & 0x0f;
		m_double_ht_wd = BIT(data, 4);
		m_cursor_blink = BIT(data, 5);

		LOGMASKED(LOG_IR, "IR7 - Underline Position %02i\n", m_cursor_underline_position);
		LOGMASKED(LOG_IR, "IR7 - Double Height %s\n", m_double_ht_wd ? "on" : "off");
		LOGMASKED(LOG_IR, "IR7 - Cursor blink %s\n", m_cursor_blink ? "on" : "off");
		LOGMASKED(LOG_IR, "IR7 - Light pen line %02i\n", ((data & 0xc0) >> 6) * 2 + 3);
		break;

	case 8:
		m_display_buffer_first_address = (m_display_buffer_first_address & 0xf00) | data;
		LOGMASKED(LOG_IR, "IR8 - Display Buffer First Address LSB %02x\n", m_display_buffer_first_address & 0xff);
		break;

	case 9:
		m_display_buffer_first_address = (m_display_buffer_first_address & 0x0ff) | (data & 0x0f) << 4;
		m_display_buffer_last_address = (data & 0xf0) >> 4;
		LOGMASKED(LOG_IR, "IR9 - Display Buffer First Address MSB %01x\n", m_display_buffer_first_address >> 8);
		LOGMASKED(LOG_IR, "IR9 - Display Buffer Last Address %02x\n", m_display_buffer_last_address);
		break;

	case 10:
		m_cursor_rate_divisor = BIT(data, 7) ? 32 : 16;
		m_split_register[0] = data & 0x7f;
		LOGMASKED(LOG_IR, "IR10 - Split Register %02x\n", m_split_register[0]);
		LOGMASKED(LOG_IR, "IR10 - Cursor rate 1/%02i\n", m_cursor_rate_divisor);
		break;

	case 11: // not valid!
	case 12:
	case 13:
	case 14:
	case 15:
		break;
	}

	if (parameters_changed)
		recompute_parameters();

	m_IR_pointer = std::min(m_IR_pointer + 1, 10);
}

// 15 Initialization Registers (8-bit each)
void scn2674_device::write_init_regs(uint8_t data)
{
	//LOGMASKED(LOG_COMMAND, "scn2674_write_init_regs %02x %02x\n",m_IR_pointer,data);

	bool parameters_changed = false;
	switch (m_IR_pointer)
	{
	case 0:
		parameters_changed = m_scanline_per_char_row != ((data & 0x78) >> 3) + 1;

		m_double_ht_wd = BIT(data, 7);
		m_scanline_per_char_row = ((data & 0x78) >> 3) + 1;
		m_csync_select = BIT(data, 2);
		m_buffer_mode_select = data & 0x03;

		LOGMASKED(LOG_IR, "IR0 - Double Ht Wd %s\n", m_double_ht_wd ? "on" : "off");//affects IR14 as well
		LOGMASKED(LOG_IR, "IR0 - Scanlines per Character Row %02i\n", m_scanline_per_char_row);//value+1 = scanlines
		LOGMASKED(LOG_IR, "IR0 - %s Output Selected\n", m_csync_select ? "CSYNC" : "VSYNC");
		switch (m_buffer_mode_select)
		{
		case 0:
			LOGMASKED(LOG_IR, "IR0 - Independent Buffer Mode\n");
			break;

		case 1:
			LOGMASKED(LOG_IR, "IR0 - Transparent Buffer Mode\n");
			break;

		case 2:
			LOGMASKED(LOG_IR, "IR0 - Shared Buffer Mode\n");
			break;

		case 3:
			LOGMASKED(LOG_IR, "IR0 - Row Buffer Mode\n");
			break;
		}
		break;

	case 1:
		parameters_changed = m_equalizing_constant != (data & 0x7f) + 1;

		m_interlace_enable = BIT(data, 7);
		m_equalizing_constant = (data & 0x7f) + 1;

		LOGMASKED(LOG_IR, "IR1 - Interlace %s\n", m_interlace_enable ? "on" : "off");
		LOGMASKED(LOG_IR, "IR1 - Equalizing Constant %02i CCLKs\n", m_equalizing_constant);
		break;

	case 2:
		parameters_changed = m_horz_sync_width != (((data & 0x78) >> 3) * 2) + 2;

		m_use_row_table = BIT(data, 7);
		m_horz_sync_width = (((data & 0x78) >> 3) * 2) + 2;
		m_horz_back_porch = ((data & 0x07) * 4) - 1;

		LOGMASKED(LOG_IR, "IR2 - Row Table %s\n", m_use_row_table ? "on" : "off");
		LOGMASKED(LOG_IR, "IR2 - Horizontal Sync Width %02i CCLKs\n", m_horz_sync_width);
		LOGMASKED(LOG_IR, "IR2 - Horizontal Back Porch %02i CCLKs\n", m_horz_back_porch);
		break;

	case 3:
		parameters_changed = m_vert_front_porch != (((data & 0xe0) >> 5) * 4) + 4
			|| m_vert_back_porch != ((data & 0x1f) * 2) + 4;

		m_vert_front_porch = (((data & 0xe0) >> 5) * 4) + 4;
		m_vert_back_porch = ((data & 0x1f) * 2) + 4;

		LOGMASKED(LOG_IR, "IR3 - Vertical Front Porch %02i Lines\n", m_vert_front_porch);
		LOGMASKED(LOG_IR, "IR3 - Vertical Back Porch %02i Lines\n", m_vert_back_porch);
		break;

	case 4:
		parameters_changed = m_rows_per_screen != (data & 0x7f) + 1;

		m_rows_per_screen = (data & 0x7f) + 1;
		m_character_blink_rate_divisor = BIT(data, 7) ? 128 : 64;

		LOGMASKED(LOG_IR, "IR4 - Rows Per Screen %02i\n", m_rows_per_screen);
		LOGMASKED(LOG_IR, "IR4 - Character Blink Rate = 1/%02i\n", m_character_blink_rate_divisor);
		break;

	case 5:
		parameters_changed = m_character_per_row != data + 1;

		m_character_per_row = data + 1;
		LOGMASKED(LOG_IR, "IR5 - Active Characters Per Row %02i\n", m_character_per_row);
		break;

	case 6:
		m_cursor_last_scanline = data & 0x0f;
		m_cursor_first_scanline = (data & 0xf0) >> 4;
		LOGMASKED(LOG_IR, "IR6 - First Line of Cursor %02i\n", m_cursor_first_scanline);
		LOGMASKED(LOG_IR, "IR6 - Last Line of Cursor %02i\n", m_cursor_last_scanline);
		break;

	case 7:
		{
			const uint8_t vsync_table[4] = {3,1,5,7};
			m_cursor_underline_position = data & 0x0f;
			m_cursor_rate_divisor = BIT(data, 4) ? 64 : 32;
			m_cursor_blink = BIT(data, 5);

			parameters_changed = m_vsync_width != vsync_table[(data & 0xc0) >> 6];
			m_vsync_width = vsync_table[(data & 0xc0) >> 6];

			LOGMASKED(LOG_IR, "IR7 - Underline Position %02i\n", m_cursor_underline_position);
			LOGMASKED(LOG_IR, "IR7 - Cursor rate 1/%02i\n", m_cursor_rate_divisor);
			LOGMASKED(LOG_IR, "IR7 - Cursor blink %s\n", m_cursor_blink ? "on" : "off");
			LOGMASKED(LOG_IR, "IR7 - Vsync Width  %02i Lines\n", m_vsync_width);
			break;
		}

	case 8:
		m_display_buffer_first_address = (m_display_buffer_first_address & 0xf00) | data;
		LOGMASKED(LOG_IR, "IR8 - Display Buffer First Address LSB %02x\n", m_display_buffer_first_address & 0xff);
		break;

	case 9:
		m_display_buffer_first_address = (m_display_buffer_first_address & 0x0ff) | (data & 0x0f) << 4;
		m_display_buffer_last_address = (data & 0xf0) >> 4;
		LOGMASKED(LOG_IR, "IR9 - Display Buffer First Address MSB %01x\n", m_display_buffer_first_address >> 8);
		LOGMASKED(LOG_IR, "IR9 - Display Buffer Last Address %02x\n", m_display_buffer_last_address);
		break;

	case 10:
		m_display_pointer_address = (m_display_pointer_address & 0x3f00) | data;
		LOGMASKED(LOG_IR, "IR10 - Display Pointer Address Lower %02x\n", m_display_pointer_address & 0x00ff);
		break;

	case 11:
		m_display_pointer_address = (m_display_pointer_address & 0x00ff) | (data & 0x3f) << 8;
		m_reset_scanline_counter_on_scrollup = BIT(data, 6);
		m_reset_scanline_counter_on_scrolldown = BIT(data, 7);

		LOGMASKED(LOG_IR, "IR11 - Display Pointer Address Upper %02x\n", m_display_pointer_address >> 8);
		LOGMASKED(LOG_IR, "IR11 - Reset Scanline Counter on Scroll Up %s\n", m_reset_scanline_counter_on_scrollup ? "on" : "off");
		LOGMASKED(LOG_IR, "IR11 - Reset Scanline Counter on Scroll Down %s\n", m_display_pointer_address ? "on" : "off");
		break;

	case 12:
		m_scroll_start = BIT(data, 7);
		m_split_register[0] = data & 0x7f;
		LOGMASKED(LOG_IR, "IR12 - Scroll Start %s\n", m_scroll_start ? "on" : "off");
		LOGMASKED(LOG_IR, "IR12 - Split Register 1 %02x\n", m_split_register[0]);
		break;

	case 13:
		m_scroll_end = BIT(data, 7);
		m_split_register[1] = data & 0x7f;
		LOGMASKED(LOG_IR, "IR13 - Scroll End %s\n", m_scroll_end ? "on" : "off");
		LOGMASKED(LOG_IR, "IR13 - Split Register 2 %02x\n", m_split_register[1]);
		break;

	case 14:
		m_scroll_lines = data & 0x0f;
		if (!m_double_ht_wd)
		{
			m_double[1] = (data & 0x30) >> 4;
			LOGMASKED(LOG_IR, "IR14 - Double 2 %02x\n", m_double[1]);
		}
		//0 normal, 1, double width, 2, double width and double tops 3, double width and double bottoms
		//1 affects SSR1, 2 affects SSR2
		//If Double Height enabled in IR0, Screen start 1 upper (bits 7 and 6)replace Double 1, and Double 2 is unused
		m_double[0] = (data & 0xc0) >> 6;
		LOGMASKED(LOG_IR, "IR14 - Double 1 %02x\n", m_double[0]);

		LOGMASKED(LOG_IR, "IR14 - Scroll Lines %02i\n", m_scroll_lines);
		break;

	case 15: // not valid!
		break;
	}

	if (parameters_changed)
		recompute_parameters();

	m_IR_pointer = std::min(m_IR_pointer + 1, 14);
}

void scn2674_device::set_gfx_enabled(bool enabled)
{
	LOGMASKED(LOG_COMMAND, "%s: GFX %s\n", machine().describe_context(), enabled ? "enabled" : "disabled");
	m_gfx_enabled = enabled;

	// FIXME: GFX enable takes effect at next character row
	recompute_parameters();
}

void scn2674_device::set_display_enabled(bool enabled, int n)
{
	if (!enabled)
	{
		m_display_enabled = false;

		if (n == 1)
		{
			LOGMASKED(LOG_COMMAND, "%s: Display off - float DADD bus\n", machine().describe_context());
			m_dadd_enabled = false;
		}
		else
		{
			LOGMASKED(LOG_COMMAND, "%s: Display off - no float DADD bus\n", machine().describe_context());
			m_dadd_enabled = true;
		}
	}
	else
	{
		if (n == 1)
		{
			m_display_enabled_field = true;
			LOGMASKED(LOG_COMMAND, "%s: Display ON - next field\n", machine().describe_context());
		}
		else
		{
			m_display_enabled_scanline = true;
			LOGMASKED(LOG_COMMAND, "%s: Display ON - next scanline\n", machine().describe_context());
		}
		recompute_parameters(); // start the scanline timer
	}
}

void scn2674_device::set_cursor_enabled(bool enabled)
{
	LOGMASKED(LOG_COMMAND, "%s: Cursor %s\n", machine().describe_context(), enabled ? "on" : "off");
	m_cursor_enabled = enabled;
}

void scn2674_device::reset_interrupt_status(uint8_t bits)
{
	LOGMASKED(LOG_COMMAND, "%s: Reset interrupts/status %02x\n", machine().describe_context(), bits);

	uint8_t old_irq = m_irq_register;
	uint8_t old_status = m_status_register;
	m_irq_register &= ~bits;
	m_status_register &= ~bits;

	if (BIT(bits, 0) && BIT(old_status, 0))
		LOGMASKED(LOG_INTR, "Split 2 %sstatus cleared\n", BIT(old_irq, 0) ? "interrupt/" : "");
	if (BIT(bits, 1) && BIT(old_status, 1))
		LOGMASKED(LOG_INTR, "Ready %sstatus cleared\n", BIT(old_irq, 1) ? "interrupt/" : "");
	if (BIT(bits, 2) && BIT(old_status, 2))
		LOGMASKED(LOG_INTR, "Split 1 %sstatus cleared\n", BIT(old_irq, 2) ? "interrupt/" : "");
	if (BIT(bits, 3) && BIT(old_status, 3))
		LOGMASKED(LOG_INTR, "Line Zero %sstatus cleared\n", BIT(old_irq, 3) ? "interrupt/" : "");
	if (BIT(bits, 4) && BIT(old_status, 4))
		LOGMASKED(LOG_INTR, "V-Blank %sstatus cleared\n", BIT(old_irq, 4) ? "interrupt/" : "");

	if (m_irq_register == 0 && old_irq != 0)
	{
		LOGMASKED(LOG_INTR, "All interrupts cleared\n");
		m_intr_cb(CLEAR_LINE);
	}
}

void scn2674_device::write_interrupt_mask(bool enabled, uint8_t bits)
{
	uint8_t changed_bits = bits;

	if (enabled)
	{
		changed_bits &= ~m_irq_mask;
		m_irq_mask |= bits;
	}
	else
	{
		changed_bits &= m_irq_mask;
		m_irq_mask &= ~bits;
	}

	LOGMASKED(LOG_INTR, "%s: %s interrupts %02x by mask\n", machine().describe_context(), enabled ? "Enable" : "Disable", bits);

	if (BIT(changed_bits, 0))
		LOGMASKED(LOG_INTR, "Split 2 IRQ %s\n", enabled ? "enabled" : "disabled");
	if (BIT(changed_bits, 1))
		LOGMASKED(LOG_INTR, "Ready IRQ %s\n", enabled ? "enabled" : "disabled");
	if (BIT(changed_bits, 2))
		LOGMASKED(LOG_INTR, "Split 1 IRQ %s\n", enabled ? "enabled" : "disabled");
	if (BIT(changed_bits, 3))
		LOGMASKED(LOG_INTR, "Line Zero IRQ %s\n", enabled ? "enabled" : "disabled");
	if (BIT(changed_bits, 4))
		LOGMASKED(LOG_INTR, "V-Blank IRQ %s\n", enabled ? "enabled" : "disabled");
}

void scn2674_device::write_delayed_command(uint8_t data)
{
	int i;

	// Delayed Commands
	// These set 0x20 in status register when done
	// These use the pointer address according to the datasheet but the pcx expects the screen start 2 address instead
	switch (data)
	{
	case 0xa4:
		// read at pointer address
		m_char_buffer = m_char_space->read_byte(m_screen2_address);
		if (m_attr_space != nullptr)
			m_attr_buffer = m_attr_space->read_byte(m_screen2_address);
		LOGMASKED(LOG_COMMAND, "%s: DELAYED read at pointer address %02x\n", machine().describe_context(), data);
		break;

	case 0xa2:
		// write at pointer address
		m_char_space->write_byte(m_screen2_address, m_char_buffer);
		if (m_attr_space != nullptr)
			m_attr_space->write_byte(m_screen2_address, m_attr_buffer);
		LOGMASKED(LOG_COMMAND, "%s: DELAYED write at pointer address %02x\n", machine().describe_context(), data);
		break;

	case 0xa6:  // used by the Octopus
		// write at pointer address
		m_char_space->write_byte(m_display_pointer_address, m_char_buffer);
		if (m_attr_space != nullptr)
			m_attr_space->write_byte(m_display_pointer_address, m_attr_buffer);
		LOGMASKED(LOG_COMMAND, "%s: DELAYED write at display pointer address %02x\n", machine().describe_context(), data);
		break;

	case 0xa9:
		// increment cursor address
		m_cursor_address++;
		LOGMASKED(LOG_COMMAND, "%s: DELAYED increase cursor address %02x\n", machine().describe_context(), data);
		break;

	case 0xac:
		// read at cursor address
		m_char_buffer = m_char_space->read_byte(m_cursor_address);
		if (m_attr_space != nullptr)
			m_attr_buffer = m_attr_space->read_byte(m_cursor_address);
		LOGMASKED(LOG_COMMAND, "%s: DELAYED read at cursor address %02x\n", machine().describe_context(), data);
		break;

	case 0xaa:
		// write at cursor address
		m_char_space->write_byte(m_cursor_address, m_char_buffer);
		if (m_attr_space != nullptr)
			m_attr_space->write_byte(m_cursor_address, m_attr_buffer);
		LOGMASKED(LOG_COMMAND, "%s: DELAYED write at cursor address %02x\n", machine().describe_context(), data);
		break;

	case 0xad:
		// read at cursor address + increment
		m_char_buffer = m_char_space->read_byte(m_cursor_address);
		if (m_attr_space != nullptr)
			m_attr_buffer = m_attr_space->read_byte(m_cursor_address);
		m_cursor_address++;
		LOGMASKED(LOG_COMMAND, "%s: DELAYED read at cursor address+increment %02x\n", machine().describe_context(), data);
		break;

	case 0xab:
	case 0xaf:  // LSI Octopus sends command 0xAF
		// write at cursor address + increment
		m_char_space->write_byte(m_cursor_address, m_char_buffer);
		if (m_attr_space != nullptr)
			m_attr_space->write_byte(m_cursor_address, m_attr_buffer);
		m_cursor_address++;
		LOGMASKED(LOG_COMMAND, "%s: DELAYED write at cursor address+increment %02x\n", machine().describe_context(), data);
		break;

	case 0xbb:
		// write from cursor address to pointer address
		// TODO: transfer only during blank
		m_display_enabled = false;
		m_display_enabled_field = true;
		m_display_enabled_scanline = false;
		for (i = m_cursor_address; i != m_screen2_address; i = ((i + 1) & 0xffff))
		{
			m_char_space->write_byte(i, m_char_buffer);
			if (m_attr_space != nullptr)
				m_attr_space->write_byte(i, m_attr_buffer);
		}
		m_char_space->write_byte(i, m_char_buffer); // get the last
		if (m_attr_space != nullptr)
			m_attr_space->write_byte(i, m_attr_buffer);
		m_cursor_address = m_screen2_address;
		LOGMASKED(LOG_COMMAND, "%s: DELAYED write from cursor address to pointer address %02x\n", machine().describe_context(), data);
		break;

	case 0xbd:
		// read from cursor address to pointer address
		LOGMASKED(LOG_COMMAND, "%s: DELAYED read from cursor address to pointer address %02x\n", machine().describe_context(), data);
		break;

	case 0xbf:
		// write from cursor address to pointer address
		// TODO: transfer only during blank
		for (i = m_cursor_address; i != m_display_pointer_address; i = (i + 1) & 0xffff)
		{
			m_char_space->write_byte(i, m_char_buffer);
			if (m_attr_space != nullptr)
				m_attr_space->write_byte(i, m_attr_buffer);
		}
		m_char_space->write_byte(i, m_char_buffer); // get the last
		if (m_attr_space != nullptr)
			m_attr_space->write_byte(i, m_attr_buffer);
		m_cursor_address = m_display_pointer_address;
		LOGMASKED(LOG_COMMAND, "%s: DELAYED write from cursor address to pointer address %02x\n", machine().describe_context(), data);
		break;

	default:
		LOGMASKED(LOG_COMMAND, "%s: Unknown delayed command %02x\n", machine().describe_context(), data);
		break;
	}
}

void scn2674_device::write_command(uint8_t data)
{
	switch (data & 0xe0)
	{
	case 0x00:
		if (!BIT(data, 4))
		{
			// master reset, configures registers
			LOGMASKED(LOG_COMMAND, "%s: Master reset\n", machine().describe_context());
			m_IR_pointer = 0;
			m_irq_register = 0x00;
			m_status_register = 0x20; // RDFLG activated
			m_irq_mask = 0x00;
			m_gfx_enabled = false;
			m_display_enabled = false;
			m_dadd_enabled = false;
			m_cursor_enabled = false;
			m_use_row_table = false;
			m_intr_cb(CLEAR_LINE);
			m_breq_cb(CLEAR_LINE);
			m_mbc_cb(0);
			m_breq_timer->adjust(attotime::never);
			m_vblank_timer->adjust(attotime::never);
		}
		else
		{
			// set IR pointer
			LOGMASKED(LOG_COMMAND, "%s: IR pointer set to %02i\n", machine().describe_context(), data & 0x0f);

			m_IR_pointer = data & 0x0f;
		}
		break;

	case 0x20:
		// Any combination of these is valid
		if (BIT(data, 1))
			set_gfx_enabled(BIT(data, 0));
		if (BIT(data, 3))
			set_display_enabled(BIT(data, 0), BIT(data, 2));
		if (BIT(data, 4))
			set_cursor_enabled(BIT(data, 0));

		/* END */
		break;

	case 0x40:
		// Reset Interrupt / Status bits
		reset_interrupt_status(data & 0x1f);
		break;

	case 0x80:
		// Disable Interrupt mask
		write_interrupt_mask(false, data & 0x1f);
		break;

	case 0x60:
		// Enable Interrupt mask
		write_interrupt_mask(true, data & 0x1f);
		break;

	case 0xa0:
		write_delayed_command(data);
		break;

	default:
		LOGMASKED(LOG_COMMAND, "%s: Unknown command %02x\n", machine().describe_context(), data);
		break;
	}
}


uint8_t scn2674_device::read(offs_t offset)
{
	/*
	Offset:  Purpose
	 0       Interrupt Register
	 1       Status Register
	 2       Screen Start 1 Lower Register
	 3       Screen Start 1 Upper Register
	 4       Cursor Address Lower Register
	 5       Cursor Address Upper Register
	 6       Screen Start 2 Lower Register
	 7       Screen Start 2 Upper Register
	*/

	switch (offset)
	{
	/*  Status / Irq Register

	    --RV ZSRs

	 6+7 -- = ALWAYS 0
	  5  R  = RDFLG (Status Register Only)
	  4  V  = Vblank
	  3  Z  = Line Zero
	  2  S  = Split 1
	  1  R  = Ready
	  0  s  = Split 2
	*/

	case 0:
		LOGMASKED(LOG_READ, "%s: Read Irq Register %02x\n", machine().describe_context(), m_irq_register);
		return m_irq_register;

	case 1:
		LOGMASKED(LOG_READ, "%s: Read Status Register %02X\n", machine().describe_context(), m_status_register);
		return m_status_register;

	case 2:
		LOGMASKED(LOG_READ, "%s: Read Screen1_l Register\n", machine().describe_context());
		return m_screen1_address & 0x00ff;

	case 3:
		LOGMASKED(LOG_READ, "%s: Read Screen1_h Register\n", machine().describe_context());
		return (m_screen1_address & 0x3f00) >> 8;

	case 4:
		LOGMASKED(LOG_READ, "%s: Read Cursor_l Register\n", machine().describe_context());
		return m_cursor_address & 0x00ff;

	case 5:
		LOGMASKED(LOG_READ, "%s: Read Cursor_h Register\n", machine().describe_context());
		return m_cursor_address >> 8;

	case 6:
		LOGMASKED(LOG_READ, "%s: Read Screen2_l Register\n", machine().describe_context());
		return m_screen2_address & 0x00ff;

	case 7:
		LOGMASKED(LOG_READ, "%s: Read Screen2_h Register\n", machine().describe_context());
		return m_screen2_address >> 8;
	}

	return 0xff;
}


void scn2674_device::write(offs_t offset, uint8_t data)
{
	/*
	Offset:  Purpose
	 0       Initialization Registers
	 1       Command Register
	 2       Screen Start 1 Lower Register
	 3       Screen Start 1 Upper Register
	 4       Cursor Address Lower Register
	 5       Cursor Address Upper Register
	 6       Screen Start 2 Lower Register
	 7       Screen Start 2 Upper Register
	*/

	switch (offset)
	{
	case 0:
		write_init_regs(data);
		break;

	case 1:
		write_command(data);
		break;

	case 2:
		m_screen1_address = (m_screen1_address & 0x3f00) | data;
		if (!screen().vblank())
			m_start1change = (m_linecounter / m_scanline_per_char_row) + 1;
		else
			m_start1change = 0;
		break;

	case 3:
		m_screen1_address = (m_screen1_address & 0x00ff) | data << 8;
		m_dbl1 = (data & 0xc0) >> 6;
		if (m_double_ht_wd)
		{
			m_double[0] = m_dbl1;
			m_screen1_address &= 0x3fff;
			LOGMASKED(LOG_IR, "IR14 - Double 1 overridden %02x\n", m_double[0]);
		}
		if (!screen().vblank())
			m_start1change = (m_linecounter / m_scanline_per_char_row) + 1;
		else
			m_start1change = 0;
		break;

	case 4:
		m_cursor_address = (m_cursor_address & 0x3f00) | data;
		break;

	case 5:
		m_cursor_address = (m_cursor_address & 0x00ff) | (data & 0x3f) << 8;
		break;

	case 6:
		m_screen2_address = (m_screen2_address & 0x3f00) | data;
		break;

	case 7:
		m_screen2_address = (m_screen2_address & 0x00ff) | (data & 0x3f) << 8;
		m_spl[0] = BIT(data, 6);
		m_spl[1] = BIT(data, 7);
		break;
	}
}

void scn2674_device::recompute_parameters()
{
	if (!m_equalizing_constant || !m_character_per_row || !m_rows_per_screen)
	{
		m_scanline_timer->adjust(attotime::never);
		return;
	}

	int horiz_chars_total = (m_equalizing_constant + (m_horz_sync_width << 1)) << 1;
	int horiz_pix_total = horiz_chars_total * m_hpixels_per_column;
	int vert_pix_total = m_rows_per_screen * m_scanline_per_char_row + m_vert_front_porch + m_vert_back_porch + m_vsync_width;
	attotime refresh = screen().frame_period();
	int max_visible_x = (m_character_per_row * m_hpixels_per_column) - 1;
	int max_visible_y = (m_rows_per_screen * m_scanline_per_char_row) - 1;

	//attotime refresh = clocks_to_attotime(horiz_chars_total * vert_pix_total);
	LOGMASKED(LOG_IR, "width %u height %u max_x %u max_y %u refresh %f\n", horiz_pix_total, vert_pix_total, max_visible_x, max_visible_y, refresh.as_hz());

	rectangle visarea(0, max_visible_x, 0, max_visible_y);
	screen().configure(horiz_pix_total, vert_pix_total, visarea, refresh.as_attoseconds());

	m_linecounter = screen().vpos();
	m_scanline_timer->adjust(screen().time_until_pos((m_linecounter + 1) % vert_pix_total, 0), 0, screen().scan_period());
}

TIMER_CALLBACK_MEMBER(scn2674_device::scanline_timer)
{
	int dw = m_double_ht_wd ? m_double[0] : 0;  // double width
	if (((m_display_enabled_scanline) || (m_display_enabled_field && !m_interlace_enable)) && (!m_display_enabled))
	{
		m_display_enabled = true;
		m_dadd_enabled = true;
		m_display_enabled_scanline = false;
		m_display_enabled_field = false;
	}

	m_linecounter++;

	if (m_linecounter >= screen().height())
	{
		m_linecounter = 0;
		m_address = m_screen1_address;
	}

	bool lastscan = m_linecounter == (m_rows_per_screen * m_scanline_per_char_row) - 1;
	if (lastscan)
	{
		int horz_sync_begin = 2 * (m_equalizing_constant + 2 * m_horz_sync_width) - m_horz_sync_width - m_horz_back_porch;
		m_vblank_timer->adjust(clocks_to_attotime(horz_sync_begin));
	}

	if (m_linecounter >= (m_rows_per_screen * m_scanline_per_char_row))
	{
		if (m_buffer_mode_select == 3 && m_linecounter == (screen().height() - 1))
			m_breq_timer->adjust(clocks_to_attotime(m_character_per_row + 1), ASSERT_LINE);
		return;
	}

	int charrow = m_linecounter % m_scanline_per_char_row;
	int tilerow = charrow;

	// should be triggered at the start of each ROW (line zero for that row)
	if (charrow == 0)
	{
		m_status_register |= 0x08;
		if (BIT(m_irq_mask, 3))
		{
			LOGMASKED(LOG_INTR, "Line Zero interrupt at line %d\n", m_linecounter);
			m_irq_register |= 0x08;
			m_intr_cb(ASSERT_LINE);
		}
		if (m_buffer_mode_select == 3)
		{
			m_mbc_cb(1);
			m_breq_timer->adjust(clocks_to_attotime(m_character_per_row + 1), CLEAR_LINE);
		}
	}
	else if (m_buffer_mode_select == 3 && charrow == (m_scanline_per_char_row - 1) && !lastscan)
		m_breq_timer->adjust(clocks_to_attotime(m_character_per_row + 1), ASSERT_LINE);

	// Handle screen splits
	for (int s = 0; s < 2; s++)
	{
		if ((m_linecounter == ((m_split_register[s] + 1) * m_scanline_per_char_row)) && m_linecounter)
		{
			uint8_t flag = (s == 0) ? 0x04 : 0x01;
			m_status_register |= flag;
			if ((m_irq_mask & flag) != 0)
			{
				LOGMASKED(LOG_INTR, "Split Screen %d interrupt at line %d\n", s + 1, m_linecounter);
				m_irq_register |= flag;
				m_intr_cb(ASSERT_LINE);
			}
			if (m_spl[s])
				m_address = m_screen2_address;
			if (!m_double_ht_wd)
				dw = m_double[s];
		}
	}

	// WY-50 requires that normal row buffering take place even after a "display off" command
	if (!m_dadd_enabled)
		return;

	if (m_use_row_table)
	{
		if (m_double_ht_wd)
			dw = m_screen1_address >> 14;
		if (!charrow)
		{
			uint16_t addr = m_screen2_address;
			uint16_t line = m_char_space->read_word(addr);
			m_screen1_address = line;
			if (m_double_ht_wd)
			{
				dw = line >> 14;
				line &= ~0xc000;
			}
			m_address = line;
			addr += 2;
			m_screen2_address = addr & 0x3fff;
		}
	}
	else if (m_start1change && (m_start1change == (m_linecounter / m_scanline_per_char_row)))
	{
		m_address = m_screen1_address;
		m_start1change = 0;
	}

	if (dw == 2)
		tilerow >>= 1;
	else if (dw == 3)
		tilerow = (charrow + m_scanline_per_char_row) >> 1;

	uint16_t address = m_address;

	const bool mbc = (charrow == 0) && (m_buffer_mode_select == 3);
	const bool blink_on = (screen().frame_number() & (m_character_blink_rate_divisor >> 1)) != 0;
	for (int i = 0; i < m_character_per_row; i++)
	{
		u8 charcode, attrcode = 0;
		if (mbc && !m_mbc_char_cb.isnull())
		{
			// row buffering DMA
			charcode = m_mbc_char_cb(address);
			m_char_space->write_byte(address, charcode);
			if (m_attr_space != nullptr && !m_mbc_attr_cb.isnull())
			{
				attrcode = m_mbc_attr_cb(address);
				m_attr_space->write_byte(address, attrcode);
			}
		}
		else
		{
			charcode = m_char_space->read_byte(address);
			if (m_attr_space != nullptr)
				attrcode = m_attr_space->read_byte(address);
		}

		if (m_display_enabled && !m_display_cb.isnull())
		{
			bool cursor_on = ((address & 0x3fff) == m_cursor_address)
				&& m_cursor_enabled
				&& (charrow >= m_cursor_first_scanline)
				&& (charrow <= m_cursor_last_scanline)
				&& (!m_cursor_blink || (screen().frame_number() & (m_cursor_rate_divisor >> 1)) != 0);
			m_display_cb(m_bitmap,
							i * m_hpixels_per_column,
							m_linecounter,
							tilerow,
							charcode,
							attrcode,
							address,
							cursor_on,
							dw != 0,
							m_gfx_enabled,
							charrow == m_cursor_underline_position,
							blink_on);

		}
		address = (address + 1) & 0xffff;

		if (address > ((m_display_buffer_last_address << 10) | 0x3ff))
			address = m_display_buffer_first_address;
	}

	if (!m_display_enabled)
		std::fill_n(&m_bitmap.pix32(m_linecounter), m_character_per_row * m_hpixels_per_column, rgb_t::black());

	if (m_gfx_enabled || (charrow == (m_scanline_per_char_row - 1)))
		m_address = address;
}

TIMER_CALLBACK_MEMBER(scn2674_device::breq_timer)
{
	LOGMASKED(LOG_INTR, "BREQ %sasserted at line %d\n", (param == ASSERT_LINE) ? "" : "de", m_linecounter);
	m_breq_cb(param);
	if (param == CLEAR_LINE)
		m_mbc_cb(0);
}

TIMER_CALLBACK_MEMBER(scn2674_device::vblank_timer)
{
	m_status_register |= 0x10;
	if (BIT(m_irq_mask, 4))
	{
		LOGMASKED(LOG_INTR, "V-Blank interrupt at line %d\n", m_linecounter);
		m_irq_register |= 0x10;
		m_intr_cb(ASSERT_LINE);
	}
}

uint32_t scn2674_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}
