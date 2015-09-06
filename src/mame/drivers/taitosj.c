// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Taito SJ system

Memory map
----------

MAIN CPU:

Address          Dir Data     Name       Description
---------------- --- -------- ---------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx ROM5       program ROM
001xxxxxxxxxxxxx R   xxxxxxxx ROM6       program ROM
010xxxxxxxxxxxxx R   xxxxxxxx ROM7       program ROM
011xxxxxxxxxxxxx R   xxxxxxxx ROM8/ROM9  program ROM (banked)
10000xxxxxxxxxxx R/W xxxxxxxx SRAMRQ     work RAM
10001----------0 R   xxxxxxxx 8800       68705 data read
10001----------1 R   -------x 68ACCEPT   the 68705 has read data from the Z80
10001----------1 R   ------x- 68READY    the 68705 has written data for the Z80
10001----------0   W xxxxxxxx            68705 data write [3]
10001----------1   W -------- ZINTRQ     trigger IRQ on 68705 [3]
1001xxxxxxxxxxxx R/W xxxxxxxx CDR1-2     character generator RAM
101xxxxxxxxxxxxx R/W xxxxxxxx CDR3-6     character generator RAM
1100xxxxxxxxxxxx R/W xxxxxxxx CHARQ      tilemap RAM [4]
11010000xxxxxxxx R/W xxxxxxxx SCRRQ      tilemap column scroll
11010001xxxxxxxx R/W xxxxxxxx OBJRQ      sprites RAM [1]
11010010-xxxxxx0   W -------x VCRRQ      palette chip (93419)
11010010-xxxxxx1   W xxxxxxxx VCRRQ      palette chip (93419)
11010011--------   W ---xxxxx PRY        priority control [2]
11010100----0000 R   xxxxxxxx H0-H7      sprite 0..7 collided with a previous one
11010100----0001 R   xxxxxxxx H8-H15     sprite 8..15 collided with a previous one
11010100----0010 R   xxxxxxxx H16-H23    sprite 16..23 collided with a previous one
11010100----0011 R   -------x OB1        sprite/tilemap 1 collision
11010100----0011 R   ------x- OB2        sprite/tilemap 2 collision
11010100----0011 R   -----x-- OB3        sprite/tilemap 3 collision
11010100----0011 R   ----x--- S12        tilemap 1/tilemap 2 collision
11010100----0011 R   ---x---- S13        tilemap 1/tilemap 3 collision
11010100----0011 R   --x----- S23        tilemap 2/tilemap 3 collision
11010100----0011 R   00------            always 0
11010100----01-- R   xxxxxxxx EXRHR      read contents of graphics ROMs (address selected by d509-d50a)
11010100----1000 R   11xxxxxx IN0        digital inputs
11010100----1001 R   11xxxxxx IN1        digital inputs
11010100----1010 R   xxxxxxxx DP0        dip switch A
11010100----1011 R   --xxxxxx IN3        come from a PAL (ROM6 @ 14)
11010100----1011 R   xx------ IN3        digital inputs
11010100----1100 R   xxxxxxxx IN4        digital inputs
11010100----1101 R   ----xxxx IN5        digital inputs
11010100----1101 R   xxxx---- IN5        from AY-3-8910 #2 port A (status from sound CPU)
11010100----111x R/W xxxxxxxx READIN     AY-3-8910 #0
11010101----0000   W xxxxxxxx SPH1       tilemap 1 H scroll
11010101----0001   W xxxxxxxx SPV1       tilemap 1 V scroll
11010101----0010   W xxxxxxxx SPH2       tilemap 2 H scroll
11010101----0011   W xxxxxxxx SPV2       tilemap 2 V scroll
11010101----0100   W xxxxxxxx SPH3       tilemap 3 H scroll
11010101----0101   W xxxxxxxx SPV3       tilemap 3 V scroll
11010101----0110   W -----xxx MD1        tilemap 1 color code
11010101----0110   W ----x--- CCH1       tilemap 1 char bank
11010101----0110   W -xxx---- MD2        tilemap 2 color code
11010101----0110   W x------- CCH2       tilemap 2 char bank
11010101----0111   W -----xxx MD3        tilemap 3 color code
11010101----0111   W ----x--- CCH3       tilemap 3 char bank
11010101----0111   W --xx---- MD0        sprite color bank
11010101----0111   W xx------ n.c.
11010101----1000   W -------- HTCLR      clear hardware collision detection registers
11010101----1001   W xxxxxxxx EXROM1     low 8 bits of address to read from graphics ROMs
11010101----1010   W xxxxxxxx EXROM2     high 8 bits of address to read from graphics ROMs
11010101----1011   W xxxxxxxx EPORT1     command to sound CPU
11010101----1100   W -------x EPORT2     single bit signal to audio CPU (not used?)
11010101----1101   W -------- TIME RESET watchdog reset
11010101----1110   W -------x COIN LOCK  coin lockout
11010101----1110   W ------x- SOUND STOP mute sound
11010101----1110   W -xxxxx-- n.c.
11010101----1110   W x------- BANK SEL   program ROM bank select
11010101----1111   W ---xxxxx EXPORT     to a PAL (ROM6 @ 14)
11010110--------   W -------x HINV       horizontal screen flip
11010110--------   W ------x- VINV       vertical screen flip
11010110--------   W -----x-- OBJEX      sprite bank select [1]
11010110--------   W ----x--- n.c.
11010110--------   W ---x---- SN1OFF     tilemap 1 enable
11010110--------   W --x----- SN2OFF     tilemap 2 enable
11010110--------   W -x------ SN3OFF     tilemap 3 enable
11010110--------   W x------- OBJOFF     sprites enable
11010111--------              n.c.
111xxxxxxxxxxxxx R   xxxxxxxx ROM10      program ROM


[1] There are 256 bytes of sprite RAM, but only 128 can be accessed by the
    video hardware at a time. OBJEX selects the high or low bank. The CPU can
    access all the memory linearly. This feature doesn't seem to be ever used
    although bioatack and spaceskr do initialise the second bank.

[2] Priority is controlled by a 256x4 PROM.
    Bits 0-3 of PRY go to A4-A7 of the PROM, bit 4 selectes D0-D1 or D2-D3.
    A0-A3 of the PROM is fed with a mask of the inactive planes in the order
    OBJ-SCN1-SCN2-SCN3. The 2-bit code which comes out from the PROM selects
    the plane to display.

[3] A jumper selects whether writing to 8800 should also automatically trigger
    the interrupt, or it should be explicitly triggered by a write to 8801.


[4] The first page of tilemap RAM c000-c3ff is used by bioatack during sprite
    collision detection. It is also initialised by wwestern and wwester1
    although they don't appear to use it.

SOUND CPU:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
0000xxxxxxxxxxxx R   xxxxxxxx ROM1      program ROM
0001xxxxxxxxxxxx R   xxxxxxxx ROM2      program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM3      program ROM
0011xxxxxxxxxxxx R   xxxxxxxx ROM4      program ROM
010000xxxxxxxxxx R/W xxxxxxxx           work RAM
01001--------00x R/W xxxxxxxx CS5       AY-3-8910 #1
01001--------01x R/W xxxxxxxx CS5       AY-3-8910 #2
01001--------1-x R/W xxxxxxxx CS5       AY-3-8910 #3
01010---------00 R   xxxxxxxx RD5000    read command from main CPU
01010---------00   W -------- WR5000    clear bit 7 of command from main CPU (not used?)
01010---------01 R   ----x--- RD5001    command pending from main CPU (not used?)
01010---------01 R   -----x-- RD5001    single bit from main CPU (not used?)
01010---------01 R   ------11 RD5001    always 1
01010---------01   W -------- WR5001    clear single bit from main CPU (not used?)
01010---------10              n.c.
01010---------11              n.c.
111xxxxxxxxxxxxx R   xxxxxxxx           space for diagnostics ROM? not shown in the schematics



8910 #0
port A: DSW B
port B: DSW C

8910 #1
port A: digital sound out
port B: digital sound volume?

8910 #2:
port A: bits 4-7 IN54-IN57 read by main CPU at d40d
port B: n.c.

8910 #3:
port A: bits 0-1 control RC filter on this chip's output
port B: bit 0 NMI enable


Notes:
------
- Alpine Sky uses the feature where write to d50e-d50f can be processed by a PAL and
  answer read back from d40b.

Kickstart Wheelie King :
- additional ram @ $d800-$dfff (scroll ram  + ??)
- color bank @ $d000-$d001
- taitosj_scroll @ $d002-$d007
- strange controls :
     - 'revolve type' - 3 pos switch (gears) +  button/pedal (accel)
     - two buttons for gear change, auto acceleration

TODO:
-----
- RC filter on 8910 #3

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/taitosj.h"


WRITE8_MEMBER(taitosj_state::taitosj_sndnmi_msk_w)
{
	m_sndnmi_disable = data & 0x01;
}

WRITE8_MEMBER(taitosj_state::taitosj_soundcommand_w)
{
	soundlatch_byte_w(space,offset,data);
	if (!m_sndnmi_disable) m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


WRITE8_MEMBER(taitosj_state::input_port_4_f0_w)
{
	m_input_port_4_f0 = data >> 4;
}

CUSTOM_INPUT_MEMBER(taitosj_state::input_port_4_f0_r)
{
	return m_input_port_4_f0;
}


static ADDRESS_MAP_START( taitosj_main_nomcu_map, AS_PROGRAM, 8, taitosj_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x07fe) AM_READWRITE(taitosj_fake_data_r, taitosj_fake_data_w)
	AM_RANGE(0x8801, 0x8801) AM_MIRROR(0x07fe) AM_READ(taitosj_fake_status_r)
	AM_RANGE(0x9000, 0xbfff) AM_WRITE(taitosj_characterram_w) AM_SHARE("characterram")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_SHARE("videoram_1")
	AM_RANGE(0xc800, 0xcbff) AM_RAM AM_SHARE("videoram_2")
	AM_RANGE(0xcc00, 0xcfff) AM_RAM AM_SHARE("videoram_3")
	AM_RANGE(0xd000, 0xd05f) AM_RAM AM_SHARE("colscrolly")
	AM_RANGE(0xd100, 0xd1ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd200, 0xd27f) AM_MIRROR(0x0080) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0xd300, 0xd300) AM_MIRROR(0x00ff) AM_WRITEONLY AM_SHARE("video_priority")
	AM_RANGE(0xd400, 0xd403) AM_MIRROR(0x00f0) AM_READONLY AM_SHARE("collision_reg")
	AM_RANGE(0xd404, 0xd404) AM_MIRROR(0x00f3) AM_READ(taitosj_gfxrom_r)
	AM_RANGE(0xd408, 0xd408) AM_MIRROR(0x00f0) AM_READ_PORT("IN0")
	AM_RANGE(0xd409, 0xd409) AM_MIRROR(0x00f0) AM_READ_PORT("IN1")
	AM_RANGE(0xd40a, 0xd40a) AM_MIRROR(0x00f0) AM_READ_PORT("DSW1")         /* DSW1 */
	AM_RANGE(0xd40b, 0xd40b) AM_MIRROR(0x00f0) AM_READ_PORT("IN2")
	AM_RANGE(0xd40c, 0xd40c) AM_MIRROR(0x00f0) AM_READ_PORT("IN3")          /* Service */
	AM_RANGE(0xd40d, 0xd40d) AM_MIRROR(0x00f0) AM_READ_PORT("IN4")
	AM_RANGE(0xd40e, 0xd40f) AM_MIRROR(0x00f0) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xd40f, 0xd40f) AM_MIRROR(0x00f0) AM_DEVREAD("ay1", ay8910_device, data_r)   /* DSW2 and DSW3 */
	AM_RANGE(0xd500, 0xd505) AM_MIRROR(0x00f0) AM_WRITEONLY AM_SHARE("scroll")
	AM_RANGE(0xd506, 0xd507) AM_MIRROR(0x00f0) AM_WRITEONLY AM_SHARE("colorbank")
	AM_RANGE(0xd508, 0xd508) AM_MIRROR(0x00f0) AM_WRITE(taitosj_collision_reg_clear_w)
	AM_RANGE(0xd509, 0xd50a) AM_MIRROR(0x00f0) AM_WRITEONLY AM_SHARE("gfxpointer")
	AM_RANGE(0xd50b, 0xd50b) AM_MIRROR(0x00f0) AM_WRITE(taitosj_soundcommand_w)
	AM_RANGE(0xd50d, 0xd50d) AM_MIRROR(0x00f0) AM_WRITEONLY /*watchdog_reset_w*/  /* Bio Attack sometimes resets after you die */
	AM_RANGE(0xd50e, 0xd50e) AM_MIRROR(0x00f0) AM_WRITE(taitosj_bankswitch_w)
	AM_RANGE(0xd50f, 0xd50f) AM_MIRROR(0x00f0) AM_WRITENOP
	AM_RANGE(0xd600, 0xd600) AM_MIRROR(0x00ff) AM_WRITEONLY AM_SHARE("video_mode")
	AM_RANGE(0xd700, 0xdfff) AM_NOP
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END


/* only difference is taitosj_fake_ replaced with taitosj_mcu_ */
static ADDRESS_MAP_START( taitosj_main_mcu_map, AS_PROGRAM, 8, taitosj_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_MIRROR(0x07fe) AM_READWRITE(taitosj_mcu_data_r, taitosj_mcu_data_w)
	AM_RANGE(0x8801, 0x8801) AM_MIRROR(0x07fe) AM_READ(taitosj_mcu_status_r)
	AM_RANGE(0x9000, 0xbfff) AM_WRITE(taitosj_characterram_w) AM_SHARE("characterram")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_SHARE("videoram_1")
	AM_RANGE(0xc800, 0xcbff) AM_RAM AM_SHARE("videoram_2")
	AM_RANGE(0xcc00, 0xcfff) AM_RAM AM_SHARE("videoram_3")
	AM_RANGE(0xd000, 0xd05f) AM_RAM AM_SHARE("colscrolly")
	AM_RANGE(0xd100, 0xd1ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd200, 0xd27f) AM_MIRROR(0x0080) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0xd300, 0xd300) AM_MIRROR(0x00ff) AM_WRITEONLY AM_SHARE("video_priority")
	AM_RANGE(0xd400, 0xd403) AM_MIRROR(0x00f0) AM_READONLY AM_SHARE("collision_reg")
	AM_RANGE(0xd404, 0xd404) AM_MIRROR(0x00f3) AM_READ(taitosj_gfxrom_r)
	AM_RANGE(0xd408, 0xd408) AM_MIRROR(0x00f0) AM_READ_PORT("IN0")
	AM_RANGE(0xd409, 0xd409) AM_MIRROR(0x00f0) AM_READ_PORT("IN1")
	AM_RANGE(0xd40a, 0xd40a) AM_MIRROR(0x00f0) AM_READ_PORT("DSW1")         /* DSW1 */
	AM_RANGE(0xd40b, 0xd40b) AM_MIRROR(0x00f0) AM_READ_PORT("IN2")
	AM_RANGE(0xd40c, 0xd40c) AM_MIRROR(0x00f0) AM_READ_PORT("IN3")          /* Service */
	AM_RANGE(0xd40d, 0xd40d) AM_MIRROR(0x00f0) AM_READ_PORT("IN4")
	AM_RANGE(0xd40e, 0xd40f) AM_MIRROR(0x00f0) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xd40f, 0xd40f) AM_MIRROR(0x00f0) AM_DEVREAD("ay1", ay8910_device, data_r)   /* DSW2 and DSW3 */
	AM_RANGE(0xd500, 0xd505) AM_MIRROR(0x00f0) AM_WRITEONLY AM_SHARE("scroll")
	AM_RANGE(0xd506, 0xd507) AM_MIRROR(0x00f0) AM_WRITEONLY AM_SHARE("colorbank")
	AM_RANGE(0xd508, 0xd508) AM_MIRROR(0x00f0) AM_WRITE(taitosj_collision_reg_clear_w)
	AM_RANGE(0xd509, 0xd50a) AM_MIRROR(0x00f0) AM_WRITEONLY AM_SHARE("gfxpointer")
	AM_RANGE(0xd50b, 0xd50b) AM_MIRROR(0x00f0) AM_WRITE(taitosj_soundcommand_w)
	AM_RANGE(0xd50d, 0xd50d) AM_MIRROR(0x00f0) AM_WRITEONLY /*watchdog_reset_w*/  /* Bio Attack sometimes resets after you die */
	AM_RANGE(0xd50e, 0xd50e) AM_MIRROR(0x00f0) AM_WRITE(taitosj_bankswitch_w)
	AM_RANGE(0xd50f, 0xd50f) AM_MIRROR(0x00f0) AM_WRITENOP
	AM_RANGE(0xd600, 0xd600) AM_MIRROR(0x00ff) AM_WRITEONLY AM_SHARE("video_mode")
	AM_RANGE(0xd700, 0xdfff) AM_NOP
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END



/* seems the most logical way to do the gears */

CUSTOM_INPUT_MEMBER(taitosj_state::kikstart_gear_r)
{
	const char *port_tag;

	int player = (int)(FPTR)param;

	if (player == 0)
		port_tag = "GEARP1";
	else
		port_tag = "GEARP2";

	/* gear MUST be 1, 2 or 3 */
	if (ioport(port_tag)->read() & 0x01) m_kikstart_gears[player] = 0x02;
	if (ioport(port_tag)->read() & 0x02) m_kikstart_gears[player] = 0x03;
	if (ioport(port_tag)->read() & 0x04) m_kikstart_gears[player] = 0x01;

	return m_kikstart_gears[player];
}


static ADDRESS_MAP_START( kikstart_main_map, AS_PROGRAM, 8, taitosj_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x8800, 0x8800) AM_READWRITE(taitosj_mcu_data_r, taitosj_mcu_data_w)
	AM_RANGE(0x8801, 0x8801) AM_READ(taitosj_mcu_status_r)
	AM_RANGE(0x8802, 0x8802) AM_NOP
	AM_RANGE(0x8a00, 0x8a5f) AM_WRITEONLY AM_SHARE("colscrolly")
	AM_RANGE(0x9000, 0xbfff) AM_WRITE(taitosj_characterram_w) AM_SHARE("characterram")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_SHARE("videoram_1")
	AM_RANGE(0xc800, 0xcbff) AM_RAM AM_SHARE("videoram_2")
	AM_RANGE(0xcc00, 0xcfff) AM_RAM AM_SHARE("videoram_3")
	AM_RANGE(0xd000, 0xd001) AM_WRITEONLY AM_SHARE("colorbank")
	AM_RANGE(0xd002, 0xd007) AM_WRITEONLY AM_SHARE("scroll")
	AM_RANGE(0xd100, 0xd1ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd200, 0xd27f) AM_RAM AM_SHARE("paletteram")
	AM_RANGE(0xd300, 0xd300) AM_WRITEONLY AM_SHARE("video_priority")
	AM_RANGE(0xd400, 0xd403) AM_READONLY AM_SHARE("collision_reg")
	AM_RANGE(0xd404, 0xd404) AM_READ(taitosj_gfxrom_r)
	AM_RANGE(0xd408, 0xd408) AM_MIRROR(0x00f0) AM_READ_PORT("IN0")
	AM_RANGE(0xd409, 0xd409) AM_MIRROR(0x00f0) AM_READ_PORT("IN1")
	AM_RANGE(0xd40a, 0xd40a) AM_MIRROR(0x00f0) AM_READ_PORT("DSW1")         /* DSW1 */
	AM_RANGE(0xd40b, 0xd40b) AM_MIRROR(0x00f0) AM_READ_PORT("IN2")
	AM_RANGE(0xd40c, 0xd40c) AM_MIRROR(0x00f0) AM_READ_PORT("IN3")          /* Service */
	AM_RANGE(0xd40d, 0xd40d) AM_MIRROR(0x00f0) AM_READ_PORT("IN4")
	AM_RANGE(0xd40e, 0xd40f) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xd40f, 0xd40f) AM_DEVREAD("ay1", ay8910_device, data_r) /* DSW2 and DSW3 */
	AM_RANGE(0xd508, 0xd508) AM_WRITE(taitosj_collision_reg_clear_w)
	AM_RANGE(0xd509, 0xd50a) AM_WRITEONLY AM_SHARE("gfxpointer")
	AM_RANGE(0xd50b, 0xd50b) AM_WRITE(taitosj_soundcommand_w)
	AM_RANGE(0xd50d, 0xd50d) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xd50e, 0xd50e) AM_WRITE(taitosj_bankswitch_w)
	AM_RANGE(0xd600, 0xd600) AM_WRITEONLY AM_SHARE("video_mode")
	AM_RANGE(0xd800, 0xdfff) AM_RAM AM_SHARE("kikstart_scroll")// scroll ram + ???
	AM_RANGE(0xe000, 0xefff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( taitosj_audio_map, AS_PROGRAM, 8, taitosj_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4800, 0x4801) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x4801, 0x4801) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x4802, 0x4803) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
	AM_RANGE(0x4803, 0x4803) AM_DEVREAD("ay3", ay8910_device, data_r)
	AM_RANGE(0x4804, 0x4805) AM_DEVWRITE("ay4", ay8910_device, address_data_w)
	AM_RANGE(0x4805, 0x4805) AM_DEVREAD("ay4", ay8910_device, data_r)
	AM_RANGE(0x5000, 0x5000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xefff) AM_ROM /* space for diagnostic ROM */
ADDRESS_MAP_END


static ADDRESS_MAP_START( taitosj_mcu_map, AS_PROGRAM, 8, taitosj_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(taitosj_68705_portA_r, taitosj_68705_portA_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(taitosj_68705_portB_r, taitosj_68705_portB_w)
	AM_RANGE(0x0002, 0x0002) AM_READ(taitosj_68705_portC_r)
	AM_RANGE(0x0003, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END


#define DSW2_PORT \
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) \
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) ) \
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) ) \
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) ) \
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) ) \
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) ) \
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) ) \
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) ) \
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) ) \
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) ) \
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) ) \
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) ) \
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) ) \
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) ) \
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

#define COMMON_IN0\
	PORT_START("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define COMMON_IN1\
	PORT_START("IN1")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define COMMON_IN2\
	PORT_START("IN2")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

#define COMMON_IN3(coin3state)\
	PORT_START("IN3")      /* Service */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x10, coin3state, IPT_COIN3 )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define WWEST_INPUT1\
	PORT_START("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_START("IN1")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

#define WWEST_INPUT2\
	PORT_START("IN3")      /* Service */\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY\
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN3 )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_START("IN4")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL\
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL )   // from sound CPU

static INPUT_PORTS_START( spaceskr )
	COMMON_IN0

	COMMON_IN1

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_HIGH)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( spacecr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_LOW)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( junglek )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_HIGH)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Finish Bonus" )
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )
	PORT_DIPSETTING(    0x02, "Timer x1" )
	PORT_DIPSETTING(    0x01, "Timer x2" )
	PORT_DIPSETTING(    0x00, "Timer x3" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Infinite Lives (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( piratpet )
	PORT_INCLUDE( junglek )

	/* These 'skip' bits actually work no matter how you set the Debug Dip!          */
	/* If cabinet is upright -> only 1 set of controls -> only P1 Skip Next Level    */
	/* If cabinet is cocktal -> 2 sets of controls -> both Skip Next Level bits work */
	PORT_MODIFY("IN0")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("P1 Skip Next Level") // Button 2 skips levels when Debug dips is on

	PORT_MODIFY("IN1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("P2 Skip Next Level") // Button 2 skips levels when Debug dips is on

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, "Debug Mode" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPSETTING(    0x03, DEF_STR( None ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x14, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Free Game (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( alpine )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL      // "Fast"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )                // "Fast"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x1e, 0x00, IPT_UNUSED )                      /* protection read */
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	COMMON_IN3(IP_ACTIVE_LOW) //Tilt flips screen

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Jump Bonus" )
	PORT_DIPSETTING(    0x00, "500-1500" )
	PORT_DIPSETTING(    0x01, "800-2000" )
	PORT_DIPSETTING(    0x02, "1000-2500" )
	PORT_DIPSETTING(    0x03, "2000-4000" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x18, 0x18, "Time" )
	PORT_DIPSETTING(    0x00, "1:00" )
	PORT_DIPSETTING(    0x08, "1:30" )
	PORT_DIPSETTING(    0x10, "2:00" )
	PORT_DIPSETTING(    0x18, "2:30" )
	PORT_DIPNAME( 0x20, 0x00, "End of Race Time Bonus" )
	PORT_DIPSETTING(    0x20, "0:10" )
	PORT_DIPSETTING(    0x00, "0:20" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "1st Extended Time" )
	PORT_DIPSETTING(    0x00, "10k" )
	PORT_DIPSETTING(    0x01, "15k" )
	PORT_DIPSETTING(    0x02, "20k" )
	PORT_DIPSETTING(    0x03, "25k" )
	PORT_DIPNAME( 0x1c, 0x1c, "Extended Time Every" )
	PORT_DIPSETTING(    0x00, "5k" )
	PORT_DIPSETTING(    0x04, "6k" )
	PORT_DIPSETTING(    0x08, "7k" )
	PORT_DIPSETTING(    0x0c, "8k" )
	PORT_DIPSETTING(    0x10, "9k" )
	PORT_DIPSETTING(    0x14, "10k" )
	PORT_DIPSETTING(    0x18, "11k" )
	PORT_DIPSETTING(    0x1c, "12k" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( alpinea )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL      // "Fast"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )                // "Fast"
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0f, 0x00, IPT_UNUSED )                      /* protection read */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	COMMON_IN3(IP_ACTIVE_LOW) //Tilt flips screen

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Jump Bonus" )
	PORT_DIPSETTING(    0x00, "500-1500" )
	PORT_DIPSETTING(    0x01, "800-2000" )
	PORT_DIPSETTING(    0x02, "1000-2500" )
	PORT_DIPSETTING(    0x03, "2000-4000" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x18, 0x18, "Time" )
	PORT_DIPSETTING(    0x00, "1:00" )
	PORT_DIPSETTING(    0x08, "1:30" )
	PORT_DIPSETTING(    0x10, "2:00" )
	PORT_DIPSETTING(    0x18, "2:30" )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "1st Extended Time" )
	PORT_DIPSETTING(    0x00, "10k" )
	PORT_DIPSETTING(    0x01, "15k" )
	PORT_DIPSETTING(    0x02, "20k" )
	PORT_DIPSETTING(    0x03, "25k" )
	PORT_DIPNAME( 0x1c, 0x1c, "Extended Time Every" )
	PORT_DIPSETTING(    0x00, "5k" )
	PORT_DIPSETTING(    0x04, "6k" )
	PORT_DIPSETTING(    0x08, "7k" )
	PORT_DIPSETTING(    0x0c, "8k" )
	PORT_DIPSETTING(    0x10, "9k" )
	PORT_DIPSETTING(    0x14, "10k" )
	PORT_DIPSETTING(    0x18, "11k" )
	PORT_DIPSETTING(    0x1c, "12k" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( timetunl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_LOW)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x10, "Coins/Credits" )
	PORT_DIPSETTING(    0x00, "Insert Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( wwestern )
	WWEST_INPUT1

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1c, 0x18, IPT_UNUSED )                  /* protection read, the game resets after a while without it */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	WWEST_INPUT2

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x03, "10000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x01, "50000" )
	PORT_DIPSETTING(    0x00, "70000" )
	PORT_DIPNAME( 0x04, 0x00, "High Score Table" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( frontlin )
	WWEST_INPUT1

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	WWEST_INPUT2

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x01, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )         PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )        PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "Coins/Credits" )
	PORT_DIPSETTING(    0x00, "Insert Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )           PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( elevator )
	COMMON_IN0

	COMMON_IN1

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_HIGH)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )    PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "10000" )
	PORT_DIPSETTING(    0x02, "15000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x00, "25000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )         PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )   PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )    PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )        PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, "Coins/Credits" )
	PORT_DIPSETTING(    0x00, "Insert Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )           PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( tinstar )
	WWEST_INPUT1

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN3")      /* Service */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Bonus Life?" )
	PORT_DIPSETTING(    0x03, "10000?" )
	PORT_DIPSETTING(    0x02, "20000?" )
	PORT_DIPSETTING(    0x01, "30000?" )
	PORT_DIPSETTING(    0x00, "50000?" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Star Sound" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x10, "Coins/Credits" )
	PORT_DIPSETTING(    0x00, "Insert Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( waterski )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Slow")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Jump")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Slow")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Jump")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_HIGH)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Time" )
	PORT_DIPSETTING(    0x00, "2:00" )
	PORT_DIPSETTING(    0x08, "2:10" )
	PORT_DIPSETTING(    0x10, "2:20" )
	PORT_DIPSETTING(    0x18, "2:30" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x10, "Coins/Credits" )
	PORT_DIPSETTING(    0x00, "Insert Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( bioatack )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	COMMON_IN3(IP_ACTIVE_LOW)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")      /* d50a */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x03, "5000" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x01, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( sfposeid )
	COMMON_IN0

	COMMON_IN1

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_HIGH)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x10, "Coins/Credits" )
	PORT_DIPSETTING(    0x00, "Insert Coin" )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( hwrace )
	COMMON_IN0

	COMMON_IN1

	COMMON_IN2

	COMMON_IN3(IP_ACTIVE_HIGH)

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,input_port_4_f0_r, NULL)    // from sound CPU

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x03, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )
INPUT_PORTS_END

static INPUT_PORTS_START( kikstart )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN3")      /* Service */
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,kikstart_gear_r, (void *)0)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* needs to be 0, otherwise cannot shift */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taitosj_state,kikstart_gear_r, (void *)1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* needs to be 0, otherwise cannot shift */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME(0x03, 0x01, "Gate Bonus" )
	PORT_DIPSETTING(   0x00, "5k Points" )
	PORT_DIPSETTING(   0x01, "10k Points" )
	PORT_DIPSETTING(   0x02, "15k Points" )
	PORT_DIPSETTING(   0x03, "20k Points" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME(0x18, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(   0x18, DEF_STR( Easy ) )      /* 3:00 */
	PORT_DIPSETTING(   0x10, DEF_STR( Normal ) )        /* 2:30 */
	PORT_DIPSETTING(   0x08, DEF_STR( Difficult ) )     /* 2:00 */
	PORT_DIPSETTING(   0x00, DEF_STR( Very_Difficult ) )    /* 1:30 */
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")      /* Coinage */
	DSW2_PORT

	PORT_START("DSW3")
	PORT_DIPNAME( 0x08, 0x08, "Control Type" )
	PORT_DIPSETTING(    0x08, "Revolve" )
	PORT_DIPSETTING(    0x00, "Buttons" )
	PORT_DIPNAME( 0x10, 0x10, "Coinage Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Year Display" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "No Hit (Cheat)") //?
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x80, "A and B" )
	PORT_DIPSETTING(    0x00, "A only" )

	/* fake for handling the gears */
	PORT_START("GEARP1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P1 1st Gear")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 2nd Gear")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P1 3rd Gear")

	PORT_START("GEARP2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("P2 1st Gear") PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P2 2nd Gear") PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("P2 3rd Gear") PORT_COCKTAIL
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{ 512*8*8, 256*8*8, 0 },        /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};
static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	64,             /* 64 sprites */
	3,      /* 3 bits per pixel */
	{ 128*16*16, 64*16*16, 0 },     /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0,
		8*8+7, 8*8+6, 8*8+5, 8*8+4, 8*8+3, 8*8+2, 8*8+1, 8*8+0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};



static GFXDECODE_START( taitosj )
	GFXDECODE_ENTRY( NULL, 0x9000, charlayout,   0, 8 )    /* the game dynamically modifies this */
	GFXDECODE_ENTRY( NULL, 0x9000, spritelayout, 0, 8 )    /* the game dynamically modifies this */
	GFXDECODE_ENTRY( NULL, 0xa800, charlayout,   0, 8 )    /* the game dynamically modifies this */
	GFXDECODE_ENTRY( NULL, 0xa800, spritelayout, 0, 8 )    /* the game dynamically modifies this */
GFXDECODE_END


static const UINT8 voltable[256] =
{
	0xff,0xfe,0xfc,0xfb,0xf9,0xf7,0xf6,0xf4,0xf3,0xf2,0xf1,0xef,0xee,0xec,0xeb,0xea,
	0xe8,0xe7,0xe5,0xe4,0xe2,0xe1,0xe0,0xdf,0xde,0xdd,0xdc,0xdb,0xd9,0xd8,0xd7,0xd6,
	0xd5,0xd4,0xd3,0xd2,0xd1,0xd0,0xcf,0xce,0xcd,0xcc,0xcb,0xca,0xc9,0xc8,0xc7,0xc6,
	0xc5,0xc4,0xc3,0xc2,0xc1,0xc0,0xbf,0xbf,0xbe,0xbd,0xbc,0xbb,0xba,0xba,0xb9,0xb8,
	0xb7,0xb7,0xb6,0xb5,0xb4,0xb3,0xb3,0xb2,0xb1,0xb1,0xb0,0xaf,0xae,0xae,0xad,0xac,
	0xab,0xaa,0xaa,0xa9,0xa8,0xa8,0xa7,0xa6,0xa6,0xa5,0xa5,0xa4,0xa3,0xa2,0xa2,0xa1,
	0xa1,0xa0,0xa0,0x9f,0x9e,0x9e,0x9d,0x9d,0x9c,0x9c,0x9b,0x9b,0x9a,0x99,0x99,0x98,
	0x97,0x97,0x96,0x96,0x95,0x95,0x94,0x94,0x93,0x93,0x92,0x92,0x91,0x91,0x90,0x90,
	0x8b,0x8b,0x8a,0x8a,0x89,0x89,0x89,0x88,0x88,0x87,0x87,0x87,0x86,0x86,0x85,0x85,
	0x84,0x84,0x83,0x83,0x82,0x82,0x82,0x81,0x81,0x81,0x80,0x80,0x7f,0x7f,0x7f,0x7e,
	0x7e,0x7e,0x7d,0x7d,0x7c,0x7c,0x7c,0x7b,0x7b,0x7b,0x7a,0x7a,0x7a,0x79,0x79,0x79,
	0x78,0x78,0x77,0x77,0x77,0x76,0x76,0x76,0x75,0x75,0x75,0x74,0x74,0x74,0x73,0x73,
	0x73,0x73,0x72,0x72,0x72,0x71,0x71,0x71,0x70,0x70,0x70,0x70,0x6f,0x6f,0x6f,0x6e,
	0x6e,0x6e,0x6d,0x6d,0x6d,0x6c,0x6c,0x6c,0x6c,0x6b,0x6b,0x6b,0x6b,0x6a,0x6a,0x6a,
	0x6a,0x69,0x69,0x69,0x68,0x68,0x68,0x68,0x68,0x67,0x67,0x67,0x66,0x66,0x66,0x66,
	0x65,0x65,0x65,0x65,0x64,0x64,0x64,0x64,0x64,0x63,0x63,0x63,0x63,0x62,0x62,0x62,
};


WRITE8_MEMBER(taitosj_state::dac_out_w)
{
	m_dac_out = data - 0x80;
	m_dac->write_signed16(m_dac_out * m_dac_vol + 0x8000);
}

WRITE8_MEMBER(taitosj_state::dac_vol_w)
{
	m_dac_vol = voltable[data];
	m_dac->write_signed16(m_dac_out * m_dac_vol + 0x8000);
}

static MACHINE_CONFIG_START( nomcu, taitosj_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,8000000/2)      /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(taitosj_main_nomcu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taitosj_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,6000000/2)    /* 3 MHz */
	MCFG_CPU_PROGRAM_MAP(taitosj_audio_map)
			/* interrupts: */
			/* - no interrupts synced with vblank */
			/* - NMI triggered by the main CPU */
			/* - periodic IRQ, with frequency 6000000/(4*16*16*10*16) = 36.621 Hz, */
	MCFG_CPU_PERIODIC_INT_DRIVER(taitosj_state, irq0_line_hold,  (double)6000000/(4*16*16*10*16))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(taitosj_state, screen_update_taitosj)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", taitosj)
	MCFG_PALETTE_ADD("palette", 64)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 6000000/4)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW2"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW3"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay2", AY8910, 6000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(taitosj_state, dac_out_w))   /* port Awrite */
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(taitosj_state, dac_vol_w))    /* port Bwrite */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay3", AY8910, 6000000/4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(taitosj_state, input_port_4_f0_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ay4", AY8910, 6000000/4)
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(taitosj_state, taitosj_sndnmi_msk_w)) /* port Bwrite */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)
MACHINE_CONFIG_END


/* same as above, but with additional 68705 MCU */
static MACHINE_CONFIG_DERIVED( mcu, nomcu )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(taitosj_main_mcu_map)

	MCFG_CPU_ADD("mcu", M68705,3000000)      /* xtal is 3MHz, divided by 4 internally */
	MCFG_CPU_PROGRAM_MAP(taitosj_mcu_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( kikstart, mcu )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(kikstart_main_map)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(taitosj_state, screen_update_kikstart)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( spaceskr )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "eb01",         0x0000, 0x1000, CRC(92345b05) SHA1(c4e211c89185a9f9a0eeae87af0bc4eb4e0653e7) )
	ROM_LOAD( "eb02",         0x1000, 0x1000, CRC(a3e21420) SHA1(02b6a6a7626b89be9cc9ee6f4b7f0b94ad328c68) )
	ROM_LOAD( "eb03",         0x2000, 0x1000, CRC(a077c52f) SHA1(cb50e3c1082be54e2239efff4e9fc9160ad6aad8) )
	ROM_LOAD( "eb04",         0x3000, 0x1000, CRC(440030cf) SHA1(3e6a512137d81ca0400a6311961df36546f6f6e3) )
	ROM_LOAD( "eb05",         0x4000, 0x1000, CRC(b0d396ab) SHA1(42d3ad74d6065947f1d25c615fd3af171d952a80) )
	ROM_LOAD( "eb06",         0x5000, 0x1000, CRC(371d2f7a) SHA1(bd323063cc9cad30d4cdafa1a5be29c3153f6c7b) )
	ROM_LOAD( "eb07",         0x6000, 0x1000, CRC(13e667c4) SHA1(0b52eee7f8ed688c497bc60482b02a03f67807ce) )
	ROM_LOAD( "eb08",         0x7000, 0x1000, CRC(f2e84015) SHA1(e51450e3b173d3d6b60026e4d32307781db33c13) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "eb13",         0x0000, 0x1000, CRC(192f6536) SHA1(343d66a44568684ad44b7077fa8f378490fc9df4) )
	ROM_LOAD( "eb14",         0x1000, 0x1000, CRC(d04d0a21) SHA1(7fb66e6b4923329df4f28342e2923fc9c1d0bcc3) )
	ROM_LOAD( "eb15",         0x2000, 0x1000, CRC(88194305) SHA1(18d3e1b72a1eb64594bc9b89b8acb1dbafeb811b) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "eb09",         0x0000, 0x1000, CRC(77af540e) SHA1(819463bcc8d808806e4294f72e20ac528f9691b3) )
	ROM_LOAD( "eb10",         0x1000, 0x1000, CRC(b10073de) SHA1(1839f49a11b3afa7b1d73f53e8b706490d687f78) )
	ROM_LOAD( "eb11",         0x2000, 0x1000, CRC(c7954bd1) SHA1(c4bcaeb4b9d3aa50e4b41a8d1913b248bae2bd02) )
	ROM_LOAD( "eb12",         0x3000, 0x1000, CRC(cd6c087b) SHA1(92aaa277381937f5d5a5708e15b71023d8e9c545) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( spacecr )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "cg01.69",         0x0000, 0x1000, CRC(2fe28b71) SHA1(868b109c16fb5ebee576b90392c6ebfec37d4139) )
	ROM_LOAD( "cg02.68",         0x1000, 0x1000, CRC(88f4f856) SHA1(f077c42e7ac865875355bcf76483fbab3d66eb38) )
	ROM_LOAD( "cg03.67",         0x2000, 0x1000, CRC(2223319c) SHA1(4a5147473a11cb8da56de9e835dacb5b3ce9b084) )
	ROM_LOAD( "cg04.66",         0x3000, 0x1000, CRC(4daeb8b5) SHA1(646e2d819d2727395d13a38a3560e5a71db700fa) )
	ROM_LOAD( "cg05.65",         0x4000, 0x1000, CRC(cdc40ca0) SHA1(f79aa1a778190ee6e30d0b78643286cbf64dca45) )
	ROM_LOAD( "cg06.64",         0x5000, 0x1000, CRC(2cc6b4c0) SHA1(e3f74fb480c265f75d0e49cd60d7cfc6e1e37eb4) )
	ROM_LOAD( "cg07.55",         0x6000, 0x1000, CRC(e4c8780a) SHA1(49d6f3d875a83584514bea8a0f6cff175f5030f5) )
	ROM_LOAD( "cg08.54",         0x7000, 0x1000, CRC(2c23ff4d) SHA1(8dcbd394e241587401db4199ac58138b1142a07a) )
	ROM_LOAD( "cg09.53",         0x10000, 0x1000, CRC(3c8bb95e) SHA1(0bd2bedb7ce2176943fdb0cd640549f09b8807fa) ) /* banked at 6000 */
	ROM_LOAD( "cg10.52",         0x11000, 0x1000, CRC(0ff17fce) SHA1(e567754b2b55489ab63ebafc5ad0cc3853d9c8a1) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "cg17.70",         0x0000, 0x1000, CRC(53486204) SHA1(36d373a5bfc5cf4fda84eb8011177737ee04acdd) )
	ROM_LOAD( "cg18.71",         0x1000, 0x1000, CRC(d1acf96c) SHA1(117a6ed2b5039bf072fb1ee59c5307ec9a2883b3) )
	ROM_LOAD( "cg19.72",         0x2000, 0x1000, CRC(ffd27215) SHA1(c0fb2dcfcc62e694ed11bd116b992859e10de55a) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "cg11.1",         0x0000, 0x1000, CRC(1e4ae527) SHA1(cac5a16a86c53e5e85a4ce323554bb0d6173622d) )
	ROM_LOAD( "cg12.2",         0x1000, 0x1000, CRC(aa57b616) SHA1(a004df9631dd13f083886cc76652166ee8c6da2c) )
	ROM_LOAD( "cg13.3",         0x2000, 0x1000, CRC(945a1b69) SHA1(6e87748733ee95c2ec360e7fb3c24059ddd72468) )
	ROM_LOAD( "cg14.4",         0x3000, 0x1000, CRC(1a29d06b) SHA1(76e866cf160bcbc353dec1d30d636c3f2f1b0ffe) )
	ROM_LOAD( "cg15.5",         0x4000, 0x1000, CRC(656f9713) SHA1(0f1bf28d9dfa50a3098820cfbd5271bc1cdc987b) )
	ROM_LOAD( "cg16.6",         0x5000, 0x1000, CRC(e2c0d585) SHA1(9ff848758b5b41b59eb4a48191c37d018810017e) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END


ROM_START( junglek )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "kn21-1.bin",   0x00000, 0x1000, CRC(45f55d30) SHA1(bb9518d7728938f673a663801e47ae0438cdbea1) )
	ROM_LOAD( "kn22-1.bin",   0x01000, 0x1000, CRC(07cc9a21) SHA1(3fe35935e0a430ab0edc6a762623972fa37ea926) )
	ROM_LOAD( "kn43.bin",     0x02000, 0x1000, CRC(a20e5a48) SHA1(af961b671dc4c865d0181d08a70b902bb96f29d0) )
	ROM_LOAD( "kn24.bin",     0x03000, 0x1000, CRC(19ea7f83) SHA1(2399cc89f73811575c3f644d5c04ef13ceec6838) )
	ROM_LOAD( "kn25.bin",     0x04000, 0x1000, CRC(844365ea) SHA1(af34712620e4b784a5014283d3111048c5f81a56) )
	ROM_LOAD( "kn46.bin",     0x05000, 0x1000, CRC(27a95fd5) SHA1(160ee5d11126ac4155b479e43ec1bd6a4e9e21e7) )
	ROM_LOAD( "kn47.bin",     0x06000, 0x1000, CRC(5c3199e0) SHA1(c57dec92998b971d76aecd23674c25cf7b8be667) )
	ROM_LOAD( "kn28.bin",     0x07000, 0x1000, CRC(194a2d09) SHA1(88999493e470acdcf932efff71cd6155387a63d0) )
	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "kn60.bin",     0x11000, 0x1000, CRC(1a9c0a26) SHA1(82f4cebeba90419e83a00427b671985824babd7a) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kn37.bin",     0x0000, 0x1000, CRC(dee7f5d4) SHA1(cd8179a17ccd054fb470c4eee97192c2dd226397) )
	ROM_LOAD( "kn38.bin",     0x1000, 0x1000, CRC(bffd3d21) SHA1(a2b3393e9694d6979d39ab0f1ab82b7ef892b3da) )
	ROM_LOAD( "kn59-1.bin",   0x2000, 0x1000, CRC(cee485fc) SHA1(1e0c52ec6b1d3cfd47247db71bcf3fe476c32039) )

	ROM_REGION( 0x8000, "gfx1", 0 )   /* graphic ROMs used at runtime */
	ROM_LOAD( "kn29.bin",     0x0000, 0x1000, CRC(8f83c290) SHA1(aa95ed2d2e15f573e092e8eed7d80479512d9409) )
	ROM_LOAD( "kn30.bin",     0x1000, 0x1000, CRC(89fd19f1) SHA1(fc7dfe3a1d78ac37a036fa9d8ebf3a33a2f4cbe8) )
	ROM_LOAD( "kn51.bin",     0x2000, 0x1000, CRC(70e8fc12) SHA1(505c90c662d372d28cb38201433054b8e3d723d1) )
	ROM_LOAD( "kn52.bin",     0x3000, 0x1000, CRC(bcbac1a3) SHA1(bcd5fc9b3791ab67e0ad9f9ced7226853e9a2a00) )
	ROM_LOAD( "kn53.bin",     0x4000, 0x1000, CRC(b946c87d) SHA1(d16cb6bf38e00ae11c204cbf8f400f8a85c807c2) )
	ROM_LOAD( "kn34.bin",     0x5000, 0x1000, CRC(320db2e1) SHA1(ca8722010712302b491eb5f51d73043bcb2ddc8f) )
	ROM_LOAD( "kn55.bin",     0x6000, 0x1000, CRC(70aef58f) SHA1(df7454a1c3676181eca698bb3b2ef3253a45ca0f) )
	ROM_LOAD( "kn56.bin",     0x7000, 0x1000, CRC(932eb667) SHA1(4bf7c01ab212b616931a21a43a453521aa01ff36) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( junglekas ) /* alternate sound - no tarzan yell */
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "kn21-1.bin",   0x00000, 0x1000, CRC(45f55d30) SHA1(bb9518d7728938f673a663801e47ae0438cdbea1) )
	ROM_LOAD( "kn22-1.bin",   0x01000, 0x1000, CRC(07cc9a21) SHA1(3fe35935e0a430ab0edc6a762623972fa37ea926) )
	ROM_LOAD( "kn43.bin",     0x02000, 0x1000, CRC(a20e5a48) SHA1(af961b671dc4c865d0181d08a70b902bb96f29d0) )
	ROM_LOAD( "kn24.bin",     0x03000, 0x1000, CRC(19ea7f83) SHA1(2399cc89f73811575c3f644d5c04ef13ceec6838) )
	ROM_LOAD( "kn25.bin",     0x04000, 0x1000, CRC(844365ea) SHA1(af34712620e4b784a5014283d3111048c5f81a56) )
	ROM_LOAD( "kn46.bin",     0x05000, 0x1000, CRC(27a95fd5) SHA1(160ee5d11126ac4155b479e43ec1bd6a4e9e21e7) )
	ROM_LOAD( "kn47.bin",     0x06000, 0x1000, CRC(5c3199e0) SHA1(c57dec92998b971d76aecd23674c25cf7b8be667) )
	ROM_LOAD( "kn28.bin",     0x07000, 0x1000, CRC(194a2d09) SHA1(88999493e470acdcf932efff71cd6155387a63d0) )
	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "kn60.bin",     0x11000, 0x1000, CRC(1a9c0a26) SHA1(82f4cebeba90419e83a00427b671985824babd7a) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kn-a17.bin",   0x0000, 0x1000, CRC(62f6763a) SHA1(84eadbc5c6a37c53c104e4ac1fd273b6b2a335e5) )
	ROM_LOAD( "kn-a18.bin",   0x1000, 0x1000, CRC(8a813a7c) SHA1(e1c1bba0e793afeebd463d5cba9ff4110af238b2) )
	ROM_LOAD( "kn-a19.bin",   0x2000, 0x1000, CRC(abbe4ae5) SHA1(992db7b1748a8c1c5093fd8441de512794380478) )

	ROM_REGION( 0x8000, "gfx1", 0 )   /* graphic ROMs used at runtime */
	ROM_LOAD( "kn29.bin",     0x0000, 0x1000, CRC(8f83c290) SHA1(aa95ed2d2e15f573e092e8eed7d80479512d9409) )
	ROM_LOAD( "kn30.bin",     0x1000, 0x1000, CRC(89fd19f1) SHA1(fc7dfe3a1d78ac37a036fa9d8ebf3a33a2f4cbe8) )
	ROM_LOAD( "kn51.bin",     0x2000, 0x1000, CRC(70e8fc12) SHA1(505c90c662d372d28cb38201433054b8e3d723d1) )
	ROM_LOAD( "kn52.bin",     0x3000, 0x1000, CRC(bcbac1a3) SHA1(bcd5fc9b3791ab67e0ad9f9ced7226853e9a2a00) )
	ROM_LOAD( "kn53.bin",     0x4000, 0x1000, CRC(b946c87d) SHA1(d16cb6bf38e00ae11c204cbf8f400f8a85c807c2) )
	ROM_LOAD( "kn34.bin",     0x5000, 0x1000, CRC(320db2e1) SHA1(ca8722010712302b491eb5f51d73043bcb2ddc8f) )
	ROM_LOAD( "kn55.bin",     0x6000, 0x1000, CRC(70aef58f) SHA1(df7454a1c3676181eca698bb3b2ef3253a45ca0f) )
	ROM_LOAD( "kn56.bin",     0x7000, 0x1000, CRC(932eb667) SHA1(4bf7c01ab212b616931a21a43a453521aa01ff36) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

// seems to be based on the junglek - Jungle King (Japan) revision
// this is sometimes sold as a prototype, it isn't, it's a cheaply hacked bootleg - it isn't even based on the oldest code revision!
ROM_START( jungleby )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "j1.bin",   0x00000, 0x1000, CRC(6f2ac11f) SHA1(c6592d369449140ed4a8dffea76f4809e0f76f06) ) // modified copyright
	ROM_LOAD( "j2.bin",   0x01000, 0x1000, CRC(07cc9a21) SHA1(3fe35935e0a430ab0edc6a762623972fa37ea926) )
	ROM_LOAD( "j3.bin",   0x02000, 0x1000, CRC(a20e5a48) SHA1(af961b671dc4c865d0181d08a70b902bb96f29d0) )
	ROM_LOAD( "j4.bin",   0x03000, 0x1000, CRC(19ea7f83) SHA1(2399cc89f73811575c3f644d5c04ef13ceec6838) )
	ROM_LOAD( "j5.bin",   0x04000, 0x1000, CRC(844365ea) SHA1(af34712620e4b784a5014283d3111048c5f81a56) )
	ROM_LOAD( "j6.bin",   0x05000, 0x1000, CRC(27a95fd5) SHA1(160ee5d11126ac4155b479e43ec1bd6a4e9e21e7) )
	ROM_LOAD( "j7.bin",   0x06000, 0x1000, CRC(5c3199e0) SHA1(c57dec92998b971d76aecd23674c25cf7b8be667) )
	ROM_LOAD( "j8.bin",   0x07000, 0x1000, CRC(895e5708) SHA1(66261a266d026d90074eafb564d647bde3a7b736) ) // modified title logo

	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "j9.bin",     0x11000, 0x1000, CRC(1a9c0a26) SHA1(82f4cebeba90419e83a00427b671985824babd7a) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "j10.bin",   0x0000, 0x1000, CRC(dee7f5d4) SHA1(cd8179a17ccd054fb470c4eee97192c2dd226397) )
	ROM_LOAD( "j11.bin",   0x1000, 0x1000, CRC(bffd3d21) SHA1(a2b3393e9694d6979d39ab0f1ab82b7ef892b3da) )
	ROM_LOAD( "j12.bin",   0x2000, 0x1000, CRC(cee485fc) SHA1(1e0c52ec6b1d3cfd47247db71bcf3fe476c32039) )

	ROM_REGION( 0x8000, "gfx1", 0 )   /* graphic ROMs used at runtime */
	ROM_LOAD( "j13.bin",     0x0000, 0x1000, CRC(8f83c290) SHA1(aa95ed2d2e15f573e092e8eed7d80479512d9409) )
	ROM_LOAD( "j14.bin",     0x1000, 0x1000, CRC(89fd19f1) SHA1(fc7dfe3a1d78ac37a036fa9d8ebf3a33a2f4cbe8) )
	ROM_LOAD( "j15.bin",     0x2000, 0x1000, CRC(70e8fc12) SHA1(505c90c662d372d28cb38201433054b8e3d723d1) )
	ROM_LOAD( "j16.bin",     0x3000, 0x1000, CRC(bcbac1a3) SHA1(bcd5fc9b3791ab67e0ad9f9ced7226853e9a2a00) )
	ROM_LOAD( "j17.bin",     0x4000, 0x1000, CRC(b946c87d) SHA1(d16cb6bf38e00ae11c204cbf8f400f8a85c807c2) )
	ROM_LOAD( "j18.bin",     0x5000, 0x1000, CRC(320db2e1) SHA1(ca8722010712302b491eb5f51d73043bcb2ddc8f) )
	ROM_LOAD( "j19.bin",     0x6000, 0x1000, CRC(8438eb41) SHA1(fc76747abe91e965390e06533c56b6cd4e2d62a0) ) // modified tiles for title logo
	ROM_LOAD( "j20.bin",     0x7000, 0x1000, CRC(932eb667) SHA1(4bf7c01ab212b616931a21a43a453521aa01ff36) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END


ROM_START( junglekj2 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "kn41.bin",     0x00000, 0x1000, CRC(7e4cd631) SHA1(512c08795d7946500b22d6f63a482c5156e6764b) )
	ROM_LOAD( "kn42.bin",     0x01000, 0x1000, CRC(bade53af) SHA1(c3d2cf776598cb2d8684fa0b3ea7af90af9e8dae) )
	ROM_LOAD( "kn43.bin",     0x02000, 0x1000, CRC(a20e5a48) SHA1(af961b671dc4c865d0181d08a70b902bb96f29d0) )
	ROM_LOAD( "kn44.bin",     0x03000, 0x1000, CRC(44c770d3) SHA1(57a1ddc07009f0dbd423cbe111b886e919a8bb0a) )
	ROM_LOAD( "kn45.bin",     0x04000, 0x1000, CRC(f60a3d06) SHA1(7c387f0aeb9497b026d8838ee6ea7ff11dea506a) )
	ROM_LOAD( "kn46.bin",     0x05000, 0x1000, CRC(27a95fd5) SHA1(160ee5d11126ac4155b479e43ec1bd6a4e9e21e7) )
	ROM_LOAD( "kn47.bin",     0x06000, 0x1000, CRC(5c3199e0) SHA1(c57dec92998b971d76aecd23674c25cf7b8be667) )
	ROM_LOAD( "kn48.bin",     0x07000, 0x1000, CRC(e690b36e) SHA1(25a6c06d6c2bf0082cc776255448c329cb2e74e0) )
	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "kn60.bin",     0x11000, 0x1000, CRC(1a9c0a26) SHA1(82f4cebeba90419e83a00427b671985824babd7a) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kn57-1.bin",   0x0000, 0x1000, CRC(62f6763a) SHA1(84eadbc5c6a37c53c104e4ac1fd273b6b2a335e5) )
	ROM_LOAD( "kn58-1.bin",   0x1000, 0x1000, CRC(9ef46c7f) SHA1(867d9352cde4d6496f59e790cbbf15302a55364e) )
	ROM_LOAD( "kn59-1.bin",   0x2000, 0x1000, CRC(cee485fc) SHA1(1e0c52ec6b1d3cfd47247db71bcf3fe476c32039) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "kn49.bin",     0x0000, 0x1000, CRC(fe275213) SHA1(5fcbe2db9371ae46610e7ce261498f3a9b4116ec) )
	ROM_LOAD( "kn50.bin",     0x1000, 0x1000, CRC(d9f93c55) SHA1(de04845a42b8214eceda1c9aa92af631f3236ee9) )
	ROM_LOAD( "kn51.bin",     0x2000, 0x1000, CRC(70e8fc12) SHA1(505c90c662d372d28cb38201433054b8e3d723d1) )
	ROM_LOAD( "kn52.bin",     0x3000, 0x1000, CRC(bcbac1a3) SHA1(bcd5fc9b3791ab67e0ad9f9ced7226853e9a2a00) )
	ROM_LOAD( "kn53.bin",     0x4000, 0x1000, CRC(b946c87d) SHA1(d16cb6bf38e00ae11c204cbf8f400f8a85c807c2) )
	ROM_LOAD( "kn54.bin",     0x5000, 0x1000, CRC(f757d8f0) SHA1(896118d990e3733aeb45842c0dc2103cbf2ba1a2) )
	ROM_LOAD( "kn55.bin",     0x6000, 0x1000, CRC(70aef58f) SHA1(df7454a1c3676181eca698bb3b2ef3253a45ca0f) )
	ROM_LOAD( "kn56.bin",     0x7000, 0x1000, CRC(932eb667) SHA1(4bf7c01ab212b616931a21a43a453521aa01ff36) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( jungleh )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "kn41a",        0x00000, 0x1000, CRC(6bf118d8) SHA1(d6de28766aab90b5dbca7f74612ec8eafd144348) )
	ROM_LOAD( "kn42.bin",     0x01000, 0x1000, CRC(bade53af) SHA1(c3d2cf776598cb2d8684fa0b3ea7af90af9e8dae) )
	ROM_LOAD( "kn43.bin",     0x02000, 0x1000, CRC(a20e5a48) SHA1(af961b671dc4c865d0181d08a70b902bb96f29d0) )
	ROM_LOAD( "kn44.bin",     0x03000, 0x1000, CRC(44c770d3) SHA1(57a1ddc07009f0dbd423cbe111b886e919a8bb0a) )
	ROM_LOAD( "kn45.bin",     0x04000, 0x1000, CRC(f60a3d06) SHA1(7c387f0aeb9497b026d8838ee6ea7ff11dea506a) )
	ROM_LOAD( "kn46a",        0x05000, 0x1000, CRC(ac89c155) SHA1(bac17c9828002b644f15933149a205a008a561d3) )
	ROM_LOAD( "kn47.bin",     0x06000, 0x1000, CRC(5c3199e0) SHA1(c57dec92998b971d76aecd23674c25cf7b8be667) )
	ROM_LOAD( "kn48a",        0x07000, 0x1000, CRC(ef80e931) SHA1(b3ddcc37860a2693d45a85970926662cbb96bd0e) )
	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "kn60.bin",     0x11000, 0x1000, CRC(1a9c0a26) SHA1(82f4cebeba90419e83a00427b671985824babd7a) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kn57-1.bin",   0x0000, 0x1000, CRC(62f6763a) SHA1(84eadbc5c6a37c53c104e4ac1fd273b6b2a335e5) )
	ROM_LOAD( "kn58-1.bin",   0x1000, 0x1000, CRC(9ef46c7f) SHA1(867d9352cde4d6496f59e790cbbf15302a55364e) )
	ROM_LOAD( "kn59-1.bin",   0x2000, 0x1000, CRC(cee485fc) SHA1(1e0c52ec6b1d3cfd47247db71bcf3fe476c32039) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "kn49a",        0x0000, 0x1000, CRC(b139e792) SHA1(10c39abc49786154396c00bd35a51b826e5f6bd0) )
	ROM_LOAD( "kn50a",        0x1000, 0x1000, CRC(1046019f) SHA1(b2d3ab8a53ef3ca55165a5bda9be0829f53be6c9) )
	ROM_LOAD( "kn51a",        0x2000, 0x1000, CRC(da50c8a4) SHA1(de5f9b953f277986679ab958772571d8417a0ce2) )
	ROM_LOAD( "kn52a",        0x3000, 0x1000, CRC(0444f06c) SHA1(80569807ae36b4c5ad90e9e736ce9d0d0ea486ec) )
	ROM_LOAD( "kn53a",        0x4000, 0x1000, CRC(6a17803e) SHA1(d7ab6a240bb1ac80d3903cb694e55fd6d3670faa) )
	ROM_LOAD( "kn54a",        0x5000, 0x1000, CRC(d41428c7) SHA1(8c926db731073313daced31a168da6ac07a6d5cb) )
	ROM_LOAD( "kn55.bin",     0x6000, 0x1000, CRC(70aef58f) SHA1(df7454a1c3676181eca698bb3b2ef3253a45ca0f) )
	ROM_LOAD( "kn56a",        0x7000, 0x1000, CRC(679c1101) SHA1(218cd75f77c858c3714a8f03aea2c7ee88a212dd) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( junglehbr )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "ic1.bin",      0x00000, 0x2000, CRC(3255a10e) SHA1(846448151e7db84b66ab6778c86c0b1bf8c3fec7) )
	ROM_LOAD( "ic2.bin",      0x02000, 0x2000, CRC(8482bc63) SHA1(56ddfc4df4867d81ad78c23fc80f53ff711dffd6) )
	ROM_LOAD( "ic3.bin",      0x04000, 0x2000, CRC(1abc661d) SHA1(58e63ac49de004e960e66fab261f405c96f1e758) )
	ROM_LOAD( "kn47.bin",     0x06000, 0x1000, CRC(5c3199e0) SHA1(c57dec92998b971d76aecd23674c25cf7b8be667) )
	ROM_LOAD( "kn48a",        0x07000, 0x1000, CRC(ef80e931) SHA1(b3ddcc37860a2693d45a85970926662cbb96bd0e) )
	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "kn60.bin",     0x11000, 0x1000, CRC(1a9c0a26) SHA1(82f4cebeba90419e83a00427b671985824babd7a) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "kn37.bin",     0x0000, 0x1000, CRC(dee7f5d4) SHA1(cd8179a17ccd054fb470c4eee97192c2dd226397) )
	ROM_LOAD( "kn38.bin",     0x1000, 0x1000, CRC(bffd3d21) SHA1(a2b3393e9694d6979d39ab0f1ab82b7ef892b3da) )
	ROM_LOAD( "kn59-1.bin",   0x2000, 0x1000, CRC(cee485fc) SHA1(1e0c52ec6b1d3cfd47247db71bcf3fe476c32039) )

	ROM_REGION( 0x8000, "gfx1", 0 )   /* graphic ROMs used at runtime */
	ROM_LOAD( "kn29.bin",     0x0000, 0x1000, CRC(8f83c290) SHA1(aa95ed2d2e15f573e092e8eed7d80479512d9409) )
	ROM_LOAD( "kn30.bin",     0x1000, 0x1000, CRC(89fd19f1) SHA1(fc7dfe3a1d78ac37a036fa9d8ebf3a33a2f4cbe8) )
	ROM_LOAD( "kn51.bin",     0x2000, 0x1000, CRC(70e8fc12) SHA1(505c90c662d372d28cb38201433054b8e3d723d1) )
	ROM_LOAD( "kn52.bin",     0x3000, 0x1000, CRC(bcbac1a3) SHA1(bcd5fc9b3791ab67e0ad9f9ced7226853e9a2a00) )
	ROM_LOAD( "kn53.bin",     0x4000, 0x1000, CRC(b946c87d) SHA1(d16cb6bf38e00ae11c204cbf8f400f8a85c807c2) )
	ROM_LOAD( "kn34.bin",     0x5000, 0x1000, CRC(320db2e1) SHA1(ca8722010712302b491eb5f51d73043bcb2ddc8f) )
	ROM_LOAD( "kn55.bin",     0x6000, 0x1000, CRC(70aef58f) SHA1(df7454a1c3676181eca698bb3b2ef3253a45ca0f) )
	ROM_LOAD( "kn56.bin",     0x7000, 0x1000, CRC(932eb667) SHA1(4bf7c01ab212b616931a21a43a453521aa01ff36) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( piratpet )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "pp0p_ic.69", 0x00000, 0x1000, CRC(8287dbc2) SHA1(bdaf6b875b91739d730675bd140288697dc13dc8) )
	ROM_LOAD( "pp1p_ic.68", 0x01000, 0x1000, CRC(27a90850) SHA1(8ba69ed4ebbb513ff8fc0c3e8f0835debb62f7ba) )
	ROM_LOAD( "pp2p_ic.67", 0x02000, 0x1000, CRC(d224fa85) SHA1(8867ee33c22e432632b7a709b721f7c07e26e001) )
	ROM_LOAD( "pp3p_ic.66", 0x03000, 0x1000, CRC(2c900874) SHA1(9505c7a7a2607144de5918525b06b36caa248f91) )
	ROM_LOAD( "pp4p_ic.65", 0x04000, 0x1000, CRC(1aed98d9) SHA1(0158952fa75d3c3c65d6efd2b9854802687d9377) )
	ROM_LOAD( "pp5p_ic.64", 0x05000, 0x1000, CRC(09c3aacd) SHA1(4dd1e4cad13b03f87fca041d41c8a4700560dfb8) )
	ROM_LOAD( "pp6p_ic.55", 0x06000, 0x1000, CRC(bdeed702) SHA1(65f82021ef15b3ba2e80321d688b5a50cce9e8d5) )
	ROM_LOAD( "pp7p_ic.54", 0x07000, 0x1000, CRC(5f36d082) SHA1(f9930197bcd36de69b7c99d50d1a0c4914ca3090) )
	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "pp7b_ic.52", 0x11000, 0x1000, CRC(bbc38b03) SHA1(1fac52ae6eb1f9874d11dcfaf69fc5cf3964979c) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "pp05_ic.70", 0x0000, 0x1000, CRC(dcb5eb9d) SHA1(79e01c12475ea3326efa446e7d2a64070f7d268e) )
	ROM_LOAD( "pp15_ic.71", 0x1000, 0x1000, CRC(3123dbe1) SHA1(0581be775d29fdbdbd8535a632dfa3f7d49c9d7d) )

	ROM_REGION( 0x8000, "gfx1", 0 ) /* graphic ROMs used at runtime */
	ROM_LOAD( "pp0e_ic.1", 0x0000, 0x1000, CRC(aceaf79b) SHA1(ef4c626a8d4e884e7d1600d69f36000316b1f213) )
	ROM_LOAD( "pp1e_ic.2", 0x1000, 0x1000, CRC(ac148214) SHA1(947e5795d9abd490b6519da33648a1f7385dc080) )
	ROM_LOAD( "pp2e_ic.3", 0x2000, 0x1000, CRC(108194d2) SHA1(49ddcaa83da7ae5658ce253a1cec9da864c5070c) )
	ROM_LOAD( "pp3e_ic.4", 0x3000, 0x1000, CRC(621b0da1) SHA1(eaec9f57031708c89e8f0ebe98973f793b7636fd) )
	ROM_LOAD( "pp4e_ic.5", 0x4000, 0x1000, CRC(e9826d90) SHA1(644ba76bb2860e4d5b20d2df2b9e5364b05db362) )
	ROM_LOAD( "pp5e_ic.6", 0x5000, 0x1000, CRC(fe0d38c6) SHA1(7ed0a8d800c4e079807cc0744832a4032f129e22) )
	ROM_LOAD( "pp6e_ic.7", 0x6000, 0x1000, CRC(2cfd127b) SHA1(bc353ac84342a11148d46a20281d9f17c0ee8903) )
	ROM_LOAD( "pp7e_ic.8", 0x7000, 0x1000, CRC(9857533f) SHA1(7682672fe651c2d54157f60a55d17bb79953d7ae) )

	ROM_REGION( 0x0100, "proms", 0 ) /* layer PROM */
	ROM_LOAD( "eb16.22", 0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( alpine )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "rh16.069",     0x0000, 0x1000, CRC(6b2a69b7) SHA1(d1904eac06f2ee0c491c2da04ec6191eb1ddca69) )
	ROM_LOAD( "rh17.068",     0x1000, 0x1000, CRC(e344b0b7) SHA1(a4f9c2b61d0d73c30f7e3a440b9c879c19809303) )
	ROM_LOAD( "rh18.067",     0x2000, 0x1000, CRC(753bdd87) SHA1(37b97dd4a5d53df9a86593fd0a53c95475fa09d0) )
	ROM_LOAD( "rh19.066",     0x3000, 0x1000, CRC(3efb3fcd) SHA1(29fb6405ced78662c4d98deeac5593d7bc42d954) )
	ROM_LOAD( "rh20.065",     0x4000, 0x1000, CRC(c2cd4e79) SHA1(0849aa0aa64c87b7f5c10f8a78caae8219059cfa) )
	ROM_LOAD( "rh21.064",     0x5000, 0x1000, CRC(74109145) SHA1(728714ec24962da1c54fc35dc3688d555a4ad101) )
	ROM_LOAD( "rh22.055",     0x6000, 0x1000, CRC(efa82a57) SHA1(b9b275014572c4c67558516d0c3c36d01e84e9ef) )
	ROM_LOAD( "rh23.054",     0x7000, 0x1000, CRC(77c25acf) SHA1(a48bf7044afa7388f68e05fdb2e63c2b04945462) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rh13.070",     0x0000, 0x1000, CRC(dcad1794) SHA1(1d5479f10cdcc437241bb17c22204fb3ee60f8cb) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "rh24.001",     0x0000, 0x1000, CRC(4b1d9455) SHA1(df68a1bae7f06dff9469f8ef3095a802af3cb354) )
	ROM_LOAD( "rh25.002",     0x1000, 0x1000, CRC(bf71e278) SHA1(0016e9fff506e3d1f6a9bd8ebb23b62af00902ca) )
	ROM_LOAD( "rh26.003",     0x2000, 0x1000, CRC(13da2a9b) SHA1(e3dd30a1036ec81b3867dc1c0d20449422d50c31) )
	ROM_LOAD( "rh27.004",     0x3000, 0x1000, CRC(425b52b0) SHA1(1a3046a7d12ad8107750abfb8a801cf9cd372d0f) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( alpinea )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "rh01-1.69",    0x0000, 0x1000, CRC(7fbcb635) SHA1(f1d7f21d98f3b899efbca446006c1a5979f2b94c) )
	ROM_LOAD( "rh02.68",      0x1000, 0x1000, CRC(c83f95af) SHA1(2bb538582d810e44c3093d4e4f73a527ca27d2f0) )
	ROM_LOAD( "rh03.67",      0x2000, 0x1000, CRC(211102bc) SHA1(4ed21b8ff90a501bf29f3d0842857db70703d990) )
	ROM_LOAD( "rh04-1.66",    0x3000, 0x1000, CRC(494a91b0) SHA1(f3a07a2a9091bb1fe9eeba62f2ecb9e2f9c8c033) )
	ROM_LOAD( "rh05.65",      0x4000, 0x1000, CRC(d85588be) SHA1(2c2fa519ea90c80984ab58645bcba148edf6d014) )
	ROM_LOAD( "rh06.64",      0x5000, 0x1000, CRC(521fddb9) SHA1(73540c8f4c15a990ee81e6cfeace94938afbad72) )
	ROM_LOAD( "rh07.55",      0x6000, 0x1000, CRC(51f369a4) SHA1(1bbc92955875794006f31dd63fbcc3b2e5e0de54) )
	ROM_LOAD( "rh08.54",      0x7000, 0x1000, CRC(e0af9cb2) SHA1(d657a8de11f202351571b822065a37ba911723c2) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rh13.070",     0x0000, 0x1000, CRC(dcad1794) SHA1(1d5479f10cdcc437241bb17c22204fb3ee60f8cb) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "rh24.001",     0x0000, 0x1000, CRC(4b1d9455) SHA1(df68a1bae7f06dff9469f8ef3095a802af3cb354) )
	ROM_LOAD( "rh25.002",     0x1000, 0x1000, CRC(bf71e278) SHA1(0016e9fff506e3d1f6a9bd8ebb23b62af00902ca) )
	ROM_LOAD( "rh26.003",     0x2000, 0x1000, CRC(13da2a9b) SHA1(e3dd30a1036ec81b3867dc1c0d20449422d50c31) )
	ROM_LOAD( "rh12.4",       0x3000, 0x1000, CRC(0ff0d1fe) SHA1(584b21b9114321439a722e35c6973e0513d696c0) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( timetunl )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "un01.69",      0x00000, 0x1000, CRC(2e56d946) SHA1(22b344b4467701d94bbd1ed7946a678893c92a75) )
	ROM_LOAD( "un02.68",      0x01000, 0x1000, CRC(f611d852) SHA1(c8709736c586b4288b19d0fbfc56ac9b988e7cdb) )
	ROM_LOAD( "un03.67",      0x02000, 0x1000, CRC(144b5e7f) SHA1(5920fd324253028bcca347b0279f24c665bcc7de) )
	ROM_LOAD( "un04.66",      0x03000, 0x1000, CRC(b6767eba) SHA1(76a005bf2984af3862ab46044dc124bc663b457e) )
	ROM_LOAD( "un05.65",      0x04000, 0x1000, CRC(91e3c558) SHA1(d7b402c58a70a99479accdfc3dd66c1bf6d2cdd6) )
	ROM_LOAD( "un06.64",      0x05000, 0x1000, CRC(af5a7d2a) SHA1(a214df8d29346f729502bb31e46b1c4e897ac5a1) )
	ROM_LOAD( "un07.55",      0x06000, 0x1000, CRC(4ee50999) SHA1(ea3f726c42c52aefcb3fb3cacffef32dfa86a9db) )
	ROM_LOAD( "un08.54",      0x07000, 0x1000, CRC(97259b57) SHA1(6c8c21e99fc4c59cd884a58b1995ecf7dca72206) )
	ROM_LOAD( "un09.53",      0x10000, 0x1000, CRC(771d0fb0) SHA1(5716df303a2b1119558d7061e262cbd39219a37e) ) /* banked at 6000 */
	ROM_LOAD( "un10.52",      0x11000, 0x1000, CRC(8b6afad2) SHA1(ff3e38182944c51e8e3b116eef304faa944910ee) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "un19.70",      0x0000, 0x1000, CRC(dbf726c6) SHA1(0c38cf641e23f98a885c20bf4e371b1af9660175) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "un11.1",       0x0000, 0x1000, CRC(3be4fed6) SHA1(fe5055d1f451d6779736e6cd7d6bd89dea6b11be) )
	ROM_LOAD( "un12.2",       0x1000, 0x1000, CRC(2dee1cf3) SHA1(d9f68f44e4f3fd4d5e95c3d87368319d1a7dd2d4) )
	ROM_LOAD( "un13.3",       0x2000, 0x1000, CRC(72b491a8) SHA1(c66e55aa7bb874053d57e6cc1c39a410ea8ee713) )
	ROM_LOAD( "un14.4",       0x3000, 0x1000, CRC(5f695369) SHA1(4b35df4475d94bdd37b62b59de85c4eb61b4f519) )
	ROM_LOAD( "un15.5",       0x4000, 0x1000, CRC(001df94b) SHA1(d6168179f7e7c4e747411381c214c9211ac6ef9f) )
	ROM_LOAD( "un16.6",       0x5000, 0x1000, CRC(e33b9019) SHA1(f9f55ec878ea7e1edac05694fe438e9826159674) )
	ROM_LOAD( "un17.7",       0x6000, 0x1000, CRC(d66025b8) SHA1(b0ca7176b38b2cf729816d796be9d50a39a5d7ee) )
	ROM_LOAD( "un18.8",       0x7000, 0x1000, CRC(e67ff377) SHA1(3dd14f55a0959684a3fb61997d78945b7326c7eb) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( wwestern )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "ww01.bin",     0x0000, 0x1000, CRC(bfe10753) SHA1(468cf5a2f7232b5faf4a371aee2c83196fc486a5) )
	ROM_LOAD( "ww02d.bin",    0x1000, 0x1000, CRC(20579e90) SHA1(be250b4b89d1742b4cffd108c32497a0db428335) )
	ROM_LOAD( "ww03d.bin",    0x2000, 0x1000, CRC(0e65be37) SHA1(5e672c6a33d9b68752c27465fd6172304de55d2d) )
	ROM_LOAD( "ww04d.bin",    0x3000, 0x1000, CRC(b3565a31) SHA1(a42a7989530108aa198d77e1369d5b2ca6c69907) )
	ROM_LOAD( "ww05d.bin",    0x4000, 0x1000, CRC(089f3d89) SHA1(aa8c9868ebc593eb1cd9af4d41e1aa0cbb8fb316) )
	ROM_LOAD( "ww06d.bin",    0x5000, 0x1000, CRC(c81c9736) SHA1(43dfdcd93f30a99ffb22d4d5fc9bc886276ab69c) )
	ROM_LOAD( "ww07.bin",     0x6000, 0x1000, CRC(1937cc17) SHA1(fcec4b6dafd631dd2b33db5852b5ae9412910527) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ww14.bin",     0x0000, 0x1000, CRC(23776870) SHA1(b15fe4f0fabd0939d87fe8c04c7edf74bbd6a23b) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "ww08.bin",     0x0000, 0x1000, CRC(041a5a1c) SHA1(c4006dc4915c6107a3f2e41534521385ba6c306c) )
	ROM_LOAD( "ww09.bin",     0x1000, 0x1000, CRC(07982ac5) SHA1(4ff76a53cb7af9ccf084513ee66c73bf16826959) )
	ROM_LOAD( "ww10.bin",     0x2000, 0x1000, CRC(f32ae203) SHA1(3455f99403246ce4483c3b6e954ebe6d93725ec3) )
	ROM_LOAD( "ww11.bin",     0x3000, 0x1000, CRC(7ff1431f) SHA1(945d7f58bd8855f046693eea7791c164bd1d7a3d) )
	ROM_LOAD( "ww12.bin",     0x4000, 0x1000, CRC(be1b563a) SHA1(447e1c982eea6198b99095e26aa5fec4e9ae1e54) )
	ROM_LOAD( "ww13.bin",     0x5000, 0x1000, CRC(092cd9e5) SHA1(56059bf41bb4ccf0f88cea679fdffc061e19f76e) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "ww17",         0x0000, 0x0100, CRC(93447d2b) SHA1(d29f4a56a06ac809b4b9efa8aa9d1f246250e3a2) )
ROM_END

ROM_START( wwestern1 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "ww01.bin",     0x0000, 0x1000, CRC(bfe10753) SHA1(468cf5a2f7232b5faf4a371aee2c83196fc486a5) )
	ROM_LOAD( "ww02",         0x1000, 0x1000, CRC(f011103a) SHA1(4d94140fb121abb366933bb56d216bdcf2b0a9f4) )
	ROM_LOAD( "ww03d.bin",    0x2000, 0x1000, CRC(0e65be37) SHA1(5e672c6a33d9b68752c27465fd6172304de55d2d) )
	ROM_LOAD( "ww04a",        0x3000, 0x1000, CRC(68b31a6e) SHA1(580e2b560275880ab8d75670f90d314341491953) )
	ROM_LOAD( "ww05",         0x4000, 0x1000, CRC(78293f81) SHA1(311d4200d3f9c1760f31a6c695b31f789ce4aadd) )
	ROM_LOAD( "ww06",         0x5000, 0x1000, CRC(d015e435) SHA1(2b0933348245f4359f5d054289c82647bab1e13d) )
	ROM_LOAD( "ww07.bin",     0x6000, 0x1000, CRC(1937cc17) SHA1(fcec4b6dafd631dd2b33db5852b5ae9412910527) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ww14.bin",     0x0000, 0x1000, CRC(23776870) SHA1(b15fe4f0fabd0939d87fe8c04c7edf74bbd6a23b) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "ww08.bin",     0x0000, 0x1000, CRC(041a5a1c) SHA1(c4006dc4915c6107a3f2e41534521385ba6c306c) )
	ROM_LOAD( "ww09.bin",     0x1000, 0x1000, CRC(07982ac5) SHA1(4ff76a53cb7af9ccf084513ee66c73bf16826959) )
	ROM_LOAD( "ww10.bin",     0x2000, 0x1000, CRC(f32ae203) SHA1(3455f99403246ce4483c3b6e954ebe6d93725ec3) )
	ROM_LOAD( "ww11.bin",     0x3000, 0x1000, CRC(7ff1431f) SHA1(945d7f58bd8855f046693eea7791c164bd1d7a3d) )
	ROM_LOAD( "ww12.bin",     0x4000, 0x1000, CRC(be1b563a) SHA1(447e1c982eea6198b99095e26aa5fec4e9ae1e54) )
	ROM_LOAD( "ww13.bin",     0x5000, 0x1000, CRC(092cd9e5) SHA1(56059bf41bb4ccf0f88cea679fdffc061e19f76e) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "ww17",         0x0000, 0x0100, CRC(93447d2b) SHA1(d29f4a56a06ac809b4b9efa8aa9d1f246250e3a2) )
ROM_END

ROM_START( frontlin )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "fl69.u69",     0x00000, 0x1000, CRC(93b64599) SHA1(1d4e135d5105d5e2949dbd791eb87c30e8505f1d) )
	ROM_LOAD( "fl68.u68",     0x01000, 0x1000, CRC(82dccdfb) SHA1(0c88feff88b61dc0ae5487aa0a83f665a308658f) )
	ROM_LOAD( "fl67.u67",     0x02000, 0x1000, CRC(3fa1ba12) SHA1(06eaccc75a4a950ed509c0dd203eeb7120849e74) )
	ROM_LOAD( "fl66.u66",     0x03000, 0x1000, CRC(4a3db285) SHA1(0b486523d4ae302962dcb4ca042754fd96208259) )
	ROM_LOAD( "fl65.u65",     0x04000, 0x1000, CRC(da00ec70) SHA1(652eb07c1e98ed04042a334ad8e27fd3da8dd6a2) )
	ROM_LOAD( "fl64.u64",     0x05000, 0x1000, CRC(9fc90a20) SHA1(2d1bc248ed68dbb1993c360a9f2e2dbe26c216fb) )
	ROM_LOAD( "fl55.u55",     0x06000, 0x1000, CRC(359242c2) SHA1(63bd845b2d881946a7904e4df1db3d78a60b57ad) )
	ROM_LOAD( "fl54.u54",     0x07000, 0x1000, CRC(d234c60f) SHA1(b45bf432a64b7aaf3762d72a762b5eca198d5b3d) )
	ROM_LOAD( "aa1_10.8",     0x0e000, 0x1000, CRC(2704aa4c) SHA1(d8dbad5deeef2c7b032b741ab3014a8402c334eb) )
	ROM_LOAD( "fl53.u53",     0x10000, 0x1000, CRC(67429975) SHA1(b84254b2d04b034c2602f95587523a77dfdbae71) ) /* banked at 6000 */
	ROM_LOAD( "fl52.u52",     0x11000, 0x1000, CRC(cb223d34) SHA1(a1a4530ed25064c6cabe34c52bb239e3656e4ced) ) /* banked at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "fl70.u70",     0x0000, 0x1000, CRC(15f4ed8c) SHA1(ec096234e4e594100180eb99c8c57eb97b9f57e2) )
	ROM_LOAD( "fl71.u71",     0x1000, 0x1000, CRC(c3eb38e7) SHA1(427e5deb6a6e22d8c34923209a818f79d50e59d4) )

	ROM_REGION( 0x0800, "mcu", 0 )       /* 2k for the microcontroller */
	ROM_LOAD( "aa1.13",       0x0000, 0x0800, CRC(7e78bdd3) SHA1(9eeb0e969fd013b9db074a15b0463216453e9364) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "fl1.u1",       0x0000, 0x1000, CRC(e82c9f46) SHA1(eaab468bb5e46e9c714e6f84e65f954331fdbc56) )
	ROM_LOAD( "fl2.u2",       0x1000, 0x1000, CRC(123055d3) SHA1(6aaddd8ebb418c7c8584eb74ad13cd5accd5a196) )
	ROM_LOAD( "fl3.u3",       0x2000, 0x1000, CRC(7ea46347) SHA1(b924a614abe01f7ca6a31463864d6cc55a47946e) )
	ROM_LOAD( "fl4.u4",       0x3000, 0x1000, CRC(9e2cff10) SHA1(0932c15eacccab5a3a931dd40c1a35b5a4ca1cd5) )
	ROM_LOAD( "fl5.u5",       0x4000, 0x1000, CRC(630b4be1) SHA1(780f75fdea68917a08f5f00da3831eaa26fd4405) )
	ROM_LOAD( "fl6.u6",       0x5000, 0x1000, CRC(9e092d58) SHA1(8388870bb40c7a2e3b4ede74c37c71c3a3d1a607) )
	ROM_LOAD( "fl7.u7",       0x6000, 0x1000, CRC(613682a3) SHA1(b681f3a4e70f207ce140adfac1388900d5013317) )
	ROM_LOAD( "fl8.u8",       0x7000, 0x1000, CRC(f73b0d5e) SHA1(3f4ae070e39fac3c64c6c438168d131bffc580e2) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END


/*
Elevator Action (4 Board Version, typical Taito SJ hardware)
Taito, 1983

Manual shows two versions, 4-board without protection, and
5-board, using an additional 68705 MCU.

PCB Layout
----------

Bottom Board (Main CPU)

WWO70003A
KNN00013 (sticker)
|---------------------------------------------|
|        IC64   IC52  Z80   2016              |
|        IC65   *                            |-|
|        IC66   IC54                         | |
|        IC67   IC55                       P | |
|        IC68   2016                         | |
|        IC69   2016                         | |
|1                                           |-|
|8                                            |
|W T                                          |
|A                                            |
|Y                                           |-|
|                                            | |
|         2114                             Q | |
|         2114                               | |
|                                            | |
| 8MHz                                       |-|
| 12MHz                                       |
|---------------------------------------------|
Notes:
      T    - 18 way connector for power
      IC*  - 2732 EPROMs
      *    - Empty DIP24 socket
      2016 - 2kx8 SRAM
      2114 - 1kx4 SRAM
      Z80  - clock 4MHz [8/2]
      P, Q - Flat cable connectors joining to middle PCB


Middle Board (Video)

WWO70002A
EBN00002 (sticker)
 |---------------------------------------------|
 |           AM93422   AM93422                 |
|-|                                           |-|
| |                                           | |
| | P                                       R | |
| |                              2114         | |
| |                              2114         | |
|-|                                           |-|
 |                                             |
 |                                             |
 |                                             |
|-|                                           |-|
| |   2016                                    | |
| | Q 2016                       IC22       S | |
| |   2016                                    | |
| |   2016                                    | |
|-|   2016                                    |-|
 |    2016                                     |
 |---------------------------------------------|
Notes:
      Lots of logic chips, not much else....
      2016    - 2kx8 SRAM
      AM93422 - AMD AM93422 256x4 SRAM
      2114    - 1kx4 SRAM
      IC22    - MMI 6301 256x4 Bi-polar PROM
      R,S,P,Q - Flat cable connectors joining to top and bottom PCB


Top Board (Sound)

WW070001A
WWN00001A
KNN00012 (sticker)
KNK00504 (sticker)
|---------------------------------------------|
|VOL-BG VOL MB3730  Z80      2114      *      |
|                  AY3-8910  2114      *     |-|
|H                 AY3-8910                  | |
|                  AY3-8910            IC71 S| |
|                                            | |
|                                      IC70  | |
|2                                           |-|
|2                                    NE555   |
|W G                                          |
|A               PAL                          |
|Y                                           |-|
|                                            | |
|                                 82S09      | |
|                                           R| |
|                                            | |
|                                AY3-8910    |-|
|         6MHz        SWC   SWB   SWA         |
|---------------------------------------------|
Notes:
      Board contains MANY resistors and capacitors in the sound section.
      Z80 - clock 3MHz [6/2]
      AY-3-8910 - clock 1.5MHz [6/4]
      IC* - 2732 EPROMs
      * - Empty DIP24 sockets
      PAL - marked 'WW-15'. Chip is 'painted' black, under the paint it reads MMI PAL16L8 ;-)
      82S09 - Signetics 82S09 576-bit (64x9) Bi-Polar RAM
      G - 22-way edge connector
      H - 12-pin power connector
      R, S - Flat cable connectors joining to middle PCB
      VSync - xx Hz
      HSync - xx kHz


ROM Daughterboard (on top of Sound PCB)

WWO70004
KNN00014
|---------------------------|
|              IC6    IC3   |
|                           |
|E       IC8   IC5    IC2   |
|                           |
|        IC7   IC4    IC1   |
|---------------------------|
Notes:
      E - 30-pin connector joining to sound PCB.
      IC* - 2732 EPROMs
*/

ROM_START( elevator )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "ea-ic69.bin",  0x0000, 0x1000, CRC(24e277ef) SHA1(764e3b3a34bf0ec849d58023f710e5b0a0d0ccb5) )
	ROM_LOAD( "ea-ic68.bin",  0x1000, 0x1000, CRC(13702e39) SHA1(b72fea84f8322463ff224e3b06698a1ed7e305b7) )
	ROM_LOAD( "ea-ic67.bin",  0x2000, 0x1000, CRC(46f52646) SHA1(11b68b89ab0f580bfe88047e59bd9bba237a2eb4) )
	ROM_LOAD( "ea-ic66.bin",  0x3000, 0x1000, CRC(e22fe57e) SHA1(50888975e698c4d2a124e5731d0922df43eb01ef) )
	ROM_LOAD( "ea-ic65.bin",  0x4000, 0x1000, CRC(c10691d7) SHA1(a7657d3d661421d1fca3b04e4025725272b77203) )
	ROM_LOAD( "ea-ic64.bin",  0x5000, 0x1000, CRC(8913b293) SHA1(163daa07b6d45469f18e4f4a1904b60a890c8699) )
	ROM_LOAD( "ea-ic55.bin",  0x6000, 0x1000, CRC(1cabda08) SHA1(8fff75a354ee7589bd0ffe8b0271fd9111b2b241) )
	ROM_LOAD( "ea-ic54.bin",  0x7000, 0x1000, CRC(f4647b4f) SHA1(711a9447d30b35bc38e149e0cf6e835ff06efd54) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ea-ic70.bin",  0x0000, 0x1000, CRC(6d5f57cb) SHA1(abb916d675ee85032697d656121d4f525202cab3) )
	ROM_LOAD( "ea-ic71.bin",  0x1000, 0x1000, CRC(f0a769a1) SHA1(9970fba3afeaaaa7fd217f0704fb9df9cf13cf65) )

	ROM_REGION( 0x0800, "mcu", 0 )       /* 2k for the microcontroller */
	ROM_LOAD( "ba3.11",       0x0000, 0x0800, CRC(9ce75afc) SHA1(4c8f5d926ae2bec8fcb70692125b9e1c863166c6) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "ea-ic1.bin",   0x0000, 0x1000, CRC(bbbb3fba) SHA1(a8e3a0886ea5dc8e70aa280b4cef5fb26ca0e125) )
	ROM_LOAD( "ea-ic2.bin",   0x1000, 0x1000, CRC(639cc2fd) SHA1(0ba292ac34dbf779a929db6358cd842d38077b3d) )
	ROM_LOAD( "ea-ic3.bin",   0x2000, 0x1000, CRC(61317eea) SHA1(f1a18c09e31edb4ec3ad7ab853f425383ca22314) )
	ROM_LOAD( "ea-ic4.bin",   0x3000, 0x1000, CRC(55446482) SHA1(0767701213920d30d5a3a226b25cfbbd3f24437a) )
	ROM_LOAD( "ea-ic5.bin",   0x4000, 0x1000, CRC(77895c0f) SHA1(fe116c53a7e8ac523a17249a56df9f40b503b30d) )
	ROM_LOAD( "ea-ic6.bin",   0x5000, 0x1000, CRC(9a1b6901) SHA1(646491c1d28904d9e662b1bff554bb74ec47708d) )
	ROM_LOAD( "ea-ic7.bin",   0x6000, 0x1000, CRC(839112ec) SHA1(30bca7f5214bf424aa10184094947496f054ddf4) )
	ROM_LOAD( "ea-ic8.bin",   0x7000, 0x1000, CRC(db7ff692) SHA1(4d0d9ab0c9d8d758e121f2bcfc6422ffadf2d760) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( elevatorb )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "ea69.bin",     0x0000, 0x1000, CRC(66baa214) SHA1(fad660d2983daad478085be3b1a951b0864485dd) )
	ROM_LOAD( "ea-ic68.bin",  0x1000, 0x1000, CRC(13702e39) SHA1(b72fea84f8322463ff224e3b06698a1ed7e305b7) )
	ROM_LOAD( "ea-ic67.bin",  0x2000, 0x1000, CRC(46f52646) SHA1(11b68b89ab0f580bfe88047e59bd9bba237a2eb4) )
	ROM_LOAD( "ea66.bin",     0x3000, 0x1000, CRC(b88f3383) SHA1(99f23d82d7866e4bca8f5a508e0913673d12489b) )
	ROM_LOAD( "ea-ic65.bin",  0x4000, 0x1000, CRC(c10691d7) SHA1(a7657d3d661421d1fca3b04e4025725272b77203) )
	ROM_LOAD( "ea-ic64.bin",  0x5000, 0x1000, CRC(8913b293) SHA1(163daa07b6d45469f18e4f4a1904b60a890c8699) )
	ROM_LOAD( "ea55.bin",     0x6000, 0x1000, CRC(d546923e) SHA1(1de128cf96e6d976f6f09e4ac2b4d2507935bfe9) )
	ROM_LOAD( "ea54.bin",     0x7000, 0x1000, CRC(963ec5a5) SHA1(b2f684b61feb31e3d1856c16edcef33262f68581) )
	/* 10000-10fff space for another banked ROM (not used) */
	ROM_LOAD( "ea52.bin",     0x11000, 0x1000, CRC(44b1314a) SHA1(e8980536f787fcb603943d4b2b3a64f475e51a16) ) /* protection crack, bank switched at 7000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ea-ic70.bin",  0x0000, 0x1000, CRC(6d5f57cb) SHA1(abb916d675ee85032697d656121d4f525202cab3) )
	ROM_LOAD( "ea-ic71.bin",  0x1000, 0x1000, CRC(f0a769a1) SHA1(9970fba3afeaaaa7fd217f0704fb9df9cf13cf65) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "ea-ic1.bin",   0x0000, 0x1000, CRC(bbbb3fba) SHA1(a8e3a0886ea5dc8e70aa280b4cef5fb26ca0e125) )
	ROM_LOAD( "ea-ic2.bin",   0x1000, 0x1000, CRC(639cc2fd) SHA1(0ba292ac34dbf779a929db6358cd842d38077b3d) )
	ROM_LOAD( "ea-ic3.bin",   0x2000, 0x1000, CRC(61317eea) SHA1(f1a18c09e31edb4ec3ad7ab853f425383ca22314) )
	ROM_LOAD( "ea-ic4.bin",   0x3000, 0x1000, CRC(55446482) SHA1(0767701213920d30d5a3a226b25cfbbd3f24437a) )
	ROM_LOAD( "ea-ic5.bin",   0x4000, 0x1000, CRC(77895c0f) SHA1(fe116c53a7e8ac523a17249a56df9f40b503b30d) )
	ROM_LOAD( "ea-ic6.bin",   0x5000, 0x1000, CRC(9a1b6901) SHA1(646491c1d28904d9e662b1bff554bb74ec47708d) )
	ROM_LOAD( "ea-ic7.bin",   0x6000, 0x1000, CRC(839112ec) SHA1(30bca7f5214bf424aa10184094947496f054ddf4) )
	ROM_LOAD( "ea08.bin",     0x7000, 0x1000, CRC(67ebf7c1) SHA1(ec4db1392967f3959574bc4ca03e95938a6e5173) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END



ROM_START( tinstar )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "ts.69",        0x0000, 0x1000, CRC(a930af60) SHA1(1644fcf3460a1dfceaa39ccc54c9506289965f4c) )
	ROM_LOAD( "ts.68",        0x1000, 0x1000, CRC(7f2714ca) SHA1(55e6c83336e4db1142cc6f867e84359fadc39d5a) )
	ROM_LOAD( "ts.67",        0x2000, 0x1000, CRC(49170786) SHA1(9749c60a225b9698bb19b3ab99ea91dd4571197d) )
	ROM_LOAD( "ts.66",        0x3000, 0x1000, CRC(3766f130) SHA1(939a2ad34c8fb3f7f99790d3e492f13be11dadf6) )
	ROM_LOAD( "ts.65",        0x4000, 0x1000, CRC(41251246) SHA1(da8323afb4967eb530f52008e49bb974b30e7e66) )
	ROM_LOAD( "ts.64",        0x5000, 0x1000, CRC(812285d5) SHA1(e57adc29567379603570316d37b8abde05c1e690) )
	ROM_LOAD( "ts.55",        0x6000, 0x1000, CRC(6b80ac51) SHA1(5b3b848273763af5629bad8b5f3a8518d37a6316) )
	ROM_LOAD( "ts.54",        0x7000, 0x1000, CRC(b352360f) SHA1(85dedb98a9d604bd816f626fb39fc49a7f1e73d2) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ts.70",        0x0000, 0x1000, CRC(4771838d) SHA1(f84f1367f8a86e6c070da29419c8de5c302d1469) )
	ROM_LOAD( "ts.71",        0x1000, 0x1000, CRC(03c91332) SHA1(3903e876ae02e9aea7ee6854bb4c6407dd7108d6) )
	ROM_LOAD( "ts.72",        0x2000, 0x1000, CRC(beeed8f3) SHA1(2a18edecabbfd10b3238338cb5554edc8c18d93c) )

	ROM_REGION( 0x0800, "mcu", 0 )       /* 2k for the microcontroller */
	ROM_LOAD( "a10-12",       0x0000, 0x0800, CRC(889eefc9) SHA1(1a31aa21c02215410eea27ed52fad67f007ee810) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "ts.1",         0x0000, 0x1000, CRC(f1160718) SHA1(d35ebb96867299137ba1ce19da998002a6b9898d) )
	ROM_LOAD( "ts.2",         0x1000, 0x1000, CRC(39dc6dbb) SHA1(223c3430ea9acaeed921f31e6c1e5153dc48874c) )
	ROM_LOAD( "ts.3",         0x2000, 0x1000, CRC(079df429) SHA1(7f1fb64dfecf7b20a7e8357393084eb9c709a898) )
	ROM_LOAD( "ts.4",         0x3000, 0x1000, CRC(e61105d4) SHA1(44dbe3be78b4ba0c1e17c46498b71158ebcfd6d2) )
	ROM_LOAD( "ts.5",         0x4000, 0x1000, CRC(ffab5d15) SHA1(63b71775646f8dc838f0fdb8abb0fe63d4f2f6ae) )
	ROM_LOAD( "ts.6",         0x5000, 0x1000, CRC(f1d8ca36) SHA1(09c5e4885edb2906a929735347f04d06cc6dfeda) )
	ROM_LOAD( "ts.7",         0x6000, 0x1000, CRC(894f6332) SHA1(9b4a0e35ec08f2d6c5340435c69a05ad0333627c) )
	ROM_LOAD( "ts.8",         0x7000, 0x1000, CRC(519aed19) SHA1(8618a794bdd091ed7c2300a2dabf99455c9e2ebd) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

/*
    The Tin Star (alt)

    Going by the labels this set seems more original than the above set, but the MCU dump came from the
    above set

    main cpu: z80
    sound cpu: z80
    sound ics: ay-3-8910 x4, DAC
    osc: 12mhz, 8mhz

    The 68705 is present but i cannot dump since the Willem doesn't support it.

*/

ROM_START( tinstar2 )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "a10-01.bin",        0x0000, 0x2000, CRC(19faf0b3) SHA1(2dfb3fa7890687cae840769849c96e04a706dd63) )
	ROM_LOAD( "a10-02.bin",        0x2000, 0x2000, CRC(99bb26ff) SHA1(80adb2d11b7fbb6fcd8b1dc9270a7fdc471cc0aa) )
	ROM_LOAD( "a10-03.bin",        0x4000, 0x2000, CRC(3169e175) SHA1(7cc92f87d511f702aebe6e82d0f435d1ff3aa828) )
	ROM_LOAD( "a10-04.bin",        0x6000, 0x2000, CRC(6641233c) SHA1(5544c31e1a44bbd056d16571056821d865cb6e29) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a10-29.bin",        0x0000, 0x2000, CRC(771f1a6a) SHA1(c5d1841840ff35e2c20a285b1b7f35150356f50f) )
	ROM_LOAD( "a10-10.bin",        0x2000, 0x1000, CRC(beeed8f3) SHA1(2a18edecabbfd10b3238338cb5554edc8c18d93c) )

	ROM_REGION( 0x0800, "mcu", 0 )       /* 2k for the microcontroller */
	ROM_LOAD( "a10-12",       0x0000, 0x0800, CRC(889eefc9) SHA1(1a31aa21c02215410eea27ed52fad67f007ee810) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "a10-05.bin",         0x0000, 0x2000, CRC(6bb1bba9) SHA1(00924ba8da95c7ca7598d462673cdb98772f4fff) )
	ROM_LOAD( "a10-06.bin",         0x2000, 0x2000, CRC(0abff1a1) SHA1(6876753c53b250968777c54b8c57d97fa45086f5) )
	ROM_LOAD( "a10-07.bin",         0x4000, 0x2000, CRC(d1bec7a8) SHA1(40873ce1b07cc21e4ed4fff78a29dfcf9d735ca8) )
	ROM_LOAD( "a10-08.bin",         0x6000, 0x2000, CRC(15c6eb41) SHA1(e208f3c3dcaa3e6e8e3dfcaddae2dbf1c57c06f1) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END


ROM_START( waterski )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "a03-01",       0x0000, 0x1000, CRC(322c4c2c) SHA1(8f25acd50fdda4cae756849f3961c79d6138866e) )
	ROM_LOAD( "a03-02",       0x1000, 0x1000, CRC(8df176d1) SHA1(db6e7a82320dc478306b0b1a06c284ed4faf5103) )
	ROM_LOAD( "a03-03",       0x2000, 0x1000, CRC(420bd04f) SHA1(1efce6d384cde94c0fd9e7c3a43feaa18b6c3d73) )
	ROM_LOAD( "a03-04",       0x3000, 0x1000, CRC(5c081a94) SHA1(eae8b84869b1bc754550c8427c510831348418fa) )
	ROM_LOAD( "a03-05",       0x4000, 0x1000, CRC(1fae90d2) SHA1(f5f2e022794593a5a6a06223e7a1ac19ef23e140) )
	ROM_LOAD( "a03-06",       0x5000, 0x1000, CRC(55b7c151) SHA1(ad05dcd07a7907d08c5d8827e323c207a7c8ace6) )
	ROM_LOAD( "a03-07",       0x6000, 0x1000, CRC(8abc7522) SHA1(4a3d8ea006630020722d385f4d7b50ece323cad2) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a03-13",       0x0000, 0x1000, CRC(78c7d37f) SHA1(460bff7c78a2fa78a7c6f95c7f2847c1c5267626) )
	ROM_LOAD( "a03-14",       0x1000, 0x1000, CRC(31f991ca) SHA1(82cbaa618ac3de6fce12e9dcbb89ab064773b2bd) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "a03-08",       0x0000, 0x1000, BAD_DUMP CRC(c206d870) SHA1(0be09b7da28d60bf23a0b87cff28957bb165bec5) ) // small glitches in font gfx
	ROM_LOAD( "a03-09",       0x1000, 0x1000, CRC(48ac912a) SHA1(09d57b5b76a4416f1ee5eb2077bc969d2afbbf11) )
	ROM_LOAD( "a03-10",       0x2000, 0x1000, CRC(a056defb) SHA1(15b4202b19f7190bfc953d0958c189db2db928cc) )
	ROM_LOAD( "a03-11",       0x3000, 0x1000, CRC(f06cddd6) SHA1(483a25880a29552f4b70cd5d41de5cfe4ed64475) )
	ROM_LOAD( "a03-12",       0x4000, 0x1000, CRC(27dfd8c2) SHA1(cf4d2c4b52598cff7a934c816514de1471e4af8d) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( bioatack )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "aa8-01.69",    0x0000, 0x1000, CRC(e5abc211) SHA1(bdec8160a99a1869bd004e8177ac075bd52ee3c9) )
	ROM_LOAD( "aa8-02.68",    0x1000, 0x1000, CRC(b5bfde00) SHA1(403fa7133d7571b53c19f9175ee844349a43ae4e) )
	ROM_LOAD( "aa8-03.67",    0x2000, 0x1000, CRC(e4e46e69) SHA1(6b0de7dbfcc2a56b6c3fce4b94687174002a7217) )
	ROM_LOAD( "aa8-04.66",    0x3000, 0x1000, CRC(86e0af8c) SHA1(672de6978a187c2014401090c1d7fd79f7dd7c28) )
	ROM_LOAD( "aa8-05.65",    0x4000, 0x1000, CRC(c6248608) SHA1(5536c37e9b6ac290fc8a86ab194d15f63a1d40b5) )
	ROM_LOAD( "aa8-06.64",    0x5000, 0x1000, CRC(685a0383) SHA1(abc00554e27cfb9db5c60e7c9127e33d1f4d5a5f) )
	ROM_LOAD( "aa8-07.55",    0x6000, 0x1000, CRC(9d58e2b7) SHA1(ed974fdd4e1d69f08eb8214cb91493086f7ae6ed) )
	ROM_LOAD( "aa8-08.54",    0x7000, 0x1000, CRC(dec5271f) SHA1(27fc30e1c82ade6a7adda1891b78ca3e44643fd1) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "aa8-17.70",    0x0000, 0x1000, CRC(36eb95b5) SHA1(ae31aff6cb34ffa8ce8a3574697b89f6d9dd273c) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "aa8-09.1",     0x0000, 0x1000, CRC(1fee5fd6) SHA1(04de2d285f98275ca5809261d264504f9638d2b0) )
	ROM_LOAD( "aa8-10.2",     0x1000, 0x1000, CRC(e0133423) SHA1(50f3aee729dfda34b1025541f1ee59394e4c386d) )
	ROM_LOAD( "aa8-11.3",     0x2000, 0x1000, CRC(0f5715c6) SHA1(a6893a86c3f6e9e17eb31381eea36bd17edf1e62) )
	ROM_LOAD( "aa8-12.4",     0x3000, 0x1000, CRC(71126dd0) SHA1(9d343bc2294a74df6fc2234e250bf4cdd4970d7c) )
	ROM_LOAD( "aa8-13.5",     0x4000, 0x1000, CRC(adcdd2f0) SHA1(b7806e5a8f639795b43553689675861f043ec702) )
	ROM_LOAD( "aa8-14.6",     0x5000, 0x1000, CRC(2fe18680) SHA1(b47095fe48572b9354dd80a9dda9dc45842f0369) )
	ROM_LOAD( "aa8-15.7",     0x6000, 0x1000, CRC(ff5aad4b) SHA1(9ce15d3e5bfbab84913a6fa0830d438c63a86de7) )
	ROM_LOAD( "aa8-16.8",     0x7000, 0x1000, CRC(ceba4036) SHA1(f059ed8482a06c5495c6b3a6e55dbda6b77a894c) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( sfposeid )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "a14-01.1",     0x00000, 0x2000, CRC(aa779fbb) SHA1(d38f16c6f3d3769f82f37ee3bcf0e7b17756dc53) )
	ROM_LOAD( "a14-02.2",     0x02000, 0x2000, CRC(ecec9dc3) SHA1(f9548a7b2bd18e8bd4bc58dd520b0926b5abafac) )
	ROM_LOAD( "a14-03.3",     0x04000, 0x2000, CRC(469498c1) SHA1(3c0a052ba3beac583fc41e55d90f13f3fa3160f1) )
	ROM_LOAD( "a14-04.6",     0x06000, 0x2000, CRC(1db4bc02) SHA1(47f469ae40fcd476edcbcb42fd1f9aa39b6aea01) )
	ROM_LOAD( "a14-05.7",     0x10000, 0x2000, CRC(95e2f903) SHA1(d872bbc2c75f7a87f721465b18881f844651dfd8) ) /* banked at 6000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a14-10.70",    0x0000, 0x1000, CRC(f1365f35) SHA1(34b3ea03eb9fbf5454858fa6e07ec49a7b3be8b4) )
	ROM_LOAD( "a14-11.71",    0x1000, 0x1000, CRC(74a12fe2) SHA1(8678ea68bd283b7a63915717cdbbedef0b699198) )

	ROM_REGION( 0x0800, "mcu", 0 )       /* 2k for the microcontroller */
	ROM_LOAD( "a14-12",       0x0000, 0x0800, CRC(091beed8) SHA1(263806aef01bbc258f5cfa92de8a9e355491fb3a) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "a14-06.4",     0x0000, 0x2000, CRC(9740493b) SHA1(7ce56a8a1f2923fe932dbe98fb8457f5f7ba3bb7) )
	ROM_LOAD( "a14-07.5",     0x2000, 0x2000, CRC(1c93de97) SHA1(a345150b1afd1352fe9103b6b5d4fbf24c7f1948) )
	ROM_LOAD( "a14-08.9",     0x4000, 0x2000, CRC(4367e65a) SHA1(d0de218377a876cd584bd80b6adfbe32e1cead99) )
	ROM_LOAD( "a14-09.10",    0x6000, 0x2000, CRC(677cffd5) SHA1(004f3df1b0f5eed0995d7ac507f4672f9e279853) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( hwrace )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "hw_race.01",   0x0000, 0x1000, CRC(8beec11f) SHA1(716b7ca0a49151e74a3c0289d0a7f80db8f76452) )
	ROM_LOAD( "hw_race.02",   0x1000, 0x1000, CRC(72ad099d) SHA1(2b3cc312203a062c01c90127369413297bd25d67) )
	ROM_LOAD( "hw_race.03",   0x2000, 0x1000, CRC(d0c221d7) SHA1(f940099b32524a01cb1ad035e0064d6038c039d2) )
	ROM_LOAD( "hw_race.04",   0x3000, 0x1000, CRC(eb97015b) SHA1(ada2832154077965ed1ef7aebc353fe232852d27) )
	ROM_LOAD( "hw_race.05",   0x4000, 0x1000, CRC(777c8007) SHA1(5241cd43716b6f1b6b29cee3c16cd517ee8a8b8f) )
	ROM_LOAD( "hw_race.06",   0x5000, 0x1000, CRC(165f46a3) SHA1(ba7a95a843ae2e749140fae5f0ee8a4c8e3e0721) )
	ROM_LOAD( "hw_race.07",   0x6000, 0x1000, CRC(53d7e323) SHA1(e59aba28947de6573290c1ff3923963596a5ffa8) )
	ROM_LOAD( "hw_race.08",   0x7000, 0x1000, CRC(bdbc1208) SHA1(9d2da1c987095fdb9443defea109f918d813bec6) )
	/* 10000-11fff space for banked ROMs (not used) */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "hw_race.17",   0x0000, 0x1000, CRC(afe24f3e) SHA1(11a794bad711057728463c4db5daadac84016bf0) )
	ROM_LOAD( "hw_race.18",   0x1000, 0x1000, CRC(dbec897d) SHA1(d69a965207eac61f0ad9ea9fa42561449c5bfd11) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "hw_race.09",   0x0000, 0x1000, CRC(345b9b88) SHA1(32be98bc127059449a52ebca153cc3614933b126) )
	ROM_LOAD( "hw_race.10",   0x1000, 0x1000, CRC(598a3c3e) SHA1(48088ab7e7daeb2cf9584ed7acf5b138c0c4a027) )
	ROM_LOAD( "hw_race.11",   0x2000, 0x1000, CRC(3f436a7d) SHA1(107dc98eea1e81da667aeca89bb5ebb904727644) )
	ROM_LOAD( "hw_race.12",   0x3000, 0x1000, CRC(8694b2c6) SHA1(e91592019e05065dae2c634b48f58cca2b90acec) )
	ROM_LOAD( "hw_race.13",   0x4000, 0x1000, CRC(a0af7711) SHA1(f148ff70f9bd13f089134079dcca985ca97c63bd) )
	ROM_LOAD( "hw_race.14",   0x5000, 0x1000, CRC(9be0f556) SHA1(5e277003a454bf9f1cfc544195c5da74202cf844) )
	ROM_LOAD( "hw_race.15",   0x6000, 0x1000, CRC(e1057eb7) SHA1(65cf73d5ba6e2a65ed32414840947dcb87ddba5e) )
	ROM_LOAD( "hw_race.16",   0x7000, 0x1000, CRC(f7104668) SHA1(240ee2fad6ac82b5d3bf7af6d3621bc86236e95e) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )
ROM_END

ROM_START( kikstart )
	ROM_REGION( 0x12000, "maincpu", 0 )
	ROM_LOAD( "a20-01",       0x00000, 0x2000, CRC(5810be97) SHA1(da3507b8274a1f5c0d5b10ad7259a0f02bfb6eda) )
	ROM_LOAD( "a20-02",       0x02000, 0x2000, CRC(13e9565d) SHA1(bb73b965262bc1bd90266b460d95fe217938a33c) )
	ROM_LOAD( "a20-03",       0x04000, 0x2000, CRC(93d7a9e1) SHA1(ab9289172a8f52e1a191efd91b52e8cf762d1f7f) )
	ROM_LOAD( "a20-04",       0x06000, 0x2000, CRC(1f23c5d6) SHA1(104312f09916e9938f71c3dffb94aaa335afb084) )
	ROM_LOAD( "a20-05",       0x10000, 0x2000, CRC(66e100aa) SHA1(a3b78b1b0250491c5e9bd39e761612a1f70640dd) ) /* banked at 6000 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a20-10",       0x0000, 0x1000, CRC(de4352a4) SHA1(1548f8d7ac9e79ccaf1a503ded4b868bd350fd7e) )
	ROM_LOAD( "a20-11",       0x1000, 0x1000, CRC(8db12dd9) SHA1(3b291d478b3f3f1bf93d95a78506d99a71f36d05) )
	ROM_LOAD( "a20-12",       0x2000, 0x1000, CRC(e7eeb933) SHA1(26f3904f6d4dc814318221f1c9cd5dcc671fe05a) )

	ROM_REGION( 0x0800, "mcu", 0 )       /* 2k for the microcontroller */
	ROM_LOAD( "a20-13.ic91",  0x0000, 0x0800, CRC(3fb6c4fb) SHA1(04b9458f21a793444cd587055e2e3ccfa3f835a2) )

	ROM_REGION( 0x8000, "gfx1", 0 )       /* graphic ROMs used at runtime */
	ROM_LOAD( "a20-06",       0x0000, 0x2000, CRC(6582fc89) SHA1(a902f4f2d3c9e352ac2f49f33b9f30be931f2be1) )
	ROM_LOAD( "a20-07",       0x2000, 0x2000, CRC(8c0b76d2) SHA1(243ef5cdd5bbf504dd040a01b8e126dbbc65f9a9) )
	ROM_LOAD( "a20-08",       0x4000, 0x2000, CRC(0cca7a9d) SHA1(bf83ad5791a2277c51b1a2644a078be0ca8a4047) )
	ROM_LOAD( "a20-09",       0x6000, 0x2000, CRC(da625ccf) SHA1(092c72b5a23d7a5683889f7597096938ba879602) )

	ROM_REGION( 0x0100, "proms", 0 )      /* layer PROM */
	ROM_LOAD( "eb16.22",      0x0000, 0x0100, CRC(b833b5ea) SHA1(d233f1bf8a3e6cd876853ffd721b9b64c61c9047) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16l8.28",   0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

void taitosj_state::reset_common()
{
	m_sndnmi_disable = 1;
	m_input_port_4_f0 = 0;
	/* start in 1st gear */
	m_kikstart_gears[0] = 0x02;
	m_kikstart_gears[1] = 0x02;
	m_dac_out = 0;
	m_dac_vol = 0;
}

void taitosj_state::init_common()
{
	save_item(NAME(m_sndnmi_disable));
	save_item(NAME(m_input_port_4_f0));
	save_item(NAME(m_kikstart_gears));
	save_item(NAME(m_dac_out));
	save_item(NAME(m_dac_vol));

	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(taitosj_state::reset_common), this));
}

DRIVER_INIT_MEMBER(taitosj_state,taitosj)
{
	init_common();
}

DRIVER_INIT_MEMBER(taitosj_state,spacecr)
{
	init_common();

	/* install protection handler */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd48b, 0xd48b, read8_delegate(FUNC(taitosj_state::spacecr_prot_r),this));
}

DRIVER_INIT_MEMBER(taitosj_state,alpine)
{
	init_common();

	/* install protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd40b, 0xd40b, read8_delegate(FUNC(taitosj_state::alpine_port_2_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd50f, 0xd50f, write8_delegate(FUNC(taitosj_state::alpine_protection_w),this));
}

DRIVER_INIT_MEMBER(taitosj_state,alpinea)
{
	init_common();

	/* install protection handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd40b, 0xd40b, read8_delegate(FUNC(taitosj_state::alpine_port_2_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd50e, 0xd50e, write8_delegate(FUNC(taitosj_state::alpinea_bankswitch_w),this));
}

DRIVER_INIT_MEMBER(taitosj_state,junglhbr)
{
	init_common();

	/* inverter on bits 0 and 1 */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x9000, 0xbfff, write8_delegate(FUNC(taitosj_state::junglhbr_characterram_w),this));
}

GAME( 1981, spaceskr, 0,        nomcu,    spaceskr, taitosj_state,   taitosj, ROT0,   "Taito Corporation", "Space Seeker", MACHINE_SUPPORTS_SAVE )
GAME( 1981, spacecr,  0,        nomcu,    spacecr, taitosj_state,    spacecr, ROT90,  "Taito Corporation", "Space Cruiser", MACHINE_SUPPORTS_SAVE )
GAME( 1982, junglek,  0,        nomcu,    junglek, taitosj_state,    taitosj, ROT180, "Taito Corporation", "Jungle King (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, junglekas,junglek,  nomcu,    junglek, taitosj_state,    taitosj, ROT180, "Taito Corporation", "Jungle King (alternate sound)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, junglekj2,junglek,  nomcu,    junglek, taitosj_state,    taitosj, ROT180, "Taito Corporation", "Jungle King (Japan, earlier)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, jungleh,  junglek,  nomcu,    junglek, taitosj_state,    taitosj, ROT180, "Taito America Corporation", "Jungle Hunt (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, junglehbr,junglek,  nomcu,    junglek, taitosj_state,    junglhbr,ROT180, "Taito do Brasil",   "Jungle Hunt (Brazil)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, piratpet, junglek,  nomcu,    piratpet, taitosj_state,   taitosj, ROT180, "Taito America Corporation", "Pirate Pete", MACHINE_SUPPORTS_SAVE )
GAME( 1982, jungleby, junglek,  nomcu,    junglek, taitosj_state,    taitosj, ROT180, "bootleg", "Jungle Boy (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, alpine,   0,        nomcu,    alpine, taitosj_state,     alpine,  ROT270, "Taito Corporation", "Alpine Ski (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, alpinea,  alpine,   nomcu,    alpinea, taitosj_state,    alpinea, ROT270, "Taito Corporation", "Alpine Ski (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, timetunl, 0,        nomcu,    timetunl, taitosj_state,   taitosj, ROT0,   "Taito Corporation", "Time Tunnel", MACHINE_SUPPORTS_SAVE )
GAME( 1982, wwestern, 0,        nomcu,    wwestern, taitosj_state,   taitosj, ROT270, "Taito Corporation", "Wild Western (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, wwestern1,wwestern, nomcu,    wwestern, taitosj_state,   taitosj, ROT270, "Taito Corporation", "Wild Western (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, frontlin, 0,        mcu,      frontlin, taitosj_state,   taitosj, ROT270, "Taito Corporation", "Front Line", MACHINE_SUPPORTS_SAVE )
GAME( 1983, elevator, 0,        mcu,      elevator, taitosj_state,   taitosj, ROT0,   "Taito Corporation", "Elevator Action", MACHINE_SUPPORTS_SAVE )
GAME( 1983, elevatorb,elevator, nomcu,    elevator, taitosj_state,   taitosj, ROT0,   "bootleg", "Elevator Action (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, tinstar,  0,        mcu,      tinstar, taitosj_state,    taitosj, ROT0,   "Taito Corporation", "The Tin Star (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, tinstar2, tinstar,  mcu,      tinstar, taitosj_state,    taitosj, ROT0,   "Taito Corporation", "The Tin Star (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, waterski, 0,        nomcu,    waterski, taitosj_state,   taitosj, ROT270, "Taito Corporation", "Water Ski", MACHINE_SUPPORTS_SAVE )
GAME( 1983, bioatack, 0,        nomcu,    bioatack, taitosj_state,   taitosj, ROT270, "Taito Corporation (Fox Video Games license)", "Bio Attack", MACHINE_SUPPORTS_SAVE )
GAME( 1984, sfposeid, 0,        mcu,      sfposeid, taitosj_state,   taitosj, ROT0,   "Taito Corporation", "Sea Fighter Poseidon", MACHINE_SUPPORTS_SAVE )
GAME( 1983, hwrace,   0,        nomcu,    hwrace, taitosj_state,     taitosj, ROT270, "Taito Corporation", "High Way Race", MACHINE_SUPPORTS_SAVE )
GAME( 1984, kikstart, 0,        kikstart, kikstart, taitosj_state,   taitosj, ROT0,   "Taito Corporation", "Kick Start - Wheelie King", MACHINE_SUPPORTS_SAVE )
