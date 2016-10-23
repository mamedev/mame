// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert
/*************************************************************************

    Taito Air System

*************************************************************************/

#include "machine/taitoio.h"
#include "video/tc0080vco.h"

enum { TAITOAIR_FRAC_SHIFT = 16, TAITOAIR_POLY_MAX_PT = 16 };

struct taitoair_spoint {
	int32_t x, y;
};

struct taitoair_poly {
	struct taitoair_spoint p[TAITOAIR_POLY_MAX_PT];
	int pcount;
	uint16_t header;
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
	required_shared_ptr<uint16_t> m_m68000_mainram;
	required_shared_ptr<uint16_t> m_line_ram;
	required_shared_ptr<uint16_t> m_dsp_ram;          // Shared 68000/TMS32025 RAM
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_gradram;
	required_shared_ptr<uint16_t> m_tc0430grw;

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

	std::unique_ptr<bitmap_ind16> m_framebuffer[2];

	/* 3d info */
	int16_t m_frustumLeft;
	int16_t m_frustumBottom;
	int16_t m_eyecoordBuffer[4];  /* homogeneous */

	bool m_gradbank;

	uint16_t m_dsp_test_object_type;
	int16_t m_dsp_test_or_clip, m_dsp_test_and_clip;
	int16_t m_dsp_test_x, m_dsp_test_y, m_dsp_test_z;

	void dsp_test_start_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_test_x_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_test_y_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_test_z_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_test_point_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_test_or_clip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t dsp_test_and_clip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	int16_t m_dsp_muldiv_a_1, m_dsp_muldiv_b_1, m_dsp_muldiv_c_1;

	void dsp_muldiv_a_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_muldiv_b_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_muldiv_c_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_muldiv_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	int16_t m_dsp_muldiv_a_2, m_dsp_muldiv_b_2, m_dsp_muldiv_c_2;

	void dsp_muldiv_a_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_muldiv_b_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dsp_muldiv_c_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_muldiv_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	//bitmap_ind16 *m_buffer3d;
	void system_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t lineram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void lineram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dspram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dspram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dsp_HOLD_signal_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void dsp_HOLDA_signal_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void airsys_paletteram16_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void airsys_gradram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t stick_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t stick2_input_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dsp_flags_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void dma_regs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_taitoair(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	int draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect, int start_offset );
	void fb_copy_op(void);
	void fb_fill_op(void);
	void fb_erase_op(void);

	void fill_slope( bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t header, int32_t x1, int32_t x2, int32_t sl1, int32_t sl2, int32_t y1, int32_t y2, int32_t *nx1, int32_t *nx2 );
	void fill_poly( bitmap_ind16 &bitmap, const rectangle &cliprect, const struct taitoair_poly *q );
};
