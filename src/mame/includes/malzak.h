/*************************************************************************

    Malzak

*************************************************************************/

#include "video/saa5050.h"

class malzak_state : public driver_device
{
public:
	malzak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_trom(*this, "saa5050"),
			m_videoram(*this, "videoram")
	{ }

	required_device<saa5050_device> m_trom;
	required_shared_ptr<UINT8> m_videoram;

	/* misc */
//  int playfield_x[256];
//  int playfield_y[256];
	int m_playfield_code[256];
	int m_malzak_x;
	int m_malzak_y;
	int m_collision_counter;

	/* devices */
	device_t *m_s2636_0;
	device_t *m_s2636_1;
	device_t *m_saa5050;
	DECLARE_READ8_MEMBER(fake_VRLE_r);
	DECLARE_READ8_MEMBER(s2636_portA_r);
	DECLARE_READ8_MEMBER(s2650_data_r);
	DECLARE_WRITE8_MEMBER(port40_w);
	DECLARE_WRITE8_MEMBER(port60_w);
	DECLARE_WRITE8_MEMBER(portc0_w);
	DECLARE_READ8_MEMBER(collision_r);
	DECLARE_WRITE8_MEMBER(malzak_playfield_w);
	DECLARE_READ8_MEMBER(videoram_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_malzak(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
