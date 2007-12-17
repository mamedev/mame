/***************************************************************************

    Sigma Spiders hardware

    original driver by K.Wilkins May 1998
    updates by Zsolt Vasvari

    Games Supported:
        * Spiders (2 sets)
        * Spinner (bootleg)

    Known bugs:
        * None at this time


    Memory map information
    ----------------------

    Main CPU - Read Range

    $0000-$1bff Video Memory (bit0)
    $4000-$5bff Video Memory (bit1)
    $8000-$9bff Video Memory (bit2)
    $c000-$c001 6845 CRT Controller (crtc6845)
    $c020-$c027 NVRAM
    $c044-$c047 MC6821 PIA 1 (Control input port - all input)
    $c048-$c04b MC6821 PIA 2 (Sprite data port - see machine/spiders.c)
    $c050-$c053 MC6821 PIA 3 (Sound control - all output)
    $c060       Dip Switch 1
    $c080       Dip Switch 2
    $c0a0       Dip Switch 3
    $c100-ffff  ROM SPACE

    Main CPU - Write Range

    $0000-$1bff Video Memory (bit0)
    $4000-$5bff Video Memory (bit1)
    $8000-$9bff Video Memory (bit2)
    $c000-$c001 6845 CRT Controller (crtc6845)
    $c044-$c047 MC6821 PIA 1
    $c048-$c04b MC6821 PIA 2 (Video port)
    $c050-$c053 MC6821 PIA 3


    DIP SWITCH 1
    ------------

       1   2   3   4   5   6   7   8    COIN/CREDIT
       ON  ON  ON                       FREE PLAY
       ON  ON  OFF                      1/2
       ON  OFF ON                       1/3
       OFF ON  ON                       2/1
       ON  OFF OFF                      4/5
       OFF OFF OFF                      1/1

    DIP SWITCH 2
    ------------

       1   2   3   4   5   6   7   8
       ON  ON                           MODE A    A'
       ON  OFF                               A    B'
       OFF ON                                B    A'
       OFF OFF                               B    B'
               ON  ON                   14 # OF SPIDERS WHICH LAND TO
               ON  OFF                  20    COMPLETE SPIDER BELT
               OFF ON                   26
               OFF OFF                  16
                       ON               4  # 0F SPARE GUNS
                       OFF              3
                           ON   ON      NONE  SCORE FOR BONUS GUN
                           ON   OFF     20K
                           OFF  ON      25K
                           OFF  OFF     15K
                                   ON   GIANT SPIDER AFTER FIRST SCREEN
                                   OFF  GIANT SPIDER AFTER EVERY SCREEN

       PATTERN   1   2   3   4   5   6   7   8   9   10  11  12  13  14
       MODE A    27  36  45  54  63  72  81  98  45  54  63  72  81  98    PCS
       MODE B    20  27  34  41  48  55  62  69  34  41  48  55  62  69    PCS
       MODE A'   1   1   1   3.5 3.5 4   4.5 5   1   3.5 3.5 4   4.5 5     SECONDS
       MODE B'   .7  .7  .7  2   3   3.2 3.4 4   .7  2   3   2.3 3.4 4     SECONDS

       MODE A & B FOR THE NUMBER OF GROWABLE COCOONS
       MODE A' & B' FOR THE FREQUENCY OF SPIDERS APPEARANCE


    DIP SWITCH 3
    ------------

       1   2   3   4   5   6   7   8
       X                                VIDEO FLIP
           ON                           UPRIGHT
           OFF                          TABLE

       SWITCHES 3,4,5 FOR ADJUSTING PICTURE VERTICALLY
       SWITCHES 6,7,8 FOR ADJUSTING PICTURE HORIZONTALLY


    Unpopulated Switches
    --------------------

      PS1 (Display Crosshatch)         - Connected to PIA1 CB1 via pull-up
      PS2 (Coin input, bypass counter) - Connected to PIA1 PA1 via pull-up and invertor
      PS3 (Display coin counter)       - Connected to PIA1 PA2 via pull-up and invertor


    Graphic notes
    -------------
    Following roms appear to have graphic data

    * Mapped in main CPU space

    * SP1.BIN   - Appears to have some sprites in it.
    * SP2.BIN   - Appears to have some 16x16 sprites. (Includes the word SIGMA)
    * SP3.BIN   - Appears to have 2-4 sprites 16x16 - spiders
    * SP4.BIN   - CPU Code 6809 - Main
      SP5.BIN   - Some 8x8 and 16x16 tiles/sprites
      SP6.BIN   - Some 8x8 tiles
      SP7.BIN   - Tiles/Sprites 8x8
      SP8.BIN   - Tiles/Sprites 8x8
      SP9A.BIN  - Tiles/Sprites 8x8
      SP9B.BIN  - Tiles/Sprites 8x8
      SP10A.BIN - Tiles/Sprites 8x8
      SP10B.BIN - CPU Code 6802 - Sound

    Spiders has a fully bitmapped display and all sprite drawing is handled by the CPU
    hence no gfxdecode is done on the sprite ROMS, the CPU accesses them via PIA2.

    Screen is arranged in three memory areas with three bits being combined from each
    area to produce a 3bit colour send directly to the screen.

    $0000-$1bff, $4000-$5bff, $8000-$9bff   Bank 0
    $2000-$3bff, %6000-$7bff, $a000-$bbff   Bank 1

    The game normally runs from bank 0 only, but when lots of screen changes are required
    e.g spider or explosion then it implements a double buffered scheme with bank 1.

    The ram bank for screens is continuous from $0000-$bfff but is physically arranged
    as 3 banks of 16k (8x16k DRAM!). The CPU stack/variables etc are stored in the unused
    spaces between screens.


    CODE NOTES
    ----------

    6809 Data page = $1c00 (DP=1c)

    Known data page contents
    $05 - Dip switch 1 copy
    $06 - Dip switch 2 copy (inverted)
    $07 - Dip switch 3 copy
    $18 - Bonus Gun Score
    $1d - Spiders to complete belt after dipsw decode


    $c496 - Wait for vblank ($c04a bit 7 goes high)
    $f9cf - Clear screen (Bank0&1)
    $c8c6 - RAM test of sorts, called from IRQ handler?
    $de2f - Delay loop.
    $f9bb - Memory clearance routine
    $c761 - Partial DipSW decode
    $F987 - Addresses table at $f98d containing four structs:
                3c 0C 04 0D 80 (Inverted screen bank 0)
                34 0C 00 0D 00 (Normal screen   bank 0)
                3C 0C 40 0D 80 (Inverted screen bank 1)
                34 0C 44 0D 00 (Inverted screen bank 1)
                XX             Written to PIA2 Reg 3 - B control
                   XX XX       Written to CRTC addr/data
                         XX XX Written to CRTC addr/data
                These tables are used for frame flipping


        Video access port definition (On PIA 2)

        Bit 7 6 5 4 3 2 1 0
            X                Mode Setup/Read 1/0
                X X          Latch select (see below)
                    X X X X  Data nibble

        When in setup mode data is clocked into the latch by a read from port A
        When in read mode the read from port A auto increments the address.

        Latch 0 - Low byte low nibble
              1 - Low byte high nibble
              2 - High order low nibble
              3 - High order high nibble

***************************************************************************/

#include "driver.h"
#include "rescap.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "video/crtc6845.h"
#include "machine/6821pia.h"
#include "machine/74123.h"
#include "spiders.h"


#define MAIN_CPU_MASTER_CLOCK	(11200000)
#define PIXEL_CLOCK				(MAIN_CPU_MASTER_CLOCK / 2)
#define CRTC_CLOCK				(MAIN_CPU_MASTER_CLOCK / 16)


static UINT8 *spiders_ram;
static UINT8 flipscreen;
static UINT16 gfx_rom_address;
static UINT8 gfx_rom_ctrl_mode;
static UINT8 gfx_rom_ctrl_latch;
static UINT8 gfx_rom_ctrl_data;



/*************************************
 *
 *  Prototypes
 *
 *************************************/

static WRITE8_HANDLER( flipscreen_w );
static WRITE8_HANDLER( gfx_rom_intf_w );
static READ8_HANDLER( gfx_rom_r );



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

static void main_cpu_irq(int state)
{
	cpunum_set_input_line(0, M6809_IRQ_LINE,  state ? ASSERT_LINE : CLEAR_LINE);
}


static void main_cpu_firq(int state)
{
	cpunum_set_input_line(0, M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


static void audio_cpu_irq(int state)
{
	cpunum_set_input_line(1, M6802_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  PIA1 - Main CPU
 *
 *************************************/

static const pia6821_interface pia_1_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ input_port_0_r, input_port_1_r, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, 0, 0, 0,
	/*irqs   : A/B             */ main_cpu_irq, main_cpu_irq
};


static INTERRUPT_GEN( update_pia_1 )
{
	/* update the different PIA pins from the input ports */

	/* CA1 - copy of PA1 (COIN1) */
	pia_1_ca1_w(0, input_port_0_r(0) & 0x02);

	/* CA2 - copy of PA0 (SERVICE1) */
	pia_1_ca2_w(0, input_port_0_r(0) & 0x01);

	/* CB1 - (crosshatch) */
	pia_1_cb1_w(0, input_port_5_r(0));

	/* CB2 - NOT CONNECTED */
}



/*************************************
 *
 *  PIA2 - Main CPU
 *
 *************************************/

static const pia6821_interface pia_2_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ gfx_rom_r, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ 0, gfx_rom_intf_w, 0, flipscreen_w,
	/*irqs   : A/B             */ main_cpu_firq, main_cpu_irq
};



/*************************************
 *
 *  PIA3 - Main CPU
 *
 *************************************/

static const pia6821_interface pia_3_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ spiders_audio_ctrl_w, spiders_audio_command_w, 0, 0,
	/*irqs   : A/B             */ main_cpu_irq, main_cpu_irq
};



/*************************************
 *
 *  PIA -  Audio CPU
 *
 *************************************/

static const pia6821_interface pia_4_intf =
{
	/*inputs : A/B,CA/B1,CA/B2 */ 0, 0, 0, 0, 0, 0,
	/*outputs: A/B,CA/B2       */ spiders_audio_a_w, spiders_audio_b_w, 0, 0,
	/*irqs   : A/B             */ audio_cpu_irq, 0
};



/*************************************
 *
 *  IC60 - 74123
 *
 *  This timer is responsible for
 *  delaying the setting of PIA2's
 *  CA1 line.  This delay ensures that
 *  CA1 is only changed in the VBLANK
 *  region, but not in HBLANK
 *
 *************************************/

static void ic60_74123_output_changed(int output)
{
	pia_2_ca1_w(0, output);
}


static TTL74123_interface ic60_intf =
{
	TTL74123_GROUNDED,	/* the hook up type */
	RES_K(22),			/* resistor connected to RCext */
	CAP_U(0.01),		/* capacitor connected to Cext and RCext */
	1,					/* A pin - driven by the CRTC */
	1,					/* B pin - pulled high */
	1,					/* Clear pin - pulled high */
	ic60_74123_output_changed
};



/*************************************
 *
 *  Machine start
 *
 *************************************/

static MACHINE_START( spiders )
{
	pia_config(1, &pia_1_intf);
	pia_config(2, &pia_2_intf);
	pia_config(3, &pia_3_intf);
	pia_config(4, &pia_4_intf);

	TTL74123_config(0, &ic60_intf);

	/* setup for save states */
	state_save_register_global(flipscreen);
	state_save_register_global(gfx_rom_address);
	state_save_register_global(gfx_rom_ctrl_mode);
	state_save_register_global(gfx_rom_ctrl_latch);
	state_save_register_global(gfx_rom_ctrl_data);
}



/*************************************
 *
 *  Machine reset
 *
 *************************************/

static MACHINE_RESET( spiders )
{
	pia_reset();
	TTL74123_reset(0);
}



/*************************************
 *
 *  Video system
 *
 *************************************/

#define NUM_PENS	(8)


static WRITE8_HANDLER( flipscreen_w )
{
	flipscreen = data;
}


static void *begin_update(running_machine *machine, int screen,
						  mame_bitmap *bitmap, const rectangle *cliprect)
{
	/* create the pens */
	offs_t i;
	static pen_t pens[NUM_PENS];

	for (i = 0; i < NUM_PENS; i++)
	{
		pens[i] = MAKE_RGB(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
	}

	return pens;
}


static void update_row(mame_bitmap *bitmap, const rectangle *cliprect,
					   UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, void *param)
{
	UINT8 cx;

	pen_t *pens = (pen_t *)param;
	UINT8 x = 0;

	for (cx = 0; cx < x_count; cx++)
	{
		int i;
		UINT8 data1, data2, data3;

		/* the memory is hooked up to the MA, RA lines this way */
		offs_t offs = ((ma << 3) & 0x3f00) |
			   		  ((ra << 5) & 0x00e0) |
			   		  ((ma << 0) & 0x001f);

		if (flipscreen)
			offs = offs ^ 0x3fff;

		data1 = spiders_ram[0x0000 | offs];
		data2 = spiders_ram[0x4000 | offs];
		data3 = spiders_ram[0x8000 | offs];

		for (i = 0; i < 8; i++)
		{
			UINT8 color;

			if (flipscreen)
			{
				color = ((data3 & 0x80) >> 5) |
						((data2 & 0x80) >> 6) |
						((data1 & 0x80) >> 7);

				data1 = data1 << 1;
				data2 = data2 << 1;
				data3 = data3 << 1;
			}
			else
			{
				color = ((data3 & 0x01) << 2) |
						((data2 & 0x01) << 1) |
						((data1 & 0x01) << 0);

				data1 = data1 >> 1;
				data2 = data2 >> 1;
				data3 = data3 >> 1;
			}

			*BITMAP_ADDR32(bitmap, y, x) = pens[color];

			x = x + 1;
		}

		ma = ma + 1;
	}
}


static void display_enable_changed(int display_enabled)
{
	TTL74123_A_w(0, display_enabled);
}


static const crtc6845_interface crtc6845_intf =
{
	0,						/* screen we are acting on */
	CRTC_CLOCK, 			/* the clock (pin 21) of the chip */
	8,						/* number of pixels per video memory address */
	begin_update,			/* before pixel update callback */
	update_row,				/* row update callback */
	0,						/* after pixel update callback */
	display_enable_changed	/* call back for display state changes */
};


static VIDEO_START( spiders )
{
	/* configure the CRT controller */
	crtc6845_config(0, &crtc6845_intf);
}



/*************************************
 *
 *  Graphics ROM access - see the
 *  comments section for description
 *
 *************************************/

static WRITE8_HANDLER( gfx_rom_intf_w )
{
	gfx_rom_ctrl_mode  = ( data >> 7) & 0x01;
	gfx_rom_ctrl_latch = ( data >> 4) & 0x03;
	gfx_rom_ctrl_data  = (~data >> 0) & 0x0f;
}


static READ8_HANDLER( gfx_rom_r )
{
	UINT8 ret;

	if (gfx_rom_ctrl_mode)
	{
		UINT8 *rom = memory_region(REGION_GFX1);

		ret = rom[gfx_rom_address];

		gfx_rom_address = gfx_rom_address + 1;
	}
	else
	{
		UINT8 shift_count = gfx_rom_ctrl_latch << 2;
		gfx_rom_address = (gfx_rom_address & ~(0x0f << shift_count)) | (gfx_rom_ctrl_data << shift_count);

		ret = 0;
	}

	return ret;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( spiders_main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_RAM AM_BASE(&spiders_ram)
	AM_RANGE(0xc000, 0xc000) AM_WRITE(crtc6845_address_w)
	AM_RANGE(0xc001, 0xc001) AM_READWRITE(crtc6845_register_r, crtc6845_register_w)
	AM_RANGE(0xc020, 0xc027) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xc044, 0xc047) AM_READWRITE(pia_1_r, pia_1_w)
	AM_RANGE(0xc048, 0xc04b) AM_READWRITE(pia_2_alt_r, pia_2_alt_w)
	AM_RANGE(0xc050, 0xc053) AM_READWRITE(pia_3_r, pia_3_w)
	AM_RANGE(0xc060, 0xc060) AM_READ(input_port_2_r)
	AM_RANGE(0xc080, 0xc080) AM_READ(input_port_3_r)
	AM_RANGE(0xc0a0, 0xc0a0) AM_READ(input_port_4_r)
	AM_RANGE(0xc100, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( spiders_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0083) AM_READWRITE(pia_4_r, pia_4_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( spiders )
    /* PIA1 PA0 - PA7 */
    PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE2 )

	/* PIA1 PB0 - PB7 */
    PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  ) PORT_2WAY PORT_PLAYER(2)
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
    PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

    PORT_START  /* IN2, DSW1 */
    PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:1,2,3")
    PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x06, DEF_STR( 4C_5C ) )
    PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
    PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

    PORT_START  /* IN3, DSW2 */
    PORT_DIPNAME( 0x03, 0x03, "Play Mode" ) PORT_DIPLOCATION("SW2:1,2")
    PORT_DIPSETTING(    0x00, "A A'" )
    PORT_DIPSETTING(    0x01, "A B'" )
    PORT_DIPSETTING(    0x02, "B A'" )
    PORT_DIPSETTING(    0x03, "B B'" )
    PORT_DIPNAME( 0x0c, 0x0c, "Spiders to Complete Belt" ) PORT_DIPLOCATION("SW2:3,4")
    PORT_DIPSETTING(    0x00, "14" )
    PORT_DIPSETTING(    0x0c, "16" )
    PORT_DIPSETTING(    0x04, "20" )
    PORT_DIPSETTING(    0x08, "26" )
    PORT_DIPNAME( 0x10, 0x10, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5")
    PORT_DIPSETTING(    0x10, "3" )
    PORT_DIPSETTING(    0x00, "4" )
    PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:6,7")
    PORT_DIPSETTING(    0x00, DEF_STR( None ) )
    PORT_DIPSETTING(    0x60, "15K" )
    PORT_DIPSETTING(    0x20, "20K" )
    PORT_DIPSETTING(    0x40, "25K" )
    PORT_DIPNAME( 0x80, 0x00, "Giant Spiders" ) PORT_DIPLOCATION("SW2:8")
    PORT_DIPSETTING(    0x00, "First Screen" )
    PORT_DIPSETTING(    0x80, "Every Screen" )

    PORT_START  /* IN4, DSW3 */
    PORT_DIPNAME( 0x01, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
    PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x01, DEF_STR( On ) )
    PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW3:2")
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
    PORT_DIPNAME( 0x1c, 0x00, "Vertical Adjust" ) PORT_DIPLOCATION("SW3:3,4,5")
    PORT_DIPSETTING(    0x00, "0" )
    PORT_DIPSETTING(    0x04, "1" )
    PORT_DIPSETTING(    0x08, "2" )
    PORT_DIPSETTING(    0x0c, "3" )
    PORT_DIPSETTING(    0x10, "4" )
    PORT_DIPSETTING(    0x14, "5" )
    PORT_DIPSETTING(    0x18, "6" )
    PORT_DIPSETTING(    0x1c, "7" )
    PORT_DIPNAME( 0xe0, 0x00, "Horizontal Adjust" ) PORT_DIPLOCATION("SW3:6,7,8")
    PORT_DIPSETTING(    0x00, "0" )
    PORT_DIPSETTING(    0x20, "1" )
    PORT_DIPSETTING(    0x40, "2" )
    PORT_DIPSETTING(    0x60, "3" )
    PORT_DIPSETTING(    0x80, "4" )
    PORT_DIPSETTING(    0xa0, "5" )
    PORT_DIPSETTING(    0xc0, "6" )
    PORT_DIPSETTING(    0xe0, "7" )

    PORT_START      /* connected to PIA1 CB1 input */
    PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PS1 (Crosshatch)") PORT_CODE(KEYCODE_F1)

INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( spiders )

	/* basic machine hardware */
	MDRV_CPU_ADD(M6809, 2800000)
	MDRV_CPU_PROGRAM_MAP(spiders_main_map,0)
	MDRV_CPU_PERIODIC_INT(update_pia_1, 25)

	/* audio CPU */
	MDRV_CPU_ADD(M6802, 3000000)
	MDRV_CPU_PROGRAM_MAP(spiders_audio_map,0)

	MDRV_MACHINE_START(spiders)
	MDRV_MACHINE_RESET(spiders)
	MDRV_NVRAM_HANDLER(generic_0fill)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_VIDEO_START(spiders)
	MDRV_VIDEO_UPDATE(crtc6845)

	MDRV_SCREEN_ADD("main", 0)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 256, 0, 256, 256, 0, 256)	/* temporary, CRTC will configure screen */

	/* audio hardware */
	MDRV_IMPORT_FROM(spiders_audio)

MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( spiders )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sp-ic74",      0xc000, 0x1000, CRC(6a2578f6) SHA1(ddfe4fb2ccc925df7ae97821f8681b32e47630b4) )
	ROM_LOAD( "sp-ic73",      0xd000, 0x1000, CRC(d69b2f21) SHA1(ea2b07d19bd50c3b57da8fd8e13b8ab0e8ca3084) )
	ROM_LOAD( "sp-ic72",      0xe000, 0x1000, CRC(464125da) SHA1(94e9edd52e8bd72bbb5dc91b0aa11955e940799c) )
	ROM_LOAD( "sp-ic71",      0xf000, 0x1000, CRC(a9539b18) SHA1(2d02343a78a4a65e5a1798552cd015f16ad5423a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "sp-ic3",       0xf800, 0x0800, CRC(944d761e) SHA1(23b1f9234e0de678e96d1a6876d8d0a341150385) )

	ROM_REGION( 0x10000, REGION_GFX1, 0 )
	ROM_LOAD( "sp-ic33",      0x0000, 0x1000, CRC(b6731baa) SHA1(b551030df417b40f4a8488fc82a8b5809d3d32f6) )
	ROM_LOAD( "sp-ic25",      0x1000, 0x1000, CRC(baec64e7) SHA1(beb45e2e6270607c14cdf964c08fe320ce8236a0) )
	ROM_LOAD( "sp-ic24",      0x2000, 0x1000, CRC(a40a5517) SHA1(3f524c7dbbfe8aad7860d15c38d2702732895681) )
	ROM_LOAD( "sp-ic23",      0x3000, 0x1000, CRC(3ca08053) SHA1(20c5709d9650c426b91aed5318a9ab0a10009f17) )
	ROM_LOAD( "sp-ic22",      0x4000, 0x1000, CRC(07ea073c) SHA1(2e57831092730db5fbdb97c2d78d8842868906f4) )
	ROM_LOAD( "sp-ic21",      0x5000, 0x1000, CRC(41b344b4) SHA1(c0eac1e332da1eada062059ae742b666051da76c) )
	ROM_LOAD( "sp-ic20",      0x6000, 0x1000, CRC(4d37da5a) SHA1(37567d19596506385e9dcc7a7c0cf65120189ae0) )
ROM_END


ROM_START( spiders2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sp-ic74",      0xc000, 0x1000, CRC(6a2578f6) SHA1(ddfe4fb2ccc925df7ae97821f8681b32e47630b4) )
	ROM_LOAD( "sp2.bin",      0xd000, 0x1000, CRC(cf71d12b) SHA1(369e91f637e8cd898354ddee04e24d4894968f79) )
	ROM_LOAD( "sp-ic72",      0xe000, 0x1000, CRC(464125da) SHA1(94e9edd52e8bd72bbb5dc91b0aa11955e940799c) )
	ROM_LOAD( "sp4.bin",      0xf000, 0x1000, CRC(f3d126bb) SHA1(ecc9156a7da661fa7543d7656aa7da77274e0842) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "sp-ic3",       0xf800, 0x0800, CRC(944d761e) SHA1(23b1f9234e0de678e96d1a6876d8d0a341150385) )

	ROM_REGION( 0x10000, REGION_GFX1, 0 )
	ROM_LOAD( "sp-ic33",      0x0000, 0x1000, CRC(b6731baa) SHA1(b551030df417b40f4a8488fc82a8b5809d3d32f6) )
	ROM_LOAD( "sp-ic25",      0x1000, 0x1000, CRC(baec64e7) SHA1(beb45e2e6270607c14cdf964c08fe320ce8236a0) )
	ROM_LOAD( "sp-ic24",      0x2000, 0x1000, CRC(a40a5517) SHA1(3f524c7dbbfe8aad7860d15c38d2702732895681) )
	ROM_LOAD( "sp-ic23",      0x3000, 0x1000, CRC(3ca08053) SHA1(20c5709d9650c426b91aed5318a9ab0a10009f17) )
	ROM_LOAD( "sp-ic22",      0x4000, 0x1000, CRC(07ea073c) SHA1(2e57831092730db5fbdb97c2d78d8842868906f4) )
	ROM_LOAD( "sp-ic21",      0x5000, 0x1000, CRC(41b344b4) SHA1(c0eac1e332da1eada062059ae742b666051da76c) )
	ROM_LOAD( "sp-ic20",      0x6000, 0x1000, CRC(4d37da5a) SHA1(37567d19596506385e9dcc7a7c0cf65120189ae0) )
ROM_END


ROM_START( spinner )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "sp01-100.r1",  0xc000, 0x1000, CRC(e85faa36) SHA1(ef1c479d503ef6a833ae1f9d5a260f9e50c1f2d4) )
	ROM_LOAD( "sp02-99.s1",   0xd000, 0x1000, CRC(4bcd2b35) SHA1(dff3c6e68cc5384863a123661422d929e7406dee) )
	ROM_LOAD( "sp03-98.t1",   0xe000, 0x1000, CRC(fdabc5df) SHA1(a3276eb1f09f6a3c406721f89993a39c92fb7728) )
	ROM_LOAD( "sp04-97.v1",   0xf000, 0x1000, CRC(62798f96) SHA1(1407a2ccb2b8f998f2ee494f52a471b627895dbe) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "sp41-28.d9",   0xf800, 0x0800, CRC(944d761e) SHA1(23b1f9234e0de678e96d1a6876d8d0a341150385) )

	ROM_REGION( 0x10000, REGION_GFX1, 0 )
	ROM_LOAD( "sp05-25.k8",   0x0000, 0x1000, CRC(ccc696ee) SHA1(1d41e9eb0cae73b221327d7b6e02450275d056c6) )
	ROM_LOAD( "sp06-17.k9",   0x1000, 0x1000, CRC(d3d06722) SHA1(da510ed162e5c310945123c9ce6d5648c7b0ae48) )
	ROM_LOAD( "sp07-16.l9",   0x2000, 0x1000, CRC(a40a5517) SHA1(3f524c7dbbfe8aad7860d15c38d2702732895681) )
	ROM_LOAD( "sp08-15.n9",   0x3000, 0x1000, CRC(3ca08053) SHA1(20c5709d9650c426b91aed5318a9ab0a10009f17) )
	ROM_LOAD( "sp09-14.o9",   0x4000, 0x1000, CRC(07ea073c) SHA1(2e57831092730db5fbdb97c2d78d8842868906f4) )
	ROM_LOAD( "sp10-13.q9",   0x5000, 0x1000, CRC(41b344b4) SHA1(c0eac1e332da1eada062059ae742b666051da76c) )
	ROM_LOAD( "sp11-12.r9",   0x6000, 0x1000, CRC(4d37da5a) SHA1(37567d19596506385e9dcc7a7c0cf65120189ae0) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* this is a newer version with just one bug fix */
GAME( 1981, spiders,  0,       spiders, spiders, 0, ROT270, "Sigma Enterprises Inc.", "Spiders (set 1)", GAME_SUPPORTS_SAVE)
GAME( 1981, spiders2, spiders, spiders, spiders, 0, ROT270, "Sigma Enterprises Inc.", "Spiders (set 2)", GAME_SUPPORTS_SAVE)
GAME( 1981, spinner,  spiders, spiders, spiders, 0, ROT270, "bootleg",				  "Spinner", GAME_SUPPORTS_SAVE)
