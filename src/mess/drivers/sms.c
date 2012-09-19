/******************************************************************************
 Contributors:

    Marat Fayzullin (MG source)
    Charles MacDonald
    Mathis Rosenhauer
    Brad Oliver
    Michael Luong

 To do:

 - SIO interface for Game Gear (needs netplay, I guess)
 - SMS lightgun support
 - LCD persistence emulation for GG
 - SMS 3D glass support

 The Game Gear SIO hardware is not emulated but has some
 placeholders in 'machine/sms.c'

 Changes:
    Apr 02 - Added raster palette effects for SMS & GG (ML)
                 - Added sprite collision (ML)
                 - Added zoomed sprites (ML)
    May 02 - Fixed paging bug (ML)
                 - Fixed sprite and tile priority bug (ML)
                 - Fixed bug #66 (ML)
                 - Fixed bug #78 (ML)
                 - try to implement LCD persistence emulation for GG (ML)
    Jun 10, 02 - Added bios emulation (ML)
    Jun 12, 02 - Added PAL & NTSC systems (ML)
    Jun 25, 02 - Added border emulation (ML)
    Jun 27, 02 - Version bits for Game Gear (bits 6 of port 00) (ML)
    Nov-Dec, 05 - Numerous cleanups, fixes, updates (WP)
    Mar, 07 - More cleanups, fixes, mapper additions, etc (WP)

SMS Store Unit memory map for the second CPU:

0000-3FFF - BIOS
4000-47FF - RAM
8000      - System Control Register (R/W)
            Reading:
            bit7      - ready (0 = ready, 1 = not ready)
            bit6-bit5 - unknown
            bit4-bit3 - timer selection bit switches
            bit2-bit0 - unknown
            Writing:
            bit7-bit4 - unknown, maybe led of selected game to set?
            bit3      - unknown, 1 seems to be written all the time
            bit2      - unknown, 1 seems to be written all the time
            bit1      - reset signal for sms cpu, 0 = reset low, 1 = reset high
            bit0      - which cpu receives interrupt signals, 0 = sms cpu, 1 = controlling cpu
C000      - Card/Cartridge selction register (W)
            bit7-bit4 - slot to select
            bit3      - slot type (0 = cartridge, 1 = card ?)
            bit2-bit0 - unknown
C400      - ???? (used once)
D800      - Selection buttons #1, 1-8 (R)
DC00      - Selection buttons #2, 9-16 (R)

 ******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/2413intf.h"
#include "video/315_5124.h"
#include "imagedev/cartslot.h"
#include "machine/eeprom.h"
#include "includes/sms.h"

#include "sms1.lh"

#define MASTER_CLOCK_PAL	53203400	/* This might be a tiny bit too low */


static ADDRESS_MAP_START( sms1_mem, AS_PROGRAM, 8, sms_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROMBANK("bank1")					/* First 0x0400 part always points to first page */
	AM_RANGE(0x0400, 0x1fff) AM_ROMBANK("bank2")					/* switchable rom bank */
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank7")					/* switchable rom bank */
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank3")					/* switchable rom bank */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank4")					/* switchable rom bank */
	AM_RANGE(0x8000, 0x9fff) AM_READ_BANK("bank5") AM_WRITE(sms_cartram_w)	/* ROM bank / on-cart RAM */
	AM_RANGE(0xa000, 0xbfff) AM_READ_BANK("bank6") AM_WRITE(sms_cartram2_w)	/* ROM bank / on-cart RAM */
	AM_RANGE(0xc000, 0xdff7) AM_MIRROR(0x2000) AM_RAM			/* RAM (mirror at 0xE000) */
	AM_RANGE(0xdff8, 0xdfff) AM_RAM						/* RAM "underneath" frame registers */
	AM_RANGE(0xfff8, 0xfffb) AM_READWRITE(sms_sscope_r, sms_sscope_w)	/* 3-D glasses */
	AM_RANGE(0xfffc, 0xffff) AM_READWRITE(sms_mapper_r, sms_mapper_w)	/* Bankswitch control */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_mem, AS_PROGRAM, 8, sms_state )
	AM_RANGE(0x0000, 0x03ff) AM_ROMBANK("bank1")					/* First 0x0400 part always points to first page */
	AM_RANGE(0x0400, 0x1fff) AM_ROMBANK("bank2")					/* switchable rom bank */
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank7")					/* switchable rom bank */
	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank3")					/* switchable rom bank */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank4")					/* switchable rom bank */
	AM_RANGE(0x8000, 0x9fff) AM_READ_BANK("bank5") AM_WRITE(sms_cartram_w)	/* ROM bank / on-cart RAM */
	AM_RANGE(0xa000, 0xbfff) AM_READ_BANK("bank6") AM_WRITE(sms_cartram2_w)	/* ROM bank / on-cart RAM */
	AM_RANGE(0xc000, 0xdffb) AM_MIRROR(0x2000) AM_RAM			/* RAM (mirror at 0xE000) */
	AM_RANGE(0xdffc, 0xdfff) AM_RAM						/* RAM "underneath" frame registers */
	AM_RANGE(0xfffc, 0xffff) AM_READWRITE(sms_mapper_r, sms_mapper_w)	/* Bankswitch control */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_store_mem, AS_PROGRAM, 8, sms_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM						/* BIOS */
	AM_RANGE(0x4000, 0x47ff) AM_RAM						/* RAM */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank10")					/* Cartridge/card peek area */
	AM_RANGE(0x8000, 0x8000) AM_READWRITE(sms_store_control_r, sms_store_control_w)	/* Control */
	AM_RANGE(0xc000, 0xc000) AM_READWRITE(sms_store_cart_select_r, sms_store_cart_select_w)	/* cartridge/card slot selector */
	AM_RANGE(0xd800, 0xd800) AM_READ(sms_store_select1)			/* Game selector port #1 */
	AM_RANGE(0xdc00, 0xdc00) AM_READ(sms_store_select2)			/* Game selector port #2 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sms_io, AS_IO, 8, sms_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_MIRROR(0x3e) AM_WRITE(sms_bios_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0x3e) AM_WRITE(sms_io_control_w)
	AM_RANGE(0x40, 0x7f)                 AM_READ(sms_count_r)
	AM_RANGE(0x40, 0x7f)                 AM_DEVWRITE("segapsg", segapsg_new_device, write)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0) AM_MIRROR(0x1e) AM_READ(sms_input_port_0_r)
	AM_RANGE(0xc1, 0xc1) AM_MIRROR(0x1e) AM_READ(sms_input_port_1_r)
	AM_RANGE(0xe0, 0xe0) AM_MIRROR(0x0e) AM_READ(sms_input_port_0_r)
	AM_RANGE(0xe1, 0xe1) AM_MIRROR(0x0e) AM_READ(sms_input_port_1_r)
	AM_RANGE(0xf0, 0xf0)                 AM_READWRITE(sms_input_port_0_r, sms_ym2413_register_port_0_w)
	AM_RANGE(0xf1, 0xf1)                 AM_READWRITE(sms_input_port_1_r, sms_ym2413_data_port_0_w)
	AM_RANGE(0xf2, 0xf2)                 AM_READWRITE(sms_fm_detect_r, sms_fm_detect_w)
	AM_RANGE(0xf3, 0xf3)                 AM_READ(sms_input_port_1_r)
	AM_RANGE(0xf4, 0xf4) AM_MIRROR(0x02) AM_READ(sms_input_port_0_r)
	AM_RANGE(0xf5, 0xf5) AM_MIRROR(0x02) AM_READ(sms_input_port_1_r)
	AM_RANGE(0xf8, 0xf8) AM_MIRROR(0x06) AM_READ(sms_input_port_0_r)
	AM_RANGE(0xf9, 0xf9) AM_MIRROR(0x06) AM_READ(sms_input_port_1_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( gg_io, AS_IO, 8, sms_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00)                 AM_READ(gg_input_port_2_r)
	AM_RANGE(0x01, 0x05)                 AM_READWRITE(gg_sio_r, gg_sio_w)
	AM_RANGE(0x06, 0x06)                 AM_DEVWRITE("gamegear", gamegear_new_device, stereo_w)
	AM_RANGE(0x07, 0x07)                 AM_WRITE(sms_io_control_w)
	AM_RANGE(0x08, 0x08) AM_MIRROR(0x06) AM_WRITE(sms_bios_w)
	AM_RANGE(0x09, 0x09) AM_MIRROR(0x06) AM_WRITE(sms_io_control_w)
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0e) AM_WRITE(sms_bios_w)
	AM_RANGE(0x11, 0x11) AM_MIRROR(0x0e) AM_WRITE(sms_io_control_w)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x1e) AM_WRITE(sms_bios_w)
	AM_RANGE(0x21, 0x21) AM_MIRROR(0x1e) AM_WRITE(sms_io_control_w)
	AM_RANGE(0x40, 0x7f)                 AM_READ(sms_count_r)
	AM_RANGE(0x40, 0x7f)                 AM_DEVWRITE("gamegear", gamegear_new_device, write)
	AM_RANGE(0x80, 0x80) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, vram_read, vram_write)
	AM_RANGE(0x81, 0x81) AM_MIRROR(0x3e) AM_DEVREADWRITE("sms_vdp", sega315_5124_device, register_read, register_write)
	AM_RANGE(0xc0, 0xc0)                 AM_READ_PORT("PORT_DC")
	AM_RANGE(0xc1, 0xc1)                 AM_READ_PORT("PORT_DD")
	AM_RANGE(0xdc, 0xdc)                 AM_READ_PORT("PORT_DC")
	AM_RANGE(0xdd, 0xdd)                 AM_READ_PORT("PORT_DD")
ADDRESS_MAP_END


static INPUT_PORTS_START( sms )
	PORT_START("PORT_DC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)

	PORT_START("PORT_DD")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x00)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) /* Software Reset bit */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) /* Port A TH */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) /* Port B TH */

	PORT_START("PAUSE")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START ) PORT_NAME(DEF_STR(Pause))

	PORT_START("LPHASER0")	/* Light phaser X - player 1 */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR( X, 1.0, 0.0, 0 ) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, sms_state, lgun1_changed, NULL) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)

	PORT_START("LPHASER1")	/* Light phaser Y - player 1 */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR( Y, 1.0, 0.0, 0 ) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, sms_state, lgun1_changed, NULL) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)

	PORT_START("LPHASER2")	/* Light phaser X - player 2 */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR( X, 1.0, 0.0, 0 ) PORT_SENSITIVITY(50) PORT_KEYDELTA(15) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, sms_state, lgun2_changed, NULL) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)

	PORT_START("LPHASER3")	/* Light phaser Y - player 2 */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR( Y, 1.0, 0.0, 0 ) PORT_SENSITIVITY(50) PORT_KEYDELTA(25) PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, sms_state, lgun2_changed, NULL) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)

	PORT_START("RFU")	/* Rapid Fire Unit */
	PORT_CONFNAME( 0x03, 0x00, "Rapid Fire Unit - Player 1" )
	PORT_CONFSETTING(	0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(	0x01, "Button A" )
	PORT_CONFSETTING(	0x02, "Button B" )
	PORT_CONFSETTING(	0x03, "Button A + B" )
	PORT_CONFNAME( 0x0c, 0x00, "Rapid Fire Unit - Player 2" )
	PORT_CONFSETTING(	0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(	0x04, "Button A" )
	PORT_CONFSETTING(	0x08, "Button B" )
	PORT_CONFSETTING(	0x0c, "Button A + B" )

	PORT_START("PADDLE0")	/* Paddle player 1 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(40) PORT_KEYDELTA(20) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)

	PORT_START("PADDLE1")	/* Paddle player 2 */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE) PORT_SENSITIVITY(40) PORT_KEYDELTA(20) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)

	PORT_START("CTRLIPT")	/* Light Phaser and Paddle Control buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x01)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x02)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x04)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x04)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x10)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x20)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x40)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x40)

	PORT_START("CTRLSEL")	/* Controller selection */
	PORT_CONFNAME( 0x0f, 0x00, "Player 1 Controller" )
	PORT_CONFSETTING( 0x00, DEF_STR( Joystick ) )
	PORT_CONFSETTING( 0x01, "Light Phaser" )
	PORT_CONFSETTING( 0x02, "Sega Paddle Control" )
	PORT_CONFSETTING( 0x04, "Sega Sports Pad" )
	PORT_CONFNAME( 0xf0, 0x00, "Player 2 Controller" )
	PORT_CONFSETTING( 0x00, DEF_STR( Joystick ) )
	PORT_CONFSETTING( 0x10, "Light Phaser" )
	PORT_CONFSETTING( 0x20, "Sega Paddle Control" )
	PORT_CONFSETTING( 0x40, "Sega Sports Pad" )

	PORT_START("SPORT0")	/* Player 1 Sports Pad X axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_RESET PORT_REVERSE PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x04)

	PORT_START("SPORT1")	/* Player 1 Sports Pad Y axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_RESET PORT_REVERSE PORT_PLAYER(1) PORT_CONDITION("CTRLSEL", 0x0f, EQUALS, 0x04)

	PORT_START("SPORT2")	/* Player 2 Sports Pad X axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_RESET PORT_REVERSE PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x40)

	PORT_START("SPORT3")	/* Player 2 Sports Pad Y axis */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(40) PORT_RESET PORT_REVERSE PORT_PLAYER(2) PORT_CONDITION("CTRLSEL", 0xf0, EQUALS, 0x40)
INPUT_PORTS_END

static INPUT_PORTS_START( sms1 )
	PORT_INCLUDE( sms )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Reset Button")

	PORT_START("SEGASCOPE")
	PORT_CONFNAME( 0x01, 0x00, "SegaScope (3-D Glasses)" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )

	PORT_START("SSCOPE_BINOCULAR")
	PORT_CONFNAME( 0x03, 0x00, "SegaScope - Binocular Hack" ) PORT_CONDITION("SEGASCOPE", 0x01, EQUALS, 0x01)
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, "Left Lens" )
	PORT_CONFSETTING( 0x02, "Right Lens" )
	PORT_CONFSETTING( 0x03, "Both Lens" )
	PORT_BIT( 0x03, 0x00, IPT_UNUSED ) PORT_CONDITION("SEGASCOPE", 0x01, EQUALS, 0x00)
INPUT_PORTS_END

static INPUT_PORTS_START( smsj )
	PORT_INCLUDE( sms1 )

	PORT_START("TVDRAW")
	PORT_CONFNAME( 0x01, 0x00, "Terebi Oekaki Graphics Tablet" )
	PORT_CONFSETTING( 0x00, DEF_STR( Off ) )
	PORT_CONFSETTING( 0x01, DEF_STR( On ) )

	PORT_START("TVDRAW_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_NAME("Tablet - X Axis") PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_CONDITION("TVDRAW", 0x01, EQUALS, 0x01)

	PORT_START("TVDRAW_Y")
	PORT_BIT( 0xff, 0x60, IPT_LIGHTGUN_Y ) PORT_NAME("Tablet - Y Axis") PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0, 191) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)
	PORT_CONDITION("TVDRAW", 0x01, EQUALS, 0x01)

	PORT_START("TVDRAW_PEN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Tablet - Pen") PORT_CONDITION("TVDRAW", 0x01, EQUALS, 0x01)
INPUT_PORTS_END

static INPUT_PORTS_START( gg )
	PORT_START("PORT_DC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PORT_DD")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start") /* Game Gear START */
INPUT_PORTS_END


static WRITE_LINE_DEVICE_HANDLER( sms_int_callback )
{
	device->machine().device("maincpu")->execute().set_input_line(0, state);
}

static const sega315_5124_interface _315_5124_ntsc_intf =
{
	false,
	"screen",
	DEVCB_LINE(sms_int_callback),
	DEVCB_LINE(sms_pause_callback)
};

static const sega315_5124_interface _315_5124_pal_intf =
{
	true,
	"screen",
	DEVCB_LINE(sms_int_callback),
	DEVCB_LINE(sms_pause_callback)
};

static const sega315_5124_interface sms_store_intf =
{
	false,
	"screen",
	DEVCB_LINE(sms_store_int_callback),
	DEVCB_LINE(sms_pause_callback)
};


//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
    DEVCB_NULL
};


static MACHINE_CONFIG_FRAGMENT( sms_cartslot )
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("sms,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("sms_cart")
	MCFG_CARTSLOT_START(sms_cart)
	MCFG_CARTSLOT_LOAD(sms_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list","sms")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( gg_cartslot )
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("gg,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("gamegear_cart")
	MCFG_CARTSLOT_START(sms_cart)
	MCFG_CARTSLOT_LOAD(sms_cart)

	MCFG_SOFTWARE_LIST_ADD("cart_list","gamegear")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sms_ntsc_base, sms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_53_693175MHz/15)
	MCFG_CPU_PROGRAM_MAP(sms_mem)
	MCFG_CPU_IO_MAP(sms_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(sms_state,sms)
	MCFG_MACHINE_RESET_OVERRIDE(sms_state,sms)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("segapsg", SEGAPSG_NEW, XTAL_53_693175MHz/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	MCFG_FRAGMENT_ADD( sms_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms2_ntsc, sms_ntsc_base )
	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH , SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms)

	MCFG_PALETTE_LENGTH(SEGA315_5124_PALETTE_SIZE)
	MCFG_PALETTE_INIT(sega315_5124)

	MCFG_SEGA315_5246_ADD("sms_vdp", _315_5124_ntsc_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms1_ntsc, sms_ntsc_base )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sms1_mem)	// This adds the SegaScope handlers for 3-D glasses
	MCFG_CPU_IO_MAP(sms_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("left_lcd", LCD)	// This is needed for SegaScope Left LCD
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("right_lcd", LCD)	// This is needed for SegaScope Right LCD
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_224_TBORDER_HEIGHT + 224)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_DEFAULT_LAYOUT(layout_sms1)

	MCFG_PALETTE_LENGTH(SEGA315_5124_PALETTE_SIZE)
	MCFG_PALETTE_INIT(sega315_5124)

	MCFG_VIDEO_START_OVERRIDE(sms_state,sms1)

	MCFG_SEGA315_5124_ADD("sms_vdp", _315_5124_ntsc_intf)
MACHINE_CONFIG_END

#define MCFG_SMSSDISP_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_ADD(_tag) \
	MCFG_CARTSLOT_EXTENSION_LIST("sms,bin") \
	MCFG_CARTSLOT_NOT_MANDATORY \
	MCFG_CARTSLOT_INTERFACE("sms_cart") \
	MCFG_CARTSLOT_START(sms_cart) \
	MCFG_CARTSLOT_LOAD(sms_cart)

static MACHINE_CONFIG_DERIVED( sms_sdisp, sms2_ntsc )

	MCFG_DEVICE_REMOVE("sms_vdp")
	MCFG_SEGA315_5246_ADD("sms_vdp", sms_store_intf)

	MCFG_CPU_ADD("control", Z80, XTAL_53_693175MHz/15)
	MCFG_CPU_PROGRAM_MAP(sms_store_mem)
	/* Both CPUs seem to communicate with the VDP etc? */
	MCFG_CPU_IO_MAP(sms_io)

	MCFG_CARTSLOT_MODIFY("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("sms,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("sms_cart")
	MCFG_CARTSLOT_START(sms_cart)
	MCFG_CARTSLOT_LOAD(sms_cart)

	MCFG_SMSSDISP_CARTSLOT_ADD("cart2")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart3")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart4")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart5")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart6")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart7")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart8")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart9")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart10")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart11")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart12")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart13")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart14")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart15")
	MCFG_SMSSDISP_CARTSLOT_ADD("cart16")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sms_pal_base, sms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK_PAL/15)
	MCFG_CPU_PROGRAM_MAP(sms_mem)
	MCFG_CPU_IO_MAP(sms_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	MCFG_MACHINE_START_OVERRIDE(sms_state,sms)
	MCFG_MACHINE_RESET_OVERRIDE(sms_state,sms)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("segapsg", SEGAPSG_NEW, MASTER_CLOCK_PAL/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
	MCFG_SOUND_CONFIG(psg_intf)

	MCFG_FRAGMENT_ADD( sms_cartslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms2_pal, sms_pal_base )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms)

	MCFG_PALETTE_LENGTH(SEGA315_5124_PALETTE_SIZE)
	MCFG_PALETTE_INIT(sega315_5124)

	MCFG_SEGA315_5246_ADD("sms_vdp", _315_5124_pal_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms1_pal, sms_pal_base )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sms1_mem)	// This adds the SegaScope handlers for 3-D glasses
	MCFG_CPU_IO_MAP(sms_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("left_lcd", LCD)	// This is needed for SegaScope Left LCD
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_SCREEN_ADD("right_lcd", LCD)	// This is needed for SegaScope Right LCD
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_PAL/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH - 2, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 256 + 10, \
		SEGA315_5124_HEIGHT_PAL, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT, SEGA315_5124_TBORDER_START + SEGA315_5124_PAL_240_TBORDER_HEIGHT + 240)
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_sms1)

	MCFG_PALETTE_LENGTH(SEGA315_5124_PALETTE_SIZE)
	MCFG_PALETTE_INIT(sega315_5124)

	MCFG_DEFAULT_LAYOUT(layout_sms1)

	MCFG_VIDEO_START_OVERRIDE(sms_state,sms1)

	MCFG_SEGA315_5124_ADD("sms_vdp", _315_5124_pal_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms_fm, sms1_ntsc )

	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_53_693175MHz/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sg1000m3, sms_fm )

	MCFG_CARTSLOT_MODIFY("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("sms,bin,sg")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_START(sms_cart)
	MCFG_CARTSLOT_LOAD(sms_cart)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sms2_fm, sms2_ntsc )

	MCFG_SOUND_ADD("ym2413", YM2413, XTAL_53_693175MHz/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gamegear, sms_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_53_693175MHz/15)
	MCFG_CPU_PROGRAM_MAP(sms_mem)
	MCFG_CPU_IO_MAP(gg_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(sms_state,sms)
	MCFG_MACHINE_RESET_OVERRIDE(sms_state,sms)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_RAW_PARAMS(XTAL_53_693175MHz/10, \
		SEGA315_5124_WIDTH, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 6*8, SEGA315_5124_LBORDER_START + SEGA315_5124_LBORDER_WIDTH + 26*8, \
		SEGA315_5124_HEIGHT_NTSC, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_192_TBORDER_HEIGHT + 3*8, SEGA315_5124_TBORDER_START + SEGA315_5124_NTSC_192_TBORDER_HEIGHT + 21*8 )
	MCFG_SCREEN_UPDATE_DRIVER(sms_state, screen_update_gamegear)

	MCFG_PALETTE_LENGTH(SEGA315_5378_PALETTE_SIZE)
	MCFG_PALETTE_INIT(sega315_5378)

	MCFG_VIDEO_START_OVERRIDE(sms_state,gamegear)

	MCFG_SEGA315_5378_ADD("sms_vdp", _315_5124_ntsc_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
	MCFG_SOUND_ADD("gamegear", GAMEGEAR_NEW, XTAL_53_693175MHz/15)
	MCFG_SOUND_CONFIG(psg_intf)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	/* cartridge */
	MCFG_FRAGMENT_ADD( gg_cartslot )

	/* Some gamegear games use a 93c46 eeprom to store information */
	MCFG_EEPROM_93C46_ADD("eeprom")
MACHINE_CONFIG_END


ROM_START(sms1)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "bios13", "US/European BIOS v1.3 (1986)" )
	ROMX_LOAD("bios13fx.rom", 0x0000, 0x2000, CRC(0072ED54) SHA1(c315672807d8ddb8d91443729405c766dd95cae7), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91E93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "hangon", "US/European BIOS v3.4 with Hang On (1988)" )
	ROMX_LOAD("mpr-11458.rom", 0x0000, 0x20000, CRC(8EDF7AC6) SHA1(51fd6d7990f62cd9d18c9ecfc62ed7936169107e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "missiled", "US/European BIOS v4.4 with Missile Defense 3D (1988)" )
	ROMX_LOAD("missiled.rom", 0x0000, 0x20000, CRC(E79BB689) SHA1(aa92ae576ca670b00855e278378d89e9f85e0351), ROM_BIOS(4))
	ROM_SYSTEM_BIOS( 4, "v10", "US Master System BIOS v1.0 (prototype)" )
	ROMX_LOAD("v1.0.bin", 0x0000, 0x2000, CRC(72bec693) SHA1(29091ff60ef4c22b1ee17aa21e0e75bac6b36474), ROM_BIOS(5))
	ROM_SYSTEM_BIOS( 5, "proto", "US Master System Prototype BIOS" )
	ROMX_LOAD("m404prot.rom", 0x0000, 0x2000, CRC(1a15dfcc) SHA1(4a06c8e66261611dce0305217c42138b71331701), ROM_BIOS(6))
ROM_END

ROM_START(sms)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(CF4A09EA) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(1)) /* "SEGA // MPR-12808 W63 // 9114E9004" @ IC2 */
ROM_END

ROM_START(smssdisp)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)

	ROM_REGION(0x4000, "user1", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x4000, "control", 0)
	ROM_LOAD("smssdisp.rom", 0x0000, 0x4000, CRC(ee2c29ba) SHA1(fc465122134d95363112eb51b9ab71db3576cefd))
ROM_END

ROM_START(sms1pal)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "bios13", "US/European BIOS v1.3 (1986)" )
	ROMX_LOAD("bios13fx.rom", 0x0000, 0x2000, CRC(0072ED54) SHA1(c315672807d8ddb8d91443729405c766dd95cae7), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "hangonsh", "US/European BIOS v2.4 with Hang On and Safari Hunt (1988)" )
	ROMX_LOAD("mpr-11459a.rom", 0x0000, 0x20000, CRC(91E93385) SHA1(9e179392cd416af14024d8f31c981d9ee9a64517), ROM_BIOS(2))
	ROM_SYSTEM_BIOS( 2, "hangon", "Sega Master System - US/European BIOS v3.4 with Hang On (1988)" )
	ROMX_LOAD("mpr-11458.rom", 0x0000, 0x20000, CRC(8EDF7AC6) SHA1(51fd6d7990f62cd9d18c9ecfc62ed7936169107e), ROM_BIOS(3))
	ROM_SYSTEM_BIOS( 3, "missiled", "US/European BIOS v4.4 with Missile Defense 3D (1988)" )
	ROMX_LOAD("missiled.rom", 0x0000, 0x20000, CRC(E79BB689) SHA1(aa92ae576ca670b00855e278378d89e9f85e0351), ROM_BIOS(4))
ROM_END

ROM_START(smspal)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x40000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "alexkidd", "US/European BIOS with Alex Kidd in Miracle World (1990)" ) /* PCB Label: SEGA // IC BD M4Jr. PAL" Master System II with 314-5246 (ZIP) VDP and 314-5237 (DIP48) IO */
	ROMX_LOAD("mpr-12808.ic2", 0x0000, 0x20000, CRC(CF4A09EA) SHA1(3af7b66248d34eb26da40c92bf2fa4c73a46a051), ROM_BIOS(1)) /* "SEGA // MPR-12808 W63 // 9114E9004" @ IC2 */
	ROM_SYSTEM_BIOS( 1, "sonic", "European/Brazilian BIOS with Sonic the Hedgehog (1991)" )
	ROMX_LOAD("sonbios.rom", 0x0000, 0x40000, CRC(81C3476B) SHA1(6aca0e3dffe461ba1cb11a86cd4caf5b97e1b8df), ROM_BIOS(2))
ROM_END

ROM_START(sg1000m3)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)
ROM_END

ROM_START(smsj) /* PCB Label: "SEGA(R) IC BOARD M4J MAIN // 837-6418"; has "YM2413 // 78 04 71 G" at IC10; Back of pcb has traces marked "171-5541 (C)SEGA 1987 MADE IN JAPAN"
    see http://www.smspower.org/Development/JapaneseSMS837-6418 */
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x4000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "jbios21", "Japanese BIOS v2.1 (1987)" )
	ROMX_LOAD("mpr-11124.ic2", 0x0000, 0x2000, CRC(48D44A13) SHA1(a8c1b39a2e41137835eda6a5de6d46dd9fadbaf2), ROM_BIOS(1)) /* "SONY 7J06 // MPR-11124 // JAPAN 021" @ IC2 */
ROM_END

ROM_START(sms2kr)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0xff)

	ROM_REGION(0x20000, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "akbioskr", "Samsung Gam*Boy II with Alex Kidd in Miracle World (1990)" )
	ROMX_LOAD("akbioskr.rom", 0x000, 0x20000, CRC(9c5bad91) SHA1(2feafd8f1c40fdf1bd5668f8c5c02e5560945b17), ROM_BIOS(1))
ROM_END

ROM_START(gamegear)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_FILL(0x0000, 0x4000, 0x00)

	ROM_REGION(0x0400, "user1", 0)
	ROM_SYSTEM_BIOS( 0, "none", "No BIOS" ) /* gamegear */
	ROM_SYSTEM_BIOS( 1, "majesco", "Majesco BIOS" ) /* gamg */
	ROMX_LOAD("majbios.rom", 0x0000, 0x0400, CRC(0EBEA9D4) SHA1(914aa165e3d879f060be77870d345b60cfeb4ede), ROM_BIOS(2))
ROM_END

#define rom_gamegeaj rom_gamegear

/***************************************************************************

  Game driver(s)

  US
   - Sega Master System I (sms1)
     - prototype (M404) bios - 1986
     - without built-in game v1.3 - 1986
     - built-in Hang On/Safari Hunt v2.4 - 1988
     - built-in Hang On v3.4 - 1988
     - built-in Missile Defense 3-D v4.4 - 1988
     - built-in Hang On/Astro Warrior ????
   - Sega Master System II (sms)
     - built-in Alex Kidd in Miracle World - 1990

  JP
   - Sega SG-1000 Mark III (sg1000m3)
     - no bios
   - Sega Master System (I) (smsj)
     - without built-in game v2.1 - 1987

  KR
   - Sega Master System II (sms2kr)
     - built-in Alex Kidd in Miracle World (Korean)

  EU
   - Sega Master System I (sms1pal)
     - without built-in game v1.3 - 1986
     - built-in Hang On/Safari Hunt v2.4 - 1988
     - built-in Hang On v3.4 - 1988
     - built-in Missile Defense 3-D v4.4 - 1988
     - built-in Hang On/Astro Warrior ????
   - Sega Master System II (smspal)
     - built-in Alex Kidd in Miracle World - 1990
     - built-in Sonic the Hedgehog - 1991

  BR
   - Sega Master System I - 1987
   - Sega Master System II???
   - Sega Master System III - Tec Toy, 1987
   - Sega Master System Compact - Tec Toy, 1992
   - Sega Master System Girl - Tec Toy, 1992

***************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE      INPUT   INIT      COMPANY     FULLNAME                            FLAGS */
CONS( 1984, sg1000m3,   sms,        0,      sg1000m3,    smsj, sms_state,   sg1000m3, "Sega",     "SG-1000 Mark III",                 0 )
CONS( 1986, sms1,       sms,        0,      sms1_ntsc,   sms1, sms_state,   sms1,     "Sega",     "Master System I",                  0 )
CONS( 1986, sms1pal,    sms,        0,      sms1_pal,    sms1, sms_state,   sms1,     "Sega",     "Master System I (PAL)" ,           0 )
CONS( 1986, smssdisp,   sms,        0,      sms_sdisp,   sms, sms_state,    smssdisp, "Sega",     "Master System Store Display Unit", GAME_NOT_WORKING )
CONS( 1987, smsj,       sms,        0,      sms_fm,      smsj, sms_state,   smsj,     "Sega",     "Master System (Japan)",            0 )
CONS( 1990, sms,        0,          0,      sms2_ntsc,   sms, sms_state,    sms1,     "Sega",     "Master System II",                 0 )
CONS( 1990, smspal,     sms,        0,      sms2_pal,    sms, sms_state,    sms1,     "Sega",     "Master System II (PAL)",           0 )
CONS( 1990, sms2kr,     sms,        0,      sms2_fm,     sms, sms_state,    sms2kr,   "Samsung",  "Gam*Boy II (Korea)",               0 )

CONS( 1990, gamegear,   0,          sms,    gamegear,    gg, sms_state,     gamegear, "Sega",     "Game Gear (Europe/America)",       0 )
CONS( 1990, gamegeaj,   gamegear,   0,      gamegear,    gg, sms_state,     gamegeaj, "Sega",     "Game Gear (Japan)",                0 )
