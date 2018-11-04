// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Kitco Crowns Golf hardware

**************************************************************************/
#include "sound/msm5205.h"
#include "machine/bankdev.h"
#define MASTER_CLOCK        XTAL(18'432'000)


class crgolf_state : public driver_device
{
public:
	crgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),

		m_videoram_a(*this, "vrama"),
		m_videoram_b(*this, "vramb"),

		m_vrambank(*this, "vrambank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_palette(*this, "palette")
	{ }



	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram_a;
	required_shared_ptr<uint8_t> m_videoram_b;

	bool m_color_select;
	bool m_screen_flip;
	bool m_screena_enable;
	bool m_screenb_enable;

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
	DECLARE_WRITE8_MEMBER(rom_bank_select_w);
	DECLARE_READ8_MEMBER(switch_input_r);
	DECLARE_READ8_MEMBER(analog_input_r);
	DECLARE_WRITE8_MEMBER(switch_input_select_w);
	DECLARE_WRITE8_MEMBER(unknown_w);
	DECLARE_WRITE8_MEMBER(main_to_sound_w);
	DECLARE_READ8_MEMBER(main_to_sound_r);
	DECLARE_WRITE8_MEMBER(sound_to_main_w);
	DECLARE_READ8_MEMBER(sound_to_main_r);
	DECLARE_WRITE_LINE_MEMBER(color_select_w);
	DECLARE_WRITE_LINE_MEMBER(screen_flip_w);
	DECLARE_WRITE_LINE_MEMBER(screen_select_w);
	DECLARE_WRITE_LINE_MEMBER(screena_enable_w);
	DECLARE_WRITE_LINE_MEMBER(screenb_enable_w);
	DECLARE_WRITE8_MEMBER(crgolfhi_sample_w);
	DECLARE_READ8_MEMBER(unk_sub_02_r);
	DECLARE_READ8_MEMBER(unk_sub_05_r);
	DECLARE_READ8_MEMBER(unk_sub_07_r);
	DECLARE_WRITE8_MEMBER(unk_sub_0c_w);
	DECLARE_DRIVER_INIT(crgolfhi);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(crgolf);
	DECLARE_PALETTE_INIT(mastrglf);
	uint32_t screen_update_crgolf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(main_to_sound_callback);
	TIMER_CALLBACK_MEMBER(sound_to_main_callback);
	void get_pens( pen_t *pens );
	DECLARE_WRITE_LINE_MEMBER(vck_callback);
	void crgolfhi(machine_config &config);
	void crgolf(machine_config &config);
	void crgolf_video(machine_config &config);
	void mastrglf(machine_config &config);
	void main_map(address_map &map);
	void mastrglf_io(address_map &map);
	void mastrglf_map(address_map &map);
	void mastrglf_subio(address_map &map);
	void mastrglf_submap(address_map &map);
	void sound_map(address_map &map);
	void vrambank_map(address_map &map);
};
