/****************************************************************************

    Bally Astrocade consumer hardware
    driver by Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles,
    Dirk Best

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/astrocde.h"
#include "sound/astrocde.h"
#include "imagedev/cartslot.h"
#include "machine/ram.h"

class astrocde_mess_state : public astrocde_state
{
public:
	astrocde_mess_state(const machine_config &mconfig, device_type type, const char *tag)
		: astrocde_state(mconfig, type, tag)
		{ }

	void get_ram_expansion_settings(int &ram_expansion_installed, int &write_protect_on, int &expansion_ram_start, int &expansion_ram_end, int &shadow_ram_end);
	DECLARE_MACHINE_RESET(astrocde);
	DECLARE_INPUT_CHANGED_MEMBER(set_write_protect);
};


/*************************************
 *
 *  Memory maps
 *
 * $0000 to $1FFF:  8K on-board ROM (could be one of three available BIOS dumps)
 * $2000 to $3FFF:  8K cartridge ROM
 * $4000 to $4FFF:  4K screen RAM
 * $5000 to $FFFF:  44K address space not available in standard machine.  With a
 * sufficiently large RAM expansion, all of this RAM can be added, and accessed
 * by an extended BASIC program.  Bally and Astrocade BASIC can access from
 * $5000 to $7FFF if available.
 *
 *  RAM Expansions
 *
 * Several third party RAM expansions have been made for the Astrocade.  These
 * allow access to various ranges of the expansion memory ($5000 to $FFFF).
 * A RAM expansion is required to use extended BASIC programs like Blue RAM BASIC
 * and VIPERSoft BASIC.  All of the expansions also have a RAM protect switch, which
 * can be flipped at any time to make the RAM act like ROM.  Extended BASIC
 * programs need access to the RAM and won't work with RAM protect enabled, but
 * this can be useful with Bally and Astrocade BASIC.  They also have a range switch
 * (not implemented).  The default position is 6K, but it can be switched to
 * 2K.  This means that the expanded memory starting at $6000 will instead be
 * mapped to the cartridge memory starting at $2000.  So it would be possible to
 * load a cartridge program from tape into the expansion memory, then flip the range
 * switch and run it as a cartridge.  This is useful for cartridge development.
 *
 * NOTE:  If you have any trouble running cartridges with a RAM expansion installed, hit reset.
 *
 * Blue RAM -- available in 4K, 16K, and 32K.  These also use an INS8154 chip,
 * (not yet implemented) which has an additional $80 bytes of RAM mapped
 * immediately after the end of the expansion address space.  This memory
 * can't be write protected.  The INS8154 has I/O features needed for loading
 * tape programs into Blue RAM BASIC, as well as running the Blue RAM Utility cart.
 * 4K:  $6000 to $6FFF (can't run VIPERSoft BASIC, because this program needs memory
 * past this range)
 * 16K:  $6000 to $9FFF
 * 32K:  $6000 to $DFFF
 *
 * VIPER System 1 -- This is available in 16K only.  It also includes a keyboard (not implemented).
 * 16K:  $6000 to $9FFF
 *
 * Lil' WHITE RAM -- This is available in 32K only.  Attempts to read and write
 * to memory outside of its address range ($D000 to $FFFF) are mapped to the expansion
 * memory $5000 to $7FFF.  The current implementation won't allow the shadow RAM area
 * to be accessed when RAM protect is on, but there is no known software that will
 * access the upper range of the expansion RAM when RAM protect is enabled.
 * 32K:  $5000 to $CFFF
 *
 * R&L 64K RAM Board -- This is a highly configurable kit.  RAM can be installed in
 * 2K increments.  So, the entire 44K expansion memory can be filled.  It is also
 * possible to override the rest of the memory map with RAM (not implemented).
 * There are 32 switches allowing users to activate and deactivate each 2K block (not implemented).
 * RAM write protection can be implemented in three ranges through jumpers or by
 * installing switches.  The ranges are $0000 to $0FFF (first 4K), $0000 to $3FFF (first 16K),
 * and $0000 to $FFFF (all 64K).  The current implementation is for 44K expansion memory mapped from
 * $5000 to $FFFF, with only a single write protect covering this entire range.
 *
 *************************************/

static ADDRESS_MAP_START( astrocade_mem, AS_PROGRAM, 8, astrocde_mess_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_WRITE(astrocade_funcgen_w)
	AM_RANGE(0x1000, 0x3fff) AM_ROM /* Star Fortress writes in here?? */
	AM_RANGE(0x4000, 0x4fff) AM_RAM AM_SHARE("videoram") /* ASG */
ADDRESS_MAP_END


static ADDRESS_MAP_START( astrocade_io, AS_IO, 8, astrocde_mess_state )
	AM_RANGE(0x00, 0x1f) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(astrocade_data_chip_register_r, astrocade_data_chip_register_w)
ADDRESS_MAP_END

INPUT_CHANGED_MEMBER(astrocde_mess_state::set_write_protect)  // run when RAM expansion write protect switch is changed
{
	int ram_expansion_installed = 0, write_protect_on = 0, expansion_ram_start = 0, expansion_ram_end = 0, shadow_ram_end = 0;
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *expram = machine().device<ram_device>("ram_tag")->pointer();

	get_ram_expansion_settings(ram_expansion_installed, write_protect_on, expansion_ram_start, expansion_ram_end, shadow_ram_end);  // passing by reference

	if (ram_expansion_installed == 1)
	{
		if (write_protect_on == 0)  // write protect off, so install memory normally
		{
			space.install_ram(expansion_ram_start, expansion_ram_end, expram);
			if (shadow_ram_end > expansion_ram_end)
				space.install_ram(expansion_ram_end + 1, shadow_ram_end, expram);
		}
		else  // write protect on, so make memory read only
		{
			space.nop_write(expansion_ram_start, expansion_ram_end);
		}
		}
}

/*************************************
 *
 *  Input ports
 *
 *
 *  The Astrocade has ports for four hand controllers.  Each controller has a
 *  knob on top that can be simultaneously pushed as an eight-way joystick and
 *  twisted as a paddle, in addition to a trigger button.  The knob can twist
 *  through about 270 degrees, registering 256 unique positions.  It does not
 *  autocenter.  When selecting options on the menu, twisting the knob to the
 *  right gives lower numbers, and twisting to the left gives larger numbers.
 *  Paddle games like Clowns have more intuitive behavior -- twisting to the
 *  right moves the character right.
 *
 *  There is a 24-key keypad on the system itself (6 rows, 4 columns).  It is
 *  labeled for the built-in calculator, but overlays were released for other
 *  programs, the most popular being the BASIC cartridges, which allowed a
 *  large number of inputs by making the bottom row shift buttons.  The labels
 *  below first list the calculator key, then the BASIC keys in the order of no
 *  shift, GREEN shift, RED shift, BLUE shift, WORDS shift.
 *
 *************************************/

static INPUT_PORTS_START( astrocde )
	PORT_START("P1HANDLE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(1)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P2HANDLE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(2)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P3HANDLE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(3) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(3) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(3) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(3)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P4HANDLE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)    PORT_PLAYER(4) PORT_8WAY
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)  PORT_PLAYER(4) PORT_8WAY
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(4) PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)        PORT_PLAYER(4)
	PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYPAD0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("%   \xC3\xB7         [   ]   LIST") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("/   x     J   K   L   NEXT") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("x   -     V   W   X   IF") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("-   +     &   @   *   GOTO") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("+   =     #   %   :   PRINT") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("=   WORDS Shift") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEYPAD1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("\xE2\x86\x93   HALT              RUN") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CH  9     G   H   I   STEP") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9   6     S   T   U   RND") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6   3     \xE2\x86\x91   .   \xE2\x86\x93   BOX") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3   ERASE (   ;   )") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(".   BLUE Shift") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYPAD2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("\xE2\x86\x91   PAUSE     /   \\") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MS  8     D   E   F   TO") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8   5     P   Q   R   RETN") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5   2     \xE2\x86\x90   '   \xE2\x86\x92   LINE") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2   0     <   \"   >   INPUT") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("0   RED Shift") PORT_CODE(KEYCODE_0)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYPAD3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C   GO                +10") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MR  7     A   B   C   FOR") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("7   4     M   N   O   GOSB") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4   1     Y   Z   !   CLEAR") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1   SPACE $   ,   ?") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CE  GREEN Shift") PORT_CODE(KEYCODE_E)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("P1_KNOB")
	PORT_BIT(0xff, 0x00, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(85) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_PLAYER(1)

	PORT_START("P2_KNOB")
	PORT_BIT(0xff, 0x00, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(85) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2)

	PORT_START("P3_KNOB")
	PORT_BIT(0xff, 0x00, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(85) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_CODE_DEC(KEYCODE_Q) PORT_CODE_INC(KEYCODE_W) PORT_PLAYER(3)

	PORT_START("P4_KNOB")
	PORT_BIT(0xff, 0x00, IPT_PADDLE) PORT_INVERT PORT_SENSITIVITY(85) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_MINMAX(0,255) PORT_CODE_DEC(KEYCODE_Y) PORT_CODE_INC(KEYCODE_U) PORT_PLAYER(4)

	PORT_START("CFG")   /* machine config */
	PORT_DIPNAME( 0x3f, 0x00, "RAM Expansion")
	PORT_DIPSETTING(    0x00, "No RAM Expansion")
	PORT_DIPSETTING(    0x01, "16KB Viper System 1 RAM Expansion")
	PORT_DIPSETTING(    0x02, "32KB Lil' WHITE RAM Expansion")
	PORT_DIPSETTING(    0x04, "R&L 64K RAM Board (44K installed)")
	PORT_DIPSETTING(    0x08, "4KB Blue RAM Expansion")
	PORT_DIPSETTING(    0x10, "16KB Blue RAM Expansion")
	PORT_DIPSETTING(    0x20, "32KB Blue RAM Expansion")

	PORT_START("PROTECT")  /* Write protect RAM */
	PORT_DIPNAME( 0x01, 0x00, "Write Protect RAM") PORT_CHANGED_MEMBER(DEVICE_SELF, astrocde_mess_state, set_write_protect, 0)
	PORT_DIPSETTING( 0x00, "Write Protect Off")
	PORT_DIPSETTING( 0x01, "Write Protect On")
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( astrocde, astrocde_mess_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, ASTROCADE_CLOCK/4)        /* 1.789 MHz */
	MCFG_CPU_PROGRAM_MAP(astrocade_mem)
	MCFG_CPU_IO_MAP(astrocade_io)

	MCFG_MACHINE_RESET_OVERRIDE(astrocde_mess_state, astrocde)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(ASTROCADE_CLOCK, 455, 0, 352, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(astrocde_state, screen_update_astrocde)

	MCFG_PALETTE_LENGTH(512)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("astrocade1", ASTROCADE, ASTROCADE_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* optional expansion ram (installed in MACHINE_RESET)*/
	MCFG_RAM_ADD("ram_tag")
	MCFG_RAM_DEFAULT_SIZE("32k")

	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_INTERFACE("astrocde_cart")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","astrocde")
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( astrocde )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astro.bin",  0x0000, 0x2000, CRC(ebc77f3a) SHA1(b902c941997c9d150a560435bf517c6a28137ecc) )
	ROM_CART_LOAD( "cart", 0x2000, 0x8000, 0 )
ROM_END

ROM_START( astrocdl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ballyhlc.bin",  0x0000, 0x2000, CRC(d7c517ba) SHA1(6b2bef5d970e54ed204549f58ba6d197a8bfd3cc) )
	ROM_CART_LOAD( "cart", 0x2000, 0x8000, 0 )
ROM_END

ROM_START( astrocdw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bioswhit.bin",  0x0000, 0x2000, CRC(6eb53e79) SHA1(d84341feec1a0a0e8aa6151b649bc3cf6ef69fbf) )
	ROM_CART_LOAD( "cart", 0x2000, 0x8000, 0 )
ROM_END

/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(astrocde_state,astrocde)
{
	m_video_config = AC_SOUND_PRESENT | AC_LIGHTPEN_INTS;
}

MACHINE_RESET_MEMBER(astrocde_mess_state, astrocde)
{
	int ram_expansion_installed = 0, write_protect_on = 0, expansion_ram_start = 0, expansion_ram_end = 0, shadow_ram_end = 0;
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *expram = machine().device<ram_device>("ram_tag")->pointer();
	space.unmap_readwrite(0x5000, 0xffff);  // unmap any previously installed expansion RAM

	get_ram_expansion_settings(ram_expansion_installed, write_protect_on, expansion_ram_start, expansion_ram_end, shadow_ram_end);  // passing by reference

	if (ram_expansion_installed == 1)
	{
		if (write_protect_on == 0)  // write protect off, so install memory normally
		{
			space.install_ram(expansion_ram_start, expansion_ram_end, expram);
			if (shadow_ram_end > expansion_ram_end)
				space.install_ram(expansion_ram_end + 1, shadow_ram_end, expram);
		}
		else  // write protect on, so make memory read only
		{
			space.nop_write(expansion_ram_start, expansion_ram_end);
		}
		}
}

void astrocde_mess_state::get_ram_expansion_settings(int &ram_expansion_installed, int &write_protect_on, int &expansion_ram_start, int &expansion_ram_end, int &shadow_ram_end)
{
	if (machine().root_device().ioport("PROTECT")->read() == 0x01)
		write_protect_on = 1;
	else
		write_protect_on = 0;

	ram_expansion_installed = 1;

	switch(machine().root_device().ioport("CFG")->read())  // check RAM expansion configuration and set address ranges
	{
		case 0x00:  // No RAM Expansion
				ram_expansion_installed = 0;
				break;
		case 0x01:  // 16KB Viper System 1 RAM Expansion
				expansion_ram_start = 0x6000;
				expansion_ram_end = 0x9fff;
				shadow_ram_end = 0;
				break;
		case 0x02:  // "32KB Lil' WHITE RAM Expansion
				expansion_ram_start = 0x5000;
				expansion_ram_end = 0xcfff;
				shadow_ram_end = 0xffff;
				break;
		case 0x04:  // R&L 64K RAM Board (44KB installed)
				expansion_ram_start = 0x5000;
				expansion_ram_end = 0xffff;
				shadow_ram_end = 0;
				break;
		case 0x08:  // 4KB Blue RAM Expansion
				expansion_ram_start = 0x6000;
				expansion_ram_end = 0x6fff;
				shadow_ram_end = 0;
				break;
		case 0x10:  // 16KB Blue RAM Expansion
				expansion_ram_start = 0x6000;
				expansion_ram_end = 0x9fff;
				shadow_ram_end = 0;
				break;
		case 0x20:  // 32KB Blue RAM Expansion
				expansion_ram_start = 0x6000;
				expansion_ram_end = 0xdfff;
				shadow_ram_end = 0;
				break;
		default:
			break;
	}
}


/*************************************
 *
 *  Driver definitions
 *
 *************************************/

/*    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT      COMPANY                FULLNAME                     FLAGS */
CONS( 1978, astrocde, 0,        0,        astrocde, astrocde, astrocde_state, astrocde, "Bally Manufacturing", "Bally Professional Arcade", GAME_SUPPORTS_SAVE )
CONS( 1977, astrocdl, astrocde, 0,        astrocde, astrocde, astrocde_state, astrocde, "Bally Manufacturing", "Bally Home Library Computer", GAME_SUPPORTS_SAVE )
CONS( 1977, astrocdw, astrocde, 0,        astrocde, astrocde, astrocde_state, astrocde, "Bally Manufacturing", "Bally Computer System", GAME_SUPPORTS_SAVE )
