// license:BSD-3-Clause
// copyright-holders:Chris Moore
/***************************************************************************

GAME PLAN driver

driver by Chris Moore

***************************************************************************/

#include "machine/6522via.h"
#include "machine/6532riot.h"
#include "machine/gen_latch.h"

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
			m_screen(*this, "screen"),
			m_soundlatch(*this, "soundlatch") { }

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


	void io_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t io_port_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void coin_w(int state);
	void audio_reset_w(int state);
	void audio_cmd_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void audio_trigger_w(int state);
	void r6532_irq(int state);
	void r6532_soundlatch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void machine_start_gameplan();
	void machine_reset_gameplan();
	void machine_start_trvquest();
	void machine_reset_trvquest();
	void video_start_gameplan();
	void video_reset_gameplan();
	void video_start_leprechn();
	void video_start_trvquest();
	void video_start_common();
	uint32_t screen_update_gameplan(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_leprechn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void trvquest_interrupt(device_t &device);
	void clear_screen_done_callback(void *ptr, int32_t param);
	void via_irq_delayed(void *ptr, int32_t param);
	void via_0_ca1_timer_callback(void *ptr, int32_t param);
	void video_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gameplan_video_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void leprechn_video_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t leprechn_videoram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void video_command_trigger_w(int state);
	void gameplan_get_pens( pen_t *pens );
	void leprechn_get_pens( pen_t *pens );
	void via_irq(int state);
	uint8_t trvquest_question_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void trvquest_coin_w(int state);
	void trvquest_misc_w(int state);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

/*----------- defined in video/gameplan.c -----------*/

MACHINE_CONFIG_EXTERN( gameplan_video );
MACHINE_CONFIG_EXTERN( leprechn_video );
MACHINE_CONFIG_EXTERN( trvquest_video );
