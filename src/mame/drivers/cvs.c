// license:BSD-3-Clause
// copyright-holders:Mike Coates, Couriersud
/***************************************************************************

Century CVS System

MAIN BOARD:

             FLAG LOW               |   FLAG HIGH
------------------------------------+-----------------------------------
1C00-1FFF    SYSTEM RAM             |   SYSTEM RAM
                                    |
                                    |
1800-1BFF    ATTRIBUTE RAM 32X28    |   SCREEN RAM 32X28
1700         2636 1                 |   CHARACTER RAM 1 256BYTES OF 1K*
1600         2636 2                 |   CHARACTER RAM 2 256BYTES OF 1K*
1500         2636 3                 |   CHARACTER RAM 3 256BYTES OF 1K*
1400         BULLET RAM             |   PALETTE RAM 16BYTES
                                    |
                                    |
0000-13FF    PROGRAM ROM'S          |   PROGRAM ROM'S

* Note the upper two address lines are latched using an I/O read. The I/O map only has
  space for 128 character bit maps

The CPU CANNOT read the character PROMs
        ------

                         CVS I/O MAP
                         -----------
ADR14 ADR13 | READ                                      | WRITE
------------+-------------------------------------------+-----------------------------
  1     0   | COLLISION RESET                           | D0 = STARS ON
            |                                           | D1 = SHADE BRIGHTER TO RIGHT
            |                                           | D2 = SCREEN ROTATE
 read/write |                                           | D3 = SHADE BRIGHTER TO LEFT
   Data     |                                           | D4 = LAMP 1 (CN1 1)
            |                                           | D5 = LAMP 2 (CN1 2)
            |                                           | D6 = SHADE BRIGHTER TO BOTTOM
            |                                           | D7 = SHADE BRIGHTER TO TOP
------------+-------------------------------------------+------------------------------
  X     1   | A0-2: 0 STROBE CN2 1                      | VERTICAL SCROLL OFFSET
            |       1 STROBE CN2 11                     |
            |       2 STROBE CN2 2                      |
            |       3 STROBE CN2 3                      |
            |       4 STROBE CN2 4                      |
            |       5 STROBE CN2 12                     |
            |       6 STROBE DIP SW3                    |
            |       7 STROBE DIP SW2                    |
            |                                           |
 read/write | A4-5: CHARACTER PROM/RAM SELECTION MODE   |
  extended  | There are 256 characters in total. The    |
            | higher ones can be user defined in RAM.   |
            | The split between PROM and RAM characters |
            | is variable.                              |
            |                ROM              RAM       |
            | A4 A5 MODE  CHARACTERS       CHARACTERS   |
            | 0  0   0    224 (0-223)      32  (224-255)|
            | 0  1   1    192 (0-191)      64  (192-255)|
            | 1  0   2    256 (0-255)      0            |
            | 1  1   3    128 (0-127)      128 (128-255)|
            |                                           |
            |                                           |
            | A6-7: SELECT CHARACTER RAM's              |
            |       UPPER ADDRESS BITS A8-9             |
            |       (see memory map)                    |
------------+-------------------------------------------+-------------------------------
  0     0   | COLLISION DATA BYTE:                      | SOUND CONTROL PORT
            | D0 = OBJECT 1 AND 2                       |
            | D1 = OBJECT 2 AND 3                       |
 read/write | D2 = OBJECT 1 AND 3                       |
  control   | D3 = ANY OBJECT AND BULLET                |
            | D4 = OBJECT 1 AND CP1 OR CP2              |
            | D5 = OBJECT 2 AND CP1 OR CP2              |
            | D6 = OBJECT 3 AND CP1 OR CP2              |
            | D7 = BULLET AND CP1 OR CP2                |
------------+-------------------------------------------+-------------------------------

Driver by
    Mike Coates

Hardware Info
 Malcolm & Darren

Additional work
    2009 Couriersud

Todo & FIXME:

- the board most probably has discrete circuits. The 393Hz tone used
  for shots (superbike) and collisions (8ball) is just a guess.

***************************************************************************/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "includes/cvs.h"


/* Turn to 1 so all inputs are always available (this shall only be a debug feature) */
#define CVS_SHOW_ALL_INPUTS 0


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


/*************************************
 *
 *  Multiplexed memory access
 *
 *************************************/

WRITE_LINE_MEMBER(cvs_state::write_s2650_flag)
{
	m_s2650_flag = state;
}

READ8_MEMBER(cvs_state::cvs_video_or_color_ram_r)
{
	if (m_s2650_flag)
		return m_video_ram[offset];
	else
		return m_color_ram[offset];
}

WRITE8_MEMBER(cvs_state::cvs_video_or_color_ram_w)
{
	if (m_s2650_flag)
		m_video_ram[offset] = data;
	else
		m_color_ram[offset] = data;
}


READ8_MEMBER(cvs_state::cvs_bullet_ram_or_palette_r)
{
	if (m_s2650_flag)
		return m_palette_ram[offset & 0x0f];
	else
		return m_bullet_ram[offset];
}

WRITE8_MEMBER(cvs_state::cvs_bullet_ram_or_palette_w)
{
	if (m_s2650_flag)
		m_palette_ram[offset & 0x0f] = data;
	else
		m_bullet_ram[offset] = data;
}


READ8_MEMBER(cvs_state::cvs_s2636_0_or_character_ram_r)
{
	if (m_s2650_flag)
		return m_character_ram[(0 * 0x800) | 0x400 | m_character_ram_page_start | offset];
	else
		return m_s2636_0->work_ram_r(space, offset);
}

WRITE8_MEMBER(cvs_state::cvs_s2636_0_or_character_ram_w)
{
	if (m_s2650_flag)
	{
		offset |= (0 * 0x800) | 0x400 | m_character_ram_page_start;
		m_character_ram[offset] = data;
		m_gfxdecode->gfx(1)->mark_dirty((offset / 8) % 256);
	}
	else
		m_s2636_0->work_ram_w(space, offset, data);
}


READ8_MEMBER(cvs_state::cvs_s2636_1_or_character_ram_r)
{
	if (m_s2650_flag)
		return m_character_ram[(1 * 0x800) | 0x400 | m_character_ram_page_start | offset];
	else
		return m_s2636_1->work_ram_r(space, offset);
}

WRITE8_MEMBER(cvs_state::cvs_s2636_1_or_character_ram_w)
{
	if (m_s2650_flag)
	{
		offset |= (1 * 0x800) | 0x400 | m_character_ram_page_start;
		m_character_ram[offset] = data;
		m_gfxdecode->gfx(1)->mark_dirty((offset / 8) % 256);
	}
	else
		m_s2636_1->work_ram_w(space, offset, data);
}


READ8_MEMBER(cvs_state::cvs_s2636_2_or_character_ram_r)
{
	if (m_s2650_flag)
		return m_character_ram[(2 * 0x800) | 0x400 | m_character_ram_page_start | offset];
	else
		return m_s2636_2->work_ram_r(space, offset);
}

WRITE8_MEMBER(cvs_state::cvs_s2636_2_or_character_ram_w)
{
	if (m_s2650_flag)
	{
		offset |= (2 * 0x800) | 0x400 | m_character_ram_page_start;
		m_character_ram[offset] = data;
		m_gfxdecode->gfx(1)->mark_dirty((offset / 8) % 256);
	}
	else
		m_s2636_2->work_ram_w(space, offset, data);
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

INTERRUPT_GEN_MEMBER(cvs_state::cvs_main_cpu_interrupt)
{
	device.execute().set_input_line_vector(0, 0x03);
	generic_pulse_irq_line(device.execute(), 0, 1);

	cvs_scroll_stars();
}


static void cvs_slave_cpu_interrupt( cpu_device *cpu, int state )
{
	cpu->set_input_line_vector(0, 0x03);
	//cpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
	cpu->set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Input port access
 *
 *************************************/

READ8_MEMBER(cvs_state::cvs_input_r)
{
	UINT8 ret = 0;

	/* the upper 4 bits of the address is used to select the character banking attributes */
	m_character_banking_mode = (offset >> 4) & 0x03;
	m_character_ram_page_start = (offset << 2) & 0x300;

	/* the lower 4 (or 3?) bits select the port to read */
	switch (offset & 0x0f)  /* might be 0x07 */
	{
	case 0x00:  ret = ioport("IN0")->read(); break;
	case 0x02:  ret = ioport("IN1")->read(); break;
	case 0x03:  ret = ioport("IN2")->read(); break;
	case 0x04:  ret = ioport("IN3")->read(); break;
	case 0x06:  ret = ioport("DSW3")->read(); break;
	case 0x07:  ret = ioport("DSW2")->read(); break;
	default:    logerror("%04x : CVS: Reading unmapped input port 0x%02x\n", space.device().safe_pc(), offset & 0x0f); break;
	}

	return ret;
}



/*************************************
 *
 *  Timing
 *
 *************************************/
#if 0
READ8_MEMBER(cvs_state::cvs_393hz_clock_r)
{
	return m_cvs_393hz_clock ? 0x80 : 0;
}
#endif

READ8_MEMBER(cvs_state::tms_clock_r)
{
	return m_tms5110->romclk_hack_r(space, 0) ? 0x80 : 0;
}

TIMER_CALLBACK_MEMBER(cvs_state::cvs_393hz_timer_cb)
{
	m_cvs_393hz_clock = !m_cvs_393hz_clock;

	/* quasar.c games use this timer but have no dac3! */
	if (m_dac3 != NULL)
	{
		if (m_dac3_state[2])
			m_dac3->write_unsigned8(m_cvs_393hz_clock * 0xff);
	}
}


void cvs_state::start_393hz_timer()
{
	m_cvs_393hz_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cvs_state::cvs_393hz_timer_cb),this));
	m_cvs_393hz_timer->adjust(attotime::from_hz(30*393), 0, attotime::from_hz(30*393));
}



/*************************************
 *
 *  4-bit DAC
 *
 *************************************/

WRITE8_MEMBER(cvs_state::cvs_4_bit_dac_data_w)
{
	UINT8 dac_value;
	static int old_data[4] = {0,0,0,0};

	if (data != old_data[offset])
	{
		LOG(("4BIT: %02x %02x\n", offset, data));
		old_data[offset] = data;
	}
	m_cvs_4_bit_dac_data[offset] = data >> 7;

	/* merge into D0-D3 */
	dac_value = (m_cvs_4_bit_dac_data[0] << 0) |
				(m_cvs_4_bit_dac_data[1] << 1) |
				(m_cvs_4_bit_dac_data[2] << 2) |
				(m_cvs_4_bit_dac_data[3] << 3);

	/* scale up to a full byte and output */
	m_dac2->write_unsigned8((dac_value << 4) | dac_value);
}

WRITE8_MEMBER(cvs_state::cvs_unknown_w)
{
	/* offset 2 is used in 8ball
	 * offset 0 is used in spacefrt
	 * offset 3 is used in darkwar
	 *
	 * offset 1 is not used (no trace in disassembly)
	 */

	if (data != m_dac3_state[offset])
	{
		if (offset != 2)
			popmessage("Unknown: %02x %02x\n", offset, data);
		m_dac3_state[offset] = data;
	}
}


/*************************************
 *
 *  Speech hardware
 *
 *************************************/


WRITE8_MEMBER(cvs_state::cvs_speech_rom_address_lo_w)
{
	/* assuming that d0-d2 are cleared here */
	m_speech_rom_bit_address = (m_speech_rom_bit_address & 0xf800) | (data << 3);
	LOG(("%04x : CVS: Speech Lo %02x Address = %04x\n", space.device().safe_pc(), data, m_speech_rom_bit_address >> 3));
}

WRITE8_MEMBER(cvs_state::cvs_speech_rom_address_hi_w)
{
	m_speech_rom_bit_address = (m_speech_rom_bit_address & 0x07ff) | (data << 11);
	LOG(("%04x : CVS: Speech Hi %02x Address = %04x\n", space.device().safe_pc(), data, m_speech_rom_bit_address >> 3));
}


READ8_MEMBER(cvs_state::cvs_speech_command_r)
{
	/* FIXME: this was by observation on board ???
	 *          -bit 7 is TMS status (active LO) */
	return ((m_tms5110->ctl_r(space, 0) ^ 1) << 7) | (soundlatch_byte_r(space, 0) & 0x7f);
}


WRITE8_MEMBER(cvs_state::cvs_tms5110_ctl_w)
{
	UINT8 ctl;
	/*
	 * offset 0: CS ?
	 */
	m_tms5110_ctl_data[offset] = (~data >> 7) & 0x01;

	ctl = 0 |                               /* CTL1 */
			(m_tms5110_ctl_data[1] << 1) |  /* CTL2 */
			(m_tms5110_ctl_data[2] << 2) |  /* CTL4 */
			(m_tms5110_ctl_data[1] << 3);   /* CTL8 */

	LOG(("CVS: Speech CTL = %04x %02x %02x\n",  ctl, offset, data));
	m_tms5110->ctl_w(space, 0, ctl);
}


WRITE8_MEMBER(cvs_state::cvs_tms5110_pdc_w)
{
	UINT8 out = ((~data) >> 7) & 1;
	LOG(("CVS: Speech PDC = %02x %02x\n", offset, out));
	m_tms5110->pdc_w(out);
}


READ_LINE_MEMBER(cvs_state::speech_rom_read_bit)
{
	int bit;
	UINT8 *ROM = memregion("speechdata")->base();

	/* before reading the bit, clamp the address to the region length */
	m_speech_rom_bit_address &= ((memregion("speechdata")->bytes() * 8) - 1);
	bit = BIT(ROM[m_speech_rom_bit_address >> 3], m_speech_rom_bit_address & 0x07);

	/* prepare for next bit */
	m_speech_rom_bit_address++;

	return bit;
}


/*************************************
 *
 *  Inter-CPU communications
 *
 *************************************/

WRITE8_MEMBER(cvs_state::audio_command_w)
{
	LOG(("data %02x\n", data));
	/* cause interrupt on audio CPU if bit 7 set */
	soundlatch_byte_w(space, 0, data);
	cvs_slave_cpu_interrupt(m_audiocpu, data & 0x80 ? 1 : 0);
}



/*************************************
 *
 *  Main CPU memory/IO handlers
 *
 *************************************/

static ADDRESS_MAP_START( cvs_main_cpu_map, AS_PROGRAM, 8, cvs_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x13ff) AM_ROM
	AM_RANGE(0x1400, 0x14ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_bullet_ram_or_palette_r, cvs_bullet_ram_or_palette_w) AM_SHARE("bullet_ram")
	AM_RANGE(0x1500, 0x15ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_s2636_2_or_character_ram_r, cvs_s2636_2_or_character_ram_w)
	AM_RANGE(0x1600, 0x16ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_s2636_1_or_character_ram_r, cvs_s2636_1_or_character_ram_w)
	AM_RANGE(0x1700, 0x17ff) AM_MIRROR(0x6000) AM_READWRITE(cvs_s2636_0_or_character_ram_r, cvs_s2636_0_or_character_ram_w)
	AM_RANGE(0x1800, 0x1bff) AM_MIRROR(0x6000) AM_READWRITE(cvs_video_or_color_ram_r, cvs_video_or_color_ram_w) AM_SHARE("video_ram")
	AM_RANGE(0x1c00, 0x1fff) AM_MIRROR(0x6000) AM_RAM
	AM_RANGE(0x2000, 0x33ff) AM_ROM
	AM_RANGE(0x4000, 0x53ff) AM_ROM
	AM_RANGE(0x6000, 0x73ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cvs_main_cpu_io_map, AS_IO, 8, cvs_state )
	AM_RANGE(0x00, 0xff) AM_READ(cvs_input_r) AM_WRITE(cvs_scroll_w)
	AM_RANGE(S2650_DATA_PORT, S2650_DATA_PORT) AM_READWRITE(cvs_collision_clear, cvs_video_fx_w)
	AM_RANGE(S2650_CTRL_PORT, S2650_CTRL_PORT) AM_READ(cvs_collision_r) AM_WRITE(audio_command_w)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ_PORT("SENSE")
ADDRESS_MAP_END

/*************************************
 *
 *  DAC driving CPU memory/IO handlers
 *
 *************************************/

static ADDRESS_MAP_START( cvs_dac_cpu_map, AS_PROGRAM, 8, cvs_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x107f) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x1840, 0x1840) AM_DEVWRITE("dac1", dac_device, write_unsigned8)
	AM_RANGE(0x1880, 0x1883) AM_WRITE(cvs_4_bit_dac_data_w) AM_SHARE("4bit_dac")
	AM_RANGE(0x1884, 0x1887) AM_WRITE(cvs_unknown_w)    AM_SHARE("dac3_state")  /* ???? not connected to anything */
ADDRESS_MAP_END


static ADDRESS_MAP_START( cvs_dac_cpu_io_map, AS_IO, 8, cvs_state )
	/* doesn't look like it is used at all */
	//AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(cvs_393hz_clock_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Speech driving CPU memory/IO handlers
 *
 *************************************/

static ADDRESS_MAP_START( cvs_speech_cpu_map, AS_PROGRAM, 8, cvs_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x1d00, 0x1d00) AM_WRITE(cvs_speech_rom_address_lo_w)
	AM_RANGE(0x1d40, 0x1d40) AM_WRITE(cvs_speech_rom_address_hi_w)
	AM_RANGE(0x1d80, 0x1d80) AM_READ(cvs_speech_command_r)
	AM_RANGE(0x1ddc, 0x1dde) AM_WRITE(cvs_tms5110_ctl_w) AM_SHARE("tms5110_ctl")
	AM_RANGE(0x1ddf, 0x1ddf) AM_WRITE(cvs_tms5110_pdc_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( cvs_speech_cpu_io_map, AS_IO, 8, cvs_state )
/* romclk is much more probable, 393 Hz results in timing issues */
//  AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(cvs_393hz_clock_r)
	AM_RANGE(S2650_SENSE_PORT, S2650_SENSE_PORT) AM_READ(tms_clock_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Standard CVS port definitions
 *
 *************************************/

static INPUT_PORTS_START( cvs )
	PORT_START("IN0")   /* Matrix 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL        /* "Red button" */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                      /* "Red button" */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL        /* "Green button" */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )                      /* "Green button" */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )           /* not sure it's SERVICE1 : it uses "Coin B" coinage and doesn't say "CREDIT" */
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW3")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_HIGH )                  /* can't tell if it's ACTIVE_HIGH or ACTIVE_LOW */
	PORT_DIPUNUSED( 0x02, IP_ACTIVE_HIGH )                  /* can't tell if it's ACTIVE_HIGH or ACTIVE_LOW */
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )                  /* can't tell if it's ACTIVE_HIGH or ACTIVE_LOW */
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )                  /* can't tell if it's ACTIVE_HIGH or ACTIVE_LOW */
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_HIGH )                  /* can't tell if it's ACTIVE_HIGH or ACTIVE_LOW */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_HIGH )                  /* can't tell if it's ACTIVE_HIGH or ACTIVE_LOW */

	PORT_START("SENSE")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static INPUT_PORTS_START( cvs_registration )
	PORT_INCLUDE(cvs)

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Registration" )              /* can't tell what shall be the default value */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Registration Length" )       /* can't tell what shall be the default value */
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x00, "10" )
	/* bits 2 and 3 determine bonus life settings but they might change from game to game - they are sometimes unused */

	PORT_MODIFY("DSW2")
	/* Told to be "Meter Pulses" but I don't know what this means */
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )          /* has an effect when COIN2 is pressed (when COIN1 is pressed, value always 1 */
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "5" )
INPUT_PORTS_END



/*************************************
 *
 *  Games port definitions
 *
 *************************************/

static INPUT_PORTS_START( cosmos )
	PORT_INCLUDE(cvs)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_SERVICE1 */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP   PORT_COCKTAIL */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN PORT_COCKTAIL */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN */
#endif

	PORT_MODIFY("DSW3")
	/* DSW3 bits 0 and 1 stored at 0x7d55 (0, 2, 1, 3) - code at 0x66f3 - not read back */
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "10k only" )                  /* displays "10000" */
	PORT_DIPSETTING(    0x08, "20k only" )                  /* displays "20000" */
	PORT_DIPSETTING(    0x04, "30k only" )                  /* displays "30000" */
	PORT_DIPSETTING(    0x00, "40k only" )                  /* displays "40000" */
INPUT_PORTS_END

static INPUT_PORTS_START( darkwar )
	PORT_INCLUDE(cvs)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_SERVICE1 */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP   PORT_COCKTAIL */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN PORT_COCKTAIL */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN */
#endif

	/* DSW3 bits 0 to 3 are not read */
INPUT_PORTS_END

static INPUT_PORTS_START( spacefrt )
	PORT_INCLUDE(cvs)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_SERVICE1 */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP   PORT_COCKTAIL */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN PORT_COCKTAIL */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN */
#endif

	PORT_MODIFY("DSW3")
	/* DSW3 bits 0 and 1 stored at 0x7d3f (0, 2, 1, 3) - code at 0x6895 - not read back */
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "100k only" )                 /* displays "50000" */
	PORT_DIPSETTING(    0x08, "150k only" )                 /* displays "110000" */
	PORT_DIPSETTING(    0x04, "200k only" )                 /* displays "200000" */
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )             /* displays "200000" */
INPUT_PORTS_END


static INPUT_PORTS_START( 8ball )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "10k only" )                  /* displays "10000" */
	PORT_DIPSETTING(    0x08, "20k only" )                  /* displays "20000" */
	PORT_DIPSETTING(    0x04, "40k only" )                  /* displays "80000" */
	PORT_DIPSETTING(    0x00, "80k only" )                  /* displays "80000" */
	PORT_DIPNAME( 0x20, 0x00, "Colors" )                    /* stored at 0x1ed4 - code at 0x0847 ('8ball') or 0x08af ('8ball1') */
	PORT_DIPSETTING(    0x00, "Palette 1" )                 /* table at 0x0781 ('8ball') or 0x07e9 ('8ball1') - 16 bytes */
	PORT_DIPSETTING(    0x20, "Palette 2" )                 /* table at 0x0791 ('8ball') or 0x07f9 ('8ball1') - 16 bytes */

	/* DSW2 bit 5 stored at 0x1d93 - code at 0x0858 ('8ball') or 0x08c0 ('8ball1') - read back code at 0x0073 */
INPUT_PORTS_END

static INPUT_PORTS_START( logger )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "10k only" )                  /* displays "10000" */
	PORT_DIPSETTING(    0x08, "20k only" )                  /* displays "20000" */
	PORT_DIPSETTING(    0x04, "40k only" )                  /* displays "40000" */
	PORT_DIPSETTING(    0x00, "80k only" )                  /* displays "80000" */
	/* DSW3 bit 5 stored at 0x7dc8 - code at 0x6eb6 - not read back */

	/* DSW2 bit 5 stored at 0x7da1 - code at 0x6ec7 - read back code at 0x0073 */
INPUT_PORTS_END

static INPUT_PORTS_START( dazzler )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "10k only" )                  /* displays "10000" */
	PORT_DIPSETTING(    0x04, "20k only" )                  /* displays "20000" */
	PORT_DIPSETTING(    0x08, "40k only" )                  /* displays "40000" */
	PORT_DIPSETTING(    0x00, "80k only" )                  /* displays "80000" */

	/* DSW2 bit 5 stored at 0x7d9c - code at 0x6b51 - read back code at 0x0099 */
INPUT_PORTS_END

static INPUT_PORTS_START( wallst )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "10k only" )                  /* displays "10000" */
	PORT_DIPSETTING(    0x04, "20k only" )                  /* displays "20000" */
	PORT_DIPSETTING(    0x08, "40k only" )                  /* displays "40000" */
	PORT_DIPSETTING(    0x00, "80k only" )                  /* displays "80000" */

	/* DSW2 bit 5 stored at 0x1e95 - code at 0x1232 - read back code at 0x6054 */
INPUT_PORTS_END

static INPUT_PORTS_START( radarzon )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "100k only" )                 /* displays "100000" */
	PORT_DIPSETTING(    0x04, "200k only" )                 /* displays "200000" */
	PORT_DIPSETTING(    0x08, "400k only" )                 /* displays "400000" */
	PORT_DIPSETTING(    0x00, "800k only" )                 /* displays "800000" */

	/* DSW2 bit 5 stored at 0x3d6e - code at 0x22aa - read back code at 0x00e4 */
INPUT_PORTS_END

static INPUT_PORTS_START( goldbug )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "100k only" )                 /* displays "100000" */
	PORT_DIPSETTING(    0x04, "200k only" )                 /* displays "200000" */
	PORT_DIPSETTING(    0x08, "400k only" )                 /* displays "400000" */
	PORT_DIPSETTING(    0x00, "800k only" )                 /* displays "800000" */

	/* DSW2 bit 5 stored at 0x3d89 - code at 0x3377 - read back code at 0x6054 */
INPUT_PORTS_END

static INPUT_PORTS_START( diggerc )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "50k only" )                  /* displays "50000" */
	PORT_DIPSETTING(    0x04, "100k only" )                 /* displays "100000" */
	PORT_DIPSETTING(    0x08, "150k only" )                 /* displays "150000" */
	PORT_DIPSETTING(    0x00, "200k only" )                 /* displays "200000" */

	/* DSW2 bit 5 stored at 0x3db3 - code at 0x22ad - read back code at 0x00e4 */
INPUT_PORTS_END

static INPUT_PORTS_START( heartatk )
	PORT_INCLUDE(cvs_registration)

	/* DSW3 bits 2 and 3 stored at 0x1c61 (0, 2, 1, 3) - code at 0x0c52
	   read back code at 0x2197 but untested value : bonus life always at 100000 */

	/* DSW2 bit 5 stored at 0x1e76 - code at 0x0c5c - read back code at 0x00e4 */
INPUT_PORTS_END

static INPUT_PORTS_START( hunchbak )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */

	PORT_MODIFY("IN3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP   PORT_COCKTAIL */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN PORT_COCKTAIL */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_UP */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_JOYSTICK_DOWN */
#endif

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "10k only" )                  /* displays "10000" */
	PORT_DIPSETTING(    0x04, "20k only" )                  /* displays "20000" */
	PORT_DIPSETTING(    0x08, "40k only" )                  /* displays "40000" */
	PORT_DIPSETTING(    0x00, "80k only" )                  /* displays "80000" */

	/* hunchbak : DSW2 bit 5 stored at 0x5e97 - code at 0x516c - read back code at 0x6054 */
	/* hunchbaka : DSW2 bit 5 stored at 0x1e97 - code at 0x0c0c - read back code at 0x6054 */
INPUT_PORTS_END

static INPUT_PORTS_START( superbik )
	PORT_INCLUDE(cvs_registration)

	/* DSW3 bits 2 and 3 are not read : bonus life alaways at 5000 */

	/* DSW2 bit 5 stored at 0x1e79 - code at 0x060f - read back code at 0x25bf */
INPUT_PORTS_END

static INPUT_PORTS_START( raiders )
	PORT_INCLUDE(cvs_registration)

	/* DSW3 bits 2 and 3 are not read : bonus life alaways at 100000 */

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_HIGH )                  /* always 4 lives - table at 0x4218 - 2 bytes */
	/* DSW2 bit 5 stored at 0x1e79 - code at 0x1307 - read back code at 0x251d */
INPUT_PORTS_END

static INPUT_PORTS_START( hero )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	/* DSW3 bits 2 and 3 are not read : bonus life alaways at 150000 */

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_HIGH )                  /* always 3 lives - table at 0x4ebb - 2 bytes */
	/* DSW2 bit 5 stored at 0x1e99 - code at 0x0fdd - read back code at 0x0352 */
INPUT_PORTS_END

static INPUT_PORTS_START( huncholy )
	PORT_INCLUDE(cvs_registration)

#if !CVS_SHOW_ALL_INPUTS
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 PORT_COCKTAIL */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )             /* IPT_BUTTON2 */
#endif

	/* DSW3 bits 2 and 3 are not read : bonus life alaways at 20000 */

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_HIGH )                  /* always 3 lives - table at 0x4531 - 2 bytes */
	/* DSW2 bit 5 stored at 0x1e7c - code at 0x067d - read back code at 0x2f95 */
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{ 0, 0x800*8, 0x1000*8 },   /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( cvs )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 256+4 )
	GFXDECODE_ENTRY( NULL,   0x0000, charlayout, 0, 256+4 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_START_MEMBER(cvs_state,cvs)
{
	/* allocate memory */
	if (m_gfxdecode->gfx(1) != NULL)
		m_gfxdecode->gfx(1)->set_source(m_character_ram);

	start_393hz_timer();

	/* register state save */
	save_item(NAME(m_color_ram));
	save_item(NAME(m_palette_ram));
	save_item(NAME(m_character_ram));
	save_item(NAME(m_character_banking_mode));
	save_item(NAME(m_character_ram_page_start));
	save_item(NAME(m_speech_rom_bit_address));
	save_item(NAME(m_s2650_flag));
	save_item(NAME(m_cvs_393hz_clock));
	save_item(NAME(m_collision_register));
	save_item(NAME(m_total_stars));
	save_item(NAME(m_stars_on));
	save_item(NAME(m_scroll_reg));
	save_item(NAME(m_stars_scroll));
}

MACHINE_RESET_MEMBER(cvs_state,cvs)
{
	m_character_banking_mode = 0;
	m_character_ram_page_start = 0;
	m_speech_rom_bit_address = 0;
	m_cvs_393hz_clock = 0;
	m_collision_register = 0;
	m_stars_on = 0;
	m_scroll_reg = 0;
	m_stars_scroll = 0;
}


static MACHINE_CONFIG_START( cvs, cvs_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", S2650, XTAL_14_31818MHz/16)
	MCFG_CPU_PROGRAM_MAP(cvs_main_cpu_map)
	MCFG_CPU_IO_MAP(cvs_main_cpu_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cvs_state, cvs_main_cpu_interrupt)
	MCFG_S2650_FLAG_HANDLER(WRITELINE(cvs_state, write_s2650_flag))

	MCFG_CPU_ADD("audiocpu", S2650, XTAL_14_31818MHz/16)
	MCFG_CPU_PROGRAM_MAP(cvs_dac_cpu_map)
	MCFG_CPU_IO_MAP(cvs_dac_cpu_io_map)

	MCFG_CPU_ADD("speechcpu", S2650, XTAL_14_31818MHz/16)
	MCFG_CPU_PROGRAM_MAP(cvs_speech_cpu_map)
	MCFG_CPU_IO_MAP(cvs_speech_cpu_io_map)

	MCFG_MACHINE_START_OVERRIDE(cvs_state,cvs)
	MCFG_MACHINE_RESET_OVERRIDE(cvs_state,cvs)

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(cvs_state,cvs)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cvs)

	MCFG_PALETTE_ADD("palette", (256+4)*8+8+1)
	MCFG_PALETTE_INDIRECT_ENTRIES(16)
	MCFG_PALETTE_INIT_OWNER(cvs_state,cvs)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_ALWAYS_UPDATE)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 30*8-1, 1*8, 32*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1000))
	MCFG_SCREEN_UPDATE_DRIVER(cvs_state, screen_update_cvs)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("s2636_0", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(CVS_S2636_Y_OFFSET, CVS_S2636_X_OFFSET)

	MCFG_DEVICE_ADD("s2636_1", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(CVS_S2636_Y_OFFSET, CVS_S2636_X_OFFSET)

	MCFG_DEVICE_ADD("s2636_2", S2636, 0)
	MCFG_S2636_WORKRAM_SIZE(0x100)
	MCFG_S2636_OFFSETS(CVS_S2636_Y_OFFSET, CVS_S2636_X_OFFSET)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_DAC_ADD("dac1")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	//MCFG_DAC_ADD("dac1a")
	//MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DAC_ADD("dac2")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DAC_ADD("dac3")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("tms", TMS5100, XTAL_640kHz)
	MCFG_TMS5110_DATA_CB(READLINE(cvs_state, speech_rom_read_bit))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

#define CVS_COMMON_ROMS                                                                                             \
	ROM_REGION( 0x8000, "speechcpu", 0 )                                                                   \
	ROM_LOAD( "5b.bin",     0x0000, 0x0800, CRC(f055a624) SHA1(5dfe89d7271092e665cdd5cd59d15a2b70f92f43) )  \
																											\
	ROM_REGION( 0x0820, "proms", 0 )                                                                    \
	ROM_LOAD( "82s185.10h", 0x0000, 0x0800, CRC(c205bca6) SHA1(ec9bd220e75f7b067ede6139763ef8aca0fb7a29) )  \
	ROM_LOAD( "82s123.10k", 0x0800, 0x0020, CRC(b5221cec) SHA1(71d9830b33b1a8140b0fe1a2ba8024ba8e6e48e0) )
#define CVS_ROM_REGION_SPEECH_DATA(name, len, hash) \
	ROM_REGION( 0x1000, "speechdata", 0 )   \
	ROM_LOAD( name, 0x0000, len, hash )

#define ROM_LOAD_STAGGERED(name, offs, hash)        \
	ROM_LOAD( name, 0x0000 + offs, 0x0400, hash )   \
	ROM_CONTINUE(   0x2000 + offs, 0x0400 )         \
	ROM_CONTINUE(   0x4000 + offs, 0x0400 )         \
	ROM_CONTINUE(   0x6000 + offs, 0x0400 )


ROM_START( huncholy )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "ho-gp1.bin", 0x0000, CRC(4f17cda7) SHA1(ae6fe495c723042c6e060d4ada50aaef1019d5eb) )
	ROM_LOAD_STAGGERED( "ho-gp2.bin", 0x0400, CRC(70fa52c7) SHA1(179813fdc204870d72c0bfa8cd5dbf277e1f67c4) )
	ROM_LOAD_STAGGERED( "ho-gp3.bin", 0x0800, CRC(931934b1) SHA1(08fe5ad3459862246e9ea845abab4e01e1dbd62d) )
	ROM_LOAD_STAGGERED( "ho-gp4.bin", 0x0c00, CRC(af5cd501) SHA1(9a79b173aa41a82faa9f19210d3e18bfa6c593fa) )
	ROM_LOAD_STAGGERED( "ho-gp5.bin", 0x1000, CRC(658e8974) SHA1(30d0ada1cce99a842bad8f5a58630bc1b7048b03) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ho-sdp1.bin", 0x0000, 0x1000, CRC(3efb3ffd) SHA1(be4807c8b4fe23f2247aa3b6ac02285bee1a0520) )

	CVS_ROM_REGION_SPEECH_DATA( "ho-sp1.bin", 0x1000, CRC(3fd39b1e) SHA1(f5d0b2cfaeda994762403f039a6f7933c5525234) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "ho-cp1.bin",  0x0000, 0x0800, CRC(c6c73d46) SHA1(63aba92f77105fedf46337b591b074020bec05d0) )
	ROM_LOAD( "ho-cp2.bin",  0x0800, 0x0800, CRC(e596371c) SHA1(93a0d0ccdf830ae72d070b03b7e2222f4a737ead) )
	ROM_LOAD( "ho-cp3.bin",  0x1000, 0x0800, CRC(11fae1cf) SHA1(5ceabfb1ff1a6f76d1649512f57d7151f5258ecb) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( darkwar )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "dw-gp1.bin", 0x0000, CRC(f10ccf24) SHA1(f694a9016fc935798e5342598e4fd60fbdbc2829) )
	ROM_LOAD_STAGGERED( "dw-gp2.bin", 0x0400, CRC(b77d0483) SHA1(47d126b9ceaf07267c9078a342a860295320b01c) )
	ROM_LOAD_STAGGERED( "dw-gp3.bin", 0x0800, CRC(c01c3281) SHA1(3c272f424f8a35d08b58f203718b579c1abbe63f) )
	ROM_LOAD_STAGGERED( "dw-gp4.bin", 0x0c00, CRC(0b0bffaf) SHA1(48db78d86dc249fb4d7d93b79b2ac269a0c6698e) )
	ROM_LOAD_STAGGERED( "dw-gp5.bin", 0x1000, CRC(7fdbcaff) SHA1(db80d0d8690105ca72df359c1dc1a43952709111) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dw-sdp1.bin", 0x0000, 0x0800, CRC(b385b669) SHA1(79621d3fb3eb4ea6fa8a733faa6f21edeacae186) )

	CVS_ROM_REGION_SPEECH_DATA( "dw-sp1.bin", 0x1000, CRC(ce815074) SHA1(105f24fb776131b30e35488cca29954298559518) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "dw-cp1.bin", 0x0000, 0x0800, CRC(7a0f9f3e) SHA1(0aa787923fbb614f15016d99c03093a59a0bfb88) )
	ROM_LOAD( "dw-cp2.bin", 0x0800, 0x0800, CRC(232e5120) SHA1(76e4d6d17e8108306761604bd56d6269bfc431e1) )
	ROM_LOAD( "dw-cp3.bin", 0x1000, 0x0800, CRC(573e0a17) SHA1(9c7991eac625b287bafb6cf722ffb405a9627e09) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( 8ball )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "8b-gp1.bin", 0x0000, CRC(1b4fb37f) SHA1(df6dd2766a3b70eec0bde0ae1932b35abdab3735) )
	ROM_LOAD_STAGGERED( "8b-gp2.bin", 0x0400, CRC(f193cdb5) SHA1(54fd1a10c1b9da0f9c4d190f95acc11b3c6e7907) )
	ROM_LOAD_STAGGERED( "8b-gp3.bin", 0x0800, CRC(191989bf) SHA1(dea129a4ed06aac453ab1fbbfae14d8048ef270d) )
	ROM_LOAD_STAGGERED( "8b-gp4.bin", 0x0c00, CRC(9c64519e) SHA1(9a5cad7ccf8f1f289da9a6de0edd4e6d4f0b12fb) )
	ROM_LOAD_STAGGERED( "8b-gp5.bin", 0x1000, CRC(c50d0f9d) SHA1(31b6ea6282fec96d9d2fb74129a215d10f12cc9b) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "8b-sdp1.bin", 0x0000, 0x1000, CRC(a571daf4) SHA1(0db5b95db9da27216bbfa8fff84491a7755f9f1a) )

	CVS_ROM_REGION_SPEECH_DATA( "8b-sp1.bin", 0x0800, CRC(1ee167f3) SHA1(40c876a60832456a27108252ba0b9963f9fe70b0) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "8b-cp1.bin", 0x0000, 0x0800, CRC(c1f68754) SHA1(481c8e3dc35300f779b7925fa8a54320688dac54) )
	ROM_LOAD( "8b-cp2.bin", 0x0800, 0x0800, CRC(6ec1d711) SHA1(768df8e621a7b110a963c93402ee01b1c9009286) )
	ROM_LOAD( "8b-cp3.bin", 0x1000, 0x0800, CRC(4a9afce4) SHA1(187e5106aa2d0bdebf6ec9f2b7c2c2f67d47d221) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( 8ball1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "8a-gp1.bin", 0x0000, CRC(b5d3b763) SHA1(23a01bcbd536ba7f773934ea9dedc7dd9f698100) )
	ROM_LOAD_STAGGERED( "8a-gp2.bin", 0x0400, CRC(5e4aa61a) SHA1(aefa79b4c63d1ac5cb000f2c7c5d06e85d58a547) )
	ROM_LOAD_STAGGERED( "8a-gp3.bin", 0x0800, CRC(3dc272fe) SHA1(303184f7c3557be91d6b8e62a9685080444a78c5) )
	ROM_LOAD_STAGGERED( "8a-gp4.bin", 0x0c00, CRC(33afedbf) SHA1(857c743fd81fbd439c204b6adb251db68465cfc3) )
	ROM_LOAD_STAGGERED( "8a-gp5.bin", 0x1000, CRC(b8b3f373) SHA1(e808db4dcac6d8a454e20b561bb4f3a3bb9c6200) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "8b-sdp1.bin", 0x0000, 0x1000, CRC(a571daf4) SHA1(0db5b95db9da27216bbfa8fff84491a7755f9f1a) )

	CVS_ROM_REGION_SPEECH_DATA( "8b-sp1.bin", 0x0800, CRC(1ee167f3) SHA1(40c876a60832456a27108252ba0b9963f9fe70b0) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "8a-cp1.bin", 0x0000, 0x0800, CRC(d9b36c16) SHA1(dbb496102fa2344f19b5d9a3eecdb29c433e4c08) )
	ROM_LOAD( "8a-cp2.bin", 0x0800, 0x0800, CRC(6f66f0ff) SHA1(1e91474973356e97f89b4d9093565747a8331f50) )
	ROM_LOAD( "8a-cp3.bin", 0x1000, 0x0800, CRC(baee8b17) SHA1(9f86f1d5903aeead17cc75dac8a2b892bb375dad) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( hunchbak ) // actual ROM label has "Century Elect. Ltd. (c)1981", and some label has HB/HB2 handwritten
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "hb-gp1.bin", 0x0000, CRC(af801d54) SHA1(68e31561e98f7e2caa337dd764941d08f075b559) )
	ROM_LOAD_STAGGERED( "hb-gp2.bin", 0x0400, CRC(b448cc8e) SHA1(ed94f662c0e08a3a0aca073fbec29ae1fbd0328e) )
	ROM_LOAD_STAGGERED( "hb-gp3.bin", 0x0800, CRC(57c6ea7b) SHA1(8c3ba01ab1917a8c24180ed1c0011dbfed36d406) )
	ROM_LOAD_STAGGERED( "hb-gp4.bin", 0x0c00, CRC(7f91287b) SHA1(9383d885c142417de73879905cbce272ba9514c7) )
	ROM_LOAD_STAGGERED( "hb-gp5.bin", 0x1000, CRC(1dd5755c) SHA1(b1e158d52bd9a238e3e32ed3024e495df2292dcb) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "6c.sdp1", 0x0000, 0x1000, CRC(f9ba2854) SHA1(d041198e2e8b8c3e668bd1610310f8d25c5b1119) )

	CVS_ROM_REGION_SPEECH_DATA( "8a.sp1", 0x0800, CRC(ed1cd201) SHA1(6cc3842dda1bfddc06ffb436c55d14276286bd67) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "11a.cp1", 0x0000, 0x0800, CRC(f256b047) SHA1(02d79882bad37ffdd58ef478e2658a1369c32ebc) )
	ROM_LOAD( "10a.cp2", 0x0800, 0x0800, CRC(b870c64f) SHA1(ce4f8de87568782ce02bba754edff85df7f5c393) )
	ROM_LOAD( "9a.cp3",  0x1000, 0x0800, CRC(9a7dab88) SHA1(cd39a9d4f982a7f49c478db1408d7e07335f2ddc) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( hunchbaka )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "1b.gp1", 0x0000, CRC(c816860b) SHA1(1109639645496d4644564d21c816b8baf8c84cf7) )
	ROM_LOAD_STAGGERED( "2a.gp2", 0x0400, CRC(cab1e524) SHA1(c3fd7ac9ce5893fd2602a15ad0f6e3267a4ca122) )
	ROM_LOAD_STAGGERED( "3a.gp3", 0x0800, CRC(b2adcfeb) SHA1(3090e2c6b945857c1e48dea395015a05c6165cd9) )
	ROM_LOAD_STAGGERED( "4c.gp4", 0x0c00, CRC(229a8b71) SHA1(ea3815eb69d4927da356eada0add8382735feb48) )
	ROM_LOAD_STAGGERED( "5a.gp5", 0x1000, CRC(cb4f0313) SHA1(1ef63cbe62e7a54d45e0afbc398c9d9b601e6403) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "6c.sdp1", 0x0000, 0x1000, CRC(f9ba2854) SHA1(d041198e2e8b8c3e668bd1610310f8d25c5b1119) )

	CVS_ROM_REGION_SPEECH_DATA( "8a.sp1", 0x0800, CRC(ed1cd201) SHA1(6cc3842dda1bfddc06ffb436c55d14276286bd67) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "11a.cp1", 0x0000, 0x0800, CRC(f256b047) SHA1(02d79882bad37ffdd58ef478e2658a1369c32ebc) )
	ROM_LOAD( "10a.cp2", 0x0800, 0x0800, CRC(b870c64f) SHA1(ce4f8de87568782ce02bba754edff85df7f5c393) )
	ROM_LOAD( "9a.cp3",  0x1000, 0x0800, CRC(9a7dab88) SHA1(cd39a9d4f982a7f49c478db1408d7e07335f2ddc) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( wallst )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "ws-gp1.bin", 0x0000, CRC(bdac81b6) SHA1(6ce865d8902e815742a9ecf10d6f9495f376dede) )
	ROM_LOAD_STAGGERED( "ws-gp2.bin", 0x0400, CRC(9ca67cdd) SHA1(575a4d8d037d2a3c07a8f49d93c7cf6781349ec1) )
	ROM_LOAD_STAGGERED( "ws-gp3.bin", 0x0800, CRC(c2f407f2) SHA1(8208064fd0138a6ccacf03275b8d28793245bfd9) )
	ROM_LOAD_STAGGERED( "ws-gp4.bin", 0x0c00, CRC(1e4b2fe1) SHA1(28eda70cc9cf619452729092e68734ab1a5dc7fb) )
	ROM_LOAD_STAGGERED( "ws-gp5.bin", 0x1000, CRC(eec7bfd0) SHA1(6485e9e2e1624118e38892e74f80431820fd9672) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ws-sdp1.bin", 0x0000, 0x1000, CRC(faed2ac0) SHA1(c2c48e24a560d918531e5c17fb109d68bdec850f) )

	CVS_ROM_REGION_SPEECH_DATA( "ws-sp1.bin",  0x0800, CRC(84b72637) SHA1(9c5834320f39545403839fb7088c37177a6c8861) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "ws-cp1.bin", 0x0000, 0x0800, CRC(5aca11df) SHA1(5ef815b5b09445515ff8b958c4ea29f1a221cee1) )
	ROM_LOAD( "ws-cp2.bin", 0x0800, 0x0800, CRC(ca530d85) SHA1(e5a78667c3583d06d8387848323b11e4a91091ec) )
	ROM_LOAD( "ws-cp3.bin", 0x1000, 0x0800, CRC(1e0225d6) SHA1(410795046c64c24de6711b167315308808b54291) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( dazzler )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "dz-gp1.bin", 0x0000, CRC(2c5d75de) SHA1(d121de662e95f2fc362e367cef57e5e70bafd197) )
	ROM_LOAD_STAGGERED( "dz-gp2.bin", 0x0400, CRC(d0db80d6) SHA1(ca57d3a1d516e0afd750a8f05ae51d4ddee60ca0) )
	ROM_LOAD_STAGGERED( "dz-gp3.bin", 0x0800, CRC(d5f07796) SHA1(110bb0e1613db3634513e8456770dd9d43ad7d34) )
	ROM_LOAD_STAGGERED( "dz-gp4.bin", 0x0c00, CRC(84e41a46) SHA1(a1c1fd9ecacf3357f5c7916cf05dc0b79e975137) )
	ROM_LOAD_STAGGERED( "dz-gp5.bin", 0x1000, CRC(2ae59c41) SHA1(a17e9535409e9e91c41f26a3543f44f20c1b07a5) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dz-sdp1.bin", 0x0000, 0x1000, CRC(89847352) SHA1(54037a4d95958c4c3383467d7f4c2c9416b2eb4a) )

	CVS_ROM_REGION_SPEECH_DATA( "dz-sp1.bin", 0x0800, CRC(25da1fc1) SHA1(c14717ec3399ce7dc47a9d42c8ac8f585db770e9) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "dz-cp1.bin", 0x0000, 0x0800, CRC(0a8a9034) SHA1(9df3d4f387bd5ce3d3580ba678aeda1b65634ac2) )
	ROM_LOAD( "dz-cp2.bin", 0x0800, 0x0800, CRC(3868dd82) SHA1(844584c5a80fb8f1797b4aa4e22024e75726293d) )
	ROM_LOAD( "dz-cp3.bin", 0x1000, 0x0800, CRC(755d9ed2) SHA1(a7165a1d12a5a81d8bb941d8ad073e2097c90beb) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( radarzon )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "rd-gp1.bin", 0x0000, CRC(775786ba) SHA1(5ad0f4e774821a7ed73615118ea42132d3b5424b) )
	ROM_LOAD_STAGGERED( "rd-gp2.bin", 0x0400, CRC(9f6be426) SHA1(24b6cf3d826f3aec0e928881f259a5bc6229232b) )
	ROM_LOAD_STAGGERED( "rd-gp3.bin", 0x0800, CRC(61d11b29) SHA1(fe321c1c912b93bbb098d591e5c4ed0b5b72c88e) )
	ROM_LOAD_STAGGERED( "rd-gp4.bin", 0x0c00, CRC(2fbc778c) SHA1(e45ba08156cf03a1c4a1bdfb8569476d0eb05847) )
	ROM_LOAD_STAGGERED( "rd-gp5.bin", 0x1000, CRC(692a99d5) SHA1(122ae802914cb9a41713536f9030cd9377cf3468) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "rd-sdp1.bin", 0x0000, 0x0800, CRC(cd5aea6d) SHA1(f7545b87e71e3108c0dec24a4e91620d006e0602) )

	CVS_ROM_REGION_SPEECH_DATA( "rd-sp1.bin", 0x0800, CRC(43b17734) SHA1(59960f0c48ed24cedb4b4655f97f6f1fdac4445e) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( radarzon1 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "r1-gp1.bin", 0x0000, CRC(7c73c21f) SHA1(1113025ea16cfcc500b9624a031f3d25290db163) )
	ROM_LOAD_STAGGERED( "r1-gp2.bin", 0x0400, CRC(dedbd2ce) SHA1(ef80bf1b4a9561ad7f54e795c78e72664abf0501) )
	ROM_LOAD_STAGGERED( "r1-gp3.bin", 0x0800, CRC(966a49e7) SHA1(6c769ac12fbfb65184131f1ab16240e422125c04) )
	ROM_LOAD_STAGGERED( "r1-gp4.bin", 0x0c00, CRC(f3175bee) SHA1(f4927eea856ae56b1854263666a48e2cfb3ab60d) )
	ROM_LOAD_STAGGERED( "r1-gp5.bin", 0x1000, CRC(7484927b) SHA1(89a67baa91075d2777f2ecd1667ed79175ad57ca) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "rd-sdp1.bin", 0x0000, 0x0800, CRC(cd5aea6d) SHA1(f7545b87e71e3108c0dec24a4e91620d006e0602) )

	CVS_ROM_REGION_SPEECH_DATA( "rd-sp1.bin", 0x0800, CRC(43b17734) SHA1(59960f0c48ed24cedb4b4655f97f6f1fdac4445e) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( radarzont )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "rt-gp1.bin", 0x0000, CRC(43573974) SHA1(854fe7022e9bdd94bb119c014156e9ffdb6682fa) )
	ROM_LOAD_STAGGERED( "rt-gp2.bin", 0x0400, CRC(257a11ce) SHA1(ca7f9260d9879ebce202f83a41838cb6dc9a6480) )
	ROM_LOAD_STAGGERED( "rt-gp3.bin", 0x0800, CRC(e00f3552) SHA1(156765809e4016527039e3d5cc1c320cfce06834) )
	ROM_LOAD_STAGGERED( "rt-gp4.bin", 0x0c00, CRC(d1e824ac) SHA1(f996813f02d32ddcde7f394740bdb3444eacda76) )
	ROM_LOAD_STAGGERED( "rt-gp5.bin", 0x1000, CRC(bc770af8) SHA1(79599b5f2f4d692986862076be1d487b45783c00) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "rd-sdp1.bin", 0x0000, 0x0800, CRC(cd5aea6d) SHA1(f7545b87e71e3108c0dec24a4e91620d006e0602) )

	CVS_ROM_REGION_SPEECH_DATA( "rd-sp1.bin", 0x0800, CRC(43b17734) SHA1(59960f0c48ed24cedb4b4655f97f6f1fdac4445e) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( outline )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "rt-gp1.bin", 0x0000, CRC(43573974) SHA1(854fe7022e9bdd94bb119c014156e9ffdb6682fa) )
	ROM_LOAD_STAGGERED( "rt-gp2.bin", 0x0400, CRC(257a11ce) SHA1(ca7f9260d9879ebce202f83a41838cb6dc9a6480) )
	ROM_LOAD_STAGGERED( "ot-gp3.bin", 0x0800, CRC(699489e1) SHA1(d4b21c294254ee0a451c29ac91028582a52f5ba3) )
	ROM_LOAD_STAGGERED( "ot-gp4.bin", 0x0c00, CRC(c94aca17) SHA1(ea4ab93c52fee37afc7033b4b2acddcdce308f6b) )
	ROM_LOAD_STAGGERED( "ot-gp5.bin", 0x1000, CRC(154712f4) SHA1(90f69e30e1c1d2348d6644406d83d2b2bcfe8171) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ot-sdp1.bin", 0x0000, 0x0800, CRC(739066a9) SHA1(7b3ba8a163d341931bc0385c298d2061fa75e644) )

	CVS_ROM_REGION_SPEECH_DATA( "ot-sp1.bin", 0x1000, CRC(fa21422a) SHA1(a75d13455c65e5a77db02fc87f0c112e329d0d6d) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "rd-cp1.bin", 0x0000, 0x0800, CRC(ed601677) SHA1(efe2b6033f319603ee80ed4ba66d3b3607537b13) )
	ROM_LOAD( "rd-cp2.bin", 0x0800, 0x0800, CRC(35e317ff) SHA1(458550b431ec66006e2966d86a2286905c0495ed) )
	ROM_LOAD( "rd-cp3.bin", 0x1000, 0x0800, CRC(90f2c43f) SHA1(406215217f6f20c1a78f31b2ae3c0a97391e3371) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( goldbug )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "gb-gp1.bin", 0x0000, CRC(8deb7761) SHA1(35f27fb6b5e3f76ddaf2c074b3391931e679df6e) )
	ROM_LOAD_STAGGERED( "gb-gp2.bin", 0x0400, CRC(135036c1) SHA1(9868eae2486687772bf0bf71b82e461a882ae1ab) )
	ROM_LOAD_STAGGERED( "gb-gp3.bin", 0x0800, CRC(d48b1090) SHA1(b3cbfeb4fc2bf1bbe0befab793fcc5e7e6ff804c) )
	ROM_LOAD_STAGGERED( "gb-gp4.bin", 0x0c00, CRC(c8053205) SHA1(7f814b059f6b9c62e8a83c1753da5e8780b09411) )
	ROM_LOAD_STAGGERED( "gb-gp5.bin", 0x1000, CRC(eca17472) SHA1(25c4ca59b4c96a22bc42b41adbf3cc33373cf85e) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "gb-sdp1.bin", 0x0000, 0x1000, CRC(c8a4b39d) SHA1(29fffaa12639f3b19db818ad374d09fbf9c7fb98) )

	CVS_ROM_REGION_SPEECH_DATA( "gb-sp1.bin", 0x0800, CRC(5d0205c3) SHA1(578937058d56e5c9fba8a2204ddbb59a6d23dec7) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "gb-cp1.bin", 0x0000, 0x0800, CRC(80e1ad5a) SHA1(0a577b0faffd9d6807c39175ce213f017a5cc7f8) )
	ROM_LOAD( "gb-cp2.bin", 0x0800, 0x0800, CRC(0a288b29) SHA1(0c6471a3517805a5c873857ff21ca94dfe91c24e) )
	ROM_LOAD( "gb-cp3.bin", 0x1000, 0x0800, CRC(e5bcf8cf) SHA1(7f53b8ee6f87e6c8761d2200e8194a7d16d8c7ac) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( diggerc )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "dig-gp1.bin", 0x0000, CRC(6a67662f) SHA1(70e9814259c1fbaf195004e1d8ce0ab1125d62d0) )
	ROM_LOAD_STAGGERED( "dig-gp2.bin", 0x0400, CRC(aa9f93b5) SHA1(e118ecb9160b7ba4a81b75b6cca56c0f01a9a3af) )
	ROM_LOAD_STAGGERED( "dig-gp3.bin", 0x0800, CRC(4aa4c87c) SHA1(bcbe291e2ca060ecc623702cba1f4189dfa9c105) )
	ROM_LOAD_STAGGERED( "dig-gp4.bin", 0x0c00, CRC(127e6520) SHA1(a4f1813a297616b7f864e235f40a432881fe252b) )
	ROM_LOAD_STAGGERED( "dig-gp5.bin", 0x1000, CRC(76786827) SHA1(43e33c47a42878a58d72051c1edeccf944db4a17) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "dig-sdp1.bin", 0x0000, 0x1000, CRC(f82e51f0) SHA1(52903c19cdf7754894cbae57a16533579737b3d5) )

	CVS_ROM_REGION_SPEECH_DATA( "dig-sp1.bin", 0x0800, CRC(db526ee1) SHA1(afe319e64350b0c54b72394294a6369c885fdb7f) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "dig-cp1.bin", 0x0000, 0x0800, CRC(ca30fb97) SHA1(148f3a6f20b1f256a73e7a1992262116d77cc0a8) )
	ROM_LOAD( "dig-cp2.bin", 0x0800, 0x0800, CRC(bed2334c) SHA1(c93902d01174e13fb9265194e5e44f67b38c5970) )
	ROM_LOAD( "dig-cp3.bin", 0x1000, 0x0800, CRC(46db9b65) SHA1(1c655b4611ab182b6e4a3cdd3ef930e0d4dad0d9) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( superbik )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "sb-gp1.bin", 0x0000, CRC(f0209700) SHA1(7843e8ebcbecb93814863ddd135f5acb0d481043) )
	ROM_LOAD_STAGGERED( "sb-gp2.bin", 0x0400, CRC(1956d687) SHA1(00e261c5b1e1414b45661310c47daeceb3d5f4bf) )
	ROM_LOAD_STAGGERED( "sb-gp3.bin", 0x0800, CRC(ceb27b75) SHA1(56fecc72746113a6611c18663d1b9e0e2daf57b4) )
	ROM_LOAD_STAGGERED( "sb-gp4.bin", 0x0c00, CRC(430b70b3) SHA1(207c4939331c1561d145cbee0538da072aa51f5b) )
	ROM_LOAD_STAGGERED( "sb-gp5.bin", 0x1000, CRC(013615a3) SHA1(1795a4dcc98255ad185503a99f48b7bacb5edc9d) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "sb-sdp1.bin", 0x0000, 0x0800, CRC(e977c090) SHA1(24bd4165434c745c1514d49cc90bcb621fb3a0f8) )

	CVS_ROM_REGION_SPEECH_DATA( "sb-sp1.bin", 0x0800, CRC(0aeb9ccd) SHA1(e7123eed21e4e758bbe1cebfd5aad44a5de45c27) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "sb-cp1.bin", 0x0000, 0x0800, CRC(03ba7760) SHA1(4ed252e2c4ec7cea2199524f7c35a1dc7c44f8d8) )
	ROM_LOAD( "sb-cp2.bin", 0x0800, 0x0800, CRC(04de69f2) SHA1(3ef3b3c159d47230622b6cc45baad8737bd93a90) )
	ROM_LOAD( "sb-cp3.bin", 0x1000, 0x0800, CRC(bb7d0b9a) SHA1(94c72d6961204be9cab351ac854ac9c69b51e79a) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( hero )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "hr-gp1.bin", 0x0000, CRC(82f39788) SHA1(44217dc2312d10fceeb35adf3999cd6f240b60be) )
	ROM_LOAD_STAGGERED( "hr-gp2.bin", 0x0400, CRC(79607812) SHA1(eaab829a2f5bcb8ec92c3f4122cffae31a4a77cb) )
	ROM_LOAD_STAGGERED( "hr-gp3.bin", 0x0800, CRC(2902715c) SHA1(cf63f72681d1dcbdabdf7673ad8f61b5969e4bd1) )
	ROM_LOAD_STAGGERED( "hr-gp4.bin", 0x0c00, CRC(696d2f8e) SHA1(73dd57f0f84e37ae707a89e17253aa3dd0c8b48b) )
	ROM_LOAD_STAGGERED( "hr-gp5.bin", 0x1000, CRC(936a4ba6) SHA1(86cddcfafbd93dcdad3a1f26e280ceb96f779ab0) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "hr-sdp1.bin", 0x0000, 0x0800, CRC(c34ecf79) SHA1(07c96283410b1e7401140094db95800708cf310f) )

	CVS_ROM_REGION_SPEECH_DATA( "hr-sp1.bin", 0x0800, CRC(a5c33cb1) SHA1(447ffb193b0dc4985bae5d8c214a893afd08664b) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "hr-cp1.bin", 0x0000, 0x0800, CRC(2d201496) SHA1(f195aa1b231a0e1752c7da824a10321f0527f8c9) )
	ROM_LOAD( "hr-cp2.bin", 0x0800, 0x0800, CRC(21b61fe3) SHA1(31882003f0557ffc4ec38ae6ee07b5d294b4162c) )
	ROM_LOAD( "hr-cp3.bin", 0x1000, 0x0800, CRC(9c8e3f9e) SHA1(9d949a4d12b45da12b434677670b2b109568564a) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( logger ) // actual ROM label has "Century Elect. Ltd. (c)1981", and LOG3 is handwritten
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "1clog3.gp1", 0x0000, CRC(0022b9ed) SHA1(4b94d2663f802a8140e8eae1b66ee78fdfa654f5) )
	ROM_LOAD_STAGGERED( "2clog3.gp2", 0x0400, CRC(23c5c8dc) SHA1(37fb6a62cb798d96de20078fe4a3af74a2be0e66) )
	ROM_LOAD_STAGGERED( "3clog3.gp3", 0x0800, CRC(f9288f74) SHA1(8bb588194186fc0e0c2d61ed2746542c978ebb76) )
	ROM_LOAD_STAGGERED( "4clog3.gp4", 0x0c00, CRC(e52ef7bf) SHA1(df5509b6847d6b9520a9d83b15083546898a981e) )
	ROM_LOAD_STAGGERED( "5clog3.gp5", 0x1000, CRC(4ee04359) SHA1(a592d4b280ac0ad5f06d68a7809092548261f123) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "6clog3.sdp1", 0x0000, 0x1000, CRC(5af8da17) SHA1(357f02cdf38c6659aca51fa0a8534542fc29623c) )

	CVS_ROM_REGION_SPEECH_DATA( "8clog3.sp1", 0x0800, CRC(74f67815) SHA1(6a26a16c27a7e4d58b611e5127115005a60cff91) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "11clog3.cp1", 0x0000, 0x0800, CRC(e4ede80e) SHA1(62f2bc78106a057b6a8420d40421908df609bf29) )
	ROM_LOAD( "10clog3.cp2", 0x0800, 0x0800, CRC(d3de8e5b) SHA1(f95320e001869c42e51195d9cc11e4f2555e153f) )
	ROM_LOAD( "9clog3.cp3",  0x1000, 0x0800, CRC(9b8d1031) SHA1(87ef12aeae80cc0f240dead651c6222848f8dccc) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( cosmos )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "cs-gp1.bin", 0x0000, CRC(7eb96ddf) SHA1(f7456ee1ace03ab98c4e8128d375464122c4df01) )
	ROM_LOAD_STAGGERED( "cs-gp2.bin", 0x0400, CRC(6975a8f7) SHA1(13192d4eedd843c0c1d7e5c54a3086f71b09fbcb) )
	ROM_LOAD_STAGGERED( "cs-gp3.bin", 0x0800, CRC(76904b13) SHA1(de219999e4a1b72142e71ea707b6250f4732ccb3) )
	ROM_LOAD_STAGGERED( "cs-gp4.bin", 0x0c00, CRC(bdc89719) SHA1(668267d0b05990ff83a9e38a62950d3d725a53b3) )
	ROM_LOAD_STAGGERED( "cs-gp5.bin", 0x1000, CRC(94be44ea) SHA1(e496ea79d177c6d2d79d59f7d45c86b547469c6f) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "cs-sdp1.bin", 0x0000, 0x0800, CRC(b385b669) SHA1(79621d3fb3eb4ea6fa8a733faa6f21edeacae186) )

	CVS_ROM_REGION_SPEECH_DATA( "cs-sp1.bin", 0x1000, CRC(3c7fe86d) SHA1(9ae0b63b231a7092820650a196cde60588bc6b58) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "cs-cp1.bin", 0x0000, 0x0800, CRC(6a48c898) SHA1(c27f7bcdb2fe042ec52d1b9b4b9a4e47c288862d) )
	ROM_LOAD( "cs-cp2.bin", 0x0800, 0x0800, CRC(db0dfd8c) SHA1(f2b0dd43f0e514fdae54e4066606187f45b98e38) )
	ROM_LOAD( "cs-cp3.bin", 0x1000, 0x0800, CRC(01eee875) SHA1(6c41d716b5795f085229d855518862fb85f395a4) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( heartatk )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "ha-gp1.bin", 0x0000, CRC(e8297c23) SHA1(e79ae7e99f904afe90b43a54df7b0e257d65ac0b) )
	ROM_LOAD_STAGGERED( "ha-gp2.bin", 0x0400, CRC(f7632afc) SHA1(ebfc6e12c8b5078e8c448aa25d9de9d39c0baa5e) )
	ROM_LOAD_STAGGERED( "ha-gp3.bin", 0x0800, CRC(a9ce3c6a) SHA1(86ddb27c1c132f3cf5ad4268ea9a458e0da23677) )
	ROM_LOAD_STAGGERED( "ha-gp4.bin", 0x0c00, CRC(090f30a9) SHA1(acd6b0c7358bf4664d0de668853076326e82fd04) )
	ROM_LOAD_STAGGERED( "ha-gp5.bin", 0x1000, CRC(163b3d2d) SHA1(275275b54533e0ce2df6d189619be05a99c68b6d) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "ha-sdp1.bin", 0x0000, 0x1000, CRC(b9c466a0) SHA1(f28c21a15cf6d52123ed7feac4eea2a42ea5e93d) )

	CVS_ROM_REGION_SPEECH_DATA( "ha-sp1.bin", 0x1000, CRC(fa21422a) SHA1(a75d13455c65e5a77db02fc87f0c112e329d0d6d) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "ha-cp1.bin", 0x0000, 0x0800, CRC(2d0f6d13) SHA1(55e45eaf1bf24a7a78a2f34ffc0d99a4c191d138) )
	ROM_LOAD( "ha-cp2.bin", 0x0800, 0x0800, CRC(7f5671bd) SHA1(7f4ae92a96c5a847c113f6f7e8d67d3e5ee0bcb0) )
	ROM_LOAD( "ha-cp3.bin", 0x1000, 0x0800, CRC(35b05ab4) SHA1(f336eb0c674c3d52e84be0f37b70953cce6112dc) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( spacefrt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "sf-gp1.bin", 0x0000, CRC(1158fc3a) SHA1(c1f470324b6ec65c3061f78a6ff8620154f20c09) )
	ROM_LOAD_STAGGERED( "sf-gp2.bin", 0x0400, CRC(8b4e1582) SHA1(5b92082d67f32197c0c61ddd8e1e3feb742195f4) )
	ROM_LOAD_STAGGERED( "sf-gp3.bin", 0x0800, CRC(48f05102) SHA1(72d40cdd0bbc4cfeb6ddf550de0dafc61270d382) )
	ROM_LOAD_STAGGERED( "sf-gp4.bin", 0x0c00, CRC(c5b14631) SHA1(360bed649185a090f7c96adadd7f045ef574865a) )
	ROM_LOAD_STAGGERED( "sf-gp5.bin", 0x1000, CRC(d7eca1b6) SHA1(8444e61827f0153d04c4f9c08416e7ab753d6918) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "sf-sdp1.bin", 0x0000, 0x0800, CRC(339a327f) SHA1(940887cd4660e37537fd9b57aa1ec3a4717ea0cf) )

	CVS_ROM_REGION_SPEECH_DATA( "sf-sp1.bin", 0x1000, CRC(c5628d30) SHA1(d29a5852a1762cbd5f3eba29ae2bf49b3a26f894) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "sf-cp1.bin", 0x0000, 0x0800, CRC(da194a68) SHA1(4215267e91644cf1e1f32f898bc9562bfba711f3) )
	ROM_LOAD( "sf-cp2.bin", 0x0800, 0x0800, CRC(b96977c7) SHA1(8f0fab044f16787bce83562e2b22d962d0a2c209) )
	ROM_LOAD( "sf-cp3.bin", 0x1000, 0x0800, CRC(f5d67b9a) SHA1(a492b41c53b1f28ac5f70969e5f06afa948c1a7d) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( raiders )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "raid4-5a.bin", 0x0000, CRC(1a92a5aa) SHA1(7f6dbbc0ac5ee2ba3efb3a9120cbee89c659b712) )
	ROM_LOAD_STAGGERED( "raid4-4b.bin", 0x0400, CRC(da69e4b9) SHA1(8a2c4130a5db2cd7dbadb220440bb94ed4513bca) )
	ROM_LOAD_STAGGERED( "raid4-3b.bin", 0x0800, CRC(ca794f92) SHA1(f281d88ffc6b7a84f9bcabc06eed99b8ae045eec) )
	ROM_LOAD_STAGGERED( "raid4-2c.bin", 0x0c00, CRC(9de2c085) SHA1(be6157904afd0dc6ef8d97ec01a9557021ac8f3a) )
	ROM_LOAD_STAGGERED( "raid4-1a.bin", 0x1000, CRC(f4db83ed) SHA1(7d519dc628c93f153ccede85e3cf77c012430f38) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "raidr1-6.bin", 0x0000, 0x0800, CRC(6f827e49) SHA1(4fb272616b60fcd468ed4074b94125e30aa46fd3) )

	CVS_ROM_REGION_SPEECH_DATA( "raidr1-8.bin", 0x0800, CRC(b6b90d2e) SHA1(a966fa208b72aec358b7fb277e603e47b6984aa7) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "raid4-11.bin", 0x0000, 0x0800, CRC(5eb7143b) SHA1(a19e803c15593b37ae2e61789f6e16f319620a37) )
	ROM_LOAD( "raid4-10.bin", 0x0800, 0x0800, CRC(391948a4) SHA1(7e20ad4f7e5bf7ad5dcb08ba6475313e2b8b1f03) )
	ROM_LOAD( "raid4-9b.bin", 0x1000, 0x0800, CRC(fecfde80) SHA1(23ea63080b8292fb00a743743cdff1a7ad0a8c6d) )

	CVS_COMMON_ROMS
ROM_END

ROM_START( raidersr3 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_STAGGERED( "raid3gp5.bin", 0x0000, CRC(e169b71c) SHA1(4e8cc8ee3032ab5a7cfc1caba83f01d6a062d0ae) )
	ROM_LOAD_STAGGERED( "raid3gp4.bin", 0x0400, CRC(9bf717ca) SHA1(7109232b7f72a325538fe6d25b8ef55747d1948d) )
	ROM_LOAD_STAGGERED( "raid3gp3.bin", 0x0800, CRC(ac304782) SHA1(01597c2808d8e33bf9f6510fa9d7a5520eebf179) )
	ROM_LOAD_STAGGERED( "raid3gp2.bin", 0x0c00, CRC(1c0fd350) SHA1(df7e64ad77755da4abdc66b08c470dff018d4592) )
	ROM_LOAD_STAGGERED( "raid3gp1.bin", 0x1000, CRC(5ea24ebf) SHA1(96f9b1f26d8f35a1505cf4d45e5d960a9bb8fb74) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "raidr1-6.bin", 0x0000, 0x0800, CRC(6f827e49) SHA1(4fb272616b60fcd468ed4074b94125e30aa46fd3) )

	CVS_ROM_REGION_SPEECH_DATA( "raidr1-8.bin", 0x0800, CRC(b6b90d2e) SHA1(a966fa208b72aec358b7fb277e603e47b6984aa7) )

	ROM_REGION( 0x1800, "gfx1", 0 )
	ROM_LOAD( "raid4-11.bin", 0x0000, 0x0800, CRC(5eb7143b) SHA1(a19e803c15593b37ae2e61789f6e16f319620a37) )
	ROM_LOAD( "raid4-10.bin", 0x0800, 0x0800, CRC(391948a4) SHA1(7e20ad4f7e5bf7ad5dcb08ba6475313e2b8b1f03) )
	ROM_LOAD( "raid4-9b.bin", 0x1000, 0x0800, CRC(fecfde80) SHA1(23ea63080b8292fb00a743743cdff1a7ad0a8c6d) )

	CVS_COMMON_ROMS
ROM_END




/*************************************
 *
 *  Game specific initalization
 *
 *************************************/

DRIVER_INIT_MEMBER(cvs_state,huncholy)
{
	UINT8 *ROM = memregion("maincpu")->base();

	/* patch out protection */
	ROM[0x0082] = 0xc0;
	ROM[0x0083] = 0xc0;
	ROM[0x0084] = 0xc0;
	ROM[0x00b7] = 0xc0;
	ROM[0x00b8] = 0xc0;
	ROM[0x00b9] = 0xc0;
	ROM[0x00d9] = 0xc0;
	ROM[0x00da] = 0xc0;
	ROM[0x00db] = 0xc0;
	ROM[0x4456] = 0xc0;
	ROM[0x4457] = 0xc0;
	ROM[0x4458] = 0xc0;
}


DRIVER_INIT_MEMBER(cvs_state,hunchbaka)
{
	UINT8 *ROM = memregion("maincpu")->base();

	offs_t offs;

	/* data lines D2 and D5 swapped */
	for (offs = 0; offs < 0x7400; offs++)
		ROM[offs] = BITSWAP8(ROM[offs],7,6,2,4,3,5,1,0);
}


DRIVER_INIT_MEMBER(cvs_state,superbik)
{
	UINT8 *ROM = memregion("maincpu")->base();

	/* patch out protection */
	ROM[0x0079] = 0xc0;
	ROM[0x007a] = 0xc0;
	ROM[0x007b] = 0xc0;
	ROM[0x0081] = 0xc0;
	ROM[0x0082] = 0xc0;
	ROM[0x0083] = 0xc0;
	ROM[0x00b6] = 0xc0;
	ROM[0x00b7] = 0xc0;
	ROM[0x00b8] = 0xc0;
	ROM[0x0168] = 0xc0;
	ROM[0x0169] = 0xc0;
	ROM[0x016a] = 0xc0;

	/* and speed up the protection check */
	ROM[0x0099] = 0xc0;
	ROM[0x009a] = 0xc0;
	ROM[0x009b] = 0xc0;
	ROM[0x00bb] = 0xc0;
	ROM[0x00bc] = 0xc0;
	ROM[0x00bd] = 0xc0;
}


DRIVER_INIT_MEMBER(cvs_state,hero)
{
	UINT8 *ROM = memregion("maincpu")->base();

	/* patch out protection */
	ROM[0x0087] = 0xc0;
	ROM[0x0088] = 0xc0;
	ROM[0x0aa1] = 0xc0;
	ROM[0x0aa2] = 0xc0;
	ROM[0x0aa3] = 0xc0;
	ROM[0x0aaf] = 0xc0;
	ROM[0x0ab0] = 0xc0;
	ROM[0x0ab1] = 0xc0;
	ROM[0x0abd] = 0xc0;
	ROM[0x0abe] = 0xc0;
	ROM[0x0abf] = 0xc0;
	ROM[0x4de0] = 0xc0;
	ROM[0x4de1] = 0xc0;
	ROM[0x4de2] = 0xc0;
}


DRIVER_INIT_MEMBER(cvs_state,raiders)
{
	UINT8 *ROM = memregion("maincpu")->base();

	offs_t offs;

	/* data lines D1 and D6 swapped */
	for (offs = 0; offs < 0x7400; offs++)
		ROM[offs] = BITSWAP8(ROM[offs],7,1,5,4,3,2,6,0);

	/* patch out protection */
	ROM[0x010a] = 0xc0;
	ROM[0x010b] = 0xc0;
	ROM[0x010c] = 0xc0;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1981, cosmos,    0,        cvs,     cosmos, driver_device,   0,        ROT90, "Century Electronics", "Cosmos", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, darkwar,   0,        cvs,     darkwar, driver_device,  0,        ROT90, "Century Electronics", "Dark Warrior", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1981, spacefrt,  0,        cvs,     spacefrt, driver_device, 0,        ROT90, "Century Electronics", "Space Fortress (CVS)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, 8ball,     0,        cvs,     8ball, driver_device,    0,        ROT90, "Century Electronics", "Video Eight Ball", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, 8ball1,    8ball,    cvs,     8ball, driver_device,    0,        ROT90, "Century Electronics", "Video Eight Ball (Rev.1)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, logger,    0,        cvs,     logger, driver_device,   0,        ROT90, "Century Electronics", "Logger", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, dazzler,   0,        cvs,     dazzler, driver_device,  0,        ROT90, "Century Electronics", "Dazzler", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, wallst,    0,        cvs,     wallst, driver_device,   0,        ROT90, "Century Electronics", "Wall Street", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, radarzon,  0,        cvs,     radarzon, driver_device, 0,        ROT90, "Century Electronics", "Radar Zone", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, radarzon1, radarzon, cvs,     radarzon, driver_device, 0,        ROT90, "Century Electronics", "Radar Zone (Rev.1)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, radarzont, radarzon, cvs,     radarzon, driver_device, 0,        ROT90, "Century Electronics (Tuni Electro Service Inc)", "Radar Zone (Tuni)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, outline,   radarzon, cvs,     radarzon, driver_device, 0,        ROT90, "Century Electronics", "Outline", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, goldbug,   0,        cvs,     goldbug, driver_device,  0,        ROT90, "Century Electronics", "Gold Bug", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, diggerc,   0,        cvs,     diggerc, driver_device,  0,        ROT90, "Century Electronics", "Digger (CVS)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, heartatk,  0,        cvs,     heartatk, driver_device, 0,        ROT90, "Century Electronics", "Heart Attack", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, hunchbak,  0,        cvs,     hunchbak, driver_device, 0,        ROT90, "Century Electronics", "Hunchback (set 1)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, hunchbaka, hunchbak, cvs,     hunchbak, cvs_state, hunchbaka,    ROT90, "Century Electronics", "Hunchback (set 2)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, superbik,  0,        cvs,     superbik, cvs_state, superbik,     ROT90, "Century Electronics", "Superbike", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, raiders,   0,        cvs,     raiders, cvs_state,  raiders,      ROT90, "Century Electronics", "Raiders", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1983, raidersr3, raiders,  cvs,     raiders, cvs_state,  raiders,      ROT90, "Century Electronics", "Raiders (Rev.3)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, hero,      0,        cvs,     hero, cvs_state,     hero,         ROT90, "Century Electronics / Seatongrove Ltd", "Hero", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // (C) 1984 CVS on titlescreen, (C) 1983 Seatongrove on highscore screen
GAME( 1984, huncholy,  0,        cvs,     huncholy, cvs_state, huncholy,     ROT90, "Century Electronics / Seatongrove Ltd", "Hunchback Olympic", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
