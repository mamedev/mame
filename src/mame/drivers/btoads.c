// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    BattleToads

    driver by Aaron Giles

**************************************************************************/

#include "emu.h"
#include "includes/btoads.h"


#define CPU_CLOCK           XTAL_64MHz
#define VIDEO_CLOCK         XTAL_20MHz
#define SOUND_CLOCK         XTAL_24MHz



/*************************************
 *
 *  Machine init
 *
 *************************************/

void btoads_state::machine_start()
{
	save_item(NAME(m_main_to_sound_data));
	save_item(NAME(m_main_to_sound_ready));
	save_item(NAME(m_sound_to_main_data));
	save_item(NAME(m_sound_to_main_ready));
	save_item(NAME(m_sound_int_state));
}



/*************************************
 *
 *  Main -> sound CPU communication
 *
 *************************************/

void btoads_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_ID_DELAYED_SOUND:
			m_main_to_sound_data = param;
			m_main_to_sound_ready = 1;
			m_audiocpu->signal_interrupt_trigger();

			// use a timer to make long transfers faster
			timer_set(attotime::from_usec(50), TIMER_ID_NOP);
			break;
	}
}


WRITE16_MEMBER( btoads_state::main_sound_w )
{
	if (ACCESSING_BITS_0_7)
		synchronize(TIMER_ID_DELAYED_SOUND, data & 0xff);
}


READ16_MEMBER( btoads_state::main_sound_r )
{
	m_sound_to_main_ready = 0;
	return m_sound_to_main_data;
}


CUSTOM_INPUT_MEMBER( btoads_state::main_to_sound_r )
{
	return m_main_to_sound_ready;
}


CUSTOM_INPUT_MEMBER( btoads_state::sound_to_main_r )
{
	return m_sound_to_main_ready;
}



/*************************************
 *
 *  Sound -> main CPU communication
 *
 *************************************/

WRITE8_MEMBER( btoads_state::sound_data_w )
{
	m_sound_to_main_data = data;
	m_sound_to_main_ready = 1;
}


READ8_MEMBER( btoads_state::sound_data_r )
{
	m_main_to_sound_ready = 0;
	return m_main_to_sound_data;
}


READ8_MEMBER( btoads_state::sound_ready_to_send_r )
{
	return m_sound_to_main_ready ? 0x00 : 0x80;
}


READ8_MEMBER( btoads_state::sound_data_ready_r )
{
	if (m_audiocpu->pc() == 0xd50 && !m_main_to_sound_ready)
		m_audiocpu->spin_until_interrupt();
	return m_main_to_sound_ready ? 0x00 : 0x80;
}



/*************************************
 *
 *  Sound CPU interrupt generation
 *
 *************************************/

WRITE8_MEMBER( btoads_state::sound_int_state_w )
{
	/* top bit controls BSMT2000 reset */
	if (!(m_sound_int_state & 0x80) && (data & 0x80))
		m_bsmt->reset();

	/* also clears interrupts */
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	m_sound_int_state = data;
}



/*************************************
 *
 *  Sound CPU BSMT2000 communication
 *
 *************************************/

READ8_MEMBER( btoads_state::bsmt_ready_r )
{
	return m_bsmt->read_status() << 7;
}


WRITE8_MEMBER( btoads_state::bsmt2000_port_w )
{
	m_bsmt->write_reg(offset >> 8);
	m_bsmt->write_data(((offset & 0xff) << 8) | data);
}



/*************************************
 *
 *  Main CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, btoads_state )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM
	AM_RANGE(0x20000000, 0x2000007f) AM_READ_PORT("P1")
	AM_RANGE(0x20000080, 0x200000ff) AM_READ_PORT("P2")
	AM_RANGE(0x20000100, 0x2000017f) AM_READ_PORT("P3")
	AM_RANGE(0x20000180, 0x200001ff) AM_READ_PORT("UNK")
	AM_RANGE(0x20000200, 0x2000027f) AM_READ_PORT("SPECIAL")
	AM_RANGE(0x20000280, 0x200002ff) AM_READ_PORT("SW1")
	AM_RANGE(0x20000000, 0x200000ff) AM_WRITEONLY AM_SHARE("sprite_scale")
	AM_RANGE(0x20000100, 0x2000017f) AM_WRITEONLY AM_SHARE("sprite_control")
	AM_RANGE(0x20000180, 0x200001ff) AM_WRITE(display_control_w)
	AM_RANGE(0x20000200, 0x2000027f) AM_WRITE(scroll0_w)
	AM_RANGE(0x20000280, 0x200002ff) AM_WRITE(scroll1_w)
	AM_RANGE(0x20000300, 0x2000037f) AM_READWRITE(paletteram_r, paletteram_w)
	AM_RANGE(0x20000380, 0x200003ff) AM_READWRITE(main_sound_r, main_sound_w)
	AM_RANGE(0x20000400, 0x2000047f) AM_WRITE(misc_control_w)
	AM_RANGE(0x40000000, 0x4000000f) AM_WRITENOP    /* watchdog? */
	AM_RANGE(0x60000000, 0x6003ffff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xa0000000, 0xa03fffff) AM_READWRITE(vram_fg_display_r, vram_fg_display_w) AM_SHARE("vram_fg0")
	AM_RANGE(0xa4000000, 0xa43fffff) AM_READWRITE(vram_fg_draw_r, vram_fg_draw_w) AM_SHARE("vram_fg1")
	AM_RANGE(0xa8000000, 0xa87fffff) AM_RAM AM_SHARE("vram_fg_data")
	AM_RANGE(0xa8800000, 0xa8ffffff) AM_WRITENOP
	AM_RANGE(0xb0000000, 0xb03fffff) AM_READWRITE(vram_bg0_r, vram_bg0_w) AM_SHARE("vram_bg0")
	AM_RANGE(0xb4000000, 0xb43fffff) AM_READWRITE(vram_bg1_r, vram_bg1_w) AM_SHARE("vram_bg1")
	AM_RANGE(0xc0000000, 0xc00003ff) AM_DEVREADWRITE("maincpu", tms34020_device, io_register_r, io_register_w)
	AM_RANGE(0xfc000000, 0xffffffff) AM_ROM AM_REGION("user1", 0)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory map
 *
 *************************************/

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, btoads_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, btoads_state )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(bsmt2000_port_w)
	AM_RANGE(0x8000, 0x8000) AM_READWRITE(sound_data_r, sound_data_w)
	AM_RANGE(0x8002, 0x8002) AM_WRITE(sound_int_state_w)
	AM_RANGE(0x8004, 0x8004) AM_READ(sound_data_ready_r)
	AM_RANGE(0x8005, 0x8005) AM_READ(sound_ready_to_send_r)
	AM_RANGE(0x8006, 0x8006) AM_READ(bsmt_ready_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( btoads )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNK")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SPECIAL")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, btoads_state, sound_to_main_r, NULL)
	PORT_SERVICE_NO_TOGGLE( 0x0002, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, btoads_state, main_to_sound_r, NULL)
	PORT_BIT( 0xff7c, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x0001, 0x0000, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0002, 0x0000, DEF_STR( Stereo ))      PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0004, 0x0000, "Common Coin Mech")     PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0008, 0x0008, "Three Players")        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Free_Play ))   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0020, 0x0020, "Blood Free Mode")      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0040, 0x0040, "Credit Retention")     PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( btoads, btoads_state )

	MCFG_CPU_ADD("maincpu", TMS34020, CPU_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TMS340X0_HALT_ON_RESET(FALSE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(VIDEO_CLOCK/2) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_RGB32_CB(btoads_state, scanline_update)     /* scanline updater (RGB32) */
	MCFG_TMS340X0_TO_SHIFTREG_CB(btoads_state, to_shiftreg)  /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(btoads_state, from_shiftreg) /* read from shiftreg function */

	MCFG_CPU_ADD("audiocpu", Z80, SOUND_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(btoads_state, irq0_line_assert,  183)

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 640, 0, 512, 257, 0, 224)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", tms34020_device, tms340x0_rgb32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_BSMT2000_ADD("bsmt", SOUND_CLOCK)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( btoads )
	ROM_REGION( 0x10000, "audiocpu", 0 )    /* sound program, M27C256B rom */
	ROM_LOAD( "bt.u102", 0x0000, 0x8000, CRC(a90b911a) SHA1(6ec25161e68df1c9870d48cc2b1f85cd1a49aba9) )

	ROM_REGION16_LE( 0x800000, "user1", 0 ) /* 34020 code, M27C322 roms */
	ROM_LOAD32_WORD( "btc0-p0.u120", 0x000000, 0x400000, CRC(0dfd1e35) SHA1(733a0a4235bebd598c600f187ed2628f28cf9bd0) )
	ROM_LOAD32_WORD( "btc0-p1.u121", 0x000002, 0x400000, CRC(df7487e1) SHA1(67151b900767bb2653b5261a98c81ff8827222c3) )

	ROM_REGION( 0x1000000, "bsmt", 0 )  /* BSMT data, M27C160 rom */
	ROM_LOAD( "btc0-s.u109", 0x00000, 0x200000, CRC(d9612ddb) SHA1(f186dfb013e81abf81ba8ac5dc7eb731c1ad82b6) )

	ROM_REGION( 0x080a, "plds", 0 )
	ROM_LOAD( "u10.bin",  0x0000, 0x0157, CRC(b1144178) SHA1(15fb047adee4125e9fcf04171e0a502655e0a3d8) ) /* GAL20V8A-15LP Located at U10. */
	ROM_LOAD( "u11.bin",  0x0000, 0x0157, CRC(7c6beb96) SHA1(2f19d21889dd765b344ad7d257ea7c244baebec2) ) /* GAL20V8A-15LP Located at U11. */
	ROM_LOAD( "u57.bin",  0x0000, 0x0157, CRC(be355a56) SHA1(975238bb1ea8fef14458d6f264a82aa77ecf865d) ) /* GAL20V8A-15LP Located at U57. */
	ROM_LOAD( "u58.bin",  0x0000, 0x0157, CRC(41ed339c) SHA1(5853c805a902e6d12c979958d878d1cefd6908cc) ) /* GAL20V8A-15LP Located at U58. */
	ROM_LOAD( "u90.bin",  0x0000, 0x0157, CRC(a0d0c3f1) SHA1(47464c2ef9fadbba933df27767f377e0c29158aa) ) /* GAL20V8A-15LP Located at U90. */
	ROM_LOAD( "u144.bin", 0x0000, 0x0157, CRC(8597017f) SHA1(003d7b5de57e48f831ab211e2783fff338ce2f32) ) /* GAL20V8A-15LP Located at U144. */
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1994, btoads, 0, btoads, btoads, driver_device, 0,  ROT0, "Rare / Electronic Arts", "Battletoads", MACHINE_SUPPORTS_SAVE )
