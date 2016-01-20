// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Signetics 2636 Programmable Video Interface

**********************************************************************/

#ifndef __S2636_H__
#define __S2636_H__


#define S2636_IS_PIXEL_DRAWN(p)     (((p) & 0x08) ? TRUE : FALSE)
#define S2636_PIXEL_COLOR(p)        ((p) & 0x07)


/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

#define MCFG_S2636_OFFSETS(_yoffs, _xoffs) \
	s2636_device::set_offsets(*device, _yoffs, _xoffs);

#define MCFG_S2636_DIVIDER(_divider) \
	s2636_device::set_divider(*device, _divider);

#define MCFG_S2623_SET_INTREQ_CALLBACK(_devcb) \
	devcb = &s2636_device::set_intreq_cb(*device, DEVCB_##_devcb);


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
	s2636_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~s2636_device() {}

	static void set_offsets(device_t &device, int y_offset, int x_offset)
	{
		s2636_device &dev = downcast<s2636_device &>(device);
		dev.m_x_offset = x_offset;
		dev.m_y_offset = y_offset;
	}

	static void set_divider(device_t &device, int divider)
	{
		s2636_device &dev = downcast<s2636_device &>(device);
		dev.m_divider = divider;
	}

	template<class _Object> static devcb_base &set_intreq_cb(device_t &device, _Object object)
	{
		s2636_device &dev = downcast<s2636_device &>(device);
		return dev.m_intreq_cb.set_callback(object);
	}

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

	DECLARE_READ8_MEMBER( read_data );
	DECLARE_WRITE8_MEMBER( write_data );

	DECLARE_WRITE_LINE_MEMBER( write_intack );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

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

	static UINT16 const SCORE_FONT[16][5];

	static int const SCORE_START_X[2][SCORE_DIGITS];
	static int const SCORE_START_Y[2];

	static void mask_offset(offs_t &offset) { offset &= ((offset & 0x0c0) == 0x0c0) ? 0x0cf : 0x0ff; }

	UINT8 object_scale(int obj) const { return (m_registers[REG_OBJ_SIZE] >> (2 * obj)) & 0x03; }
	UINT8 object_color(int obj) const { return (m_registers[REG_OBJ_CLR_1_2 + (obj >> 1)] >> ((obj & 1) ? 0 : 3)) & 0x07; }
	UINT8 score_digit(int digit) const { return (m_registers[REG_SCORE_1_2 + (digit >> 1)] >> ((digit & 1) ? 0 : 4)) & 0x0f; }

	void update_intreq(int value);

	// Configuration
	int     m_divider;
	int     m_y_offset;
	int     m_x_offset;

	// interfacing with other devices
	devcb_write_line    m_intreq_cb;
	bitmap_ind16        m_bitmap;

	// 256-byte register file (not all of this really exists)
	UINT8   m_registers[0x100];

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

extern const device_type S2636;

#endif /* __S2636_H__ */
