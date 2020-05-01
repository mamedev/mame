// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  cdplay.cpp: Generic CHD-based CD player

****************************************************************************/

#include "emu.h"
#include "imagedev/chd_cd.h"
#include "sound/cdda.h"
#include "sound/vgm_visualizer.h"
#include "speaker.h"

enum cdplay_inputs : uint8_t
{
	CDPLAY_STOP,
	CDPLAY_PAUSE,
	CDPLAY_PLAY,
	CDPLAY_NEXT,
	CDPLAY_PREV,
	CDPLAY_VIZ,
};

class cdplay_state : public driver_device
{
public:
	cdplay_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cdda(*this, "cdda")
		, m_cdrom(*this, "cdrom")
		, m_visualizer(*this, "visualizer")
	{ }

	void cdplay(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_pressed);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static constexpr device_timer_id TIMER_DISK_CHECK = 0;

	void next_track();
	void prev_track();
	void set_track(int track);
	void play_track(int track);
	void play();
	void stop();
	void pause();

	required_device<cdda_device> m_cdda;
	required_device<cdrom_image_device> m_cdrom;
	required_device<vgmviz_device> m_visualizer;

	int m_track;
	int m_max_track;
	bool m_playing;
	cdrom_file *m_file;
	emu_timer *m_disk_check_timer;
};

static INPUT_PORTS_START( cdplay )
	PORT_START("CONTROLS")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_CHANGED_MEMBER(DEVICE_SELF, cdplay_state, key_pressed, CDPLAY_STOP)  PORT_CODE(KEYCODE_Z) PORT_NAME("Stop")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_BUTTON2)  PORT_CHANGED_MEMBER(DEVICE_SELF, cdplay_state, key_pressed, CDPLAY_PAUSE) PORT_CODE(KEYCODE_X) PORT_NAME("Pause")
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_BUTTON3)  PORT_CHANGED_MEMBER(DEVICE_SELF, cdplay_state, key_pressed, CDPLAY_PLAY)  PORT_CODE(KEYCODE_C) PORT_NAME("Play")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_BUTTON4)  PORT_CHANGED_MEMBER(DEVICE_SELF, cdplay_state, key_pressed, CDPLAY_NEXT)  PORT_CODE(KEYCODE_A) PORT_NAME("Next Track")
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_BUTTON5)  PORT_CHANGED_MEMBER(DEVICE_SELF, cdplay_state, key_pressed, CDPLAY_PREV)  PORT_CODE(KEYCODE_S) PORT_NAME("Previous Track")
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_BUTTON6)  PORT_CHANGED_MEMBER(DEVICE_SELF, cdplay_state, key_pressed, CDPLAY_VIZ)   PORT_CODE(KEYCODE_D) PORT_NAME("Visualization Mode")
INPUT_PORTS_END

void cdplay_state::machine_start()
{
	save_item(NAME(m_track));
	save_item(NAME(m_max_track));
	save_item(NAME(m_playing));

	m_disk_check_timer = timer_alloc(TIMER_DISK_CHECK);
}

void cdplay_state::machine_reset()
{
	m_track = -1;
	m_max_track = -1;
	m_playing = false;
	m_file = nullptr;

	m_disk_check_timer->adjust(attotime::from_msec(100), 0, attotime::from_msec(100));
}

void cdplay_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_DISK_CHECK)
	{
		cdrom_file *new_file = m_cdrom->get_cdrom_file();
		if (m_file != new_file)
		{
			if (new_file == nullptr && m_playing)
			{
				m_cdda->stop_audio();
				m_cdda->set_cdrom(nullptr);
			}

			m_track = -1;
			m_max_track = -1;
			m_playing = false;

			if (new_file != nullptr)
			{
				m_max_track = cdrom_get_last_track(new_file);
				printf("New file found, max track is %d\n", m_max_track);
				m_file = new_file;
				m_cdda->set_cdrom(m_file);
			}
		}
	}
}

INPUT_CHANGED_MEMBER(cdplay_state::key_pressed)
{
	if (!newval)
		return;

	switch (param)
	{
	case CDPLAY_STOP:
		stop();
		break;
	case CDPLAY_PAUSE:
		pause();
		break;
	case CDPLAY_PLAY:
		play();
		break;
	case CDPLAY_NEXT:
		next_track();
		break;
	case CDPLAY_PREV:
		prev_track();
		break;
	case CDPLAY_VIZ:
		m_visualizer->cycle_viz_mode();
		break;
	}
}

void cdplay_state::next_track()
{
	if (m_file == nullptr)
	{
		return;
	}

	do
	{
		m_track++;
		if (m_track > m_max_track)
		{
			m_track = 0;
		}
	} while (cdrom_get_track_type(m_file, m_track) != CD_TRACK_AUDIO);
	play_track(m_track);
}

void cdplay_state::prev_track()
{
	if (m_file == nullptr)
	{
		return;
	}

	do
	{
		m_track--;
		if (m_track < 0)
		{
			m_track = m_max_track;
		}
	} while (cdrom_get_track_type(m_file, m_track) != CD_TRACK_AUDIO);
	play_track(m_track);
}

void cdplay_state::set_track(int track)
{
	m_track = track;
	if (m_track <= m_max_track)
	{
		play_track(m_track);
	}
}

void cdplay_state::play_track(int track)
{
	const cdrom_toc *toc = cdrom_get_toc(m_file);
	printf("Playing track %d, start frame %d, length %d frames\n", m_track, toc->tracks[track].chdframeofs, toc->tracks[track].frames);
	m_cdda->start_audio(toc->tracks[track].chdframeofs, toc->tracks[track].frames);
}

void cdplay_state::play()
{
	if (m_file == nullptr)
	{
		return;
	}

	m_track = 0;

	while (cdrom_get_track_type(m_file, m_track) != CD_TRACK_AUDIO && m_track != m_max_track)
	{
		m_track++;
	}

	if (m_track <= m_max_track)
	{
		m_cdda->set_cdrom(m_file);
		play_track(m_track);
		m_playing = true;
	}
	else
	{
		printf("Unable to play this disc; there are no audio tracks present\n");
		m_track = -1;
		m_playing = false;
	}
}

void cdplay_state::stop()
{
	if (m_file == nullptr)
	{
		return;
	}

	m_track = -1;
	m_playing = false;
	m_cdda->stop_audio();
}

void cdplay_state::pause()
{
	printf("%s track %d\n", m_playing ? "Pausing" : "Resuming", m_track);
	m_cdda->pause_audio(m_playing ? 1 : 0);
	m_playing = !m_playing;
}

void cdplay_state::cdplay(machine_config &config)
{
	/* sound hardware */
	VGMVIZ(config, m_visualizer, 0);
	m_visualizer->add_route(0, "lspeaker", 1);
	m_visualizer->add_route(1, "rspeaker", 1);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	CDDA(config, m_cdda);
	m_cdda->add_route(0, m_visualizer, 1, AUTO_ALLOC_INPUT, 0);
	m_cdda->add_route(1, m_visualizer, 1, AUTO_ALLOC_INPUT, 1);

	CDROM(config, m_cdrom, 0);
	m_cdrom->set_interface("cdplay_cdrom");
}

ROM_START( cdplay )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY  FULLNAME     FLAGS */
CONS( 2020, cdplay, 0,      0,      cdplay,  cdplay, cdplay_state, empty_init, "MAME",  "CD Player", 0 )
