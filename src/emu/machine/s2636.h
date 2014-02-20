/**********************************************************************

    Signetics 2636 video chip

**********************************************************************/

#ifndef __S2636_H__
#define __S2636_H__



#define S2636_IS_PIXEL_DRAWN(p)     (((p) & 0x08) ? TRUE : FALSE)
#define S2636_PIXEL_COLOR(p)        ((p) & 0x07)

/*************************************
 *
 *  Type definitions
 *
 *************************************/

struct s2636_interface
{
	int        m_work_ram_size;
	int        m_y_offset;
	int        m_x_offset;
};

/*************************************
 *
 *  Device configuration macros
 *
 *************************************/

class s2636_device : public device_t,
				public device_video_interface,
				public device_sound_interface,
				public s2636_interface
{
public:
	s2636_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~s2636_device() {}

	/* returns a BITMAP_FORMAT_IND16 bitmap the size of the screen
	   D0-D2 of each pixel is the pixel color
	   D3 indicates whether the S2636 drew this pixel - 0 = not drawn, 1 = drawn */

	bitmap_ind16 &update( const rectangle &cliprect );
	DECLARE_WRITE8_MEMBER( work_ram_w );
	DECLARE_READ8_MEMBER( work_ram_r );

	void soundport_w (int mode, int data);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state
	dynamic_buffer m_work_ram;
	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_collision_bitmap;

	sound_stream *m_channel;
	UINT8 m_reg[1];
	int m_size;
	int m_pos;
	unsigned m_level;

	int check_collision( int spriteno1, int spriteno2, const rectangle &cliprect );
};

extern const device_type S2636;


#define MCFG_S2636_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, S2636, 0) \
	MCFG_DEVICE_CONFIG(_interface)


#endif /* __S2636_H__ */
