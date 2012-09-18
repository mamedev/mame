/*************************************************************************

    Coors Light Bowling/Bowl-O-Rama hardware

*************************************************************************/

#include "machine/nvram.h"

class capbowl_state : public driver_device
{
public:
	capbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_rowaddress(*this, "rowaddress"){ }

	void init_nvram(nvram_device &nvram, void *base, size_t size);

	/* memory pointers */
	required_shared_ptr<UINT8> m_rowaddress;

	/* video-related */
	offs_t m_blitter_addr;

	/* input-related */
	UINT8 m_last_trackball_val[2];

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_WRITE8_MEMBER(capbowl_rom_select_w);
	DECLARE_READ8_MEMBER(track_0_r);
	DECLARE_READ8_MEMBER(track_1_r);
	DECLARE_WRITE8_MEMBER(track_reset_w);
	DECLARE_WRITE8_MEMBER(capbowl_sndcmd_w);
	DECLARE_WRITE8_MEMBER(capbowl_tms34061_w);
	DECLARE_READ8_MEMBER(capbowl_tms34061_r);
	DECLARE_WRITE8_MEMBER(bowlrama_blitter_w);
	DECLARE_READ8_MEMBER(bowlrama_blitter_r);
	DECLARE_DRIVER_INIT(capbowl);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_capbowl(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};
