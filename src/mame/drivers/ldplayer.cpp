// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    ldplayer.c

    Laserdisc player driver.

**************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/ldpr8210.h"
#include "machine/ldv1000.h"

#include "ui/uimain.h"

#include "emuopts.h"
#include "fileio.h"
#include "romload.h"
#include "speaker.h"
#include "screen.h"

#include "chd.h"

#include "pr8210.lh"

#include <cctype>


class ldplayer_state : public driver_device
{
public:
	// construction/destruction
	ldplayer_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_last_controls(0)
		, m_playing(false) { }

	void ldplayer_ntsc(machine_config &config);

protected:
	// device overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// callback hook
	chd_file *get_disc();

	// internal helpers
	void process_commands();

	// derived classes
	virtual void execute_command(int command) { assert(false); }

	// timer IDs
	enum
	{
		TIMER_ID_AUTOPLAY,
		TIMER_ID_VSYNC_UPDATE
	};

	// commands
	enum
	{
		CMD_SCAN_REVERSE,
		CMD_STEP_REVERSE,
		CMD_SLOW_REVERSE,
		CMD_FAST_REVERSE,
		CMD_SCAN_FORWARD,
		CMD_STEP_FORWARD,
		CMD_SLOW_FORWARD,
		CMD_FAST_FORWARD,
		CMD_PLAY,
		CMD_PAUSE,
		CMD_FRAME_TOGGLE,
		CMD_CHAPTER_TOGGLE,
		CMD_CH1_TOGGLE,
		CMD_CH2_TOGGLE,
		CMD_0,
		CMD_1,
		CMD_2,
		CMD_3,
		CMD_4,
		CMD_5,
		CMD_6,
		CMD_7,
		CMD_8,
		CMD_9,
		CMD_SEARCH
	};

	// internal state
	required_device<screen_device> m_screen;
	std::string m_filename;
	ioport_value m_last_controls;
	bool m_playing;
};


class pr8210_state : public ldplayer_state
{
public:
	// construction/destruction
	pr8210_state(const machine_config &mconfig, device_type type, const char *tag)
		: ldplayer_state(mconfig, type, tag),
			m_laserdisc(*this, "laserdisc"),
			m_command_buffer_in(0),
			m_command_buffer_out(0) { }

			void pr8210(machine_config &config);
protected:
	// device overrides
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;
	virtual void machine_start() override;
	virtual void machine_reset() override;

	// command execution hook
	virtual void execute_command(int command) override;

	// internal helpers
	inline void add_command(uint8_t command);

	// timer IDs
	enum
	{
		TIMER_ID_BIT = 100,
		TIMER_ID_BIT_OFF
	};

	required_device<pioneer_pr8210_device> m_laserdisc;

	// internal state
	emu_timer *m_bit_timer;
	uint32_t m_command_buffer_in;
	uint32_t m_command_buffer_out;
	uint8_t m_command_buffer[10];
};


class ldv1000_state : public ldplayer_state
{
public:
	// construction/destruction
	ldv1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: ldplayer_state(mconfig, type, tag),
			m_laserdisc(*this, "laserdisc") { }

			void ldv1000(machine_config &config);
protected:
	required_device<pioneer_ldv1000_device> m_laserdisc;

	// command execution hook
	virtual void execute_command(int command) override;
};



/*************************************
 *
 *  Disc location
 *
 *************************************/

chd_file *ldplayer_state::get_disc()
{
	bool found = false;
	// open a path to the ROMs and find the first CHD file
	file_enumerator path(machine().options().media_path());

	// iterate while we get new objects
	const osd::directory::entry *dir;
	while ((dir = path.next()) != NULL)
	{
		int length = strlen(dir->name);

		// look for files ending in .chd
		if (length > 4 &&
			dir->name[length - 4] == '.' &&
			tolower(dir->name[length - 3]) == 'c' &&
			tolower(dir->name[length - 2]) == 'h' &&
			tolower(dir->name[length - 1]) == 'd')
		{
			// open the file itself via our search path
			emu_file image_file(machine().options().media_path(), OPEN_FLAG_READ);
			std::error_condition filerr = image_file.open(dir->name);
			if (!filerr)
			{
				std::string fullpath(image_file.fullpath());
				image_file.close();

				// try to open the CHD

				if (!machine().rom_load().set_disk_handle("laserdisc", fullpath.c_str()))
				{
					m_filename.assign(dir->name);
					found = true;
					break;
				}
			}
		}
	}

	// if we failed, pop a message and exit
	if (!found)
	{
		machine().ui().popup_time(10, "No valid image file found!\n");
		return nullptr;
	}

	return machine().rom_load().get_disk_handle("laserdisc");
}



/*************************************
 *
 *  Timers and sync
 *
 *************************************/

void ldplayer_state::process_commands()
{
	ioport_value controls = ioport("controls")->read();
	int number;

	// step backwards
	if (!(m_last_controls & 0x01) && (controls & 0x01))
		execute_command(CMD_STEP_REVERSE);

	// step forwards
	if (!(m_last_controls & 0x02) && (controls & 0x02))
		execute_command(CMD_STEP_FORWARD);

	// scan backwards
	if (controls & 0x04)
		execute_command(CMD_SCAN_REVERSE);

	// scan forwards
	if (controls & 0x08)
		execute_command(CMD_SCAN_FORWARD);

	// slow backwards
	if (!(m_last_controls & 0x10) && (controls & 0x10))
		execute_command(CMD_SLOW_REVERSE);

	// slow forwards
	if (!(m_last_controls & 0x20) && (controls & 0x20))
		execute_command(CMD_SLOW_FORWARD);

	// fast backwards
	if (controls & 0x40)
		execute_command(CMD_FAST_REVERSE);

	// fast forwards
	if (controls & 0x80)
		execute_command(CMD_FAST_FORWARD);

	// play/pause
	if (!(m_last_controls & 0x100) && (controls & 0x100))
	{
		m_playing = !m_playing;
		execute_command(m_playing ? CMD_PLAY : CMD_PAUSE);
	}

	// toggle frame display
	if (!(m_last_controls & 0x200) && (controls & 0x200))
		execute_command(CMD_FRAME_TOGGLE);

	// toggle chapter display
	if (!(m_last_controls & 0x400) && (controls & 0x400))
		execute_command(CMD_CHAPTER_TOGGLE);

	// toggle left channel
	if (!(m_last_controls & 0x800) && (controls & 0x800))
		execute_command(CMD_CH1_TOGGLE);

	// toggle right channel
	if (!(m_last_controls & 0x1000) && (controls & 0x1000))
		execute_command(CMD_CH2_TOGGLE);

	// numbers
	for (number = 0; number < 10; number++)
		if (!(m_last_controls & (0x10000 << number)) && (controls & (0x10000 << number)))
			execute_command(CMD_0 + number);

	// enter
	if (!(m_last_controls & 0x4000000) && (controls & 0x4000000))
		execute_command(CMD_SEARCH);

	m_last_controls = controls;
}


void ldplayer_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TIMER_ID_VSYNC_UPDATE:
		{
			// handle commands
			if (param == 0)
				process_commands();

			// set a timer to go off on the next VBLANK
			int vblank_scanline = m_screen->visible_area().max_y + 1;
			attotime target = m_screen->time_until_pos(vblank_scanline);
			timer_set(target, TIMER_ID_VSYNC_UPDATE);
			break;
		}

		case TIMER_ID_AUTOPLAY:
			// start playing
			execute_command(CMD_PLAY);
			m_playing = true;
			break;
	}
}


void ldplayer_state::machine_start()
{
	// start the vsync timer going
	timer_set(attotime::zero, TIMER_ID_VSYNC_UPDATE, 1);
}


void ldplayer_state::machine_reset()
{
	// set up a timer to start playing immediately
	timer_set(attotime::zero, TIMER_ID_AUTOPLAY);

	// indicate the name of the file we opened
	popmessage("Opened %s\n", m_filename);
}



/*************************************
 *
 *  PR-8210 implementation
 *
 *************************************/

void pr8210_state::add_command(uint8_t command)
{
	m_command_buffer[m_command_buffer_in++ % std::size(m_command_buffer)] = (command & 0x1f) | 0x20;
	m_command_buffer[m_command_buffer_in++ % std::size(m_command_buffer)] = 0x00 | 0x20;
}


void pr8210_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
		case TIMER_ID_BIT:
		{
			attotime duration = attotime::from_msec(30);
			uint8_t bitsleft = param >> 16;
			uint8_t data = param;

			// if we have bits, process
			if (bitsleft != 0)
			{
				// assert the line and set a timer for deassertion
				m_laserdisc->control_w(ASSERT_LINE);
				timer_set(attotime::from_usec(250), TIMER_ID_BIT_OFF);

				// space 0 bits apart by 1msec, and 1 bits by 2msec
				duration = attotime::from_msec((data & 0x80) ? 2 : 1);
				data <<= 1;
				bitsleft--;
			}

			// if we're out of bits, queue up the next command
			else if (bitsleft == 0 && m_command_buffer_in != m_command_buffer_out)
			{
				data = m_command_buffer[m_command_buffer_out++ % std::size(m_command_buffer)];
				bitsleft = 12;
			}
			m_bit_timer->adjust(duration, (bitsleft << 16) | data);
			break;
		}

		// deassert the control line
		case TIMER_ID_BIT_OFF:
			m_laserdisc->control_w(CLEAR_LINE);
			break;

		// others to the parent class
		default:
			ldplayer_state::device_timer(timer, id, param);
			break;
	}
}

void pr8210_state::machine_start()
{
	ldplayer_state::machine_start();
	m_bit_timer = timer_alloc(TIMER_ID_BIT);
}

void pr8210_state::machine_reset()
{
	ldplayer_state::machine_reset();
	m_bit_timer->adjust(attotime::zero);
}


void pr8210_state::execute_command(int command)
{
	static const uint8_t digits[10] = { 0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d, 0x03, 0x13 };

	switch (command)
	{
		case CMD_SCAN_REVERSE:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % std::size(m_command_buffer))
			{
				add_command(0x1c);
				m_playing = true;
			}
			break;

		case CMD_STEP_REVERSE:
			add_command(0x12);
			m_playing = false;
			break;

		case CMD_SLOW_REVERSE:
			add_command(0x02);
			m_playing = true;
			break;

		case CMD_FAST_REVERSE:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % std::size(m_command_buffer))
			{
				add_command(0x0c);
				m_playing = true;
			}
			break;

		case CMD_SCAN_FORWARD:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % std::size(m_command_buffer))
			{
				add_command(0x08);
				m_playing = true;
			}
			break;

		case CMD_STEP_FORWARD:
			add_command(0x04);
			m_playing = false;
			break;

		case CMD_SLOW_FORWARD:
			add_command(0x18);
			m_playing = true;
			break;

		case CMD_FAST_FORWARD:
			if (m_command_buffer_in == m_command_buffer_out ||
				m_command_buffer_in == (m_command_buffer_out + 1) % std::size(m_command_buffer))
			{
				add_command(0x10);
				m_playing = true;
			}
			break;

		case CMD_PLAY:
			add_command(0x14);
			m_playing = true;
			break;

		case CMD_PAUSE:
			add_command(0x0a);
			m_playing = false;
			break;

		case CMD_FRAME_TOGGLE:
			add_command(0x0b);
			break;

		case CMD_CHAPTER_TOGGLE:
			add_command(0x06);
			break;

		case CMD_CH1_TOGGLE:
			add_command(0x0e);
			break;

		case CMD_CH2_TOGGLE:
			add_command(0x16);
			break;

		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
			add_command(digits[command - CMD_0]);
			break;

		case CMD_SEARCH:
			add_command(0x1a);
			m_playing = false;
			break;
	}
}



/*************************************
 *
 *  LD-V1000 implementation
 *
 *************************************/

void ldv1000_state::execute_command(int command)
{
	static const uint8_t digits[10] = { 0x3f, 0x0f, 0x8f, 0x4f, 0x2f, 0xaf, 0x6f, 0x1f, 0x9f, 0x5f };
	switch (command)
	{
		case CMD_SCAN_REVERSE:
			m_laserdisc->data_w(0xf8);
			m_playing = true;
			break;

		case CMD_STEP_REVERSE:
			m_laserdisc->data_w(0xfe);
			m_playing = false;
			break;

		case CMD_SCAN_FORWARD:
			m_laserdisc->data_w(0xf0);
			m_playing = true;
			break;

		case CMD_STEP_FORWARD:
			m_laserdisc->data_w(0xf6);
			m_playing = false;
			break;

		case CMD_PLAY:
			m_laserdisc->data_w(0xfd);
			m_playing = true;
			break;

		case CMD_PAUSE:
			m_laserdisc->data_w(0xa0);
			m_playing = false;
			break;

		case CMD_FRAME_TOGGLE:
			m_laserdisc->data_w(0xf1);
			break;

		case CMD_0:
		case CMD_1:
		case CMD_2:
		case CMD_3:
		case CMD_4:
		case CMD_5:
		case CMD_6:
		case CMD_7:
		case CMD_8:
		case CMD_9:
			m_laserdisc->data_w(digits[command - CMD_0]);
			break;

		case CMD_SEARCH:
			m_laserdisc->data_w(0xf7);
			m_playing = false;
			break;
	}
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( ldplayer )
	PORT_START("controls")
	PORT_BIT( 0x0000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Step reverse") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x0000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Step forward") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x0000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Scan reverse") PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x0000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Scan forward") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x0000010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Slow reverse") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x0000020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Slow forward") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x0000040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Fast reverse") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x0000080, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Fast forward") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x0000100, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Play/Pause") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x0000200, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Toggle frame display") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x0000400, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle chapter display") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0000800, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle left channel") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x0001000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Toggle right channel") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x0010000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("0") PORT_PLAYER(2) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x0020000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("1") PORT_PLAYER(2) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x0040000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("2") PORT_PLAYER(2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x0080000, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("3") PORT_PLAYER(2) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x0100000, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("4") PORT_PLAYER(2) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x0200000, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("5") PORT_PLAYER(2) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x0400000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("6") PORT_PLAYER(2) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x0800000, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("7") PORT_PLAYER(2) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x1000000, IP_ACTIVE_HIGH, IPT_BUTTON9 ) PORT_NAME("8") PORT_PLAYER(2) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x2000000, IP_ACTIVE_HIGH, IPT_BUTTON10 ) PORT_NAME("9") PORT_PLAYER(2) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x4000000, IP_ACTIVE_HIGH, IPT_BUTTON11 ) PORT_NAME("Enter") PORT_PLAYER(2) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void ldplayer_state::ldplayer_ntsc(machine_config &config)
{
}


void ldv1000_state::ldv1000(machine_config &config)
{
	ldplayer_ntsc(config);
	pioneer_ldv1000_device &laserdisc(PIONEER_LDV1000(config, "laserdisc"));
	laserdisc.set_get_disc(FUNC(ldv1000_state::get_disc));
	laserdisc.add_ntsc_screen(config, "screen");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	laserdisc.add_route(0, "lspeaker", 1.0);
	laserdisc.add_route(1, "rspeaker", 1.0);
}


void pr8210_state::pr8210(machine_config &config)
{
	ldplayer_ntsc(config);
	pioneer_pr8210_device &laserdisc(PIONEER_PR8210(config, "laserdisc"));
	laserdisc.set_get_disc(FUNC(pr8210_state::get_disc));
	laserdisc.add_ntsc_screen(config, "screen");

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	laserdisc.add_route(0, "lspeaker", 1.0);
	laserdisc.add_route(1, "rspeaker", 1.0);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( simldv1000 )
	DISK_REGION( "laserdisc" )
ROM_END


ROM_START( simpr8210 )
	DISK_REGION( "laserdisc" )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 2008, simldv1000, 0, ldv1000, ldplayer, ldv1000_state, empty_init, ROT0, "MAME", "Pioneer LDV-1000 Simulator", 0 )
GAMEL(2008, simpr8210,  0, pr8210,  ldplayer, pr8210_state,  empty_init, ROT0, "MAME", "Pioneer PR-8210 Simulator",  0, layout_pr8210 )
