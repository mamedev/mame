// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Kitco Crowns Golf hardware

**************************************************************************/
#include "sound/msm5205.h"
#include "machine/bankdev.h"
#define MASTER_CLOCK        XTAL_18_432MHz


class crgolf_state : public driver_device
{
public:
	crgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),

		m_videoram_a(*this, "vrama"),
		m_videoram_b(*this, "vramb"),
		m_color_select(*this, "color_select"),
		m_screen_flip(*this, "screen_flip"),
		m_screenb_enable(*this, "screenb_enable"),
		m_screena_enable(*this, "screena_enable"),

		m_vrambank(*this, "vrambank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette")
	{ }



	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram_a;
	required_shared_ptr<uint8_t> m_videoram_b;

	required_shared_ptr<uint8_t> m_color_select;
	required_shared_ptr<uint8_t> m_screen_flip;
	required_shared_ptr<uint8_t> m_screenb_enable;
	required_shared_ptr<uint8_t> m_screena_enable;

	/* misc */
	uint8_t    m_port_select;
	uint8_t    m_main_to_sound_data;
	uint8_t    m_sound_to_main_data;
	uint16_t   m_sample_offset;
	uint8_t    m_sample_count;

	/* devices */
	required_device<address_map_bank_device> m_vrambank;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<palette_device> m_palette;
	void rom_bank_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t switch_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t analog_input_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void switch_input_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unknown_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void main_to_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t main_to_sound_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sound_to_main_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sound_to_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void screen_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void crgolfhi_sample_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t unk_sub_02_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unk_sub_05_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t unk_sub_07_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void unk_sub_0c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_crgolfhi();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void palette_init_crgolf(palette_device &palette);
	void palette_init_mastrglf(palette_device &palette);
	uint32_t screen_update_crgolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void main_to_sound_callback(void *ptr, int32_t param);
	void sound_to_main_callback(void *ptr, int32_t param);
	void get_pens( pen_t *pens );
	void vck_callback(int state);
};

/*----------- defined in video/crgolf.c -----------*/
MACHINE_CONFIG_EXTERN( crgolf_video );
