// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
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
    $c000-$c001 6845 CRT Controller (mc6845)
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
    $c000-$c001 6845 CRT Controller (mc6845)
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

#include "emu.h"
#include "machine/rescap.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "machine/6821pia.h"
#include "machine/74123.h"
#include "includes/spiders.h"
#include "machine/nvram.h"


#define MAIN_CPU_MASTER_CLOCK   (11200000)
#define PIXEL_CLOCK             (MAIN_CPU_MASTER_CLOCK / 2)
#define CRTC_CLOCK              (MAIN_CPU_MASTER_CLOCK / 16)


/*************************************
 *
 *  Prototypes
 *
 *************************************/







/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

WRITE_LINE_MEMBER(spiders_state::main_cpu_irq)
{
	pia6821_device *pia1 = machine().device<pia6821_device>("pia1");
	pia6821_device *pia2 = machine().device<pia6821_device>("pia2");
	pia6821_device *pia3 = machine().device<pia6821_device>("pia3");
	int combined_state = pia1->irq_a_state() | pia1->irq_b_state() |
													pia2->irq_b_state() |
							pia3->irq_a_state() | pia3->irq_b_state();

	m_maincpu->set_input_line(M6809_IRQ_LINE, combined_state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(spiders_state::main_cpu_firq)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(spiders_state::audio_cpu_irq)
{
	m_audiocpu->set_input_line(M6800_IRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  PIA1 - Main CPU
 *
 *************************************/

INTERRUPT_GEN_MEMBER(spiders_state::update_pia_1)
{
	pia6821_device *pia1 = machine().device<pia6821_device>("pia1");
	/* update the different PIA pins from the input ports */

	/* CA1 - copy of PA1 (COIN1) */
	pia1->ca1_w(ioport("IN0")->read() & 0x02);

	/* CA2 - copy of PA0 (SERVICE1) */
	pia1->ca2_w(ioport("IN0")->read() & 0x01);

	/* CB1 - (crosshatch) */
	pia1->cb1_w(ioport("XHATCH")->read());

	/* CB2 - NOT CONNECTED */
}



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

WRITE8_MEMBER(spiders_state::ic60_74123_output_changed)
{
	pia6821_device *pia2 = machine().device<pia6821_device>("pia2");
	pia2->ca1_w(data);
}

/*************************************
 *
 *  Machine start
 *
 *************************************/

void spiders_state::machine_start()
{
	/* setup for save states */
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_gfx_rom_address));
	save_item(NAME(m_gfx_rom_ctrl_mode));
	save_item(NAME(m_gfx_rom_ctrl_latch));
	save_item(NAME(m_gfx_rom_ctrl_data));
}



/*************************************
 *
 *  Video system
 *
 *************************************/


WRITE_LINE_MEMBER(spiders_state::flipscreen_w)
{
	m_flipscreen = state;
}


MC6845_UPDATE_ROW( spiders_state::crtc_update_row )
{
	UINT8 x = 0;

	for (UINT8 cx = 0; cx < x_count; cx++)
	{
		UINT8 data1, data2, data3;

		/* the memory is hooked up to the MA, RA lines this way */
		offs_t offs = ((ma << 3) & 0x3f00) |
						((ra << 5) & 0x00e0) |
						((ma << 0) & 0x001f);

		if (m_flipscreen)
			offs = offs ^ 0x3fff;

		data1 = m_ram[0x0000 | offs];
		data2 = m_ram[0x4000 | offs];
		data3 = m_ram[0x8000 | offs];

		for (int i = 0; i < 8; i++)
		{
			UINT8 color;

			if (m_flipscreen)
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

			bitmap.pix32(y, x) = m_palette->pen_color(color);

			x = x + 1;
		}

		ma = ma + 1;
	}
}


WRITE_LINE_MEMBER(spiders_state::display_enable_changed)
{
	machine().device<ttl74123_device>("ic60")->a_w(generic_space(), 0, state);
}


/*************************************
 *
 *  Graphics ROM access - see the
 *  comments section for description
 *
 *************************************/

WRITE8_MEMBER(spiders_state::gfx_rom_intf_w)
{
	m_gfx_rom_ctrl_mode  = ( data >> 7) & 0x01;
	m_gfx_rom_ctrl_latch = ( data >> 4) & 0x03;
	m_gfx_rom_ctrl_data  = (~data >> 0) & 0x0f;
}


READ8_MEMBER(spiders_state::gfx_rom_r)
{
	UINT8 ret;

	if (m_gfx_rom_ctrl_mode)
	{
		UINT8 *rom = memregion("gfx1")->base();

		ret = rom[m_gfx_rom_address];

		m_gfx_rom_address = m_gfx_rom_address + 1;
	}
	else
	{
		UINT8 shift_count = m_gfx_rom_ctrl_latch << 2;
		m_gfx_rom_address = (m_gfx_rom_address & ~(0x0f << shift_count)) | (m_gfx_rom_ctrl_data << shift_count);

		ret = 0;
	}

	return ret;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( spiders_main_map, AS_PROGRAM, 8, spiders_state )
	AM_RANGE(0x0000, 0xbfff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xc000, 0xc000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xc001, 0xc001) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0xc020, 0xc027) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xc044, 0xc047) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0xc048, 0xc04b) AM_DEVREADWRITE("pia2", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xc050, 0xc053) AM_DEVREADWRITE("pia3", pia6821_device, read, write)
	AM_RANGE(0xc060, 0xc060) AM_READ_PORT("DSW1")
	AM_RANGE(0xc080, 0xc080) AM_READ_PORT("DSW2")
	AM_RANGE(0xc0a0, 0xc0a0) AM_READ_PORT("DSW3")
	AM_RANGE(0xc100, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( spiders_audio_map, AS_PROGRAM, 8, spiders_state )
	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0083) AM_DEVREADWRITE("pia4", pia6821_device, read, write)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( spiders )
	/* PIA1 PA0 - PA7 */
	PORT_START("IN0")   /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE2 )

	/* PIA1 PB0 - PB7 */
	PORT_START("IN1")   /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT  ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW1")  /* IN2, DSW1 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")  /* IN3, DSW2 */
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

	PORT_START("DSW3")  /* IN4, DSW3 */
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

	PORT_START("XHATCH")    /* connected to PIA1 CB1 input */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PS1 (Crosshatch)") PORT_CODE(KEYCODE_F1)

INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( spiders, spiders_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 2800000)
	MCFG_CPU_PROGRAM_MAP(spiders_main_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(spiders_state, update_pia_1,  25)

	MCFG_CPU_ADD("audiocpu", M6802, 3000000)
	MCFG_CPU_PROGRAM_MAP(spiders_audio_map)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 256, 0, 256, 256, 0, 256)   /* temporary, CRTC will configure screen */
	MCFG_SCREEN_UPDATE_DEVICE("crtc", mc6845_device, screen_update)

	MCFG_PALETTE_ADD_3BIT_RGB("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(spiders_state, crtc_update_row)
	MCFG_MC6845_OUT_DE_CB(WRITELINE(spiders_state, display_enable_changed))

	/* 74LS123 */

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(IOPORT("IN0"))
	MCFG_PIA_READPB_HANDLER(IOPORT("IN1"))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(spiders_state,main_cpu_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(spiders_state,main_cpu_irq))

	MCFG_DEVICE_ADD("pia2", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(spiders_state,gfx_rom_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(spiders_state,gfx_rom_intf_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(spiders_state,flipscreen_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(spiders_state,main_cpu_firq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(spiders_state,main_cpu_irq))

	MCFG_DEVICE_ADD("pia3", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(spiders_state, spiders_audio_ctrl_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(spiders_state, spiders_audio_command_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(spiders_state,main_cpu_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(spiders_state,main_cpu_irq))

	MCFG_DEVICE_ADD("pia4", PIA6821, 0)
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(spiders_state, spiders_audio_a_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(spiders_state, spiders_audio_b_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(spiders_state, audio_cpu_irq))

	MCFG_DEVICE_ADD("ic60", TTL74123, 0)
	MCFG_TTL74123_CONNECTION_TYPE(TTL74123_GROUNDED)    /* the hook up type */
	MCFG_TTL74123_RESISTOR_VALUE(RES_K(22))               /* resistor connected to RCext */
	MCFG_TTL74123_CAPACITOR_VALUE(CAP_U(0.01))               /* capacitor connected to Cext and RCext */
	MCFG_TTL74123_A_PIN_VALUE(1)                  /* A pin - driven by the CRTC */
	MCFG_TTL74123_B_PIN_VALUE(1)                  /* B pin - pulled high */
	MCFG_TTL74123_CLEAR_PIN_VALUE(1)                  /* Clear pin - pulled high */
	MCFG_TTL74123_OUTPUT_CHANGED_CB(WRITE8(spiders_state, ic60_74123_output_changed))

	/* audio hardware */
	MCFG_FRAGMENT_ADD(spiders_audio)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( spiders )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp-ic74",      0xc000, 0x1000, CRC(6a2578f6) SHA1(ddfe4fb2ccc925df7ae97821f8681b32e47630b4) )
	ROM_LOAD( "sp-ic73",      0xd000, 0x1000, CRC(d69b2f21) SHA1(ea2b07d19bd50c3b57da8fd8e13b8ab0e8ca3084) )
	ROM_LOAD( "sp-ic72",      0xe000, 0x1000, CRC(464125da) SHA1(94e9edd52e8bd72bbb5dc91b0aa11955e940799c) )
	ROM_LOAD( "sp-ic71",      0xf000, 0x1000, CRC(a9539b18) SHA1(2d02343a78a4a65e5a1798552cd015f16ad5423a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sp-ic3",       0xf800, 0x0800, CRC(944d761e) SHA1(23b1f9234e0de678e96d1a6876d8d0a341150385) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sp-ic33",      0x0000, 0x1000, CRC(b6731baa) SHA1(b551030df417b40f4a8488fc82a8b5809d3d32f6) )
	ROM_LOAD( "sp-ic25",      0x1000, 0x1000, CRC(baec64e7) SHA1(beb45e2e6270607c14cdf964c08fe320ce8236a0) )
	ROM_LOAD( "sp-ic24",      0x2000, 0x1000, CRC(a40a5517) SHA1(3f524c7dbbfe8aad7860d15c38d2702732895681) )
	ROM_LOAD( "sp-ic23",      0x3000, 0x1000, CRC(3ca08053) SHA1(20c5709d9650c426b91aed5318a9ab0a10009f17) )
	ROM_LOAD( "sp-ic22",      0x4000, 0x1000, CRC(07ea073c) SHA1(2e57831092730db5fbdb97c2d78d8842868906f4) )
	ROM_LOAD( "sp-ic21",      0x5000, 0x1000, CRC(41b344b4) SHA1(c0eac1e332da1eada062059ae742b666051da76c) )
	ROM_LOAD( "sp-ic20",      0x6000, 0x1000, CRC(4d37da5a) SHA1(37567d19596506385e9dcc7a7c0cf65120189ae0) )
ROM_END


ROM_START( spiders2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp-ic74",      0xc000, 0x1000, CRC(6a2578f6) SHA1(ddfe4fb2ccc925df7ae97821f8681b32e47630b4) )
	ROM_LOAD( "sp2.bin",      0xd000, 0x1000, CRC(cf71d12b) SHA1(369e91f637e8cd898354ddee04e24d4894968f79) )
	ROM_LOAD( "sp-ic72",      0xe000, 0x1000, CRC(464125da) SHA1(94e9edd52e8bd72bbb5dc91b0aa11955e940799c) )
	ROM_LOAD( "sp4.bin",      0xf000, 0x1000, CRC(f3d126bb) SHA1(ecc9156a7da661fa7543d7656aa7da77274e0842) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sp-ic3",       0xf800, 0x0800, CRC(944d761e) SHA1(23b1f9234e0de678e96d1a6876d8d0a341150385) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sp-ic33",      0x0000, 0x1000, CRC(b6731baa) SHA1(b551030df417b40f4a8488fc82a8b5809d3d32f6) )
	ROM_LOAD( "sp-ic25",      0x1000, 0x1000, CRC(baec64e7) SHA1(beb45e2e6270607c14cdf964c08fe320ce8236a0) )
	ROM_LOAD( "sp-ic24",      0x2000, 0x1000, CRC(a40a5517) SHA1(3f524c7dbbfe8aad7860d15c38d2702732895681) )
	ROM_LOAD( "sp-ic23",      0x3000, 0x1000, CRC(3ca08053) SHA1(20c5709d9650c426b91aed5318a9ab0a10009f17) )
	ROM_LOAD( "sp-ic22",      0x4000, 0x1000, CRC(07ea073c) SHA1(2e57831092730db5fbdb97c2d78d8842868906f4) )
	ROM_LOAD( "sp-ic21",      0x5000, 0x1000, CRC(41b344b4) SHA1(c0eac1e332da1eada062059ae742b666051da76c) )
	ROM_LOAD( "sp-ic20",      0x6000, 0x1000, CRC(4d37da5a) SHA1(37567d19596506385e9dcc7a7c0cf65120189ae0) )
ROM_END


ROM_START( spinner )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp01-100.r1",  0xc000, 0x1000, CRC(e85faa36) SHA1(ef1c479d503ef6a833ae1f9d5a260f9e50c1f2d4) )
	ROM_LOAD( "sp02-99.s1",   0xd000, 0x1000, CRC(4bcd2b35) SHA1(dff3c6e68cc5384863a123661422d929e7406dee) )
	ROM_LOAD( "sp03-98.t1",   0xe000, 0x1000, CRC(fdabc5df) SHA1(a3276eb1f09f6a3c406721f89993a39c92fb7728) )
	ROM_LOAD( "sp04-97.v1",   0xf000, 0x1000, CRC(62798f96) SHA1(1407a2ccb2b8f998f2ee494f52a471b627895dbe) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sp41-28.d9",   0xf800, 0x0800, CRC(944d761e) SHA1(23b1f9234e0de678e96d1a6876d8d0a341150385) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sp05-25.k8",   0x0000, 0x1000, CRC(ccc696ee) SHA1(1d41e9eb0cae73b221327d7b6e02450275d056c6) )
	ROM_LOAD( "sp06-17.k9",   0x1000, 0x1000, CRC(d3d06722) SHA1(da510ed162e5c310945123c9ce6d5648c7b0ae48) )
	ROM_LOAD( "sp07-16.l9",   0x2000, 0x1000, CRC(a40a5517) SHA1(3f524c7dbbfe8aad7860d15c38d2702732895681) )
	ROM_LOAD( "sp08-15.n9",   0x3000, 0x1000, CRC(3ca08053) SHA1(20c5709d9650c426b91aed5318a9ab0a10009f17) )
	ROM_LOAD( "sp09-14.o9",   0x4000, 0x1000, CRC(07ea073c) SHA1(2e57831092730db5fbdb97c2d78d8842868906f4) )
	ROM_LOAD( "sp10-13.q9",   0x5000, 0x1000, CRC(41b344b4) SHA1(c0eac1e332da1eada062059ae742b666051da76c) )
	ROM_LOAD( "sp11-12.r9",   0x6000, 0x1000, CRC(4d37da5a) SHA1(37567d19596506385e9dcc7a7c0cf65120189ae0) )
ROM_END

ROM_START( spiders3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sp-ic74.bin",   0xc000, 0x1000, CRC(ed6b5dcd) SHA1(551fecb9adbab5cb5ac07fb27cc1fcfa03b5e832) )
	ROM_LOAD( "sp-ic73.bin",   0xd000, 0x1000, CRC(bc04f779) SHA1(a6366461a120d978b27470dad34a2270aee71428) )
	ROM_LOAD( "sp-ic72.bin",   0xe000, 0x1000, CRC(c7dd097d) SHA1(c7802ac33c1a67ffcb1891e74f0e43483f5b1016) )
	ROM_LOAD( "sp-ic71.bin",   0xf000, 0x1000, CRC(593b43c4) SHA1(f0cf06a22be205f00a9d52f766c3aae1985f0090) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sp-ic3.bin",   0xf800, 0x0800, CRC(153adee7) SHA1(45dbd756adb5d75562b066fc152c0af7925052de) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sp-ic33.bin",  0x0000, 0x1000, CRC(7209dacd) SHA1(142de81c013f4074a8eaedc5cd7e9498e6910eb9) )
	ROM_LOAD( "sp-ic25.bin",  0x1000, 0x1000, CRC(d3d06722) SHA1(da510ed162e5c310945123c9ce6d5648c7b0ae48) )
	ROM_LOAD( "sp-ic24",      0x2000, 0x1000, CRC(a40a5517) SHA1(3f524c7dbbfe8aad7860d15c38d2702732895681) )
	ROM_LOAD( "sp-ic23",      0x3000, 0x1000, CRC(3ca08053) SHA1(20c5709d9650c426b91aed5318a9ab0a10009f17) )
	ROM_LOAD( "sp-ic22",      0x4000, 0x1000, CRC(07ea073c) SHA1(2e57831092730db5fbdb97c2d78d8842868906f4) )
	ROM_LOAD( "sp-ic21",      0x5000, 0x1000, CRC(41b344b4) SHA1(c0eac1e332da1eada062059ae742b666051da76c) )
	ROM_LOAD( "sp-ic20",      0x6000, 0x1000, CRC(4d37da5a) SHA1(37567d19596506385e9dcc7a7c0cf65120189ae0) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

/* this is a newer version with just one bug fix */
GAME( 1981, spiders,  0,       spiders, spiders, driver_device, 0, ROT270, "Sigma Enterprises Inc.", "Spiders (set 1)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE)
GAME( 1981, spiders2, spiders, spiders, spiders, driver_device, 0, ROT270, "Sigma Enterprises Inc.", "Spiders (set 2)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE)
GAME( 1981, spiders3, spiders, spiders, spiders, driver_device, 0, ROT270, "Sigma Enterprises Inc.", "Spiders (set 3)", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE)
GAME( 1981, spinner,  spiders, spiders, spiders, driver_device, 0, ROT270, "bootleg",                 "Spinner", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE)
