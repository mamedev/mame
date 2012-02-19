/***************************************************************************

Misc early Z80 games with simple color bitmap graphics

    - Space King 2    (c) Konami
    - Kosmo Killer    bootleg
    - Kamikaze        (c) Leijac Corporation
    - Astro Invader   (c) Stern Electronics
    - Space Intruder  (c) Shoei

Space Intruder emulation by Lee Taylor (lee@defender.demon.co.uk),
    December 1998.

DIP locations verified for:
    - astinvad (manual)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/8255ppi.h"
#include "sound/samples.h"


#define MASTER_CLOCK         XTAL_2MHz
#define VIDEO_CLOCK          XTAL_4_9152MHz


/* sample sound IDs - must match sample file name table below */
enum
{
	SND_UFO = 0,
	SND_SHOT,
	SND_BASEHIT,
	SND_INVADERHIT,
	SND_FLEET1,
	SND_FLEET2,
	SND_FLEET3,
	SND_FLEET4,
	SND_UFOHIT
};


class astinvad_state : public driver_device
{
public:
	astinvad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *    m_colorram;
	UINT8 *    m_videoram;
	size_t     m_videoram_size;

	emu_timer  *m_int_timer;
	UINT8      m_sound_state[2];
	UINT8      m_screen_flip;
	UINT8      m_screen_red;
	UINT8      m_flip_yoffs;
	UINT8      m_color_latch;

	device_t *m_maincpu;
	device_t *m_ppi8255_0;
	device_t *m_ppi8255_1;
	samples_device *m_samples;
};


/*************************************
 *
 *  Prototypes and interfaces
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( astinvad_sound1_w );
static WRITE8_DEVICE_HANDLER( astinvad_sound2_w );

static const ppi8255_interface ppi8255_intf[2] =
{
	{
		DEVCB_INPUT_PORT("IN0"),
		DEVCB_INPUT_PORT("IN1"),
		DEVCB_INPUT_PORT("IN2"),
		DEVCB_NULL,
		DEVCB_NULL,
		DEVCB_NULL
	},
	{
		DEVCB_NULL,
		DEVCB_INPUT_PORT("CABINET"),
		DEVCB_NULL,
		DEVCB_HANDLER(astinvad_sound1_w),
		DEVCB_HANDLER(astinvad_sound2_w),
		DEVCB_NULL
	}
};



/*************************************
 *
 *  Spaceint color RAM handling
 *
 *************************************/

static VIDEO_START( spaceint )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();
	state->m_colorram = auto_alloc_array(machine, UINT8, state->m_videoram_size);

	state->save_item(NAME(state->m_color_latch));
	state->save_pointer(NAME(state->m_colorram), state->m_videoram_size);
}


static WRITE8_HANDLER( color_latch_w )
{
	astinvad_state *state = space->machine().driver_data<astinvad_state>();
	state->m_color_latch = data & 0x0f;
}


static WRITE8_HANDLER( spaceint_videoram_w )
{
	astinvad_state *state = space->machine().driver_data<astinvad_state>();
	state->m_videoram[offset] = data;
	state->m_colorram[offset] = state->m_color_latch;
}



/*************************************
 *
 *  Spaceint color RAM handling
 *
 *************************************/

static void plot_byte( running_machine &machine, bitmap_rgb32 &bitmap, UINT8 y, UINT8 x, UINT8 data, UINT8 color )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();
	pen_t fore_pen = MAKE_RGB(pal1bit(color >> 0), pal1bit(color >> 2), pal1bit(color >> 1));
	UINT8 flip_xor = state->m_screen_flip & 7;

	bitmap.pix32(y, x + (0 ^ flip_xor)) = (data & 0x01) ? fore_pen : RGB_BLACK;
	bitmap.pix32(y, x + (1 ^ flip_xor)) = (data & 0x02) ? fore_pen : RGB_BLACK;
	bitmap.pix32(y, x + (2 ^ flip_xor)) = (data & 0x04) ? fore_pen : RGB_BLACK;
	bitmap.pix32(y, x + (3 ^ flip_xor)) = (data & 0x08) ? fore_pen : RGB_BLACK;
	bitmap.pix32(y, x + (4 ^ flip_xor)) = (data & 0x10) ? fore_pen : RGB_BLACK;
	bitmap.pix32(y, x + (5 ^ flip_xor)) = (data & 0x20) ? fore_pen : RGB_BLACK;
	bitmap.pix32(y, x + (6 ^ flip_xor)) = (data & 0x40) ? fore_pen : RGB_BLACK;
	bitmap.pix32(y, x + (7 ^ flip_xor)) = (data & 0x80) ? fore_pen : RGB_BLACK;
}


static SCREEN_UPDATE_RGB32( astinvad )
{
	astinvad_state *state = screen.machine().driver_data<astinvad_state>();
	const UINT8 *color_prom = screen.machine().region("proms")->base();
	UINT8 yoffs = state->m_flip_yoffs & state->m_screen_flip;
	int x, y;

	/* render the visible pixels */
	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		for (x = cliprect.min_x & ~7; x <= cliprect.max_x; x += 8)
		{
			UINT8 color = color_prom[((y & 0xf8) << 2) | (x >> 3)] >> (state->m_screen_flip ? 0 : 4);
			UINT8 data = state->m_videoram[(((y ^ state->m_screen_flip) + yoffs) << 5) | ((x ^ state->m_screen_flip) >> 3)];
			plot_byte(screen.machine(), bitmap, y, x, data, state->m_screen_red ? 1 : color);
		}

	return 0;
}


static SCREEN_UPDATE_RGB32( spaceint )
{
	astinvad_state *state = screen.machine().driver_data<astinvad_state>();
	const UINT8 *color_prom = screen.machine().region("proms")->base();
	int offs;

	for (offs = 0; offs < state->m_videoram_size; offs++)
	{
		UINT8 data = state->m_videoram[offs];
		UINT8 color = state->m_colorram[offs];

		UINT8 y = ~offs;
		UINT8 x = offs >> 8 << 3;

		/* this is almost certainly wrong */
		offs_t n = ((offs >> 5) & 0xf0) | color;
		color = color_prom[n] & 0x07;

		plot_byte(screen.machine(), bitmap, y, x, data, color);
	}

	return 0;
}



/*************************************
 *
 *  Interrupts
 *
 *************************************/

static TIMER_CALLBACK( kamikaze_int_off )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();
	device_set_input_line(state->m_maincpu, 0, CLEAR_LINE);
}


static TIMER_CALLBACK( kamizake_int_gen )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();
	/* interrupts are asserted on every state change of the 128V line */
	device_set_input_line(state->m_maincpu, 0, ASSERT_LINE);
	param ^= 128;
	state->m_int_timer->adjust(machine.primary_screen->time_until_pos(param), param);

	/* an RC circuit turns the interrupt off after a short amount of time */
	machine.scheduler().timer_set(attotime::from_double(300 * 0.1e-6), FUNC(kamikaze_int_off));
}


static MACHINE_START( kamikaze )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_ppi8255_0 = machine.device("ppi8255_0");
	state->m_ppi8255_1 = machine.device("ppi8255_1");
	state->m_samples = machine.device<samples_device>("samples");

	state->m_int_timer = machine.scheduler().timer_alloc(FUNC(kamizake_int_gen));
	state->m_int_timer->adjust(machine.primary_screen->time_until_pos(128), 128);

	state->save_item(NAME(state->m_screen_flip));
	state->save_item(NAME(state->m_screen_red));
	state->save_item(NAME(state->m_sound_state));
}

static MACHINE_RESET( kamikaze )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();

	state->m_screen_flip = 0;
	state->m_screen_red = 0;
	state->m_sound_state[0] = 0;
	state->m_sound_state[1] = 0;
}


static MACHINE_START( spaceint )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();

	state->m_maincpu = machine.device("maincpu");
	state->m_samples = machine.device<samples_device>("samples");

	state->save_item(NAME(state->m_screen_flip));
	state->save_item(NAME(state->m_sound_state));
}

static MACHINE_RESET( spaceint )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();

	state->m_screen_flip = 0;
	state->m_sound_state[0] = 0;
	state->m_sound_state[1] = 0;
	state->m_color_latch = 0;
}


static INPUT_CHANGED( spaceint_coin_inserted )
{
	astinvad_state *state = field.machine().driver_data<astinvad_state>();
	/* coin insertion causes an NMI */
	device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, newval ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  8255 PPI handlers
 *
 *************************************/

static READ8_HANDLER( kamikaze_ppi_r )
{
	astinvad_state *state = space->machine().driver_data<astinvad_state>();
	UINT8 result = 0xff;

	/* the address lines are used for /CS; yes, they can overlap! */
	if (!(offset & 4))
		result &= ppi8255_r(state->m_ppi8255_0, offset);
	if (!(offset & 8))
		result &= ppi8255_r(state->m_ppi8255_1, offset);
	return result;
}


static WRITE8_HANDLER( kamikaze_ppi_w )
{
	astinvad_state *state = space->machine().driver_data<astinvad_state>();

	/* the address lines are used for /CS; yes, they can overlap! */
	if (!(offset & 4))
		ppi8255_w(state->m_ppi8255_0, offset, data);
	if (!(offset & 8))
		ppi8255_w(state->m_ppi8255_1, offset, data);
}



/*************************************
 *
 *  Sound and I/O port handlers
 *
 *************************************/

static WRITE8_DEVICE_HANDLER( astinvad_sound1_w )
{
	astinvad_state *state = device->machine().driver_data<astinvad_state>();
	int bits_gone_hi = data & ~state->m_sound_state[0];
	state->m_sound_state[0] = data;

	if (bits_gone_hi & 0x01) state->m_samples->start(0, SND_UFO, true);
	if (!(data & 0x01))      state->m_samples->stop(0);
	if (bits_gone_hi & 0x02) state->m_samples->start(1, SND_SHOT);
	if (bits_gone_hi & 0x04) state->m_samples->start(2, SND_BASEHIT);
	if (bits_gone_hi & 0x08) state->m_samples->start(3, SND_INVADERHIT);

	device->machine().sound().system_enable(data & 0x20);
	state->m_screen_red = data & 0x04;
}


static WRITE8_DEVICE_HANDLER( astinvad_sound2_w )
{
	astinvad_state *state = device->machine().driver_data<astinvad_state>();
	int bits_gone_hi = data & ~state->m_sound_state[1];
	state->m_sound_state[1] = data;

	if (bits_gone_hi & 0x01) state->m_samples->start(5, SND_FLEET1);
	if (bits_gone_hi & 0x02) state->m_samples->start(5, SND_FLEET2);
	if (bits_gone_hi & 0x04) state->m_samples->start(5, SND_FLEET3);
	if (bits_gone_hi & 0x08) state->m_samples->start(5, SND_FLEET4);
	if (bits_gone_hi & 0x10) state->m_samples->start(4, SND_UFOHIT);

	state->m_screen_flip = (input_port_read(device->machine(), "CABINET") & data & 0x20) ? 0xff : 0x00;
}


static WRITE8_HANDLER( spaceint_sound1_w )
{
	astinvad_state *state = space->machine().driver_data<astinvad_state>();
	int bits_gone_hi = data & ~state->m_sound_state[0];
	state->m_sound_state[0] = data;

	if (bits_gone_hi & 0x01) state->m_samples->start(1, SND_SHOT);
	if (bits_gone_hi & 0x02) state->m_samples->start(2, SND_BASEHIT);
	if (bits_gone_hi & 0x04) state->m_samples->start(4, SND_UFOHIT);
	if (bits_gone_hi & 0x08) state->m_samples->start(0, SND_UFO, true);
	if (!(data & 0x08))      state->m_samples->stop(0);

	if (bits_gone_hi & 0x10) state->m_samples->start(5, SND_FLEET1);
	if (bits_gone_hi & 0x20) state->m_samples->start(5, SND_FLEET2);
	if (bits_gone_hi & 0x40) state->m_samples->start(5, SND_FLEET3);
	if (bits_gone_hi & 0x80) state->m_samples->start(5, SND_FLEET4);
}


static WRITE8_HANDLER( spaceint_sound2_w )
{
	astinvad_state *state = space->machine().driver_data<astinvad_state>();
	int bits_gone_hi = data & ~state->m_sound_state[1];
	state->m_sound_state[1] = data;

	space->machine().sound().system_enable(data & 0x02);

	if (bits_gone_hi & 0x04) state->m_samples->start(3, SND_INVADERHIT);

	state->m_screen_flip = (input_port_read(space->machine(), "CABINET") & data & 0x80) ? 0xff : 0x00;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( kamikaze_map, AS_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x1bff) AM_ROM
	AM_RANGE(0x1c00, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_BASE_SIZE_MEMBER(astinvad_state, m_videoram, m_videoram_size)
ADDRESS_MAP_END


static ADDRESS_MAP_START( spaceint_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x5fff) AM_RAM_WRITE(spaceint_videoram_w) AM_BASE_SIZE_MEMBER(astinvad_state, m_videoram, m_videoram_size)
ADDRESS_MAP_END


static ADDRESS_MAP_START( kamikaze_portmap, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0xff) AM_READWRITE(kamikaze_ppi_r, kamikaze_ppi_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( spaceint_portmap, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x02, 0x02) AM_WRITE(spaceint_sound1_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(color_latch_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(spaceint_sound2_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( kamikaze )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME( 0x88, 0x88, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x88, "5000" )
	PORT_DIPSETTING(    0x80, "10000" )
	PORT_DIPSETTING(    0x08, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("CABINET")
	PORT_DIPNAME( 0xff, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0xff, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( astinvad )
	PORT_INCLUDE(kamikaze)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Bonus_Life ) )       PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x88, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x88, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( spcking2 )
	PORT_INCLUDE(kamikaze)

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Info" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( spaceint )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN1")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )            /* code at 0x0d4a */
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
//  PORT_DIPSETTING(    0x06, "5" )                         /* duplicate settings */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(spaceint_coin_inserted, 0)

	PORT_START("CABINET")
	PORT_DIPNAME( 0xff, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0xff, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spaceintj )
	PORT_INCLUDE( spaceint )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Lives ) )            /* code at 0x0d37 */
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x02, "5" )
//  PORT_DIPSETTING(    0x04, "5" )                         /* duplicate settings */
//  PORT_DIPSETTING(    0x06, "5" )                         /* duplicate settings */
INPUT_PORTS_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const char *const astinvad_sample_names[] =
{
	"*invaders",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	0
};

static const samples_interface astinvad_samples_interface =
{
	6,   /* channels */
	astinvad_sample_names
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( kamikaze, astinvad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(kamikaze_map)
	MCFG_CPU_IO_MAP(kamikaze_portmap)

	MCFG_MACHINE_START(kamikaze)
	MCFG_MACHINE_RESET(kamikaze)

	MCFG_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )
	MCFG_PPI8255_ADD( "ppi8255_1", ppi8255_intf[1] )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK, 320, 0, 256, 256, 32, 256)
	MCFG_SCREEN_UPDATE_STATIC(astinvad)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", astinvad_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( spcking2, kamikaze )

	/* basic machine hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK, 320, 0, 256, 256, 16, 240)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( spaceint, astinvad_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)        /* a guess */
	MCFG_CPU_PROGRAM_MAP(spaceint_map)
	MCFG_CPU_IO_MAP(spaceint_portmap)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_MACHINE_START(spaceint)
	MCFG_MACHINE_RESET(spaceint)

	/* video hardware */
	MCFG_VIDEO_START(spaceint)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_STATIC(spaceint)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SAMPLES_ADD("samples", astinvad_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( kamikaze )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "km01",         0x0000, 0x0800, CRC(8aae7414) SHA1(91cb5c268a03960d50401000903d70dc29f904fb) )
	ROM_LOAD( "km02",         0x0800, 0x0800, CRC(6c7a2beb) SHA1(86447d077a58e8c1fc096d0d32b02d18523019a6) )
	ROM_LOAD( "km03",         0x1000, 0x0800, CRC(3e8dedb6) SHA1(19679d0e8ebe2d19dc766b12a07335b1220fb568) )
	ROM_LOAD( "km04",         0x1800, 0x0800, CRC(494e1f6d) SHA1(f9626072d80897a977c10fe9523a8b608f1f7b7c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ai_vid_c.rom", 0x0000, 0x0400, BAD_DUMP CRC(b45287ff) SHA1(7e558eaf402641d7ff60171f854030219fbf9a59)  )
ROM_END

ROM_START( astinvad )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ai_cpu_1.rom", 0x0000, 0x0400, CRC(20e3ec41) SHA1(7e77fa3c51d1e83ce91a24808301d9f1e0bed18e) )
	ROM_LOAD( "ai_cpu_2.rom", 0x0400, 0x0400, CRC(e8f1ab55) SHA1(b3e38f2d6bdb65ee7c53c8d5dd3951a3fd43c51c) )
	ROM_LOAD( "ai_cpu_3.rom", 0x0800, 0x0400, CRC(a0092553) SHA1(34fced8ce06d912980ba45fad8d80d2a2e3357b9) )
	ROM_LOAD( "ai_cpu_4.rom", 0x0c00, 0x0400, CRC(be14185c) SHA1(59ecf450682dab9840c891c18ccda1d5ec4cc954) )
	ROM_LOAD( "ai_cpu_5.rom", 0x1000, 0x0400, CRC(fee681ec) SHA1(b4b94f62e598030e6a432a0bb83d18d0e342aed9) )
	ROM_LOAD( "ai_cpu_6.rom", 0x1400, 0x0400, CRC(eb338863) SHA1(e841c6c5903dd6dee9ec2fedaff431f4a31d738a) )
	ROM_LOAD( "ai_cpu_7.rom", 0x1800, 0x0400, CRC(16dcfea4) SHA1(b6a0e206a604297f548ac4658664e98b2d04f75f) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ai_vid_c.rom", 0x0000, 0x0400, CRC(b45287ff) SHA1(7e558eaf402641d7ff60171f854030219fbf9a59) )
ROM_END

ROM_START( kosmokil )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",     0x0000, 0x0400, CRC(8d851fb2) SHA1(06b3816ecb45c5d034447a875669a96f443339f5) )
	ROM_LOAD( "us1-2.bin", 0x0400, 0x0400, CRC(786599d2) SHA1(70db8dae052c3556948d75b741ef4346aa947479) )
	ROM_LOAD( "si1-3.bin", 0x0800, 0x0400, CRC(12621222) SHA1(062b1dff3e129dff23e55bef0d29c72ac5f212c4) )
	ROM_LOAD( "ib1-4.bin", 0x0c00, 0x0400, CRC(a5c56156) SHA1(557f5fbb5e9fe4d7450f3cf97ed3c935b9dd5c1a) )
	ROM_LOAD( "bi1-5.bin", 0x1000, 0x0400, CRC(5e8b2b6f) SHA1(ec8499325d5a3dcb0d10e9f12b9d3a03f629bbfd) )
	ROM_LOAD( "il1-6.bin", 0x1400, 0x0400, CRC(a076de05) SHA1(bdb076b89795d69824c99a27473089f203690055) )
	ROM_LOAD( "li1-7.bin", 0x1800, 0x0400, CRC(ee9adb63) SHA1(038e6cadfdfe05c45a79ed9a54cff428de33d24c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "40.bin", 0x0000, 0x0400,  CRC(d62a3e62) SHA1(00d42988203fbf167791cf5b887f06d1d015e942) )
ROM_END

ROM_START( spcking2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x0400, CRC(716fe9e0) SHA1(d5131abf6e3e6650ff9f649a999bf1d8ae8afb78) )
	ROM_LOAD( "2.bin",        0x0400, 0x0400, CRC(6f6d4e5c) SHA1(0269c3b9da2723411c16ee13ff53e2140e49e7ff) )
	ROM_LOAD( "3.bin",        0x0800, 0x0400, CRC(2ab1c280) SHA1(62cb2445b3f859bddd5617e4ebfb37eedf8bd11e) )
	ROM_LOAD( "4.bin",        0x0c00, 0x0400, CRC(07ba1f21) SHA1(26468e142edef3475e71320292bd1817552a9218) )
	ROM_LOAD( "5.bin",        0x1000, 0x0400, CRC(b084c074) SHA1(1c7e86ae35cd69679712cd8a209b4a70a2075163) )
	ROM_LOAD( "6.bin",        0x1400, 0x0400, CRC(b53d7791) SHA1(45415bcccb03a9c61cea611df807b011e8cc0d2d) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "c.bin",        0x0000, 0x0400, CRC(d27fe595) SHA1(1781281110b57ab3a5eef7a3dbaa93f11c013554) )
ROM_END

ROM_START( spaceint )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1",			  0x0000, 0x0400, CRC(184314d2) SHA1(76789780c46e19c73904b229d23c865819915558) )
	ROM_LOAD( "2",			  0x0400, 0x0400, CRC(55459aa1) SHA1(5631d8de4e41682962cde65002b0fe86f2b189f9) )
	ROM_LOAD( "3",			  0x0800, 0x0400, CRC(9d6819be) SHA1(da061b908ca6a9f3312d6adc4395a138eed473c8) )
	ROM_LOAD( "4",			  0x0c00, 0x0400, CRC(432052d4) SHA1(0c944c91cc7b1f03cd817250af13238eb62539ec) )
	ROM_LOAD( "5",			  0x1000, 0x0400, CRC(c6cfa650) SHA1(afdfaedddf6703101856944bb49ba13fc40ede39) )
	ROM_LOAD( "6",			  0x1400, 0x0400, CRC(c7ccf40f) SHA1(10efe05a4e0625ce427871fbb6e55df112fdd783) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "clr",		  0x0000, 0x0100, CRC(13c1803f) SHA1(da59bf63d9e84aca32904c107674bc89974648eb) )
ROM_END

ROM_START( spaceintj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3j",			  0x0000, 0x0800, CRC(b26c57a1) SHA1(456330c09130f910e847ef4bfe773421615d1448) )
	ROM_LOAD( "3f",			  0x0800, 0x0800, CRC(bac8b96c) SHA1(5a7b24402c7a1a08e69cf15eb31c93d411a7e929) )
	ROM_LOAD( "3e",			  0x1000, 0x0800, CRC(346125f3) SHA1(59c120ac3b120fa28acef3b9041c03939f2981f8) )
	ROM_LOAD( "3d",			  0x1800, 0x0800, CRC(3a3a261f) SHA1(0604ec621180016acab804b57ac405e434d6f0c0) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "clr",		  0x0000, 0x0100, BAD_DUMP CRC(13c1803f) SHA1(da59bf63d9e84aca32904c107674bc89974648eb) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( kamikaze )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();

	/* the flip screen logic adds 32 to the Y after flipping */
	state->m_flip_yoffs = 32;
}


static DRIVER_INIT( spcking2 )
{
	astinvad_state *state = machine.driver_data<astinvad_state>();

	/* don't have the schematics, but the blanking must center the screen here */
	state->m_flip_yoffs = 0;
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, kamikaze, 0,        kamikaze, kamikaze, kamikaze, ROT270, "Leijac Corporation", "Kamikaze",      GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1980, astinvad, kamikaze, kamikaze, astinvad, kamikaze, ROT270, "Leijac Corporation (Stern Electronics license)", "Astro Invader", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 19??, kosmokil, kamikaze, kamikaze, kamikaze, kamikaze, ROT270, "bootleg",            "Kosmo Killer",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE ) // says >BEM< Mi Italy but it looks hacked in, dif revision of game tho.
GAME( 1979, spcking2, 0,        spcking2, spcking2, spcking2, ROT270, "Konami",             "Space King 2",  GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1980, spaceint, 0,        spaceint, spaceint, 0,        ROT90,  "Shoei",              "Space Intruder", GAME_IMPERFECT_SOUND | GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
GAME( 1980, spaceintj,spaceint, spaceint, spaceintj,0,        ROT90,  "Shoei",              "Space Intruder (Japan)", GAME_IMPERFECT_SOUND | GAME_WRONG_COLORS | GAME_SUPPORTS_SAVE )
