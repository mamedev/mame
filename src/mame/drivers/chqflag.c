// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Manuel Abadia
/***************************************************************************

    Chequered Flag / Checkered Flag (GX717) (c) Konami 1988

    Notes:
    - 007232 volume & panning control is almost certainly wrong.
    - Needs HW tests or side-by-side tests to determine if the protection
      is 100% ok now;

    2008-07
    Dip locations and recommended settings verified with manual

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/konami.h"
#include "sound/2151intf.h"
#include "includes/chqflag.h"
#include "includes/konamipt.h"
#include "chqflag.lh"


/* these trampolines are less confusing than nested address_map_bank_devices */
READ8_MEMBER(chqflag_state::k051316_1_ramrom_r)
{
	if (m_k051316_readroms)
		return m_k051316_1->rom_r(space, offset);
	else
		return m_k051316_1->read(space, offset);
}

READ8_MEMBER(chqflag_state::k051316_2_ramrom_r)
{
	if (m_k051316_readroms)
		return m_k051316_2->rom_r(space, offset);
	else
		return m_k051316_2->read(space, offset);
}

WRITE8_MEMBER(chqflag_state::chqflag_bankswitch_w)
{
	/* bits 0-4 = ROM bank # (0x00-0x11) */
	int bankaddress = data & 0x1f;
	if (bankaddress < (0x50000 / 0x4000))
		m_rombank->set_entry(bankaddress);

	/* bit 5 = select work RAM or k051316 + palette */
	m_bank1000->set_bank((data & 0x20) >> 5);

	/* other bits unknown/unused */
}

WRITE8_MEMBER(chqflag_state::chqflag_vreg_w)
{
	/* bits 0 & 1 = coin counters */
	coin_counter_w(machine(), 1, data & 0x01);
	coin_counter_w(machine(), 0, data & 0x02);

	/* bit 4 = enable rom reading through K051316 #1 & #2 */
	m_k051316_readroms = (data & 0x10);

	/* Bits 3-7 probably control palette dimming in a similar way to TMNT2/Sunset Riders, */
	/* however I don't have enough evidence to determine the exact behaviour. */
	/* Bits 3 and 7 are set in night stages, where the background should get darker and */
	/* the headlight (which have the shadow bit set) become highlights */
	/* Maybe one of the bits inverts the SHAD line while the other darkens the background. */
	if (data & 0x08)
		m_palette->set_shadow_factor(1 / PALETTE_DEFAULT_SHADOW_FACTOR);
	else
		m_palette->set_shadow_factor(PALETTE_DEFAULT_SHADOW_FACTOR);

	if ((data & 0x80) != m_last_vreg)
	{
		double brt = (data & 0x80) ? PALETTE_DEFAULT_SHADOW_FACTOR : 1.0;
		int i;

		m_last_vreg = data & 0x80;

		/* only affect the background */
		for (i = 512; i < 1024; i++)
			m_palette->set_pen_contrast(i, brt);
	}

//if ((data & 0xf8) && (data & 0xf8) != 0x88)
//  popmessage("chqflag_vreg_w %02x",data);


	/* other bits unknown. bit 5 is used. */
}

WRITE8_MEMBER(chqflag_state::select_analog_ctrl_w)
{
	m_analog_ctrl = data;
}

READ8_MEMBER(chqflag_state::analog_read_r)
{
	switch (m_analog_ctrl & 0x03)
	{
		case 0x00: return (m_accel = ioport("IN3")->read());    /* accelerator */
		case 0x01: return (m_wheel = ioport("IN4")->read());    /* steering */
		case 0x02: return m_accel;                      /* accelerator (previous?) */
		case 0x03: return m_wheel;                      /* steering (previous?) */
	}

	return 0xff;
}

WRITE8_MEMBER(chqflag_state::chqflag_sh_irqtrigger_w)
{
	soundlatch2_byte_w(space, 0, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


/****************************************************************************/

static ADDRESS_MAP_START( chqflag_map, AS_PROGRAM, 8, chqflag_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x1fff) AM_DEVICE("bank1000", address_map_bank_device, amap8)
	AM_RANGE(0x2000, 0x2007) AM_DEVREADWRITE("k051960", k051960_device, k051937_r, k051937_w)            /* Sprite control registers */
	AM_RANGE(0x2400, 0x27ff) AM_DEVREADWRITE("k051960", k051960_device, k051960_r, k051960_w)            /* Sprite RAM */
	AM_RANGE(0x2800, 0x2fff) AM_READ(k051316_2_ramrom_r) AM_DEVWRITE("k051316_2", k051316_device, write) /* 051316 zoom/rotation (chip 2) */
	AM_RANGE(0x3000, 0x3000) AM_WRITE(soundlatch_byte_w)                        /* sound code # */
	AM_RANGE(0x3001, 0x3001) AM_WRITE(chqflag_sh_irqtrigger_w)                  /* cause interrupt on audio CPU */
	AM_RANGE(0x3002, 0x3002) AM_WRITE(chqflag_bankswitch_w)                     /* bankswitch control */
	AM_RANGE(0x3003, 0x3003) AM_WRITE(chqflag_vreg_w)                           /* enable K051316 ROM reading */
	AM_RANGE(0x3100, 0x3100) AM_READ_PORT("DSW1")                               /* DIPSW #1  */
	AM_RANGE(0x3200, 0x3200) AM_READ_PORT("IN1")                                /* COINSW, STARTSW, test mode */
	AM_RANGE(0x3201, 0x3201) AM_READ_PORT("IN0")                                /* DIPSW #3, SW 4 */
	AM_RANGE(0x3203, 0x3203) AM_READ_PORT("DSW2")                               /* DIPSW #2 */
	AM_RANGE(0x3300, 0x3300) AM_WRITE(watchdog_reset_w)                         /* watchdog timer */
	AM_RANGE(0x3400, 0x341f) AM_DEVREADWRITE("k051733", k051733_device, read, write)                    /* 051733 (protection) */
	AM_RANGE(0x3500, 0x350f) AM_DEVWRITE("k051316_1", k051316_device, ctrl_w)                            /* 051316 control registers (chip 1) */
	AM_RANGE(0x3600, 0x360f) AM_DEVWRITE("k051316_2", k051316_device, ctrl_w)                            /* 051316 control registers (chip 2) */
	AM_RANGE(0x3700, 0x3700) AM_WRITE(select_analog_ctrl_w)                     /* select accelerator/wheel */
	AM_RANGE(0x3701, 0x3701) AM_READ_PORT("IN2")                                /* Brake + Shift + ? */
	AM_RANGE(0x3702, 0x3702) AM_READWRITE(analog_read_r, select_analog_ctrl_w)  /* accelerator/wheel */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("rombank")                              /* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("maincpu", 0x48000)               /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bank1000_map, AS_PROGRAM, 8, chqflag_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1000, 0x17ff) AM_READ(k051316_1_ramrom_r) AM_DEVWRITE("k051316_1", k051316_device, write)
	AM_RANGE(0x1800, 0x1fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
ADDRESS_MAP_END


WRITE8_MEMBER(chqflag_state::k007232_bankswitch_w)
{
	int bank_A, bank_B;

	/* banks # for the 007232 (chip 1) */
	bank_A = ((data >> 4) & 0x03);
	bank_B = ((data >> 6) & 0x03);
	m_k007232_1->set_bank(bank_A, bank_B);

	/* banks # for the 007232 (chip 2) */
	bank_A = ((data >> 0) & 0x03);
	bank_B = ((data >> 2) & 0x03);
	m_k007232_2->set_bank(bank_A, bank_B);
}

static ADDRESS_MAP_START( chqflag_sound_map, AS_PROGRAM, 8, chqflag_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM /* ROM */
	AM_RANGE(0x8000, 0x87ff) AM_RAM /* RAM */
	AM_RANGE(0x9000, 0x9000) AM_WRITE(k007232_bankswitch_w) /* 007232 bankswitch */
	AM_RANGE(0xa000, 0xa00d) AM_DEVREADWRITE("k007232_1", k007232_device, read, write)  /* 007232 (chip 1) */
	AM_RANGE(0xa01c, 0xa01c) AM_WRITE(k007232_extvolume_w)  /* extra volume, goes to the 007232 w/ A4 */
															/* selecting a different latch for the external port */
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232_2", k007232_device, read, write)  /* 007232 (chip 2) */
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)   /* YM2151 */
	AM_RANGE(0xd000, 0xd000) AM_READ(soundlatch_byte_r)         /* soundlatch_byte_r */
	AM_RANGE(0xe000, 0xe000) AM_READ(soundlatch2_byte_r)         /* engine sound volume */
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP                    /* ??? */
ADDRESS_MAP_END


static INPUT_PORTS_START( chqflag )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* Invalid = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )   /* Manual says it's not used */
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )   /* Manual says it's not used */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:4" )   /* Manual says it's not used */

	PORT_START("IN1")
	/* COINSW + STARTSW */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	/* DIPSW #3 */
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW3:1" )   /* Manual says it's not used */
	PORT_DIPNAME( 0x40, 0x40, "Title" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x40, "Chequered Flag" )
	PORT_DIPSETTING(    0x00, "Checkered Flag" )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW3:3" )

	PORT_START("IN2")   /* Brake, Shift + ??? */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* if this is set, it goes directly to test mode */
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNKNOWN )   /* if bit 7 == 0, the game resets */

	PORT_START("IN3")   /* Accelerator */
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(50) PORT_KEYDELTA(5)

	PORT_START("IN4")   /* Driving wheel */
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xef) PORT_SENSITIVITY(80) PORT_KEYDELTA(8)
INPUT_PORTS_END

static INPUT_PORTS_START( chqflagj )
	PORT_INCLUDE( chqflag )

	PORT_MODIFY("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), " 1 Coin/1 Credit", SW1)
	// Manual says 1-5, 1-6, 1-7 and 1-8 are not used, but they work

	PORT_MODIFY("IN1")
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:2" )   /* Manual says it's not used */
INPUT_PORTS_END



WRITE8_MEMBER(chqflag_state::volume_callback0)
{
	// volume/pan for one of the channels on this chip
	// which channel and which bits are left/right is a guess
	m_k007232_1->set_volume(0, (data & 0x0f) * 0x11/2, (data >> 4) * 0x11/2);
}

WRITE8_MEMBER(chqflag_state::k007232_extvolume_w)
{
	// volume/pan for one of the channels on this chip
	// which channel and which bits are left/right is a guess
	m_k007232_1->set_volume(1, (data & 0x0f) * 0x11/2, (data >> 4) * 0x11/2);
}

WRITE8_MEMBER(chqflag_state::volume_callback1)
{
	m_k007232_2->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232_2->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void chqflag_state::machine_start()
{
	m_rombank->configure_entries(0, 0x50000 / 0x4000, memregion("maincpu")->base(), 0x4000);

	save_item(NAME(m_k051316_readroms));
	save_item(NAME(m_last_vreg));
	save_item(NAME(m_analog_ctrl));
	save_item(NAME(m_accel));
	save_item(NAME(m_wheel));
}

void chqflag_state::machine_reset()
{
	m_k051316_readroms = 0;
	m_last_vreg = 0;
	m_analog_ctrl = 0;
	m_accel = 0;
	m_wheel = 0;
}

static MACHINE_CONFIG_START( chqflag, chqflag_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI, XTAL_24MHz/2/4)    /* 052001 (verified on pcb) */
	MCFG_CPU_PROGRAM_MAP(chqflag_map)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(chqflag_sound_map)

	MCFG_DEVICE_ADD("bank1000", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(bank1000_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDRBUS_WIDTH(13)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x1000)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_24MHz/3, 528, 96, 400, 256, 16, 240) // measured Vsync 59.17hz Hsync 15.13 / 15.19khz
//  6MHz dotclock is more realistic, however needs drawing updates. replace when ready
//  MCFG_SCREEN_RAW_PARAMS(XTAL_24MHz/4, 396, hbend, hbstart, 256, 16, 240)
	MCFG_SCREEN_UPDATE_DRIVER(chqflag_state, screen_update_chqflag)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_DEVICE_ADD("k051960", K051960, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051960_SCREEN_TAG("screen")
	MCFG_K051960_CB(chqflag_state, sprite_callback)
	MCFG_K051960_IRQ_HANDLER(INPUTLINE("maincpu", KONAMI_IRQ_LINE))
	MCFG_K051960_NMI_HANDLER(INPUTLINE("maincpu", INPUT_LINE_NMI))

	MCFG_DEVICE_ADD("k051316_1", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_OFFSETS(7, 0)
	MCFG_K051316_CB(chqflag_state, zoom_callback_1)

	MCFG_DEVICE_ADD("k051316_2", K051316, 0)
	MCFG_GFX_PALETTE("palette")
	MCFG_K051316_BPP(8)
	MCFG_K051316_LAYER_MASK(0xc0)
	MCFG_K051316_WRAP(1)
	MCFG_K051316_CB(chqflag_state, zoom_callback_2)

	MCFG_K051733_ADD("k051733")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", XTAL_3_579545MHz) /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", INPUT_LINE_NMI))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.00)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.00)

	MCFG_SOUND_ADD("k007232_1", K007232, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(chqflag_state, volume_callback0))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.20)

	MCFG_SOUND_ADD("k007232_2", K007232, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_K007232_PORT_WRITE_HANDLER(WRITE8(chqflag_state, volume_callback1))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.20)
MACHINE_CONFIG_END

ROM_START( chqflag )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 052001 code */
	ROM_LOAD( "717e10",     0x00000, 0x40000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )
	ROM_LOAD( "717h02",     0x40000, 0x10000, CRC(f5bd4e78) SHA1(7bab02152d055a6c3a322c88e7ee0b85a39d8ef2) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "717e01",     0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "717e04",     0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )
	ROM_LOAD32_WORD( "717e05",     0x000002, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )

	ROM_REGION( 0x020000, "k051316_1", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e06.n16",     0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )

	ROM_REGION( 0x100000, "k051316_2", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e07.l20",     0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )
	ROM_LOAD( "717e08.l22",     0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )
	ROM_LOAD( "717e11.n20",     0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )
	ROM_LOAD( "717e12.n22",     0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )

	ROM_REGION( 0x080000, "k007232_1", 0 )  /* 007232 data (chip 1) */
	ROM_LOAD( "717e03",     0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, "k007232_2", 0 )  /* 007232 data (chip 2) */
	ROM_LOAD( "717e09",     0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END

ROM_START( chqflagj )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 052001 code */
	ROM_LOAD( "717e10",     0x00000, 0x40000, CRC(72fc56f6) SHA1(433ea9a33f0230e046c731c70060f6a38db14ac7) )
	ROM_LOAD( "717j02.bin", 0x40000, 0x10000, CRC(05355daa) SHA1(130ddbc289c077565e44f33c63a63963e6417e19) )

	ROM_REGION( 0x08000, "audiocpu", 0 )    /* Z80 code */
	ROM_LOAD( "717e01",     0x000000, 0x008000, CRC(966b8ba8) SHA1(ab7448cb61fa5922b1d8ae5f0d0f42d734ed4f93) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "717e04",     0x000000, 0x080000, CRC(1a50a1cc) SHA1(bc16fab84c637ed124e37b115ddc0149560b727d) )
	ROM_LOAD32_WORD( "717e05",     0x000002, 0x080000, CRC(46ccb506) SHA1(3ed1f54744fc5cdc0f48e42f250c366267a8199a) )

	ROM_REGION( 0x020000, "k051316_1", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e06.n16",     0x000000, 0x020000, CRC(1ec26c7a) SHA1(05b5b522c5ebf5d0a71a7fc39ec9382008ef33c8) )

	ROM_REGION( 0x100000, "k051316_2", 0 )      /* zoom/rotate */
	ROM_LOAD( "717e07.l20",     0x000000, 0x040000, CRC(b9a565a8) SHA1(a11782f7336e5ad58a4c6ea81f2eeac35d5e7d0a) )
	ROM_LOAD( "717e08.l22",     0x040000, 0x040000, CRC(b68a212e) SHA1(b2bd121a43552c3ade528ac763a0df40c3e648e0) )
	ROM_LOAD( "717e11.n20",     0x080000, 0x040000, CRC(ebb171ec) SHA1(d65d4a6b169ce03e4427b2a397484634f938236b) )
	ROM_LOAD( "717e12.n22",     0x0c0000, 0x040000, CRC(9269335d) SHA1(af298c8cff50d707d6abc806065f8e931f975dc0) )

	ROM_REGION( 0x080000, "k007232_1", 0 )  /* 007232 data (chip 1) */
	ROM_LOAD( "717e03",     0x000000, 0x080000, CRC(ebe73c22) SHA1(fad3334e5e91bf8d11b74ffdbbfd57567e6f6f8c) )

	ROM_REGION( 0x080000, "k007232_2", 0 )  /* 007232 data (chip 2) */
	ROM_LOAD( "717e09",     0x000000, 0x080000, CRC(d74e857d) SHA1(00c851c857650d67fc4caccea4461d99be4acb3c) )
ROM_END


//     YEAR, NAME,     PARENT,  MACHINE, INPUT,    INIT,MONITOR,COMPANY,FULLNAME,FLAGS
GAMEL( 1988, chqflag,  0,       chqflag, chqflag, driver_device,  0,   ROT90,  "Konami", "Chequered Flag", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_chqflag )
GAMEL( 1988, chqflagj, chqflag, chqflag, chqflagj, driver_device, 0,   ROT90,  "Konami", "Chequered Flag (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_chqflag )
