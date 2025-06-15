// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, Manuel Abadia
/***************************************************************************

"Combat School" (also known as "Boot Camp") - (Konami GX611)

TODO:
- understand how the trackball really works for clone sets.
- it seems that to get correct target colors in firing range III we have to
  use the WRONG lookup table (the one for tiles instead of the one for
  sprites).
- in combatscb, wrong sprite/char priority (see cpu head at beginning of arm
  wrestling, and heads in intermission after firing range III)
- improve sound hook up in bootleg.
- YM2203 pitch is wrong. Fixing it screws up the tempo.

  Update: 3MHz(24MHz/8) is the more appropriate clock speed for the 2203.
  It gives the correct pitch(ear subjective) compared to the official
  soundtrack albeit the music plays slow by about 10%.

  Execution timing of the Z80 is important because it maintains music tempo
  by polling the 2203's second timer. Even when working alone with no
  context-switch the chip shouldn't be running at 1.5MHz otherwise it won't
  keep the right pace. Similar Konami games from the same period(mainevt,
  battlnts, flkatck...etc.) all have a 3.579545MHz Z80 for sound.

  In spite of adjusting clock speed polling is deemed inaccurate when
  interleaving is taken into account. A high resolution timer around the
  poll loop is probably the best bet. The driver sets its timer manually
  because strange enough, interleaving doesn't occur immediately when
  perfect_quantum() is called. Speculations are TIME_NOWs could have
  been used as the timer durations to force instant triggering.


Credits:

    Hardware Info:
        Jose Tejada Gomez
        Manuel Abadia
        Cesareo Gutierrez

    MAME Driver:
        Phil Stroffolino
        Manuel Abadia

Memory Maps (preliminary):

****************************
* Combat School (Original) *
****************************

0000-005f   Video Registers (banked)
0400-0407   input ports
0408        coin counters
0410        bankswitch control
0600-06ff   palette
0800-1fff   RAM
2000-2fff   Video RAM (banked)
3000-3fff   Object RAM (banked)
4000-7fff   Banked Area + IO + Video Registers
8000-ffff   ROM

SOUND CPU:
----------
0000-8000   ROM
8000-87ff   RAM
9000        uPD7759
b000        uPD7759
c000        uPD7759
d000        soundlatch read
e000-e001   YM2203


***************************
* Combat School (bootleg) *
***************************

MAIN CPU:
---------
00c0-00c3   Objects control
0500        bankswitch control
0600-06ff   palette
0800-1fff   RAM
2000-2fff   Video RAM (banked)
3000-3fff   Object RAM (banked)
4000-7fff   Banked Area + IO + Video Registers
8000-ffff   ROM

SOUND CPU:
----------
0000-8000   ROM
8000-87ef   RAM
87f0-87ff   ???
9000-9001   YM2203
9008        ???
9800        OKIM5205?
a000        soundlatch?
a800        OKIM5205?
fffc-ffff   ???

Notes about the sound system of the bootleg:
---------------------------------------------
The positions 0x87f0-0x87ff are very important, it
does work similar to a semaphore (same as a lot of
vblank bits). For example in the init code, it writes
zero to 0x87fa, then it waits until it'll be different
to zero, but it isn't written by this cpu. (shareram?)
I have tried to put here a K007232 chip, but it didn't
work.

Sound chips: OKI M5205 & YM2203

We are using the other sound hardware for now.

***************************************************************************/

#include "emu.h"
#include "combatsc.h"

#include "cpu/m6809/hd6309.h"
#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/ymopn.h"

#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

template <uint8_t Which>
void combatsc_state::flipscreen_w(int state)
{
	const uint32_t flip = state ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0;

	m_bg_tilemap[Which]->set_flip(flip);
	if (Which == 0)
		m_textlayer->set_flip(flip);
}

template <uint8_t Which>
void combatsc_state::dirtytiles()
{
	m_bg_tilemap[Which]->mark_all_dirty();
	if (Which == 0)
		m_textlayer->mark_all_dirty();
}

void combatsc_base_state::vreg_w(uint8_t data)
{
	if (data != m_vreg)
	{
		m_textlayer->mark_all_dirty();
		if ((data & 0x0f) != (m_vreg & 0x0f))
			m_bg_tilemap[0]->mark_all_dirty();
		if ((data >> 4) != (m_vreg >> 4))
			m_bg_tilemap[1]->mark_all_dirty();
		m_vreg = data;
	}
}

void combatsc_state::bankselect_w(uint8_t data)
{
	m_priority = BIT(data, 5);

	if (data & 0x40)
	{
		m_pf_view.select(1);
		m_scroll_view.select(1);
		m_video_view.select(1);
	}
	else
	{
		m_pf_view.select(0);
		m_scroll_view.select(0);
		m_video_view.select(0);
	}

	if (data & 0x10)
		m_mainbank->set_entry((data & 0x0e) >> 1);
	else
		m_mainbank->set_entry(8 + (data & 1));
}

void combatscb_state::bankselect_w(uint8_t data)
{
	m_video_view.select(BIT(data, 6));

	data &= 0x1f;

	if (data != m_bank_select)
	{
		m_bank_select = data;

		if (data & 0x10)
			m_mainbank->set_entry((data & 0x0e) >> 1);
		else
			m_mainbank->set_entry(8 + (data & 1));

		if (data == 0x1f)
		{
			m_mainbank->set_entry(8 + (data & 1));
			m_bank_io_view.select(0);
		}
		else
			m_bank_io_view.disable();
	}
}

void combatscb_state::priority_w(uint8_t data)
{
	m_priority = BIT(data, 5);
	m_video_view.select(BIT(data, 6));
}

void combatscb_state::io_w(offs_t offset, uint8_t data)
{
	m_io_ram[offset] = data;
}


/****************************************************************************/

void combatsc_state::coin_counter_w(uint8_t data)
{
	// b7-b3: unused?
	// b1: coin counter 2
	// b0: coin counter 1

	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);
}

uint8_t combatsc_state::trackball_r(offs_t offset)
{
	if (offset == 0)
	{
		int dir[4];

		for (int i = 0; i < 4; i++)
		{
			uint8_t curr = m_track_ports[i].read_safe(0xff);

			dir[i] = curr - m_pos[i];
			m_sign[i] = dir[i] & 0x80;
			m_pos[i] = curr;
		}

		// fix sign for orthogonal movements
		if (dir[0] || dir[1])
		{
			if (!dir[0]) m_sign[0] = m_sign[1] ^ 0x80;
			if (!dir[1]) m_sign[1] = m_sign[0];
		}
		if (dir[2] || dir[3])
		{
			if (!dir[2]) m_sign[2] = m_sign[3] ^ 0x80;
			if (!dir[3]) m_sign[3] = m_sign[2];
		}
	}

	return m_sign[offset] | (m_pos[offset] & 0x7f);
}


/****************************************************************************/

void combatsc_state::sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

uint8_t combatsc_state::busy_r()
{
	return m_upd7759->busy_r() ? 1 : 0;
}

void combatsc_state::play_w(uint8_t data)
{
	m_upd7759->start_w(!BIT(data, 1));
}

void combatsc_state::voice_reset_w(uint8_t data)
{
	m_upd7759->reset_w(BIT(data, 0));
}

void combatsc_state::portA_w(uint8_t data)
{
	// unknown. always write 0
}

// causes scores to disappear during fire ranges, either sprite busy flag or screen frame number related
uint8_t combatsc_state::unk_r()
{
	return 0; //m_screen->frame_number() & 1;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void combatsc_state::main_map(address_map &map)
{
	map(0x0000, 0x0007).view(m_pf_view);
	m_pf_view[0](0x0000, 0x0007).w(m_k007121[0], FUNC(k007121_device::ctrl_w));
	m_pf_view[1](0x0000, 0x0007).w(m_k007121[1], FUNC(k007121_device::ctrl_w));

	map(0x001f, 0x001f).r(FUNC(combatsc_state::unk_r));
	map(0x0020, 0x005f).view(m_scroll_view);
	m_scroll_view[0](0x0020, 0x005f).rw(m_k007121[0], FUNC(k007121_device::scroll_r), FUNC(k007121_device::scroll_w));
	m_scroll_view[1](0x0020, 0x005f).rw(m_k007121[1], FUNC(k007121_device::scroll_r), FUNC(k007121_device::scroll_w));

	map(0x0200, 0x0207).rw("k007452", FUNC(k007452_device::read), FUNC(k007452_device::write));

	map(0x0400, 0x0400).portr("IN0");
	map(0x0401, 0x0401).portr("DSW3");
	map(0x0402, 0x0402).portr("DSW1");
	map(0x0403, 0x0403).portr("DSW2");
	map(0x0404, 0x0407).r(FUNC(combatsc_state::trackball_r)); // 1P & 2P controls / trackball
	map(0x0408, 0x0408).w(FUNC(combatsc_state::coin_counter_w));
	map(0x040c, 0x040c).w(FUNC(combatsc_state::vreg_w));
	map(0x0410, 0x0410).nopr().w(FUNC(combatsc_state::bankselect_w)); // read is clr a (discarded)
	map(0x0414, 0x0414).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x0418, 0x0418).w(FUNC(combatsc_state::sh_irqtrigger_w));
	map(0x041c, 0x041c).w("watchdog", FUNC(watchdog_timer_device::reset_w)); // watchdog reset?

	map(0x0600, 0x06ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");
	map(0x0800, 0x1fff).ram();
	map(0x2000, 0x3fff).view(m_video_view);
	m_video_view[0](0x2000, 0x3fff).ram().share(m_videoram[0]).w(FUNC(combatsc_state::videoview0_w));
	m_video_view[1](0x2000, 0x3fff).ram().share(m_videoram[1]).w(FUNC(combatsc_state::videoview1_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x8000, 0xffff).rom().region("maincpu", 0x28000);
}

void combatscb_state::main_map(address_map &map)
{
	map(0x0000, 0x04ff).ram();
	map(0x0500, 0x0500).w(FUNC(combatscb_state::bankselect_w));
	map(0x0600, 0x06ff).ram().w(m_palette, FUNC(palette_device::write_indirect)).share("palette");
	map(0x0800, 0x1fff).ram();
	map(0x2000, 0x3fff).view(m_video_view);
	m_video_view[0](0x2000, 0x3fff).ram().share(m_videoram[0]).w(FUNC(combatscb_state::videoview0_w));
	m_video_view[1](0x2000, 0x3fff).ram().share(m_videoram[1]).w(FUNC(combatscb_state::videoview1_w));
	map(0x4000, 0x7fff).bankr(m_mainbank);
	map(0x4000, 0x7fff).view(m_bank_io_view);
	m_bank_io_view[0](0x4000, 0x7fff).w(FUNC(combatscb_state::io_w));
	m_bank_io_view[0](0x4400, 0x4400).w(FUNC(combatscb_state::priority_w));
	m_bank_io_view[0](0x4800, 0x4800).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	m_bank_io_view[0](0x4c00, 0x4c00).w(FUNC(combatscb_state::vreg_w));
	m_bank_io_view[0](0x4400, 0x4400).portr("IN0");
	m_bank_io_view[0](0x4401, 0x4401).portr("IN1");
	m_bank_io_view[0](0x4402, 0x4402).portr("DSW1");
	m_bank_io_view[0](0x4403, 0x4403).portr("DSW2");
	map(0x8000, 0xffff).rom().region("maincpu", 0x28000);
}

void combatsc_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();

	map(0x9000, 0x9000).w(FUNC(combatsc_state::play_w));                  // upd7759 play voice
	map(0xa000, 0xa000).w(m_upd7759, FUNC(upd7759_device::port_w));       // upd7759 voice select
	map(0xb000, 0xb000).r(FUNC(combatsc_state::busy_r));                  // upd7759 busy?
	map(0xc000, 0xc000).w(FUNC(combatsc_state::voice_reset_w));           // upd7759 reset?

	map(0xd000, 0xd000).r(m_soundlatch, FUNC(generic_latch_8_device::read)); // soundlatch read?
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));   // YM 2203 intercepted
}

void combatscb_state::msm_w(uint8_t data)
{
	m_soundbank->set_entry(BIT(data, 7));

	m_msm->reset_w(BIT(data, 4));
	m_msm->data_w(data & 0x0f);
}

void combatscb_state::sound_irq_ack(uint8_t data)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
}

void combatscb_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0x9008, 0x9009).r("ymsnd", FUNC(ym2203_device::read));               // ???
	map(0x9800, 0x9800).w(FUNC(combatscb_state::msm_w));
	map(0xa000, 0xa000).r(m_soundlatch, FUNC(generic_latch_8_device::read)); // soundlatch read?
	map(0xa800, 0xa800).w(FUNC(combatscb_state::sound_irq_ack));
	map(0xc000, 0xffff).bankr(m_soundbank);
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( common_inputs )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On )  )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW3:2" )   // Not Used according to the manual
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW3:4" )   // Not Used according to the manual
INPUT_PORTS_END

static INPUT_PORTS_START( dips )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	// None = coin slot B disabled

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )   // Not Used according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )   // Not Used according to the manual
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )   // Not Used according to the manual
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )   // Not Used according to the manual
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING( 0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING( 0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING( 0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING( 0x80, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( combatsc )
	PORT_INCLUDE( dips )

	PORT_INCLUDE( common_inputs )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( combatsct )
	PORT_INCLUDE( dips )

	PORT_INCLUDE( common_inputs )

	// trackball 1P
	PORT_START("TRACK0_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("TRACK0_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_PLAYER(1)

	// trackball 2P (not implemented yet)
	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( combatscb )
	PORT_INCLUDE( dips )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:3" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tile_layout =
{
	8,8,
	0x2000, // number of tiles
	4,      // bitplanes
	{ 0*0x10000*8, 1*0x10000*8, 2*0x10000*8, 3*0x10000*8 }, // plane offsets
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	0x800,  // number of sprites
	4,      // bitplanes
	{ 3*0x10000*8, 2*0x10000*8, 1*0x10000*8, 0*0x10000*8 }, // plane offsets
	{
		0,1,2,3,4,5,6,7,
		16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7
	},
	{
		0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
		8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8
	},
	8*8*4
};

static GFXDECODE_START( gfx_combatsc_1 )
	GFXDECODE_ENTRY( "gfx1", 0x00000, gfx_8x8x4_packed_msb, 0, 8*16 )
GFXDECODE_END

static GFXDECODE_START( gfx_combatsc_2 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, gfx_8x8x4_packed_msb, 0, 8*16 )
GFXDECODE_END

static GFXDECODE_START( gfx_combatscb )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tile_layout,   0, 8*16 )
	GFXDECODE_ENTRY( "gfx1", 0x40000, tile_layout,   0, 8*16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, sprite_layout, 0, 8*16 )
	GFXDECODE_ENTRY( "gfx2", 0x40000, sprite_layout, 0, 8*16 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void combatsc_base_state::machine_start()
{
	m_mainbank->configure_entries(0, 10, memregion("maincpu")->base(), 0x4000);

	save_item(NAME(m_priority));
	save_item(NAME(m_vreg));
}

void combatsc_state::machine_start()
{
	combatsc_base_state::machine_start();

	save_item(NAME(m_pos));
	save_item(NAME(m_sign));
}

void combatscb_state::machine_start()
{
	combatsc_base_state::machine_start();

	m_soundbank->configure_entries(0, 2, memregion("audiocpu")->base() + 0x8000, 0x4000);

	save_item(NAME(m_bank_select));
}

void combatsc_base_state::machine_reset()
{
	m_vreg = -1;
}

void combatsc_state::machine_reset()
{
	combatsc_base_state::machine_reset();

	for (int i = 0; i < 4; i++)
	{
		m_pos[i] = 0;
		m_sign[i] = 0;
	}

	bankselect_w(0);
}

void combatscb_state::machine_reset()
{
	combatsc_base_state::machine_reset();

	m_bank_select = -1;

	bankselect_w(0);
}


// Combat School (original)
void combatsc_state::combatsc(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 24_MHz_XTAL / 8); // HD63C09E, 3 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &combatsc_state::main_map);

	Z80(config, m_audiocpu, 3579545); // 3.579545 MHz??? (no such XTAL on board!)
	m_audiocpu->set_addrmap(AS_PROGRAM, &combatsc_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(1200));

	WATCHDOG_TIMER(config, "watchdog");

	KONAMI_007452_MATH(config, "k007452");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(24_MHz_XTAL / 3, 512, 0, 256, 264, 16, 240);
	m_screen->set_screen_update(FUNC(combatsc_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(combatsc_state::palette));
	m_palette->set_format(palette_device::xBGR_555, 8 * 16 * 16, 128);
	m_palette->set_endianness(ENDIANNESS_LITTLE);

	K007121(config, m_k007121[0], 0, gfx_combatsc_1, m_palette, m_screen);
	m_k007121[0]->set_irq_cb().set_inputline(m_maincpu, HD6309_IRQ_LINE);
	m_k007121[0]->set_flipscreen_cb().set(FUNC(combatsc_state::flipscreen_w<0>));
	m_k007121[0]->set_dirtytiles_cb(FUNC(combatsc_state::dirtytiles<0>));

	K007121(config, m_k007121[1], 0, gfx_combatsc_2, m_palette, m_screen);
	m_k007121[1]->set_flipscreen_cb().set(FUNC(combatsc_state::flipscreen_w<1>));
	m_k007121[1]->set_dirtytiles_cb(FUNC(combatsc_state::dirtytiles<1>));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 24_MHz_XTAL / 8));
	ymsnd.port_a_write_callback().set(FUNC(combatsc_state::portA_w));
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.20);

	UPD7759(config, m_upd7759).add_route(ALL_OUTPUTS, "mono", 0.70);
}


// Combat School (bootleg on different hardware)
void combatscb_state::combatscb(machine_config &config)
{
	// basic machine hardware
	HD6309E(config, m_maincpu, 3000000);  // 3 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &combatscb_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(combatsc_state::irq0_line_hold));

	Z80(config, m_audiocpu, 3579545);   // 3.579545 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &combatscb_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(1200));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(combatscb_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_combatscb);
	PALETTE(config, m_palette, FUNC(combatscb_state::palette));
	m_palette->set_format(palette_device::xBGR_555, 8 * 16 * 16, 128);
	m_palette->set_endianness(ENDIANNESS_LITTLE);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	YM2203(config, "ymsnd", 3000000).add_route(ALL_OUTPUTS, "mono", 0.20);

	MSM5205(config, m_msm, 384000);
	m_msm->vck_callback().set_inputline("audiocpu", 0, ASSERT_LINE);
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);
	m_msm->add_route(ALL_OUTPUTS, "mono", 0.30);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( combatsc )
	ROM_REGION( 0x30000, "maincpu", 0 ) // 6309 code
	ROM_LOAD( "611g02.rom", 0x00000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	ROM_LOAD( "611g01.rom", 0x20000, 0x10000, CRC(857ffffe) SHA1(de7566d58314df4b7fdc07eb31a3f9bdd12d1a73) )

	ROM_REGION( 0x10000 , "audiocpu", 0 )
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "ampal16l8.e7", 0x0000, 0x0104, CRC(300a9936) SHA1(a4a87e93f41392fc7d7d8601d7187d87b9f9ab01) )
	ROM_LOAD( "pal16r6.16d",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal20l8.8h",   0x0400, 0x0144, NO_DUMP ) // PAL is read protected
ROM_END

ROM_START( combatsct )
	ROM_REGION( 0x30000, "maincpu", 0 ) // 6309 code
	ROM_LOAD( "611g02.rom",  0x00000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	ROM_LOAD( "g01.rom",     0x20000, 0x10000, CRC(489c132f) SHA1(c717195f89add4be4a21ecc1ddd58361b0ab4a74) )

	ROM_REGION( 0x10000 , "audiocpu", 0 )
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )
ROM_END

ROM_START( combatscj )
	ROM_REGION( 0x30000, "maincpu", 0 ) // 6309 code
	ROM_LOAD( "611g02.rom",  0x00000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	ROM_LOAD( "611p01.a14",  0x20000, 0x10000, CRC(d748268e) SHA1(91588b6a0d3af47065204b980a56544a9f29b6d9) )

	ROM_REGION( 0x10000 , "audiocpu", 0 )
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )
ROM_END

ROM_START( bootcamp )
	ROM_REGION( 0x30000, "maincpu", 0 ) // 6309 code
	ROM_LOAD( "611g02.rom",  0x00000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	ROM_LOAD( "xxx-v01.12a", 0x20000, 0x10000, CRC(c10dca64) SHA1(f34de26e998b1501e430d46e96cdc58ebc68481e) )

	ROM_REGION( 0x10000 , "audiocpu", 0 )
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )
ROM_END

ROM_START( bootcampa )
	ROM_REGION( 0x30000, "maincpu", 0 ) // 6309 code
	ROM_LOAD( "611g02.rom",  0x00000, 0x20000, CRC(9ba05327) SHA1(ea03845fb49d18ac4fca97cfffce81db66b9967b) )
	ROM_LOAD( "611x01.a-14", 0x20000, 0x10000, CRC(98ffc6ed) SHA1(ab02532333272683d889f209d3fc01235871d909) )

	ROM_REGION( 0x10000 , "audiocpu", 0 )
	ROM_LOAD( "611g03.rom", 0x00000, 0x08000, CRC(2a544db5) SHA1(94a97c3c54bf13ccc665aa5057ac6b1d700fae2d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "611g07.rom",    0x00000, 0x40000, CRC(73b38720) SHA1(e109eb78aea464127d813284ca040e8d719599e3) )
	ROM_LOAD16_BYTE( "611g08.rom",    0x00001, 0x40000, CRC(46e7d28c) SHA1(1ece7fac954204ac35d00f3d573964fcf82dcf77) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "611g11.rom",    0x00000, 0x40000, CRC(69687538) SHA1(4349a1c052a759acdf7259f8bf8c5c9489b788f2) )
	ROM_LOAD16_BYTE( "611g12.rom",    0x00001, 0x40000, CRC(9c6bf898) SHA1(eafc227b4e7df0c652ec7d78784c039c35965fdc) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "611g06.h14",  0x0000, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g05.h15",  0x0100, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table
	ROM_LOAD( "611g10.h6",   0x0200, 0x0100, CRC(f916129a) SHA1(d5e4a8a3baab8fcdac86ef5182858cede1abf040) ) // sprites lookup table
	ROM_LOAD( "611g09.h7",   0x0300, 0x0100, CRC(207a7b07) SHA1(f4e638e7f182e5228a062b243406d0ceaaa5bfdc) ) // chars lookup table

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "611g04.rom",  0x00000, 0x20000, CRC(2987e158) SHA1(87c5129161d3be29a339083349807e60b625c3f7) )
ROM_END

ROM_START( combatscb )
	ROM_REGION( 0x30000, "maincpu", 0 ) // 6809 code
	ROM_LOAD( "combat.003",  0x00000, 0x10000, CRC(229c93b2) SHA1(ac3fd3df1bb5f6a461d0d1423c50568348ef69df) )
	ROM_LOAD( "combat.004",  0x10000, 0x10000, CRC(a069cb84) SHA1(f49f70afb17df46b16f5801ef42edb0706730723) )
	ROM_LOAD( "combat.002",  0x20000, 0x10000, CRC(0996755d) SHA1(bb6bbbf7ab3b5fab5e1c6cebc7b3f0d720493c3b) )

	ROM_REGION( 0x10000 , "audiocpu", 0 )
	ROM_LOAD( "combat.001",  0x00000, 0x10000, CRC(61456b3b) SHA1(320db628283dd1bec465e95020d1a1158e6d6ae4) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )
	ROM_LOAD( "combat.006",  0x00000, 0x10000, CRC(8dc29a1f) SHA1(564dd7c6acff34db93b8e300dda563f5f38ba159) ) // tiles, bank 0
	ROM_LOAD( "combat.008",  0x10000, 0x10000, CRC(61599f46) SHA1(cfd79a88bb496773daf207552c67f595ee696bc4) )
	ROM_LOAD( "combat.010",  0x20000, 0x10000, CRC(d5cda7cd) SHA1(140db6270c3f358aa27013db3bb819a48ceb5142) )
	ROM_LOAD( "combat.012",  0x30000, 0x10000, CRC(ca0a9f57) SHA1(d6b3daf7c34345bb2f64068d480bd51d7bb36e4d) )
	ROM_LOAD( "combat.005",  0x40000, 0x10000, CRC(0803a223) SHA1(67d4162385dd56d5396e181070bfa6760521eb45) ) // tiles, bank 1
	ROM_LOAD( "combat.007",  0x50000, 0x10000, CRC(23caad0c) SHA1(0544cde479c6d4192da5bb4b6f0e2e75d09663c3) )
	ROM_LOAD( "combat.009",  0x60000, 0x10000, CRC(5ac80383) SHA1(1e89c371a92afc000d593daebda4156952a15244) )
	ROM_LOAD( "combat.011",  0x70000, 0x10000, CRC(cda83114) SHA1(12d2a9f694287edb3bb0ee7a8ba0e0724dad8e1f) )

	ROM_REGION( 0x80000, "gfx2", ROMREGION_INVERT )
	ROM_LOAD( "combat.013",  0x00000, 0x10000, CRC(4bed2293) SHA1(3369de47d4ba041d9f17a18dcca2af7ac9f8bc0c) ) // sprites, bank 0
	ROM_LOAD( "combat.015",  0x10000, 0x10000, CRC(26c41f31) SHA1(f8eb7d0729a21a0dd92ce99c9cda0cde9526b861) )
	ROM_LOAD( "combat.017",  0x20000, 0x10000, CRC(6071e6da) SHA1(ba5f8e83b07faaffc564d3568630e17efdb5a09f) )
	ROM_LOAD( "combat.019",  0x30000, 0x10000, CRC(3b1cf1b8) SHA1(ff4de37c051bcb374c44d1b99006ff6ff5e1f927) )
	ROM_LOAD( "combat.014",  0x40000, 0x10000, CRC(82ea9555) SHA1(59bf7836938ce9e3242d1cca754de8dbe85bbfb7) ) // sprites, bank 1
	ROM_LOAD( "combat.016",  0x50000, 0x10000, CRC(2e39bb70) SHA1(a6c4acd93cc803e987de6e18fbdc5ce4634b14a8) )
	ROM_LOAD( "combat.018",  0x60000, 0x10000, CRC(575db729) SHA1(6b1676da4f24fc90c77262789b6cc116184ab912) )
	ROM_LOAD( "combat.020",  0x70000, 0x10000, CRC(8d748a1a) SHA1(4386e14e19b91e053033dde2a13019bc6d8e1d5a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "prom.d10",    0x0000, 0x0100, CRC(265f4c97) SHA1(76f1b75a593d3d77ef6173a1948f842d5b27d418) ) // sprites lookup table
	ROM_LOAD( "prom.c11",    0x0100, 0x0100, CRC(a7a5c0b4) SHA1(48bfc3af40b869599a988ebb3ed758141bcfd4fc) ) // priority?
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void combatsc_state::init_combatsc()
{
	// joystick instead of trackball
	m_maincpu->space(AS_PROGRAM).install_read_port(0x0404, 0x0404, "IN1");
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, combatsc,  0,        combatsc,  combatsc,  combatsc_state,  init_combatsc, ROT0, "Konami",          "Combat School (joystick)",        MACHINE_SUPPORTS_SAVE )
GAME( 1987, combatsct, combatsc, combatsc,  combatsct, combatsc_state,  empty_init,    ROT0, "Konami",          "Combat School (trackball)",       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1987, combatscj, combatsc, combatsc,  combatsct, combatsc_state,  empty_init,    ROT0, "Konami",          "Combat School (Japan trackball)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1987, bootcamp,  combatsc, combatsc,  combatsct, combatsc_state,  empty_init,    ROT0, "Konami",          "Boot Camp (set 1)",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1987, bootcampa, combatsc, combatsc,  combatsct, combatsc_state,  empty_init,    ROT0, "Konami",          "Boot Camp (set 2)",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1988, combatscb, combatsc, combatscb, combatscb, combatscb_state, empty_init,    ROT0, "bootleg (Datsu)", "Combat School (bootleg)",         MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
