// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Aaron Giles
/***************************************************************************

    Pipe Dream

    driver by Bryan McPhail & Aaron Giles

Pipe Dreams
  PCB# OP-11A-05
  OSC:
   12.000MHz   (next to Z80B Main CPU)
   14.31818MHz (next to Z80B Sub CPU)
    8.000MHz   (near YM2610 audio chip)

Hatris coinage:
  MAME uses the values listed in Test Mode.
  This does NOT match what's listed in Hatris Manual:

DIPSW-1 (from Hatris manual)
------------------------------------------------------------------
    DipSwitch Title   | Function | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
------------------------------------------------------------------
                      | 1cn/1pl  |off|off|off|off|               |*
                      | 1cn/2pl  |on |off|off|off|               |
                      | 1cn/3pl  |off|on |off|off|               |
                      | 1cn/4pl  |on |on |off|off|               |
                      | 1cn/5pl  |off|off|on |off|               |
                      | 1cn/6pl  |on |off|on |off|               |
     Coin Chute 1     | 2cn/1pl  |off|on |on |off|               |
                      | 3cn/1pl  |on |on |on |off|               |
                      | 4cn/1pl  |off|off|off|on |               |
                      | 5cn/1pl  |on |off|off|on |               |
                      | 2cn/3pl  |off|on |off|on |               |
                      | Multi A  |on |on |off|on |               |
                      | Multi B  |off|off|on |on |               |
                      | Multi C  |on |off|on |on |               |
                      | Multi D  |off|on |on |on |               |
                      | Multi E  |on |on |on |on |               |
------------------------------------------------------------------
                      | 1cn/1pl  |               |off|off|off|off|*
                      | 1cn/2pl  |               |on |off|off|off|
                      | 1cn/3pl  |               |off|on |off|off|
                      | 1cn/4pl  |               |on |on |off|off|
                      | 1cn/5pl  |               |off|off|on |off|
                      | 1cn/6pl  |               |on |off|on |off|
     Coin Chute 2     | 2cn/1pl  |               |off|on |on |off|
                      | 3cn/1pl  |               |on |on |on |off|
                      | 4cn/1pl  |               |off|off|off|on |
                      | 5cn/1pl  |               |on |off|off|on |
                      | 2cn/3pl  |               |off|on |off|on |
                      | Multi A  |               |on |on |off|on |
                      | Multi B  |               |off|off|on |on |
                      | Multi C  |               |on |off|on |on |
                      | Multi D  |               |off|on |on |on |
                      | Multi E  |               |on |on |on |on |
------------------------------------------------------------------

Added Multiple Coin Feature:
  Multi A = 2cn/1pl  4cn/2pl  5cn/3pl  6cn/4pl
  Multi B = 2cn/1pl  4cn/3pl
  Multi C = 1cn/1pl  2cn/2pl  3cn/3pl  4cn/4pl  5cn/6pl
  Multi D = 1cn/1pl  2cn/2pl  3cn/3pl  4cn/5pl
  Multi E = 1cn/1pl  2cn/3pl

****************************************************************************

    Memory map

****************************************************************************

    ========================================================================
    MAIN CPU
    ========================================================================
    0000-7FFF   R     xxxxxxxx   Program ROM
    8000-9FFF   R/W   xxxxxxxx   Program RAM
    A000-BFFF   R     xxxxxxxx   Banked ROM
    C000-CBFF   R/W   xxxxxxxx   Palette RAM (1536 entries x 2 bytes)
                R/W   ---xxxxx      (0: Blue)
                R/W   xxx-----      (0: Green, 3 LSB)
                R/W   ------xx      (1: Green, 2 MSB)
                R/W   -xxxxx--      (1: Red)
    CC00-CFFF   R/W   xxxxxxxx   Sprite RAM (256 entries x 8 bytes)
                R/W   xxxxxxxx      (0: Y position, 8 LSB)
                R/W   -------x      (1: Y position, 1 MSB)
                R/W   xxxx----      (1: Y zoom factor)
                R/W   xxxxxxxx      (2: X position, 8 LSB)
                R/W   -------x      (3: X position, 1 MSB)
                R/W   xxxx----      (3: X zoom factor)
                R/W   ---x----      (4: Priority)
                R/W   ----xxxx      (4: Palette entry)
                R/W   x-------      (5: Y flip)
                R/W   -xxx----      (5: Number of Y tiles - 1)
                R/W   ----x---      (5: X flip)
                R/W   -----xxx      (5: Number of X tiles - 1)
                R/W   xxxxxxxx      (6: Starting tile index, 8 LSB)
                R/W   ----xxxx      (7: Starting tile index, 4 MSB)
    D000-DFFF   R/W   --xxxxxx   Background tile color
    E000-EFFF   R/W   xxxxxxxx   Background tile index, 8 MSB
    F000-FFFF   R/W   xxxxxxxx   Background tile index, 8 LSB
    ========================================================================
    0020        R     xxxxxxxx   Player 1 controls
                R     --x-----      (Fast button)
                R     ---x----      (Place button)
                R     ----xxxx      (Joystick RLDU)
    0020          W   xxxxxxxx   Sound command
    0021        R     xxxxxxxx   Player 2 controls
                R     --x-----      (Fast button)
                R     ---x----      (Place button)
                R     ----xxxx      (Joystick RLDU)
    0021          W   -xxxxxxx   Bankswitch/video control
                  W   -x------      (Flip screen)
                  W   --x-----      (Background 2 X scroll, 1 MSB)
                  W   ---x----      (Background 1 X scroll, 1 MSB)
                  W   ----x---      (Background videoram select)
                  W   -----xxx      (Bank ROM select)
    0022        R     xxxxxxxx   Coinage DIP switch
                R     xxxx----      (Coin B)
                R     ----xxxx      (Coin A)
    0022          W   xxxxxxxx   Background 1 X scroll, 8 LSB
    0023        R     xxxxxxxx   Game options DIP switch
                R     x-------      (Test switch)
                R     -x------      (Training mode enable)
                R     --x-----      (Flip screen)
                R     ---x----      (Demo sounds)
                R     ----xx--      (Lives)
                R     ------xx      (Difficulty)
    0023          W   xxxxxxxx   Background 1 Y scroll
    0024        R     -x--xxxx   Coinage/start
                R     -x------      (Service coin)
                R     ----x---      (2 player start)
                R     -----x--      (1 player start)
                R     ------x-      (Coin B)
                R     -------x      (Coin A)
    0024          W   xxxxxxxx   Background 2 X scroll, 8 LSB
    0025        R     -------x   Sound command pending
    0025          W   xxxxxxxx   Background 2 Y scroll
    ========================================================================
    Interrupts:
       INT generated by CRTC VBLANK
    ========================================================================


    ========================================================================
    SOUND CPU
    ========================================================================
    0000-77FF   R     xxxxxxxx   Program ROM
    7800-7FFF   R/W   xxxxxxxx   Program RAM
    8000-FFFF   R     xxxxxxxx   Banked ROM
    ========================================================================
    0004          W   -------x   Bank ROM select
    0016        R     xxxxxxxx   Sound command read
    0017          W   --------   Sound command acknowledge
    0018-0019   R/W   xxxxxxxx   YM2610 port A
    001A-001B   R/W   xxxxxxxx   YM2610 port B
    ========================================================================
    Interrupts:
       INT generated by YM2610
       NMI generated by command from main CPU
    ========================================================================

***************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2608intf.h"
#include "sound/2610intf.h"
#include "includes/fromance.h"


class pipedrm_state : public fromance_state
{
public:
	pipedrm_state(const machine_config &mconfig, device_type type, const char *tag)
		: fromance_state(mconfig, type, tag)
	{ }

	DECLARE_MACHINE_START(pipedrm);
	DECLARE_MACHINE_RESET(pipedrm);
	DECLARE_DRIVER_INIT(pipedrm);
	DECLARE_DRIVER_INIT(hatris);
	DECLARE_WRITE8_MEMBER( pipedrm_bankswitch_w );
	DECLARE_WRITE8_MEMBER( sound_bankswitch_w );
	TIMER_CALLBACK_MEMBER( delayed_command_w );
	DECLARE_WRITE8_MEMBER( sound_command_w );
	DECLARE_WRITE8_MEMBER( sound_command_nonmi_w );
	DECLARE_WRITE8_MEMBER( pending_command_clear_w );
	DECLARE_READ8_MEMBER( pending_command_r );
	DECLARE_READ8_MEMBER( sound_command_r );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
};


/*************************************
 *
 *  Bankswitching
 *
 *************************************/

WRITE8_MEMBER(pipedrm_state::pipedrm_bankswitch_w )
{
	/*
	    Bit layout:

	    D7 = unknown
	    D6 = flip screen
	    D5 = background 2 X scroll MSB
	    D4 = background 1 X scroll MSB
	    D3 = background videoram select
	    D2-D0 = program ROM bank select
	*/

	/* set the memory bank on the Z80 using the low 3 bits */
	membank("bank1")->set_entry(data & 0x7);

	/* map to the fromance gfx register */
	fromance_gfxreg_w(space, offset, ((data >> 6) & 0x01) |  /* flipscreen */
								((~data >> 2) & 0x02)); /* videoram select */
}


WRITE8_MEMBER(pipedrm_state::sound_bankswitch_w )
{
	membank("bank2")->set_entry(data & 0x01);
}



/*************************************
 *
 *  Sound CPU I/O
 *
 *************************************/

TIMER_CALLBACK_MEMBER(pipedrm_state::delayed_command_w)
{
	m_sound_command = param & 0xff;
	m_pending_command = 1;

	/* Hatris polls commands *and* listens to the NMI; this causes it to miss */
	/* sound commands. It's possible the NMI isn't really hooked up on the YM2608 */
	/* sound board. */
	if (param & 0x100)
		m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


WRITE8_MEMBER(pipedrm_state::sound_command_w )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pipedrm_state::delayed_command_w),this), data | 0x100);
}


WRITE8_MEMBER(pipedrm_state::sound_command_nonmi_w )
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pipedrm_state::delayed_command_w),this), data);
}


WRITE8_MEMBER(pipedrm_state::pending_command_clear_w )
{
	m_pending_command = 0;
	m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


READ8_MEMBER(pipedrm_state::pending_command_r )
{
	return m_pending_command;
}


READ8_MEMBER(pipedrm_state::sound_command_r )
{
	return m_sound_command;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, pipedrm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xd000, 0xffff) AM_READWRITE(fromance_videoram_r, fromance_videoram_w) AM_SHARE("videoram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( main_portmap, AS_IO, 8, pipedrm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_WRITE(fromance_crtc_data_w)
	AM_RANGE(0x11, 0x11) AM_WRITE(fromance_crtc_register_w)
	AM_RANGE(0x20, 0x20) AM_READ_PORT("P1") AM_WRITE(sound_command_w)
	AM_RANGE(0x21, 0x21) AM_READ_PORT("P2") AM_WRITE(pipedrm_bankswitch_w)
	AM_RANGE(0x22, 0x25) AM_WRITE(fromance_scroll_w)
	AM_RANGE(0x22, 0x22) AM_READ_PORT("DSW1")
	AM_RANGE(0x23, 0x23) AM_READ_PORT("DSW2")
	AM_RANGE(0x24, 0x24) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x25, 0x25) AM_READ(pending_command_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, pipedrm_state )
	AM_RANGE(0x0000, 0x77ff) AM_ROM
	AM_RANGE(0x7800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROMBANK("bank2")
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, pipedrm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x04, 0x04) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0x16, 0x16) AM_READ(sound_command_r)
	AM_RANGE(0x17, 0x17) AM_WRITE(pending_command_clear_w)
	AM_RANGE(0x18, 0x1b) AM_DEVREADWRITE("ymsnd", ym2610_device, read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( hatris_sound_portmap, AS_IO, 8, pipedrm_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_MIRROR(0x08) AM_DEVREADWRITE("ymsnd", ym2608_device, read, write)
	AM_RANGE(0x04, 0x04) AM_READ(sound_command_r)
	AM_RANGE(0x05, 0x05) AM_READWRITE(pending_command_r, pending_command_clear_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( pipedrm )
	PORT_START("P1")    /* $20 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    /* $21 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* $24 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* $22 */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x06, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x60, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")  /* $23 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, "Super" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Training Mode" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static INPUT_PORTS_START( hatris )
	PORT_START("P1")    /* $20 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    /* $21 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* $24 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")  /* $22 */
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x09, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0b, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0d, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(    0x0a, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x90, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xb0, "6 Coins/4 Credits" )
	PORT_DIPSETTING(    0xc0, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xd0, "5 Coins/6 Credits" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 2C_3C ) )
//  PORT_DIPSETTING(    0xa0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")  /* $23 */
	PORT_DIPNAME( 0x03, 0x00, "Hat Fall Velocity" ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, "Super" )
	PORT_DIPNAME( 0x0c, 0x00, "End Line Position" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0c, "Super" )
	PORT_SERVICE_DIPLOC(  0x0010, IP_ACTIVE_HIGH, "SW2:5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW2:8" ) /* Listed as "N.C." */
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout bglayout =
{
	8,4,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24 },
	{ 0*32, 1*32, 2*32, 3*32 },
	8*16
};


static const gfx_layout splayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 12, 8, 28, 24, 4, 0, 20, 16, 44, 40, 60, 56, 36, 32, 52, 48 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	8*128
};


static GFXDECODE_START( pipedrm )
	GFXDECODE_ENTRY( "gfx1", 0, bglayout,    0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, bglayout,    0, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, splayout, 1024, 32 )
GFXDECODE_END


static GFXDECODE_START( hatris )
	GFXDECODE_ENTRY( "gfx1", 0, bglayout,    0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, bglayout,    0, 128 )
GFXDECODE_END



/*************************************
 *
 *  Sound definitions
 *
 *************************************/

WRITE_LINE_MEMBER(pipedrm_state::irqhandler)
{
	m_subcpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

MACHINE_START_MEMBER(pipedrm_state,pipedrm)
{
	/* initialize main Z80 bank */
	membank("bank1")->configure_entries(0, 8, memregion("maincpu")->base() + 0x10000, 0x2000);
	membank("bank1")->set_entry(0);

	/* initialize sound bank */
	membank("bank2")->configure_entries(0, 2, memregion("sub")->base() + 0x10000, 0x8000);
	membank("bank2")->set_entry(0);

	/* state save */
	save_item(NAME(m_pending_command));
	save_item(NAME(m_sound_command));

	/* video-related elements are saved in video_start */
}

MACHINE_RESET_MEMBER(pipedrm_state,pipedrm)
{
	int i;

	m_pending_command = 0;
	m_sound_command = 0;

	m_flipscreen_old = -1;
	m_scrollx_ofs = 0x159;
	m_scrolly_ofs = 0x10;

	m_selected_videoram = m_selected_paletteram = 0;
	m_scrollx[0] = 0;
	m_scrollx[1] = 0;
	m_scrolly[0] = 0;
	m_scrolly[1] = 0;
	m_gfxreg = 0;
	m_flipscreen = 0;
	m_crtc_register = 0;

	for (i = 0; i < 0x10; i++)
		m_crtc_data[i] = 0;
}

static MACHINE_CONFIG_START( pipedrm, pipedrm_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pipedrm_state, irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,14318000/4)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	MCFG_MACHINE_START_OVERRIDE(pipedrm_state,pipedrm)
	MCFG_MACHINE_RESET_OVERRIDE(pipedrm_state,pipedrm)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(44*8, 30*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 44*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pipedrm_state, screen_update_pipedrm)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pipedrm)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_DEVICE_ADD("vsystem_spr_old", VSYSTEM_SPR2, 0)
	MCFG_VSYSTEM_SPR2_SET_GFXREGION(2)
	MCFG_VSYSTEM_SPR2_SET_OFFSETS(-13, -6)
	MCFG_VSYSTEM_SPR2_SET_PRITYPE(3)
	MCFG_VSYSTEM_SPR2_GFXDECODE("gfxdecode")
	MCFG_VSYSTEM_SPR2_PALETTE("palette")

	MCFG_VIDEO_START_OVERRIDE(pipedrm_state,pipedrm)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2610, 8000000)
	MCFG_YM2610_IRQ_HANDLER(WRITELINE(pipedrm_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( hatris, pipedrm_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,12000000/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pipedrm_state, irq0_line_hold)

	MCFG_CPU_ADD("sub", Z80,14318000/4)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(hatris_sound_portmap)

	MCFG_MACHINE_START_OVERRIDE(pipedrm_state,pipedrm)
	MCFG_MACHINE_RESET_OVERRIDE(pipedrm_state,pipedrm)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(44*8, 30*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 44*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pipedrm_state, screen_update_fromance)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hatris)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_VIDEO_START_OVERRIDE(pipedrm_state,hatris)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2608, 8000000)
	MCFG_YM2608_IRQ_HANDLER(WRITELINE(pipedrm_state, irqhandler))
	MCFG_SOUND_ROUTE(0, "mono", 0.50)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)
	MCFG_SOUND_ROUTE(2, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( pipedrm )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ya.u129", 0x00000, 0x08000, CRC(9b4d84a2) SHA1(82c26cf52b37ca3bcc10a534759e7bb52b1daa2d) )
	ROM_LOAD( "yb.u110", 0x10000, 0x10000, CRC(7416554a) SHA1(612aff94da3ec282e200c07eae9af26a28e071bd) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "u4.u86", 0x00000, 0x08000, CRC(497fad4c) SHA1(f151543a0c4a1d6d5d2de5e1dc12fd59dabcf1a8) )
	ROM_LOAD( "u3.u99", 0x10000, 0x10000, CRC(4800322a) SHA1(a616c497ac18351b68b8307050a2a62c717a7873) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "s73", 0x000000, 0x80000, CRC(63f4e10c) SHA1(ba935490578887080d8b16508fa6191236a8fea6) )
	ROM_LOAD( "s72", 0x080000, 0x80000, CRC(4e669e97) SHA1(1de8a8cd8f8f69fa86b8fe2c73c6997e7a89c706) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "s71",  0x000000, 0x80000, CRC(431485ee) SHA1(70a2ba5338598db9fcd9ef2be46e5cc2fd9510ee) )
	ROM_COPY( "gfx1", 0x080000, 0x080000, 0x80000 )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "a30.u12", 0x00000, 0x40000, CRC(50bc5e98) SHA1(b351af780d04e67a560935a9eeaedf597ac5bb1f) )
	ROM_LOAD16_BYTE( "a29.u2",  0x00001, 0x40000, CRC(a240a448) SHA1(d64169258e91eb09e8685bcdd96b16bf56e82ef1) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "g71.u118", 0x00000, 0x80000, CRC(488e2fd1) SHA1(8ef8ceb2bd36a245138802f51babf62f17c30942) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "g72.u83", 0x00000, 0x80000, CRC(dc3d14be) SHA1(4220f3fd13487dd861ac84b1b0d3e92125b3cc19) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce16v8h.114", 0x0000, 0x0117, CRC(1f3a3816) SHA1(2b4d84ab98036b8861961f610b1b1ec23a653ef7) ) /* Stamped 1023 */
	ROM_LOAD( "gal16v8a.115",   0x0200, 0x0117, CRC(2b32e239) SHA1(a3b9e45a1ce15ea4cc5754b2bf89cbaa416e814a) ) /* Stamped 1015 */
	ROM_LOAD( "gal16v8a.116",   0x0400, 0x0117, CRC(3674f043) SHA1(06c88f65877a6575149bdd4f7cea64cd310227bd) ) /* Stamped 1014 */
	ROM_LOAD( "gal16v8a.127",   0x0600, 0x0117, CRC(7115d95c) SHA1(23044039373b5a2face63d72c3fc6bf7f0c8a475) ) /* Stamped 1016 */
ROM_END

ROM_START( pipedrmu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "01.u129", 0x00000, 0x08000, CRC(9fe261fb) SHA1(57beeeade8809be0a71086f55b14b1676c0b3759) )
	ROM_LOAD( "02.u110", 0x10000, 0x10000, CRC(c8209b67) SHA1(cca7356d75e8091b07e3328aef523ff452abbcd8) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "u4.u86", 0x00000, 0x08000, CRC(497fad4c) SHA1(f151543a0c4a1d6d5d2de5e1dc12fd59dabcf1a8) )
	ROM_LOAD( "u3.u99", 0x10000, 0x10000, CRC(4800322a) SHA1(a616c497ac18351b68b8307050a2a62c717a7873) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "s73", 0x000000, 0x80000, CRC(63f4e10c) SHA1(ba935490578887080d8b16508fa6191236a8fea6) )
	ROM_LOAD( "s72", 0x080000, 0x80000, CRC(4e669e97) SHA1(1de8a8cd8f8f69fa86b8fe2c73c6997e7a89c706) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "s71",  0x000000, 0x80000, CRC(431485ee) SHA1(70a2ba5338598db9fcd9ef2be46e5cc2fd9510ee) )
	ROM_COPY( "gfx1", 0x080000, 0x080000, 0x80000 )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "a30.u12", 0x00000, 0x40000, CRC(50bc5e98) SHA1(b351af780d04e67a560935a9eeaedf597ac5bb1f) )
	ROM_LOAD16_BYTE( "a29.u2",  0x00001, 0x40000, CRC(a240a448) SHA1(d64169258e91eb09e8685bcdd96b16bf56e82ef1) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "g71.u118", 0x00000, 0x80000, CRC(488e2fd1) SHA1(8ef8ceb2bd36a245138802f51babf62f17c30942) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "g72.u83", 0x00000, 0x80000, CRC(dc3d14be) SHA1(4220f3fd13487dd861ac84b1b0d3e92125b3cc19) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce16v8h.114", 0x0000, 0x0117, CRC(1f3a3816) SHA1(2b4d84ab98036b8861961f610b1b1ec23a653ef7) ) /* Stamped 1023 */
	ROM_LOAD( "gal16v8a.115",   0x0200, 0x0117, CRC(2b32e239) SHA1(a3b9e45a1ce15ea4cc5754b2bf89cbaa416e814a) ) /* Stamped 1015 */
	ROM_LOAD( "gal16v8a.116",   0x0400, 0x0117, CRC(3674f043) SHA1(06c88f65877a6575149bdd4f7cea64cd310227bd) ) /* Stamped 1014 */
	ROM_LOAD( "gal16v8a.127",   0x0600, 0x0117, CRC(7115d95c) SHA1(23044039373b5a2face63d72c3fc6bf7f0c8a475) ) /* Stamped 1016 */
ROM_END


ROM_START( pipedrmj )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "1.u129", 0x00000, 0x08000, CRC(dbfac46b) SHA1(98ddfaed61de28b238964445572eb398b9dd03c7) )
	ROM_LOAD( "2.u110", 0x10000, 0x10000, CRC(b7adb99a) SHA1(fdab2b99e86aa0b6b17ec95556222e5211ba55e9) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "u4.u86", 0x00000, 0x08000, CRC(497fad4c) SHA1(f151543a0c4a1d6d5d2de5e1dc12fd59dabcf1a8) )
	ROM_LOAD( "u3.u99", 0x10000, 0x10000, CRC(4800322a) SHA1(a616c497ac18351b68b8307050a2a62c717a7873) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "s73", 0x000000, 0x80000, CRC(63f4e10c) SHA1(ba935490578887080d8b16508fa6191236a8fea6) )
	ROM_LOAD( "s72", 0x080000, 0x80000, CRC(4e669e97) SHA1(1de8a8cd8f8f69fa86b8fe2c73c6997e7a89c706) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "s71",  0x000000, 0x80000, CRC(431485ee) SHA1(70a2ba5338598db9fcd9ef2be46e5cc2fd9510ee) )
	ROM_COPY( "gfx1", 0x080000, 0x080000, 0x80000 )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "a30.u12", 0x00000, 0x40000, CRC(50bc5e98) SHA1(b351af780d04e67a560935a9eeaedf597ac5bb1f) )
	ROM_LOAD16_BYTE( "a29.u2",  0x00001, 0x40000, CRC(a240a448) SHA1(d64169258e91eb09e8685bcdd96b16bf56e82ef1) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "g71.u118", 0x00000, 0x80000, CRC(488e2fd1) SHA1(8ef8ceb2bd36a245138802f51babf62f17c30942) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "g72.u83", 0x00000, 0x80000, CRC(dc3d14be) SHA1(4220f3fd13487dd861ac84b1b0d3e92125b3cc19) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce16v8h.114", 0x0000, 0x0117, CRC(1f3a3816) SHA1(2b4d84ab98036b8861961f610b1b1ec23a653ef7) ) /* Stamped 1023 */
	ROM_LOAD( "gal16v8a.115",   0x0200, 0x0117, CRC(2b32e239) SHA1(a3b9e45a1ce15ea4cc5754b2bf89cbaa416e814a) ) /* Stamped 1015 */
	ROM_LOAD( "gal16v8a.116",   0x0400, 0x0117, CRC(3674f043) SHA1(06c88f65877a6575149bdd4f7cea64cd310227bd) ) /* Stamped 1014 */
	ROM_LOAD( "gal16v8a.127",   0x0600, 0x0117, CRC(7115d95c) SHA1(23044039373b5a2face63d72c3fc6bf7f0c8a475) ) /* Stamped 1016 */
ROM_END


ROM_START( pipedrmt )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "t1.u129", 0x00000, 0x08000, CRC(335401a4) SHA1(1839bfbecd1b5198c93fd6ea182cd3b1b9c3ba6a) )
	ROM_LOAD( "t2.u110", 0x10000, 0x10000, CRC(c8209b67) SHA1(cca7356d75e8091b07e3328aef523ff452abbcd8) )

	ROM_REGION( 0x20000, "sub", 0 )
	ROM_LOAD( "u4.u86", 0x00000, 0x08000, CRC(497fad4c) SHA1(f151543a0c4a1d6d5d2de5e1dc12fd59dabcf1a8) )
	ROM_LOAD( "u3.u99", 0x10000, 0x10000, CRC(4800322a) SHA1(a616c497ac18351b68b8307050a2a62c717a7873) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "s73", 0x000000, 0x80000, CRC(63f4e10c) SHA1(ba935490578887080d8b16508fa6191236a8fea6) )
	ROM_LOAD( "s72", 0x080000, 0x80000, CRC(4e669e97) SHA1(1de8a8cd8f8f69fa86b8fe2c73c6997e7a89c706) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "s71",  0x000000, 0x80000, CRC(431485ee) SHA1(70a2ba5338598db9fcd9ef2be46e5cc2fd9510ee) )
	ROM_COPY( "gfx1", 0x080000, 0x080000, 0x80000 )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "a30.u12", 0x00000, 0x40000, CRC(50bc5e98) SHA1(b351af780d04e67a560935a9eeaedf597ac5bb1f) )
	ROM_LOAD16_BYTE( "a29.u2",  0x00001, 0x40000, CRC(a240a448) SHA1(d64169258e91eb09e8685bcdd96b16bf56e82ef1) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "g71.u118", 0x00000, 0x80000, CRC(488e2fd1) SHA1(8ef8ceb2bd36a245138802f51babf62f17c30942) )

	ROM_REGION( 0x80000, "ymsnd.deltat", 0 )
	ROM_LOAD( "g72.u83", 0x00000, 0x80000, CRC(dc3d14be) SHA1(4220f3fd13487dd861ac84b1b0d3e92125b3cc19) )

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce16v8h.114", 0x0000, 0x0117, CRC(1f3a3816) SHA1(2b4d84ab98036b8861961f610b1b1ec23a653ef7) ) /* Stamped 1023 */
	ROM_LOAD( "gal16v8a.115",   0x0200, 0x0117, CRC(2b32e239) SHA1(a3b9e45a1ce15ea4cc5754b2bf89cbaa416e814a) ) /* Stamped 1015 */
	ROM_LOAD( "gal16v8a.116",   0x0400, 0x0117, CRC(3674f043) SHA1(06c88f65877a6575149bdd4f7cea64cd310227bd) ) /* Stamped 1014 */
	ROM_LOAD( "gal16v8a.127",   0x0600, 0x0117, CRC(7115d95c) SHA1(23044039373b5a2face63d72c3fc6bf7f0c8a475) ) /* Stamped 1016 */
ROM_END


ROM_START( hatris )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2.ic79", 0x00000, 0x08000, CRC(4ab50b54) SHA1(0eaab164a88c127bdf05c72f36d95be7fa3bb7de) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1-ic81.bin", 0x00000, 0x08000, CRC(db25e166) SHA1(3538963d092967311d0a216b1e33ea39389b0d87) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b0-ic56.bin", 0x00000, 0x20000, CRC(34f337a4) SHA1(ad74bb3fbfd16c9e92daa1cf5c5e522d11ba7dfb) )
	ROM_FILL(                0x20000, 0x20000, 0 )
	ROM_LOAD( "b1-ic73.bin", 0x40000, 0x08000, CRC(6351d0ba) SHA1(6d6b2e23f0569e625414de11803955df60bbbd48) )
	ROM_FILL(                0x48000, 0x18000, 0 )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "a0-ic55.bin", 0x00000, 0x20000, CRC(7b7bc619) SHA1(b661c772e33aa7352dcdc20c4a9a84ed25ff89d7) )
	ROM_LOAD( "a1-ic60.bin", 0x20000, 0x20000, CRC(f74d4168) SHA1(9ac433c4ce61fe402334aa97d32a51cfac634c46) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "pc-ic53.bin", 0x00000, 0x20000, CRC(07147712) SHA1(97692186e85f3a4a19dbd1bd95ed882e903a3c4a) )
ROM_END


ROM_START( hatrisj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2-ic79.bin", 0x00000, 0x08000, CRC(bbcaddbf) SHA1(7f01493dadfed87112644a8ef77ae58fa273980d) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "1-ic81.bin", 0x00000, 0x08000, CRC(db25e166) SHA1(3538963d092967311d0a216b1e33ea39389b0d87) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "b0-ic56.bin", 0x00000, 0x20000, CRC(34f337a4) SHA1(ad74bb3fbfd16c9e92daa1cf5c5e522d11ba7dfb) )
	ROM_FILL(                0x20000, 0x20000, 0 )
	ROM_LOAD( "b1-ic73.bin", 0x40000, 0x08000, CRC(6351d0ba) SHA1(6d6b2e23f0569e625414de11803955df60bbbd48) )
	ROM_FILL(                0x48000, 0x18000, 0 )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "a0-ic55.bin", 0x00000, 0x20000, CRC(7b7bc619) SHA1(b661c772e33aa7352dcdc20c4a9a84ed25ff89d7) )
	ROM_LOAD( "a1-ic60.bin", 0x20000, 0x20000, CRC(f74d4168) SHA1(9ac433c4ce61fe402334aa97d32a51cfac634c46) )

	ROM_REGION( 0x20000, "ymsnd", 0 )
	ROM_LOAD( "pc-ic53.bin", 0x00000, 0x20000, CRC(07147712) SHA1(97692186e85f3a4a19dbd1bd95ed882e903a3c4a) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(pipedrm_state,pipedrm)
{
	const memory_share *share = memshare("palette");
	/* sprite RAM lives at the end of palette RAM */
	m_spriteram.set_target((UINT8*)share->ptr() + 0xc00, 0x400);
	m_maincpu->space(AS_PROGRAM).install_ram(0xcc00, 0xcfff, m_spriteram);
}


DRIVER_INIT_MEMBER(pipedrm_state,hatris)
{
	m_maincpu->space(AS_IO).install_write_handler(0x20, 0x20, write8_delegate(FUNC(pipedrm_state::sound_command_nonmi_w),this));
	m_maincpu->space(AS_IO).install_write_handler(0x21, 0x21, write8_delegate(FUNC(pipedrm_state::fromance_gfxreg_w),this));
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1990, pipedrm,  0,       pipedrm, pipedrm, pipedrm_state, pipedrm, ROT0, "Video System Co.", "Pipe Dream (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pipedrmu, pipedrm, pipedrm, pipedrm, pipedrm_state, pipedrm, ROT0, "Video System Co.", "Pipe Dream (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pipedrmj, pipedrm, pipedrm, pipedrm, pipedrm_state, pipedrm, ROT0, "Video System Co.", "Pipe Dream (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, pipedrmt, pipedrm, pipedrm, pipedrm, pipedrm_state, pipedrm, ROT0, "Video System Co.", "Pipe Dream (Taiwan)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hatris,   0,       hatris,  hatris, pipedrm_state,  hatris,  ROT0, "Video System Co.", "Hatris (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, hatrisj,  hatris,  hatris,  hatris, pipedrm_state,  hatris,  ROT0, "Video System Co.", "Hatris (Japan)", MACHINE_SUPPORTS_SAVE )
