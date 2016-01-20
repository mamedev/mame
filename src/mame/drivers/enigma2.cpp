// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Tomasz Slanina
/********************************************************************

Enigma 2 (c) Zilec Electronics

driver by Pierpaolo Prazzoli and Tomasz Slanina


enigma2 (1981)
 2xZ80 + AY8910
 Original dedicated board

enigma2a (1984?)
 Conversion applied to a Taito Space Invaders Part II board set with 1984 copyrighy. hack / Bootleg ?

enigma2b (1981)
 Conversion like enigma2a, but boots with 1981 copyright and Phantoms II title

TODO:
 - enigma2  - Star blinking frequency
 - enigma2a + enigma2b - bad sound ROM?

*********************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "sound/ay8910.h"

#define LOG_PROT    (0)

/* these values provide a fairly low refresh rate of around 53Hz, but
   they were derived from the schemtics.  The horizontal synch chain
   counts from 0x0c0-0x1ff and the vertical one from 0x0d8-0x1ff.  */

#define MASTER_CLOCK        (10000000)
#define CPU_CLOCK           (MASTER_CLOCK / 4)
#define PIXEL_CLOCK         (MASTER_CLOCK / 2)
#define AY8910_CLOCK        (MASTER_CLOCK / 8)
#define HCOUNTER_START      (0x0c0)
#define HCOUNTER_END        (0x1ff)
#define HTOTAL              (HCOUNTER_END + 1 - HCOUNTER_START)
#define HBEND               (0x000)
#define HBSTART             (0x100)
#define VCOUNTER_START      (0x0d8)
#define VCOUNTER_END        (0x1ff)
#define VTOTAL              (VCOUNTER_END + 1 - VCOUNTER_START)
#define VBEND               (0x048)
#define VBSTART             (VTOTAL)

/* the IRQ line is cleared (active LO) at these vertical sync counter
   values and raised one scan line later */
#define INT_TRIGGER_COUNT_1 (0x10f)
#define INT_TRIGGER_COUNT_2 (0x18f)


class enigma2_state : public driver_device
{
public:
	enigma2_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_colors(*this, "colors"),
		m_stars(*this, "stars"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

	/* misc */
	int m_blink_count;
	UINT8 m_sound_latch;
	UINT8 m_last_sound_data;
	UINT8 m_protection_data;
	UINT8 m_flip_screen;

	emu_timer *m_interrupt_clear_timer;
	emu_timer *m_interrupt_assert_timer;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_region_ptr<UINT8> m_colors;
	optional_region_ptr<UINT8> m_stars;

	DECLARE_READ8_MEMBER(dip_switch_r);
	DECLARE_WRITE8_MEMBER(sound_data_w);
	DECLARE_WRITE8_MEMBER(enigma2_flip_screen_w);
	DECLARE_CUSTOM_INPUT_MEMBER(p1_controls_r);
	DECLARE_CUSTOM_INPUT_MEMBER(p2_controls_r);
	DECLARE_READ8_MEMBER(sound_latch_r);
	DECLARE_WRITE8_MEMBER(protection_data_w);
	DECLARE_DRIVER_INIT(enigma2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_enigma2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_enigma2a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(interrupt_clear_callback);
	TIMER_CALLBACK_MEMBER(interrupt_assert_callback);
	inline UINT16 vpos_to_vysnc_chain_counter( int vpos );
	inline int vysnc_chain_counter_to_vpos( UINT16 counter );
	void create_interrupt_timers(  );
	void start_interrupt_timers(  );
};


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/


UINT16 enigma2_state::vpos_to_vysnc_chain_counter( int vpos )
{
	return vpos + VCOUNTER_START;
}


int enigma2_state::vysnc_chain_counter_to_vpos( UINT16 counter )
{
	return counter - VCOUNTER_START;
}


TIMER_CALLBACK_MEMBER(enigma2_state::interrupt_clear_callback)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(enigma2_state::interrupt_assert_callback)
{
	UINT16 next_counter;
	int next_vpos;

	/* compute vector and set the interrupt line */
	int vpos = m_screen->vpos();
	UINT16 counter = vpos_to_vysnc_chain_counter(vpos);
	UINT8 vector = 0xc7 | ((counter & 0x80) >> 3) | ((~counter & 0x80) >> 4);
	m_maincpu->set_input_line_and_vector(0, ASSERT_LINE, vector);

	/* set up for next interrupt */
	if (counter == INT_TRIGGER_COUNT_1)
		next_counter = INT_TRIGGER_COUNT_2;
	else
		next_counter = INT_TRIGGER_COUNT_1;

	next_vpos = vysnc_chain_counter_to_vpos(next_counter);
	m_interrupt_assert_timer->adjust(m_screen->time_until_pos(next_vpos));
	m_interrupt_clear_timer->adjust(m_screen->time_until_pos(vpos + 1));
}


void enigma2_state::create_interrupt_timers(  )
{
	m_interrupt_clear_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(enigma2_state::interrupt_clear_callback),this));
	m_interrupt_assert_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(enigma2_state::interrupt_assert_callback),this));
}


void enigma2_state::start_interrupt_timers(  )
{
	int vpos = vysnc_chain_counter_to_vpos(INT_TRIGGER_COUNT_1);
	m_interrupt_assert_timer->adjust(m_screen->time_until_pos(vpos));
}



void enigma2_state::machine_start()
{
	create_interrupt_timers();


	save_item(NAME(m_blink_count));
	save_item(NAME(m_sound_latch));
	save_item(NAME(m_last_sound_data));
	save_item(NAME(m_protection_data));
	save_item(NAME(m_flip_screen));
}


void enigma2_state::machine_reset()
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_last_sound_data = 0;
	m_flip_screen = 0;
	m_sound_latch = 0;
	m_blink_count = 0;

	start_interrupt_timers();
}


/*************************************
 *
 *  Video system
 *
 *************************************/

UINT32 enigma2_state::screen_update_enigma2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const rectangle &visarea = screen.visible_area();

	UINT8 x = 0;
	UINT16 bitmap_y = visarea.min_y;
	UINT8 y = (UINT8)vpos_to_vysnc_chain_counter(bitmap_y);
	UINT8 video_data = 0;
	UINT8 fore_color = 0;
	UINT8 star_color = 0;

	while (1)
	{
		UINT8 bit;
		UINT8 color;

		/* read the video RAM */
		if ((x & 0x07) == 0x00)
		{
			offs_t color_map_address = (y >> 3 << 5) | (x >> 3);

			/* the schematics shows it like this, but it doesn't work as this would
			   produce no stars, due to the contents of the PROM -- maybe there is
			   a star disabled bit somewhere that's connected here instead of flip_screen() */
			/* star_map_address = (y >> 4 << 6) | (engima2_flip_screen_get() << 5) | (x >> 3); */
			offs_t star_map_address = (y >> 4 << 6) | 0x20 | (x >> 3);
			if (m_blink_count & 0x08)
				star_map_address |= 0x400;

			offs_t videoram_address = (y << 5) | (x >> 3);

			/* when the screen is flipped, all the video address bits are inverted,
			   and the adder at 16A is activated */
			if (m_flip_screen)
			{
				color_map_address |= 0x400;
				videoram_address = (~videoram_address + 0x0400) & 0x1fff;
			}

			video_data = m_videoram[videoram_address];

			fore_color = m_colors[color_map_address] & 0x07;
			star_color = m_stars[star_map_address] & 0x07;
		}

		/* plot the current pixel */
		if (m_flip_screen)
		{
			bit = video_data & 0x80;
			video_data = video_data << 1;
		}
		else
		{
			bit = video_data & 0x01;
			video_data = video_data >> 1;
		}

		if (bit)
			color = fore_color;
		else
			/* stars only appear at certain positions */
			color = ((x & y & 0x0f) == 0x0f) ? star_color : 0;

		bitmap.pix32(bitmap_y, x) = m_palette->pen_color(color);

		/* next pixel */
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* end of screen? */
			if (bitmap_y == visarea.max_y)
				break;

			/* next row */
			y = y + 1;
			bitmap_y = bitmap_y + 1;
		}
	}

	m_blink_count++;

	return 0;
}


UINT32 enigma2_state::screen_update_enigma2a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 x = 0;
	const rectangle &visarea = screen.visible_area();
	UINT16 bitmap_y = visarea.min_y;
	UINT8 y = (UINT8)vpos_to_vysnc_chain_counter(bitmap_y);
	UINT8 video_data = 0;

	while (1)
	{
		UINT8 bit;
		pen_t pen;

		/* read the video RAM */
		if ((x & 0x07) == 0x00)
		{
			offs_t videoram_address = (y << 5) | (x >> 3);

			/* when the screen is flipped, all the video address bits are inverted,
			   and the adder at 16A is activated */
			if (m_flip_screen)  videoram_address = (~videoram_address + 0x0400) & 0x1fff;

			video_data = m_videoram[videoram_address];
		}

		/* plot the current pixel */
		if (m_flip_screen)
		{
			bit = video_data & 0x80;
			video_data = video_data << 1;
		}
		else
		{
			bit = video_data & 0x01;
			video_data = video_data >> 1;
		}

		pen = bit ? rgb_t::white : rgb_t::black;
		bitmap.pix32(bitmap_y, x) = pen;

		/* next pixel */
		x = x + 1;

		/* end of line? */
		if (x == 0)
		{
			/* end of screen? */
			if (bitmap_y == visarea.max_y)
				break;

			/* next row */
			y = y + 1;
			bitmap_y = bitmap_y + 1;
		}
	}

	return 0;
}



READ8_MEMBER(enigma2_state::dip_switch_r)
{
	UINT8 ret = 0x00;

	if (LOG_PROT) logerror("DIP SW Read: %x at %x (prot data %x)\n", offset, space.device().safe_pc(), m_protection_data);
	switch (offset)
	{
	case 0x01:
		/* For the DIP switches to be read, protection_data must be
		   0xff on reset. The AY8910 reset ensures this. */
		if (m_protection_data != 0xff)
			ret = m_protection_data ^ 0x88;
		else
			ret = ioport("DSW")->read();
		break;

	case 0x02:
		if (space.device().safe_pc() == 0x07e5)
			ret = 0xaa;
		else
			ret = 0xf4;
		break;

	case 0x35:  ret = 0x38; break;
	case 0x51:  ret = 0xaa; break;
	case 0x79:  ret = 0x38; break;
	}

	return ret;
}


WRITE8_MEMBER(enigma2_state::sound_data_w)
{
	/* clock sound latch shift register on rising edge of D2 */
	if (!(data & 0x04) && (m_last_sound_data & 0x04))
		m_sound_latch = (m_sound_latch << 1) | (~data & 0x01);

	m_audiocpu->set_input_line(INPUT_LINE_NMI, (data & 0x02) ? ASSERT_LINE : CLEAR_LINE);

	m_last_sound_data = data;
}


READ8_MEMBER(enigma2_state::sound_latch_r)
{
	return BITSWAP8(m_sound_latch,0,1,2,3,4,5,6,7);
}


WRITE8_MEMBER(enigma2_state::protection_data_w)
{
	if (LOG_PROT) logerror("%s: Protection Data Write: %x\n", machine().describe_context(), data);
	m_protection_data = data;
}


WRITE8_MEMBER(enigma2_state::enigma2_flip_screen_w)
{
	m_flip_screen = ((data >> 5) & 0x01) && ((ioport("DSW")->read() & 0x20) == 0x20);
}


CUSTOM_INPUT_MEMBER(enigma2_state::p1_controls_r)
{
	return ioport("P1CONTROLS")->read();
}


CUSTOM_INPUT_MEMBER(enigma2_state::p2_controls_r)
{
	if (m_flip_screen)
		return ioport("P2CONTROLS")->read();
	else
		return ioport("P1CONTROLS")->read();
}

static ADDRESS_MAP_START( engima2_main_cpu_map, AS_PROGRAM, 8, enigma2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x2000, 0x3fff) AM_MIRROR(0x4000) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x4fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x5000, 0x57ff) AM_READ(dip_switch_r) AM_WRITENOP
	AM_RANGE(0x5800, 0x5800) AM_MIRROR(0x07f8) AM_NOP
	AM_RANGE(0x5801, 0x5801) AM_MIRROR(0x07f8) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x5802, 0x5802) AM_MIRROR(0x07f8) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0x5803, 0x5803) AM_MIRROR(0x07f8) AM_READNOP AM_WRITE(sound_data_w)
	AM_RANGE(0x5804, 0x5804) AM_MIRROR(0x07f8) AM_NOP
	AM_RANGE(0x5805, 0x5805) AM_MIRROR(0x07f8) AM_READNOP AM_WRITE(enigma2_flip_screen_w)
	AM_RANGE(0x5806, 0x5807) AM_MIRROR(0x07f8) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( engima2a_main_cpu_map, AS_PROGRAM, 8, enigma2_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x2000, 0x3fff) AM_MIRROR(0x4000) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x4000, 0x4fff) AM_ROM AM_WRITENOP
	AM_RANGE(0x5000, 0x57ff) AM_READ(dip_switch_r) AM_WRITENOP
	AM_RANGE(0x5800, 0x5fff) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( engima2a_main_cpu_io_map, AS_IO, 8, enigma2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7)
	AM_RANGE(0x00, 0x00) AM_NOP
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0") AM_WRITENOP
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0x03, 0x03) AM_READNOP AM_WRITE(sound_data_w)
	AM_RANGE(0x04, 0x04) AM_NOP
	AM_RANGE(0x05, 0x05) AM_READNOP AM_WRITE(enigma2_flip_screen_w)
	AM_RANGE(0x06, 0x07) AM_NOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( engima2_audio_cpu_map, AS_PROGRAM, 8, enigma2_state )
	AM_RANGE(0x0000, 0x0fff) AM_MIRROR(0x1000) AM_ROM AM_WRITENOP
	AM_RANGE(0x2000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x83ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0xa000, 0xa001) AM_MIRROR(0x1ffc) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x1ffc) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0xa003, 0xa003) AM_MIRROR(0x1ffc) AM_NOP
	AM_RANGE(0xc000, 0xffff) AM_NOP
ADDRESS_MAP_END



static INPUT_PORTS_START( enigma2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, enigma2_state,p1_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x78, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, enigma2_state,p2_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, "Skill level" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, "Number of invaders" )
	PORT_DIPSETTING(    0x40, "16" )
	PORT_DIPSETTING(    0x00, "32" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("P1CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( enigma2a )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, enigma2_state,p1_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, enigma2_state,p2_controls_r, NULL)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, "Skill level" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x14, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )

	PORT_START("P1CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2CONTROLS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static MACHINE_CONFIG_START( enigma2, enigma2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(engima2_main_cpu_map)

	MCFG_CPU_ADD("audiocpu", Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(engima2_audio_cpu_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(enigma2_state, irq0_line_hold, 8*52)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(enigma2_state, screen_update_enigma2)

	MCFG_PALETTE_ADD_3BIT_BGR("palette")

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, AY8910_CLOCK)
	MCFG_AY8910_PORT_A_READ_CB(READ8(enigma2_state, sound_latch_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(enigma2_state, protection_data_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( enigma2a, enigma2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8080, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(engima2a_main_cpu_map)
	MCFG_CPU_IO_MAP(engima2a_main_cpu_io_map)

	MCFG_CPU_ADD("audiocpu", Z80, 2500000)
	MCFG_CPU_PROGRAM_MAP(engima2_audio_cpu_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(enigma2_state, irq0_line_hold, 8*52)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(enigma2_state, screen_update_enigma2a)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, AY8910_CLOCK)
	MCFG_AY8910_PORT_A_READ_CB(READ8(enigma2_state, sound_latch_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(enigma2_state, protection_data_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( enigma2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.5d",         0x0000, 0x0800, CRC(499749de) SHA1(401928ff41d3b4cbb68e6ad3bf3be4a10ae1781f) )
	ROM_LOAD( "2.7d",         0x0800, 0x0800, CRC(173c1329) SHA1(3f1ad46d0e58ab236e4ff2b385d09fbf113627da) )
	ROM_LOAD( "3.8d",         0x1000, 0x0800, CRC(c7d3e6b1) SHA1(43f7c3a02b46747998260d5469248f21714fe12b) )
	ROM_LOAD( "4.10d",        0x1800, 0x0800, CRC(c6a7428c) SHA1(3503f09856655c5973fb89f60d1045fe41012aa9) )
	ROM_LOAD( "5.11d",        0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "6.13d",        0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "enigma2.s",    0x0000, 0x1000, CRC(68fd8c54) SHA1(69996d5dfd996f0aacb26e397bef314204a2a88a) )

	ROM_REGION( 0x0800, "colors", 0 )
	ROM_LOAD( "7.11f",        0x0000, 0x0800, CRC(409b5aad) SHA1(1b774a70f725637458ed68df9ed42476291b0e43) )

	ROM_REGION( 0x0800, "stars", 0 )
	ROM_LOAD( "8.13f",        0x0000, 0x0800, CRC(e9cb116d) SHA1(41da4f46c5614ec3345c233467ebad022c6b0bf5) )
ROM_END


ROM_START( enigma2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "36_en1.bin",   0x0000, 0x0800, CRC(15f44806) SHA1(4a2f7bc91d4edf7a069e0865d964371c97af0a0a) )
	ROM_LOAD( "35_en2.bin",   0x0800, 0x0800, CRC(e841072f) SHA1(6ab02fd9fdeac5ab887cd25eee3d6b70ab01f849) )
	ROM_LOAD( "34_en3.bin",   0x1000, 0x0800, CRC(43d06cf4) SHA1(495af05d54c0325efb67347f691e64d194645d85) )
	ROM_LOAD( "33_en4.bin",   0x1800, 0x0800, CRC(8879a430) SHA1(c97f44bef3741eef74e137d2459e79f1b3a90457) )
	ROM_LOAD( "5.11d",        0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "6.13d",        0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.bin",    0x0000, 0x0800, BAD_DUMP CRC(5f092d3c) SHA1(17c70f6af1b5560a45e6b1bdb330a98b27570fe9) )
ROM_END

ROM_START( enigma2b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic36.bin",   0x0000, 0x0800, CRC(71dc9ecc) SHA1(a41260259cf0a36b01b5e8ad35cf968e920d22d9) )
	ROM_LOAD( "ic35.bin",   0x0800, 0x0800, CRC(e841072f) SHA1(6ab02fd9fdeac5ab887cd25eee3d6b70ab01f849) )
	ROM_LOAD( "ic34.bin",   0x1000, 0x0800, CRC(1384073d) SHA1(7a3a910c0431e680cc952a10a040b02f3df0532a) )
	ROM_LOAD( "ic33.bin",   0x1800, 0x0800, CRC(ac6c2410) SHA1(d35565a5ffe795d0c36970bd9c2f948bf79e0ed8) )
	ROM_LOAD( "ic32.bin",   0x4000, 0x0800, CRC(098ac15b) SHA1(cce28a2540a9eabb473391fff92895129ae41751) )
	ROM_LOAD( "ic42.bin",   0x4800, 0x0800, CRC(240a9d4b) SHA1(ca1c69fafec0471141ce1254ddfaef54fecfcbf0) )

	/* this rom was completely broken on this pcb.. */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound.bin",    0x0000, 0x0800, BAD_DUMP CRC(5f092d3c) SHA1(17c70f6af1b5560a45e6b1bdb330a98b27570fe9) )
ROM_END


DRIVER_INIT_MEMBER(enigma2_state,enigma2)
{
	offs_t i;
	UINT8 *rom = memregion("audiocpu")->base();

	for(i = 0; i < 0x2000; i++)
	{
		rom[i] = BITSWAP8(rom[i],4,5,6,0,7,1,3,2);
	}
}



GAME( 1981, enigma2,  0,       enigma2,  enigma2, enigma2_state,  enigma2, ROT270, "Game Plan (Zilec Electronics license)", "Enigma II", MACHINE_SUPPORTS_SAVE )
GAME( 1984, enigma2a, enigma2, enigma2a, enigma2a, enigma2_state, enigma2, ROT270, "Zilec Electronics", "Enigma II (Space Invaders hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, enigma2b, enigma2, enigma2a, enigma2a, enigma2_state, enigma2, ROT270, "Zilec Electronics", "Phantoms II (Space Invaders hardware)", MACHINE_SUPPORTS_SAVE )
