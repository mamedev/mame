/***************************************************************************

Burger Time

driver by Zsolt Vasvari

hardware description:

Actually Lock'n'Chase is (C)1981 while Burger Time is (C)1982, so it might
be more accurate to say 'Lock'n'Chase hardware'.

The bootleg called Cook Race runs on hardware similar but different. The fact
that it addresses the program ROMs in the range 0500-3fff instead of the usual
c000-ffff makes me suspect that it is a bootleg of the *tape system* version.
Little is known about that system, but it is quite likely that it would have
RAM in the range 0000-3fff and load the program there from tape.


This hardware is pretty straightforward, but has a couple of interesting
twists. There are two ports to the video and color RAMs, one normal access,
and one with X and Y coordinates swapped. The sprite RAM occupies the
first row of the swapped area, so it appears in the regular video RAM as
the first column of on the left side.

These games don't have VBLANK interrupts, but instead an IRQ or NMI
(depending on the particular board) is generated when a coin is inserted.

Some of the games also have a background playfield which, in the
case of Bump 'n' Jump and Zoar, can be scrolled vertically.

These boards use two 8910's for sound, controlled by a dedicated 6502. The
main processor triggers an IRQ request when writing a command to the sound
CPU.

Main clock: XTAL = 12 MHz
Horizontal video frequency: HSYNC = XTAL/768?? = 15.625 kHz ??
Video frequency: VSYNC = HSYNC/272 = 57.44 Hz ?
VBlank duration: 1/VSYNC * (24/272) = 1536 us ?


Note on Lock'n'Chase:

The watchdog test prints "WATCHDOG TEST ER". Just by looking at the code,
I can't see how it could print anything else, there is only one path it
can take. Should the game reset????

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "includes/btime.h"

static WRITE8_HANDLER( audio_command_w );

static UINT8 *decrypted;
static UINT8 *rambase;
static UINT8 *audio_rambase;



INLINE UINT8 swap_bits_5_6(UINT8 data)
{
	return BITSWAP8(data,7,5,6,4,3,2,1,0);
}


static void btime_decrypt(void)
{
	UINT8 *src, *src1;
	int addr, addr1;


	/* the encryption is a simple bit rotation: 76543210 -> 65342710, but */
	/* with a catch: it is only applied if the previous instruction did a */
	/* memory write. Also, only opcodes at addresses with this bit pattern: */
	/* xxxx xxx1 xxxx x1xx are encrypted. */

	/* get the address of the next opcode */
	addr = activecpu_get_pc();

	/* however if the previous instruction was JSR (which caused a write to */
	/* the stack), fetch the address of the next instruction. */
	addr1 = activecpu_get_previouspc();
	src1 = (addr1 < 0x9000) ? rambase : memory_region(REGION_CPU1);
	if (decrypted[addr1] == 0x20)	/* JSR $xxxx */
		addr = src1[addr1+1] + 256 * src1[addr1+2];

	/* If the address of the next instruction is xxxx xxx1 xxxx x1xx, decode it. */
	src = (addr < 0x9000) ? rambase : memory_region(REGION_CPU1);
	if ((addr & 0x0104) == 0x0104)
	{
		/* 76543210 -> 65342710 bit rotation */
		decrypted[addr] = BITSWAP8(src[addr],6,5,3,4,2,7,1,0);
	}
}

static WRITE8_HANDLER( lnc_w )
{
	if      (offset <= 0x3bff)                       ;
	else if (offset >= 0x3c00 && offset <= 0x3fff) { lnc_videoram_w(machine,offset - 0x3c00,data); return; }
	else if (offset >= 0x7c00 && offset <= 0x7fff) { lnc_mirrorvideoram_w(machine,offset - 0x7c00,data); return; }
	else if (offset == 0x8000)                     { return; }  /* SMH_NOP */
	else if (offset == 0x8001)                     { lnc_video_control_w(machine,0,data); return; }
	else if (offset == 0x8003)                       ;
	else if (offset == 0x9000)                     { return; }  /* SMH_NOP */
	else if (offset == 0x9002)                     { audio_command_w(machine,0,data); return; }
	else if (offset >= 0xb000 && offset <= 0xb1ff)   ;
	else logerror("CPU #%d PC %04x: warning - write %02x to unmapped memory address %04x\n",cpu_getactivecpu(),activecpu_get_pc(),data,offset);

	rambase[offset] = data;

	/* Swap bits 5 & 6 for opcodes */
	decrypted[offset] = swap_bits_5_6(data);
}

static WRITE8_HANDLER( mmonkey_w )
{
	if      (offset <= 0x3bff)                       ;
	else if (offset >= 0x3c00 && offset <= 0x3fff) { lnc_videoram_w(machine,offset - 0x3c00,data); return; }
	else if (offset >= 0x7c00 && offset <= 0x7fff) { lnc_mirrorvideoram_w(machine,offset - 0x7c00,data); return; }
	else if (offset == 0x8001)                     { lnc_video_control_w(machine,0,data); return; }
	else if (offset == 0x8003)                       ;
	else if (offset == 0x9000)                     { return; }  /* SMH_NOP */
	else if (offset == 0x9002)                     { audio_command_w(machine,0,data); return; }
	else if (offset >= 0xb000 && offset <= 0xbfff) { mmonkey_protection_w(machine,offset - 0xb000, data); return; }
	else logerror("CPU #%d PC %04x: warning - write %02x to unmapped memory address %04x\n",cpu_getactivecpu(),activecpu_get_pc(),data,offset);

	rambase[offset] = data;

	/* Swap bits 5 & 6 for opcodes */
	decrypted[offset] = swap_bits_5_6(data);
}

static WRITE8_HANDLER( btime_w )
{
	if      (offset <= 0x07ff)                     ;
	else if (offset >= 0x0c00 && offset <= 0x0c0f) btime_paletteram_w(machine,offset - 0x0c00,data);
	else if (offset >= 0x1000 && offset <= 0x17ff) ;
	else if (offset >= 0x1800 && offset <= 0x1bff) btime_mirrorvideoram_w(machine,offset - 0x1800,data);
	else if (offset >= 0x1c00 && offset <= 0x1fff) btime_mirrorcolorram_w(machine,offset - 0x1c00,data);
	else if (offset == 0x4002)                     btime_video_control_w(machine,0,data);
	else if (offset == 0x4003)                     audio_command_w(machine,0,data);
	else if (offset == 0x4004)                     bnj_scroll1_w(machine,0,data);
	else logerror("CPU #%d PC %04x: warning - write %02x to unmapped memory address %04x\n",cpu_getactivecpu(),activecpu_get_pc(),data,offset);

	rambase[offset] = data;

	btime_decrypt();
}

static WRITE8_HANDLER( zoar_w )
{
	if      (offset <= 0x07ff) 					   ;
	else if (offset >= 0x8000 && offset <= 0x87ff) ;
	else if (offset >= 0x8800 && offset <= 0x8bff) btime_mirrorvideoram_w(machine,offset - 0x8800,data);
	else if (offset >= 0x8c00 && offset <= 0x8fff) btime_mirrorcolorram_w(machine,offset - 0x8c00,data);
	else if (offset == 0x9000)					   zoar_video_control_w(machine,0, data);
	else if (offset >= 0x9800 && offset <= 0x9803) ;
	else if (offset == 0x9804)                     bnj_scroll2_w(machine,0,data);
	else if (offset == 0x9805)                     bnj_scroll1_w(machine,0,data);
	else if (offset == 0x9806)                     audio_command_w(machine,0,data);
	else logerror("CPU #%d PC %04x: warning - write %02x to unmapped memory address %04x\n",cpu_getactivecpu(),activecpu_get_pc(),data,offset);

	rambase[offset] = data;

	btime_decrypt();
}

static WRITE8_HANDLER( disco_w )
{
	if      (offset <= 0x04ff)                     ;
	else if (offset >= 0x2000 && offset <= 0x7fff) deco_charram_w(machine,offset - 0x2000,data);
	else if (offset >= 0x8000 && offset <= 0x881f) ;
	else if (offset == 0x9a00)                     audio_command_w(machine,0,data);
	else if (offset == 0x9c00)                     disco_video_control_w(machine,0,data);
	else logerror("CPU #%d PC %04x: warning - write %02x to unmapped memory address %04x\n",cpu_getactivecpu(),activecpu_get_pc(),data,offset);

	rambase[offset] = data;

	btime_decrypt();
}


static ADDRESS_MAP_START( btime_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(btime_w)	/* override the following entries to */
												/* support ROM decryption */
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&rambase)
	AM_RANGE(0x0c00, 0x0c0f) AM_WRITE(btime_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_BASE(&btime_videoram) AM_SIZE(&btime_videoram_size)
	AM_RANGE(0x1400, 0x17ff) AM_RAM AM_BASE(&btime_colorram)
	AM_RANGE(0x1800, 0x1bff) AM_READWRITE(btime_mirrorvideoram_r, btime_mirrorvideoram_w)
	AM_RANGE(0x1c00, 0x1fff) AM_READWRITE(btime_mirrorcolorram_r, btime_mirrorcolorram_w)
	AM_RANGE(0x4000, 0x4000) AM_READWRITE(input_port_0_r, SMH_NOP)
	AM_RANGE(0x4001, 0x4001) AM_READ(input_port_1_r)
	AM_RANGE(0x4002, 0x4002) AM_READWRITE(input_port_2_r, btime_video_control_w)
	AM_RANGE(0x4003, 0x4003) AM_READWRITE(input_port_3_r, audio_command_w)
	AM_RANGE(0x4004, 0x4004) AM_READWRITE(input_port_4_r, bnj_scroll1_w)
	AM_RANGE(0xb000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cookrace_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_BASE(&rambase)
	AM_RANGE(0x0500, 0x3fff) AM_ROM
	AM_RANGE(0xc000, 0xc3ff) AM_RAM AM_BASE(&btime_videoram) AM_SIZE(&btime_videoram_size)
	AM_RANGE(0xc400, 0xc7ff) AM_RAM AM_BASE(&btime_colorram)
	AM_RANGE(0xc800, 0xcbff) AM_READWRITE(btime_mirrorvideoram_r, btime_mirrorvideoram_w)
	AM_RANGE(0xcc00, 0xcfff) AM_READWRITE(btime_mirrorcolorram_r, btime_mirrorcolorram_w)
	AM_RANGE(0xd000, 0xd0ff) AM_RAM							/* background? */
	AM_RANGE(0xd100, 0xd3ff) AM_RAM							/* ? */
	AM_RANGE(0xd400, 0xd7ff) AM_RAM AM_BASE(&bnj_backgroundram) AM_SIZE(&bnj_backgroundram_size)
	AM_RANGE(0xe000, 0xe000) AM_READWRITE(input_port_3_r, bnj_video_control_w)
	AM_RANGE(0xe300, 0xe300) AM_READ(input_port_3_r)     /* mirror address used on high score name entry */
											/* screen */
	AM_RANGE(0xe001, 0xe001) AM_READWRITE(input_port_4_r, audio_command_w)
	AM_RANGE(0xe002, 0xe002) AM_READ(input_port_0_r)
	AM_RANGE(0xe003, 0xe003) AM_READ(input_port_1_r)
	AM_RANGE(0xe004, 0xe004) AM_READ(input_port_2_r)
	AM_RANGE(0xfff9, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( zoar_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(zoar_w)	/* override the following entries to */
												/* support ROM decryption */
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&rambase)
	AM_RANGE(0x8000, 0x83ff) AM_WRITE(SMH_RAM) AM_BASE(&btime_videoram) AM_SIZE(&btime_videoram_size)
	AM_RANGE(0x8400, 0x87ff) AM_WRITE(SMH_RAM) AM_BASE(&btime_colorram)
	AM_RANGE(0x8800, 0x8bff) AM_WRITE(btime_mirrorvideoram_w)
	AM_RANGE(0x8c00, 0x8fff) AM_WRITE(btime_mirrorcolorram_w)
	AM_RANGE(0x9000, 0x9000) AM_WRITE(zoar_video_control_w)
	AM_RANGE(0x9800, 0x9800) AM_READ(input_port_3_r)
	AM_RANGE(0x9801, 0x9801) AM_READ(input_port_4_r)
	AM_RANGE(0x9802, 0x9802) AM_READ(input_port_0_r)
	AM_RANGE(0x9803, 0x9803) AM_READ(input_port_1_r)
	AM_RANGE(0x9800, 0x9803) AM_WRITE(SMH_RAM) AM_BASE(&zoar_scrollram)
	AM_RANGE(0x9804, 0x9804) AM_READWRITE(input_port_2_r, bnj_scroll2_w)
	AM_RANGE(0x9805, 0x9805) AM_WRITE(bnj_scroll1_w)
	AM_RANGE(0x9806, 0x9806) AM_WRITE(audio_command_w)
	AM_RANGE(0xd000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lnc_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(lnc_w)	/* override the following entries to */
												/* support ROM decryption */
	AM_RANGE(0x0000, 0x3bff) AM_RAM AM_BASE(&rambase)
	AM_RANGE(0x3c00, 0x3fff) AM_RAM_WRITE(lnc_videoram_w) AM_BASE(&btime_videoram) AM_SIZE(&btime_videoram_size)
	AM_RANGE(0x7800, 0x7bff) AM_WRITE(SMH_RAM) AM_BASE(&btime_colorram)  /* this is just here to initialize the pointer */
	AM_RANGE(0x7c00, 0x7fff) AM_READWRITE(btime_mirrorvideoram_r, lnc_mirrorvideoram_w)
	AM_RANGE(0x8000, 0x8000) AM_READWRITE(input_port_3_r, SMH_NOP)     /* ??? */
	AM_RANGE(0x8001, 0x8001) AM_READWRITE(input_port_4_r, lnc_video_control_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE(SMH_RAM) AM_BASE(&lnc_charbank)
	AM_RANGE(0x9000, 0x9000) AM_READWRITE(input_port_0_r, SMH_NOP)     /* IRQ ack??? */
	AM_RANGE(0x9001, 0x9001) AM_READ(input_port_1_r)
	AM_RANGE(0x9002, 0x9002) AM_READWRITE(input_port_2_r, audio_command_w)
	AM_RANGE(0xb000, 0xb1ff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mmonkey_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(mmonkey_w)	/* override the following entries to */
													/* support ROM decryption */
	AM_RANGE(0x0000, 0x3bff) AM_RAM AM_BASE(&rambase)
	AM_RANGE(0x3c00, 0x3fff) AM_RAM_WRITE(lnc_videoram_w) AM_BASE(&btime_videoram) AM_SIZE(&btime_videoram_size)
	AM_RANGE(0x7800, 0x7bff) AM_WRITE(SMH_RAM) AM_BASE(&btime_colorram)  /* this is just here to initialize the pointer */
	AM_RANGE(0x7c00, 0x7fff) AM_READWRITE(btime_mirrorvideoram_r, lnc_mirrorvideoram_w)
	AM_RANGE(0x8000, 0x8000) AM_READ(input_port_3_r)
	AM_RANGE(0x8001, 0x8001) AM_READWRITE(input_port_4_r, lnc_video_control_w)
	AM_RANGE(0x8003, 0x8003) AM_WRITE(SMH_RAM) AM_BASE(&lnc_charbank)
	AM_RANGE(0x9000, 0x9000) AM_READWRITE(input_port_0_r, SMH_NOP)     /* IRQ ack??? */
	AM_RANGE(0x9001, 0x9001) AM_READ(input_port_1_r)
	AM_RANGE(0x9002, 0x9002) AM_READWRITE(input_port_2_r, audio_command_w)
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE(mmonkey_protection_r, mmonkey_protection_w)
	AM_RANGE(0xc000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bnj_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_BASE(&rambase)
	AM_RANGE(0x1000, 0x1000) AM_READ(input_port_3_r)
	AM_RANGE(0x1001, 0x1001) AM_READWRITE(input_port_4_r, bnj_video_control_w)
	AM_RANGE(0x1002, 0x1002) AM_READWRITE(input_port_0_r, audio_command_w)
	AM_RANGE(0x1003, 0x1003) AM_READ(input_port_1_r)
	AM_RANGE(0x1004, 0x1004) AM_READ(input_port_2_r)
	AM_RANGE(0x4000, 0x43ff) AM_RAM AM_BASE(&btime_videoram) AM_SIZE(&btime_videoram_size)
	AM_RANGE(0x4400, 0x47ff) AM_RAM AM_BASE(&btime_colorram)
	AM_RANGE(0x4800, 0x4bff) AM_READWRITE(btime_mirrorvideoram_r, btime_mirrorvideoram_w)
	AM_RANGE(0x4c00, 0x4fff) AM_READWRITE(btime_mirrorcolorram_r, btime_mirrorcolorram_w)
	AM_RANGE(0x5000, 0x51ff) AM_WRITE(bnj_background_w) AM_BASE(&bnj_backgroundram) AM_SIZE(&bnj_backgroundram_size)
	AM_RANGE(0x5400, 0x5400) AM_WRITE(bnj_scroll1_w)
	AM_RANGE(0x5800, 0x5800) AM_WRITE(bnj_scroll2_w)
	AM_RANGE(0x5c00, 0x5c0f) AM_WRITE(btime_paletteram_w) AM_BASE(&paletteram)
	AM_RANGE(0xa000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END

static ADDRESS_MAP_START( disco_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_WRITE(disco_w)	/* override the following entries to */
												/* support ROM decryption */
	AM_RANGE(0x0000, 0x04ff) AM_RAM AM_BASE(&rambase)
	AM_RANGE(0x2000, 0x7fff) AM_RAM_WRITE(deco_charram_w) AM_BASE(&deco_charram)
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_BASE(&btime_videoram) AM_SIZE(&btime_videoram_size)
	AM_RANGE(0x8400, 0x87ff) AM_RAM AM_BASE(&btime_colorram)
	AM_RANGE(0x8800, 0x881f) AM_RAM AM_BASE(&spriteram) AM_SIZE(&spriteram_size)
	AM_RANGE(0x9000, 0x9000) AM_READ(input_port_2_r)
	AM_RANGE(0x9200, 0x9200) AM_READ(input_port_0_r)
	AM_RANGE(0x9400, 0x9400) AM_READ(input_port_1_r)
	AM_RANGE(0x9800, 0x9800) AM_READ(input_port_3_r)
	AM_RANGE(0x9a00, 0x9a00) AM_READWRITE(input_port_4_r, audio_command_w)
	AM_RANGE(0x9c00, 0x9c00) AM_READWRITE(input_port_5_r, disco_video_control_w)
	AM_RANGE(0xa000, 0xffff) AM_READ(SMH_ROM)
ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_BASE(&audio_rambase)
	AM_RANGE(0x0400, 0x0fff) AM_ROM AM_REGION(REGION_CPU2, 0xf400)
	AM_RANGE(0x2000, 0x2fff) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x8000, 0x8fff) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0xa000, 0xafff) AM_READ(soundlatch_r)
	AM_RANGE(0xc000, 0xcfff) AM_WRITE(interrupt_enable_w)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( disco_audio_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x4000, 0x4fff) AM_WRITE(AY8910_write_port_0_w)
	AM_RANGE(0x5000, 0x5fff) AM_WRITE(AY8910_control_port_0_w)
	AM_RANGE(0x6000, 0x6fff) AM_WRITE(AY8910_write_port_1_w)
	AM_RANGE(0x7000, 0x7fff) AM_WRITE(AY8910_control_port_1_w)
	AM_RANGE(0x8000, 0x8fff) AM_READWRITE(soundlatch_r, SMH_NOP) /* ack ? */
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_CHANGED( coin_inserted_irq_hi )
{
	if (newval)
		cpunum_set_input_line(field->port->machine, 0, 0, HOLD_LINE);
}

static INPUT_CHANGED( coin_inserted_irq_lo )
{
	if (!newval)
		cpunum_set_input_line(field->port->machine, 0, 0, HOLD_LINE);
}

static INPUT_CHANGED( coin_inserted_nmi_lo )
{
	cpunum_set_input_line(field->port->machine, 0, INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


static WRITE8_HANDLER( audio_command_w )
{
	soundlatch_w(machine,offset,data);
	cpunum_set_input_line(machine, 1, 0, HOLD_LINE);
}

#define COMMON_INPUTS\
	PORT_START_TAG("IN0")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_START_TAG("IN1")\
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL\
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL\
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL\
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL\
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL\
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )\
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )\
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )\

static INPUT_PORTS_START( btime )
	COMMON_INPUTS

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED(coin_inserted_irq_hi, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED(coin_inserted_irq_hi, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x20, "Cross Hatch Pattern" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "15000" )
	PORT_DIPSETTING(    0x02, "20000"  )
	PORT_DIPSETTING(    0x00, "30000"  )
	PORT_DIPNAME( 0x08, 0x08, "Enemies" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "End of Level Pepper" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cookrace )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_COIN2 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_COIN1 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000"  )
	PORT_DIPSETTING(    0x00, "50000"  )
	PORT_DIPNAME( 0x08, 0x08, "Enemies" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "End of Level Pepper" )
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( zoar )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED(coin_inserted_irq_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED(coin_inserted_irq_lo, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )    /* almost certainly unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	/* Service mode doesn't work because of missing ROMs */
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "5000" )
	PORT_DIPSETTING(    0x04, "10000" )
	PORT_DIPSETTING(    0x02, "15000"  )
	PORT_DIPSETTING(    0x00, "20000"  )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, "Weapon Select" )
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_DIPSETTING(    0x10, "Auto" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )    /* These 3 switches     */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )        /* have to do with      */
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )         /* coinage.             */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )    /* See code at $d234.   */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )        /* Feel free to figure  */
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )         /* them out.            */
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lnc )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x30, "Test Mode" )
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, "RAM Test Only" )
	PORT_DIPSETTING(    0x20, "Watchdog Test Only" )
	PORT_DIPSETTING(    0x10, "All Tests" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "15000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Speed" )
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wtennis )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )   /* definitely used */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )    /* These 3 switches     */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )        /* have to do with      */
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )         /* coinage.             */
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mmonkey )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )   /* almost certainly unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x02, "Every 15000" )
	PORT_DIPSETTING(    0x04, "Every 30000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, "Level Skip Mode (Cheat)")
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )   /* almost certainly unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )   /* almost certainly unused */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( bnj )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "Every 30000" )
	PORT_DIPSETTING(    0x04, "Every 70000" )
	PORT_DIPSETTING(    0x02, "20000 Only"  )
	PORT_DIPSETTING(    0x00, "30000 Only"  )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( disco )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START2 )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_COIN1 ) PORT_CHANGED(coin_inserted_irq_hi, 0)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_COIN2 ) PORT_CHANGED(coin_inserted_irq_hi, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x02, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x06, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, "Music Weapons" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START_TAG("VBLANK")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK )
INPUT_PORTS_END

static INPUT_PORTS_START( sdtennis )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED(coin_inserted_nmi_lo, 0)

	PORT_START_TAG("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_VBLANK  )

	PORT_START_TAG("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "1 Set won" )
	PORT_DIPSETTING(    0x04, "2 Sets won" )
	PORT_DIPSETTING(    0x02, "3 Sets won"  )
	PORT_DIPSETTING(    0x00, DEF_STR( None )  )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )		// Check code at 0xc55b
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )		// Check code at 0xc5af
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Copyright" )
	PORT_DIPSETTING(    0xe0, "Data East Corporation" )
	PORT_DIPSETTING(    0xc0, "Data East USA" )
	/* Other values are the same as 0xe0 */
	/* 0x60 also gives a special coinage : COIN1 gives 3 credits and COIN2 gives 8 credits
      whatever the coinage Dip Switch are (they are not read in this case) */
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{ 2*1024*8*8, 1024*8*8, 0 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	3,      /* 3 bits per pixel */
	{ 2*256*16*16, 256*16*16, 0 },  /* the bitplanes are separated */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
	  0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout zoar_spritelayout =
{
	16,16,  /* 16*16 sprites */
	128,    /* 256 sprites */
	3,      /* 3 bits per pixel */
	{ 2*128*16*16, 128*16*16, 0 },  /* the bitplanes are separated */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
	  0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout btime_tilelayout =
{
	16,16,  /* 16*16 characters */
	64,    /* 64 characters */
	3,      /* 3 bits per pixel */
	{  2*64*16*16, 64*16*16, 0 },    /* the bitplanes are separated */
	{ 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
	  0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    /* every tile takes 32 consecutive bytes */
};

static const gfx_layout cookrace_tilelayout =
{
	8,8,  /* 8*8 characters */
	256,    /* 256 characters */
	3,      /* 3 bits per pixel */
	{  2*256*8*8, 256*8*8, 0 },    /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8    /* every tile takes 8 consecutive bytes */
};

static const gfx_layout bnj_tilelayout =
{
	16,16,  /* 16*16 characters */
	64, /* 64 characters */
	3,  /* 3 bits per pixel */
	{ 2*64*16*16+4, 0, 4 },
	{ 3*16*8+0, 3*16*8+1, 3*16*8+2, 3*16*8+3, 2*16*8+0, 2*16*8+1, 2*16*8+2, 2*16*8+3,
	  16*8+0, 16*8+1, 16*8+2, 16*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	64*8    /* every tile takes 64 consecutive bytes */
};

static GFXDECODE_START( btime )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,          0, 1 ) /* char set #1 */
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout,        0, 1 ) /* sprites */
	GFXDECODE_ENTRY( REGION_GFX2, 0, btime_tilelayout,    8, 1 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( cookrace )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,          0, 1 ) /* char set #1 */
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout,        0, 1 ) /* sprites */
	GFXDECODE_ENTRY( REGION_GFX2, 0, cookrace_tilelayout, 8, 1 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( lnc )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,          0, 1 )     /* char set #1 */
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout,        0, 1 )     /* sprites */
GFXDECODE_END

static GFXDECODE_START( bnj )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,          0, 1 ) /* char set #1 */
	GFXDECODE_ENTRY( REGION_GFX1, 0, spritelayout,        0, 1 ) /* sprites */
	GFXDECODE_ENTRY( REGION_GFX2, 0, bnj_tilelayout,      8, 1 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( zoar )
	GFXDECODE_ENTRY( REGION_GFX1, 0, charlayout,          0, 8 ) /* char set #1 */
	GFXDECODE_ENTRY( REGION_GFX4, 0, zoar_spritelayout,   0, 8 ) /* sprites */
	GFXDECODE_ENTRY( REGION_GFX2, 0, btime_tilelayout,    0, 8 ) /* background tiles */
GFXDECODE_END

static GFXDECODE_START( disco )
	GFXDECODE_ENTRY( 0, 0x2000, charlayout,          0, 4 ) /* char set #1 */
	GFXDECODE_ENTRY( 0, 0x2000, spritelayout,        0, 4 ) /* sprites */
GFXDECODE_END



static MACHINE_DRIVER_START( btime )

	/* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M6502, 1500000)
	MDRV_CPU_PROGRAM_MAP(btime_map,0)

	/* audio CPU */
	MDRV_CPU_ADD_TAG("audio", M6502, 500000)
	MDRV_CPU_PROGRAM_MAP(audio_map,0)
	MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,16)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(3072))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)

	MDRV_GFXDECODE(btime)
	MDRV_PALETTE_LENGTH(16)

	MDRV_PALETTE_INIT(btime)
	MDRV_VIDEO_START(btime)
	MDRV_VIDEO_UPDATE(btime)

	/* audio hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)

	MDRV_SOUND_ADD(AY8910, 1500000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.23)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( cookrace )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(btime)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(cookrace_map,0)

	MDRV_CPU_MODIFY("audio")
	MDRV_CPU_PROGRAM_MAP(audio_map,0)

	/* video hardware */
	MDRV_GFXDECODE(cookrace)
	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_UPDATE(cookrace)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( lnc )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(btime)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(lnc_map,0)

	MDRV_CPU_MODIFY("audio")
	MDRV_CPU_VBLANK_INT_HACK(lnc_sound_interrupt,16)

	MDRV_MACHINE_RESET(lnc)

	/* video hardware */
	MDRV_GFXDECODE(lnc)
	MDRV_PALETTE_LENGTH(8)

	MDRV_PALETTE_INIT(lnc)
	MDRV_VIDEO_UPDATE(lnc)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( wtennis )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(lnc)
	MDRV_CPU_MODIFY("audio")
	MDRV_CPU_VBLANK_INT_HACK(nmi_line_pulse,16)

	/* video hardware */
	MDRV_VIDEO_UPDATE(eggs)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( mmonkey )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(wtennis)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(mmonkey_map,0)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( bnj )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(btime)
	MDRV_CPU_REPLACE("main", M6502, 750000)
	MDRV_CPU_PROGRAM_MAP(bnj_map,0)

	/* video hardware */
	MDRV_GFXDECODE(bnj)
	MDRV_PALETTE_LENGTH(16)

	MDRV_VIDEO_START(bnj)
	MDRV_VIDEO_UPDATE(bnj)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( zoar )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(btime)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(zoar_map,0)

	/* video hardware */
	MDRV_GFXDECODE(zoar)
	MDRV_PALETTE_LENGTH(64)

	MDRV_VIDEO_UPDATE(zoar)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( disco )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(btime)
	MDRV_CPU_REPLACE("main", M6502, 750000)
	MDRV_CPU_PROGRAM_MAP(disco_map,0)

	MDRV_CPU_MODIFY("audio")
	MDRV_CPU_PROGRAM_MAP(disco_audio_map,0)

	/* video hardware */
	MDRV_GFXDECODE(disco)
	MDRV_PALETTE_LENGTH(32)

	MDRV_VIDEO_UPDATE(disco)
MACHINE_DRIVER_END

/***************************************************************************

    Game driver(s)

***************************************************************************/

ROM_START( btime )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "aa04.9b",      0xc000, 0x1000, CRC(368a25b5) SHA1(ed3f3712423979dcb351941fa85dce6a0a7bb16b) )
	ROM_LOAD( "aa06.13b",     0xd000, 0x1000, CRC(b4ba400d) SHA1(8c77397e934907bc47a739f263196a0f2f81ba3d) )
	ROM_LOAD( "aa05.10b",     0xe000, 0x1000, CRC(8005bffa) SHA1(d0da4e360039f6a8d8142a4e8e05c1f90c0af68a) )
	ROM_LOAD( "aa07.15b",     0xf000, 0x1000, CRC(086440ad) SHA1(4a32bc92f8ff5fbe112f56e62d2c03da8851a7b9) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ab14.12h",     0xf000, 0x1000, CRC(f55e5211) SHA1(27940026d0c6212d1138d2fd88880df697218627) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "aa12.7k",      0x0000, 0x1000, CRC(c4617243) SHA1(24204d591aa2c264a852ee9ba8c4be63efd97728) )    /* charset #1 */
	ROM_LOAD( "ab13.9k",      0x1000, 0x1000, CRC(ac01042f) SHA1(e64b6381a9298eaf74e79fa5f1ea8e9596c58a49) )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, CRC(854a872a) SHA1(3d2ecfd54a5a9d68b53cf4b4ee1f2daa6aef2123) )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, CRC(d4848014) SHA1(0a55b091cd4e7f317c35defe13d5051b26042eee) )
	ROM_LOAD( "aa8.13k",      0x4000, 0x1000, CRC(8650c788) SHA1(d9b1ee2d1f2fd66705d497c80252861b49aa9254) )
	ROM_LOAD( "ab9.15k",      0x5000, 0x1000, CRC(8dec15e6) SHA1(b72633de6268ce16742bba4dcba835df860d6c2f) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ab00.1b",      0x0000, 0x0800, CRC(c7a14485) SHA1(6a0a8e6b7860859f22daa33634e34fbf91387659) )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x0800, 0x0800, CRC(25b49078) SHA1(4abdcbd4f3362c3e4463a1274731289f1a72d2e6) )
	ROM_LOAD( "ab02.4b",      0x1000, 0x0800, CRC(b8ef56c3) SHA1(4a03bf011dc1fb2902f42587b1174b880cf06df1) )

	ROM_REGION( 0x0800, REGION_GFX3, 0 )	/* background tilemaps */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, CRC(d26bc1f3) SHA1(737af6e264183a1f151f277a07cf250d6abb3fd8) )
ROM_END

ROM_START( btime2 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "aa04.9b2",     0xc000, 0x1000, CRC(a041e25b) SHA1(caaab3ae46619d0a87a8985d316411f23be0b696) )
	ROM_LOAD( "aa06.13b",     0xd000, 0x1000, CRC(b4ba400d) SHA1(8c77397e934907bc47a739f263196a0f2f81ba3d) )
	ROM_LOAD( "aa05.10b",     0xe000, 0x1000, CRC(8005bffa) SHA1(d0da4e360039f6a8d8142a4e8e05c1f90c0af68a) )
	ROM_LOAD( "aa07.15b",     0xf000, 0x1000, CRC(086440ad) SHA1(4a32bc92f8ff5fbe112f56e62d2c03da8851a7b9) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ab14.12h",     0xf000, 0x1000, CRC(f55e5211) SHA1(27940026d0c6212d1138d2fd88880df697218627) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "aa12.7k",      0x0000, 0x1000, CRC(c4617243) SHA1(24204d591aa2c264a852ee9ba8c4be63efd97728) )    /* charset #1 */
	ROM_LOAD( "ab13.9k",      0x1000, 0x1000, CRC(ac01042f) SHA1(e64b6381a9298eaf74e79fa5f1ea8e9596c58a49) )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, CRC(854a872a) SHA1(3d2ecfd54a5a9d68b53cf4b4ee1f2daa6aef2123) )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, CRC(d4848014) SHA1(0a55b091cd4e7f317c35defe13d5051b26042eee) )
	ROM_LOAD( "aa8.13k",      0x4000, 0x1000, CRC(8650c788) SHA1(d9b1ee2d1f2fd66705d497c80252861b49aa9254) )
	ROM_LOAD( "ab9.15k",      0x5000, 0x1000, CRC(8dec15e6) SHA1(b72633de6268ce16742bba4dcba835df860d6c2f) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ab00.1b",      0x0000, 0x0800, CRC(c7a14485) SHA1(6a0a8e6b7860859f22daa33634e34fbf91387659) )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x0800, 0x0800, CRC(25b49078) SHA1(4abdcbd4f3362c3e4463a1274731289f1a72d2e6) )
	ROM_LOAD( "ab02.4b",      0x1000, 0x0800, CRC(b8ef56c3) SHA1(4a03bf011dc1fb2902f42587b1174b880cf06df1) )

	ROM_REGION( 0x0800, REGION_GFX3, 0 )	/* background tilemaps */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, CRC(d26bc1f3) SHA1(737af6e264183a1f151f277a07cf250d6abb3fd8) )
ROM_END

ROM_START( btimem )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ab05a1.12b",   0xb000, 0x1000, CRC(0a98b230) SHA1(aeee4f6f0aaa27575b80261d03c5453cc6ebd646) )
	ROM_LOAD( "ab04.9b",      0xc000, 0x1000, CRC(797e5f75) SHA1(35ea5fa4b8f3494adf7774b3946ed2540ac826ff) )
	ROM_LOAD( "ab06.13b",     0xd000, 0x1000, CRC(c77f3f64) SHA1(f283087fad0a102fe92be7ce80ed18e64dc93b67) )
	ROM_LOAD( "ab05.10b",     0xe000, 0x1000, CRC(b0d3640f) SHA1(6ba28971714ece6f1c04fa2dbf1f9f216ded7cfa) )
	ROM_LOAD( "ab07.15b",     0xf000, 0x1000, CRC(a142f862) SHA1(39d7ef172d18874885f1b1542e885cc4287dc344) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ab14.12h",     0xf000, 0x1000, CRC(f55e5211) SHA1(27940026d0c6212d1138d2fd88880df697218627) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ab12.7k",      0x0000, 0x1000, CRC(6c79f79f) SHA1(338009199b5889621693833d88c35abb8e9e38a2) )    /* charset #1 */
	ROM_LOAD( "ab13.9k",      0x1000, 0x1000, CRC(ac01042f) SHA1(e64b6381a9298eaf74e79fa5f1ea8e9596c58a49) )
	ROM_LOAD( "ab10.10k",     0x2000, 0x1000, CRC(854a872a) SHA1(3d2ecfd54a5a9d68b53cf4b4ee1f2daa6aef2123) )
	ROM_LOAD( "ab11.12k",     0x3000, 0x1000, CRC(d4848014) SHA1(0a55b091cd4e7f317c35defe13d5051b26042eee) )
	ROM_LOAD( "ab8.13k",      0x4000, 0x1000, CRC(70b35bbe) SHA1(ee8d70d6792ac4b8fe3de90c665457fedb94a7ba) )
	ROM_LOAD( "ab9.15k",      0x5000, 0x1000, CRC(8dec15e6) SHA1(b72633de6268ce16742bba4dcba835df860d6c2f) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ab00.1b",      0x0000, 0x0800, CRC(c7a14485) SHA1(6a0a8e6b7860859f22daa33634e34fbf91387659) )    /* charset #2 */
	ROM_LOAD( "ab01.3b",      0x0800, 0x0800, CRC(25b49078) SHA1(4abdcbd4f3362c3e4463a1274731289f1a72d2e6) )
	ROM_LOAD( "ab02.4b",      0x1000, 0x0800, CRC(b8ef56c3) SHA1(4a03bf011dc1fb2902f42587b1174b880cf06df1) )

	ROM_REGION( 0x0800, REGION_GFX3, 0 )	/* background tilemaps */
	ROM_LOAD( "ab03.6b",      0x0000, 0x0800, CRC(d26bc1f3) SHA1(737af6e264183a1f151f277a07cf250d6abb3fd8) )
ROM_END

ROM_START( cookrace )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	/* code is in the range 0500-3fff, encrypted */
	ROM_LOAD( "1f.1",         0x0000, 0x2000, CRC(68759d32) SHA1(2112a6f17b871aefdb39739e47d4a9f368a2eb3c) )
	ROM_LOAD( "2f.2",         0x2000, 0x2000, CRC(be7d72d1) SHA1(232d108098cb490e7c828aa4524ad09d3866ae18) )
	ROM_LOAD( "2k",           0xffe0, 0x0020, CRC(e2553b3d) SHA1(0a38929cdb3f37c6e4bacc5c3f94c049b4352858) )	/* reset/interrupt vectors */

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "6f.6",         0xf000, 0x1000, CRC(6b8e0272) SHA1(372a891b7b357aea0297ba9bcae752c3c9d8c1be) ) /* starts at 0000, not f000; 0000-01ff is RAM */

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "m8.7",         0x0000, 0x2000, CRC(a1a0d5a6) SHA1(e9583320e9c303407abfe02988b95403e5209c52) )  /* charset #1 */
	ROM_LOAD( "m7.8",         0x2000, 0x2000, CRC(1104f497) SHA1(60abd05c2549fe014660c169011480beb191f36d) )
	ROM_LOAD( "m6.9",         0x4000, 0x2000, CRC(d0d94477) SHA1(74ca9134a52cabe5769d714855b38a49632b9e40) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "2f.3",         0x0000, 0x0800, CRC(28609a75) SHA1(ab5d02bc0a771227db820a79b16aa662fb2140cf) )  /* garbage?? */
	ROM_CONTINUE(             0x0000, 0x0800 )              /* charset #2 */
	ROM_LOAD( "4f.4",         0x0800, 0x0800, CRC(7742e771) SHA1(c938c5714273bd4f2a1beb23d781ecbe7b023e6d) )  /* garbage?? */
	ROM_CONTINUE(             0x0800, 0x0800 )
	ROM_LOAD( "5f.5",         0x1000, 0x0800, CRC(611c686f) SHA1(e2c45061597d3d1a855a625a906b5a17a87deb2c) )  /* garbage?? */
	ROM_CONTINUE(             0x1000, 0x0800 )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "f9.clr",       0x0000, 0x0020, CRC(c2348c1d) SHA1(a7cc4b499b6c89c5966711f8bb922026c2978e1a) )	/* palette */
	ROM_LOAD( "b7",           0x0020, 0x0020, CRC(e4268fa6) SHA1(93f74e633c3a19755e78e0e2883109cd8ccde9a8) )	/* unknown */
ROM_END

/* There is a flyer with a screen shot for Lock'n'Chase at:
   http://www.gamearchive.com/flyers/video/taito/locknchase_f.jpg  */

ROM_START( lnc )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "s3-3d",        0xc000, 0x1000, CRC(1ab4f2c2) SHA1(c5890b768172cd2e3912b84db5f71546969ad7e2) )
	ROM_LOAD( "s2-3c",        0xd000, 0x1000, CRC(5e46b789) SHA1(00b2510e07eb565cb373db798dd537191b0b7cc8) )
	ROM_LOAD( "s1-3b",        0xe000, 0x1000, CRC(1308a32e) SHA1(da64fe7b76f5ac8ac35460e6c789ab1e986c78ef) )
	ROM_LOAD( "s0-3a",        0xf000, 0x1000, CRC(beb4b1fc) SHA1(166a96b5757946231f3619844366218065412935) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "sa-1h",        0xf000, 0x1000, CRC(379387ec) SHA1(29d37f04c64ed53a2573962dfa9c0623b89e0045) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "s4-11l",       0x0000, 0x1000, CRC(a2162a9e) SHA1(2729cef805c8e863af540424faa1aca82d3525e2) )
	ROM_LOAD( "s5-11m",       0x1000, 0x1000, CRC(12f1c2db) SHA1(004e25a53ffa197e1238dfa53c530f128cf40516) )
	ROM_LOAD( "s6-13l",       0x2000, 0x1000, CRC(d21e2a57) SHA1(0462cd3a5be87da97ed1bd8b79f8822cd5a33cf1) )
	ROM_LOAD( "s7-13m",       0x3000, 0x1000, CRC(c4f247cd) SHA1(2c86bf479169981daf0378eb0b3e1a600937aaf2) )
	ROM_LOAD( "s8-15l",       0x4000, 0x1000, CRC(672a92d0) SHA1(1bc89f6a76873504aa0fcfa0c6a43e8546edde27) )
	ROM_LOAD( "s9-15m",       0x5000, 0x1000, CRC(87c8ee9a) SHA1(158019b18bc3e5104bebeb241c077a706bf72ff2) )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "sc-5m",        0x0000, 0x0020, CRC(2a976ebe) SHA1(f3c1b0d98f431f9cd0d5fa009fafa1115aabe6e5) )	/* palette */
	ROM_LOAD( "sb-4c",        0x0020, 0x0020, CRC(a29b4204) SHA1(7f15cae5c4aaa29638fb45029782dafd2b3d1484) )	/* RAS/CAS logic - not used */
ROM_END

ROM_START( wtennis )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "tx",           0xc000, 0x0800, CRC(fd343474) SHA1(1e1fd3f20ce1c7533767344f924029c8c62139a1) )
	ROM_LOAD( "t4",           0xd000, 0x1000, CRC(e465d82c) SHA1(c357dcf17539150425574985afa559db2e6ab834) )
	ROM_LOAD( "t3",           0xe000, 0x1000, CRC(8f090eab) SHA1(baeef8ee05010bf44cf8865a22911f3d458df1b0) )
	ROM_LOAD( "t2",           0xf000, 0x1000, CRC(d2f9dd30) SHA1(1faa088806e8627b5e561d8b99054d295045dcfb) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "t1",           0xf000, 0x1000, CRC(40737ea7) SHA1(27e8474028385574035d3982f9c576bb9bb3facd) ) /* starts at 0000, not f000; 0000-01ff is RAM */

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "t7",           0x0000, 0x1000, CRC(aa935169) SHA1(965f41a9fcf35ac7c899e79acd0a85ab588d5831) )
	ROM_LOAD( "t10",          0x1000, 0x1000, CRC(746be927) SHA1(a3361384437ac7c494fde92953c5aa5e3c104644) )
	ROM_LOAD( "t6",           0x2000, 0x1000, CRC(4fb8565d) SHA1(6de865e41dcba45190af0753baebf5ab66e4eeb4) )
	ROM_LOAD( "t9",           0x3000, 0x1000, CRC(4893286d) SHA1(f2c330286272b8d334b887bc4dd9608158249fc3) )
	ROM_LOAD( "t5",           0x4000, 0x1000, CRC(ea1efa5d) SHA1(dd8ef1991d74778e6844a669e6de649e1130ec79) )
	ROM_LOAD( "t8",           0x5000, 0x1000, CRC(542ace7b) SHA1(b1423d39302ad7d98c9223d8b1d6d062b7676dd9) )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "mb7051.m5",    0x0000, 0x0020, CRC(f051cb28) SHA1(6aebccd38ba7887caff248c8acddb8e14526f1e7) )	/* palette */
	ROM_LOAD( "sb-4c",        0x0020, 0x0020, CRC(a29b4204) SHA1(7f15cae5c4aaa29638fb45029782dafd2b3d1484) )	/* RAS/CAS logic - not used */
ROM_END

ROM_START( mmonkey )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "mmonkey.e4",   0xc000, 0x1000, CRC(8d31bf6a) SHA1(77b44d8e2b4db148727e7bfc5162c7e9e9cfc662) )
	ROM_LOAD( "mmonkey.d4",   0xd000, 0x1000, CRC(e54f584a) SHA1(a03fef09f6a0bb6802b33b28c45548efb85cda5c) )
	ROM_LOAD( "mmonkey.b4",   0xe000, 0x1000, CRC(399a161e) SHA1(0eb3c5031a7d8c7b14019e215b18dac24a9e70dd) )
	ROM_LOAD( "mmonkey.a4",   0xf000, 0x1000, CRC(f7d3d1e3) SHA1(ff650a833e5e8975fe5b4a644ce6c35de5e04740) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "mmonkey.h1",   0xf000, 0x1000, CRC(5bcb2e81) SHA1(60fb8fd83c83b278e3aaf96f0b6dbefbc1eef0f7) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "mmonkey.l11",  0x0000, 0x1000, CRC(b6aa8566) SHA1(bc90d4cfa9a221477d1989fea532621ce3e76439) )
	ROM_LOAD( "mmonkey.m11",  0x1000, 0x1000, CRC(6cc4d0c4) SHA1(f43450e97dd0c6d0a269c06e4c4253d0814590e9) )
	ROM_LOAD( "mmonkey.l13",  0x2000, 0x1000, CRC(2a343b7e) SHA1(1dba32a83db933096b9a9fbcfd8e0290aba76483) )
	ROM_LOAD( "mmonkey.m13",  0x3000, 0x1000, CRC(0230b50d) SHA1(d62b5d1be35c8bf29483fb616cd7e3949a422e76) )
	ROM_LOAD( "mmonkey.l14",  0x4000, 0x1000, CRC(922bb3e1) SHA1(72d2017e80bea7700a3a61a06882839ecffcabe8) )
	ROM_LOAD( "mmonkey.m14",  0x5000, 0x1000, CRC(f943e28c) SHA1(6ff536a21f34cbb958f6d0f84791102938966ff3) )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "mmi6331.m5",   0x0000, 0x0020, CRC(55e28b32) SHA1(b73f85224738252dc8dbb38a54250dcfe1fc3ae3) )	/* palette */
	ROM_LOAD( "sb-4c",        0x0020, 0x0020, CRC(a29b4204) SHA1(7f15cae5c4aaa29638fb45029782dafd2b3d1484) )	/* RAS/CAS logic - not used */
ROM_END

ROM_START( brubber )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	/* a000-bfff space for the service ROM */
	ROM_LOAD( "brubber.12c",  0xc000, 0x2000, CRC(b5279c70) SHA1(5fb1c50040dc4e9444aed440e2c3cf4c79b72311) )
	ROM_LOAD( "brubber.12d",  0xe000, 0x2000, CRC(b2ce51f5) SHA1(5e38ea24bcafef1faba023def96532abd6f97d38) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "bnj6c.bin",    0xf000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bnj4e.bin",    0x0000, 0x2000, CRC(b864d082) SHA1(cacf71fa6c0f7121d077381a0ff6222f534295ab) )
	ROM_LOAD( "bnj4f.bin",    0x2000, 0x2000, CRC(6c31d77a) SHA1(5e52554f594f569527af4768d244cc40a7b4460a) )
	ROM_LOAD( "bnj4h.bin",    0x4000, 0x2000, CRC(5824e6fb) SHA1(e98f0eb476b8f033f5cc70a6e503afc4e651fd45) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "bnj10e.bin",   0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "bnj10f.bin",   0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )
ROM_END

ROM_START( bnj )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "bnj12b.bin",   0xa000, 0x2000, CRC(ba3e3801) SHA1(56284076d938c33c1492a07281b936681eb09808) )
	ROM_LOAD( "bnj12c.bin",   0xc000, 0x2000, CRC(fb3a2cdd) SHA1(4a964389cc8035b9264d4cb133eb6d3826e74b95) )
	ROM_LOAD( "bnj12d.bin",   0xe000, 0x2000, CRC(b88bc99e) SHA1(08a4ddea4037f9e14d0d9f4262a1746b0a3a140c) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "bnj6c.bin",    0xf000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "bnj4e.bin",    0x0000, 0x2000, CRC(b864d082) SHA1(cacf71fa6c0f7121d077381a0ff6222f534295ab) )
	ROM_LOAD( "bnj4f.bin",    0x2000, 0x2000, CRC(6c31d77a) SHA1(5e52554f594f569527af4768d244cc40a7b4460a) )
	ROM_LOAD( "bnj4h.bin",    0x4000, 0x2000, CRC(5824e6fb) SHA1(e98f0eb476b8f033f5cc70a6e503afc4e651fd45) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "bnj10e.bin",   0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "bnj10f.bin",   0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )
ROM_END

ROM_START( caractn )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	/* a000-bfff space for the service ROM */
	ROM_LOAD( "c7.12c",  0xc000, 0x2000, CRC(b5279c70) SHA1(5fb1c50040dc4e9444aed440e2c3cf4c79b72311) )
	ROM_LOAD( "c6.12d",  0xe000, 0x2000, CRC(1d6957c4) SHA1(bd30f00187e56eef9adcc167dd752a3bb616454c) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "c5.6c",   0xf000, 0x1000, CRC(8c02f662) SHA1(1279d564e65fd3ccac25b1f9fbb40d910de2b544) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "c0.4e",   0x0000, 0x2000, CRC(bf3ea732) SHA1(d98970b2dda8c3435506656909e5e3aa70d45652) )
	ROM_LOAD( "c1.4f",   0x2000, 0x2000, CRC(9789f639) SHA1(77a4d494698718c052fa1967242a0e4fa263b6ad) )
	ROM_LOAD( "c2.4h",   0x4000, 0x2000, CRC(51dcc111) SHA1(9753d682ba2f4fb4d3b14783ac35ad214bf788b5) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "c3.10e",  0x0000, 0x1000, CRC(f4e9eb49) SHA1(b356512d2ebd4e2005e76496b434e5ecebadb251) )
	ROM_LOAD( "c4.10f",  0x1000, 0x1000, CRC(a9ffacb4) SHA1(49d5f9c0b695f474197fbb761bacc065b6b5808a) )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "tbp18s030.11a",   0x0000, 0x020, CRC(318d25b9) SHA1(9a82619c94f5911d01ddf6b85f7e30cdc6f1d0a3) )	/* palette */
	ROM_LOAD( "tbp18s030.cpu",   0x0020, 0x020, CRC(6b0c2942) SHA1(7d25acc753923b265792fc78f8fc70175c0e0ec2) )	/* RAS/CAS logic - not used */

	ROM_REGION( 0x0140, REGION_PLDS, 0 )
	ROM_LOAD( "pal10l8.10k",   0x0000, 0x002c, CRC(dc72a65f) SHA1(d61c149d4df93a2074debf7c5e46557c6b06d10d) )
	ROM_LOAD( "pal16r4a.2d",   0x0030, 0x0104, CRC(fd1f3aa2) SHA1(67f1e74fcfc0e2301204ed58b8c6e35d4866a344) )
ROM_END

ROM_START( zoar )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "zoar15",       0xd000, 0x1000, BAD_DUMP CRC(1f0cfdb7) SHA1(ce7e871f17c52b6eaf99cfb721e702e4f0e6bb25) )
	ROM_LOAD( "zoar16",       0xe000, 0x1000, CRC(7685999c) SHA1(fabe38d71e797ae0b04b5d3aba228b4c85d96185) )
	ROM_LOAD( "zoar17",       0xf000, 0x1000, CRC(619ea867) SHA1(0a3735384f03a1052d54ab799b5e37038d8ece2a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "zoar09",       0xf000, 0x1000, CRC(18d96ff1) SHA1(671d934a451e0b042450ea86d24c3751a39b38f8) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "zoar00",       0x0000, 0x1000, CRC(fd2dcb64) SHA1(1a49a6ec6ffd354d872b1af83d55ec96e8215b2b) )
	ROM_LOAD( "zoar01",       0x1000, 0x1000, CRC(74d3ca48) SHA1(2c75ea246f86a057467deb35ef6a6e72f667dd84) )
	ROM_LOAD( "zoar03",       0x2000, 0x1000, CRC(77b7df14) SHA1(a1cbc214fc849b7e3417b1156d1e4440ab67f631) )
	ROM_LOAD( "zoar04",       0x3000, 0x1000, CRC(9be786de) SHA1(480733a1438dffa4b0fac6f76bf84a0deec5d1fa) )
	ROM_LOAD( "zoar06",       0x4000, 0x1000, CRC(07638c71) SHA1(1a7fc49657ac7ac0033bd60c86663bd615079230) )
	ROM_LOAD( "zoar07",       0x5000, 0x1000, CRC(f4710f25) SHA1(08b4cc4252f83a689cded38d9a5a50f55ee6beee) )

	ROM_REGION( 0x1800, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "zoar10",       0x0000, 0x0800, CRC(aa8bcab8) SHA1(81f1a9fd754fd6f8030ff6b5aa80c7670be9d02e) )
	ROM_LOAD( "zoar11",       0x0800, 0x0800, CRC(dcdad357) SHA1(d1569e1d38f14f5f457547e24df4f80f726c6157) )
	ROM_LOAD( "zoar12",       0x1000, 0x0800, CRC(ed317e40) SHA1(db70889af5f233ca71acf734abfbdb74b6a393c0) )

	ROM_REGION( 0x1000, REGION_GFX3, 0 )	/* background tilemaps */
	ROM_LOAD( "zoar13",       0x0000, 0x1000, CRC(8fefa960) SHA1(614026aa71703dd3898e470f45730e5c6934b31b) )

	ROM_REGION( 0x3000, REGION_GFX4, ROMREGION_DISPOSE )
	ROM_LOAD( "zoar02",       0x0000, 0x1000, CRC(d8c3c122) SHA1(841006cc84622e851df462a64696b64bb8cb62a1) )
	ROM_LOAD( "zoar05",       0x1000, 0x1000, CRC(05dc6b09) SHA1(197c720544a090e12980513b441a2b9cf04e212f) )
	ROM_LOAD( "zoar08",       0x2000, 0x1000, CRC(9a148551) SHA1(db92dd7552c6f76a062910f37a3fe3524fdffd38) )

	ROM_REGION( 0x0040, REGION_PROMS, 0 )
	ROM_LOAD( "z20-1l",       0x0000, 0x0020, CRC(a63f0a07) SHA1(16532d3ac0536ad4b712005fd722ee8c14d02e9b) )
	ROM_LOAD( "z21-1l",       0x0020, 0x0020, CRC(5e1e5788) SHA1(56068b209cc7c734bbcbb9858f40faa6474c8095) )
ROM_END

ROM_START( disco )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "disco.w5",     0xa000, 0x1000, CRC(b2c87b78) SHA1(4095f0052ff0ac35ecd2ec1c1e99d21283d336e1) )
	ROM_LOAD( "disco.w4",     0xb000, 0x1000, CRC(ad7040ee) SHA1(287a4ff06edda4c66e2351e49a94212728aacb4e) )
	ROM_LOAD( "disco.w3",     0xc000, 0x1000, CRC(12fb4f08) SHA1(d6095f20d8676df89b1459134b5521ac311ddded) )
	ROM_LOAD( "disco.w2",     0xd000, 0x1000, CRC(73f6fb2f) SHA1(7b75b825d9bf7e512e054762500f79c18a276e1f) )
	ROM_LOAD( "disco.w1",     0xe000, 0x1000, CRC(ee7b536b) SHA1(b2de5da15cee1d80391eafd0a08361803f859c89) )
	ROM_LOAD( "disco.w0",     0xf000, 0x1000, CRC(7c26e76b) SHA1(952e91c4acc18d01b0e2c3efd764da8768f583da) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "disco.w6",     0xf000, 0x1000, CRC(d81e781e) SHA1(bde510bfed06a13bd56bf7ddbf220e7cf82f79b6) )

	/* no gfx1 */

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "disco.clr",    0x0000, 0x0020, CRC(a393f913) SHA1(42dce159283427064b3f5ce3a6e2189744ecd943) )
ROM_END

ROM_START( discof )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "w5-f",     0xa000, 0x1000, CRC(9d53c71c) SHA1(53c410cfa4fbbfd08e1c3cf7aeba1c9627171a71) )
	ROM_LOAD( "w4-f",     0xb000, 0x1000, CRC(c1f8d747) SHA1(33f5fe73d1851ef4da670075d1aec1550e0417ce) )
	ROM_LOAD( "w3-f",     0xc000, 0x1000, CRC(9aadd252) SHA1(c6da7ef46333d525e676c59f03ccc908108b41ba) )
	ROM_LOAD( "w2-f",     0xd000, 0x1000, CRC(f131a5bb) SHA1(84b7dea112dce12e5cb235a13f6dc4edcfb18c06) )
	ROM_LOAD( "w1-f",     0xe000, 0x1000, CRC(c8ec57c5) SHA1(904a9ed0a7f1230c611bf473b9bc52e63eb56dbe) )
	ROM_LOAD( "w0-f",     0xf000, 0x1000, CRC(b3787a92) SHA1(7f40621dc739c1108a5df43142ab04709a380219) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "disco.w6",     0xf000, 0x1000, CRC(d81e781e) SHA1(bde510bfed06a13bd56bf7ddbf220e7cf82f79b6) )

	/* no gfx1 */

	ROM_REGION( 0x0020, REGION_PROMS, 0 )
	ROM_LOAD( "disco.clr",    0x0000, 0x0020, CRC(a393f913) SHA1(42dce159283427064b3f5ce3a6e2189744ecd943) )
ROM_END

ROM_START( sdtennis )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "ao_08.12b",  0xa000, 0x2000, CRC(6193724c) SHA1(97239c5aa8c8cd1812fba1b15be4d9a48eb0651a) )
	ROM_LOAD( "ao_07.12c",  0xc000, 0x2000, CRC(064888db) SHA1(f7bb728ab3408bb553191d9e131a441db1b39666) )
	ROM_LOAD( "ao_06.12d",  0xe000, 0x2000, CRC(413c984c) SHA1(1431df4db52d621ba39fd47dbd49da103b5c0bcf) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 )
	ROM_LOAD( "ao_05.6c",    0xf000, 0x1000, CRC(46833e38) SHA1(420831149a566199d6a3c74ef3df0687b4ddcbe4) )

	ROM_REGION( 0x6000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "ao_00.4e",    0x0000, 0x2000, CRC(f4e0cbd6) SHA1(a2ede0ce4a26957a5d3b62872a42b8979f5000aa) )
	ROM_LOAD( "ao_01.4f",    0x2000, 0x2000, CRC(f99029da) SHA1(45bc56ff6284d02371d5e1cd5239be665f9e56c7) )
	ROM_LOAD( "ao_02.4h",    0x4000, 0x2000, CRC(c3077555) SHA1(addfc67735dc22dfed9c4c4ec8d9dcf590c76737) )

	ROM_REGION( 0x2000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "ao_03.10e",   0x0000, 0x1000, CRC(1977db9b) SHA1(d175974967fdeb608df668089fa2a14b2d1609e6) )
	ROM_LOAD( "ao_04.10f",   0x1000, 0x1000, CRC(921952af) SHA1(4e9248f3493a5f4651278f27c11f507571242317) )
ROM_END

static void decrypt_C10707_cpu(int cpu, int region)
{
	UINT8 *decrypt = auto_malloc(0x10000);
	UINT8 *rom = memory_region(region);
	offs_t addr;

	memory_set_decrypted_region(cpu, 0x0000, 0xffff, decrypt);

	/* Swap bits 5 & 6 for opcodes */
	for (addr = 0; addr < 0x10000; addr++)
		decrypt[addr] = swap_bits_5_6(rom[addr]);

	if (cpu == 0)
		decrypted = decrypt;
}

static READ8_HANDLER( wtennis_reset_hack_r )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	/* Otherwise the game goes into test mode and there is no way out that I
       can see.  I'm not sure how it can work, it probably somehow has to do
       with the tape system */

	RAM[0xfc30] = 0;

	return RAM[0xc15f];
}

static void init_rom1(void)
{
	UINT8 *rom = memory_region(REGION_CPU1);

	decrypted = auto_malloc(0x10000);
	memory_set_decrypted_region(0, 0x0000, 0xffff, decrypted);

	/* For now, just copy the RAM array over to ROM. Decryption will happen */
	/* at run time, since the CPU applies the decryption only if the previous */
	/* instruction did a memory write. */
	memcpy(decrypted,rom,0x10000);
}

static DRIVER_INIT( btime )
{
	init_rom1();
}

static DRIVER_INIT( zoar )
{
	UINT8 *rom = memory_region(REGION_CPU1);

	/* At location 0xD50A is what looks like an undocumented opcode. I tried
       implementing it given what opcode 0x23 should do, but it still didn't
       work in demo mode. So this could be another protection or a bad ROM read.
       I'm NOPing it out for now. */
	memset(&rom[0xd50a],0xea,8);

	init_rom1();
}

static DRIVER_INIT( lnc )
{
	decrypt_C10707_cpu(0, REGION_CPU1);
}

static DRIVER_INIT( cookrace )
{
	memcpy(&audio_rambase[0x200], memory_region(REGION_CPU2) + 0xf200, 0x200);
	decrypt_C10707_cpu(0, REGION_CPU1);
}

static DRIVER_INIT( wtennis )
{
	memcpy(&audio_rambase[0x200], memory_region(REGION_CPU2) + 0xf200, 0x200);
	memory_install_read8_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xc15f, 0xc15f, 0, 0, wtennis_reset_hack_r);
	decrypt_C10707_cpu(0, REGION_CPU1);
}

static DRIVER_INIT( sdtennis )
{
	decrypt_C10707_cpu(0, REGION_CPU1);
	decrypt_C10707_cpu(1, REGION_CPU2);
}


GAME( 1982, btime,    0,       btime,    btime,    btime,   ROT270, "Data East Corporation", "Burger Time (Data East set 1)", 0 )
GAME( 1982, btime2,   btime,   btime,    btime,    btime,   ROT270, "Data East Corporation", "Burger Time (Data East set 2)", 0 )
GAME( 1982, btimem,   btime,   btime,    btime,    btime,   ROT270, "Data East (Bally Midway license)", "Burger Time (Midway)", 0 )
GAME( 1982, cookrace, btime,   cookrace, cookrace, cookrace,ROT270, "bootleg", "Cook Race", 0 )
GAME( 1981, lnc,      0,       lnc,      lnc,      lnc,     ROT270, "Data East Corporation", "Lock'n'Chase", 0 )
GAME( 1982, wtennis,  0,       wtennis,  wtennis,  wtennis, ROT270, "bootleg", "World Tennis", 0 )
GAME( 1982, mmonkey,  0,       mmonkey,  mmonkey,  lnc,     ROT270, "Technos + Roller Tron", "Minky Monkey", 0 )
GAME( 1982, brubber,  0,       bnj,      bnj,      lnc,     ROT270, "Data East", "Burnin' Rubber", 0 )
GAME( 1982, bnj,      brubber, bnj,      bnj,      lnc,     ROT270, "Data East USA (Bally Midway license)", "Bump 'n' Jump", 0 )
GAME( 1982, caractn,  brubber, bnj,      bnj,      lnc,     ROT270, "bootleg", "Car Action", 0 )
GAME( 1982, zoar,     0,       zoar,     zoar,     zoar,    ROT270, "Data East USA", "Zoar", 0 )
GAME( 1982, disco,    0,       disco,    disco,    btime,   ROT270, "Data East", "Disco No.1", 0 )
GAME( 1982, discof,   disco,   disco,    disco,    btime,   ROT270, "Data East", "Disco No.1 (Rev.F)", 0 )
GAME( 1983, sdtennis, 0,       bnj,      sdtennis, sdtennis,ROT270, "Data East Corporation", "Super Doubles Tennis", 0 )
