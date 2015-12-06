// license:BSD-3-Clause
// copyright-holders:Yochizo, Takahiro Nogi
#include "sound/msm5205.h"
#include "video/seta001.h"

struct iox_t
{
	int reset,ff_event,ff_1,protcheck[4],protlatch[4];
	UINT8 data;
	UINT8 mux;
	UINT8 ff;
};

class srmp2_state : public driver_device
{
public:
	srmp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seta001(*this, "spritegen"),
		m_msm(*this, "msm") { }

	required_device<cpu_device> m_maincpu;
	required_device<seta001_device> m_seta001;
	required_device<msm5205_device> m_msm;

	int m_color_bank;
	int m_gfx_bank;
	int m_adpcm_bank;
	int m_adpcm_data;
	UINT32 m_adpcm_sptr;
	UINT32 m_adpcm_eptr;
	iox_t m_iox;

	// common
	DECLARE_READ8_MEMBER(vox_status_r);
	DECLARE_READ8_MEMBER(iox_mux_r);
	DECLARE_READ8_MEMBER(iox_status_r);
	DECLARE_WRITE8_MEMBER(iox_command_w);
	DECLARE_WRITE8_MEMBER(iox_data_w);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);

	// mjuugi
	DECLARE_WRITE16_MEMBER(mjyuugi_flags_w);
	DECLARE_WRITE16_MEMBER(mjyuugi_adpcm_bank_w);
	DECLARE_READ8_MEMBER(mjyuugi_irq2_ack_r);
	DECLARE_READ8_MEMBER(mjyuugi_irq4_ack_r);

	// rmgoldyh
	DECLARE_WRITE8_MEMBER(rmgoldyh_rombank_w);

	// srmp2
	DECLARE_WRITE8_MEMBER(srmp2_irq2_ack_w);
	DECLARE_WRITE8_MEMBER(srmp2_irq4_ack_w);
	DECLARE_WRITE16_MEMBER(srmp2_flags_w);
	DECLARE_WRITE16_MEMBER(srmp2_adpcm_code_w);

	// srmp3
	DECLARE_WRITE8_MEMBER(srmp3_rombank_w);
	DECLARE_WRITE8_MEMBER(srmp3_flags_w);
	DECLARE_WRITE8_MEMBER(srmp3_irq_ack_w);
	DECLARE_WRITE8_MEMBER(srmp3_adpcm_code_w);

	virtual void machine_start() override;
	DECLARE_MACHINE_START(srmp2);
	DECLARE_PALETTE_INIT(srmp2);
	DECLARE_MACHINE_START(srmp3);
	DECLARE_PALETTE_INIT(srmp3);
	DECLARE_MACHINE_START(rmgoldyh);
	DECLARE_MACHINE_START(mjyuugi);

	UINT32 screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mjyuugi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	SETA001_SPRITE_GFXBANK_CB_MEMBER(srmp3_gfxbank_callback);

	UINT8 iox_key_matrix_calc(UINT8 p_side);
};
