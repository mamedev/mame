// license:BSD-3-Clause
// copyright-holders:Chris Moore
/***************************************************************************

    Killer Comet driver

***************************************************************************/

#include "machine/6522via.h"
#include "machine/mos6530.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"


class killcom_state : public driver_device
{
public:
	killcom_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via%u", 1U),
		m_screen(*this, "screen"),
		m_inputs(*this, "IN%u", 0U),
		m_dsw(*this, "DSW%u", 0U),
		m_audiocpu(*this, "audiocpu"),
		m_riot(*this, "riot"),
		m_ay(*this, "ay")
	{ }

	void killcom(machine_config &config);
	void killcom_video(machine_config &config);
	void leprechn(machine_config &config);
	void piratetr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device_array<via6522_device, 3> m_via;
	required_device<screen_device> m_screen;
	optional_ioport_array<4> m_inputs;
	optional_ioport_array<4> m_dsw;

	void coin_w(int state);
	void video_data_w(uint8_t data);
	void video_command_w(uint8_t data);
	uint8_t video_status_r();
	void video_command_trigger_w(int state);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	/* machine state */
	uint8_t m_current_port = 0;
	uint8_t m_audio_reset = 0;

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
	optional_device<mos6532_device> m_riot;
	optional_device<ay8910_device> m_ay;

	void io_select_w(uint8_t data);
	uint8_t io_port_r();

	void audio_cmd_w_sync(int32_t param);
	void audio_trigger_w_sync(int32_t param);
	void audio_reset_w_sync(int32_t param);
	void audio_cmd_w(uint8_t data) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(killcom_state::audio_cmd_w_sync), this), data); }
	void audio_trigger_w(int state) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(killcom_state::audio_trigger_w_sync), this), state); }
	void audio_reset_w(int state) { machine().scheduler().synchronize(timer_expired_delegate(FUNC(killcom_state::audio_reset_w_sync), this), state); }

	TIMER_CALLBACK_MEMBER(hblank_callback);
	uint8_t leprechn_videoram_r();

	void killcom_main_map(address_map &map) ATTR_COLD;
	void killcom_audio_map(address_map &map) ATTR_COLD;
	void leprechn_audio_map(address_map &map) ATTR_COLD;
	void piratetr_main_map(address_map &map) ATTR_COLD;
};
