// license:BSD-3-Clause
// copyright-holders:Lee Taylor
/***************************************************************************

    Astro Fighter hardware

    driver by Lee Taylor 28/11/1997

    Games supported:
        * Astro Fighter (3 sets)
        * Astro Battle (2 sets)
        * Astro Fire
        * Astro Combat (2 sets)
        * Super Star Battle
        * Space Fighter Mark II (2 sets)
        * Tomahawk 777 late version with changed game play
        * Tomahawk 777 early version

    Notes:
        * Astro Battle added by HIGHWAYMAN with help from Reip.
          2 sets, 1 may be a bad set, or they may simply be different - don't know yet.
          protection involves the EPROM datalines being routed through an 8bit x 256byte PROM.
          it *may* have a more complex palette, it needs to be investigated when i get more time.

        * Astro Fighter set differences:

          The differences are minor. From newest to oldest:

          Main Set: 16Kbit ROMs
                    Green/Hollow empty fuel bar.
                    60 points for every bomb destroyed.

          Set 2:    8Kbit ROMs
                    Blue/Solid empty fuel bar.
                    60 points for every bomb destroyed.

          Set 3:    8Kbit ROMs
                    Blue/Solid empty fuel bar.
                   300 points for every seven bombs destroyed.

        * I know there must be at least one other ROM set for Astro Fighter
          I have played one that stoped between waves to show the next enemy

    Known issues/to-do's:
        * Analog sound in all games

    About Colours:
        * It was fairly common to have wiremods on these PCBs to change the
          background colours, this is why you see Astro Fighter and Tomahawk
          games with both Blue and Black backgrounds.  By default MAME
          emulates an unmodified PCB, you can enable the hacks in the DRIVER
          CONFIGURATION menu.

          Versions of Tomahawk using the Astro Fighter PROM have been seen,
          see notes in ROM loading.


****************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/astrof.h"


#define MASTER_CLOCK        (XTAL_10_595MHz)
#define MAIN_CPU_CLOCK      (MASTER_CLOCK / 16)
#define PIXEL_CLOCK         (MASTER_CLOCK / 2)
#define HTOTAL              (0x150)
#define HBEND               (0x000)
#define HBSTART             (0x100)
#define VTOTAL              (0x118)
#define VBEND               (0x000)
#define VBSTART             (0x100)



/*************************************
 *
 *  IRQ generation
 *
 *************************************/

READ8_MEMBER(astrof_state::irq_clear_r)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(astrof_state::irq_callback)
{
	m_maincpu->set_input_line(0, ASSERT_LINE);
}



/*************************************
 *
 *  Input handling
 *
 *************************************/

INPUT_CHANGED_MEMBER(astrof_state::coin_inserted)
{
	/* coin insertion causes an NMI */
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
	machine().bookkeeping().coin_counter_w(0, newval);
}


INPUT_CHANGED_MEMBER(astrof_state::service_coin_inserted)
{
	/* service coin insertion causes an NMI */
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}


CUSTOM_INPUT_MEMBER(astrof_state::astrof_p1_controls_r)
{
	return ioport("P1")->read();
}


CUSTOM_INPUT_MEMBER(astrof_state::astrof_p2_controls_r)
{
	UINT32 ret;

	/* on an upright cabinet, a single set of controls
	   is connected to both sets of pins on the edge
	   connector */
	if (ioport("CAB")->read())
		ret = ioport("P2")->read();
	else
		ret = ioport("P1")->read();

	return ret;
}


CUSTOM_INPUT_MEMBER(astrof_state::tomahawk_controls_r)
{
	UINT32 ret;

	/* on a cocktail cabinet, two sets of controls are
	   multiplexed on a single set of inputs
	     (not verified on pcb) */

	if (m_flipscreen)
		ret = ioport("P2")->read();
	else
		ret = ioport("P1")->read();

	return ret;
}



/*************************************
 *
 *  Video system
 *
 *************************************/

#define ASTROF_NUM_PENS     (0x10)
#define TOMAHAWK_NUM_PENS   (0x20)


void astrof_state::video_start()
{
	/* allocate the color RAM -- half the size of the video RAM as A0 is not connected */
	m_colorram = std::make_unique<UINT8[]>(m_videoram.bytes() / 2);
	save_pointer(NAME(m_colorram.get()), m_videoram.bytes() / 2);
}


rgb_t astrof_state::make_pen( UINT8 data )
{
	UINT8 r1_bit = m_red_on ? 0x01 : (data >> 0) & 0x01;
	UINT8 r2_bit = m_red_on ? 0x01 : (data >> 1) & 0x01;
	UINT8 g1_bit = (data >> 2) & 0x01;
	UINT8 g2_bit = (data >> 3) & 0x01;
	UINT8 b1_bit = (data >> 4) & 0x01;
	UINT8 b2_bit = (data >> 5) & 0x01;

	/* this is probably not quite right, but I don't have the
	   knowledge to figure out the actual weights - ZV */
	UINT8 r = (0xc0 * r1_bit) + (0x3f * r2_bit);
	UINT8 g = (0xc0 * g1_bit) + (0x3f * g2_bit);
	UINT8 b = (0xc0 * b1_bit) + (0x3f * b2_bit);

	return rgb_t(r, g, b);
}


void astrof_state::astrof_get_pens( pen_t *pens )
{
	offs_t i;
	UINT8 bank = (m_astrof_palette_bank ? 0x10 : 0x00);
	UINT8 config = read_safe(ioport("FAKE"), 0x00);
	UINT8 *prom = memregion("proms")->base();

	/* a common wire hack to the pcb causes the prom halves to be inverted */
	/* this results in e.g. astrof background being black */
	switch (config)
	{
	case 0:
		/* normal PROM access */
		break;
	case 1:
		/* invert PROM acess */
		bank ^= 0x10;
		break;
	case 2:
		/* force low */
		bank = 0x00;
		break;
	default:
		/* force high */
		bank = 0x10;
		break;
	}

	for (i = 0; i < ASTROF_NUM_PENS; i++)
	{
		UINT8 data = prom[bank | i];
		pens[i] = make_pen(data);
	}
}


void astrof_state::tomahawk_get_pens( pen_t *pens )
{
	offs_t i;
	UINT8 *prom = memregion("proms")->base();
	UINT8 config = read_safe(ioport("FAKE"), 0x00);

	for (i = 0; i < TOMAHAWK_NUM_PENS; i++)
	{
		UINT8 data;
		UINT8 pen;

		/* a common wire hack to the pcb causes the prom halves to be inverted */
		/* this results in e.g. astrof background being black */
		switch (config)
		{
		case 0:
			/* normal PROM access */
			pen = i;
			break;
		case 1:
			/* invert PROM acess */
			pen = i ^ 0x10;
			break;
		case 2:
			/* force low */
			pen = i & 0x0f;
			break;
		default:
			/* force high */
			pen = i | 0x10;
			break;
		}

		data = prom[pen];

		pens[i] = make_pen(data);
	}
}


WRITE8_MEMBER(astrof_state::astrof_videoram_w)
{
	m_videoram[offset] = data;
	m_colorram[offset >> 1] = *m_astrof_color & 0x0e;
}


WRITE8_MEMBER(astrof_state::tomahawk_videoram_w)
{
	m_videoram[offset] = data;
	m_colorram[offset >> 1] = (*m_astrof_color & 0x0e) | ((*m_astrof_color & 0x01) << 4);
}


WRITE8_MEMBER(astrof_state::video_control_1_w)
{
	m_flipscreen = ((data >> 0) & 0x01) & ioport("CAB")->read();

	/* this ties to the CLR pin of the shift registers */
	m_screen_off = (data & 0x02) ? TRUE : FALSE;

	/* D2 - not connected in the schematics, but at one point Astro Fighter sets it to 1 */
	/* D3-D7 - not connected */

	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::astrof_set_video_control_2( UINT8 data )
{
	/* D0 - OUT0 - goes to edge conn. pin A10 - was perhaps meant to be a start lamp */
	/* D1 - OUT1 - goes to edge conn. pin A11 - was perhaps meant to be a start lamp */

	/* D2 - selects one of the two palette banks */
	m_astrof_palette_bank = (data & 0x04) ? TRUE : FALSE;

	/* D3 - turns on the red color gun regardless of the value in the color PROM */
	m_red_on = (data & 0x08) ? TRUE : FALSE;

	/* D4-D7 - not connected */
}

WRITE8_MEMBER(astrof_state::astrof_video_control_2_w)
{
	astrof_set_video_control_2(data);
	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::spfghmk2_set_video_control_2( UINT8 data )
{
	/* D0 - OUT0 - goes to edge conn. pin A10 - was perhaps meant to be a start lamp */
	/* D1 - OUT1 - goes to edge conn. pin A11 - was perhaps meant to be a start lamp */

	/* D2 - selects one of the two palette banks */
	m_astrof_palette_bank = (data & 0x04) ? TRUE : FALSE;

	/* D3-D7 - not connected */
}

WRITE8_MEMBER(astrof_state::spfghmk2_video_control_2_w)
{
	spfghmk2_set_video_control_2(data);
	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::tomahawk_set_video_control_2( UINT8 data )
{
	/* D0 - OUT0 - goes to edge conn. pin A10 - was perhaps meant to be a start lamp */
	/* D1 - OUT1 - goes to edge conn. pin A11 - was perhaps meant to be a start lamp */
	/* D2 - not connected */

	/* D3 - turns on the red color gun regardless of the value in the color PROM */
	m_red_on = (data & 0x08) ? TRUE : FALSE;
}

WRITE8_MEMBER(astrof_state::tomahawk_video_control_2_w)
{
	tomahawk_set_video_control_2(data);
	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::video_update_common( bitmap_rgb32 &bitmap, const rectangle &cliprect, pen_t *pens, int num_pens )
{
	offs_t offs;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		UINT8 data;
		int i;

		UINT8 color = m_colorram[offs >> 1];

		pen_t back_pen = pens[(color & (num_pens-1)) | 0x00];
		pen_t fore_pen = pens[(color & (num_pens-1)) | 0x01];

		UINT8 y = offs;
		UINT8 x = offs >> 8 << 3;

		if (!m_flipscreen)
			y = ~y;

		if ((y <= cliprect.min_y) || (y >= cliprect.max_y))
			continue;

		if (m_screen_off)
			data = 0;
		else
			data = m_videoram[offs];

		for (i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x01) ? fore_pen : back_pen;

			if (m_flipscreen)
				bitmap.pix32(y, 255 - x) = pen;
			else
				bitmap.pix32(y, x) = pen;

			x = x + 1;
			data = data >> 1;
		}
	}
}


UINT32 astrof_state::screen_update_astrof(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[ASTROF_NUM_PENS];

	astrof_get_pens(pens);

	video_update_common(bitmap, cliprect, pens, ASTROF_NUM_PENS);

	return 0;
}


UINT32 astrof_state::screen_update_tomahawk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[TOMAHAWK_NUM_PENS];

	tomahawk_get_pens(pens);

	video_update_common(bitmap, cliprect, pens, TOMAHAWK_NUM_PENS);

	return 0;
}



/*************************************
 *
 *  Protection
 *
 *************************************/

READ8_MEMBER(astrof_state::shoot_r)
{
	/* not really sure about this */
	return machine().rand() & 8;
}


READ8_MEMBER(astrof_state::abattle_coin_prot_r)
{
	m_abattle_count = (m_abattle_count + 1) % 0x0101;
	return m_abattle_count ? 0x07 : 0x00;
}


READ8_MEMBER(astrof_state::afire_coin_prot_r)
{
	m_abattle_count = m_abattle_count ^ 0x01;
	return m_abattle_count ? 0x07 : 0x00;
}


READ8_MEMBER(astrof_state::tomahawk_protection_r)
{
	/* flip the byte */
	return BITSWAP8(*m_tomahawk_protection, 0, 1, 2, 3, 4, 5, 6, 7);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

MACHINE_START_MEMBER(astrof_state,astrof)
{
	/* the 74175 outputs all HI's if not otherwise set */
	astrof_set_video_control_2(0xff);

	/* register for state saving */
	save_item(NAME(m_red_on));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_screen_off));
	save_item(NAME(m_astrof_palette_bank));
	save_item(NAME(m_port_1_last));
	save_item(NAME(m_port_2_last));
	save_item(NAME(m_astrof_start_explosion));
	save_item(NAME(m_astrof_death_playing));
	save_item(NAME(m_astrof_bosskill_playing));
}


MACHINE_START_MEMBER(astrof_state,abattle)
{
	/* register for state saving */
	save_item(NAME(m_abattle_count));

	MACHINE_START_CALL_MEMBER(astrof);
}


MACHINE_START_MEMBER(astrof_state,spfghmk2)
{
	/* the 74175 outputs all HI's if not otherwise set */
	spfghmk2_set_video_control_2(0xff);


	/* the red background circuit is disabled */
	m_red_on = FALSE;

	/* register for state saving */
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_screen_off));
	save_item(NAME(m_astrof_palette_bank));
}


MACHINE_START_MEMBER(astrof_state,tomahawk)
{
	/* the 74175 outputs all HI's if not otherwise set */
	tomahawk_set_video_control_2(0xff);

	/* register for state saving */
	save_item(NAME(m_red_on));
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_screen_off));
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

MACHINE_RESET_MEMBER(astrof_state,abattle)
{
	m_abattle_count = 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( astrof_map, AS_PROGRAM, 8, astrof_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_NOP
	AM_RANGE(0x4000, 0x5fff) AM_RAM_WRITE(astrof_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x8002) AM_MIRROR(0x1ff8) AM_NOP
	AM_RANGE(0x8003, 0x8003) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITEONLY AM_SHARE("astrof_color")
	AM_RANGE(0x8004, 0x8004) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(video_control_1_w)
	AM_RANGE(0x8005, 0x8005) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(astrof_video_control_2_w)
	AM_RANGE(0x8006, 0x8006) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(astrof_audio_1_w)
	AM_RANGE(0x8007, 0x8007) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(astrof_audio_2_w)
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1ff8) AM_READ_PORT("IN") AM_WRITENOP
	AM_RANGE(0xa001, 0xa001) AM_MIRROR(0x1ff8) AM_READ_PORT("DSW") AM_WRITENOP
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x1ff8) AM_READ(irq_clear_r) AM_WRITENOP
	AM_RANGE(0xa003, 0xa007) AM_MIRROR(0x1ff8) AM_NOP
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( spfghmk2_map, AS_PROGRAM, 8, astrof_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_NOP
	AM_RANGE(0x4000, 0x5fff) AM_RAM_WRITE(astrof_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x8002) AM_MIRROR(0x1ff8) AM_NOP
	AM_RANGE(0x8003, 0x8003) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITEONLY AM_SHARE("astrof_color")
	AM_RANGE(0x8004, 0x8004) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(video_control_1_w)
	AM_RANGE(0x8005, 0x8005) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(spfghmk2_video_control_2_w)
	AM_RANGE(0x8006, 0x8006) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(spfghmk2_audio_w)
	AM_RANGE(0x8007, 0x8007) AM_MIRROR(0x1ff8) AM_NOP
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1ff8) AM_READ_PORT("IN") AM_WRITENOP
	AM_RANGE(0xa001, 0xa001) AM_MIRROR(0x1ff8) AM_READ_PORT("DSW") AM_WRITENOP
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x1ff8) AM_READ(irq_clear_r) AM_WRITENOP
	AM_RANGE(0xa003, 0xa007) AM_MIRROR(0x1ff8) AM_NOP
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( tomahawk_map, AS_PROGRAM, 8, astrof_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_NOP
	AM_RANGE(0x4000, 0x5fff) AM_RAM_WRITE(tomahawk_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x6000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x8002) AM_MIRROR(0x1ff8) AM_NOP
	AM_RANGE(0x8003, 0x8003) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITEONLY AM_SHARE("astrof_color")
	AM_RANGE(0x8004, 0x8004) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(video_control_1_w)
	AM_RANGE(0x8005, 0x8005) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(tomahawk_video_control_2_w)
	AM_RANGE(0x8006, 0x8006) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITE(tomahawk_audio_w)
	AM_RANGE(0x8007, 0x8007) AM_MIRROR(0x1ff8) AM_READNOP AM_WRITEONLY AM_SHARE("tomahawk_prot")
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1ff8) AM_READ_PORT("IN") AM_WRITENOP
	AM_RANGE(0xa001, 0xa001) AM_MIRROR(0x1ff8) AM_READ_PORT("DSW") AM_WRITENOP
	AM_RANGE(0xa002, 0xa002) AM_MIRROR(0x1ff8) AM_READ(irq_clear_r) AM_WRITENOP
	AM_RANGE(0xa003, 0xa003) AM_MIRROR(0x1ff8) AM_READ(tomahawk_protection_r) AM_WRITENOP
	AM_RANGE(0xa004, 0xa007) AM_MIRROR(0x1ff8) AM_NOP
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( astrof_common )
	PORT_START("FAKE")
	/* There are PCB wire-mods which limit / change PROM access */
	PORT_CONFNAME( 0x03, 0x00, "Color PROM Wiremod" )
	PORT_CONFSETTING(    0x00, "Normal (no mod)" )
	PORT_CONFSETTING(    0x01, "Invert Access" )
	PORT_CONFSETTING(    0x02, "Force Low" )
	PORT_CONFSETTING(    0x03, "Force High" )
INPUT_PORTS_END


static INPUT_PORTS_START( astrof )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p1_controls_r, NULL)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p2_controls_r, NULL)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )            /* duplicate settings */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW:5,6")    /* table at 0xf6b2 */
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "7000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,service_coin_inserted, 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* same as 'astrof', but inputs are ACTIVE_HIGH instead of ACTIVE_LOW */
static INPUT_PORTS_START( abattle )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p1_controls_r, NULL)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p2_controls_r, NULL)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )            /* duplicate settings */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW:5,6")    /* table at 0xf87a */
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "7000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,service_coin_inserted, 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spfghmk2 )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p1_controls_r, NULL)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p2_controls_r, NULL)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW:1" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )            /* duplicate settings */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, "Free Credit" )               PORT_DIPLOCATION("SW:5,6")    /* table at 0xfa58 */
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x20, "2500" )
	PORT_DIPSETTING(    0x30, "3000" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW:7" )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,service_coin_inserted, 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spfghmk22 )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p1_controls_r, NULL)
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,astrof_p2_controls_r, NULL)

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xf8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )            /* duplicate settings */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, "Free Credit" )               PORT_DIPLOCATION("SW:5,6")    /* table at 0xf9f8 */
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPSETTING(    0x10, "3000" )
	PORT_DIPSETTING(    0x20, "4000" )
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPNAME( 0x40, 0x00, "Kill Saucer after Invaders" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )              /* if saucer lands, game is over */
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,service_coin_inserted, 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tomahawk )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, astrof_state,tomahawk_controls_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
//  PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )            /* duplicate settings */
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW:5,6")    /* table at 0xf428 */
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x10, "7000" )
	PORT_DIPSETTING(    0x20, "10000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, astrof_state,service_coin_inserted, 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tomahawk1 )
	PORT_INCLUDE( tomahawk )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW:5,6")    /* table at 0xf3c8 */
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x20, "7000" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW:7" )
INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( base, astrof_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MAIN_CPU_CLOCK)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("vblank", astrof_state, irq_callback, "screen", VBSTART, 0)

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( astrof, base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(astrof_map)

	MCFG_MACHINE_START_OVERRIDE(astrof_state,astrof)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(astrof_state, screen_update_astrof)

	/* audio hardware */
	MCFG_FRAGMENT_ADD(astrof_audio)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( abattle, astrof )

	/* basic machine hardware */

	MCFG_MACHINE_START_OVERRIDE(astrof_state,abattle)
	MCFG_MACHINE_RESET_OVERRIDE(astrof_state,abattle)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spfghmk2, base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(spfghmk2_map)

	MCFG_MACHINE_START_OVERRIDE(astrof_state,spfghmk2)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(astrof_state, screen_update_astrof)

	/* audio hardware */
	MCFG_FRAGMENT_ADD(spfghmk2_audio)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tomahawk, base )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tomahawk_map)

	MCFG_MACHINE_START_OVERRIDE(astrof_state,tomahawk)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(astrof_state, screen_update_tomahawk)

	/* audio hardware */
	MCFG_FRAGMENT_ADD(tomahawk_audio)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( astrof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "afii.6",       0xd000, 0x0800, CRC(d6cd13a4) SHA1(359b00b02f4256f1138c8526214c6a34d2e5b47a) )
	ROM_LOAD( "afii.5",       0xd800, 0x0800, CRC(6fd3c4df) SHA1(73aad03e2588ac9f249d5751eb4a7c7cd12fd3b9) )
	ROM_LOAD( "afii.4",       0xe000, 0x0800, CRC(9612dae3) SHA1(8ee1797c212e06c381972b7b555f240ff317d75d) )
	ROM_LOAD( "afii.3",       0xe800, 0x0800, CRC(5a0fef42) SHA1(92a575abdf17bbb5ed6bc67479049523a985aa75) )
	ROM_LOAD( "afii.2",       0xf000, 0x0800, CRC(69f8a4fc) SHA1(9f9a935f19187640018009ade92f8993912ef6c2) )
	ROM_LOAD( "afii.1",       0xf800, 0x0800, CRC(322c09d2) SHA1(89723e3d998ff9cb9b174bca4b072b412b290c04) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


ROM_START( astrof2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kei2",         0xd000, 0x0400, CRC(9f0bd355) SHA1(45db9229dcd8bbd366ff13c683625c3d1c175598) )
	ROM_LOAD( "keii",         0xd400, 0x0400, CRC(71f229f0) SHA1(be426360567066df01fb428dc5cd2d6ef01a4cf7) )
	ROM_LOAD( "kei0",         0xd800, 0x0400, CRC(88114f7c) SHA1(e64ae3cac92d2a3c02edc8e81c88d5d275e89293) )
	ROM_LOAD( "af579.08",     0xdc00, 0x0400, CRC(9793c124) SHA1(ae0352ed13fa21a00181669e92f9e66c938e4843) )
	ROM_LOAD( "ke8",          0xe000, 0x0400, CRC(08e44b12) SHA1(0e156fff081ae74321597eca1a02920bfc464651) )
	ROM_LOAD( "ke7",          0xe400, 0x0400, CRC(8a42d62c) SHA1(f5c0043be113c88f87deee3a2acd7d778a569e4f) )
	ROM_LOAD( "ke6",          0xe800, 0x0400, CRC(3e9aa743) SHA1(5f473afee7a416bb6f4e658cf8e46f8362ae3bba) )
	ROM_LOAD( "ke5",          0xec00, 0x0400, CRC(712a4557) SHA1(66a19378782c3911b8740ca25451ce84e1096fd0) )
	ROM_LOAD( "ke4",          0xf000, 0x0400, CRC(ad06f306) SHA1(d6ab7cba97658a46a63846a203eb89d9fc367e4f) )
	ROM_LOAD( "ke3",          0xf400, 0x0400, CRC(680b91b4) SHA1(004fd0f6564c19277632adec42bcf1054d043e4a) )
	ROM_LOAD( "ke2",          0xf800, 0x0400, CRC(2c4cab1a) SHA1(3171764a17f2c5fda39f0b32ccce60bc107d306e) )
	ROM_LOAD( "af583.00",     0xfc00, 0x0400, CRC(f699dda3) SHA1(e595cb93df40f64f7521afa51a879d53e1d04126) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


ROM_START( astrof3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "kei2",         0xd000, 0x0400, CRC(9f0bd355) SHA1(45db9229dcd8bbd366ff13c683625c3d1c175598) )
	ROM_LOAD( "keii",         0xd400, 0x0400, CRC(71f229f0) SHA1(be426360567066df01fb428dc5cd2d6ef01a4cf7) )
	ROM_LOAD( "kei0",         0xd800, 0x0400, CRC(88114f7c) SHA1(e64ae3cac92d2a3c02edc8e81c88d5d275e89293) )
	ROM_LOAD( "ke9",          0xdc00, 0x0400, CRC(29cbaea6) SHA1(da29e8156218884195b16839be9ad1e98a8348ac) )
	ROM_LOAD( "ke8",          0xe000, 0x0400, CRC(08e44b12) SHA1(0e156fff081ae74321597eca1a02920bfc464651) )
	ROM_LOAD( "ke7",          0xe400, 0x0400, CRC(8a42d62c) SHA1(f5c0043be113c88f87deee3a2acd7d778a569e4f) )
	ROM_LOAD( "ke6",          0xe800, 0x0400, CRC(3e9aa743) SHA1(5f473afee7a416bb6f4e658cf8e46f8362ae3bba) )
	ROM_LOAD( "ke5",          0xec00, 0x0400, CRC(712a4557) SHA1(66a19378782c3911b8740ca25451ce84e1096fd0) )
	ROM_LOAD( "ke4",          0xf000, 0x0400, CRC(ad06f306) SHA1(d6ab7cba97658a46a63846a203eb89d9fc367e4f) )
	ROM_LOAD( "ke3",          0xf400, 0x0400, CRC(680b91b4) SHA1(004fd0f6564c19277632adec42bcf1054d043e4a) )
	ROM_LOAD( "ke2",          0xf800, 0x0400, CRC(2c4cab1a) SHA1(3171764a17f2c5fda39f0b32ccce60bc107d306e) )
	ROM_LOAD( "kei",          0xfc00, 0x0400, CRC(fce4718d) SHA1(3a313328609f6bef644a2d906d8ca74c5d52058b) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


ROM_START( abattle )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10405-b.bin",  0xd000, 0x0400, CRC(9ba57987) SHA1(becf89b7d474f86839f13f9be5502c91491e8584) )
	ROM_LOAD( "10405-a.bin",  0xd400, 0x0400, CRC(3fbbeeba) SHA1(1c9f519a0797f90524adf187b0761f150db0828d) )
	ROM_LOAD( "10405-9.bin",  0xd800, 0x0400, CRC(354cf432) SHA1(138956ea8064eba0dcd8b2f175d4981b689a2077) )
	ROM_LOAD( "10405-8.bin",  0xdc00, 0x0400, CRC(4cee0c8b) SHA1(98bfdda9d2d368db16d6e9090536b09d8337c0e5) )
	ROM_LOAD( "10405-4.bin",  0xe000, 0x0400, CRC(9cb477f3) SHA1(6866264aa8d0479cee237a00e4a919e3981144a5) )
	ROM_LOAD( "10405-6.bin",  0xe400, 0x0400, CRC(272de8f1) SHA1(e917b3b8bb96fedacd6d5cb3d1c30977818f2e85) )
	ROM_LOAD( "10405-5.bin",  0xe800, 0x0400, CRC(ff25acaa) SHA1(5cb360c556c9b36039ae05702e6900b82fe5676b) )
	ROM_LOAD( "10405-3.bin",  0xec00, 0x0400, CRC(6edf202d) SHA1(a4cab2f10a99e0a4b1c571168e17cbee1d18cf06) )
	ROM_LOAD( "10405-7.bin",  0xf000, 0x0400, CRC(02a35ad9) SHA1(d54afff13f8d5a6544dda49c766a147fa0172cfa) )
	ROM_LOAD( "10405-1.bin",  0xf400, 0x0400, CRC(c68f6657) SHA1(a38c24670fcbbf7844ca15f918efcb467bae7bef) )
	ROM_LOAD( "10405-2.bin",  0xf800, 0x0400, CRC(b206deda) SHA1(9ab52920c06ed6beb38bc7f97ffd00e8ad46c17d) )
	ROM_LOAD( "10405-0.bin",  0xfc00, 0x0400, CRC(c836a152) SHA1(418b64d50bb2f849b1e7177c7bf2fdd0cc99e079) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "8f-clr.bin",   0x0000, 0x0100, CRC(3bf3ccb0) SHA1(d61d19d38045f42a9adecf295e479fee239bed48) )

	ROM_REGION( 0x0100, "user1", 0 )    /* decryption table */
	ROM_LOAD( "2h-prot.bin",  0x0000, 0x0100, CRC(a6bdd18c) SHA1(438bfc543730afdb531204585f17a68ddc03ded0) )
ROM_END


ROM_START( abattle2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "10405-b.bin",  0xd000, 0x0400, CRC(9ba57987) SHA1(becf89b7d474f86839f13f9be5502c91491e8584) )
	ROM_LOAD( "10405-a.bin",  0xd400, 0x0400, CRC(3fbbeeba) SHA1(1c9f519a0797f90524adf187b0761f150db0828d) )
	ROM_LOAD( "10405-9.bin",  0xd800, 0x0400, CRC(354cf432) SHA1(138956ea8064eba0dcd8b2f175d4981b689a2077) )
	ROM_LOAD( "10405-8.bin",  0xdc00, 0x0400, CRC(4cee0c8b) SHA1(98bfdda9d2d368db16d6e9090536b09d8337c0e5) )
	ROM_LOAD( "sidam-4.bin",  0xe000, 0x0400, CRC(f6998053) SHA1(f1a868e68db1ca89c54ee179aa4c922ec49b686b) )
	ROM_LOAD( "10405-6.bin",  0xe400, 0x0400, CRC(272de8f1) SHA1(e917b3b8bb96fedacd6d5cb3d1c30977818f2e85) )
	ROM_LOAD( "sidam-5.bin",  0xe800, 0x0400, CRC(6ddd78ff) SHA1(2fdf3fd145446f174293818aa81463097227361e) )
	ROM_LOAD( "10405-3.bin",  0xec00, 0x0400, CRC(6edf202d) SHA1(a4cab2f10a99e0a4b1c571168e17cbee1d18cf06) )
	ROM_LOAD( "10405-7.bin",  0xf000, 0x0400, CRC(02a35ad9) SHA1(d54afff13f8d5a6544dda49c766a147fa0172cfa) )
	ROM_LOAD( "10405-1.bin",  0xf400, 0x0400, CRC(c68f6657) SHA1(a38c24670fcbbf7844ca15f918efcb467bae7bef) )
	ROM_LOAD( "10405-2.bin",  0xf800, 0x0400, CRC(b206deda) SHA1(9ab52920c06ed6beb38bc7f97ffd00e8ad46c17d) )
	ROM_LOAD( "10405-0.bin",  0xfc00, 0x0400, CRC(c836a152) SHA1(418b64d50bb2f849b1e7177c7bf2fdd0cc99e079) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "8f-clr.bin",   0x0000, 0x0100, CRC(3bf3ccb0) SHA1(d61d19d38045f42a9adecf295e479fee239bed48) )

	ROM_REGION( 0x0100, "user1", 0 )    /* decryption table */
	ROM_LOAD( "2h-prot.bin",  0x0000, 0x0100, CRC(a6bdd18c) SHA1(438bfc543730afdb531204585f17a68ddc03ded0) )
ROM_END


ROM_START( afire )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b.bin",        0xd000, 0x0400, CRC(16ad2bcc) SHA1(e7f55d17ee18afbb045cd0fd8d3ffc0c8300130a) )
	ROM_LOAD( "a.bin",        0xd400, 0x0400, CRC(ce8b6e4f) SHA1(b85ab709d80324df5d2c4b0dbbc5e6aeb4003077) )
	ROM_LOAD( "9.bin",        0xd800, 0x0400, CRC(e0f45b07) SHA1(091e1ea4b3726888dc488bb01e0bd4e588eccae5) )
	ROM_LOAD( "8.bin",        0xdc00, 0x0400, CRC(85b96728) SHA1(dbbfbc085f19184d861c42a0307f95f9105a677b) )
	ROM_LOAD( "4.bin",        0xe000, 0x0400, CRC(271f90ad) SHA1(fe41a0f35d30d38fc21ac19982899d93cbd292f0) )
	ROM_LOAD( "6.bin",        0xe400, 0x0400, CRC(568efbfe) SHA1(ef39f0fc4c030fc7f688515415aedeb4c039b73a) )
	ROM_LOAD( "5.bin",        0xe800, 0x0400, CRC(1c0b298a) SHA1(61677f8f402679fcfbb9fb12f9dfde7b6e1cdd1c) )
	ROM_LOAD( "3.bin",        0xec00, 0x0400, CRC(2938c641) SHA1(c8655a8218818c12eca0f00a361412e4946f8b5c) )
	ROM_LOAD( "7.bin",        0xf000, 0x0400, CRC(912c8fe1) SHA1(1ae1eb13858d39200386f59c3381eef2699e4647) )
	ROM_LOAD( "1.bin",        0xf400, 0x0400, CRC(0ef045d8) SHA1(c41b284ccdf5da3a5e9b4732324b3d61440ce9db) )
	ROM_LOAD( "2.bin",        0xf800, 0x0400, CRC(d4ea2760) SHA1(57c9a4d21fbb28019fcd2f60c0424b3c9ae1055c) )
	ROM_LOAD( "0.bin",        0xfc00, 0x0400, CRC(fe695575) SHA1(b12587a4de624ab712ed6336bd2eb69b12bde563) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


/* This is a newer revision of "Astro Combat" (most probably manufactured by Sidam),
   with correct spelling for FUEL and the main boss sporting "CB". */
ROM_START( acombat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b.bin",        0xd000, 0x0400, CRC(16ad2bcc) SHA1(e7f55d17ee18afbb045cd0fd8d3ffc0c8300130a) )
	ROM_LOAD( "a.bin",        0xd400, 0x0400, CRC(ce8b6e4f) SHA1(b85ab709d80324df5d2c4b0dbbc5e6aeb4003077) )
	ROM_LOAD( "9.bin",        0xd800, 0x0400, CRC(e0f45b07) SHA1(091e1ea4b3726888dc488bb01e0bd4e588eccae5) )
	ROM_LOAD( "8.bin",        0xdc00, 0x0400, CRC(85b96728) SHA1(dbbfbc085f19184d861c42a0307f95f9105a677b) )
	ROM_LOAD( "4.bin",        0xe000, 0x0400, CRC(271f90ad) SHA1(fe41a0f35d30d38fc21ac19982899d93cbd292f0) )
	ROM_LOAD( "6.bin",        0xe400, 0x0400, CRC(568efbfe) SHA1(ef39f0fc4c030fc7f688515415aedeb4c039b73a) )
	ROM_LOAD( "5.bin",        0xe800, 0x0400, CRC(1c0b298a) SHA1(61677f8f402679fcfbb9fb12f9dfde7b6e1cdd1c) )
	ROM_LOAD( "3.bin",        0xec00, 0x0400, CRC(2938c641) SHA1(c8655a8218818c12eca0f00a361412e4946f8b5c) )
	ROM_LOAD( "7.bin",        0xf000, 0x0400, CRC(912c8fe1) SHA1(1ae1eb13858d39200386f59c3381eef2699e4647) )
	ROM_LOAD( "1a",           0xf400, 0x0400, CRC(7193f999) SHA1(13ddeddb1f22cae973102203ab4917b1407b6401) )
	ROM_LOAD( "2a",           0xf800, 0x0400, CRC(3b6ccbbe) SHA1(f9cf023557ee769bcb92df808628a39630b258f2) )
	ROM_LOAD( "0a",           0xfc00, 0x0400, CRC(355da937) SHA1(e50f364372120926d062203bd476ff68ab3bb5cf) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


/* It is on older revision of "Astro Combat" (most probably manufactured by Sidam),
   with incorrect spelling for fuel as FLUEL and the main boss sporting "PZ" */
ROM_START( acombato )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b.bin",        0xd000, 0x0400, CRC(16ad2bcc) SHA1(e7f55d17ee18afbb045cd0fd8d3ffc0c8300130a) )
	ROM_LOAD( "a.bin",        0xd400, 0x0400, CRC(ce8b6e4f) SHA1(b85ab709d80324df5d2c4b0dbbc5e6aeb4003077) )
	ROM_LOAD( "9.bin",        0xd800, 0x0400, CRC(e0f45b07) SHA1(091e1ea4b3726888dc488bb01e0bd4e588eccae5) )
	ROM_LOAD( "8.bin",        0xdc00, 0x0400, CRC(85b96728) SHA1(dbbfbc085f19184d861c42a0307f95f9105a677b) )
	ROM_LOAD( "4.bin",        0xe000, 0x0400, CRC(271f90ad) SHA1(fe41a0f35d30d38fc21ac19982899d93cbd292f0) )
	ROM_LOAD( "6.bin",        0xe400, 0x0400, CRC(568efbfe) SHA1(ef39f0fc4c030fc7f688515415aedeb4c039b73a) )
	ROM_LOAD( "5.bin",        0xe800, 0x0400, CRC(1c0b298a) SHA1(61677f8f402679fcfbb9fb12f9dfde7b6e1cdd1c) )
	ROM_LOAD( "3.bin",        0xec00, 0x0400, CRC(2938c641) SHA1(c8655a8218818c12eca0f00a361412e4946f8b5c) )
	ROM_LOAD( "7.bin",        0xf000, 0x0400, CRC(912c8fe1) SHA1(1ae1eb13858d39200386f59c3381eef2699e4647) )
	ROM_LOAD( "1a",           0xf400, 0x0400, CRC(7193f999) SHA1(13ddeddb1f22cae973102203ab4917b1407b6401) )
	ROM_LOAD( "2a",           0xf800, 0x0400, CRC(3b6ccbbe) SHA1(f9cf023557ee769bcb92df808628a39630b258f2) )
	ROM_LOAD( "0",            0xfc00, 0x0400, CRC(c4f3eaad) SHA1(51f03f35c45ac00a7f38fd97386be92bcb562ca2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END

ROM_START( acombat3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "l.bin",  0xd000, 0x0400, CRC(41227b16) SHA1(1270f1b88e7fa2dd24fc80703dbc5ae33c8b454e) )
	ROM_LOAD( "k.bin",  0xd400, 0x0400, CRC(99043e95) SHA1(5375c9389170a21c8e5a9a89ee98bba0a2fb4f34) )
	ROM_LOAD( "m.bin",  0xd800, 0x0400, CRC(b77b0bdd) SHA1(95eb4758a3cbbfa5085c62a132f289fccfbaa818) )
	ROM_LOAD( "n.bin",  0xdc00, 0x0400, CRC(d23637f2) SHA1(c13dc224c45506d141dcb107f323900ea6974c60) )
	ROM_LOAD( "d.bin",  0xe000, 0x0400, CRC(7090c077) SHA1(283c60cdfafb3f9026dd2241f3fc35e7545d7cf6) )
	ROM_LOAD( "c.bin",  0xe400, 0x0400, CRC(0101ab24) SHA1(90e99001ef6890fbc76b4f58cc54df12dfa79e2e) )
	ROM_LOAD( "b.bin",  0xe800, 0x0400, CRC(4b847950) SHA1(49de63ac72dff22b962fc7b8d7b1c97939072b4a) ) // sldh
	ROM_LOAD( "a.bin",  0xec00, 0x0400, CRC(7eb7969b) SHA1(1aaf7af67b544d01a66a1f3d0eda8d42d2c5bb5d) ) // sldh
	ROM_LOAD( "j.bin",  0xf000, 0x0400, CRC(a7e2b47e) SHA1(97eda53dfc7f1914f7df36c13747d1d824b62734) )
	ROM_LOAD( "h.bin",  0xf400, 0x0400, CRC(261ca943) SHA1(17136843f74d2236a5f81f2261b166c02dde138d) )
	ROM_LOAD( "f.bin",  0xf800, 0x0400, CRC(6ce39b64) SHA1(ba99ddc2d1208afd3f27a9749dac1faca645bbdc) )
	ROM_LOAD( "e.bin",  0xfc00, 0x0400, CRC(2acae469) SHA1(80019bc645a9919f9ec455452ea3e588d61a563f) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "8f-clr.bin",   0x0000, 0x0100, CRC(3bf3ccb0) SHA1(d61d19d38045f42a9adecf295e479fee239bed48) )
ROM_END

ROM_START( sstarbtl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b.bin",        0xd000, 0x0400, CRC(16ad2bcc) SHA1(e7f55d17ee18afbb045cd0fd8d3ffc0c8300130a) )
	ROM_LOAD( "a.rom",        0xd400, 0x0400, CRC(5a75891d) SHA1(71cde93a219ec3735cead7ec89f77bc8b11bfc64) )
	ROM_LOAD( "9.rom",        0xd800, 0x0400, CRC(de3f8063) SHA1(77b89ef0b356316e463d7575c037069d0c14a850) )
	ROM_LOAD( "8.bin",        0xdc00, 0x0400, CRC(85b96728) SHA1(dbbfbc085f19184d861c42a0307f95f9105a677b) )
	ROM_LOAD( "4.bin",        0xe000, 0x0400, CRC(271f90ad) SHA1(fe41a0f35d30d38fc21ac19982899d93cbd292f0) )
	ROM_LOAD( "6.bin",        0xe400, 0x0400, CRC(568efbfe) SHA1(ef39f0fc4c030fc7f688515415aedeb4c039b73a) )
	ROM_LOAD( "5.rom",        0xe800, 0x0400, CRC(4202b7f8) SHA1(c9d153323bdc0c99f4987895d1fba1ebf3bc7f2d) )
	ROM_LOAD( "3.bin",        0xec00, 0x0400, CRC(2938c641) SHA1(c8655a8218818c12eca0f00a361412e4946f8b5c) )
	ROM_LOAD( "7.rom",        0xf000, 0x0400, CRC(76990bf1) SHA1(e0d8e2015401d1190fc8cd9dac3e20a4a54cdc02) )
	ROM_LOAD( "1.rom",        0xf400, 0x0400, CRC(c72dd542) SHA1(08b6aab4c53dac77c6e0af21bae3fed4facef7ef) )
	ROM_LOAD( "2a",           0xf800, 0x0400, CRC(3b6ccbbe) SHA1(f9cf023557ee769bcb92df808628a39630b258f2) )
	ROM_LOAD( "0.rom",        0xfc00, 0x0400, CRC(b31ed075) SHA1(faaa21c9b62deb36dcc4805b38ef55db63fb854a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


ROM_START( spfghmk2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2708.5e",      0xe400, 0x0400, CRC(cd5f66de) SHA1(aea3d88eb1d59a9279361369991fcace90c4b61a) )
	ROM_LOAD( "2708.5d",      0xe800, 0x0400, CRC(385cca72) SHA1(8d38a127f7603f1573df24cb028e1f41098a61c1) )
	ROM_LOAD( "2708.5c",      0xec00, 0x0400, CRC(e6eaac70) SHA1(3af366f190ed0aed43cc584c6bd472da957c725a) )
	ROM_LOAD( "2708.4h",      0xf000, 0x0400, CRC(27945183) SHA1(7907d3d2b90d38c35fb6cf194408d2be23769c8c) )
	ROM_LOAD( "2708.4e",      0xf400, 0x0400, CRC(2115e25f) SHA1(a7c529f42d9bf70c7f81df949ba4666bde8da4c5) )
	ROM_LOAD( "2708.4d",      0xf800, 0x0400, CRC(b9655874) SHA1(22e53bc0b68acc8483bd18b15a020af19cf3e151) )
	ROM_LOAD( "2708.4c",      0xfc00, 0x0400, CRC(7d67f6b5) SHA1(aed42c2c48d50fb9e3c2860cdc9d448024a554ae) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "709-5.1a",     0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


ROM_START( spfghmk22 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2708mkii.5e",  0xe400, 0x0400, CRC(27d7060d) SHA1(796c44a395e1c54769dc57050503b4b111bde7ef) )
	ROM_LOAD( "2708mkii.5d",  0xe800, 0x0400, CRC(6ccb3b0a) SHA1(566104ca2e0fae741d4650e7159c9ddb48f59e8b) )
	ROM_LOAD( "2708mkii.5c",  0xec00, 0x0400, CRC(68eb0ad5) SHA1(d303685ffd67898cec3e7c51b3831558a837e5a3) )
	ROM_LOAD( "2708mkii.4h",  0xf000, 0x0400, CRC(ea8d1f2f) SHA1(35b01e76284080d5bd0270e4004a54386d9eb697) )
	ROM_LOAD( "2708mkii.4e",  0xf400, 0x0400, CRC(6e7f00ae) SHA1(91ca17d5dc75be641c059fd84bed7cced2a2ef69) )
	ROM_LOAD( "2708mkii.4d",  0xf800, 0x0400, CRC(29501dba) SHA1(978d7009eab8da40ccf0d026c9dabc0a3fa95d76) )
	ROM_LOAD( "2708mkii.4c",  0xfc00, 0x0400, CRC(9bd589a6) SHA1(bce92fcab5220ff68526bc8c1c88ab0f317fe400) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "709-5.1a",     0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


ROM_START( tomahawk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "thawk.l8",     0xdc00, 0x0400, CRC(b01dab4b) SHA1(d8b4266359a3b18d649f539fad8dce4d73cec412) )
	ROM_LOAD( "thawk.l7",     0xe000, 0x0400, CRC(3a6549e8) SHA1(2ba622d78596c72998784432cf8fbbe733c50ce5) )
	ROM_LOAD( "thawk.l6",     0xe400, 0x0400, CRC(863e47f7) SHA1(e8e48560c217025796be20f51c50ec276dba3eb5) )
	ROM_LOAD( "thawk.l5",     0xe800, 0x0400, CRC(de0183bc) SHA1(7cb8d013750c8fb423ab2759443f805bc8440d53) )
	ROM_LOAD( "thawk.l4",     0xec00, 0x0400, CRC(11e9c7ea) SHA1(9dbdce7d518891aa8b08dca50d4e8aaec89cc038) )
	ROM_LOAD( "thawk.l3",     0xf000, 0x0400, CRC(ec44d388) SHA1(7dda9db5ce2271988e9316dacf4b6ccbb72f50c9) )
	ROM_LOAD( "thawk.l2",     0xf400, 0x0400, CRC(dc0a0f54) SHA1(8e5c94706768ffafaba96382f2e757ecb825799f) )
	ROM_LOAD( "thawk.l1",     0xf800, 0x0400, CRC(1d9dab9c) SHA1(54dd91164db0489bd5984f10d4f0254184302ae4) )
	ROM_LOAD( "thawk.l0",     0xfc00, 0x0400, CRC(d21a1eba) SHA1(ce9ad7a1a3b069ef4eb8b5ce569e52c488a224f2) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "t777.clr",     0x0000, 0x0020, CRC(d6a528fd) SHA1(5fc08252a2d7c5405f601efbfb7d84bec328d733) )

	// versions of this have also been seen using the standard Astro Fighter PROM, giving a blue submarine
	// in blue water, with pink / yellow borders.  I think these are just unofficial conversions of Astro
	// Fighter without the PROM properly replaced tho.
	//ROM_LOAD( "astrf.clr",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END


ROM_START( tomahawk1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "l8-1",         0xdc00, 0x0400, CRC(7c911661) SHA1(3fc75bb0e6a89d41d76f82eeb0fde7d33809dddf) )
	ROM_LOAD( "l7-1",         0xe000, 0x0400, CRC(adeffb69) SHA1(8ff7ada883825a8b56cae3368ce377228922ab1d) )
	ROM_LOAD( "l6-1",         0xe400, 0x0400, CRC(9116e59d) SHA1(22a6d410fff8534b3aa7eb2ed0a8c096c890acf5) )
	ROM_LOAD( "l5-1",         0xe800, 0x0400, CRC(01e4c7c4) SHA1(fbb37539d08284bae6454cd57650e8507a88acdb) )
	ROM_LOAD( "l4-1",         0xec00, 0x0400, CRC(d9f69cb0) SHA1(d6a2dcaf867f33068e7d7ad7a3faf62a360456a6) )
	ROM_LOAD( "l3-1",         0xf000, 0x0400, CRC(7ce7183f) SHA1(949c7b696fe215b68af450299c91e90fb27b0141) )
	ROM_LOAD( "l2-1",         0xf400, 0x0400, CRC(43fea29d) SHA1(6890311440089a16d2e4d502855670723df41e16) )
	ROM_LOAD( "l1-1",         0xf800, 0x0400, CRC(f2096ba9) SHA1(566f6d49cdacb5e39c40eb3773640270ef5f272c) )
	ROM_LOAD( "l0-1",         0xfc00, 0x0400, CRC(42edbc28) SHA1(bab1fe8591509783dfdd4f53b9159263b9201970) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "t777.clr",     0x0000, 0x0020, CRC(d6a528fd) SHA1(5fc08252a2d7c5405f601efbfb7d84bec328d733) )
ROM_END



/*************************************
 *
 *  Game specific initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(astrof_state,abattle)
{
	/* use the protection PROM to decrypt the ROMs */
	UINT8 *rom = memregion("maincpu")->base();
	UINT8 *prom = memregion("user1")->base();
	int i;

	for(i = 0xd000; i < 0x10000; i++)
		rom[i] = prom[rom[i]];

	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8_delegate(FUNC(astrof_state::shoot_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8_delegate(FUNC(astrof_state::abattle_coin_prot_r),this));
}


DRIVER_INIT_MEMBER(astrof_state,afire)
{
	UINT8 *rom = memregion("maincpu")->base();
	int i;

	for(i = 0xd000; i < 0x10000; i++)
		rom[i] = ~rom[i];

	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8_delegate(FUNC(astrof_state::shoot_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8_delegate(FUNC(astrof_state::afire_coin_prot_r),this));
}


DRIVER_INIT_MEMBER(astrof_state,sstarbtl)
{
	UINT8 *rom = memregion("maincpu")->base();
	int i;

	for(i = 0xd000; i < 0x10000; i++)
		rom[i] = ~rom[i];

	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8_delegate(FUNC(astrof_state::shoot_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8_delegate(FUNC(astrof_state::abattle_coin_prot_r),this));
}

DRIVER_INIT_MEMBER(astrof_state,acombat3)
{
	/* set up protection handlers */
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8_delegate(FUNC(astrof_state::shoot_r),this));
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8_delegate(FUNC(astrof_state::abattle_coin_prot_r),this));
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, astrof,   0,        astrof,   astrof, driver_device,  0,       ROT90, "Data East",             "Astro Fighter (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, astrof2,  astrof,   astrof,   astrof, driver_device,  0,       ROT90, "Data East",             "Astro Fighter (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, astrof3,  astrof,   astrof,   astrof, driver_device,  0,       ROT90, "Data East",             "Astro Fighter (set 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, abattle,  astrof,   abattle,  abattle, astrof_state,  abattle, ROT90, "bootleg? (Sidam)",      "Astro Battle (set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, abattle2, astrof,   abattle,  abattle, astrof_state,  abattle, ROT90, "bootleg? (Sidam)",      "Astro Battle (set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, afire,    astrof,   abattle,  abattle, astrof_state,  afire,   ROT90, "bootleg (Rene Pierre)", "Astro Fire", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, acombat,  astrof,   abattle,  abattle, astrof_state,  afire,   ROT90, "bootleg",               "Astro Combat (newer, CB)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, acombato, astrof,   abattle,  abattle, astrof_state,  afire,   ROT90, "bootleg",               "Astro Combat (older, PZ)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, acombat3, astrof,   abattle,  abattle, astrof_state,  acombat3,ROT90, "bootleg (Proel)",       "Astro Combat (unencrypted)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, sstarbtl, astrof,   abattle,  abattle, astrof_state,  sstarbtl,ROT90, "bootleg",               "Super Star Battle", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1979, spfghmk2, 0,        spfghmk2, spfghmk2, driver_device,0,       ROT90, "Data East",            "Space Fighter Mark II (set 1)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, spfghmk22,spfghmk2, spfghmk2, spfghmk22,driver_device,0,       ROT90, "Data East",            "Space Fighter Mark II (set 2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1980, tomahawk, 0,        tomahawk, tomahawk, driver_device,0,       ROT90, "Data East",            "Tomahawk 777 (rev 5)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, tomahawk1,tomahawk, tomahawk, tomahawk1,driver_device,0,       ROT90, "Data East",            "Tomahawk 777 (rev 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
