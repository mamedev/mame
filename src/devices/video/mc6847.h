// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    mc6847.h

    Implementation of Motorola 6847 video hardware chip

***************************************************************************/

#pragma once

#ifndef __MC6847__
#define __MC6847__


#define MC6847_MODE_AG      0x80
#define MC6847_MODE_GM2     0x40
#define MC6847_MODE_GM1     0x20
#define MC6847_MODE_GM0     0x10
#define MC6847_MODE_CSS     0x08
#define MC6847_MODE_AS      0x04
#define MC6847_MODE_INTEXT  0x02
#define MC6847_MODE_INV     0x01


//**************************************************************************
//  MC6847 CONFIGURATION / INTERFACE
//**************************************************************************

#define MCFG_SCREEN_MC6847_NTSC_ADD(_tag, _mctag) \
	MCFG_SCREEN_ADD(_tag, RASTER)                               \
	MCFG_SCREEN_UPDATE_DEVICE(_mctag, mc6847_base_device, screen_update) \
	MCFG_SCREEN_REFRESH_RATE(60)                                \
	MCFG_SCREEN_SIZE(320, 243)                                  \
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 1, 241-1)                \
	MCFG_SCREEN_VBLANK_TIME(0)

#define MCFG_SCREEN_MC6847_PAL_ADD(_tag, _mctag) \
	MCFG_SCREEN_ADD(_tag, RASTER)                               \
	MCFG_SCREEN_UPDATE_DEVICE(_mctag, mc6847_base_device, screen_update) \
	MCFG_SCREEN_REFRESH_RATE(50)                                \
	MCFG_SCREEN_SIZE(320, 243)                                  \
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 1, 241-1)                \
	MCFG_SCREEN_VBLANK_TIME(0)

#define MCFG_MC6847_HSYNC_CALLBACK(_write) \
	devcb = &mc6847_friend_device::set_hsync_wr_callback(*device, DEVCB_##_write);

#define MCFG_MC6847_FSYNC_CALLBACK(_write) \
	devcb = &mc6847_friend_device::set_fsync_wr_callback(*device, DEVCB_##_write);

#define MCFG_MC6847_CHARROM_CALLBACK(_class, _method) \
	mc6847_friend_device::set_get_char_rom(*device, mc6847_get_char_rom_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_MC6847_INPUT_CALLBACK(_read) \
	devcb = &mc6847_base_device::set_input_callback(*device, DEVCB_##_read);

#define MCFG_MC6847_FIXED_MODE(_mode) \
	mc6847_base_device::set_get_fixed_mode(*device, _mode);

#define MCFG_MC6847_BW(_bw) \
	mc6847_base_device::set_black_and_white(*device, _bw);


typedef device_delegate<UINT8 (UINT8 ch, int line)> mc6847_get_char_rom_delegate;
#define MC6847_GET_CHARROM_MEMBER(_name)   UINT8 _name(UINT8 ch, int line)


#define ARTIFACTING_TAG     "artifacting"

INPUT_PORTS_EXTERN(mc6847_artifacting);


//**************************************************************************
//  MC6847 CORE
//**************************************************************************

// base class so that the GIME emulation can access mc6847 stuff
class mc6847_friend_device : public device_t
{
public:
	// inlines
	bool hs_r(void)                 { return m_horizontal_sync; }
	bool fs_r(void)                 { return m_field_sync; }

	template<class _Object> static devcb_base &set_hsync_wr_callback(device_t &device, _Object object) { return downcast<mc6847_friend_device &>(device).m_write_hsync.set_callback(object); }
	template<class _Object> static devcb_base &set_fsync_wr_callback(device_t &device, _Object object) { return downcast<mc6847_friend_device &>(device).m_write_fsync.set_callback(object); }

	static void set_get_char_rom(device_t &device, mc6847_get_char_rom_delegate callback) { downcast<mc6847_friend_device &>(device).m_charrom_cb = callback; }

protected:
	mc6847_friend_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock,
		const UINT8 *fontdata, bool is_mc6847t1, double tpfs, int field_sync_falling_edge_scanline, bool supports_partial_body_scanlines, const char *shortname, const char *source);

	// video mode constants
	static const UINT8 MODE_AG      = 0x80;
	static const UINT8 MODE_GM2     = 0x40;
	static const UINT8 MODE_GM1     = 0x20;
	static const UINT8 MODE_GM0     = 0x10;
	static const UINT8 MODE_CSS     = 0x08;
	static const UINT8 MODE_AS      = 0x04;
	static const UINT8 MODE_INTEXT  = 0x02;
	static const UINT8 MODE_INV     = 0x01;

	// timer constants
	static const device_timer_id TIMER_FRAME = 0;
	static const device_timer_id TIMER_HSYNC_OFF = 1;
	static const device_timer_id TIMER_HSYNC_ON = 2;
	static const device_timer_id TIMER_FSYNC = 3;

	// fonts
	static const UINT8 pal_round_fontdata8x12[];
	static const UINT8 pal_square_fontdata8x12[];
	static const UINT8 ntsc_round_fontdata8x12[];
	static const UINT8 ntsc_square_fontdata8x12[];
	static const UINT8 semigraphics4_fontdata8x12[];
	static const UINT8 semigraphics6_fontdata8x12[];
	static const UINT8 s68047_fontdata8x12[];

	// pixel definitions
	typedef UINT32 pixel_t;

	pixel_t *bitmap_addr(bitmap_rgb32 &bitmap, int y, int x)
	{
		return &bitmap.pix32(y, x);
	}

	static UINT8 simplify_mode(UINT8 data, UINT8 mode)
	{
		// simplifies MC6847 modes to drop mode flags that are not significant
		return mode & ~((mode & MODE_AG) ? (MODE_AS | MODE_INV) : 0);
	}

	// internal class that represents a MC6847 character map
	class character_map
	{
	public:
		// constructor that sets up the font data
		character_map(const UINT8 *fontdata, bool is_mc6847t1);

		// optimized template function that emits a single character
		template<int xscale>
		ATTR_FORCE_INLINE void emit_character(UINT8 mode, const UINT8 *data, int length, pixel_t *RESTRICT pixels, int y, const pixel_t *palette)
		{
			for (int i = 0; i < length; i++)
			{
				// get the character
				UINT8 character = data[i];

				// based on the mode, determine which entry to use
				const entry *e = &m_entries[mode % ARRAY_LENGTH(m_entries)];

				// identify the character in the font data
				const UINT8 *font_character = e->m_fontdata + (character & e->m_character_mask) * 12;

				// get the particular slice out
				UINT8 font_character_slice = font_character[y % 12];

				// get the two colors
				UINT16 color_base_0 = e->m_color_base_0 + ((character >> e->m_color_shift_0) & e->m_color_mask_0);
				UINT16 color_base_1 = e->m_color_base_1 + ((character >> e->m_color_shift_1) & e->m_color_mask_1);
				pixel_t color_0 = palette[color_base_0];
				pixel_t color_1 = palette[color_base_1];

				// emit the bits
				for (int j = 0; j < 8; j++)
				{
					for (int k = 0; k < xscale; k++)
					{
						pixels[(i * 8 + j) * xscale + k] = bit_test(font_character_slice, j, color_0, color_1);
					}
				}
			}
		}

	private:
		struct entry
		{
			const UINT8 *m_fontdata;
			UINT8 m_character_mask;
			UINT8 m_color_shift_0;
			UINT8 m_color_shift_1;
			UINT8 m_color_mask_0;
			UINT8 m_color_mask_1;
			UINT16 m_color_base_0;
			UINT16 m_color_base_1;
		};

		// lookup table for MC6847 modes to determine font data and color
		entry m_entries[128];

		// text font data calculated on startup
		UINT8 m_text_fontdata_inverse[64*12];
		UINT8 m_text_fontdata_lower_case[64*12];
		UINT8 m_text_fontdata_lower_case_inverse[64*12];

		// optimized function that tests a single bit
		ATTR_FORCE_INLINE pixel_t bit_test(UINT8 data, int shift, pixel_t color_0, pixel_t color_1)
		{
			return data & (0x80 >> shift) ? color_1 : color_0;
		}
	};

	// artficater internal class
	class artifacter
	{
	public:
		artifacter();

		// artifacting config
		void setup_config(device_t *device);
		void poll_config(void) { m_artifacting = (m_config!=NULL) ? m_config->read() : 0; }

		// artifacting application
		template<int xscale>
		ATTR_FORCE_INLINE void process_artifacts(pixel_t *pixels, UINT8 mode, const pixel_t *palette)
		{
			if (((mode & (MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0)) == (MODE_AG|MODE_GM2|MODE_GM1|MODE_GM0))
				&& (m_artifacting != 0))
			{
				// identify the new colors and update
				pixel_t c0 = palette[(mode & MODE_CSS) ? 10 : 8];
				pixel_t c1 = palette[(mode & MODE_CSS) ? 11 : 9];
				update_colors(c0, c1);

				// generate the new line
				pixel_t new_line[256];
				int x, i;
				for (x = 0; x < 256; x += 2)
				{
					UINT8 val = ((pixels[(x - 2) * xscale] == c1) ? 0x20 : 0x00)
						|   ((pixels[(x - 1) * xscale] == c1) ? 0x10 : 0x00)
						|   ((pixels[(x + 0) * xscale] == c1) ? 0x08 : 0x00)
						|   ((pixels[(x + 1) * xscale] == c1) ? 0x04 : 0x00)
						|   ((pixels[(x + 2) * xscale] == c1) ? 0x02 : 0x00)
						|   ((pixels[(x + 3) * xscale] == c1) ? 0x01 : 0x00);

					new_line[x + 0] = m_expanded_colors[val * 2 + 0];
					new_line[x + 1] = m_expanded_colors[val * 2 + 1];
				}

				// and copy it in
				for (x = 0; x < 256; x++)
				{
					for (i = 0; i < xscale; i++)
						pixels[x * xscale + i] = new_line[x];
				}
			}
		}

	private:
		ioport_port *m_config;
		ioport_value m_artifacting;
		ioport_value m_saved_artifacting;
		pixel_t m_saved_c0, m_saved_c1;
		pixel_t m_expanded_colors[128];

		void update_colors(pixel_t c0, pixel_t c1);
		static pixel_t mix_color(double factor, UINT8 c0, UINT8 c1);
	};

	enum border_color_t
	{
		BORDER_COLOR_BLACK,
		BORDER_COLOR_GREEN,
		BORDER_COLOR_WHITE,
		BORDER_COLOR_ORANGE
	};

	// callbacks
	devcb_write_line   m_write_hsync;
	devcb_write_line   m_write_fsync;

	/* if specified, this reads the external char rom off of the driver state */
	// moved here from mc6847_base_device so to be useable in GIME
	mc6847_get_char_rom_delegate m_charrom_cb;

	// incidentals
	character_map m_character_map;
	artifacter m_artifacter;

	// device-level overrides
	virtual void device_start(void);
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_reset(void);
	virtual void device_post_load(void);

	// other overridables
	virtual void new_frame(void);
	virtual void horizontal_sync_changed(bool line);
	virtual void field_sync_changed(bool line);
	virtual void enter_bottom_border(void);
	virtual void record_border_scanline(UINT16 physical_scanline);
	virtual void record_body_scanline(UINT16 physical_scanline, UINT16 logical_scanline) = 0;
	virtual void record_partial_body_scanline(UINT16 physical_scanline, UINT16 logical_scanline, INT32 start_clock, INT32 end_clock) = 0;

	// miscellaneous
	void video_flush(void);
	const char *describe_context(void);

	// setup functions
	emu_timer *setup_timer(device_timer_id id, double offset, double period);

	// converts to B&W
	static pixel_t black_and_white(rgb_t color)
	{
		UINT8 average_color = (color.r() + color.g() + color.b()) / 3;
		return rgb_t(average_color, average_color, average_color);
	}

	// changes the geometry
	ATTR_FORCE_INLINE void set_geometry(UINT16 top_border_scanlines, UINT16 body_scanlines, bool wide)
	{
		if (UNEXPECTED((m_top_border_scanlines != top_border_scanlines)
			|| (m_body_scanlines != body_scanlines)
			|| (m_wide != wide)))
		{
			m_top_border_scanlines = top_border_scanlines;
			m_body_scanlines = body_scanlines;
			m_wide = wide;
			update_field_sync_timer();
		}
	}

	// calculates the border color
	static ATTR_FORCE_INLINE border_color_t border_value(UINT8 mode, bool is_mc6847t1)
	{
		border_color_t result;

		if (mode & MODE_AG)
		{
			// graphics
			result = mode & MODE_CSS ? BORDER_COLOR_WHITE : BORDER_COLOR_GREEN;
		}
		else if (!is_mc6847t1 || ((mode & MODE_GM2) == 0))
		{
			// text, black border
			result = BORDER_COLOR_BLACK;
		}
		else
		{
			// text, green or orange border
			result = mode & MODE_CSS ? BORDER_COLOR_ORANGE : BORDER_COLOR_GREEN;
		}
		return result;
	}

	// checks to see if the video has changed
	ATTR_FORCE_INLINE bool has_video_changed(void)
	{
		/* poll the artifacting config */
		m_artifacter.poll_config();

		/* if the video didn't change, indicate as much */
		bool video_changed = m_video_changed;
		m_video_changed = false;
		return video_changed;
	}

	// updates a byte in the video state
	template<class T>
	ATTR_FORCE_INLINE bool update_value(T *ptr, T byte)
	{
		bool result = false;
		if (*ptr != byte)
		{
			*ptr = byte;
			m_video_changed = true;
			result = true;
		}
		return result;
	}

	// template function for emitting graphics bytes
	template<int bits_per_pixel, int xscale>
	ATTR_FORCE_INLINE void emit_graphics(const UINT8 *data, int length, pixel_t *RESTRICT pixels, UINT16 color_base, const pixel_t *RESTRICT palette)
	{
		for (int i = 0; i < length; i++)
		{
			for (int j = 0; j < (8 / bits_per_pixel); j++)
			{
				for (int k = 0; k < xscale; k++)
				{
					UINT16 color = color_base + ((data[i] >> (8 - (j + 1) * bits_per_pixel)) & ((1 << bits_per_pixel) - 1));
					pixels[(i * (8 / bits_per_pixel) + j) * xscale + k] = palette[color];
				}
			}
		}
	}

	// template function for external bytes
	template<int bits_per_pixel, int xscale>
	ATTR_FORCE_INLINE void emit_extbytes(const UINT8 *data, int length, pixel_t *RESTRICT pixels, UINT16 color_base, const pixel_t *RESTRICT palette)
	{
		for (int i = 0; i < length; i++)
		{
			for (int j = 0; j < (8 / bits_per_pixel); j++)
			{
				for (int k = 0; k < xscale; k++)
				{
					UINT16 color = color_base + BIT(data[i], 7-j);
					pixels[(i * (8 / bits_per_pixel) + j) * xscale + k] = palette[color];
				}
			}
		}
	}

	// template function for emitting samples
	template<int xscale>
	UINT32 emit_mc6847_samples(UINT8 mode, const UINT8 *data, int length, pixel_t *RESTRICT pixels, const pixel_t *RESTRICT palette,
		mc6847_get_char_rom_delegate get_char_rom, int x, int y)
	{
		UINT32 result = 0;
		if (mode & MODE_AG)
		{
			/* graphics */
			switch(mode & (MODE_GM2|MODE_GM1|MODE_GM0))
			{
				case 0:
					emit_graphics<2, xscale * 4>(data, length, pixels, (mode & MODE_CSS) ? 4 : 0, palette);
					result = length * 8 * xscale * 2;
					break;

				case MODE_GM0:
				case MODE_GM1|MODE_GM0:
				case MODE_GM2|MODE_GM0:
					emit_graphics<1, xscale * 2>(data, length, pixels, (mode & MODE_CSS) ? 10 : 8, palette);
					result = length * 8 * xscale * 2;
					break;

				case MODE_GM1:
				case MODE_GM2:
				case MODE_GM2|MODE_GM1:
					emit_graphics<2, xscale * 2>(data, length, pixels, (mode & MODE_CSS) ? 4 : 0, palette);
					result = length * 8 * xscale;
					break;

				case MODE_GM2|MODE_GM1|MODE_GM0:
					emit_graphics<1, xscale * 1>(data, length, pixels, (mode & MODE_CSS) ? 10 : 8, palette);
					result = length * 8 * xscale;
					break;

				default:
					/* should not get here */
					fatalerror("Should not get here\n");
					break;
			}
		}
		else if (!get_char_rom.isnull() && ((mode & (MODE_AG|MODE_AS|MODE_INTEXT)) == MODE_INTEXT))
		{
			/* external ROM */
			for (int i = 0; i < length; i++)
			{
				UINT8 byte = get_char_rom(data[i], y % 12) ^ (mode & MODE_INV ? 0xFF : 0x00);
				emit_extbytes<1, xscale>(&byte, 1, &pixels[i * 8], (mode & MODE_CSS) ? 14 : 12, palette);
			}
			result = length * 8 * xscale;
		}
		else
		{
			/* text/semigraphics */
			m_character_map.emit_character<xscale>(mode, data, length, pixels, y, palette);
			result = length * 8 * xscale;
		}
		return result;
	}

private:
	enum scanline_zone
	{
		SCANLINE_ZONE_TOP_BORDER,
		SCANLINE_ZONE_BODY,
		SCANLINE_ZONE_BOTTOM_BORDER,
		SCANLINE_ZONE_RETRACE,
		SCANLINE_ZONE_VBLANK,
		SCANLINE_ZONE_FRAME_END
	};

	// timers
	emu_timer *m_frame_timer;
	emu_timer *m_hsync_on_timer;
	emu_timer *m_hsync_off_timer;
	emu_timer *m_fsync_timer;

	// incidentals
	double m_tpfs;
	int m_field_sync_falling_edge_scanline;
	bool m_wide;
	bool m_video_changed;
	UINT16 m_top_border_scanlines;
	UINT16 m_body_scanlines;
	bool m_recording_scanline;
	bool m_supports_partial_body_scanlines;

	// video state
	UINT16 m_physical_scanline;
	UINT16 m_logical_scanline;
	UINT16 m_logical_scanline_zone;
	bool m_horizontal_sync;
	bool m_field_sync;
	UINT32 m_partial_scanline_clocks;

	// functions
	void change_horizontal_sync(bool line);
	void change_field_sync(bool line);
	void update_field_sync_timer(void);
	void next_scanline(void);
	INT32 get_clocks_since_hsync();

	// debugging
	const char *scanline_zone_string(scanline_zone zone);
};

// actual base class for MC6847 family of devices
class mc6847_base_device : public mc6847_friend_device
{
public:
	template<class _Object> static devcb_base &set_input_callback(device_t &device, _Object object) { return downcast<mc6847_base_device &>(device).m_input_cb.set_callback(object); }

	static void set_get_fixed_mode(device_t &device, UINT8 mode) { downcast<mc6847_base_device &>(device).m_fixed_mode = mode; }
	static void set_black_and_white(device_t &device, bool bw) { downcast<mc6847_base_device &>(device).m_black_and_white = bw; }

	/* updates the screen -- this will call begin_update(),
	   followed by update_row() reapeatedly and after all row
	   updating is complete, end_update() */
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// mode changing operations
	DECLARE_WRITE_LINE_MEMBER( ag_w )       { change_mode(MODE_AG, state); }
	DECLARE_WRITE_LINE_MEMBER( gm2_w )      { change_mode(MODE_GM2, state); }
	DECLARE_WRITE_LINE_MEMBER( gm1_w )      { change_mode(MODE_GM1, state); }
	DECLARE_WRITE_LINE_MEMBER( gm0_w )      { change_mode(MODE_GM0, state); }
	DECLARE_WRITE_LINE_MEMBER( as_w )       { change_mode(MODE_AS, state); }
	DECLARE_WRITE_LINE_MEMBER( css_w )      { change_mode(MODE_CSS, state); }
	DECLARE_WRITE_LINE_MEMBER( intext_w )   { change_mode(MODE_INTEXT, state); }
	DECLARE_WRITE_LINE_MEMBER( inv_w )      { change_mode(MODE_INV, state); }

protected:
	mc6847_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const UINT8 *fontdata, double tpfs, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual ioport_constructor device_input_ports() const;

	// other overrides
	virtual void field_sync_changed(bool line);
	virtual void record_body_scanline(UINT16 physical_scanline, UINT16 scanline);
	virtual void record_partial_body_scanline(UINT16 physical_scanline, UINT16 logical_scanline, INT32 start_clock, INT32 end_clock);

	void set_custom_palette(const pixel_t *custom_palette)
	{
		if ( m_palette != m_bw_palette )
		{
			m_palette = custom_palette ? custom_palette : s_palette;
		}
	}

private:
	struct video_scanline
	{
		UINT8 m_sample_count;
		UINT8 m_mode[32];
		UINT8 m_data[32];
	};

	// palette
	static const int PALETTE_LENGTH = 16;
	static const UINT32 s_palette[PALETTE_LENGTH];

	// callbacks

	/* if specified, this gets called whenever reading a byte (offs_t ~0 specifies DA* entering the tristate mode) */
	devcb_read8 m_input_cb;

	/* if true, this is black and white */
	bool m_black_and_white;

	// incidentals
	UINT8 m_fixed_mode;
	UINT8 m_fixed_mode_mask;
	const pixel_t *m_palette;
	pixel_t m_bw_palette[PALETTE_LENGTH];

	// state
	UINT8 m_mode;
	UINT16 m_video_address;
	bool m_dirty;
	video_scanline m_data[192];

	void change_mode(UINT8 mode, int state)
	{
		// sanity check, to ensure that we're not changing fixed modes
		assert((mode & m_fixed_mode_mask) == 0);

		// calculate new mode
		UINT8 new_mode;
		if (state)
			new_mode = m_mode | mode;
		else
			new_mode = m_mode & ~mode;

		// has the mode changed?
		if (new_mode != m_mode)
		{
			// it has!  check dirty flag
			video_flush();
			if (!m_dirty)
			{
				m_dirty = true;
			}

			// and set the new mode
			m_mode = new_mode;
		}
	}

	// setup functions
	void setup_fixed_mode();

	// runtime functions
	void record_body_scanline(UINT16 physical_scanline, UINT16 scanline, INT32 start_pos, INT32 end_pos);
	pixel_t border_value(UINT8 mode, const pixel_t *palette, bool is_mc6847t1);

	template<int xscale>
	void emit_samples(UINT8 mode, const UINT8 *data, int length, pixel_t *pixels, int x, int y);

	// template function for doing video update collection
	template<int sample_count, int yres>
	void record_scanline_res(int scanline, INT32 start_pos, INT32 end_pos);

	// miscellaneous
	UINT8 input(UINT16 address);
	INT32 scanline_position_from_clock(INT32 clocks_since_hsync);
};


//**************************************************************************
//  VARIATIONS
//**************************************************************************

class mc6847_ntsc_device : public mc6847_base_device
{
public:
	mc6847_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mc6847_pal_device : public mc6847_base_device
{
public:
	mc6847_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mc6847y_ntsc_device : public mc6847_base_device
{
public:
	mc6847y_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mc6847y_pal_device : public mc6847_base_device
{
public:
	mc6847y_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mc6847t1_ntsc_device : public mc6847_base_device
{
public:
	mc6847t1_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class mc6847t1_pal_device : public mc6847_base_device
{
public:
	mc6847t1_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class s68047_device : public mc6847_base_device
{
public:
	s68047_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void hack_black_becomes_blue(bool flag);

private:
	static const UINT32 s_s68047_hack_palette[16];
};

class m5c6847p1_device : public mc6847_base_device
{
public:
	m5c6847p1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type MC6847_NTSC;
extern const device_type MC6847_PAL;
extern const device_type MC6847Y_NTSC;
extern const device_type MC6847Y_PAL;
extern const device_type MC6847T1_NTSC;
extern const device_type MC6847T1_PAL;
extern const device_type S68047;
extern const device_type M5C6847P1;

#endif /* __MC6847__ */
