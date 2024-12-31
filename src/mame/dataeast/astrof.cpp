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
        * Saturn: Space Fighter 3D
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

        * Saturn: Space Fighter 3D is basically Space Fighter with extra game modes.
          It's not 3-dimensional. The game modes are: Storm, Sweeper, Scramble.

    Known issues/to-do's:
        * Analog sound in all games
        * satsf3d color prom is not dumped, using the one from spfghmk2 instead
          (it looks good compared to photo)

    About Colours:
        * It was fairly common to have wiremods on these PCBs to change the
          background colours, this is why you see Astro Fighter and Tomahawk
          games with both Blue and Black backgrounds.  By default MAME
          emulates an unmodified PCB, you can enable the hacks in the DRIVER
          CONFIGURATION menu.

          B&W cabinet versions of the Space Fighter games also exist.

          Versions of Tomahawk using the Astro Fighter PROM have been seen,
          see notes in ROM loading.

****************************************************************************/


#include "emu.h"
#include "astrof.h"
#include "cpu/m6502/m6502.h"


#define MASTER_CLOCK        (XTAL(10'595'000))
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

uint8_t astrof_state::irq_clear_r()
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


ioport_value astrof_state::astrof_p1_controls_r()
{
	return ioport("P1")->read();
}


ioport_value astrof_state::astrof_p2_controls_r()
{
	uint32_t ret;

	/* on an upright cabinet, a single set of controls
	   is connected to both sets of pins on the edge
	   connector */
	if (ioport("CAB")->read())
		ret = ioport("P2")->read();
	else
		ret = ioport("P1")->read();

	return ret;
}


ioport_value astrof_state::tomahawk_controls_r()
{
	uint32_t ret;

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
	m_colorram = std::make_unique<uint8_t[]>(m_videoram.bytes() / 2);
	save_pointer(NAME(m_colorram), m_videoram.bytes() / 2);
}


rgb_t astrof_state::make_pen( uint8_t data )
{
	uint8_t r1_bit = m_red_on ? 0x01 : (data >> 0) & 0x01;
	uint8_t r2_bit = m_red_on ? 0x01 : (data >> 1) & 0x01;
	uint8_t g1_bit = (data >> 2) & 0x01;
	uint8_t g2_bit = (data >> 3) & 0x01;
	uint8_t b1_bit = (data >> 4) & 0x01;
	uint8_t b2_bit = (data >> 5) & 0x01;

	/* this is probably not quite right, but I don't have the
	   knowledge to figure out the actual weights - ZV */
	uint8_t r = (0xc0 * r1_bit) + (0x3f * r2_bit);
	uint8_t g = (0xc0 * g1_bit) + (0x3f * g2_bit);
	uint8_t b = (0xc0 * b1_bit) + (0x3f * b2_bit);

	return rgb_t(r, g, b);
}


void astrof_state::astrof_get_pens( pen_t *pens )
{
	offs_t i;
	uint8_t bank = (m_astrof_palette_bank ? 0x10 : 0x00);
	uint8_t config = m_fake_port.read_safe(0x00);
	uint8_t *prom = memregion("proms")->base();

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
		uint8_t data = prom[bank | i];
		pens[i] = make_pen(data);
	}
}


void astrof_state::tomahawk_get_pens( pen_t *pens )
{
	offs_t i;
	uint8_t *prom = memregion("proms")->base();
	uint8_t config = m_fake_port.read_safe(0x00);

	for (i = 0; i < TOMAHAWK_NUM_PENS; i++)
	{
		uint8_t data;
		uint8_t pen;

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


void astrof_state::astrof_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_colorram[offset >> 1] = *m_astrof_color & 0x0e;
}


void astrof_state::tomahawk_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_colorram[offset >> 1] = (*m_astrof_color & 0x0e) | ((*m_astrof_color & 0x01) << 4);
}


void astrof_state::video_control_1_w(uint8_t data)
{
	m_flipscreen = ((data >> 0) & 0x01) & ioport("CAB")->read();

	/* this ties to the CLR pin of the shift registers */
	m_screen_off = (data & 0x02) ? true : false;

	/* D2 - not connected in the schematics, but at one point Astro Fighter sets it to 1 */
	/* D3-D7 - not connected */

	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::astrof_set_video_control_2( uint8_t data )
{
	/* D0 - OUT0 - goes to edge conn. pin A10 - was perhaps meant to be a start lamp */
	/* D1 - OUT1 - goes to edge conn. pin A11 - was perhaps meant to be a start lamp */

	/* D2 - selects one of the two palette banks */
	m_astrof_palette_bank = (data & 0x04) ? true : false;

	/* D3 - turns on the red color gun regardless of the value in the color PROM */
	m_red_on = (data & 0x08) ? true : false;

	/* D4-D7 - not connected */
}

void astrof_state::astrof_video_control_2_w(uint8_t data)
{
	astrof_set_video_control_2(data);
	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::spfghmk2_set_video_control_2( uint8_t data )
{
	/* D0 - OUT0 - goes to edge conn. pin A10 - was perhaps meant to be a start lamp */
	/* D1 - OUT1 - goes to edge conn. pin A11 - was perhaps meant to be a start lamp */

	/* D2 - selects one of the two palette banks */
	m_astrof_palette_bank = (data & 0x04) ? true : false;

	/* D3-D7 - not connected */
}

void astrof_state::spfghmk2_video_control_2_w(uint8_t data)
{
	spfghmk2_set_video_control_2(data);
	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::tomahawk_set_video_control_2( uint8_t data )
{
	/* D0 - OUT0 - goes to edge conn. pin A10 - was perhaps meant to be a start lamp */
	/* D1 - OUT1 - goes to edge conn. pin A11 - was perhaps meant to be a start lamp */
	/* D2 - not connected */

	/* D3 - turns on the red color gun regardless of the value in the color PROM */
	m_red_on = (data & 0x08) ? true : false;
}

void astrof_state::tomahawk_video_control_2_w(uint8_t data)
{
	tomahawk_set_video_control_2(data);
	m_screen->update_partial(m_screen->vpos());
}


void astrof_state::video_update_common( bitmap_rgb32 &bitmap, const rectangle &cliprect, pen_t *pens, int num_pens )
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t const color = m_colorram[offs >> 1];

		pen_t back_pen = pens[(color & (num_pens-1)) | 0x00];
		pen_t fore_pen = pens[(color & (num_pens-1)) | 0x01];

		uint8_t y = offs;
		uint8_t x = offs >> 8 << 3;

		if (!m_flipscreen)
			y = ~y;

		if ((y <= cliprect.top()) || (y >= cliprect.bottom()))
			continue;

		uint8_t data;
		if (m_screen_off)
			data = 0;
		else
			data = m_videoram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t const pen = (data & 0x01) ? fore_pen : back_pen;

			if (m_flipscreen)
				bitmap.pix(y, 255 - x) = pen;
			else
				bitmap.pix(y, x) = pen;

			x++;
			data >>= 1;
		}
	}
}


uint32_t astrof_state::screen_update_astrof(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	pen_t pens[ASTROF_NUM_PENS];

	astrof_get_pens(pens);

	video_update_common(bitmap, cliprect, pens, ASTROF_NUM_PENS);

	return 0;
}


uint32_t astrof_state::screen_update_tomahawk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

uint8_t astrof_state::shoot_r()
{
	/* not really sure about this */
	return machine().rand() & 8;
}


uint8_t astrof_state::abattle_coin_prot_r()
{
	m_abattle_count = (m_abattle_count + 1) % 0x0101;
	return m_abattle_count ? 0x07 : 0x00;
}


uint8_t astrof_state::afire_coin_prot_r()
{
	m_abattle_count = m_abattle_count ^ 0x01;
	return m_abattle_count ? 0x07 : 0x00;
}


uint8_t astrof_state::tomahawk_protection_r()
{
	/* flip the byte */
	return bitswap<8>(*m_tomahawk_protection, 0, 1, 2, 3, 4, 5, 6, 7);
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
	m_red_on = false;

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

void astrof_state::astrof_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x1c00).ram();
	map(0x2000, 0x3fff).noprw();
	map(0x4000, 0x5fff).ram().w(FUNC(astrof_state::astrof_videoram_w)).share("videoram");
	map(0x6000, 0x7fff).noprw();
	map(0x8000, 0x8002).mirror(0x1ff8).noprw();
	map(0x8003, 0x8003).mirror(0x1ff8).nopr().writeonly().share("astrof_color");
	map(0x8004, 0x8004).mirror(0x1ff8).nopr().w(FUNC(astrof_state::video_control_1_w));
	map(0x8005, 0x8005).mirror(0x1ff8).nopr().w(FUNC(astrof_state::astrof_video_control_2_w));
	map(0x8006, 0x8006).mirror(0x1ff8).nopr().w(FUNC(astrof_state::astrof_audio_1_w));
	map(0x8007, 0x8007).mirror(0x1ff8).nopr().w(FUNC(astrof_state::astrof_audio_2_w));
	map(0xa000, 0xa000).mirror(0x1ff8).portr("IN").nopw();
	map(0xa001, 0xa001).mirror(0x1ff8).portr("DSW").nopw();
	map(0xa002, 0xa002).mirror(0x1ff8).r(FUNC(astrof_state::irq_clear_r)).nopw();
	map(0xa003, 0xa007).mirror(0x1ff8).noprw();
	map(0xc000, 0xffff).rom();
}


void astrof_state::spfghmk2_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x1c00).ram();
	map(0x2000, 0x3fff).noprw();
	map(0x4000, 0x5fff).ram().w(FUNC(astrof_state::astrof_videoram_w)).share("videoram");
	map(0x6000, 0x7fff).noprw();
	map(0x8000, 0x8002).mirror(0x1ff8).noprw();
	map(0x8003, 0x8003).mirror(0x1ff8).nopr().writeonly().share("astrof_color");
	map(0x8004, 0x8004).mirror(0x1ff8).nopr().w(FUNC(astrof_state::video_control_1_w));
	map(0x8005, 0x8005).mirror(0x1ff8).nopr().w(FUNC(astrof_state::spfghmk2_video_control_2_w));
	map(0x8006, 0x8006).mirror(0x1ff8).nopr().w(FUNC(astrof_state::spfghmk2_audio_w));
	map(0x8007, 0x8007).mirror(0x1ff8).noprw();
	map(0xa000, 0xa000).mirror(0x1ff8).portr("IN").nopw();
	map(0xa001, 0xa001).mirror(0x1ff8).portr("DSW").nopw();
	map(0xa002, 0xa002).mirror(0x1ff8).r(FUNC(astrof_state::irq_clear_r)).nopw();
	map(0xa003, 0xa007).mirror(0x1ff8).noprw();
	map(0xc000, 0xffff).rom();
}


void astrof_state::tomahawk_map(address_map &map)
{
	map(0x0000, 0x03ff).mirror(0x1c00).ram();
	map(0x2000, 0x3fff).noprw();
	map(0x4000, 0x5fff).ram().w(FUNC(astrof_state::tomahawk_videoram_w)).share("videoram");
	map(0x6000, 0x7fff).noprw();
	map(0x8000, 0x8002).mirror(0x1ff8).noprw();
	map(0x8003, 0x8003).mirror(0x1ff8).nopr().writeonly().share("astrof_color");
	map(0x8004, 0x8004).mirror(0x1ff8).nopr().w(FUNC(astrof_state::video_control_1_w));
	map(0x8005, 0x8005).mirror(0x1ff8).nopr().w(FUNC(astrof_state::tomahawk_video_control_2_w));
	map(0x8006, 0x8006).mirror(0x1ff8).nopr().w(FUNC(astrof_state::tomahawk_audio_w));
	map(0x8007, 0x8007).mirror(0x1ff8).nopr().writeonly().share("tomahawk_prot");
	map(0xa000, 0xa000).mirror(0x1ff8).portr("IN").nopw();
	map(0xa001, 0xa001).mirror(0x1ff8).portr("DSW").nopw();
	map(0xa002, 0xa002).mirror(0x1ff8).r(FUNC(astrof_state::irq_clear_r)).nopw();
	map(0xa003, 0xa003).mirror(0x1ff8).r(FUNC(astrof_state::tomahawk_protection_r)).nopw();
	map(0xa004, 0xa007).mirror(0x1ff8).noprw();
	map(0xc000, 0xffff).rom();
}



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
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p1_controls_r))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p2_controls_r))

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
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::coin_inserted), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::service_coin_inserted), 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/* same as 'astrof', but inputs are ACTIVE_HIGH instead of ACTIVE_LOW */
static INPUT_PORTS_START( abattle )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p1_controls_r))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p2_controls_r))

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
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::coin_inserted), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::service_coin_inserted), 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( spfghmk2 )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p1_controls_r))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p2_controls_r))

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
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::coin_inserted), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::service_coin_inserted), 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( spfghmk22 )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p1_controls_r))
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::astrof_p2_controls_r))

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
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::coin_inserted), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::service_coin_inserted), 0)
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( tomahawk )
	PORT_INCLUDE( astrof_common )

	PORT_START("IN")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(astrof_state::tomahawk_controls_r))
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
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("screen", FUNC(screen_device::vblank))

	PORT_START("CAB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )          PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::coin_inserted), 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(astrof_state::service_coin_inserted), 0)
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

void astrof_state::base(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MAIN_CPU_CLOCK);
	TIMER(config, "vblank").configure_scanline(FUNC(astrof_state::irq_callback), "screen", VBSTART, 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
}


void astrof_state::astrof(machine_config &config)
{
	base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrof_state::astrof_map);

	MCFG_MACHINE_START_OVERRIDE(astrof_state,astrof)

	/* video hardware */
	m_screen->set_screen_update(FUNC(astrof_state::screen_update_astrof));

	/* audio hardware */
	astrof_audio(config);
}


void astrof_state::abattle(machine_config &config)
{
	astrof(config);

	/* basic machine hardware */
	MCFG_MACHINE_START_OVERRIDE(astrof_state,abattle)
	MCFG_MACHINE_RESET_OVERRIDE(astrof_state,abattle)
}


void astrof_state::spfghmk2(machine_config &config)
{
	base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrof_state::spfghmk2_map);

	MCFG_MACHINE_START_OVERRIDE(astrof_state,spfghmk2)

	/* video hardware */
	m_screen->set_screen_update(FUNC(astrof_state::screen_update_astrof));

	/* audio hardware */
	spfghmk2_audio(config);
}


void astrof_state::tomahawk(machine_config &config)
{
	base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrof_state::tomahawk_map);

	MCFG_MACHINE_START_OVERRIDE(astrof_state,tomahawk)

	/* video hardware */
	m_screen->set_screen_update(FUNC(astrof_state::screen_update_tomahawk));

	/* audio hardware */
	tomahawk_audio(config);
}



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

// from an original Taito PCB with silkscreen
ROM_START( astroft )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "as_19.bin",       0xd000, 0x0800, CRC(fa9e5607) SHA1(246a3591196939ecca9088f44035cceb4ee3531e) )
	ROM_LOAD( "as_18.bin",       0xd800, 0x0800, CRC(1c104d3d) SHA1(20015808ad87421e90c0eac806bb39dee5226b51) )
	ROM_LOAD( "as_17.bin",       0xe000, 0x0800, CRC(20dc4d50) SHA1(cfb3d98677f213e21593116db4804f965e6467f9) )
	ROM_LOAD( "as_16.bin",       0xe800, 0x0800, CRC(4b488aa5) SHA1(fd5177b99215ec6bb73b0bb0cd0ef7745168636f) )
	ROM_LOAD( "as_15.bin",       0xf000, 0x0800, CRC(47fbe257) SHA1(0aa01be624c701822463060a0be943d5b7788b38) )
	ROM_LOAD( "as_14.bin",       0xf800, 0x0800, CRC(91476301) SHA1(2f47ab248538e5fe7462486f395072dec8af1012) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "74s288.bin",    0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END

// Famaresa "500" PCB set (500-001, 500-002, 500-003 and 500-004).
ROM_START( astroff )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b.4a",        0xd000, 0x0400, CRC(18bdc2d9) SHA1(1fc93de1212f04f04acc224d91c2308758f5d5ca) )
	ROM_LOAD( "a.4c",        0xd400, 0x0400, CRC(2344521c) SHA1(7f50890df10219b51bf4cf617e8ffb4336e19ae3) )
	ROM_LOAD( "9.5a",        0xd800, 0x0400, CRC(9bd0e10d) SHA1(0170f56ce0c7a3827548bdd954acb2c0e4a2cb24) )
	ROM_LOAD( "8.5c",        0xdc00, 0x0400, CRC(c00d8f28) SHA1(08abc667b2c079bee1242b30545002fa7fea8a72) )
	ROM_LOAD( "7.4d",        0xe000, 0x0400, CRC(6cd01b9e) SHA1(597db331008a4a561bc8d9bbd42ffde9bcd565dc) )
	ROM_LOAD( "6.4f",        0xe400, 0x0400, CRC(69cd2604) SHA1(09201ae691330e3bd788f9c714f6dcbb7e8335d0) )
	ROM_LOAD( "5.5d",        0xe800, 0x0400, CRC(14a4118f) SHA1(d99876f405dd646a7f0e85e584f2509d9cf17372) )
	ROM_LOAD( "4.5f",        0xec00, 0x0400, CRC(0513bc92) SHA1(dbf774e8353c966c303e02ce8196b888d235bdb2) )
	ROM_LOAD( "3.4h",        0xf000, 0x0400, CRC(e01713bd) SHA1(e0370a070c727f8f7ff767fdd84f7aeef31c8347) )
	ROM_LOAD( "2.4k",        0xf400, 0x0400, CRC(24ab94d5) SHA1(1e0a61d83fa67340d4b1477377ed9d65cf826f53) )
	ROM_LOAD( "1.5h",        0xf800, 0x0400, CRC(142bbc5d) SHA1(55017fd88a19d09943cde16d583db00cf4c3218b) )
	ROM_LOAD( "0.5k",        0xfc00, 0x0400, CRC(c63a9c80) SHA1(f51694912d823a5ae7883c603915b9c1aa0e0733) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "im5610-82s123.2f", 0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
ROM_END

// Famaresa "500" PCB set (500-001, 500-002, 500-003 and 500-004). Tested, shows a black background.
ROM_START( astroff2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b.4a",        0xd000, 0x0400, CRC(18bdc2d9) SHA1(1fc93de1212f04f04acc224d91c2308758f5d5ca) )
	ROM_LOAD( "a.4c",        0xd400, 0x0400, CRC(2344521c) SHA1(7f50890df10219b51bf4cf617e8ffb4336e19ae3) )
	ROM_LOAD( "9.5a",        0xd800, 0x0400, CRC(9bd0e10d) SHA1(0170f56ce0c7a3827548bdd954acb2c0e4a2cb24) )
	ROM_LOAD( "8r.5c",       0xdc00, 0x0400, CRC(06ff0192) SHA1(f581eb508364b44e51aa998b7dba6a97578a3a92) )
	ROM_LOAD( "7.4d",        0xe000, 0x0400, CRC(6cd01b9e) SHA1(597db331008a4a561bc8d9bbd42ffde9bcd565dc) )
	ROM_LOAD( "6.4f",        0xe400, 0x0400, CRC(69cd2604) SHA1(09201ae691330e3bd788f9c714f6dcbb7e8335d0) )
	ROM_LOAD( "5.5d",        0xe800, 0x0400, CRC(14a4118f) SHA1(d99876f405dd646a7f0e85e584f2509d9cf17372) )
	ROM_LOAD( "4.5f",        0xec00, 0x0400, CRC(0513bc92) SHA1(dbf774e8353c966c303e02ce8196b888d235bdb2) )
	ROM_LOAD( "3.4h",        0xf000, 0x0400, CRC(e01713bd) SHA1(e0370a070c727f8f7ff767fdd84f7aeef31c8347) )
	ROM_LOAD( "2.4k",        0xf400, 0x0400, CRC(24ab94d5) SHA1(1e0a61d83fa67340d4b1477377ed9d65cf826f53) )
	ROM_LOAD( "1.5h",        0xf800, 0x0400, CRC(142bbc5d) SHA1(55017fd88a19d09943cde16d583db00cf4c3218b) )
	ROM_LOAD( "0.5k",        0xfc00, 0x0400, CRC(c63a9c80) SHA1(f51694912d823a5ae7883c603915b9c1aa0e0733) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "im5610-82s123.2f", 0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) )
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


ROM_START( asterion )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "asterion eprom 12.bin", 0xd000, 0x0400, CRC(9ba57987) SHA1(becf89b7d474f86839f13f9be5502c91491e8584) )
	ROM_LOAD( "asterion eprom 11.bin", 0xd400, 0x0400, CRC(8974715e) SHA1(1a4a576d51a0788dd973de5c5fde5131a940d3f0) )
	ROM_LOAD( "asterion eprom 10.bin", 0xd800, 0x0400, CRC(354cf432) SHA1(138956ea8064eba0dcd8b2f175d4981b689a2077) )
	ROM_LOAD( "asterion eprom 9.bin",  0xdc00, 0x0400, CRC(4cee0c8b) SHA1(98bfdda9d2d368db16d6e9090536b09d8337c0e5) )
	ROM_LOAD( "asterion eprom 5.bin",  0xe000, 0x0400, CRC(9cb477f3) SHA1(6866264aa8d0479cee237a00e4a919e3981144a5) )
	ROM_LOAD( "asterion eprom 7.bin",  0xe400, 0x0400, CRC(272de8f1) SHA1(e917b3b8bb96fedacd6d5cb3d1c30977818f2e85) )
	ROM_LOAD( "asterion eprom 6.bin",  0xe800, 0x0400, CRC(ff25acaa) SHA1(5cb360c556c9b36039ae05702e6900b82fe5676b) )
	ROM_LOAD( "asterion eprom 4.bin",  0xec00, 0x0400, CRC(6edf202d) SHA1(a4cab2f10a99e0a4b1c571168e17cbee1d18cf06) )
	ROM_LOAD( "asterion eprom 8.bin",  0xf000, 0x0400, CRC(47dccb04) SHA1(b6b6c6685c93ac9531efb970b2e82ad68eea87ba) )
	ROM_LOAD( "asterion eprom 2.bin",  0xf400, 0x0400, CRC(86c6ae5c) SHA1(c5d8dab0ef3168884ae6fe7099708a290daba2ba) )
	ROM_LOAD( "asterion eprom 3.bin",  0xf800, 0x0400, CRC(b206deda) SHA1(9ab52920c06ed6beb38bc7f97ffd00e8ad46c17d) )
	ROM_LOAD( "asterion eprom 1.bin",  0xfc00, 0x0400, CRC(99008e90) SHA1(bfa1c3a66992f432fad37203f052f820b098e05f) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "sn74s470.bin",   0x0000, 0x0100, CRC(3bf3ccb0) SHA1(d61d19d38045f42a9adecf295e479fee239bed48) )

	ROM_REGION( 0x0200, "user1", 0 ) // data relevant for the decryption is the same as the one for abattle, but it's on a larger PROM
	ROM_LOAD( "sn74s472.bin",   0x0000, 0x0200, CRC(d437a9d8) SHA1(32e3513ab2ce1cde5196d80c53f5aa98b339929f) )
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

ROM_START( acombat4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "11.bin", 0xd000, 0x0400, CRC(9ba57987) SHA1(becf89b7d474f86839f13f9be5502c91491e8584) )
	ROM_LOAD( "10.bin", 0xd400, 0x0400, CRC(22493f2a) SHA1(4a2569f500c022b77d99855ca38a3591ed56e055) )
	ROM_LOAD( "9.bin",  0xd800, 0x0400, CRC(354cf432) SHA1(138956ea8064eba0dcd8b2f175d4981b689a2077) )
	ROM_LOAD( "8.bin",  0xdc00, 0x0400, CRC(4cee0c8b) SHA1(98bfdda9d2d368db16d6e9090536b09d8337c0e5) )
	ROM_LOAD( "4.bin",  0xe000, 0x0400, CRC(9cb477f3) SHA1(6866264aa8d0479cee237a00e4a919e3981144a5) )
	ROM_LOAD( "6.bin",  0xe400, 0x0400, CRC(272de8f1) SHA1(e917b3b8bb96fedacd6d5cb3d1c30977818f2e85) )
	ROM_LOAD( "5.bin",  0xe800, 0x0400, CRC(ff25acaa) SHA1(5cb360c556c9b36039ae05702e6900b82fe5676b) )
	ROM_LOAD( "3.bin",  0xec00, 0x0400, CRC(6edf202d) SHA1(a4cab2f10a99e0a4b1c571168e17cbee1d18cf06) )
	ROM_LOAD( "7.bin",  0xf000, 0x0400, CRC(47dccb04) SHA1(b6b6c6685c93ac9531efb970b2e82ad68eea87ba) )
	ROM_LOAD( "1.bin",  0xf400, 0x0400, CRC(5874584f) SHA1(8794c17ac156e7c59631d683bbf100036ab45713) )
	ROM_LOAD( "2.bin",  0xf800, 0x0400, CRC(b206deda) SHA1(9ab52920c06ed6beb38bc7f97ffd00e8ad46c17d) )
	ROM_LOAD( "0.bin",  0xfc00, 0x0400, CRC(4d52948a) SHA1(bcf9590a8049cada958531f6b7ae0d499c1096e2) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "8f-clr.bin",   0x0000, 0x0100, CRC(3bf3ccb0) SHA1(d61d19d38045f42a9adecf295e479fee239bed48) )

	ROM_REGION( 0x0100, "user1", 0 )    /* decryption table */
	ROM_LOAD( "74471.cpu",  0x0000, 0x0100, CRC(a6bdd18c) SHA1(438bfc543730afdb531204585f17a68ddc03ded0) )
ROM_END

/* Star Fighter (VGG)
CPUs
QTY     Type    clock   position    function
1x  R6502-13    2a  8-bit Microprocessor - main
1x  TBA810      2f  Audio Amplifier - sound
1x  oscillator  10595   9c

ROMs
QTY     Type    position    status
6x  TMS2716     0-5     dumped
1x  MMI6341-1J  12c     dumped

RAMs
QTY     Type    position
19x     ITT4027     1-19
2x  2114L3PC    5b,6b

Others
1x 22x2 edge connector
1x trimmer (volume)(1f)
1x 8 DIP switches bank (1e)
1x 4 DIP switches bank (6e)*/

ROM_START( strfight )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sf00.bin",       0xd000, 0x0800, CRC(35662bf6) SHA1(f8e4a116c6eedc25949dd4c2744e83a3cc6a4a41) )
	ROM_LOAD( "sf01.bin",       0xd800, 0x0800, CRC(535f97bd) SHA1(7ea3e02627364db0ae6cbbfc5452a85624540c12) )
	ROM_LOAD( "sf02.bin",       0xe000, 0x0800, CRC(2146c290) SHA1(82a7334fbe1a05fc3a58db881c46be6368cab4fd) )
	ROM_LOAD( "sf03.bin",       0xe800, 0x0800, CRC(53e7ac18) SHA1(131016eac8785141bccc446b024d556f12f7484d) )
	ROM_LOAD( "sf04.bin",       0xf000, 0x0800, CRC(059dd113) SHA1(23f908e8f456843a3360ece713dba8d2b4d16a63) )
	ROM_LOAD( "sf05.bin",       0xf800, 0x0800, CRC(f4669140) SHA1(45b53ed9e65d16fd463df812cbf3d796bd30424f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "mmi6341-1j.12c", 0x0000, 0x0200, CRC(528034d3) SHA1(29ef9cfe2540f9a1fb9d0184a4c8fd74a4d6e6ba) )
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


ROM_START( satsf3d )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b8-2.5d",  0xdc00, 0x0400, CRC(4f0fc817) SHA1(b4fed244f93425107d36fa8f61ada5dc92034716) )
	ROM_LOAD( "b7-1.4e",  0xe000, 0x0400, CRC(6235c388) SHA1(57645eeffea61b06e38819e5f17fbeaa412f673d) )
	ROM_LOAD( "b6-1.4f",  0xe400, 0x0400, CRC(56735767) SHA1(73cd8a4dfa88de4d9e9451b36440688984af79a4) )
	ROM_LOAD( "b5-2.5e",  0xe800, 0x0400, CRC(0af82ccf) SHA1(7924b6be2a71361a1f054bf7cb95a00367516267) )
	ROM_LOAD( "b4-1.5f",  0xec00, 0x0400, CRC(d68b9c7a) SHA1(b03322557be60d14d56b624a120807bcaa18a905) )
	ROM_LOAD( "b3-1.4h",  0xf000, 0x0400, CRC(cc4ecb1b) SHA1(8ecf39c62ec6a4a8a7bf02a044c9cc492070e1aa) )
	ROM_LOAD( "b2-1.4k",  0xf400, 0x0400, CRC(84cb830c) SHA1(ddc67c48ae80f8833330517015fac66433a65759) )
	ROM_LOAD( "b1-1.5h",  0xf800, 0x0400, CRC(53e32287) SHA1(9a924dfc3c175641c15b759302ddcc4edf44fe74) )
	ROM_LOAD( "b0-1.5k",  0xfc00, 0x0400, CRC(c79a8346) SHA1(a46a66e080603c2cee09378f59f52986f48c6151) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "709-5.1a", 0x0000, 0x0020, CRC(61329fd1) SHA1(15782d8757d4dda5a8b97815e94c90218f0e08dd) BAD_DUMP )
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

void astrof_state::init_abattle()
{
	/* use the protection PROM to decrypt the ROMs */
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *prom = memregion("user1")->base();

	for (int i = 0xd000; i < 0x10000; i++)
		rom[i] = prom[rom[i]];

	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8smo_delegate(*this, FUNC(astrof_state::shoot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8smo_delegate(*this, FUNC(astrof_state::abattle_coin_prot_r)));
}


void astrof_state::init_afire()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0xd000; i < 0x10000; i++)
		rom[i] = ~rom[i];

	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8smo_delegate(*this, FUNC(astrof_state::shoot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8smo_delegate(*this, FUNC(astrof_state::afire_coin_prot_r)));
}


void astrof_state::init_sstarbtl()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0xd000; i < 0x10000; i++)
		rom[i] = ~rom[i];

	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8smo_delegate(*this, FUNC(astrof_state::shoot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8smo_delegate(*this, FUNC(astrof_state::abattle_coin_prot_r)));
}

void astrof_state::init_acombat3()
{
	/* set up protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8smo_delegate(*this, FUNC(astrof_state::shoot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8smo_delegate(*this, FUNC(astrof_state::abattle_coin_prot_r)));
}


void astrof_state::init_asterion()
{
	// use the protection PROM to decrypt the ROMs
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *prom = memregion("user1")->base();

	for (int i = 0xd000; i < 0x10000; i++)
	{
		rom[i] = prom[(rom[i] & 0x1f) + ((rom[i] & 0xe0) << 1)];
	}

	// set up protection handlers
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa003, 0xa003, read8smo_delegate(*this, FUNC(astrof_state::shoot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xa004, 0xa004, read8smo_delegate(*this, FUNC(astrof_state::abattle_coin_prot_r)));
}


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, astrof,    0,        astrof,   astrof,    astrof_state, empty_init,    ROT90, "Data East",                "Astro Fighter (set 1)",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, astrof2,   astrof,   astrof,   astrof,    astrof_state, empty_init,    ROT90, "Data East",                "Astro Fighter (set 2)",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, astrof3,   astrof,   astrof,   astrof,    astrof_state, empty_init,    ROT90, "Data East",                "Astro Fighter (set 3)",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, astroft,   astrof,   astrof,   astrof,    astrof_state, empty_init,    ROT90, "Data East (Taito license)","Astro Fighter (Taito)",                   MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, astroff,   astrof,   astrof,   astrof,    astrof_state, empty_init,    ROT90, "bootleg (Famaresa)",       "Astro Fighter (Famaresa bootleg, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, astroff2,  astrof,   astrof,   astrof,    astrof_state, empty_init,    ROT90, "bootleg (Famaresa)",       "Astro Fighter (Famaresa bootleg, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, abattle,   astrof,   abattle,  abattle,   astrof_state, init_abattle,  ROT90, "bootleg? (Sidam)",         "Astro Battle (set 1)",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, abattle2,  astrof,   abattle,  abattle,   astrof_state, init_abattle,  ROT90, "bootleg? (Sidam)",         "Astro Battle (set 2)",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, afire,     astrof,   abattle,  abattle,   astrof_state, init_afire,    ROT90, "bootleg (Rene Pierre)",    "Astro Fire",                              MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, asterion,  astrof,   abattle,  abattle,   astrof_state, init_asterion, ROT90, "bootleg? (Olympia)",       "Asterion",                                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, acombat,   astrof,   abattle,  abattle,   astrof_state, init_afire,    ROT90, "bootleg",                  "Astro Combat (newer, CB)",                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, acombato,  astrof,   abattle,  abattle,   astrof_state, init_afire,    ROT90, "bootleg",                  "Astro Combat (older, PZ)",                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, acombat3,  astrof,   abattle,  abattle,   astrof_state, init_acombat3, ROT90, "bootleg (Proel)",          "Astro Combat (unencrypted)",              MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, acombat4,  astrof,   abattle,  abattle,   astrof_state, init_abattle,  ROT90, "bootleg (Proel)",          "Astro Combat (encrypted)",                MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1979, strfight,  astrof,   abattle,  abattle,   astrof_state, init_acombat3, ROT90, "bootleg (VGG)",            "Star Fighter (bootleg of Astro Fighter)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, sstarbtl,  astrof,   abattle,  abattle,   astrof_state, init_sstarbtl, ROT90, "bootleg (SG-Florence)",    "Super Star Battle",                       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1979, spfghmk2,  0,        spfghmk2, spfghmk2,  astrof_state, empty_init,    ROT90, "Data East",                "Space Fighter Mark II (set 1)",           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, spfghmk22, spfghmk2, spfghmk2, spfghmk22, astrof_state, empty_init,    ROT90, "Data East",                "Space Fighter Mark II (set 2)",           MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1979, satsf3d,   0,        spfghmk2, spfghmk2,  astrof_state, empty_init,    ROT90, "Data East",                "Saturn: Space Fighter 3D",                MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1980, tomahawk,  0,        tomahawk, tomahawk,  astrof_state, empty_init,    ROT90, "Data East",                "Tomahawk 777 (rev 5)",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, tomahawk1, tomahawk, tomahawk, tomahawk1, astrof_state, empty_init,    ROT90, "Data East",                "Tomahawk 777 (rev 1)",                    MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
