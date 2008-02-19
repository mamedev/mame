#define XE_DEBUG 0
#define XE_SKIPIDLE 1
#define XE_DMADELAY ATTOTIME_IN_USEC(256)

/***************************************************************************

    Xexex  (c) 1991 Konami - GX067


Change Log
----------

(ATXXXX03)

Hooked up missing memory handler, emulated object DMA, revised IRQ,
rewrote the K053250(LVC) effect generator, ported tilemaps to use the
K056832 emulation(the K054157 is a complete subset of the K056832),
corrected a few K054539 PCM chip misbehaviors, etc.


The following bugs appear to be fixed:

General:

- game doesn't slow down like the arcade
    IRQ 5 is the "OBJDMA end interrupt" and shouldn't be triggered
    if DMA didn't complete within the frame.

    * game speed may not be 100% correct but close to that on the
    Gamest video especially in stage 6. Xexex is 384x256 which suggests
    an 8Mhz horizontal dotclock and DMA delay can range up to 32.0us(clear)
    + 256.0us(transfer). Increase XE_DMADELAY if emulation runs faster
    than the original or use cheat to overclock CPU 0 if you prefer faster
    gameplay.

- sprite lag, dithering, flicking (DMA)
- line effects go out of sync (K053250 also does DMA)
- inconsistent reverb (maths bug)
- lasers don't change color (IRQ masking)
- xexex057gre_1 (delayed sfx, missing speech, Xexexj only: random 1-up note)
- xexex057gre_2 (reversed stereo)
- xexex065gre (coin up problems, IRQ order)

- L1: xexex067gre (tilemap boundary), misaligned bosses (swapXY)
- L2: xexex061gre (K054157 offset)
- L4: half the foreground missing (LVC no-wraparound)
- L5: poly-face boss missing (coordinate masking)
- L6: sticky galaxies (LVC scroll bug)
- L7: misaligned ship patches (swapXY)


Unresolved Issues:

- random 1-up notes still pop up in the world version (filtered temporarily)
- mono/stereo softdip has no effect (xexex057gre_3, external mixing?)
- K053250 shows a one-frame glitch at stage 1 boss (DMA timing?)
- stage 3 intro missing alpha effect (known K054338 deficiency)
- the stage 4 boss(tentacles) sometimes appears darker (palette update timing?)
- the furthest layer in stage 5 shakes when scrolling up or down (needs verification)
- Elaine's end-game graphics has wrong masking effect (known non-zoomed pdrawgfx issue)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "video/konamiic.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/k054539.h"
#include "sound/2151intf.h"
#include "sound/flt_vol.h"

VIDEO_START( xexex );
VIDEO_UPDATE( xexex );
void xexex_set_alpha(int on);

static MACHINE_START( xexex );
static MACHINE_RESET( xexex );

static UINT16 *xexex_workram;
static UINT16 cur_control2;
static int init_eeprom_count;
static INT32 cur_sound_region, xexex_strip0x1a;
static int suspension_active, resume_trigger;
static emu_timer *dmadelay_timer;


static const struct EEPROM_interface eeprom_interface =
{
	7,				/* address bits */
	8,				/* data bits */
	"011000",		/*  read command */
	"011100",		/* write command */
	"0100100000000",/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

static NVRAM_HANDLER( xexex )
{
	if (read_or_write)
		EEPROM_save(file);
	else
	{
		EEPROM_init(&eeprom_interface);

		if (file)
		{
			init_eeprom_count = 0;
			EEPROM_load(file);
		}
		else
			init_eeprom_count = 10;
	}
}


#if 0 // (for reference; do not remove)

/* the interface with the 053247 is weird. The chip can address only 0x1000 bytes */
/* of RAM, but they put 0x8000 there. The CPU can access them all. Address lines */
/* A1, A5 and A6 don't go to the 053247. */
static READ16_HANDLER( K053247_scattered_word_r )
{
	if (offset & 0x0031)
		return spriteram16[offset];
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		return K053247_word_r(offset,mem_mask);
	}
}

static WRITE16_HANDLER( K053247_scattered_word_w )
{
	if (offset & 0x0031)
		COMBINE_DATA(spriteram16+offset);
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		K053247_word_w(offset,data,mem_mask);
	}
}

#endif


static void xexex_objdma(int limiter)
{
	static int frame = -1;

	int counter, num_inactive;
	UINT16 *src, *dst;

	counter = frame;
	frame = cpu_getcurrentframe();
	if (limiter && counter == frame) return; // make sure we only do DMA transfer once per frame

	K053247_export_config(&dst, 0, 0, 0, &counter);
	src = spriteram16;
	num_inactive = counter = 256;

	do
	{
		if (*src & 0x8000)
		{
			dst[0] = src[0x0];  dst[1] = src[0x2];
			dst[2] = src[0x4];  dst[3] = src[0x6];
			dst[4] = src[0x8];  dst[5] = src[0xa];
			dst[6] = src[0xc];  dst[7] = src[0xe];
			dst += 8;
			num_inactive--;
		}
		src += 0x40;
	}
	while (--counter);

	if (num_inactive) do { *dst = 0; dst += 8; } while (--num_inactive);
}

static READ16_HANDLER( spriteram16_mirror_r )
{
	return(spriteram16[offset]);
}

static WRITE16_HANDLER( spriteram16_mirror_w )
{
	COMBINE_DATA(spriteram16+offset);
}

static READ16_HANDLER( xexex_waitskip_r )
{
	if (activecpu_get_pc() == 0x1158)
	{
		cpu_spinuntil_trigger(resume_trigger);
		suspension_active = 1;
	}

	return(xexex_workram[0x14/2]);
}


static READ16_HANDLER( control1_r )
{
	int res;

	/* bit 0 is EEPROM data */
	/* bit 1 is EEPROM ready */
	/* bit 3 is service button */
	res = EEPROM_read_bit() | input_port_1_r(0);

	if (init_eeprom_count)
	{
		init_eeprom_count--;
		res &= 0xf7;
	}

	return res;
}

static void parse_control2(void)
{
	/* bit 0  is data */
	/* bit 1  is cs (active low) */
	/* bit 2  is clock (active high) */
	/* bit 5  is enable irq 6 */
	/* bit 6  is enable irq 5 */
	/* bit 11 is watchdog */

	EEPROM_write_bit(cur_control2 & 0x01);
	EEPROM_set_cs_line((cur_control2 & 0x02) ? CLEAR_LINE : ASSERT_LINE);
	EEPROM_set_clock_line((cur_control2 & 0x04) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 8 = enable sprite ROM reading */
	K053246_set_OBJCHA_line((cur_control2 & 0x0100) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 9 = disable alpha channel on K054157 plane 0 (under investigation) */
	xexex_set_alpha(!(cur_control2 & 0x200));
}

static READ16_HANDLER( control2_r )
{
	return cur_control2;
}

static WRITE16_HANDLER( control2_w )
{
	COMBINE_DATA(&cur_control2);
	parse_control2();
}


static WRITE16_HANDLER( sound_cmd1_w )
{
	if(ACCESSING_LSB)
	{
		// anyone knows why 0x1a keeps lurking the sound queue in the world version???
		if (xexex_strip0x1a)
			if (soundlatch2_r(0)==1 && data==0x1a) return;

		soundlatch_w(0, data & 0xff);
	}
}

static WRITE16_HANDLER( sound_cmd2_w )
{
	if (ACCESSING_LSB)
	{
		soundlatch2_w(0, data & 0xff);
	}
}

static WRITE16_HANDLER( sound_irq_w )
{
	cpunum_set_input_line(Machine, 1, 0, HOLD_LINE);
}

static READ16_HANDLER( sound_status_r )
{
	return soundlatch3_r(0);
}

static void reset_sound_region(void)
{
	memory_set_bankptr(2, memory_region(REGION_CPU2) + 0x10000 + cur_sound_region*0x4000);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	cur_sound_region = data & 7;
	reset_sound_region();
}

static void ym_set_mixing(double left, double right)
{
	flt_volume_set_volume(0, (71.0*left)/55.0);
	flt_volume_set_volume(1, (71.0*right)/55.0);
	flt_volume_set_volume(2, (71.0*left)/55.0);
	flt_volume_set_volume(3, (71.0*right)/55.0);
}

static TIMER_CALLBACK( dmaend_callback )
{
	if (cur_control2 & 0x0040)
	{
		// foul-proof (CPU0 could be deactivated while we wait)
		if (suspension_active) { suspension_active = 0; cpu_trigger(machine, resume_trigger); }

		// IRQ 5 is the "object DMA end interrupt" and shouldn't be triggered
		// if object data isn't ready for DMA within the frame.
		cpunum_set_input_line(machine, 0, 5, HOLD_LINE);
	}
}

static INTERRUPT_GEN( xexex_interrupt )
{
	if (suspension_active) { suspension_active = 0; cpu_trigger(machine, resume_trigger); }

	switch (cpu_getiloops())
	{
		case 0:
			// IRQ 6 is for test mode only
			if (cur_control2 & 0x0020)
				cpunum_set_input_line(machine, 0, 6, HOLD_LINE);
		break;

		case 1:
			if (K053246_is_IRQ_enabled())
			{
				// OBJDMA starts at the beginning of V-blank
				xexex_objdma(0);

				// schedule DMA end interrupt
				timer_adjust_oneshot(dmadelay_timer, XE_DMADELAY, 0);
			}

			// IRQ 4 is the V-blank interrupt. It controls color, sound and
			// vital game logics that shouldn't be interfered by frame-drop.
			if (cur_control2 & 0x0800)
				cpunum_set_input_line(machine, 0, 4, HOLD_LINE);
		break;
	}
}


static ADDRESS_MAP_START( readmem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_READ(MRA16_ROM)
#if XE_SKIPIDLE
	AM_RANGE(0x080014, 0x080015) AM_READ(xexex_waitskip_r)		// helps sound CPU by giving back control as early as possible
#endif
	AM_RANGE(0x080000, 0x08ffff) AM_READ(MRA16_RAM)
	AM_RANGE(0x090000, 0x097fff) AM_READ(MRA16_RAM)				// K053247 sprite RAM
	AM_RANGE(0x098000, 0x09ffff) AM_READ(spriteram16_mirror_r)	// K053247 sprite RAM mirror read
	AM_RANGE(0x0c4000, 0x0c4001) AM_READ(K053246_word_r)			// Passthrough to sprite roms
	AM_RANGE(0x0c6000, 0x0c7fff) AM_READ(K053250_0_ram_r)		// K053250 "road" RAM
	AM_RANGE(0x0c8000, 0x0c800f) AM_READ(K053250_0_r)
	AM_RANGE(0x0d6014, 0x0d6015) AM_READ(sound_status_r)
	AM_RANGE(0x0d6000, 0x0d601f) AM_READ(MRA16_RAM)
	AM_RANGE(0x0da000, 0x0da001) AM_READ(input_port_2_word_r)
	AM_RANGE(0x0da002, 0x0da003) AM_READ(input_port_3_word_r)
	AM_RANGE(0x0dc000, 0x0dc001) AM_READ(input_port_0_word_r)
	AM_RANGE(0x0dc002, 0x0dc003) AM_READ(control1_r)
	AM_RANGE(0x0de000, 0x0de001) AM_READ(control2_r)
	AM_RANGE(0x100000, 0x17ffff) AM_READ(MRA16_ROM)
	AM_RANGE(0x180000, 0x181fff) AM_READ(K056832_ram_word_r)
	AM_RANGE(0x182000, 0x183fff) AM_READ(K056832_ram_word_r)
	AM_RANGE(0x190000, 0x191fff) AM_READ(K056832_rom_word_r)		// Passthrough to tile roms
	AM_RANGE(0x1a0000, 0x1a1fff) AM_READ(K053250_0_rom_r)
	AM_RANGE(0x1b0000, 0x1b1fff) AM_READ(MRA16_RAM)
#if XE_DEBUG
	AM_RANGE(0x0c0000, 0x0c003f) AM_READ(K056832_word_r)
	AM_RANGE(0x0c2000, 0x0c2007) AM_READ(K053246_reg_word_r)
	AM_RANGE(0x0ca000, 0x0ca01f) AM_READ(K054338_word_r)
	AM_RANGE(0x0cc000, 0x0cc01f) AM_READ(K053251_lsb_r)
	AM_RANGE(0x0d0000, 0x0d001f) AM_READ(K053252_word_r)
	AM_RANGE(0x0d8000, 0x0d8007) AM_READ(K056832_b_word_r)
#endif
ADDRESS_MAP_END

static ADDRESS_MAP_START( writemem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_WRITE(MWA16_ROM)					// main ROM
	AM_RANGE(0x080000, 0x08ffff) AM_WRITE(MWA16_RAM) AM_BASE(&xexex_workram)	// work RAM
	AM_RANGE(0x090000, 0x097fff) AM_WRITE(MWA16_RAM) AM_BASE(&spriteram16)	// K053247 sprite RAM
	AM_RANGE(0x098000, 0x09ffff) AM_WRITE(spriteram16_mirror_w)		// K053247 sprite RAM mirror write
	AM_RANGE(0x0c0000, 0x0c003f) AM_WRITE(K056832_word_w)				// VACSET (K054157)
	AM_RANGE(0x0c2000, 0x0c2007) AM_WRITE(K053246_word_w)				// OBJSET1
	AM_RANGE(0x0c6000, 0x0c7fff) AM_WRITE(K053250_0_ram_w)			// K053250 "road" RAM
	AM_RANGE(0x0c8000, 0x0c800f) AM_WRITE(K053250_0_w)				// background effects generator
	AM_RANGE(0x0ca000, 0x0ca01f) AM_WRITE(K054338_word_w)				// CLTC
	AM_RANGE(0x0cc000, 0x0cc01f) AM_WRITE(K053251_lsb_w)				// priority encoder
	AM_RANGE(0x0d0000, 0x0d001f) AM_WRITE(K053252_word_w)				// CCU
	AM_RANGE(0x0d4000, 0x0d4001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x0d600c, 0x0d600d) AM_WRITE(sound_cmd1_w)
	AM_RANGE(0x0d600e, 0x0d600f) AM_WRITE(sound_cmd2_w)
	AM_RANGE(0x0d6000, 0x0d601f) AM_WRITE(MWA16_RAM)					// sound regs fall through
	AM_RANGE(0x0d8000, 0x0d8007) AM_WRITE(K056832_b_word_w)			// VSCCS regs
	AM_RANGE(0x0de000, 0x0de001) AM_WRITE(control2_w)
	AM_RANGE(0x100000, 0x17ffff) AM_WRITE(MWA16_ROM)
	AM_RANGE(0x180000, 0x181fff) AM_WRITE(K056832_ram_word_w) 		// tilemap RAM
	AM_RANGE(0x182000, 0x183fff) AM_WRITE(K056832_ram_word_w) 		// tilemap RAM mirror
	AM_RANGE(0x190000, 0x191fff) AM_WRITE(MWA16_ROM)					// tile ROM
	AM_RANGE(0x1b0000, 0x1b1fff) AM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE(&paletteram16)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(MRA8_ROM)
	AM_RANGE(0x8000, 0xbfff) AM_READ(MRA8_BANK2)
	AM_RANGE(0xc000, 0xdfff) AM_READ(MRA8_RAM)
	AM_RANGE(0xe000, 0xe22f) AM_READ(K054539_0_r)
	AM_RANGE(0xec01, 0xec01) AM_READ(YM2151_status_port_0_r)
	AM_RANGE(0xf002, 0xf002) AM_READ(soundlatch_r)
	AM_RANGE(0xf003, 0xf003) AM_READ(soundlatch2_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_WRITE(MWA8_ROM)
	AM_RANGE(0xc000, 0xdfff) AM_WRITE(MWA8_RAM)
	AM_RANGE(0xe000, 0xe22f) AM_WRITE(K054539_0_w)
	AM_RANGE(0xec00, 0xec00) AM_WRITE(YM2151_register_port_0_w)
	AM_RANGE(0xec01, 0xec01) AM_WRITE(YM2151_data_port_0_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch3_w)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( xexex )
	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* EEPROM data */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END



static const struct K054539interface k054539_interface =
{
	REGION_SOUND1,
	ym_set_mixing
};

static MACHINE_DRIVER_START( xexex )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, 16000000)	// 16MHz (32MHz xtal)
	MDRV_CPU_PROGRAM_MAP(readmem,writemem)
	MDRV_CPU_VBLANK_INT(xexex_interrupt,2)

	// 8MHz (PCB shows one 32MHz/18.432MHz xtal, reference: www.system16.com)
	// more likely 32MHz since 18.432MHz yields 4.608MHz(too slow) or 9.216MHz(too fast) with integer divisors
	MDRV_CPU_ADD(Z80, 8000000)

	/* audio CPU */
	MDRV_CPU_PROGRAM_MAP(sound_readmem,sound_writemem)

	MDRV_INTERLEAVE(32)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

	MDRV_MACHINE_START(xexex)
	MDRV_MACHINE_RESET(xexex)
	MDRV_NVRAM_HANDLER(xexex)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER | VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS | VIDEO_UPDATE_BEFORE_VBLANK)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(40, 40+384-1, 0, 0+256-1)

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(xexex)
	MDRV_VIDEO_UPDATE(xexex)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, 4000000)
	MDRV_SOUND_ROUTE(0, "filter1l", 0.50)
	MDRV_SOUND_ROUTE(0, "filter1r", 0.50)
	MDRV_SOUND_ROUTE(1, "filter2l", 0.50)
	MDRV_SOUND_ROUTE(1, "filter2r", 0.50)

	MDRV_SOUND_ADD(K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(0, "right", 1.0)
	MDRV_SOUND_ROUTE(1, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

	MDRV_SOUND_ADD_TAG("filter1l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ADD_TAG("filter1r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
	MDRV_SOUND_ADD_TAG("filter2l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 1.0)
	MDRV_SOUND_ADD_TAG("filter2r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 1.0)
MACHINE_DRIVER_END


ROM_START( xexex ) /* Europe, Version AA */
	ROM_REGION( 0x180000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "067eaa01.16d", 0x000000, 0x40000, CRC(3ebcb066) SHA1(83a20433d9fdcc8b8d7133991f9a8164dddb61f3) )
	ROM_LOAD16_BYTE( "067eaa02.16f", 0x000001, 0x40000, CRC(36ea7a48) SHA1(34f8046d7ecf5ea66c59c5bc0d7627942c28fd3b) )
	ROM_LOAD16_BYTE( "067_b03.rom",  0x100000, 0x40000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067_b04.rom",  0x100001, 0x40000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x030000, REGION_CPU2, 0 )
	ROM_LOAD( "067eaa05.4e", 0x000000, 0x020000, CRC(0e33d6ec) SHA1(4dd68cb78c779e2d035e43fec35a7672ed1c259b) )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "067_b14.rom", 0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD( "067_b13.rom", 0x100000, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 )
	ROM_LOAD( "067_b12.rom", 0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD( "067_b11.rom", 0x100000, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD( "067_b10.rom", 0x200000, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD( "067_b09.rom", 0x300000, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x100000, REGION_GFX3, 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, REGION_SOUND1, 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )
ROM_END

ROM_START( xexexa ) /* Asia, Version AA */
	ROM_REGION( 0x180000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "067aaa01.16d", 0x000000, 0x40000, CRC(cf557144) SHA1(4ce587580d953b88864652dd66485d49ca719ec5) )
	ROM_LOAD16_BYTE( "067aaa02.16f", 0x000001, 0x40000, CRC(b7b98d52) SHA1(ca2343bf37f779699b6782772e559ea5662c1742) )
	ROM_LOAD16_BYTE( "067_b03.rom",  0x100000, 0x40000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067_b04.rom",  0x100001, 0x40000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x030000, REGION_CPU2, 0 )
	ROM_LOAD( "067eaa05.4e", 0x000000, 0x020000, CRC(0e33d6ec) SHA1(4dd68cb78c779e2d035e43fec35a7672ed1c259b) )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "067_b14.rom", 0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD( "067_b13.rom", 0x100000, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 )
	ROM_LOAD( "067_b12.rom", 0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD( "067_b11.rom", 0x100000, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD( "067_b10.rom", 0x200000, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD( "067_b09.rom", 0x300000, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x100000, REGION_GFX3, 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, REGION_SOUND1, 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )
ROM_END

ROM_START( xexexj ) /* Japan, Version AA */
	ROM_REGION( 0x180000, REGION_CPU1, 0 )
	ROM_LOAD16_BYTE( "067jaa01.16d", 0x000000, 0x40000, CRC(06e99784) SHA1(d53fe3724608992a6938c36aa2719dc545d6b89e) )
	ROM_LOAD16_BYTE( "067jaa02.16f", 0x000001, 0x40000, CRC(30ae5bc4) SHA1(60491e31eef64a9206d1372afa32d83c6c0968b3) )
	ROM_LOAD16_BYTE( "067_b03.rom",  0x100000, 0x40000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067_b04.rom",  0x100001, 0x40000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x030000, REGION_CPU2, 0 )
	ROM_LOAD( "067jaa05.4e", 0x000000, 0x020000, CRC(2f4dd0a8) SHA1(bfa76c9c968f1beba648a2911510e3d666a8fe3a) )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "067_b14.rom", 0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD( "067_b13.rom", 0x100000, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, REGION_GFX2, 0 )
	ROM_LOAD( "067_b12.rom", 0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD( "067_b11.rom", 0x100000, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD( "067_b10.rom", 0x200000, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD( "067_b09.rom", 0x300000, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x100000, REGION_GFX3, 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, REGION_SOUND1, 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )
ROM_END

static MACHINE_RESET( xexex )
{
	cur_sound_region = 0;
	suspension_active = 0;
	K054539_init_flags(0, K054539_REVERSE_STEREO);
}

static MACHINE_START( xexex )
{
	state_save_register_global(cur_control2);
	state_save_register_func_postload(parse_control2);
	state_save_register_global(cur_sound_region);
	state_save_register_func_postload(reset_sound_region);

	resume_trigger = 1000;

	dmadelay_timer = timer_alloc(dmaend_callback, NULL);
}


static DRIVER_INIT( xexex )
{
	if (!strcmp(machine->gamedrv->name, "xexex"))
	{
		// Invulnerability
//      *(UINT16 *)(memory_region(REGION_CPU1) + 0x648d4) = 0x4a79;
//      *(UINT16 *)(memory_region(REGION_CPU1) + 0x00008) = 0x5500;
		xexex_strip0x1a = 1;
	}

	konami_rom_deinterleave_2(REGION_GFX1);
	konami_rom_deinterleave_4(REGION_GFX2);
	K053250_unpack_pixels(REGION_GFX3);
}

GAME( 1991, xexex,  0,     xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver EAA)", 0 )
GAME( 1991, xexexa, xexex, xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver AAA)", 0 )
GAME( 1991, xexexj, xexex, xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver JAA)", 0 )
