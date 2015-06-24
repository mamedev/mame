// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert
/*************************************************************************

    Taito Air System

*************************************************************************/

#include "machine/taitoio.h"
#include "video/tc0080vco.h"

enum { TAITOAIR_FRAC_SHIFT = 16, TAITOAIR_POLY_MAX_PT = 16 };

struct taitoair_spoint {
	INT32 x, y;
};

struct taitoair_poly {
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT];
	int pcount;
	int col;
};


class taitoair_state : public driver_device
{
public:
	taitoair_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_m68000_mainram(*this, "m68000_mainram"),
			m_line_ram(*this, "line_ram"),
			m_dsp_ram(*this, "dsp_ram"),
			m_paletteram(*this, "paletteram"),
			m_gradram(*this, "gradram"),
			m_tc0430grw(*this, "tc0430grw"),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_dsp(*this, "dsp"),
			m_tc0080vco(*this, "tc0080vco"),
			m_tc0220ioc(*this, "tc0220ioc"),
			m_gfxdecode(*this, "gfxdecode"),
			m_screen(*this, "screen"),
			m_palette(*this, "palette")
			{ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_m68000_mainram;
	required_shared_ptr<UINT16> m_line_ram;
	required_shared_ptr<UINT16> m_dsp_ram;          // Shared 68000/TMS32025 RAM
	required_shared_ptr<UINT16> m_paletteram;
	required_shared_ptr<UINT16> m_gradram;
	required_shared_ptr<UINT16> m_tc0430grw;

	/* video-related */
	taitoair_poly  m_q;

	/* misc */
	int           m_dsp_hold_signal;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_dsp;
	required_device<tc0080vco_device> m_tc0080vco;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	bitmap_ind16 *m_framebuffer[2];

	/* 3d info */
	INT16 m_frustumLeft;
	INT16 m_frustumBottom;
	INT16 m_eyecoordBuffer[4];  /* homogeneous */

	//bitmap_ind16 *m_buffer3d;
	DECLARE_WRITE16_MEMBER(system_control_w);
	DECLARE_READ16_MEMBER(lineram_r);
	DECLARE_WRITE16_MEMBER(lineram_w);
	DECLARE_READ16_MEMBER(dspram_r);
	DECLARE_WRITE16_MEMBER(dspram_w);
	DECLARE_READ16_MEMBER(dsp_HOLD_signal_r);
	DECLARE_WRITE16_MEMBER(dsp_HOLDA_signal_w);
	DECLARE_WRITE16_MEMBER(airsys_paletteram16_w);
	DECLARE_WRITE16_MEMBER(airsys_gradram_w);
	DECLARE_READ16_MEMBER(stick_input_r);
	DECLARE_READ16_MEMBER(stick2_input_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(dsp_flags_w);
	DECLARE_WRITE16_MEMBER(dsp_x_eyecoord_w);
	DECLARE_WRITE16_MEMBER(dsp_y_eyecoord_w);
	DECLARE_WRITE16_MEMBER(dsp_z_eyecoord_w);
	DECLARE_WRITE16_MEMBER(dsp_frustum_left_w);
	DECLARE_WRITE16_MEMBER(dsp_frustum_bottom_w);
	DECLARE_WRITE16_MEMBER(dsp_rasterize_w);
	DECLARE_READ16_MEMBER(dsp_x_return_r);
	DECLARE_READ16_MEMBER(dsp_y_return_r);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_taitoair(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void fill_slope( bitmap_ind16 &bitmap, const rectangle &cliprect, int color, INT32 x1, INT32 x2, INT32 sl1, INT32 sl2, INT32 y1, INT32 y2, INT32 *nx1, INT32 *nx2 );
	void multVecMtx(const INT16* vec4, const float* m, float* result);
	void airInfernoFrustum(const INT16 leftExtent, const INT16 bottomExtent, float* m);
	void fill_poly( bitmap_ind16 &bitmap, const rectangle &cliprect, const struct taitoair_poly *q );
	int projectEyeCoordToScreen(float* projectionMatrix,const int Res,INT16* eyePoint3d,int type);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
};
