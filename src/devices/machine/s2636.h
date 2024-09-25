// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Signetics 2636 Programmable Video Interface

**********************************************************************/

#ifndef MAME_MACHINE_S2636_H
#define MAME_MACHINE_S2636_H

#pragma once


#define S2636_IS_PIXEL_DRAWN(p)     (((p) & 0x08) ? true : false)
#define S2636_PIXEL_COLOR(p)        ((p) & 0x07)


/*************************************
 *
 *  Device state class
 *
 *************************************/

class s2636_device : public device_t,
				public device_video_interface,
				public device_sound_interface
{
public:
	s2636_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~s2636_device();

	void set_offsets(int y_offset, int x_offset) { m_x_offset = x_offset; m_y_offset = y_offset; }

	void set_divider(int divider) { m_divider = divider; }

	auto intreq_cb() { return m_intreq_cb.bind(); }

	// returns a BITMAP_FORMAT_IND16 bitmap the size of the screen
	// D0-D2 of each pixel is the pixel color
	// D3 indicates how the S2636 drew this pixel - 0 = background, 1 = object/score
	bitmap_ind16 const &bitmap() const { return m_bitmap; }

	// this function is for backwards compatibility and will eventually be removed
	// use the functions below for per-scanline drawing/collisions
	bitmap_ind16 const &update(const rectangle &cliprect);

	// call render_first_line to render the first line of the display and render_next_line for each additional line
	void render_first_line();
	void render_next_line();

	uint8_t read_data(offs_t offset);
	void write_data(offs_t offset, uint8_t data);

	void write_intack(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	enum
	{
		OBJ_COUNT       = 4,

		OBJ_WIDTH       = 8,
		OBJ_HEIGHT      = 10,

		BG_START_X      = 32,
		BG_START_Y      = 20,
		BG_WIDTH        = 8 * 16,
		BG_HEIGHT       = 200,

		SCORE_DIGITS    = 4,

		SCORE_WIDTH     = 12,
		SCORE_HEIGHT    = 20,

		OFFS_HC         = 0x00a,
		OFFS_HCB        = 0x00b,
		OFFS_VC         = 0x00c,
		OFFS_VCB        = 0x00d,

		OFFS_VBAR_DEF   = 0x080,
		OFFS_HBAR_DEF   = 0x0a8,

		REG_OBJ_SIZE    = 0x0c0,
		REG_OBJ_CLR_1_2 = 0x0c1,
		REG_OBJ_CLR_3_4 = 0x0c2,
		REG_SCORE_FMT   = 0x0c3,
		REG_BG_ENB_CLR  = 0x0c6,
		REG_SND_PERIOD  = 0x0c7,
		REG_SCORE_1_2   = 0x0c8,
		REG_SCORE_3_4   = 0x0c9,
		REG_COL_BG_CMPL = 0x0ca,
		REG_VBL_COL_OBJ = 0x0cb,
		REG_AD_POT1     = 0x0cc,
		REG_AD_POT2     = 0x0cd
	};

	static int const OFFS_OBJ[OBJ_COUNT];

	static uint16_t const SCORE_FONT[16][5];

	static int const SCORE_START_X[2][SCORE_DIGITS];
	static int const SCORE_START_Y[2];

	static void mask_offset(offs_t &offset) { offset &= ((offset & 0x0c0) == 0x0c0) ? 0x0cf : 0x0ff; }

	uint8_t object_scale(int obj) const { return (m_registers[REG_OBJ_SIZE] >> (2 * obj)) & 0x03; }
	uint8_t object_color(int obj) const { return (m_registers[REG_OBJ_CLR_1_2 + (obj >> 1)] >> ((obj & 1) ? 0 : 3)) & 0x07; }
	uint8_t score_digit(int digit) const { return (m_registers[REG_SCORE_1_2 + (digit >> 1)] >> ((digit & 1) ? 0 : 4)) & 0x0f; }

	void update_intreq(int value);

	// Configuration
	int     m_divider;
	int     m_y_offset;
	int     m_x_offset;

	// interfacing with other devices
	devcb_write_line    m_intreq_cb;
	bitmap_ind16        m_bitmap;

	// 256-byte register file (not all of this really exists)
	uint8_t   m_registers[0x100];

	// tracking where we're up to in the screen update
	bool    m_vrst;
	int     m_screen_line;
	int     m_vis_line;

	// current display state of object
	int     m_obj_cnt[OBJ_COUNT];
	bool    m_obj_disp[OBJ_COUNT];
	bool    m_obj_dup[OBJ_COUNT];

	// interrupt generation
	int     m_intreq;
	int     m_intack;

	// sound generation state
	sound_stream    *m_stream;
	int             m_sample_cnt;
	bool            m_sound_lvl;
};

DECLARE_DEVICE_TYPE(S2636, s2636_device)

#endif // MAME_MACHINE_S2636_H
