/*****************************************************************************
 *
 * includes/odyssey2.h
 *
 ****************************************************************************/

#ifndef ODYSSEY2_H_
#define ODYSSEY2_H_

#define P1_BANK_LO_BIT            (0x01)
#define P1_BANK_HI_BIT            (0x02)
#define P1_KEYBOARD_SCAN_ENABLE   (0x04)  /* active low */
#define P1_VDC_ENABLE             (0x08)  /* active low */
#define P1_EXT_RAM_ENABLE         (0x10)  /* active low */
#define P1_VDC_COPY_MODE_ENABLE   (0x40)

#define P2_KEYBOARD_SELECT_MASK   (0x07)  /* select row to scan */

#define VDC_CONTROL_REG_STROBE_XY (0x02)

#define I824X_START_ACTIVE_SCAN			6
#define I824X_END_ACTIVE_SCAN			(6 + 160)
#define I824X_START_Y					1
#define I824X_SCREEN_HEIGHT				243
#define I824X_LINE_CLOCKS				228

union o2_vdc_t {
    UINT8 reg[0x100];
    struct {
	struct {
	    UINT8 y,x,color,res;
	} sprites[4];
	struct {
	    UINT8 y,x,ptr,color;
	} foreground[12];
	struct {
	    struct {
		UINT8 y,x,ptr,color;
	    } single[4];
	} quad[4];
	UINT8 shape[4][8];
	UINT8 control;
	UINT8 status;
	UINT8 collision;
	UINT8 color;
	UINT8 y;
	UINT8 x;
	UINT8 res;
	UINT8 shift1,shift2,shift3;
	UINT8 sound;
	UINT8 res2[5+0x10];
	UINT8 hgrid[2][0x10];
	UINT8 vgrid[0x10];
    } s;
};

struct ef9341_t
{
	UINT8	TA;
	UINT8	TB;
	UINT8	busy;
};

struct ef9340_t
{
	UINT8	X;
	UINT8	Y;
	UINT8	Y0;
	UINT8	R;
	UINT8	M;
};

class odyssey2_state : public driver_device
{
public:
	odyssey2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_g7400(false)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	int m_the_voice_lrq_state;
	UINT8 *m_ram;
	UINT8 m_p1;
	UINT8 m_p2;
	size_t m_cart_size;
	o2_vdc_t m_o2_vdc;
	UINT32 m_o2_snd_shift[2];
	UINT8 m_x_beam_pos;
	UINT8 m_y_beam_pos;
	UINT8 m_control_status;
	UINT8 m_collision_status;
	int m_iff;
	bitmap_ind16 m_tmp_bitmap;
	int m_start_vpos;
	int m_start_vblank;
	UINT8 m_lum;
	sound_stream *m_sh_channel;
	UINT16 m_sh_count;
	DECLARE_READ8_MEMBER(t0_read);
	DECLARE_READ8_MEMBER(io_read);
	DECLARE_WRITE8_MEMBER(io_write);
	DECLARE_READ8_MEMBER(bus_read);
	DECLARE_WRITE8_MEMBER(bus_write);
	DECLARE_READ8_MEMBER(g7400_io_read);
	DECLARE_WRITE8_MEMBER(g7400_io_write);
	DECLARE_READ8_MEMBER(p1_read);
	DECLARE_WRITE8_MEMBER(p1_write);
	DECLARE_READ8_MEMBER(p2_read);
	DECLARE_WRITE8_MEMBER(p2_write);
	DECLARE_READ8_MEMBER(video_read);
	DECLARE_WRITE8_MEMBER(video_write);
	DECLARE_WRITE8_MEMBER(lum_write);
	DECLARE_READ8_MEMBER(t1_read);
	DECLARE_DRIVER_INIT(odyssey2);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	void video_start_g7400();
	virtual void palette_init();
	UINT32 screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(the_voice_lrq_callback);

	void ef9341_w( UINT8 command, UINT8 b, UINT8 data );
	UINT8 ef9341_r( UINT8 command, UINT8 b );

protected:
	ef9340_t m_ef9340;
	ef9341_t m_ef9341;
	UINT8	m_ef934x_ram_a[1024];
	UINT8	m_ef934x_ram_b[1024];
	UINT8	m_ef934x_ext_char_ram[1024];
	bool	m_g7400;

	inline UINT16 ef9340_get_c_addr(UINT8 x, UINT8 y);
	inline void ef9340_inc_c();
	// Calculate the external chargen address for a character and slice
	inline UINT16 external_chargen_address(UINT8 b, UINT8 slice);

	void i824x_scanline(int vpos);
	void ef9340_scanline(int vpos);

	/* timers */
	static const device_timer_id TIMER_LINE = 0;
	static const device_timer_id TIMER_HBLANK = 1;

	emu_timer *m_line_timer;
	emu_timer *m_hblank_timer;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void switch_banks();
};


/*----------- defined in video/odyssey2.c -----------*/


STREAM_UPDATE( odyssey2_sh_update );

class odyssey2_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	odyssey2_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
};

extern const device_type ODYSSEY2;


#endif /* ODYSSEY2_H_ */
