// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/*************************************************************************

    Atari Liberator hardware

*************************************************************************/

#include "cpu/m6502/m6502.h"
#include "machine/atarigen.h"
#include "sound/pokey.h"

class liberatr_state : public atarigen_state
{
public:
	liberatr_state(const machine_config &mconfig, device_type type, const char *tag)
		: atarigen_state(mconfig, type, tag),
			m_base_ram(*this, "base_ram"),
			m_planet_frame(*this, "planet_frame"),
			m_planet_select(*this, "planet_select"),
			m_xcoord(*this, "xcoord"),
			m_ycoord(*this, "ycoord"),
			m_bitmapram(*this, "bitmapram"),
			m_colorram(*this, "colorram") { }

	DECLARE_WRITE8_MEMBER( led_w );
	DECLARE_WRITE8_MEMBER( coin_counter_w );

	DECLARE_WRITE8_MEMBER( trackball_reset_w );
	DECLARE_READ8_MEMBER( port0_r );

	DECLARE_WRITE8_MEMBER( bitmap_w );
	DECLARE_READ8_MEMBER( bitmap_xy_r );
	DECLARE_WRITE8_MEMBER( bitmap_xy_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void machine_start() override;
	virtual void video_start() override;

	virtual void update_interrupts() override { }

	struct planet;

	void init_planet(planet &liberatr_planet, UINT8 *planet_rom);
	void get_pens(pen_t *pens);
	void draw_planet(bitmap_rgb32 &bitmap, pen_t *pens);
	void draw_bitmap(bitmap_rgb32 &bitmap, pen_t *pens);

	required_shared_ptr<UINT8> m_base_ram;
	required_shared_ptr<UINT8> m_planet_frame;
	required_shared_ptr<UINT8> m_planet_select;
	required_shared_ptr<UINT8> m_xcoord;
	required_shared_ptr<UINT8> m_ycoord;
	required_shared_ptr<UINT8> m_bitmapram;
	required_shared_ptr<UINT8> m_colorram;

	UINT8       m_trackball_offset;
	UINT8       m_ctrld;
	UINT8       m_videoram[0x10000];

	// The following structure describes the (up to 32) line segments
	// that make up one horizontal line (latitude) for one display frame of the planet.
	// Note: this and the following structure is only used to collect the
	// data before it is packed for actual use.
	struct planet_frame_line
	{
		UINT8 segment_count;    // the number of segments on this line
		UINT8 max_x;            // the maximum value of x_array for this line
		UINT8 color_array[32];  // the color values
		UINT8 x_array[32];      // and maximum x values for each segment
	};

	// The following structure describes the lines (latitudes)
	// that make up one complete display frame of the planet.
	// Note: this and the previous structure is only used to collect the
	// data before it is packed for actual use.
	struct planet_frame
	{
		planet_frame_line lines[0x80];
	};

	// The following structure collects the 256 frames of the
	// planet (one per value of longitude).
	// The data is packed segment_count,segment_start,color,length,color,length,...  then
	//                    segment_count,segment_start,color,length,color,length...  for the next line, etc
	// for the 128 lines.
	struct planet
	{
		UINT8 *frames[256];
	};

	// The following array collects the 2 different planet
	// descriptions, which are selected by planetbit
	planet m_planets[2];
};
