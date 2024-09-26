// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Mike Coates, Frank Palazzolo, Aaron Giles, Dirk Best
/****************************************************************************

    Bally Astrocade consumer hardware

****************************************************************************/

#include "emu.h"
#include "astrocde.h"

#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "sound/astrocde.h"

#include "bus/astrocde/slot.h"
#include "bus/astrocde/rom.h"
#include "bus/astrocde/exp.h"
#include "bus/astrocde/ram.h"
#include "bus/astrocde/ctrl.h"
#include "bus/astrocde/accessory.h"

#include "softlist_dev.h"
#include "speaker.h"


namespace {

class astrocde_home_state : public astrocde_state
{
public:
	astrocde_home_state(const machine_config &mconfig, device_type type, const char *tag)
		: astrocde_state(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_exp(*this, "exp")
		, m_ctrl(*this, "ctrl%u", 1U)
		, m_accessory(*this, "accessory")
		, m_keypad(*this, "KEYPAD%u", 0U)
	{ }

	void astrocde(machine_config &config);

	void init_astrocde();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t inputs_r(offs_t offset);

	void astrocade_io(address_map &map) ATTR_COLD;
	void astrocade_mem(address_map &map) ATTR_COLD;

	required_device<astrocade_cart_slot_device> m_cart;
	required_device<astrocade_exp_device> m_exp;
	required_device_array<astrocade_ctrl_port_device, 4> m_ctrl;
	required_device<astrocade_accessory_port_device> m_accessory;
	required_ioport_array<4> m_keypad;
};


/*********************************************************************************
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
 *********************************************************************************/

void astrocde_home_state::astrocade_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().w(FUNC(astrocde_home_state::astrocade_funcgen_w));
	map(0x1000, 0x3fff).rom(); /* Star Fortress writes in here?? */
	map(0x4000, 0x4fff).ram().share("videoram"); /* ASG */
	//map(0x5000, 0xffff).rw("exp", FUNC(astrocade_exp_device::read), FUNC(astrocade_exp_device::write));
}


void astrocde_home_state::astrocade_io(address_map &map)
{
	map(0x00, 0x0f).select(0xff00).rw(FUNC(astrocde_state::video_register_r), FUNC(astrocde_state::video_register_w));
	map(0x10, 0x1f).select(0xff00).r(m_astrocade_sound[0], FUNC(astrocade_io_device::read));
	map(0x10, 0x18).select(0xff00).w(m_astrocade_sound[0], FUNC(astrocade_io_device::write));
	map(0x19, 0x19).mirror(0xff00).w(FUNC(astrocde_state::expand_register_w));
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

uint8_t astrocde_home_state::inputs_r(offs_t offset)
{
	if (BIT(offset, 2))
		return m_keypad[offset & 3]->read();
	else
		return m_ctrl[offset & 3]->read_handle();
}

static INPUT_PORTS_START( astrocde )
	PORT_START("KEYPAD0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(u8"%   ÷         [   ]   LIST") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("/   x     J   K   L   NEXT") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("x   -     V   W   X   IF") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("-   +     &   @   *   GOTO") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("+   =     #   %   :   PRINT") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("=   WORDS Shift") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("KEYPAD1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(u8"\u2193   HALT              RUN") PORT_CODE(KEYCODE_PGDN) // U+2193 = ↓
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CH  9     G   H   I   STEP") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("9   6     S   T   U   RND") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(u8"6   3     \u2191   .   \u2193   BOX") PORT_CODE(KEYCODE_6) // U+2191 = ↑, U+2193 = ↓
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3   ERASE (   ;   )") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(".   BLUE Shift") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEYPAD2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(u8"\u2191   PAUSE     /   \\") PORT_CODE(KEYCODE_PGUP) // U+2191 = ↑
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MS  8     D   E   F   TO") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("8   5     P   Q   R   RETN") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME(u8"5   2     \u2190   '   \u2192   LINE") PORT_CODE(KEYCODE_5) // U+2190 = ←,  U+2192 = →
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
INPUT_PORTS_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static void astrocade_cart(device_slot_interface &device)
{
	device.option_add_internal("rom",       ASTROCADE_ROM_STD);
	device.option_add_internal("rom_256k",  ASTROCADE_ROM_256K);
	device.option_add_internal("rom_512k",  ASTROCADE_ROM_512K);
	device.option_add_internal("rom_cass",  ASTROCADE_ROM_CASS);
}

static void astrocade_exp(device_slot_interface &device)
{
	device.option_add("blue_ram_4k",   ASTROCADE_BLUERAM_4K);
	device.option_add("blue_ram_16k",  ASTROCADE_BLUERAM_16K);
	device.option_add("blue_ram_32k",  ASTROCADE_BLUERAM_32K);
	device.option_add("viper_sys1",    ASTROCADE_VIPER_SYS1);
	device.option_add("lil_white_ram", ASTROCADE_WHITERAM);
	device.option_add("rl64_ram",      ASTROCADE_RL64RAM);
}


void astrocde_home_state::astrocde(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, ASTROCADE_CLOCK/4); /* 1.789 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &astrocde_home_state::astrocade_mem);
	m_maincpu->set_addrmap(AS_IO, &astrocde_home_state::astrocade_io);

	config.set_perfect_quantum(m_maincpu);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(ASTROCADE_CLOCK, 455, 0, 352, 262, 0, 240);
	m_screen->set_screen_update(FUNC(astrocde_state::screen_update_astrocde));
	m_screen->set_palette(m_palette);

	PALETTE(config, "palette", FUNC(astrocde_home_state::astrocade_palette), 512);

	/* control ports */
	for (uint32_t port = 0; port < 4; port++)
	{
		ASTROCADE_CTRL_PORT(config, m_ctrl[port], astrocade_controllers, port == 0 ? "joy" : nullptr);
		m_ctrl[port]->ltpen_handler().set(FUNC(astrocde_home_state::lightpen_trigger_w));
	}

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ASTROCADE_IO(config, m_astrocade_sound[0], ASTROCADE_CLOCK/4);
	m_astrocade_sound[0]->si_cb().set(FUNC(astrocde_home_state::inputs_r));
	m_astrocade_sound[0]->pot_cb<0>().set(m_ctrl[0], FUNC(astrocade_ctrl_port_device::read_knob));
	m_astrocade_sound[0]->pot_cb<1>().set(m_ctrl[1], FUNC(astrocade_ctrl_port_device::read_knob));
	m_astrocade_sound[0]->pot_cb<2>().set(m_ctrl[2], FUNC(astrocade_ctrl_port_device::read_knob));
	m_astrocade_sound[0]->pot_cb<3>().set(m_ctrl[3], FUNC(astrocade_ctrl_port_device::read_knob));
	m_astrocade_sound[0]->add_route(ALL_OUTPUTS, "mono", 1.0);

	/* expansion port */
	ASTROCADE_EXP_SLOT(config, m_exp, astrocade_exp, nullptr);

	/* cartridge */
	ASTROCADE_CART_SLOT(config, m_cart, astrocade_cart, nullptr);

	/* cartridge */
	ASTROCADE_ACCESSORY_PORT(config, m_accessory, m_screen, astrocade_accessories, nullptr);
	m_accessory->ltpen_handler().set(FUNC(astrocde_home_state::lightpen_trigger_w));

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("astrocde");
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( astrocde )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "astro.bin",  0x0000, 0x2000, CRC(ebc77f3a) SHA1(b902c941997c9d150a560435bf517c6a28137ecc) )
ROM_END

ROM_START( astrocdl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ballyhlc.bin",  0x0000, 0x2000, CRC(d7c517ba) SHA1(6b2bef5d970e54ed204549f58ba6d197a8bfd3cc) )
ROM_END

ROM_START( astrocdw )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bioswhit.bin",  0x0000, 0x2000, CRC(6eb53e79) SHA1(d84341feec1a0a0e8aa6151b649bc3cf6ef69fbf) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void astrocde_home_state::init_astrocde()
{
	m_video_config = AC_SOUND_PRESENT;
}

void astrocde_home_state::machine_start()
{
	if (m_cart->exists())
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x2000, 0x3fff, read8sm_delegate(*m_cart, FUNC(astrocade_cart_slot_device::read_rom)));

	// if no RAM is mounted and the handlers are installed, the system starts with garbage on screen and a RESET is necessary
	// thus, install RAM only if an expansion is mounted
	if (m_exp->get_card_mounted())
	{
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x5000, 0xffff, read8sm_delegate(*m_exp, FUNC(astrocade_exp_device::read)), write8sm_delegate(*m_exp, FUNC(astrocade_exp_device::write)));
		m_maincpu->space(AS_IO).install_readwrite_handler(0x0080, 0x00ff, 0x0000, 0x0000, 0xff00, read8sm_delegate(*m_exp, FUNC(astrocade_exp_device::read_io)), write8sm_delegate(*m_exp, FUNC(astrocade_exp_device::write_io)));
	}
}

} // Anonymous namespace


/*************************************
 *
 *  Driver definitions
 *
 *************************************/

/*    YEAR  NAME      PARENT    COMPAT  MACHINE   INPUT     CLASS                INIT           COMPANY                FULLNAME                       FLAGS */
CONS( 1978, astrocde, 0,        0,      astrocde, astrocde, astrocde_home_state, init_astrocde, "Bally Manufacturing", "Bally Professional Arcade",   MACHINE_SUPPORTS_SAVE )
CONS( 1977, astrocdl, astrocde, 0,      astrocde, astrocde, astrocde_home_state, init_astrocde, "Bally Manufacturing", "Bally Home Library Computer", MACHINE_SUPPORTS_SAVE )
CONS( 1977, astrocdw, astrocde, 0,      astrocde, astrocde, astrocde_home_state, init_astrocde, "Bally Manufacturing", "Bally Computer System",       MACHINE_SUPPORTS_SAVE )
