/***************************************************************************

Fire Trap memory map

driver by Nicola Salmoria

Z80:
0000-7fff ROM
8000-bfff Banked ROM (4 banks)
c000-cfff RAM
d000-d7ff bg #1 video/color RAM (alternating pages 0x100 long)
d000-dfff bg #2 video/color RAM (alternating pages 0x100 long)
e000-e3ff fg video RAM
e400-e7ff fg color RAM
e800-e97f sprites RAM

memory mapped ports:
read:
f010      IN0
f011      IN1
f012      IN2
f013      DSW0
f014      DSW1
f015      from pin 10 of 8751 controller
f016      from port #1 of 8751 controller

write:
f000      IRQ acknowledge
f001      sound command (also causes NMI on sound CPU)
f002      ROM bank selection
f003      flip screen
f004      NMI disable
f005      to port #2 of 8751 controller (signal on P3.2)
f008-f009 bg #1 x scroll
f00a-f00b bg #1 y scroll
f00c-f00d bg #2 x scroll
f00e-f00f bg #2 y scroll

interrupts:
VBlank triggers NMI.
the 8751 triggers IRQ

6502:
0000-07ff RAM
4000-7fff Banked ROM (2 banks)
8000-ffff ROM

read:
3400      command from the main cpu

write:
1000-1001 YM3526
2000      ADPCM data for the MSM5205 chip
2400      bit 0 = to sound chip MSM5205 (1 = play sample); bit 1 = IRQ enable
2800      ROM bank select

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"
#include "sound/3526intf.h"
#include "sound/msm5205.h"


extern UINT8 *firetrap_bg1videoram;
extern UINT8 *firetrap_bg2videoram;
extern UINT8 *firetrap_fgvideoram;

WRITE8_HANDLER( firetrap_fgvideoram_w );
WRITE8_HANDLER( firetrap_bg1videoram_w );
WRITE8_HANDLER( firetrap_bg2videoram_w );
WRITE8_HANDLER( firetrap_bg1_scrollx_w );
WRITE8_HANDLER( firetrap_bg1_scrolly_w );
WRITE8_HANDLER( firetrap_bg2_scrollx_w );
WRITE8_HANDLER( firetrap_bg2_scrolly_w );
VIDEO_START( firetrap );
PALETTE_INIT( firetrap );
VIDEO_UPDATE( firetrap );


static int firetrap_irq_enable = 0;
static int firetrap_nmi_enable;

static WRITE8_HANDLER( firetrap_nmi_disable_w )
{
	firetrap_nmi_enable=~data & 1;
}

static WRITE8_HANDLER( firetrap_bankselect_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(space->machine, "maincpu");

	bankaddress = 0x10000 + (data & 0x03) * 0x4000;
	memory_set_bankptr(space->machine, 1,&RAM[bankaddress]);
}

static READ8_HANDLER( firetrap_8751_bootleg_r )
{
	/* Check for coin insertion */
	/* the following only works in the bootleg version, which doesn't have an */
	/* 8751 - the real thing is much more complicated than that. */
	if ((input_port_read(space->machine, "IN2") & 0x70) != 0x70) return 0xff;
	return 0;
}

static int i8751_return,i8751_current_command;

static MACHINE_RESET( firetrap )
{
	i8751_current_command=0;
}

static READ8_HANDLER( firetrap_8751_r )
{
	//logerror("PC:%04x read from 8751\n",cpu_get_pc(space->cpu));
	return i8751_return;
}

static WRITE8_HANDLER( firetrap_8751_w )
{
	static int i8751_init_ptr=0;
	static const UINT8 i8751_init_data[]={
		0xf5,0xd5,0xdd,0x21,0x05,0xc1,0x87,0x5f,0x87,0x83,0x5f,0x16,0x00,0xdd,0x19,0xd1,
		0xf1,0xc9,0xf5,0xd5,0xfd,0x21,0x2f,0xc1,0x87,0x5f,0x16,0x00,0xfd,0x19,0xd1,0xf1,
		0xc9,0xe3,0xd5,0xc5,0xf5,0xdd,0xe5,0xfd,0xe5,0xe9,0xe1,0xfd,0xe1,0xdd,0xe1,0xf1,
		0xc1,0xd1,0xe3,0xc9,0xf5,0xc5,0xe5,0xdd,0xe5,0xc5,0x78,0xe6,0x0f,0x47,0x79,0x48,
		0x06,0x00,0xdd,0x21,0x00,0xd0,0xdd,0x09,0xe6,0x0f,0x6f,0x26,0x00,0x29,0x29,0x29,
		0x29,0xeb,0xdd,0x19,0xc1,0x78,0xe6,0xf0,0x28,0x05,0x11,0x00,0x02,0xdd,0x19,0x79,
		0xe6,0xf0,0x28,0x05,0x11,0x00,0x04,0xdd,0x19,0xdd,0x5e,0x00,0x01,0x00,0x01,0xdd,
		0x09,0xdd,0x56,0x00,0xdd,0xe1,0xe1,0xc1,0xf1,0xc9,0xf5,0x3e,0x01,0x32,0x04,0xf0,
		0xf1,0xc9,0xf5,0x3e,0x00,0x32,0x04,0xf0,0xf1,0xc9,0xf5,0xd5,0xdd,0x21,0x05,0xc1,
		0x87,0x5f,0x87,0x83,0x5f,0x16,0x00,0xdd,0x19,0xd1,0xf1,0xc9,0xf5,0xd5,0xfd,0x21,
		0x2f,0xc1,0x87,0x5f,0x16,0x00,0xfd,0x19,0xd1,0xf1,0xc9,0xe3,0xd5,0xc5,0xf5,0xdd,
		0xe5,0xfd,0xe5,0xe9,0xe1,0xfd,0xe1,0xdd,0xe1,0xf1,0xc1,0xd1,0xe3,0xc9,0xf5,0xc5,
		0xe5,0xdd,0xe5,0xc5,0x78,0xe6,0x0f,0x47,0x79,0x48,0x06,0x00,0xdd,0x21,0x00,0xd0,
		0xdd,0x09,0xe6,0x0f,0x6f,0x26,0x00,0x29,0x29,0x29,0x29,0xeb,0xdd,0x19,0xc1,0x78,
		0xe6,0xf0,0x28,0x05,0x11,0x00,0x02,0xdd,0x19,0x79,0xe6,0xf0,0x28,0x05,0x11,0x00,
		0x04,0xdd,0x19,0xdd,0x5e,0x00,0x01,0x00,0x01,0xdd,0x09,0xdd,0x56,0x00,0xdd,0x00
	};
	static const int i8751_coin_data[]={ 0x00, 0xb7 };
	static const int i8751_36_data[]={ 0x00, 0xbc };

	/* End of command - important to note, as coin input is supressed while commands are pending */
	if (data==0x26) {
		i8751_current_command=0;
		i8751_return=0xff; /* This value is XOR'd and must equal 0 */
		cputag_set_input_line_and_vector(space->machine, "maincpu", 0, HOLD_LINE, 0xff);
		return;
	}

	/* Init sequence command */
	else if (data==0x13) {
		if (!i8751_current_command)
			i8751_init_ptr=0;
		i8751_return=i8751_init_data[i8751_init_ptr++];
	}

	/* Used to calculate a jump address when coins are inserted */
	else if (data==0xbd) {
		if (!i8751_current_command)
			i8751_init_ptr=0;
		i8751_return=i8751_coin_data[i8751_init_ptr++];
	}

	else if (data==0x36) {
		if (!i8751_current_command)
			i8751_init_ptr=0;
		i8751_return=i8751_36_data[i8751_init_ptr++];
	}

	/* Static value commands */
	else if (data==0x14)
		i8751_return=1;
	else if (data==0x02)
		i8751_return=0;
	else if (data==0x72)
		i8751_return=3;
	else if (data==0x69)
		i8751_return=2;
	else if (data==0xcb)
		i8751_return=0;
	else if (data==0x49)
		i8751_return=1;
	else if (data==0x17)
		i8751_return=2;
	else if (data==0x88)
		i8751_return=3;
	else {
		i8751_return=0xff;
		logerror("%04x: Unknown i8751 command %02x!\n",cpu_get_pc(space->cpu),data);
	}

	/* Signal main cpu task is complete */
	cputag_set_input_line_and_vector(space->machine, "maincpu", 0, HOLD_LINE, 0xff);
	i8751_current_command=data;
}

static WRITE8_HANDLER( firetrap_sound_command_w )
{
	soundlatch_w(space, offset, data);
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( firetrap_sound_2400_w )
{
	msm5205_reset_w(devtag_get_device(space->machine, "msm"),~data & 0x01);
	firetrap_irq_enable = data & 0x02;
}

static WRITE8_HANDLER( firetrap_sound_bankselect_w )
{
	int bankaddress;
	UINT8 *RAM = memory_region(space->machine, "audiocpu");

	bankaddress = 0x10000 + (data & 0x01) * 0x4000;
	memory_set_bankptr(space->machine, 2, &RAM[bankaddress]);
}

static int msm5205next;

static void firetrap_adpcm_int (const device_config *device)
{
	static int toggle = 0;

	msm5205_data_w(device, msm5205next >> 4);
	msm5205next <<= 4;

	toggle ^= 1;
	if (firetrap_irq_enable && toggle)
		cputag_set_input_line (device->machine, "audiocpu", M6502_IRQ_LINE, HOLD_LINE);
}

static WRITE8_HANDLER( firetrap_adpcm_data_w )
{
	msm5205next = data;
}

static WRITE8_HANDLER( flip_screen_w )
{
	flip_screen_set(space->machine, data);
}


static ADDRESS_MAP_START( firetrap_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(firetrap_bg1videoram_w) AM_BASE(&firetrap_bg1videoram)
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(firetrap_bg2videoram_w) AM_BASE(&firetrap_bg2videoram)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(firetrap_fgvideoram_w) AM_BASE(&firetrap_fgvideoram)
	AM_RANGE(0xe800, 0xe97f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP	/* IRQ acknowledge */
	AM_RANGE(0xf001, 0xf001) AM_WRITE(firetrap_sound_command_w)
	AM_RANGE(0xf002, 0xf002) AM_WRITE(firetrap_bankselect_w)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(flip_screen_w)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(firetrap_nmi_disable_w)
	AM_RANGE(0xf005, 0xf005) AM_WRITE(firetrap_8751_w)
	AM_RANGE(0xf008, 0xf009) AM_WRITE(firetrap_bg1_scrollx_w)
	AM_RANGE(0xf00a, 0xf00b) AM_WRITE(firetrap_bg1_scrolly_w)
	AM_RANGE(0xf00c, 0xf00d) AM_WRITE(firetrap_bg2_scrollx_w)
	AM_RANGE(0xf00e, 0xf00f) AM_WRITE(firetrap_bg2_scrolly_w)
	AM_RANGE(0xf010, 0xf010) AM_READ_PORT("IN0")
	AM_RANGE(0xf011, 0xf011) AM_READ_PORT("IN1")
	AM_RANGE(0xf012, 0xf012) AM_READ_PORT("IN2")
	AM_RANGE(0xf013, 0xf013) AM_READ_PORT("DSW0")
	AM_RANGE(0xf014, 0xf014) AM_READ_PORT("DSW1")
	AM_RANGE(0xf016, 0xf016) AM_READ(firetrap_8751_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( firetrap_bootleg_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(firetrap_bg1videoram_w) AM_BASE(&firetrap_bg1videoram)
	AM_RANGE(0xd800, 0xdfff) AM_RAM_WRITE(firetrap_bg2videoram_w) AM_BASE(&firetrap_bg2videoram)
	AM_RANGE(0xe000, 0xe7ff) AM_RAM_WRITE(firetrap_fgvideoram_w) AM_BASE(&firetrap_fgvideoram)
	AM_RANGE(0xe800, 0xe97f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP	/* IRQ acknowledge */
	AM_RANGE(0xf001, 0xf001) AM_WRITE(firetrap_sound_command_w)
	AM_RANGE(0xf002, 0xf002) AM_WRITE(firetrap_bankselect_w)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(flip_screen_w)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(firetrap_nmi_disable_w)
	AM_RANGE(0xf005, 0xf005) AM_WRITENOP
	AM_RANGE(0xf008, 0xf009) AM_WRITE(firetrap_bg1_scrollx_w)
	AM_RANGE(0xf00a, 0xf00b) AM_WRITE(firetrap_bg1_scrolly_w)
	AM_RANGE(0xf00c, 0xf00d) AM_WRITE(firetrap_bg2_scrollx_w)
	AM_RANGE(0xf00e, 0xf00f) AM_WRITE(firetrap_bg2_scrolly_w)
	AM_RANGE(0xf010, 0xf010) AM_READ_PORT("IN0")
	AM_RANGE(0xf011, 0xf011) AM_READ_PORT("IN1")
	AM_RANGE(0xf012, 0xf012) AM_READ_PORT("IN2")
	AM_RANGE(0xf013, 0xf013) AM_READ_PORT("DSW0")
	AM_RANGE(0xf014, 0xf014) AM_READ_PORT("DSW1")
	AM_RANGE(0xf016, 0xf016) AM_READ(firetrap_8751_bootleg_r)
	AM_RANGE(0xf800, 0xf8ff) AM_ROM /* extra ROM in the bootleg with unprotection code */
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE("ym", ym3526_w)
	AM_RANGE(0x2000, 0x2000) AM_WRITE(firetrap_adpcm_data_w)	/* ADPCM data for the MSM5205 chip */
	AM_RANGE(0x2400, 0x2400) AM_WRITE(firetrap_sound_2400_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(firetrap_sound_bankselect_w)
	AM_RANGE(0x3400, 0x3400) AM_READ(soundlatch_r)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK(2)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( firetrap )
	PORT_START("IN0")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("IN2")	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("DSW0")	/* DSW0 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "50000 70000" )
	PORT_DIPSETTING(    0x20, "60000 80000" )
	PORT_DIPSETTING(    0x10, "80000 100000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("COIN")	/* Connected to i8751 directly */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END

static INPUT_PORTS_START( firetpbl )
	PORT_START("IN0")	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY

	PORT_START("IN1")	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_COCKTAIL

	PORT_START("IN2")	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )	/* bootleg only */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )	/* bootleg only */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )	/* bootleg only */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START("DSW0")	/* DSW0 */
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
//  PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x30, "50000 70000" )
	PORT_DIPSETTING(    0x20, "60000 80000" )
	PORT_DIPSETTING(    0x10, "80000 100000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 0, 4 },
	{ 3, 2, 1, 0, RGN_FRAC(1,2)+3, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};
static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 3, 2, 1, 0, RGN_FRAC(1,4)+3, RGN_FRAC(1,4)+2, RGN_FRAC(1,4)+1, RGN_FRAC(1,4)+0,
			16*8+3, 16*8+2, 16*8+1, 16*8+0, RGN_FRAC(1,4)+16*8+3, RGN_FRAC(1,4)+16*8+2, RGN_FRAC(1,4)+16*8+1, RGN_FRAC(1,4)+16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8
};
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
			16*8+7, 16*8+6, 16*8+5, 16*8+4, 16*8+3, 16*8+2, 16*8+1, 16*8+0 },
	{ 15*8, 14*8, 13*8, 12*8, 11*8, 10*8, 9*8, 8*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	32*8
};

static GFXDECODE_START( firetrap )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x00, 16 )	/* colors 0x00-0x3f */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,   0x80,  4 )	/* colors 0x80-0xbf */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,   0xc0,  4 )	/* colors 0xc0-0xff */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout, 0x40,  4 )	/* colors 0x40-0x7f */
GFXDECODE_END



static const msm5205_interface msm5205_config =
{
	firetrap_adpcm_int,	/* interrupt function */
	MSM5205_S48_4B		/* 8KHz ?             */
};

static INTERRUPT_GEN( firetrap )
{
	static int latch=0;
	static int coin_command_pending=0;

	/* Check for coin IRQ */
	if (cpu_getiloops(device))
	{
		if ((input_port_read(device->machine, "COIN") & 0x7) != 0x7 && !latch)
		{
			coin_command_pending = ~input_port_read(device->machine, "COIN");
			latch=1;
		}
		if ((input_port_read(device->machine, "COIN") & 0x7) == 0x7)
			latch=0;

		/* Make sure coin IRQ's aren't generated when another command is pending, the main cpu
            definitely doesn't expect them as it locks out the coin routine */
		if (coin_command_pending && !i8751_current_command) {
			i8751_return=coin_command_pending;
			cpu_set_input_line_and_vector(device,0,HOLD_LINE,0xff);
			coin_command_pending=0;
		}
	}

	if (firetrap_nmi_enable && !cpu_getiloops(device))
		cpu_set_input_line (device, INPUT_LINE_NMI, PULSE_LINE);
}

static INTERRUPT_GEN( bootleg )
{
	if (firetrap_nmi_enable)
		cpu_set_input_line (device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_DRIVER_START( firetrap )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 6000000)	/* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(firetrap_map)
	MDRV_CPU_VBLANK_INT_HACK(firetrap,2)

	MDRV_CPU_ADD("audiocpu", M6502,3072000/2)	/* 1.536 MHz? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
							/* IRQs are caused by the ADPCM chip */
							/* NMIs are caused by the main CPU */
	MDRV_MACHINE_RESET(firetrap)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(firetrap)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(firetrap)
	MDRV_VIDEO_START(firetrap)
	MDRV_VIDEO_UPDATE(firetrap)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM3526, 3000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( firetpbl )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 6000000)	/* 6 MHz */
	MDRV_CPU_PROGRAM_MAP(firetrap_bootleg_map)
	MDRV_CPU_VBLANK_INT("screen", bootleg)

	MDRV_CPU_ADD("audiocpu", M6502,3072000/2) /* 1.536 MHz? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
							/* IRQs are caused by the ADPCM chip */
							/* NMIs are caused by the main CPU */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(firetrap)
	MDRV_PALETTE_LENGTH(256)

	MDRV_PALETTE_INIT(firetrap)
	MDRV_VIDEO_START(firetrap)
	MDRV_VIDEO_UPDATE(firetrap)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM3526, 3000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("msm", MSM5205, 384000)
	MDRV_SOUND_CONFIG(msm5205_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( firetrap )
	ROM_REGION( 0x20000, "maincpu", 0 )	/* 64k for code + 64k for banked ROMs */
	ROM_LOAD( "di02.bin",     0x00000, 0x8000, CRC(3d1e4bf7) SHA1(ee903b469619f49edb1727fb545c9a6085f50746) )
	ROM_LOAD( "di01.bin",     0x10000, 0x8000, CRC(9bbae38b) SHA1(dc1d3ed5da71bfb104fd54fc70c56833f31d281f) )
	ROM_LOAD( "di00.bin",     0x18000, 0x8000, CRC(d0dad7de) SHA1(8783ebf6ddfef32f6036913d403f76c1545b813d) )

	ROM_REGION( 0x18000, "audiocpu", 0 )	/* 64k for the sound CPU + 32k for banked ROMs */
	ROM_LOAD( "di17.bin",     0x08000, 0x8000, CRC(8605f6b9) SHA1(4fba88f34afd91d2cbc578b3b70f5399b8844390) )
	ROM_LOAD( "di18.bin",     0x10000, 0x8000, CRC(49508c93) SHA1(3812b0b1a33a1506d2896d2b676ed6aabb29dac0) )

	/* there's also a protected 8751 microcontroller with ROM onboard */

	ROM_REGION( 0x02000, "gfx1", 0 )	/* characters */
	ROM_LOAD( "di03.bin",     0x00000, 0x2000, CRC(46721930) SHA1(a605fe993166e95c1602a35b548649ceae77bff2) )

	ROM_REGION( 0x20000, "gfx2", 0 )	/* tiles */
	ROM_LOAD( "di06.bin",     0x00000, 0x2000, CRC(441d9154) SHA1(340804e82d4aba8e9fcdd08cce0cfecefd2f77a9) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di04.bin",     0x04000, 0x2000, CRC(8e6e7eec) SHA1(9cff147702620987346449e2f83ef9b2efef7798) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di07.bin",     0x10000, 0x2000, CRC(ef0a7e23) SHA1(7c67ac27e6bde0f4943e8bed9898e730ae7ddd75) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di05.bin",     0x14000, 0x2000, CRC(ec080082) SHA1(3b034496bfa2aba9ed58ceba670d0364a9c2211d) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "di09.bin",     0x00000, 0x2000, CRC(d11e28e8) SHA1(3e91764f74d551e0984bac92daeab4e094e8dc13) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di08.bin",     0x04000, 0x2000, CRC(c32a21d8) SHA1(01898abf24aa40b13939afed96c990f430eb3bf1) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di11.bin",     0x10000, 0x2000, CRC(6424d5c3) SHA1(9ad6cfe6effca795709f90839a338f2a9148128f) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di10.bin",     0x14000, 0x2000, CRC(9b89300a) SHA1(5575daa226188cb1ea7d7a23f4966252bfb748e0) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "gfx4", 0 )	/* sprites */
	ROM_LOAD( "di16.bin",     0x00000, 0x8000, CRC(0de055d7) SHA1(ef763237c317545520c659f438b572b11c342d5a) )
	ROM_LOAD( "di13.bin",     0x08000, 0x8000, CRC(869219da) SHA1(9ab2439d6d1c62fce24c4f78ac7887f34c86cd75) )
	ROM_LOAD( "di14.bin",     0x10000, 0x8000, CRC(6b65812e) SHA1(209e07b2fced6b033c6d5398a998374588a35f46) )
	ROM_LOAD( "di15.bin",     0x18000, 0x8000, CRC(3e27f77d) SHA1(9ceccb1f56a8d0e05f6dea45d102690a1370624e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "firetrap.3b",  0x0000,  0x0100, CRC(8bb45337) SHA1(deaf6ea53eb3955230db1fdcb870079758a0c996) ) /* palette red and green component */
	ROM_LOAD( "firetrap.4b",  0x0100,  0x0100, CRC(d5abfc64) SHA1(6c808c1d6087804214dc29d35280f42382c40b18) ) /* palette blue component */
ROM_END

ROM_START( firetpbl )
	ROM_REGION( 0x28000, "maincpu", 0 )	/* 64k for code + 96k for banked ROMs */
	ROM_LOAD( "ft0d.bin",     0x00000, 0x8000, CRC(793ef849) SHA1(5a2c587370733d43484ba0a38a357260cdde8357) )
	ROM_LOAD( "ft0a.bin",     0x08000, 0x8000, CRC(613313ee) SHA1(54e386b2b1faada3441e3e0bb7822a63eab36930) )	/* unprotection code */
	ROM_LOAD( "ft0c.bin",     0x10000, 0x8000, CRC(5c8a0562) SHA1(856766851faa4353445d944b7705e348fd1379e4) )
	ROM_LOAD( "ft0b.bin",     0x18000, 0x8000, CRC(f2412fe8) SHA1(28a9143e36c31fe34f40888dc848aed3d572d801) )

	ROM_REGION( 0x18000, "audiocpu", 0 )	/* 64k for the sound CPU + 32k for banked ROMs */
	ROM_LOAD( "di17.bin",     0x08000, 0x8000, CRC(8605f6b9) SHA1(4fba88f34afd91d2cbc578b3b70f5399b8844390) )
	ROM_LOAD( "di18.bin",     0x10000, 0x8000, CRC(49508c93) SHA1(3812b0b1a33a1506d2896d2b676ed6aabb29dac0) )

	ROM_REGION( 0x02000, "gfx1", 0 )	/* characters */
	ROM_LOAD( "ft0e.bin",     0x00000, 0x2000, CRC(a584fc16) SHA1(6ac3692a14cb7c70799c23f8f6726fa5be1ac0d8) )

	ROM_REGION( 0x20000, "gfx2", 0 )	/* tiles */
	ROM_LOAD( "di06.bin",     0x00000, 0x2000, CRC(441d9154) SHA1(340804e82d4aba8e9fcdd08cce0cfecefd2f77a9) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di04.bin",     0x04000, 0x2000, CRC(8e6e7eec) SHA1(9cff147702620987346449e2f83ef9b2efef7798) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di07.bin",     0x10000, 0x2000, CRC(ef0a7e23) SHA1(7c67ac27e6bde0f4943e8bed9898e730ae7ddd75) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di05.bin",     0x14000, 0x2000, CRC(ec080082) SHA1(3b034496bfa2aba9ed58ceba670d0364a9c2211d) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "di09.bin",     0x00000, 0x2000, CRC(d11e28e8) SHA1(3e91764f74d551e0984bac92daeab4e094e8dc13) )
	ROM_CONTINUE(             0x08000, 0x2000 )
	ROM_CONTINUE(             0x02000, 0x2000 )
	ROM_CONTINUE(             0x0a000, 0x2000 )
	ROM_LOAD( "di08.bin",     0x04000, 0x2000, CRC(c32a21d8) SHA1(01898abf24aa40b13939afed96c990f430eb3bf1) )
	ROM_CONTINUE(             0x0c000, 0x2000 )
	ROM_CONTINUE(             0x06000, 0x2000 )
	ROM_CONTINUE(             0x0e000, 0x2000 )
	ROM_LOAD( "di11.bin",     0x10000, 0x2000, CRC(6424d5c3) SHA1(9ad6cfe6effca795709f90839a338f2a9148128f) )
	ROM_CONTINUE(             0x18000, 0x2000 )
	ROM_CONTINUE(             0x12000, 0x2000 )
	ROM_CONTINUE(             0x1a000, 0x2000 )
	ROM_LOAD( "di10.bin",     0x14000, 0x2000, CRC(9b89300a) SHA1(5575daa226188cb1ea7d7a23f4966252bfb748e0) )
	ROM_CONTINUE(             0x1c000, 0x2000 )
	ROM_CONTINUE(             0x16000, 0x2000 )
	ROM_CONTINUE(             0x1e000, 0x2000 )

	ROM_REGION( 0x20000, "gfx4", 0 )	/* sprites */
	ROM_LOAD( "di16.bin",     0x00000, 0x8000, CRC(0de055d7) SHA1(ef763237c317545520c659f438b572b11c342d5a) )
	ROM_LOAD( "di13.bin",     0x08000, 0x8000, CRC(869219da) SHA1(9ab2439d6d1c62fce24c4f78ac7887f34c86cd75) )
	ROM_LOAD( "di14.bin",     0x10000, 0x8000, CRC(6b65812e) SHA1(209e07b2fced6b033c6d5398a998374588a35f46) )
	ROM_LOAD( "di15.bin",     0x18000, 0x8000, CRC(3e27f77d) SHA1(9ceccb1f56a8d0e05f6dea45d102690a1370624e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "firetrap.3b",  0x0000,  0x0100, CRC(8bb45337) SHA1(deaf6ea53eb3955230db1fdcb870079758a0c996) ) /* palette red and green component */
	ROM_LOAD( "firetrap.4b",  0x0100,  0x0100, CRC(d5abfc64) SHA1(6c808c1d6087804214dc29d35280f42382c40b18) ) /* palette blue component */
ROM_END



GAME( 1986, firetrap, 0,        firetrap, firetrap, 0, ROT90, "Data East USA", "Fire Trap (US)", 0 )
GAME( 1986, firetpbl, firetrap, firetpbl, firetpbl, 0, ROT90, "bootleg", "Fire Trap (Japan bootleg)", 0 )
