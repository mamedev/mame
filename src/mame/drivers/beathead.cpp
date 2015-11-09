// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari "Stella on Steroids" hardware

****************************************************************************

    Games supported:
        * BeatHead

    Known bugs:
        * none known

****************************************************************************

    Memory map

    ===================================================================================================
    MAIN CPU
    ===================================================================================================
    00000000-0001FFFFF  R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Main RAM
    01800000-01BFFFFFF  R     xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   Main ROM
    40000000-4000007FF  R/W   -------- -------- -------- xxxxxxxx   EEPROM
    41000000            R     -------- -------- -------- xxxxxxxx   Data from sound board
    41000000              W   -------- -------- -------- xxxxxxxx   Data to sound board
    41000100            R     -------- -------- -------- -----xxx   Interrupt enables
                              -------- -------- -------- -----x--      (scanline int enable)
                              -------- -------- -------- ------x-      (unknown int enable)
                              -------- -------- -------- -------x      (unknown int enable)
    41000100              W   -------- -------- -------- --------   Interrupt acknowledge
    41000104              W   -------- -------- -------- --------   Unknown int disable
    41000108              W   -------- -------- -------- --------   Unknown int disable
    4100010c              W   -------- -------- -------- --------   Scanline int disable
    41000114              W   -------- -------- -------- --------   Unknown int enable
    41000118              W   -------- -------- -------- --------   Unknown int enable
    4100011c              W   -------- -------- -------- --------   Scanline int enable
    41000200            R     -------- -------- xxxx--xx xxxx--xx   Player 2/3 inputs
                        R     -------- -------- xxxx---- --------      (player 3 joystick UDLR)
                        R     -------- -------- ------x- --------      (player 3 button 1)
                        R     -------- -------- -------x --------      (player 3 button 2)
                        R     -------- -------- -------- xxxx----      (player 2 joystick UDLR)
                        R     -------- -------- -------- ------x-      (player 2 button 1)
                        R     -------- -------- -------- -------x      (player 2 button 2)
    41000204            R     -------- -------- xxxx--xx xxxx--xx   Player 1/4 inputs
                        R     -------- -------- xxxx---- --------      (player 1 joystick UDLR)
                        R     -------- -------- ------x- --------      (player 1 button 1)
                        R     -------- -------- -------x --------      (player 1 button 2)
                        R     -------- -------- -------- xxxx----      (player 4 joystick UDLR)
                        R     -------- -------- -------- ------x-      (player 4 button 1)
                        R     -------- -------- -------- -------x      (player 4 button 2)
    41000208              W   -------- -------- -------- --------   Sound /RESET assert
    4100020C              W   -------- -------- -------- --------   Sound /RESET deassert
    41000220              W   -------- -------- -------- --------   Coin counter assert
    41000224              W   -------- -------- -------- --------   Coin counter deassert
    41000300            R     -------- -------- xxxxxxxx -xxx----   DIP switches/additional inputs
                        R     -------- -------- xxxxxxxx --------      (debug DIP switches)
                        R     -------- -------- -------- -x------      (service switch)
                        R     -------- -------- -------- --x-----      (sound output buffer full)
                        R     -------- -------- -------- ---x----      (sound input buffer full)
    41000304            R     -------- -------- -------- xxxxxxxx   Coin/service inputs
                        R     -------- -------- -------- xxxx----      (service inputs: R,RC,LC,L)
                        R     -------- -------- -------- ----xxxx      (coin inputs: R,RC,LC,L)
    41000400              W   -------- -------- -------- -xxxxxxx   Palette select
    41000500              W   -------- -------- -------- --------   EEPROM write enable
    41000600              W   -------- -------- -------- ----xxxx   Finescroll, vertical SYNC flags
                          W   -------- -------- -------- ----x---      (VBLANK)
                          W   -------- -------- -------- -----x--      (VSYNC)
                          W   -------- -------- -------- ------xx      (fine scroll value)
    41000700              W   -------- -------- -------- --------   Watchdog reset
    42000000-4201FFFF   R/W   -------- -------- xxxxxxxx xxxxxxxx   Palette RAM
                        R/W   -------- -------- x------- --------      (LSB of all three components)
                        R/W   -------- -------- -xxxxx-- --------      (red component)
                        R/W   -------- -------- ------xx xxx-----      (green component)
                        R/W   -------- -------- -------- ---xxxxx      (blue component)
    43000000              W   -------- -------- ----xxxx xxxxxxxx   HSYNC RAM address latch
                          W   -------- -------- ----x--- --------      (counter enable)
                          W   -------- -------- -----xxx xxxxxxxx      (RAM address)
    43000004            R/W   -------- -------- -------- xxxxx---   HSYNC RAM data latch
                        R/W   -------- -------- -------- x-------      (generate IRQ)
                        R/W   -------- -------- -------- -x------      (VRAM shift enable)
                        R/W   -------- -------- -------- --x-----      (HBLANK)
                        R/W   -------- -------- -------- ---x----      (/HSYNC)
                        R/W   -------- -------- -------- ----x---      (release wait for sync)
    43000008              W   -------- -------- -------- ---x-xx-   HSYNC unknown control
    8DF80000            R     -------- -------- -------- --------   Unknown
    8F380000-8F3FFFFF     W   -------- -------- -------- --------   VRAM latch address
    8F900000-8F97FFFF     W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM transparent write
    8F980000-8F9FFFFF   R/W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM standard read/write
    8FB80000-8FBFFFFF     W   ----xxxx ----xxxx ----xxxx ----xxxx   VRAM "bulk" write
                          W   ----xxxx -------- -------- --------      (enable byte lanes for word 3?)
                          W   -------- ----xxxx -------- --------      (enable byte lanes for word 2?)
                          W   -------- -------- ----xxxx --------      (enable byte lanes for word 1?)
                          W   -------- -------- -------- ----xxxx      (enable byte lanes for word 0?)
    8FFF8000              W   xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx   VRAM "bulk" data latch
    9E280000-9E2FFFFF     W   -------- -------- -------- --------   VRAM copy destination address latch
    ===================================================================================================

***************************************************************************/


#include "emu.h"
#include "includes/beathead.h"
#include "machine/nvram.h"



#define MAX_SCANLINES   262



/*************************************
 *
 *  Machine init
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(beathead_state::scanline_callback)
{
	int scanline = param;

	/* update the video */
//  m_screen->update_now();
	m_screen->update_partial(m_screen->vpos());

	/* on scanline zero, clear any halt condition */
	if (scanline == 0)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);

	/* wrap around at 262 */
	scanline++;
	if (scanline >= MAX_SCANLINES)
		scanline = 0;

	/* set the scanline IRQ */
	m_irq_state[2] = 1;
	update_interrupts();

	/* set the timer for the next one */
	timer.adjust(m_screen->time_until_pos(scanline) - m_hblank_offset, scanline);
}


void beathead_state::machine_reset()
{
	/* reset the common subsystems */
	atarigen_state::machine_reset();

	/* the code is temporarily mapped at 0 at startup */
	/* just copying the first 0x40 bytes is sufficient */
	memcpy(m_ram_base, m_rom_base, 0x40);

	/* compute the timing of the HBLANK interrupt and set the first timer */
	m_hblank_offset = m_screen->scan_period() * (455 - 336 - 25) / 455;
	timer_device *scanline_timer = machine().device<timer_device>("scan_timer");
	scanline_timer->adjust(m_screen->time_until_pos(0) - m_hblank_offset);

	/* reset IRQs */
	m_irq_line_state = CLEAR_LINE;
	m_irq_state[0] = m_irq_state[1] = m_irq_state[2] = 0;
	m_irq_enable[0] = m_irq_enable[1] = m_irq_enable[2] = 0;
}



/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

void beathead_state::update_interrupts()
{
	int gen_int;

	/* compute the combined interrupt signal */
	gen_int  = m_irq_state[0] & m_irq_enable[0];
	gen_int |= m_irq_state[1] & m_irq_enable[1];
	gen_int |= m_irq_state[2] & m_irq_enable[2];
	gen_int  = gen_int ? ASSERT_LINE : CLEAR_LINE;

	/* if it's changed since the last time, call through */
	if (m_irq_line_state != gen_int)
	{
		m_irq_line_state = gen_int;
		//if (m_irq_line_state != CLEAR_LINE)
			m_maincpu->set_input_line(ASAP_IRQ0, m_irq_line_state);
		//else
			//asap_set_irq_line(ASAP_IRQ0, m_irq_line_state);
	}
}


WRITE32_MEMBER( beathead_state::interrupt_control_w )
{
	int irq = offset & 3;
	int control = (offset >> 2) & 1;

	/* offsets 1-3 seem to be the enable latches for the IRQs */
	if (irq != 0)
		m_irq_enable[irq - 1] = control;

	/* offset 0 seems to be the interrupt ack */
	else
		m_irq_state[0] = m_irq_state[1] = m_irq_state[2] = 0;

	/* update the current state */
	update_interrupts();
}


READ32_MEMBER( beathead_state::interrupt_control_r )
{
	/* return the enables as a bitfield */
	return (m_irq_enable[0]) | (m_irq_enable[1] << 1) | (m_irq_enable[2] << 2);
}



/*************************************
 *
 *  EEPROM handling
 *
 *************************************/

WRITE32_MEMBER( beathead_state::eeprom_data_w )
{
	if (m_eeprom_enabled)
	{
		mem_mask &= 0x000000ff;
		COMBINE_DATA(m_nvram + offset);
		m_eeprom_enabled = 0;
	}
}


WRITE32_MEMBER( beathead_state::eeprom_enable_w )
{
	m_eeprom_enabled = 1;
}



/*************************************
 *
 *  Sound communication
 *
 *************************************/

WRITE32_MEMBER( beathead_state::sound_reset_w )
{
	logerror("Sound reset = %d\n", !offset);
	m_jsa->soundcpu().set_input_line(INPUT_LINE_RESET, offset ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Misc other I/O
 *
 *************************************/

WRITE32_MEMBER( beathead_state::coin_count_w )
{
	coin_counter_w(machine(), 0, !offset);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, beathead_state)
	AM_RANGE(0x00000000, 0x0001ffff) AM_RAM AM_SHARE("ram_base")
	AM_RANGE(0x01800000, 0x01bfffff) AM_ROM AM_REGION("user1", 0) AM_SHARE("rom_base")
	AM_RANGE(0x40000000, 0x400007ff) AM_RAM_WRITE(eeprom_data_w) AM_SHARE("nvram")
	AM_RANGE(0x41000000, 0x41000003) AM_DEVREADWRITE8("jsa", atari_jsa_iii_device, main_response_r, main_command_w, 0x000000ff)
	AM_RANGE(0x41000100, 0x41000103) AM_READ(interrupt_control_r)
	AM_RANGE(0x41000100, 0x4100011f) AM_WRITE(interrupt_control_w)
	AM_RANGE(0x41000200, 0x41000203) AM_READ_PORT("IN1")
	AM_RANGE(0x41000204, 0x41000207) AM_READ_PORT("IN0")
	AM_RANGE(0x41000208, 0x4100020f) AM_WRITE(sound_reset_w)
	AM_RANGE(0x41000220, 0x41000227) AM_WRITE(coin_count_w)
	AM_RANGE(0x41000300, 0x41000303) AM_READ_PORT("IN2")
	AM_RANGE(0x41000304, 0x41000307) AM_READ_PORT("IN3")
	AM_RANGE(0x41000400, 0x41000403) AM_WRITEONLY AM_SHARE("palette_select")
	AM_RANGE(0x41000500, 0x41000503) AM_WRITE(eeprom_enable_w)
	AM_RANGE(0x41000600, 0x41000603) AM_WRITE(finescroll_w)
	AM_RANGE(0x41000700, 0x41000703) AM_WRITE(watchdog_reset32_w)
	AM_RANGE(0x42000000, 0x4201ffff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0x43000000, 0x43000007) AM_READWRITE(hsync_ram_r, hsync_ram_w)
	AM_RANGE(0x8df80000, 0x8df80003) AM_READNOP /* noisy x4 during scanline int */
	AM_RANGE(0x8f380000, 0x8f3fffff) AM_WRITE(vram_latch_w)
	AM_RANGE(0x8f900000, 0x8f97ffff) AM_WRITE(vram_transparent_w)
	AM_RANGE(0x8f980000, 0x8f9fffff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x8fb80000, 0x8fbfffff) AM_WRITE(vram_bulk_w)
	AM_RANGE(0x8fff8000, 0x8fff8003) AM_WRITEONLY AM_SHARE("vram_bulk_latch")
	AM_RANGE(0x9e280000, 0x9e2fffff) AM_WRITE(vram_copy_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( beathead )
	PORT_START("IN0")       /* Player 1 */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("IN1")       /* Player 2 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_SOUND_TO_MAIN_READY("jsa")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_ATARI_JSA_MAIN_TO_SOUND_READY("jsa")
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0006, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNUSED )

// to do
//  PORT_MODIFY("jsa:JSAIII")
// coin 1+2 import from JSAIII not used - set to unused
//  PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( beathead, beathead_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ASAP, ATARI_CLOCK_14MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_TIMER_DRIVER_ADD("scan_timer", beathead_state, scanline_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DRIVER(beathead_state, screen_update)
	MCFG_SCREEN_SIZE(42*8, 262)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 42*8-1, 0*8, 30*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 32768)
	MCFG_PALETTE_FORMAT(IRRRRRGGGGGBBBBB)
	MCFG_PALETTE_MEMBITS(16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_ATARI_JSA_III_ADD("jsa", WRITELINE(atarigen_state, sound_int_write_line))
	MCFG_ATARI_JSA_TEST_PORT("IN2", 6)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( beathead )
	ROM_REGION( 0x14000, "jsa:cpu", 0 )         /* 64k + 16k for 6502 code */
	ROM_LOAD( "bhsnd.bin",  0x10000, 0x4000, CRC(dfd33f02) SHA1(479a4838c89691d5a4654a4cd84b6433a9e86109) )
	ROM_CONTINUE(           0x04000, 0xc000 )

	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* 4MB for ASAP code */
	ROM_LOAD32_BYTE( "bhprog0.bin", 0x000000, 0x80000, CRC(87975721) SHA1(862cb3a290c829aedea26ee7100c50a12e9517e7) )
	ROM_LOAD32_BYTE( "bhprog1.bin", 0x000001, 0x80000, CRC(25d89743) SHA1(9ff9a41355aa6914efc4a44909026e648a3c40f3) )
	ROM_LOAD32_BYTE( "bhprog2.bin", 0x000002, 0x80000, CRC(87722609) SHA1(dbd766fa57f4528702a98db28ae48fb5d2a7f7df) )
	ROM_LOAD32_BYTE( "bhprog3.bin", 0x000003, 0x80000, CRC(a795d616) SHA1(d3b201be62486f3b12e1b20c4694eeff0b4e3fca) )
	ROM_LOAD32_BYTE( "bhpics0.bin", 0x200000, 0x80000, CRC(926bf65d) SHA1(49f25a2844ca1cd940d17fc56c0d2698e95e0e1d) )
	ROM_LOAD32_BYTE( "bhpics1.bin", 0x200001, 0x80000, CRC(a8f12e41) SHA1(693cb7a2510f34af5442870a6ae4d19445d991f9) )
	ROM_LOAD32_BYTE( "bhpics2.bin", 0x200002, 0x80000, CRC(00b96481) SHA1(39daa46321c1d4f8bce8c25d0450b97f1f19dedb) )
	ROM_LOAD32_BYTE( "bhpics3.bin", 0x200003, 0x80000, CRC(99c4f1db) SHA1(aba4440c5cdf413f970a0c65457e2d1b37caf2d6) )

	ROM_REGION( 0x80000, "jsa:oki1", 0 )       /* 1MB for ADPCM */
	ROM_LOAD( "bhpcm0.bin",  0x00000, 0x20000, CRC(609ca626) SHA1(9bfc913fc4c3453b132595f8553245376bce3a51) )
	ROM_LOAD( "bhpcm1.bin",  0x20000, 0x20000, CRC(35511509) SHA1(41294b81e253db5d2f30f8589dd59729a31bb2bb) )
	ROM_LOAD( "bhpcm2.bin",  0x40000, 0x20000, CRC(f71a840a) SHA1(09d045552704cd1434307f9a36ce03c5c06a8ff6) )
	ROM_LOAD( "bhpcm3.bin",  0x60000, 0x20000, CRC(fedd4936) SHA1(430ed894fa4bfcd56ee5a8a8ef5e161246530e2d) )
ROM_END



/*************************************
 *
 *  Driver speedups
 *
 *************************************/

/*
    In-game hotspot @ 0180F8D8
*/


READ32_MEMBER( beathead_state::speedup_r )
{
	UINT32 result = *m_speedup_data;
	if ((space.device().safe_pcbase() & 0xfffff) == 0x006f0 && result == space.device().state().state_int(ASAP_R3))
		space.device().execute().spin_until_interrupt();
	return result;
}


READ32_MEMBER( beathead_state::movie_speedup_r )
{
	int result = *m_movie_speedup_data;
	if ((space.device().safe_pcbase() & 0xfffff) == 0x00a88 && (space.device().state().state_int(ASAP_R28) & 0xfffff) == 0x397c0 &&
		m_movie_speedup_data[4] == space.device().state().state_int(ASAP_R1))
	{
		UINT32 temp = (INT16)result + m_movie_speedup_data[4] * 262;
		if (temp - (UINT32)space.device().state().state_int(ASAP_R15) < (UINT32)space.device().state().state_int(ASAP_R23))
			space.device().execute().spin_until_interrupt();
	}
	return result;
}



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(beathead_state,beathead)
{
	/* prepare the speedups */
	m_speedup_data = m_maincpu->space(AS_PROGRAM).install_read_handler(0x00000ae8, 0x00000aeb, 0, 0, read32_delegate(FUNC(beathead_state::speedup_r), this));
	m_movie_speedup_data = m_maincpu->space(AS_PROGRAM).install_read_handler(0x00000804, 0x00000807, 0, 0, read32_delegate(FUNC(beathead_state::movie_speedup_r), this));
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, beathead, 0, beathead, beathead, beathead_state, beathead, ROT0, "Atari Games", "BeatHead (prototype)", 0 )
