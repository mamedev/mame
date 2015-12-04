// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    Taito F3 Package System (aka F3 Cybercore System)

    Emulation by Bryan McPhail, mish@tendril.co.uk/mish@mame.net
    Thanks to Ian Schmidt and Stiletto for sound information!
    Major thanks to Aaron Giles for sound info, figuring out the 68K/ES5505
    rom interface and ES5505 emulator!
    Thanks to Acho A. Tang for Kirameki Star Road sound banking info!
    Thank you to Shiriru for the scanline rendering (including alpha blending),
    sprite sync fixes, sprite zoom fixes and others!

    Other Issues:
    - ES5510 DSP isn't hooked up.
    - Various hacks in video core that needs squashing;
    - When playing space invaders dx in original mode, t.t. with overlay, the
      alpha blending effect is wrong (see Taito B version of game)
    - Bubble Symphony has an alpha transition effect that doesn't appear in Mame
    - Various other missing blending effects (see Mametesters)
    - Find how this HW drives the CRTC, and convert video timings to use screen raw params;

    Feel free to report any other issues to me.

    Taito custom chips on motherboard:

        TC0630FDP - Playfield generator?  (Nearest tile roms)
        TC0640FIO - I/O & watchdog?
        TC0650FDA - Priority mixer?  (Near paletteram & video output)
        TC0660FCM - Sprites? (Nearest sprite roms)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "includes/taito_f3.h"
#include "sound/es5506.h"
#include "audio/taito_en.h"
#include "sound/okim6295.h"


/******************************************************************************/

CUSTOM_INPUT_MEMBER(taito_f3_state::f3_analog_r)
{
	int num = (FPTR)param;
	int data = m_dial[num]->read();
	return ((data & 0xf)<<12) | ((data & 0xff0)>>4);
}


CUSTOM_INPUT_MEMBER(taito_f3_state::f3_coin_r)
{
	int num = (FPTR)param;
	return m_coin_word[num];
}

READ32_MEMBER(taito_f3_state::f3_control_r)
{
	if (offset<6)
		return m_input[offset]->read();
	else logerror("CPU #0 PC %06x: warning - read unmapped control address %06x\n", space.device().safe_pc(), offset);

	return 0xffffffff;
}

WRITE32_MEMBER(taito_f3_state::f3_control_w)
{
	switch (offset)
	{
		case 0x00: /* Watchdog */
			machine().watchdog_reset();
			return;

		case 0x01: /* Coin counters & lockouts */
			if (ACCESSING_BITS_24_31)
			{
				coin_lockout_w(machine(), 0,~data & 0x01000000);
				coin_lockout_w(machine(), 1,~data & 0x02000000);
				coin_counter_w(machine(), 0, data & 0x04000000);
				coin_counter_w(machine(), 1, data & 0x08000000);
				m_coin_word[0]=(data>>16)&0xffff;
			}
			return;

		case 0x04: /* Eeprom */
			if (ACCESSING_BITS_0_7)
			{
				ioport("EEPROMOUT")->write(data, 0xff);
			}
			return;

		case 0x05:  /* Player 3 & 4 coin counters */
			if (ACCESSING_BITS_24_31)
			{
				coin_lockout_w(machine(), 2,~data & 0x01000000);
				coin_lockout_w(machine(), 3,~data & 0x02000000);
				coin_counter_w(machine(), 2, data & 0x04000000);
				coin_counter_w(machine(), 3, data & 0x08000000);
				m_coin_word[1]=(data>>16)&0xffff;
			}
			return;
	}
	logerror("CPU #0 PC %06x: warning - write unmapped control address %06x %08x\n",space.device().safe_pc(),offset,data);
}

WRITE32_MEMBER(taito_f3_state::f3_sound_reset_0_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

WRITE32_MEMBER(taito_f3_state::f3_sound_reset_1_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

WRITE32_MEMBER(taito_f3_state::f3_sound_bankswitch_w)
{
	if (m_f3_game==KIRAMEKI) {
		UINT16 *rom = (UINT16 *)memregion("audiocpu")->base();
		UINT32 idx;

		idx = (offset << 1) & 0x1e;
		if (ACCESSING_BITS_0_15)
			idx += 1;

		if (idx >= 8)
			idx -= 8;

		/* Banks are 0x20000 bytes each, divide by two to get data16
		pointer rather than byte pointer */
		membank("bank2")->set_base(&rom[(idx*0x20000)/2 + 0x80000]);

	} else {
		logerror("Sound bankswitch in unsupported game\n");
	}
}

WRITE16_MEMBER(taito_f3_state::f3_unk_w)
{
	/*
	Several games writes a value here at POST, dunno what kind of config this is ...
	ringrage:  0x0000
	arabianm:  0x0000
	ridingf: (no init)
	gseeker: (no init)
	commandw:(no init)
	cupfinal:  0x0100
	trstar:  (no init)
	gunlock:   0x0000
	scfinals:  0x0100
	lightbr: (no init)
	intcup94:  0x0100
	kaiserkn:  0x0100
	dariusg:   0x278b
	bublbob2:(no init)
	pwrgoal:   0x0100
	qtheater:  0x0090
	elvactr:   0x278b
	recalh:    0x0090
	spcinv95:  0x0100
	twinqix: (no init)
	quizhuhu:  0x0000
	pbobble2:  0x278b
	gekiridn:  0x278b
	tcobra2:   0x0000
	bubblem: (no init)
	cleopatr:  0x0100
	pbobble3:  0x278b
	arkretn:   0x0000
	kirameki:  0x0100
	puchicar:  0x0000
	pbobble4:  0x278b
	popnpop:   0x0000
	landmakr:  0x278b
	*/
	if(offset == 0)
		logerror("0x4c0000 write %04x\n",data);
	else
		popmessage("0x4c0002 write %04x, contact MAMEdev",data); //shouldn't happen
}

/******************************************************************************/

static ADDRESS_MAP_START( f3_map, AS_PROGRAM, 32, taito_f3_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x300000, 0x30007f) AM_WRITE(f3_sound_bankswitch_w)
	AM_RANGE(0x400000, 0x41ffff) AM_MIRROR(0x20000) AM_RAM AM_SHARE("f3_ram")
	AM_RANGE(0x440000, 0x447fff) AM_RAM_WRITE(f3_palette_24bit_w) AM_SHARE("paletteram")
	AM_RANGE(0x4a0000, 0x4a001f) AM_READWRITE(f3_control_r,  f3_control_w)
	AM_RANGE(0x4c0000, 0x4c0003) AM_WRITE16(f3_unk_w,0xffffffff)
	AM_RANGE(0x600000, 0x60ffff) AM_READWRITE16(f3_spriteram_r,f3_spriteram_w,0xffffffff) //AM_SHARE("spriteram")
	AM_RANGE(0x610000, 0x61bfff) AM_READWRITE16(f3_pf_data_r,f3_pf_data_w,0xffffffff)       //AM_SHARE("f3_pf_data")
	AM_RANGE(0x61c000, 0x61dfff) AM_READWRITE16(f3_videoram_r,f3_videoram_w,0xffffffff)     //AM_SHARE("videoram")
	AM_RANGE(0x61e000, 0x61ffff) AM_READWRITE16(f3_vram_r,f3_vram_w,0xffffffff)             //AM_SHARE("f3_vram")
	AM_RANGE(0x620000, 0x62ffff) AM_READWRITE16(f3_lineram_r,f3_lineram_w,0xffffffff)       //AM_SHARE("f3_line_ram")
	AM_RANGE(0x630000, 0x63ffff) AM_READWRITE16(f3_pivot_r,f3_pivot_w,0xffffffff)           //AM_SHARE("f3_pivot_ram")
	AM_RANGE(0x660000, 0x66000f) AM_WRITE16(f3_control_0_w,0xffffffff)
	AM_RANGE(0x660010, 0x66001f) AM_WRITE16(f3_control_1_w,0xffffffff)
	AM_RANGE(0xc00000, 0xc007ff) AM_RAM AM_SHARE("snd_shared")
	AM_RANGE(0xc80000, 0xc80003) AM_WRITE(f3_sound_reset_0_w)
	AM_RANGE(0xc80100, 0xc80103) AM_WRITE(f3_sound_reset_1_w)
ADDRESS_MAP_END



/******************************************************************************/

CUSTOM_INPUT_MEMBER( taito_f3_state::eeprom_read )
{
	return m_eepromin->read();
}


static INPUT_PORTS_START( f3 )
	/* MSW: Test switch, coins, eeprom access, LSW: Player Buttons, Start, Tilt, Service */
	PORT_START("IN.0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_SERVICE1 ) /* Service */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* Only on some games */
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_SERVICE3 ) /* Only on some games */
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taito_f3_state,eeprom_read, (void *)nullptr)
	PORT_BIT( 0xff000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taito_f3_state,eeprom_read, (void *)nullptr)

	/* MSW: Coin counters/lockouts are readable, LSW: Joysticks (Player 1 & 2) */
	PORT_START("IN.1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* These must be high */
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taito_f3_state,f3_coin_r, (void *)nullptr)

	/* Player 3 & 4 fire buttons (Player 2 top fire buttons in Kaiser Knuckle) */
	PORT_START("IN.4")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Player 3 & 4 joysticks (Player 1 top fire buttons in Kaiser Knuckle) */
	PORT_START("IN.5")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taito_f3_state,f3_coin_r, (void *)1)

	/* Analog control 1 */
	PORT_START("IN.2")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taito_f3_state,f3_analog_r, (void*)nullptr)
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Analog control 2 */
	PORT_START("IN.3")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, taito_f3_state,f3_analog_r, (void *)1)
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* These are not read directly, but through PORT_CUSTOMs above */
	PORT_START("EEPROMIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) /* Another service mode */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("DIAL.0")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_PLAYER(1)

	PORT_START("DIAL.1")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

static INPUT_PORTS_START( kn )
	PORT_INCLUDE( f3 )

	PORT_MODIFY("IN.0")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN.4")
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x0000f800, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN.5")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x000000f8, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	256,
	4,
	{ 0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout pivotlayout =
{
	8,8,
	2048,
	4,
	{ 0,1,2,3 },
	{ 20, 16, 28, 24, 4, 0, 12, 8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spriteram_layout =
{
	16,16,
	RGN_FRAC(1,2),
	6,  /* Palettes have 4 bpp indexes despite up to 6 bpp data */
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, 0, 1, 2, 3 },
	{
	4, 0, 12, 8,
	16+4, 16+0, 16+12, 16+8,
	32+4, 32+0, 32+12, 32+8,
	48+4, 48+0, 48+12, 48+8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,2),
	6,  /* Palettes have 4 bpp indexes despite up to 6 bpp data */
	{ RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3, 0, 1, 2, 3 },
	{
	4, 0, 16+4, 16+0,
	8+4, 8+0, 24+4, 24+0,
	32+4, 32+0, 48+4, 48+0,
	40+4, 40+0, 56+4, 56+0,
	},
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( taito_f3 )
	GFXDECODE_ENTRY( nullptr,   0x000000, charlayout,       0x0000, 0x0400>>4 ) /* Dynamically modified */
	GFXDECODE_ENTRY( "gfx2", 0x000000, tile_layout,      0x0000, 0x2000>>4 ) /* Tiles area */
	GFXDECODE_ENTRY( "gfx1", 0x000000, spriteram_layout, 0x1000, 0x1000>>4 ) /* Sprites area */
	GFXDECODE_ENTRY( nullptr,   0x000000, pivotlayout,      0x0000,  0x400>>4 ) /* Dynamically modified */
GFXDECODE_END

/******************************************************************************/

void taito_f3_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_F3_INTERRUPT3:
		m_maincpu->set_input_line(3, HOLD_LINE);    // some signal from video hardware?
		break;
	default:
		assert_always(FALSE, "Unknown id in taito_f3_state::device_timer");
	}
}

INTERRUPT_GEN_MEMBER(taito_f3_state::f3_interrupt2)
{
	device.execute().set_input_line(2, HOLD_LINE);  // vblank
	timer_set(downcast<cpu_device *>(&device)->cycles_to_attotime(10000), TIMER_F3_INTERRUPT3);
}

static const UINT16 recalh_eeprom[64] = {
	0x8554,0x0000,0x3000,0x0000,0x0000,0x0000,0x0000,0xf335,
	0x0001,0x86a0,0x0013,0x0413,0x0000,0xc350,0x0019,0x000a,
	0x0000,0x4e20,0x0003,0x180d,0x0000,0x2710,0x0005,0x1418,
	0x0000,0x1388,0x0000,0x1227,0xffff,0xffff,0xffff,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,
	0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff
};

MACHINE_START_MEMBER(taito_f3_state,f3)
{
	save_item(NAME(m_coin_word));
}

MACHINE_RESET_MEMBER(taito_f3_state,f3)
{
	/* start with sound m68k off, qtheater relies on it (otherwise main CPU tries to reset it while 68k is working with irq table vectors). */
	m_audiocpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

static MACHINE_CONFIG_START( f3, taito_f3_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(f3_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taito_f3_state,  f3_interrupt2)

	MCFG_MACHINE_START_OVERRIDE(taito_f3_state,f3)
	MCFG_MACHINE_RESET_OVERRIDE(taito_f3_state,f3)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58.97)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(624) /* 58.97 Hz, 624us vblank time */)
	MCFG_SCREEN_SIZE(40*8+48*2, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(46, 40*8-1 + 46, 24, 24+232-1)
	MCFG_SCREEN_UPDATE_DRIVER(taito_f3_state, screen_update_f3)
	MCFG_SCREEN_VBLANK_DRIVER(taito_f3_state, screen_eof_f3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", taito_f3)
	MCFG_PALETTE_ADD("palette", 0x2000)

	MCFG_VIDEO_START_OVERRIDE(taito_f3_state,f3)

	/* sound hardware */
	MCFG_FRAGMENT_ADD(taito_en_sound)
MACHINE_CONFIG_END

/* These games reprogram the video output registers to display different scanlines,
 we can't change our screen display at runtime, so we do it here instead.  None
 of the games change the registers during the game (to do so would probably require
 monitor recalibration.)
*/
static MACHINE_CONFIG_DERIVED( f3_224a, f3 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(46, 40*8-1 + 46, 31, 31+224-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( f3_224b, f3 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(46, 40*8-1 + 46, 32, 32+224-1)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( f3_224c, f3 )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_VISIBLE_AREA(46, 40*8-1 + 46, 24, 24+224-1)
MACHINE_CONFIG_END

/* recalh and gseeker need a default EEPROM to work */
static MACHINE_CONFIG_DERIVED( f3_eeprom, f3 )

	MCFG_DEVICE_REMOVE("eeprom")
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(recalh_eeprom, 128) //TODO: convert this into ROM
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( f3_224b_eeprom, f3_224b )

	MCFG_DEVICE_REMOVE("eeprom")
	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DATA(recalh_eeprom, 128) //TODO: convert this into ROM
MACHINE_CONFIG_END

static const gfx_layout bubsympb_sprite_layout =
{
	16,16,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(0,6), RGN_FRAC(1,6), RGN_FRAC(2,6), RGN_FRAC(3,6), RGN_FRAC(4,6), RGN_FRAC(5,6) },
	{ 7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static const gfx_layout bubsympb_tile_layout =
{
	16,16,
	RGN_FRAC(1,2),
	5,
	{ RGN_FRAC(1,2)+0, 0,1,2,3 },
	{ 20,16,12,8,4,0,28,24,52,48,44,40,36,32,60,56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	16*64
};


static GFXDECODE_START( bubsympb )
	GFXDECODE_ENTRY( nullptr,           0x000000, charlayout,          0,  64 ) /* Dynamically modified */
	GFXDECODE_ENTRY( "gfx2", 0x000000, bubsympb_tile_layout, 0, 512 ) /* Tiles area */
	GFXDECODE_ENTRY( "gfx1", 0x000000, bubsympb_sprite_layout, 4096, 256 ) /* Sprites area */
	GFXDECODE_ENTRY( nullptr,           0x000000, pivotlayout,         0,  64 ) /* Dynamically modified */
GFXDECODE_END

static MACHINE_CONFIG_START( bubsympb, taito_f3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(f3_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", taito_f3_state,  f3_interrupt2)

	MCFG_MACHINE_START_OVERRIDE(taito_f3_state,f3)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58.97)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(624) /* 58.97 Hz, 624us vblank time */)
	MCFG_SCREEN_SIZE(40*8+48*2, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(46, 40*8-1 + 46, 31, 31+224-1)
	MCFG_SCREEN_UPDATE_DRIVER(taito_f3_state, screen_update_f3)
	MCFG_SCREEN_VBLANK_DRIVER(taito_f3_state, screen_eof_f3)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bubsympb)
	MCFG_PALETTE_ADD("palette", 8192)

	MCFG_VIDEO_START_OVERRIDE(taito_f3_state,f3)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000 , OKIM6295_PIN7_HIGH) // not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( ringrage )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d21-23.40", 0x000000, 0x40000, CRC(14e9ed65) SHA1(956db484375a43bcdf5a6a104e3c7d7ef5baaa1b) )
	ROM_LOAD32_BYTE("d21-22.38", 0x000001, 0x40000, CRC(6f8b65b0) SHA1(a4f786b72068c6c9c1b23df67029eb0e2a982789) )
	ROM_LOAD32_BYTE("d21-21.36", 0x000002, 0x40000, CRC(bf7234bc) SHA1(781b8f850275537e1ae2dae18a4554a1283cb432) )
	ROM_LOAD32_BYTE("d21-25.34", 0x000003, 0x40000, CRC(aeff6e19) SHA1(7fd8f64f0a52dfe369a3709af8c043286b5b6fdf) )

	ROM_REGION(0x800000, "gfx1" , 0 ) /* Sprites */
	ROM_LOAD16_BYTE("d21-02.66", 0x000000, 0x200000, CRC(facd3a02) SHA1(360c6e65d01dd2c33495ba928ef9986f754b3694) )
	ROM_LOAD16_BYTE("d21-03.67", 0x000001, 0x200000, CRC(6f653e68) SHA1(840905f012c37d58160cc554c036ad25218ce3e6) )
	ROM_LOAD       ("d21-04.68", 0x600000, 0x200000, CRC(9dcdceca) SHA1(e12bab5307ebe4c3b5f9284c91f3bf7ba4c8e9bc) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0 ) /* Tiles */
	ROM_LOAD16_BYTE("d21-06.49", 0x000000, 0x080000, CRC(92d4a720) SHA1(81dc58c58d5f4f20ceeb5d6b90491f1efcbc67d3) )
	ROM_LOAD16_BYTE("d21-07.50", 0x000001, 0x080000, CRC(6da696e9) SHA1(74332090b0de4193a669cd5242fd655e7b90f772) )
	ROM_LOAD       ("d21-08.51", 0x180000, 0x080000, CRC(a0d95be9) SHA1(1746097e827ac10906f012c5c4f93c388a30f4b3) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d21-18.5", 0x100000, 0x20000, CRC(133b55d0) SHA1(feb5c9d0b1adcae3b16eb206c8ac4e73fd88bef4) )
	ROM_LOAD16_BYTE("d21-19.6", 0x100001, 0x20000, CRC(1f98908f) SHA1(972c8f7e4e417831466714efd0b4cadca1f3e8e5) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d21-01.17", 0x000000, 0x200000, CRC(1fb6f07d) SHA1(a7d21d4b0b0b141c4dbe66554e5362e2c8876067) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d21-05.18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( ringrageu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d21-23.40", 0x000000, 0x40000, CRC(14e9ed65) SHA1(956db484375a43bcdf5a6a104e3c7d7ef5baaa1b) )
	ROM_LOAD32_BYTE("d21-22.38", 0x000001, 0x40000, CRC(6f8b65b0) SHA1(a4f786b72068c6c9c1b23df67029eb0e2a982789) )
	ROM_LOAD32_BYTE("d21-21.36", 0x000002, 0x40000, CRC(bf7234bc) SHA1(781b8f850275537e1ae2dae18a4554a1283cb432) )
	ROM_LOAD32_BYTE("d21-24.34", 0x000003, 0x40000, CRC(404dee67) SHA1(1e46d52d72b6cbe5e8af9f0f8e8b1acf9c6feb26) )

	ROM_REGION(0x800000, "gfx1" , 0 ) /* Sprites */
	ROM_LOAD16_BYTE("d21-02.66", 0x000000, 0x200000, CRC(facd3a02) SHA1(360c6e65d01dd2c33495ba928ef9986f754b3694) )
	ROM_LOAD16_BYTE("d21-03.67", 0x000001, 0x200000, CRC(6f653e68) SHA1(840905f012c37d58160cc554c036ad25218ce3e6) )
	ROM_LOAD       ("d21-04.68", 0x600000, 0x200000, CRC(9dcdceca) SHA1(e12bab5307ebe4c3b5f9284c91f3bf7ba4c8e9bc) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0 ) /* Tiles */
	ROM_LOAD16_BYTE("d21-06.49", 0x000000, 0x080000, CRC(92d4a720) SHA1(81dc58c58d5f4f20ceeb5d6b90491f1efcbc67d3) )
	ROM_LOAD16_BYTE("d21-07.50", 0x000001, 0x080000, CRC(6da696e9) SHA1(74332090b0de4193a669cd5242fd655e7b90f772) )
	ROM_LOAD       ("d21-08.51", 0x180000, 0x080000, CRC(a0d95be9) SHA1(1746097e827ac10906f012c5c4f93c388a30f4b3) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d21-18.5", 0x100000, 0x20000, CRC(133b55d0) SHA1(feb5c9d0b1adcae3b16eb206c8ac4e73fd88bef4) )
	ROM_LOAD16_BYTE("d21-19.6", 0x100001, 0x20000, CRC(1f98908f) SHA1(972c8f7e4e417831466714efd0b4cadca1f3e8e5) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d21-01.17", 0x000000, 0x200000, CRC(1fb6f07d) SHA1(a7d21d4b0b0b141c4dbe66554e5362e2c8876067) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d21-05.18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( ringragej )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d21-23.40", 0x000000, 0x40000, CRC(14e9ed65) SHA1(956db484375a43bcdf5a6a104e3c7d7ef5baaa1b) )
	ROM_LOAD32_BYTE("d21-22.38", 0x000001, 0x40000, CRC(6f8b65b0) SHA1(a4f786b72068c6c9c1b23df67029eb0e2a982789) )
	ROM_LOAD32_BYTE("d21-21.36", 0x000002, 0x40000, CRC(bf7234bc) SHA1(781b8f850275537e1ae2dae18a4554a1283cb432) )
	ROM_LOAD32_BYTE("d21-20.34", 0x000003, 0x40000, CRC(a8eb68a4) SHA1(040238fad0d17cac21b144b0fcab1774d2924da9) )

	ROM_REGION(0x800000, "gfx1" , 0 ) /* Sprites */
	ROM_LOAD16_BYTE("d21-02.66", 0x000000, 0x200000, CRC(facd3a02) SHA1(360c6e65d01dd2c33495ba928ef9986f754b3694) )
	ROM_LOAD16_BYTE("d21-03.67", 0x000001, 0x200000, CRC(6f653e68) SHA1(840905f012c37d58160cc554c036ad25218ce3e6) )
	ROM_LOAD       ("d21-04.68", 0x600000, 0x200000, CRC(9dcdceca) SHA1(e12bab5307ebe4c3b5f9284c91f3bf7ba4c8e9bc) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0 ) /* Tiles */
	ROM_LOAD16_BYTE("d21-06.49", 0x000000, 0x080000, CRC(92d4a720) SHA1(81dc58c58d5f4f20ceeb5d6b90491f1efcbc67d3) )
	ROM_LOAD16_BYTE("d21-07.50", 0x000001, 0x080000, CRC(6da696e9) SHA1(74332090b0de4193a669cd5242fd655e7b90f772) )
	ROM_LOAD       ("d21-08.51", 0x180000, 0x080000, CRC(a0d95be9) SHA1(1746097e827ac10906f012c5c4f93c388a30f4b3) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d21-18.5", 0x100000, 0x20000, CRC(133b55d0) SHA1(feb5c9d0b1adcae3b16eb206c8ac4e73fd88bef4) )
	ROM_LOAD16_BYTE("d21-19.6", 0x100001, 0x20000, CRC(1f98908f) SHA1(972c8f7e4e417831466714efd0b4cadca1f3e8e5) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d21-01.17", 0x000000, 0x200000, CRC(1fb6f07d) SHA1(a7d21d4b0b0b141c4dbe66554e5362e2c8876067) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d21-05.18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( arabianm )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d29-23.ic40", 0x000000, 0x40000, CRC(89a0c706) SHA1(d0df02be2b63566ec776bb13947f975062766a01) )
	ROM_LOAD32_BYTE("d29-22.ic38", 0x000001, 0x40000, CRC(4afc22a4) SHA1(9579d134a1e4b0c86af9b41d136acfe6cc7f6624) )
	ROM_LOAD32_BYTE("d29-21.ic36", 0x000002, 0x40000, CRC(ac32eb38) SHA1(19d8d965497e41a7a2f490a197322da7fd1fa40a) )
	ROM_LOAD32_BYTE("d29-25.ic34", 0x000003, 0x40000, CRC(b9b652ed) SHA1(19cceef87884adeb03e5e330159541a1e503a7f2) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d29-03.ic66", 0x000000, 0x100000, CRC(aeaff456) SHA1(e70d0089e69d33d213be8195c31891f38fbcb53a) )
	ROM_LOAD16_BYTE("d29-04.ic67", 0x000001, 0x100000, CRC(01711cfe) SHA1(26da4cc9dcb8d38bdf8c93015f77e58ffc9d1ba9) )
	ROM_LOAD       ("d29-05.ic68", 0x300000, 0x100000, CRC(9b5f7a17) SHA1(89d9faedc7b55df6237f2c2ebb43de7de685cb66) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d29-06.ic49", 0x000000, 0x080000, CRC(eea07bf3) SHA1(7860a2c0c592af000b56e59bd39d67b086fe3606) )
	ROM_LOAD16_BYTE("d29-07.ic50", 0x000001, 0x080000, CRC(db3c094d) SHA1(5b60f0fa1054bf93ce51d310376b1abdb3022541) )
	ROM_LOAD       ("d29-08.ic51", 0x180000, 0x080000, CRC(d7562851) SHA1(91d86e75ba7a590ca298b932b4cf8fa9228f115e) )
	ROM_FILL       (               0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d29-18.ic5", 0x100000, 0x20000, CRC(d97780df) SHA1(d0f9d2fd7ce13f620bb44083bf012f67dda4b10b) )
	ROM_LOAD16_BYTE("d29-19.ic6", 0x100001, 0x20000, CRC(b1ad365c) SHA1(1cd26d8feaaa06b50dfee32e9b7950b8ee92ac55) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d29-01.ic17", 0x000000, 0x200000, CRC(545ac4b3) SHA1(f89513fca8a03cab11160aa1f0a9c3adbc8bda08) )   // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d29-02.ic18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) )   // -std-

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "palce20v8h.1",  0x0000, 0x0157, CRC(5dd5c8f9) SHA1(5e6153d9e08985b2326dfd6d73f7b90136a7a4b1) ) /* D29-11 */
	ROM_LOAD( "pal20l8b.2",    0x0200, 0x0144, CRC(c91437e2) SHA1(5bd6fb57fd7e0ff957a6ef9509b8f2e35a8ca29a) ) /* D29-12 */
	ROM_LOAD( "palce20v8h.3",  0x0400, 0x0157, CRC(74d61d36) SHA1(c34d8b2d227f69c167d1516dea53e4bcb76491d1) ) /* D29-13 */
	ROM_LOAD( "palce16v8h.11", 0x0600, 0x0117, CRC(51088324) SHA1(b985835b92c9d1e1dae6ae7cba9fa83c4db58bbb) ) /* D29-16 */
	ROM_LOAD( "pal16l8b.22",   0x0800, 0x0104, CRC(3e01e854) SHA1(72f48982673ac8337dac3358b7a79e45c60b9601) ) /* D29-09 */
	ROM_LOAD( "palce16v8h.31", 0x0a00, 0x0117, CRC(e0789727) SHA1(74add02cd194741de5ca6e36a99f9dd3e756fbdf) ) /* D29-17 */
	ROM_LOAD( "pal16l8b.62",   0x0c00, 0x0104, CRC(7093e2f3) SHA1(62bb0085ed93cc8a5fb3a1b08ce9c8071ebda657) ) /* D29-10 */
	ROM_LOAD( "palce20v8h.69", 0x0e00, 0x0157, CRC(25d205d5) SHA1(8859fd498e4d84a55424899d23db470be217eaba) ) /* D29-14 */
	ROM_LOAD( "pal20l8b.70",   0x1000, 0x0144, CRC(92b5b97c) SHA1(653ab0467f71d93eceb8143b124cdedaf1ede750) ) /* D29-15 */
ROM_END

ROM_START( arabianmj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d29-23.ic40", 0x000000, 0x40000, CRC(89a0c706) SHA1(d0df02be2b63566ec776bb13947f975062766a01) )
	ROM_LOAD32_BYTE("d29-22.ic38", 0x000001, 0x40000, CRC(4afc22a4) SHA1(9579d134a1e4b0c86af9b41d136acfe6cc7f6624) )
	ROM_LOAD32_BYTE("d29-21.ic36", 0x000002, 0x40000, CRC(ac32eb38) SHA1(19d8d965497e41a7a2f490a197322da7fd1fa40a) )
	ROM_LOAD32_BYTE("d29-20.ic34", 0x000003, 0x40000, CRC(57b833c1) SHA1(69beff431e400db17ca1983a7a4d6684a1ea701c) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d29-03.ic66", 0x000000, 0x100000, CRC(aeaff456) SHA1(e70d0089e69d33d213be8195c31891f38fbcb53a) )
	ROM_LOAD16_BYTE("d29-04.ic67", 0x000001, 0x100000, CRC(01711cfe) SHA1(26da4cc9dcb8d38bdf8c93015f77e58ffc9d1ba9) )
	ROM_LOAD       ("d29-05.ic68", 0x300000, 0x100000, CRC(9b5f7a17) SHA1(89d9faedc7b55df6237f2c2ebb43de7de685cb66) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d29-06.ic49", 0x000000, 0x080000, CRC(eea07bf3) SHA1(7860a2c0c592af000b56e59bd39d67b086fe3606) )
	ROM_LOAD16_BYTE("d29-07.ic50", 0x000001, 0x080000, CRC(db3c094d) SHA1(5b60f0fa1054bf93ce51d310376b1abdb3022541) )
	ROM_LOAD       ("d29-08.ic51", 0x180000, 0x080000, CRC(d7562851) SHA1(91d86e75ba7a590ca298b932b4cf8fa9228f115e) )
	ROM_FILL       (               0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d29-18.ic5", 0x100000, 0x20000, CRC(d97780df) SHA1(d0f9d2fd7ce13f620bb44083bf012f67dda4b10b) )
	ROM_LOAD16_BYTE("d29-19.ic6", 0x100001, 0x20000, CRC(b1ad365c) SHA1(1cd26d8feaaa06b50dfee32e9b7950b8ee92ac55) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d29-01.ic17", 0x000000, 0x200000, CRC(545ac4b3) SHA1(f89513fca8a03cab11160aa1f0a9c3adbc8bda08) )   // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d29-02.ic18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) )   // -std-

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "palce20v8h.1",  0x0000, 0x0157, CRC(5dd5c8f9) SHA1(5e6153d9e08985b2326dfd6d73f7b90136a7a4b1) ) /* D29-11 */
	ROM_LOAD( "pal20l8b.2",    0x0200, 0x0144, CRC(c91437e2) SHA1(5bd6fb57fd7e0ff957a6ef9509b8f2e35a8ca29a) ) /* D29-12 */
	ROM_LOAD( "palce20v8h.3",  0x0400, 0x0157, CRC(74d61d36) SHA1(c34d8b2d227f69c167d1516dea53e4bcb76491d1) ) /* D29-13 */
	ROM_LOAD( "palce16v8h.11", 0x0600, 0x0117, CRC(51088324) SHA1(b985835b92c9d1e1dae6ae7cba9fa83c4db58bbb) ) /* D29-16 */
	ROM_LOAD( "pal16l8b.22",   0x0800, 0x0104, CRC(3e01e854) SHA1(72f48982673ac8337dac3358b7a79e45c60b9601) ) /* D29-09 */
	ROM_LOAD( "palce16v8h.31", 0x0a00, 0x0117, CRC(e0789727) SHA1(74add02cd194741de5ca6e36a99f9dd3e756fbdf) ) /* D29-17 */
	ROM_LOAD( "pal16l8b.62",   0x0c00, 0x0104, CRC(7093e2f3) SHA1(62bb0085ed93cc8a5fb3a1b08ce9c8071ebda657) ) /* D29-10 */
	ROM_LOAD( "palce20v8h.69", 0x0e00, 0x0157, CRC(25d205d5) SHA1(8859fd498e4d84a55424899d23db470be217eaba) ) /* D29-14 */
	ROM_LOAD( "pal20l8b.70",   0x1000, 0x0144, CRC(92b5b97c) SHA1(653ab0467f71d93eceb8143b124cdedaf1ede750) ) /* D29-15 */
ROM_END

ROM_START( arabianmu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d29-23.ic40", 0x000000, 0x40000, CRC(89a0c706) SHA1(d0df02be2b63566ec776bb13947f975062766a01) )
	ROM_LOAD32_BYTE("d29-22.ic38", 0x000001, 0x40000, CRC(4afc22a4) SHA1(9579d134a1e4b0c86af9b41d136acfe6cc7f6624) )
	ROM_LOAD32_BYTE("d29-21.ic36", 0x000002, 0x40000, CRC(ac32eb38) SHA1(19d8d965497e41a7a2f490a197322da7fd1fa40a) )
	ROM_LOAD32_BYTE("d29-24.ic34", 0x000003, 0x40000, CRC(ceb1627b) SHA1(c705ed956fa80ad77c53e8e2b9d27020255578bd) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d29-03.ic66", 0x000000, 0x100000, CRC(aeaff456) SHA1(e70d0089e69d33d213be8195c31891f38fbcb53a) )
	ROM_LOAD16_BYTE("d29-04.ic67", 0x000001, 0x100000, CRC(01711cfe) SHA1(26da4cc9dcb8d38bdf8c93015f77e58ffc9d1ba9) )
	ROM_LOAD       ("d29-05.ic68", 0x300000, 0x100000, CRC(9b5f7a17) SHA1(89d9faedc7b55df6237f2c2ebb43de7de685cb66) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d29-06.ic49", 0x000000, 0x080000, CRC(eea07bf3) SHA1(7860a2c0c592af000b56e59bd39d67b086fe3606) )
	ROM_LOAD16_BYTE("d29-07.ic50", 0x000001, 0x080000, CRC(db3c094d) SHA1(5b60f0fa1054bf93ce51d310376b1abdb3022541) )
	ROM_LOAD       ("d29-08.ic51", 0x180000, 0x080000, CRC(d7562851) SHA1(91d86e75ba7a590ca298b932b4cf8fa9228f115e) )
	ROM_FILL       (               0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d29-18.ic5", 0x100000, 0x20000, CRC(d97780df) SHA1(d0f9d2fd7ce13f620bb44083bf012f67dda4b10b) )
	ROM_LOAD16_BYTE("d29-19.ic6", 0x100001, 0x20000, CRC(b1ad365c) SHA1(1cd26d8feaaa06b50dfee32e9b7950b8ee92ac55) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d29-01.ic17", 0x000000, 0x200000, CRC(545ac4b3) SHA1(f89513fca8a03cab11160aa1f0a9c3adbc8bda08) )   // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d29-02.ic18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) )   // -std-

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD( "palce20v8h.1",  0x0000, 0x0157, CRC(5dd5c8f9) SHA1(5e6153d9e08985b2326dfd6d73f7b90136a7a4b1) ) /* D29-11 */
	ROM_LOAD( "pal20l8b.2",    0x0200, 0x0144, CRC(c91437e2) SHA1(5bd6fb57fd7e0ff957a6ef9509b8f2e35a8ca29a) ) /* D29-12 */
	ROM_LOAD( "palce20v8h.3",  0x0400, 0x0157, CRC(74d61d36) SHA1(c34d8b2d227f69c167d1516dea53e4bcb76491d1) ) /* D29-13 */
	ROM_LOAD( "palce16v8h.11", 0x0600, 0x0117, CRC(51088324) SHA1(b985835b92c9d1e1dae6ae7cba9fa83c4db58bbb) ) /* D29-16 */
	ROM_LOAD( "pal16l8b.22",   0x0800, 0x0104, CRC(3e01e854) SHA1(72f48982673ac8337dac3358b7a79e45c60b9601) ) /* D29-09 */
	ROM_LOAD( "palce16v8h.31", 0x0a00, 0x0117, CRC(e0789727) SHA1(74add02cd194741de5ca6e36a99f9dd3e756fbdf) ) /* D29-17 */
	ROM_LOAD( "pal16l8b.62",   0x0c00, 0x0104, CRC(7093e2f3) SHA1(62bb0085ed93cc8a5fb3a1b08ce9c8071ebda657) ) /* D29-10 */
	ROM_LOAD( "palce20v8h.69", 0x0e00, 0x0157, CRC(25d205d5) SHA1(8859fd498e4d84a55424899d23db470be217eaba) ) /* D29-14 */
	ROM_LOAD( "pal20l8b.70",   0x1000, 0x0144, CRC(92b5b97c) SHA1(653ab0467f71d93eceb8143b124cdedaf1ede750) ) /* D29-15 */
ROM_END

ROM_START( ridingf )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d34-12.40", 0x000000, 0x40000, CRC(e67e69d4) SHA1(9960629a7576f6a1d8a0e17c1bfc202ceee9e824) )
	ROM_LOAD32_BYTE("d34-11.38", 0x000001, 0x40000, CRC(7d240a88) SHA1(3410d3f66e868f2696dbd95adbbd393596d34351) )
	ROM_LOAD32_BYTE("d34-10.36", 0x000002, 0x40000, CRC(8aa3f4ac) SHA1(ba3c1274dcaccf4ba97ff224cb453eb1ead510ed) )
	ROM_LOAD32_BYTE("d34_14.34", 0x000003, 0x40000, CRC(e000198e) SHA1(3c9fdd40ade7b02021d23b7ce63a28d80bb6e8e0) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d34-01.66", 0x000000, 0x200000, CRC(7974e6aa) SHA1(f3f697d0e2f52011046aa2db2df25a4b55a631d5) )
	ROM_LOAD16_BYTE("d34-02.67", 0x000001, 0x200000, CRC(f4422370) SHA1(27ba051e0dc27b39652ff1d940a2dd29965c6528) )
	ROM_FILL       (             0x400000, 0x400000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d34-05.49", 0x000000, 0x080000, CRC(72e3ee4b) SHA1(2b69487338c18ced7a2ac4f280e8e22aa7209eb3) )
	ROM_LOAD16_BYTE("d34-06.50", 0x000001, 0x080000, CRC(edc9b9f3) SHA1(c57bec1c8882d95388c3fae7cb5a321bdb202737) )
	ROM_FILL       (             0x100000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d34-07.5", 0x100000, 0x20000, CRC(67239e2b) SHA1(8e0268fab53d26cde5c1928326c4787533dc6ffe) )
	ROM_LOAD16_BYTE("d34-08.6", 0x100001, 0x20000, CRC(2cf20323) SHA1(b2bbac3714ecfd75506ae000c7eec603dfe3e13d) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d34-03.17", 0x000000, 0x200000, CRC(e534ef74) SHA1(532d00e927d3704e7557abd59e35de8b7661c8fa) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d34-04.18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( ridingfj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d34-12.40", 0x000000, 0x40000, CRC(e67e69d4) SHA1(9960629a7576f6a1d8a0e17c1bfc202ceee9e824) )
	ROM_LOAD32_BYTE("d34-11.38", 0x000001, 0x40000, CRC(7d240a88) SHA1(3410d3f66e868f2696dbd95adbbd393596d34351) )
	ROM_LOAD32_BYTE("d34-10.36", 0x000002, 0x40000, CRC(8aa3f4ac) SHA1(ba3c1274dcaccf4ba97ff224cb453eb1ead510ed) )
	ROM_LOAD32_BYTE("d34-09.34", 0x000003, 0x40000, CRC(0e0e78a2) SHA1(4c8d0a5d6b8c77be34fd7b9c4a2df4022e74443a) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d34-01.66", 0x000000, 0x200000, CRC(7974e6aa) SHA1(f3f697d0e2f52011046aa2db2df25a4b55a631d5) )
	ROM_LOAD16_BYTE("d34-02.67", 0x000001, 0x200000, CRC(f4422370) SHA1(27ba051e0dc27b39652ff1d940a2dd29965c6528) )
	ROM_FILL       (             0x400000, 0x400000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d34-05.49", 0x000000, 0x080000, CRC(72e3ee4b) SHA1(2b69487338c18ced7a2ac4f280e8e22aa7209eb3) )
	ROM_LOAD16_BYTE("d34-06.50", 0x000001, 0x080000, CRC(edc9b9f3) SHA1(c57bec1c8882d95388c3fae7cb5a321bdb202737) )
	ROM_FILL       (             0x100000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d34-07.5", 0x100000, 0x20000, CRC(67239e2b) SHA1(8e0268fab53d26cde5c1928326c4787533dc6ffe) )
	ROM_LOAD16_BYTE("d34-08.6", 0x100001, 0x20000, CRC(2cf20323) SHA1(b2bbac3714ecfd75506ae000c7eec603dfe3e13d) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d34-03.17", 0x000000, 0x200000, CRC(e534ef74) SHA1(532d00e927d3704e7557abd59e35de8b7661c8fa) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d34-04.18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( ridingfu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d34-12.40", 0x000000, 0x40000, CRC(e67e69d4) SHA1(9960629a7576f6a1d8a0e17c1bfc202ceee9e824) )
	ROM_LOAD32_BYTE("d34-11.38", 0x000001, 0x40000, CRC(7d240a88) SHA1(3410d3f66e868f2696dbd95adbbd393596d34351) )
	ROM_LOAD32_BYTE("d34-10.36", 0x000002, 0x40000, CRC(8aa3f4ac) SHA1(ba3c1274dcaccf4ba97ff224cb453eb1ead510ed) )
	ROM_LOAD32_BYTE("d34_13.34", 0x000003, 0x40000, CRC(97072918) SHA1(7ae96fb7a07b7192c39ec496e1193c1cbfbbc770) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d34-01.66", 0x000000, 0x200000, CRC(7974e6aa) SHA1(f3f697d0e2f52011046aa2db2df25a4b55a631d5) )
	ROM_LOAD16_BYTE("d34-02.67", 0x000001, 0x200000, CRC(f4422370) SHA1(27ba051e0dc27b39652ff1d940a2dd29965c6528) )
	ROM_FILL       (             0x400000, 0x400000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d34-05.49", 0x000000, 0x080000, CRC(72e3ee4b) SHA1(2b69487338c18ced7a2ac4f280e8e22aa7209eb3) )
	ROM_LOAD16_BYTE("d34-06.50", 0x000001, 0x080000, CRC(edc9b9f3) SHA1(c57bec1c8882d95388c3fae7cb5a321bdb202737) )
	ROM_FILL       (             0x100000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d34-07.5", 0x100000, 0x20000, CRC(67239e2b) SHA1(8e0268fab53d26cde5c1928326c4787533dc6ffe) )
	ROM_LOAD16_BYTE("d34-08.6", 0x100001, 0x20000, CRC(2cf20323) SHA1(b2bbac3714ecfd75506ae000c7eec603dfe3e13d) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d34-03.17", 0x000000, 0x200000, CRC(e534ef74) SHA1(532d00e927d3704e7557abd59e35de8b7661c8fa) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d34-04.18", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( gseeker )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d40_12.rom", 0x000000, 0x40000, CRC(884055fb) SHA1(a91c3dcd635a3e22fbec94c9fc46d3c29fde5e71) )
	ROM_LOAD32_BYTE("d40_11.rom", 0x000001, 0x40000, CRC(85e701d2) SHA1(da0bc34a2c64d2db2fe143ad5a77bf667de5b015) )
	ROM_LOAD32_BYTE("d40_10.rom", 0x000002, 0x40000, CRC(1e659ac5) SHA1(cd435d361c4353b361ef975f208d81369d5d079f) )
	ROM_LOAD32_BYTE("d40_14.rom", 0x000003, 0x40000, CRC(d9a76bd9) SHA1(8edaf114c1342b33fd518320a181743d1dd324c1) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d40_03.rom", 0x000000, 0x100000, CRC(bcd70efc) SHA1(c65f934022d84e29cfd396cf70b6c1afdf90500b) )
	ROM_LOAD16_BYTE("d40_04.rom", 0x100001, 0x080000, CRC(cd2ac666) SHA1(abf09979f1fe8575323b95b95688628ce4fc2350) )
	ROM_CONTINUE(0,0x80000)
	ROM_LOAD16_BYTE("d40_15.rom", 0x000000, 0x080000, CRC(50555125) SHA1(587cdfb2e027c1d96ecc46d2612956deca5fd36f) )
	ROM_LOAD16_BYTE("d40_16.rom", 0x000001, 0x080000, CRC(3f9bbe1e) SHA1(6d9b2d2d893257ad096c276eff4077f60a81921f) )
	/* Taito manufactured mask roms 3 + 4 wrong, and later added 15 + 16 as a patch */
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d40_05.rom", 0x000000, 0x100000, CRC(be6eec8f) SHA1(725e5e09f6ee60bd4c38fa223c4dea202c56f75f) )
	ROM_LOAD16_BYTE("d40_06.rom", 0x000001, 0x100000, CRC(a822abe4) SHA1(1a0dd9dcb8e24daab6eb1661307ef0e10f7e4d4e) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d40_07.rom", 0x100000, 0x20000, CRC(7e9b26c2) SHA1(d88ad39a9d70b4a5bd3f83e0d4d0725f659f1d2a) )
	ROM_LOAD16_BYTE("d40_08.rom", 0x100001, 0x20000, CRC(9c926a28) SHA1(9d9ee75eb895edc381c3ab4df5af941f84cd2073) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d40_01.rom", 0x000000, 0x200000, CRC(ee312e95) SHA1(885553950c2b2195d664639bf7e0d1ffa3e8346a) )    // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d40_02.rom", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) )    // -std-
ROM_END

ROM_START( gseekerj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d40_12.rom", 0x000000, 0x40000, CRC(884055fb) SHA1(a91c3dcd635a3e22fbec94c9fc46d3c29fde5e71) )
	ROM_LOAD32_BYTE("d40_11.rom", 0x000001, 0x40000, CRC(85e701d2) SHA1(da0bc34a2c64d2db2fe143ad5a77bf667de5b015) )
	ROM_LOAD32_BYTE("d40_10.rom", 0x000002, 0x40000, CRC(1e659ac5) SHA1(cd435d361c4353b361ef975f208d81369d5d079f) )
	ROM_LOAD32_BYTE("d40-09.34",  0x000003, 0x40000, CRC(37a90af5) SHA1(1f3401148375c9ca638ca6db6098ea4acf7d63a6) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d40_03.rom", 0x000000, 0x100000, CRC(bcd70efc) SHA1(c65f934022d84e29cfd396cf70b6c1afdf90500b) )
	ROM_LOAD16_BYTE("d40_04.rom", 0x100001, 0x080000, CRC(cd2ac666) SHA1(abf09979f1fe8575323b95b95688628ce4fc2350) )
	ROM_CONTINUE(0,0x80000)
	ROM_LOAD16_BYTE("d40_15.rom", 0x000000, 0x080000, CRC(50555125) SHA1(587cdfb2e027c1d96ecc46d2612956deca5fd36f) )
	ROM_LOAD16_BYTE("d40_16.rom", 0x000001, 0x080000, CRC(3f9bbe1e) SHA1(6d9b2d2d893257ad096c276eff4077f60a81921f) )
	/* Taito manufactured mask roms 3 + 4 wrong, and later added 15 + 16 as a patch */
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d40_05.rom", 0x000000, 0x100000, CRC(be6eec8f) SHA1(725e5e09f6ee60bd4c38fa223c4dea202c56f75f) )
	ROM_LOAD16_BYTE("d40_06.rom", 0x000001, 0x100000, CRC(a822abe4) SHA1(1a0dd9dcb8e24daab6eb1661307ef0e10f7e4d4e) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d40_07.rom", 0x100000, 0x20000, CRC(7e9b26c2) SHA1(d88ad39a9d70b4a5bd3f83e0d4d0725f659f1d2a) )
	ROM_LOAD16_BYTE("d40_08.rom", 0x100001, 0x20000, CRC(9c926a28) SHA1(9d9ee75eb895edc381c3ab4df5af941f84cd2073) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d40_01.rom", 0x000000, 0x200000, CRC(ee312e95) SHA1(885553950c2b2195d664639bf7e0d1ffa3e8346a) )    // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d40_02.rom", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) )    // -std-
ROM_END

ROM_START( gseekeru )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d40_12.rom", 0x000000, 0x40000, CRC(884055fb) SHA1(a91c3dcd635a3e22fbec94c9fc46d3c29fde5e71) )
	ROM_LOAD32_BYTE("d40_11.rom", 0x000001, 0x40000, CRC(85e701d2) SHA1(da0bc34a2c64d2db2fe143ad5a77bf667de5b015) )
	ROM_LOAD32_BYTE("d40_10.rom", 0x000002, 0x40000, CRC(1e659ac5) SHA1(cd435d361c4353b361ef975f208d81369d5d079f) )
	ROM_LOAD32_BYTE("d40-13.bin", 0x000003, 0x40000, CRC(aea05b4f) SHA1(4be054ebec49d694a7a3ee9bc66c22c46126ea4f) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d40_03.rom", 0x000000, 0x100000, CRC(bcd70efc) SHA1(c65f934022d84e29cfd396cf70b6c1afdf90500b) )
	ROM_LOAD16_BYTE("d40_04.rom", 0x100001, 0x080000, CRC(cd2ac666) SHA1(abf09979f1fe8575323b95b95688628ce4fc2350) )
	ROM_CONTINUE(0,0x80000)
	ROM_LOAD16_BYTE("d40_15.rom", 0x000000, 0x080000, CRC(50555125) SHA1(587cdfb2e027c1d96ecc46d2612956deca5fd36f) )
	ROM_LOAD16_BYTE("d40_16.rom", 0x000001, 0x080000, CRC(3f9bbe1e) SHA1(6d9b2d2d893257ad096c276eff4077f60a81921f) )
	/* Taito manufactured mask roms 3 + 4 wrong, and later added 15 + 16 as a patch */
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d40_05.rom", 0x000000, 0x100000, CRC(be6eec8f) SHA1(725e5e09f6ee60bd4c38fa223c4dea202c56f75f) )
	ROM_LOAD16_BYTE("d40_06.rom", 0x000001, 0x100000, CRC(a822abe4) SHA1(1a0dd9dcb8e24daab6eb1661307ef0e10f7e4d4e) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d40_07.rom", 0x100000, 0x20000, CRC(7e9b26c2) SHA1(d88ad39a9d70b4a5bd3f83e0d4d0725f659f1d2a) )
	ROM_LOAD16_BYTE("d40_08.rom", 0x100001, 0x20000, CRC(9c926a28) SHA1(9d9ee75eb895edc381c3ab4df5af941f84cd2073) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d40_01.rom", 0x000000, 0x200000, CRC(ee312e95) SHA1(885553950c2b2195d664639bf7e0d1ffa3e8346a) )    // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d40_02.rom", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) )    // -std-
ROM_END

ROM_START( commandw )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("cw_mpr3.bin", 0x000000, 0x40000, CRC(636944fc) SHA1(47deffed68ac179f27bfdb21ed62d6555a4b8988) )
	ROM_LOAD32_BYTE("cw_mpr2.bin", 0x000001, 0x40000, CRC(1151a42b) SHA1(e938913ecd3211a8fb4041ec5a5694cd9df9be69) )
	ROM_LOAD32_BYTE("cw_mpr1.bin", 0x000002, 0x40000, CRC(93669389) SHA1(11336a15900c4f419f3af5c423fbc502f4db616b) )
	ROM_LOAD32_BYTE("cw_mpr0.bin", 0x000003, 0x40000, CRC(0468df52) SHA1(0da923aa779b541e700c5249272e9c59ab59e863) )

	ROM_REGION(0x1000000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("cw_objl0.bin",  0x000000, 0x200000, CRC(9822102e) SHA1(c4e80ab4d54c39676ee6e557a03828250077765b) )
	ROM_LOAD16_BYTE("cw_objm0.bin",  0x000001, 0x200000, CRC(f7687684) SHA1(0bed6362dee96083e2e8b6448c9f7bfa5166bfb7) )
	ROM_LOAD16_BYTE("cw_objl1.bin",  0x400000, 0x200000,  CRC(ca3ad7f6) SHA1(849fbb89f0b132c83db5b7d699078da3cc10baf6) )
	ROM_LOAD16_BYTE("cw_objm1.bin",  0x400001, 0x200000, CRC(504b1bf5) SHA1(7b8ff7834907a9cdab5416bf713487bf71b9070e) )
	ROM_LOAD       ("cw_objh0.bin",  0xc00000, 0x200000, CRC(83d7e0ae) SHA1(774a07d0cadc2c8f5ec155270bf927e4462654e2) )
	ROM_LOAD       ("cw_objh1.bin",  0xe00000, 0x200000, CRC(324f5832) SHA1(ff91243c5d09c4c46904640fe278a7485db70577) )
	ROM_FILL       (                 0x800000, 0x400000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("cw_scr_l.bin", 0x000000, 0x100000, CRC(4d202323) SHA1(0150bcba6d2bf2c3cde88bb519f57f3b58314244) )
	ROM_LOAD16_BYTE("cw_scr_m.bin", 0x000001, 0x100000, CRC(537b1c7d) SHA1(bc61aa61891366cbea4b8ecb820d93e28d01f8d2) )
	ROM_LOAD       ("cw_scr_h.bin", 0x300000, 0x100000, CRC(001f85dd) SHA1(2532377c0b54bc964ea4e74911ff62fea2d53975) )
	ROM_FILL       (                0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("cw_spr1.bin", 0x100000, 0x20000, CRC(c8f81c25) SHA1(1c914053826587cc2d5d2c0220a3e29a641fe6f9) )
	ROM_LOAD16_BYTE("cw_spr0.bin", 0x100001, 0x20000, CRC(2aaa9dfb) SHA1(6d4c36ff54a84035c0ddf40e4f3eafd2adc15a5e) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("cw_pcm_0.bin", 0x000000, 0x200000, CRC(a1e26629) SHA1(0c5899a767f66f67a5d59b8d287d74b54f8c3727) )  // C8 C9 CA CB
	ROM_LOAD16_BYTE("cw_pcm_1.bin", 0x400000, 0x200000, CRC(39fc6cf4) SHA1(d43ef294af62765bfec089fac1d67ad81e1b06da) )  // CC -- -std-
ROM_END

ROM_START( cupfinal )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d49-13.20", 0x000000, 0x20000, CRC(ccee5e73) SHA1(5273e3b9bc6fc4fa0c63d9c62aa6b638e9780c24) )
	ROM_LOAD32_BYTE("d49-14.19", 0x000001, 0x20000, CRC(2323bf2e) SHA1(e43f9eac6887e39d5c0f39264aa914a5d5f84cca) )
	ROM_LOAD32_BYTE("d49-16.18", 0x000002, 0x20000, CRC(8e73f739) SHA1(620a4d52abc00908cd1393babdc600b929019a51) )
	ROM_LOAD32_BYTE("d49-20.17", 0x000003, 0x20000, CRC(1e9c392c) SHA1(4ed9390b84c23809215a42c930ab0451531cfef1) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d49-01.12", 0x000000, 0x200000, CRC(1dc89f1c) SHA1(9597b1d8c9b447080ca9401aee83bb4a64bb8332) )
	ROM_LOAD16_BYTE("d49-02.8",  0x000001, 0x200000, CRC(1e4c374f) SHA1(512edc6a934578d0e7371410a041150d3b13aaad) )
	ROM_LOAD16_BYTE("d49-06.11", 0x400000, 0x100000, CRC(71ef4ee1) SHA1(1d7729dbc77f7201ff574e8aef65a55bd81c25a7) )
	ROM_LOAD16_BYTE("d49-07.7",  0x400001, 0x100000, CRC(e5655b8f) SHA1(2c21745370bfe9dbf0e95f7ce42ed34a162bff64) )
	ROM_LOAD       ("d49-03.4",  0x900000, 0x200000, CRC(cf9a8727) SHA1(f21787fdcdd8be2009c2d481a9b2d7fc03ce782e) )
	ROM_LOAD       ("d49-08.3",  0xb00000, 0x100000, CRC(7d3c6536) SHA1(289b4bf79ebd9cbdf64ab956784d226e6d546654) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d49-09.47", 0x000000, 0x080000, CRC(257ede01) SHA1(c36397d95706c5e68a7738c84829a51c5e8f5ef7) )
	ROM_LOAD16_BYTE("d49-10.45", 0x000001, 0x080000, CRC(f587b787) SHA1(22db4904c134756ddd0f753f197419d27e60a827) )
	ROM_LOAD       ("d49-11.43", 0x180000, 0x080000, CRC(11318b26) SHA1(a7153f9f406d52189f59cbe58d65f88f4e2e6fcc) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d49-17.32", 0x100000, 0x20000, CRC(f2058eba) SHA1(7faaa94fadf02b6304287b61fb9613f9f4169fef) )
	ROM_LOAD16_BYTE("d49-18.33", 0x100001, 0x20000, CRC(a0fdd270) SHA1(9b5a2c8d35ea3bc6842e3c328447c3bf641b9237) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d49-04.38", 0x000000, 0x200000, CRC(44b365a9) SHA1(14c4a6b193a0069360406c74c500ba24f2a55b62) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d49-05.41", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( hthero93 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d49-13.20", 0x000000, 0x20000, CRC(ccee5e73) SHA1(5273e3b9bc6fc4fa0c63d9c62aa6b638e9780c24) )
	ROM_LOAD32_BYTE("d49-14.19", 0x000001, 0x20000, CRC(2323bf2e) SHA1(e43f9eac6887e39d5c0f39264aa914a5d5f84cca) )
	ROM_LOAD32_BYTE("d49-16.18", 0x000002, 0x20000, CRC(8e73f739) SHA1(620a4d52abc00908cd1393babdc600b929019a51) )
	ROM_LOAD32_BYTE("d49-19.17",  0x000003, 0x20000, CRC(f0925800) SHA1(e8d91b216a0409080b77cc1e832b7d15c66a5eef) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d49-01.12", 0x000000, 0x200000, CRC(1dc89f1c) SHA1(9597b1d8c9b447080ca9401aee83bb4a64bb8332) )
	ROM_LOAD16_BYTE("d49-02.8",  0x000001, 0x200000, CRC(1e4c374f) SHA1(512edc6a934578d0e7371410a041150d3b13aaad) )
	ROM_LOAD16_BYTE("d49-06.11", 0x400000, 0x100000, CRC(71ef4ee1) SHA1(1d7729dbc77f7201ff574e8aef65a55bd81c25a7) )
	ROM_LOAD16_BYTE("d49-07.7",  0x400001, 0x100000, CRC(e5655b8f) SHA1(2c21745370bfe9dbf0e95f7ce42ed34a162bff64) )
	ROM_LOAD       ("d49-03.4",  0x900000, 0x200000, CRC(cf9a8727) SHA1(f21787fdcdd8be2009c2d481a9b2d7fc03ce782e) )
	ROM_LOAD       ("d49-08.3",  0xb00000, 0x100000, CRC(7d3c6536) SHA1(289b4bf79ebd9cbdf64ab956784d226e6d546654) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d49-09.47", 0x000000, 0x080000, CRC(257ede01) SHA1(c36397d95706c5e68a7738c84829a51c5e8f5ef7) )
	ROM_LOAD16_BYTE("d49-10.45", 0x000001, 0x080000, CRC(f587b787) SHA1(22db4904c134756ddd0f753f197419d27e60a827) )
	ROM_LOAD       ("d49-11.43", 0x180000, 0x080000, CRC(11318b26) SHA1(a7153f9f406d52189f59cbe58d65f88f4e2e6fcc) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d49-17.32", 0x100000, 0x20000, CRC(f2058eba) SHA1(7faaa94fadf02b6304287b61fb9613f9f4169fef) )
	ROM_LOAD16_BYTE("d49-18.33", 0x100001, 0x20000, CRC(a0fdd270) SHA1(9b5a2c8d35ea3bc6842e3c328447c3bf641b9237) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d49-04.38", 0x000000, 0x200000, CRC(44b365a9) SHA1(14c4a6b193a0069360406c74c500ba24f2a55b62) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d49-05.41", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( trstar )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d53-15-1.24", 0x000000, 0x40000, CRC(098bba94) SHA1(b77990213ac790d15bdc0dc1e8f7adf04fe5e952) )
	ROM_LOAD32_BYTE("d53-16-1.26", 0x000001, 0x40000, CRC(4fa8b15c) SHA1(821c21e1b958614ba6636330583b3661f9e0cebb) )
	ROM_LOAD32_BYTE("d53-18-1.37", 0x000002, 0x40000, CRC(aa71cfcc) SHA1(ba62c01255cdfe0821d1b72b7f11d6e1f88b09d7) )
	ROM_LOAD32_BYTE("d53-20-1.35", 0x000003, 0x40000, CRC(4de1e287) SHA1(2b592ecbf8d81aca49844ed81c351818409f596f) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d53-03.45", 0x000000, 0x200000, CRC(91b66145) SHA1(df5bc2e544ce80a98db1fe28b4a8af8c3905c7eb) )
	ROM_LOAD16_BYTE("d53-04.46", 0x000001, 0x200000, CRC(ac3a5e80) SHA1(8a6ea8096099b465d63d56abc79ed77304fd4fa4) )
	ROM_LOAD16_BYTE("d53-06.64", 0x400000, 0x100000, CRC(f4bac410) SHA1(569bcd81d596b24add5db4a145ae04750a1bb086) )
	ROM_LOAD16_BYTE("d53-07.65", 0x400001, 0x100000, CRC(2f4773c3) SHA1(17cef13de0836923743b336cc5a64f7452629486) )
	ROM_LOAD       ("d53-05.47", 0x900000, 0x200000, CRC(b9b68b15) SHA1(c3783b09b22954a959188b80e537fa84d827ac47) )
	ROM_LOAD       ("d53-08.66", 0xb00000, 0x100000, CRC(ad13a1ee) SHA1(341112055b6bee33072c262f4ea7c4d0970888a6) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d53-09.48", 0x000000, 0x100000, CRC(690554d3) SHA1(113afd8fe7b77a30c2e3c5baca3f19d74902625b) )
	ROM_LOAD16_BYTE("d53-10.49", 0x000001, 0x100000, CRC(0ec05dc5) SHA1(781a6362ef963417fb6383a62dcc70d6f5b3131b) )
	ROM_LOAD       ("d53-11.50", 0x300000, 0x100000, CRC(39c0a546) SHA1(53f03586f6586032fc3b4f90e987c1128edbb0a7) )
	ROM_FILL       (             0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d53-13.10", 0x100000, 0x20000, CRC(877f0361) SHA1(eda58d71fb06f739bee1451d7aa7e7e6dee10e03) )
	ROM_LOAD16_BYTE("d53-14.23", 0x100001, 0x20000, CRC(a8664867) SHA1(dffddca469019abac33a1abe41c3fe83fbf553ce) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d53-01.2", 0x000000, 0x200000, CRC(28fd2d9b) SHA1(e08037795952a28e7a5e90437f1b9675aadfa136) )  // C8 C9 CA CB
	ROM_LOAD16_BYTE("d53-02.3", 0x400000, 0x200000, CRC(8bd4367a) SHA1(9b274fe321c4faedb7d44f7998ae2e37c6899688) )  // CC CD -std-
ROM_END

ROM_START( trstarj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d53-15-1.24", 0x000000, 0x40000, CRC(098bba94) SHA1(b77990213ac790d15bdc0dc1e8f7adf04fe5e952) )
	ROM_LOAD32_BYTE("d53-16-1.26", 0x000001, 0x40000, CRC(4fa8b15c) SHA1(821c21e1b958614ba6636330583b3661f9e0cebb) )
	ROM_LOAD32_BYTE("d53-18-1.37", 0x000002, 0x40000, CRC(aa71cfcc) SHA1(ba62c01255cdfe0821d1b72b7f11d6e1f88b09d7) )
	ROM_LOAD32_BYTE("d53-17-1.35", 0x000003, 0x40000, CRC(a3ef83ab) SHA1(c99170047e678a7acde1bf64f903f240e9384b94) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d53-03.45", 0x000000, 0x200000, CRC(91b66145) SHA1(df5bc2e544ce80a98db1fe28b4a8af8c3905c7eb) )
	ROM_LOAD16_BYTE("d53-04.46", 0x000001, 0x200000, CRC(ac3a5e80) SHA1(8a6ea8096099b465d63d56abc79ed77304fd4fa4) )
	ROM_LOAD16_BYTE("d53-06.64", 0x400000, 0x100000, CRC(f4bac410) SHA1(569bcd81d596b24add5db4a145ae04750a1bb086) )
	ROM_LOAD16_BYTE("d53-07.65", 0x400001, 0x100000, CRC(2f4773c3) SHA1(17cef13de0836923743b336cc5a64f7452629486) )
	ROM_LOAD       ("d53-05.47", 0x900000, 0x200000, CRC(b9b68b15) SHA1(c3783b09b22954a959188b80e537fa84d827ac47) )
	ROM_LOAD       ("d53-08.66", 0xb00000, 0x100000, CRC(ad13a1ee) SHA1(341112055b6bee33072c262f4ea7c4d0970888a6) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d53-09.48", 0x000000, 0x100000, CRC(690554d3) SHA1(113afd8fe7b77a30c2e3c5baca3f19d74902625b) )
	ROM_LOAD16_BYTE("d53-10.49", 0x000001, 0x100000, CRC(0ec05dc5) SHA1(781a6362ef963417fb6383a62dcc70d6f5b3131b) )
	ROM_LOAD       ("d53-11.50", 0x300000, 0x100000, CRC(39c0a546) SHA1(53f03586f6586032fc3b4f90e987c1128edbb0a7) )
	ROM_FILL       (             0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d53-13.10", 0x100000, 0x20000, CRC(877f0361) SHA1(eda58d71fb06f739bee1451d7aa7e7e6dee10e03) )
	ROM_LOAD16_BYTE("d53-14.23", 0x100001, 0x20000, CRC(a8664867) SHA1(dffddca469019abac33a1abe41c3fe83fbf553ce) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d53-01.2", 0x000000, 0x200000, CRC(28fd2d9b) SHA1(e08037795952a28e7a5e90437f1b9675aadfa136) )  // C8 C9 CA CB
	ROM_LOAD16_BYTE("d53-02.3", 0x400000, 0x200000, CRC(8bd4367a) SHA1(9b274fe321c4faedb7d44f7998ae2e37c6899688) )  // CC CD -std-
ROM_END

ROM_START( prmtmfgt )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d53-15-1.24", 0x000000, 0x40000, CRC(098bba94) SHA1(b77990213ac790d15bdc0dc1e8f7adf04fe5e952) )
	ROM_LOAD32_BYTE("d53-16-1.26", 0x000001, 0x40000, CRC(4fa8b15c) SHA1(821c21e1b958614ba6636330583b3661f9e0cebb) )
	ROM_LOAD32_BYTE("d53-18-1.37", 0x000002, 0x40000, CRC(aa71cfcc) SHA1(ba62c01255cdfe0821d1b72b7f11d6e1f88b09d7) )
	ROM_LOAD32_BYTE("d53-19-1.35", 0x000003, 0x40000, CRC(3ae6d211) SHA1(f3e27e0169686633d8d8f2cbac05375aa94cfde9) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d53-03.45", 0x000000, 0x200000, CRC(91b66145) SHA1(df5bc2e544ce80a98db1fe28b4a8af8c3905c7eb) )
	ROM_LOAD16_BYTE("d53-04.46", 0x000001, 0x200000, CRC(ac3a5e80) SHA1(8a6ea8096099b465d63d56abc79ed77304fd4fa4) )
	ROM_LOAD16_BYTE("d53-06.64", 0x400000, 0x100000, CRC(f4bac410) SHA1(569bcd81d596b24add5db4a145ae04750a1bb086) )
	ROM_LOAD16_BYTE("d53-07.65", 0x400001, 0x100000, CRC(2f4773c3) SHA1(17cef13de0836923743b336cc5a64f7452629486) )
	ROM_LOAD       ("d53-05.47", 0x900000, 0x200000, CRC(b9b68b15) SHA1(c3783b09b22954a959188b80e537fa84d827ac47) )
	ROM_LOAD       ("d53-08.66", 0xb00000, 0x100000, CRC(ad13a1ee) SHA1(341112055b6bee33072c262f4ea7c4d0970888a6) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d53-09.48", 0x000000, 0x100000, CRC(690554d3) SHA1(113afd8fe7b77a30c2e3c5baca3f19d74902625b) )
	ROM_LOAD16_BYTE("d53-10.49", 0x000001, 0x100000, CRC(0ec05dc5) SHA1(781a6362ef963417fb6383a62dcc70d6f5b3131b) )
	ROM_LOAD       ("d53-11.50", 0x300000, 0x100000, CRC(39c0a546) SHA1(53f03586f6586032fc3b4f90e987c1128edbb0a7) )
	ROM_FILL       (             0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d53-13.10", 0x100000, 0x20000, CRC(877f0361) SHA1(eda58d71fb06f739bee1451d7aa7e7e6dee10e03) )
	ROM_LOAD16_BYTE("d53-14.23", 0x100001, 0x20000, CRC(a8664867) SHA1(dffddca469019abac33a1abe41c3fe83fbf553ce) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d53-01.2", 0x000000, 0x200000, CRC(28fd2d9b) SHA1(e08037795952a28e7a5e90437f1b9675aadfa136) )  // C8 C9 CA CB
	ROM_LOAD16_BYTE("d53-02.3", 0x400000, 0x200000, CRC(8bd4367a) SHA1(9b274fe321c4faedb7d44f7998ae2e37c6899688) )  // CC CD -std-
ROM_END

ROM_START( trstaro )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d53-15.24", 0x000000, 0x40000, CRC(f24de51b) SHA1(d45d1b60901995edf0721eae7eb8c6e829f47d8d) )
	ROM_LOAD32_BYTE("d53-16.26", 0x000001, 0x40000, CRC(ffc84429) SHA1(23354c1a65853c06e5c959957a92b700b1418fd4) )
	ROM_LOAD32_BYTE("d53-18.37", 0x000002, 0x40000, CRC(ea2d6e13) SHA1(96461b73de745c4b0ac99267931106e1d5dcb664) )
	ROM_LOAD32_BYTE("d53-20.35", 0x000003, 0x40000, CRC(77e1f267) SHA1(763ccab234c45ea00908198b0aef3ba63ddfb8f8) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d53-03.45", 0x000000, 0x200000, CRC(91b66145) SHA1(df5bc2e544ce80a98db1fe28b4a8af8c3905c7eb) )
	ROM_LOAD16_BYTE("d53-04.46", 0x000001, 0x200000, CRC(ac3a5e80) SHA1(8a6ea8096099b465d63d56abc79ed77304fd4fa4) )
	ROM_LOAD16_BYTE("d53-06.64", 0x400000, 0x100000, CRC(f4bac410) SHA1(569bcd81d596b24add5db4a145ae04750a1bb086) )
	ROM_LOAD16_BYTE("d53-07.65", 0x400001, 0x100000, CRC(2f4773c3) SHA1(17cef13de0836923743b336cc5a64f7452629486) )
	ROM_LOAD       ("d53-05.47", 0x900000, 0x200000, CRC(b9b68b15) SHA1(c3783b09b22954a959188b80e537fa84d827ac47) )
	ROM_LOAD       ("d53-08.66", 0xb00000, 0x100000, CRC(ad13a1ee) SHA1(341112055b6bee33072c262f4ea7c4d0970888a6) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d53-09.48", 0x000000, 0x100000, CRC(690554d3) SHA1(113afd8fe7b77a30c2e3c5baca3f19d74902625b) )
	ROM_LOAD16_BYTE("d53-10.49", 0x000001, 0x100000, CRC(0ec05dc5) SHA1(781a6362ef963417fb6383a62dcc70d6f5b3131b) )
	ROM_LOAD       ("d53-11.50", 0x300000, 0x100000, CRC(39c0a546) SHA1(53f03586f6586032fc3b4f90e987c1128edbb0a7) )
	ROM_FILL       (             0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d53-13.10", 0x100000, 0x20000, CRC(877f0361) SHA1(eda58d71fb06f739bee1451d7aa7e7e6dee10e03) )
	ROM_LOAD16_BYTE("d53-14.23", 0x100001, 0x20000, CRC(a8664867) SHA1(dffddca469019abac33a1abe41c3fe83fbf553ce) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d53-01.2", 0x000000, 0x200000, CRC(28fd2d9b) SHA1(e08037795952a28e7a5e90437f1b9675aadfa136) )  // C8 C9 CA CB
	ROM_LOAD16_BYTE("d53-02.3", 0x400000, 0x200000, CRC(8bd4367a) SHA1(9b274fe321c4faedb7d44f7998ae2e37c6899688) )  // CC CD -std-
ROM_END

ROM_START( trstaroj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d53-15.24", 0x000000, 0x40000, CRC(f24de51b) SHA1(d45d1b60901995edf0721eae7eb8c6e829f47d8d) )
	ROM_LOAD32_BYTE("d53-16.26", 0x000001, 0x40000, CRC(ffc84429) SHA1(23354c1a65853c06e5c959957a92b700b1418fd4) )
	ROM_LOAD32_BYTE("d53-18.37", 0x000002, 0x40000, CRC(ea2d6e13) SHA1(96461b73de745c4b0ac99267931106e1d5dcb664) )
	ROM_LOAD32_BYTE("d53-17.35", 0x000003, 0x40000, CRC(99ef934b) SHA1(a04a27f67b2db87549f4dc09cf9d00f3480351a6) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d53-03.45", 0x000000, 0x200000, CRC(91b66145) SHA1(df5bc2e544ce80a98db1fe28b4a8af8c3905c7eb) )
	ROM_LOAD16_BYTE("d53-04.46", 0x000001, 0x200000, CRC(ac3a5e80) SHA1(8a6ea8096099b465d63d56abc79ed77304fd4fa4) )
	ROM_LOAD16_BYTE("d53-06.64", 0x400000, 0x100000, CRC(f4bac410) SHA1(569bcd81d596b24add5db4a145ae04750a1bb086) )
	ROM_LOAD16_BYTE("d53-07.65", 0x400001, 0x100000, CRC(2f4773c3) SHA1(17cef13de0836923743b336cc5a64f7452629486) )
	ROM_LOAD       ("d53-05.47", 0x900000, 0x200000, CRC(b9b68b15) SHA1(c3783b09b22954a959188b80e537fa84d827ac47) )
	ROM_LOAD       ("d53-08.66", 0xb00000, 0x100000, CRC(ad13a1ee) SHA1(341112055b6bee33072c262f4ea7c4d0970888a6) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d53-09.48", 0x000000, 0x100000, CRC(690554d3) SHA1(113afd8fe7b77a30c2e3c5baca3f19d74902625b) )
	ROM_LOAD16_BYTE("d53-10.49", 0x000001, 0x100000, CRC(0ec05dc5) SHA1(781a6362ef963417fb6383a62dcc70d6f5b3131b) )
	ROM_LOAD       ("d53-11.50", 0x300000, 0x100000, CRC(39c0a546) SHA1(53f03586f6586032fc3b4f90e987c1128edbb0a7) )
	ROM_FILL       (             0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d53-13.10", 0x100000, 0x20000, CRC(877f0361) SHA1(eda58d71fb06f739bee1451d7aa7e7e6dee10e03) )
	ROM_LOAD16_BYTE("d53-14.23", 0x100001, 0x20000, CRC(a8664867) SHA1(dffddca469019abac33a1abe41c3fe83fbf553ce) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d53-01.2", 0x000000, 0x200000, CRC(28fd2d9b) SHA1(e08037795952a28e7a5e90437f1b9675aadfa136) )  // C8 C9 CA CB
	ROM_LOAD16_BYTE("d53-02.3", 0x400000, 0x200000, CRC(8bd4367a) SHA1(9b274fe321c4faedb7d44f7998ae2e37c6899688) )  // CC CD -std-
ROM_END

ROM_START( prmtmfgto )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d53-15.24", 0x000000, 0x40000, CRC(f24de51b) SHA1(d45d1b60901995edf0721eae7eb8c6e829f47d8d) )
	ROM_LOAD32_BYTE("d53-16.26", 0x000001, 0x40000, CRC(ffc84429) SHA1(23354c1a65853c06e5c959957a92b700b1418fd4) )
	ROM_LOAD32_BYTE("d53-18.37", 0x000002, 0x40000, CRC(ea2d6e13) SHA1(96461b73de745c4b0ac99267931106e1d5dcb664) )
	ROM_LOAD32_BYTE("d53-19.35", 0x000003, 0x40000, CRC(00e6c2f1) SHA1(cf4b9ee35be8138abfaa354d01184efbfe83cea2) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d53-03.45", 0x000000, 0x200000, CRC(91b66145) SHA1(df5bc2e544ce80a98db1fe28b4a8af8c3905c7eb) )
	ROM_LOAD16_BYTE("d53-04.46", 0x000001, 0x200000, CRC(ac3a5e80) SHA1(8a6ea8096099b465d63d56abc79ed77304fd4fa4) )
	ROM_LOAD16_BYTE("d53-06.64", 0x400000, 0x100000, CRC(f4bac410) SHA1(569bcd81d596b24add5db4a145ae04750a1bb086) )
	ROM_LOAD16_BYTE("d53-07.65", 0x400001, 0x100000, CRC(2f4773c3) SHA1(17cef13de0836923743b336cc5a64f7452629486) )
	ROM_LOAD       ("d53-05.47", 0x900000, 0x200000, CRC(b9b68b15) SHA1(c3783b09b22954a959188b80e537fa84d827ac47) )
	ROM_LOAD       ("d53-08.66", 0xb00000, 0x100000, CRC(ad13a1ee) SHA1(341112055b6bee33072c262f4ea7c4d0970888a6) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d53-09.48", 0x000000, 0x100000, CRC(690554d3) SHA1(113afd8fe7b77a30c2e3c5baca3f19d74902625b) )
	ROM_LOAD16_BYTE("d53-10.49", 0x000001, 0x100000, CRC(0ec05dc5) SHA1(781a6362ef963417fb6383a62dcc70d6f5b3131b) )
	ROM_LOAD       ("d53-11.50", 0x300000, 0x100000, CRC(39c0a546) SHA1(53f03586f6586032fc3b4f90e987c1128edbb0a7) )
	ROM_FILL       (             0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d53-13.10", 0x100000, 0x20000, CRC(877f0361) SHA1(eda58d71fb06f739bee1451d7aa7e7e6dee10e03) )
	ROM_LOAD16_BYTE("d53-14.23", 0x100001, 0x20000, CRC(a8664867) SHA1(dffddca469019abac33a1abe41c3fe83fbf553ce) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d53-01.2", 0x000000, 0x200000, CRC(28fd2d9b) SHA1(e08037795952a28e7a5e90437f1b9675aadfa136) )  // C8 C9 CA CB
	ROM_LOAD16_BYTE("d53-02.3", 0x400000, 0x200000, CRC(8bd4367a) SHA1(9b274fe321c4faedb7d44f7998ae2e37c6899688) )  // CC CD -std-
ROM_END

ROM_START( gunlock )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d66-18.ic24", 0x000000, 0x40000, CRC(8418513e) SHA1(a071647268fad08802b88fb6d795612218b7ddef) )
	ROM_LOAD32_BYTE("d66-19.ic26", 0x000001, 0x40000, CRC(95731473) SHA1(ab79821cd6098a4db84ebc9a499c29b1525510a5) )
	ROM_LOAD32_BYTE("d66-21.ic37", 0x000002, 0x40000, CRC(bd0d60f2) SHA1(609ed2b04cb9efc4b370dcbdf22fd168318989be) )
	ROM_LOAD32_BYTE("d66-24.ic35", 0x000003, 0x40000, CRC(97816378) SHA1(b22cb442b663c7a10fbc292583cd788f66f10a25) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d66-03.ic45", 0x000000, 0x100000, CRC(e7a4a491) SHA1(87837e8dd1c9a1db5e540b678233634bd52328f0) )
	ROM_LOAD16_BYTE("d66-04.ic46", 0x000001, 0x100000, CRC(c1c7aaa7) SHA1(f929516cf50d82b2d1d1b4c49a0eb1dea819aae1) )
	ROM_LOAD       ("d66-05.ic47", 0x300000, 0x100000, CRC(a3cefe04) SHA1(dd4f47a814853f4512ce25c5f25121c53ee4ada1) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d66-06.ic48", 0x000000, 0x100000, CRC(b3d8126d) SHA1(3cbb44f396973c36abdf3fdf391becb22bb6d661) )
	ROM_LOAD16_BYTE("d66-07.ic49", 0x000001, 0x100000, CRC(a6da9be7) SHA1(b528505ab925db75acf31bfbed2035cbe36e7a74) )
	ROM_LOAD       ("d66-08.ic49", 0x300000, 0x100000, CRC(9959f30b) SHA1(64bf2bf995c283c00d968e3c078b824de4084d3d) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("d66-23.ic10", 0x100000, 0x40000, CRC(57fb7c49) SHA1(f8709fd1e9ea7cee10ee2288d13339f675a7d3ae) )
	ROM_LOAD16_BYTE("d66-22.ic23", 0x100001, 0x40000, CRC(83dd7f9b) SHA1(dae21f64232d3e268f22b5e9899e0b726fdc9a9f) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d66-01.ic2", 0x000000, 0x200000, CRC(58c92efa) SHA1(bb207b35b8f9538362bb99a9ec8df206694f00ce) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d66-02.ic3", 0x400000, 0x200000, CRC(dcdafaab) SHA1(c981c7e54a2a9aaa85bb758691858495d623b029) )    // CC CD -std-
ROM_END

ROM_START( rayforce )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d66-18.ic24", 0x000000, 0x40000, CRC(8418513e) SHA1(a071647268fad08802b88fb6d795612218b7ddef) )
	ROM_LOAD32_BYTE("d66-19.ic26", 0x000001, 0x40000, CRC(95731473) SHA1(ab79821cd6098a4db84ebc9a499c29b1525510a5) )
	ROM_LOAD32_BYTE("d66-21.ic37", 0x000002, 0x40000, CRC(bd0d60f2) SHA1(609ed2b04cb9efc4b370dcbdf22fd168318989be) )
	ROM_LOAD32_BYTE("d66-25.ic35", 0x000003, 0x40000, CRC(e08653ee) SHA1(03ae4e457369a4b29cd7d52408e28725e41ee244) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d66-03.ic45", 0x000000, 0x100000, CRC(e7a4a491) SHA1(87837e8dd1c9a1db5e540b678233634bd52328f0) )
	ROM_LOAD16_BYTE("d66-04.ic46", 0x000001, 0x100000, CRC(c1c7aaa7) SHA1(f929516cf50d82b2d1d1b4c49a0eb1dea819aae1) )
	ROM_LOAD       ("d66-05.ic47", 0x300000, 0x100000, CRC(a3cefe04) SHA1(dd4f47a814853f4512ce25c5f25121c53ee4ada1) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d66-06.ic48", 0x000000, 0x100000, CRC(b3d8126d) SHA1(3cbb44f396973c36abdf3fdf391becb22bb6d661) )
	ROM_LOAD16_BYTE("d66-07.ic49", 0x000001, 0x100000, CRC(a6da9be7) SHA1(b528505ab925db75acf31bfbed2035cbe36e7a74) )
	ROM_LOAD       ("d66-08.ic49", 0x300000, 0x100000, CRC(9959f30b) SHA1(64bf2bf995c283c00d968e3c078b824de4084d3d) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("d66-23.ic10", 0x100000, 0x40000, CRC(57fb7c49) SHA1(f8709fd1e9ea7cee10ee2288d13339f675a7d3ae) )
	ROM_LOAD16_BYTE("d66-22.ic23", 0x100001, 0x40000, CRC(83dd7f9b) SHA1(dae21f64232d3e268f22b5e9899e0b726fdc9a9f) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d66-01.ic2", 0x000000, 0x200000, CRC(58c92efa) SHA1(bb207b35b8f9538362bb99a9ec8df206694f00ce) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d66-02.ic3", 0x400000, 0x200000, CRC(dcdafaab) SHA1(c981c7e54a2a9aaa85bb758691858495d623b029) )    // CC CD -std-
ROM_END

ROM_START( rayforcej )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d66-18.ic24", 0x000000, 0x40000, CRC(8418513e) SHA1(a071647268fad08802b88fb6d795612218b7ddef) )
	ROM_LOAD32_BYTE("d66-19.ic26", 0x000001, 0x40000, CRC(95731473) SHA1(ab79821cd6098a4db84ebc9a499c29b1525510a5) )
	ROM_LOAD32_BYTE("d66-21.ic37", 0x000002, 0x40000, CRC(bd0d60f2) SHA1(609ed2b04cb9efc4b370dcbdf22fd168318989be) )
	ROM_LOAD32_BYTE("d66-20.ic35", 0x000003, 0x40000, CRC(798f0254) SHA1(b070588053bddc3d0b0c2660192b0cb16bf8247f) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d66-03.ic45", 0x000000, 0x100000, CRC(e7a4a491) SHA1(87837e8dd1c9a1db5e540b678233634bd52328f0) )
	ROM_LOAD16_BYTE("d66-04.ic46", 0x000001, 0x100000, CRC(c1c7aaa7) SHA1(f929516cf50d82b2d1d1b4c49a0eb1dea819aae1) )
	ROM_LOAD       ("d66-05.ic47", 0x300000, 0x100000, CRC(a3cefe04) SHA1(dd4f47a814853f4512ce25c5f25121c53ee4ada1) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d66-06.ic48", 0x000000, 0x100000, CRC(b3d8126d) SHA1(3cbb44f396973c36abdf3fdf391becb22bb6d661) )
	ROM_LOAD16_BYTE("d66-07.ic49", 0x000001, 0x100000, CRC(a6da9be7) SHA1(b528505ab925db75acf31bfbed2035cbe36e7a74) )
	ROM_LOAD       ("d66-08.ic49", 0x300000, 0x100000, CRC(9959f30b) SHA1(64bf2bf995c283c00d968e3c078b824de4084d3d) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("d66-23.ic10", 0x100000, 0x40000, CRC(57fb7c49) SHA1(f8709fd1e9ea7cee10ee2288d13339f675a7d3ae) )
	ROM_LOAD16_BYTE("d66-22.ic23", 0x100001, 0x40000, CRC(83dd7f9b) SHA1(dae21f64232d3e268f22b5e9899e0b726fdc9a9f) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d66-01.ic2", 0x000000, 0x200000, CRC(58c92efa) SHA1(bb207b35b8f9538362bb99a9ec8df206694f00ce) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d66-02.ic3", 0x400000, 0x200000, CRC(dcdafaab) SHA1(c981c7e54a2a9aaa85bb758691858495d623b029) )    // CC CD -std-
ROM_END

ROM_START( scfinals ) /* This is the single PCB version */
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d68-09.ic40", 0x000000, 0x40000, CRC(28193b3f) SHA1(cdbc185bbfbd34a1b892bacec4695b97c50a0bb7) )
	ROM_LOAD32_BYTE("d68-10.ic38", 0x000001, 0x40000, CRC(67481bad) SHA1(97c7db7e705a2194b29c8a985702d9ccc936fd97) )
	ROM_LOAD32_BYTE("d68-11.ic36", 0x000002, 0x40000, CRC(d456c124) SHA1(8466273ee6d81808d10b2d6be92f87a062da2131) )
	ROM_LOAD32_BYTE("d68-12.ic34", 0x000003, 0x40000, CRC(dec41397) SHA1(17f41a42461c822f4b6d65e96e1ff7c20b0168c7) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d49-01.12", 0x000000, 0x200000, CRC(1dc89f1c) SHA1(9597b1d8c9b447080ca9401aee83bb4a64bb8332) ) /* Single PCB version locations may differ */
	ROM_LOAD16_BYTE("d49-02.8",  0x000001, 0x200000, CRC(1e4c374f) SHA1(512edc6a934578d0e7371410a041150d3b13aaad) )
	ROM_LOAD16_BYTE("d49-06.11", 0x400000, 0x100000, CRC(71ef4ee1) SHA1(1d7729dbc77f7201ff574e8aef65a55bd81c25a7) )
	ROM_LOAD16_BYTE("d49-07.7",  0x400001, 0x100000, CRC(e5655b8f) SHA1(2c21745370bfe9dbf0e95f7ce42ed34a162bff64) )
	ROM_LOAD       ("d49-03.4",  0x900000, 0x200000, CRC(cf9a8727) SHA1(f21787fdcdd8be2009c2d481a9b2d7fc03ce782e) )
	ROM_LOAD       ("d49-08.3",  0xb00000, 0x100000, CRC(7d3c6536) SHA1(289b4bf79ebd9cbdf64ab956784d226e6d546654) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d49-09.47", 0x000000, 0x080000, CRC(257ede01) SHA1(c36397d95706c5e68a7738c84829a51c5e8f5ef7) ) /* Single PCB version locations may differ */
	ROM_LOAD16_BYTE("d49-10.45", 0x000001, 0x080000, CRC(f587b787) SHA1(22db4904c134756ddd0f753f197419d27e60a827) )
	ROM_LOAD       ("d49-11.43", 0x180000, 0x080000, CRC(11318b26) SHA1(a7153f9f406d52189f59cbe58d65f88f4e2e6fcc) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d49-17.ic5", 0x100000, 0x20000, CRC(f2058eba) SHA1(7faaa94fadf02b6304287b61fb9613f9f4169fef) )
	ROM_LOAD16_BYTE("d49-18.ic6", 0x100001, 0x20000, CRC(a0fdd270) SHA1(9b5a2c8d35ea3bc6842e3c328447c3bf641b9237) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d49-04.38", 0x000000, 0x200000, CRC(44b365a9) SHA1(14c4a6b193a0069360406c74c500ba24f2a55b62) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d49-05.41", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "scfinals.nv", 0x0000, 0x0080, CRC(f25945fc) SHA1(9eed644767b5bd2a13f9158e81c94fd36ba63d71) )
ROM_END

ROM_START( scfinalso ) /* Cart version */
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d68-01.20", 0x000000, 0x40000, CRC(cb951856) SHA1(c7b0418b957ed0feecc9dffe5a963bd22df0ac4e) )
	ROM_LOAD32_BYTE("d68-02.19", 0x000001, 0x40000, CRC(4f94413a) SHA1(b46a35ab0150d5d5e53149c53f11978fbfa28159) )
	ROM_LOAD32_BYTE("d68-04.18", 0x000002, 0x40000, CRC(4a4e4972) SHA1(5300380a57f70fe91c69f2b1e9d25253081e61da) )
	ROM_LOAD32_BYTE("d68-03.17", 0x000003, 0x40000, CRC(a40be699) SHA1(03101d2aef8e7c0c332a3c8c0a025024f6cfe580) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d49-01.12", 0x000000, 0x200000, CRC(1dc89f1c) SHA1(9597b1d8c9b447080ca9401aee83bb4a64bb8332) )
	ROM_LOAD16_BYTE("d49-02.8",  0x000001, 0x200000, CRC(1e4c374f) SHA1(512edc6a934578d0e7371410a041150d3b13aaad) )
	ROM_LOAD16_BYTE("d49-06.11", 0x400000, 0x100000, CRC(71ef4ee1) SHA1(1d7729dbc77f7201ff574e8aef65a55bd81c25a7) )
	ROM_LOAD16_BYTE("d49-07.7",  0x400001, 0x100000, CRC(e5655b8f) SHA1(2c21745370bfe9dbf0e95f7ce42ed34a162bff64) )
	ROM_LOAD       ("d49-03.4",  0x900000, 0x200000, CRC(cf9a8727) SHA1(f21787fdcdd8be2009c2d481a9b2d7fc03ce782e) )
	ROM_LOAD       ("d49-08.3",  0xb00000, 0x100000, CRC(7d3c6536) SHA1(289b4bf79ebd9cbdf64ab956784d226e6d546654) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d49-09.47", 0x000000, 0x080000, CRC(257ede01) SHA1(c36397d95706c5e68a7738c84829a51c5e8f5ef7) )
	ROM_LOAD16_BYTE("d49-10.45", 0x000001, 0x080000, CRC(f587b787) SHA1(22db4904c134756ddd0f753f197419d27e60a827) )
	ROM_LOAD       ("d49-11.43", 0x180000, 0x080000, CRC(11318b26) SHA1(a7153f9f406d52189f59cbe58d65f88f4e2e6fcc) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d49-17.32", 0x100000, 0x20000, CRC(f2058eba) SHA1(7faaa94fadf02b6304287b61fb9613f9f4169fef) )
	ROM_LOAD16_BYTE("d49-18.33", 0x100001, 0x20000, CRC(a0fdd270) SHA1(9b5a2c8d35ea3bc6842e3c328447c3bf641b9237) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V1: 2 banks
	ROM_LOAD16_BYTE("d49-04.38", 0x000000, 0x200000, CRC(44b365a9) SHA1(14c4a6b193a0069360406c74c500ba24f2a55b62) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d49-05.41", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "scfinalso.nv", 0x0000, 0x0080, CRC(1319752e) SHA1(7d1890ebc7d4e2074d3e820e3991b5c57756ad92) )
ROM_END

ROM_START( lightbr )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d69-25.ic40", 0x000000, 0x80000, CRC(27f1b8be) SHA1(e5fc47644a000c96056e2013c42272ae5beeb98e) )
	ROM_LOAD32_BYTE("d69-26.ic38", 0x000001, 0x80000, CRC(2ff7dba6) SHA1(8757d949f44bb69fbf918852046ed0cd46ab7864) )
	ROM_LOAD32_BYTE("d69-28.ic36", 0x000002, 0x80000, CRC(a5546162) SHA1(35d9cd41f379e7fc4092c6c519b158208b977d89) )
	ROM_LOAD32_BYTE("d69-27.ic34", 0x000003, 0x80000, CRC(e232a949) SHA1(aa6969e0aa195dbae1aecc4e812c590ab4389174) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d69-06.bin", 0x000000, 0x200000, CRC(cb4aac81) SHA1(15d315c6b9695cc2fe07defc67c7a4fb26de1950) )
	ROM_LOAD16_BYTE("d69-07.bin", 0x000001, 0x200000, CRC(b749f984) SHA1(39fd662bdc42e812519181a640a83e29e300826a) )
	ROM_LOAD16_BYTE("d69-09.bin", 0x400000, 0x100000, CRC(a96c19b8) SHA1(7872b4dd9d51877bed709fec393413e41d6b954f) )
	ROM_LOAD16_BYTE("d69-10.bin", 0x400001, 0x100000, CRC(36aa80c6) SHA1(aeb5f7632810564426761b5798539bf4c4a0c64c) )
	ROM_LOAD       ("d69-08.bin", 0x900000, 0x200000, CRC(5b68d7d8) SHA1(f2ee3dd7100a3c9d8f402fe36dae2bc66cb17be3) )
	ROM_LOAD       ("d69-11.bin", 0xb00000, 0x100000, CRC(c11adf92) SHA1(ee9ce49a43b419c4f44ac1aea8d0a12d7b289244) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d69-03.bin", 0x000000, 0x200000, CRC(6999c86f) SHA1(8a91930edfc0b5d23e59f8c3b43131db6edb4d37) )
	ROM_LOAD16_BYTE("d69-04.bin", 0x000001, 0x200000, CRC(cc91dcb7) SHA1(97f510b1e1a3adf49efe82babdd7abce3756ce4b) )
	ROM_LOAD       ("d69-05.bin", 0x600000, 0x200000, CRC(f9f5433c) SHA1(d3de66385d883c72967c44bc29983d7a79f665d1) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d69-18.bin", 0x100000, 0x20000, CRC(04600d7b) SHA1(666cfab09b61fd6e0bc4ff277018ebf1cda01b0e) )
	ROM_LOAD16_BYTE("d69-19.bin", 0x100001, 0x20000, CRC(1484e853) SHA1(4459c18ba005786483c652857e527c6093efb036) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d69-01.bin", 0x000000, 0x200000, CRC(9ac93ac2) SHA1(1c44f6ba95505f85b0c8a90395f09d2a49da3553) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d69-02.bin", 0x400000, 0x200000, CRC(dce28dd7) SHA1(eacfc98349b0608fc1a944c11f0483fb6caa4445) )    // CC CD -std-
ROM_END

ROM_START( dungeonm )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d69-20.bin", 0x000000, 0x80000, CRC(33650fe4) SHA1(df8b775749b1f0f02d0df6141597cc49fb3ae227) )
	ROM_LOAD32_BYTE("d69-13.bin", 0x000001, 0x80000, CRC(dec2ec17) SHA1(8472a5aaea9e4e4fb5f7f4b5eda356b590d1541d) )
	ROM_LOAD32_BYTE("d69-15.bin", 0x000002, 0x80000, CRC(323e1955) SHA1(d76582d1ff5a9aa87a498fea3280bc3c25ee9ec0) )
	ROM_LOAD32_BYTE("d69-22.bin", 0x000003, 0x80000, CRC(f99e175d) SHA1(8f5f4710d72faed978e68e6e36703f47e8bab06f) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d69-06.bin", 0x000000, 0x200000, CRC(cb4aac81) SHA1(15d315c6b9695cc2fe07defc67c7a4fb26de1950) )
	ROM_LOAD16_BYTE("d69-07.bin", 0x000001, 0x200000, CRC(b749f984) SHA1(39fd662bdc42e812519181a640a83e29e300826a) )
	ROM_LOAD16_BYTE("d69-09.bin", 0x400000, 0x100000, CRC(a96c19b8) SHA1(7872b4dd9d51877bed709fec393413e41d6b954f) )
	ROM_LOAD16_BYTE("d69-10.bin", 0x400001, 0x100000, CRC(36aa80c6) SHA1(aeb5f7632810564426761b5798539bf4c4a0c64c) )
	ROM_LOAD       ("d69-08.bin", 0x900000, 0x200000, CRC(5b68d7d8) SHA1(f2ee3dd7100a3c9d8f402fe36dae2bc66cb17be3) )
	ROM_LOAD       ("d69-11.bin", 0xb00000, 0x100000, CRC(c11adf92) SHA1(ee9ce49a43b419c4f44ac1aea8d0a12d7b289244) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d69-03.bin", 0x000000, 0x200000, CRC(6999c86f) SHA1(8a91930edfc0b5d23e59f8c3b43131db6edb4d37) )
	ROM_LOAD16_BYTE("d69-04.bin", 0x000001, 0x200000, CRC(cc91dcb7) SHA1(97f510b1e1a3adf49efe82babdd7abce3756ce4b) )
	ROM_LOAD       ("d69-05.bin", 0x600000, 0x200000, CRC(f9f5433c) SHA1(d3de66385d883c72967c44bc29983d7a79f665d1) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d69-18.bin", 0x100000, 0x20000, CRC(04600d7b) SHA1(666cfab09b61fd6e0bc4ff277018ebf1cda01b0e) )
	ROM_LOAD16_BYTE("d69-19.bin", 0x100001, 0x20000, CRC(1484e853) SHA1(4459c18ba005786483c652857e527c6093efb036) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d69-01.bin", 0x000000, 0x200000, CRC(9ac93ac2) SHA1(1c44f6ba95505f85b0c8a90395f09d2a49da3553) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d69-02.bin", 0x400000, 0x200000, CRC(dce28dd7) SHA1(eacfc98349b0608fc1a944c11f0483fb6caa4445) )    // CC CD -std-
ROM_END

ROM_START( dungeonmu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d69-20.bin", 0x000000, 0x80000, CRC(33650fe4) SHA1(df8b775749b1f0f02d0df6141597cc49fb3ae227) )
	ROM_LOAD32_BYTE("d69-13.bin", 0x000001, 0x80000, CRC(dec2ec17) SHA1(8472a5aaea9e4e4fb5f7f4b5eda356b590d1541d) )
	ROM_LOAD32_BYTE("d69-15.bin", 0x000002, 0x80000, CRC(323e1955) SHA1(d76582d1ff5a9aa87a498fea3280bc3c25ee9ec0) )
	ROM_LOAD32_BYTE("d69-21.bin", 0x000003, 0x80000, CRC(c9d4e051) SHA1(7c7e76f0d0bca305ff6761aa509d344c2dac8e2e) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d69-06.bin", 0x000000, 0x200000, CRC(cb4aac81) SHA1(15d315c6b9695cc2fe07defc67c7a4fb26de1950) )
	ROM_LOAD16_BYTE("d69-07.bin", 0x000001, 0x200000, CRC(b749f984) SHA1(39fd662bdc42e812519181a640a83e29e300826a) )
	ROM_LOAD16_BYTE("d69-09.bin", 0x400000, 0x100000, CRC(a96c19b8) SHA1(7872b4dd9d51877bed709fec393413e41d6b954f) )
	ROM_LOAD16_BYTE("d69-10.bin", 0x400001, 0x100000, CRC(36aa80c6) SHA1(aeb5f7632810564426761b5798539bf4c4a0c64c) )
	ROM_LOAD       ("d69-08.bin", 0x900000, 0x200000, CRC(5b68d7d8) SHA1(f2ee3dd7100a3c9d8f402fe36dae2bc66cb17be3) )
	ROM_LOAD       ("d69-11.bin", 0xb00000, 0x100000, CRC(c11adf92) SHA1(ee9ce49a43b419c4f44ac1aea8d0a12d7b289244) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d69-03.bin", 0x000000, 0x200000, CRC(6999c86f) SHA1(8a91930edfc0b5d23e59f8c3b43131db6edb4d37) )
	ROM_LOAD16_BYTE("d69-04.bin", 0x000001, 0x200000, CRC(cc91dcb7) SHA1(97f510b1e1a3adf49efe82babdd7abce3756ce4b) )
	ROM_LOAD       ("d69-05.bin", 0x600000, 0x200000, CRC(f9f5433c) SHA1(d3de66385d883c72967c44bc29983d7a79f665d1) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d69-18.bin", 0x100000, 0x20000, CRC(04600d7b) SHA1(666cfab09b61fd6e0bc4ff277018ebf1cda01b0e) )
	ROM_LOAD16_BYTE("d69-19.bin", 0x100001, 0x20000, CRC(1484e853) SHA1(4459c18ba005786483c652857e527c6093efb036) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d69-01.bin", 0x000000, 0x200000, CRC(9ac93ac2) SHA1(1c44f6ba95505f85b0c8a90395f09d2a49da3553) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d69-02.bin", 0x400000, 0x200000, CRC(dce28dd7) SHA1(eacfc98349b0608fc1a944c11f0483fb6caa4445) )    // CC CD -std-
ROM_END


/*
Light Bringer
Taito, 1993

This game runs on Taito F3 hardware (single PCB)


PCB Layout
----------

F3 MAIN PCB  K1100746A  J1100318A
|------------------------------------------------------|
| TDA1543  MC33274 MB87078                D29-11       |
|26.686MHz  D49-12-1            HM511664  D29-13 D29-12|
|                     TC0630FDP                        |
|                               HM511664    68000      |
|                                                      |
|             D69-05   TC518128 HM511664 D69-19 D69-18 |
|J       2088          TC518128         TC15832 TC15832|
|        2088 D69-04            HM511664               |
|A       2088                               68681      |
|   TC0650FDA D69-03     30.47618MHz 16MHz        PAL  |
|M                                  MB8421             |
|    D69-11   D69-08  TC0660FCM             MB8421     |
|M                             D29-15                  |
|    D69-10   D69-07           D29-14       5510-ESP   |
|A                    TC51832  TC51832                 |
|    D69-09   D69-06  D69-15   D69-13   HM511664   5701|
|                                                      |
|           MC68EC020 TC51832  TC51832   D53-12 D69-02 |
|     TC0640FIO       D69-14   D69-20           D69-01 |
|                                        D29-17-1      |
|  SW1  93C46 3771                            5505-OTIS|
|------------------------------------------------------|

Notes:
      68020 clock    : 15.23809 (30.47618 / 2)
      68000 clock    : 15.23809 (30.47618 / 2)
      68681 clocks   : pin2- 500kHz, pin32- 4.000MHz, pin36- 1.000MHz, pin38- 1.000MHz, pin39- 500kHz
      5505 clock     : 15.23809MHz [30.47618/2]
      5510 ESP clocks: pin1- 8.000MHz, pins4-6- 2.6686MHz, pins9-10- 29.7623kHz, pin16- 3.8095225MHz
      VSync          : 59Hz
      HSync          : 15.97kHz

      D29* /D49* /D53* = PALs
                         D53-12         PAL16L8B
                         D29-17-1       PALCE16V8H
                         D29-14,D29-15  PALCE20V8H
                         D29-12,D29-11  PALCE20V8H
                         D29-13         PAL20L8B
                         PAL.7          PALCE16V8H
                         D49-12-1       PAL16L8B

      D69-01 to D69-08          16M MASKROM (DIP42)
      d69-09 to d69-11          8M MASKROM  (DIP42)
      D69-13 to D69-15/D69-20   27C040 EPROM (DIP32)
      D69-18/D69-19             27C1001 EPROM (DIP32)
*/

ROM_START( lightbrj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d69-20.bin", 0x000000, 0x80000, CRC(33650fe4) SHA1(df8b775749b1f0f02d0df6141597cc49fb3ae227) )
	ROM_LOAD32_BYTE("d69-13.bin", 0x000001, 0x80000, CRC(dec2ec17) SHA1(8472a5aaea9e4e4fb5f7f4b5eda356b590d1541d) )
	ROM_LOAD32_BYTE("d69-15.bin", 0x000002, 0x80000, CRC(323e1955) SHA1(d76582d1ff5a9aa87a498fea3280bc3c25ee9ec0) )
	ROM_LOAD32_BYTE("d69-14.bin", 0x000003, 0x80000, CRC(990bf945) SHA1(797794d7afc1e6e98ce1bfb3de3c241a96a8fa01) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d69-06.bin", 0x000000, 0x200000, CRC(cb4aac81) SHA1(15d315c6b9695cc2fe07defc67c7a4fb26de1950) )
	ROM_LOAD16_BYTE("d69-07.bin", 0x000001, 0x200000, CRC(b749f984) SHA1(39fd662bdc42e812519181a640a83e29e300826a) )
	ROM_LOAD16_BYTE("d69-09.bin", 0x400000, 0x100000, CRC(a96c19b8) SHA1(7872b4dd9d51877bed709fec393413e41d6b954f) )
	ROM_LOAD16_BYTE("d69-10.bin", 0x400001, 0x100000, CRC(36aa80c6) SHA1(aeb5f7632810564426761b5798539bf4c4a0c64c) )
	ROM_LOAD       ("d69-08.bin", 0x900000, 0x200000, CRC(5b68d7d8) SHA1(f2ee3dd7100a3c9d8f402fe36dae2bc66cb17be3) )
	ROM_LOAD       ("d69-11.bin", 0xb00000, 0x100000, CRC(c11adf92) SHA1(ee9ce49a43b419c4f44ac1aea8d0a12d7b289244) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d69-03.bin", 0x000000, 0x200000, CRC(6999c86f) SHA1(8a91930edfc0b5d23e59f8c3b43131db6edb4d37) )
	ROM_LOAD16_BYTE("d69-04.bin", 0x000001, 0x200000, CRC(cc91dcb7) SHA1(97f510b1e1a3adf49efe82babdd7abce3756ce4b) )
	ROM_LOAD       ("d69-05.bin", 0x600000, 0x200000, CRC(f9f5433c) SHA1(d3de66385d883c72967c44bc29983d7a79f665d1) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d69-18.bin", 0x100000, 0x20000, CRC(04600d7b) SHA1(666cfab09b61fd6e0bc4ff277018ebf1cda01b0e) )
	ROM_LOAD16_BYTE("d69-19.bin", 0x100001, 0x20000, CRC(1484e853) SHA1(4459c18ba005786483c652857e527c6093efb036) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d69-01.bin", 0x000000, 0x200000, CRC(9ac93ac2) SHA1(1c44f6ba95505f85b0c8a90395f09d2a49da3553) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d69-02.bin", 0x400000, 0x200000, CRC(dce28dd7) SHA1(eacfc98349b0608fc1a944c11f0483fb6caa4445) )    // CC CD -std-
ROM_END

ROM_START( intcup94 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d78-07.20", 0x000000, 0x20000, CRC(8525d990) SHA1(b28aeb8727d615cae9eafd7710bf833a612ef7d4) )
	ROM_LOAD32_BYTE("d78-06.19", 0x000001, 0x20000, CRC(42db1d41) SHA1(daf617764b04cd24e76dfa95423213c2a3692068) )
	ROM_LOAD32_BYTE("d78-05.18", 0x000002, 0x20000, CRC(5f7fbbbc) SHA1(8936bcc4026b2819b8708911c9defe4436d070ad) )
	ROM_LOAD32_BYTE("d78-11.17", 0x000003, 0x20000, CRC(bb9d2987) SHA1(98bea0346702eefd9f6f1839b95932b9b8bca902) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d49-01.12", 0x000000, 0x200000, CRC(1dc89f1c) SHA1(9597b1d8c9b447080ca9401aee83bb4a64bb8332) )
	ROM_LOAD16_BYTE("d49-02.8",  0x000001, 0x200000, CRC(1e4c374f) SHA1(512edc6a934578d0e7371410a041150d3b13aaad) )
	ROM_LOAD16_BYTE("d49-06.11", 0x400000, 0x100000, CRC(71ef4ee1) SHA1(1d7729dbc77f7201ff574e8aef65a55bd81c25a7) )
	ROM_LOAD16_BYTE("d49-07.7",  0x400001, 0x100000, CRC(e5655b8f) SHA1(2c21745370bfe9dbf0e95f7ce42ed34a162bff64) )
	ROM_LOAD       ("d49-03.4",  0x900000, 0x200000, CRC(cf9a8727) SHA1(f21787fdcdd8be2009c2d481a9b2d7fc03ce782e) )
	ROM_LOAD       ("d49-08.3",  0xb00000, 0x100000, CRC(7d3c6536) SHA1(289b4bf79ebd9cbdf64ab956784d226e6d546654) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d78-01.47", 0x000000, 0x080000, CRC(543f8967) SHA1(2efa935e7d0fd317bbbad2758a618d408a56317c) )
	ROM_LOAD16_BYTE("d78-02.45", 0x000001, 0x080000, CRC(e8289394) SHA1(b9957675f868f772943678b6a19fcc21dfd97a8d) )
	ROM_LOAD       ("d78-03.43", 0x180000, 0x080000, CRC(a8bc36e5) SHA1(5777b9457292e8a9cbb4e8226ba939530ffab07c) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d78-08.32", 0x100000, 0x20000, CRC(a629d07c) SHA1(b2904e106633a3960ceb2bc58b600ea60034ff0b) )
	ROM_LOAD16_BYTE("d78-09.33", 0x100001, 0x20000, CRC(1f0efe01) SHA1(7bff748b9fcee170e430d90ee07eb9975d8fba59) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d49-04.38", 0x000000, 0x200000, CRC(44b365a9) SHA1(14c4a6b193a0069360406c74c500ba24f2a55b62) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d49-05.41", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( hthero94 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d78-07.20", 0x000000, 0x20000, CRC(8525d990) SHA1(b28aeb8727d615cae9eafd7710bf833a612ef7d4) )
	ROM_LOAD32_BYTE("d78-06.19", 0x000001, 0x20000, CRC(42db1d41) SHA1(daf617764b04cd24e76dfa95423213c2a3692068) )
	ROM_LOAD32_BYTE("d78-05.18", 0x000002, 0x20000, CRC(5f7fbbbc) SHA1(8936bcc4026b2819b8708911c9defe4436d070ad) )
	ROM_LOAD32_BYTE("d78-10.17", 0x000003, 0x20000, CRC(cc9a1911) SHA1(341b6c33b182e3a64a22f1dc43e9cf72c6aeea7b) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d49-01.12", 0x000000, 0x200000, CRC(1dc89f1c) SHA1(9597b1d8c9b447080ca9401aee83bb4a64bb8332) )
	ROM_LOAD16_BYTE("d49-02.8",  0x000001, 0x200000, CRC(1e4c374f) SHA1(512edc6a934578d0e7371410a041150d3b13aaad) )
	ROM_LOAD16_BYTE("d49-06.11", 0x400000, 0x100000, CRC(71ef4ee1) SHA1(1d7729dbc77f7201ff574e8aef65a55bd81c25a7) )
	ROM_LOAD16_BYTE("d49-07.7",  0x400001, 0x100000, CRC(e5655b8f) SHA1(2c21745370bfe9dbf0e95f7ce42ed34a162bff64) )
	ROM_LOAD       ("d49-03.4",  0x900000, 0x200000, CRC(cf9a8727) SHA1(f21787fdcdd8be2009c2d481a9b2d7fc03ce782e) )
	ROM_LOAD       ("d49-08.3",  0xb00000, 0x100000, CRC(7d3c6536) SHA1(289b4bf79ebd9cbdf64ab956784d226e6d546654) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d78-01.47", 0x000000, 0x080000, CRC(543f8967) SHA1(2efa935e7d0fd317bbbad2758a618d408a56317c) )
	ROM_LOAD16_BYTE("d78-02.45", 0x000001, 0x080000, CRC(e8289394) SHA1(b9957675f868f772943678b6a19fcc21dfd97a8d) )
	ROM_LOAD       ("d78-03.43", 0x180000, 0x080000, CRC(a8bc36e5) SHA1(5777b9457292e8a9cbb4e8226ba939530ffab07c) )
	ROM_FILL       (             0x100000, 0x080000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d78-08.32", 0x100000, 0x20000, CRC(a629d07c) SHA1(b2904e106633a3960ceb2bc58b600ea60034ff0b) )
	ROM_LOAD16_BYTE("d78-09.33", 0x100001, 0x20000, CRC(1f0efe01) SHA1(7bff748b9fcee170e430d90ee07eb9975d8fba59) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d49-04.38", 0x000000, 0x200000, CRC(44b365a9) SHA1(14c4a6b193a0069360406c74c500ba24f2a55b62) ) // C8 C9 CA CB
	// half empty
	ROM_LOAD16_BYTE("d49-05.41", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) ) // -std-
ROM_END

ROM_START( recalh )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("rh_mpr3.bin", 0x000000, 0x80000, CRC(65202dd4) SHA1(8d5748d03868b127a7d727e00c1bce51a5bae129) )
	ROM_LOAD32_BYTE("rh_mpr2.bin", 0x000001, 0x80000, CRC(3eda66db) SHA1(6d726762404d85008d6bebe5a77cebe505b650fc) )
	ROM_LOAD32_BYTE("rh_mpr1.bin", 0x000002, 0x80000, CRC(536e74ca) SHA1(2a50bb2e93563273c4b0c0c59143893fe25d007e) )
	ROM_LOAD32_BYTE("rh_mpr0.bin", 0x000003, 0x80000, CRC(38025817) SHA1(fa4cf98cfca95c462b19b873a7660f7cec71cf56) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("rh_objl.bin", 0x000000, 0x100000, CRC(c1772b55) SHA1(f9a04b968c63e61fa8ca60d6f331f6df0d7dd10a) )
	ROM_LOAD16_BYTE("rh_objm.bin", 0x000001, 0x100000, CRC(ef87c0fd) SHA1(63e99f331d05a1ff4faf0ea94019393fe2117f54) )
	ROM_FILL       (               0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("rh_scrl.bin", 0x000000, 0x100000, CRC(1e3f6b79) SHA1(fef029def6393a13f4a638686a7ec7c13851a5c0) )
	ROM_LOAD16_BYTE("rh_scrm.bin", 0x000001, 0x100000, CRC(37200968) SHA1(4a8d5a17af7eb732f481bf174099845e8d8d6b87) )
	ROM_FILL       (               0x200000, 0x200000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("rh_spr1.bin", 0x100000, 0x20000, CRC(504cbc1d) SHA1(35a775c1ebc8107c553e43b9d84eb735446c26fd) )
	ROM_LOAD16_BYTE("rh_spr0.bin", 0x100001, 0x20000, CRC(78fba467) SHA1(4586b061724be7ec413784b820c33cc0d6bbcd0c) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("rh_snd0.bin", 0x000000, 0x200000, CRC(386f5e1b) SHA1(d67d5f057c6db3092643f10ea10f977b1caa662f) )   // C8 CB CA C9
	// half empty
	ROM_LOAD16_BYTE("rh_snd1.bin", 0x600000, 0x100000, CRC(ed894fe1) SHA1(5bf2fb6abdcf25bc525a2c3b29dbf7aca0b18fea) )   // -std-
ROM_END

ROM_START( kaiserkn )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d84-25.20", 0x000000, 0x80000, CRC(2840893f) SHA1(079dece4667b029189622476cc618b88e57243a6) )
	ROM_LOAD32_BYTE("d84-24.19", 0x000001, 0x80000, CRC(bf20c755) SHA1(9f6edfe9bb40051e8a93d06a391c993ed7288db6) )
	ROM_LOAD32_BYTE("d84-23.18", 0x000002, 0x80000, CRC(39f12a9b) SHA1(4b3fe9b8b0abb46feacd11ffb6b505568f892483) )
	ROM_LOAD32_BYTE("d84-29.17", 0x000003, 0x80000, CRC(9821f17a) SHA1(4a2c1ebeb1a1d3d756957956c883f8374aaf4f8d) )

	ROM_REGION(0x1a00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d84-03.rom", 0x0000000, 0x200000, CRC(d786f552) SHA1(f73146892f714b5706d568fc8a135fddaa656570) )
	ROM_LOAD16_BYTE("d84-04.rom", 0x0000001, 0x200000, CRC(d1f32b5d) SHA1(35289cce64fdbb8d966dd1d5307b5393be5e7799) )
	ROM_LOAD16_BYTE("d84-06.rom", 0x0400000, 0x200000, CRC(fa924dab) SHA1(28a8c3cd701f8df0c53069bb576bb2a820f3a331) )
	ROM_LOAD16_BYTE("d84-07.rom", 0x0400001, 0x200000, CRC(54517a6b) SHA1(6e2c213c7ec1a3b78ad7e71db2326602557fd0f8) )
	ROM_LOAD16_BYTE("d84-09.rom", 0x0800000, 0x200000, CRC(faa78d98) SHA1(da3a2c5a45dd169743f113aa08e574f732e1f0fd) )
	ROM_LOAD16_BYTE("d84-10.rom", 0x0800001, 0x200000, CRC(b84b7320) SHA1(f5de0d6da50d8ed753607b51e46bc9a4572ef431) )
	ROM_LOAD16_BYTE("d84-19.rom", 0x0c00000, 0x080000, CRC(6ddf77e5) SHA1(a1323acaed37fce62a19e63a0800d9d1dc2cfff7) )
	ROM_LOAD16_BYTE("d84-20.rom", 0x0c00001, 0x080000, CRC(f85041e5) SHA1(6b2814514338f550d6aa14dbe39e848e8e64edee) )
	ROM_LOAD       ("d84-05.rom", 0x1380000, 0x200000, CRC(31a3c75d) SHA1(1a16ccb6a0a03ab715e5b016ab3b1b2cd0f1ae41) )
	ROM_LOAD       ("d84-08.rom", 0x1580000, 0x200000, CRC(07347bf1) SHA1(34bd359933acdec7fd1ce047092a30d1177afc2c) )
	ROM_LOAD       ("d84-11.rom", 0x1780000, 0x200000, CRC(a062c1d4) SHA1(158912aa3dd75c3961bf738f9ac9034f0b005b60) )
	ROM_LOAD       ("d84-21.rom", 0x1980000, 0x080000, CRC(89f68b66) SHA1(95916f02f71357324effe59da4f847f2f30ea34a) )
	ROM_FILL       (              0x0d00000, 0x680000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d84-12.rom", 0x000000, 0x200000, CRC(66a7a9aa) SHA1(a7d21f8b6370d16de3c1569019f2ad71d36e7a61) )
	ROM_LOAD16_BYTE("d84-13.rom", 0x000001, 0x200000, CRC(ae125516) SHA1(d54e76e398ab0b0fb82f3154ba54fc823ff49a1a) )
	ROM_LOAD16_BYTE("d84-16.rom", 0x400000, 0x100000, CRC(bcff9b2d) SHA1(0ca50ec809564eddf0ba7448a8fae9087d3b600b) )
	ROM_LOAD16_BYTE("d84-17.rom", 0x400001, 0x100000, CRC(0be37cc3) SHA1(b10c10b93858cad0c962ef614cfd6daea712ef6b) )
	ROM_LOAD       ("d84-14.rom", 0x900000, 0x200000, CRC(2b2e693e) SHA1(03eb37fa7dc68d54bf0f1800b8c0b581c344a40f) )
	ROM_LOAD       ("d84-18.rom", 0xb00000, 0x100000, CRC(e812bcc5) SHA1(3574e4a99232d9fc7989ec5d1e8fe76b4b30784a) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d84-26.32", 0x100000, 0x40000, CRC(4f5b8563) SHA1(1d4e06cbea7bc73a99d6e30be714fff420151bbc) )
	ROM_LOAD16_BYTE("d84-27.33", 0x100001, 0x40000, CRC(fb0cb1ba) SHA1(16a79b53651a6131f7636db19738b456b7c28bff) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("d84-01.rom", 0x400000, 0x200000, CRC(9ad22149) SHA1(48055822e0cea228cdecf3d05ac24e50979b6f4d) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d84-02.rom", 0x800000, 0x200000, CRC(9e1827e4) SHA1(1840881b0f8f7b6225e6ffa12a8d4b463554988e) )    // CC CD CE CF
	// half empty
	ROM_LOAD16_BYTE("d84-15.rom", 0xe00000, 0x100000, CRC(31ceb152) SHA1(d9d0bc631a6a726376f566a49605b50485ac7bf4) )    // D0 D1
ROM_END

ROM_START( kaiserknj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d84-25.20", 0x000000, 0x80000, CRC(2840893f) SHA1(079dece4667b029189622476cc618b88e57243a6) )
	ROM_LOAD32_BYTE("d84-24.19", 0x000001, 0x80000, CRC(bf20c755) SHA1(9f6edfe9bb40051e8a93d06a391c993ed7288db6) )
	ROM_LOAD32_BYTE("d84-23.18", 0x000002, 0x80000, CRC(39f12a9b) SHA1(4b3fe9b8b0abb46feacd11ffb6b505568f892483) )
	ROM_LOAD32_BYTE("d84-22.17", 0x000003, 0x80000, CRC(762f9056) SHA1(c39854d865210d05fe745493098ef5990327c56e) )

	ROM_REGION(0x1a00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d84-03.rom", 0x0000000, 0x200000, CRC(d786f552) SHA1(f73146892f714b5706d568fc8a135fddaa656570) )
	ROM_LOAD16_BYTE("d84-04.rom", 0x0000001, 0x200000, CRC(d1f32b5d) SHA1(35289cce64fdbb8d966dd1d5307b5393be5e7799) )
	ROM_LOAD16_BYTE("d84-06.rom", 0x0400000, 0x200000, CRC(fa924dab) SHA1(28a8c3cd701f8df0c53069bb576bb2a820f3a331) )
	ROM_LOAD16_BYTE("d84-07.rom", 0x0400001, 0x200000, CRC(54517a6b) SHA1(6e2c213c7ec1a3b78ad7e71db2326602557fd0f8) )
	ROM_LOAD16_BYTE("d84-09.rom", 0x0800000, 0x200000, CRC(faa78d98) SHA1(da3a2c5a45dd169743f113aa08e574f732e1f0fd) )
	ROM_LOAD16_BYTE("d84-10.rom", 0x0800001, 0x200000, CRC(b84b7320) SHA1(f5de0d6da50d8ed753607b51e46bc9a4572ef431) )
	ROM_LOAD16_BYTE("d84-19.rom", 0x0c00000, 0x080000, CRC(6ddf77e5) SHA1(a1323acaed37fce62a19e63a0800d9d1dc2cfff7) )
	ROM_LOAD16_BYTE("d84-20.rom", 0x0c00001, 0x080000, CRC(f85041e5) SHA1(6b2814514338f550d6aa14dbe39e848e8e64edee) )
	ROM_LOAD       ("d84-05.rom", 0x1380000, 0x200000, CRC(31a3c75d) SHA1(1a16ccb6a0a03ab715e5b016ab3b1b2cd0f1ae41) )
	ROM_LOAD       ("d84-08.rom", 0x1580000, 0x200000, CRC(07347bf1) SHA1(34bd359933acdec7fd1ce047092a30d1177afc2c) )
	ROM_LOAD       ("d84-11.rom", 0x1780000, 0x200000, CRC(a062c1d4) SHA1(158912aa3dd75c3961bf738f9ac9034f0b005b60) )
	ROM_LOAD       ("d84-21.rom", 0x1980000, 0x080000, CRC(89f68b66) SHA1(95916f02f71357324effe59da4f847f2f30ea34a) )
	ROM_FILL       (              0x0d00000, 0x680000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d84-12.rom", 0x000000, 0x200000, CRC(66a7a9aa) SHA1(a7d21f8b6370d16de3c1569019f2ad71d36e7a61) )
	ROM_LOAD16_BYTE("d84-13.rom", 0x000001, 0x200000, CRC(ae125516) SHA1(d54e76e398ab0b0fb82f3154ba54fc823ff49a1a) )
	ROM_LOAD16_BYTE("d84-16.rom", 0x400000, 0x100000, CRC(bcff9b2d) SHA1(0ca50ec809564eddf0ba7448a8fae9087d3b600b) )
	ROM_LOAD16_BYTE("d84-17.rom", 0x400001, 0x100000, CRC(0be37cc3) SHA1(b10c10b93858cad0c962ef614cfd6daea712ef6b) )
	ROM_LOAD       ("d84-14.rom", 0x900000, 0x200000, CRC(2b2e693e) SHA1(03eb37fa7dc68d54bf0f1800b8c0b581c344a40f) )
	ROM_LOAD       ("d84-18.rom", 0xb00000, 0x100000, CRC(e812bcc5) SHA1(3574e4a99232d9fc7989ec5d1e8fe76b4b30784a) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d84-26.32", 0x100000, 0x40000, CRC(4f5b8563) SHA1(1d4e06cbea7bc73a99d6e30be714fff420151bbc) )
	ROM_LOAD16_BYTE("d84-27.33", 0x100001, 0x40000, CRC(fb0cb1ba) SHA1(16a79b53651a6131f7636db19738b456b7c28bff) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("d84-01.rom", 0x400000, 0x200000, CRC(9ad22149) SHA1(48055822e0cea228cdecf3d05ac24e50979b6f4d) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d84-02.rom", 0x800000, 0x200000, CRC(9e1827e4) SHA1(1840881b0f8f7b6225e6ffa12a8d4b463554988e) )    // CC CD CE CF
	// half empty
	ROM_LOAD16_BYTE("d84-15.rom", 0xe00000, 0x100000, CRC(31ceb152) SHA1(d9d0bc631a6a726376f566a49605b50485ac7bf4) )    // D0 D1
ROM_END

ROM_START( gblchmp )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d84-25.20", 0x000000, 0x80000, CRC(2840893f) SHA1(079dece4667b029189622476cc618b88e57243a6) )
	ROM_LOAD32_BYTE("d84-24.19", 0x000001, 0x80000, CRC(bf20c755) SHA1(9f6edfe9bb40051e8a93d06a391c993ed7288db6) )
	ROM_LOAD32_BYTE("d84-23.18", 0x000002, 0x80000, CRC(39f12a9b) SHA1(4b3fe9b8b0abb46feacd11ffb6b505568f892483) )
	ROM_LOAD32_BYTE("d84-28.17", 0x000003, 0x80000, CRC(ef26c1ec) SHA1(99440573704252b59148b3c30a006ce152b30ada) )

	ROM_REGION(0x1a00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d84-03.rom", 0x0000000, 0x200000, CRC(d786f552) SHA1(f73146892f714b5706d568fc8a135fddaa656570) )
	ROM_LOAD16_BYTE("d84-04.rom", 0x0000001, 0x200000, CRC(d1f32b5d) SHA1(35289cce64fdbb8d966dd1d5307b5393be5e7799) )
	ROM_LOAD16_BYTE("d84-06.rom", 0x0400000, 0x200000, CRC(fa924dab) SHA1(28a8c3cd701f8df0c53069bb576bb2a820f3a331) )
	ROM_LOAD16_BYTE("d84-07.rom", 0x0400001, 0x200000, CRC(54517a6b) SHA1(6e2c213c7ec1a3b78ad7e71db2326602557fd0f8) )
	ROM_LOAD16_BYTE("d84-09.rom", 0x0800000, 0x200000, CRC(faa78d98) SHA1(da3a2c5a45dd169743f113aa08e574f732e1f0fd) )
	ROM_LOAD16_BYTE("d84-10.rom", 0x0800001, 0x200000, CRC(b84b7320) SHA1(f5de0d6da50d8ed753607b51e46bc9a4572ef431) )
	ROM_LOAD16_BYTE("d84-19.rom", 0x0c00000, 0x080000, CRC(6ddf77e5) SHA1(a1323acaed37fce62a19e63a0800d9d1dc2cfff7) )
	ROM_LOAD16_BYTE("d84-20.rom", 0x0c00001, 0x080000, CRC(f85041e5) SHA1(6b2814514338f550d6aa14dbe39e848e8e64edee) )
	ROM_LOAD       ("d84-05.rom", 0x1380000, 0x200000, CRC(31a3c75d) SHA1(1a16ccb6a0a03ab715e5b016ab3b1b2cd0f1ae41) )
	ROM_LOAD       ("d84-08.rom", 0x1580000, 0x200000, CRC(07347bf1) SHA1(34bd359933acdec7fd1ce047092a30d1177afc2c) )
	ROM_LOAD       ("d84-11.rom", 0x1780000, 0x200000, CRC(a062c1d4) SHA1(158912aa3dd75c3961bf738f9ac9034f0b005b60) )
	ROM_LOAD       ("d84-21.rom", 0x1980000, 0x080000, CRC(89f68b66) SHA1(95916f02f71357324effe59da4f847f2f30ea34a) )
	ROM_FILL       (              0x0d00000, 0x680000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d84-12.rom", 0x000000, 0x200000, CRC(66a7a9aa) SHA1(a7d21f8b6370d16de3c1569019f2ad71d36e7a61) )
	ROM_LOAD16_BYTE("d84-13.rom", 0x000001, 0x200000, CRC(ae125516) SHA1(d54e76e398ab0b0fb82f3154ba54fc823ff49a1a) )
	ROM_LOAD16_BYTE("d84-16.rom", 0x400000, 0x100000, CRC(bcff9b2d) SHA1(0ca50ec809564eddf0ba7448a8fae9087d3b600b) )
	ROM_LOAD16_BYTE("d84-17.rom", 0x400001, 0x100000, CRC(0be37cc3) SHA1(b10c10b93858cad0c962ef614cfd6daea712ef6b) )
	ROM_LOAD       ("d84-14.rom", 0x900000, 0x200000, CRC(2b2e693e) SHA1(03eb37fa7dc68d54bf0f1800b8c0b581c344a40f) )
	ROM_LOAD       ("d84-18.rom", 0xb00000, 0x100000, CRC(e812bcc5) SHA1(3574e4a99232d9fc7989ec5d1e8fe76b4b30784a) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d84-26.32", 0x100000, 0x40000, CRC(4f5b8563) SHA1(1d4e06cbea7bc73a99d6e30be714fff420151bbc) )
	ROM_LOAD16_BYTE("d84-27.33", 0x100001, 0x40000, CRC(fb0cb1ba) SHA1(16a79b53651a6131f7636db19738b456b7c28bff) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("d84-01.rom", 0x400000, 0x200000, CRC(9ad22149) SHA1(48055822e0cea228cdecf3d05ac24e50979b6f4d) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d84-02.rom", 0x800000, 0x200000, CRC(9e1827e4) SHA1(1840881b0f8f7b6225e6ffa12a8d4b463554988e) )    // CC CD CE CF
	// half empty
	ROM_LOAD16_BYTE("d84-15.rom", 0xe00000, 0x100000, CRC(31ceb152) SHA1(d9d0bc631a6a726376f566a49605b50485ac7bf4) )    // D0 D1
ROM_END

ROM_START( dankuga )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("dkg_mpr3.20", 0x000000, 0x80000, CRC(ee1531ca) SHA1(5a78f44906c77a3195cbb41d256292255275643f) )
	ROM_LOAD32_BYTE("dkg_mpr2.19", 0x000001, 0x80000, CRC(18a4748b) SHA1(31b912b532329d2cbd43df44f21e0923af7157d5) )
	ROM_LOAD32_BYTE("dkg_mpr1.18", 0x000002, 0x80000, CRC(97566f69) SHA1(2f1ae6b9a463f20beea1558278741ddfe3901a6d) )
	ROM_LOAD32_BYTE("dkg_mpr0.17", 0x000003, 0x80000, CRC(ad6ada07) SHA1(124db0cf8a5fbd99525633a2f783a0e1b281badf) )

	ROM_REGION(0x1a00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d84-03.rom", 0x0000000, 0x200000, CRC(d786f552) SHA1(f73146892f714b5706d568fc8a135fddaa656570) )
	ROM_LOAD16_BYTE("d84-04.rom", 0x0000001, 0x200000, CRC(d1f32b5d) SHA1(35289cce64fdbb8d966dd1d5307b5393be5e7799) )
	ROM_LOAD16_BYTE("d84-06.rom", 0x0400000, 0x200000, CRC(fa924dab) SHA1(28a8c3cd701f8df0c53069bb576bb2a820f3a331) )
	ROM_LOAD16_BYTE("d84-07.rom", 0x0400001, 0x200000, CRC(54517a6b) SHA1(6e2c213c7ec1a3b78ad7e71db2326602557fd0f8) )
	ROM_LOAD16_BYTE("d84-09.rom", 0x0800000, 0x200000, CRC(faa78d98) SHA1(da3a2c5a45dd169743f113aa08e574f732e1f0fd) )
	ROM_LOAD16_BYTE("d84-10.rom", 0x0800001, 0x200000, CRC(b84b7320) SHA1(f5de0d6da50d8ed753607b51e46bc9a4572ef431) )
	ROM_LOAD16_BYTE("d84-19.rom", 0x0c00000, 0x080000, CRC(6ddf77e5) SHA1(a1323acaed37fce62a19e63a0800d9d1dc2cfff7) )
	ROM_LOAD16_BYTE("d84-20.rom", 0x0c00001, 0x080000, CRC(f85041e5) SHA1(6b2814514338f550d6aa14dbe39e848e8e64edee) )
	ROM_LOAD       ("d84-05.rom", 0x1380000, 0x200000, CRC(31a3c75d) SHA1(1a16ccb6a0a03ab715e5b016ab3b1b2cd0f1ae41) )
	ROM_LOAD       ("d84-08.rom", 0x1580000, 0x200000, CRC(07347bf1) SHA1(34bd359933acdec7fd1ce047092a30d1177afc2c) )
	ROM_LOAD       ("d84-11.rom", 0x1780000, 0x200000, CRC(a062c1d4) SHA1(158912aa3dd75c3961bf738f9ac9034f0b005b60) )
	ROM_LOAD       ("d84-21.rom", 0x1980000, 0x080000, CRC(89f68b66) SHA1(95916f02f71357324effe59da4f847f2f30ea34a) )
	ROM_FILL       (              0x0d00000, 0x680000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d84-12.rom", 0x000000, 0x200000, CRC(66a7a9aa) SHA1(a7d21f8b6370d16de3c1569019f2ad71d36e7a61) )
	ROM_LOAD16_BYTE("d84-13.rom", 0x000001, 0x200000, CRC(ae125516) SHA1(d54e76e398ab0b0fb82f3154ba54fc823ff49a1a) )
	ROM_LOAD16_BYTE("d84-16.rom", 0x400000, 0x100000, CRC(bcff9b2d) SHA1(0ca50ec809564eddf0ba7448a8fae9087d3b600b) )
	ROM_LOAD16_BYTE("d84-17.rom", 0x400001, 0x100000, CRC(0be37cc3) SHA1(b10c10b93858cad0c962ef614cfd6daea712ef6b) )
	ROM_LOAD       ("d84-14.rom", 0x900000, 0x200000, CRC(2b2e693e) SHA1(03eb37fa7dc68d54bf0f1800b8c0b581c344a40f) )
	ROM_LOAD       ("d84-18.rom", 0xb00000, 0x100000, CRC(e812bcc5) SHA1(3574e4a99232d9fc7989ec5d1e8fe76b4b30784a) )
	ROM_FILL       (              0x600000, 0x300000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d84-26.32", 0x100000, 0x40000, CRC(4f5b8563) SHA1(1d4e06cbea7bc73a99d6e30be714fff420151bbc) )
	ROM_LOAD16_BYTE("d84-27.33", 0x100001, 0x40000, CRC(fb0cb1ba) SHA1(16a79b53651a6131f7636db19738b456b7c28bff) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("d84-01.rom", 0x400000, 0x200000, CRC(9ad22149) SHA1(48055822e0cea228cdecf3d05ac24e50979b6f4d) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d84-02.rom", 0x800000, 0x200000, CRC(9e1827e4) SHA1(1840881b0f8f7b6225e6ffa12a8d4b463554988e) )    // CC CD CE CF
	// half empty
	ROM_LOAD16_BYTE("d84-15.rom", 0xe00000, 0x100000, CRC(31ceb152) SHA1(d9d0bc631a6a726376f566a49605b50485ac7bf4) )    // D0 D1
ROM_END

ROM_START( dariusg )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d87-12.bin", 0x000000, 0x80000, CRC(de78f328) SHA1(126464a826685f5bfab6cc099448ce4df207a407) )
	ROM_LOAD32_BYTE("d87-11.bin", 0x000001, 0x80000, CRC(f7bed18e) SHA1(db7d92680f9f406a5295ee85ce110c1a56ed386f) )
	ROM_LOAD32_BYTE("d87-10.bin", 0x000002, 0x80000, CRC(4149f66f) SHA1(57d36a62d490d9e53b6b80a92ea0e8c41d61799f) )
	ROM_LOAD32_BYTE("d87-16.bin", 0x000003, 0x80000, CRC(8f7e5901) SHA1(b920f43374af30e2f7d7d01049af6746206c8ece) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d87-03.bin", 0x000000, 0x200000, CRC(4be1666e) SHA1(35ba7bcf29ec7a8f8b6944ee3544693d4df1bfc2) )
	ROM_LOAD16_BYTE("d87-04.bin", 0x000001, 0x200000, CRC(2616002c) SHA1(003f98b740a697274385b8da03c78f3c6f7b5e89) )
	ROM_LOAD       ("d87-05.bin", 0x600000, 0x200000, CRC(4e5891a9) SHA1(fd08d848079841c9237fa359a850980fd00114d8) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d87-06.bin", 0x000000, 0x200000, CRC(3b97a07c) SHA1(72cdeffedeab0c1bd0e47f03172085390a2be393) )
	ROM_LOAD16_BYTE("d87-17.bin", 0x000001, 0x200000, CRC(e601d63e) SHA1(256a6aeb5633fe1db407fad567169a9d0c911219) )
	ROM_LOAD       ("d87-08.bin", 0x600000, 0x200000, CRC(76d23602) SHA1(ca53ea6641182c44a4038bbeaa5effb1687f1980) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d87-13.bin", 0x100000, 0x40000, CRC(15b1fff4) SHA1(28692b731ae98a47c2c5e11a8a71b61a813d9a64) )
	ROM_LOAD16_BYTE("d87-14.bin", 0x100001, 0x40000, CRC(eecda29a) SHA1(6eb238e47bc7bf635ffbdbb25fb06a37db980ef8) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d87-01.bin", 0x000000, 0x200000, CRC(3848a110) SHA1(802e91695a526f665c7fd261f0a7639a0b883c9e) )    // C9 CA CB CC
	ROM_LOAD16_BYTE("d87-02.bin", 0x400000, 0x200000, CRC(9250abae) SHA1(07cae8edbc3cca0a95022d9b40a5c18a55350b67) )    // CD CE CF D0
ROM_END

ROM_START( dariusgj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d87-12.bin", 0x000000, 0x80000, CRC(de78f328) SHA1(126464a826685f5bfab6cc099448ce4df207a407) )
	ROM_LOAD32_BYTE("d87-11.bin", 0x000001, 0x80000, CRC(f7bed18e) SHA1(db7d92680f9f406a5295ee85ce110c1a56ed386f) )
	ROM_LOAD32_BYTE("d87-10.bin", 0x000002, 0x80000, CRC(4149f66f) SHA1(57d36a62d490d9e53b6b80a92ea0e8c41d61799f) )
	ROM_LOAD32_BYTE("d87-09.bin", 0x000003, 0x80000, CRC(6170382d) SHA1(85b0f9a3400884e1c073d5bdcdf7318377650eed) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d87-03.bin", 0x000000, 0x200000, CRC(4be1666e) SHA1(35ba7bcf29ec7a8f8b6944ee3544693d4df1bfc2) )
	ROM_LOAD16_BYTE("d87-04.bin", 0x000001, 0x200000, CRC(2616002c) SHA1(003f98b740a697274385b8da03c78f3c6f7b5e89) )
	ROM_LOAD       ("d87-05.bin", 0x600000, 0x200000, CRC(4e5891a9) SHA1(fd08d848079841c9237fa359a850980fd00114d8) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d87-06.bin", 0x000000, 0x200000, CRC(3b97a07c) SHA1(72cdeffedeab0c1bd0e47f03172085390a2be393) )
	ROM_LOAD16_BYTE("d87-17.bin", 0x000001, 0x200000, CRC(e601d63e) SHA1(256a6aeb5633fe1db407fad567169a9d0c911219) )
	ROM_LOAD       ("d87-08.bin", 0x600000, 0x200000, CRC(76d23602) SHA1(ca53ea6641182c44a4038bbeaa5effb1687f1980) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d87-13.bin", 0x100000, 0x40000, CRC(15b1fff4) SHA1(28692b731ae98a47c2c5e11a8a71b61a813d9a64) )
	ROM_LOAD16_BYTE("d87-14.bin", 0x100001, 0x40000, CRC(eecda29a) SHA1(6eb238e47bc7bf635ffbdbb25fb06a37db980ef8) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d87-01.bin", 0x000000, 0x200000, CRC(3848a110) SHA1(802e91695a526f665c7fd261f0a7639a0b883c9e) )    // C9 CA CB CC
	ROM_LOAD16_BYTE("d87-02.bin", 0x400000, 0x200000, CRC(9250abae) SHA1(07cae8edbc3cca0a95022d9b40a5c18a55350b67) )    // CD CE CF D0
ROM_END

ROM_START( dariusgu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d87-12.bin", 0x000000, 0x80000, CRC(de78f328) SHA1(126464a826685f5bfab6cc099448ce4df207a407) )
	ROM_LOAD32_BYTE("d87-11.bin", 0x000001, 0x80000, CRC(f7bed18e) SHA1(db7d92680f9f406a5295ee85ce110c1a56ed386f) )
	ROM_LOAD32_BYTE("d87-10.bin", 0x000002, 0x80000, CRC(4149f66f) SHA1(57d36a62d490d9e53b6b80a92ea0e8c41d61799f) )
	ROM_LOAD32_BYTE("d87-15.bin", 0x000003, 0x80000, CRC(f8796997) SHA1(fa286561bac9894cb260944ffa14d0059b882ab9) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d87-03.bin", 0x000000, 0x200000, CRC(4be1666e) SHA1(35ba7bcf29ec7a8f8b6944ee3544693d4df1bfc2) )
	ROM_LOAD16_BYTE("d87-04.bin", 0x000001, 0x200000, CRC(2616002c) SHA1(003f98b740a697274385b8da03c78f3c6f7b5e89) )
	ROM_LOAD       ("d87-05.bin", 0x600000, 0x200000, CRC(4e5891a9) SHA1(fd08d848079841c9237fa359a850980fd00114d8) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d87-06.bin", 0x000000, 0x200000, CRC(3b97a07c) SHA1(72cdeffedeab0c1bd0e47f03172085390a2be393) )
	ROM_LOAD16_BYTE("d87-17.bin", 0x000001, 0x200000, CRC(e601d63e) SHA1(256a6aeb5633fe1db407fad567169a9d0c911219) )
	ROM_LOAD       ("d87-08.bin", 0x600000, 0x200000, CRC(76d23602) SHA1(ca53ea6641182c44a4038bbeaa5effb1687f1980) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d87-13.bin", 0x100000, 0x40000, CRC(15b1fff4) SHA1(28692b731ae98a47c2c5e11a8a71b61a813d9a64) )
	ROM_LOAD16_BYTE("d87-14.bin", 0x100001, 0x40000, CRC(eecda29a) SHA1(6eb238e47bc7bf635ffbdbb25fb06a37db980ef8) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d87-01.bin", 0x000000, 0x200000, CRC(3848a110) SHA1(802e91695a526f665c7fd261f0a7639a0b883c9e) )    // C9 CA CB CC
	ROM_LOAD16_BYTE("d87-02.bin", 0x400000, 0x200000, CRC(9250abae) SHA1(07cae8edbc3cca0a95022d9b40a5c18a55350b67) )    // CD CE CF D0
ROM_END

ROM_START( dariusgx )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("dge_mpr3.bin", 0x000000, 0x80000, CRC(1c1e24a7) SHA1(eafde331c3be5be55d0d838a84017f357ff92634) )
	ROM_LOAD32_BYTE("dge_mpr2.bin", 0x000001, 0x80000, CRC(7be23e23) SHA1(4764355f51e207f4538dd753aea59bf2689835de) )
	ROM_LOAD32_BYTE("dge_mpr1.bin", 0x000002, 0x80000, CRC(bc030f6f) SHA1(841396911d26ddfae0c9863431e02e0b5e762ac6) )
	ROM_LOAD32_BYTE("dge_mpr0.bin", 0x000003, 0x80000, CRC(c5bd135c) SHA1(402e26a05f1c3162fa3a8d3fcb81ef334b733699) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d87-03.bin", 0x000000, 0x200000, CRC(4be1666e) SHA1(35ba7bcf29ec7a8f8b6944ee3544693d4df1bfc2) )
	ROM_LOAD16_BYTE("d87-04.bin", 0x000001, 0x200000, CRC(2616002c) SHA1(003f98b740a697274385b8da03c78f3c6f7b5e89) )
	ROM_LOAD       ("d87-05.bin", 0x600000, 0x200000, CRC(4e5891a9) SHA1(fd08d848079841c9237fa359a850980fd00114d8) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d87-06.bin", 0x000000, 0x200000, CRC(3b97a07c) SHA1(72cdeffedeab0c1bd0e47f03172085390a2be393) )
	ROM_LOAD16_BYTE("d87-17.bin", 0x000001, 0x200000, CRC(e601d63e) SHA1(256a6aeb5633fe1db407fad567169a9d0c911219) )
	ROM_LOAD       ("d87-08.bin", 0x600000, 0x200000, CRC(76d23602) SHA1(ca53ea6641182c44a4038bbeaa5effb1687f1980) )
	ROM_FILL       (              0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d87-13.bin", 0x100000, 0x40000, CRC(15b1fff4) SHA1(28692b731ae98a47c2c5e11a8a71b61a813d9a64) )
	ROM_LOAD16_BYTE("d87-14.bin", 0x100001, 0x40000, CRC(eecda29a) SHA1(6eb238e47bc7bf635ffbdbb25fb06a37db980ef8) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d87-01.bin", 0x000000, 0x200000, CRC(3848a110) SHA1(802e91695a526f665c7fd261f0a7639a0b883c9e) )    // C9 CA CB CC
	ROM_LOAD16_BYTE("d87-02.bin", 0x400000, 0x200000, CRC(9250abae) SHA1(07cae8edbc3cca0a95022d9b40a5c18a55350b67) )    // CD CE CF D0
ROM_END


ROM_START( bublbob2 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d90-21.ic20", 0x000000, 0x40000, CRC(2a2b771a) SHA1(7f9bd768cf34069ca139261ebd8304325598fec6) )
	ROM_LOAD32_BYTE("d90-20.ic19", 0x000001, 0x40000, CRC(f01f63b6) SHA1(cbdc8c6248a2c0c1bc77fdc28738f67ce9a6aec3) )
	ROM_LOAD32_BYTE("d90-19.ic18", 0x000002, 0x40000, CRC(86eef19a) SHA1(9a389fefa280662843cafb68b5ae411e9348d34d) )
	ROM_LOAD32_BYTE("d90-18.ic17", 0x000003, 0x40000, CRC(f5b8cdce) SHA1(cf6ce6638eebd7d2e1defdd48110cc3002109c5c) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d90-03", 0x000000, 0x100000, CRC(6fa894a1) SHA1(7c33e6d41e8928029b92d66557a3712b51c49c67) )
	ROM_LOAD16_BYTE("d90-02", 0x000001, 0x100000, CRC(5ab04ca2) SHA1(6d87e7ca3167ff81a041cfedbbed84d51da997de) )
	ROM_LOAD       ("d90-01", 0x300000, 0x100000, CRC(8aedb9e5) SHA1(fb49330f7985a829c9544ecfd0bc672494f29cf6) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d90-08", 0x000000, 0x100000, CRC(25a4fb2c) SHA1(c8bf6fe2291c05386b32cd26bfcb379da756d7b5) )
	ROM_LOAD16_BYTE("d90-07", 0x000001, 0x100000, CRC(b436b42d) SHA1(559827120273733147b260e0723054d926dbea5e) )
	ROM_LOAD       ("d90-06", 0x300000, 0x100000, CRC(166a72b8) SHA1(7f70b8c960794322e1dc88e6600a2d13d948d873) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d90-13.ic32", 0x100000, 0x40000, CRC(6762bd90) SHA1(771db0382bc8dab2caf13d0fc20648366c685829) )
	ROM_LOAD16_BYTE("d90-14.ic33", 0x100001, 0x40000, CRC(8e33357e) SHA1(68b81693c22e6357e37244f2a416818a81338138) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d90-04", 0x000000, 0x200000, CRC(feee5fda) SHA1(b89354013ec4d34bcd51ecded412effa66dd2f2f) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d90-05", 0x400000, 0x200000, CRC(c192331f) SHA1(ebab05b3681c70b373bc06c1826be1cc397d3af7) )    // CC CD -std-

	ROM_REGION( 0x1200, "plds", 0 )
	ROM_LOAD("d77-14_palce16v8q-15.ic21.bin", 0x000, 0x117, CRC(2c798a1c) SHA1(e8ac31c3cd53eb61fedfd710c31356e8fa968cbc) )
	ROM_LOAD("d77-12_palce16v8q-15.ic48.bin", 0x000, 0x117, CRC(b1cc6195) SHA1(629ef8416a2cb51fcbc48e5c306dd04c96902726) )
	ROM_LOAD("d77-11_palce16v8q-15.ic37.bin", 0x000, 0x117, CRC(a733f0de) SHA1(6eec26043cedb3cae4efe93faa84a07327be468b) )
ROM_END


ROM_START( bublbob2o )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d90-12", 0x000000, 0x40000, CRC(9e523996) SHA1(c49a426f9865f96e8021c8ed9a6ac094c5e586b1) )
	ROM_LOAD32_BYTE("d90-11", 0x000001, 0x40000, CRC(edfdbb7f) SHA1(698ad631d5b13661645f2c5ccd3e4fbf0248053c) )
	ROM_LOAD32_BYTE("d90-10", 0x000002, 0x40000, CRC(8e957d3d) SHA1(5db31e5788483b802592e1092bf98df51ff4b70e) )
	ROM_LOAD32_BYTE("d90-17", 0x000003, 0x40000, CRC(711f1894) SHA1(8e574d9a63593fbe0c87840e79a2e2dbfc227671) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d90-03", 0x000000, 0x100000, CRC(6fa894a1) SHA1(7c33e6d41e8928029b92d66557a3712b51c49c67) )
	ROM_LOAD16_BYTE("d90-02", 0x000001, 0x100000, CRC(5ab04ca2) SHA1(6d87e7ca3167ff81a041cfedbbed84d51da997de) )
	ROM_LOAD       ("d90-01", 0x300000, 0x100000, CRC(8aedb9e5) SHA1(fb49330f7985a829c9544ecfd0bc672494f29cf6) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d90-08", 0x000000, 0x100000, CRC(25a4fb2c) SHA1(c8bf6fe2291c05386b32cd26bfcb379da756d7b5) )
	ROM_LOAD16_BYTE("d90-07", 0x000001, 0x100000, CRC(b436b42d) SHA1(559827120273733147b260e0723054d926dbea5e) )
	ROM_LOAD       ("d90-06", 0x300000, 0x100000, CRC(166a72b8) SHA1(7f70b8c960794322e1dc88e6600a2d13d948d873) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d90-13.ic32", 0x100000, 0x40000, CRC(6762bd90) SHA1(771db0382bc8dab2caf13d0fc20648366c685829) )
	ROM_LOAD16_BYTE("d90-14.ic33", 0x100001, 0x40000, CRC(8e33357e) SHA1(68b81693c22e6357e37244f2a416818a81338138) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d90-04", 0x000000, 0x200000, CRC(feee5fda) SHA1(b89354013ec4d34bcd51ecded412effa66dd2f2f) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d90-05", 0x400000, 0x200000, CRC(c192331f) SHA1(ebab05b3681c70b373bc06c1826be1cc397d3af7) )    // CC CD -std-
ROM_END

/* Very early revision protoype boardset & roms */
// todo, transfer information on PCB differences from http://forum.arcadeotaku.com/viewtopic.php?f=2&t=24965
ROM_START( bublbob2p )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("soft-3-8c9b.ic60", 0x000000, 0x40000, CRC(15d0594e) SHA1(7556377355860c3f7f600c2c352e5291da6a62f1) )
	ROM_LOAD32_BYTE("soft-2-0587.ic61", 0x000001, 0x40000, CRC(d1a5231f) SHA1(1e9ccac78f690ef79f933743ce7c4d6fa42f5acd) )
	ROM_LOAD32_BYTE("soft-1-9a9c.ic62", 0x000002, 0x40000, CRC(c11a4d26) SHA1(b327413f5420608f1ccbbac2e8941a82862377c5) )
	ROM_LOAD32_BYTE("soft-0-a523.ic63", 0x000003, 0x40000, CRC(58131f9e) SHA1(d07d34bf079277a48151ef9e5e7c1240a36d1bdb) )

	ROM_REGION(0x200000, "gfx1" , ROMREGION_ERASE00 ) /* Sprites */
	ROM_LOAD16_BYTE       ("cq80-obj-0l-c166.ic8",  0x000000, 0x80000, CRC(9bff223b) SHA1(acf22731d91d61aefc3373f78006fd310bb89edf) )
	ROM_LOAD16_BYTE       ("cq80-obj-0m-24f4.ic30", 0x000001, 0x80000, CRC(ee71f643) SHA1(7a2042c6fad8f1b7e0a3ad077d054dc163a22230) )
	ROM_LOAD              ("cq80-obj-0h-990d.ic32", 0x180000, 0x80000, CRC(4d3a78e0) SHA1(b19fb66e6082a68dc8600b8882ba50a3afce27c3) )

	ROM_REGION(0x400000, "gfx2" , ROMREGION_ERASE00 ) /* Tiles */
	ROM_LOAD32_BYTE("cq80-scr0-5ba4.ic7", 0x000000, 0x080000, CRC(044dc38b) SHA1(0bb715c9ae8298c6852c6309d69f769e87ab2fdc) )
	ROM_LOAD32_BYTE("cq80-scr1-a5f3.ic6", 0x000002, 0x080000, CRC(3cf3a3ba) SHA1(da7282104fbd9108bae12fa6722e077d80107d6d) )
	ROM_LOAD32_BYTE("cq80-scr2-cc11.ic5", 0x000001, 0x080000, CRC(b81aa2c7) SHA1(4650c431dc2ed73f1e71337f3e7d4c1837b65bcf) )
	ROM_LOAD32_BYTE("cq80-scr3-4266.ic4", 0x000003, 0x080000, CRC(c114583f) SHA1(ec85e8f4135e48607bb84b810d57d570ef56b228) )
	ROM_LOAD16_BYTE("cq80-scr4-7fe1.ic3", 0x300000, 0x080000, CRC(2bba1728) SHA1(cdd2e651c233a185fcbb3fd8f5eabee2af30f781) )

	ROM_REGION(0x180000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("snd-h-348f.ic66", 0x100000, 0x20000, CRC(f66e60f2) SHA1(b94c97ccde179a69811137c77730c91924236bfe))
	ROM_LOAD16_BYTE("snd-l-4ec1.ic65", 0x100001, 0x20000, CRC(d302d8bc) SHA1(02a2e69d0f4406578b12b05ab25d2abdf5bbba3c) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE("cq80-snd-data0-7b5f.ic43", 0x000000, 0x080000, CRC(bf8f26d3) SHA1(b165fc62ed30ae56d27caffbb0b16321d3c5ef8b) )    // C8
	ROM_LOAD16_BYTE("cq80-snd-data1-933b.ic44", 0x100000, 0x080000, CRC(62b00475) SHA1(d2b44940cefca76897b291d83b5ca8ec18dbe1fa) )    // C9

	ROM_LOAD16_BYTE("cq80-snd3-std5-3a9c.ic10", 0x600000, 0x080000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )    // -std-
	ROM_LOAD16_BYTE("cq80-snd2-std6-a148.ic11", 0x700000, 0x080000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )    // -std-

	ROM_REGION(0x180000, "pals", 0)
	// dumped regular way, appear to be protected
	//ROM_LOAD("pal20l10a.ic12.bin", 0x000, 0xcc, BAD_DUMP CRC(5d695690) SHA1(713cdbb894861eb5a6361026af8618df7e7db467) )
	//ROM_LOAD("pal20l10a.ic24.bin", 0x000, 0xcc, BAD_DUMP CRC(5d695690) SHA1(713cdbb894861eb5a6361026af8618df7e7db467) )
	// dumped using ABI Boardmaster, valid data but now in format for a GAL22V10 instead so still marked as BAD_DUMP
	ROM_LOAD("bb2proto-ic12.bin", 0x000, 0x2e5, BAD_DUMP CRC(acf20b88) SHA1(46ba1bcfd034685c81f597c8d4efdf1cefa5157c) )
	ROM_LOAD("bb2proto-ic24.bin", 0x000, 0x2e5, BAD_DUMP CRC(d15a4987) SHA1(628899931e71e19a7d574fcde9a7768f2c65a426) )
	// unprotected?
	ROM_LOAD("pal16l8b.ic57.bin", 0x000, 0x104, CRC(74b4d8be) SHA1(23ba316bc5550dd69f07f9a0f697927656a35e5a) )
	ROM_LOAD("pal16l8b.ic58.bin", 0x000, 0x104, CRC(17e2c9b8) SHA1(841b290ee3a3089a975456b20018c04e6afb2257) )
	ROM_LOAD("pal16l8b.ic59.bin", 0x000, 0x104, CRC(dc0db200) SHA1(9f46e7edf053ad9ee3a4a6dd00f9f6996203fc60) )
	ROM_LOAD("pal16l8b.ic64.bin", 0x000, 0x104, CRC(3aed3d98) SHA1(8f0b02baad87aeda1767f0e95aa1a84545376076) )
ROM_END


ROM_START( bubsymphe )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d90-12", 0x000000, 0x40000, CRC(9e523996) SHA1(c49a426f9865f96e8021c8ed9a6ac094c5e586b1) )
	ROM_LOAD32_BYTE("d90-11", 0x000001, 0x40000, CRC(edfdbb7f) SHA1(698ad631d5b13661645f2c5ccd3e4fbf0248053c) )
	ROM_LOAD32_BYTE("d90-10", 0x000002, 0x40000, CRC(8e957d3d) SHA1(5db31e5788483b802592e1092bf98df51ff4b70e) )
	ROM_LOAD32_BYTE("d90-16", 0x000003, 0x40000, CRC(d12ef19b) SHA1(8715102b54c730c809b3964a80cd1aed863ba334) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d90-03", 0x000000, 0x100000, CRC(6fa894a1) SHA1(7c33e6d41e8928029b92d66557a3712b51c49c67) )
	ROM_LOAD16_BYTE("d90-02", 0x000001, 0x100000, CRC(5ab04ca2) SHA1(6d87e7ca3167ff81a041cfedbbed84d51da997de) )
	ROM_LOAD       ("d90-01", 0x300000, 0x100000, CRC(8aedb9e5) SHA1(fb49330f7985a829c9544ecfd0bc672494f29cf6) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d90-08", 0x000000, 0x100000, CRC(25a4fb2c) SHA1(c8bf6fe2291c05386b32cd26bfcb379da756d7b5) )
	ROM_LOAD16_BYTE("d90-07", 0x000001, 0x100000, CRC(b436b42d) SHA1(559827120273733147b260e0723054d926dbea5e) )
	ROM_LOAD       ("d90-06", 0x300000, 0x100000, CRC(166a72b8) SHA1(7f70b8c960794322e1dc88e6600a2d13d948d873) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d90-13.ic32", 0x100000, 0x40000, CRC(6762bd90) SHA1(771db0382bc8dab2caf13d0fc20648366c685829) )
	ROM_LOAD16_BYTE("d90-14.ic33", 0x100001, 0x40000, CRC(8e33357e) SHA1(68b81693c22e6357e37244f2a416818a81338138) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d90-04", 0x000000, 0x200000, CRC(feee5fda) SHA1(b89354013ec4d34bcd51ecded412effa66dd2f2f) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d90-05", 0x400000, 0x200000, CRC(c192331f) SHA1(ebab05b3681c70b373bc06c1826be1cc397d3af7) )    // CC CD -std-
ROM_END

ROM_START( bubsymphj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d90-12", 0x000000, 0x40000, CRC(9e523996) SHA1(c49a426f9865f96e8021c8ed9a6ac094c5e586b1) )
	ROM_LOAD32_BYTE("d90-11", 0x000001, 0x40000, CRC(edfdbb7f) SHA1(698ad631d5b13661645f2c5ccd3e4fbf0248053c) )
	ROM_LOAD32_BYTE("d90-10", 0x000002, 0x40000, CRC(8e957d3d) SHA1(5db31e5788483b802592e1092bf98df51ff4b70e) )
	ROM_LOAD32_BYTE("d90-09", 0x000003, 0x40000, CRC(3f2090b7) SHA1(2a95c8c8dc23b618c0ce65497391d464494f4d6a) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d90-03", 0x000000, 0x100000, CRC(6fa894a1) SHA1(7c33e6d41e8928029b92d66557a3712b51c49c67) )
	ROM_LOAD16_BYTE("d90-02", 0x000001, 0x100000, CRC(5ab04ca2) SHA1(6d87e7ca3167ff81a041cfedbbed84d51da997de) )
	ROM_LOAD       ("d90-01", 0x300000, 0x100000, CRC(8aedb9e5) SHA1(fb49330f7985a829c9544ecfd0bc672494f29cf6) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d90-08", 0x000000, 0x100000, CRC(25a4fb2c) SHA1(c8bf6fe2291c05386b32cd26bfcb379da756d7b5) )
	ROM_LOAD16_BYTE("d90-07", 0x000001, 0x100000, CRC(b436b42d) SHA1(559827120273733147b260e0723054d926dbea5e) )
	ROM_LOAD       ("d90-06", 0x300000, 0x100000, CRC(166a72b8) SHA1(7f70b8c960794322e1dc88e6600a2d13d948d873) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d90-13.ic32", 0x100000, 0x40000, CRC(6762bd90) SHA1(771db0382bc8dab2caf13d0fc20648366c685829) )
	ROM_LOAD16_BYTE("d90-14.ic33", 0x100001, 0x40000, CRC(8e33357e) SHA1(68b81693c22e6357e37244f2a416818a81338138) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d90-04", 0x000000, 0x200000, CRC(feee5fda) SHA1(b89354013ec4d34bcd51ecded412effa66dd2f2f) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d90-05", 0x400000, 0x200000, CRC(c192331f) SHA1(ebab05b3681c70b373bc06c1826be1cc397d3af7) )    // CC CD -std-

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-14.bin", 0x0800, 0x0117, CRC(7427e777) SHA1(e692cedb13e5bc02edc4b25e9dcea51e6715de85) )
ROM_END

/* bootleg without ensoniq sound system */
/* the bootleg has some graphical problems because the actual bootleg hardware
   doesn't replicate the f3 system perfectly */

/*

Bubble Symphony hack
This romset comes from a bootleg single board modified Taito F3 System,the only know bootleg/hacked game of this system.Differences are:Date in copyright string removed,part of background layer corrupted or missing (as i have seen in arcade).
Hardware info:
Main cpu 68020
Sound chip OKI6295
Other chips 2x A1020B PLC
OSC 32 and 26Mhz.
The audio section was totally modified.It lacks 68000 and Ensoniq chips,these surely due to availability problems and to hold low production costs.
Some musics are missing,too.
Sound volume regulation output is gained via common analog operational and power ic amplification (LM324 and 1241).In test mode,digital regulation hasn't effect,due to obvious reason.
Rom definition:
D11->ADPCM sound data
D12 to D15->main program
D11b to D15b and D16 to D20->graphics
All eprom are 8bit (27C020,27C208 and 27C040,27C4001).

Dumped by tirino73

*/


ROM_START( bubsymphb )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("bsb_d12.bin", 0x000000, 0x40000, CRC(d05160fc) SHA1(62eb47827d4089711af1187455faeb62a8d9e369) )
	ROM_LOAD32_BYTE("bsb_d13.bin", 0x000001, 0x40000, CRC(83fc0d2c) SHA1(275a923e6e6b6763cb7036d94943609eb9ce866b) )
	ROM_LOAD32_BYTE("bsb_d14.bin", 0x000002, 0x40000, CRC(e6d49bb7) SHA1(9ca509ffd3f46160f3bbb0ceee9abcd09f9f9ad1) )
	ROM_LOAD32_BYTE("bsb_d15.bin", 0x000003, 0x40000, CRC(014cf8e0) SHA1(5e91fc6665c1d472d0651c06108ea6a8eb5ae8b4) )

	ROM_REGION(0x300000, "gfx1" , 0) /* Sprites (5bpp) */
	ROM_FILL       (        0x000000, 0x080000, nullptr )
	ROM_LOAD("bsb_d18.bin", 0x080000, 0x080000, CRC(22d7eeb5) SHA1(30aa4493c5bc98d9256817b9e3341b20d2b76f1f) )
	ROM_LOAD("bsb_d19.bin", 0x100000, 0x080000, CRC(d36801fd) SHA1(5dc00942dbd9bb29214c206ea192f158b71b09dc) )
	ROM_LOAD("bsb_d20.bin", 0x180000, 0x080000, CRC(20222e15) SHA1(41f2f94afd65d0c106752f254f20b593906cba28) )
	ROM_LOAD("bsb_d17.bin", 0x200000, 0x080000, CRC(ea2eadfc) SHA1(2c4176e89f816166f410888cd2e837891a4289a3) )
	ROM_LOAD("bsb_d16.bin", 0x280000, 0x080000, CRC(edccd4e0) SHA1(e00d3c1a91f9a96e1ae7d45842315d839c9cd440) )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD32_BYTE("bsb_d13b.bin", 0x000000, 0x080000, CRC(430af2aa) SHA1(e935f9f4e0558a25bd4010b44dbb4f38a9d359e0) )
	ROM_LOAD32_BYTE("bsb_d14b.bin", 0x000001, 0x080000, CRC(c006e832) SHA1(4aebbac188af1ef9176581ac68ac12ce397f2f08) )
	ROM_LOAD32_BYTE("bsb_d15b.bin", 0x000002, 0x080000, CRC(74644ad4) SHA1(c7610e413c965eb4e40b548d13efdcbc3dde23be) )
	ROM_LOAD32_BYTE("bsb_d12b.bin", 0x000003, 0x080000, CRC(cb2e2abb) SHA1(7e3a90cb8af298bac2aef80778341833e473b671) )
	ROM_LOAD32_BYTE("bsb_d11b.bin", 0x200000, 0x080000, CRC(d0607829) SHA1(546c629ec22bb98202c7127ccb77df0b8f3a1966) )

	ROM_REGION(0x100000, "oki" , ROMREGION_ERASE00 ) // OKI6295 samples
	ROM_LOAD("bsb_d11.bin", 0x000000, 0x080000, CRC(26bdc617) SHA1(993e7a52128fdd58f22d95521a629beb71ca7b91) ) // I haven't verified this dump.. but given how bad the rest is I'm not confident
	ROM_RELOAD(0x80000,0x80000)
ROM_END

ROM_START( bubsymphu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d90-12", 0x000000, 0x40000, CRC(9e523996) SHA1(c49a426f9865f96e8021c8ed9a6ac094c5e586b1) )
	ROM_LOAD32_BYTE("d90-11", 0x000001, 0x40000, CRC(edfdbb7f) SHA1(698ad631d5b13661645f2c5ccd3e4fbf0248053c) )
	ROM_LOAD32_BYTE("d90-10", 0x000002, 0x40000, CRC(8e957d3d) SHA1(5db31e5788483b802592e1092bf98df51ff4b70e) )
	ROM_LOAD32_BYTE("d90-15", 0x000003, 0x40000, CRC(06182802) SHA1(c068ea8e8852033d0cf7bd4bca4b0411b7aebded) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d90-03", 0x000000, 0x100000, CRC(6fa894a1) SHA1(7c33e6d41e8928029b92d66557a3712b51c49c67) )
	ROM_LOAD16_BYTE("d90-02", 0x000001, 0x100000, CRC(5ab04ca2) SHA1(6d87e7ca3167ff81a041cfedbbed84d51da997de) )
	ROM_LOAD       ("d90-01", 0x300000, 0x100000, CRC(8aedb9e5) SHA1(fb49330f7985a829c9544ecfd0bc672494f29cf6) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d90-08", 0x000000, 0x100000, CRC(25a4fb2c) SHA1(c8bf6fe2291c05386b32cd26bfcb379da756d7b5) )
	ROM_LOAD16_BYTE("d90-07", 0x000001, 0x100000, CRC(b436b42d) SHA1(559827120273733147b260e0723054d926dbea5e) )
	ROM_LOAD       ("d90-06", 0x300000, 0x100000, CRC(166a72b8) SHA1(7f70b8c960794322e1dc88e6600a2d13d948d873) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("d90-13.ic32", 0x100000, 0x40000, CRC(6762bd90) SHA1(771db0382bc8dab2caf13d0fc20648366c685829) )
	ROM_LOAD16_BYTE("d90-14.ic33", 0x100001, 0x40000, CRC(8e33357e) SHA1(68b81693c22e6357e37244f2a416818a81338138) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d90-04", 0x000000, 0x200000, CRC(feee5fda) SHA1(b89354013ec4d34bcd51ecded412effa66dd2f2f) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d90-05", 0x400000, 0x200000, CRC(c192331f) SHA1(ebab05b3681c70b373bc06c1826be1cc397d3af7) )    // CC CD -std-
ROM_END

ROM_START( spcinvdj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d93-04.bin", 0x000000, 0x20000, CRC(cd9a4e5c) SHA1(b163b8274570610af8697b1b38116dcf4c3593db) )
	ROM_LOAD32_BYTE("d93-03.bin", 0x000001, 0x20000, CRC(0174bfc1) SHA1(133452f6a5bdf01b1b436077288a597734a8731a) )
	ROM_LOAD32_BYTE("d93-02.bin", 0x000002, 0x20000, CRC(01922b31) SHA1(660c9c20e76a5f4094f1bfee9d75146f0829daeb) )
	ROM_LOAD32_BYTE("d93-01.bin", 0x000003, 0x20000, CRC(4a74ab1c) SHA1(5f7ae70d8fa3f141239ed3de3a45c50e2d824864) )

	ROM_REGION(0x200000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d93-07.12", 0x000000, 0x80000, CRC(8cf5b972) SHA1(75e383aed8548f4ac7d38f1f08bf33fae2a93064) )
	ROM_LOAD16_BYTE("d93-08.08", 0x000001, 0x80000, CRC(4c11af2b) SHA1(e332372ab0d1322faa8d6d98f8a6e3bbf51d2008) )
	ROM_FILL       (             0x100000, 0x100000,nullptr )

	ROM_REGION(0x80000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d93-09.47", 0x000000, 0x20000, CRC(9076f663) SHA1(c94e93e40926df33b6bb528e0ef30381631913d7) )
	ROM_LOAD16_BYTE("d93-10.45", 0x000001, 0x20000, CRC(8a3f531b) SHA1(69f9971c45971018108a5d312d5bbcfd3caf9bd0) )
	ROM_FILL       (             0x040000, 0x40000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d93-05.bin", 0x100000, 0x20000, CRC(ff365596) SHA1(4cf2e0d6f42cf3fb69796be6092eff8a47f7f8b9) )
	ROM_LOAD16_BYTE("d93-06.bin", 0x100001, 0x20000, CRC(ef7ad400) SHA1(01be403d575a543f089b910a5a8c381a6603e67e) )

	ROM_REGION16_BE(0x400000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 1 populated
	ROM_LOAD16_BYTE("d93-11.38", 0x000000, 0x80000, CRC(df5853de) SHA1(bb1ea604d44819dc7c82848c5bde9612f70f7528) )  // C8
	ROM_LOAD16_BYTE("d93-12.39", 0x100000, 0x80000, CRC(b0f71d60) SHA1(35fc32764d9b82b1b40c5e9cc8e367cf842531a2) )  // C9
	ROM_LOAD16_BYTE("d93-13.40", 0x200000, 0x80000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) )  // -std-
	ROM_LOAD16_BYTE("d93-14.41", 0x300000, 0x80000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) )  // -std-
ROM_END

ROM_START( pwrgoal )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d94-18.bin", 0x000000, 0x40000, CRC(b92681c3) SHA1(0ca05a69d046668c878df3d2b7ae3172d748e290) )
	ROM_LOAD32_BYTE("d94-17.bin", 0x000001, 0x40000, CRC(6009333e) SHA1(4ab28f2d9e2b75adc668f5d9390e06086bbd97dc) )
	ROM_LOAD32_BYTE("d94-16.bin", 0x000002, 0x40000, CRC(c6dbc9c8) SHA1(4f096b59734db51eeddcf0649f2a6f11bdde9590) )
	ROM_LOAD32_BYTE("d94-22.rom", 0x000003, 0x40000, CRC(f672e487) SHA1(da62afc82aeae4aeeebbee0965cda3d84464ad09) )

	ROM_REGION(0x1800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d94-09.bin", 0x000000, 0x200000, CRC(425e6bec) SHA1(512508e7137fcdebdf2240dbbd37ea0cf1c4dcdc) )
	ROM_LOAD16_BYTE("d94-08.bin", 0x400000, 0x200000, CRC(bd909caf) SHA1(33952883afb8fe9b55dd258435af99881925e8d5) )
	ROM_LOAD16_BYTE("d94-07.bin", 0x800000, 0x200000, CRC(c8c95e49) SHA1(9bfdf63d6059b01a4cd5813239ba1bd98453a56b) )
	ROM_LOAD16_BYTE("d94-06.bin", 0x000001, 0x200000, CRC(0ed1df55) SHA1(10b22407ad0e03c37363783ee80f2cbf98a802a0) )
	ROM_LOAD16_BYTE("d94-05.bin", 0x400001, 0x200000, CRC(121c8542) SHA1(ec9b7e56c97a8b6ed0423f05b789ca89b1bb0d36) )
	ROM_LOAD16_BYTE("d94-04.bin", 0x800001, 0x200000, CRC(24958b50) SHA1(ea15ffa3a615e3e67c1bade6f6ef45424479115e) )
	ROM_LOAD       ("d94-03.bin", 0x1200000, 0x200000, CRC(95e32072) SHA1(9797f65ecadc6b0f209bf262396315b61855c433) )
	ROM_LOAD       ("d94-02.bin", 0x1400000, 0x200000, CRC(f460b9ac) SHA1(e36a812791bd0360380f397b1bc6c357391f585a) )
	ROM_LOAD       ("d94-01.bin", 0x1600000, 0x200000, CRC(410ffccd) SHA1(0cab00c8e9de92ad81ac61f25bbe8bfd60f45ae0) )
	ROM_FILL       (              0xc00000, 0x600000,nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d94-14.bin", 0x000000, 0x100000, CRC(b8ba5761) SHA1(7966ef3166d7d6b9913478eaef5dd4a2bf7d5a06) )
	ROM_LOAD16_BYTE("d94-13.bin", 0x000001, 0x100000, CRC(cafc68ce) SHA1(5c1f49951e83d812f0c7697751f4876ab1d08141) )
	ROM_LOAD       ("d94-12.bin", 0x300000, 0x100000, CRC(47064189) SHA1(99ceeb326dcc2e1c3acba8ac14d94dcb17c6e032) )
	ROM_FILL       (              0x200000, 0x100000,nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d94-19.bin", 0x100000, 0x40000, CRC(c93dbcf4) SHA1(413520e652d809651aff9b1b74e6353112d34c12) ) /* Over dump?? 0x20000-0x3ffff == 0xFF */
	ROM_LOAD16_BYTE("d94-20.bin", 0x100001, 0x40000, CRC(f232bf64) SHA1(bbfeae0785fc49c12aa6d9b1bd6ff7c8515f8fe7) ) /* Over dump?? 0x20000-0x3ffff == 0xFF */

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d94-10.bin", 0x000000, 0x200000, CRC(a22563ae) SHA1(85f2a4ca5e085ac1d4c15feb737229764697ae85) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d94-11.bin", 0x400000, 0x200000, CRC(61ed83fa) SHA1(f6ca60b7af61fd3ac01a987f949d7a7bc96e43ff) )    // CD CE -std-
ROM_END

ROM_START( hthero95 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d94-18.bin", 0x000000, 0x40000, CRC(b92681c3) SHA1(0ca05a69d046668c878df3d2b7ae3172d748e290) )
	ROM_LOAD32_BYTE("d94-17.bin", 0x000001, 0x40000, CRC(6009333e) SHA1(4ab28f2d9e2b75adc668f5d9390e06086bbd97dc) )
	ROM_LOAD32_BYTE("d94-16.bin", 0x000002, 0x40000, CRC(c6dbc9c8) SHA1(4f096b59734db51eeddcf0649f2a6f11bdde9590) )
	ROM_LOAD32_BYTE("d94-15.bin", 0x000003, 0x40000, CRC(187c85ab) SHA1(8270930b95fafe5ad92ea978c1558c491d9668b0) )

	ROM_REGION(0x1800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d94-09.bin", 0x000000, 0x200000, CRC(425e6bec) SHA1(512508e7137fcdebdf2240dbbd37ea0cf1c4dcdc) )
	ROM_LOAD16_BYTE("d94-08.bin", 0x400000, 0x200000, CRC(bd909caf) SHA1(33952883afb8fe9b55dd258435af99881925e8d5) )
	ROM_LOAD16_BYTE("d94-07.bin", 0x800000, 0x200000, CRC(c8c95e49) SHA1(9bfdf63d6059b01a4cd5813239ba1bd98453a56b) )
	ROM_LOAD16_BYTE("d94-06.bin", 0x000001, 0x200000, CRC(0ed1df55) SHA1(10b22407ad0e03c37363783ee80f2cbf98a802a0) )
	ROM_LOAD16_BYTE("d94-05.bin", 0x400001, 0x200000, CRC(121c8542) SHA1(ec9b7e56c97a8b6ed0423f05b789ca89b1bb0d36) )
	ROM_LOAD16_BYTE("d94-04.bin", 0x800001, 0x200000, CRC(24958b50) SHA1(ea15ffa3a615e3e67c1bade6f6ef45424479115e) )
	ROM_LOAD       ("d94-03.bin", 0x1200000, 0x200000, CRC(95e32072) SHA1(9797f65ecadc6b0f209bf262396315b61855c433) )
	ROM_LOAD       ("d94-02.bin", 0x1400000, 0x200000, CRC(f460b9ac) SHA1(e36a812791bd0360380f397b1bc6c357391f585a) )
	ROM_LOAD       ("d94-01.bin", 0x1600000, 0x200000, CRC(410ffccd) SHA1(0cab00c8e9de92ad81ac61f25bbe8bfd60f45ae0) )
	ROM_FILL       (              0xc00000, 0x600000,nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d94-14.bin", 0x000000, 0x100000, CRC(b8ba5761) SHA1(7966ef3166d7d6b9913478eaef5dd4a2bf7d5a06) )
	ROM_LOAD16_BYTE("d94-13.bin", 0x000001, 0x100000, CRC(cafc68ce) SHA1(5c1f49951e83d812f0c7697751f4876ab1d08141) )
	ROM_LOAD       ("d94-12.bin", 0x300000, 0x100000, CRC(47064189) SHA1(99ceeb326dcc2e1c3acba8ac14d94dcb17c6e032) )
	ROM_FILL       (              0x200000, 0x100000,nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d94-19.bin", 0x100000, 0x40000, CRC(c93dbcf4) SHA1(413520e652d809651aff9b1b74e6353112d34c12) ) /* Over dump?? 0x20000-0x3ffff == 0xFF */
	ROM_LOAD16_BYTE("d94-20.bin", 0x100001, 0x40000, CRC(f232bf64) SHA1(bbfeae0785fc49c12aa6d9b1bd6ff7c8515f8fe7) ) /* Over dump?? 0x20000-0x3ffff == 0xFF */

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d94-10.bin", 0x000000, 0x200000, CRC(a22563ae) SHA1(85f2a4ca5e085ac1d4c15feb737229764697ae85) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d94-11.bin", 0x400000, 0x200000, CRC(61ed83fa) SHA1(f6ca60b7af61fd3ac01a987f949d7a7bc96e43ff) )    // CD CE -std-
ROM_END

ROM_START( hthero95u )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d94-18.bin", 0x000000, 0x40000, CRC(b92681c3) SHA1(0ca05a69d046668c878df3d2b7ae3172d748e290) )
	ROM_LOAD32_BYTE("d94-17.bin", 0x000001, 0x40000, CRC(6009333e) SHA1(4ab28f2d9e2b75adc668f5d9390e06086bbd97dc) )
	ROM_LOAD32_BYTE("d94-16.bin", 0x000002, 0x40000, CRC(c6dbc9c8) SHA1(4f096b59734db51eeddcf0649f2a6f11bdde9590) )
	ROM_LOAD32_BYTE("d94-21.bin", 0x000003, 0x40000, CRC(8175d411) SHA1(b93ffef510ecfaced6cae07ea6cd549af7473049) )

	ROM_REGION(0x1800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d94-09.bin", 0x000000, 0x200000, CRC(425e6bec) SHA1(512508e7137fcdebdf2240dbbd37ea0cf1c4dcdc) )
	ROM_LOAD16_BYTE("d94-08.bin", 0x400000, 0x200000, CRC(bd909caf) SHA1(33952883afb8fe9b55dd258435af99881925e8d5) )
	ROM_LOAD16_BYTE("d94-07.bin", 0x800000, 0x200000, CRC(c8c95e49) SHA1(9bfdf63d6059b01a4cd5813239ba1bd98453a56b) )
	ROM_LOAD16_BYTE("d94-06.bin", 0x000001, 0x200000, CRC(0ed1df55) SHA1(10b22407ad0e03c37363783ee80f2cbf98a802a0) )
	ROM_LOAD16_BYTE("d94-05.bin", 0x400001, 0x200000, CRC(121c8542) SHA1(ec9b7e56c97a8b6ed0423f05b789ca89b1bb0d36) )
	ROM_LOAD16_BYTE("d94-04.bin", 0x800001, 0x200000, CRC(24958b50) SHA1(ea15ffa3a615e3e67c1bade6f6ef45424479115e) )
	ROM_LOAD       ("d94-03.bin", 0x1200000, 0x200000, CRC(95e32072) SHA1(9797f65ecadc6b0f209bf262396315b61855c433) )
	ROM_LOAD       ("d94-02.bin", 0x1400000, 0x200000, CRC(f460b9ac) SHA1(e36a812791bd0360380f397b1bc6c357391f585a) )
	ROM_LOAD       ("d94-01.bin", 0x1600000, 0x200000, CRC(410ffccd) SHA1(0cab00c8e9de92ad81ac61f25bbe8bfd60f45ae0) )
	ROM_FILL       (              0xc00000, 0x600000,nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d94-14.bin", 0x000000, 0x100000, CRC(b8ba5761) SHA1(7966ef3166d7d6b9913478eaef5dd4a2bf7d5a06) )
	ROM_LOAD16_BYTE("d94-13.bin", 0x000001, 0x100000, CRC(cafc68ce) SHA1(5c1f49951e83d812f0c7697751f4876ab1d08141) )
	ROM_LOAD       ("d94-12.bin", 0x300000, 0x100000, CRC(47064189) SHA1(99ceeb326dcc2e1c3acba8ac14d94dcb17c6e032) )
	ROM_FILL       (              0x200000, 0x100000,nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d94-19.bin", 0x100000, 0x40000, CRC(c93dbcf4) SHA1(413520e652d809651aff9b1b74e6353112d34c12) ) /* Over dump?? 0x20000-0x3ffff == 0xFF */
	ROM_LOAD16_BYTE("d94-20.bin", 0x100001, 0x40000, CRC(f232bf64) SHA1(bbfeae0785fc49c12aa6d9b1bd6ff7c8515f8fe7) ) /* Over dump?? 0x20000-0x3ffff == 0xFF */

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d94-10.bin", 0x000000, 0x200000, CRC(a22563ae) SHA1(85f2a4ca5e085ac1d4c15feb737229764697ae85) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("d94-11.bin", 0x400000, 0x200000, CRC(61ed83fa) SHA1(f6ca60b7af61fd3ac01a987f949d7a7bc96e43ff) )    // CD CE -std-
ROM_END

ROM_START( qtheater )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("d95-12.20", 0x000000, 0x80000, CRC(fcee76ee) SHA1(9ffeeda656368d1c065ba1cdb51a8665f7f7262a) )
	ROM_LOAD32_BYTE("d95-11.19", 0x000001, 0x80000, CRC(b3c2b8d5) SHA1(536c13d71e858309f41e7c387cd988e8fe356bee) )
	ROM_LOAD32_BYTE("d95-10.18", 0x000002, 0x80000, CRC(85236e40) SHA1(727c8f7361d7e0af3239bb0c0e7778ab30b12739) )
	ROM_LOAD32_BYTE("d95-09.17", 0x000003, 0x80000, CRC(f456519c) SHA1(9226d33d8d16a7d1054c1183ac013fc5caf229e2) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("d95-02.12", 0x000000, 0x200000, CRC(74ce6f3e) SHA1(eb03f44889bd2d5705e9d8cda6516b39758d9554) )
	ROM_LOAD16_BYTE("d95-01.8",  0x000001, 0x200000, CRC(141beb7d) SHA1(bba91f47f68367e2bb3d89298cb62fac2d4edf7b) )
	ROM_FILL       (             0x400000, 0x400000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("d95-06.47", 0x000000, 0x200000, CRC(70a0dcbb) SHA1(b1abe6a9a4afe55201229a62bae11ad1d96ca244) )
	ROM_LOAD16_BYTE("d95-05.45", 0x000001, 0x200000, CRC(1a1a852b) SHA1(89827485a31af4e2457775a5d16f747a764b6d67) )
	ROM_FILL       (             0x400000, 0x400000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("d95-07.32", 0x100000, 0x40000, CRC(3c201d70) SHA1(89fe4d363f4e1a847ba7d2894a2092708b287a33) )
	ROM_LOAD16_BYTE("d95-08.33", 0x100001, 0x40000, CRC(01c23354) SHA1(7b332edc844b1b1c1513e879215089987645fa3f) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("d95-03.38", 0x000000, 0x200000, CRC(4149ea67) SHA1(35fc9e60cd368c6eab20e23deb581aa4f46e164e) ) // C8 C9 CA CB
	ROM_LOAD16_BYTE("d95-04.41", 0x400000, 0x200000, CRC(e9049d16) SHA1(ffa7dfc5d1cb82a601bad26b634c993aedda7803) ) // CC CD -std-
ROM_END

ROM_START( spcinv95u )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e06-14.20", 0x000000, 0x20000, CRC(71ba7f00) SHA1(6b1d994c8319778aad3bec7bf7b24dc4944a36f0) )
	ROM_LOAD32_BYTE("e06-13.19", 0x000001, 0x20000, CRC(f506ba4b) SHA1(551f9e87d2bfd513998648b175b63677cd6bdd74) )
	ROM_LOAD32_BYTE("e06-12.18", 0x000002, 0x20000, CRC(06cbd72b) SHA1(0c8e11bd5f3fcf7451908c53e74ae545a0d97640) )
	ROM_LOAD32_BYTE("e06-15.17", 0x000003, 0x20000, CRC(a6ec0103) SHA1(4f524a6b52bbdb370b8f98d26e7446da943e3edd) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e06-03", 0x000000, 0x100000, CRC(a24070ef) SHA1(9b6ac7852114c606e871a08cfb3b9e1081ac7030) )
	ROM_LOAD16_BYTE("e06-02", 0x000001, 0x100000, CRC(8f646dea) SHA1(07cd79671f36df1a5bbf2434e92a601351a36259) )
	ROM_LOAD       ("e06-01", 0x300000, 0x100000, CRC(51721b15) SHA1(448a9a7f3631072d8987351f5c0b8dd2c908d266) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e06-08", 0x000000, 0x100000, CRC(72ae2fbf) SHA1(7fa7ec94a8031342a2446fb8eca0d89ecfd2fa4f) )
	ROM_LOAD16_BYTE("e06-07", 0x000001, 0x100000, CRC(4b02e8f5) SHA1(02d8a97da52f9ba4033b8f0c3f455a908a9dce89) )
	ROM_LOAD       ("e06-06", 0x300000, 0x100000, CRC(9380db3c) SHA1(83f5a46a01b9c15499e0dc2222df496d26baa0d4) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e06-09.32", 0x100000, 0x40000, CRC(9bcafc87) SHA1(10b3f6da00a41550fe6a705232f0e33fda3c7e7c) )
	ROM_LOAD16_BYTE("e06-10.33", 0x100001, 0x40000, CRC(b752b61f) SHA1(e948a8af19c70ba8b8e908c869bc88ed0cac8420) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e06-04", 0x000000, 0x200000, CRC(1dac29df) SHA1(ed68a41def148dcf4057cfac87a2a563c6882e1d) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e06-05", 0x400000, 0x200000, CRC(f370ff15) SHA1(4bc464d1c3a28326c8b1ae2036387954cb1dd813) )    // CC CD CE CF

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-13.bin", 0x0800, 0x0117, CRC(66e32e73) SHA1(350d0f98809f3623eab999540c7f47d1190dd648) )
ROM_END

ROM_START( spcinv95 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e06-14.20", 0x000000, 0x20000, CRC(71ba7f00) SHA1(6b1d994c8319778aad3bec7bf7b24dc4944a36f0) )
	ROM_LOAD32_BYTE("e06-13.19", 0x000001, 0x20000, CRC(f506ba4b) SHA1(551f9e87d2bfd513998648b175b63677cd6bdd74) )
	ROM_LOAD32_BYTE("e06-12.18", 0x000002, 0x20000, CRC(06cbd72b) SHA1(0c8e11bd5f3fcf7451908c53e74ae545a0d97640) )
	ROM_LOAD32_BYTE("e06-16.17", 0x000003, 0x20000, CRC(d1eb3195) SHA1(40c5e326e8dd9a892abdab952f853799f26601b7) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e06-03", 0x000000, 0x100000, CRC(a24070ef) SHA1(9b6ac7852114c606e871a08cfb3b9e1081ac7030) )
	ROM_LOAD16_BYTE("e06-02", 0x000001, 0x100000, CRC(8f646dea) SHA1(07cd79671f36df1a5bbf2434e92a601351a36259) )
	ROM_LOAD       ("e06-01", 0x300000, 0x100000, CRC(51721b15) SHA1(448a9a7f3631072d8987351f5c0b8dd2c908d266) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e06-08", 0x000000, 0x100000, CRC(72ae2fbf) SHA1(7fa7ec94a8031342a2446fb8eca0d89ecfd2fa4f) )
	ROM_LOAD16_BYTE("e06-07", 0x000001, 0x100000, CRC(4b02e8f5) SHA1(02d8a97da52f9ba4033b8f0c3f455a908a9dce89) )
	ROM_LOAD       ("e06-06", 0x300000, 0x100000, CRC(9380db3c) SHA1(83f5a46a01b9c15499e0dc2222df496d26baa0d4) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e06-09.32", 0x100000, 0x40000, CRC(9bcafc87) SHA1(10b3f6da00a41550fe6a705232f0e33fda3c7e7c) )
	ROM_LOAD16_BYTE("e06-10.33", 0x100001, 0x40000, CRC(b752b61f) SHA1(e948a8af19c70ba8b8e908c869bc88ed0cac8420) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e06-04", 0x000000, 0x200000, CRC(1dac29df) SHA1(ed68a41def148dcf4057cfac87a2a563c6882e1d) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e06-05", 0x400000, 0x200000, CRC(f370ff15) SHA1(4bc464d1c3a28326c8b1ae2036387954cb1dd813) )    // CC CD CE CF

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-13.bin", 0x0800, 0x0117, CRC(66e32e73) SHA1(350d0f98809f3623eab999540c7f47d1190dd648) )
ROM_END

ROM_START( akkanvdr )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e06-14.20", 0x000000, 0x20000, CRC(71ba7f00) SHA1(6b1d994c8319778aad3bec7bf7b24dc4944a36f0) )
	ROM_LOAD32_BYTE("e06-13.19", 0x000001, 0x20000, CRC(f506ba4b) SHA1(551f9e87d2bfd513998648b175b63677cd6bdd74) )
	ROM_LOAD32_BYTE("e06-12.18", 0x000002, 0x20000, CRC(06cbd72b) SHA1(0c8e11bd5f3fcf7451908c53e74ae545a0d97640) )
	ROM_LOAD32_BYTE("e06-11.17", 0x000003, 0x20000, CRC(3fe550b9) SHA1(6258d72204834abfba58bc2d5882f3616a6fd784) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e06-03", 0x000000, 0x100000, CRC(a24070ef) SHA1(9b6ac7852114c606e871a08cfb3b9e1081ac7030) )
	ROM_LOAD16_BYTE("e06-02", 0x000001, 0x100000, CRC(8f646dea) SHA1(07cd79671f36df1a5bbf2434e92a601351a36259) )
	ROM_LOAD       ("e06-01", 0x300000, 0x100000, CRC(51721b15) SHA1(448a9a7f3631072d8987351f5c0b8dd2c908d266) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e06-08", 0x000000, 0x100000, CRC(72ae2fbf) SHA1(7fa7ec94a8031342a2446fb8eca0d89ecfd2fa4f) )
	ROM_LOAD16_BYTE("e06-07", 0x000001, 0x100000, CRC(4b02e8f5) SHA1(02d8a97da52f9ba4033b8f0c3f455a908a9dce89) )
	ROM_LOAD       ("e06-06", 0x300000, 0x100000, CRC(9380db3c) SHA1(83f5a46a01b9c15499e0dc2222df496d26baa0d4) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e06-09.32", 0x100000, 0x40000, CRC(9bcafc87) SHA1(10b3f6da00a41550fe6a705232f0e33fda3c7e7c) )
	ROM_LOAD16_BYTE("e06-10.33", 0x100001, 0x40000, CRC(b752b61f) SHA1(e948a8af19c70ba8b8e908c869bc88ed0cac8420) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e06-04", 0x000000, 0x200000, CRC(1dac29df) SHA1(ed68a41def148dcf4057cfac87a2a563c6882e1d) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e06-05", 0x400000, 0x200000, CRC(f370ff15) SHA1(4bc464d1c3a28326c8b1ae2036387954cb1dd813) )    // CC CD CE CF
ROM_END


/*
Elevator Action Returns (Asia)
Taito, 1994

This game runs on Taito F3 System (F3 Motherboard and game cart)

PCB Layout
----------

Main Board
NEW F3 MOTHER (ASIA) M20A0001B MOTHER PCB J1100335B
|------------------------------------------------------|
| MB87078 51832       D77-07 D77-05                    |
| MC33274 51832       68000  D77-04            HM511664|
| TDA1543 D77-08             D77-03  TC0630FDP         |
|                                              HM511664|
|    HM511664                                  HM511664|
|    5510-ESPR5                      TC518128  HM511664|
|J                    MB8421         TC518128          |
|                                                      |
|A       TC0650FDA    MB8421                           |
|                                                      |
|M                                   16MHz             |
|         2088        D77-06                  TC0660FCM|
|M        2088                       30.4761MHz        |
|         2088                                         |
|A                    68681          26.686MHz         |
|                                                      |
|  JP3                                          D77-02 |
|                                                      |
|                     ENSONIC                          |
|        TC0640FIO    5701                             |
|                                               D77-01 |
|                                51832                 |
|                                                      |
|                                51832                 |
|                  ENSONIC                             |
|                  5505-OTISR2   51832                 |
|  SW1  93C46 3771                          68EC020    |
|                                51832                 |
|------------------------------------------------------|

Notes:
      68020 clock    : 15.23809 (30.47618 / 2)
      68000 clock    : 15.23809 (30.47618 / 2)
      68681 clocks   : pin2- 500kHz, pin32- 4.000MHz, pin36- 1.000MHz, pin38- 1.000MHz, pin39- 500kHz,
      5505 clocks    : pin12- 2.6686MHz, pin34- 15.23809MHz,
      5510 clocks    : pin1- 8.000MHz, pins4-6- 2.6686MHz, pin16- 2.6686MHz
      VSync          : 59Hz
      HSync          : 15.78kHz

      D77* = PALs


ROM Board

J9100361A ROM PCB (sticker M20A0112A  223101651)
|-------------------------------------------------------|
|D77-09  E02-03.12  IC10*  E02-02.8 IC6*  E02-01.4 IC2* |
|             IC11*    IC9*     IC7*   IC5*    IC3* IC1*|
|                                                       |
|                                                       |
|                                                       |
|                                                       |
|                                                       |
| E02-14.33          D77-15    E02-12.20   E02-10.18    |
|      E02-13.32  D77-10           E02-11.19   E02-16.17|
|                                                       |
|                                                       |
|                                                       |
|                                                       |
|D77-12                                                 |
|  E02-08.47 E02-07.45 E02-06.43  IC40*  E02-05.39      |
|      IC46*     IC44*    IC42* IC41*     E02-04.38     |
|                                               D77-11  |
|-------------------------------------------------------|
Notes:
      *: Unpopulated socket
      D77* = PALs
      E02-13/14 = 27C020 EPROMs
      E02-10/11/12/16 = 27C4001 EPROMs
      Remainder of ROMs are 42 pin 16M MASK or 8M MASK.
*/

ROM_START( elvactr )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e02-12.20", 0x000000, 0x80000, CRC(ea5f5a32) SHA1(4f30c56fbf068fee6d3afb2479043c7e89f6c055) )
	ROM_LOAD32_BYTE("e02-11.19", 0x000001, 0x80000, CRC(bcced8ff) SHA1(09c38c78300ba9d710b4e46ca71014bbc5ac46b4) )
	ROM_LOAD32_BYTE("e02-10.18", 0x000002, 0x80000, CRC(72f1b952) SHA1(9fc41ecfbee3581d9e92ff3a2ab6e4b93567e31d) )
	ROM_LOAD32_BYTE("e02-16.17",  0x000003, 0x80000, CRC(cd97182b) SHA1(b3387980acfeec81eb0178d5b2955ac39595e22d) )

	ROM_REGION(0x800000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD16_BYTE("e02-03.12", 0x000000, 0x200000, CRC(c884ebb5) SHA1(49009056bfdc564eac0ae6b7b49f070f05dc4ee3) )
	ROM_LOAD16_BYTE("e02-02.8",  0x000001, 0x200000, CRC(c8e06cfb) SHA1(071d095a4930ce18a782c577811b553a9705fbd7) )
	ROM_LOAD       ("e02-01.4",  0x600000, 0x200000, CRC(2ba94726) SHA1(3e9cdd076338e0e5358571ce4f97576f1a6a12a7) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2", 0 ) /* Tiles */
	ROM_LOAD16_BYTE("e02-08.47", 0x000000, 0x200000, CRC(29c9bd02) SHA1(a5b552ae7ac15f514ee6105410ec3e6e34ea0adb) )
	ROM_LOAD16_BYTE("e02-07.45", 0x000001, 0x200000, CRC(5eeee925) SHA1(d302da28df8ac6d406ef45f1d282ee22ce243857) )
	ROM_LOAD       ("e02-06.43", 0x600000, 0x200000, CRC(4c8726e9) SHA1(8ce2320a087f43c49428a39dafffec8c40d61b03) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e02-13.32", 0x100000, 0x40000, CRC(80932702) SHA1(c468234d03aa31b2aa0c3bd6bec32034216c2ae4) )
	ROM_LOAD16_BYTE("e02-14.33", 0x100001, 0x40000, CRC(706671a5) SHA1(1ac90647d617e73f12a67274a025ae43a6b3a316) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e02-04.38", 0x000000, 0x200000, CRC(b74307af) SHA1(deb42415049efa2df70e7b25ba8b1b716aa227f1) ) // C8 C9 CA CB
	ROM_LOAD16_BYTE("e02-05.39", 0x400000, 0x200000, CRC(eb729855) SHA1(85253efe794e8b5ffaf16bcb1123bca831e776a5) ) // CC CD CE CF

	ROM_REGION( 0x0e00, "plds", 0 )
	ROM_LOAD( "ampal20l10a.a12", 0x0000, 0x00cc, CRC(e719542f) SHA1(b28b9e13ec1ca98203ebbfd592dbdc44d3a6e936) )
	ROM_LOAD( "pal20l10b.a24",   0x0200, 0x00cc, NO_DUMP )  /* read protected */
	ROM_LOAD( "pal16l8b.b24",    0x0400, 0x0104, CRC(0b73a7d1) SHA1(953234ce95a8c2dfb9ba276742411cb2ca4d4bf8) )
	ROM_LOAD( "pal16l8b.b57",    0x0600, 0x0104, CRC(74b4d8be) SHA1(23ba316bc5550dd69f07f9a0f697927656a35e5a) )
	ROM_LOAD( "pal16l8b.b58",    0x0800, 0x0104, CRC(17e2c9b8) SHA1(841b290ee3a3089a975456b20018c04e6afb2257) )
	ROM_LOAD( "pal16l8b.b59",    0x0a00, 0x0104, CRC(dc0db200) SHA1(9f46e7edf053ad9ee3a4a6dd00f9f6996203fc60) )
	ROM_LOAD( "pal16l8b.b64",    0x0c00, 0x0104, CRC(3aed3d98) SHA1(8f0b02baad87aeda1767f0e95aa1a84545376076) )
ROM_END

ROM_START( elvactrj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e02-12.20", 0x000000, 0x80000, CRC(ea5f5a32) SHA1(4f30c56fbf068fee6d3afb2479043c7e89f6c055) )
	ROM_LOAD32_BYTE("e02-11.19", 0x000001, 0x80000, CRC(bcced8ff) SHA1(09c38c78300ba9d710b4e46ca71014bbc5ac46b4) )
	ROM_LOAD32_BYTE("e02-10.18", 0x000002, 0x80000, CRC(72f1b952) SHA1(9fc41ecfbee3581d9e92ff3a2ab6e4b93567e31d) )
	ROM_LOAD32_BYTE("e02-09.17", 0x000003, 0x80000, CRC(23997907) SHA1(e5b0b9069b29cf08e1a782c73f42137aec198f7f) )

	ROM_REGION(0x800000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD16_BYTE("e02-03.12", 0x000000, 0x200000, CRC(c884ebb5) SHA1(49009056bfdc564eac0ae6b7b49f070f05dc4ee3) )
	ROM_LOAD16_BYTE("e02-02.8",  0x000001, 0x200000, CRC(c8e06cfb) SHA1(071d095a4930ce18a782c577811b553a9705fbd7) )
	ROM_LOAD       ("e02-01.4",  0x600000, 0x200000, CRC(2ba94726) SHA1(3e9cdd076338e0e5358571ce4f97576f1a6a12a7) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2", 0 ) /* Tiles */
	ROM_LOAD16_BYTE("e02-08.47", 0x000000, 0x200000, CRC(29c9bd02) SHA1(a5b552ae7ac15f514ee6105410ec3e6e34ea0adb) )
	ROM_LOAD16_BYTE("e02-07.45", 0x000001, 0x200000, CRC(5eeee925) SHA1(d302da28df8ac6d406ef45f1d282ee22ce243857) )
	ROM_LOAD       ("e02-06.43", 0x600000, 0x200000, CRC(4c8726e9) SHA1(8ce2320a087f43c49428a39dafffec8c40d61b03) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e02-13.32", 0x100000, 0x40000, CRC(80932702) SHA1(c468234d03aa31b2aa0c3bd6bec32034216c2ae4) )
	ROM_LOAD16_BYTE("e02-14.33", 0x100001, 0x40000, CRC(706671a5) SHA1(1ac90647d617e73f12a67274a025ae43a6b3a316) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e02-04.38", 0x000000, 0x200000, CRC(b74307af) SHA1(deb42415049efa2df70e7b25ba8b1b716aa227f1) ) // C8 C9 CA CB
	ROM_LOAD16_BYTE("e02-05.39", 0x400000, 0x200000, CRC(eb729855) SHA1(85253efe794e8b5ffaf16bcb1123bca831e776a5) ) // CC CD CE CF
ROM_END

ROM_START( elvact2u )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e02-12.20", 0x000000, 0x80000, CRC(ea5f5a32) SHA1(4f30c56fbf068fee6d3afb2479043c7e89f6c055) )
	ROM_LOAD32_BYTE("e02-11.19", 0x000001, 0x80000, CRC(bcced8ff) SHA1(09c38c78300ba9d710b4e46ca71014bbc5ac46b4) )
	ROM_LOAD32_BYTE("e02-10.18", 0x000002, 0x80000, CRC(72f1b952) SHA1(9fc41ecfbee3581d9e92ff3a2ab6e4b93567e31d) )
	ROM_LOAD32_BYTE("e02-15.17", 0x000003, 0x80000, CRC(ba9028bd) SHA1(1d04ce5333143ed78ec297d89c0cdb99bf6e4bde) )

	ROM_REGION(0x800000, "gfx1", 0 ) /* Sprites */
	ROM_LOAD16_BYTE("e02-03.12", 0x000000, 0x200000, CRC(c884ebb5) SHA1(49009056bfdc564eac0ae6b7b49f070f05dc4ee3) )
	ROM_LOAD16_BYTE("e02-02.8",  0x000001, 0x200000, CRC(c8e06cfb) SHA1(071d095a4930ce18a782c577811b553a9705fbd7) )
	ROM_LOAD       ("e02-01.4",  0x600000, 0x200000, CRC(2ba94726) SHA1(3e9cdd076338e0e5358571ce4f97576f1a6a12a7) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2", 0 ) /* Tiles */
	ROM_LOAD16_BYTE("e02-08.47", 0x000000, 0x200000, CRC(29c9bd02) SHA1(a5b552ae7ac15f514ee6105410ec3e6e34ea0adb) )
	ROM_LOAD16_BYTE("e02-07.45", 0x000001, 0x200000, CRC(5eeee925) SHA1(d302da28df8ac6d406ef45f1d282ee22ce243857) )
	ROM_LOAD       ("e02-06.43", 0x600000, 0x200000, CRC(4c8726e9) SHA1(8ce2320a087f43c49428a39dafffec8c40d61b03) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e02-13.32", 0x100000, 0x40000, CRC(80932702) SHA1(c468234d03aa31b2aa0c3bd6bec32034216c2ae4) )
	ROM_LOAD16_BYTE("e02-14.33", 0x100001, 0x40000, CRC(706671a5) SHA1(1ac90647d617e73f12a67274a025ae43a6b3a316) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e02-04.38", 0x000000, 0x200000, CRC(b74307af) SHA1(deb42415049efa2df70e7b25ba8b1b716aa227f1) ) // C8 C9 CA CB
	ROM_LOAD16_BYTE("e02-05.39", 0x400000, 0x200000, CRC(eb729855) SHA1(85253efe794e8b5ffaf16bcb1123bca831e776a5) ) // CC CD CE CF
ROM_END

ROM_START( twinqix )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("mpr0-3.b60", 0x000000, 0x40000, CRC(1a63d0de) SHA1(7d8d8a6c9c7f9dfc0a8a528a905e33388b8fe13d) )
	ROM_LOAD32_BYTE("mpr0-2.b61", 0x000001, 0x40000, CRC(45a70987) SHA1(8cca6845064d943fd28416143e60399188b023cd) )
	ROM_LOAD32_BYTE("mpr0-1.b62", 0x000002, 0x40000, CRC(531f9447) SHA1(4d18efaad9c3dd2b14d3125c0f9e18cfcde3a1f2) )
	ROM_LOAD32_BYTE("mpr0-0.b63", 0x000003, 0x40000, CRC(a4c44c11) SHA1(b928134028bb8cddd6e34a501a4aad56173e2ae2) )

	ROM_REGION(0x200000, "gfx1" , 0 ) /* Sprites */
	ROM_LOAD16_BYTE("obj0-0.a08", 0x000000, 0x080000, CRC(c6ea845c) SHA1(9df710637e8f64f7fec232b5ebbede588e07c2db) )
	ROM_LOAD16_BYTE("obj0-1.a20", 0x000001, 0x080000, CRC(8c12b7fb) SHA1(8a52870fb9f508148619763fb6f37dd74b5386ca) )
	ROM_FILL       (              0x100000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0 ) /* Tiles */
	ROM_LOAD32_BYTE("scr0-0.b07",  0x000000, 0x080000, CRC(9a1b9b34) SHA1(ddf9c6ba0f9c340b580573e1d96ac76b1cd35beb) )
	ROM_LOAD32_BYTE("scr0-1.b06",  0x000002, 0x080000, CRC(e9bef879) SHA1(7e720f5054a1ef3a28353f1c221f4cf15d3b7428) )
	ROM_LOAD32_BYTE("scr0-2.b05",  0x000001, 0x080000, CRC(cac6854b) SHA1(c97fb7de48e1644695bbe431587d6c1be01ea62d) )
	ROM_LOAD32_BYTE("scr0-3.b04",  0x000003, 0x080000, CRC(ce063034) SHA1(2ecff74427d7d2fa8d1db4ac87481d123d7ce003) )
	ROM_LOAD16_BYTE("scr0-4.b03",  0x300000, 0x080000, CRC(d32280fe) SHA1(56b120128c5e4b8c6598a1de51269e6702a63175) )
	ROM_LOAD16_BYTE("scr0-5.b02",  0x300001, 0x080000, CRC(fdd1a85b) SHA1(1d94a4858baef3e78c456049dc58249a574205fe) )

	ROM_REGION(0x180000, "audiocpu", 0) /* sound CPU */
	ROM_LOAD16_BYTE("spr0-1.b66", 0x100000, 0x40000, CRC(4b20e99d) SHA1(faf184daea0f1131bafa50edb48bd470d4c0b141) )
	ROM_LOAD16_BYTE("spr0-0.b65", 0x100001, 0x40000, CRC(2569eb30) SHA1(ec804131025e600198cd8342925823340e7ef458) )

	ROM_REGION16_BE(0x400000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 1 populated
	ROM_LOAD16_BYTE("snd-0.b43",  0x000000, 0x80000, CRC(ad5405a9) SHA1(67ee42498d2c3c00015237b3b5cd020f9a7c4a18) ) // C8
	ROM_LOAD16_BYTE("snd-1.b44",  0x100000, 0x80000, CRC(274864af) SHA1(47fefee23038bb751bdf6b6f48312ba0b6e38b90) ) // C9
	ROM_LOAD16_BYTE("snd-14.b10", 0x200000, 0x80000, CRC(26312451) SHA1(9f947a11592fd8420fc581914bf16e7ade75390c) ) // -std-
	ROM_LOAD16_BYTE("snd-15.b11", 0x300000, 0x80000, CRC(2edaa9dc) SHA1(72fead505c4f44e5736ff7d545d72dfa37d613e2) ) // -std-

	ROM_REGION( 0x0c00, "plds", 0 )
	ROM_LOAD( "pal20l10a.a12", 0x0000, 0x00cc, NO_DUMP)  /* PAL is read protected */
	ROM_LOAD( "pal20l10a.a24", 0x0100, 0x00cc, NO_DUMP)  /* PAL is read protected */
	ROM_LOAD( "pal16l8b.b24",  0x0200, 0x0104, CRC(0b73a7d1) SHA1(953234ce95a8c2dfb9ba276742411cb2ca4d4bf8) )
	ROM_LOAD( "pal16l8b.b57",  0x0400, 0x0104, CRC(74b4d8be) SHA1(23ba316bc5550dd69f07f9a0f697927656a35e5a) )
	ROM_LOAD( "pal16l8b.b58",  0x0600, 0x0104, CRC(17e2c9b8) SHA1(841b290ee3a3089a975456b20018c04e6afb2257) )
	ROM_LOAD( "pal16l8b.b59",  0x0800, 0x0104, CRC(dc0db200) SHA1(9f46e7edf053ad9ee3a4a6dd00f9f6996203fc60) )
	ROM_LOAD( "pal16l8b.b64",  0x0a00, 0x0104, CRC(3aed3d98) SHA1(8f0b02baad87aeda1767f0e95aa1a84545376076) )
ROM_END

ROM_START( quizhuhu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e08-16.20", 0x000000, 0x80000, CRC(faa8f373) SHA1(e263b058288fcacf9b15188ab78e8fb05a8971a7) )
	ROM_LOAD32_BYTE("e08-15.19", 0x000001, 0x80000, CRC(23acf231) SHA1(a87933439b3d1d92f8b9d545b13f20cc47a7fd4e) )
	ROM_LOAD32_BYTE("e08-14.18", 0x000002, 0x80000, CRC(33a4951d) SHA1(69e8fe994f620ce056cdedca77bff1d0c6e74483) )
	ROM_LOAD32_BYTE("e08-13.17", 0x000003, 0x80000, CRC(0936fd2a) SHA1(f0f7017c755b28644b67b4fd6d5e19c272e9c3a2) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e08-06.12", 0x000000, 0x200000, CRC(8dadc9ac) SHA1(469e3c5063f5cb0832fb5bb5000ecd3c342cd095) )
	ROM_LOAD16_BYTE("e08-04.8",  0x000001, 0x200000, CRC(5423721d) SHA1(7e9f4492845b7b4df0336203b1da6ca5ffeb36de) )
	ROM_LOAD16_BYTE("e08-05.11", 0x400000, 0x100000, CRC(79d2e516) SHA1(7dc0c23f3995d14b443a3f67d488e5ab780e8a94) )
	ROM_LOAD16_BYTE("e08-03.7",  0x400001, 0x100000, CRC(07b9ab6a) SHA1(db205822233c385e1dbe4a9d40b311df9bca7053) )
	ROM_LOAD       ("e08-02.4",  0x900000, 0x200000, CRC(d89eb067) SHA1(bd8e1cf4c2046894c629d927fa05806b9b73505d) )
	ROM_LOAD       ("e08-01.3",  0xb00000, 0x100000, CRC(90223c06) SHA1(f07dae563946908d471ae89db74a2e55c5ab5890) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e08-12.47", 0x000000, 0x100000, CRC(6c711d36) SHA1(3fbff7783323d968ade72ac53531a7bcf7b9d234) )
	ROM_LOAD16_BYTE("e08-11.45", 0x000001, 0x100000, CRC(56775a60) SHA1(8bb8190101f2e8487ebb707022ff89d97bb7b39a) )
	ROM_LOAD       ("e08-10.43", 0x300000, 0x100000, CRC(60abc71b) SHA1(f4aa906920c6134c33a4dfb51724f3adbd3d7de4) )
	ROM_FILL       (             0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e08-18.32", 0x100000, 0x20000, CRC(e695497e) SHA1(9d845b4c0bd9b40471fb4b5ab2f9240058bc324f) )
	ROM_LOAD16_BYTE("e08-17.33", 0x100001, 0x20000, CRC(fafc7e4e) SHA1(26f46d5900fbf26d25651e7e818e486fc7a878ec) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e08-07.38", 0x400000, 0x200000, CRC(c05dc85b) SHA1(d46ae3f066bbe041edde40358dd54f93e8e195de) ) // C8 C9 CA CB
	ROM_LOAD16_BYTE("e08-08.39", 0x800000, 0x200000, CRC(3eb94a99) SHA1(e6e8832e87397811dfc40525f2a15fc0415cec68) ) // CC CD CE CF
	ROM_LOAD16_BYTE("e08-09.41", 0xc00000, 0x200000, CRC(200b26ee) SHA1(c689d0a1c1f5d71e0af3d94073b29d3619187c5f) ) // D0 D1 -std-
ROM_END


ROM_START( pbobble2 )
	ROM_REGION(0x200000, "maincpu", ROMREGION_ERASE00) /* 68020 code */
	ROM_LOAD32_BYTE("e10-22.bin", 0x000002, 0x40000, CRC(7b12105d) SHA1(b69379c5a79c365ed6e62e7ae478e4bbb4edfcb1) )
	ROM_LOAD32_BYTE("e10-23.bin", 0x000001, 0x40000, CRC(56a66435) SHA1(ba9d405416090f3482c6ed610e7eb77b43459ff1) )
	ROM_LOAD32_BYTE("e10-24.bin", 0x000000, 0x40000, CRC(f9d0794b) SHA1(320eee7790bf9a5141ad7b0ebfdec47e8f85a1c2) )
	ROM_LOAD32_BYTE("e10-25.bin", 0x000003, 0x40000, CRC(ff0407d3) SHA1(4616bb9132f78c4f0212afbcc8d528934f822f44) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e10-02.rom", 0x000000, 0x100000, CRC(c0564490) SHA1(cbe9f880192c08f4d1db21d5ba14073b97e5f1d3) )
	ROM_LOAD16_BYTE("e10-01.rom", 0x000001, 0x100000, CRC(8c26ff49) SHA1(cbb514c061106003d2ae2b6c43958b24feaad656) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e10-07.rom", 0x000000, 0x100000, CRC(dcb3c29b) SHA1(b80c3a8ce53d696c57675e654c9927ef8687759e) )
	ROM_LOAD16_BYTE("e10-06.rom", 0x000001, 0x100000, CRC(1b0f20e2) SHA1(66b44d059c2896abac2f0e7fc932489dee440ba0) )
	ROM_LOAD       ("e10-05.rom", 0x300000, 0x100000, CRC(81266151) SHA1(aa3b144f32995425db97efce440e234a3c7a6715) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e10-12.32", 0x100000, 0x40000, CRC(b92dc8ad) SHA1(0c1428d313507b1ae5a2af3b2fbaaa5650135e1e) )
	ROM_LOAD16_BYTE("e10-13.33", 0x100001, 0x40000, CRC(87842c13) SHA1(d15b47c7430e677ae172f86fd5be595e4fe72e42) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e10-04.rom", 0x000000, 0x200000, CRC(5c0862a6) SHA1(f916f63b8629239e3221e1e231e1b39962ef38ba) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e10-03.rom", 0x400000, 0x200000, CRC(46d68ac8) SHA1(ad014e9f0d458308014959ca6823077f581ab088) )    // CC CD CE CF

	ROM_REGION(0x2000, "extra", 0)
	ROM_LOAD("e10-21.bin", 0x000000, 0x117, CRC(458499b7) SHA1(0c49aaf75539587d1f5367b3dc72799003824544) )
//  ROM_LOAD("e10-21.jed", 0x000000, 0xc2b, CRC(8e9fa5d6) SHA1(5fb120d80f7ceee96a2fad863cf61a1f0b02877f) )
ROM_END


ROM_START( pbobble2o )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e10-11.20", 0x000000, 0x40000, CRC(b82f81da) SHA1(2cd0fb321c853497058545525f430b52c0788fb1) )
	ROM_LOAD32_BYTE("e10-10.19", 0x000001, 0x40000, CRC(f432267a) SHA1(f9778fc627773e4e254faa0ce10e68407251ce95) )
	ROM_LOAD32_BYTE("e10-09.18", 0x000002, 0x40000, CRC(e0b1b599) SHA1(99ef34b014db7c52f2ced05b2b90099a9c873259) )
	ROM_LOAD32_BYTE("e10-15.17", 0x000003, 0x40000, CRC(a2c0a268) SHA1(c96bb8a2959266c5c832fb77d119ad129b9ef9ee) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e10-02.rom", 0x000000, 0x100000, CRC(c0564490) SHA1(cbe9f880192c08f4d1db21d5ba14073b97e5f1d3) )
	ROM_LOAD16_BYTE("e10-01.rom", 0x000001, 0x100000, CRC(8c26ff49) SHA1(cbb514c061106003d2ae2b6c43958b24feaad656) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e10-07.rom", 0x000000, 0x100000, CRC(dcb3c29b) SHA1(b80c3a8ce53d696c57675e654c9927ef8687759e) )
	ROM_LOAD16_BYTE("e10-06.rom", 0x000001, 0x100000, CRC(1b0f20e2) SHA1(66b44d059c2896abac2f0e7fc932489dee440ba0) )
	ROM_LOAD       ("e10-05.rom", 0x300000, 0x100000, CRC(81266151) SHA1(aa3b144f32995425db97efce440e234a3c7a6715) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e10-12.32", 0x100000, 0x40000, CRC(b92dc8ad) SHA1(0c1428d313507b1ae5a2af3b2fbaaa5650135e1e) )
	ROM_LOAD16_BYTE("e10-13.33", 0x100001, 0x40000, CRC(87842c13) SHA1(d15b47c7430e677ae172f86fd5be595e4fe72e42) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e10-04.rom", 0x000000, 0x200000, CRC(5c0862a6) SHA1(f916f63b8629239e3221e1e231e1b39962ef38ba) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e10-03.rom", 0x400000, 0x200000, CRC(46d68ac8) SHA1(ad014e9f0d458308014959ca6823077f581ab088) )    // CC CD CE CF
ROM_END

ROM_START( pbobble2j )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e10-11.20", 0x000000, 0x40000, CRC(b82f81da) SHA1(2cd0fb321c853497058545525f430b52c0788fb1) )
	ROM_LOAD32_BYTE("e10-10.19", 0x000001, 0x40000, CRC(f432267a) SHA1(f9778fc627773e4e254faa0ce10e68407251ce95) )
	ROM_LOAD32_BYTE("e10-09.18", 0x000002, 0x40000, CRC(e0b1b599) SHA1(99ef34b014db7c52f2ced05b2b90099a9c873259) )
	ROM_LOAD32_BYTE("e10-08.17", 0x000003, 0x40000, CRC(4ccec344) SHA1(dfb30d149dde6d8e1a117bf0bafb85178540aa58) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e10-02.rom", 0x000000, 0x100000, CRC(c0564490) SHA1(cbe9f880192c08f4d1db21d5ba14073b97e5f1d3) )
	ROM_LOAD16_BYTE("e10-01.rom", 0x000001, 0x100000, CRC(8c26ff49) SHA1(cbb514c061106003d2ae2b6c43958b24feaad656) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e10-07.rom", 0x000000, 0x100000, CRC(dcb3c29b) SHA1(b80c3a8ce53d696c57675e654c9927ef8687759e) )
	ROM_LOAD16_BYTE("e10-06.rom", 0x000001, 0x100000, CRC(1b0f20e2) SHA1(66b44d059c2896abac2f0e7fc932489dee440ba0) )
	ROM_LOAD       ("e10-05.rom", 0x300000, 0x100000, CRC(81266151) SHA1(aa3b144f32995425db97efce440e234a3c7a6715) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e10-12.32", 0x100000, 0x40000, CRC(b92dc8ad) SHA1(0c1428d313507b1ae5a2af3b2fbaaa5650135e1e) )
	ROM_LOAD16_BYTE("e10-13.33", 0x100001, 0x40000, CRC(87842c13) SHA1(d15b47c7430e677ae172f86fd5be595e4fe72e42) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e10-04.rom", 0x000000, 0x200000, CRC(5c0862a6) SHA1(f916f63b8629239e3221e1e231e1b39962ef38ba) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e10-03.rom", 0x400000, 0x200000, CRC(46d68ac8) SHA1(ad014e9f0d458308014959ca6823077f581ab088) )    // CC CD CE CF
ROM_END

ROM_START( pbobble2u )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e10-20.20", 0x000000, 0x40000, CRC(97eb15c6) SHA1(712ed06ee3582e30acb03c06f4981b0f3d7c64f4) )
	ROM_LOAD32_BYTE("e10-19.19", 0x000001, 0x40000, CRC(7082d796) SHA1(9dd8216123ae94f4e9bacea9a088ae73c71cfd19) )
	ROM_LOAD32_BYTE("e10-18.18", 0x000002, 0x40000, CRC(2ffa3ef2) SHA1(dcf2cce623daaaacb53f17657019a4e334be0a16) )
	ROM_LOAD32_BYTE("e10-14.17", 0x000003, 0x40000, CRC(4a19ed67) SHA1(21de5f6c3ab5e13b085010a49f33b88cb6388fa9) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e10-02.rom", 0x000000, 0x100000, CRC(c0564490) SHA1(cbe9f880192c08f4d1db21d5ba14073b97e5f1d3) )
	ROM_LOAD16_BYTE("e10-01.rom", 0x000001, 0x100000, CRC(8c26ff49) SHA1(cbb514c061106003d2ae2b6c43958b24feaad656) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e10-07.rom", 0x000000, 0x100000, CRC(dcb3c29b) SHA1(b80c3a8ce53d696c57675e654c9927ef8687759e) )
	ROM_LOAD16_BYTE("e10-06.rom", 0x000001, 0x100000, CRC(1b0f20e2) SHA1(66b44d059c2896abac2f0e7fc932489dee440ba0) )
	ROM_LOAD       ("e10-05.rom", 0x300000, 0x100000, CRC(81266151) SHA1(aa3b144f32995425db97efce440e234a3c7a6715) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e10-16.32", 0x100000, 0x40000, CRC(765ce77a) SHA1(e2723bd6238da91d28307081909a7172a1825c83) )
	ROM_LOAD16_BYTE("e10-17.33", 0x100001, 0x40000, CRC(0aec3b1e) SHA1(a76a020cefcfbf86b0d893a6eb8ff93cb571abeb) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e10-04.rom", 0x000000, 0x200000, CRC(5c0862a6) SHA1(f916f63b8629239e3221e1e231e1b39962ef38ba) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e10-03.rom", 0x400000, 0x200000, CRC(46d68ac8) SHA1(ad014e9f0d458308014959ca6823077f581ab088) )    // CC CD CE CF
ROM_END

ROM_START( pbobble2x )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e10-29.20", 0x000000, 0x40000, CRC(f1e9ad3f) SHA1(8689d85f30e075d21e4be01a2a097a850a921c47) )
	ROM_LOAD32_BYTE("e10-28.19", 0x000001, 0x40000, CRC(412a3602) SHA1(d754e6ac886676d2c1eb52de3a727894f316e6b5) )
	ROM_LOAD32_BYTE("e10-27.18", 0x000002, 0x40000, CRC(88cc0b5c) SHA1(bb08a7b8b37356376052ed03f8515677811823c0) )
	ROM_LOAD32_BYTE("e10-26.17", 0x000003, 0x40000, CRC(a5c24047) SHA1(62861577ce0aedb8d05360f0302fceecbde15420) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e10-02.rom", 0x000000, 0x100000, CRC(c0564490) SHA1(cbe9f880192c08f4d1db21d5ba14073b97e5f1d3) )
	ROM_LOAD16_BYTE("e10-01.rom", 0x000001, 0x100000, CRC(8c26ff49) SHA1(cbb514c061106003d2ae2b6c43958b24feaad656) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e10-07.rom", 0x000000, 0x100000, CRC(dcb3c29b) SHA1(b80c3a8ce53d696c57675e654c9927ef8687759e) )
	ROM_LOAD16_BYTE("e10-06.rom", 0x000001, 0x100000, CRC(1b0f20e2) SHA1(66b44d059c2896abac2f0e7fc932489dee440ba0) )
	ROM_LOAD       ("e10-05.rom", 0x300000, 0x100000, CRC(81266151) SHA1(aa3b144f32995425db97efce440e234a3c7a6715) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e10-30.32", 0x100000, 0x40000, CRC(bb090c1e) SHA1(af2ff23d6f9bd56c25530cb9bf9f452b6f5210f5) )
	ROM_LOAD16_BYTE("e10-31.33", 0x100001, 0x40000, CRC(f4b88d65) SHA1(c74dcb4bed979039fad1d5c7528c14ce4db1d5ec) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e10-04.rom", 0x000000, 0x200000, CRC(5c0862a6) SHA1(f916f63b8629239e3221e1e231e1b39962ef38ba) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e10-03.rom", 0x400000, 0x200000, CRC(46d68ac8) SHA1(ad014e9f0d458308014959ca6823077f581ab088) )    // CC CD CE CF

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-14.bin", 0x0800, 0x0117, CRC(7427e777) SHA1(e692cedb13e5bc02edc4b25e9dcea51e6715de85) )
ROM_END

ROM_START( gekiridn )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e11-12.ic20", 0x000000, 0x40000, CRC(6a7aaacf) SHA1(a8114c84e76c75c908a61d985d96aa4eb9a0ac5a) )
	ROM_LOAD32_BYTE("e11-11.ic19", 0x000001, 0x40000, CRC(2284a08e) SHA1(3dcb91be0d3491ad5e77efd30bacd506dad0f848) )
	ROM_LOAD32_BYTE("e11-10.ic18", 0x000002, 0x40000, CRC(8795e6ba) SHA1(9128c29fdce3276f55aad47451e4a507470c8b9f) )
	ROM_LOAD32_BYTE("e11-15.ic17", 0x000003, 0x40000, CRC(5aef1fd8) SHA1(a94884e39172e664759bff53a6dd2f93422d3299) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e11-03.ic12", 0x000000, 0x200000, CRC(f73877c5) SHA1(1f6b7c0b8a0aaab3e5427d21de7fad3d3cbf737a) )
	ROM_LOAD16_BYTE("e11-02.ic8",  0x000001, 0x200000, CRC(5722a83b) SHA1(823c20a33016a5506ca5415ec615c3d2546ca9ab) )
	ROM_LOAD       ("e11-01.ic4",  0x600000, 0x200000, CRC(c2cd1069) SHA1(9744dd3d8a6d9200cea4429dafce5620b60e2960) )
	ROM_FILL       (               0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e11-08.ic47", 0x000000, 0x200000, CRC(907f69d3) SHA1(0899ed58edcae22144625c349c9d2fe4d46d11e3) )
	ROM_LOAD16_BYTE("e11-07.ic45", 0x000001, 0x200000, CRC(ef018607) SHA1(61b602b13754c3be21caf76acbfc10c87518ba47) )
	ROM_LOAD       ("e11-06.ic43", 0x600000, 0x200000, CRC(200ce305) SHA1(c80a0b96510913a6411e6763fb72bf413fb792da) )
	ROM_FILL       (               0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e11-13.ic32", 0x100000, 0x40000, CRC(f5c5486a) SHA1(4091f3ddb1e6cbc9dc89485e1e784a4b6fa191b7) )
	ROM_LOAD16_BYTE("e11-14.ic33", 0x100001, 0x40000, CRC(7fa10f96) SHA1(50efefd890535e952022a494c5b4e9b33bb90fad) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e11-04.ic38", 0x000000, 0x200000, CRC(e0ff4fb1) SHA1(81e186e3a692af1da316b8085a729c4f103d9a52) )   // C8 C9 CA CB
	ROM_LOAD16_BYTE("e11-05.ic41", 0x400000, 0x200000, CRC(a4d08cf1) SHA1(ae2cabef7b7bcb8a788988c73d7af6fa4bb2c444) )   // CC CD -std-
ROM_END

ROM_START( gekiridnj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e11-12.ic20", 0x000000, 0x40000, CRC(6a7aaacf) SHA1(a8114c84e76c75c908a61d985d96aa4eb9a0ac5a) )
	ROM_LOAD32_BYTE("e11-11.ic19", 0x000001, 0x40000, CRC(2284a08e) SHA1(3dcb91be0d3491ad5e77efd30bacd506dad0f848) )
	ROM_LOAD32_BYTE("e11-10.ic18", 0x000002, 0x40000, CRC(8795e6ba) SHA1(9128c29fdce3276f55aad47451e4a507470c8b9f) )
	ROM_LOAD32_BYTE("e11-09.ic17", 0x000003, 0x40000, CRC(b4e17ef4) SHA1(ab06ab68aaa487cc3046a15fef3dde8581197391) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e11-03.ic12", 0x000000, 0x200000, CRC(f73877c5) SHA1(1f6b7c0b8a0aaab3e5427d21de7fad3d3cbf737a) )
	ROM_LOAD16_BYTE("e11-02.ic8",  0x000001, 0x200000, CRC(5722a83b) SHA1(823c20a33016a5506ca5415ec615c3d2546ca9ab) )
	ROM_LOAD       ("e11-01.ic4",  0x600000, 0x200000, CRC(c2cd1069) SHA1(9744dd3d8a6d9200cea4429dafce5620b60e2960) )
	ROM_FILL       (               0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e11-08.ic47", 0x000000, 0x200000, CRC(907f69d3) SHA1(0899ed58edcae22144625c349c9d2fe4d46d11e3) )
	ROM_LOAD16_BYTE("e11-07.ic45", 0x000001, 0x200000, CRC(ef018607) SHA1(61b602b13754c3be21caf76acbfc10c87518ba47) )
	ROM_LOAD       ("e11-06.ic43", 0x600000, 0x200000, CRC(200ce305) SHA1(c80a0b96510913a6411e6763fb72bf413fb792da) )
	ROM_FILL       (               0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e11-13.ic32", 0x100000, 0x40000, CRC(f5c5486a) SHA1(4091f3ddb1e6cbc9dc89485e1e784a4b6fa191b7) )
	ROM_LOAD16_BYTE("e11-14.ic33", 0x100001, 0x40000, CRC(7fa10f96) SHA1(50efefd890535e952022a494c5b4e9b33bb90fad) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e11-04.ic38", 0x000000, 0x200000, CRC(e0ff4fb1) SHA1(81e186e3a692af1da316b8085a729c4f103d9a52) )   // C8 C9 CA CB
	ROM_LOAD16_BYTE("e11-05.ic41", 0x400000, 0x200000, CRC(a4d08cf1) SHA1(ae2cabef7b7bcb8a788988c73d7af6fa4bb2c444) )   // CC CD -std-
ROM_END

/*
    ROM Board: J9100361A ROM PCB
*/

ROM_START( tcobra2 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e15-14.ic20", 0x000000, 0x40000, CRC(b527b733) SHA1(19efd647ea9c277b306714fe79ebf40d5f9d2187) )
	ROM_LOAD32_BYTE("e15-13.ic19", 0x000001, 0x40000, CRC(0f03daf7) SHA1(de5aee5a339224dfe5e03a02d3ef5ffd5a39211e) )
	ROM_LOAD32_BYTE("e15-12.ic18", 0x000002, 0x40000, CRC(59d832f2) SHA1(27019b4121b1f8b0b9e141234192b3da1a4af718) )
	ROM_LOAD32_BYTE("e15-18.ic17", 0x000003, 0x40000, CRC(4908c3aa) SHA1(9b0230e6bafd0533ecbe89bc18fae6f3425ea1a3) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e15-04.ic12", 0x000000, 0x200000, CRC(6ea8d9bd) SHA1(c31644e89752325ba2f174b60e31bd9659479391) )
	ROM_LOAD16_BYTE("e15-02.ic8",  0x000001, 0x200000, CRC(bf1232aa) SHA1(1381bae2a18ed62f4ca28bcdaf07debfc9bf21af) )
	ROM_LOAD16_BYTE("e15-03.ic11", 0x400000, 0x100000, CRC(be45a52f) SHA1(5d9735a774233b43003057cbab6ae7d6e0195dd2) )
	ROM_LOAD16_BYTE("e15-01.ic7",  0x400001, 0x100000, CRC(85421aac) SHA1(327e72f0107e024ec9fc9dc20d381e2e20f36248) )
	ROM_FILL       (               0x600000, 0x600000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e15-10.ic47", 0x000000, 0x200000, CRC(d8c96b00) SHA1(9cd275abb66b3475433ea2649dc872d7d35eb5b8) )
	ROM_LOAD16_BYTE("e15-08.ic45", 0x000001, 0x200000, CRC(4bdb2bf3) SHA1(1146b7a5d9f26d3173a7c64768e55d53a0ab7b8e) )
	ROM_LOAD16_BYTE("e15-09.ic46", 0x400000, 0x100000, CRC(07c29f60) SHA1(3ca0f632e7047cc50ee3ce24cd6c0c8c7252a278) )
	ROM_LOAD16_BYTE("e15-07.ic44", 0x400001, 0x100000, CRC(8164f7ee) SHA1(4550521f820e93ec08b86d148135966d016cbf22) )
	ROM_FILL       (               0x600000, 0x600000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e15-15.ic32", 0x100000, 0x20000, CRC(22126dfb) SHA1(a1af17e5c3440f1bab50d79f92c251f1a4536ca0) )
	ROM_LOAD16_BYTE("e15-16.ic33", 0x100001, 0x20000, CRC(f8b58ea0) SHA1(c9e196620765efc4c7b535793a5d1f586698ce55) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e15-05.ic38", 0x000000, 0x200000, CRC(3e5da5f6) SHA1(da6fc8b26cd02c45cfc0f1aa5292614e4d28cae4) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e15-06.ic41", 0x400000, 0x200000, CRC(b182a3e1) SHA1(db8569b069911bb84900b2aa5168c45ba3e985c7) )    // CC CD -std-

	ROM_REGION( 0x034a, "pals", 0 )
	ROM_LOAD( "d77-12.ic48.bin",    0x0000, 0x0117, BAD_DUMP CRC(6f93a4d8) SHA1(8c69688cf1159691439ebc4edfba52ab13f645b9) ) /* D77-12 @ IC48 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "d77-14.ic21.bin",    0x0118, 0x0117, BAD_DUMP CRC(f2264f51) SHA1(6f18bad9e5318fa40dbce32c0a036b7588651660) ) /* D77-14 @ IC21 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "palce16v8.ic37.bin", 0x0230, 0x0117, BAD_DUMP CRC(6ccd8168) SHA1(98f85455585ba2f5ab834fa30addec498e94f814) ) /* Label unreadable @ IC37 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "d77-09.ic14.bin",    0x0348, 0x0001, NO_DUMP) /* D77-09 @ IC14 (PAL16L8ACN) */
	ROM_LOAD( "d77-10.ic28.bin",    0x0349, 0x0001, NO_DUMP) /* D77-10 @ IC28 (PAL16L8ACN) */
ROM_END

ROM_START( tcobra2u )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e15-14.ic20", 0x000000, 0x40000, CRC(b527b733) SHA1(19efd647ea9c277b306714fe79ebf40d5f9d2187) )
	ROM_LOAD32_BYTE("e15-13.ic19", 0x000001, 0x40000, CRC(0f03daf7) SHA1(de5aee5a339224dfe5e03a02d3ef5ffd5a39211e) )
	ROM_LOAD32_BYTE("e15-12.ic18", 0x000002, 0x40000, CRC(59d832f2) SHA1(27019b4121b1f8b0b9e141234192b3da1a4af718) )
	ROM_LOAD32_BYTE("e15-17.ic17", 0x000003, 0x40000, CRC(3e0ff33c) SHA1(6da0a69272172e03921417f3949817756c7894b4) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e15-04.ic12", 0x000000, 0x200000, CRC(6ea8d9bd) SHA1(c31644e89752325ba2f174b60e31bd9659479391) )
	ROM_LOAD16_BYTE("e15-02.ic8",  0x000001, 0x200000, CRC(bf1232aa) SHA1(1381bae2a18ed62f4ca28bcdaf07debfc9bf21af) )
	ROM_LOAD16_BYTE("e15-03.ic11", 0x400000, 0x100000, CRC(be45a52f) SHA1(5d9735a774233b43003057cbab6ae7d6e0195dd2) )
	ROM_LOAD16_BYTE("e15-01.ic7",  0x400001, 0x100000, CRC(85421aac) SHA1(327e72f0107e024ec9fc9dc20d381e2e20f36248) )
	ROM_FILL       (               0x600000, 0x600000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e15-10.ic47", 0x000000, 0x200000, CRC(d8c96b00) SHA1(9cd275abb66b3475433ea2649dc872d7d35eb5b8) )
	ROM_LOAD16_BYTE("e15-08.ic45", 0x000001, 0x200000, CRC(4bdb2bf3) SHA1(1146b7a5d9f26d3173a7c64768e55d53a0ab7b8e) )
	ROM_LOAD16_BYTE("e15-09.ic46", 0x400000, 0x100000, CRC(07c29f60) SHA1(3ca0f632e7047cc50ee3ce24cd6c0c8c7252a278) )
	ROM_LOAD16_BYTE("e15-07.ic44", 0x400001, 0x100000, CRC(8164f7ee) SHA1(4550521f820e93ec08b86d148135966d016cbf22) )
	ROM_FILL       (               0x600000, 0x600000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e15-15.ic32", 0x100000, 0x20000, CRC(22126dfb) SHA1(a1af17e5c3440f1bab50d79f92c251f1a4536ca0) )
	ROM_LOAD16_BYTE("e15-16.ic33", 0x100001, 0x20000, CRC(f8b58ea0) SHA1(c9e196620765efc4c7b535793a5d1f586698ce55) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e15-05.ic38", 0x000000, 0x200000, CRC(3e5da5f6) SHA1(da6fc8b26cd02c45cfc0f1aa5292614e4d28cae4) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e15-06.ic41", 0x400000, 0x200000, CRC(b182a3e1) SHA1(db8569b069911bb84900b2aa5168c45ba3e985c7) )    // CC CD -std-

	ROM_REGION( 0x034a, "pals", 0 )
	ROM_LOAD( "d77-12.ic48.bin",    0x0000, 0x0117, BAD_DUMP CRC(6f93a4d8) SHA1(8c69688cf1159691439ebc4edfba52ab13f645b9) ) /* D77-12 @ IC48 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "d77-14.ic21.bin",    0x0118, 0x0117, BAD_DUMP CRC(f2264f51) SHA1(6f18bad9e5318fa40dbce32c0a036b7588651660) ) /* D77-14 @ IC21 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "palce16v8.ic37.bin", 0x0230, 0x0117, BAD_DUMP CRC(6ccd8168) SHA1(98f85455585ba2f5ab834fa30addec498e94f814) ) /* Label unreadable @ IC37 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "d77-09.ic14.bin",    0x0348, 0x0001, NO_DUMP) /* D77-09 @ IC14 (PAL16L8ACN) */
	ROM_LOAD( "d77-10.ic28.bin",    0x0349, 0x0001, NO_DUMP) /* D77-10 @ IC28 (PAL16L8ACN) */
ROM_END

ROM_START( ktiger2 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e15-14.ic20", 0x000000, 0x40000, CRC(b527b733) SHA1(19efd647ea9c277b306714fe79ebf40d5f9d2187) )
	ROM_LOAD32_BYTE("e15-13.ic19", 0x000001, 0x40000, CRC(0f03daf7) SHA1(de5aee5a339224dfe5e03a02d3ef5ffd5a39211e) )
	ROM_LOAD32_BYTE("e15-12.ic18", 0x000002, 0x40000, CRC(59d832f2) SHA1(27019b4121b1f8b0b9e141234192b3da1a4af718) )
	ROM_LOAD32_BYTE("e15-11.ic17", 0x000003, 0x40000, CRC(a706a286) SHA1(c3d1cdb0c5b1004acadc926ffd9083c9afea8608) )

	ROM_REGION(0xc00000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e15-04.ic12", 0x000000, 0x200000, CRC(6ea8d9bd) SHA1(c31644e89752325ba2f174b60e31bd9659479391) )
	ROM_LOAD16_BYTE("e15-02.ic8",  0x000001, 0x200000, CRC(bf1232aa) SHA1(1381bae2a18ed62f4ca28bcdaf07debfc9bf21af) )
	ROM_LOAD16_BYTE("e15-03.ic11", 0x400000, 0x100000, CRC(be45a52f) SHA1(5d9735a774233b43003057cbab6ae7d6e0195dd2) )
	ROM_LOAD16_BYTE("e15-01.ic7",  0x400001, 0x100000, CRC(85421aac) SHA1(327e72f0107e024ec9fc9dc20d381e2e20f36248) )
	ROM_FILL       (               0x600000, 0x600000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e15-10.ic47", 0x000000, 0x200000, CRC(d8c96b00) SHA1(9cd275abb66b3475433ea2649dc872d7d35eb5b8) )
	ROM_LOAD16_BYTE("e15-08.ic45", 0x000001, 0x200000, CRC(4bdb2bf3) SHA1(1146b7a5d9f26d3173a7c64768e55d53a0ab7b8e) )
	ROM_LOAD16_BYTE("e15-09.ic46", 0x400000, 0x100000, CRC(07c29f60) SHA1(3ca0f632e7047cc50ee3ce24cd6c0c8c7252a278) )
	ROM_LOAD16_BYTE("e15-07.ic44", 0x400001, 0x100000, CRC(8164f7ee) SHA1(4550521f820e93ec08b86d148135966d016cbf22) )
	ROM_FILL       (               0x600000, 0x600000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e15-15.ic32", 0x100000, 0x20000, CRC(22126dfb) SHA1(a1af17e5c3440f1bab50d79f92c251f1a4536ca0) )
	ROM_LOAD16_BYTE("e15-16.ic33", 0x100001, 0x20000, CRC(f8b58ea0) SHA1(c9e196620765efc4c7b535793a5d1f586698ce55) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e15-05.ic38", 0x000000, 0x200000, CRC(3e5da5f6) SHA1(da6fc8b26cd02c45cfc0f1aa5292614e4d28cae4) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e15-06.ic41", 0x400000, 0x200000, CRC(b182a3e1) SHA1(db8569b069911bb84900b2aa5168c45ba3e985c7) )    // CC CD -std-

	ROM_REGION( 0x034a, "pals", 0 )
	ROM_LOAD( "d77-12.ic48.bin",    0x0000, 0x0117, BAD_DUMP CRC(6f93a4d8) SHA1(8c69688cf1159691439ebc4edfba52ab13f645b9) ) /* D77-12 @ IC48 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "d77-14.ic21.bin",    0x0118, 0x0117, BAD_DUMP CRC(f2264f51) SHA1(6f18bad9e5318fa40dbce32c0a036b7588651660) ) /* D77-14 @ IC21 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "palce16v8.ic37.bin", 0x0230, 0x0117, BAD_DUMP CRC(6ccd8168) SHA1(98f85455585ba2f5ab834fa30addec498e94f814) ) /* Label unreadable @ IC37 (PALCE16V8Q-15PC/4) */
	ROM_LOAD( "d77-09.ic14.bin",    0x0348, 0x0001, NO_DUMP) /* D77-09 @ IC14 (PAL16L8ACN) */
	ROM_LOAD( "d77-10.ic28.bin",    0x0349, 0x0001, NO_DUMP) /* D77-10 @ IC28 (PAL16L8ACN) */
ROM_END

ROM_START( bubblem )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e21-21.20", 0x000000, 0x080000, CRC(cac4169c) SHA1(e1e5b9bbaecfd29ee764c8b29df8ffd08ef01866) )
	ROM_LOAD32_BYTE("e21-20.19", 0x000001, 0x080000, CRC(7727c673) SHA1(cda3dbcf8da06e81b899008462bcd6b2ea43db81) )
	ROM_LOAD32_BYTE("e21-19.18", 0x000002, 0x080000, CRC(be0b907d) SHA1(8bb6a149a4b0ccdb32396f7e750218a0bdc31965) )
	ROM_LOAD32_BYTE("e21-18.17", 0x000003, 0x080000, CRC(d14e313a) SHA1(3913d396a6a72f539163c216809e54a06ecd3b96) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e21-02.rom", 0x000000, 0x200000, CRC(b7cb9232) SHA1(ba71cb98d49eadebb26d9f53bbaec1dc211077f5) )
	ROM_LOAD16_BYTE("e21-01.rom", 0x000001, 0x200000, CRC(a11f2f99) SHA1(293c5996600cad05bf98f936f5f820d93d546099) )
	ROM_FILL       (              0x400000, 0x400000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e21-07.rom", 0x000000, 0x100000, CRC(7789bf7c) SHA1(bc8ef1696adac99a1fabae9b79afcd3461cf323b) )
	ROM_LOAD16_BYTE("e21-06.rom", 0x000001, 0x100000, CRC(997fc0d7) SHA1(58a546f739072fedebfe7c972fe85f72107726b2) )
	ROM_LOAD       ("e21-05.rom", 0x300000, 0x100000, CRC(07eab58f) SHA1(ae2d7b839b39d88d11652df74804a39230674467) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* Sound CPU */
	ROM_LOAD16_BYTE("e21-12.32", 0x100000, 0x40000, CRC(34093de1) SHA1(d69d6b5f10b8fe86f727d739ed5aecceb15e01f7) )
	ROM_LOAD16_BYTE("e21-13.33", 0x100001, 0x40000, CRC(9e9ec437) SHA1(b0265b688846c642d240b2f3677d2330d31eaa87) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e21-03.rom", 0x000000, 0x200000, CRC(54c5f83d) SHA1(10a993199c8d5a1361bd29a4b92c404451c6da01) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e21-04.rom", 0x400000, 0x200000, CRC(e5af2a2d) SHA1(62a49504decc7160b710260218920d2d6d2af8f0) )    // CC CD -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "bubblem.nv", 0x0000, 0x0080, CRC(9a59326e) SHA1(071dbfbfd77f7020476ddb54c93f5fafa7a08159) )
ROM_END

ROM_START( bubblemj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e21-11.20", 0x000000, 0x080000, CRC(df0eeae4) SHA1(4cc8d350da881947c1b9c4e0b8fbe220494f6c38) )
	ROM_LOAD32_BYTE("e21-10.19", 0x000001, 0x080000, CRC(cdfb58f6) SHA1(70d2b8228ab4ddd572fe2ee53c1b7205b66ef6a3) )
	ROM_LOAD32_BYTE("e21-09.18", 0x000002, 0x080000, CRC(6c305f17) SHA1(c4118722d697ccf54b43626a47673892a6c2caaf) )
	ROM_LOAD32_BYTE("e21-08.17", 0x000003, 0x080000, CRC(27381ae2) SHA1(29b5d4bafa4ac02d35cb3ed7b7461e749ef2d6d6) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e21-02.rom", 0x000000, 0x200000, CRC(b7cb9232) SHA1(ba71cb98d49eadebb26d9f53bbaec1dc211077f5) )
	ROM_LOAD16_BYTE("e21-01.rom", 0x000001, 0x200000, CRC(a11f2f99) SHA1(293c5996600cad05bf98f936f5f820d93d546099) )
	ROM_FILL       (              0x400000, 0x400000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e21-07.rom", 0x000000, 0x100000, CRC(7789bf7c) SHA1(bc8ef1696adac99a1fabae9b79afcd3461cf323b) )
	ROM_LOAD16_BYTE("e21-06.rom", 0x000001, 0x100000, CRC(997fc0d7) SHA1(58a546f739072fedebfe7c972fe85f72107726b2) )
	ROM_LOAD       ("e21-05.rom", 0x300000, 0x100000, CRC(07eab58f) SHA1(ae2d7b839b39d88d11652df74804a39230674467) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* Sound CPU */
	ROM_LOAD16_BYTE("e21-12.32", 0x100000, 0x40000, CRC(34093de1) SHA1(d69d6b5f10b8fe86f727d739ed5aecceb15e01f7) )
	ROM_LOAD16_BYTE("e21-13.33", 0x100001, 0x40000, CRC(9e9ec437) SHA1(b0265b688846c642d240b2f3677d2330d31eaa87) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e21-03.rom", 0x000000, 0x200000, CRC(54c5f83d) SHA1(10a993199c8d5a1361bd29a4b92c404451c6da01) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e21-04.rom", 0x400000, 0x200000, CRC(e5af2a2d) SHA1(62a49504decc7160b710260218920d2d6d2af8f0) )    // CC CD -std-

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "bubblemj.nv", 0x0000, 0x0080, CRC(cb4ef35c) SHA1(e0202b775d2494b77b1c08bafbfd239e40555dc6) )
ROM_END

ROM_START( cleopatr )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e28-10.bin", 0x000000, 0x80000, CRC(013fbc39) SHA1(d36ac44609b88e1da35c98dda381042e0112ea00) )
	ROM_LOAD32_BYTE("e28-09.bin", 0x000001, 0x80000, CRC(1c48a1f9) SHA1(791d321c03073cdd0269b970f926897446d2a6fb) )
	ROM_LOAD32_BYTE("e28-08.bin", 0x000002, 0x80000, CRC(7564f199) SHA1(ec4b19edb0660ad478f6c0ec27d701368696a2e4) )
	ROM_LOAD32_BYTE("e28-07.bin", 0x000003, 0x80000, CRC(a507797b) SHA1(6fa04091df1fa8c08f03b1ee378b4ec4a6ef7f51) )

	ROM_REGION(0x200000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e28-02.bin", 0x000000, 0x080000, CRC(b20d47cb) SHA1(6888e5564688840fed1c123ab38467066cd59c7f) )
	ROM_LOAD16_BYTE("e28-01.bin", 0x000001, 0x080000, CRC(4440e659) SHA1(71dece81bac8d638473c6531fed5c32798096af9) )
	ROM_FILL       (              0x100000, 0x100000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e28-06.bin", 0x000000, 0x100000, CRC(21d0c454) SHA1(f4c815984b19321cfab303fa6f21d9cad35b09f2) )
	ROM_LOAD16_BYTE("e28-05.bin", 0x000001, 0x100000, CRC(2870dbbc) SHA1(4e412b90cbd9a05956cde3d8cff615ebadca9db6) )
	ROM_LOAD       ("e28-04.bin", 0x300000, 0x100000, CRC(57aef029) SHA1(5c07209015d4749d1ffb3e9c1a890e6cfeec8cb0) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x140000, "audiocpu", 0) /* Sound CPU */
	ROM_LOAD16_BYTE("e28-11.bin", 0x100000, 0x20000, CRC(01a06950) SHA1(94d22cd839f9027e9d45264c366e0cb5d698e0b6) )
	ROM_LOAD16_BYTE("e28-12.bin", 0x100001, 0x20000, CRC(dc19260f) SHA1(fa0ca03a236326652e4f9898d07cd837c1507a9d) )

	ROM_REGION16_BE(0x400000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 1 populated
	ROM_LOAD16_BYTE("e28-03.bin", 0x000000, 0x200000, CRC(15c7989d) SHA1(7cc63d93e5c1f9f52f889e973bbefd5e6f7ce807) )    // C8 C9 CA CB
ROM_END

ROM_START( pbobble3 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e29-12.rom", 0x000000, 0x80000, CRC(9eb19a00) SHA1(5a6417e4377070f9f01110dc6d513d0de01cff1e) )
	ROM_LOAD32_BYTE("e29-11.rom", 0x000001, 0x80000, CRC(e54ada97) SHA1(325e2bc7156656cc262989910dde07a1746cf790) )
	ROM_LOAD32_BYTE("e29-10.rom", 0x000002, 0x80000, CRC(1502a122) SHA1(cb981a4578aa30276c491a0ef47f5e05c05d8b28) )
	ROM_LOAD32_BYTE("e29-16.rom", 0x000003, 0x80000, CRC(aac293da) SHA1(2188d1abe6aeefa872cf16db40999574497d982e) )

	ROM_REGION(0x400000, "gfx1" , 0 ) /* Sprites */
	ROM_LOAD16_BYTE("e29-02.rom", 0x000000, 0x100000, CRC(437391d3) SHA1(b3cc64c68553d37e0bd09e0dece14901d8df5866) )
	ROM_LOAD16_BYTE("e29-01.rom", 0x000001, 0x100000, CRC(52547c77) SHA1(d0cc8b8915cec1506c9733a1ce1638038ea93d25) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0 ) /* Tiles */
	ROM_LOAD16_BYTE("e29-08.rom", 0x000000, 0x100000, CRC(7040a3d5) SHA1(ea284ec530aac20348f84122e38a508bbc283f44) )
	ROM_LOAD16_BYTE("e29-07.rom", 0x000001, 0x100000, CRC(fca2ea9b) SHA1(a87ebedd0d16657288df434a70b8933fafe0ca25) )
	ROM_LOAD       ("e29-06.rom", 0x300000, 0x100000, CRC(c16184f8) SHA1(ded417d9d116b5a2f7518fa404bc2dda1c6a6366) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("e29-13.rom", 0x100000, 0x40000, CRC(1ef551ef) SHA1(527defe8f35314304adb4b483285b08cd6ebe865) )
	ROM_LOAD16_BYTE("e29-14.rom", 0x100001, 0x40000, CRC(7ee7e688) SHA1(d65aa9c449e1d64f10d1be9727a9d93ab1571e65) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e29-03.rom", 0x400000, 0x200000, CRC(a4371658) SHA1(26510a3f6de97f49b10dfc5cb9b7da947a44bfcb) )    // CE CF D0 D1
	ROM_LOAD16_BYTE("e29-04.rom", 0x800000, 0x200000, CRC(d1f42457) SHA1(2c77be6365deb5ef215da0c66da23b415623bdb1) )    // D2 C8 C9 CA
	ROM_LOAD16_BYTE("e29-05.rom", 0xc00000, 0x200000, CRC(e33c1234) SHA1(84c336ed6fd8723e824889fe7b52c284be659e62) )    // CB CC -std-
ROM_END

ROM_START( pbobble3u )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e29-12.rom", 0x000000, 0x80000, CRC(9eb19a00) SHA1(5a6417e4377070f9f01110dc6d513d0de01cff1e) )
	ROM_LOAD32_BYTE("e29-11.rom", 0x000001, 0x80000, CRC(e54ada97) SHA1(325e2bc7156656cc262989910dde07a1746cf790) )
	ROM_LOAD32_BYTE("e29-10.rom", 0x000002, 0x80000, CRC(1502a122) SHA1(cb981a4578aa30276c491a0ef47f5e05c05d8b28) )
	ROM_LOAD32_BYTE("e29-15.bin", 0x000003, 0x80000, CRC(ddc5a34c) SHA1(f38c99ac33b199b3ed99a84c67984f23a864e5d4) )

	ROM_REGION(0x400000, "gfx1" , 0 ) /* Sprites */
	ROM_LOAD16_BYTE("e29-02.rom", 0x000000, 0x100000, CRC(437391d3) SHA1(b3cc64c68553d37e0bd09e0dece14901d8df5866) )
	ROM_LOAD16_BYTE("e29-01.rom", 0x000001, 0x100000, CRC(52547c77) SHA1(d0cc8b8915cec1506c9733a1ce1638038ea93d25) )
	ROM_FILL       (              0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0 ) /* Tiles */
	ROM_LOAD16_BYTE("e29-08.rom", 0x000000, 0x100000, CRC(7040a3d5) SHA1(ea284ec530aac20348f84122e38a508bbc283f44) )
	ROM_LOAD16_BYTE("e29-07.rom", 0x000001, 0x100000, CRC(fca2ea9b) SHA1(a87ebedd0d16657288df434a70b8933fafe0ca25) )
	ROM_LOAD       ("e29-06.rom", 0x300000, 0x100000, CRC(c16184f8) SHA1(ded417d9d116b5a2f7518fa404bc2dda1c6a6366) )
	ROM_FILL       (              0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("e29-13.rom", 0x100000, 0x40000, CRC(1ef551ef) SHA1(527defe8f35314304adb4b483285b08cd6ebe865) )
	ROM_LOAD16_BYTE("e29-14.rom", 0x100001, 0x40000, CRC(7ee7e688) SHA1(d65aa9c449e1d64f10d1be9727a9d93ab1571e65) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e29-03.rom", 0x400000, 0x200000, CRC(a4371658) SHA1(26510a3f6de97f49b10dfc5cb9b7da947a44bfcb) )    // CE CF D0 D1
	ROM_LOAD16_BYTE("e29-04.rom", 0x800000, 0x200000, CRC(d1f42457) SHA1(2c77be6365deb5ef215da0c66da23b415623bdb1) )    // D2 C8 C9 CA
	ROM_LOAD16_BYTE("e29-05.rom", 0xc00000, 0x200000, CRC(e33c1234) SHA1(84c336ed6fd8723e824889fe7b52c284be659e62) )    // CB CC -std-
ROM_END

ROM_START( pbobble3j )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e29-12.ic20", 0x000000, 0x80000, CRC(9eb19a00) SHA1(5a6417e4377070f9f01110dc6d513d0de01cff1e) )
	ROM_LOAD32_BYTE("e29-11.ic19", 0x000001, 0x80000, CRC(e54ada97) SHA1(325e2bc7156656cc262989910dde07a1746cf790) )
	ROM_LOAD32_BYTE("e29-10.ic18", 0x000002, 0x80000, CRC(1502a122) SHA1(cb981a4578aa30276c491a0ef47f5e05c05d8b28) )
	ROM_LOAD32_BYTE("e29-09.ic17", 0x000003, 0x80000, CRC(44ccf2f6) SHA1(60877525feaa992b1b374acfb5c16439e5f32161) )

	ROM_REGION(0x400000, "gfx1" , 0 ) /* Sprites */
	ROM_LOAD16_BYTE("e29-02.ic8",  0x000000, 0x100000, CRC(437391d3) SHA1(b3cc64c68553d37e0bd09e0dece14901d8df5866) )
	ROM_LOAD16_BYTE("e29-01.ic12", 0x000001, 0x100000, CRC(52547c77) SHA1(d0cc8b8915cec1506c9733a1ce1638038ea93d25) )
	ROM_FILL       (               0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0 ) /* Tiles */
	ROM_LOAD16_BYTE("e29-08.ic47", 0x000000, 0x100000, CRC(7040a3d5) SHA1(ea284ec530aac20348f84122e38a508bbc283f44) )
	ROM_LOAD16_BYTE("e29-07.ic45", 0x000001, 0x100000, CRC(fca2ea9b) SHA1(a87ebedd0d16657288df434a70b8933fafe0ca25) )
	ROM_LOAD       ("e29-06.ic43", 0x300000, 0x100000, CRC(c16184f8) SHA1(ded417d9d116b5a2f7518fa404bc2dda1c6a6366) )
	ROM_FILL       (               0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("e29-13.ic32", 0x100000, 0x40000, CRC(1ef551ef) SHA1(527defe8f35314304adb4b483285b08cd6ebe865) )
	ROM_LOAD16_BYTE("e29-14.ic33", 0x100001, 0x40000, CRC(7ee7e688) SHA1(d65aa9c449e1d64f10d1be9727a9d93ab1571e65) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e29-03.ic38", 0x400000, 0x200000, CRC(a4371658) SHA1(26510a3f6de97f49b10dfc5cb9b7da947a44bfcb) )    // CE CF D0 D1
	ROM_LOAD16_BYTE("e29-04.ic39", 0x800000, 0x200000, CRC(d1f42457) SHA1(2c77be6365deb5ef215da0c66da23b415623bdb1) )    // D2 C8 C9 CA
	ROM_LOAD16_BYTE("e29-05.ic41", 0xc00000, 0x200000, CRC(e33c1234) SHA1(84c336ed6fd8723e824889fe7b52c284be659e62) )    // CB CC -std-

	ROM_REGION(0x034a, "pals", 0)
	ROM_LOAD("d77-12.ic48.bin", 0x0000, 0x0001, NO_DUMP) /* PALCE16V8Q-15PC/4 */
	ROM_LOAD("d77-14.ic21.bin", 0x0001, 0x0001, NO_DUMP) /* PALCE16V8Q-15PC/4 */
	ROM_LOAD("d77-11.ic37.bin", 0x0002, 0x0001, NO_DUMP) /* PALCE16V8Q-15PC/4 */
	ROM_LOAD("d77-09.ic14.bin", 0x0003, 0x0001, NO_DUMP) /* PAL16L8ACN */
	ROM_LOAD("d77-10.ic28.bin", 0x0004, 0x0001, NO_DUMP) /* PAL16L8ACN */
ROM_END

ROM_START( arkretrn )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e36-11.20", 0x000000, 0x040000, CRC(b50cfb92) SHA1(dac69fc9ef03315b11bb94d19e3dfdc8867b08ed) )
	ROM_LOAD32_BYTE("e36-10.19", 0x000001, 0x040000, CRC(c940dba1) SHA1(ec87c9e4250f8b2f15094681a4783bca8c68f576) )
	ROM_LOAD32_BYTE("e36-09.18", 0x000002, 0x040000, CRC(f16985e0) SHA1(a74cfee8f958e7a32354d4353eeb199a7fb1ce64) )
	ROM_LOAD32_BYTE("e36-15.17", 0x000003, 0x040000, CRC(4467ff37) SHA1(509a0d516def02d86d81b9868de0d9593539e65c) )

	ROM_REGION(0x180000, "audiocpu", 0) /* Sound CPU */
	ROM_LOAD16_BYTE("e36-12.32", 0x100000, 0x40000, CRC(3bae39be) SHA1(777142ecc24799b934ed51ac4cd8700bb6da7e3c) )
	ROM_LOAD16_BYTE("e36-13.33", 0x100001, 0x40000, CRC(94448e82) SHA1(d7766490318623be770545918391c5e6144dd619) )

	ROM_REGION(0x100000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e36-03.12", 0x000000, 0x040000, CRC(1ea8558b) SHA1(b8ea4d6e1fb551b3c47f336a5e60ec33f7be525f) )
	ROM_LOAD16_BYTE("e36-02.8",  0x000001, 0x040000, CRC(694eda31) SHA1(1a6f85057395052571491f85c633d5632ab64865) )
	ROM_LOAD       ("e36-01.4",  0x0c0000, 0x040000, CRC(54b9b2cd) SHA1(55ae964ea1d2cc40a6578c5339754a270096f01f) )
	ROM_FILL       (             0x080000, 0x040000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e36-07.47", 0x000000, 0x080000, CRC(266bf1c1) SHA1(489549478d7016400af2e643d4b98bf605237d34) )
	ROM_LOAD16_BYTE("e36-06.45", 0x000001, 0x080000, CRC(110ab729) SHA1(0ccc0a5abbcfd79a069daf5162cd344a5fb225d5) )
	ROM_LOAD       ("e36-05.43", 0x180000, 0x080000, CRC(db18bce2) SHA1(b6653facc7f5c624f5710a51f2b2abfe640177e2) )
	ROM_FILL       (          0x100000, 0x080000, nullptr )

	ROM_REGION16_BE(0x400000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 1 populated
	ROM_LOAD16_BYTE("e36-04.38", 0x000000, 0x200000, CRC(2250959b) SHA1(06943f1b72bdf325485356a01278d88aeae93d87) )    // C8 C9 CA CB

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",  0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",  0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8-d77-14.bin", 0x0800, 0x0117, CRC(7427e777) SHA1(e692cedb13e5bc02edc4b25e9dcea51e6715de85) )
ROM_END

ROM_START( arkretrnu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e36-11.20", 0x000000, 0x040000, CRC(b50cfb92) SHA1(dac69fc9ef03315b11bb94d19e3dfdc8867b08ed) )
	ROM_LOAD32_BYTE("e36-10.19", 0x000001, 0x040000, CRC(c940dba1) SHA1(ec87c9e4250f8b2f15094681a4783bca8c68f576) )
	ROM_LOAD32_BYTE("e36-09.18", 0x000002, 0x040000, CRC(f16985e0) SHA1(a74cfee8f958e7a32354d4353eeb199a7fb1ce64) )
	ROM_LOAD32_BYTE("e36-14.17", 0x000003, 0x040000, CRC(3360cfa1) SHA1(b06afc392b3864a895aed3a406d5d9886b1d0894) )

	ROM_REGION(0x180000, "audiocpu", 0) /* Sound CPU */
	ROM_LOAD16_BYTE("e36-12.32", 0x100000, 0x40000, CRC(3bae39be) SHA1(777142ecc24799b934ed51ac4cd8700bb6da7e3c) )
	ROM_LOAD16_BYTE("e36-13.33", 0x100001, 0x40000, CRC(94448e82) SHA1(d7766490318623be770545918391c5e6144dd619) )

	ROM_REGION(0x100000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e36-03.12", 0x000000, 0x040000, CRC(1ea8558b) SHA1(b8ea4d6e1fb551b3c47f336a5e60ec33f7be525f) )
	ROM_LOAD16_BYTE("e36-02.8",  0x000001, 0x040000, CRC(694eda31) SHA1(1a6f85057395052571491f85c633d5632ab64865) )
	ROM_LOAD       ("e36-01.4",  0x0c0000, 0x040000, CRC(54b9b2cd) SHA1(55ae964ea1d2cc40a6578c5339754a270096f01f) )
	ROM_FILL       (             0x080000, 0x040000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e36-07.47", 0x000000, 0x080000, CRC(266bf1c1) SHA1(489549478d7016400af2e643d4b98bf605237d34) )
	ROM_LOAD16_BYTE("e36-06.45", 0x000001, 0x080000, CRC(110ab729) SHA1(0ccc0a5abbcfd79a069daf5162cd344a5fb225d5) )
	ROM_LOAD       ("e36-05.43", 0x180000, 0x080000, CRC(db18bce2) SHA1(b6653facc7f5c624f5710a51f2b2abfe640177e2) )
	ROM_FILL       (          0x100000, 0x080000, nullptr )

	ROM_REGION16_BE(0x400000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 1 populated
	ROM_LOAD16_BYTE("e36-04.38", 0x000000, 0x200000, CRC(2250959b) SHA1(06943f1b72bdf325485356a01278d88aeae93d87) )    // C8 C9 CA CB

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",  0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",  0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8-d77-14.bin", 0x0800, 0x0117, CRC(7427e777) SHA1(e692cedb13e5bc02edc4b25e9dcea51e6715de85) )
ROM_END

ROM_START( arkretrnj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e36-11.20", 0x000000, 0x040000, CRC(b50cfb92) SHA1(dac69fc9ef03315b11bb94d19e3dfdc8867b08ed) )
	ROM_LOAD32_BYTE("e36-10.19", 0x000001, 0x040000, CRC(c940dba1) SHA1(ec87c9e4250f8b2f15094681a4783bca8c68f576) )
	ROM_LOAD32_BYTE("e36-09.18", 0x000002, 0x040000, CRC(f16985e0) SHA1(a74cfee8f958e7a32354d4353eeb199a7fb1ce64) )
	ROM_LOAD32_BYTE("e36-08.17", 0x000003, 0x040000, CRC(aa699e1b) SHA1(6bde0759940e0f238e4fa5bd228115574ff927d8) )

	ROM_REGION(0x180000, "audiocpu", 0) /* Sound CPU */
	ROM_LOAD16_BYTE("e36-12.32", 0x100000, 0x40000, CRC(3bae39be) SHA1(777142ecc24799b934ed51ac4cd8700bb6da7e3c) )
	ROM_LOAD16_BYTE("e36-13.33", 0x100001, 0x40000, CRC(94448e82) SHA1(d7766490318623be770545918391c5e6144dd619) )

	ROM_REGION(0x100000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e36-03.12", 0x000000, 0x040000, CRC(1ea8558b) SHA1(b8ea4d6e1fb551b3c47f336a5e60ec33f7be525f) )
	ROM_LOAD16_BYTE("e36-02.8",  0x000001, 0x040000, CRC(694eda31) SHA1(1a6f85057395052571491f85c633d5632ab64865) )
	ROM_LOAD       ("e36-01.4",  0x0c0000, 0x040000, CRC(54b9b2cd) SHA1(55ae964ea1d2cc40a6578c5339754a270096f01f) )
	ROM_FILL       (             0x080000, 0x040000, nullptr )

	ROM_REGION(0x200000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e36-07.47", 0x000000, 0x080000, CRC(266bf1c1) SHA1(489549478d7016400af2e643d4b98bf605237d34) )
	ROM_LOAD16_BYTE("e36-06.45", 0x000001, 0x080000, CRC(110ab729) SHA1(0ccc0a5abbcfd79a069daf5162cd344a5fb225d5) )
	ROM_LOAD       ("e36-05.43", 0x180000, 0x080000, CRC(db18bce2) SHA1(b6653facc7f5c624f5710a51f2b2abfe640177e2) )
	ROM_FILL       (          0x100000, 0x080000, nullptr )

	ROM_REGION16_BE(0x400000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 1 populated
	ROM_LOAD16_BYTE("e36-04.38", 0x000000, 0x200000, CRC(2250959b) SHA1(06943f1b72bdf325485356a01278d88aeae93d87) )    // C8 C9 CA CB

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",  0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",  0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8-d77-14.bin", 0x0800, 0x0117, CRC(7427e777) SHA1(e692cedb13e5bc02edc4b25e9dcea51e6715de85) )
ROM_END

ROM_START( kirameki )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e44-19.20", 0x000000, 0x80000, CRC(2f3b239a) SHA1(fb79955eba377d429ece04553251146b406143b1) )
	ROM_LOAD32_BYTE("e44-18.19", 0x000001, 0x80000, CRC(3137c8bc) SHA1(5f5cb47e214fe116cf985e847fa8340cf2ea5a64) )
	ROM_LOAD32_BYTE("e44-17.18", 0x000002, 0x80000, CRC(5905cd20) SHA1(52545622d3c7a31a9e95ab48e7251f1eae2c25b4) )
	ROM_LOAD32_BYTE("e44-16.17", 0x000003, 0x80000, CRC(5e9ac3fd) SHA1(3c45707d0d260961df99249978c7e8f51dd1720e) )

	ROM_REGION(0x1800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e44-06.12", 0x0000000, 0x400000, CRC(80526d58) SHA1(d32bf1e6494848b6e258b747245742310398be22) )
	ROM_LOAD16_BYTE("e44-04.8",  0x0000001, 0x400000, CRC(28c7c295) SHA1(90189ae26833499218b2236d48ce500a2eea3235) )
	ROM_LOAD16_BYTE("e44-05.11", 0x0800000, 0x200000, CRC(0fbc2b26) SHA1(0edd67213a9eba15fff0931a07608f9523ae1d95) )
	ROM_LOAD16_BYTE("e44-03.7",  0x0800001, 0x200000, CRC(d9e63fb0) SHA1(f2d8c30a4aaa2090673d5d2b1071e586a05c0236) )
	ROM_LOAD       ("e44-02.4",  0x1200000, 0x400000, CRC(5481efde) SHA1(560a1b8acf672781e912dca51599b5aa4d69520a) )
	ROM_LOAD       ("e44-01.3",  0x1600000, 0x200000, CRC(c4bdf727) SHA1(793a22a30ef44db818cfac96ff8e9d2f99cb672f) )
	ROM_FILL       (             0x0c00000, 0x600000, nullptr )

	ROM_REGION(0xc00000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e44-14.47", 0x000000, 0x200000, CRC(4597c034) SHA1(3c16c20969df9e439d42a89e649146319df1b996) )
	ROM_LOAD16_BYTE("e44-12.45", 0x000001, 0x200000, CRC(7160a181) SHA1(d4a351f38219694d6545b4c502c0ba0a7f0bdfd0) )
	ROM_LOAD16_BYTE("e44-13.46", 0x400000, 0x100000, CRC(0b016c4e) SHA1(670d1741376cf929adad3c5e45f921ed4b317d31) )
	ROM_LOAD16_BYTE("e44-11.44", 0x400001, 0x100000, CRC(34d84143) SHA1(d553ab2da9188b1881f70802637d46574a42787e) )
	ROM_LOAD       ("e44-10.43", 0x900000, 0x200000, CRC(326f738e) SHA1(29c0c870341345eba10993446fecee08b6f13027) )
	ROM_LOAD       ("e44-09.42", 0xb00000, 0x100000, CRC(a8e68eb7) SHA1(843bbb8a61bd4b9cbb14c7242281ce0c83c432ff) )
	ROM_FILL       (             0x600000, 0x300000, nullptr )

	ROM_REGION(0x400000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e44-20.51",      0x100000, 0x080000, CRC(4df7e051) SHA1(db0f5758458764a1c04116d5d5bbb20d4d36c875) )
	ROM_LOAD16_BYTE("e44-21.52",      0x100001, 0x080000, CRC(d31b94b8) SHA1(41ee381d10254dc6e7163c5f353568539a96fc20) )
	ROM_LOAD16_WORD_SWAP("e44-15.53", 0x200000, 0x200000, CRC(5043b608) SHA1(a328b8cc27ba1620a75a17cdf8571e217c42b9fd) ) /* Banked data */

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	ROM_LOAD16_BYTE("e44-07.38", 0x000000, 0x400000, CRC(a9e28544) SHA1(0dc3e1755a93fda310d26ed5f95dd211c05e579e) ) // D2 C8 C8 C9 CA CB CC CD
	ROM_LOAD16_BYTE("e44-08.39", 0x800000, 0x400000, CRC(33ba3037) SHA1(b4bbc4198929938607c444edf159ff40f53235d7) ) // CE CF D0 -- -- -- -std-
ROM_END

ROM_START( puchicar )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e46-16", 0x000000, 0x80000, CRC(cf2accdf) SHA1(b1e9808299a3c68c939009275108ee76cd7f5749) )
	ROM_LOAD32_BYTE("e46-15", 0x000001, 0x80000, CRC(c32c6ed8) SHA1(b0c4cca836e6957ecabdaddff23439f9d038a161) )
	ROM_LOAD32_BYTE("e46-14", 0x000002, 0x80000, CRC(a154c300) SHA1(177d9f3514f1e59a1036b979d2ab969249f519ca) )
	ROM_LOAD32_BYTE("e46-18", 0x000003, 0x80000, CRC(396e3122) SHA1(1000bfe22f783f7121a261d37551bde5687fff8b) )

	ROM_REGION(0x1000000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e46-06", 0x000000, 0x200000, CRC(b74336f2) SHA1(f5039a4d2117c78e905e2ef6dec43e143a91915e) )
	ROM_LOAD16_BYTE("e46-04", 0x000001, 0x200000, CRC(463ecc4c) SHA1(e1649e68acc1637a5dc596b1b29c735e286056af) )
	ROM_LOAD16_BYTE("e46-05", 0x400000, 0x200000, CRC(83a32eee) SHA1(1a1059b0a5ba1542c866072ffe1874daba982387) )
	ROM_LOAD16_BYTE("e46-03", 0x400001, 0x200000, CRC(eb768193) SHA1(1d48334c0dfb9f72484717c267ac9b9b8d887fc8) )
	ROM_LOAD       ("e46-02", 0xc00000, 0x200000, CRC(fb046018) SHA1(48d9c582ec9ef59dcc7538598fbd7ea2117896af) )
	ROM_LOAD       ("e46-01", 0xe00000, 0x200000, CRC(34fc2103) SHA1(dca199bbd0ad28a11960101cda8d110943b10822) )
	ROM_FILL       (          0x800000, 0x400000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e46-12", 0x000000, 0x100000, CRC(1f3a9851) SHA1(c8711e2ef0096b41cc9b4c41e521d44b824f7181) )
	ROM_LOAD16_BYTE("e46-11", 0x000001, 0x100000, CRC(e9f10bf1) SHA1(4ee9be3846b262dc0992c904c40580353b164d46) )
	ROM_LOAD       ("e46-10", 0x300000, 0x100000, CRC(1999b76a) SHA1(83d6d2efe250bf3b119982bbf701f9b9d856af2d) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e46-21", 0x100000, 0x40000, CRC(b466cff6) SHA1(0757984f15a6ac9939c1e8625d5b9bfcbc788acc) )
	ROM_LOAD16_BYTE("e46-22", 0x100001, 0x40000, CRC(c67b767e) SHA1(19f3db6615d7a6ed4d2636b6beabe2f3ed6d0c38) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e46-07", 0x400000, 0x200000, CRC(f20af91e) SHA1(86040ff7ce591418b32c06c3a02fabcbe76281f5) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e46-08", 0x800000, 0x200000, CRC(f7f96e1d) SHA1(8a83ea9036e8647b8dec6b5e144288ed9c025779) )    // CC CD CE CF
	ROM_LOAD16_BYTE("e46-09", 0xc00000, 0x200000, CRC(824135f8) SHA1(13e9edeac38e63fa27d9fd7892d51c216f36ec30) )    // D0 D1 D2 D3

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09a.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10a.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11a.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12a.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15a.bin", 0x0800, 0x0117, CRC(0edf820a) SHA1(6bfcc711a15a31dd57e863c46537df9de1a66c2f) )
ROM_END

ROM_START( puchicarj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e46-16", 0x000000, 0x80000, CRC(cf2accdf) SHA1(b1e9808299a3c68c939009275108ee76cd7f5749) )
	ROM_LOAD32_BYTE("e46-15", 0x000001, 0x80000, CRC(c32c6ed8) SHA1(b0c4cca836e6957ecabdaddff23439f9d038a161) )
	ROM_LOAD32_BYTE("e46-14", 0x000002, 0x80000, CRC(a154c300) SHA1(177d9f3514f1e59a1036b979d2ab969249f519ca) )
	ROM_LOAD32_BYTE("e46.13", 0x000003, 0x80000, CRC(59fbdf3a) SHA1(4499a7579907e8e1d8ca2c29e8e8d12185e8fe4d) )

	ROM_REGION(0x1000000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e46-06", 0x000000, 0x200000, CRC(b74336f2) SHA1(f5039a4d2117c78e905e2ef6dec43e143a91915e) )
	ROM_LOAD16_BYTE("e46-04", 0x000001, 0x200000, CRC(463ecc4c) SHA1(e1649e68acc1637a5dc596b1b29c735e286056af) )
	ROM_LOAD16_BYTE("e46-05", 0x400000, 0x200000, CRC(83a32eee) SHA1(1a1059b0a5ba1542c866072ffe1874daba982387) )
	ROM_LOAD16_BYTE("e46-03", 0x400001, 0x200000, CRC(eb768193) SHA1(1d48334c0dfb9f72484717c267ac9b9b8d887fc8) )
	ROM_LOAD       ("e46-02", 0xc00000, 0x200000, CRC(fb046018) SHA1(48d9c582ec9ef59dcc7538598fbd7ea2117896af) )
	ROM_LOAD       ("e46-01", 0xe00000, 0x200000, CRC(34fc2103) SHA1(dca199bbd0ad28a11960101cda8d110943b10822) )
	ROM_FILL       (          0x800000, 0x400000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e46-12", 0x000000, 0x100000, CRC(1f3a9851) SHA1(c8711e2ef0096b41cc9b4c41e521d44b824f7181) )
	ROM_LOAD16_BYTE("e46-11", 0x000001, 0x100000, CRC(e9f10bf1) SHA1(4ee9be3846b262dc0992c904c40580353b164d46) )
	ROM_LOAD       ("e46-10", 0x300000, 0x100000, CRC(1999b76a) SHA1(83d6d2efe250bf3b119982bbf701f9b9d856af2d) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e46-19", 0x100000, 0x40000, CRC(2624eba0) SHA1(ba0b13bda241c648c7f8520106acd8c0c888fe29) )
	ROM_LOAD16_BYTE("e46-20", 0x100001, 0x40000, CRC(065e934f) SHA1(0ec1b5ae33b1c43776b9327c9d380787d64ed5f9) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e46-07", 0x400000, 0x200000, CRC(f20af91e) SHA1(86040ff7ce591418b32c06c3a02fabcbe76281f5) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e46-08", 0x800000, 0x200000, CRC(f7f96e1d) SHA1(8a83ea9036e8647b8dec6b5e144288ed9c025779) )    // CC CD CE CF
	ROM_LOAD16_BYTE("e46-09", 0xc00000, 0x200000, CRC(824135f8) SHA1(13e9edeac38e63fa27d9fd7892d51c216f36ec30) )    // D0 D1 D2 D3

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09a.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10a.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11a.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12a.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15a.bin", 0x0800, 0x0117, CRC(0edf820a) SHA1(6bfcc711a15a31dd57e863c46537df9de1a66c2f) )
ROM_END

ROM_START( pbobble4 )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e49-12.20", 0x000000, 0x80000, CRC(fffea203) SHA1(6351209c78099f270c8d8c8aa4a2dd9ce126c4ed) )
	ROM_LOAD32_BYTE("e49-11.19", 0x000001, 0x80000, CRC(bf69a087) SHA1(2acbdb7faf5bdb1d9b5b9506c0b6f02fedcbd6a5) )
	ROM_LOAD32_BYTE("e49-10.18", 0x000002, 0x80000, CRC(0307460b) SHA1(7ad9c6e5d319d6727444ffd14a87c6885445cee0) )
	ROM_LOAD32_BYTE("e49-16.17", 0x000003, 0x80000, CRC(0a021624) SHA1(21a948f9f4adce0aaf76292f419a7c289f265d30) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e49-02", 0x000000, 0x100000, CRC(c7d2f40b) SHA1(cc6813189d6b31d7db099e49443af395f137462c) )
	ROM_LOAD16_BYTE("e49-01", 0x000001, 0x100000, CRC(a3dd5f85) SHA1(2b305fdc18806bb5d7c3de0ac6a6eb07f98b4d3d) )
	ROM_FILL       (          0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e49-08", 0x000000, 0x100000, CRC(87408106) SHA1(6577568ab4b92d6a81f43cf9ea2f0e30e17e2742) )
	ROM_LOAD16_BYTE("e49-07", 0x000001, 0x100000, CRC(1e1e8e1c) SHA1(9c3b994064c6af62b6a24cab85089a74fd92cf7f) )
	ROM_LOAD       ("e49-06", 0x300000, 0x100000, CRC(ec85f7ce) SHA1(9fead68c38fc9ca84d34d70343b13665978c362b) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("e49-13.32", 0x100000, 0x40000, CRC(83536f7f) SHA1(2252cee00ae260954cc76d92af8eb2a87d23cbfb) )
	ROM_LOAD16_BYTE("e49-14.33", 0x100001, 0x40000, CRC(19815bdb) SHA1(38ad682236c7df0710055dd8dbdec30d5da0839d) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e49-03", 0x400000, 0x200000, CRC(f64303e0) SHA1(4d5df77047522419d21ff36402076e9b7c5acff8) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e49-04", 0x800000, 0x200000, CRC(09be229c) SHA1(a3a88969b34628d2bf3163bdf85d520feac9a7ac) )    // E7 CD E8 E6
	ROM_LOAD16_BYTE("e49-05", 0xc00000, 0x200000, CRC(5ce90ee2) SHA1(afafc1f64ecf2dbd94a9f7871a26150ac2d22be5) )    // D3 D4 D5 DB

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15.bin", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( pbobble4j )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e49-12.20", 0x000000, 0x80000, CRC(fffea203) SHA1(6351209c78099f270c8d8c8aa4a2dd9ce126c4ed) )
	ROM_LOAD32_BYTE("e49-11.19", 0x000001, 0x80000, CRC(bf69a087) SHA1(2acbdb7faf5bdb1d9b5b9506c0b6f02fedcbd6a5) )
	ROM_LOAD32_BYTE("e49-10.18", 0x000002, 0x80000, CRC(0307460b) SHA1(7ad9c6e5d319d6727444ffd14a87c6885445cee0) )
	ROM_LOAD32_BYTE("e49-09.17", 0x000003, 0x80000, CRC(e40c7708) SHA1(0a8e973bb1d8c6dea9124d2742d354c6f20c08ee) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e49-02", 0x000000, 0x100000, CRC(c7d2f40b) SHA1(cc6813189d6b31d7db099e49443af395f137462c) )
	ROM_LOAD16_BYTE("e49-01", 0x000001, 0x100000, CRC(a3dd5f85) SHA1(2b305fdc18806bb5d7c3de0ac6a6eb07f98b4d3d) )
	ROM_FILL       (          0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e49-08", 0x000000, 0x100000, CRC(87408106) SHA1(6577568ab4b92d6a81f43cf9ea2f0e30e17e2742) )
	ROM_LOAD16_BYTE("e49-07", 0x000001, 0x100000, CRC(1e1e8e1c) SHA1(9c3b994064c6af62b6a24cab85089a74fd92cf7f) )
	ROM_LOAD       ("e49-06", 0x300000, 0x100000, CRC(ec85f7ce) SHA1(9fead68c38fc9ca84d34d70343b13665978c362b) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("e49-13.32", 0x100000, 0x40000, CRC(83536f7f) SHA1(2252cee00ae260954cc76d92af8eb2a87d23cbfb) )
	ROM_LOAD16_BYTE("e49-14.33", 0x100001, 0x40000, CRC(19815bdb) SHA1(38ad682236c7df0710055dd8dbdec30d5da0839d) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e49-03", 0x400000, 0x200000, CRC(f64303e0) SHA1(4d5df77047522419d21ff36402076e9b7c5acff8) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e49-04", 0x800000, 0x200000, CRC(09be229c) SHA1(a3a88969b34628d2bf3163bdf85d520feac9a7ac) )    // E7 CD E8 E6
	ROM_LOAD16_BYTE("e49-05", 0xc00000, 0x200000, CRC(5ce90ee2) SHA1(afafc1f64ecf2dbd94a9f7871a26150ac2d22be5) )    // D3 D4 D5 DB

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15.bin", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( pbobble4u )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e49-12.20", 0x000000, 0x80000, CRC(fffea203) SHA1(6351209c78099f270c8d8c8aa4a2dd9ce126c4ed) )
	ROM_LOAD32_BYTE("e49-11.19", 0x000001, 0x80000, CRC(bf69a087) SHA1(2acbdb7faf5bdb1d9b5b9506c0b6f02fedcbd6a5) )
	ROM_LOAD32_BYTE("e49-10.18", 0x000002, 0x80000, CRC(0307460b) SHA1(7ad9c6e5d319d6727444ffd14a87c6885445cee0) )
	ROM_LOAD32_BYTE("e49-15.17", 0x000003, 0x80000, CRC(7d0526b2) SHA1(1b769f735735e9135418ff26c020d8ac7f62d857) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e49-02", 0x000000, 0x100000, CRC(c7d2f40b) SHA1(cc6813189d6b31d7db099e49443af395f137462c) )
	ROM_LOAD16_BYTE("e49-01", 0x000001, 0x100000, CRC(a3dd5f85) SHA1(2b305fdc18806bb5d7c3de0ac6a6eb07f98b4d3d) )
	ROM_FILL       (          0x200000, 0x200000, nullptr )

	ROM_REGION(0x400000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e49-08", 0x000000, 0x100000, CRC(87408106) SHA1(6577568ab4b92d6a81f43cf9ea2f0e30e17e2742) )
	ROM_LOAD16_BYTE("e49-07", 0x000001, 0x100000, CRC(1e1e8e1c) SHA1(9c3b994064c6af62b6a24cab85089a74fd92cf7f) )
	ROM_LOAD       ("e49-06", 0x300000, 0x100000, CRC(ec85f7ce) SHA1(9fead68c38fc9ca84d34d70343b13665978c362b) )
	ROM_FILL       (          0x200000, 0x100000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 code */
	ROM_LOAD16_BYTE("e49-13.32", 0x100000, 0x40000, CRC(83536f7f) SHA1(2252cee00ae260954cc76d92af8eb2a87d23cbfb) )
	ROM_LOAD16_BYTE("e49-14.33", 0x100001, 0x40000, CRC(19815bdb) SHA1(38ad682236c7df0710055dd8dbdec30d5da0839d) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e49-03", 0x400000, 0x200000, CRC(f64303e0) SHA1(4d5df77047522419d21ff36402076e9b7c5acff8) )    // C8 C9 CA CB
	ROM_LOAD16_BYTE("e49-04", 0x800000, 0x200000, CRC(09be229c) SHA1(a3a88969b34628d2bf3163bdf85d520feac9a7ac) )    // E7 CD E8 E6
	ROM_LOAD16_BYTE("e49-05", 0xc00000, 0x200000, CRC(5ce90ee2) SHA1(afafc1f64ecf2dbd94a9f7871a26150ac2d22be5) )    // D3 D4 D5 DB

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.bin",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.bin",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.bin", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.bin", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15.bin", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( popnpopj )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e51-12.20", 0x000000, 0x80000, CRC(86a237d5) SHA1(4b87a51705a4d831b21ee770af17310c6c091c2e) )
	ROM_LOAD32_BYTE("e51-11.19", 0x000001, 0x80000, CRC(8a49f34f) SHA1(8691fbc1e96f0c9eb550dbb8ae8d7ef371397166) )
	ROM_LOAD32_BYTE("e51-10.18", 0x000002, 0x80000, CRC(4bce68f8) SHA1(1a9220926f4d8db509f4ccbf318d123f34c42153) )
	ROM_LOAD32_BYTE("e51-09.17", 0x000003, 0x80000, CRC(4a086017) SHA1(edec4120b3d96a179f12949bd261b97d41351cab) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e51-03.12",0x000000, 0x100000, CRC(a24c4607) SHA1(2396fa927446568ad8d98ad6756813e2f30523dd) )
	ROM_LOAD16_BYTE("e51-02.8", 0x000001, 0x100000, CRC(6aa8b96c) SHA1(aaf7917dce5fed43c68cd3065538b58666ef3dbc) )
	ROM_LOAD       ("e51-01.4", 0x300000, 0x100000, CRC(70347e24) SHA1(6b4ab90f0209e50eac7bee3abcf40afb71ab950a) )
	ROM_FILL       (            0x200000, 0x100000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e51-08.47", 0x000000, 0x200000, CRC(3ad41f02) SHA1(a4959113c01062003df41cdf6146e8a034917ee2) )
	ROM_LOAD16_BYTE("e51-07.45", 0x000001, 0x200000, CRC(95873e46) SHA1(02504cbd920c8dbcb5abec6388305eff38f7efe0) )
	ROM_LOAD       ("e51-06.43", 0x600000, 0x200000, CRC(c240d6c8) SHA1(6f3b5224b7eb8783893375d432bbbfc37f81c230) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e51-13.32", 0x100000, 0x40000, CRC(3b9e3986) SHA1(26340bda0ad2ea580e2395135617966676a71ad5) )
	ROM_LOAD16_BYTE("e51-14.33", 0x100001, 0x40000, CRC(1f9a5015) SHA1(5da38c5fe2a50bcde6bd46ab1cb9a56dbab1a882) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e51-04.38", 0x000000, 0x200000, CRC(66790f55) SHA1(ac539b2655dbcda1bdffb9f3cf4c96fb05721e9d) ) // C9 CA CB CC
	ROM_LOAD16_BYTE("e51-05.41", 0x400000, 0x200000, CRC(4d08b26d) SHA1(071a11a1b1ee8b8129d02b15ec0e533912c91b04) ) // CD CE -std-

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.ic14",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.ic28",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.ic37", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.ic48", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15.ic21", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( popnpop )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e51-12.20", 0x000000, 0x80000, CRC(86a237d5) SHA1(4b87a51705a4d831b21ee770af17310c6c091c2e) )
	ROM_LOAD32_BYTE("e51-11.19", 0x000001, 0x80000, CRC(8a49f34f) SHA1(8691fbc1e96f0c9eb550dbb8ae8d7ef371397166) )
	ROM_LOAD32_BYTE("e51-10.18", 0x000002, 0x80000, CRC(4bce68f8) SHA1(1a9220926f4d8db509f4ccbf318d123f34c42153) )
	ROM_LOAD32_BYTE("e51-16.17", 0x000003, 0x80000, CRC(2a9d8e0f) SHA1(a5363a98f03cbc7b5f7c393b21062730bd6ee2a8) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e51-03.12",0x000000, 0x100000, CRC(a24c4607) SHA1(2396fa927446568ad8d98ad6756813e2f30523dd) )
	ROM_LOAD16_BYTE("e51-02.8", 0x000001, 0x100000, CRC(6aa8b96c) SHA1(aaf7917dce5fed43c68cd3065538b58666ef3dbc) )
	ROM_LOAD       ("e51-01.4", 0x300000, 0x100000, CRC(70347e24) SHA1(6b4ab90f0209e50eac7bee3abcf40afb71ab950a) )
	ROM_FILL       (            0x200000, 0x100000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e51-08.47", 0x000000, 0x200000, CRC(3ad41f02) SHA1(a4959113c01062003df41cdf6146e8a034917ee2) )
	ROM_LOAD16_BYTE("e51-07.45", 0x000001, 0x200000, CRC(95873e46) SHA1(02504cbd920c8dbcb5abec6388305eff38f7efe0) )
	ROM_LOAD       ("e51-06.43", 0x600000, 0x200000, CRC(c240d6c8) SHA1(6f3b5224b7eb8783893375d432bbbfc37f81c230) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e51-13.32", 0x100000, 0x40000, CRC(3b9e3986) SHA1(26340bda0ad2ea580e2395135617966676a71ad5) )
	ROM_LOAD16_BYTE("e51-14.33", 0x100001, 0x40000, CRC(1f9a5015) SHA1(5da38c5fe2a50bcde6bd46ab1cb9a56dbab1a882) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e51-04.38", 0x000000, 0x200000, CRC(66790f55) SHA1(ac539b2655dbcda1bdffb9f3cf4c96fb05721e9d) ) // C9 CA CB CC
	ROM_LOAD16_BYTE("e51-05.41", 0x400000, 0x200000, CRC(4d08b26d) SHA1(071a11a1b1ee8b8129d02b15ec0e533912c91b04) ) // CD CE -std-

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.ic14",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.ic28",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.ic37", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.ic48", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15.ic21", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( popnpopu )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e51-12.20", 0x000000, 0x80000, CRC(86a237d5) SHA1(4b87a51705a4d831b21ee770af17310c6c091c2e) )
	ROM_LOAD32_BYTE("e51-11.19", 0x000001, 0x80000, CRC(8a49f34f) SHA1(8691fbc1e96f0c9eb550dbb8ae8d7ef371397166) )
	ROM_LOAD32_BYTE("e51-10.18", 0x000002, 0x80000, CRC(4bce68f8) SHA1(1a9220926f4d8db509f4ccbf318d123f34c42153) )
	ROM_LOAD32_BYTE("e51-15.17", 0x000003, 0x80000, CRC(1ad77903) SHA1(d5e631d70108d1f15bcfcacde914ac2fb95cb102) )

	ROM_REGION(0x400000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e51-03.12",0x000000, 0x100000, CRC(a24c4607) SHA1(2396fa927446568ad8d98ad6756813e2f30523dd) )
	ROM_LOAD16_BYTE("e51-02.8", 0x000001, 0x100000, CRC(6aa8b96c) SHA1(aaf7917dce5fed43c68cd3065538b58666ef3dbc) )
	ROM_LOAD       ("e51-01.4", 0x300000, 0x100000, CRC(70347e24) SHA1(6b4ab90f0209e50eac7bee3abcf40afb71ab950a) )
	ROM_FILL       (            0x200000, 0x100000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e51-08.47", 0x000000, 0x200000, CRC(3ad41f02) SHA1(a4959113c01062003df41cdf6146e8a034917ee2) )
	ROM_LOAD16_BYTE("e51-07.45", 0x000001, 0x200000, CRC(95873e46) SHA1(02504cbd920c8dbcb5abec6388305eff38f7efe0) )
	ROM_LOAD       ("e51-06.43", 0x600000, 0x200000, CRC(c240d6c8) SHA1(6f3b5224b7eb8783893375d432bbbfc37f81c230) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )

	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e51-13.32", 0x100000, 0x40000, CRC(3b9e3986) SHA1(26340bda0ad2ea580e2395135617966676a71ad5) )
	ROM_LOAD16_BYTE("e51-14.33", 0x100001, 0x40000, CRC(1f9a5015) SHA1(5da38c5fe2a50bcde6bd46ab1cb9a56dbab1a882) )

	ROM_REGION16_BE(0x800000, "ensoniq.0" , ROMREGION_ERASE00 ) // V2: 4 banks, only 2 populated
	ROM_LOAD16_BYTE("e51-04.38", 0x000000, 0x200000, CRC(66790f55) SHA1(ac539b2655dbcda1bdffb9f3cf4c96fb05721e9d) ) // C9 CA CB CC
	ROM_LOAD16_BYTE("e51-05.41", 0x400000, 0x200000, CRC(4d08b26d) SHA1(071a11a1b1ee8b8129d02b15ec0e533912c91b04) ) // CD CE -std-

	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "pal16l8a-d77-09.ic14",   0x0000, 0x0104, CRC(b371532b) SHA1(7f875f380e69d14326a036a09d2fda7554b73664) )
	ROM_LOAD( "pal16l8a-d77-10.ic28",   0x0200, 0x0104, CRC(42f59227) SHA1(1f0db7489b49c00603ea51d130c9dfc237545390) )
	ROM_LOAD( "palce16v8q-d77-11.ic37", 0x0400, 0x0117, CRC(eacc294e) SHA1(90679d523d90c1f8d2ecbd7b6fac2861f94cf107) )
	ROM_LOAD( "palce16v8q-d77-12.ic48", 0x0600, 0x0117, CRC(e9920cfe) SHA1(58b73fe65f118d57fdce56d781593fc70c797f1b) )
	ROM_LOAD( "palce16v8q-d77-15.ic21", 0x0800, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END


/*
Landmaker
Taito, 1998

PCB Layouts
-----------

Top board

TK8X3004A
NICEB PCB
|--------------------------------------------------------------|
| |-----------------|   |-----------------| PAL16L8.24         |
| |-----------------|   |-----------------|                    |
|                                                              |
| |------------------|                                         |
| |     68000        |   SND-3.46  SND-7.35 SND-11.23 SND-15.11|
| |                  |   SND-2.45  SND-6.34 SND-10.22 SND-14.10|
| |------------------|   SND-1.44  SND-5.33 SND-9.21  SND-13.9 |
|                        SND-0.43  SND-4.32 SND-8.20  SND-12.8 |
| SPRO-1.66  PAL16L8.58  SCR3-0.42                             |
| SPRO-0.65  PAL16L8.57  SCR3-1.41                             |
| PAL16L8.64             SCR3-2.40                             |
|                        SCR3-3.39                             |
|                        SCR3-4.38                             |
| MPRO-0.63              SCR3-5.37                             |
| MPRO-1.62                                                    |
| MPRO-2.61                                                    |
| MPRO-3.60     |---------|    |---------|                     |
|               |MC68EC020|    |         |                     |
| PAL16L8.59    |RP25     |    |    *    |                     |
|               |         |    |         |                     |
|               |         |    |         |                     |
|               |---------|    |---------|                     |
| |-----------------|   |-----------------|                    |
| |-----------------|   |-----------------|                    |
|--------------------------------------------------------------|
Notes:
      * - Empty PGA socket
*/

ROM_START( landmakr )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("e61-13.20", 0x000000, 0x80000, CRC(0af756a2) SHA1(2dadac6873f2491ee77703f07f00dde2aa909355) )
	ROM_LOAD32_BYTE("e61-12.19", 0x000001, 0x80000, CRC(636b3df9) SHA1(78a5bf4977bb90d710942188ce5016f3df499feb) )
	ROM_LOAD32_BYTE("e61-11.18", 0x000002, 0x80000, CRC(279a0ee4) SHA1(08380286737b33db76a79b27d0df5faba17dfb96) )
	ROM_LOAD32_BYTE("e61-10.17", 0x000003, 0x80000, CRC(daabf2b2) SHA1(dbfbe38841fc2f937052353eff1202790d364b9f) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("e61-03.12",0x000000, 0x200000, CRC(e8abfc46) SHA1(fbde006f9822af3ed8debec525270d329981ea21) )
	ROM_LOAD16_BYTE("e61-02.08",0x000001, 0x200000, CRC(1dc4a164) SHA1(33b412d9653099aaff8ed5e62d1ba4fc30aa9058) )
	ROM_LOAD       ("e61-01.04",0x600000, 0x200000, CRC(6cdd8311) SHA1(7810a5a81f3b5a730d2088c79b12fffd77659b5b) )
	ROM_FILL       (            0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD16_BYTE("e61-09.47", 0x000000, 0x200000, CRC(6ba29987) SHA1(b63c12523e19da66b3ca07c3548ac81bf57b59a1) )
	ROM_LOAD16_BYTE("e61-08.45", 0x000001, 0x200000, CRC(76c98e14) SHA1(c021c325ab4ae410fa54e2eab61d34318867432b) )
	ROM_LOAD       ("e61-07.43", 0x600000, 0x200000, CRC(4a57965d) SHA1(8e80788e0f47fb242da9af3aa19077dc0ec829b8) )
	ROM_FILL       (             0x400000, 0x200000, nullptr )



	ROM_REGION(0x140000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("e61-14.32", 0x100000, 0x20000, CRC(b905f4a7) SHA1(613b954e3e129fd44b4ce64958f16e5636012d6e) )
	ROM_LOAD16_BYTE("e61-15.33", 0x100001, 0x20000, CRC(87909869) SHA1(7b90c23899a673966cac3352d375d17b83e66596) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("e61-04.38", 0x400000, 0x200000, CRC(c27aec0c) SHA1(e95da2db07a20a53662ebd45c033966e8a22a15a) ) // C8 C9 CA CB
	ROM_LOAD16_BYTE("e61-05.39", 0x800000, 0x200000, CRC(83920d9d) SHA1(019e39ae85d1129f6d3b8460c4b1bd925f868ee2) ) // CC CD CE CF
	ROM_LOAD16_BYTE("e61-06.40", 0xc00000, 0x200000, CRC(2e717bfe) SHA1(1be54cd2ec65d8fd49a5c09b5d27791fd7a320d4) ) // D0 D1 D2 D3
ROM_END

ROM_START( landmakrp )
	ROM_REGION(0x200000, "maincpu", 0) /* 68020 code */
	ROM_LOAD32_BYTE("mpro-3.60", 0x000000, 0x80000, CRC(f92eccd0) SHA1(88e836390be1ca08c578080662d17007a9e0bcc3) )
	ROM_LOAD32_BYTE("mpro-2.61", 0x000001, 0x80000, CRC(5a26c9e0) SHA1(e7f09722f6b7a459248c2c8ad0a2695365cc78dc) )
	ROM_LOAD32_BYTE("mpro-1.62", 0x000002, 0x80000, CRC(710776a8) SHA1(669aa086e7a5faedd90407e558c01bf5f0869790) )
	ROM_LOAD32_BYTE("mpro-0.63", 0x000003, 0x80000, CRC(bc71dd2f) SHA1(ec0d07f9729a53737975547066bd1221f78563c5) )

	ROM_REGION(0x800000, "gfx1" , 0) /* Sprites */
	ROM_LOAD16_BYTE("obj0-0.8", 0x000000, 0x080000, CRC(4b862c1b) SHA1(ef46af27d0657b95f5f3bad13629f9119958fe78) )
	ROM_LOAD16_BYTE("obj1-0.7", 0x100000, 0x080000, CRC(90502355) SHA1(e1edc0cec8ca53fda4d42f9b9fdd385379d7a958) )
	ROM_LOAD16_BYTE("obj2-0.6", 0x200000, 0x080000, CRC(3bffe4b2) SHA1(6e9bb8716f312cb8c81ecebfc61f9dfc8c9013dc) )
	ROM_LOAD16_BYTE("obj3-0.5", 0x300000, 0x080000, CRC(3a0e1479) SHA1(50430c304c437caadebf04499f49ca6323ebdaba) )
	ROM_LOAD16_BYTE("obj0-1.20",0x000001, 0x080000, CRC(1dc6e1ae) SHA1(1e8fa89b1a8846de1516ca9d2ef9227b4af07e38) )
	ROM_LOAD16_BYTE("obj1-1.19",0x100001, 0x080000, CRC(a24edb24) SHA1(81fe77eccdd2a7ea02454e57e52b21ad57eb817e) )
	ROM_LOAD16_BYTE("obj2-1.18",0x200001, 0x080000, CRC(1b2a87f3) SHA1(b7dc871196b92bb4f6ea31bff0717cb3a508bc05) )
	ROM_LOAD16_BYTE("obj3-1.17",0x300001, 0x080000, CRC(c7e91180) SHA1(c8bfa43ab3b9a6c4ba08e1a7389880e964bb1d80) )
	ROM_LOAD       ("obj0-2.32",0x600000, 0x080000, CRC(94cc01d0) SHA1(f4cf4cb237a3f2bd9df35424f85a84b70b47d402) )
	ROM_LOAD       ("obj1-2.31",0x680000, 0x080000, CRC(c2757722) SHA1(83a921647eb0375e10c7f76c08ebe66f2a6fdcd9) )
	ROM_LOAD       ("obj2-2.30",0x700000, 0x080000, CRC(934556ff) SHA1(aca8585680e66635b8872259cfd38edc96e92066) )
	ROM_LOAD       ("obj3-2.29",0x780000, 0x080000, CRC(97f0f777) SHA1(787a33b91cb262cc3983a46ba259dd9b153d532a) )
	ROM_FILL       (            0x400000, 0x200000, nullptr )

	ROM_REGION(0x800000, "gfx2" , 0) /* Tiles */
	ROM_LOAD32_BYTE("scr0-0.7", 0x000000, 0x080000, CRC(da6ba562) SHA1(6aefd249d3491380837e04583a0069ed9c495d05) )
	ROM_LOAD32_BYTE("scr0-1.6", 0x000002, 0x080000, CRC(8c201d27) SHA1(83147c35d03c7b5c84220a6442e99b87ba99cfbc) )
	ROM_LOAD32_BYTE("scr0-2.5", 0x000001, 0x080000, CRC(36756b9c) SHA1(3d293b11d03fb4cdc5c041fcdade9941bf6a72d0) )
	ROM_LOAD32_BYTE("scr0-3.4", 0x000003, 0x080000, CRC(4e0274f3) SHA1(d65378db78a310c664ef49a216f18e16c932f58d) )
	ROM_LOAD32_BYTE("scr1-0.19",0x200000, 0x080000, CRC(2689f716) SHA1(6849e7d36aca5a678b74e1cce9e6a2381928c127) )
	ROM_LOAD32_BYTE("scr1-1.18",0x200002, 0x080000, CRC(f3086949) SHA1(c21f5384294a9fcfb422dbb85565305520a334b5) )
	ROM_LOAD32_BYTE("scr1-2.17",0x200001, 0x080000, CRC(7841468a) SHA1(58b60cbb4ec7e2d0d64fc42161b53b9ff5e2ca8c) )
	ROM_LOAD32_BYTE("scr1-3.16",0x200003, 0x080000, CRC(926ad229) SHA1(4840227c184bde8d125122a90a70102bf2757ccc) )
	ROM_LOAD16_BYTE("scr0-4.3", 0x600000, 0x080000, CRC(5b3cf564) SHA1(003f1e4c653897016c95dee67161fa3964d4f5a8) )
	ROM_LOAD16_BYTE("scr0-5.2", 0x600001, 0x080000, CRC(8e1ea0fe) SHA1(aa815d1d67bf72be6a0c4076490dfd36f28a82ab) )
	ROM_LOAD16_BYTE("scr1-4.15",0x700000, 0x080000, CRC(783b6d10) SHA1(eab2c7b19890c1f6c13f0062978db5b81988499b) )
	ROM_LOAD16_BYTE("scr1-5.14",0x700001, 0x080000, CRC(24aba128) SHA1(b03804c738d86bfafc1f8fb91f8e77e878d2dc83) )
	ROM_FILL       (            0x400000, 0x200000, nullptr )



	ROM_REGION(0x180000, "audiocpu", 0) /* 68000 sound CPU */
	ROM_LOAD16_BYTE("spro-1.66", 0x100000, 0x40000, CRC(18961bbb) SHA1(df054def35a49c0754356c15ec15336cbf28b063) )
	ROM_LOAD16_BYTE("spro-0.65", 0x100001, 0x40000, CRC(2c64557a) SHA1(768007162d5d2cbe650c735bc1af2c10ed13b046) )

	ROM_REGION16_BE(0x1000000, "ensoniq.0" , ROMREGION_ERASE00 )    // V2: 4 banks
	// empty
	ROM_LOAD16_BYTE("snd-0.43", 0x400000, 0x80000, CRC(0e5ef5c8) SHA1(e2840c9cedb9361b7eb307e87ea96f3bb6225487) )   // C8
	ROM_LOAD16_BYTE("snd-1.44", 0x500000, 0x80000, CRC(2998fd65) SHA1(192e32f9934465bb0da5c1ad116c5ea9b286f36a) )   // C9
	ROM_LOAD16_BYTE("snd-2.45", 0x600000, 0x80000, CRC(da7477ad) SHA1(52c69e86e8f8004d616265f3c1f508e7fac19fdc) )   // CA
	ROM_LOAD16_BYTE("snd-3.46", 0x700000, 0x80000, CRC(141670b9) SHA1(d1d75bc9c27481de68b446e397c448b0163a7916) )   // CB
	ROM_LOAD16_BYTE("snd-4.32", 0x800000, 0x80000, CRC(e9dc18f6) SHA1(c84920246a9967b155e137893c080bce6850db85) )   // CC
	ROM_LOAD16_BYTE("snd-5.33", 0x900000, 0x80000, CRC(8af91ca8) SHA1(853d2a036602338539cf25e68eac1e686c0861d5) )   // CD
	ROM_LOAD16_BYTE("snd-6.34", 0xa00000, 0x80000, CRC(6f520b82) SHA1(c559c80386de08256b2f8cbf198271223a83fdb9) )   // CE
	ROM_LOAD16_BYTE("snd-7.35", 0xb00000, 0x80000, CRC(69410f0f) SHA1(ff023842383ce26818ec7361831e122737a9e94b) )   // CF
	ROM_LOAD16_BYTE("snd-8.20", 0xc00000, 0x80000, CRC(d98c275e) SHA1(862f5759d2e41243b8a6a3f27ab2da2a1456d92c) )   // D0
	ROM_LOAD16_BYTE("snd-9.21", 0xd00000, 0x80000, CRC(82a76cfc) SHA1(a9bdc9b05cfb658165165c3292a698ed0e977ede) )   // D1
	ROM_LOAD16_BYTE("snd-10.22",0xe00000, 0x80000, CRC(0345f585) SHA1(de8a9816eba7d4db73a53103479ee9d56889e127) )   // D2
	ROM_LOAD16_BYTE("snd-11.23",0xf00000, 0x80000, CRC(4caf571a) SHA1(c209f78362442f8a952c180e3d01a5e8e9d5c71c) )   // D3
ROM_END

/******************************************************************************/

static void tile_decode(running_machine &machine)
{
	UINT8 lsb,msb;
	UINT32 offset,i;
	UINT8 *gfx = machine.root_device().memregion("gfx2")->base();
	int size=machine.root_device().memregion("gfx2")->bytes();
	int data;

	/* Setup ROM formats:

	    Some games will only use 4 or 5 bpp sprites, and some only use 4 bpp tiles,
	    I don't believe this is software or prom controlled but simply the unused data lines
	    are tied low on the game board if unused.  This is backed up by the fact the palette
	    indices are always related to 4 bpp data, even in 6 bpp games.

	    Most (all?) games with 5bpp tiles have the sixth bit set. Also, in Arabian Magic
	    sprites 1200-120f contain 6bpp data which is probably bogus.
	    video_start clears the fifth and sixth bit of the decoded graphics according
	    to the bit depth specified in f3_config_table.

	*/

	offset = size/2;
	for (i = size/2+size/4; i<size; i+=2)
	{
		/* Expand 2bits into 4bits format */
		lsb = gfx[i+1];
		msb = gfx[i];

		gfx[offset+0]=((msb&0x02)<<3) | ((msb&0x01)>>0) | ((lsb&0x02)<<4) | ((lsb&0x01)<<1);
		gfx[offset+2]=((msb&0x08)<<1) | ((msb&0x04)>>2) | ((lsb&0x08)<<2) | ((lsb&0x04)>>1);
		gfx[offset+1]=((msb&0x20)>>1) | ((msb&0x10)>>4) | ((lsb&0x20)<<0) | ((lsb&0x10)>>3);
		gfx[offset+3]=((msb&0x80)>>3) | ((msb&0x40)>>6) | ((lsb&0x80)>>2) | ((lsb&0x40)>>5);

		offset+=4;
	}

	gfx = machine.root_device().memregion("gfx1")->base();
	size=machine.root_device().memregion("gfx1")->bytes();

	offset = size/2;
	for (i = size/2+size/4; i<size; i++)
	{
		int d1,d2,d3,d4;

		/* Expand 2bits into 4bits format */
		data = gfx[i];
		d1 = (data>>0) & 3;
		d2 = (data>>2) & 3;
		d3 = (data>>4) & 3;
		d4 = (data>>6) & 3;

		gfx[offset] = (d1<<2) | (d2<<6);
		offset++;

		gfx[offset] = (d3<<2) | (d4<<6);
		offset++;
	}
}

DRIVER_INIT_MEMBER(taito_f3_state,ringrage)
{
	m_f3_game=RINGRAGE;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,arabianm)
{
	m_f3_game=ARABIANM;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,ridingf)
{
	m_f3_game=RIDINGF;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,gseeker)
{
	m_f3_game=GSEEKER;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,gunlock)
{
	m_f3_game=GUNLOCK;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,elvactr)
{
	m_f3_game=EACTION2;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,cupfinal)
{
	m_f3_game=SCFINALS;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,trstaroj)
{
	m_f3_game=TRSTAR;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,scfinals)
{
	m_f3_game=SCFINALS;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,lightbr)
{
	m_f3_game=LIGHTBR;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,kaiserkn)
{
	m_f3_game=KAISERKN;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,dariusg)
{
	m_f3_game=DARIUSG;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,spcinvdj)
{
	m_f3_game=SPCINVDX;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,qtheater)
{
	m_f3_game=QTHEATER;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,spcinv95)
{
	m_f3_game=SPCINV95;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,gekirido)
{
	m_f3_game=GEKIRIDO;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,ktiger2)
{
	m_f3_game=KTIGER2;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,bubsymph)
{
	m_f3_game=BUBSYMPH;
	tile_decode(machine());
}


READ32_MEMBER(taito_f3_state::bubsympb_oki_r)
{
	return m_oki->read(space,0);
}
WRITE32_MEMBER(taito_f3_state::bubsympb_oki_w)
{
	//printf("write %08x %08x\n",data,mem_mask);
	if (ACCESSING_BITS_0_7) m_oki->write(space, 0,data&0xff);
	//if (mem_mask==0x000000ff) downcast<okim6295_device *>(device)->write(0,data&0xff);
	if (ACCESSING_BITS_16_23)
	{
		UINT8 *snd = memregion("oki")->base();
		int bank = (data & 0x000f0000) >> 16;
		// almost certainly wrong
		memcpy(snd+0x30000, snd+0x80000+0x30000+bank*0x10000, 0x10000);

		//printf("oki bank w %08x\n",data);
	}


}


DRIVER_INIT_MEMBER(taito_f3_state,bubsympb)
{
	m_f3_game=BUBSYMPH;
	//tile_decode(machine());

	/* expand gfx rom */
	{
		int i;
		UINT8 *gfx = memregion("gfx2")->base();

		for (i=0x200000;i<0x400000; i+=4)
		{
			UINT8 byte = gfx[i];
			gfx[i+0] = (byte & 0x80)? 1<<4 : 0<<4;
			gfx[i+0]|= (byte & 0x40)? 1<<0 : 0<<0;
			gfx[i+1] = (byte & 0x20)? 1<<4 : 0<<4;
			gfx[i+1]|= (byte & 0x10)? 1<<0 : 0<<0;
			gfx[i+2] = (byte & 0x08)? 1<<4 : 0<<4;
			gfx[i+2]|= (byte & 0x04)? 1<<0 : 0<<0;
			gfx[i+3] = (byte & 0x02)? 1<<4 : 0<<4;
			gfx[i+3]|= (byte & 0x01)? 1<<0 : 0<<0;
		}
	}

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x4a001c, 0x4a001f, read32_delegate(FUNC(taito_f3_state::bubsympb_oki_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x4a001c, 0x4a001f, write32_delegate(FUNC(taito_f3_state::bubsympb_oki_w),this));
}


DRIVER_INIT_MEMBER(taito_f3_state,bubblem)
{
	m_f3_game=BUBBLEM;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,cleopatr)
{
	m_f3_game=CLEOPATR;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,popnpop)
{
	m_f3_game=POPNPOP;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,landmakr)
{
	m_f3_game=LANDMAKR;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,landmkrp)
{
	UINT32 *RAM = (UINT32 *)memregion("maincpu")->base();

	/* For some reason the least significant byte in the last 2 long words of
	ROM is swapped.  As the roms have been verified ok, I assume this is some
	kind of basic security on the prototype development board to prevent 'release'
	roms running on it.  Easiest thing to do is switch the data around here */
	RAM[0x1ffff8/4]=0xffffffff; /* From 0xffffff03 */
	RAM[0x1ffffc/4]=0xffff0003; /* From 0xffff00ff */

	m_f3_game=LANDMAKR;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,pbobble3)
{
	m_f3_game=PBOBBLE3;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,pbobble4)
{
	m_f3_game=PBOBBLE4;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,quizhuhu)
{
	m_f3_game=QUIZHUHU;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,pbobble2)
{
	m_f3_game=PBOBBLE2;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,pbobbl2p)
{
	// has 040092: beq     $30000; (2+)
	// which eventually causes the game to crash
	//  -- protection check?? or some kind of checksum fail?

	UINT32 *ROM = (UINT32 *)memregion("maincpu")->base();

	/* protection? */
	ROM[0x40090/4]=0x00004e71|(ROM[0x40090/4]&0xffff0000);
	ROM[0x40094/4]=0x4e714e71;

	m_f3_game=PBOBBLE2;
	tile_decode(machine());
}



DRIVER_INIT_MEMBER(taito_f3_state,pbobbl2x)
{
	m_f3_game=PBOBBLE2;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,hthero95)
{
	m_f3_game=HTHERO95;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,kirameki)
{
	m_f3_game=KIRAMEKI;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,puchicar)
{
	m_f3_game=PUCHICAR;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,twinqix)
{
	m_f3_game=TWINQIX;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,arkretrn)
{
	m_f3_game=ARKRETRN;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,intcup94)
{
	m_f3_game=SCFINALS;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,recalh)
{
	m_f3_game=RECALH;
	tile_decode(machine());
}

DRIVER_INIT_MEMBER(taito_f3_state,commandw)
{
	m_f3_game=COMMANDW;
	tile_decode(machine());
}

/******************************************************************************/

GAME( 1992, ringrage, 0,        f3_224a, f3, taito_f3_state, ringrage, ROT0,   "Taito Corporation Japan",   "Ring Rage (Ver 2.3O 1992/08/09)", 0 )
GAME( 1992, ringragej,ringrage, f3_224a, f3, taito_f3_state, ringrage, ROT0,   "Taito Corporation",         "Ring Rage (Ver 2.3J 1992/08/09)", 0 )
GAME( 1992, ringrageu,ringrage, f3_224a, f3, taito_f3_state, ringrage, ROT0,   "Taito America Corporation", "Ring Rage (Ver 2.3A 1992/08/09)", 0 )
GAME( 1992, arabianm, 0,        f3_224a, f3, taito_f3_state, arabianm, ROT0,   "Taito Corporation Japan",   "Arabian Magic (Ver 1.0O 1992/07/06)", 0 )
GAME( 1992, arabianmj,arabianm, f3_224a, f3, taito_f3_state, arabianm, ROT0,   "Taito Corporation",         "Arabian Magic (Ver 1.0J 1992/07/06)", 0 )
GAME( 1992, arabianmu,arabianm, f3_224a, f3, taito_f3_state, arabianm, ROT0,   "Taito America Corporation", "Arabian Magic (Ver 1.0A 1992/07/06)", 0 )
GAME( 1992, ridingf,  0,        f3_224b, f3, taito_f3_state, ridingf,  ROT0,   "Taito Corporation Japan",   "Riding Fight (Ver 1.0O)", 0 )
GAME( 1992, ridingfj, ridingf,  f3_224b, f3, taito_f3_state, ridingf,  ROT0,   "Taito Corporation",         "Riding Fight (Ver 1.0J)", 0 )
GAME( 1992, ridingfu, ridingf,  f3_224b, f3, taito_f3_state, ridingf,  ROT0,   "Taito America Corporation", "Riding Fight (Ver 1.0A)", 0 )
GAME( 1992, gseeker,  0,        f3_224b_eeprom, f3, taito_f3_state, gseeker,  ROT90,  "Taito Corporation Japan",   "Grid Seeker: Project Storm Hammer (Ver 1.3O)", 0 )
GAME( 1992, gseekerj, gseeker,  f3_224b_eeprom, f3, taito_f3_state, gseeker,  ROT90,  "Taito Corporation",         "Grid Seeker: Project Storm Hammer (Ver 1.3J)", 0 )
GAME( 1992, gseekeru, gseeker,  f3_224b_eeprom, f3, taito_f3_state, gseeker,  ROT90,  "Taito America Corporation", "Grid Seeker: Project Storm Hammer (Ver 1.3A)", 0 )
GAME( 1992, commandw, 0,        f3_224b, f3, taito_f3_state, commandw, ROT0,   "Taito Corporation",         "Command War - Super Special Battle & War Game (Ver 0.0J, prototype)", MACHINE_IMPERFECT_GRAPHICS )
/* Most of the football games share some GFX roms but shouldn't be considered clones unless they have the same Taito game code for the program roms */
GAME( 1993, cupfinal, 0,        f3_224a, f3, taito_f3_state, cupfinal, ROT0,   "Taito Corporation Japan",   "Taito Cup Finals (Ver 1.0O 1993/02/28)", 0 )
GAME( 1993, hthero93, cupfinal, f3_224a, f3, taito_f3_state, cupfinal, ROT0,   "Taito Corporation",         "Hat Trick Hero '93 (Ver 1.0J 1993/02/28)", 0 )
GAME( 1993, trstar,   0,        f3,      f3, taito_f3_state, trstaroj, ROT0,   "Taito Corporation Japan",   "Top Ranking Stars (Ver 2.1O 1993/05/21) (New Version)", 0 )
GAME( 1993, trstarj,  trstar,   f3,      f3, taito_f3_state, trstaroj, ROT0,   "Taito Corporation",         "Top Ranking Stars (Ver 2.1J 1993/05/21) (New Version)", 0 )
GAME( 1993, trstaro,  trstar,   f3,      f3, taito_f3_state, trstaroj, ROT0,   "Taito Corporation Japan",   "Top Ranking Stars (Ver 2.1O 1993/05/21) (Old Version)", 0 )
GAME( 1993, trstaroj, trstar,   f3,      f3, taito_f3_state, trstaroj, ROT0,   "Taito Corporation",         "Top Ranking Stars (Ver 2.1J 1993/05/21) (Old Version)", 0 )
GAME( 1993, prmtmfgt, trstar,   f3,      f3, taito_f3_state, trstaroj, ROT0,   "Taito America Corporation", "Prime Time Fighter (Ver 2.1A 1993/05/21) (New Version)", 0 )
GAME( 1993, prmtmfgto,trstar,   f3,      f3, taito_f3_state, trstaroj, ROT0,   "Taito America Corporation", "Prime Time Fighter (Ver 2.1A 1993/05/21) (Old Version)", 0 )
GAME( 1993, gunlock,  0,        f3_224a, f3, taito_f3_state, gunlock,  ROT90,  "Taito Corporation Japan",   "Gunlock (Ver 2.3O 1994/01/20)", 0 )
GAME( 1993, rayforcej,gunlock,  f3_224a, f3, taito_f3_state, gunlock,  ROT90,  "Taito Corporation",         "Ray Force (Ver 2.3J 1994/01/20)", 0 )
GAME( 1993, rayforce, gunlock,  f3_224a, f3, taito_f3_state, gunlock,  ROT90,  "Taito America Corporation", "Ray Force (Ver 2.3A 1994/01/20)", 0 )
GAME( 1993, scfinals, 0,        f3_224a, f3, taito_f3_state, scfinals, ROT0,   "Taito Corporation Japan",   "Super Cup Finals (Ver 2.2O 1994/01/13)", 0 )
GAME( 1993, scfinalso,scfinals, f3_224a, f3, taito_f3_state, scfinals, ROT0,   "Taito Corporation Japan",   "Super Cup Finals (Ver 2.1O 1993/11/19)", 0 )
GAME( 1993, lightbr,  0,        f3_224a, f3, taito_f3_state, lightbr,  ROT0,   "Taito Corporation Japan",   "Light Bringer (Ver 2.2O 1994/04/08)", 0 )
GAME( 1993, lightbrj, lightbr,  f3_224a, f3, taito_f3_state, lightbr,  ROT0,   "Taito Corporation",         "Light Bringer (Ver 2.1J 1994/02/18)", 0 )
GAME( 1993, dungeonm, lightbr,  f3_224a, f3, taito_f3_state, lightbr,  ROT0,   "Taito Corporation Japan",   "Dungeon Magic (Ver 2.1O 1994/02/18)", 0 )
GAME( 1993, dungeonmu,lightbr,  f3_224a, f3, taito_f3_state, lightbr,  ROT0,   "Taito America Corporation", "Dungeon Magic (Ver 2.1A 1994/02/18)", 0 )
GAME( 1994, intcup94, 0,        f3_224a, f3, taito_f3_state, intcup94, ROT0,   "Taito Corporation Japan",   "International Cup '94 (Ver 2.2O 1994/05/26)", 0 )
GAME( 1994, hthero94, intcup94, f3_224a, f3, taito_f3_state, intcup94, ROT0,   "Taito America Corporation", "Hat Trick Hero '94 (Ver 2.2A 1994/05/26)", 0 )
GAME( 1994, kaiserkn, 0,        f3_224a, kn, taito_f3_state, kaiserkn, ROT0,   "Taito Corporation Japan",   "Kaiser Knuckle (Ver 2.1O 1994/07/29)", 0 )
GAME( 1994, kaiserknj,kaiserkn, f3_224a, kn, taito_f3_state, kaiserkn, ROT0,   "Taito Corporation",         "Kaiser Knuckle (Ver 2.1J 1994/07/29)", 0 )
GAME( 1994, gblchmp,  kaiserkn, f3_224a, kn, taito_f3_state, kaiserkn, ROT0,   "Taito America Corporation", "Global Champion (Ver 2.1A 1994/07/29)", 0 )
GAME( 1994, dankuga,  kaiserkn, f3_224a, kn, taito_f3_state, kaiserkn, ROT0,   "Taito Corporation",         "Dan-Ku-Ga (Ver 0.0J 1994/12/13, prototype)", 0 )
GAME( 1994, dariusg,  0,        f3,      f3, taito_f3_state, dariusg,  ROT0,   "Taito Corporation Japan",   "Darius Gaiden - Silver Hawk (Ver 2.5O 1994/09/19)", 0 )
GAME( 1994, dariusgj, dariusg,  f3,      f3, taito_f3_state, dariusg,  ROT0,   "Taito Corporation",         "Darius Gaiden - Silver Hawk (Ver 2.5J 1994/09/19)", 0 )
GAME( 1994, dariusgu, dariusg,  f3,      f3, taito_f3_state, dariusg,  ROT0,   "Taito America Corporation", "Darius Gaiden - Silver Hawk (Ver 2.5A 1994/09/19)", 0 )
GAME( 1994, dariusgx, dariusg,  f3,      f3, taito_f3_state, dariusg,  ROT0,   "Taito Corporation",         "Darius Gaiden - Silver Hawk Extra Version (Ver 2.7J 1995/03/06) (Official Hack)", 0 )
GAME( 1994, bublbob2, 0,        f3_224a, f3, taito_f3_state, bubsymph, ROT0,   "Taito Corporation Japan",   "Bubble Bobble II (Ver 2.6O 1994/12/16)", 0 )
GAME( 1994, bublbob2o,bublbob2, f3_224a, f3, taito_f3_state, bubsymph, ROT0,   "Taito Corporation Japan",   "Bubble Bobble II (Ver 2.5O 1994/10/05)", 0 )
GAME( 1994, bublbob2p,bublbob2, f3_224a, f3, taito_f3_state, bubsymph, ROT0,   "Taito Corporation Japan",   "Bubble Bobble II (Ver 0.0J 1993/12/13, prototype)", 0 )
GAME( 1994, bubsymphe,bublbob2, f3_224a, f3, taito_f3_state, bubsymph, ROT0,   "Taito Corporation Japan",   "Bubble Symphony (Ver 2.5O 1994/10/05)", 0 )
GAME( 1994, bubsymphu,bublbob2, f3_224a, f3, taito_f3_state, bubsymph, ROT0,   "Taito America Corporation", "Bubble Symphony (Ver 2.5A 1994/10/05)", 0 )
GAME( 1994, bubsymphj,bublbob2, f3_224a, f3, taito_f3_state, bubsymph, ROT0,   "Taito Corporation",         "Bubble Symphony (Ver 2.5J 1994/10/05)", 0 )
GAME( 1994, bubsymphb,bublbob2, bubsympb,f3, taito_f3_state, bubsympb, ROT0,   "bootleg",                   "Bubble Symphony (bootleg with OKI6295)", MACHINE_NOT_WORKING ) // backgrounds don't display
GAME( 1994, spcinvdj, spacedx,  f3,      f3, taito_f3_state, spcinvdj, ROT0,   "Taito Corporation",         "Space Invaders DX (Ver 2.6J 1994/09/14) (F3 Version)", 0 )
GAME( 1994, pwrgoal,  0,        f3_224a, f3, taito_f3_state, hthero95, ROT0,   "Taito Corporation Japan",   "Taito Power Goal (Ver 2.5O 1994/11/03)", 0 )
GAME( 1994, hthero95, pwrgoal,  f3_224a, f3, taito_f3_state, hthero95, ROT0,   "Taito Corporation",         "Hat Trick Hero '95 (Ver 2.5J 1994/11/03)", 0 )
GAME( 1994, hthero95u,pwrgoal,  f3_224a, f3, taito_f3_state, hthero95, ROT0,   "Taito America Corporation", "Hat Trick Hero '95 (Ver 2.5A 1994/11/03)", 0 )
GAME( 1994, qtheater, 0,        f3_224c, f3, taito_f3_state, qtheater, ROT0,   "Taito Corporation",         "Quiz Theater - 3tsu no Monogatari (Ver 2.3J 1994/11/10)", 0 )
GAME( 1994, elvactr,  0,        f3,      f3, taito_f3_state, elvactr,  ROT0,   "Taito Corporation Japan",   "Elevator Action Returns (Ver 2.2O 1995/02/20)", 0 )
GAME( 1994, elvactrj, elvactr,  f3,      f3, taito_f3_state, elvactr,  ROT0,   "Taito Corporation",         "Elevator Action Returns (Ver 2.2J 1995/02/20)", 0 )
GAME( 1994, elvact2u, elvactr,  f3,      f3, taito_f3_state, elvactr,  ROT0,   "Taito America Corporation", "Elevator Action II (Ver 2.2A 1995/02/20)", 0 )
/* There is also a prototype Elevator Action II (US) pcb with the graphics in a different rom format (same program code) */
GAME( 1994, recalh,   0,        f3_eeprom,f3, taito_f3_state, recalh,  ROT0,   "Taito Corporation",         "Recalhorn (Ver 1.42J 1994/5/11, prototype)", 0 )
GAME( 1995, spcinv95, 0,        f3_224a, f3, taito_f3_state, spcinv95, ROT270, "Taito Corporation Japan",   "Space Invaders '95: The Attack Of Lunar Loonies (Ver 2.5O 1995/06/14)", 0 )
GAME( 1995, spcinv95u,spcinv95, f3_224a, f3, taito_f3_state, spcinv95, ROT270, "Taito America Corporation", "Space Invaders '95: The Attack Of Lunar Loonies (Ver 2.5A 1995/06/14)", 0 )
GAME( 1995, akkanvdr, spcinv95, f3_224a, f3, taito_f3_state, spcinv95, ROT270, "Taito Corporation",         "Akkanbeder (Ver 2.5J 1995/06/14)", 0 )
GAME( 1995, twinqix,  0,        f3_224a, f3, taito_f3_state, twinqix,  ROT0,   "Taito America Corporation", "Twin Qix (Ver 1.0A 1995/01/17, prototype)", 0 )
GAME( 1995, quizhuhu, 0,        f3,      f3, taito_f3_state, quizhuhu, ROT0,   "Taito Corporation",         "Moriguchi Hiroko no Quiz de Hyuu!Hyuu! (Ver 2.2J 1995/05/25)", 0 )
GAME( 1995, pbobble2, 0,        f3,      f3, taito_f3_state, pbobbl2p, ROT0,   "Taito Corporation Japan",   "Puzzle Bobble 2 (Ver 2.3O 1995/07/31)", 0 )
GAME( 1995, pbobble2o,pbobble2, f3,      f3, taito_f3_state, pbobble2, ROT0,   "Taito Corporation Japan",   "Puzzle Bobble 2 (Ver 2.2O 1995/07/20)", 0 )
GAME( 1995, pbobble2j,pbobble2, f3,      f3, taito_f3_state, pbobble2, ROT0,   "Taito Corporation",         "Puzzle Bobble 2 (Ver 2.2J 1995/07/20)", 0 )
GAME( 1995, pbobble2u,pbobble2, f3,      f3, taito_f3_state, pbobble2, ROT0,   "Taito America Corporation", "Bust-A-Move Again (Ver 2.3A 1995/07/31)", 0 )
GAME( 1995, pbobble2x,pbobble2, f3,      f3, taito_f3_state, pbobbl2x, ROT0,   "Taito Corporation",         "Puzzle Bobble 2X (Ver 2.2J 1995/11/11)", 0 )
GAME( 1995, gekiridn, 0,        f3,      f3, taito_f3_state, gekirido, ROT270, "Taito Corporation",         "Gekirindan (Ver 2.3O 1995/09/21)", 0 )
GAME( 1995, gekiridnj,gekiridn, f3,      f3, taito_f3_state, gekirido, ROT270, "Taito Corporation",         "Gekirindan (Ver 2.3J 1995/09/21)", 0 )
GAME( 1995, tcobra2,  0,        f3,      f3, taito_f3_state, ktiger2,  ROT270, "Taito Corporation Japan",   "Twin Cobra II (Ver 2.1O 1995/11/30)", 0 )
GAME( 1995, tcobra2u, tcobra2,  f3,      f3, taito_f3_state, ktiger2,  ROT270, "Taito America Corporation", "Twin Cobra II (Ver 2.1A 1995/11/30)", 0 )
GAME( 1995, ktiger2,  tcobra2,  f3,      f3, taito_f3_state, ktiger2,  ROT270, "Taito Corporation",         "Kyukyoku Tiger II (Ver 2.1J 1995/11/30)", 0 )
GAME( 1995, bubblem,  0,        f3_224a, f3, taito_f3_state, bubblem,  ROT0,   "Taito Corporation Japan",   "Bubble Memories: The Story Of Bubble Bobble III (Ver 2.4O 1996/02/15)", 0 )
GAME( 1995, bubblemj, bubblem,  f3_224a, f3, taito_f3_state, bubblem,  ROT0,   "Taito Corporation",         "Bubble Memories: The Story Of Bubble Bobble III (Ver 2.3J 1996/02/07)", 0 )
GAME( 1996, cleopatr, 0,        f3_224a, f3, taito_f3_state, cleopatr, ROT0,   "Taito Corporation",         "Cleopatra Fortune (Ver 2.1J 1996/09/05)", 0 )
GAME( 1996, pbobble3, 0,        f3,      f3, taito_f3_state, pbobble3, ROT0,   "Taito Corporation",         "Puzzle Bobble 3 (Ver 2.1O 1996/09/27)", 0 )
GAME( 1996, pbobble3u,pbobble3, f3,      f3, taito_f3_state, pbobble3, ROT0,   "Taito Corporation",         "Puzzle Bobble 3 (Ver 2.1A 1996/09/27)", 0 )
GAME( 1996, pbobble3j,pbobble3, f3,      f3, taito_f3_state, pbobble3, ROT0,   "Taito Corporation",         "Puzzle Bobble 3 (Ver 2.1J 1996/09/27)", 0 )
GAME( 1997, arkretrn, 0,        f3,      f3, taito_f3_state, arkretrn, ROT0,   "Taito Corporation",         "Arkanoid Returns (Ver 2.02O 1997/02/10)", 0 )
GAME( 1997, arkretrnu,arkretrn, f3,      f3, taito_f3_state, arkretrn, ROT0,   "Taito Corporation",         "Arkanoid Returns (Ver 2.02A 1997/02/10)", 0 )
GAME( 1997, arkretrnj,arkretrn, f3,      f3, taito_f3_state, arkretrn, ROT0,   "Taito Corporation",         "Arkanoid Returns (Ver 2.02J 1997/02/10)", 0 )
GAME( 1997, kirameki, 0,        f3_224a, f3, taito_f3_state, kirameki, ROT0,   "Taito Corporation",         "Kirameki Star Road (Ver 2.10J 1997/08/29)", 0 )
GAME( 1997, puchicar, 0,        f3,      f3, taito_f3_state, puchicar, ROT0,   "Taito Corporation",         "Puchi Carat (Ver 2.02O 1997/10/29)", 0 )
GAME( 1997, puchicarj,puchicar, f3,      f3, taito_f3_state, puchicar, ROT0,   "Taito Corporation",         "Puchi Carat (Ver 2.02J 1997/10/29)", 0 )
GAME( 1997, pbobble4, 0,        f3,      f3, taito_f3_state, pbobble4, ROT0,   "Taito Corporation",         "Puzzle Bobble 4 (Ver 2.04O 1997/12/19)", 0 )
GAME( 1997, pbobble4j,pbobble4, f3,      f3, taito_f3_state, pbobble4, ROT0,   "Taito Corporation",         "Puzzle Bobble 4 (Ver 2.04J 1997/12/19)", 0 )
GAME( 1997, pbobble4u,pbobble4, f3,      f3, taito_f3_state, pbobble4, ROT0,   "Taito Corporation",         "Puzzle Bobble 4 (Ver 2.04A 1997/12/19)", 0 )
GAME( 1997, popnpop,  0,        f3,      f3, taito_f3_state, popnpop,  ROT0,   "Taito Corporation",         "Pop'n Pop (Ver 2.07O 1998/02/09)", 0 )
GAME( 1997, popnpopj, popnpop,  f3,      f3, taito_f3_state, popnpop,  ROT0,   "Taito Corporation",         "Pop'n Pop (Ver 2.07J 1998/02/09)", 0 )
GAME( 1997, popnpopu, popnpop,  f3,      f3, taito_f3_state, popnpop,  ROT0,   "Taito Corporation",         "Pop'n Pop (Ver 2.07A 1998/02/09)", 0 )
GAME( 1998, landmakr, 0,        f3,      f3, taito_f3_state, landmakr, ROT0,   "Taito Corporation",         "Land Maker (Ver 2.01J 1998/06/01)", 0 )
GAME( 1998, landmakrp,landmakr, f3,      f3, taito_f3_state, landmkrp, ROT0,   "Taito Corporation",         "Land Maker (Ver 2.02O 1998/06/02, prototype)", 0 )
