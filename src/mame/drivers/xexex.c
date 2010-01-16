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

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "deprecat.h"
#include "video/konicdev.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/k054539.h"
#include "sound/2151intf.h"
#include "sound/flt_vol.h"
#include "includes/xexex.h"
#include "includes/konamipt.h"

#define XE_DEBUG      0
#define XE_SKIPIDLE   1
#define XE_DMADELAY   ATTOTIME_IN_USEC(256)

static const eeprom_interface eeprom_intf =
{
	7,				/* address bits */
	8,				/* data bits */
	"011000",		/*  read command */
	"011100",		/* write command */
	"0100100000000",/* erase command */
	"0100000000000",/* lock command */
	"0100110000000" /* unlock command */
};

#if 0 // (for reference; do not remove)

/* the interface with the 053247 is weird. The chip can address only 0x1000 bytes */
/* of RAM, but they put 0x8000 there. The CPU can access them all. Address lines */
/* A1, A5 and A6 don't go to the 053247. */
static READ16_HANDLER( K053247_scattered_word_r )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;

	if (offset & 0x0031)
		return state->spriteram[offset];
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		return k053247_word_r(state->k053246, offset, mem_mask);
	}
}

static WRITE16_HANDLER( K053247_scattered_word_w )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;

	if (offset & 0x0031)
		COMBINE_DATA(state->spriteram + offset);
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		k053247_word_w(state->k053246, offset, data, mem_mask);
	}
}

#endif


static void xexex_objdma( running_machine *machine, int limiter )
{
	xexex_state *state = (xexex_state *)machine->driver_data;
	int counter, num_inactive;
	UINT16 *src, *dst;

	counter = state->frame;
	state->frame = video_screen_get_frame_number(machine->primary_screen);
	if (limiter && counter == state->frame)
		return; // make sure we only do DMA transfer once per frame

	k053247_get_ram(state->k053246, &dst);
	counter = k053247_get_dy(state->k053246);
	src = state->spriteram;
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

static READ16_HANDLER( spriteram_mirror_r )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;
	return state->spriteram[offset];
}

static WRITE16_HANDLER( spriteram_mirror_w )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;
	COMBINE_DATA(state->spriteram + offset);
}

static READ16_HANDLER( xexex_waitskip_r )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;

	if (cpu_get_pc(space->cpu) == 0x1158)
	{
		cpu_spinuntil_trigger(space->cpu, state->resume_trigger);
		state->suspension_active = 1;
	}

	return state->workram[0x14/2];
}


static void parse_control2( running_machine *machine )
{
	xexex_state *state = (xexex_state *)machine->driver_data;

	/* bit 0  is data */
	/* bit 1  is cs (active low) */
	/* bit 2  is clock (active high) */
	/* bit 5  is enable irq 6 */
	/* bit 6  is enable irq 5 */
	/* bit 11 is watchdog */
	input_port_write(machine, "EEPROMOUT", state->cur_control2, 0xff);

	/* bit 8 = enable sprite ROM reading */
	k053246_set_objcha_line(state->k053246, (state->cur_control2 & 0x0100) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 9 = disable alpha channel on K054157 plane 0 (under investigation) */
	state->cur_alpha = !(state->cur_control2 & 0x200);
}

static READ16_HANDLER( control2_r )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;
	return state->cur_control2;
}

static WRITE16_HANDLER( control2_w )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;
	COMBINE_DATA(&state->cur_control2);
	parse_control2(space->machine);
}


static WRITE16_HANDLER( sound_cmd1_w )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;

	if(ACCESSING_BITS_0_7)
	{
		// anyone knows why 0x1a keeps lurking the sound queue in the world version???
		if (state->strip_0x1a)
			if (soundlatch2_r(space, 0) == 1 && data == 0x1a)
				return;

		soundlatch_w(space, 0, data & 0xff);
	}
}

static WRITE16_HANDLER( sound_cmd2_w )
{
	if (ACCESSING_BITS_0_7)
		soundlatch2_w(space, 0, data & 0xff);
}

static WRITE16_HANDLER( sound_irq_w )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;
	cpu_set_input_line(state->audiocpu, 0, HOLD_LINE);
}

static READ16_HANDLER( sound_status_r )
{
	return soundlatch3_r(space, 0);
}

static void reset_sound_region(running_machine *machine)
{
	xexex_state *state = (xexex_state *)machine->driver_data;
	memory_set_bank(machine, "bank2", state->cur_sound_region & 0x07);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	xexex_state *state = (xexex_state *)space->machine->driver_data;
	state->cur_sound_region = data & 7;
	reset_sound_region(space->machine);
}

static void ym_set_mixing(const device_config *device, double left, double right)
{
	xexex_state *state = (xexex_state *)device->machine->driver_data;
	flt_volume_set_volume(state->filter1l, (71.0 * left) / 55.0);
	flt_volume_set_volume(state->filter1r, (71.0 * right) / 55.0);
	flt_volume_set_volume(state->filter2l, (71.0 * left) / 55.0);
	flt_volume_set_volume(state->filter2r, (71.0 * right) / 55.0);
}

static TIMER_CALLBACK( dmaend_callback )
{
	xexex_state *state = (xexex_state *)machine->driver_data;

	if (state->cur_control2 & 0x0040)
	{
		// foul-proof (CPU0 could be deactivated while we wait)
		if (state->suspension_active)
		{
			state->suspension_active = 0;
			cpuexec_trigger(machine, state->resume_trigger);
		}

		// IRQ 5 is the "object DMA end interrupt" and shouldn't be triggered
		// if object data isn't ready for DMA within the frame.
		cpu_set_input_line(state->maincpu, 5, HOLD_LINE);
	}
}

static INTERRUPT_GEN( xexex_interrupt )
{
	xexex_state *state = (xexex_state *)device->machine->driver_data;

	if (state->suspension_active)
	{
		state->suspension_active = 0;
		cpuexec_trigger(device->machine, state->resume_trigger);
	}

	switch (cpu_getiloops(device))
	{
		case 0:
			// IRQ 6 is for test mode only
			if (state->cur_control2 & 0x0020)
				cpu_set_input_line(device, 6, HOLD_LINE);
		break;

		case 1:
			if (k053246_is_irq_enabled(state->k053246))
			{
				// OBJDMA starts at the beginning of V-blank
				xexex_objdma(device->machine, 0);

				// schedule DMA end interrupt
				timer_adjust_oneshot(state->dmadelay_timer, XE_DMADELAY, 0);
			}

			// IRQ 4 is the V-blank interrupt. It controls color, sound and
			// vital game logics that shouldn't be interfered by frame-drop.
			if (state->cur_control2 & 0x0800)
				cpu_set_input_line(device, 4, HOLD_LINE);
		break;
	}
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM AM_BASE_MEMBER(xexex_state, workram)			// work RAM

#if XE_SKIPIDLE
	AM_RANGE(0x080014, 0x080015) AM_READ(xexex_waitskip_r)				// helps sound CPU by giving back control as early as possible
#endif

	AM_RANGE(0x090000, 0x097fff) AM_RAM AM_BASE_MEMBER(xexex_state, spriteram)			// K053247 sprite RAM
	AM_RANGE(0x098000, 0x09ffff) AM_READWRITE(spriteram_mirror_r, spriteram_mirror_w)	// K053247 sprite RAM mirror read
	AM_RANGE(0x0c0000, 0x0c003f) AM_DEVWRITE("k056832", k056832_word_w)				// VACSET (K054157)
	AM_RANGE(0x0c2000, 0x0c2007) AM_DEVWRITE("k053246", k053246_word_w)				// OBJSET1
	AM_RANGE(0x0c4000, 0x0c4001) AM_DEVREAD("k053246", k053246_word_r)				// Passthrough to sprite roms
	AM_RANGE(0x0c6000, 0x0c7fff) AM_DEVREADWRITE("k053250", k053250_ram_r, k053250_ram_w)	// K053250 "road" RAM
	AM_RANGE(0x0c8000, 0x0c800f) AM_DEVREADWRITE("k053250", k053250_r, k053250_w)
	AM_RANGE(0x0ca000, 0x0ca01f) AM_DEVWRITE("k054338", k054338_word_w)				// CLTC
	AM_RANGE(0x0cc000, 0x0cc01f) AM_DEVWRITE("k053251", k053251_lsb_w)				// priority encoder
	AM_RANGE(0x0d0000, 0x0d001f) AM_DEVWRITE("k053252", k053252_word_w)				// CCU
	AM_RANGE(0x0d4000, 0x0d4001) AM_WRITE(sound_irq_w)
	AM_RANGE(0x0d600c, 0x0d600d) AM_WRITE(sound_cmd1_w)
	AM_RANGE(0x0d600e, 0x0d600f) AM_WRITE(sound_cmd2_w)
	AM_RANGE(0x0d6014, 0x0d6015) AM_READ(sound_status_r)
	AM_RANGE(0x0d6000, 0x0d601f) AM_RAM									// sound regs fall through
	AM_RANGE(0x0d8000, 0x0d8007) AM_DEVWRITE("k056832", k056832_b_word_w)				// VSCCS regs
	AM_RANGE(0x0da000, 0x0da001) AM_READ_PORT("P1")
	AM_RANGE(0x0da002, 0x0da003) AM_READ_PORT("P2")
	AM_RANGE(0x0dc000, 0x0dc001) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x0dc002, 0x0dc003) AM_READ_PORT("EEPROM")
	AM_RANGE(0x0de000, 0x0de001) AM_READWRITE(control2_r, control2_w)
	AM_RANGE(0x100000, 0x17ffff) AM_ROM
	AM_RANGE(0x180000, 0x181fff) AM_DEVREADWRITE("k056832", k056832_ram_word_r, k056832_ram_word_w)
	AM_RANGE(0x182000, 0x183fff) AM_DEVREADWRITE("k056832", k056832_ram_word_r, k056832_ram_word_w)
	AM_RANGE(0x190000, 0x191fff) AM_DEVREAD("k056832", k056832_rom_word_r)		// Passthrough to tile roms
	AM_RANGE(0x1a0000, 0x1a1fff) AM_DEVREAD("k053250", k053250_rom_r)
	AM_RANGE(0x1b0000, 0x1b1fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)

#if XE_DEBUG
	AM_RANGE(0x0c0000, 0x0c003f) AM_DEVREAD("k056832", k056832_word_r)
	AM_RANGE(0x0c2000, 0x0c2007) AM_DEVREAD("k053246", k053246_reg_word_r)
	AM_RANGE(0x0ca000, 0x0ca01f) AM_DEVREAD("k054338", k054338_word_r)
	AM_RANGE(0x0cc000, 0x0cc01f) AM_DEVREAD("k053251", k053251_lsb_r)
	AM_RANGE(0x0d0000, 0x0d001f) AM_DEVREAD("k053252", k053252_word_r)
	AM_RANGE(0x0d8000, 0x0d8007) AM_DEVREAD("k056832", k056832_b_word_r)
#endif

ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe22f) AM_DEVREADWRITE("k054539", k054539_r, k054539_w)
	AM_RANGE(0xec00, 0xec01) AM_DEVREADWRITE("ymsnd", ym2151_r, ym2151_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(soundlatch3_w)
	AM_RANGE(0xf002, 0xf002) AM_READ(soundlatch_r)
	AM_RANGE(0xf003, 0xf003) AM_READ(soundlatch2_r)
	AM_RANGE(0xf800, 0xf800) AM_WRITE(sound_bankswitch_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( xexex )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	KONAMI16_LSB(1, IPT_UNKNOWN, IPT_START1 )

	PORT_START("P2")
	KONAMI16_LSB(2, IPT_UNKNOWN, IPT_START2 )

	PORT_START("EEPROM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
INPUT_PORTS_END



static const k054539_interface k054539_config =
{
	NULL,
	ym_set_mixing
};

static const k054338_interface xexex_k054338_intf =
{
	"screen",
	0,
	"none"
};


static const k053250_interface xexex_k053250_intf =
{
	"screen",
	"gfx3",
	-5, -16
};

static const k056832_interface xexex_k056832_intf =
{
	"gfx1", 0,
	K056832_BPP_4,
	1, 0,
	KONAMI_ROM_DEINTERLEAVE_2,
	xexex_tile_callback, "none"
};

static const k053247_interface xexex_k053246_intf =
{
	"screen",
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	-48, 32,
	KONAMI_ROM_DEINTERLEAVE_4,
	xexex_sprite_callback
};

static STATE_POSTLOAD( xexex_postload )
{
	parse_control2(machine);
	reset_sound_region(machine);
}

static MACHINE_START( xexex )
{
	xexex_state *state = (xexex_state *)machine->driver_data;
	UINT8 *ROM = memory_region(machine, "audiocpu");

	memory_configure_bank(machine, "bank2", 0, 8, &ROM[0x10000], 0x4000);
	memory_set_bank(machine, "bank2", 0);

	state->maincpu = devtag_get_device(machine, "maincpu");
	state->audiocpu = devtag_get_device(machine, "audiocpu");
	state->k053246 = devtag_get_device(machine, "k053246");
	state->k053250 = devtag_get_device(machine, "k053250");
	state->k053251 = devtag_get_device(machine, "k053251");
	state->k053252 = devtag_get_device(machine, "k053252");
	state->k056832 = devtag_get_device(machine, "k056832");
	state->k054338 = devtag_get_device(machine, "k054338");
	state->k054539 = devtag_get_device(machine, "k054539");
	state->filter1l = devtag_get_device(machine, "filter1l");
	state->filter1r = devtag_get_device(machine, "filter1r");
	state->filter2l = devtag_get_device(machine, "filter2l");
	state->filter2r = devtag_get_device(machine, "filter2r");

	state_save_register_global(machine, state->cur_alpha);
	state_save_register_global(machine, state->sprite_colorbase);
	state_save_register_global_array(machine, state->layer_colorbase);
	state_save_register_global_array(machine, state->layerpri);

	state_save_register_global(machine, state->suspension_active);
	state_save_register_global(machine, state->frame);

	state_save_register_global(machine, state->cur_control2);
	state_save_register_global(machine, state->cur_sound_region);
	state_save_register_postload(machine, xexex_postload, NULL);

	state->dmadelay_timer = timer_alloc(machine, dmaend_callback, NULL);
}

static MACHINE_RESET( xexex )
{
	xexex_state *state = (xexex_state *)machine->driver_data;
	int i;

	for (i = 0; i < 4; i++)
	{
		state->layerpri[i] = 0;
		state->layer_colorbase[i] = 0;
	}

	state->sprite_colorbase = 0;

	state->cur_control2 = 0;
	state->cur_sound_region = 0;
	state->suspension_active = 0;
	state->resume_trigger = 1000;
	state->frame = -1;
	k054539_init_flags(devtag_get_device(machine, "k054539"), K054539_REVERSE_STEREO);
}

static MACHINE_DRIVER_START( xexex )

	/* driver data */
	MDRV_DRIVER_DATA(xexex_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)	// 16MHz (32MHz xtal)
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT_HACK(xexex_interrupt,2)

	// 8MHz (PCB shows one 32MHz/18.432MHz xtal, reference: www.system16.com)
	// more likely 32MHz since 18.432MHz yields 4.608MHz(too slow) or 9.216MHz(too fast) with integer divisors
	MDRV_CPU_ADD("audiocpu", Z80, 8000000)
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_QUANTUM_TIME(HZ(1920))

	MDRV_MACHINE_START(xexex)
	MDRV_MACHINE_RESET(xexex)

	MDRV_EEPROM_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS | VIDEO_UPDATE_BEFORE_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
//  MDRV_SCREEN_REFRESH_RATE(8000000/512/288)
	MDRV_SCREEN_RAW_PARAMS(8000000, 384+33+40+55, 0, 383, 256+12+6+14, 0, 255)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(40, 40+384-1, 0, 0+256-1)

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(xexex)
	MDRV_VIDEO_UPDATE(xexex)

	MDRV_K056832_ADD("k056832", xexex_k056832_intf)
	MDRV_K053246_ADD("k053246", xexex_k053246_intf)
	MDRV_K053250_ADD("k053250", xexex_k053250_intf)
	MDRV_K053251_ADD("k053251")
	MDRV_K053252_ADD("k053252")
	MDRV_K054338_ADD("k054338", xexex_k054338_intf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ymsnd", YM2151, 4000000)
	MDRV_SOUND_ROUTE(0, "filter1l", 0.50)
	MDRV_SOUND_ROUTE(0, "filter1r", 0.50)
	MDRV_SOUND_ROUTE(1, "filter2l", 0.50)
	MDRV_SOUND_ROUTE(1, "filter2r", 0.50)

	MDRV_SOUND_ADD("k054539", K054539, 48000)
	MDRV_SOUND_CONFIG(k054539_config)
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "lspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 1.0)

	MDRV_SOUND_ADD("filter1l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("filter1r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MDRV_SOUND_ADD("filter2l", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MDRV_SOUND_ADD("filter2r", FILTER_VOLUME, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_DRIVER_END


ROM_START( xexex ) /* Europe, Version AA */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "067eaa01.16d", 0x000000, 0x40000, CRC(3ebcb066) SHA1(83a20433d9fdcc8b8d7133991f9a8164dddb61f3) )
	ROM_LOAD16_BYTE( "067eaa02.16f", 0x000001, 0x40000, CRC(36ea7a48) SHA1(34f8046d7ecf5ea66c59c5bc0d7627942c28fd3b) )
	ROM_LOAD16_BYTE( "067_b03.rom",  0x100000, 0x40000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067_b04.rom",  0x100001, 0x40000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x030000, "audiocpu", 0 )
	ROM_LOAD( "067eaa05.4e", 0x000000, 0x020000, CRC(0e33d6ec) SHA1(4dd68cb78c779e2d035e43fec35a7672ed1c259b) )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "067_b14.rom", 0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD( "067_b13.rom", 0x100000, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "067_b12.rom", 0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD( "067_b11.rom", 0x100000, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD( "067_b10.rom", 0x200000, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD( "067_b09.rom", 0x300000, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )
ROM_END

ROM_START( xexexa ) /* Asia, Version AA */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "067aaa01.16d", 0x000000, 0x40000, CRC(cf557144) SHA1(4ce587580d953b88864652dd66485d49ca719ec5) )
	ROM_LOAD16_BYTE( "067aaa02.16f", 0x000001, 0x40000, CRC(b7b98d52) SHA1(ca2343bf37f779699b6782772e559ea5662c1742) )
	ROM_LOAD16_BYTE( "067_b03.rom",  0x100000, 0x40000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067_b04.rom",  0x100001, 0x40000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x030000, "audiocpu", 0 )
	ROM_LOAD( "067eaa05.4e", 0x000000, 0x020000, CRC(0e33d6ec) SHA1(4dd68cb78c779e2d035e43fec35a7672ed1c259b) )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "067_b14.rom", 0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD( "067_b13.rom", 0x100000, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "067_b12.rom", 0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD( "067_b11.rom", 0x100000, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD( "067_b10.rom", 0x200000, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD( "067_b09.rom", 0x300000, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )
ROM_END

ROM_START( xexexj ) /* Japan, Version AA */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "067jaa01.16d", 0x000000, 0x40000, CRC(06e99784) SHA1(d53fe3724608992a6938c36aa2719dc545d6b89e) )
	ROM_LOAD16_BYTE( "067jaa02.16f", 0x000001, 0x40000, CRC(30ae5bc4) SHA1(60491e31eef64a9206d1372afa32d83c6c0968b3) )
	ROM_LOAD16_BYTE( "067_b03.rom",  0x100000, 0x40000, CRC(97833086) SHA1(a564f7b1b52c774d78a59f4418c7ecccaf94ad41) )
	ROM_LOAD16_BYTE( "067_b04.rom",  0x100001, 0x40000, CRC(26ec5dc8) SHA1(9da62683bfa8f16607cbea2d59a1446ec8588c5b) )

	ROM_REGION( 0x030000, "audiocpu", 0 )
	ROM_LOAD( "067jaa05.4e", 0x000000, 0x020000, CRC(2f4dd0a8) SHA1(bfa76c9c968f1beba648a2911510e3d666a8fe3a) )
	ROM_RELOAD(              0x010000, 0x020000 )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "067_b14.rom", 0x000000, 0x100000, CRC(02a44bfa) SHA1(ad95df4dbf8842820ef20f54407870afb6d0e4a3) )
	ROM_LOAD( "067_b13.rom", 0x100000, 0x100000, CRC(633c8eb5) SHA1(a11f78003a1dffe2d8814d368155059719263082) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD( "067_b12.rom", 0x000000, 0x100000, CRC(08d611b0) SHA1(9cac60131e0411f173acd8ef3f206e5e58a7e5d2) )
	ROM_LOAD( "067_b11.rom", 0x100000, 0x100000, CRC(a26f7507) SHA1(6bf717cb9fcad59a2eafda967f14120b9ebbc8c5) )
	ROM_LOAD( "067_b10.rom", 0x200000, 0x100000, CRC(ee31db8d) SHA1(c41874fb8b401ea9cdd327ee6239b5925418cf7b) )
	ROM_LOAD( "067_b09.rom", 0x300000, 0x100000, CRC(88f072ef) SHA1(7ecc04dbcc29b715117e970cc96e11137a21b83a) )

	ROM_REGION( 0x100000, "gfx3", 0 ) // NOTE: region must be 2xROM size for unpacking
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )
ROM_END


static DRIVER_INIT( xexex )
{
	xexex_state *state = (xexex_state *)machine->driver_data;

	state->strip_0x1a = 0;

	if (!strcmp(machine->gamedrv->name, "xexex"))
	{
		// Invulnerability
//      *(UINT16 *)(memory_region(machine, "maincpu") + 0x648d4) = 0x4a79;
//      *(UINT16 *)(memory_region(machine, "maincpu") + 0x00008) = 0x5500;
		state->strip_0x1a = 1;
	}
}

GAME( 1991, xexex,  0,     xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver EAA)", GAME_SUPPORTS_SAVE )
GAME( 1991, xexexa, xexex, xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver AAA)", GAME_SUPPORTS_SAVE )
GAME( 1991, xexexj, xexex, xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver JAA)", GAME_SUPPORTS_SAVE )
