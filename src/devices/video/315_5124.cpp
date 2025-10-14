// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Enik Land
/*********************************************************************

    sega315_5124.cpp

    Implementation of video hardware chips used by Sega System E,
    Master System, and Game Gear.

    Some specific behavior of the chip used by Sega Genesis/Mega
    Drive is also implemented for mode 4 only.

**********************************************************************/

/*

To do:

  - Mid-scanline changes
  - VRAM/CRAM access constraints


SMS Display Timing
------------------
    For more information, please see:
    - http://cgfm2.emuviews.com/txt/msvdp.txt
    - http://www.smspower.org/forums/viewtopic.php?p=37142
    - http://www.smspower.org/forums/viewtopic.php?p=92925

A scanline contains the following sections:
  - horizontal sync    26  E9-F5   => HSYNC low
  - left blanking       2  F6-F6   => HSYNC high
  - color burst        14  F7-FD
  - left blanking       8  FE-01
  - left border        13  02-08
  - active display    256  08-88
  - right border       15  88-8F
  - right blanking      8  90-93

  Probably the processing done for the active display occurs when HCount
  is in the 00-7F range and there is a delay until its signal is shown on
  screen, as happens on the TMS9918 chip according to this timing diagram:
      http://www.smspower.org/Development/TMS9918MasterTimingDiagram


NTSC frame timing
                       256x192         256x224        256x240 (doesn't work on real hardware)
  - vertical sync       3  D5-D7        3  E5-E7       3  ED-EF
  - top blanking       13  D8-E4       13  E8-F4      13  F0-FC
  - top border         27  E5-FF       11  F5-FF       3  FD-FF
  - active display    192  00-BF      224  00-DF     240  00-EF
  - bottom border      24  C0-D7        8  E0-E7       0  F0-F0
  - bottom blanking     3  D8-DA        3  E8-EA       3  F0-F2


PAL frame timing
                       256x192         256x224        256x240
  - vertical sync       3  BA-BC        3  CA-CC       3  D2-D4
  - top blanking       13  BD-C9       13  CD-D9      13  D5-E1
  - top border         54  CA-FF       38  DA-FF      30  E2-FF
  - active display    192  00-BF      224  00-DF     240  00-EF
  - bottom border      48  C0-EF       32  E0-FF      24  F0-07
  - bottom blanking     3  F0-F2        3  00-02       3  08-0A

*/

#include "emu.h"
#include "315_5124.h"


/****************************************************************************
 * Configurable logging
 ****************************************************************************/

#define LOG_VIDMODE    (1U << 1)
#define LOG_REGWRITE   (1U << 2)
#define LOG_VCOUNTREAD (1U << 3)

//#define VERBOSE (LOG_GENERAL | LOG_VIDMODE | LOG_REGWRITE | LOG_VCOUNTREAD)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGVIDMODE(...)    LOGMASKED(LOG_VIDMODE, __VA_ARGS__)
#define LOGREGWRITE(...)   LOGMASKED(LOG_REGWRITE, __VA_ARGS__)
#define LOGVCOUNTREAD(...) LOGMASKED(LOG_VCOUNTREAD, __VA_ARGS__)

/****************************************************************************/

#define SEGA315_5124_PALETTE_SIZE     (64 + 16)
#define SEGA315_5377_PALETTE_SIZE     4096

#define VRAM_SIZE             0x4000

#define STATUS_VINT           0x80  /* Pending vertical interrupt flag */
#define STATUS_SPROVR         0x40  /* Sprite overflow flag */
#define STATUS_SPRCOL         0x20  /* Object collision flag */

#define PRIORITY_BIT          0x1000
#define BACKDROP_COLOR        ((m_vdp_mode == 4 ? 0x10 : 0x00) + (m_reg[0x07] & 0x0f))

#define VERTICAL_SYNC         0
#define TOP_BLANKING          1
#define TOP_BORDER            2
#define ACTIVE_DISPLAY_V      3
#define BOTTOM_BORDER         4
#define BOTTOM_BLANKING       5

static constexpr u8 ntsc_192[6] = { 3, 13, 27, 192, 24, 3 };
static constexpr u8 ntsc_224[6] = { 3, 13, 11, 224,  8, 3 };
static constexpr u8 ntsc_240[6] = { 3, 13,  3, 240,  0, 3 };
static constexpr u8 pal_192[6]  = { 3, 13, 54, 192, 48, 3 };
static constexpr u8 pal_224[6]  = { 3, 13, 38, 224, 32, 3 };
static constexpr u8 pal_240[6]  = { 3, 13, 30, 240, 24, 3 };

#define VINT_HPOS             0
#define VINT_FLAG_HPOS        1
#define HINT_HPOS             2
#define NMI_HPOS              3
#define XSCROLL_HPOS          4
#define VCOUNT_CHANGE_HPOS    5
#define SPROVR_HPOS           6
#define SPRCOL_BASEHPOS       7

static constexpr u8 line_315_5124[8] = { 24, 24, 26, 28 /* not verified */, 21, 23, 24, 59 };
static constexpr u8 line_315_5377[8] = { 26, 26, 27, 28 /* not verified */, 24, 28, 26, 62 };

#define DISPLAY_DISABLED_HPOS 24 /* not verified, works if above 18 (for 'pstrike2') and below 25 (for 'fantdizzy') */
#define DISPLAY_CB_HPOS       2  /* fixes 'roadrash' (SMS game) title scrolling, due to line counter reload timing */

#define DRAW_TIME_GG         111      /* 26 + 2 + 14 + 8 + 13 + 96/2 */
#define DRAW_TIME_SMS         63      /* 26 + 2 + 14 + 8 + 13 */


DEFINE_DEVICE_TYPE(SEGA315_5124, sega315_5124_device, "sega315_5124", "Sega 315-5124 SMS1 VDP")
DEFINE_DEVICE_TYPE(SEGA315_5246, sega315_5246_device, "sega315_5246", "Sega 315-5246 SMS2 VDP")
DEFINE_DEVICE_TYPE(SEGA315_5377, sega315_5377_device, "sega315_5377", "Sega 315-5377 Gamegear VDP")

// (reference for VDP colors: http://www.sega-16.com/forum/showthread.php?30530-SMS-VDP-output-levels)
void sega315_5124_device::sega315_5124_palette(palette_device &palette) const
{
	// blue channel is non-linear, verified from die shot
	// reference: https://www.retrorgb.com/sega-master-system-blue-channel-mysteries-further-uncovered.html
	static constexpr u8 level[4] = {0,78,160,238};
	static constexpr u8 blue_level[4] = {0,98,160,238};
	for (int i = 0; i < 64; i++)
	{
		const u8 r = i & 0x03;
		const u8 g = (i & 0x0c) >> 2;
		const u8 b = (i & 0x30) >> 4;
		palette.set_pen_color(i, level[r], level[g], blue_level[b]);
	}
	// sms and sg1000-mark3 uses a different palette for modes 0 to 3 - see http://www.smspower.org/Development/Palette
	// TMS9918 palette
	palette.set_pen_color(64+ 0, level[0], level[0], blue_level[0]); // palette.set_pen_color(64+ 0,   0,   0,   0);
	palette.set_pen_color(64+ 1, level[0], level[0], blue_level[0]); // palette.set_pen_color(64+ 1,   0,   0,   0);
	palette.set_pen_color(64+ 2, level[0], level[2], blue_level[0]); // palette.set_pen_color(64+ 2,  33, 200,  66);
	palette.set_pen_color(64+ 3, level[0], level[3], blue_level[0]); // palette.set_pen_color(64+ 3,  94, 220, 120);
	palette.set_pen_color(64+ 4, level[0], level[0], blue_level[1]); // palette.set_pen_color(64+ 4,  84,  85, 237);
	palette.set_pen_color(64+ 5, level[0], level[0], blue_level[3]); // palette.set_pen_color(64+ 5, 125, 118, 252);
	palette.set_pen_color(64+ 6, level[1], level[0], blue_level[0]); // palette.set_pen_color(64+ 6, 212,  82,  77);
	palette.set_pen_color(64+ 7, level[0], level[3], blue_level[3]); // palette.set_pen_color(64+ 7,  66, 235, 245);
	palette.set_pen_color(64+ 8, level[2], level[0], blue_level[0]); // palette.set_pen_color(64+ 8, 252,  85,  84);
	palette.set_pen_color(64+ 9, level[3], level[0], blue_level[0]); // palette.set_pen_color(64+ 9, 255, 121, 120);
	palette.set_pen_color(64+10, level[1], level[1], blue_level[0]); // palette.set_pen_color(64+10, 212, 193,  84);
	palette.set_pen_color(64+11, level[3], level[3], blue_level[0]); // palette.set_pen_color(64+11, 230, 206, 128);
	palette.set_pen_color(64+12, level[0], level[1], blue_level[0]); // palette.set_pen_color(64+12,  33, 176,  59);
	palette.set_pen_color(64+13, level[3], level[0], blue_level[3]); // palette.set_pen_color(64+13, 201,  91, 186);
	palette.set_pen_color(64+14, level[1], level[1], blue_level[1]); // palette.set_pen_color(64+14, 204, 204, 204);
	palette.set_pen_color(64+15, level[3], level[3], blue_level[3]); // palette.set_pen_color(64+15, 255, 255, 255);
}


void sega315_5246_device::sega315_5246_palette(palette_device &palette) const
{
	// bit different output level compare to 315_5124
	static constexpr u8 level[4] = {0,89,174,255};
	for (int i = 0; i < 64; i++)
	{
		const u8 r = i & 0x03;
		const u8 g = (i & 0x0c) >> 2;
		const u8 b = (i & 0x30) >> 4;
		palette.set_pen_color(i, level[r], level[g], level[b]);
	}
	// TMS9918 palette (see sega315_5124_palette)
	palette.set_pen_color(64+ 0, level[0], level[0], level[0]); // palette.set_pen_color(64+ 0,   0,   0,   0);
	palette.set_pen_color(64+ 1, level[0], level[0], level[0]); // palette.set_pen_color(64+ 1,   0,   0,   0);
	palette.set_pen_color(64+ 2, level[0], level[2], level[0]); // palette.set_pen_color(64+ 2,  33, 200,  66);
	palette.set_pen_color(64+ 3, level[0], level[3], level[0]); // palette.set_pen_color(64+ 3,  94, 220, 120);
	palette.set_pen_color(64+ 4, level[0], level[0], level[1]); // palette.set_pen_color(64+ 4,  84,  85, 237);
	palette.set_pen_color(64+ 5, level[0], level[0], level[3]); // palette.set_pen_color(64+ 5, 125, 118, 252);
	palette.set_pen_color(64+ 6, level[1], level[0], level[0]); // palette.set_pen_color(64+ 6, 212,  82,  77);
	palette.set_pen_color(64+ 7, level[0], level[3], level[3]); // palette.set_pen_color(64+ 7,  66, 235, 245);
	palette.set_pen_color(64+ 8, level[2], level[0], level[0]); // palette.set_pen_color(64+ 8, 252,  85,  84);
	palette.set_pen_color(64+ 9, level[3], level[0], level[0]); // palette.set_pen_color(64+ 9, 255, 121, 120);
	palette.set_pen_color(64+10, level[1], level[1], level[0]); // palette.set_pen_color(64+10, 212, 193,  84);
	palette.set_pen_color(64+11, level[3], level[3], level[0]); // palette.set_pen_color(64+11, 230, 206, 128);
	palette.set_pen_color(64+12, level[0], level[1], level[0]); // palette.set_pen_color(64+12,  33, 176,  59);
	palette.set_pen_color(64+13, level[3], level[0], level[3]); // palette.set_pen_color(64+13, 201,  91, 186);
	palette.set_pen_color(64+14, level[1], level[1], level[1]); // palette.set_pen_color(64+14, 204, 204, 204);
	palette.set_pen_color(64+15, level[3], level[3], level[3]); // palette.set_pen_color(64+15, 255, 255, 255);
}


void sega315_5377_device::sega315_5377_palette(palette_device &palette) const
{
	// TODO: linear? measure this
	for (int i = 0; i < 4096; i++)
	{
		const u8 r = i & 0x000f;
		const u8 g = (i & 0x00f0) >> 4;
		const u8 b = (i & 0x0f00) >> 8;
		palette.set_pen_color(i, pal4bit(r), pal4bit(g), pal4bit(b));
	}
}


void sega315_5313_mode4_device::sega315_5313_palette(palette_device &palette) const
{
	// non-linear (reference: http://gendev.spritesmind.net/forum/viewtopic.php?f=22&t=2188)
	static constexpr u8 level[15] = {0,29,52,70,87,101,116,130,144,158,172,187,206,228,255};
	for (int i = 0; i < 512; i++)
	{
		const u8 r = (i & 0x0007) >> 0;
		const u8 g = (i & 0x0038) >> 3;
		const u8 b = (i & 0x01c0) >> 6;
		palette.set_pen_color(i + (512 * 0), level[r << 1], level[g << 1], level[b << 1]); // normal
		palette.set_pen_color(i + (512 * 1), level[r], level[g], level[b]); // shadow
		palette.set_pen_color(i + (512 * 2), level[7 + r], level[7 + g], level[7 + b]); // hilight
	}
	// separated SMS compatible mode color
	static constexpr u8 sms_level[4] = {0,99,162,255};
	for (int i = 0; i < 64; i++)
	{
		const u8 r = (i & 0x0003) >> 0;
		const u8 g = (i & 0x000c) >> 2;
		const u8 b = (i & 0x0030) >> 4;
		palette.set_pen_color(i + (512 * 3), sms_level[r], sms_level[g], sms_level[b]); // normal
	}
}


// default address map
void sega315_5124_device::sega315_5124(address_map &map)
{
	if (!has_configured_map(0))
		map(0x0000, VRAM_SIZE - 1).ram();
}


sega315_5124_device::sega315_5124_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sega315_5124_device(mconfig, SEGA315_5124, tag, owner, clock, SEGA315_5124_CRAM_SIZE, 0x00, 0x0f, 4, 8, line_315_5124)
{
}


sega315_5124_device::sega315_5124_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cram_size, u8 palette_offset, u8 reg_num_mask, int max_sprite_zoom_hcount, int max_sprite_zoom_vcount, const u8 *line_timing)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, m_hcounter_divide(1)
	, m_cram_size(cram_size)
	, m_line_timing(line_timing)
	, m_palette_offset(palette_offset)
	, m_reg_num_mask(reg_num_mask)
	, m_max_sprite_zoom_hcount(max_sprite_zoom_hcount)
	, m_max_sprite_zoom_vcount(max_sprite_zoom_vcount)
	, m_is_pal(false)
	, m_vblank_cb(*this)
	, m_n_csync_cb(*this)
	, m_n_int_cb(*this)
	, m_n_nmi_cb(*this)
	, m_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, address_map_constructor(FUNC(sega315_5124_device::sega315_5124), this))
	, m_palette_lut(*this, "palette_lut")
	, m_snsnd(*this, "snsnd")
{
}


sega315_5246_device::sega315_5246_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sega315_5124_device(mconfig, SEGA315_5246, tag, owner, clock, SEGA315_5124_CRAM_SIZE, 0x00, 0x0f, 8, 8, line_315_5124)
{
}


sega315_5246_device::sega315_5246_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cram_size, u8 palette_offset, u8 reg_num_mask, int max_sprite_zoom_hcount, int max_sprite_zoom_vcount, const u8 *line_timing)
	: sega315_5124_device(mconfig, type, tag, owner, clock, cram_size, palette_offset, reg_num_mask, max_sprite_zoom_hcount, max_sprite_zoom_vcount, line_timing)
{
}


// Embedded mode 4 support of the 315-5313 VDP (see 315_5313.cpp), used by Sega Genesis/Mega Drive
sega315_5313_mode4_device::sega315_5313_mode4_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cram_size, u8 palette_offset, u8 reg_num_mask, int max_sprite_zoom_hcount, int max_sprite_zoom_vcount, const u8 *line_timing)
	: sega315_5246_device(mconfig, type, tag, owner, clock, cram_size, palette_offset, reg_num_mask, max_sprite_zoom_hcount, max_sprite_zoom_vcount, line_timing)
{
}


sega315_5377_device::sega315_5377_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: sega315_5246_device(mconfig, SEGA315_5377, tag, owner, clock, SEGA315_5377_CRAM_SIZE, 0x10, 0x0f, 8, 8, line_315_5377)
{
}


device_memory_interface::space_config_vector sega315_5124_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


void sega315_5124_device::select_extended_res_mode4(bool M1, bool M2, bool M3)
{
	// no extended resolution supported.
	return;
}


void sega315_5246_device::select_extended_res_mode4(bool M1, bool M2, bool M3)
{
	if (M2)
	{
		if (M1 && !M3)
			m_y_pixels = 224;   // 224-line display
		else if (!M1 && M3)
			m_y_pixels = 240;   // 240-line display
	}
}

void sega315_5313_mode4_device::select_extended_res_mode4(bool M1, bool M2, bool M3)
{
	// no extended resolution supported.
}


void sega315_5124_device::select_display_mode()
{
	const bool M1 = BIT(m_reg[0x01], 4);
	const bool M2 = BIT(m_reg[0x00], 1);
	const bool M3 = BIT(m_reg[0x01], 3);
	const bool M4 = BIT(m_reg[0x00], 2);

	if (M4)
	{
		// mode 4
		m_vdp_mode = 4;
		select_extended_res_mode4(M1, M2, M3);
	}
	else // original TMS9918 mode
	{
		if (!M1 && !M2 && !M3) // Mode 0 (Graphics I Mode)
			m_vdp_mode = 0;
		else if (M1 && !M2 && !M3) // Mode 1 (Text Mode)
			m_vdp_mode = 1;
		else if (!M1 && M2 && !M3) // Mode 2 (Graphics II Mode)
			m_vdp_mode = 2;
		else if (!M1 && !M2 && M3) // Mode 3 (Multicolor Mode)
			m_vdp_mode = 3;
		else
			LOGVIDMODE("Unknown video mode detected (M1 = %c, M2 = %c, M3 = %c, M4 = %c)\n", M1 ? '1' : '0', M2 ? '1' : '0', M3 ? '1' : '0', M4 ? '1' : '0');
	}

}


void sega315_5313_mode4_device::select_display_mode()
{
	const bool M5 = BIT(m_reg[0x01], 2);

	if (M5)
	{
		/* mode 5, not implemented */
		m_vdp_mode = 5;
		LOGVIDMODE("Switched to unimplemented video mode 5 !\n");
	}
	else
	{
		/* mode 4, SMS compatibility */
		m_vdp_mode = 4;
	}
}


void sega315_5124_device::set_display_settings()
{
	m_y_pixels = 192;

	select_display_mode();
	set_frame_timing();
	m_cram_dirty = true;
}


void sega315_5124_device::set_frame_timing()
{
	switch (m_y_pixels)
	{
	case 192:
		m_frame_timing = (m_is_pal) ? pal_192 : ntsc_192;
		break;

	case 224:
		m_frame_timing = (m_is_pal) ? pal_224 : ntsc_224;
		break;

	case 240:
		m_frame_timing = (m_is_pal) ? pal_240 : ntsc_240;
		break;
	}
}


u8 sega315_5124_device::vcount_read()
{
	const u8 vc = vcount();

	LOGVCOUNTREAD(
			"fr %d vpos %3d hpos %3d hc 0x%02X %s: vcount read value 0x%02X\n",
			screen().frame_number(),
			screen().vpos(),
			screen_hpos(),
			hcount(),
			machine().describe_context(),
			vc);

	return vc;
}


u8 sega315_5124_device::vcount()
{
	const int active_scr_start = m_frame_timing[VERTICAL_SYNC] + m_frame_timing[TOP_BLANKING] + m_frame_timing[TOP_BORDER];
	int vpos = screen().vpos();

	if (screen_hpos() < m_line_timing[VCOUNT_CHANGE_HPOS])
	{
		vpos--;
		if (vpos < 0)
			vpos += screen().height();
	}

	return (vpos - active_scr_start) & 0xff;
}


u8 sega315_5124_device::hcount_read()
{
	m_hcounter_latched = false;
	return m_hcounter;
}


void sega315_5124_device::hcount_latch()
{
	m_hcounter = hcount();
	m_hcounter_latched = true;
}


u8 sega315_5124_device::hcount()
{
	/* The hcount value returned by the VDP seems to be based on the previous hpos */
	int hclock = screen_hpos() - 1;
	if (hclock < 0)
		hclock += WIDTH;

	return ((hclock - 46) >> 1) & 0xff;
}


void sega315_5377_device::set_sega315_5124_compatibility_mode(bool sega315_5124_compatibility_mode)
{
	m_sega315_5124_compatibility_mode = sega315_5124_compatibility_mode;
	m_cram_mask = (!m_sega315_5124_compatibility_mode) ? (SEGA315_5377_CRAM_SIZE - 1) : (SEGA315_5124_CRAM_SIZE - 1);
	m_draw_time = m_sega315_5124_compatibility_mode ? DRAW_TIME_SMS : DRAW_TIME_GG;
}


TIMER_CALLBACK_MEMBER(sega315_5124_device::eol_flag_check)
{
	/* Activate flags that were pending until the end of the line. */
	check_pending_flags();
}

TIMER_CALLBACK_MEMBER(sega315_5124_device::draw_lborder)
{
	rectangle rec;
	rec.min_y = rec.max_y = param;

	update_palette();

	/* Draw left border */
	rec.min_x = LBORDER_START;
	rec.max_x = LBORDER_START + LBORDER_WIDTH - 1;
	m_tmpbitmap.fill(m_palette_lut->pen(m_current_palette[BACKDROP_COLOR]), rec);
	m_y1_bitmap.fill((m_reg[0x07] & 0x0f) ? 1 : 0, rec);
}

TIMER_CALLBACK_MEMBER(sega315_5124_device::draw_rborder)
{
	rectangle rec;
	rec.min_y = rec.max_y = param;

	update_palette();

	/* Draw right border */
	rec.min_x = LBORDER_START + LBORDER_WIDTH + 256;
	rec.max_x = rec.min_x + RBORDER_WIDTH - 1;
	m_tmpbitmap.fill(m_palette_lut->pen(m_current_palette[BACKDROP_COLOR]), rec);
	m_y1_bitmap.fill((m_reg[0x07] & 0x0f) ? 1 : 0, rec);
}

TIMER_CALLBACK_MEMBER(sega315_5124_device::trigger_hint)
{
	if (m_pending_hint || m_hint_occurred)
	{
		if (BIT(m_reg[0x00], 4))
		{
			m_n_int_state = 0;
			m_n_int_cb(ASSERT_LINE);
		}
	}
}

TIMER_CALLBACK_MEMBER(sega315_5124_device::trigger_vint)
{
	if ((m_pending_status & STATUS_VINT) || (m_status & STATUS_VINT))
	{
		if (BIT(m_reg[0x01], 5))
		{
			m_n_int_state = 0;
			m_n_int_cb(ASSERT_LINE);
		}
	}
}

TIMER_CALLBACK_MEMBER(sega315_5124_device::update_nmi)
{
	if (!m_n_nmi_in_state)
	{
		if (m_n_nmi_state)
			m_n_nmi_cb(ASSERT_LINE);
	}
	else
	{
		if (!m_n_nmi_state)
			m_n_nmi_cb(CLEAR_LINE);
	}
	m_n_nmi_state = m_n_nmi_in_state;
}


void sega315_5124_device::vblank_end(int vpos)
{
	m_nmi_timer->adjust(screen().time_until_pos(vpos, m_line_timing[NMI_HPOS]));
}


void sega315_5377_device::vblank_end(int vpos)
{
	// Assume the VBlank line is used to trigger the NMI logic performed by the 315-5378 chip.
	m_vblank_cb(0);
}


TIMER_CALLBACK_MEMBER(sega315_5124_device::process_line_timer)
{
	const int vpos = screen().vpos();
	int vpos_limit = m_frame_timing[VERTICAL_SYNC] + m_frame_timing[TOP_BLANKING]
					+ m_frame_timing[TOP_BORDER] + m_frame_timing[ACTIVE_DISPLAY_V]
					+ m_frame_timing[BOTTOM_BORDER] + m_frame_timing[BOTTOM_BLANKING];

	/* copy current values in case they are not changed until latch time */
	m_display_disabled = !BIT(m_reg[0x01], 6);
	m_reg8copy = m_reg[0x08];

	/* Check if the /CSYNC signal must be active (low) */
	/* /CSYNC is signals /HSYNC and /VSYNC (both internals) ANDed together.
	   According to Charles MacDonald, /HSYNC goes low for 28 pixels on beginning
	   (before active screen) of all lines except on vertical sync area, where
	   /VSYNC goes low for 3 full lines, and except the two lines that follows,
	   because /VSYNC goes high for another line and remains high until the
	   active screen of the next line, what avoids a /HSYNC pulse there.
	*/
	if (vpos == 0 || vpos > (m_frame_timing[VERTICAL_SYNC] + 1))
	{
		m_n_csync_cb(0);
	}

	vpos_limit -= m_frame_timing[BOTTOM_BLANKING];

	/* Check if we're below the bottom border */
	if (vpos >= vpos_limit)
	{
		m_line_counter = m_reg[0x0a];
		return;
	}

	vpos_limit -= m_frame_timing[BOTTOM_BORDER];

	/* Check if we're in the bottom border area */
	if (vpos >= vpos_limit)
	{
		if (vpos == vpos_limit)
		{
			if (m_line_counter == 0x00)
			{
				m_line_counter = m_reg[0x0a];
				m_hint_timer->adjust(screen().time_until_pos(vpos, m_line_timing[HINT_HPOS]));
				m_pending_hint = true;
			}
			else
			{
				m_line_counter--;
			}
		}
		else
		{
			m_line_counter = m_reg[0x0a];
		}

		// vpos_limit + 1 because VINT fires at the end of the first logical line of the bottom border.
		if (vpos == vpos_limit + 1)
		{
			m_vint_timer->adjust(screen().time_until_pos(vpos, m_line_timing[VINT_HPOS]));
			m_pending_status |= STATUS_VINT;
			m_vblank_cb(1);
		}

		/* Draw borders */
		m_lborder_timer->adjust(screen().time_until_pos(vpos, LBORDER_START), vpos);
		m_rborder_timer->adjust(screen().time_until_pos(vpos, LBORDER_START + LBORDER_WIDTH + 256), vpos);

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function */
		/* so sprite collisions can occur on the border. */
		select_sprites(vpos - (vpos_limit - m_frame_timing[ACTIVE_DISPLAY_V]));
		m_draw_timer->adjust(screen().time_until_pos(vpos, m_draw_time ), vpos_limit - m_frame_timing[ACTIVE_DISPLAY_V]);
		return;
	}

	vpos_limit -= m_frame_timing[ACTIVE_DISPLAY_V];

	/* Check if we're in the active display area */
	if (vpos >= vpos_limit)
	{
		if (vpos == vpos_limit)
		{
			m_reg9copy = m_reg[0x09];
		}

		if (m_line_counter == 0x00)
		{
			m_line_counter = m_reg[0x0a];
			m_hint_timer->adjust(screen().time_until_pos(vpos, m_line_timing[HINT_HPOS]));
			m_pending_hint = true;
		}
		else
		{
			m_line_counter--;
		}

		/* Draw borders */
		m_lborder_timer->adjust(screen().time_until_pos(vpos, LBORDER_START), vpos);
		m_rborder_timer->adjust(screen().time_until_pos(vpos, LBORDER_START + LBORDER_WIDTH + 256), vpos);

		/* Draw active display */
		select_sprites(vpos - vpos_limit);
		m_draw_timer->adjust(screen().time_until_pos(vpos, m_draw_time), vpos_limit);
		return;
	}

	vpos_limit -= m_frame_timing[TOP_BORDER];

	/* Check if we're in the top border area */
	if (vpos >= vpos_limit)
	{
		m_line_counter = m_reg[0x0a];

		/* Check if we're on the last line of the top border */
		if (vpos == vpos_limit + m_frame_timing[TOP_BORDER] - 1)
		{
			m_hcounter_latched = false;
			vblank_end(vpos);
		}

		/* Draw borders */
		m_lborder_timer->adjust(screen().time_until_pos(vpos, LBORDER_START), vpos);
		m_rborder_timer->adjust(screen().time_until_pos(vpos, LBORDER_START + LBORDER_WIDTH + 256), vpos);

		/* Draw middle of the border */
		/* We need to do this through the regular drawing function */
		/* so sprite collisions can occur on the border. */
		select_sprites(vpos - (vpos_limit + m_frame_timing[TOP_BORDER]));
		m_draw_timer->adjust(screen().time_until_pos(vpos, m_draw_time), vpos_limit + m_frame_timing[TOP_BORDER]);
		return;
	}

	/* we're in the vertical sync or top blanking areas */
	m_line_counter = m_reg[0x0a];
}


u8 sega315_5124_device::data_read()
{
	/* Return data buffer contents */
	const u8 temp = m_buffer;

	/* Clear pending write flag */
	m_pending_control_write = false;

	if (!machine().side_effects_disabled())
	{
		/* Load data buffer */
		m_buffer = this->space().read_byte(m_addr & 0x3fff);

		/* Bump internal address register */
		m_addr += 1;
	}
	return temp;
}


void sega315_5124_device::check_pending_flags()
{
	if (!(m_pending_status & (STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL)) && !m_pending_hint)
		return;

	/* A timer ensures that this function will run at least at end of each line.
	   When this function runs through a CPU instruction executed when the timer
	   was about to fire, the time added in the CPU timeslice may make hpos()
	   return some position in the beginning of next line. To ensure the instruction
	   will get updated status, here a maximum hpos is set if the timer reports no
	   remaining time, what could also occur due to the ahead time of the timeslice. */
	int const hpos = (m_pending_flags_timer->remaining() == attotime::zero) ? (WIDTH - 1) : screen_hpos();

	if ((m_pending_hint) && hpos >= m_line_timing[HINT_HPOS])
	{
		m_pending_hint = false;
		m_hint_occurred = true;
	}
	if ((m_pending_status & STATUS_VINT) && hpos >= m_line_timing[VINT_FLAG_HPOS])
	{
		m_pending_status &= ~STATUS_VINT;
		m_status |= STATUS_VINT;
	}
	if ((m_pending_status & STATUS_SPROVR) && hpos >= m_line_timing[SPROVR_HPOS])
	{
		m_pending_status &= ~STATUS_SPROVR;
		m_status |= STATUS_SPROVR;
		// copy and reset the pending bits that were based on the number
		// of the first sprite that overflowed.
		m_status &= m_pending_status | (STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL);
		m_pending_status |= ~(STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL);
	}
	if ((m_pending_status & STATUS_SPRCOL) && hpos >= m_pending_sprcol_x)
	{
		m_pending_status &= ~STATUS_SPRCOL;
		m_status |= STATUS_SPRCOL;
		m_pending_sprcol_x = 0;
	}
}


u8 sega315_5124_device::control_read()
{
	check_pending_flags();
	const u8 result = m_status;

	if (!machine().side_effects_disabled())
	{
		/* Clear pending write flag */
		m_pending_control_write = false;

		/* Clear status flags */
		m_hint_occurred = false;
		m_status = u8(~(STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL));

		if (m_n_int_state == 0)
		{
			m_n_int_state = 1;
			m_n_int_cb(CLEAR_LINE);
		}
	}

	return result;
}


void sega315_5124_device::write_memory(u8 data)
{
	switch (m_addrmode)
	{
	case 0x00:
	case 0x01:
	case 0x02:
		this->space().write_byte(m_addr & 0x3fff, data);
		break;

	case 0x03:
		cram_write(data);
		break;
	}

	// data written to data port loads the data buffer.
	m_buffer = data;
}


void sega315_5313_mode4_device::write_memory(u8 data)
{
	switch (m_addrmode)
	{
	case 0x00:
	case 0x01:
		this->space().write_byte(m_addr & 0x3fff, data);
		break;

	case 0x02:
	case 0x03:
		cram_write(data);
		break;
	}
	// data buffer isn't loaded on data port write.
}


void sega315_5124_device::data_write(u8 data)
{
	/* Clear pending write flag */
	m_pending_control_write = false;

	write_memory(data);
	m_addr += 1;
}


void sega315_5124_device::load_vram_addr(u8 data)
{
	// Seems like the latched data is passed straight through
	// to the address register when in the middle of doing a command.
	// Cosmic Spacehead needs this, among others
	if (m_pending_control_write)
		m_addr = (m_addr & 0xff00) | data;
	else
		m_addr = (data << 8) | (m_addr & 0xff);
}


void sega315_5313_mode4_device::load_vram_addr(u8 data)
{
	if (m_pending_control_write)
		m_control_write_data_latch = data;
	else
		m_addr = (data << 8) | m_control_write_data_latch;
}


void sega315_5124_device::control_write(u8 data)
{
	if (!m_pending_control_write)
	{
		m_pending_control_write = true;
		load_vram_addr(data);
	}
	else
	{
		int reg_num;

		/* Clear pending write flag */
		m_pending_control_write = false;

		m_addrmode = (data >> 6) & 0x03;
		load_vram_addr(data);
		switch (m_addrmode)
		{
		case 0:     /* VRAM reading mode */
			m_buffer = this->space().read_byte(m_addr & 0x3fff);
			m_addr += 1;
			break;

		case 1:     /* VRAM writing mode */
			break;

		case 2:     /* VDP register write */
			reg_num = data & m_reg_num_mask;
			// for the 315-5313, the proper bit count for register
			// numbers is emulated, but because it allows for more
			// than 16 registers, what is not implemented, there is
			// a break here if the limit is surpassed.
			if (reg_num >= 16)
			{
				break;
			}
			m_reg[reg_num] = m_addr & 0xff;
			LOGREGWRITE("%s: %s: setting register %x to %02x\n", machine().describe_context(), tag(), reg_num, m_addr & 0xff);

			switch (reg_num)
			{
			case 0:
				set_display_settings();
				if (BIT(m_addr, 1))
					LOGVIDMODE("overscan enabled.\n");
				break;
			case 1:
				set_display_settings();
				if (screen_hpos() <= DISPLAY_DISABLED_HPOS)
					m_display_disabled = !BIT(m_reg[0x01], 6);
				break;
			case 8:
				if (screen_hpos() <= m_line_timing[XSCROLL_HPOS])
					m_reg8copy = m_reg[0x08];
			}

			check_pending_flags();

			if ((reg_num == 0 && (m_hint_occurred)) || (reg_num == 1 && (m_status & STATUS_VINT)))
			{
				// For HINT disabling through register 00:
				// "Line IRQ VCount" test, of Flubba's VDPTest ROM, disables HINT to wait
				// for next VINT, but HINT occurs when the operation is about to execute.
				// So here, where the setting is done, the /INT state needs to be cleared.
				//
				// For VINT disabling through register 01:
				// When running eagles5 on the smskr driver the /INT state is 0 because of some
				// previous HINTs that occurred. eagles5 sets register 01 to 0x02 and expects
				// the irq state to be cleared after that.
				// The following bit of code takes care of that.
				//
				if ((reg_num == 0 && !BIT(m_reg[0x00], 4)) ||
						(reg_num == 1 && !BIT(m_reg[0x01], 5)))
				{
					if (m_n_int_state == 0)
					{
						m_n_int_state = 1;
						m_n_int_cb(CLEAR_LINE);
					}
				}
				else
				{
					// For register 01 and VINT enabling:
					// Assert the /INT line for the scoreboard of robocop3,
					// on the sms/smspal driver, be displayed correctly.
					//
					// Assume the same behavior for reg0+HINT.
					//
					m_n_int_state = 0;
					m_n_int_cb(ASSERT_LINE);
				}
			}
			m_addrmode = 0;
			break;

		case 3:     /* CRAM writing mode */
			break;
		}
	}
}


void sega315_5124_device::draw_leftmost_pixels_mode4(int *line_buffer, int *priority_selected, int fine_x_scroll, int palette_selected, int tile_line)
{
	// To draw the leftmost pixels when they aren't part of a tile column
	// due to scrolling, Sega Master System has a weird behaviour to select
	// which palette will be used to obtain the color in entry 0, that
	// depends on the content of tile 0x100.
	// This implementation mimics the behaviour of the Emulicious emulator,
	// seen with the test ROM provided by sverx here:
	//
	// http://www.smspower.org/forums/15653-CommunityEffortRequestHelpDiscoverHowTheVDPHandlesTheLeftmostPixelsWhenScrolling
	//
	// From the tile 0x100, it uses bit 1 of the plane that would
	// select the color for the pixel 4 at the current line.

	const int pixel_x = 4;
	// The parsing seems to occur before the line counter is incremented
	// to the number of the line to be drawn.
	int parse_line = tile_line - 1;

	// Select tile 0x100.
	// Tried before with the number stored in m_sprite_tile_selected[0],
	// but with it the expected behaviour only occurs with some BIOSes.
	const int tile_selected = 0x100;

	// Load data of bit plane 1 for the selected tile.
	const int tmp_bit_plane_1 = space().read_byte((tile_selected << 5) + ((parse_line & 0x07) << 2) + 0x01);
	const u8 pen_bit_1 = BIT(tmp_bit_plane_1, 7 - pixel_x);

	for (int pixel_plot_x = 0; pixel_plot_x < fine_x_scroll; pixel_plot_x++)
	{
		line_buffer[pixel_plot_x] = m_current_palette[pen_bit_1 ? 0x10 : 0x00];
		priority_selected[pixel_plot_x] = 0;
	}
}


void sega315_5313_mode4_device::draw_leftmost_pixels_mode4(int *line_buffer, int *priority_selected, int fine_x_scroll, int palette_selected, int tile_line)
{
	// To draw the leftmost pixels when they aren't part of a tile column
	// due to scrolling, Sega Genesis/Mega Drive seems to use entry #0 of
	// the palette selected by the next background tile to be drawn on screen.

	for (int pixel_plot_x = 0; pixel_plot_x < fine_x_scroll; pixel_plot_x++)
	{
		line_buffer[pixel_plot_x] = m_current_palette[palette_selected ? 0x10 : 0x00];
		priority_selected[pixel_plot_x] = 0;
	}
}


u16 sega315_5124_device::name_row_mode4(u16 row)
{
	return row & (((m_reg[0x02] & 0x01) << 10) | 0x3bff);
}


u16 sega315_5246_device::name_row_mode4(u16 row)
{
	return row;
}


u16 sega315_5124_device::tile1_select_mode4(u16 tile_number)
{
	return tile_number & ((m_reg[0x03] << 1) | 1);
}


u16 sega315_5246_device::tile1_select_mode4(u16 tile_number)
{
	return tile_number;
}


u16 sega315_5124_device::tile2_select_mode4(u16 tile_number)
{
	return tile_number & (((m_reg[0x04] & 0x07) << 6) | 0x03f);
}


u16 sega315_5246_device::tile2_select_mode4(u16 tile_number)
{
	return tile_number;
}


void sega315_5124_device::draw_scanline_mode4(int *line_buffer, int *priority_selected, int line)
{
	// if top 2 rows of screen not affected by horizontal scrolling, then x_scroll = 0
	// else x_scroll = m_reg8copy
	const int x_scroll = ((BIT(m_reg[0x00], 6) && (line < 16)) ? 0 : m_reg8copy);

	const int x_scroll_start_column = 32 - (x_scroll >> 3);             /* x starting column tile */
	const int fine_x_scroll = (x_scroll & 0x07);

	int scroll_mod;
	u16 name_base;
	if (m_y_pixels != 192)
	{
		name_base = ((m_reg[0x02] & 0x0c) << 10) | 0x0700;
		scroll_mod = 256;
	}
	else
	{
		name_base = (m_reg[0x02] << 10) & 0x3800;
		scroll_mod = 224;
	}

	/* Draw background layer */
	for (int tile_column = 0; tile_column < 32; tile_column++)
	{
		int tile_line;
		const int table_column = ((tile_column + x_scroll_start_column) & 0x1f) << 1;

		/* Rightmost 8 columns for SMS (or 2 columns for GG) not affected by */
		/* vertical scrolling when bit 7 of reg[0x00] is set */
		const int y_scroll = (BIT(m_reg[0x00], 7) && (tile_column > 23)) ? 0 : m_reg9copy;

		const u16 name_row = name_row_mode4((((line + y_scroll) % scroll_mod) >> 3) << 6);
		const u16 tile_data = space().read_word(name_base + name_row + table_column);

		const int tile1_selected = tile1_select_mode4(tile_data & 0x01ff);
		const int tile2_selected = tile2_select_mode4(tile_data & 0x01ff);
		const int priority_select = tile_data & PRIORITY_BIT;
		const int palette_selected = BIT(tile_data, 11);
		const int vert_selected = BIT(tile_data, 10);
		const int horiz_selected = BIT(tile_data, 9);

		tile_line = line - ((0x07 - (y_scroll & 0x07)) + 1);
		if (vert_selected)
			tile_line = 0x07 - tile_line;

		const u8 bit_plane_0 = space().read_byte(((tile1_selected << 5) + ((tile_line & 0x07) << 2)) + 0x00);
		const u8 bit_plane_1 = space().read_byte(((tile1_selected << 5) + ((tile_line & 0x07) << 2)) + 0x01);
		const u8 bit_plane_2 = space().read_byte(((tile2_selected << 5) + ((tile_line & 0x07) << 2)) + 0x02);
		const u8 bit_plane_3 = space().read_byte(((tile2_selected << 5) + ((tile_line & 0x07) << 2)) + 0x03);

		/* Column 0 is the leftmost tile column that completely entered in the screen.
		   If the leftmost pixels aren't part of a complete tile, due to horizontal
		   scrolling, they are drawn only with color #0 of the selected palette. */
		if (tile_column == 0 && fine_x_scroll > 0)
			draw_leftmost_pixels_mode4(line_buffer, priority_selected, fine_x_scroll, palette_selected, tile_line);

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			const u8 pen_bit_0 = BIT(bit_plane_0, 7 - pixel_x);
			const u8 pen_bit_1 = BIT(bit_plane_1, 7 - pixel_x);
			const u8 pen_bit_2 = BIT(bit_plane_2, 7 - pixel_x);
			const u8 pen_bit_3 = BIT(bit_plane_3, 7 - pixel_x);

			u8 pen_selected = (pen_bit_3 << 3 | pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0);
			if (palette_selected)
				pen_selected |= 0x10;

			int pixel_plot_x = !horiz_selected ? pixel_x : (7 - pixel_x);

			pixel_plot_x = fine_x_scroll + (tile_column << 3) + pixel_plot_x;
			if (pixel_plot_x < 256)
			{
				line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
				priority_selected[pixel_plot_x] = priority_select | (pen_selected & 0x0f);
			}
		}
	}
}


u8 sega315_5124_device::sprite_attribute_extra_offset_mode4(u8 offset)
{
	return offset & ((BIT(m_reg[0x05], 0) << 7) | 0x7f);
}


u8 sega315_5246_device::sprite_attribute_extra_offset_mode4(u8 offset)
{
	return offset;
}


u8 sega315_5124_device::sprite_tile_select_mode4(u8 tile_number)
{
	return tile_number & (((m_reg[0x06] & 0x03) << 6) | 0x3f);
}


u8 sega315_5246_device::sprite_tile_select_mode4(u8 tile_number)
{
	return tile_number;
}


void sega315_5124_device::sprite_count_overflow(int line, int sprite_index)
{
	/* Overflow is flagged only on active display and when VINT isn't active */
	if (!(m_status & STATUS_VINT) && line >= 0 && line < m_frame_timing[ACTIVE_DISPLAY_V])
	{
		u8 sprite_number_bits;
		m_pending_status |= STATUS_SPROVR;
		if (sprite_index < 14)
			sprite_number_bits = (sprite_index + 1) / 2;
		else
			sprite_number_bits = sprite_index / 2;

		m_pending_status &= sprite_number_bits | (STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL);
	}
}


void sega315_5313_mode4_device::sprite_count_overflow(int line, int sprite_index)
{
	if (!m_display_disabled)
	{
		sega315_5124_device::sprite_count_overflow(line, sprite_index);
	}
}


void sega315_5124_device::select_sprites(int line)
{
	m_sprite_count = 0;
	if (m_vdp_mode == 1)
	{
		/* Text mode, no sprite processing */
		return;
	}

	/* Check if SI is set */
	m_sprite_height = BIT(m_reg[0x01], 1) ? 16 : 8;
	/* Check if MAG is set */
	m_sprite_zoom_scale = BIT(m_reg[0x01], 0) ? 2 : 1;

	if (m_vdp_mode < 4)
	{
		/* TMS9918 compatibility sprites */

		const int max_sprites = 4;

		m_sprite_attribute_base = ((m_reg[0x05] & 0x7f) << 7);

		for (int sprite_index = 0; (sprite_index < 32 * 4); sprite_index += 4)
		{
			/* At this point the VDP vcount still doesn't refer the new line,
			   because the logical start point is slightly shifted on the scanline */
			int parse_line = line - 1;

			int sprite_y = space().read_byte(m_sprite_attribute_base + sprite_index);
			if (sprite_y == 0xd0)
				break;

			if (sprite_y >= 240)
			{
				sprite_y -= 256; /* wrap from top if y position is >= 240 */
			}

			if (m_sprite_zoom_scale > 1 && m_sprite_count < m_max_sprite_zoom_vcount)
			{
				/* Divide before use the value for comparison, or else an
				   off-by-one bug could occur, as seen with Tarzan, for Game Gear */
				parse_line >>= 1;
				sprite_y >>= 1;
			}

			if ((parse_line >= sprite_y) && (parse_line < (sprite_y + m_sprite_height)))
			{
				if (m_sprite_count < max_sprites)
				{
					const int sprite_x = space().read_byte(m_sprite_attribute_base + sprite_index + 1);
					int sprite_tile_selected = space().read_byte(m_sprite_attribute_base + sprite_index + 2);
					const u8 flags = space().read_byte(m_sprite_attribute_base + sprite_index + 3);

					int sprite_line = parse_line - sprite_y;

					if (m_sprite_height == 16)
					{
						sprite_tile_selected &= 0xfc;

						if (sprite_line > 0x07)
						{
							sprite_tile_selected += 1;
							sprite_line -= 8;
						}
					}

					m_sprite_x[m_sprite_count] = sprite_x;
					m_sprite_tile_selected[m_sprite_count] = sprite_tile_selected;
					m_sprite_flags[m_sprite_count] = flags;
					m_sprite_pattern_line[m_sprite_count] = ((m_reg[0x06] & 0x07) << 11) + sprite_line;

					m_sprite_count++;
				}
				else
				{
					sprite_count_overflow(line, sprite_index);
				}
			}
		}
	}
	else
	{
		/* Regular sprites */

		const int max_sprites = 8;

		m_sprite_attribute_base = (m_reg[0x05] << 7) & 0x3f00;
		const u16 sprite_attribute_extra_base = m_sprite_attribute_base + sprite_attribute_extra_offset_mode4(0x80);

		for (int sprite_index = 0; (sprite_index < 64); sprite_index++)
		{
			/* At this point the VDP vcount still doesn't refer the new line,
			   because the logical start point is slightly shifted on the scanline */
			int parse_line = line - 1;

			int sprite_y = space().read_byte(m_sprite_attribute_base + sprite_index);
			if (m_y_pixels == 192 && sprite_y == 0xd0)
				break;

			if (sprite_y >= 240)
			{
				sprite_y -= 256; /* wrap from top if y position is >= 240 */
			}

			if (m_sprite_zoom_scale > 1 && m_sprite_count < m_max_sprite_zoom_vcount)
			{
				/* Divide before use the value for comparison, or else an
				   off-by-one bug could occur, as seen with Tarzan, for Game Gear */
				parse_line >>= 1;
				sprite_y >>= 1;
			}

			if ((parse_line >= sprite_y) && (parse_line < (sprite_y + m_sprite_height)))
			{
				if (m_sprite_count < max_sprites)
				{
					const int sprite_line = parse_line - sprite_y;
					int sprite_x = space().read_byte(sprite_attribute_extra_base + (sprite_index << 1));
					int sprite_tile_number = space().read_byte(sprite_attribute_extra_base + (sprite_index << 1) + 1);

					int sprite_tile_selected = sprite_tile_select_mode4(sprite_tile_number);

					if (BIT(m_reg[0x00], 3))
						sprite_x -= 0x08;    /* sprite shift */

					if (BIT(m_reg[0x06], 2))
						sprite_tile_selected += 256; /* pattern table select */

					if (m_sprite_height == 16)
						sprite_tile_selected &= 0x01fe; /* force even index */

					if (sprite_line > 0x07)
						sprite_tile_selected += 1;

					m_sprite_x[m_sprite_count] = sprite_x;
					m_sprite_tile_selected[m_sprite_count] = sprite_tile_selected;
					m_sprite_pattern_line[m_sprite_count] = ((sprite_line & 0x07) << 2);

					m_sprite_count++;
				}
				else
				{
					sprite_count_overflow(line, sprite_index);
				}
			}
		}
	}
}


void sega315_5313_mode4_device::select_sprites(int line)
{
	sega315_5124_device::select_sprites(line);

	/*
	    Info from Charles MacDonald regarding real hardware behavior:
	    (http://www.smspower.org/forums/15772-Sprites8x16Question)

	    "As I recall the SMS will parses the sprite table on line N to find the
	    sprite numbers to display on line N+1, and on the next line it displays
	    those sprites. The data for the sprites is read in real-time and is not
	    buffered from any earlier time. The only thing that's buffered are the
	    eight sprite numbers.

	    On the MD it has two line buffers (RAM) that it renders sprite data into
	    on one line, and displays that buffer on the next line while preparing
	    the next one. This only reveals itself if you play with the screen
	    blanking or left-column blanking bits, as the display process erases the
	    other buffer and on a blanked line nothing gets erased.

	    So if you turn on either of those bits later they show the old sprite
	    data that hadn't been erased yet. This is how you can show a line of
	    sprite data further down on the screen than where the sprites were."
	*/

	// Runs the function that draws sprites, but only to check for
	// sprite collision, due to it be flagged on the 315-5313 VDP in
	// a point where the active screen was not drawn yet.
	if (m_sprite_count > 0 && !m_display_disabled)
	{
		int blitline_buffer[256];
		int priority_selected[256];
		draw_sprites_mode4(blitline_buffer, priority_selected, line);
	}
}


void sega315_5124_device::sprite_collision(int line, int sprite_col_x)
{
	/* SMS/GG: collisions don't occur on column 0 if it is disabled. */
	if (BIT(m_reg[0x00], 5) && sprite_col_x < 8)
		return;

	m_pending_status |= STATUS_SPRCOL;
	m_pending_sprcol_x = m_line_timing[SPRCOL_BASEHPOS] + sprite_col_x;
}


void sega315_5313_mode4_device::sprite_collision(int line, int sprite_col_x)
{
	if (line >= 0 && line < m_frame_timing[ACTIVE_DISPLAY_V])
	{
		// This function is been used to check for sprite collision of
		// the 315-5313 VDP, that occurs before the active screen is
		// drawn, so it must not flag a collision again when drawing.
		if (screen_hpos() < m_draw_time)
		{
			m_pending_status |= STATUS_SPRCOL;
			m_pending_sprcol_x = m_line_timing[SPRCOL_BASEHPOS];
		}
	}
}


void sega315_5124_device::draw_sprites_mode4(int *line_buffer, int *priority_selected, int line)
{
	if (m_display_disabled || m_sprite_count == 0)
		return;

	bool sprite_col_occurred = false;
	int sprite_col_x = 255;
	u8 collision_buffer[256] = { 0 };

	// Draw sprite layer
	for (int sprite_buffer_index = m_sprite_count - 1; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		const int sprite_x = m_sprite_x[sprite_buffer_index];
		const int sprite_tile_selected = m_sprite_tile_selected[sprite_buffer_index];
		const u16 sprite_pattern_line = m_sprite_pattern_line[sprite_buffer_index];
		const int zoom_scale = sprite_buffer_index < m_max_sprite_zoom_hcount ? m_sprite_zoom_scale : 1;

		const u8 bit_plane_0 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x00);
		const u8 bit_plane_1 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x01);
		const u8 bit_plane_2 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x02);
		const u8 bit_plane_3 = space().read_byte((sprite_tile_selected << 5) + sprite_pattern_line + 0x03);

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			const u8 pen_bit_0 = BIT(bit_plane_0, 7 - pixel_x);
			const u8 pen_bit_1 = BIT(bit_plane_1, 7 - pixel_x);
			const u8 pen_bit_2 = BIT(bit_plane_2, 7 - pixel_x);
			const u8 pen_bit_3 = BIT(bit_plane_3, 7 - pixel_x);
			const u8 pen_selected = (pen_bit_3 << 3 | pen_bit_2 << 2 | pen_bit_1 << 1 | pen_bit_0) | 0x10;

			if (pen_selected == 0x10) // Transparent palette so skip draw
				continue;

			int pixel_plot_x;
			if (zoom_scale > 1)
			{
				/* sprite doubling is enabled */
				pixel_plot_x = sprite_x + (pixel_x << 1);
			}
			else
			{
				pixel_plot_x = sprite_x + pixel_x;
			}

			/* Draw at pixel position and, if zoomed, at pixel+1 */
			for (int zoom = 0; zoom < zoom_scale; zoom++)
			{
				pixel_plot_x += zoom;

				/* check to prevent going outside of active display area */
				if (pixel_plot_x < 0 || pixel_plot_x > 255)
					continue;

				/* Draw sprite pixel */
				/* Check if the background has lower priority */
				if (!(priority_selected[pixel_plot_x] & PRIORITY_BIT))
				{
					line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
					priority_selected[pixel_plot_x] = pen_selected;
				}
				else
				{
					/* Check if the higher priority background has transparent pixel */
					if (priority_selected[pixel_plot_x] == PRIORITY_BIT)
					{
						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
						priority_selected[pixel_plot_x] = pen_selected;
					}
				}
				if (collision_buffer[pixel_plot_x] != 1)
				{
					collision_buffer[pixel_plot_x] = 1;
				}
				else
				{
					sprite_col_occurred = true;
					sprite_col_x = std::min(sprite_col_x, pixel_plot_x);
				}
			}
		}

		if (sprite_col_occurred)
			sprite_collision(line, sprite_col_x);
	}
}


void sega315_5124_device::draw_sprites_tms9918_mode(int *line_buffer, int line)
{
	if (m_display_disabled || m_sprite_count == 0)
		return;

	bool sprite_col_occurred = false;
	int sprite_col_x = 255;
	u8 collision_buffer[256] = { 0 };

	/* Draw sprite layer */
	for (int sprite_buffer_index = m_sprite_count - 1; sprite_buffer_index >= 0; sprite_buffer_index--)
	{
		int sprite_x = m_sprite_x[sprite_buffer_index];
		int sprite_tile_selected = m_sprite_tile_selected[sprite_buffer_index];
		const u16 sprite_pattern_line = m_sprite_pattern_line[sprite_buffer_index];
		const u8 flags = m_sprite_flags[sprite_buffer_index];
		const int pen_selected = m_palette_offset + (flags & 0x0f);
		const int zoom_scale = sprite_buffer_index < m_max_sprite_zoom_hcount ? m_sprite_zoom_scale : 1;

		if (BIT(flags, 7))
			sprite_x -= 32;

		for (int height = 8; height <= m_sprite_height; height += 8)
		{
			if (height == 16)
			{
				sprite_tile_selected += 2;
				sprite_x += (zoom_scale > 1 ? 16 : 8);
			}

			u8 pattern = space().read_byte(sprite_pattern_line + sprite_tile_selected * 8);

			for (int pixel_x = 0; pixel_x < 8; pixel_x++)
			{
				if (pen_selected && BIT(pattern, 7 - pixel_x))
				{
					int pixel_plot_x;
					if (zoom_scale > 1)
						pixel_plot_x = sprite_x + (pixel_x << 1);
					else
						pixel_plot_x = sprite_x + pixel_x;

					/* Draw at pixel position and, if zoomed, at pixel+1 */
					for (int zoom = 0; zoom < zoom_scale; zoom++)
					{
						pixel_plot_x += zoom;

						/* check to prevent going outside of active display area */
						if (pixel_plot_x < 0 || pixel_plot_x > 255)
							continue;

						line_buffer[pixel_plot_x] = m_current_palette[pen_selected];

						if (collision_buffer[pixel_plot_x] != 1)
						{
							collision_buffer[pixel_plot_x] = 1;
						}
						else
						{
							sprite_col_occurred = true;
							sprite_col_x = std::min(sprite_col_x, pixel_plot_x);
						}
					}
				}
			}
		}

		if (sprite_col_occurred)
			sprite_collision(line, sprite_col_x);
	}
}


// Display mode 2 (Graphics II Mode)
void sega315_5124_device::draw_scanline_mode2(int *line_buffer, int line)
{
	const u16 name_base = ((m_reg[0x02] & 0x0f) << 10);
	const u16 color_base = ((m_reg[0x03] & 0x80) << 6);
	const int color_mask = ((m_reg[0x03] & 0x7f) << 3) | 0x07;
	const u16 pattern_base = ((m_reg[0x04] & 0x04) << 11);
	const int pattern_mask = ((m_reg[0x04] & 0x03) << 8) | 0xff;
	const int pattern_offset = (line & 0xc0) << 2;
	const u16 name_row_base = name_base + ((line >> 3) * 32);

	/* Draw background layer */
	for (int tile_column = 0; tile_column < 32; tile_column++)
	{
		const u8 name = space().read_byte(name_row_base + tile_column);
		const u8 pattern = space().read_byte(pattern_base + (((pattern_offset + name) & pattern_mask) * 8) + (line & 0x07) );
		const u8 colors = space().read_byte(color_base + (((pattern_offset + name) & color_mask) * 8) + (line & 0x07) );

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			const int pixel_plot_x = (tile_column << 3) + pixel_x;

			u8 pen_selected;
			if (BIT(pattern, 7 - pixel_x))
				pen_selected = colors >> 4;
			else
				pen_selected = colors & 0x0f;

			if (!pen_selected)
				pen_selected = BACKDROP_COLOR;

			pen_selected += m_palette_offset;
			line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
		}
	}
}


// Display mode 0 (Graphics I Mode)
void sega315_5124_device::draw_scanline_mode0(int *line_buffer, int line)
{
	const u16 name_base = ((m_reg[0x02] & 0x0f) << 10);
	const u16 color_base = ((m_reg[0x03] << 6) & (VRAM_SIZE - 1));
	const u16 pattern_base = ((m_reg[0x04] << 11) & (VRAM_SIZE - 1));
	const u16 name_row_base = name_base + ((line >> 3) * 32);

	/* Draw background layer */
	for (int tile_column = 0; tile_column < 32; tile_column++)
	{
		const u8 name = space().read_byte(name_row_base + tile_column);
		const u8 pattern = space().read_byte(pattern_base + (name << 3) + (line & 0x07));
		const u8 colors = space().read_byte(color_base + (name >> 3));

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			int pen_selected;
			const int pixel_plot_x = (tile_column << 3) + pixel_x;

			if (BIT(pattern, 7 - pixel_x))
				pen_selected = colors >> 4;
			else
				pen_selected = colors & 0x0f;

			if (!pen_selected)
				pen_selected = BACKDROP_COLOR;

			pen_selected += m_palette_offset;
			line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
		}
	}
}


// Display mode 1 (Text Mode)
void sega315_5124_device::draw_scanline_mode1(int *line_buffer, int line)
{
	const u16 name_base = ((m_reg[0x02] & 0x0f) << 10);
	const u16 pattern_base = ((m_reg[0x04] << 11) & (VRAM_SIZE - 1));
	const u16 name_row_base = name_base + ((line >> 3) * 32);

	for (int pixel_plot_x = 0; pixel_plot_x < 8; pixel_plot_x++)
	{
		line_buffer[pixel_plot_x] = m_current_palette[BACKDROP_COLOR];
	}

	/* Draw background layer */
	for (int tile_column = 0; tile_column < 40; tile_column++)
	{
		const u8 name = space().read_byte(name_row_base + tile_column);
		const u8 pattern = space().read_byte(pattern_base + (name << 3) + (line & 0x07));

		for (int pixel_x = 0; pixel_x < 6; pixel_x++)
		{
			const int pixel_plot_x = (tile_column * 6) + pixel_x + 8;

			int pen_selected;
			if (BIT(pattern, 7 - pixel_x))
				pen_selected = m_reg[0x07] >> 4;
			else
				pen_selected = m_reg[0x07] & 0x0f;

			if (!pen_selected)
				pen_selected = BACKDROP_COLOR;

			pen_selected += m_palette_offset;
			line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
		}
	}

	for (int pixel_plot_x = 248; pixel_plot_x < 256; pixel_plot_x++)
		line_buffer[pixel_plot_x] = m_current_palette[BACKDROP_COLOR];
}


// Display mode 3 (Multicolor Mode)
void sega315_5124_device::draw_scanline_mode3(int *line_buffer, int line)
{
	const u16 name_base = ((m_reg[0x02] & 0x0f) << 10);
	const u16 pattern_base = ((m_reg[0x04] << 11) & (VRAM_SIZE - 1));
	const u16 name_row_base = name_base + ((line >> 3) * 32);

	/* Draw background layer */
	for (int tile_column = 0; tile_column < 32; tile_column++)
	{
		const u8 name = space().read_byte(name_row_base + tile_column);
		const u8 pattern = space().read_byte(pattern_base + (name << 3) + (((line >> 3) & 3) << 1) + ((line & 4) >> 2));

		for (int pixel_x = 0; pixel_x < 8; pixel_x++)
		{
			const int pixel_plot_x = (tile_column << 3) + pixel_x;
			int pen_selected = (pattern >> (~pixel_x & 4)) & 0x0f;

			if (!pen_selected)
				pen_selected = BACKDROP_COLOR;

			pen_selected += m_palette_offset;
			line_buffer[pixel_plot_x] = m_current_palette[pen_selected];
		}
	}
}


TIMER_CALLBACK_MEMBER(sega315_5124_device::draw_scanline)
{
	int pixel_offset_x = LBORDER_START + LBORDER_WIDTH;
	int pixel_plot_y = param;
	int line = screen().vpos() - param;

	update_palette();

	int blitline_buffer[256];
	int priority_selected[256];

	/* Sprite processing is restricted because collisions on top border of extended
	   resolution break the scoreboard of Fantasy Dizzy (SMS) on smspal driver */

	if (line < m_frame_timing[ACTIVE_DISPLAY_V])
	{
		std::fill(std::begin(priority_selected), std::end(priority_selected), 1);

		switch (m_vdp_mode)
		{
		case 0:
			if (line >= 0)
				draw_scanline_mode0(blitline_buffer, line);
			if (line >= 0 || (line >= -13 && m_y_pixels == 192))
				draw_sprites_tms9918_mode(blitline_buffer, line);
			break;

		case 1:
			if (line >= 0)
				draw_scanline_mode1(blitline_buffer, line);
			// Text Mode, no sprite drawing.
			break;

		case 2:
			if (line >= 0)
				draw_scanline_mode2(blitline_buffer, line);
			if (line >= 0 || (line >= -13 && m_y_pixels == 192))
				draw_sprites_tms9918_mode(blitline_buffer, line);
			break;

		case 3:
			if (line >= 0)
				draw_scanline_mode3(blitline_buffer, line);
			if (line >= 0 || (line >= -13 && m_y_pixels == 192))
				draw_sprites_tms9918_mode(blitline_buffer, line);
			break;

		case 4:
		default:
			if (line >= 0)
				draw_scanline_mode4(blitline_buffer, priority_selected, line);
			if (line >= 0 || (line >= -13 && m_y_pixels == 192))
				draw_sprites_mode4(blitline_buffer, priority_selected, line);
			break;

		case 5:
			// Mode 5 (Mega Drive / Genesis) not implemented.
			break;
		}
	}

	/* Check if display is disabled or we're below/above active area */
	if (m_display_disabled || line < 0 || line >= m_frame_timing[ACTIVE_DISPLAY_V])
	{
		const rectangle rec(
				pixel_offset_x,
				pixel_offset_x + 255,
				pixel_plot_y + line,
				pixel_plot_y + line);

		m_tmpbitmap.fill(m_palette_lut->pen(m_current_palette[BACKDROP_COLOR]), rec);
		m_y1_bitmap.fill((m_reg[0x07] & 0x0f) ? 1 : 0, rec);
	}
	else
	{
		blit_scanline(blitline_buffer, priority_selected, pixel_offset_x, pixel_plot_y, line);
	}
}


void sega315_5124_device::blit_scanline(int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line)
{
	u32 *const p_bitmap = &m_tmpbitmap.pix(pixel_plot_y + line, pixel_offset_x);
	u8  *const p_y1 = &m_y1_bitmap.pix(pixel_plot_y + line, pixel_offset_x);
	int x = 0;

	if (m_vdp_mode == 4 && BIT(m_reg[0x00], 5))
	{
		/* Fill column 0 with overscan color from m_reg[0x07] */
		do
		{
			p_bitmap[x] = m_palette_lut->pen(m_current_palette[BACKDROP_COLOR]);
			p_y1[x] = (m_reg[0x07] & 0x0f) ? 1 : 0;
		}
		while(++x < 8);
	}

	do
	{
		p_bitmap[x] = m_palette_lut->pen(line_buffer[x]);
		p_y1[x] = (priority_selected[x] & 0x0f) ? 1 : 0;
	}
	while(++x < 256);
}


void sega315_5377_device::blit_scanline(int *line_buffer, int *priority_selected, int pixel_offset_x, int pixel_plot_y, int line)
{
	if (m_sega315_5124_compatibility_mode)
	{
		sega315_5124_device::blit_scanline(line_buffer, priority_selected, pixel_offset_x, pixel_plot_y, line);
	}
	else
	{
		u32 *const p_bitmap = &m_tmpbitmap.pix(pixel_plot_y + line, pixel_offset_x);
		u8  *const p_y1 = &m_y1_bitmap.pix(pixel_plot_y + line, pixel_offset_x);
		int x = 0;

		/* border on left side of the GG active screen */
		do
		{
			p_bitmap[x] = m_palette_lut->pen(m_current_palette[BACKDROP_COLOR]);
			p_y1[x] = (m_reg[0x07] & 0x0f) ? 1 : 0;
		}
		while (++x < 48);

		if (line >= 24 && line < 168)
		{
			do
			{
				p_bitmap[x] = m_palette_lut->pen(line_buffer[x]);
				p_y1[x] = (priority_selected[x] & 0x0f) ? 1 : 0;
			}
			while (++x < 208);
		}
		else
		{
			/* top/bottom GG border */
			do
			{
				p_bitmap[x] = m_palette_lut->pen(m_current_palette[BACKDROP_COLOR]);
				p_y1[x] = (m_reg[0x07] & 0x0f) ? 1 : 0;
			}
			while (++x < 208);
		}

		/* border on right side of the GG active screen */
		do
		{
			p_bitmap[x] = m_palette_lut->pen(m_current_palette[BACKDROP_COLOR]);
			p_y1[x] = (m_reg[0x07] & 0x0f) ? 1 : 0;
		}
		while (++x < 256);
	}
}


void sega315_5124_device::update_palette()
{
	/* Exit if palette has no changes */
	if (!m_cram_dirty)
	{
		return;
	}
	m_cram_dirty = false;

	if (m_vdp_mode != 4)
	{
		for(int i = 0; i < 16; i++)
		{
			m_current_palette[i] = 64 + i;
		}
		return;
	}

	for (int i = 0; i < 32; i++)
	{
		m_current_palette[i] = m_CRAM[i] & 0x3f;
	}
}


void sega315_5377_device::update_palette()
{
	/* Exit if palette has no changes */
	if (!m_cram_dirty)
		return;

	m_cram_dirty = false;

	if (m_sega315_5124_compatibility_mode)
	{
		for (int i = 0; i < 32; i++)
			m_current_palette[i] = ((m_CRAM[i] & 0x30) << 6) | ((m_CRAM[i] & 0x0c ) << 4) | ((m_CRAM[i] & 0x03) << 2);
	}
	else
	{
		for (int i = 0; i < 32; i++)
			m_current_palette[i] = (m_CRAM[2*i] | (m_CRAM[2*i+1] << 8)) & 0x0fff;
	}
}


void sega315_5313_mode4_device::update_palette()
{
	/* Exit if palette has no changes */
	if (!m_cram_dirty)
	{
		return;
	}
	m_cram_dirty = false;

	if (m_vdp_mode != 4)
	{
		return;
	}

	for (int i = 0; i < 32; i++)
	{
		m_current_palette[i] = (512 * 3) + (m_CRAM[i] & 0x3f);
	}
}


void sega315_5124_device::cram_write(u8 data)
{
	const u16 address = m_addr & m_cram_mask;
	if (data != m_CRAM[address])
	{
		m_CRAM[address] = data;
		m_cram_dirty = true;
	}
}


void sega315_5377_device::cram_write(u8 data)
{
	if (m_sega315_5124_compatibility_mode)
	{
		sega315_5124_device::cram_write(data);
	}
	else
	{
		if (m_addr & 1)
		{
			const u16 address = (m_addr & m_cram_mask) & ~1;
			if (m_buffer != m_CRAM[address] || data != m_CRAM[address + 1])
			{
				m_CRAM[address] = m_buffer;
				m_CRAM[address + 1] = data;
				m_cram_dirty = true;
			}
		}
	}
}


u32 sega315_5124_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_tmpbitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


// MegaDrive/Genesis VDP (315-5313) is currently coded as superset of the 315-5124
// To support properly SMS VDP in MegaTech and MegaPlay, we start the 315-5124
// in all systems using MegaDrive/Genesis VDP, but this affects the performance
// of the emulator hence we stop it in systems that don't need it
// Proper way to handle this would be implement the 315-5124 modes in the 315-5313
// device instead of running the two chips separately...
void sega315_5313_mode4_device::stop_timers()
{
	m_display_timer->adjust(attotime::never);
	m_pending_flags_timer->adjust(attotime::never);
	m_hint_timer->adjust(attotime::never);
	m_vint_timer->adjust(attotime::never);
	m_nmi_timer->adjust(attotime::never);
	m_draw_timer->adjust(attotime::never);
	m_lborder_timer->adjust(attotime::never);
	m_rborder_timer->adjust(attotime::never);
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

void sega315_5124_device::device_post_load()
{
	set_frame_timing();
}

void sega315_5124_device::device_start()
{
	/* Make temp bitmap for rendering */
	screen().register_screen_bitmap(m_tmpbitmap);
	screen().register_screen_bitmap(m_y1_bitmap);

	m_display_timer = timer_alloc(FUNC(sega315_5124_device::process_line_timer), this);
	m_display_timer->adjust(screen().time_until_pos(0, DISPLAY_CB_HPOS), 0, screen().scan_period());
	m_pending_flags_timer = timer_alloc(FUNC(sega315_5124_device::eol_flag_check), this);
	m_pending_flags_timer->adjust(screen().time_until_pos(0, WIDTH - 1), 0, screen().scan_period());
	m_draw_timer = timer_alloc(FUNC(sega315_5124_device::draw_scanline), this);
	m_lborder_timer = timer_alloc(FUNC(sega315_5124_device::draw_lborder), this);
	m_rborder_timer = timer_alloc(FUNC(sega315_5124_device::draw_rborder), this);
	m_hint_timer = timer_alloc(FUNC(sega315_5124_device::trigger_hint), this);
	m_vint_timer = timer_alloc(FUNC(sega315_5124_device::trigger_vint), this);
	m_nmi_timer = timer_alloc(FUNC(sega315_5124_device::update_nmi), this);

	save_item(NAME(m_status));
	save_item(NAME(m_pending_status));
	save_item(NAME(m_pending_sprcol_x));
	save_item(NAME(m_reg8copy));
	save_item(NAME(m_reg9copy));
	save_item(NAME(m_addrmode));
	save_item(NAME(m_addr));
	save_item(NAME(m_cram_mask));
	save_item(NAME(m_cram_dirty));
	save_item(NAME(m_hint_occurred));
	save_item(NAME(m_pending_hint));
	save_item(NAME(m_pending_control_write));
	save_item(NAME(m_buffer));
	save_item(NAME(m_control_write_data_latch));
	save_item(NAME(m_sega315_5124_compatibility_mode));
	save_item(NAME(m_display_disabled));
	save_item(NAME(m_n_int_state));
	save_item(NAME(m_n_nmi_state));
	save_item(NAME(m_n_nmi_in_state));
	save_item(NAME(m_vdp_mode));
	save_item(NAME(m_y_pixels));
	save_item(NAME(m_line_counter));
	save_item(NAME(m_hcounter));
	save_item(NAME(m_hcounter_latched));
	save_item(NAME(m_reg));
	save_item(NAME(m_current_palette));

	// these were created with register_screen_bitmap which is dynamic, and will reallocate if the screen size changes, saving them is NOT safe with the current core.
	// The Genesis VDP (315_5313.cpp) which uses this as a base in order to support the legacy SMS operation mode can change resolutions for example.
	//save_item(NAME(m_tmpbitmap));
	//save_item(NAME(m_y1_bitmap));
	save_item(NAME(m_draw_time));
	save_item(NAME(m_sprite_attribute_base));
	save_item(NAME(m_sprite_pattern_line));
	save_item(NAME(m_sprite_tile_selected));
	save_item(NAME(m_sprite_x));
	save_item(NAME(m_sprite_flags));
	save_item(NAME(m_sprite_count));
	save_item(NAME(m_sprite_height));
	save_item(NAME(m_sprite_zoom_scale));
	save_item(NAME(m_max_sprite_zoom_hcount));
	save_item(NAME(m_max_sprite_zoom_vcount));
	save_item(NAME(m_CRAM));
}


void sega315_5124_device::device_reset()
{
	/* Most register are 0x00 at power-up */
	std::fill(std::begin(m_reg), std::end(m_reg), 0x00);

	m_reg[0x02] = 0x0e;
	m_reg[0x0a] = 0xff;

	m_status = m_pending_status = u8(~(STATUS_VINT | STATUS_SPROVR | STATUS_SPRCOL));
	m_pending_sprcol_x = 0;
	m_pending_control_write = false;
	m_pending_hint = false;
	m_hint_occurred = false;
	m_reg8copy = 0;
	m_reg9copy = 0;
	m_addrmode = 0;
	m_addr = 0;
	m_sega315_5124_compatibility_mode = false;
	m_display_disabled = false;
	m_cram_mask = m_cram_size - 1;
	m_cram_dirty = true;
	m_buffer = 0;
	m_control_write_data_latch = 0;
	m_n_int_state = 1;
	m_n_nmi_state = 1;
	m_n_nmi_in_state = 1;
	m_line_counter = 0;
	m_hcounter = 0;
	m_hcounter_latched = false;
	m_draw_time = DRAW_TIME_SMS;

	std::fill(std::begin(m_current_palette), std::end(m_current_palette), 0);

	set_display_settings();

	/* Clear RAM */
	std::fill(std::begin(m_CRAM), std::end(m_CRAM), 0);
}

//-------------------------------------------------
//  device_add_mconfig - add machine configuration
//-------------------------------------------------

void sega315_5124_device::device_add_mconfig(machine_config &config)
{
	PALETTE(config, m_palette_lut, FUNC(sega315_5124_device::sega315_5124_palette), SEGA315_5124_PALETTE_SIZE);

	SEGAPSG(config, m_snsnd, DERIVED_CLOCK(1, 3)).add_route(ALL_OUTPUTS, *this, 1.0, 0);
}

void sega315_5246_device::device_add_mconfig(machine_config &config)
{
	sega315_5124_device::device_add_mconfig(config);
	m_palette_lut->set_init(FUNC(sega315_5246_device::sega315_5246_palette));
}

void sega315_5377_device::device_reset()
{
	sega315_5124_device::device_reset();
	m_draw_time = DRAW_TIME_GG;
}

//-------------------------------------------------
//  device_add_mconfig - add machine configuration
//-------------------------------------------------

void sega315_5377_device::device_add_mconfig(machine_config &config)
{
	sega315_5246_device::device_add_mconfig(config);

	m_palette_lut->set_entries(SEGA315_5377_PALETTE_SIZE);
	m_palette_lut->set_init(FUNC(sega315_5377_device::sega315_5377_palette));

	GAMEGEAR(config.replace(), m_snsnd, DERIVED_CLOCK(1, 3));
	m_snsnd->add_route(0, *this, 1.0, 0);
	m_snsnd->add_route(1, *this, 1.0, 1);
}

//-------------------------------------------------
//  device_add_mconfig - add machine configuration
//-------------------------------------------------

void sega315_5313_mode4_device::device_add_mconfig(machine_config &config)
{
	sega315_5246_device::device_add_mconfig(config);

	m_palette_lut->set_entries((512 * 3) + 64);
	m_palette_lut->set_init(FUNC(sega315_5313_mode4_device::sega315_5313_palette));
}
