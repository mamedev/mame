// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

#include "machine/gen_latch.h"
#include "sound/hc55516.h"

class redalert_state : public driver_device
{
public:
	redalert_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bitmap_videoram(*this, "bitmap_videoram"),
		m_charmap_videoram(*this, "charram"),
		m_video_control(*this, "video_control"),
		m_bitmap_color(*this, "bitmap_color"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_cvsd(*this, "cvsd"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2") { }

	uint8_t m_ay8910_latch_1;
	uint8_t m_ay8910_latch_2;

	required_shared_ptr<uint8_t> m_bitmap_videoram;
	required_shared_ptr<uint8_t> m_charmap_videoram;
	required_shared_ptr<uint8_t> m_video_control;
	required_shared_ptr<uint8_t> m_bitmap_color;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<hc55516_device> m_cvsd;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	optional_device<generic_latch_8_device> m_soundlatch2;

	std::unique_ptr<uint8_t[]> m_bitmap_colorram;
	uint8_t m_control_xor;
	uint8_t redalert_interrupt_clear_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void redalert_interrupt_clear_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t panther_interrupt_clear_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t panther_unk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void redalert_bitmap_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redalert_audio_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t redalert_ay8910_latch_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void redalert_ay8910_latch_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redalert_voice_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void demoneye_audio_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void video_start_redalert();
	void video_start_ww3();
	void sound_start_redalert();
	void sound_start_demoneye();
	uint32_t screen_update_redalert(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_demoneye(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_panther(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void redalert_vblank_interrupt(device_t &device);
	void redalert_analog_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void redalert_AY8910_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sod_callback(int state);
	int sid_callback();
	void demoneye_ay8910_latch_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t demoneye_ay8910_latch_2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void demoneye_ay8910_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_pens(pen_t *pens);
	void get_panther_pens(pen_t *pens);
};
/*----------- defined in audio/redalert.c -----------*/

MACHINE_CONFIG_EXTERN( redalert_audio );
MACHINE_CONFIG_EXTERN( ww3_audio );
MACHINE_CONFIG_EXTERN( demoneye_audio );

/*----------- defined in video/redalert.c -----------*/

MACHINE_CONFIG_EXTERN( ww3_video );
MACHINE_CONFIG_EXTERN( panther_video );
MACHINE_CONFIG_EXTERN( redalert_video );
MACHINE_CONFIG_EXTERN( demoneye_video );
