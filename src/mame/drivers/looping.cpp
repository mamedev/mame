// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*
To Do:
- redump COP420 internal ROM
- get sound working
- map and test any remaining input ports

Looping
(C)1981 Venture Line

    Main CPU
        TMS9995

    COP420 Microcontroller
        protection

    Sound CPU
        TMS9980
        AY-3-8910
        TMS5220 (SPEECH)

---------------------------------------------------------------

Sky Bumper
(C)1982 Venture Line

    This is a ROM swap for Looping.  There are two 6116's on
    the CPU board, where there is only one on Looping.

===============================================================

LOOPING CHIP PLACEMENT

THERE ARE AT LEAST TWO VERSIONS OF THIS GAME
VERSION NUMBERS FOR THIS PURPOSE ARE CHOSEN AT RANDOM

IC NAME   POSITION   BOARD  TYPE   IC NAME  POSITION  TYPE
VER-1                         VER-2
---------------------------------------------------------------
LOS-2-7   13A        I/O    2532    SAME    13A       2532
LOS-1-1-2 11A         "      "      SAME    11A        "
LOS-3-1   13C         "      "      I-O-V2  13C        "

VLI1      2A         ROM    2764    VLI-7-1 2A         "
VLI3      5A          "      "      VLI-7-2 4A         "
VLI9-5    8A          "      "      VLI-4-3 5A         "
L056-6    9A          "      "      VLI-8-4 7A         "
                      "             LO56-5  8A         "
                      "             LO56-6  9A         "
                      "             VLI-8-7 10A        "
                  ON RIBBON CABLE   18S030  11B             color prom?
                     REAR BD      LOG.1-9-3 6A        2716  tiles
                                  LOG.3     8A         "    tiles
*/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "cpu/tms9900/tms9980a.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/tms5220.h"
#include "video/resnet.h"
#include "cpu/cop400/cop400.h"


/*************************************
 *
 *  Constants
 *
 *************************************/

#define MAIN_CPU_CLOCK      (12000000)
#define SOUND_CLOCK         (8000000)
#define COP_CLOCK           (SOUND_CLOCK/2)     /* unknown guess */
#define TMS_CLOCK           (640000)

/* the schematics are very blurry here and don't actually specify the clock */
/* values; however, everything else about the sync chain implies the standard */
/* 18.432MHz master clock, like is used in similar hardware of the time */
#define MASTER_CLOCK        (18432000)

#define PIXEL_CLOCK         (MASTER_CLOCK/3)

#define HTOTAL              (384)
#define HBEND               (0)
#define HBSTART             (256)

#define VTOTAL              (264)
#define VBEND               (16)
#define VBSTART             (224+16)


/*************************************
 *
 *  Type definitions
 *
 *************************************/

class looping_state : public driver_device
{
public:
	looping_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;
	UINT8 m_cop_port_l;

	/* tilemaps */
	tilemap_t * m_bg_tilemap;

	/* sound state */
	UINT8       m_sound[8];

	int     m_last;

	DECLARE_WRITE8_MEMBER(flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(looping_videoram_w);
	DECLARE_WRITE8_MEMBER(looping_colorram_w);
	DECLARE_WRITE8_MEMBER(level2_irq_set);
	DECLARE_WRITE8_MEMBER(main_irq_ack_w);
	DECLARE_WRITE8_MEMBER(looping_souint_clr);
	DECLARE_WRITE8_MEMBER(looping_soundlatch_w);
	DECLARE_WRITE8_MEMBER(ballon_enable_w);
	DECLARE_WRITE8_MEMBER(out_0_w);
	DECLARE_WRITE8_MEMBER(out_2_w);
	DECLARE_READ8_MEMBER(adc_r);
	DECLARE_WRITE8_MEMBER(adc_w);
	DECLARE_WRITE8_MEMBER(plr2_w);
	DECLARE_READ8_MEMBER(cop_unk_r);
	DECLARE_READ_LINE_MEMBER(cop_serial_r);
	DECLARE_WRITE8_MEMBER(cop_l_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE_LINE_MEMBER(looping_spcint);
	DECLARE_WRITE8_MEMBER(looping_sound_sw);
	DECLARE_WRITE8_MEMBER(ay_enable_w);
	DECLARE_WRITE8_MEMBER(speech_enable_w);
	DECLARE_DRIVER_INIT(looping);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(looping);
	UINT32 screen_update_looping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(looping_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



/*************************************
 *
 *  Palette conversion
 *
 *************************************/

PALETTE_INIT_MEMBER(looping_state, looping)
{
	const UINT8 *color_prom = memregion("proms")->base();
	static const int resistances[3] = { 1000, 470, 220 };
	double rweights[3], gweights[3], bweights[2];
	int i;

	/* compute the color output resistor weights */
	compute_resistor_weights(0, 255, -1.0,
			3,  &resistances[0], rweights, 470, 0,
			3,  &resistances[0], gweights, 470, 0,
			2,  &resistances[1], bweights, 470, 0);

	/* initialize the palette with these colors */
	for (i = 0; i < 32; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 1;
		bit1 = (color_prom[i] >> 1) & 1;
		bit2 = (color_prom[i] >> 2) & 1;
		r = combine_3_weights(rweights, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 1;
		bit1 = (color_prom[i] >> 4) & 1;
		bit2 = (color_prom[i] >> 5) & 1;
		g = combine_3_weights(gweights, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 1;
		bit1 = (color_prom[i] >> 7) & 1;
		b = combine_2_weights(bweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Video startup/tilemap config
 *
 *************************************/

TILE_GET_INFO_MEMBER(looping_state::get_tile_info)
{
	int tile_number = m_videoram[tile_index];
	int color = m_colorram[(tile_index & 0x1f) * 2 + 1] & 0x07;
	SET_TILE_INFO_MEMBER(0, tile_number, color, 0);
}


void looping_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(looping_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_bg_tilemap->set_scroll_cols(0x20);
}



/*************************************
 *
 *  Video write handlers
 *
 *************************************/

WRITE8_MEMBER(looping_state::flip_screen_x_w)
{
	flip_screen_x_set(~data & 0x01);
	m_bg_tilemap->set_scrollx(0, flip_screen() ? 128 : 0);
}


WRITE8_MEMBER(looping_state::flip_screen_y_w)
{
	flip_screen_y_set(~data & 0x01);
	m_bg_tilemap->set_scrollx(0, flip_screen() ? 128 : 0);
}


WRITE8_MEMBER(looping_state::looping_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(looping_state::looping_colorram_w)
{
	int i;

	m_colorram[offset] = data;

	/* odd bytes are column color attribute */
	if (offset & 1)
	{
		/* mark the whole column dirty */
		offs_t offs = (offset/2);
		for (i = 0; i < 0x20; i++)
			m_bg_tilemap->mark_tile_dirty(i * 0x20 + offs);
	}

	/* even bytes are column scroll */
	else
		m_bg_tilemap->set_scrolly(offset/2, data);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void looping_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const UINT8 *source;

	for (source = m_spriteram; source < m_spriteram + 0x40; source += 4)
	{
		int sx = source[3];
		int sy = 240 - source[0];
		int flipx = source[1] & 0x40;
		int flipy = source[1] & 0x80;
		int code  = source[1] & 0x3f;
		int color = source[2];

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, code, color, flipx, flipy, sx, sy, 0);
	}
}


UINT32 looping_state::screen_update_looping(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	return 0;
}



/*************************************
 *
 *  Machine init/reset
 *
 *************************************/

void looping_state::machine_start()
{
	save_item(NAME(m_sound));
}

void looping_state::machine_reset()
{
	// Disable auto wait state generation by raising the READY line on reset
	static_cast<tms9995_device*>(machine().device("maincpu"))->ready_line(ASSERT_LINE);
}

/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

INTERRUPT_GEN_MEMBER(looping_state::looping_interrupt)
{
	m_maincpu->set_input_line(INT_9995_INT1, ASSERT_LINE);
}


WRITE8_MEMBER(looping_state::level2_irq_set)
{
	logerror("Level 2 int = %d\n", data);
	if (!(data & 1))
		m_maincpu->set_input_line(INT_9995_INT1, ASSERT_LINE);
}


WRITE8_MEMBER(looping_state::main_irq_ack_w)
{
	if (data == 0)
		m_maincpu->set_input_line(INT_9995_INT1, CLEAR_LINE);
}


WRITE8_MEMBER(looping_state::looping_souint_clr)
{
	logerror("Soundint clr = %d\n", data);
	if (data == 0)
		m_audiocpu->set_input_line(0, CLEAR_LINE);
}


WRITE_LINE_MEMBER(looping_state::looping_spcint)
{
	logerror("Speech /int = %d\n", state==ASSERT_LINE? 1:0);
	m_audiocpu->set_input_line(INT_9980A_LEVEL4, state==ASSERT_LINE? CLEAR_LINE : ASSERT_LINE);
}


WRITE8_MEMBER(looping_state::looping_soundlatch_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(INT_9980A_LEVEL2, ASSERT_LINE);
}

/*************************************
 *
 *  Custom DAC handling
 *
 *************************************/

WRITE8_MEMBER(looping_state::looping_sound_sw)
{
	/* this can be improved by adding the missing signals for decay etc. (see schematics)

	    0001 = ASOV
	    0002 = AVOL2
	    0003 = AVOL1
	    0004 = ADECAY1
	    0005 = ADECAY
	    0006 = ASA
	    0007 = AFA
	*/

	m_sound[offset + 1] = data ^ 1;
	m_dac->write_unsigned8(((m_sound[2] << 7) + (m_sound[3] << 6)) * m_sound[7]);
}



/*************************************
 *
 *  Sound controls
 *
 *************************************/

WRITE8_MEMBER(looping_state::ay_enable_w)
{
	device_t *device = machine().device("aysnd");
	int output;

	device_sound_interface *sound;
	device->interface(sound);
	for (output = 0; output < 3; output++)
		sound->set_output_gain(output, (data & 1) ? 1.0 : 0.0);
}


WRITE8_MEMBER(looping_state::speech_enable_w)
{
	device_t *device = machine().device("tms");
	device_sound_interface *sound;
	device->interface(sound);
	sound->set_output_gain(0, (data & 1) ? 1.0 : 0.0);
}


WRITE8_MEMBER(looping_state::ballon_enable_w)
{
	if (m_last != data)
		osd_printf_debug("ballon_enable_w = %d\n", data);
	m_last = data;
}



/*************************************
 *
 *  Misc I/O
 *
 *************************************/

WRITE8_MEMBER(looping_state::out_0_w){ osd_printf_debug("out0 = %02X\n", data); }
WRITE8_MEMBER(looping_state::out_2_w){ osd_printf_debug("out2 = %02X\n", data); }

READ8_MEMBER(looping_state::adc_r){ osd_printf_debug("%04X:ADC read\n", space.device().safe_pc()); return 0xff; }
WRITE8_MEMBER(looping_state::adc_w){ osd_printf_debug("%04X:ADC write = %02X\n", space.device().safe_pc(), data); }

WRITE8_MEMBER(looping_state::plr2_w)
{
	/* set to 1 after IDLE, cleared to 0 during processing */
	/* is this an LED on the PCB? */
}



/*************************************
 *
 *  Protection
 *
 *************************************/

READ8_MEMBER(looping_state::cop_unk_r)
{
	return 1;
}

READ_LINE_MEMBER(looping_state::cop_serial_r)
{
	return 1;
}

WRITE8_MEMBER(looping_state::cop_l_w)
{
	m_cop_port_l = data;
	logerror("%02x  ",data);
}

READ8_MEMBER(looping_state::protection_r)
{
//        The code reads ($7002) ($7004) alternately
//        The result must change at least once every 10 reads
//        A read from ($34b0 + result) must == $01

//        Valid values:
//            $61 $67
//            $B7 $BF
//            $DB
//            $E1
//            $F3 $F7 $FD $FF

//        Because they read alternately from different locations,
//        it is trivial to bypass the protection.

//        cop write alternately $02 $01 $08 $04 in port $102
//        cop write randomly fc (unfortunatly) but 61,67,b7,bf,db,e1,f3,fd,ff too and only these values

	// missing something
	if(m_cop_port_l != 0xfc) return m_cop_port_l;
	return 0xff;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( looping_map, AS_PROGRAM, 8, looping_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM

	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(looping_videoram_w) AM_SHARE("videoram")

	AM_RANGE(0x9800, 0x983f) AM_MIRROR(0x0700) AM_RAM_WRITE(looping_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x9840, 0x987f) AM_MIRROR(0x0700) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9880, 0x98ff) AM_MIRROR(0x0700) AM_RAM

	AM_RANGE(0xb001, 0xb001) AM_MIRROR(0x07f8) AM_WRITE(level2_irq_set)
	AM_RANGE(0xb006, 0xb006) AM_MIRROR(0x07f8) AM_WRITE(flip_screen_x_w)
	AM_RANGE(0xb007, 0xb007) AM_MIRROR(0x07f8) AM_WRITE(flip_screen_y_w)

	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_MIRROR(0x03fc) AM_READ_PORT("P1") AM_WRITE(out_0_w)                 /* /OUT0 */
	AM_RANGE(0xf801, 0xf801) AM_MIRROR(0x03fc) AM_READ_PORT("P2") AM_WRITE(looping_soundlatch_w)    /* /OUT1 */
	AM_RANGE(0xf802, 0xf802) AM_MIRROR(0x03fc) AM_READ_PORT("DSW") AM_WRITE(out_2_w)                /* /OUT2 */
	AM_RANGE(0xf803, 0xf803) AM_MIRROR(0x03fc) AM_READWRITE(adc_r, adc_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( looping_io_map, AS_IO, 8, looping_state )
	/* 400 = A16 */
	/* 401 = A17 */
	/* 402 = COLOR 9 */
	AM_RANGE(0x403, 0x403) AM_WRITE(plr2_w)
	/* 404 = C0 */
	/* 405 = C1 */
	AM_RANGE(0x406, 0x406) AM_WRITE(main_irq_ack_w)
	AM_RANGE(0x407, 0x407) AM_WRITE(watchdog_reset_w)

	AM_RANGE(0x10000, 0x10000) AM_NOP       /* external IDLE signal -- we can ignore it */
ADDRESS_MAP_END


/* complete memory map derived from schematics */
static ADDRESS_MAP_START( looping_sound_map, AS_PROGRAM, 8, looping_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x37ff) AM_ROM
	AM_RANGE(0x3800, 0x3bff) AM_RAM
	AM_RANGE(0x3c00, 0x3c00) AM_MIRROR(0x00f4) AM_DEVREADWRITE("aysnd", ay8910_device, data_r, address_w)
	AM_RANGE(0x3c02, 0x3c02) AM_MIRROR(0x00f4) AM_READNOP AM_DEVWRITE("aysnd", ay8910_device, data_w)
	AM_RANGE(0x3c03, 0x3c03) AM_MIRROR(0x00f6) AM_NOP
	AM_RANGE(0x3e00, 0x3e00) AM_MIRROR(0x00f4) AM_READNOP AM_DEVWRITE("tms", tms5220_device, data_w)
	AM_RANGE(0x3e02, 0x3e02) AM_MIRROR(0x00f4) AM_DEVREAD("tms", tms5220_device, status_r) AM_WRITENOP
	AM_RANGE(0x3e03, 0x3e03) AM_MIRROR(0x00f6) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( looping_sound_io_map, AS_IO, 8, looping_state )
	AM_RANGE(0x000, 0x000) AM_WRITE(looping_souint_clr)
	AM_RANGE(0x001, 0x007) AM_WRITE(looping_sound_sw)
	AM_RANGE(0x008, 0x008) AM_WRITE(ay_enable_w)
	AM_RANGE(0x009, 0x009) AM_WRITE(speech_enable_w)
	AM_RANGE(0x00a, 0x00a) AM_WRITE(ballon_enable_w)
	AM_RANGE(0x00b, 0x00f) AM_NOP
ADDRESS_MAP_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ STEP8(0,1), STEP8(64,1) },
	{ STEP8(0,8), STEP8(128,8) },
	8*8*4
};


static GFXDECODE_START( looping )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x2_planar, 0, 8 )
	GFXDECODE_ENTRY( "gfx1", 0, sprite_layout,    0, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( looping, looping_state )

	// CPU TMS9995, standard variant; no line connections
	MCFG_TMS99xx_ADD("maincpu", TMS9995, MAIN_CPU_CLOCK, looping_map, looping_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", looping_state,  looping_interrupt)

	// CPU TMS9980A for audio subsystem; no line connections
	MCFG_TMS99xx_ADD("audiocpu", TMS9980A,  SOUND_CLOCK/4, looping_sound_map, looping_sound_io_map)

	MCFG_CPU_ADD("mcu", COP420, COP_CLOCK)
	MCFG_COP400_CONFIG( COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false )
	MCFG_COP400_WRITE_L_CB(WRITE8(looping_state, cop_l_w))
	MCFG_COP400_READ_L_CB(READ8(looping_state, cop_unk_r))
	MCFG_COP400_READ_G_CB(READ8(looping_state, cop_unk_r))
	MCFG_COP400_READ_IN_CB(READ8(looping_state, cop_unk_r))
	MCFG_COP400_READ_SI_CB(READLINE(looping_state, cop_serial_r))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(looping_state, screen_update_looping)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", looping)

	MCFG_PALETTE_ADD("palette", 32)
	MCFG_PALETTE_INIT_OWNER(looping_state, looping)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, SOUND_CLOCK/4)
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_SOUND_ADD("tms", TMS5220, TMS_CLOCK)
	MCFG_TMS52XX_IRQ_HANDLER(WRITELINE(looping_state, looping_spcint))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( looping )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Accelerate?")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2") /* cocktail? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0e, 0x02, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      // Check code at 0x2c00
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

/* Same as 'looping' but additional "Infinite Lives" Dip Switch */
static INPUT_PORTS_START( skybump )
	PORT_INCLUDE(looping)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
//  PORT_DIPSETTING(    0x20, "Infinite (Cheat)")
INPUT_PORTS_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( loopingv )
	ROM_REGION( 0x8000, "maincpu", 0 ) /* TMS9995 code */
	ROM_LOAD( "vli3.5a",        0x0000, 0x2000, CRC(1ac3ccdf) SHA1(9d1cde8bd4d0f12eaf06225b3ecc4a5c3e4f0c11) )
	ROM_LOAD( "vli1.2a",        0x2000, 0x2000, CRC(97755fd4) SHA1(4a6ef02b0128cd516ff95083a7caaad8f3756f09) )
	ROM_LOAD( "l056-6.9a",      0x4000, 0x2000, CRC(548afa52) SHA1(0b88ac7394feede023519c585a4084591eb9661a) )
	ROM_LOAD( "vli9-5.8a",      0x6000, 0x2000, CRC(5d122f86) SHA1(d1c66b890142bb4d4648f3edec6567f58107dbf0) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) /* TMS9980 code */
	ROM_LOAD( "i-o.13c",        0x0000, 0x0800, CRC(21e9350c) SHA1(f30a180309e373a17569351944f5e7982c3b3f9d) )
	ROM_LOAD( "i-o.13a",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) )
	ROM_LOAD( "i-o.11a",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) ) /* speech */

	ROM_REGION( 0x1000, "mcu", 0 ) /* COP420 microcontroller code */
/*
    ROM_LOAD( "cop.bin",        0x0000, 0x0400, BAD_DUMP CRC(bbfd26d5) SHA1(5f78b32b6e7c003841ef5b635084db2cdfebf0e1) ) // overdumped 4 times, and starting PC is not 0
    ROM_CONTINUE(           0x0000, 0x0400)
    ROM_CONTINUE(           0x0000, 0x0400)
    ROM_CONTINUE(           0x0000, 0x0400)
*/
	ROM_LOAD( "cop.bin",        0x00c2, 0x033e, CRC(bbfd26d5) SHA1(5f78b32b6e7c003841ef5b635084db2cdfebf0e1) ) // overdumped 4 times and shifted
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "log2.8a",        0x0000, 0x800, CRC(ef3284ac) SHA1(8719c9df8c972a56c306b3c707aaa53092ffa2d6) )
	ROM_LOAD( "log1-9-3.6a",    0x0800, 0x800, CRC(c434c14c) SHA1(3669aaf7adc6b250378bcf62eb8e7058f55476ef) )

	ROM_REGION( 0x0020, "proms", 0 ) /* color prom */
	ROM_LOAD( "18s030.11b",     0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( loopingva )
	ROM_REGION( 0x8000, "maincpu", 0 ) /* TMS9995 code */
	ROM_LOAD( "vli3.5a",        0x0000, 0x2000, CRC(1ac3ccdf) SHA1(9d1cde8bd4d0f12eaf06225b3ecc4a5c3e4f0c11) )
	ROM_LOAD( "vli-4-3",        0x2000, 0x1000, CRC(f32cae2b) SHA1(2c6ef82af438e588b56fd58b95cf969c97bb9a66) )
	ROM_LOAD( "vli-8-4",        0x3000, 0x1000, CRC(611e1dbf) SHA1(0ab6669f1dec30c3f7bca49e158e4790a78fa308) )
	ROM_LOAD( "l056-6.9a",      0x4000, 0x2000, CRC(548afa52) SHA1(0b88ac7394feede023519c585a4084591eb9661a) )
	ROM_LOAD( "vli9-5.8a",      0x6000, 0x2000, CRC(5d122f86) SHA1(d1c66b890142bb4d4648f3edec6567f58107dbf0) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) /* TMS9980 code */
	ROM_LOAD( "i-o-v2.13c",     0x0000, 0x0800, CRC(09765ebe) SHA1(93b035c3a94f2f6d5e463256e26b600a4dd5d3ea) )
	ROM_LOAD( "i-o.13a",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) ) /* speech */
	ROM_LOAD( "i-o.11a",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* COP420 microcontroller code */
/*
    ROM_LOAD( "cop.bin",        0x0000, 0x0400, BAD_DUMP CRC(bbfd26d5) SHA1(5f78b32b6e7c003841ef5b635084db2cdfebf0e1) ) // overdumped 4 times, and starting PC is not 0
    ROM_CONTINUE(           0x0000, 0x0400)
    ROM_CONTINUE(           0x0000, 0x0400)
    ROM_CONTINUE(           0x0000, 0x0400)
*/
	ROM_LOAD( "cop.bin",        0x00c2, 0x033e, CRC(bbfd26d5) SHA1(5f78b32b6e7c003841ef5b635084db2cdfebf0e1) ) // overdumped 4 times and shifted
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "log2.8a",        0x0000, 0x800, CRC(ef3284ac) SHA1(8719c9df8c972a56c306b3c707aaa53092ffa2d6) )
	ROM_LOAD( "log1-9-3.6a",    0x0800, 0x800, CRC(c434c14c) SHA1(3669aaf7adc6b250378bcf62eb8e7058f55476ef) )

	ROM_REGION( 0x0020, "proms", 0 ) /* color prom */
	ROM_LOAD( "18s030.11b",     0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( looping )
	ROM_REGION( 0x8000, "maincpu", 0 ) /* TMS9995 code */
	ROM_LOAD( "loop551.bin",        0x0000, 0x1000, CRC(d6bb6db6) SHA1(074eb3bc101096bfe67c3107f306df829ae38548) )
	ROM_LOAD( "loop552.bin",        0x1000, 0x1000, CRC(bc32956d) SHA1(6ef8d76df1d5b1ed52a4057eae2bf4eb394e4c54) )
	ROM_LOAD( "loop553.bin",        0x2000, 0x1000, CRC(5f8b9aed) SHA1(32be61788e3d54b23d1663025365b1ab6b96dc91) )
	ROM_LOAD( "loop554b.bin",       0x3000, 0x1000, CRC(381a9625) SHA1(07d775125be1f761dad568f8ccce600414a9d15f) )
	ROM_LOAD( "loop555.bin",        0x4000, 0x1000, CRC(0ef4c922) SHA1(df6db0897a51aa10e106865a643588d866ef8c4e) )
	ROM_LOAD( "loop556.bin",        0x5000, 0x1000, CRC(3419a5d5) SHA1(2b0249c54985ab5e12de17c0e3d62caa0c7575e3) )
	ROM_LOAD( "loop557.bin",        0x6000, 0x1000, CRC(d430e287) SHA1(b0edd25ef4d2468cc1f8c10ac49c545a89d398d7) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) /* TMS9980 code */
	ROM_LOAD( "loopc13.bin",        0x0000, 0x1000, CRC(ff9ac4ec) SHA1(9f8df94cd79d86fe4c384df1d5d729b58a7ca7a8) )
	ROM_LOAD( "loopa13.bin",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) )
	ROM_LOAD( "loopa11.bin",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) ) /* speech */

	ROM_REGION( 0x1000, "mcu", 0 ) /* COP420 microcontroller code */
	/* taken from the other sets */
	ROM_LOAD( "cop.bin",        0x00c2, 0x033e, CRC(bbfd26d5) SHA1(5f78b32b6e7c003841ef5b635084db2cdfebf0e1) ) // overdumped 4 times and shifted
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "loopaa8.bin",        0x0000, 0x800, CRC(ef3284ac) SHA1(8719c9df8c972a56c306b3c707aaa53092ffa2d6) )
	ROM_LOAD( "loopaa6.bin",    0x0800, 0x800, CRC(c434c14c) SHA1(3669aaf7adc6b250378bcf62eb8e7058f55476ef) )

	ROM_REGION( 0x0020, "proms", 0 ) /* color prom */
	ROM_LOAD( "loopvp1.bin",        0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( skybump )
	ROM_REGION( 0x8000, "maincpu", 0 ) /* TMS9995 code */
	ROM_LOAD( "cpu.5a",         0x0000, 0x2000, CRC(dca38df0) SHA1(86abe04cbabf81399f842f53668fe7a3f7ed3757) )
	ROM_LOAD( "cpu.2a",         0x2000, 0x2000, CRC(6bcc211a) SHA1(245ebae3934df9c3920743a941546d96bb2e7c03) )
	ROM_LOAD( "cpu.9a",         0x4000, 0x2000, CRC(c7a50797) SHA1(60aa0a28ba970f12d0a0e538ae1c6807d105855c) )
	ROM_LOAD( "cpu.8a",         0x6000, 0x2000, CRC(a718c6f2) SHA1(19afa8c353829232cb96c27b87f13b43166ab6fc) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) /* TMS9980 code */
	ROM_LOAD( "snd.13c",        0x0000, 0x0800, CRC(21e9350c) SHA1(f30a180309e373a17569351944f5e7982c3b3f9d) )
	ROM_LOAD( "snd.13a",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) )
	ROM_LOAD( "snd.11a",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) )

	ROM_REGION( 0x1000, "mcu", 0 ) /* COP420 microcontroller code */
/*
    ROM_LOAD( "cop.bin",        0x0000, 0x0400, BAD_DUMP CRC(bbfd26d5) SHA1(5f78b32b6e7c003841ef5b635084db2cdfebf0e1) ) // overdumped 4 times, and starting PC is not 0
    ROM_CONTINUE(           0x0000, 0x0400)
    ROM_CONTINUE(           0x0000, 0x0400)
    ROM_CONTINUE(           0x0000, 0x0400)
*/
	ROM_LOAD( "cop.bin",        0x00c2, 0x033e, CRC(bbfd26d5) SHA1(5f78b32b6e7c003841ef5b635084db2cdfebf0e1) ) // overdumped 4 times and shifted
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)
	ROM_CONTINUE(           0x00c2, 0x033e)
	ROM_CONTINUE(           0x0000, 0x00c2)

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "vid.8a",         0x0000, 0x800, CRC(459ccc55) SHA1(747f6789605b48be9e22f779f9e3f6c98ad4e594) )
	ROM_LOAD( "vid.6a",         0x0800, 0x800, CRC(12ebbe74) SHA1(0f87c81a45d1bf3b8c6a70ee5e1a014069f67755) )

	ROM_REGION( 0x0020, "proms", 0 ) /* color prom */
	ROM_LOAD( "vid.clr",        0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END



/*************************************
 *
 *  Driver config
 *
 *************************************/

DRIVER_INIT_MEMBER(looping_state,looping)
{
	int length = memregion("maincpu")->bytes();
	UINT8 *rom = memregion("maincpu")->base();
	int i;

	m_cop_port_l = 0;

	/* bitswap the TMS9995 ROMs */
	for (i = 0; i < length; i++)
		rom[i] = BITSWAP8(rom[i], 0,1,2,3,4,5,6,7);

	/* install protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x7000, 0x7007, read8_delegate(FUNC(looping_state::protection_r), this));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, looping,   0,        looping, looping, looping_state, looping, ROT90, "Video Games GmbH", "Looping", MACHINE_IMPERFECT_SOUND /*| MACHINE_SUPPORTS_SAVE */)
GAME( 1982, loopingv,  looping,  looping, looping, looping_state, looping, ROT90, "Video Games GmbH (Venture Line license)", "Looping (Venture Line license, set 1)", MACHINE_IMPERFECT_SOUND /* | MACHINE_SUPPORTS_SAVE */)
GAME( 1982, loopingva, looping,  looping, looping, looping_state, looping, ROT90, "Video Games GmbH (Venture Line license)", "Looping (Venture Line license, set 2)", MACHINE_IMPERFECT_SOUND /* | MACHINE_SUPPORTS_SAVE */ )
GAME( 1982, skybump,   0,        looping, skybump, looping_state, looping, ROT90, "Venture Line", "Sky Bumper", MACHINE_IMPERFECT_SOUND /* | MACHINE_SUPPORTS_SAVE  */)
