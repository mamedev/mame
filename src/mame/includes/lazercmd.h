#include "sound/dac.h"

#define HORZ_RES		32
#define VERT_RES		24
#define HORZ_CHR        8
#define VERT_CHR		10
#define VERT_FNT		8

#define HORZ_BAR        0x40
#define VERT_BAR        0x80

#define MARKER_ACTIVE_L 0x03
#define MARKER_ACTIVE_R 0x04
#define MARKER_VERT_R   0x0a
#define MARKER_HORZ_L   0x0b
#define MARKER_VERT_L   0x0c
#define MARKER_HORZ_R   0x0d

#define MARKER_HORZ_ADJ -1
#define MARKER_VERT_ADJ -10

class lazercmd_state : public driver_device
{
public:
	lazercmd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram"){ }

	/* device */
	required_device<cpu_device> m_maincpu;
	dac_device *m_dac;
	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	int      m_marker_x;
	int      m_marker_y;

	/* misc */
	int      m_timer_count;
	int      m_sense_state;
	int      m_dac_data;

	DECLARE_WRITE8_MEMBER(lazercmd_ctrl_port_w);
	DECLARE_READ8_MEMBER(lazercmd_ctrl_port_r);
	DECLARE_WRITE8_MEMBER(lazercmd_data_port_w);
	DECLARE_READ8_MEMBER(lazercmd_data_port_r);
	DECLARE_WRITE8_MEMBER(lazercmd_hardware_w);
	DECLARE_WRITE8_MEMBER(medlanes_hardware_w);
	DECLARE_WRITE8_MEMBER(bbonk_hardware_w);
	DECLARE_READ8_MEMBER(lazercmd_hardware_r);
	DECLARE_DRIVER_INIT(lazercmd);
	DECLARE_DRIVER_INIT(bbonk);
	DECLARE_DRIVER_INIT(medlanes);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void palette_init();
	UINT32 screen_update_lazercmd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
