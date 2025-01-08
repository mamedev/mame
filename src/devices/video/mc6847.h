// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    mc6847.h

    Implementation of Motorola 6847 video hardware chip

***************************************************************************/
#ifndef MAME_VIDEO_MC6847_H
#define MAME_VIDEO_MC6847_H

#pragma once

#include "screen.h"
#include <map>

//**************************************************************************
//  MC6847 CONFIGURATION / INTERFACE
//**************************************************************************

#define MC6847_GET_CHARROM_MEMBER(_name)   uint8_t _name(uint8_t ch, int line)


#define ARTIFACTING_TAG     "artifacting"

INPUT_PORTS_EXTERN(mc6847_artifacting);


//**************************************************************************
//  MC6847 CORE
//**************************************************************************

// base class so that the GIME emulation can access mc6847 stuff
class mc6847_friend_device : public device_t, public device_video_interface
{
public:
	// video mode constants
	static constexpr uint8_t MODE_AG      = 0x80;
	static constexpr uint8_t MODE_GM2     = 0x40;
	static constexpr uint8_t MODE_GM1     = 0x20;
	static constexpr uint8_t MODE_GM0     = 0x10;
	static constexpr uint8_t MODE_CSS     = 0x08;
	static constexpr uint8_t MODE_AS      = 0x04;
	static constexpr uint8_t MODE_INTEXT  = 0x02;
	static constexpr uint8_t MODE_INV     = 0x01;

	typedef device_delegate<uint8_t (uint8_t ch, int line)> get_char_rom_delegate;

	// inlines
	bool hs_r() const { return m_horizontal_sync; }
	bool fs_r() const { return m_field_sync; }

	auto hsync_wr_callback() { return m_write_hsync.bind(); }
	auto fsync_wr_callback() { return m_write_fsync.bind(); }

	template <typename... T> void set_get_char_rom(T &&... args) { m_charrom_cb.set(std::forward<T>(args)...); }

protected:
	mc6847_friend_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock,
			const uint8_t *fontdata, bool is_mc6847t1, double tpfs, int field_sync_falling_edge_scanline, int divider,
			bool supports_partial_body_scanlines, bool pal);

	// fonts
	static const uint8_t vdg_t1_fontdata8x12[];
	static const uint8_t vdg_fontdata8x12[];
	static const uint8_t semigraphics4_fontdata8x12[];
	static const uint8_t semigraphics6_fontdata8x12[];
	static const uint8_t s68047_fontdata8x12[];
	static const uint8_t stripes[];

	// pixel definitions
	typedef uint32_t pixel_t;

	pixel_t *bitmap_addr(bitmap_rgb32 &bitmap, int y, int x)
	{
		return &bitmap.pix(y, x);
	}

	static uint8_t simplify_mode(uint8_t data, uint8_t mode)
	{
		// simplifies MC6847 modes to drop mode flags that are not significant
		return mode & ~((mode & MODE_AG) ? (MODE_AS | MODE_INV) : 0);
	}

	// internal class that represents a MC6847 character map
	class character_map
	{
	public:
		// constructor that sets up the font data
		character_map(const uint8_t *fontdata, bool is_mc6847t1);

		// optimized template function that emits a single character
		template<int xscale>
		ATTR_FORCE_INLINE void emit_character(uint8_t mode, const uint8_t *data, int length, pixel_t *RESTRICT pixels, int y, const pixel_t *palette)
		{
			for (int i = 0; i < length; i++)
			{
				// get the character
				uint8_t character = data[i];

				// based on the mode, determine which entry to use
				const entry *e = &m_entries[mode % std::size(m_entries)];

				// identify the character in the font data
				const uint8_t *font_character = e->m_fontdata + (character & e->m_character_mask) * 12;

				// get the particular slice out
				uint8_t font_character_slice = font_character[y % 12];

				// get the two colors
				uint16_t color_base_0 = e->m_color_base_0 + ((character >> e->m_color_shift_0) & e->m_color_mask_0);
				uint16_t color_base_1 = e->m_color_base_1 + ((character >> e->m_color_shift_1) & e->m_color_mask_1);
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
			const uint8_t *m_fontdata;
			uint8_t m_character_mask;
			uint8_t m_color_shift_0;
			uint8_t m_color_shift_1;
			uint8_t m_color_mask_0;
			uint8_t m_color_mask_1;
			uint16_t m_color_base_0;
			uint16_t m_color_base_1;
		};

		// lookup table for MC6847 modes to determine font data and color
		entry m_entries[128];

		// text font data calculated on startup
		uint8_t m_text_fontdata_inverse[64*12];
		uint8_t m_text_fontdata_lower_case[64*12];
		uint8_t m_text_fontdata_lower_case_inverse[64*12];
		uint8_t m_stripes[128*12];

		// optimized function that tests a single bit
		ATTR_FORCE_INLINE pixel_t bit_test(uint8_t data, int shift, pixel_t color_0, pixel_t color_1)
		{
			return data & (0x80 >> shift) ? color_1 : color_0;
		}
	};

	// artificater internal class
	class artifacter
	{
	public:
		artifacter();

		// artifacting config
		void setup_config(device_t *device);
		bool poll_config();
		void set_pal_artifacting( bool palartifacting ) { m_palartifacting = palartifacting; }
		bool get_pal_artifacting() { return m_palartifacting; }
		void create_color_blend_table( const pixel_t *palette );

		// artifacting application
		template<int xscale>
		void process_artifacts_pal(bitmap_rgb32 &bitmap, int y, int base_x, int base_y, uint8_t mode, const pixel_t *palette)
		{
			if( !m_artifacting || !m_palartifacting )
				return;

			if( (mode & MODE_AS) || ((mode & (MODE_AG|MODE_GM0) ) == MODE_AG) )
			{
				pixel_t *line1 = &bitmap.pix(y + base_y, base_x);
				pixel_t *line2 = &bitmap.pix(y + base_y + 1, base_x);
				std::map<std::pair<pixel_t,pixel_t>,pixel_t>::const_iterator newColor;

				for( int pixel = 0; pixel < bitmap.width() - (base_x * 2); ++pixel )
				{
					if( line1[pixel] == line2[pixel] )
						continue;

					newColor = m_palcolorblendmap.find(std::pair<pixel_t,pixel_t>(line1[pixel],line2[pixel]));
					if( newColor != m_palcolorblendmap.end() )
					{
						line1[pixel] = newColor->second;
						line2[pixel] = newColor->second;
					}
				}
			}
		}

		template<int xscale>
		ATTR_FORCE_INLINE void process_artifacts(pixel_t *pixels, uint8_t mode, const pixel_t *palette)
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
					uint8_t val = ((pixels[(x - 2) * xscale] == c1) ? 0x20 : 0x00)
							| ((pixels[(x - 1) * xscale] == c1) ? 0x10 : 0x00)
							| ((pixels[(x + 0) * xscale] == c1) ? 0x08 : 0x00)
							| ((pixels[(x + 1) * xscale] == c1) ? 0x04 : 0x00)
							| ((pixels[(x + 2) * xscale] == c1) ? 0x02 : 0x00)
							| ((pixels[(x + 3) * xscale] == c1) ? 0x01 : 0x00);

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
		bool m_palartifacting;
		ioport_port *m_config;
		ioport_value m_artifacting;
		ioport_value m_saved_artifacting;
		pixel_t m_saved_c0, m_saved_c1;
		pixel_t m_expanded_colors[128];

		// PAL color blend emulation values.
		std::map<std::pair<pixel_t,pixel_t>,pixel_t> m_palcolorblendmap;

		void update_colors(pixel_t c0, pixel_t c1);
		static pixel_t mix_color(double factor, uint8_t c0, uint8_t c1);
	};

	// callbacks
	devcb_write_line   m_write_hsync;
	devcb_write_line   m_write_fsync;

	/* if specified, this reads the external char rom off of the driver state */
	// moved here from mc6847_base_device so to be useable in GIME
	get_char_rom_delegate m_charrom_cb;

	// incidentals
	character_map m_character_map;
	artifacter m_artifacter;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// other overridables
	virtual void new_frame();
	virtual TIMER_CALLBACK_MEMBER(horizontal_sync_changed);
	virtual void field_sync_changed(bool line);
	virtual void enter_bottom_border();
	virtual void record_border_scanline(uint16_t physical_scanline);
	virtual void record_full_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline) = 0;
	virtual void record_partial_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline, int32_t start_clock, int32_t end_clock) = 0;

	// miscellaneous
	void video_flush();
	std::string describe_context() const;

	// converts to B&W
	static pixel_t black_and_white(rgb_t color)
	{
		uint8_t average_color = (color.r() + color.g() + color.b()) / 3;
		return rgb_t(average_color, average_color, average_color);
	}

	// changes the geometry
	ATTR_FORCE_INLINE void set_geometry(uint16_t top_border_scanlines, uint16_t body_scanlines, bool wide)
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

	// checks to see if the video has changed
	ATTR_FORCE_INLINE bool has_video_changed()
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
	ATTR_FORCE_INLINE void emit_graphics(const uint8_t *data, int length, pixel_t *RESTRICT pixels, uint16_t color_base, const pixel_t *RESTRICT palette)
	{
		for (int i = 0; i < length; i++)
		{
			for (int j = 0; j < (8 / bits_per_pixel); j++)
			{
				for (int k = 0; k < xscale; k++)
				{
					uint16_t color = color_base + ((data[i] >> (8 - (j + 1) * bits_per_pixel)) & ((1 << bits_per_pixel) - 1));
					pixels[(i * (8 / bits_per_pixel) + j) * xscale + k] = palette[color];
				}
			}
		}
	}

	// template function for external bytes
	template<int bits_per_pixel, int xscale>
	ATTR_FORCE_INLINE void emit_extbytes(const uint8_t *data, int length, pixel_t *RESTRICT pixels, uint16_t color_base, const pixel_t *RESTRICT palette)
	{
		for (int i = 0; i < length; i++)
		{
			for (int j = 0; j < (8 / bits_per_pixel); j++)
			{
				for (int k = 0; k < xscale; k++)
				{
					uint16_t color = color_base + BIT(data[i], 7-j);
					pixels[(i * (8 / bits_per_pixel) + j) * xscale + k] = palette[color];
				}
			}
		}
	}

	// template function for emitting samples
	template<int xscale>
	uint32_t emit_mc6847_samples(uint8_t mode, const uint8_t *data, int length, pixel_t *RESTRICT pixels, const pixel_t *RESTRICT palette,
			get_char_rom_delegate const &get_char_rom, int x, int y)
	{
		uint32_t result;
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
		else if (!m_charrom_cb.isnull() && ((mode & (MODE_AG|MODE_AS|MODE_INTEXT)) == MODE_INTEXT))
		{
			/* external ROM */
			for (int i = 0; i < length; i++)
			{
				uint8_t byte = m_charrom_cb(data[i], y % 12) ^ (mode & MODE_INV ? 0xFF : 0x00);
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
	};

	// timers
	emu_timer *m_hsync_on_timer;
	emu_timer *m_hsync_off_timer;
	emu_timer *m_fsync_timer;

protected:
	const double m_tpfs;

private:
	// incidentals
	const int m_divider;
	const int m_field_sync_falling_edge_scanline;
	bool m_wide;
	bool m_video_changed;
	uint16_t m_top_border_scanlines;
	uint16_t m_body_scanlines;
	bool m_recording_scanline;
	const bool m_supports_partial_body_scanlines;

protected:
	const bool m_pal;
	const uint16_t m_lines_top_border;
	const uint16_t m_lines_until_vblank;
	const uint16_t m_lines_until_retrace;

private:
	// video state
	uint16_t m_physical_scanline;
	uint16_t m_logical_scanline;
	uint16_t m_logical_scanline_zone;
	bool m_horizontal_sync;
	bool m_field_sync;
	uint32_t m_partial_scanline_clocks;

	// functions
	virtual TIMER_CALLBACK_MEMBER(change_horizontal_sync);
	TIMER_CALLBACK_MEMBER(change_field_sync);
	void update_field_sync_timer();
	void next_scanline();
	int32_t get_clocks_since_hsync();

	// debugging
	std::string scanline_zone_string(scanline_zone zone) const;

protected:
	bool is_top_pal_padding_line(int scanline) const;
	bool is_bottom_pal_padding_line(int scanline) const;
	bool is_pal_padding_line(int scanline) const;
};

// actual base class for MC6847 family of devices
class mc6847_base_device : public mc6847_friend_device
{
public:
	auto input_callback() { return m_input_cb.bind(); }

	void set_get_fixed_mode(uint8_t mode) { m_fixed_mode = mode; }
	void set_black_and_white(bool bw) { m_black_and_white = bw; }

	/* updates the screen -- this will call begin_update(),
	   followed by update_row() repeatedly and after all row
	   updating is complete, end_update() */
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// mode changing operations
	void ag_w(int state)       { change_mode(MODE_AG, state); }
	void gm2_w(int state)      { change_mode(MODE_GM2, state); }
	void gm1_w(int state)      { change_mode(MODE_GM1, state); }
	void gm0_w(int state)      { change_mode(MODE_GM0, state); }
	void as_w(int state)       { change_mode(MODE_AS, state); }
	void css_w(int state)      { change_mode(MODE_CSS, state); }
	void intext_w(int state)   { change_mode(MODE_INTEXT, state); }
	void inv_w(int state)      { change_mode(MODE_INV, state); }

	// palette
	void set_palette(const uint32_t *palette) { m_palette = (palette) ? palette : default_palette(); }

protected:
	mc6847_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint8_t *fontdata, double tpfs, bool pal);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// other overrides
	virtual void field_sync_changed(bool line) override;
	virtual void record_full_body_scanline(uint16_t physical_scanline, uint16_t scanline) override;
	virtual void record_partial_body_scanline(uint16_t physical_scanline, uint16_t logical_scanline, int32_t start_clock, int32_t end_clock) override;

	virtual uint32_t emit_samples(uint8_t mode, const uint8_t *data, int length, pixel_t *RESTRICT pixels, const pixel_t *RESTRICT palette,
			get_char_rom_delegate const &get_char_rom, int x, int y)
	{
		return emit_mc6847_samples<1>(mode, data, length, pixels, palette, get_char_rom, x, y);
	}
	virtual const uint32_t* default_palette() { return s_palette; }

	// runtime functions
	virtual void record_body_scanline(uint8_t mode, uint16_t physical_scanline, uint16_t scanline, int32_t start_pos, int32_t end_pos);
	virtual uint8_t border_value(uint8_t mode);

	// template function for doing video update collection
	template<int sample_count, int yres>
	void record_scanline_res(int scanline, int32_t start_pos, int32_t end_pos);

private:
	struct video_scanline
	{
		uint8_t m_sample_count;
		uint8_t m_mode[32];
		uint8_t m_data[32];
	};

	// palette
	static const int PALETTE_LENGTH = 16;
	static const uint32_t s_palette[PALETTE_LENGTH];

	/* if specified, this gets called whenever reading a byte (offs_t ~0 specifies DA* entering the tristate mode) */
	devcb_read8 m_input_cb;

	/* if true, this is black and white */
	bool m_black_and_white;

	// incidentals
	uint8_t m_fixed_mode;
	uint8_t m_fixed_mode_mask;
	const pixel_t *m_palette;
	pixel_t m_bw_palette[PALETTE_LENGTH];

	// state
	uint8_t m_mode;
	uint16_t m_video_address;
	bool m_dirty;
	video_scanline m_data[192];

	void change_mode(uint8_t mode, int state)
	{
		// sanity check, to ensure that we're not changing fixed modes
		assert((mode & m_fixed_mode_mask) == 0);

		// calculate new mode
		uint8_t new_mode;
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

	// miscellaneous
	uint8_t input(uint16_t address);
	int32_t scanline_position_from_clock(int32_t clocks_since_hsync);
};


//**************************************************************************
//  VARIATIONS
//**************************************************************************

class mc6847_ntsc_device : public mc6847_base_device
{
public:
	mc6847_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mc6847_pal_device : public mc6847_base_device
{
public:
	mc6847_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mc6847y_ntsc_device : public mc6847_base_device
{
public:
	mc6847y_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mc6847y_pal_device : public mc6847_base_device
{
public:
	mc6847y_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class mc6847t1_ntsc_device : public mc6847_base_device
{
public:
	mc6847t1_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	mc6847t1_ntsc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint8_t *fontdata, double tpfs, bool pal);

	virtual uint8_t border_value(uint8_t mode) override;
};

class mc6847t1_pal_device : public mc6847t1_ntsc_device
{
public:
	mc6847t1_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class s68047_device : public mc6847_base_device
{
public:
	s68047_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint32_t emit_samples(uint8_t mode, const uint8_t *data, int length, pixel_t *RESTRICT pixels, const pixel_t *RESTRICT palette,
			get_char_rom_delegate const &get_char_rom, int x, int y) override;
	virtual const uint32_t* default_palette() override { return s_s68047_palette; }

	virtual void record_body_scanline(uint8_t mode, uint16_t physical_scanline, uint16_t scanline, int32_t start_pos, int32_t end_pos) override;
	virtual uint8_t border_value(uint8_t mode) override;

private:
	static const uint32_t s_s68047_palette[16];
};

class m5c6847p1_device : public mc6847_base_device
{
public:
	m5c6847p1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


DECLARE_DEVICE_TYPE(MC6847_NTSC,   mc6847_ntsc_device)
DECLARE_DEVICE_TYPE(MC6847_PAL,    mc6847_pal_device)
DECLARE_DEVICE_TYPE(MC6847Y_NTSC,  mc6847y_ntsc_device)
DECLARE_DEVICE_TYPE(MC6847Y_PAL,   mc6847y_pal_device)
DECLARE_DEVICE_TYPE(MC6847T1_NTSC, mc6847t1_ntsc_device)
DECLARE_DEVICE_TYPE(MC6847T1_PAL,  mc6847t1_pal_device)
DECLARE_DEVICE_TYPE(S68047,        s68047_device)
DECLARE_DEVICE_TYPE(M5C6847P1,     m5c6847p1_device)

#endif // MAME_VIDEO_MC6847_H
