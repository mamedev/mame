// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/******************************************************************************

        LLC driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

        July 2012, updates by Robbbert

        Very little info available on these computers.

        LLC1:
        Handy addresses (set the pc register in the debugger, because the
        monitor's Go command has problems):
        0800 = BASIC cold start
        0803 = BASIC warm start
        13BE = display Monitor logo
        This machine has an 8-digit LED display with hex keyboard,
        and also a 64x16 monochrome screen with full keyboard.
        The monitor uses the hex keyboard, while Basic uses the full keyboard.
        Monitor output is on the digits, but the single-step command displays
        a running register dump on the main screen.
        There are no storage facilities, and no sound.
        BASIC is integer only (-32768 to 32767), about 6k of space, and all
        input is to be in uppercase. It does have minimal string capability,
        but there is no $ sign, so how to create a string variable is unknown.
        To exit back to the monitor, type BYE.
        The user instructions of the monitor (in German) are here:
        http://www.jens-mueller.org/jkcemu/llc1.html

        LLC2:
        The BEL character plays a short tune.
        In the monitor, it is case sensitive, most commands are uppercase,
        but some are lowercase.
        To start Basic, the command is b. To quit basic, use BYE.
        Inside Basic, it is not case-sensitive.

        ToDo:
        - LLC1: Get good dump of monitor rom, has a number of bad bytes
        - LLC1: In Basic, pressing enter several times causes the start
          of the line to be shifted 1 or 2 characters to the right.
        - LLC1: Go command crashes
        - LLC2: Keyboard is incomplete
        - Lots of other things

*******************************************************************************/


#include "machine/keyboard.h"
#include "includes/llc.h"
#include "llc1.lh"

#define KEYBOARD_TAG "keyboard"

/* Address maps */
static ADDRESS_MAP_START( llc1_mem, AS_PROGRAM, 8, llc_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM // Monitor ROM
	AM_RANGE(0x0800, 0x13ff) AM_ROM // BASIC ROM
	AM_RANGE(0x1400, 0x1bff) AM_RAM // RAM
	AM_RANGE(0x1c00, 0x1fff) AM_RAM AM_SHARE("videoram") // Video RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( llc1_io, AS_IO, 8, llc_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xEC, 0xEF) AM_DEVREADWRITE("z80pio2", z80pio_device, read, write)
	AM_RANGE(0xF4, 0xF7) AM_DEVREADWRITE("z80pio1", z80pio_device, read, write)
	AM_RANGE(0xF8, 0xFB) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( llc2_mem, AS_PROGRAM, 8, llc_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAMBANK("bank1")
	AM_RANGE(0x4000, 0x5fff) AM_RAMBANK("bank2")
	AM_RANGE(0x6000, 0xbfff) AM_RAMBANK("bank3")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( llc2_io, AS_IO, 8, llc_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xE0, 0xE3) AM_WRITE(llc2_rom_disable_w)
	AM_RANGE(0xE4, 0xE7) AM_DEVREADWRITE("z80pio2", z80pio_device, read, write)
	AM_RANGE(0xE8, 0xEB) AM_DEVREADWRITE("z80pio1", z80pio_device, read, write)
	AM_RANGE(0xEC, 0xEC) AM_WRITE(llc2_basic_enable_w)
	AM_RANGE(0xF8, 0xFB) AM_DEVREADWRITE("z80ctc", z80ctc_device, read, write)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( llc1 )
	PORT_START("X4") // out F4,BF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_START("X5") // out F4,DF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M (Mem)") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ST (Start)") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('^')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) // resets
	PORT_START("X6") // out F4,EF
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ES (Step)") PORT_CODE(KEYCODE_S) PORT_CHAR('S')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DL (Go)") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HP (BP)") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) // does nothing
INPUT_PORTS_END

static INPUT_PORTS_START( llc2 )
INPUT_PORTS_END


WRITE8_MEMBER( llc_state::kbd_put )
{
	static UINT8 s1[16]={0x40, 0x1e, 0x12, 0x1b, 0x19, 0x14, 0x15, 0x1d, 0x16, 0x17, 0x1c, 0x3c, 0x3f, 0x3d, 0x3e, 0x10}; // 0x20 to 0x2F
	static UINT8 s2[7] ={0x1a, 0x11, 0x7c, 0x13, 0x7b, 0x1f, 0x00}; // 0x3A to 0x40
	static UINT8 s3[6] ={0x5c, 0x00, 0x5b, 0x7e, 0x00, 0x5e}; // 0x5B to 0x60

	m_term_data = data;

	if ((data >= 0x20) && (data <= 0x2f))
		m_term_data = s1[data-0x20];
	else
	if ((data >= 0x3a) && (data <= 0x40))
		m_term_data = s2[data-0x3a];
	else
	if ((data >= 0x5b) && (data <= 0x60))
		m_term_data = s3[data-0x5b];
	else
	if (data >= 0x7b)
		m_term_data = 0;

	if (m_term_data)
		m_term_status = 0xff;
}

static const z80_daisy_config llc1_daisy_chain[] =
{
	{ "z80ctc" },
	{ nullptr }
};

static const z80_daisy_config llc2_daisy_chain[] =
{
	{ "z80pio1" },
	{ "z80ctc" },
	{ nullptr }
};

/* F4 Character Displayer */
static const gfx_layout llc1_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0, 1*128*8, 2*128*8, 3*128*8, 4*128*8, 5*128*8, 6*128*8, 7*128*8 },
	8                   /* every char takes 8 x 1 bytes */
};

static const gfx_layout llc2_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( llc1 )
	GFXDECODE_ENTRY( "chargen", 0x0000, llc1_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( llc2 )
	GFXDECODE_ENTRY( "chargen", 0x0000, llc2_charlayout, 0, 1 )
GFXDECODE_END

/* Machine driver */
static MACHINE_CONFIG_START( llc1, llc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3MHz)
	MCFG_CPU_CONFIG(llc1_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(llc1_mem)
	MCFG_CPU_IO_MAP(llc1_io)

	MCFG_MACHINE_START_OVERRIDE(llc_state, llc1 )
	MCFG_MACHINE_RESET_OVERRIDE(llc_state, llc1 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(64*8, 16*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 16*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(llc_state, screen_update_llc1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", llc1)
	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_DEFAULT_LAYOUT(layout_llc1)

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL_3MHz)
	MCFG_Z80PIO_IN_PA_CB(READ8(llc_state, llc1_port1_a_r))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(llc_state, llc1_port1_a_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(llc_state, llc1_port1_b_w))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL_3MHz)
	MCFG_Z80PIO_IN_PA_CB(READ8(llc_state, llc1_port2_a_r))
	MCFG_Z80PIO_IN_PB_CB(READ8(llc_state, llc1_port2_b_r))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_3MHz)
	// timer 0 irq does digit display, and timer 3 irq does scan of the
	// monitor keyboard.
	// No idea how the CTC is connected, so guessed.
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC0_CB(DEVWRITELINE("z80ctc", z80ctc_device, trg1))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("z80ctc", z80ctc_device, trg3))

	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(llc_state, kbd_put))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( llc2, llc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_3MHz)
	MCFG_CPU_CONFIG(llc2_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(llc2_mem)
	MCFG_CPU_IO_MAP(llc2_io)

	MCFG_MACHINE_RESET_OVERRIDE(llc_state, llc2 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(llc_state, screen_update_llc2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", llc2)
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL_3MHz)
	MCFG_Z80PIO_IN_PA_CB(DEVREAD8(K7659_KEYBOARD_TAG, k7659_keyboard_device, read))
	MCFG_Z80PIO_IN_PB_CB(READ8(llc_state, llc2_port1_b_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(llc_state, llc2_port1_b_w))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL_3MHz)
	MCFG_Z80PIO_IN_PA_CB(READ8(llc_state, llc2_port2_a_r))

	MCFG_DEVICE_ADD("z80ctc", Z80CTC, XTAL_3MHz)

	MCFG_K7659_KEYBOARD_ADD()

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END
/* ROM definition */

ROM_START( llc1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	//ROM_LOAD( "llc1-monitor.rom", 0x0000, 0x0800, BAD_DUMP CRC(0e81378d) SHA1(0fbb6eca016d0f439ea1c9aa0cb0affb5f49ea69) )
	ROM_LOAD( "llc1mon.bin", 0x0000, 0x0800, BAD_DUMP CRC(e291dd63) SHA1(31a71bef84f7c164a270d0895cb645e078e9c6f2) )
	ROM_LOAD( "llc1_tb1.bin", 0x0800, 0x0400, CRC(0d9d4039) SHA1(b515e385af57f4faf3a9f7b4a1edd59a1c1ea260) )
	ROM_LOAD( "llc1_tb2.bin", 0x0c00, 0x0400, CRC(28bfea2a) SHA1(a68a8b87bfc931627ddd8d124b153e511477fbaf) )
	ROM_LOAD( "llc1_tb3.bin", 0x1000, 0x0400, CRC(fe5e3132) SHA1(cc3b191e41f5772a4b86b8eb0ebe6fce67872df6) )
	ROM_FILL(0x23b, 1, 0x00) // don't reboot when typing into the monitor
	ROM_FILL(0x2dc, 1, 0x0f) // fix display of AF in the reg command
	ROM_REGION(0x0400, "chargen",0)
	ROM_LOAD ("llc1_zg.bin", 0x0000, 0x0400, CRC(fa2cd659) SHA1(1fa5f9992f35929f656c4ce55ed6980c5da1772b) )
ROM_END

ROM_START( llc2 )
	ROM_REGION( 0x12000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "scchmon_91.bin", 0x0000, 0x1000, CRC(218d8236) SHA1(b8297272cc79751afc2eb8688d99b40691346dcb) )
	ROM_LOAD( "gsbasic.bin", 0x10000, 0x2000, CRC(78a5f388) SHA1(e7b475b98dce36b24540ad11eb89046ddb4f02af) )
	ROM_REGION(0x0800, "chargen",0)
	ROM_LOAD ("llc2font.bin", 0x0000, 0x0800, CRC(ce53e55d) SHA1(da23d93f14a8a1f8d82bb72470a96b0bfd81ed1b) )
ROM_END


/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT       INIT     COMPANY    FULLNAME       FLAGS */
COMP( 1984, llc1,   0,      0,      llc1,       llc1, llc_state,       llc1,    "SCCH",    "LLC-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1984, llc2,   llc1,   0,      llc2,       llc2, llc_state,       llc2,    "SCCH",    "LLC-2", 0 )
