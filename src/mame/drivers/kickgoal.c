// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Driver Info


Kick Goal (c)1995 TCH
Action Hollywood (c)1995 TCH

 prelim driver by David Haywood


todo:

Sound - Not possible without PIC dump?
  the PIC is protected, sound will have to be simulated
  the kickgoal sound rom is also bad.

Should the screen size really be doubled in kickgoal or should the fg tiles be 8bpp instead
because otherwise these don't seem much like the same hardware..

Both games have problems with the Eeprom (settings are not saved)


*/

/* Notes

68k interrupts
lev 1 : 0x64 : 0000 0000 - x
lev 2 : 0x68 : 0000 0000 - x
lev 3 : 0x6c : 0000 0000 - x
lev 4 : 0x70 : 0000 0000 - x
lev 5 : 0x74 : 0000 0000 - x
lev 6 : 0x78 : 0000 0510 - vblank?
lev 7 : 0x7c : 0000 0000 - x

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "includes/kickgoal.h"

/**************************************************************************
   This table converts commands sent from the main CPU, into sample numbers
   played back by the sound processor.
   All commentry and most sound effects are correct, however the music
   tracks may be playing at the wrong times.
   Accordingly, the commands for playing the below samples is just a guess:
   1A, 1B, 1C, 1D, 1E, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 6A, 6B, 6C
   Note: that samples 60, 61 and 62 combine to form a music track.
   Ditto for samples 65, 66, 67 and 68.
*/

#ifdef UNUSED_DEFINITION
static const UINT8 kickgoal_cmd_snd[128] =
{
/*00*/  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
/*08*/  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x70, 0x71,
/*10*/  0x72, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
/*18*/  0x15, 0x16, 0x17, 0x18, 0x19, 0x73, 0x74, 0x75,
/*20*/  0x76, 0x1a, 0x1b, 0x1c, 0x1d, 0x00, 0x1f, 0x6c,
/*28*/  0x1e, 0x65, 0x00, 0x00, 0x60, 0x20, 0x69, 0x65,
/*30*/  0x00, 0x00, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
/*38*/  0x29, 0x2a, 0x2b, 0x00, 0x6b, 0x00, 0x00, 0x00
};
#endif

/* Sound numbers in the sample ROM
01 Melody A     Bank 0
02 Melody B     Bank 0
03 Melody C     Bank 1
04 Melody D     Bank 1
05 Melody E     Bank 2
06 Melody F     Bank 2

20 Kick
21 Kick (loud)
22 Bounce
23 Bounce (loud)
24 Hit post
25 Close door
26 Gunshot
27 "You've scored!"
28 "Goal"
29 "Goal" (loud)
2a Kick (loud)
2b Throw ball
2c Coin
2d Crowd
2e Crowd (loud)
2f 27 - 29
30 Goal (in room?)
31 2B - 2D
32 2D - 2E
33 Crowd (short)
34 Crowd (shortest)

****************************************************************
Hollywood Action

01-19 Samples
21-26 Melodies Bank 0
41-48 Melodies Bank 1
61-63 Melodies Bank 2

*/


#define oki_time_base 0x08


#ifdef UNUSED_FUNCTION

//static int kickgoal_sound;
//static int kickgoal_melody;
//static int kickgoal_snd_bank;

void ::kickgoal_play(okim6295_device *oki, int melody, int data)
{
	int status = oki->read(0);

	logerror("Playing sample %01x:%02x from command %02x\n",kickgoal_snd_bank,kickgoal_sound,data);
	if (kickgoal_sound == 0) popmessage("Unknown sound command %02x",kickgoal_sound);

	if (melody) {
		if (m_melody != kickgoal_sound) {
			m_melody      = kickgoal_sound;
			m_melody_loop = kickgoal_sound;
			if (status & 0x08)
				oki->write(0,0x40);
			oki->write(0,(0x80 | m_melody));
			oki->write(0,0x81);
		}
	}
	else {
		if ((status & 0x01) == 0) {
		oki->write(0,(0x80 | kickgoal_sound));
			oki->write(0,0x11);
		}
		else if ((status & 0x02) == 0) {
		oki->write(0,(0x80 | kickgoal_sound));
			oki->write(0,0x21);
		}
		else if ((status & 0x04) == 0) {
		oki->write(0,(0x80 | kickgoal_sound));
			oki->write(0,0x41);
		}
	}
}

WRITE16_MEMBER(kickgoal_state::kickgoal_snd_w)
{
	if (ACCESSING_BITS_0_7)
	{
		logerror("PC:%06x Writing %04x to Sound CPU\n",space.device().safe_pcbase(),data);
		if (data >= 0x40) {
			if (data == 0xfe) {
				m_oki->write(0,0x40); /* Stop playing the melody */
				m_melody      = 0x00;
				m_melody_loop = 0x00;
			}
			else {
				logerror("Unknown command (%02x) sent to the Sound controller\n",data);
			}
		}
		else if (data == 0) {
			m_oki->write(0,0x38);     /* Stop playing effects */
		}
		else {
			kickgoal_sound = kickgoal_cmd_snd[data];

			if (kickgoal_sound >= 0x70) {
				if (kickgoal_snd_bank != 1)
					m_oki->set_bank_base((1 * 0x40000));
				kickgoal_snd_bank = 1;
				kickgoal_play(m_oki, 0, data);
			}
			else if (kickgoal_sound >= 0x69) {
				if (kickgoal_snd_bank != 2)
					m_oki->set_bank_base((2 * 0x40000));
				kickgoal_snd_bank = 2;
				kickgoal_play(m_oki, 4, data);
			}
			else if (kickgoal_sound >= 0x65) {
				if (kickgoal_snd_bank != 1)
					m_oki->set_bank_base((1 * 0x40000));
				kickgoal_snd_bank = 1;
				kickgoal_play(m_oki, 4, data);
			}
			else if (kickgoal_sound >= 0x60) {
				kickgoal_snd_bank = 0;
					m_oki->set_bank_base(device, (0 * 0x40000));
				kickgoal_snd_bank = 0;
				kickgoal_play(m_oki, 4, data);
			}
			else {
				kickgoal_play(m_oki, 0, data);
			}
		}
	}
}
#endif

WRITE16_MEMBER(kickgoal_state::actionhw_snd_w)
{
	logerror("%s: Writing %04x to Sound CPU - mask %04x\n",machine().describe_context(),data,mem_mask);

	if (!ACCESSING_BITS_0_7)
		data >>= 8;

	switch (data)
	{
		case 0xfc:  m_oki->set_bank_base((0 * 0x40000)); break;
		case 0xfd:  m_oki->set_bank_base((2 * 0x40000)); break;
		case 0xfe:  m_oki->set_bank_base((1 * 0x40000)); break;
		case 0xff:  m_oki->set_bank_base((3 * 0x40000)); break;
		case 0x78:
				m_oki->write_command(data);
				m_snd_sam[0] = 00; m_snd_sam[1]= 00; m_snd_sam[2] = 00; m_snd_sam[3] = 00;
				break;
		default:
				if (m_snd_new) /* Play new sample */
				{
					if ((data & 0x80) && (m_snd_sam[3] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read_status() & 0x08) != 0x08)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write_command(m_snd_new);
							m_oki->write_command(data);
						}
						m_snd_new = 00;
					}
					if ((data & 0x40) && (m_snd_sam[2] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read_status() & 0x04) != 0x04)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write_command(m_snd_new);
							m_oki->write_command(data);
						}
						m_snd_new = 00;
					}
					if ((data & 0x20) && (m_snd_sam[1] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read_status() & 0x02) != 0x02)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write_command(m_snd_new);
							m_oki->write_command(data);
						}
						m_snd_new = 00;
					}
					if ((data & 0x10) && (m_snd_sam[0] != m_snd_new))
					{
						logerror("About to play sample %02x at vol %02x\n", m_snd_new, data);
						if ((m_oki->read_status() & 0x01) != 0x01)
						{
							logerror("Playing sample %02x at vol %02x\n", m_snd_new, data);
							m_oki->write_command(m_snd_new);
							m_oki->write_command(data);
						}
						m_snd_new = 00;
					}
					break;
				}
				else if (data > 0x80) /* New sample command */
				{
					logerror("Next sample %02x\n", data);
					m_snd_new = data;
					break;
				}
				else /* Turn a channel off */
				{
					logerror("Turning channel %02x off\n", data);
					m_oki->write_command(data);
					if (data & 0x40) m_snd_sam[3] = 00;
					if (data & 0x20) m_snd_sam[2] = 00;
					if (data & 0x10) m_snd_sam[1] = 00;
					if (data & 0x08) m_snd_sam[0] = 00;
					m_snd_new = 00;
					break;
				}
	}
}


INTERRUPT_GEN_MEMBER(kickgoal_state::kickgoal_interrupt)
{
	if ((m_adpcm->read_status() & 0x08) == 0)
	{
		switch(m_melody_loop)
		{
			case 0x060: m_melody_loop = 0x061; break;
			case 0x061: m_melody_loop = 0x062; break;
			case 0x062: m_melody_loop = 0x060; break;

			case 0x065: m_melody_loop = 0x165; break;
			case 0x165: m_melody_loop = 0x265; break;
			case 0x265: m_melody_loop = 0x365; break;
			case 0x365: m_melody_loop = 0x066; break;
			case 0x066: m_melody_loop = 0x067; break;
			case 0x067: m_melody_loop = 0x068; break;
			case 0x068: m_melody_loop = 0x065; break;

			case 0x063: m_melody_loop = 0x063; break;
			case 0x064: m_melody_loop = 0x064; break;
			case 0x069: m_melody_loop = 0x069; break;
			case 0x06a: m_melody_loop = 0x06a; break;
			case 0x06b: m_melody_loop = 0x06b; break;
			case 0x06c: m_melody_loop = 0x06c; break;

			default:    m_melody_loop = 0x00; break;
		}

		if (m_melody_loop)
		{
//          logerror("Changing to sample %02x\n", m_melody_loop);
			m_adpcm->write_command((0x80 | m_melody_loop) & 0xff);
			m_adpcm->write_command(0x81);
		}
	}
	if (machine().input().code_pressed_once(KEYCODE_PGUP))
	{
		if (m_m6295_key_delay >= (0x60 * oki_time_base))
		{
			m_m6295_bank += 0x01;
			m_m6295_bank &= 0x03;
			if (m_m6295_bank == 0x03)
				m_m6295_bank = 0x00;
			popmessage("Changing Bank to %02x", m_m6295_bank);
			m_adpcm->set_bank_base(((m_m6295_bank) * 0x40000));

			if (m_m6295_key_delay == 0xffff)
				m_m6295_key_delay = 0x00;
			else
				m_m6295_key_delay = (0x30 * oki_time_base);
		}
		else
			m_m6295_key_delay += (0x01 * oki_time_base);
	}
	else if (machine().input().code_pressed_once(KEYCODE_PGDN))
	{
		if (m_m6295_key_delay >= (0x60 * oki_time_base))
		{
			m_m6295_bank -= 0x01;
			m_m6295_bank &= 0x03;
			if (m_m6295_bank == 0x03)
				m_m6295_bank = 0x02;
			popmessage("Changing Bank to %02x", m_m6295_bank);
			m_adpcm->set_bank_base(((m_m6295_bank) * 0x40000));

			if (m_m6295_key_delay == 0xffff)
				m_m6295_key_delay = 0x00;
			else
				m_m6295_key_delay = (0x30 * oki_time_base);
		}
		else
			m_m6295_key_delay += (0x01 * oki_time_base);
	}
	else if (machine().input().code_pressed_once(KEYCODE_INSERT))
	{
		if (m_m6295_key_delay >= (0x60 * oki_time_base))
		{
			m_m6295_comm += 1;
			m_m6295_comm &= 0x7f;
			if (m_m6295_comm == 0x00) { m_adpcm->set_bank_base((0 * 0x40000)); m_m6295_bank = 0; }
			if (m_m6295_comm == 0x60) { m_adpcm->set_bank_base((0 * 0x40000)); m_m6295_bank = 0; }
			if (m_m6295_comm == 0x65) { m_adpcm->set_bank_base((1 * 0x40000)); m_m6295_bank = 1; }
			if (m_m6295_comm == 0x69) { m_adpcm->set_bank_base((2 * 0x40000)); m_m6295_bank = 2; }
			if (m_m6295_comm == 0x70) { m_adpcm->set_bank_base((1 * 0x40000)); m_m6295_bank = 1; }
			popmessage("Sound test command %02x on Bank %02x", m_m6295_comm, m_m6295_bank);

			if (m_m6295_key_delay == 0xffff)
				m_m6295_key_delay = 0x00;
			else
				m_m6295_key_delay = (0x5d * oki_time_base);
		}
		else
			m_m6295_key_delay += (0x01 * oki_time_base);
	}
	else if (machine().input().code_pressed_once(KEYCODE_DEL))
	{
		if (m_m6295_key_delay >= (0x60 * oki_time_base))
		{
			m_m6295_comm -= 1;
			m_m6295_comm &= 0x7f;
			if (m_m6295_comm == 0x2b) { m_adpcm->set_bank_base((0 * 0x40000)); m_m6295_bank = 0; }
			if (m_m6295_comm == 0x64) { m_adpcm->set_bank_base((0 * 0x40000)); m_m6295_bank = 0; }
			if (m_m6295_comm == 0x68) { m_adpcm->set_bank_base((1 * 0x40000)); m_m6295_bank = 1; }
			if (m_m6295_comm == 0x6c) { m_adpcm->set_bank_base((2 * 0x40000)); m_m6295_bank = 2; }
			if (m_m6295_comm == 0x76) { m_adpcm->set_bank_base((1 * 0x40000)); m_m6295_bank = 1; }
			popmessage("Sound test command %02x on Bank %02x", m_m6295_comm, m_m6295_bank);

			if (m_m6295_key_delay == 0xffff)
				m_m6295_key_delay = 0x00;
			else
				m_m6295_key_delay = (0x5d * oki_time_base);
		}
		else
			m_m6295_key_delay += (0x01 * oki_time_base);
	}
	else if (machine().input().code_pressed_once(KEYCODE_Z))
	{
		if (m_m6295_key_delay >= (0x80 * oki_time_base))
		{
			m_adpcm->write_command(0x78);
			m_adpcm->write_command(0x80 | m_m6295_comm);
			m_adpcm->write_command(0x11);

			popmessage("Playing sound %02x on Bank %02x", m_m6295_comm, m_m6295_bank);

			if (m_m6295_key_delay == 0xffff)
				m_m6295_key_delay = 0x00;
			else
				m_m6295_key_delay = (0x60 * oki_time_base);
		}
		else
			m_m6295_key_delay += (0x01 * oki_time_base);
//      logerror("Sending %02x to the sound CPU\n", m_m6295_comm);
	}
	else
		m_m6295_key_delay = 0xffff;
}


static const UINT16 kickgoal_default_eeprom_type1[64] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};



READ16_MEMBER(kickgoal_state::kickgoal_eeprom_r)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_eeprom->do_read();
	}
	return 0;
}


WRITE16_MEMBER(kickgoal_state::kickgoal_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		switch (offset)
		{
			case 0:
				m_eeprom->cs_write((data & 0x0001) ? ASSERT_LINE : CLEAR_LINE);
				break;
			case 1:
				m_eeprom->clk_write((data & 0x0001) ? ASSERT_LINE : CLEAR_LINE);
				break;
			case 2:
				m_eeprom->di_write(data & 0x0001);
				break;
		}
	}
}


/* Memory Maps *****************************************************************/

static ADDRESS_MAP_START( kickgoal_program_map, AS_PROGRAM, 16, kickgoal_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
/// AM_RANGE(0x30001e, 0x30001f) AM_WRITE(kickgoal_snd_w)
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("P1_P2")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("SYSTEM")
/// AM_RANGE(0x800004, 0x800005) AM_WRITE(soundlatch_word_w)
	AM_RANGE(0x800004, 0x800005) AM_WRITE(actionhw_snd_w)
	AM_RANGE(0x900000, 0x900005) AM_WRITE(kickgoal_eeprom_w)
	AM_RANGE(0x900006, 0x900007) AM_READ(kickgoal_eeprom_r)
	AM_RANGE(0xa00000, 0xa03fff) AM_RAM_WRITE(kickgoal_fgram_w) AM_SHARE("fgram") /* FG Layer */
	AM_RANGE(0xa04000, 0xa07fff) AM_RAM_WRITE(kickgoal_bgram_w) AM_SHARE("bgram") /* Higher BG Layer */
	AM_RANGE(0xa08000, 0xa0bfff) AM_RAM_WRITE(kickgoal_bg2ram_w) AM_SHARE("bg2ram") /* Lower BG Layer */
	AM_RANGE(0xa0c000, 0xa0ffff) AM_RAM // more tilemap?
	AM_RANGE(0xa10000, 0xa1000f) AM_WRITEONLY AM_SHARE("scrram") /* Scroll Registers */
	AM_RANGE(0xb00000, 0xb007ff) AM_WRITEONLY AM_SHARE("spriteram") /* Sprites */
	AM_RANGE(0xc00000, 0xc007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* Palette */ // actionhw reads this
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


/* INPUT ports ***************************************************************/

static INPUT_PORTS_START( kickgoal )
	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x0800, IP_ACTIVE_LOW )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/* GFX Decodes ***************************************************************/

static const gfx_layout fg88_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8  },  // note 1*3, 3*8, 5*8 etc. not used, the pixel data is the same, CPS1-like
	16*8
};

static const gfx_layout fg88_alt_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8, 5*8, 6*8,  7*8 },
	8*8
};


static const gfx_layout bg1616_charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
	{ STEP16(0,16) },
	16*16
};


static const gfx_layout bg3232_charlayout =
{
	32,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
	},
	{ STEP32(0,32) },
	32*32,
};

static GFXDECODE_START( kickgoal )
	GFXDECODE_ENTRY( "gfx1", 0, fg88_charlayout,    0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx1", 0, bg1616_charlayout,  0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx1", 0, bg3232_charlayout,  0x000, 0x40 )
GFXDECODE_END

static GFXDECODE_START( actionhw )
	GFXDECODE_ENTRY( "gfx1", 0, fg88_alt_charlayout,    0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx1", 0, bg1616_charlayout,      0x000, 0x40 )
GFXDECODE_END

/* MACHINE drivers ***********************************************************/

void kickgoal_state::machine_start()
{
	save_item(NAME(m_snd_sam));
	save_item(NAME(m_melody_loop));
	save_item(NAME(m_snd_new));
	save_item(NAME(m_m6295_comm));
	save_item(NAME(m_m6295_bank));
	save_item(NAME(m_m6295_key_delay));
}

void kickgoal_state::machine_reset()
{
	m_melody_loop = 0;
	m_snd_new = 0;
	m_snd_sam[0] = 0;
	m_snd_sam[1] = 0;
	m_snd_sam[2] = 0;
	m_snd_sam[3] = 0;
	m_m6295_comm = 0;
	m_m6295_bank = 0;
	m_m6295_key_delay = 0;
}

static MACHINE_CONFIG_START( kickgoal, kickgoal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(kickgoal_program_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kickgoal_state,  irq6_line_hold)
	MCFG_CPU_PERIODIC_INT_DRIVER(kickgoal_state, kickgoal_interrupt,  240)

	MCFG_CPU_ADD("audiocpu", PIC16C57, 12000000/4)  /* 3MHz ? */
	MCFG_DEVICE_DISABLE()   /* Disables since the internal rom isn't dumped */
	/* Program and Data Maps are internal to the MCU */

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(kickgoal_default_eeprom_type1, 128)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(9*8, 55*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(kickgoal_state, screen_update_kickgoal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kickgoal)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(kickgoal_state,kickgoal)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", 12000000/8, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( actionhw, kickgoal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(kickgoal_program_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kickgoal_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_12MHz/3)    /* verified on pcb */
	MCFG_DEVICE_DISABLE() /* Disables since the internal rom isn't dumped */
	/* Program and Data Maps are internal to the MCU */

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(kickgoal_default_eeprom_type1, 128)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(10*8+2, 54*8-1+2, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(kickgoal_state, screen_update_kickgoal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", actionhw)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_VIDEO_START_OVERRIDE(kickgoal_state,actionhw)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", XTAL_12MHz/12, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END



/* Rom Loading ***************************************************************/

ROM_START( kickgoal )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "ic6",   0x000000, 0x40000, CRC(498ca792) SHA1(c638c3a1755870010c5961b58bcb02458ff4e238) )
	ROM_LOAD16_BYTE( "ic5",   0x000001, 0x40000, CRC(d528740a) SHA1(d56a71004aabc839b0833a6bf383e5ef9d4948fa) )

	ROM_REGION( 0x1000, "audiocpu", 0 ) /* sound? (missing) */
	/* Remove the CPU_DISABLED flag in MACHINE_DRIVER when the rom is dumped */
	ROM_LOAD( "pic16c57",     0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ic33",   0x000000, 0x80000, CRC(5038f52a) SHA1(22ed0e2c8a99056e73cff912731626689996a276) )
	ROM_LOAD( "ic34",   0x080000, 0x80000, CRC(06e7094f) SHA1(e41b893ef91d541d2623d76ce6c69ecf4218c16d) )
	ROM_LOAD( "ic35",   0x100000, 0x80000, CRC(ea010563) SHA1(5e474db372550e9d33f933ab00881a9b29a712d1) )
	ROM_LOAD( "ic36",   0x180000, 0x80000, CRC(b6a86860) SHA1(73ab43830d5e62154bc8953615cdb397c7a742aa) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is the area that gets switched */
	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "ic13",        0x00000, 0x40000, BAD_DUMP CRC(c6cb56e9) SHA1(835773b3f0647d3c553180bcf10e57ad44d68353) ) // BAD ADDRESS LINES (mask=010000)
	ROM_CONTINUE(            0x60000, 0x20000 )
	ROM_CONTINUE(            0xa0000, 0x20000 )
	ROM_COPY( "oki", 0x00000, 0x40000, 0x20000)
	ROM_COPY( "oki", 0x00000, 0x80000, 0x20000)
	ROM_COPY( "oki", 0x00000, 0xc0000, 0x20000)
ROM_END

ROM_START( actionhw )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.ic6",  0x000000, 0x80000, CRC(2b71d58c) SHA1(3e58531fa56d41a3c7944e3beab4850907564a89) )
	ROM_LOAD16_BYTE( "1.ic5",  0x000001, 0x80000, CRC(136b9711) SHA1(553f9fdd99bb9ce2e1492d0755633075e59ba587) )

	ROM_REGION( 0x1000, "audiocpu", 0 ) /* sound? (missing) */
	/* Remove the CPU_DISABLED flag in MACHINE_DRIVER when the rom is dumped */
	ROM_LOAD( "pic16c57",     0x0000, 0x0800, NO_DUMP )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "4.ic29",  0x000000, 0x80000, CRC(df076744) SHA1(4b2c8e21a201e1491e4ba3cda8d71b51e0943431) )
	ROM_LOAD( "5.ic33",  0x080000, 0x80000, CRC(8551fdd4) SHA1(f29bdfb75af7607534de171d7b3927419c00377c) )
	ROM_LOAD( "6.ic30",  0x100000, 0x80000, CRC(5cb005a5) SHA1(d3a5ab8f9a520bfaa53fdf6145142ccba416fbb8) )
	ROM_LOAD( "7.ic34",  0x180000, 0x80000, CRC(c2f7d284) SHA1(b3c3d6aa932c813affd667344ea5ddefa55f219b) )
	ROM_LOAD( "8.ic31",  0x200000, 0x80000, CRC(50dffa47) SHA1(33da3b2cabb7b0e480158d343e876563bd0f0930) )
	ROM_LOAD( "9.ic35",  0x280000, 0x80000, CRC(c1ea0370) SHA1(c836611e478d2bf9ae2a5d7e7665982c2b731189) )
	ROM_LOAD( "10.ic32", 0x300000, 0x80000, CRC(5ee5db3e) SHA1(c79f84548ce5311acac478c5180330bf56485863) )
	ROM_LOAD( "11.ic36", 0x380000, 0x80000, CRC(8d376b1e) SHA1(37f16b3237d9813a8d153ab5640252e7643f3b99) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is the area that gets switched */
	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	ROM_LOAD( "3.ic13",      0x00000, 0x40000, CRC(b8f6705d) SHA1(55116e14aba6dac7334e26f704b3e6b0b9f856c2) )
	ROM_CONTINUE(            0x60000, 0x20000 )
	ROM_CONTINUE(            0xa0000, 0x20000 )
	ROM_COPY( "oki", 0x00000, 0x40000, 0x20000)
	ROM_COPY( "oki", 0x00000, 0x80000, 0x20000)
	ROM_COPY( "oki", 0x00000, 0xc0000, 0x20000) /* Last bank used in Test Mode */
ROM_END

/* GAME drivers **************************************************************/

DRIVER_INIT_MEMBER(kickgoal_state,kickgoal)
{
#if 0 /* we should find a real fix instead  */
	UINT16 *rom = (UINT16 *)memregion("maincpu")->base();

	/* fix "bug" that prevents game from writing to EEPROM */
	rom[0x12b0/2] = 0x0001;
#endif
}


GAME( 1995, kickgoal,0, kickgoal, kickgoal, kickgoal_state, kickgoal, ROT0, "TCH", "Kick Goal", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, actionhw,0, actionhw, kickgoal, kickgoal_state, kickgoal, ROT0, "TCH", "Action Hollywood", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
