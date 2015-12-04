// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Quench
/* Super Slam (c)1993 Playmark */

/*

Still some unknown reads / writes (it writes all over the place ...)
Inputs / DSW's need finishing / verifying.

The 87C751 MCU for sound has not had its internal program ROM dumped.
So the sound MCU is simulated here - and therefore not 100% correct.

Update 12/03/2005 - Pierpaolo Prazzoli
- Fixed sprites
- Fixed text tilemap colors
- Fixed text tilemap scrolls
- Fixed VSync
- Fixed middle tilemap removing wraparound in the title screen (24/03/2005)

  Sound command 29h is fired when the game is completed successfully. It requires
  a melody from bank one to be playing, the real MCU doesn't set the bank though.
  Consequently, the "Game, Set and Match" vocal is not played.
*/

/*

Here's the info about this dump:

Name:            Super Slam
Manufacturer:    PlayMark
Year:            1993
Date Dumped:     18-07-2002

CPU:             ST TS68000CP12, Signetics 87C751
SOUND:           OKIM6295
GFX:             TI TPC1020AFN-084C
CRYSTALS:        12MHz, 26MHz
DIPSW:           Two 8-Way
Country:         Italy

About the game:

This is a Tennis game :) You can play with some boys and girls, an old man,
a small kid and even with a dog! And remember, Winners don't use Drugs ;)



PCB Layout
----------

----------------------------------------------------
|      6295   3              62256             4   |
|          87C751            62256                 |
|      1MHz                                    5   |
|       6116                                       |
|       6116                                   6   |
|     DSW1(8)                                      |
|J                                             7   |
|A                                 TPC1020         |
|M    DSW2(8)                                  8   |
|M                                                 |
|A                                             9   |
|                                                  |
|                                              10  |
|                                                  |
|                                              11  |
|12MHz                          2018  2018         |
|       68000P12 62256                             |
|                 2             2018  2018         |
|                62256                             |
|                 1     26MHz                      |
----------------------------------------------------

Notes:
      68k clock: 12MHz
          VSync: 58Hz
          HSync: 14.62kHz
   87C751 clock: 12MHz (87C751 is 8051 based DIP24 MCU with)
                       (2Kx8 OTP EPROM and 64Kx8 SRAM)
                       (unfortunately, it's protected)
*/



#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "includes/sslam.h"


#define oki_time_base 0x08




/**************************************************************************
   This table converts commands sent from the main CPU, into sample numbers
   played back by the sound processor (the 87C751).
   Values of 0xff indicate unused commands.
   Note, that some samples such as 60, 61 and 62 are sequenced to form a
   music track. Ditto for samples 65, 66, 67 and 68, and also 6A and 6B.
   The sequencing of the music tracks are handled in the second table below.
*/

static const UINT8 sslam_snd_cmd[64] =
{
/*00*/  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
/*08*/  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x70, 0x71,
/*10*/  0x72, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
/*18*/  0x15, 0x16, 0x17, 0x18, 0x19, 0x73, 0x74, 0x75,
/*20*/  0x76, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x63,
/*28*/  0x64, 0x6b, 0xff, 0xff, 0x60, 0x20, 0x6c, 0x65,
/*30*/  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
/*38*/  0x29, 0x2a, 0x2b, 0xff, 0x69, 0xff, 0x20, 0xff
};


/**************************************************************************
   Music Sequencing:
   The columns contain the sequence (from left to right) of samples to play in each backing music track
   The first row is zero as a place holder to indicate that music has/should stop
   The second row onwards are the various music tracks
   The last (right most) byte in each row (track) is a flag to indicate what to do at the end of the sequence:
     If the last byte is 0x00, the track should not be repeated
     If the last byte is 0xff, the track should loop by restarting at the first column sample
*/

static const UINT8 sslam_snd_loop[8][19] =
{
/*NA*/  { 0x00, 0x00 }, /* Not a loop - just a parking position for stopping track playback */
/*60*/  { 0x60, 0x60, 0x61, 0x61, 0x60, 0x60, 0x61, 0x62, 0xff },
/*63*/  { 0x63, 0x00 },
/*64*/  { 0x64, 0x00 },
/*65*/  { 0x65, 0x65, 0x66, 0x66, 0x65, 0x65, 0x66, 0x67, 0x67, 0x68, 0x65, 0x65, 0x67, 0x65, 0x66, 0x66, 0x67, 0x68, 0xff },
/*69*/  { 0x69, 0xff },
/*6B*/  { 0x6b, 0x6a, 0x6a, 0x6b, 0x6a, 0xff },
/*6C*/  { 0x6c, 0xff }
};




/* Sound numbers in the Super Slam sample ROM
01 "Out"
02 "Fault"
03 "Deuce"
04 "Let"
05 "Net"
06 "Let's Play"
07 "Play"
08 "Love"
09 "All"
0a "15"
0b "30"
0c "40"
0d "Change Court"
0e "Game"
0f "0"
10 "1"
11 "2"
12 "3"
13 "4"
14 "5"
15 "6"
16 "7"
17 "8"
18 "9"
19 "10"
1a Ball Hit
1b Ball Hit
1c Ball Bounce
1d Swing
1e Crowd Cheer
1f Crowd Clap
20 Horns
21 Ball Smash
22 Ball Hit
23 "Oww!"
24 "Ooh"
25 "Set Point"
26 "Match Point"
27 Boy "Ouch"
28 Girl "Ouch"
29 Synth
2a Three step rising synth
2b Ball Hit

60 Melody A1 -------------
61 Melody A2
62 Melody A3        Bank 0
63 Pattern B
64 Pattern C  ------------

65 Melody D1 -------------
66 Melody D2        Bank 1
67 Melody D3
68 Melody D4 -------------

69 Melody E --------------
6a Melody F1        Bank 2
6b Melody F2
6c Melody G --------------

70 "Tie Break"          -------------
71 "Advantage Server"
72 "Advantage Receiver"
73 "Game and First Set"        Bank 1
74 "Game and Second Set"
75 "Game and Third Set"
76 "Game Set and Match" -------------
*/





TIMER_CALLBACK_MEMBER(sslam_state::music_playback)
{
	int pattern = 0;
	if ((m_oki->read_status() & 0x08) == 0)
	{
		m_bar += 1;
		pattern = sslam_snd_loop[m_melody][m_bar];

		if (pattern) {
			if (pattern == 0xff) {      /* Repeat track from first bar */
				m_bar = 0;
				pattern = sslam_snd_loop[m_melody][m_bar];
			}
			logerror("Changing bar in music track to pattern %02x\n",pattern);
			m_oki->write_command(0x80 | pattern);
			m_oki->write_command(0x81);
		}
		else if (pattern == 0x00) {     /* Non-looped track. Stop playing it */
			m_track = 0;
			m_melody = 0;
			m_bar = 0;
			m_music_timer->enable(false);
		}
	}

	if (0)
	{
		pattern = sslam_snd_loop[m_melody][m_bar];
		popmessage("Music track: %02x, Melody: %02x, Pattern: %02x, Bar:%02d",m_track,m_melody,pattern,m_bar);
	}
}


void sslam_state::sslam_play(int track, int data)
{
	int status = m_oki->read_status();

	if (data < 0x80) {
		if (track) {
			if (m_track != data) {
				m_track  = data;
				m_bar = 0;
				if (status & 0x08)
					m_oki->write_command(0x40);
				m_oki->write_command((0x80 | data));
				m_oki->write_command(0x81);
				m_music_timer->adjust(attotime::from_msec(4), 0, attotime::from_hz(250));    /* 250Hz for smooth sequencing */
			}
		}
		else {
			if ((status & 0x01) == 0) {
				m_oki->write_command((0x80 | data));
				m_oki->write_command(0x11);
			}
			else if ((status & 0x02) == 0) {
				m_oki->write_command((0x80 | data));
				m_oki->write_command(0x21);
			}
			else if ((status & 0x04) == 0) {
				m_oki->write_command((0x80 | data));
				m_oki->write_command(0x41);
			}
		}
	}
	else {      /* use above 0x80 to turn off channels */
		if (track) {
			m_music_timer->enable(false);
			m_track = 0;
			m_melody = 0;
			m_bar = 0;
		}
		data &= 0x7f;
		m_oki->write_command(data);
	}
}

WRITE8_MEMBER(sslam_state::sslam_snd_w)
{
	logerror("%s Writing %04x to Sound CPU\n",machine().describe_context(),data);
	if (data >= 0x40) {
		if (data == 0xfe) {
			/* This should reset the sound MCU and stop audio playback, but here, it */
			/* chops the first coin insert. So let's only stop any playing melodies. */
			sslam_play(1, (0x80 | 0x40));       /* Stop playing the melody */
		}
		else {
			logerror("Unknown command (%02x) sent to the Sound controller\n",data);
			popmessage("Unknown command (%02x) sent to the Sound controller",data);
		}
	}
	else if (data == 0) {
		m_bar = 0;      /* Complete any current bars then stop sequencing */
		m_melody = 0;
	}
	else {
		m_sound = sslam_snd_cmd[data];

		if (m_sound == 0xff) {
			popmessage("Unmapped sound command %02x on Bank %02x",data,m_snd_bank);
		}
		else if (m_sound >= 0x70) {
			/* These vocals are in bank 1, but a bug in the actual MCU doesn't set the bank */
//          if (m_snd_bank != 1)
//          m_oki->set_bank_base((1 * 0x40000));
//          sslam_snd_bank = 1;
			sslam_play(0, m_sound);
		}
		else if (m_sound >= 0x69) {
			if (m_snd_bank != 2)
				m_oki->set_bank_base(2 * 0x40000);
			m_snd_bank = 2;
			switch (m_sound)
			{
				case 0x69:  m_melody = 5; break;
				case 0x6b:  m_melody = 6; break;
				case 0x6c:  m_melody = 7; break;
				default:    m_melody = 0; m_bar = 0; break; /* Invalid */
			}
			sslam_play(m_melody, m_sound);
		}
		else if (m_sound >= 0x65) {
			if (m_snd_bank != 1)
				m_oki->set_bank_base(1 * 0x40000);
			m_snd_bank = 1;
			m_melody = 4;
			sslam_play(m_melody, m_sound);
		}
		else if (m_sound >= 0x60) {
			if (m_snd_bank != 0)
				m_oki->set_bank_base(0 * 0x40000);
			m_snd_bank = 0;
			switch (m_sound)
			{
				case 0x60:  m_melody = 1; break;
				case 0x63:  m_melody = 2; break;
				case 0x64:  m_melody = 3; break;
				default:    m_melody = 0; m_bar = 0; break; /* Invalid */
			}
			sslam_play(m_melody, m_sound);
		}
		else {
			sslam_play(0, m_sound);
		}
	}
}



WRITE16_MEMBER(sslam_state::powerbls_sound_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(MCS51_INT1_LINE, HOLD_LINE);
}

/* Memory Maps */

/* these will need verifying .. the game writes all over the place ... */

static ADDRESS_MAP_START( sslam_program_map, AS_PROGRAM, 16, sslam_state )
	AM_RANGE(0x000400, 0x07ffff) AM_RAM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(sslam_bg_tileram_w) AM_SHARE("bg_tileram")
	AM_RANGE(0x104000, 0x107fff) AM_RAM_WRITE(sslam_md_tileram_w) AM_SHARE("md_tileram")
	AM_RANGE(0x108000, 0x10ffff) AM_RAM_WRITE(sslam_tx_tileram_w) AM_SHARE("tx_tileram")
	AM_RANGE(0x110000, 0x11000d) AM_RAM AM_SHARE("regs")
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP
	AM_RANGE(0x280000, 0x280fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x201000, 0x201fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("IN0")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("IN1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("IN2")
	AM_RANGE(0x300016, 0x300017) AM_READ_PORT("IN3")
	AM_RANGE(0x300018, 0x300019) AM_READ_PORT("IN4")
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW1")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITE8(sslam_snd_w, 0x00ff)
	AM_RANGE(0xf00000, 0xffffff) AM_RAM   /* Main RAM */

	AM_RANGE(0x000000, 0xffffff) AM_ROM   /* I don't honestly know where the rom is mirrored .. so all unmapped reads / writes go to rom */
ADDRESS_MAP_END

static ADDRESS_MAP_START( powerbls_map, AS_PROGRAM, 16, sslam_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(powerbls_bg_tileram_w) AM_SHARE("bg_tileram")
	AM_RANGE(0x104000, 0x107fff) AM_RAM // not used
	AM_RANGE(0x110000, 0x11000d) AM_RAM AM_SHARE("regs")
	AM_RANGE(0x200000, 0x200001) AM_WRITENOP
	AM_RANGE(0x201000, 0x201fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x280000, 0x2803ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("IN0")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("IN1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("IN2")
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW1")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITE(powerbls_sound_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP
	AM_RANGE(0xff0000, 0xffffff) AM_RAM   /* Main RAM */
ADDRESS_MAP_END


/*
    Sound MCU mapping
*/

READ8_MEMBER(sslam_state::playmark_snd_command_r)
{
	UINT8 data = 0;

	if ((m_oki_control & 0x38) == 0x30) {
		data = soundlatch_byte_r(space,0);
	}
	else if ((m_oki_control & 0x38) == 0x28) {
		data = (m_oki->read(space,0) & 0x0f);
	}

	return data;
}

WRITE8_MEMBER(sslam_state::playmark_oki_w)
{
	m_oki_command = data;
}

WRITE8_MEMBER(sslam_state::playmark_snd_control_w)
{
	m_oki_control = data;

	if (data & 3)
	{
		if (m_oki_bank != ((data & 3) - 1))
		{
			m_oki_bank = (data & 3) - 1;
			m_oki->set_bank_base(0x40000 * m_oki_bank);
		}
	}

	if ((data & 0x38) == 0x18)
	{
		m_oki->write(space, 0, m_oki_command);
	}

//  !(data & 0x80) -> sound enable
//   (data & 0x40) -> always set
}

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, sslam_state )
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_WRITE(playmark_snd_control_w)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(playmark_snd_command_r, playmark_oki_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( sslam )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "Coin(s) per Player" )    PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0x38, 0x38, "Coin Multiplicator" )    PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x38, "*1" )
	PORT_DIPSETTING(    0x30, "*2" )
	PORT_DIPSETTING(    0x28, "*3" )
	PORT_DIPSETTING(    0x20, "*4" )
	PORT_DIPSETTING(    0x18, "*5" )
	PORT_DIPSETTING(    0x10, "*6" )
	PORT_DIPSETTING(    0x08, "*7" )
	PORT_DIPSETTING(    0x00, "*8" )
	PORT_DIPNAME( 0x40, 0x40, "On Time Up" )            PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, "End After Point" )
	PORT_DIPSETTING(    0x40, "End After Game" )
	PORT_DIPNAME( 0x80, 0x80, "Coin Slots" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:1,2") // 0x000522 = 0x00400e
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x04, 0x04, "Singles Game Time" )     PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "180 Seconds" )
	PORT_DIPSETTING(    0x00, "120 Seconds" )
	PORT_DIPNAME( 0x08, 0x08, "Doubles Game Time" )     PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "180 Seconds" )
	PORT_DIPSETTING(    0x00, "120 Seconds" )
	PORT_DIPNAME( 0x30, 0x30, "Starting Score" )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "4-4" )
	PORT_DIPSETTING(    0x20, "3-4" )
	PORT_DIPSETTING(    0x10, "3-3" )
	PORT_DIPSETTING(    0x00, "0-0" )
	PORT_DIPNAME( 0x40, 0x40, "Play Mode"   )           PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x40, "4 Players" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( powerbls )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Language ) )         PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( English ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x20, 0x00, "Weapon" )                    PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* GFX Decodes */

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles16x16_layout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*16
};

static GFXDECODE_START( sslam )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout,   0x300, 16 ) /* spr */
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout,     0, 16 ) /* bg */
	GFXDECODE_ENTRY( "gfx1", 0, tiles16x16_layout, 0x100, 16 ) /* mid */
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,   0x200, 16 ) /* tx */
GFXDECODE_END

static GFXDECODE_START( powerbls )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8_layout,   0x100, 16 ) /* spr */
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,       0, 16 ) /* bg */
GFXDECODE_END


/* Machine Driver */

static MACHINE_CONFIG_START( sslam, sslam_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(sslam_program_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sslam_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", I8051, 12000000)
	MCFG_DEVICE_DISABLE()       /* Internal code is not dumped - 2 boards were protected */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 39*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sslam_state, screen_update_sslam)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sslam)
	MCFG_PALETTE_ADD("palette", 0x800)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(sslam_state,sslam)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( powerbls, sslam_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(powerbls_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sslam_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", I80C51, 12000000)      /* 83C751 */
	MCFG_CPU_IO_MAP(sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sslam_state, screen_update_powerbls)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", powerbls)
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(sslam_state,powerbls)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)   /* verified on original PCB */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

/* maybe one dump is bad .. which? -> 2nd set was verified good from 2 pcbs */

ROM_START( sslam )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.u67", 0x00000, 0x80000, CRC(1ce52917) SHA1(b9b1d14ea44c248ce6e615c5c553c0d485c1302b) )
	ROM_RELOAD ( 0x100000, 0x80000 )
	ROM_RELOAD ( 0x200000, 0x80000 )
	ROM_RELOAD ( 0x300000, 0x80000 )
	ROM_RELOAD ( 0x400000, 0x80000 )
	ROM_RELOAD ( 0x500000, 0x80000 )
	ROM_RELOAD ( 0x600000, 0x80000 )
	ROM_RELOAD ( 0x700000, 0x80000 )
	ROM_RELOAD ( 0x800000, 0x80000 )
	ROM_RELOAD ( 0x900000, 0x80000 )
	ROM_RELOAD ( 0xa00000, 0x80000 )
	ROM_RELOAD ( 0xb00000, 0x80000 )
	ROM_RELOAD ( 0xc00000, 0x80000 )
	ROM_RELOAD ( 0xd00000, 0x80000 )
	ROM_RELOAD ( 0xe00000, 0x80000 )
	ROM_RELOAD ( 0xf00000, 0x80000 )
	ROM_LOAD16_BYTE( "it_22.bin", 0x00001, 0x80000, CRC(51c56828) SHA1(d71d64b0268c156456bed64b4c13b98181fa3e0f) )
	ROM_RELOAD ( 0x100001, 0x80000 )
	ROM_RELOAD ( 0x200001, 0x80000 )
	ROM_RELOAD ( 0x300001, 0x80000 )
	ROM_RELOAD ( 0x400001, 0x80000 )
	ROM_RELOAD ( 0x500001, 0x80000 )
	ROM_RELOAD ( 0x600001, 0x80000 )
	ROM_RELOAD ( 0x700001, 0x80000 )
	ROM_RELOAD ( 0x800001, 0x80000 )
	ROM_RELOAD ( 0x900001, 0x80000 )
	ROM_RELOAD ( 0xa00001, 0x80000 )
	ROM_RELOAD ( 0xb00001, 0x80000 )
	ROM_RELOAD ( 0xc00001, 0x80000 )
	ROM_RELOAD ( 0xd00001, 0x80000 )
	ROM_RELOAD ( 0xe00001, 0x80000 )
	ROM_RELOAD ( 0xf00001, 0x80000 )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "s87c751.bin",  0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0  ) /* Bg */
	ROM_LOAD( "7.u45",     0x000000, 0x80000, CRC(64ecdde9) SHA1(576ba1169d90970622249e532baa4209bf12de5a) )
	ROM_LOAD( "6.u39",     0x080000, 0x80000, CRC(6928065c) SHA1(ad5b1889bebf0358df0295d6041b798ac53ac625) )
	ROM_LOAD( "5.u42",     0x100000, 0x80000, CRC(8d18bdc6) SHA1(cacc4f475f85438a00ead4911730202e995983a7) )
	ROM_LOAD( "4.u36",     0x180000, 0x80000, CRC(8e15fb9d) SHA1(47917d8aac1bce2e15f36904f5c2534e5b80236b) )

	ROM_REGION( 0x200000, "gfx2", 0  ) /* Sprites */
	ROM_LOAD( "8.u83",     0x000000, 0x80000, CRC(19bb89dd) SHA1(c2a0c32d350a193d366b5086502998281fd0bec4) )
	ROM_LOAD( "9.u84",     0x080000, 0x80000, CRC(d50d86c7) SHA1(7ecbcc03851a8174610f7f5ad889e40543da928e) )
	ROM_LOAD( "10.u85",    0x100000, 0x80000, CRC(681b8ac8) SHA1(ebfeffc091f53af246311574b9c5d83d2716a7be) )
	ROM_LOAD( "11.u86",    0x180000, 0x80000, CRC(e41f89e3) SHA1(e4b39411a4cea6aa6c01564f74bb8e432d382a73) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is the area that gets switched */
	ROM_REGION( 0xc0000, "oki", 0 ) /* OKI Samples */
	ROM_LOAD( "3.u13",       0x00000, 0x40000, CRC(d0a9245f) SHA1(2e840cdd7bdfe7c6f986daf88576de0559597499) )
	ROM_CONTINUE(            0x60000, 0x20000 )
	ROM_CONTINUE(            0xa0000, 0x20000 )
	ROM_COPY( "oki", nullptr, 0x40000, 0x20000)
	ROM_COPY( "oki", nullptr, 0x80000, 0x20000)
ROM_END

ROM_START( sslama )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "2.u67", 0x00000, 0x80000, CRC(1ce52917) SHA1(b9b1d14ea44c248ce6e615c5c553c0d485c1302b) )
	ROM_RELOAD ( 0x100000, 0x80000 )
	ROM_RELOAD ( 0x200000, 0x80000 )
	ROM_RELOAD ( 0x300000, 0x80000 )
	ROM_RELOAD ( 0x400000, 0x80000 )
	ROM_RELOAD ( 0x500000, 0x80000 )
	ROM_RELOAD ( 0x600000, 0x80000 )
	ROM_RELOAD ( 0x700000, 0x80000 )
	ROM_RELOAD ( 0x800000, 0x80000 )
	ROM_RELOAD ( 0x900000, 0x80000 )
	ROM_RELOAD ( 0xa00000, 0x80000 )
	ROM_RELOAD ( 0xb00000, 0x80000 )
	ROM_RELOAD ( 0xc00000, 0x80000 )
	ROM_RELOAD ( 0xd00000, 0x80000 )
	ROM_RELOAD ( 0xe00000, 0x80000 )
	ROM_RELOAD ( 0xf00000, 0x80000 )
	ROM_LOAD16_BYTE( "1.u56", 0x00001, 0x80000,  CRC(59bec8ae) SHA1(2d53213a1d335184384b2138d18d496b602dc3fb) )
	ROM_RELOAD ( 0x100001, 0x80000 )
	ROM_RELOAD ( 0x200001, 0x80000 )
	ROM_RELOAD ( 0x300001, 0x80000 )
	ROM_RELOAD ( 0x400001, 0x80000 )
	ROM_RELOAD ( 0x500001, 0x80000 )
	ROM_RELOAD ( 0x600001, 0x80000 )
	ROM_RELOAD ( 0x700001, 0x80000 )
	ROM_RELOAD ( 0x800001, 0x80000 )
	ROM_RELOAD ( 0x900001, 0x80000 )
	ROM_RELOAD ( 0xa00001, 0x80000 )
	ROM_RELOAD ( 0xb00001, 0x80000 )
	ROM_RELOAD ( 0xc00001, 0x80000 )
	ROM_RELOAD ( 0xd00001, 0x80000 )
	ROM_RELOAD ( 0xe00001, 0x80000 )
	ROM_RELOAD ( 0xf00001, 0x80000 )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "s87c751.bin",  0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0  ) /* Bg */
	ROM_LOAD( "7.u45",     0x000000, 0x80000, CRC(64ecdde9) SHA1(576ba1169d90970622249e532baa4209bf12de5a) )
	ROM_LOAD( "6.u39",     0x080000, 0x80000, CRC(6928065c) SHA1(ad5b1889bebf0358df0295d6041b798ac53ac625) )
	ROM_LOAD( "5.u42",     0x100000, 0x80000, CRC(8d18bdc6) SHA1(cacc4f475f85438a00ead4911730202e995983a7) )
	ROM_LOAD( "4.u36",     0x180000, 0x80000, CRC(8e15fb9d) SHA1(47917d8aac1bce2e15f36904f5c2534e5b80236b) )

	ROM_REGION( 0x200000, "gfx2", 0  ) /* Sprites */
	ROM_LOAD( "8.u83",     0x000000, 0x80000, CRC(19bb89dd) SHA1(c2a0c32d350a193d366b5086502998281fd0bec4) )
	ROM_LOAD( "9.u84",     0x080000, 0x80000, CRC(d50d86c7) SHA1(7ecbcc03851a8174610f7f5ad889e40543da928e) )
	ROM_LOAD( "10.u85",    0x100000, 0x80000, CRC(681b8ac8) SHA1(ebfeffc091f53af246311574b9c5d83d2716a7be) )
	ROM_LOAD( "11.u86",    0x180000, 0x80000, CRC(e41f89e3) SHA1(e4b39411a4cea6aa6c01564f74bb8e432d382a73) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is the area that gets switched */
	ROM_REGION( 0xc0000, "oki", 0 ) /* OKI Samples */
	ROM_LOAD( "3.u13",       0x00000, 0x40000, CRC(d0a9245f) SHA1(2e840cdd7bdfe7c6f986daf88576de0559597499) )
	ROM_CONTINUE(            0x60000, 0x20000 )
	ROM_CONTINUE(            0xa0000, 0x20000 )
	ROM_COPY( "oki", nullptr, 0x40000, 0x20000)
	ROM_COPY( "oki", nullptr, 0x80000, 0x20000)
ROM_END

// it's a conversion for a sslam pcb
ROM_START( powerbals )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "21.u67", 0x00000, 0x40000, CRC(4e302381) SHA1(5685d15fd3137866093ff13b95a7df2265a8bc64) )
	ROM_LOAD16_BYTE( "22.u66", 0x00001, 0x40000, CRC(89b70599) SHA1(57a5d71e4d8ca62fffe2e81116c5236d2194ae11) )

	ROM_REGION( 0x1000, "audiocpu", 0 )
	ROM_LOAD( "s87c751.bin",  0x0000, 0x0800, CRC(5b8b2d3a) SHA1(c3409243dfc0ca959a80f6890c87b4ce9eb0741d) )

	ROM_REGION( 0x200000, "gfx1", 0  ) /* Bg */
	ROM_LOAD( "26.u45",    0x000000, 0x80000, CRC(fc9d25c7) SHA1(057702753eddffb9e7bff76311c5e8891343174b) )
	ROM_LOAD( "25.u39",    0x080000, 0x80000, CRC(f20ea774) SHA1(fd284a5ee2cd9d1b5db53225bdfb31dc5bd3f581) )
	ROM_LOAD( "24.u42",    0x100000, 0x80000, CRC(e1829809) SHA1(2fdf0b5580609bff0040c909d2e1ff9fae7dcc9c) )
	ROM_LOAD( "23.u36",    0x180000, 0x80000, CRC(7805275e) SHA1(f0499cf4c84704a6de93a2a1a229af6068ad8771) )

	ROM_REGION( 0x200000, "gfx2", 0  ) /* Sprites */
	ROM_LOAD( "27.u83",    0x000000, 0x80000, CRC(92d7d40a) SHA1(81879945790feb9aeb45750e9b5ded3356571503) )
	ROM_LOAD( "28.u84",    0x080000, 0x80000, CRC(90412135) SHA1(499619c72613a1dd63a6504e39b159a18a71f4fa) )
	ROM_LOAD( "29.u85",    0x100000, 0x80000, CRC(e7bcd2e7) SHA1(01a5e5ac5da2fd79a0c9088f775096b9915bae92) )
	ROM_LOAD( "30.u86",    0x180000, 0x80000, CRC(4130694c) SHA1(581d0035ce1624568f635bd79290be6c587a2533) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is the area that gets switched */
	ROM_REGION( 0xc0000, "oki", 0 ) /* OKI Samples */
	ROM_LOAD( "20.i013",     0x00000, 0x40000, CRC(12776dbc) SHA1(9ab9930fd581296642834d2cb4ba65264a588af3) )
	ROM_CONTINUE(            0x60000, 0x20000 )
	ROM_CONTINUE(            0xa0000, 0x20000 )
	ROM_COPY( "oki", nullptr, 0x40000, 0x20000)
	ROM_COPY( "oki", nullptr, 0x80000, 0x20000)
ROM_END

DRIVER_INIT_MEMBER(sslam_state,sslam)
{
	m_track = 0;
	m_melody = 0;
	m_bar = 0;

	save_item(NAME(m_track));
	save_item(NAME(m_melody));
	save_item(NAME(m_bar));
	save_item(NAME(m_snd_bank));

	m_music_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sslam_state::music_playback),this));
}

DRIVER_INIT_MEMBER(sslam_state,powerbls)
{
	save_item(NAME(m_oki_control));
	save_item(NAME(m_oki_command));
	save_item(NAME(m_oki_bank));
}


GAME( 1993, sslam,    0,        sslam,    sslam, sslam_state,    sslam,    ROT0, "Playmark", "Super Slam (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, sslama,   sslam,    sslam,    sslam, sslam_state,    sslam,    ROT0, "Playmark", "Super Slam (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, powerbals,powerbal, powerbls, powerbls, sslam_state, powerbls, ROT0, "Playmark", "Power Balls (Super Slam conversion)", MACHINE_SUPPORTS_SAVE )
