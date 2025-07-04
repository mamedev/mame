// license:BSD-3-Clause
// copyright-holders:J.Wallace

/*************************************************************************

    ldp1450hle.cpp

    HLE for the LDP 1000 series, such as LDP-1450
    NTSC for now, PAL (P suffix) would set base speed to 50, at the least


**************************************************************************

    To do:

        * On-screen display support needs aligning with real hardware
        * Better CLV support
        * Chapter-search support
        * Repeat support (we only store the command)
        * Commands that Time Traveler doesn't use:
          - Scan Forward/Reverse
          - Multitrack-Jump Forward/Reverse
          - Clear
          - Leadout Symbol
          - OSD
          - Status Requests
        * Repeat behaviour for reverse play should restart in reverse
        * Delay timing of queue is a guess based on LDP1000A guide
        * Not all features are fully hooked up
        * Still step back and forth in Time Traveler glitches
          - (level select doesn't stay in place)
        * resizing, positioning of OSD bitmaps is not optimal
          - 16x16 is supported, but native size and location may be wrong
*************************************************************************/

#include "emu.h"
#include "ldp1450hle.h"


#define LOG_COMMAND_BYTES       (1U << 1)
#define LOG_COMMANDS            (1U << 2)
#define LOG_COMMAND_BUFFERS     (1U << 3)
#define LOG_REPLIES             (1U << 4)
#define LOG_REPLY_BYTES         (1U << 5)
#define LOG_SEARCHES            (1U << 6)
#define LOG_STOPS               (1U << 7)
#define LOG_SQUELCHES           (1U << 8)
#define LOG_FRAMES              (1U << 9)
#define LOG_ALL                 (LOG_COMMAND_BYTES | LOG_COMMANDS | LOG_COMMAND_BUFFERS | LOG_REPLY_BYTES | LOG_SEARCHES | LOG_STOPS | LOG_SQUELCHES | LOG_FRAMES)


#define VERBOSE (LOG_COMMANDS)

#include "logmacro.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(SONY_LDP1450HLE, sony_ldp1450hle_device, "ldp1450hle", "Sony LDP-1450 HLE")


//-------------------------------------------------
//  sony_ldp1450hle_device - constructor
//-------------------------------------------------

sony_ldp1450hle_device::sony_ldp1450hle_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: laserdisc_device(mconfig, SONY_LDP1450HLE, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_serial_tx(*this)
	, m_vbi_fetch(nullptr)
	, m_cmd_running(false)
	, m_reply_write_index(0)
	, m_reply_read_index(0)
	, m_mode(MODE_PARK)
	, m_chapter(0)
	, m_time(0)
	, m_search_chapter(~u32(0))
	, m_search_frame(~u32(0))
	, m_mark_chapter(~u32(0))
	, m_mark_frame(~u32(0))
	, m_video_switch(1)
	, m_ch1_switch(false)
	, m_ch2_switch(false)
	, m_display_switch(0)
	, m_address_flag(ADDRESS_FRAME)
	, m_base_speed(60)
	, m_speed(60)
	, m_speed_accum(0)
	, m_curr_frame(0)
{
}

static const u8 text_size[0x04] =
{
	16,
	32,
	48,
	64
};

// bitmaps for the characters
static const u16 text_bitmap[0x60][0x10] =
{
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },                                                                                                          // <space>
	{0x0000,0x0000,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x00c0,0x0000,0x0000,0x00c0,0x00c0},// !
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 },
	{0x0000,0x0000,0x00f0,0x01f8,0x039c,0x038c,0x01cc,0x00fc,0x30fc,0x3878,0x1cf8,0x0fcc,0x0786,0x0787,0x3ffe,0x3cfc}, // &
	{0x0000,0x0000,0x03c0,0x03c0,0x0300,0x0380,0x01c0,0x00c0,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}, // '
	{0x0000,0x0000,0x3000,0x3800,0x1c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x1c00,0x3800,0x3000}, // (
	{0x0000,0x0000,0x00c0,0x01c0,0x0380,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0380,0x01c0,0x00c0}, // )
	{0x0000,0x0000,0x0000,0x0000,0x0c00,0x0c00,0xccc0,0xedc0,0x7f80,0x3f00,0x3f00,0x7f80,0xedc0,0xccc0,0x0c00,0x0c00}, // *
	{0x0000,0x0000,0x0000,0x0000,0x0300,0x0300,0x0300,0x0300,0x3ff0,0x3ff0,0x0300,0x0300,0x0300,0x0300,0x0000,0x0000}, // +
	{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x03c0,0x03c0,0x0300,0x0380,0x01c0,0x00c0}, // ,
	{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1ff8,0x1ff8,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}, // -
	{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x03c0,0x03c0,0x03c0,0x03c0}, // .
	{0x0000,0x0000,0xc000,0xe000,0x7000,0x3800,0x1c00,0x0e00,0x0700,0x0380,0x01c0,0x00e0,0x0070,0x0038,0x001c,0x000c}, // /
	{0x0000,0x0000,0x3fc0,0x7fe0,0xe070,0xc030,0xc030,0xc030,0xc030,0xc030,0xc030,0xc030,0xc030,0xe070,0x7fe0,0x3fc0}, // 0
	{0x0000,0x0000,0x0c00,0x0c00,0x0fc0,0x0fc0,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00}, // 1
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x3000,0x3800,0x1fc0,0x0fe0,0x0070,0x0038,0x001c,0x000c,0x3ffc,0x3ffc}, // 2
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x3000,0x3800,0x1fc0,0x1fc0,0x3800,0x3000,0x300c,0x381c,0x1ff8,0x0ff0}, // 3
	{0x0000,0x0000,0x0f00,0x0f80,0x0dc0,0x0ce0,0x0c70,0x0c38,0x0c1c,0x0c0c,0x0c0c,0x0c0c,0x3ffc,0x3ffc,0x0c00,0x0c00}, // 4
	{0x0000,0x0000,0x3ffc,0x3ffc,0x000c,0x000c,0x0ffc,0x1ffc,0x3800,0x3000,0x3000,0x3000,0x300c,0x381c,0x1ff8,0x0ff0}, // 5
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x000c,0x000c,0x0ffc,0x1ffc,0x300c,0x300c,0x300c,0x381c,0x1ff8,0x0ff0}, // 6
	{0x0000,0x0000,0x3ffc,0x3ffc,0x3000,0x3800,0x1c00,0x0e00,0x0700,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300,0x0300}, // 7
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x300c,0x381c,0x1ff8,0x1ff8,0x381c,0x300c,0x300c,0x381c,0x1ff8,0x0ff0}, // 8
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x300c,0x301c,0x3ff8,0x3ff0,0x3000,0x3000,0x300c,0x381c,0x1ff8,0x0ff0}, // 9
	{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0180,0x0180,0x0000,0x0000,0x0180,0x0180,0x0000,0x0000,0x0000,0x0000}, // :
	{ 0 },
	{ 0 },
	{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1ff8,0x1ff8,0x0000,0x1ff8,0x1ff8,0x0000,0x0000,0x0000,0x0000}, // =
	{ 0 },
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x3000,0x3800,0x1c00,0x0e00,0x0700,0x0300,0x0000,0x0000,0x0300,0x0300}, // ?
	{ 0 },
	{0x0000,0x0000,0x03c0,0x07e0,0x0e70,0x1c38,0x381c,0x300c,0x300c,0x300c,0x3ffc,0x3ffc,0x300c,0x300c,0x300c,0x300c}, // A
	{0x0000,0x0000,0x1ffe,0x3ffe,0x7018,0x6018,0x6018,0x7018,0x3ff8,0x3ff8,0x7018,0x6018,0x6018,0x7018,0x3ffe,0x1ffe}, // B
	{0x0000,0x0000,0x3fc0,0x7fe0,0xe070,0xc030,0x0030,0x0030,0x0030,0x0030,0x0030,0x0030,0xc030,0xe070,0x7fe0,0x3fc0}, // C
	{0x0000,0x0000,0x1ffe,0x3ffe,0x7018,0x6018,0x6018,0x6018,0x6018,0x6018,0x6018,0x6018,0x6018,0x7018,0x3ffe,0x1ffe}, // D
	{0x0000,0x0000,0x3ffc,0x3ffc,0x000c,0x000c,0x000c,0x000c,0x0ffc,0x0ffc,0x000c,0x000c,0x000c,0x000c,0x3ffc,0x3ffc}, // E
	{0x0000,0x0000,0x3ffc,0x3ffc,0x000c,0x000c,0x000c,0x000c,0x0ffc,0x0ffc,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c}, // F
	{0x0000,0x0000,0x3fc0,0x7fe0,0xe070,0xc030,0x0030,0x0030,0xfc30,0xfc30,0xc030,0xc030,0xc030,0xe070,0x7fe0,0x3fc0}, // G
	{0x0000,0x0000,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x3ffc,0x3ffc,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c}, // H
	{0x0000,0x0000,0x07e0,0x07e0,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x07e0,0x07e0}, // I
	{0x0000,0x0000,0x3f00,0x3f00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c00,0x0c0c,0x0c0c,0x0c0c,0x0e1c,0x07f8,0x03f0}, // J
	{0x0000,0x0000,0x300c,0x380c,0x1c0c,0x0e0c,0x070c,0x038c,0x01fc,0x01fc,0x038c,0x070c,0x0e0c,0x1c0c,0x380c,0x300c}, // K
	{0x0000,0x0000,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c,0x3ffc,0x3ffc}, // L
	{0x0000,0x0000,0x6006,0x6006,0x781e,0x7c3e,0x6e76,0x67e6,0x63c6,0x6186,0x6186,0x6186,0x6006,0x6006,0x6006,0x6006}, // M
	{0x0000,0x0000,0x300c,0x300c,0x301c,0x303c,0x307c,0x30ec,0x31cc,0x338c,0x370c,0x3e0c,0x3c0c,0x380c,0x300c,0x300c}, // N
	{0x0000,0x0000,0x3fc0,0x7fe0,0xe070,0xc030,0xc030,0xc030,0xc030,0xc030,0xc030,0xc030,0xc030,0xe070,0x7fe0,0x3fc0}, // O
	{0x0000,0x0000,0x0ffc,0x1ffc,0x380c,0x300c,0x300c,0x380c,0x1ffc,0x0ffc,0x000c,0x000c,0x000c,0x000c,0x000c,0x000c}, // P
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x300c,0x300c,0x300c,0x300c,0x330c,0x3f0c,0x1e0c,0x1e1c,0x3ff8,0x33f0}, // Q
	{0x0000,0x0000,0x0ffc,0x1ffc,0x380c,0x300c,0x300c,0x380c,0x1ffc,0x0ffc,0x030c,0x070c,0x0e0c,0x1c0c,0x380c,0x300c}, // R
	{0x0000,0x0000,0x0ff0,0x1ff8,0x381c,0x300c,0x000c,0x001c,0x0ff8,0x1ff0,0x3800,0x3000,0x300c,0x381c,0x1ff8,0x0ff0}, // S
	{0x0000,0x0000,0x7ffe,0x7ffe,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180}, // T
	{0x0000,0x0000,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x1ff8,0x0ff0}, // U
	{0x0000,0x0000,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x300c,0x381c,0x1c38,0x0e70,0x07e0,0x03c0}, // V
	{0x0000,0x0000,0x6006,0x6006,0x6006,0x6006,0x6006,0x6186,0x6186,0x6186,0x6186,0x63c6,0x67e6,0x7e7e,0x3c3c,0x1818}, // W
	{0x0000,0x0000,0x6006,0x700e,0x381c,0x1c38,0x0e70,0x07e0,0x03c0,0x03c0,0x07e0,0x0e70,0x1c38,0x381c,0x700e,0x6006}, // X
	{0x0000,0x0000,0x6006,0x700e,0x381c,0x1c38,0x0e70,0x07e0,0x03c0,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180,0x0180}, // Y
	{0x0000,0x0000,0x3ffc,0x3ffc,0x1c00,0x0e00,0x0700,0x0380,0x01c0,0x00e0,0x0070,0x0038,0x001c,0x000c,0x3ffc,0x3ffc}, // Z
	{ 0 },                                  //
	{ 0 },                                  //
	{ 0 },                                  //
	{ 0 },                                  //
	{0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x1ff8,0x1ff8,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}, // -
};

#define OVERLAY_PIXEL_WIDTH             1+(1.3f / 720.0f)
#define OVERLAY_PIXEL_HEIGHT            1

//-------------------------------------------------
//  overlay_draw_group - draw a single group of
//  characters
//-------------------------------------------------

void sony_ldp1450hle_device::overlay_fill(bitmap_yuy16 &bitmap, uint8_t yval, uint8_t cr, uint8_t cb)
{
	uint16_t color0 = (yval << 8) | cb;
	uint16_t color1 = (yval << 8) | cr;

	// write 32 bits of color (2 pixels at a time)
	for (int y = 0; y < bitmap.height(); y++)
	{
		uint16_t *dest = &bitmap.pix(y);
		for (int x = 0; x < bitmap.width() / 2; x++)
		{
			*dest++ = color0;
			*dest++ = color1;
		}
	}
}

void sony_ldp1450hle_device::overlay_draw_group(bitmap_yuy16 &bitmap, const uint8_t *text, int start, int xstart, int ystart, int mode)
{
	u8 char_width = text_size[m_user_index_mode & 0x03];

	u8 char_height = text_size[(m_user_index_mode >> 2) & 0x03];

	float xstart_normalised = (bitmap.width()/2/ 64) * xstart;
	float ystart_normalised = (bitmap.height()/ 64) * ystart;

	if (m_user_index_mode & 0x80) //blue screen
	{
		overlay_fill(bitmap, 0x28, 0x6d, 0xf0);
	}

	u8 count;
	if (m_user_index_mode & 0x10) // 3 line mode
	{
		LOGMASKED(LOG_COMMANDS, "3 Line\n");
		int x = 0;
		for (int y = 0; y < 3; y++)
		{
			bool eos = false;
			for (int char_idx = x; char_idx < (x+10); char_idx++)
			{
				if (text[char_idx] == 0x1a || eos)
				{
					eos = true;
					x -= 9;// adjust pointer to account for the + 10 we do automatically
					break;
				}
				else
				{
					overlay_draw_char(bitmap, text[char_idx], xstart_normalised + (char_width  * (char_idx %10)), ystart_normalised + (y*char_height), char_width, char_height);
				}
			}
			x += 10;
		}
	}
	else
	{
		count = 0x20 - start;
		LOGMASKED(LOG_COMMANDS, "1 Line\n");

		for (int x = start; x < count; x++)
		{
			if (text[x] == 0x1a)
			{
				break;
			}
			else
			{
				overlay_draw_char(bitmap, text[x], xstart_normalised + ((char_width) * x), ystart_normalised, char_width, char_height);
			}
		}
	}
}


//-------------------------------------------------
//  overlay_draw_char - draw a single character
//  of the text overlay
//-------------------------------------------------

void sony_ldp1450hle_device::overlay_draw_char(bitmap_yuy16 &bitmap, uint8_t ch, float xstart, int ystart, int char_width, int char_height)
{

	// m_user_index_mode >> 5 & 0x04: 0,2 = normal, 1 = 1px shadow, 3 = grey box
	uint16_t black = 0x0080;

	u8 modeval = (m_user_index_mode >> 5) & 0x04;

	for (u32 y = 0; y < char_height; y++)
	{
		for (u8 x = 0; x < char_width; x++)
		{
			u32 xmin = xstart + x;
			for (u32 yy = 0; yy < OVERLAY_PIXEL_HEIGHT; yy++)
			{

				for (u32 xx = 0; xx < OVERLAY_PIXEL_WIDTH; xx++)
				{
					if (modeval == 0x03)
					{
						// FIXME: this is unreachable because modeval is masked with 0x04 above
						//fill with grey
					}

					if (m_osd_font[ch].pix(y,x) != black)
					{
						bitmap.pix(ystart + (y + 1) * OVERLAY_PIXEL_HEIGHT + yy, xmin+xx) = m_osd_font[ch].pix(y,x);
					}
				}
			}
		}
	}
}

void sony_ldp1450hle_device::player_overlay(bitmap_yuy16 &bitmap)
{
	if (m_user_index_flag)
	{
		overlay_draw_group(bitmap, m_user_index_chars, m_user_index_window_idx, m_user_index_x, m_user_index_y, m_user_index_mode);
	}
}


void sony_ldp1450hle_device::queue_reply(u8 reply, float delay)
{
	const u8 reply_buffer[5] = {reply, 0, 0, 0, 0};
	queue_reply_buffer(reply_buffer, delay);
}

void sony_ldp1450hle_device::queue_reply_buffer(const u8 reply[], float delay)
{
	const u8 max_writable = u8(std::size(m_reply_buffer));
	for (u8 i = 0; i < 5 && reply[i] != 0; i++)
	{
		m_reply_buffer[m_reply_write_index] = reply[i];
		m_reply_write_index = (m_reply_write_index + 1) % max_writable;
	}

	m_queue_timer->adjust(attotime::from_nsec(delay * 1000000));
}

TIMER_CALLBACK_MEMBER(sony_ldp1450hle_device::process_queue)
{
	LOGMASKED(LOG_REPLY_BYTES, "Sending reply byte: %02x\n", (u8)m_reply_buffer[m_reply_read_index]);
	transmit_register_setup(m_reply_buffer[m_reply_read_index]);
}

//-------------------------------------------------
//  bcd_to_literal - converts a BCD value used in
//  commands a direct numeric value
//-------------------------------------------------

u32 sony_ldp1450hle_device::bcd_to_literal(u32 bcd)
{
	u32 value = 0;
	u32 shift = 28;
	u32 multiplier = 10000000;
	for (u32 i = 0; i < 8; i++)
	{
		u32 digit = (bcd >> shift) & 0xf;
		bcd &= ~(0xf << shift);

		value += digit * multiplier;

		multiplier /= 10;
		shift -= 4;
	}
	return value;
}

//-------------------------------------------------
//  process_command - processes a single command
//  from the command buffer
//-------------------------------------------------

void sony_ldp1450hle_device::add_command_byte(u8 command)
{
	switch (m_submode)
	{
		case SUBMODE_USER_INDEX:
		{
			switch (command)
			{
				case 0x00:
				{
					m_submode = SUBMODE_USER_INDEX_MODE_1;
					queue_reply(0x0a, 4.3);
					break;
				}
				case 0x01:
				{
					m_submode = SUBMODE_USER_INDEX_STRING_1;
					queue_reply(0x0a, 4.3);
					break;
				}
				case 0x02:
				{
					m_submode = SUBMODE_USER_INDEX_WINDOW;
					queue_reply(0x0a, 4.3);
					break;
				}
			}
			break;
		}
		case SUBMODE_USER_INDEX_MODE_1:
		{
			m_user_index_x = command & 0x3f;

			LOGMASKED(LOG_COMMANDS, "User Index X: %02x\n", m_user_index_x);

			m_submode = SUBMODE_USER_INDEX_MODE_2;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_MODE_2:
		{
			m_user_index_y = command & 0x3f;
			LOGMASKED(LOG_COMMANDS, "User Index Y: %02x\n", m_user_index_y);
			m_submode = SUBMODE_USER_INDEX_MODE_3;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_MODE_3:
		{
			m_user_index_mode = command;
			LOGMASKED(LOG_COMMANDS, "User Index Mode: %02x\n", m_user_index_mode);
			m_submode = SUBMODE_NORMAL;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_STRING_1:
		{
			m_user_index_char_idx = command & 0x1f;
			LOGMASKED(LOG_COMMANDS, "User Index Charpos: %02x\n", m_user_index_char_idx);
			m_submode = SUBMODE_USER_INDEX_STRING_2;
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_STRING_2:
		{
			m_user_index_chars[m_user_index_char_idx] = (command);
			LOGMASKED(LOG_COMMANDS, "User Index char idx %x: %02x ('%c')\n", m_user_index_char_idx, command, command);

			if (command == 0x1a)
			{
				m_submode = SUBMODE_NORMAL;
			}
			else
			{
				m_user_index_char_idx++;
				if (m_user_index_char_idx > 32)
				{
					m_user_index_char_idx = 0;
				}
			}
			queue_reply(0x0a, 4.3);
			break;
		}
		case SUBMODE_USER_INDEX_WINDOW:
		{
			m_user_index_window_idx = command & 0x1f;
			LOGMASKED(LOG_COMMANDS, "User Index Window idx: %02x\n", m_user_index_window_idx);
			m_submode = SUBMODE_NORMAL;
			queue_reply(0x0a, 4.3);
			break;
		}

	default:
		if (command >= 0x30 && command <=0x39)
		{
			// Reset flags
			if (m_cmd_buffer == -2)
			{
				m_cmd_buffer = 0;
			}

			if (m_cmd_buffer != 0)
			{
				m_cmd_buffer *= 10;
			}
			m_cmd_buffer += (command - 0x30);
			queue_reply(0x0a, 4.3);
		}
		else
		{
			switch (command)
			{
			case CMD_ENTER:
				if (m_mode == MODE_SEARCH_CMD)
				{
					begin_search(m_cmd_buffer);
					m_mode = MODE_SEARCH;
				}
				else if (m_mode == MODE_REPEAT_CMD_MARK)
				{
					if (m_address_flag == ADDRESS_FRAME)
					{
						m_repeat_frame_end = m_cmd_buffer;
					}
					else if (m_address_flag == ADDRESS_CHAPTER)
					{
						m_repeat_chapter_end = m_cmd_buffer;
					}
					m_mode = MODE_REPEAT_CMD_REPS;
					m_cmd_buffer = -2;
				}
				else if (m_mode == MODE_REPEAT_CMD_REPS)
				{
					// if no number, presume 1
					if (m_cmd_buffer == -2)
					{
						m_cmd_buffer = 1;
					}
					else if (m_cmd_buffer == 0)
					{
						m_cmd_buffer = -1; // infinite
					}
					m_repeat_repetitions = m_cmd_buffer;
					m_mode = MODE_PLAY;
					m_cmd_buffer = 0;
					update_video_enable();
					update_audio_squelch();
				}
				else if (m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)
				{
					m_speed = m_base_speed / m_cmd_buffer;
					update_video_enable();
					update_audio_squelch();
				}
				queue_reply(0x0a, 1.3);
				break;

			case CMD_STEP_STILL:
				m_mode = MODE_STILL;
				set_audio_squelch(true, true);
				m_mark_frame = ~u32(0);
				advance_slider(1);
				queue_reply(0x0a, 1.4);
				break;

			case CMD_STEP_STILL_REVERSE:
				m_mode = MODE_STILL;
				set_audio_squelch(true, true);
				m_mark_frame = ~u32(0);
				advance_slider(-1);
				queue_reply(0x0a, 1.4);
				break;

			case CMD_STOP:
				m_mode = MODE_PARK;
				set_audio_squelch(true, true);
				set_video_squelch(true);
				queue_reply(0x0a, 5.5);
				break;

			case CMD_PLAY:
				m_speed_accum = 0;
				m_speed = m_base_speed;
				m_mode = MODE_PLAY;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_FAST_FORWARD:
				m_speed_accum = 0;
				m_speed = m_base_speed * 3;
				m_mode = MODE_MS_FORWARD;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_SLOW_FORWARD:
				m_speed_accum = 0;
				m_speed = m_base_speed / 5;
				m_mode = MODE_MS_FORWARD;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_STEP_FORWARD:
				// Most ROM revisions start at a slow speed, and then update to correct one
				m_speed = m_base_speed / 7;
				m_mode = MODE_MS_FORWARD;
				set_audio_squelch(true, true);
				queue_reply(0x0a, 5.5);
				break;

			case CMD_SCAN_FORWARD:
				m_speed_accum = 0;
				m_speed = m_base_speed * 100;
				m_mode = MODE_MS_FORWARD;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_PLAY_REVERSE:
				m_speed_accum = 0;
				m_speed = m_base_speed;
				m_mode = MODE_MS_REVERSE;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_FAST_REVERSE:
				m_speed_accum = 0;
				m_speed = m_base_speed * 3;
				m_mode = MODE_MS_REVERSE;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_SLOW_REVERSE:
				m_speed_accum = 0;
				m_speed = m_base_speed / 5;
				m_mode = MODE_MS_REVERSE;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_STEP_REVERSE:
				m_speed = m_base_speed / 7;
				m_mode = MODE_MS_REVERSE;
				set_audio_squelch(true, true);
				m_mark_frame = 0;
				advance_slider(-1);
				queue_reply(0x0a, 5.5);
				break;

			case CMD_SCAN_REVERSE:
				m_speed = m_base_speed / 7;
				m_mode = MODE_MS_REVERSE;
				update_audio_squelch();
				update_video_enable();
				queue_reply(0x0a, 5.5);
				break;

			case CMD_STILL:
				m_mode = MODE_STILL;
				set_audio_squelch(true, true);
				queue_reply(0x0a, 0.4);
				break;

			case CMD_SEARCH:
				m_mode = MODE_SEARCH_CMD;
				m_cmd_buffer = 0;
				m_repeat_frame_start = 0;
				m_repeat_frame_end = 0;
				m_repeat_chapter_start = 0;
				m_repeat_chapter_end = 0;
				m_repeat_repetitions = 0;
				m_search_frame = ~u32(0);
				update_audio_squelch();
				queue_reply(0x0a, 9.0);
				break;

			case CMD_REPEAT:
				m_mode = MODE_REPEAT_CMD_MARK;
				m_cmd_buffer = 0;
				if (m_address_flag == ADDRESS_FRAME)
				{
					m_repeat_frame_start = m_curr_frame;
				}
				else if (m_address_flag == ADDRESS_CHAPTER)
				{
					m_repeat_chapter_start = m_chapter;
				}
				m_repeat_frame_end = 0;
				m_repeat_chapter_end = 0;
				m_repeat_repetitions = 0;
				update_video_enable();
				update_audio_squelch();
				queue_reply(0x0a, 8.0);
				break;

			case CMD_FRAME_SET:
				m_address_flag = ADDRESS_FRAME;
				queue_reply(0x0a, 0.4);
				break;

			case CMD_CHAPTER_SET:
				m_address_flag = ADDRESS_CHAPTER;
				queue_reply(0x0a, 0.4);
				break;

			case CMD_CLEAR:
				m_cmd_buffer = 0;
				queue_reply(0x0a, 8.0);
				break;

			case CMD_CLEAR_ALL:
				m_cmd_buffer = 0;
				m_search_frame = m_curr_frame;
				m_search_chapter = 0;
				m_repeat_chapter_start = 0;
				m_repeat_frame_start = 0;
				m_repeat_chapter_end = 0;
				m_repeat_frame_end = 0;
				m_repeat_repetitions = 0;

				m_speed = m_base_speed;
				m_submode = SUBMODE_NORMAL;
				m_mode = MODE_PLAY;
				queue_reply(0x0a, 5.5);
				break;

			case CMD_CH1_ON:
				m_ch1_switch=true;
				update_audio_squelch();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_CH1_OFF:
				m_ch1_switch=false;
				update_audio_squelch();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_CH2_ON:
				m_ch2_switch=true;
				update_audio_squelch();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_CH2_OFF:
				m_ch2_switch=false;
				update_audio_squelch();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_AUDIO_ON:
				m_ch1_switch=true;
				m_ch2_switch=true;
				update_audio_squelch();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_AUDIO_OFF:
				m_ch1_switch=false;
				m_ch2_switch=false;
				update_audio_squelch();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_VIDEO_ON:
				m_video_switch = 1;
				update_video_enable();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_VIDEO_OFF:
				m_video_switch = 0;
				update_video_enable();
				queue_reply(0x0a, 0.4);
				break;

			case CMD_INDEX_ON:
				m_display_switch = 1;
				queue_reply(0x0a, 0.4);
				break;

			case CMD_INDEX_OFF:
				m_display_switch =0;
				queue_reply(0x0a, 0.4);
				break;

			case CMD_ADDR_INQ:
				{
					u8 frame_buffer[5];
					u32 frame_val = m_curr_frame;
					for (u8 i = 0; i < 5; i++)
					{
						frame_buffer[4 - i] = frame_val%10 + 0x30;
						frame_val /= 10;
					}
					queue_reply_buffer(frame_buffer, 1.3);
				}
				break;

			case CMD_MOTOR_ON:
				//Presume we're already running
				queue_reply(0x0b, 0.4);
				break;

			case CMD_STATUS_INQ:
				{
					u8 status_buffer[5] = { 0x80, 0x00, 0x10, 0x00, 0xff};

					if (m_mode == MODE_PLAY || m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)
					{
						status_buffer[4] = 0x01;
					}
					else if (m_mode == MODE_PAUSE || m_mode == MODE_STILL)
					{
						status_buffer[4] = 0x20;
					}
					queue_reply_buffer(status_buffer, 1.3);
				}
				break;

			case CMD_USER_INDEX_CTRL:
				m_submode = SUBMODE_USER_INDEX;
				queue_reply(0x0a, 0.4);
				break;

			case CMD_USER_INDEX_ON:
				{
					m_user_index_flag = true;
					queue_reply(0x0a, 0.4);
					break;
				}

			case CMD_USER_INDEX_OFF:
				{
					m_user_index_flag = false;
					queue_reply(0x0a, 0.4);
					break;
				}

			default:
				popmessage("no implementation cmd %x", command);
				queue_reply(0x0b, 0.4);
				break;
			}
		}
		LOGMASKED(LOG_SEARCHES, "Command %x\n", command);

	}
}

//-------------------------------------------------
//  begin_search - initiates a search operation
//-------------------------------------------------

void sony_ldp1450hle_device::begin_search(u32 value)
{
	if (m_address_flag == ADDRESS_FRAME)
	{
		m_search_frame = value;
	}
	else if (m_address_flag == ADDRESS_CHAPTER)
	{
		m_search_chapter = value;
	}

	set_audio_squelch(true, true);

	if (std::abs(int32_t(m_search_frame) - int32_t(m_curr_frame)) > 100)
	{
		video_enable(false);
		set_audio_squelch(true, true);
	}
}


//-------------------------------------------------
//  update_audio_squelch - set audio squelch state
//  on the base device based on our audio switches
//-------------------------------------------------

void sony_ldp1450hle_device::update_audio_squelch()
{
	const bool squelch_both = (m_mode == MODE_STILL || m_mode == MODE_PAUSE || m_mode == MODE_SEARCH || m_mode == MODE_MS_REVERSE || m_mode == MODE_SEARCH_CMD || m_mode == MODE_SEARCH_REP );
	const bool squelch_left = !(m_ch1_switch) || squelch_both;
	const bool squelch_right = !(m_ch2_switch) || squelch_both;
	set_audio_squelch(squelch_left, squelch_right);
}

//-------------------------------------------------
//  update_video_enable - set video enable state
//  on the base device based on our video switch
//-------------------------------------------------

void sony_ldp1450hle_device::update_video_enable()
{
	const bool enable = m_video_switch == 1 && (m_mode == MODE_STILL || m_mode == MODE_PLAY || m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE);
	set_video_squelch(!enable);
	video_enable(enable);
}


bitmap_yuy16 sony_ldp1450hle_device::osd_char_gen(uint8_t idx)
{
	uint16_t white = 0xeb80;
	uint16_t black = 0x0080;

	// iterate over pixels
	const u16 *chdataptr = &text_bitmap[idx][0];

	bitmap_yuy16 char_bmp = bitmap_yuy16(16,16);
	for (u8 y = 0; y < 16; y++)
	{
		u16 chdata = *chdataptr++;

		for (u8 x = 0; x < 16; x++, chdata >>= 1)
		{
			if (chdata & 0x01)
			{
				char_bmp.pix(y, x) = white;
			}
			else
			{
				char_bmp.pix(y, x) = black;
			}
		}
	}
	return char_bmp;
}


//-------------------------------------------------
//  device_start - device initialization
//-------------------------------------------------

void sony_ldp1450hle_device::device_start()
{
	// pass through to the parent
	laserdisc_device::device_start();

	// allocate timers
	m_vbi_fetch = timer_alloc(FUNC(sony_ldp1450hle_device::process_vbi_data), this);
	m_queue_timer = timer_alloc(FUNC(sony_ldp1450hle_device::process_queue), this);

	// register state saving
	save_item(NAME(m_baud));
	save_item(NAME(m_cmd_buffer));
	save_item(NAME(m_cmd_running));
	save_item(NAME(m_reply_buffer));
	save_item(NAME(m_reply_write_index));
	save_item(NAME(m_reply_read_index));
	save_item(NAME(m_reply));
	save_item(NAME(m_mode));
	save_item(NAME(m_chapter));
	save_item(NAME(m_time));
	save_item(NAME(m_search_chapter));
	save_item(NAME(m_search_frame));
	save_item(NAME(m_mark_chapter));
	save_item(NAME(m_mark_frame));
	save_item(NAME(m_video_switch));
	save_item(NAME(m_ch1_switch));
	save_item(NAME(m_ch2_switch));
	save_item(NAME(m_display_switch));
	save_item(NAME(m_address_flag));
	save_item(NAME(m_base_speed));
	save_item(NAME(m_speed));
	save_item(NAME(m_speed_accum));
	save_item(NAME(m_curr_frame));
	save_item(NAME(m_frame));
	save_item(NAME(m_repeat_chapter_start));
	save_item(NAME(m_repeat_chapter_end));
	save_item(NAME(m_repeat_frame_start));
	save_item(NAME(m_repeat_frame_end));

	save_item(NAME(m_user_index_flag));
	save_item(NAME(m_user_index_x));
	save_item(NAME(m_user_index_y));
	save_item(NAME(m_user_index_mode));
	save_item(NAME(m_user_index_char_idx));
	save_item(NAME(m_user_index_window_idx));
	save_item(NAME(m_user_index_chars));


	for (u8 chr_idx = 0; chr_idx < 96; chr_idx ++)
	{
		m_osd_font[chr_idx] = osd_char_gen(chr_idx);
	}

}


//-------------------------------------------------
//  device_reset - device reset
//-------------------------------------------------

void sony_ldp1450hle_device::device_reset()
{
	// pass through to the parent
	laserdisc_device::device_reset();

	// initialize diserial
	set_tra_rate(attotime::from_hz(m_baud));
	set_rcv_rate(attotime::from_hz(m_baud));
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);

	// reset our state
	m_vbi_fetch->adjust(attotime::never);
	m_queue_timer->adjust(attotime::never);

	m_cmd_running = false;
	std::fill_n(m_reply_buffer, std::size(m_reply_buffer), 0);
	m_reply_write_index = 0;
	m_reply_read_index = 0;

	m_mode = MODE_PARK;
	m_chapter = 0;
	m_time = 0;
	m_cmd_buffer = ~u32(0);
	m_search_chapter = ~u32(0);
	m_search_frame = ~u32(0);
	m_mark_chapter = ~u32(0);
	m_mark_frame = ~u32(0);
	m_repeat_chapter_start = ~u32(0);
	m_repeat_chapter_end = ~u32(0);
	m_repeat_frame_start = ~u32(0);
	m_repeat_frame_end = ~u32(0);
	m_video_switch = 1;
	m_ch1_switch = false;
	m_ch2_switch = false;
	m_display_switch = 0;
	m_address_flag = ADDRESS_FRAME;
	m_base_speed = 60;
	m_speed = m_base_speed;
	m_speed_accum = 0;
	m_user_index_flag = false;

	video_enable(true);
	set_audio_squelch(true, true);
}


//-------------------------------------------------
//  process_vbi_data - process VBI data and
//  act on search/play seeking
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sony_ldp1450hle_device::process_vbi_data)
{
	u32 line = get_field_code(LASERDISC_CODE_LINE1718, false);
	if ((line & 0xf80000) == 0xf80000 || line == VBI_CODE_LEADIN || line == VBI_CODE_LEADOUT)
	{
		u32 old_frame = m_curr_frame;
		if (line == VBI_CODE_LEADIN)
			m_curr_frame = 0;
		else if (line == VBI_CODE_LEADOUT)
			m_curr_frame = 54000;
		else
			m_curr_frame = bcd_to_literal(line & 0x7ffff);

		LOGMASKED(LOG_FRAMES, "Current frame is %d (VBI 16: %06x, VBI 17: %06x, VBI 18: %06x, VBI 1718: %06x\n", m_curr_frame,
				get_field_code(LASERDISC_CODE_LINE16, false),
				get_field_code(LASERDISC_CODE_LINE17, false),
				get_field_code(LASERDISC_CODE_LINE18, false),
				line);

		if (m_mode != MODE_STILL && m_mode != MODE_PAUSE)
		{

			if (m_mark_frame != ~u32(0) && m_search_frame == ~u32(0))
			{
				int32_t old_delta = int32_t(m_mark_frame) - int32_t(old_frame);
				int32_t curr_delta = int32_t(m_mark_frame) - int32_t(m_curr_frame);
				LOGMASKED(LOG_STOPS, "Stop Mark is currently %d, old frame is %d, current frame is %d, old delta %d, curr delta %d\n", m_mark_frame, old_frame, m_curr_frame, old_delta, curr_delta);
				if (curr_delta == 0 || (old_delta < 0) != (curr_delta < 0))
				{
					m_mark_frame = ~u32(0);
					if (is_cav_disc())
					{
						m_mode = MODE_STILL;
						update_video_enable();
					}
					else
					{
						m_mode = MODE_PAUSE;
						video_enable(false);
					}

					set_audio_squelch(true, true);
				}
			}

			if (m_search_frame != ~u32(0))
			{
				// TODO: Chapter-search support
				int32_t delta = int32_t(m_search_frame) - int32_t(m_curr_frame);
				if (delta == 0)
				{
					m_search_frame = ~u32(0);
					if (is_cav_disc())
					{
						if (m_mode == MODE_SEARCH_REP)
						{
							m_mode = MODE_PLAY;
						}
						else
						{
							m_mode = MODE_STILL;
						}
						update_video_enable();
					}
					else
					{
						m_mode = MODE_PLAY;
						update_video_enable();
					}

					queue_reply(0x01, 1.3); //completion
				}
				else if (delta <= 2 && delta > 0)
				{
					// We're approaching our frame, let it run up.
				}
				else
				{
					if (delta < 0)
					{
						advance_slider(std::min(-2, delta / 2));
					}
					else
					{
						advance_slider(std::max(1, delta / 2));
					}
				}
			}


			const bool repeat_hit = ((m_repeat_frame_end != ~u32(0) && m_curr_frame >= m_repeat_frame_end) ||  (m_mode == MODE_MS_REVERSE && (m_repeat_frame_start != ~u32(0) && m_curr_frame <= m_repeat_frame_start) ));
			if (m_repeat_repetitions != 0 && repeat_hit)
			{

				if (m_repeat_repetitions > 0)
				{
					m_repeat_repetitions -= 1;
				}

				const bool reverse = m_mode == MODE_MS_REVERSE;

				m_mode = MODE_STILL;
				update_video_enable();
				update_audio_squelch();
				if (m_repeat_repetitions != 0)
				{
					m_mode = MODE_SEARCH_REP;
					begin_search(reverse ? m_repeat_frame_end : m_repeat_frame_start);
				}
				else
				{
					m_repeat_frame_end = ~u32(0);
					m_repeat_frame_start = ~u32(0);
					queue_reply(0x01, 1.3);
				}
			}

		}
	}
}


//-------------------------------------------------
//  player_vsync - VSYNC callback, called at the
//  start of the blanking period
//-------------------------------------------------

void sony_ldp1450hle_device::player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	// set a timer to fetch the VBI data when it is ready
	if (m_mode > MODE_DOOR_OPEN)
	{
		m_vbi_fetch->adjust(screen().time_until_pos(19*2));
	}
}


//-------------------------------------------------
//  player_update - update callback, called on
//  the first visible line of the frame
//-------------------------------------------------

int32_t sony_ldp1450hle_device::player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime)
{
	if (!fieldnum)
		return 0;

	if (m_mode == MODE_MS_FORWARD || m_mode == MODE_MS_REVERSE)
	{
		m_speed_accum += m_speed;
		int elapsed_tracks = m_speed_accum / m_base_speed;
		m_speed_accum -= elapsed_tracks * m_base_speed;
		if (m_mode == MODE_MS_REVERSE)
			elapsed_tracks *= -1;

		if (m_mark_frame != ~u32(0))
		{
			const int32_t jump_frame = int32_t(m_curr_frame) + elapsed_tracks;
			const int32_t curr_delta = int32_t(m_mark_frame) - int32_t(m_curr_frame);
			const int32_t next_delta = int32_t(m_mark_frame) - int32_t(jump_frame);
			if ((curr_delta < 0) != (next_delta < 0))
			{
				elapsed_tracks = curr_delta;
			}
		}
		return elapsed_tracks;
	}


	if (m_mode == MODE_PLAY || m_mode == MODE_SEARCH || m_mode == MODE_SEARCH_REP)
	{
		return 1;
	}

	return 0;
}


//-------------------------------------------------
//  rcv_complete - called by diserial when we
//  have received a complete byte
//-------------------------------------------------

void sony_ldp1450hle_device::rcv_complete()
{
	receive_register_extract();
	add_command_byte(get_received_char());
}


//-------------------------------------------------
//  tra_complete - called by diserial when we
//  have transmitted a complete byte
//-------------------------------------------------

void sony_ldp1450hle_device::tra_complete()
{
	m_reply_read_index = (m_reply_read_index + 1) % u8(std::size(m_reply_buffer));
	if (m_reply_read_index != m_reply_write_index)
	{
		const u8 data = u8(m_reply_buffer[m_reply_read_index]);
		LOGMASKED(LOG_REPLY_BYTES, "Sending reply byte: %02x\n", data);
		transmit_register_setup(data);
	}
}


//-------------------------------------------------
//  tra_callback - called by diserial when we
//  transmit a single bit
//-------------------------------------------------

void sony_ldp1450hle_device::tra_callback()
{
	m_serial_tx(transmit_register_get_data_bit());
}
