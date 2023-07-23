// license:BSD-3-Clause
// copyright-holders:Chris Moore
/***************************************************************************

GAME PLAN driver

***************************************************************************/

#include "machine/6522via.h"
#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "machine/mos6530n.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"


class gameplan_state : public driver_device
{
public:
	gameplan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via%u", 1U),
		m_screen(*this, "screen"),
		m_inputs(*this, "IN%u", 0U),
		m_dsw(*this, "DSW%u", 0U),
		m_audiocpu(*this, "audiocpu"),
		m_riot(*this, "riot"),
		m_ay(*this, "ay"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void gameplan(machine_config &config);
	void gameplan_video(machine_config &config);
	void leprechn(machine_config &config);
	void piratetr(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_device_array<via6522_device, 3> m_via;
	required_device<screen_device> m_screen;
	optional_ioport_array<4> m_inputs;
	optional_ioport_array<4> m_dsw;

	void video_data_w(uint8_t data);
	void video_command_w(uint8_t data);
	uint8_t video_status_r();
	void video_command_trigger_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	/* machine state */
	uint8_t m_current_port = 0;
	uint8_t m_audio_reset = 0;
	uint8_t m_audio_trigger = 0;

	/* video state */
	std::unique_ptr<uint8_t[]> m_videoram;
	uint8_t m_video_x = 0;
	uint8_t m_video_y = 0;
	uint8_t m_video_command = 0;
	uint8_t m_video_command_trigger = 0;
	uint8_t m_video_data = 0;
	uint8_t m_video_previous = 0;
	emu_timer *m_hblank_timer[2];

	/* devices */
	optional_device<cpu_device> m_audiocpu;
	optional_device<mos6532_new_device> m_riot;
	optional_device<ay8910_device> m_ay;
	optional_device<generic_latch_8_device> m_soundlatch;

	void io_select_w(uint8_t data);
	uint8_t io_port_r();
	void coin_w(int state);
	void audio_reset_w(int state);
	void audio_reset_sync_w(int param);
	void audio_trigger_w(int state);
	void audio_trigger_sync_w(int param);
	uint8_t soundlatch_r();

	TIMER_CALLBACK_MEMBER(hblank_callback);
	uint8_t leprechn_videoram_r();

	void gameplan_main_map(address_map &map);
	void gameplan_audio_map(address_map &map);
	void leprechn_audio_map(address_map &map);
	void piratetr_main_map(address_map &map);
};
