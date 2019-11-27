// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Wilbert Pol
/***************************************************************************

  Atari VCS 2600 driver

TODO:
- Move the 2 32-in-1 rom dumps into their own driver
- Add 128-in-1 driver

***************************************************************************/

// the new RIOT does not work with the SuperCharger
// for example "mame64 a2600 scharger -cass offifrog" fails to load after playing the tape

#include "emu.h"

#include "includes/a2600.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


#define MASTER_CLOCK_NTSC   3579545
#define MASTER_CLOCK_PAL    3546894
#define CATEGORY_SELECT     16

static const uint16_t supported_screen_heights[4] = { 262, 312, 328, 342 };


void a2600_base_state::a2600_mem(address_map &map) // 6507 has 13-bit address space, 0x0000 - 0x1fff
{
	map(0x0000, 0x007f).mirror(0x0f00).rw(m_tia, FUNC(tia_video_device::read), FUNC(tia_video_device::write));
	map(0x0080, 0x00ff).mirror(0x0d00).ram().share("riot_ram");
#if USE_NEW_RIOT
	map(0x0280, 0x029f).mirror(0x0d00).m("riot", FUNC(mos6532_t::io_map));
#else
	map(0x0280, 0x029f).mirror(0x0d00).rw("riot", FUNC(riot6532_device::read), FUNC(riot6532_device::write));
#endif
	// map(0x1000, 0x1fff) is cart data and it is configured at reset time, depending on the mounted cart!
}


READ8_MEMBER(a2600_state::cart_over_all_r)
{
	if (!machine().side_effects_disabled())
		m_cart->write_bank(space, offset, 0);

	int masked_offset = offset &~ 0x0d00;
	uint8_t ret = 0x00;

	if (masked_offset < 0x80)
	{
		ret = m_tia->read(space, masked_offset&0x7f);
	}
	else if (masked_offset < 0x100)
	{
		ret = m_riot_ram[masked_offset & 0x7f];
	}
	/* 0x100 - 0x1ff already masked out */
	else if (masked_offset < 0x280)
	{
		ret = m_tia->read(space, masked_offset&0x7f);
	}
	else if (masked_offset < 0x2a0)
	{
#if USE_NEW_RIOT
		ret = m_riot->io_r(space, masked_offset);
#else
		ret = m_riot->read(space, masked_offset);
#endif
	}
	else if (masked_offset < 0x300)
	{
		/* 0x2a0 - 0x2ff nothing? */
	}
	/* 0x300 - 0x3ff already masked out */

	return ret;
}

WRITE8_MEMBER(a2600_state::cart_over_all_w)
{
	m_cart->write_bank(space, offset, 0);

	int masked_offset = offset &~ 0x0d00;

	if (masked_offset < 0x80)
	{
		m_tia->write(space, masked_offset & 0x7f, data);
	}
	else if (masked_offset < 0x100)
	{
		m_riot_ram[masked_offset & 0x7f] = data;
	}
	/* 0x100 - 0x1ff already masked out */
	else if (masked_offset < 0x280)
	{
		m_tia->write(space, masked_offset & 0x7f, data);
	}
	else if (masked_offset < 0x2a0)
	{
#if USE_NEW_RIOT
		m_riot->io_w(space, masked_offset, data);
#else
		m_riot->write(space, masked_offset, data);
#endif
	}
	else if (masked_offset < 0x300)
	{
		/* 0x2a0 - 0x2ff nothing? */
	}
	/* 0x300 - 0x3ff already masked out */
}

WRITE8_MEMBER(a2600_base_state::switch_A_w)
{
	/* Left controller port */
	m_joy1->joy_w( data >> 4 );

	/* Right controller port */
	m_joy2->joy_w( data & 0x0f );

//  switch( ioport("CONTROLLERS")->read() % CATEGORY_SELECT )
//  {
//  case 0x0a:  /* KidVid voice module */
//      m_cassette->change_state(( data & 0x02 ) ? CASSETTE_MOTOR_DISABLED : (CASSETTE_MOTOR_ENABLED | CASSETTE_PLAY), CASSETTE_MOTOR_DISABLED );
//      break;
//  }
}

READ8_MEMBER(a2600_base_state::switch_A_r)
{
	uint8_t val = 0;

	// Left controller port PINs 1-4 ( 4321 )
	val |= (m_joy1->read_joy() & 0x0f) << 4;

	// Right controller port PINs 1-4 ( 4321 )
	val |= m_joy2->read_joy() & 0x0f;

	return val;
}

WRITE8_MEMBER(a2600_base_state::switch_B_w)
{
}

WRITE_LINE_MEMBER(a2600_base_state::irq_callback)
{
}

READ8_MEMBER(a2600_base_state::riot_input_port_8_r)
{
	return m_swb->read();
}


READ16_MEMBER(a2600_base_state::a2600_read_input_port)
{
	switch (offset)
	{
	case 0: // Left controller port PIN 5
		return m_joy1->read_pot_x();

	case 1: // Left controller port PIN 9
		return m_joy1->read_pot_y();

	case 2: // Right controller port PIN 5
		return m_joy2->read_pot_x();

	case 3: // Right controller port PIN 9
		return m_joy2->read_pot_y();

	case 4: // Left controller port PIN 6
		return (m_joy1->read_joy() & 0x20) ? 0xff : 0x7f;

	case 5: // Right controller port PIN 6
		return (m_joy2->read_joy() & 0x20) ? 0xff : 0x7f;
	}
	return 0xff;
}

/* There are a few games that do an LDA ($80-$FF),Y instruction.
   The contents off the databus then depend on whatever was read
   from the RAM. To do this really properly the 6502 core would
   need to keep track of the last databus contents so we can query
   that. For now this is a quick hack to determine that value anyway.
   Examples:
   Q-Bert's Qubes (NTSC,F6) at 0x1594
   Berzerk at 0xF093.
*/
READ8_MEMBER(a2600_base_state::a2600_get_databus_contents)
{
	uint16_t  last_address, prev_address;
	uint8_t   last_byte, prev_byte;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);

	last_address = m_maincpu->pc() + 1;
	if ( ! ( last_address & 0x1080 ) )
	{
		return offset;
	}
	last_byte = prog_space.read_byte(last_address );
	if ( last_byte < 0x80 || last_byte == 0xFF )
	{
		return last_byte;
	}
	prev_address = last_address - 1;
	if ( ! ( prev_address & 0x1080 ) )
	{
		return last_byte;
	}
	prev_byte = prog_space.read_byte(prev_address );
	if ( prev_byte == 0xB1 )
	{   /* LDA (XX),Y */
		return prog_space.read_byte(last_byte + 1 );
	}
	return last_byte;
}

#if 0
static const rectangle visarea[4] = {
	{ 26, 26 + 160 + 16, 24, 24 + 192 + 31 },   /* 262 */
	{ 26, 26 + 160 + 16, 32, 32 + 228 + 31 },   /* 312 */
	{ 26, 26 + 160 + 16, 45, 45 + 240 + 31 },   /* 328 */
	{ 26, 26 + 160 + 16, 48, 48 + 240 + 31 }    /* 342 */
};
#endif

WRITE16_MEMBER(a2600_base_state::a2600_tia_vsync_callback)
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_NTSC ) * 228 * m_current_screen_height );
			}
		}
	}
}

WRITE16_MEMBER(a2600_base_state::a2600_tia_vsync_callback_pal)
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(supported_screen_heights); i++ )
	{
		if ( data >= supported_screen_heights[i] - 3 && data <= supported_screen_heights[i] + 3 )
		{
			if ( supported_screen_heights[i] != m_current_screen_height )
			{
				m_current_screen_height = supported_screen_heights[i];
//              m_screen->configure(228, m_current_screen_height, &visarea[i], HZ_TO_ATTOSECONDS( MASTER_CLOCK_PAL ) * 228 * m_current_screen_height );
			}
		}
	}
}

// TODO: is this the correct behavior for the real hardware?!?
READ8_MEMBER(a2600_state::cart_over_riot_r)
{
	if (!machine().side_effects_disabled())
		m_cart->write_bank(space, offset, 0);
	return m_riot_ram[0x20 + offset];
}

WRITE8_MEMBER(a2600_state::cart_over_riot_w)
{
	m_cart->write_bank(space, offset, 0);
	m_riot_ram[0x20 + offset] = data;

}

WRITE8_MEMBER(a2600_state::cart_over_tia_w)
{
	// Both Cart & TIA see these addresses
	m_cart->write_bank(space, offset, data);
	m_tia->write(space, offset, data);
}

void a2600_base_state::machine_start()
{
	m_current_screen_height = m_screen->height();
	memset(m_riot_ram, 0x00, 0x80);

	save_item(NAME(m_current_screen_height));
}

void a2600_state::machine_start()
{
	a2600_base_state::machine_start();

	switch (m_cart->get_cart_type())
	{
	case A26_2K:
	case A26_4K:
	case A26_F4:
	case A26_F8:
	case A26_F8SW:
	case A26_FA:
	case A26_E0:
	case A26_E7:
	case A26_CV:
	case A26_DC:
	case A26_FV:
	case A26_8IN1:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_F6:
	case A26_DPC:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_FE:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_ram)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x01fe, 0x01ff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_bank)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x01fe, 0x01fe, write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_3E:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_ram)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x00, 0x3f, write8_delegate(*this, FUNC(a2600_state::cart_over_tia_w)));
		break;
	case A26_3F:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x00, 0x3f, write8_delegate(*this, FUNC(a2600_state::cart_over_tia_w)));
		break;
	case A26_UA:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x200, 0x27f, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_bank)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	case A26_JVP:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		// to verify the actual behavior...
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xfa0, 0xfc0, read8_delegate(*this, FUNC(a2600_state::cart_over_riot_r)), write8_delegate(*this, FUNC(a2600_state::cart_over_riot_w)));
		break;
	case A26_4IN1:
	case A26_32IN1:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		break;
	case A26_SS:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		break;
	case A26_CM:
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)));
		break;
	case A26_X07:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x0000, 0x0fff, read8_delegate(*this, FUNC(a2600_state::cart_over_all_r)), write8_delegate(*this, FUNC(a2600_state::cart_over_all_w)));
		break;
	case A26_HARMONY:
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1000, 0x1fff, read8_delegate(*m_cart, FUNC(vcs_cart_slot_device::read_rom)), write8_delegate(*m_cart, FUNC(vcs_cart_slot_device::write_bank)));
		break;
	}

	// Banks may have changed, reset the cpu so it uses the correct reset vector
	m_maincpu->reset();
}


#ifdef UNUSED_FUNCTIONS
// try to detect 2600 controller setup. returns 32bits with left/right controller info
unsigned a2600_state::long detect_2600controllers()
{
	enum
	{
		JOYS = 0x001,
		PADD = 0x002,
		KEYP = 0x004,
		LGUN = 0x008,
		INDY = 0x010,
		BOOS = 0x020,
		KVID = 0x040,
		CMTE = 0x080,
		MLNK = 0x100,
		AMSE = 0x200,
		CX22 = 0x400,
		CX80 = 0x800
	};

	unsigned int left,right;
	int i,j,foundkeypad = 0;
	uint8_t *cart;
	static const unsigned char signatures[][5] =  {
		{ 0x55, 0xa5, 0x3c, 0x29, 0}, // star raiders
		{ 0xf9, 0xff, 0xa5, 0x80, 1}, // sentinel
		{ 0x81, 0x02, 0xe8, 0x86, 1}, // shooting arcade
		{ 0x02, 0xa9, 0xec, 0x8d, 1}, // guntest4 tester
		{ 0x85, 0x2c, 0x85, 0xa7, 2}, // INDY 500
		{ 0xa1, 0x8d, 0x9d, 0x02, 2}, // omega race INDY
		{ 0x65, 0x72, 0x44, 0x43, 2}, // Sprintmaster INDY
		{ 0x89, 0x8a, 0x99, 0xaa, 3}, // omega race
		{ 0x9a, 0x8e, 0x81, 0x02, 4},
		{ 0xdd, 0x8d, 0x80, 0x02, 4},
		{ 0x85, 0x8e, 0x81, 0x02, 4},
		{ 0x8d, 0x81, 0x02, 0xe6, 4},
		{ 0xff, 0x8d, 0x81, 0x02, 4},
		{ 0xa9, 0x03, 0x8d, 0x81, 5},
		{ 0xa9, 0x73, 0x8d, 0x80, 6},
		//                                  { 0x82, 0x02, 0x85, 0x8f, 7}, // Mind Maze (really Mind Link??)
		{ 0xa9, 0x30, 0x8d, 0x80, 7}, // Bionic Breakthrough
		{ 0x02, 0x8e, 0x81, 0x02, 7}, // Telepathy
		{ 0x41, 0x6d, 0x69, 0x67, 9}, // Missile Command Amiga Mouse
		{ 0x43, 0x58, 0x2d, 0x32, 10}, // Missile Command CX22 TrackBall
		{ 0x43, 0x58, 0x2d, 0x38, 11}, // Missile Command CX80 TrackBall
		{ 0x4e, 0xa8, 0xa4, 0xa2, 12}, // Omega Race for Joystick ONLY
		{ 0xa6, 0xef, 0xb5, 0x38, 8} // Warlords.. paddles ONLY
	};
	// start with this.. if anyone finds a game that does NOT work with both controllers enabled
	// it can be fixed here with a new signature (note that the Coleco Gemini has this setup also)
	left = JOYS+PADD; right = JOYS+PADD;
	// default for bad dumps and roms too large to have special controllers
	if ((m_cart_size > 0x4000) || (m_cart_size & 0x7ff)) return (left << 16) + right;

	cart = CART;
	for (i = 0; i < m_cart_size - (sizeof signatures/sizeof signatures[0]); i++)
	{
		for (j = 0; j < (sizeof signatures/sizeof signatures[0]); j++)
		{
			if (!memcmp(&cart[i], &signatures[j],sizeof signatures[0] - 1))
			{
				int k = signatures[j][4];
				if (k == 0) return (JOYS << 16) + KEYP;
				if (k == 1) return (LGUN << 16);
				if (k == 2) return (INDY << 16) + INDY;
				if (k == 3) return (BOOS << 16) + BOOS;
				if (k == 5) return (JOYS << 16) + KVID;
				if (k == 6) return (CMTE << 16) + CMTE;
				if (k == 7) return (MLNK << 16) + MLNK;
				if (k == 8) return (PADD << 16) + PADD;
				if (k == 9) return (AMSE << 16) + AMSE;
				if (k == 10) return (CX22 << 16) + CX22;
				if (k == 11) return (CX80 << 16) + CX80;
				if (k == 12) return (JOYS << 16) + JOYS;
				if (k == 4) foundkeypad = 1;
			}
		}
	}
	if (foundkeypad) return (KEYP << 16) + KEYP;
	return (left << 16) + right;
}
#endif


static INPUT_PORTS_START( a2600 )
	PORT_START("SWB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset Game") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Select Game") PORT_CODE(KEYCODE_1)
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "TV Type" ) PORT_CODE(KEYCODE_C) PORT_TOGGLE
	PORT_DIPSETTING(    0x08, "Color" )
	PORT_DIPSETTING(    0x00, "B&W" )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x00, "Left Diff. Switch" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x40, "A" )
	PORT_DIPSETTING(    0x00, "B" )
	PORT_DIPNAME( 0x80, 0x00, "Right Diff. Switch" ) PORT_CODE(KEYCODE_4) PORT_TOGGLE
	PORT_DIPSETTING(    0x80, "A" )
	PORT_DIPSETTING(    0x00, "B" )
INPUT_PORTS_END


static void a2600_cart(device_slot_interface &device)
{
	device.option_add_internal("a26_2k",    A26_ROM_2K);
	device.option_add_internal("a26_4k",    A26_ROM_4K);
	device.option_add_internal("a26_f4",    A26_ROM_F4);
	device.option_add_internal("a26_f6",    A26_ROM_F6);
	device.option_add_internal("a26_f8",    A26_ROM_F8);
	device.option_add_internal("a26_f8sw",  A26_ROM_F8_SW);
	device.option_add_internal("a26_fa",    A26_ROM_FA);
	device.option_add_internal("a26_fe",    A26_ROM_FE);
	device.option_add_internal("a26_3e",    A26_ROM_3E);
	device.option_add_internal("a26_3f",    A26_ROM_3F);
	device.option_add_internal("a26_e0",    A26_ROM_E0);
	device.option_add_internal("a26_e7",    A26_ROM_E7);
	device.option_add_internal("a26_ua",    A26_ROM_UA);
	device.option_add_internal("a26_cv",    A26_ROM_CV);
	device.option_add_internal("a26_dc",    A26_ROM_DC);
	device.option_add_internal("a26_fv",    A26_ROM_FV);
	device.option_add_internal("a26_jvp",   A26_ROM_JVP);
	device.option_add_internal("a26_cm",    A26_ROM_COMPUMATE);
	device.option_add_internal("a26_ss",    A26_ROM_SUPERCHARGER);
	device.option_add_internal("a26_dpc",   A26_ROM_DPC);
	device.option_add_internal("a26_4in1",  A26_ROM_4IN1);
	device.option_add_internal("a26_8in1",  A26_ROM_8IN1);
	device.option_add_internal("a26_32in1", A26_ROM_32IN1);
	device.option_add_internal("a26_x07",   A26_ROM_X07);
	device.option_add_internal("a26_harmony",   A26_ROM_HARMONY);
}

void a2600_state::a2600_cartslot(machine_config &config)
{
	VCS_CART_SLOT(config, "cartslot", a2600_cart, nullptr);

	/* software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("a2600");
	SOFTWARE_LIST(config, "cass_list").set_original("a2600_cass");
}

void a2600_state::a2600(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, MASTER_CLOCK_NTSC / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_state::a2600_mem);

	/* video hardware */
	TIA_NTSC_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(a2600_state::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(a2600_state::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(a2600_state::a2600_tia_vsync_callback));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK_NTSC, 228, 26, 26 + 160 + 16, 262, 24 , 24 + 192 + 31);
	m_screen->set_screen_update("tia_video", FUNC(tia_video_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", MASTER_CLOCK_NTSC/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
#if USE_NEW_RIOT
	MOS6532_NEW(config, m_riot, MASTER_CLOCK_NTSC / 3);
	m_riot->pa_rd_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->pb_rd_callback().set(FUNC(a2600_state::riot_input_port_8_r));
	m_riot->pb_wr_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(a2600_state::irq_callback));
#else
	RIOT6532(config, m_riot, MASTER_CLOCK_NTSC / 3);
	m_riot->in_pa_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->out_pa_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->in_pb_callback().set(FUNC(a2600_state::riot_input_port_8_r));
	m_riot->out_pb_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_callback().set(FUNC(a2600_state::irq_callback));
#endif

	VCS_CONTROL_PORT(config, CONTROL1_TAG, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, CONTROL2_TAG, vcs_control_port_devices, nullptr);

	a2600_cartslot(config);
	subdevice<software_list_device>("cart_list")->set_filter("NTSC");
}


void a2600_state::a2600p(machine_config &config)
{
	/* basic machine hardware */
	M6507(config, m_maincpu, MASTER_CLOCK_PAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &a2600_state::a2600_mem);

	/* video hardware */
	TIA_PAL_VIDEO(config, m_tia, 0, "tia");
	m_tia->read_input_port_callback().set(FUNC(a2600_state::a2600_read_input_port));
	m_tia->databus_contents_callback().set(FUNC(a2600_state::a2600_get_databus_contents));
	m_tia->vsync_callback().set(FUNC(a2600_state::a2600_tia_vsync_callback_pal));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK_PAL, 228, 26, 26 + 160 + 16, 312, 32, 32 + 228 + 31);
	m_screen->set_screen_update("tia_video", FUNC(tia_video_device::screen_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	TIA(config, "tia", MASTER_CLOCK_PAL/114).add_route(ALL_OUTPUTS, "mono", 0.90);

	/* devices */
#if USE_NEW_RIOT
	MOS6532_NEW(config, m_riot, MASTER_CLOCK_PAL / 3);
	m_riot->pa_rd_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->pa_wr_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->pb_rd_callback().set(FUNC(a2600_state::riot_input_port_8_r));
	m_riot->pb_wr_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_wr_callback().set(FUNC(a2600_state::irq_callback));
#else
	RIOT6532(config, m_riot, MASTER_CLOCK_PAL / 3);
	m_riot->in_pa_callback().set(FUNC(a2600_state::switch_A_r));
	m_riot->out_pa_callback().set(FUNC(a2600_state::switch_A_w));
	m_riot->in_pb_callback().set(FUNC(a2600_state::riot_input_port_8_r));
	m_riot->out_pb_callback().set(FUNC(a2600_state::switch_B_w));
	m_riot->irq_callback().set(FUNC(a2600_state::irq_callback));
#endif

	VCS_CONTROL_PORT(config, CONTROL1_TAG, vcs_control_port_devices, "joy");
	VCS_CONTROL_PORT(config, CONTROL2_TAG, vcs_control_port_devices, nullptr);

	a2600_cartslot(config);
	subdevice<software_list_device>("cart_list")->set_filter("PAL");
}


ROM_START( a2600 )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_ERASEFF )
ROM_END

#define rom_a2600p rom_a2600

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME */
CONS( 1977, a2600,  0,      0,      a2600,   a2600, a2600_state, empty_init, "Atari",    "Atari 2600 (NTSC)" , MACHINE_SUPPORTS_SAVE )
CONS( 1978, a2600p, a2600,  0,      a2600p,  a2600, a2600_state, empty_init, "Atari",    "Atari 2600 (PAL)",   MACHINE_SUPPORTS_SAVE )
