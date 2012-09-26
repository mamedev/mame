/******************************************************************************
    Atari 400/800

    MESS Driver

    Juergen Buchmueller, June 1998

    2009-05 FP changes:
     Factored out MESS specific code from MAME
     Added skeleton support for other XL/XE machines (VERY preliminary):
     - a600xl based on maxaflex emulation in MAME
     - a1200xl sharing a800xl code without BASIC
     - a65xe, a65xea, a130xe, a800xe, xegs sharing a800xl code (and this is wrong
      at least for xegs)
     Added proper dumps and labels, thanks to Freddy Offenga researches (a few
     are still marked BAD_DUMP while waiting for crc confirmation, since they
     have been obtained by splitting whole dumps)

     To Do:
     - Find out why a600xl and a800xl don't work (xe machines should then follow)
     - Investigate supported RAM sizes and OS versions in different models
     - Implement differences between various models (currently most of the
      XL/XE are exactly an a800xl, but this will change as soon as emulation
      starts to work)
     - Fix various keyboard differences
     - Freddy emulation for 800XLF?
     - Add support for proto boards and expansions (a1400xl, C/PM board, etc.)
     - Clean up the whole driver + cart + floppy structure

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"
#include "machine/ataridev.h"
#include "imagedev/cartslot.h"
#include "sound/pokey.h"
#include "machine/6821pia.h"
#include "video/gtia.h"
#include "sound/dac.h"
#include "machine/ram.h"

/******************************************************************************
    Atari 800 memory map (preliminary)

    ***************** read access *******************
    range     short   description
    0000-9FFF RAM     main memory
    A000-BFFF RAM/ROM RAM or (banked) ROM cartridges
    C000-CFFF ROM     unused or monitor ROM

    ********* GTIA    ********************************

    D000      m0pf    missile 0 playfield collisions
    D001      m1pf    missile 1 playfield collisions
    D002      m2pf    missile 2 playfield collisions
    D003      m3pf    missile 3 playfield collisions
    D004      p0pf    player 0 playfield collisions
    D005      p1pf    player 1 playfield collisions
    D006      p2pf    player 2 playfield collisions
    D007      p3pf    player 3 playfield collisions
    D008      m0pl    missile 0 player collisions
    D009      m1pl    missile 1 player collisions
    D00A      m2pl    missile 2 player collisions
    D00B      m3pl    missile 3 player collisions
    D00C      p0pl    player 0 player collisions
    D00D      p1pl    player 1 player collisions
    D00E      p2pl    player 2 player collisions
    D00F      p3pl    player 3 player collisions
    D010      but0    button stick 0
    D011      but1    button stick 1
    D012      but2    button stick 2
    D013      but3    button stick 3
    D014      xff     unused
    D015      xff     unused
    D016      xff     unused
    D017      xff     unused
    D018      xff     unused
    D019      xff     unused
    D01A      xff     unused
    D01B      xff     unused
    D01C      xff     unused
    D01D      xff     unused
    D01E      xff     unused
    D01F      cons    console keys
    D020-D0FF repeated 7 times

    D100-D1FF xff

    ********* POKEY   ********************************
    D200      pot0    paddle 0
    D201      pot1    paddle 1
    D202      pot2    paddle 2
    D203      pot3    paddle 3
    D204      pot4    paddle 4
    D205      pot5    paddle 5
    D206      pot6    paddle 6
    D207      pot7    paddle 7
    D208      potb    all paddles
    D209      kbcode  keyboard scan code
    D20A      random  random number generator
    D20B      xff     unused
    D20C      xff     unused
    D20D      serin   serial input
    D20E      irqst   IRQ status
    D20F      skstat  sk status
    D210-D2FF repeated 15 times

    ********* PIO     ********************************
    D300      porta   read pio port A
    D301      portb   read pio port B
    D302      pactl   read pio port A control
    D303      pbctl   read pio port B control
    D304-D3FF repeated 63 times

    ********* ANTIC   ********************************
    D400      xff     unused
    D401      xff     unused
    D402      xff     unused
    D403      xff     unused
    D404      xff     unused
    D405      xff     unused
    D406      xff     unused
    D407      xff     unused
    D408      xff     unused
    D409      xff     unused
    D40A      xff     unused
    D40B      vcount  vertical (scanline) counter
    D40C      penh    light pen horizontal pos
    D40D      penv    light pen vertical pos
    D40E      xff     unused
    D40F      nmist   NMI status

    D500-D7FF xff     unused memory

    D800-DFFF ROM     floating point ROM
    E000-FFFF ROM     bios ROM

    ***************** write access *******************
    range     short   description
    0000-9FFF RAM     main memory
    A000-BFFF RAM/ROM RAM or (banked) ROM
    C000-CFFF ROM     unused or monitor ROM

    ********* GTIA    ********************************
    D000      hposp0  player 0 horz position
    D001      hposp1  player 1 horz position
    D002      hposp2  player 2 horz position
    D003      hposp3  player 3 horz position
    D004      hposm0  missile 0 horz position
    D005      hposm1  missile 0 horz position
    D006      hposm2  missile 0 horz position
    D007      hposm3  missile 0 horz position
    D008      sizep0  size player 0
    D009      sizep1  size player 0
    D00A      sizep2  size player 0
    D00B      sizep3  size player 0
    D00C      sizem   size missiles
    D00D      grafp0  graphics data for player 0
    D00E      grafp1  graphics data for player 1
    D00F      grafp2  graphics data for player 2
    D010      grafp3  graphics data for player 3
    D011      grafm   graphics data for missiles
    D012      colpm0  color for player/missile 0
    D013      colpm1  color for player/missile 1
    D014      colpm2  color for player/missile 2
    D015      colpm3  color for player/missile 3
    D016      colpf0  color 0 playfield
    D017      colpf1  color 1 playfield
    D018      colpf2  color 2 playfield
    D019      colpf3  color 3 playfield
    D01A      colbk   background playfield
    D01B      prior   priority select
    D01C      vdelay  delay until vertical retrace
    D01D      gractl  graphics control
    D01E      hitclr  clear collisions
    D01F      wcons   write console (speaker)
    D020-D0FF repeated 7 times

    D100-D1FF xff     unused

    ********* POKEY   ********************************
    D200      audf1   frequency audio chan #1
    D201      audc1   control audio chan #1
    D202      audf2   frequency audio chan #2
    D203      audc2   control audio chan #2
    D204      audf3   frequency audio chan #3
    D205      audc3   control audio chan #3
    D206      audf4   frequency audio chan #4
    D207      audc4   control audio chan #4
    D208      audctl  audio control
    D209      stimer  start timer
    D20A      skres   sk reset
    D20B      potgo   start pot AD conversion
    D20C      xff     unused
    D20D      serout  serial output
    D20E      irqen   IRQ enable
    D20F      skctl   sk control
    D210-D2FF repeated 15 times

    ********* PIO     ********************************
    D300      porta   write pio port A (output or mask)
    D301      portb   write pio port B (output or mask)
    D302      pactl   write pio port A control
    D303      pbctl   write pio port B control
    D304-D3FF         repeated

    ********* ANTIC   ********************************
    D400      dmactl  write DMA control
    D401      chactl  write character control
    D402      dlistl  write display list lo
    D403      dlisth  write display list hi
    D404      hscrol  write horz scroll
    D405      vscrol  write vert scroll
    D406      xff     unused
    D407      pmbash  player/missile base addr hi
    D408      xff     unused
    D409      chbash  character generator base addr hi
    D40A      wsync   wait for hsync
    D40B      xff     unused
    D40C      xff     unused
    D40D      xff     unused
    D40E      nmien   NMI enable
    D40F      nmires  NMI reset

    D500-D7FF xff     unused memory

    D800-DFFF ROM     floating point ROM
    E000-FFFF ROM     BIOS ROM
******************************************************************************/

class a400_state : public driver_device
{
public:
	a400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	DECLARE_DRIVER_INIT(xegs);
	DECLARE_DRIVER_INIT(a800xl);
	DECLARE_DRIVER_INIT(a600xl);
	virtual void palette_init();
	DECLARE_WRITE8_MEMBER(a1200xl_pia_pb_w);
	DECLARE_WRITE8_MEMBER(a800xl_pia_pb_w);
	DECLARE_WRITE8_MEMBER(xegs_pia_pb_w);
};

/**************************************************************
 *
 * Memory maps
 *
 **************************************************************/


static ADDRESS_MAP_START(a400_mem, AS_PROGRAM, 8, a400_state)
	AM_RANGE(0x0000, 0x9fff) AM_NOP	/* RAM installed at runtime */
	AM_RANGE(0xa000, 0xbfff) AM_RAMBANK("a000")
	AM_RANGE(0xc000, 0xcfff) AM_ROM
	AM_RANGE(0xd000, 0xd0ff) AM_READWRITE_LEGACY(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd100, 0xd1ff) AM_NOP
	AM_RANGE(0xd200, 0xd2ff) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0xd300, 0xd3ff) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xd400, 0xd4ff) AM_READWRITE_LEGACY(atari_antic_r, atari_antic_w)
	AM_RANGE(0xd500, 0xd7ff) AM_NOP
	AM_RANGE(0xd800, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(a800_mem, AS_PROGRAM, 8, a400_state)
	AM_RANGE(0x0000, 0x7fff) AM_RAMBANK("0000")
	AM_RANGE(0x8000, 0x9fff) AM_RAMBANK("8000")
	AM_RANGE(0xa000, 0xbfff) AM_RAMBANK("a000")
	AM_RANGE(0xc000, 0xcfff) AM_ROM
	AM_RANGE(0xd000, 0xd0ff) AM_READWRITE_LEGACY(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd100, 0xd1ff) AM_NOP
	AM_RANGE(0xd200, 0xd2ff) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0xd300, 0xd3ff) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xd400, 0xd4ff) AM_READWRITE_LEGACY(atari_antic_r, atari_antic_w)
	AM_RANGE(0xd500, 0xd7ff) AM_NOP
	AM_RANGE(0xd800, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(a600xl_mem, AS_PROGRAM, 8, a400_state)
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x5000, 0x57ff) AM_ROM AM_REGION("maincpu", 0x5000)	/* self test */
	AM_RANGE(0xa000, 0xbfff) AM_ROM	/* BASIC */
	AM_RANGE(0xc000, 0xcfff) AM_ROM /* OS */
	AM_RANGE(0xd000, 0xd0ff) AM_READWRITE_LEGACY(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd100, 0xd1ff) AM_NOP
	AM_RANGE(0xd200, 0xd2ff) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0xd300, 0xd3ff) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xd400, 0xd4ff) AM_READWRITE_LEGACY(atari_antic_r, atari_antic_w)
	AM_RANGE(0xd500, 0xd7ff) AM_NOP
	AM_RANGE(0xd800, 0xffff) AM_ROM /* OS */
ADDRESS_MAP_END


static ADDRESS_MAP_START(a800xl_mem, AS_PROGRAM, 8, a400_state)
	AM_RANGE(0x0000, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x57ff) AM_RAMBANK("bank2")
	AM_RANGE(0x5800, 0x9fff) AM_RAM
	AM_RANGE(0xa000, 0xbfff) AM_RAMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAMBANK("bank3")
	AM_RANGE(0xd000, 0xd0ff) AM_READWRITE_LEGACY(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd100, 0xd1ff) AM_NOP
	AM_RANGE(0xd200, 0xd2ff) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0xd300, 0xd3ff) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xd400, 0xd4ff) AM_READWRITE_LEGACY(atari_antic_r, atari_antic_w)
	AM_RANGE(0xd500, 0xd7ff) AM_NOP
	AM_RANGE(0xd800, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START(xegs_mem, AS_PROGRAM, 8, a400_state)
	AM_RANGE(0x0000, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x57ff) AM_RAMBANK("bank2")
	AM_RANGE(0x5800, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0x9fff) AM_ROMBANK("bank0")
	AM_RANGE(0xa000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAMBANK("bank3")
	AM_RANGE(0xd000, 0xd0ff) AM_READWRITE_LEGACY(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd100, 0xd1ff) AM_NOP
	AM_RANGE(0xd200, 0xd2ff) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0xd300, 0xd3ff) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xd400, 0xd4ff) AM_READWRITE_LEGACY(atari_antic_r, atari_antic_w)
	AM_RANGE(0xd500, 0xd7ff) AM_NOP
	AM_RANGE(0xd800, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END


static ADDRESS_MAP_START(a5200_mem, AS_PROGRAM, 8, a400_state)
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc0ff) AM_READWRITE_LEGACY(atari_gtia_r, atari_gtia_w)
	AM_RANGE(0xd400, 0xd5ff) AM_READWRITE_LEGACY(atari_antic_r, atari_antic_w)
	AM_RANGE(0xe800, 0xe8ff) AM_DEVREADWRITE("pokey", pokey_device, read, write)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END



/**************************************************************
 *
 * Input ports
 *
 **************************************************************/


#define JOYSTICK_DELTA			10
#define JOYSTICK_SENSITIVITY	200

static INPUT_PORTS_START( atari_artifacting )
	PORT_START("artifacts")
	PORT_CONFNAME(0x40, 0x00, "Television Artifacts" )
	PORT_CONFSETTING(0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(0x40, DEF_STR( On ) )
INPUT_PORTS_END



static INPUT_PORTS_START( atari_console )
	PORT_START("console")
	PORT_BIT(0x04, 0x04, IPT_KEYBOARD) PORT_NAME("CONS.2: Option") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, 0x02, IPT_KEYBOARD) PORT_NAME("CONS.1: Select") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x01, 0x01, IPT_KEYBOARD) PORT_NAME("CONS.0: Start") PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END



static INPUT_PORTS_START( atari_digital_joystick2 )
	PORT_START("djoy_0_1")	/* IN1 digital joystick #1 + #2 (PIA port A) */
	PORT_BIT(0x01, 0x01, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1)
	PORT_BIT(0x04, 0x04, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(1)
	PORT_BIT(0x08, 0x08, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)
	PORT_BIT(0x10, 0x10, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(2)
	PORT_BIT(0x20, 0x20, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(2)
	PORT_BIT(0x80, 0x80, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)

	PORT_START("djoy_2_3")	/* IN2 digital joystick #3 + #4 (PIA port B) */
	PORT_BIT(0x01, 0x01, IPT_UNUSED)
	PORT_BIT(0x02, 0x02, IPT_UNUSED)
	PORT_BIT(0x04, 0x04, IPT_UNUSED)
	PORT_BIT(0x08, 0x08, IPT_UNUSED)
	PORT_BIT(0x10, 0x10, IPT_UNUSED)
	PORT_BIT(0x20, 0x20, IPT_UNUSED)
	PORT_BIT(0x40, 0x40, IPT_UNUSED)
	PORT_BIT(0x80, 0x80, IPT_UNUSED)

	PORT_START("djoy_b")	/* IN3 digital joystick buttons (GTIA button bits) */
	PORT_BIT(0x01, 0x01, IPT_BUTTON1) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_BUTTON1) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x04, 0x04, IPT_UNUSED)
	PORT_BIT(0x08, 0x08, IPT_UNUSED)
	PORT_BIT(0x10, 0x10, IPT_BUTTON2) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, 0x20, IPT_BUTTON2) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_UNUSED)
	PORT_BIT(0x80, 0x80, IPT_UNUSED)
INPUT_PORTS_END



static INPUT_PORTS_START( atari_digital_joystick4 )
	PORT_START("djoy_0_1")	/* digital joystick #1 + #2 (PIA port A) */
	PORT_BIT(0x01, 0x01, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(1)
	PORT_BIT(0x04, 0x04, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(1)
	PORT_BIT(0x08, 0x08, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(1)
	PORT_BIT(0x10, 0x10, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(2)
	PORT_BIT(0x20, 0x20, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(2)
	PORT_BIT(0x80, 0x80, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(2)

	PORT_START("djoy_2_3")	/* digital joystick #3 + #4 (PIA port B) */
	PORT_BIT(0x01, 0x01, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(3)
	PORT_BIT(0x02, 0x02, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(3)
	PORT_BIT(0x04, 0x04, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(3)
	PORT_BIT(0x08, 0x08, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(3)
	PORT_BIT(0x10, 0x10, IPT_JOYSTICK_UP)    PORT_CODE(KEYCODE_8_PAD) PORT_CODE(JOYCODE_Y_UP_SWITCH)    PORT_PLAYER(4)
	PORT_BIT(0x20, 0x20, IPT_JOYSTICK_DOWN)  PORT_CODE(KEYCODE_2_PAD) PORT_CODE(JOYCODE_Y_DOWN_SWITCH)  PORT_PLAYER(4)
	PORT_BIT(0x40, 0x40, IPT_JOYSTICK_LEFT)  PORT_CODE(KEYCODE_4_PAD) PORT_CODE(JOYCODE_X_LEFT_SWITCH)  PORT_PLAYER(4)
	PORT_BIT(0x80, 0x80, IPT_JOYSTICK_RIGHT) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(JOYCODE_X_RIGHT_SWITCH) PORT_PLAYER(4)

	PORT_START("djoy_b")	/* digital joystick buttons (GTIA button bits) */
	PORT_BIT(0x01, 0x01, IPT_BUTTON1) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_BUTTON1) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x04, 0x04, IPT_BUTTON1) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON3) PORT_PLAYER(3)
	PORT_BIT(0x08, 0x08, IPT_BUTTON1) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(JOYCODE_BUTTON4) PORT_PLAYER(4)
	PORT_BIT(0x10, 0x10, IPT_BUTTON2) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, 0x20, IPT_BUTTON2) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_BUTTON2) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON3) PORT_PLAYER(3)
	PORT_BIT(0x80, 0x80, IPT_BUTTON2) PORT_CODE(KEYCODE_DEL_PAD) PORT_CODE(JOYCODE_BUTTON4) PORT_PLAYER(4)
INPUT_PORTS_END



/* 2009-04 FP:
Small note about natural keyboard support: currently,
- "Break" is mapped to 'F1'
- "Clear" is mapped to 'F2'
- "Atari" is mapped to 'F3'                         */

static INPUT_PORTS_START( atari_keyboard )
	PORT_START("keyboard_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('+') PORT_CHAR('\\')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('*') PORT_CHAR('^')

	PORT_START("keyboard_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) // None!
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('=') PORT_CHAR('|')

	PORT_START("keyboard_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("keyboard_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("keyboard_4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR(']')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Atari") PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("keyboard_5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("keyboard_6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BackS  Delete") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(8) PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('@')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<  Clear") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('<') PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(">  Insert") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('>') PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("keyboard_7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("fake")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
INPUT_PORTS_END


static INPUT_PORTS_START( atari_analog_paddles )
	PORT_START("analog_0") /* IN8 analog in #1 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(1) PORT_REVERSE

	PORT_START("analog_1") /* IN9 analog in #2 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(2) PORT_REVERSE

	PORT_START("analog_2") /* IN10 analog in #3 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(3) PORT_REVERSE

	PORT_START("analog_3") /* IN11 analog in #4 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(4) PORT_REVERSE

	PORT_START("analog_4") /* IN12 analog in #5 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_REVERSE /* PORT_PLAYER(5) */

	PORT_START("analog_5") /* IN13 analog in #6 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_REVERSE /* PORT_PLAYER(6) */

	PORT_START("analog_6") /* IN14 analog in #7 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_REVERSE /* PORT_PLAYER(7) */

	PORT_START("analog_7") /* IN15 analog in #8 */
	PORT_BIT(0xff, 0x74, IPT_PADDLE) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_REVERSE /* PORT_PLAYER(8) */
INPUT_PORTS_END



static INPUT_PORTS_START( a800 )
	PORT_INCLUDE( atari_artifacting )
	PORT_INCLUDE( atari_console )
	PORT_INCLUDE( atari_digital_joystick4 )
	PORT_INCLUDE( atari_keyboard )
	PORT_INCLUDE( atari_analog_paddles )
INPUT_PORTS_END



static INPUT_PORTS_START( a800xl )
	PORT_INCLUDE( atari_artifacting )
	PORT_INCLUDE( atari_console )
	PORT_INCLUDE( atari_digital_joystick2 )
	PORT_INCLUDE( atari_keyboard )
	PORT_INCLUDE( atari_analog_paddles )
INPUT_PORTS_END



static INPUT_PORTS_START( a5200 )
	PORT_INCLUDE( atari_artifacting )

	PORT_START("djoy_b")	/* lower/upper buttons */
	PORT_BIT(0x01, 0x01, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x04, 0x04, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x08, 0x08, IPT_BUTTON1) PORT_PLAYER(4)
	PORT_BIT(0x10, 0x10, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x20, 0x20, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_BUTTON2) PORT_PLAYER(3)
	PORT_BIT(0x80, 0x80, IPT_BUTTON2) PORT_PLAYER(4)

	PORT_START("keypad_0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("(Break)") PORT_CODE(KEYCODE_PAUSE)	// is this correct?
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[#]") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[0]") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[*]") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keypad_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[9]") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[8]") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[7]") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keypad_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(DEF_STR(Pause)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[6]") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[5]") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[4]") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("keypad_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_START)    PORT_NAME("Start")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[3]") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[2]") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("[1]") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("analog_0")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(1)

	PORT_START("analog_1")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(1)

	PORT_START("analog_2")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(2)

	PORT_START("analog_3")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(2)

	PORT_START("analog_4")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(3)

	PORT_START("analog_5")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(3)

	PORT_START("analog_6")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_X) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(4)

	PORT_START("analog_7")
	PORT_BIT(0xff, 0x72, IPT_AD_STICK_Y) PORT_SENSITIVITY(JOYSTICK_SENSITIVITY) PORT_KEYDELTA(JOYSTICK_DELTA) PORT_MINMAX(0x00,0xe4) PORT_PLAYER(4)

INPUT_PORTS_END


/**************************************************************
 *
 * Palette
 *
 **************************************************************/

static const UINT8 atari_palette[256*3] =
{
	/* Grey */
	0x00,0x00,0x00, 0x25,0x25,0x25, 0x34,0x34,0x34, 0x4e,0x4e,0x4e,
	0x68,0x68,0x68, 0x75,0x75,0x75, 0x8e,0x8e,0x8e, 0xa4,0xa4,0xa4,
	0xb8,0xb8,0xb8, 0xc5,0xc5,0xc5, 0xd0,0xd0,0xd0, 0xd7,0xd7,0xd7,
	0xe1,0xe1,0xe1, 0xea,0xea,0xea, 0xf4,0xf4,0xf4, 0xff,0xff,0xff,
	/* Gold */
	0x41,0x20,0x00, 0x54,0x28,0x00, 0x76,0x37,0x00, 0x9a,0x50,0x00,
	0xc3,0x68,0x06, 0xe4,0x7b,0x07, 0xff,0x91,0x1a, 0xff,0xab,0x1d,
	0xff,0xc5,0x1f, 0xff,0xd0,0x3b, 0xff,0xd8,0x4c, 0xff,0xe6,0x51,
	0xff,0xf4,0x56, 0xff,0xf9,0x70, 0xff,0xff,0x90, 0xff,0xff,0xaa,
	/* Orange */
	0x45,0x19,0x04, 0x72,0x1e,0x11, 0x9f,0x24,0x1e, 0xb3,0x3a,0x20,
	0xc8,0x51,0x20, 0xe3,0x69,0x20, 0xfc,0x81,0x20, 0xfd,0x8c,0x25,
	0xfe,0x98,0x2c, 0xff,0xae,0x38, 0xff,0xb9,0x46, 0xff,0xbf,0x51,
	0xff,0xc6,0x6d, 0xff,0xd5,0x87, 0xff,0xe4,0x98, 0xff,0xe6,0xab,
	/* Red-Orange */
	0x5d,0x1f,0x0c, 0x7a,0x24,0x0d, 0x98,0x2c,0x0e, 0xb0,0x2f,0x0f,
	0xbf,0x36,0x24, 0xd3,0x4e,0x2a, 0xe7,0x62,0x3e, 0xf3,0x6e,0x4a,
	0xfd,0x78,0x54, 0xff,0x8a,0x6a, 0xff,0x98,0x7c, 0xff,0xa4,0x8b,
	0xff,0xb3,0x9e, 0xff,0xc2,0xb2, 0xff,0xd0,0xc3, 0xff,0xda,0xd0,
	/* Pink */
	0x4a,0x17,0x00, 0x72,0x1f,0x00, 0xa8,0x13,0x00, 0xc8,0x21,0x0a,
	0xdf,0x25,0x12, 0xec,0x3b,0x24, 0xfa,0x52,0x36, 0xfc,0x61,0x48,
	0xff,0x70,0x5f, 0xff,0x7e,0x7e, 0xff,0x8f,0x8f, 0xff,0x9d,0x9e,
	0xff,0xab,0xad, 0xff,0xb9,0xbd, 0xff,0xc7,0xce, 0xff,0xca,0xde,
	/* Purple */
	0x49,0x00,0x36, 0x66,0x00,0x4b, 0x80,0x03,0x5f, 0x95,0x0f,0x74,
	0xaa,0x22,0x88, 0xba,0x3d,0x99, 0xca,0x4d,0xa9, 0xd7,0x5a,0xb6,
	0xe4,0x67,0xc3, 0xef,0x72,0xce, 0xfb,0x7e,0xda, 0xff,0x8d,0xe1,
	0xff,0x9d,0xe5, 0xff,0xa5,0xe7, 0xff,0xaf,0xea, 0xff,0xb8,0xec,
	/* Purple-Blue */
	0x48,0x03,0x6c, 0x5c,0x04,0x88, 0x65,0x0d,0x90, 0x7b,0x23,0xa7,
	0x93,0x3b,0xbf, 0x9d,0x45,0xc9, 0xa7,0x4f,0xd3, 0xb2,0x5a,0xde,
	0xbd,0x65,0xe9, 0xc5,0x6d,0xf1, 0xce,0x76,0xfa, 0xd5,0x83,0xff,
	0xda,0x90,0xff, 0xde,0x9c,0xff, 0xe2,0xa9,0xff, 0xe6,0xb6,0xff,
	/* Blue 1 */
	0x05,0x1e,0x81, 0x06,0x26,0xa5, 0x08,0x2f,0xca, 0x26,0x3d,0xd4,
	0x44,0x4c,0xde, 0x4f,0x5a,0xec, 0x5a,0x68,0xff, 0x65,0x75,0xff,
	0x71,0x83,0xff, 0x80,0x91,0xff, 0x90,0xa0,0xff, 0x97,0xa9,0xff,
	0x9f,0xb2,0xff, 0xaf,0xbe,0xff, 0xc0,0xcb,0xff, 0xcd,0xd3,0xff,
	/* Blue 2 */
	0x0b,0x07,0x79, 0x20,0x1c,0x8e, 0x35,0x31,0xa3, 0x46,0x42,0xb4,
	0x57,0x53,0xc5, 0x61,0x5d,0xcf, 0x6d,0x69,0xdb, 0x7b,0x77,0xe9,
	0x89,0x85,0xf7, 0x91,0x8d,0xff, 0x9c,0x98,0xff, 0xa7,0xa4,0xff,
	0xb2,0xaf,0xff, 0xbb,0xb8,0xff, 0xc3,0xc1,0xff, 0xd3,0xd1,0xff,
	/* Light-Blue */
	0x1d,0x29,0x5a, 0x1d,0x38,0x76, 0x1d,0x48,0x92, 0x1d,0x5c,0xac,
	0x1d,0x71,0xc6, 0x32,0x86,0xcf, 0x48,0x9b,0xd9, 0x4e,0xa8,0xec,
	0x55,0xb6,0xff, 0x69,0xca,0xff, 0x74,0xcb,0xff, 0x82,0xd3,0xff,
	0x8d,0xda,0xff, 0x9f,0xd4,0xff, 0xb4,0xe2,0xff, 0xc0,0xeb,0xff,
	/* Turquoise */
	0x00,0x4b,0x59, 0x00,0x5d,0x6e, 0x00,0x6f,0x84, 0x00,0x84,0x9c,
	0x00,0x99,0xbf, 0x00,0xab,0xca, 0x00,0xbc,0xde, 0x00,0xd0,0xf5,
	0x10,0xdc,0xff, 0x3e,0xe1,0xff, 0x64,0xe7,0xff, 0x76,0xea,0xff,
	0x8b,0xed,0xff, 0x9a,0xef,0xff, 0xb1,0xf3,0xff, 0xc7,0xf6,0xff,
	/* Green-Blue */
	0x00,0x48,0x00, 0x00,0x54,0x00, 0x03,0x6b,0x03, 0x0e,0x76,0x0e,
	0x18,0x80,0x18, 0x27,0x92,0x27, 0x36,0xa4,0x36, 0x4e,0xb9,0x4e,
	0x51,0xcd,0x51, 0x72,0xda,0x72, 0x7c,0xe4,0x7c, 0x85,0xed,0x85,
	0x99,0xf2,0x99, 0xb3,0xf7,0xb3, 0xc3,0xf9,0xc3, 0xcd,0xfc,0xcd,
	/* Green */
	0x16,0x40,0x00, 0x1c,0x53,0x00, 0x23,0x66,0x00, 0x28,0x78,0x00,
	0x2e,0x8c,0x00, 0x3a,0x98,0x0c, 0x47,0xa5,0x19, 0x51,0xaf,0x23,
	0x5c,0xba,0x2e, 0x71,0xcf,0x43, 0x85,0xe3,0x57, 0x8d,0xeb,0x5f,
	0x97,0xf5,0x69, 0xa0,0xfe,0x72, 0xb1,0xff,0x8a, 0xbc,0xff,0x9a,
	/* Yellow-Green */
	0x2c,0x35,0x00, 0x38,0x44,0x00, 0x44,0x52,0x00, 0x49,0x56,0x00,
	0x60,0x71,0x00, 0x6c,0x7f,0x00, 0x79,0x8d,0x0a, 0x8b,0x9f,0x1c,
	0x9e,0xb2,0x2f, 0xab,0xbf,0x3c, 0xb8,0xcc,0x49, 0xc2,0xd6,0x53,
	0xcd,0xe1,0x53, 0xdb,0xef,0x6c, 0xe8,0xfc,0x79, 0xf2,0xff,0xab,
	/* Orange-Green */
	0x46,0x3a,0x09, 0x4d,0x3f,0x09, 0x54,0x45,0x09, 0x6c,0x58,0x09,
	0x90,0x76,0x09, 0xab,0x8b,0x0a, 0xc1,0xa1,0x20, 0xd0,0xb0,0x2f,
	0xde,0xbe,0x3d, 0xe6,0xc6,0x45, 0xed,0xcd,0x4c, 0xf5,0xd8,0x62,
	0xfb,0xe2,0x76, 0xfc,0xee,0x98, 0xfd,0xf3,0xa9, 0xfd,0xf3,0xbe,
	/* Light-Orange */
	0x40,0x1a,0x02, 0x58,0x1f,0x05, 0x70,0x24,0x08, 0x8d,0x3a,0x13,
	0xab,0x51,0x1f, 0xb5,0x64,0x27, 0xbf,0x77,0x30, 0xd0,0x85,0x3a,
	0xe1,0x93,0x44, 0xed,0xa0,0x4e, 0xf9,0xad,0x58, 0xfc,0xb7,0x5c,
	0xff,0xc1,0x60, 0xff,0xca,0x69, 0xff,0xcf,0x7e, 0xff,0xda,0x96
};


/* Initialise the palette */
void a400_state::palette_init()
{
	int i;

	for ( i = 0; i < sizeof(atari_palette) / 3; i++ )
	{
		palette_set_color_rgb(machine(), i, atari_palette[i*3], atari_palette[i*3+1], atari_palette[i*3+2]);
	}
}

/**************************************************************
 *
 * Memory banking
 *
 **************************************************************/

static void a800xl_mmu(running_machine &machine, UINT8 new_mmu)
{
	UINT8 *base = machine.root_device().memregion("maincpu")->base();
	UINT8 *base1, *base2, *base3, *base4;

	/* check if memory C000-FFFF changed */
	if( new_mmu & 0x01 )
	{
		logerror("%s MMU BIOS ROM\n", machine.system().name);
		base3 = base + 0x14000;  /* 8K lo BIOS */
		base4 = base + 0x15800;  /* 4K FP ROM + 8K hi BIOS */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0xc000, 0xcfff, "bank3");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0xc000, 0xcfff);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0xd800, 0xffff, "bank4");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0xd800, 0xffff);
	}
	else
	{
		logerror("%s MMU BIOS RAM\n", machine.system().name);
		base3 = base + 0x0c000;  /* 8K RAM */
		base4 = base + 0x0d800;  /* 4K RAM + 8K RAM */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xc000, 0xcfff, "bank3");
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xd800, 0xffff, "bank4");
	}
	machine.root_device().membank("bank3")->set_base(base3);
	machine.root_device().membank("bank4")->set_base(base4);

	/* check if BASIC changed */
	if( new_mmu & 0x02 )
	{
		logerror("%s MMU BASIC RAM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xa000, 0xbfff, "bank1");
		base1 = base + 0x0a000;  /* 8K RAM */
	}
	else
	{
		logerror("%s MMU BASIC ROM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0xa000, 0xbfff, "bank1");
		machine.device("maincpu")->memory().space(AS_PROGRAM).nop_write(0xa000, 0xbfff);
		base1 = base + 0x10000;  /* 8K BASIC */
	}

	machine.root_device().membank("bank1")->set_base(base1);

	/* check if self-test ROM changed */
	if( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0x5000, 0x57ff, "bank2");
		base2 = base + 0x05000;  /* 0x0800 bytes */
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0x5000, 0x57ff, "bank2");
		machine.device("maincpu")->memory().space(AS_PROGRAM).nop_write(0x5000, 0x57ff);
		base2 = base + 0x15000;  /* 0x0800 bytes */
	}
	machine.root_device().membank("bank2")->set_base(base2);
}

/* BASIC was available in a separate cart, so we don't test it */
static void a1200xl_mmu(running_machine &machine, UINT8 new_mmu)
{
	UINT8 *base = machine.root_device().memregion("maincpu")->base();
	UINT8 *base2, *base3, *base4;

	/* check if memory C000-FFFF changed */
	if( new_mmu & 0x01 )
	{
		logerror("%s MMU BIOS ROM\n", machine.system().name);
		base3 = base + 0x14000;  /* 8K lo BIOS */
		base4 = base + 0x15800;  /* 4K FP ROM + 8K hi BIOS */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0xc000, 0xcfff, "bank3");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0xc000, 0xcfff);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0xd800, 0xffff, "bank4");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0xd800, 0xffff);
	}
	else
	{
		logerror("%s MMU BIOS RAM\n", machine.system().name);
		base3 = base + 0x0c000;  /* 8K RAM */
		base4 = base + 0x0d800;  /* 4K RAM + 8K RAM */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xc000, 0xcfff, "bank3");
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xd800, 0xffff, "bank4");
	}
	machine.root_device().membank("bank3")->set_base(base3);
	machine.root_device().membank("bank4")->set_base(base4);

	/* check if self-test ROM changed */
	if( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", machine.system().name);
		base2 = base + 0x05000;  /* 0x0800 bytes */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0x5000, 0x57ff, "bank2");
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine.system().name);
		base2 = base + 0x15000;  /* 0x0800 bytes */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0x5000, 0x57ff, "bank2");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0x5000, 0x57ff);
	}
	machine.root_device().membank("bank2")->set_base(base2);
}

static void xegs_mmu(running_machine &machine, UINT8 new_mmu)
{
	UINT8 *base = machine.root_device().memregion("maincpu")->base();
	UINT8 *base2, *base3, *base4;

	/* check if memory C000-FFFF changed */
	if( new_mmu & 0x01 )
	{
		logerror("%s MMU BIOS ROM\n", machine.system().name);
		base3 = base + 0x14000;  /* 8K lo BIOS */
		base4 = base + 0x15800;  /* 4K FP ROM + 8K hi BIOS */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0xc000, 0xcfff, "bank3");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0xc000, 0xcfff);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0xd800, 0xffff, "bank4");
		machine.device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0xd800, 0xffff);
	}
	else
	{
		logerror("%s MMU BIOS RAM\n", machine.system().name);
		base3 = base + 0x0c000;  /* 8K RAM */
		base4 = base + 0x0d800;  /* 4K RAM + 8K RAM */
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xc000, 0xcfff, "bank3");
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0xd800, 0xffff, "bank4");
	}
	machine.root_device().membank("bank3")->set_base(base3);
	machine.root_device().membank("bank4")->set_base(base4);


	/* check if self-test ROM changed */
	if( new_mmu & 0x80 )
	{
		logerror("%s MMU SELFTEST RAM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_readwrite_bank(0x5000, 0x57ff, "bank2");
		base2 = base + 0x05000;  /* 0x0800 bytes */
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine.system().name);
		machine.device("maincpu")->memory().space(AS_PROGRAM).install_read_bank(0x5000, 0x57ff, "bank2");
		machine.device("maincpu")->memory().space(AS_PROGRAM).nop_write(0x5000, 0x57ff);
		base2 = base + 0x15000;  /* 0x0800 bytes */
	}
	machine.root_device().membank("bank2")->set_base(base2);
}

/**************************************************************
 *
 * PIA interface
 *
 **************************************************************/

WRITE8_MEMBER(a400_state::a1200xl_pia_pb_w){ device_t *device = machine().device("pia");  a1200xl_mmu(device->machine(), data); }
WRITE8_MEMBER(a400_state::a800xl_pia_pb_w)
{
	device_t *device = machine().device("pia");
	if (downcast<pia6821_device *>(device)->port_b_z_mask() != 0xff)
		a800xl_mmu(machine(), data);
}

WRITE8_MEMBER(a400_state::xegs_pia_pb_w)
{
	device_t *device = machine().device("pia");
	if (downcast<pia6821_device *>(device)->port_b_z_mask() != 0xff)
		xegs_mmu(machine(), data);
}

static const pokey_interface atari_pokey_interface =
{
	{
		DEVCB_INPUT_PORT("analog_0"),
		DEVCB_INPUT_PORT("analog_1"),
		DEVCB_INPUT_PORT("analog_2"),
		DEVCB_INPUT_PORT("analog_3"),
		DEVCB_INPUT_PORT("analog_4"),
		DEVCB_INPUT_PORT("analog_5"),
		DEVCB_INPUT_PORT("analog_6"),
		DEVCB_INPUT_PORT("analog_7")
	},
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("fdc", atari_serin_r),
	DEVCB_DEVICE_HANDLER("fdc", atari_serout_w)
};


static const pia6821_interface atari_pia_interface =
{
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pa_r),		/* port A in */
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DEVICE_LINE("fdc",atarifdc_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface a600xl_pia_interface =
{
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pa_r),		/* port A in */
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DEVICE_HANDLER("pia", a600xl_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DEVICE_LINE("fdc",atarifdc_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface a1200xl_pia_interface =
{
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pa_r),		/* port A in */
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DRIVER_MEMBER(a400_state, a1200xl_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DEVICE_LINE("fdc",atarifdc_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface a800xl_pia_interface =
{
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pa_r),		/* port A in */
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DRIVER_MEMBER(a400_state,a800xl_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DEVICE_LINE("fdc",atarifdc_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

static const pia6821_interface xegs_pia_interface =
{
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pa_r),		/* port A in */
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_DRIVER_MEMBER(a400_state,xegs_pia_pb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_DEVICE_LINE("fdc",atarifdc_pia_cb2_w),		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

// FIXME: should there be anything connected where other system have the fdc?
static const pokey_interface a5200_pokey_interface =
{
	{
		DEVCB_INPUT_PORT("analog_0"),
		DEVCB_INPUT_PORT("analog_1"),
		DEVCB_INPUT_PORT("analog_2"),
		DEVCB_INPUT_PORT("analog_3"),
		DEVCB_INPUT_PORT("analog_4"),
		DEVCB_INPUT_PORT("analog_5"),
		DEVCB_INPUT_PORT("analog_6"),
		DEVCB_INPUT_PORT("analog_7")
	},
	DEVCB_NULL,
	DEVCB_NULL,	// FIXME: is there anything connected here?
	DEVCB_NULL	// FIXME: is there anything connected here?
};

static const pia6821_interface a5200_pia_interface =
{
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pa_r),		/* port A in */
	DEVCB_DEVICE_HANDLER("pia", atari_pia_pb_r),	/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_NULL,		/* port A out */
	DEVCB_NULL,		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */	// FIXME: is there anything connected here
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};


/**************************************************************
 *
 * Machine drivers
 *
 **************************************************************/

static MACHINE_CONFIG_FRAGMENT( a400_cartslot )
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(a800_cart)
	MCFG_CARTSLOT_UNLOAD(a800_cart)
	MCFG_CARTSLOT_INTERFACE("a800_cart")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( a800_cartslot )
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(a800_cart)
	MCFG_CARTSLOT_UNLOAD(a800_cart)
	MCFG_CARTSLOT_INTERFACE("a800_cart")

	MCFG_CARTSLOT_ADD("cart2")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(a800_cart_right)
	MCFG_CARTSLOT_UNLOAD(a800_cart_right)
	MCFG_CARTSLOT_INTERFACE("a800_cart")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( atari_common_nodac, a400_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, FREQ_17_EXACT)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(1))
	MCFG_SCREEN_VISIBLE_AREA(MIN_X, MAX_X, MIN_Y, MAX_Y)
	MCFG_PALETTE_LENGTH(sizeof(atari_palette) / 3)
	MCFG_SCREEN_UPDATE_STATIC(atari)

	MCFG_VIDEO_START(atari)

	MCFG_PIA6821_ADD( "pia", atari_pia_interface )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_POKEY_ADD("pokey", FREQ_17_EXACT)
	MCFG_POKEY_CONFIG(atari_pokey_interface)
	MCFG_POKEY_KEYBOARD_HANDLER(atari_a800_keyboard)
	MCFG_POKEY_INTERRUPT_HANDLER(atari_interrupt_cb)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("40K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( atari_common, atari_common_nodac )
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_ATARI_FDC_ADD("fdc")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a400, atari_common )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(a400_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", a400_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START( a400 )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)

	MCFG_FRAGMENT_ADD(a400_cartslot)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a800")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a400pal, atari_common )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(a400_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", a400_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START( a400 )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_50HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_50HZ)

	MCFG_FRAGMENT_ADD(a400_cartslot)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a800")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a800, atari_common )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(a800_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", a800_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START( a800 )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)

	MCFG_FRAGMENT_ADD(a800_cartslot)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a800")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a800pal, atari_common )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(a800_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", a800_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START( a800 )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_50HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_50HZ)

	MCFG_FRAGMENT_ADD(a800_cartslot)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a800")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a600xl, atari_common )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(a600xl_mem)	// FIXME?
	MCFG_TIMER_ADD_SCANLINE("scantimer", a800xl_interrupt, "screen", 0, 1)

	MCFG_PIA6821_MODIFY( "pia", a600xl_pia_interface )

	MCFG_MACHINE_START( a800xl )	// FIXME?

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)

	MCFG_FRAGMENT_ADD(a400_cartslot)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a800")

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a800xl, atari_common )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(a800xl_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", a800xl_interrupt, "screen", 0, 1)

	MCFG_PIA6821_MODIFY( "pia", a800xl_pia_interface )

	MCFG_MACHINE_START( a800xl )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)

	MCFG_FRAGMENT_ADD(a400_cartslot)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a800")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a800xlpal, a800xl )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_CLOCK( 1773000 )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_50HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_50HZ)

	MCFG_SOUND_MODIFY("pokey")
	MCFG_SOUND_CLOCK(1773000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( a1200xl, a800xl )

	MCFG_PIA6821_MODIFY( "pia", a1200xl_pia_interface )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( xegs, a800xl )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(xegs_mem)

	MCFG_MACHINE_START( xegs )

	MCFG_PIA6821_MODIFY( "pia", xegs_pia_interface )

	MCFG_DEVICE_REMOVE("cart1")
	MCFG_DEVICE_REMOVE("cart_list")

	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(xegs_cart)
	MCFG_CARTSLOT_UNLOAD(xegs_cart)
	MCFG_CARTSLOT_INTERFACE("xegs_cart")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","xegs")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( a5200, atari_common_nodac )

	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP(a5200_mem)
	MCFG_TIMER_ADD_SCANLINE("scantimer", a5200_interrupt, "screen", 0, 1)

	MCFG_DEVICE_REMOVE("pokey")
	MCFG_POKEY_ADD("pokey", FREQ_17_EXACT)
	MCFG_POKEY_CONFIG(a5200_pokey_interface)
	MCFG_POKEY_KEYBOARD_HANDLER(atari_a5200_keypads)
	MCFG_POKEY_INTERRUPT_HANDLER(atari_interrupt_cb)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_PIA6821_MODIFY( "pia", a5200_pia_interface )

	MCFG_MACHINE_START( a5200 )

	MCFG_SCREEN_MODIFY( "screen" )
	MCFG_SCREEN_REFRESH_RATE(FRAME_RATE_60HZ)
	MCFG_SCREEN_SIZE(HWIDTH*8, TOTAL_LINES_60HZ)

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("rom,bin,a52")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(a5200_cart)
	MCFG_CARTSLOT_UNLOAD(a5200_cart)
	MCFG_CARTSLOT_INTERFACE("a5200_cart")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","a5200")

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
MACHINE_CONFIG_END


/**************************************************************
 *
 * ROM loading
 *
 **************************************************************/

ROM_START(a400)
	ROM_REGION(0x14000, "maincpu", 0) /* 64K for the CPU + 2 * 8K for cartridges */
	ROM_LOAD( "co12399b.rom", 0xd800, 0x0800, CRC(6a5d766e) SHA1(01a6044f7a81d409c938e7dfde0a1af5832229d2) )
	ROM_SYSTEM_BIOS(0, "default", "OS Rev. B")
	ROMX_LOAD( "co12499b.rom",  0xe000, 0x1000, BAD_DUMP CRC(d818f3e8) SHA1(bcdec2188f6a6a5bfc1df4e383bd828d34b5c4ac), ROM_BIOS(1) )	// CRC and label waiting for confirmation
	ROMX_LOAD( "co14599b.rom",  0xf000, 0x1000, BAD_DUMP CRC(c1690a9b) SHA1(c5248e8565574fd39ae1c3f4f356aa4cac07df95), ROM_BIOS(1) )	// CRC and label waiting for confirmation
	ROM_SYSTEM_BIOS(1, "reva", "OS Rev. A")
	ROMX_LOAD( "co12499a.rom",  0xe000, 0x1000, BAD_DUMP CRC(29f64e17) SHA1(abf7ec488c6b600f1b7f30bdc7f8a2bf6a727675), ROM_BIOS(2) )	// CRC and label waiting for confirmation
	ROMX_LOAD( "co14599a.rom",  0xf000, 0x1000, BAD_DUMP CRC(bc533f0c) SHA1(e217148495fa747fe5488132d8d22533e68c7e58), ROM_BIOS(2) )	// CRC and label waiting for confirmation

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a400pal)
	ROM_REGION(0x14000, "maincpu", 0) /* 64K for the CPU + 2 * 8K for cartridges */
	ROM_LOAD( "co12399b.rom", 0xd800, 0x0800, CRC(6a5d766e) SHA1(01a6044f7a81d409c938e7dfde0a1af5832229d2) )
	ROM_LOAD( "co15199.rom", 0xe000, 0x1000, BAD_DUMP CRC(8e547f56) SHA1(1bd746ea798b723bfb18495a7facca113183d713) )	// Rev. A - CRC and label waiting for confirmation
	ROM_LOAD( "co15299.rom", 0xf000, 0x1000, BAD_DUMP CRC(be55b413) SHA1(d88afae49b08e75943d0258cb580e5d34756414a) )	// Rev. A - CRC and label waiting for confirmation

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a800)
	ROM_REGION(0x14000, "maincpu", 0) /* 64K for the CPU + 2 * 8K for cartridges */
	ROM_LOAD( "co12399b.rom", 0xd800, 0x0800, CRC(6a5d766e) SHA1(01a6044f7a81d409c938e7dfde0a1af5832229d2) )
	ROM_SYSTEM_BIOS(0, "default", "OS Rev. B")
	ROMX_LOAD( "co12499b.rom",  0xe000, 0x1000, BAD_DUMP CRC(d818f3e8) SHA1(bcdec2188f6a6a5bfc1df4e383bd828d34b5c4ac), ROM_BIOS(1) )	// CRC and label waiting for confirmation
	ROMX_LOAD( "co14599b.rom",  0xf000, 0x1000, BAD_DUMP CRC(c1690a9b) SHA1(c5248e8565574fd39ae1c3f4f356aa4cac07df95), ROM_BIOS(1) )	// CRC and label waiting for confirmation
	ROM_SYSTEM_BIOS(1, "reva", "OS Rev. A")
	ROMX_LOAD( "co12499a.rom",  0xe000, 0x1000, BAD_DUMP CRC(29f64e17) SHA1(abf7ec488c6b600f1b7f30bdc7f8a2bf6a727675), ROM_BIOS(2) )	// CRC and label waiting for confirmation
	ROMX_LOAD( "co14599a.rom",  0xf000, 0x1000, BAD_DUMP CRC(bc533f0c) SHA1(e217148495fa747fe5488132d8d22533e68c7e58), ROM_BIOS(2) )	// CRC and label waiting for confirmation

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)

	ROM_REGION(0x2000, "rslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a800pal)
	ROM_REGION(0x14000, "maincpu", 0) /* 64K for the CPU + 2 * 8K for cartridges */
	ROM_LOAD( "co12399b.rom", 0xd800, 0x0800, CRC(6a5d766e) SHA1(01a6044f7a81d409c938e7dfde0a1af5832229d2) )
	ROM_LOAD( "co15199.rom", 0xe000, 0x1000, BAD_DUMP CRC(8e547f56) SHA1(1bd746ea798b723bfb18495a7facca113183d713) )	// Rev. A - CRC and label waiting for confirmation
	ROM_LOAD( "co15299.rom", 0xf000, 0x1000, BAD_DUMP CRC(be55b413) SHA1(d88afae49b08e75943d0258cb580e5d34756414a) )	// Rev. A - CRC and label waiting for confirmation

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)

	ROM_REGION(0x2000, "rslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a1200xl)
	ROM_REGION(0x18000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "default", "OS Rev. 11")
	ROMX_LOAD( "co60616b.rom", 0x14000, 0x2000, BAD_DUMP CRC(6e29ec8d) SHA1(3f9c06d6b4d261f3d5bf4354e3cff0c17b9347b9), ROM_BIOS(1) )	// CRC and label waiting for confirmation
	ROMX_LOAD( "co60617b.rom", 0x16000, 0x2000, BAD_DUMP CRC(d73ce29a) SHA1(64790242d902643fe0c40dd842749f1fe461831b), ROM_BIOS(1) )	// CRC and label waiting for confirmation
	ROM_SYSTEM_BIOS(1, "rev10", "OS Rev. 10")
	ROMX_LOAD( "co60616a.rom", 0x14000, 0x2000, BAD_DUMP CRC(0391386b) SHA1(7c176657c88b89b8a69bf021fa8e0939efc0dff2), ROM_BIOS(2) )	// CRC and label waiting for confirmation
	ROMX_LOAD( "co60617a.rom", 0x16000, 0x2000, BAD_DUMP CRC(b502f1e7) SHA1(6688db57d97fa570aef5c15cef3e5fb2688879c2), ROM_BIOS(2) )	// CRC and label waiting for confirmation

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a600xl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "co60302a.rom", 0xa000, 0x2000, CRC(f0202fb3) SHA1(7ad88dd99ff4a6ee66f6d162074db6f8bef7a9b6) )	// Rev. B
	ROM_LOAD( "co62024.rom",  0xc000, 0x4000, CRC(643bcc98) SHA1(881d030656b40bbe48f15a696b28f22c0b752ab0) )	// Rev. 1

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a800xl)
	ROM_REGION(0x18000, "maincpu", 0)
	ROM_FILL( 0, 0x10000, 0x00 )
	ROM_LOAD( "co60302a.rom", 0x10000, 0x2000, CRC(f0202fb3) SHA1(7ad88dd99ff4a6ee66f6d162074db6f8bef7a9b6) )	// Rev. B
	ROM_LOAD( "co61598b.rom", 0x14000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55) )	// Rev. 2

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

#define rom_a800xlp rom_a800xl

ROM_START(a65xe)
	ROM_REGION(0x18000, "maincpu", 0)
	ROM_LOAD( "co24947a.rom", 0x10000, 0x2000, CRC(7d684184) SHA1(3693c9cb9bf3b41bae1150f7a8264992468fc8c0) )	// Rev. C
	ROM_LOAD( "co61598b.rom", 0x14000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55) )	// Rev. 2

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a65xea)
	ROM_REGION(0x18000, "maincpu", 0)
	ROM_LOAD( "basic_ar.rom", 0x10000, 0x2000, CRC(c899f4d6) SHA1(043df191d1fe402e792266a108e147ffcda35130) )	// is this correct? or shall we use Rev. C?
//  ROM_LOAD( "c101700.rom",  0x14000, 0x4000, CRC(7f9a76c8) SHA1(57eb6d87850a763f11767f53d4eaede186f831a2) )   // this was from Savetz and has wrong bits!
	ROM_LOAD( "c101700.rom",  0x14000, 0x4000, CRC(45f47988) SHA1(a36b8b20f657580f172749bb0625c08706ed824c) )	// Rev. 3B ?

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a130xe)
	ROM_REGION(0x18000, "maincpu", 0)
	ROM_LOAD( "co24947a.rom", 0x10000, 0x2000, CRC(7d684184) SHA1(3693c9cb9bf3b41bae1150f7a8264992468fc8c0) )	// Rev. C
	ROM_LOAD( "co61598b.rom", 0x14000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55) )	// Rev. 2

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(a800xe)
	ROM_REGION(0x18000, "maincpu", 0)
	ROM_LOAD( "co24947a.rom", 0x10000, 0x2000, CRC(7d684184) SHA1(3693c9cb9bf3b41bae1150f7a8264992468fc8c0) )	// Rev. C
	ROM_LOAD( "c300717.rom",  0x14000, 0x4000, CRC(29f133f7) SHA1(f03b9b93000ee84abb9cf8d6367241006f172182) )	// Rev. 3

	ROM_REGION(0x10000, "lslot", ROMREGION_ERASEFF)
ROM_END

ROM_START(xegs)
	ROM_REGION(0x1a000, "maincpu", 0)
	ROM_LOAD( "c101687.rom", 0x10000, 0x8000, CRC(d50260d1) SHA1(0e0625ab2473f8431640df3ac8af61925760b9b9) )	// Rev. C + Rev. 4 + Missile Command

	ROM_REGION(0x20000, "user1", ROMREGION_ERASE00)
ROM_END


ROM_START(a5200)
	ROM_REGION(0x14000, "maincpu", 0) /* 64K for the CPU + 16K for cartridges */
	ROM_SYSTEM_BIOS(0, "default", "a5200")
	ROMX_LOAD( "5200.rom",  0xf800, 0x0800, CRC(4248d3e3) SHA1(6ad7a1e8c9fad486fbec9498cb48bf5bc3adc530), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS(1, "alt", "a5200 (alt)")
	ROMX_LOAD( "5200a.rom", 0xf800, 0x0800, CRC(c2ba2613) SHA1(1d2a3f00109d75d2d79fecb565775eb95b7d04d5), ROM_BIOS(2) )
ROM_END

/**************************************************************
 *
 * Driver initializations
 *
 **************************************************************/

DRIVER_INIT_MEMBER(a400_state,a800xl)
{
	a800xl_mmu(machine(), 0xff);
}

DRIVER_INIT_MEMBER(a400_state,xegs)
{
	xegs_mmu(machine(), 0xff);
}

DRIVER_INIT_MEMBER(a400_state,a600xl)
{
	UINT8 *rom = machine().root_device().memregion("maincpu")->base();
	memcpy( rom + 0x5000, rom + 0xd000, 0x800 );
}

/**************************************************************
 *
 * Game driver(s)
 *
 **************************************************************/

/*     YEAR  NAME      PARENT    COMPAT MACHINE     INPUT    INIT      COMPANY    FULLNAME */
COMP ( 1979, a400,     0,        0,     a400,       a800, driver_device,    0,      "Atari",   "Atari 400 (NTSC)", 0)
COMP ( 1979, a400pal,  a400,     0,     a400pal,    a800, driver_device,    0,      "Atari",   "Atari 400 (PAL)",  0)
COMP ( 1979, a800,     0,        0,     a800,       a800, driver_device,    0,      "Atari",   "Atari 800 (NTSC)", 0)
COMP ( 1979, a800pal,  a800,     0,     a800pal,    a800, driver_device,    0,      "Atari",   "Atari 800 (PAL)",  0)
COMP ( 1982, a1200xl,  a800,     0,     a1200xl,    a800xl, a400_state,  a800xl, "Atari",   "Atari 1200XL",     GAME_NOT_WORKING )		// 64k RAM
COMP ( 1983, a600xl,   a800xl,   0,     a600xl,     a800xl, a400_state,  a600xl, "Atari",   "Atari 600XL",      GAME_NOT_WORKING )		// 16k RAM
COMP ( 1983, a800xl,   0,		 0,     a800xl,     a800xl, a400_state,  a800xl, "Atari",   "Atari 800XL (NTSC)",GAME_IMPERFECT_GRAPHICS )		// 64k RAM
COMP ( 1983, a800xlp,  a800xl,	 0,     a800xlpal,  a800xl, a400_state,  a800xl, "Atari",   "Atari 800XL (PAL)", GAME_IMPERFECT_GRAPHICS )		// 64k RAM
COMP ( 1986, a65xe,    a800xl,   0,     a800xl,     a800xl, a400_state,  a800xl, "Atari",   "Atari 65XE",       GAME_NOT_WORKING )		// 64k RAM
COMP ( 1986, a65xea,   a800xl,   0,     a800xl,     a800xl, a400_state,  a800xl, "Atari",   "Atari 65XE (Arabic)", GAME_NOT_WORKING )
COMP ( 1986, a130xe,   a800xl,   0,     a800xl,     a800xl, a400_state,  a800xl, "Atari",   "Atari 130XE",      GAME_NOT_WORKING )		// 128k RAM
COMP ( 1986, a800xe,   a800xl,   0,     a800xl,     a800xl, a400_state,  a800xl, "Atari",   "Atari 800XE",      GAME_NOT_WORKING )		// 64k RAM
COMP ( 1987, xegs,     0,        0,     xegs,       a800xl, a400_state,  xegs,   "Atari",   "Atari XE Game System", GAME_NOT_WORKING )	// 64k RAM

CONS ( 1982, a5200,    0,        0,     a5200,      a5200, driver_device,   0,      "Atari",   "Atari 5200",       0)
