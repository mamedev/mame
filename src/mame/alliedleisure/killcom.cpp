// license:BSD-3-Clause
// copyright-holders:Chris Moore
/***************************************************************************

Killer Comet hardware, by Centuri (formerly known as Allied Leisure)
driver by Chris Moore

Killer Comet memory map

MAIN CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
00000-xxxxxxxxxx R/W xxxxxxxx RAM       can be either 256 bytes (2x2101) or 1kB (2x2114)
00001-----------              n.c.
00010-----------              n.c.
00011-----------              n.c.
00100-------xxxx R/W xxxxxxxx VIA 1     6522 for video interface
00101-------xxxx R/W xxxxxxxx VIA 2     6522 for I/O interface
00110-------xxxx R/W xxxxxxxx VIA 3     6522 for interface with audio CPU
00111-----------              n.c.
01--------------              n.c.
10--------------              n.c.
11000xxxxxxxxxxx R   xxxxxxxx ROM E2    program ROM
11001xxxxxxxxxxx R   xxxxxxxx ROM F2    program ROM
11010xxxxxxxxxxx R   xxxxxxxx ROM G2    program ROM
11011xxxxxxxxxxx R   xxxxxxxx ROM J2    program ROM
11100xxxxxxxxxxx R   xxxxxxxx ROM J1    program ROM
11101xxxxxxxxxxx R   xxxxxxxx ROM G1    program ROM
11110xxxxxxxxxxx R   xxxxxxxx ROM F1    program ROM
11111xxxxxxxxxxx R   xxxxxxxx ROM E1    program ROM


SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000-0----xxxxxxx R/W xxxxxxxx VIA 5     6532 internal RAM
000-1------xxxxx R/W xxxxxxxx VIA 5     6532 for interface with main CPU
001-------------              n.c.
010-------------              n.c.
011-------------              n.c.
100-------------              n.c.
101-----------xx R/W xxxxxxxx PSG 1     AY-3-8910
110-------------              n.c.
111--xxxxxxxxxxx R   xxxxxxxx ROM E1


Notes:
- There are two dip switch banks connected to the 8910 ports. They are only
  used for testing.

- Megatack's test mode reports the same fire buttons as Killer Comet, but this
  is wrong: there is only one fire button, not three.

- Megatack's actual name which displays proudly on the cover and everywhere in
  the manual as "MEGATTACK"

- Checked and verified DIPs from manuals and service mode for:
  Challenger
  Kaos
  Killer Comet
  Megattack
  Pot Of Gold (Leprechaun)

- Leprechaun and Pirate Treasure were made for the Moppet Video arcade series
  (Enter-Tech arcade cabs aimed at younger children). That's why the games are
  so easy.


TODO:
- The board has, instead of a watchdog, a timed reset that has to be disabled
  on startup. The disable line is tied to CA2 of VIA2, but I don't see writes
  to that pin in the log. Missing support in machine/6522via.cpp?
- Investigate and document the 8910 dip switches
- Fix the input ports of Kaos
- Some of the games do unmapped reads all over the place, probably just bad
  programming. One of these is piratetr, and it will lock up eventually when
  reading from VIA1. It's possible to fix with a simple wire mod/pcb cut trace,
  like done with piratetr_main_map. Let's assume they did something like that.

BTANB:
- BGM stops in leprechn when you touch a tree

****************************************************************************/

#include "emu.h"
#include "killcom.h"

#include "cpu/m6502/m6502.h"
#include "machine/input_merger.h"
#include "speaker.h"



/*************************************
 *
 *  Initialization
 *
 *************************************/

void killcom_state::machine_start()
{
	// register for save states
	save_item(NAME(m_current_port));
	save_item(NAME(m_audio_reset));
}

void killcom_state::machine_reset()
{
	m_video_x = 0;
	m_video_y = 0;
	m_video_previous = 0;
}


void killcom_state::video_start()
{
	m_videoram = make_unique_clear<uint8_t[]>(0x10000);
	save_pointer(NAME(m_videoram), 0x10000);

	m_hblank_timer[0] = timer_alloc(FUNC(killcom_state::hblank_callback), this);
	m_hblank_timer[0]->adjust(attotime::zero, 0, m_screen->scan_period());

	m_hblank_timer[1] = timer_alloc(FUNC(killcom_state::hblank_callback), this);
	m_hblank_timer[1]->adjust(256 * m_screen->pixel_period(), 1, m_screen->scan_period());

	// register for save states
	save_item(NAME(m_video_x));
	save_item(NAME(m_video_y));
	save_item(NAME(m_video_command));
	save_item(NAME(m_video_command_trigger));
	save_item(NAME(m_video_data));
	save_item(NAME(m_video_previous));
}



/*************************************
 *
 *  Screen update
 *
 *************************************/

uint32_t killcom_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bitmap.pix(y, x) = m_videoram[y << 8 | x] & 0x07;
		}
	}

	return 0;
}



/*************************************
 *
 *  VIA 1 - video
 *
 *************************************/

TIMER_CALLBACK_MEMBER(killcom_state::hblank_callback)
{
	// screen HBLANK is tied to VIA1 PB6
	m_via[0]->write_pb6(param);
}


void killcom_state::video_data_w(uint8_t data)
{
	m_video_data = data;
}


void killcom_state::video_command_w(uint8_t data)
{
	m_video_command = data & 0x07;
}


uint8_t killcom_state::video_status_r()
{
	// video busy flags on the upper bits
	return 0xff;
}


uint8_t killcom_state::leprechn_videoram_r()
{
	return (video_status_r() & 0xf8) | m_video_previous;
}


void killcom_state::video_command_trigger_w(int state)
{
	if (state && !m_video_command_trigger)
	{
		switch (m_video_command)
		{
		// draw pixel
		case 0:
			// auto-adjust X?
			if (m_video_data & 0x10)
			{
				if (m_video_data & 0x40)
					m_video_x = m_video_x - 1;
				else
					m_video_x = m_video_x + 1;
			}

			// auto-adjust Y?
			if (m_video_data & 0x20)
			{
				if (m_video_data & 0x80)
					m_video_y = m_video_y - 1;
				else
					m_video_y = m_video_y + 1;
			}

			m_video_previous = m_videoram[m_video_y << 8 | m_video_x];

			m_screen->update_now();
			m_videoram[m_video_y << 8 | m_video_x] = m_video_data & 0x07;

			break;

		// load X register
		case 1:
			m_video_x = m_video_data;
			break;

		// load Y register
		case 2:
			m_video_y = m_video_data;
			break;

		// clear screen
		case 3:
			m_screen->update_now();
			memset(m_videoram.get(), m_video_data & 0x07, 0x10000);
			break;

		// N/C
		default:
			break;
		}
	}

	m_video_command_trigger = state;
}



/*************************************
 *
 *  VIA 2 - I/O
 *
 *************************************/

void killcom_state::io_select_w(uint8_t data)
{
	m_current_port = data;
}


uint8_t killcom_state::io_port_r()
{
	uint8_t data = 0xff;

	// COL A-D (E,F are N/C)
	for (int i = 0; i < 4; i++)
		if (BIT(m_current_port, i))
			data &= m_inputs[i]->read();

	// DS A,B
	for (int i = 0; i < 2; i++)
		if (BIT(m_current_port, i ^ 7))
			data &= m_dsw[i]->read();

	return data;
}


void killcom_state::coin_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, ~state & 1);
}



/*************************************
 *
 *  VIA 3 - audio
 *
 *************************************/

void killcom_state::audio_cmd_w_sync(int32_t param)
{
	m_riot->pa_w(0, param, 0x7f);
}


void killcom_state::audio_trigger_w_sync(int32_t param)
{
	m_riot->pa_bit_w<7>(param);
}


void killcom_state::audio_reset_w_sync(int32_t param)
{
	if (param && !m_audio_reset)
	{
		m_ay->reset();
		m_riot->reset();
	}

	m_audio_reset = param;
	m_audiocpu->set_input_line(INPUT_LINE_RESET, param ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void killcom_state::killcom_main_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x0400).ram();
	map(0x2000, 0x200f).mirror(0x07f0).m(m_via[0], FUNC(via6522_device::map)); // VIA 1
	map(0x2800, 0x280f).mirror(0x07f0).m(m_via[1], FUNC(via6522_device::map)); // VIA 2
	map(0x3000, 0x300f).mirror(0x07f0).m(m_via[2], FUNC(via6522_device::map)); // VIA 3
	map(0x8000, 0xffff).rom();
}

void killcom_state::piratetr_main_map(address_map &map)
{
	killcom_main_map(map);
	map(0x2010, 0x201f).mirror(0x07e0).unmaprw(); // A4, see TODO
}



/*************************************
 *
 *  Audio CPU memory handlers
 *
 *************************************/

void killcom_state::killcom_audio_map(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x1780).m(m_riot, FUNC(mos6532_device::ram_map));
	map(0x0800, 0x081f).mirror(0x17e0).m(m_riot, FUNC(mos6532_device::io_map));
	map(0xa000, 0xa000).mirror(0x1ffc).w(m_ay, FUNC(ay8910_device::address_w));
	map(0xa001, 0xa001).mirror(0x1ffc).r(m_ay, FUNC(ay8910_device::data_r));
	map(0xa002, 0xa002).mirror(0x1ffc).w(m_ay, FUNC(ay8910_device::data_w));
	map(0xe000, 0xe7ff).mirror(0x1800).rom();
}

void killcom_state::leprechn_audio_map(address_map &map)
{
	killcom_audio_map(map);
	map(0xe000, 0xefff).mirror(0x1000).rom();
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( killcom )
	PORT_START("IN0")   /* COL. A - from "TEST NO.7 - status locator - coin-door" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Do Tests") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Select Test") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")   /* COL. B - from "TEST NO.7 - status locator - start sws." */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")   /* COL. C - from "TEST NO.8 - status locator - player no.1" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN3")   /* COL. D - from "TEST NO.8 - status locator - player no.2" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL

	PORT_START("DSW0")  /* DSW A - from "TEST NO.6 - dip switch A" */
	PORT_DIPNAME( 0x03, 0x03, "Coinage P1/P2" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "1 Credit/2 Credits" )
	PORT_DIPSETTING(    0x02, "2 Credits/3 Credits" )
	PORT_DIPSETTING(    0x01, "2 Credits/4 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0xc0, 0xc0, "Reaction" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "Slowest" )
	PORT_DIPSETTING(    0x80, "Slow" )
	PORT_DIPSETTING(    0x40, "Fast" )
	PORT_DIPSETTING(    0x00, "Fastest" )

	PORT_START("DSW1")  /* DSW B - from "TEST NO.6 - dip switch B" */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")  /* audio board DSW A */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("DSW3")  /* audio board DSW B */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( megatack )
	PORT_START("IN0")   /* COL. A - from "TEST NO.7 - status locator - coin-door" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Do Tests") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Select Test") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")   /* COL. B - from "TEST NO.7 - status locator - start sws." */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")   /* COL. C - from "TEST NO.8 - status locator - player no.1" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* COL. D - from "TEST NO.8 - status locator - player no.2" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")  /* DSW A - from "TEST NO.6 - dip switch A" */
	PORT_DIPNAME( 0x03, 0x03, "Coinage P1/P2" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "1 Credit/2 Credits" )
	PORT_DIPSETTING(    0x02, "2 Credits/3 Credits" )
	PORT_DIPSETTING(    0x01, "2 Credits/4 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW1")  /* DSW B - from "TEST NO.6 - dip switch B" */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, "20000" )
	PORT_DIPSETTING(    0x06, "30000" )
	PORT_DIPSETTING(    0x05, "40000" )
	PORT_DIPSETTING(    0x04, "50000" )
	PORT_DIPSETTING(    0x03, "60000" )
	PORT_DIPSETTING(    0x02, "70000" )
	PORT_DIPSETTING(    0x01, "80000" )
	PORT_DIPSETTING(    0x00, "90000" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPNAME( 0x10, 0x10, "Monitor View" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Direct" )
	PORT_DIPSETTING(    0x00, "Mirror" )
	PORT_DIPNAME( 0x20, 0x20, "Monitor Orientation" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")  /* audio board DSW A */
	PORT_DIPNAME( 0x01, 0x00, "Sound Test A 0" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Sound Test A 1" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Sound Test A 2" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Sound Test A 3" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Sound Test A 4" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound Test A 5" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Test A 6" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Sound Test Enable" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* audio board DSW B */
	PORT_DIPNAME( 0x01, 0x00, "Sound Test B 0" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Sound Test B 1" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Sound Test B 2" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Sound Test B 3" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Sound Test B 4" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound Test B 5" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Test B 6" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Sound Test B 7" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( challeng )
	PORT_START("IN0")   /* COL. A - from "TEST NO.7 - status locator - coin-door" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Do Tests") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Select Test") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")   /* COL. B - from "TEST NO.7 - status locator - start sws." */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")   /* COL. C - from "TEST NO.8 - status locator - player no.1" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* COL. D - from "TEST NO.8 - status locator - player no.2" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")  /* DSW A - from "TEST NO.6 - dip switch A" */
	PORT_DIPNAME( 0x03, 0x03, "Coinage P1/P2" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "1 Credit/2 Credits" )
	PORT_DIPSETTING(    0x02, "2 Credits/3 Credits" )
	PORT_DIPSETTING(    0x01, "2 Credits/4 Credits" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "6" )

	// Manual states information which differs from actual settings for DSW1
	// Switches 4 & 5 are factory settings and remain in the OFF position.
	// Switches 6 & 7 are factory settings which remain in the ON position.
	PORT_START("DSW1")  /* DSW B - from "TEST NO.6 - dip switch B" */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x07, "40000" )
	PORT_DIPSETTING(    0x06, "50000" )
	PORT_DIPSETTING(    0x05, "60000" )
	PORT_DIPSETTING(    0x04, "70000" )
	PORT_DIPSETTING(    0x03, "80000" )
	PORT_DIPSETTING(    0x02, "90000" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPNAME( 0x10, 0x10, "Monitor View" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Direct" )
	PORT_DIPSETTING(    0x00, "Mirror" )
	PORT_DIPNAME( 0x20, 0x00, "Monitor Orientation" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Horizontal" )
	PORT_DIPSETTING(    0x00, "Vertical" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")  /* audio board DSW A */
	PORT_DIPNAME( 0x01, 0x00, "Sound Test A 0" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Sound Test A 1" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Sound Test A 2" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Sound Test A 3" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Sound Test A 4" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound Test A 5" ) PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Test A 6" ) PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Sound Test Enable" ) PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* audio board DSW B */
	PORT_DIPNAME( 0x01, 0x00, "Sound Test B 0" ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Sound Test B 1" ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Sound Test B 2" ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Sound Test B 3" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Sound Test B 4" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Sound Test B 5" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Test B 6" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Sound Test B 7" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( kaos )
	PORT_START("IN0")   /* COL. A - from "TEST NO.7 - status locator - coin-door" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Do Tests") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Select Test") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")   /* COL. B - from "TEST NO.7 - status locator - start sws." */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")   /* COL. C - from "TEST NO.8 - status locator - player no.1" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")   /* COL. D - from "TEST NO.8 - status locator - player no.2" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0f, 0x0e, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_10C ) )
	PORT_DIPSETTING(    0x04, "1 Coin/11 Credits" )
	PORT_DIPSETTING(    0x03, "1 Coin/12 Credits" )
	PORT_DIPSETTING(    0x02, "1 Coin/13 Credits" )
	PORT_DIPSETTING(    0x01, "1 Coin/14 Credits" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Max Credits" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x20, "30" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x02, 0x00, "Speed" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x02, "Fast" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "No Bonus" )
	PORT_DIPSETTING(    0x08, "10k" )
	PORT_DIPSETTING(    0x04, "10k 30k" )
	PORT_DIPSETTING(    0x00, "10k 30k 60k" )
	PORT_DIPNAME( 0x10, 0x10, "Number of $" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "12" )
	PORT_DIPNAME( 0x20, 0x00, "Bonus erg" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Every other screen" )
	PORT_DIPSETTING(    0x00, "Every screen" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")  /* audio board DSW A */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("DSW3")  /* audio board DSW B */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( leprechn )
	PORT_START("IN0")   /* COL. A - from "TEST NO.7 - status locator - coin-door" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Do Tests") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Select Test") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")   /* COL. B - from "TEST NO.7 - status locator - start sws." */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")   /* COL. C - from "TEST NO.8 - status locator - player no.1" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN3")   /* COL. D - from "TEST NO.8 - status locator - player no.2" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL

	PORT_START("DSW0")  /* DSW A - from "TEST NO.6 - dip switch A" */
	PORT_DIPNAME( 0x09, 0x09, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:1,4")
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x22, 0x22, "Max Credits" ) PORT_DIPLOCATION("SW1:2,6")
	PORT_DIPSETTING(    0x22, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x02, "30" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )

	PORT_START("DSW1")  /* DSW B - from "TEST NO.6 - dip switch B" */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x80, "60000" )
	PORT_DIPSETTING(    0x00, "90000" )
	PORT_DIPSETTING(    0xc0, DEF_STR( None ) )

	PORT_START("DSW2")  /* audio board DSW A */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("DSW3")  /* audio board DSW B */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( leprechna )
	PORT_INCLUDE(leprechn)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END


static INPUT_PORTS_START( piratetr )
	PORT_START("IN0")   /* COL. A - from "TEST NO.7 - status locator - coin-door" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Do Tests") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Select Test") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 )

	PORT_START("IN1")   /* COL. B - from "TEST NO.7 - status locator - start sws." */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")   /* COL. C - from "TEST NO.8 - status locator - player no.1" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )

	PORT_START("IN3")   /* COL. D - from "TEST NO.8 - status locator - player no.2" */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL

	PORT_START("DSW0")  /* DSW A - from "TEST NO.6 - dip switch A" */
	PORT_DIPNAME( 0x09, 0x09, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:1,4")
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x22, 0x22, "Max Credits" ) PORT_DIPLOCATION("SW1:2,6")
	PORT_DIPSETTING(    0x22, "10" )
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x02, "30" )
	PORT_DIPSETTING(    0x00, "40" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )

	PORT_START("DSW1")  /* DSW B - from "TEST NO.6 - dip switch B" */
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Stringing Check" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPNAME( 0xc0, 0x40, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x40, "30000" )
	PORT_DIPSETTING(    0x80, "60000" )
	PORT_DIPSETTING(    0x00, "90000" )
	PORT_DIPSETTING(    0xc0, DEF_STR( None ) )

	PORT_START("DSW2")  /* audio board DSW A */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW3:8" )

	PORT_START("DSW3")  /* audio board DSW B */
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW4:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW4:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW4:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW4:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW4:8" )
INPUT_PORTS_END



/*************************************
 *
 *  Machine configs
 *
 *************************************/

void killcom_state::killcom_video(machine_config &config)
{
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(11.6688_MHz_XTAL / 2, 352, 0, 256, 280, 0, 256);
	m_screen->set_screen_update(FUNC(killcom_state::screen_update));
	m_screen->screen_vblank().set(m_via[0], FUNC(via6522_device::write_ca1)); // VBLANK is connected to CA1
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_3BIT);
}

void killcom_state::killcom(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 3.579545_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &killcom_state::killcom_main_map);

	input_merger_device &main_irqs(INPUT_MERGER_ANY_HIGH(config, "main_irqs"));
	main_irqs.output_handler().set_inputline(m_maincpu, 0);

	M6502(config, m_audiocpu, 3.579545_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &killcom_state::killcom_audio_map);

	MOS6532(config, m_riot, 3.579545_MHz_XTAL / 4);
	m_riot->pb_wr_callback().set(m_via[2], FUNC(via6522_device::write_pb));
	m_riot->irq_wr_callback().set_inputline(m_audiocpu, 0);

	// via
	MOS6522(config, m_via[0], 3.579545_MHz_XTAL / 4);
	m_via[0]->writepa_handler().set(FUNC(killcom_state::video_data_w));
	m_via[0]->writepb_handler().set(FUNC(killcom_state::video_command_w));
	m_via[0]->readpb_handler().set(FUNC(killcom_state::video_status_r));
	m_via[0]->ca2_handler().set(FUNC(killcom_state::video_command_trigger_w));
	m_via[0]->irq_handler().set("main_irqs", FUNC(input_merger_device::in_w<0>));

	MOS6522(config, m_via[1], 3.579545_MHz_XTAL / 4);
	m_via[1]->readpa_handler().set(FUNC(killcom_state::io_port_r));
	m_via[1]->writepb_handler().set(FUNC(killcom_state::io_select_w));
	m_via[1]->cb2_handler().set(FUNC(killcom_state::coin_w));
	m_via[1]->irq_handler().set("main_irqs", FUNC(input_merger_device::in_w<1>));

	MOS6522(config, m_via[2], 3.579545_MHz_XTAL / 4);
	m_via[2]->writepa_handler().set(FUNC(killcom_state::audio_cmd_w));
	m_via[2]->ca2_handler().set(FUNC(killcom_state::audio_trigger_w));
	m_via[2]->cb2_handler().set(FUNC(killcom_state::audio_reset_w));
	m_via[2]->irq_handler().set("main_irqs", FUNC(input_merger_device::in_w<2>));

	// video hardware
	killcom_video(config);

	// audio hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay, 3.579545_MHz_XTAL / 2);
	m_ay->port_a_read_callback().set_ioport("DSW2");
	m_ay->port_b_read_callback().set_ioport("DSW3");
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.33);
}

void killcom_state::leprechn(machine_config &config)
{
	killcom(config);

	// basic machine hardware
	m_maincpu->set_clock(4_MHz_XTAL / 4);
	m_via[0]->set_clock(4_MHz_XTAL / 4);
	m_via[1]->set_clock(4_MHz_XTAL / 4);
	m_via[2]->set_clock(4_MHz_XTAL / 4);

	m_audiocpu->set_clock(4_MHz_XTAL / 4);
	m_riot->set_clock(4_MHz_XTAL / 4);
	m_ay->set_clock(4_MHz_XTAL / 2);

	m_audiocpu->set_addrmap(AS_PROGRAM, &killcom_state::leprechn_audio_map);

	// via
	m_via[0]->readpb_handler().set(FUNC(killcom_state::leprechn_videoram_r));
	m_via[0]->writepb_handler().set(FUNC(killcom_state::video_command_w)).rshift(3);
}

void killcom_state::piratetr(machine_config &config)
{
	leprechn(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &killcom_state::piratetr_main_map);
}



/*************************************
 *
 *  ROM defintions
 *
 *************************************/

ROM_START( killcom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "killcom.e2",   0xc000, 0x0800, CRC(a01cbb9a) SHA1(a8769243adbdddedfda5f3c8f054e9281a0eca46) )
	ROM_LOAD( "killcom.f2",   0xc800, 0x0800, CRC(bb3b4a93) SHA1(a0ea61ac30a4d191db619b7bfb697914e1500036) )
	ROM_LOAD( "killcom.g2",   0xd000, 0x0800, CRC(86ec68b2) SHA1(a09238190d61684d943ce0acda25eb901d2580ac) )
	ROM_LOAD( "killcom.j2",   0xd800, 0x0800, CRC(28d8c6a1) SHA1(d9003410a651221e608c0dd20d4c9c60c3b0febc) )
	ROM_LOAD( "killcom.j1",   0xe000, 0x0800, CRC(33ef5ac5) SHA1(42f839ad295d3df457ced7886a0003eff7e6c4ae) )
	ROM_LOAD( "killcom.g1",   0xe800, 0x0800, CRC(49cb13e2) SHA1(635e4665042ddd9b8c0b9f275d4bcc6830dc6a98) )
	ROM_LOAD( "killcom.f1",   0xf000, 0x0800, CRC(ef652762) SHA1(414714e5a3f83916bd3ae54afe2cb2271ee9008b) )
	ROM_LOAD( "killcom.e1",   0xf800, 0x0800, CRC(bc19dcb7) SHA1(eb983d2df010c12cb3ffb584fceafa54a4e956b3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "killsnd.e1",   0xe000, 0x0800, CRC(77d4890d) SHA1(a3ed7e11dec5d404f022c521256ff50aa6940d3c) )
ROM_END

ROM_START( megatack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "megattac.e2",  0xc000, 0x0800, CRC(33fa5104) SHA1(15693eb540563e03502b53ed8a83366e395ca529) )
	ROM_LOAD( "megattac.f2",  0xc800, 0x0800, CRC(af5e96b1) SHA1(5f6ab47c12d051f6af446b08f3cd459fbd2c13bf) )
	ROM_LOAD( "megattac.g2",  0xd000, 0x0800, CRC(670103ea) SHA1(e11f01e8843ed918c6ea5dda75319dc95105d345) )
	ROM_LOAD( "megattac.j2",  0xd800, 0x0800, CRC(4573b798) SHA1(388db11ab114b3575fe26ed65bbf49174021939a) )
	ROM_LOAD( "megattac.j1",  0xe000, 0x0800, CRC(3b1d01a1) SHA1(30bbf51885b1e510b8d21cdd82244a455c5dada0) )
	ROM_LOAD( "megattac.g1",  0xe800, 0x0800, CRC(eed75ef4) SHA1(7c02337344f2716d2f2771229f7dee7b651eeb95) )
	ROM_LOAD( "megattac.f1",  0xf000, 0x0800, CRC(c93a8ed4) SHA1(c87e2f13f2cc00055f4941c272a3126b165a6252) )
	ROM_LOAD( "megattac.e1",  0xf800, 0x0800, CRC(d9996b9f) SHA1(e71d65b695000fdfd5fd1ce9ae39c0cb0b61669e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "megatsnd.e1",  0xe000, 0x0800, CRC(0c186bdb) SHA1(233af9481a3979971f2d5aa75ec8df4333aa5e0d) )
ROM_END

ROM_START( megatacka ) // original Centuri PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "meg-e2.e2",    0xc000, 0x0800, CRC(9664c7b1) SHA1(356e7f5f3b2a9b829fac53e7bf9193278b4de2ed) )
	ROM_LOAD( "meg-f2.f2",    0xc800, 0x0800, CRC(67c42523) SHA1(f9fc88cdea05a2d0e89e3ba9b545bf3476b37d2d) )
	ROM_LOAD( "meg-g2.g2",    0xd000, 0x0800, CRC(71f36604) SHA1(043988126343b6224e8e1d6c0dbba6b6b08fe493) )
	ROM_LOAD( "meg-j2.j2",    0xd800, 0x0800, CRC(4ddcc145) SHA1(3a6d42a58c388eaaf6561351fa98936d98975e0b) )
	ROM_LOAD( "meg-j1.j1",    0xe000, 0x0800, CRC(911d5d9a) SHA1(92bfe0f69a6e563363df59ebee745d7b3cfc0141) )
	ROM_LOAD( "meg-g1.g1",    0xe800, 0x0800, CRC(22a51c9b) SHA1(556e09216ed85eaf3870f85515c273c7eb1ab13a) )
	ROM_LOAD( "meg-f1.f1",    0xf000, 0x0800, CRC(2ffa51ac) SHA1(7c5d8295c5e71a9918a02d203139b024bd3bf8f4) )
	ROM_LOAD( "meg-e1.e1",    0xf800, 0x0800, CRC(01dbe4ad) SHA1(af72778ae112f24a92fb3007bb456331c3896b50) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "megatsnd.e1",  0xe000, 0x0800, CRC(0c186bdb) SHA1(233af9481a3979971f2d5aa75ec8df4333aa5e0d) ) // missing for this board, using the one from the parent
ROM_END

ROM_START( challeng )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "chall.6",      0xa000, 0x1000, CRC(b30fe7f5) SHA1(ce93a57d626f90d31ddedbc35135f70758949dfa) )
	ROM_LOAD( "chall.5",      0xb000, 0x1000, CRC(34c6a88e) SHA1(250577e2c8eb1d3a78cac679310ec38924ac1fe0) )
	ROM_LOAD( "chall.4",      0xc000, 0x1000, CRC(0ddc18ef) SHA1(9f1aa27c71d7e7533bddf7de420c06fb0058cf60) )
	ROM_LOAD( "chall.3",      0xd000, 0x1000, CRC(6ce03312) SHA1(69c047f501f327f568fe4ad1274168f9dda1ca70) )
	ROM_LOAD( "chall.2",      0xe000, 0x1000, CRC(948912ad) SHA1(e79738ab94501f858f4d5f218787267523611e92) )
	ROM_LOAD( "chall.1",      0xf000, 0x1000, CRC(7c71a9dc) SHA1(2530ada6390fb46c44bf7bf2636910ee54786493) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "chall.snd",    0xe000, 0x0800, CRC(1b2bffd2) SHA1(36ceb5abbc92a17576c375019f1c5900320398f9) )
ROM_END

ROM_START( kaos )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kaosab.g2",    0x9000, 0x0800, CRC(b23d858f) SHA1(e31fa657ace34130211a0b9fc0d115fd89bb20dd) )
	ROM_CONTINUE(             0xd000, 0x0800 )
	ROM_LOAD( "kaosab.j2",    0x9800, 0x0800, CRC(4861e5dc) SHA1(96ca0b8625af3897bd4a50a45ea964715f9e4973) )
	ROM_CONTINUE(             0xd800, 0x0800 )
	ROM_LOAD( "kaosab.j1",    0xa000, 0x0800, CRC(e055db3f) SHA1(099176629723c1a9bdc59f440339b2e8c38c3261) )
	ROM_CONTINUE(             0xe000, 0x0800 )
	ROM_LOAD( "kaosab.g1",    0xa800, 0x0800, CRC(35d7c467) SHA1(6d5bfd29ff7b96fed4b24c899ddd380e47e52bc5) )
	ROM_CONTINUE(             0xe800, 0x0800 )
	ROM_LOAD( "kaosab.f1",    0xb000, 0x0800, CRC(995b9260) SHA1(580896aa8b6f0618dc532a12d0795b0d03f7cadd) )
	ROM_CONTINUE(             0xf000, 0x0800 )
	ROM_LOAD( "kaosab.e1",    0xb800, 0x0800, CRC(3da5202a) SHA1(6b5aaf44377415763aa0895c64765a4b82086f25) )
	ROM_CONTINUE(             0xf800, 0x0800 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kaossnd.e1",   0xe000, 0x0800, CRC(ab23d52a) SHA1(505f3e4a56e78a3913010f5484891f01c9831480) )
ROM_END

ROM_START( leprechn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.u13",        0x8000, 0x1000, CRC(2c4a46ca) SHA1(28a157c1514bc9f27cc27baddb83cf1a1887f3d1) )
	ROM_LOAD( "2.u14",        0x9000, 0x1000, CRC(6ed26b3e) SHA1(4ee5d09200d9e8f94ae29751c8ee838faa268f15) )
	ROM_LOAD( "3.u15",        0xa000, 0x1000, CRC(b5f79fd8) SHA1(271f7b55ecda5bb99f40687264256b82649e2141) )
	ROM_LOAD( "4.u16",        0xb000, 0x1000, CRC(6c12a065) SHA1(2acae6a5b94cbdcc550cee88a7be9254fdae908c) )
	ROM_LOAD( "5.u17",        0xc000, 0x1000, CRC(21ddb539) SHA1(b4dd0a1916adc076fa6084c315459fcb2522161e) )
	ROM_LOAD( "6.u18",        0xd000, 0x1000, CRC(03c34dce) SHA1(6dff202e1a3d0643050f3287f6b5906613d56511) )
	ROM_LOAD( "7.u19",        0xe000, 0x1000, CRC(7e06d56d) SHA1(5f68f2047969d803b752a4cd02e0e0af916c8358) )
	ROM_LOAD( "8.u20",        0xf000, 0x1000, CRC(097ede60) SHA1(5509c41167c066fa4e7f4f4bd1ce9cd00773a82c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "lepsound",     0xe000, 0x1000, CRC(6651e294) SHA1(ce2875fc4df61a30d51d3bf2153864b562601151) )
ROM_END

ROM_START( leprechna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lep1.u13",     0x8000, 0x1000, CRC(2c4a46ca) SHA1(28a157c1514bc9f27cc27baddb83cf1a1887f3d1) )
	ROM_LOAD( "lep2.u14",     0x9000, 0x1000, CRC(6ed26b3e) SHA1(4ee5d09200d9e8f94ae29751c8ee838faa268f15) )
	ROM_LOAD( "lep3.u15",     0xa000, 0x1000, CRC(a2eaa016) SHA1(be992ee787766137fd800ec59529c98ef2e6991e) )
	ROM_LOAD( "lep4.u16",     0xb000, 0x1000, CRC(6c12a065) SHA1(2acae6a5b94cbdcc550cee88a7be9254fdae908c) )
	ROM_LOAD( "lep5.u17",     0xc000, 0x1000, CRC(21ddb539) SHA1(b4dd0a1916adc076fa6084c315459fcb2522161e) )
	ROM_LOAD( "lep6.u18",     0xd000, 0x1000, CRC(03c34dce) SHA1(6dff202e1a3d0643050f3287f6b5906613d56511) )
	ROM_LOAD( "lep7.u19",     0xe000, 0x1000, CRC(7e06d56d) SHA1(5f68f2047969d803b752a4cd02e0e0af916c8358) )
	ROM_LOAD( "lep8.u20",     0xf000, 0x1000, CRC(097ede60) SHA1(5509c41167c066fa4e7f4f4bd1ce9cd00773a82c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "lepsound",     0xe000, 0x1000, CRC(6651e294) SHA1(ce2875fc4df61a30d51d3bf2153864b562601151) )
ROM_END

ROM_START( potogold )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pog.pg1",      0x8000, 0x1000, CRC(9f1dbda6) SHA1(baf20e9a0793c0f1529396f95a820bd1f9431465) )
	ROM_LOAD( "pog.pg2",      0x9000, 0x1000, CRC(a70e3811) SHA1(7ee306dc7d75a7d3fd497870ec92bef9d86535e9) )
	ROM_LOAD( "pog.pg3",      0xa000, 0x1000, CRC(81cfb516) SHA1(12732707e2a51ec39563f2d1e898cc567ab688f0) )
	ROM_LOAD( "pog.pg4",      0xb000, 0x1000, CRC(d61b1f33) SHA1(da024c0776214b8b5a3e49401c4110e86a1bead1) )
	ROM_LOAD( "pog.pg5",      0xc000, 0x1000, CRC(eee7597e) SHA1(9b5cd293580c5d212f8bf39286070280d55e4cb3) )
	ROM_LOAD( "pog.pg6",      0xd000, 0x1000, CRC(25e682bc) SHA1(085d2d553ec10f2f830918df3a7fb8e8c1e5d18c) )
	ROM_LOAD( "pog.pg7",      0xe000, 0x1000, CRC(84399f54) SHA1(c90ba3e3120adda2785ab5abd309e0a703d39f8b) )
	ROM_LOAD( "pog.pg8",      0xf000, 0x1000, CRC(9e995a1a) SHA1(5c525e6c161d9d7d646857b27cecfbf8e0943480) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pog.snd",      0xe000, 0x1000, CRC(ec61f0a4) SHA1(26944ecc3e7413259928c8b0a74b2260e67d2c4e) )
ROM_END

ROM_START( piratetr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.u13",        0x8000, 0x1000, CRC(4433bb61) SHA1(eee0d7f356118f8595dd7533541db744a63a8176) )
	ROM_LOAD( "2.u14",        0x9000, 0x1000, CRC(9bdc4b77) SHA1(ebaab8b3024efd3d0b76647085d441ca204ad5d5) )
	ROM_LOAD( "3.u15",        0xa000, 0x1000, CRC(ebced718) SHA1(3a2f4385347f14093360cfa595922254c9badf1a) )
	ROM_LOAD( "4.u16",        0xb000, 0x1000, CRC(f494e657) SHA1(83a31849de8f4f70d7547199f229079f491ddc61) )
	ROM_LOAD( "5.u17",        0xc000, 0x1000, CRC(2789d68e) SHA1(af8f334ce4938cd75143b729c97cfbefd68c9e13) )
	ROM_LOAD( "6.u18",        0xd000, 0x1000, CRC(d91abb3a) SHA1(11170e69686c2a1f2dc31d41516f44b612f99bad) )
	ROM_LOAD( "7.u19",        0xe000, 0x1000, CRC(6e8808c4) SHA1(d1f76fd37d8f78552a9d53467073cc9a571d96ce) )
	ROM_LOAD( "8.u20",        0xf000, 0x1000, CRC(2802d626) SHA1(b0db688500076ee73e0001c00089a8d552c6f607) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "s.u31",        0xe000, 0x1000, CRC(2fe86a11) SHA1(aaafe411b9cb3d0221cc2af73d34ad8bb74f8327) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT      CLASS          INIT        SCREEN  COMPANY                        FULLNAME            FLAGS
GAME( 1980, killcom,   0,        killcom,  killcom,   killcom_state, empty_init, ROT0,   "Centuri (Game Plan license)", "Killer Comet",     MACHINE_SUPPORTS_SAVE )
GAME( 1980, megatack,  0,        killcom,  megatack,  killcom_state, empty_init, ROT0,   "Centuri (Game Plan license)", "Megatack (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1980, megatacka, megatack, killcom,  megatack,  killcom_state, empty_init, ROT0,   "Centuri (Game Plan license)", "Megatack (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, challeng,  0,        killcom,  challeng,  killcom_state, empty_init, ROT270, "Centuri",                     "Challenger",       MACHINE_SUPPORTS_SAVE ) // cab is vertical

GAME( 1981, kaos,      0,        killcom,  kaos,      killcom_state, empty_init, ROT270, "Pacific Polytechnical Corp. (Game Plan license)",       "Kaos",               MACHINE_SUPPORTS_SAVE )
GAME( 1982, leprechn,  0,        leprechn, leprechn,  killcom_state, empty_init, ROT0,   "Pacific Polytechnical Corp.",                           "Leprechaun (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, leprechna, leprechn, leprechn, leprechna, killcom_state, empty_init, ROT0,   "Pacific Polytechnical Corp. (Tong Electronic license)", "Leprechaun (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, potogold,  leprechn, leprechn, leprechn,  killcom_state, empty_init, ROT0,   "Pacific Polytechnical Corp. (Game Plan license)",       "Pot of Gold",        MACHINE_SUPPORTS_SAVE )
GAME( 1982, piratetr,  0,        piratetr, piratetr,  killcom_state, empty_init, ROT0,   "Pacific Polytechnical Corp. (Tong Electronic license)", "Pirate Treasure",    MACHINE_SUPPORTS_SAVE )
