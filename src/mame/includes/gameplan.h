// license:BSD-3-Clause
// copyright-holders:Chris Moore
/***************************************************************************

GAME PLAN driver

driver by Chris Moore

***************************************************************************/

#include "machine/6522via.h"
#include "machine/6532riot.h"

#define GAMEPLAN_MAIN_MASTER_CLOCK       (XTAL_3_579545MHz)
#define GAMEPLAN_AUDIO_MASTER_CLOCK      (XTAL_3_579545MHz)
#define GAMEPLAN_MAIN_CPU_CLOCK          (GAMEPLAN_MAIN_MASTER_CLOCK / 4)
#define GAMEPLAN_AUDIO_CPU_CLOCK         (GAMEPLAN_AUDIO_MASTER_CLOCK / 4)
#define GAMEPLAN_AY8910_CLOCK            (GAMEPLAN_AUDIO_MASTER_CLOCK / 2)
#define GAMEPLAN_PIXEL_CLOCK             (XTAL_11_6688MHz / 2)

/* Used Leprechaun/Pot of Gold (and Pirate Treasure) - as stated in manual for Pot Of Gold */

#define LEPRECHAUN_MAIN_MASTER_CLOCK     (XTAL_4MHz)
#define LEPRECHAUN_MAIN_CPU_CLOCK        (LEPRECHAUN_MAIN_MASTER_CLOCK / 4)


class gameplan_state : public driver_device
{
public:
	enum
	{
		TIMER_CLEAR_SCREEN_DONE,
		TIMER_VIA_IRQ_DELAYED,
		TIMER_VIA_0_CAL
	};

	gameplan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_trvquest_question(*this, "trvquest_q"),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_riot(*this, "riot"),
			m_via_0(*this, "via6522_0"),
			m_via_1(*this, "via6522_1"),
			m_via_2(*this, "via6522_2"),
			m_screen(*this, "screen") { }

	/* machine state */
	UINT8   m_current_port;
	optional_shared_ptr<UINT8> m_trvquest_question;

	/* video state */
	UINT8   *m_videoram;
	size_t   m_videoram_size;
	UINT8    m_video_x;
	UINT8    m_video_y;
	UINT8    m_video_command;
	UINT8    m_video_data;
	emu_timer *m_via_0_ca1_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<riot6532_device> m_riot;
	required_device<via6522_device> m_via_0;
	required_device<via6522_device> m_via_1;
	required_device<via6522_device> m_via_2;
	required_device<screen_device> m_screen;
	DECLARE_WRITE8_MEMBER(io_select_w);
	DECLARE_READ8_MEMBER(io_port_r);
	DECLARE_WRITE_LINE_MEMBER(coin_w);
	DECLARE_WRITE_LINE_MEMBER(audio_reset_w);
	DECLARE_WRITE8_MEMBER(audio_cmd_w);
	DECLARE_WRITE_LINE_MEMBER(audio_trigger_w);
	DECLARE_WRITE_LINE_MEMBER(r6532_irq);
	DECLARE_WRITE8_MEMBER(r6532_soundlatch_w);
	DECLARE_MACHINE_START(gameplan);
	DECLARE_MACHINE_RESET(gameplan);
	DECLARE_MACHINE_START(trvquest);
	DECLARE_MACHINE_RESET(trvquest);
	DECLARE_VIDEO_START(gameplan);
	DECLARE_VIDEO_RESET(gameplan);
	DECLARE_VIDEO_START(leprechn);
	DECLARE_VIDEO_START(trvquest);
	DECLARE_VIDEO_START(common);
	UINT32 screen_update_gameplan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_leprechn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(trvquest_interrupt);
	TIMER_CALLBACK_MEMBER(clear_screen_done_callback);
	TIMER_CALLBACK_MEMBER(via_irq_delayed);
	TIMER_CALLBACK_MEMBER(via_0_ca1_timer_callback);
	DECLARE_WRITE8_MEMBER(video_data_w);
	DECLARE_WRITE8_MEMBER(gameplan_video_command_w);
	DECLARE_WRITE8_MEMBER(leprechn_video_command_w);
	DECLARE_READ8_MEMBER(leprechn_videoram_r);
	DECLARE_WRITE_LINE_MEMBER(video_command_trigger_w);
	void gameplan_get_pens( pen_t *pens );
	void leprechn_get_pens( pen_t *pens );
	DECLARE_WRITE_LINE_MEMBER(via_irq);
	DECLARE_READ8_MEMBER(trvquest_question_r);
	DECLARE_WRITE_LINE_MEMBER(trvquest_coin_w);
	DECLARE_WRITE_LINE_MEMBER(trvquest_misc_w);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in video/gameplan.c -----------*/

MACHINE_CONFIG_EXTERN( gameplan_video );
MACHINE_CONFIG_EXTERN( leprechn_video );
MACHINE_CONFIG_EXTERN( trvquest_video );
