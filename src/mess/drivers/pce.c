/****************************************************************************

 PC-Engine / Turbo Grafx 16 driver
 by Charles Mac Donald
 E-Mail: cgfm2@hooked.net

 Thanks to David Shadoff and Brian McPhail for help with the driver.

****************************************************************************/

/**********************************************************************
          To-Do List:
- convert h6280-based drivers to internal memory map for the I/O region
- test sprite collision and overflow interrupts
- sprite precaching
- rewrite the base renderer loop
- Add CD support
- Add 6 button joystick support
- Add 263 line mode
- Sprite DMA should use vdc VRAM functions
- properly implement the pixel clocks instead of the simple scaling we do now

Banking
=======

Normally address spacebanks 00-F6 are assigned to regular HuCard ROM space. There
are a couple of special situations:

Street Fighter II:
  - address space banks 40-7F switchable by writing to 1FF0-1FF3
    1FF0 - select rom banks 40-7F
    1FF1 - select rom banks 80-BF
    1FF2 - select rom banks C0-FF
    1FF3 - select rom banks 100-13F

Populous:
  - address space banks 40-43 contains 32KB RAM

CDRom units:
  - address space banks 80-87 contains 64KB RAM

Super System Card:
  - address space banks 68-7F contains 192KB RAM

**********************************************************************/

/**********************************************************************
                          Known Bugs
***********************************************************************
- Benkei Gaiden: hangs after a few Sunsoft logo spins.
- Busou Keiji Cyber Cross: hangs on attract mode
- Cadash: After choosing character and name, the game starts and the display 'jiggles' like tracking if off a VCR
- Fighting Run: has color and sprite issues during gameplay;
- Hisou Kihei - Xserd: black screen;
**********************************************************************/

#include "emu.h"
#include "video/vdc.h"
#include "cpu/h6280/h6280.h"
#include "includes/pce.h"
#include "imagedev/cartslot.h"
#include "imagedev/chd_cd.h"
#include "sound/c6280.h"
#include "sound/cdda.h"
#include "sound/msm5205.h"
#include "video/huc6270.h"
#include "video/huc6202.h"


static ADDRESS_MAP_START( pce_mem , AS_PROGRAM, 8, pce_state )
	AM_RANGE( 0x000000, 0x07FFFF) AM_ROMBANK("bank1")
	AM_RANGE( 0x080000, 0x087FFF) AM_ROMBANK("bank2")
	AM_RANGE( 0x088000, 0x0CFFFF) AM_ROMBANK("bank3")
	AM_RANGE( 0x0D0000, 0x0FFFFF) AM_ROMBANK("bank4")
	AM_RANGE( 0x100000, 0x10FFFF) AM_RAM AM_SHARE("cd_ram")
	AM_RANGE( 0x110000, 0x1EDFFF) AM_NOP
	AM_RANGE( 0x1EE000, 0x1EE7FF) AM_ROMBANK("bank10") AM_WRITE(pce_cd_bram_w )
	AM_RANGE( 0x1EE800, 0x1EFFFF) AM_NOP
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000) AM_SHARE("user_ram")
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_READWRITE_LEGACY(vdc_0_r, vdc_0_w )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_READWRITE_LEGACY(vce_r, vce_w )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE(C6280_TAG, c6280_device, c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_DEVREADWRITE("maincpu", h6280_device, timer_r, timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE(mess_pce_joystick_r, mess_pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_DEVREADWRITE("maincpu", h6280_device, irq_status_r, irq_status_w )
	AM_RANGE( 0x1FF800, 0x1FFBFF) AM_READWRITE(pce_cd_intf_r, pce_cd_intf_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io , AS_IO, 8, pce_state )
	AM_RANGE( 0x00, 0x03) AM_READWRITE_LEGACY(vdc_0_r, vdc_0_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( sgx_mem , AS_PROGRAM, 8, pce_state )
	AM_RANGE( 0x000000, 0x07FFFF) AM_ROMBANK("bank1")
	AM_RANGE( 0x080000, 0x087FFF) AM_ROMBANK("bank2")
	AM_RANGE( 0x088000, 0x0CFFFF) AM_ROMBANK("bank3")
	AM_RANGE( 0x0D0000, 0x0FFFFF) AM_ROMBANK("bank4")
	AM_RANGE( 0x100000, 0x10FFFF) AM_RAM AM_SHARE("cd_ram")
	AM_RANGE( 0x110000, 0x1EDFFF) AM_NOP
	AM_RANGE( 0x1EE000, 0x1EE7FF) AM_ROMBANK("bank10") AM_WRITE(pce_cd_bram_w )
	AM_RANGE( 0x1EE800, 0x1EFFFF) AM_NOP
	AM_RANGE( 0x1F0000, 0x1F7FFF) AM_RAM AM_SHARE("user_ram")
	AM_RANGE( 0x1FE000, 0x1FE007) AM_READWRITE_LEGACY(vdc_0_r, vdc_0_w ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1FE008, 0x1FE00F) AM_READWRITE_LEGACY(vpc_r, vpc_w ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1FE010, 0x1FE017) AM_READWRITE_LEGACY(vdc_1_r, vdc_1_w ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_READWRITE_LEGACY(vce_r, vce_w )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE(C6280_TAG, c6280_device, c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_DEVREADWRITE("maincpu", h6280_device, timer_r, timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE(mess_pce_joystick_r, mess_pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_DEVREADWRITE("maincpu", h6280_device, irq_status_r, irq_status_w )
	AM_RANGE( 0x1FF800, 0x1FFBFF) AM_READWRITE(pce_cd_intf_r, pce_cd_intf_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( sgx_io , AS_IO, 8, pce_state )
	AM_RANGE( 0x00, 0x03) AM_READWRITE_LEGACY(sgx_vdc_r, sgx_vdc_w )
ADDRESS_MAP_END

/* todo: alternate forms of input (multitap, mouse, etc.) */
static INPUT_PORTS_START( pce )

	PORT_START("JOY_P1")
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button I") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button II") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P1 Select") PORT_PLAYER(1)		PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P1 Run") PORT_PLAYER(1)			PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)							PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)						PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)						PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)						PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0000)

	PORT_START("JOY_P2")
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Button I") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Button II") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P2 Select") PORT_PLAYER(2)		PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P2 Run") PORT_PLAYER(2)			PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) 						PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)						PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)						PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)						PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0000)

	PORT_START("JOY_P3")
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P3 Button I") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Button II") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P3 Select") PORT_PLAYER(3)		PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P3 Run") PORT_PLAYER(3)			PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3) 						PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)						PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)						PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)						PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0000)

	PORT_START("JOY_P4")
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P4 Button I") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P4 Button II") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P4 Select") PORT_PLAYER(4)		PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P4 Run") PORT_PLAYER(4)			PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4) 						PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)						PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)						PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)						PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0000)

	PORT_START("JOY_P5")
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P5 Button I") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P5 Button II") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P5 Select") PORT_PLAYER(5)		PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P5 Run") PORT_PLAYER(5)			PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(5) 						PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(5)						PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(5)						PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(5)						PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0000)

	PORT_START("JOY6B_P1")
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Button I") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Button II") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P1 Select") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P1 Run") PORT_PLAYER(1)		PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)						PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)						PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)						PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)					PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Button III") PORT_PLAYER(1) PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Button IV") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Button V") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Button VI") PORT_PLAYER(1)	PORT_CONDITION("JOY_TYPE", 0x0003, EQUALS, 0x0002)
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH,IPT_UNUSED ) //6-button pad header

	PORT_START("JOY6B_P2")  /* Player 2 controls */
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Button I") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Button II") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P2 Select") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P2 Run") PORT_PLAYER(2)		PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)						PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) 					PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)						PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)					PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Button III") PORT_PLAYER(2) PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Button IV") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Button V") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Button VI") PORT_PLAYER(2)	PORT_CONDITION("JOY_TYPE", 0x000c, EQUALS, 0x0008)
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH,IPT_UNUSED ) //6-button pad header

	PORT_START("JOY6B_P3")  /* Player 3 controls */
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P3 Button I") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3 Button II") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P3 Select") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P3 Run") PORT_PLAYER(3)		PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)						PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3) 					PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)						PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)					PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P3 Button III") PORT_PLAYER(3) PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P3 Button IV") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P3 Button V") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P3 Button VI") PORT_PLAYER(3)	PORT_CONDITION("JOY_TYPE", 0x0030, EQUALS, 0x0020)
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH,IPT_UNUSED ) //6-button pad header

	PORT_START("JOY6B_P4")  /* Player 4 controls */
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P4 Button I") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P4 Button II") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P4 Select") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P4 Run") PORT_PLAYER(4)		PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)						PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)						PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)						PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)					PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P3 Button III") PORT_PLAYER(4) PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P3 Button IV") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P3 Button V") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P3 Button VI") PORT_PLAYER(4)	PORT_CONDITION("JOY_TYPE", 0x00c0, EQUALS, 0x0080)
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH,IPT_UNUSED ) //6-button pad header

	PORT_START("JOY6B_P5")  /* Player 5 controls */
	/* II is left of I on the original pad so we map them in reverse order */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P5 Button I") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P5 Button II") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SELECT  ) PORT_NAME("P5 Select") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START   ) PORT_NAME("P5 Run") PORT_PLAYER(5)		PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(5)						PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(5)						PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(5)						PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(5)					PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P5 Button III") PORT_PLAYER(5) PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P5 Button IV") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P5 Button V") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P5 Button VI") PORT_PLAYER(5)	PORT_CONDITION("JOY_TYPE", 0x0300, EQUALS, 0x0200)
	PORT_BIT( 0xf000, IP_ACTIVE_HIGH,IPT_UNUSED ) //6-button pad header

	PORT_START("JOY_TYPE")
	PORT_CONFNAME(0x0003,0x0000,"Joystick Type Player 1")
	PORT_CONFSETTING( 0x0000,"2-buttons" )
//  PORT_CONFSETTING( 0x0001,"3-buttons" )
	PORT_CONFSETTING( 0x0002,"6-buttons" )
//  PORT_CONFSETTING( 0x0003,"Mouse" )
	PORT_CONFNAME(0x000c,0x0000,"Joystick Type Player 2")
	PORT_CONFSETTING( 0x0000,"2-buttons" )
//  PORT_CONFSETTING( 0x0004,"3-buttons" )
	PORT_CONFSETTING( 0x0008,"6-buttons" )
//  PORT_CONFSETTING( 0x000c,"Mouse" )
	PORT_CONFNAME(0x0030,0x0000,"Joystick Type Player 3")
	PORT_CONFSETTING( 0x0000,"2-buttons" )
//  PORT_CONFSETTING( 0x0010,"3-buttons" )
	PORT_CONFSETTING( 0x0020,"6-buttons" )
//  PORT_CONFSETTING( 0x0030,"Mouse" )
	PORT_CONFNAME(0x00c0,0x0000,"Joystick Type Player 4")
	PORT_CONFSETTING( 0x0000,"2-buttons" )
//  PORT_CONFSETTING( 0x0040,"3-buttons" )
	PORT_CONFSETTING( 0x0080,"6-buttons" )
//  PORT_CONFSETTING( 0x00c0,"Mouse" )
	PORT_CONFNAME(0x0300,0x0000,"Joystick Type Player 5")
	PORT_CONFSETTING( 0x0000,"2-buttons" )
//  PORT_CONFSETTING( 0x0100,"3-buttons" )
	PORT_CONFSETTING( 0x0200,"6-buttons" )
//  PORT_CONFSETTING( 0x0300,"Mouse" )

	PORT_START("A_CARD")
	PORT_CONFNAME( 0x01, 0x01, "Arcade Card" )
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )
INPUT_PORTS_END


static void pce_partialhash(hash_collection &dest, const unsigned char *data,
	unsigned long length, const char *functions)
{
	if ( ( length <= PCE_HEADER_SIZE ) || ( length & PCE_HEADER_SIZE ) ) {
			dest.compute(&data[PCE_HEADER_SIZE], length - PCE_HEADER_SIZE, functions);
	} else {
		dest.compute(data, length, functions);
	}
}

static const c6280_interface c6280_config =
{
	"maincpu"
};

struct cdrom_interface pce_cdrom =
{
	"pce_cdrom",
	NULL
};

static MACHINE_CONFIG_FRAGMENT( pce_cdslot )
	MCFG_CDROM_ADD("cdrom",pce_cdrom)

	MCFG_SOFTWARE_LIST_ADD("cd_list","pcecd")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( pce_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("pce,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("pce_cart")
	MCFG_CARTSLOT_LOAD(pce_cart)
	MCFG_CARTSLOT_PARTIALHASH(pce_partialhash)
	MCFG_SOFTWARE_LIST_ADD("cart_list","pce")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( tg16_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("pce,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("tg16_cart")
	MCFG_CARTSLOT_LOAD(pce_cart)
	MCFG_CARTSLOT_PARTIALHASH(pce_partialhash)
	MCFG_SOFTWARE_LIST_ADD("cart_list","tg16")
MACHINE_CONFIG_END

static MACHINE_CONFIG_FRAGMENT( sgx_cartslot )
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("pce,bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("pce_cart")
	MCFG_CARTSLOT_LOAD(pce_cart)
	MCFG_CARTSLOT_PARTIALHASH(pce_partialhash)
	MCFG_SOFTWARE_LIST_ADD("cart_list","sgx")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pce_common, pce_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(pce_mem)
	MCFG_CPU_IO_MAP(pce_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", pce_interrupt, "screen", 0, 1)

//  MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pce_state, pce )
	MCFG_MACHINE_RESET_OVERRIDE(pce_state, mess_pce )

    /* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/2, VDC_WPF, 70, 70 + 512 + 32, VDC_LPF, 14, 14+242)
	MCFG_SCREEN_UPDATE_STATIC( pce )

	/* MCFG_GFXDECODE( pce ) */
	MCFG_PALETTE_LENGTH(1024)
	MCFG_PALETTE_INIT( vce )

	MCFG_VIDEO_START( pce )

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD(C6280_TAG, C6280, MAIN_CLOCK/6)
	MCFG_SOUND_CONFIG(c6280_config)
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )

	MCFG_SOUND_ADD( "msm5205", MSM5205, PCE_CD_CLOCK / 6 )
	MCFG_SOUND_CONFIG( pce_cd_msm5205_interface )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 0.50 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pce, pce_common )
	MCFG_FRAGMENT_ADD( pce_cartslot )
	MCFG_FRAGMENT_ADD( pce_cdslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tg16, pce_common )
	MCFG_FRAGMENT_ADD( tg16_cartslot )
	MCFG_FRAGMENT_ADD( pce_cdslot )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sgx, pce_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(sgx_mem)
	MCFG_CPU_IO_MAP(sgx_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", sgx_interrupt, "screen", 0, 1)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pce_state, pce )
	MCFG_MACHINE_RESET_OVERRIDE(pce_state, mess_pce )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/2, VDC_WPF, 70, 70 + 512 + 32, VDC_LPF, 14, 14+242)
	MCFG_SCREEN_UPDATE_STATIC( pce )

	MCFG_PALETTE_LENGTH(1024)
	MCFG_PALETTE_INIT( vce )

	MCFG_VIDEO_START( pce )

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD(C6280_TAG, C6280, MAIN_CLOCK/6)
	MCFG_SOUND_CONFIG(c6280_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_SOUND_ADD( "msm5205", MSM5205, PCE_CD_CLOCK / 6 )
	MCFG_SOUND_CONFIG( pce_cd_msm5205_interface )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 0.00 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 0.00 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )

	MCFG_FRAGMENT_ADD( sgx_cartslot )
	MCFG_FRAGMENT_ADD( pce_cdslot )
MACHINE_CONFIG_END



static ADDRESS_MAP_START( pce_mem_new , AS_PROGRAM, 8, pce_state )
	AM_RANGE( 0x000000, 0x07FFFF) AM_ROMBANK("bank1")
	AM_RANGE( 0x080000, 0x087FFF) AM_ROMBANK("bank2")
	AM_RANGE( 0x088000, 0x0CFFFF) AM_ROMBANK("bank3")
	AM_RANGE( 0x0D0000, 0x0FFFFF) AM_ROMBANK("bank4")
	AM_RANGE( 0x100000, 0x10FFFF) AM_RAM AM_SHARE("cd_ram")
	AM_RANGE( 0x110000, 0x1EDFFF) AM_NOP
	AM_RANGE( 0x1EE000, 0x1EE7FF) AM_ROMBANK("bank10") AM_WRITE(pce_cd_bram_w )
	AM_RANGE( 0x1EE800, 0x1EFFFF) AM_NOP
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000) AM_SHARE("user_ram")
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_DEVREADWRITE( "huc6260", huc6260_device, read, write )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE( C6280_TAG, c6280_device, c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_DEVREADWRITE( "maincpu", h6280_device, timer_r, timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE( mess_pce_joystick_r, mess_pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_DEVREADWRITE( "maincpu", h6280_device, irq_status_r, irq_status_w )
	AM_RANGE( 0x1FF800, 0x1FFBFF) AM_READWRITE( pce_cd_intf_r, pce_cd_intf_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io_new , AS_IO, 8, pce_state )
	AM_RANGE( 0x00, 0x03) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
ADDRESS_MAP_END


static ADDRESS_MAP_START( sgx_mem_new , AS_PROGRAM, 8, pce_state )
	AM_RANGE( 0x000000, 0x07FFFF) AM_ROMBANK("bank1")
	AM_RANGE( 0x080000, 0x087FFF) AM_ROMBANK("bank2")
	AM_RANGE( 0x088000, 0x0CFFFF) AM_ROMBANK("bank3")
	AM_RANGE( 0x0D0000, 0x0FFFFF) AM_ROMBANK("bank4")
	AM_RANGE( 0x100000, 0x10FFFF) AM_RAM AM_SHARE("cd_ram")
	AM_RANGE( 0x110000, 0x1EDFFF) AM_NOP
	AM_RANGE( 0x1EE000, 0x1EE7FF) AM_ROMBANK("bank10") AM_WRITE(pce_cd_bram_w )
	AM_RANGE( 0x1EE800, 0x1EFFFF) AM_NOP
	AM_RANGE( 0x1F0000, 0x1F7FFF) AM_RAM AM_SHARE("user_ram")
	AM_RANGE( 0x1FE000, 0x1FE007) AM_DEVREADWRITE( "huc6270_0", huc6270_device, read, write ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1FE008, 0x1FE00F) AM_DEVREADWRITE( "huc6202", huc6202_device, read, write ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1FE010, 0x1FE017) AM_DEVREADWRITE( "huc6270_1", huc6270_device, read, write ) AM_MIRROR(0x03E0)
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_DEVREADWRITE( "huc6260", huc6260_device, read, write )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE(C6280_TAG, c6280_device, c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_DEVREADWRITE("maincpu", h6280_device, timer_r, timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE(mess_pce_joystick_r, mess_pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_DEVREADWRITE("maincpu", h6280_device, irq_status_r, irq_status_w )
	AM_RANGE( 0x1FF800, 0x1FFBFF) AM_READWRITE(pce_cd_intf_r, pce_cd_intf_w )
ADDRESS_MAP_END


static ADDRESS_MAP_START( sgx_io_new , AS_IO, 8, pce_state )
	AM_RANGE( 0x00, 0x03) AM_DEVREADWRITE( "huc6202", huc6202_device, io_read, io_write )
ADDRESS_MAP_END


UINT32 pce_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_huc6260->video_update( bitmap, cliprect );
	return 0;
}


WRITE_LINE_MEMBER(pce_state::pce_irq_changed)
{
	machine().device("maincpu")->execute().set_input_line(0, state);
}


static const huc6270_interface pce_huc6270_config =
{
	0x10000,
	DEVCB_DRIVER_LINE_MEMBER(pce_state,pce_irq_changed)
};


static const huc6260_interface pce_huc6260_config =
{
	"screen",
	DEVCB_DEVICE_MEMBER16( "huc6270", huc6270_device, next_pixel ),
	DEVCB_DEVICE_MEMBER16( "huc6270", huc6270_device, time_until_next_event ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6270", huc6270_device, vsync_changed ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6270", huc6270_device, hsync_changed )
};


static const huc6270_interface sgx_huc6270_0_config =
{
	0x10000,
	DEVCB_DRIVER_LINE_MEMBER(pce_state,pce_irq_changed)
};


static const huc6270_interface sgx_huc6270_1_config =
{
	0x10000,
	DEVCB_DRIVER_LINE_MEMBER(pce_state,pce_irq_changed)
};


static const huc6202_interface sgx_huc6202_config =
{
	DEVCB_DEVICE_MEMBER16( "huc6270_0", huc6270_device, next_pixel ),
	DEVCB_DEVICE_MEMBER16( "huc6270_0", huc6270_device, time_until_next_event ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6270_0", huc6270_device, vsync_changed ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6270_0", huc6270_device, hsync_changed ),
	DEVCB_DEVICE_MEMBER( "huc6270_0", huc6270_device, read ),
	DEVCB_DEVICE_MEMBER( "huc6270_0", huc6270_device, write ),
	DEVCB_DEVICE_MEMBER16( "huc6270_1", huc6270_device, next_pixel ),
	DEVCB_DEVICE_MEMBER16( "huc6270_1", huc6270_device, time_until_next_event ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6270_1", huc6270_device, vsync_changed ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6270_1", huc6270_device, hsync_changed ),
	DEVCB_DEVICE_MEMBER( "huc6270_1", huc6270_device, read ),
	DEVCB_DEVICE_MEMBER( "huc6270_1", huc6270_device, write ),
};


static const huc6260_interface sgx_huc6260_config =
{
	"screen",
	DEVCB_DEVICE_MEMBER16( "huc6202", huc6202_device, next_pixel ),
	DEVCB_DEVICE_MEMBER16( "huc6202", huc6202_device, time_until_next_event ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6202", huc6202_device, vsync_changed ),
	DEVCB_DEVICE_LINE_MEMBER( "huc6202", huc6202_device, hsync_changed )
};

static MACHINE_CONFIG_START( pce_common_new, pce_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(pce_mem_new)
	MCFG_CPU_IO_MAP(pce_io_new)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pce_state, pce )
	MCFG_MACHINE_RESET_OVERRIDE(pce_state, mess_pce )

    /* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK, HUC6260_WPF, 64, 64 + 1024 + 64, HUC6260_LPF, 18, 18 + 242)
	MCFG_SCREEN_UPDATE_DRIVER( pce_state, screen_update )

	MCFG_PALETTE_LENGTH( HUC6260_PALETTE_SIZE )
	MCFG_PALETTE_INIT( huc6260 )

	MCFG_HUC6260_ADD( "huc6260", MAIN_CLOCK, pce_huc6260_config )
	MCFG_HUC6270_ADD( "huc6270", pce_huc6270_config )

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD(C6280_TAG, C6280, MAIN_CLOCK/6)
	MCFG_SOUND_CONFIG(c6280_config)
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )

	MCFG_SOUND_ADD( "msm5205", MSM5205, PCE_CD_CLOCK / 6 )
	MCFG_SOUND_CONFIG( pce_cd_msm5205_interface )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 0.50 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( pce_new, pce_common_new )
	MCFG_FRAGMENT_ADD( pce_cartslot )
	MCFG_FRAGMENT_ADD( pce_cdslot )
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( tg16_new, pce_common_new )
	MCFG_FRAGMENT_ADD( tg16_cartslot )
	MCFG_FRAGMENT_ADD( pce_cdslot )
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( sgx_new, pce_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(sgx_mem_new)
	MCFG_CPU_IO_MAP(sgx_io_new)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	MCFG_MACHINE_START_OVERRIDE(pce_state, pce )
	MCFG_MACHINE_RESET_OVERRIDE(pce_state, mess_pce )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK, HUC6260_WPF, 64, 64 + 1024 + 64, HUC6260_LPF, 18, 18 + 242)
	MCFG_SCREEN_UPDATE_DRIVER( pce_state, screen_update )

	MCFG_PALETTE_LENGTH( HUC6260_PALETTE_SIZE )
	MCFG_PALETTE_INIT( huc6260 )

	MCFG_HUC6260_ADD( "huc6260", MAIN_CLOCK, sgx_huc6260_config )
	MCFG_HUC6270_ADD( "huc6270_0", sgx_huc6270_0_config )
	MCFG_HUC6270_ADD( "huc6270_1", sgx_huc6270_1_config )
	MCFG_HUC6202_ADD( "huc6202", sgx_huc6202_config )

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD(C6280_TAG, C6280, MAIN_CLOCK/6)
	MCFG_SOUND_CONFIG(c6280_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_SOUND_ADD( "msm5205", MSM5205, PCE_CD_CLOCK / 6 )
	MCFG_SOUND_CONFIG( pce_cd_msm5205_interface )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "lspeaker", 0.00 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "rspeaker", 0.00 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "rspeaker", 1.00 )

	MCFG_FRAGMENT_ADD( sgx_cartslot )
	MCFG_FRAGMENT_ADD( pce_cdslot )
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pce )
	ROM_REGION( PCE_ROM_MAXSIZE, "user1", ROMREGION_ERASEFF )		/* Cartridge ROM area */
ROM_END

#define rom_tg16 rom_pce
#define rom_sgx rom_pce

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT    INIT   COMPANY  FULLNAME */
CONS( 1987, pce,    0,      0,      pce,    pce, pce_state,     mess_pce,   "Nippon Electronic Company", "PC Engine", GAME_IMPERFECT_SOUND)
CONS( 1989, tg16,   pce,    0,      tg16,   pce, pce_state,     tg16,		 "Nippon Electronic Company", "TurboGrafx 16", GAME_IMPERFECT_SOUND)
CONS( 1989, sgx,    pce,    0,      sgx,    pce, pce_state,     sgx,		 "Nippon Electronic Company", "SuperGrafx", GAME_IMPERFECT_SOUND)

/* Experimental versions of drivers using newer but slower device implementation of the video chips */
#define rom_pce_new rom_pce
#define rom_tg16_new rom_pce
#define rom_sgx_new rom_pce
CONS( 1987, pce_new,    0,      0,      pce_new,    pce, pce_state,     mess_pce,   "Nippon Electronic Company", "PC Engine", GAME_IMPERFECT_SOUND)
CONS( 1989, tg16_new,   pce_new,    0,      tg16_new,   pce, pce_state,     tg16,        "Nippon Electronic Company", "TurboGrafx 16", GAME_IMPERFECT_SOUND)
CONS( 1989, sgx_new,    pce_new,    0,      sgx_new,    pce, pce_state,     sgx,         "Nippon Electronic Company", "SuperGrafx", GAME_IMPERFECT_SOUND)

