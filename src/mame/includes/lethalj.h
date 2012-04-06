/*************************************************************************

    The Game Room Lethal Justice hardware

**************************************************************************/

class lethalj_state : public driver_device
{
public:
	lethalj_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_blitter_data[8];
	UINT16 *m_screenram;
	UINT8 m_vispage;
	UINT16 *m_blitter_base;
	int m_blitter_rows;
	UINT16 m_gunx;
	UINT16 m_guny;
	UINT8 m_blank_palette;
	DECLARE_WRITE16_MEMBER(ripribit_control_w);
	DECLARE_WRITE16_MEMBER(cfarm_control_w);
	DECLARE_WRITE16_MEMBER(cclownz_control_w);
	DECLARE_READ16_MEMBER(lethalj_gun_r);
	DECLARE_WRITE16_MEMBER(lethalj_blitter_w);
	void do_blit();
};


/*----------- defined in video/lethalj.c -----------*/


VIDEO_START( lethalj );


void lethalj_scanline_update(screen_device &screen, bitmap_ind16 &bitmap, int scanline, const tms34010_display_params *params);
