// license:BSD-3-Clause
// copyright-holders:Chris Moore
/***************************************************************************

GAME PLAN driver

driver by Chris Moore

***************************************************************************/

#include "machine/6522via.h"
#include "machine/6532riot.h"
#include "machine/gen_latch.h"
#include "screen.h"

#define GAMEPLAN_MAIN_MASTER_CLOCK       (XTAL(3'579'545))
#define GAMEPLAN_AUDIO_MASTER_CLOCK      (XTAL(3'579'545))
#define GAMEPLAN_MAIN_CPU_CLOCK          (GAMEPLAN_MAIN_MASTER_CLOCK / 4)
#define GAMEPLAN_AUDIO_CPU_CLOCK         (GAMEPLAN_AUDIO_MASTER_CLOCK / 4)
#define GAMEPLAN_AY8910_CLOCK            (GAMEPLAN_AUDIO_MASTER_CLOCK / 2)
#define GAMEPLAN_PIXEL_CLOCK             (XTAL(11'668'800) / 2)

/* Used Leprechaun/Pot of Gold (and Pirate Treasure) - as stated in manual for Pot Of Gold */

#define LEPRECHAUN_MAIN_MASTER_CLOCK     (XTAL(4'000'000))
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
			m_screen(*this, "screen"),
			m_soundlatch(*this, "soundlatch") { }

	void gameplan(machine_config &config);
	void gameplan_video(machine_config &config);
	void leprechn(machine_config &config);
	void leprechn_video(machine_config &config);
	void trvquest(machine_config &config);
	void trvquest_video(machine_config &config);

private:
	/* machine state */
	uint8_t   m_current_port;
	optional_shared_ptr<uint8_t> m_trvquest_question;

	/* video state */
	std::unique_ptr<uint8_t[]>   m_videoram;
	size_t   m_videoram_size;
	uint8_t    m_video_x;
	uint8_t    m_video_y;
	uint8_t    m_video_command;
	uint8_t    m_video_data;
	emu_timer *m_via_0_ca1_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<riot6532_device> m_riot;
	required_device<via6522_device> m_via_0;
	required_device<via6522_device> m_via_1;
	required_device<via6522_device> m_via_2;
	required_device<screen_device> m_screen;
	optional_device<generic_latch_8_device> m_soundlatch;


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
	uint32_t screen_update_gameplan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_leprechn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
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

	void cpu_map(address_map &map);
	void gameplan_audio_map(address_map &map);
	void gameplan_main_map(address_map &map);
	void leprechn_audio_map(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
