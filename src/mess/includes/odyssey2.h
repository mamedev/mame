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
	UINT8	X;
	UINT8	Y;
	UINT8	Y0;
	UINT8	R;
	UINT8	M;
	UINT8	TA;
	UINT8	TB;
	UINT8	busy;
	UINT8	ram[1024];
};


class odyssey2_state : public driver_device
{
public:
	odyssey2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

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
	emu_timer *m_i824x_line_timer;
	emu_timer *m_i824x_hblank_timer;
	bitmap_ind16 m_tmp_bitmap;
	int m_start_vpos;
	int m_start_vblank;
	UINT8 m_lum;
	sound_stream *m_sh_channel;
	UINT16 m_sh_count;
	//ef9341_t ef9341;
	DECLARE_READ8_MEMBER(odyssey2_t0_r);
	DECLARE_READ8_MEMBER(odyssey2_bus_r);
	DECLARE_WRITE8_MEMBER(odyssey2_bus_w);
	DECLARE_READ8_MEMBER(g7400_bus_r);
	DECLARE_WRITE8_MEMBER(g7400_bus_w);
	DECLARE_READ8_MEMBER(odyssey2_getp1);
	DECLARE_WRITE8_MEMBER(odyssey2_putp1);
	DECLARE_READ8_MEMBER(odyssey2_getp2);
	DECLARE_WRITE8_MEMBER(odyssey2_putp2);
	DECLARE_READ8_MEMBER(odyssey2_getbus);
	DECLARE_WRITE8_MEMBER(odyssey2_putbus);
	DECLARE_READ8_MEMBER(odyssey2_video_r);
	DECLARE_WRITE8_MEMBER(odyssey2_video_w);
	DECLARE_WRITE8_MEMBER(odyssey2_lum_w);
	DECLARE_READ8_MEMBER(odyssey2_t1_r);
	DECLARE_DRIVER_INIT(odyssey2);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/odyssey2.c -----------*/

extern const UINT8 odyssey2_colors[];


SCREEN_UPDATE_IND16( odyssey2 );


STREAM_UPDATE( odyssey2_sh_update );

void odyssey2_ef9341_w( running_machine &machine, int command, int b, UINT8 data );
UINT8 odyssey2_ef9341_r( running_machine &machine, int command, int b );

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


/*----------- defined in machine/odyssey2.c -----------*/



/* i/o ports */




void odyssey2_the_voice_lrq_callback( device_t *device, int state );


int odyssey2_cart_verify(const UINT8 *cartdata, size_t size);


#endif /* ODYSSEY2_H_ */
