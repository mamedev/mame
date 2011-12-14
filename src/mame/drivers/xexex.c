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
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "machine/k053252.h"
#include "sound/k054539.h"
#include "sound/2151intf.h"
#include "sound/flt_vol.h"
#include "video/konicdev.h"
#include "includes/xexex.h"
#include "includes/konamipt.h"

#define XE_DEBUG      0
#define XE_SKIPIDLE   1
#define XE_DMADELAY   attotime::from_usec(256)

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
	xexex_state *state = space->machine().driver_data<xexex_state>();

	if (offset & 0x0031)
		return state->m_spriteram[offset];
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		return k053247_word_r(state->m_k053246, offset, mem_mask);
	}
}

static WRITE16_HANDLER( K053247_scattered_word_w )
{
	xexex_state *state = space->machine().driver_data<xexex_state>();

	if (offset & 0x0031)
		COMBINE_DATA(state->m_spriteram + offset);
	else
	{
		offset = ((offset & 0x000e) >> 1) | ((offset & 0x3fc0) >> 3);
		k053247_word_w(state->m_k053246, offset, data, mem_mask);
	}
}

#endif


static void xexex_objdma( running_machine &machine, int limiter )
{
	xexex_state *state = machine.driver_data<xexex_state>();
	int counter, num_inactive;
	UINT16 *src, *dst;

	counter = state->m_frame;
	state->m_frame = machine.primary_screen->frame_number();
	if (limiter && counter == state->m_frame)
		return; // make sure we only do DMA transfer once per frame

	k053247_get_ram(state->m_k053246, &dst);
	counter = k053247_get_dy(state->m_k053246);
	src = state->m_spriteram;
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
	xexex_state *state = space->machine().driver_data<xexex_state>();
	return state->m_spriteram[offset];
}

static WRITE16_HANDLER( spriteram_mirror_w )
{
	xexex_state *state = space->machine().driver_data<xexex_state>();
	COMBINE_DATA(state->m_spriteram + offset);
}

static READ16_HANDLER( xexex_waitskip_r )
{
	xexex_state *state = space->machine().driver_data<xexex_state>();

	if (cpu_get_pc(&space->device()) == 0x1158)
	{
		device_spin_until_trigger(&space->device(), state->m_resume_trigger);
		state->m_suspension_active = 1;
	}

	return state->m_workram[0x14/2];
}


static void parse_control2( running_machine &machine )
{
	xexex_state *state = machine.driver_data<xexex_state>();

	/* bit 0  is data */
	/* bit 1  is cs (active low) */
	/* bit 2  is clock (active high) */
	/* bit 5  is enable irq 6 */
	/* bit 6  is enable irq 5 */
	/* bit 11 is watchdog */
	input_port_write(machine, "EEPROMOUT", state->m_cur_control2, 0xff);

	/* bit 8 = enable sprite ROM reading */
	k053246_set_objcha_line(state->m_k053246, (state->m_cur_control2 & 0x0100) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 9 = disable alpha channel on K054157 plane 0 (under investigation) */
	state->m_cur_alpha = !(state->m_cur_control2 & 0x200);
}

static READ16_HANDLER( control2_r )
{
	xexex_state *state = space->machine().driver_data<xexex_state>();
	return state->m_cur_control2;
}

static WRITE16_HANDLER( control2_w )
{
	xexex_state *state = space->machine().driver_data<xexex_state>();
	COMBINE_DATA(&state->m_cur_control2);
	parse_control2(space->machine());
}


static WRITE16_HANDLER( sound_cmd1_w )
{
	xexex_state *state = space->machine().driver_data<xexex_state>();

	if(ACCESSING_BITS_0_7)
	{
		// anyone knows why 0x1a keeps lurking the sound queue in the world version???
		if (state->m_strip_0x1a)
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
	xexex_state *state = space->machine().driver_data<xexex_state>();
	device_set_input_line(state->m_audiocpu, 0, HOLD_LINE);
}

static READ16_HANDLER( sound_status_r )
{
	return soundlatch3_r(space, 0);
}

static void reset_sound_region(running_machine &machine)
{
	xexex_state *state = machine.driver_data<xexex_state>();
	memory_set_bank(machine, "bank2", state->m_cur_sound_region & 0x07);
}

static WRITE8_HANDLER( sound_bankswitch_w )
{
	xexex_state *state = space->machine().driver_data<xexex_state>();
	state->m_cur_sound_region = data & 7;
	reset_sound_region(space->machine());
}

static void ym_set_mixing(device_t *device, double left, double right)
{
	xexex_state *state = device->machine().driver_data<xexex_state>();
	flt_volume_set_volume(state->m_filter1l, (71.0 * left) / 55.0);
	flt_volume_set_volume(state->m_filter1r, (71.0 * right) / 55.0);
	flt_volume_set_volume(state->m_filter2l, (71.0 * left) / 55.0);
	flt_volume_set_volume(state->m_filter2r, (71.0 * right) / 55.0);
}

static TIMER_CALLBACK( dmaend_callback )
{
	xexex_state *state = machine.driver_data<xexex_state>();

	if (state->m_cur_control2 & 0x0040)
	{
		// foul-proof (CPU0 could be deactivated while we wait)
		if (state->m_suspension_active)
		{
			state->m_suspension_active = 0;
			machine.scheduler().trigger(state->m_resume_trigger);
		}

		// IRQ 5 is the "object DMA end interrupt" and shouldn't be triggered
		// if object data isn't ready for DMA within the frame.
		device_set_input_line(state->m_maincpu, 5, HOLD_LINE);
	}
}

static TIMER_DEVICE_CALLBACK( xexex_interrupt )
{
	xexex_state *state = timer.machine().driver_data<xexex_state>();
	int scanline = param;

	if (state->m_suspension_active)
	{
		state->m_suspension_active = 0;
		timer.machine().scheduler().trigger(state->m_resume_trigger);
	}

	if(scanline == 0)
	{
		// IRQ 6 is for test mode only
			if (state->m_cur_control2 & 0x0020)
				device_set_input_line(state->m_maincpu, 6, HOLD_LINE);
	}

	/* TODO: vblank is at 256! (enable CCU then have fun in fixing offsetted layers) */
	if(scanline == 128)
	{
		if (k053246_is_irq_enabled(state->m_k053246))
		{
			// OBJDMA starts at the beginning of V-blank
			xexex_objdma(timer.machine(), 0);

			// schedule DMA end interrupt
			state->m_dmadelay_timer->adjust(XE_DMADELAY);
		}

		// IRQ 4 is the V-blank interrupt. It controls color, sound and
		// vital game logics that shouldn't be interfered by frame-drop.
		if (state->m_cur_control2 & 0x0800)
			device_set_input_line(state->m_maincpu, 4, HOLD_LINE);
	}
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x08ffff) AM_RAM AM_BASE_MEMBER(xexex_state, m_workram)			// work RAM

#if XE_SKIPIDLE
	AM_RANGE(0x080014, 0x080015) AM_READ(xexex_waitskip_r)				// helps sound CPU by giving back control as early as possible
#endif

	AM_RANGE(0x090000, 0x097fff) AM_RAM AM_BASE_MEMBER(xexex_state, m_spriteram)			// K053247 sprite RAM
	AM_RANGE(0x098000, 0x09ffff) AM_READWRITE(spriteram_mirror_r, spriteram_mirror_w)	// K053247 sprite RAM mirror read
	AM_RANGE(0x0c0000, 0x0c003f) AM_DEVWRITE("k056832", k056832_word_w)				// VACSET (K054157)
	AM_RANGE(0x0c2000, 0x0c2007) AM_DEVWRITE("k053246", k053246_word_w)				// OBJSET1
	AM_RANGE(0x0c4000, 0x0c4001) AM_DEVREAD("k053246", k053246_word_r)				// Passthrough to sprite roms
	AM_RANGE(0x0c6000, 0x0c7fff) AM_DEVREADWRITE_MODERN("k053250", k053250_t, ram_r, ram_w)	// K053250 "road" RAM
	AM_RANGE(0x0c8000, 0x0c800f) AM_DEVREADWRITE_MODERN("k053250", k053250_t, reg_r, reg_w)
	AM_RANGE(0x0ca000, 0x0ca01f) AM_DEVWRITE("k054338", k054338_word_w)				// CLTC
	AM_RANGE(0x0cc000, 0x0cc01f) AM_DEVWRITE("k053251", k053251_lsb_w)				// priority encoder
//  AM_RANGE(0x0d0000, 0x0d001f) AM_DEVREADWRITE8("k053252", k053252_r,k053252_w,0x00ff)                // CCU
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
	AM_RANGE(0x1a0000, 0x1a1fff) AM_DEVREAD_MODERN("k053250", k053250_t, rom_r)
	AM_RANGE(0x1b0000, 0x1b1fff) AM_RAM_WRITE(paletteram16_xrgb_word_be_w) AM_BASE_GENERIC(paletteram)

#if XE_DEBUG
	AM_RANGE(0x0c0000, 0x0c003f) AM_DEVREAD("k056832", k056832_word_r)
	AM_RANGE(0x0c2000, 0x0c2007) AM_DEVREAD("k053246", k053246_reg_word_r)
	AM_RANGE(0x0ca000, 0x0ca01f) AM_DEVREAD("k054338", k054338_word_r)
	AM_RANGE(0x0cc000, 0x0cc01f) AM_DEVREAD("k053251", k053251_lsb_r)
	AM_RANGE(0x0d8000, 0x0d8007) AM_DEVREAD("k056832", k056832_b_word_r)
#endif

ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8 )
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SPECIAL )	/* EEPROM ready (always 1) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_cs_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_device, set_clock_line)
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

static const k053252_interface xexex_k053252_intf =
{
	"screen",
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	0, 0
};

static void xexex_postload(running_machine &machine)
{
	parse_control2(machine);
	reset_sound_region(machine);
}

static MACHINE_START( xexex )
{
	xexex_state *state = machine.driver_data<xexex_state>();
	UINT8 *ROM = machine.region("audiocpu")->base();

	memory_configure_bank(machine, "bank2", 0, 8, &ROM[0x10000], 0x4000);
	memory_set_bank(machine, "bank2", 0);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_k053246 = machine.device("k053246");
	state->m_k053250 = machine.device<k053250_t>("k053250");
	state->m_k053251 = machine.device("k053251");
	state->m_k053252 = machine.device("k053252");
	state->m_k056832 = machine.device("k056832");
	state->m_k054338 = machine.device("k054338");
	state->m_k054539 = machine.device("k054539");
	state->m_filter1l = machine.device("filter1l");
	state->m_filter1r = machine.device("filter1r");
	state->m_filter2l = machine.device("filter2l");
	state->m_filter2r = machine.device("filter2r");

	state->save_item(NAME(state->m_cur_alpha));
	state->save_item(NAME(state->m_sprite_colorbase));
	state->save_item(NAME(state->m_layer_colorbase));
	state->save_item(NAME(state->m_layerpri));

	state->save_item(NAME(state->m_suspension_active));
	state->save_item(NAME(state->m_frame));

	state->save_item(NAME(state->m_cur_control2));
	state->save_item(NAME(state->m_cur_sound_region));
	machine.save().register_postload(save_prepost_delegate(FUNC(xexex_postload), &machine));

	state->m_dmadelay_timer = machine.scheduler().timer_alloc(FUNC(dmaend_callback));
}

static MACHINE_RESET( xexex )
{
	xexex_state *state = machine.driver_data<xexex_state>();
	int i;

	for (i = 0; i < 4; i++)
	{
		state->m_layerpri[i] = 0;
		state->m_layer_colorbase[i] = 0;
	}

	state->m_sprite_colorbase = 0;

	state->m_cur_control2 = 0;
	state->m_cur_sound_region = 0;
	state->m_suspension_active = 0;
	state->m_resume_trigger = 1000;
	state->m_frame = -1;
	k054539_init_flags(machine.device("k054539"), K054539_REVERSE_STEREO);
}

static MACHINE_CONFIG_START( xexex, xexex_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 32000000/2)	// 16MHz (32MHz xtal)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", xexex_interrupt, "screen", 0, 1)

	// 8MHz (PCB shows one 32MHz/18.432MHz xtal, reference: www.system16.com)
	// more likely 32MHz since 18.432MHz yields 4.608MHz(too slow) or 9.216MHz(too fast) with integer divisors
	MCFG_CPU_ADD("audiocpu", Z80, 8000000)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(1920))

	MCFG_MACHINE_START(xexex)
	MCFG_MACHINE_RESET(xexex)

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS | VIDEO_UPDATE_BEFORE_VBLANK)

	MCFG_SCREEN_ADD("screen", RASTER)
//  MCFG_SCREEN_REFRESH_RATE(8000000/512/288)
	MCFG_SCREEN_RAW_PARAMS(8000000, 384+33+40+55, 0, 383, 256+12+6+14, 0, 255)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(40, 40+384-1, 0, 0+256-1)
	MCFG_SCREEN_UPDATE(xexex)

	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(xexex)

	MCFG_K056832_ADD("k056832", xexex_k056832_intf)
	MCFG_K053246_ADD("k053246", xexex_k053246_intf)
	MCFG_K053250_ADD("k053250", "screen", -5, -16)
	MCFG_K053251_ADD("k053251")
	MCFG_K053252_ADD("k053252", 32000000/4, xexex_k053252_intf)
	MCFG_K054338_ADD("k054338", xexex_k054338_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 4000000)
	MCFG_SOUND_ROUTE(0, "filter1l", 0.50)
	MCFG_SOUND_ROUTE(0, "filter1r", 0.50)
	MCFG_SOUND_ROUTE(1, "filter2l", 0.50)
	MCFG_SOUND_ROUTE(1, "filter2r", 0.50)

	MCFG_SOUND_ADD("k054539", K054539, 48000)
	MCFG_SOUND_CONFIG(k054539_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("filter1l", FILTER_VOLUME, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("filter1r", FILTER_VOLUME, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_SOUND_ADD("filter2l", FILTER_VOLUME, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ADD("filter2r", FILTER_VOLUME, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


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

	ROM_REGION( 0x080000, "k053250", 0 )
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "xexex.nv", 0x0000, 0x080, CRC(155624cc) SHA1(457f921e3a5d053c53e4f1a44941eb0a1f22e1b2) )
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

	ROM_REGION( 0x080000, "k053250", 0 )
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "xexexa.nv", 0x0000, 0x080, CRC(051c14c6) SHA1(23addbaa2ce323c06551b343ca45dea4fd2b9eee) )
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

	ROM_REGION( 0x080000, "k053250", 0 )
	ROM_LOAD( "067_b08.rom", 0x000000, 0x080000, CRC(ca816b7b) SHA1(769ce3700e41200c34adec98598c0fe371fe1e6d) )

	ROM_REGION( 0x300000, "k054539", 0 )
	ROM_LOAD( "067_b06.rom", 0x000000, 0x200000, CRC(3b12fce4) SHA1(c69172d9965b8da8a539812fac92d5f1a3c80d17) )
	ROM_LOAD( "067_b07.rom", 0x200000, 0x100000, CRC(ec87fe1b) SHA1(ec9823aea5a1fc5c47c8262e15e10b28be87231c) )

	ROM_REGION( 0x80, "eeprom", 0 ) // default eeprom to prevent game booting upside down with error
	ROM_LOAD( "xexexj.nv", 0x0000, 0x080, CRC(79a79c7b) SHA1(02eb235226949af0147d6d0fd2bd3d7a68083ae6) )
ROM_END


static DRIVER_INIT( xexex )
{
	xexex_state *state = machine.driver_data<xexex_state>();

	state->m_strip_0x1a = 0;

	if (!strcmp(machine.system().name, "xexex"))
	{
		// Invulnerability
//      *(UINT16 *)(machine.region("maincpu")->base() + 0x648d4) = 0x4a79;
//      *(UINT16 *)(machine.region("maincpu")->base() + 0x00008) = 0x5500;
		state->m_strip_0x1a = 1;
	}
}

GAME( 1991, xexex,  0,     xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver EAA)", GAME_SUPPORTS_SAVE )
GAME( 1991, xexexa, xexex, xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver AAA)", GAME_SUPPORTS_SAVE )
GAME( 1991, xexexj, xexex, xexex, xexex, xexex, ROT0, "Konami", "Xexex (ver JAA)", GAME_SUPPORTS_SAVE )
