// license:BSD-3-Clause
// copyright-holders:Yochizo, Takahiro Nogi
#include "sound/msm5205.h"
#include "video/seta001.h"

struct iox_t
{
	int reset,ff_event,ff_1,protcheck[4],protlatch[4];
	uint8_t data;
	uint8_t mux;
	uint8_t ff;
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
	uint32_t m_adpcm_sptr;
	uint32_t m_adpcm_eptr;
	iox_t m_iox;

	// common
	uint8_t vox_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t iox_mux_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t iox_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void iox_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void iox_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_int(int state);

	// mjuugi
	void mjyuugi_flags_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mjyuugi_adpcm_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t mjyuugi_irq2_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mjyuugi_irq4_ack_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	// rmgoldyh
	void rmgoldyh_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// srmp2
	void srmp2_irq2_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srmp2_irq4_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srmp2_flags_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void srmp2_adpcm_code_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// srmp3
	void srmp3_rombank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srmp3_flags_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srmp3_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void srmp3_adpcm_code_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	virtual void machine_start() override;
	void machine_start_srmp2();
	void palette_init_srmp2(palette_device &palette);
	void machine_start_srmp3();
	void palette_init_srmp3(palette_device &palette);
	void machine_start_rmgoldyh();
	void machine_start_mjyuugi();

	uint32_t screen_update_srmp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_srmp3(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mjyuugi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	SETA001_SPRITE_GFXBANK_CB_MEMBER(srmp3_gfxbank_callback);

	uint8_t iox_key_matrix_calc(uint8_t p_side);
};
