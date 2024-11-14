// license:BSD-3-Clause
// copyright-holders:James Wallace, blueonesarefaster
/**********************************************************************

    This is derived from bfm_bda by James Wallace.

    Bell Fruit Games 96x8 Dot matrix VFD module interface and emulation.

    This was a replacement for the previous 5 x 7 x 16 display.
    The two displays are meant to be compatible although there are some
    differences in behaviour when sending undocumented commands or
    commands with "don't care" bits.

    TODO: Scrolling text
          Test sequence
          Background character
          Background colour enable/disable/protection
          LED backlight isn't visually correct. The area outside the 96x8
          matrix should be brighter than that shining through the gap
          between the digits.
**********************************************************************/

#include "emu.h"
#include "bfm_gu96x8m_k657c2.h"

DEFINE_DEVICE_TYPE(BFM_GU96X8M_K657C2, bfm_gu96x8m_k657c2_device, "bfm_gu96x8m_k657c2", "BFG 192x8 VFD controller")

static const uint8_t charset[][6]=
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x12, 0x7e, 0x92, 0x82, 0x42, 0x00 }, // pound
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // space
	{ 0x00, 0x00, 0xf2, 0x00, 0x00, 0x00 }, // !
	{ 0x00, 0xc0, 0x00, 0xc0, 0x00, 0x00 }, // "
	{ 0x28, 0x7c, 0x28, 0x7c, 0x28, 0x00 }, // #
	{ 0x20, 0x54, 0xfe, 0x54, 0x08, 0x00 }, // $
	{ 0xc6, 0xc8, 0x10, 0x26, 0xc6, 0x00 }, // %
	{ 0x2c, 0x52, 0x2a, 0x04, 0x0a, 0x00 }, // &
	{ 0x00, 0x00, 0x20, 0x40, 0x80, 0x00 }, // '
	{ 0x00, 0x38, 0x44, 0x82, 0x00, 0x00 }, // (
	{ 0x00, 0x82, 0x44, 0x38, 0x00, 0x00 }, // )
	{ 0x54, 0x38, 0x10, 0x38, 0x54, 0x00 }, // *
	{ 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00 }, // +
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }, // .
	{ 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 }, // -
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }, // .
	{ 0x06, 0x08, 0x10, 0x20, 0xc0, 0x00 }, // /
	{ 0x7c, 0x82, 0x82, 0x82, 0x7c, 0x00 }, // 0
	{ 0x00, 0x42, 0xfe, 0x02, 0x00, 0x00 }, // 1
	{ 0x42, 0x86, 0x8a, 0x92, 0x62, 0x00 }, // 2
	{ 0x44, 0x82, 0x92, 0x92, 0x6c, 0x00 }, // 3
	{ 0x18, 0x28, 0x48, 0xfe, 0x08, 0x00 }, // 4
	{ 0xe4, 0xa2, 0xa2, 0xa2, 0x9c, 0x00 }, // 5
	{ 0x3c, 0x52, 0x92, 0x12, 0x0c, 0x00 }, // 6
	{ 0x80, 0x80, 0x8e, 0x90, 0xe0, 0x00 }, // 7
	{ 0x6c, 0x92, 0x92, 0x92, 0x6c, 0x00 }, // 8
	{ 0x60, 0x90, 0x92, 0x94, 0x78, 0x00 }, // 9
	{ 0x00, 0x00, 0x44, 0x00, 0x00, 0x00 }, // :
	{ 0x00, 0x02, 0x44, 0x00, 0x00, 0x00 }, // semicolon
	{ 0x10, 0x28, 0x44, 0x82, 0x00, 0x00 }, // <
	{ 0x00, 0x28, 0x28, 0x28, 0x00, 0x00 }, // =
	{ 0x82, 0x44, 0x28, 0x10, 0x00, 0x00 }, // >
	{ 0x40, 0x80, 0x9a, 0xa0, 0x40, 0x00 }, // ?
	{ 0x7c, 0x82, 0xba, 0xaa, 0x78, 0x00 }, // @
	{ 0x7e, 0x90, 0x90, 0x90, 0x7e, 0x00 }, // A
	{ 0x82, 0xfe, 0x92, 0x92, 0x6c, 0x00 }, // B
	{ 0x7c, 0x82, 0x82, 0x82, 0x44, 0x00 }, // C
	{ 0x82, 0xfe, 0x82, 0x82, 0x7c, 0x00 }, // D
	{ 0xfe, 0x92, 0x92, 0x92, 0x82, 0x00 }, // E
	{ 0xfe, 0x90, 0x90, 0x90, 0x80, 0x00 }, // F
	{ 0x7c, 0x82, 0x82, 0x92, 0x5c, 0x00 }, // G
	{ 0xfe, 0x08, 0x08, 0x08, 0xfe, 0x00 }, // H
	{ 0x00, 0x82, 0xfe, 0x82, 0x00, 0x00 }, // I
	{ 0x04, 0x02, 0x82, 0xfc, 0x80, 0x00 }, // J
	{ 0xfe, 0x10, 0x28, 0x44, 0x82, 0x00 }, // K
	{ 0xfe, 0x02, 0x02, 0x02, 0x02, 0x00 }, // L
	{ 0xfe, 0x40, 0x30, 0x40, 0xfe, 0x00 }, // M
	{ 0xfe, 0x20, 0x10, 0x08, 0xfe, 0x00 }, // N
	{ 0x7c, 0x82, 0x82, 0x82, 0x7c, 0x00 }, // O
	{ 0xfe, 0x90, 0x90, 0x90, 0x60, 0x00 }, // P
	{ 0x7c, 0x82, 0x8a, 0x86, 0x7e, 0x00 }, // Q
	{ 0xfe, 0x90, 0x98, 0x94, 0x62, 0x00 }, // R
	{ 0x64, 0x92, 0x92, 0x92, 0x4c, 0x00 }, // S
	{ 0x80, 0x80, 0xfe, 0x80, 0x80, 0x00 }, // T
	{ 0xfc, 0x02, 0x02, 0x02, 0xfc, 0x00 }, // U
	{ 0xf8, 0x04, 0x02, 0x04, 0xf8, 0x00 }, // V
	{ 0xfc, 0x02, 0x0c, 0x02, 0xfc, 0x00 }, // W
	{ 0xc6, 0x28, 0x10, 0x28, 0xc6, 0x00 }, // X
	{ 0xe0, 0x10, 0x0e, 0x10, 0xe0, 0x00 }, // Y
	{ 0x86, 0x8a, 0x92, 0xa2, 0xc2, 0x00 }, // Z
	{ 0x00, 0xfe, 0x82, 0x82, 0x00, 0x00 }, // [
	{ 0x40, 0x20, 0x10, 0x08, 0x04, 0x00 }, // back slash
	{ 0x00, 0x82, 0x82, 0xfe, 0x00, 0x00 }, // ]
	{ 0x20, 0x40, 0x80, 0x40, 0x20, 0x00 }, // ^
	{ 0x02, 0x02, 0x02, 0x02, 0x02, 0x00 }, // _
	{ 0x80, 0x40, 0x20, 0x00, 0x00, 0x00 }, // `
	{ 0x04, 0x2a, 0x2a, 0x2a, 0x1e, 0x00 }, // a
	{ 0xfe, 0x12, 0x12, 0x12, 0x0c, 0x00 }, // b
	{ 0x1c, 0x22, 0x22, 0x22, 0x14, 0x00 }, // c
	{ 0x0c, 0x12, 0x12, 0x12, 0xfe, 0x00 }, // d
	{ 0x1c, 0x2a, 0x2a, 0x2a, 0x18, 0x00 }, // e
	{ 0x00, 0x10, 0x7e, 0x90, 0x40, 0x00 }, // f
	{ 0x12, 0x2a, 0x2a, 0x2a, 0x3c, 0x00 }, // g
	{ 0xfe, 0x10, 0x10, 0x10, 0x0e, 0x00 }, // h
	{ 0x00, 0x22, 0xbe, 0x02, 0x00, 0x00 }, // i
	{ 0x04, 0x02, 0x02, 0xbc, 0x00, 0x00 }, // j
	{ 0xfe, 0x08, 0x14, 0x22, 0x00, 0x00 }, // k
	{ 0x00, 0x82, 0xfe, 0x02, 0x00, 0x00 }, // l
	{ 0x3e, 0x20, 0x1e, 0x20, 0x1e, 0x00 }, // m
	{ 0x1e, 0x20, 0x20, 0x3e, 0x00, 0x00 }, // n
	{ 0x1c, 0x22, 0x22, 0x22, 0x1c, 0x00 }, // o
	{ 0x3e, 0x28, 0x28, 0x28, 0x10, 0x00 }, // p
	{ 0x10, 0x28, 0x28, 0x28, 0x3e, 0x00 }, // q
	{ 0x00, 0x3e, 0x10, 0x20, 0x20, 0x00 }, // r
	{ 0x10, 0x2a, 0x2a, 0x2a, 0x04, 0x00 }, // s
	{ 0x20, 0x20, 0xfc, 0x22, 0x24, 0x00 }, // t
	{ 0x3c, 0x02, 0x02, 0x02, 0x3e, 0x00 }, // u
	{ 0x38, 0x04, 0x02, 0x04, 0x38, 0x00 }, // v
	{ 0x3c, 0x02, 0x3c, 0x02, 0x3c, 0x00 }, // w
	{ 0x22, 0x14, 0x08, 0x14, 0x22, 0x00 }, // x
	{ 0x00, 0x32, 0x0a, 0x0a, 0x3c, 0x00 }, // y
	{ 0x22, 0x26, 0x2a, 0x32, 0x22, 0x00 }, // z
	{ 0x00, 0x10, 0xee, 0x82, 0x00, 0x00 }, // {
	{ 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00 }, // |
	{ 0x00, 0x82, 0xee, 0x10, 0x00, 0x00 }, // }
	{ 0x10, 0x20, 0x10, 0x08, 0x10, 0x00 }, // ~
	{ 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0x00 }, // filled block
	{ 0x1e, 0xa8, 0x28, 0xa8, 0x1e, 0x00 }, // accented A
	{ 0x1e, 0x68, 0xa8, 0x28, 0x1e, 0x00 }, //
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //
	{ 0x1e, 0x28, 0xa8, 0x68, 0x1e, 0x00 }, //
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // accented E
	{ 0x3e, 0x6a, 0xaa, 0x2a, 0x2a, 0x00 }, //
	{ 0x3e, 0x6a, 0xaa, 0x6a, 0x2a, 0x00 }, //
	{ 0x3e, 0x2a, 0xaa, 0x6a, 0x2a, 0x00 }, //
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // accented I
	{ 0x00, 0x62, 0xbe, 0x22, 0x00, 0x00 }, //
	{ 0x00, 0x62, 0xbe, 0x62, 0x00, 0x00 }, //
	{ 0x00, 0x22, 0xbe, 0x62, 0x00, 0x00 }, //
	{ 0x1c, 0xa2, 0x22, 0xa2, 0x1c, 0x00 }, // accented O
	{ 0x1c, 0x62, 0xa2, 0x22, 0x1c, 0x00 }, //
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //
	{ 0x1c, 0x62, 0xa2, 0x62, 0x1c, 0x00 }, //
	{ 0x3c, 0x82, 0x02, 0x82, 0x3c, 0x00 }, // accented U
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //
	{ 0x3c, 0x02, 0x82, 0x42, 0x3c, 0x00 }  //
};

enum
{
	LOAD_COMPLETE = 0,
	LOAD_USER_DEFINED_CHAR,
	LOAD_LED_COLOUR,
	LOAD_LED_FLASH_CONTROL,
	LOAD_LED_FREQUENCY,
	LOAD_GRAPHIC_RANGE_START,
	LOAD_GRAPHIC_RANGE_END,
	LOAD_GRAPHIC_DATA,
	LOAD_BRIGHTNESS
};

enum
{
	AT_FLASH = 0,
	AT_DP = 1,
	AT_GRAPHICS = 2,
	CHAR_AT_UDF = 7
};

bfm_gu96x8m_k657c2_device::bfm_gu96x8m_k657c2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BFM_GU96X8M_K657C2, tag, owner, clock)
	, m_vfd_background(*this,"vfdbackground0")
	, m_dotmatrix(*this, "vfddotmatrix%u", 0U)
	, m_duty(*this, "vfdduty%u", 0U)
{
}

void bfm_gu96x8m_k657c2_device::device_start()
{
	m_vfd_background.resolve();
	m_dotmatrix.resolve();
	m_duty.resolve();

	m_frame_timer = timer_alloc(FUNC(bfm_gu96x8m_k657c2_device::frame_update_callback), this);

	save_item(NAME(m_cursor_pos));
	save_item(NAME(m_window_start));
	save_item(NAME(m_window_end));
	save_item(NAME(m_window_size));
	save_item(NAME(m_shift_count));
	save_item(NAME(m_shift_data));
	save_item(NAME(m_pcursor_pos));
	save_item(NAME(m_brightness));
	save_item(NAME(m_display_mode));
	save_item(NAME(m_scroll_active));
	save_item(NAME(m_led_flash_enabled));
	save_item(NAME(m_extended_commands_enabled));
	save_item(NAME(m_graphics_commands_enabled));
	save_item(NAME(m_led_colour));
	save_item(NAME(m_graphics_data));
	save_item(NAME(m_graphics_start));
	save_item(NAME(m_graphics_end));
	save_item(NAME(m_cursor));
	save_item(NAME(m_chars));
	save_item(NAME(m_attributes));
	save_item(NAME(m_charset_offset));
	save_item(NAME(m_extra_data));
	save_item(NAME(m_udf));
	save_item(NAME(m_blank_control));
	save_item(NAME(m_flash_blank));
	save_item(NAME(m_flash_rate));
	save_item(NAME(m_flash_count));
	save_item(NAME(m_led_flash_blank));
	save_item(NAME(m_led_flash_rate));
	save_item(NAME(m_led_flash_count));
	save_item(NAME(m_ascii_charset));
	save_item(NAME(m_extra_data_count));
	save_item(NAME(m_load_extra_data));
}

void bfm_gu96x8m_k657c2_device::device_reset()
{
	m_cursor = 0;
	m_cursor_pos = 0;
	m_window_start = 0;
	m_window_end = 15;
	m_window_size = 16;
	m_shift_count = 0;
	m_shift_data = 0;
	m_pcursor_pos = 0;
	m_brightness = 0;
	m_display_mode = 0;
	m_scroll_active = 0;
	m_blank_control = 3;
	m_flash_blank = 0;
	m_led_flash_enabled = 0;
	m_extended_commands_enabled = 0;
	m_graphics_commands_enabled = 0;
	m_led_colour = 0;
	m_flash_rate = 0;
	m_flash_count = 0;
	m_ascii_charset = 0;
	m_extra_data_count = 0;
	m_load_extra_data = LOAD_COMPLETE;
	m_graphics_start = 0;
	m_graphics_end = 0;

	std::fill(std::begin(m_chars), std::end(m_chars), ' ');
	std::fill(std::begin(m_attributes), std::end(m_attributes), 0);
	std::fill(std::begin(m_extra_data), std::end(m_extra_data), 0);
	std::fill(m_udf[0], m_udf[0]+(16*6),0);
	std::fill(std::begin(m_graphics_data), std::end(m_graphics_data), 0);


	for(int i = 0; i < 32; i++)
	{
		m_charset_offset[i] = i | 0x40;
	}
	for(int i = 32; i < 64; i++)
	{
		m_charset_offset[i] = i;
	}
	m_charset_offset[0x21] = 0x1f; // replace ! with £
	std::copy(m_charset_offset, m_charset_offset + 64, m_charset_offset + 64);

	m_frame_timer->adjust(attotime::from_hz(253), 0, attotime::from_hz(253));
}

void bfm_gu96x8m_k657c2_device::device_post_load()
{
	update_display();
}

void bfm_gu96x8m_k657c2_device::update_display()
{
	if(m_led_flash_enabled && m_led_flash_blank)
	{
		m_vfd_background[0] = 0;
	}
	else
	{
		m_vfd_background[0] = m_led_colour;
	}

	for(int pos = 0;pos < 16;pos++)
	{
		uint8_t const *char_data;
		bool dp = false;
		bool blanked = false;

		switch(m_blank_control)
		{
			case 0:
				blanked = true;
				break;

			case 1:
				if(m_window_size == 0 || pos < m_window_start || pos > m_window_end)
				{
					blanked = true;
				}
				break;

			case 2:
				if(m_window_size > 0 && (pos >= m_window_start && pos <= m_window_end))
				{
					blanked = true;
				}
				break;
		}

		if(BIT(m_attributes[pos], AT_GRAPHICS))
		{
			for(int x = 0; x < 6; x++)
			{
				for(int y = 0; y < 8; y++)
				{
					m_dotmatrix[(y * 96) + (pos * 6) + x] = BIT(m_graphics_data[(pos * 6) + x],7 - y);
				}
			}
		}
		else
		{
			if((BIT(m_attributes[pos], AT_FLASH) && m_flash_blank) || blanked)
			{
				char_data = &charset[0][0];
			}
			else if(BIT(m_chars[pos], CHAR_AT_UDF))
			{
				char_data = &m_udf[m_chars[pos] & 0x0f][0];
				dp = BIT(m_attributes[pos], AT_DP);
			}
			else
			{
				char_data = &charset[m_chars[pos]][0];
				dp = BIT(m_attributes[pos], AT_DP);
			}

			for(int y = 0; y < 8; y++)
			{
				for(int x = 0; x < 6; x++)
				{
					if(y == 7 && x == 5)
					{
						m_dotmatrix[(y * 96) + (pos * 6) + x] = BIT(char_data[x], 7 - y) | dp;
					}
					else
					{
						m_dotmatrix[(y * 96) + (pos * 6) + x] = BIT(char_data[x], 7 - y);
					}
				}
			}
		}
	}
	m_duty[0] = m_brightness;
}

void bfm_gu96x8m_k657c2_device::write_char(int data)
{
	if(m_load_extra_data)
	{
		switch(m_load_extra_data)
		{
			case LOAD_USER_DEFINED_CHAR:
				m_extra_data_count--;
				m_extra_data[m_extra_data_count] = data;
				if(m_extra_data_count == 0)
				{
					uint8_t *udf = &m_udf[m_extra_data[6] & 0x0f][0];

					std::fill(udf,udf + 6,0);

					m_charset_offset[m_extra_data[5] & 0x7f] = (m_extra_data[6] & 0x0f) | (1 << CHAR_AT_UDF);

					udf[5] = BIT(m_extra_data[0], 7) | BIT(m_extra_data[0], 6);

					for(int i = 0; i < 35; i++)
					{
						udf[4 - (i % 5)] |= BIT(m_extra_data[(i + 2) / 8], 7 - ((i + 2) % 8)) << ((i / 5) + 1);
					}
					m_load_extra_data = LOAD_COMPLETE;
				}
				break;

			case LOAD_LED_COLOUR:
				m_load_extra_data = LOAD_COMPLETE;

				switch(data)
				{
					case 0xda:
						m_led_colour |= 1;
						break;

					case 0xdb:
						m_led_colour &= ~1;
						break;

					case 0xde:
						m_led_colour |= 2;
						break;

					case 0xdf:
						m_led_colour &= ~2;
						break;

					case 0xdc:
						m_led_colour |= 4;
						break;

					case 0xdd:
						m_led_colour &= ~4;
						break;
				}
				break;

			case LOAD_LED_FREQUENCY:
				m_load_extra_data = LOAD_COMPLETE;

				if(data >= 0xda && data <= 0xdf)
				{
					static constexpr uint16_t rates[]={8, 12, 20, 32, 64, 128 };

					m_led_flash_rate = rates[data - 0xda];
				}
				break;

			case LOAD_LED_FLASH_CONTROL:
				m_load_extra_data = LOAD_COMPLETE;

				if(data == 0xda)
				{
					m_led_flash_enabled = 0;
				}
				else if(data == 0xdb)
				{
					m_led_flash_enabled = 1;
				}
				break;

			case LOAD_GRAPHIC_RANGE_START:
				m_load_extra_data = LOAD_GRAPHIC_RANGE_END;

				m_graphics_start = data ;
				if(m_graphics_start > 95)
				{
					m_graphics_start = 95;
				}
				break;

			case LOAD_GRAPHIC_RANGE_END:
				m_graphics_end = data + 1 ;
				if(m_graphics_end > 96)
				{
					m_graphics_end = 96;
				}
				if(m_graphics_start < m_graphics_end)
				{
					m_load_extra_data = LOAD_GRAPHIC_DATA;
					m_extra_data_count = m_graphics_end - m_graphics_start;
				}
				else
				{
					m_load_extra_data = LOAD_COMPLETE;
				}
				break;

			case LOAD_GRAPHIC_DATA:
				m_graphics_data[m_graphics_end - m_extra_data_count] = data;
				m_extra_data_count--;
				if(m_extra_data_count==0)
				{
					m_load_extra_data = LOAD_COMPLETE;
				}
				break;

			case LOAD_BRIGHTNESS:
				m_load_extra_data = LOAD_COMPLETE;
				m_brightness = data & 7;
				break;

			default:
				m_load_extra_data = LOAD_COMPLETE;
				break;
		}
	}
	else
	{
		if(!BIT(data,7))
		{
			setdata(data);
		}
		else
		{
			switch(data & 0xf0)
			{
				case 0x80:
					if(data <= 0x83)
					{
						m_blank_control = data & 3;
					}
					else if(data == 0x84)
					{
						m_load_extra_data = LOAD_BRIGHTNESS;
					}
					else if(data == 0x85)
					{
						m_extended_commands_enabled = 1;
					}
					else if(m_extended_commands_enabled)
					{
						if(data == 0x86)
						{
							m_graphics_commands_enabled = 1;
						}
						else if(data == 0x87)
						{
							m_graphics_commands_enabled = 0;
						}
					}
					break;

				case 0x90:
					m_cursor_pos = data & 0x0f;
					m_pcursor_pos = m_cursor_pos;

					m_scroll_active = 0;
					if(m_display_mode == 2)
					{
						if(m_cursor_pos == m_window_end)
						{
							m_scroll_active = 1;
						}
					}
					else if(m_display_mode == 3)
					{
						if(m_cursor_pos == m_window_start)
						{
							m_scroll_active = 1;
						}
					}
					break;

				case 0xa0:
					if(data <= 0xa3)
					{
						m_display_mode = data & 0x03;
						m_scroll_active = 0;
						if(m_display_mode == 2)
						{
							if(m_cursor_pos == m_window_end)
							{
								m_scroll_active = 1;
							}
						}
						else if(m_display_mode == 3)
						{
							if(m_cursor_pos == m_window_start)
							{
								m_scroll_active = 1;
							}
						}
					}
					else if(data == 0xa4)
					{
						if(m_graphics_commands_enabled)
						{
							if(m_window_size > 0)
							{
								for(int i = 0; i < m_window_size; i++)
								{
									m_attributes[m_window_start + i] |= (1 << AT_GRAPHICS);
								}
							}
						}
					}
					else if(data == 0xa5)
					{
						if(m_graphics_commands_enabled)
						{
							std::fill(std::begin(m_graphics_data), std::end(m_graphics_data), 0);
						}
					}
					else if(data == 0xa8)
					{
						if(m_extended_commands_enabled)
						{
							m_load_extra_data = LOAD_USER_DEFINED_CHAR;
							m_extra_data_count = 7;
						}
					}
					else if(data == 0xae)
					{
						if(m_graphics_commands_enabled)
						{
							m_load_extra_data = LOAD_GRAPHIC_RANGE_START;
						}
					}
					break;

				case 0xb0:
				{
					if(data <= 0xb3)
					{
						switch(data & 3)
						{
							case 0x01:  // clr inside window
								if(m_window_size > 0)
								{
									std::fill_n(m_chars + m_window_start, m_window_size, ' ');
									std::fill_n(m_attributes + m_window_start, m_window_size, 0);
								}
								break;

							case 0x02:  // clr outside window
								if(m_window_size > 0)
								{
									std::fill_n(m_chars, m_window_start, ' ');
									std::fill_n(m_chars+m_window_end, 16-m_window_end, ' ');
									std::fill_n(m_attributes, m_window_start, 0);
									std::fill_n(m_attributes+m_window_end, 16-m_window_end, 0);
								}
								break;

							case 0x03:  // clr entire display
								std::fill(std::begin(m_chars), std::end(m_chars), ' ');
								std::fill(std::begin(m_attributes), std::end(m_attributes), 0);
								break;
						}
					}
					else if(data == 0xbc)
					{
						if(m_extended_commands_enabled)
						{
							m_ascii_charset = 1;
							for(int i = 0; i < 128; i++)
							{
								m_charset_offset[i] = i;
								if(i < 16)
								{
									m_charset_offset[i] |= (1 << CHAR_AT_UDF);
								}
							}
						}
					}
				}
				break;

			case 0xc0:
				static const uint8_t flash_rates[]={0, 6, 9, 15, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84};
				m_flash_rate = flash_rates[data & 0x0f];
				break;

			case 0xd0:
				if(data <= 0xd3)
				{
					for(int i = 0; i < 16; i++)
					{
						m_attributes[i] &= ~(1 << AT_FLASH);
					}

					switch(data & 3)
					{
						case 0x01:
							if(m_window_size > 0)
							{
								for(int i = 0; i < m_window_size; i++)
								{
									m_attributes[m_window_start + i] |= (1 << AT_FLASH);
								}
							}
							break;

						case 0x02:
							if(m_window_size)
							{
								if(m_window_start > 0)
								{
									for(int i = 0; i < m_window_start; i++)
									{
										m_attributes[i] |= (1 << AT_FLASH);
									}
								}

								if(m_window_end < 15)
								{
									for(int i = m_window_end + 1; i < 16; i++)
									{
										m_attributes[i] |= (1 << AT_FLASH);
									}
								}
							}
							break;

						case 0x03:
							for(int i = 0; i < 16; i++)
							{
								m_attributes[i] |= (1 << AT_FLASH);
							}
							break;
					}
				}
				else if(data == 0xdd)
				{
					m_load_extra_data = LOAD_LED_FREQUENCY;
				}
				else if(data == 0xde)
				{
					m_load_extra_data = LOAD_LED_FLASH_CONTROL;
				}
				else if(data == 0xdf)
				{
					m_load_extra_data = LOAD_LED_COLOUR;
				}
				break;

			case 0xe0:
				m_window_start = data & 0x0f;
				if(m_window_end >= m_window_start)
				{
					m_window_size = (m_window_end - m_window_start) + 1;
					m_scroll_active = 0;
					if(m_display_mode == 2)
					{
						if(m_cursor_pos == m_window_end)
						{
							m_scroll_active = 1;
						}
					}
					else if(m_display_mode == 3)
					{
						if(m_cursor_pos == m_window_start)
						{
							m_scroll_active = 1;
						}
					}
				}
				else
				{
					m_window_size = 0;
				}
				break;

			case 0xf0:
				m_window_end = data & 0x0f;
				if(m_window_end >= m_window_start)
				{
					m_window_size = (m_window_end - m_window_start) + 1;
					m_scroll_active = 0;
					if(m_display_mode == 2)
					{
						if(m_cursor_pos == m_window_end)
						{
							m_scroll_active = 1;
						}
					}
					else if(m_display_mode == 3)
					{
						if(m_cursor_pos == m_window_start)
						{
							m_scroll_active = 1;
						}
					}
				}
				else
				{
					m_window_size = 0;
				}
				break;
			}
		}
	}
}

void bfm_gu96x8m_k657c2_device::setdata(int data)
{
	int move = 0;
	int change =0;

	switch(data)
	{
		case 0x25: // undefined
			move++;
			break;

		case 0x26:
			if(m_ascii_charset)
			{
				change++;
			}
			move++;
			break;

		case 0x2c:  // semicolon
		case 0x2e:  // decimal point
			m_attributes[m_pcursor_pos] |= (1 << AT_DP);
			m_attributes[m_pcursor_pos] &= ~(1 << AT_GRAPHICS);
			break;

		case 0x3a:
		case 0x3b:
			if(m_ascii_charset)
			{
				change++;
			}
			move++;
			break;

		default:
			move++;
			change++;
	}

	if(move)
	{
		uint8_t mode = m_display_mode;

		m_pcursor_pos = m_cursor_pos;

		if(m_window_size == 0)
		{ // if no window selected default to equivalent rotate mode
				if(mode == 2)      mode = 0;
				else if(mode == 3) mode = 1;
		}
		switch(mode)
		{
			case 0: // rotate left
				if(change)
				{
					set_char(data);
				}
				m_cursor_pos++;
				if(m_cursor_pos >= 16)
				{
					m_cursor_pos = 0;
				}
				break;

			case 1: // Rotate right
				if(change)
				{
					set_char(data);
				}
				if(m_cursor_pos>0)
				{
					m_cursor_pos--;
				}
				else
				{
					m_cursor_pos = 15;
				}
				break;

			case 2: // Scroll left
				if(m_cursor_pos < m_window_end)
				{
					if(change)
					{
						set_char(data);
					}
					m_cursor_pos++;
				}
				else
				{
					if(move)
					{
						if(m_scroll_active)
						{
							std::copy(m_chars + m_window_start + 1, m_chars + m_window_start + m_window_size, m_chars + m_window_start);
							std::copy(m_attributes + m_window_start + 1, m_attributes + m_window_start + m_window_size, m_attributes + m_window_start);
						}
						else
						{
							m_scroll_active=1;
						}
					}

					if(change)
					{
						set_char(data);
					}
				}
				break;

			case 3: // Scroll right
				if(m_cursor_pos > m_window_start)
				{
					if(change)
					{
						set_char(data);
					}
					if(m_cursor_pos>0)
					{
						m_cursor_pos--;
					}
				}
				else
				{
					if(move)
					{
						if(m_scroll_active)
						{
							std::move(m_chars + m_window_start, m_chars + m_window_start + m_window_size - 1, m_chars + m_window_start + 1);
							std::move(m_attributes + m_window_start, m_attributes + m_window_start + m_window_size - 1, m_attributes + m_window_start + 1);
						}
						else
						{
							m_scroll_active=1;
						}
					}
					if(change)
					{
						set_char(data);
					}
				}
				break;
		}
	}
}

void bfm_gu96x8m_k657c2_device::set_char(int data)
{
	m_chars[m_cursor_pos]=m_charset_offset[data];
	m_attributes[m_cursor_pos]=0;

	if(m_ascii_charset == 0 && (data == 0x6c || data == 0x6e))
	{
		m_attributes[m_cursor_pos] |= (1 << AT_DP);
	}
}

TIMER_CALLBACK_MEMBER(bfm_gu96x8m_k657c2_device::frame_update_callback)
{
	if(m_flash_rate)
	{
		m_flash_count++;
		if(m_flash_count >= m_flash_rate)
		{
			m_flash_count = 0;
			m_flash_blank = !m_flash_blank;
		}
	}
	else
	{
		m_flash_blank = 0;
	}

	if(m_led_flash_enabled)
	{
		if(m_led_flash_rate)
		{
			m_led_flash_count++;
			if(m_led_flash_count >= m_led_flash_rate)
			{
				m_led_flash_count = 0;
				m_led_flash_blank = !m_led_flash_blank;
			}
		}
		else
		{
			m_led_flash_blank = 0;
		}
	}
	else
	{
		m_led_flash_blank=0;
	}

	update_display();
}

void bfm_gu96x8m_k657c2_device::shift_data(int data)
{
	m_shift_data <<= 1;

	if(!data) m_shift_data |= 1;

	if(++m_shift_count >= 8)
	{
		write_char(m_shift_data);
		m_shift_count = 0;
		m_shift_data  = 0;
	}
}
